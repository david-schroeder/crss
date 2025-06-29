#ifndef GRAPH_H
#define GRAPH_H

#include "utils.h"

typedef enum {
    REPEATER,
    COMPARATOR,
    TORCH,
    DUST,
    LAMP,
    LEVER,
    BUTTON
} rs_nodetype_e;

struct _rs_connection_t;

typedef struct {
    rs_nodetype_e node_type;
    struct _rs_connection_t *back_input;
    struct _rs_connection_t *side_input;
    struct _rs_connection_t *output;
    union {
        bool state; // on/off for binary components (torch/lamp/lever)
        struct {
            bool locked;
            bool output;
        } repeater_state;
        struct {
            bool mode;
            uint8_t output;
        } comparator_state;
    };
    uint32_t ticks;
} rs_node_t;

// aggregate num_inputs incoming rs_node_t's to outgoing
typedef struct _rs_connection_t {
    rs_node_t *incoming;
    rs_node_t *outgoing;
    uint8_t *weights;
    uint8_t num_inputs;
} rs_connection_t;

#endif // GRAPH_H
