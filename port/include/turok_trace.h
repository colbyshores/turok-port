/*
 * turok_trace.h — gated, categorized pipeline trace (renamed from bk_trace.h; Banjo-Kazooie heritage).
 * The internal BK_TR* / -DBK_TRACE macro names are retained as-is (inert in turok's build, which never
 * defines BK_TRACE) to avoid churn across the borrowed fast3d TUs.
 *
 * Mirrors the Forsaken 3DS port's RT_* harness (test-harness branch). Enabled by
 * -DBK_TRACE (Makefile.3ds `TRACE=1`, the default while bringing the renderer up
 * on hardware); compiles to nothing when off, so it costs zero in a shipping
 * build and is trivially removable.
 *
 * Sink: bkTraceStr() appends each line to sdmc:/3ds/banjo/trace.log
 * (open-flush-close per line = crash-survivable; the last line before a Luma
 * data-abort still reaches the FAT) and svcOutputDebugString. Pull it off the
 * device via the netload stub's HTTP server:  curl http://<ip>:8080/3ds/banjo/trace.log
 *
 * Two macro forms:
 *   BK_TR(cat, fmt, ...)        log every call (use for once-per-frame events
 *                               and the crash path — frame begin/end, clears,
 *                               target switches, present, alloc results).
 *   BK_TR_EVERY(cat, n, fmt...) rate-limit to one call in every n frames (use
 *                               for high-frequency events — per-draw samples —
 *                               so the log and the SD writes don't explode).
 *
 * Both stamp the current frame ([f=N]) and the category. Categories are a
 * runtime bitmask (bkTraceMask, default BK_TR_ALL) so they can be flipped
 * without rebuilding.
 */
#ifndef BK_TRACE_H
#define BK_TRACE_H

#ifdef BK_TRACE

#include <stdio.h>

/* --- categories (one bit each) --- */
#define BK_TR_FRAME   (1u <<  0)  /* frame loop + video lifecycle (start/end/present cadence) */
#define BK_TR_RCP     (1u <<  1)  /* bk_rcp seam: gfx task submit, DL ptr/size, gfx_run handoff */
#define BK_TR_GFX     (1u <<  2)  /* gfx_pc gfx_run begin/end, DL decode summary, opcode counts */
#define BK_TR_C3D     (1u <<  3)  /* citro3d frame lifecycle: C3D_FrameBegin/End */
#define BK_TR_TARGET  (1u <<  4)  /* render target switch (top-left/right, bottom) */
#define BK_TR_CLEAR   (1u <<  5)  /* render target clear */
#define BK_TR_MATRIX  (1u <<  6)  /* viewport / projection / MVP uploads */
#define BK_TR_DRAW    (1u <<  7)  /* per-draw vtx/tri counts */
#define BK_TR_TEX     (1u <<  8)  /* texture upload / bind */
#define BK_TR_BUFFER  (1u <<  9)  /* linearAlloc / vertex-buffer alloc — log the RESULT ptr (catch NULL) */
#define BK_TR_PRESENT (1u << 10)  /* VRAM framebuffer present / display transfer */
#define BK_TR_HEAP    (1u << 11)  /* heap / large allocations */
#define BK_TR_GAME    (1u << 12)  /* game mainLoop sub-phases */
#define BK_TR_ALL     0xFFFFu

#ifdef __cplusplus
extern "C" {
#endif

extern unsigned int bkTraceMask;   /* runtime category mask (default BK_TR_ALL) */
extern int          bkTraceFrame;  /* current frame number (stamp + rate-limit) */
extern void         bkTraceStr(const char *msg);  /* the single SD-log sink */

#ifdef __cplusplus
}
#endif

#define BK_TR(cat, fmt, ...) do { \
    if (bkTraceMask & (cat)) { \
        char _bktb[256]; \
        snprintf(_bktb, sizeof(_bktb), "[%s][f=%d] " fmt, #cat, bkTraceFrame, ##__VA_ARGS__); \
        bkTraceStr(_bktb); \
    } \
} while (0)

#define BK_TR_EVERY(cat, n, fmt, ...) do { \
    if ((bkTraceMask & (cat)) && ((n) <= 1 || (bkTraceFrame % (n)) == 0)) { \
        char _bktb[256]; \
        snprintf(_bktb, sizeof(_bktb), "[%s][f=%d] " fmt, #cat, bkTraceFrame, ##__VA_ARGS__); \
        bkTraceStr(_bktb); \
    } \
} while (0)

#else /* !BK_TRACE */

#define BK_TR(cat, fmt, ...)          do {} while (0)
#define BK_TR_EVERY(cat, n, fmt, ...) do {} while (0)

#endif /* BK_TRACE */

#endif /* BK_TRACE_H */
