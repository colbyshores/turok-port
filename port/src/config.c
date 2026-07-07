/*
 * config.c — host settings persistence (the SHARED-layer role, plain name -> port_config.o).
 *
 * Loads/saves a plain `key value` text file (turok.cfg) so the player's control tuning — mouse sensitivity,
 * invert-Y, run/walk default, window size — survives across runs without environment variables. This is the
 * data layer the in-game options menu reads and writes; for now it's also editable by hand. Environment
 * variables still WIN at read time (so headless tests can override a saved config).
 *
 * Path: PC = $TUROK_CFG or ./turok.cfg ; 3DS = sdmc:/3ds/turok/turok.cfg.
 */
#include <stdio.h>
#include <string.h>

#include "turok_binds.h"             /* PC-only bind table + capture seam (empty on 3DS/N64) */

extern char *getenv(const char *);
extern int   g_turok_walk_mode;      /* input.c — seeded from walk_default at load */

/* Live settings (defaults match the SDL2 backend's prior hard-coded env defaults). */
float g_cfg_mouse_sens   = 6.0f;     /* TUROK_MOUSE_SENS  */
int   g_cfg_mouse_invert = 0;        /* TUROK_MOUSE_INVERT */
int   g_cfg_walk_default = 0;        /* 0 = run, 1 = walk  */
int   g_cfg_win_w        = 1600;     /* TUROK_WIN_W (16:9 default for widescreen) */
int   g_cfg_win_h        = 900;      /* TUROK_WIN_H */
int   g_cfg_audio_3ds    = 1;        /* 3DS: 1 (DEFAULT) = call ndspInit + play audio; real HW always has dspfirm.cdc.
                                       * Set `audio_3ds 0` in turok.cfg ONLY for a firmware-less emulator where
                                       * ndspInit would hang. PC unaffected (uses the SDL sink, not this flag). */
int   g_cfg_music        = 1;        /* music (CSP sequence player) ON by default. USER-CONFIRMED working on
                                       * real 3DS hardware (2026-07-06) — an older "__CSPHandleMIDIMsg wild
                                       * pointer crashes real HW" note was stale (fixed by the ARM-alignment
                                       * sweep / ALCMidiHdr header swap). 0 = off (turok.cfg `music` / PC TUROK_MUSIC=0). */
int   g_cfg_warp         = -1;       /* 3DS bring-up: warp-to-level id (0,1000..8000); -1 = normal legal-screen boot */
int   g_cfg_debug        = 0;        /* 1 = enable the 3DS boot.log / svcOutputDebugString trace logging (off = clean play) */
int   g_cfg_fps          = -1;       /* present-rate cap (3DS); -1 = platform default (30 on 3DS = locked, beat-free), 0 = uncapped/vsync */
int   g_cfg_tick         = -1;       /* logic tick rate (3DS); -1 = platform default (0 on 3DS = logic every present), 30 = 30Hz logic + interp */
int   g_cfg_bake         = 0;        /* 3DS facade texture baking: 0 = OFF (default — Turok's organic art rarely needs the PICA tiled-UV bake, cf. sm64-port; no worker, no hitch), 1 = on (async worker) */
float g_cfg_stereo_z     = -1.0f;    /* 3DS stereo shear depth term (default 0.04); -1 = compiled default. On-device tuning, real-HW only. */
float g_cfg_stereo_w     = -1.0f;    /* 3DS stereo shear convergence term (default 0.012); -1 = compiled default. Raise = screen plane nearer. */
int   g_cfg_gamepad      = 1;        /* PC: 1 = use a connected game controller; 0 = ignore it entirely (escape hatch for a drifting pad that auto-strafes/spins). Env TUROK_GAMEPAD overrides. */
int   g_cfg_fullscreen   = 0;        /* PC: 1 = create/toggle the SDL2 window to borderless desktop fullscreen; 0 = windowed. Set from the options menu; turok.cfg `fullscreen`. */
int   g_cfg_swap_sticks  = 0;        /* New 3DS: 0 (DEFAULT) = C-stick NUB is the analog MOVE stick (up=fwd, down=back, left/right=strafe), Circle Pad UNTOUCHED (classic move+turn); 1 = swapped (nub = classic move+turn, Circle Pad = analog move+strafe). Options-menu toggle; turok.cfg `swap_sticks`. OG 3DS (no nub) ignores it. */
int   g_cfg_recenter_look = 1;       /* 3DS vertical-look: 1 (DEFAULT = shipped behavior) = auto-recenter (releasing the look stick eases the view back to the horizon); 0 = HOLD (the pitch stays where you left it). Options-menu toggle; turok.cfg `recenter_look`. */
int   g_cfg_invert_look  = 1;        /* 3DS vertical-look invert: 1 (DEFAULT) = inverted (push the look stick UP = look DOWN); 0 = normal (push up = look up). Options-menu toggle; turok.cfg `invert_look`. */

