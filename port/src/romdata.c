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
 * selected region ROM for Path B).
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
#define TUROK_ROM_OFFSET     0x1F00
extern u8 _staticSegmentRomStart[];
extern char g_cfg_rom_path[];
extern const char *sysArgGetString(const char *arg);

static int turok_rom_is_v64(FILE *f)
{
    unsigned char magic[4];
    long pos = ftell(f);
    int v64 = 0;
    if (fseek(f, 0, SEEK_SET) == 0 && fread(magic, 1, sizeof magic, f) == sizeof magic)
        v64 = magic[0] == 0x37 && magic[1] == 0x80 && magic[2] == 0x40 && magic[3] == 0x12;
    fseek(f, pos, SEEK_SET);
    return v64;
}

static void turok_swap_v64(void *data, size_t size)
{
    unsigned char *p = (unsigned char *)data;
    size_t i;
    for (i = 0; i + 1 < size; i += 2) {
        unsigned char b = p[i]; p[i] = p[i + 1]; p[i + 1] = b;
    }
}

int turokRomIsGerman(const char *path)
{
    FILE *f;
    unsigned char header[0x40];
    int v64;
    if (!path || !(f = fopen(path, "rb"))) return 0;
    v64 = turok_rom_is_v64(f);
    if (fseek(f, 0, SEEK_SET) != 0 || fread(header, 1, sizeof header, f) != sizeof header) {
        fclose(f); return 0;
    }
    fclose(f);
    if (v64) turok_swap_v64(header, sizeof header);
    return header[0x3b] == 'N' && header[0x3c] == 'T' && header[0x3d] == 'U' && header[0x3e] == 'D';
}

static const char *DEFAULT_CARTDATA[] = {
    "cartdata.dat",                   /* next to the binary / in the CWD (a normal install) */
    "src/PR/cartdata.dat",            /* running from the repo root: full blob (11 root items, 50 levels) */
    "src/PR/tengine/oldcartdata.dat", /* fallback */
    NULL,
};

#if defined(PLATFORM_PORT) && !defined(PLATFORM_3DS)
#include <unistd.h>                   /* readlink */
/* NB: no <string.h> — its <strings.h> pulls bcopy/bzero/bcmp which clash with os_libc.h (see line 22).
 * Use the compiler builtins for the string ops; snprintf comes from <stdio.h> (already included). */
static int rom_file_exists(const char *p) { FILE *f = fopen(p, "rb"); if (f) { fclose(f); return 1; } return 0; }
/* Build "<dir-of-this-executable>/<name>" into buf. Returns buf, or NULL if the exe dir can't be found. */
static const char *rom_exe_relative(const char *name, char *buf, size_t bufsz)
{
    char exe[4096];
    ssize_t n = readlink("/proc/self/exe", exe, sizeof(exe) - 1);
    char *slash;
    if (n <= 0) return NULL;
    exe[n] = 0;
    slash = __builtin_strrchr(exe, '/');
    if (!slash) return NULL;
    *slash = 0;
    if ((size_t)snprintf(buf, bufsz, "%s/%s", exe, name) >= bufsz) return NULL;
    return buf;
}
#endif

/* The retail ROM path (Path B). PC: $TUROK_ROM (NULL if unset -> caller uses the dev fallback). 3DS: the
 * SD-card ROM, since there are no env vars. Shared by romdataInit AND the audio bank loads (audio.c) — the
 * audio used bare getenv("TUROK_ROM"), which is NULL on 3DS, so the SFX/SEQ banks never loaded and the SFX
 * player dereferenced a NULL bank. */
#ifdef PLATFORM_3DS
/* libctru: Result romfsMountSelf(const char*) mounts the title's own RomFS at "<name>:/".
 * (romfsInit() is only a static-inline wrapper in the header, so we can't link to it; call the
 * real exported symbol directly to avoid pulling in <3ds.h>, whose u8/u32 typedefs clash with ultra64.h.) */
extern int romfsMountSelf(const char *name);
#endif

