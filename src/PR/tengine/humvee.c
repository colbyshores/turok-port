// Humveeer boss control file by Stephen Broumley

#include "stdafx.h"

#include "ai.h"
#include "aistand.h"

#include "romstruc.h"
#include "tengine.h"
#include "scaling.h"
#include "audio.h"
#include "sfx.h"
#include "tmove.h"
#include "humvee.h"
#include "boss.h"
#include "bossflgs.h"
#include "onscrn.h"
#include "dlists.h"
#include "cammodes.h"


/////////////////////////////////////////////////////////////////////////////
// General Constants
/////////////////////////////////////////////////////////////////////////////
#define	MAKE_HUMVEE_START_RIGHT_AWAY	0	// Skips cinema

#define	DEBUG_HUMVEE					0
#define	HUMVEE_HEALTH					750



/////////////////////////////////////////////////////////////////////////////
// Movement constants
/////////////////////////////////////////////////////////////////////////////

#define	MAX_HUMVEES						2

#define	HUMVEE_GRAVITY					GRAVITY_ACCELERATION

#define	HUMVEE_FIRE_ROCKET_MAX_DIST		(200*SCALING_FACTOR)
#define	HUMVEE_FIRE_ROCKET_MIN_DIST		(50*SCALING_FACTOR)

#define	HUMVEE_FIRE_GUN_MAX_DIST			(125*SCALING_FACTOR)
#define	HUMVEE_FIRE_GUN_MIN_DIST			(25*SCALING_FACTOR)


#define	HUMVEE_TUROK_TARGET_OFFSET	(0*SCALING_FACTOR)

#define	HUMVEE_TURN_SPEED 			ANGLE_DTOR(5)
#define	HUMVEE_TURN90_ANGLE 			ANGLE_DTOR(90)
#define	HUMVEE_TURN180_ANGLE			ANGLE_DTOR(160)
#define	HUMVEE_FACING_ANGLE 			ANGLE_DTOR(25)
#define	HUMVEE_MAX_WHEEL_ANGLE		ANGLE_DTOR(45)

#define	HUMVEE_ARENA_RADIUS			(170*SCALING_FACTOR)

#define	HUMVEE_INNER_TARGET_DIST 	(60*SCALING_FACTOR)
#define	HUMVEE_OUTER_TARGET_DIST 	(HUMVEE_ARENA_RADIUS - (60*SCALING_FACTOR))
#define	HUMVEE_AT_TARGET_DIST	 	(20*SCALING_FACTOR)

#define	HUMVEE_FIRE_ROCKET_DIST		(80*SCALING_FACTOR)



/////////////////////////////////////////////////////////////////////////////
// Anim constants
/////////////////////////////////////////////////////////////////////////////

// Movement modes

#define AI_ANIM_BOSS_HUMVEE_WAIT_FOR_START		AI_ANIM_IDLE
#define AI_ANIM_BOSS_HUMVEE_JUMP_INTO_ARENA		AI_ANIM_TELEPORT_APPEAR

#define AI_ANIM_BOSS_HUMVEE_IDLE					AI_ANIM_IDLE
#define AI_ANIM_BOSS_HUMVEE_START				AI_ANIM_WALK
#define AI_ANIM_BOSS_HUMVEE_RUN					AI_ANIM_RUN


#define AI_ANIM_BOSS_HUMVEE_TURN_LEFT90		AI_ANIM_TURNL90
#define AI_ANIM_BOSS_HUMVEE_TURN_RIGHT90		AI_ANIM_TURNR90
#define AI_ANIM_BOSS_HUMVEE_TURN180				AI_ANIM_TURN180

#define AI_ANIM_BOSS_HUMVEE_STOP					AI_ANIM_RTURN180

// Hurt modes
#define AI_ANIM_BOSS_HUMVEE_EXPLODE_LEFT		AI_ANIM_IMPACT_EXPLODE_LEFT
#define AI_ANIM_BOSS_HUMVEE_EXPLODE_RIGHT		AI_ANIM_IMPACT_EXPLODE_RIGHT
#define AI_ANIM_BOSS_HUMVEE_DEATH				AI_ANIM_DEATH_NORMAL




/////////////////////////////////////////////////////////////////////////////
// Initialisation Function prototypes
/////////////////////////////////////////////////////////////////////////////

CBoss *Humvee_Initialise(CGameObjectInstance *pThis) ;
void CCollisionInfo__SetHumveeEnterCollisionDefaults(CCollisionInfo2 *pThis) ;
void CCollisionInfo__SetHumveeRunCollisionDefaults(CCollisionInfo2 *pThis) ;
void Humvee_SetupStageInfo(CGameObjectInstance *pThis, CHumvee *pHumvee, CHumveeStageInfo *Info) ;




/////////////////////////////////////////////////////////////////////////////
// General Function prototypes
/////////////////////////////////////////////////////////////////////////////

void Humvee_Update(CGameObjectInstance *pThis, CHumvee *pHumvee) ;
INT32 Humvee_SetupMode(CGameObjectInstance *pThis, INT32 nNewMode) ;
void Humvee_CollisionFunction(CGameObjectInstance *pThis, CCollisionInfo2 *pCollInfo) ;
void Humvee_GetTarget(CGameObjectInstance *pThis, CHumvee *pHumvee) ;
BOOL Humvee_CheckHitByExplosion(CGameObjectInstance *pThis, CHumvee *pHumvee) ;

void CFireWeaponState__Construct(CFireWeaponState *pThis, CFireWeaponInfo *pInfo,
										   CGameObjectInstance *pObject,
										   BossBooleanFunction pCanFireFunction,
										   BossFunction pFireFunction) ;

void CFireWeaponState__Update(CFireWeaponState *pThis, CFireWeaponInfo *pInfo) ;

void Humvee_FireRockets(CGameObjectInstance *pThis, CHumvee *pHumvee) ;
BOOL Humvee_CanFireRockets(CGameObjectInstance *pThis, CHumvee *pHumvee) ;
void Humvee_FireGuns(CGameObjectInstance *pThis, CHumvee *pHumvee) ;
BOOL Humvee_CanFireGuns(CGameObjectInstance *pThis, CHumvee *pHumvee) ;







/////////////////////////////////////////////////////////////////////////////
// Mode Function prototypes
/////////////////////////////////////////////////////////////////////////////
void Humvee_Code_WAIT_FOR_START(CGameObjectInstance *pThis, CHumvee *pHumvee) ;
void Humvee_Setup_JUMP_INTO_ARENA(CGameObjectInstance *pThis, CHumvee *pHumvee) ;
void Humvee_Code_JUMP_INTO_ARENA(CGameObjectInstance *pThis, CHumvee *pHumvee) ;
void Humvee_Code_IDLE(CGameObjectInstance *pThis, CHumvee *pHumvee) ;
void Humvee_Code_Movement(CGameObjectInstance *pThis, CHumvee *pHumvee) ;
void Humvee_Code_STOP(CGameObjectInstance *pThis, CHumvee *pHumvee) ;
void Humvee_Code_Explode(CGameObjectInstance *pThis, CHumvee *pHumvee) ;
void Humvee_Setup_DEATH(CGameObjectInstance *pThis, CHumvee *pHumvee) ;

void Humvee_UpdateSound(CGameObjectInstance *pThis, CHumvee *pHumvee) ;
void Humvee_StopSound(CGameObjectInstance *pThis, CHumvee *pHumvee) ;

void Humvee_PreDraw(CGameObjectInstance *pThis, CHumvee *pHumvee, Gfx **ppDLP) ;
void Humvee_PostDraw(CGameObjectInstance *pThis, CHumvee *pHumvee, Gfx **ppDLP) ;


#ifndef MAKE_CART
void Humvee_DisplayMode(CBoss *pBoss) ;
#endif


/////////////////////////////////////////////////////////////////////////////
// Variables
/////////////////////////////////////////////////////////////////////////////


// Humvees
CHumvee	HumveeBossList[MAX_HUMVEES] ;


