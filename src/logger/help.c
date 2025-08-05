#include "help.h"

command_t command_tree_root;

static void init_command(command_t *cmd, const char *name, const char *desc, const char *aliases, const char *usage) {
    cmd->name = name;
    cmd->aliases = aliases;
    cmd->description = desc;
    cmd->usage = usage;
    cmd->options = NULL;
    cmd->n_options = 0;
    cmd->parent = NULL;
    cmd->subcommands = NULL;
    cmd->n_subcommands = 0;
}

command_t *new_command(const char *name, const char *description, const char *aliases, const char *usage) {
    command_t *cmd = malloc(sizeof(command_t));
    init_command(cmd, name, description, aliases, usage);
    return cmd;
}

command_t *register_subcommand(command_t *parent, const char *name, const char *description, const char *aliases, const char *usage) {
    command_t *cmd = new_command(name, description, aliases, usage);
    cmd->parent = parent;
    parent->subcommands = realloc(
        parent->subcommands,
        (++parent->n_subcommands) * sizeof(command_t *)
    );
    parent->subcommands[parent->n_subcommands - 1] = cmd;
    return cmd;
}

void register_option(command_t *cmd, const char *name, const char *description, const char *aliases) {
    command_option_t *opt = malloc(sizeof(command_option_t));
    opt->option = name;
    opt->description = description;
    opt->aliases = aliases;
    cmd->options = realloc(
        cmd->options,
        (++cmd->n_options) * sizeof(command_option_t *)
    );
    cmd->options[cmd->n_options - 1] = opt;
}

void free_command(command_t *cmd) {
    for (int i = 0; i < cmd->n_options; i++) {
        free(cmd->options[i]);
    }
    for (int i = 0; i < cmd->n_subcommands; i++) {
        free_command(cmd->subcommands[i]);
    }
    free(cmd);
}

/*
Helper for `get_cmd_help_table`.
Determines how many rows the table displaying command options for `cmd` will have, including the header row.
*/
static uint16_t count_table_rows(command_t *cmd) {
    uint16_t command_rows, option_rows;
    if (cmd->name == NULL) command_rows = 0;
    else command_rows = 1;
    command_rows += cmd->n_subcommands;
    option_rows = cmd->n_options;

    /* Subcommand option row counting */
    for (int i = 0; i < cmd->n_subcommands; i++) {
        option_rows += cmd->subcommands[i]->n_options;
    }
    return option_rows + command_rows;
}

/*
Get the full name of a node in the command tree.
Is equivalent to all `name` fields of all parent nodes + the current node, joined by spaces.
If `final_fmt` is not NULL, then it is inserted before the name of the current node.
*/
static char *full_command_name(command_t *cmd, const char *final_fmt) {
    /* Assumes that cmd->name is not NULL given non-NULL cmd */
    if (cmd == NULL || cmd->name == NULL) return NULL;
    char *parent_cmd_name = full_command_name(cmd->parent, NULL);
    size_t full_command_name_len = 0;
    if (parent_cmd_name) {
        full_command_name_len = strlen(parent_cmd_name);
    }
    full_command_name_len += strlen(cmd->name) + 2 + (final_fmt ? strlen(final_fmt) : 0); // space, nullptr, format
    char *child_cmd_name = realloc(parent_cmd_name, full_command_name_len);
    if (parent_cmd_name) {
        sprintf(child_cmd_name, "%s %s%s", child_cmd_name, final_fmt ? final_fmt : "", cmd->name);
    } else {
        sprintf(child_cmd_name, "%s%s", final_fmt ? final_fmt : "", cmd->name);
    }
    return child_cmd_name;
}

