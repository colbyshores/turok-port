/* os_shim.c — libultra (os*) shims for the Turok host port.
 *
 * Model (same as the Banjo/Perfect-Dark ports): the port is SINGLE-THREADED and
 * COOPERATIVE. osCreateThread/osStartThread record a thread but do not spawn one — the
 * host driver (turok_main.c) calls the game's entry chain directly. OSMesgQueue is a real
 * ring buffer so the game's producer/consumer handshakes resolve synchronously. The RCP
 * is virtual: osSpTaskLoad/osViSwapBuffer feed the Fast3D + present seam (turok_rcp.c, M2);
 * for M1 they just satisfy the handshake so the game boots headless.
 *
 * Includes ONLY <ultra64.h> — no game headers — so the libultra API types match the game.
 */
/* libc first: the vendored libultra include dir shadows some glibc headers, so the
 * standard types (size_t etc.) must be established before <ultra64.h>. */
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ultra64.h>
/* note: no <string.h> — glibc's <strings.h> bcopy/bzero conflict with libultra's
 * os_libc.h declarations. Use __builtin_mem* (no header, no conflict). */
#define memset __builtin_memset

/* romdata seam */
extern void romPiRead(void *dst, u32 devAddr, u32 nbytes);

/* ---- global libultra data the game reads -------------------------------- */
s32       osTvType   = OS_TV_NTSC;          /* drives refresh_rate = 60Hz in CEngineApp__Main */
void     *osRomBase  = (void *)0xB0000000;
OSViMode  osViModeTable[1];                 /* dummy; game configures VI via osViSetMode */

/* ---- threads (cooperative; entries invoked by the driver) --------------- */
void osCreateThread(OSThread *t, OSId id, void (*entry)(void *), void *arg,
                    void *sp, OSPri pri)
{
    if (!t) return;
    t->id = id; t->priority = pri; t->next = 0; t->queue = 0;
    /* stash entry/arg in the context PC/A0 slots so the driver can find them */
    t->context.pc = (u64)(uintptr_t)entry;
    t->context.a0 = (u64)(uintptr_t)arg;
    t->state = OS_STATE_STOPPED;
}
void  osStartThread(OSThread *t)              { if (t) t->state = OS_STATE_RUNNABLE; }
void  osStopThread(OSThread *t)               { if (t) t->state = OS_STATE_STOPPED; }
void  osDestroyThread(OSThread *t)            { (void)t; }
void  osYieldThread(void)                     { }
OSId  osGetThreadId(OSThread *t)              { return t ? t->id : 0; }
OSPri osGetThreadPri(OSThread *t)             { return t ? t->priority : 0; }
void  osSetThreadPri(OSThread *t, OSPri p)    { if (t) t->priority = p; }

/* ---- message queues (real synchronous ring buffer) ---------------------- */
void osCreateMesgQueue(OSMesgQueue *mq, OSMesg *msgBuf, s32 count)
{
    mq->mtqueue = (OSThread *)0;   /* no blocking in the cooperative model */
    mq->fullqueue = (OSThread *)0;
    mq->validCount = 0; mq->first = 0; mq->msgCount = count; mq->msg = msgBuf;
}
s32 osSendMesg(OSMesgQueue *mq, OSMesg msg, s32 flag)
{
    (void)flag;
    if (mq->validCount >= mq->msgCount) return -1;          /* full */
    mq->msg[(mq->first + mq->validCount) % mq->msgCount] = msg;
    mq->validCount++;
    return 0;
}
s32 osJamMesg(OSMesgQueue *mq, OSMesg msg, s32 flag)
{
    (void)flag;
    if (mq->validCount >= mq->msgCount) return -1;
    mq->first = (mq->first + mq->msgCount - 1) % mq->msgCount;
    mq->msg[mq->first] = msg; mq->validCount++;
    return 0;
}
/* Cooperative frame pump: the game's main loop blocks on osRecvMesg(frameQ, BLOCK)
 * waiting for the next vblank (OS_SC_RETRACE_MSG, a GFXMsg whose first short == 1).
 * With no interrupts we synthesize that message at the block point and advance one
 * frame. (OS_SC_RETRACE_MSG = 1; see PR/sched.h.) */
#define OS_SC_RETRACE_MSG 1
static short g_retrace_msg = OS_SC_RETRACE_MSG;

