/**
 * @file bootinfo.h
 *
 * Boot contract between the ESP-IDF loader (core 0) and the Xinu AMP
 * payload (core 1).
 *
 * This file is intentionally data-only.  The loader and the payload are
 * built as separate images with different C ABIs (IDF uses the windowed
 * ABI, Xinu uses CALL0), so no function pointers or compiler-specific
 * objects may cross this boundary.
 *
 * Keep this layout identical in both trees:
 *
 *     xinu-s3-amp/include/bootinfo.h
 *     esp32-xinu-loader/main/bootinfo.h
 *
 * The loader validates magic/version/size before jumping.
 */
/* Embedded Xinu, ESP32-S3 AMP contract. */

#ifndef _BOOTINFO_H_
#define _BOOTINFO_H_

#include <stdint.h>

#define XINU_BOOT_MAGIC       0x58494E55u   /* "XINU" */
#define XINU_BOOT_VERSION     7u

/* bootinfo flags */
#define XINU_BOOT_PSRAM_MAPPED   (1u << 0)  /* PSRAM already mapped by IDF */
#define XINU_BOOT_CONSOLE_OWNED  (1u << 1)  /* Xinu owns UART0               */
#define XINU_BOOT_AMP            (1u << 2)  /* running as core-1 payload    */
#define XINU_BOOT_XIP            (1u << 3)  /* code is executing from flash */

/**
 * Boot information passed in a2 to _start.
 *
 * The loader owns this structure and it must remain valid for the whole
 * lifetime of the payload.  All addresses are absolute virtual addresses.
 * The image header carries the loader-only data (entry point, segment
 * table, XIP size, etc.).
 */
struct xinu_bootinfo
{
    uint32_t magic;             /* 0   XINU_BOOT_MAGIC                    */
    uint32_t version;           /* 4   XINU_BOOT_VERSION                  */
    uint32_t size;              /* 8   sizeof(struct xinu_bootinfo)       */
    uint32_t ccount_hz;         /* 12  CCOUNT/timer frequency, in Hz      */
    uint32_t core;              /* 16  core running Xinu (normally 1)     */
    uint32_t flags;             /* 20  XINU_BOOT_*                        */

    /* PSRAM as mapped by the loader.  Xinu must not initialize the die. */
    uint32_t psram_va;          /* 24  virtual base of Xinu's PSRAM window */
    uint32_t psram_size;        /* 28  usable window from psram_va        */
};

/* In Xinu this points at the bootinfo passed by the loader.  This port is
 * AMP-only, so it is always valid after start.S runs. */
extern struct xinu_bootinfo *xinu_bootinfo;

#endif                          /* _BOOTINFO_H_ */
