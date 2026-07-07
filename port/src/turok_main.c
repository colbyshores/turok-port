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
#ifndef PLATFORM_3DS              /* watchdog + crash-capture = host pthread + glibc backtrace (no 3DS equivalent) */
#include <pthread.h>
#include <signal.h>
#include <execinfo.h>
#include <fcntl.h>
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
#if defined(PLATFORM_PORT) && !defined(PLATFORM_3DS)
    /* TUROK_CRASHTEST=N: deliberately fault on the MAIN thread after N frames, to VERIFY the
     * crash-capture handler writes turok_crash.log. Tiny, env-gated, off unless the env var is set.
     * (default N=30 if TUROK_CRASHTEST=1). Remove/ignore in normal play. */
    { static long s_crashtest = -2;
      if (s_crashtest == -2) { const char *e = getenv("TUROK_CRASHTEST");
                               s_crashtest = (e && *e) ? strtol(e, NULL, 10) : -1;
                               if (s_crashtest == 1) s_crashtest = 30; }
      if (s_crashtest > 0 && g_frame >= s_crashtest) {
          fprintf(stderr, "[turok] TUROK_CRASHTEST: forcing a NULL deref on the main thread at frame %ld\n", g_frame);
          *(volatile int *)0 = 0;   /* -> SIGSEGV -> crash_handler */
      }
    }
#endif
    /* NOTE: capture is done in turokGfxEndFrame (keyed to REAL render frames), NOT here —
     * turokVideoSwap is also called by the osRecvMesg frame-pump on non-render ticks. */
    if (g_max_frames && g_frame >= g_max_frames)
        longjmp(g_escape, 1);              /* bounded run complete */
}

#ifdef PLATFORM_3DS
static void turok_watchdog_start(void) {}   /* 3DS: no pthread/backtrace; on-device triage via plat3dsBootLog */
static void turok_crash_capture_install(void) {}  /* 3DS: Luma crash dumps handle fatal faults */
#else
/* =====================================================================================
 * CRASH CAPTURE — a robust FATAL-SIGNAL handler for the RELEASE/play build.
 *
 * The user hits an INTERMITTENT crash (suspected: the audio/music thread) on level 3, and the
 * release build had NO crash handler — it just core-dumped with nothing to send back. This
 * catches SIGSEGV/SIGABRT/SIGBUS/SIGFPE/SIGILL and writes a self-contained crash report to a
 * FILE (default ./turok_crash.log, override with $TUROK_CRASHLOG) AND stderr, containing:
 *   - the signal name + fault address (siginfo si_addr)
 *   - WHICH THREAD faulted (MAIN vs NON-MAIN/audio — pthread_self vs the recorded main tid)
 *   - the current g_frame + the watchdog phase breadcrumb (g_turok_phase from os_shim.c)
 *   - a symbolized backtrace of the FAULTING thread (needs -rdynamic, already in build_port.sh)
 *   - the RAW return addresses so the user can addr2line against the exact binary
 *   - on a MAIN-thread fault, a best-effort backtrace of the AUDIO thread too (the suspect)
 *
 * Async-signal-safety: the report is written with open()/write()/backtrace_symbols_fd() only
 * (no fprintf/malloc on the hot path); a re-entrancy flag survives a crash-during-crash.
 * glibc backtrace() is not perfectly async-signal-safe but is the pragmatic standard here.
 * ===================================================================================== */
static pthread_t s_wd_main;                     /* the MAIN thread (recorded at install) */
extern volatile const char *g_turok_phase;      /* os_shim.c freeze/crash phase breadcrumb */
extern int  audioThreadActive(void);            /* audio.c */
extern void audioThreadSignal(int sig);         /* audio.c: pthread_kill the audio thread */

static volatile int          g_crash_fd = -1;   /* the crash-log fd during a fault (shared w/ the SIGUSR2 handler) */
static volatile sig_atomic_t g_in_crash = 0;    /* re-entrancy guard (crash during crash) */
static char                  g_crashlog_path[512] = "turok_crash.log";

