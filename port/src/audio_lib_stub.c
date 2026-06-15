/* audio_lib_stub.c — libaudio (al*) + ADPCM stubs.
 *
 * Turok's audio (audio.c/audiomgr.c) drives the libultra n_audio synth (alInit, the
 * sequence player alCSP*, the sound player alSndp*). On the host the synth is replaced by
 * a software mixer at M4. Until then these are benign stubs: just enough that the boot-time
 * audio init runs without crashing and the title screen (M2) comes up silent. Allocators
 * hand out from a static arena so callers that dereference the result don't fault.
 *
 * No <libaudio.h> include — the linker resolves these by name; real implementations land
 * with the mixer at M4.
 */
#include <stddef.h>

/* static arena for the al* allocators (sized for the audio heap the game requests) */
static unsigned char al_arena[512 * 1024];
static unsigned long  al_off = 0;
static void *al_alloc(unsigned long sz)
{
    void *p;
    sz = (sz + 15u) & ~15u;
    if (al_off + sz > sizeof(al_arena)) return al_arena;   /* clamp; never NULL */
    p = &al_arena[al_off];
    al_off += sz;
    return p;
}

/* heap / init */
void  alInit(void *gp, void *cfg)              { (void)gp; (void)cfg; }
void  alClose(void *gp)                         { (void)gp; }
void  alLink(void *e, void *l)                  { (void)e; (void)l; }
void  alUnlink(void *e)                         { (void)e; }
void  alHeapInit(void *hp, unsigned char *base, long len) { (void)hp; (void)base; (void)len; }
void *alHeapDBAlloc(void *hp, long count, long sz, char *f, int ln)
{ (void)hp; (void)f; (void)ln; return al_alloc((unsigned long)count * (unsigned long)sz); }

/* bank + sequence */
void  alBnkfNew(void *bank, void *tbl)          { (void)bank; (void)tbl; }
void *alCSeqNew(void *seq, unsigned char *base) { (void)base; return seq; }

/* compressed sequence player (music) */
void  alCSPNew(void *csp, void *cfg)            { (void)csp; (void)cfg; }
void  alCSPPlay(void *csp)                      { (void)csp; }
void  alCSPStop(void *csp)                      { (void)csp; }
void  alCSPSetSeq(void *csp, void *seq)         { (void)csp; (void)seq; }
void  alCSPSetBank(void *csp, void *bank)       { (void)csp; (void)bank; }
void  alCSPSetVol(void *csp, short vol)         { (void)csp; (void)vol; }
void  alCSPSetChlFXMix(void *csp, unsigned char ch, unsigned char m) { (void)csp;(void)ch;(void)m; }

/* sound player (SFX) */
void *alSndpNew(void *sndp, void *cfg)          { (void)cfg; return sndp; }
int   alSndpAllocate(void *sndp, void *snd)     { (void)sndp; (void)snd; return 0; }
void  alSndpDeallocate(void *sndp, int id)      { (void)sndp; (void)id; }
void  alSndpPlay(void *sndp)                    { (void)sndp; }
void  alSndpStop(void *sndp)                    { (void)sndp; }
int   alSndpGetState(void *sndp)                { (void)sndp; return 0; /* AL_STOPPED */ }
void  alSndpSetSound(void *sndp, int id)        { (void)sndp; (void)id; }
void  alSndpSetVol(void *sndp, short v)         { (void)sndp; (void)v; }
void  alSndpSetPitch(void *sndp, float p)       { (void)sndp; (void)p; }
void  alSndpSetPan(void *sndp, unsigned char p) { (void)sndp; (void)p; }
void  alSndpSetFXMix(void *sndp, unsigned char m){ (void)sndp; (void)m; }

/* per-frame synth entry (audio) */
void *alAudioFrame(short *cmdList, int *cmdLen, short *outBuf, int outLen)
{ (void)outBuf; (void)outLen; if (cmdLen) *cmdLen = 0; return cmdList; }
/* NOTE: adpcmDecode() is NOT stubbed here — it is the ANIMATION keyframe ADPCM
 * decompressor (anim.c, per-node quaternion/position streams), real impl in
 * port/src/turok_adpcm.c. A no-op stub here silently zeroed every node rotation,
 * collapsing all animated models. */
