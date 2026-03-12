#ifndef TEST_ALLOC_H
#define TEST_ALLOC_H

#include <stdlib.h>

extern int fail_alloc_after; // -1: never fail; 0: fail everytime; n: fail after n times

static inline void *test_malloc(size_t size){
    if(fail_alloc_after == 0) return NULL;
    if(fail_alloc_after > 0) fail_alloc_after--;
    return malloc(size);
}

static inline void *test_realloc(void* ptr, size_t size){
    if(fail_alloc_after == 0) return NULL;
    if(fail_alloc_after > 0) fail_alloc_after--;
    return realloc(ptr, size);
}

#endif