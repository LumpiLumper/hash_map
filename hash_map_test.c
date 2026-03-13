// gcc -DTESTING -o hash_map_test hash_map_test.c hash_map.c && ./hash_map_test

#include "hash_map.h"
#include "test_alloc.h" 

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static int passed = 0;
static int failed = 0;

int fail_alloc_after = -1; // alloc will never fail while fail_alloc_after is negative
                           // see test_alloc.h

void check_found(const char *label, Slot result, int expected_idx, long expected_data)
{
    if (!result.used) {
        printf("[FAIL] %s  got=not found  expected idx=%d\n", label, expected_idx);
        failed++;
    } else if (result.idx == expected_idx && (long)result.data == expected_data) {
        printf("[PASS] %s  idx=%d, data=%ld\n", label, result.idx, (long)result.data);
        passed++;
    } else {
        printf("[FAIL] %s  got idx=%d  expected idx=%d, got data=%ld expected data = %ld\n", 
            label, result.idx, expected_idx, (long)result.data, expected_data);
        failed++;
    }
}

bool check_is_there(int key, Slot result){
    if(!result.used){
        printf("[NOT FOUND] Key=%d not found\n", key);
        return false;
    }
    else{
        printf("[FOUND] Key=%d found at idx=%d with data=%p \n", key, result.idx, result.data);
        return true;
    }
}

bool check_is_there_no_printf(int key, Slot result){
    if(!result.used){
        return false;
    }
    else{
        if(result.key != key) return false;
        return true;
    }
}

void check_not_found(const char *label, Slot result)
{
    if (!result.used) {
        printf("[PASS] %s  correctly not found\n", label);
        passed++;
    } else {
        printf("[FAIL] %s  expected not found but got idx=%d\n", label, result.idx);
        failed++;
    }
}

void print_error(HashMapError error){
    switch(error){
        case HASH_MAP_OK:
            printf("Map Status: HASH_MAP_OK\n");
            break;
        case HASH_MAP_NOT_FOUND:
            printf("Map Status: HASH_MAP_NOT_FOUND\n");
            break;
        case HASH_MAP_ERR_ALLOC:
            printf("Map Status: HASH_MAP_ERR_ALLOC\n");
            break;
        default:
            break;
  
    }
}

int rand_number(int min, int max){
    return (rand() % (max + 1)) + min;
}

