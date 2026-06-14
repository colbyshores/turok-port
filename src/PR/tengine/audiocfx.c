#include <ultra64.h>
#include <libaudio.h>
#include <ultralog.h>
#include <ramrom.h>
#include <rmon.h>
#include "tengine.h"
#include "tmove.h"
#include "audio.h"
#include "audiocfx.h"
#include "scaling.h"
#include "snddefs.h"
#include "sfx.h"
#include "scene.h"
#include "regtype.h"



#define MAX_SOUND_SCENES					4
#define MAX_VOLUMETABLE_ENTRIES			91
#define MAX_PITCHTABLE_ENTRIES			120
#define MAX_RAND_DISPLACEMENT				10*SCALING_FACTOR
#define MAX_SOUND_SCENE_DISP				20*SCALING_FACTOR
#define BASE_DISTANCE						40*SCALING_FACTOR


extern char *test_sound_effects[];
int	db_CFX = -1;
int	db_CFX_counter = 0;

// globals
INT16    CFXVolumeTable[MAX_VOLUMETABLE_ENTRIES];
INT16    AmbientSoundRegion = -1;

#define PITCH_OFFSET_COUNT 11

INT16		PitchOffset[] = {100,80,60,40,20,0,-20,-40,-60,-80,-100} ;

t_AmbientSounds	AmbientSoundTable[] =
{
    // jungle
    {{SOUND_GENERIC_54, SOUND_GENERIC_184, SOUND_GENERIC_183, SOUND_BIRD_CALL_3,
      SOUND_MONKEY_CHIPSCREAM_3,  SOUND_GENERIC_183,  SOUND_GENERIC_55,  SOUND_GENERIC_30,
      SOUND_GENERIC_55,  SOUND_BIRD_CALL_4, SOUND_MONKEY_CHIPSCREAM_3, SOUND_MONKEY_CHIPSCREAM_4,
      SOUND_MONKEY_CHIPSCREAM_4,  SOUND_GENERIC_54,  SOUND_GENERIC_28,  SOUND_GENERIC_51,
      SOUND_GENERIC_56,  SOUND_GENERIC_55,  SOUND_GENERIC_49,  SOUND_GENERIC_50,
      SOUND_GENERIC_51,  SOUND_GENERIC_52,  SOUND_GENERIC_53,  SOUND_GENERIC_184},
     22,24},

	// water
    {{SOUND_BIRD_CALL_4, SOUND_BIRD_CALL_5, SOUND_BIRD_CALL_6, SOUND_BIRD_CALL_7,
      SOUND_GENERIC_26,  SOUND_GENERIC_29,  SOUND_GENERIC_30,  SOUND_GENERIC_32,
      SOUND_GENERIC_35,  SOUND_GENERIC_31,  SOUND_GENERIC_33,  SOUND_GENERIC_34,
      SOUND_CRICKET_CHIRP, SOUND_CICAADA_CHIRP, SOUND_LOCUST_CHIRP, SOUND_CICAADA_CHIRP,
      SOUND_GENERIC_54,  SOUND_GENERIC_56, SOUND_MONKEY_CHIPSCREAM_1, SOUND_MONKEY_CHIPSCREAM_2,
      SOUND_MONKEY_CHIPSCREAM_3, SOUND_GENERIC_53, SOUND_GENERIC_57, SOUND_GENERIC_58},
    30,24},

	//caves
    {{SOUND_GENERIC_42, SOUND_GENERIC_43, SOUND_GENERIC_44, SOUND_GENERIC_45,
      SOUND_GENERIC_46, SOUND_GENERIC_59, SOUND_GENERIC_60, SOUND_GENERIC_61,
      SOUND_OMINOUS_CAVE_GROWL_1, SOUND_OMINOUS_CAVE_GROWL_2, SOUND_OMINOUS_CAVE_GROWL_1, SOUND_OMINOUS_CAVE_GROWL_2,
      -1, -1, -1, -1,
	  -1, -1 ,-1, -1,
	  -1, -1, -1, -1},
      15,12},

	// catacombs
    {{SOUND_OMINOUS_CAVE_GROWL_1, SOUND_OMINOUS_CAVE_GROWL_2, SOUND_OMINOUS_CAVE_GROWL_3, SOUND_GENERIC_62,
      SOUND_GENERIC_63, SOUND_GENERIC_64, SOUND_WIND_BLOW_5, SOUND_WIND_BLOW_4,
      SOUND_WIND_BLOW_5, SOUND_WIND_BLOW_4, SOUND_WIND_BLOW_5, SOUND_WIND_BLOW_4,
      SOUND_WIND_BLOW_3, SOUND_WIND_BLOW_4, SOUND_WIND_BLOW_3, SOUND_WIND_BLOW_4,
      SOUND_GENERIC_63, SOUND_GENERIC_64, -1, -1,
	  -1,-1,-1,-1},
      13,18},

	// lost lands
	{{SOUND_THUNDER_ROLL_1, SOUND_THUNDER_ROLL_2, SOUND_THUNDER_ROLL_3, SOUND_THUNDER_ROLL_4,
	  SOUND_WIND_BLOW_3, SOUND_WIND_BLOW_4, SOUND_WIND_BLOW_3, SOUND_WIND_BLOW_4,
	  SOUND_GENERIC_183, SOUND_GENERIC_183, SOUND_GENERIC_242, SOUND_GENERIC_183,
	  -1,-1,-1,-1,
	  -1,-1,-1,-1,
	  -1,-1,-1,-1},
	  10,12},

	// campaigner
    {{SOUND_GENERIC_37, SOUND_GENERIC_38, SOUND_GENERIC_39, SOUND_GENERIC_40,
      SOUND_GENERIC_41, -1, -1,-1,
	  -1,-1,-1,-1,
	  -1,-1,-1,-1,
	  -1,-1,-1,-1,
	  -1,-1,-1,-1},
      5,5},

	// catacombs large
    {{SOUND_OMINOUS_CAVE_GROWL_1, SOUND_OMINOUS_CAVE_GROWL_2, SOUND_OMINOUS_CAVE_GROWL_3, SOUND_GENERIC_62,
      SOUND_GENERIC_63, SOUND_GENERIC_64, SOUND_WIND_BLOW_5, SOUND_WIND_BLOW_4,
      SOUND_WIND_BLOW_5, SOUND_WIND_BLOW_4, SOUND_WIND_BLOW_5, SOUND_WIND_BLOW_4,
      SOUND_WIND_BLOW_3, SOUND_WIND_BLOW_4, SOUND_WIND_BLOW_3, SOUND_WIND_BLOW_4,
      SOUND_GENERIC_63, SOUND_GENERIC_64, -1, -1,
	  -1,-1,-1,-1},
      13,18},

	// city
	{{SOUND_WIND_BLOW_5, SOUND_WIND_BLOW_4, SOUND_WIND_BLOW_5, SOUND_WIND_BLOW_4,
	  SOUND_WIND_BLOW_5, SOUND_WIND_BLOW_4, SOUND_WIND_BLOW_5, SOUND_WIND_BLOW_4,
	  SOUND_WIND_BLOW_3, SOUND_WIND_BLOW_3, SOUND_GENERIC_184, SOUND_GENERIC_184,
	  SOUND_GENERIC_64, SOUND_GENERIC_64, -1, -1,
	  -1,-1,-1,-1,
	  -1,-1,-1,-1},
	  8,14},

	// ruins
	{{SOUND_WIND_BLOW_5, SOUND_WIND_BLOW_4, SOUND_WIND_BLOW_5, SOUND_WIND_BLOW_4,
	  SOUND_WIND_BLOW_5, SOUND_WIND_BLOW_4, SOUND_WIND_BLOW_5, SOUND_WIND_BLOW_4,
	  SOUND_WIND_BLOW_3, SOUND_WIND_BLOW_3, SOUND_GENERIC_242, SOUND_GENERIC_184,
	  -1,-1,-1,-1,
	  -1,-1,-1,-1,
	  -1,-1,-1,-1},
	  8,12},

    // treetop
	{{SOUND_GENERIC_36, SOUND_THUNDER_ROLL_1, SOUND_BIRD_CALL_2, SOUND_THUNDER_ROLL_4,
      SOUND_MONKEY_CHIPSCREAM_3,  SOUND_THUNDER_ROLL_2, SOUND_GENERIC_55,  SOUND_GENERIC_30,
      SOUND_GENERIC_31,  SOUND_THUNDER_ROLL_3, SOUND_MONKEY_CHIPSCREAM_3, SOUND_MONKEY_CHIPSCREAM_4,
      SOUND_MONKEY_CHIPSCREAM_4,  SOUND_GENERIC_54,  SOUND_GENERIC_28,  SOUND_GENERIC_51,
      SOUND_GENERIC_56,  SOUND_GENERIC_55,  SOUND_GENERIC_49,  SOUND_GENERIC_50,
      SOUND_THUNDER_ROLL_1, SOUND_THUNDER_ROLL_2, SOUND_THUNDER_ROLL_3, SOUND_THUNDER_ROLL_4},
      10,24},

	// warp
	{{-1, -1,-1,-1,
	  -1,-1,-1,-1,
	  -1,-1,-1,-1,
	  -1,-1,-1,-1,
	  -1,-1,-1,-1,
	  -1,-1,-1,-1},
	  25,0}
};


