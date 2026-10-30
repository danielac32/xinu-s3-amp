/**
 * @file rgbRead.c
 *
 * read(RGB0, buf, 3): copy the current red, green and blue channel values.
 * When the LED is off the three bytes are zero, matching the colour reported
 * by RGB_CTRL_GET.  This mirrors write(RGB0, buf, 3).
 */
/* Embedded Xinu, ported to ESP32-S3. */

#include <device.h>
#include <kernel.h>
#include <stddef.h>
#include <stdint.h>
#include <rgb.h>

devcall rgbRead(device *devptr, void *buf, uint len)
{
    struct rgb *p = &rgbtab[devptr->minor];
    uint8_t *rgb = (uint8_t *)buf;

    if (!p->ready || NULL == rgb || len < 3)
    {
        return SYSERR;
    }

    lock(p->lock);
    if (p->lit)
    {
        rgb[0] = p->r;
        rgb[1] = p->g;
        rgb[2] = p->b;
    }
    else
    {
        rgb[0] = 0;
        rgb[1] = 0;
        rgb[2] = 0;
    }
    unlock(p->lock);

    return 3;
}
