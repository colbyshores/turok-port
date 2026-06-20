#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <unistd.h>
#include <time.h>

#include "platform.h"
#include "system.h"
#include "input.h"   /* the opaque-pad-handle contract is libultra-free, so it includes cleanly in C++ */

#include "gfx_window_manager_api.h"
#include "gfx_screen_config.h"

static SDL_Window* wnd;
static SDL_GLContext ctx;
static SDL_Renderer* renderer;
static int sdl_to_lus_table[512];
static bool vsync_enabled = true;
// OTRTODO: These are redundant. Info can be queried from SDL.
static int window_width = DESIRED_SCREEN_WIDTH;
static int window_height = DESIRED_SCREEN_HEIGHT;
/* saved PC settings (config.c) — used by gfx_sdl_init below + the mouse helpers further down. */
extern "C" { extern float g_cfg_mouse_sens; extern int g_cfg_mouse_invert, g_cfg_win_w, g_cfg_win_h; }
static uint32_t fullscreen_flag = SDL_WINDOW_FULLSCREEN_DESKTOP;
static bool fullscreen_state;
static bool maximized_state;
static bool is_running = true;
static void (*on_fullscreen_changed_callback)(bool is_now_fullscreen);
static SDL_GameController *g_sdl_controller = NULL;   /* PORT: opened in gfx_sdl_init */

static int target_fps = 120; // above 60 since vsync is enabled by default
static uint64_t previous_time;
static uint64_t qpc_freq;

#define FRAME_INTERVAL_US_NUMERATOR 1000000
#define FRAME_INTERVAL_US_DENOMINATOR (target_fps)

static int32_t gfx_sdl_get_maximized_state(void) {
    return (int32_t)maximized_state;
}

static int32_t gfx_sdl_get_fullscreen_state(void) {
    return (int32_t)fullscreen_state;
}

static int32_t gfx_sdl_get_fullscreen_flag_mode(void) {
    return fullscreen_flag == SDL_WINDOW_FULLSCREEN_DESKTOP ? 0 : 1;
}

static void gfx_sdl_set_fullscreen_flag(int32_t mode) {
    switch (mode) {
        case 0: {
            fullscreen_flag = SDL_WINDOW_FULLSCREEN_DESKTOP;
        } break;
        case 1: {
            fullscreen_flag = SDL_WINDOW_FULLSCREEN;
        } break;
    }
}

static void set_fullscreen(bool on, bool call_callback) {
    fullscreen_state = on;
    SDL_SetWindowFullscreen(wnd, on ? fullscreen_flag : 0);
    if (call_callback && on_fullscreen_changed_callback) {
        on_fullscreen_changed_callback(on);
    }
}

static void set_maximize_window(bool on) {
	maximized_state = on;
	if (on) {
		SDL_MaximizeWindow(wnd);
	} else {
		SDL_RestoreWindow (wnd);
	}
}

static void gfx_sdl_get_active_window_refresh_rate(uint32_t* refresh_rate) {
    int display_in_use = SDL_GetWindowDisplayIndex(wnd);

    SDL_DisplayMode mode;
    SDL_GetCurrentDisplayMode(display_in_use, &mode);
    *refresh_rate = mode.refresh_rate;
}

