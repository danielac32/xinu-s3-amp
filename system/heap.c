/**
 * @file heap.c
 *
 * The one and only kernel heap.  It lives in the PSRAM window mapped by the
 * ESP-IDF loader.  memget(), memfree(), stkget(), malloc(), free(), calloc()
 * and realloc() all use the same free list (memlist).
 *
 * Limits:
 *   heap_base = first 8-byte-aligned byte of _end
 *   heap_end  = end of the loader-provided PSRAM window, rounded down
 *
 * _end is after the kernel image and the reserved panic/null stacks, so the
 * heap can never overlap either of them.
 */
/* Embedded Xinu, ESP32-S3 AMP. */

#include <stddef.h>
#include <stdint.h>
#include <kernel.h>
#include <interrupt.h>
#include <memory.h>
#include <psram.h>

/** PSRAM window published by platforminit(). */
uint32_t psram_base;
uint32_t psram_size;
uint32_t psram_exec_base;
uint32_t psram_exec_size;
uint32_t psram_heap_start;
uint32_t psram_heap_size;

/** The single kernel free list. */
struct memblock memlist;

/** First and last byte managed by this heap. */
static uintptr_t heap_base;
static uintptr_t heap_end;

/** Diagnostic counters. */
static uint32_t heap_alloc_count;
static uint32_t heap_alloc_bytes;
static uint32_t heap_largest_alloc;

void heapInit(uintptr_t base, uintptr_t size)
{
    struct memblock *pm;
    uintptr_t start;
    uintptr_t end;

    start = (base + MEMORY_DMBDEC) & ~MEMORY_DMBDEC;
    end = (base + size) & ~MEMORY_DMBDEC;

    if (end <= start)
    {
        heap_base = 0;
        heap_end = 0;
        memlist.next = NULL;
        memlist.length = 0;
        return;
    }

    heap_base = start;
    heap_end = end;

    /* Publish the effective boundaries for the shell and for boot reports. */
    psram_heap_start = (uint32_t)heap_base;
    psram_heap_size = (uint32_t)(heap_end - heap_base);

    memlist.next = pm = (struct memblock *)heap_base;
    memlist.length = heap_end - heap_base;
    pm->next = NULL;
    pm->length = heap_end - heap_base;
}

void *heap_memget(uintptr_t nbytes)
{
    register struct memblock *prev, *curr, *leftover;
    irqmask im;

    if (0 == nbytes || heap_base == 0)
    {
        return (void *)SYSERR;
    }

    nbytes = (uintptr_t)roundmb(nbytes);

    im = disable();

    prev = &memlist;
    curr = memlist.next;
    while (curr != NULL)
    {
        if (curr->length == nbytes)
        {
            prev->next = curr->next;
            memlist.length -= nbytes;
            heap_alloc_count++;
            heap_alloc_bytes += nbytes;
            if (nbytes > heap_largest_alloc)
            {
                heap_largest_alloc = nbytes;
            }
            restore(im);
            return (void *)curr;
        }
        else if (curr->length > nbytes)
        {
            leftover = (struct memblock *)((ulong)curr + nbytes);
            leftover->next = curr->next;
            leftover->length = curr->length - nbytes;
            prev->next = leftover;
            memlist.length -= nbytes;
            heap_alloc_count++;
            heap_alloc_bytes += nbytes;
            if (nbytes > heap_largest_alloc)
            {
                heap_largest_alloc = nbytes;
            }
            restore(im);
            return (void *)curr;
        }
        prev = curr;
        curr = curr->next;
    }

    restore(im);
    return (void *)SYSERR;
}

syscall heap_memfree(void *memptr, uintptr_t nbytes)
{
    register struct memblock *block, *next, *prev;
    irqmask im;
    ulong top;

    if (heap_base == 0 || 0 == nbytes
        || (ulong)memptr < heap_base || (ulong)memptr >= heap_end)
    {
        return SYSERR;
    }

    block = (struct memblock *)memptr;
    nbytes = (ulong)roundmb(nbytes);

    im = disable();

    prev = &memlist;
    next = memlist.next;
    while ((next != NULL) && (next < block))
    {
        prev = next;
        next = next->next;
    }

    if (prev == &memlist)
    {
        top = NULL;
    }
    else
    {
        top = (ulong)prev + prev->length;
    }

    if ((top > (ulong)block)
        || ((next != NULL) && ((ulong)block + nbytes) > (ulong)next))
    {
        restore(im);
        return SYSERR;
    }

    memlist.length += nbytes;

    if (top == (ulong)block)
    {
        prev->length += nbytes;
        block = prev;
    }
    else
    {
        block->next = next;
        block->length = nbytes;
        prev->next = block;
    }

    if (((ulong)block + block->length) == (ulong)next)
    {
        block->length += next->length;
        block->next = next->next;
    }

    restore(im);
    return OK;
}

void *heap_stkget(uintptr_t nbytes)
{
    irqmask im;
    struct memblock *prev, *next, *fits, *fitsprev;

    if (0 == nbytes || heap_base == 0)
    {
        return (void *)SYSERR;
    }

    nbytes = (uintptr_t)roundmb(nbytes);

    im = disable();

    prev = &memlist;
    next = memlist.next;
    fits = NULL;
    fitsprev = NULL;

    /* scan list for highest block that fits */
    while (next != NULL)
    {
        if (next->length >= nbytes)
        {
            fits = next;
            fitsprev = prev;
        }
        prev = next;
        next = next->next;
    }

    if (NULL == fits)
    {
        restore(im);
        return (void *)SYSERR;
    }

    if (nbytes == fits->length)
    {
        fitsprev->next = fits->next;
    }
    else
    {
        /* take top portion */
        fits->length -= nbytes;
        fits = (struct memblock *)((ulong)fits + fits->length);
    }

    memlist.length -= nbytes;
    restore(im);
    return (void *)((ulong)fits + nbytes - sizeof(void *));
}

uint32_t heapFreeBytes(void)
{
    return (uint32_t)memlist.length;
}

uint32_t heapUsedBytes(void)
{
    return (uint32_t)(heap_end - heap_base) - (uint32_t)memlist.length;
}

uint32_t heapLargestFree(void)
{
    struct memblock *curr = memlist.next;
    uint32_t best = 0;

    while (curr != NULL)
    {
        if (curr->length > best)
        {
            best = (uint32_t)curr->length;
        }
        curr = curr->next;
    }
    return best;
}

uint32_t heapAllocCount(void) { return heap_alloc_count; }
uint32_t heapAllocBytes(void) { return heap_alloc_bytes; }
uint32_t heapLargestAlloc(void) { return heap_largest_alloc; }
