// Cinema camera control file by Stephen Broumley

#include "cppu64.h"

#ifndef MAKE_CART
#include <PR/ramrom.h>	// needed for argument passing into the app
#endif

#include "tengine.h"
#include "dlists.h"
#include "scaling.h"
#include "particle.h"
#include "mattable.h"
#include "tcontrol.h"
#include "tmove.h"
#include "audio.h"
#include "sfx.h"
#include "textload.h"
#include "regtype.h"


#include "cinecam.h"
#include "cammodes.h"
#include "camera.h"
#include "mantis.h"
#include "trex.h"
#include "campaign.h"
#include "collinfo.h"
#include "bossflgs.h"

#ifdef ENGLISH
// ---------------------- English text ---------------------------
static char	text_levelkey[] = {"level 0 key"};
#endif

#ifdef GERMAN
// ---------------------- German text ---------------------------
static char	text_levelkey[] = {"schl/ssel f/r level 0"};		//u.u.
#endif


#ifdef KANJI
// ---------------------- Japanese text ---------------------------
static char	text_levelkey[] = {0x2A, 0x41, 0x13, 0x41, 0xEB, 0x82, 0x83, 0x93, -1} ;
#endif


int				CGameObjectInstance__GetAnimType(CGameObjectInstance *pThis, int nAnimIndex);




float CGameObjectInstance__AnimSpeedScaler(CGameObjectInstance *pThis)
{
	int nAnim = CGameObjectInstance__GetAnimType(pThis, pThis->m_asCurrent.m_nAnim) ;
	int nObject = CGameObjectInstance__TypeFlag(pThis) ;

	// Special case object anims
	switch(nObject)
	{
		// Cinema arrow
		case AI_OBJECT_CINEMA_TUROK_ARROW:
			return 0.5 ;

		// Turok
		case AI_OBJECT_CHARACTER_PLAYER:

			switch(nAnim)
			{
				case AI_ANIM_EXTRA1:							// Bow draw
				case AI_ANIM_DEATH_SWIM_SINK:				// Drown
				case AI_ANIM_EXTRA10:						// Holdkey
				case AI_ANIM_IDLE:							// Victory
				case AI_ANIM_INTERACTIVE_ANIMATION3:	// Getting killed by campaigner
				case AI_ANIM_TELEPORT_APPEAR:				// Resurrection
				case AI_ANIM_INTERACTIVE_ANIMATION2:	// Getting killed by the trex
				case AI_ANIM_EXTRA16:						// Watching explosions
				case AI_ANIM_EXTRA17:						// Idle
				case AI_ANIM_EXTRA18:						// Leaping
				case AI_ANIM_EXTRA19:						// Running
				case AI_ANIM_EXTRA20:						// Turok in water
				case AI_ANIM_DEATH_NORMAL:					// Normal death
				case AI_ANIM_DEATH_HIGH_FALL:				// Fall death
				case AI_ANIM_INTERACTIVE_ANIMATION1:	// Getting killed by the mantis
					return 0.5 ;
			}
			break ;


		// Mantis
		case AI_OBJECT_CHARACTER_MANTIS_BOSS:
			switch(nAnim)
			{
				case AI_ANIM_BOSS_MANTIS_BREAK_FLOOR:
				case AI_ANIM_BOSS_MANTIS_DEATH_FLOOR:
					return 0.5 ;
			}
			break ;

		// TRex
		case AI_OBJECT_CHARACTER_TREX_BOSS:

			switch(nAnim)
			{
				case AI_ANIM_DEATH_NORMAL:
					return 0.5 ;
			}
			break ;

	}

	// Special case levels
	switch(GetApp()->m_CurrentWarpID)
	{
		case INTRO_IGGY_WARP_ID:
			// Iggy?
			if ((nObject == AI_OBJECT_DEVICE_ACTION) && (AI_GetEA(pThis)->m_Id == 999))
				return 0.5 ;

			// Turok's arrow?
			if (nObject == AI_OBJECT_CINEMA_TUROK_ARROW)
				return 0.5 ;
	}

	// Default
	return 1.0 ;
}




BOOL CCamera__TryMoveInstanceToCollFreePos(CCamera *pThis, CGameObjectInstance *pInst, FLOAT Radius)
{
	CROMRegionSet			*pInstRegionSet = CScene__GetRegionAttributes(&GetApp()->m_Scene, pInst->ah.ih.m_pCurrentRegion) ;
	CGameRegion				*pRegion ;

	CVector3					vPos, vOutPos ;

	CVector3					vAveragePos ;

	INT32						Total ;

	INT32						i ;
	FLOAT						Rot ;

	// Clear final pos
	vAveragePos.x = 0 ;
	vAveragePos.y = 0 ;
	vAveragePos.z = 0 ;
	Total = 0 ;

	// Setup position
	Rot = 0 ;
	i = 360/32 ;
	while(i--)
	{
		// Calculate check position
		vPos.x = (Radius * sin(Rot)) +  pInst->ah.ih.m_vPos.x ;
		vPos.y = pInst->ah.ih.m_vPos.y ;
		vPos.z = (Radius * cos(Rot)) +  pInst->ah.ih.m_vPos.z ;

		// Get region/pos info
		CInstanceHdr__GetNearPositionAndRegion(&pInst->ah.ih,
															vPos,
															&vOutPos,
															&pRegion,
															INTERSECT_BEHAVIOR_IGNORE, INTERSECT_BEHAVIOR_STICK) ;

		// Position valid?
		if ((!CAnimInstanceHdr__IsObstructed(&pInst->ah, vPos, NULL)) &&
			 (vOutPos.x == vPos.x) && (vOutPos.z == vPos.z))
		{
			// Region set valid?
			if (pInstRegionSet == CScene__GetRegionAttributes(&GetApp()->m_Scene, pRegion))
			{
				Total++ ;
				vAveragePos.x += vPos.x ;
				vAveragePos.z += vPos.z ;
			}
		}

		// Try next angle
		Rot += ANGLE_DTOR(360/32) ;
	}

	// Did we find a position - if so move instance
	if (Total)
	{
		vAveragePos.x /= Total ;
		vAveragePos.y /= Total ;
		vAveragePos.z /= Total ;

		CInstanceHdr__GetNearPositionAndRegion(&pInst->ah.ih,
															vAveragePos,
															&vOutPos,
															&pRegion,
															INTERSECT_BEHAVIOR_IGNORE, INTERSECT_BEHAVIOR_IGNORE) ;

		pInst->ah.ih.m_vPos = vOutPos ;
		pInst->ah.ih.m_pCurrentRegion = pRegion ;

		return TRUE ;
	}
	else
	{
		return FALSE ;
	}
}



void CCamera__MoveTurokToCollFreePos(CCamera *pThis, FLOAT StartRadius, FLOAT DeltaRadius, INT32 Checks)
{
	CGameObjectInstance *pTurok = CEngineApp__GetPlayer(GetApp()) ;

	// Keep going until turok is moved!
	while((Checks--) && (!CCamera__TryMoveInstanceToCollFreePos(pThis, pTurok, StartRadius)))
		StartRadius += DeltaRadius ;
}




/////////////////////////////////////////////////////////////////////////////
//	Modes:	  	 CAMERA_CINEMA_TUROK_DEATH_MODE
//	Description: Turok being killed by an enemy
/////////////////////////////////////////////////////////////////////////////
CCameraTrackInfo DeathRadius10Track =
{
	ANGLE_DTOR(180+90),	ANGLE_DTOR(75),		// YRotOffset,		FinalYRotOffset ;
	10*SCALING_FACTOR,  	9*SCALING_FACTOR, 	//	Radius,			FinalRadius ;
	20*SCALING_FACTOR,	5*SCALING_FACTOR,  	// EyeYOffset,		FinalEyeYOffset ;
	0*SCALING_FACTOR,		0*SCALING_FACTOR,  	// FocusYOffset,	FinalFocusYOffset ;
} ;

CCameraTrackInfo DeathRadius6Track =
{
	ANGLE_DTOR(180+90), 	ANGLE_DTOR(75),		// YRotOffset,		FinalYRotOffset ;
	6*SCALING_FACTOR,  	5*SCALING_FACTOR, 	//	Radius,			FinalRadius ;
	20*SCALING_FACTOR,	6*SCALING_FACTOR,  	// EyeYOffset,		FinalEyeYOffset ;
	0*SCALING_FACTOR,		0*SCALING_FACTOR,  	// LookYOffset,	FinalLookYOffset ;
} ;

CCameraTrackInfo DeathRadius2Track =
{
	ANGLE_DTOR(180+90),	ANGLE_DTOR(90), 		// YRotOffset,		FinalYRotOffset ;
	2*SCALING_FACTOR,  	1*SCALING_FACTOR, 	//	Radius,			FinalRadius ;
	20*SCALING_FACTOR,	8*SCALING_FACTOR,  	// EyeYOffset,		FinalEyeYOffset ;
	0*SCALING_FACTOR,		0*SCALING_FACTOR,  	// LookYOffset,	FinalLookYOffset ;
} ;

CCameraTrackInfo DeathRadius0Track =
{
	ANGLE_DTOR(180), 		ANGLE_DTOR(0),			// YRotOffset,		FinalYRotOffset ;
	0*SCALING_FACTOR,  	0*SCALING_FACTOR, 	//	Radius,			FinalRadius ;
	20*SCALING_FACTOR,	10*SCALING_FACTOR,  	// EyeYOffset,		FinalEyeYOffset ;
	0*SCALING_FACTOR,		0*SCALING_FACTOR,  	// LookYOffset,	FinalLookYOffset ;
} ;




#define DEATH_TIME	SECONDS_TO_FRAMES(6)

void CCamera__Setup_CINEMATIC_TUROK_DEATH(CCamera *pThis)
{
	CGameObjectInstance *pTurok = CEngineApp__GetPlayer(GetApp()) ;

	pThis->m_Mode = CAMERA_CINEMA_TUROK_DEATH_MODE ;
	pThis->m_Stage = 0 ;
	pThis->m_StageTime = 0 ;

	// Setup camera radius
	if (CGameObjectInstance__CylinderCollisionFree(pTurok, 12*SCALING_FACTOR))
		pThis->m_pTrackInfo = &DeathRadius10Track ;
	else
	if (CGameObjectInstance__CylinderCollisionFree(pTurok, 8*SCALING_FACTOR))
		pThis->m_pTrackInfo = &DeathRadius6Track ;
	else
	if (CGameObjectInstance__CylinderCollisionFree(pTurok, 4*SCALING_FACTOR))
		pThis->m_pTrackInfo = &DeathRadius2Track ;
	else
	{
		pThis->m_pTrackInfo = &DeathRadius0Track ;
		pThis->m_View.m_vRotation.z = pTurok->m_RotY + ANGLE_DTOR(90) ;
	}

	// Put into cinema mode
	pThis->m_CinemaMode = TRUE ;
	pThis->m_LetterBoxScale = LETTERBOX_SCALE ;
}

void CCamera__Code_CINEMATIC_TUROK_DEATH(CCamera *pThis)
{
	CGameObjectInstance *pTurok = CEngineApp__GetPlayer(GetApp()) ;

	// Swap objects
	CScene__LoadObjectModelType(&GetApp()->m_Scene, pTurok,
									    AI_OBJECT_CHARACTER_PLAYER, AI_ANIM_DEATH_NORMAL) ;


	AI_SetDesiredAnim(pTurok, AI_ANIM_DEATH_NORMAL, TRUE) ;

	// Zoom position
	CCameraView__TweenTrackAroundTurok(&pThis->m_View, pThis->m_pTrackInfo,
												  INVERSE_SQR_TWEEN(pThis->m_StageTime / DEATH_TIME)) ;

	// Check for ending
	if (!pThis->m_CinemaFadingOut)
	{
		// Finished?
		pThis->m_StageTime += frame_increment ;
		if (pThis->m_StageTime > DEATH_TIME)
			pThis->m_StageTime = DEATH_TIME ;

		if ((pThis->m_StartPressed) || (pThis->m_StageTime >= DEATH_TIME))
		{
			pThis->m_CinemaFadingOut = TRUE ;
			CEngineApp__DoDeath(GetApp()) ;
		}
	}
}


