#include "hashmap.h"

uint32_t hash(char *key) {
    uint32_t hash = 5381;
    int c;

    while ((c = *key++)) {
        // Use 17 as a multiplier; as it empirically yields slightly
        // better results for my use case
        hash = ((hash << 4) + hash) + c;
    }
    
    return hash;
}

hashmap_t *new_hashmap() {
    hashmap_t *map = malloc(sizeof(hashmap_t));
    #define NUM_BUCKETS_LOG 10 // TODO: Dynamic resizing
    map->num_buckets_log = NUM_BUCKETS_LOG;
    map->num_items = 0;
    map->buckets = calloc(1 << NUM_BUCKETS_LOG, sizeof(hashmap_item_t *));
    return map;
}

/* Return a pointer to the bucket that <key> falls in */
static hashmap_bucket_t *get_hashmap_bucket(hashmap_t *map, char *key) {
    uint32_t key_hash = hash(key);
    // For e.g. log2(#buckets) = 5 => 0x0000001F = 5 lower bits
    uint32_t hash_idx_bits = (1 << map->num_buckets_log) - 1;
    // Isolate relevant bits from hash
    uint32_t bucket_idx = key_hash & hash_idx_bits;
    return &map->buckets[bucket_idx];
}

char *get_any_key(hashmap_t *map) {
    for (int idx = 0; idx < (1 << map->num_buckets_log); idx++) {
        hashmap_item_t *bucket_item = map->buckets[idx];
        if (bucket_item != NULL) {
            return bucket_item->key;
        }
    }
    return NULL;
}

// TODO: Dynamic resizing
void *hashmap_update_item(hashmap_t *map, char *key, void *data) {
    hashmap_bucket_t *bucket = get_hashmap_bucket(map, key);
    hashmap_item_t *bucket_item = (hashmap_item_t *)*bucket;
    // This is a sketchy type cast because the bucket pointed to by
    // <bucket> is of course not an actual item. An explicit check
    // is thus required later to make sure the pointers stay correct.
    hashmap_item_t *previous_item = (hashmap_item_t *)bucket;

    // Find item
    while (bucket_item != NULL) {
        if (bucket_item->hash == hash(key)) {
            // only compare actual keys if the hashes map for performance
            if (strcmp(bucket_item->key, key) == 0) break;
        }
        previous_item = bucket_item;
        bucket_item = bucket_item->next;
    }

    if (bucket_item == NULL) {
        if (data == NULL) return NULL; // Miss while reading shouldn't create a value
        // no item found -> insert item
        hashmap_item_t *new_item = malloc(sizeof(hashmap_item_t));
        char *key_dup = mystrdup(key);
        new_item->data = data;
        new_item->key = key_dup;
        new_item->hash = hash(key);
        new_item->next = NULL;
        new_item->prev = previous_item;
        if (previous_item == (hashmap_item_t *)bucket) {
            *bucket = new_item;
        } else {
            previous_item->next = new_item;
        }
        map->num_items++;
        return NULL;
    }

    //printf("Replacing '%s' with '%s' for key '%s'!\n", (char *)bucket_item->data, (char *)data, key);
    void *old_data = bucket_item->data;
    if (data != NULL) {
        bucket_item->data = data;
    }
    return old_data;
}

void *hashmap_get_item(hashmap_t *map, char *key) {
    void *ret = hashmap_update_item(map, key, NULL);
    return ret;
}

// TODO: Dynamic resizing
bool hashmap_remove_item(hashmap_t *map, char *key) {
    hashmap_bucket_t *bucket = get_hashmap_bucket(map, key);
    hashmap_item_t *bucket_item = (hashmap_item_t *)*bucket;
    // This is a sketchy type cast because the bucket pointed to by
    // <bucket> is of course not an actual item. An explicit check
    // is thus required later to make sure the pointers stay correct.
    hashmap_item_t *previous_item = (hashmap_item_t *)bucket;

    // Find item
    while (bucket_item != NULL) {
        if (bucket_item->hash == hash(key)) {
            // only compare actual keys if the hashes map for performance
            if (strcmp(bucket_item->key, key) == 0) break;
        }
        previous_item = bucket_item;
        bucket_item = bucket_item->next;
    }

    // Two cases:
    // 1. bucket_item is NULL, so no item was found
    // 2. bucket_item is not NULL, so make previous point to bucket's next
    if (bucket_item == NULL) return false;
    // previous_item is a hashmap_bucket_t* when previous_item = bucket
    // in that case, just set it to bucket_item->next
    // otherwise, it's a hashmap_item_t* and its next should become bucket_item->next
    // (this is the aforementioned explicit pointer check)
    if (previous_item == (hashmap_item_t *)bucket) {
        *bucket = bucket_item->next;
    } else {
        previous_item->next = bucket_item->next;
    }
    free(bucket_item->key);
    free(bucket_item);
    map->num_items--;

    return NULL;
}

