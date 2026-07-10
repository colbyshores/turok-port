/*
 * turok_padbinds.h — user-remappable GAMEPAD/BUTTON bindings (3DS AND PC controller).
 *
 * Unlike turok_binds.h (PC keyboard/mouse ONLY), this pad-button remap is shared by BOTH the 3DS and PC
 * ports. The bindings (game ACTION -> abstract PAD BUTTON) live in config.c — an always-linked, backend-free
 * TU compiled on both platforms — so the options menu (options.c, C), the 3DS HID backend (input_3ds.c) and
 * the PC SDL backend (gfx_sdl2.cpp, C++) all reference the SAME storage. Each platform maps the abstract
 * PADBTN_* onto its native buttons (3DS KEY_*, SDL_CONTROLLER_BUTTON_*), and each applies the bound actions
 * with its own mechanism (3DS asserts held N64 bits + edge toggles; PC also routes weapon-cycle through the
 * FPS>TICK-safe g_weapon_cycle seam). Only the BINDING and the MENU are shared; the physical read + apply are
 * per-backend.
 *
 * Gated PLATFORM_PORT (PC + 3DS); EMPTY on N64 (input mapping there is the raw pad), so an N64 preprocess of
 * any TU that includes it shows ZERO new symbols.
 */
#ifndef TUROK_PADBINDS_H
#define TUROK_PADBINDS_H

#ifdef PLATFORM_PORT

#ifdef __cplusplus
extern "C" {
#endif

/* Remappable pad ACTIONS. Order is load-bearing (config.c / options.c / both backends index it). PAUSE is
 * deliberately NOT here — it stays hard-wired to START (3DS) / START+ESC (PC) so a bad rebind can never lock
 * the player out of the menu. Movement sticks (Circle Pad / left stick) and the look stick are also not here;
 * those are the analog seams tuned by the sensitivity sliders, not digital buttons. */
enum {
    PADACT_FORWARD = 0, PADACT_BACK, PADACT_STRAFE_L, PADACT_STRAFE_R,  /* digital move -> C-buttons */
    PADACT_FIRE, PADACT_JUMP, PADACT_MAP, PADACT_WALK,                  /* fire / jump / map / walk toggle */
    PADACT_WEAP_NEXT, PADACT_WEAP_PREV,                                 /* weapon cycle */
    PADACT_MAX
};

/* Abstract pad BUTTONS, mapped per-platform BY NAME (PADBTN_A -> KEY_A on 3DS, SDL_CONTROLLER_BUTTON_A on PC),
 * so each controller's physically-labelled button drives the matching binding. PADBTN_NONE = unbound. ZL/ZR
 * are New-3DS / PC analog-trigger buttons (never fire on an OG 3DS). */
enum {
    PADBTN_NONE = 0,
    PADBTN_A, PADBTN_B, PADBTN_X, PADBTN_Y,
    PADBTN_L, PADBTN_R, PADBTN_ZL, PADBTN_ZR,
    PADBTN_START, PADBTN_SELECT,
    PADBTN_DUP, PADBTN_DDOWN, PADBTN_DLEFT, PADBTN_DRIGHT,
    PADBTN_MAX
};

/* Binding table: action -> abstract button (single slot). SOURCE OF TRUTH, persisted in turok.cfg as
 * `pad_<action> <buttonname>`. */
extern int g_cfg_padbind[PADACT_MAX];
extern int g_cfg_padbind_dirty;       /* options.c sets on a rebind/reset; the backend clears it + re-reads. */

/* Interactive capture seam (mirrors the keyboard g_bind_capture_*). The options menu ARMS it for an action;
 * the platform backend FILLS the result on the next pad-button DOWN (and suppresses that press so it doesn't
 * leak into the game / the menu). */
extern int g_padbind_capture_action;  /* -1 = idle; >=0 = the PADACT_* awaiting a button. */
extern int g_padbind_capture_done;    /* 1 = backend finished (captured OR cancelled). */
extern int g_padbind_capture_cancel;  /* 1 = cancelled (leave the binding unchanged). */
extern int g_padbind_capture_result;  /* the captured PADBTN_* (valid when done && !cancel). */

/* config.c helpers (pure, backend-free). */
void        turokPadSetDefaults(void);          /* reset g_cfg_padbind[] to the per-platform stock scheme. */
const char *turokPadActionLabel(int action);    /* menu label (LARGE_FONT-safe: lowercase + space). */
const char *turokPadButtonName(int padbtn);     /* menu display name for a bound button. */

#ifdef __cplusplus
}
#endif

#endif /* PLATFORM_PORT */
#endif /* TUROK_PADBINDS_H */
