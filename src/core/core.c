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

    uuid_t __server_uuid;
    memset(__server_uuid, 0, 16);

    // TODO: turn command loop into dispatcher function
    LVERBOSE("Entering command park loop...");
    RUN_CMD_HANDLER({}, {
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
        else HANDLE_COMMAND("plot list", {
            /* Log information about all plots */
            LINFO("Plot manager: \033[32mPlots:\033[0m");
            prettyprint_hashmap(fnpath, plotworld->plots, (char *(*)(void *))serialize_plot);
            LINFO("Plot manager: \033[32mPosition to plot name map:\033[0m");
            prettyprint_hashmap(fnpath, plotworld->world_positions, world_pos_serializer);
        })
        else HANDLE_COMMAND("plot info", {
            /* Log information about a specific plot */
            if (RECEIVED_CMD_ARGC == 3) {
                crss_plot_t *plot = hashmap_get_item(plotworld->plots, RECEIVED_CMD_ARGV[2]);
                if (plot == NULL) {
                    LFATAL("Plot '%s' does not exist!", RECEIVED_CMD_ARGV[2]);
                } else {
                    LINFO("\033[32mInformation about plot '\033[1m%s\033[32;0m':\033[0m", RECEIVED_CMD_ARGV[2]);
                    char *uuid_str;
                    bool server_is_owner = uuidcmp(&plot->owner_uuid, &__server_uuid) == 0;
                    if (server_is_owner) uuid_str = "\033[2m<Server>\033[0m";
                    else uuid_str = uuid_to_string(&plot->owner_uuid);
                    LINFO("    \033[1mPlot Owner:\033[0m %s", uuid_str);
                    if (!server_is_owner) free(uuid_str);
                    LINFO("    \033[1mX Position:\033[0m %d", plot->virt_x);
                    LINFO("    \033[1mZ Position:\033[0m %d", plot->virt_z);
                    LINFO("    \033[1mSide length:\033[0m %u blocks", plot->size << 4);
                    LINFO("    \033[1mEditor count:\033[0m %u", plot->num_editors);
                    if (plot->num_editors > 0) {
                        LINFO("    \033[1mEditor list:\033[0m");
                        for (int i = 0; i < plot->num_editors; i++) {
                            uuid_t *editor_uuid = &plot->editor_uuids[i];
                            char *ed_uuid_str = uuid_to_string(editor_uuid);
                            LINFO("        %d: %s", i, ed_uuid_str);
                            free(ed_uuid_str);
                        }
                    }
                    LINFO("    \033[1mAnybody can edit:\033[0m %s", plot->any_can_edit ? "Yes" : "No");
                }
            } else {
                LFATAL("The 'plot info' command takes exactly one argument: <name>.");
            }
        })
        else HANDLE_COMMAND("plot config set owner", {
            if (RECEIVED_CMD_ARGC == 6) {
                crss_plot_t *plot = hashmap_get_item(plotworld->plots, RECEIVED_CMD_ARGV[4]);
                if (plot == NULL) {
                    LFATAL("Plot '%s' does not exist!", RECEIVED_CMD_ARGV[4]);
                } else {
                    if (string_to_uuid(RECEIVED_CMD_ARGV[5], &plot->owner_uuid)) {
                        LINFO("Updated owner of plot '%s'.", RECEIVED_CMD_ARGV[4]);
                    } else {
                        LFATAL("'%s' is not a valid UUID!", RECEIVED_CMD_ARGV[5]);
                    }
                }
            } else {
                LFATAL("The 'plot config set owner' command takes exactly 2 arguments: <plotname>, <owner-uuid>.");
            }
        })
        else HANDLE_COMMAND("plot config set anycanedit", {
            if (RECEIVED_CMD_ARGC == 6) {
                crss_plot_t *plot = hashmap_get_item(plotworld->plots, RECEIVED_CMD_ARGV[4]);
                if (plot == NULL) {
                    LFATAL("Plot '%s' does not exist!", RECEIVED_CMD_ARGV[4]);
                } else {
                    bool state;
                    bool state_is_valid = false;
                    if (strcmp(RECEIVED_CMD_ARGV[5], "true") == 0) {
                        state_is_valid = true;
                        state = true;
                    }
                    if (strcmp(RECEIVED_CMD_ARGV[5], "false") == 0) {
                        state_is_valid = true;
                        state = false;
                    }
                    if (state_is_valid) {
                        plot->any_can_edit = state;
                        LINFO("Updated global editability for plot '%s'.", RECEIVED_CMD_ARGV[4]);
                    } else {
                        LFATAL("Unrecognized boolean attribute '%s'. Valid values are 'true', 'false'.", RECEIVED_CMD_ARGV[5]);
                    }
                }
            } else {
                LFATAL("The 'plot config set anycanedit' command takes exactly 2 arguments: <plotname>, [true|false].");
            }
        })
        else HANDLE_COMMAND("plot config", {
            LINFO("\033[32mPlot configuration options:");
            LINFO("    \033[1mplot config set owner <plot> <uuid>");
            LINFO("    \033[1mplot config set anycanedit <plot> [true|false]");
        })
        else HANDLE_COMMAND("plot trust", {
            if (RECEIVED_CMD_ARGC == 4) {
                crss_plot_t *plot = hashmap_get_item(plotworld->plots, RECEIVED_CMD_ARGV[2]);
                if (plot == NULL) {
                    LFATAL("Plot '%s' does not exist!", RECEIVED_CMD_ARGV[2]);
                } else {
                    if (plot->num_editors < PLOT_MAX_TRUSTED) {
                        uuid_t uuid;
                        if (string_to_uuid(RECEIVED_CMD_ARGV[3], &uuid)) {
                            uuid_t *match = NULL;
                            for (int i = 0; i < plot->num_editors; i++) {
                                if (uuidcmp(&plot->editor_uuids[i], &uuid) == 0) {
                                    match = &plot->editor_uuids[i];
                                }
                            }
                            if (!match) {
                                plot->num_editors++;
                                plot->editor_uuids = realloc(plot->editor_uuids, plot->num_editors*sizeof(uuid_t));
                                copy_uuid(&plot->editor_uuids[plot->num_editors-1], &uuid);
                                LINFO("Added '%s' as an editor to plot '%s'!", RECEIVED_CMD_ARGV[3], RECEIVED_CMD_ARGV[2]);
                            } else {
                                LFATAL("UUID '%s' is already trusted on plot '%s'!", RECEIVED_CMD_ARGV[3], RECEIVED_CMD_ARGV[2]);
                            }
                        } else {
                            LFATAL("'%s' is not a valid UUID!", RECEIVED_CMD_ARGV[3]);
                        }
                    } else {
                        LFATAL("Cannot trust more than %u users on plot '%s'!", PLOT_MAX_TRUSTED, RECEIVED_CMD_ARGV[2]);
                    }
                }
            } else {
                LFATAL("The 'plot trust' command takes exactly 2 arguments: <plotname>, <uuid>.");
            }
        })
        else HANDLE_COMMAND("plot remove", {
            if (RECEIVED_CMD_ARGC == 4) {
                crss_plot_t *plot = hashmap_get_item(plotworld->plots, RECEIVED_CMD_ARGV[2]);
                if (plot == NULL) {
                    LFATAL("Plot '%s' does not exist!", RECEIVED_CMD_ARGV[2]);
                } else {
                    uuid_t uuid;
                    if (string_to_uuid(RECEIVED_CMD_ARGV[3], &uuid)) {
                        uuid_t *match = NULL;
                        for (int i = 0; i < plot->num_editors; i++) {
                            if (uuidcmp(&plot->editor_uuids[i], &uuid) == 0) {
                                match = &plot->editor_uuids[i];
                            }
                        }
                        if (match) {
                            copy_uuid(match, &plot->editor_uuids[--plot->num_editors]);
                            // realloc does not need to be called; the entire buffer is freed anyways
                            LINFO("Removed '%s' as an editor from plot '%s'!", RECEIVED_CMD_ARGV[3], RECEIVED_CMD_ARGV[2]);
                        } else {
                            LFATAL("UUID '%s' isn't trusted on plot '%s'!", RECEIVED_CMD_ARGV[3], RECEIVED_CMD_ARGV[2]);
                        }
                    } else {
                        LFATAL("'%s' is not a valid UUID!", RECEIVED_CMD_ARGV[3]);
                    }
                }
            } else {
                LFATAL("The 'plot remove' command takes exactly 2 arguments: <plotname>, <uuid>.");
            }
        })
        else HANDLE_COMMAND("plot", {
            LINFO("\033[32mPlot management commands:");
            LINFO("    \033[1mplot config set <option> [<args>...]");
            LINFO("    \033[1mplot trust <plot> <uuid>");
            LINFO("    \033[1mplot remove <plot> <uuid>");
            LINFO("    \033[1mplot list");
            LINFO("    \033[1mplot info <plot>");
        })
    })

    CMD_HANDLER_CLEANUP();

    return 0;
}
