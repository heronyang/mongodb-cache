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

/******************** Worker ********************/

void operation_handler(int clientfd) {

    // get len
    size_t len = read_len(clientfd);
    if(len == 0) {
        return;
    }

    // get content
    uint8_t *content = read_content(clientfd, len);
    if(content == NULL) {
        return;
    }

    Operation *operation;
    operation = operation__unpack(NULL, len, content);
    if(operation == NULL) {
        printf("Error found while unpacking operation\n");
        return;
    }

    if(operation->op == OP_GET) {
        Meta *meta = db_get(operation->cid);
        printf("Get sid = %.*s\n", SHA1_LENGTH, meta->sid);
    } else if(operation->op == OP_POST) {
        // FIXME: close clientfd before db_post
        db_post(operation->meta);
    } else {
        printf("Error: unseen operation\n");
    }

    operation__free_unpacked(operation, NULL);

}

/*
 * Entry function run by each worker
 */
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

/******************** Main ********************/

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