INT16	SoundScenes[] =
{
	// Raptor vs Human
	SOUND_RAPTOR_ALERT, SOUND_HUMAN_EFFORTINJURY_GRUNT_1, SOUND_GENERIC_8, SOUND_GENERIC_8,
   SOUND_GENERIC_8,  SOUND_RAPTOR_ATTACK,  SOUND_HUMAN_EFFORTINJURY_GRUNT_2,  SOUND_HUMAN_EFFORTINJURY_GRUNT_3,

	// Hulk vs Leaper
	 SOUND_HULK_ALERT, SOUND_HULK_ATTACK, SOUND_STONEBOULDER_THUD, SOUND_LEAPER_INJURY,
    SOUND_LEAPER_ATTACK,  SOUND_STONEBOULDER_THUD,  SOUND_HULK_ATTACK,  SOUND_LEAPER_DEATH,

	// Robot vs Human
	SOUND_ROBOT_FOOTFALL, SOUND_ROBOT_SHORT_2, SOUND_GENERIC_8, SOUND_ROBOT_FOOTFALL,
   SOUND_HUMAN_EFFORTINJURY_GRUNT_1,  SOUND_MISSILE_LAUNCH,  SOUND_ROBOT_FOOTFALL,  SOUND_HUMAN_DEATH_SCREAM_2,

	// Human vs Human
	SOUND_HUMAN_EFFORTINJURY_GRUNT_3, SOUND_HUMAN_EFFORTINJURY_GRUNT_1, SOUND_GENERIC_8, SOUND_PISTOL_SHOT,
   SOUND_GENERIC_8,  SOUND_AUTO_SHOTGUN_SHOT,  SOUND_HUMAN_EFFORTINJURY_GRUNT_2,  SOUND_HUMAN_DEATH_SCREAM_1,


};