/* --- tiny async-signal-safe writers (to stderr fd 2 AND the crash-log fd) --- */
static void cw(int fd, const char *s) { if (fd >= 0 && s) { size_t n = 0; while (s[n]) n++; if (write(fd, s, n) < 0) {} } }
static void cw_both(const char *s) { cw(2, s); cw(g_crash_fd, s); }
static void cw_hex(unsigned long v) {
    const int nib = (int)(sizeof(unsigned long) * 2);   /* 8 on -m32, 16 on 64-bit — avoids a >=width shift (UB) */
    char b[2 + 16 + 1]; int i; b[0]='0'; b[1]='x';
    for (i = 0; i < nib; i++) { int d = (int)((v >> ((nib - 1 - i) * 4)) & 0xf); b[2+i] = d < 10 ? ('0'+d) : ('a'+d-10); }
    b[2+nib] = 0; cw_both(b);
}
static void cw_dec(long v) {
    char b[24]; int i = 23; b[i--] = 0;
    unsigned long u = v < 0 ? (unsigned long)(-(v+1)) + 1UL : (unsigned long)v;
    if (u == 0) b[i--] = '0';
    while (u) { b[i--] = (char)('0' + (u % 10)); u /= 10; }
    if (v < 0) b[i--] = '-';
    cw_both(&b[i+1]);
}
static const char *crash_signame(int s) {
    switch (s) { case SIGSEGV: return "SIGSEGV (bad memory access)";
                 case SIGABRT: return "SIGABRT (abort/assert/heap-corruption)";
                 case SIGBUS:  return "SIGBUS (misaligned/bad bus access)";
                 case SIGFPE:  return "SIGFPE (div-by-zero/FP error)";
                 case SIGILL:  return "SIGILL (illegal instruction)";
                 default:      return "SIGNAL"; }
}
static void crash_backtrace_both(void) {
    void *bt[96]; int n = backtrace(bt, 96), i;
    backtrace_symbols_fd(bt, n, 2);                 /* symbolized (function names via -rdynamic) */
    if (g_crash_fd >= 0) backtrace_symbols_fd(bt, n, g_crash_fd);
    cw_both("raw pcs (addr2line -f -e <binary>): ");
    for (i = 0; i < n; i++) { cw_hex((unsigned long)bt[i]); cw_both(" "); }
    cw_both("\n");
}

/* SIGUSR2 = "backtrace the thread you land on" — the AUDIO thread is signaled with this both by the
 * crash handler (main-thread fault → dump the suspect audio thread) and by the freeze-watchdog (the
 * audio thread commonly HOLDS the synth lock the main thread is blocked on). Writes to both fds. */
static void crash_audio_bt(int s) {
    (void)s;
    cw_both("\n[BT] --- AUDIO thread backtrace ---\n");
    crash_backtrace_both();
}

/* The fatal-signal handler. Runs on the FAULTING thread (on a sigaltstack, so a stack-overflow
 * SIGSEGV still reports). */
