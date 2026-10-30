/**
 * @file psram.h
 *
 * PSRAM window information for the ESP32-S3 AMP payload.
 *
 * The ESP-IDF loader maps the die and reserves the window used by Xinu.
 * Xinu never initializes the PSRAM controller.  All allocations come from
 * the single kernel heap described in memory.h.
 */
/* Embedded Xinu, ESP32-S3 AMP. */

#ifndef _PSRAM_H_
#define _PSRAM_H_

#include <stddef.h>
#include <stdint.h>

/** Virtual address where the loader mapped the PSRAM window. */
extern uint32_t psram_base;

/** Usable size of that window, in bytes. */
extern uint32_t psram_size;

/** Instruction-cache alias of psram_base.  Zero when unavailable. */
extern uint32_t psram_exec_base;
extern uint32_t psram_exec_size;

/** First byte of the kernel heap and its size.  Set by platforminit(). */
extern uint32_t psram_heap_start;
extern uint32_t psram_heap_size;

/** Compatibility alias used by reports and drivers. */
#define PSRAM_BASE psram_base

/**
 * Translate a pointer inside the PSRAM data window to the matching address
 * in the instruction-cache alias.  The loader maps the same physical pages
 * at both virtual addresses and publishes the executable base; Xinu only
 * translates pointers.
 *
 * @param p pointer inside [psram_base, psram_base + psram_size)
 * @return executable alias, or NULL if @p p is outside the window
 */
static inline void *psram_exec_addr(const void *p)
{
    uintptr_t a = (uintptr_t)p;

    if (0 == psram_exec_base || 0 == psram_size
        || a < (uintptr_t)psram_base
        || a >= (uintptr_t)psram_base + (uintptr_t)psram_size)
    {
        return NULL;
    }
    return (void *)((uintptr_t)psram_exec_base
                    + (a - (uintptr_t)psram_base));
}

#endif                          /* _PSRAM_H_ */