float		CFXPitchTable[] =
{

		0.0313, 0.0324, 0.0335,	0.0347,
		0.0359, 0.0372, 0.0385, 0.0398, 0.0412, 0.0427,
		0.0442, 0.0458, 0.0474, 0.0490, 0.0508, 0.0526,
		0.0544, 0.0563, 0.0583, 0.0604, 0.0625,0.0647,
		0.0670, 0.0693, 0.0718, 0.0743, 0.0769, 0.0797,
		0.0825, 0.0854, 0.0884, 0.0915, 0.0947, 0.0981,
		0.1015, 0.1051, 0.1088, 0.1127, 0.1166, 0.1207,
		0.125, 0.129, 0.134, 0.139, 0.144, 0.149, 0.154,
		0.159, 0.165, 0.171, 0.177, 0.183, 0.189, 0.196,
		0.203, 0.21,  0.218, 0.225, 0.233, 0.241, 0.25,
		0.259, 0.268, 0.277, 0.287, 0.297, 0.308, 0.319,
		0.33,  0.342, 0.354, 0.366, 0.379, 0.392, 0.41,
		0.42,  0.435, 0.451, 0.467, 0.483, 0.50,  0.518,
		0.536, 0.555, 0.574, 0.595, 0.616, 0.637, 0.66,
		0.683, 0.707, 0.732, 0.758, 0.785, 0.812, 0.841,
		0.871, 0.901, 0.933, 0.966, 1.00,  1.04, 	1.07,
		1.11,  1.15,  1.20, 	1.23,  1.28,  1.32,  1.37,
		1.41,  1.46,  1.52,  1.57,  1.63,  1.68,  1.74,
		1.80,  1.87,  1.93,  2.00
};

//#define		BlendFLOAT(u, A, B) ((A) + (u)*(((float)(B)) - (A)))

/*****************************************************************************
*   Function Title: INT32 pow(float x, INT32 y)
******************************************************************************
*   Description:    This will initialize tables and things
*   Inputs:
*	Outputs:
*
*****************************************************************************/
float pow(float x, INT32 y)
{
	register float		temp1;
	int		j;

	temp1 = 1;
	for(j = 0; j < y; j++)
		temp1 *= x;

	return temp1;
}


/*****************************************************************************
*	Function Title: void DoSoundElement(CROMSoundElement *elements, float x, float y, float z, float Volume, int cfxnum);
******************************************************************************
*	Description:
*	Inputs:
*	Outputs:
*
*****************************************************************************/
void DoSoundElement(CROMSoundElement *pElement,
						  void *pInstance,	CVector3 *vPos,
						  float Volume, int cfxnum, int cfxhandle, int SndFlags)
{

	INT32						handle, channel;
	CVector3					VecPos;


	if (		(pElement->m_wFlags & LOOPED_FLAG)
			&&	(pInstance) )
	{

		PlayEnvironmentSound(pElement, pInstance,	vPos,
								SF2FLOAT(pElement->m_Volume.m_MaxValue) * Volume,
								SF2FLOAT(pElement->m_Pitch.m_MaxValue));
	}
	else
	{

		handle = initCFX(pElement, cfxnum, cfxhandle, vPos);

		channel = SoundHandleToChannel(handle, 0);
 		if (channel == -1)
		{
			//rmonPrintf("SoundDropped\n");
			//rmonPrintf("SoundDropped: %d, %s\n", pElement->m_nSampleNum, test_sound_effects[pElement->m_nSampleNum]);

			return;
		}

		SetReverbMix(channel, SndFlags);
		SetCFXPitch(pElement, channel, SndFlags);
		SetCFXVolume(pElement, channel);

		VecPos.x = vPos->x;
		VecPos.y = vPos->y;
		VecPos.z = vPos->z;
		DoSoundRandomization(cfxnum, &VecPos, SndFlags);
		SetSoundPosition(channel, VecPos.x, VecPos.y, VecPos.z, INITIAL_POS);
		playElement(handle, &VecPos, 0);
	}

}

/*****************************************************************************
*	Function Title: void DoSoundElement(CROMSoundElement *elements, float x, float y, float z, float Volume, int cfxnum);
******************************************************************************
*	Description:
*	Inputs:
*	Outputs:
*
*****************************************************************************/
void DoFrontEndSound(int SampleNum, int SndFlags)
{
#if 1
	INT32						handle, channel;
	CVector3					VecPos;
	CROMSoundElement		Element;

	Element.m_Pitch.m_MaxValue = FLOAT2SF(0.0);
	Element.m_Pitch.m_StartFactor = FLOAT2SF(0.0);
	Element.m_Pitch.m_EndFactor = FLOAT2SF(0.0);
	Element.m_Pitch.m_nDuration = 0;
	Element.m_Pitch.m_EnvStartTime = 0;

	Element.m_Volume.m_MaxValue = FLOAT2SF(0.0);
	Element.m_Volume.m_StartFactor = FLOAT2SF(0.0);
	Element.m_Volume.m_EndFactor = FLOAT2SF(0.0);
	Element.m_Volume.m_nDuration = 0;
	Element.m_Volume.m_EnvStartTime = 0;

	Element.m_nSampleNum = SampleNum;
	Element.m_nDelayTime = 0;
	Element.m_Priority = DEFAULT_PRIORITY;
	Element.m_wFlags = 0;
	Element.m_Probability = 100;

	handle = initCFX(&Element, 0, 0, &AW.Ears.vPos);
	channel = SoundHandleToChannel(handle, 0);
	if (channel == -1)
	{
		//rmonPrintf("sound dropped\n");
		return;
	}

	SetCFXPitch(&Element, channel, 0);
	SetCFXVolume(&Element, channel);


	VecPos.x = AW.Ears.vPos.x;
	VecPos.y = AW.Ears.vPos.y;
	VecPos.z = AW.Ears.vPos.z;

	//rmonPrintf("play sound (%d)\n", handle);

	playElement(handle, &VecPos, 0);
#endif

}

