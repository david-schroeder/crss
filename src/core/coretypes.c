#include "coretypes.h"
#include "ctconfig.h"

void set_uuid_from_uint32(uuid_t *uuid, uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    uint32_t *uuid_uints = (uint32_t *)uuid;
    /* For ordering see notes in header file */
    uuid_uints[3] = a;
    uuid_uints[2] = b;
    uuid_uints[1] = c;
    uuid_uints[0] = d;
}

void get_uuid_as_uint32(uuid_t *uuid, uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *d) {
    uint32_t *uuid_uints = (uint32_t *)uuid;
    /* For ordering see notes in header file */
    *a = uuid_uints[3];
    *b = uuid_uints[2];
    *c = uuid_uints[1];
    *d = uuid_uints[0];
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
    
    /* Special case: Last 12 digits are one block, so split them up into 4+8 */
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

nbt_node_t *new_nbt_node(uint8_t type, char *name) {
    nbt_node_t *node = calloc(1, sizeof(nbt_node_t));
    node->type = type;
    node->name = name;
    node->name_length = strlen(name);
    return node;
}

nbt_node_t *new_nbt_byte(char *name, int8_t value) {
    nbt_node_t *node = new_nbt_node(NBT_TAG_Byte, name);
    node->payload.data_byte = value;
    return node;
}

nbt_node_t *new_nbt_short(char *name, int16_t value) {
    nbt_node_t *node = new_nbt_node(NBT_TAG_Short, name);
    node->payload.data_short = value;
    return node;
}

nbt_node_t *new_nbt_int(char *name, int32_t value) {
    nbt_node_t *node = new_nbt_node(NBT_TAG_Int, name);
    node->payload.data_int = value;
    return node;
}

nbt_node_t *new_nbt_long(char *name, int64_t value) {
    nbt_node_t *node = new_nbt_node(NBT_TAG_Long, name);
    node->payload.data_long = value;
    return node;
}

nbt_node_t *new_nbt_float(char *name, float value) {
    nbt_node_t *node = new_nbt_node(NBT_TAG_Float, name);
    node->payload.data_float = value;
    return node;
}

nbt_node_t *new_nbt_double(char *name, double value) {
    nbt_node_t *node = new_nbt_node(NBT_TAG_Double, name);
    node->payload.data_double = value;
    return node;
}

nbt_node_t *new_nbt_byte_array(char *name, uint8_t *buf, uint32_t len) {
    nbt_node_t *node = new_nbt_node(NBT_TAG_Byte_Array, name);
    node->payload.data_byte_array.data = buf;
    node->payload.data_byte_array.size = len;
    return node;
}

nbt_node_t *new_nbt_string(char *name, char *buf, uint32_t len) {
    nbt_node_t *node = new_nbt_node(NBT_TAG_String, name);
    node->payload.data_string.data = buf;
    node->payload.data_string.size = len;
    return node;
}

nbt_node_t *new_nbt_cstring(char *name, char *buf) {
    return new_nbt_string(name, buf, strlen(buf));
}

nbt_node_t *new_nbt_list(char *name, uint8_t content_tagid) {
    nbt_node_t *node = new_nbt_node(NBT_TAG_List, name);
    node->payload.data_list.content_tagid = content_tagid;
    node->payload.data_list.size = NBT_LIST_INIT_SIZE;
    node->payload.data_list.used_size = 0;
    node->payload.data_list.contents = calloc(NBT_LIST_INIT_SIZE, sizeof(nbt_node_t));
    return node;
}

nbt_node_t *new_nbt_int_array(char *name) {
    nbt_node_t *node = new_nbt_node(NBT_TAG_Int_Array, name);
    node->payload.data_list.size = NBT_LIST_INIT_SIZE;
    node->payload.data_list.used_size = 0;
    node->payload.data_list.contents = calloc(NBT_LIST_INIT_SIZE, sizeof(nbt_node_t));
    return node;
}

nbt_node_t *new_nbt_long_array(char *name) {
    nbt_node_t *node = new_nbt_node(NBT_TAG_Long_Array, name);
    node->payload.data_list.size = NBT_LIST_INIT_SIZE;
    node->payload.data_list.used_size = 0;
    node->payload.data_list.contents = calloc(NBT_LIST_INIT_SIZE, sizeof(nbt_node_t));
    return node;
}

nbt_node_t *new_nbt_compound(char *name) {
    nbt_node_t *node = new_nbt_node(NBT_TAG_Compound, name);
    node->payload.data_list.size = NBT_LIST_INIT_SIZE;
    node->payload.data_list.used_size = 0;
    node->payload.data_list.contents = calloc(NBT_LIST_INIT_SIZE, sizeof(nbt_node_t));
    return node;
}

void nbt_append_to_list(nbt_node_t *list, nbt_node_t *item) {
    if (list->payload.data_list.used_size == list->payload.data_list.size) {
        /* Allocate more space in list */
        list->payload.data_list.size <<= 1;
        list->payload.data_list.contents = realloc(list->payload.data_list.contents, list->payload.data_list.size*sizeof(nbt_node_t *));
    }
    /* Save item reference into list */
    list->payload.data_list.contents[list->payload.data_list.used_size++] = item;
}

/* Convert `root` into an NBT byte representation */
/* @param named Whether the node is the value of a key-value pair, i.e. the child of a compound */
length_buffer_t *serialize_nbt(nbt_node_t *root, bool named, bool is_root) {
    length_buffer_t *lbuf = malloc(sizeof(length_buffer_t));
    uint32_t bufsize = 0;
    uint8_t *buf = NULL;
    if (is_root) {
        bufsize = 2 + root->name_length;
        buf = malloc(bufsize);
        buf[0] = root->name_length > 8;
        buf[1] = root->name_length & 0xff;
        strncpy((char *)&buf[2], root->name, root->name_length);

    } else if (named && root->type != NBT_TAG_End) {
        /* Place ID + 2-byte length and node name inside buf */
        bufsize = 3 + root->name_length;
        buf = malloc(bufsize);
        buf[0] = root->type;
        buf[1] = root->name_length >> 8; /* Big endian -> MSB first */
        buf[2] = root->name_length & 0xff;
        strncpy((char *)&buf[3], root->name, root->name_length);
    } else if (named) { /* root->type == TAG_End or TAG_Compound */
        /* TAG_End, TAG_Compound have an ID, but no length or name */
        buf = malloc(1);
        buf[0] = root->type;
        bufsize++;
    }
    uint32_t init_bufsize;
    switch (root->type) {
        case NBT_TAG_Byte:
            bufsize++;
            buf = realloc(buf, bufsize);
            buf[bufsize-1] = root->payload.data_byte;
            break;
        case NBT_TAG_Short:
            bufsize += 2;
            buf = realloc(buf, bufsize);
            buf[bufsize-2] = root->payload.data_short >> 8; /* Big Endian */
            buf[bufsize-1] = root->payload.data_short & 0xff;
            break;
        case NBT_TAG_Int:
            bufsize += 4;
            buf = realloc(buf, bufsize);
            buf[bufsize-4] = root->payload.data_int >> 24; /* Big Endian */
            buf[bufsize-3] = (root->payload.data_int >> 16) & 0xff;
            buf[bufsize-2] = (root->payload.data_int >> 8) & 0xff;
            buf[bufsize-1] = root->payload.data_int & 0xff;
            break;
        case NBT_TAG_Long:
            bufsize += 8;
            buf = realloc(buf, bufsize);
            for (int i = 8; i > 0; i--) {
                buf[bufsize-i] = (root->payload.data_long >> ((i-1)*8)) & 0xff;
            }
            break;
        case NBT_TAG_Float:
            bufsize += 4;
            buf = realloc(buf, bufsize);
            float fdata = root->payload.data_float;
            /* No strict aliasing allowed */
            uint32_t f_as_int = *((uint32_t *)&fdata);
            buf[bufsize-4] = f_as_int >> 24; /* Big Endian */
            buf[bufsize-3] = (f_as_int >> 16) & 0xff;
            buf[bufsize-2] = (f_as_int >> 8) & 0xff;
            buf[bufsize-1] = f_as_int & 0xff;
            break;
        case NBT_TAG_Double:
            bufsize += 8;
            buf = realloc(buf, bufsize);
            double ddata = root->payload.data_double;
            uint64_t d_as_long = *((uint64_t *)&ddata);
            for (int i = 8; i > 0; i--) {
                buf[bufsize-i] = (d_as_long >> ((i-1)*8)) & 0xff;
            }
            break;
        case NBT_TAG_String:
            init_bufsize = bufsize;
            bufsize += 2 + root->payload.data_string.size;
            buf = realloc(buf, bufsize);
            buf[init_bufsize] = root->payload.data_string.size >> 8;
            buf[init_bufsize + 1] = root->payload.data_string.size & 0xff;
            strncpy((char *)&buf[init_bufsize + 2], root->payload.data_string.data, root->payload.data_string.size);
            break;
        case NBT_TAG_Byte_Array:
            init_bufsize = bufsize;
            bufsize += 4 + root->payload.data_byte_array.size;
            buf = realloc(buf, bufsize);
            buf[init_bufsize] = root->payload.data_byte_array.size >> 24; /* Big Endian */
            buf[init_bufsize + 1] = (root->payload.data_byte_array.size >> 16) & 0xff;
            buf[init_bufsize + 2] = (root->payload.data_byte_array.size >> 8) & 0xff;
            buf[init_bufsize + 3] = root->payload.data_byte_array.size & 0xff;
            memcpy(&buf[init_bufsize+4], root->payload.data_byte_array.data, root->payload.data_byte_array.size);
            break;
        case NBT_TAG_List:
            init_bufsize = bufsize;
            /* content tagid */
            bufsize += 1;
            buf = realloc(buf, bufsize);
            buf[init_bufsize] = root->payload.data_list.content_tagid;
        case NBT_TAG_Int_Array:
        case NBT_TAG_Long_Array:
            init_bufsize = bufsize;
            /* length */
            bufsize += 4;
            buf = realloc(buf, bufsize);
            buf[init_bufsize] = root->payload.data_list.used_size >> 24; /* Big Endian */
            buf[init_bufsize + 1] = (root->payload.data_list.used_size >> 16) & 0xff;
            buf[init_bufsize + 2] = (root->payload.data_list.used_size >> 8) & 0xff;
            buf[init_bufsize + 3] = root->payload.data_list.used_size & 0xff;
            /* payloads*/
            for (int i = 0; i < root->payload.data_list.used_size; i++) {
                length_buffer_t *serialized_child = serialize_nbt(root->payload.data_list.contents[i], false, false);
                init_bufsize = bufsize;
                bufsize += serialized_child->length;
                buf = realloc(buf, bufsize);
                memcpy(&buf[init_bufsize], serialized_child->buffer, serialized_child->length);
                free(serialized_child->buffer);
                free(serialized_child);
            }
            break;
        case NBT_TAG_Compound:
            /* payloads*/
            for (int i = 0; i < root->payload.data_list.used_size; i++) {
                length_buffer_t *serialized_child = serialize_nbt(root->payload.data_list.contents[i], true, false);
                init_bufsize = bufsize;
                bufsize += serialized_child->length;
                buf = realloc(buf, bufsize);
                memcpy(&buf[init_bufsize], serialized_child->buffer, serialized_child->length);
                free(serialized_child->buffer);
                free(serialized_child);
            }
            /* add TAG_End */
            buf = realloc(buf, bufsize + 1);
            buf[bufsize++] = 0x00;
            break;
        default:
            break;
    }
    lbuf->buffer = buf;
    lbuf->length = bufsize;
    return lbuf;
}

void free_nbt_node(nbt_node_t *node, bool free_children) {
    if (node->type == NBT_TAG_List || node->type == NBT_TAG_Compound
            || node->type == NBT_TAG_Int_Array || node->type == NBT_TAG_Long_Array) {
        if (free_children) {
            /* Free all children */
            for (int i = 0; i < node->payload.data_list.used_size; i++) {
                free_nbt_node(node->payload.data_list.contents[i], free_children);
            }
        }
        free(node->payload.data_list.contents);
    }
    free(node);
}

int nbt_main() {
    reset_all_settings();
    nbt_node_t *c = new_nbt_compound("test_root");
    nbt_node_t *c2 = new_nbt_compound("test_inner");
    nbt_node_t *n1 = new_nbt_byte("test_byte", 0xff);
    nbt_node_t *n2 = new_nbt_short("test_short", 0xabcd);
    nbt_node_t *n3 = new_nbt_int("test_int", 0x9876);
    nbt_node_t *n4 = new_nbt_long("test_long", 0xdcba4321);
    nbt_node_t *n5 = new_nbt_float("test_float", 3.141592653f);
    nbt_node_t *n6 = new_nbt_double("test_double", 3.141592653589793);
    uint8_t test_buf[6] = {0x12, 0x34, 0x56, 0x67, 0x89, 0xab};
    nbt_node_t *n7 = new_nbt_byte_array("test_byte_array", test_buf, 6);
    nbt_node_t *n8 = new_nbt_list("test_list_int", NBT_TAG_Int_Array);
    nbt_node_t *n9 = new_nbt_int_array("test_int_arr");
    nbt_node_t *n10 = new_nbt_long_array("test_long_arr");
    
    nbt_node_t *i1 = new_nbt_int("", 10);
    nbt_node_t *i2 = new_nbt_int("", 20);
    nbt_node_t *i3 = new_nbt_int("", 42);
    nbt_node_t *i4 = new_nbt_int("", 69);
    nbt_node_t *i5 = new_nbt_int("", -9999);
    nbt_append_to_list(n9, i1);
    nbt_append_to_list(n9, i2);
    nbt_append_to_list(n9, i3);
    nbt_append_to_list(n9, i4);
    nbt_append_to_list(n9, i5);

    nbt_node_t *l1 = new_nbt_long("", 10);
    nbt_node_t *l2 = new_nbt_long("", 20);
    nbt_node_t *l3 = new_nbt_long("", 42);
    nbt_node_t *l4 = new_nbt_long("", 69);
    nbt_node_t *l5 = new_nbt_long("", -9999);
    nbt_append_to_list(n10, l1);
    nbt_append_to_list(n10, l2);
    nbt_append_to_list(n10, l3);
    nbt_append_to_list(n10, l4);
    nbt_append_to_list(n10, l5);

    nbt_append_to_list(n8, n9);
    nbt_append_to_list(n8, n9);

    nbt_append_to_list(c, n1);
    nbt_append_to_list(c, n2);
    nbt_append_to_list(c, n3);
    nbt_append_to_list(c, n4);
    nbt_append_to_list(c, c2);
    nbt_append_to_list(c2, n5);
    nbt_append_to_list(c2, n6);
    nbt_append_to_list(c2, n7);
    nbt_append_to_list(c2, n8);
    nbt_append_to_list(c2, n10);

    length_buffer_t *s = serialize_nbt(c, true, true);
    char s_bytes[1024];
    for (int i = 0; i < s->length; i++) {
        uint8_t byte = s->buffer[i];
        sprintf(&s_bytes[3*i], "%02x ", byte & 0xff);
    }
    DLINFO("Generated serial:\n'%s'", s_bytes);

    const char *fp = "test.nbt";
    DLINFO("Writing to file %s...", fp);
    FILE *file = fopen(fp, "wb");
    fwrite(s->buffer, 1, s->length, file);
    fclose(file);

    free(s->buffer);
    free(s);
    free_nbt_node(c, false);
    free_nbt_node(c2, false);
    free_nbt_node(n1, false);
    free_nbt_node(n2, false);
    free_nbt_node(n3, false);
    free_nbt_node(n4, false);
    free_nbt_node(n5, false);
    free_nbt_node(n6, false);
    free_nbt_node(n7, false);
    free_nbt_node(n8, false);
    free_nbt_node(n9, true);
    free_nbt_node(n10, true);
    return 0;
}

int nbt_main2() {
    reset_all_settings();

    nbt_node_t *dim_codec = new_nbt_compound("crss.dimension_codec");
            nbt_node_t *dim_type = new_nbt_compound("dimension_type");
            nbt_append_to_list(dim_codec, dim_type);
                nbt_node_t *dt_type = new_nbt_cstring("type", "minecraft:dimension_type");
                nbt_append_to_list(dim_type, dt_type);
                nbt_node_t *dt_value = new_nbt_list("value", NBT_TAG_Compound);
                nbt_append_to_list(dim_type, dt_value);
                    nbt_node_t *dim = new_nbt_compound("crss:world");
                    nbt_append_to_list(dt_value, dim);
                        nbt_node_t *dtv_name = new_nbt_cstring("name", "crss:world");
                        nbt_append_to_list(dim, dtv_name);
                        nbt_node_t *dtv_id = new_nbt_int("id", 0);
                        nbt_append_to_list(dim, dtv_id);
                        nbt_node_t *dtv_elem = new_nbt_compound("element");
                        nbt_append_to_list(dim, dtv_elem);
                            nbt_append_to_list(dtv_elem, new_nbt_byte("piglin_safe", DIM_CODEC_PIGLIN_SAFE));
                            nbt_append_to_list(dtv_elem, new_nbt_byte("natural", DIM_CODEC_NATURAL));
                            nbt_append_to_list(dtv_elem, new_nbt_float("ambient_light", DIM_CODEC_AMBIENT_LIGHT));
                            nbt_append_to_list(dtv_elem, new_nbt_long("fixed_time", DIM_CODEC_FIXED_TIME));
                            nbt_append_to_list(dtv_elem, new_nbt_cstring("infiniburn", DIM_CODEC_INFINIBURN));
                            nbt_append_to_list(dtv_elem, new_nbt_byte("respawn_anchor_works", DIM_CODEC_RESPAWN_ANCHOR_WORKS));
                            nbt_append_to_list(dtv_elem, new_nbt_byte("has_skylight", DIM_CODEC_HAS_SKYLIGHT));
                            nbt_append_to_list(dtv_elem, new_nbt_byte("bed_works", DIM_CODEC_BED_WORKS));
                            nbt_append_to_list(dtv_elem, new_nbt_cstring("effects", DIM_CODEC_EFFECTS));
                            nbt_append_to_list(dtv_elem, new_nbt_byte("has_raids", DIM_CODEC_HAS_RAIDS));
                            nbt_append_to_list(dtv_elem, new_nbt_int("min_y", DIM_CODEC_MIN_Y));
                            nbt_append_to_list(dtv_elem, new_nbt_int("height", DIM_CODEC_HEIGHT));
                            nbt_append_to_list(dtv_elem, new_nbt_int("logical_height", DIM_CODEC_LOGICAL_HEIGHT));
                            nbt_append_to_list(dtv_elem, new_nbt_double("coordinate_scale", DIM_CODEC_COORDINATE_SCALE));
                            nbt_append_to_list(dtv_elem, new_nbt_byte("ultrawarm", DIM_CODEC_ULTRAWARM));
                            nbt_append_to_list(dtv_elem, new_nbt_byte("has_ceiling", DIM_CODEC_HAS_CEILING));
            nbt_node_t *worldgen_biome = new_nbt_compound("worldgen/biome");
            nbt_append_to_list(dim_codec, worldgen_biome);
                nbt_node_t *wb_type = new_nbt_cstring("type", "minecraft:worldgen/biome");
                nbt_append_to_list(worldgen_biome, wb_type);
                nbt_node_t *wb_value = new_nbt_list("value", NBT_TAG_Compound);
                nbt_append_to_list(worldgen_biome, wb_value);
                    /* Biome: Plot */
                    nbt_node_t *biome = new_nbt_compound("crss:plot");
                    nbt_append_to_list(wb_value, biome);
                        nbt_node_t *wbv_name = new_nbt_cstring("name", "crss:plot");
                        nbt_append_to_list(biome, wbv_name);
                        nbt_node_t *wbv_id = new_nbt_int("id", 0);
                        nbt_append_to_list(biome, wbv_id);
                        nbt_node_t *wbv_elem = new_nbt_compound("element");
                        nbt_append_to_list(biome, wbv_elem);
                            nbt_append_to_list(wbv_elem, new_nbt_cstring("precipitation", "none"));
                            nbt_append_to_list(wbv_elem, new_nbt_float("depth", 0.0));
                            nbt_append_to_list(wbv_elem, new_nbt_float("temperature", 0.8));
                            nbt_append_to_list(wbv_elem, new_nbt_float("scale", 1.0));
                            nbt_append_to_list(wbv_elem, new_nbt_float("downfall", 0.0));
                            nbt_append_to_list(wbv_elem, new_nbt_cstring("category", "plains"));
                            nbt_node_t *wbv_effects = new_nbt_compound("effects");
                            nbt_append_to_list(wbv_elem, wbv_effects);
                                nbt_append_to_list(wbv_effects, new_nbt_int("sky_color", 0x7FA1FF));
                                nbt_append_to_list(wbv_effects, new_nbt_int("water_fog_color", 0x7FA1FF));
                                nbt_append_to_list(wbv_effects, new_nbt_int("fog_color", 0x7FA1FF));
                                nbt_append_to_list(wbv_effects, new_nbt_int("water_color", 0x7FA1FF));

    length_buffer_t *s = serialize_nbt(dim_codec, true, true);
    free_nbt_node(dim_codec, true);
    char s_bytes[8192];
    for (int i = 0; i < s->length; i++) {
        uint8_t byte = s->buffer[i];
        sprintf(&s_bytes[3*i], "%02x ", byte & 0xff);
    }
    DLINFO("Generated serial:\n'%s'", s_bytes);

    const char *fp = "test.nbt";
    DLINFO("Writing to file %s...", fp);
    FILE *file = fopen(fp, "wb");
    fwrite(s->buffer, 1, s->length, file);
    fclose(file);

    free(s->buffer);
    free(s);

    return 0;
}
