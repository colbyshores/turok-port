/*
 * turok_amixer.c — host software interpreter ("mixer") for turok's CLASSIC libultra audio
 * Acmd list (M4 audio, S4). The N64 RSP turned the synth's Acmd list into PCM; on the host
 * turokAudioManagerFrame() (audiomgr.c) calls alAudioFrame() to build the list, then this.
 *
 * The DSP kernels (VADPCM decode, polyphase resample, envelope mixer, gain-mix, interleave)
 * are lifted from perfect_dark/port/src/mixer.c (the proven n_audio aXxxImpl scalar paths,
 * which are themselves the SM64/libultra reference math). The ONE structural difference is
 * the ABI: turok's CLASSIC ABI is STATEFUL — each DSP command takes its DMEM in/out/count
 * from a preceding aSetBuffer (A_SETBUFF), not from inline args (n_audio inlines them). So
 * this file adds the ~150-LOC SETBUFFER latch + the classic aSetVolume + the dispatch loop;
 * the inner sample math is unchanged.
 *
 * DMEM is emulated as a flat byte buffer (g_dmem); the synth's aSetBuffer offsets (turok's
 * AL_MAIN_L_OUT=1088, AL_AUX_R_OUT=2048, AL_DECODER_OUT=320, ...) are just indices into it,
 * so no offset constants are hardcoded here — whatever the SetBuffer says is used. DRAM
 * addresses (LOADBUFF/SAVEBUFF/LOADADPCM/state) arrive as real host pointers because
 * osVirtualToPhysical + K0_TO_PHYS are identity on host (see os_shim.c / R4300.h) and
 * __amDMA returns the sample pointer directly.
 *
 * Map cites: abi.h opcode/bit layout; load.c/resample.c/env.c/save.c/mainbus.c SetBuffer args.
 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <PR/ultratypes.h>
#include <PR/abi.h>          /* Acmd, A_* opcodes + A_INIT/A_LOOP/A_VOL/A_LEFT/A_AUX flags, *_STATE */

#define ROUND_UP_16(v) (((v) + 15) & ~15)
#define ROUND_UP_8(v)  (((v) +  7) & ~7)
#define ROUND_UP_32(v) (((v) + 31) & ~31)

/* ---- emulated RSP DMEM (flat scratch; synth uses offsets 0..~0x900) ---- */
#define DMEM_BYTES 0x1000
static union { uint8_t u8[DMEM_BYTES]; int16_t s16[DMEM_BYTES / 2]; } g_dmem;
#define DM_U8(a)  (g_dmem.u8  + ((a) & (DMEM_BYTES - 1)))
#define DM_S16(a) (g_dmem.s16 + (((a) & (DMEM_BYTES - 1)) >> 1))

/* ---- classic-ABI SETBUFFER latch ---- *
 * A_MAIN sets the in/out/count used by ADPCM/RESAMPLE/LOAD/SAVE/MIX/INTERLEAVE.
 * A_AUX additionally carries the envmixer's MAIN_R/AUX_L/AUX_R targets, overloaded
 * into its in/out/count fields (env.c:382-383). */
static uint16_t sb_in, sb_out, sb_count;              /* A_MAIN triple */
static uint16_t sb_aux_in, sb_aux_out, sb_aux_count;  /* A_AUX triple  */

/* ---- synth-carried mixer state (same set as PD's rspa) ---- */
static int16_t  s_vol[2], s_target[2];
static int32_t  s_rate[2];
static int16_t  s_vol_dry, s_vol_wet;
static int16_t *s_adpcm_loop_state;
static int16_t  s_adpcm_table[8][2][8];

/* S5 debug: volume-chain diagnostics (TUROK_AUDIO_DUMP) */
static int g_amix_dump = 0;
static int g_dbg_voldry = 0, g_dbg_inpk = 0, g_dbg_gain = 0;

