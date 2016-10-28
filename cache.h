#ifndef __CACHE_H__
#define __CACHE_H__

#include "meta.h"

Meta *get(char *cid);
int put(Meta *meta);

#endif
