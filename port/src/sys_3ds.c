/*
 * sys_3ds.c — Turok 3DS system layer: stack size, the on-device diagnostic log, framebuffer capture.
 *
 * The whole N64 boot (boot() -> mainproc()) runs INLINE on the single 3dsx main thread (the libultra
 * shim's osStartThread is a cooperative no-op), and the deep cart/asset loader chain overflows libctru's
 * default 32 KB stack -> heap free-list corruption / data-abort. Override __stacksize__ to 2 MB (both
 * sibling 3DS ports do this).
 *
 * plat3dsBootLog is THE on-hardware trace channel (a .3dsx has no visible stdout): append+flush to
 * sdmc:/3ds/turok/boot.log every line (so a data-abort can't lose the trail) + svcOutputDebugString
 * (Luma / Mandarine log). It's the equivalent of the PC build's stderr trace; gfx_citro3d.cpp calls it.
 */
#ifdef PLATFORM_3DS
#include <3ds.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

/* 2 MB; overrides libctru's WEAK 32 KB default. */
unsigned int __stacksize__ = 2 * 1024 * 1024;

/* ★★ HEAP HEADROOM OVERRIDE (weak-override of libctru's __system_allocateHeaps, same mechanism as
 * __stacksize__). THE 3DS-BOOT FIX: libctru's DEFAULT __system_allocateHeaps grabs ALL remaining
 * application FCRAM (main heap = free - 32 MB linear), so Turok's ~16 MB BSS + 75.8 MB main + 32 MB
 * linear = ~124 MB = 100% of the New-3DS application region, ZERO headroom. The very next kernel
 * allocation — svcCreateThread for libctru aptInit's APT event-handler thread (the 2nd thread) needs
 * to map the new thread's TLS page from FCRAM — then has nowhere to go and HANGS pre-main. (Pinned via
 * gdb-on-Mandarine: boot reaches aptInit -> APT_GetLockHandle/Initialize OK -> threadCreate's memalign
 * OK -> svcCreateThread hangs; Perfect Dark, which DOES cap its heaps, creates the same thread and
 * boots.) Fix: leave a fixed FCRAM SAFETY margin unmapped so the kernel can create threads + map the
 * GSP/APT shared memory. Turok's big pools are static BSS, not malloc, so a modest main heap is plenty.
 * 3DS-only; PC is unaffected. */
extern char* fake_heap_start;
extern char* fake_heap_end;
extern u32 __ctru_heap, __ctru_heap_size, __ctru_linear_heap, __ctru_linear_heap_size;

void __system_allocateHeaps(void)   /* strong -> overrides libctru's weak default */
{
    u32 avail = osGetMemRegionFree(MEMREGION_APPLICATION) & ~0xFFFu;

    const u32 SAFETY   = 16u * 1024u * 1024u;  /* FCRAM left UNMAPPED for kernel thread-TLS + shared-mem maps */
    const u32 MAIN_RSV = 16u * 1024u * 1024u;  /* main (malloc) heap reserve — Turok's big pools are static BSS, so this is plenty */
    const u32 LIN_CAP  = 56u * 1024u * 1024u;  /* ★ GPU/LINEAR heap = where Citro3D textures live (gfx_citro3d's "scratch ram").
                                                * Was 24 MB (a conservative cap from the boot investigation — but the ACTUAL boot
                                                * fix was the memset override, not heap headroom). 24 MB is too small for a
                                                * texture-heavy area's working set -> C3D_TexInit OOMs -> stale/garbage texture
                                                * slots = the "texture corruption after a while". Give it the lion's share. */

    u32 usable = (avail > SAFETY + (16u << 20)) ? (avail - SAFETY) : (avail / 2u);
    /* Hand the GPU/linear heap everything past a small main-heap reserve, capped at LIN_CAP. */
    u32 lin = (usable > MAIN_RSV) ? (usable - MAIN_RSV) : (usable / 2u);
    if (lin > LIN_CAP) lin = LIN_CAP;
    lin &= ~0xFFFu;

    /* allocate the LINEAR (GPU) heap first; shrink-and-retry so it's robust on any memory tier */
    while (lin >= (4u << 20) &&
           R_FAILED(svcControlMemory(&__ctru_linear_heap, 0, 0, lin, MEMOP_ALLOC_LINEAR, MEMPERM_READWRITE)))
        lin = (lin - (2u << 20)) & ~0xFFFu;
    __ctru_linear_heap_size = lin;

    /* main (malloc) heap = the rest of `usable` (NOT all of `avail` — that's the whole point) */
    __ctru_heap = OS_HEAP_AREA_BEGIN;
    __ctru_heap_size = (usable - lin) & ~0xFFFu;
    u32 tmp = 0;
    if (R_FAILED(svcControlMemory(&tmp, __ctru_heap, 0, __ctru_heap_size, MEMOP_ALLOC, MEMPERM_READWRITE)))
        svcBreak(USERBREAK_PANIC);   /* can't run without a main heap */

    fake_heap_start = (char*)__ctru_heap;
    fake_heap_end   = fake_heap_start + __ctru_heap_size;

    /* ★ REQUIRED: an override REPLACES libctru's default, which initialises the mappable VA region;
     * we must too, or mappableAlloc() (gspInit's GSP shared memory, C3D) returns NULL -> abort. */
    mappableInit(OS_MAP_AREA_BEGIN, OS_MAP_AREA_END);
}

