/**
 * @file xsh_udpserver.c
 *
 * Small UDP command server for the ESP32-S3 AMP port.
 *
 * The server listens on a fixed port, accepts one request at a time and
 * answers the sender.  The command channel is plain text; the firmware
 * upload keeps a small session and accepts binary blocks after an ASCII
 * header.  The Qt application can use it for the dashboard, hardware tests
 * and the firmware update.
 */
/* Embedded Xinu, ported to ESP32-S3. */

#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <button.h>
#include <clock.h>
#include <device.h>
#include <fat_filelib.h>
#include <fat_port.h>
#include <memory.h>
#include <network.h>
#include <platform.h>
#include <psram.h>
#include <rgb.h>
#include <sd.h>
#include <shell.h>
#include <tft.h>
#include <thread.h>
#include <udp.h>
#include <version.h>
#include <xtensa.h>

#define UDPSERVER_PORT      5000
#define UDPSERVER_STK       12288
#define UDPSERVER_REPLY     1024
#define UDPSERVER_RX        (sizeof(struct udpPseudoHdr) + UDP_HDR_LEN + \
                             UDP_MAX_DATALEN)
#define UDPSERVER_MAXARGS   8
#define UDPSERVER_GPIO_MAX  48

/* Firmware upload.  The block size leaves room for the longest ASCII
 * header inside one UDP payload. */
#define FW_BLOCK_SIZE       960
#define FW_MAX_IMAGE        0x200000UL
#define FW_TEMP_PATH        "/xinu.tmp"
#define FW_FINAL_PATH       "/xinu.img"

#if NETHER

static bool udpServerStarted = FALSE;

/** State of the current firmware upload, if any. */
struct fwstate
{
    bool active;
    FL_FILE *file;
    uint32_t expected;
    uint32_t received;
    uint32_t next_seq;
    struct netaddr client;
    ushort client_port;
};

static struct fwstate fw;

/** Bounded output buffer used by udpAppend(). */
struct udpout
{
    char *buf;
    uint cap;
    uint len;
};

/** _doprnt() callback that never writes past the end of the buffer. */
static int udpOutPutc(int c, uintptr_t arg)
{
    struct udpout *out = (struct udpout *)arg;

    if (out->len + 1 < out->cap)
    {
        out->buf[out->len++] = (char)c;
        out->buf[out->len] = '\0';
    }
    return (unsigned char)c;
}

/** Append a formatted fragment to an already null-terminated buffer. */
static void udpAppend(char *buf, uint cap, const char *fmt, ...)
{
    struct udpout out;
    va_list ap;

    out.buf = buf;
    out.cap = cap;
    out.len = strlen(buf);

    va_start(ap, fmt);
    _doprnt(fmt, ap, udpOutPutc, (uintptr_t)&out);
    va_end(ap);
}

/** Split a command line in place. */
static int udpSplit(char *line, char **argv, int maxargs)
{
    int argc = 0;
    char *p = line;

    while (*p != '\0' && argc < maxargs)
    {
        while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
        {
            p++;
        }
        if (*p == '\0')
        {
            break;
        }

        argv[argc++] = p;
        while (*p != '\0' && *p != ' ' && *p != '\t'
               && *p != '\r' && *p != '\n')
        {
            p++;
        }
        if (*p != '\0')
        {
            *p++ = '\0';
        }
    }

    return argc;
}

/**
 * Send one text response.  The receive side is in passive mode so that the
 * source address is available; temporarily leave passive mode to send with
 * the remote address bound by udpHandle().
 */
static void udpReply(int dev, const char *text)
{
    int len = strlen(text);

    if (len <= 0)
    {
        return;
    }

    control(dev, UDP_CTRL_CLRFLAG, UDP_FLAG_PASSIVE, 0);
    write(dev, text, len);
    control(dev, UDP_CTRL_SETFLAG, UDP_FLAG_PASSIVE, 0);
}

/** Close the current upload, optionally deleting its temporary file. */
static void fwReset(bool remove_temp)
{
    if (NULL != fw.file)
    {
        fl_fclose(fw.file);
        fw.file = NULL;
    }
    if (remove_temp && fat_mounted())
    {
        fl_remove(FW_TEMP_PATH);
    }

    fw.active = FALSE;
    fw.expected = 0;
    fw.received = 0;
    fw.next_seq = 0;
    bzero(&fw.client, sizeof(fw.client));
    fw.client_port = 0;
}

