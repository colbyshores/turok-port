//#define IG_DEBUG

#include <ultra64.h>
#include <libaudio.h>
#include <ultralog.h>
#include <ramrom.h>
#include <rmon.h>

#include "tengine.h"
#include "audio.h"
#include "audiocfx.h"
#include "sfx.h"
#include "scaling.h"
#include "snddefs.h"
#include "tmove.h"

#ifdef PLATFORM_PORT
/* PORT: the N64 serialised these audio-event-queue critical sections by raising to PRIORITY_AUDIOLOCK
 * (osSetThreadPri), a NO-OP in the cooperative port. Each PRIORITY_AUDIOLOCK acquire / ospri restore now
 * also takes/releases the recursive synthLock (port/src/audio.c) — the real serialiser vs the dedicated
 * audio thread. Without it the per-frame SFX updates race the synth and splice the libaudio event list into
 * a CYCLE -> the audio thread spins in the list walk holding the lock -> the game thread blocks forever on
 * audioSynthLock (the watchdog "main thread STUCK ... audioSynthLock"). c6e12dc fixed 3 of ~12 sections; this
 * covers the rest. Balanced because every AUDIOLOCK acquire is matched by an ospri restore on every path. */
extern void audioSynthLock(void);
extern void audioSynthUnlock(void);
#else
#define audioSynthLock()   ((void)0)
#define audioSynthUnlock() ((void)0)
#endif

#define AUDIO_DEBUG	   		1
#define MAX_CHANNEL_SAVER		5
#define ms *(((s32) ((f32) OUTPUT_RATE/1000))&~0x7)
#define SECTION_COUNT			4
#define SECTION_SIZE				8
#define CHORUS_K					33554432

extern OSSched         sc;
extern char *test_sound_effects[];

extern char _sfxctlSegmentRomStart[], _sfxctlSegmentRomEnd[];
extern char _sfxtblSegmentRomStart[], _sfxtblSegmentRomEnd[];
extern char _seqtblSegmentRomStart[], _seqtblSegmentRomEnd[];
extern char _seqSegmentRomStart[],		_seqSegmentRomEnd[];
extern char _seq2SegmentRomStart[],		_seq2SegmentRomEnd[];
extern char _seqctlSegmentRomStart[], _seqctlSegmentRomEnd[];
extern void alSeqNewMarker(ALSeq *seq, ALSeqMarker *m, u32 ticks);
extern INT16    CFXVolumeTable[];


t_WaitQue WaitQueue;
t_WaitQue AvailQueue;


int	 audio_debug_flag = 0;

/*
input: specifies the address of the input for this section of the effect
output: specifies the address of the output for this section of the effect
fbcoef: specfies the coefficient of the feedback portion of the section
ffcoef: specfies the coefficient of the feedforward protion of the section
gain: specifies how much of this primitives output to contribute to the
		total effect output.
chorus rate: specifies the modulation freq.
chorus depth: specifies modulation depth
filter coef: specifies a single low-pass filter coefficient. Only positive values
				will actually be low-pass. (so far I have not been able to get
				the  low-pass filter to work right

*/

s32	params[SECTION_COUNT*SECTION_SIZE+2] =
{

	 SECTION_COUNT,         180 ms,
											/*       chorus  chorus   filter
	 input  output  fbcoef  ffcoef   gain     rate   depth     coef  */
	0 ms,	  38 ms,  3276,  -3276,   0x11eb,      0,      0,      0,
	0 ms,   56 ms,  3276,  -3276,   0x15eb,      0,      0,      0,
	0 ms,   70 ms,  3276,  -3276,   0x15eb,      0,      0,      0,
	20 ms,  150 ms,  10000,  0,      0x21eb,      0,     0,		0x5000,

};


//#define SEQUENCE_BUFFER_SIZE 16

/**** audio globals ****/
t_AudioWorld			AW;
ALBankFile				*seqbankPtr;
float						__GlobalSFXscalar = 1.0;
INT16						__GlobalSEQvolume = MAX_SEQ_VOL;
int						start_stop;
int						stop_channel = -1;
ALHeap					hp;

INT8						SeqBuffer[SEQUENCE_BUFFER_SIZE];
static ALCSeq			*seq;
ALCSPlayer				*seqp;
int						global_seqnum = -1;
int						SeqStateFlags = SEQSTATE_IDLE;
int						SeqStateAction = 0;
int						SeqFadeLevel;
int						testhandle = -1;
//unsigned long			audio_ambient_flags = 0;


//int						db_channel= -1;

void romcpy(const char *src, const char *dest, const int len);
void SortSounds();

void InsertQueueEntryBefore(t_WaitQue *Queue, t_WaitQue *Entry);
void InitQueue(t_WaitQue *Queue);
t_WaitQue *RemoveQueueEntry(t_WaitQue *Entry);






//****************///////////////****************///////////////****************/////////////
//****************///////////////****************///////////////****************/////////////
//****************///////////////****************///////////////****************/////////////
//****************///////////////****************///////////////****************/////////////
#ifdef PLATFORM_PORT
/* S3 ready-gate: the audio thread (port/src/audio.c) is started right after boot(), but
 * initAudio (which builds the synth + players + banks) runs later, in the first game frame.
 * The thread pushes silence until this flag flips at the end of initAudio, so it never drives
 * turokAudioManagerFrame against a half-initialised audio manager. */
