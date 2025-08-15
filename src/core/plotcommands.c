#include "plotcommands.h"

#define THROW_WRONG_ARGC(cmd_name, options) { \
    LFATAL("Invalid argument count %u for `" cmd_name "`.", argc); \
    LINFO("Usage: " cmd_name " " options); \
    return; }

#define TRY_GET_PLOT(name) \
    crss_plot_t *plot = hashmap_get_item(mgr->plots, name); \
    if (plot == NULL) { \
        LFATAL("Plot '%s' does not exist!", name); \
        return; \
    }

/*
Get UUID from string representation `str`.

Creates a variable `var` of type `uuid_t` which, if successful, contains the UUID represented as a string in `str`.
*/
#define TRY_GET_UUID(var, str) \
    uuid_t var; \
    if (!string_to_uuid(str, &var)) { \
        LFATAL("'%s' is not a valid UUID!", str); \
        return; \
    }

static char *world_pos_serializer(void *data) {
    return mystrdup((char *)data);
}

void plot_new(crss_plot_manager_t *mgr, uint32_t argc, char **argv) {
    char *fnpath = "core.plotcommands.new";

    uuid_t plot_owner;
    set_uuid_from_uint32(&plot_owner, 0, 0, 0, 0);

    uint8_t plot_size = PLOT_SIZE_DEFAULT;

    crss_plot_t *new = NULL;

    switch (argc) {
        case 4:
            plot_size = atoi(argv[3]);
            if (plot_size == 0) {
                LFATAL("Cannot create a plot with the provided size!");
                return;
            }
        case 3:
            new = new_plot_from_name(
                mgr,
                &plot_owner,
                argv[2],
                plot_size
            );
            break;
        case 6:
            plot_size = atoi(argv[5]);
            if (plot_size == 0) {
                LFATAL("Cannot create a plot with the provided size!");
                return;
            }
        case 5:
            new = new_plot_from_pos(
                mgr,
                &plot_owner,
                argv[2],
                plot_size,
                atoi(argv[3]),
                atoi(argv[4])
            );
            break;
        default:
            THROW_WRONG_ARGC("plot new", "<plotname> [<x> <z>] [<size>]");
    }

    if (new) {
        char *serial = serialize_plot(new);
        LINFO("Created new plot \"\033[1m%s\033[0m\"", serial);
        free(serial);
    }
}

void plot_list(crss_plot_manager_t *mgr, uint32_t argc, char **argv) {
    char *fnpath = "core.plotcommands.list";

    if (argc > 2) THROW_WRONG_ARGC("plot list", "");

    /* Log information about all plots */
    LINFO("Plot manager: \033[32mPlots:\033[0m");
    prettyprint_hashmap(fnpath, mgr->plots, (char *(*)(void *))serialize_plot);
    LINFO("Plot manager: \033[32mPosition to plot name map:\033[0m");
    prettyprint_hashmap(fnpath, mgr->world_positions, world_pos_serializer);
}

