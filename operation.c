#include "operation.h"

Operation_Encoded *encode_operation(Operation *operation) {

    Operation_Encoded *operation_e = malloc_w(sizeof(Operation_Encoded));

    unsigned len;
    char *raw_data;
    Meta_Encoded *meta_e;

    // prepare
    meta_e = encode_meta(operation->meta);
    len = 1 + CID_LENGTH + meta_e->len;
    raw_data = malloc_w(len);

    // put data on corresponding location
    raw_data[0] = operation->op;
    memcpy(operation + 1, operation->meta->cid, CID_LENGTH);
    memcpy(operation + 1 + CID_LENGTH, meta_e->raw_data, meta_e->len);

    // return
    operation_e->raw_data = raw_data;
    operation_e->len = len;

    return operation_e;

}

Operation *decode_operation(Operation_Encoded *operation_e) {

    Operation *operation = malloc_w(sizeof(Operation));

    // op 
    operation->op = operation_e->raw_data[0];

    // cid
    operation->cid = malloc_w(CID_LENGTH);
    memcpy(operation->cid, operation_e->raw_data + 1, CID_LENGTH);

    // meta_e
    Meta_Encoded *meta_e;
    unsigned meta_e_len = (operation_e->len) - 1 - CID_LENGTH;

    if(meta_e_len > 0) {
        meta_e = malloc_w(sizeof(Meta_Encoded));
        meta_e->len = meta_e_len;
        meta_e->raw_data = malloc(meta_e_len);
        operation->meta = decode_meta(meta_e);
    } else {
        operation->meta = NULL;
    }

    return operation;

}

void free_operation(Operation *operation) {

    if(operation == NULL) {
        return;
    }

    if(operation->cid != NULL) {
        free(operation->cid);
    }
    if(operation->meta != NULL) {
        free_meta(operation->meta);
    }
    free(operation);

}
