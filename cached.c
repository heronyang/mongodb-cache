#include <stdio.h>
#include <signal.h>

#include "config.h"
#include "wrapper.h"
#include "meta.h"
#include "cache.h"

static volatile bool running = true;
sem_t sem;

/******************** Worker ********************/

/*
 * Dummy, print current time
 * Reference: http://stackoverflow.com/questions/5141960/get-the-current-time-in-c
 */
void print_current_time() {

    time_t rawtime;
    struct tm * timeinfo;

    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    printf ( "Current local time and date: %s", asctime (timeinfo) );

}

/*
 * Dummy, hold time
 */
void limited_worker_can_come_in() {
    sleep(3);
    print_current_time();
}

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
    if(db_put(meta) < 0) {
        printf("Error found in putting meta into the cache.\n");
    }

    printf("successfully inserted one data\n");

    // release from heap
    free_meta(meta);

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
    put_one_dummy_cache_record();
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
