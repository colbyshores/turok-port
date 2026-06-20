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
int   g_cfg_win_w        = 1280;     /* TUROK_WIN_W */
int   g_cfg_win_h        = 1024;     /* TUROK_WIN_H */

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
    }
    fclose(f);

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