/*****************************************************************************
*	Function Title: void DoSoundRandomization(CVector3 *vPos, int SndFlags)
******************************************************************************
*	Description:
*	Inputs:
*	Outputs:
*
*****************************************************************************/
void DoSoundRandomization(int cfxnum, CVector3 *vPos, int SndFlags)
{
	float						sinRotY,
								cosRotY;
	static float			Angle;
	static int				randpos = MAX_RAND_DISPLACEMENT;
	static int				prev_cfxnum = -1;



	if((SndFlags & RANDOMIZE_POS) || (SndFlags & AMB_RANDOMIZE_FAR) || (SndFlags & AMB_SCENE_POSITION))
	{
		vPos->y = AW.Ears.vPos.y;	// elevation is not implemented
		vPos->x = AW.Ears.vPos.x;
		vPos->z = AW.Ears.vPos.z;

		if(prev_cfxnum != cfxnum)
		{
			// get new position and angle
			randpos = (SOUND_RANDOM(1000)/1000.0)*(MAX_RAND_DISPLACEMENT);
			Angle = ANGLE_DTOR(SOUND_RANDOM(360));
		}

		vPos->x += (BASE_DISTANCE+randpos)*sin(Angle);
		vPos->z += (BASE_DISTANCE+randpos)*cos(Angle);

		if(SndFlags & AMB_SCENE_POSITION)
		{
			vPos->x = AW.Ears.vPos.x;
			vPos->z = AW.Ears.vPos.z;
			vPos->z = AW.Ears.vPos.z;
			// calculate sin & cos for straight ahead direction
			sinRotY = sin(GetApp()->m_RotY + ANGLE_PI);
			cosRotY = cos(GetApp()->m_RotY + ANGLE_PI);
			vPos->x += sinRotY*((BASE_DISTANCE+MAX_SOUND_SCENE_DISP)*SCALING_FACTOR);
			vPos->z += cosRotY*((BASE_DISTANCE+MAX_SOUND_SCENE_DISP)*SCALING_FACTOR);
		}
	}

	prev_cfxnum = cfxnum;

}




/*****************************************************************************
*	Function Title: INT32 initCFX(CROMSoundElement *pElement, int cfxnum, CVector3 *vPos)
******************************************************************************
*	Description:
*	Inputs:
*	Outputs:
*
*****************************************************************************/
INT32 initCFX(CROMSoundElement *pElement, int cfxnum, int cfxhandle, CVector3 *vPos)
{
	static int		count;
	t_SndObject		*sndptr;
	INT16				NextVoice;
	ALSndId			*idPtr;
	ALSound			*newsndptr;
	ALSndPlayer		*PlayerPtr;
	t_SoundPlayer	*PlayerInfo;
	ALInstrument	*inst;
	OSPri				ospri;
	int				sfxnum = pElement->m_nSampleNum;
	t_Channel		*pChannel;
	INT16				Priority = pElement->m_Priority;

	ASSERT(pElement);


	// does sfxnum exist ?
	if (sfxnum > AW.SndPlayerList.sfxBank->instArray[0]->soundCount)
		return -1;

#ifdef DISABLE_SFX

	return -1;

#endif

	ospri = osGetThreadPri(NULL);
	osSetThreadPri(NULL, PRIORITY_AUDIOLOCK);

	if(pElement->m_Probability != 100)
	{
		if(CheckProbability(pElement->m_Probability) == FALSE)
		{
			osSetThreadPri(NULL, ospri);
			return -1;
		}
	}


	NextVoice = GetNextVoice(cfxnum, sfxnum, Priority, vPos, pElement->m_wFlags);

	if (NextVoice == -1)
	{
		osSetThreadPri(NULL, ospri);
		return -1;
	}

	pChannel = &AW.VoiceChannels[NextVoice];


	if (NextVoice == DUMMY_CHANNEL)
	{
		pChannel->CFXhandle = cfxhandle;
		pChannel->VirtualHandle = AW.sfx_counter++;
		pChannel->PhysicalHandle = DUMMY_VOICE_ID;
		//rmonPrintf("DUMMY CHANNEL\n");
	}
	else
	{
		//ASSERT(AW.ChannelLock == -1);
		AW.ChannelLock = NextVoice;

		// allocate sound in player
		inst =  AW.SndPlayerList.sfxBank->instArray[0];
		idPtr = AW.SndPlayerList.idlist;
		PlayerPtr = &AW.SndPlayerList.Sndp;
		PlayerInfo = &AW.SndPlayerList;

		newsndptr = inst->soundArray[sfxnum];
		idPtr[NextVoice] = alSndpAllocate(PlayerPtr, newsndptr);

		// we were unable to allocate sound
		if (idPtr[NextVoice] == -1)
		{
			pChannel->VirtualHandle = -1;
			pChannel->PhysicalHandle = -1;
			pChannel->theSFX.Priority = 0;
			AW.ChannelLock = -1;

			osSetThreadPri(NULL, ospri);
			return -1;
		}
		else
		{
			PlayerInfo->SoundCount++;
			pChannel->CFXhandle = cfxhandle;
			pChannel->VirtualHandle = AW.sfx_counter++;
			pChannel->PhysicalHandle = idPtr[NextVoice];
			pChannel->delay = pElement->m_nDelayTime;
		}
	}

	sndptr = &pChannel->theSFX;
	sndptr->Priority = Priority;
	sndptr->MaxVol = DEFAULT_VOLUME;
	sndptr->Vol = DEFAULT_VOLUME;
	sndptr->Pitch = DEFAULT_PITCH;
	sndptr->FXmix = 0;
	sndptr->Pan = DEFAULT_PAN;
	sndptr->pEnvSound = 0;
	sndptr->SfxNum = sfxnum;
	sndptr->CfxNum = cfxnum;
	sndptr->StartTime = AW.sfx_timer;
	sndptr->RadioVolume = pElement->m_RadioVolume;
   pChannel->ChannelFlags = pElement->m_wFlags;

	osSetThreadPri(NULL, ospri);
	return pChannel->VirtualHandle;
}


