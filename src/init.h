#ifndef __INIT__H
#define __INTI__H

#include "stdint.h"
#include "stdbool.h"

int init_small_memory_heap(void);
int setup_bitmap(void);

int init_medium_memory_heap(void);
int setup_nodes(void);
extern bool memory_small_heap_initiated;
extern bool memory_medium_heap_initiated;
#endif