/* PC live-window request seam. The options menu (options.c) sets these when the player changes the resolution
 * or fullscreen row; the SDL2 backend (gfx_sdl2.cpp) consumes them once per frame at the top of its event loop
 * (the Alt-Enter-safe game-thread/frame boundary) and applies SDL_SetWindowSize / SDL_SetWindowFullscreen.
 * Defined HERE (an always-compiled TU) so the reference from options.c links on every PC gfx backend, not just
 * SDL2 — on the headless EGL/OSMesa builds nothing consumes them (menu resize/fullscreen are inert there). */
int   g_turok_req_win_w      = 0;    /* >0 = set the windowed size to exactly this; -1 = DESKTOP (native mode). */
int   g_turok_req_win_h      = 0;
int   g_turok_req_fullscreen = -1;   /* -1 = no change; 0 = windowed; 1 = fullscreen. */
int   g_turok_req_dirty      = 0;    /* set by options.c; the backend clears it after applying. */
/* INTERNAL render resolution while fullscreen. Fullscreen is ALWAYS borderless SDL_WINDOW_FULLSCREEN_DESKTOP
 * (matches the desktop's native mode 1:1 — no real monitor mode-switch, so no XRandR/Wayland flakiness and no
 * risk of a crash/kill stranding the display at a switched resolution). So a resolution PRESET picked while
 * fullscreen can't resize the actual window/output; instead gfx_sdl2.cpp mirrors it here, and gfx_pc.cpp
 * (gfx_start_frame/gfx_end_frame) renders the whole scene into an offscreen framebuffer at exactly this size,
 * then scales+letterboxes (preserves aspect — no stretch/distortion) it up onto the real, native-resolution
 * backbuffer at present time. 0 = disabled (render 1:1 at the real window/output size — windowed mode, or
 * fullscreen at the native/DESKTOP preset, both leave this at 0 so there's no extra blit overhead). */
int   g_turok_internal_w     = 0;
int   g_turok_internal_h     = 0;
/* In-game DRAW-DISTANCE slider (single slider; the fog recedes with it because the engine's fog is normalized to
 * the projection far plane). Defaults = ORIGINAL Turok. Backed by turok.cfg `drawdist` / `fog`. */
float g_cfg_drawdist     = 1.0f;     /* DRAW DISTANCE multiplier (options slider). 1 = stock far clip; up to turok_drawdist_max() pushes the projection far plane out so the fog recedes/thins and reveals the vista it hid. */
int   g_cfg_fog          = 1;        /* fog master. 1 = on (stock haze); 0 = off (turok.cfg `fog`). */

/* Widescreen (Hor+). g_turok_aspect is published each frame by gfx_pc = the real output aspect ratio (PC window /
 * 3DS 400x240); camera.c projects the 3D world at it so the horizontal FOV widens with no stretch. g_cfg_widescreen
 * gates it (1 = on; 0 = stock 4:3). The HUD is kept 4:3-centered in the Fast3D seam. */
float g_turok_aspect     = 1.3333f;  /* published by gfx_pc each frame; default 4:3 until the first frame. */
int   g_cfg_widescreen   = 1;        /* 1 = Hor+ widescreen (project at the output aspect); 0 = stock 4:3. turok.cfg `widescreen`. */

