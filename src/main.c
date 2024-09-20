#include "stdint.h"
#include "stddef.h"
#include "stdbool.h"
#include "smallalloc.h"
#include <stdio.h>
#include "string.h"
#include "alloc.h"
#include "init.h"
#include "medalloc.h"
#include <stdlib.h>
#define ALLOC_BLOCK_SIZE 1025
#define TEST_ALLOC_LARGE 10000
unsigned char generate_value(size_t i);
unsigned char generate_value(size_t i) {
    return (unsigned char)(i % 256);  // Ensure value fits in one byte
}
int main(void) {
    
// realloc()
// calloc()
    return 0;
}