static void crash_handler(int sig, siginfo_t *si, void *uc) {
    (void)uc;
    if (g_in_crash) { _exit(139); }     /* crash inside the crash report — bail immediately */
    g_in_crash = 1;
    signal(sig, SIG_DFL);               /* if we fault again, re-raise cleanly */

    g_crash_fd = open(g_crashlog_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    cw_both("\n==================== TUROK CRASH ====================\n");
    cw_both("signal:     "); cw_both(crash_signame(sig));
    cw_both("\nfault_addr: "); cw_hex((unsigned long)(si ? si->si_addr : 0)); cw_both("\n");
    int on_main = pthread_equal(pthread_self(), s_wd_main);
    cw_both("thread:     ");
    cw_both(on_main ? "MAIN (game/render loop)"
                    : "NON-MAIN (audio/music thread or a worker) <-- fault is OFF the main thread");
    cw_both("\ng_frame:    "); cw_dec(*(volatile long *)&g_frame);
    cw_both("\nphase:      "); cw_both(g_turok_phase ? (const char *)g_turok_phase : "?");
    cw_both("\nbinary:     addr2line against your build, e.g. /tmp/turok_sdl/turok\n");
    cw_both("\n[BT] --- FAULTING thread backtrace ---\n");
    crash_backtrace_both();

    /* Best-effort: on a MAIN-thread fault, also dump the AUDIO thread (the user's prime suspect). */
    if (on_main && audioThreadActive()) {
        cw_both("\n[BT] signaling the audio thread for its stack (best-effort)...\n");
        audioThreadSignal(SIGUSR2);
        struct timespec p; p.tv_sec = 0; p.tv_nsec = 300000000L; nanosleep(&p, NULL);  /* let it report */
    }
    cw_both("\n==================== END CRASH ======================\n");
    if (g_crash_fd >= 0) { int fd = g_crash_fd; g_crash_fd = -1; close(fd); }
    fprintf(stderr, "[turok] crash report written to %s\n", g_crashlog_path);
    _exit(134);
}

/* memcpy_guard.c (debug/asan builds only) installs its OWN SIGSEGV handler via a constructor. Don't
 * double-install SIGSEGV in that case — declare its wrap symbol weak and detect its presence. */
extern void *__wrap_memcpy(void *, const void *, size_t) __attribute__((weak));

static void turok_crash_capture_install(void) {
    /* dedicated alt stack so a stack-overflow SIGSEGV can still run the handler.
     * Fixed 128 KB — SIGSTKSZ is not a compile-time constant on modern glibc (can't size a static array). */
    static char altbuf[128 * 1024];
    static stack_t ss;
    const char *cl = getenv("TUROK_CRASHLOG");
    s_wd_main = pthread_self();
    if (cl && *cl) { size_t i = 0; for (; cl[i] && i < sizeof(g_crashlog_path) - 1; i++) g_crashlog_path[i] = cl[i]; g_crashlog_path[i] = 0; }

    ss.ss_sp = altbuf; ss.ss_size = sizeof(altbuf); ss.ss_flags = 0;
    sigaltstack(&ss, NULL);

    struct sigaction sa; memset(&sa, 0, sizeof sa);
    sa.sa_sigaction = crash_handler; sa.sa_flags = SA_SIGINFO | SA_ONSTACK; sigemptyset(&sa.sa_mask);
    int guard = (&__wrap_memcpy != 0);   /* debug/asan build: memcpy_guard owns SIGSEGV */
    if (!guard) sigaction(SIGSEGV, &sa, NULL);
    sigaction(SIGABRT, &sa, NULL);
    sigaction(SIGBUS,  &sa, NULL);
    sigaction(SIGFPE,  &sa, NULL);
    sigaction(SIGILL,  &sa, NULL);

    /* SIGUSR2 = audio-thread backtrace (shared by the crash handler AND the freeze-watchdog) */
    struct sigaction su; memset(&su, 0, sizeof su);
    su.sa_handler = crash_audio_bt; su.sa_flags = SA_RESTART; sigemptyset(&su.sa_mask);
    sigaction(SIGUSR2, &su, NULL);

    fprintf(stderr, "[crash] fatal-signal capture armed%s -> %s\n",
            guard ? " (SIGSEGV left to memcpy_guard)" : "", g_crashlog_path);
}

/* TUROK_WATCHDOG=1: catch infinite-loop freezes (no crash dump) — gdb-attach is blocked by ptrace_scope.
 * A watchdog thread backtraces the MAIN thread if g_frame stops advancing for ~4s (= a spin in a sub-call). */
/* SIGUSR1 = backtrace the MAIN thread (the stalled one) then exit. (SIGUSR2 = audio backtrace, installed
 * by turok_crash_capture_install above and shared.) */
static void wd_sig(int s) {
    (void)s;
    cw_both("\n[WATCHDOG] main thread STUCK in an infinite loop — backtrace:\n");
    crash_backtrace_both();
    _exit(42);
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
    s_wd_main = pthread_self();               /* also set by turok_crash_capture_install (harmless dup) */
    signal(SIGUSR1, wd_sig);                  /* SIGUSR2 (audio backtrace) is owned by the crash installer */
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
    /* Create sdmc:/3ds/turok before anything writes there (save/settings) — the CIA bundles the ROM in
     * RomFS so this folder may not exist on a fresh install, and fopen(w) won't create it. */
    { extern void plat3dsEnsureDataDir(void); plat3dsEnsureDataDir(); }
#endif
    setvbuf(stderr, NULL, _IONBF, 0);
    BL("main: start");
    { extern void turokConfigLoad(void); turokConfigLoad(); }   /* load turok.cfg before gfx/input init */
    turok_crash_capture_install();   /* always-on in release: catch a fatal fault -> turok_crash.log */
    turok_watchdog_start();          /* opt-in (TUROK_WATCHDOG=1): catch a freeze/spin */
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
    { int _gw = 320, _gh = 240;
#if defined(PLATFORM_PORT) && !defined(PLATFORM_3DS)
      /* Headless (EGL/OSMesa) render resolution — honor TUROK_WIN_W/H so a widescreen capture is possible
       * (the SDL2 window already uses g_cfg_win_w/h). Default stays 320x240. */
      { const char *ew = getenv("TUROK_WIN_W"), *eh = getenv("TUROK_WIN_H");
        if (ew && *ew) _gw = atoi(ew); if (eh && *eh) _gh = atoi(eh);
        if (_gw < 64) _gw = 320; if (_gh < 64) _gh = 240; }
#endif
      turokGfxInit(_gw, _gh); }   /* Fast3D + Citro3D (3DS) */
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
