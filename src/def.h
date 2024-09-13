#ifndef __DEF__H
#define __DEF__H

#define ALLOC_THRESHOLD 1024
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

#endif