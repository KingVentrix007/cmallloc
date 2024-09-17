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
size_t current_nodes_heap_size = 0; // The current size of the nodes heap


void *medalloc(size_t size)
{
    int error;
    size_t loop_count = 0;
    void *ptr = internal_medalloc(size, &error);
    while(error == -1)
    {
        ptr = internal_medalloc(size, &error);
        loop_count++;
        if(loop_count >= 100)
        {
            printf("Max attempted allocations reached\n");
            return NULL;
        }
    }
    if(error == 0)
    {
        return ptr;
    }
    return NULL;
}

void *internal_medalloc(size_t size, int *err)
{
    if (memory_medium_heap_initiated != true)
    {
        // printf("Setting up memory heap\n");
        int ret = init_medium_memory_heap(); // Initialize the memory heap
        if (ret != 0)
        {
            printf("Failed to initialize the memory heap %d\n", ret);
            *err = -2;
            return NULL;
        }
        memory_medium_heap_initiated = true;
    }
    // #ifdef P
    // printf("Allocating memory of size %ld\n", size);
    block_t *current_node = (block_t *)nodes;

    // First, check if any single free node has enough space
    while (current_node != NULL)
    {
        if (current_node->magic != MAGIC_NUMBER)
        {
            printf("Nodes are corrupted\n");
            *err = -3;

            return NULL;
        }

        if (current_node->free == true && current_node->size >= size)
        {
            // printf("Found free node\n");
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
            *err = -3;

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

                    // printf("Merged %ld bytes of memory\n", found_size);
                    *err = 0;
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
    *err = -1;
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
void *step_through_nodes(void)
{
    printf("Stepping through nodes\n");

    size_t num_nodes = current_nodes_heap_size / sizeof(block_t); // Calculate number of nodes
    printf("Total number of nodes: %ld\n", num_nodes-1);

    block_t *current_node = (block_t *)nodes; // Start from the first node
    void *end_adder = NULL;
    for (size_t i = 0; i < num_nodes-1; i++)
    {
        if (current_node == NULL) {
            printf("Node %ld is NULL, something is wrong\n", i);
            break;
        }

        // Print details about the current node
        printf("Node %ld: \n", i);
        printf("  Free: %s\n", current_node->free ? "true" : "false");
        printf("  Size: %ld\n", current_node->size);
        printf("  Region: %p\n", current_node->region);
        printf("  Magic: %x\n", current_node->magic);
        end_adder = current_node->region;
        // Sanity check: Ensure that the node has the correct magic number
        if (current_node->magic != MAGIC_NUMBER) {
            printf("  ERROR: Magic number does not match!\n");
            printf("  Traversed through %ld nodes of %ld\n", i, num_nodes-1);
            break;
        }

        // Move to the next node
        current_node = current_node->next;

        // If the next pointer is NULL and we're not at the end, we stop early
        if (current_node == NULL && i != num_nodes - 2) {
            printf("Early termination: Node %ld's next pointer is NULL\n", i);
            break;
        }
    }
    if(end_adder != NULL)
    {
        printf("Node %ld: \n", num_nodes-1);
        if(end_adder == NULL)
        {
            // printf("Node %ld is NULL, something is wrong\n", num_nodes-1);
            return NULL;
        }
        return end_adder;
    }
    return NULL;
}
