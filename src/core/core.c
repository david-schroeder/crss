#include "core.h"

static char *world_pos_serializer(void *data) {
    return strdup((char *)data);
}

int crss_core_run() {
    char *fnpath = "core";

    LVERBOSE("Setting up sockets...");

    CONNECT_TO_CMD_BROADCAST();
    SUBSCRIBE_TO_CMD("core");
    SUBSCRIBE_TO_CMD("exit");
    SUBSCRIBE_TO_CMD("quit");
    SUBSCRIBE_TO_CMD("plot");

    LVERBOSE("Notifying Logger...");
    logger_notify("Core");

    LVERBOSE("Setting up plotworld...");
    crss_plot_manager_t *plotworld = new_plot_manager();

    LVERBOSE("Entering command park loop...");
    RUN_CMD_HANDLER({}, {
        DLDEBUG("Handling command '%s'!", RECEIVED_CMD);
        HANDLE_COMMAND("exit", {
            free_plot_manager(plotworld);
            EXIT_CMD_HANDLER();
        })
        HANDLE_COMMAND("quit", {
            free_plot_manager(plotworld);
            EXIT_CMD_HANDLER();
        })
        HANDLE_COMMAND("plot new", {
            if (RECEIVED_CMD_ARGC == 3) {
                // by name
                uuid_t owner;
                set_uuid_from_uint32(&owner, 0, 0, 0, 0);
                crss_plot_t *new = new_plot_from_name(
                    plotworld,
                    &owner,
                    RECEIVED_CMD_ARGV[2],
                    PLOT_SIZE_DEFAULT
                );
                if (new) {
                    char *serial = serialize_plot(new);
                    LINFO("Created new plot \"\033[1m%s\033[0m\"", serial);
                    free(serial);
                }
            } else if (RECEIVED_CMD_ARGC == 4) {
                // by name with size
                if (atoi(RECEIVED_CMD_ARGV[3]) == 0) LFATAL("Cannot create plot with size 0!");
                else {
                    uuid_t owner;
                    set_uuid_from_uint32(&owner, 0, 0, 0, 0);
                    crss_plot_t *new = new_plot_from_name(
                        plotworld,
                        &owner,
                        RECEIVED_CMD_ARGV[2],
                        atoi(RECEIVED_CMD_ARGV[3])
                    );
                    if (new) {
                        char *serial = serialize_plot(new);
                        LINFO("Created new plot \"\033[1m%s\033[0m\"", serial);
                        free(serial);
                    }
                }
            } else if (RECEIVED_CMD_ARGC == 5) {
                // by position
                uuid_t owner;
                set_uuid_from_uint32(&owner, 0, 0, 0, 0);
                crss_plot_t *new = new_plot_from_pos(
                    plotworld,
                    &owner,
                    RECEIVED_CMD_ARGV[2],
                    PLOT_SIZE_DEFAULT,
                    atoi(RECEIVED_CMD_ARGV[3]),
                    atoi(RECEIVED_CMD_ARGV[4])
                );
                if (new) {
                    char *serial = serialize_plot(new);
                    LINFO("Created new plot \"\033[1m%s\033[0m\"", serial);
                    free(serial);
                }
            } else if (RECEIVED_CMD_ARGC == 6) {
                // by position with size
                if (atoi(RECEIVED_CMD_ARGV[5]) == 0) {
                    LFATAL("Invalid size '%s'", RECEIVED_CMD_ARGV[5]);
                } else {
                    uuid_t owner;
                    set_uuid_from_uint32(&owner, 0, 0, 0, 0);
                    crss_plot_t *new = new_plot_from_pos(
                        plotworld,
                        &owner,
                        RECEIVED_CMD_ARGV[2],
                        atoi(RECEIVED_CMD_ARGV[5]),
                        atoi(RECEIVED_CMD_ARGV[3]),
                        atoi(RECEIVED_CMD_ARGV[4])
                    );
                    if (new) {
                        char *serial = serialize_plot(new);
                        LINFO("Created new plot \"\033[1m%s\033[0m\"", serial);
                        free(serial);
                    }
                }
            } else {
                LFATAL("Unknown format for `plot new`!");
                LINFO("Usage: plot new <name> [<x> <z> [<size>]]");
                LINFO("       plot new <name> <size>");
            }
        })
        HANDLE_COMMAND("plot list", {
            /* Log information about all plots */
            LINFO("Plot manager: \033[32mPlots:\033[0m");
            prettyprint_hashmap(fnpath, plotworld->plots, (char *(*)(void *))serialize_plot);
            LINFO("Plot manager: \033[32mPosition to plot name map:\033[0m");
            prettyprint_hashmap(fnpath, plotworld->world_positions, world_pos_serializer);
        })
        HANDLE_COMMAND("plot info", {
            /* Log information about a specific plot */
        })
    })

    CMD_HANDLER_CLEANUP();

    return 0;
}