/////////////////////////////////////////////////////////////////////////////
//
// The Top Level AI Mode Table
//
/////////////////////////////////////////////////////////////////////////////
CBossModeInfo	HumveeModeTable[] =
{
	// Movement modes
	BOSS_MODE_INFO(NULL,									Humvee_Code_WAIT_FOR_START,	AI_ANIM_BOSS_HUMVEE_WAIT_FOR_START,		1.2,BF_ENTER_MODE),
	BOSS_MODE_INFO(Humvee_Setup_JUMP_INTO_ARENA,	Humvee_Code_JUMP_INTO_ARENA,	AI_ANIM_BOSS_HUMVEE_JUMP_INTO_ARENA,	1.5,BF_ENTER_MODE),

	BOSS_MODE_INFO(NULL,							Humvee_Code_IDLE,			AI_ANIM_BOSS_HUMVEE_IDLE,	1, BF_LASER_ON),

	BOSS_MODE_INFO(Humvee_Code_Movement,	Humvee_Code_Movement,	AI_ANIM_BOSS_HUMVEE_RUN,				1,	BF_CAN_FIRE | BF_LASER_ON),
	BOSS_MODE_INFO(NULL,							Humvee_Code_Movement,	AI_ANIM_BOSS_HUMVEE_TURN_LEFT90,		1,	BF_CAN_FIRE | BF_LASER_ON),
	BOSS_MODE_INFO(NULL,							Humvee_Code_Movement,	AI_ANIM_BOSS_HUMVEE_TURN_RIGHT90,	1,	BF_CAN_FIRE | BF_LASER_ON),
	BOSS_MODE_INFO(NULL,							Humvee_Code_Movement,	AI_ANIM_BOSS_HUMVEE_TURN180,			1,	BF_LASER_ON),
	BOSS_MODE_INFO(NULL,							Humvee_Code_STOP,			AI_ANIM_BOSS_HUMVEE_STOP,				1,	BF_LASER_ON),

	// Hurt modes
	BOSS_MODE_INFO(NULL,	Humvee_Code_Explode,	AI_ANIM_BOSS_HUMVEE_EXPLODE_LEFT,	1,	BF_EXPLODE_MODE),
	BOSS_MODE_INFO(NULL,	Humvee_Code_Explode,	AI_ANIM_BOSS_HUMVEE_EXPLODE_RIGHT,	1,	BF_EXPLODE_MODE),
	BOSS_MODE_INFO(Humvee_Setup_DEATH,			NULL,						AI_ANIM_BOSS_HUMVEE_DEATH,				1,	BF_DEATH_MODE | BF_CLEAR_VELS)
} ;





/////////////////////////////////////////////////////////////////////////////
//
// The big stage table
//
/////////////////////////////////////////////////////////////////////////////
CHumveeStageInfo HumveeStageInfo[] =
{
	//	STAGE 1
	{0,										// Health limit for this stage
	 0.5,										// Anim speed
	 1,										// Hits before explode anim

	 {1,										// Rockets to fire in a row
	  SECONDS_TO_FRAMES(0.5),			// Spacing between each rocket
	  SECONDS_TO_FRAMES(6)}, 			// Spacing between firing rockets

	 {24, 									// Gun bullets to fire in a row
	  SECONDS_TO_FRAMES(0.2),				// Spacing between each bullet
	  SECONDS_TO_FRAMES(6)},	   	// Spacing between firing bullets
	},


	//	STAGE 2
	{75,										// Health limit for this stage
	 0.4,										// Anim speed
	 1,										// Hits before explode anim

	 {1,										// Rockets to fire in a row
	  SECONDS_TO_FRAMES(0.25),			// Spacing between each rocket
	  SECONDS_TO_FRAMES(5)},	 		// Spacing between firing rockets

	 {32, 									// Gun bullets to fire in a row
	  SECONDS_TO_FRAMES(0.2),				// Spacing between each bullet
	  SECONDS_TO_FRAMES(5)},	   	// Spacing between firing bullets
	},

	//	STAGE 3
	{50,										// Health limit for this stage
	 0.55,									// Anim speed
	 2,										// Hits before explode anim

	 {1,										// Rockets to fire in a row
	  SECONDS_TO_FRAMES(0.25),			// Spacing between each rocket
	  SECONDS_TO_FRAMES(4)},			// Spacing between firing rockets

	 {40,										// Gun bullets to fire in a row
	  SECONDS_TO_FRAMES(0.2),				// Spacing between each bullet
	  SECONDS_TO_FRAMES(4)},	   	// Spacing between firing bullets
	},

	//	STAGE 4
	{25,										// Health limit for this stage
	 0.60,									// Anim speed
	 3,										// Hits before explode anim

	 {1,										// Rockets to fire in a row
	  SECONDS_TO_FRAMES(0.25),			// Spacing between each rocket
	  SECONDS_TO_FRAMES(3)}, 			// Spacing between firing rockets

	 {48,										// Gun bullets to fire in a row
	  SECONDS_TO_FRAMES(0.2),				// Spacing between each bullet
	  SECONDS_TO_FRAMES(3)},	   	// Spacing between firing bullets
	},

	//	STAGE 5
	{0,										// Health limit for this stage
	 0.65,									// Anim speed
	 4,										// Hits before explode anim

	 {2,										// Rockets to fire in a row
	  SECONDS_TO_FRAMES(0.25),			// Spacing between each rocket
	  SECONDS_TO_FRAMES(2)}, 			// Spacing between firing rockets

	 {48,										// Gun bullets to fire in a row
	  SECONDS_TO_FRAMES(0.2),				// Spacing between each bullet
	  SECONDS_TO_FRAMES(2)},	   	// Spacing between firing bullets
	}
} ;





/*****************************************************************************
*
*
*
*	INITIALISATION CODE
*
*
*
*****************************************************************************/




/*****************************************************************************
*
*	Function:		Humvee_Initialise()
*
******************************************************************************
*
*	Description:	Prepares humvee
*
*	Inputs:			*pThis			-	Ptr to game object instance
*
*	Outputs:			Ptr to humvee boss vars
*
*****************************************************************************/
CBoss *Humvee_Initialise(CGameObjectInstance *pThis)
{
	// Allocate humvee!
	CHumvee *pHumvee = &HumveeBossList[AI_GetEA(pThis)->m_Id] ;

	// Setup defaults
	CBoss__Construct(&pHumvee->m_Boss) ;

	// Setup boss vars
	pHumvee->m_Boss.m_pUpdateFunction = (void *)Humvee_Update ;
	pHumvee->m_Boss.m_pModeTable = HumveeModeTable ;

	pHumvee->m_Boss.m_Rot1 = 0 ;
	pHumvee->m_Boss.m_Rot2 = 0 ;
	pHumvee->m_Boss.m_Rot3 = 0 ;
	pHumvee->m_Boss.m_Rot4 = 0 ;
	pHumvee->m_Boss.m_Rot5 = 0 ;

	pHumvee->m_WheelVel = 0 ;
	pHumvee->m_WheelAngle = 0 ;

	// Setup humvee vars
	pHumvee->m_pStageInfo = HumveeStageInfo ;
	Humvee_SetupStageInfo(pThis, pHumvee, HumveeStageInfo) ;

	// Make boss invincible for now
	AI_GetDyn(pThis)->m_Invincible = TRUE ;

	// Put into start mode
	pHumvee->m_Boss.m_Mode = HUMVEE_WAIT_FOR_START_MODE ;
	pHumvee->m_Boss.m_ModeAnimSpeed = 0 ;

	pHumvee->m_Boss.m_AnimRepeats = 1 ;
	pHumvee->m_Boss.m_AnimSpeed = 1.0 ;

	pHumvee->m_Boss.m_pPreDrawFunction = (void *)Humvee_PreDraw ;
	pHumvee->m_Boss.m_pPostDrawFunction = (void *)Humvee_PostDraw ;

	pHumvee->m_Boss.m_pPauseFunction = (void *)Humvee_UpdateSound ;


#ifndef MAKE_CART
	pHumvee->m_Boss.m_pDisplayModeFunction = (void *)Humvee_DisplayMode ;
#endif

	// Aim for Turok to begin with - fixes running into wall bug
	pHumvee->m_TargetNumber = 1 ;


	pHumvee->m_ExplosionHits = 0 ;


	pThis->m_pCollisionFunction = (void *)Humvee_CollisionFunction ;
	pHumvee->m_CollisionTime = 0 ;

#if DEBUG_HUMVEE
	rmonPrintf("\r\n\r\n\r\nHumvee:%d - ", AI_GetEA(pThis)->m_Id) ;

	rmonPrintf("HUMVEE HERE WE GO!!\r\n") ;
#endif

	// Setup misc
	CCollisionInfo__SetHumveeEnterCollisionDefaults(&pHumvee->m_Boss.m_CollisionInfo) ;

	BOSS_Health(pThis) = HUMVEE_HEALTH ;

	pHumvee->m_LightGlareTime = 0 ;

	pHumvee->m_Sound = -1 ;
	pHumvee->m_SoundHandle = -1 ;

	// Prepare guns
	CFireWeaponState__Construct(&pHumvee->m_GunsState,
									    &pHumvee->m_pStageInfo->FireGunsInfo,
									    pThis,
									    (BossBooleanFunction)Humvee_CanFireGuns,
									    (BossFunction)Humvee_FireGuns) ;

	// Prepare rockets
	CFireWeaponState__Construct(&pHumvee->m_RocketsState,
									    &pHumvee->m_pStageInfo->FireRocketsInfo,
									    pThis,
									    (BossBooleanFunction)Humvee_CanFireRockets,
									    (BossFunction)Humvee_FireRockets) ;

	// Return pointer to humvee boss
	return (CBoss *)pHumvee ;
}



