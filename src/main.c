#include "stdint.h"
#include "stddef.h"
#include "stdbool.h"
#include "smallalloc.h"
#include <stdio.h>
#include "string.h"
int main(void)
{
    char *ptr = small_malloc(300);
    if(ptr == NULL)
    {
        printf("It worked\n");
    }
    printf("ptr = %p\n",ptr);
    //Test the allocation by putting data into memory
    char *test_string = "Hello World";
    strcpy(ptr,test_string);
    printf("Output %s\n",ptr);
    small_free(ptr);
    return 0;
}