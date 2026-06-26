/* romdata.c — the cartridge data seam (CLAUDE.md §1 Path A, §4).
 *
 * Turok locates every asset by walking a self-describing CIndexedSet directory anchored
 * at _staticSegmentRomStart (see scene.c / cart.c). The game uses (u32)base+offset as a
 * "device address" handed to osPiStartDma. We load the cart data blob into the
 * _staticSegmentRomStart buffer (turok_runtime.c), so those device addresses are already
 * valid host pointers — romPiRead is then just a memcpy, exactly like cart.c's existing
 * #ifdef WIN32 path (`memcpy(pData, (void*)addr, len)`).
 *
 * Path A uses the source tree's own matched cartdata.dat (drift-free). Override with
 * TUROK_CARTDATA=<path> to point at a different blob (e.g. one extracted from the
 * retail ROM for Path B).
 */
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#ifndef PLATFORM_3DS
#include <execinfo.h>            /* glibc backtrace() — host-only DMA-OOB diagnostic */
#endif
#include <ultra64.h>
/* no <string.h> (bcopy/bzero clash with os_libc.h); use the builtin */
#define memcpy __builtin_memcpy

#define TUROK_CARTDATA_SIZE  6489336
extern u8 _staticSegmentRomStart[];

static const char *DEFAULT_CARTDATA[] = {
    "src/PR/cartdata.dat",            /* full blob: 11 root items, 50 levels, 1035 texsets */
    "src/PR/tengine/oldcartdata.dat", /* fallback */
    NULL,
};

/* The retail ROM path (Path B). PC: $TUROK_ROM (NULL if unset -> caller uses the dev fallback). 3DS: the
 * SD-card ROM, since there are no env vars. Shared by romdataInit AND the audio bank loads (audio.c) — the
 * audio used bare getenv("TUROK_ROM"), which is NULL on 3DS, so the SFX/SEQ banks never loaded and the SFX
 * player dereferenced a NULL bank. */
const char *turokRomPath(void)
{
    const char *rom = getenv("TUROK_ROM");
#ifdef PLATFORM_3DS
    if (!rom || !*rom) rom = "sdmc:/3ds/turok/baserom.us.v12.z64";
#endif
    return rom;
}

/* Load the cart data blob into the static segment. Returns 0 on success. */
int romdataInit(void)
{
    const char *path = getenv("TUROK_CARTDATA");
    FILE *f = NULL;
    long sz;
    int i;

    /* Path B (TUROK_ROM=<retail .z64>): load the v1.2 asset blob straight from the retail ROM
     * at ROM offset 0x1F00. Verified byte-for-byte to be the same CIndexedSet format + size as
     * cartdata.dat (root word 0x0b, index size 0x38), so the whole cart cache / offset / RNC
     * path works unchanged — but it's the RETAIL v1.2 content, not the v49 dev cartdata.dat
     * (the two share a root header but their data is ~99% different). */
    { const char *rom = getenv("TUROK_ROM"); FILE *rf;
#ifdef PLATFORM_3DS
      /* 3DS: no env vars — default to the retail ROM on the SD card (Path B). */
      if (!rom || !*rom) rom = "sdmc:/3ds/turok/baserom.us.v12.z64";
#endif
      if (rom && *rom) {
        rf = fopen(rom, "rb");
        if (!rf) { fprintf(stderr, "[romdata] FATAL: TUROK_ROM=%s not found\n", rom); return -1; }
        if (fseek(rf, 0x1F00, SEEK_SET) != 0 ||
            fread(_staticSegmentRomStart, 1, TUROK_CARTDATA_SIZE, rf) != (size_t)TUROK_CARTDATA_SIZE) {
            fprintf(stderr, "[romdata] FATAL: short read of asset blob from %s @0x1F00\n", rom);
            fclose(rf); return -1;
        }
        fclose(rf);
        { u32 root = ((u32)_staticSegmentRomStart[0]<<24)|((u32)_staticSegmentRomStart[1]<<16)
                   | ((u32)_staticSegmentRomStart[2]<<8) | (u32)_staticSegmentRomStart[3];
          fprintf(stderr, "[romdata] Path B: RETAIL v1.2 assets from %s @0x1F00 (%d bytes); root items=%u %s\n",
                  rom, TUROK_CARTDATA_SIZE, root, root==11?"(OK)":"(UNEXPECTED)"); }
        return 0;
      }
    }

    if (path) f = fopen(path, "rb");
    for (i = 0; !f && DEFAULT_CARTDATA[i]; i++) {
        path = DEFAULT_CARTDATA[i];
        f = fopen(path, "rb");
    }
    if (!f) {
        fprintf(stderr, "[romdata] FATAL: no cartdata.dat found "
                        "(set TUROK_CARTDATA=<path>)\n");
        return -1;
    }

    fseek(f, 0, SEEK_END);
    sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz > TUROK_CARTDATA_SIZE) sz = TUROK_CARTDATA_SIZE;
    if (fread(_staticSegmentRomStart, 1, (size_t)sz, f) != (size_t)sz) {
        fprintf(stderr, "[romdata] FATAL: short read on %s\n", path);
        fclose(f);
        return -1;
    }
    fclose(f);

    /* Sanity: the root CIndexedSet header is a big-endian DWORD blockCount == 11
     * (CART_ROOT_nItems). This is our drift check from CLAUDE.md §1 in code form. */
    {
        u32 root = ((u32)_staticSegmentRomStart[0] << 24) | ((u32)_staticSegmentRomStart[1] << 16)
                 | ((u32)_staticSegmentRomStart[2] << 8)  |  (u32)_staticSegmentRomStart[3];
        fprintf(stderr, "[romdata] loaded %s (%ld bytes); cart root items = %u %s\n",
                path, sz, root, root == 11 ? "(OK)" : "(UNEXPECTED — drift?)");
    }
    return 0;
}

/* The single PI/DMA read seam. devAddr is base+offset into the loaded blob (already a
 * host pointer on the -m32 build); copy nbytes to dst. Mirrors cart.c's WIN32 memcpy. */
extern u8 _staticSegmentRomEnd[];
static int s_dma_calls = 0;
void romPiRead(void *dst, u32 devAddr, u32 nbytes)
{
    uintptr_t base = (uintptr_t)_staticSegmentRomStart;
    uintptr_t end  = base + TUROK_CARTDATA_SIZE;
    uintptr_t a    = (uintptr_t)devAddr;
    int oob = (a < base || a + nbytes > end);
    if (oob && nbytes > 0)                       /* real OOB always-on */
        fprintf(stderr, "[dma %d] dst=%p devAddr=0x%08x nbytes=%u  off=%ld%s\n",
                s_dma_calls, dst, devAddr, nbytes, (long)(a - base),
                "  <<< OUT OF CARTDATA RANGE");
    if (nbytes > 0x1000000u) {                 /* absurd size — show who asked */
        fprintf(stderr, "[dma] ABSURD nbytes=%u\n", nbytes);
#ifndef PLATFORM_3DS
        { void *bt[16]; int n = backtrace(bt, 16);
          fprintf(stderr, "[dma]  caller backtrace:\n"); backtrace_symbols_fd(bt, n, 2); }
#endif
        fflush(stderr);
        return;                                /* don't actually crash; let us read the trace */
    }
    s_dma_calls++;
    memcpy(dst, (const void *)a, nbytes);
}