int main(void)
{
    srand(time(NULL));
    HashMap *map = create_hash_map(10);
    HashMapError error;

    /* basic write and read */
    error = write_to_map(map, 5, (void*)(long)-3);
    error = write_to_map(map, 3, (void*)(long)4);
    error = write_to_map(map, 7, (void*)(long)7);

    check_found("read key=5", read_from_map(map, 5), 5, -3);
    check_found("read key=3", read_from_map(map, 3), 3, 4);
    check_found("read key=7", read_from_map(map, 7), 7, 7);
    print_error(error);

    //print_shape_map(map);

    /* key not in map */
    check_not_found("read key=99 (not inserted)", read_from_map(map, 99));
    /* collision: 5 and 15 both hash to slot 5 with size=10 */
    error = write_to_map(map, 15, (void*)(long)157);
    check_found("read key=5  after collision", read_from_map(map, 5),  5, -3);
    check_found("read key=15 after collision", read_from_map(map, 15), 5, 157);
    print_error(error);

    //print_shape_map(map);

    /* overwrite data saved at key */
    error = write_to_map(map, 5, (void*)(long)3);
    error = write_to_map(map, 15, (void*)(long)100);
    check_found("read data at key=5 after overwriting", read_from_map(map, 5), 5, 3);
    check_found("read data at key=15 after overwriting", read_from_map(map, 15), 5, 100);
    
    print_error(error);

    //print_shape_map(map);

    /* collision: 3, 13, 23 all hash to slot 3 */
    error = write_to_map(map, 13, (void*)(long)3);
    error = write_to_map(map, 23, (void*)(long)-157);
    check_found("read key=3",  read_from_map(map, 3),  3, 4);
    check_found("read key=13", read_from_map(map, 13), 3, 3);
    check_found("read key=23", read_from_map(map, 23), 3, -157);

    print_error(error);

//    print_shape_map(map);

    /* key=0 edge case */
    error = write_to_map(map, 0, (void*)(int)1);
    check_found("read key=0", read_from_map(map, 0), 0, 1);

    print_error(error);

//    print_shape_map(map);

    /* key negative numbers edge case*/
    error = write_to_map(map, -1, (void*)(long)15);
    error = write_to_map(map, -5, (void*)(long)6);
    error = write_to_map(map, -100, (void*)(long)255);
    check_found("read key=-1", read_from_map(map, -1), 1, 15);
    check_found("read key=-5", read_from_map(map, -5), 5, 6);
    check_found("read key=-17", read_from_map(map, -100), 0, 255);
    check_not_found("read key=-10", read_from_map(map, -10));
    check_not_found("read key=-99", read_from_map(map, -99));

    print_error(error);
    
    /* delete key form hash map */
    error = delete_from_map(map, -1);
    print_error(error);
    error = delete_from_map(map, 15);
    print_error(error);
    check_not_found("read key=-1 after deleting", read_from_map(map, -1));
    check_not_found("read key=15 after deleting", read_from_map(map, 15));

    error = write_to_map(map, 15, (void*)(long)-100);
    error = delete_from_map(map, 5);
    check_found("read key=15 after adding it again collision key=5 deleted", read_from_map(map, 15), 5, -100);
    check_not_found("read key=5 after deleting", read_from_map(map, 5));

    error = write_to_map(map, 6, (void*)(int)100);
    error = write_to_map(map, -6, (void*)(int)-100);

    print_map(map);

    error = delete_from_map(map, 6);
    check_found("read key=-6 after sub slot was deleted", read_from_map(map, -6), 6, -100);

    print_error(error);

    print_map(map);

    /* rehashing map */
    map = rehash(map);

//    print_map(map);

    /* find keys in rehashed map */
    check_found("read key=0", read_from_map(map, 0), 0, 1);
    check_found("read key=15", read_from_map(map, 15), 0, -100);
    check_found("read key=3", read_from_map(map, 3), 3, 4);
    check_found("read key=-5", read_from_map(map, -5), 5, 6);
    check_found("read key=7", read_from_map(map, 7), 7, 7);
    check_found("read key=23", read_from_map(map, 23), 8, -157);
    check_found("read key=-100", read_from_map(map, -100), 10, 255);
    check_found("read key=13", read_from_map(map, 13), 13, 3);

    /* rehashing when no rehash possible */
    map = rehash(map);

//    print_map(map);

    check_found("read key=0", read_from_map(map, 0), 0, 1);
    check_found("read key=15", read_from_map(map, 15), 0, -100);
    check_found("read key=3", read_from_map(map, 3), 3, 4);
    check_found("read key=-5", read_from_map(map, -5), 5, 6);
    check_found("read key=7", read_from_map(map, 7), 7, 7);
    check_found("read key=23", read_from_map(map, 23), 8, -157);
    check_found("read key=-100", read_from_map(map, -100), 10, 255);
    check_found("read key=13", read_from_map(map, 13), 13, 3);

    /* empty overflow (reduce overflow to 0 * sizeof(int)) */
    error = delete_from_map(map, 13);
    error = delete_from_map(map, 13);

    print_error(error);

    /* test alloc fail handeling */
    printf("-------- Alloc Fail Testing --------\n");
    
    fail_alloc_after = 0;
    bool found;

    /* write to map !hash_slot_used fail realloc
       (hash_slot 12 is unused) */
    fail_alloc_after = 0;
    error = write_to_map(map, 12, (void*)(int)1);
    fail_alloc_after = -1;
    found = check_is_there(12, read_from_map(map, 12));
    print_error(error);
    if(error == HASH_MAP_ERR_ALLOC && !found){
        printf("[PASS] Alloc fail realloc at unused slot inside write_to_map succesfull\n");
        passed += 1;
    }
    else{
        printf("[FAILED] Alloc fail realloc at unused slot inside write_to_map unsuccesful\n");
        failed += 1;
    }

    /* write to map new overflow slot fail realloc
       (hash_slot 3 has key 3 in it) */
    fail_alloc_after = 0;
    error = write_to_map(map, -3, (void*)(int)1);
    fail_alloc_after = -1;
    found = check_is_there(-3, read_from_map(map, -3));
    print_error(error);
    if(error == HASH_MAP_ERR_ALLOC && !found){
        printf("[PASS] Alloc fail realloc at used slot inside write_to_map succesfull\n");
        passed += 1;
    }
    else{
        printf("[FAILED] Alloc fail realloc at used slot inside write_to_map unsuccesful\n");
        failed += 1;
    }

    destroy_hash_map(map);

    /* Volume Test */
    map = create_hash_map(100);
    int number_keys = 20000;

    int keys[number_keys];

    bool pass = true;

    error = HASH_MAP_OK;

    for(int i = 0; i < number_keys; i++){
        keys[i] = rand_number(-16000, 16000);
        error = write_to_map(map, keys[i], &i);
        if(error != HASH_MAP_OK){
            printf("[FAILED] write at key=%d\n", keys[i]);
            pass = false;
            failed += 1;
        }
    }
    if(pass == true){
        printf("[PASSED] writing keys passed\n");
        passed += 1;
    }
    pass = true;

    for(int i = 0; i < number_keys; i++){
        if(!check_is_there_no_printf(keys[i], read_from_map(map, keys[i]))){
            printf("[FAILED] read at key=%d\n", keys[i]);
            pass = false;
            failed += 1;
        }
    }
    if(pass == true){
        printf("[PASSED] reading keys passed\n");
        passed += 1;
    }
    pass = true;

    print_shape_map(map);

    int number_keys_to_delete = number_keys / 2;

    int keys_to_delete[number_keys_to_delete];

    for(int i = 0; i < number_keys_to_delete; i++){
        keys_to_delete[i] = rand_number(0, number_keys);
        error = delete_from_map(map, keys[keys_to_delete[i]]);
        if(error != HASH_MAP_OK){
            printf("[FAILED] delete key=%d\n", keys[keys_to_delete[i]]);
            pass = false;
            failed += 1;
        }
    }
    if(pass == true){
        printf("[PASSED] deleting keys passed\n");
        passed += 1;
    }
    pass = true;

    for(int i = 0; i < number_keys_to_delete; i++){
        if(check_is_there_no_printf(keys[keys_to_delete[i]], read_from_map(map, keys[keys_to_delete[i]]))){
            printf("[FAILED] found key=%d after deleting\n", keys[keys_to_delete[i]]);
            pass = false;
            failed += 1;
        }
        
    }
    if(pass == true){
        printf("[PASSED] didn't find keys after deleting\n");
        passed += 1;
    }
    pass = true;

    print_shape_map(map);

    for(int i = 0; i < number_keys_to_delete; i++){
        error = write_to_map(map, keys[keys_to_delete[i]], &i);
        if(error != HASH_MAP_OK){
            printf("[FAILED] delete key=%d\n", keys[keys_to_delete[i]]);
            pass = false;
            failed += 1;
        }
    }
    if(pass == true){
        printf("[PASSED] deleting keys passed\n");
        passed += 1;
    }
    pass = true;

    map = rehash(map);

    //print_shape_map(map);

    for(int i = 0; i < number_keys; i++){
        if(!check_is_there_no_printf(keys[i], read_from_map(map, keys[i]))){
            printf("[FAILED] read at key=%d after rehash\n", keys[i]);
            pass = false;
            failed += 1;
        }
    }
    if(pass == true){
        printf("[PASSED] reading keys after rehash passed\n");
        passed += 1;
    }

    pass = true;
    
    
    printf("\n%d passed, %d failed\n", passed, failed);
    destroy_hash_map(map);
    return failed > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}