#ifndef __audio__
#define __audio__



#define SWAP_U32(a) ( ((a) << 24) | 	\
		(((a) << 8) & 0x00ff0000) | 		\
		(((a) >> 8) & 0x0000ff00) | 		\
		((unsigned long)(a) >>24) )


#define SWAP_U16(a) ( ((a & 0xff) << 8) | ((unsigned short)(a) >> 8) )

//#ifdef DEBUG_TRACE

#ifdef DEBUG_TRACE
#define AUDIOTRACE(x) rmonPrintf x

#else
#define AUDIOTRACE(x)
#endif

#define SOUND_PLAY_MUSIC
#define SOUND_PLAY_EFFECTS
#define SOUND_USE_REVERB
//#define DISABLE_SFX
#define SEQUENCE_BUFFER_SIZE 16384

#define INITIAL_POS				0
#define UPDATE_POS				1

#define LOOPED_FLAG				1
#define ENVIRONMENT_LIFE		30
#define MAX_QUE_TIME				15
#define MAX_QUE					32
#define MAX_VOICES				16
#define MAX_VOICES_ALL			MAX_VOICES+1
#define MIN_WORLD_VOL			0
#define MAX_WORLD_VOL			32000
#define MAX_PAN_LEFT				0
#define MAX_PAN_RIGHT			127
#define PAN_CENTER				64
#define TICKS_PER_MEASURE		1920

#define DUMMY_CHANNEL			MAX_VOICES
#define DUMMY_VOICE_ID			99
#define DEBUG_TRACE				1
//#define MAX_UPDATES             64
#define MAX_UPDATES           128
//#define MAX_EVENTS            64
#define MAX_EVENTS            128
//#define AUDIO_HEAP_SIZE       233472
#define AUDIO_HEAP_SIZE       299008
#define EXTRA_SAMPLES         80
#define NUM_OUTPUT_BUFFERS    3			/* Need three of these */
#define OUTPUT_RATE           22050
#define QUIT_MSG              10

#define DMA_BUFFER_LENGTH     0x800		/* Larger buffers result in fewer DMA' but more  */
//#define DMA_BUFFER_LENGTH     0x400		/* Larger buffers result in fewer DMA' but more  */
                                       /* memory being used.  */

#define NUM_ACMD_LISTS        2			/* two lists used by this example                */
#define MAX_RSP_CMDS          4096		/* max number of commands in any command list.   */
                                       /* Mainly dependent on sequences used            */

#define NUM_DMA_BUFFERS       64			/* max number of dma buffers needed.             */
                                       /* Mainly dependent on sequences and sfx's       */

#define NUM_DMA_MESSAGES      64			/* The maximum number of DMAs any one frame can  */
                                       /* have.                                         */

#define FRAME_LAG             1      /* The number of frames to keep a dma buffer.    */
                                       /* Increasing this number causes buffers to not  */
                                       /* be deleted as quickly. This results in fewer  */
                                       /* DMA's but you need more buffers.              */

#define	ENVIRONMENTAL_PRIORITY	100
#define	LOOP_PRIORITY				100
#define	DEFAULT_PRIORITY			50
#define	DEFAULT_PITCH				0
#define	DEFAULT_FXMIX				0
#define	DEFAULT_PAN					64
#define	DEFAULT_VOLUME				32000
#define	DEFAULT_RADIUS				10
#define	MAX_SEQ_LENGTH				20000
//#define EVT_COUNT					64
#define	MAX_PITCH					1
#define	MAX_SEQ_VOL					32000//0x7fff
#define	AMBIENT_FXMIX				70
#define	CATACOMBS_FXMIX			30
#define	CAVE_FXMIX					60
#define	CATACOMBS_LARGE_FXMIX	60
#define	SEQ_FX_MIX					0


#define	ELEMENT_HANDLE				0
#define	CFX_HANDLE					1


// Sequence defines
#define	MAX_SEQ						2
#define	SEQ_FADE_SPEED				500
#define	SEQ_FADE_UP					(1 << 0)
#define	SEQ_FADE_DOWN				(1 << 1)
#define	SEQ_FADE_DONE				(1 << 2)
#define	SEQ_LOAD_DONE				(1 << 3)
#define	SEQSTATE_IDLE				(1 << 4)
#define	SEQSTATE_FADE				(1 << 5)
#define	SEQSTATE_LOAD				(1 << 6)
#define	SEQSTATE_PLAY				(1 << 7)
#define	SEQSTATE_LOADING			(1 << 8)


#define	CUT_OFF						0
#define	FADE_OUT						1


typedef struct {
    ALVoice     voice;
    ALSound     *sound;         /* sound referenced here */
    s16         priority;
    f32         pitch;          /* current playback pitch                    */
    s32         state;          /* play state for this sound                 */
    s16         vol;            /* volume - combined with volume from bank   */
    ALPan       pan;            /* pan - 0 = left, 127 = right               */
    u8          fxMix;          /* wet/dry mix - 0 = dry, 127 = wet          */
} ALSoundState;


typedef struct
{
    u32       outputRate;
    u32       framesPerField;
    u32       maxACMDSize;
} amConfig;




// *********Audio Structures********

typedef struct
{
	void			*ROMaddr;
	void			*RAMaddr;
	INT32			size;

} t_SeqInfo;


