#include <stdlib.h>
#include <string.h>
#include "meta.h"

Meta *create_meta(char *cid,
                  char *sid,
                  char *content,
                  uint32_t initial_seq,
                  time_t ttl) {

    Meta *meta = malloc(sizeof(Meta));

    // cid
    meta->cid = malloc(strlen(cid));
    strcpy(meta->cid, cid);

    // sid
    meta->sid = malloc(strlen(sid));
    strcpy(meta->sid, sid);

    // content
    meta->content = malloc(strlen(content));
    strcpy(meta->content, content);

    // initial_seq
    meta->initial_seq = initial_seq;

    // ttl
    meta->ttl = ttl;

    return meta;
}

void free_meta(Meta *meta) {

    free(meta->cid);
    free(meta->sid);
    free(meta->content);

    free(meta);

}