/////////////////////////////////////////////////////////////////////////////
//	Modes:	  	 CAMERA_CINEMA_TUROK_FALL_DEATH_MODE
//	Description: Turok falling off a cliff to a nice death
/////////////////////////////////////////////////////////////////////////////
void CCamera__Setup_CINEMATIC_TUROK_FALL_DEATH(CCamera *pThis)
{
	CGameObjectInstance *pTurok = CEngineApp__GetPlayer(GetApp()) ;

	pThis->m_Mode = CAMERA_CINEMA_TUROK_FALL_DEATH_MODE ;
	pThis->m_Stage = 0 ;
	pThis->m_StageTime = 0 ;

	// Setup all player gravities
	ci_player.GravityAcceleration = GRAVITY_ACCELERATION/8 ;
	ci_player.GroundBehavior = INTERSECT_BEHAVIOR_IGNORE;
	ci_player.dwFlags &= ~COLLISIONFLAG_TRACKGROUND;

	pTurok->ah.m_vVelocity.y = -6*SCALING_FACTOR*15 ;

	// Move turok up in the world
	pTurok->ah.ih.m_vPos.y += SCALING_FACTOR * 25 ;

	// Update focus position
	pThis->m_View.m_vFocus = pTurok->ah.ih.m_vPos ;

	// Setup camera position 1 (directly below turok)
	CCamera__GetCollisionFreePos(pThis, &pTurok->ah.ih.m_vPos, &pThis->m_vPos1, 35 * SCALING_FACTOR) ;

	// Move camera below turok
	pThis->m_vPos1.y -= 20*SCALING_FACTOR ;

	// Setup position 2 (to side of turok)
	CCamera__GetCollisionFreePos(pThis, &pThis->m_vPos1, &pThis->m_vPos2, -15 * SCALING_FACTOR) ;

	// Put camera eye to position 1
	pThis->m_View.m_vEye = pThis->m_vPos1 ;

	// Force turok into the open!
	CCamera__SetTurokPosition(pThis, pThis->m_vPos1) ;

	// Put into cinema mode
	pThis->m_CinemaMode = TRUE ;
	pThis->m_LetterBoxScale = LETTERBOX_SCALE ;
	pThis->m_Flags = 0 ;

	// Do falling sfx
	AI_SEvent_GeneralSound(&pTurok->ah.ih, pTurok->ah.ih.m_vPos, SOUND_GENERIC_25) ;
}


void CCamera__Code_CINEMATIC_TUROK_FALL_DEATH(CCamera *pThis)
{
	CGameObjectInstance *pTurok = CEngineApp__GetPlayer(GetApp()) ;

	// Swap objects
	CScene__LoadObjectModelType(&GetApp()->m_Scene, CEngineApp__GetPlayer(GetApp()),
										    AI_OBJECT_CHARACTER_PLAYER, AI_ANIM_DEATH_HIGH_FALL) ;

	AI_SetDesiredAnim(pTurok, AI_ANIM_DEATH_HIGH_FALL, TRUE) ;

	ci_player.GroundBehavior = INTERSECT_BEHAVIOR_IGNORE;
	ci_player.dwFlags &= ~COLLISIONFLAG_TRACKGROUND;

	pTurok->ah.m_vVelocity.y = -6*SCALING_FACTOR*15 ;

	pTurok->m_RotY += ANGLE_DTOR(8) * frame_increment ;

	// Update focus position
	pThis->m_View.m_vFocus = pTurok->ah.ih.m_vPos ;

	// If Turok is above camera zoom to pos2, else pos1 (directly underneath Turok)
	if (pTurok->ah.ih.m_vPos.y > (pThis->m_View.m_vEye.y - (10*SCALING_FACTOR)))
		CCameraView__ZoomEyeToPos(&pThis->m_View, &pThis->m_vPos2, 1.0/8.0) ;
	else
		CCameraView__ZoomEyeToPos(&pThis->m_View, &pThis->m_vPos1, 1.0/32.0) ;

	// Check for ending
	if (!pThis->m_CinemaFadingOut)
	{
		// Finished?
		pThis->m_StageTime += frame_increment ;
		if ((pThis->m_StartPressed) || (pThis->m_StageTime > SECONDS_TO_FRAMES(4)))
		{
			pThis->m_CinemaFadingOut = TRUE ;
			CEngineApp__DoDeath(GetApp()) ;
		}
	}

	// Make sure turok falls through everything!!!!
	ci_player.GroundBehavior = INTERSECT_BEHAVIOR_IGNORE ;
	ci_playerdead.GroundBehavior = INTERSECT_BEHAVIOR_IGNORE ;
	ci_playerunderwater.GroundBehavior = INTERSECT_BEHAVIOR_IGNORE ;
	ci_playerantigrav.GroundBehavior = INTERSECT_BEHAVIOR_IGNORE ;
	ci_playeronwatersurface.GroundBehavior = INTERSECT_BEHAVIOR_IGNORE ;
	ci_playerfallintowater.GroundBehavior = INTERSECT_BEHAVIOR_IGNORE ;
	ci_player_climbing_sidestep_character.GroundBehavior = INTERSECT_BEHAVIOR_IGNORE ;
}





/////////////////////////////////////////////////////////////////////////////
//	Modes:	  	 CAMERA_CINEMA_TUROK_WATER_DEATH_MODE
//	Description: Turok drowning
/////////////////////////////////////////////////////////////////////////////

#define WATER_DEATH_TIME			SECONDS_TO_FRAMES(6)

CCameraTrackInfo WaterDeathTrack =
{
	ANGLE_DTOR(0),			ANGLE_DTOR(360),		// YRotOffset,		FinalYRotOffset ;
	10*SCALING_FACTOR,  	1*SCALING_FACTOR, 	//	Radius,			FinalRadius ;
	10*SCALING_FACTOR,	15*SCALING_FACTOR,  	// EyeYOffset,		FinalEyeYOffset ;
	0*SCALING_FACTOR,		0*SCALING_FACTOR,  	// FocusYOffset,	FinalFocusYOffset ;
} ;

void CCamera__Setup_CINEMATIC_TUROK_WATER_DEATH(CCamera *pThis)
{
	CVector3	vPos ;
	CGameObjectInstance *pTurok = CEngineApp__GetPlayer(GetApp()) ;

	pThis->m_Mode = CAMERA_CINEMA_TUROK_WATER_DEATH_MODE ;
	pThis->m_Stage = 0 ;
	pThis->m_StageTime = 0 ;
	ci_playerunderwater.GravityAcceleration	= GRAVITY_WATER_ACCELERATION/3;


	// Back off turok from walls
	CCamera__GetCollisionFreePos(pThis, &pTurok->ah.ih.m_vPos, &vPos, 35 * SCALING_FACTOR) ;
	CCamera__SetTurokPosition(pThis, vPos) ;

	// Move into open
//	CCamera__MoveTurokToCollFreePos(pThis, 15*SCALING_FACTOR, -1*SCALING_FACTOR, 14) ;

	// Put into cinema mode
	pThis->m_CinemaMode = TRUE ;
	pThis->m_LetterBoxScale = LETTERBOX_SCALE ;
}



void CCamera__Code_CINEMATIC_TUROK_WATER_DEATH(CCamera *pThis)
{
	CGameObjectInstance *pTurok = CEngineApp__GetPlayer(GetApp()) ;

	// Swap objects
	CScene__LoadObjectModelType(&GetApp()->m_Scene, CEngineApp__GetPlayer(GetApp()),
									    AI_OBJECT_CHARACTER_PLAYER, AI_ANIM_DEATH_SWIM_SINK) ;

	AI_SetDesiredAnim(pTurok, AI_ANIM_DEATH_SWIM_SINK, TRUE) ;

	// Next position
	pThis->m_Time += frame_increment ;

	pThis->m_StageTime += frame_increment ;
	if (pThis->m_StageTime > WATER_DEATH_TIME)
		pThis->m_StageTime = WATER_DEATH_TIME ;

	// Zoom position
	CCameraView__TweenTrackAroundTurok(&pThis->m_View, &WaterDeathTrack,
												  INVERSE_SQR_TWEEN(pThis->m_StageTime / WATER_DEATH_TIME)) ;

	// Check for ending
	if ((!pThis->m_CinemaFadingOut) &&
		 ((pThis->m_StartPressed) || (pThis->m_Time >= WATER_DEATH_TIME)))
	{
		pThis->m_CinemaFadingOut = TRUE ;
		CEngineApp__DoDeath(GetApp());
	}
}



/////////////////////////////////////////////////////////////////////////////
//	Modes:	  	 CAMERA_CINEMA_TUROK_RESURRECT_MODE
//	Description: Turok comming back to life
/////////////////////////////////////////////////////////////////////////////
void CCamera__Setup_CINEMATIC_TUROK_RESURRECT(CCamera *pThis)
{
	CGameObjectInstance	*pTurok = CEngineApp__GetPlayer(GetApp()) ;
	CEngineApp				*pEngine = GetApp() ;
	CROMWarpPoint			*pWarpPoint ;

	pTurok->m_asCurrent.m_CycleCompleted = FALSE ;

	pThis->m_Mode = CAMERA_CINEMA_TUROK_RESURRECT_MODE ;
	pThis->m_Stage = 0 ;
	pThis->m_StageTime = 0 ;

	// Put into cinema mode
	pThis->m_CinemaMode = TRUE ;
	pThis->m_LetterBoxScale = LETTERBOX_SCALE ;

	// Put turok back to last start position if in a boss level
	if ((pEngine->m_BossLevel) && (pEngine->m_Scene.m_WarpFound))
	{
		// Set turok back to level start warp
		pWarpPoint = &pEngine->m_Scene.m_WarpPoint ;
		pTurok->ah.ih.m_vPos = pWarpPoint->m_vPos ;
		pTurok->ah.ih.m_pCurrentRegion = CScene__GetRegionPtr(&pEngine->m_Scene, ORDERBYTES(pWarpPoint->m_nRegion)) ;
		pTurok->m_RotY = pWarpPoint->m_RotY ;
	}
}



CCameraTrackInfo ResurrectStage0Track =
{
	ANGLE_DTOR(0),			ANGLE_DTOR(360*2),	// YRotOffset,		FinalYRotOffset ;
	12*SCALING_FACTOR,  	9.5*SCALING_FACTOR, 	//	Radius,			FinalRadius ;
	0.5*SCALING_FACTOR,	11*SCALING_FACTOR,  	// EyeYOffset,		FinalEyeYOffset ;
	0.5*SCALING_FACTOR,	10*SCALING_FACTOR,  	// FocusYOffset,	FinalFocusYOffset ;
} ;

CCameraTrackInfo ResurrectStage1Track =
{
	ANGLE_DTOR(360*2),	ANGLE_DTOR(360*2),	// YRotOffset,		FinalYRotOffset ;
	9.5*SCALING_FACTOR, 	11.0*SCALING_FACTOR, //	Radius,			FinalRadius ;
	11*SCALING_FACTOR,	5.5*SCALING_FACTOR, 	// EyeYOffset,	  	FinalEyeYOffset ;
	10*SCALING_FACTOR,	4.5*SCALING_FACTOR, 	// FocusYOffset,	FinalFocusYOffset ;
} ;



#define	RESURRECT_STAGE0_TIME		SECONDS_TO_FRAMES(8)
#define	RESURRECT_STAGE1_TIME		SECONDS_TO_FRAMES(2)
#define	RESURRECT_TOTAL_TIME			SECONDS_TO_FRAMES(11)

void CCamera__Code_CINEMATIC_TUROK_RESURRECT(CCamera *pThis)
{
	CGameObjectInstance *pTurok = CEngineApp__GetPlayer(GetApp()) ;


	// Swap objects
	CScene__LoadObjectModelType(&GetApp()->m_Scene, pTurok,
									    AI_OBJECT_CHARACTER_PLAYER, AI_ANIM_TELEPORT_APPEAR) ;

	AI_SetDesiredAnim(pTurok, AI_ANIM_TELEPORT_APPEAR, TRUE) ;

	// Move the camera
	switch(pThis->m_Stage)
	{
		// Zoom up to head
		case 0:
			pThis->m_StageTime += frame_increment ;

			CCameraView__TweenTrackAroundTurok(&pThis->m_View, &ResurrectStage0Track,
														  INVERSE_SQR_TWEEN(pThis->m_StageTime / RESURRECT_STAGE0_TIME)) ;


			if (pThis->m_StageTime > RESURRECT_STAGE0_TIME)
			{
				pThis->m_StageTime = 0 ;
				pThis->m_Stage++ ;
			}

			break ;

		// Zoom to infront of turok
		case 1:
			pThis->m_StageTime += frame_increment ;
			if (pThis->m_StageTime > RESURRECT_STAGE1_TIME)
				pThis->m_StageTime = RESURRECT_STAGE1_TIME ;

			CCameraView__TweenTrackAroundTurok(&pThis->m_View, &ResurrectStage1Track,
														  COSINE_TWEEN(INVERSE_SQR_TWEEN(pThis->m_StageTime / RESURRECT_STAGE1_TIME))) ;
			break ;

	}

	// Total time reached?
	pThis->m_Time += frame_increment ;
	if (pThis->m_Time > RESURRECT_TOTAL_TIME)
		pTurok->m_asCurrent.m_CycleCompleted = TRUE ;


	// Check for ending
	if (!pThis->m_CinemaFadingOut)
	{
		// Exit, done?
		if ((pThis->m_StartPressed) || (pTurok->m_asCurrent.m_CycleCompleted))
		{
			CCamera__KillSoundHandle(pThis, 0) ;	// Kill wind sound

			pThis->m_CinemaFadingOut = TRUE ;
			CEngineApp__SetupFadeTo(GetApp(), MODE_RESETLEVEL) ;
		}
	}
}