/* NEAR CLIP distance. Stock Turok uses SCALING_NEAR_CLIP = far/64 = 16 units — a big near plane chosen for the
 * N64's 16-bit z-buffer. On the port's 24-bit depth that's wastefully far, and it's the root of the "camera
 * clips into the wall / see through geometry" bug: when the eye gets within 16 units of a wall the wall is
 * entirely behind the near plane and can't render correctly (esp. on the 3DS, where the §29 emulation can only
 * clamp such verts' depth, not fix their projection). 4 units keeps the eye from ever clipping a wall in normal
 * play while leaving far-plane depth precision far better than the N64 had (24-bit, near/far=4/1024). Tune on
 * 3DS HW via turok.cfg `nearclip`: lower (2) if a wall still clips, higher (8) if distant z-fighting appears. */
float g_cfg_nearclip     = 4.0f;
int   g_cfg_memlog       = 0;        /* 3DS: 1 = emit the per-second MEM heartbeat (linFreeKB/linMinKB/texOOM) to boot.log (needs `debug 1`). Diagnostic for the FCRAM/linear-heap pressure that drops alpha textures then wedges the GPU. */
int   g_cfg_fogclamp     = 6;        /* 3DS: max combiner stages that still get the appended TEV fog stage. 6 = stock (fog on any draw with a free stage, can hit the 6-stage PICA ceiling). Lower (e.g. 4) clamps busy combiners back to the hardware FogLut so dense fogged levels (Lost City) can't run the GPU to the stage limit. turok.cfg `fogclamp`. */

#if defined(PLATFORM_PORT) && !defined(PLATFORM_3DS)
/* ── User-remappable input bindings (PC only) ───────────────────────────────────────────────────────────────
 * Storage of the raw TOKENS (the source of truth, persisted in turok.cfg). config.c is SDL-free, so it only
 * stores/serializes tokens; gfx_sdl2.cpp resolves them to SDL scancodes/buttons. See turok_binds.h. */
char g_cfg_bind[BIND_MAX][TUROK_BIND_SLOTS][TUROK_BIND_TOKLEN];
int  g_cfg_bind_dirty      = 0;

/* Capture seam (the options menu arms; gfx_sdl2 fills). */
int  g_bind_capture_action = -1;
int  g_bind_capture_done   = 0;
int  g_bind_capture_cancel = 0;
char g_bind_capture_result[TUROK_BIND_TOKLEN];

/* cfg key stem <-> action id (the file keys are `bind_<name>` primary, `bind_<name>2` alt). */
static const struct { const char *name; int action; } s_bind_keys[] = {
    {"forward",     BIND_FORWARD},  {"back",        BIND_BACK},
    {"strafe_left", BIND_STRAFE_L}, {"strafe_right",BIND_STRAFE_R},
    {"fire",        BIND_FIRE},     {"jump",        BIND_JUMP},
    {"map",         BIND_MAP},      {"pause",       BIND_PAUSE},
    {"weapon_next", BIND_WEAP_NEXT},{"weapon_prev", BIND_WEAP_PREV},
    {"walk",        BIND_WALK},     {"quicksave",   BIND_QUICKSAVE},
    {"quickload",   BIND_QUICKLOAD},
};
#define N_BIND_KEYS ((int)(sizeof s_bind_keys / sizeof s_bind_keys[0]))

/* Reset the whole table to the stock PC scheme (== the port's prior hard-coded gfx_sdl2 mapping). */
void turokBindsSetDefaults(void)
{
    static const char *def[BIND_MAX][2] = {
        {"key:w","none"},   {"key:s","none"},   {"key:a","none"},   {"key:d","none"},
        {"mouse1","key:left_ctrl"}, {"mouse3","key:space"}, {"key:tab","key:m"}, {"key:return","none"},
        {"wheelup","none"}, {"wheeldown","none"},
        {"key:e","none"},   {"key:f5","none"},  {"key:f9","none"},
    };
    int a, s;
    for (a = 0; a < BIND_MAX; a++)
        for (s = 0; s < TUROK_BIND_SLOTS; s++) {
            strncpy(g_cfg_bind[a][s], def[a][s], TUROK_BIND_TOKLEN - 1);
            g_cfg_bind[a][s][TUROK_BIND_TOKLEN - 1] = 0;
        }
}

