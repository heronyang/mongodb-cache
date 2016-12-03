/**
 * cache
 * =====
 * A non thread-safe cache interface, provides get and post operations.
 *
 * Reference
 * =========
 *
 * MongoDB C Driver API
 * - http://mongoc.org/libmongoc/1.4.2/index.html#api-reference
 * - https://raw.githubusercontent.com/mongodb/mongo-c-driver/master/tests/test-mongoc-client-pool.c
 * Bson:
 * - http://mongoc.org/libbson/1.4.2/
 * - https://github.com/mongodb/libbson/blob/ca2f3ad7548a25580312814ab54bf3e93a9b6a30/src/bson/bson.h
 *
 */

#include "cache.h"

bool isValidChecksum(Meta *meta);

/* Connection Objects */

static mongoc_uri_t *uri;
static mongoc_client_pool_t *pool;

typedef struct _Connection {
    mongoc_client_t      *client;
    mongoc_database_t    *database;
    mongoc_collection_t  *collection;
} Connection;

/* Prototypes */

void meta2bson(bson_t *doc, Meta *meta);
Meta *bson2meta(const bson_t *doc, const char *cid);

// garbage collection helper functions
void db_update_accessed_time(Connection *connection, const char *cid);
void db_scan_and_mark_ttl_expired_meta(Connection *connection);
void db_scan_and_mark_old_meta(Connection *connection);
int64_t get_collection_count(Connection *connection);
time_t *get_accessed_times(Connection *connection, int64_t count);
void db_mark_meta(Connection *connection, const char *cid);
void db_check_and_mark_ttl_expired_meta(Connection *connection,
        const bson_t *doc);

void db_delete_marked_meta(Connection *connection);

// bson parsers
char *get_cid_from_doc(const bson_t *doc);
uint32_t get_ttl_from_doc(const bson_t *doc);
time_t get_created_time_from_doc(const bson_t *doc);
time_t get_accessed_time_from_doc(const bson_t *doc);

/* Database */

void db_init() {

    mongoc_init();
    uri = mongoc_uri_new(MONGODB_URL);
    pool = mongoc_client_pool_new(uri);

}

void db_deinit() {

    mongoc_uri_destroy(uri);
    mongoc_client_pool_destroy(pool);
    mongoc_cleanup();

}

Connection *retrive_connection() {

    Connection *connection  = malloc_w(sizeof(Connection));

    mongoc_client_t *client = mongoc_client_pool_pop(pool);
    connection->client      = client;
    connection->database    = mongoc_client_get_database(client, MONGODB_DB);
    connection->collection  = mongoc_client_get_collection(client, MONGODB_DB, MONGODB_COLLECTION);

    return connection;

}

void release_connection(Connection *connection) {

    mongoc_collection_destroy(connection->collection);
    mongoc_database_destroy(connection->database);
    mongoc_client_pool_push(pool, connection->client);

    free(connection);

}

/* Operation */

Meta *db_get(const char *cid) {

    // connect to database
    Connection *connection = retrive_connection();
    mongoc_collection_t *collection = connection->collection;

    // variable setup
    mongoc_cursor_t *cursor;
    const bson_t *doc;
    bson_t *query;
    bson_oid_t oid;

    // query
    query = bson_new();
    bson_oid_init_from_string (&oid, cid);
    BSON_APPEND_OID(query, "_id", &oid);

    cursor = mongoc_collection_find(collection,
            MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);

    if(!mongoc_cursor_next(cursor, &doc)) {
        bson_destroy(query);
        mongoc_cursor_destroy(cursor);
        release_connection(connection);
        return NULL;
    }

    // update the accessed time
    db_update_accessed_time(connection, cid);

    // parse bson return back to meta
    Meta *meta = bson2meta(doc, cid);

    bson_destroy(query);
    mongoc_cursor_destroy(cursor);
    release_connection(connection);

    return meta;

}

