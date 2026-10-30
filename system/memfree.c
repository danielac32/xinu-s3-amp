/**
 * @file memfree.c
 *
 * Free a block from the single PSRAM heap.
 */
/* Embedded Xinu, ESP32-S3 AMP. */

#include <kernel.h>
#include <memory.h>

syscall memfree(void *memptr, uintptr_t nbytes)
{
    return heap_memfree(memptr, nbytes);
}