const char *turokBindActionLabel(int action)
{
    switch (action) {
        case BIND_FORWARD:   return "forward";
        case BIND_BACK:      return "back";
        case BIND_STRAFE_L:  return "strafe l";
        case BIND_STRAFE_R:  return "strafe r";
        case BIND_FIRE:      return "fire";
        case BIND_JUMP:      return "jump";
        case BIND_MAP:       return "map";
        case BIND_PAUSE:     return "pause";
        case BIND_WEAP_NEXT: return "weap next";
        case BIND_WEAP_PREV: return "weap prev";
        case BIND_WALK:      return "walk";
        case BIND_QUICKSAVE: return "quicksave";
        case BIND_QUICKLOAD: return "quickload";
    }
    return "?";
}

/* Token -> menu display name (LARGE_FONT-safe: lowercase letters + digits + space; anything else -> space,
 * so ':' / '-' / '#' can't alias to the Z/M glyphs). Drops the "key:" prefix, underscores become spaces. */
void turokBindDisplayName(const char *token, char *out, int outsz)
{
    int o = 0, i;
    if (outsz <= 0) return;
    if (!token || !token[0] || !strcmp(token, "none")) { strncpy(out, "none", outsz - 1); out[outsz - 1] = 0; return; }
    if (!strncmp(token, "key:", 4)) token += 4;
    for (i = 0; token[i] && o < outsz - 1; i++) {
        char c = token[i];
        if (c == '_') c = ' ';
        if (c >= 'A' && c <= 'Z') c = (char)(c + 32);
        if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == ' ')) c = ' ';
        out[o++] = c;
    }
    out[o] = 0;
    if (o == 0) { strncpy(out, "none", outsz - 1); out[outsz - 1] = 0; }
}

/* Parse a `bind_<name>[2] <token>` line into g_cfg_bind. */
static void turokConfigParseBind(const char *bkey, const char *bval)
{
    char name[48];
    int  i, L;
    strncpy(name, bkey + 5, sizeof name - 1);   /* skip "bind_" */
    name[sizeof name - 1] = 0;
    for (i = 0; i < N_BIND_KEYS; i++)
        if (!strcmp(name, s_bind_keys[i].name)) {
            strncpy(g_cfg_bind[s_bind_keys[i].action][0], bval, TUROK_BIND_TOKLEN - 1);
            g_cfg_bind[s_bind_keys[i].action][0][TUROK_BIND_TOKLEN - 1] = 0;
            return;
        }
    L = (int)strlen(name);
    if (L > 1 && name[L - 1] == '2') {          /* alt slot: `bind_<name>2` */
        name[L - 1] = 0;
        for (i = 0; i < N_BIND_KEYS; i++)
            if (!strcmp(name, s_bind_keys[i].name)) {
                strncpy(g_cfg_bind[s_bind_keys[i].action][1], bval, TUROK_BIND_TOKLEN - 1);
                g_cfg_bind[s_bind_keys[i].action][1][TUROK_BIND_TOKLEN - 1] = 0;
                return;
            }
    }
}
#endif  /* PLATFORM_PORT && !PLATFORM_3DS */

/* Draw-distance ceiling. PC = up to 3x (the slider is a PC feature). 3DS is locked at stock 1x (draw distance is
 * framerate-gated there); the options menu hides the slider row when the max is 1x. */
float turok_drawdist_max(void)
{
#ifdef PLATFORM_3DS
    return 1.0f;                              /* 3DS: locked at stock (slider row hidden) */
#else
    return 3.0f;                              /* PC = up to 3x */
#endif
}

static const char *cfg_path(void)
{
#ifdef PLATFORM_3DS
    return "sdmc:/3ds/turok/turok.cfg";
#else
    const char *e = getenv("TUROK_CFG");
    return (e && *e) ? e : "turok.cfg";
#endif
}

