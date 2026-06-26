/*
 * audio_3ds.c — Turok 3DS audio OUTPUT sink (ndsp) + the dedicated audio thread.
 *
 * The 3DS sibling of the PC port/src/audio.c: same port/include/audio.h contract, same
 * stash(audioSetNextBuffer)+push(audioEndFrame) sink model, same thread loop driving the real
 * synth (turokAudioManagerFrame) under a recursive synthLock. Only the device (ndsp instead of
 * SDL) and the threading primitive (libctru threadCreate/RecursiveLock instead of pthread) differ.
 *
 * Output rate = 22050 Hz = the retail ROM banks' stored sample rate (the synth plays ratio=1.0
 * native, so the device rate MUST equal the bank's stored rate; see CLAUDE.md). ndsp wave buffers
 * live in linear memory (DSP DMA). If ndspInit fails (no dspfirm dumped) the game still boots,
 * silent.
 */
#ifdef PLATFORM_3DS
#include <3ds.h>
#include <string.h>
#include <PR/ultratypes.h>
#include "audio.h"

#define OUTPUT_RATE_HZ     22050.0f
#define NUM_WAVE_BUFFERS   4
#define WAVE_BUFFER_BYTES  (16 * 1024)        /* ~4096 stereo S16 frames per buffer */
#define AUDIO_QUEUE_LIMIT  2048               /* samples buffered ahead = SFX trigger latency (~93ms) */
#define AUDIO_REFILL_GUARD 8                  /* cap frames produced per wake so we never spin */

extern int  turok_audio_ready;                /* set at end of initAudio — gate before the synth exists */
extern void turokAudioManagerFrame(void);     /* audiomgr.c — synth one frame -> audioSetNextBuffer */

static int          sReady    = 0;            /* ndsp opened OK */
static ndspWaveBuf  sWaveBufs[NUM_WAVE_BUFFERS];
static int          sCurBuf   = 0;

/* stash set by audioSetNextBuffer, pushed by audioEndFrame (the PD/PC model). */
static const s16   *sNext     = 0;
static u32          sNextBytes = 0;

/* --- sink ----------------------------------------------------------------- */

s32 audioGetBytesBuffered(void)
{
    s32 total = 0;
    int i;
    if (!sReady) return 0;
    for (i = 0; i < NUM_WAVE_BUFFERS; i++)
        if (sWaveBufs[i].status == NDSP_WBUF_QUEUED || sWaveBufs[i].status == NDSP_WBUF_PLAYING)
            total += (s32)sWaveBufs[i].nsamples * 4;   /* stereo S16 = 4 bytes/sample */
    return total;
}

s32 audioGetSamplesBuffered(void) { return audioGetBytesBuffered() / 4; }

s32 audioInit(void)
{
    int i;
    float mix[12];
    /* ★ 3DS BOOT: ndspInit() HANGS in Mandarine when the DSP firmware (dspfirm.cdc) isn't dumped —
     * it blocks on the DSP-ready sync instead of returning an error (the `!= 0` guard below assumes a
     * clean failure, which only happens on real HW / with firmware). Skip it unless explicitly enabled
     * via turok.cfg `audio_3ds 1`; sReady stays 0 so the whole audio path runs silent + the game boots.
     * Re-enable once the DSP firmware / Mandarine DSP-HLE path is sorted (then audio works). */
    { extern int g_cfg_audio_3ds; if (!g_cfg_audio_3ds) return 0; }
    if (ndspInit() != 0) {                      /* no dspfirm.cdc -> run silent, still boot */
        { extern void plat3dsLogv(const char*, ...); plat3dsLogv("[AUD] ndspInit FAILED -> silent"); }
        return 0;
    }

    ndspSetOutputMode(NDSP_OUTPUT_STEREO);
    ndspChnReset(0);
    ndspChnSetInterp(0, NDSP_INTERP_LINEAR);
    ndspChnSetRate(0, OUTPUT_RATE_HZ);
    ndspChnSetFormat(0, NDSP_FORMAT_STEREO_PCM16);
    memset(mix, 0, sizeof(mix));
    mix[0] = mix[1] = 1.0f;
    ndspChnSetMix(0, mix);

    memset(sWaveBufs, 0, sizeof(sWaveBufs));
    for (i = 0; i < NUM_WAVE_BUFFERS; i++) {
        sWaveBufs[i].data_vaddr = linearAlloc(WAVE_BUFFER_BYTES);
        sWaveBufs[i].status     = NDSP_WBUF_DONE;
    }
    sReady = 1;
    { extern void plat3dsLogv(const char*, ...); plat3dsLogv("[AUD] ndspInit OK, sReady=1, rate=%d", (int)OUTPUT_RATE_HZ); }
    return 0;
}

/* Stash the just-synthesized frame; audioEndFrame pushes it. (turokAudioManagerFrame, run on the
 * audio thread, calls this via osAiSetNextBuffer.) */
