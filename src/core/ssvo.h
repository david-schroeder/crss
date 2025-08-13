#ifndef SSVO_H
#define SSVO_H

#include "utils.h"

/*
Specialized Sparse Voxel Octree (SSVO) interface for CRSS.

Provides an SSVO data structure as well as SSVO node data structures, as well as getter and setter methods for leaf nodes.

An SSVO allows getting and setting in O(log_8(n)), where n is the number of leaf nodes.

An SSVO has a fixed size, named the "side length", which must be a power of two. Its logarithm (base 2) indicates how many layers the SSVO will have.

Each SSVO has an OVB (Octree Voxel Buffer) associated with it, which is a buffer containing SSVO Node objects. The advantage of using a such buffer is that the number of bits required for pointers within a node can be significantly reduced with this approach, storing an index in the OVB as opposed to a 4 (or 8) byte pointer.

Nodes in the OVB are layed out such that the eight child nodes of a parent node are consecutively packed, thus each node needs only store the index of the first child node in the OVB.

An important design decision is that 16-bit values are used for these indices. This makes the SSVO data more cache-friendly, yet comes with a few trade-offs. Most importantly, the restriction to 2-byte indices means that at most 65536 values can be stored; since this only enables (non-sparse) octrees of depth up to 5, an additional workaround is needed for larger trees. This workaround is to use a so-called OVT, or Octree Voxel Table, which stores pointers to OVBs. If the MSB of a node is set, then the lower 15 bits serve as an index into an octree's OVT, which supplies a pointer to a new OVB which can be used for the subtree where the root is the node whose MSB was initially set.
Note that this again cuts the number of values that can be stored in a single OVB to 2^15, or 32768.

Leaf nodes are represented in the OVB as 8-entry arrays of void pointers. The internal OVB allocator must make sure that arrays don't conflict with each other and that an array is only allocated in the OVB if there is enough space.

A large problem however arises when naively filling OVBs before allocating OVTs to new nodes. The problem is that root nodes of newly allocated OVBs within an OVT tend, with this naive approach, to have a very shallow depth, leading to effective loss of most storage space in non-root OVBs and thus a premature exhaustion of available OVBs in an SSVO's OVT. The solution to this problem is to select OVT allocation before an (especially the root) OVB is exhausted. A relevant observation here is that six layers of the octree take up ((8^6)-1)/7 = 37.449 entries, which is just above the 32.768 natively supported by the octree. A complete octree should thus ideally store about six child layers per OVB. However, since we assume a mostly-sparse tree, we should assume that these layers won't be completely filled; thus this implementation tries to fit 5 layers of children per OVB.

*/

/* Do NOT change these values unless you know what you're doing! */
/* Practically, they should never be changed */

// May not be more than 65536
#define OVB_SIZE 32768
#define OVT_SIZE 32768


/*
An SSVO node must only contain the index of the first child in the OVB. Thus, the node itself effectively can become this index.

If the MSB is set, then the item in the OVB at the corresponding index is an index into the container SSVO's OVT.

If the value is 0 (which would be a reference to the root node), it is safe to assume that the current node has no child nodes yet.
*/
typedef uint16_t ssvo_node_t;

typedef struct {
    ssvo_node_t nodes[OVB_SIZE];
    uint16_t used;
    uint16_t top_level_id;
} ovb_t;

typedef struct {
    ovb_t *entries[OVT_SIZE];
    uint16_t used;
} ovt_t;

typedef struct {
    ovt_t ovt;
    uint8_t log2_sidelength;
    uint32_t sidelength;
} ssvo_t;

ssvo_t *new_ssvo(uint8_t log2_size);

void free_ssvo(ssvo_t *ssvo);

void *ssvo_get_leaf(ssvo_t *ssvo, uint32_t x, uint32_t y, uint32_t z);

void ssvo_set_leaf(ssvo_t *ssvo, uint32_t x, uint32_t y, uint32_t z, void *data);

void prettyprint_ssvo(char *fnpath, ssvo_t *ssvo);

#endif // SSVO_H