/** TRUE when the request comes from the client that opened the session. */
static bool fwClientMatches(const struct netaddr *src, ushort srcport)
{
    return (NULL != src) && (fw.client_port == srcport)
        && netaddrequal(&fw.client, src);
}

/** Check the size stored in a FAT directory entry. */
static bool fwFileSizeIs(const char *path, uint32_t expected)
{
    FL_FILE *file;
    bool ok;

    file = fl_fopen(path, "r");
    if (NULL == file)
    {
        return FALSE;
    }

    ok = (file->filelength == expected);
    fl_fclose(file);
    return ok;
}

/** Start a new upload, aborting any previous one. */
static void fwBegin(char *reply, uint cap, unsigned long size,
                    const struct netaddr *src, ushort srcport)
{
    FL_FILE *file;

    if (0 == size || size > FW_MAX_IMAGE)
    {
        udpAppend(reply, cap, "ERR FW_SIZE max=%lu\r\n",
                  (unsigned long)FW_MAX_IMAGE);
        return;
    }

    /* A new BEGIN discards a stale temporary file and any open session. */
    fwReset(TRUE);

    if (!fat_mounted() && !fat_mount())
    {
        udpAppend(reply, cap, "ERR FW_NO_SD\r\n");
        return;
    }

    file = fl_fopen(FW_TEMP_PATH, "w");
    if (NULL == file)
    {
        udpAppend(reply, cap, "ERR FW_OPEN temp=%s\r\n", FW_TEMP_PATH);
        return;
    }

    fw.active = TRUE;
    fw.file = file;
    fw.expected = (uint32_t)size;
    fw.received = 0;
    fw.next_seq = 0;
    bzero(&fw.client, sizeof(fw.client));
    if (NULL != src)
    {
        netaddrcpy(&fw.client, src);
    }
    fw.client_port = srcport;

    udpAppend(reply, cap,
              "OK FW_BEGIN size=%lu block=%u temp=%s\r\n",
              (unsigned long)fw.expected, FW_BLOCK_SIZE, FW_TEMP_PATH);
}

/**
 * Handle one binary firmware block.
 *
 * Wire format: "FW_DATA <seq> <len>\n" followed by exactly <len> bytes.
 * Sequence numbers start at 0 and must be consecutive.  A retransmitted
 * block with an older sequence number is acknowledged without writing it
 * again.
 */
static void fwHandleData(int dev, const char *data, int total,
                         const struct netaddr *src, ushort srcport)
{
    char header[64];
    char reply[256];
    const char *body;
    int header_len = -1;
    int body_len;
    int i;
    unsigned long seq;
    unsigned int block_len;

    reply[0] = '\0';

    if (!fw.active || NULL == fw.file)
    {
        udpAppend(reply, sizeof(reply), "ERR FW_IDLE\r\n");
        udpReply(dev, reply);
        return;
    }

    if (!fwClientMatches(src, srcport))
    {
        udpAppend(reply, sizeof(reply), "ERR FW_CLIENT\r\n");
        udpReply(dev, reply);
        return;
    }

    for (i = 0; i < total && i < (int)sizeof(header) - 1; i++)
    {
        if (data[i] == '\n')
        {
            header_len = i;
            break;
        }
    }

    if (header_len < 0)
    {
        udpAppend(reply, sizeof(reply), "ERR FW_HEADER\r\n");
        udpReply(dev, reply);
        return;
    }

    memcpy(header, data, header_len);
    header[header_len] = '\0';
    if (header_len > 0 && header[header_len - 1] == '\r')
    {
        header[header_len - 1] = '\0';
    }

    if (2 != sscanf(header, "FW_DATA %lu %u", &seq, &block_len))
    {
        udpAppend(reply, sizeof(reply), "ERR FW_USAGE\r\n");
        udpReply(dev, reply);
        return;
    }

    body = data + header_len + 1;
    body_len = total - header_len - 1;

    if (body_len != (int)block_len)
    {
        udpAppend(reply, sizeof(reply),
                  "ERR FW_LEN header=%u actual=%d\r\n",
                  block_len, body_len);
        udpReply(dev, reply);
        return;
    }

    if (0 == block_len || block_len > FW_BLOCK_SIZE)
    {
        udpAppend(reply, sizeof(reply),
                  "ERR FW_BLOCK max=%u\r\n", FW_BLOCK_SIZE);
        udpReply(dev, reply);
        return;
    }

    if (seq > fw.next_seq)
    {
        udpAppend(reply, sizeof(reply),
                  "ERR FW_SEQ expected=%lu got=%lu\r\n",
                  (unsigned long)fw.next_seq, seq);
        udpReply(dev, reply);
        return;
    }

    if (seq < fw.next_seq)
    {
        udpAppend(reply, sizeof(reply),
                  "OK FW_DATA %lu duplicate %lu/%lu\r\n",
                  seq, (unsigned long)fw.received,
                  (unsigned long)fw.expected);
        udpReply(dev, reply);
        return;
    }

    if (fw.received > fw.expected
        || block_len > (fw.expected - fw.received))
    {
        udpAppend(reply, sizeof(reply),
                  "ERR FW_OVERFLOW received=%lu len=%u size=%lu\r\n",
                  (unsigned long)fw.received, block_len,
                  (unsigned long)fw.expected);
        udpReply(dev, reply);
        return;
    }

    if (fl_fwrite(body, 1, (int)block_len, fw.file) != (int)block_len
        || fl_ftell(fw.file) != (long)(fw.received + block_len))
    {
        fwReset(TRUE);
        udpAppend(reply, sizeof(reply), "ERR FW_WRITE\r\n");
        udpReply(dev, reply);
        return;
    }

    fw.received += block_len;
    fw.next_seq++;

    udpAppend(reply, sizeof(reply),
              "OK FW_DATA %lu %lu/%lu%s\r\n",
              seq, (unsigned long)fw.received, (unsigned long)fw.expected,
              (fw.received == fw.expected) ? " complete" : "");
    udpReply(dev, reply);
}