char *get_cmd_help_table(command_t *cmd) {
    // + 1 in row count is for header row
    table_t *t = new_table(count_table_rows(cmd) + 1, 5, (char *)cmd->name);

    /* Header row*/
    table_set_cell(t, 0, 0, "Command");
    table_set_cell(t, 0, 1, "Option");
    table_set_cell(t, 0, 2, "Usage");
    table_set_cell(t, 0, 3, "Aliases");
    table_set_cell(t, 0, 4, "Description");

    /* Command itself */
    uint8_t current_row = 1;
    if (cmd->name != NULL) {
        // table row 1 is command row, subsequent rows are option rows
        table_set_cell(t, current_row, 0, full_command_name(cmd, "\033[32m"));
        table_set_cell(t, current_row, 1, "-");
        table_set_cell(t, current_row, 2, (char *)cmd->usage);
        table_set_cell(t, current_row, 3, (char *)cmd->aliases);
        table_set_cell(t, current_row, 4, (char *)cmd->description);
        current_row++;
        for (int i = 0; i < cmd->n_options; i++) {
            table_set_cell(t, current_row, 1, (char *)cmd->options[i]->option);
            table_set_cell(t, current_row, 2, "-");
            table_set_cell(t, current_row, 3, (char *)cmd->options[i]->aliases);
            table_set_cell(t, current_row, 4, (char *)cmd->options[i]->description);
            table_no_format_line(t, current_row - 1);
            current_row++;
        }
    }
    /* Subcommands */
    for (int i = 0; i < cmd->n_subcommands; i++) {
        command_t *subcmd = cmd->subcommands[i];
        table_set_cell(t, current_row, 0, full_command_name(subcmd, "\033[32m"));
        table_set_cell(t, current_row, 1, "-");
        table_set_cell(t, current_row, 2, (char *)subcmd->usage);
        table_set_cell(t, current_row, 3, (char *)subcmd->aliases);
        table_set_cell(t, current_row, 4, (char *)subcmd->description);
        current_row++;
        for (int i = 0; i < subcmd->n_options; i++) {
            table_set_cell(t, current_row, 1, (char *)subcmd->options[i]->option);
            table_set_cell(t, current_row, 2, "-");
            table_set_cell(t, current_row, 3, (char *)subcmd->options[i]->aliases);
            table_set_cell(t, current_row, 4, (char *)subcmd->options[i]->description);
            table_no_format_line(t, current_row - 1);
            current_row++;
        }
    }
    char *ret = dump_table(t, true, "\033[0m\033[2m", "\033[0;1m", "\033[0m");
    /* Free command names */
    // Row 0 is header row -> no free
    for (int i = 1; i < t->rows; i++) {
        if (table_get_cell(t, i, 0) != NULL) {
            free(table_get_cell(t, i, 0));
        }
    }
    free_table(t);
    return ret;
}

/*
Gets the full help message that is printed when help is inquired about a `command_t` object.

@cmd Command object to print help on
*/
char *get_full_help_string(command_t *cmd) {
    char *static_help_text = malloc(strlen(RESOURCE_HELP_GENERAL) + 128);

    sprintf(static_help_text, RESOURCE_HELP_GENERAL, SOFTWARE_NAME, VERSION_STRING);

    char *cmd_name = full_command_name(cmd, "\033[32m");
    char *cmd_table = get_cmd_help_table(cmd);
    size_t tabsize = strlen(cmd_table);
    if (cmd_name == NULL) {
        /* could not get command name -> omit it */
        size_t bufsize = tabsize + strlen(static_help_text) + 128;
        char *buf = malloc(bufsize);
        buf[bufsize-1] = '\0';
        buf[0] = '\0';

        sprintf(buf, "\n\033[1m Displaying general command help.\033[0m%s\n%s",
                static_help_text, cmd_table);
        buf = realloc(buf, strlen(buf)+1);

        free(cmd_table);
        free(static_help_text);
        
        return buf;
    }
    size_t cmd_name_sz = strlen(cmd_name);

    size_t bufsize = tabsize + cmd_name_sz + strlen(static_help_text) + 128;
    char *buf = malloc(bufsize);
    buf[bufsize-1] = '\0';
    buf[0] = '\0';

    sprintf(buf, "\n\033[1m Displaying help for command: `%s`\033[0m%s\n%s",
            cmd_name, static_help_text, cmd_table);
    buf = realloc(buf, strlen(buf)+1);
    
    free(cmd_name);
    free(cmd_table);
    free(static_help_text);
    
    return buf;
}