/* Load settings from disk (silent no-op if the file is absent). Call once at startup, before gfx/input init. */
void turokConfigLoad(void)
{
    FILE  *f;
    char   line[160], key[64];
    double val;

#if defined(PLATFORM_PORT) && !defined(PLATFORM_3DS)
    turokBindsSetDefaults();          /* stock scheme first, so cfg `bind_*` lines override + an absent file = defaults */
#endif

    f = fopen(cfg_path(), "r");
    if (!f)
        return;
    while (fgets(line, sizeof(line), f)) {
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r')
            continue;
#if defined(PLATFORM_PORT) && !defined(PLATFORM_3DS)
        /* input binds have STRING values (`bind_fire mouse1`), so handle them before the numeric parse
         * (which would reject the token and skip the line). */
        {   char bkey[64], bval[TUROK_BIND_TOKLEN];
            if (sscanf(line, "%63s %23s", bkey, bval) == 2 && !strncmp(bkey, "bind_", 5)) {
                turokConfigParseBind(bkey, bval);
                continue;
            }
        }
#endif
        if (sscanf(line, "%63s %lf", key, &val) != 2)
            continue;
        if      (!strcmp(key, "mouse_sensitivity")) g_cfg_mouse_sens   = (float)val;
        else if (!strcmp(key, "mouse_invert"))      g_cfg_mouse_invert = (int)val ? 1 : 0;
        else if (!strcmp(key, "walk_default"))      g_cfg_walk_default = (int)val ? 1 : 0;
        else if (!strcmp(key, "window_width"))      g_cfg_win_w        = (int)val;
        else if (!strcmp(key, "window_height"))     g_cfg_win_h        = (int)val;
        else if (!strcmp(key, "audio_3ds"))         g_cfg_audio_3ds    = (int)val ? 1 : 0;
        else if (!strcmp(key, "music"))             g_cfg_music        = (int)val ? 1 : 0;
        else if (!strcmp(key, "warp"))              g_cfg_warp         = (int)val;
        else if (!strcmp(key, "debug"))             g_cfg_debug        = (int)val ? 1 : 0;
        else if (!strcmp(key, "fps"))               g_cfg_fps          = (int)val;
        else if (!strcmp(key, "tick"))              g_cfg_tick         = (int)val;
        else if (!strcmp(key, "bake"))              g_cfg_bake         = (int)val ? 1 : 0;
        else if (!strcmp(key, "stereo_z"))          g_cfg_stereo_z     = (float)val;
        else if (!strcmp(key, "stereo_w"))          g_cfg_stereo_w     = (float)val;
        else if (!strcmp(key, "gamepad"))           g_cfg_gamepad      = (int)val ? 1 : 0;
        else if (!strcmp(key, "fullscreen"))        g_cfg_fullscreen   = (int)val ? 1 : 0;
        else if (!strcmp(key, "swap_sticks"))       g_cfg_swap_sticks  = (int)val ? 1 : 0;
        else if (!strcmp(key, "recenter_look"))     g_cfg_recenter_look = (int)val ? 1 : 0;
        else if (!strcmp(key, "invert_look"))       g_cfg_invert_look  = (int)val ? 1 : 0;
        else if (!strcmp(key, "fog"))               g_cfg_fog          = (int)val ? 1 : 0;
        else if (!strcmp(key, "drawdist"))          g_cfg_drawdist     = (float)val;
        else if (!strcmp(key, "widescreen"))        g_cfg_widescreen   = (int)val ? 1 : 0;
        else if (!strcmp(key, "nearclip"))          g_cfg_nearclip     = (float)val;
        else if (!strcmp(key, "memlog"))            g_cfg_memlog       = (int)val ? 1 : 0;
        else if (!strcmp(key, "fogclamp"))          g_cfg_fogclamp     = (int)val;
    }
    fclose(f);

    /* Clamp the draw distance to the platform ceiling (PC 3x; 3DS 1x). */
    { float mx = turok_drawdist_max();
      if (g_cfg_drawdist > mx)   g_cfg_drawdist = mx;
      if (g_cfg_drawdist < 1.0f) g_cfg_drawdist = 1.0f; }

    /* Clamp the near clip to a sane range (0.5..16). 16 = stock; below ~0.5 risks close-up z precision. */
    if (g_cfg_nearclip > 16.0f) g_cfg_nearclip = 16.0f;
    if (g_cfg_nearclip < 0.5f)  g_cfg_nearclip = 0.5f;

    /* Clamp the fog stage cap to 0..6. 0 = no TEV fog at all (all FogLut); 6 = stock. */
    if (g_cfg_fogclamp > 6) g_cfg_fogclamp = 6;
    if (g_cfg_fogclamp < 0) g_cfg_fogclamp = 0;

    g_turok_walk_mode = g_cfg_walk_default;   /* seed the run/walk toggle from the saved default */
    fprintf(stderr, "[config] loaded '%s' (sens=%.2f invert=%d walk=%d win=%dx%d)\n",
            cfg_path(), g_cfg_mouse_sens, g_cfg_mouse_invert, g_cfg_walk_default, g_cfg_win_w, g_cfg_win_h);
#if defined(PLATFORM_PORT) && !defined(PLATFORM_3DS)
    fprintf(stderr, "[input] %d binds active, fire=%s forward=%s jump=%s\n",
            (int)BIND_MAX, g_cfg_bind[BIND_FIRE][0], g_cfg_bind[BIND_FORWARD][0], g_cfg_bind[BIND_JUMP][0]);
#endif
}

