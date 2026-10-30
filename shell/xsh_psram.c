/**
 * @file     xsh_psram.c
 *
 * Shell command (psram) reports the PSRAM window mapped by the loader and
 * the state of the single kernel heap that lives in it.  There is no
 * separate PSRAM allocator: memget(), stkget(), malloc() and the JVM all
 * use the same free list.
 */
/* Embedded Xinu, ESP32-S3 AMP. */

#include <stddef.h>
#include <platform.h>
#include <psram.h>
#include <thread.h>
#include <memory.h>
#include <conf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define KB(x)   ((ulong)(x) / 1024UL)

/** Whole-percent of @p part with respect to @p whole (rounded). */
static ulong pct(ulong part, ulong whole)
{
    if (0 == whole)
    {
        return 0;
    }
    return (part * 100UL + whole / 2) / whole;
}

static void usage(char *command)
{
    printf("Usage: %s [-t]\n\n", command);
    printf("Description:\n");
    printf("\tReports the PSRAM window and the single kernel heap:\n");
    printf("\tsize, usage, free space and the largest free block.\n");
    printf("Options:\n");
    printf("\t-t\trun a malloc/calloc/realloc/free self-test\n");
}

/**
 * Exercise the standard C allocator on top of memget()/memfree().  This is
 * non-destructive with respect to live allocations: it only touches blocks
 * it owns.
 */
static int allocator_test(void)
{
    unsigned char *a;
    unsigned char *b;
    unsigned char *c;
    int i;
    int bad = 0;

    a = (unsigned char *)malloc(100);
    if (NULL == a)
    {
        return 1;
    }
    for (i = 0; i < 100; i++)
    {
        a[i] = (unsigned char)(i ^ 0x5A);
    }
    for (i = 0; i < 100; i++)
    {
        if (a[i] != (unsigned char)(i ^ 0x5A))
        {
            bad = 1;
        }
    }

    b = (unsigned char *)calloc(16, 16);
    if (NULL == b)
    {
        free(a);
        return 1;
    }
    for (i = 0; i < 16 * 16; i++)
    {
        if (0 != b[i])
        {
            bad = 1;
        }
    }

    c = (unsigned char *)realloc(a, 200);
    if (NULL == c)
    {
        free(a);
        free(b);
        return 1;
    }
    for (i = 0; i < 100; i++)
    {
        if (c[i] != (unsigned char)(i ^ 0x5A))
        {
            bad = 1;
        }
    }

    free(b);
    free(c);
    return bad;
}

/**
 * @ingroup shell
 *
 * Shell command (psram) reports PSRAM and kernel-heap usage.
 *
 * @param nargs number of arguments in args array
 * @param args  array of arguments
 * @return non-zero value on error
 */
shellcmd xsh_psram(int nargs, char *args[])
{
    ulong total;
    ulong free;
    ulong used;
    bool test = FALSE;
    int i;

    for (i = 1; i < nargs; i++)
    {
        if (0 == strcmp(args[i], "--help"))
        {
            usage(args[0]);
            return 0;
        }
        else if (0 == strcmp(args[i], "-t"))
        {
            test = TRUE;
        }
        else
        {
            fprintf(stderr, "%s: invalid argument '%s'\n", args[0], args[i]);
            fprintf(stderr, "Try '%s --help' for more information\n", args[0]);
            return 1;
        }
    }

    printf("External PSRAM and kernel heap\n");
    printf("===========================================================\n");

    if (0 == psram_size || 0 == psram_heap_size)
    {
        printf("  Status            : not available\n");
        printf("===========================================================\n");
        return 0;
    }

    total = (ulong)psram_heap_size;
    free = (ulong)heapFreeBytes();
    used = (ulong)heapUsedBytes();

    printf("  Mapped base       : 0x%08lX\n", (ulong)PSRAM_BASE);
    printf("  Mapped size       : %8lu B  (%lu KB)\n",
           (ulong)psram_size, KB((ulong)psram_size));
    printf("  Kernel reserved   : %8lu B  (%lu KB)\n",
           (ulong)(psram_heap_start - psram_base),
           KB((ulong)(psram_heap_start - psram_base)));
    printf("  Heap base         : 0x%08lX\n", (ulong)psram_heap_start);
    printf("  Heap size         : %8lu B  (%lu KB)\n", total, KB(total));
    printf("-----------------------------------------------------------\n");
    printf("  In use            : %8lu B  %3lu%%\n", used, pct(used, total));
    printf("  Free              : %8lu B  %3lu%%\n", free, pct(free, total));
    printf("  Largest free block: %8lu B\n", (ulong)heapLargestFree());

    /* How much of the heap is currently covered by thread stacks. */
    {
        ulong stack = 0;

        for (i = 0; i < NTHREAD; i++)
        {
            ulong b;

            if (thrtab[i].state == THRFREE)
            {
                continue;
            }
            b = (ulong)thrtab[i].stkbase;
            if (b >= (ulong)psram_heap_start &&
                b < (ulong)psram_heap_start + total)
            {
                stack += thrtab[i].stklen;
            }
        }
        printf("  Thread stacks     : %8lu B\n", stack);
        printf("  Alloc count       : %8lu\n", (ulong)heapAllocCount());
        printf("  Alloc bytes       : %8lu B\n", (ulong)heapAllocBytes());
        printf("  Largest alloc     : %8lu B\n", (ulong)heapLargestAlloc());
    }

    if (test)
    {
        int bad = allocator_test();

        printf("-----------------------------------------------------------\n");
        printf("  Allocator test    : %s\n", bad ? "FAIL" : "PASS");
        if (bad)
        {
            printf("===========================================================\n");
            return 1;
        }
    }

    printf("===========================================================\n");
    return 0;
}