static void gfx_sdl_init(const struct GfxWindowInitSettings *set) {
    window_width = set->width;
    window_height = set->height;

    /* PC: default to the saved window size (1280x1024 out of the box) instead of a postage stamp on a 4K
     * panel; TUROK_WIN_W / TUROK_WIN_H override the saved config. (Native widescreen = later TODO.) */
    { const char *w = getenv("TUROK_WIN_W"), *h = getenv("TUROK_WIN_H");
      window_width  = (w && atoi(w) > 0) ? atoi(w) : (g_cfg_win_w > 0 ? g_cfg_win_w : 1280);
      window_height = (h && atoi(h) > 0) ? atoi(h) : (g_cfg_win_h > 0 ? g_cfg_win_h : 1024); }

#if defined(__linux__)
    /* On a Wayland session SDL2 prefers the Wayland video driver even when DISPLAY (XWayland)
     * is set, and its GL window creation NULL-derefs inside SDL_CreateWindow on some drivers
     * (segfault at gfx_sdl2.cpp:193). We launch against an X display (DISPLAY=:1 via XWayland),
     * so force the x11 driver unless the user explicitly chose one. x11 is verified working. */
    if (getenv("DISPLAY") && !getenv("SDL_VIDEODRIVER")) {
        setenv("SDL_VIDEODRIVER", "x11", 1);
    }
#endif

#ifdef SDL_HINT_VIDEO_HIGHDPI_DISABLED
    if (!set->allow_hidpi) {
        // HiDPI control, if available
        SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "1");
#if defined(PLATFORM_WIN32) && defined(SDL_HINT_WINDOWS_DPI_AWARENESS)
        // if HiDPI is disabled, declare ourselves DPI aware to get 1:1 window size on Windows
        SDL_SetHint(SDL_HINT_WINDOWS_DPI_AWARENESS, "permonitor");
#endif
    }
#endif

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        sysFatalError("Could not init SDL:\n%s", SDL_GetError());
    }

    /* PORT: open a game controller for input (keyboard always works via state poll). */
    if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) == 0) {
        for (int i = 0; i < SDL_NumJoysticks(); i++) {
            if (SDL_IsGameController(i)) { g_sdl_controller = SDL_GameControllerOpen(i); if (g_sdl_controller) break; }
        }
    }

    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    if (sysArgCheck("--debug-gl")) {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
    }

    int posX = set->x;
    int posY = set->y;
    int display_in_use = SDL_GetWindowDisplayIndex(wnd);
    if (display_in_use < 0) { // Fallback to default if out of bounds
        posX = SDL_WINDOWPOS_UNDEFINED;
        posY = SDL_WINDOWPOS_UNDEFINED;
    }

    if (set->centered) {
        SDL_DisplayMode mode = {};
        SDL_GetCurrentDisplayMode(0, &mode);
        posX = mode.w / 2 - window_width / 2;
        posY = mode.h / 2 - window_height / 2;
    }

    if (set->fullscreen_is_exclusive) {
        fullscreen_flag = SDL_WINDOW_FULLSCREEN;
    }

    // we will unhide the window once the GL context is successfully created
    Uint32 flags = SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL;

    // if fullscreen was requested, start the window in fullscreen right away
    if (set->fullscreen) {
        flags |= fullscreen_flag;
        fullscreen_state = true;
    }

    if (set->maximized) {
        flags |= SDL_WINDOW_MAXIMIZED;
        maximized_state = true;
    }

#ifdef SDL_WINDOW_ALLOW_HIGHDPI
    if (set->allow_hidpi) {
        flags |= SDL_WINDOW_ALLOW_HIGHDPI;
    }