/* PORT: 1 on frames where the game LOGIC should advance, 0 on render-only frames. Read by
 * CEngineApp__UpdateGAME (tengine.c) to force frame_increment=0 when 0, decoupling the logic tick
 * rate from the render/present rate. See the TUROK_TICK_FPS block in osViSwapBuffer below. */
int g_turok_logic_tick = 1;
static struct timespec g_tick_last = {0, 0};   /* real time of the last logic tick */
static long g_tick_interval_ns = 0;            /* 1/TICK_FPS in ns; 0 = no logic-rate cap */

/* PORT: render-side interpolation factor (0..1) — how far the current real time is from the last
 * logic tick toward the next. CEngineApp__UpdateGAME renders the player at lerp(prev,cur,alpha) so
 * motion is smooth at the 60fps render rate despite the 30Hz logic. 0 when TUROK_TICK_FPS=0. */
float turok_render_alpha(void)
{
    if (g_tick_interval_ns <= 0 || g_tick_last.tv_sec == 0) return 0.0f;
    struct timespec now; clock_gettime(CLOCK_MONOTONIC, &now);
    long el = (now.tv_sec - g_tick_last.tv_sec) * 1000000000L + (now.tv_nsec - g_tick_last.tv_nsec);
    float a = (float)el / (float)g_tick_interval_ns;
    return a < 0.0f ? 0.0f : (a > 1.0f ? 1.0f : a);
}

s32 osRecvMesg(OSMesgQueue *mq, OSMesg *msg, s32 flag)
{
    if (mq->validCount == 0) {
        if (flag != OS_MESG_BLOCK) return -1;          /* NOBLOCK: nothing pending */
        /* BLOCK on empty == "wait for next frame": pace + tick + deliver a retrace.
         * Frame PACING (TUROK_FPS, default 60): the game's intro/attract state machine
         * advances by frame_increment per frame, so with no real timing it cycles the whole
         * sequence instantly and just spins, re-loading the scene every few frames (looks
         * "not responding"). Sleep to cap synthesized retraces to ~60Hz so it runs at real
         * speed. Set TUROK_FPS=0 to disable (fast/unpaced for bounded captures). */
        {
            static int s_fps = -1;
            if (s_fps < 0) { const char *e = getenv("TUROK_FPS"); s_fps = e ? atoi(e) : 60; }
            if (s_fps > 0) {
                static struct timespec last = {0, 0};
                struct timespec now; clock_gettime(CLOCK_MONOTONIC, &now);
                if (last.tv_sec) {
                    long tgt = 1000000000L / s_fps;
                    long el = (now.tv_sec - last.tv_sec) * 1000000000L + (now.tv_nsec - last.tv_nsec);
                    if (el < tgt) { struct timespec d = {0, tgt - el}; nanosleep(&d, NULL);
                                    clock_gettime(CLOCK_MONOTONIC, &now); }
                }
                last = now;
            }
        }
        extern void turokVideoSwap(void *);
        turokVideoSwap((void *)0);
        if (msg) *msg = (OSMesg)&g_retrace_msg;
        return 0;
    }
    if (msg) *msg = mq->msg[mq->first];
    mq->first = (mq->first + 1) % mq->msgCount;
    mq->validCount--;
    return 0;
}
/* PORT: remember the SI (controller) event registration so osContStartReadData can
 * post it — simulating the serial read completing — which drives the main loop's
 * CONTROLLER_MSG -> UpdateController -> osContGetReadData (input) path. */
static OSMesgQueue *g_si_mq  = 0;
static OSMesg       g_si_msg = 0;
void osSetEventMesg(OSEvent e, OSMesgQueue *mq, OSMesg msg)
{
    if (e == OS_EVENT_SI) { g_si_mq = mq; g_si_msg = msg; }
}

/* ---- PI / DMA (cartridge reads) → romdata seam -------------------------- */
void osCreatePiManager(OSPri pri, OSMesgQueue *cmdQ, OSMesg *cmdBuf, s32 n)
{ (void)pri;(void)cmdQ;(void)cmdBuf;(void)n; }

s32 osPiStartDma(OSIoMesg *mb, s32 pri, s32 dir, u32 devAddr, void *vAddr,
                 u32 nbytes, OSMesgQueue *mq)
{
    (void)pri; (void)dir;
    romPiRead(vAddr, devAddr, nbytes);
    if (mq) osSendMesg(mq, (OSMesg)mb, OS_MESG_NOBLOCK);   /* signal completion */
    return 0;
}
u32 osVirtualToPhysical(void *p) { return (u32)(uintptr_t)p & 0x1FFFFFFF; }

