#ifndef __SMALLALLOC__H
#define __SMALLALLOC__H
#include "stdint.h"
#include "stddef.h"
#include "stdbool.h"

//Function Prototypes
void *small_malloc(size_t size);
int small_free(void *ptr);

//Private functions
void *salloc(size_t size);
int sfree(void *ptr);
void print_bits(size_t start_bit, size_t num_bits);
//variables
extern uint8_t *small_heap;
extern size_t current_small_heap_size;
extern uint8_t *bitmap;
#endif