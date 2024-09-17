#include "stdint.h"
#include "stddef.h"
#include "stdbool.h"
#include "init.h"
#include "def.h"
#include <stdio.h>
#include "alloc.h"
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include "largealloc.h"

large_allocs_t *large_allocations;
bool large_allocations_inited = false;
size_t max_large_allocations;
size_t current_number_large_allocations;

void *large_alloc(size_t size)
{
    if(large_allocations_inited == false)
    {
        large_allocations = (large_allocs_t *)mmap(NULL, 200 * sizeof(large_allocs_t), PROT_READ | PROT_WRITE,MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if(large_allocations == MAP_FAILED)
        {
            return NULL;
        }
        max_large_allocations = 200;
        current_number_large_allocations = 0;
        large_allocations_inited = true;
                                            
    }
    size_t page_size = sysconf(_SC_PAGESIZE);
    //Convert size to the nearest page size
    size_t page_aligned_size = (size + page_size - 1) & ~(page_size - 1);
    // printf("Allocating %ld bytes\n",page_aligned_size);
    void *allocation = mmap(NULL, page_aligned_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    
    if(allocation == MAP_FAILED)
    {
        return NULL;
    }
    large_allocations[current_number_large_allocations].size = page_aligned_size;
    large_allocations[current_number_large_allocations].ptr = allocation;
    large_allocations[current_number_large_allocations].magic = MAGIC_NUMBER;
    current_number_large_allocations++;
    return allocation;
    // return NULL;
}

int large_free(void *ptr)
{
    if(ptr == NULL)
    {
        return -1;
    }
    for (size_t i = 0; i < current_number_large_allocations; i++)
    {
        if(large_allocations[i].magic == MAGIC_NUMBER && large_allocations[i].ptr == ptr)
        {
            size_t size = large_allocations[i].size;
            int ret = munmap(ptr,size);
            return ret;

        }
    }
    return -3;
    

}