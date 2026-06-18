/*
 * turok_audiobank.c — TRACKED host helpers for the audio banks (M4 audio, S2).
 *
 * Lives in port/src (tracked) because turoksnd/ is the untracked, user-supplied leaked audio SDK,
 * so the bank ENDIAN-SWAP and the missing alReverbSetType symbol can't be committed inside it.
 *
 *  - turokBnkfNew(): the host replacement for libaudio's alBnkfNew(). A .ctl (ALBankFile) is a
 *    big-endian N64 asset; on the LE host every offset/count/scalar in the
 *    ALBankFile -> ALBank -> ALInstrument -> ALSound -> ALWaveTable/ALEnvelope/ALADPCMBook/ALADPCMloop
 *    tree is byte-swapped DURING the relocation walk, AFTER each struct's `flags` guard (so a shared
 *    struct is swapped exactly once). The .tbl VADPCM payload stays raw big-endian (the ADPCM decoder
 *    reads it exactly as the N64 RSP did). The game's audio.c calls this instead of alBnkfNew under
 *    PLATFORM_PORT.
 *  - turokAudioLoadBank(): read a .ctl/.tbl file from disk into a malloc'd buffer (the N64 ROM
 *    segments are zero-span aliases on the host).
 *  - alReverbSetType(): referenced by turoksnd/abi/synsetfxtype.c (alSynSetFXtype) but defined
 *    nowhere in the leak — a no-op host stub resolves the link (reverb plays dry until ported).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <PR/ultratypes.h>
#include <PR/libaudio.h>

#define BSW16(x) ((s16)__builtin_bswap16((u16)(x)))
#define BSW32(x) ((s32)__builtin_bswap32((u32)(x)))

static void turok_patch_bank(ALBank *bank, s32 offset, s32 table);
static void turok_patch_inst(ALInstrument *inst, s32 offset, s32 table);
static void turok_patch_sound(ALSound *s, s32 offset, s32 table);
static void turok_patch_wavetable(ALWaveTable *w, s32 offset, s32 table);

/* Leaf structs the walk only points at (never recurses into) — swap their contents. */
static void turok_swap_envelope(ALEnvelope *e)
{
    if (!e) return;
    e->attackTime  = BSW32(e->attackTime);
    e->decayTime   = BSW32(e->decayTime);
    e->releaseTime = BSW32(e->releaseTime);
}
static void turok_swap_book(ALADPCMBook *b)
{
    s32 i, n;
    if (!b) return;
    b->order       = BSW32(b->order);
    b->npredictors = BSW32(b->npredictors);
    n = b->order * b->npredictors * 8;          /* coefficient count (s16) */
    if (n < 0 || n > 4096) return;              /* malformed/garbage book — don't over-read */
    for (i = 0; i < n; i++) b->book[i] = BSW16(b->book[i]);
}
static void turok_swap_loop(ALADPCMloop *l)
{
    s32 i, n; s16 *st;
    if (!l) return;
    l->start = BSW32(l->start);
    l->end   = BSW32(l->end);
    l->count = BSW32(l->count);
    st = (s16 *)l->state;
    n  = (s32)(sizeof(l->state) / sizeof(s16));
    for (i = 0; i < n; i++) st[i] = BSW16(st[i]);
}

void turokBnkfNew(ALBankFile *file, u8 *table)
{
    s32 offset = (s32)file, woffset = (s32)table, i;

    file->revision  = BSW16(file->revision);
    file->bankCount = BSW16(file->bankCount);
    for (i = 0; i < file->bankCount; i++) {
        file->bankArray[i] = (ALBank *)BSW32((s32)file->bankArray[i]);
        file->bankArray[i] = (ALBank *)((u8 *)file->bankArray[i] + offset);
        if (file->bankArray[i])
            turok_patch_bank(file->bankArray[i], offset, woffset);
    }
}

static void turok_patch_bank(ALBank *bank, s32 offset, s32 table)
{
    s32 i;
    if (bank->flags) return;
    bank->flags = 1;
    bank->instCount  = BSW16(bank->instCount);
    bank->sampleRate = BSW32(bank->sampleRate);
    if (bank->percussion) {
        bank->percussion = (ALInstrument *)BSW32((s32)bank->percussion);
        bank->percussion = (ALInstrument *)((u8 *)bank->percussion + offset);
        turok_patch_inst(bank->percussion, offset, table);
    }
    for (i = 0; i < bank->instCount; i++) {
        bank->instArray[i] = (ALInstrument *)BSW32((s32)bank->instArray[i]);
        bank->instArray[i] = (ALInstrument *)((u8 *)bank->instArray[i] + offset);
        if (bank->instArray[i])
            turok_patch_inst(bank->instArray[i], offset, table);
    }
}

static void turok_patch_inst(ALInstrument *inst, s32 offset, s32 table)
{
    s32 i;
    if (inst->flags) return;
    inst->flags = 1;
    inst->bendRange  = BSW16(inst->bendRange);
    inst->soundCount = BSW16(inst->soundCount);
    for (i = 0; i < inst->soundCount; i++) {
        inst->soundArray[i] = (ALSound *)BSW32((s32)inst->soundArray[i]);
        inst->soundArray[i] = (ALSound *)((u8 *)inst->soundArray[i] + offset);
        turok_patch_sound(inst->soundArray[i], offset, table);
    }
}

