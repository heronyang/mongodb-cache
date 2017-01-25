/**
 * cache-test
 * ==========
 * Test code for cace, which does get then post operation calls.
 */

#include <stdio.h>
#include <time.h>
#include "cache.h"
#include "helper.h"

#include "proto/meta.pb.h"
#include "proto/operation.pb.h"

void db_post_test() {

    time_t now;
    time(&now);

    // Create dummy meta
    Meta meta;
    meta.set_cid("93e9ce27e198605616ef247aa5aeb411dcac065c");
    meta.set_sid("5b9f884a931a2c8f161c24739393f71895d645c1");
        
    meta.set_content((const char *)malloc_w(26));

    meta.set_initial_seq(200);

    meta.set_ttl(300);
    meta.set_created_time(now);
    meta.set_accessed_time(now);

    // Run db post operation
    print_meta(meta);
    db_post(meta);

}

void db_get_test() {

    // Retrive the dummy meta
    Meta *meta = db_get("93e9ce27e198605616ef247aa5aeb411dcac065b");
    if(meta == NULL) {
        printf("Meta not found\n");
    } else {
        print_meta(*meta);
        delete meta;
    }

    return;

}

int main() {

    GOOGLE_PROTOBUF_VERIFY_VERSION;

    db_init();

    db_post_test();
    db_get_test();

    db_deinit();

    google::protobuf::ShutdownProtobufLibrary();

    return 0;
}