static const int16_t resample_table[64][4] = {
    {0x0c39, 0x66ad, 0x0d46, 0xffdf}, {0x0b39, 0x6696, 0x0e5f, 0xffd8},
    {0x0a44, 0x6669, 0x0f83, 0xffd0}, {0x095a, 0x6626, 0x10b4, 0xffc8},
    {0x087d, 0x65cd, 0x11f0, 0xffbf}, {0x07ab, 0x655e, 0x1338, 0xffb6},
    {0x06e4, 0x64d9, 0x148c, 0xffac}, {0x0628, 0x643f, 0x15eb, 0xffa1},
    {0x0577, 0x638f, 0x1756, 0xff96}, {0x04d1, 0x62cb, 0x18cb, 0xff8a},
    {0x0435, 0x61f3, 0x1a4c, 0xff7e}, {0x03a4, 0x6106, 0x1bd7, 0xff71},
    {0x031c, 0x6007, 0x1d6c, 0xff64}, {0x029f, 0x5ef5, 0x1f0b, 0xff56},
    {0x022a, 0x5dd0, 0x20b3, 0xff48}, {0x01be, 0x5c9a, 0x2264, 0xff3a},
    {0x015b, 0x5b53, 0x241e, 0xff2c}, {0x0101, 0x59fc, 0x25e0, 0xff1e},
    {0x00ae, 0x5896, 0x27a9, 0xff10}, {0x0063, 0x5720, 0x297a, 0xff02},
    {0x001f, 0x559d, 0x2b50, 0xfef4}, {0xffe2, 0x540d, 0x2d2c, 0xfee8},
    {0xffac, 0x5270, 0x2f0d, 0xfedb}, {0xff7c, 0x50c7, 0x30f3, 0xfed0},
    {0xff53, 0x4f14, 0x32dc, 0xfec6}, {0xff2e, 0x4d57, 0x34c8, 0xfebd},
    {0xff0f, 0x4b91, 0x36b6, 0xfeb6}, {0xfef5, 0x49c2, 0x38a5, 0xfeb0},
    {0xfedf, 0x47ed, 0x3a95, 0xfeac}, {0xfece, 0x4611, 0x3c85, 0xfeab},
    {0xfec0, 0x4430, 0x3e74, 0xfeac}, {0xfeb6, 0x424a, 0x4060, 0xfeaf},
    {0xfeaf, 0x4060, 0x424a, 0xfeb6}, {0xfeac, 0x3e74, 0x4430, 0xfec0},
    {0xfeab, 0x3c85, 0x4611, 0xfece}, {0xfeac, 0x3a95, 0x47ed, 0xfedf},
    {0xfeb0, 0x38a5, 0x49c2, 0xfef5}, {0xfeb6, 0x36b6, 0x4b91, 0xff0f},
    {0xfebd, 0x34c8, 0x4d57, 0xff2e}, {0xfec6, 0x32dc, 0x4f14, 0xff53},
    {0xfed0, 0x30f3, 0x50c7, 0xff7c}, {0xfedb, 0x2f0d, 0x5270, 0xffac},
    {0xfee8, 0x2d2c, 0x540d, 0xffe2}, {0xfef4, 0x2b50, 0x559d, 0x001f},
    {0xff02, 0x297a, 0x5720, 0x0063}, {0xff10, 0x27a9, 0x5896, 0x00ae},
    {0xff1e, 0x25e0, 0x59fc, 0x0101}, {0xff2c, 0x241e, 0x5b53, 0x015b},
    {0xff3a, 0x2264, 0x5c9a, 0x01be}, {0xff48, 0x20b3, 0x5dd0, 0x022a},
    {0xff56, 0x1f0b, 0x5ef5, 0x029f}, {0xff64, 0x1d6c, 0x6007, 0x031c},
    {0xff71, 0x1bd7, 0x6106, 0x03a4}, {0xff7e, 0x1a4c, 0x61f3, 0x0435},
    {0xff8a, 0x18cb, 0x62cb, 0x04d1}, {0xff96, 0x1756, 0x638f, 0x0577},
    {0xffa1, 0x15eb, 0x643f, 0x0628}, {0xffac, 0x148c, 0x64d9, 0x06e4},
    {0xffb6, 0x1338, 0x655e, 0x07ab}, {0xffbf, 0x11f0, 0x65cd, 0x087d},
    {0xffc8, 0x10b4, 0x6626, 0x095a}, {0xffd0, 0x0f83, 0x6669, 0x0a44},
    {0xffd8, 0x0e5f, 0x6696, 0x0b39}, {0xffdf, 0x0d46, 0x66ad, 0x0c39}
};