typedef struct
{
	ALBank		*sfxBank;
	u8				*sfxBankPtr;
	ALSndPlayer	Sndp;						// first sound player
	ALSndPlayer	*NextPlayer;			// pointer to any additional players allocated
	ALSndId		idlist[MAX_VOICES];	// id list of sounds allocated to this player
	INT16			SoundCount;				// sound count number of sounds allocated to this player
} t_SoundPlayer;

// forward declaration
struct CROMSoundElement_t;

typedef struct
{
	DWORD						StartTime;		// start time
	INT32						Vol;				// current volume
	INT32						MaxVol;			// maxvol
	INT16						FXmix;			// the FXmix
	float						Pitch;			// the pitch
	INT16						Pan;				// the pan
	void						*pEnvSound;		// pointer to instance of env sound
	INT16						Priority;
	INT16						SfxNum;
	INT16						CfxNum;
	float						dist;			// distance from player
	INT32						QuedVoice;		// holds the virtual voice of the sfx
	CVector3					vPos;
	BYTE						RadioVolume,
								unused2;
} t_SndObject;





typedef struct
{
	float RawValue;
	float	IncrValue;
} t_EnvClac;

typedef struct
{
	INT32						CFXhandle;
	INT32						PhysicalHandle;
	INT32						VirtualHandle;
	t_SndObject				theSFX;
	CROMEnvelope			PitchEnv;
	t_EnvClac				PitchCalc;
	CROMEnvelope			VolEnv;
	t_EnvClac				VolCalc;
	INT32						delay;
	ALSndPlayer				*SndPlayer;
	int						ChannelFlags;
} t_Channel;

typedef struct s_WaitQue
{
	struct s_WaitQue *Prev;
	struct s_WaitQue *Next;
	t_Channel		QuedChannel;
} t_WaitQue;

extern t_WaitQue WaitQueue;
extern t_WaitQue AvailQueue;

typedef struct
{
	CVector3	vPos;
	float		YRot;
	float		Radius;

} t_Ears;


typedef struct
{

	ALSynConfig		SynthConfig;		// info on how synth is configured
	ALSndpConfig	SPConfig;			// info on how sound player is configured
	ALGlobals		GlobalAudio;		// audio environment globals

	t_SoundPlayer	SndPlayerList;		// list of sound players
	INT16				NumPlayers;			// number of current players allocated
	t_Channel		VoiceChannels[MAX_VOICES_ALL];	// audio channels
	t_WaitQue		SFXWaitQue[MAX_QUE];	// sfx wait que

	INT16				ChannelLock;
	t_Ears			Ears;
	float				VolScaleFactor;
	float				PanScaleFactor;

	INT32				sfx_timer;
	INT32				sfx_counter;
	INT32				cfx_counter;
} t_AudioWorld;

void	UpdateEars(t_Ears *EarLocation);
void	InitSoundGlobals(float Radius);
INT16 playElement(INT32 VirtualHandle, CVector3 *vPos, int flags);
INT16 GetNextVoice(INT16	cfxnum, INT16	sfxnum, INT16 Priority, CVector3 *vPos, WORD Flags);
void	killCFX(INT32 VirtualHandle);
void	UpdateWorldSound(void);
void  amCreateAudioMgr(ALSynConfig *c, OSPri priority, amConfig *amc);
int	SoundHandleToChannel(INT32 VirtualHandle, INT32 HandleType);
void	SetSoundPosition(INT32 channel, float X, float Y, float Z, int flags);
INT16 modifySFX(INT32 VirtualHandle);
INT16	CheckChannelStatus(INT16 sfxnum, int Channel, INT16 Priority);
void	EnqueSFX(int Channel, INT32 VirtualHandle, CVector3 *vPos);
void	ProcessSFXQue();
INT32 PlayEnvironmentSound(CROMSoundElement *pElement,
									void *pInstance, CVector3 *vPos,
									float Volume, float Pitch);
void	UpdateEnvironmentSound(t_SoundPlayer	*sndplayer, INT16	i);
int	FindEnvSound(void  *pHandle);
int	VirtualToPhysical(INT32 VirtualHandle, INT32 HandleType);
void	PlaySeq(int seqnum);
void	StopSeq(int StopMode);
void	DoSeqFades();
void	SetupSeq();
void	StopChannel(INT16 Channel);
int	SetChannel(INT16 Channel);
int	DeallocateChannel(INT16 Channel);
void	StartChannel(INT16 Channel);
void	UpdateSeq();
BOOL	LoadSeq(int nTune);
void	SeqReceived(void *pThis, CCacheEntry **ppceTarget);

void	UpdateSoundVector(int CFXhandle, CVector3 *NewPos);
void	UpdateSoundPitch(int CFXhandle, FLOAT NewPitch) ;
void	UpdateSoundVolume(int CFXhandle, INT16 NewVolume) ;
INT16 GetSoundVolume(int CFXhandle) ;
void	SetAudioVolume(float Music, float Sfx);
void	killSoundType(int nType);


// CAudioList class
void CAudioList__Construct(void) ;
void CAudioList__AddGeneralSound(CInstanceHdr *pInst, CVector3 vPos, float Value) ;
void CAudioList__Flush(void) ;



///// Externals
extern u64						audYieldBuf[];
extern u64						audioHeap[];
extern t_AudioWorld			AW;
extern float					__GlobalSFXscalar;
extern INT16					__GlobalSEQvolume;
extern ALCSPlayer				*seqp;
//extern unsigned long			audio_ambient_flags;

#endif

