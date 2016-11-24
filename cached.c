#include <stdio.h>
#include <signal.h>

#include "config.h"
#include "wrapper.h"
#include "cache.h"

#include "proto/meta.pb-c.h"
#include "proto/operation.pb-c.h"

static volatile bool running = true;
sem_t sem;

/******************** Worker ********************/

void print_buffer(int len, uint8_t *buffer) {

    int i;

    printf("> %d,\t", len);
    for(i=0; i<len; i++) {
        if(i % 20 == 0) {
            printf("\n");
        }
        printf("[%02x]  ", buffer[i]);
    }

    printf("\n");

}

/*
 * First 4 bytes of the data indicates the len
 */
size_t read_len(int clientfd) {

    uint8_t buffer[HEADER_SIZE];
    int n_read = recv(clientfd, buffer, HEADER_SIZE, 0);

    if(n_read != HEADER_SIZE) {
        perror("Error in reading header");
        return 0;
    }

    int len = 0, i;
    for(i=0; i<HEADER_SIZE; i++) {
        len += buffer[i];
        if(i != HEADER_SIZE - 1) {
            len <<= 8;
        }
        printf("%d, %02x, len = %x\n", i, buffer[i], len);
    }

    return len;
}

/*
 * Read following content with specific len, if failed, return NULL
 */
uint8_t *read_content(int clientfd, size_t len) {

    int n_read, n_read_total = 0;
    uint8_t buffer[BUFFER_SIZE];
    uint8_t *content = malloc(len);
    uint8_t *iterator = content;

    while(true) {

        n_read = recv(clientfd, buffer, BUFFER_SIZE, 0);

        if(n_read == -1) {
            perror("Error in reading content");
            return NULL;
        } else if(n_read == 0) {    // EOF
            break;
        }

        memcpy(iterator, buffer, n_read);
        print_buffer(n_read, buffer);
        iterator += n_read;
        n_read_total += n_read;

    }

    if(n_read != 0) {
        perror("Recv failed\n");
        free(content);
        return NULL;
    }

    printf("Successfully read %d bytes for content\n", n_read_total);
    return content;

}

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
