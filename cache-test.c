/**
 * cache-test
 * ==========
 * Test code for cace, which does get then post operation calls.
 */

#include <stdio.h>
#include <time.h>
#include "cache.h"
#include "helper.h"

#include "proto/meta.pb-c.h"
#include "proto/operation.pb-c.h"

void db_post_test() {

    time_t now;
    time(&now);

    // Create dummy meta
    Meta meta = META__INIT;
    meta.cid = "83e9ce27e198605616ef247aa5aeb411dcac065c";
    meta.sid = "5b9f884a931a2c8f161c24739393f71895d645c1";

    ProtobufCBinaryData content; 
    content.data = malloc_w(26);
    content.len  = 26;
    meta.content = content;

    meta.initial_seq = 200;

    meta.ttl = 300;
    meta.created_time = now;
    meta.accessed_time = now;

    // Run db post operation
    print_meta(&meta);
    db_post(&meta);

}

void db_get_test() {

    // Retrive the dummy meta
    Meta *meta = db_get("83e9ce27e198605616ef247aa5aeb411dcac065c");

    // Print meta, free meta
    print_meta(meta);
    free(meta);

    return;
}

int main() {

    db_init();

    db_post_test();
    db_get_test();

    db_deinit();

    return 0;
}
