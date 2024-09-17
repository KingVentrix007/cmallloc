#include "stdint.h"
#include "stddef.h"
#include "stdbool.h"
#include "init.h"
#include "def.h"
#include "smallalloc.h"
#include <stdio.h>
#include "alloc.h"
#include <string.h>
//Flag to determine if the memory heap is setup

uint8_t  *small_heap = NULL; // The memory heap for allocations less than MAX_SIZE_SMALL_ALLOC bytes;
uint8_t *bitmap = NULL; // The bitmap for the small memory heap;
size_t current_small_heap_size = 0; // The current size of the memory heap
int init_count = 0;

//Function to allocate memory for small allocations
void *small_malloc(size_t size) {
    if(memory_small_heap_initiated != true)
    {
        // printf("Setting up memory heap\n");
        int ret = init_small_memory_heap(); // Initialize the memory heap
        if(ret != 0)
        {
            printf("Failed to initialize the memory heap %d\n",ret);
            return NULL;
        }
        memory_small_heap_initiated = true;
        init_count++;
        if(init_count > 1)
        {
            printf("There is a problem\n");
            return NULL;
        }
    }
    #ifdef PRINTF_DEBUG
    printf("Allocating memory of size %ld\n",size);
    #endif
    return salloc(size);
}
void *salloc(size_t size)
{
    size+=sizeof(memory_alloc_t);
    size_t num_blocks_needed = (size + SMALL_BLOCK_SIZE - 1) / SMALL_BLOCK_SIZE;
    #ifdef PRINTF_DEBUG
    printf("Trying to allocate %ld blocks of %d bytes for %ld sized region\n",num_blocks_needed,SMALL_BLOCK_SIZE,size);
    #endif
    bool found_start = false;
    size_t start_byte_index = 0;
    int start_bit_index = 0;
    #ifdef PRINTF_DEBUG
    size_t end_byte_index = 0;
    int end_bit_index = 0;
    #endif
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
            #ifdef PRINTF_DEBUG
            end_byte_index = i;
            end_bit_index = 8;
            #endif
           
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
                    #ifdef PRINTF_DEBUG
                    end_byte_index = i;
                    end_bit_index = bit_index;
                    #endif
                }
                else
                {
                    found_start = false;
                }
                #ifdef PRINTF_DEBUG
                printf("Checked Byte %zu, Bit %d: %d", i, bit_index, bit);
                if(bit == 0)
                {
                    printf(" Valid\n");
                }
                else
                {
                    printf(" Invalid\n");
                }
                #endif
            }
        }
        if(num_found_bits >= num_blocks_needed)
        {
            #ifdef PRINTF_DEBUG
            printf("Found %zu bits, starting at byte %zu, bit %d ending at byte %zu, bit %d\n", num_found_bits, start_byte_index, start_bit_index, end_byte_index, end_bit_index);
            printf("Allocted region of memory of size %ld\n",num_found_bits*SMALL_BLOCK_SIZE);
            #endif
            void *adder = small_heap + (start_byte_index * 8 + (size_t)start_bit_index) * SMALL_BLOCK_SIZE;

            size_t bits_to_set = num_blocks_needed;
            size_t current_byte = start_byte_index;
            int current_bit = start_bit_index;

            while (bits_to_set > 0) 
            {
                bitmap[current_byte] |= (uint8_t)(1 << (7 - current_bit));
                uint8_t bit = (bitmap[current_byte] >> (7 - current_bit)) & 1;
                if(bit != 1)
                {
                    printf("Byte %zu, Bit %d: %d Is not set\n", current_byte, current_bit, bit);
                }
                bits_to_set--;
                current_bit++;
                if (current_bit == 8) {
                    current_bit = 0;
                    current_byte++;
                }
            }
            #ifdef DEBUG
            bits_to_set = num_blocks_needed;
            current_byte = start_byte_index;
            current_bit = start_bit_index;

            while (bits_to_set > 0) 
            {
                uint8_t bit = (bitmap[current_byte] >> (7 - current_bit)) & 1;
                printf("Byte %zu, Bit %d: %d\n", current_byte, current_bit, bit);
                bits_to_set--;
                current_bit++;
                if (current_bit == 8) {
                    current_bit = 0;
                    current_byte++;
                }
            }
            #endif
            memory_alloc_t data;
            data.size = size -= sizeof(memory_alloc_t);
            data.ptr =  (void *)((char *)adder + sizeof(memory_alloc_t)); 
            data.dirty = false;
            data.magic = MAGIC_NUMBER;
            memcpy(adder, &data, sizeof(memory_alloc_t));
            #ifdef PRINTF_DEBUG
            printf("data %p\n",data.ptr);
            #endif

            return data.ptr;
            // return adder;
        }
    }
    return NULL;
    
}


