/**
 * @file realloc.c
 *
 * Resize a block allocated with malloc().  The kernel heap has no in-place
 * growth, so this allocates a new block, copies the old contents and frees
 * the old block.
 */
/* Embedded Xinu, ESP32-S3 AMP. */

#include <memory.h>
#include <stdlib.h>
#include <string.h>

void *realloc(void *ptr, size_t size)
{
    struct memblock *block;
    void *fresh;
    size_t old;

    if (NULL == ptr)
    {
        return malloc(size);
    }
    if (0 == size)
    {
        free(ptr);
        return NULL;
    }

    block = ((struct memblock *)ptr) - 1;
    if (block->next != block)           /* not a block malloc() handed out */
    {
        return NULL;
    }
    old = block->length - sizeof(struct memblock);

    fresh = malloc(size);
    if (NULL == fresh)
    {
        return NULL;
    }
    memcpy(fresh, ptr, (old < size) ? old : size);
    free(ptr);
    return fresh;
}
