/*
 * turok_binds.h — user-remappable input bindings (PC only).
 *
 * The bind TOKENS (human-readable strings persisted in turok.cfg) live in config.c — an always-linked,
 * SDL-free TU — so both the game options menu (options.c, C) and the SDL2 backend (gfx_sdl2.cpp, C++)
 * reference the SAME storage on every PC gfx backend (sdl2/egl/osmesa). SDL scancode-name<->code
 * resolution and raw-key capture live ONLY in gfx_sdl2.cpp. On the headless EGL/OSMesa builds nothing
 * consumes the tokens (rebinding is inert there), but the symbols still link because they are in config.c.
 *
 * Gated PC-only; on 3DS/N64 this header is EMPTY (input mapping is input_3ds.c / the raw N64 pad), so a
 * 3DS/N64 preprocess of any TU that includes it shows ZERO new symbols.
 */
#ifndef TUROK_BINDS_H
#define TUROK_BINDS_H

#if defined(PLATFORM_PORT) && !defined(PLATFORM_3DS)

#ifdef __cplusplus
extern "C" {
#endif

/* Rebindable actions. The first 8 assert a HELD N64 pad bit (polled each frame); the last 5 are EDGE
 * seams (fire once on the key/button/wheel DOWN event). An action's CONSUMPTION mechanism is fixed;
 * only its TRIGGER input is remappable. Order is load-bearing (config.c/gfx_sdl2/options.c index it). */
enum {
    BIND_FORWARD = 0, BIND_BACK, BIND_STRAFE_L, BIND_STRAFE_R,  /* HELD: N64 C-up/C-down/C-left/C-right */
    BIND_FIRE, BIND_JUMP, BIND_MAP, BIND_PAUSE,                 /* HELD: N64 Z / R_TRIG / L_TRIG / START */
    BIND_WEAP_NEXT, BIND_WEAP_PREV,                             /* EDGE: g_weapon_cycle +1 / -1          */
    BIND_WALK, BIND_QUICKSAVE, BIND_QUICKLOAD,                  /* EDGE: walk toggle / F5 save / F9 load  */
    BIND_MAX
};
#define TUROK_BIND_SLOTS   2      /* primary + alt (so W and an alt, or LMB and Ctrl, can coexist). */
#define TUROK_BIND_TOKLEN 24      /* max token length incl. NUL. */

/* Token storage — the SOURCE OF TRUTH, persisted in turok.cfg. Tokens are whitespace-free words:
 *   "key:<sdl_scancode_name>"  keyboard (spaces -> underscores, lowercased); "key:#<int>" = raw scancode
 *   "mouse1".."mouse5"         SDL mouse buttons (1=left 2=middle 3=right 4=x1 5=x2)
 *   "wheelup" / "wheeldown"    mouse-wheel notches
 *   "none"                     empty slot                                                            */
extern char g_cfg_bind[BIND_MAX][TUROK_BIND_SLOTS][TUROK_BIND_TOKLEN];
extern int  g_cfg_bind_dirty;     /* options.c sets on a rebind/reset; gfx_sdl2 clears it and re-resolves. */

/* Capture seam: the options menu ARMS it for an action; the SDL2 backend FILLS it on the next raw press
 * (and, while armed, suppresses the pad so the pressed key does not leak into the game). */
extern int  g_bind_capture_action;   /* -1 = idle; >=0 = the BIND_* awaiting the next raw input.          */
extern int  g_bind_capture_done;     /* 1 = the backend finished (captured OR cancelled).                 */
extern int  g_bind_capture_cancel;   /* 1 = ESC cancelled (leave the binding unchanged).                  */
extern char g_bind_capture_result[TUROK_BIND_TOKLEN];  /* the captured token (valid when done && !cancel).*/

/* config.c helpers (pure string, SDL-free). */
void        turokBindsSetDefaults(void);              /* reset g_cfg_bind[][] to the stock PC scheme. */
const char *turokBindActionLabel(int action);         /* short menu label, LARGE_FONT-safe (lowercase+space). */
void        turokBindDisplayName(const char *token, char *out, int outsz);  /* token -> menu display name. */

#ifdef __cplusplus
}
#endif

#endif /* PLATFORM_PORT && !PLATFORM_3DS */
#endif /* TUROK_BINDS_H */
