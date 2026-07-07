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
extern int   g_cfg_swap_sticks;        /* config.c — 0 (default): nub = analog move; 1: swapped (New 3DS) */
extern float g_turok_strafe;           /* input.c — analog strafe level (-1..+1, + = right), read by CTMove */
extern float g_turok_forward;          /* input.c — analog fwd/back level (-1..+1, + = forward), read by CTMove */

static s8 cpad_axis(int v)             /* circle pad ~±156 -> N64 stick ±80, deadzone */
{
    if (v > -24 && v < 24) return 0;
    int s = (v * 80) / 156;
    if (s >  80) s =  80;
    if (s < -80) s = -80;
    return (s8)s;
}

/* C-stick NUB scale: the New-3DS nub is stiffer with a SMALLER raw range than the Circle Pad, so the
 * /156 pad scale never reaches full ±80 -> as the analog MOVE stick it topped out below full run speed,
 * which walks on flat ground but is too slow to climb slopes/cliffs (the digital forward ramps to full
 * TMOVE_MAX_RUNSPEED; the nub must too). Normalize the nub by its OWN range (/90) + clamp so a firm push
 * = full ±80 = 1.0 = full run speed, matching the digital forward; still proportional below that. */
static s8 cstick_axis(int v)
{
    if (v > -24 && v < 24) return 0;
    int s = (v * 80) / 90;
    if (s >  80) s =  80;
    if (s < -80) s = -80;
    return (s8)s;
}

void input3dsScan(void)
{
    u32 kHeld, kDown;
    circlePosition cp, cs = {0};        /* cs zero-init = fail-safe: if the New-3DS C-stick (irrst) read
                                           fails to populate it, the seams see 0, not stack garbage. */
    u16 btn = 0;
    s8  sx = 0, sy = 0;
    s8  cpx, cpy, csx, csy;
    static int s_new3ds = -1;           /* -1 = not yet probed; 0 = OG 3DS (no nub); 1 = New 3DS */

    hidScanInput();
    kHeld = hidKeysHeld();
    kDown = hidKeysDown();
    hidCircleRead(&cp);

    /* New 3DS has the C-stick nub; OG 3DS doesn't. Probe once (cached). The nub-as-movement-stick + the flip
     * need two sticks, so OG 3DS (no nub) just keeps the Circle Pad on its classic move+turn scheme. */
    if (s_new3ds < 0) { bool n = false; APT_CheckNew3DS(&n); s_new3ds = n ? 1 : 0; }

    /* C-stick (the nub). irrst is a SEPARATE libctru service from hid — lazy-init it once. On OG 3DS unused. */
    if (s_new3ds) {
        static int s_irrst = 0; if (!s_irrst) { irrstInit(); s_irrst = 1; }
        irrstScanInput();
        hidCstickRead(&cs);             /* macro -> irrstCstickRead */
    } else { cs.dx = cs.dy = 0; }

    cpx = cpad_axis(cp.dx);    cpy = cpad_axis(cp.dy);   /* circle pad -80..80 (its ±156 range) */
    csx = cstick_axis(cs.dx);  csy = cstick_axis(cs.dy); /* C-stick nub -80..80 (reaches full for climb) */

    /* ★ Turok's N64 analog stick is the LOOK stick (stick_x = turn, stick_y = LOOK up/down). So the two roles:
     *   LOOK stick (UNTOUCHED): X -> stick_x (turn), Y -> stick_y (look up/down). The shipped Circle Pad.
     *   MOVE stick (analog):    X -> g_turok_strafe (strafe), Y -> g_turok_forward (fwd/back). Injected into
     *                           the movement code (CTMove) — NEVER the analog stick, or "move up" would LOOK up.
     * Default (swap_sticks 0): Circle Pad = LOOK (untouched), C-stick nub = MOVE.  swap_sticks 1 flips them.
     * OG 3DS (no nub): Circle Pad stays LOOK; movement is the face-button C-buttons (flip ignored). */
    {
        s8 lkx, lky, mvx, mvy;
        if (s_new3ds && g_cfg_swap_sticks) { lkx = csx; lky = csy; mvx = cpx; mvy = cpy; }  /* flip: nub=LOOK, pad=MOVE */
        else if (s_new3ds)                 { lkx = cpx; lky = cpy; mvx = csx; mvy = csy; }  /* default: pad=LOOK, nub=MOVE */
        else                               { lkx = cpx; lky = cpy; mvx = 0;   mvy = 0;   }  /* OG: pad=LOOK only */

        sx = lkx;                                     /* LOOK X -> stick_x = TURN  (untouched) */
        sy = lky;                                     /* LOOK Y -> stick_y = LOOK up/down (untouched) */
        g_turok_strafe  = (float)mvx / 80.0f;         /* MOVE X -> analog STRAFE (-1..+1, + = right) */
        g_turok_forward = (float)mvy / 80.0f;         /* MOVE Y -> analog FWD/BACK (-1..+1, + = forward) */
    }

    /* ── PAUSE / OPTIONS MENU: route the D-pad to menu nav (the menu reads U/D/L/R_JPAD for navigation AND
     * L/R_JPAD for slider adjust), A -> accept (N64 A_BUTTON = the menu's UseMenu), B -> cancel/back, START
     * passes through. Totally independent of the in-game map: the in-game movement/weapon/look buttons are
     * suppressed while a menu is up so they can't leak into it, and the analog seams are zeroed so the player
     * can't drift while paused. */
    {   extern int turokMenuActive(void);
        extern int g_turok_menu_cancel;
        if (turokMenuActive()) {
            u16 mb = 0;
            if (kHeld & KEY_DUP)    mb |= N64_DU;      /* menu up    */
            if (kHeld & KEY_DDOWN)  mb |= N64_DD;      /* menu down  */
            if (kHeld & KEY_DLEFT)  mb |= N64_DL;      /* menu left / slider - */
            if (kHeld & KEY_DRIGHT) mb |= N64_DR;      /* menu right / slider + */
            if (kHeld & KEY_A)      mb |= N64_A;       /* accept (menu accept is single-press) */
            if (kHeld & KEY_START)  mb |= N64_START;   /* start also accepts / toggles pause */
            g_turok_menu_cancel = (kDown & KEY_B) ? 1 : 0;   /* B = cancel / back one level */
            g_turok_strafe = 0.0f;  g_turok_forward = 0.0f;  /* no player movement while paused */
            inputSetState(mb, 0, 0);
            return;
        }
        g_turok_menu_cancel = 0;
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
