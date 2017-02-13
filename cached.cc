/**
 * cached
 * ======
 * Cache daemon which reads operation requestions from socket
 */
#include <stdio.h>
#include <signal.h>
#include <string.h>

#include "cache.h"

#include "config.h"
#include "wrapper.h"
#include "helper.h"

#include "proto/meta.pb.h"
#include "proto/operation.pb.h"

static volatile bool running = true;
sem_t sem;

/* Garbage collector */
void *garbage_collection_worker(void *) {

    while(true) {

        sleep(GARBAGE_COLLECTION_TIME_PERIOD);
        db_cleanup();

        if(!running) {
            break;
        }

    }

    return NULL;

}

/* Operation Handlers */

void operation_get_handler(int clientfd, const char *cid);
void operation_post_handler(int clientfd, Meta meta);

void operation_handler(int clientfd) {

    // get len
    size_t len = read_len(clientfd);
    if(len == 0) {
        printf("Error while reading socket\n");
        return;
    }

    // get content
    uint8_t *content = read_content(clientfd, len);
    if(content == NULL) {
        printf("Error found while reading content\n");
        return;
    }

    // unpack operation
    std::string content_str(content, content + len);

    Operation operation;
    operation.ParseFromString(content_str);

    // run different operation based on the op field
    if(operation.op() == OP_GET) {
        const char *cid = operation.cid().c_str();
        operation_get_handler(clientfd, cid);
    } else if(operation.op() == OP_POST) {
        // FIXME: close clientfd before db_post, faster
        operation_post_handler(clientfd, operation.meta());
    } else {
        printf("Error: unseen operation\n");
    }

    // free
    free(content);

}

void operation_get_handler(int clientfd, const char *cid) {

    // get requested meta
    Meta *meta = db_get(cid);

    // not found
    if(meta == NULL) {
        printf("Requested meta not found\n");
        return;
    }

    // parse response into proper buffer
    Buffer *buffer = (Buffer *)malloc_w(sizeof(Buffer));
    buffer->len = meta->ByteSize();

    std::string data_str;
    meta->SerializeToString(&data_str);
    const char *data = data_str.c_str();
    buffer->data = (uint8_t *)data;

    // write to the client socket
    write_socket(clientfd, buffer);
    free_buffer(buffer);

}

void operation_post_handler(int clientfd, Meta meta) {
    db_post(meta);
}

/* Worker Flow */
void *worker(void *clientfd_p) {

    // parse parameters and release resources
    int clientfd = *((int *) clientfd_p);
    pthread_detach_w(pthread_self());   // no pthread_join
    free(clientfd_p);

    // run job
    sem_wait_w(&sem);
    printf("Connected\n");
    operation_handler(clientfd);
    sem_post_w(&sem);

    // close
    close(clientfd);
    return NULL;

}

/* Main Flow */

void signal_handler(int dummy) {
    running = false;
    connect_to(HOST, PORT); // to terminate accept in main loop
}

void init() {

    signal(SIGINT, signal_handler);

    sem_init_w(&sem, 0, MAX_CONNECTION);
    db_init();
}

void deinit() {

    printf("shutting down...");

    sem_destroy(&sem);
    db_deinit();
}

int main(int argc, char **argv) {

    GOOGLE_PROTOBUF_VERIFY_VERSION;

    int listenfd, *clientfd_p;
    struct sockaddr_storage client_addr;
    socklen_t client_addr_len;
    pthread_t thread;

    //
    init();

    // garbage collection worker
    if(ENABLE_GARBAGE_COLLECTION) {
        pthread_create_w(&thread, NULL, garbage_collection_worker, NULL);
    }

    // start listening
    listenfd = listen_on_w(PORT);
    printf("Listening on %d...\n", PORT);

    while(true) {

        // wait for connection
        client_addr_len = sizeof(client_addr);
        clientfd_p = (int *) malloc_w(sizeof(int));
        *clientfd_p = accept_w(listenfd,
                (SA *) &client_addr,
                &client_addr_len);

        if(!running) {
            break;
        }

        // create new create per new connection
        pthread_create_w(&thread, NULL, worker, clientfd_p);

    }

    // soft shutdown
    deinit();
    return EXIT_SUCCESS;

}
