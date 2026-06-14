/*****************************************************************************
*
*
*
*				Generic head tracking control file by Stephen Broumley
*
*
*
*****************************************************************************/


#include "stdafx.h"
#include "boss.h"
#include "hedtrack.h"
#include "scaling.h"
#include "tengine.h"
#include "ai.h"

/*****************************************************************************
*	Constants
*****************************************************************************/

#define AI_GetVel(pThis)	((pThis)->ah.m_vVelocity)



/*****************************************************************************
*	Enemy/Boss head track information
*****************************************************************************/


// MANTIS

CHeadTrackNodeInfo MantisHeadTrackNodeInfo[] =
{
	{REVERSE_CONCAT | TURN_AXIS_Z | TILT_AXIS_X},	// Flags
} ;

CHeadTrackInfo	MantisHeadTrackInfo =
{
	14*SCALING_FACTOR,			// Eye height relative to feet

	ANGLE_DTOR(0),					// Turn offset
	ANGLE_DTOR(90),				// Max left/right rotation angle
	ANGLE_DTOR(10),				// Max left/right rotation tracking speed

	ANGLE_DTOR(-90),				// Min looking up angle
	ANGLE_DTOR(90),				// Max looking down angle
	ANGLE_DTOR(10),				// Max up/down rotation tracking speed

	29,				 				// Min head node	
	29,				 				// Max head node	

	MantisHeadTrackNodeInfo,	// Ptr to node info

	2									// Big head cheat scale
} ;


// CAMPAINGER
CHeadTrackNodeInfo CampaignerHeadTrackNodeInfo[] =
{
	{REVERSE_CONCAT | TURN_FLIP_DIR | TURN_AXIS_Y | TILT_AXIS_X},	// Flags
} ;

CHeadTrackInfo	CampaignerHeadTrackInfo =
{
	6.5*SCALING_FACTOR,				// Eye height relative to feet

	ANGLE_DTOR(18),					// Turn offset
	ANGLE_DTOR(75),					// Max left/right rotation angle
	ANGLE_DTOR(10),					// Max left/right rotation tracking speed

	ANGLE_DTOR(-90),					// Min looking up angle
	ANGLE_DTOR(90),					// Max looking down angle
	ANGLE_DTOR(10),					// Max up/down rotation tracking speed

	21,			 						// Min head node	
	21,										// Max head node	

	CampaignerHeadTrackNodeInfo,	// Ptr to node info

	4										// Big head cheat scale
} ;

CHeadTrackInfo	CampaignerRunHeadTrackInfo =
{
	6.5*SCALING_FACTOR,				// Eye height relative to feet

	ANGLE_DTOR(0),						// Turn offset
	ANGLE_DTOR(75),					// Max left/right rotation angle
	ANGLE_DTOR(10),					// Max left/right rotation tracking speed

	ANGLE_DTOR(-90),					// Min looking up angle
	ANGLE_DTOR(90),					// Max looking down angle
	ANGLE_DTOR(10),					// Max up/down rotation tracking speed

	21,			 						// Min head node	
	21,										// Max head node	

	CampaignerHeadTrackNodeInfo,	// Ptr to node info

	4										// Big head cheat scale
} ;



// LONGHUNTER
CHeadTrackNodeInfo LonghuntHeadTrackNodeInfo[] =
{
	{REVERSE_CONCAT | TURN_FLIP_DIR | TURN_AXIS_Y | TILT_AXIS_X},	// Flags
} ;

CHeadTrackInfo	LonghuntHeadTrackInfo =
{
	6.0*SCALING_FACTOR,				// Eye height relative to feet

	ANGLE_DTOR(0),					// Turn offset
	ANGLE_DTOR(75),				 // Max left/right rotation angle
	ANGLE_DTOR(10),					 // Max left/right rotation tracking speed

	ANGLE_DTOR(-90),				 // Min looking up angle
	ANGLE_DTOR(90),				 // Max looking down angle
	ANGLE_DTOR(10), 				 // Max up/down rotation tracking speed

	20,								 // Min head node	
	20,									 // Max head node	

	LonghuntHeadTrackNodeInfo,	// Ptr to node info

	4									// Big head cheat scale
} ;


