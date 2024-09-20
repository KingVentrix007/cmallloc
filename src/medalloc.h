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
    bool is_end;
}block_t;
typedef struct
{
    size_t invalid_nodes;
    size_t allocated_nodes;
    size_t allocated_space;
    size_t total_space;
}medium_analysis_t;
void *medalloc(size_t size);
int medfree(void *ptr);
void *step_through_nodes(void);
void *internal_medalloc(size_t size, int *err);
medium_analysis_t *find_invalid_nodes(const void *ptr_find);
size_t get_medalloc_size(const void *ptr);
void display_medalloc_region(block_t *data,size_t check_size);
extern uint8_t  *medium_heap; // The memory heap for allocations 
extern uint8_t *nodes; // The nodes for the small memory heap;
extern size_t current_medium_heap_size; // The current size of the memory heap
extern size_t current_nodes_heap_size; // The current size of the nodes heap

#endif