Meta *bson2meta(const bson_t *doc, const char *cid) {

    if(doc == NULL) {
        return NULL;
    }

    bson_iter_t iter;
    if(!bson_iter_init(&iter, doc)) {
        return NULL;
    }

    Meta *meta = malloc_w(sizeof(Meta));
    meta__init(meta);

    // cid
    meta->cid = malloc_w(SHA1_LENGTH);
    memcpy(meta->cid, cid, SHA1_LENGTH);

    // sid
    bson_iter_find(&iter, "sid");
    const char *sid = bson_iter_utf8(&iter, 0);
    meta->sid = malloc_w(SHA1_LENGTH);
    memcpy(meta->sid, sid, SHA1_LENGTH);

    // len
    bson_iter_find(&iter, "len");
    uint32_t len  = bson_iter_int64(&iter);
    meta->content.len = len;

    // content
    bson_subtype_t subtype;
    meta->content.data = malloc_w(len);
    uint32_t len_saved;
    bson_iter_find(&iter, "content");
    bson_iter_binary(&iter, &subtype, &len_saved,
            (const uint8_t **) (&meta->content.data));

    printf("len saved = %u\n", len_saved);
    int i;
    for(i=0;i<len_saved;i++) {
        printf("[%02x] ", meta->content.data[i]);
    }
    printf("\n");
    
    if(len != len_saved) {
        printf("Error found in validating content length\n");
        return NULL;
    }

    // initial_seq
    bson_iter_find(&iter, "initial_seq");
    meta->initial_seq = bson_iter_int64(&iter);

    // ttl
    bson_iter_find(&iter, "ttl");
    meta->ttl = bson_iter_int64(&iter);

    // created_time
    bson_iter_find(&iter, "created_time");
    meta->created_time = bson_iter_time_t(&iter);

    // accessed_time
    bson_iter_find(&iter, "accessed_time");
    meta->accessed_time = bson_iter_time_t(&iter);

    return meta;

}

void db_update_accessed_time(Connection *connection, const char *cid) {

    // get collection
    mongoc_collection_t *collection = connection->collection;

    // variable setup
    bson_t *query, *update;
    bson_oid_t oid;
    bson_error_t error;

    time_t now = time(NULL);
    bson_oid_init_from_string (&oid, cid);
    query = BCON_NEW("_id", BCON_OID(&oid));
    update = BCON_NEW ("$set", "{",
            "accessed_time", BCON_DATE_TIME(now * 1000),
            "}");

    // update
    if (!mongoc_collection_update(collection,
                MONGOC_UPDATE_NONE, query, update, NULL, &error)) {
        printf("Error: %s\n", error.message);
    }

    // release
    bson_destroy(query);
    bson_destroy(update);

}

bool db_post(Meta *meta) {

    if(!isValidChecksum(meta)) {
        printf("Invalid meta with wrong checksum\n");
        return false;
    }

    Connection *connection = retrive_connection();
    mongoc_collection_t *collection = connection->collection;

    bson_t *doc;
    doc = bson_new();
    meta2bson(doc, meta);

    bson_error_t error;
    if (!mongoc_collection_insert(collection, MONGOC_INSERT_NONE, doc, NULL, &error)) {
        bson_destroy (doc);
        release_connection(connection);
        printf("%s\n", error.message);
        return false;
    }

    bson_destroy (doc);

    release_connection(connection);
    return true;

}

void meta2bson(bson_t *doc, Meta *meta) {

    bson_oid_t oid;

    bson_init(doc);

    bson_oid_init_from_string (&oid, meta->cid);
    BSON_APPEND_OID (doc, "_id", &oid);
    BSON_APPEND_UTF8(doc, "sid", meta->sid);

    BSON_APPEND_INT64(doc, "len", meta->content.len);
    BSON_APPEND_BINARY(doc, "content", BSON_SUBTYPE_BINARY,
            meta->content.data, meta->content.len);
    BSON_APPEND_INT64(doc, "initial_seq", meta->initial_seq);

    BSON_APPEND_INT64(doc, "ttl", meta->ttl);

    // set to current time
    BSON_APPEND_TIME_T(doc, "created_time", time(NULL));
    BSON_APPEND_TIME_T(doc, "accessed_time", time(NULL));

}

void db_cleanup() {

    printf("Garbage collecting starts\n");

    Connection *connection = retrive_connection();

    db_scan_and_mark_ttl_expired_meta(connection);
    db_scan_and_mark_old_meta(connection);
    db_delete_marked_meta(connection);

    release_connection(connection);

    printf("Garbage collecting done\n");
    fflush(stdout);

}

void db_scan_and_mark_ttl_expired_meta(Connection *connection) {

    // get collection
    mongoc_collection_t *collection = connection->collection;

    // variable setup
    mongoc_cursor_t *cursor;
    const bson_t *doc;
    bson_t *query;

    // query
    query = bson_new();
    cursor = mongoc_collection_find(collection,
            MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);

    while(mongoc_cursor_next(cursor, &doc)) {
        // check and mark ttl expired meta
        db_check_and_mark_ttl_expired_meta(connection, doc);
    }

    // release
    bson_destroy(query);
    mongoc_cursor_destroy(cursor);

}

