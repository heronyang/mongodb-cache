#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include "wrapper.h"
#include "config.h"
#include "helper.h"
#include "proto/meta.pb-c.h"
#include "proto/operation.pb-c.h"

/*
 * Create dummy serialized POST operation structure
 */
Buffer *generate_post_operation_serialized() {

    Buffer *buffer = malloc_w(sizeof(Buffer));

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

    // Serialize
    buffer->len = (size_t) operation__get_packed_size(&operation);
    buffer->data = malloc_w(buffer->len);
    operation__pack(&operation, buffer->data);

    printf("Get %zu serialized bytes (post operation)\n", buffer->len);

    return buffer;

}

Buffer *generate_get_operation_serialized() {

    Buffer *buffer = malloc_w(sizeof(Buffer));

    // Operation
    Operation operation = OPERATION__INIT;
    operation.op = OP_GET;
    operation.cid = "83e9ce27e198605616ef247aa5aeb411dcac065c";

    // Serialize
    buffer->len = (size_t) operation__get_packed_size(&operation);
    buffer->data = malloc_w(buffer->len);
    operation__pack(&operation, buffer->data);

    printf("Get %zu serialized bytes (get operation)\n", buffer->len);

    return buffer;
}

void post_operation() {

    Buffer *buffer = generate_post_operation_serialized();

    // socket
    int sockfd = connect_to(HOST, PORT);

    // send request, don't read response
    write_socket(sockfd, buffer);

    close(sockfd);
    free(buffer);

}

void get_operation() {

    Buffer *buffer = generate_get_operation_serialized();

    // socket
    int sockfd = connect_to(HOST, PORT);

    // send request
    write_socket(sockfd, buffer);

    // read response
    // get len
    size_t len = read_len(sockfd);
    if(len == 0) {
        free_buffer(buffer);
        return;
    }

    // get content
    uint8_t *content = read_content(sockfd, len);
    if(content == NULL) {
        free_buffer(buffer);
        return;
    }

    Meta *meta = meta__unpack(NULL, len, content);
    printf("sid = %.*s\n", SHA1_LENGTH, meta->sid);

    // free
    free_buffer(buffer);

}

int main(int argc, char *argv[]) {

    // post operation
    post_operation();

    // get operation
    get_operation();

    return 0;

}
