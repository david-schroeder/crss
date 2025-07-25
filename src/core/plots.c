#include "plots.h"

crss_plot_manager_t *new_plot_manager() {
    crss_plot_manager_t *mgr = malloc(sizeof(crss_plot_manager_t));
    hashmap_t *map = new_hashmap();
    hashmap_t *wpmap = new_hashmap();
    mgr->plots = map;
    mgr->world_positions = wpmap;
    return mgr;
}

typedef struct {
    int16_t x;
    int16_t z;
} plotpos_t;

/*
@return false if there was no free position, true on success
*/
static bool find_free_position(crss_plot_manager_t *mgr, plotpos_t *pos) {
    // Plot 0; 0 is always taken and holds the server spawn
    // So continually increase radius and search in a spiral around 0; 0
    int16_t x, z;
    char pos_str[10];
    pos_str[9] = '\0';
    uint8_t radius = 1;
    bool found_location = false;
    while (radius > 0 && !found_location) {
        for (uint16_t step = 0; step < 2*radius; step++) {
            /* South border */
            x = radius - step;
            z = radius;
            sprintf(pos_str, "%d;%d", x, z);
            if (hashmap_get_item(mgr->world_positions, pos_str) == NULL) {
                // Found empty position
                found_location = true;
                break;
            }
            /* West border */
            x = -radius;
            z = radius - step;
            sprintf(pos_str, "%d;%d", x, z);
            if (hashmap_get_item(mgr->world_positions, pos_str) == NULL) {
                found_location = true;
                break;
            }
            /* North border */
            x = -radius + step;
            z = -radius;
            sprintf(pos_str, "%d;%d", x, z);
            if (hashmap_get_item(mgr->world_positions, pos_str) == NULL) {
                found_location = true;
                break;
            }
            /* East border */
            x = radius;
            z = -radius + step;
            sprintf(pos_str, "%d;%d", x, z);
            if (hashmap_get_item(mgr->world_positions, pos_str) == NULL) {
                found_location = true;
                break;
            }
        }
        radius++;
    }
    if (radius == 0) return false;
    pos->x = x;
    pos->z = z;
    return true;
}

void free_plot_manager(crss_plot_manager_t *mgr) {
    hashmap_map(mgr->plots, (void(*)(void*))free_plot);
    hashmap_map(mgr->world_positions, free);
    free_hashmap(mgr->plots);
    free_hashmap(mgr->world_positions);
    free(mgr);
}

crss_plot_t *new_plot() {
    crss_plot_t *plot = malloc(sizeof(crss_plot_t));
    plot->name = NULL;
    plot->virt_x = 0;
    plot->virt_z = 0;
    set_uuid_from_uint32(&plot->owner_uuid, 0, 0, 0, 0);
    plot->size = PLOT_SIZE_DEFAULT;
    plot->editor_uuids = NULL;
    plot->num_editors = 0;
    plot->any_can_edit = false;
    return plot;
}

crss_plot_t *new_plot_from_name(crss_plot_manager_t *mgr, uuid_t *owner, char *name, uint8_t size) {
    char *fnpath = "core.plot.create_plot";

    LINFO("Creating plot '%s'!", name);

    LDEBUG("Checking for existing plot with the same name...");

    /* Check for existing plot */
    crss_plot_t *existing_plot = hashmap_get_item(mgr->plots, name);
    if (existing_plot != NULL) {
        // Plot already exists -> error
        LFATAL("Plot with the name '%s' already exists!", name);
        return NULL;
    }
    
    LDEBUG("Finding a free position...");

    /* Find free position */
    plotpos_t pos;
    if (!find_free_position(mgr, &pos)) {
        LFATAL("No free plots to allocate!");
        return NULL;
    }

    LDEBUG("Initializing plot...");

    /* Create new plot */
    crss_plot_t *new = new_plot();
    new->name = strdup(name);
    new->virt_x = pos.x;
    new->virt_z = pos.z;
    copy_uuid(&new->owner_uuid, owner);
    new->size = size;

    LDEBUG("Registering plot with manager...");

    /* Update manager */
    hashmap_update_item(mgr->plots, name, new);
    char pos_string[10];
    pos_string[9] = '\0';
    sprintf(pos_string, "%d;%d", pos.x, pos.z);
    hashmap_update_item(mgr->world_positions, pos_string, strdup(name));

    return new;
}

crss_plot_t *new_plot_from_pos(crss_plot_manager_t *mgr, uuid_t *owner, char *name, uint8_t size, int32_t x, int32_t z) {
    char *fnpath = "core.plot.create_plot";

    LINFO("Creating plot '%s' at (%d; %d)!", name, x, z);

    LDEBUG("Checking for existing plot with the same name...");

    /* Check for existing plot name */
    crss_plot_t *existing_plot;
    existing_plot = hashmap_get_item(mgr->plots, name);
    if (existing_plot != NULL) {
        // Plot already exists -> error
        LFATAL("Plot with the name '%s' already exists!", name);
        return NULL;
    }

    LDEBUG("Checking for existing plot at requested position...");

    /* Check for existing plot at requested position */
    char pos_string[10];
    pos_string[9] = '\0';
    sprintf(pos_string, "%d;%d", x, z);
    existing_plot = hashmap_get_item(mgr->world_positions, pos_string);
    if (existing_plot != NULL) {
        LFATAL("Plot '%s' already exists at the requested position!", existing_plot);
        return NULL;
    }

    LDEBUG("Initializing plot...");

    /* Create new plot */
    crss_plot_t *new = new_plot();
    new->name = strdup(name);
    new->virt_x = x;
    new->virt_z = z;
    copy_uuid(&new->owner_uuid, owner);
    new->size = size;

    LDEBUG("Registering plot with manager...");

    /* Update manager */
    hashmap_update_item(mgr->plots, name, new);
    hashmap_update_item(mgr->world_positions, pos_string, strdup(name));

    return new;
}

char *serialize_plot(crss_plot_t *plot) {
    #define SERIAL_MAX_LEN 256
    char *serial = calloc(SERIAL_MAX_LEN, sizeof(char));
    snprintf(serial, SERIAL_MAX_LEN - 1,
        "%dx%d Plot '%s' @ (%d;%d) [%s editable by %u]",
        plot->size << 4, plot->size << 4,
        plot->name,
        plot->virt_x,
        plot->virt_z,
        plot->any_can_edit ? "Explicitly" : "Only",
        plot->num_editors
    );
    return serial;
}

void free_plot(crss_plot_t *plot) {
    free(plot->name);
    free(plot->editor_uuids);
    free(plot);
}