/*****************************************************************************
*       Function Title: void SetReverbMix(int channel, int SndFlags)
******************************************************************************
*       Description:
*       Input:
*       Output:
*
*****************************************************************************/
void SetReverbMix(int channel, int SndFlags)
{

	t_Channel		*pChannel = &AW.VoiceChannels[channel];
	//int				dwFlags;


	if (		(CTurokMovement.WaterFlag == PLAYER_BELOW_WATERSURFACE)
			&&	(!CTurokMovement.InAntiGrav) )
	{
		pChannel->theSFX.FXmix = 0;
		return;
	}

	if((SndFlags & AMB_RANDOMIZE_FAR) || (SndFlags & AMB_SCENE_POSITION))
	{
		pChannel->theSFX.FXmix = AMBIENT_FXMIX;
		return;
	}


	//dwFlags = audio_ambient_flags;

	switch(CTurokMovement.AmbientSound)
	{
		case REG_AMBIENTCAVES:
			pChannel->theSFX.FXmix = CAVE_FXMIX;
			break;

		case REG_AMBIENTCATACOMBS:
			pChannel->theSFX.FXmix = CATACOMBS_FXMIX;
			break;

		case REG_AMBIENTCATACOMBLARGE:
			pChannel->theSFX.FXmix = CATACOMBS_LARGE_FXMIX;
			break;
	}


/*	if(dwFlags & REGFLAG_AMBIENTCAVES)
		pChannel->theSFX.FXmix = CAVE_FXMIX;

	else if(dwFlags & REGFLAG_AMBIENTCATACOMBS)
		pChannel->theSFX.FXmix = CATACOMBS_FXMIX;

	else if(dwFlags & REGFLAG_AMBIENTCATACOMBLARGE)
		pChannel->theSFX.FXmix = CATACOMBS_LARGE_FXMIX;
*/

}


/*****************************************************************************
*       Function Title: void SetCFXPitch(INT32 Channel, t_ELEMENT_INFO *pElementInfo)
******************************************************************************
*       Description:    This will set the pitch of a CFX sound. Pitch is specified
*                       in cents: 0 is no change in pitch, +1200 is up an octave
*                       -1200 is down an octave, and -2400 is down 2 octaves
*       Input:
*       Output:
*
*****************************************************************************/
void SetCFXPitch(CROMSoundElement *pElement, INT32 channel, int SndFlags)
{
	CROMEnvelope	*pEnv = &AW.VoiceChannels[channel].PitchEnv ;
	t_SndObject		*pSound = &AW.VoiceChannels[channel].theSFX;
	t_Channel		*pChannel = &AW.VoiceChannels[channel];
	float				PitchCents;
	float				dif;
//	static int		RandPitch = 0;



	// copy pitch env to channel
	*pEnv = pElement->m_Pitch;

	if((SndFlags & RANDOMIZE_PITCH) || (pElement->m_wFlags & CH_FLAG_RAND_PITCH))
		pEnv->m_MaxValue  = FLOAT2SF (SF2FLOAT(pEnv->m_MaxValue) + PitchOffset[SOUND_RANDOM(PITCH_OFFSET_COUNT)]);

	if (SndFlags & SMALLENEMY_PITCH)
		pEnv->m_MaxValue = FLOAT2SF (SF2FLOAT(pEnv->m_MaxValue) + 1000.0);

	if (SndFlags & BIGENEMY_PITCH)
		pEnv->m_MaxValue = FLOAT2SF (SF2FLOAT(pEnv->m_MaxValue) - 1000.0);

	if (SF2FLOAT(pEnv->m_MaxValue) >= 1200.0)
		pEnv->m_MaxValue = FLOAT2SF(1200.0);

   if (pElement->m_wFlags & PITCH_ENV_ENABLED)
	{
		pSound->Pitch = SF2FLOAT(pEnv->m_MaxValue);
		dif = (SF2FLOAT(pEnv->m_StartFactor) - SF2FLOAT(pEnv->m_EndFactor));
		pChannel->PitchCalc.IncrValue = dif/pEnv->m_nDuration;     // calculate incriment
		pChannel->PitchCalc.RawValue = SF2FLOAT(pEnv->m_StartFactor);
	}

	//PitchCents = pElement->m_Pitch.m_MaxValue;
	PitchCents = SF2FLOAT(pEnv->m_MaxValue);
	pSound->Pitch = ConvertPitch(PitchCents);
	pEnv->m_MaxValue = FLOAT2SF(pSound->Pitch);


#if 0
	rmonPrintf("Env, Max: %2.2f\n",  SF2FLOAT(pEnv->m_MaxValue));
	rmonPrintf("Env, Start: %2.2f\n", SF2FLOAT(pEnv->m_StartFactor));
	rmonPrintf("Env, End: %2.2f\n", SF2FLOAT(pEnv->m_EndFactor));
	rmonPrintf("Env, Dur: %d\n", pEnv->m_nDuration);
	rmonPrintf("Env, Time: %d\n", pEnv->m_EnvStartTime);
	rmonPrintf("Incr: %2.5f\n", pChannel->PitchCalc.IncrValue);
	rmonPrintf("Raw: %2.5f\n", pChannel->PitchCalc.RawValue);
#endif

}

/*****************************************************************************
*       Function Title: void SetCFXVolume(INT32 Channel, t_ELEMENT_INFO *pElementInfo)
******************************************************************************
*       Description:    This will set the volume of a CFX sound. CFX volumes are
*                       specified in decibles. 0 means maximum volume -90 means
*                       no sound. A lookup table must be used to convert
*                       to the volume scale the N64 uses
*       Input:
*       Output:
*
*****************************************************************************/
void SetCFXVolume(CROMSoundElement *pElement, INT32 channel)
{
   int               dbVolume;
	t_SndObject			*pSound = &AW.VoiceChannels[channel].theSFX;
	CROMEnvelope		*pEnv = &AW.VoiceChannels[channel].VolEnv ;
	t_Channel			*pChannel = &AW.VoiceChannels[channel];
	float					dif;


	if (pElement->m_wFlags & VOL_ENV_ENABLED)
	{
		*pEnv = pElement->m_Volume;
		pSound->Vol = SF2FLOAT(pEnv->m_MaxValue);
		dif = (SF2FLOAT(pEnv->m_StartFactor) - SF2FLOAT(pEnv->m_EndFactor));
		pChannel->VolCalc.IncrValue = dif/(pEnv->m_nDuration);     // calculate incriment
		pChannel->VolCalc.RawValue = SF2FLOAT(pEnv->m_StartFactor);

 		dbVolume = SF2FLOAT(pElement->m_Volume.m_MaxValue);
		pSound->Vol = CFXVolumeTable[abs(dbVolume)] * pChannel->VolCalc.RawValue;

		// pEnv->m_MaxValue now hlods the MAX converted volume, its in N64
		// units NOT decibels
		pEnv->m_MaxValue = FLOAT2SF(CFXVolumeTable[abs(dbVolume)]);
		pSound->MaxVol = SF2FLOAT(pEnv->m_MaxValue);


#if 0
		rmonPrintf("Env, Max: %2.2f\n",  SF2FLOAT(pEnv->m_MaxValue));
		rmonPrintf("Env, Start: %2.2f\n", SF2FLOAT(pEnv->m_StartFactor));
		rmonPrintf("Env, End: %2.2f\n", SF2FLOAT(pEnv->m_EndFactor));
		rmonPrintf("Env, Dur: %d\n", pEnv->m_nDuration	);
		rmonPrintf("Env, Time: %d\n", pEnv->m_EnvStartTime	 );
		rmonPrintf("Incr: %2.5f\n", pChannel->VolCalc.IncrValue);
		rmonPrintf("Raw: %2.5f\n", pChannel->VolCalc.RawValue);
#endif

	}
	else
	{


		dbVolume = SF2FLOAT(pElement->m_Volume.m_MaxValue);
		pSound->Vol = CFXVolumeTable[abs(dbVolume)];  // find new volume in table
		pSound->MaxVol = pSound->Vol;
	}



}

