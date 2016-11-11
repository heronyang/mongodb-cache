#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "meta.h"

#define DEBUG 0

Meta *create_meta(char *cid,
                  char *sid,
                  uint8_t *content,
                  uint32_t len,
                  uint32_t initial_seq,
                  time_t ttl) {

    time_t now = time(NULL);
    Meta *meta = malloc(sizeof(Meta));

    // cid
    meta->cid = malloc(strlen(cid) + 1);
    strcpy(meta->cid, cid);

    // sid
    meta->sid = malloc(strlen(sid) + 1);
    strcpy(meta->sid, sid);

    // content
    meta->content = malloc(len);
    memcpy(meta->content, content, len);

    // len
    meta->len = len;

    // initial_seq
    meta->initial_seq = initial_seq;

    // ttl
    meta->ttl = ttl;

    if(DEBUG) {
        print_meta(meta);
    }

    // created
    meta->created = now;

    // accessed
    meta->accessed = now;

    return meta;
}

void free_meta(Meta *meta) {

    if(meta == NULL) {
        return;
    }

    free(meta->cid);
    free(meta->sid);
    free(meta->content);

    free(meta);

}

void print_meta(Meta *meta) {
    printf("[Meta] sid: %s\tcid: %s\tlen: %u\n",
            meta->sid, meta->cid, meta->len);
}
