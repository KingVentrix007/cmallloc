#include "init.h"
#include "stdbool.h"
#include "stdint.h"
#include "stddef.h"
#include "alloc.h"
#include "smallalloc.h"
#include <sys/mman.h>
#include "def.h"
#include <stdio.h>
bool memory_small_heap_initiated = false; //Flag to determine if the memory heap is setup

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
        printf("Here now\n");
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