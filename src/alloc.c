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
    // printf("Well this happend\n");
    return NULL;
}

int cfree(void *ptr)
{
    if(ptr == NULL)
    {
        return -1;
    }
    if(memory_small_heap_initiated != true || memory_medium_heap_initiated != true)
    {
        return -2;
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
        return -3;
    }
}