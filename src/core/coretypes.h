#ifndef CORETYPES_H
#define CORETYPES_H

#include "utils.h"
#include <stdlib.h>

typedef uint8_t uuid_t[16];

void set_uuid_from_uint32(uuid_t *uuid, uint32_t a, uint32_t b, uint32_t c, uint32_t d);

void get_uuid_as_uint32(uuid_t *uuid, uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *d);

void copy_uuid(uuid_t *dest, uuid_t *src);

/* Convert a UUID to a string, with appropriate hyphen delimiting. */
char *uuid_to_string(uuid_t *uuid);

/* @return Whether the input was in a valid format */
bool string_to_uuid(char *str, uuid_t *uuid);

int uuidcmp(uuid_t *a, uuid_t *b);


#endif // CORETYPES_H
