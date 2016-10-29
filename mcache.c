#include <stdio.h>
#include "meta.h"
#include "cache.h"

int main(int argc, char *argv[]) {

    // init
    init();

    // create a new meta
    // NOTE: cid length should >= 24
    char cid[]              = "123456789012345678901234";
    char sid[]              = "432143214321423143214321";
    uint8_t content[]       = "\xaa\xbb\xcc\xdd";
    uint64_t len            = 4;
    uint32_t initial_seq    = 15;
    time_t ttl              = 0;

    Meta *meta = create_meta(cid, sid, content, len, initial_seq, ttl);

    // put into the cache
    if(put(meta) < 0) {
        printf("Error found in putting meta into the cache.\n");
    }

    // get it out of the cache
    Meta *meta_retrieved = get(cid);
    if(meta_retrieved == NULL) {
        printf("Can't find the meta while retrieving.\n");
    } else {
        printf("Found meta with sid: %s\n", meta_retrieved->sid);
    }

    // deinit
    deinit();

    return 0;

}
