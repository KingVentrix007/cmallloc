#ifndef __INIT__H
#define __INTI__H

#include "stdint.h"
#include "stdbool.h"
#include <stddef.h>

//Small memory heap functions
int init_small_memory_heap(void);
int setup_bitmap(void);

//Medium memory heap functions
int init_medium_memory_heap(void);
int setup_nodes(void);
int expand_medium_region(void);
int expand_medium_heap(void);
int expand_medium_nodes(void);
size_t calculate_list_memory(size_t heap_size, size_t block_size, size_t node_size);
int setup_expand_medium_nodes(void);


extern bool memory_small_heap_initiated;
extern bool memory_medium_heap_initiated;
#endif