/*****************************************************************************
*
*	Function:		CCollisionInfo__SetHumveeCollisionDefaults()
*
******************************************************************************
*
*	Description:	Prepares humvee collision structure
*
*	Inputs:			*pThis	-	Ptr to collision structure
*
*	Outputs:			None
*
*****************************************************************************/
void CCollisionInfo__SetHumveeEnterCollisionDefaults(CCollisionInfo2 *pThis)
{
	pThis->dwFlags =		COLLISIONFLAG_PHYSICS
							|	COLLISIONFLAG_EXITWATER
							|	COLLISIONFLAG_WATERCOLLISION
							|	COLLISIONFLAG_IGNOREDEAD
							|	COLLISIONFLAG_CLIMBUP
							|	COLLISIONFLAG_CLIMBDOWN
							|	COLLISIONFLAG_ENTERWATER
							|	COLLISIONFLAG_EXITWATER ;

	pThis->InstanceBehavior			= INTERSECT_BEHAVIOR_IGNORE;
	pThis->WallBehavior				= INTERSECT_BEHAVIOR_SLIDE ;
	pThis->GroundBehavior			= INTERSECT_BEHAVIOR_SLIDE ;
	pThis->GravityAcceleration		= HUMVEE_GRAVITY;
	pThis->BounceReturnEnergy		= 0.3;
	pThis->GroundFriction			= 0.0;
	pThis->AirFriction				= 0.0;
	pThis->WaterFriction				= 0.0;
}


void CCollisionInfo__SetHumveeRunCollisionDefaults(CCollisionInfo2 *pThis)
{
	pThis->dwFlags =		COLLISIONFLAG_PHYSICS
							|	COLLISIONFLAG_EXITWATER
							|	COLLISIONFLAG_USEWALLRADIUS
							|	COLLISIONFLAG_TRACKGROUND
							|	COLLISIONFLAG_CLIMBDOWN
							|	COLLISIONFLAG_WATERCOLLISION
							|	COLLISIONFLAG_IGNOREDEAD ;

	pThis->InstanceBehavior			= INTERSECT_BEHAVIOR_SLIDE;
	pThis->WallBehavior				= INTERSECT_BEHAVIOR_SLIDE;
	pThis->GroundBehavior			= INTERSECT_BEHAVIOR_SLIDE;
	pThis->GravityAcceleration		= HUMVEE_GRAVITY;
	pThis->BounceReturnEnergy		= 0.3;
	pThis->GroundFriction			= 0.7;
	pThis->AirFriction				= 0.0;
	pThis->WaterFriction				= 0.0;
}



/*****************************************************************************
*
*	Function:		Humvee_SetupStageInfo()
*
******************************************************************************
*
*	Description:	Prepares new stage info
*
*	Inputs:			*pThis	-	Ptr to game object instance
*						*pHumvee	-	Ptr to humvee boss vars
*						*Info		-	Ptr to stage info
*
*	Outputs:			None
*
*****************************************************************************/
void Humvee_SetupStageInfo(CGameObjectInstance *pThis, CHumvee *pHumvee, CHumveeStageInfo *Info)
{
}




/*****************************************************************************
*
*	Function:		Humvee_PreDraw()
*
******************************************************************************
*
*	Description:	Turns lights on and off
*
*	Inputs:			*pThis	-	Ptr to game object instance
*						*pHumvee	-	Ptr to boss vars
*
*	Outputs:			None
*
*****************************************************************************/
void Humvee_PreDraw(CGameObjectInstance *pThis, CHumvee *pHumvee, Gfx **ppDLP)
{
	if (!(pHumvee->m_Boss.m_ModeFlags & BF_LASER_ON))
	{
		draw_transparent_geometry = FALSE ;
		draw_intersect_geometry = FALSE ;
	}
}

/*****************************************************************************
*
*	Function:		Humvee_PostDraw()
*
******************************************************************************
*
*	Description:	Resets draw mode so special fx don't affect next objects
*
*	Inputs:			*pThis	-	Ptr to game object instance
*						*pHumvee	-	Ptr to boss vars
*
*	Outputs:			None
*
*****************************************************************************/
void Humvee_PostDraw(CGameObjectInstance *pThis, CHumvee *pHumvee, Gfx **ppDLP)
{
	draw_transparent_geometry = TRUE ;
	draw_intersect_geometry = TRUE ;
}




/*****************************************************************************
*
*
*
*	GENERAL CODE
*
*
*
*****************************************************************************/



void Humvee_LightGlare(CGameObjectInstance *pThis, CHumvee *pHumvee)
{
	CGameObjectInstance	*pTurok = CEngineApp__GetPlayer(GetApp()) ;

	// Perform head light glare?
	if ((pHumvee->m_Boss.m_ModeFlags & BF_LASER_ON) &&
		 (pHumvee->m_LightGlareTime == 0) &&
		 (pHumvee->m_Boss.m_DistFromTurok < (150*SCALING_FACTOR)) &&
		 (AngleDist(pTurok->m_RotY + ANGLE_DTOR(180), pThis->m_RotY) < ANGLE_DTOR(60)) &&
		 (((pHumvee->m_Boss.m_LastTurokDeltaAngle > 0) && (pHumvee->m_Boss.m_TurokDeltaAngle < 0)) ||
		  ((pHumvee->m_Boss.m_LastTurokDeltaAngle < 0) && (pHumvee->m_Boss.m_TurokDeltaAngle > 0))))
	{
			pHumvee->m_LightGlareTime = SECONDS_TO_FRAMES(1) ;
			CEngineApp__DoColorFlash(GetApp(), 255, 64, 255,255,255) ;
	}

	// Decrease glare timer
	pHumvee->m_LightGlareTime -= frame_increment ;
	if (pHumvee->m_LightGlareTime < 0)
		pHumvee->m_LightGlareTime = 0 ;
}



