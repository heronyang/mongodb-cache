#ifndef __CONFIG_H__
#define __CONFIG_H__

#define PORT                8000
#define LISTEN_BACKLOG      1024
#define MAX_CONNECTION      3
#define BUFFER_SIZE         256

#define OP_GET              0
#define OP_POST             1

#define MONGODB_URL         "mongodb://localhost:27017?maxpoolsize=10&minpoolsize=1" 
#define MONGODB_DB          "cache"
#define MONGODB_COLLECTION  "cache"

#define SHA1_LENGTH         40  // SHA1: 160 bits = 40 hex char

#endif
