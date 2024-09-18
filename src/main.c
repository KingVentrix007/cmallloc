#include "stdint.h"
#include "stddef.h"
#include "stdbool.h"
#include "smallalloc.h"
#include <stdio.h>
#include "string.h"
#include "alloc.h"
#include "init.h"
#include "medalloc.h"
#define ALLOC_BLOCK_SIZE 1025
#define TEST_ALLOC_LARGE 10000
unsigned char generate_value(size_t i);
unsigned char generate_value(size_t i) {
    return (unsigned char)(i % 256);  // Ensure value fits in one byte
}
int main(void) {
    void *allocs[TEST_ALLOC_LARGE];

    // Allocate memory and set unique values
    for (size_t i = 0; i < TEST_ALLOC_LARGE; i++) {
        allocs[i] = cmalloc(ALLOC_BLOCK_SIZE);
        if (allocs[i] == NULL) {
            printf("Failed to allocate after %zu allocations\n", i);
            return -1;
        }

        // Fill the allocated region with a unique value based on the loop index
        unsigned char value = generate_value(i);
        memset(allocs[i], value, ALLOC_BLOCK_SIZE);

        // Check all previous allocations to ensure they have the correct values
        for (size_t j = 0; j <= i; j++) {
            unsigned char expected_value = generate_value(j);
            unsigned char *block = (unsigned char *)allocs[j];

            // Verify the data block
            for (size_t k = 0; k < ALLOC_BLOCK_SIZE; k++) {
                if (block[k] != expected_value) {
                    printf("Data mismatch at allocation %zu, byte %zu. Expected: %u, Got: %u\n", j, k, expected_value, block[k]);
                    return -1;
                }
            }
        }

        // Optionally, print the progress after every 1000 iterations or so
        if (i % 1000 == 0) {
            printf("Allocated and verified %zu blocks\n", i + 1);
        }
    }

    printf("All allocations and verifications successful.\n");

    // Free allocated memory
    // for (size_t i = 0; i < TEST_ALLOC_LARGE; i++) {
    //     free(allocs[i]);
    // }

    return 0;
}