int turok_audio_ready = 0;
#endif
void initAudio(void)
{
    u32					bankLen;
    ALSynConfig		c;
    ALSeqpConfig		seqc;
    amConfig			amc;
	int					i;
	ALInstrument		*inst;
	//ALSndId			*idPtr;
	//ALSound			*newsndptr;
	//ALSndPlayer		*PlayerPtr;
	OSPri					ospri;

	ospri = osGetThreadPri(NULL);	// set priority higher to avoid
	osSetThreadPri(NULL, PRIORITY_AUDIOLOCK); audioSynthLock();

	InitSoundGlobals(DEFAULT_RADIUS);

   alHeapInit(&hp, (u8*)audioHeap, AUDIO_HEAP_SIZE);

	// Load the SEQ sound ctl file from ROM
#ifdef PLATFORM_PORT
   /* PORT (M4-S2 + retail audio): load + endian-swap+relocate the SEQ (music) bank via turokBnkfNew.
    * dev tree = the placeholder testbank.ctl/.tbl; with a retail ROM (TUROK_ROM / Path B) load the REAL
    * music bank from the ROM: seqctl @ 0x626dd0 (24 instruments, 44100Hz), seqtbl @ 0x628350 (~256KB),
    * US v1.2. Falls back to the dev testbank if the ROM/offset doesn't validate (header != "B1"). */
   {
      extern u8 *turokAudioLoadBank(const char *path, u32 *outLen);
      extern u8 *turokAudioLoadBankFromROM(const char *rompath, long offset, u32 size, int expectBank);
      extern void turokBnkfNew(ALBankFile *file, u8 *table);
      u32 seqtblLen; u8 *seqtbl = 0; extern const char *turokRomPath(void); const char *rom = turokRomPath();
      seqbankPtr = 0;
      if (rom && *rom) {
         seqbankPtr = (ALBankFile *)turokAudioLoadBankFromROM(rom, 0x626dd0, 0x1580, 1);
         if (seqbankPtr) {
            seqtbl = turokAudioLoadBankFromROM(rom, 0x628350, 0x3ef00, 0);
            if (!seqtbl) seqbankPtr = 0;
         }
      }
      if (!seqbankPtr) {
         seqbankPtr = (ALBankFile *)turokAudioLoadBank("src/PR/testbank.ctl", &bankLen);
         seqtbl     = turokAudioLoadBank("src/PR/testbank.tbl", &seqtblLen);
      }
      if (seqbankPtr && seqtbl) turokBnkfNew(seqbankPtr, seqtbl);
   }
#else
   bankLen = _seqctlSegmentRomEnd - _seqctlSegmentRomStart;
   seqbankPtr = alHeapAlloc(&hp, 1, bankLen);
   romcpy(_seqctlSegmentRomStart, (char *)seqbankPtr, bankLen);
   alBnkfNew(seqbankPtr, (u8 *) _seqtblSegmentRomStart);
#endif

	/*
     * Create the Audio Manager
     */
    c.maxVVoices	= MAX_VOICES;
    c.maxPVoices	= MAX_VOICES*2;
    c.maxUpdates	= MAX_UPDATES;
    c.dmaproc		= 0;                  // audio mgr will fill this in
#ifdef SOUND_USE_REVERB
	 c.fxType		= AL_FX_CUSTOM;
#else
	 c.fxType		= AL_FX_NONE;
#endif

    c.heap			= &hp;

	 if (c.fxType == AL_FX_CUSTOM)
		c.params = params;

    amc.outputRate = OUTPUT_RATE;
    amc.framesPerField = NUM_FIELDS;
    amc.maxACMDSize = MAX_RSP_CMDS;

	// Intialize AudioWorld globals
	AW.SndPlayerList.NextPlayer = NULL;
	for(i = 0; i<MAX_VOICES_ALL;i++)
	{
		AW.VoiceChannels[i].VirtualHandle = -1;
		AW.VoiceChannels[i].PhysicalHandle = -1;
		AW.VoiceChannels[i].theSFX.SfxNum = -1;
		AW.VoiceChannels[i].theSFX.Priority = 0;
		AW.VoiceChannels[i].theSFX.pEnvSound = NULL;
		AW.VoiceChannels[i].theSFX.QuedVoice = -1;
		AW.VoiceChannels[i].SndPlayer = &AW.SndPlayerList.Sndp;
		AW.VoiceChannels[i].ChannelFlags = 0;
	}

	for(i = 0; i<MAX_VOICES;i++)
		AW.SndPlayerList.idlist[i] = -1;

	InitQueue(&WaitQueue);
	InitQueue(&AvailQueue);

	for(i = 0; i < MAX_QUE; i++)
	{
		InsertQueueEntryBefore(AvailQueue.Next,&AW.SFXWaitQue[i]);
	}

	/* PORT (M4-S2): the early-return that deferred the audio manager + players is REMOVED — the real
	 * libaudio (turoksnd/abi) is linked and the SEQ + SFX banks are loaded from disk (above / below)
	 * and endian-swapped via turokBnkfNew, so initAudio now runs fully: amCreateAudioMgr + alCSPNew +
	 * the SFX player + SortSounds all build real state. The synth is driven on the dedicated audio
	 * thread (port/src/audio.c). */

	amCreateAudioMgr(&c, PRIORITY_AUDIO, &amc);

#if 1
    /*
     * Create the sequence and the sequence player
     */
    seqc.maxVoices      = 24;
    seqc.maxEvents      = MAX_EVENTS;
    seqc.maxChannels    = 16;
    seqc.heap           = &hp;
    seqc.initOsc        = 0;
    seqc.updateOsc      = 0;
    seqc.stopOsc        = 0;

	 seqp = alHeapAlloc(&hp, 1, sizeof(ALCSPlayer));

	 ASSERT(seqp);
    alCSPNew(seqp, &seqc);
    seq = alHeapAlloc(&hp, 1, sizeof(ALCSeq));
	 ASSERT(seq);

#endif


 	//load SFX sound info
#ifdef PLATFORM_PORT
	/* PORT (M4-S2 + retail audio): load + endian-swap+relocate the SFX bank via turokBnkfNew. The dev
	 * tree's sfx.ctl/.tbl are PLACEHOLDER (sports-announcer scratch samples — none of those phrases are
	 * in the shipped game). When a retail ROM is supplied (TUROK_ROM / Path B) load the REAL Turok SFX
	 * bank straight from the ROM: sfxctl @ 0x667230 (228 sounds, 44100Hz), sfxtbl @ 0x672500 (~1.1MB),
	 * US v1.2. The cart sound-elements come from the same ROM (Path B), so their m_nSampleNum indices
	 * match this bank. Falls back to the dev bank if the ROM/offset doesn't validate (header != "B1"). */
	{
		extern u8 *turokAudioLoadBank(const char *path, u32 *outLen);
		extern u8 *turokAudioLoadBankFromROM(const char *rompath, long offset, u32 size, int expectBank);
		extern void turokBnkfNew(ALBankFile *file, u8 *table);
		u32 sfxtblLen; u8 *sfxtbl = 0; extern const char *turokRomPath(void); const char *rom = turokRomPath();
		AW.SndPlayerList.sfxBankPtr = 0;
		if (rom && *rom) {
			AW.SndPlayerList.sfxBankPtr = turokAudioLoadBankFromROM(rom, 0x667230, 0xb2d0, 1);
			if (AW.SndPlayerList.sfxBankPtr) {
				sfxtbl = turokAudioLoadBankFromROM(rom, 0x672500, 0x111200, 0);
				if (!sfxtbl) AW.SndPlayerList.sfxBankPtr = 0;   /* tbl failed -> fall back fully */
			}
		}
		if (!AW.SndPlayerList.sfxBankPtr) {   /* Path A / no ROM: the dev placeholder bank */
			AW.SndPlayerList.sfxBankPtr = turokAudioLoadBank("src/PR/tengine/sfx.ctl", &bankLen);
			sfxtbl = turokAudioLoadBank("src/PR/tengine/sfx.tbl", &sfxtblLen);
		}
		if (AW.SndPlayerList.sfxBankPtr && sfxtbl)
			turokBnkfNew((ALBankFile *) AW.SndPlayerList.sfxBankPtr, sfxtbl);
	}
#else
	bankLen = _sfxctlSegmentRomEnd - _sfxctlSegmentRomStart;
	AW.SndPlayerList.sfxBankPtr = alHeapAlloc(&hp, 1, bankLen);
	ASSERT(AW.SndPlayerList.sfxBankPtr);
	romcpy(_sfxctlSegmentRomStart,  AW.SndPlayerList.sfxBankPtr, bankLen);
	alBnkfNew((ALBankFile *) AW.SndPlayerList.sfxBankPtr, _sfxtblSegmentRomStart);
#endif
#ifdef PLATFORM_PORT
	/* PORT: the SFX bank can fail to load (3DS: no TUROK_ROM env + no dev sfx.ctl on the SD) -> sfxBankPtr
	 * is NULL. The bankArray/instArray walk below would deref NULL — harmless on N64 (a low-RDRAM read) but a
	 * data-abort on a protected host / real 3DS HW (Mandarine tolerates it: unmapped Read @ +0xC/+0xE, PC in
	 * initAudio). Skip it when the bank is absent; audio just stays silent (already gated off on 3DS). */
	if (AW.SndPlayerList.sfxBankPtr)
#endif
	{
		AW.SndPlayerList.sfxBank = ((ALBankFile *) AW.SndPlayerList.sfxBankPtr)->bankArray[0];
		ASSERT(AW.SndPlayerList.sfxBank);
		//sort sfx
		inst =  AW.SndPlayerList.sfxBank->instArray[0];
		SortSounds(inst);
	}

#ifndef DISABLE_SFX
	 // Initialize the soundplayer
   AW.SPConfig.maxSounds = MAX_VOICES;
   AW.SPConfig.maxEvents = MAX_EVENTS;
   AW.SPConfig.heap  = &hp;
	alSndpNew(&AW.SndPlayerList.Sndp, &AW.SPConfig);

#endif

	AW.sfx_counter = 0;
	AW.ChannelLock = -1;
	AW.sfx_timer = 0;

	InitCFXVars();

#ifdef PLATFORM_PORT
	turok_audio_ready = 1;   /* synth + players + banks built — audio thread may now drive the synth */
#endif

#if 0		// test code
	 		// allocate sound in player
	inst =  AW.SndPlayerList.sfxBank->instArray[0];
	idPtr = AW.SndPlayerList.idlist;
	PlayerPtr = &AW.SndPlayerList.Sndp;

	newsndptr = inst->soundArray[57];
	idPtr[0] = alSndpAllocate(PlayerPtr, newsndptr);
	SetChannel(0);
	StartChannel(0);
#endif

	audioSynthUnlock(); osSetThreadPri(NULL, ospri);
}




/*****************************************************************************
*	Function Title: void SortSounds(ALInstrument	*inst)
******************************************************************************
*	Description:	This is a lovely swap sort that sorts
*						the addresses of sounds within instruments
*	Inputs:
*	Outputs:
*
*****************************************************************************/
void SortSounds(ALInstrument	*inst)
{
	int				i, j;
	ALSound			*temp;



	for(i = 0; i < inst->soundCount; i++)
	{
		for(j = i+1; j < inst->soundCount; j++)
		{
			if(inst->soundArray[j] < inst->soundArray[i])
			{
				temp = inst->soundArray[i];
				inst->soundArray[i] = inst->soundArray[j];
				inst->soundArray[j] = temp;
			}
		}
	}
}

