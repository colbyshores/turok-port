/* gfx_egl.h — headless hardware-GL window-manager backend for Fast3D (EGL surfaceless
 * + an FBO render target), the fast counterpart to the software-OSMesa backend. */
#ifndef GFX_EGL_H
#define GFX_EGL_H

#include "gfx_window_manager_api.h"

#ifdef __cplusplus
extern "C" {
#endif

extern struct GfxWindowManagerAPI gfx_egl_wm;

/* read the FBO back and write a top-down RGBA8 PNG; returns 0 on success */
int gfx_egl_save_png(const char *path);

#ifdef __cplusplus
}
#endif

#endif