static inline int16_t clamp16(int32_t v) {
    if (v < -0x8000) return -0x8000;
    if (v >  0x7fff) return  0x7fff;
    return (int16_t)v;
}

/* ---- memory ops (DMEM offsets are indices; DRAM args are real host pointers) ---- */
static void k_clear(uint16_t addr, int nbytes) {
    memset(DM_U8(addr), 0, ROUND_UP_16(nbytes));
}
static void k_dmemmove(uint16_t in, uint16_t out, int nbytes) {
    memmove(DM_U8(out), DM_U8(in), ROUND_UP_16(nbytes));
}
static void k_loadbuffer(const void *src, uint16_t dst_dmem, uint16_t nbytes) {
    memcpy(DM_U8(dst_dmem), src, ROUND_UP_8(nbytes));
}
static void k_savebuffer(uint16_t src_dmem, int16_t *dst, uint16_t nbytes) {
    memcpy(dst, DM_S16(src_dmem), ROUND_UP_8(nbytes));
}
static void k_loadadpcm(int nbytes, const int16_t *book) {
    if (nbytes > (int)sizeof(s_adpcm_table)) nbytes = sizeof(s_adpcm_table);
    memcpy(s_adpcm_table, book, nbytes);
}
static void k_setloop(int16_t *state) { s_adpcm_loop_state = state; }

/* VADPCM decode: DMEM[in] (4-bit nibbles) -> DMEM[out] (s16). Scalar path from PD mixer.c. */
static void k_adpcm(uint8_t flags, int16_t *state, int nbytes, uint16_t inofs, uint16_t outofs) {
    uint8_t *in  = DM_U8(inofs);
    int16_t *out = DM_S16(outofs);
    nbytes = ROUND_UP_32(nbytes);
    if (flags & A_INIT)       memset(out, 0, 16 * sizeof(int16_t));
    else if (flags & A_LOOP)  memcpy(out, s_adpcm_loop_state, 16 * sizeof(int16_t));
    else                      memcpy(out, state, 16 * sizeof(int16_t));
    out += 16;
    while (nbytes > 0) {
        int shift = *in >> 4;             /* 0..12 */
        int table_index = *in++ & 0xf;    /* 0..7  */
        int16_t (*tbl)[8] = s_adpcm_table[table_index];
        int i;
        for (i = 0; i < 2; i++) {
            int16_t ins[8];
            int16_t prev1 = out[-1];
            int16_t prev2 = out[-2];
            int j, k;
            for (j = 0; j < 4; j++) {
                ins[j * 2]     = (((*in   >> 4) << 28) >> 28) << shift;
                ins[j * 2 + 1] = (((*in++ & 0xf) << 28) >> 28) << shift;
            }
            for (j = 0; j < 8; j++) {
                int32_t acc = tbl[0][j] * prev2 + tbl[1][j] * prev1 + (ins[j] << 11);
                for (k = 0; k < j; k++)
                    acc += tbl[1][((j - k) - 1)] * ins[k];
                acc >>= 11;
                *out++ = clamp16(acc);
            }
        }
        nbytes -= 16 * sizeof(int16_t);
    }
    memcpy(state, out - 16, 16 * sizeof(int16_t));
}

