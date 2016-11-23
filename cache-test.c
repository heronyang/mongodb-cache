#include <stdio.h>
#include <time.h>
#include "cache.c"

void db_post_test() {

    time_t now;
    time(&now);

    // Meta
    Meta meta = META__INIT;
    meta.cid = "83e9ce27e198605616ef247aa5aeb411dcac065c";
    meta.sid = "5b9f884a931a2c8f161c24739393f71895d645c1";

    ProtobufCBinaryData content; 
    content.data = malloc_w(26);
    content.len  = 26;
    meta.content = content;

    meta.initial_seq = 200;

    meta.created_time = now;
    meta.accessed_time = now;

    db_post(meta);

}

int main() {

    db_init();

    // db_post();
    // db_get();

    db_deinit();

    return 0;
}
