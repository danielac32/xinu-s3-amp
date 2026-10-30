/**
 * @file nam.h
 *
 * Syntactic namespace device.
 */
/* Embedded Xinu, ESP32-S3 AMP port. */

#ifndef _NAM_H_
#define _NAM_H_

#include <device.h>
#include <name.h>
#include <stdarg.h>

/* Add a prefix mapping to the namespace. */
syscall mount(const char *prefix, const char *replace, int device);

/* String length helper used by the namespace table. */
int namlen(const char *name, int maxlen);

/* Namespace device entry points. */
devcall naminit(device *devptr);
devcall namopen(device *devptr, va_list ap);

/* Name mapping helpers. */
int nammap(const char *name, char *newname, int namdev);
int namrepl(const char *name, char *newname);
int namcpy(char *newname, const char *oldname, int buflen);

#endif /* _NAM_H_ */