/*****************************************************************************
*	Function Title: INT32 PlayEnvironmentSound(void *pInstance, INT16	sfxnum, float x, float y, float z)
******************************************************************************
*	Description:
*	Inputs:
*	Outputs:
*
*****************************************************************************/
INT32 PlayEnvironmentSound(CROMSoundElement *pElement,
									void *pInstance, CVector3 *vPos,
									float Volume, float Pitch)
{

	t_SndObject		*sndptr;
	ALSndId			*idPtr;
	ALSound			*newsndptr;
	ALSndPlayer		*PlayerPtr;
	t_SoundPlayer	*PlayerInfo;
	ALInstrument	*inst;
	OSPri				ospri;
	INT16				Channel;
	t_Channel		*pChannel;
	CGameObjectInstance		*pTurok,
									*pInst;
	CEngineApp					*pApp;
	int				sfxnum = pElement->m_nSampleNum;
	static int		ObstructedFadeVol = DEFAULT_VOLUME;


#ifdef DISABLE_SFX

	return -1;

#endif

#ifdef PLATFORM_PORT
	/* PORT/HW: bail if the SFX bank never loaded (no ROM on the SD) — the sfxBank->instArray[0] deref
	 * further down is a low-NULL read the N64 tolerates but real 3DS HW data-aborts on. See initCFX. */
	if (!AW.SndPlayerList.sfxBank)
		return -1;
#endif

	ospri = osGetThreadPri(NULL);	// synchronize with audio thread
	osSetThreadPri(NULL, PRIORITY_AUDIOLOCK); audioSynthLock();

	Channel = FindEnvSound(pInstance);
	if(Channel == -1)
	{
		//Channel = GetNextVoice(-1, sfxnum, ENVIRONMENTAL_PRIORITY, vPos, 0);
		Channel = GetNextVoice(-1, sfxnum, pElement->m_Priority, vPos, 0);

		if ( (Channel == -1) || (Channel == DUMMY_CHANNEL) )
		{
			audioSynthUnlock(); osSetThreadPri(NULL, ospri);
			return -1;
		}


		// normally you would check to see if the channels were all full.
		// If there were you would put the sfx in the wait que. Since Env. sounds
		// are called continuously, there is no need to.

		// allocate sound in player
		inst =  AW.SndPlayerList.sfxBank->instArray[0];
		idPtr = AW.SndPlayerList.idlist;
		PlayerPtr = &AW.SndPlayerList.Sndp;
		PlayerInfo = &AW.SndPlayerList;
		newsndptr = inst->soundArray[sfxnum];
		idPtr[Channel] = alSndpAllocate(PlayerPtr, newsndptr);
		pChannel = &AW.VoiceChannels[Channel];

		if (idPtr[Channel] == -1)
		{
			// we were unable to allocate sound
			//onPrintf("env sound alloc failed\n");
			pChannel->VirtualHandle = -1;
			pChannel->PhysicalHandle = -1;
			pChannel->theSFX.Priority = 0;
			AW.ChannelLock = -1;
			audioSynthUnlock(); osSetThreadPri(NULL, ospri);
			return -1;
		}

		PlayerInfo->SoundCount++;
		pChannel->SndPlayer = &AW.SndPlayerList.Sndp;
		pChannel->VirtualHandle = AW.sfx_counter++;
		pChannel->PhysicalHandle = idPtr[Channel];
	   pChannel->ChannelFlags = 0;

		sndptr = &pChannel->theSFX;
		//sndptr->Priority = ENVIRONMENTAL_PRIORITY;


		sndptr->Vol = DEFAULT_VOLUME;
		sndptr->Pitch = Pitch;
		sndptr->FXmix = 0;
		sndptr->Pan = DEFAULT_PAN;
		sndptr->pEnvSound = pInstance;
		sndptr->SfxNum = sfxnum;
		sndptr->StartTime = AW.sfx_timer;
		sndptr->RadioVolume = pElement->m_RadioVolume;
		pChannel->ChannelFlags = pElement->m_wFlags;

		SetReverbMix(Channel, 0);
		SetCFXPitch(pElement, Channel, 0);
		SetCFXVolume(pElement, Channel);
		SetSoundPosition(Channel, vPos->x, vPos->y, vPos->z, INITIAL_POS);
		playElement(pChannel->VirtualHandle, vPos, 0);
	}
	else	// update sound
	{
		pChannel = &AW.VoiceChannels[Channel];
		sndptr = &AW.VoiceChannels[Channel].theSFX;

		SetSoundPosition(Channel, vPos->x, vPos->y, vPos->z, INITIAL_POS);

		// check if sound is obstructed
		//(pElement->m_dwFlags & FADE_IF_OBSTRUCTED)
		if(pChannel->ChannelFlags & FADE_IF_OBSTRUCTED)
		{
			pApp = GetApp();
			if (pApp)
				pTurok = CEngineApp__GetPlayer(pApp);

			if (pTurok)
			{
				pInst = (CGameObjectInstance *)pInstance;
				if (CAnimInstanceHdr__CastYeThyRay(&pTurok->ah, &pInst->ah.ih))
				{
					// turok is in line of sound
					if(ObstructedFadeVol < sndptr->Vol)
					{
						// if we suddenly come up to sound, fade it in
						ObstructedFadeVol += 2000;
						sndptr->Vol = ObstructedFadeVol;
					}
					else
						ObstructedFadeVol = sndptr->Vol;
				}
				else
				{
					// turok cannot here this sound directly
					ObstructedFadeVol -= 1000;
					if(ObstructedFadeVol < 10)
						ObstructedFadeVol = 10;

					sndptr->Vol = ObstructedFadeVol;

				}
			}
		}


		// if turok is underwater (not in an anti grav chamber) reduce volume
		if(		(CTurokMovement.WaterFlag == PLAYER_BELOW_WATERSURFACE)
				&&	(!CTurokMovement.InAntiGrav) )
		{
			sndptr->Vol = 10;
		}

		modifySFX(AW.VoiceChannels[Channel].VirtualHandle);
		sndptr->StartTime = AW.sfx_timer;
	}

	audioSynthUnlock(); osSetThreadPri(NULL, ospri);
	return AW.VoiceChannels[Channel].VirtualHandle;
}

/*****************************************************************************
 *		Function Title:	INT16 playElement(INT32 VirtualHandle, int flags)
 *****************************************************************************
 *		Description:	this will play the specified sound effect at x,y,z
 *		Inputs:			voice handle
 *		Outputs:		voice number
 *
 ****************************************************************************/
INT16 playElement(INT32 VirtualHandle, CVector3 *vPos, int flags)
{
//	t_SoundPlayer	*sndplayer;
	INT16				Channel;
	ALSndPlayer		*Sndp;
	t_SndObject		*sndptr;
	OSPri				ospri;



	if(VirtualHandle == -1)
	{
		return -1;
	}

	Channel = SoundHandleToChannel(VirtualHandle, 0);
	if(Channel == -1)
	{
		return -1;
	}

	if(Channel == DUMMY_CHANNEL)
	{
		EnqueSFX(DUMMY_CHANNEL, VirtualHandle, vPos);
		return -1;
	}

	ospri = osGetThreadPri(NULL);
	osSetThreadPri(NULL, PRIORITY_AUDIOLOCK); audioSynthLock();

//	sndplayer = &AW.SndPlayerList;
	Sndp = AW.VoiceChannels[Channel].SndPlayer;
	sndptr = &AW.VoiceChannels[Channel].theSFX;


	// Special case check for sounds that always need to be started!!!
	switch(AW.VoiceChannels[Channel].theSFX.CfxNum)
	{
		// minigun & shockwave looping sfx's
		case SOUND_MINI_GUN_WHIR:
		case SOUND_GENERIC_164:

		// trex sounds
		case SOUND_TREX_SERVO_1:		// laser
		case SOUND_TREX_FIRE_BREATHE:

		// humvee sounds
		case SOUND_GENERIC_82:
		case SOUND_GENERIC_83:
		case SOUND_GENERIC_84:
		case SOUND_GENERIC_85:
		case SOUND_GENERIC_86:
		case SOUND_GENERIC_87:
			break ;

		default:
			if ((sndptr->Vol == 0) && !( AW.VoiceChannels[Channel].ChannelFlags & VOL_ENV_ENABLED) )
			{
				// will get deallocated in UpdateWorldSound()
				AW.VoiceChannels[Channel].ChannelFlags  |= CH_FLAG_STARTED ;
				AW.ChannelLock = -1;

				audioSynthUnlock(); osSetThreadPri(NULL, ospri);
				return 0;
			}
	}

	if(SetChannel(Channel)!= AL_STOPPED)
	{
		AW.ChannelLock = -1;
		audioSynthUnlock(); osSetThreadPri(NULL, ospri);
		return 0;
	}

	alSndpSetPitch(Sndp, sndptr->Pitch);
#ifdef SOUND_USE_REVERB
	alSndpSetFXMix(Sndp, sndptr->FXmix);
#else
	alSndpSetFXMix(Sndp, 0);
#endif
   alSndpSetPan(Sndp, sndptr->Pan);

   alSndpSetVol(Sndp, sndptr->Vol*__GlobalSFXscalar);

	if(AW.VoiceChannels[Channel].delay == 0)
	{
		StartChannel(Channel);
	}

	AW.ChannelLock = -1;
	audioSynthUnlock(); osSetThreadPri(NULL, ospri);
	return 0;
}

/*****************************************************************************
*	Function Title: void StopChannel(INT16 Channel)
******************************************************************************
*	Description:
*	Inputs:
*	Outputs:
*
*****************************************************************************/
void StopChannel(INT16 Channel)
{
	t_SoundPlayer	*sndplayer;
	ALSndPlayer		*Sndp;
	//t_SndObject		*pSound = &AW.VoiceChannels[Channel].theSFX;

	sndplayer = &AW.SndPlayerList;
	Sndp = &sndplayer->Sndp;

	if (SetChannel(Channel) != AL_STOPPED)
	{
		alSndpStop(Sndp);
	}

	//SetChannel(Channel);
	//alSndpStop(Sndp);
}


/*****************************************************************************
*	Function Title: void StartChannel(INT16 Channel)
******************************************************************************
*	Description:
*	Inputs:
*	Outputs:
*
*****************************************************************************/
void StartChannel(INT16 Channel)
{

	t_Channel		*pChannel = &AW.VoiceChannels[Channel];
	ALSndPlayer		*Sndp = &AW.SndPlayerList.Sndp;

		// start new channel
	alSndpPlay(Sndp);
	pChannel->ChannelFlags  |= CH_FLAG_STARTED ;

}

/*****************************************************************************
*	Function Title: int SetChannel(INT16 Channel)
******************************************************************************
*	Description:
*	Inputs:
*	Outputs:
*
*****************************************************************************/
int SetChannel(INT16 Channel)
{
	t_SoundPlayer	*sndplayer;
	ALSndPlayer		*Sndp;



	sndplayer = &AW.SndPlayerList;
	Sndp = &sndplayer->Sndp;
	alSndpSetSound(Sndp, sndplayer->idlist[Channel]);

	return alSndpGetState(Sndp);
}

