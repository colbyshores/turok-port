#!/usr/bin/env python3
"""
turok_rom.py — Turok: Dinosaur Hunter (N64) ROM inspector.

First tool in the Turok port pipeline (see ../CLAUDE.md §6). Does three things,
all verifiable against the raw ROM bytes:

  1. header   — parse + validate the N64 ROM header (byte order, title, code, CRCs).
  2. scan     — find every RNC (Rob Northen ProPack) compressed block and read its
                self-describing header (method, packed/unpacked size, CRCs).
  3. manifest — emit the scan as JSON for downstream tooling.

RNC *decompression* is intentionally NOT reimplemented here from memory: the
authoritative decoder ships in the leak at src/PR/tengine/unpack.c (+ pp.h, huffman.c).
Port THAT to host C/Python for byte-exact output — see TODO at bottom.

Usage:
    python3 turok_rom.py header   [rom]
    python3 turok_rom.py scan     [rom] [--limit N] [--max-offset BYTES]
    python3 turok_rom.py manifest [rom] [-o out.json]

`rom` defaults to ../baserom.us.v12.z64 relative to this script.
"""
import sys, os, json, struct, argparse

DEFAULT_ROM = os.path.join(os.path.dirname(__file__), "..", "baserom.us.v12.z64")

# N64 ROM byte-order magic (first word).
BYTE_ORDERS = {
    0x80371240: "big-endian (.z64, native)",
    0x37804012: "byte-swapped (.v64)",
    0x40123780: "little-endian (.n64)",
}

# RNC ProPack signature: 'RNC' followed by a method byte (1 or 2).
RNC_SIG = b"RNC"


def read_rom(path):
    with open(path, "rb") as f:
        return f.read()


def be32(b, off):
    return struct.unpack_from(">I", b, off)[0]


def be16(b, off):
    return struct.unpack_from(">H", b, off)[0]


def cmd_header(rom, args):
    """Parse the 0x40-byte N64 cartridge header."""
    magic = be32(rom, 0)
    order = BYTE_ORDERS.get(magic, "UNKNOWN")
    title = rom[0x20:0x34].split(b"\x00")[0].decode("ascii", "replace")
    game_code = rom[0x3B:0x3F].decode("ascii", "replace")  # media+id+region
    print(f"file            : {os.path.relpath(args.rom)}")
    print(f"size            : {len(rom):,} bytes ({len(rom)//1024} KiB)")
    print(f"byte order      : 0x{magic:08X}  -> {order}")
    print(f"clock rate      : 0x{be32(rom, 0x04):08X}")
    print(f"entry point     : 0x{be32(rom, 0x08):08X}")
    print(f"release/libultra: 0x{be32(rom, 0x0C):08X}")
    print(f"CRC1            : 0x{be32(rom, 0x10):08X}")
    print(f"CRC2            : 0x{be32(rom, 0x14):08X}")
    print(f"title           : {title!r}")
    print(f"game code       : {game_code!r}   (N=cart, TU=Turok, E=USA)")
    print(f"version          : 0x{rom[0x3F]:02X}")
    if order.startswith("UNKNOWN"):
        print("WARNING: unrecognised byte order — not a clean .z64?", file=sys.stderr)


