#ifndef __BCON_WRAPPER_H__
#define __BCON_WRAPPER_H__

#include <stdbool.h>
#include <bson.h>
#include <mongoc.h>
#include <stdbool.h>

#include "proto/meta.pb-c.h"
#include "wrapper.h"

// bson parsers
char *get_cid_from_doc(const bson_t *doc);
uint32_t get_ttl_from_doc(const bson_t *doc);
time_t get_created_time_from_doc(const bson_t *doc);
time_t get_accessed_time_from_doc(const bson_t *doc);

// meta / bson
void meta2bson(bson_t *doc, Meta *meta);
Meta *bson2meta(const bson_t *doc, const char *cid);

#endif
