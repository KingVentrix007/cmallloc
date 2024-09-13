#ifndef __ALLOC__H
#define __ALLOC__H
#include "stdint.h"
#include "stddef.h"
#include "stdbool.h"

typedef struct
{
    bool dirty; // Flag to say wether or not the block is dirty
    size_t size; // Size of the allocation
    uint8_t *ptr; // ptr of the allocation;
}memory_alloc_t;

#define ALLOC_STRUCT_SIZE sizeof(memory_alloc_t);

#endif  