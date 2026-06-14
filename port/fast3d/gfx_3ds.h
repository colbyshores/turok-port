#ifndef GFX_3DS_H
#define GFX_3DS_H

#include "gfx_window_manager_api.h"

#ifdef __cplusplus
extern "C" {
#endif

// NOTE: this header is included by video.c (libultra type world) as well as the
// renderer/window-manager (libctru type world). To avoid the libultra-vs-libctru
// `u32` typedef clash it must NOT pull in <3ds.h>/<citro3d.h>. The render-target
// accessors therefore return `void*` (really C3D_RenderTarget*); the Citro3D
// renderer casts them. See CLAUDE.md.

extern struct GfxWindowManagerAPI gfx_3ds;

// Render targets created by the window manager (gfx_3ds.c) at init and consumed
// by the renderer's per-eye replay. eye 0 = top-left, 1 = top-right (stereo).
void *gfx3dsTopTarget(int eye);     // C3D_RenderTarget*
void *gfx3dsBottomTarget(void);     // C3D_RenderTarget*

// 3D-slider amount, clamped to [0,1] (Mandarine returns garbage so clamp).
float gfx3dsStereoLevel(void);
// True when stereo should be rendered this frame.
int gfx3dsStereoActive(void);

#ifdef __cplusplus
}
#endif

#endif
