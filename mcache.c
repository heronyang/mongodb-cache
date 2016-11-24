#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include "wrapper.h"
#include "config.h"
#include "proto/meta.pb-c.h"
#include "proto/operation.pb-c.h"

typedef struct _Buffer {
    size_t len;
    uint8_t *data;
} Buffer;

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

uint8_t *generate_packet(Buffer *buffer) {

    uint8_t *packet = malloc_w(HEADER_SIZE + buffer->len);

    // header (len)
    packet[0] = (buffer->len >> 24) & 0xff;
    packet[1] = (buffer->len >> 16) & 0xff;
    packet[2] = (buffer->len >> 8) & 0xff;
    packet[3] = (buffer->len >> 0) & 0xff;

    // content
    memcpy(packet + HEADER_SIZE, buffer->data, buffer->len);

    return packet;

}

void write_socket(Buffer *buffer) {

    // packet
    uint8_t *packet = malloc_w(HEADER_SIZE + buffer->len);
    packet = generate_packet(buffer);

    // socket
    printf("Connecting to %s:%d...\n", HOST, PORT);
    int sockfd = connect_to(HOST, PORT);

    // write
    int n;
    n = write_w(sockfd, packet, HEADER_SIZE + buffer->len);

    printf("Write %d bytes succeed\n", n);

}

uint8_t *read_socket() {
    return NULL;
}

void free_buffer(Buffer *buffer) {
    free(buffer->data);
    free(buffer);
}

void post_operation() {

    Buffer *buffer = generate_post_operation_serialized();
    write_socket(buffer);
    free(buffer);

}

void get_operation() {
    Buffer *buffer = generate_get_operation_serialized();
    write_socket(buffer);
    uint8_t *data = read_socket();
    // Meta *meta = meta__unpack(NULL, len, content);
    free(buffer);
}

int main(int argc, char *argv[]) {

    // post operation
    post_operation();

    // get operation
    get_operation();

    return 0;

}
