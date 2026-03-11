
#include "hash_map.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

HashMap* create_hash_map(int size){
    HashMap *hash_map = malloc(sizeof(HashMap));
    hash_map->map = malloc(size * sizeof(HashSlot));
    memset(hash_map->map, 0, size * sizeof(HashSlot));
    hash_map->size = size;
    return hash_map;
}

void destroy_hash_map(HashMap *hash_map){
    for (int i = 0; i < hash_map->size; i++){
        free(hash_map->map[i].overflow);
    }
    free(hash_map->map);
    free(hash_map);
}

int hash(HashMap *hash_map, int key){
    key = abs(key);
    return key % hash_map->size;
}

/*
Writing to hashmap checks if slot with key is already in hashmap. If not, 
creates a new slot inside overflow. Every slot is in overflow so reading 
from hashmap always returns an type Slot. If there is slot collision, 
a new overflow slot is created that stores the key.
if statement is needed because overflow size is uninitialised when hash slot is created
*/
void write_to_map(HashMap *hash_map, int key, void* data){
    int h = hash(hash_map, key);
    HashSlot *hash_slot = &hash_map->map[h];
    int *check_key = malloc(2 * sizeof(int));
    int overflow_idx;
    
    if(!hash_slot->used) {
        hash_slot->overflow_size = 1;
        hash_slot->overflow = realloc(hash_slot->overflow, hash_slot->overflow_size * sizeof(Slot));
        overflow_idx = 0;
        hash_slot->used = true;
    }
    else{
        check_key = key_in_hash_slot(hash_slot, key, check_key);
        if(check_key[0]) {
            overflow_idx = check_key[1];
        }
        else{
            hash_slot->overflow_size += 1;
            hash_slot->overflow = realloc(hash_slot->overflow, hash_slot->overflow_size * sizeof(Slot));
            overflow_idx = hash_slot->overflow_size - 1;
        }
    }
    hash_slot->overflow[overflow_idx].idx = h;
    hash_slot->overflow[overflow_idx].key = key;
    hash_slot->overflow[overflow_idx].data = data;
    hash_slot->overflow[overflow_idx].used = true;
    free(check_key);
}

void delete_from_map(HashMap *hash_map, int key){
    int h = hash(hash_map, key);
    HashSlot *hash_slot = &hash_map->map[h];
    int *check_key = malloc(2 * sizeof(int));

    check_key = key_in_hash_slot(hash_slot, key, check_key);
    if(!check_key[0]) return;
    for(int i = check_key[1] + 1; i < hash_slot->overflow_size; i++){
        hash_slot->overflow[i-1] = hash_slot->overflow[i];
    }
    hash_slot->overflow_size -= 1;
    hash_slot->overflow = realloc(hash_slot->overflow, hash_slot->overflow_size * sizeof(Slot));
    if(hash_slot->overflow_size == 0) hash_slot->used = false;
    free(check_key);
}

/*
if key is found in overflow, check_key will be 1 at pos 0 to indicate that the key has been
found, in pos 1, index of slot with key is stored.
If key was not found, pos 0 will be 0.
!! don't forget to free check_key after check is over !!
*/
int* key_in_hash_slot(HashSlot *hash_slot, int key, int *check_key){
    for(int i = 0; i < hash_slot->overflow_size; i++){
        if(hash_slot->overflow[i].key == key){
            check_key[0] = 1;
            check_key[1] = i;
            return check_key;
        }
    }
    check_key[0] = 0;
    return check_key;
}

/*
function returns copy of slot where key is stored, 
if key was not foud, function returns a newly initialised slot.
if returned slot has used = false -> key was not foud
*/
Slot read_from_map(HashMap *hash_map, int key){
    int h = hash(hash_map, key);
    HashSlot *hash_slot = &hash_map->map[h];

    if(!hash_slot->used){
        Slot slot;
        slot.used = false;
        return slot; // key not found
    }
    for(int i = 0; i < hash_slot->overflow_size; i++){
        if(hash_slot->overflow[i].key == key){
           return hash_slot->overflow[i]; // key found
        }
    }
    Slot slot;
    slot.used = false;
    return slot; // key not found
}

int get_max_overflow(HashMap *hash_map){
    int max_overflow = 0;
    for(int i = 0; i < hash_map->size; i++){
        if(hash_map->map[i].overflow_size > max_overflow){
            max_overflow = hash_map->map[i].overflow_size;
        }
    }
    return max_overflow;
}

HashMap* rehash(HashMap *hash_map){
    int target_max_overflow = 2;
    int actual_max_overflow = get_max_overflow(hash_map);
    HashMap *rehashed_map = NULL;
    int map_size = hash_map->size;
    if(actual_max_overflow <= target_max_overflow){
        free(rehashed_map);
        return hash_map;
    }
    while(actual_max_overflow > target_max_overflow){
        map_size = map_size + map_size / 2;
        if(rehashed_map) destroy_hash_map(rehashed_map);
        rehashed_map = create_hash_map(map_size);
        initialize_map_from_keys_in_map(rehashed_map, hash_map);
        actual_max_overflow = get_max_overflow(rehashed_map);
    }
    destroy_hash_map(hash_map);
    return rehashed_map;
}

void initialize_map_from_keys_in_map(HashMap *new_map, HashMap *key_donor_map){
    for(int i = 0; i < key_donor_map->size; i++){
        for(int u = 0; u < key_donor_map->map[i].overflow_size; u++){
            write_to_map(new_map, 
                         key_donor_map->map[i].overflow[u].key, 
                         (void*)(long)key_donor_map->map[i].overflow[u].data);
        }
    }
}

void print_shape_map(HashMap *hash_map){
    printf("---- Shape of Map ----\n");
    for(int i = 0; i < hash_map->size; i++){
        int overflow_size = hash_map->map[i].overflow_size;
        printf("Hashslot [%d]: overflow: [%d]\n", i, overflow_size);
    }
}

void print_map(HashMap *hash_map){
    printf("-------- Map --------\n");
    for(int i = 0; i < hash_map->size; i++){
        int overflow_size = hash_map->map[i].overflow_size;
        printf("Hashslot[%d]:\n", i);
        if(overflow_size > 0){
            for(int u = 0; u < overflow_size; u++){
                int key = hash_map->map[i].overflow[u].key;
                long data = (long)hash_map->map[i].overflow[u].data;
                int idx = hash_map->map[i].overflow[u].idx;
                int used = hash_map->map[i].overflow[u].used;
                printf("Subslot[%d]: Key[%d]; Data[%ld]; Index[%d]; Used[%d]\n",
                        u, key, data, idx, used);
            }
            printf("\n");
        }
        else{
            printf("No Subslots\n\n");
        }
    }
}