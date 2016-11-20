#include <stdio.h>
#include <signal.h>

#include "config.h"
#include "wrapper.h"
#include "meta.h"
#include "cache.h"

static volatile bool running = true;
sem_t sem;

void request_get(int connfd);
void remove_tailing_newline(char *str);
void write_to_conn(int connfd, char *content, int size);
void request_post(int connfd);
void request_failed(int connfd, char *message);

/******************** Worker ********************/

// TODO: remove this one and use real operations
// FIXME: use at least 24 byte hex string as cid (> 24 * 4 bits)
void put_one_dummy_cache_record() {

    char cid[]              = "123456789012345678901234";
    char sid[]              = "432143214321423143214321";
    uint8_t content[]       = "\xaa\xbb\xcc\xdd";
    uint64_t len            = 4;
    uint32_t initial_seq    = 15;
    time_t ttl              = 0;

    Meta *meta = create_meta(cid, sid, content, len, initial_seq, ttl);

    // put into the cache
    if(db_post(meta) < 0) {
        printf("Error found in putting meta into the cache.\n");
    }

    printf("successfully inserted one data\n");

    // release from heap
    free_meta(meta);

}

void operation_handler(int connfd) {

    int n;
    char operation_code[1], op;
    bzero(operation_code, 1);

    n = read(connfd, operation_code, 1);
    if(n < 0) {
        printf("Error found in reading from socket\n");
        request_failed(connfd, "can\'t read from socket");
        return;
    }

    op = operation_code[0];
    if(op == OP_GET) {
        request_get(connfd);
    } else if(op == OP_POST) {
        request_post(connfd);
    } else {
        request_failed(connfd, "invalid operation code");
    }

}

void request_get(int connfd) {

    int n;
    char buffer[BUFFER_SIZE];
    char *cid;
    bzero(buffer, BUFFER_SIZE);

    n = read(connfd, buffer, BUFFER_SIZE);
    if(n < 0) {
        printf("Error found in reading from socket\n");
        request_failed(connfd, "can\'t read from socket");
        return;
    }

    remove_tailing_newline(buffer);
    cid = malloc_w(strlen(buffer));
    strncpy(cid, buffer, strlen(buffer));

    Meta *meta = db_get(cid);
    if(meta == NULL) {
        printf("Error meta not found\n");
        request_failed(connfd, "Error: can\'t find meta\n");
        return;
    }

    write_to_conn(connfd, (char *)meta->content, sizeof(meta->content));

}

void remove_tailing_newline(char *str) {
    size_t len = strlen(str) - 1;
    while(len >= 0 && (str[len] == '\n' || str[len] == '\r')) {
        str[len] = '\0';
        len --;
    }
}

void write_to_conn(int connfd, char *content, int size) {

    int n, i=0, to_send_size;

    while(true) {

        to_send_size = (size > BUFFER_SIZE) ? BUFFER_SIZE : size;

        n = write(connfd, content + i * BUFFER_SIZE, to_send_size);
        if(n < 0) {
            printf("Error found in writing to socket\n");
        }

        size -= to_send_size;
        if(size <= 0) {
            break;
        }

        i++;

    }

}

void request_post(int connfd) {
}

void request_failed(int connfd, char *message) {

    char buffer[BUFFER_SIZE];
    bzero(buffer, BUFFER_SIZE);
    sprintf(buffer, "Error: %s\n", message);

    write_to_conn(connfd, buffer, BUFFER_SIZE);

}

/*
 * Entry function run by each worker
 */
void *worker(void *connfd_p) {

    // parse parameters and release resources
    int connfd = *((int *) connfd_p);
    pthread_detach_w(pthread_self());   // no pthread_join
    free(connfd_p);

    // run job
    sem_wait_w(&sem);
    operation_handler(connfd);
    sem_post_w(&sem);

    // close
    close(connfd);
    return NULL;
}

/******************** Main ********************/

void signal_handler(int dummy) {
    running = false;
    printf("Will shutdown after next connection\n");
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

    int listenfd, *connfd_p;
    struct sockaddr_storage client_addr;
    socklen_t client_addr_len;
    pthread_t thread;

    //
    init();

    // start listening
    listenfd = listen_on_w(PORT);
    printf("Listening on %d...\n", PORT);

    while(running) {

        // wait for connection
        client_addr_len = sizeof(client_addr);
        connfd_p = malloc_w(sizeof(int));
        *connfd_p = accept_w(listenfd,
                (SA *) &client_addr,
                &client_addr_len);

        // create new create per new connection
        pthread_create_w(&thread, NULL, worker, connfd_p);

    }

    // soft shutdown
    deinit();
    return EXIT_SUCCESS;

}
