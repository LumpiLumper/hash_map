#ifndef HASH_MAP_H
#define HASH_MAP_H

#include <stdbool.h>

typedef struct{
    int key;
    int idx;
    int data;
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
void write_to_map(HashMap *hash_map, int key, int data);
Slot read_from_map(HashMap *hash_map, int key);

#endif