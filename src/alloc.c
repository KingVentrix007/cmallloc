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

#ifdef CMALLOC_LIB 
void *malloc(size_t size)
{
    // printf("Calling malloc\n");
    return cmalloc(size);
}
void free(void *ptr)
{
    cfree(ptr);
}
void *realloc(void *ptr,size_t new_size)
{
    size_t old_size = get_size_cmalloc(ptr);
    if(old_size == 0)
    {
        return NULL;
    }
    return crealloc(ptr,old_size,new_size);
}
void *calloc(size_t __nmemb, size_t __size)
{
    return ccalloc(__nmemb,__size);
}
__attribute__((constructor))
void cmalloc_init() {
    // Try minimal code first
    // printf("cmalloc.so initialized\n");
    int small_ret = init_small_memory_heap();
    int med_ret = init_medium_memory_heap();
    int large_ret = init_large_allocations();
    if(small_ret != 0 || med_ret != 0 || large_ret != 0)
    {
        _exit(-1);
    }
    small_region_start = small_heap;
    small_region_end = small_heap+current_small_heap_size;
    medium_region_start = medium_heap;
    medium_region_end = medium_heap+current_medium_heap_size;
}

#endif



void *cmalloc(size_t size)
{
    
    long page_size = sysconf(_SC_PAGESIZE);
    if(size < ALLOC_THRESHOLD)
    {
        if(memory_small_heap_initiated != true)
        {
            // printf_debug("Setting up memory heap\n");
            int ret = init_small_memory_heap(); // Initialize the memory heap
            if(ret != 0)
            {
                #ifdef DEBUG
                printf_debug("Failed to initialize the memory heap %d\n",ret);
                #endif
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
            // printf_debug("Setting up memory heap\n");
            int ret = init_medium_memory_heap(); // Initialize the memory heap
            if(ret != 0)
            {
                #ifdef DEBUG
                printf_debug("Failed to initialize the memory heap %d\n",ret);
                #endif
                return NULL;
            }
            medium_region_start = medium_heap;
            medium_region_end = medium_heap+current_medium_heap_size;
            memory_medium_heap_initiated = true;
        }
        void *ptr =  medalloc(size);
        if(ptr >= medium_region_start && ptr < medium_region_end)
        {
            return ptr;
        }
        else
        {
            #ifdef DEBUG
            printf_debug("ptr is invalid\n");
            #endif
            return NULL;
        }
        
    }
    else if (size > (size_t)page_size)
    {
        return large_alloc(size);
    }

    printf_debug("I have absolutely no idea what you passed to get here\n");
    
    return NULL;
}


size_t get_size_cmalloc(const void *ptr)
{
    if(small_region_start == NULL || small_region_end == NULL || medium_region_start == NULL || medium_region_end)
    {
        return 0;
    }
    if(ptr == NULL)
    {
        return 0;
    }
    else if(ptr >= small_region_start && ptr < small_region_end)
    {
        return get_smallalloc_size(ptr);
    }
    else if(ptr >= medium_region_start && ptr < medium_region_end)
    {
        return get_medalloc_size(ptr);
    }
    else
    {
        return get_large_alloc_size(ptr);
        
    }
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
    if(memory_medium_heap_initiated == false || memory_small_heap_initiated == false)
    {
        return NULL;
    }
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