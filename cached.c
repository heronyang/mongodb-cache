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

#include "proto/meta.pb-c.h"
#include "proto/operation.pb-c.h"

static volatile bool running = true;
sem_t sem;

/* Garbage collector */
void *garbage_collection_worker() {

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

void operation_get_handler(int clientfd, char *cid);
void operation_post_handler(int clientfd, Meta *meta);

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
    Operation *operation;
    operation = operation__unpack(NULL, len, content);

    if(operation == NULL) {
        free(content);
        printf("Error found while unpacking operation\n");
        return;
    }

    // run different operation based on the op field
    if(operation->op == OP_GET) {
        operation_get_handler(clientfd, operation->cid);
    } else if(operation->op == OP_POST) {
        // FIXME: close clientfd before db_post, faster
        operation_post_handler(clientfd, operation->meta);
    } else {
        printf("Error: unseen operation\n");
    }

    // free
    operation__free_unpacked(operation, NULL);
    free(content);

}

void operation_get_handler(int clientfd, char *cid) {

    // get requested meta
    Meta *meta = db_get(cid);

    // parse response into proper buffer
    Buffer *buffer = malloc_w(sizeof(Buffer));
    buffer->len = (size_t) meta__get_packed_size(meta);
    buffer->data = malloc_w(buffer->len);
    meta__pack(meta, buffer->data);

    // write to the client socket
    write_socket(clientfd, buffer);
    free_buffer(buffer);

}

void operation_post_handler(int clientfd, Meta *meta) {
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
        clientfd_p = malloc_w(sizeof(int));
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
