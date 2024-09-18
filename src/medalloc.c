#include "stdint.h"
#include "stddef.h"
#include "stdbool.h"
#include "init.h"
#include "def.h"
#include <stdio.h>
#include "alloc.h"
#include <string.h>
#include "medalloc.h"
#include <stdlib.h>
// Error codes
#define ERR_NO_SPACE -1
#define ERR_INIT_FAIL -2
#define ERR_CORRUPTED_NODE -3
#define ERR_MAX_ATTEMPTS -4
#define ERR_NULL_PTR -5
#define ERR_NOT_FOUND -6

uint8_t *medium_heap = NULL; // The memory heap for allocations 
uint8_t *nodes = NULL; // The nodes for the medium memory heap
size_t current_medium_heap_size = 0; // The current size of the memory heap
size_t current_nodes_heap_size = 0; // The current size of the nodes heap
size_t allocated_space = 0;

// Function to allocate memory from the medium heap
void *medalloc(size_t size)
{
    int error;
    size_t loop_count = 0;
    void *ptr = internal_medalloc(size, &error);

    // Retry allocation if there is no space
    while (error == ERR_NO_SPACE)
    {
        int ret = expand_medium_region();
        if(ret != 0)
        {
            printf("Failed to expand medium memory\n");
            return NULL;
        }
        ptr = internal_medalloc(size, &error);
        loop_count++;
        if (loop_count >= 100)
        {
            printf("Max attempted allocations reached\n");
            return NULL;
        }
        // printf("Allocation error: %d\n", error);
    }

    // Check for other errors
    if (error != 0)
    {
        printf("Allocation failed with error code: %d\n", error);
        return NULL;
    }

    allocated_space += size;
    return ptr;
}

