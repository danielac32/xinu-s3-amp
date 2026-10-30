#!/usr/bin/env python3
"""Package a linked Xinu ELF into the AMP payload format.

The ESP-IDF loader maps the payload partition at XIP_BASE (0x42100000).
The first 0x1000 bytes are reserved for this header; executable sections
must therefore be linked at 0x42101000 or above.

Image format (little endian):

    struct xinu_img_header {
        uint32 magic;       /* 'XIMG' */
        uint32 version;
        uint32 header_size;
        uint32 entry;       /* virtual address */
        uint32 nseg;
        uint32 xip_base;    /* 0x42100000 */
        uint32 image_size;
        uint32 crc;         /* reserved, 0 */
    };

    struct xinu_img_seg {
        uint32 vaddr;
        uint32 paddr;
        uint32 size;
        uint32 offset;
        uint32 flags;
    };

Flags:
    XIP    = 1   bytes already in flash at xip_base + offset
    COPY   = 2   loader copies bytes from image offset to vaddr
    ZERO   = 4   loader zeroes size bytes at vaddr
    EXEC   = 8
    WRITE  = 16
    VECTOR = 32  vector table linked in the XIP window
"""

from __future__ import annotations

import argparse
import struct
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path

MAGIC = 0x58494D47  # "XIMG"
VERSION = 1
XIP_BASE = 0x42100000
HEADER_OFFSET = 0x1000

F_XIP = 1
F_COPY = 2
F_ZERO = 4
F_EXEC = 8
F_WRITE = 16
F_VECTOR = 32

SHF_ALLOC = 0x2
SHT_NOBITS = 8


@dataclass
class Section:
    name: str
    addr: int
    offset: int
    size: int
    flags: int
    type: int


def read_elf_sections(path: Path) -> list[Section]:
    data = path.read_bytes()
    if data[:4] != b"\x7fELF" or data[4] != 1 or data[5] != 1:
        raise SystemExit("not a 32-bit little-endian ELF")

    e_shoff = struct.unpack_from("<I", data, 0x20)[0]
    e_shentsize = struct.unpack_from("<H", data, 0x2E)[0]
    e_shnum = struct.unpack_from("<H", data, 0x30)[0]
    e_shstrndx = struct.unpack_from("<H", data, 0x32)[0]

    shstr_hdr = e_shoff + e_shstrndx * e_shentsize
    shstr_off = struct.unpack_from("<I", data, shstr_hdr + 0x10)[0]
    shstr_size = struct.unpack_from("<I", data, shstr_hdr + 0x14)[0]
    shstr = data[shstr_off : shstr_off + shstr_size]

    out: list[Section] = []
    for i in range(e_shnum):
        hdr = e_shoff + i * e_shentsize
        name_off, sh_type, sh_flags, sh_addr, sh_offset, sh_size = struct.unpack_from(
            "<IIIIII", data, hdr
        )
        name_end = shstr.find(b"\x00", name_off)
        name = shstr[name_off:name_end].decode("ascii", "replace")
        if sh_size == 0 or not (sh_flags & SHF_ALLOC):
            continue
        if name.startswith(".debug") or name.startswith(".comment"):
            continue
        out.append(Section(name, sh_addr, sh_offset, sh_size, sh_flags, sh_type))
    return out


def symbol_addr(elf: Path, symbol: str) -> int:
    nm = "xtensa-esp32s3-elf-nm"
    try:
        text = subprocess.check_output([nm, "-n", str(elf)], text=True)
    except FileNotFoundError:
        text = subprocess.check_output(["nm", "-n", str(elf)], text=True)
    for line in text.splitlines():
        parts = line.split()
        if len(parts) >= 3 and parts[2] == symbol:
            return int(parts[0], 16)
    raise SystemExit(f"symbol {symbol!r} not found in {elf}")


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("elf", type=Path)
    ap.add_argument("out", type=Path)
    ap.add_argument("--xip-base", type=lambda s: int(s, 0), default=XIP_BASE)
    args = ap.parse_args()

    elf = args.elf
    raw = elf.read_bytes()
    sections = read_elf_sections(elf)
    entry = symbol_addr(elf, "_start")

    # First pass: place XIP sections at their natural MMU offset; place all
    # other segments after the XIP area.
    xip_end = HEADER_OFFSET
    for s in sections:
        if s.addr >= args.xip_base:
            xip_end = max(xip_end, s.addr - args.xip_base + s.size)

    data_off = (xip_end + 15) & ~15
    placed: list[tuple[Section, int, int]] = []
    for s in sections:
        if s.type == SHT_NOBITS:
            placed.append((s, 0, F_ZERO | F_WRITE))
            continue
        if s.name == ".vector":
            if s.addr < args.xip_base:
                raise SystemExit(".vector must be linked in the XIP window")
            off = s.addr - args.xip_base
            placed.append((s, off, F_XIP | F_EXEC | F_VECTOR))
            continue
        if s.addr >= args.xip_base:
            off = s.addr - args.xip_base
            placed.append((s, off, F_XIP | F_EXEC))
        else:
            off = data_off
            data_off = (data_off + s.size + 15) & ~15
            flags = F_COPY
            if s.flags & 0x4:  # SHF_EXECINSTR
                flags |= F_EXEC
            if s.flags & 0x1:  # SHF_WRITE
                flags |= F_WRITE
            placed.append((s, off, flags))

    image_size = max(HEADER_OFFSET, (data_off + 15) & ~15)
    image = bytearray(image_size)

    nseg = len(placed)
    header_size = 32 + nseg * 20
    if header_size > HEADER_OFFSET:
        raise SystemExit(f"too many segments ({nseg}); header {header_size:#x} > {HEADER_OFFSET:#x}")

    struct.pack_into(
        "<IIIIIIII",
        image,
        0,
        MAGIC,
        VERSION,
        header_size,
        entry,
        nseg,
        args.xip_base,
        image_size,
        0,
    )

    for i, (s, off, flags) in enumerate(placed):
        rec = 32 + i * 20
        paddr = args.xip_base + off if (flags & F_XIP) else 0
        struct.pack_into("<IIIII", image, rec, s.addr, paddr, s.size, off, flags)
        if flags & F_ZERO:
            continue
        chunk = raw[s.offset : s.offset + s.size]
        if len(chunk) != s.size:
            raise SystemExit(f"short section {s.name}")
        image[off : off + s.size] = chunk

    args.out.write_bytes(image)
    print(
        f"xinu.img: {len(image)} bytes, {nseg} segments, entry={entry:#010x}, "
        f"xip_base={args.xip_base:#010x}"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