/* Write the current settings back to disk (called by the options menu when a setting changes).
 * ★ Emits EVERY supported key — a partial rewrite (the old 5-key version) would DELETE the user's hand-edited
 * drawdist/widescreen/nearclip/fullscreen/etc. lines, since fopen("w") truncates the whole file. */
void turokConfigSave(void)
{
    FILE *f = fopen(cfg_path(), "w");
    if (!f) {
        fprintf(stderr, "[config] could not write '%s'\n", cfg_path());
        return;
    }
    fprintf(f, "# Turok PC settings — edit by hand or via the in-game options menu\n");
    fprintf(f, "mouse_sensitivity %.2f\n", g_cfg_mouse_sens);
    fprintf(f, "mouse_invert %d\n",        g_cfg_mouse_invert);
    fprintf(f, "walk_default %d\n",        g_cfg_walk_default);
    fprintf(f, "window_width %d\n",        g_cfg_win_w);
    fprintf(f, "window_height %d\n",       g_cfg_win_h);
    fprintf(f, "fullscreen %d\n",          g_cfg_fullscreen);
    fprintf(f, "swap_sticks %d\n",         g_cfg_swap_sticks);
    fprintf(f, "recenter_look %d\n",       g_cfg_recenter_look);
    fprintf(f, "invert_look %d\n",         g_cfg_invert_look);
    fprintf(f, "audio_3ds %d\n",           g_cfg_audio_3ds);
    fprintf(f, "music %d\n",               g_cfg_music);
    fprintf(f, "warp %d\n",                g_cfg_warp);
    fprintf(f, "debug %d\n",               g_cfg_debug);
    fprintf(f, "fps %d\n",                 g_cfg_fps);
    fprintf(f, "tick %d\n",                g_cfg_tick);
    fprintf(f, "bake %d\n",                g_cfg_bake);
    fprintf(f, "stereo_z %.4f\n",          g_cfg_stereo_z);
    fprintf(f, "stereo_w %.4f\n",          g_cfg_stereo_w);
    fprintf(f, "gamepad %d\n",             g_cfg_gamepad);
    fprintf(f, "fog %d\n",                 g_cfg_fog);
    fprintf(f, "drawdist %.4f\n",          g_cfg_drawdist);
    fprintf(f, "widescreen %d\n",          g_cfg_widescreen);
    fprintf(f, "nearclip %.4f\n",          g_cfg_nearclip);
    fprintf(f, "memlog %d\n",              g_cfg_memlog);
    fprintf(f, "fogclamp %d\n",            g_cfg_fogclamp);
#if defined(PLATFORM_PORT) && !defined(PLATFORM_3DS)
    {   int i;
        fprintf(f, "# input bindings — key:<sdl name>, mouse1..5, wheelup/wheeldown, none\n");
        for (i = 0; i < N_BIND_KEYS; i++) {
            fprintf(f, "bind_%s %s\n",  s_bind_keys[i].name, g_cfg_bind[s_bind_keys[i].action][0]);
            fprintf(f, "bind_%s2 %s\n", s_bind_keys[i].name, g_cfg_bind[s_bind_keys[i].action][1]);
        }
    }
#endif
    fclose(f);
    fprintf(stderr, "[config] saved '%s' (win=%dx%d fullscreen=%d drawdist=%.2f)\n",
            cfg_path(), g_cfg_win_w, g_cfg_win_h, g_cfg_fullscreen, g_cfg_drawdist);
}
