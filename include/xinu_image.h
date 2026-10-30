/**
 * @file xinu_image.h
 * @brief On-flash packaging format for the Xinu AMP payload.
 *
 * Keep in sync with esp32-xinu-loader/main/xinu_image.h.
 */
#ifndef _XINU_IMAGE_H_
#define _XINU_IMAGE_H_

#include <stdint.h>

#define XINU_IMAGE_MAGIC      0x58494D47u   /* "XIMG" */
#define XINU_IMAGE_VERSION    1u
#define XINU_IMAGE_XIP_BASE   0x42100000u
#define XINU_IMAGE_CODE_BASE  0x42101000u
#define XINU_IMAGE_HEADER_OFF 0x1000u

#define XINU_SEG_XIP    (1u << 0)   /* already in flash at xip_base + offset */
#define XINU_SEG_COPY   (1u << 1)   /* copy from image offset to vaddr        */
#define XINU_SEG_ZERO   (1u << 2)   /* zero size bytes at vaddr               */
#define XINU_SEG_EXEC   (1u << 3)
#define XINU_SEG_WRITE  (1u << 4)
#define XINU_SEG_VECTOR (1u << 5)   /* vector table linked in the XIP window  */

struct xinu_image_header
{
    uint32_t magic;
    uint32_t version;
    uint32_t header_size;
    uint32_t entry;
    uint32_t nseg;
    uint32_t xip_base;
    uint32_t image_size;
    uint32_t crc;
};

struct xinu_image_segment
{
    uint32_t vaddr;
    uint32_t paddr;
    uint32_t size;
    uint32_t offset;
    uint32_t flags;
};

#endif /* _XINU_IMAGE_H_ */
