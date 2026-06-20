/*
 * input_3ds.c — Turok 3DS controller source (libctru HID).
 *
 * Reads the 3DS pad each frame and pushes it through the SAME portable seam the PC backend uses:
 * inputSetState() -> g_button/g_stick in input.c, which the libultra shim (os_shim osContGetReadData)
 * reads via inputReadController(). So input.c stays platform-agnostic (kept in the 3DS build); this
 * file only adds the HID poll + the 3DS button map.
 *
 * input3dsScan() is called once per frame from gfx_3ds.c::handle_events (the per-frame wapi callback).
 *
 * First-boot FPS map (refine later): circle pad = move/turn (analog stick), A=jump, B=fire, X/Y=weapon
 * prev/next, L=aim/map, R=fire, D-pad=digital move (C-buttons), START=pause. (Default engine config is
 * right-handed: movement on the C-buttons, jump on R_TRIG, fire on Z_TRIG.)
 */
#ifdef PLATFORM_3DS
#include <3ds.h>
#include <PR/ultratypes.h>
#include "input.h"

/* N64 OSContPad button bits (match gfx_sdl2.cpp / ultra64 CONT_*). */
#define N64_A     0x8000u
#define N64_B     0x4000u
#define N64_Z     0x2000u
#define N64_START 0x1000u
#define N64_DU    0x0800u
#define N64_DD    0x0400u
#define N64_DL    0x0200u
#define N64_DR    0x0100u
#define N64_L     0x0020u
#define N64_R     0x0010u
#define N64_CU    0x0008u
#define N64_CD    0x0004u
#define N64_CL    0x0002u
#define N64_CR    0x0001u

extern int g_turok_walk_mode;          /* input.c — E-equivalent run/walk toggle (SELECT) */

static s8 cpad_axis(int v)             /* circle pad ~±156 -> N64 stick ±80, deadzone */
{
    if (v > -24 && v < 24) return 0;
    int s = (v * 80) / 156;
    if (s >  80) s =  80;
    if (s < -80) s = -80;
    return (s8)s;
}

void input3dsScan(void)
{
    u32 kHeld, kDown;
    circlePosition cp;
    u16 btn = 0;
    s8  sx, sy;

    hidScanInput();
    kHeld = hidKeysHeld();
    kDown = hidKeysDown();
    hidCircleRead(&cp);

    /* circle pad -> analog stick (Turok: stick_y = fwd/back, stick_x = turn). */
    sx = cpad_axis(cp.dx);
    sy = cpad_axis(cp.dy);

    /* digital movement on the C-buttons (engine default right-handed config). */
    if (kHeld & KEY_DUP)    btn |= N64_CU;   /* forward  */
    if (kHeld & KEY_DDOWN)  btn |= N64_CD;   /* backward */
    if (kHeld & KEY_DLEFT)  btn |= N64_CL;   /* strafe L */
    if (kHeld & KEY_DRIGHT) btn |= N64_CR;   /* strafe R */

    /* actions */
    if (kHeld & KEY_B)      btn |= N64_Z;    /* fire (Z_TRIG)  */
    if (kHeld & KEY_R)      btn |= N64_Z;    /* fire (shoulder)*/
    if (kHeld & KEY_A)      btn |= N64_R;    /* jump (R_TRIG)  */
    if (kHeld & KEY_Y)      btn |= N64_A;    /* next weapon    */
    if (kHeld & KEY_X)      btn |= N64_B;    /* prev weapon    */
    if (kHeld & KEY_L)      btn |= N64_L;    /* map / aim (L_TRIG) */
    if (kHeld & KEY_START)  btn |= N64_START;/* pause */

    /* SELECT toggles run/walk (the 3DS E-equivalent), edge-triggered. */
    if (kDown & KEY_SELECT) g_turok_walk_mode = !g_turok_walk_mode;

    inputSetState(btn, sx, sy);
}
#endif /* PLATFORM_3DS */