void hashmap_map(hashmap_t *map, void (*func)(void *)) {
    //int n = 0;
    for (int idx = 0; idx < (1 << map->num_buckets_log); idx++) {
        hashmap_item_t *item = (hashmap_item_t *)map->buckets[idx];
        while (item != NULL) {
            func(item->data);
            //n++;
            hashmap_item_t *next = item->next;
            item = next;
        }
    }
    //printf("Ran func %d times\n", n);
}

void free_hashmap(hashmap_t *map) {
    for (int idx = 0; idx < (1 << map->num_buckets_log); idx++) {
        hashmap_item_t *item = (hashmap_item_t *)map->buckets[idx];
        while (item != NULL) {
            hashmap_item_t *next = item->next;
            free(item->key);
            free(item);
            item = next;
        }
    }
    free(map->buckets);
    free(map);
}

/*
Pretty logs a hash map.

@param map Hash map to print
@param serializer Function to convert internal data pointers [subclasses] to strings.
Serialized strings are freed.
*/
void prettyprint_hashmap(char *fnpath, hashmap_t *map, char *(*serializer)(void *)) {
    FUNCPATH("hashmap_printer");
    uint32_t min_count = -1;
    uint32_t max_count = 0;
    uint32_t itemcount = 0;
    for (int idx = 0; idx < (1 << map->num_buckets_log); idx++) {
        //printf("Bucket %d: ", idx);
        hashmap_item_t *item = (hashmap_item_t *)map->buckets[idx];
        int i = 0;
        while (item != NULL) {
            char *serialized = serializer(item->data);
            LINFO("    %d: '%s': \"\033[1m%s\033[0m\" \033[2m[Hash 0x%08x]\033[0m", itemcount, item->key, serialized, item->hash);
            free(serialized);
            itemcount++;
            i++;
            item = item->next;
        }
        if (i < min_count) min_count = i;
        if (i > max_count) max_count = i;
        //printf("%d Items\n", i);
    }
    //printf("Min %d, Max %d items in a bucket\n", min_count, max_count);
    LINFO("(%d items)", itemcount);
    free(fnpath);
}

/* Test Program */
int hash_main() {
    hashmap_t *map = new_hashmap();
    //uint32_t rollhash = hash("axchtparj39oaijewiudjmnvfsa");
    uint32_t data = hash("axchtparj39oaijewiudjmnvfsa");
    char rollhash_str[11];
    char data_str[11];
    rollhash_str[10] = '\0';
    data_str[10] = '\0';
    int d[0x10000];
    int k = 0;
    for (int i = 0; i < 0x10000; i++) {
        d[i] = i;
        sprintf((char *)data_str, "%u", data);
        sprintf((char *)rollhash_str, "%u", k);
        k = (k + (i * 0xC93)) ^ 0xB8A25D53;

        char *value = mystrdup(data_str);
        void *old_data;
        if ((old_data = hashmap_update_item(map, rollhash_str, value))) {
            free(old_data);
        }
        
        //rollhash = hash(rollhash_str);
        data = hash(data_str);
    }
    hashmap_update_item(map, "testtest", mystrdup("hello world"));
    /*for (int i = 0; i < 0x100000; i++) {
        char adr[11];
        adr[10] = '\0';
        sprintf(adr, "%u", i);
        int *data = hashmap_get_item(map, adr);
        if (*data != i) {
            printf("Wrong data! Key '%s', value %d, expected %d\n", adr, *data, i);
        }
    }*/
    hashmap_map(map, free);
    free_hashmap(map);   
    return 0; 
}
