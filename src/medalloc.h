#ifndef __MEDALLOC__H
#define __MEDALLOC__H
#include "stdint.h"
#include "stddef.h"
#include "stdbool.h"

typedef struct block_t
{
    size_t size; // Size of the block in bytes
    struct block_t *next; // Pointer to the next block
    struct block_t *prev; // Pointer to the previous block
    bool free; // Indicates if the block is free
    uint32_t magic;// Magic number to detect corruption
    void *region; //Region of memory that the nodes point to
}block_t;
void *medalloc(size_t size);
int medfree(void *ptr);
void *step_through_nodes(void);
void *internal_medalloc(size_t size, int *err);
extern uint8_t  *medium_heap; // The memory heap for allocations 
extern uint8_t *nodes; // The nodes for the small memory heap;
extern size_t current_medium_heap_size; // The current size of the memory heap
extern size_t current_nodes_heap_size; // The current size of the nodes heap

#endif