/** Finish the upload and leave the new image ready for the loader. */
static bool fwEnd(char *reply, uint cap, const struct netaddr *src,
                  ushort srcport)
{
    uint32_t size;

    if (!fw.active || NULL == fw.file)
    {
        udpAppend(reply, cap, "ERR FW_IDLE\r\n");
        return FALSE;
    }

    if (!fwClientMatches(src, srcport))
    {
        udpAppend(reply, cap, "ERR FW_CLIENT\r\n");
        return FALSE;
    }

    if (fw.received != fw.expected)
    {
        udpAppend(reply, cap,
                  "ERR FW_SIZE expected=%lu received=%lu\r\n",
                  (unsigned long)fw.expected,
                  (unsigned long)fw.received);
        return FALSE;
    }

    size = fw.expected;

    fl_fclose(fw.file);
    fw.file = NULL;

    if (!fwFileSizeIs(FW_TEMP_PATH, size))
    {
        fwReset(TRUE);
        udpAppend(reply, cap, "ERR FW_VERIFY\r\n");
        return FALSE;
    }

    /* fl_rename() moves the existing cluster chain to the final name.  It
     * removes any previous /xinu.img first and never copies the image. */
    if (fl_rename(FW_TEMP_PATH, FW_FINAL_PATH) != 0
        || !fwFileSizeIs(FW_FINAL_PATH, size))
    {
        /* Never leave a partial or stale image with the name the loader
         * reads.  The current Xinu partition remains untouched. */
        fl_remove(FW_FINAL_PATH);
        fwReset(TRUE);
        udpAppend(reply, cap, "ERR FW_FINALIZE\r\n");
        return FALSE;
    }

    fwReset(TRUE);

    udpAppend(reply, cap, "OK FW_END size=%lu rebooting\r\n",
              (unsigned long)size);
    return TRUE;
}

/** Abort the current upload. */
static void fwAbort(char *reply, uint cap, const struct netaddr *src,
                    ushort srcport)
{
    if (fw.active && !fwClientMatches(src, srcport))
    {
        udpAppend(reply, cap, "ERR FW_CLIENT\r\n");
        return;
    }

    fwReset(TRUE);
    udpAppend(reply, cap, "OK FW_ABORT\r\n");
}

/** Report the current upload state. */
static void fwStatus(char *reply, uint cap)
{
    if (!fw.active)
    {
        udpAppend(reply, cap, "FW_STATUS IDLE\r\n");
        return;
    }

    udpAppend(reply, cap,
              "FW_STATUS ACTIVE size=%lu received=%lu next=%lu block=%u "
              "temp=%s\r\n",
              (unsigned long)fw.expected, (unsigned long)fw.received,
              (unsigned long)fw.next_seq, FW_BLOCK_SIZE, FW_TEMP_PATH);
}

