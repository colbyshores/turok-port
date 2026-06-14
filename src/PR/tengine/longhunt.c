// Longhunter boss control file by Stephen Broumley

#include "stdafx.h"

#include "ai.h"
#include "aistand.h"

#include "romstruc.h"
#include "tengine.h"
#include "scaling.h"
#include "audio.h"
#include "sfx.h"
#include "tmove.h"
#include "longhunt.h"
#include "boss.h"
#include "onscrn.h"
#include "dlists.h"
#include "regtype.h"
#include "wallcoll.h"
#include "bossflgs.h"

// NOTE: For all as one switches, set all pressure & switch id's to 101
// NOTE: For individual switches, set pressure & switch id's to 101-108


/////////////////////////////////////////////////////////////////////////////
// General constants
/////////////////////////////////////////////////////////////////////////////
#define	MAKE_LONGHUNT_START_REGARDLESS_OF_HUMVEES	0
#define	DEBUG_LONGHUNT										0
#define	DEBUG_LONGHUNT_HEADTRACK						0
#define	LONGHUNT_HEALTH									1000


/////////////////////////////////////////////////////////////////////////////
// Movement constants
/////////////////////////////////////////////////////////////////////////////
#define	LONGHUNT_GRAVITY					GRAVITY_ACCELERATION

#define	LONGHUNT_STAND_TURN90_ANGLE	ANGLE_DTOR(15)
#define	LONGHUNT_STAND_TURN180_ANGLE	ANGLE_DTOR(100)
#define	LONGHUNT_STAND_FACING_ANGLE 	ANGLE_DTOR(15)

#define	LONGHUNT_WALK_DIST				(8*SCALING_FACTOR)	// Dist before walking
#define	LONGHUNT_WALK_TURN_SPEED		ANGLE_DTOR(6)
#define	LONGHUNT_WALK_FACING_ANGLE		ANGLE_DTOR(30)
#define	LONGHUNT_WALK_TURN90_ANGLE		ANGLE_DTOR(20)
#define	LONGHUNT_WALK_TURN180_ANGLE	ANGLE_DTOR(150)

#define	LONGHUNT_RUN_DIST					(20*SCALING_FACTOR)	// Dist before running
#define	LONGHUNT_RUN_TURN_SPEED			ANGLE_DTOR(12)
#define	LONGHUNT_RUN_FACING_ANGLE		ANGLE_DTOR(35)
#define	LONGHUNT_RUN_TURN90_ANGLE		ANGLE_DTOR(25)
#define	LONGHUNT_RUN_TURN180_ANGLE		ANGLE_DTOR(150)

#define	LONGHUNT_LEAP_SMASH_ROLL_PERCENTAGE			(20)
#define	LONGHUNT_LEAP_SMASH_ROLL_TUROK_MAX_ANGLE	ANGLE_DTOR(10)
#define	LONGHUNT_LEAP_SMASH_ROLL_TUROK_MIN_SPEED	(0.5*SCALING_FACTOR)
#define	LONGHUNT_LEAP_SMASH_ROLL_RUN_TIME 	SECONDS_TO_FRAMES(2)		// Min running time before performing
#define	LONGHUNT_LEAP_SMASH_ROLL_MIN_DIST	(25*SCALING_FACTOR)
#define	LONGHUNT_LEAP_SMASH_ROLL_MAX_DIST	(30*SCALING_FACTOR)


// Head tracking stuff
#define	LONGHUNT_ROTY_OFFSET				ANGLE_DTOR(-20)

#define	LONGHUNT_HEIGHT  					(7.6*SCALING_FACTOR)
#define	LONGHUNT_HEAD_HEIGHT				(7*SCALING_FACTOR)
#define	LONGHUNT_SHOULDER_HEIGHT		(5*SCALING_FACTOR)
#define	LONGHUNT_STOMACH_HEIGHT			(4*SCALING_FACTOR)
#define	LONGHUNT_LEG_HEIGHT				(3*SCALING_FACTOR)


#define	LONGHUNT_PROJECTILE_ANGLE 		ANGLE_DTOR(10)
#define	LONGHUNT_FLINCH_PERCENTAGE	30



/////////////////////////////////////////////////////////////////////////////
// Animation constants
/////////////////////////////////////////////////////////////////////////////

// Stand modes
#define AI_ANIM_BOSS_LONGHUNT_STAND_IDLE				AI_ANIM_IDLE
#define AI_ANIM_BOSS_LONGHUNT_STAND_TURN180			AI_ANIM_TURN180
#define AI_ANIM_BOSS_LONGHUNT_STAND_TURN_LEFT90		AI_ANIM_TURNL90
#define AI_ANIM_BOSS_LONGHUNT_STAND_TURN_RIGHT90  	AI_ANIM_TURNR90

// Start modes
#define AI_ANIM_BOSS_LONGHUNT_WAIT_FOR_START					AI_ANIM_IDLE
#define AI_ANIM_BOSS_LONGHUNT_SUMMON_HUMVEE					AI_ANIM_EXTRA8
#define AI_ANIM_BOSS_LONGHUNT_WAIT_FOR_HUMVEE_ENTRANCE	AI_ANIM_IDLE
#define AI_ANIM_BOSS_LONGHUNT_WAIT_FOR_HUMVEES_DEATH		AI_ANIM_IDLE


// Taunt modes
#define AI_ANIM_BOSS_LONGHUNT_TAUNT1				  	AI_ANIM_EXTRA8

// Walk modes
#define AI_ANIM_BOSS_LONGHUNT_WALK						AI_ANIM_WALK
#define AI_ANIM_BOSS_LONGHUNT_WALK_TURN180			AI_ANIM_WTURN180
#define AI_ANIM_BOSS_LONGHUNT_WALK_TURN_LEFT90		AI_ANIM_WTURNL90
#define AI_ANIM_BOSS_LONGHUNT_WALK_TURN_RIGHT90		AI_ANIM_WTURNR90

// Run modes
#define AI_ANIM_BOSS_LONGHUNT_RUN						AI_ANIM_RUN
#define AI_ANIM_BOSS_LONGHUNT_RUN_TURN180				AI_ANIM_RTURN180
#define AI_ANIM_BOSS_LONGHUNT_RUN_TURN_LEFT90		AI_ANIM_RTURNL90
#define AI_ANIM_BOSS_LONGHUNT_RUN_TURN_RIGHT90		AI_ANIM_RTURNR90

// Evade modes
#define AI_ANIM_BOSS_LONGHUNT_EVADE_LEFT				AI_ANIM_EVADE_LEFT
#define AI_ANIM_BOSS_LONGHUNT_EVADE_RIGHT				AI_ANIM_EVADE_RIGHT

// Close attacks
#define AI_ANIM_BOSS_LONGHUNT_ROUNDHOUSE				AI_ANIM_ATTACK_STRONG1
#define AI_ANIM_BOSS_LONGHUNT_JUMP_KICK				AI_ANIM_ATTACK_STRONG2
#define AI_ANIM_BOSS_LONGHUNT_SWING_PUNCH				AI_ANIM_ATTACK_STRONG3

// Run attacks
#define AI_ANIM_BOSS_LONGHUNT_FIRE_GUN					AI_ANIM_ATTACK_STRONG4
#define AI_ANIM_BOSS_LONGHUNT_THROW_GRENADE			AI_ANIM_ATTACK_STRONG5
#define AI_ANIM_BOSS_LONGHUNT_RUN_FIRE_GUN			AI_ANIM_ATTACK_STRONG6

// Death modes
#define AI_ANIM_BOSS_LONGHUNT_DEATH						AI_ANIM_DEATH_NORMAL


/////////////////////////////////////////////////////////////////////////////
// Function prototypes
/////////////////////////////////////////////////////////////////////////////


// Initialisation
void CCollisionInfo__SetLonghuntCollisionDefaults(CCollisionInfo2 *pThis) ;
void Longhunt_SetupStageInfo(CGameObjectInstance *pThis, CLonghunt *pLonghunt, CLonghuntStageInfo *Info) ;


// General code
void Longhunt_GetTarget(CGameObjectInstance *pThis, CLonghunt *pLonghunt) ;
BOOL Longhunt_InProjectilePath(CGameObjectInstance *pThis, CLonghunt *pLonghunt) ;
void Longhunt_CheckForEvade(CGameObjectInstance *pThis, CLonghunt *pLonghunt) ;
BOOL Longhunt_CheckForWalkRunAttack(CGameObjectInstance *pThis, CLonghunt *pLonghunt) ;


// Mode code
void Longhunt_Code_WAIT_FOR_START(CGameObjectInstance *pThis, CLonghunt *pLonghunt) ;
void Longhunt_Code_SUMMON_HUMVEE(CGameObjectInstance *pThis, CLonghunt *pLonghunt) ;
void Longhunt_Code_WAIT_FOR_HUMVEE_ENTRANCE(CGameObjectInstance *pThis, CLonghunt *pLonghunt) ;
void Longhunt_Code_WAIT_FOR_HUMVEES_DEATH(CGameObjectInstance *pThis, CLonghunt *pLonghunt) ;

void Longhunt_Code_Movement(CGameObjectInstance *pThis, CLonghunt *pLonghunt) ;

void Longhunt_Code_Evade(CGameObjectInstance *pThis, CLonghunt *pLonghunt) ;

