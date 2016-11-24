#ifndef __CACHE_H__
#define __CACHE_H__

#include <stdbool.h>
#include <bson.h>
#include <mongoc.h>
#include <stdbool.h>

#include "proto/meta.pb-c.h"
#include "proto/operation.pb-c.h"

#include "config.h"
#include "wrapper.h"

void db_init();
void db_deinit();

Meta *db_get(const char *cid);
bool db_post(Meta *meta);

#endif
