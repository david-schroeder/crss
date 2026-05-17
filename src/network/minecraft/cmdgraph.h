#ifndef COMMAND_GRAPH_H
#define COMMAND_GRAPH_H

#include "utils.h"

/*
    Minecraft Command management utility
*/

typedef enum {
    NT_Root,
    NT_Literal,
    NT_Argument
} cmdnode_type_e;

typedef enum {
    // Parsers with properties
    P_Brigadier_Double,
    P_Brigadier_Float,
    P_Brigadier_Int,
    P_Brigadier_Long,
    P_Brigadier_String,
    P_Minecraft_Entity,
    P_Minecraft_Score_Holder,
    P_Minecraft_Range,
    P_Minecraft_Resource_or_Tag,
    P_Minecraft_Resource,
    // Parsers without properties
    P_Brigadier_Bool,
    P_Minecraft_Game_Profile,
    P_Minecraft_Block_Pos,
    P_Minecraft_Column_Pos,
    P_Minecraft_Vec3,
    P_Minecraft_Vec2,
    P_Minecraft_Block_State,
    P_Minecraft_Block_Predicate,
    P_Minecraft_Item_Stack,
    P_Minecraft_Item_Predicate,
    P_Minecraft_Color,
    P_Minecraft_Component,
    P_Minecraft_Message,
    P_Minecraft_NBT,
    P_Minecraft_NBT_Path,
    P_Minecraft_Objective,
    P_Minecraft_Objective_Criteria,
    P_Minecraft_Operation,
    P_Minecraft_Particle,
    P_Minecraft_Rotation,
    P_Minecraft_Angle,
    P_Minecraft_Scoreboard_Slot,
    P_Minecraft_Swizzle,
    P_Minecraft_Team,
    P_Minecraft_Item_Slot,
    P_Minecraft_Resource_Location,
    P_Minecraft_Mob_Effect,
    P_Minecraft_Function,
    P_Minecraft_Entity_Anchor,
    P_Minecraft_Int_Range,
    P_Minecraft_Float_Range,
    P_Minecraft_Item_Enchantment,
    P_Minecraft_Entity_Summon,
    P_Minecraft_Dimension,
    P_Minecraft_UUID,
    P_Minecraft_NBT_Tag,
    P_Minecraft_NBT_Compound_Tag,
    P_Minecraft_Time
} cmd_parser_e;

typedef enum {
    ST_Ask_Server,
    ST_All_Recipes,
    ST_Available_Sounds,
    ST_Available_Biomes,
    ST_Summonable_Entities
} cmd_suggestions_type_e;

typedef uint8_t    cmd_flag_t;
typedef cmd_flag_t cmd_flagset_t;

const cmd_flag_t CMD_FLAG_NT_ROOT         = 0x00;
const cmd_flag_t CMD_FLAG_NT_LITERAL      = 0x01;
const cmd_flag_t CMD_FLAG_NT_ARGUMENT     = 0x02;
const cmd_flag_t CMD_FLAG_EXECUTABLE      = 0x04;
const cmd_flag_t CMD_FLAG_REDIRECT        = 0x08;
const cmd_flag_t CMD_FLAG_SUGGESTION_TYPE = 0x10;

typedef enum {
    SPP_SINGLE_WORD = 0,
    SPP_QUOTABLE_PHRASE = 1,
    SPP_GREEDY_PHRASE = 2
} cmd_strparser_prop_e;

typedef union {
    struct {
        cmd_flagset_t double_flags;
        double double_min;
        double double_max;
    };
    struct {
        cmd_flagset_t float_flags;
        float float_min;
        float float_max;
    };
    struct {
        cmd_flagset_t int_flags;
        int int_min;
        int int_max;
    };
    struct {
        cmd_flagset_t long_flags;
        long long_min;
        long long_max;
    };
    cmd_strparser_prop_e str_props;
    cmd_flagset_t entity_flags;
    cmd_flagset_t score_holder_flags;
    bool range_decimals;
    const char *resource_or_tag_registry;
    const char *resource_registry;
} cmd_parser_properties_t;

typedef struct cmdgraph_node_ {
    // Children
    uint32_t child_count;
    struct cmdgraph_node_ **children;
    struct cmdgraph_node_ *redirect_node;
    // Node-specific data
    cmd_parser_properties_t parser_props;
    const char *name;
    cmdnode_type_e type;
    cmd_flagset_t flags;
    cmd_parser_e parser;
    cmd_suggestions_type_e suggestions_types;
} cmdgraph_node_t;

cmdgraph_node_t *cmdgraph_node_new(const char *name, cmdnode_type_e type, bool is_executable);

void cmdgraph_node_free(cmdgraph_node_t *node); // Recursively free children too

void cmdgraph_init_argument_node(cmdgraph_node_t *node, cmd_parser_e parser, cmd_parser_properties_t *props);

void cmdgraph_node_add_child(cmdgraph_node_t *parent, cmdgraph_node_t *child);

void cmdgraph_node_redirect(cmdgraph_node_t *node, cmdgraph_node_t *to);

void cmdgraph_add_suggestions_type(cmdgraph_node_t *node, cmd_suggestions_type_e type);

uint32_t cmdgraph_get_node_count(cmdgraph_node_t *root);

#endif // COMMAND_GRAPH_H
