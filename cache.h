#ifndef __CACHE_H__
#define __CACHE_H__

#include "meta.h"

#define MONGODB_URL         "mongodb://localhost:27017" 
#define MONGODB_DB          "cache"
#define MONGODB_COLLECTION  "cache"

void init();
void deinit();

Meta *get(char *cid);
int put(Meta *meta);

#endif
