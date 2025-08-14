#ifndef PLOT_STORAGE_H
#define PLOT_STORAGE_H

#include "utils.h"
#include "ssvo.h"

/*
Plot storage utility

Extends the SSVO utility by adding chunks as leaf data objects.

The primary provided interface is a per-block one, where a "block" corresponds to a unit volume inside a plot with x, y and z coordinates.
*/

typedef struct {
    ssvo_t *ssvo;
} plot_storage_t;

/* Blocks */
typedef uint8_t block_t;

/* Block singletons */
const block_t AIR_BLOCK = 0;

typedef struct {
    block_t blocks[16][16][16];
} chunk_t;

plot_storage_t *new_plot_storage(uint8_t log2_sidelength);

void free_plot_storage(plot_storage_t *storage);

block_t *get_block(plot_storage_t *storage, uint32_t x, uint32_t y, uint32_t z);

void set_block(plot_storage_t *storage, uint32_t x, uint32_t y, uint32_t z, block_t *block);

/* TODO: [some] worldedit operations here :) */

#endif // PLOT_STORAGE_H