void build_command_tree(void) {
    command_t *c1, *c2, *c3, *c4;
    (void)(c1);
    (void)(c2);
    (void)(c3);
    (void)(c4);

    init_command(&command_tree_root, NULL, NULL, NULL, NULL);

    c1 = register_subcommand(&command_tree_root, "debug", "Set the debug level", "", "<level>");
        register_option(c1, "verbose", "Most verbose log level", "v");
        register_option(c1, "debug", "Debugging messages", "d, debg");
        register_option(c1, "info", "User Information", "i");
        register_option(c1, "warning", "Warnings", "w, warn");
        register_option(c1, "fatal", "Critical errors", "f, fail");

    register_subcommand(&command_tree_root, "quit", "Exit the application", "exit", "");
    
    c1 = register_subcommand(&command_tree_root, "plot", "Manage plots", "", "<option> [<arg>*]");
        c2 = register_subcommand(c1, "new", "Create a new plot", "", "<plotname> [<x> <z>] [<size>]");
        register_subcommand(c1, "list", "Display information about all existing plots", "", "-");
        register_subcommand(c1, "info", "Display information about one plot", "", "<plotname>");
        c2 = register_subcommand(c1, "config", "Configure a plot", "", "[get|set] <option> [<arg>*]");
            c3 = register_subcommand(c2, "set", "Set a configuration parameter", "", "<option> [<arg>*]");
                register_subcommand(c3, "owner", "Set a plot's owner", "", "<plotname> <uuid>");
                register_subcommand(c3, "anycanedit", "Set whether anybody can edit a plot", "", "<plotname> [true|false]");
        register_subcommand(c1, "trust", "Trust a player to a plot", "", "<plotname> <uuid>");
        register_subcommand(c1, "remove", "Remove a player's trust on a plot", "", "<plotname> <uuid>");
}

/*
Get the full string response to a help command.

Equates to a tree traversal of `command_tree_root`, always matching each child node's name to the next argument in `argv`.

Assumes argv[0] is "help" (or any other prefix).

@return Help string, or NULL if the command doesn't exist
*/
char *get_help_cmd_response(int argc, char **argv) {
    int argvptr = 0;
    command_t *cur_cmd = &command_tree_root;
    char *arg;
    bool success = false;

    /* Go through all arguments */
    while (++argvptr < argc) {
        success = false;
        arg = argv[argvptr];

        /* Go through each subcommand of the current cmdtree node */
        /* And see if one matches */
        for (int i = 0; i < cur_cmd->n_subcommands; i++) {
            command_t *subcmd = cur_cmd->subcommands[i];
            /* Match checking */
            if (strcmp(arg, subcmd->name) == 0) {
                cur_cmd = subcmd;
                success = true;
            }
        }
        if (!success) {
            break;
        }
    }

    if (argc == 1) {
        /* cur_cmd is already command_tree_root */
        /* -> print command tree root when plain `help` is executed */
        success = true;
    }

    if (!success) {
        return NULL;
    }

    return get_full_help_string(cur_cmd);
}

int run_help(void) {
    char *fnpath = "help";

    LDEBUG("Building command tree...");
    build_command_tree();

    LVERBOSE("Setting up sockets...");

    CONNECT_TO_CMD_BROADCAST();
    SUBSCRIBE_TO_CMD("help");
    SUBSCRIBE_TO_CMD("exit");
    SUBSCRIBE_TO_CMD("quit");

    LVERBOSE("Notifying Logger...");
    logger_notify("Help");

    LVERBOSE("Entering command park loop...");
    RUN_CMD_HANDLER({}, {
        HANDLE_COMMAND("exit", {
            EXIT_CMD_HANDLER();
        })
        HANDLE_COMMAND("quit", {
            EXIT_CMD_HANDLER();
        })
        HANDLE_COMMAND("help", {
            char *response = get_help_cmd_response(RECEIVED_CMD_ARGC, RECEIVED_CMD_ARGV);
            if (response == NULL) {
                /* RECEIVED_CMD[4] will never be an invalid read because command has to start with 'help' to reach this point */
                LFATAL("The provided command does not exist!");
            } else {
                LINFO("%s", response);
                free(response);
            }
        })
    })

    CMD_HANDLER_CLEANUP();

    return 0;
}
