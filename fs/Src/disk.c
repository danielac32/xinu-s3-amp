/**
 * @file disk.c
 *
 * Low-level disk interface for the FAT file library.
 *
 * This wrapper deliberately goes through the Xinu device table and binds the
 * file system to SD0.  The SDHOST driver underneath is shared with the
 * xsh_sd commands, so the monitor in this file also makes the seek+read and
 * seek+write pairs atomic with respect to other FAT callers.
 */
/* Embedded Xinu, ported to ESP32-S3. */

#include <device.h>
#include <kernel.h>
#include <monitor.h>
#include <stddef.h>
#include <stdint.h>
#include <sd.h>

#include "disk.h"

static monitor s_disk_mon = SYSERR;

static int disk_ready(void)
{
    if (SYSERR == s_disk_mon)
    {
        s_disk_mon = moncreate();
        if (SYSERR == s_disk_mon)
        {
            return 0;
        }
    }

    return (OK == open(SD0)) ? 1 : 0;
}

int sd_init(void)
{
    long blocks;

    if (!disk_ready())
    {
        return 0;
    }

    blocks = control(SD0, SD_CTRL_BLOCKS, 0, 0);
    return (SYSERR == blocks) ? 0 : (int)blocks;
}

int sd_readsector(uint32 sector, uint8 *buffer, uint32 sector_count)
{
    uint32 i;

    if (NULL == buffer || 0 == sector_count || !disk_ready())
    {
        return 0;
    }

    lock(s_disk_mon);
    for (i = 0; i < sector_count; i++)
    {
        if (SYSERR == control(SD0, SD_CTRL_SEEK, (long)(sector + i), 0)
            || SYSERR == read(SD0, buffer + (i * SD_BLOCK_SIZE), SD_BLOCK_SIZE))
        {
            unlock(s_disk_mon);
            return 0;
        }
    }
    unlock(s_disk_mon);
    return 1;
}

int sd_writesector(uint32 sector, uint8 *buffer, uint32 sector_count)
{
    uint32 i;

    if (NULL == buffer || 0 == sector_count || !disk_ready())
    {
        return 0;
    }

    lock(s_disk_mon);
    for (i = 0; i < sector_count; i++)
    {
        if (SYSERR == control(SD0, SD_CTRL_SEEK, (long)(sector + i), 0)
            || SYSERR == write(SD0, buffer + (i * SD_BLOCK_SIZE), SD_BLOCK_SIZE))
        {
            unlock(s_disk_mon);
            return 0;
        }
    }
    unlock(s_disk_mon);
    return 1;
}
