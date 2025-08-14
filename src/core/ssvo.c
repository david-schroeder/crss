#include "ssvo.h"

ssvo_t *new_ssvo(uint8_t log2_size) {
    ssvo_t *ssvo = calloc(1, sizeof(ssvo_t));
    ssvo->sidelength = 1 << log2_size;
    ssvo->log2_sidelength = log2_size;
    ovb_t *root_ovb = calloc(1, sizeof(ovb_t));
    root_ovb->top_level_id = log2_size;
    ssvo->ovt.entries[0] = root_ovb;
    ssvo->ovt.used++;
    if (log2_size == 0) {
        // Single data pointer
        root_ovb->used += sizeof(void*) / sizeof(ssvo_node_t);
    } else if (log2_size == 1) {
        // 8 data pointers
        root_ovb->used += 8 * sizeof(void*) / sizeof(ssvo_node_t);
    } else {
        // 8 children nodes
        root_ovb->used += 8;
    }
    return ssvo;
}

void free_ssvo(ssvo_t *ssvo) {
    for (int i = 0; i < ssvo->ovt.used; i++) {
        if (ssvo->ovt.entries[i]) {
            free(ssvo->ovt.entries[i]);
        }
    }
    free(ssvo);
}

/*
Allocate new node in an OVB.
@param ovb OVB to allocate new node in
@param node_is_leaf Whether the node has (only) leaf children
@return 0 If the OVB is full, else a valid index for a new node
*/
static ssvo_node_t allocate_new_ssvo_node(ovb_t *ovb, bool node_is_leaf) {
    size_t node_size = node_is_leaf ? sizeof(void*)*8 : sizeof(ssvo_node_t) * 8;
    if (ovb->used >= (OVB_SIZE - node_size)) return 0;
    // Allocated position is just after all used data
    // A drawback of this is that nodes that become empty can effectively not be freed
    // This is however not very problematic because
    //   a) A program restart rebuilds the SSVO, fixing the "problem"
    //   b) Already having allocated structures for empty nodes means that if they
    //      eventually become used again, the allocation doesn't need to reoccur
    ssvo_node_t ret = (ssvo_node_t)ovb->used;
    ovb->used += node_size / sizeof(ssvo_node_t); // divisibility is always guaranteed by size
    //DLDEBUG("Allocated new %s OVB entry @ [%d]!", node_is_leaf ? "leaf" : "non-leaf", ret);
    return ret;
}

/*
Allocate new OVB in an SSVO's OVT.
@return Identifier of OVT which is qualified as an OVB entry, 0 if OVT is full
*/
static ssvo_node_t allocate_new_ovb(ssvo_t *ssvo, uint16_t level, bool node_is_leaf) {
    if (ssvo->ovt.used == OVT_SIZE) return 0; // Critical failure
    
    /* Allocate a new OVB and place it in the OVT */
    ovb_t *entry = calloc(1, sizeof(ovb_t));
    if (entry == NULL) return 0;
    entry->top_level_id = level;
    uint16_t ovt_idx = ssvo->ovt.used++;
    ssvo->ovt.entries[ovt_idx] = entry;
    
    /* Update used entry count */
    size_t node_size = node_is_leaf ? sizeof(void*)*8 : sizeof(ssvo_node_t) * 8;
    entry->used += node_size / sizeof(ssvo_node_t);

    //DLDEBUG("Allocated new OVT @ [%d]!", ovt_idx);
    return (ssvo_node_t)(0x8000 | ovt_idx); // Set MSB in returned index
}

