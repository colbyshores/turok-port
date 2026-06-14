#!/bin/bash
# build + run in ONE invocation (the /tmp binary vanishes between separate commands here).
# args: [maxframes] [capframe] [pngpath].  env: GFX (default sdl2), TUROK_FPS (default 60).
cd /mnt/nas/Development/turok || exit 2
pkill -9 -f '/tmp/tbx/turok' 2>/dev/null
export GFX="${GFX:-sdl2}"
export TUROK_OUT=/tmp/tbx
rm -rf /tmp/tbx
echo "=== building (GFX=$GFX) ==="
bash tools/build_port.sh release > /tmp/tbx_build.log 2>&1
if [ ! -x /tmp/tbx/turok ]; then echo "BUILD FAILED:"; tail -6 /tmp/tbx_build.log; exit 1; fi
echo "=== built ($(stat -c%s /tmp/tbx/turok) bytes), running ==="
export DISPLAY=:1
export TUROK_FPS="${TUROK_FPS:-60}"
export TUROK_MAX_FRAMES="${1:-600}"
export TUROK_CAPTURE_FRAME="${2:-5}"
export TUROK_CAPTURE_PATH="${3:-/tmp/cap5.png}"
export TUROK_GFX_DUMP="${TUROK_GFX_DUMP:-0}"
exec timeout "${TUROK_TIMEOUT:-90}" /tmp/tbx/turok
