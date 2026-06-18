/*
 * audio.c — host audio SINK + the dedicated audio thread.
 *
 * Matches the perfect_dark / banjo-kazooie port audio layer (port/include/audio.h) so the
 * whole thing can be lifted into a shared reusable N64->PC/3DS library later:
 *   - The SINK is the same stash (audioSetNextBuffer) + push-with-back-pressure
 *     (audioEndFrame) model over SDL_QueueAudio (callback=NULL) as PD's audio.c. Headless
 *     builds (GFX=egl/osmesa, no SDL) or TUROK_AUDIO_WAV=path dump 16-bit stereo PCM to a WAV.
 *   - The dedicated AUDIO THREAD mirrors PD's audio_3ds.c thread: a recursive synthLock + a
 *     poll-paced refill loop, gated by TUROK_AUDIO_THREAD. PD spins it only on 3DS (desktop
 *     inline); turok runs it on desktop too (pthread, so it works in every backend incl.
 *     headless), per this port's "audio on its own thread" goal. The N64 already ran audio
 *     on a dedicated thread (audiomgr.c osCreateThread(THREAD_AUDIO)); this restores it while
 *     game/gfx stay cooperative.
 *
 * STAGED (M4): audio_synth_frame() is the SINGLE seam later stages swap from the S1 test
 * tone / silence to the real synth — at S3 its body becomes amgrFrame() (the naudio synth,
 * which calls osAiSetNextBuffer -> audioSetNextBuffer itself); the sink + threading below
 * never change again.
 *
 * Format: 16-bit signed, 2ch interleaved L/R, native-endian = AUDIO_S16SYS (no swap out).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <time.h>

#if defined(GFX_USE_SDL2)
#include <SDL.h>
#endif

#include "audio.h"   /* pulls <PR/ultratypes.h> — included AFTER the libc headers: the vendored
                      * libultra include dir shadows some glibc headers, so size_t etc. must be
                      * seen first (the documented port include-order rule). */

#define AUDIO_RATE          44100               /* MUST equal the game's OUTPUT_RATE (audio.h) AND the
                                                * bank sampleRate (44100): the classic synth applies no
                                                * runtime sample-rate correction, so output rate != bank
                                                * rate = every sound at the wrong speed. */
#define AUDIO_FRAME_SAMPLES 512                /* stereo frames produced per synth pump */
#define AUDIO_QUEUE_LIMIT   8192               /* samples; refill below this (PD uses 8192) */
#define AUDIO_REFILL_GUARD  64                 /* cap frames/iter so a non-backing sink (WAV) can't spin */

static int        s_rate    = AUDIO_RATE;
static int        s_enabled = 1;
static const s16 *s_nextBuf  = NULL;
static u32        s_nextSize  = 0;

#if defined(GFX_USE_SDL2)
static SDL_AudioDeviceID s_dev = 0;
#endif

/* headless WAV dump (TUROK_AUDIO_WAV) */
static FILE *s_wav = NULL;
static long  s_wav_bytes = 0;
static long long s_produce_start_ns = 0;   /* first produced frame; paces ANY non-device sink to real-time */
static long long s_produced_bytes  = 0;    /* total bytes the synth has produced (sink-agnostic pacer) */