/** Append the interface address fields shared by INFO and NET. */
static void udpAppendNet(char *reply, uint cap)
{
    struct netif *iface;
    char ip[32];
    char mask[32];
    char gateway[32];
    char mac[32];

    iface = netLookup(ETH0);
    if (NULL == iface)
    {
        udpAppend(reply, cap, "ERR no network interface\r\n");
        return;
    }

    netaddrsprintf(ip, &iface->ip);
    netaddrsprintf(mask, &iface->mask);
    netaddrsprintf(gateway, &iface->gateway);
    netaddrsprintf(mac, &iface->hwaddr);

    udpAppend(reply, cap, "ip: %s\r\n", ip);
    udpAppend(reply, cap, "netmask: %s\r\n", mask);
    udpAppend(reply, cap, "gateway: %s\r\n", gateway);
    udpAppend(reply, cap, "mac: %s\r\n", mac);
    udpAppend(reply, cap, "mtu: %u\r\n", iface->mtu);
}

/** Handle the GPIO command family. */
static void udpHandleGpio(char *reply, uint cap, int argc, char **argv)
{
    int pin;
    int value;

    if (argc < 3)
    {
        udpAppend(reply, cap, "ERR usage: GPIO READ|WRITE|MODE <pin> [value]\r\n");
        return;
    }

    pin = atoi(argv[2]);
    if (pin < 0 || pin >= UDPSERVER_GPIO_MAX)
    {
        udpAppend(reply, cap, "ERR invalid pin %d\r\n", pin);
        return;
    }

    if (0 == strcmp(argv[1], "READ"))
    {
        udpAppend(reply, cap, "GPIO %d = %u\r\n", pin,
                  xt_gpio_get_level((uint)pin));
        return;
    }

    /* GPIO15 is the shared reset line of the TFT and the W5500.  Driving it
     * from the network would reset the Ethernet controller and drop the
     * connection carrying this very reply.  Reading it is still allowed. */
    if (pin == TFT_PIN_RST && 0 != strcmp(argv[1], "READ"))
    {
        udpAppend(reply, cap,
                  "ERR GPIO15 is the shared TFT/W5500 reset line\r\n");
        return;
    }

    if (0 == strcmp(argv[1], "WRITE") && argc >= 4)
    {
        value = atoi(argv[3]) ? 1 : 0;
        xt_gpio_output((uint)pin, 0);
        xt_gpio_set_level((uint)pin, (uint)value);
        udpAppend(reply, cap, "GPIO %d = %d\r\n", pin, value);
        return;
    }

    if (0 == strcmp(argv[1], "MODE") && argc >= 4)
    {
        if (0 == strcmp(argv[3], "OUT"))
        {
            xt_gpio_output((uint)pin, 0);
            udpAppend(reply, cap, "GPIO %d mode OUT\r\n", pin);
            return;
        }
        if (0 == strcmp(argv[3], "IN"))
        {
            xt_gpio_input((uint)pin, 0);
            udpAppend(reply, cap, "GPIO %d mode IN\r\n", pin);
            return;
        }
        udpAppend(reply, cap, "ERR usage: GPIO MODE <pin> IN|OUT\r\n");
        return;
    }

    udpAppend(reply, cap, "ERR usage: GPIO READ|WRITE|MODE <pin> [value]\r\n");
}

/** Handle the SD command family. */
static void udpHandleSd(char *reply, uint cap, int argc, char **argv)
{
    struct sdinfo info;

    if (argc < 2 || 0 != strcmp(argv[1], "INFO"))
    {
        udpAppend(reply, cap, "ERR usage: SD INFO\r\n");
        return;
    }

    if (SYSERR == open(SD0))
    {
        udpAppend(reply, cap, "SD: no card\r\n");
        return;
    }

    if (SYSERR == control(SD0, SD_CTRL_INFO, (long)&info, 0))
    {
        udpAppend(reply, cap, "SD: card not available\r\n");
        return;
    }

    udpAppend(reply, cap, "SD: %s\r\n", info.present ? "present" : "missing");
    udpAppend(reply, cap, "blocks: %lu\r\n", (unsigned long)info.blocks);
    udpAppend(reply, cap, "size_mb: %lu\r\n",
              (unsigned long)(info.blocks / 2048UL));
    udpAppend(reply, cap, "type: %s\r\n",
              info.highcap ? "SDHC/SDXC" : "SDSC");
    udpAppend(reply, cap, "bus: %u-bit at %lu kHz\r\n", info.bus_width,
              (unsigned long)info.card_clk_khz);
    udpAppend(reply, cap, "rca: 0x%04x\r\n", info.rca);
    udpAppend(reply, cap, "fat: %s\r\n",
              fat_mounted() ? "mounted" : "not mounted");
}