void Longhunt_PlayAttackTauntSount(CGameObjectInstance *pThis, CLonghunt *pLonghunt) ;
void Longhunt_Setup_CloseAttack(CGameObjectInstance *pThis, CLonghunt *pLonghunt) ;
void Longhunt_Code_CloseAttack(CGameObjectInstance *pThis, CLonghunt *pLonghunt) ;
void Longhunt_Setup_WalkAttack(CGameObjectInstance *pThis, CLonghunt *pLonghunt) ;
void Longhunt_Code_WalkAttack(CGameObjectInstance *pThis, CLonghunt *pLonghunt) ;
void Longhunt_Setup_RunAttack(CGameObjectInstance *pThis, CLonghunt *pLonghunt) ;
void Longhunt_Code_RunAttack(CGameObjectInstance *pThis, CLonghunt *pLonghunt) ;


void Longhunt_Setup_Taunt(CGameObjectInstance *pThis, CLonghunt *pLonghunt) ;
void Longhunt_Code_Taunt(CGameObjectInstance *pThis, CLonghunt *pLonghunt) ;

void Longhunt_OpenForceField(CGameObjectInstance *pThis, CLonghunt *pLonghunt) ;
void Longhunt_Setup_DEATH(CGameObjectInstance *pThis, CLonghunt *pLonghunt) ;
void Longhunt_Code_DEATH(CGameObjectInstance *pThis, CLonghunt *pLonghunt) ;


#ifndef MAKE_CART
void Longhunt_DisplayMode(CBoss *pBoss) ;
#endif



/////////////////////////////////////////////////////////////////////////////
// Variables
/////////////////////////////////////////////////////////////////////////////

// Longhunt itself
CLonghunt	LonghuntBoss ;


// Longhunt mode table
CBossModeInfo	LonghuntModeTable[] =
{
	// Start modes
	BOSS_MODE_INFO(NULL,		Longhunt_Code_WAIT_FOR_START,	AI_ANIM_BOSS_LONGHUNT_WAIT_FOR_START,	1, 0 | BF_HEAD_TRACK_TUROK | BF_INVINCIBLE),
	BOSS_MODE_INFO(NULL,		Longhunt_Code_SUMMON_HUMVEE,	AI_ANIM_BOSS_LONGHUNT_SUMMON_HUMVEE,	1, 0 | BF_HEAD_TRACK_TUROK | BF_INVINCIBLE),
	BOSS_MODE_INFO(NULL,		Longhunt_Code_WAIT_FOR_HUMVEE_ENTRANCE,	AI_ANIM_BOSS_LONGHUNT_WAIT_FOR_HUMVEE_ENTRANCE,	1, 0 | BF_HEAD_TRACK_TUROK | BF_INVINCIBLE),
	BOSS_MODE_INFO(NULL,		Longhunt_Code_WAIT_FOR_HUMVEES_DEATH,		AI_ANIM_BOSS_LONGHUNT_WAIT_FOR_HUMVEES_DEATH,		1, 0 | BF_HEAD_TRACK_TUROK | BF_INVINCIBLE),

	// Stand modes
	BOSS_MODE_INFO(Longhunt_Code_Movement,	Longhunt_Code_Movement,	AI_ANIM_BOSS_LONGHUNT_STAND_IDLE,			1, BF_CAN_EVADE | BF_CAN_FLINCH | BF_HEAD_TRACK_TUROK),
	BOSS_MODE_INFO(NULL,		Longhunt_Code_Movement,		AI_ANIM_BOSS_LONGHUNT_STAND_TURN180,		1,BF_CAN_EVADE | BF_CAN_FLINCH | BF_HEAD_TRACK_TUROK),
	BOSS_MODE_INFO(NULL,		Longhunt_Code_Movement,		AI_ANIM_BOSS_LONGHUNT_STAND_TURN_LEFT90,	1,BF_CAN_EVADE | BF_CAN_FLINCH | BF_HEAD_TRACK_TUROK),
	BOSS_MODE_INFO(NULL,		Longhunt_Code_Movement,		AI_ANIM_BOSS_LONGHUNT_STAND_TURN_RIGHT90,	1,BF_CAN_EVADE | BF_CAN_FLINCH | BF_HEAD_TRACK_TUROK),

	// Taunt modes
	BOSS_MODE_INFO(Longhunt_Setup_Taunt,	Longhunt_Code_Taunt,	AI_ANIM_BOSS_LONGHUNT_TAUNT1,			1, BF_CAN_EVADE | BF_CAN_FLINCH | BF_HEAD_TRACK_TUROK),

	// Walk modes
	BOSS_MODE_INFO(Longhunt_Code_Movement,	Longhunt_Code_Movement,		AI_ANIM_BOSS_LONGHUNT_WALK,					1,BF_CAN_EVADE | BF_CAN_FLINCH | BF_HEAD_TRACK_TUROK | BF_WALKING_MODE),
	BOSS_MODE_INFO(NULL,		Longhunt_Code_Movement,		AI_ANIM_BOSS_LONGHUNT_WALK_TURN180,			1,BF_CAN_EVADE | BF_CAN_FLINCH | BF_HEAD_TRACK_TUROK | BF_WALKING_MODE),
	BOSS_MODE_INFO(NULL,		Longhunt_Code_Movement,		AI_ANIM_BOSS_LONGHUNT_WALK_TURN_LEFT90,	1,BF_CAN_EVADE | BF_CAN_FLINCH | BF_HEAD_TRACK_TUROK | BF_WALKING_MODE),
	BOSS_MODE_INFO(NULL,		Longhunt_Code_Movement,		AI_ANIM_BOSS_LONGHUNT_WALK_TURN_RIGHT90,	1,BF_CAN_EVADE | BF_CAN_FLINCH | BF_HEAD_TRACK_TUROK | BF_WALKING_MODE),

	// Run modes
	BOSS_MODE_INFO(Longhunt_Code_Movement,		Longhunt_Code_Movement,		AI_ANIM_BOSS_LONGHUNT_RUN,						1,BF_CAN_EVADE | BF_CAN_FLINCH | BF_RUNNING_MODE | BF_HEAD_TRACK_TUROK | BF_RUNNING_MODE),
	BOSS_MODE_INFO(NULL,		Longhunt_Code_Movement,		AI_ANIM_BOSS_LONGHUNT_RUN_TURN180,			1,BF_CAN_EVADE | BF_CAN_FLINCH | BF_RUNNING_MODE | BF_HEAD_TRACK_TUROK | BF_RUNNING_MODE),
	BOSS_MODE_INFO(NULL,		Longhunt_Code_Movement,		AI_ANIM_BOSS_LONGHUNT_RUN_TURN_LEFT90,		1,BF_CAN_EVADE | BF_CAN_FLINCH | BF_RUNNING_MODE | BF_HEAD_TRACK_TUROK | BF_RUNNING_MODE),
	BOSS_MODE_INFO(NULL,		Longhunt_Code_Movement,		AI_ANIM_BOSS_LONGHUNT_RUN_TURN_RIGHT90,	1,BF_CAN_EVADE | BF_CAN_FLINCH | BF_RUNNING_MODE | BF_HEAD_TRACK_TUROK | BF_RUNNING_MODE),

	// Evade modes
	BOSS_MODE_INFO(NULL,		Longhunt_Code_Evade,			AI_ANIM_BOSS_LONGHUNT_EVADE_LEFT,	1,	BF_EVADE_MODE),
	BOSS_MODE_INFO(NULL,		Longhunt_Code_Evade,			AI_ANIM_BOSS_LONGHUNT_EVADE_RIGHT,	1,	BF_EVADE_MODE),

	// Close/walk attacks
	BOSS_MODE_INFO(NULL,		Longhunt_Code_CloseAttack,		AI_ANIM_BOSS_LONGHUNT_ROUNDHOUSE,	1.6,	BF_HEAD_TRACK_TUROK | BF_KICK_IMPACT_MODE),
	BOSS_MODE_INFO(NULL,		Longhunt_Code_WalkAttack,		AI_ANIM_BOSS_LONGHUNT_JUMP_KICK,		1.5,	BF_HEAD_TRACK_TUROK | BF_KICK_IMPACT_MODE),
	BOSS_MODE_INFO(NULL,		Longhunt_Code_CloseAttack,		AI_ANIM_BOSS_LONGHUNT_SWING_PUNCH,	2.0,	BF_HEAD_TRACK_TUROK | BF_PUNCH_IMPACT_MODE),

	// Running attacks
	BOSS_MODE_INFO(NULL,		Longhunt_Code_WalkAttack,			AI_ANIM_BOSS_LONGHUNT_FIRE_GUN,			1.0,	BF_HEAD_TRACK_TUROK),
	BOSS_MODE_INFO(NULL,		Longhunt_Code_RunAttack,			AI_ANIM_BOSS_LONGHUNT_THROW_GRENADE,	3.5,	BF_HEAD_TRACK_TUROK),
	BOSS_MODE_INFO(NULL,		Longhunt_Code_RunAttack,			AI_ANIM_BOSS_LONGHUNT_RUN_FIRE_GUN,		1.0,	BF_HEAD_TRACK_TUROK),

	// Death modes
	BOSS_MODE_INFO(Longhunt_Setup_DEATH,	Longhunt_Code_DEATH,	AI_ANIM_BOSS_LONGHUNT_DEATH,						0.9,	BF_DEATH_MODE | BF_CLEAR_VELS),

	{NULL}
} ;


