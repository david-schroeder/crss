#ifndef CORETYPES_H
#define CORETYPES_H

#include "utils.h"
#include <stdlib.h>

typedef uint8_t uuid_t[16];

void set_uuid_from_uint32(uuid_t *uuid, uint32_t a, uint32_t b, uint32_t c, uint32_t d);

void get_uuid_as_uint32(uuid_t *uuid, uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *d);

void copy_uuid(uuid_t *dest, uuid_t *src);

#endif // CORETYPES_H
