/**
 * @file start.S
 *
 * Startup code for the Xinu ESP32-S3 AMP payload.
 *
 * The IDF loader has already initialized the clock tree, flash cache,
 * PSRAM and the MMU.  It passes a pointer to struct xinu_bootinfo in a2 and
 * jumps to _start.  This code masks interrupts, points VECBASE at the
 * vector table linked in the XIP window, clears .bss and enters nulluser().
 *
 * All kernel code runs from flash XIP.  All data, .bss, the boot stacks and
 * the kernel heap live in PSRAM.
 */
/* Embedded Xinu, ESP32-S3 AMP. */

    .section .init, "ax"
    .globl  _start
    .align  4
    .type   _start, @function
_start:
    /* The loader passes the bootinfo pointer in a2 (CALL0 first argument).
     * Keep it in a6 while we set up the core and clear .bss. */
    mov     a6, a2

    /* Mask every interrupt, clear the pending ones and stop the core
     * timers (CCOMPARE0..2). */
    movi    a2, 0
    wsr     a2, interrupt
    wsr     a2, intenable
    movi    a2, -1
    wsr     a2, intclear
    wsr     a2, ccompare0
    wsr     a2, ccompare1
    wsr     a2, ccompare2

    /* The vector table is linked in the XIP window.  Xinu knows its own
     * address, so the loader does not need to publish it. */
    movi    a2, _xt_vector_table
    wsr     a2, vecbase
    rsync

/* Copy .rodata from its flash LMA to the PSRAM VMA.  The LMA is in the
     * instruction window; the same flash bytes are readable through IDF's
     * flash-data window, which is 0x06000000 below the XIP window. */
    movi    a2, _rodata_lma
    movi    a5, 0x06000000
    sub     a2, a2, a5
    movi    a3, _rodata_start
    movi    a4, _rodata_end
.Lrodata_copy:
    bgeu    a3, a4, .Lrodata_done
    l8ui    a5, a2, 0
    s8i     a5, a3, 0
    addi    a2, a2, 1
    addi    a3, a3, 1
    j       .Lrodata_copy
.Lrodata_done:

    /* Copy .data the same way. */
    movi    a2, _data_lma
    movi    a5, 0x06000000
    sub     a2, a2, a5
    movi    a3, _data_start
    movi    a4, _data_end
.Ldata_copy:
    bgeu    a3, a4, .Ldata_done
    l8ui    a5, a2, 0
    s8i     a5, a3, 0
    addi    a2, a2, 1
    addi    a3, a3, 1
    j       .Ldata_copy
.Ldata_done:

    /* Zero .bss and the boot stacks. */
    movi    a2, _bss_start
    movi    a3, _stacks_end
    movi    a4, 0
.Lzero_loop:
    bgeu    a2, a3, .Lzero_done
    s8i     a4, a2, 0
    addi    a2, a2, 1
    j       .Lzero_loop
.Lzero_done:

    /* Publish the loader's bootinfo pointer after .bss has been cleared. */
    movi    a4, xinu_bootinfo
    s32i    a6, a4, 0

    /* The linker reserves .stacks (panic + null) immediately before _end.
     * The panic stack grows down from _panicstk_top; the null thread runs
     * on the stack above it, up to _nullstk_top.  The kernel heap starts
     * after _end and never overlaps either stack. */
    movi    a3, _panicstk_top
    movi    a4, panicstk
    s32i    a3, a4, 0
    movi    sp, _nullstk_top

    movi    a0, nulluser
    jx      a0

.Lspin:
    j       .Lspin
    .size   _start, . - _start
