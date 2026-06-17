/*
 * Headless HARDWARE-GL window-manager backend for Fast3D.
 *
 * Surfaceless EGL (EGL_PLATFORM_SURFACELESS_MESA / EGL_DEFAULT_DISPLAY) gives a real
 * GPU OpenGL context with no display server. There is no default framebuffer 0 in a
 * surfaceless context, so we render into an FBO (RGBA8 colour renderbuffer + packed
 * depth/stencil) and glReadPixels it for capture. ~100x faster than software OSMesa for
 * the texrect-heavy 2D screens. See ../../CLAUDE.md M2.
 *
 * The FBO is created lazily on the first start_frame — by then gfx_opengl's init has run
 * gladLoadGLLoader (via eglGetProcAddress), so the GL entry points exist.
 */
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <gbm.h>
#include "glad/glad.h"
#include "gfx_egl.h"

#ifndef EGL_PLATFORM_GBM_KHR
#define EGL_PLATFORM_GBM_KHR 0x31D7
#endif
#ifndef EGL_CONTEXT_OPENGL_PROFILE_MASK
#define EGL_CONTEXT_OPENGL_PROFILE_MASK 0x30FD
#endif
#ifndef EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT
#define EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT 0x00000002
#endif

static EGLDisplay s_dpy = EGL_NO_DISPLAY;
static EGLContext s_ctx = EGL_NO_CONTEXT;
static int        s_w = 0, s_h = 0;
static int               s_drm_fd = -1;       /* render node fd (GBM path) */
static struct gbm_device *s_gbm    = nullptr; /* GBM device (GBM path) */

static GLuint     s_fbo = 0, s_color_rb = 0, s_depth_rb = 0;
static uint8_t   *s_buf = nullptr;          /* glReadPixels target for capture */

static void egl_make_fbo(void) {
    if (s_fbo) return;
    glGenFramebuffers(1, &s_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, s_fbo);

    glGenRenderbuffers(1, &s_color_rb);
    glBindRenderbuffer(GL_RENDERBUFFER, s_color_rb);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, s_w, s_h);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, s_color_rb);

    glGenRenderbuffers(1, &s_depth_rb);
    glBindRenderbuffer(GL_RENDERBUFFER, s_depth_rb);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, s_w, s_h);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, s_depth_rb);

    GLenum st = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (st != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "gfx_egl: FBO incomplete (0x%x)\n", st); abort();
    }
    glViewport(0, 0, s_w, s_h);
    s_buf = (uint8_t *)calloc((size_t)s_w * s_h, 4);
    fprintf(stderr, "gfx_egl: %dx%d FBO ready (hardware GL)\n", s_w, s_h);
}

/* ------------------------------------------------------------------ WM impl */

