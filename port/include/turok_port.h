/* turok_port.h — port-wide compatibility shim, force-included on the host build.
 *
 * This header is injected into every tengine translation unit via `-include` from
 * Makefile.port (PLATFORM_PORT builds only). It restores the few N64/IDO-era
 * assumptions the original source was written against, so the game C compiles on a
 * modern host toolchain WITHOUT editing the game files themselves. Keep it tiny and
 * codegen-neutral; anything bigger belongs in a proper port/ source file.
 *
 * See ../../CLAUDE.md for the porting strategy and milestone roadmap.
 */
#ifndef _TUROK_PORT_H
#define _TUROK_PORT_H

#ifdef PLATFORM_PORT

/* Unaligned-access accessors (turok_rd_u16/s16/u32/s32/f32, turok_memcpy_unaligned) for the
 * ARM11 port — route every float/struct read of a byte-parsed asset buffer through an integer
 * load into an aligned local. Codegen-neutral on x86, ARM-safe (verified). See turok_align.h. */
#include "turok_align.h"

/* Symbol collision with libc: the game defines its own 5-arg qsort(base,num,wid,comp,SWAP),
 * which interposes (as an executable global) on libc's 4-arg qsort that OSMesa/Mesa call
 * internally — Mesa then invokes the missing `swap` arg as a function pointer (garbage/NULL)
 * and crashes. Rename the game's version so libc keeps its own qsort. (This force-include is
 * applied to game TUs only, so it renames the definition AND every game call site.) */
#define qsort turok_qsort

/* NULL-as-integer note:
 * The IDO N64 toolchain expanded NULL to integer 0, which tengine relied on in a few
 * flag-field initialisers (`NULL | SOME_FLAG` in the boss tables) and switch labels
 * (`case NULL:`). Modern <stddef.h> makes NULL `((void*)0)`, illegal there. A force-
 * include can't fix it (libultra's <stddef.h> re-#defines NULL *after* this header),
 * so those ~10 sites were edited in place to literal `0` (trex.c, campaign.c,
 * loadsave.c, pause.c) — see CLAUDE.md "Port edits to game source". This header is
 * kept as the home for genuine codegen-neutral host shims as the port grows.
 */

/* Single-precision math prototype: the game's math headers declare the DOUBLE
 * variants (sin/cos/sqrt…) but not the float ones, so a bare `fmodf()` is implicitly
 * declared as `int fmodf()` — wrong ABI, garbage result (it compiles silently under
 * -Wno-implicit-function-declaration, then BLANKS the render because the wrapped angle
 * comes back as garbage). The port uses fmodf for O(1) angle wrapping in
 * graphu64.c (NormalizeRotation), boss.c (AngleDiffFromZero) and tmove.c; declare it
 * with the correct signature so it's called/returned as float. (Matches <math.h>, so a
 * TU that also includes <math.h> sees a compatible redeclaration.) */
extern float fmodf(float, float);

/* O(1) angle wrap to [-PI, PI) — replaces the spin-prone `while (a > PI) a -= 2PI;` loops in the port's
 * render-interpolation paths (tengine.c/romstruc.c). A garbage/uninitialised prev- or cur-angle makes the
 * delta huge and a raw while-loop spins ~1e17 times = a freeze with no crash dump. fmodf is O(1) for any
 * magnitude; NaN -> 0. */
static __inline__ float turok_wrap_pi(float a) {
    a = fmodf(a, 6.28318531f);
    if (a >  3.14159265f) a -= 6.28318531f;
    else if (a < -3.14159265f) a += 6.28318531f;
    if (a != a) a = 0.0f;
    return a;
}

#endif /* PLATFORM_PORT */
#endif /* _TUROK_PORT_H */