static void **ssvo_get_leaf_ptr(ssvo_t *ssvo, uint32_t x, uint32_t y, uint32_t z) {
    /* SSVO Traversal */
    // TODO: check whether (x, y, z) is actually in the SSVO

    /* Bit manipulation constants for isolating bits from x, y, z */
    const uint8_t shamt = ssvo->log2_sidelength;

    // Since bit order is ZYX, shift inputs to simplify calculation later
    z <<= 3;
    y <<= 2;
    x <<= 1; // We shift everything by at least 1 to allow rsh by shamt

    /* Current node information */
    ovb_t *current_ovb = ssvo->ovt.entries[0];
    ssvo_node_t current_node = 0;
    ssvo_node_t next_node_idx = 0;
    uint8_t child_idx;
    ssvo_node_t *children; // 8-Array of child indices
    
    /* Traverse SSVO */
    for (uint8_t level = ssvo->log2_sidelength; level > 0; level--) {
        // current_node is an index to an 8-element array of child nodes
        // or a pointer to a new OVB in the OVT
        if (current_node & 0x8000) {
            // We don't need to worry about the entry not existing because
            // it must exist if the MSB is set :)
            current_ovb = ssvo->ovt.entries[current_node & 0x7FFF];
            current_node = 0;
        }
        children = &current_ovb->nodes[current_node];

        /* Find child index */
        // Bit order: ZYX
        child_idx = ((z >> shamt) & 4) | ((y >> shamt) & 2) | ((x >> shamt) & 1);

        /* If we're on the final level, return the child */
        // Reinterpret child array as array of void* data pointers according to spec
        if (level == 1) {
            //DLDEBUG("%p", children);
            return &((void**)children)[child_idx];
        }

        /* Otherwise, find child node index */
        next_node_idx = children[child_idx];
        if (next_node_idx == 0) {
            /* Child does not exist -> allocate new one */
            // If we're beyond the optimum number of layers, allocate new OVB instead
            if (level % 5 != 0) {
                // new node is a leaf node iff we're on the last level of traversing
                //DLVERBOSE("Allocating SSVO node on level %d!", level);
                next_node_idx = allocate_new_ssvo_node(current_ovb, level == 2);
            }
            if (next_node_idx == 0) {
                // Allocate new OVB in OVT
                //DLVERBOSE("Allocating OVB on level %d!", level);
                next_node_idx = allocate_new_ovb(ssvo, level, level == 2);
                if (next_node_idx == 0) {
                    /* OVT allocation failed -> Critical error */
                    DLFATAL("Critical World Storage error! [OVT Allocation Fail]");
                    return NULL;
                }
            }
            /* If we allocated a new node, add it as a child to the OVB */
            children[child_idx] = next_node_idx;
        }
        // Shift so that next bits are in the right position
        x <<= 1;
        y <<= 1;
        z <<= 1;
        current_node = next_node_idx;
    }
    // If we reach here, then log2_sidelength must be 0 -> there is only one valid data pointer
    // Which is located in the first ovb at position 0
    return (void**)(&ssvo->ovt.entries[0]->nodes[0]);
}

void *ssvo_get_leaf(ssvo_t *ssvo, uint32_t x, uint32_t y, uint32_t z) {
    void **leafptr = ssvo_get_leaf_ptr(ssvo, x, y, z);
    if (leafptr == NULL) return NULL;
    return *leafptr;
}

void ssvo_set_leaf(ssvo_t *ssvo, uint32_t x, uint32_t y, uint32_t z, void *data) {
    void **leafptr = ssvo_get_leaf_ptr(ssvo, x, y, z);
    if (leafptr == NULL) {
        DLFATAL("%d/%d/%d failed!", x, y, z);
        return;
    }
    *leafptr = data;
}

