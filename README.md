# Turok: Dinosaur Hunter — PC / 3DS Port

A native port of *Turok: Dinosaur Hunter* (N64, 1997) to PC (ground-truth) and the
Nintendo 3DS, built from the **original Iguana/Acclaim engine source** (`src/PR/tengine`)
rather than a decompilation. The original C compiles off-IRIX against a shimmed `libultra`;
F3DEX-NoN display lists are interpreted on the host via Fast3D (`port/fast3d`) into OpenGL
(PC) / Citro3D (3DS); assets stream from the retail ROM with byte-swapping at the seams.

See **[CLAUDE.md](CLAUDE.md)** for the full engineering rosetta-stone (architecture,
milestones, every seam and port edit). Source-only; **no game data is committed.**

## Status

- **M0 — compiles**, **M1 — boots headless**, **M2 — renders the legal screen**,
  **M5 — the first level loads, parses, and renders as walkable 3D geometry.**
  Animated objects (enemies/weapon/doors) draw without crashing after the
  collision/instance/grid/geometry/animation big-endian sweeps.

## You must supply your own game data (not in this repo)

This repository is **source only**. Copy your own legally-obtained files in before building:

- `baserom.us.v12.z64` — retail US v1.2 ROM (repo root)
- `src/PR/cartdata.dat` — cartridge asset blob

## Build & run (PC)

```bash
# Headless render to PNG (EGL/GBM, no X server):
GFX=egl TUROK_OUT=/tmp/tbe bash tools/build_port.sh release
unset DISPLAY
TUROK_CARTDATA=$PWD/src/PR/cartdata.dat TUROK_WARP=0 TUROK_FPS=0 \
  TUROK_MAX_FRAMES=30 TUROK_CAPTURE_FRAME=20 TUROK_CAPTURE_PATH=level.png /tmp/tbe/turok

# Windowed, first level, on your desktop (SDL2):
./play_level.sh        # DISPLAY=:0 ./play_level.sh  if your desktop is elsewhere
```

Useful env vars: `TUROK_WARP=<0|1000|…|8000>` (level entry point), `TUROK_FPS` (pacing),
`TUROK_GFX_DUMP=1` (per-frame triangle counts), `TUROK_ANIMOBJ=0` (skip animated objects),
`TUROK_HUD=1` (attempt the HUD). devkitPro is required for the 3DS target.

## Layout

| Path | What |
|------|------|
| `src/PR/tengine/` | Original game engine C (compiled as-is; port edits gated behind `PLATFORM_PORT`) |
| `port/` | The only platform-specific layer — libultra shims, ROM/DMA seam, Fast3D, GL/SDL/EGL backends |
| `lib/ultralib/` | Vendored portable libultra headers/source |
| `tools/` | `build_port.sh` (the build), `turok_rom.py` (ROM inspector) |
| `docs/` | Distilled notes on the sibling ports (Banjo-Kazooie, Perfect Dark, Forsaken) |

Lineage: follows the Banjo-Kazooie / Perfect Dark / Forsaken port patterns so code is shared.
