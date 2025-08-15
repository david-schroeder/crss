#ifndef PLOT_COMMANDS_H
#define PLOT_COMMANDS_H

#include "utils.h"
#include "plots.h"

void plot_new(crss_plot_manager_t *mgr, uint32_t argc, char **argv);

void plot_list(crss_plot_manager_t *mgr, uint32_t argc, char **argv);

void plot_info(crss_plot_manager_t *mgr, uint32_t argc, char **argv);

void plot_set_owner(crss_plot_manager_t *mgr, uint32_t argc, char **argv);

void plot_trust(crss_plot_manager_t *mgr, uint32_t argc, char **argv);

void plot_remove(crss_plot_manager_t *mgr, uint32_t argc, char **argv);

void plot_place(crss_plot_manager_t *mgr, uint32_t argc, char **argv);

void plot_get(crss_plot_manager_t *mgr, uint32_t argc, char **argv);

/*
Set a configuration value of type bool in a plot

Command format: `plot config set <plotname> <optname> [true|false]`
*/
void plot_config_bool(crss_plot_manager_t *mgr, uint32_t argc, char **argv);

#endif // PLOT_COMMANDS_H
