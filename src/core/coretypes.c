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

char *uuid_to_string(uuid_t *uuid) {
    char *uuid_str = calloc(37, sizeof(char));
    uint32_t a, b, c, d;
    get_uuid_as_uint32(uuid, &a, &b, &c, &d);
    sprintf(uuid_str, "%08x-%04x-%04x-%04x-%04x%08x",
            a,
            b >> 16,
            b & 0xFFFF,
            c >> 16,
            c & 0xFFFF, d);
    return uuid_str;
}

bool string_to_uuid(char *str, uuid_t *uuid) {
    /* Expected format: 01234567-0123-0123-0123-0123456789AB (36 chars) */
    /* Input validation */
    if (strlen(str) != 36) {
        DLDEBUG("1 (%d)", strlen(str));
        return false;
    }
    if (str[8] != '-') {
        DLDEBUG("2");
        return false;
    }
    if (str[13] != '-') {
        DLDEBUG("3");
        return false;
    }
    if (str[18] != '-') {
        DLDEBUG("4");
        return false;
    }
    if (str[23] != '-') {
        DLDEBUG("5");
        return false;
    }
    
    /* Special case: Last 12 digits are one block, so split them up into 4+12*/
    char c_lower[5];
    for (int i = 0; i < 4; i++) c_lower[i] = str[24+i];
    c_lower[4] = '\0';

    /* Get UUID uint32s */
    uint32_t a, b, c, d;
    a = strtol(&str[0], NULL, 16);
    b = (strtol(&str[9], NULL, 16) << 16) + strtol(&str[14], NULL, 16);
    c = (strtol(&str[18], NULL, 16) << 16) + strtol(c_lower, NULL, 16);
    d = strtol(&str[27], NULL, 16);
    set_uuid_from_uint32(uuid, a, b, c, d);

    return true;
}

int uuidcmp(uuid_t *a, uuid_t *b) {
    int diff = 0;
    for (int i = 0; i < 16; i++) {
        uint8_t val_a, val_b;
        val_a = ((uint8_t *)a)[i];
        val_b = ((uint8_t *)b)[i];
        if (val_a != val_b) {
            diff++;
        }
    }
    return diff;
}
