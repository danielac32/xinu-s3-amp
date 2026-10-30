/**
 * @file initialize.c
 * The system begins initializing after the C environment has been
 * established.  After initialization, the null thread remains always in
 * a ready (THRREADY) or running (THRCURR) state.
 */
/* Embedded Xinu, Copyright (C) 2009, 2013.  All rights reserved. */

#include <kernel.h>
#include <clock.h>
#include <device.h>
#include <memory.h>
#include <bufpool.h>
#include <thread.h>
#include <queue.h>
#include <semaphore.h>
#include <monitor.h>
#include <mailbox.h>
#include <network.h>
#include <stddef.h>
#include <string.h>
#include <platform.h>
#include <psram.h>
#include <bootinfo.h>

/* Function prototypes */
extern thread main(void);       /* main is the first thread created    */
static int sysinit(void);       /* intializes system structures        */

/* Declarations of major kernel variables */
struct thrent thrtab[NTHREAD];  /* Thread table                   */
struct sement semtab[NSEM];     /* Semaphore table                */
struct monent montab[NMON];     /* Monitor table                  */
qid_typ readylist;              /* List of READY threads          */
struct bfpentry bfptab[NPOOL];  /* List of memory buffer pools    */

/* Active system status */
int thrcount;                   /* Number of live user threads         */
tid_typ thrcurrent;             /* Id of currently running thread      */

/* Set by start.S when Xinu is entered as an ESP-IDF AMP payload. */
struct xinu_bootinfo *xinu_bootinfo = NULL;

struct platform platform;       /* Platform specific configuration     */

/**
 * Initializes the system and becomes the null thread.
 * This is where the system begins after the C environment has been 
 * established.  Interrupts are initially DISABLED, and must eventually 
 * be enabled explicitly.  This routine turns itself into the null thread 
 * after initialization.  Because the null thread must always remain ready 
 * to run, it cannot execute code that might cause it to be suspended, wait 
 * for a semaphore, or put to sleep, or exit.  In particular, it must not 
 * do I/O unless it uses kprintf for synchronous output.
 */
void nulluser(void)
{
    /* Platform-specific initialization  */
    platforminit();

    /* General initialization  */
    sysinit();

    /* Enable interrupts  */
    enable();

    /* Spawn the main thread  */
    ready(create(main, INITSTK, INITPRIO, "MAIN", 0), RESCHED_YES);

    /* null thread has nothing else to do but cannot exit  */
    while (TRUE)
    {
        pause();
    }
}

/**
 * Initializes all Xinu data structures and devices.
 * @return OK if everything is initialized successfully
 */
static int sysinit(void)
{
    int i;
    struct thrent *thrptr;      /* thread control block pointer  */

    /* Initialize system variables */
    /* Count this NULLTHREAD as the first thread in the system. */
    thrcount = 1;

    /* There is exactly one heap, and it lives in PSRAM.  The kernel image
     * and the boot stacks end at _end; everything from there to the end of
     * the loader-provided PSRAM window is the single free list used by
     * memget()/memfree()/stkget() and by malloc()/free()/calloc()/realloc(). */
    heapInit((uintptr_t)&_end,
             (uintptr_t)psram_base + (uintptr_t)psram_size
             - (uintptr_t)&_end);

    /* Initialize thread table */
    for (i = 0; i < NTHREAD; i++)
    {
        thrtab[i].state = THRFREE;
    }

    /* initialize null thread entry */
    thrptr = &thrtab[NULLTHREAD];
    thrptr->state = THRCURR;
    thrptr->prio = 0;
    strlcpy(thrptr->name, "prnull", TNMLEN);
    /* The null thread runs on the upper half of the reserved .stacks
     * region; the lower half is the panic stack.  Both are outside the
     * heap, which starts at _end. */
    thrptr->stkbase = (void *)(uintptr_t)&_panicstk_top;
    thrptr->stklen = (ulong)((uintptr_t)&_nullstk_top
                             - (uintptr_t)&_panicstk_top);
    thrptr->stkptr = (void *)(uintptr_t)&_nullstk_top;
    thrptr->memlist.next = NULL;
    thrptr->memlist.length = 0;

    /* The null thread does not pass through create(). */
    thrptr->cpu_ticks = 0;
    thrptr->born_ticks = 0;
    thrptr->gen = 0;
    thrptr->stk_peak = 0;
    thrptr->stk_overflow = FALSE;

    thrcurrent = NULLTHREAD;

    /* Initialize semaphores */
    for (i = 0; i < NSEM; i++)
    {
        semtab[i].state = SFREE;
        semtab[i].queue = queinit();
    }

    /* Initialize monitors */
    for (i = 0; i < NMON; i++)
    {
        montab[i].state = MFREE;
    }

    /* Initialize buffer pools */
    for (i = 0; i < NPOOL; i++)
    {
        bfptab[i].state = BFPFREE;
    }

    /* initialize thread ready list */
    readylist = queinit();

#if RTCLOCK
    /* initialize real time clock */
    clkinit();
#endif                          /* RTCLOCK */

#if NMAILBOX
    /* Mailboxes are used by the ARP, ICMP and routing daemons. */
    mailboxInit();
#endif

#if NDEVS
    for (i = 0; i < NDEVS; i++)
    {
        devtab[i].init((device*)&devtab[i]);
    }
#endif

#if NNETIF
    if (SYSERR == netInit())
    {
        kprintf("WARNING: netInit failed\r\n");
    }
#endif

    return OK;
}
