#include "stdint.h"
#include "stddef.h"
#include "stdbool.h"
#include "smallalloc.h"
#include <stdio.h>
#include "string.h"
#include "alloc.h"
int main(void)
{
    char *test_alloc_small = cmalloc(300);
    if(test_alloc_small == NULL)
    {
        printf("Failed to allocate %d bytes of memory for test_alloc_small\n",300);
        return -1;
    }
    char *test_string = "Testing allocation\n";
    strcpy(test_alloc_small,test_string);
    if(strcmp(test_string,test_alloc_small)!=0)
    {
        printf("Strings do not match\n");
        printf("test_string: %s\n",test_string);
        printf("test_alloc_small: %s\n",test_alloc_small);
        return -1;
    }
    else
    {
        printf("Allocation for test_alloc_small worked\n");
    }
    char *test_alloc_small_2 = cmalloc(1025);
    if(test_alloc_small_2 == NULL)
    {
        printf("Failed to allocate %d bytes of memory for test_alloc_small_2\n",400);
        return -1;
    }
    char *test_string_2 = "Diffrent data\n";
    strcpy(test_alloc_small_2,test_string_2);
    if(strcmp(test_string_2,test_alloc_small_2)!=0)
    {
        printf("Strings do not match\n");
        printf("test_string: %s\n",test_string_2);
        printf("test_alloc_small: %s\n",test_alloc_small_2);
        return -1;
    }
    else
    {
        printf("Allocation for test_alloc_small_2 worked\n");
    }
    printf("test_alloc_small  : %p\n",test_alloc_small);
    printf("test_alloc_small_2: %p\n",test_alloc_small_2);
    printf("%d\n",cfree(test_alloc_small));
     printf("%d\n",cfree(test_alloc_small_2));
    return 0;
}