#ifndef __HELPER_H__
#define __HELPER_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "wrapper.h"

#include "proto/meta.pb-c.h"
#include "proto/operation.pb-c.h"

size_t read_len(int clientfd);
uint8_t *read_content(int clientfd, size_t len);

#endif
