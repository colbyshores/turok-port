#!/bin/bash
# one-shot clean runner: kills stragglers, cds to repo (so cartdata.dat resolves),
# runs a single paced SDL2 instance, captures. args: [maxframes] [capframe] [pngpath]
cd /mnt/nas/Development/turok || exit 2
pkill -9 -f tb_sdl2p 2>/dev/null
sleep 1
export DISPLAY=:1
export TUROK_MAX_FRAMES="${1:-4000}"
export TUROK_CAPTURE_FRAME="${2:-30}"
export TUROK_CAPTURE_PATH="${3:-/tmp/cap_paced.png}"
export TUROK_GFX_DUMP=1
rm -f "$TUROK_CAPTURE_PATH"
exec timeout 70 /tmp/tb_sdl2p/turok