// Internal function to handle the actual allocation logic
void *internal_medalloc(size_t size, int *err)
{
    // Initialize memory heap if not already done
    if (memory_medium_heap_initiated != true)
    {
        int ret = init_medium_memory_heap(); // Initialize the memory heap
        if (ret != 0)
        {
            printf("Failed to initialize the memory heap: %d\n", ret);
            *err = ERR_INIT_FAIL;
            return NULL;
        }
        memory_medium_heap_initiated = true;
    }

    // Check if there is enough space
    if (allocated_space + size >= current_medium_heap_size)
    {
        printf("Not enough space in the medium heap\n");
        expand_medium_region();
    }

    block_t *current_node = (block_t *)nodes;

    // First, check if any single free node has enough space
    size_t single_node_counter = 0;
    while (current_node != NULL)
    {
        if (current_node->magic != MAGIC_NUMBER)
        {
            printf("Nodes(Number %ld) are corrupted in single block allocation\n", single_node_counter);
            medium_analysis_t *allocs = find_invalid_nodes();
            printf("Analysis of medium heap\n");
            printf("\tCorrupted Nodes: %ld\n",allocs->invalid_nodes);
            printf("\tAllocated Nodes: %ld\n",allocs->allocated_nodes);

            printf("\tAllocated Space: %ld\n",allocs->allocated_space);
            
            printf("\tTotal space:      %ld\n",current_medium_heap_size);
            printf("\tTotal Node space: %ld\n",allocs->total_space);
            printf("\tNode space exceeds total space: %s (Total Space: %zu, Medium Heap Size: %zu)\n",
       allocs->total_space > current_medium_heap_size ? "true" : "false",
       allocs->total_space, current_medium_heap_size);


            
            
            *err = ERR_CORRUPTED_NODE;
            return NULL;
        }

        if (current_node->free == true && current_node->size >= size)
        {
            current_node->free = false;
            *err = 0;
            return (void *)current_node->region;
        }
        single_node_counter++;
        current_node = current_node->next;
    }

    // Now, try to find multiple consecutive nodes to merge
    current_node = (block_t *)nodes;
    block_t *start_of_alloc = NULL; // Start of the merged allocation
    size_t found_size = 0;

    while (current_node != NULL)
    {
        if (current_node->magic != MAGIC_NUMBER)
        {
            printf("Nodes are corrupted in multiblock allocation\n");
            *err = ERR_CORRUPTED_NODE;
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
    // printf("Memory allocation failed, not enough free space\n");
    *err = ERR_NO_SPACE;
    return NULL;
}

// Function to free allocated memory
int medfree(void *ptr)
{
    if (ptr == NULL)
    {
        return ERR_NULL_PTR;
    }

    block_t *current_node = (block_t *)nodes;
    while (current_node != NULL)
    {
        if (current_node->region == ptr)
        {
            current_node->free = true;
            allocated_space -= current_node->size;
            return 0;
        }
        current_node = current_node->next;
    }

    return ERR_NOT_FOUND;
}

// Function to step through nodes and print their details
void *step_through_nodes(void)
{
    printf("Stepping through nodes\n");

    size_t num_nodes = current_nodes_heap_size / sizeof(block_t); // Calculate number of nodes
    printf("Total number of nodes: %ld\n", num_nodes - 1);

    block_t *current_node = (block_t *)nodes; // Start from the first node
    void *end_adder = NULL;
    for (size_t i = 0; i < num_nodes - 1; i++)
    {
        if (current_node == NULL)
        {
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
        if (current_node->magic != MAGIC_NUMBER)
        {
            printf("  ERROR: Magic number does not match!\n");
            printf("  Traversed through %ld nodes of %ld\n", i, num_nodes - 1);
            break;
        }

        // Move to the next node
        current_node = current_node->next;

        // If the next pointer is NULL and we're not at the end, we stop early
        if (current_node == NULL && i != num_nodes - 2)
        {
            printf("Early termination: Node %ld's next pointer is NULL\n", i);
            break;
        }
    }

    if (end_adder != NULL)
    {
        printf("Node %ld: \n", num_nodes - 1);
        if (end_adder == NULL)
        {
            return NULL;
        }
        return end_adder;
    }

    return NULL;
}

//Searches for nodes that are invalid. 
medium_analysis_t *find_invalid_nodes()
{
    //!WARNING
    // This code operates under the assumption that all of the node region was zero'd
    block_t *current_node = (block_t *)nodes;
    size_t invalid_nodes = 0;
    size_t allocated_nodes = 0;
    size_t medium_allocated_space = 0;
    size_t total_space = 0;
    size_t i = 0;

    while (current_node != NULL)
    {
        // Bounds check
        if ((void *)current_node < (void *)nodes || (void *)current_node >= (void *)((char *)nodes + current_nodes_heap_size))
        {
            printf("Node %ld exceeds legal bounds\n", i);
            printf("\tNode: %p\n", current_node);
            printf("\tStart: %p\n", nodes);
            printf("\tEnd: %p\n", (char *)nodes + current_nodes_heap_size);

            // Attempting to repair the last valid node
            block_t *last_valid_node = (block_t *)nodes + (i - 1);
            if(last_valid_node == NULL)
            {
                printf("Last valid node is NULL\n");
                break;
            }
            if (last_valid_node->next != NULL)
            {
                printf("Attempting to repair nodes\n");
                if (last_valid_node->next == current_node)
                {
                    printf("Node %ld points to corrupted node %ld\n", i - 1, i);
                }
                last_valid_node->next = NULL;
                last_valid_node->is_end = true;
            }
            break;
        }

        // Check if node is corrupted
        if (current_node->magic != MAGIC_NUMBER)
        {
            printf("Node %ld is corrupted\n", i);
            invalid_nodes++;

            // Reset to the next node (continue the loop)
            current_node = (block_t *)nodes + i; // Correct pointer arithmetic
            if (current_node == NULL) // Use NULL instead of 0 for pointer checks
            {
                printf("2. Node %ld is corrupted\n", i);
                break;
            }
        }
        else
        {
            // Valid node
            if (!current_node->free) // Node is allocated
            {
                allocated_nodes++;
                medium_allocated_space += current_node->size;
            }
            total_space+=current_node->size;
            current_node = current_node->next; // Move to next node
        }

        i++;
    }

    // Allocate and populate the analysis struct
    medium_analysis_t *analysis = malloc(sizeof(medium_analysis_t));
    if (analysis == NULL)
    {
        printf("Memory allocation failed for analysis struct\n");
        return NULL; // Handle malloc failure
    }

    analysis->allocated_nodes = allocated_nodes;
    analysis->allocated_space = medium_allocated_space;
    analysis->invalid_nodes = invalid_nodes;
    analysis->total_space = total_space;
    return analysis;
}
