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
    printf("Initializing medium heap\n");
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

