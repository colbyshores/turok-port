#!/bin/bash
# Boot Turok directly into the FIRST LEVEL in a visible window (SDL2 hardware GL),
# for visual debugging on your desktop. Close the window (or Ctrl-C) to quit.
#
# Usage:
#   ./play_level.sh                 # uses DISPLAY :1
#   DISPLAY=:0 ./play_level.sh       # if your desktop is on another display
#   WARP=1000 ./play_level.sh        # boot a different level (0,1000,...,8000)
#
# Notes:
#  - No input is wired yet (controller is stubbed), so the camera is static — you
#    see the level from the spawn point. Animated objects (player/enemies) draw but
#    may be culled/off-screen at the spawn. Resize the window freely.
#  - Levels 2-8 (WARP 2000+) are large and load slowly; WARP=0 (default) is fast.
set -e
cd "$(dirname "$0")"

: "${DISPLAY:=:1}"
: "${WARP:=0}"
: "${FPS:=60}"
OUT=/tmp/turok_sdl

echo "[play_level] building SDL2 windowed release -> $OUT ..."
if ! GFX=sdl2 TUROK_OUT="$OUT" bash tools/build_port.sh release >/tmp/turok_sdl_build.log 2>&1; then
    echo "[play_level] BUILD FAILED:"; tail -12 /tmp/turok_sdl_build.log; exit 1
fi
echo "[play_level] launching first level (WARP=$WARP, DISPLAY=$DISPLAY) — close the window to quit."

exec env DISPLAY="$DISPLAY" \
    TUROK_CARTDATA="$PWD/src/PR/cartdata.dat" \
    TUROK_WARP="$WARP" \
    TUROK_FPS="$FPS" \
    "$OUT/turok"