void Humvee_UpdateWheels(CGameObjectInstance *pThis, CHumvee *pHumvee)
{
	FLOAT			Speed, VelAngle, Diff ;

	// Moving?
	VelAngle = CVector3__XZAngle(&pHumvee->m_Boss.m_LastVel) ;
	Diff = AngleDist(VelAngle, pThis->m_RotY) ;

	// Go backwards or forwards?
	if (BOSS_Health(pThis))
	{
		if (Diff < ANGLE_DTOR(90))
			Speed = -ANGLE_DTOR(pHumvee->m_Boss.m_Speed*4) ;	// Forwards
		else
			Speed = ANGLE_DTOR(pHumvee->m_Boss.m_Speed*4) ;		// Backwards

		// Zoom to speed
		pHumvee->m_WheelVel += (Speed - pHumvee->m_WheelVel) / 2 ;
	}
	else
	{
		// SHM
		pHumvee->m_WheelVel -= pHumvee->m_Boss.m_Rot1 / (1<<13) ;

		// Dampen
		pHumvee->m_Boss.m_Rot1 -= pHumvee->m_Boss.m_Rot1 / (1<<9) ;
	}

	// Increase rotation of wheels
	pHumvee->m_Boss.m_Rot1 += pHumvee->m_WheelVel ;
	pHumvee->m_Boss.m_Rot2 += pHumvee->m_WheelVel ;
	pHumvee->m_Boss.m_Rot3 += pHumvee->m_WheelVel ;
	pHumvee->m_Boss.m_Rot4 += pHumvee->m_WheelVel ;


	// Set wheel angle
	if (pHumvee->m_Boss.m_Mode == HUMVEE_RUN_MODE)
	{
		// Set wheel angle
		pHumvee->m_WheelAngle = -pHumvee->m_Boss.m_DeltaAngle ;
		if (pHumvee->m_WheelAngle > HUMVEE_MAX_WHEEL_ANGLE)
			pHumvee->m_WheelAngle = HUMVEE_MAX_WHEEL_ANGLE ;
		else
		if (pHumvee->m_WheelAngle < -HUMVEE_MAX_WHEEL_ANGLE)
			pHumvee->m_WheelAngle = -HUMVEE_MAX_WHEEL_ANGLE ;
	}
	else
		pHumvee->m_WheelAngle = 0 ;


	// Zoom to wheel angle
	pHumvee->m_Boss.m_Rot5 += (pHumvee->m_WheelAngle - pHumvee->m_Boss.m_Rot5) / 8 ;
}







/*****************************************************************************
*
*	Function:		Humvee_Update()
*
******************************************************************************
*
*	Description:	Top level ai update - moves humvee around to do its stuff
*
*	Inputs:			*pThis	-	Ptr to game object instance
*						*pHumvee	-	Ptr to humvee boss vars
*
*	Outputs:			None
*
*****************************************************************************/
void Humvee_Update(CGameObjectInstance *pThis, CHumvee *pHumvee)
{
	// Update special fx
	Humvee_LightGlare(pThis, pHumvee) ;
	Humvee_UpdateWheels(pThis, pHumvee) ;

	// Update sound
	Humvee_UpdateSound(pThis, pHumvee) ;

	// Check for explosion hits
	if (BOSS_Health(pThis) > 0)
		Humvee_CheckHitByExplosion(pThis, pHumvee) ;

	// No interruptions on blends!
	if (CGameObjectInstance__IsBlending(pThis))
		return ;

	// Decrease collision timer
	pHumvee->m_CollisionTime -= frame_increment ;
	if (pHumvee->m_CollisionTime < 0)
		pHumvee->m_CollisionTime = 0 ;

	// Check for death
	if ((BOSS_Health(pThis) <= 0) && (pHumvee->m_Boss.m_Mode != HUMVEE_DEATH_MODE))
	{
		// Start SHM going on wheels
		pHumvee->m_Boss.m_Rot1 = 0 ;
		pHumvee->m_WheelVel = ANGLE_DTOR(10) ;

		Humvee_SetupMode(pThis, HUMVEE_DEATH_MODE) ;

		// Start humvee 2?
		if (AI_GetEA(pThis)->m_Id == 0)
		{
			GetApp()->m_BossFlags |= BOSS_FLAG_HUMVEE1_DEAD | BOSS_FLAG_LONGHUNTER_START_HUMVEE2 ;
			GetApp()->m_BossVars.m_StartBossTimer = 0 ;
		}
		else
		{
			GetApp()->m_BossFlags |= BOSS_FLAG_HUMVEE2_DEAD | BOSS_FLAG_LONGHUNTER_START_LONGHUNTER ;
			GetApp()->m_BossVars.m_StartBossTimer = 0 ;
		}
	}

	// Update weapons
	CFireWeaponState__Update(&pHumvee->m_GunsState, &pHumvee->m_pStageInfo->FireGunsInfo) ;
	CFireWeaponState__Update(&pHumvee->m_RocketsState, &pHumvee->m_pStageInfo->FireRocketsInfo) ;

	// Calculate target and look angle
	Humvee_GetTarget(pThis, pHumvee) ;

	// Call update function if there is one
	if ((pHumvee->m_Boss.m_Mode >= 0 ) &&
		 (pHumvee->m_Boss.m_pModeTable[pHumvee->m_Boss.m_Mode].m_pUpdateFunction))
		 (pHumvee->m_Boss.m_pModeTable[pHumvee->m_Boss.m_Mode].m_pUpdateFunction)(pThis, (CBoss *)pHumvee) ;

	// Check for updating stage
	if (pHumvee->m_Boss.m_PercentageHealth < pHumvee->m_pStageInfo->HealthLimit)
	{
		Humvee_SetupStageInfo(pThis, pHumvee, ++pHumvee->m_pStageInfo) ;
#if DEBUG_HUMVEE
			rmonPrintf("Humvee:%d - ", AI_GetEA(pThis)->m_Id) ;
			rmonPrintf("\r\n....NEXT ATTACK STAGE....\r\n") ;
#endif
	}

	// Update anim speed
	switch(pHumvee->m_Boss.m_Mode)
	{
		case HUMVEE_WAIT_FOR_START_MODE:
		case HUMVEE_JUMP_INTO_ARENA_MODE:
			break ;

		default:
			pHumvee->m_Boss.m_AnimSpeed = pHumvee->m_pStageInfo->AnimSpeed ;
	}
}




/*****************************************************************************
*
*	Function:		Humvee_Setup()
*
******************************************************************************
*
*	Description:	Sets up a new humvee mode
*
*	Inputs:			*pThis	-	Ptr to game object instance
*						nNewMode	-	Mode id
*
*	Outputs:			TRUE if new mode was activated, else FALSE
*
*****************************************************************************/
INT32 Humvee_SetupMode(CGameObjectInstance *pThis, INT32 nNewMode)
{
	BOOL	Result ;

	// Call normal boss setup
	Result = BOSS_SetupMode(pThis, nNewMode) ;

	return Result ;
}






/*****************************************************************************
*
*	Function:		Humvee_CollisionFunction()
*
******************************************************************************
*
*	Description:	Function for when humvee hits another instance. Only reacts
*						to hitting player. Player will recieve damage if hit.
*
*	Inputs:			*pThis		-	Ptr to game object instance
*						*pCollInfo	-	Ptr to collision info used in collision
*
*	Outputs:			None
*
*****************************************************************************/
void Humvee_CollisionFunction(CGameObjectInstance *pThis, CCollisionInfo2 *pCollInfo)
{
	CHumvee		*pHumvee = (CHumvee *)pThis->m_pBoss ;
	CGameObjectInstance *pTurok = CEngineApp__GetPlayer(GetApp()) ;

	// Hit Turok?
	if (((CGameObjectInstance *)pCollInfo->pInstanceCollision == pTurok) &&
		 (TUROK_Health(pTurok) > 0) &&
		 (pHumvee->m_Boss.m_Mode != HUMVEE_EXPLODE_LEFT_MODE) &&
		 (pHumvee->m_Boss.m_Mode != HUMVEE_EXPLODE_RIGHT_MODE) &&
		 (pHumvee->m_Boss.m_Mode != HUMVEE_DEATH_MODE))
	{

#if DEBUG_HUMVEE
		rmonPrintf("Humvee:%d - ", AI_GetEA(pThis)->m_Id) ;
		rmonPrintf("Collision Speed:%f\r\n", pHumvee->m_Boss.m_Speed/SCALING_FACTOR) ;
#endif

		// Going fast enough and collision timer done?
		if ((pHumvee->m_Boss.m_Speed > (1.0*SCALING_FACTOR)) && (pHumvee->m_CollisionTime == 0))
		{
			// Temp impact sound for now
			AI_DoSound(&pThis->ah.ih, 466, 1, 0) ;

			AI_Decrease_Health(pTurok, 10, TRUE, TRUE) ;
			pHumvee->m_CollisionTime = SECONDS_TO_FRAMES(2) ;


			// shove turok
			AI_MEvent_ShoveWithCamera(pTurok, &pThis->ah.ih, pThis->ah.ih.m_vPos, 100) ;
			CCamera__SetShakeX(&GetApp()->m_Camera,5) ;
			CCamera__SetShakeY(&GetApp()->m_Camera,5) ;
			CCamera__SetShakeZ(&GetApp()->m_Camera,5) ;
		}
	}
}






