#ifndef GFX_OSMESA_H
#define GFX_OSMESA_H
/*
 * Headless OSMesa window-manager backend for Fast3D (ground-truth PC render).
 * OSMesa renders into a host RGBA8 memory buffer via Mesa's software rasterizer
 * (llvmpipe) with NO display server required — exactly what we need to capture a
 * reference frame on a headless box before porting the same gfx_pc pipeline to
 * citro3d/PICA200. The GL/osmesa.h system header is absent here, so we declare
 * the handful of OSMesa entry points ourselves (stable ABI).
 */
#include "gfx_window_manager_api.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ---- OSMesa ABI (subset) — mirrors GL/osmesa.h ---- */
struct osmesa_context;
typedef struct osmesa_context *OSMesaContext;
#define OSMESA_RGBA 0x1908 /* == GL_RGBA */
extern OSMesaContext OSMesaCreateContextExt(unsigned int format, int depthBits, int stencilBits,
                                            int accumBits, OSMesaContext sharelist);
extern unsigned char OSMesaMakeCurrent(OSMesaContext ctx, void *buffer, unsigned int type,
                                       int width, int height);
extern void OSMesaDestroyContext(OSMesaContext ctx);
extern void *OSMesaGetProcAddress(const char *funcName);
extern void OSMesaPixelStore(int pname, int value);

/* the exported window-manager the port wires into GfxInitSettings.wapi */
extern struct GfxWindowManagerAPI gfx_osmesa_wm;

/* headless capture helpers (implemented in gfx_osmesa.cpp) */
const unsigned char *gfx_osmesa_get_buffer(int *out_w, int *out_h); /* RGBA8, row 0 = bottom */
int gfx_osmesa_save_png(const char *path);                          /* flips to top-down, writes PNG; 0 = ok */

#ifdef __cplusplus
}
#endif

#endif /* GFX_OSMESA_H */
