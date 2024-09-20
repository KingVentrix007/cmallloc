#ifndef __LARGEALLOC__H
#define __LARGEALLOC__H
#include "stdint.h"
#include "stddef.h"
#include "stdbool.h"
typedef struct large_allocs_t
{
    size_t size;
    bool dirty;
    uint32_t magic;
    void *ptr;
}large_allocs_t;
void *large_alloc(size_t size);
int large_free(void *ptr);
size_t get_large_alloc_size(const void *ptr);
int init_large_allocations(void);
#endif