/*****************************************************************************
*	Function Title: int DeallocateChannel(INT16 Channel)
******************************************************************************
*	Description:
*	Inputs:
*	Outputs:
*
*****************************************************************************/
int DeallocateChannel(INT16 Channel)
{
	t_SoundPlayer	*sndplayer;
	ALSndPlayer		*Sndp;
	t_Channel		*pChannel = &AW.VoiceChannels[Channel];


	sndplayer = &AW.SndPlayerList;
	Sndp = &sndplayer->Sndp;

	alSndpDeallocate(Sndp, sndplayer->idlist[Channel]);
	sndplayer->idlist[Channel] = -1;
	pChannel->theSFX.Priority = 0;
	pChannel->VirtualHandle = -1;
	pChannel->PhysicalHandle = -1;
	pChannel->theSFX.SfxNum = -1;
	pChannel->theSFX.pEnvSound = NULL;

	//rmonPrintf("dealloc ch(%d)\n", Channel);
	return 0;

}


/*****************************************************************************
*	Function Title: INT16 GetNextVoice(INT16	sfxnum, UINT8 Priority)
******************************************************************************
*	Description:	This will return the voice number of the next available voice.
*	Inputs:			none
*	Outputs:		voice number
*
*****************************************************************************/
INT16 GetNextVoice(INT16	cfxnum, INT16	sfxnum, INT16 Priority, CVector3 *vPos, WORD Flags)
{
	//INT16			PriorityCount, Priority;
	int				i;
//	int				TimeDif;
	t_SoundPlayer	*sndplayer;
	int				lowestPriority, nextChannel, oldestAge, age;
	t_Channel		*pChannel;
	//t_SndObject		*pSound;
//	float				NewSndDist;
//	float				dx, dy, dz;

	//ASSERT(Priority >= 1);
	//TRACE1("(g) WARNING: Priority = %d\n", Priority);

	sndplayer = &AW.SndPlayerList;
	lowestPriority = Priority;
	nextChannel = -1;
	oldestAge = 0;

	pChannel = &AW.VoiceChannels[0];
	// this will find th the oldest, lowest priority sound
	for (i=0; i < MAX_VOICES; i++)
	{
		if (sndplayer->idlist[i]==-1)
			return(i);

		if (Priority > pChannel->theSFX.Priority)
		{
			if ( (pChannel->theSFX.Priority <= lowestPriority)
							&& (pChannel->theSFX.Priority != LOOP_PRIORITY))
			{
				age = AW.sfx_timer - pChannel->theSFX.StartTime;

				//if (age >= oldestAge)
				{
					oldestAge = age;
					lowestPriority = pChannel->theSFX.Priority;
					nextChannel = i;
				}
			}
		}

		pChannel++;
	}

	// can not find a sound of lower priority to kill,
	// drop incoming sound
	if (nextChannel == -1)
	{
		return -1;
	}

	if (sndplayer->idlist[nextChannel] == -1)
	{
		// If we found an effect to replace, we need to check the priority against that at the top of
		// the waiting queue because this will allow us to stop environment effects stealing channels
		// from the queue
		if ( (WaitQueue.Next != &WaitQueue) && (WaitQueue.Next->QuedChannel.theSFX.Priority > Priority) )
		{
			osSyncPrintf("Queue priority issue sfx #%d, priority %d\n",sfxnum,Priority);
			return(-1);
		}
		return nextChannel;
	}
	else
	{
		// something has been allocated, stop it and use dummy channel to gather info on dummy channel
		StopChannel(nextChannel);

		if(AW.VoiceChannels[DUMMY_CHANNEL].VirtualHandle != -1)
		{
			return -1;		// something as already waiting in transfer channel to be put in the que
		}
		else
			return DUMMY_CHANNEL;	// transfer channel free
	}
}

/*****************************************************************************
*	Function Title: void EnqueSFX(t_SndObject *sndptr)
******************************************************************************
*	Description:
*	Inputs:
*	Outputs:
*
*****************************************************************************/
void EnqueSFX(int Channel, INT32 VirtualHandle, CVector3 *vPos)
{

	t_Channel		*pChannel = &AW.VoiceChannels[Channel];
	t_WaitQue		*pQue,*pPos;
	s32				Priority;
	OSPri				ospri;


	//rmonPrintf("EnqueSFX(%d, %d)\n", Channel, VirtualHandle);
	// check if there is any room in the que

	ospri = osGetThreadPri(NULL);
	osSetThreadPri(NULL, PRIORITY_AUDIOLOCK); audioSynthLock();

	//
	// Move an entry from the available queue to the wait queue
	//
	// check if Que is full
	if (AvailQueue.Next!=&AvailQueue)
	{
		// copy channel to que
		pQue = RemoveQueueEntry(AvailQueue.Next);
		memcpy(&pQue->QuedChannel, pChannel, sizeof(t_Channel));
		pQue->QuedChannel.theSFX.QuedVoice = VirtualHandle;
		pQue->QuedChannel.theSFX.vPos.x = vPos->x;
		pQue->QuedChannel.theSFX.vPos.y = vPos->y;
		pQue->QuedChannel.theSFX.vPos.z = vPos->z;
		//InsertQueueEntryBefore(&WaitQueue,pQue);
		pPos = WaitQueue.Next;
		// Find correct position within wait queue
		Priority = pQue->QuedChannel.theSFX.Priority;

		while (pPos != &WaitQueue)
		{
			if (pPos->QuedChannel.theSFX.Priority<Priority)
				break;
			pPos = pPos->Next;
		}
		InsertQueueEntryBefore(pPos,pQue);
	}

	// clear dummy channel
	AW.VoiceChannels[DUMMY_CHANNEL].theSFX.Priority = 0;
	AW.VoiceChannels[DUMMY_CHANNEL].theSFX.SfxNum = -1;
	AW.VoiceChannels[DUMMY_CHANNEL].VirtualHandle = -1;
	AW.VoiceChannels[DUMMY_CHANNEL].PhysicalHandle = -1;

	audioSynthUnlock(); osSetThreadPri(NULL, ospri);

}


/*****************************************************************************
*		Function Title:	void ProcessSFXQue()
*****************************************************************************
*		Description:
*		Inputs:
*		Outputs:
*
****************************************************************************/
void ProcessSFXQue()
{
	INT16				NextVoice;
	ALSndId			*idPtr;
	ALSound			*newsndptr;
	ALSndPlayer		*PlayerPtr;
	ALInstrument	*inst;
	t_Channel		*pChannel;
	t_WaitQue		*pQue,*pNext;


	inst =  AW.SndPlayerList.sfxBank->instArray[0];
	idPtr = AW.SndPlayerList.idlist;
	PlayerPtr = &AW.SndPlayerList.Sndp;

	pQue=WaitQueue.Next;
	while(pQue != &WaitQueue )
	{
		pNext = pQue->Next;
		//osSyncPrintf("(%d, %d)\n", AW.QueStartPos, AW.QueCurrentPos );
		if((AW.sfx_timer - pQue->QuedChannel.theSFX.StartTime) > MAX_QUE_TIME)
		{
			RemoveQueueEntry(pQue);
			InsertQueueEntryBefore(&AvailQueue,pQue);
		}
		else
		{
			// find next available channel
			NextVoice = GetNextVoice(pQue->QuedChannel.theSFX.CfxNum, pQue->QuedChannel.theSFX.SfxNum, pQue->QuedChannel.theSFX.Priority, &pQue->QuedChannel.theSFX.vPos, 0);
			if ( (NextVoice == -1) || (NextVoice == DUMMY_CHANNEL) )
			{
				// currently if GetNextVoice returns -1, we stop looking for voices
				// because there will most likely be no other free voices this
				// audio frame
				return;
			}

			if (idPtr[NextVoice] == -1)
			{

				// there is no sound allocated here, try to allocate new one
				newsndptr = inst->soundArray[pQue->QuedChannel.theSFX.SfxNum];
				idPtr[NextVoice] = alSndpAllocate(PlayerPtr, newsndptr);
				if (idPtr[NextVoice] != -1)
				{
					// everything is cool, sound has been allocated, copy data from que to channel
					pChannel = &AW.VoiceChannels[NextVoice];
					memcpy(pChannel, &pQue->QuedChannel, sizeof(t_Channel));
					pChannel->VirtualHandle = pQue->QuedChannel.theSFX.QuedVoice;
					pChannel->PhysicalHandle = idPtr[NextVoice];
					pChannel->theSFX.StartTime = AW.sfx_timer;

					//rmonPrintf("Qplay (%d, %d)\n", NextVoice, pChannel->VirtualHandle);
					playElement(pChannel->VirtualHandle, &pChannel->theSFX.vPos, 0);

					// remove sfx from que
					pQue->QuedChannel.theSFX.QuedVoice = -1;
					pQue->QuedChannel.theSFX.Priority = -1;

					// Remove SFX from wait queue because we have started playing it
					RemoveQueueEntry(pQue);
					InsertQueueEntryBefore(&AvailQueue,pQue);
				}
				else
				{
					return;
				}
			}
			else
			{
				// try stopping sound again, this may not be needed
				//StopChannel(NextVoice);
				//rmonPrintf("(p) channel(%d) not free\n", NextVoice);
				return;
			}
		}
		pQue = pNext;
	}
}

