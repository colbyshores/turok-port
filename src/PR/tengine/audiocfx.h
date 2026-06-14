#ifndef _INC_AUDIOCFX
#define _INC_AUDIOCFX


#define CheckProbability(Percent) (SOUND_RANDOM(100) < Percent)

#define AMBIENT_SOUND_RATE			30

#define AMB_RGN_JUNGLE				0
#define AMB_RGN_WATER				1
#define AMB_RGN_CAVES				2
#define AMB_RGN_CATACOMBS			3
#define AMB_RGN_LOSTLANDS			4
#define AMB_RGN_CAMPAINER			5
#define AMB_RGN_CATACOMBS_LARGE	6
#define AMB_RGN_CITY					7
#define AMB_RGN_RUINS				8
#define AMB_RGN_TREETOP				9
#define AMB_RGN_WARP					10

#define MAX_NUM_AMBIENT_SOUNDS	24


#define RANDOMIZE_VOL				(1  << 0)
#define RANDOMIZE_PITCH				(1  << 1)
#define RANDOMIZE_POS				(1  << 2)
#define AMB_RANDOMIZE_FAR			(1  << 3)
#define AMB_RANDOMIZE_NEAR			(1  << 4)
#define AMB_SCENE_POSITION			(1  << 5)
#define SMALLENEMY_PITCH			(1  << 6)
#define BIGENEMY_PITCH				(1  << 7)

typedef struct
{
	INT16		SoundNum[MAX_NUM_AMBIENT_SOUNDS];
	int		Probability;
	int		TotalNum;

} t_AmbientSounds;


// Prototypes
void SetCFXPitch(CROMSoundElement *pElement, INT32 channel, int SndFlags);
void	SetCFXVolume(CROMSoundElement *pElement, INT32 channel);
void DoSoundElement(CROMSoundElement *pElement, 
						  void *pInstance,	CVector3 *vPos,
						  float Volume, int cfxnum, int cfxhandle, int SndFlags);		
INT32 initCFX(CROMSoundElement *pElement, int cfxnum, int cfxhandle, CVector3 *vPos);
void	InitCFXVars();
float pow(float x, INT32 y);
void	UpdateCFXChannel(t_SoundPlayer *sndplayer, INT16 Channel);
void	UpdateCFXPitch(INT32 channel);
void	UpdateCFXVolume(INT32 channel);
void	UpdateCFXPanning(INT32 channel);
float ConvertPitch(float PitchCents);	
void	DoSoundScenes();
void	DoSoundRandomization(int cfxnum, CVector3 *vPos, int SndFlags);
void	SetReverbMix(int channel, int SndFlags);
void	DoFrontEndSound(int SampleNum, int SndFlags);		

#endif
