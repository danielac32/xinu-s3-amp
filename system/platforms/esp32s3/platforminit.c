/**
 * @file platforminit.c
 *
 * ESP32-S3 hardware platform initialization for the AMP payload.
 *
 * Runs first thing in nulluser(), while interrupts are still disabled and
 * before the kernel heap is initialized.  IDF already owns the clock tree,
 * flash cache, PSRAM controller and MMU; this function only validates the
 * loader contract, publishes the memory boundaries and disables the
 * watchdogs that would otherwise reset the chip.
 */
/* Embedded Xinu, ESP32-S3 AMP. */

#include <platform.h>
#include <string.h>
#include <stdint.h>
#include <kernel.h>
#include <interrupt.h>
#include <xtensa.h>
#include <psram.h>
#include <bootinfo.h>

/* Top of the panic stack.  start.S fills this from the linker's
 * _panicstk_top symbol; the panic handler uses it when the running thread's
 * stack is not trustworthy. */
void *panicstk;



/* Disables the watchdogs after the C environment exists.  It only writes
 * absolute peripheral registers, so it is safe before the heap is ready. */
void xt_disable_watchdogs(void);

static void invalid_bootinfo(void)
{
    kprintf("xinu: invalid AMP bootinfo\r\n");
    for (;;)
    {
        /* spin */
    }
}

int platforminit(void)
{
    struct xinu_bootinfo *bi = xinu_bootinfo;

    strlcpy(platform.family, "esp32s3", PLT_STRMAX);
    strlcpy(platform.name, "ESP32-S3", PLT_STRMAX);

    if (NULL == bi
        || XINU_BOOT_MAGIC != bi->magic
        || XINU_BOOT_VERSION != bi->version
        || sizeof(struct xinu_bootinfo) != bi->size
        || 0 == bi->ccount_hz
        || 0 == bi->psram_size)
    {
        invalid_bootinfo();
    }

    platform.minaddr = (void *)(uintptr_t)bi->psram_va;
    platform.maxaddr = (void *)(uintptr_t)(bi->psram_va + bi->psram_size);
    platform.clkfreq = bi->ccount_hz;

    platform.uart_dll = 0;
    platform.uart_irqnum = XT_IRQ_UART0;
    platform.serial_low = 0;
    platform.serial_high = 0;

    /* The loader already mapped PSRAM.  Publish the exact window; Xinu must
     * not reinitialize the die.  The heap is derived from _end below. */
    psram_base = bi->psram_va;
    psram_size = bi->psram_size;

    /* IDF owns the clock tree, but it does not necessarily leave the
     * peripheral gates enabled.  These are the UART console, the bit-banged
     * SPI2/SPI3 buses used by SD/TFT/W5500, and the timer group.  This does
     * not change the CPU/PLL frequency; CCOUNT comes from bootinfo. */
    REG32(SYSTEM_PERIP_CLK_EN0) |= SYS_CLK_EN_UART0 | SYS_CLK_EN_UART_MEM
                                 | SYS_CLK_EN_SPI2 | SYS_CLK_EN_SPI3
                                 | SYS_CLK_EN_TIMERGROUP;

    /* IDF leaves the timer-group watchdog armed.  Nothing feeds it once the
     * AMP handoff is complete, so disable it together with the RTC/SWD
     * watchdogs. */
    xt_disable_watchdogs();

    return OK;
}

/**
 * Disable the RTC watchdog, the "super" watchdog and the Timer Group 0
 * main watchdog.  Any of them will reset the chip if left armed.
 */
void xt_disable_watchdogs(void)
{
    /* Super watchdog (SWD). */
    REG32(RTC_CNTL_SWD_WPROTECT) = RTC_CNTL_SWD_WKEY;
    REG32(RTC_CNTL_WDTCONFIG1) = 0;
    REG32(RTC_CNTL_SWD_CONF) = (1U << 30);   /* SWD_AUTO_FEED_EN */
    REG32(RTC_CNTL_SWD_WPROTECT) = 0;

    /* RTC watchdog. */
    REG32(RTC_CNTL_WDTWPROTECT) = RTC_CNTL_WDT_WKEY;
    REG32(RTC_CNTL_WDTCONFIG0) = 0;
    REG32(RTC_CNTL_WDTWPROTECT) = 0;

    /* Timer Group 0 main watchdog. */
    REG32(TIMG0_WDTWPROTECT) = TIMG0_WDT_WKEY;
    REG32(TIMG0_WDTCONFIG0) = 0;
    REG32(TIMG0_WDTWPROTECT) = 0;
}
