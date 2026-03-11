#ifndef HASH_MAP_H
#define HASH_MAP_H

#include <stdbool.h>

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
void write_to_map(HashMap *hash_map, int key, void* data);
void delete_from_map(HashMap *hash_map, int key);
int* key_in_hash_slot(HashSlot *hash_slot, int key, int *check_key);
Slot read_from_map(HashMap *hash_map, int key);
void rehash(HashMap *hash_map);
void initialize_map_from_keys_in_map(HashMap *new_map, HashMap *key_doner_map);

#endif