#endif

    // ideally we need 3.0 compat
    // if that doesn't work, try 3.2 core in case we're on mac, 2.1 compat as a last resort
    static u32 glver[][3] = {
        { 0, 0, 0                                    }, // for command line override
        { 3, 0, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY }, // 3.0: default, has all the features required
        { 4, 1, SDL_GL_CONTEXT_PROFILE_CORE          }, // 4.1core: macs only have core profile and this is the latest
        { 3, 2, SDL_GL_CONTEXT_PROFILE_CORE          }, // 3.2core: older macs will only have this at best
        { 3, 0, SDL_GL_CONTEXT_PROFILE_ES            }, // es3: don't really support ES properly, but we can try
        { 2, 1, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY }, // 2.1: absolute last resort, will still require GLSL130 as an extension
    };

    u32 verstart = 1;
    const u32 verend = sizeof(glver) / sizeof(*glver);
    const char *verstr = sysArgGetString("--gl-version");
    if (verstr && *verstr) {
        // user override
        glver[0][2] = strstr(verstr, "core") ? SDL_GL_CONTEXT_PROFILE_CORE :
            (strstr(verstr, "es") ? SDL_GL_CONTEXT_PROFILE_ES :
            SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
        sscanf(verstr, "%d.%d", &glver[0][0], &glver[0][1]);
        if (glver[0][0] >= 1 && glver[0][0] <= 4 && glver[0][1] < 9) {
            verstart = 0;
        }
    }

    ctx = NULL;
    u32 vmin = 0, vmaj = 0, vprof = SDL_GL_CONTEXT_PROFILE_COMPATIBILITY;
    const char *vprofstr = "";
    for (u32 i = verstart; i < verend && !ctx; ++i) {
        vmaj = glver[i][0];
        vmin = glver[i][1];
        vprof = glver[i][2];
        vprofstr = (vprof == SDL_GL_CONTEXT_PROFILE_CORE ? "core" :
            (vprof == SDL_GL_CONTEXT_PROFILE_ES ? "es" : ""));

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, vmaj);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, vmin);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, vprof);

        wnd = SDL_CreateWindow(set->title, posX, posY, window_width, window_height, flags);
        if (!wnd) {
            sysLogPrintf(LOG_WARNING, "SDL: could not open SDL window for GL%d.%d%s:\n%s", vmaj, vmin, vprofstr, SDL_GetError());
            continue;
        }

        ctx = SDL_GL_CreateContext(wnd);
        if (!ctx) {
            sysLogPrintf(LOG_WARNING, "SDL: could not create GL%d.%d%s context: %s", vmaj, vmin, vprofstr, SDL_GetError());
            SDL_DestroyWindow(wnd);
            wnd = nullptr;
        }
    }

    if (!wnd || !ctx) {
        sysFatalError("Could not open SDL window with an OpenGL context of any supported version:\n%s", SDL_GetError());
    } else {
        sysLogPrintf(LOG_NOTE, "SDL: created GL%d.%d%s context", vmaj, vmin, vprofstr);
    }

    SDL_GL_MakeCurrent(wnd, ctx);
    SDL_GL_SetSwapInterval(1);   /* v-sync ON: render runs at the monitor refresh (e.g. 144Hz), no tearing */

    /* PORT: the CPU-side frame timer (target_fps, default 120) would cap the render BELOW a high-refresh
     * monitor. Honour TUROK_FPS so the render rate matches the rest of the port: TUROK_FPS=0 disables the
     * timer (render limited only by v-sync = the monitor refresh, ideal for the 30/60Hz-logic + interpolation
     * setup); TUROK_FPS=N caps the render at N. The game LOGIC rate is separate (TUROK_TICK_FPS). */
    { const char *fe = getenv("TUROK_FPS"); if (fe && *fe) target_fps = atoi(fe); }

    SDL_ShowWindow(wnd);

    qpc_freq = SDL_GetPerformanceFrequency();
}

static void gfx_sdl_close(void) {
    is_running = false;
}

static void gfx_sdl_set_fullscreen_changed_callback(void (*on_fullscreen_changed)(bool is_now_fullscreen)) {
    on_fullscreen_changed_callback = on_fullscreen_changed;
}

static void gfx_sdl_set_fullscreen(bool enable) {
    set_fullscreen(enable, true);
}

static void gfx_sdl_set_fullscreen_exclusive(bool enable) {
    const uint32_t newflag = enable ? SDL_WINDOW_FULLSCREEN : SDL_WINDOW_FULLSCREEN_DESKTOP;
    if (fullscreen_flag != newflag) {
        fullscreen_flag = newflag;
        // reset fullscreen to take new value into account if it already is in fullscreen
        if (fullscreen_state) {
            fullscreen_state = false;
            set_fullscreen(enable, true);
        }
    }
}

static void gfx_sdl_set_maximize_window(bool enable) {
    set_maximize_window(enable);
}

static void gfx_sdl_set_cursor_visibility(bool visible) {
    if (visible) {
        SDL_ShowCursor(SDL_ENABLE);
    } else {
        SDL_ShowCursor(SDL_DISABLE);
    }
}

