/**
 * @file memory.h
 * Definitions for the kernel memory allocator and maintenance.
 *
 */
/* Embedded Xinu, Copyright (C) 2009.  All rights reserved. */

#ifndef _MEMORY_H_
#define _MEMORY_H_

#include <stddef.h>
#include <stdint.h>

#define MEMORY_DMBDEC (2*sizeof(uintptr_t)-1)
/* roundmb - round address up to size of memblock  */
#define roundmb(x)  (void *)( (MEMORY_DMBDEC + (ulong)(x)) & ~MEMORY_DMBDEC )
/* truncmb - truncate address down to size of memblock */
#define truncmb(x)  (void *)( ((ulong)(x)) & ~MEMORY_DMBDEC )

/**
 * @ingroup memory_mgmt
 *
 * Frees memory allocated with stkget().
 *
 * @param p
 *      Pointer to the topmost (highest address) word of the allocated stack (as
 *      returned by stkget()).
 * @param len
 *      Size of the allocated stack, in bytes.  (Same value passed to stkget().)
 */
#define stkfree(p, len) memfree((void *)((ulong)(p)         \
                                - (ulong)roundmb(len)       \
                                + (ulong)sizeof(ulong)),    \
                                (ulong)roundmb(len))


/**
 * Structure for a block of memory.
 */
struct memblock
{
    struct memblock *next;          /**< pointer to next memory block       */
    uintptr_t length;               /**< size of memory block (with struct) */
};

/** The one and only kernel free list.  It lives in PSRAM. */
extern struct memblock memlist;

/* Linker-provided boundaries.  The kernel image, .stacks and the heap are
 * laid out in this order in PSRAM:
 *
 *   .rodata/.data/.bss -> .stacks -> heap
 */
extern void *_end;              /**< end of the kernel image and boot stacks */
extern void *_etext;            /**< end of text segment                     */
extern void *_stacks_start;     /**< bottom of the reserved boot stacks       */
extern void *_stacks_end;       /**< top of the reserved boot stacks          */
extern void *_panicstk_top;     /**< top of the panic stack                   */
extern void *_nullstk_top;      /**< top of the null thread stack             */

/** Initialize the single PSRAM heap.  Called once from sysinit(). */
void heapInit(uintptr_t base, uintptr_t size);

/* Low-level kernel heap API.  All three entry points use the same free list
 * (memlist).  There is no separate internal-DRAM allocator. */
void *memget(uintptr_t);
syscall memfree(void *, uintptr_t);
void *stkget(uintptr_t);

/* Internal entry points used by memget()/memfree()/stkget(). */
void *heap_memget(uintptr_t);
syscall heap_memfree(void *, uintptr_t);
void *heap_stkget(uintptr_t);

/* The standard C entry points (malloc/free/calloc/realloc) are implemented
 * in libxc on top of memget()/memfree(); they use the same free list. */

/* Heap statistics for the shell. */
uint32_t heapFreeBytes(void);
uint32_t heapUsedBytes(void);
uint32_t heapLargestFree(void);
uint32_t heapAllocCount(void);
uint32_t heapAllocBytes(void);
uint32_t heapLargestAlloc(void);

#endif                          /* _MEMORY_H_ */
