/*
 * turok_audio.c — host audio OUTPUT seam (the N64 AI / DAC replacement).
 *
 * The N64 hands finished 16-bit-stereo PCM to the audio DAC via osAiSetNextBuffer().
 * On the host that PCM (produced by the software mixer / n_audio HLE) is pushed here:
 *   - SDL2 builds  -> SDL audio device queue (real sound on the user's desktop)
 *   - headless      -> optional WAV dump (TUROK_AUDIO_WAV=path) for offline verification
 *
 * This is purely the sink. What FILLS the PCM (real libaudio synth + mixer) is wired
 * separately; until that lands these calls just see silence and do nothing harmful.
 *
 * Format: 16-bit signed, 2 channels, interleaved L/R, native-endian — exactly what the
 * host C mixer emits and what SDL's AUDIO_S16SYS expects (no swap on the output path).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(GFX_USE_SDL2)
#include <SDL.h>
#endif

#define TKA_DEFAULT_RATE 22050

static int   s_rate    = TKA_DEFAULT_RATE;
static int   s_opened  = 0;      /* device/file open attempted */
static int   s_enabled = 1;      /* TUROK_NOAUDIO=1 disables entirely */

#if defined(GFX_USE_SDL2)
static SDL_AudioDeviceID s_dev = 0;
#endif

/* headless WAV dump */
static FILE *s_wav = NULL;
static long  s_wav_data_bytes = 0;

static void tka_wav_header(FILE *f, int rate)
{
    /* canonical 16-bit stereo PCM WAV; data size patched at close */
    unsigned char h[44];
    unsigned int  byterate = (unsigned)rate * 2 /*ch*/ * 2 /*bytes*/;
    memset(h, 0, sizeof(h));
    memcpy(h + 0,  "RIFF", 4);                 /* [4..8) riff size patched later */
    memcpy(h + 8,  "WAVE", 4);
    memcpy(h + 12, "fmt ", 4);
    h[16] = 16;                                /* fmt chunk size */
    h[20] = 1;                                 /* PCM */
    h[22] = 2;                                 /* channels */
    h[24] = (unsigned char)(rate & 0xff);  h[25] = (unsigned char)((rate >> 8) & 0xff);
    h[26] = (unsigned char)((rate >> 16) & 0xff); h[27] = (unsigned char)((rate >> 24) & 0xff);
    h[28] = (unsigned char)(byterate & 0xff); h[29] = (unsigned char)((byterate >> 8) & 0xff);
    h[30] = (unsigned char)((byterate >> 16) & 0xff); h[31] = (unsigned char)((byterate >> 24) & 0xff);
    h[32] = 4;                                 /* block align (2ch*2B) */
    h[34] = 16;                                /* bits per sample */
    memcpy(h + 36, "data", 4);                 /* [40..44) data size patched later */
    fwrite(h, 1, sizeof(h), f);
}

/* Backend -> here: the synth's chosen output rate (from osAiSetFrequency). */
void turokAudioSetRate(int rate)
{
    if (rate >= 8000 && rate <= 48000)
        s_rate = rate;
}

static void tka_open_once(void)
{
    const char *e;
    if (s_opened) return;
    s_opened = 1;

    e = getenv("TUROK_NOAUDIO");
    if (e && atoi(e)) { s_enabled = 0; return; }

    /* TUROK_AUDIO_WAV takes precedence over the live device, so headless WAV
     * verification of the audio thread works in ANY build (including GFX=sdl2). */
    e = getenv("TUROK_AUDIO_WAV");
    if (e && *e) {
        s_wav = fopen(e, "wb");
        if (s_wav) { tka_wav_header(s_wav, s_rate); s_wav_data_bytes = 0;
            fprintf(stderr, "[audio] WAV dump -> %s (%d Hz)\n", e, s_rate); }
        return;
    }

#if defined(GFX_USE_SDL2)
    {
        SDL_AudioSpec want, got;
        if (SDL_WasInit(SDL_INIT_AUDIO) == 0)
            SDL_InitSubSystem(SDL_INIT_AUDIO);
        memset(&want, 0, sizeof(want));
        want.freq     = s_rate;
        want.format   = AUDIO_S16SYS;
        want.channels = 2;
        want.samples  = 1024;          /* device buffer; small for low latency */
        want.callback = NULL;          /* push model via SDL_QueueAudio */
        s_dev = SDL_OpenAudioDevice(NULL, 0, &want, &got, 0);
        if (s_dev) {
            s_rate = got.freq;
            SDL_PauseAudioDevice(s_dev, 0);   /* start playback */
            fprintf(stderr, "[audio] SDL device open: %d Hz, 16-bit stereo\n", s_rate);
            return;
        }
        fprintf(stderr, "[audio] SDL_OpenAudioDevice failed: %s\n", SDL_GetError());
    }
#endif
}

/* AI seam -> here: queue one finished PCM buffer (16-bit stereo, native-endian). */
void turokAudioPush(const void *pcm, int nbytes)
{
    if (!pcm || nbytes <= 0) return;
    tka_open_once();
    if (!s_enabled) return;

#if defined(GFX_USE_SDL2)
    if (s_dev) { SDL_QueueAudio(s_dev, pcm, (Uint32)nbytes); return; }
#endif
    if (s_wav) { fwrite(pcm, 1, (size_t)nbytes, s_wav); s_wav_data_bytes += nbytes; }
}

/* How many output bytes are still queued (osAiGetLength backpressure). */
unsigned int turokAudioQueuedBytes(void)
{
    if (!s_opened || !s_enabled) return 0;
#if defined(GFX_USE_SDL2)
    if (s_dev) return (unsigned int)SDL_GetQueuedAudioSize(s_dev);
#endif
    return 0;   /* WAV/null sink never backs up */
}

void turokAudioClose(void)
{
#if defined(GFX_USE_SDL2)
    if (s_dev) { SDL_CloseAudioDevice(s_dev); s_dev = 0; }
#endif
    if (s_wav) {
        long riff = 36 + s_wav_data_bytes;
        unsigned char v[4];
        v[0]=(unsigned char)(riff&0xff); v[1]=(unsigned char)((riff>>8)&0xff);
        v[2]=(unsigned char)((riff>>16)&0xff); v[3]=(unsigned char)((riff>>24)&0xff);
        fseek(s_wav, 4, SEEK_SET);  fwrite(v, 1, 4, s_wav);
        v[0]=(unsigned char)(s_wav_data_bytes&0xff); v[1]=(unsigned char)((s_wav_data_bytes>>8)&0xff);
        v[2]=(unsigned char)((s_wav_data_bytes>>16)&0xff); v[3]=(unsigned char)((s_wav_data_bytes>>24)&0xff);
        fseek(s_wav, 40, SEEK_SET); fwrite(v, 1, 4, s_wav);
        fclose(s_wav); s_wav = NULL;
    }
}