/*****************************************************************************
*
*	Function:		Humvee_GetTarget()
*
******************************************************************************
*
*	Description:	Calculates humvees target to aim for. It toggles between
*						player and edge of arena (pt is calculated by taking the
*						line between the player and the middle of arena).
*
*	Inputs:			*pThis	-	Ptr to game object instance
*						*pHumvee	-	Ptr to humvee boss vars
*
*	Outputs:			None
*
*****************************************************************************/
void Humvee_GetTarget(CGameObjectInstance *pThis, CHumvee *pHumvee)
{
	CGameObjectInstance *pTurok ;
	CVector3		vDir, vTurokPos ;
	FLOAT			DistFromCentre, rot ;
	BOOL			SetNewTarget ;


	// Calculate distance from centre of arena
	pHumvee->m_DistFromCentre = (BOSS_GetPos(pThis).x * BOSS_GetPos(pThis).x) +
										 (BOSS_GetPos(pThis).z * BOSS_GetPos(pThis).z) ;

	if (pHumvee->m_DistFromCentre)
		pHumvee->m_DistFromCentre = sqrt(pHumvee->m_DistFromCentre) ;
	else
		pHumvee->m_DistFromCentre = 0 ;


	// Swap targets
	if (pHumvee->m_Boss.m_DistFromTarget < RandomRangeFLOAT(20*SCALING_FACTOR, 40*SCALING_FACTOR))
	{
		// Next target
		pHumvee->m_TargetNumber++ ;
		if (pHumvee->m_TargetNumber > 1)
			pHumvee->m_TargetNumber = 0 ;


#if DEBUG_HUMVEE
		if (BOSS_Health(pThis) > 0)
		{
			if (pHumvee->m_TargetNumber)
			{
				rmonPrintf("\r\nHumvee:%d - ", AI_GetEA(pThis)->m_Id) ;
				rmonPrintf("Going to run down Turok!\r\n") ;
			}
			else
			{
				rmonPrintf("\r\nHumvee:%d - ", AI_GetEA(pThis)->m_Id) ;
				rmonPrintf("Going away for a little drive...\r\n") ;
			}
		}
#endif

		SetNewTarget = TRUE ;
	}
	else
		SetNewTarget = FALSE ;


	// Setup Turok pos
	pTurok = CEngineApp__GetPlayer(GetApp());
	vTurokPos = TUROK_GetPos(pTurok) ;

	// Calculate direction towards centre of arena
	vDir = vTurokPos ;
	vDir.y = 0 ;
	CVector3__Normalize(&vDir) ;


	// Which target?
	switch(pHumvee->m_TargetNumber)
	{

		// Go to middle or outside of arena
		case 0:
			if (SetNewTarget)
			{
				// Calculate Turok distance from centre of arena
				if (DistFromCentre = (vTurokPos.x * vTurokPos.x) +	(vTurokPos.z * vTurokPos.z))
					DistFromCentre = sqrt(DistFromCentre) ;

#define	HUMVEE_INNER_TARGET_DIST 	(60*SCALING_FACTOR)
#define	HUMVEE_OUTER_TARGET_DIST 	(HUMVEE_ARENA_RADIUS - (60*SCALING_FACTOR))


				// Use outer or inner?
				if (DistFromCentre > (HUMVEE_ARENA_RADIUS / 2))
				{
					vDir.x *= HUMVEE_INNER_TARGET_DIST ;
					vDir.z *= HUMVEE_INNER_TARGET_DIST ;
				}
				else
				{
					vDir.x *= HUMVEE_OUTER_TARGET_DIST ;
					vDir.z *= HUMVEE_OUTER_TARGET_DIST ;
				}
			}
			else
				vDir = pHumvee->m_Boss.m_vTarget ;

			break ;


		// Go for Turok
		case 1:
			rot = pTurok->m_RotY ;
			vDir.x = vTurokPos.x - (HUMVEE_TUROK_TARGET_OFFSET * sin(rot)) ;
			vDir.z = vTurokPos.z - (HUMVEE_TUROK_TARGET_OFFSET * cos(rot)) ;
			break ;
	}


	// Calculate distance from this target
	pHumvee->m_Boss.m_vTarget = vDir ;
	pHumvee->m_Boss.m_DistFromTargetSqr = AI_DistanceSquared(pThis, vDir) ;
	if (pHumvee->m_Boss.m_DistFromTargetSqr)
		pHumvee->m_Boss.m_DistFromTarget = sqrt(pHumvee->m_Boss.m_DistFromTargetSqr) ;
	else
		pHumvee->m_Boss.m_DistFromTarget = 0 ;


	// Calculate angle to drive at
	pHumvee->m_Boss.m_DeltaAngle =
		AI_GetAvoidanceAngle(pThis, pHumvee->m_Boss.m_vTarget, pTurok, AVOIDANCE_RADIUS_DISTANCE_FACTOR) ;
}






/*****************************************************************************
*
*	Function:		Humvee_CheckHitByExplosion()
*
******************************************************************************
*
*	Description:	Checks to see if the humvee has been hit by an explosion.
*						If humvee is hit so many times (defined in stage info), then
*						an explosion flinch anim is played.
*
*	Inputs:			*pThis	-	Ptr to game object instance
*						*pHumvee	-	Ptr to humvee boss vars
*
*	Outputs:			TRUE if explosion anim was started, else FALSE
*
*****************************************************************************/
INT16 HumveeExplodeModes[] =
{
	0,									// Back
	HUMVEE_EXPLODE_LEFT_MODE,	// Left
	0,									// Front
	HUMVEE_EXPLODE_RIGHT_MODE,	// Right
} ;

BOOL Humvee_CheckHitByExplosion(CGameObjectInstance *pThis, CHumvee *pHumvee)
{
	CGameObjectInstance *pTurok ;
	FLOAT						AngleDiff ;
	INT32						Index, NewMode ;

	// Hit by an explosion?
	if (AI_GetDyn(pThis)->m_dwStatusFlags & AI_EV_EXPLOSION)
	{
		// Clear explosion flag
		AI_GetDyn(pThis)->m_dwStatusFlags &= ~AI_EV_EXPLOSION ;


#if DEBUG_HUMVEE
		rmonPrintf("Humvee:%d - ", AI_GetEA(pThis)->m_Id) ;
		rmonPrintf("\r\n...EXPLOSION HIT!!...\r\n") ;
#endif

		// Been hit enough?
		if (++pHumvee->m_ExplosionHits >= pHumvee->m_pStageInfo->HitsBeforeExplode)
		{
			pHumvee->m_ExplosionHits = 0 ;

			// Decide what to do
			switch(pHumvee->m_Boss.m_Mode)
			{
				case HUMVEE_DEATH_MODE:
					return FALSE ;

				default:

					// Force anim to restart if same mode
					pHumvee->m_Boss.m_OldMode = -1 ;

					pTurok = CEngineApp__GetPlayer(GetApp());
					AngleDiff = (pTurok->m_RotY -  pThis->m_RotY - ANGLE_DTOR(45)) / ANGLE_DTOR(90) ;
					Index = (INT32)AngleDiff ;

					NewMode = HumveeExplodeModes[Index & 3] ;
					if (NewMode)
						Humvee_SetupMode(pThis, NewMode) ;
					else
					if (RANDOM(1))
						Humvee_SetupMode(pThis, HUMVEE_EXPLODE_LEFT_MODE) ;
					else
						Humvee_SetupMode(pThis, HUMVEE_EXPLODE_RIGHT_MODE) ;

					// Give some y vel if on the ground
					if (CInstanceHdr__IsOnGround(&pThis->ah.ih))
						BOSS_GetVel(pThis).y = 2*SCALING_FACTOR*15 ;

					return TRUE ;
			}
		}
	}

	return FALSE ;
}






