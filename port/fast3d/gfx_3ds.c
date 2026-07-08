/*
 * gfx_3ds.c — Perfect Dark window manager for the Nintendo 3DS (libctru +
 * Citro3D). Implements `struct GfxWindowManagerAPI`, replacing gfx_sdl2.cpp.
 *
 * Owns C3D init, the three render targets (top-left, top-right for stereo, and
 * the bottom screen), the 3D-slider read, the APT main-loop / event handling,
 * and frame timing. The actual C3D_FrameBegin/End live in the renderer
 * (gfx_citro3d.cpp) which performs the per-eye replay; the swap hooks here are
 * intentionally no-ops. See CLAUDE.md §6.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <3ds.h>
#include <citro3d.h>

#include "gfx_window_manager_api.h"
#include "gfx_3ds.h"
#include "turok_trace.h"

#define DISPLAY_TRANSFER_FLAGS \
    (GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) | GX_TRANSFER_RAW_COPY(0) | \
     GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) | \
     GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))

static C3D_RenderTarget *sTopLeft  = NULL;
static C3D_RenderTarget *sTopRight = NULL;
static C3D_RenderTarget *sBottom   = NULL;

static bool  sStereoActive = false;
static float sStereoLevel  = 0.0f;
static bool  sStereoEnabled = true;   // master toggle (config/arg)

static int   sTargetFps = 60;

// ── Bottom-screen backlight (battery) ──────────────────────────────────────
// The bottom screen is UNUSED (Turok's HUD is all top-screen; the bottom target is only cleared to black),
// so its backlight is wasted power. Turn it off (turok.cfg bottom_backlight, default 0 = off). ★ But whenever
// we LEAVE the game — HOME menu (ONSUSPEND) or sleep (ONSLEEP) — power it back ON so the HOME menu / other
// titles have a lit bottom screen; then re-apply our preference on return (ONRESTORE/ONWAKEUP). Without the
// ONSUSPEND restore the HOME menu's bottom screen stays dark. gfx_3ds had no other APT hook, so this is it.
static aptHookCookie s_lcd_hook;
static void lcd_set_bottom(int on) {
    if (R_SUCCEEDED(gspLcdInit())) {                       // transient gsp::Lcd session (standard idiom)
        if (on) GSPLCD_PowerOnBacklight(GSPLCD_SCREEN_BOTTOM);
        else    GSPLCD_PowerOffBacklight(GSPLCD_SCREEN_BOTTOM);
        gspLcdExit();
    }
}
static void lcd_backlight_apply(void) {
    extern int g_cfg_bottom_backlight;                     // 1 = keep lit, 0 = off (save battery)
    lcd_set_bottom(g_cfg_bottom_backlight);
}
// Live re-apply from the options menu when the player toggles bottom_backlight (both directions).
void turok3dsRefreshBottomBacklight(void) { lcd_backlight_apply(); }
static void lcd_apt_hook(APT_HookType hook, void *param) {
    (void)param;
    if (hook == APTHOOK_ONSUSPEND || hook == APTHOOK_ONSLEEP)    // HOME menu / sleep → give the bottom screen back
        lcd_set_bottom(1);
    else if (hook == APTHOOK_ONRESTORE || hook == APTHOOK_ONWAKEUP)  // back in-game → our preference (off by default)
        lcd_backlight_apply();
}

// ---------------------------------------------------------------------------

void *gfx3dsTopTarget(int eye) {
    return eye == 0 ? sTopLeft : sTopRight;
}
void *gfx3dsBottomTarget(void) {
    return sBottom;
}
float gfx3dsStereoLevel(void) {
    return sStereoLevel;
}
int gfx3dsStereoActive(void) {
    return sStereoActive ? 1 : 0;
}

// ---------------------------------------------------------------------------

static void gfx_3ds_init(const struct GfxWindowInitSettings *settings) {
    (void)settings;

    // Run the New 3DS at full clock + extra L2 cache if available (no-op on O3DS).
    osSetSpeedupEnable(true);

    // gfxInit(..., true) puts the screen framebuffers in VRAM (0x1F000000+),
    // NOT at the start of the linear heap. gfxInitDefault() uses the linear heap
    // WITHOUT registering the buffers with linearAlloc's bookkeeping, so
    // citro3d's linearAlloc calls (13 sites in gfx_citro3d) return overlapping
    // addresses and the GPU's display transfer overwrites them with the screen
    // image — a hardware-only corruption (emulators don't model the overlap).
    // (Rosetta: the Forsaken 3DS port hit and fixed exactly this.)
    gfxInit(GSP_BGR8_OES, GSP_BGR8_OES, true);
    gfxSet3D(true); // enable the stereo display; per-frame we toggle via gfxSet3D
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE * 4);

    // Top screen GPU framebuffers are 240 wide x 400 tall (portrait). We render
    // rotated (see the vertex-shader transform). Depth+stencil per target.
    sTopLeft  = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
    sTopRight = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
    sBottom   = C3D_RenderTargetCreate(240, 320, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);

    C3D_RenderTargetSetOutput(sTopLeft,  GFX_TOP,    GFX_LEFT,  DISPLAY_TRANSFER_FLAGS);
    C3D_RenderTargetSetOutput(sTopRight, GFX_TOP,    GFX_RIGHT, DISPLAY_TRANSFER_FLAGS);
    C3D_RenderTargetSetOutput(sBottom,   GFX_BOTTOM, GFX_LEFT,  DISPLAY_TRANSFER_FLAGS);

    BK_TR(BK_TR_TARGET, "RTs created topL=%p topR=%p bottom=%p%s",
          (void *)sTopLeft, (void *)sTopRight, (void *)sBottom,
          (!sTopLeft || !sTopRight || !sBottom) ? " NULL!" : "");

    // Power off the unused bottom-screen backlight (battery), and keep it off across sleep/wake.
    lcd_backlight_apply();
    aptHook(&s_lcd_hook, lcd_apt_hook, NULL);
}

static void gfx_3ds_close(void) {
    aptUnhook(&s_lcd_hook);   // stop re-asserting; HOME/Luma re-lights both panels on exit
    C3D_Fini();
    gfxExit();
}

static int gfx_3ds_get_display_mode(int modenum, int *w, int *h) {
    (void)modenum;
    *w = 400; *h = 240;
    return 1;
}
static int gfx_3ds_get_current_display_mode(int *w, int *h) {
    *w = 400; *h = 240;
    return 1;
}
static int gfx_3ds_get_num_display_modes(void) { return 1; }

static int32_t gfx_3ds_get_fullscreen_state(void) { return 1; }
static void gfx_3ds_set_fullscreen_changed_callback(void (*cb)(bool)) { (void)cb; }
static void gfx_3ds_set_fullscreen(bool enable) { (void)enable; }
static void gfx_3ds_set_fullscreen_exclusive(bool exc) { (void)exc; }
static void gfx_3ds_set_fullscreen_flag(int32_t mode) { (void)mode; }
static int32_t gfx_3ds_get_fullscreen_flag_mode(void) { return 1; }
static int32_t gfx_3ds_get_maximized_state(void) { return 0; }
static void gfx_3ds_set_maximize(bool enable) { (void)enable; }
static void gfx_3ds_get_active_window_refresh_rate(uint32_t *rr) { if (rr) *rr = 60; }
static void gfx_3ds_set_cursor_visibility(bool v) { (void)v; }
static void gfx_3ds_set_closest_resolution(int32_t w, int32_t h, bool c) { (void)w; (void)h; (void)c; }
static void gfx_3ds_set_dimensions(uint32_t w, uint32_t h, int32_t x, int32_t y) { (void)w; (void)h; (void)x; (void)y; }
static void gfx_3ds_get_dimensions(uint32_t *w, uint32_t *h, int32_t *x, int32_t *y) {
    if (w) *w = 400; if (h) *h = 240; if (x) *x = 0; if (y) *y = 0;
}
static void gfx_3ds_get_centered_positions(int32_t w, int32_t h, int32_t *x, int32_t *y) {
    (void)w; (void)h; if (x) *x = 0; if (y) *y = 0;
}

static void gfx_3ds_handle_events(void) {
    if (!aptMainLoop()) {
        // HOME-menu close / power → return to the homebrew menu. videoShutdown() does NOT call
        // wmAPI->close, so C3D/gfx were never torn down on exit: the GSP event thread kept running and
        // faulted (gspWaitForAnyEvent → syncArbitrateAddress) when svcExitProcess unmapped its stack —
        // the quit crash (Luma data-abort, far≈sp). The standard teardown (C3D_Fini + gfxExit) stops the
        // GSP thread cleanly before exit. (Do NOT C3D_FrameSync here — after HOME-close no new frame is
        // submitted, so it blocks forever → the "closing software" hang.)
        extern void audioThreadStop(void);  // join the audio worker (no-op if PD_AUDIO_THREAD=0); it is
        audioThreadStop();                  // JOINABLE, so left running it would also crash on svcExitProcess
        gfx_3ds_close();   // C3D_Fini() + gfxExit() — joins/stops the GSP event thread before we exit
        exit(0);
    }

    { extern void input3dsScan(void); input3dsScan(); }   // poll HID -> the inputSetState seam each frame

    // 3D slider: osGet3DSliderState() returns garbage in Mandarine, so clamp.
    float s = osGet3DSliderState();
    if (s < 0.0f || s > 1.0f || s != s /* NaN */) s = 0.0f;
    sStereoLevel = s;
    sStereoActive = sStereoEnabled && (s > 0.0f) && (sTopRight != NULL);

    // Toggle the parallax barrier each frame: off when mono so we only draw the
    // left target once (the single-pass fast path), on when the slider is up.
    gfxSet3D(sStereoActive);
}

