/**
 * @file memget.c
 *
 * Allocate heap memory from the single PSRAM heap.
 */
/* Embedded Xinu, ESP32-S3 AMP. */

#include <kernel.h>
#include <memory.h>

void *memget(uintptr_t nbytes)
{
    return heap_memget(nbytes);
}
