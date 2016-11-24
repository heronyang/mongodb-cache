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
        printf("n_read = %d\n", n_read);
        printf("Error in reading header\n");
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
    size_t left = len;

    while(true) {

        n_read = recv(clientfd, buffer, left, 0);

        if(n_read == -1) {
            perror("Error in reading content");
            return NULL;
        } else if(n_read == 0) {    // EOF
            break;
        }

        memcpy(iterator, buffer, n_read);
        iterator += n_read;
        n_read_total += n_read;
        left -= n_read;

        if(left == 0) {
            break;
        }

    }

    print_buffer(n_read_total, buffer);

    printf("Successfully read %d bytes for content\n", n_read_total);
    return content;

}

uint8_t *generate_packet(Buffer *buffer) {

    uint8_t *packet = malloc_w(HEADER_SIZE + buffer->len);

    // header (len)
    packet[0] = (buffer->len >> 24) & 0xff;
    packet[1] = (buffer->len >> 16) & 0xff;
    packet[2] = (buffer->len >> 8) & 0xff;
    packet[3] = (buffer->len >> 0) & 0xff;

    // content
    memcpy(packet + HEADER_SIZE, buffer->data, buffer->len);

    return packet;

}

void write_socket(int sockfd, Buffer *buffer) {

    // packet
    uint8_t *packet = malloc_w(HEADER_SIZE + buffer->len);
    packet = generate_packet(buffer);

    // write
    int n = write_w(sockfd, packet, HEADER_SIZE + buffer->len);

    printf("Write %d bytes succeed\n", n);

}

void free_buffer(Buffer *buffer) {
    free(buffer->data);
    free(buffer);
}