/** Handle the TFT command family. */
static void udpHandleTft(char *reply, uint cap, int argc, char **argv)
{
    if (argc < 2 || 0 != strcmp(argv[1], "TEST"))
    {
        udpAppend(reply, cap, "ERR usage: TFT TEST\r\n");
        return;
    }

    if (SYSERR == open(TFT0))
    {
        udpAppend(reply, cap, "ERR TFT not available\r\n");
        return;
    }

    control(TFT0, TFTCTL_CLEAR, TFT_BLACK, 0);
    control(TFT0, TFTCTL_SETCOLOR, TFT_GREEN, TFT_BLACK);
    control(TFT0, TFTCTL_CURSOR, 4, 4);
    write(TFT0, "UDP TFT TEST", 12);
    control(TFT0, TFTCTL_SETCOLOR, TFT_WHITE, TFT_BLACK);
    control(TFT0, TFTCTL_CURSOR, 4, 20);
    write(TFT0, "OK", 2);
    control(TFT0, TFTCTL_FLUSH, 0, 0);

    udpAppend(reply, cap, "TFT: test pattern drawn\r\n");
}

/** Handle the BUTTON command family. */
static void udpHandleButton(char *reply, uint cap)
{
    if (SYSERR == open(BTN0)
        && SYSERR == control(BTN0, BUTTON_CTRL_PRESSED, 0, 0))
    {
        udpAppend(reply, cap, "ERR button not available\r\n");
        return;
    }

    udpAppend(reply, cap, "BUTTON: %s\r\n",
              control(BTN0, BUTTON_CTRL_PRESSED, 0, 0) ? "down" : "up");
}

/** Parse exactly one to six hexadecimal digits into a 24-bit colour. */
static bool udpParseRgbColour(const char *text, unsigned long *value)
{
    unsigned long colour = 0;
    int digits = 0;

    if (NULL == text || '\0' == *text)
    {
        return FALSE;
    }

    while (*text != '\0')
    {
        int digit;

        if (*text >= '0' && *text <= '9')
        {
            digit = *text - '0';
        }
        else if (*text >= 'a' && *text <= 'f')
        {
            digit = *text - 'a' + 10;
        }
        else if (*text >= 'A' && *text <= 'F')
        {
            digit = *text - 'A' + 10;
        }
        else
        {
            return FALSE;
        }

        if (digits >= 6)
        {
            return FALSE;
        }
        colour = (colour << 4) | (unsigned long)digit;
        ++digits;
        ++text;
    }

    *value = colour;
    return TRUE;
}

/** Handle the RGB LED command family.
 *
 *   RGB SET <rrggbb>   set the three channels (hex, 000000..ffffff)
 *   RGB OFF            turn all channels off
 *   RGB ON             restore the last colour
 *   RGB GET            report the last colour
 *
 * The discrete LED is wired to BOARD_RGB_* in <board.h>.  The command is
 * separate from GPIO so the three channels are changed atomically.
 */