/* DIAGNOSTIC __appInit (strong override of libctru's weak one) — pin the pre-main boot hang.
 * Mirrors libctru's exact order (srvInit/aptInit/hidInit/fsInit/archiveMountSdmc) but with clean
 * function boundaries to gdb-breakpoint (no JIT-block imprecision inside libctru), PLUS a throwaway
 * test thread right after srvInit to answer: does svcCreateThread work AT ALL in Turok's process,
 * independent of APT? (svcOutputDebugString is NOT surfaced by Mandarine pre-sdmc, so these traces are
 * for on-HW/Luma + gdb-breakpoint anchors only.) Gated TUROK_DIAG_APPINIT so it's easy to drop. */
#ifdef TUROK_DIAG_APPINIT
extern Result archiveMountSdmc(void);
static void diag_trace(const char *s) { svcOutputDebugString(s, (int)strlen(s)); }
static void diag_testthread(void *arg) { (void)arg; }   /* returns immediately */
volatile Thread g_diag_testthread;   /* volatile so the result isn't optimised away (gdb reads it) */

void __appInit(void)
{
    diag_trace("TUROK_APPINIT:start");
    srvInit();                                   diag_trace("TUROK_APPINIT:srv");
    /* THE KEY TEST: a plain thread, before APT. If this hangs/returns NULL, svcCreateThread is broken
     * for Turok's process (memory/layout), independent of the APT event thread. */
    g_diag_testthread = threadCreate(diag_testthread, 0, 4096, 0x30, -2, true);  /* <-- bp after: read g_diag_testthread.
        FINDING (2026-06-21): this hangs in Mandarine for Turok's process — i.e. svcCreateThread hangs for ANY
        thread, before APT, independent of heap size / BSS size / JIT / core_id. PD does the identical call and
        boots ([New Thread 2]). On real HW/Luma this trace will show whether it's a Mandarine HLE limit or real. */
    diag_trace("TUROK_APPINIT:testthread");
    aptInit();                                   diag_trace("TUROK_APPINIT:apt");
    hidInit();                                   diag_trace("TUROK_APPINIT:hid");
    fsInit();                                    diag_trace("TUROK_APPINIT:fs");
    archiveMountSdmc();                          diag_trace("TUROK_APPINIT:sdmc");
    diag_trace("TUROK_APPINIT:done");
}
#endif /* TUROK_DIAG_APPINIT */

#define TUROK_BOOTLOG_DIR  "sdmc:/3ds/turok"
#define TUROK_BOOTLOG_PATH "sdmc:/3ds/turok/boot.log"

void plat3dsBootLog(const char *msg)
{
    static int started = 0;
    FILE *f;
    /* Diagnostics OFF by default — a clean play build writes no boot.log / debug spam. Re-enable the
     * boot + trace logging with turok.cfg `debug 1`. (g_cfg_debug defaults 0; the few BL() lines before
     * turokConfigLoad runs are simply skipped, which is fine.) */
    extern int g_cfg_debug;
    if (!g_cfg_debug) return;
    if (!msg) return;
    svcOutputDebugString(msg, (int)strlen(msg));   /* Luma3DS / Mandarine debug console */
    if (!started) mkdir(TUROK_BOOTLOG_DIR, 0777);  /* best-effort (the ROM lives there too) */
    f = fopen(TUROK_BOOTLOG_PATH, started ? "a" : "w");
    if (f) { fputs(msg, f); fputc('\n', f); fflush(f); fclose(f); started = 1; }  /* per-line: survives a crash */
}

/* printf-style boot.log trace (debugging). Same `debug 1` gate (via plat3dsBootLog). */
#include <stdarg.h>
void plat3dsLogv(const char *fmt, ...)
{
    char buf[160];
    va_list ap; va_start(ap, fmt); vsnprintf(buf, sizeof(buf), fmt, ap); va_end(ap);
    plat3dsBootLog(buf);
}


/* "Did anything rasterize?" signal — count non-black pixels on the top-left framebuffer (mirror of
 * the PC TUROK_CAPTURE_FRAME). Cheap, no file written; logs the count. */
void plat3dsCaptureTopFB(void)
{
    u16 w = 0, h = 0;
    u8 *fb = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, &w, &h);
    long nonblack = 0, i, n = (long)w * h * 3;
    char b[80];
    if (!fb) return;
    for (i = 0; i + 2 < n; i += 3)
        if (fb[i] | fb[i + 1] | fb[i + 2]) nonblack++;
    snprintf(b, sizeof(b), "[capture] top-fb %ux%u nonblack=%ld", w, h, nonblack);
    plat3dsBootLog(b);
}

/* Pass the real newlib stderr (a macro here) to the global-symbol shim in stderr_3ds.c — the game's
 * port traces reference a global `stderr` symbol that newlib (macro) doesn't provide. */
void *plat3dsRealStderr(void) { return (void *)stderr; }

/* DIAGNOSTIC __appInit (libctru's EXACT order) + an SD trace AFTER sdmc mounts. This decides where the
 * pre-main hang is, since the SD log is the only readable channel and it needs sdmc:
 *   - "appInit: 5/5 OK" appears + "main: start" appears  -> it BOOTS.
 *   - "appInit: 5/5 OK" appears, "main: start" does NOT  -> hang is in init_array (a C++ global ctor:
 *     gfx_citro3d's std::unordered_map sShaderPool / the libstdc++ EH init) = the RENDERER.
 *   - NEITHER appears                                     -> hang is inside libctru __appInit. */
#endif /* PLATFORM_3DS */