void audioSetNextBuffer(const s16 *buf, u32 len)
{
    sNext      = buf;
    sNextBytes = len;
}

/* Push the stashed frame into a free ndsp wave buffer. */
void audioEndFrame(void)
{
    ndspWaveBuf *wb;
    u32 n;
    if (!sReady || !sNext || sNextBytes == 0) { sNext = 0; sNextBytes = 0; return; }

    wb = &sWaveBufs[sCurBuf];
    if (wb->status == NDSP_WBUF_QUEUED || wb->status == NDSP_WBUF_PLAYING) {
        sNext = 0; sNextBytes = 0; return;     /* ring saturated — drop (back-pressure) */
    }
    n = sNextBytes;
    if (n > WAVE_BUFFER_BYTES) n = WAVE_BUFFER_BYTES;
    memcpy(wb->data_vaddr, sNext, n);
    DSP_FlushDataCache(wb->data_vaddr, n);
    wb->nsamples = n / 4;
    ndspChnWaveBufAdd(0, wb);
    sCurBuf = (sCurBuf + 1) % NUM_WAVE_BUFFERS;
    { extern void plat3dsLogv(const char*, ...); static int _p=0, _t=0;
      const s16 *s = (const s16*)wb->data_vaddr; u32 k, pk=0; u32 ns = wb->nsamples*2;
      for (k=0;k<ns;k++){ int v=s[k]; if(v<0)v=-v; if((u32)v>pk)pk=v; }
      _p++; if ((_t++ % 64)==0) plat3dsLogv("[AUD] sReady=%d push#%d bytes=%lu peak=%lu", sReady, _p, (unsigned long)n, (unsigned long)pk); }
    sNext = 0; sNextBytes = 0;
}

void audioClose(void)
{
    int i;
    if (!sReady) return;
    ndspChnWaveBufClear(0);
    for (i = 0; i < NUM_WAVE_BUFFERS; i++)
        if (sWaveBufs[i].data_vaddr) { linearFree((void*)sWaveBufs[i].data_vaddr); sWaveBufs[i].data_vaddr = 0; }
    ndspExit();
    sReady = 0;
}

/* --- dedicated audio thread ----------------------------------------------- */

static RecursiveLock sSynthLock;
static volatile int  sActive = 0;
static volatile int  sQuit   = 0;
static Thread        sThread;

void audioSynthLock(void)    { if (sActive) RecursiveLock_Lock(&sSynthLock); }
void audioSynthUnlock(void)  { if (sActive) RecursiveLock_Unlock(&sSynthLock); }
int  audioThreadActive(void) { return sActive; }

/* The single seam: silence until the synth/banks exist, then the real synth. */
static void audio_synth_frame(void)
{
    if (turok_audio_ready) { turokAudioManagerFrame(); return; }  /* -> audioSetNextBuffer */
    /* not ready: emit nothing (the thread loop's audioEndFrame is a no-op with no stash). */
}

static void audioThreadMain(void *arg)
{
    (void)arg;
    while (!sQuit) {
        int n = 0;
        while (audioGetSamplesBuffered() < AUDIO_QUEUE_LIMIT && n++ < AUDIO_REFILL_GUARD) {
            RecursiveLock_Lock(&sSynthLock);
            audio_synth_frame();   /* -> audioSetNextBuffer */
            audioEndFrame();       /* -> ndsp ring */
            RecursiveLock_Unlock(&sSynthLock);
        }
        svcSleepThread(2 * 1000000LL);   /* 2ms poll pacing — never spin */
    }
}

void audioThreadStart(void)
{
    s32 prio = 0x30;
    bool isN3ds = false;
    int  core;

    if (sActive) return;
    RecursiveLock_Init(&sSynthLock);
    sQuit = 0;

    APT_CheckNew3DS(&isN3ds);
    svcGetThreadPriority(&prio, CUR_THREAD_HANDLE);
    prio -= 1;                       /* slightly above the main thread */
    /* Get the synth OFF the game's core. New-3DS -> spare app core 2; OG-3DS -> system core 1
     * (needs APT_SetAppCpuTimeLimit). Fall back to the app core if the requested core refuses. */
    core = isN3ds ? 2 : 1;
    if (core == 1) APT_SetAppCpuTimeLimit(30);

    sThread = threadCreate(audioThreadMain, NULL, 256 * 1024, prio, core, false);
    if (!sThread)
        sThread = threadCreate(audioThreadMain, NULL, 256 * 1024, prio, -2, false);  /* app core */
    sActive = (sThread != NULL);
}

void audioThreadStop(void)
{
    if (!sActive) return;
    sQuit = 1;
    threadJoin(sThread, U64_MAX);
    threadFree(sThread);
    sThread = 0;
    sActive = 0;
}
#endif /* PLATFORM_3DS */
