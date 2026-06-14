#ifndef GBI_EXT_H
#define GBI_EXT_H
/*
 * Perfect-Dark Fast3D extension opcodes/values that gfx_pc.cpp references but
 * are absent from Banjo's stock F3DEX 1.x <PR/gbi.h>. Banjo never emits these,
 * so the corresponding gfx_pc switch cases are dead — we just need values that
 * (a) compile and (b) don't collide with any opcode Banjo actually emits.
 * Banjo's F3DEX-1.x opcodes occupy 0x00-0x09 and 0xB0-0xFF; 0x11-0x19 are free.
 */
#ifndef G_TRI4
#define G_TRI4 0x11
#endif
#ifndef G_SETFB_EXT
#define G_SETFB_EXT 0x12
#endif
#ifndef G_SETTIMG_FB_EXT
#define G_SETTIMG_FB_EXT 0x13
#endif
#ifndef G_SETGRAYSCALE_EXT
#define G_SETGRAYSCALE_EXT 0x15
#endif
#ifndef G_SETINTENSITY_EXT
#define G_SETINTENSITY_EXT 0x16
#endif
#ifndef G_SETSUBPIXELOFFSET_EXT
#define G_SETSUBPIXELOFFSET_EXT 0x17
#endif
#ifndef G_TEXRECT_WIDE_EXT
#define G_TEXRECT_WIDE_EXT 0x19
#endif
/* texture-filter extension mode (not an opcode); Banjo uses POINT/BILERP only */
#ifndef G_TF_BLUR_EXT
#define G_TF_BLUR_EXT 0x7F
#endif

/* round 2 PD extensions */
#ifndef G_COL
#define G_COL 0x1a
#endif
#ifndef G_RDPHALF_CONT
#define G_RDPHALF_CONT 0x1b
#endif
#ifndef G_RDPFLUSH_EXT
#define G_RDPFLUSH_EXT 0x1d
#endif
#ifndef G_NO_CLIPPING_EXT
#define G_NO_CLIPPING_EXT 0x1e
#endif
#ifndef G_INVALTEXCACHE_EXT
#define G_INVALTEXCACHE_EXT 0x1f
#endif
#ifndef G_IMAGERECT_EXT
#define G_IMAGERECT_EXT 0x21
#endif
#ifndef G_FILLRECT_WIDE_EXT
#define G_FILLRECT_WIDE_EXT 0x22
#endif
#ifndef G_COPYFB_EXT
#define G_COPYFB_EXT 0x23
#endif
#ifndef G_CLEAR_DEPTH_EXT
#define G_CLEAR_DEPTH_EXT 0x25
#endif
#ifndef G_ASPECT_WIDE_EXT
#define G_ASPECT_WIDE_EXT 0x26
#endif
#ifndef G_MODULATE_EXT
#define G_MODULATE_EXT 0x27
#endif
#ifndef G_EXTRAGEOMETRYMODE_EXT
#define G_EXTRAGEOMETRYMODE_EXT 0x29
#endif
#ifndef G_ASPECT_RIGHT_EXT
#define G_ASPECT_RIGHT_EXT 0x2a
#endif
#ifndef G_ASPECT_MODE_EXT
#define G_ASPECT_MODE_EXT 0x2b
#endif
#ifndef G_ASPECT_LEFT_EXT
#define G_ASPECT_LEFT_EXT 0x2d
#endif
#ifndef G_ASPECT_CENTER_EXT
#define G_ASPECT_CENTER_EXT 0x2e
#endif
#endif /* GBI_EXT_H */
