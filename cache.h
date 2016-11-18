#ifndef __CACHE_H__
#define __CACHE_H__

#include <stdbool.h>
#include "meta.h"

#define MONGODB_URL         "mongodb://localhost:27017" 
#define MONGODB_DB          "cache"
#define MONGODB_COLLECTION  "cache"

void db_init();
void db_deinit();

Meta *db_get(char *cid);
bool db_put(Meta *meta);

#endif