/*****************************************************************************
 *		Function Title:	INT16 modifySFX(INT32 VirtualHandle)
 *****************************************************************************
 *		Description:	this will play the specified sound effect at x,y,z
 *		Inputs:			voice handle
 *		Outputs:		voice number
 *
 ****************************************************************************/
INT16 modifySFX(INT32 VirtualHandle)
{
	t_SoundPlayer	*sndplayer;
	int				Channel;
	ALSndPlayer		*Sndp;
	t_SndObject			*sndptr;
	OSPri				ospri;

	if(VirtualHandle == -1)
	{
		return -1;
	}

	Channel = SoundHandleToChannel(VirtualHandle, 0);

	if((Channel == -1) || (Channel == DUMMY_CHANNEL))
		return -1;


	ospri = osGetThreadPri(NULL);
	osSetThreadPri(NULL, PRIORITY_AUDIOLOCK); audioSynthLock();

	sndplayer = &AW.SndPlayerList;
	// this should work, will have to check this later
	//Sndp = AW.VoiceChannels[Channel].SndPlayer;
	Sndp = &AW.SndPlayerList.Sndp;
	sndptr = &AW.VoiceChannels[Channel].theSFX;

	ASSERT(Sndp);
	ASSERT(sndplayer->idlist[Channel] != -1);								// FAILS HERE!!!!
	ASSERT(sndplayer->idlist[Channel]  < MAX_VOICES);

	if (SetChannel(Channel) == AL_STOPPED)
	{
		audioSynthUnlock(); osSetThreadPri(NULL, ospri);
		return 0;
	}

	alSndpSetPitch(Sndp, sndptr->Pitch);
#ifdef SOUND_USE_REVERB
   alSndpSetFXMix(Sndp, sndptr->FXmix);
#else
	alSndpSetFXMix(Sndp, 0);
#endif
   alSndpSetPan(Sndp, sndptr->Pan);
   alSndpSetVol(Sndp, sndptr->Vol*__GlobalSFXscalar);
	audioSynthUnlock(); osSetThreadPri(NULL, ospri);
	return 0;
}



void killSoundType(int nType)
{
	t_SoundPlayer	*sndplayer;
	t_Channel		*pChannel;
	OSPri				ospri;
	int				i;


	ospri = osGetThreadPri(NULL);
	osSetThreadPri(NULL, PRIORITY_AUDIOLOCK); audioSynthLock();

	sndplayer = &AW.SndPlayerList;
	for (i=0; i < MAX_VOICES_ALL; i++)
	{
		if(AW.VoiceChannels[i].theSFX.CfxNum == nType)
		{
			pChannel = &AW.VoiceChannels[i];
			if(sndplayer->idlist[i] != -1)
			{
				if (SetChannel(i) != AL_STOPPED)
					StopChannel(i);
			}
		}
	}

	audioSynthUnlock(); osSetThreadPri(NULL, ospri);
}



/*****************************************************************************
*		Function Title:	void killSFX(INT32 VirtualHandle)
*****************************************************************************
*		Description:	this will kill the specified sound effect
*		Inputs:			voice handle
*		Outputs:		voice number
*
****************************************************************************/
void killCFX(INT32 VirtualHandle)
{
	t_SoundPlayer	*sndplayer;
	t_Channel		*pChannel;
	OSPri				ospri;
	int				i;
	t_WaitQue		*queue,*next;


	if (VirtualHandle == -1)
		return;

	ospri = osGetThreadPri(NULL);
	osSetThreadPri(NULL, PRIORITY_AUDIOLOCK); audioSynthLock();

	//
	// We need to check through the waiting queue for this SFX and remove it
	//
	queue=WaitQueue.Next;
	while (queue!=&WaitQueue)
	{
		next=queue->Next;
		if (queue->QuedChannel.CFXhandle==VirtualHandle)
		{
			RemoveQueueEntry(queue);
			InsertQueueEntryBefore(&AvailQueue,queue);
		}
		queue=next;
	}


	sndplayer = &AW.SndPlayerList;
	for (i=0; i < MAX_VOICES_ALL; i++)
	{
		if(AW.VoiceChannels[i].CFXhandle == VirtualHandle)
		{
			pChannel = &AW.VoiceChannels[i];
			if(i == DUMMY_CHANNEL)
			{
				pChannel->CFXhandle = -1;
				pChannel->theSFX.Priority = 0;
				pChannel->theSFX.SfxNum = -1;
				pChannel->VirtualHandle = -1;
				pChannel->PhysicalHandle = -1;
			}
			else
			{
					// Sound is being delayed, clear timer
				if(pChannel->delay > 0)
				{
					//rmonPrintf("ch(%d): delayed(%d), stopping...\n", i, pChannel->delay);
					pChannel->delay = 0;
					pChannel->ChannelFlags &= ~CH_FLAG_DELAYED_START;
//					if(sndplayer->idlist[i] != -1)
//					{
//						if (SetChannel(i) != AL_STOPPED)
//						{
//							StopChannel(i);
//						}
//					}
				}
				else
				{
					if(sndplayer->idlist[i] != -1)
					{
						if (SetChannel(i) != AL_STOPPED)
						{
							StopChannel(i);
						}
					}
				}
			}
		}
//		else
//		{
//			
//		}
	}

	audioSynthUnlock(); osSetThreadPri(NULL, ospri);
}


/*****************************************************************************
*		Function Title:	void killAllSFX(void)
*****************************************************************************
*		Description:	this will kill the specified sound effect
*		Inputs:			voice handle
*		Outputs:		voice number
*
****************************************************************************/
void killAllSFX(void)
{
	t_SoundPlayer	*sndplayer;
	INT16				i;
	t_Channel		*pChannel;


	OSPri				ospri;
	ospri = osGetThreadPri(NULL);
	osSetThreadPri(NULL, PRIORITY_AUDIOLOCK); audioSynthLock();

	//rmonPrintf("Killall...\n");

	sndplayer = &AW.SndPlayerList;
	pChannel = &AW.VoiceChannels[0];
	for(i = 0; i < MAX_VOICES; i++)
	{
		if(sndplayer->idlist[i] != -1)
		{
			//rmonPrintf("Ch(%d,%d) killed\n", i, AW.VoiceChannels[i].CFXhandle);

			if(pChannel->theSFX.StartTime > 0)
				pChannel->theSFX.StartTime = 0;

			if (SetChannel(i) != AL_STOPPED)
				StopChannel(i);
		}
		pChannel++;
	}

	audioSynthUnlock(); osSetThreadPri(NULL, ospri);

}



/*****************************************************************************
*	Function Title:  void UpdateWorldSound(void);
******************************************************************************
*	Description:	This updates world sound. It keeps track of environmental
*					variables and monitors voices. It also does clean up of
*					voices that have finished playing
*	Inputs:		None
*	Outputs:		Number of active voices still playing
*
*****************************************************************************/
void  UpdateWorldSound(void)
{
	int				i;
	t_SoundPlayer	*sndplayer;
	//ALSndPlayer		*Sndp;
	t_Channel		*pChannel;
	int				channel_count = 0;


	sndplayer = &AW.SndPlayerList;
	pChannel = &AW.VoiceChannels[0];

   for (i=0; i<MAX_VOICES; i++)
	{
		if((pChannel->PhysicalHandle != -1) && (AW.ChannelLock != i))
		{
			if(pChannel->theSFX.pEnvSound)
				UpdateEnvironmentSound(sndplayer, i);
			else
				UpdateCFXChannel(sndplayer, i);

			channel_count++;
		}
		pChannel++;

   }

	ProcessSFXQue();

	//if((AW.sfx_timer%120) == 0)
//		rmonPrintf("count: %d\n", channel_count);
		//CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_HUMAN_DEATH_SCREAM_1+RANDOM(3), 0, NULL, &AW.Ears.vPos, RANDOMIZE_PITCH);


// DO NOT UNCOMMENT - SEE IAN IF YOU'VE GOT ANY PROBLEMS WITH THIS!!!
//	DoSoundScenes();

	DoAmbientSounds();
	//ProcessSFXQue();
	DoSeqFades();
	AW.sfx_timer++;

}


/*****************************************************************************
*	Function Title: void UpdateSoundVec(int CFXhandle, CVector3 *vPos)
******************************************************************************
*	Description:
*	Inputs:
*	Outputs:
*
*****************************************************************************/
void UpdateSoundVector(int CFXhandle, CVector3 *NewPos)
{
//	int			Channel ;
	int			i ;
	t_Channel	*pChannel;


	OSPri				ospri;
	ospri = osGetThreadPri(NULL);
	osSetThreadPri(NULL, PRIORITY_AUDIOLOCK); audioSynthLock();


	pChannel = &AW.VoiceChannels[0];

	for (i=0; i < MAX_VOICES_ALL; i++)
	{
			if(CFXhandle == pChannel->CFXhandle)
				pChannel->theSFX.vPos = *NewPos;

			pChannel++;
	}

	audioSynthUnlock(); osSetThreadPri(NULL, ospri);
}

