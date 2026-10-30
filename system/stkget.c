/**
 * @file stkget.c
 *
 * Allocate a downward-growing stack from the single PSRAM heap.
 */
/* Embedded Xinu, ESP32-S3 AMP. */

#include <kernel.h>
#include <memory.h>

void *stkget(uintptr_t nbytes)
{
    return heap_stkget(nbytes);
}