/*****************************************************************************
*
*	Function:		Humvee_FireRockets()
*
******************************************************************************
*
*	Description:	Makes humvee fire a rocket from the front
*
*	Inputs:			*pThis	-	Ptr to game object instance
*						*pHumvee	-	Ptr to humvee boss vars
*
*	Outputs:			None
*
*****************************************************************************/


void CFireWeaponState__Construct(CFireWeaponState *pThis,
											CFireWeaponInfo *pInfo,
										   CGameObjectInstance *pObject,
										   BossBooleanFunction pCanFireFunction,
										   BossFunction pFireFunction)
{
	pThis->pObject = pObject ;
	pThis->pCanFireFunction = pCanFireFunction ;
	pThis->pFireFunction = pFireFunction ;

	pThis->ShotsLeft = pInfo->TotalShots ;
	pThis->SingleShotTime = 0 ;
	pThis->FireShotsTime = pInfo->FireShotsSpacing ;
}



void CFireWeaponState__Update(CFireWeaponState *pThis, CFireWeaponInfo *pInfo)
{
	// Update spacing
	pThis->FireShotsTime -= frame_increment ;
	if (pThis->FireShotsTime < 0)
	{
		pThis->ShotsLeft = pInfo->TotalShots ;
		pThis->SingleShotTime = 0 ;
		pThis->FireShotsTime = pInfo->FireShotsSpacing ;
	}

	// Update firing?
	if ((pThis->ShotsLeft) && (pThis->pCanFireFunction(pThis->pObject,pThis->pObject->m_pBoss)))
	{
		// Fire a shot?
		pThis->SingleShotTime -= frame_increment ;
		if (pThis->SingleShotTime <= 0)
		{
			pThis->SingleShotTime = pInfo->SingleShotSpacing ;
			pThis->ShotsLeft-- ;
			pThis->pFireFunction(pThis->pObject, pThis->pObject->m_pBoss) ;
		}
	}
}


void Humvee_TransformPos(CGameObjectInstance *pThis, CVector3 *pvPos, CVector3 *pvWorld)
{
	CMtxF			mfBoundsOrient, mfOrient ;
	CQuatern    qRotY, qTemp1, qTemp2 ;

	// Calculate orientation matrix
	CQuatern__BuildFromAxisAngle(&qRotY, 0,1,0, pThis->m_RotY) ;
	CQuatern__Mult(&qTemp1, &pThis->m_qRot, &qRotY) ;
	CQuatern__Mult(&qTemp2, &qTemp1, &pThis->m_qGround) ;
	CQuatern__ToMatrix(&qTemp2, mfBoundsOrient) ;
	CMtxF__PreMultScale(mfBoundsOrient, pThis->m_vScale.x, pThis->m_vScale.y, pThis->m_vScale.z) ;
	CMtxF__PostMultTranslate(mfBoundsOrient, pThis->ah.ih.m_vPos.x, pThis->ah.ih.m_vPos.y, pThis->ah.ih.m_vPos.z) ;
	CMtxF__CopyPreMult3DSToU64(mfOrient, mfBoundsOrient) ;

	// Transform event pos
	CMtxF__VectorMult(mfOrient, pvPos, pvWorld) ;
}



void Humvee_SendEvent(CGameObjectInstance *pThis, CHumvee *pHumvee, INT32 Event, CVector3 *pvPos)
{
	CVector3	vWorld ;

	// Transform event position
	Humvee_TransformPos(pThis, pvPos, &vWorld) ;

//	AI_DoParticle(pThis, PARTICLE_TYPE_SARROWFLASH, vWorld) ;
//	AI_DoParticle(pThis, PARTICLE_TYPE_ENEMY_MUZZLEFLASH, vWorld) ;

	// Send event
	AI_Event_Dispatcher(&pThis->ah.ih, &pThis->ah.ih,
							  Event, AI_SPECIES_ALL,
							  vWorld, 1.0) ;

}

void Humvee_FireRockets(CGameObjectInstance *pThis, CHumvee *pHumvee)
{
	static	CVector3	RocketLeftPos = {-6.347656*SCALING_FACTOR,
												  -1.348877*SCALING_FACTOR,
												  16.601563*SCALING_FACTOR} ;

	static	CVector3	RocketRightPos = {7.861328*SCALING_FACTOR,
												   -1.116943*SCALING_FACTOR,
												   16.601563*SCALING_FACTOR} ;

	// Fire the rockets
	Humvee_SendEvent(pThis, pHumvee, AI_SEVENT_GENERIC124, &RocketLeftPos) ;
	Humvee_SendEvent(pThis, pHumvee, AI_SEVENT_GENERIC121, &RocketRightPos) ;
	AI_DoSound((CInstanceHdr *)pThis, SOUND_MISSILE_LAUNCH, 1, 0) ;
}


BOOL Humvee_CanFireRockets(CGameObjectInstance *pThis, CHumvee *pHumvee)
{
	// Within range?
	return ((pHumvee->m_Boss.m_DistFromTurok <= HUMVEE_FIRE_ROCKET_MAX_DIST) &&
			  (pHumvee->m_Boss.m_DistFromTurok >= HUMVEE_FIRE_ROCKET_MIN_DIST) &&
			  (pHumvee->m_Boss.m_ModeFlags & BF_CAN_FIRE) &&
			  (pHumvee->m_Boss.m_AbsTurokDeltaAngle < ANGLE_DTOR(25))) ;
}


void Humvee_FireGuns(CGameObjectInstance *pThis, CHumvee *pHumvee)
{
	static	CVector3	MachineGunLeftPos = {-7.519531*SCALING_FACTOR,
														-23.144531*SCALING_FACTOR,
														11.621094*SCALING_FACTOR} ;

	static	CVector3	MachineGunRightPos = {7.861328*SCALING_FACTOR,
														 -23.144531*SCALING_FACTOR,
														 11.621094*SCALING_FACTOR} ;

	// Send out bullets
	Humvee_SendEvent(pThis, pHumvee, AI_SEVENT_GENERIC128, &MachineGunLeftPos) ;
	Humvee_SendEvent(pThis, pHumvee, AI_SEVENT_GENERIC128, &MachineGunRightPos) ;

	// Play sound every 4 frames
	if ((game_frame_number & 3) == 3)
		AI_DoSound((CInstanceHdr *)pThis, SOUND_RIFLE_SHOT, 1, 0) ;
//		AI_DoSound((CInstanceHdr *)pThis, SOUND_MACHINE_GUN_SHOT_1, 1, 0) ;
}




BOOL Humvee_CanFireGuns(CGameObjectInstance *pThis, CHumvee *pHumvee)
{
	// Within range?
	return ((pHumvee->m_Boss.m_DistFromTurok <= HUMVEE_FIRE_GUN_MAX_DIST) &&
			  (pHumvee->m_Boss.m_DistFromTurok >= HUMVEE_FIRE_GUN_MIN_DIST) &&
			  (pHumvee->m_Boss.m_ModeFlags & BF_CAN_FIRE) &&
			  (pHumvee->m_Boss.m_AbsTurokDeltaAngle < ANGLE_DTOR(25))) ;
}



/*****************************************************************************
*
*
*
*	MODE CODE
*
*
*
*****************************************************************************/


