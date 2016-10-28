#ifndef __META_H__
#define __META_H__

#include <time.h>
#include <stdint.h>

typedef struct _Meta {

    char *cid;
    char *sid;

    char *content;

    uint64_t len;
    uint32_t initial_seq;

    time_t ttl;

    time_t created;
    time_t accessed;

} Meta;

Meta *create_meta(char *cid,
                  char *sid,
                  char *content,
                  uint32_t initial_seq,
                  time_t ttl);

void free_meta(Meta *meta);

#endif
