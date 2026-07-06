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
 * Control layout (user, 2026-06-21): Y=forward, A=backward, X=strafe-left, B=strafe-right (the face
 * buttons -> the engine's C-button movement); D-pad up/right=cycle weapon up, down/left=cycle weapon
 * down (-> A/B weapon next/prev); R=fire (Z_TRIG), L=jump (R_TRIG); START=pause; SELECT=run/walk toggle;
 * circle pad = turn/move (analog stick). (Default engine config is right-handed: movement on the
 * C-buttons, jump on R_TRIG, fire on Z_TRIG, weapon next/prev on A/B.)
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

extern int   g_turok_walk_mode;        /* input.c — E-equivalent run/walk toggle (SELECT) */
extern int   g_cfg_swap_sticks;        /* config.c — 0: Circle Pad moves + C-stick looks; 1: swapped */
extern float g_look_yaw, g_look_pitch; /* input.c — per-frame HELD look deltas (rad), consumed by tengine.c */

/* C-stick "look" sensitivity: radians of aim per frame at full deflection. Tuned for the 3DS locked-30
 * present (fps==tick), the same held look seam the PC mouse feeds (+yaw = turn right, +pitch = look up). */
#define STICK_LOOK_YAW    0.060f       /* ~103 deg/s yaw at full deflection @30fps */
#define STICK_LOOK_PITCH  0.045f       /* pitch a touch slower than yaw */

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
    circlePosition cp, cs;
    u16 btn = 0;
    s8  sx, sy;
    s8  cpx, cpy, csx, csy;

    hidScanInput();
    kHeld = hidKeysHeld();
    kDown = hidKeysDown();
    hidCircleRead(&cp);

    /* New-3DS C-stick (the nub). irrst is a SEPARATE libctru service from hid — lazy-init it once. On an
     * OG 3DS with no C-stick this just reads (0,0), so nub-look is harmlessly inert there. */
    { static int s_irrst = 0; if (!s_irrst) { irrstInit(); s_irrst = 1; } }
    irrstScanInput();
    hidCstickRead(&cs);                 /* macro -> irrstCstickRead */

    cpx = cpad_axis(cp.dx);  cpy = cpad_axis(cp.dy);   /* circle pad -80..80 */
    csx = cpad_axis(cs.dx);  csy = cpad_axis(cs.dy);   /* C-stick    -80..80 */

    /* Assign the two physical sticks to MOVE (engine analog: x=turn, y=fwd/back) vs LOOK (held aim seam
     * g_look_yaw/pitch). Default (swap_sticks 0): Circle Pad moves+turns, C-stick looks/aims. The toggle
     * (options menu / turok.cfg `swap_sticks`) flips which stick does which. */
    {   s8 lkx, lky;
        if (g_cfg_swap_sticks) { sx = csx; sy = csy; lkx = cpx; lky = cpy; }
        else                   { sx = cpx; sy = cpy; lkx = csx; lky = csy; }
        g_look_yaw   += ((float)lkx / 80.0f) * STICK_LOOK_YAW;
        g_look_pitch += ((float)lky / 80.0f) * STICK_LOOK_PITCH;
    }

    /* ── User control layout (2026-06-21) ─────────────────────────────────────
     * Engine right-handed config: movement = C-buttons, Fire=Z_TRIG, Jump=R_TRIG,
     * WeaponNext=A_BUTTON, WeaponPrev=B_BUTTON. We map the 3DS physical buttons
     * onto those N64 bits. NB: the 3DS D-pad is NOT mapped to the N64 D-pad (that
     * IS the engine's run/walk toggle) — it drives the weapon cycle via A/B. */

    /* movement — face buttons -> C-buttons. Natural diamond (user-confirmed on hardware):
     * X=top=forward, B=bottom=back, Y=left=strafe-left, A=right=strafe-right. */
    if (kHeld & KEY_X)  btn |= N64_CU;   /* X = move forward   */
    if (kHeld & KEY_B)  btn |= N64_CD;   /* B = move backward  */
    if (kHeld & KEY_Y)  btn |= N64_CL;   /* Y = strafe left    */
    if (kHeld & KEY_A)  btn |= N64_CR;   /* A = strafe right   */

    /* weapon cycle — D-pad -> A/B (next/prev). up & right = up; down & left = down */
    if (kHeld & (KEY_DUP   | KEY_DRIGHT)) btn |= N64_A;  /* cycle weapons up   (next) */
    if (kHeld & (KEY_DDOWN | KEY_DLEFT))  btn |= N64_B;  /* cycle weapons down (prev) */

    /* shoulders */
    if (kHeld & KEY_R)  btn |= N64_Z;    /* R = fire (Z_TRIG) */
    if (kHeld & KEY_L)  btn |= N64_R;    /* L = jump (R_TRIG) */

    if (kHeld & KEY_START) btn |= N64_START;  /* pause */

    /* SELECT toggles run/walk (the 3DS E-equivalent), edge-triggered. */
    if (kDown & KEY_SELECT) g_turok_walk_mode = !g_turok_walk_mode;

    inputSetState(btn, sx, sy);
}
#endif /* PLATFORM_3DS */
