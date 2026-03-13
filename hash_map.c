
#include "hash_map.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdbool.h>


#ifdef TESTING
    #include "test_alloc.h"
    #define MALLOC test_malloc
    #define REALLOC test_realloc
#else
    #define MALLOC malloc
    #define REALLOC realloc
#endif

HashMap* create_hash_map(int size){
    HashMap *hash_map = MALLOC(sizeof(HashMap));
    hash_map->map = MALLOC(size * sizeof(HashSlot));
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
HashMapError write_to_map(HashMap *hash_map, int key, void* data){
    int h = hash(hash_map, key);
    HashSlot *hash_slot = &hash_map->map[h];
    KeyChecker key_checker;
    int overflow_idx;
    
    if(!hash_slot->used) {
        // use temporary sizes that drop when alloc fails -> map only changes when alloc succeedes
        int tmp_overflow_size = 1;
        void *tmp = REALLOC(hash_slot->overflow, tmp_overflow_size * sizeof(Slot));
        if(alloc_failed(tmp)){
            return HASH_MAP_ERR_ALLOC;
        }
        hash_slot->overflow_size = tmp_overflow_size;
        hash_slot->overflow = tmp;
        overflow_idx = 0;
        hash_slot->used = true;
    }
    else{
        key_in_hash_slot(hash_slot, key, &key_checker);
        if(key_checker.key_found) {
            overflow_idx = key_checker.overflow_idx;
        }
        else{
            // use temporary sizes that drop when alloc fails -> map only changes when alloc succeedes
            int tmp_overflow_size = hash_slot->overflow_size + 1;
            void *tmp = REALLOC(hash_slot->overflow, tmp_overflow_size * sizeof(Slot));
            if(alloc_failed(tmp)){
                return HASH_MAP_ERR_ALLOC;
            }
            hash_slot->overflow_size = tmp_overflow_size;
            hash_slot->overflow = tmp;
            overflow_idx = hash_slot->overflow_size - 1;
        }
    }
    hash_slot->overflow[overflow_idx].idx = h;
    hash_slot->overflow[overflow_idx].key = key;
    hash_slot->overflow[overflow_idx].data = data;
    hash_slot->overflow[overflow_idx].used = true;
    return HASH_MAP_OK;
}

HashMapError delete_from_map(HashMap *hash_map, int key){
    int h = hash(hash_map, key);
    HashSlot *hash_slot = &hash_map->map[h];
    KeyChecker key_checker;

    key_in_hash_slot(hash_slot, key, &key_checker);
    if(!key_checker.key_found){ 
        return HASH_MAP_OK;
    }
    hash_slot->overflow_size -= 1;
    if(hash_slot->overflow_size <= 0){          // realloc(..., 0 * sizeof(size_t)) is a memory leak
        hash_slot->overflow_size = 0;           // that's why we free overflow when size drops to zero
        free(hash_slot->overflow);
        hash_slot->overflow = NULL;
        hash_slot->used = false;
    }
    else {
        for(int i = key_checker.overflow_idx + 1; i < hash_slot->overflow_size + 1; i++){
            hash_slot->overflow[i-1] = hash_slot->overflow[i];
        }
        /* no realloc fail check because realloc to smaller size than before, can never fail. 
           realloc fail check leads to bugs because can't change subslots before reallocs and can't realloc 
           before changes subslots -> would need to temporarly malloc new overflow and then drop old.
           design decision against is because it defeats purpose because of optimistic memory allocation. */
        hash_slot->overflow = realloc(hash_slot->overflow, hash_slot->overflow_size * sizeof(Slot));
    }
    return HASH_MAP_OK;
}

void key_in_hash_slot(HashSlot *hash_slot, int key, KeyChecker *key_checker){
    for(int i = 0; i < hash_slot->overflow_size; i++){
        if(hash_slot->overflow[i].key == key){
            key_checker->key_found = true;
            key_checker->overflow_idx = i;
            return;
        }
    }
    key_checker->key_found = false;
    return;
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
    HashMapError error;
    int target_max_overflow = 2;
    int actual_max_overflow = get_max_overflow(hash_map);
    HashMap *rehashed_map = NULL;
    int map_size = hash_map->size;
    if(actual_max_overflow <= target_max_overflow){
        return hash_map;
    }
    while(actual_max_overflow > target_max_overflow){
        map_size = map_size + map_size / 2;
        if(rehashed_map) destroy_hash_map(rehashed_map);
        rehashed_map = create_hash_map(map_size);
        error = initialize_map_from_keys_in_map(rehashed_map, hash_map);
        if(error){
            destroy_hash_map(rehashed_map);
            return hash_map;
        }
        actual_max_overflow = get_max_overflow(rehashed_map);
    }
    destroy_hash_map(hash_map);
    return rehashed_map;
}

HashMapError initialize_map_from_keys_in_map(HashMap *new_map, HashMap *key_donor_map){
    HashMapError error;
    for(int i = 0; i < key_donor_map->size; i++){
        for(int u = 0; u < key_donor_map->map[i].overflow_size; u++){
            error = write_to_map(new_map, 
                                 key_donor_map->map[i].overflow[u].key, 
                                 (void*)key_donor_map->map[i].overflow[u].data);
            if(error){
                return HASH_MAP_ERR_ALLOC;
            }
        }
    }
    return HASH_MAP_OK;
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

bool alloc_failed(void *ptr){
    if(ptr){
        return false;
    }
    return true;
}