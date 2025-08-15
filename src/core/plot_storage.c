#include "plot_storage.h"

plot_storage_t *new_plot_storage(uint8_t log2_sidelength) {
    plot_storage_t *storage = malloc(sizeof(plot_storage_t));
    ssvo_t *s = new_ssvo(log2_sidelength);
    storage->ssvo = s;
    return storage;
}

void free_plot_storage(plot_storage_t *storage) {
    free_ssvo(storage->ssvo);
    free(storage);
}

block_t *get_block(plot_storage_t *storage, uint32_t x, uint32_t y, uint32_t z) {
    uint32_t chunk_x = x >> 4;
    uint32_t chunk_y = y >> 4;
    uint32_t chunk_z = z >> 4;
    uint8_t ofs_x = x & 0xF;
    uint8_t ofs_y = y & 0xF;
    uint8_t ofs_z = z & 0xF;
    chunk_t *chunk = ssvo_get_leaf(storage->ssvo, chunk_x, chunk_y, chunk_z);
    if (chunk == NULL) return NULL;
    block_t *blk = &chunk->blocks[ofs_z][ofs_y][ofs_x];
    return blk;
}

/*
@param block Pointer to block, which is copied into storage.
*/
void set_block(plot_storage_t *storage, uint32_t x, uint32_t y, uint32_t z, block_t *block) {
    uint32_t chunk_x = x >> 4;
    uint32_t chunk_y = y >> 4;
    uint32_t chunk_z = z >> 4;
    uint8_t ofs_x = x & 0xF;
    uint8_t ofs_y = y & 0xF;
    uint8_t ofs_z = z & 0xF;
    chunk_t *chunk = ssvo_get_leaf(storage->ssvo, chunk_x, chunk_y, chunk_z);
    if (chunk == NULL) {
        /* Allocate new chunk */
        chunk = calloc(1, sizeof(chunk_t)); // TODO: free this
        ssvo_set_leaf(storage->ssvo, chunk_x, chunk_y, chunk_z, chunk);
    }
    memcpy(&chunk->blocks[ofs_z][ofs_y][ofs_x], block, sizeof(block_t));

}
