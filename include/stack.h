/**
 * @file stack.h
 *
 * Per-thread stack high-water mark measurement.
 */
/* Embedded Xinu, Copyright (C) 2026.  All rights reserved. */

#ifndef _STACK_H_
#define _STACK_H_

#include <stdint.h>
#include <thread.h>

/* Record the current SP of a running thread.  resched() calls this on every
 * context switch; the peak is the maximum of (top - sp). */
void stack_note(tid_typ tid, uint32_t sp);

/** Maximum observed usage, in bytes (0 if the thread never ran). */
uint32_t stack_peak(tid_typ tid);

/** Total stack size, in bytes. */
uint32_t stack_total(tid_typ tid);

/** Stack bytes that were never used (total - peak). */
uint32_t stack_free(tid_typ tid);

/** TRUE if the observed SP fell outside the thread's stack. */
int stack_overflow(tid_typ tid);

/** Reset the observed peak for one thread. */
void stack_reset(tid_typ tid);

/** Reset the peaks of all threads. */
void stack_reset_all(void);

#endif                          /* _STACK_H_ */
