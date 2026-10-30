/**
 * @file     xsh_ps.c 
 *
 */
/* Embedded Xinu, Copyright (C) 2009.  All rights reserved. */

#include <stddef.h>
#include <thread.h>
#include <stdio.h>
#include <string.h>
#include <clock.h>
#include <stack.h>

/**
 * @ingroup shell
 *
 * Shell command (ps) outputs thread table information.
 * @param nargs  number of arguments in args array
 * @param args   array of arguments
 * @return non-zero value on error
 */
shellcmd xsh_ps(int nargs, char *args[])
{
    struct thrent *thrptr;      /* pointer to thread entry  */
    int i;                      /* temp variable            */
    uint32_t total0;            /* clktotal al empezar      */
    uint32_t total1;            /* clktotal al terminar     */
    uint32_t elapsed;           /* ticks del intervalo      */
    uint32_t ticks0[NTHREAD];   /* cpu_ticks al empezar     */
    uint32_t gen0[NTHREAD];     /* generación al empezar    */

    /* readable names for PR* status in thread.h */
    static const char * const pstnams[] = {
        "curr ", "free ", "ready", "recv ",
        "sleep", "susp ", "wait ", "rtim ", "migr "
    };

    /* Output help, if '--help' argument was supplied */
    if (nargs == 2 && strcmp(args[1], "--help") == 0)
    {
        printf("Usage: %s\n\n", args[0]);
        printf("Description:\n");
        printf("\tDisplays a table of running threads.\n");
        printf("\tCPU%% is measured over a 1 second window.\n");
        printf("\tprnull is the idle task; a high value means idle CPU.\n");
        printf("Options:\n");
        printf("\t--help\t display this help and exit\n");

        return 0;
    }

    /* Check for correct number of arguments */
    if (nargs > 1)
    {
        fprintf(stderr, "%s: too many arguments\n", args[0]);
        fprintf(stderr, "Try '%s --help' for more information\n",
                args[0]);
        return 1;
    }

    /* Tomar una foto de los contadores y medir durante un segundo.
     * Así se muestra el uso reciente, no el promedio desde el arranque. */
    total0 = (uint32_t)clktotal;
    for (i = 0; i < NTHREAD; i++)
    {
        ticks0[i] = thrtab[i].cpu_ticks;
        gen0[i] = thrtab[i].gen;
    }

    sleep(1000);

    total1 = (uint32_t)clktotal;
    elapsed = total1 - total0;
    if (elapsed == 0)
    {
        elapsed = 1;
    }

    /* Output thread table header */
    printf("%3s %-16s %5s %4s %4s %5s %-17s\n",
           "TID", "NAME", "STATE", "PRIO", "PPID", "CPU%",
           "STK peak/total");

    printf("%3s %-16s %5s %4s %4s %5s %-17s\n",
           "---", "----------------", "-----", "----", "----",
           "-----", "---------------");

    /* Output information for each thread */
    for (i = 0; i < NTHREAD; i++)
    {
        uint32_t used;
        uint32_t window;
        uint32_t pct;

        thrptr = &thrtab[i];
        if (thrptr->state == THRFREE)
        {
            continue;
        }

        if (thrptr->gen == gen0[i] && thrptr->born_ticks <= total0)
        {
            /* Tarea que ya existía al empezar: usar el delta. */
            used = thrptr->cpu_ticks - ticks0[i];
            window = elapsed;
        }
        else
        {
            /* Tarea creada (o slot reutilizado) durante la muestra:
             * usar su vida completa, que es más corta que el intervalo. */
            used = thrptr->cpu_ticks;
            window = (uint32_t)clktotal - thrptr->born_ticks;
            if (window == 0)
            {
                window = 1;
            }
        }

        if (used > window)
        {
            used = window;
        }

        /* Evitar la división de 64 bits (el target no enlaza libgcc). */
        if (window >= 100)
        {
            pct = used / (window / 100);
        }
        else
        {
            pct = (used * 100) / window;
        }
        if (pct > 100)
        {
            pct = 100;
        }

        printf("%3d %-16s %s %4d %4d %4lu%% %8lu/%-8lu\n",
               i, thrptr->name,
               (thrptr->state >= 1 && thrptr->state <= 9)
                   ? pstnams[(int)thrptr->state - 1] : "?????",
               thrptr->prio, thrptr->parent,
               (ulong)pct,
               (ulong)stack_peak(i), (ulong)stack_total(i));
    }

    return 0;
}
