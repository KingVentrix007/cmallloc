#include "stdint.h"
#include "stddef.h"
#include "stdbool.h"
#include "smallalloc.h"
#include <stdio.h>
int main(void)
{
    char *ptr = small_malloc(300);
    if(ptr == NULL)
    {
        printf("It worked\n");
    }
    return 0;
}