/////////////////////////////////////////////////////////////////////////////
//
// The big stage table
//
/////////////////////////////////////////////////////////////////////////////
CLonghuntStageInfo LonghuntStageInfo[] =
{
	//	STAGE 1
	{75,								// Health limit for this stage
	 1.0,								// Anim speed

	 SECONDS_TO_FRAMES(1),		// Evade time
	 4,								// Evade skips during evade time (1 = all time)
	 SECONDS_TO_FRAMES(4),		// No evade time

	 20,								// Flinch percentage

	 SECONDS_TO_FRAMES(2),		// Run time before an attack

	 2,								// Gun fire total
	},

	//	STAGE 2
	{50,			  					// Health limit for this stage
	 1.05,		  					// Anim speed

	 SECONDS_TO_FRAMES(2),		// Evade time
	 3,								// Evade skips during evade time (1 = all time)
	 SECONDS_TO_FRAMES(5),		// No evade time

	 12,								// Flinch percentage

	 SECONDS_TO_FRAMES(1.75),		// Run time before an attack
	 2,								// Gun fire total
	},

	//	STAGE 3
	{25,								// Health limit for this stage
	 1.10, 				  			// Anim speed

	 SECONDS_TO_FRAMES(2),		// Evade time
	 2,								// Evade skips during evade time (1 = all time)
	 SECONDS_TO_FRAMES(6),		// No evade time

	 6,								// Flinch percentage

	 SECONDS_TO_FRAMES(1.5),		// Run time before an attack
	 3,								// Gun fire total
	},

	//	STAGE 4
	{0,								// Health limit for this stage
	 1.15,				  			// Anim speed

	 SECONDS_TO_FRAMES(3),		// Evade time
	 1,								// Evade skips during evade time (1 = all time)
	 SECONDS_TO_FRAMES(7),		// No evade time

	 4,								// Flinch percentage

	 SECONDS_TO_FRAMES(1),		// Run time before an attack
	 3,								// Gun fire total
	},
} ;

// Run attack selections
CSelection	LonghuntRunAttackSelection[] =
{
	{LONGHUNT_FIRE_GUN_MODE,		50},
	{LONGHUNT_THROW_GRENADE_MODE,	50},
	{LONGHUNT_RUN_FIRE_GUN_MODE,	25},
	{-1}
} ;

// Walk attack selections
CSelection	LonghuntWalkAttackSelection[] =
{
	{LONGHUNT_JUMP_KICK_MODE,		50},
	{LONGHUNT_FIRE_GUN_MODE,		25},
	{-1}
} ;

