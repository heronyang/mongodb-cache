#ifndef __CONFIG_H__
#define __CONFIG_H__

#define PORT                8000
#define LISTEN_BACKLOG      1024
#define MAX_CONNECTION      10

#define MONGODB_URL         "mongodb://localhost:27017?maxpoolsize=10&minpoolsize=1" 
#define MONGODB_DB          "cache"
#define MONGODB_COLLECTION  "cache"

#endif
