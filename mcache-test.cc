#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>

#include "helper.h"
#include "wrapper.h"
#include "config.h"

#include "proto/meta.pb.h"
#include "proto/operation.pb.h"

#define DEFAULT_PUT_AMOUNT      20
#define DEFAULT_CHUNCK_SIZE     (1024 * 1024)
#define DEFAULT_THREAD          2
#define DEFAULT_TTL             600

/* Configuration */

typedef struct {
    int put_amount;
    int thread_amount;
    int chunk_size;
} Config;

typedef struct {
    int chunk_size;
    int put_amount_per_thread;
    int tid;
} Worker_Args;

/* Function Prototypes */

void generate_random_string(char *s, int len);
void print_succeed_failed_count();
void print_time_duration(int begin_time, int end_time);
void print_config(Config *config);

int put_succeed = 0, put_failed = 0;    // should use array if multi-thread
int *put_succeed_on_thread;
int *put_failed_on_thread;

static void usage(char *argv[]) {
	printf("Usage: %s [OPTIONS]\n", argv[0]);
	printf("  -p            Amount of put operations in total.\n");
	printf("  -t            Number of threads.\n");
	printf("  -s            Chunk size.\n");
	printf("  -h            Displays this help.\n");
};

bool parseArgv(int argc, char *argv[], Config *config) {

    int c;

	struct option options[] = {
		{"put",         required_argument, 0, 'p'},
		{"thread",      required_argument, 0, 't'},
        {"chunk_size",  required_argument, 0, 's'},
		{"help",        no_argument, 0, 'h'},
		{0, 0, 0, 0},
	};

	while(1) {

		int option_index = 0;

		c = getopt_long(argc, argv, "p:t:s:h", options, &option_index);
		if(c == -1) {
			break;
        }

        switch(c) {
            case 'p':
                config->put_amount = atoi(optarg);
                break;

            case 't':
                config->thread_amount = atoi(optarg);
                break;

            case 's':
                config->chunk_size = atoi(optarg);
                break;

            case 'h':
            default:
                usage(argv);
                return false;
        }

	}

    return true;

}

/* Generate test meta */

void fill_default_config(Config *config) {

    config->put_amount = DEFAULT_PUT_AMOUNT;
    config->thread_amount = DEFAULT_THREAD;
    config->chunk_size = DEFAULT_CHUNCK_SIZE;

}

uint32_t generate_random_number() {
    return (uint32_t) rand();
}

Meta generate_dummy_meta(uint64_t chunk_size) {

    Meta meta;

    // get random id
    char *random_str = (char *) malloc_w(SHA1_LENGTH);
    generate_random_string(random_str, SHA1_LENGTH);

    // cid
    meta.set_cid(random_str);

    // sid
    meta.set_sid(random_str);

    // content
    meta.set_content((const char *) malloc_w(chunk_size));

    // misc
    meta.set_initial_seq(generate_random_number());
    meta.set_ttl(DEFAULT_TTL);

    // created, accessed time
    struct timeval now;
    gettimeofday(&now, NULL);
    long int now_ms = now.tv_sec * 1000 + now.tv_usec / 1000;

    meta.set_created_time(now_ms);
    meta.set_accessed_time(now_ms);

    // free
    free(random_str);

    return meta;

}

Buffer *generate_post_operation(Meta meta) {

    Buffer *buffer = (Buffer *) malloc_w(sizeof(Buffer));

    // Operation
    Operation operation;
    operation.set_op(OP_POST);
    operation.set_cid(meta.cid());
    operation.set_allocated_meta(&meta);

    // Serialize
    buffer->len = operation.ByteSize();
    std::string data_str;
    operation.SerializeToString(&data_str);
    const char *data = data_str.c_str();
    buffer->data = (uint8_t *)data;

    return buffer;

}

void generate_random_string(char *s, int len) {

    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    int i;
    for(i = 0; i < len; i++) {
        s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    s[len] = 0;

}

void put_one_meta(int chunk_size, int tid) {

    Meta meta = generate_dummy_meta((uint64_t)chunk_size);
    Buffer *buffer = generate_post_operation(meta);

    // socket
    int sockfd = connect_to(HOST, PORT);

    // send request, don't read response
    write_socket(sockfd, buffer);
    put_succeed_on_thread[tid] ++;

    free_buffer(buffer);

}

void put_n_meta(int chunk_size, int n, int tid) {
    while(n--) {
        put_one_meta(chunk_size, tid);
    }
}

void print_config(Config *config) {
    printf("[Config]\n");
    printf("\tPut operations: %d (total)\n", config->put_amount);
    printf("\tThread number: %d\n", config->thread_amount);
    printf("\tChunk size: %d (bytes)\n", config->chunk_size);
}

void print_time_duration(int begin_time, int end_time) {
    double time_spent = (double)(end_time - begin_time) / CLOCKS_PER_SEC;
    printf("Time spent: %lf seconds\n", time_spent);
}

void print_succeed_failed_count(int thread_amount) {
    int i;
    for(i=0; i<thread_amount; i++) {
        printf("[Thread %d] Succeed: %d, Failed: %d\n", i, put_succeed_on_thread[i], put_failed_on_thread[i]);
    }
}

void *run_worker(void *worker_args) {

    Worker_Args *wa = (Worker_Args *)worker_args;
    put_n_meta(wa->chunk_size, wa->put_amount_per_thread, wa->tid);

    free(wa);
    return NULL;

}

void run_on_threads(int chunk_size, int put_amount, int thread_amount) {

    // result allocation for threads
    put_succeed_on_thread = (int *) calloc(thread_amount, sizeof(int));
    put_failed_on_thread = (int *) calloc(thread_amount, sizeof(int));

    // thread id pool
    pthread_t *tids = (pthread_t *) malloc(thread_amount * sizeof(pthread_t));

    int i;
    for(i=0; i<thread_amount; i++) {

        // work arguments
        Worker_Args *wa = (Worker_Args *) malloc(sizeof(Worker_Args));
        wa->chunk_size = chunk_size;
        wa->put_amount_per_thread = put_amount / thread_amount;
        wa->tid = i;

        pthread_create(&tids[i], NULL, run_worker, (void *)wa);

    }

    for(i=0; i<thread_amount; i++) {
        pthread_join(tids[i], NULL);
        printf("Thread %d: Done\n", i);
    }

    free(tids);

}

int main(int argc, char *argv[]) {

    // setup configurations
    Config *config = (Config *) calloc(sizeof(Config), 1);
    fill_default_config(config);
    if(!parseArgv(argc, argv, config)) {
        return 1;
    }
    print_config(config);

    //
    srand(time(NULL));
    setbuf(stdout, NULL);

    clock_t begin_time = clock();
    run_on_threads(config->chunk_size,
                   config->put_amount,
                   config->thread_amount);

    // end
    clock_t end_time = clock();

    //
    print_time_duration(begin_time, end_time);
    print_succeed_failed_count(config->thread_amount);

    // release
    free(put_succeed_on_thread);
    free(put_failed_on_thread);
    free(config);

    return 0;

}