def iter_rnc_blocks(rom, max_offset=None, limit=None):
    """Yield dicts for each RNC header found in the ROM.

    RNC header (big-endian, 18 bytes):
        0x00  'RNC'              3 bytes
        0x03  method            1 byte  (1 or 2)
        0x04  unpacked length   4 bytes
        0x08  packed length     4 bytes
        0x0C  unpacked CRC      2 bytes
        0x0E  packed CRC        2 bytes
        0x10  leeway            1 byte
        0x11  pack chunks       1 byte
    """
    end = len(rom) if max_offset is None else min(len(rom), max_offset)
    start = 0
    count = 0
    while True:
        i = rom.find(RNC_SIG, start, end)
        if i < 0:
            break
        method = rom[i + 3] if i + 3 < len(rom) else 0xFF
        start = i + 1  # allow overlapping/adjacent matches
        if method not in (1, 2):
            continue                      # 'RNC' that isn't a real ProPack header
        if i + 18 > len(rom):
            continue
        unpacked = be32(rom, i + 4)
        packed = be32(rom, i + 8)
        # Sanity: sizes must be plausible and packed block must fit in the ROM.
        if packed == 0 or packed > len(rom) or i + 18 + packed > len(rom):
            continue
        if unpacked == 0 or unpacked > 0x400000:   # > 4 MiB unpacked is implausible here
            continue
        yield {
            "offset": i,
            "method": method,
            "unpacked_size": unpacked,
            "packed_size": packed,
            "unpacked_crc": be16(rom, i + 12),
            "packed_crc": be16(rom, i + 14),
            "leeway": rom[i + 16],
            "pack_chunks": rom[i + 17],
            "block_end": i + 18 + packed,
        }
        count += 1
        if limit and count >= limit:
            break


def cmd_scan(rom, args):
    blocks = list(iter_rnc_blocks(rom, args.max_offset, args.limit))
    print(f"found {len(blocks)} plausible RNC block(s)"
          + (f" (limit {args.limit})" if args.limit else "") + ":\n")
    print(f"{'#':>4}  {'offset':>10}  {'m':>1}  {'packed':>9}  {'unpacked':>9}  {'ratio':>6}")
    print("-" * 56)
    tot_p = tot_u = 0
    for n, b in enumerate(blocks):
        ratio = b["unpacked_size"] / b["packed_size"] if b["packed_size"] else 0
        tot_p += b["packed_size"]; tot_u += b["unpacked_size"]
        print(f"{n:>4}  0x{b['offset']:08X}  {b['method']:>1}  "
              f"{b['packed_size']:>9}  {b['unpacked_size']:>9}  {ratio:>5.2f}x")
    if blocks:
        print("-" * 56)
        print(f"{'tot':>4}  {'':>10}  {'':>1}  {tot_p:>9}  {tot_u:>9}  "
              f"{(tot_u/tot_p if tot_p else 0):>5.2f}x")


def cmd_manifest(rom, args):
    blocks = list(iter_rnc_blocks(rom, args.max_offset, args.limit))
    out = {
        "rom": os.path.basename(args.rom),
        "size": len(rom),
        "title": rom[0x20:0x34].split(b"\x00")[0].decode("ascii", "replace"),
        "game_code": rom[0x3B:0x3F].decode("ascii", "replace"),
        "rnc_block_count": len(blocks),
        "rnc_blocks": blocks,
    }
    text = json.dumps(out, indent=2)
    if args.out:
        with open(args.out, "w") as f:
            f.write(text)
        print(f"wrote {args.out}  ({len(blocks)} RNC blocks)")
    else:
        print(text)


def main(argv):
    p = argparse.ArgumentParser(description="Turok N64 ROM inspector")
    sub = p.add_subparsers(dest="cmd", required=True)
    for name in ("header", "scan", "manifest"):
        sp = sub.add_parser(name)
        sp.add_argument("rom", nargs="?", default=DEFAULT_ROM)
        sp.add_argument("--max-offset", type=lambda x: int(x, 0), default=None,
                        help="stop scanning after this byte offset")
        sp.add_argument("--limit", type=int, default=None, help="cap number of blocks")
        if name == "manifest":
            sp.add_argument("-o", "--out", default=None, help="write JSON here")
    args = p.parse_args(argv)
    args.rom = os.path.abspath(args.rom)
    if not os.path.exists(args.rom):
        p.error(f"ROM not found: {args.rom}")
    rom = read_rom(args.rom)
    {"header": cmd_header, "scan": cmd_scan, "manifest": cmd_manifest}[args.cmd](rom, args)


if __name__ == "__main__":
    main(sys.argv[1:])

# TODO(next): port src/PR/tengine/unpack.c (RNC method 1 & 2) + huffman.c to a host
# decoder `rnc_unpack`, verify each block's unpacked_crc, then build the asset
# extractor keyed off the cart directory structures in cart.c / romstruc.c (CLAUDE.md §6).
