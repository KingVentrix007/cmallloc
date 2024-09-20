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
    size+=1;
    size_t loop_count = 0;
    void *ptr = internal_medalloc(size, &error);
    if(ptr != NULL && error == 0)
    {
        allocated_space += size;
        return ptr;
    }
    // Retry allocation if there is no space
    while (error == ERR_NO_SPACE)
    {
        printf_debug("Attempting to expand memory\n");
        int ret = expand_medium_region();
        if(ret != 0)
        {
            printf_debug("Failed to expand medium memory\n");
            return NULL;
        }
        ptr = internal_medalloc(size, &error);
        if(ptr != NULL)
        {
            memset(ptr,1,size);
            // memcmp
            char *test_ptr = (char *)ptr;
            memset(test_ptr,1,size);
            printf_debug("Size %ld\n",size);
            for (size_t i = 0; i < size; i++)
            {
                if(test_ptr[i] != 1)
                {
                    printf_debug("Failed alloc\n");
                    return NULL;
                }
            }
            
        }
        loop_count++;
        if (loop_count >= 100)
        {
            printf_debug("Max attempted allocations reached\n");
            return NULL;
        }
    }

    // Check for other errors
    if (error != 0)
    {
        printf_debug("Allocation failed with error code: %d\n", error);
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
            printf_debug("Failed to initialize the memory heap: %d\n", ret);
            *err = ERR_INIT_FAIL;
            return NULL;
        }
        memory_medium_heap_initiated = true;
    }

    // Check if there is enough space
    if (allocated_space + size >= current_medium_heap_size)
    {
        printf_debug("Not enough space in the medium heap\n");
        expand_medium_region();
    }

    block_t *current_node = (block_t *)nodes;

    // First, check if any single free node has enough space
    size_t single_node_counter = 0;
    while (current_node != NULL)
    {
        if (current_node->magic != MAGIC_NUMBER)
        {
            printf_debug("Nodes(Number %ld) are corrupted in single block allocation\n", single_node_counter);
            find_invalid_nodes(NULL);

            


            
            
            *err = ERR_CORRUPTED_NODE;
            return NULL;
        }

        if (current_node->free == true && current_node->size >= size)
        {
            current_node->free = false;
            *err = 0;
            return current_node->region;
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
            printf_debug("Nodes are corrupted in multiblock allocation\n");
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
                    printf_debug("Merging nodes\n");
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
    // printf_debug("Memory allocation failed, not enough free space\n");
    // expand_medium_region();
    *err = ERR_NO_SPACE;
    return NULL;
}

// Function to free allocated memory
int medfree(const void *ptr)
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
            allocated_space -= current_node->size;
            return 0;
        }
        if(current_node->next != NULL)
        {
            current_node = current_node->next;
        }
        else
        {
            return ERR_NOT_FOUND;
        }
       
    }

    return ERR_NOT_FOUND;
}

// Function to step through nodes and print their details
void *step_through_nodes(void)
{
    printf_debug("Stepping through nodes\n");

    size_t num_nodes = current_nodes_heap_size / sizeof(block_t); // Calculate number of nodes
    printf_debug("Total number of nodes: %ld\n", num_nodes - 1);

    block_t *current_node = (block_t *)nodes; // Start from the first node
    void *end_adder = NULL;
    for (size_t i = 0; i < num_nodes - 1; i++)
    {
        if (current_node == NULL)
        {
            printf_debug("Node %ld is NULL, something is wrong\n", i);

            break;
        }
        else if (current_node->magic != MAGIC_NUMBER)
        {
            printf_debug("  ERROR: Magic number does not match!\n");
            printf_debug("  Traversed through %ld nodes of %ld\n", i, num_nodes - 1);
            break;
        }
        else if (current_node->next == NULL)
        {
             printf_debug("Node %ld: \n", i);
            printf_debug("  Free: %s\n", current_node->free ? "true" : "false");
            printf_debug("  Size: %ld\n", current_node->size);
            printf_debug("  Region: %p\n", current_node->region);
            printf_debug("  Magic: %x\n", current_node->magic);
            end_adder = current_node->region;

            // Sanity check: Ensure that the node has the correct magic number
            
            // Move to the next node
            current_node = current_node->next;
        }
    }

    if (end_adder != NULL)
    {
        printf_debug("Node %ld: \n", num_nodes - 1);
        if (end_adder == NULL)
        {
            return NULL;
        }
        return end_adder;
    }

    return NULL;
}

void display_medalloc_region(block_t *data,size_t check_size)
{
    for (size_t i = 0; i < data->size+check_size; i++)
    {
        printf_debug("%x",((char *)(data->region))[i]);
        if(i == data->size)
        {
            printf_debug("|NEXT %ld BYTES|",check_size);
        }
    }
    printf_debug("\n");
    
}

//Searches for nodes that are invalid. 
void find_invalid_nodes(const void *ptr_find)
{
    //!WARNING
    // This code operates under the assumption that all of the node region was zero'd
    block_t *current_node = (block_t *)nodes;
    size_t invalid_nodes = 0;
    size_t allocated_nodes = 0;
    size_t medium_allocated_space = 0;
    size_t total_space = 0;
    size_t i = 0;
    #ifdef DEBUG
    void *last_region = 0;
    #endif
    bool exit_loop = false;

    while (current_node != NULL && !exit_loop)
    {
        // Bounds check
        if ((void *)current_node < (void *)nodes || (void *)current_node >= (void *)((char *)nodes + current_nodes_heap_size))
        {
            printf_debug("Node %ld exceeds legal bounds\n", i);
            printf_debug("\tNode: %p\n", current_node);
            printf_debug("\tStart: %p\n", nodes);
            printf_debug("\tEnd: %p\n", (char *)nodes + current_nodes_heap_size);

            // Attempting to repair the last valid node
            block_t *last_valid_node = (block_t *)nodes + (i - 1);
            if(last_valid_node == NULL)
            {
                printf_debug("Last valid node is NULL\n");
            }
            else if (last_valid_node->next != NULL)
            {
                printf_debug("Attempting to repair nodes\n");
                last_valid_node->next = NULL;
                last_valid_node->is_end = true;
            }
            exit_loop = true;
        }
        // Check if node is corrupted
        else if (current_node->magic != MAGIC_NUMBER)
        {
            printf_debug("Node %ld is corrupted\n", i);
            invalid_nodes++;

            // Reset to the next node (continue the loop)
            current_node = (block_t *)nodes + i;
            if (current_node == NULL) // Use NULL instead of 0 for pointer checks
            {
                printf_debug("2. Node %ld is corrupted\n", i);
                exit_loop = true;
            }
        }
        else
        {
            printf_debug("Region = (%ld)%p: %ld bytes from (%ld)%p\n",i,current_node->region,((char *)current_node->region-(char *)last_region),i-1,last_region);
            if(current_node->region != NULL && ptr_find != NULL && ptr_find == current_node->region)
            {
                printf_debug("Found region\n");
                display_medalloc_region(current_node,5);
                printf_debug("\n");
                
            }
            #ifdef DEBUG
            
            last_region = current_node->region;
            #endif
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

}


size_t get_medalloc_size(const void *ptr)
{
     block_t *current_node = (block_t *)nodes;
     while(current_node != NULL)
     {
        if(current_node->region == ptr || current_node->magic == MAGIC_NUMBER)
        {
            return current_node->size;
        }
        current_node = current_node->next;
     }
     return 0;

}