static void get_centered_positions_native(int32_t width, int32_t height, int32_t *posX, int32_t *posY) {
    const int disp_idx = SDL_GetWindowDisplayIndex(wnd);
    SDL_DisplayMode mode = {};
    SDL_GetDesktopDisplayMode(disp_idx, &mode);
    *posX = mode.w / 2 - width / 2;
    *posY = mode.h / 2 - height / 2;
}

static void gfx_sdl_get_centered_positions(int32_t width, int32_t height, int32_t *posX, int32_t *posY) {
    const int disp_idx = SDL_GetWindowDisplayIndex(wnd);
    SDL_DisplayMode mode = {};
    SDL_GetCurrentDisplayMode(disp_idx, &mode);
    *posX = mode.w / 2 - width / 2;
    *posY = mode.h / 2 - height / 2;
}

static void gfx_sdl_set_closest_resolution(int32_t width, int32_t height, bool should_center) {
    const SDL_DisplayMode mode = {.w = width, .h = height};
    const int disp_idx = SDL_GetWindowDisplayIndex(wnd);
    SDL_DisplayMode closest = {};
    if (SDL_GetClosestDisplayMode(disp_idx, &mode, &closest)) {
        SDL_SetWindowDisplayMode(wnd, &closest);
        SDL_SetWindowSize(wnd, closest.w, closest.h);
        if (should_center) {
            int32_t posX = 0;
            int32_t posY = 0;
            get_centered_positions_native(closest.w, closest.h, &posX, &posY);
            SDL_SetWindowPosition(wnd, posX, posY);
        }
    }
}

static void gfx_sdl_set_dimensions(uint32_t width, uint32_t height, int32_t posX, int32_t posY) {
    SDL_SetWindowSize(wnd, width, height);
    SDL_SetWindowPosition(wnd, posX, posY);
}

static void gfx_sdl_get_dimensions(uint32_t* width, uint32_t* height, int32_t* posX, int32_t* posY) {
    SDL_GL_GetDrawableSize(wnd, static_cast<int*>((void*)width), static_cast<int*>((void*)height));
    SDL_GetWindowPosition(wnd, static_cast<int*>(posX), static_cast<int*>(posY));
}

/* ---- PORT: controller input (keyboard + gamepad -> N64 pad bits) ---------- */
/* inputSetState() is declared in "input.h" (included above). */

/* N64 controller bits (PR/os_cont.h) */
#define N64_A 0x8000u
#define N64_B 0x4000u
#define N64_Z 0x2000u
#define N64_START 0x1000u
#define N64_DU 0x0800u
#define N64_DD 0x0400u
#define N64_DL 0x0200u
#define N64_DR 0x0100u
#define N64_L  0x0020u
#define N64_R  0x0010u
#define N64_CU 0x0008u
#define N64_CD 0x0004u
#define N64_CL 0x0002u
#define N64_CR 0x0001u

static signed char axis_to_n64(int v) {            /* SDL axis -> N64 stick (-80..80) w/ deadzone */
    const int dz = 7000;
    if (v > -dz && v < dz) return 0;
    int s = (v * 80) / 32767;
    if (s > 80) s = 80; if (s < -80) s = -80;
    return (signed char)s;
}

/* PORT (PC FPS controls): mouse-look drives turn/pitch, WASD moves (A/D strafe), LMB fires, E toggles
 * run/walk. The walk flag is read by tmove.c (TUROK_WALKCAP). Sensitivity: TUROK_MOUSE_SENS (default 6).
 *
 * ★ The engine's DEFAULT control config is right-handed (m_RHControl=TRUE, options.c:205), where MOVEMENT
 * lives on the C-buttons (Forward=C-up, Back=C-down, SideStep=C-left/right) and the D-PAD is the native
 * run/walk + burst-swim TOGGLE (CTTYPE_SINGLE). So WASD must map to the C-BUTTONS ONLY — touching any D-pad
 * bit fires the engine's run/walk toggle on every keypress (the "W keeps toggling run/walk" bug). The N64
 * action map in this config: Z_TRIG=fire, R_TRIG=jump, L_TRIG=map, A=next-weapon, B=prev-weapon, Start=pause. */
