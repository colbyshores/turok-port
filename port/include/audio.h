#ifndef _IN_AUDIO_H
#define _IN_AUDIO_H

#include <PR/ultratypes.h>

/*
 * Audio backend contract — matches perfect_dark / banjo-kazooie `port/include/audio.h`
 * (the desktop SDL / 3DS ndsp sink) so the layer can be lifted into a shared reusable
 * library once enough N64->PC/3DS ports converge. The sink is a stash (audioSetNextBuffer)
 * + push-with-back-pressure (audioEndFrame) model, exactly like the siblings.
 *
 * The dedicated-audio-thread entry points mirror perfect_dark/port/src/audio_3ds.c
 * (audioThreadStart/Stop, audioSynthLock/Unlock) — the N64 audio-manager thread made
 * real. PD spins it only on 3DS (desktop inline); turok also runs it on desktop (pthread),
 * gated by TUROK_AUDIO_THREAD, but the contract is identical so a future library can choose
 * per platform.
 */

/* --- the sink (same 5-function contract as PD/banjo audio.c) --- */
s32  audioInit(void);
s32  audioGetBytesBuffered(void);
s32  audioGetSamplesBuffered(void);
void audioSetNextBuffer(const s16 *buf, u32 len);
void audioEndFrame(void);

/* --- the dedicated audio thread (mirrors PD audio_3ds.c) --- */
void audioThreadStart(void);
void audioThreadStop(void);
void audioSynthLock(void);
void audioSynthUnlock(void);
int  audioThreadActive(void);

/* turok extension: flush/close the sink on exit (finalizes the headless WAV dump). */
void audioClose(void);

#endif
