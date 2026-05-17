#include "cmdgraph.h"

static void init_flags(cmdgraph_node_t *node, cmdnode_type_e type) {
    switch (type) {
        case NT_Root:
            node->flags |= CMD_FLAG_NT_ROOT;
            break;
        case NT_Argument:
            node->flags |= CMD_FLAG_NT_ARGUMENT;
            break;
        case NT_Literal:
            node->flags |= CMD_FLAG_NT_LITERAL;
            break;
        default:
            break;
    }
}

cmdgraph_node_t *cmdgraph_node_new(const char *name, cmdnode_type_e type, bool is_executable) {
    cmdgraph_node_t *node = malloc(sizeof(cmdgraph_node_t));
    node->child_count = 0;
    node->children = NULL;
    node->redirect_node = NULL;
    node->name = name;
    node->type = type;
    node->flags = is_executable ? CMD_FLAG_EXECUTABLE : 0x00;
    init_flags(node, type);
    node->parser = P_Brigadier_Int;
    node->suggestions_types = ST_Ask_Server;
    return node;
}

void cmdgraph_node_free(cmdgraph_node_t *node) {
    for (int i = 0; i < node->child_count; i++) {
        cmdgraph_node_free(node->children[i]);
    }
    free(node);
}

void cmdgraph_init_argument_node(cmdgraph_node_t *node, cmd_parser_e parser, cmd_parser_properties_t *props) {
    node->type = NT_Argument;
    init_flags(node, NT_Argument);
    node->parser = parser;
    memcpy(&node->parser_props, props, sizeof(cmd_parser_properties_t));
}

void cmdgraph_node_add_child(cmdgraph_node_t *parent, cmdgraph_node_t *child) {
    parent->children = realloc(parent->children, (++parent->child_count)*sizeof(cmdgraph_node_t *));
    parent->children[parent->child_count-1] = child;
}

void cmdgraph_node_redirect(cmdgraph_node_t *node, cmdgraph_node_t *to) {
    node->flags |= CMD_FLAG_REDIRECT;
    node->redirect_node = to;
}

void cmdgraph_add_suggestions_type(cmdgraph_node_t *node, cmd_suggestions_type_e type) {
    node->flags |= CMD_FLAG_SUGGESTION_TYPE;
    node->suggestions_types = type;
}

uint32_t cmdgraph_get_node_count(cmdgraph_node_t *root) {
    uint32_t sum = root->child_count;
    for (uint32_t i = 0; i < root->child_count; i++) {
        sum += cmdgraph_get_node_count(root->children[i]);
    }
    return sum;
}
