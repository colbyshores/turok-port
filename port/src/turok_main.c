/* turok_main.c — host entry point + boot driver (CLAUDE.md §4, milestone M1).
 *
 * On the N64 the game boots through libultra threads:
 *     boot()      -> CEngineApp__Boot : OS/app init, creates+starts idleThread, ASSERT(FALSE)
 *     idle()      -> CEngineApp__Idle : osViBlack, PI manager, creates+starts mainThread, becomes idle
 *     mainproc()  -> CEngineApp__Main : the per-frame game loop (never returns)
 *
 * Our os_shim is single-threaded cooperative: osStartThread is a no-op, and under SHIP_IT
 * the ASSERT(FALSE) after each start compiles to nothing — so each wrapper RETURNS after its
 * setup. The driver therefore just runs the chain by hand:
 *     romdataInit();  boot();  idle(NULL);  mainproc(NULL);
 * CEngineApp__Main loops forever; we escape it after TUROK_MAX_FRAMES via a longjmp from the
 * per-frame osViSwapBuffer hook, so headless bounded runs (and later frame capture) terminate.
 */
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#ifndef PLATFORM_3DS              /* watchdog = host pthread + glibc backtrace (no 3DS equivalent) */
#include <pthread.h>
#include <signal.h>
#include <execinfo.h>
#endif

/* game boot-chain entry wrappers (tengine.c) */
extern void boot(void);
extern void idle(void *);
extern void mainproc(void *);

/* romdata seam */
extern int romdataInit(void);

/* gfx bridge (turok_gfx.c -> Fast3D/OSMesa) */
extern void turokGfxInit(int w, int h);
extern int  turokGfxSavePng(const char *path);

/* host audio layer (audio.c) — the sink + the dedicated audio thread (PD/banjo contract) */
extern int  audioInit(void);
extern void audioThreadStart(void);
extern void audioThreadStop(void);
extern void audioClose(void);        /* flush/close the sink (finalize the WAV header) */

static jmp_buf  g_escape;
static long     g_frame      = 0;
static long     g_max_frames = 0;          /* 0 = run unbounded */
static long     g_capture_frame = -1;      /* -1 = no capture */
static const char *g_capture_path = "turok_frame.png";

/* Called from os_shim's osViSwapBuffer once per presented frame. */
void turokVideoSwap(void *frameBuf)
{
    (void)frameBuf;
    g_frame++;
    if (g_frame <= 8 || (g_frame % 60) == 0)
        fprintf(stderr, "[turok] frame %ld\n", g_frame);
#ifdef PLATFORM_3DS
    /* RW-loop render-liveness trace -> boot.log: if these appear, mainproc IS looping + presenting. */
    if (g_frame == 1 || g_frame == 5 || (g_frame % 120) == 0) {
        extern void plat3dsBootLog(const char *);
        char b[48];
        snprintf(b, sizeof b, "mainproc: presented frame %ld", g_frame);
        plat3dsBootLog(b);
    }
#endif
    /* NOTE: capture is done in turokGfxEndFrame (keyed to REAL render frames), NOT here —
     * turokVideoSwap is also called by the osRecvMesg frame-pump on non-render ticks. */
    if (g_max_frames && g_frame >= g_max_frames)
        longjmp(g_escape, 1);              /* bounded run complete */
}

#ifdef PLATFORM_3DS
static void turok_watchdog_start(void) {}   /* 3DS: no pthread/backtrace; on-device triage via plat3dsBootLog */
#else
/* TUROK_WATCHDOG=1: catch infinite-loop freezes (no crash dump) — gdb-attach is blocked by ptrace_scope.
 * A watchdog thread backtraces the MAIN thread if g_frame stops advancing for ~4s (= a spin in a sub-call). */
static pthread_t s_wd_main;
/* SIGUSR1 = backtrace the MAIN thread (the stalled one) then exit. SIGUSR2 = backtrace the AUDIO thread
 * (it commonly HOLDS the synth lock the main thread is blocked on, so the real loop is over there) then
 * RETURN (don't exit — let the main report + exit). */