/////////////////////////////////////////////////////////////////////////////
//	Modes:	  	 CAMERA_CINEMA_LONGHUNTER_MODE
//	Description: Watching longhunter and jeep enter arena
/////////////////////////////////////////////////////////////////////////////
void CCamera__Setup_CINEMATIC_LONGHUNTER(CCamera *pThis)
{
	CGameObjectInstance *pTurok = CEngineApp__GetPlayer(GetApp()) ;

	pThis->m_Mode = CAMERA_CINEMA_LONGHUNTER_MODE ;

	// Put into cinema mode
	pThis->m_CinemaMode = TRUE ;
	pThis->m_LetterBoxScale = LETTERBOX_SCALE ;

	// Setup camera position
	pThis->m_View.m_vEye.x = pTurok->ah.ih.m_vPos.x ;
	pThis->m_View.m_vEye.y = pTurok->ah.ih.m_vPos.y + (4*SCALING_FACTOR) ;
	pThis->m_View.m_vEye.z = pTurok->ah.ih.m_vPos.z ;

	pThis->m_View.m_vFocus.x = pThis->m_pObject->ah.ih.m_vPos.x ;
	pThis->m_View.m_vFocus.y = pThis->m_pObject->ah.ih.m_vPos.y + (4*SCALING_FACTOR) ;
	pThis->m_View.m_vFocus.z = pThis->m_pObject->ah.ih.m_vPos.z ;
}

void CCamera__Code_CINEMATIC_LONGHUNTER(CCamera *pThis)
{
}




/////////////////////////////////////////////////////////////////////////////
//	Modes:	  	 CAMERA_CINEMA_FIRST_HUMVEE_MODE
//	Description: Watching jeep jump in
/////////////////////////////////////////////////////////////////////////////
void CCamera__Setup_CINEMATIC_FIRST_HUMVEE(CCamera *pThis)
{
	CGameObjectInstance *pTurok = CEngineApp__GetPlayer(GetApp()) ;

	pThis->m_Mode = CAMERA_CINEMA_FIRST_HUMVEE_MODE ;

	// Put into cinema mode
	pThis->m_CinemaMode = TRUE ;
	pThis->m_LetterBoxScale = LETTERBOX_SCALE ;

	// Setup camera position
	pThis->m_View.m_vEye.x = pTurok->ah.ih.m_vPos.x ;
	pThis->m_View.m_vEye.y = pTurok->ah.ih.m_vPos.y + (4*SCALING_FACTOR) ;
	pThis->m_View.m_vEye.z = pTurok->ah.ih.m_vPos.z ;
}

void CCamera__Code_CINEMATIC_FIRST_HUMVEE(CCamera *pThis)
{
	// Update focus
	if (pThis->m_pObject)
	{
		pThis->m_View.m_vFocus.x = pThis->m_pObject->ah.ih.m_vPos.x ;
		pThis->m_View.m_vFocus.y = pThis->m_pObject->ah.ih.m_vPos.y + (20*SCALING_FACTOR) ;
		pThis->m_View.m_vFocus.z = pThis->m_pObject->ah.ih.m_vPos.z ;
	}
}


/////////////////////////////////////////////////////////////////////////////
//	Modes:	  	 CAMERA_CINEMA_LONGHUNTER_MODE
//	Description: Watching final jeep enter arena
/////////////////////////////////////////////////////////////////////////////
void CCamera__Setup_CINEMATIC_FINAL_HUMVEE(CCamera *pThis)
{
	CGameObjectInstance *pTurok = CEngineApp__GetPlayer(GetApp()) ;

	pThis->m_Mode = CAMERA_CINEMA_FINAL_HUMVEE_MODE ;

	// Put into cinema mode
	pThis->m_CinemaMode = TRUE ;
	pThis->m_LetterBoxScale = LETTERBOX_SCALE ;


	// Setup camera position
	pThis->m_View.m_FocusYOffset = 20 * SCALING_FACTOR ;
	pThis->m_View.m_vEye.x = pTurok->ah.ih.m_vPos.x ;
	pThis->m_View.m_vEye.y = pTurok->ah.ih.m_vPos.y + (4*SCALING_FACTOR) ;
	pThis->m_View.m_vEye.z = pTurok->ah.ih.m_vPos.z ;
}

void CCamera__Code_CINEMATIC_FINAL_HUMVEE(CCamera *pThis)
{
	// Update focus
	if (pThis->m_pObject)
	{
		pThis->m_View.m_vFocus.x = pThis->m_pObject->ah.ih.m_vPos.x ;
		pThis->m_View.m_vFocus.y = pThis->m_pObject->ah.ih.m_vPos.y + pThis->m_View.m_FocusYOffset ;
		pThis->m_View.m_vFocus.z = pThis->m_pObject->ah.ih.m_vPos.z ;

		pThis->m_View.m_FocusYOffset -= 1*SCALING_FACTOR*frame_increment ;
		if (pThis->m_View.m_FocusYOffset < 0)
			pThis->m_View.m_FocusYOffset = 0 ;
	}
}


/////////////////////////////////////////////////////////////////////////////
//	Modes:	  	 CAMERA_CINEMA_TUROK_KILL_MANTIS_MODE
//	Description: Turok slaying the mantis
/////////////////////////////////////////////////////////////////////////////
void CCamera__Setup_CINEMATIC_TUROK_KILL_MANTIS(CCamera *pThis)
{
	CGameObjectInstance *pTurok = CEngineApp__GetPlayer(GetApp()) ;

	pThis->m_Mode = CAMERA_CINEMA_TUROK_KILL_MANTIS_MODE ;

	// Put into cinema mode
	pThis->m_CinemaMode = TRUE ;
	pThis->m_LetterBoxScale = LETTERBOX_SCALE ;

	// Setup camera position
	pThis->m_View.m_FocusYOffset = 0 * SCALING_FACTOR ;
	pThis->m_View.m_vEye.x = pTurok->ah.ih.m_vPos.x ;
	pThis->m_View.m_vEye.y = pTurok->ah.ih.m_vPos.y + (4*SCALING_FACTOR) ;
	pThis->m_View.m_vEye.z = pTurok->ah.ih.m_vPos.z ;

	// tell the engine that the mantis is dead
	//BOSS_LevelComplete() ;
}

#define	KILLMANTIS_TOTAL_TIME		120

