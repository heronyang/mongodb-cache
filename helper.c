#include "helper.h"

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