void db_check_and_mark_ttl_expired_meta(Connection *connection,
        const bson_t *doc) {

    char *cid;
    uint32_t ttl;
    time_t created_time, now;

    // get ttl
    ttl = get_ttl_from_doc(doc);
    if(ttl == 0) {
        printf("Error in accessing ttl, skip\n");
        return;
    }

    // get created_time
    created_time = get_created_time_from_doc(doc);
    if(created_time < 0) {
        printf("Error in accessing created_time, skip\n");
        return;
    }

    // get cid
    cid = get_cid_from_doc(doc);
    if(cid == NULL) {
        printf("Error in accessing cid, skip\n");
        return;
    }

    // mark it if it's expired
    now = time(NULL);
    if(created_time + ttl <= now) {
        db_mark_meta(connection, cid);
        printf("marked: cid = %*s, ttl = %u, created_time = %ld\n",
                SHA1_LENGTH, cid, ttl, created_time);
    }

    free(cid);

}

char *get_cid_from_doc(const bson_t *doc) {

    bson_iter_t iter;
    const bson_oid_t *oid;
    char *cid = malloc_w(SHA1_LENGTH);

    if(bson_iter_init(&iter, doc) &&
            bson_iter_find(&iter, "_id") &&
            BSON_ITER_HOLDS_OID(&iter)) {
        oid = bson_iter_oid(&iter);
        bson_oid_to_string(oid, cid);
        return cid;
    }

    return NULL;

}

uint32_t get_ttl_from_doc(const bson_t *doc) {

    bson_iter_t iter;
    if(bson_iter_init(&iter, doc) &&
            bson_iter_find(&iter, "ttl")) {
        return bson_iter_int64(&iter);
    } 

    return 0;

}

time_t get_created_time_from_doc(const bson_t *doc) {

    bson_iter_t iter;
    if(bson_iter_init(&iter, doc) &&
            bson_iter_find(&iter, "created_time")) {
        return bson_iter_time_t(&iter);
    } 

    return 0;

}

time_t get_accessed_time_from_doc(const bson_t *doc) {

    bson_iter_t iter;
    if(bson_iter_init(&iter, doc) &&
            bson_iter_find(&iter, "accessed_time")) {
        return bson_iter_time_t(&iter);
    } 

    return 0;

}


void db_scan_and_mark_old_meta(Connection *connection) {

    time_t *accessed_times;

    int64_t count = get_collection_count(connection);
    if(count < 0) {
        printf("Error in counting documents in collection\n");
        return;
    }

    if(count < GARBAGE_COLLECTION_THRESHOLD) {
        return;
    }

    accessed_times = get_accessed_times(connection, count);
    if(accessed_times == NULL) {
        return;
    }

    // print for debug
    int64_t i;
    printf("%"PRId64" documents counted.\n", count);
    for(i=0; i<count; i++) {
        printf("time = %ld\n", accessed_times[i]);
    }

    free(accessed_times);

}

int64_t get_collection_count(Connection *connection) {

    bson_t *query;
    bson_error_t error;

    query = bson_new();
    mongoc_collection_t *collection = connection->collection;

    return mongoc_collection_count(collection,
            MONGOC_QUERY_NONE, query, 0, 0, NULL, &error);

}

time_t *get_accessed_times(Connection *connection, int64_t count) {

    int64_t i = 0;
    time_t *accessed_times = malloc_w(sizeof(time_t) * count);

    // get collection
    mongoc_collection_t *collection = connection->collection;

    // variable setup
    mongoc_cursor_t *cursor;
    const bson_t *doc;
    bson_t *query;

    // query
    query = bson_new();
    cursor = mongoc_collection_find(collection,
            MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);

    while(mongoc_cursor_next(cursor, &doc)) {

        time_t accessed_time = get_accessed_time_from_doc(doc);
        if(accessed_time < 0) {
            printf("Error in accessing accessed_time\n");
            return NULL;
        }

        accessed_times[i] =accessed_time;
        i++;

    }

    // release
    bson_destroy(query);
    mongoc_cursor_destroy(cursor);

    return accessed_times;
}

void db_mark_meta(Connection *connection, const char *cid) {

    // get collection
    mongoc_collection_t *collection = connection->collection;

    // variable setup
    bson_t *query, *update;
    bson_oid_t oid;
    bson_error_t error;

    bson_oid_init_from_string (&oid, cid);
    query = BCON_NEW("_id", BCON_OID(&oid));
    update = BCON_NEW ("$set", "{",
            "is_garbage", BCON_BOOL(true),
            "}");

    // update
    if (!mongoc_collection_update(collection,
                MONGOC_UPDATE_NONE, query, update, NULL, &error)) {
        printf("Error: %s\n", error.message);
    }

    // release
    bson_destroy(query);
    bson_destroy(update);

}

void db_delete_marked_meta(Connection *connection) {
}

/* Helper */

bool isValidChecksum(Meta *meta) {
    // TODO
    return true;
}