static bool gfx_3ds_start_frame(void) {
    return true;
}
static void gfx_3ds_swap_buffers_begin(void) {} // present is C3D_FrameEnd in the renderer
static void gfx_3ds_swap_buffers_end(void) {}

static double gfx_3ds_get_time(void) {
    // Use the CPU tick directly rather than osGetTime() — PD's libultra defines
    // its own osGetTime (which wins at link), so its units aren't libctru's ms.
    // Only used for the FPS-counter delta, so the exact epoch doesn't matter.
    return svcGetSystemTick() / (double)SYSCLOCK_ARM11; // seconds
}
static int32_t gfx_3ds_get_target_fps(void) { return sTargetFps; }
static void gfx_3ds_set_target_fps(int fps) { sTargetFps = fps; }
static bool gfx_3ds_can_disable_vsync(void) { return false; }
static void *gfx_3ds_get_window_handle(void) { return (void *)sTopLeft; }
static void gfx_3ds_set_window_title(const char *title) { (void)title; }
static int gfx_3ds_get_swap_interval(void) { return 1; }
static bool gfx_3ds_set_swap_interval(int interval) { (void)interval; return true; }

// ---------------------------------------------------------------------------

struct GfxWindowManagerAPI gfx_3ds = {
    gfx_3ds_init,
    gfx_3ds_close,
    gfx_3ds_get_display_mode,
    gfx_3ds_get_current_display_mode,
    gfx_3ds_get_num_display_modes,
    gfx_3ds_get_fullscreen_state,
    gfx_3ds_set_fullscreen_changed_callback,
    gfx_3ds_set_fullscreen,
    gfx_3ds_set_fullscreen_exclusive,
    gfx_3ds_set_fullscreen_flag,
    gfx_3ds_get_fullscreen_flag_mode,
    gfx_3ds_get_maximized_state,
    gfx_3ds_set_maximize,
    gfx_3ds_get_active_window_refresh_rate,
    gfx_3ds_set_cursor_visibility,
    gfx_3ds_set_closest_resolution,
    gfx_3ds_set_dimensions,
    gfx_3ds_get_dimensions,
    gfx_3ds_get_centered_positions,
    gfx_3ds_handle_events,
    gfx_3ds_start_frame,
    gfx_3ds_swap_buffers_begin,
    gfx_3ds_swap_buffers_end,
    gfx_3ds_get_time,
    gfx_3ds_get_target_fps,
    gfx_3ds_set_target_fps,
    gfx_3ds_can_disable_vsync,
    gfx_3ds_get_window_handle,
    gfx_3ds_set_window_title,
    gfx_3ds_get_swap_interval,
    gfx_3ds_set_swap_interval,
};