void plot_info(crss_plot_manager_t *mgr, uint32_t argc, char **argv) {
    char *fnpath = "core.plotcommands.info";

    uint32_t server_uuid_[4] = { 0, 0, 0, 0 };
    uuid_t *server_uuid = (uuid_t *)server_uuid_;

    if (argc != 3) THROW_WRONG_ARGC("plot info", "<plotname>");

    TRY_GET_PLOT(argv[2]);

    LINFO("\033[32mInformation about plot '\033[1m%s\033[32;0m':\033[0m", argv[2]);

    char *uuid_str;
    bool server_is_owner = uuidcmp(&plot->owner_uuid, server_uuid) == 0;
    uuid_str = server_is_owner
                ? "\033[2m<Server>\033[0m"
                : uuid_to_string(&plot->owner_uuid);
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

typedef enum {
    REL_OWNER,
    REL_TRUST,
    REL_REMOVE
} crss_plot_relation_t;

/*
Establish or modify a relationship between a plot and a UUID.

This can be either an ownership establishment, trusting a player or removing a player from a plot, given by `rel`.
*/
static void plot_config_relate_uuid(char *fnpath, crss_plot_manager_t *mgr, char *plotname, char *uuid_string, crss_plot_relation_t rel) {
    TRY_GET_PLOT(plotname);

    TRY_GET_UUID(rel_uuid, uuid_string);

    uuid_t *match; // For finding UUIDs in trusted player lists

    switch (rel) {
        case REL_OWNER:
            copy_uuid(&plot->owner_uuid, &rel_uuid);
            break;

        case REL_TRUST:
            if (plot->num_editors >= PLOT_MAX_TRUSTED) {
                LFATAL("Cannot trust more than %u users on plot '%s'!", PLOT_MAX_TRUSTED, plotname);
                return;
            }

            /* Check whether user is already trusted */
            match = NULL;
            for (int i = 0; i < plot->num_editors; i++) {
                if (uuidcmp(&plot->editor_uuids[i], &rel_uuid) == 0) {
                    match = &plot->editor_uuids[i];
                }
            }
            if (match) {
                LFATAL("User '%s' is already trusted on plot '%s'!", uuid_string, plotname);
                return;
            }

            plot->num_editors++;
            plot->editor_uuids = realloc(plot->editor_uuids, plot->num_editors*sizeof(uuid_t));
            copy_uuid(&plot->editor_uuids[plot->num_editors-1], &rel_uuid);
            LINFO("Trusted user '%s' on plot '%s'!", uuid_string, plotname);
            break;

        case REL_REMOVE:
            /* Find user in list of trusted users */
            match = NULL;
            for (int i = 0; i < plot->num_editors; i++) {
                if (uuidcmp(&plot->editor_uuids[i], &rel_uuid) == 0) {
                    match = &plot->editor_uuids[i];
                }
            }
            if (!match) {
                LFATAL("User '%s' is not trusted on plot '%s'!", uuid_string, plotname);
                return;
            }

            /* Copy last UUID in editor list to the one being replaced */
            /* Since the order of UUIDs doesn't matter, this removes in O(1) */
            copy_uuid(match, &plot->editor_uuids[--plot->num_editors]);
            /* A realloc is not needed; excessive sizes are prevented via PLOT_MAX_TRUSTED anyways */
            break;

        default:
            LFATAL("Internal error [Unknown plot <-> UUID relation enum]");
            return;
    }
}

void plot_set_owner(crss_plot_manager_t *mgr, uint32_t argc, char **argv) {
    char *fnpath = "core.plotcommands.set_owner";
    
    if (argc != 6) THROW_WRONG_ARGC("plot config set owner", "<plotname> <owner>");

    plot_config_relate_uuid(fnpath, mgr, argv[4], argv[5], REL_OWNER);
}

void plot_trust(crss_plot_manager_t *mgr, uint32_t argc, char **argv) {
    char *fnpath = "core.plotcommands.trust";

    if (argc != 4) THROW_WRONG_ARGC("plot trust", "<plotname> <user>");

    plot_config_relate_uuid(fnpath, mgr, argv[2], argv[3], REL_TRUST);
}

void plot_remove(crss_plot_manager_t *mgr, uint32_t argc, char **argv) {
    char *fnpath = "core.plotcommands.remove";

    if (argc != 4) THROW_WRONG_ARGC("plot remove", "<plotname> <user>");

    plot_config_relate_uuid(fnpath, mgr, argv[2], argv[3], REL_REMOVE);
}

typedef enum {
    CFG_ANYCANEDIT
} plot_boolean_flags_t;

// TODO: extend / modify this to not be exclusive to boolean options
void plot_config_bool(crss_plot_manager_t *mgr, uint32_t argc, char **argv) {
    char *fnpath = "core.plotcommands.config";

    if (argc != 6) THROW_WRONG_ARGC("plot config set", "<plotname> <config> [true|false]");

    TRY_GET_PLOT(argv[3]);

    plot_boolean_flags_t config;

    /* Check which option is selected */
    if (strcmp(argv[4], "anycanedit") == 0) {
        config = CFG_ANYCANEDIT;
    } else {
        LFATAL("Invalid configuration option '%s'. Use `help plot config` for more information.");
        return;
    }

    /* Check value parameter */
    bool state;
    if (strcmp(argv[5], "true") == 0 || strcmp(argv[5], "True") == 0) {
        state = true;
    } else if (strcmp(argv[5], "false") == 0 || strcmp(argv[5], "False") == 0) {
        state = false;
    } else {
        LFATAL("Invalid configuration state '%s'. Try one of true, false.", argv[5]);
        return;
    }

    /* Apply the option change */
    switch (config) {
        case CFG_ANYCANEDIT:
            plot->any_can_edit = state;
            break;
        default:
            /* Unreachable */
            LFATAL("Internal error");
    }
    LINFO("Updated configuration values for plot '%s'.", argv[3]);
}

void plot_place(crss_plot_manager_t *mgr, uint32_t argc, char **argv) {
    char *fnpath = "core.plotcommands.place";

    if (argc != 7) THROW_WRONG_ARGC("plot place", "<plotname> <x> <y> <z> <block>");

    TRY_GET_PLOT(argv[2]);

    uint32_t x = atoi(argv[3]);
    uint32_t y = atoi(argv[4]);
    uint32_t z = atoi(argv[5]);
    uint8_t block = atoi(argv[6]);
    set_block(plot->storage, x, y, z, &block);
    LINFO("Success");
}

void plot_get(crss_plot_manager_t *mgr, uint32_t argc, char **argv) {
    char *fnpath = "core.plotcommands.get";

    if (argc != 6) THROW_WRONG_ARGC("plot get", "<plotname> <x> <y> <z>");

    TRY_GET_PLOT(argv[2]);

    uint32_t x = atoi(argv[3]);
    uint32_t y = atoi(argv[4]);
    uint32_t z = atoi(argv[5]);
    block_t *blk = get_block(plot->storage, x, y, z);

    if (blk == NULL) LINFO("No block at that position.");
    else LINFO("Block: %d", *blk);
}
