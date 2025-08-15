#include "core.h"

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
            EXIT_CMD_HANDLER();
        })
        HANDLE_COMMAND("quit", {
            EXIT_CMD_HANDLER();
        })
        HANDLE_COMMAND("plot new", {
            plot_new(plotworld, RECEIVED_CMD_ARGC, RECEIVED_CMD_ARGV);
        })
        else HANDLE_COMMAND("plot list", {
            plot_list(plotworld, RECEIVED_CMD_ARGC, RECEIVED_CMD_ARGV);
        })
        else HANDLE_COMMAND("plot info", {
            plot_info(plotworld, RECEIVED_CMD_ARGC, RECEIVED_CMD_ARGV);
        })
        else HANDLE_COMMAND("plot config set owner", {
            plot_set_owner(plotworld, RECEIVED_CMD_ARGC, RECEIVED_CMD_ARGV);
        })
        else HANDLE_COMMAND("plot config set", {
            plot_config_bool(plotworld, RECEIVED_CMD_ARGC, RECEIVED_CMD_ARGV);
        })
        else HANDLE_COMMAND("plot trust", {
            plot_trust(plotworld, RECEIVED_CMD_ARGC, RECEIVED_CMD_ARGV);
        })
        else HANDLE_COMMAND("plot remove", {
            plot_remove(plotworld, RECEIVED_CMD_ARGC, RECEIVED_CMD_ARGV);
        })
        HANDLE_COMMAND("plot place", {
            plot_place(plotworld, RECEIVED_CMD_ARGC, RECEIVED_CMD_ARGV);
        })
        HANDLE_COMMAND("plot get", {
            plot_get(plotworld, RECEIVED_CMD_ARGC, RECEIVED_CMD_ARGV);
        })
    })

    CMD_HANDLER_CLEANUP();

    free_plot_manager(plotworld);

    return 0;
}