/*****************************************************************************
*       void UpdateCFXPanning(INT32 channel)
******************************************************************************
*       Description:    This will set the volume of a CFX sound. CFX volumes are
*                       specified in decibles. 0 means maximum volume -90 means
*                       no sound. A lookup table must be used to convert
*                       to the volume scale the N64 uses
*       Input:
*       Output:
*
*****************************************************************************/
void UpdateCFXPanning(INT32 channel)
{
	ALSndPlayer       *Sndp = &AW.SndPlayerList.Sndp;
	t_SndObject			*pSound = &AW.VoiceChannels[channel].theSFX;
	t_Channel			*pChannel = &AW.VoiceChannels[channel];

	if(pChannel->ChannelFlags & UPDATE_NONENV)
	{
		SetChannel(channel);
		alSndpSetPan(Sndp, pSound->Pan);
	}

}

/*****************************************************************************
*       void UpdateCFXVolume(INT32 channel)
******************************************************************************
*       Description:    This will set the volume of a CFX sound. CFX volumes are
*                       specified in decibles. 0 means maximum volume -90 means
*                       no sound. A lookup table must be used to convert
*                       to the volume scale the N64 uses
*       Input:
*       Output:
*
*****************************************************************************/
void UpdateCFXVolume(INT32 channel)
{
	ALSndPlayer       *Sndp = &AW.SndPlayerList.Sndp;
	t_SndObject			*pSound = &AW.VoiceChannels[channel].theSFX;
	t_Channel			*pChannel = &AW.VoiceChannels[channel];
	//t_SoundPlayer		*sndplayer;
	int					ramp_up;
	CROMEnvelope		*pEnv = &AW.VoiceChannels[channel].VolEnv ;
   //int               dbVolume;


   if ( pChannel->ChannelFlags & VOL_ENV_ENABLED )
	{
		if(pChannel->VolEnv.m_EnvStartTime > 0)
		{
			pChannel->VolEnv.m_EnvStartTime--;
		}
		else
		{
			if(pChannel->VolCalc.IncrValue < 0)
			{
				ramp_up = 1;
			}
			else
			{
				ramp_up = 0;

			}

			if( (ramp_up && (pChannel->VolCalc.RawValue < SF2FLOAT(pEnv->m_EndFactor))) ||
					(!ramp_up && (pChannel->VolCalc.RawValue > SF2FLOAT(pEnv->m_EndFactor))))
			{
				pChannel->VolCalc.RawValue -= pChannel->VolCalc.IncrValue ;

				if(ramp_up && (pChannel->VolCalc.RawValue > SF2FLOAT(pEnv->m_EndFactor)))
				{
					pChannel->VolCalc.RawValue  = SF2FLOAT(pEnv->m_EndFactor);
				}
				else if(!ramp_up && (pChannel->VolCalc.RawValue < SF2FLOAT(pEnv->m_EndFactor)))
				{
					pChannel->VolCalc.RawValue  = SF2FLOAT(pEnv->m_EndFactor);
					if(pChannel->VolCalc.RawValue == 0.0)
					{
						StopChannel(channel);
					}

				}

				pSound->Vol	= SF2FLOAT(pChannel->VolEnv.m_MaxValue) * pChannel->VolCalc.RawValue;
				//pSound->MaxVol = pSound->Vol;   // 10/2
				SetChannel(channel);
				alSndpSetVol(Sndp, pSound->Vol*__GlobalSFXscalar);
			}
		}
	}
	else
	{
		if(pChannel->ChannelFlags & UPDATE_NONENV)
		{
			SetChannel(channel);
			alSndpSetVol(Sndp, pSound->Vol*__GlobalSFXscalar);
		}
	}

}

/*****************************************************************************
*       void UpdateCFXPitch(INT32 channel)
******************************************************************************
*       Description:    This will upadte the pitch of a CFX sound.
*       Input:
*       Output:
*
*****************************************************************************/