static void udpHandleRgb(char *reply, uint cap, int argc, char **argv)
{
    unsigned long colour;
    uint8_t channels[3];
    /* open() returns OK/SYSERR; RGB0 itself is the descriptor used by
     * control(), write() and read(). */
    int rgbdev = RGB0;

    if (argc < 2)
    {
        udpAppend(reply, cap, "ERR usage: RGB SET <rrggbb>|OFF|ON|GET\r\n");
        return;
    }

    if (SYSERR == open(rgbdev))
    {
        udpAppend(reply, cap, "ERR RGB not available\r\n");
        return;
    }

    if (0 == strcmp(argv[1], "SET") && argc >= 3)
    {
        if (!udpParseRgbColour(argv[2], &colour))
        {
            close(rgbdev);
            udpAppend(reply, cap, "ERR RGB colour must be rrggbb\r\n");
            return;
        }

        channels[0] = (uint8_t)((colour >> 16) & 0xFF);
        channels[1] = (uint8_t)((colour >> 8) & 0xFF);
        channels[2] = (uint8_t)(colour & 0xFF);
        if (SYSERR == write(rgbdev, channels, sizeof(channels)))
        {
            close(rgbdev);
            udpAppend(reply, cap, "ERR RGB write failed\r\n");
            return;
        }

        close(rgbdev);
        udpAppend(reply, cap, "RGB %06lx\r\n", colour);
        return;
    }

    if (0 == strcmp(argv[1], "OFF"))
    {
        if (SYSERR == control(rgbdev, RGB_CTRL_OFF, 0, 0))
        {
            close(rgbdev);
            udpAppend(reply, cap, "ERR RGB control failed\r\n");
            return;
        }
        close(rgbdev);
        udpAppend(reply, cap, "RGB off\r\n");
        return;
    }

    if (0 == strcmp(argv[1], "ON"))
    {
        if (SYSERR == control(rgbdev, RGB_CTRL_ON, 0, 0))
        {
            close(rgbdev);
            udpAppend(reply, cap, "ERR RGB control failed\r\n");
            return;
        }
        if (3 != read(rgbdev, channels, sizeof(channels)))
        {
            close(rgbdev);
            udpAppend(reply, cap, "ERR RGB read failed\r\n");
            return;
        }
        close(rgbdev);
        colour = ((unsigned long)channels[0] << 16)
                 | ((unsigned long)channels[1] << 8)
                 | (unsigned long)channels[2];
        udpAppend(reply, cap, "RGB %06lx\r\n", colour & 0xFFFFFFUL);
        return;
    }

    if (0 == strcmp(argv[1], "GET"))
    {
        if (3 != read(rgbdev, channels, sizeof(channels)))
        {
            close(rgbdev);
            udpAppend(reply, cap, "ERR RGB read failed\r\n");
            return;
        }
        close(rgbdev);
        colour = ((unsigned long)channels[0] << 16)
                 | ((unsigned long)channels[1] << 8)
                 | (unsigned long)channels[2];
        udpAppend(reply, cap, "RGB %06lx\r\n", colour & 0xFFFFFFUL);
        return;
    }

    close(rgbdev);
    udpAppend(reply, cap, "ERR usage: RGB SET <rrggbb>|OFF|ON|GET\r\n");
}

