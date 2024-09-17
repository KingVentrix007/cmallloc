#include "init.h"
#include "stdbool.h"
#include "stdint.h"
#include "stddef.h"
#include "alloc.h"
#include "smallalloc.h"
#include <sys/mman.h>
#include "def.h"
#include <stdio.h>
#include "medalloc.h"
#include <unistd.h>
#include <string.h>
bool memory_small_heap_initiated = false; //Flag to determine if the memory heap is setup
bool memory_medium_heap_initiated = false; //Flag to determine if the memory heap is setup
//Initialize the memory heap
int init_small_memory_heap() {

    if(memory_small_heap_initiated == true)
    {
        return -1;
    }
    if(memory_small_heap_initiated == true && small_heap == NULL)
    {
        return -1;
    }
    if(memory_small_heap_initiated == false && small_heap == NULL)
    {
        // printf("Here now\n");
        small_heap = (uint8_t *)mmap(NULL, SMALL_HEAP_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0); // Allocate memory of size for the heap using mmap
        if(small_heap == MAP_FAILED)
        {
            return -1;
        }
        bitmap = (uint8_t *)mmap(NULL,SMALL_HEAP_BITMAP_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if(bitmap == MAP_FAILED)
        {
            return -2;
        }
        current_small_heap_size = SMALL_HEAP_SIZE;
        setup_bitmap();
        
        

    }
    return 0;
}

//Initialize the bitmap
int setup_bitmap()
{
    if(bitmap == NULL)
    {
        printf("Bitmap is null\n");
        return -1;
    }
    for(int i = 0; i < SMALL_HEAP_BITMAP_SIZE; i++)
    {
        bitmap[i] = 0;
    }
    return 0;
}

int init_medium_memory_heap()
{
    // printf("Initializing medium heap\n");
    if(memory_medium_heap_initiated == true)
    {
        return -1;
    }
    if(memory_medium_heap_initiated == true && medium_heap == NULL)
    {
        return -1;
    }
    if(memory_medium_heap_initiated == false && medium_heap == NULL)
    {
        medium_heap = (uint8_t *)mmap(NULL, MEDIUM_HEAP_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0); // Allocate memory of size for the heap using mmap
        if(medium_heap == MAP_FAILED)
        {
            return -1;
        }
        nodes = (uint8_t *)mmap(NULL,MEDIUM_HEAP_NODES_SIZE*100, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        
        if(nodes == MAP_FAILED)
        {
            return -2;
        }
        current_nodes_heap_size = MEDIUM_HEAP_NODES_SIZE*100;
        current_medium_heap_size = MEDIUM_HEAP_SIZE;
        setup_nodes();
    }
    return 0;
}

int setup_nodes()
{
    if(nodes == NULL)
    {
        printf("Nodes is null\n");
        return -1;
    }
    block_t *current_node = (block_t *)nodes;
    bool head = true;
    for(size_t i = 0; i < MEDIUM_HEAP_NODES_SIZE; i++)
    {
        // printf("Loop %ld\n",i);
        if(current_node == NULL)
        {
            printf("Current node is NULL\n");
            return -1;
        }
        current_node->free = true;
        current_node->size = 0;
        if(head == true)
        {
            current_node->prev = NULL;
            head = false;
        }
        else
        {
            current_node->prev = (block_t *)(nodes + (i-1)*sizeof(block_t));
        }
        current_node->magic = MAGIC_NUMBER;
        block_t *check = (block_t *)(nodes + (i)*sizeof(block_t));
        if(check == NULL)
        {
            printf("Check is NULL\n");
            return -1;
        }
        if(check->magic != MAGIC_NUMBER)
        {
            printf("Check magic is not MAGIC_NUMBER\n");
            return -1;
        }
        // printf("")
        current_node->size = 1024;
        current_node->region = (void *)(medium_heap + (i+1)*1024);
        if (i == MEDIUM_HEAP_NODES_SIZE - 1) {
            // This is the last node
            current_node->next = NULL;
        } else {
            block_t *next_node = (block_t *)(nodes + (i+1)*sizeof(block_t));
            current_node->next = next_node;
            current_node = next_node;
        }
        // block_t *next_node = (block_t *)(nodes + (i+1)*sizeof(block_t));
        // current_node->next = next_node;
        // current_node = next_node;


    }
    return 0;
}


int expand_medium_region()
{
   
    int heap_ret = expand_medium_heap();
    if(heap_ret == -1)
    {
        return -1;
    }
    int nodes_ret = expand_medium_nodes();
    if(nodes_ret == -1)
    {
        return -1;
    }
    return 0;



}    
int expand_medium_heap()
{
    // Setup new heap
    if(memory_medium_heap_initiated == false)
    {
        return -1;
    }
    void *tmp_region = mmap(NULL, MEDIUM_HEAP_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if(tmp_region == MAP_FAILED)
    {
        return -1;
    }
    void *medium_heap_save_adder = medium_heap;
    memcpy(tmp_region, medium_heap, MEDIUM_HEAP_SIZE);
    munmap(medium_heap, MEDIUM_HEAP_SIZE);
    // void *new_addr = mmap(old_addr, new_size, , -1, 0);
    //Flags to use PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED
    medium_heap = mmap(medium_heap_save_adder, current_medium_heap_size*2, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED, -1, 0);
    if(medium_heap == MAP_FAILED)
    {
        return -1;
    }
    memcpy(medium_heap, tmp_region, MEDIUM_HEAP_SIZE);
    munmap(tmp_region, MEDIUM_HEAP_SIZE);
    current_medium_heap_size = current_medium_heap_size*2;
    return 0;

}
int expand_medium_nodes()
{
    // Setup new heap
    size_t region_size = calculate_list_memory(current_medium_heap_size, 1024, sizeof(block_t));
    void *tmp_region = mmap(NULL, region_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if(tmp_region == MAP_FAILED)
    {
        return -1;
    }
    void *node_ptr_save_adder = nodes;
    memcpy(tmp_region, nodes, region_size);
    munmap(nodes, region_size);
    nodes = mmap(node_ptr_save_adder, region_size*2, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED, -1, 0);
    if(nodes == MAP_FAILED)
    {
        return -1;
    }
    memcpy(nodes, tmp_region, region_size);
    munmap(tmp_region, region_size);
    current_nodes_heap_size = region_size;
    setup_expand_medium_nodes();
    return 0;

}
// #include <stdio.h> // Include for debug printing


int setup_expand_medium_nodes()
{
    block_t *current_node = (block_t *)nodes;
    void *end = ((char *)nodes)+current_nodes_heap_size;
    // size_t total_num_nodes = current_nodes_heap_size/sizeof(block_t);
    // printf("Total number of nodes to be processed: %ld\n", total_num_nodes);
    // printf("Size of node region: %ld\n", current_nodes_heap_size);
    // if((void *)current_node >= end)
    // {
    //     printf("Current node is NULL\n");
    //     return -1;
    // }
    size_t counts = 0; // Initial count
    bool found_end_of_region = false;
    bool added_first_new_node = false;
    while (current_node != NULL && (void *)current_node < end)
    {
        // printf("Current node(%ld): %p\n", counts,current_node);
        if (current_node->next == NULL && found_end_of_region == false)
        {
            found_end_of_region = true;
        }
        if (found_end_of_region == true)
        {
            block_t *block = (block_t *)(nodes + (counts)*sizeof(block_t));
            if ((void *)block >= end)
            {
                printf("We might have an issue\n");
                break;
            }
            block->free = true;
            block->size = 1024;
            block->magic = MAGIC_NUMBER;
            block->region = (void *)(medium_heap + (counts + 1)*MEDIUM_BLOCK_SIZE);
            block_t *next_block = (block_t *)(nodes + (counts + 1)*sizeof(block_t));
            block->next = next_block;
            block->prev = (block_t *)(nodes + (counts - 1)*sizeof(block_t));
            
            if (added_first_new_node == false)
            {
                current_node->next = block;
                added_first_new_node = true;
            }
        }

        current_node = current_node->next;
        counts++;
    }

    // printf("Counts: %ld\n", counts);
    return 0;
}
size_t calculate_list_memory(size_t heap_size, size_t block_size, size_t node_size) {
    // Calculate the number of blocks
    size_t num_blocks = heap_size / block_size;

    // Calculate the memory required for the linked list
    size_t list_memory_size = num_blocks * node_size;

    return list_memory_size;
}