void UpdateCFXPitch(INT32 channel)
{
	ALSndPlayer       *Sndp = &AW.SndPlayerList.Sndp;
	t_SndObject			*pSound = &AW.VoiceChannels[channel].theSFX;
	t_Channel			*pChannel = &AW.VoiceChannels[channel];
//	t_SoundPlayer		*sndplayer;
	float					PitchCents;
	CROMEnvelope		*pEnv = &AW.VoiceChannels[channel].PitchEnv ;
	int					ramp_up;


   if (( pChannel->ChannelFlags & PITCH_ENV_ENABLED ) && (pChannel->ChannelFlags & CH_FLAG_STARTED))
	{

		if(pChannel->PitchEnv.m_EnvStartTime > 0)
		{
			pChannel->PitchEnv.m_EnvStartTime--;
		}
		else
		{
//			sndplayer = &AW.SndPlayerList;

			if(pChannel->PitchCalc.IncrValue < 0)
				ramp_up = 1;
			else
				ramp_up = 0;

			//if(pChannel->PitchCalc.RawValue != pEnv->m_EndFactor)
			if( (ramp_up && (pChannel->PitchCalc.RawValue < SF2FLOAT(pEnv->m_EndFactor))) ||
						(!ramp_up && (pChannel->PitchCalc.RawValue > SF2FLOAT(pEnv->m_EndFactor))))
			{
				pChannel->PitchCalc.RawValue -= pChannel->PitchCalc.IncrValue ;

				if(ramp_up && (pChannel->PitchCalc.RawValue > SF2FLOAT(pEnv->m_EndFactor)))
					pChannel->PitchCalc.RawValue  = SF2FLOAT(pEnv->m_EndFactor);
				else if(!ramp_up && (pChannel->PitchCalc.RawValue < SF2FLOAT(pEnv->m_EndFactor)))
					pChannel->PitchCalc.RawValue  = SF2FLOAT(pEnv->m_EndFactor);

				PitchCents = pChannel->PitchCalc.RawValue;
				pSound->Pitch = ConvertPitch(PitchCents);
				//osSyncPrintf("PitchCents: %2.4f\n", PitchCents);
				//osSyncPrintf("N64Pitch: %2.4f\n", pSound->Pitch );
				//osSyncPrintf("Pitch: (%2.2f, %2.2f)\n", PitchCents, pSound->Pitch );
				SetChannel(channel);
				alSndpSetPitch(Sndp, pSound->Pitch);
			}

		}
	}

}



/*****************************************************************************
*	Function Title:  float ConvertPitch(INT16 PitchCents)
******************************************************************************
*	Description:
*	Inputs:
*	Outputs:
*
*****************************************************************************/
float ConvertPitch(float PitchCents)
{
	float					u, Value1, Value2, TableIndex2;
	INT16					TableIndex1, TableIndex3;


	TableIndex2 = (PitchCents+6000)/60;
	TableIndex1 = (INT16)(TableIndex2);

	if((float)(TableIndex1) != TableIndex2)
	{

		TableIndex3 = TableIndex1+1;
		Value1 = CFXPitchTable[TableIndex1];
		Value2 = CFXPitchTable[TableIndex3];
		u = TableIndex2-TableIndex1;
		//osSyncPrintf("index1: %d, index2: %2.2f\n", TableIndex1, TableIndex2);
		//osSyncPrintf("Value1: %2.2f, Value2: %2.2f\n", Value1, Value2);
		//osSyncPrintf("blend: (%2.2f, %2.2f, %2.2f) %2.2f\n", u, Value1, Value2, BlendFLOAT(u, Value1, Value2));
		return  BlendFLOAT(u , Value1, Value2);
	}
	else
	{
		return CFXPitchTable[TableIndex1];
	}

}


/*****************************************************************************
*	Function Title:  void UpdateCFXsounds();
******************************************************************************
*	Description:
*	Inputs:
*	Outputs:
*
*****************************************************************************/
void UpdateCFXChannel(t_SoundPlayer *sndplayer, INT16 Channel)
{
//	ALSndPlayer		*Sndp;
	int				status;
	t_Channel		*pChannel = &AW.VoiceChannels[Channel];


	// find out if Channel[i] is playing a CFX sound
//	Sndp = &sndplayer->Sndp;
   ASSERT(sndplayer->idlist[Channel] != -1);
   ASSERT(sndplayer->idlist[Channel] < MAX_VOICES);
	status = SetChannel(Channel);


	if ((status == AL_STOPPED) && (pChannel->delay > 0))
	{
		AW.VoiceChannels[Channel].delay--;
		pChannel->ChannelFlags |= CH_FLAG_DELAYED_START;
	}

	else if((pChannel->ChannelFlags & CH_FLAG_DELAYED_START) &&
							!(pChannel->ChannelFlags & CH_FLAG_STARTED))

	{
		StartChannel(Channel);
	}
	else if(status == AL_STOPPED)
	{
		DeallocateChannel(Channel);
	}
	else
   {

		if(pChannel->ChannelFlags & UPDATE_NONENV)
		{
			SetSoundPosition(Channel, pChannel->theSFX.vPos.x, pChannel->theSFX.vPos.y, pChannel->theSFX.vPos.z, UPDATE_POS);
		}

		// if playing update volume, pitch, and panning
		UpdateCFXVolume(Channel);
		UpdateCFXPanning(Channel);
		UpdateCFXPitch(Channel);
   }
}




