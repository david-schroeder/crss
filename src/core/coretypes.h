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

/*
NBT
*/

typedef struct nbt_node_ {
    uint8_t type;
    uint16_t name_length;
    char *name;
    union {
        int8_t data_byte;
        int16_t data_short;
        int32_t data_int;
        int64_t data_long;
        float data_float;
        double data_double;
        struct {
            int32_t size;
            uint8_t *data;
        } data_byte_array;
        struct {
            uint16_t size;
            char *data;
        } data_string;
        struct { /* Used for List, Compound, Int array and Long array */
            uint8_t content_tagid;
            uint32_t size;
            uint32_t used_size;
            struct nbt_node_ **contents; /* Array of pointers to child objects */
        } data_list;
    } payload;
} nbt_node_t;

typedef struct {
    size_t length;
    uint8_t *buffer;
} length_buffer_t;

#define NBT_TAG_End 0x00
#define NBT_TAG_Byte 0x01
#define NBT_TAG_Short 0x02
#define NBT_TAG_Int 0x03
#define NBT_TAG_Long 0x04
#define NBT_TAG_Float 0x05
#define NBT_TAG_Double 0x06
#define NBT_TAG_Byte_Array 0x07
#define NBT_TAG_String 0x08
#define NBT_TAG_List 0x09
#define NBT_TAG_Compound 0x0A
#define NBT_TAG_Int_Array 0x0B
#define NBT_TAG_Long_Array 0x0C

nbt_node_t *new_nbt_node(uint8_t type, char *name);

nbt_node_t *new_nbt_byte(char *name, int8_t value);

nbt_node_t *new_nbt_short(char *name, int16_t value);

nbt_node_t *new_nbt_int(char *name, int32_t value);

nbt_node_t *new_nbt_long(char *name, int64_t value);

nbt_node_t *new_nbt_float(char *name, float value);

nbt_node_t *new_nbt_double(char *name, double value);

nbt_node_t *new_nbt_byte_array(char *name, uint8_t *buf, uint32_t len);

nbt_node_t *new_nbt_string(char *name, char *buf, uint32_t len);

nbt_node_t *new_nbt_cstring(char *name, char *buf);

nbt_node_t *new_nbt_list(char *name, uint8_t content_tagid);

nbt_node_t *new_nbt_compound(char *name);

void nbt_append_to_list(nbt_node_t *list, nbt_node_t *item);

length_buffer_t *serialize_nbt(nbt_node_t *root, bool named, bool is_root);

#define NBT_LIST_INIT_SIZE 8

void free_nbt_node(nbt_node_t *node, bool free_children);

/*
Player Data

This is local player data necessary for communicating with the client. One object is maintained per player/connection.
*/

typedef enum {
    CL_CHAT_SETTINGS_FULL,
    CL_CHAT_SETTINGS_CMDONLY,
    CL_CHAT_SETTINGS_HIDDEN
} mcclient_chat_settings_e;

#define SKIN_PART_CAPE 0x01
#define SKIN_PART_JACKET 0x02
#define SKIN_PART_LEFT_SLEEVE 0x04
#define SKIN_PART_RIGHT_SLEEVE 0x08
#define SKIN_PART_LEFT_PANTS 0x10
#define SKIN_PART_RIGHT_PANTS 0x20
#define SKIN_PART_HAT 0x40

typedef struct {
    char *id; /* NOTE: consider integer/enum */
    uint8_t count;
} slot_t;

typedef struct {
    slot_t *slots;
    uint32_t count;
} container_t;

typedef struct {
    uuid_t uuid;
    struct {
        char *locale;
        int8_t view_dist;
        mcclient_chat_settings_e chat_mode;
        bool chat_colors;
        uint8_t skin_parts;
        uint32_t main_hand;
        bool enable_text_filtering;
        bool allow_server_listings;
    } settings;
    double x;
    double y;
    double z;
    float theta; // facing (yaw)
    float phi; // facing (pitch)
    container_t *inventory;
    uint8_t held_item; // slot id of the held item (0-8)
} client_data_t;

#endif // CORETYPES_H