/////////////////////////////////////////////////////////////////////////////
//	Modes:		 HUMVEE_WAIT_FOR_START_MODE
//	Description: Only starts when there is one humvee on the screen
/////////////////////////////////////////////////////////////////////////////
void Humvee_Code_WAIT_FOR_START(CGameObjectInstance *pThis, CHumvee *pHumvee)
{
	// Keep invisible until needed
	CGameObjectInstance__SetGone(pThis) ;

	// No movement for now!
	pHumvee->m_Boss.m_AnimSpeed = 0.0 ;

#if MAKE_HUMVEE_START_RIGHT_AWAY
	if (1)
#else
	// Humvee 0 jumps in over player
	if ((AI_GetEA(pThis)->m_Id == 0) && (GetApp()->m_BossFlags & BOSS_FLAG_LONGHUNTER_START_HUMVEE1))
#endif
	{
		GetApp()->m_BossVars.m_StartBossTimer += frame_increment ;
		if (GetApp()->m_BossVars.m_StartBossTimer > SECONDS_TO_FRAMES(0.5))
		{
			// Start the humvee
			Humvee_SetupMode(pThis, HUMVEE_JUMP_INTO_ARENA_MODE) ;
		}
	}
	else
	// Humvee 2 jumps in from over the wall!
	if ((AI_GetEA(pThis)->m_Id == 1) && (GetApp()->m_BossFlags & BOSS_FLAG_LONGHUNTER_START_HUMVEE2))
	{
		// Wait for abit before starting
		GetApp()->m_BossVars.m_StartBossTimer += frame_increment ;
		if (GetApp()->m_BossVars.m_StartBossTimer > SECONDS_TO_FRAMES(3))
		{
			// Start the humvee
			Humvee_SetupMode(pThis, HUMVEE_JUMP_INTO_ARENA_MODE) ;
		}
	}
}


/////////////////////////////////////////////////////////////////////////////
//	Modes:		 HUMVEE_JUMP_INTO_ARENA_MODE
//	Description: Jumping into the arena, dukes of hazard style
/////////////////////////////////////////////////////////////////////////////


BOOL BOSS_InfrontOfTurok(CGameObjectInstance *pThis)
{
	CGameObjectInstance	*pTurok = CEngineApp__GetPlayer(GetApp()) ;
	CVector3	vDir ;
	FLOAT		Angle ;

	CVector3__Subtract(&vDir, &pThis->ah.ih.m_vPos, &pTurok->ah.ih.m_vPos) ;
	Angle = CVector3__XZAngle(&vDir) ;

	return (AngleDist(Angle, pTurok->m_RotY) < ANGLE_DTOR(90)) ;

}

void Humvee_Setup_JUMP_INTO_ARENA(CGameObjectInstance *pThis, CHumvee *pHumvee)
{
	pHumvee->m_Boss.m_Mode = HUMVEE_JUMP_INTO_ARENA_MODE ;

	CGameObjectInstance__SetNotGone(pThis) ;

	// Allow movement through forcefield
	CCollisionInfo__SetHumveeEnterCollisionDefaults(&pHumvee->m_Boss.m_CollisionInfo) ;

	// Make camera follow humvee
	if (AI_GetEA(pThis)->m_Id == 0)
	{
		CCamera__SetObjectToTrack(&GetApp()->m_Camera, pThis) ;
		CCamera__SetupMode(&GetApp()->m_Camera, CAMERA_CINEMA_FIRST_HUMVEE_MODE) ;
	}
	else
	if (AI_GetEA(pThis)->m_Id == 1)
	{
		// Play final humvee cinema
		CCamera__SetObjectToTrack(&GetApp()->m_Camera, pThis) ;
		CCamera__FadeToCinema(&GetApp()->m_Camera, CINEMA_FLAG_PLAY_FINAL_HUMVEE) ;
	}
}

void Humvee_Code_JUMP_INTO_ARENA(CGameObjectInstance *pThis, CHumvee *pHumvee)
{
//	if (BOSS_InfrontOfTurok(pThis))
//		rmonPrintf("Infront!!\r\n") ;
//	else
//		rmonPrintf("Behind!!\r\n") ;

	// If fade finished, bring on that puppy
	if ((GetApp()->m_Camera.m_Mode == CAMERA_CINEMA_FIRST_HUMVEE_MODE) ||
		 (GetApp()->m_Camera.m_Mode == CAMERA_CINEMA_FINAL_HUMVEE_MODE))
		pHumvee->m_Boss.m_AnimSpeed = 1.0 ;

	// Finished?
	if (pHumvee->m_Boss.m_AnimPlayed)
	{
		// Stop movement through forcefield
		CCollisionInfo__SetHumveeRunCollisionDefaults(&pHumvee->m_Boss.m_CollisionInfo) ;

		// Make boss hitable
		AI_GetDyn(pThis)->m_Invincible = FALSE ;
		COnScreen__AddBossBar(&GetApp()->m_OnScreen, (CBoss *)pHumvee) ;

		Humvee_SetupMode(pThis, HUMVEE_TURN180_MODE) ;

		// Exit cinematic
		Humvee_StopSound(pThis, pHumvee) ;
		CEngineApp__SetupFadeTo(GetApp(), MODE_RESETLEVEL) ;
		GetApp()->m_UseCinemaWarp = TRUE ;
	}
}



/////////////////////////////////////////////////////////////////////////////
//	Modes:		 HUMVEE_IDLE_MODE
//	Description: Waiting for Turok to come back alive
/////////////////////////////////////////////////////////////////////////////
void Humvee_Code_IDLE(CGameObjectInstance *pThis, CHumvee *pHumvee)
{
	// Turok back alive yet?
	if (CTurokMovement.Active)
		Humvee_SetupMode(pThis, HUMVEE_RUN_MODE) ;
}


/////////////////////////////////////////////////////////////////////////////
//	Modes:		 HUMVEE_RUN_MODE
//					 HUMVEE_TURN_LEFT90_MODE
//					 HUMVEE_TURN_RIGHT90_MODE
//					 HUMVEE_TURN180_MODE
//	Description: Merrily speeding along
/////////////////////////////////////////////////////////////////////////////
void Humvee_Code_Movement(CGameObjectInstance *pThis, CHumvee *pHumvee)
{
	FLOAT	DeltaAngle = pHumvee->m_Boss.m_DeltaAngle ;
//	FLOAT	Assist = HUMVEE_TURN_SPEED * pHumvee->m_Boss.m_PlaybackAnimSpeed ;

	FLOAT	Speed, Assist ;
	
	// Calc assist
	Speed = pHumvee->m_Boss.m_Speed / 14 ;
	if (Speed > 1.0)
		Speed = 1.0 ;
	Assist = HUMVEE_TURN_SPEED * Speed ;

	// Go straight?
//	if (abs(DeltaAngle) <= HUMVEE_FACING_ANGLE)
		pHumvee->m_Boss.m_Mode = HUMVEE_RUN_MODE ;

	// Turok dead?
	if (!CTurokMovement.Active)
		Humvee_SetupMode(pThis, HUMVEE_IDLE_MODE) ;
	else
	// Turning?
	if (pHumvee->m_Boss.m_Mode != HUMVEE_TURN180_MODE)
	{
		// Turn 180?
		if (DeltaAngle < -HUMVEE_TURN180_ANGLE)
		{
			if (RANDOM(2))
				pHumvee->m_Boss.m_Mode = HUMVEE_TURN180_MODE ;
			else
				pHumvee->m_Boss.m_Mode = HUMVEE_STOP_MODE ;
		}
		else
		{
			// Turn left?
			if (DeltaAngle < -HUMVEE_TURN90_ANGLE)
			{
//				if (pHumvee->m_Boss.m_Mode != HUMVEE_TURN_LEFT90_MODE)
				{
					Assist *= 2 ;
					pThis->ah.m_vVelocity.x = pHumvee->m_Boss.m_LastVel.x * 2 ;
					pThis->ah.m_vVelocity.z = pHumvee->m_Boss.m_LastVel.z * 2 ;

					Humvee_SEvent_GeneralSound(&pThis->ah.ih, pThis->ah.ih.m_vPos, SOUND_GENERIC_85) ;
//					pHumvee->m_Boss.m_Mode = HUMVEE_TURN_LEFT90_MODE ;
				}
			}
			else
			// Turn Right?
			if (DeltaAngle > HUMVEE_TURN90_ANGLE)
			{
//				if (pHumvee->m_Boss.m_Mode != HUMVEE_TURN_RIGHT90_MODE)
				{
					Assist *= 2 ;
					pThis->ah.m_vVelocity.x = pHumvee->m_Boss.m_LastVel.x * 2 ;
					pThis->ah.m_vVelocity.z = pHumvee->m_Boss.m_LastVel.z * 2 ;


					Humvee_SEvent_GeneralSound(&pThis->ah.ih, pThis->ah.ih.m_vPos, SOUND_GENERIC_85) ;
//					pHumvee->m_Boss.m_Mode = HUMVEE_TURN_RIGHT90_MODE ;
				}
			}
		}
	}

	// Assist with the turn
	BOSS_PerformTurn(pThis, (CBoss *)pHumvee, Assist, DeltaAngle) ;
}