extern "C" { extern int g_turok_walk_mode; extern float g_look_yaw, g_look_pitch; extern int g_weapon_cycle;
             extern int g_quicksave_req, g_quickload_req; }  /* g_cfg_* declared near the top of the file */
static int mouse_invert(void) {                          /* 0 = forward looks up (standard FPS); 1 = inverted */
    static int v = -2;                                   /* env TUROK_MOUSE_INVERT overrides the saved config */
    if (v == -2) { const char *e = getenv("TUROK_MOUSE_INVERT"); v = e ? atoi(e) : g_cfg_mouse_invert; }
    return v;
}
static float    g_mouse_dx = 0.0f, g_mouse_dy = 0.0f;   /* accumulated relative motion since last poll */
static unsigned g_mouse_buttons = 0;                    /* SDL_BUTTON_* mask */
static bool     g_relmouse_on = false;                  /* cursor captured for look */
static float mouse_sens(void) {
    static float s = -1.0f;                              /* env TUROK_MOUSE_SENS overrides the saved config */
    if (s < 0.0f) { const char *e = getenv("TUROK_MOUSE_SENS"); s = e ? (float)atof(e) : g_cfg_mouse_sens;
                    if (s <= 0.0f) s = g_cfg_mouse_sens > 0.0f ? g_cfg_mouse_sens : 6.0f; }
    return s;
}

