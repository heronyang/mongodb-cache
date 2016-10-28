#include <stdio.h>
#include "meta.h"
#include "cache.h"

int main(int argc, char *argv[]) {

    // create a new meta
    Meta *meta = create_meta("cidcidcid", "sidsidsid", "contentishere", 1, 15);

    // put into the cache
    if(put(meta) < 0) {
        printf("Error found in putting meta into the cache.\n");
    }

    // get it out of the cache
    Meta *meta_retrieved = get("cidcidcid");
    if(meta_retrieved == NULL) {
        printf("Can't find the meta while retrieving.\n");
    } else {
        printf("Found meta with content: %s\n", meta->content);
    }

    return 0;
}