void UpdateSoundPitch(int CFXhandle, FLOAT NewPitch)
{
	// How f*cking difficult can it be to change the pitch of a sound!
	// This is a crap sound system!!

	int			i ;
	t_Channel	*pChannel;


	OSPri				ospri;
	ospri = osGetThreadPri(NULL);
	osSetThreadPri(NULL, PRIORITY_AUDIOLOCK); audioSynthLock();


	pChannel = &AW.VoiceChannels[0];

	for (i=0; i < MAX_VOICES_ALL; i++)
	{
			if(CFXhandle == pChannel->CFXhandle)
			{
				pChannel->theSFX.Pitch = NewPitch ;
				modifySFX(AW.VoiceChannels[i].VirtualHandle) ;
			}

			pChannel++;
	}

	audioSynthUnlock(); osSetThreadPri(NULL, ospri);
}

void UpdateSoundVolume(int CFXhandle, INT16 NewVolume)
{
	int			i ;
	t_Channel	*pChannel;


	OSPri				ospri;
	ospri = osGetThreadPri(NULL);
	osSetThreadPri(NULL, PRIORITY_AUDIOLOCK); audioSynthLock();


	pChannel = &AW.VoiceChannels[0];

	for (i=0; i < MAX_VOICES_ALL; i++)
	{
			if(CFXhandle == pChannel->CFXhandle)
			{
				pChannel->theSFX.Vol = NewVolume ;
				modifySFX(AW.VoiceChannels[i].VirtualHandle) ;
			}

			pChannel++;
	}

	audioSynthUnlock(); osSetThreadPri(NULL, ospri);
}

#if 0
INT16 GetSoundVolume(int CFXhandle)
{
	int			i ;
	t_Channel	*pChannel;

	pChannel = &AW.VoiceChannels[0] ;

	for (i=0; i < MAX_VOICES_ALL; i++)
	{
			if(CFXhandle == pChannel->CFXhandle)
				return pChannel->theSFX.Vol ;

			pChannel++;
	}

	return 0 ;
}
#endif


/*****************************************************************************
*	Function Title:  void UpdateEnvironmentSound(t_SoundPlayer	*sndplayer, INT16	i)
******************************************************************************
*	Description:	This will update a channel playing an environment sound
*	Inputs:
*	Outputs:
*
*****************************************************************************/
void UpdateEnvironmentSound(t_SoundPlayer	*sndplayer, INT16	i)
{
	t_Channel		*pChannel = &AW.VoiceChannels[i];
	static INT8		EnvFadeoutValue[MAX_VOICES];

	ASSERT(sndplayer->idlist[i] != -1);
	ASSERT(sndplayer->idlist[i] < MAX_VOICES);

	if((SetChannel(i) == AL_STOPPED) && (pChannel->ChannelFlags & CH_FLAG_ENV_UPDATE))
	{
		DeallocateChannel(i);
	}
	else
	{
		pChannel->ChannelFlags  |= CH_FLAG_ENV_UPDATE ;

		if((AW.sfx_timer - pChannel->theSFX.StartTime) > ENVIRONMENT_LIFE)
		{
			//rmonPrintf("env stopping(%d:%d)\n", i, AW.sfx_timer - pChannel->theSFX.StartTime);

			if((pChannel->ChannelFlags & CH_FLAG_ENV_FADEOUT) == 0)
			{
				// particle effect has finished do quick fade out
				EnvFadeoutValue[i] = 10;
				pChannel->ChannelFlags |= CH_FLAG_ENV_FADEOUT;
			}
			else
			{
				EnvFadeoutValue[i]--;
				pChannel->theSFX.Vol -= 3200;

				if(pChannel->theSFX.Vol < 0)
				{
					pChannel->theSFX.Vol = 0;
					StopChannel(i);
				}
				else
					modifySFX(AW.VoiceChannels[i].VirtualHandle);

			}
		}
	}
}


/*****************************************************************************
*	Function Title:  int VirtualToPhysical(INT32 VirtualHandle)
******************************************************************************
*	Description:
*	Inputs:
*	Outputs:
*
*****************************************************************************/
int VirtualToPhysical(INT32 VirtualHandle, INT32 HandleType)
{
	int i ;
//	int SearchKey;
	t_Channel *pChannel;



	if (VirtualHandle == -1)
		return -1;

	pChannel = &AW.VoiceChannels[0];

#if 1


	for (i=0; i < MAX_VOICES_ALL; i++)
	{
		if (pChannel->VirtualHandle == VirtualHandle)
			return i;

		pChannel++;
	}


#else

	for (i=0; i < MAX_VOICES_ALL; i++)
	{
			if(HandleType == ELEMENT_HANDLE)
				SearchKey = pChannel->VirtualHandle;
			else
				SearchKey = pChannel->CFXhandle;

			if (SearchKey == VirtualHandle)
				return i;

			pChannel++;
	}
#endif

	return -1;
}

/*****************************************************************************
*	Function Title:  int SoundHandleToChannel(INT32 VirtualHandle, INT32 HandleType)
******************************************************************************
*	Description:
*	Inputs:
*	Outputs:
*
*****************************************************************************/
int SoundHandleToChannel(INT32 VirtualHandle, INT32 HandleType)
{
	INT32		PhysicalChannel;

	PhysicalChannel = VirtualToPhysical(VirtualHandle, HandleType);

	if (PhysicalChannel == -1)
	{
		return -1;
	}

	if (PhysicalChannel == DUMMY_CHANNEL)
	{
		return DUMMY_CHANNEL;
	}

	return PhysicalChannel;
}

/*****************************************************************************
*	Function Title:  int FindEnvSound(void  *pHandle)
******************************************************************************
*	Description:
*	Inputs:
*	Outputs:
*
*****************************************************************************/
int FindEnvSound(void  *pHandle)
{
	int i;
	t_Channel *pChannel = &AW.VoiceChannels[0];

	ASSERT(pHandle);

	for (i=0; i<MAX_VOICES; i++)
	{
		if (pChannel->theSFX.pEnvSound == pHandle)
			return i;

		pChannel++;
	}

	return -1;
}

/*
//*****************************************************************************
//*	Function Title:  void SetSoundPosition(INT32 VirtualHandle, float X, float Y, float Z)
//******************************************************************************
//*	Description:
//*	Inputs:
//*	Outputs:
//*
//*****************************************************************************
#define SOUND_NOPAN_DIST	(3*SCALING_FACTOR)
void SetSoundPosition(INT32 channel, float X, float Y, float Z, int flags)
{
	t_SndObject			*pSound;
	float					dx, dy, dz, d,
							udx, udz,
							dist, invDist,
							rightX, rightZ,
							dot;
	t_Channel			*pChannel;

	//rmonPrintf("(%2.2f, %2.2f, %2.2f)\n", X, Y, Z);

	if (channel == -1)
		return;


	pChannel = &AW.VoiceChannels[channel];
	pSound = &AW.VoiceChannels[channel].theSFX;

	if(flags == INITIAL_POS)
	{
		pChannel->theSFX.vPos.x = X;
		pChannel->theSFX.vPos.y = Y;
		pChannel->theSFX.vPos.z = Z;

	}

	dx = X - AW.Ears.vPos.x;
	dy = Y - AW.Ears.vPos.y;
	dz = Z - AW.Ears.vPos.z;

	dist = sqrt(dx*dx + dy*dy + dz*dz);
	pSound->dist = dist;	// this is used later to see which sound is farther

	if (dist <= SOUND_NOPAN_DIST)
	{
		udx = 0;
		udz = 1;

		dot = 0;

		rightX = rightZ = 0;
	}
	else
	{
		invDist = 1/dist;

		udx = dx*invDist;
		udz = dz*invDist;

		rightX = -cos(AW.Ears.YRot);
		rightZ = sin(AW.Ears.YRot);

		dot = rightX*udx + rightZ*udz;
	}

	ASSERT(AW.Ears.Radius != 0);

	if ((pChannel->theSFX.pEnvSound) || (pChannel->ChannelFlags & LINK_TO_ENV))
	{
		d = min(1, max(0, dist/GetApp()->m_FarClip));
		pSound->Vol = (INT32) (pSound->MaxVol * (1 - d*d));
		pSound->Pan = ((char) ((d*dot + 1)*127)) >> 1;
	}
	else
	{
		d = min(1, max(0, dist/SCALING_FAR_CLIP));
		//pSound->Vol = (INT32) (pSound->MaxVol * (1 - min(0.7, d)));

		if (!(pChannel->ChannelFlags & ALWAYS_MAXVOL))
		{
			if(d < 0.5)
				pSound->Vol = (INT32) (pSound->MaxVol * (1 - d*d));
			else
				pSound->Vol = (INT32) (pSound->MaxVol * (1 - d*d*d));
		}

		pSound->Pan = ((char) ((dot + 1)*127)) >> 1;
	}

	// we have modified the max volume update m_MaxValue so that
	// any ramps will still work. pSound->MaxVol always carries the original maximum volume

	pChannel->VolEnv.m_MaxValue = pSound->Vol;

	if(pChannel->ChannelFlags & VOL_ENV_ENABLED )
	{
		pSound->Vol *= pChannel->VolCalc.RawValue;
	}
}
*/



