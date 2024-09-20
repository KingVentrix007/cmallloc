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
    uint32_t  magic; // Magic number
}memory_alloc_t;

#define ALLOC_STRUCT_SIZE sizeof(memory_alloc_t);
void *cmalloc(size_t size);
int cfree(void *ptr);
void *crealloc(void *ptr, unsigned int old_size, unsigned int new_size);
void* ccalloc(size_t num_elements, size_t element_size) ;
size_t get_size_cmalloc(const void *ptr);
#ifdef CMALLOC_LIB

#endif
void *malloc(size_t size);
void free(void *ptr);
void *realloc(void *ptr,size_t new_size);
void *calloc(size_t __nmemb, size_t __size);
__attribute__((constructor))void cmalloc_init(void);
#endif  