// IDLE TREX
CHeadTrackNodeInfo TRexHeadTrackNodeInfo[] =
{
	{TURN_AXIS_Z | TILT_AXIS_X},	// Flags
	{TURN_AXIS_Z | TILT_AXIS_X},	// Flags
	{TURN_AXIS_Z | TILT_AXIS_X},	// Flags
} ;

CHeadTrackInfo	TRexHeadTrackInfo =
{
	18*SCALING_FACTOR,	// Eye height relative to feet

	ANGLE_DTOR(0),	 	// Turn offset
	ANGLE_DTOR(90),	 // Max left/right rotation angle
	ANGLE_DTOR(10),	 // Max left/right rotation tracking speed

	ANGLE_DTOR(-90),	 // Min looking up angle
	ANGLE_DTOR(90),	 // Max looking down angle
	ANGLE_DTOR(10),	 // Max up/down rotation tracking speed

	2,						 // Min head node	
	4,						 // Min head node	

	TRexHeadTrackNodeInfo,	// Ptr to node info

	2 								// Big head cheat scale
} ;


// HUMAN1
CHeadTrackNodeInfo Human1HeadTrackRunNodeInfo[] =
{
	{REVERSE_CONCAT | TURN_FLIP_DIR | TURN_AXIS_Y | TILT_AXIS_X},	// Flags
} ;

CHeadTrackInfo	Human1HeadTrackInfo =
{
	6*SCALING_FACTOR,	 // Eye height relative to feet

	ANGLE_DTOR(0),		// Turn offset
	ANGLE_DTOR(75),	 // Max left/right rotation angle
	ANGLE_DTOR(10),	 // Max left/right rotation tracking speed

	ANGLE_DTOR(-90),	 // Min looking up angle
	ANGLE_DTOR(90),	 // Max looking down angle
	ANGLE_DTOR(10),	 // Max up/down rotation tracking speed

	19,					 // Min head node	
	19,					 // Min head node	

	Human1HeadTrackRunNodeInfo,	// Ptr to node info

	4										// Big head cheat scale
} ;


// ALIEN
CHeadTrackNodeInfo AlienHeadTrackRunNodeInfo[] =
{
	{REVERSE_CONCAT | TURN_AXIS_Z | TILT_AXIS_X},	// Flags
	{REVERSE_CONCAT | TURN_AXIS_X | TILT_AXIS_Y | TILT_FLIP_DIR},	// Flags
} ;


CHeadTrackInfo	AlienHeadTrackInfo =
{
	3.8*SCALING_FACTOR,// Eye height relative to feet

	ANGLE_DTOR(0),					// Turn offset
	ANGLE_DTOR(75),	 // Max left/right rotation angle
	ANGLE_DTOR(10),	 // Max left/right rotation tracking speed

	ANGLE_DTOR(-90),	 // Min looking up angle
	ANGLE_DTOR(90),	 // Max looking down angle
	ANGLE_DTOR(10),	 // Max up/down rotation tracking speed

	11,						 // Min head node	
	12,						 // Max head node	

	AlienHeadTrackRunNodeInfo,	// Ptr to node info

	5								  	// Big head cheat scale
} ;



// RAPTOR
CHeadTrackNodeInfo RaptorHeadTrackRunNodeInfo[] =
{
	{REVERSE_CONCAT | TURN_FLIP_DIR | TURN_AXIS_Z | TILT_FLIP_DIR | TILT_AXIS_X},
	{REVERSE_CONCAT | TURN_FLIP_DIR | TURN_AXIS_Z | TILT_FLIP_DIR | TILT_AXIS_X},
	{REVERSE_CONCAT | TURN_FLIP_DIR | TURN_AXIS_Z | TILT_FLIP_DIR | TILT_AXIS_X},
} ;


CHeadTrackInfo	RaptorHeadTrackInfo =
{
	7.4*SCALING_FACTOR,	 // Eye height relative to feet

	ANGLE_DTOR(0),					// Turn offset
	ANGLE_DTOR(90),		 // Max left/right rotation angle
	ANGLE_DTOR(16),		 // Max left/right rotation tracking speed

	ANGLE_DTOR(-90),		 // Min looking up angle
	ANGLE_DTOR(90),		 // Max looking down angle
	ANGLE_DTOR(16),		 // Max up/down rotation tracking speed

	3,							 // Min head node	
	5,							 // Max head node	

	RaptorHeadTrackRunNodeInfo,	// Ptr to node info

	2										// Big head cheat scale
} ;