//*****************************************************************************
//*	Function Title:  void SetSoundPosition(INT32 VirtualHandle, float X, float Y, float Z)
//******************************************************************************
//*	Description:
//*	Inputs:
//*	Outputs:
//*
//*****************************************************************************
#define SOUND_NOPAN_DIST	(3*SCALING_FACTOR)
void SetSoundPosition(INT32 channel, float X, float Y, float Z, int flags)
{
	t_SndObject			*pSound;
	float					dx, dy, dz, d,
							udx, udz,
							dist, invDist,
							rightX, rightZ,
							dot,
							minVol;
	t_Channel			*pChannel;

#define	DEF_FAR_CLIP		SCALING_FAR_CLIP
#define	QUIET_MINVOL		0
#define	NORMAL_MINVOL		0.5
#define	LOUD_MINVOL			0.8

	//rmonPrintf("(%2.2f, %2.2f, %2.2f)\n", X, Y, Z);

	if (channel == -1)
		return;


	pChannel = &AW.VoiceChannels[channel];
	pSound = &AW.VoiceChannels[channel].theSFX;

	if(flags == INITIAL_POS)
	{
		pChannel->theSFX.vPos.x = X;
		pChannel->theSFX.vPos.y = Y;
		pChannel->theSFX.vPos.z = Z;

	}

	dx = X - AW.Ears.vPos.x;
	dy = Y - AW.Ears.vPos.y;
	dz = Z - AW.Ears.vPos.z;

	dist = max(0.001, sqrt(dx*dx + dy*dy + dz*dz));
	pSound->dist = dist;	// this is used later to see which sound is farther

	invDist = 1 / dist;

	udx = dx*invDist;
	udz = dz*invDist;

	rightX = -cos(AW.Ears.YRot);
	rightZ = sin(AW.Ears.YRot);

	dot = rightX*udx + rightZ*udz;

	ASSERT(AW.Ears.Radius != 0);

	if ((pChannel->theSFX.pEnvSound) || (pChannel->ChannelFlags & LINK_TO_ENV))
	{
		// environment sfx
		if (GetApp()->m_FarClip == 0)
			d = 0;
		else
			d = min(1, max(0, dist/GetApp()->m_FarClip));

		pSound->Pan = (char) max(0, min(127, (d*dot + 1)*(127.0/2.0)));

		switch (pSound->RadioVolume)
		{
			case VOLUME_QUIET:	minVol = d;			break;
			case VOLUME_LOUD:		minVol = d*d*d;	break;
			case VOLUME_NORMAL:	minVol = d*d;		break;
			default:	ASSERT(FALSE);	break;
		}

		pSound->Vol = (INT32) (pSound->MaxVol * (1 - minVol));
	}
	else
	{
		// normal sfx
		switch (pSound->RadioVolume)
		{
			case VOLUME_QUIET:	minVol = QUIET_MINVOL;	break;
			case VOLUME_LOUD:		minVol = LOUD_MINVOL;	break;
			case VOLUME_NORMAL:	minVol = NORMAL_MINVOL;	break;
			default:	ASSERT(FALSE);	break;
		}

		if (dist>DEF_FAR_CLIP)
			pSound->Vol = (INT32) (minVol * pSound->MaxVol);
		else
			pSound->Vol = (INT32) ((1 + ((dist*minVol - dist) / DEF_FAR_CLIP)) * pSound->MaxVol);

		if (dist < SOUND_NOPAN_DIST)
			pSound->Pan = 64;
		else
			pSound->Pan = (char) max(0, min(127, (dot + 1)*(127.0/2.0)));
	}


	// we have modified the max volume update m_MaxValue so that
	// any ramps will still work. pSound->MaxVol always carries the original maximum volume
	pChannel->VolEnv.m_MaxValue = FLOAT2SF(pSound->Vol);

	if(pChannel->ChannelFlags & VOL_ENV_ENABLED )
	{
		pSound->Vol *= pChannel->VolCalc.RawValue;
	}
}


/*****************************************************************************
*	Function Title:  void InitSoundGlobals(float Radius)
******************************************************************************
*	Description:
*	Inputs:
*	Outputs:
*
*****************************************************************************/
void InitSoundGlobals(float Radius)
{
	AW.VolScaleFactor = MAX_WORLD_VOL/Radius;
	AW.Ears.Radius = Radius;
	AW.Ears.vPos.x = 0;
	AW.Ears.vPos.y = 0;
	AW.Ears.vPos.z = 0;
	AW.Ears.YRot = 0;

	__GlobalSFXscalar = 1.0;
	__GlobalSEQvolume = MAX_SEQ_VOL;

}

/*****************************************************************************
*	Function Title:  INT16 UpdateEars(void);
******************************************************************************
*	Description:	This updates world sound. It keeps track of environmental
*						variables and monitors voices. It also does clean up of
*						voices that have finished playing
*	Inputs:			None
*	Outputs:
*
*****************************************************************************/
void UpdateEars(t_Ears *EarLocation)
{
	AW.Ears.vPos.x = EarLocation->vPos.x;
	AW.Ears.vPos.y = EarLocation->vPos.y;
	AW.Ears.vPos.z = EarLocation->vPos.z;
	AW.Ears.YRot = EarLocation->YRot;
	AW.Ears.Radius = EarLocation->Radius;
}


/*****************************************************************************
*	Function Title: void SetupSeq(int seqnum)
******************************************************************************
*	Description:	This will set up the sequence specified and play it
*	Inputs:
*	Outputs:
*
*****************************************************************************/
void SetupSeq()
{
	u8				*seqPtr = (u8 *)SeqBuffer;
	int			i;

#ifdef PLATFORM_PORT
	{ extern void audioSynthLock(void); audioSynthLock(); }   /* serialise the seq build with the audio thread */
	/* PORT/HW: the music (seq) bank can fail to load (no ROM on the SD) -> seqbankPtr is NULL. The
	 * seqbankPtr->bankArray[0] deref below is a low-NULL read the N64 tolerates but real HW data-aborts
	 * on; bail (no music) when the bank is absent. No-op once the bank loads (the working path). */
	if (!seqbankPtr) { extern void audioSynthUnlock(void); audioSynthUnlock(); return; }
#endif
	alCSeqNew(seq, seqPtr);
   alCSPSetSeq(seqp, seq);
	alCSPSetBank(seqp, seqbankPtr->bankArray[0]);

	for(i = 0; i < MAX_VOICES; i++)
	{
		//alCSPSetChlVol(seqp, i, __GlobalSEQvolume);
		alCSPSetChlFXMix(seqp, i, 0);
	}

	alCSPSetVol(seqp, __GlobalSEQvolume);

#ifdef PLATFORM_PORT
	{ extern void audioSynthUnlock(void); audioSynthUnlock(); }
#endif
}


/*****************************************************************************
*	Function Title: void DoSeqFades()
******************************************************************************
*	Description:	void DoSeqFades(int seqnum)
*	Inputs:
*	Outputs:
*
*****************************************************************************/
void DoSeqFades()

{
	#define		MAX_SEQ_DELAY		30


	if(SeqStateAction & SEQ_FADE_DOWN)
	{
		alCSPSetVol(seqp, SeqFadeLevel);

		SeqFadeLevel -= SEQ_FADE_SPEED;

		if(SeqFadeLevel <= 0)
		{
			SeqFadeLevel = 0;
			alCSPStop(seqp);
			SeqStateAction  |= SEQ_FADE_DONE;
		}
	}
}




/*****************************************************************************
*	Function Title: BOOL LoadSeq(int nTune)
******************************************************************************
*	Description:	This will set up the sequence specified and play it
*	Inputs:
*	Outputs:	TRUE if sequence loaded, FALSE if not.
*
*****************************************************************************/
BOOL LoadSeq(int nTune)
{
	static CCacheEntry *pceSequence;

#ifdef PLATFORM_PORT
	/* S6 (music) — WORKING, default ON. The leaked source stubs music loading OFF (the unconditional
	 * return FALSE below makes RequestBinaryBlock dead code). We re-enable it: the sequence loads as a
	 * cart binary block (RNC-decompressed, keyed by MusicID), and SeqReceived swaps its big-endian
	 * ALCMidiHdr (turokCSeqHeaderSwap) before alCSeqNew parses it — without that swap the track offsets
	 * are wild pointers and alCSeqNew SIGSEGVs. Validated rc=0 + continuous music on warps
	 * 0/3000/6000/8000 (MusicID 0/14/5/8). TUROK_MUSIC=0 disables. */
#ifdef PLATFORM_3DS
	/* 3DS: the CSP music player reads a WILD track pointer mid-playback (unmapped Read16 @ ~0xEA000014 in
	 * __CSPHandleMIDIMsg, csplayer.c — an ARM-codegen/alignment issue in the untracked event parser) and
	 * crashes the level after a variable number of frames. Audio OUTPUT is off by default anyway (ndsp
	 * skipped, see audio_3ds.c), so gate music on the SAME audio_3ds flag: default 0 => no music => no CSP
	 * => the level is stable. Re-enable with turok.cfg `audio_3ds 1` once the csplayer ARM parse is fixed. */
	{ extern int g_cfg_audio_3ds, g_cfg_music; if (!g_cfg_audio_3ds || !g_cfg_music) return FALSE; }
#else
	{ static int m=-1; if(m<0){const char*e=getenv("TUROK_MUSIC"); m=(e&&!atoi(e))?0:1;} if(!m) return FALSE; }
#endif
#else
//	if (!cache_is_valid)
		return FALSE;
#endif

	return CScene__RequestBinaryBlock(&GetApp()->m_Scene,
												 nTune,
												 &pceSequence,
												 NULL, (pfnCACHENOTIFY) SeqReceived);
}