// Close attack selections
CSelection	LonghuntCloseAttackSelection[] =
{
	{LONGHUNT_ROUNDHOUSE_MODE,		50},
	{LONGHUNT_SWING_PUNCH_MODE,	50},
	{LONGHUNT_TAUNT1_MODE,			5},
	{-1}
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
*	Function:		Longhunt_Initialise()
*
******************************************************************************
*
*	Description:	Prepares longhunter boss
*
*	Inputs:			*pThis	-	Ptr to object instance
*
*	Outputs:			CBoss *	-	Ptr to boss vars
*
*****************************************************************************/
CBoss *Longhunt_Initialise(CGameObjectInstance *pThis)
{
	CLonghunt		*pLonghunt = &LonghuntBoss ;

	// Setup defaults
	CBoss__Construct(&pLonghunt->m_Boss) ;

	// Setup boss vars
	pLonghunt->m_Boss.m_pUpdateFunction = (void *)Longhunt_Update ;
	pLonghunt->m_Boss.m_pModeTable = LonghuntModeTable ;
	pLonghunt->m_Boss.m_Rot1 = 0 ;
	pLonghunt->m_Boss.m_Rot2 = 0 ;

	// Setup longhunt vars
	pLonghunt->m_pStageInfo = LonghuntStageInfo ;
	Longhunt_SetupStageInfo(pThis, pLonghunt, LonghuntStageInfo) ;

	// Put into start mode
	pLonghunt->m_Boss.m_Mode = LONGHUNT_WAIT_FOR_START_MODE ;
	pLonghunt->m_Boss.m_ModeAnimSpeed = 0 ;
	pLonghunt->m_Boss.m_AnimRepeats = 1 ;
	pLonghunt->m_Boss.m_AnimSpeed = 1.0 ;

	pLonghunt->m_Boss.m_Action = LONGHUNT_WAIT_FOR_HUMVEES_DEATH_ACTION ;
	pLonghunt->m_Boss.m_ActionStage = 0 ;

	pLonghunt->m_Boss.m_pRestoreFunction = (void *)Longhunt_OpenForceField ;

#ifndef MAKE_CART
	pLonghunt->m_Boss.m_pDisplayModeFunction = (void *)Longhunt_DisplayMode ;
#endif

#if DEBUG_LONGHUNT
	rmonPrintf("\r\n\r\n\r\nLONGHUNTER HERE WE GO!! Prepare for Attack Action\r\n") ;
#endif


	// Make boss invincible for now
	AI_GetDyn(pThis)->m_Invincible = TRUE ;

	// Setup misc
	CCollisionInfo__SetLonghuntCollisionDefaults(&pLonghunt->m_Boss.m_CollisionInfo) ;
	BOSS_Health(pThis) = LONGHUNT_HEALTH ;

	// Evade
	pLonghunt->m_EvadeTime = 0 ;
	pLonghunt->m_EvadeCount = 0 ;
	pLonghunt->m_EvadeActive = FALSE ;

	// Reset run timers
	pLonghunt->m_RunTime = 0 ;
	pLonghunt->m_RunAttackTime = 0 ;
	pLonghunt->m_RunAngle = 0 ;

	pLonghunt->m_SoundTimer = SECONDS_TO_FRAMES(2) ;

	// Return pointer to longhunt boss
	return (CBoss *)pLonghunt ;
}




/*****************************************************************************
*
*	Function:		CCollisionInfo__SetLonghuntCollisionDefaults()
*
******************************************************************************
*
*	Description:	Sets up default longhunter collision info
*
*	Inputs:			*pThis	-	Ptr to collision info structure
*
*	Outputs:			None
*
*****************************************************************************/
void CCollisionInfo__SetLonghuntCollisionDefaults(CCollisionInfo2 *pThis)
{
	pThis->dwFlags =		COLLISIONFLAG_PHYSICS
							|	COLLISIONFLAG_EXITWATER
							|	COLLISIONFLAG_USEWALLRADIUS
							|	COLLISIONFLAG_TRACKGROUND
							|	COLLISIONFLAG_WATERCOLLISION
							|	COLLISIONFLAG_IGNOREDEAD;

	pThis->InstanceBehavior			= INTERSECT_BEHAVIOR_GREASE;
	pThis->WallBehavior				= INTERSECT_BEHAVIOR_SLIDEBOUNCE;
	pThis->GroundBehavior			= INTERSECT_BEHAVIOR_SLIDE;
	pThis->GravityAcceleration		= LONGHUNT_GRAVITY;
	pThis->BounceReturnEnergy		= 0.0;
	pThis->GroundFriction			= 0.0;
	pThis->AirFriction				= 0.0;
	pThis->WaterFriction				= 0.0;
}




/*****************************************************************************
*
*	Function:		Longhunt_SetupStageInfo()
*
******************************************************************************
*
*	Description:	Sets up stage vars from stage info table
*
*	Inputs:			*pThis		-	Ptr to game object instance
*						*pLonghunt	-	Ptr to boss vars
*						*Info			-	Ptr to stage info vars
*
*	Outputs:			None
*
*****************************************************************************/
void Longhunt_SetupStageInfo(CGameObjectInstance *pThis, CLonghunt *pLonghunt, CLonghuntStageInfo *Info)
{
}

#define Particle_GetPos(pThis) (pThis->ah.ih.m_vPos)
#define Particle_GetVel(pThis) (pThis->ah.m_vVelocity)







/*****************************************************************************
*
*
*
*	GENERAL CODE
*
*
*
*****************************************************************************/


void Longhunt_UpdateAction(CGameObjectInstance *pThis, CLonghunt *pLonghunt)
{
	// Start attack and make longhunter mortal?
	if (((GetApp()->m_BossFlags & (BOSS_FLAG_HUMVEE1_DEAD | BOSS_FLAG_HUMVEE2_DEAD)) ==
				(BOSS_FLAG_HUMVEE1_DEAD | BOSS_FLAG_HUMVEE2_DEAD)) &&
		 (pLonghunt->m_Boss.m_Mode == LONGHUNT_STAND_IDLE_MODE))
	{
		COnScreen__AddBossBar(&GetApp()->m_OnScreen, (CBoss *)pLonghunt) ;
		pLonghunt->m_Boss.m_Action = LONGHUNT_ATTACK_ACTION ;
		AI_GetDyn(pThis)->m_Invincible = FALSE ;
	}
	else
	// Come out of victory?
	if ((CTurokMovement.Active) && (pLonghunt->m_Boss.m_Action == LONGHUNT_VICTORY_ACTION))
		pLonghunt->m_Boss.m_Action = LONGHUNT_ATTACK_ACTION ;
	else
	// Goto victory?
	if ((!CTurokMovement.Active) && (pLonghunt->m_Boss.m_Action == LONGHUNT_ATTACK_ACTION))
		pLonghunt->m_Boss.m_Action = LONGHUNT_VICTORY_ACTION ;
}



/*****************************************************************************
*
*	Function:		Longhunt_Update()
*
******************************************************************************
*
*	Description:	The big top level AI update routine - calls mode code,
*						performs checks etc.
*
*	Inputs:			*pThis		-	Ptr to game object instance
*						*pLonghunt	-	Ptr to boss vars
*
*	Outputs:			None
*
*****************************************************************************/
void Longhunt_Update(CGameObjectInstance *pThis, CLonghunt *pLonghunt)
{
	CVector3			vPos ;
	CGameRegion		*pRegion ;
	int				i ;

	// Play hurt sound every 10% damage
	for (i = 10 ; i <= 100 ; i += 10)
	{
		if ((pLonghunt->m_Boss.m_LastPercentageHealth >= i) &&
			 (pLonghunt->m_Boss.m_PercentageHealth < i))
		{
			AI_SEvent_GeneralSound(&pThis->ah.ih, pThis->ah.ih.m_vPos, SOUND_LONGHUNTER_TAUNT_3) ;
			break ;
		}
	}

	// Update sound taunt timer
	pLonghunt->m_SoundTimer -= frame_increment ;
	if (pLonghunt->m_SoundTimer < 0)
		pLonghunt->m_SoundTimer = 0 ;

	// Generate key
	if ((game_frame_number > 10) && (!(GetApp()->m_BossFlags & BOSS_FLAG_LONGHUNTER_GENERATED_KEY)))
	{
		GetApp()->m_BossFlags |= BOSS_FLAG_LONGHUNTER_GENERATED_KEY ;
		CVector3__Set(&vPos,0,0,0) ;
		pRegion = CScene__NearestRegion(&GetApp()->m_Scene, &vPos) ;
		CSimplePool__CreateSimpleInstance(&GetApp()->m_Scene.m_SimplePool,
												    AI_OBJECT_PICKUP_KEY5, vPos,vPos,pRegion,0) ;
	}

//	enemy_speed_scaler = 0.25 ;
	// Update action AI
	Longhunt_UpdateAction(pThis, pLonghunt) ;

	// Get target and angle to aim for
	Longhunt_GetTarget(pThis, pLonghunt) ;

	// Check for death
	if (BOSS_Health(pThis) <= 0)
	{
		switch(pLonghunt->m_Boss.m_Mode)
		{
			// On floor modes - goto death
			case LONGHUNT_DEATH_MODE:
				break ;

			default:
				Longhunt_SetupMode(pThis, LONGHUNT_DEATH_MODE) ;
				break ;
		}
	}


	// Update run time
	if (!(pLonghunt->m_Boss.m_ModeFlags & BF_RUNNING_MODE))
		pLonghunt->m_RunTime = 0 ;
	else
		pLonghunt->m_RunTime += frame_increment ;


	// No interruptions on blends!
//	if (CGameObjectInstance__IsBlending(pThis))
//		return ;


	// Call update function if there is one
	if ((pLonghunt->m_Boss.m_Mode >= 0 ) &&
		 (pLonghunt->m_Boss.m_pModeTable[pLonghunt->m_Boss.m_Mode].m_pUpdateFunction))
		 (pLonghunt->m_Boss.m_pModeTable[pLonghunt->m_Boss.m_Mode].m_pUpdateFunction)(pThis, (CBoss *)pLonghunt) ;


	// Check for evading projectiles
	Longhunt_CheckForEvade(pThis, pLonghunt) ;


	// Check for updating stage
	if (pLonghunt->m_Boss.m_PercentageHealth < pLonghunt->m_pStageInfo->HealthLimit)
	{
		pLonghunt->m_Boss.m_Stage++ ;
		Longhunt_SetupStageInfo(pThis, pLonghunt, ++pLonghunt->m_pStageInfo) ;
#if DEBUG_LONGHUNT
			rmonPrintf("\r\n....NEXT ATTACK STAGE....\r\n") ;
#endif
	}


	// Update anim speed
	switch(pLonghunt->m_Boss.m_Mode)
	{
		default:
			pLonghunt->m_Boss.m_AnimSpeed = pLonghunt->m_pStageInfo->AnimSpeed ;
	}
}







/*****************************************************************************
*
*	Function:		Longhunt_SetupMode()
*
******************************************************************************
*
*	Description:	Sets up new longhunter mode
*
*	Inputs:			*pThis		-	Ptr to game object instance
*						*pLonghunt	-	Ptr to boss vars
*
*	Outputs:			TRUE if new mode was setup, else FALSE
*
*****************************************************************************/
INT32 Longhunt_SetupMode(CGameObjectInstance *pThis, INT32 nNewMode)
{
	BOOL	Result ;

	// Call normal boss setup
	Result = BOSS_SetupMode(pThis, nNewMode) ;

	return Result ;
}

void Longhunt_ChooseRunningTarget(CGameObjectInstance *pThis, CLonghunt *pLonghunt)
{
	CVector3		vDest, vPos ;
	FLOAT			rot, radius, closestdist, dist ;
	INT32			i ;

	// Choose destination to side of longhunter
	radius = 150*SCALING_FACTOR ;
	rot = pThis->m_RotY + ANGLE_DTOR(90) ;
	vDest.x = pThis->ah.ih.m_vPos.x - (radius * sin(rot)) ;
	vDest.z = pThis->ah.ih.m_vPos.z - (radius * cos(rot)) ;

	// Get closest pos around arena
	radius = 168*SCALING_FACTOR ;
	closestdist = SQR(500*SCALING_FACTOR) ;
	for (i = 0 ; i < 8 ; i++)
	{
		rot = i * ANGLE_DTOR(45) ;
		vPos.x = vDest.x - (radius * sin(rot)) ;
		vPos.y = pThis->ah.ih.m_vPos.y ;
		vPos.z = vDest.z - (radius * cos(rot)) ;

		dist = AI_DistanceSquared(pThis, vPos) ;
		if (dist < closestdist)
		{
			pLonghunt->m_Boss.m_ActionStage = i ;
			closestdist = dist ;
		}
	}

	pLonghunt->m_Boss.m_ActionTime = 0 ;
}



/*****************************************************************************
*
*	Function:		Longhunt_GetTarget()
*
******************************************************************************
*
*	Description:	Calculates target to aim for, distance from target, and the
*						correct angle to head.
*
*	Inputs:			*pThis		-	Ptr to game object instance
*						*pLonghunt	-	Ptr to boss vars
*
*	Outputs:			None
*
*****************************************************************************/
void Longhunt_GetTarget(CGameObjectInstance *pThis, CLonghunt *pLonghunt)
{
	CGameObjectInstance *pTurok ;
	CVector3		vTurokPos, vDir ;
	FLOAT			rot, radius ;
	CLine			Line ;

	// Setup Turok pos
	pTurok = CEngineApp__GetPlayer(GetApp());
	vTurokPos = TUROK_GetPos(pTurok) ;

	// Setup target pos depending upon action
	switch(pLonghunt->m_Boss.m_Action)
	{
		// Goto corners of arena
		case LONGHUNT_WAIT_FOR_HUMVEES_DEATH_ACTION:
			radius = 168*SCALING_FACTOR ;
			rot = pLonghunt->m_Boss.m_ActionStage * ANGLE_DTOR(45) ;

			pLonghunt->m_Boss.m_ActionTime += frame_increment ;
			if (pLonghunt->m_Boss.m_ActionTime > SECONDS_TO_FRAMES(20))
			{
				pLonghunt->m_Boss.m_ActionTime -= SECONDS_TO_FRAMES(20) ;
				pLonghunt->m_Boss.m_ActionStage++ ;
				pLonghunt->m_Boss.m_ActionStage &= 7 ;
			}

			vDir.x = - (radius * sin(rot)) ;
			vDir.y = vTurokPos.y ;
			vDir.z = - (radius * cos(rot)) ;
			break ;

		// Go zip zag to Turok
		case LONGHUNT_ATTACK_ACTION:
			radius = 0 ;
			rot = 0 ;

			vDir.x = vTurokPos.x - (radius * sin(rot)) ;
			vDir.y = vTurokPos.y ;
			vDir.z = vTurokPos.z - (radius * cos(rot)) ;

			// Add sine wave to target
			radius = (sqrt(AI_DistanceSquared(pThis, pTurok->ah.ih.m_vPos)) - (15*SCALING_FACTOR)) * 2 ;
			if (radius > (80*SCALING_FACTOR))
				radius = (80*SCALING_FACTOR) ;
			else
			if (radius < 0)
				radius = 0 ;

			// Add sine along normal
			CLine__Construct(&Line, &pThis->ah.ih.m_vPos, &pTurok->ah.ih.m_vPos) ;
			CVector3__Normalize(&Line.m_vNormal) ;
			vDir.x += Line.m_vNormal.x * radius * sin(pLonghunt->m_RunAngle) ;
			vDir.z += Line.m_vNormal.z * radius * cos(pLonghunt->m_RunAngle) ;

			// Next sine angle
			pLonghunt->m_RunAngle += ANGLE_DTOR(5) ;
			break ;

		// Go infront of Turok
		case LONGHUNT_VICTORY_ACTION:
			radius = 15*SCALING_FACTOR ;
			rot = pTurok->m_RotY ;

			vDir.x = vTurokPos.x - (radius * sin(rot)) ;
			vDir.y = vTurokPos.y ;
			vDir.z = vTurokPos.z - (radius * cos(rot)) ;
			break ;

	}

	// Calculate distance from this target
	pLonghunt->m_Boss.m_vTarget = vDir ;
	pLonghunt->m_Boss.m_DistFromTargetSqr = AI_DistanceSquared(pThis, vDir) ;
	if (pLonghunt->m_Boss.m_DistFromTargetSqr)
		pLonghunt->m_Boss.m_DistFromTarget = sqrt(pLonghunt->m_Boss.m_DistFromTargetSqr) ;
	else
		pLonghunt->m_Boss.m_DistFromTarget = 0 ;

	// Calculate angle to drive at
	if (CTurokMovement.Active)
		pLonghunt->m_Boss.m_DeltaAngle = AI_GetAvoidanceAngle(pThis, pLonghunt->m_Boss.m_vTarget, pTurok, AVOIDANCE_RADIUS_DISTANCE_FACTOR) ;
	else
		pLonghunt->m_Boss.m_DeltaAngle = AI_GetAvoidanceAngle(pThis, pLonghunt->m_Boss.m_vTarget, NULL, AVOIDANCE_RADIUS_DISTANCE_FACTOR) ;

	// Calculate angle to look at
	pLonghunt->m_Boss.m_LookDeltaAngle = AI_GetAngle(pThis, pTurok->ah.ih.m_vPos) ;
}






/*****************************************************************************
*
*	Function:		Longhunt_InProjectilePath()
*
******************************************************************************
*
*	Description:	Determines whether the longhunter is in the path of one
*						of Turok's projectile weapons
*
*	Inputs:			*pThis		-	Ptr to game object instance
*						*pLonghunt	-	Ptr to boss vars
*
*	Outputs:			TRUE if in path, else FALSE
*
*****************************************************************************/
BOOL Longhunt_InProjectilePath(CGameObjectInstance *pThis, CLonghunt *pLonghunt)
{

#if DEBUG_LONGHUNT
	if (BOSS_InProjectilePath(pThis, (CBoss *)pLonghunt, LONGHUNT_PROJECTILE_ANGLE))
	{
		rmonPrintf("In projectile path!\r\n") ;
		return TRUE ;
	}
	else
		return FALSE ;
#else

	return (BOSS_InProjectilePath(pThis, (CBoss *)pLonghunt,
			  LONGHUNT_PROJECTILE_ANGLE)) ;

#endif

}




/*****************************************************************************
*
*	Function:		Longhunt_CheckForEvade()
*
******************************************************************************
*
*	Description:	Checks to see if the longhunter should evade - if so evades
*						in the correct direction.
*
*	Inputs:			*pThis		-	Ptr to game object instance
*						*pLonghunt	-	Ptr to boss vars
*
*	Outputs:			None
*
*****************************************************************************/
void Longhunt_CheckForEvade(CGameObjectInstance *pThis, CLonghunt *pLonghunt)
{
	// Decrease evade timer
	pLonghunt->m_EvadeTime -= frame_increment ;
	if (pLonghunt->m_EvadeTime < 0)
	{
		// Swap evade states
		pLonghunt->m_EvadeActive ^= TRUE ;

		// Reset evade/non-evade time
		if (pLonghunt->m_EvadeActive)
			pLonghunt->m_EvadeTime = pLonghunt->m_pStageInfo->EvadeTime ;
		else
			pLonghunt->m_EvadeTime = pLonghunt->m_pStageInfo->NoEvadeTime ;
	}

	// Can longhunter evade in this mode?
	if ((pLonghunt->m_Boss.m_ModeFlags & BF_CAN_EVADE) &&
		 (pLonghunt->m_EvadeActive) &&
 		 (++pLonghunt->m_EvadeCount >= pLonghunt->m_pStageInfo->EvadeSkips) &&
		 (Longhunt_InProjectilePath(pThis, pLonghunt)))
	{
		pLonghunt->m_EvadeCount = 0 ;

		// Evade to left or right?
		if (pLonghunt->m_Boss.m_TurokAngleDiff < 0)
			Longhunt_SetupMode(pThis, LONGHUNT_EVADE_LEFT_MODE) ;
		else
			Longhunt_SetupMode(pThis, LONGHUNT_EVADE_RIGHT_MODE) ;
	}
}





/*****************************************************************************
*
*	Function:		Longhunt_CheckForWalkRunAttack()
*
******************************************************************************
*
*	Description:	Checks to see if the longhunter should perform a run attack
*
*	Inputs:			*pThis		-	Ptr to game object instance
*						*pLonghunt	-	Ptr to boss vars
*
*	Outputs:			TRUE - if attack is started, else FALSE
*
*****************************************************************************/
BOOL Longhunt_CheckForWalkRunAttack(CGameObjectInstance *pThis, CLonghunt *pLonghunt)
{
	// Must be in attack action
	if (pLonghunt->m_Boss.m_Action != LONGHUNT_ATTACK_ACTION)
		return FALSE ;

	// Update timer
	if (pLonghunt->m_Boss.m_ModeFlags & BF_WALKING_MODE)
		pLonghunt->m_RunAttackTime += frame_increment*2 ;
	else
	if (pLonghunt->m_Boss.m_ModeFlags & BF_RUNNING_MODE)
		pLonghunt->m_RunAttackTime += frame_increment ;

	// Been /walking running to long?
	if (pLonghunt->m_RunAttackTime > pLonghunt->m_pStageInfo->RunAttackTime)
	{
		// Reset time
		pLonghunt->m_RunAttackTime = 0 ;

		// Perform attack
		if (pLonghunt->m_Boss.m_ModeFlags & BF_WALKING_MODE)
			Longhunt_Setup_WalkAttack(pThis, pLonghunt) ;
		else
		if (pLonghunt->m_Boss.m_ModeFlags & BF_RUNNING_MODE)
			Longhunt_Setup_RunAttack(pThis, pLonghunt) ;
	}

	// Return true if mode changed
	return (pLonghunt->m_Boss.m_OldMode != pLonghunt->m_Boss.m_Mode) ;
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
//	Modes:		 LONGHUNT_WAIT_FOR_START_MODE,
//	Description: Waiting turok to try get keys
/////////////////////////////////////////////////////////////////////////////
void Longhunt_Code_WAIT_FOR_START(CGameObjectInstance *pThis, CLonghunt *pLonghunt)
{
	CGameObjectInstance *pTurok = CEngineApp__GetPlayer(GetApp()) ;
	CGameRegion	*pRegion ;
	CVector3		vPos ;

//	osSyncPrintf("SwitchesOn:%d\r\n", GetApp()->m_BossVars.m_SwitchesOn) ;

#if MAKE_LONGHUNT_START_REGARDLESS_OF_HUMVEES

	// Add boss bar and start pickups
	COnScreen__AddBossBar(&GetApp()->m_OnScreen, (CBoss *)pLonghunt) ;
	AI_GetDyn(pThis)->m_Invincible = FALSE ;
	CPickupMonitor__Activate(&GetApp()->m_Scene.m_PickupMonitor) ;

	// Set longhunter position
	CVector3__Set(&vPos, -60*SCALING_FACTOR,0,0) ;
	pRegion = CScene__NearestRegion(&GetApp()->m_Scene, &vPos) ;
	pThis->ah.ih.m_vPos = vPos ;
	pThis->ah.ih.m_pCurrentRegion = pRegion ;
	AI_NudgeRotY(pThis, AI_GetAngle(pThis, pTurok->ah.ih.m_vPos)) ;

	// Pu into start modes
	pLonghunt->m_Boss.m_Mode = LONGHUNT_RUN_MODE ;
	pLonghunt->m_Boss.m_Action = LONGHUNT_ATTACK_ACTION ;


#else
	// Player trying to get key?
	if ((CInstanceHdr__IsOnGround(&pTurok->ah.ih)) && (ci_player.pWallCollisionRegion))
	{
		if ((SQR(pTurok->ah.ih.m_vPos.x) + SQR(pTurok->ah.ih.m_vPos.z)) < SQR(60*SCALING_FACTOR))
		{
			// Find collision free position
			if (BOSS_OffsetFromPlayerObstructed(pThis, 15*SCALING_FACTOR,ANGLE_DTOR(180), &pRegion, &vPos))
			{
				if (BOSS_OffsetFromPlayerObstructed(pThis, 15*SCALING_FACTOR,ANGLE_DTOR(90), &pRegion, &vPos))
				{
					if (BOSS_OffsetFromPlayerObstructed(pThis, 15*SCALING_FACTOR,ANGLE_DTOR(-90), &pRegion, &vPos))
						return ;
				}
			}

			// Set longhunter position
			pThis->ah.ih.m_vPos = vPos ;
			pThis->ah.ih.m_pCurrentRegion = pRegion ;
			AI_NudgeRotY(pThis, AI_GetAngle(pThis, pTurok->ah.ih.m_vPos)) ;

			// Make camera track
			CCamera__SetObjectToTrack(&GetApp()->m_Camera, pThis) ;
			CPickupMonitor__Activate(&GetApp()->m_Scene.m_PickupMonitor) ;

			// Summon that jeep
			pLonghunt->m_SoundTimer = SECONDS_TO_FRAMES(1.5) ;
			Longhunt_SetupMode(pThis, LONGHUNT_SUMMON_HUMVEE_MODE) ;

			// Goto cinema
			CCamera__FadeToCinema(&GetApp()->m_Camera, CINEMA_FLAG_PLAY_LONGHUNTER) ;
		}
	}
#endif
}


/////////////////////////////////////////////////////////////////////////////
//	Modes:		 LONGHUNT_SUMMON_HUMVEE_MODE
//	Description: Summon on jumping humvee!
/////////////////////////////////////////////////////////////////////////////
void Longhunt_Code_SUMMON_HUMVEE(CGameObjectInstance *pThis, CLonghunt *pLonghunt)
{
	// Play c'mon sound
	if (pLonghunt->m_SoundTimer <= 0)
	{
		AI_SEvent_GeneralSound(&pThis->ah.ih, pThis->ah.ih.m_vPos, SOUND_LONGHUNTER_TAUNT_1) ;
		pLonghunt->m_SoundTimer = RandomRangeFLOAT(SECONDS_TO_FRAMES(8), SECONDS_TO_FRAMES(12)) ;
	}

	// Finished?
	if (pThis->m_asCurrent.m_cFrame >= (pThis->m_asCurrent.m_nFrames - 5))
	{
		Longhunt_ChooseRunningTarget(pThis, pLonghunt) ;
		Longhunt_SetupMode(pThis, LONGHUNT_RUN_TURN_LEFT90_MODE) ;

		// Signal humvee to start
		GetApp()->m_BossFlags |= BOSS_FLAG_LONGHUNTER_START_HUMVEE1 ;
		GetApp()->m_BossVars.m_StartBossTimer = 0 ;

		// Don't track longhunter running off!
		CCamera__StopTrackingObject(&GetApp()->m_Camera) ;
	}
}


/////////////////////////////////////////////////////////////////////////////
//	Modes:		 LONGHUNT_WAIT_FOR_HUMVEE_ENTRANCE_MODE
//	Description: Wait until humvee has finished entering
/////////////////////////////////////////////////////////////////////////////
void Longhunt_Code_WAIT_FOR_HUMVEE_ENTRANCE(CGameObjectInstance *pThis, CLonghunt *pLonghunt)
{
}

/////////////////////////////////////////////////////////////////////////////
//	Modes:		 LONGHUNT_WAIT_FOR_HUMVEES_DEATH_MODE
//	Description: Waiting for humvee to die before attacking
/////////////////////////////////////////////////////////////////////////////
void Longhunt_Code_WAIT_FOR_HUMVEES_DEATH(CGameObjectInstance *pThis, CLonghunt *pLonghunt)
{
}







/////////////////////////////////////////////////////////////////////////////
//	Modes:
//	Description: Movement ai
/////////////////////////////////////////////////////////////////////////////
void Longhunt_Code_Movement(CGameObjectInstance *pThis, CLonghunt *pLonghunt)
{
	FLOAT	DeltaAngle = pLonghunt->m_Boss.m_DeltaAngle ;
	FLOAT		WalkDist ;

	// Get real close to target position if turok is dead
	if (CTurokMovement.Active)
		WalkDist = LONGHUNT_WALK_DIST ;
	else
		WalkDist = 6*SCALING_FACTOR ;

	// Within run distance?
	if ((pLonghunt->m_Boss.m_DistFromTarget > LONGHUNT_RUN_DIST) ||
		 (pLonghunt->m_Boss.m_ModeFlags & BF_RUNNING_MODE))
	{
		// Perform attack?
		if (!Longhunt_CheckForWalkRunAttack(pThis, pLonghunt))
		{
			// Turn?
			if (pLonghunt->m_Boss.m_Mode != LONGHUNT_RUN_TURN180_MODE)
			{
				// Perform gradual turn
				BOSS_PerformTurn(pThis, (CBoss *)pLonghunt, LONGHUNT_RUN_TURN_SPEED, DeltaAngle) ;

				// Turn 180?
				if (DeltaAngle < -LONGHUNT_RUN_TURN180_ANGLE)
					pLonghunt->m_Boss.m_Mode = LONGHUNT_RUN_TURN180_MODE  ;
				else
				// Turn left?
				if (DeltaAngle < -LONGHUNT_RUN_TURN90_ANGLE)
					pLonghunt->m_Boss.m_Mode = LONGHUNT_RUN_TURN_LEFT90_MODE  ;
				else
				// Turn Right?
				if (DeltaAngle > LONGHUNT_RUN_TURN90_ANGLE)
					pLonghunt->m_Boss.m_Mode = LONGHUNT_RUN_TURN_RIGHT90_MODE  ;
			}

			// Perform run
			if (abs(DeltaAngle) <= LONGHUNT_RUN_FACING_ANGLE)
			{
				// Put into run mode
				pLonghunt->m_Boss.m_Mode = LONGHUNT_RUN_MODE ;
			}
		}
	}
	else
	// Within walk distance?
	if (pLonghunt->m_Boss.m_DistFromTarget	> WalkDist)
	{
		// Perform attack?
		if (!Longhunt_CheckForWalkRunAttack(pThis, pLonghunt))
		{
			// Turn?
			if (pLonghunt->m_Boss.m_Mode != LONGHUNT_WALK_TURN180_MODE)
			{
				// Perform gradual turn
				BOSS_PerformTurn(pThis, (CBoss *)pLonghunt, LONGHUNT_WALK_TURN_SPEED, DeltaAngle) ;

				// Turn 180?
				if (DeltaAngle < -LONGHUNT_WALK_TURN180_ANGLE)
					pLonghunt->m_Boss.m_Mode = LONGHUNT_WALK_TURN180_MODE  ;
				else
				// Turn left?
				if (DeltaAngle < -LONGHUNT_WALK_TURN90_ANGLE)
					pLonghunt->m_Boss.m_Mode = LONGHUNT_WALK_TURN_LEFT90_MODE  ;
				else
				// Turn Right?
				if (DeltaAngle > LONGHUNT_WALK_TURN90_ANGLE)
					pLonghunt->m_Boss.m_Mode = LONGHUNT_WALK_TURN_RIGHT90_MODE  ;
			}

			// Stop turning
			if (abs(DeltaAngle) <= LONGHUNT_WALK_FACING_ANGLE)
			{
				// Put into run mode
				pLonghunt->m_Boss.m_Mode = LONGHUNT_WALK_MODE ;
			}
		}
	}

	// Attack?
	if (pLonghunt->m_Boss.m_DistFromTarget <= WalkDist)
	{
		DeltaAngle = pLonghunt->m_Boss.m_LookDeltaAngle ;

		// Turn 180?
		if (pLonghunt->m_Boss.m_Mode != LONGHUNT_STAND_TURN180_MODE)
		{
			if (DeltaAngle < -LONGHUNT_STAND_TURN180_ANGLE)
				pLonghunt->m_Boss.m_Mode = LONGHUNT_STAND_TURN180_MODE ;
			else
			// Turn left, right or walk?
			if (DeltaAngle < -LONGHUNT_STAND_TURN90_ANGLE)
				pLonghunt->m_Boss.m_Mode = LONGHUNT_STAND_TURN_LEFT90_MODE ;
			else
			if (DeltaAngle > LONGHUNT_STAND_TURN90_ANGLE)
				pLonghunt->m_Boss.m_Mode = LONGHUNT_STAND_TURN_RIGHT90_MODE ;
		}

		// Perform attack
		if (abs(DeltaAngle) <= LONGHUNT_STAND_FACING_ANGLE)
		{
			pLonghunt->m_Boss.m_Mode = LONGHUNT_STAND_IDLE_MODE ;

			// Only attack if not waiting for humvees
			if (pLonghunt->m_Boss.m_Action != LONGHUNT_WAIT_FOR_HUMVEES_DEATH_ACTION)
				Longhunt_Setup_CloseAttack(pThis, pLonghunt) ;
		}
	}
}


/////////////////////////////////////////////////////////////////////////////
//	Modes:		 LONGHUNT_EVADE_LEFT_MODE
//					 LONGHUNT_EVADE_RIGHT_MODE
//	Description: Evading to the side in a stylish roll type manner
/////////////////////////////////////////////////////////////////////////////
void Longhunt_Code_Evade(CGameObjectInstance *pThis, CLonghunt *pLonghunt)
{
	// Finished?
	if (pThis->m_asCurrent.m_cFrame >= (pThis->m_asCurrent.m_nFrames - 5))
		Longhunt_SetupMode(pThis, LONGHUNT_STAND_IDLE_MODE) ;
}



/////////////////////////////////////////////////////////////////////////////
//	Modes:		 LONGHUNT_ROUNDHOUSE_MODE
//					 LONGHUNT_JUMP_KICK_MODE
//					 LONGHUNT_SWING_PUNCH_MODE
//	Description: Generic close up attack moves
/////////////////////////////////////////////////////////////////////////////

void Longhunt_PlayAttackTauntSount(CGameObjectInstance *pThis, CLonghunt *pLonghunt)
{
	if (pLonghunt->m_SoundTimer <= 0)
	{
		pLonghunt->m_SoundTimer = RandomRangeFLOAT(SECONDS_TO_FRAMES(8), SECONDS_TO_FRAMES(12)) ;

		// Play sound
		if (RANDOM(2))
			AI_SEvent_GeneralSound(&pThis->ah.ih, pThis->ah.ih.m_vPos, SOUND_LONGHUNTER_TAUNT_1) ;	// Hey
		else
			AI_SEvent_GeneralSound(&pThis->ah.ih, pThis->ah.ih.m_vPos, SOUND_LONGHUNTER_TAUNT_2) ;	// Common
	}
}

void Longhunt_Setup_CloseAttack(CGameObjectInstance *pThis, CLonghunt *pLonghunt)
{
	Longhunt_PlayAttackTauntSount(pThis, pLonghunt) ;
	Longhunt_SetupMode(pThis, CSelection__Choose(LonghuntCloseAttackSelection)) ;
	pLonghunt->m_Boss.m_OldMode = -1 ;
}

void Longhunt_Code_CloseAttack(CGameObjectInstance *pThis, CLonghunt *pLonghunt)
{
	// Finished?
	if (pThis->m_asCurrent.m_cFrame >= (pThis->m_asCurrent.m_nFrames - 5))
			Longhunt_SetupMode(pThis, LONGHUNT_STAND_IDLE_MODE) ;
}


/////////////////////////////////////////////////////////////////////////////
//	Modes: 		 LONGHUNT_JUMP_KICK_MODE
//					 LONGHUNT_FIRE_GUN_MODE
//	Description: Walking attack moves
/////////////////////////////////////////////////////////////////////////////
void Longhunt_Setup_WalkAttack(CGameObjectInstance *pThis, CLonghunt *pLonghunt)
{
	Longhunt_PlayAttackTauntSount(pThis, pLonghunt) ;
	Longhunt_SetupMode(pThis, CSelection__Choose(LonghuntWalkAttackSelection)) ;
	pLonghunt->m_Boss.m_OldMode = -1 ;
}

void Longhunt_Code_WalkAttack(CGameObjectInstance *pThis, CLonghunt *pLonghunt)
{
	// Looping modes
	switch(pLonghunt->m_Boss.m_Mode)
	{
		case LONGHUNT_FIRE_GUN_MODE:
		case LONGHUNT_THROW_GRENADE_MODE:
		case LONGHUNT_RUN_FIRE_GUN_MODE:
			
			// Fired gun enough (note - blend cause one extra fire!)?
			if (pLonghunt->m_Boss.m_AnimPlayed < (pLonghunt->m_pStageInfo->GunFireTotal-1))
				return ;
	}

	// Finished?
	switch(pLonghunt->m_Boss.m_Mode)
	{
		case LONGHUNT_FIRE_GUN_MODE:
		case LONGHUNT_RUN_FIRE_GUN_MODE:

			// Finished?
			if (pThis->m_asCurrent.m_cFrame >= (pThis->m_asCurrent.m_nFrames - 2))
					Longhunt_SetupMode(pThis, LONGHUNT_WALK_MODE) ;
			break ;

		default:

			// Finished?
			if (pThis->m_asCurrent.m_cFrame >= (pThis->m_asCurrent.m_nFrames - 5))
					Longhunt_SetupMode(pThis, LONGHUNT_WALK_MODE) ;
	}
}



/////////////////////////////////////////////////////////////////////////////
//	Modes:		 LONGHUNT_FIRE_GUN_MODE
//					 LONGHUNT_THROW_GRENADE_MODE
//					 LONGHUNT_RUN_FIRE_GUN_MODE
//	Description: Attacks during running
/////////////////////////////////////////////////////////////////////////////
void Longhunt_Setup_RunAttack(CGameObjectInstance *pThis, CLonghunt *pLonghunt)
{
	Longhunt_PlayAttackTauntSount(pThis, pLonghunt) ;
	Longhunt_SetupMode(pThis, CSelection__Choose(LonghuntRunAttackSelection)) ;
	pLonghunt->m_Boss.m_OldMode = -1 ;
}

void Longhunt_Code_RunAttack(CGameObjectInstance *pThis, CLonghunt *pLonghunt)
{
	// Looping modes
	switch(pLonghunt->m_Boss.m_Mode)
	{
		case LONGHUNT_FIRE_GUN_MODE:
		case LONGHUNT_THROW_GRENADE_MODE:
		case LONGHUNT_RUN_FIRE_GUN_MODE:
			
			// Fired gun enough?
			if (pLonghunt->m_Boss.m_AnimPlayed < pLonghunt->m_pStageInfo->GunFireTotal)
				return ;
	}


	// Finished?
	switch(pLonghunt->m_Boss.m_Mode)
	{
		case LONGHUNT_FIRE_GUN_MODE:
		case LONGHUNT_RUN_FIRE_GUN_MODE:

			// Finished?
			if (pThis->m_asCurrent.m_cFrame >= (pThis->m_asCurrent.m_nFrames - 2))
					Longhunt_SetupMode(pThis, LONGHUNT_RUN_MODE) ;
			break ;

		default:
			// Finished?
			if (pThis->m_asCurrent.m_cFrame >= (pThis->m_asCurrent.m_nFrames - 5))
					Longhunt_SetupMode(pThis, LONGHUNT_RUN_MODE) ;
	}
}



/////////////////////////////////////////////////////////////////////////////
//	Modes:		 LONGHUNT_TAUNT1_MODE
//	Description: Waving hand at player
/////////////////////////////////////////////////////////////////////////////
void Longhunt_Setup_Taunt(CGameObjectInstance *pThis, CLonghunt *pLonghunt)
{
	pLonghunt->m_Boss.m_Mode = LONGHUNT_TAUNT1_MODE ;

	// Play sound
	if (CCamera__InCinemaMode(&GetApp()->m_Camera))
		AI_SEvent_GeneralSound(&pThis->ah.ih, pThis->ah.ih.m_vPos, SOUND_LONGHUNTER_TAUNT_2) ;	// c'mon
	else
	if (RANDOM(2))
		AI_SEvent_GeneralSound(&pThis->ah.ih, pThis->ah.ih.m_vPos, SOUND_LONGHUNTER_TAUNT_1) ;	// hey
	else
		AI_SEvent_GeneralSound(&pThis->ah.ih, pThis->ah.ih.m_vPos, SOUND_LONGHUNTER_TAUNT_2) ;	// c'mon
}

void Longhunt_Code_Taunt(CGameObjectInstance *pThis, CLonghunt *pLonghunt)
{
	// Finished?
	if (pThis->m_asCurrent.m_cFrame >= (pThis->m_asCurrent.m_nFrames - 5))
		Longhunt_SetupMode(pThis, LONGHUNT_STAND_IDLE_MODE) ;
}


/////////////////////////////////////////////////////////////////////////////
//	Modes:		 LONGHUNT_DEATH_MODE
//	Description: Let's player get keys
/////////////////////////////////////////////////////////////////////////////

void Longhunt_OpenForceField(CGameObjectInstance *pThis, CLonghunt *pLonghunt)
{
	CROMRegionSet	*pRegionSet;
	CGameRegion		*pRegion ;
	CVector3			vDesiredPos ;

	if (GetApp()->m_BossFlags & BOSS_FLAG_LONGHUNTER_FORCEFIELD_GONE)
	{

		// Open force field door
		vDesiredPos.x = 17.29 * SCALING_FACTOR ;
		vDesiredPos.y = 0 ;
		vDesiredPos.z = 5.14 * SCALING_FACTOR ;
		pRegion = CScene__NearestRegion(&GetApp()->m_Scene, &vDesiredPos) ;
		CGameRegion__OpenDoor(pRegion) ;

		// Switch floor to stone
		pRegionSet = CScene__GetRegionAttributes(&GetApp()->m_Scene, pRegion);
		if (pRegionSet)
			pRegionSet->m_GroundMat = REGMAT_STONE ;
	}
}

void Longhunt_OpenDamWalls(CGameObjectInstance *pThis, CLonghunt *pLonghunt)
{
	CInstanceHdr	*pOrigin = &pThis->ah.ih ;
	INT32				i ;

	// Trigger dam walls (id's are 11,12,13,14) and water
	for (i = LONGHUNT_LEVEL_DAM_WALL1_ID ; i <= LONGHUNT_LEVEL_DAM_WALL4_ID ; i++)
	{
		AI_Event_Dispatcher(pOrigin, pOrigin, AI_MEVENT_PRESSUREPLATE,
								  AI_SPECIES_ALL, pOrigin->m_vPos, (FLOAT)i) ;
	}

	// Trigger elevator in middle of arena to rise?
	AI_Event_Dispatcher(pOrigin, pOrigin, AI_MEVENT_PRESSUREPLATE,
							  AI_SPECIES_ALL, pOrigin->m_vPos, LONGHUNT_LEVEL_ELEVATOR_ID) ;
}



void Longhunt_Setup_DEATH(CGameObjectInstance *pThis, CLonghunt *pLonghunt)
{
	CInstanceHdr	*pOrigin = (CInstanceHdr *)pThis ;
	CLine				Line ;
	CVector3			vMiddle, vPos ;
	CDynamicSimple	*pPickup ;

	// Play sound
	AI_SEvent_GeneralSound(&pThis->ah.ih, pThis->ah.ih.m_vPos, SOUND_LONGHUNTER_TAUNT_3) ;

	// Put into mode
	pLonghunt->m_Boss.m_Mode = LONGHUNT_DEATH_MODE ;

	// Open door region in middle of arena
	Longhunt_OpenForceField(pThis, pLonghunt) ;

	// Turn off the force field object
	AI_Event_Dispatcher(pOrigin, pOrigin, AI_MEVENT_PRESSUREPLATE, pOrigin->m_pEA->m_dwSpecies, pOrigin->m_vPos, LONGHUNT_LEVEL_FORCEFIELD_ID) ;
	GetApp()->m_BossFlags |= BOSS_FLAG_LONGHUNTER_FORCEFIELD_GONE ;

	// tell the engine that the main boss for this level is dead
	//BOSS_LevelComplete() ;

	CCamera__SetObjectToTrack(&GetApp()->m_Camera, pThis) ;
	CPickupMonitor__Deactivate(&GetApp()->m_Scene.m_PickupMonitor) ;

	// Generate longhunter gun pickup and record status
	if (!(GetApp()->m_BossFlags & BOSS_FLAG_LONGHUNTER_DEAD))
	{
		CVector3__Set(&vMiddle,0,0,0) ;
		CLine__Construct(&Line, &pThis->ah.ih.m_vPos, &vMiddle) ;
		CLine__CalculateXZDirection(&Line) ;
		vPos = pThis->ah.ih.m_vPos ;
		vPos.x += 15*SCALING_FACTOR * Line.m_vDir.x ;
		vPos.z += 15*SCALING_FACTOR * Line.m_vDir.z ;

		// Start and bounce the pickup
		pPickup = AI_DoPickup(&pThis->ah.ih, vPos, AI_OBJECT_PICKUP_MACHINEGUN,0) ;
		if (pPickup)
			pPickup->s.ah.m_vVelocity.y = 5*SCALING_FACTOR ;

		GetApp()->m_BossFlags |= BOSS_FLAG_LONGHUNTER_DEAD	;
	}

	// Start cinema
	CCamera__FadeToCinema(&GetApp()->m_Camera, CINEMA_FLAG_TUROK_KILL_LONGHUNTER) ;
	pLonghunt->m_Boss.m_AnimPlayed = FALSE ;
}

void Longhunt_Code_DEATH(CGameObjectInstance *pThis, CLonghunt *pLonghunt)
{
	// Trigger the dam walls to open and stop camera tracking him
	if (pLonghunt->m_Boss.m_AnimPlayed)
	{
		Longhunt_OpenDamWalls(pThis, pLonghunt) ;
		CCamera__StopTrackingObject(&GetApp()->m_Camera) ;
	}
}



/*****************************************************************************
*
*	Function:		Longhunt_DisplayMode()
*
******************************************************************************
*
*	Description:	Displays current mode when debug is set
*
*	Inputs:			*pBoo	-	Ptr to boss vars
*
*	Outputs:			None
*
*****************************************************************************/
#ifndef MAKE_CART

void Longhunt_DisplayMode(CBoss *pBoss)
{
#if DEBUG_LONGHUNT
	switch(pBoss->m_Mode)
	{
		case LONGHUNT_WAIT_FOR_START_MODE:
			rmonPrintf("LONGHUNT_WAIT_FOR_START_MODE\r\n") ;
			break ;
		case LONGHUNT_SUMMON_HUMVEE_MODE:
			rmonPrintf("LONGHUNT_SUMMON_HUMVEE_MODE\r\n") ;
			break ;
		case LONGHUNT_WAIT_FOR_HUMVEE_ENTRANCE_MODE:
			rmonPrintf("LONGHUNT_WAIT_FOR_HUMVEE_ENTRANCE_MODE\r\n") ;
			break ;
		case LONGHUNT_WAIT_FOR_HUMVEES_DEATH_MODE:
			rmonPrintf("LONGHUNT_WAIT_FOR_HUMVEES_DEATH_MODE\r\n") ;
			break ;

		case LONGHUNT_STAND_IDLE_MODE:
			rmonPrintf("LONGHUNT_STAND_IDLE_MODE\r\n") ;
			break ;
		case LONGHUNT_STAND_TURN180_MODE:
			rmonPrintf("LONGHUNT_STAND_TURN180_MODE\r\n") ;
			break ;
		case LONGHUNT_STAND_TURN_LEFT90_MODE:
			rmonPrintf("LONGHUNT_STAND_TURN_LEFT90_MODE\r\n") ;
			break ;
		case LONGHUNT_STAND_TURN_RIGHT90_MODE:
			rmonPrintf("LONGHUNT_STAND_TURN_RIGHT90_MODE\r\n") ;
			break ;
		case LONGHUNT_TAUNT1_MODE:
			rmonPrintf("LONGHUNT_TAUNT1_MODE\r\n") ;
			break ;
		case LONGHUNT_WALK_MODE:
			rmonPrintf("LONGHUNT_WALK_MODE\r\n") ;
			break ;
		case LONGHUNT_WALK_TURN180_MODE:
			rmonPrintf("LONGHUNT_WALK_TURN180_MODE\r\n") ;
			break ;
		case LONGHUNT_WALK_TURN_LEFT90_MODE:
			rmonPrintf("LONGHUNT_WALK_TURN_LEFT90_MODE\r\n") ;
			break ;
		case LONGHUNT_WALK_TURN_RIGHT90_MODE:
			rmonPrintf("LONGHUNT_WALK_TURN_RIGHT90_MODE\r\n") ;
			break ;
		case LONGHUNT_RUN_MODE:
			rmonPrintf("LONGHUNT_RUN_MODE\r\n") ;
			break ;
		case LONGHUNT_RUN_TURN180_MODE:
			rmonPrintf("LONGHUNT_RUN_TURN180_MODE\r\n") ;
			break ;
		case LONGHUNT_RUN_TURN_LEFT90_MODE:
			rmonPrintf("LONGHUNT_RUN_TURN_LEFT90_MODE\r\n") ;
			break ;
		case LONGHUNT_RUN_TURN_RIGHT90_MODE:
			rmonPrintf("LONGHUNT_RUN_TURN_RIGHT90_MODE\r\n") ;
			break ;
		case LONGHUNT_EVADE_LEFT_MODE:
			rmonPrintf("LONGHUNT_EVADE_LEFT_MODE\r\n") ;
			break ;
		case LONGHUNT_EVADE_RIGHT_MODE:
			rmonPrintf("LONGHUNT_EVADE_RIGHT_MODE\r\n") ;
			break ;
		case LONGHUNT_ROUNDHOUSE_MODE:
			rmonPrintf("LONGHUNT_ROUNDHOUSE_MODE\r\n") ;
			break ;
		case LONGHUNT_JUMP_KICK_MODE:
			rmonPrintf("LONGHUNT_JUMP_KICK_MODE\r\n") ;
			break ;
		case LONGHUNT_SWING_PUNCH_MODE:
			rmonPrintf("LONGHUNT_SWING_PUNCH_MODE\r\n") ;
			break ;

		case LONGHUNT_LOAD_NET_MODE:
			rmonPrintf("LONGHUNT_LOAD_NET_MODE\r\n") ;
			break ;
		case LONGHUNT_FIRE_NET_MODE:
			rmonPrintf("LONGHUNT_FIRE_NET_MODE\r\n") ;
			break ;
		case LONGHUNT_LOAD_GRENADE_MODE:
			rmonPrintf("LONGHUNT_LOAD_GRENADE_MODE\r\n") ;
			break ;
		case LONGHUNT_FIRE_GRENADE_MODE:
			rmonPrintf("LONGHUNT_FIRE_GRENADE_MODE\r\n") ;
			break ;
		case LONGHUNT_LEAP_SMASH_ROLL_MODE:
			rmonPrintf("LONGHUNT_LEAP_SMASH_ROLL_MODE\r\n") ;
			break ;

		case LONGHUNT_BACK_FLINCH_MODE:
			rmonPrintf("LONGHUNT_BACK_FLINCH_MODE\r\n") ;
			break ;
		case LONGHUNT_HEAD_FLINCH_MODE:
			rmonPrintf("LONGHUNT_HEAD_FLINCH_MODE\r\n") ;
			break ;
		case LONGHUNT_LEFT_SHOULDER_FLINCH_MODE:
			rmonPrintf("LONGHUNT_LEFT_SHOULDER_FLINCH_MODE\r\n") ;
			break ;
		case LONGHUNT_RIGHT_SHOULDER_FLINCH_MODE:
			rmonPrintf("LONGHUNT_RIGHT_SHOULDER_FLINCH_MODE\r\n") ;
			break ;
		case LONGHUNT_STOMACH_FLINCH_MODE:
			rmonPrintf("LONGHUNT_STOMACH_FLINCH_MODE\r\n") ;
			break ;
		case LONGHUNT_KNEE_FLINCH_MODE:
			rmonPrintf("LONGHUNT_KNEE_FLINCH_MODE\r\n") ;
			break ;


		case LONGHUNT_DEATH_MODE:
			rmonPrintf("LONGHUNT_DEATH_MODE\r\n") ;
			break ;


		default:
			rmonPrintf("Mode: %d\r\n", pBoss->m_Mode) ;
			break ;
	}
#endif
}
#endif



/////////////////////////////////////////////////////////////////////////////
// Longhunter model index
/////////////////////////////////////////////////////////////////////////////
int CGameObjectInstance__GetLonghunterModelIndex(CGameObjectInstance *pThis, int nNode)
{
	switch(nNode)
	{
		// Right hand (with gun)
		case 18:
			return (pThis->m_pBoss->m_Mode == LONGHUNT_DEATH_MODE) ;

		default:
			return 0 ;
	}
}


