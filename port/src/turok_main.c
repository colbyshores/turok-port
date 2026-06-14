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

/* game boot-chain entry wrappers (tengine.c) */
extern void boot(void);
extern void idle(void *);
extern void mainproc(void *);

/* romdata seam */
extern int romdataInit(void);

/* gfx bridge (turok_gfx.c -> Fast3D/OSMesa) */
extern void turokGfxInit(int w, int h);
extern int  turokGfxSavePng(const char *path);

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
    /* NOTE: capture is done in turokGfxEndFrame (keyed to REAL render frames), NOT here —
     * turokVideoSwap is also called by the osRecvMesg frame-pump on non-render ticks. */
    if (g_max_frames && g_frame >= g_max_frames)
        longjmp(g_escape, 1);              /* bounded run complete */
}

int main(int argc, char **argv)
{
    const char *mf = getenv("TUROK_MAX_FRAMES");
    const char *cf = getenv("TUROK_CAPTURE_FRAME");
    const char *cp = getenv("TUROK_CAPTURE_PATH");
    (void)argc; (void)argv;
    if (mf) g_max_frames = strtol(mf, NULL, 10);
    if (cf) g_capture_frame = strtol(cf, NULL, 10);
    if (cp) g_capture_path = cp;

    fprintf(stderr, "[turok] M2 host boot — max_frames=%ld capture_frame=%ld\n",
            g_max_frames, g_capture_frame);

    if (romdataInit() != 0)
        return 1;

    turokGfxInit(320, 240);      /* OSMesa + Fast3D (headless ground truth) */

    if (setjmp(g_escape) == 0) {
        /* boot() = CEngineApp__Boot (OS/app init). We then SKIP idle() — on the N64 the
         * idle thread does a little setup then spins forever as the lowest-priority thread
         * while the scheduler runs mainThread; its setup (osViBlack/osCreatePiManager/fault
         * thread) is all no-op/bypassed here. Call mainproc directly; the cooperative frame
         * pump in osRecvMesg drives CEngineApp__Main's loop. */
        fprintf(stderr, "[turok] -> boot()\n");
        boot();
        fprintf(stderr, "[turok] -> mainproc() (idle bypassed)\n");
        mainproc(NULL);  /* CEngineApp__Main — game loop (escapes via turokVideoSwap longjmp) */
        fprintf(stderr, "[turok] mainproc returned (unexpected)\n");
    }

    fprintf(stderr, "[turok] exited cleanly after %ld frame(s)\n", g_frame);
    return 0;
}
