#ifndef _IN_INPUT_H
#define _IN_INPUT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <PR/ultratypes.h>

/*
 * Input backend contract — the minimal turok seam, structured like perfect_dark /
 * banjo-kazooie `port/include/input.h` (a subset: PD's input.h is a full subsystem with
 * rebinding / mouse / rumble; turok only needs read + push).
 *
 * THE PAD-HANDLE SHAPE: PD declares `inputReadController(idx, OSContPad *npad)`, but
 * `OSContPad` (a libultra type) in the header forces every includer to pull in libultra —
 * which the C++ Fast3D layer (gfx_sdl2.cpp) can't cleanly do. So turok passes the pad
 * OPAQUELY (`void *npad`): this header has ZERO libultra type dependency, so BOTH the C
 * `os_shim.c` and the C++ `gfx_sdl2.cpp` include it. The C consumer passes `&osContPad`;
 * `input.c` (which does include <ultra64.h>) casts it back to `OSContPad *`.
 */

#define INPUT_MAX_CONTROLLERS 4

/* Initialise the input backend. No-op on the host — the window manager pumps the events
 * and pushes the mapped state via inputSetState(). Returns 0 on success. */
s32 inputInit(void);

/* Read player `idx`'s inputs into the N64 controller pad (passed opaquely — see above).
 * Returns 0 on success. (PD analog: inputReadController(idx, OSContPad*).) */
s32 inputReadController(s32 idx, void *npad);

/* The window/input backend (gfx_sdl2 on PC, hid on 3DS) -> here: push the current mapped
 * N64 pad state. turok's split-architecture extension over PD (whose input.c reads SDL
 * directly inside inputReadController). */
void inputSetState(u16 button, s8 stick_x, s8 stick_y);

#ifdef __cplusplus
}
#endif

#endif