/* Polyphase resample DMEM[in] -> DMEM[out], `pitch` 16.16 (0x8000=1.0). Scalar path from PD. */
static void k_resample(uint8_t flags, uint16_t pitch, int16_t *state,
                       uint16_t inofs, uint16_t outofs, uint16_t count) {
    int16_t tmp[16];
    int16_t *in_initial = DM_S16(inofs);
    int16_t *in = in_initial;
    int16_t *out = DM_S16(outofs);
    int nbytes = ROUND_UP_16(count);
    uint32_t pitch_accumulator;
    const int16_t *tbl;
    int32_t sample;
    int i;
    if (flags & A_INIT) memset(tmp, 0, 5 * sizeof(int16_t));
    else                memcpy(tmp, state, 16 * sizeof(int16_t));
    if (flags & 2) {
        memcpy(in - 8, tmp + 8, 8 * sizeof(int16_t));
        in -= tmp[5] / (int)sizeof(int16_t);
    }
    in -= 4;
    pitch_accumulator = (uint16_t)tmp[4];
    memcpy(in, tmp, 4 * sizeof(int16_t));
    do {
        for (i = 0; i < 8; i++) {
            tbl = resample_table[pitch_accumulator * 64 >> 16];
            sample = ((in[0] * tbl[0] + 0x4000) >> 15) +
                     ((in[1] * tbl[1] + 0x4000) >> 15) +
                     ((in[2] * tbl[2] + 0x4000) >> 15) +
                     ((in[3] * tbl[3] + 0x4000) >> 15);
            *out++ = clamp16(sample);
            pitch_accumulator += (pitch << 1);
            in += pitch_accumulator >> 16;
            pitch_accumulator %= 0x10000;
        }
        nbytes -= 8 * sizeof(int16_t);
    } while (nbytes > 0);
    state[4] = (int16_t)pitch_accumulator;
    memcpy(state, in, 4 * sizeof(int16_t));
    i = (in - in_initial + 4) & 7;
    in -= i;
    if (i != 0) i = -8 - i;
    state[5] = i;
    memcpy(state + 8, in, 8 * sizeof(int16_t));
}

/* Envelope mixer: mono DMEM[in] -> dry L/R + wet L/R (accumulate), per-sample volume ramp.
 * DSP from PD aEnvMixerImpl; buffers rewired to the classic A_MAIN/A_AUX SETBUFFER triples. */
static void k_envmixer(uint8_t flags, int16_t *state) {
    int16_t *in     = DM_S16(sb_in);
    int16_t *dry[2] = { DM_S16(sb_out),     DM_S16(sb_aux_in)   };
    int16_t *wet[2] = { DM_S16(sb_aux_out), DM_S16(sb_aux_count) };
    int nsamples = sb_count >> 1;            /* count is bytes-per-channel; samples = /2 */
    struct {
        int32_t t[2];
        int32_t rate[2];
        int16_t tgt[2];
        int16_t voldry;
        int16_t volwet;
    } *ss = (void *)state;
    int32_t t[2], tgt[2], rate[2];
    int16_t voldry, volwet;
    int i, j;

    if (flags & A_INIT) {
        for (i = 0; i < 2; ++i) {
            t[i]    = s_vol[i] << 16;
            rate[i] = s_rate[i] >> 3;
            tgt[i]  = s_target[i] << 16;
        }
        voldry = s_vol_dry;
        volwet = s_vol_wet;
    } else {
        for (i = 0; i < 2; ++i) {
            t[i]    = ss->t[i];
            rate[i] = ss->rate[i];
            tgt[i]  = ss->tgt[i] << 16;
        }
        voldry = ss->voldry;
        volwet = ss->volwet;
    }

#define XOR 1   /* LE host; the index permutation cancels within the loop (read+write same i^1) */
    for (i = 0; i < nsamples; ++i) {
        int16_t gain[4];
        int16_t vol[2];
        int16_t *outptr[4];
        for (j = 0; j < 2; ++j) {
            t[j] += rate[j];
            if ((rate[j] <= 0 && t[j] <= tgt[j]) || (rate[j] > 0 && t[j] >= tgt[j])) {
                t[j] = tgt[j];
                rate[j] = 0;
            }
            vol[j] = t[j] >> 16;
        }
        outptr[0] = dry[0] + (i ^ XOR);
        outptr[1] = dry[1] + (i ^ XOR);
        outptr[2] = wet[0] + (i ^ XOR);
        outptr[3] = wet[1] + (i ^ XOR);
        gain[0] = clamp16((vol[0] * voldry + 0x4000) >> 15);
        gain[1] = clamp16((vol[1] * voldry + 0x4000) >> 15);
        gain[2] = clamp16((vol[0] * volwet + 0x4000) >> 15);
        gain[3] = clamp16((vol[1] * volwet + 0x4000) >> 15);
        {
            const int16_t insamp = in[i ^ XOR];
            if (g_amix_dump) {
                int a = insamp < 0 ? -insamp : insamp;
                if (a > g_dbg_inpk) g_dbg_inpk = a;
                if (gain[0] > g_dbg_gain) g_dbg_gain = gain[0];
            }
            for (j = 0; j < 4; ++j)
                *outptr[j] = clamp16(*outptr[j] + ((insamp * gain[j]) >> 15));
        }
    }
#undef XOR

    for (i = 0; i < 2; ++i) {
        ss->t[i]    = t[i];
        ss->rate[i] = rate[i];
        ss->tgt[i]  = tgt[i] >> 16;
    }
    ss->voldry = voldry;
    ss->volwet = volwet;
}

