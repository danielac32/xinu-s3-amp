/**
 * @file calloc.c
 *
 * Allocate zeroed memory from the single kernel heap.
 */
/* Embedded Xinu, ESP32-S3 AMP. */

#include <stdlib.h>
#include <string.h>

void *calloc(size_t nmemb, size_t size)
{
    void *ptr;
    size_t total;

    if (0 == nmemb || 0 == size)
    {
        return NULL;
    }

    total = nmemb * size;
    if (total / nmemb != size)          /* multiplication overflow */
    {
        return NULL;
    }

    ptr = malloc(total);
    if (ptr != NULL)
    {
        memset(ptr, 0, total);
    }
    return ptr;
}
