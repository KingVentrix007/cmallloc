#include "stdint.h"
#include "stddef.h"
#include "stdbool.h"
#include "init.h"
#include "def.h"
#include "smallalloc.h"
#include <stdio.h>
//Flag to determine if the memory heap is setup

uint8_t  *small_heap = NULL; // The memory heap for allocations less than MAX_SIZE_SMALL_ALLOC bytes;
uint8_t *bitmap = NULL; // The bitmap for the small memory heap;
size_t current_small_heap_size = 0; // The current size of the memory heap


//Function to allocate memory for small allocations
void *small_malloc(size_t size) {
    if(memory_small_heap_initiated != true)
    {
        printf("Setting up memory heap\n");
        int ret = init_small_memory_heap(); // Initialize the memory heap
        if(ret != 0)
        {
            printf("Failed to initialize the memory heap %d\n",ret);
            return NULL;
        }
    }
    printf("Allocating memory of size %ld\n",size);
    return salloc(size);
}
void *salloc(size_t size)
{
    size_t num_blocks_needed = (size + SMALL_BLOCK_SIZE - 1) / SMALL_BLOCK_SIZE;
    printf("Trying to allocate %ld blocks of %d bytes for %ld sized region\n",num_blocks_needed,SMALL_BLOCK_SIZE,size);

    bool found_start = false;
    size_t start_byte_index = 0;
    int start_bit_index = 0;

    size_t end_byte_index = 0;
    int end_bit_index = 0;
    size_t num_found_bits = 0;
    printf("%p\n",bitmap);
    for (size_t i = 0; i < SMALL_HEAP_BITMAP_SIZE; i++)
    {
        if(bitmap[i] == 0)
        {
            if(found_start == false)
            {
                start_byte_index = i;
                start_bit_index = 0;
                found_start = true;
            }
            num_found_bits+=8;
            end_byte_index = i;
            end_bit_index = 0;
           
        }
        else
        {
            uint8_t byte = bitmap[i];
            for (int bit_index = 7; bit_index >= 0; --bit_index) 
            {
                // Check if the bit is 1 or 0 using bitwise AND
                uint8_t bit = (byte >> i) & 1;
                if(bit == 0)
                {
                    if(found_start == false)
                    {
                        start_byte_index = i;
                        start_bit_index = bit_index;
                        found_start = true;
                    }
                    num_found_bits++;
                    end_byte_index = i;
                    end_bit_index = bit_index;
                }
                else
                {
                    found_start = false;
                }
                
                printf("Byte %zu, Bit %d: %d\n", i, bit_index, bit);
            }
        }
        if(num_found_bits >= num_blocks_needed)
        {
            printf("Found %zu bits, starting at byte %zu, bit %d ending at byte %zu, bit %d\n", num_found_bits, start_byte_index, start_bit_index, end_byte_index, end_bit_index);
            return NULL;
        }
    }
    return NULL;
    
}