static void egl_init(const struct GfxWindowInitSettings *set) {
    s_w = (int)set->width ? (int)set->width : 320;
    s_h = (int)set->height ? (int)set->height : 240;

    PFNEGLGETPLATFORMDISPLAYEXTPROC getPlatformDisplay =
        (PFNEGLGETPLATFORMDISPLAYEXTPROC)eglGetProcAddress("eglGetPlatformDisplayEXT");

    /* Preferred path: GBM on an explicit DRM render node. This forces the real hardware
     * driver (radeonsi here) with NO display server, so it sidesteps both the surfaceless
     * eglInitialize hang AND any X-server contention. Surfaceless/default is the fallback. */
    const char *force_sl = getenv("TUROK_EGL_SURFACELESS");
    if (!(force_sl && *force_sl == '1') && getPlatformDisplay) {
        const char *node = getenv("TUROK_DRI_NODE");
        const char *cands[3]; int nc = 0;
        if (node && *node) cands[nc++] = node;
        cands[nc++] = "/dev/dri/renderD128";
        cands[nc++] = "/dev/dri/renderD129";
        for (int i = 0; i < nc && s_dpy == EGL_NO_DISPLAY; i++) {
            int fd = open(cands[i], O_RDWR | O_CLOEXEC);
            if (fd < 0) continue;
            struct gbm_device *gbm = gbm_create_device(fd);
            if (!gbm) { close(fd); continue; }
            EGLDisplay dpy = getPlatformDisplay(EGL_PLATFORM_GBM_KHR, gbm, nullptr);
            EGLint mj = 0, mn = 0;
            if (dpy != EGL_NO_DISPLAY && eglInitialize(dpy, &mj, &mn)) {
                s_dpy = dpy; s_gbm = gbm; s_drm_fd = fd;
                fprintf(stderr, "gfx_egl: GBM/EGL %d.%d on %s (hardware, headless)\n", mj, mn, cands[i]);
            } else {
                if (gbm) gbm_device_destroy(gbm);
                close(fd);
            }
        }
    }

    if (s_dpy == EGL_NO_DISPLAY) {
        if (getPlatformDisplay)
            s_dpy = getPlatformDisplay(EGL_PLATFORM_SURFACELESS_MESA, EGL_DEFAULT_DISPLAY, nullptr);
        if (s_dpy == EGL_NO_DISPLAY)
            s_dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        if (s_dpy == EGL_NO_DISPLAY) { fprintf(stderr, "gfx_egl: no EGL display\n"); abort(); }
        EGLint major0 = 0, minor0 = 0;
        if (!eglInitialize(s_dpy, &major0, &minor0)) { fprintf(stderr, "gfx_egl: eglInitialize failed\n"); abort(); }
        fprintf(stderr, "gfx_egl: surfaceless/default EGL %d.%d (fallback)\n", major0, minor0);
    }

    const EGLint cfg_attr[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 24,
        EGL_NONE
    };
    EGLConfig config; EGLint n = 0;
    if (!eglChooseConfig(s_dpy, cfg_attr, &config, 1, &n) || n < 1) {
        fprintf(stderr, "gfx_egl: eglChooseConfig found no config\n"); abort();
    }
    if (!eglBindAPI(EGL_OPENGL_API)) { fprintf(stderr, "gfx_egl: eglBindAPI failed\n"); abort(); }

    /* Request a COMPATIBILITY profile: gfx_opengl assumes a compat desktop context for the
     * headless path (GLSL 130 + the default VAO). A core context (what radeonsi may hand back
     * for 3.3 with no profile mask) has no default VAO, so glDrawArrays silently renders nothing
     * — the "draws everything but the screen stays black" bug. */
    const EGLint ctx_attr[] = {
        EGL_CONTEXT_MAJOR_VERSION, 3, EGL_CONTEXT_MINOR_VERSION, 1,
        EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT,
        EGL_NONE
    };
    s_ctx = eglCreateContext(s_dpy, config, EGL_NO_CONTEXT, ctx_attr);
    if (s_ctx == EGL_NO_CONTEXT) {  /* retry: compat with no explicit version */
        const EGLint ctx_attr2[] = {
            EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT, EGL_NONE };
        s_ctx = eglCreateContext(s_dpy, config, EGL_NO_CONTEXT, ctx_attr2);
    }
    if (s_ctx == EGL_NO_CONTEXT) {  /* last resort: no explicit attributes */
        s_ctx = eglCreateContext(s_dpy, config, EGL_NO_CONTEXT, nullptr);
    }
    if (s_ctx == EGL_NO_CONTEXT) { fprintf(stderr, "gfx_egl: eglCreateContext failed\n"); abort(); }

    if (!eglMakeCurrent(s_dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, s_ctx)) {
        fprintf(stderr, "gfx_egl: eglMakeCurrent (surfaceless) failed 0x%x\n", eglGetError()); abort();
    }
    fprintf(stderr, "gfx_egl: context current (%s)\n", s_gbm ? "GBM/hardware" : "surfaceless");
    /* FBO is created lazily in start_frame, after gfx_opengl loaded GL via glad. */
}

