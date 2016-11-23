#include <stdio.h>
#include "wrapper.h"
#include "config.h"
#include "proto/meta.pb-c.h"
#include "proto/operation.pb-c.h"

/*
 * Create dummy serialized POST operation structure
 */
void *generate_post_operation_serialized() {

    void *buf;
    unsigned len;

    // Meta
    Meta meta = META__INIT;
    meta.cid = "83e9ce27e198605616ef247aa5aeb411dcac065c";
    meta.sid = "5b9f884a931a2c8f161c24739393f71895d645c1";

    ProtobufCBinaryData content; 
    content.data = malloc_w(26);
    content.len  = 26;
    meta.content = content;

    meta.initial_seq = 200;

    // Operation
    Operation operation = OPERATION__INIT;
    operation.op = OP_POST;
    operation.cid = meta.cid;
    operation.meta = &meta;

    // serialize
    len = operation__get_packed_size(&operation);
    buf = malloc_w(len);
    operation__pack(&operation, buf);

    printf("Get %d serialized bytes\n", len);

    return buf;

}

void post_operation() {

    void *buf = generate_post_operation_serialized();
    free(buf);

}

void get_operation() {
}

int main(int argc, char *argv[]) {

    // post operation
    post_operation();

    // get operation
    get_operation();

    return 0;

}
