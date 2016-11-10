#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>
#include <string.h>
#include "meta.h"
#include "cache.h"

typedef struct {
    int put_amount;
    int thread_amount;
} Config;

static void usage(char *argv[]) {
	printf("Usage: %s [OPTIONS]\n", argv[0]);
	printf("  -p, --put                 Amount of put operations per thread worker.\n");
	printf("  -t, --thread              Number of threads.\n");
	printf("  -h, --help                Displays this help.\n");
};

bool parseArgv(int argc, char *argv[], Config *config) {

    int c;

	struct option options[] = {
		{"put",     required_argument, 0, 'p'},
		{"thread",  required_argument, 0, 't'},
		{"help",    no_argument, 0, 'h'},
		{0, 0, 0, 0},
	};

    if(argc <= 1) {
        usage(argv);
        return false;
    }

	while(1) {

		int option_index = 0;

		c = getopt_long(argc, argv, "p:t:h", options, &option_index);
		if(c == -1) {
			break;
        }

        switch(c) {

            case 0:

                if(!strcmp(options[option_index].name, "put")) {
                    config->put_amount = atoi(optarg);
                }

                if(!strcmp(options[option_index].name, "thread")) {
                    config->thread_amount = atoi(optarg);
                }

                break;

            case 'p':
                config->put_amount = atoi(optarg);
                break;

            case 't':
                config->thread_amount = atoi(optarg);
                break;

            case 'h':
            default:
                usage(argv);
                return false;

        }

	}

    return true;

}


int main(int argc, char *argv[]) {

    Config *config = calloc(sizeof(Config), 0);

    if(!parseArgv(argc, argv, config)) {
        return 1;
    }

    printf("Setup: thread = %d, put = %d\n", config->thread_amount, config->put_amount);

    return 0;

}