// LEAPER
CHeadTrackNodeInfo LeaperHeadTrackRunNodeInfo[] =
{
	{REVERSE_CONCAT | TILT_AXIS_X | TURN_AXIS_Z},
	{REVERSE_CONCAT | TURN_AXIS_Z | TILT_AXIS_X},
} ;


CHeadTrackInfo	LeaperHeadTrackInfo =
{
	2*SCALING_FACTOR,	// Eye height relative to feet

	ANGLE_DTOR(0),					// Turn offset
	ANGLE_DTOR(75),	 // Max left/right rotation angle
	ANGLE_DTOR(10),	 // Max left/right rotation tracking speed

	ANGLE_DTOR(-90),	 // Min looking up angle
	ANGLE_DTOR(90),	 // Max looking down angle
	ANGLE_DTOR(10),	 // Max up/down rotation tracking speed

	35, 					 // Min head node	
	36, 					 // Max head node	

	LeaperHeadTrackRunNodeInfo,	// Ptr to node info

	2										// Big head cheat scale
} ;




// HULK
CHeadTrackNodeInfo HulkHeadTrackRunNodeInfo[] =
{
	{TURN_FLIP_DIR | TURN_AXIS_Y | TILT_AXIS_X},
} ;


CHeadTrackInfo	HulkHeadTrackInfo =
{
	6*SCALING_FACTOR,	// Eye height relative to feet

	ANGLE_DTOR(0),					// Turn offset
	ANGLE_DTOR(25),	 // Max left/right rotation angle
	ANGLE_DTOR(10),	 // Max left/right rotation tracking speed

	ANGLE_DTOR(-25),	 // Min looking up angle
	ANGLE_DTOR(25),	 // Max looking down angle
	ANGLE_DTOR(10), 	 // Max up/down rotation tracking speed

	22, 					 // Min head node	
	22, 					 // Max head node	

	HulkHeadTrackRunNodeInfo,	// Ptr to node info

	2								 	// Big head cheat scale
} ;








/*****************************************************************************
*	Functions
*****************************************************************************/


BOOL CGameObjectInstance__IsDoingTurnAnim(CGameObjectInstance *pThis)
{
	INT32	Anim, AnimType ;

	Anim = CGameObjectInstance__GetCurrentAnim(pThis) ;
	AnimType = CGameObjectInstance__LookupAIAnimType(pThis, Anim) ;
	switch (AnimType)
	{
		case AI_ANIM_TURNL90:
		case AI_ANIM_TURNR90:
		case AI_ANIM_TURN180:

		case AI_ANIM_WTURNL90:
		case AI_ANIM_WTURNR90:
		case AI_ANIM_WTURN180:

		case AI_ANIM_RTURNL90:
		case AI_ANIM_RTURNR90:
		case AI_ANIM_RTURN180:

		case AI_ANIM_LTURNL90:
		case AI_ANIM_LTURNR90:
		case AI_ANIM_LTURN180:

		case AI_ANIM_BOSS_MANTIS_TURN_CEILING_LEFT90:
		case AI_ANIM_BOSS_MANTIS_TURN_CEILING_RIGHT90:
		case AI_ANIM_BOSS_MANTIS_TURN_FLOOR_LEFT90:
		case AI_ANIM_BOSS_MANTIS_TURN_FLOOR_RIGHT90:

			return TRUE;

		default:
			return FALSE;
	}
}


/*****************************************************************************
*
*	Function:		AI_TrackHead()
*
******************************************************************************
*
*	Description:	Makes enemies/bosses head track Turok
*
*	Inputs:			*pThis -	Ptr to game object instance
*						*pInfo - Ptr to head tracking info
*						Track	 - Set to TRUE if tracking should be done
*
*	Outputs:			None
*
*****************************************************************************/
#define	HEAD_TRACK_BLEND_IN_TIME	SECONDS_TO_FRAMES(0.5)
#define	HEAD_TRACK_BLEND_OUT_TIME	SECONDS_TO_FRAMES(0.5)


