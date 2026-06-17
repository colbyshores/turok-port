/* turok_gfx.c — C-linkage bridge: port video seam -> Fast3D (gfx_pc).
 *
 * gfx_pc.cpp exposes gfx_init/gfx_run/gfx_start_frame/gfx_end_frame as extern "C",
 * so this C TU binds to them directly. For the PC ground-truth build we drive the
 * OSMesa window-manager + the OpenGL rendering API (headless, frame-capturable).
 *
 * The seam (see os_shim.c):
 *   - osSpTaskLoad(M_GFXTASK)  -> turokGfxRun(task->data_ptr)   [interpret the display list]
 *   - osViSwapBuffer()         -> turokGfxEndFrame()+present, then turokGfxStartFrame() for next
 * See ../../CLAUDE.md M2.
 */
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <PR/gbi.h>          /* Gfx */
#include "gfx_api.h"         /* GfxInitSettings + gfx_init/gfx_run/... */
#if defined(GFX_USE_EGL)
#include "gfx_egl.h"         /* surfaceless EGL (hardware GL, headless) — WIP: hangs at init */
#define TUROK_WM         gfx_egl_wm
#define TUROK_SAVEPNG(p) gfx_egl_save_png(p)
#elif defined(GFX_USE_SDL2)
#include "gfx_sdl.h"         /* SDL2 hidden-window hardware GL (fast, via the X display) */
extern int gfx_glreadpixels_png(const char *path, int w, int h);
#define TUROK_WM         gfx_sdl
#define TUROK_SAVEPNG(p) gfx_glreadpixels_png(p, s_w, s_h)
#else
#include "gfx_osmesa.h"      /* software OSMesa (slow but works, default) */
#define TUROK_WM         gfx_osmesa_wm
#define TUROK_SAVEPNG(p) gfx_osmesa_save_png(p)
#endif

extern struct GfxWindowManagerAPI TUROK_WM;        /* gfx_sdl2 / gfx_egl / gfx_osmesa */
extern struct GfxRenderingAPI     gfx_opengl_api;  /* gfx_opengl.cpp */
/* gfx_framebuffers_enabled is declared in gfx_api.h (bool) */

static int s_inited = 0;
static int s_w = 320, s_h = 240;      /* N64 hi-res-ish; Turok renders 320x240 */
static int s_frame_open = 0;          /* a gfx frame is in progress (start..end) */

void turokGfxInit(int w, int h)
{
    struct GfxInitSettings settings;
    if (s_inited) return;
    if (w > 0) s_w = w;
    if (h > 0) s_h = h;

    for (size_t i = 0; i < sizeof(settings); i++) ((char*)&settings)[i] = 0;
    settings.wapi = &TUROK_WM;
    settings.rapi = &gfx_opengl_api;
    settings.window_settings.title = "turok";
    settings.window_settings.width = (uint32_t)s_w;
    settings.window_settings.height = (uint32_t)s_h;

    gfx_init(&settings);
    /* render straight to the default (captured) framebuffer — no FBO indirection. The
     * game's framebuffer-effect draws (G_SETCIMAGE to cfb) otherwise land in an FBO that
     * isn't blitted to the OSMesa buffer, giving a black capture. */
    gfx_framebuffers_enabled = 0;
    s_inited = 1;
    gfx_start_frame();        /* open the first frame so the first gfx task has a frame */
    s_frame_open = 1;
}

/* Interpret one N64 display list (the gfx task's data_ptr) through Fast3D. */
void turokGfxRun(void *dl)
{
    if (!s_inited || !dl) return;
    if (!s_frame_open) { gfx_start_frame(); s_frame_open = 1; }
    gfx_run((Gfx *)dl);
}

/* Finish + present the current frame (osViSwapBuffer). Caller may capture between
 * this and turokGfxStartFrame(). */
extern unsigned long g_turok_tris;     /* size_t in gfx_pc; per-frame triangle count */
static int s_frame_no = 0;             /* RENDER frame counter (real presents only) */
static int s_cap_render = -2;          /* TUROK_CAPTURE_FRAME = which RENDER frame to PNG */
void turokGfxEndFrame(void)
{
    if (!s_inited) return;
    if (s_frame_open) { gfx_end_frame(); s_frame_open = 0; }   /* buffer now holds this render frame */

    if (s_cap_render == -2) { const char *e = getenv("TUROK_CAPTURE_FRAME"); s_cap_render = e ? atoi(e) : -1; }
    /* Capture keys off REAL render frames (not the osRecvMesg frame-pump ticks). */
    if (s_cap_render >= 0 && s_frame_no == s_cap_render) {
        const char *p = getenv("TUROK_CAPTURE_PATH"); if (!p) p = "turok_frame.png";
        int rc = turokGfxSavePng(p);
        fprintf(stderr, "[gfx] captured RENDER frame %d -> %s (rc=%d)\n", s_frame_no, p, rc);
        exit(0);   /* got the requested render frame — done */
    }
    g_turok_tris = 0;
    s_frame_no++;
}

/* Open the next frame. */
void turokGfxStartFrame(void)
{
    if (!s_inited) return;
    if (!s_frame_open) { gfx_start_frame(); s_frame_open = 1; }
}

int turokGfxSavePng(const char *path)
{
    if (!s_inited) return -1;
    return TUROK_SAVEPNG(path);
}

int turokGfxInited(void) { return s_inited; }