static void prettyprint_ssvo_node(char *fnpath, ssvo_t *ssvo, ovb_t *ovb, ssvo_node_t idx, char *prefix, uint8_t level, uint32_t x, uint32_t y, uint32_t z) {
    uint32_t sidelength = 1 << level;
    uint32_t half_sidelength = sidelength >> 1;
    
    // multiply by 2 for 16-bit box chars to work
    char *new_prefix = calloc(strlen(prefix)*2 + 4*2+1, sizeof(char));
    sprintf(new_prefix, "%s│   ", prefix);

    ssvo_node_t *children = &ovb->nodes[idx];
    for (int child_idx = 0; child_idx < 8; child_idx++) {
        uint8_t child_x = child_idx & 1;
        uint8_t child_y = (child_idx & 2) >> 1;
        uint8_t child_z = (child_idx & 4) >> 2;
        uint32_t child_x_ofs = half_sidelength * child_x;
        uint32_t child_y_ofs = half_sidelength * child_y;
        uint32_t child_z_ofs = half_sidelength * child_z;
        /* Leaf nodes */
        if (level == 1) {
            void *data = ((void **)children)[child_idx];
            if (child_idx != 7) {
                LINFO("\033[2m[%2d]\033[0m %s├── Child %d \033[2m(%d,%d,%d) \033[1m[%p]\033[0m", level, prefix, child_idx, x+child_x_ofs, y+child_y_ofs, z+child_z_ofs, data);
            } else {
                LINFO("\033[2m[%2d]\033[0m %s└── Child %d \033[2m(%d,%d,%d) \033[1m[%p]\033[0m", level, prefix, child_idx, x+child_x_ofs, y+child_y_ofs, z+child_z_ofs, data);
            }
            continue;
        }
        /* Other nodes */
        if (children[child_idx] != 0) {
            ovb_t *child_ovb = ovb;
            ssvo_node_t next_idx = children[child_idx];
            if (children[child_idx] & 0x8000) {
                child_ovb = ssvo->ovt.entries[children[child_idx] & 0x7FFF];
                next_idx = 0;
            }
            if (child_idx != 7) {
                LINFO("\033[2m[%2d]\033[0m %s├── Child %d \033[2m[(%d,%d,%d) - (%d,%d,%d)]\033[0m", level, prefix, child_idx, x+child_x_ofs, y+child_y_ofs, z+child_z_ofs, x+child_x_ofs+half_sidelength-1, y+child_y_ofs+half_sidelength-1, z+child_z_ofs+half_sidelength-1);
            } else {
                LINFO("\033[2m[%2d]\033[0m %s└── Child %d \033[2m[(%d,%d,%d) - (%d,%d,%d)]\033[0m", level, prefix, child_idx, x+child_x_ofs, y+child_y_ofs, z+child_z_ofs, x+child_x_ofs+half_sidelength-1, y+child_y_ofs+half_sidelength-1, z+child_z_ofs+half_sidelength-1);
                sprintf(new_prefix, "%s    ", prefix);
            }
            prettyprint_ssvo_node(fnpath, ssvo, child_ovb, next_idx, new_prefix, level-1,
                x + (half_sidelength * child_x),
                y + (half_sidelength * child_y),
                z + (half_sidelength * child_z)
            );
        } else {
            if (child_idx != 7) {
                LINFO("\033[2m[%2d]\033[0m %s├── Child %d \033[2m[(%d,%d,%d) - (%d,%d,%d)] \033[0;32m(Empty)\033[0m", level, prefix, child_idx, x+child_x_ofs, y+child_y_ofs, z+child_z_ofs, x+child_x_ofs+half_sidelength-1, y+child_y_ofs+half_sidelength-1, z+child_z_ofs+half_sidelength-1);
            } else {
                LINFO("\033[2m[%2d]\033[0m %s└── Child %d \033[2m[(%d,%d,%d) - (%d,%d,%d)] \033[0;32m(Empty)\033[0m", level, prefix, child_idx, x+child_x_ofs, y+child_y_ofs, z+child_z_ofs, x+child_x_ofs+half_sidelength-1, y+child_y_ofs+half_sidelength-1, z+child_z_ofs+half_sidelength-1);
            }
        }
    }
    free(new_prefix);
}

void prettyprint_ssvo(char *fnpath, ssvo_t *ssvo) {
    FUNCPATH("ssvo.printer");
    // Tree traversal with printing
    LINFO("SSVO Root [Side length %d]", ssvo->sidelength);
    prettyprint_ssvo_node(fnpath, ssvo, ssvo->ovt.entries[0], 0, "", ssvo->log2_sidelength, 0, 0, 0);
    free(fnpath);
}

int ssvo_main(int argc, char *argv[]) {
    char *fnpath = "ssvo.main";
    reset_all_settings();

    size_t TEST_SIZE = 32;
    size_t sidelength = 12;
    if (argc >= 2) {
        TEST_SIZE = atoi(argv[1]);
    }
    if (argc >= 3) {
        sidelength = atoi(argv[2]);
    }

    LINFO("Running tests with TEST_SIZE=%d, SIDELENGTH=%d...", TEST_SIZE, sidelength);

    size_t TS_SQUARED = TEST_SIZE*TEST_SIZE;
    ssvo_t *s = new_ssvo(sidelength);
    uint8_t *test_array = malloc(TEST_SIZE*TS_SQUARED);
    for (int x = 0; x < TEST_SIZE; x++) {
        for (int y = 0; y < TEST_SIZE; y++) {
            for (int z = 0; z < TEST_SIZE; z++) {
                ssvo_set_leaf(s, x, y, z, &test_array[z*TS_SQUARED+y*TEST_SIZE+x]);
            }
        }
    }
    bool pass = true;
    for (int x = 0; x < TEST_SIZE; x++) {
        for (int y = 0; y < TEST_SIZE; y++) {
            for (int z = 0; z < TEST_SIZE; z++) {
                void *res = ssvo_get_leaf(s, x, y, z);
                if (res != &test_array[z*TS_SQUARED+y*TEST_SIZE+x]) {
                    LFATAL("SSVO test failed @ (%d,%d,%d)!", x, y, z);
                    pass = false;
                }
            }
        }
    }
    if (pass) LINFO("Passed all tests!");
    //prettyprint_ssvo(fnpath, s);
    free_ssvo(s);
    free(test_array);
    return 0;
}
