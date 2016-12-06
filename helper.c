/**
 * helper
 * ======
 * Helper functions which can be applied by different components
 */
#include "helper.h"

/* Log & Debug */

void print_meta(Meta *meta) {

    int i;

    printf("[Meta]\n");
    printf("\tcid = %.*s\n", SHA1_LENGTH, meta->cid);
    printf("\tsid = %.*s\n", SHA1_LENGTH, meta->sid);
    printf("\tcontent = %.*s (%zu)\n", (int)meta->content.len, meta->content.data, meta->content.len);
    printf("\t");
    for(i=0;i<meta->content.len;i++) {
        printf("[%02x] ", meta->content.data[i]);
    }
    printf("\n");
    printf("\tttl = %u, initial_seq = %u\n", meta->ttl, meta->initial_seq);

}

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

/* Socket */

/*
 * Generate simple packet with header size = 4 (contains body size)
 */
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

/*
 * First 4 bytes of the data indicates the len
 */
size_t read_len(int clientfd) {

    uint8_t buffer[HEADER_SIZE];
    int n_read = recv(clientfd, buffer, HEADER_SIZE, 0);

    if(n_read != HEADER_SIZE) {
        printf("n_read = %d\n", n_read);
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
            printf("Error in reading content\n");
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

    printf("Read %d bytes for content\n", n_read_total);
    return content;

}

/*
 * Write buffer to socket
 */
void write_socket(int sockfd, Buffer *buffer) {

    // packet
    uint8_t *packet = malloc_w(HEADER_SIZE + buffer->len);
    packet = generate_packet(buffer);

    // write
    int n = write_w(sockfd, packet, HEADER_SIZE + buffer->len);

    printf("Write %d bytes succeed\n", n);

}

/*
 * Free buffer
 */
void free_buffer(Buffer *buffer) {
    if(buffer == NULL) {
        return;
    }
    if(buffer->data != NULL) {
        free(buffer->data);
    }
    free(buffer);
}

void free_meta(Meta *meta) {
    if(meta == NULL) {
        return;
    }
    if(meta->cid != NULL) {
        free(meta->cid);
    }
    if(meta->sid != NULL) {
        free(meta->sid);
    }
    if(meta->content.len != 0) {
        free(meta->content.data);
    }
    free(meta);
}
