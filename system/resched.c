/**
 * @file resched.c
 *
 */
/* Embedded Xinu, Copyright (C) 2009.  All rights reserved. */

#include <thread.h>
#include <clock.h>
#include <queue.h>
#include <memory.h>
#include <stack.h>

extern void ctxsw(void *, void *, uchar);
int resdefer;                   /* >0 if rescheduling deferred */

/**
 * @ingroup threads
 *
 * Reschedule processor to highest priority ready thread.
 * Upon entry, thrcurrent gives current thread id.
 * Threadtab[thrcurrent].pstate gives correct NEXT state
 * for current thread if other than THRREADY.
 * @return OK when the thread is context switched back
 */
int resched(void)
{
    uchar asid;                 /* address space identifier */
    struct thrent *throld;      /* old thread entry */
    struct thrent *thrnew;      /* new thread entry */
    volatile uint32_t stkmark;  /* current SP on the outgoing stack */

    /* High-water mark: resched runs on the stack of the thread that is
     * leaving (there is no separate IRQ stack), so &stkmark is a reliable
     * sample of the current usage. */
    stack_note(thrcurrent, (uint32_t)(uintptr_t)&stkmark);

    if (resdefer > 0)
    {                           /* if deferred, increase count & return */
        resdefer++;
        return (OK);
    }

    throld = &thrtab[thrcurrent];

    throld->intmask = disable();

    if (THRCURR == throld->state)
    {
        if (nonempty(readylist) && (throld->prio > firstkey(readylist)))
        {
            restore(throld->intmask);
            return OK;
        }
        throld->state = THRREADY;
        insert(thrcurrent, readylist, throld->prio);
    }

    /* get highest priority thread from ready list */
    thrcurrent = dequeue(readylist);
    thrnew = &thrtab[thrcurrent];
    thrnew->state = THRCURR;

    /* change address space identifier to thread id */
    asid = thrcurrent & 0xff;
    ctxsw(&throld->stkptr, &thrnew->stkptr, asid);

    /* old thread returns here when resumed */
    restore(throld->intmask);
    return OK;
}
