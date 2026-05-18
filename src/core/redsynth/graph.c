#include "graph.h"

#define MAX_NODES (1<<20)
#define MAX_EDGES (1<<22)

////////////////////////////
//                        //
// MASTER GRAPH FUNCTIONS //
//                        //
////////////////////////////

rs_graph_t *rs_graph_new(void) {
    rs_graph_t *graph = malloc(sizeof(rs_graph_t));
    
    graph->nodes_used = 0;
    graph->nodes = malloc(sizeof(rs_node_t) * MAX_NODES); // 64 MB

    graph->edges_used = 0;
    graph->edges = malloc(sizeof(rs_edge_t) * MAX_EDGES); // 48 MB

    return graph;
}

static inline void rs_graph_free_node(rs_node_t *node) {
    if (node->valid) {
        if (node->in_degree > DIRECT_IN_EDGE_COUNT) {
            free(node->in_edge_array->ptr);
            free(node->in_edge_array);
        }
        if (node->out_degree > DIRECT_OUT_EDGE_COUNT) {
            free(node->out_edge_array->ptr);
            free(node->out_edge_array);
        }
    }
}

void rs_graph_free(rs_graph_t *graph) {
    for (uint32_t i = 0; i < graph->nodes_used; i++) {
        rs_graph_free_node(&graph->nodes[i]);
    }
    free(graph->nodes);
    free(graph->edges);
    free(graph);
}

/////////////////////
//                 //
// EDGE MANAGEMENT //
//                 //
/////////////////////

static inline void edge_init(rs_edge_t *edge) {
    edge->valid = false;
}

rs_edge_t *rs_graph_alloc_edge(rs_graph_t *graph) {
    if (graph->edges_used == MAX_EDGES) {
        for (uint32_t i = 0; i < MAX_EDGES; i++) {
            if (!graph->edges[i].valid) {
                // Found invalid edge to reallocate
                edge_init(&graph->edges[i]);
                return &graph->edges[i];
            }
        }
        return NULL; // All entries full
    }
    edge_init(&graph->edges[graph->edges_used]);
    return &graph->edges[graph->edges_used++];
}

static inline void add_in_edge(rs_graph_t *graph, rs_node_t *target, rs_edge_t *edge) {
    // The most significant part of the logic here is managing the occasional use of
    // an rs_edge_array_t, specifically when the edge count exceeds the number of
    // immediately stored edges in the node.
    
    uint16_t fan_in = target->in_degree++; // preemptively increase stored fanout

    uint32_t edge_idx = edge - graph->edges;

    if (fan_in < DIRECT_IN_EDGE_COUNT) {
        // We can afford to store an additional output in the node directly.

        for (uint32_t i = 0; i < DIRECT_IN_EDGE_COUNT; i++) {
            if (target->in_edges[i] & 0x80000000u) {
                target->in_edges[i] = edge_idx;
                return;
            }
        }
        /* unreachable */

    } else if (fan_in == DIRECT_IN_EDGE_COUNT) {
        // The interesting case. Migrate entries to an rs_edge_array_t.

        rs_edge_array_t *edge_array = malloc(sizeof(rs_edge_array_t));
        edge_array->ptr = malloc(sizeof(uint32_t)*INITIAL_EDGE_ARRAY_SIZE);
        edge_array->size = INITIAL_EDGE_ARRAY_SIZE;
        edge_array->used = fan_in + 1;

        // Populate edge array
        memcpy(edge_array->ptr, &target->in_edges, sizeof(uint32_t)*DIRECT_IN_EDGE_COUNT);
        edge_array->ptr[DIRECT_IN_EDGE_COUNT] = edge_idx;

        target->in_edge_array = edge_array;

    } else if (fan_in > DIRECT_IN_EDGE_COUNT) {
        // Edges are already stored in an edge array.

        // Potentially resize edge array
        if (target->in_edge_array->used == target->in_edge_array->size) {
            // Resize edge array
            uint32_t old_size = target->in_edge_array->size;
            target->in_edge_array->size = old_size * 2;
            target->in_edge_array->ptr = realloc(
                target->in_edge_array->ptr,
                target->in_edge_array->size * sizeof(uint32_t)
            );
        }

        target->in_edge_array->ptr[target->in_edge_array->used++] = edge_idx;
    }
}

