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
extern int   g_cfg_swap_sticks;        /* config.c — 0: nub moves + Circle Pad looks; 1: swapped (New 3DS) */
extern float g_look_yaw, g_look_pitch; /* input.c — per-frame HELD look deltas (rad), consumed by tengine.c */
extern float g_turok_strafe;           /* input.c — analog strafe level (-1..+1, + = right), read by CTMove */

/* Look-stick sensitivity: radians of aim per frame at full deflection. Tuned for the 3DS locked-30 present
 * (fps==tick), the same held look seam the PC mouse feeds (+yaw = turn right, +pitch = look up). */
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
    s8  sx = 0, sy = 0;
    s8  cpx, cpy, csx, csy;
    static int s_new3ds = -1;           /* -1 = not yet probed; 0 = OG 3DS (no nub); 1 = New 3DS */

    hidScanInput();
    kHeld = hidKeysHeld();
    kDown = hidKeysDown();
    hidCircleRead(&cp);

    /* New 3DS has the C-stick nub; OG 3DS doesn't. Probe once (cached). Dual-analog needs two sticks, so OG
     * 3DS stays on the classic single-stick (move+turn) scheme; only New 3DS splits move/look across sticks. */
    if (s_new3ds < 0) { bool n = false; APT_CheckNew3DS(&n); s_new3ds = n ? 1 : 0; }

    /* C-stick (the nub). irrst is a SEPARATE libctru service from hid — lazy-init it once. On OG 3DS this
     * just reads (0,0); we don't use it there anyway (s_new3ds==0 path below). */
    if (s_new3ds) {
        static int s_irrst = 0; if (!s_irrst) { irrstInit(); s_irrst = 1; }
        irrstScanInput();
        hidCstickRead(&cs);             /* macro -> irrstCstickRead */
    } else { cs.dx = cs.dy = 0; }

    cpx = cpad_axis(cp.dx);  cpy = cpad_axis(cp.dy);   /* circle pad -80..80 */
    csx = cpad_axis(cs.dx);  csy = cpad_axis(cs.dy);   /* C-stick    -80..80 */

    if (!s_new3ds)
    {
        /* OG 3DS (single stick): classic move+TURN on the Circle Pad, exactly as shipped. No strafe/look split. */
        sx = cpx;                       /* stick_x = turn   */
        sy = cpy;                       /* stick_y = fwd/back */
        g_turok_strafe = 0.0f;
    }
    else
    {
        /* New 3DS DUAL-ANALOG. BOTH sticks read as proportional analog axes (cpad_axis, same scale/deadzone).
         * MOVE stick = fwd/back (stick_y) + STRAFE (g_turok_strafe), NO turn (stick_x=0); AIM stick = yaw
         * (g_look_yaw) + pitch (g_look_pitch). Default (swap_sticks 0): the C-stick NUB MOVES (fwd/back/strafe),
         * the Circle Pad AIMS. swap_sticks 1 flips them. NB this is the analog-stick swap only — distinct from
         * the engine's right/left-handed C-button option (which swaps the C-buttons <-> stick). */
        s8 mvx, mvy, lkx, lky;
        if (g_cfg_swap_sticks) { mvx = cpx; mvy = cpy; lkx = csx; lky = csy; }  /* swapped: Circle Pad moves, C-stick aims */
        else                   { mvx = csx; mvy = csy; lkx = cpx; lky = cpy; }  /* default: C-stick (nub) moves, Circle Pad aims */
        sx = 0;                                       /* no analog turn from the move stick */
        sy = mvy;                                     /* analog forward/back */
        g_turok_strafe = (float)mvx / 80.0f;          /* analog strafe, -1..+1 (+ = right) */
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