static void egl_close(void) {
    if (s_dpy != EGL_NO_DISPLAY) {
        eglMakeCurrent(s_dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (s_ctx != EGL_NO_CONTEXT) eglDestroyContext(s_dpy, s_ctx);
        eglTerminate(s_dpy);
    }
    if (s_gbm) { gbm_device_destroy(s_gbm); s_gbm = nullptr; }
    if (s_drm_fd >= 0) { close(s_drm_fd); s_drm_fd = -1; }
    free(s_buf); s_buf = nullptr;
}

static int  egl_get_display_mode(int n, int *w, int *h) { (void)n; *w = s_w; *h = s_h; return 1; }
static int  egl_get_current_display_mode(int *w, int *h) { *w = s_w; *h = s_h; return 1; }
static int  egl_get_num_display_modes(void) { return 1; }
static int32_t egl_zero32(void) { return 0; }
static void egl_set_fs_changed_cb(void (*cb)(bool)) { (void)cb; }
static void egl_set_bool(bool b) { (void)b; }
static void egl_set_i32(int32_t m) { (void)m; }
static int32_t egl_get_fs_flag_mode(void) { return 0; }
static void egl_get_refresh_rate(uint32_t *r) { *r = 30; }
static void egl_set_closest_res(int32_t w, int32_t h, bool c) { (void)w; (void)h; (void)c; }
static void egl_set_dimensions(uint32_t w, uint32_t h, int32_t x, int32_t y) { (void)w;(void)h;(void)x;(void)y; }
static void egl_get_dimensions(uint32_t *w, uint32_t *h, int32_t *x, int32_t *y) {
    *w = (uint32_t)s_w; *h = (uint32_t)s_h; if (x) *x = 0; if (y) *y = 0;
}
static void egl_get_centered(int32_t w, int32_t h, int32_t *x, int32_t *y) { (void)w;(void)h; *x = 0; *y = 0; }
static void egl_handle_events(void) {}
static bool egl_start_frame(void) {
    egl_make_fbo();                                   /* lazy: GL is loaded by now */
    glBindFramebuffer(GL_FRAMEBUFFER, s_fbo);         /* keep our FBO the render target */
    glViewport(0, 0, s_w, s_h);
    return true;
}
static void egl_noop(void) {}
static double egl_get_time(void) { return 0.0; }
static int32_t egl_get_target_fps(void) { return 30; }
static void egl_set_target_fps(int f) { (void)f; }
static bool egl_true(void) { return true; }
static void *egl_get_window_handle(void) { return nullptr; }
static void egl_set_title(const char *t) { (void)t; }
static int  egl_get_swap_interval(void) { return 0; }
static bool egl_set_swap_interval(int i) { (void)i; return false; }

extern "C" struct GfxWindowManagerAPI gfx_egl_wm = {
    egl_init,
    egl_close,
    egl_get_display_mode,
    egl_get_current_display_mode,
    egl_get_num_display_modes,
    egl_zero32,                 /* get_fullscreen_state */
    egl_set_fs_changed_cb,
    egl_set_bool,               /* set_fullscreen */
    egl_set_bool,               /* set_fullscreen_exclusive */
    egl_set_i32,                /* set_fullscreen_flag */
    egl_get_fs_flag_mode,
    egl_zero32,                 /* get_maximized_state */
    egl_set_bool,               /* set_maximize */
    egl_get_refresh_rate,
    egl_set_bool,               /* set_cursor_visibility */
    egl_set_closest_res,
    egl_set_dimensions,
    egl_get_dimensions,
    egl_get_centered,
    egl_handle_events,
    egl_start_frame,
    egl_noop,                   /* swap_buffers_begin */
    egl_noop,                   /* swap_buffers_end */
    egl_get_time,
    egl_get_target_fps,
    egl_set_target_fps,
    egl_true,                   /* can_disable_vsync */
    egl_get_window_handle,
    egl_set_title,
    egl_get_swap_interval,
    egl_set_swap_interval,
};

/* --------------------------------------------------------------- capture */
static uint32_t crc32_buf(const uint8_t *p, size_t n, uint32_t crc) {
    static uint32_t tab[256]; static int init = 0;
    if (!init) { for (uint32_t i=0;i<256;i++){ uint32_t c=i; for(int k=0;k<8;k++) c=(c&1)?0xEDB88320u^(c>>1):c>>1; tab[i]=c; } init=1; }
    crc ^= 0xFFFFFFFFu;
    for (size_t i=0;i<n;i++) crc = tab[(crc^p[i])&0xFF]^(crc>>8);
    return crc ^ 0xFFFFFFFFu;
}
static void put_be32(FILE *f, uint32_t v){ uint8_t b[4]={(uint8_t)(v>>24),(uint8_t)(v>>16),(uint8_t)(v>>8),(uint8_t)v}; fwrite(b,1,4,f); }
static void png_chunk(FILE *f, const char *type, const uint8_t *data, size_t len){
    put_be32(f,(uint32_t)len); fwrite(type,1,4,f); if(len) fwrite(data,1,len,f);
    uint8_t *tmp=(uint8_t*)malloc(4+len); memcpy(tmp,type,4); if(len) memcpy(tmp+4,data,len);
    uint32_t crc=crc32_buf(tmp,4+len,0); free(tmp); put_be32(f,crc);
}

int gfx_egl_save_png(const char *path) {
    if (!s_fbo || !s_buf || s_w <= 0 || s_h <= 0) return -1;
    glFinish();
    GLint drawfb = -1, readfb = -1;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawfb);
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &readfb);
    GLenum err = glGetError();
    fprintf(stderr, "gfx_egl: pre-capture s_fbo=%u draw_fb=%d read_fb=%d glerr=0x%x\n",
            s_fbo, drawfb, readfb, err);
    glBindFramebuffer(GL_FRAMEBUFFER, s_fbo);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);   /* gfx may have left color writes masked off */
    glDisable(GL_SCISSOR_TEST);
    glViewport(0, 0, s_w, s_h);
