/* turok_runtime.c — symbols the N64 linker (spec/makerom) would have provided.
 *
 * On the N64 these come from the `spec` ROM layout: segment-boundary symbols
 * (_xxxSegmentRomStart/End), the microcode blobs (gspF3DEX_NoN_*), and the RSP/audio
 * ucode. The host has no makerom, so we define them here. See ../../CLAUDE.md §4 (seams).
 *
 *   - _staticSegmentRomStart : the cart data blob (cartdata.dat). This is the REAL one
 *     that matters — every asset DMA resolves against it (CLAUDE.md §1, drift note).
 *     romdata.c fills it at startup. The game only ever uses (u32)_staticSegmentRomStart
 *     as a device address handed to osPiStartDma, so romdata translates back to this
 *     buffer; we never depend on the symbol's actual link address.
 *   - everything else : boot-loader code segments (boot.c, which the host driver skips),
 *     audio banks (M4), and microcode (never executed — Fast3D interprets the gfx task
 *     in turok_rcp.c before it would reach the RSP). Dummy storage is sufficient for M1.
 */
#include <ultra64.h>

/* Path A: the cart data blob is src/PR/cartdata.dat (6,489,336 bytes, 11 root items).
 * Reserve the exact size; romdataInit() loads the file into it. */
#define TUROK_CARTDATA_SIZE  6489336

u8 _staticSegmentRomStart[TUROK_CARTDATA_SIZE];
u8 _staticSegmentRomEnd[1];               /* cosmetic: only an init-time TRACE size print */

/* Boot-loader / code segments — referenced by boot.c (skipped by the host driver) and a
 * couple of cosmetic TRACE prints in CEngineApp__Construct. Dummy storage. */
u8 _codeSegmentStart[1], _codeSegmentEnd[1];
u8 _codeSegmentRomStart[1], _codeSegmentRomEnd[1];
u8 _codeSegmentTextStart[1], _codeSegmentTextEnd[1];
u8 _codeSegmentBssStart[1], _codeSegmentBssEnd[1];
u8 _mempoolSegmentStart[1];

/* Audio banks (sequence + sfx control/table) — DEFERRED to M4 (audio).
 * initAudio() computes `bankLen = _xxxSegmentRomEnd - _xxxSegmentRomStart` and DMAs that
 * many bytes. With detached dummy arrays that span is garbage (multi-GB) → crash. We alias
 * each Start/End pair to the SAME storage so the span is 0 — initAudio then loads nothing
 * and runs harmlessly (the al* synth is stubbed anyway). The *tbl* symbols are bank-table
 * bases (alBnkfNew, stubbed); a small zeroed buffer is enough. */
/* initialized (not `={...}`-less) so they are real .data symbols, not -fcommon commons —
 * aliases cannot target common symbols. */
u8 g_seqctl_seg[8] = {0}, g_sfxctl_seg[8] = {0};
extern u8 _seqctlSegmentRomStart[8] __attribute__((alias("g_seqctl_seg")));
extern u8 _seqctlSegmentRomEnd[8]   __attribute__((alias("g_seqctl_seg")));  /* End==Start -> span 0 */
extern u8 _sfxctlSegmentRomStart[8] __attribute__((alias("g_sfxctl_seg")));
extern u8 _sfxctlSegmentRomEnd[8]   __attribute__((alias("g_sfxctl_seg")));
u8 _seqtblSegmentRomStart[64];
u8 _sfxtblSegmentRomStart[64];

/* Microcode text/data blobs. The host never runs the RSP/RDP — the gfx display list is
 * intercepted and interpreted by Fast3D (turok_rcp.c), and audio by the software mixer —
 * so these only need to exist as valid pointers for the task-setup code. libultra declares
 * them as `long long int []` (ucode words), so match that to avoid type conflicts. */
long long int gspF3DEX_NoN_fifoTextStart[1], gspF3DEX_NoN_fifoDataStart[1];
long long int gspF3DLX_NoN_fifoTextStart[1], gspF3DLX_NoN_fifoDataStart[1];
long long int gspL3DEX_fifoTextStart[1], gspL3DEX_fifoDataStart[1];
long long int aspMainTextStart[1], aspMainDataStart[1];
long long int rspbootTextStart[1], rspbootTextEnd[1];

/* ---- RNC (Rob Northen ProPack) decompressor entry points ------------------
 * On the N64 these were hand-written MIPS asm (rnc1.s/rnc2.s). The portable C decoder
 * UnpackRNC (unpack.c) reads the method byte from the RNC header and dispatches, so both
 * M1 and M2 wrappers route through it. Needed to decode the cart's RNC assets at runtime. */
extern unsigned short UnpackRNC(void *FilePtr, unsigned char *OutputBuffer);

int Propack_UnpackM1(void *src, void *dst) { return UnpackRNC(src, (unsigned char *)dst); }
int Propack_UnpackM2(void *src, void *dst) { return UnpackRNC(src, (unsigned char *)dst); }

/* ---- libultra internals the game references (fault/FPU control) ------------
 * Diagnostics only on N64; harmless no-ops on the host. */
OSThread *__osGetCurrFaultedThread(void) { return (OSThread *)0; }
u32 __osGetFpcCsr(void)        { return 0; }
u32 __osSetFpcCsr(u32 fpccsr)  { (void)fpccsr; return 0; }