const char *turokRomPath(void)
{
    const char *rom = getenv("TUROK_ROM");
    const char *arg = sysArgGetString("--rom");
    if (arg && *arg) return arg;
    if (rom && *rom) return rom;
#ifdef PLATFORM_3DS
    /* A CIA install carries the selected ROM inside RomFS; a config path may select a
     * different staged filename or an SD-card override. */
    {
        static int s_romfs = -1;
        if (s_romfs < 0) s_romfs = (romfsMountSelf("romfs") == 0) ? 1 : 0;
        if (g_cfg_rom_path[0] && __builtin_strcmp(g_cfg_rom_path, "none")) {
            static char cfg_path[4096];
            if (!__builtin_strncmp(g_cfg_rom_path, "romfs:/", 7) ||
                !__builtin_strncmp(g_cfg_rom_path, "sdmc:/", 6))
                return g_cfg_rom_path;
            if (s_romfs == 1) {
                snprintf(cfg_path, sizeof cfg_path, "romfs:/%s", g_cfg_rom_path);
                return cfg_path;
            }
            snprintf(cfg_path, sizeof cfg_path, "sdmc:/3ds/turok/%s", g_cfg_rom_path);
            return cfg_path;
        }
        if (s_romfs == 1) return "romfs:/turok.rom";
    }
    return "sdmc:/3ds/turok/turok.rom";
#else
    if (g_cfg_rom_path[0]) {
        if (!__builtin_strcmp(g_cfg_rom_path, "none")) return NULL;
        return g_cfg_rom_path;
    }

    /* PC: with no explicit $TUROK_ROM, AUTO-DISCOVER the retail ROM so the game runs as a plain
     * `./turok` (or an installed `turok`) with the ROM sitting alongside it — no env var / launcher
     * script needed. Search the executable's own directory first (a normal install: turok + the ROM
     * in one folder), then the current directory (running from the repo root). Cached after the first
     * lookup. Not found -> return NULL and the caller falls back to the dev cartdata.dat (v49 assets). */
    {
        static char found[4096];
        static int  state = 0;              /* 0 untried, 1 found (in found[]), 2 not found */
        if (state == 0) {
            state = 2;
            if (rom_exe_relative("baserom.us.v12.z64", found, sizeof found) && rom_file_exists(found)) {
                state = 1;
            } else if (rom_file_exists("baserom.us.v12.z64")) {
                snprintf(found, sizeof found, "baserom.us.v12.z64");
                state = 1;
            }
            if (state == 1)
                fprintf(stderr, "[romdata] auto-found retail ROM: %s\n", found);
        }
        if (state == 1) return found;
    }
    return NULL;                            /* NULL -> caller uses the dev cartdata.dat fallback */
#endif
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
    { const char *rom = turokRomPath(); FILE *rf; int v64 = 0;   /* PC: CLI/config/env. 3DS: romfs:/ or SD. */
      if (rom && *rom) {
        rf = fopen(rom, "rb");
        if (!rf) { fprintf(stderr, "[romdata] FATAL: ROM=%s not found\n", rom); return -1; }
        v64 = turok_rom_is_v64(rf);
        if (fseek(rf, TUROK_ROM_OFFSET, SEEK_SET) != 0 ||
            fread(_staticSegmentRomStart, 1, TUROK_CARTDATA_SIZE, rf) != (size_t)TUROK_CARTDATA_SIZE) {
            fprintf(stderr, "[romdata] FATAL: short read of asset blob from %s @0x1F00\n", rom);
            fclose(rf); return -1;
        }
        fclose(rf);
        if (v64) turok_swap_v64(_staticSegmentRomStart, TUROK_CARTDATA_SIZE);
        { u32 root = ((u32)_staticSegmentRomStart[0]<<24)|((u32)_staticSegmentRomStart[1]<<16)
                   | ((u32)_staticSegmentRomStart[2]<<8) | (u32)_staticSegmentRomStart[3];
          fprintf(stderr, "[romdata] Path B: RETAIL assets from %s @0x%X (%d bytes, %s); root items=%u %s\n",
                  rom, TUROK_ROM_OFFSET, TUROK_CARTDATA_SIZE, v64 ? "v64 normalized" : "z64", root, root==11?"(OK)":"(UNEXPECTED)"); }
        return 0;
      }
    }

    if (path) f = fopen(path, "rb");
#if defined(PLATFORM_PORT) && !defined(PLATFORM_3DS)
    /* dev-asset fallback: cartdata.dat next to the executable (a normal install with no ROM). */
    if (!f) { char rel[4096];
        if (rom_exe_relative("cartdata.dat", rel, sizeof rel) && (f = fopen(rel, "rb"))) path = rel; }
#endif
    for (i = 0; !f && DEFAULT_CARTDATA[i]; i++) {
        path = DEFAULT_CARTDATA[i];
        f = fopen(path, "rb");
    }
    if (!f) {
        fprintf(stderr, "[romdata] FATAL: no selected ROM or cartdata.dat found "
                        "next to the executable or in the current directory "
                        "(or set TUROK_ROM=<path> / TUROK_CARTDATA=<path>)\n");
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
