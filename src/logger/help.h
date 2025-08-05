#ifndef CRSS_HELP_H
#define CRSS_HELP_H

#include "utils.h"
#include "internals.h"
#include "table.h"
#include "resources.h"
#include "settings.h"

/*
Utility for command help.

Extends `table.h` by "subclassing" the table struct to a command help table.

Defines a command tree structure, where each child node is a subcommand of the parent node.

Launches a command handler thread solely for processing the help command.
*/

typedef struct {
    const char *option;
    const char *description;
    const char *aliases;
} command_option_t;

struct command_;

typedef struct command_ {
    /* Command info */
    const char *name; // ONLY name of the subcommand, i.e. not the full name
    const char *aliases;
    const char *usage;
    const char *description;
    /* Options */
    command_option_t **options;
    uint32_t n_options;
    /* Command tree fields */
    struct command_ *parent; // NULL when root
    struct command_ **subcommands;
    uint32_t n_subcommands;
} command_t;

command_t *new_command(const char *name, const char *description, const char *aliases, const char *usage);

command_t *register_subcommand(command_t *parent, const char *name, const char *description, const char *aliases, const char *usage);

void register_option(command_t *cmd, const char *name, const char *description, const char *aliases);

void free_command(command_t *cmd);

/*
Get a string containing a table displaying a command, its subcommands and all options pertaining to the listed commands.

It has columns named Command, Option, Alias, and Description.
*/
char *get_cmd_help_table(command_t *cmd);

int run_help(void);

#endif // CRSS_HELP_H
