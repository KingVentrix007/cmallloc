#ifndef __INIT__H
#define __INTI__H

#include "stdint.h"
#include "stdbool.h"

int init_small_memory_heap(void);
int setup_bitmap(void);

extern bool memory_small_heap_initiated;
#endif