static inline void add_out_edge(rs_graph_t *graph, rs_node_t *source, rs_edge_t *edge) {
    // The most significant part of the logic here is managing the occasional use of
    // an rs_edge_array_t, specifically when the edge count exceeds the number of
    // immediately stored edges in the node.
    
    uint16_t fanout = source->out_degree++; // preemptively increase stored fanout

    uint32_t edge_idx = edge - graph->edges;

    if (fanout < DIRECT_OUT_EDGE_COUNT) {
        // We can afford to store an additional output in the node directly.

        for (uint32_t i = 0; i < DIRECT_OUT_EDGE_COUNT; i++) {
            if (source->out_edges[i] & 0x80000000u) {
                source->out_edges[i] = edge_idx;
                return;
            }
        }
        /* unreachable */

    } else if (fanout == DIRECT_OUT_EDGE_COUNT) {
        // The interesting case. Migrate entries to an rs_edge_array_t.

        rs_edge_array_t *edge_array = malloc(sizeof(rs_edge_array_t));
        edge_array->ptr = malloc(sizeof(uint32_t)*INITIAL_EDGE_ARRAY_SIZE);
        edge_array->size = INITIAL_EDGE_ARRAY_SIZE;
        edge_array->used = fanout + 1;

        // Populate edge array
        memcpy(edge_array->ptr, &source->out_edges, sizeof(uint32_t)*DIRECT_OUT_EDGE_COUNT);
        edge_array->ptr[DIRECT_OUT_EDGE_COUNT] = edge_idx;

        source->out_edge_array = edge_array;

    } else if (fanout > DIRECT_OUT_EDGE_COUNT) {
        // Edges are already stored in an edge array.

        // Potentially resize edge array
        if (source->out_edge_array->used == source->out_edge_array->size) {
            // Resize edge array
            uint32_t old_size = source->out_edge_array->size;
            source->out_edge_array->size = old_size * 2;
            source->out_edge_array->ptr = realloc(
                source->out_edge_array->ptr,
                source->out_edge_array->size * sizeof(uint32_t)
            );
            for (uint32_t i = old_size; i < old_size * 2; i++) {
                source->out_edge_array->ptr[i] = 0x80000000u;
            }
        }

        source->out_edge_array->used++;

        for (uint32_t i = 0; i < source->out_edge_array->used; i++) {
            if (source->out_edge_array->ptr[i] & 0x80000000u) {
                source->out_edge_array->ptr[i] = edge_idx;
                return;
            }
        }
        /* unreachable */
    }
}

rs_edge_t *rs_graph_add_edge(rs_graph_t *graph, rs_node_t *source, rs_node_t *target) {
    rs_edge_t *edge = rs_graph_alloc_edge(graph);
    edge->source = source - graph->nodes;
    edge->target = target - graph->nodes;
    add_out_edge(graph, source, edge);
    add_in_edge(graph, target, edge);
    return edge;
}

static inline void remove_in_edge(rs_graph_t *graph, rs_edge_t *edge) {
    rs_node_t *target = &graph->nodes[edge->target];
    uint16_t new_fan_in = --target->in_degree;
    uint32_t edge_idx = edge - graph->edges;

    if (new_fan_in < DIRECT_IN_EDGE_COUNT) {
        for (uint32_t i = 0; i < DIRECT_IN_EDGE_COUNT; i++) {
            if (target->in_edges[i] == edge_idx) {
                target->in_edges[i] = 0x80000000u;
                return;
            }
        }
        /* unreachable */
    } else {
        // Delete from edge array
        for (uint32_t i = 0; i < target->in_edge_array->used; i++) {
            if (target->in_edge_array->ptr[i] == edge_idx) {
                target->in_edge_array->ptr[i] = target->in_edge_array->ptr[
                    --target->in_edge_array->used
                ];
                break;
            }
        }

        if (new_fan_in == DIRECT_IN_EDGE_COUNT) {
            // Delete edge array + move back to in-node edge list
            rs_edge_array_t *edge_array = target->in_edge_array;
            memcpy(target->in_edges, edge_array->ptr, DIRECT_IN_EDGE_COUNT * sizeof(uint32_t));
            free(edge_array->ptr);
            free(edge_array);
        }
    }
}

static inline void remove_out_edge(rs_graph_t *graph, rs_edge_t *edge) {
    rs_node_t *source = &graph->nodes[edge->source];
    uint16_t new_fan_out = --source->out_degree;
    uint32_t edge_idx = edge - graph->edges;

    if (new_fan_out < DIRECT_OUT_EDGE_COUNT) {
        for (uint32_t i = 0; i < DIRECT_OUT_EDGE_COUNT; i++) {
            if (source->out_edges[i] == edge_idx) {
                source->out_edges[i] = 0x80000000u;
                return;
            }
        }
        /* unreachable */
    } else {
        // Delete from edge array
        for (uint32_t i = 0; i < source->out_edge_array->used; i++) {
            if (source->out_edge_array->ptr[i] == edge_idx) {
                source->out_edge_array->ptr[i] = source->out_edge_array->ptr[
                    --source->out_edge_array->used
                ];
                break;
            }
        }

        if (new_fan_out == DIRECT_OUT_EDGE_COUNT) {
            // Delete edge array + move back to in-node edge list
            rs_edge_array_t *edge_array = source->out_edge_array;
            memcpy(source->out_edges, edge_array->ptr, DIRECT_OUT_EDGE_COUNT * sizeof(uint32_t));
            free(edge_array->ptr);
            free(edge_array);
        }
    }
}

