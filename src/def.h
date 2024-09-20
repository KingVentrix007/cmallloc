#ifndef __DEF__H
#define __DEF__H
#include "medalloc.h"
#include <sys/mman.h>
#define ALLOC_THRESHOLD 1024 // Max size of a small allocations
#define MAX_SIZE_SMALL_ALLOC 256 // the max size of a small allocation
#define STARTING_SIZE_SMALL_ALLOC 1024 // The starting number of small allocations

// Define the size of the small heap and small block
#define SMALL_HEAP_SIZE (MAX_SIZE_SMALL_ALLOC * STARTING_SIZE_SMALL_ALLOC) // 1 MB small heap
#define SMALL_BLOCK_SIZE 16 // 16 bytes per small block




// Calculate number of blocks in small heap
#define SMALL_HEAP_BLOCKS (SMALL_HEAP_SIZE / SMALL_BLOCK_SIZE)

// Calculate bitmap size in bytes (round up to nearest byte)
#define SMALL_HEAP_BITMAP_SIZE ((SMALL_HEAP_BLOCKS + 7) / 8)

// Calculate bitmap size in words (assuming 32-bit words)
#define SMALL_HEAP_BITMAP_WORDS ((SMALL_HEAP_BITMAP_SIZE + 3) / 4)

#define SMALL_BUFFER_REGION_SIZE SMALL_BLOCK_SIZE // Size of small buffer protection region


#define MAGIC_NUMBER 0x5464


#ifdef DEBUG
    #define printf_debug(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
    #define printf_debug(fmt, ...) // Do nothing if DEBUG is not defined
#endif




#define MEDIUM_HEAP_SIZE 1024 * 1024
#define MEDIUM_BLOCK_SIZE 1024

#define MEDIUM_HEAP_NODES_SIZE (MEDIUM_HEAP_SIZE/MEDIUM_BLOCK_SIZE)*sizeof(block_t)
#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS 0x20
#endif
#endif