/*****************************************************************************
*   Function Title: void DoAmbientSounds()
******************************************************************************
*   Description:
*   Inputs:
*	Outputs:
*
*****************************************************************************/
void DoAmbientSounds()
{
	t_AmbientSounds		*pSoundList = AmbientSoundTable;
	int						s, Prob ;
	static int				ambient_timer = 0;
	int						rand_type;

	if (!cache_is_valid)
		return;

	if((ambient_timer % AMBIENT_SOUND_RATE) != 0)
	{
		ambient_timer++;
		return;
	}

	ambient_timer++;

	if (		(CTurokMovement.WaterFlag == PLAYER_BELOW_WATERSURFACE)
			&&	(!CTurokMovement.InAntiGrav) )
	{
		return;
	}

	// we dont want to add to confusion of battle
	if(CTMove__IsInCombat(&CTurokMovement))
	{
		return;
	}

	// get region set ambient type
	rand_type = AMB_RANDOMIZE_FAR;

	switch(CTurokMovement.AmbientSound)
	{
		case	REG_AMBIENTJUNGLE:
			AmbientSoundRegion = AMB_RGN_JUNGLE;
			break;

		case	REG_AMBIENTWATER:
			AmbientSoundRegion = AMB_RGN_WATER;
			break;

		case	REG_AMBIENTCAVES:
			AmbientSoundRegion = AMB_RGN_CAVES;
			break;

		case	REG_AMBIENTCATACOMBS:
			AmbientSoundRegion = AMB_RGN_CATACOMBS;
			break;

		case	REG_AMBIENTLOSTLANDS:
			AmbientSoundRegion = AMB_RGN_LOSTLANDS;
			break;

		case	REG_AMBIENTCAMPAIGNERSLAIR:
			AmbientSoundRegion = AMB_RGN_CAMPAINER;
			break;

		case	REG_AMBIENTCATACOMBLARGE:
			AmbientSoundRegion = AMB_RGN_CATACOMBS_LARGE;
			break;

		case REG_AMBIENTCITY:
			AmbientSoundRegion = AMB_RGN_CITY;
			break;

		case REG_AMBIENTRUINS:
			AmbientSoundRegion = AMB_RGN_RUINS;
			break;

		case REG_AMBIENTTREETOP:
			AmbientSoundRegion = AMB_RGN_TREETOP;
			break;

		case REG_AMBIENTWARP:
			AmbientSoundRegion = AMB_RGN_WARP;
			break;

		default:
		case	REG_AMBIENTNONE:
			AmbientSoundRegion = -1;
			break;
	}

	if(AmbientSoundRegion == AMB_RGN_CATACOMBS_LARGE)
		AmbientSoundRegion = AMB_RGN_CATACOMBS;	// same sounds as small catacombs

	if(AmbientSoundRegion >= 0)
	{
		pSoundList = &AmbientSoundTable[AmbientSoundRegion];

		if(pSoundList->TotalNum == 0)
			return;

		// Make intro have bigger probabilities
		Prob = pSoundList->Probability ;
		if ((GetApp()->m_Mode == MODE_INTRO) || (GetApp()->m_Mode == MODE_TITLE))
			Prob *= 2 ;

		if(CheckProbability(Prob) == TRUE)
		{
			s = SOUND_RANDOM(pSoundList->TotalNum);

			if(pSoundList->SoundNum[s] >= 0)
				CScene__DoSoundEffect(&GetApp()->m_Scene, pSoundList->SoundNum[s], 0, NULL, &AW.Ears.vPos, rand_type);
		}
	}

	//audio_ambient_flags = 0;	// clear flag
}

/*****************************************************************************
*   Function Title: void DoSoundScenes()
******************************************************************************
*   Description:
*   Inputs:
*	 Outputs:
*
*****************************************************************************/
void DoSoundScenes()
{
	static	int	SoundCount = 0;
	static	int	CurtainUp = 0;
	static	int	CurrentSound = 0;
	static	int	CurrentDelay = 0;
	static INT16	PrevSound, PrevPrevSound;
	static INT16	*pSoundScene = SoundScenes;
	static	int	SoundSceneCounter = 0;
	//int				dwFlags;


	if (!cache_is_valid)
		return;

	if((AW.sfx_timer%(15)) != 0)
		return;

	SoundSceneCounter++;

//	if (!audio_ambient_flags)
//		return ;

	//dwFlags = audio_ambient_flags;

	if (		(CTurokMovement.AmbientSound == REG_AMBIENTCAVES)
			||	(		(CTurokMovement.WaterFlag == PLAYER_BELOW_WATERSURFACE)
					&&	(!CTurokMovement.InAntiGrav) )
			||	(CTMove__IsInCombat(&CTurokMovement)) )
	{
		CurtainUp = 0;
		return;
	}

/*	if((dwFlags & REGFLAG_AMBIENTCAVES) || (CTurokMovement.WaterFlag == PLAYER_BELOW_WATERSURFACE) || (CTMove__IsInCombat(&CTurokMovement)))
 	{
		CurtainUp = 0;
		return;
	}
*/

	if((CheckProbability(10) == TRUE)&& (SoundSceneCounter%20 == 0) && !CurtainUp)
	//if(((AW.sfx_timer%(60*6)) == 0) && !CurtainUp)
	{
		//rmonPrintf("SoundScene started...\n");
		pSoundScene = SoundScenes + (8*SOUND_RANDOM(MAX_SOUND_SCENES));
		CurtainUp = 1;
		CurrentDelay = 0;
		SoundCount = 0;
		PrevSound = 0;
		PrevPrevSound = 0;
	}

	if(CurtainUp)
	{
		if(CurrentDelay <= 0)
		{
			PrevSound = CurrentSound ;
			PrevPrevSound = PrevSound ;

			if(SoundCount == 6)
				CurrentSound = pSoundScene[7];
			else
				CurrentSound = pSoundScene[SOUND_RANDOM(6)];

			// three in a row sounds bad
			if((PrevPrevSound == PrevSound) && (PrevPrevSound == CurrentSound) && (SoundCount >= 3))
				CurrentSound = pSoundScene[SOUND_RANDOM(6)];

			CScene__DoSoundEffect(&GetApp()->m_Scene, CurrentSound, 0, NULL, &AW.Ears.vPos, AMB_SCENE_POSITION);
			SoundCount++;
			//CurrentDelay = RANDOM(90);
			CurrentDelay = SOUND_RANDOM(6);
		}
		else
			CurrentDelay--;

		if((SoundCount == 7) && (CurrentDelay == 0))
		{
			CurtainUp = 0;
			//rmonPrintf("SoundScene done...\n");
		}
	}
}


/*****************************************************************************
*   Function Title: void InitCFXVars()
******************************************************************************
*   Description:    This will initialize tables and things
*   Inputs:
*	Outputs:
*
*****************************************************************************/
void InitCFXVars()
{
	INT32       i;
   float       temp1, temp2;

   // fill in Volume lookup table
   for ( i = 0; i < MAX_VOLUMETABLE_ENTRIES; i++ )
   {
		temp1 = i/(float)(MAX_VOLUMETABLE_ENTRIES-1);
		temp2 = pow(temp1, 5);
		CFXVolumeTable[MAX_VOLUMETABLE_ENTRIES-i-1] = temp2*MAX_WORLD_VOL;
	}

}