static void turok_sdl_update_input(void) {
    unsigned short btn = 0;
    int sx = 0, sy = 0;
    const Uint8 *k = SDL_GetKeyboardState(NULL);

    /* PC FPS scheme (right-handed config): WASD = MOVE on the C-BUTTONS ONLY (W/S forward/back, A/D strafe);
     * the D-pad is the native run/walk toggle so it must stay clear. MOUSE-LOOK drives a HELD turn+pitch via
     * the g_look_* seam; LMB=fire, RMB=jump, wheel=cycle weapons, E toggles run/walk (event loop). */
    { static bool inited = false; if (!inited) { SDL_SetRelativeMouseMode(SDL_TRUE); g_relmouse_on = true; inited = true; } }

    if (k[SDL_SCANCODE_W]) btn |= N64_CU;   /* forward (C-up)        */
    if (k[SDL_SCANCODE_S]) btn |= N64_CD;   /* backward (C-down)     */
    if (k[SDL_SCANCODE_A]) btn |= N64_CL;   /* strafe left (C-left)  */
    if (k[SDL_SCANCODE_D]) btn |= N64_CR;   /* strafe right (C-right)*/

    /* mouse-look -> HELD body-turn (yaw) + held pitch via the port seam (g_look_*), NOT the spring stick
     * (which recenters on rest). Scaled to radians; TUROK_MOUSE_SENS tunes it. Pitch: forward(up)=look up
     * by default (standard FPS); TUROK_MOUSE_INVERT=1 flips it. */
    { float s = mouse_sens() * 0.0005f;
      float pitch_sign = mouse_invert() ? 1.0f : -1.0f;   /* SDL yrel +down; -1 => mouse-forward looks up */
      g_look_yaw   += g_mouse_dx * s;
      g_look_pitch += g_mouse_dy * s * pitch_sign;
      g_mouse_dx = g_mouse_dy = 0.0f; }

    /* arrow keys: turn/look fallback for no-mouse play (analog stick: x=turn, y=move). */
    if (k[SDL_SCANCODE_LEFT])  sx -= 80;
    if (k[SDL_SCANCODE_RIGHT]) sx += 80;
    if (k[SDL_SCANCODE_UP])    sy += 80;
    if (k[SDL_SCANCODE_DOWN])  sy -= 80;

    /* actions (right-handed map): LMB/Ctrl = fire (Z), RMB/Space = jump (R_TRIG), wheel = cycle weapons
     * (discrete g_weapon_cycle seam, NOT a held button), Tab/M = map (L_TRIG), Enter/Esc = pause (Start). */
    if (g_mouse_buttons & SDL_BUTTON(SDL_BUTTON_LEFT))  btn |= N64_Z;   /* fire */
    if (g_mouse_buttons & SDL_BUTTON(SDL_BUTTON_RIGHT)) btn |= N64_R;   /* jump (R_TRIG) */
    if (k[SDL_SCANCODE_SPACE])  btn |= N64_R;                           /* jump (kbd)  */
    if (k[SDL_SCANCODE_LCTRL] || k[SDL_SCANCODE_RCTRL]) btn |= N64_Z;   /* fire (kbd)  */
    if (k[SDL_SCANCODE_TAB] || k[SDL_SCANCODE_M]) btn |= N64_L;         /* map toggle (L_TRIG) */
    if (k[SDL_SCANCODE_RETURN] || k[SDL_SCANCODE_ESCAPE]) btn |= N64_START; /* pause */

    /* gamepad: left stick = move (Y fwd/back, X strafe via C-buttons); right stick = HELD look (g_look_*);
     * RT fire, A jump, shoulders cycle weapons (A/B button path, tick-gated), Start pause. D-pad -> C-buttons
     * (movement) NOT the raw D-pad, which would fire the native run/walk toggle. */
    if (g_sdl_controller) {
        SDL_GameController *c = g_sdl_controller;
        signed char gy = (signed char)(-(int)axis_to_n64(SDL_GameControllerGetAxis(c, SDL_CONTROLLER_AXIS_LEFTY)));
        if (gy) sy = gy;
        int lx = SDL_GameControllerGetAxis(c, SDL_CONTROLLER_AXIS_LEFTX);
        if (lx < -12000) btn |= N64_CL; else if (lx > 12000) btn |= N64_CR;   /* strafe */
        int rx = SDL_GameControllerGetAxis(c, SDL_CONTROLLER_AXIS_RIGHTX);
        int ry = SDL_GameControllerGetAxis(c, SDL_CONTROLLER_AXIS_RIGHTY);
        float gs = mouse_sens() * 0.00004f;
        if (rx < -8000 || rx > 8000) g_look_yaw   += (float)rx * gs;
        if (ry < -8000 || ry > 8000) g_look_pitch += (float)ry * gs * (mouse_invert() ? 1.0f : -1.0f);
        if (SDL_GameControllerGetAxis(c, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) > 8000) btn |= N64_Z;  /* fire */
        if (SDL_GameControllerGetButton(c, SDL_CONTROLLER_BUTTON_A))            btn |= N64_R;      /* jump */
        if (SDL_GameControllerGetButton(c, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER))btn |= N64_A;      /* next weapon */
        if (SDL_GameControllerGetButton(c, SDL_CONTROLLER_BUTTON_LEFTSHOULDER)) btn |= N64_B;      /* prev weapon */
        if (SDL_GameControllerGetButton(c, SDL_CONTROLLER_BUTTON_BACK))         btn |= N64_L;      /* map */
        if (SDL_GameControllerGetButton(c, SDL_CONTROLLER_BUTTON_START))        btn |= N64_START;
        if (SDL_GameControllerGetButton(c, SDL_CONTROLLER_BUTTON_DPAD_UP))      btn |= N64_CU;
        if (SDL_GameControllerGetButton(c, SDL_CONTROLLER_BUTTON_DPAD_DOWN))    btn |= N64_CD;
        if (SDL_GameControllerGetButton(c, SDL_CONTROLLER_BUTTON_DPAD_LEFT))    btn |= N64_CL;
        if (SDL_GameControllerGetButton(c, SDL_CONTROLLER_BUTTON_DPAD_RIGHT))   btn |= N64_CR;
    }

    if (sx > 80) sx = 80; if (sx < -80) sx = -80;
    if (sy > 80) sy = 80; if (sy < -80) sy = -80;
    inputSetState(btn, (signed char)sx, (signed char)sy);
}