void print_bits(size_t start_bit, size_t num_bits) {
    size_t byte_pos = start_bit / 8;
    size_t bit_pos = start_bit % 8;
    for (size_t i = 0; i < num_bits; i++) {
        printf("%d", (bitmap[byte_pos] & (1 << (7 - bit_pos))) ? 1 : 0);
        bit_pos++;
        if (bit_pos == 8) {
            bit_pos = 0;
            byte_pos++;
        }
        if ((i + 1) % 8 == 0) {
            printf(" ");
        }
    }
    printf("\n");
}

/**
 * Frees a small memory block.
 *
 * This function is responsible for freeing a small memory block that was 
 * previously allocated. It ensures that the memory is properly deallocated 
 * and can be reused by the system.
 *
 * @param ptr A pointer to the memory block to be freed. This pointer must 
 *            have been returned by a previous call to a memory allocation 
 *            function.
 * @return An integer indicating the success or failure of the operation.
 *         Typically, 0 indicates success, while a non-zero value indicates 
 *         an error.
 */
int small_free(void *ptr)
{
    if(ptr == NULL)
    {
        return -1;
    }

    memory_alloc_t *data = (memory_alloc_t *)((char *)ptr - sizeof(memory_alloc_t));
    if(data->magic != MAGIC_NUMBER)
    {
        printf("Data struct is invalid\n");
        return -1;
    }
    ptr = (char *)ptr - sizeof(memory_alloc_t);
    printf("data %p\n", data->ptr);

    size_t num_blocks_needed = (data->size + sizeof(memory_alloc_t) + SMALL_BLOCK_SIZE - 1) / SMALL_BLOCK_SIZE;

    size_t block_number = ((uintptr_t)ptr - (uintptr_t)small_heap) / SMALL_BLOCK_SIZE;
size_t start_byte_index = block_number / 8;
size_t start_bit_index = block_number % 8;
    printf("Starting at byte %zu, bit %zu\n", start_byte_index, start_bit_index);

    size_t bits_to_set = num_blocks_needed;
    size_t current_byte = start_byte_index;
    int current_bit = start_bit_index;
    printf("Unsetting %zu bits\n", bits_to_set);

    while (bits_to_set > 0)
    {
        if (current_byte >= sizeof(bitmap)) {
            printf("Error: Byte index out of bounds\n");
            return -1;
        }
        
        uint8_t mask = ~(1 << (7 - current_bit));

        bitmap[current_byte] &= mask;  // Unset the bit
        uint8_t new_bit = (bitmap[current_byte] >> (7 - current_bit)) & 1;
        if ((bitmap[current_byte] & (1 << (7 - current_bit))) != 0)
        {
            printf("Error: Bit %d in byte %zu is still set\n", current_bit, current_byte);
            return -1;
        }
        // printf("Cleared bit %d in byte %zu, bitmap[current_byte] == 0x%02X\n", current_bit, current_byte, bitmap[current_byte]);
        #ifdef DEBUG
        printf("Byte %ld, Bit %d: %d\n", current_byte, current_bit, new_bit);
        #endif
        bits_to_set--;
        current_bit++;
        if (current_bit == 8) {
            current_bit = 0;
            current_byte++;
        }
    }
    return 0;
}


int sfree(void *ptr)
{
    printf("%p\n",ptr);
    return 0;
}