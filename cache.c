#include "cache.h"

bool isValidChecksum(Meta *meta);

/*
 * Reference
 *
 * MongoDB C Driver API
 * - http://mongoc.org/libmongoc/1.4.2/index.html#api-reference
 * - https://raw.githubusercontent.com/mongodb/mongo-c-driver/master/tests/test-mongoc-client-pool.c
 * Bson:
 * - http://mongoc.org/libbson/1.4.2/
 * - https://github.com/mongodb/libbson/blob/ca2f3ad7548a25580312814ab54bf3e93a9b6a30/src/bson/bson.h
 *
 */

static mongoc_uri_t *uri;
static mongoc_client_pool_t *pool;

typedef struct _Connection {
    mongoc_client_t      *client;
    mongoc_database_t    *database;
    mongoc_collection_t  *collection;
} Connection;

void bson_init_from_meta(bson_t *doc, Meta *meta);
Meta *bson2meta(const bson_t *doc, const char *cid);

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
    uint32_t len  = bson_iter_int32(&iter);

    // content
    bson_subtype_t subtype;
    meta->content.data = malloc_w(len);
    const uint8_t *content = meta->content.data;
    uint32_t len_saved;
    bson_iter_find(&iter, "content");
    bson_iter_binary(&iter, &subtype, &len_saved, &content);
    
    if(len != len_saved) {
        printf("Error found in validating content length\n");
        return NULL;
    }

    // initial_seq
    bson_iter_find(&iter, "initial_seq");
    meta->initial_seq = bson_iter_int32(&iter);

    // ttl
    bson_iter_find(&iter, "ttl");
    meta->ttl = bson_iter_int32(&iter);

    // created_time
    bson_iter_find(&iter, "created_time");
    meta->created_time = bson_iter_time_t(&iter);

    // accessed_time
    bson_iter_find(&iter, "accessed_time");
    meta->accessed_time = bson_iter_time_t(&iter);

    return meta;

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
    bson_init_from_meta(doc, meta);

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

void bson_init_from_meta(bson_t *doc, Meta *meta) {

    bson_oid_t oid;

    bson_init(doc);

    bson_oid_init_from_string (&oid, meta->cid);
    BSON_APPEND_OID (doc, "_id", &oid);
    BSON_APPEND_UTF8(doc, "sid", meta->sid);

    BSON_APPEND_INT32(doc, "len", meta->content.len);
    BSON_APPEND_BINARY(doc, "content", BSON_SUBTYPE_BINARY,
            meta->content.data, meta->content.len);
    BSON_APPEND_INT32(doc, "initial_seq", meta->initial_seq);

    BSON_APPEND_INT32(doc, "ttl", meta->ttl);
    BSON_APPEND_TIME_T(doc, "created_time", meta->created_time);
    BSON_APPEND_TIME_T(doc, "accessed_time", meta->accessed_time);

}

bool isValidChecksum(Meta *meta) {
    // TODO
    return true;
}
