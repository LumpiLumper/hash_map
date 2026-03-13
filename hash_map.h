#ifndef HASH_MAP_H
#define HASH_MAP_H

#include <stdbool.h>
#include <stddef.h>

typedef enum {
    HASH_MAP_OK = 0,
    HASH_MAP_NOT_FOUND,
    HASH_MAP_ERR_ALLOC
} HashMapError;

typedef struct{
    bool key_found;
    int overflow_idx;
} KeyChecker;

typedef struct{
    int key;
    int idx;
    void* data;
    bool used;
} Slot;

typedef struct{
    Slot *overflow;
    int overflow_size;
    bool used;
} HashSlot;

typedef struct{
    HashSlot *map;
    int size;
} HashMap;

HashMap* create_hash_map(int size);
void destroy_hash_map(HashMap *hash_map);
int hash(HashMap *hash_map, int key);
HashMapError write_to_map(HashMap *hash_map, int key, void* data);
HashMapError delete_from_map(HashMap *hash_map, int key);
void key_in_hash_slot(HashSlot *hash_slot, int key, KeyChecker *key_checker);
Slot read_from_map(HashMap *hash_map, int key);
HashMap* rehash(HashMap *hash_map);
HashMapError initialize_map_from_keys_in_map(HashMap *new_map, HashMap *key_donor_map);
void print_shape_map(HashMap *hash_map);
void print_map(HashMap *hash_map);
bool alloc_failed(void *ptr);

#endif