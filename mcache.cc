/**
 * mcache
 * ======
 * Cached user (main file), which does one post the one get operation
 */
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/time.h>

#include "wrapper.h"
#include "config.h"
#include "helper.h"

#include "proto/meta.pb.h"
#include "proto/operation.pb.h"

/*
 * Create dummy serialized POST operation structure
 */
Buffer *generate_post_operation_serialized() {

    Buffer *buffer = (Buffer *)malloc_w(sizeof(Buffer));

    // Meta
    Meta meta;
    meta.set_cid("c3e9ce27e198605616ef547a");
    meta.set_sid("5a9f884a931a2c8f161c2473");

    meta.set_content((const char *)malloc_w(26));

    meta.set_initial_seq(3456);
    meta.set_ttl(30);

    // now
    struct timeval now;
    gettimeofday(&now, NULL);
    long int now_ms = now.tv_sec * 1000 + now.tv_usec / 1000;

    meta.set_created_time(now_ms);
    meta.set_accessed_time(now_ms);

    print_meta(meta);

    // Operation
    Operation *operation = new Operation();
    operation->set_op(OP_POST);
    operation->set_cid(meta.cid());
    operation->set_allocated_meta(&meta);

    // Serialize
    buffer->len = operation->ByteSize();
    std::string data_str;
    operation->SerializeToString(&data_str);

    buffer->data = (uint8_t *)malloc_w(buffer->len);
    memcpy(buffer->data, data_str.c_str(), buffer->len);

    printf("Get %zu serialized bytes (post operation)\n", buffer->len);

    return buffer;

}

Buffer *generate_get_operation_serialized() {

    Buffer *buffer = (Buffer *)malloc_w(sizeof(Buffer));

    // Operation
    Operation *operation = new Operation();
    operation->set_op(OP_GET);
    operation->set_cid("c3e9ce27e198605616ef547a");

    // Serialize
    buffer->len = operation->ByteSize();
    std::string data_str;
    operation->SerializeToString(&data_str);

    buffer->data = (uint8_t *)malloc_w(buffer->len);
    memcpy(buffer->data, data_str.c_str(), buffer->len);

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

    free_buffer(buffer);

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
        // not found
        printf("meta not found\n");
        free_buffer(buffer);
        return;
    }

    // get content
    uint8_t *content = read_content(sockfd, len);
    close(sockfd);

    if(content == NULL) {
        // not found
        printf("Error in reading content\n");
        free_buffer(buffer);
        return;
    }

    std::string content_str(content, content + len);
    Meta meta;
    meta.ParseFromString(content_str);
    print_meta(meta);

    // free
    free_buffer(buffer);

}

int main(int argc, char *argv[]) {

    // post operation
    post_operation();

    // post doesn't wait for response, so we freeze a while for the chunk to
    // be inserted
    usleep(1000);

    // get operation
    get_operation();

    return 0;

}