void AI_HeadTrack(CGameObjectInstance *pThis, CHeadTrackInfo *pInfo, 
						BOOL TrackTarget, BOOL CanTrackDuringTurn)
{
	FLOAT						TargetAngle, DeltaAngle ;
	CVector3					vThis, vTarget ;
	CGameObjectInstance *pTarget ;

	// Finally turn head tracking on
	pThis->m_pHeadTrackInfo = pInfo ;

	// Get target to look at
	pTarget = pThis->m_pAttackerCGameObjectInstance ;
//	pTarget = CEngineApp__GetPlayer(GetApp()) ;


#if 0
	pTarget = CEngineApp__GetPlayer(GetApp()) ;

//	pThis->m_HeadTiltAngle += ANGLE_DTOR(5) ;
	pThis->m_HeadTiltAngle = 0 ;

	pThis->m_HeadTurnAngle += ANGLE_DTOR(5) ;
//	pThis->m_HeadTurnAngle = 0 ;

	pThis->m_HeadTrackBlend = 1.0 ;
	return ;
#endif


	// No tracking?
	if ((!TrackTarget) || (!pTarget) || (!pThis->m_AI.m_Health) ||
		 ((!CanTrackDuringTurn) && (CGameObjectInstance__IsDoingTurnAnim(pThis))))
	{
		// Blend out
		pThis->m_HeadTrackBlend = DecelerateFLOAT(pThis->m_HeadTrackBlend, 1.0 / HEAD_TRACK_BLEND_OUT_TIME) ;

		// Take down angles
		pThis->m_HeadTurnAngle = DecelerateFLOAT(pThis->m_HeadTurnAngle, pInfo->TurnSpeed) ;
		pThis->m_HeadTiltAngle = DecelerateFLOAT(pThis->m_HeadTiltAngle, pInfo->TiltSpeed) ;
	}
	else
	{
		// Blend in
		pThis->m_HeadTrackBlend = AccelerateFLOAT(pThis->m_HeadTrackBlend, 1.0 / HEAD_TRACK_BLEND_IN_TIME, 1.0) ;

		// Find head angle to look at target
		TargetAngle = -AI_GetAvoidanceAngle(pThis, AI_GetPos(pTarget), pTarget, AVOIDANCE_RADIUS_DISTANCE_FACTOR) ;

		// If on ceiling, swap head turn for bosses (the mantis!)
		if ((pThis->m_pBoss) && (pThis->m_pBoss->m_ModeFlags & BF_CEILING_MODE))
			TargetAngle = -TargetAngle ;

		// Add in turn offset
		TargetAngle += pInfo->TurnAngleOffset ;

		// Limit turn speed
		DeltaAngle = (TargetAngle - pThis->m_HeadTurnAngle) / 2 ;
		if (DeltaAngle > pInfo->TurnSpeed)
			DeltaAngle = pInfo->TurnSpeed ;
		else
		if (DeltaAngle < -pInfo->TurnSpeed)
			DeltaAngle = -pInfo->TurnSpeed ;

		pThis->m_HeadTurnAngle += DeltaAngle * frame_increment ;

		// Limit turn angle
		if (pThis->m_HeadTurnAngle > (pInfo->MaxTurnAngle + pInfo->TurnAngleOffset))
			pThis->m_HeadTurnAngle = (pInfo->MaxTurnAngle + pInfo->TurnAngleOffset) ;
		else
		if (pThis->m_HeadTurnAngle < (-pInfo->MaxTurnAngle + pInfo->TurnAngleOffset))
			pThis->m_HeadTurnAngle = (-pInfo->MaxTurnAngle + pInfo->TurnAngleOffset) ;


		// Find head tilt angle(-90 = up, 0 = level ,90 =down)
		vThis = AI_GetPos(pThis) ;
		vThis.y += pInfo->EyeHeight ;

		vTarget = AI_GetPos(pTarget) ;
		vTarget.y += AI_GetEA(pTarget)->m_CollisionHeight*0.75 ;

		vTarget.x -= vThis.x ;
		vTarget.y -= vThis.y ;
		vTarget.z -= vThis.z ;

		TargetAngle = CVector3__XYAngle(&vTarget) ;

		// Limit tilt speed
		DeltaAngle = (TargetAngle - pThis->m_HeadTiltAngle) / 2 ;
		if (DeltaAngle > pInfo->TiltSpeed)
			DeltaAngle = pInfo->TiltSpeed ;
		else
		if (DeltaAngle < -pInfo->TiltSpeed)
			DeltaAngle = -pInfo->TiltSpeed ;
		pThis->m_HeadTiltAngle += DeltaAngle * frame_increment ;

		// Limit tilt angle
		if (pThis->m_HeadTiltAngle > pInfo->MaxTiltAngle)
			pThis->m_HeadTiltAngle = pInfo->MaxTiltAngle ;
		else
		if (pThis->m_HeadTiltAngle < -pInfo->MaxTiltAngle)
			pThis->m_HeadTiltAngle = -pInfo->MaxTiltAngle ;
	}
}