/** Handle a single request. */
static void udpHandle(int dev, const char *data, int len,
                      const struct netaddr *src, ushort srcport)
{
    static const char * const pstnams[] = {
        "curr", "free", "ready", "recv", "sleep", "susp", "wait", "rtim",
        "migr"
    };
    char line[UDPSERVER_REPLY];
    char reply[UDPSERVER_REPLY];
    char *argv[UDPSERVER_MAXARGS];
    unsigned long size;
    int argc;
    int i;

    if (len <= 0)
    {
        return;
    }

    /* FW_DATA carries a short ASCII header followed by binary bytes, so it
     * must bypass the text splitter and its null-terminated line buffer. */
    if (len >= 8 && 0 == memcmp(data, "FW_DATA", 7) && data[7] == ' ')
    {
        fwHandleData(dev, data, len, src, srcport);
        return;
    }

    if (len >= (int)sizeof(line))
    {
        len = (int)sizeof(line) - 1;
    }
    memcpy(line, data, len);
    line[len] = '\0';

    /* Drop trailing line endings and spaces left by a text client. */
    while (len > 0
           && (line[len - 1] == '\r' || line[len - 1] == '\n'
               || line[len - 1] == ' ' || line[len - 1] == '\t'))
    {
        line[--len] = '\0';
    }

    argc = udpSplit(line, argv, UDPSERVER_MAXARGS);
    if (argc < 1)
    {
        return;
    }

    reply[0] = '\0';

    if (0 == strcmp(argv[0], "PING"))
    {
        udpAppend(reply, sizeof(reply), "PONG %lu\r\n",
                  (unsigned long)clktotal);
    }
    else if (0 == strcmp(argv[0], "INFO"))
    {
        udpAppend(reply, sizeof(reply), "Xinu ESP32-S3 AMP\r\n");
        udpAppend(reply, sizeof(reply), "version: %s\r\n", VERSION);
        udpAppend(reply, sizeof(reply), "uptime_ms: %lu\r\n",
                  (unsigned long)clktotal);
        udpAppendNet(reply, sizeof(reply));
        udpAppend(reply, sizeof(reply), "heap_free: %lu\r\n",
                  (unsigned long)heapFreeBytes());
        udpAppend(reply, sizeof(reply), "psram_size: %lu\r\n",
                  (unsigned long)psram_size);
    }
    else if (0 == strcmp(argv[0], "VERSION"))
    {
        udpAppend(reply, sizeof(reply), "%s\r\n", VERSION);
    }
    else if (0 == strcmp(argv[0], "UPTIME"))
    {
        udpAppend(reply, sizeof(reply), "uptime_ms: %lu\r\n",
                  (unsigned long)clktotal);
    }
    else if (0 == strcmp(argv[0], "MEM"))
    {
        udpAppend(reply, sizeof(reply), "heap_free: %lu\r\n",
                  (unsigned long)heapFreeBytes());
        udpAppend(reply, sizeof(reply), "heap_used: %lu\r\n",
                  (unsigned long)heapUsedBytes());
        udpAppend(reply, sizeof(reply), "heap_largest: %lu\r\n",
                  (unsigned long)heapLargestFree());
        udpAppend(reply, sizeof(reply), "alloc_count: %lu\r\n",
                  (unsigned long)heapAllocCount());
        udpAppend(reply, sizeof(reply), "alloc_bytes: %lu\r\n",
                  (unsigned long)heapAllocBytes());
        udpAppend(reply, sizeof(reply), "psram_size: %lu\r\n",
                  (unsigned long)psram_size);
    }
    else if (0 == strcmp(argv[0], "PS"))
    {
        uint32_t total0;
        uint32_t total1;
        uint32_t elapsed;
        uint32_t ticks0[NTHREAD];
        uint32_t gen0[NTHREAD];

        /* Medir el uso de CPU en una ventana reciente de 1 segundo.
         * El promedio desde el arranque hacía que prnull (idle) saliera
         * siempre al 99% y las demás tareas se redondearan a 0%. */
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

        udpAppend(reply, sizeof(reply),
                  "TID NAME             STATE PRIO PPID CPU% STACK\r\n");

        for (i = 0; i < NTHREAD; i++)
        {
            struct thrent *thrptr = &thrtab[i];
            uint32_t used;
            uint32_t window;
            uint32_t pct;

            if (thrptr->state == THRFREE)
            {
                continue;
            }

            if (thrptr->gen == gen0[i] && thrptr->born_ticks <= total0)
            {
                used = thrptr->cpu_ticks - ticks0[i];
                window = elapsed;
            }
            else
            {
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

            udpAppend(reply, sizeof(reply),
                      "%3d %-16s %-5s %4d %4d %3lu%% %lu/%lu\r\n",
                      i, thrptr->name,
                      (thrptr->state >= 1 && thrptr->state <= 9)
                          ? pstnams[(int)thrptr->state - 1] : "?????",
                      thrptr->prio, thrptr->parent,
                      (unsigned long)pct,
                      (unsigned long)thrptr->stk_peak,
                      (unsigned long)thrptr->stklen);
        }
    }
    else if (0 == strcmp(argv[0], "NET"))
    {
        udpAppendNet(reply, sizeof(reply));
    }
    else if (0 == strcmp(argv[0], "GPIO"))
    {
        udpHandleGpio(reply, sizeof(reply), argc, argv);
    }
    else if (0 == strcmp(argv[0], "SD"))
    {
        udpHandleSd(reply, sizeof(reply), argc, argv);
    }
    else if (0 == strcmp(argv[0], "TFT"))
    {
        udpHandleTft(reply, sizeof(reply), argc, argv);
    }
    else if (0 == strcmp(argv[0], "BUTTON"))
    {
        udpHandleButton(reply, sizeof(reply));
    }
    else if (0 == strcmp(argv[0], "RGB"))
    {
        udpHandleRgb(reply, sizeof(reply), argc, argv);
    }
    else if (0 == strcmp(argv[0], "FW_BEGIN"))
    {
        if (argc < 2 || 1 != sscanf(argv[1], "%lu", &size))
        {
            udpAppend(reply, sizeof(reply),
                      "ERR usage: FW_BEGIN <image-size>\r\n");
        }
        else
        {
            fwBegin(reply, sizeof(reply), size, src, srcport);
        }
    }
    else if (0 == strcmp(argv[0], "FW_END"))
    {
        if (fwEnd(reply, sizeof(reply), src, srcport))
        {
            udpReply(dev, reply);
            sleep(500);
            xsh_reset(1, argv);
            return;
        }
    }
    else if (0 == strcmp(argv[0], "FW_ABORT"))
    {
        fwAbort(reply, sizeof(reply), src, srcport);
    }
    else if (0 == strcmp(argv[0], "FW_STATUS"))
    {
        fwStatus(reply, sizeof(reply));
    }
    else if (0 == strcmp(argv[0], "REBOOT"))
    {
        udpAppend(reply, sizeof(reply), "OK rebooting\r\n");
        udpReply(dev, reply);
        sleep(200);
        xsh_reset(1, argv);
        return;
    }
    else if (0 == strcmp(argv[0], "HELP"))
    {
        udpAppend(reply, sizeof(reply),
                  "Commands: PING INFO VERSION UPTIME MEM PS NET "
                  "GPIO SD TFT BUTTON RGB FW_BEGIN FW_DATA FW_END FW_ABORT "
                  "FW_STATUS REBOOT HELP\r\n");
    }
    else
    {
        udpAppend(reply, sizeof(reply), "ERR unknown command '%s'\r\n",
                  argv[0]);
    }

    udpReply(dev, reply);
}

/**
 * UDP command server.  It uses one UDP device in passive mode so that every
 * request carries the sender address, then binds the remote endpoint for the
 * reply and unbinds it again.
 */
thread udpServer(int ethdev, int port)
{
    struct netif *iface;
    uchar rx[UDPSERVER_RX];
    ushort udpdev;
    int n;
    int payload_len;
    struct udpPseudoHdr *pseudo;
    struct udpPkt *pkt;
    struct netaddr src;

    iface = netLookup(ethdev);
    if (NULL == iface)
    {
        fprintf(stderr, "udpserver: no network interface\n");
        udpServerStarted = FALSE;
        return SYSERR;
    }

    udpdev = udpAlloc();
    if ((ushort)SYSERR == udpdev)
    {
        fprintf(stderr, "udpserver: no UDP device available\n");
        udpServerStarted = FALSE;
        return SYSERR;
    }

    if (SYSERR == open(udpdev, &iface->ip, NULL, port, 0))
    {
        fprintf(stderr, "udpserver: cannot open UDP port %d\n", port);
        close(udpdev);
        udpServerStarted = FALSE;
        return SYSERR;
    }

    if (SYSERR == control(udpdev, UDP_CTRL_SETFLAG, UDP_FLAG_PASSIVE, 0))
    {
        fprintf(stderr, "udpserver: cannot set passive mode\n");
        close(udpdev);
        udpServerStarted = FALSE;
        return SYSERR;
    }

    fprintf(stderr, "udpserver: listening on UDP port %d\n", port);

    while (TRUE)
    {
        n = read(udpdev, rx, sizeof(rx));
        if (n < (int)(sizeof(struct udpPseudoHdr) + UDP_HDR_LEN))
        {
            if (SYSERR == n)
            {
                break;
            }
            continue;
        }

        pseudo = (struct udpPseudoHdr *)rx;
        pkt = (struct udpPkt *)(pseudo + 1);

        if (pkt->len < UDP_HDR_LEN)
        {
            continue;
        }

        payload_len = pkt->len - UDP_HDR_LEN;
        if (payload_len < 0
            || n < (int)(sizeof(struct udpPseudoHdr) + UDP_HDR_LEN
                         + payload_len))
        {
            continue;
        }

        /* The UDP layer already converted the header fields to host order. */
        bzero(&src, sizeof(src));
        src.type = NETADDR_IPv4;
        src.len = IPv4_ADDR_LEN;
        memcpy(src.addr, pseudo->srcIp, IPv4_ADDR_LEN);

        if (SYSERR == control(udpdev, UDP_CTRL_BIND, pkt->srcPort,
                              (long)&src))
        {
            continue;
        }

        udpHandle(udpdev, (const char *)pkt->data, payload_len,
                  &src, pkt->srcPort);

        control(udpdev, UDP_CTRL_BIND, 0, (long)NULL);
    }

    close(udpdev);
    udpServerStarted = FALSE;
    return SYSERR;
}

/**
 * @ingroup shell
 *
 * Start the UDP command server.
 *
 * @param nargs number of arguments in args array
 * @param args  args[1] is the UDP port; 5000 by default
 * @return SHELL_OK on success, SHELL_ERROR on failure
 */
shellcmd xsh_udpserver(int nargs, char *args[])
{
    int port = UDPSERVER_PORT;
    tid_typ tid;

    if (nargs > 1)
    {
        port = atoi(args[1]);
    }
    if (port <= 0 || port > 65535)
    {
        printf("Usage: udpserver [port]\n");
        return SHELL_ERROR;
    }

    if (udpServerStarted)
    {
        printf("udpserver: already running\n");
        return SHELL_OK;
    }

    tid = create((void *)udpServer, UDPSERVER_STK, INITPRIO, "UDPSERVER",
                 2, ETH0, port);
    if (SYSERR == tid)
    {
        printf("udpserver: failed to create thread\n");
        return SHELL_ERROR;
    }

    udpServerStarted = TRUE;
    ready(tid, RESCHED_YES);
    return SHELL_OK;
}

#endif /* NETHER */
