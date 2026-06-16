#!/bin/bash
# Boot Turok directly into the FIRST LEVEL in a visible window (SDL2 hardware GL),
# for visual debugging on your desktop. Close the window (or Ctrl-C) to quit.
#
# Usage:
#   ./play_level.sh                 # uses DISPLAY :1, v49 dev assets (cartdata.dat)
#   DISPLAY=:0 ./play_level.sh       # if your desktop is on another display
#   WARP=1000 ./play_level.sh        # boot a different level (0,1000,...,8000)
#   ROM=baserom.us.v12.z64 ./play_level.sh   # RETAIL v1.2 assets (Path B) — finished art
#   DEBUG=1 ROM=… ./play_level.sh    # crash-diagnosis build: prints fault addr + backtrace on segfault
#
# Notes:
#  - ROM=<retail .z64> selects Path B: assets stream from the retail ROM (offset 0x1F00),
#    giving the FINISHED v1.2 models/textures instead of the v49 leak's placeholder art.
#    Leave ROM unset to use the in-tree v49 cartdata.dat.
#  - DEBUG=1 builds the -O0 + SIGSEGV-handler binary: on a crash it prints the fault address,
#    last memcpy, and a short backtrace — send that output for diagnosis.
#  - Input is wired (WASD/arrows + gamepad). Levels 2-8 (WARP 2000+) load slowly;
#    WARP=0 (default) is fast.
set -e
cd "$(dirname "$0")"

: "${DISPLAY:=:1}"
: "${WARP:=0}"
: "${FPS:=60}"
if [ -n "${DEBUG:-}" ] && [ "$DEBUG" != "0" ]; then BUILD_MODE=debug; OUT=/tmp/turok_sdl_dbg; else BUILD_MODE=release; OUT=/tmp/turok_sdl; fi

# Path B: if ROM is set, resolve to an absolute path and stream retail v1.2 assets.
ROM_ARG=()
if [ -n "${ROM:-}" ]; then
    case "$ROM" in /*) ROM_ABS="$ROM";; *) ROM_ABS="$PWD/$ROM";; esac
    if [ ! -f "$ROM_ABS" ]; then echo "[play_level] ROM not found: $ROM_ABS"; exit 1; fi
    ROM_ARG=(TUROK_ROM="$ROM_ABS")
    echo "[play_level] Path B: streaming RETAIL v1.2 assets from $ROM_ABS"
fi

echo "[play_level] building SDL2 windowed $BUILD_MODE -> $OUT ..."
if ! GFX=sdl2 TUROK_OUT="$OUT" bash tools/build_port.sh "$BUILD_MODE" >/tmp/turok_sdl_build.log 2>&1; then
    echo "[play_level] BUILD FAILED:"; tail -12 /tmp/turok_sdl_build.log; exit 1
fi
echo "[play_level] launching first level (WARP=$WARP, DISPLAY=$DISPLAY) — close the window to quit."

# Force SDL2's x11 video driver: on a Wayland session SDL2 picks the Wayland driver even with
# DISPLAY set, and its GL window creation segfaults. We launch against XWayland (DISPLAY), so x11.
# TUROK_VTXBAD=1: corruption detectors. They print ONLY when a matrix/vertex actually goes
# NaN/huge (i.e. the camera/HUD-corruption moment) — silent otherwise. If the camera glitches,
# the [CANARY]/[CAMBAD]/[VTXBAD] lines name exactly what went bad. Set HUD=0 to hide the HUD.
exec env DISPLAY="$DISPLAY" \
    SDL_VIDEODRIVER="${SDL_VIDEODRIVER:-x11}" \
    TUROK_CARTDATA="$PWD/src/PR/cartdata.dat" \
    "${ROM_ARG[@]}" \
    TUROK_WARP="$WARP" \
    TUROK_FPS="$FPS" \
    TUROK_HUD="${HUD:-1}" \
    TUROK_VTXBAD=1 \
    TUROK_FXLEAK=1 \
    "$OUT/turok"