void AI_PerformHeadTracking(CGameObjectInstance *pThis)
{
	// Do head track?
	switch(CGameObjectInstance__TypeFlag(pThis))
	{
		// MANTIS - Done
		case AI_OBJECT_CHARACTER_MANTIS_BOSS:
			if (pThis->m_pBoss->m_ModeFlags & BF_HEAD_TRACK_TUROK)
				AI_HeadTrack(pThis, &MantisHeadTrackInfo, TRUE, FALSE) ;
			else
				AI_HeadTrack(pThis, &MantisHeadTrackInfo, FALSE, FALSE) ;
			break ;

		// CAMPAIGNER - Done
		case AI_OBJECT_CHARACTER_CAMPAIGNER_BOSS:
			if (pThis->m_pBoss->m_ModeFlags & BF_HEAD_TRACK_TUROK)
			{
				if (pThis->m_pBoss->m_ModeFlags & BF_RUNNING_MODE)
					AI_HeadTrack(pThis, &CampaignerRunHeadTrackInfo, TRUE, TRUE) ;
				else	
					AI_HeadTrack(pThis, &CampaignerHeadTrackInfo, TRUE, TRUE) ;
			}
			else
				AI_HeadTrack(pThis, &CampaignerHeadTrackInfo, FALSE, TRUE) ;

			break ;

		// LONGHUNTER - Done
		case AI_OBJECT_CHARACTER_LONGHUNTER_BOSS:
			if (pThis->m_pBoss->m_ModeFlags & BF_HEAD_TRACK_TUROK)
				AI_HeadTrack(pThis, &LonghuntHeadTrackInfo, TRUE, FALSE) ;
			else
				AI_HeadTrack(pThis, &LonghuntHeadTrackInfo, FALSE, FALSE) ;
			break ;

		// TREX - Done
		case AI_OBJECT_CHARACTER_TREX_BOSS:
			if (pThis->m_pBoss->m_ModeFlags & BF_HEAD_TRACK_TUROK)
				AI_HeadTrack(pThis, &TRexHeadTrackInfo, TRUE, FALSE) ;
			else
				AI_HeadTrack(pThis, &TRexHeadTrackInfo, FALSE, FALSE) ;
			break ;

		// HUMAN1 - Done
		case AI_OBJECT_CHARACTER_HUMAN1:
		 	AI_HeadTrack(pThis, &Human1HeadTrackInfo, TRUE, FALSE) ;
			break ;

		// ALIEN - Done
		case AI_OBJECT_CHARACTER_ALIEN:
		 	AI_HeadTrack(pThis, &AlienHeadTrackInfo, TRUE, FALSE) ;
			break ;

		// RAPTOR - Done
		case AI_OBJECT_CHARACTER_RAPTOR:
		 	AI_HeadTrack(pThis, &RaptorHeadTrackInfo, TRUE, FALSE) ;
			break ;

		// LEAPER - Done
		case AI_OBJECT_CHARACTER_LEAPER:
		 	AI_HeadTrack(pThis, &LeaperHeadTrackInfo, TRUE, FALSE) ;
			break ;

		// HULK - Done
		case AI_OBJECT_CHARACTER_HULK:
		 	AI_HeadTrack(pThis, &HulkHeadTrackInfo, TRUE, FALSE) ;
			break ;
	}
}