/* Gain-mix DMEM[in] into DMEM[out]: out = clamp16(out + (in*gain)>>15). From PD aMixImpl. */
static void k_mix(int16_t gain, uint16_t in_addr, uint16_t out_addr, int nbytes) {
    int16_t *in  = DM_S16(in_addr);
    int16_t *out = DM_S16(out_addr);
    int i;
    int32_t sample;
    nbytes = ROUND_UP_16(nbytes);
    if (gain == -0x8000) {
        while (nbytes > 0) {
            for (i = 0; i < 8; i++) { sample = *out - *in++; *out++ = clamp16(sample); }
            nbytes -= 8 * sizeof(int16_t);
        }
        return;
    }
    while (nbytes > 0) {
        for (i = 0; i < 8; i++) {
            sample = ((*out * 0x7fff + *in++ * gain) + 0x4000) >> 15;
            *out++ = clamp16(sample);
        }
        nbytes -= 8 * sizeof(int16_t);
    }
}

/* Interleave two mono DMEM buffers -> stereo at DMEM[out]. samples = count(bytes-per-ch)/2. */
static void k_interleave(uint16_t inL, uint16_t inR, uint16_t out, int count) {
    const int16_t *l = DM_S16(inL);
    const int16_t *r = DM_S16(inR);
    int16_t *d = DM_S16(out);
    int n = ROUND_UP_16(count >> 1);     /* samples per channel, rounded to 16 */
    while (n > 0) {
        *d++ = *l++;
        *d++ = *r++;
        n--;
    }
}

/* classic aSetVolume — populate the envmixer's vol/target/rate/dry/wet registers
 * (env.c emits 5 of these before each first-subframe aEnvMixer). Distinct from PD's
 * n_audio aSetVolumeImpl, whose flag packing differs. */
static void k_setvol(uint16_t flags, int16_t v, int16_t t, int16_t r) {
    if (flags & A_AUX) {                 /* dry/wet amounts */
        s_vol_dry = v;
        s_vol_wet = r;
        if (g_amix_dump && v > g_dbg_voldry) g_dbg_voldry = v;
    } else if (flags & A_VOL) {          /* current volume */
        if (flags & A_LEFT) s_vol[0] = v; else s_vol[1] = v;
    } else {                             /* A_RATE: target + 32-bit rate */
        int ch = (flags & A_LEFT) ? 0 : 1;
        s_target[ch] = v;
        s_rate[ch]   = (int32_t)(((uint32_t)(uint16_t)t << 16) | (uint16_t)r);
    }
}

