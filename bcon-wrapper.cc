#include "bcon-wrapper.h"

char *get_cid_from_doc(const bson_t *doc) {

    bson_iter_t iter;
    const bson_oid_t *oid;
    char *cid = (char *)malloc_w(SHA1_LENGTH);

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

void meta2bson(bson_t *doc, Meta meta) {

    bson_oid_t oid;

    bson_init(doc);

    bson_oid_init_from_string(&oid, meta.cid().c_str());
    BSON_APPEND_OID (doc, "_id", &oid);
    BSON_APPEND_UTF8(doc, "sid", meta.sid().c_str());

    BSON_APPEND_INT64(doc, "len", meta.content().size());
    BSON_APPEND_BINARY(doc, "content", BSON_SUBTYPE_BINARY,
            (const uint8_t*)meta.content().c_str(), meta.content().size());
    BSON_APPEND_INT64(doc, "initial_seq", meta.initial_seq());

    BSON_APPEND_INT64(doc, "ttl", meta.ttl());

    // set to current time
    BSON_APPEND_TIME_T(doc, "created_time", time(NULL));
    BSON_APPEND_TIME_T(doc, "accessed_time", time(NULL));

}

Meta *bson2meta(const bson_t *doc, const char *cid) {

    if(doc == NULL) {
        return NULL;
    }

    bson_iter_t iter;
    if(!bson_iter_init(&iter, doc)) {
        return NULL;
    }

    Meta *meta = new Meta();

    // cid
    meta->set_cid(cid);

    // sid
    bson_iter_find(&iter, "sid");
    const char *sid = bson_iter_utf8(&iter, 0);
    meta->set_sid(sid);

    // len
    bson_iter_find(&iter, "len");
    uint32_t len  = bson_iter_int64(&iter);

    char *content_holder;
    content_holder = (char *)malloc_w(len);

    // content
    bson_subtype_t subtype;
    uint32_t len_saved;
    bson_iter_find(&iter, "content");
    bson_iter_binary(&iter, &subtype, &len_saved,
            (const uint8_t **) (&content_holder));

    meta->set_content(content_holder);

    printf("len saved = %u\n", len_saved);
    unsigned i;
    for(i=0;i<len_saved;i++) {
        printf("[%02x] ", meta->content().at(i));
    }
    printf("\n");
    
    if(len != len_saved) {
        printf("Error found in validating content length\n");
        return NULL;
    }

    // initial_seq
    bson_iter_find(&iter, "initial_seq");
    meta->set_initial_seq(bson_iter_int64(&iter));

    // ttl
    bson_iter_find(&iter, "ttl");
    meta->set_ttl(bson_iter_int64(&iter));

    // created_time
    bson_iter_find(&iter, "created_time");
    meta->set_created_time(bson_iter_time_t(&iter));

    // accessed_time
    bson_iter_find(&iter, "accessed_time");
    meta->set_accessed_time(bson_iter_time_t(&iter));

    return meta;

}

