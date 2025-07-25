#ifndef HASHMAP_H
#define HASHMAP_H

#include "utils.h"

/* Hash map utility for CRSS */

struct hashmap_item_;

typedef struct hashmap_item_ {
    void *data;
    char *key;
    uint32_t hash;
    struct hashmap_item_ *next;
    struct hashmap_item_ *prev;
} hashmap_item_t;

typedef hashmap_item_t *hashmap_bucket_t;

typedef struct {
    hashmap_bucket_t *buckets;
    uint32_t num_buckets_log; // log2(#buckets)
    uint32_t num_items;
} hashmap_t;

uint32_t hash(char *key);

hashmap_t *new_hashmap(void);

/*
Update hashmap item.

@param map Hashmap
@param key Key to update
@param data New data or NULL to prevent updating value
@return Old data or NULL if key not found
*/
void *hashmap_update_item(hashmap_t *map, char *key, void *data);

void *hashmap_get_item(hashmap_t *map, char *key);

/*
Remove item from hashmap.

Data associated with the key should already be freed, e.g. via `free(hashmap_get_item(map, key))` if the key is guaranteed to be in the hashmap.
@return true if the item was found and removed
*/
bool hashmap_remove_item(hashmap_t *map, char *key);

/*
Map an operation to a hashmap.

Calls the function `func` on each data element of the hashmap `map`.
*/
void hashmap_map(hashmap_t *map, void (*func)(void *));

void prettyprint_hashmap(char *fnpath, hashmap_t *map, char *(*serializer)(void *));

void free_hashmap(hashmap_t *map);

#endif // HASHMAP_H