static void turok_patch_sound(ALSound *s, s32 offset, s32 table)
{
    if (s->flags) return;
    s->flags = 1;
    s->envelope  = (ALEnvelope *)BSW32((s32)s->envelope);
    s->keyMap    = (ALKeyMap *)BSW32((s32)s->keyMap);
    s->wavetable = (ALWaveTable *)BSW32((s32)s->wavetable);
    s->envelope  = (ALEnvelope *)((u8 *)s->envelope + offset);
    s->keyMap    = (ALKeyMap *)((u8 *)s->keyMap + offset);
    s->wavetable = (ALWaveTable *)((u8 *)s->wavetable + offset);
    turok_swap_envelope(s->envelope);          /* keyMap is all u8/s8 — no swap */
    turok_patch_wavetable(s->wavetable, offset, table);
}

static void turok_patch_wavetable(ALWaveTable *w, s32 offset, s32 table)
{
    ALADPCMBook *bk; ALADPCMloop *lp;
    if (w->flags) return;
    w->flags = 1;
    w->len  = BSW32(w->len);
    w->base = (u8 *)BSW32((s32)w->base);
    w->base += table;                          /* .tbl wave payload base (relocate, NOT swap) */
    /* ADPCM book: relocate + swap if present (NULL/0 for a RAW16 wave — leave it). */
    bk = (ALADPCMBook *)BSW32((s32)w->waveInfo.adpcmWave.book);
    if (bk) { bk = (ALADPCMBook *)((u8 *)bk + offset); turok_swap_book(bk); }
    w->waveInfo.adpcmWave.book = bk;
    /* ADPCM loop: optional. */
    lp = (ALADPCMloop *)BSW32((s32)w->waveInfo.adpcmWave.loop);
    if (lp) { lp = (ALADPCMloop *)((u8 *)lp + offset); turok_swap_loop(lp); }
    w->waveInfo.adpcmWave.loop = lp;
}

/* Load a .ctl/.tbl from disk into a malloc'd buffer (the N64 ROM segments are zero-span on the host). */
u8 *turokAudioLoadBank(const char *path, u32 *outLen)
{
    FILE *f = fopen(path, "rb");
    long sz; u8 *buf;
    if (outLen) *outLen = 0;
    if (!f) { fprintf(stderr, "[audio] bank file not found: %s\n", path); return NULL; }
    fseek(f, 0, SEEK_END); sz = ftell(f); fseek(f, 0, SEEK_SET);
    if (sz <= 0) { fclose(f); return NULL; }
    buf = (u8 *)malloc((size_t)sz);
    if (!buf) { fclose(f); return NULL; }
    if (fread(buf, 1, (size_t)sz, f) != (size_t)sz) { free(buf); fclose(f); return NULL; }
    fclose(f);
    if (outLen) *outLen = (u32)sz;
    fprintf(stderr, "[audio] loaded bank %s (%ld bytes)\n", path, sz);
    return buf;
}

/* Load a bank/sample chunk straight from the retail ROM at a fixed offset (Path B / TUROK_ROM). The dev
 * tree's sfx.ctl/.tbl are PLACEHOLDER (sports-announcer scratch samples reused during development); the
 * REAL Turok SFX + music banks are plain segments inside the retail ROM. The .ctl is big-endian
 * (turokBnkfNew swaps it); the .tbl is raw big-endian VADPCM. `expectBank` validates the ALBankFile "B1"
 * revision so a wrong offset / non-US-v1.2 ROM falls back to the dev bank instead of feeding garbage. */
u8 *turokAudioLoadBankFromROM(const char *rompath, long offset, u32 size, int expectBank)
{
    FILE *f = fopen(rompath, "rb");
    u8 *buf;
    if (!f) { fprintf(stderr, "[audio] ROM not found: %s\n", rompath); return NULL; }
    buf = (u8 *)malloc(size);
    if (!buf) { fclose(f); return NULL; }
    if (fseek(f, offset, SEEK_SET) != 0 || fread(buf, 1, size, f) != size) {
        free(buf); fclose(f); fprintf(stderr, "[audio] ROM read failed @0x%lx\n", offset); return NULL;
    }
    fclose(f);
    if (expectBank && !(buf[0] == 0x42 && buf[1] == 0x31)) {   /* not "B1" -> wrong offset/ROM version */
        fprintf(stderr, "[audio] ROM @0x%lx is not an ALBankFile (got %02x%02x) — using dev bank\n",
                offset, buf[0], buf[1]);
        free(buf); return NULL;
    }
    fprintf(stderr, "[audio] loaded RETAIL bank from ROM @0x%lx (%u bytes)\n", offset, size);
    return buf;
}

/* M4-S6 music: swap the on-disk ALCMidiHdr (16 u32 trackOffset + 1 u32 division = 17 DWORDs, 68 bytes)
 * to host order, in place, ONCE at sequence load (SeqReceived). alCSeqNew (cseq.c:47,60) reads those
 * track offsets + division RAW and walks ptr+offset into a track pointer — big-endian raw => garbage
 * offset => wild pointer => the music SIGSEGV. The compact-MIDI EVENT STREAM after the 68-byte header
 * is byte-oriented (status / var-len delta / data + byte-assembled tempo & loop offsets) — endian-
 * neutral, stays raw big-endian (same rule as the VADPCM .tbl payload). Mirrors turokBnkfNew. */
void turokCSeqHeaderSwap(u8 *ptr)
{
    u32 *w = (u32 *)ptr;
    int i;
    for (i = 0; i < 17; i++)            /* trackOffset[0..15] + division */
        w[i] = (u32)__builtin_bswap32(w[i]);
}

/* Referenced by turoksnd/abi/synsetfxtype.c (alSynSetFXtype), defined nowhere in the leak.
 * No-op resolves the link; reverb plays dry until a real implementation is ported. */
void alReverbSetType(void *fx, int fxid, int rate) { (void)fx; (void)fxid; (void)rate; }