/* ---- the dispatch loop ---- *
 * Decode each packed Acmd by hand (the synth built w0/w1 in native order on the host, so no
 * byte-swap). `out`/`nSamples` are unused: the final A_SAVEBUFF carries the real DRAM output
 * pointer (info->data), so the mix lands there directly. */
void turokAudioMixer(Acmd *list, s32 len, s16 *out, s32 nSamples) {
    static int  s_dump = -1;
    static long s_calls = 0, s_op[16];
    s32 i;
    (void)out; (void)nSamples;
    if (s_dump < 0) { const char *e = getenv("TUROK_AUDIO_DUMP"); s_dump = (e && atoi(e)) ? 1 : 0; g_amix_dump = s_dump; }
    for (i = 0; i < len; i++) {
        uint32_t w0 = list[i].words.w0;
        uint32_t w1 = list[i].words.w1;
        uint8_t  op = (uint8_t)(w0 >> 24);
        if (s_dump && op < 16) s_op[op]++;
        switch (op) {
        case A_SEGMENT:   break;                                    /* seg 0 identity */
        case A_SETBUFF: {
            uint8_t  f  = (w0 >> 16) & 0xff;
            uint16_t in = w0 & 0xffff, o = (w1 >> 16) & 0xffff, c = w1 & 0xffff;
            if (f & A_AUX) { sb_aux_in = in; sb_aux_out = o; sb_aux_count = c; }
            else           { sb_in = in;     sb_out = o;     sb_count = c;     }
            break;
        }
        case A_CLEARBUFF: k_clear(w0 & 0xffffff, w1); break;
        case A_DMEMMOVE:  k_dmemmove(w0 & 0xffffff, (w1 >> 16) & 0xffff, w1 & 0xffff); break;
        case A_LOADBUFF:  k_loadbuffer((const void *)(uintptr_t)w1, sb_in, sb_count); break;
        case A_SAVEBUFF:  k_savebuffer(sb_out, (int16_t *)(uintptr_t)w1, sb_count); break;
        case A_LOADADPCM: k_loadadpcm(w0 & 0xffffff, (const int16_t *)(uintptr_t)w1); break;
        case A_SETLOOP:   k_setloop((int16_t *)(uintptr_t)w1); break;
        case A_ADPCM:     k_adpcm((w0 >> 16) & 0xff, (int16_t *)(uintptr_t)w1, sb_count, sb_in, sb_out); break;
        case A_RESAMPLE:  k_resample((w0 >> 16) & 0xff, w0 & 0xffff,
                                     (int16_t *)(uintptr_t)w1, sb_in, sb_out, sb_count); break;
        case A_SETVOL:    k_setvol((w0 >> 16) & 0xffff, (int16_t)(w0 & 0xffff),
                                   (int16_t)((w1 >> 16) & 0xffff), (int16_t)(w1 & 0xffff)); break;
        case A_ENVMIXER:  k_envmixer((w0 >> 16) & 0xff, (int16_t *)(uintptr_t)w1); break;
        case A_MIXER:     k_mix((int16_t)(w0 & 0xffff), (w1 >> 16) & 0xffff, w1 & 0xffff, sb_count); break;
        case A_INTERLEAVE:k_interleave((w1 >> 16) & 0xffff, w1 & 0xffff, sb_out, sb_count); break;
        case A_POLEF:     break;   /* reverb low-pass — AL_FX_NONE, not emitted (stub) */
        default:          break;
        }
    }
    if (s_dump && (++s_calls % 50) == 0)
        fprintf(stderr, "[amix] %ld fr: ADPCM=%ld RESAMP=%ld ENVMIX=%ld MIX=%ld | inpk=%d gain=%d voldry=%d\n",
                s_calls, s_op[A_ADPCM], s_op[A_RESAMPLE], s_op[A_ENVMIXER], s_op[A_MIXER],
                g_dbg_inpk, g_dbg_gain, g_dbg_voldry);
}
