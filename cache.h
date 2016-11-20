#ifndef __CACHE_H__
#define __CACHE_H__

#include <stdbool.h>
#include <bson.h>
#include <bcon.h>
#include <mongoc.h>
#include <stdbool.h>

#include "meta.h"
#include "config.h"
#include "wrapper.h"

void db_init();
void db_deinit();

Meta *db_get(char *cid);
bool db_post(Meta *meta);

#endif
