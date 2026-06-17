#!/bin/bash
# build_port.sh — reproducible PC host build for the Turok port.
#
# WHY THIS EXISTS: the repo lives on an NFS mount whose timestamps are unreliable, which
# defeats `make` (it skips rebuilds of changed files, linking stale objects — e.g. a cart.h
# ORDERBYTES change silently not taking). This script compiles EVERY TU fresh to a LOCAL
# (non-NFS) object dir and links there, sidestepping the staleness entirely.
#
#   tools/build_port.sh            # build -> $OUT/turok
#   tools/build_port.sh asan       # AddressSanitizer build (note: 32-bit ASan may not init)
#   tools/build_port.sh debug      # -O0 + SIGSEGV/memcpy guard wrapped in
#   TUROK_MAX_FRAMES=8 $OUT/turok   # run headless, bounded
#
# Debugging note: gdb can't unwind through the hand-written asm memcpy, and 32-bit ASan
# aborts in its own init here. The reliable tool is port-side instrumentation: romPiRead
# logs every DMA (dst/devAddr/nbytes) and bounds-checks vs cartdata; the optional memcpy
# wrapper (--wrap) installs a SIGSEGV handler that prints the fault addr + last copy +
# backtrace. Map addresses with: addr2line -f -e $OUT/turok 0x<addr>.

set -e
cd "$(dirname "$0")/.."
ROOT="$(pwd)"
OUT="${TUROK_OUT:-/tmp/turok_build}"          # LOCAL dir — never on NFS
OBJ="$OUT/obj"
rm -rf "$OBJ"          # clean every build — we compile all TUs fresh anyway (NFS staleness),
mkdir -p "$OBJ"        # and this avoids a stale debug zz_guard.o leaking into a release link.

MODE="${1:-release}"
GFX="${GFX:-sdl2}"         # sdl2 = hidden-window HARDWARE GL via X display (fast, DEFAULT, like Perfect Dark)
                           # osmesa = software GL (slow, headless, like Banjo)
                           # egl = headless HARDWARE GL via GBM render node (no X server; fast, reliable)
case "$GFX" in
  osmesa) GFX_DEF="-DGFX_USE_OSMESA"; GFX_SRC="gfx_osmesa"; GFX_LIB="-l:libOSMesa.so.8 -l:libGL.so.1" ;;
  egl)    GFX_DEF="-DGFX_USE_EGL";    GFX_SRC="gfx_egl";    GFX_LIB="-l:libEGL.so.1 -l:libGL.so.1 -l:libgbm.so.1" ;;
  sdl2)   GFX_DEF="-DGFX_USE_SDL2 -I/usr/include/SDL2 -I/usr/include/x86_64-linux-gnu -D_REENTRANT";
          GFX_SRC="gfx_sdl2 gfx_glcapture";
          GFX_LIB="-L/usr/lib/i386-linux-gnu -l:libSDL2-2.0.so.0 -l:libGL.so.1" ;;
  *) echo "unknown GFX=$GFX (use sdl2|osmesa|egl)"; exit 1 ;;
esac
OPT="-O1"; SAN=""; WRAP=""; EXTRA_OBJ=""
case "$MODE" in
  debug) OPT="-O0 -fno-omit-frame-pointer" ;;
  asan)  OPT="-O0 -fno-omit-frame-pointer"; SAN="-fsanitize=address" ;;
esac

GAME_D="-DPLATFORM_PORT=1 -D_LANGUAGE_C=1 -D_MIPS_SZLONG=32 -DF3DEX_GBI -DN_MICRO -D_FINALROM -DNDEBUG -DSHIP_IT"
GAME_I="-Isrc/PR/tengine -Ilib/ultralib/include -Ilib/ultralib/include/PR -Iport/include"
PORT_I="-Ilib/ultralib/include -Ilib/ultralib/include/PRinternal -Iport/include"
GU_I="$PORT_I -Ilib/ultralib/include/PR"
WARN="-w -fcommon -fno-builtin -fno-strict-aliasing -fno-stack-protector -fno-pie -Wno-implicit-function-declaration -Wno-int-conversion"
FI="-include port/include/turok_port.h"
CC="gcc -m32 $OPT -g $SAN"

echo "[build] mode=$MODE out=$OUT"

# Game TUs (coll.c deferred — stale vs CCollisionInfo2, off the M0->M2 path).
ng=0
for f in src/PR/tengine/*.c; do
  b=$(basename "$f" .c); [ "$b" = coll ] && continue
  $CC $GAME_D $GAME_I $FI $WARN -c "$f" -o "$OBJ/$b.o"; ng=$((ng+1))
done
echo "[build] game TUs: $ng"

# Port layer (turok_gfx.c needs the fast3d headers too).
for f in port/src/*.c; do
  b=$(basename "$f" .c); $CC -DPLATFORM_PORT=1 -D_LANGUAGE_C=1 -DF3DEX_GBI -DNDEBUG $GFX_DEF $PORT_I -Iport/fast3d -Iport/fast3d/shaders $WARN -c "$f" -o "$OBJ/port_$b.o"
done
# libultra gu (matrix utils).
for f in mtxutil ortho perspective lookat lookatref; do
  $CC -DPLATFORM_PORT=1 -D_LANGUAGE_C=1 -DF3DEX_GBI -DNDEBUG $GU_I $WARN -c lib/ultralib/src/gu/$f.c -o "$OBJ/gu_$f.o"
done
# Fast3D layer (C++20) — F3DEX interpreter + OpenGL backend + OSMesa headless WM.
F3D_D="-DPLATFORM_PORT=1 $GFX_DEF -D_LANGUAGE_C=1 -DF3DEX_GBI -DN_MICRO -DNDEBUG"
F3D_I="-Ilib/ultralib/include -Iport/fast3d -Iport/fast3d/shaders -Iport/include"
for f in gfx_pc gfx_cc gfx_opengl $GFX_SRC; do
  g++ -m32 -std=gnu++20 $OPT -g $SAN $F3D_D $F3D_I -w -fno-pie -c port/fast3d/$f.cpp -o "$OBJ/f3d_$f.o"
done
gcc -m32 $OPT -g $SAN -Iport/fast3d -w -fno-pie -c port/fast3d/glad/glad.c -o "$OBJ/f3d_glad.o"

if [ "$MODE" = debug ] || [ "$MODE" = asan ]; then
  # optional memcpy/bcopy guard + SIGSEGV handler (see port/tools/memcpy_guard.c)
  if [ -f port/tools/memcpy_guard.c ]; then
    gcc -m32 -O0 -g -fno-pie -c port/tools/memcpy_guard.c -o "$OBJ/zz_guard.o"  # picked up by the glob
    WRAP="-Wl,--wrap=memcpy,--wrap=memmove,--wrap=bcopy"
  fi
fi

# Link with g++ (C++ runtime for Fast3D) + OSMesa/GL for headless rendering.
# The 32-bit libs ship only as versioned .so (no -dev symlinks), so link them by exact name.
GLLIBS="-L/usr/lib/i386-linux-gnu $GFX_LIB"
g++ -m32 $SAN -no-pie -fno-pie -rdynamic $WRAP "$OBJ"/*.o $GLLIBS -lm -lpthread -o "$OUT/turok"
echo "[build] linked -> $OUT/turok"
