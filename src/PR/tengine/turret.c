// Turret control file by Stephen Broumley

#include "stdafx.h"
#include "debug.h"
#include "ai.h"
#include "turret.h"
#include "tengine.h"
#include "scaling.h"
#include "particle.h"
#include "boss.h"




/////////////////////////////////////////////////////////////////////////////
//	Structures
/////////////////////////////////////////////////////////////////////////////


// Tracking structure
typedef struct CTurretTrackInfo_t
{
	CVector3	vPivotOffset ;				// Pivot offset relative to world pos

	FLOAT	TurnTrackFactor ;				//	SHM
	FLOAT MaxTurnSpeed ;	  				// Max left/right rotation tracking speed

	FLOAT TiltTrackFactor ;				// Up/down track factor (bigger = slower)
	FLOAT MaxTiltSpeed ;						// Max up/down rotation tracking speed

	FLOAT MinTiltAngle ;					// Min looking up angle
	FLOAT MaxTiltAngle ;					// Max looking down angle
} CTurretTrackInfo ;



/*****************************************************************************
*
*	Function:		Turret_TrackPlayer()
*
******************************************************************************
*
*	Description:	Makes turret track the player - in true turret style
*
*	Inputs:			*pThis	-	Ptr to turret game object instance
*						*pInfo	-	Ptr to turret tracking info
*
*	Outputs:			None
*
*****************************************************************************/
void Turret_TrackPlayer(CGameObjectInstance *pThis, CTurretTrackInfo *pInfo)
{
	CGameObjectInstance	*pTarget ;
	FLOAT						TargetAngle, DeltaAngle ;
	CVector3					vThis, vTarget ;

	// Set death anim
	if (pThis->m_AI.m_Health <= 0)
		AI_SetDesiredAnim(pThis, AI_ANIM_DEATH_NORMAL, TRUE) ;

#if 1
	// Get target to aim at
	pTarget = pThis->m_pAttackerCGameObjectInstance ;
	if (!pTarget)
		pTarget = CEngineApp__GetPlayer(GetApp()) ;

	// Move to correct angle
	pThis->m_RotY = pThis->m_HeadTurnAngle ;

	if (pThis->m_AI.m_Health > 0)
		DeltaAngle = AI_GetAngle(pThis, pTarget->ah.ih.m_vPos) ;
	else
		DeltaAngle = 0 ;

	TargetAngle = DeltaAngle * pInfo->TurnTrackFactor * frame_increment ;
	if (abs(TargetAngle) > abs(DeltaAngle))
		TargetAngle = DeltaAngle ;
	pThis->m_HeadTurnAngle += TargetAngle ;
	pThis->m_RotY = pThis->m_HeadTurnAngle ;


	// Limit turn speed
	if (pThis->m_HeadTurnAngleVel > pInfo->MaxTurnSpeed)
		pThis->m_HeadTurnAngleVel = pInfo->MaxTurnSpeed ;
	else
	if (pThis->m_HeadTurnAngleVel < -pInfo->MaxTurnSpeed)
		pThis->m_HeadTurnAngleVel = -pInfo->MaxTurnSpeed ;

	// Finally move turret
	pThis->m_HeadTurnAngle += pThis->m_HeadTurnAngleVel ;


	// Only track tilt if turret is almost facing target
	if (pThis->m_AI.m_Health <=  0)
		TargetAngle = pInfo->MaxTiltAngle ;
	else
	if (abs(DeltaAngle) > ANGLE_DTOR(180))
		TargetAngle = 0 ;
	else
	{
		// Find head tilt angle(-90 = up, 0 = level ,90 =down)
		CVector3__Add(&vThis, &pThis->ah.ih.m_vPos, &pInfo->vPivotOffset) ;

		vTarget = pTarget->ah.ih.m_vPos ;
		vTarget.y += AI_GetEA(pTarget)->m_CollisionHeight*0.75 ;

		vTarget.x -= vThis.x ;
		vTarget.y -= vThis.y ;
		vTarget.z -= vThis.z ;

		TargetAngle = CVector3__XYAngle(&vTarget) ;
	}

	// Limit tilt speed
	DeltaAngle = (TargetAngle - pThis->m_HeadTiltAngle) * pInfo->TiltTrackFactor ;
	if (DeltaAngle > pInfo->MaxTiltSpeed)
		DeltaAngle = pInfo->MaxTiltSpeed ;
	else
	if (DeltaAngle < -pInfo->MaxTiltSpeed)
		DeltaAngle = -pInfo->MaxTiltSpeed ;
	pThis->m_HeadTiltAngle += DeltaAngle * frame_increment ;

	// Limit tilt angle
	if (pThis->m_HeadTiltAngle > pInfo->MaxTiltAngle)
		pThis->m_HeadTiltAngle = pInfo->MaxTiltAngle ;
	else
	if (pThis->m_HeadTiltAngle < -pInfo->MaxTiltAngle)
		pThis->m_HeadTiltAngle = -pInfo->MaxTiltAngle ;
#else

	pThis->m_HeadTiltAngle += ANGLE_DTOR(2) ;
//	pThis->m_HeadTurnAngle += ANGLE_DTOR(2) ;

#endif
}





