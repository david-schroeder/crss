#ifndef CORETYPES_H
#define CORETYPES_H

#include "utils.h"
#include <stdlib.h>

/*
UUIDs

Most of the UUID code is self-explanatory, however there is a key design decision to
reflect when it comes to representation as uint32_t's. Functions like `set_uuid_from_uint32`
expect the a integer to be the most significant one, whilst internally the
little-endianness of the host machine means that the bytes within the uint are "in the wrong
order". In practice, the significance of the bytes of a uint using an intuitive
implementation, where the order of uint32_t's is as expected, would thus be:

(low address)                    (high address)
12 13 14 15 08 09 10 11 04 05 06 07 00 01 02 03

Since this is tricky to access at best, the integers a, b, c and d are ordered in reverse,
yielding the significance order:

(low address)                    (high address)
00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15
*/

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
