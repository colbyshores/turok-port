/*
 * input.c — host controller seam (the same shared-layer role as perfect_dark /
 * banjo-kazooie `port/src/input.c`; a minimal subset — read + push, no rebind/mouse/rumble).
 *
 * The window/input backend (SDL2 on PC via gfx_sdl2.cpp, hid on 3DS) maps real device input
 * to N64 controller bits and pushes it via inputSetState(). The libultra shim (os_shim.c
 * osContGetReadData) reads the current pad via inputReadController().
 *
 * Headless builds (EGL/OSMesa capture) have no backend feeding input, so the pad stays
 * neutral — unless TUROK_FAKEINPUT is set, which injects synthetic motion so the
 * input -> player-movement path can be exercised without a window.
 *
 * See input.h for the opaque-pad-handle rationale (npad is an OSContPad* passed as void*
 * so the contract header stays libultra-free and C++-includable).
 */
#include <stddef.h>
#include <stdlib.h>
#include <ultra64.h>          /* OSContPad */
#include "input.h"

#define INPUT_MEMZERO(p, n) do { char *_p = (char*)(p); size_t _i; for (_i = 0; _i < (n); _i++) _p[_i] = 0; } while (0)

/* Current mapped N64 pad state (written by the backend, read by the shim). */
static volatile unsigned short g_button = 0;
static volatile signed char    g_stick_x = 0;
static volatile signed char    g_stick_y = 0;

/* PC walk/run toggle (bound to E in the SDL2 backend; read by tmove.c TUROK_WALKCAP). Defined here in
 * an always-linked TU so the EGL/OSMesa headless builds — which don't compile the SDL2 backend — still
 * resolve the symbol. */
int g_turok_walk_mode = 0;

/* Per-frame HELD mouse-look deltas (radians), written by the SDL2 backend (PC) / hid (3DS), consumed +
 * zeroed by the tengine.c mouse-look hook. Held (no spring back to center), gated PLATFORM_PORT. */
float g_look_yaw = 0.0f, g_look_pitch = 0.0f;

/* Analog STRAFE level (-1..+1, + = strafe right), written by the backend (the 3DS dual-analog move-stick X),
 * read by CTMove__ControlSideStep and injected as an analog sidestep velocity (frame_increment-gated like all
 * movement, so it's a LEVEL not a delta — no FPS coupling). 0 = no strafe. Always-linked so every build resolves
 * the symbol; only the 3DS backend writes it today (keyboard/mouse strafe still uses the C-button/bind path). */
float g_turok_strafe = 0.0f;

/* Discrete weapon-cycle accumulator (signed notch count: +next, -prev), written by the scroll wheel in
 * the SDL2 backend, consumed ONE step per LOGIC TICK by tmove.c (CTMove__UpdateTurokInstance). A dedicated
 * seam — NOT a held N64 button — so each wheel notch switches exactly one weapon regardless of render rate
 * (the button path's SelectWeaponTimer is frame_increment-gated, so a held button over-cycles at FPS>TICK).
 * Defined here (always-linked) so the EGL/OSMesa headless builds resolve the symbol. */
int g_weapon_cycle = 0;

/* PC-ONLY quick-save / quick-load requests (F5 / F9 in the SDL2 backend), consumed by the tengine.c hook.
 * A PC dev convenience, separate from the in-game N64 save system; the 3DS uses save points + the file-backed
 * pak (os_shim.c) and has no F5/F9. Gated off 3DS so the whole quick-save path is PC-only. */
#if defined(PLATFORM_PORT) && !defined(PLATFORM_3DS)
int g_quicksave_req = 0, g_quickload_req = 0;
#endif

s32 inputInit(void) { return 0; }   /* host: the window manager pumps events; nothing to init here */

/* Backend -> here: set the current N64 pad (button bits + stick, -80..80). */
void inputSetState(u16 button, s8 stick_x, s8 stick_y)
{
    g_button  = button;
    g_stick_x = stick_x;
    g_stick_y = stick_y;
}

/* Shim -> here: fill controller `idx`'s OSContPad (passed opaquely) with the current input. */
s32 inputReadController(s32 idx, void *npad)
{
    OSContPad *pad = (OSContPad *)npad;
    (void)idx;                          /* turok wires controller 0 only */

    if (!pad)
        return 1;

    INPUT_MEMZERO(pad, sizeof(*pad));    /* clears button/stick/errno */

    /* Synthetic input for headless testing: TUROK_FAKEINPUT drifts the player forward
     * (=1), forward+turn (=2), or patrols (=7) so the movement/collision path can be
     * validated without a window. */
    {
        static int fake = -1;
        if (fake < 0) { const char *e = getenv("TUROK_FAKEINPUT"); fake = e ? atoi(e) : 0; }
        if (fake == 3) { pad->button = 0x0008; return 0; }            /* forward (C-up) */
        if (fake == 5) { pad->button = 0x0008 | 0x2000; return 0; }   /* forward + fire (Z trigger) */
        if (fake == 6) { pad->button = 0x2000; return 0; }            /* fire only */
        if (fake == 8) {                                              /* MAP render test: walk (reveal regions), then hold L + turn */
            static unsigned t8 = 0; t8++;
            if (t8 < 180) { pad->button = 0x0008; }                   /* phase 1: forward — reveal a few regions */
            else { pad->button = 0x0020;                              /* phase 2: hold MAP (L) */
                   pad->stick_x = (signed char)(((t8 / 60) & 1) ? 40 : -40); }  /* + sweep so the map ROTATES (smear test) */
            return 0;
        }
        if (fake == 7) {                                              /* PATROL: forward + sweep turn */
            static unsigned t = 0; t++;
            pad->button  = 0x0008;
            pad->stick_x = (signed char)((t % 240) < 120 ? 40 : -40);
            return 0;
        }
        if (fake) {
            pad->stick_y = 64;
            pad->stick_x = (signed char)(fake > 1 ? 28 : 0);
            return 0;
        }
    }

    pad->button  = g_button;
    pad->stick_x = g_stick_x;
    pad->stick_y = g_stick_y;
    return 0;
}
