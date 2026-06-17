/*
 * turok_audio_thread.c — the dedicated host AUDIO THREAD (the N64 audio-manager
 * thread, made real on the host).
 *
 * WHY A THREAD: the N64 runs audio on its own thread — audiomgr.c does
 * `osCreateThread(&__am.thread, THREAD_AUDIO, __amMain, ...)` and the audio manager's
 * retrace loop drives the synth there, concurrently with the game/gfx threads. The
 * port made every libultra thread a cooperative no-op, so the audio thread never ran
 * and no PCM was ever produced. This file restores it as a REAL host thread, mirroring
 * the perfect_dark / banjo-kazooie / forsaken ports: game + gfx stay cooperative (one
 * frame per main-loop iteration), and ONLY audio runs concurrently — exactly the N64's
 * own split.
 *
 * STAGED BRING-UP (this is Stage 1). The loop body's `produce_one_frame()` is the
 * single seam that later stages swap:
 *   S1 (here):  a 440 Hz test tone / silence — proves thread + mutex + poll-paced
 *               refill loop + push-to-sink + clean join, with ZERO dependence on the
 *               synth, the banks, or the mixer (the three high-risk pieces).
 *   S3+:        produce_one_frame() -> amgrPumpOneFrame() -> alAudioFrame (synth ->
 *               packed Acmd list) -> turokAcmdRun (the classic-ABI mixer -> PCM) ->
 *               turokAudioPush. The threading below never changes again.
 *
 * SYNC: one coarse RECURSIVE mutex (`synthLock`), exactly like all three siblings
 * (PD/Banjo `sSynthLock`, Forsaken `LightLock`). The game thread takes it around the
 * synth's voice-list mutations (the alSndp / alCSP event sites — wired at S3); a
 * lock-free ring is unnecessary because the device queue depth IS the back-pressure.
 *
 * GATE: TUROK_AUDIO_THREAD=0 disables the thread (debugging / A-B).
 *       TUROK_AUDIO_TESTTONE=1 makes S1 emit an audible 440 Hz tone (default: silence,
 *       which still exercises the push path so the device stays fed for S2+).
 *
 * PRIMITIVE: pthreads (not SDL_Thread) so the thread works in EVERY PC backend —
 * including the headless GFX=egl/osmesa builds used for WAV verification, which don't
 * link SDL. The 3DS port (M3) swaps pthread->threadCreate + SDL->ndsp; the loop shape,
 * the lock, and the producer split are identical (see port/README.md / the playbook).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <time.h>

/* the output sink (turok_audio.c): PCM -> SDL device queue, or WAV dump when headless */
extern void         turokAudioPush(const void *pcm, int nbytes);
extern unsigned int turokAudioQueuedBytes(void);

#define TKA_RATE         22050
#define TKA_FRAME_SAMP   512            /* stereo frames produced per synth pump (~23 ms) */
#define TKA_QUEUE_LIMIT  16384          /* keep ~0.18 s buffered; refill below this depth */
#define TKA_REFILL_GUARD 64             /* cap frames/iteration so a non-backing sink (WAV) can't spin forever */

static pthread_t       s_thr;
static pthread_mutex_t s_lock;          /* the synthLock (recursive) */
static volatile int    s_exit    = 0;
static int             s_started  = 0;
static int             s_testtone = 0;
static double          s_phase    = 0.0;

/* S1 frame producer. Later stages REPLACE this body with the real synth+mixer pump;
 * the surrounding thread/lock/refill machinery stays exactly as-is. */
static void produce_one_frame(void)
{
    short pcm[TKA_FRAME_SAMP * 2];
    if (s_testtone) {
        const double step = 2.0 * 3.14159265358979323846 * 440.0 / TKA_RATE;
        int i;
        for (i = 0; i < TKA_FRAME_SAMP; i++) {
            short v = (short)(8000.0 * sin(s_phase));   /* moderate level, not full-scale */
            pcm[i * 2 + 0] = v;
            pcm[i * 2 + 1] = v;
            s_phase += step;
            if (s_phase > 6.283185307179586) s_phase -= 6.283185307179586;
        }
    } else {
        memset(pcm, 0, sizeof(pcm));     /* silence — still drives the push path for S2+ */
    }
    turokAudioPush(pcm, (int)sizeof(pcm));
}

static void tka_msleep(long ms)
{
    struct timespec ts;
    ts.tv_sec  = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}

static void *audio_thread_main(void *arg)
{
    (void)arg;
    while (!s_exit) {
        pthread_mutex_lock(&s_lock);
        /* refill the device until the backlog is healthy. Back-pressure = the device
         * queue depth (turokAudioQueuedBytes); the guard bounds the WAV/null sink, which
         * never backs up. */
        {
            int n = 0;
            while (turokAudioQueuedBytes() < TKA_QUEUE_LIMIT && n++ < TKA_REFILL_GUARD)
                produce_one_frame();
        }
        pthread_mutex_unlock(&s_lock);
        tka_msleep(2);    /* poll-paced like the siblings — never spin */
    }
    return NULL;
}

/* Taken by the GAME thread around synth voice-list mutations (the alSndp / alCSP event
 * sites, wired at S3). No-ops until the thread is running. */
void audioSynthLock(void)   { if (s_started) pthread_mutex_lock(&s_lock); }
void audioSynthUnlock(void) { if (s_started) pthread_mutex_unlock(&s_lock); }

void audioThreadStart(void)
{
    const char *e;
    pthread_mutexattr_t attr;

    if (s_started) return;
    e = getenv("TUROK_AUDIO_THREAD");  if (e && atoi(e) == 0) { fprintf(stderr, "[audio] thread disabled (TUROK_AUDIO_THREAD=0)\n"); return; }
    e = getenv("TUROK_AUDIO_TESTTONE"); s_testtone = (e && atoi(e)) ? 1 : 0;

    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&s_lock, &attr);
    pthread_mutexattr_destroy(&attr);

    s_exit = 0;
    if (pthread_create(&s_thr, NULL, audio_thread_main, NULL) == 0) {
        s_started = 1;
        fprintf(stderr, "[audio] thread started%s\n", s_testtone ? " (S1 440Hz test tone)" : " (silent — awaiting synth)");
    } else {
        pthread_mutex_destroy(&s_lock);
        fprintf(stderr, "[audio] pthread_create failed — audio thread disabled\n");
    }
}

void audioThreadStop(void)
{
    if (!s_started) return;
    s_exit = 1;
    pthread_join(s_thr, NULL);
    pthread_mutex_destroy(&s_lock);
    s_started = 0;
}