/////////////////////////////////////////////////////////////////////////////
//	Modes:		 HUMVEE_STOP_MODE
//	Description: Hitting the breaks, complete with a nice U-Turn
/////////////////////////////////////////////////////////////////////////////
void Humvee_Code_STOP(CGameObjectInstance *pThis, CHumvee *pHumvee)
{
	// Finished?
	if (pHumvee->m_Boss.m_AnimPlayed)
	{
//		if (pHumvee->m_Boss.m_Action == HUMVEE_SHOOT_ACTION)
//		{
//			pHumvee->m_MachineGunTime = SECONDS_TO_FRAMES(3) ;
//			Humvee_SetupMode(pThis, HUMVEE_FIRE_MACHINE_GUN_MODE) ;
//		}
//		else
			Humvee_SetupMode(pThis, HUMVEE_RUN_MODE) ;
	}
}


/////////////////////////////////////////////////////////////////////////////
//	Modes:		 HUMVEE_EXPLODE_LEFT_MODE
//					 HUMVEE_EXPLODE_RIGHT_MODE
//	Description: Hit by a nasty explosion
/////////////////////////////////////////////////////////////////////////////
void Humvee_Code_Explode(CGameObjectInstance *pThis, CHumvee *pHumvee)
{
	// Finished?
	if (pHumvee->m_Boss.m_AnimPlayed)
		Humvee_SetupMode(pThis, HUMVEE_RUN_MODE) ;
}




/////////////////////////////////////////////////////////////////////////////
//	Modes:		 HUMVEE_DEATH_MODE
//	Description: The final death
/////////////////////////////////////////////////////////////////////////////
void Humvee_Setup_DEATH(CGameObjectInstance *pThis, CHumvee *pHumvee)
{
	pHumvee->m_Boss.m_Mode = HUMVEE_DEATH_MODE ;

	// Stop sounds
	if (pHumvee->m_Sound != -1)
	{
		pHumvee->m_Sound = -1 ;
		killCFX(pHumvee->m_SoundHandle) ;
	}
}



/*****************************************************************************
*
*	Function:		Humvee_DisplayMode()
*
******************************************************************************
*
*	Description:	Displays current mode when debug is set
*
*	Inputs:			*pBoss	-	Ptr to boss vars
*
*	Outputs:			None
*
*****************************************************************************/
#ifndef MAKE_CART
void Humvee_DisplayMode(CBoss *pBoss)
{
#if DEBUG_HUMVEE
	rmonPrintf("Humvee:%d - ", ((CHumvee *)pBoss)->m_Id) ;

	switch(pBoss->m_Mode)
	{
		case HUMVEE_WAIT_FOR_START_MODE:
			rmonPrintf("HUMVEE_WAIT_FOR_START_MODE\r\n") ;
			break ;
		case HUMVEE_JUMP_INTO_ARENA_MODE:
			rmonPrintf("HUMVEE_JUMP_INTO_ARENA_MODE\r\n") ;
			break ;
		case HUMVEE_IDLE_MODE:
			rmonPrintf("HUMVEE_IDLE_MODE\r\n") ;
			break ;
		case HUMVEE_FIRE_MACHINE_GUN_MODE:
			rmonPrintf("HUMVEE_FIRE_MACHINE_GUN_MODE\r\n") ;
			break ;
		case HUMVEE_RUN_MODE:
			rmonPrintf("HUMVEE_RUN_MODE\r\n") ;
			break ;
		case HUMVEE_TURN_LEFT90_MODE:
			rmonPrintf("HUMVEE_TURN_LEFT90_MODE\r\n") ;
			break ;
		case HUMVEE_TURN_RIGHT90_MODE:
			rmonPrintf("HUMVEE_TURN_RIGHT90_MODE\r\n") ;
			break ;
		case HUMVEE_TURN180_MODE:
			rmonPrintf("HUMVEE_TURN180_MODE\r\n") ;
			break ;
		case HUMVEE_STOP_MODE:
			rmonPrintf("HUMVEE_STOP_MODE\r\n") ;
			break ;
		case HUMVEE_EXPLODE_LEFT_MODE:
			rmonPrintf("HUMVEE_EXPLODE_LEFT_MODE\r\n") ;
			break ;
		case HUMVEE_EXPLODE_RIGHT_MODE:
			rmonPrintf("HUMVEE_EXPLODE_RIGHT_MODE\r\n") ;
			break ;
		case HUMVEE_DEATH_MODE:
			rmonPrintf("HUMVEE_DEATH_MODE\r\n") ;
			break ;

		default:
			rmonPrintf("Mode: %d\r\n", pBoss->m_Mode) ;
			break ;
	}
#endif
}
#endif




void Humvee_StopSound(CGameObjectInstance *pThis, CHumvee *pHumvee)
{
	if (pHumvee->m_SoundHandle != -1)
	{
		killCFX(pHumvee->m_SoundHandle) ;
		pHumvee->m_SoundHandle = -1 ;
	}
}

void Humvee_UpdateSound(CGameObjectInstance *pThis, CHumvee *pHumvee)
{
	// Update position
	if ((pHumvee->m_Boss.m_Mode == HUMVEE_DEATH_MODE) || (GetApp()->m_bPause))
		Humvee_StopSound(pThis, pHumvee) ;
}

void Humvee_SEvent_GeneralSound(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	INT32		Sound = (INT32)Value ;
	CHumvee	*pHumvee = (CHumvee *) ((CGameObjectInstance *)pThis)->m_pBoss ;

	// No sounds until humvee has began mission, or if he's dead
	if ((pHumvee->m_Boss.m_Mode == HUMVEE_WAIT_FOR_START_MODE) ||
		 (pHumvee->m_Boss.m_Mode == HUMVEE_DEATH_MODE))
		return ;

	// Start sound
	switch(Sound)
	{
		// Looping sounds
		case SOUND_GENERIC_82:
		case SOUND_GENERIC_83:
		case SOUND_GENERIC_86:

			// Already a looping sound playing?
			if (pHumvee->m_SoundHandle != -1)
			{
				// Requesting looping sound which is already playing?
				if (pHumvee->m_Sound == Sound)
					return ;

				// Kill current sound
				Humvee_StopSound((CGameObjectInstance *)pThis, pHumvee) ;
			}

			// Start sound
			pHumvee->m_Sound = Sound ;
			pHumvee->m_SoundHandle = AI_DoSound(pThis, Sound, 1, 0) ;
			break ;
	
		// Skidding sound - add some randomness
		case SOUND_GENERIC_85:
			if (RANDOM(3) == 0)
				AI_DoSound(pThis, Sound, 1, 0) ;
			break ;

		// Start none looping sounds
		default:
			AI_DoSound(pThis, Sound, 1, 0) ;
	}

}
