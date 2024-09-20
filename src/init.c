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

// Error codes
#define ERR_ALREADY_INITIATED -1
#define ERR_MMAP_FAILED -2
#define ERR_BITMAP_SETUP_FAILED -3
#define ERR_NODES_SETUP_FAILED -4
#define ERR_EXPAND_HEAP_FAILED -5
#define ERR_EXPAND_NODES_FAILED -6

// Flags to determine if the memory heaps are set up
bool memory_small_heap_initiated = false;
bool memory_medium_heap_initiated = false;

// Initialize the small memory heap
int init_small_memory_heap() {
    if (memory_small_heap_initiated) {
        return ERR_ALREADY_INITIATED;
    }

    if (small_heap != NULL) {
        return ERR_ALREADY_INITIATED;
    }

    // Allocate memory for the small heap using mmap
    small_heap = (uint8_t *)mmap(NULL, SMALL_HEAP_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (small_heap == MAP_FAILED) {
        return ERR_MMAP_FAILED;
    }

    // Allocate memory for the bitmap using mmap
    bitmap = (uint8_t *)mmap(NULL, SMALL_HEAP_BITMAP_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (bitmap == MAP_FAILED) {
        return ERR_MMAP_FAILED;
    }

    current_small_heap_size = SMALL_HEAP_SIZE;

    // Initialize the bitmap
    if (setup_bitmap() != 0) {
        return ERR_BITMAP_SETUP_FAILED;
    }

    memory_small_heap_initiated = true;
    return 0;
}

// Initialize the bitmap
int setup_bitmap() {
    if (bitmap == NULL) {
        printf("Error: Bitmap is null\n");
        return ERR_BITMAP_SETUP_FAILED;
    }

    // Set all bits in the bitmap to 0
    for (int i = 0; i < SMALL_HEAP_BITMAP_SIZE; i++) {
        bitmap[i] = 0;
    }

    return 0;
}

// Initialize the medium memory heap
int init_medium_memory_heap() {
    if (memory_medium_heap_initiated) {
        return ERR_ALREADY_INITIATED;
    }

    if (medium_heap != NULL) {
        return ERR_ALREADY_INITIATED;
    }

    // Allocate memory for the medium heap using mmap
    medium_heap = (uint8_t *)mmap(NULL, MEDIUM_HEAP_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (medium_heap == MAP_FAILED) {
        return ERR_MMAP_FAILED;
    }

    // Allocate memory for the nodes using mmap
    nodes = (uint8_t *)mmap(NULL, MEDIUM_HEAP_NODES_SIZE * 100, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (nodes == MAP_FAILED) {
        return ERR_MMAP_FAILED;
    }

    current_nodes_heap_size = MEDIUM_HEAP_NODES_SIZE * 100;
    current_medium_heap_size = MEDIUM_HEAP_SIZE;

    // Initialize the nodes
    if (setup_nodes() != 0) {
        return ERR_NODES_SETUP_FAILED;
    }

    memory_medium_heap_initiated = true;
    return 0;
}

// Initialize the nodes
int setup_nodes() {
    if (nodes == NULL) {
        printf("Error: Nodes is null\n");
        return ERR_NODES_SETUP_FAILED;
    }

    block_t *current_node = (block_t *)nodes;
    bool head = true;

    for (size_t i = 0; i < current_medium_heap_size / 1024; i++) {
        if (current_node == NULL) {
            printf("Error: Current node is NULL\n");
            return ERR_NODES_SETUP_FAILED;
        }

        current_node->free = true;
        current_node->size = 0;

        if (head) {
            current_node->prev = NULL;
            head = false;
        } else {
            current_node->prev = (block_t *)(nodes + (i - 1) * sizeof(block_t));
        }

        current_node->magic = MAGIC_NUMBER;

        const block_t *check = (block_t *)(nodes + i * sizeof(block_t));
        if (check == NULL) {
            printf("Error: Check is NULL\n");
            return ERR_NODES_SETUP_FAILED;
        }

        if (check->magic != MAGIC_NUMBER) {
            printf("Error: Check magic is not MAGIC_NUMBER\n");
            return ERR_NODES_SETUP_FAILED;
        }

        current_node->size = 1024;
        current_node->region = (void *)(medium_heap + (i + 1) * 1024);
        memset(current_node->region, 0, 1024);

        if (i == current_medium_heap_size / 1024 - 1) {
            // This is the last node
            current_node->next = NULL;
        } else {
            block_t *next_node = (block_t *)(nodes + (i + 1) * sizeof(block_t));
            current_node->next = next_node;
            current_node = next_node;
        }
    }

    return 0;
}

// Expand the medium region
int expand_medium_region() {
    int heap_ret = expand_medium_heap();
    if (heap_ret != 0) {
        return ERR_EXPAND_HEAP_FAILED;
    }

    int nodes_ret = expand_medium_nodes();
    if (nodes_ret != 0) {
        return ERR_EXPAND_NODES_FAILED;
    }

    return 0;
}

// Expand the medium heap
int expand_medium_heap() {
    if (!memory_medium_heap_initiated) {
        return ERR_EXPAND_HEAP_FAILED;
    }

    // Allocate temporary memory for the current heap
    void *tmp_region = mmap(NULL, MEDIUM_HEAP_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (tmp_region == MAP_FAILED) {
        return ERR_MMAP_FAILED;
    }

    void *medium_heap_save_adder = medium_heap;
    memcpy(tmp_region, medium_heap, MEDIUM_HEAP_SIZE);
    munmap(medium_heap, MEDIUM_HEAP_SIZE);

    // Allocate new memory for the expanded heap
    medium_heap = mmap(medium_heap_save_adder, current_medium_heap_size * 2, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED, -1, 0);
    if (medium_heap == MAP_FAILED) {
        return ERR_MMAP_FAILED;
    }

    memcpy(medium_heap, tmp_region, MEDIUM_HEAP_SIZE);
    munmap(tmp_region, MEDIUM_HEAP_SIZE);
    current_medium_heap_size *= 2;

    return 0;
}

// Expand the medium nodes
int expand_medium_nodes() {
    size_t region_size = calculate_list_memory(current_medium_heap_size, 1024, sizeof(block_t));

    // Allocate temporary memory for the current nodes
    void *tmp_region = mmap(NULL, region_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (tmp_region == MAP_FAILED) {
        return ERR_MMAP_FAILED;
    }

    void *node_ptr_save_adder = nodes;
    memcpy(tmp_region, nodes, region_size);
    munmap(nodes, region_size);

    // Allocate new memory for the expanded nodes
    nodes = mmap(node_ptr_save_adder, region_size * 3, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED, -1, 0);
    if (nodes == MAP_FAILED) {
        printf("Error: Failed to setup new node region of size %ld\n", region_size * 3);
        return ERR_MMAP_FAILED;
    }
    memset(nodes,0,region_size * 3);
    memcpy(nodes, tmp_region, region_size);
    munmap(tmp_region, region_size);
    current_nodes_heap_size = region_size * 3;
    // memset()
    // Initialize the expanded nodes
    if (setup_expand_medium_nodes() != 0) {

        return ERR_NODES_SETUP_FAILED;
    }

    return 0;
}

// Initialize the expanded medium nodes
int setup_expand_medium_nodes() {
    block_t *current_node = (block_t *)nodes;
    void *end = ((char *)nodes) + current_nodes_heap_size;
    size_t counts = 0;
    bool found_end_of_region = false;
    bool added_first_new_node = false;
    size_t created_size = 0;
    while (current_node != NULL && (void *)current_node < end) {
        if (current_node->next == NULL && !found_end_of_region) {
            found_end_of_region = true;
        

        }
        else
        {
            created_size += current_node->size;
        }
        if (found_end_of_region) {
            block_t *block = (block_t *)(nodes + counts * sizeof(block_t));
            if ((void *)block >= end) {
                printf("Error: Node exceeds the end of nodes region\n");
                break;
            }

            block->free = true;
            block->size = 1024;
            created_size+=1024;
            block->magic = MAGIC_NUMBER;
            block->region = (void *)(medium_heap + (counts + 1) * MEDIUM_BLOCK_SIZE);
            block_t *next_block = (block_t *)(nodes + (counts + 1) * sizeof(block_t));
            block->next = next_block;
            block->prev = (block_t *)(nodes + (counts - 1) * sizeof(block_t));

            if (!added_first_new_node) {
                current_node->next = block;
                added_first_new_node = true;
            }
            if(created_size >= current_medium_heap_size)
            {
                block->is_end = true;
                block->next = NULL;
                block->prev = (block_t *)(nodes + (counts - 1) * sizeof(block_t));
                block->is_end = true;
                return 0;
            }
        }

        current_node = current_node->next;
        counts++;
    }

    return 0;
}

// Calculate the memory required for the linked list
size_t calculate_list_memory(size_t heap_size, size_t block_size, size_t node_size) {
    size_t num_blocks = heap_size / block_size;
    size_t list_memory_size = num_blocks * node_size;
    return list_memory_size;
}
