#include "coretypes.h"

void set_uuid_from_uint32(uuid_t *uuid, uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    uint32_t *uuid_uints = (uint32_t *)uuid;
    uuid_uints[0] = a;
    uuid_uints[1] = b;
    uuid_uints[2] = c;
    uuid_uints[3] = d;
}

void get_uuid_as_uint32(uuid_t *uuid, uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *d) {
    uint32_t *uuid_uints = (uint32_t *)uuid;
    *a = uuid_uints[0];
    *b = uuid_uints[1];
    *c = uuid_uints[2];
    *d = uuid_uints[3];
}

void copy_uuid(uuid_t *dest, uuid_t *src) {
    memcpy(dest, src, sizeof(uuid_t));
}