/* ---- SP (graphics/audio task) — virtual RCP: gfx task -> Fast3D ---------- */
extern void turokGfxRun(void *dl);          /* turok_gfx.c -> gfx_run (F3DEX interp) */
void osSpTaskLoad(OSTask *t)        { (void)t; }   /* load is a no-op; we run on StartGo */
void osSpTaskStartGo(OSTask *t)
{
    if (t && t->t.type == M_GFXTASK)
        turokGfxRun(t->t.data_ptr);             /* interpret this frame's display list */
    /* M_AUDTASK is handled by the software mixer (M4); ignored here. */
}
void osSpTaskYield(void)            { }
OSYieldResult osSpTaskYielded(OSTask *t) { (void)t; return 0; }

/* ---- VI (frame present) — wired to the host backend at M2 --------------- */
void osViBlack(u8 b)                                 { (void)b; }
void osViSetMode(OSViMode *m)                        { (void)m; }
void osViSetEvent(OSMesgQueue *mq, OSMesg m, u32 r)  { (void)mq;(void)m;(void)r; }
void osViSetSpecialFeatures(u32 f)                   { (void)f; }
extern void turokVideoSwap(void *frameBuf);          /* driver frame hook (turok_main.c): count+capture+escape */
extern void turokGfxEndFrame(void);                  /* present the rendered frame */
extern void turokGfxStartFrame(void);                /* open the next frame */
void osViSwapBuffer(void *frameBuf)
{
#ifdef PLATFORM_PORT
    /* PORT: decouple the game LOGIC tick rate (TUROK_TICK_FPS, default 30 = Turok's native step — its
     * frame_increment is sized for 30fps) from the render/present rate. This is THE per-frame present;
     * advance the logic only when ~1/TICK_FPS sec has really elapsed (g_turok_logic_tick=1), else the
     * next CEngineApp__UpdateGAME forces frame_increment=0 and the frame just re-presents the same
     * state. Without this the 30fps-sized step was applied at the 60fps render rate -> game ran ~2x too
     * fast. TUROK_TICK_FPS=0 = logic every frame (old behaviour); higher = faster, lower = slower. */
    {
        static int s_tick = -1;
        if (s_tick < 0) { const char *e = getenv("TUROK_TICK_FPS"); s_tick = e ? atoi(e) : 30; }
        if (s_tick > 0) {
            g_tick_interval_ns = 1000000000L / s_tick;
            struct timespec now; clock_gettime(CLOCK_MONOTONIC, &now);
            if (g_tick_last.tv_sec == 0) { g_tick_last = now; g_turok_logic_tick = 1; }
            else {
                long el = (now.tv_sec - g_tick_last.tv_sec) * 1000000000L + (now.tv_nsec - g_tick_last.tv_nsec);
                if (el >= g_tick_interval_ns) {
                    g_turok_logic_tick = 1;
                    /* PHASE-ACCUMULATE: advance the tick time by exactly one interval, NOT to 'now'.
                     * Reseeding to 'now' every fire re-randomises the phase, so when the tick interval
                     * equals the v-sync frame time (TICK_FPS == monitor Hz, e.g. 60 on a 60Hz panel) tiny
                     * jitter makes "el >= interval" pass-or-fail unpredictably -> randomly skipped ticks =
                     * a ~30Hz-feeling beat. Advancing by one interval keeps the cadence locked to real time
                     * and self-corrects, so 60Hz logic ticks cleanly against 60Hz v-sync. */
                    g_tick_last.tv_nsec += g_tick_interval_ns;
                    while (g_tick_last.tv_nsec >= 1000000000L) { g_tick_last.tv_nsec -= 1000000000L; g_tick_last.tv_sec++; }
                    /* If a slow frame left us a whole interval behind, don't bank a backlog (we run at most
                     * one logic tick per present) — snap forward so we don't fast-forward to "catch up". */
                    { long beh = (now.tv_sec - g_tick_last.tv_sec) * 1000000000L + (now.tv_nsec - g_tick_last.tv_nsec);
                      if (beh >= g_tick_interval_ns) g_tick_last = now; }
                } else g_turok_logic_tick = 0;
            }
        } else { g_turok_logic_tick = 1; g_tick_interval_ns = 0; }
    }
#endif
    turokGfxEndFrame();          /* finish + present this frame's Fast3D rendering */
    turokVideoSwap(frameBuf);    /* frame count; capture PNG if requested; longjmp at max */
    turokGfxStartFrame();        /* open the next frame */
}
s32  osDpSetNextBuffer(void *p, u64 sz)              { (void)p;(void)sz; return 0; }