void CCamera__Code_CINEMATIC_TUROK_KILL_MANTIS(CCamera *pThis)
{
	// Update focus
	if (pThis->m_pObject)
	{
		pThis->m_View.m_vFocus.x = pThis->m_pObject->ah.ih.m_vPos.x ;
		pThis->m_View.m_vFocus.y = pThis->m_pObject->ah.ih.m_vPos.y ;
		pThis->m_View.m_vFocus.z = pThis->m_pObject->ah.ih.m_vPos.z ;

		pThis->m_View.m_vEye.x += (pThis->m_View.m_vFocus.x - pThis->m_View.m_vEye.x)/80 ;
		pThis->m_View.m_vEye.z += (pThis->m_View.m_vFocus.z - pThis->m_View.m_vEye.z)/80 ;
		pThis->m_View.m_vEye.y += 4 ;

		if (pThis->m_View.m_vEye.y > (40*SCALING_FACTOR))
			pThis->m_View.m_vEye.y = 40*SCALING_FACTOR ;
	}

	// Check for ending with button press or anim finished
	if (!pThis->m_CinemaFadingOut)
	{
		// Exit, done?
		pThis->m_Time += frame_increment ;
		if (pThis->m_Time > KILLMANTIS_TOTAL_TIME)
		{
			pThis->m_CinemaFadingOut = TRUE ;
			GetApp()->m_CinemaFlags |= CINEMA_FLAG_TUROK_VICTORY ;
			CEngineApp__SetupFadeTo(GetApp(), MODE_RESETLEVEL) ;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
//	Modes:	  	 CAMERA_CINEMA_MANTIS_KILL_TUROK_MODE
//	Description: Mantis defeating poor old Turok
/////////////////////////////////////////////////////////////////////////////
void CCamera__Setup_CINEMATIC_MANTIS_KILL_TUROK(CCamera *pThis)
{
	CGameObjectInstance *pTurok = CEngineApp__GetPlayer(GetApp()) ;
	CGameObjectInstance *pMantis = GetApp()->m_pBossActive ;
	CVector3	vPos ;

	// set mantis death anim
	Mantis_SetupMode(pMantis, MANTIS_KILLTUROK_MODE) ;

	// move MANTIS into position
	vPos.x = 0 * SCALING_FACTOR ;
	vPos.y = 0 * SCALING_FACTOR ;
	vPos.z = -70 * SCALING_FACTOR ;
	pMantis->ah.ih.m_pCurrentRegion = CScene__NearestRegion(&GetApp()->m_Scene, &vPos) ;
	pMantis->ah.ih.m_vPos = vPos ;
	pMantis->m_RotY = ANGLE_DTOR(0) ;

	// move TUROK into position
	vPos.x = 0 * SCALING_FACTOR ;
	vPos.y = 0 * SCALING_FACTOR ;
	vPos.z = -94 * SCALING_FACTOR ;
	pTurok->ah.ih.m_pCurrentRegion = CScene__NearestRegion(&GetApp()->m_Scene, &vPos) ;
	vPos.y = CGameRegion__GetGroundHeight(pTurok->ah.ih.m_pCurrentRegion, vPos.x, vPos.z) ;
	pTurok->ah.ih.m_vPos = vPos ;
	pTurok->m_RotY = ANGLE_DTOR(0) ;

	pThis->m_Mode = CAMERA_CINEMA_MANTIS_KILL_TUROK_MODE ;

	pThis->m_Stage = 0 ;
	pThis->m_Time = 0 ;
	pThis->m_StageTime = 0 ;
	// Put into cinema mode
	pThis->m_CinemaMode = TRUE ;
	pThis->m_LetterBoxScale = LETTERBOX_SCALE ;
}

#define	MANTISKILLTUROK_TRACK_TIME		80
#define	MANTISKILLTUROK_TOTAL_TIME		80

#define	MANTISKILLTUROK_TIME_BEFORE_FINAL	55

CCameraTrackInfo MantisKillTurokTrack =
{
	ANGLE_DTOR(-30),		ANGLE_DTOR(30),		// YRotOffset,		FinalYRotOffset ;
	12*SCALING_FACTOR,  	12*SCALING_FACTOR, 	//	Radius,			FinalRadius ;
	2*SCALING_FACTOR,		4*SCALING_FACTOR,  	// EyeYOffset,		FinalEyeYOffset ;
	2*SCALING_FACTOR,		4*SCALING_FACTOR,  	// FocusYOffset,	FinalFocusYOffset ;
} ;



void CCamera__Code_CINEMATIC_MANTIS_KILL_TUROK(CCamera *pThis)
{
	CGameObjectInstance *pTurok = CEngineApp__GetPlayer(GetApp()) ;
	CGameObjectInstance *pMantis = GetApp()->m_pBossActive ;

	// Swap objects
	CScene__LoadObjectModelType(&GetApp()->m_Scene, pTurok,
									    AI_OBJECT_CHARACTER_PLAYER, AI_ANIM_INTERACTIVE_ANIMATION1) ;

	AI_SetDesiredAnim(pTurok, AI_ANIM_INTERACTIVE_ANIMATION1, TRUE) ;

	// Update camera
	switch(pThis->m_Stage)
	{
		// Track turok
		case 0:
			pThis->m_StageTime += frame_increment ;
			if (pThis->m_StageTime > MANTISKILLTUROK_TRACK_TIME)
				pThis->m_StageTime = MANTISKILLTUROK_TRACK_TIME ;

			CCameraView__TweenTrackAroundObject(&pThis->m_View, &MantisKillTurokTrack,
														  COSINE_TWEEN(pThis->m_StageTime / MANTISKILLTUROK_TRACK_TIME),
														  pTurok) ;

			// Look from Turok's eye?
			if (pThis->m_Time > MANTISKILLTUROK_TIME_BEFORE_FINAL)
				pThis->m_Stage++ ;
			break ;


		// View from Turoks eye as acid hits it
		case 1:
			CVector3__Set(&pThis->m_View.m_vEye, 
							  0*SCALING_FACTOR,			
							  6.5*SCALING_FACTOR,			
							  -94*SCALING_FACTOR) ;

			CVector3__Set(&pThis->m_View.m_vFocus, 
							  0*SCALING_FACTOR,			
							  6.5*SCALING_FACTOR,			
							  -70*SCALING_FACTOR) ;

			// move TUROK into position
			CVector3__Set(&pTurok->ah.ih.m_vPos,
							  0 * SCALING_FACTOR,
							  0 * SCALING_FACTOR,
							  -94 * SCALING_FACTOR) ;

			pTurok->ah.ih.m_pCurrentRegion = CScene__NearestRegion(&GetApp()->m_Scene, &pTurok->ah.ih.m_vPos) ;
			pTurok->ah.ih.m_vPos.y = CGameRegion__GetGroundHeight(pTurok->ah.ih.m_pCurrentRegion, pTurok->ah.ih.m_vPos.x, pTurok->ah.ih.m_vPos.z) ;
			pTurok->m_RotY = ANGLE_DTOR(0) ;

			break ;
	}

	// Check for ending with button press or anim finished
	if (!pThis->m_CinemaFadingOut)
	{
		// Exit, done?
		pThis->m_Time += frame_increment ;
		if ((pThis->m_StartPressed) || (pThis->m_Time > MANTISKILLTUROK_TOTAL_TIME))
		{
			pThis->m_CinemaFadingOut = TRUE ;
			CEngineApp__DoDeath(GetApp()) ;
			Mantis_SetupMode(pMantis, MANTIS_IDLE_FLOOR_MODE) ;
		}
	}
}




/////////////////////////////////////////////////////////////////////////////
//	Modes:	  	 CAMERA_CINEMA_TUROK_KILL_TREX_MODE
//	Description: Turok slaying the trex
/////////////////////////////////////////////////////////////////////////////
void CCamera__Setup_CINEMATIC_TUROK_KILL_TREX(CCamera *pThis)
{
	pThis->m_Mode = CAMERA_CINEMA_TUROK_KILL_TREX_MODE ;

	// Put into cinema mode
	pThis->m_CinemaMode = TRUE ;
	pThis->m_LetterBoxScale = LETTERBOX_SCALE ;
}

#define	KILLTREX_TRACK_TIME		120
#define	KILLTREX_TOTAL_TIME		150

CCameraTrackInfo KillTrexTrack =
{
	ANGLE_DTOR(0),			ANGLE_DTOR(360),		// YRotOffset,		FinalYRotOffset ;
	10*SCALING_FACTOR,  	40*SCALING_FACTOR, 	//	Radius,			FinalRadius ;
	50*SCALING_FACTOR,	10*SCALING_FACTOR,  	// EyeYOffset,		FinalEyeYOffset ;
	20*SCALING_FACTOR,	8*SCALING_FACTOR,  	// FocusYOffset,	FinalFocusYOffset ;
} ;


void CCamera__Code_CINEMATIC_TUROK_KILL_TREX(CCamera *pThis)
{
	pThis->m_StageTime += frame_increment ;
	pThis->m_Time += frame_increment ;

	if (pThis->m_StageTime > KILLTREX_TRACK_TIME)
		pThis->m_StageTime = KILLTREX_TRACK_TIME ;

	if (pThis->m_pObject)
	{
		CCameraView__TweenTrackAroundObject(&pThis->m_View, &KillTrexTrack,
													  COSINE_TWEEN(pThis->m_StageTime / KILLTREX_TRACK_TIME),
													  pThis->m_pObject) ;
	}



	if (!pThis->m_CinemaFadingOut)
	{
		// Exit, done?
		if (pThis->m_Time > KILLTREX_TOTAL_TIME)
		{
			pThis->m_CinemaFadingOut = TRUE ;
			GetApp()->m_CinemaFlags |= CINEMA_FLAG_TUROK_VICTORY ;
			CEngineApp__SetupFadeTo(GetApp(), MODE_RESETLEVEL) ;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
//	Modes:	  	 CAMERA_CINEMA_TREX_KILL_TUROK_MODE
//	Description: Trex snacking on Turok
/////////////////////////////////////////////////////////////////////////////

CGameObjectInstance	*pTurokCorpse ;
CGameObjectInstance	*pTurokFeathers ;

void CCamera__Setup_CINEMATIC_TREX_KILL_TUROK(CCamera *pThis)
{
	CGameObjectInstance *pTurok = CEngineApp__GetPlayer(GetApp()) ;
	CGameObjectInstance *pTrex = GetApp()->m_pBossActive ;
	CVector3	vPos ;

	// set trex death anim
	TRex_SetupMode(pTrex, TREX_EATTUROK_MODE) ;

	// move TREX into position
	vPos.x = 63 * SCALING_FACTOR ;
	vPos.y = 0 * SCALING_FACTOR ;
	vPos.z = -29 * SCALING_FACTOR ;
	pTrex->ah.ih.m_pCurrentRegion = CScene__NearestRegion(&GetApp()->m_Scene, &vPos) ;
	vPos.y = CGameRegion__GetGroundHeight(pTrex->ah.ih.m_pCurrentRegion, vPos.x, vPos.z) ;
	pTrex->ah.ih.m_vPos = vPos ;
	pTrex->m_RotY = ANGLE_DTOR(-90) ;

	// move TUROK into position
	vPos.x = 90 * SCALING_FACTOR ;
	vPos.y = 0 * SCALING_FACTOR ;
	vPos.z = -29 * SCALING_FACTOR ;
	pTurok->ah.ih.m_pCurrentRegion = CScene__NearestRegion(&GetApp()->m_Scene, &vPos) ;
	vPos.y = CGameRegion__GetGroundHeight(pTurok->ah.ih.m_pCurrentRegion, vPos.x, vPos.z) ;
	pTurok->ah.ih.m_vPos = vPos ;
	pTurok->m_RotY = ANGLE_DTOR(-90) ;


	// Setup initial camera position
	pThis->m_View.m_vEye.x = 98*SCALING_FACTOR ;
	pThis->m_View.m_vEye.y = pTurok->ah.ih.m_vPos.y + (1*SCALING_FACTOR) ;
	pThis->m_View.m_vEye.z = -29*SCALING_FACTOR ;

	pThis->m_View.m_vFocus.x = (80*SCALING_FACTOR) ;
	pThis->m_View.m_vFocus.y = pTurok->ah.ih.m_vPos.y +(10*SCALING_FACTOR);
	pThis->m_View.m_vFocus.z = (-29*SCALING_FACTOR) ;

	pTurokCorpse = NULL ;
	pTurokFeathers = NULL ;

	pThis->m_Mode = CAMERA_CINEMA_TREX_KILL_TUROK_MODE ;

	pThis->m_Stage = 0 ;
	pThis->m_StageTime = 0 ;
	// Put into cinema mode
	pThis->m_CinemaMode = TRUE ;
	pThis->m_LetterBoxScale = LETTERBOX_SCALE ;
}

#define	TREXKILLTUROK_STAGE1_TIME		80
#define	TREXKILLTUROK_STAGE2_TIME		60
#define	TREXKILLTUROK_TOTAL_TIME		170


void CCamera__Code_CINEMATIC_TREX_KILL_TUROK(CCamera *pThis)
{
	CGameObjectInstance *pTrex = GetApp()->m_pBossActive ;
	CVector3		vOutPos, vPos ;
	CGameRegion	*pOutRegion ;
	CGameObjectInstance *pTurok = CEngineApp__GetPlayer(GetApp()) ;
	int	i, nInsts ;
	CGameObjectInstance	*pInstance ;

	// Swap objects
	CScene__LoadObjectModelType(&GetApp()->m_Scene, pTurok,
									    AI_OBJECT_CHARACTER_PLAYER, AI_ANIM_INTERACTIVE_ANIMATION2) ;

	AI_SetDesiredAnim(pTurok, AI_ANIM_INTERACTIVE_ANIMATION2, TRUE) ;


	// find the turok corpse & feather object
	if ((!pTurokCorpse) || (!pTurokFeathers))
	{
		nInsts = GetApp()->m_Scene.m_nActiveAnimInstances;
		for (i=0; i<nInsts; i++)
		{
			pInstance = GetApp()->m_Scene.m_pActiveAnimInstances[i];

			// corpse
			if ((CGameObjectInstance__IsDevice(pInstance)) &&
				(pInstance->ah.ih.m_pEA->m_Id == 1))
			{
				pTurokCorpse = pInstance ;
				pTurokCorpse->m_RotY = pTrex->m_RotY ;
				pTurokCorpse->m_vScale = pTrex->m_vScale ;
				AI_SetDesiredAnim(pInstance, AI_ANIM_ACTION_GO, TRUE);
			}
  
			// feathers
			if ((CGameObjectInstance__IsDevice(pInstance)) &&
				(pInstance->ah.ih.m_pEA->m_Id == 2))
			{
				pTurokFeathers = pInstance ;
				pTurokFeathers->m_RotY = pTrex->m_RotY ;
				pTurokFeathers->m_vScale = pTrex->m_vScale ;
				AI_SetDesiredAnim(pInstance, AI_ANIM_ACTION_GO, TRUE);
			}
		}
	}



	pThis->m_StageTime += frame_increment ;
	pThis->m_Time += frame_increment ;

	switch (pThis->m_Stage)
	{
		case 0:
			if (pThis->m_StageTime > TREXKILLTUROK_STAGE1_TIME)
			{
				pThis->m_StageTime = 0 ;
				pThis->m_Stage++ ;

				// Setup new camera position
				pThis->m_View.m_vEye.x = 74*SCALING_FACTOR ;
				pThis->m_View.m_vEye.y = pTrex->ah.ih.m_vPos.y + (21*SCALING_FACTOR) ;
				pThis->m_View.m_vEye.z = -48*SCALING_FACTOR ;

				pThis->m_View.m_vFocus.x = (77*SCALING_FACTOR) ;
				pThis->m_View.m_vFocus.y = pTrex->ah.ih.m_vPos.y +(21*SCALING_FACTOR);
				pThis->m_View.m_vFocus.z = (-29*SCALING_FACTOR) ;
			}
			break ;

		case 1:
			if (pThis->m_StageTime > TREXKILLTUROK_STAGE2_TIME)
			{
				pThis->m_StageTime = 0 ;
				pThis->m_Stage++ ;

				// Setup new camera position
				pThis->m_View.m_vEye.x = 75*SCALING_FACTOR ;
				pThis->m_View.m_vEye.y = pTrex->ah.ih.m_vPos.y + (21*SCALING_FACTOR) ;
				pThis->m_View.m_vEye.z = -32*SCALING_FACTOR ;

				pThis->m_View.m_vFocus.x = (75*SCALING_FACTOR) ;
				pThis->m_View.m_vFocus.y = pTrex->ah.ih.m_vPos.y +(21*SCALING_FACTOR);
				pThis->m_View.m_vFocus.z = (-29*SCALING_FACTOR) ;
			}
			break ;

		case 2:
			break ;
	}



	if (pTurokCorpse)
	{
		vPos = pTrex->ah.ih.m_vPos ;
		vPos.x += (21*SCALING_FACTOR)	;
		vPos.z += (12*SCALING_FACTOR) ;
		CInstanceHdr__GetNearPositionAndRegion(&pTurokCorpse->ah.ih, vPos, &vOutPos, &pOutRegion, INTERSECT_BEHAVIOR_IGNORE, INTERSECT_BEHAVIOR_IGNORE);
		pTurokCorpse->ah.ih.m_vPos = vOutPos ;
		pTurokCorpse->ah.ih.m_pCurrentRegion = pOutRegion ;
		pTurokCorpse->m_RotY = pTrex->m_RotY ;
	}
	if (pTurokFeathers)
	{
		vPos = pTrex->ah.ih.m_vPos ;
		vPos.x += (26*SCALING_FACTOR)	;
		vPos.z += (6*SCALING_FACTOR) ;
		vPos.y += (16*SCALING_FACTOR) ;
		CInstanceHdr__GetNearPositionAndRegion(&pTurokFeathers->ah.ih, vPos, &vOutPos, &pOutRegion, INTERSECT_BEHAVIOR_IGNORE, INTERSECT_BEHAVIOR_IGNORE);
		pTurokFeathers->ah.ih.m_vPos = vOutPos ;
		pTurokFeathers->ah.ih.m_pCurrentRegion = pOutRegion ;
		pTurokFeathers->m_RotY = pTrex->m_RotY ;
	}


	// Check for ending with button press or anim finished
	if (!pThis->m_CinemaFadingOut)
	{
		// Exit, done?
		if ((pThis->m_StartPressed) || (pThis->m_Time > TREXKILLTUROK_TOTAL_TIME))
		{
			pThis->m_CinemaFadingOut = TRUE ;
			CEngineApp__DoDeath(GetApp()) ;

			// Force into idle mode and start the anim
			pTrex->m_pBoss->m_Mode = TREX_IDLE_MODE ;
			pTrex->m_pBoss->m_OldMode = -1 ;

			// Reset corpse and feathers
			if (pTurokCorpse)
				AI_SetDesiredAnim(pTurokCorpse, AI_ANIM_ACTION_START, TRUE);
			if (pTurokFeathers)
				AI_SetDesiredAnim(pTurokFeathers, AI_ANIM_ACTION_START, TRUE);
		}
	}
}




/////////////////////////////////////////////////////////////////////////////
//	Modes:	  	 CAMERA_CINEMA_TUROK_KILL_LONGHUNTER_MODE
//	Description: Turok slaying the longhunter
/////////////////////////////////////////////////////////////////////////////
void CCamera__Setup_CINEMATIC_TUROK_KILL_LONGHUNTER(CCamera *pThis)
{
	pThis->m_Mode = CAMERA_CINEMA_TUROK_KILL_LONGHUNTER_MODE ;

	// Put into cinema mode
	pThis->m_CinemaMode = TRUE ;
	pThis->m_LetterBoxScale = LETTERBOX_SCALE ;


	// Look at elevator just incase object is not there
	CVector3__Set(&pThis->m_View.m_vEye,
					  0 * SCALING_FACTOR,
					  10 * SCALING_FACTOR,
					  50 * SCALING_FACTOR) ;

	CVector3__Set(&pThis->m_View.m_vFocus,
					  0 * SCALING_FACTOR,
					  0 * SCALING_FACTOR,
					  0 * SCALING_FACTOR) ;

}

#define	KILLLONGHUNTER_TRACK_TIME		50
#define	KILLLONGHUNTER_TOTAL_TIME		140

CCameraTrackInfo KillLonghunterTrack =
{
	ANGLE_DTOR(-0),		ANGLE_DTOR(160),		// YRotOffset,		FinalYRotOffset ;
	7*SCALING_FACTOR,  	8*SCALING_FACTOR, 	//	Radius,			FinalRadius ;
	8*SCALING_FACTOR,		2*SCALING_FACTOR,  	// EyeYOffset,		FinalEyeYOffset ;
	5*SCALING_FACTOR,		3*SCALING_FACTOR,  	// FocusYOffset,	FinalFocusYOffset ;
} ;


void CCamera__Code_CINEMATIC_TUROK_KILL_LONGHUNTER(CCamera *pThis)
{
	CVector3	vDir ;

	pThis->m_StageTime += frame_increment ;
	pThis->m_Time += frame_increment ;

	if (pThis->m_StageTime > KILLLONGHUNTER_TRACK_TIME)
		pThis->m_StageTime = KILLLONGHUNTER_TRACK_TIME ;

	// Track the longhunter
	if (pThis->m_pObject)
	{
		CCameraView__TweenTrackAroundObject(&pThis->m_View, &KillLonghunterTrack,
													  COSINE_TWEEN(pThis->m_StageTime / KILLLONGHUNTER_TRACK_TIME),
													  pThis->m_pObject) ;
	}
	else
	{
		// Setup camera position
		if (pThis->m_Stage == 0)
		{
			pThis->m_Stage = 1 ;

			// Position eye
			vDir = pThis->m_View.m_vEye ;
			vDir.y = 0 ;
			CVector3__Normalize(&vDir) ;
			vDir.x *= 50*SCALING_FACTOR ;
			vDir.y = 10 * SCALING_FACTOR ;
			vDir.z *= 50*SCALING_FACTOR ;
			pThis->m_View.m_vEye  = vDir ;
			
			// Look at center
			CVector3__Set(&pThis->m_View.m_vFocus,
							  0 * SCALING_FACTOR,
							  0 * SCALING_FACTOR,
							  0 * SCALING_FACTOR) ;
		}
	}

	// Check for ending with timeout
	if (!pThis->m_CinemaFadingOut)
	{
		// Exit, done?
		if (pThis->m_Time > KILLLONGHUNTER_TOTAL_TIME)
		{
			pThis->m_CinemaFadingOut = TRUE ;

//			GetApp()->m_CinemaFlags |= CINEMA_FLAG_TUROK_VICTORY ;
//			CEngineApp__SetupFadeTo(GetApp(), MODE_RESETLEVEL) ;

			CCamera__FadeToCinema(pThis, CINEMA_FLAG_TUROK_VICTORY) ;
		}
	}
}




/////////////////////////////////////////////////////////////////////////////
//	Modes:	  	 CAMERA_CINEMA_TUROK_KILL_CAMPAIGNER_MODE
//	Description: Turok slaying the campaigner
/////////////////////////////////////////////////////////////////////////////
void CCamera__Setup_CINEMATIC_TUROK_KILL_CAMPAIGNER(CCamera *pThis)
{
	CGameObjectInstance *pTurok = CEngineApp__GetPlayer(GetApp()) ;
	CGameObjectInstance *pCampaigner = pThis->m_pObject ;
	CVector3		vPos ;

	ASSERT(pTurok) ;
	ASSERT(pCampaigner) ;

	// move CAMPAIGNER into position
	vPos.x = 30 * SCALING_FACTOR ;
	vPos.y = 0 * SCALING_FACTOR ;
	vPos.z = 10 * SCALING_FACTOR ;
	pCampaigner->ah.ih.m_pCurrentRegion = CScene__NearestRegion(&GetApp()->m_Scene, &vPos) ;
	pCampaigner->ah.ih.m_vPos = vPos ;
	pCampaigner->m_RotY = ANGLE_DTOR(-85) ;

	// move TUROK into position
	vPos.x = -4 * SCALING_FACTOR ;
	vPos.y = 0 * SCALING_FACTOR ;
	vPos.z = 10 * SCALING_FACTOR ;
	pTurok->ah.ih.m_pCurrentRegion = CScene__NearestRegion(&GetApp()->m_Scene, &vPos) ;
	vPos.y = CGameRegion__GetGroundHeight(pTurok->ah.ih.m_pCurrentRegion, vPos.x, vPos.z) ;
	pTurok->ah.ih.m_vPos = vPos ;
	pTurok->m_RotY = ANGLE_DTOR(-90) ;


	pThis->m_Mode = CAMERA_CINEMA_TUROK_KILL_CAMPAIGNER_MODE ;

	// Put into cinema mode
	pThis->m_CinemaMode = TRUE ;
	pThis->m_LetterBoxScale = LETTERBOX_SCALE ;

	pThis->m_Stage = 0 ;
	pThis->m_StageTime = 0 ;
}

#define	KILLCAMPAIGNER_TRACK_TIME1		140
#define	KILLCAMPAIGNER_TRACK_TIME2		70
#define	KILLCAMPAIGNER_TRACK_TIME3		80
#define	KILLCAMPAIGNER_TOTAL_TIME		330

CCameraTrackInfo KillCampaignerTrack1 =
{
	ANGLE_DTOR(0),			ANGLE_DTOR(360*2),		// YRotOffset,		FinalYRotOffset ;
	5*SCALING_FACTOR,  	10*SCALING_FACTOR, 	//	Radius,			FinalRadius ;
	12*SCALING_FACTOR,	5*SCALING_FACTOR,  	// EyeYOffset,		FinalEyeYOffset ;
	6*SCALING_FACTOR,		5*SCALING_FACTOR,  	// FocusYOffset,	FinalFocusYOffset ;
} ;

CCameraTrackInfo KillCampaignerTrack2 =
{
	ANGLE_DTOR(0),			ANGLE_DTOR(110),			// YRotOffset,		FinalYRotOffset ;
	10*SCALING_FACTOR,  	5*SCALING_FACTOR, 	//	Radius,			FinalRadius ;
	5*SCALING_FACTOR,		4*SCALING_FACTOR,  	// EyeYOffset,		FinalEyeYOffset ;
	5*SCALING_FACTOR,		1*SCALING_FACTOR,  	// FocusYOffset,	FinalFocusYOffset ;
} ;



void CCamera__Code_CINEMATIC_TUROK_KILL_CAMPAIGNER(CCamera *pThis)
{
	CGameObjectInstance *pTurok = CEngineApp__GetPlayer(GetApp()) ;

	// play turoks end seq idle anim
	CScene__LoadObjectModelType(&GetApp()->m_Scene, pTurok,
									    AI_OBJECT_CHARACTER_PLAYER, AI_ANIM_EXTRA17) ;

	AI_SetDesiredAnim(pTurok, AI_ANIM_EXTRA17, TRUE) ;


	// Update stage
	switch (pThis->m_Stage)
	{
		case 0:
			if (pThis->m_StageTime >= KILLCAMPAIGNER_TRACK_TIME1)
			{
				pThis->m_Stage++ ;
				pThis->m_StageTime = 0 ;
			}
			break ;

		case 1:
			break ;

		case 2:
			if (pThis->m_StageTime > KILLCAMPAIGNER_TRACK_TIME3)
				pThis->m_StageTime = KILLCAMPAIGNER_TRACK_TIME3 ;
			break ;
	}

	// Update camera position
	switch (pThis->m_Stage)
	{
		case 0:
			if (pThis->m_pObject)
			{
				CCameraView__TweenTrackAroundObject(&pThis->m_View, &KillCampaignerTrack1,
															  COSINE_TWEEN(pThis->m_StageTime / KILLCAMPAIGNER_TRACK_TIME1),
															  pThis->m_pObject) ;
			}
			break ;

		case 1:
			if (pThis->m_pObject)
			{
				CCameraView__TweenTrackAroundObject(&pThis->m_View, &KillCampaignerTrack2,
																0, pThis->m_pObject) ;
			}
			break ;

		case 2:
			if (pThis->m_pObject)
			{
				CCameraView__TweenTrackAroundObject(&pThis->m_View, &KillCampaignerTrack2,
															  COSINE_TWEEN(pThis->m_StageTime / KILLCAMPAIGNER_TRACK_TIME3),
															  pThis->m_pObject) ;
			}
			break ;
	}

	// Update time
	pThis->m_StageTime += frame_increment ;
	pThis->m_Time += frame_increment ;

	// Check for ending with timeout
	if (!pThis->m_CinemaFadingOut)
	{
		// Exit, done?
		if (pThis->m_Time > KILLCAMPAIGNER_TOTAL_TIME)
		{
			pThis->m_CinemaFadingOut = TRUE ;
			CCamera__FadeToCinema(&GetApp()->m_Camera, CINEMA_FLAG_END_SEQUENCE) ;
		}
	}
}



/////////////////////////////////////////////////////////////////////////////
//	Modes:	  	 CAMERA_CINEMA_CAMPAIGNER_KILL_TUROK_MODE
//	Description: Campaigner killed turok
/////////////////////////////////////////////////////////////////////////////
void CCamera__Setup_CINEMATIC_CAMPAIGNER_KILL_TUROK(CCamera *pThis)
{
	CGameObjectInstance *pTurok = CEngineApp__GetPlayer(GetApp()) ;
	CGameObjectInstance *pCampaigner = GetApp()->m_pBossActive ;
	CVector3		vPos ;

	// set campaigner death anim
	Campaigner_SetupMode(pCampaigner, CAMPAIGNER_KILL_TUROK_MODE) ;

	// move CAMPAIGNER into position
	vPos.x = 43 * SCALING_FACTOR ;
	vPos.y = 0 * SCALING_FACTOR ;
	vPos.z = 10 * SCALING_FACTOR ;
	pCampaigner->ah.ih.m_pCurrentRegion = CScene__NearestRegion(&GetApp()->m_Scene, &vPos) ;
	pCampaigner->ah.ih.m_vPos = vPos ;
	pCampaigner->m_RotY = ANGLE_DTOR(-85) ;

	// move TUROK into position
	vPos.x = 38 * SCALING_FACTOR ;
	vPos.y = 0 * SCALING_FACTOR ;
	vPos.z = 10 * SCALING_FACTOR ;
	pTurok->ah.ih.m_pCurrentRegion = CScene__NearestRegion(&GetApp()->m_Scene, &vPos) ;
	vPos.y = CGameRegion__GetGroundHeight(pTurok->ah.ih.m_pCurrentRegion, vPos.x, vPos.z) ;
	pTurok->ah.ih.m_vPos = vPos ;
	pTurok->m_RotY = ANGLE_DTOR(-90) ;


	// Setup initial camera position
	pThis->m_View.m_vEye.x = 38*SCALING_FACTOR ;
	pThis->m_View.m_vEye.y = pTurok->ah.ih.m_vPos.y + (6*SCALING_FACTOR) ;
	pThis->m_View.m_vEye.z = 28*SCALING_FACTOR ;


	pThis->m_Mode = CAMERA_CINEMA_CAMPAIGNER_KILL_TUROK_MODE ;

	// Put into cinema mode
	pThis->m_CinemaMode = TRUE ;
	pThis->m_LetterBoxScale = LETTERBOX_SCALE ;

	pThis->m_Stage = 0 ;
	pThis->m_StageTime = 0 ;
}

#define	CAMPAIGNERKILLTUROK_STAGE1_TIME		40
#define	CAMPAIGNERKILLTUROK_STAGE2_TIME		35
#define	CAMPAIGNERKILLTUROK_TOTAL_TIME		102


void CCamera__Code_CINEMATIC_CAMPAIGNER_KILL_TUROK(CCamera *pThis)
{
	CGameObjectInstance *pTurok = CEngineApp__GetPlayer(GetApp()) ;
	CGameObjectInstance *pCampaigner = GetApp()->m_pBossActive ;

	// Swap objects
	CScene__LoadObjectModelType(&GetApp()->m_Scene, pTurok,
									    AI_OBJECT_CHARACTER_PLAYER, AI_ANIM_INTERACTIVE_ANIMATION3) ;

	AI_SetDesiredAnim(pTurok, AI_ANIM_INTERACTIVE_ANIMATION3, TRUE) ;


	switch (pThis->m_Stage)
	{
		case 0:
			if (pThis->m_StageTime > CAMPAIGNERKILLTUROK_STAGE1_TIME)
			{
				pThis->m_Stage++ ;
				pThis->m_StageTime = 0 ;

				pThis->m_View.m_vEye.x = 10*SCALING_FACTOR ;
				pThis->m_View.m_vEye.y = pTurok->ah.ih.m_vPos.y + (3*SCALING_FACTOR) ;
				pThis->m_View.m_vEye.z = 15*SCALING_FACTOR ;
			}
			pThis->m_View.m_vFocus.x = pTurok->ah.ih.m_vPos.x ;
			pThis->m_View.m_vFocus.y = pTurok->ah.ih.m_vPos.y +(4*SCALING_FACTOR);
			pThis->m_View.m_vFocus.z = pTurok->ah.ih.m_vPos.z ;
			break ;

		case 1:
			if (pThis->m_StageTime > CAMPAIGNERKILLTUROK_STAGE2_TIME)
			{
				pThis->m_Stage++ ;
				pThis->m_StageTime = 0 ;

				pThis->m_View.m_vEye.x = pTurok->ah.ih.m_vPos.x - (6*SCALING_FACTOR) ;
				pThis->m_View.m_vEye.y = pTurok->ah.ih.m_vPos.y + (3*SCALING_FACTOR) ;
				pThis->m_View.m_vEye.z = pTurok->ah.ih.m_vPos.z ;//+ (3*SCALING_FACTOR) ;
			}
			pThis->m_View.m_vFocus.x = pCampaigner->ah.ih.m_vPos.x ;
			pThis->m_View.m_vFocus.y = pCampaigner->ah.ih.m_vPos.y +(4*SCALING_FACTOR);
			pThis->m_View.m_vFocus.z = pCampaigner->ah.ih.m_vPos.z ;
			break ;

		case 2:
			pThis->m_View.m_vFocus.x = pTurok->ah.ih.m_vPos.x ;
			pThis->m_View.m_vFocus.y = pTurok->ah.ih.m_vPos.y +(4*SCALING_FACTOR);
			pThis->m_View.m_vFocus.z = pTurok->ah.ih.m_vPos.z ;
			break ;

	}

	// Update time
	pThis->m_StageTime += frame_increment ;
	pThis->m_Time += frame_increment ;

	// Check for ending with timeout
	if (!pThis->m_CinemaFadingOut)
	{
		// Exit, done?
		if ((pThis->m_StartPressed) || (pThis->m_Time > CAMPAIGNERKILLTUROK_TOTAL_TIME))
		{
			pThis->m_CinemaFadingOut = TRUE ;
			CEngineApp__DoDeath(GetApp()) ;
			Campaigner_SetupMode(pCampaigner, CAMPAIGNER_IDLE_MODE) ;
		}
	}
}



/////////////////////////////////////////////////////////////////////////////
//	Modes:	  	 CAMERA_CINEMA_TUROK_PICKUP_KEY
//	Description: Turok picks up a key, whoppee doo!
/////////////////////////////////////////////////////////////////////////////
void CCamera__Setup_CINEMATIC_TUROK_PICKUP_KEY(CCamera *pThis)
{
	pThis->m_Mode = CAMERA_CINEMA_TUROK_PICKUP_KEY_MODE ;

	// Put into cinema mode
	pThis->m_CinemaMode = TRUE ;
	pThis->m_LetterBoxScale = LETTERBOX_SCALE ;

	pThis->m_Time = 0 ;
}

#define	TUROKPICKUPKEY_TOTAL_TIME		60


//char	KeyBuffer[20] ;
void CCamera__Code_CINEMATIC_TUROK_PICKUP_KEY(CCamera *pThis)
{
	int keylev ;

	CGameObjectInstance *pTurok = CEngineApp__GetPlayer(GetApp()) ;

	// Swap objects
	CScene__LoadObjectModelType(&GetApp()->m_Scene, pTurok,
									    AI_OBJECT_CHARACTER_PLAYER, AI_ANIM_EXTRA10) ;

	AI_SetDesiredAnim(pTurok, AI_ANIM_EXTRA10, TRUE) ;

	if (CTurokMovement.LastKeyTexture >=0)
		keylev = CTurokMovement.LastKeyTexture+2 ;
	else
		keylev = 8 ;
#ifdef ENGLISH
	text_levelkey[6] = keylev | 0x30 ;
#endif
#ifdef GERMAN
	text_levelkey[20] = keylev | 0x30 ;
#endif
#ifdef KANJI
	text_levelkey[3] = keylev + 0x41 ;
#endif
	COnScreen__AddGameTextWithId(text_levelkey, 100) ;

	// Update time
	pThis->m_Time += frame_increment ;

	pThis->m_View.m_vEye.x = pTurok->ah.ih.m_vPos.x - ((6*SCALING_FACTOR)*sin(pTurok->m_RotY)) ;
	pThis->m_View.m_vEye.y = pTurok->ah.ih.m_vPos.y + (6*SCALING_FACTOR) ;
	pThis->m_View.m_vEye.z = pTurok->ah.ih.m_vPos.z - ((6*SCALING_FACTOR)*cos(pTurok->m_RotY)) ;

	pThis->m_View.m_vFocus.x = pTurok->ah.ih.m_vPos.x ;
	pThis->m_View.m_vFocus.y = pTurok->ah.ih.m_vPos.y +(7*SCALING_FACTOR);
	pThis->m_View.m_vFocus.z = pTurok->ah.ih.m_vPos.z ;


	// Check for ending with timeout
	if (!pThis->m_CinemaFadingOut)
	{
		// Exit, done?
		if ((pThis->m_StartPressed) || (pThis->m_Time > TUROKPICKUPKEY_TOTAL_TIME))
		{
			GetApp()->m_UseCinemaWarp = 2 ;	// special case, so can print text afterwards...
			pThis->m_CinemaFadingOut = TRUE ;

			// Auto end some of the boss levels
			switch(GetApp()->m_BossLevel)
			{
				case LONGHUNTER_BOSS_LEVEL:
					GetApp()->m_WarpID = KILLED_LONGHUNTER_BOSS_WARP_ID ;
					CTurokMovement.CurrentCheckpoint = KILLED_LONGHUNTER_BOSS_WARP_ID ;

					GetApp()->m_UseCinemaWarp = 3 ;	// special case, so can print text afterwards...
					break ;

				case MANTIS_BOSS_LEVEL:
					GetApp()->m_WarpID = KILLED_MANTIS_BOSS_WARP_ID ;
					CTurokMovement.CurrentCheckpoint = KILLED_MANTIS_BOSS_WARP_ID ;

					GetApp()->m_UseCinemaWarp = 3 ;	// special case, so can print text afterwards...
					break ;
			}

			// Restart new level
			if (GetApp()->m_UseCinemaWarp == 3)
				CEngineApp__Warp(GetApp(), GetApp()->m_WarpID, WARP_BETWEENLEVELS, FALSE) ;
			else
				CEngineApp__SetupFadeTo(GetApp(), MODE_RESETLEVEL) ;
		}
	}
}






/////////////////////////////////////////////////////////////////////////////
//	Modes:	  	 CAMERA_CINEMA_END_SEQUENCE
//	Description: Turok escapes from campaigners lair
/////////////////////////////////////////////////////////////////////////////

// Create an explosion in the arena
void CampArenaInsideExplosion(CTimerFx *pThis)
{
	CVector3		vPos ;
	int			i = 3 ;
	CCamera		*pCamera = &GetApp()->m_Camera ;
	CGameObjectInstance *pTurok = CEngineApp__GetPlayer(GetApp()) ;

	// Turok valid?
	if (!pTurok)
		return ;

	// Start a bunch of explosions
	while(i--)
	{
		// Setup a random direction ray
		vPos.x = RandomRangeFLOAT(-200*SCALING_FACTOR, 130*SCALING_FACTOR) ;
		vPos.z = RandomRangeFLOAT(-115*SCALING_FACTOR, 130*SCALING_FACTOR) ;

		if (vPos.x > 0)
			vPos.y = (130*SCALING_FACTOR) - vPos.x ;
		else
			vPos.y = RandomRangeFLOAT(0, 30*SCALING_FACTOR) ;

		// Start explosion
		AI_DoParticle(&pTurok->ah.ih, PARTICLE_TYPE_GENERIC235, vPos) ;
	}

	// Start tremor going
	pCamera->m_vTremor.x = RandomRangeFLOAT(ANGLE_DTOR(-0.5), ANGLE_DTOR(0.5)) ;
	pCamera->m_vTremor.y = RandomRangeFLOAT(0*SCALING_FACTOR, 0.25*SCALING_FACTOR) ;
	pCamera->m_vTremor.z = RandomRangeFLOAT(ANGLE_DTOR(-0.5), ANGLE_DTOR(0.5)) ;


	// Play explosion sound
	if ((game_frame_number & 7) == 0)
	{
		if (RANDOM(2))
			AI_SEvent_GeneralSound(&pTurok->ah.ih, vPos, SOUND_GENERIC_218) ;
		else
			AI_SEvent_GeneralSound(&pTurok->ah.ih, vPos, SOUND_EXPLOSION_2) ;
	}
}

// Trigger inside explosions
void StartCampArenaInsideExplosions(void)
{
	// Add inside explosions
	CFxSystem__AddTimerFx(&GetApp()->m_FxSystem, CampArenaInsideExplosion,
								 CEngineApp__GetPlayer(GetApp()), 10000, 0) ;

}



void CCamera__Setup_CINEMATIC_END_SEQUENCE(CCamera *pThis)
{
	CGameObjectInstance *pTurok = CEngineApp__GetPlayer(GetApp()) ;

	pThis->m_Mode = CAMERA_CINEMA_END_SEQUENCE_MODE ;

	// Put into cinema mode
	pThis->m_CinemaMode = TRUE ;
	pThis->m_LetterBoxScale = LETTERBOX_SCALE ;

	// point camera at Turok
	pThis->m_pObject = pTurok ;

	pThis->m_StageTime = 0 ;
	pThis->m_Stage = 0 ;
	pThis->m_Time = 0 ;
}

#define	ENDSEQ_IDLE_TIME  	60
#define	ENDSEQ_PREELEVATOR_TIME  	140

#define	ENDSEQ_STAGE1_TIME  	150
#define	ENDSEQ_STAGE2_TIME  	20
#define	ENDSEQ_STAGE3_TIME  	60
#define	ENDSEQ_STAGE4_TIME  	10


CCameraTrackInfo EndSeqTrack1 =
{
	ANGLE_DTOR(-40),		ANGLE_DTOR(40),		// YRotOffset,		FinalYRotOffset ;
	6*SCALING_FACTOR,  	15*SCALING_FACTOR, 	//	Radius,			FinalRadius ;
	4*SCALING_FACTOR,		3*SCALING_FACTOR,  	// EyeYOffset,		FinalEyeYOffset ;
	6*SCALING_FACTOR,		2*SCALING_FACTOR,  	// FocusYOffset,	FinalFocusYOffset ;
} ;

CCameraTrackInfo EndSeqTrack2 =
{
	ANGLE_DTOR(40),		ANGLE_DTOR(190),		// YRotOffset,		FinalYRotOffset ;
	15*SCALING_FACTOR,  	20*SCALING_FACTOR, 	//	Radius,			FinalRadius ;
	3*SCALING_FACTOR,		2*SCALING_FACTOR,  	// EyeYOffset,		FinalEyeYOffset ;
	2*SCALING_FACTOR,		8*SCALING_FACTOR,  	// FocusYOffset,	FinalFocusYOffset ;
} ;




void CCamera__Code_CINEMATIC_END_SEQUENCE(CCamera *pThis)
{
	CGameObjectInstance *pTurok = CEngineApp__GetPlayer(GetApp()) ;
	CInstanceHdr		*pInst = &pTurok->ah.ih ;
	CVector3				vFoci ;

	// Update times
	pThis->m_StageTime += frame_increment ;
	pThis->m_Time += frame_increment ;

	if (pThis->m_Time > ENDSEQ_IDLE_TIME)
	{

		// start in idle mode
		CScene__LoadObjectModelType(&GetApp()->m_Scene, pTurok,
										    AI_OBJECT_CHARACTER_PLAYER, AI_ANIM_EXTRA16) ;


		AI_SetDesiredAnim(pTurok, AI_ANIM_EXTRA16, TRUE) ;

		// Start explosions going
		if (!GetApp()->m_FxSystem.m_TimerFxList.m_ActiveList.m_pHead)
			StartCampArenaInsideExplosions() ;
	}
	else
	{
		// start in idle mode
		CScene__LoadObjectModelType(&GetApp()->m_Scene, pTurok,
											 AI_OBJECT_CHARACTER_PLAYER, AI_ANIM_EXTRA17) ;

		AI_SetDesiredAnim(pTurok, AI_ANIM_EXTRA17, TRUE) ;
	}

	// start elevator?
	if (pThis->m_Time > ENDSEQ_PREELEVATOR_TIME)
		AI_Event_Dispatcher(pInst, pInst, AI_MEVENT_PRESSUREPLATE, pInst->m_pEA->m_dwSpecies, pInst->m_vPos, 50);

	vFoci.x = -4 * SCALING_FACTOR ;
	vFoci.y = 0 ;
	vFoci.z = 10 * SCALING_FACTOR ;

	switch (pThis->m_Stage)
	{
		// idling over dead campaigner
		case 0:
			CCameraView__TweenTrackAroundPos(&pThis->m_View, &EndSeqTrack1,
														  COSINE_TWEEN(pThis->m_StageTime / ENDSEQ_STAGE1_TIME),
														  &vFoci, -90) ;
			if (pThis->m_StageTime > ENDSEQ_STAGE1_TIME)
			{
				pThis->m_Stage++ ;
				pThis->m_StageTime = 0 ;

				// explosion anim
				//AI_SetDesiredAnim(pTurok, AI_ANIM_EXTRA16, TRUE) ;
			}
			break ;

		// looking around, going down elevator
		case 1:
			if (pThis->m_StageTime > ENDSEQ_STAGE2_TIME)
			{
				pThis->m_Stage++ ;
				pThis->m_StageTime = 0 ;
			}
			break ;


		// panning around to see explosions
		case 2:
			CCameraView__TweenTrackAroundPos(&pThis->m_View, &EndSeqTrack2,
														  COSINE_TWEEN(pThis->m_StageTime / ENDSEQ_STAGE3_TIME),
														  &vFoci, -90) ;
			if (pThis->m_StageTime > ENDSEQ_STAGE3_TIME)
			{
				pThis->m_Stage++ ;
				pThis->m_StageTime = 0 ;
			}
			break ;

		// hold on explosions for a while
		case 3:
			if (pThis->m_StageTime > ENDSEQ_STAGE4_TIME)
			{
				pThis->m_Stage++ ;
				pThis->m_StageTime = 0 ;

				// switch to next sequence
				GetApp()->m_pBossActive = NULL ;
				GetApp()->m_WarpID = END_SEQUENCE_WARP_ID ;
				GetApp()->m_UseCinemaWarp = FALSE ;
				GetApp()->m_CinemaFlags = CINEMA_FLAG_END_SEQUENCE2 ;
				CEngineApp__SetupFadeTo(GetApp(), MODE_RESETLEVEL) ;
			}
			break ;
	}
}






/////////////////////////////////////////////////////////////////////////////
//	Modes:	  	 CAMERA_CINEMA_END_SEQUENCE2
//	Description: Turok running down corridor with explosions etc...
/////////////////////////////////////////////////////////////////////////////

// Create an explosion outside the arena
void CampArenaOutsideExplosion(CTimerFx *pThis)
{
	CVector3		vPos ;
	int			i = 3 ;
	CCamera		*pCamera = &GetApp()->m_Camera ;
	CGameObjectInstance *pTurok = CEngineApp__GetPlayer(GetApp()) ;

	// Turok valid?
	if (!pTurok)
		return ;

	// Start a bunch of explosions
	while(i--)
	{
		// Setup a random direction ray
		vPos.x = RandomRangeFLOAT(-612*SCALING_FACTOR, -553*SCALING_FACTOR) ;
		vPos.y = RandomRangeFLOAT(0*SCALING_FACTOR, 100*SCALING_FACTOR) ;
		vPos.z = RandomRangeFLOAT(94*SCALING_FACTOR, 105*SCALING_FACTOR) ;

		// Start explosion
		AI_DoParticle(&pTurok->ah.ih, PARTICLE_TYPE_GENERIC235, vPos) ;
	}

	// Start tremor going
	pCamera->m_vTremor.x = RandomRangeFLOAT(ANGLE_DTOR(-0.5), ANGLE_DTOR(0.5)) ;
	pCamera->m_vTremor.y = RandomRangeFLOAT(0*SCALING_FACTOR, 0.25*SCALING_FACTOR) ;
	pCamera->m_vTremor.z = RandomRangeFLOAT(ANGLE_DTOR(-0.5), ANGLE_DTOR(0.5)) ;

	// Play explosion sound behind turok
	if ((game_frame_number & 7) == 0)
	{
		if (RANDOM(2))
			AI_SEvent_GeneralSound(&pTurok->ah.ih, vPos, SOUND_GENERIC_218) ;
		else
			AI_SEvent_GeneralSound(&pTurok->ah.ih, vPos, SOUND_EXPLOSION_2) ;
	}
}


// Trigger inside explosions
void StartCampArenaOutsideExplosions(void)
{
	// Add inside explosions
	CFxSystem__AddTimerFx(&GetApp()->m_FxSystem, CampArenaOutsideExplosion,
								 CEngineApp__GetPlayer(GetApp()), 10000, 0) ;
}






void CCamera__Setup_CINEMATIC_END_SEQUENCE2(CCamera *pThis)
{
	CGameObjectInstance *pTurok = CEngineApp__GetPlayer(GetApp()) ;

	// start from explosion
	CScene__LoadObjectModelType(&GetApp()->m_Scene, pTurok,
									    AI_OBJECT_CHARACTER_PLAYER, AI_ANIM_EXTRA17) ;

	pThis->m_Mode = CAMERA_CINEMA_END_SEQUENCE2_MODE ;

	// make gravity a little slower so we can see turok fly out of the tunnel...
//	ci_player.m_GravityAcceleration = .01 ;//GRAVITY_ACCELERATION/8 ;

	// Put into cinema mode
	pThis->m_CinemaMode = TRUE ;
	pThis->m_LetterBoxScale = LETTERBOX_SCALE ;

	// point camera at Turok
	//pThis->m_pObject = pTurok ;

	pThis->m_StageTime = 0 ;
	pThis->m_Stage = 1 ;
	pThis->m_Time = 0 ;

	pThis->m_Flags = 0 ;

	pThis->m_View.m_vEye.x = pTurok->ah.ih.m_vPos.x + (8 * SCALING_FACTOR);
	pThis->m_View.m_vEye.y = pTurok->ah.ih.m_vPos.y + (6*SCALING_FACTOR) ;
	pThis->m_View.m_vEye.z = pTurok->ah.ih.m_vPos.z - (40 * SCALING_FACTOR) ;
}

//#define	ENDSEQ2_STAGE1_TIME  	15
#define	ENDSEQ2_STAGE2_TIME  	(30*(1/1.5))
#define	ENDSEQ2_STAGE3_TIME  	(30*(1/1.5))
#define	ENDSEQ2_STAGE4_TIME  	(40*(1/1.5))
#define	ENDSEQ2_STAGE5_TIME  	(60*(1/1.5))
#define	ENDSEQ2_STAGE6_TIME  	(50*(1/1.5))
#define	ENDSEQ2_STAGE7_TIME  	(40*(1/1.5))
#define	ENDSEQ2_STAGE8_TIME  	7
#define	ENDSEQ2_LEAP1_TIME  		30
#define	ENDSEQ2_FALL1_TIME  		40
#define	ENDSEQ2_SWIM1_TIME  		(60)
#define	ENDSEQ2_SWIM2_TIME  		(60*2)

//CCameraTrackInfo EndSeq2Track1 =
//{
//	ANGLE_DTOR(-30),		ANGLE_DTOR(30),		// YRotOffset,		FinalYRotOffset ;
//	8*SCALING_FACTOR,  	8*SCALING_FACTOR, 	//	Radius,			FinalRadius ;
//	3*SCALING_FACTOR,		3*SCALING_FACTOR,  	// EyeYOffset,		FinalEyeYOffset ;
//	6*SCALING_FACTOR,		6*SCALING_FACTOR,  	// FocusYOffset,	FinalFocusYOffset ;
//} ;

// pan around while surfacing water
CCameraTrackInfo EndSeq2TrackSurface =
{
	ANGLE_DTOR(-120),		ANGLE_DTOR(0),			// YRotOffset,		FinalYRotOffset ;
	6*SCALING_FACTOR,  	6*SCALING_FACTOR, 	//	Radius,			FinalRadius ;
	10*SCALING_FACTOR,		7.5*SCALING_FACTOR,  	// EyeYOffset,		FinalEyeYOffset ;
	7.5*SCALING_FACTOR,		11*SCALING_FACTOR,  	// FocusYOffset,	FinalFocusYOffset ;
} ;



void CCamera__Code_CINEMATIC_END_SEQUENCE2(CCamera *pThis)
{
	CEngineApp *pEngine = GetApp() ;
	CGameObjectInstance *pTurok = CEngineApp__GetPlayer(GetApp()) ;
	CVector3			vPos ;
	
	// Load model
	if ((!(pThis->m_Flags & CF_TUROK_LOADED)) && (pTurok))
	{
		// start from explosion
		CScene__LoadObjectModelType(&GetApp()->m_Scene, pTurok,
										    AI_OBJECT_CHARACTER_PLAYER, AI_ANIM_EXTRA17) ;
		pThis->m_Flags |= CF_TUROK_LOADED ;
	}

	// Update times
	pThis->m_StageTime += frame_increment ;
	pThis->m_Time += frame_increment ;

	// point camera at Turok
	pThis->m_pObject = pTurok ;


	// Play explosion sound behind turok
	if ((pThis->m_Stage <= 6) &&  (game_frame_number & 7) == 0)
	{
		vPos = pTurok->ah.ih.m_vPos ;
		vPos.x += RandomRangeFLOAT(-14*SCALING_FACTOR, 14*SCALING_FACTOR) ;
		vPos.z += RandomRangeFLOAT(20*SCALING_FACTOR, 35*SCALING_FACTOR) ;

		if (RANDOM(2))
			AI_SEvent_GeneralSound(&pTurok->ah.ih, vPos, SOUND_GENERIC_218) ;
		else
			AI_SEvent_GeneralSound(&pTurok->ah.ih, vPos, SOUND_EXPLOSION_2) ;
	}

	// move TUROK into position on the water surface
	if (pThis->m_Stage >= 10)
	{
		vPos.x = -580 * SCALING_FACTOR ;
		vPos.y = 0 ;
		vPos.z = -22 * SCALING_FACTOR ;
		pTurok->ah.ih.m_pCurrentRegion = CScene__NearestRegion(&GetApp()->m_Scene, &vPos) ;
		vPos.y = CGameRegion__GetGroundHeight(pTurok->ah.ih.m_pCurrentRegion, vPos.x, vPos.z) ;
		pTurok->ah.ih.m_vPos = vPos ;
	}

	// Update stage
	switch (pThis->m_Stage)
	{
		// explosion going off all around (again)
//		case 0:
//			if (pTurok)
//			{
//				AI_SetDesiredAnim(pTurok, AI_ANIM_EXTRA17, TRUE) ;
//				CCameraView__TweenTrackAroundObject(&pThis->m_View, &EndSeq2Track1,
//															  COSINE_TWEEN(pThis->m_StageTime / ENDSEQ2_STAGE1_TIME),
//															  pThis->m_pObject) ;
//			}
//
//			if (pThis->m_StageTime > ENDSEQ2_STAGE1_TIME)
//			{
//				pThis->m_Stage++ ;
//				pThis->m_StageTime = 0 ;
//				AI_SetDesiredAnim(pTurok, AI_ANIM_EXTRA19, TRUE) ;
//			}
//			break ;

		// running away down a hall (cut1) eye at track position, focus on turok
		case 1:
			AI_SetDesiredAnim(pTurok, AI_ANIM_EXTRA19, TRUE) ;
			pThis->m_View.m_vFocus.x = pTurok->ah.ih.m_vPos.x ;
			pThis->m_View.m_vFocus.y = pTurok->ah.ih.m_vPos.y +(5*SCALING_FACTOR);
			pThis->m_View.m_vFocus.z = pTurok->ah.ih.m_vPos.z ;

			if (pThis->m_StageTime > ENDSEQ2_STAGE2_TIME)
			{
				pThis->m_Stage++ ;
				pThis->m_StageTime = 0 ;

				pThis->m_View.m_vEye.x = pTurok->ah.ih.m_vPos.x - (8*SCALING_FACTOR);
				pThis->m_View.m_vEye.y = pTurok->ah.ih.m_vPos.y + (10*SCALING_FACTOR) ;
				pThis->m_View.m_vEye.z = pTurok->ah.ih.m_vPos.z - (40 * SCALING_FACTOR) ;
			}
			break ;

		// running away down a hall (cut2) top view down
		case 2:
			pThis->m_View.m_vFocus.x = pTurok->ah.ih.m_vPos.x ;
			pThis->m_View.m_vFocus.y = pTurok->ah.ih.m_vPos.y + (5*SCALING_FACTOR);
			pThis->m_View.m_vFocus.z = pTurok->ah.ih.m_vPos.z ;

			if (pThis->m_StageTime > ENDSEQ2_STAGE3_TIME)
			{
				pThis->m_Stage++ ;
				pThis->m_StageTime = 0 ;

			pThis->m_View.m_vEye.x = pTurok->ah.ih.m_vPos.x ;
			pThis->m_View.m_vEye.y = pTurok->ah.ih.m_vPos.y + (8*SCALING_FACTOR) ;
			pThis->m_View.m_vEye.z = pTurok->ah.ih.m_vPos.z - (10 * SCALING_FACTOR) ;

			pThis->m_View.m_vFocus.x = pTurok->ah.ih.m_vPos.x ;
			pThis->m_View.m_vFocus.y = pTurok->ah.ih.m_vPos.y +(6*SCALING_FACTOR);
			pThis->m_View.m_vFocus.z = pTurok->ah.ih.m_vPos.z ;
			}
			break ;

		// running away down a hall (cut3) view from behind
		case 3:
			pThis->m_View.m_vEye.z -= (2.4 * SCALING_FACTOR) * frame_increment ;
			pThis->m_View.m_vFocus.z -= (2.4 * SCALING_FACTOR) * frame_increment ;

			if (pThis->m_StageTime > ENDSEQ2_STAGE4_TIME)
			{
				pThis->m_Stage++ ;
				pThis->m_StageTime = 0 ;

			pThis->m_View.m_vEye.x = pTurok->ah.ih.m_vPos.x ;
			pThis->m_View.m_vEye.y = pTurok->ah.ih.m_vPos.y + (6*SCALING_FACTOR) ;
			pThis->m_View.m_vEye.z = pTurok->ah.ih.m_vPos.z - (10 * SCALING_FACTOR);

			pThis->m_View.m_vFocus.x = pTurok->ah.ih.m_vPos.x ;
			pThis->m_View.m_vFocus.y = pTurok->ah.ih.m_vPos.y +(6*SCALING_FACTOR);
			pThis->m_View.m_vFocus.z = pTurok->ah.ih.m_vPos.z ;

			}
			break ;

		// running away down a hall (cut4) toward camera
		case 4:
			pThis->m_View.m_vEye.z -= (2.4 * SCALING_FACTOR) * frame_increment ;

			if (pThis->m_StageTime > ENDSEQ2_STAGE5_TIME)
			{
				pThis->m_Stage++ ;
				pThis->m_StageTime = 0 ;

				pThis->m_View.m_vEye.x = pTurok->ah.ih.m_vPos.x - (8*SCALING_FACTOR) ;
				pThis->m_View.m_vEye.y = pTurok->ah.ih.m_vPos.y + (8*SCALING_FACTOR) ;
				pThis->m_View.m_vEye.z = pTurok->ah.ih.m_vPos.z  ;

				pThis->m_View.m_vFocus.x = pTurok->ah.ih.m_vPos.x ;
				pThis->m_View.m_vFocus.y = pTurok->ah.ih.m_vPos.y +(6*SCALING_FACTOR);
				pThis->m_View.m_vFocus.z = pTurok->ah.ih.m_vPos.z ;
			}
			break ;

		// running away down a hall (cut5) side view
		case 5:
			pThis->m_View.m_vEye.z -= (2.4 * SCALING_FACTOR) * frame_increment ;
			pThis->m_View.m_vFocus.z -= (2.4 * SCALING_FACTOR) * frame_increment ;

			if (pThis->m_StageTime > ENDSEQ2_STAGE6_TIME)
			{
				pThis->m_Stage++ ;
				pThis->m_StageTime = 0 ;

			pThis->m_View.m_vEye.x = pTurok->ah.ih.m_vPos.x ;
			pThis->m_View.m_vEye.y = pTurok->ah.ih.m_vPos.y + (6 * SCALING_FACTOR) ;
			pThis->m_View.m_vEye.z = pTurok->ah.ih.m_vPos.z - (20 * SCALING_FACTOR) ;

			pThis->m_View.m_vFocus.x = pTurok->ah.ih.m_vPos.x ;
			pThis->m_View.m_vFocus.y = pTurok->ah.ih.m_vPos.y +(6*SCALING_FACTOR);
			pThis->m_View.m_vFocus.z = pTurok->ah.ih.m_vPos.z ;
			}
			break ;

		// running away down a hall (cut6) front view
		case 6:
			pThis->m_View.m_vEye.z -= (2.4 * SCALING_FACTOR) * frame_increment ;

			if (pThis->m_StageTime > ENDSEQ2_STAGE7_TIME)
			{
				pThis->m_Stage++ ;
				pThis->m_StageTime = 0 ;
				AI_SetDesiredAnim(pTurok, AI_ANIM_EXTRA18, TRUE) ;
			}
			break ;

		// starting to leap
		case 7:
			pThis->m_View.m_vFocus.x = pTurok->ah.ih.m_vPos.x ;
			pThis->m_View.m_vFocus.y = pTurok->ah.ih.m_vPos.y +(6*SCALING_FACTOR);
			pThis->m_View.m_vFocus.z = pTurok->ah.ih.m_vPos.z ;

			if (pThis->m_StageTime > ENDSEQ2_STAGE8_TIME)
			{
				pThis->m_Stage++ ;
				pThis->m_StageTime = 0 ;
				StartCampArenaOutsideExplosions() ;

				// move TUROK into position over edge of hole
				vPos.x = -582 * SCALING_FACTOR ;
				vPos.y = 0 ;
				vPos.z = 110 * SCALING_FACTOR ;
				pTurok->ah.ih.m_pCurrentRegion = CScene__NearestRegion(&GetApp()->m_Scene, &vPos) ;
				vPos.y = CGameRegion__GetGroundHeight(pTurok->ah.ih.m_pCurrentRegion, vPos.x, vPos.z) ;
				pTurok->ah.ih.m_vPos = vPos ;

				pThis->m_View.m_vEye.x = -575 * SCALING_FACTOR ;
				pThis->m_View.m_vEye.y = 15 * SCALING_FACTOR ;
				pThis->m_View.m_vEye.z = 80 * SCALING_FACTOR ;

				pThis->m_View.m_vFocus.x = -580 * SCALING_FACTOR ;
				pThis->m_View.m_vFocus.y = 18*SCALING_FACTOR ;
				pThis->m_View.m_vFocus.z = 100 * SCALING_FACTOR ;
			}
			break ;

		// leaping out of exit (cut1) see those flames
		case 8:
			if (pThis->m_StageTime > ENDSEQ2_LEAP1_TIME)
			{
				pThis->m_Stage++ ;
				pThis->m_StageTime = 0 ;

				AI_SetDesiredAnim(pTurok, AI_ANIM_EXTRA15, TRUE) ;

				// position the camera on the ground
				vPos.x = -580 * SCALING_FACTOR ;
				vPos.y = 0 ;
				vPos.z = -40 * SCALING_FACTOR ;
				pTurok->ah.ih.m_pCurrentRegion = CScene__NearestRegion(&GetApp()->m_Scene, &vPos) ;
				vPos.y = CGameRegion__GetGroundHeight(pTurok->ah.ih.m_pCurrentRegion, vPos.x, vPos.z) ;
				vPos.y += 20 * SCALING_FACTOR ;

				pThis->m_View.m_vEye.x = vPos.x  ;
				pThis->m_View.m_vEye.y = vPos.y  ;
				pThis->m_View.m_vEye.z = vPos.z  ;


				// move TUROK into position to fall into water
				vPos.x = -580 * SCALING_FACTOR ;
				vPos.y = 0 ;
				vPos.z = 0 * SCALING_FACTOR ;
				pTurok->ah.ih.m_pCurrentRegion = CScene__NearestRegion(&GetApp()->m_Scene, &vPos) ;
				vPos.y = CGameRegion__GetGroundHeight(pTurok->ah.ih.m_pCurrentRegion, vPos.x, vPos.z) ;
				// move height up to make turok fall
				vPos.y += 40 * SCALING_FACTOR ;
				pTurok->ah.ih.m_vPos = vPos ;
			}
			break ;

		// see turok fall - eye on water, focus on turok
		case 9:
			pThis->m_View.m_vFocus.x = pTurok->ah.ih.m_vPos.x ;
			pThis->m_View.m_vFocus.y = pTurok->ah.ih.m_vPos.y +(6*SCALING_FACTOR) ;
			pThis->m_View.m_vFocus.z = pTurok->ah.ih.m_vPos.z ;

			if (pThis->m_StageTime > ENDSEQ2_FALL1_TIME)
			{
				pThis->m_Stage++ ;
				pThis->m_StageTime = 0 ;

				AI_SetDesiredAnim(pTurok, AI_ANIM_EXTRA20, TRUE) ;
			}

			// Start splash effect
			if ((pTurok->ah.ih.m_vPos.y < ((-120+30)*SCALING_FACTOR)) &&
				 (!(pThis->m_Flags & CF_TUROK_IN_WATER)))
			{
				pThis->m_Flags |= CF_TUROK_IN_WATER ;
				vPos = pTurok->ah.ih.m_vPos ;
				vPos.y = -120*SCALING_FACTOR ; 
				AI_DoParticle(&pTurok->ah.ih, PARTICLE_TYPE_GENERIC96, vPos) ;
			}

			break;

		// swimming
		case 10:

			CCameraView__TweenTrackAroundObject(&pThis->m_View, &EndSeq2TrackSurface,
															  COSINE_TWEEN(pThis->m_StageTime / ENDSEQ2_SWIM1_TIME),
															  pTurok) ;

//			pThis->m_View.m_vEye.x = pTurok->ah.ih.m_vPos.x ;
//			pThis->m_View.m_vEye.y = pTurok->ah.ih.m_vPos.y + (7.5 * SCALING_FACTOR) ;
//			pThis->m_View.m_vEye.z = pTurok->ah.ih.m_vPos.z - (6 * SCALING_FACTOR) ;

//			pThis->m_View.m_vFocus.x = pTurok->ah.ih.m_vPos.x ;
//			pThis->m_View.m_vFocus.y = pTurok->ah.ih.m_vPos.y +(11*SCALING_FACTOR);
//			pThis->m_View.m_vFocus.z = pTurok->ah.ih.m_vPos.z ;

			if (pThis->m_StageTime > ENDSEQ2_SWIM1_TIME)
			{
				pThis->m_Stage++ ;
				pThis->m_StageTime = 0 ;
			}
			break ;

		// looking at exploding lair
		case 11:

			// Fade in game over
			if ((pThis->m_StageTime > SECONDS_TO_FRAMES(3)) && (!pEngine->m_bGameOver))
			{
				pEngine->m_bGameOver = TRUE ;
				pEngine->m_GameOverAlpha = 0 ;
				pEngine->m_GameOverMode = 0 ;
				pEngine->m_GameOverTime = SECONDS_TO_FRAMES(100) ;
			}

			// The very, very, very, very, very... end?
			if (pThis->m_StageTime > ENDSEQ2_SWIM2_TIME)
			{
				pThis->m_Stage++ ;
				pThis->m_StageTime = 0 ;

				GetApp()->m_WarpID = CREDITS_WARP_ID ;
				CEngineApp__SetupFadeTo(GetApp(), MODE_RESETLEVEL) ;
			}
			break ;
	}
}









/////////////////////////////////////////////////////////////////////////////
//	Modes:	  	 CAMERA_CINEMA_TUROK_VICTORY_MODE
//	Description: Turok doing his little victory dance
/////////////////////////////////////////////////////////////////////////////
#define CINEMA_TUROK_VICTORY_TRACK_TIME	SECONDS_TO_FRAMES(4)
#define CINEMA_TUROK_VICTORY_TOTAL_TIME	SECONDS_TO_FRAMES(5)

CCameraTrackInfo TurokVictoryTrack =
{
	ANGLE_DTOR(45),		ANGLE_DTOR(0),			// YRotOffset,		FinalYRotOffset ;
	10.5*SCALING_FACTOR,  	10.5*SCALING_FACTOR, 	//	Radius,			FinalRadius ;
	3.8*SCALING_FACTOR,		3.8*SCALING_FACTOR,  	// EyeYOffset,		FinalEyeYOffset ;
	3.8*SCALING_FACTOR,		3.8*SCALING_FACTOR,  	// FocusYOffset,	FinalFocusYOffset ;
} ;

void CCamera__Setup_CINEMATIC_TUROK_VICTORY(CCamera *pThis)
{
	pThis->m_Mode = CAMERA_CINEMA_TUROK_VICTORY_MODE ;

	// Put into cinema mode
	pThis->m_CinemaMode = TRUE ;
	pThis->m_LetterBoxScale = LETTERBOX_SCALE ;

	pThis->m_StageTime = 0 ;
	pThis->m_Stage = 0 ;
	pThis->m_Time = 0 ;
}


void CCamera__Code_CINEMATIC_TUROK_VICTORY(CCamera *pThis)
{
	CGameObjectInstance *pTurok = CEngineApp__GetPlayer(GetApp()) ;
	CEngineApp	*pEngine = GetApp() ;

	// Set Turok's anim
	CScene__LoadObjectModelType(&GetApp()->m_Scene, pTurok,
									    AI_OBJECT_CHARACTER_PLAYER, AI_ANIM_IDLE) ;

	AI_SetDesiredAnim(pTurok, AI_ANIM_IDLE, TRUE) ;

	// Update stage time
	pThis->m_StageTime += frame_increment ;
	if (pThis->m_StageTime > CINEMA_TUROK_VICTORY_TRACK_TIME)
		pThis->m_StageTime = CINEMA_TUROK_VICTORY_TRACK_TIME ;

	// Set camera pos
	CCameraView__TweenTrackAroundTurok(&pThis->m_View, &TurokVictoryTrack,
												  COSINE_TWEEN(pThis->m_StageTime / CINEMA_TUROK_VICTORY_TRACK_TIME)) ;

	// Check for ending with timeout
	if (!pThis->m_CinemaFadingOut)
	{
		// Exit, done?
		pThis->m_Time += frame_increment ;
		if (pThis->m_Time > CINEMA_TUROK_VICTORY_TOTAL_TIME)
		{
			pThis->m_CinemaFadingOut = TRUE ;

			// Auto end some of the boss levels which don't generate a key after the boss is dead
			switch(pEngine->m_BossLevel)
			{
				case TREX_BOSS_LEVEL:
					pEngine->m_WarpID = KILLED_TREX_BOSS_WARP_ID ;
					CEngineApp__Warp(GetApp(), GetApp()->m_WarpID, WARP_BETWEENLEVELS, FALSE) ;
					break ;

				case CAMPAIGNER_BOSS_LEVEL:
					pEngine->m_WarpID = KILLED_CAMPAIGNER_BOSS_WARP_ID ;
					CEngineApp__Warp(GetApp(), GetApp()->m_WarpID, WARP_BETWEENLEVELS, FALSE) ;
					break ;

				default:

					CCamera__FadeToCinema(pThis, 0) ;
					break ;
			}

		}
	}
}