static void gfx_sdl_handle_events(void) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_RETURN && (event.key.keysym.mod & KMOD_ALT)) {
                    // alt-enter received, switch fullscreen state
                    set_fullscreen(!fullscreen_state, true);
                } else if (event.key.keysym.sym == SDLK_e && !event.key.repeat) {
                    g_turok_walk_mode = !g_turok_walk_mode;                       /* run/walk toggle */
                } else if (event.key.keysym.sym == SDLK_F5 && !event.key.repeat) {
                    g_quicksave_req = 1;                                          /* quick-save (game thread) */
                } else if (event.key.keysym.sym == SDLK_F9 && !event.key.repeat) {
                    g_quickload_req = 1;                                          /* quick-load (game thread) */
                }
                break;
            case SDL_MOUSEMOTION:
                if (g_relmouse_on) { g_mouse_dx += (float)event.motion.xrel; g_mouse_dy += (float)event.motion.yrel; }
                break;
            case SDL_MOUSEBUTTONDOWN:
                g_mouse_buttons |= SDL_BUTTON(event.button.button);
                break;
            case SDL_MOUSEBUTTONUP:
                g_mouse_buttons &= ~SDL_BUTTON(event.button.button);
                break;
            case SDL_MOUSEWHEEL:
                g_weapon_cycle += event.wheel.y;                                 /* +up = next weapon, -down = prev (one per tick) */
                if (g_weapon_cycle >  12) g_weapon_cycle =  12;                  /* cap the backlog (< #weapons) so scrolling */
                if (g_weapon_cycle < -12) g_weapon_cycle = -12;                  /* while ducking/paused can't dump a huge run  */
                break;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                    SDL_GL_GetDrawableSize(wnd, &window_width, &window_height);
                    if (!fullscreen_state) {
                        maximized_state = SDL_GetWindowFlags(wnd) & SDL_WINDOW_MAXIMIZED ? true : false;
                    }
                } else if (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
                    SDL_SetRelativeMouseMode(SDL_FALSE); g_relmouse_on = false;   /* Alt-Tab away: release cursor to OS */
                } else if (event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
                    SDL_SetRelativeMouseMode(SDL_TRUE);  g_relmouse_on = true;    /* refocus: re-grab the cursor */
                } else if (event.window.event == SDL_WINDOWEVENT_CLOSE &&
                           event.window.windowID == SDL_GetWindowID(wnd)) {
                    // We listen specifically for main window close because closing main window
                    // on macOS does not trigger SDL_Quit.
                    exit(0);
                }
                break;
            case SDL_QUIT:
                exit(0);
                break;
        }
    }
    turok_sdl_update_input();   /* PORT: map keyboard+gamepad -> N64 pad each frame */
}

static bool gfx_sdl_start_frame(void) {
    return true;
}

static uint64_t qpc_to_100ns(uint64_t qpc) {
    return qpc / qpc_freq * 10000000 + qpc % qpc_freq * 10000000 / qpc_freq;
}

static inline void sync_framerate_with_timer(void) {
    uint64_t t;
    t = qpc_to_100ns(SDL_GetPerformanceCounter());

    const int64_t next = previous_time + 10 * FRAME_INTERVAL_US_NUMERATOR / FRAME_INTERVAL_US_DENOMINATOR;
    int64_t left = next - t;
    // We want to exit a bit early, so we can busy-wait the rest to never miss the deadline
    left -= 15000UL;
    if (left > 0) {
        sysSleep(left);
    }

    do {
        sysCpuRelax();
        t = qpc_to_100ns(SDL_GetPerformanceCounter());
    } while ((int64_t)t < next);

    t = qpc_to_100ns(SDL_GetPerformanceCounter());
    if (left > 0 && t - next < 10000) {
        // In case it takes some time for the application to wake up after sleep,
        // or inaccurate timer,
        // don't let that slow down the framerate.
        t = next;
    }
    previous_time = t;
}

