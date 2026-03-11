// gcc -o hash_map_test hash_map_test.c hash_map.c && ./hash_map_test

#include "hash_map.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

static int passed = 0;
static int failed = 0;

void check_found(const char *label, Slot result, int expected_idx, int expected_data)
{
    if (!result.used) {
        printf("[FAIL] %s  got=not found  expected idx=%d\n", label, expected_idx);
        failed++;
    } else if (result.idx == expected_idx && result.data == expected_data) {
        printf("[PASS] %s  idx=%d, data=%ld\n", label, result.idx, result.data);
        passed++;
    } else {
        printf("[FAIL] %s  got idx=%d  expected idx=%d, got data=%ld expected data = %d\n", 
            label, result.idx, expected_idx, result.data, expected_data);
        failed++;
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

int main(void)
{
    HashMap *map = create_hash_map(10);

    /* basic write and read */
    write_to_map(map, 5, -3);
    write_to_map(map, 3, 4);
    write_to_map(map, 7, 7);

    check_found("read key=5", read_from_map(map, 5), 5, -3);
    check_found("read key=3", read_from_map(map, 3), 3, 4);
    check_found("read key=7", read_from_map(map, 7), 7, 7);

    /* key not in map */
    check_not_found("read key=99 (not inserted)", read_from_map(map, 99));

    /* collision: 5 and 15 both hash to slot 5 with size=10 */
    write_to_map(map, 15, 157);
    check_found("read key=5  after collision", read_from_map(map, 5),  5, -3);
    check_found("read key=15 after collision", read_from_map(map, 15), 5, 157);

    /* overwriting data at key */
    write_to_map(map, 5, 3);
    check_found("read data at key=5 after overwriting", read_from_map(map, 5), 5, 3);

    /* collision: 3, 13, 23 all hash to slot 3 */
    write_to_map(map, 13, 3);
    write_to_map(map, 23, -157);
    check_found("read key=3",  read_from_map(map, 3),  3, 4);
    check_found("read key=13", read_from_map(map, 13), 3, 3);
    check_found("read key=23", read_from_map(map, 23), 3, -157);

    /* key=0 edge case */
    write_to_map(map, 0, 1);
    check_found("read key=0", read_from_map(map, 0), 0, 1);

    /* key negative numbers edge case*/
    write_to_map(map, -1, 15);
    write_to_map(map, -5, 6);
    write_to_map(map, -100, 255);
    check_found("read key=-1", read_from_map(map, -1), 1, 15);
    check_found("read key=-5", read_from_map(map, -5), 5, 6);
    check_found("read key=-17", read_from_map(map, -100), 0, 255);
    check_not_found("read key=-10", read_from_map(map, -10));
    check_not_found("read key=-99", read_from_map(map, -99));

    
    printf("\n%d passed, %d failed\n", passed, failed);
    return failed > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}