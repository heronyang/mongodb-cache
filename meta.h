#ifndef __META_H__
#define __META_H__

#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "wrapper.h"

typedef struct _Meta {

    char *cid;
    char *sid;

    uint8_t *content;

    uint32_t len;
    uint32_t initial_seq;

    time_t ttl;

    time_t created;
    time_t accessed;

} Meta;

typedef struct _Meta_Encoded {
    unsigned len;
    char *raw_data;
} Meta_Encoded;

Meta *create_meta(char *cid,
                  char *sid,
                  uint8_t *content,
                  uint32_t len,
                  uint32_t initial_seq,
                  time_t ttl);
void free_meta(Meta *meta);

Meta_Encoded *encode_meta(Meta *meta);
Meta *decode_meta(Meta_Encoded *meta_e);

void print_meta(Meta *meta);

#endif