/*****************************************************************************
*	Function Title: void SeqReceived(void *pThis, CCacheEntry **ppceTarget)
******************************************************************************
*	Description:	This will set up the sequence specified and play it
*	Inputs:
*	Outputs:
*
*****************************************************************************/
void SeqReceived(void *pThis, CCacheEntry **ppceTarget)
{
	void *pbTune = CCacheEntry__GetData(*ppceTarget);
	int	size = (*ppceTarget)->m_DataLength;
//	int nRequestedSeq = CTurokMovement.MusicID;



	ASSERT(size <= SEQUENCE_BUFFER_SIZE);

	memcpy(SeqBuffer, pbTune, size);

#ifdef PLATFORM_PORT
	/* M4-S6: the sequence (ALCMidiHdr) is big-endian — swap its 68-byte header (17 DWORDs) here, once
	 * per load, BEFORE alCSeqNew parses it (else the track offsets become wild pointers -> SIGSEGV). The
	 * MIDI event stream after the header is byte-oriented, stays raw. Done in SeqReceived (not SetupSeq)
	 * so it's tied to exactly one event — the memcpy — and alCSeqNew/alCSeqNewMarker stay stock. */
	{ extern void turokCSeqHeaderSwap(u8 *ptr); turokCSeqHeaderSwap((u8 *)SeqBuffer); }
#endif

	SeqStateAction |= SEQ_LOAD_DONE;
}

/*****************************************************************************
*	Function Title: void UpdateSeq(int seqnum)
******************************************************************************
*	Description:	This will set up the sequence specified and play it
*	Inputs:
*	Outputs:
*
*****************************************************************************/
void UpdateSeq()
{
	int nRequestedSeq = CTurokMovement.MusicID;
	static	int		load_timeout = 0;
//	static	int		timeout_counter = 0;

#ifdef PLATFORM_PORT
	{ extern void *stderr; extern int fprintf(void*, const char*, ...);
	  static int s=-1, c=0; if(s<0){const char*e=getenv("TUROK_SEQLOG"); s=(e&&atoi(e))?1:0;}
	  if(s && (c++%60)==0) fprintf(stderr,"[seq] req=%d cur=%d state=%d action=%d\n",
	      nRequestedSeq, global_seqnum, SeqStateFlags, SeqStateAction); }
#endif

	//return;


	switch(SeqStateFlags)
	{

		case SEQSTATE_FADE:
			if(SeqStateAction & SEQ_FADE_DONE)
			{
				// When the fade is done, we start loading the requested sequence
				// if it is already available in the cache, we just switch to playing
				//
				SeqStateAction = 0;
				global_seqnum = -1;				// No sequence valid
				SeqStateFlags = SEQSTATE_LOAD;	// Otherwise, switch to load sequence
			}
			break;

		case SEQSTATE_LOAD:
			TRACE("Attempt Load Sequence %d\n",nRequestedSeq);
			if (LoadSeq(nRequestedSeq))		// We attempt to load the sequence again
			{
				SeqStateFlags = SEQSTATE_PLAY;	// If that passes (i.e., it just finished loading) play it
			}
			else
			{
				SeqStateFlags = SEQSTATE_LOADING;	// Otherwise, we stay in the load state
				load_timeout = 0;				// and reset the load timeout
				SeqStateAction = 0;
			}
			break;

		case SEQSTATE_LOADING:
			if( SeqStateAction & SEQ_LOAD_DONE)
			{
				SeqStateFlags = SEQSTATE_PLAY;		// If the load is complete, switch to play
//				timeout_counter = 0;
				SeqStateAction = 0;
			}
			else
			{
				load_timeout++;						// Increment timeout counter
				if( load_timeout >= 10)				// If we reach 10 frames without the sequence ready
				{
					global_seqnum = -1;				// We mark no global sequence
					SeqStateFlags = SEQSTATE_LOAD;	// Retry the load request
					TRACE0("Load timeout\n");
				}
			}
			break;

		case SEQSTATE_PLAY:
			SetupSeq();
			alCSPPlay(seqp);
			global_seqnum = nRequestedSeq ;
			SeqStateAction = 0;
			SeqStateFlags = SEQSTATE_IDLE;
			break;

		case SEQSTATE_IDLE:
		default:
			if (global_seqnum != nRequestedSeq)
			{
				TRACE2("newseq(req=%d,cur=%d)\n", nRequestedSeq, global_seqnum);

				if(global_seqnum >= 0)
				{
					SeqStateAction = 0;
					SeqFadeLevel  =  __GlobalSEQvolume;
					SeqStateAction = SEQ_FADE_DOWN;
					SeqStateFlags = SEQSTATE_FADE;
				}
				else
				{
					SeqStateAction = 0;
					SeqStateFlags = SEQSTATE_LOAD;
				}

			}

/*
			if(CTurokMovement.WaterFlag == PLAYER_BELOW_WATERSURFACE)
			{
				SeqStateAction = 0;
				SeqFadeLevel  =  __GlobalSEQvolume;
				SeqStateAction = SEQ_FADE_DOWN;
				SeqStateFlags = SEQSTATE_FADE;
			}
*/

			break;
	}
}

void InitQueue(t_WaitQue *Queue)
{
	Queue->Next = Queue;
	Queue->Prev = Queue;
}

t_WaitQue *RemoveQueueEntry(t_WaitQue *Entry)
{
	//
	// Reset the prev and next ptrs of the node before and after
	//
	Entry->Prev->Next = Entry->Next;
	Entry->Next->Prev = Entry->Prev;
	return(Entry);
}

void InsertQueueEntryBefore(t_WaitQue *Queue, t_WaitQue *Entry)
{
	//
	// Set up this entry's next and prev ptrs
	//
	Entry->Prev = Queue->Prev;
	Entry->Next = Queue;
	//
	// Link this entry into the list
	//
	Queue->Prev->Next = Entry;
	Queue->Prev = Entry;
}


// update sfx & volume settings
// Music = 0.0 to 1.0
// Sfx = 0.0 to 1.0
//
void SetAudioVolume(float Music, float Sfx)
{
#ifdef PLATFORM_PORT
	/* ★ AUDIO-THREAD RACE FIX: the synth runs on its own thread (port/audio.c) holding synthLock during
	 * alAudioFrame; this (game thread) mutates the SAME libaudio event queue via alCSPSetVol -> alEvtqPostEvent
	 * EVERY FRAME. Unsynchronised, the two corrupt the event linked-list into a cycle -> alEvtqPostEvent spins
	 * forever = the "music keeps playing but the game freezes, no crash dump" hang. Take the (recursive)
	 * synthLock around every game-thread libaudio call. */
	extern void audioSynthLock(void); extern void audioSynthUnlock(void);
	audioSynthLock();
#endif
	__GlobalSEQvolume = (INT16)(Music * MAX_SEQ_VOL);
	alCSPSetVol(seqp, __GlobalSEQvolume);

	__GlobalSFXscalar = Sfx;
#ifdef PLATFORM_PORT
	audioSynthUnlock();
#endif
}









/*****************************************************************************
*
*
*
*	AUDIO LIST CODE
*
*
*
*****************************************************************************/

#define AUDIO_LIST_LENGTH	8

typedef struct CAudioEvent_t
{
	CInstanceHdr	*m_pInst ;		// Instance
	CVector3			m_vPos ;			// Position
	float				m_Value ;		// Sound
} CAudioEvent ;


typedef struct CAudioList_t
{
	CAudioEvent		m_EventList[AUDIO_LIST_LENGTH] ;	// Array of sound events
	int				m_Total ;								// Total sounds in list
} CAudioList ;


// Global audio list
CAudioList	AudioList ;


//////////////////////////////////////////////////////////////////////////////
//	Reset list
//////////////////////////////////////////////////////////////////////////////
void CAudioList__Construct(void)
{
	CAudioList *pThis = &AudioList ;
	pThis->m_Total = 0 ;
}

//////////////////////////////////////////////////////////////////////////////
//	Add sound to list
//////////////////////////////////////////////////////////////////////////////


void CAudioList__AddGeneralSound(CInstanceHdr *pInst, CVector3 vPos, float Value)
{
	CAudioList	*pThis = &AudioList ;
	CAudioEvent	*pEvent ;

	if (pThis->m_Total >= AUDIO_LIST_LENGTH)
		return ;

	// Setup sound event
	pEvent = &pThis->m_EventList[pThis->m_Total++] ;
	pEvent->m_pInst = pInst ;
	pEvent->m_vPos = vPos ;
	pEvent->m_Value = Value ;
}

//////////////////////////////////////////////////////////////////////////////
//	Play all sounds in list
//////////////////////////////////////////////////////////////////////////////
void CAudioList__Flush(void)
{
	CAudioList *pThis = &AudioList ;
	int			i = pThis->m_Total ;
	CAudioEvent *pEvent = pThis->m_EventList ;
	while(i--)
	{
		AI_SEvent_GeneralSound(pEvent->m_pInst, pEvent->m_vPos, pEvent->m_Value) ;
		pEvent++ ;
	}

	CAudioList__Construct() ;
}