/*****************************************************************************
*
*	Function:		Turret_TransformPos()
*
******************************************************************************
*
*	Description:	Tranforms a local position (relative to object position)
*						into a world position. (This function is used to correctly
*						place events)
*
*	Inputs:			*pThis	-	Ptr to turret game object instance
*						*pvPos	-	Ptr to local position
*						*pvWorld	-	Ptr to store world position
*						*pInfo	-	Ptr to turret tracking info
*
*	Outputs:			None
*
*****************************************************************************/
void Turret_TransformPos(CGameObjectInstance *pThis, CVector3 *pvPos, CVector3 *pvWorld, CTurretTrackInfo *pInfo)
{
	CMtxF		mfMatrix ;

	// Initialise matrix
	CMtxF__Scale(mfMatrix, pThis->m_vScale.x, pThis->m_vScale.y, pThis->m_vScale.z) ;

	// Translate relative to pivot origin
	CMtxF__PostMultTranslate(mfMatrix, -pInfo->vPivotOffset.x,
												  -pInfo->vPivotOffset.y,
												  -pInfo->vPivotOffset.z) ;

	// Rotate pt around X
	CMtxF__PostMultRotateX(mfMatrix, -pThis->m_HeadTiltAngle) ;

	// Rotate pt around Y
	CMtxF__PostMultRotateY(mfMatrix, pThis->m_HeadTurnAngle) ;

	// Translate back to pivot and world
	CMtxF__PostMultTranslate(mfMatrix, pThis->ah.ih.m_vPos.x + pInfo->vPivotOffset.x,
												  pThis->ah.ih.m_vPos.y + pInfo->vPivotOffset.y,
												  pThis->ah.ih.m_vPos.z + pInfo->vPivotOffset.z) ;

	// Finally convert 3ds to u64
	CMtxF__PreMult3DSToU64(mfMatrix) ;

	// Transform from local position to world position
	CMtxF__VectorMult(mfMatrix, pvPos, pvWorld) ;
}







/////////////////////////////////////////////////////////////////////////////
//
//
//	Ceiling Turret
//
//
/////////////////////////////////////////////////////////////////////////////

// Tracking info
CTurretTrackInfo	CeilingTurretTrackInfo =
{
	{0*SCALING_FACTOR, 17*SCALING_FACTOR, 0*SCALING_FACTOR},	// Pivot offset, relative to pos

	1.0 / 4.0,								//	Turn track factor
	ANGLE_DTOR(3),				  			// Max turn speed

	1.0 / 4.0, 								// Up/down track factor (bigger = slower)
	ANGLE_DTOR(5),							// Max up/down rotation tracking speed

	ANGLE_DTOR(-80),						// Min looking up angle
	ANGLE_DTOR(80)							// Max looking down angle
} ;



// Transforms local position to world position
void CeilingTurret_TransformPos(CGameObjectInstance *pThis, CVector3 *pvPos, CVector3 *pvWorld)
{
	Turret_TransformPos(pThis, pvPos, pvWorld, &CeilingTurretTrackInfo) ;
}

// Uses Ian's AI, but overides rotations with sexual tracking
void CeilingTurretAI_Advance(CGameObjectInstance *pThis)
{
	// Call normal AI
	AI_Advance(pThis) ;

	// Turrets can olnmy attack player!
	pThis->m_pAttackerCGameObjectInstance = CEngineApp__GetPlayer(GetApp()) ;

	// Overide direction
	Turret_TrackPlayer(pThis, &CeilingTurretTrackInfo) ;
}



/////////////////////////////////////////////////////////////////////////////
//
//	Bunker Turret
//
/////////////////////////////////////////////////////////////////////////////

// Tracking info
CTurretTrackInfo	BunkerTurretTrackInfo =
{
	{0*SCALING_FACTOR, 5.5*SCALING_FACTOR, 0*SCALING_FACTOR},	// Pivot offset, relative to pos

	1.0 / 4.0,								//	Turn track factor
	ANGLE_DTOR(3),				  			// Max turn speed

	1.0 / 4.0,								// Up/down track factor (bigger = slower)
	ANGLE_DTOR(5),							// Max up/down rotation tracking speed

	ANGLE_DTOR(-45),						// Min looking up angle
	ANGLE_DTOR(45)							// Max looking down angle
} ;


// Transforms local position to world position
void BunkerTurret_TransformPos(CGameObjectInstance *pThis, CVector3 *pvPos, CVector3 *pvWorld)
{
	Turret_TransformPos(pThis, pvPos, pvWorld, &BunkerTurretTrackInfo) ;
}

// Uses Ian's AI, but overides rotations with sexual tracking
void BunkerTurretAI_Advance(CGameObjectInstance *pThis)
{
	// Call normal AI
	AI_Advance(pThis) ;

	// Turrets can olnmy attack player!
	pThis->m_pAttackerCGameObjectInstance = CEngineApp__GetPlayer(GetApp()) ;

	// Overide direction
	Turret_TrackPlayer(pThis, &BunkerTurretTrackInfo) ;
}


