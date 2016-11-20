#ifndef __OPERATION_H__
#define __OPERATION_H__

#include <string.h>
#include "meta.h"
#include "wrapper.h"
#include "config.h"

typedef struct _Operation {
    char op;
    char *cid;
    Meta *meta;
} Operation;

typedef struct _Operation_Encoded {
    /*
     * raw_data contains:
     *  --------------------
     *  | op  (1)          |
     *  --------------------
     *  | cid (CID_LENGTH) |
     *  --------------------
     *  | meta_e (-)       |
     *  --------------------
     */
    unsigned len;
    char *raw_data;
} Operation_Encoded;


Operation_Encoded *encode_operation(Operation *operation);
Operation *decode_operation(Operation_Encoded *operation_e);

void free_operation(Operation *operation);

#endif