static void gfx_sdl_swap_buffers_begin(void) {
    if (target_fps) {
        sync_framerate_with_timer();
    }
    SDL_GL_SwapWindow(wnd);
}

static void gfx_sdl_swap_buffers_end(void) {

}

static double gfx_sdl_get_time(void) {
    return SDL_GetPerformanceCounter() / (double)qpc_freq;
}

static int32_t gfx_sdl_get_target_fps(void) {
    return target_fps;
}

static void gfx_sdl_set_target_fps(int fps) {
    target_fps = fps;
}

static bool gfx_sdl_can_disable_vsync(void) {
    return true;
}

static void *gfx_sdl_get_window_handle(void) {
    return (void *)wnd;
}

static void gfx_sdl_set_window_title(const char *title) {
    SDL_SetWindowTitle(wnd, title);
}

static int gfx_sdl_get_swap_interval(void) {
    return SDL_GL_GetSwapInterval();
}

static bool gfx_sdl_set_swap_interval(int interval) {
    const bool success = SDL_GL_SetSwapInterval(interval) >= 0;
    vsync_enabled = success && (interval != 0);
    if (!success) {
        sysLogPrintf(LOG_WARNING, "SDL: failed to set vsync %d: %s", interval, SDL_GetError());
    }
    return success;
}

int gfx_sdl_get_display_mode(int modenum, int *out_w, int *out_h) {
    const int display_in_use = SDL_GetWindowDisplayIndex(wnd);
    SDL_DisplayMode sdlmode;
    if (SDL_GetDisplayMode(display_in_use, modenum, &sdlmode) == 0) {
        *out_w = sdlmode.w;
        *out_h = sdlmode.h;
        return 1;
    }
    return 0;
}

int gfx_sdl_get_current_display_mode(int *out_w, int *out_h) {
    const int display_in_use = SDL_GetWindowDisplayIndex(wnd);
    SDL_DisplayMode sdlmode;
    if (SDL_GetCurrentDisplayMode(display_in_use, &sdlmode) == 0) {
        *out_w = sdlmode.w;
        *out_h = sdlmode.h;
        return 1;
    }
    return 0;
}

int gfx_sdl_get_num_display_modes(void) {
    const int display_in_use = SDL_GetWindowDisplayIndex(wnd);
    return SDL_GetNumDisplayModes(display_in_use);
}

struct GfxWindowManagerAPI gfx_sdl = {
    gfx_sdl_init,
    gfx_sdl_close,
    gfx_sdl_get_display_mode,
    gfx_sdl_get_current_display_mode,
    gfx_sdl_get_num_display_modes,
    gfx_sdl_get_fullscreen_state,
    gfx_sdl_set_fullscreen_changed_callback,
    gfx_sdl_set_fullscreen,
    gfx_sdl_set_fullscreen_exclusive,
    gfx_sdl_set_fullscreen_flag,
    gfx_sdl_get_fullscreen_flag_mode,
    gfx_sdl_get_maximized_state,
    gfx_sdl_set_maximize_window,
    gfx_sdl_get_active_window_refresh_rate,
    gfx_sdl_set_cursor_visibility,
    gfx_sdl_set_closest_resolution,
    gfx_sdl_set_dimensions,
    gfx_sdl_get_dimensions,
    gfx_sdl_get_centered_positions,
    gfx_sdl_handle_events,
    gfx_sdl_start_frame,
    gfx_sdl_swap_buffers_begin,
    gfx_sdl_swap_buffers_end,
    gfx_sdl_get_time,
    gfx_sdl_get_target_fps,
    gfx_sdl_set_target_fps,
    gfx_sdl_can_disable_vsync,
    gfx_sdl_get_window_handle,
    gfx_sdl_set_window_title,
    gfx_sdl_get_swap_interval,
    gfx_sdl_set_swap_interval,
};
