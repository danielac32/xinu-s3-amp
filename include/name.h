/**
 * @file name.h
 *
 * Syntactic namespace table definitions.
 *
 * The namespace maps names such as "/dev/tft0" to entries in the device
 * table.  It is only a name-to-device-ID translation table; the actual
 * device initialization and open are still performed by the driver.
 */
/* Embedded Xinu, ESP32-S3 AMP port. */

#ifndef _NAME_H_
#define _NAME_H_

#include <conf.h>

/*
 * Number of prefix mappings.  The old STM32 kernel had NDEVS == 5 and
 * therefore NNAMES == 5.  This port mounts one name per device, so the
 * table must be at least NDEVS.  Leave a few spare entries for aliases.
 */
#ifndef NNAMES
#define NNAMES      (NDEVS + 4)
#endif

#define NM_PRELEN   16      /* Max size of a prefix string              */
#define NM_REPLLEN  1       /* Max size of a replacement string         */
#define NM_MAXLEN   16      /* Max size of a file name                  */

/* Definition of the name prefix table that defines all name mappings. */
struct nmentry
{
    char nprefix[NM_PRELEN];    /* Null-terminated prefix               */
    char nreplace[NM_REPLLEN];  /* Null-terminated replacement          */
    int ndevice;                /* Device descriptor for prefix         */
};

extern struct nmentry nametab[];    /* Table of name mappings           */
extern int nnames;                  /* Number of entries allocated      */

#endif /* _NAME_H_ */