#ifdef GL_COLOR_ATTACHMENT0
    glReadBuffer(GL_COLOR_ATTACHMENT0);
#endif
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, s_w, s_h, GL_RGBA, GL_UNSIGNED_BYTE, s_buf);   /* rows bottom-up */
    { size_t c = ((size_t)(s_h/2)*s_w + s_w/2)*4;
      fprintf(stderr, "gfx_egl: center px = (%u,%u,%u,%u)\n", s_buf[c],s_buf[c+1],s_buf[c+2],s_buf[c+3]); }

    FILE *f = fopen(path, "wb"); if (!f) return -1;
    static const uint8_t sig[8]={0x89,'P','N','G',0x0D,0x0A,0x1A,0x0A}; fwrite(sig,1,8,f);
    uint8_t ihdr[13];
    ihdr[0]=(uint8_t)(s_w>>24);ihdr[1]=(uint8_t)(s_w>>16);ihdr[2]=(uint8_t)(s_w>>8);ihdr[3]=(uint8_t)s_w;
    ihdr[4]=(uint8_t)(s_h>>24);ihdr[5]=(uint8_t)(s_h>>16);ihdr[6]=(uint8_t)(s_h>>8);ihdr[7]=(uint8_t)s_h;
    ihdr[8]=8; ihdr[9]=6; ihdr[10]=ihdr[11]=ihdr[12]=0; png_chunk(f,"IHDR",ihdr,13);

    size_t raw_len=(size_t)s_h*(1+(size_t)s_w*4);
    uint8_t *raw=(uint8_t*)malloc(raw_len); size_t o=0;
    for (int y=0;y<s_h;y++){ const uint8_t *src=s_buf+(size_t)(s_h-1-y)*s_w*4; raw[o++]=0; memcpy(raw+o,src,(size_t)s_w*4); o+=(size_t)s_w*4; }

    size_t max_z=2+raw_len+(raw_len/65535+1)*5+4; uint8_t *z=(uint8_t*)malloc(max_z); size_t zo=0;
    z[zo++]=0x78; z[zo++]=0x01; size_t pos=0;
    while (pos<raw_len){ size_t blk=raw_len-pos; if(blk>65535)blk=65535; int fin=(pos+blk>=raw_len)?1:0;
        z[zo++]=(uint8_t)fin; z[zo++]=(uint8_t)(blk&0xFF); z[zo++]=(uint8_t)(blk>>8);
        uint16_t nlen=(uint16_t)~blk; z[zo++]=(uint8_t)(nlen&0xFF); z[zo++]=(uint8_t)(nlen>>8);
        memcpy(z+zo,raw+pos,blk); zo+=blk; pos+=blk; }
    uint32_t a=1,b=0; for(size_t i=0;i<raw_len;i++){ a=(a+raw[i])%65521; b=(b+a)%65521; }
    uint32_t adler=(b<<16)|a; z[zo++]=(uint8_t)(adler>>24);z[zo++]=(uint8_t)(adler>>16);z[zo++]=(uint8_t)(adler>>8);z[zo++]=(uint8_t)adler;
    png_chunk(f,"IDAT",z,zo); png_chunk(f,"IEND",nullptr,0);
    free(z); free(raw); fclose(f);
    fprintf(stderr, "gfx_egl: wrote %s (%dx%d)\n", path, s_w, s_h);
    return 0;
}
