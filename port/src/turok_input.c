/*
 * turok_input.c — platform-agnostic controller seam.
 *
 * The window/input backend (SDL2 on PC, hid on 3DS) maps real device input to N64
 * controller bits and pushes it via turokInputSetState(). The libultra shim
 * (os_shim.c osContGetReadData) pulls the current pad via turokInputGetPad().
 *
 * Headless builds (EGL/OSMesa capture) have no backend feeding input, so the pad
 * stays neutral — unless TUROK_FAKEINPUT is set, which injects synthetic motion so
 * the input -> player-movement path can be exercised without a window.
 */
#include <stddef.h>
#include <stdlib.h>
#include <ultra64.h>          /* OSContPad, u16, s8 */

#define INPUT_MEMZERO(p, n) do { char *_p = (char*)(p); size_t _i; for (_i = 0; _i < (n); _i++) _p[_i] = 0; } while (0)

/* Current mapped N64 pad state (written by the backend, read by the shim). */
static volatile unsigned short g_button = 0;
static volatile signed char    g_stick_x = 0;
static volatile signed char    g_stick_y = 0;

/* Backend -> here: set the current N64 pad (button bits + stick, -80..80). */
void turokInputSetState(unsigned short button, signed char stick_x, signed char stick_y)
{
    g_button  = button;
    g_stick_x = stick_x;
    g_stick_y = stick_y;
}

/* Shim -> here: fill one OSContPad (controller 0) with the current input. */
void turokInputGetPad(OSContPad *pad)
{
    if (!pad)
        return;

    INPUT_MEMZERO(pad, sizeof(*pad));   /* clears button/stick/errno */

    /* Synthetic input for headless testing: TUROK_FAKEINPUT drifts the player
     * forward (=1) or forward+turn (=2) so the movement/collision path can be
     * validated without a window. */
    {
        static int fake = -1;
        if (fake < 0) { const char *e = getenv("TUROK_FAKEINPUT"); fake = e ? atoi(e) : 0; }
        if (fake) {
            pad->stick_y = 64;          /* forward */
            pad->stick_x = (signed char)(fake > 1 ? 28 : 0);  /* TUROK_FAKEINPUT=2 also turns */
            return;
        }
    }

    pad->button  = g_button;
    pad->stick_x = g_stick_x;
    pad->stick_y = g_stick_y;
}
