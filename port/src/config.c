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

extern char *getenv(const char *);
extern int   g_turok_walk_mode;      /* input.c — seeded from walk_default at load */

/* Live settings (defaults match the SDL2 backend's prior hard-coded env defaults). */
float g_cfg_mouse_sens   = 6.0f;     /* TUROK_MOUSE_SENS  */
int   g_cfg_mouse_invert = 0;        /* TUROK_MOUSE_INVERT */
int   g_cfg_walk_default = 0;        /* 0 = run, 1 = walk  */
int   g_cfg_win_w        = 1600;     /* TUROK_WIN_W (16:9 default for widescreen) */
int   g_cfg_win_h        = 900;      /* TUROK_WIN_H */
int   g_cfg_audio_3ds    = 0;        /* 3DS: 1 = call ndspInit (needs dspfirm.cdc); 0 = skip (boots silent) */
int   g_cfg_warp         = -1;       /* 3DS bring-up: warp-to-level id (0,1000..8000); -1 = normal legal-screen boot */
int   g_cfg_debug        = 0;        /* 1 = enable the 3DS boot.log / svcOutputDebugString trace logging (off = clean play) */
int   g_cfg_fps          = -1;       /* present-rate cap (3DS); -1 = platform default (30 on 3DS = locked, beat-free), 0 = uncapped/vsync */
int   g_cfg_tick         = -1;       /* logic tick rate (3DS); -1 = platform default (0 on 3DS = logic every present), 30 = 30Hz logic + interp */
int   g_cfg_bake         = 0;        /* 3DS facade texture baking: 0 = OFF (default — Turok's organic art rarely needs the PICA tiled-UV bake, cf. sm64-port; no worker, no hitch), 1 = on (async worker) */
float g_cfg_stereo_z     = -1.0f;    /* 3DS stereo shear depth term (default 0.04); -1 = compiled default. On-device tuning, real-HW only. */
float g_cfg_stereo_w     = -1.0f;    /* 3DS stereo shear convergence term (default 0.012); -1 = compiled default. Raise = screen plane nearer. */
int   g_cfg_gamepad      = 1;        /* PC: 1 = use a connected game controller; 0 = ignore it entirely (escape hatch for a drifting pad that auto-strafes/spins). Env TUROK_GAMEPAD overrides. */
/* In-game DRAW-DISTANCE slider (single slider; the fog recedes with it because the engine's fog is normalized to
 * the projection far plane). Defaults = ORIGINAL Turok. Backed by turok.cfg `drawdist` / `fog`. */
float g_cfg_drawdist     = 1.0f;     /* DRAW DISTANCE multiplier (options slider). 1 = stock far clip; up to turok_drawdist_max() pushes the projection far plane out so the fog recedes/thins and reveals the vista it hid. */
int   g_cfg_fog          = 1;        /* fog master. 1 = on (stock haze); 0 = off (turok.cfg `fog`). */

/* Widescreen (Hor+). g_turok_aspect is published each frame by gfx_pc = the real output aspect ratio (PC window /
 * 3DS 400x240); camera.c projects the 3D world at it so the horizontal FOV widens with no stretch. g_cfg_widescreen
 * gates it (1 = on; 0 = stock 4:3). The HUD is kept 4:3-centered in the Fast3D seam. */
float g_turok_aspect     = 1.3333f;  /* published by gfx_pc each frame; default 4:3 until the first frame. */
int   g_cfg_widescreen   = 1;        /* 1 = Hor+ widescreen (project at the output aspect); 0 = stock 4:3. turok.cfg `widescreen`. */

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
    FILE  *f = fopen(cfg_path(), "r");
    char   line[160], key[64];
    double val;

    if (!f)
        return;
    while (fgets(line, sizeof(line), f)) {
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r')
            continue;
        if (sscanf(line, "%63s %lf", key, &val) != 2)
            continue;
        if      (!strcmp(key, "mouse_sensitivity")) g_cfg_mouse_sens   = (float)val;
        else if (!strcmp(key, "mouse_invert"))      g_cfg_mouse_invert = (int)val ? 1 : 0;
        else if (!strcmp(key, "walk_default"))      g_cfg_walk_default = (int)val ? 1 : 0;
        else if (!strcmp(key, "window_width"))      g_cfg_win_w        = (int)val;
        else if (!strcmp(key, "window_height"))     g_cfg_win_h        = (int)val;
        else if (!strcmp(key, "audio_3ds"))         g_cfg_audio_3ds    = (int)val ? 1 : 0;
        else if (!strcmp(key, "warp"))              g_cfg_warp         = (int)val;
        else if (!strcmp(key, "debug"))             g_cfg_debug        = (int)val ? 1 : 0;
        else if (!strcmp(key, "fps"))               g_cfg_fps          = (int)val;
        else if (!strcmp(key, "tick"))              g_cfg_tick         = (int)val;
        else if (!strcmp(key, "bake"))              g_cfg_bake         = (int)val ? 1 : 0;
        else if (!strcmp(key, "stereo_z"))          g_cfg_stereo_z     = (float)val;
        else if (!strcmp(key, "stereo_w"))          g_cfg_stereo_w     = (float)val;
        else if (!strcmp(key, "gamepad"))           g_cfg_gamepad      = (int)val ? 1 : 0;
        else if (!strcmp(key, "fog"))               g_cfg_fog          = (int)val ? 1 : 0;
        else if (!strcmp(key, "drawdist"))          g_cfg_drawdist     = (float)val;
        else if (!strcmp(key, "widescreen"))        g_cfg_widescreen   = (int)val ? 1 : 0;
    }
    fclose(f);

    /* Clamp the draw distance to the platform ceiling (PC 3x; 3DS 1x). */
    { float mx = turok_drawdist_max();
      if (g_cfg_drawdist > mx)   g_cfg_drawdist = mx;
      if (g_cfg_drawdist < 1.0f) g_cfg_drawdist = 1.0f; }

    g_turok_walk_mode = g_cfg_walk_default;   /* seed the run/walk toggle from the saved default */
    fprintf(stderr, "[config] loaded '%s' (sens=%.2f invert=%d walk=%d win=%dx%d)\n",
            cfg_path(), g_cfg_mouse_sens, g_cfg_mouse_invert, g_cfg_walk_default, g_cfg_win_w, g_cfg_win_h);
}

/* Write the current settings back to disk (called by the options menu when a control setting changes). */
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
    fclose(f);
}