static long long audio_now_ns(void)
{
    struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

static void wav_header(FILE *f, int rate)
{
    unsigned char h[44]; unsigned int br = (unsigned)rate * 2 * 2;
    memset(h, 0, sizeof(h));
    memcpy(h + 0, "RIFF", 4); memcpy(h + 8, "WAVE", 4); memcpy(h + 12, "fmt ", 4);
    h[16] = 16; h[20] = 1; h[22] = 2;
    h[24]=(unsigned char)rate; h[25]=(unsigned char)(rate>>8); h[26]=(unsigned char)(rate>>16); h[27]=(unsigned char)(rate>>24);
    h[28]=(unsigned char)br;   h[29]=(unsigned char)(br>>8);   h[30]=(unsigned char)(br>>16);   h[31]=(unsigned char)(br>>24);
    h[32] = 4; h[34] = 16; memcpy(h + 36, "data", 4);
    fwrite(h, 1, sizeof(h), f);
}

/* ---- the sink (the 5-function PD/banjo audio.c contract) ---- */

s32 audioInit(void)
{
    const char *e = getenv("TUROK_NOAUDIO");
    if (e && atoi(e)) { s_enabled = 0; return 0; }

    /* TUROK_AUDIO_WAV takes precedence (headless verification, any backend incl. GFX=sdl2). */
    e = getenv("TUROK_AUDIO_WAV");
    if (e && *e) {
        s_wav = fopen(e, "wb");
        if (s_wav) { wav_header(s_wav, s_rate); s_wav_bytes = 0;
            fprintf(stderr, "[audio] WAV dump -> %s (%d Hz)\n", e, s_rate); }
        return s_wav ? 0 : -1;
    }

#if defined(GFX_USE_SDL2)
    {
        SDL_AudioSpec want, have;
        if (SDL_WasInit(SDL_INIT_AUDIO) == 0 && SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
            fprintf(stderr, "[audio] SDL audio init error: %s\n", SDL_GetError()); return -1;
        }
        SDL_zero(want);
        want.freq = s_rate; want.format = AUDIO_S16SYS; want.channels = 2;
        want.samples = AUDIO_FRAME_SAMPLES; want.callback = NULL;
        s_dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
        if (!s_dev) { fprintf(stderr, "[audio] SDL_OpenAudioDevice error: %s\n", SDL_GetError()); return -1; }
        s_rate = have.freq;
        SDL_PauseAudioDevice(s_dev, 0);
        fprintf(stderr, "[audio] SDL device open: %d Hz 16-bit stereo\n", s_rate);
        return 0;
    }
#else
    return -1;   /* no SDL + no WAV -> silent (headless without TUROK_AUDIO_WAV) */
#endif
}

s32 audioGetBytesBuffered(void)
{
#if defined(GFX_USE_SDL2)
    if (s_dev) return (s32)SDL_GetQueuedAudioSize(s_dev);
#endif
    /* Sink-agnostic synthetic back-pressure (the WAV dump AND the NOAUDIO/null sink — neither has a
     * real device queue): model production as a device draining at the sample rate, so the audio
     * thread produces at ~real-time instead of flat-out. Flat-out spins a core and hogs the synth
     * lock, starving the game thread — a headless-only artifact (the SDL device path returns above
     * with the real queue depth, so user gameplay is unaffected). */
    if (s_produce_start_ns) {
        long long elapsed = audio_now_ns() - s_produce_start_ns;
        long long drained = (long long)((double)elapsed * 1e-9 * (double)s_rate) * 4;
        long long backed  = s_produced_bytes - drained;
        return backed > 0 ? (s32)backed : 0;
    }
    return 0;
}

s32 audioGetSamplesBuffered(void) { return audioGetBytesBuffered() / 4; }

void audioSetNextBuffer(const s16 *buf, u32 len)
{
    s_nextBuf = buf; s_nextSize = len;
    if (!s_produce_start_ns) s_produce_start_ns = audio_now_ns();
    s_produced_bytes += len;   /* drives the sink-agnostic real-time pacer in audioGetBytesBuffered */
}

void audioEndFrame(void)
{
    /* Always flush the produced frame. Back-pressure lives in the PRODUCER (the thread loop's
     * audioGetSamplesBuffered() < LIMIT gate decides WHETHER to produce a frame); re-checking it
     * here would DROP every frame produced right at the LIMIT boundary — and with the real-time
     * pacer holding the buffer at ~LIMIT, that's nearly all of them, starving the WAV/device. */
    if (s_nextBuf && s_nextSize && s_enabled) {
#if defined(GFX_USE_SDL2)
        if (s_dev) SDL_QueueAudio(s_dev, s_nextBuf, s_nextSize);
        else
#endif
        if (s_wav) { fwrite(s_nextBuf, 1, s_nextSize, s_wav); s_wav_bytes += s_nextSize; }
    }
    s_nextBuf = NULL; s_nextSize = 0;
}

void audioClose(void)
{
#if defined(GFX_USE_SDL2)
    if (s_dev) { SDL_CloseAudioDevice(s_dev); s_dev = 0; }
#endif
    if (s_wav) {
        long riff = 36 + s_wav_bytes; unsigned char v[4];
        v[0]=(unsigned char)riff; v[1]=(unsigned char)(riff>>8); v[2]=(unsigned char)(riff>>16); v[3]=(unsigned char)(riff>>24);
        fseek(s_wav, 4, SEEK_SET);  fwrite(v, 1, 4, s_wav);
        v[0]=(unsigned char)s_wav_bytes; v[1]=(unsigned char)(s_wav_bytes>>8); v[2]=(unsigned char)(s_wav_bytes>>16); v[3]=(unsigned char)(s_wav_bytes>>24);
        fseek(s_wav, 40, SEEK_SET); fwrite(v, 1, 4, s_wav);
        fclose(s_wav); s_wav = NULL;
    }
}

/* ---- the dedicated audio thread (the PD audio_3ds.c contract) ---- */

static pthread_t       s_thr;
static pthread_mutex_t s_synthLock;
static volatile int    s_quit   = 0;
static int             s_active = 0;
static int             s_testtone = 0;
static double          s_phase  = 0.0;

void audioSynthLock(void)    { if (s_active) pthread_mutex_lock(&s_synthLock); }
void audioSynthUnlock(void)  { if (s_active) pthread_mutex_unlock(&s_synthLock); }
int  audioThreadActive(void) { return s_active; }

/* The single seam later stages swap. S1: a 440Hz tone / silence -> audioSetNextBuffer.
 * S3+: replace the body with amgrFrame() (the naudio synth writes PCM and calls
 * osAiSetNextBuffer -> audioSetNextBuffer itself); the loop below is unchanged. */
static void audio_synth_frame(void)
{
    static s16 frame[AUDIO_FRAME_SAMPLES * 2];
    if (s_testtone) {
        const double step = 2.0 * 3.14159265358979323846 * 440.0 / s_rate;
        int i;
        for (i = 0; i < AUDIO_FRAME_SAMPLES; i++) {
            s16 v = (s16)(8000.0 * sin(s_phase));
            frame[i * 2 + 0] = v;
            frame[i * 2 + 1] = v;
            s_phase += step;
            if (s_phase > 6.283185307179586) s_phase -= 6.283185307179586;
        }
        audioSetNextBuffer(frame, (u32)sizeof(frame));
        return;
    }
    /* S3/S4 (default, non-tone): drive the real synth + the classic-ABI Acmd mixer.
     * turokAudioManagerFrame() (audiomgr.c) synthesizes one frame straight into the
     * audio-heap output buffer and calls audioSetNextBuffer itself, so the thread loop's
     * audioEndFrame() pushes it to the device. This is the SINGLE S3 seam. The ready-gate
     * (set at the end of the game's initAudio) keeps us pushing silence until the synth +
     * players + banks exist — the audio thread is started before initAudio runs. */
    {
        extern int turok_audio_ready;
        extern void turokAudioManagerFrame(void);
        if (turok_audio_ready) { turokAudioManagerFrame(); return; }
    }
    memset(frame, 0, sizeof(frame));
    audioSetNextBuffer(frame, (u32)sizeof(frame));
}

static void audio_msleep(long ms)
{
    struct timespec ts; ts.tv_sec = ms / 1000; ts.tv_nsec = (ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}

static void *audioThreadMain(void *arg)
{
    (void)arg;
    while (!s_quit) {
        int n = 0;
        /* refill until the device backlog is healthy (back-pressure = queue depth), exactly
         * like PD audio_3ds.c's audioThreadMain. Lock PER frame (released between) so a game
         * thread producer isn't blocked for a whole multi-frame batch. */
        while (audioGetSamplesBuffered() < AUDIO_QUEUE_LIMIT && n++ < AUDIO_REFILL_GUARD) {
            pthread_mutex_lock(&s_synthLock);
            audio_synth_frame();   /* -> audioSetNextBuffer  (S3: amgrFrame) */
            audioEndFrame();       /* -> push to device / WAV */
            pthread_mutex_unlock(&s_synthLock);
        }
        audio_msleep(2);           /* poll-paced like PD (svcSleepThread) — never spin */
    }
    return NULL;
}

void audioThreadStart(void)
{
    const char *e;
    pthread_mutexattr_t attr;

    if (s_active) return;
    e = getenv("TUROK_AUDIO_THREAD");   if (e && atoi(e) == 0) { fprintf(stderr, "[audio] thread disabled (TUROK_AUDIO_THREAD=0)\n"); return; }
    e = getenv("TUROK_AUDIO_TESTTONE"); s_testtone = (e && atoi(e)) ? 1 : 0;

    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&s_synthLock, &attr);
    pthread_mutexattr_destroy(&attr);

    s_quit = 0;
    if (pthread_create(&s_thr, NULL, audioThreadMain, NULL) == 0) {
        s_active = 1;
        fprintf(stderr, "[audio] thread started%s\n", s_testtone ? " (S1 440Hz test tone)" : " (silent — awaiting synth)");
    } else {
        pthread_mutex_destroy(&s_synthLock);
        fprintf(stderr, "[audio] pthread_create failed — audio thread disabled\n");
    }
}

void audioThreadStop(void)
{
    if (!s_active) return;
    s_quit = 1;
    pthread_join(s_thr, NULL);
    pthread_mutex_destroy(&s_synthLock);
    s_active = 0;
}
