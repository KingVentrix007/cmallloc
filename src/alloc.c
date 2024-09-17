#include "stdint.h"
#include "stddef.h"
#include "stdbool.h"
#include "init.h"
#include "def.h"
#include "smallalloc.h"
#include <stdio.h>
#include "alloc.h"
#include <string.h>
#include <unistd.h>
#include "largealloc.h"
#include "medalloc.h"
void *small_region_start = NULL; //Start address for the small allocations
void *small_region_end = NULL;// End address for the small allocations



void *medium_region_start = NULL; //Start address for the medium allocations
void *medium_region_end = NULL; //End address for the medium allocations




void *cmalloc(size_t size)
{
    // int 0;
    // printf("Calling cmalloc\n");
    long page_size = sysconf(_SC_PAGESIZE);
    if(size < ALLOC_THRESHOLD)
    {
        if(memory_small_heap_initiated != true)
        {
            // printf("Setting up memory heap\n");
            int ret = init_small_memory_heap(); // Initialize the memory heap
            if(ret != 0)
            {
                printf("Failed to initialize the memory heap %d\n",ret);
                return NULL;
            }
            small_region_start = small_heap;
            small_region_end = small_heap+current_small_heap_size;
            memory_small_heap_initiated = true;
        }
        return small_malloc(size);
    }
    else if (size >= ALLOC_THRESHOLD && size <= (size_t)page_size)
    {
        if(memory_medium_heap_initiated != true)
        {
            // printf("Setting up memory heap\n");
            int ret = init_medium_memory_heap(); // Initialize the memory heap
            if(ret != 0)
            {
                printf("Failed to initialize the memory heap %d\n",ret);
                return NULL;
            }
            medium_region_start = medium_heap;
            medium_region_end = medium_heap+current_medium_heap_size;
            memory_medium_heap_initiated = true;
        }
        return medalloc(size);
    }
    else if (size > (size_t)page_size)
    {
        return large_alloc(size);
    }
    printf("I have absolutely no idea what you passed to get here\n");
    // printf("Well this happend\n");
    return NULL;
}

int cfree(void *ptr)
{
    if(ptr == NULL)
    {
        return -1;
    }
    if(ptr >= small_region_start && ptr < small_region_end)
    {
        return small_free(ptr);
    }
    else if(ptr >= medium_region_start && ptr < medium_region_end)
    {
        return medfree(ptr);
    }
    else
    {
        return large_free(ptr);
    }
}

void *crealloc(void *ptr, unsigned int old_size, unsigned int new_size)
{
    void * new_adder = cmalloc(new_size);
    if(new_adder == NULL)
    {
        return NULL;
    }
    memcpy(new_adder,ptr,old_size);
    memset(ptr,0,old_size);
    cfree(ptr);
    return new_adder;
}

void* ccalloc(size_t num_elements, size_t element_size) {
    // Calculate total memory required
    size_t total_size = num_elements * element_size;

    // Use your custom malloc to allocate the memory
    void* ptr = cmalloc(total_size);

    // If allocation was successful, initialize memory to zero
    if (ptr != NULL) {
        memset(ptr, 0, total_size); // Set all bytes to 0
    }

    return ptr; // Return the allocated and zero-initialized memory
}