void rs_graph_delete_edge(rs_graph_t *graph, rs_edge_t *edge) {
    remove_in_edge(graph, edge);
    remove_out_edge(graph, edge);
    edge->valid = false;
}

/////////////////////
//                 //
// NODE MANAGEMENT //
//                 //
/////////////////////

static inline void node_init(rs_node_t *node) {
    node->valid = true;
    node->in_degree = 0;
    node->out_degree = 0;
    for (int i = 0; i < DIRECT_IN_EDGE_COUNT; i++) {
        node->in_edges[i] = 0x80000000u;
    }
    for (int i = 0; i < DIRECT_OUT_EDGE_COUNT; i++) {
        node->out_edges[i] = 0x80000000u;
    }
}

rs_node_t *rs_graph_alloc_node(rs_graph_t *graph) {
    if (graph->nodes_used == MAX_NODES) {
        for (uint32_t i = 0; i < MAX_NODES; i++) {
            if (!graph->nodes[i].valid) {
                // Found invalid node to reallocate
                node_init(&graph->nodes[i]);
                return &graph->nodes[i];
            }
        }
        return NULL; // All entries full
    }
    node_init(&graph->nodes[graph->nodes_used]);
    return &graph->nodes[graph->nodes_used++];
}

rs_node_t *rs_graph_get_node_at(rs_graph_t *graph, uint16_t x, uint16_t y, uint16_t z) {
    for (uint32_t i = 0; i < graph->nodes_used; i++) {
        rs_node_t *node = &graph->nodes[i];
        if (node->valid && node->x == x && node->y == y && node->z == z) {
            return node;
        }
    }
    return NULL;
}

void rs_graph_delete_node(rs_graph_t *graph, rs_node_t *node) {
    RS_GRAPH_ITER_FAN_IN(graph, node, {
        remove_out_edge(graph, ITER_EDGE);
        ITER_EDGE->valid = false;
    });
    RS_GRAPH_ITER_FANOUT(graph, node, {
        remove_in_edge(graph, ITER_EDGE);
        ITER_EDGE->valid = false;
    });
    rs_graph_free_node(node);
    node->valid = false;
}

////////////////////
//                //
// MAIN / TESTING //
//                //
////////////////////

static void prettyprint_graph(rs_graph_t *graph) {
    for (uint32_t i = 0; i < graph->nodes_used; i++) {
        if (graph->nodes[i].valid) {
            printf("\033[35m%08x\033[0m -> [ \033[2m", i);
            RS_GRAPH_ITER_FANOUT(graph, &graph->nodes[i], {
                printf("%08x ", ITER_EDGE->target);
            });
            printf("\033[0m]\n");
        }
    }
}

int graph_main() {
    rs_graph_t *graph = rs_graph_new();
    rs_node_t *node1 = rs_graph_alloc_node(graph);
    rs_node_t *node2 = rs_graph_alloc_node(graph);
    rs_node_t *node3 = rs_graph_alloc_node(graph);
    rs_node_t *node4 = rs_graph_alloc_node(graph);
    rs_node_t *node5 = rs_graph_alloc_node(graph);
    rs_node_t *node6 = rs_graph_alloc_node(graph);
    rs_node_t *node7 = rs_graph_alloc_node(graph);
    rs_node_t *node8 = rs_graph_alloc_node(graph);
    rs_edge_t *edge1 = rs_graph_add_edge(graph, node1, node2);
    rs_edge_t *edge2 = rs_graph_add_edge(graph, node1, node3);
    rs_edge_t *edge3 = rs_graph_add_edge(graph, node1, node4);
    rs_edge_t *edge4 = rs_graph_add_edge(graph, node1, node5);
    rs_edge_t *edge5 = rs_graph_add_edge(graph, node1, node6);
    rs_edge_t *edge6 = rs_graph_add_edge(graph, node1, node7);
    rs_edge_t *edge7 = rs_graph_add_edge(graph, node1, node8);
    rs_edge_t *edge8 = rs_graph_add_edge(graph, node2, node4);
    rs_edge_t *edge9 = rs_graph_add_edge(graph, node2, node3);
    rs_edge_t *edge10 = rs_graph_add_edge(graph, node4, node7);
    rs_edge_t *edge11 = rs_graph_add_edge(graph, node6, node4);
    rs_edge_t *edge12 = rs_graph_add_edge(graph, node3, node5);
    prettyprint_graph(graph);
    rs_graph_delete_node(graph, node2);
    rs_graph_delete_edge(graph, edge5);

    printf("----\n");

    prettyprint_graph(graph);

    (void)node1;
    (void)node2;
    (void)node3;
    (void)node4;
    (void)node5;
    (void)node6;
    (void)node7;
    (void)node8;
    (void)edge1;
    (void)edge2;
    (void)edge3;
    (void)edge4;
    (void)edge5;
    (void)edge6;
    (void)edge7;
    (void)edge8;
    (void)edge9;
    (void)edge10;
    (void)edge11;
    (void)edge12;

    rs_graph_free(graph);
    return 0;
}
