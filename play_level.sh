#!/bin/bash
# Boot Turok directly into the FIRST LEVEL in a visible window (SDL2 hardware GL),
# for visual debugging on your desktop. Close the window (or Ctrl-C) to quit.
#
# Usage:
#   ./play_level.sh                 # DISPLAY :1; RETAIL v1.2 assets if baserom.us.v12.z64 is present
#   DISPLAY=:0 ./play_level.sh       # if your desktop is on another display
#   WARP=1000 ./play_level.sh        # boot a different level (0,1000,...,8000)
#   WARP=menu ./play_level.sh        # boot the NORMAL front-end: legal screen -> Acclaim/Iguana logos ->
#                                     # title menu -> attract demo (skips the dev level-warp entirely)
#   ROM=/path/to/other.z64 ./play_level.sh   # use a specific ROM
#   ROM=none ./play_level.sh         # force the v49 dev assets (cartdata.dat) instead of retail
#   DEBUG=1 ./play_level.sh          # crash-diagnosis build: prints fault addr + backtrace on segfault
#
# Notes:
#  - Path B streams assets from the retail ROM (offset 0x1F00), giving the FINISHED v1.2 geometry/art
#    — including the level-1 walkway the v49 leak is MISSING. It's now the DEFAULT when the ROM is
#    present in this dir; use ROM=none to force the v49 cartdata.dat (placeholder art, no walkway).
#  - DEBUG=1 builds the -O0 + SIGSEGV-handler binary: on a crash it prints the fault address,
#    last memcpy, and a short backtrace — send that output for diagnosis.
#  - Input is wired (WASD/arrows + gamepad). Levels 2-8 (WARP 2000+) load slowly;
#    WARP=0 (default) is fast. WARP=menu boots the title/attract flow instead of a level.
set -e
cd "$(dirname "$0")"

: "${DISPLAY:=:1}"
: "${WARP:=0}"
# Render cap. FPS=0 = uncapped: render runs at the monitor's refresh (v-sync, e.g. 144Hz) so the
# interpolation has extra frames to fill between logic ticks -> liquid-smooth camera. FPS=N caps render
# at N (e.g. FPS=60 to force 60).
: "${FPS:=0}"
# Game LOGIC tick rate. TICK=30 = Turok's native step rate = the CORRECT game speed (the per-tick step is
# sized for 30fps, so TICK=60 runs everything 2x too fast). The render runs faster (uncapped/v-sync above)
# and INTERPOLATES between the 30Hz logic ticks -> buttery 60fps motion at the correct speed. TICK=0 = logic
# every render frame (legacy, too fast).
: "${TICK:=30}"
if [ -n "${DEBUG:-}" ] && [ "$DEBUG" != "0" ]; then BUILD_MODE=debug; OUT=/tmp/turok_sdl_dbg; else BUILD_MODE=release; OUT=/tmp/turok_sdl; fi

# Default to the retail ROM (Path B) when it's present: it has the FINISHED level geometry — e.g. the
# level-1 walkway over the water at the fire-pit start — that the v49 leak's cartdata.dat is MISSING
# (walk forward on v49 assets and you drop into a blue void). Pass ROM=<path> to use a specific ROM, or
# ROM=none to force the v49 dev assets.
if [ -z "${ROM:-}" ] && [ -f "$PWD/baserom.us.v12.z64" ]; then ROM="baserom.us.v12.z64"; fi
if [ "${ROM:-}" = "none" ]; then ROM=""; fi

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
# WARP=menu (or "boot"/"none") skips the dev level-warp entirely: leaving TUROK_WARP UNSET makes
# tengine.c take its normal MODE_RESETGAME path -> LEGALSCREEN_WARP_ID -> the real legal screen /
# Acclaim+Iguana intro logos / title menu / attract demo (see frontend.c CLegalScreen__Update — the
# old port-only freeze on this screen was removed once the 30Hz logic-tick decouple landed).
# -u (env's "unset" option, must precede any NAME=VALUE pairs) explicitly unsets TUROK_WARP even
# if it's already exported in the calling shell (e.g. left over from an earlier session) — belt
# and suspenders, since the game only takes the normal boot path when TUROK_WARP is truly absent.
ENV_UNSET_ARG=()
WARP_ARG=()
case "$WARP" in
    menu|boot|none)
        ENV_UNSET_ARG=(-u TUROK_WARP)
        echo "[play_level] launching to the TITLE/ATTRACT front-end (DISPLAY=$DISPLAY) — close the window to quit." ;;
    *) WARP_ARG=(TUROK_WARP="$WARP"); echo "[play_level] launching first level (WARP=$WARP, DISPLAY=$DISPLAY) — close the window to quit." ;;
esac

# Force SDL2's x11 video driver: on a Wayland session SDL2 picks the Wayland driver even with
# DISPLAY set, and its GL window creation segfaults. We launch against XWayland (DISPLAY), so x11.
# TUROK_VTXBAD=1: corruption detectors. They print ONLY when a matrix/vertex actually goes
# NaN/huge (i.e. the camera/HUD-corruption moment) — silent otherwise. If the camera glitches,
# the [CANARY]/[CAMBAD]/[VTXBAD] lines name exactly what went bad. Set HUD=0 to hide the HUD.
exec env "${ENV_UNSET_ARG[@]}" DISPLAY="$DISPLAY" \
    SDL_VIDEODRIVER="${SDL_VIDEODRIVER:-x11}" \
    TUROK_CARTDATA="$PWD/src/PR/cartdata.dat" \
    "${ROM_ARG[@]}" \
    "${WARP_ARG[@]}" \
    TUROK_FPS="$FPS" \
    TUROK_TICK_FPS="$TICK" \
    TUROK_HUD="${HUD:-1}" \
    TUROK_VTXBAD=1 \
    TUROK_FXLEAK=1 \
    TUROK_WATCHDOG="${WATCHDOG:-1}" \
    TUROK_CRASHLOG="${TUROK_CRASHLOG:-$PWD/turok_crash.log}" \
    "$OUT/turok"
# Crash capture is ALWAYS on in the release build: a fatal fault (SIGSEGV/ABRT/BUS/FPE/ILL) writes a
# full report (signal, fault addr, faulting thread [main vs audio], g_frame, phase, symbolized
# backtrace + raw pcs) to $TUROK_CRASHLOG (default ./turok_crash.log). Send that file back after a crash.
# Symbolize the raw pcs with:  addr2line -f -e "$OUT/turok" 0x<pc> 0x<pc> ...
