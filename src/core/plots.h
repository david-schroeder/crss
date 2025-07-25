#ifndef PLOTS_H
#define PLOTS_H

#include "utils.h"
#include "internals.h"
#include "hashmap.h"
#include "coretypes.h"
#include "ctconfig.h"

/*
CRSS Plot Manager.

Provides utilities for tracking plots, their redstone forests (= multiple graphs) and their respective data.
*/

typedef struct {
    /* Meta- and Identification data */
    char *name;
    int16_t virt_x;
    int16_t virt_z;
    uuid_t owner_uuid;
    uint8_t size;

    /* Permissions */
    uuid_t *editor_uuids;
    uint32_t num_editors;
    bool any_can_edit;

    /* Data */
    // TODO
} crss_plot_t;

typedef struct {
    hashmap_t *plots;
    hashmap_t *world_positions; // Map of "x;z" strings to plot names
} crss_plot_manager_t;

typedef struct {
    uint64_t length;
    uint8_t *data;
} crss_binary_blob_t;

/* Create a new plot manager */
crss_plot_manager_t *new_plot_manager();

void free_plot_manager(crss_plot_manager_t *mgr);

/* Create a new plot in memory */
crss_plot_t *new_plot();

/*
Create a new plot in a manager

@param mgr Plot Manager to create plot in
@param owner UUID of the player creating the plot (0 for server-owned plot)
@param name Unique identifier of the plot
@param size Side length of the plot in multiples of 16
@return Generated plot object or NULL if the name is taken
*/
crss_plot_t *new_plot_from_name(crss_plot_manager_t *mgr, uuid_t *owner, char *name, uint8_t size);

/*
Create a new plot in a manager at a position

@param mgr Plot Manager to create plot in
@param owner UUID of the player creating the plot (0 for server-owned plot)
@param name Unique identifier of the plot
@param size Side length of the plot in multiples of 16
@param x Requested X position
@param z Requested X position
@return Generated plot object or NULL if either the name or the position is taken
*/
crss_plot_t *new_plot_from_pos(crss_plot_manager_t *mgr, uuid_t *owner, char *name, uint8_t size, int32_t x, int32_t z);

/*
Get a string representation of a plot.

@return Plot in serialized string form, must be freed.
*/
char *serialize_plot(crss_plot_t *plot);

void free_plot(crss_plot_t *plot);

crss_binary_blob_t *dump_plotworld(crss_plot_manager_t *mgr);

#endif // PLOTS_H