static void wd_sig(int s) {
    const char *hdr = (s == SIGUSR2)
        ? "\n[WATCHDOG] AUDIO thread backtrace (it likely holds the synth lock):\n"
        : "\n[WATCHDOG] main thread STUCK in an infinite loop — backtrace:\n";
    void *bt[80]; int n;
    write(2, hdr, strlen(hdr));
    n = backtrace(bt, 80);
    backtrace_symbols_fd(bt, n, 2);
    if (s != SIGUSR2) _exit(42);
}
static void *wd_thread(void *a) {
    long last = -1; int stuck = 0;
    (void)a;
    for (;;) {
        struct timespec ts; ts.tv_sec = 1; ts.tv_nsec = 0; nanosleep(&ts, NULL);
        long cur = *(volatile long *)&g_frame;
        /* Don't arm until the game is actually running. The graphics/SDL2-GL init + first asset load run
         * BEFORE the first frame and can take well over the timeout (SDL2 shader/driver bring-up alone is
         * several seconds on some machines), during which g_frame stays 0 — that is NOT a lock-up. Only
         * monitor once frames have started advancing; an in-game freeze leaves g_frame > 0 and frozen. */
        if (cur <= 0) { stuck = 0; last = cur; continue; }
        if (cur == last) {
            if (++stuck >= 6) {
                /* backtrace the AUDIO thread first (it usually holds the synth lock), then MAIN + exit. */
                extern void audioThreadSignal(int sig);
                struct timespec p; p.tv_sec = 0; p.tv_nsec = 300000000L;  /* 300ms for the audio report */
                audioThreadSignal(SIGUSR2);
                nanosleep(&p, NULL);
                pthread_kill(s_wd_main, SIGUSR1);
                return NULL;
            }
        }
        else { stuck = 0; last = cur; }
    }
}
static void turok_watchdog_start(void) {
    pthread_t t;
    if (!getenv("TUROK_WATCHDOG")) return;
    s_wd_main = pthread_self();
    signal(SIGUSR1, wd_sig);
    signal(SIGUSR2, wd_sig);
    pthread_create(&t, NULL, wd_thread, NULL);
    fprintf(stderr, "[WATCHDOG] armed (backtraces the main + audio threads after ~6s of no frame progress)\n");
}
#endif  /* !PLATFORM_3DS (watchdog) */

/* 3DS boot-milestone trace to sdmc:/3ds/turok/boot.log (the on-device debugging channel; a no-op on PC). */
#ifdef PLATFORM_3DS
extern void plat3dsBootLog(const char *);
#define BL(s) plat3dsBootLog(s)
#else
#define BL(s) ((void)0)
#endif

int main(int argc, char **argv)
{
    const char *mf = getenv("TUROK_MAX_FRAMES");
    const char *cf = getenv("TUROK_CAPTURE_FRAME");
    const char *cp = getenv("TUROK_CAPTURE_PATH");
    (void)argc; (void)argv;

    /* Unbuffer stderr so a crash doesn't swallow the last (most diagnostic) lines —
     * release builds otherwise buffer it and lose the location on a segfault. */
#ifdef PLATFORM_3DS
    /* point the game's global `stderr` symbol (stderr_3ds.c) at the real newlib stream before boot() —
     * the game's port traces fprintf(stderr,...) through it. */
    { extern void turok3dsSetStderr(void *); extern void *plat3dsRealStderr(void);
      turok3dsSetStderr(plat3dsRealStderr()); }
#endif
    setvbuf(stderr, NULL, _IONBF, 0);
    BL("main: start");
    { extern void turokConfigLoad(void); turokConfigLoad(); }   /* load turok.cfg before gfx/input init */
    turok_watchdog_start();
    if (mf) g_max_frames = strtol(mf, NULL, 10);
    if (cf) g_capture_frame = strtol(cf, NULL, 10);
    if (cp) g_capture_path = cp;

    fprintf(stderr, "[turok] M2 host boot — max_frames=%ld capture_frame=%ld\n",
            g_max_frames, g_capture_frame);

    BL("main: -> romdataInit");
    if (romdataInit() != 0) {
        BL("main: romdataInit FAILED (no ROM on sdmc:/3ds/turok/?)");
        return 1;
    }
    BL("main: romdataInit ok -> turokGfxInit");
    turokGfxInit(320, 240);      /* Fast3D + Citro3D (3DS) */
    BL("main: turokGfxInit done");

    if (setjmp(g_escape) == 0) {
        /* boot() = CEngineApp__Boot (OS/app init). We then SKIP idle() — on the N64 the
         * idle thread does a little setup then spins forever as the lowest-priority thread
         * while the scheduler runs mainThread; its setup (osViBlack/osCreatePiManager/fault
         * thread) is all no-op/bypassed here. Call mainproc directly; the cooperative frame
         * pump in osRecvMesg drives CEngineApp__Main's loop. */
        fprintf(stderr, "[turok] -> boot()\n");
        BL("main: -> boot()");
        boot();
        BL("main: boot() done -> audioInit");
        audioInit();          /* open the host audio device on the main thread, then ... */
        BL("main: audioInit done -> audioThreadStart");
        audioThreadStart();   /* ... start the dedicated audio thread (post OS/app init) */
        BL("main: audioThreadStart done -> mainproc");
        fprintf(stderr, "[turok] -> mainproc() (idle bypassed)\n");
        mainproc(NULL);  /* CEngineApp__Main — game loop (escapes via turokVideoSwap longjmp) */
        fprintf(stderr, "[turok] mainproc returned (unexpected)\n");
    }

    audioThreadStop();    /* signal + join the audio thread before exit (no more pushes after) */
    audioClose();         /* then flush/close the device or finalize the WAV header */
    fprintf(stderr, "[turok] exited cleanly after %ld frame(s)\n", g_frame);
    return 0;
}
