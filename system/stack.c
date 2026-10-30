/**
 * @file stack.c
 *
 * Per-thread stack high-water mark.  On every context switch resched() records
 * the SP of the thread that is leaving; the maximum of (top - sp) is an upper
 * bound on the real usage (it includes the resched/IRQ frame, which also uses
 * the stack).
 *
 * The null thread is special: it does not pass through create(), and
 * initialize.c sets stkbase to _panicstk_top (the BOTTOM of its stack, not the
 * top) and stklen to the size of [_panicstk_top, _nullstk_top).  Threads
 * created by create() get stkbase = the topmost word returned by stkget().
 */
/* Embedded Xinu, Copyright (C) 2026.  All rights reserved. */

#include <stack.h>
#include <memory.h>

static uint32_t stack_low(tid_typ tid)
{
    if (tid == NULLTHREAD)
    {
        return (uint32_t)(uintptr_t)thrtab[tid].stkbase;
    }

    return (uint32_t)(uintptr_t)thrtab[tid].stkbase
         - (uint32_t)(uintptr_t)roundmb(thrtab[tid].stklen)
         + (uint32_t)sizeof(ulong);
}

static uint32_t stack_top(tid_typ tid)
{
    if (tid == NULLTHREAD)
    {
        return (uint32_t)(uintptr_t)thrtab[tid].stkbase
             + (uint32_t)thrtab[tid].stklen;
    }

    return (uint32_t)(uintptr_t)thrtab[tid].stkbase
         + (uint32_t)sizeof(ulong);
}

uint32_t stack_total(tid_typ tid)
{
    if (tid < 0 || tid >= NTHREAD || thrtab[tid].state == THRFREE)
    {
        return 0;
    }

    return stack_top(tid) - stack_low(tid);
}

void stack_note(tid_typ tid, uint32_t sp)
{
    struct thrent *t;
    uint32_t top;
    uint32_t low;
    uint32_t used;

    if (tid < 0 || tid >= NTHREAD)
    {
        return;
    }

    t = &thrtab[tid];
    if (t->state == THRFREE)
    {
        return;
    }

    top = stack_top(tid);
    low = stack_low(tid);

    if (sp <= top && sp >= low)
    {
        used = top - sp;
        if (used > t->stk_peak)
        {
            t->stk_peak = used;
        }
    }
    else
    {
        /* The SP is not inside this thread's stack: either the stack is
         * corrupted or the sample came from another stack.  Mark it and
         * saturate the peak. */
        t->stk_overflow = TRUE;
        used = stack_total(tid);
        if (used > t->stk_peak)
        {
            t->stk_peak = used;
        }
    }
}

uint32_t stack_peak(tid_typ tid)
{
    if (tid < 0 || tid >= NTHREAD)
    {
        return 0;
    }

    return thrtab[tid].stk_peak;
}

uint32_t stack_free(tid_typ tid)
{
    uint32_t total = stack_total(tid);
    uint32_t used = stack_peak(tid);

    return (used < total) ? (total - used) : 0;
}

int stack_overflow(tid_typ tid)
{
    if (tid < 0 || tid >= NTHREAD)
    {
        return FALSE;
    }

    return thrtab[tid].stk_overflow ? TRUE : FALSE;
}

void stack_reset(tid_typ tid)
{
    if (tid < 0 || tid >= NTHREAD)
    {
        return;
    }

    thrtab[tid].stk_peak = 0;
    thrtab[tid].stk_overflow = FALSE;
}

void stack_reset_all(void)
{
    tid_typ tid;

    for (tid = 0; tid < NTHREAD; tid++)
    {
        stack_reset(tid);
    }
}
