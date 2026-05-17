#ifndef GRAPH_H
#define GRAPH_H

#include "utils.h"

typedef enum : uint8_t {
    REPEATER,
    COMPARATOR,
    TORCH,
    DUST,
    LAMP,
    LEVER
} rs_nodetype_e;

typedef enum : uint8_t {
    UNIPORT,
    COMPARATOR_SIDE,
    COMPARATOR_BACK,
    REPEATER_SIDE,
    REPEATER_BACK
} rs_porttype_e;

typedef enum : uint8_t {
    NORTH,
    SOUTH,
    EAST,
    WEST
} rs_direction_e;

#define DIRECT_IN_EDGE_COUNT 6
#define DIRECT_OUT_EDGE_COUNT 6

typedef struct {
    uint32_t size;
    uint32_t used;
    uint32_t *ptr;
} rs_edge_array_t;

#define INITIAL_EDGE_ARRAY_SIZE 64

typedef struct {
    union {
        // MSB set: entry dead (free)
        uint32_t in_edges[DIRECT_IN_EDGE_COUNT]; // 24 bytes
        rs_edge_array_t *in_edge_array; // 8 bytes
    };
    union {
        // MSB set: entry dead (free)
        uint32_t out_edges[DIRECT_OUT_EDGE_COUNT]; // 24 bytes
        rs_edge_array_t *out_edge_array; // 8 bytes
    };
    uint16_t in_degree, out_degree; // 4 bytes
    uint16_t x, y, z;               // 6 bytes

    rs_nodetype_e type;             // 1 byte
    uint8_t constant_ss;            // 1 byte
    uint8_t repeater_ticks;         // 1 byte
    bool subtract_mode;             // 1 byte
    rs_direction_e facing;          // 1 byte
    bool valid;                     // 1 byte
} rs_node_t; // precisely fits in a cache line

typedef struct {
    uint32_t source;
    uint32_t target;
    uint8_t decay; // reduction of signal strength across edge
    rs_porttype_e port;
    bool valid;
} rs_edge_t;

/*
Convention: Nodes are never deleted from or added to
the graph after initial construction. They may be
replaced or marked as invalid. The same goes for
edges.
*/
typedef struct {
    uint32_t nodes_used;
    rs_node_t *nodes;

    uint32_t edges_used;
    rs_edge_t *edges;
} rs_graph_t;

rs_graph_t *rs_graph_new(void);

void rs_graph_free(rs_graph_t *graph);


rs_edge_t *rs_graph_alloc_edge(rs_graph_t *graph);

rs_edge_t *rs_graph_add_edge(rs_graph_t *graph, rs_node_t *source, rs_node_t *target);

void rs_graph_delete_edge(rs_graph_t *graph, rs_edge_t *edge);


rs_node_t *rs_graph_alloc_node(rs_graph_t *graph);

rs_node_t *rs_graph_get_node_at(rs_graph_t *graph, uint16_t x, uint16_t y, uint16_t z);

void rs_graph_delete_node(rs_graph_t *graph, rs_node_t *node);


#define RS_GRAPH_ITER_FAN_IN(graph, node, codeblock) do {                  \
    if ((node)->in_degree > DIRECT_IN_EDGE_COUNT) {                          \
        for (uint32_t __i = 0; __i < (node)->in_edge_array->used; __i++) {   \
            rs_edge_t *ITER_EDGE = &(graph)->edges[                          \
                (node)->in_edge_array->ptr[__i]                              \
            ];                                                             \
            codeblock                                                      \
        }                                                                  \
    } else {                                                               \
        for (uint32_t __i = 0; __i < DIRECT_IN_EDGE_COUNT; __i++) {        \
            if (!((node)->in_edges[__i] & 0x80000000)) {                     \
                rs_edge_t *ITER_EDGE = &(graph)->edges[(node)->in_edges[__i]]; \
                codeblock                                                  \
            }                                                              \
        }                                                                  \
    }                                                                      \
} while (0)

#define RS_GRAPH_ITER_FANOUT(graph, node, codeblock) do {                   \
    if ((node)->out_degree > DIRECT_OUT_EDGE_COUNT) {                         \
        for (uint32_t __i = 0; __i < (node)->out_edge_array->used; __i++) {   \
            rs_edge_t *ITER_EDGE = &(graph)->edges[                           \
                (node)->out_edge_array->ptr[__i]                              \
            ];                                                              \
            codeblock                                                       \
        }                                                                   \
    } else {                                                                \
        for (uint32_t __i = 0; __i < DIRECT_OUT_EDGE_COUNT; __i++) {        \
            if (!((node)->out_edges[__i] & 0x80000000)) {                     \
                rs_edge_t *ITER_EDGE = &(graph)->edges[(node)->out_edges[__i]]; \
                codeblock                                                   \
            }                                                               \
        }                                                                   \
    }                                                                       \
} while (0)

#endif // GRAPH_H
