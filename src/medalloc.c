#include "stdint.h"
#include "stddef.h"
#include "stdbool.h"
#include "init.h"
#include "def.h"
#include <stdio.h>
#include "alloc.h"
#include <string.h>
#include "medalloc.h"
uint8_t  *medium_heap = NULL; // The memory heap for allocations 
uint8_t *nodes = NULL; // The nodes for the medium memory heap;
size_t current_medium_heap_size = 0; // The current size of the memory heap





void *medalloc(size_t size)
{
    if (memory_medium_heap_initiated != true)
    {
        printf("Setting up memory heap\n");
        int ret = init_medium_memory_heap(); // Initialize the memory heap
        if (ret != 0)
        {
            printf("Failed to initialize the memory heap %d\n", ret);
            return NULL;
        }
        memory_medium_heap_initiated = true;
    }

    printf("Allocating memory of size %ld\n", size);
    block_t *current_node = (block_t *)nodes;

    // First, check if any single free node has enough space
    while (current_node != NULL)
    {
        if (current_node->magic != MAGIC_NUMBER)
        {
            printf("Nodes are corrupted\n");
            return NULL;
        }

        if (current_node->free == true && current_node->size >= size)
        {
            printf("Found free node\n");
            current_node->free = false;
            return (void *)current_node->region;
        }

        current_node = current_node->next;
    }

    // Now, we try to find multiple consecutive nodes to merge
    current_node = (block_t *)nodes;
    block_t *start_of_alloc = NULL; // Start of the merged allocation
    size_t found_size = 0;

    while (current_node != NULL)
    {
        if (current_node->magic != MAGIC_NUMBER)
        {
            printf("Nodes are corrupted\n");
            return NULL;
        }

        if (current_node->free == true)
        {
            // Start merging free nodes
            block_t *internal_current_block = current_node;
            start_of_alloc = current_node;
            found_size = 0;

            while (internal_current_block != NULL && internal_current_block->free == true)
            {
                found_size += internal_current_block->size;

                if (found_size >= size)
                {
                    // We've found enough space, now merge the nodes

                    // Mark all the intermediate nodes as used
                    block_t *tmp = current_node;
                    while (tmp != internal_current_block->next)
                    {
                        tmp->free = false;
                        tmp = tmp->next;
                    }

                    // Adjust the size of the first block (start_of_alloc)
                    start_of_alloc->size = found_size;
                    start_of_alloc->next = internal_current_block->next; // Skip the merged nodes

                    printf("Merged %ld bytes of memory\n", found_size);
                    return (void *)start_of_alloc->region;
                }

                // Move to the next free node
                internal_current_block = internal_current_block->next;
            }
        }

        current_node = current_node->next;
    }

    // If we can't find any available or mergeable space
    printf("Memory allocation failed, not enough free space\n");
    return NULL;
}

int medfree(void *ptr)
{
    if (ptr == NULL)
    {
        return -1;
    }
    block_t *current_node = (block_t *)nodes;
    while (current_node != NULL)
    {
        if(current_node->region == ptr)
        {
            current_node->free = true;
            return 0;
        }
        current_node = current_node->next;
    }
    return -1;
    
}