/* ---- AI (audio out) — host DAC replacement (turok_audio.c sink) --------- */
extern void         turokAudioPush(const void *pcm, int nbytes);
extern unsigned int turokAudioQueuedBytes(void);
extern void         turokAudioSetRate(int rate);

s32 osAiSetNextBuffer(void *buf, u32 sz) { turokAudioPush(buf, (int)sz); return 0; }
u32 osAiGetLength(void)                  { return turokAudioQueuedBytes(); }
u32 osAiGetStatus(void)                  { return 0; }
s32 osAiSetFrequency(u32 f)              { if (f < 8000) f = 22050; if (f > 48000) f = 48000;
                                           turokAudioSetRate((int)f); return (s32)f; }

/* ---- controllers — no input at M1 -------------------------------------- */
s32 osContInit(OSMesgQueue *mq, u8 *bitpattern, OSContStatus *st)
{ (void)mq; if (bitpattern) *bitpattern = 1; if (st) memset(st, 0, sizeof(*st)); return 0; }
s32  osContStartReadData(OSMesgQueue *mq)
{
    /* PORT: post the registered SI-event message so the loop reads the controller this frame. */
    if (g_si_msg) osSendMesg(g_si_mq ? g_si_mq : mq, g_si_msg, OS_MESG_NOBLOCK);
    return 0;
}
extern void turokInputGetPad(OSContPad *pad);        /* turok_input.c: current mapped pad */
void osContGetReadData(OSContPad *pad)               { if (pad) turokInputGetPad(pad); }

/* ---- controller pak / pfs (saves) — empty at M1 ------------------------- */
s32 osPfsInitPak(OSMesgQueue *mq, OSPfs *pfs, int ch){ (void)mq;(void)pfs;(void)ch; return 1; }
s32 osPfsInit(OSMesgQueue *mq, OSPfs *pfs, int ch)   { (void)mq;(void)pfs;(void)ch; return 1; }
s32 osPfsNumFiles(OSPfs *pfs, s32 *mx, s32 *used)    { (void)pfs; if(mx)*mx=0; if(used)*used=0; return 0; }
s32 osPfsFreeBlocks(OSPfs *pfs, s32 *bytes)          { (void)pfs; if(bytes)*bytes=0; return 0; }
s32 osPfsAllocateFile(OSPfs *pfs, u16 c, u32 g, u8 *n, u8 *e, int sz, s32 *fn)
{ (void)pfs;(void)c;(void)g;(void)n;(void)e;(void)sz;(void)fn; return 1; }
s32 osPfsDeleteFile(OSPfs *pfs, u16 c, u32 g, u8 *n, u8 *e)
{ (void)pfs;(void)c;(void)g;(void)n;(void)e; return 1; }
s32 osPfsFileState(OSPfs *pfs, s32 fn, OSPfsState *st){ (void)pfs;(void)fn;(void)st; return 1; }
s32 osPfsReadWriteFile(OSPfs *pfs, s32 fn, u8 m, int o, int sz, u8 *d)
{ (void)pfs;(void)fn;(void)m;(void)o;(void)sz;(void)d; return 1; }
s32 osPfsIsPlug(OSMesgQueue *mq, u8 *p)              { (void)mq; if(p)*p=0; return 0; }

/* ---- timing / system ---------------------------------------------------- */
OSTime osGetTime(void)
{
    struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);
    /* N64 counter is ~46.875 MHz; scale host ns to that domain */
    return (OSTime)ts.tv_sec * 46875000ULL + (OSTime)ts.tv_nsec * 46875ULL / 1000000ULL;
}
void osInitialize(void)            { }
OSIntMask osSetIntMask(OSIntMask m){ return m; }
void osSyncPrintf(const char *fmt, ...) { (void)fmt; }
void osCreateViManager(OSPri p) { (void)p; }

/* cache ops are no-ops on a coherent host */
void osInvalDCache(void *p, s32 n)      { (void)p;(void)n; }
void osInvalICache(void *p, s32 n)      { (void)p;(void)n; }
void osWritebackDCache(void *p, s32 n)  { (void)p;(void)n; }
void osWritebackDCacheAll(void)         { }
