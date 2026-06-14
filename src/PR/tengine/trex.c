// TRex boss control file by Stephen Broumley

#include "stdafx.h"

#include "ai.h"
#include "aistand.h"

#include "romstruc.h"
#include "tengine.h"
#include "scaling.h"
#include "audio.h"
#include "sfx.h"
#include "tmove.h"
#include "trex.h"
#include "boss.h"
#include "bossflgs.h"
#include "onscrn.h"
#include "dlists.h"
#include "regtype.h"
#include "wallcoll.h"
#include "audio.h"
#include "cammodes.h"




/////////////////////////////////////////////////////////////////////////////
// General constants
/////////////////////////////////////////////////////////////////////////////
#define DEBUG_TREX	0
#define TREX_HEALTH	6000
//#define TREX_HEALTH	5





/////////////////////////////////////////////////////////////////////////////
// Movement constants
/////////////////////////////////////////////////////////////////////////////
#define TREX_GRAVITY							GRAVITY_ACCELERATION

#define TREX_TARGET_DIST					(0*SCALING_FACTOR)
#define TREX_AT_TARGET_DIST				(40*SCALING_FACTOR)

#define TREX_LEDGE_TARGET_DIST			(28*SCALING_FACTOR)
#define TREX_LEDGE_AT_TARGET_DIST 		(12*SCALING_FACTOR)

#define TREX_CLOSE_ATTACK_RANGE			(17*SCALING_FACTOR)

#define TREX_IDLE_TURN90_ANGLE			ANGLE_DTOR(35)
#define TREX_IDLE_TURN180_ANGLE			ANGLE_DTOR(150)
#define TREX_IDLE_TURN_SPEED				ANGLE_DTOR(4)
#define TREX_IDLE_FACING_ANGLE			ANGLE_DTOR(15)

#define TREX_RUN_BITE_DIST					(20*SCALING_FACTOR)
#define TREX_RUN_TURN90_ANGLE				ANGLE_DTOR(35)
#define TREX_RUN_TURN_SPEED				ANGLE_DTOR(8)
#define TREX_RUN_FACING_ANGLE				ANGLE_DTOR(15)

#define TREX_LIMP_TURN90_ANGLE			ANGLE_DTOR(35)
#define TREX_LIMP_TURN_SPEED				ANGLE_DTOR(8)
#define TREX_LIMP_FACING_ANGLE			ANGLE_DTOR(15)

#define TREX_MAX_RUN_TURN_ATTACK_DIST	(200*SCALING_FACTOR)		// Dist attacks can be in
#define TREX_MAX_RUN_TIME					SECONDS_TO_FRAMES(2)		// Time before attack
#define TREX_MAX_TURN_TIME					SECONDS_TO_FRAMES(2)		// Time before attack

#define TREX_STUNNED_MAX_ANIM_SPEED		4.0
#define TREX_STUNNED_TIME					SECONDS_TO_FRAMES(2)

#define TREX_CAVE_CRUSH_TIME				SECONDS_TO_FRAMES(20)
#define TREX_CAVE_TARGET_DIST				(0*SCALING_FACTOR)
#define TREX_CAVE_AT_TARGET_DIST	 		(12*SCALING_FACTOR)
#define TREX_STOOP_DIST						(50*SCALING_FACTOR)	// Dist to get before stoop
#define TREX_AT_CAVE_DIST					(24*SCALING_FACTOR)	// Dist from centre of cave before attacking

#define TREX_TURN_ON_THE_SPOT_SPEED		ANGLE_DTOR(8)




/////////////////////////////////////////////////////////////////////////////
// Animation constants
/////////////////////////////////////////////////////////////////////////////

// Idle modes
#define AI_ANIM_BOSS_TREX_IDLE			AI_ANIM_IDLE
#define AI_ANIM_BOSS_TREX_LOOK			AI_ANIM_IDLE_INTEL1

#define AI_ANIM_BOSS_TREX_TURN_LEFT90	AI_ANIM_TURNL90
#define AI_ANIM_BOSS_TREX_TURN_RIGHT90	AI_ANIM_TURNR90
#define AI_ANIM_BOSS_TREX_TURN180		AI_ANIM_TURN180

// Move modes
#define AI_ANIM_BOSS_TREX_RUN						AI_ANIM_RUN
#define AI_ANIM_BOSS_TREX_RUN_TURN_LEFT90		AI_ANIM_RTURNL90
#define AI_ANIM_BOSS_TREX_RUN_TURN_RIGHT90	AI_ANIM_RTURNR90
#define AI_ANIM_BOSS_TREX_RUN_BITE				AI_ANIM_ATTACK_WEAK2

#define AI_ANIM_BOSS_TREX_LIMP					AI_ANIM_WALK
#define AI_ANIM_BOSS_TREX_LIMP_LEFT90			AI_ANIM_WTURNL90
#define AI_ANIM_BOSS_TREX_LIMP_RIGHT90			AI_ANIM_WTURNR90

// Attacks
#define AI_ANIM_BOSS_TREX_TAIL_SLAM				AI_ANIM_ATTACK_WEAK3
#define AI_ANIM_BOSS_TREX_BITE					AI_ANIM_ATTACK_STRONG7
#define AI_ANIM_BOSS_TREX_RAGE					AI_ANIM_RAGE

// Looping attacks
#define AI_ANIM_BOSS_TREX_BREATH_FIRE			AI_ANIM_ATTACK_STRONG9
#define AI_ANIM_BOSS_TREX_BREATH_FIRE_IN		AI_ANIM_ATTACK_STRONG10
#define AI_ANIM_BOSS_TREX_BREATH_FIRE_OUT		AI_ANIM_ATTACK_STRONG11

#define AI_ANIM_BOSS_TREX_EYE_FIRE				AI_ANIM_ATTACK_STRONG13
#define AI_ANIM_BOSS_TREX_EYE_FIRE_IN			AI_ANIM_ATTACK_STRONG14
#define AI_ANIM_BOSS_TREX_EYE_FIRE_OUT			AI_ANIM_ATTACK_WEAK1

// Up Attacks
#define AI_ANIM_BOSS_TREX_BITE_UP				AI_ANIM_ATTACK_STRONG8
#define AI_ANIM_BOSS_TREX_FIRE_UP				AI_ANIM_ATTACK_WEAK4

// Cave specials
#define AI_ANIM_BOSS_TREX_CAVE_STOOP		AI_ANIM_EVADE_CROUCH
#define AI_ANIM_BOSS_TREX_CAVE_LOOK			AI_ANIM_IDLE_INTEL2
#define AI_ANIM_BOSS_TREX_CAVE_BITE			AI_ANIM_ATTACK_STRONG12
#define AI_ANIM_BOSS_TREX_CAVE_NOSE_HIT	AI_ANIM_IMPACT_NORMAL
#define AI_ANIM_BOSS_TREX_CAVE_OUT			AI_ANIM_RETREAT

// Explode modes
#define AI_ANIM_BOSS_TREX_EXPLODE_LEFT		AI_ANIM_IMPACT_EXPLODE_LEFT
#define AI_ANIM_BOSS_TREX_EXPLODE_RIGHT	AI_ANIM_IMPACT_EXPLODE_RIGHT
#define AI_ANIM_BOSS_TREX_EXPLODE_FRONT	AI_ANIM_IMPACT_EXPLODE_FORWARD
#define AI_ANIM_BOSS_TREX_EXPLODE_BACK		AI_ANIM_IMPACT_EXPLODE_BACKWARD

// Hurt
#define AI_ANIM_BOSS_TREX_STUNNED			AI_ANIM_IMPACT_STUNNED
#define AI_ANIM_BOSS_TREX_STUN_OUT			AI_ANIM_ATTACK_WEAK5
#define AI_ANIM_BOSS_TREX_DEATH				AI_ANIM_DEATH_NORMAL

#define AI_ANIM_BOSS_TREX_EATTUROK			AI_ANIM_INTERACTIVE_ABS_31


/////////////////////////////////////////////////////////////////////////////
// Function prototypes
/////////////////////////////////////////////////////////////////////////////

// Initialisation
void CCollisionInfo__SetTRexCollisionDefaults(CCollisionInfo2 *pThis) ;
void TRex_SetupStageInfo(CGameObjectInstance *pThis, CTRex *pTRex, CTRexStageInfo *Info) ;
CBoss *TRex_Initialise(CGameObjectInstance *pThis) ;
void TRex_PreDraw(CGameObjectInstance *pThis, CTRex *pTRex, Gfx **ppDLP) ;
void TRex_PostDraw(CGameObjectInstance *pThis, CTRex *pTRex, Gfx **ppDLP) ;


// General code
void TRex_UpdateFireSound(CGameObjectInstance *pThis, CTRex *pTRex) ;
void TRex_UpdateLaserSound(CGameObjectInstance *pThis, CTRex *pTRex) ;
void TRex_UpdateSound(CGameObjectInstance *pThis, CTRex *pTRex) ;
void TRex_Update(CGameObjectInstance *pThis, CTRex *pTRex) ;
INT32 TRex_SetupMode(CGameObjectInstance *pThis, INT32 nNewMode) ;
void TRex_GetTarget(CGameObjectInstance *pThis, CTRex *pTRex) ;
BOOL TRex_CheckForTurokInCave(CGameObjectInstance *pThis, CTRex *pTRex) ;
void TRex_RunScriptCommand(CGameObjectInstance *pThis, CTRex *pTRex, CBossScript *pScript, INT32 EndOfAttackMode) ;
BOOL TRex_CheckForMaxRunTime(CGameObjectInstance *pThis, CTRex *pTRex)  ;
BOOL TRex_CheckForMaxTurnTime(CGameObjectInstance *pThis, CTRex *pTRex)  ;
BOOL TRex_CheckHitByExplosion(CGameObjectInstance *pThis, CTRex *pTRex) ;


// Mode code
void TRex_Code_OnElevator(CGameObjectInstance *pThis, CTRex *pTRex) ;

void TRex_Code_Movement(CGameObjectInstance *pThis, CTRex *pTRex) ;

void TRex_Code_RUN_BITE(CGameObjectInstance *pThis, CTRex *pTRex) ;

void TRex_Code_Attack(CGameObjectInstance *pThis, CTRex *pTRex) ;
void TRex_Code_LedgeAttack(CGameObjectInstance *pThis, CTRex *pTRex) ;
void TRex_Setup_BREATH_FIRE(CGameObjectInstance *pThis, CTRex *pTRex) ;
void TRex_Setup_EYE_FIRE(CGameObjectInstance *pThis, CTRex *pTRex) ;
void TRex_Setup_FireLoop(CGameObjectInstance *pThis, CTRex *pTRex, INT32 NewMode) ;
void TRex_Code_FireLoop(CGameObjectInstance *pThis, CTRex *pTRex) ;
void TRex_Code_FireIn(CGameObjectInstance *pThis, CTRex *pTRex) ;
void TRex_Code_FireOut(CGameObjectInstance *pThis, CTRex *pTRex) ;

void TRex_Code_CAVE_STOOP(CGameObjectInstance *pThis, CTRex *pTRex) ;
void TRex_Code_CaveAttack(CGameObjectInstance *pThis, CTRex *pTRex) ;
void TRex_Setup_CAVE_OUT(CGameObjectInstance *pThis, CTRex *pTRex) ;
void TRex_Code_CAVE_OUT(CGameObjectInstance *pThis, CTRex *pTRex) ;

void TRex_Code_CAVE_NOSE_HIT(CGameObjectInstance *pThis, CTRex *pTRex) ;
void TRex_Code_CAVE_RAGE(CGameObjectInstance *pThis, CTRex *pTRex) ;
void TRex_Code_CAVE_TAIL_SLAM(CGameObjectInstance *pThis, CTRex *pTRex) ;

void TRex_Code_Explode(CGameObjectInstance *pThis, CTRex *pTRex) ;

void TRex_Code_STUNNED(CGameObjectInstance *pThis, CTRex *pTRex) ;
void TRex_Code_STUN_OUT(CGameObjectInstance *pThis, CTRex *pTRex)  ;

void TRex_Code_EAT_TUROK(CGameObjectInstance *pThis, CTRex *pTRex) ;


#ifndef MAKE_CART
void TRex_DisplayMode(CBoss *pBoss) ;
#endif


/////////////////////////////////////////////////////////////////////////////
// Variables
/////////////////////////////////////////////////////////////////////////////

// TRex itself
CTRex				TRexBoss ;

// TRex mode table
CBossModeInfo	TRexModeTable[] =
{
	// Elevator modes
	BOSS_MODE_INFO(NULL,								TRex_Code_OnElevator,	AI_ANIM_BOSS_TREX_IDLE,1,BF_INVINCIBLE | BF_ENTER_MODE),
	BOSS_MODE_INFO(NULL,								TRex_Code_OnElevator,	AI_ANIM_BOSS_TREX_RAGE,1,BF_INVINCIBLE | BF_ENTER_MODE),

//	BOSS_MODE_INFO(NULL,								BOSS_Code_NextModeAfterAnim,			AI_ANIM_BOSS_TREX_LOOK,1,BF_INVINCIBLE | BF_ENTER_MODE | BF_LASER_ON),
	BOSS_MODE_INFO(NULL,								BOSS_Code_NextModeAfterAnim,			AI_ANIM_BOSS_TREX_BREATH_FIRE, 1, BF_INVINCIBLE | BF_ENTER_MODE | BF_LASER_ON),

	BOSS_MODE_INFO(NULL,								NULL,							AI_ANIM_BOSS_TREX_RUN,1.5,BF_HEAD_TRACK_TUROK | BF_INVINCIBLE | BF_ENTER_MODE | BF_LASER_ON),

	// Idle modes
	BOSS_MODE_INFO(TRex_Code_Movement,			TRex_Code_Movement,	AI_ANIM_BOSS_TREX_IDLE,1,BF_HEAD_TRACK_TUROK | BF_LASER_ON),
	BOSS_MODE_INFO(NULL,								TRex_Code_Attack,		AI_ANIM_BOSS_TREX_LOOK,1,0 | BF_LASER_ON),

	BOSS_MODE_INFO(NULL,								TRex_Code_Movement,	AI_ANIM_BOSS_TREX_TURN_LEFT90,1.5,	BF_RUNNING_MODE | BF_HEAD_TRACK_TUROK | BF_LASER_ON),
	BOSS_MODE_INFO(NULL,								TRex_Code_Movement,	AI_ANIM_BOSS_TREX_TURN_RIGHT90,1.5,BF_RUNNING_MODE | BF_HEAD_TRACK_TUROK | BF_LASER_ON),
	BOSS_MODE_INFO(NULL,								TRex_Code_Movement,	AI_ANIM_BOSS_TREX_TURN180,1.5,		BF_RUNNING_MODE | BF_HEAD_TRACK_TUROK | BF_LASER_ON),

	// Move modes
	BOSS_MODE_INFO(NULL,								TRex_Code_Movement,	AI_ANIM_BOSS_TREX_RUN,1.5,					BF_RUNNING_MODE | BF_HEAD_TRACK_TUROK | BF_LASER_ON),
	BOSS_MODE_INFO(NULL,								TRex_Code_Movement,	AI_ANIM_BOSS_TREX_RUN_TURN_LEFT90,1.5,	BF_RUNNING_MODE | BF_HEAD_TRACK_TUROK | BF_LASER_ON),
	BOSS_MODE_INFO(NULL,								TRex_Code_Movement,	AI_ANIM_BOSS_TREX_RUN_TURN_RIGHT90,1.5,	BF_RUNNING_MODE | BF_HEAD_TRACK_TUROK | BF_LASER_ON),
	BOSS_MODE_INFO(NULL,								TRex_Code_RUN_BITE,	AI_ANIM_BOSS_TREX_RUN_BITE,1.5,				BF_RUNNING_MODE | BF_HEAD_TRACK_TUROK),

	BOSS_MODE_INFO(NULL,								TRex_Code_Movement,	AI_ANIM_BOSS_TREX_LIMP,1.5,					BF_RUNNING_MODE | BF_HEAD_TRACK_TUROK | BF_LASER_ON),
	BOSS_MODE_INFO(NULL,								TRex_Code_Movement,	AI_ANIM_BOSS_TREX_LIMP_LEFT90,1.5,			BF_RUNNING_MODE | BF_HEAD_TRACK_TUROK | BF_LASER_ON),
	BOSS_MODE_INFO(NULL,								TRex_Code_Movement,	AI_ANIM_BOSS_TREX_LIMP_RIGHT90,1.5,			BF_RUNNING_MODE | BF_HEAD_TRACK_TUROK | BF_LASER_ON),

	// Attacks
	BOSS_MODE_INFO(NULL,								TRex_Code_Attack,		AI_ANIM_BOSS_TREX_TAIL_SLAM,1,0 | BF_HEAD_TRACK_TUROK),
	BOSS_MODE_INFO(NULL,								TRex_Code_Attack,		AI_ANIM_BOSS_TREX_BITE,1,0 | BF_HEAD_TRACK_TUROK),
	BOSS_MODE_INFO(NULL,								TRex_Code_Attack,		AI_ANIM_BOSS_TREX_RAGE,1,0 | BF_HEAD_TRACK_TUROK),

	// Looping attacks
	BOSS_MODE_INFO(TRex_Setup_BREATH_FIRE,		TRex_Code_FireLoop,	AI_ANIM_BOSS_TREX_BREATH_FIRE,1,BF_FLAME_MODE),
	BOSS_MODE_INFO(NULL,								TRex_Code_FireIn,		AI_ANIM_BOSS_TREX_BREATH_FIRE_IN,1,BF_FLAME_MODE),
	BOSS_MODE_INFO(NULL,								TRex_Code_FireOut,	AI_ANIM_BOSS_TREX_BREATH_FIRE_OUT,1,BF_FLAME_MODE),

	BOSS_MODE_INFO(TRex_Setup_EYE_FIRE,			TRex_Code_FireLoop,	AI_ANIM_BOSS_TREX_EYE_FIRE,1,0),
	BOSS_MODE_INFO(NULL,								TRex_Code_FireIn,		AI_ANIM_BOSS_TREX_EYE_FIRE_IN,1,0),
	BOSS_MODE_INFO(NULL,								TRex_Code_FireOut,	AI_ANIM_BOSS_TREX_EYE_FIRE_OUT,1,0),

	// Up attacks
	BOSS_MODE_INFO(NULL,								TRex_Code_LedgeAttack,		AI_ANIM_BOSS_TREX_BITE_UP,1,0 | BF_HEAD_TRACK_TUROK),
	BOSS_MODE_INFO(NULL,								TRex_Code_LedgeAttack,		AI_ANIM_BOSS_TREX_FIRE_UP,1,BF_FLAME_MODE),

	// Cave specials
	BOSS_MODE_INFO(NULL,								TRex_Code_CAVE_STOOP,AI_ANIM_BOSS_TREX_CAVE_STOOP,1,0 | BF_LASER_ON),
	BOSS_MODE_INFO(NULL,								TRex_Code_CaveAttack,AI_ANIM_BOSS_TREX_CAVE_LOOK,1,0),
	BOSS_MODE_INFO(NULL,								TRex_Code_CaveAttack,AI_ANIM_BOSS_TREX_CAVE_BITE,1,0),
	BOSS_MODE_INFO(TRex_Setup_CAVE_OUT,			TRex_Code_CAVE_OUT,	AI_ANIM_BOSS_TREX_CAVE_OUT,1,0),

	BOSS_MODE_INFO(NULL,								TRex_Code_CAVE_NOSE_HIT, 	AI_ANIM_BOSS_TREX_CAVE_NOSE_HIT,1,0),
	BOSS_MODE_INFO(NULL,								TRex_Code_CAVE_RAGE,			AI_ANIM_BOSS_TREX_RAGE,1,0 | BF_HEAD_TRACK_TUROK),
	BOSS_MODE_INFO(NULL,								TRex_Code_CAVE_TAIL_SLAM,	AI_ANIM_BOSS_TREX_TAIL_SLAM,1,0),

	// Explode modes
	BOSS_MODE_INFO(NULL,								TRex_Code_Explode,	AI_ANIM_BOSS_TREX_EXPLODE_LEFT,1,BF_EXPLODE_MODE),
	BOSS_MODE_INFO(NULL,								TRex_Code_Explode,	AI_ANIM_BOSS_TREX_EXPLODE_RIGHT,1,BF_EXPLODE_MODE),
	BOSS_MODE_INFO(NULL,								TRex_Code_Explode,	AI_ANIM_BOSS_TREX_EXPLODE_FRONT,1,BF_EXPLODE_MODE),
	BOSS_MODE_INFO(NULL,								TRex_Code_Explode,	AI_ANIM_BOSS_TREX_EXPLODE_BACK,1,BF_EXPLODE_MODE),

	// Hurt modes
	BOSS_MODE_INFO(NULL,								TRex_Code_STUNNED,	AI_ANIM_BOSS_TREX_STUNNED,1.5,0),
	BOSS_MODE_INFO(NULL,								TRex_Code_STUN_OUT,	AI_ANIM_BOSS_TREX_STUN_OUT,1,0),
	BOSS_MODE_INFO(NULL,								NULL,						AI_ANIM_BOSS_TREX_DEATH,1,BF_DEATH_MODE | BF_RUN_AT_MODE_ANIM_SPEED),

	BOSS_MODE_INFO(NULL,								TRex_Code_EAT_TUROK,	AI_ANIM_BOSS_TREX_EATTUROK,1, BF_RUN_AT_MODE_ANIM_SPEED),

	{NULL}
} ;

/////////////////////////////////////////////////////////////////////////////
//
// Normal attacks
//
/////////////////////////////////////////////////////////////////////////////
DEFINE_TREXSCRIPT(TRexAttackScript_Stage1)

	TREXSCRIPT_RAGE(1)
	TREXSCRIPT_END_ATTACK

	TREXSCRIPT_BREATH_FIRE(1)
	TREXSCRIPT_END_ATTACK

	TREXSCRIPT_EYE_FIRE(2)
	TREXSCRIPT_END_ATTACK

END_TREXSCRIPT

DEFINE_TREXSCRIPT(TRexAttackScript_Stage2)

	TREXSCRIPT_RAGE(2)
	TREXSCRIPT_END_ATTACK

	TREXSCRIPT_BREATH_FIRE(2)
	TREXSCRIPT_END_ATTACK

	TREXSCRIPT_EYE_FIRE(2)
	TREXSCRIPT_END_ATTACK

END_TREXSCRIPT


DEFINE_TREXSCRIPT(TRexAttackScript_Stage3)

	TREXSCRIPT_RAGE(1)
	TREXSCRIPT_END_ATTACK

	TREXSCRIPT_BREATH_FIRE(3)
	TREXSCRIPT_END_ATTACK

	TREXSCRIPT_EYE_FIRE(3)
	TREXSCRIPT_END_ATTACK

END_TREXSCRIPT




/////////////////////////////////////////////////////////////////////////////
//
// Close attacks
//
/////////////////////////////////////////////////////////////////////////////
DEFINE_TREXSCRIPT(TRexCloseAttackScript_Stage1)

	TREXSCRIPT_BITE(1)
	TREXSCRIPT_END_ATTACK

	TREXSCRIPT_TAIL_SLAM(1)
	TREXSCRIPT_END_ATTACK

END_TREXSCRIPT

DEFINE_TREXSCRIPT(TRexCloseAttackScript_Stage2)

	TREXSCRIPT_BITE(2)
	TREXSCRIPT_END_ATTACK

	TREXSCRIPT_TAIL_SLAM(2)
	TREXSCRIPT_END_ATTACK

END_TREXSCRIPT

DEFINE_TREXSCRIPT(TRexCloseAttackScript_Stage3)

	TREXSCRIPT_BITE(3)
	TREXSCRIPT_END_ATTACK

	TREXSCRIPT_TAIL_SLAM(3)
	TREXSCRIPT_END_ATTACK

END_TREXSCRIPT






/////////////////////////////////////////////////////////////////////////////
//
// Run attacks
//
/////////////////////////////////////////////////////////////////////////////
DEFINE_TREXSCRIPT(TRexRunAttackScript_Stage1)

	TREXSCRIPT_BREATH_FIRE(1)
	TREXSCRIPT_END_ATTACK

	TREXSCRIPT_EYE_FIRE(2)
	TREXSCRIPT_END_ATTACK

END_TREXSCRIPT


DEFINE_TREXSCRIPT(TRexRunAttackScript_Stage2)

	TREXSCRIPT_BREATH_FIRE(2)
	TREXSCRIPT_END_ATTACK

	TREXSCRIPT_EYE_FIRE(2)
	TREXSCRIPT_END_ATTACK

END_TREXSCRIPT



DEFINE_TREXSCRIPT(TRexRunAttackScript_Stage3)

	TREXSCRIPT_BREATH_FIRE(3)
	TREXSCRIPT_END_ATTACK

	TREXSCRIPT_EYE_FIRE(3)
	TREXSCRIPT_END_ATTACK

END_TREXSCRIPT




/////////////////////////////////////////////////////////////////////////////
//
// Turn attacks
//
/////////////////////////////////////////////////////////////////////////////
DEFINE_TREXSCRIPT(TRexTurnAttackScript_Stage1)

	TREXSCRIPT_TAIL_SLAM(1)
	TREXSCRIPT_END_ATTACK

END_TREXSCRIPT


DEFINE_TREXSCRIPT(TRexTurnAttackScript_Stage2)

	TREXSCRIPT_TAIL_SLAM(2)
	TREXSCRIPT_END_ATTACK

END_TREXSCRIPT


DEFINE_TREXSCRIPT(TRexTurnAttackScript_Stage3)

	TREXSCRIPT_TAIL_SLAM(3)
	TREXSCRIPT_END_ATTACK

END_TREXSCRIPT




/////////////////////////////////////////////////////////////////////////////
//
// Ledge attacks
//
/////////////////////////////////////////////////////////////////////////////
DEFINE_TREXSCRIPT(TRexLedgeAttackScript_Stage1)

	TREXSCRIPT_BITE_UP(1)
	TREXSCRIPT_END_ATTACK

	TREXSCRIPT_FIRE_UP(2)
	TREXSCRIPT_END_ATTACK

END_TREXSCRIPT

DEFINE_TREXSCRIPT(TRexLedgeAttackScript_Stage2)

	TREXSCRIPT_BITE_UP(3)
	TREXSCRIPT_END_ATTACK

	TREXSCRIPT_FIRE_UP(3)
	TREXSCRIPT_END_ATTACK

END_TREXSCRIPT

DEFINE_TREXSCRIPT(TRexLedgeAttackScript_Stage3)

	TREXSCRIPT_BITE_UP(3)
	TREXSCRIPT_END_ATTACK

	TREXSCRIPT_FIRE_UP(3)
	TREXSCRIPT_END_ATTACK

END_TREXSCRIPT



/////////////////////////////////////////////////////////////////////////////
//
// Cave attacks
//
/////////////////////////////////////////////////////////////////////////////
DEFINE_TREXSCRIPT(TRexCaveAttackScript_Stage1)

	TREXSCRIPT_CAVE_LOOK(1)
	TREXSCRIPT_BITE_LOOK(1)

END_TREXSCRIPT

DEFINE_TREXSCRIPT(TRexCaveAttackScript_Stage2)

	TREXSCRIPT_CAVE_LOOK(1)
	TREXSCRIPT_BITE_LOOK(2)

END_TREXSCRIPT

DEFINE_TREXSCRIPT(TRexCaveAttackScript_Stage3)

	TREXSCRIPT_CAVE_LOOK(1)
	TREXSCRIPT_BITE_LOOK(3)

END_TREXSCRIPT




/////////////////////////////////////////////////////////////////////////////
//
// The big stage table
//
/////////////////////////////////////////////////////////////////////////////
CTRexStageInfo TRexStageInfo[] =
{
	//	STAGE 1
	{75,										// Health limit for this stage
	 1.0, 									// Anim speed
	 1,										// Hits before explode
	 1,										// Cave Tail Slams

	 TRexAttackScript_Stage1,			// Attack script
	 TRexCloseAttackScript_Stage1, 	// Attack script
	 TRexRunAttackScript_Stage1,	 	// Run attack script
	 TRexTurnAttackScript_Stage1,	 	// Turn attack script
	 TRexLedgeAttackScript_Stage1,	//	Ledge attack script
	 TRexCaveAttackScript_Stage1,		// Cave attack script
	},

	//	STAGE 2
	{50,										// Health limit for this stage
	 1.1,										// Anim speed
	 2,										// Hits before explode
	 2,										// Cave Tail Slams

	 TRexAttackScript_Stage1,			// Attack script
	 TRexCloseAttackScript_Stage1, 	// Attack script
	 TRexRunAttackScript_Stage1,	 	// Run attack script
	 TRexTurnAttackScript_Stage1,	 	// Turn attack script
	 TRexLedgeAttackScript_Stage1,	//	Ledge attack script
	 TRexCaveAttackScript_Stage1,		// Cave attack script
	},

	//	STAGE 3
	{25,										// Health limit for this stage
	 1.2,										// Anim speed
	 4,										// Hits before explode
	 4,										// Cave Tail Slams

	 TRexAttackScript_Stage2,			// Attack script
	 TRexCloseAttackScript_Stage2, 	// Attack script
	 TRexRunAttackScript_Stage2,	 	// Run attack script
	 TRexTurnAttackScript_Stage2,	 	// Turn attack script
	 TRexLedgeAttackScript_Stage2,	//	Ledge attack script
	 TRexCaveAttackScript_Stage2,		// Cave attack script
	},

	//	STAGE 4
	{0,										// Health limit for this stage
	 1.3,										// Anim speed
	 6,										// Hits before explode
	 6,										// Cave Tail Slams

	 TRexAttackScript_Stage3,			// Attack script
	 TRexCloseAttackScript_Stage3, 	// Attack script
	 TRexRunAttackScript_Stage3,	 	// Run attack script
	 TRexTurnAttackScript_Stage3,	 	// Turn attack script
	 TRexLedgeAttackScript_Stage3,	//	Ledge attack script
	 TRexCaveAttackScript_Stage3,		// Cave attack script
	},
} ;



// Arena cave hole list
CCaveInfo CaveHoleList[] =
{
	// Left hole 1
	{{-200*SCALING_FACTOR, 0, 5*SCALING_FACTOR},	// x1,y1,z1
	 {-175*SCALING_FACTOR, 0, 26*SCALING_FACTOR},	// x2,y2,z2
	 {(-175*SCALING_FACTOR)+TREX_STOOP_DIST,0,((24+7)/2)*SCALING_FACTOR},	// TRex target before stoop
	 {((-192-175)/2)*SCALING_FACTOR,0,((24+7)/2)*SCALING_FACTOR}},			// TRex target look pos

	// Left hole 2
	{{-200*SCALING_FACTOR, 0, -25*SCALING_FACTOR},	// x1,y1,z1
	 {-175*SCALING_FACTOR, 0, -4*SCALING_FACTOR},	// x2,y2,z2
	 {(-175*SCALING_FACTOR)+TREX_STOOP_DIST,0,((-23-6)/2)*SCALING_FACTOR},	// TRex target before stoop
	 {((-192-175)/2)*SCALING_FACTOR,0,((-23-6)/2)*SCALING_FACTOR}},			// TRex target look pos

	// Left hole 3
	{{-200*SCALING_FACTOR, 0, -55*SCALING_FACTOR},	// x1,y1,z1
	 {-175*SCALING_FACTOR, 0, -34*SCALING_FACTOR},	// x2,y2,z2
	 {(-175*SCALING_FACTOR)+TREX_STOOP_DIST,0,((-53-36)/2)*SCALING_FACTOR},	// TRex target before stoop
	 {((-192-175)/2)*SCALING_FACTOR,0,((-53-36)/2)*SCALING_FACTOR}},			// TRex target look pos


	// Right hole 1
	{{175*SCALING_FACTOR, 0, 5*SCALING_FACTOR},	// x1,y1,z1
	 {200*SCALING_FACTOR, 0, 26*SCALING_FACTOR},	// x2,y2,z2
	 {(175*SCALING_FACTOR)-TREX_STOOP_DIST,0,((24+7)/2)*SCALING_FACTOR},	// TRex target before stoop
	 {((175+192)/2)*SCALING_FACTOR,0,((24+7)/2)*SCALING_FACTOR}},			// TRex target look pos

	// Right hole 2
	{{175*SCALING_FACTOR, 0, -25*SCALING_FACTOR},	// x1,y1,z1
	 {200*SCALING_FACTOR, 0, -4*SCALING_FACTOR},	// x2,y2,z2
	 {(175*SCALING_FACTOR)-TREX_STOOP_DIST,0,((-23-6)/2)*SCALING_FACTOR},	// TRex target before stoop
	 {((175+192)/2)*SCALING_FACTOR,0,((-23-6)/2)*SCALING_FACTOR}},			// TRex target look pos

	// Right hole 3
	{{175*SCALING_FACTOR, 0, -55*SCALING_FACTOR},	// x1,y1,z1
	 {200*SCALING_FACTOR, 0, -34*SCALING_FACTOR},	// x2,y2,z2
	 {(175*SCALING_FACTOR)-TREX_STOOP_DIST,0,((-53-36)/2)*SCALING_FACTOR},	// TRex target before stoop
	 {((175+192)/2)*SCALING_FACTOR,0,((-53-36)/2)*SCALING_FACTOR}},			// TRex target look pos

#if 0
	// Left tunnel
	{{-59*SCALING_FACTOR, 0, 118*SCALING_FACTOR},	// x1,y1,z1
	 {-15*SCALING_FACTOR, 0, 153*SCALING_FACTOR},	// x2,y2,z2
	 {((-57-47)/2)*SCALING_FACTOR,0,(118*SCALING_FACTOR)-TREX_STOOP_DIST},	// TRex target before stoop
	 {((-57-47)/2)*SCALING_FACTOR,0,125*SCALING_FACTOR}},			// TRex target look pos

	// Right tunnel
	{{15*SCALING_FACTOR, 0, 118*SCALING_FACTOR},	// x1,y1,z1
	 {59*SCALING_FACTOR, 0, 153*SCALING_FACTOR},	// x2,y2,z2
	 {((48+57)/2)*SCALING_FACTOR,0,(118*SCALING_FACTOR)-TREX_STOOP_DIST},	// TRex target before stoop
	 {((48+57)/2)*SCALING_FACTOR,0,125*SCALING_FACTOR}},			// TRex target look pos
#endif

} ;

// Damage flags
#define TREX_TAIL_TEX_DAMAGE_FLAG					(1<<0)
#define TREX_WRIST_TEX_DAMAGE_FLAG					(1<<1)
#define TREX_FOREARM_TEX_DAMAGE_FLAG				(1<<2)
#define TREX_UPPER_ARM_TEX_DAMAGE_FLAG	 			(1<<3)
#define TREX_CHEST_TEX_DAMAGE_FLAG					(1<<4)
#define TREX_THIGH_TEX_DAMAGE_FLAG					(1<<5)

enum TRexHierarchy
{
	TREX_HIPS_NODE,
	TREX_CHEST_NODE,
	TREX_NECK_DOWN_NODE,
	TREX_NECK_UP_NODE,
	TREX_HEAD_NODE,
	TREX_JAW_NODE,

	TREX_SHOULDER1_NODE,
	TREX_BICEP1_NODE,
	TREX_FOREARM1_NODE,
	TREX_WRIST1_NODE,
	TREX_NAIL1_NODE,
	TREX_NAIL1_TIP_NODE,
	TREX_NAIL2_NODE,
	TREX_NAIL2_TIP_NODE,
	TREX_NAIL3_NODE,
	TREX_NAIL3_TIP_NODE,

	TREX_SHOULDER2_NODE,
	TREX_BICEP2_NODE,
	TREX_FOREARM2_NODE,
	TREX_WRIST2_NODE,
	TREX_NAIL4_NODE,
	TREX_NAIL4_TIP_NODE,
	TREX_NAIL5_NODE,
	TREX_NAIL5_TIP_NODE,
	TREX_NAIL6_NODE,
	TREX_NAIL6_TIP_NODE,

	TREX_TAIL1_NODE,
	TREX_TAIL2_NODE,
	TREX_TAIL3_NODE,
	TREX_TAIL4_NODE,
	TREX_TAIL5_NODE,

	TREX_THIGH1_NODE,
	TREX_CALVE1_NODE,
	TREX_FOOT1_NODE,
	TREX_TOES1_NODE,
	TREX_CLAW1_A1_NODE,
	TREX_CLAW1_B1_NODE,
	TREX_CLAW1_C1_NODE,

	TREX_CLAW2_A1_NODE,
	TREX_CLAW2_B1_NODE,
	TREX_CLAW2_C1_NODE,

	TREX_CLAW3_A1_NODE,
	TREX_CLAW3_B1_NODE,
	TREX_CLAW3_C1_NODE,

	TREX_THIGH2_NODE,
	TREX_CALVE2_NODE,
	TREX_FOOT2_NODE,
	TREX_TOES2_NODE,
	TREX_CLAW1_A2_NODE,
	TREX_CLAW1_B2_NODE,
	TREX_CLAW1_C2_NODE,

	TREX_CLAW2_A2_NODE,
	TREX_CLAW2_B2_NODE,
	TREX_CLAW2_C2_NODE,

	TREX_CLAW3_A2_NODE,
	TREX_CLAW3_B2_NODE,
	TREX_CLAW3_C2_NODE
} ;

CDamage TRexDamageTable[] =
{
	// Stage 1 - No damage
	{80,												// Health before next stage
	 -1,-1,											// Damaged nodes
	 0},

	// Stage 2 - Damage tail
	{65,												// Health before next stage
    TREX_TAIL1_NODE, TREX_TAIL2_NODE,		// Damaged nodes
	 TREX_TAIL_TEX_DAMAGE_FLAG
	},

	// Stage 3 - Damage wrists
	{50,								  				// Health before next stage
    TREX_WRIST1_NODE, TREX_WRIST2_NODE, 	// Damaged nodes
	 TREX_TAIL_TEX_DAMAGE_FLAG |
	 TREX_WRIST_TEX_DAMAGE_FLAG
	},

	// Stage 4 - Damage forearms
	{35,												 	// Health before next stage
    TREX_FOREARM1_NODE, TREX_FOREARM2_NODE,	// Damaged nodes
	 TREX_TAIL_TEX_DAMAGE_FLAG |
	 TREX_WRIST_TEX_DAMAGE_FLAG |
    TREX_FOREARM_TEX_DAMAGE_FLAG
	},

	// Stage 5 - Upper arms
	{25,													// Health before next stage
    TREX_BICEP1_NODE, TREX_BICEP2_NODE,  		// Damaged nodes
	 TREX_TAIL_TEX_DAMAGE_FLAG |
	 TREX_WRIST_TEX_DAMAGE_FLAG |
    TREX_FOREARM_TEX_DAMAGE_FLAG |
	 TREX_UPPER_ARM_TEX_DAMAGE_FLAG
	},

	// Stage 6 - Damage chest
	{15,												// Health before next stage
    TREX_CHEST_NODE, 0,							// Damaged nodes
	 TREX_TAIL_TEX_DAMAGE_FLAG |
	 TREX_WRIST_TEX_DAMAGE_FLAG |
    TREX_FOREARM_TEX_DAMAGE_FLAG |
	 TREX_UPPER_ARM_TEX_DAMAGE_FLAG |
	 TREX_CHEST_TEX_DAMAGE_FLAG
	},

	// Stage 7 - Damage thighs
	{0,													// Health before next stage
    TREX_THIGH1_NODE, TREX_THIGH2_NODE,		// Damaged nodes
	 TREX_TAIL_TEX_DAMAGE_FLAG |
	 TREX_WRIST_TEX_DAMAGE_FLAG |
    TREX_FOREARM_TEX_DAMAGE_FLAG |
	 TREX_UPPER_ARM_TEX_DAMAGE_FLAG |
	 TREX_CHEST_TEX_DAMAGE_FLAG |
	 TREX_THIGH_TEX_DAMAGE_FLAG
	},

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
*	Function:		CCollisionInfo__SetTRexCollisionDefaults()
*
******************************************************************************
*
*	Description:	Sets up default collision
*
*	Inputs:			*pThis	-	Ptr to collision structure
*
*	Outputs:			None
*
*****************************************************************************/
void CCollisionInfo__SetTRexCollisionDefaults(CCollisionInfo2 *pThis)
{
	pThis->dwFlags =		COLLISIONFLAG_PHYSICS
							|	COLLISIONFLAG_EXITWATER
							|	COLLISIONFLAG_USEWALLRADIUS
							|	COLLISIONFLAG_TRACKGROUND
							|	COLLISIONFLAG_CANENTERRESTRICTEDAREA ;
//							|  COLLISIONFLAG_CLIMBUP 
//							|  COLLISIONFLAG_CLIMBDOWN ;

	pThis->InstanceBehavior			= INTERSECT_BEHAVIOR_SLIDE ;
	pThis->WallBehavior				= INTERSECT_BEHAVIOR_SLIDE ;
	pThis->GroundBehavior			= INTERSECT_BEHAVIOR_SLIDE ;
	pThis->GravityAcceleration		= TREX_GRAVITY ;
	pThis->BounceReturnEnergy		= 0.0;
	pThis->GroundFriction			= 0.0;
	pThis->AirFriction				= 0.0;
	pThis->WaterFriction				= 0.0;
}




/*****************************************************************************
*
*	Function:		TRex_SetupStageInfo()
*
******************************************************************************
*
*	Description:	Sets up stage info
*
*	Inputs:			*pThis	-	Ptr to trex game object instance
*						*pTRex	-	Ptr to trex boss vars
*						Info		-	Ptr to stage info
*
*	Outputs:			None
*
*****************************************************************************/
void TRex_SetupStageInfo(CGameObjectInstance *pThis, CTRex *pTRex, CTRexStageInfo *Info)
{
	pTRex->m_AttackScript.m_pStart =
	pTRex->m_AttackScript.m_pCurrent = Info->pAttackScript ;

	pTRex->m_CloseAttackScript.m_pStart =
	pTRex->m_CloseAttackScript.m_pCurrent = Info->pCloseAttackScript ;

	pTRex->m_RunAttackScript.m_pStart =
	pTRex->m_RunAttackScript.m_pCurrent = Info->pRunAttackScript ;

	pTRex->m_TurnAttackScript.m_pStart =
	pTRex->m_TurnAttackScript.m_pCurrent = Info->pTurnAttackScript ;

	pTRex->m_LedgeAttackScript.m_pStart =
	pTRex->m_LedgeAttackScript.m_pCurrent = Info->pLedgeAttackScript ;

	pTRex->m_CaveAttackScript.m_pStart =
	pTRex->m_CaveAttackScript.m_pCurrent = Info->pCaveAttackScript ;
}



/*****************************************************************************
*
*	Function:		TRex_Initialise()
*
******************************************************************************
*
*	Description:	Initialises trex game object instance
*
*	Inputs:			*pThis	-	Ptr to trex game object instance
*
*	Outputs:			CBoss*	-	Ptr to trex boss vars
*
*****************************************************************************/
CBoss *TRex_Initialise(CGameObjectInstance *pThis)
{
	CTRex *pTRex = &TRexBoss ;

	// Setup defaults
	CBoss__Construct(&pTRex->m_Boss) ;

	// Setup boss vars
	pTRex->m_Boss.m_pUpdateFunction = (void *)TRex_Update ;
	pTRex->m_Boss.m_pModeTable = TRexModeTable ;

	// Setup trex vars
	pTRex->m_pStageInfo = TRexStageInfo ;
	TRex_SetupStageInfo(pThis, pTRex, TRexStageInfo) ;

	// Put into start mode
	pTRex->m_Boss.m_Action = TREX_ON_ELEVATOR_ACTION ;
	pTRex->m_Boss.m_ActionTime = 0 ;
	pTRex->m_Boss.m_Mode = TREX_ON_ELEVATOR_IDLE_MODE ;
	pTRex->m_Boss.m_ModeAnimSpeed = 0 ;
	pTRex->m_Boss.m_AnimRepeats = 1 ;
	pTRex->m_Boss.m_AnimSpeed = 1.0 ;

	pTRex->m_Boss.m_pPreDrawFunction = (void *)TRex_PreDraw ;
	pTRex->m_Boss.m_pPostDrawFunction = (void *)TRex_PostDraw ;
	pTRex->m_Boss.m_pPauseFunction = (void *)TRex_UpdateSound ;


#ifndef MAKE_CART
	pTRex->m_Boss.m_pDisplayModeFunction = (void *)TRex_DisplayMode ;
#endif

#if DEBUG_TREX
	rmonPrintf("\r\n\r\n\r\nTREX HERE WE GO!! Prepare for Attack Action") ;
#endif

	// Setup misc
	CCollisionInfo__SetTRexCollisionDefaults(&pTRex->m_Boss.m_CollisionInfo) ;
	BOSS_Health(pThis) = TREX_HEALTH ;
	pTRex->m_ExplosionHits = 0 ;

	// Reset caves
	pTRex->m_LookTime = 0 ;
	pTRex->m_pTurokInCave = NULL ;
	pTRex->m_pTargetCave = NULL ;
	pTRex->m_TurokTimeInCave = 0 ;
	pTRex->m_TurokInCaveWaitTime = RandomRangeFLOAT(SECONDS_TO_FRAMES(2), SECONDS_TO_FRAMES(5)) ;

	// No sound to begin with
	pTRex->m_LaserSoundHandle = -1 ;

	pTRex->m_FireSoundHandle = -1 ;
	pTRex->m_FireSoundVolume = 0 ;

	pTRex->m_pDamage = TRexDamageTable ;

	// Return pointer to trex boss
	return (CBoss *)pTRex ;
}


/*****************************************************************************
*
*	Function:		TRex_PreDraw()
*
******************************************************************************
*
*	Description:	Turns laser on and off
*
*	Inputs:			*pThis			-	Ptr to game object instance
*						*pTRex	-	Ptr to boss vars
*
*	Outputs:			None
*
*****************************************************************************/
void TRex_PreDraw(CGameObjectInstance *pThis, CTRex *pTRex, Gfx **ppDLP)
{
	if (!(pTRex->m_Boss.m_ModeFlags & BF_LASER_ON))
	{
		draw_transparent_geometry = FALSE ;
		draw_intersect_geometry = FALSE ;
	}
}


/*****************************************************************************
*
*	Function:		TRex_PostDraw()
*
******************************************************************************
*
*	Description:	Resets draw mode so special fx don't affect next objects
*
*	Inputs:			*pThis	-	Ptr to game object instance
*						*pTRex	-	Ptr to boss vars
*
*	Outputs:			None
*
*****************************************************************************/
void TRex_PostDraw(CGameObjectInstance *pThis, CTRex *pTRex, Gfx **ppDLP)
{
	draw_transparent_geometry = TRUE ;
	draw_intersect_geometry = TRUE ;
}


#define L_X				(0.01)
#define L_Y				(-0.2)
#define L_Z				(0.65)

#define L_DELTA_X		-100		// (+ve = back, -ve = front)
#define L_DELTA_Y		0			// (+ve = to right, -ve = to left)
#define L_DELTA_Z		-78		// (+ve = up, -ve = down)

#define L_S				0
#define L_E				1


CSwooshEdgeVertexInfo TRexLaserSwooshVertices[] =
{
	// Pos													Color					 Alpha
	{{{L_X + (L_S * L_DELTA_X), L_Y + (L_S * L_DELTA_Y), L_Z + (L_S * L_DELTA_Z)},
	  {255, 0, 0}, MAX_OPAQ/16},

    {{L_X + (L_S * L_DELTA_X), L_Y + (L_S * L_DELTA_Y), L_Z + (L_S * L_DELTA_Z)},
	  {255, 0, 0}, MIN_OPAQ}},

	 {{{L_X + (L_E * L_DELTA_X), L_Y + (L_E * L_DELTA_Y), L_Z + (L_E * L_DELTA_Z)},
	  {255, 150, 150}, MAX_OPAQ/8},

    {{L_X + (L_E * L_DELTA_X), L_Y + (L_E * L_DELTA_Y), L_Z + (L_E * L_DELTA_Z)},
	  {255, 0, 0}, MIN_OPAQ}}
} ;

//	{{{150.1*0.01,	-0.2,		(124*0.01) + 0.6},	{255,	0,	0},		MAX_OPAQ/16},	// Start
//	 {{150.1*0.01,	-0.2,		(124*0.01) + 0.6},	{255,	0, 0},		MIN_OPAQ}},		// end

	// BEFORE
//	{{{150.1,		-0.2,		124 + 0.6},			{255,	150, 150},	MAX_OPAQ/8},	// Start
//	 {{150.1,		-0.2,		124 + 0.6},			{255,	0,	0},		MIN_OPAQ}},		// end


// Front
//	{{{150,		0,	0},		{255,	0, 0},	MAX_OPAQ},	// Start
//	 {{150,		0,	0},		{255,	0,	0},	MIN_OPAQ}},	// end

// Left
//	{{{0,		-150,	0},		{255,	0, 0},	MAX_OPAQ},	// Start
//	 {{0,		-150,	0},		{255,	0,	0},	MIN_OPAQ}},	// end

// UP
//	{{{0,		0,	150},		{255,	0, 0},	MAX_OPAQ},	// Start
//	 {{0,		0,	150},		{255,	0,	0},	MIN_OPAQ}},	// end
//} ;

CSwooshInfo	TRexLaserSwooshInfo =
{
	SECONDS_TO_FRAMES(10000), 	// Total time on screen
	SECONDS_TO_FRAMES(10000), 	// Total time before fade swoosh out
	SECONDS_TO_FRAMES(0.2),		// Total edge life time
	4,								 	// Object node
	SCALING_FACTOR*10,			// Scale
	NULL,					 			// General flags
	2,						 			// No of edge vertices
	TRexLaserSwooshVertices 			// Edge vertices
} ;

void TRex_StartFireSound(CGameObjectInstance *pThis, CTRex *pTRex)
{
	// Sound not playing and in a flame mode?
	if (pTRex->m_FireSoundHandle == -1)
		pTRex->m_FireSoundHandle = AI_DoSound(&pThis->ah.ih, SOUND_TREX_FIRE_BREATHE, 1, 0) ;

	// Put volume back to the max
	pTRex->m_FireSoundVolume = 24000 ;
	TRex_UpdateFireSound(pThis, pTRex) ;
}


void TRex_StopFireSound(CGameObjectInstance *pThis, CTRex *pTRex)
{
	// Sound playing?
	if (pTRex->m_FireSoundHandle != -1)
	{
 		killCFX(pTRex->m_FireSoundHandle) ;
 		pTRex->m_FireSoundHandle = -1 ;
	}
}

void TRex_UpdateFireSound(CGameObjectInstance *pThis, CTRex *pTRex)
{
	// Update the sound volume
	if (pTRex->m_FireSoundHandle != -1)
	{
		// Set position
		UpdateSoundVector(pTRex->m_FireSoundHandle, &pThis->ah.ih.m_vPos) ;

		// CRASHES IT!!!!
		// Set new volume
//		UpdateSoundVolume(pTRex->m_FireSoundHandle,
//								((float)pTRex->m_FireSoundVolume * __GlobalSFXscalar)) ;

		// Volume off?
		if (pTRex->m_FireSoundVolume == 0)
			TRex_StopFireSound(pThis, pTRex) ;
		else
		{
			// Fade out volume (fast during pause)
			if (GetApp()->m_bPause)
				pTRex->m_FireSoundVolume -= 8000 ;
			else
				pTRex->m_FireSoundVolume -= 2000 ;

			if (pTRex->m_FireSoundVolume < 0)
				pTRex->m_FireSoundVolume = 0 ;
		}
	}
}


void TRex_UpdateLaserSound(CGameObjectInstance *pThis, CTRex *pTRex)
{
	CMtxF		mfMtx ;
	CVector3	vLaserStart, vLaserStartLocal ;
	CVector3	vLaserEnd, vLaserEndLocal ;
	CVector3	vI, vPos ;
	CLine		LaserLine, TurokLine, NormalLine ;
	CGameObjectInstance *pTurok ;
	FLOAT		t ;

	// Turn laser off?
	if ((!(pTRex->m_Boss.m_ModeFlags & BF_LASER_ON)) ||
		 (GetApp()->m_bPause) ||
		 (CCamera__InCinemaMode(&GetApp()->m_Camera)))
	{
		// Kill sound
		if (pTRex->m_LaserSoundHandle != -1)
		{
			killCFX(pTRex->m_LaserSoundHandle) ;
			pTRex->m_LaserSoundHandle = -1 ;
		}
		return ;
	}
	else
	if (!pThis->m_ActiveSwooshes)
		CFxSystem__AddObjectSwoosh(&GetApp()->m_FxSystem, pThis, &TRexLaserSwooshInfo) ;


	// Must have a node and a matrix
	if (!pThis->pmtDrawMtxs)
		return ;

	// Setup local positions for laser start and end
	CVector3__Set(&vLaserStartLocal,	  0*SCALING_FACTOR*10, 0*SCALING_FACTOR*10, 0.5*SCALING_FACTOR*10) ;
	CVector3__Set(&vLaserEndLocal,	  150*SCALING_FACTOR*10, 0*SCALING_FACTOR*10, 124.5*SCALING_FACTOR*10) ;

	// Transform local positions into world positions using node matrix
	CMtxF__FromMtx(mfMtx, pThis->pmtDrawMtxs[4]) ;
	CMtxF__VectorMult(mfMtx, &vLaserStartLocal, &vLaserStart) ;
	CMtxF__VectorMult(mfMtx, &vLaserEndLocal, &vLaserEnd) ;

	// Calculate laser line
	CLine__Construct(&LaserLine, &vLaserStart, &vLaserEnd) ;

	// Calculate normal line of laser start to turok
	pTurok = CEngineApp__GetPlayer(GetApp()) ;
	CLine__Construct(&TurokLine, &vLaserStart, &pTurok->ah.ih.m_vPos) ;
	CVector3__Add(&vPos, &TurokLine.m_vNormal, &pTurok->ah.ih.m_vPos) ;
	CLine__Construct(&NormalLine, &pTurok->ah.ih.m_vPos, &vPos) ;

	// Calculate intersection pt on laser line
	if ((CLine__IntersectsLine(&NormalLine, &LaserLine, &t)) && (t >= 0))
	{
		if (t > 1)
			t = 1 ;

		vI.x = LaserLine.m_vPt[0].x + (t * LaserLine.m_vDelta.x) ;
		vI.y = LaserLine.m_vPt[0].y + (t * LaserLine.m_vDelta.y) ;
		vI.z = LaserLine.m_vPt[0].z + (t * LaserLine.m_vDelta.z) ;

		if (pTRex->m_LaserSoundHandle == -1)
			pTRex->m_vLaserLastSoundPos = vI ;

		t = (5*SCALING_FACTOR) - CVector3__Dist(&vI, &pTRex->m_vLaserLastSoundPos) ;
		if (t < 0)
			t = 0 ;
		else
		if (t > (5*SCALING_FACTOR))
			t = 5 * SCALING_FACTOR ;

		pTRex->m_vLaserLastSoundPos = vI ;

		vI.x += (vI.x - pTurok->ah.ih.m_vPos.x) * t ;
		vI.y += (vI.y - pTurok->ah.ih.m_vPos.y) * t ;
		vI.z += (vI.z - pTurok->ah.ih.m_vPos.z) * t ;

#if 0
		osSyncPrintf("Beam:(x,y,z): %f,%f,%f Turok(x,y,z): %f,%f,%f\r\n",
						 vI.x, vI.y, vI.z,
						 pTurok->ah.ih.m_vPos.x,
						 pTurok->ah.ih.m_vPos.y,
						 pTurok->ah.ih.m_vPos.z) ;
#endif

		// Start laser?
		if (pTRex->m_LaserSoundHandle == -1)
		{
			vPos = pThis->ah.ih.m_vPos ;
			pThis->ah.ih.m_vPos = vI ;
			pTRex->m_LaserSoundHandle = AI_DoSound(&pThis->ah.ih, SOUND_TREX_SERVO_1, 1, 0) ;
			pThis->ah.ih.m_vPos = vPos ;

		}

		// Update laser sound position
		if (pTRex->m_LaserSoundHandle != -1)
		{
			UpdateSoundVector(pTRex->m_LaserSoundHandle, &vI) ;
			t = sqrt(AI_DistanceSquared(pTurok, vI)) ;

#if 0
			osSyncPrintf("Dist from Turok:%f\r\n", t / SCALING_FACTOR) ;
#endif

			t = 1.0 - (t / (SCALING_FACTOR * 300)) ;
			t *= t*t ;
//			osSyncPrintf("t:%f\r\n", t) ;

			if (t < 0.2)
				t = 0.2 ;
			else
			if (t > 0.4)
				t = 0.4 ;

			UpdateSoundPitch(pTRex->m_LaserSoundHandle, t) ;
		}
	}
	else
	// Kill sound
	if (pTRex->m_LaserSoundHandle != -1)
	{
		killCFX(pTRex->m_LaserSoundHandle) ;
		pTRex->m_LaserSoundHandle = -1 ;
	}
}


void TRex_UpdateSound(CGameObjectInstance *pThis, CTRex *pTRex)
{
	TRex_UpdateLaserSound(pThis, pTRex) ;
	TRex_UpdateFireSound(pThis, pTRex) ;
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



/*****************************************************************************
*
*	Function:		TRex_Update()
*
******************************************************************************
*
*	Description:	Top level AI function - controls trex's actions
*
*	Inputs:			*pThis	-	Ptr to trex game object instance
*						*pTRex	-	Ptr to trex boss vars
*
*	Outputs:			None
*
*****************************************************************************/
void TRex_Update(CGameObjectInstance *pThis, CTRex *pTRex)
{
	CGameObjectInstance *pTurok ;

#if DEBUG_TREX
	INT32	OldAction ;
#endif

 
//	pTRex->m_Boss.m_ModeFlags |= BF_LASER_ON ;

	// Update looping sounds
	TRex_UpdateSound(pThis, pTRex) ;

	// Turn laser swoosh off
	if (!(pTRex->m_Boss.m_ModeFlags & BF_LASER_ON))
		CFxSystem__StopSwooshesOnObject(&GetApp()->m_FxSystem, pThis) ;

	// Leave cinema's alone
	if (CCamera__InCinemaMode(&GetApp()->m_Camera))
		return ;

	// Hit by explosion?
	if (BOSS_Health(pThis) > 0)
	{
		TRex_CheckHitByExplosion(pThis, pTRex) ;

		// Increase damage
		if (		(!(pTRex->m_Boss.m_ModeFlags & (BF_ENTER_MODE | BF_INVINCIBLE)))
			 &&	(pTRex->m_Boss.m_PercentageHealth < pTRex->m_pDamage->m_Health)  )
		{
			pTRex->m_pDamage++ ;
			TRex_SetupMode((CGameObjectInstance *)pThis, TREX_STUNNED_MODE) ;
		}
	}

	// No interruptions on blends!
	if (CGameObjectInstance__IsBlending(pThis))
		return ;

	// Check for death
	if (BOSS_Health(pThis) <= 0)
	{
		// tell the engine that the main boss for this level is dead
		//BOSS_LevelComplete() ;

		switch(pTRex->m_Boss.m_Mode)
		{
			// On floor modes - goto death
			case TREX_DEATH_MODE:
				break ;

			default:
				GetApp()->m_BossFlags |= BOSS_FLAG_TREX_DEAD	;
				TRex_SetupMode((CGameObjectInstance *)pThis, TREX_DEATH_MODE) ;

				// Kill laser sound
				if (pTRex->m_LaserSoundHandle != -1)
				{
					killCFX(pTRex->m_LaserSoundHandle) ;
					pTRex->m_LaserSoundHandle = -1 ;
				}

				CCamera__SetObjectToTrack(&GetApp()->m_Camera, pThis) ;
				CPickupMonitor__Deactivate(&GetApp()->m_Scene.m_PickupMonitor) ;
				CCamera__FadeToCinema(&GetApp()->m_Camera, CINEMA_FLAG_TUROK_KILL_TREX) ;
				break ;
		}
	}

	// Get player
	pTurok = CEngineApp__GetPlayer(GetApp());

	// Check for turok being in a cave
	TRex_CheckForTurokInCave(pThis, pTRex) ;

	// Decide which action to do
#if DEBUG_TREX
	OldAction = pTRex->m_Boss.m_Action ;
#endif


	if (pTRex->m_Boss.m_Action == TREX_ON_ELEVATOR_ACTION)
	{
		pTRex->m_Boss.m_ActionTime += frame_increment ;
		if (pTRex->m_Boss.m_ActionTime >= SECONDS_TO_FRAMES(3.5))
		{
// ELEVATOR NOW STAYS UP - SIMPLY MAKE TREX ATTACK!!
#if 0
			// Make trex walk off elevator
			pTRex->m_Boss.m_Action = TREX_EXIT_ELEVATOR_ACTION ;
			pTRex->m_Boss.m_ActionTime = 0 ;
			pTRex->m_Boss.m_Mode = TREX_ON_ELEVATOR_LOOK_MODE ;
#else
			// Make trex attack
			pTRex->m_Boss.m_Action = TREX_ATTACK_ACTION ;
			pTRex->m_Boss.m_ActionTime = 0 ;
			TRex_SetupMode((CGameObjectInstance *)pThis, TREX_TAIL_SLAM_MODE) ;
			pTRex->m_Boss.m_AnimRepeats = 1 ;

			return ;
#endif

		}
	}
	else
	if (pTRex->m_Boss.m_Action == TREX_EXIT_ELEVATOR_ACTION)
	{
		if (pTRex->m_Boss.m_Mode == TREX_EXIT_ELEVATOR_MODE)
		{
			pTRex->m_Boss.m_ActionTime += frame_increment ;
			if (pTRex->m_Boss.m_ActionTime >= SECONDS_TO_FRAMES(2))
			{
				// Make trex attack
				pTRex->m_Boss.m_Action = TREX_ATTACK_ACTION ;
				pTRex->m_Boss.m_ActionTime = 0 ;
				TRex_SetupMode((CGameObjectInstance *)pThis, TREX_TAIL_SLAM_MODE) ;
				pTRex->m_Boss.m_AnimRepeats = 1 ;

// PLATFORMS HAVE GONE!!!!!
#if 0
				// Make TRex avoid center hole in arena
				pTRex->m_Boss.m_CollisionInfo.dwFlags &= ~COLLISIONFLAG_CANENTERRESTRICTEDAREA ;
#endif

				// This return stop update function from stopping tail slam anim
				return ;
			}
		}
	}
	else
	if ((pTRex->m_pTurokInCave) && (pTRex->m_TurokTimeInCave >= pTRex->m_TurokInCaveWaitTime))
	{
		// Make trex look around if just gone in cave
		if (pTRex->m_Boss.m_Action != TREX_CAVE_ATTACK_ACTION)
			TRex_SetupMode((CGameObjectInstance *)pThis, TREX_LOOK_MODE) ;

		if (pTRex->m_Boss.m_Action != TREX_CAVE_ATTACK_ACTION)
		{
			pTRex->m_Boss.m_Action = TREX_CAVE_ATTACK_ACTION ;
		}
	}
	else
	if ((pTurok->ah.ih.m_vPos.y - BOSS_GetPos(pThis).y) > (SCALING_FACTOR * 6))
		pTRex->m_Boss.m_Action = TREX_LEDGE_ATTACK_ACTION ;
	else
		pTRex->m_Boss.m_Action = TREX_ATTACK_ACTION ;


#if DEBUG_TREX
	if (OldAction != pTRex->m_Boss.m_Action)
	{
		switch(pTRex->m_Boss.m_Action)
		{
			case TREX_ON_ELEVATOR_ACTION:
				rmonPrintf("\r\n...ON ELEVATOR ACTION...\r\n") ;
				break ;

			case TREX_EXIT_ELEVATOR_ACTION:
				rmonPrintf("\r\n...EXIT ELEVATOR ACTION...\r\n") ;
				break ;

			case TREX_CAVE_ATTACK_ACTION:
				rmonPrintf("\r\n...CAVE ATTACK ACTION...\r\n") ;
				break ;

			case TREX_LEDGE_ATTACK_ACTION:
				rmonPrintf("\r\n...LEDGE ATTACK ACTION...\r\n") ;
				break ;

			case TREX_ATTACK_ACTION:
				rmonPrintf("\r\n...ATTACK ACTION...\r\n") ;
				break ;
		}
	}
#endif

	// Get target and distance from it
	TRex_GetTarget(pThis, pTRex) ;

	// Call update function if there is one
	if ((pTRex->m_Boss.m_Mode >= 0 ) &&
		 (pTRex->m_Boss.m_pModeTable[pTRex->m_Boss.m_Mode].m_pUpdateFunction))
		 (pTRex->m_Boss.m_pModeTable[pTRex->m_Boss.m_Mode].m_pUpdateFunction)(pThis, (CBoss *)pTRex) ;

	// Check for updating stage
	if (pTRex->m_Boss.m_PercentageHealth < pTRex->m_pStageInfo->HealthLimit)
	{
		TRex_SetupStageInfo(pThis, pTRex, ++pTRex->m_pStageInfo) ;
#if DEBUG_TREX
			rmonPrintf("\r\n....NEXT ATTACK STAGE....\r\n") ;
#endif
	}


	// Update anim speed
	switch(pTRex->m_Boss.m_Mode)
	{
		default:
			pTRex->m_Boss.m_AnimSpeed = pTRex->m_pStageInfo->AnimSpeed ;
	}
}




/*****************************************************************************
*
*	Function:		TRex_SetupMode()
*
******************************************************************************
*
*	Description:	Sets up a new trex mode
*
*	Inputs:			*pThis	-	Ptr to trex game object instance
*						nNewMode	-	Id of new mode
*
*	Outputs:			TRUE if new mode was activated, else FALSe
*
*****************************************************************************/
INT32 TRex_SetupMode(CGameObjectInstance *pThis, INT32 nNewMode)
{
	BOOL	Result ;

	// Call normal boss setup
	Result = BOSS_SetupMode(pThis, nNewMode) ;

	return Result ;
}





/*****************************************************************************
*
*	Function:		TRex_GetTarget()
*
******************************************************************************
*
*	Description:	Gets target position and look direction for trex
*
*	Inputs:			*pThis	-	Ptr to trex game object instance
*						*pTRex	-	Ptr to trex boss vars
*
*	Outputs:			None
*
*****************************************************************************/

BOOL TRex_TurokInLeftTunnel(CGameObjectInstance *pThis, CTRex *pTRex)
{
	CVector3 *pvPos = &(CEngineApp__GetPlayer(GetApp())->ah.ih.m_vPos) ;
	return ((pvPos->z > (116*SCALING_FACTOR)) && (pvPos->x <= (-26*SCALING_FACTOR))) ;
}

BOOL TRex_TurokInRightTunnel(CGameObjectInstance *pThis, CTRex *pTRex)
{
	CVector3 *pvPos = &(CEngineApp__GetPlayer(GetApp())->ah.ih.m_vPos) ;
	return ((pvPos->z > (116*SCALING_FACTOR)) && (pvPos->x >= (26*SCALING_FACTOR))) ;
}

BOOL TRex_TurokInMiddleTunnel(CGameObjectInstance *pThis, CTRex *pTRex)
{
	CVector3 *pvPos = &(CEngineApp__GetPlayer(GetApp())->ah.ih.m_vPos) ;
	return ((pvPos->z > (116*SCALING_FACTOR)) &&
			  (pvPos->x > (-26*SCALING_FACTOR)) && (pvPos->x < (26*SCALING_FACTOR))) ;
}


void TRex_GetTarget(CGameObjectInstance *pThis, CTRex *pTRex)
{
	CGameObjectInstance *pTurok = CEngineApp__GetPlayer(GetApp());
	FLOAT	rot ;
	CVector3 vLookPos ;
	BOOL	TurokInTunnel ;

	TurokInTunnel = FALSE ;

	// Setup target distances
	switch(pTRex->m_Boss.m_Action)
	{
		case TREX_CAVE_ATTACK_ACTION:
			pTRex->m_TargetDist = TREX_CAVE_TARGET_DIST ;
			pTRex->m_AtTargetDist = TREX_CAVE_AT_TARGET_DIST ;
			break ;

		case TREX_LEDGE_ATTACK_ACTION:
			pTRex->m_TargetDist = TREX_LEDGE_TARGET_DIST ;
			pTRex->m_AtTargetDist = TREX_LEDGE_AT_TARGET_DIST ;
			break ;

		case TREX_ATTACK_ACTION:
			pTRex->m_TargetDist = TREX_TARGET_DIST ;
			pTRex->m_AtTargetDist = TREX_AT_TARGET_DIST ;
			break ;

		default:
			pTRex->m_TargetDist = 0 ;
			pTRex->m_AtTargetDist = 0 ;
			break ;

	}

	// Calculate target position
	switch(pTRex->m_Boss.m_Action)
	{
		case TREX_CAVE_ATTACK_ACTION:
			pTRex->m_Boss.m_vTarget.x = pTRex->m_pTurokInCave->m_vTargetPos.x ;
			pTRex->m_Boss.m_vTarget.y = pTRex->m_pTurokInCave->m_vTargetPos.y ;
			pTRex->m_Boss.m_vTarget.z = pTRex->m_pTurokInCave->m_vTargetPos.z ;
			break ;

		case TREX_LEDGE_ATTACK_ACTION:
			// Get rotation from centre of arena
			rot = CVector3__XZAngle(&TUROK_GetPos(pTurok)) + ANGLE_DTOR(180) ;
			pTRex->m_Boss.m_vTarget.x = pTurok->ah.ih.m_vPos.x - (pTRex->m_TargetDist * sin(rot)) ;
			pTRex->m_Boss.m_vTarget.y = BOSS_GetPos(pThis).y ;
			pTRex->m_Boss.m_vTarget.z = pTurok->ah.ih.m_vPos.z - (pTRex->m_TargetDist * cos(rot)) ;
			break ;

		case TREX_ATTACK_ACTION:

			// Special case when Turok is in tunnels
			if (TRex_TurokInLeftTunnel(pThis, pTRex))
			{
				TurokInTunnel = TRUE ;
				pTRex->m_Boss.m_vTarget.x = -52*SCALING_FACTOR ;
				pTRex->m_Boss.m_vTarget.y = 90*SCALING_FACTOR ;
				pTRex->m_Boss.m_vTarget.z = 97*SCALING_FACTOR ;
			}
			else
			if (TRex_TurokInRightTunnel(pThis, pTRex))
			{
				TurokInTunnel = TRUE ;
				pTRex->m_Boss.m_vTarget.x = 52*SCALING_FACTOR ;
				pTRex->m_Boss.m_vTarget.y = 90*SCALING_FACTOR ;
				pTRex->m_Boss.m_vTarget.z = 97*SCALING_FACTOR ;
			}
			else
			if (TRex_TurokInMiddleTunnel(pThis, pTRex))
			{
				TurokInTunnel = TRUE ;
				pTRex->m_Boss.m_vTarget.x = 0*SCALING_FACTOR ;
				pTRex->m_Boss.m_vTarget.y = 90*SCALING_FACTOR ;
				pTRex->m_Boss.m_vTarget.z = 97*SCALING_FACTOR ;
			}
			else
			{
				rot = pTurok->m_RotY ;
				pTRex->m_Boss.m_vTarget.x = pTurok->ah.ih.m_vPos.x - (pTRex->m_TargetDist * sin(rot)) ;
				pTRex->m_Boss.m_vTarget.y = BOSS_GetPos(pThis).y ;
				pTRex->m_Boss.m_vTarget.z = pTurok->ah.ih.m_vPos.z - (pTRex->m_TargetDist * cos(rot)) ;
			}
			break ;

		default:
			pTRex->m_Boss.m_vTarget = pTurok->ah.ih.m_vPos ;
			break ;
	}

	// Be more accurate if Turok in in a tunnel
	if (TurokInTunnel)
	{
		pTRex->m_TargetDist *= 0.5 ;
		pTRex->m_AtTargetDist *= 0.5 ;
	}

	// Setup distance stuff
	pTRex->m_Boss.m_DistFromTargetSqr = AI_XZDistanceSquared(pThis, pTRex->m_Boss.m_vTarget) ;
	if (pTRex->m_Boss.m_DistFromTargetSqr)
		pTRex->m_Boss.m_DistFromTarget = sqrt(pTRex->m_Boss.m_DistFromTargetSqr) ;
	else
		pTRex->m_Boss.m_DistFromTarget = 0 ;


	// Get angle diffence to face correct direction
	switch(pTRex->m_Boss.m_Action)
	{
		case TREX_CAVE_ATTACK_ACTION:
			pTRex->m_Boss.m_DeltaAngle = AI_GetAvoidanceAngle(pThis, pTRex->m_pTurokInCave->m_vTargetPos, pTurok, AVOIDANCE_RADIUS_DISTANCE_FACTOR) ;
			pTRex->m_Boss.m_LookDeltaAngle = AI_GetAngle(pThis, pTRex->m_pTurokInCave->m_vLookPos) ;
			break ;

		default:
			vLookPos = pTurok->ah.ih.m_vPos ;

			// Special case when Turok is in tunnels
			if (TurokInTunnel)
			{
				vLookPos = pTRex->m_Boss.m_vTarget ;
				vLookPos.z = 146*SCALING_FACTOR ;
			}

			// Setup look angle
			pTRex->m_Boss.m_DeltaAngle = AI_GetAvoidanceAngle(pThis, pTRex->m_Boss.m_vTarget, pTurok, AVOIDANCE_RADIUS_DISTANCE_FACTOR) ;
			pTRex->m_Boss.m_LookDeltaAngle = AI_GetAngle(pThis, vLookPos) ;
			break ;
	}
}





/*****************************************************************************
*
*	Function:		TRex_CheckForTurokInCave()
*
******************************************************************************
*
*	Description:	Checks to see if turok has entered a hole in the arena
*						Also activates crushes if turok is in a hole for too long
*
*	Inputs:			*pThis	-	Ptr to trex game object instance
*						*pTRex	-	Ptr to trex boss vars
*
*	Outputs:			TRUE if turok is in a hole, else FALSE
*
*****************************************************************************/
BOOL TRex_CheckForTurokInCave(CGameObjectInstance *pThis, CTRex *pTRex)
{
	CGameObjectInstance	*pTurok = CEngineApp__GetPlayer(GetApp());
	CVector3					*pTurokPos = &pTurok->ah.ih.m_vPos, vDir ;
	CCaveInfo				*pCave = CaveHoleList ;
	INT32						i = sizeof(CaveHoleList) / sizeof(CCaveInfo) ;
	CGameRegion				*pRegion;
	CCaveInfo	*pOldCave = pTRex->m_pTurokInCave ;

	// If turok is on a brige then he can't be in a hole (which are under bridges!)
	pRegion = pTurok->ah.ih.m_pCurrentRegion;
	if ((!pRegion) || (pRegion->m_wFlags & REGFLAG_BRIDGE) ||
		 (pTurok->ah.ih.m_vPos.y > (SCALING_FACTOR * 90)))
		return FALSE ;

	// In hole?
	pTRex->m_pTurokInCave = NULL ;
	for (i = 0 ; i < (sizeof(CaveHoleList) / sizeof(CCaveInfo)) ; i++)
	{
		// In square bounds of cave?
		if ((pTurokPos->x >= pCave->m_Corner1.x) && (pTurokPos->x <= pCave->m_Corner2.x) &&
			 (pTurokPos->z >= pCave->m_Corner1.z) && (pTurokPos->z <= pCave->m_Corner2.z))
		{

#if DEBUG_TREX
			if (pOldCave != pCave)
				rmonPrintf("Turok entered Hole\r\n") ;
#endif

			// Record cave
			pTRex->m_pTurokInCave = pCave ;
			break ;
		}

		// Check next cave
		pCave++ ;
	}

	// If been in cave too long, then start crusher, id's are 50,51,52,53,54,55
	if ((pOldCave) && (pCave == pOldCave) && (i < 6) && (TUROK_Health(pTurok)))
	{
		pTRex->m_TurokTimeInCave += frame_increment ;
		if ((pTRex->m_TurokTimeInCave > TREX_CAVE_CRUSH_TIME) ||
			 ((GetApp()->m_Camera.m_ShakeY.m_Amp > 0) &&
			  ((pTRex->m_Boss.m_Mode == TREX_CAVE_TAIL_SLAM_MODE) ||
			   (pTRex->m_Boss.m_Mode == TREX_TAIL_SLAM_MODE))))
		{
#if DEBUG_TREX
			rmonPrintf("Crush event sent!\r\n") ;
#endif

			AI_Event_Dispatcher(&pTurok->ah.ih, &pTurok->ah.ih, AI_MEVENT_PRESSUREPLATE,
									  pTurok->ah.ih.m_pEA->m_dwSpecies, pTurok->ah.ih.m_vPos,
									  TREX_LEVEL_CRUSHERS_START_ID+i) ;

			// Flinch back turok
			CVector3__Subtract(&pTurok->ah.m_vVelocity, &pCave->m_vTargetPos, &pCave->m_vLookPos) ;
			CVector3__MultScaler(&pTurok->ah.m_vVelocity, &pTurok->ah.m_vVelocity, 1.0/3.0) ;

			// Make Turok face crusher
			CVector3__Subtract(&vDir, &pCave->m_vLookPos, &pCave->m_vTargetPos) ;
			pTurok->m_RotY = CVector3__XZAngle(&vDir) ;

			// Kill Turok without too mush red!
			AI_Decrease_Health(pTurok, 1, TRUE, TRUE) ;
			AI_Decrease_Health(pTurok, 100, FALSE, FALSE) ;
		}
	}
	else
		pTRex->m_TurokTimeInCave = 0 ;

	// Return result
	if (pTRex->m_pTurokInCave)
		return TRUE ;
	else
		return FALSE ;

}



/*****************************************************************************
*
*	Function:		TRex_RunScriptCommand()
*
******************************************************************************
*
*	Description:	Runs trex script
*
*	Inputs:			*pThis				- Ptr to trex game object instance
*						*pTRex				- Ptr to trex boss vars
*						*pScript				- Ptr to script vars
*						EndOfAttackMode	- Mode to perform at end of attacks
*
*	Outputs:			None
*
*****************************************************************************/
void TRex_RunScriptCommand(CGameObjectInstance *pThis, CTRex *pTRex, CBossScript *pScript,
									INT32 EndOfAttackMode)
{
	INT16	NewMode ;

	// Read script mode
	NewMode = *pScript->m_pCurrent++ ;


	// Special "Repeat" ?
	if (NewMode == TREXCMND_REPEAT)
	{
		pScript->m_pCurrent = pScript->m_pStart ;
		NewMode = *pScript->m_pCurrent++ ;
	}

	// Special "End of attack" ?
	if (NewMode == TREXCMND_END_ATTACK)
		TRex_SetupMode((CGameObjectInstance *)pThis, EndOfAttackMode) ;
	else
	{
		// Read repeats
		pTRex->m_Boss.m_AnimRepeats = *pScript->m_pCurrent++ ;

		// Goto mode!
		TRex_SetupMode((CGameObjectInstance *)pThis, NewMode) ;
	}
}





/*****************************************************************************
*
*	Function:		TRex_CheckForMaxRunTime()
*
******************************************************************************
*
*	Description:	Checks to see if trex has been running too long without doing
*						an attack. If so, one is performed.
*
*	Inputs:			*pThis				- Ptr to trex game object instance
*						*pTRex				- Ptr to trex boss vars
*
*	Outputs:			TRUE is attack is performed, else FALSE
*
*****************************************************************************/
BOOL TRex_CheckForMaxRunTime(CGameObjectInstance *pThis, CTRex *pTRex)
{
	// Close enough?
	if ((pTRex->m_Boss.m_Action != TREX_CAVE_ATTACK_ACTION) &&
	    (pTRex->m_Boss.m_DistFromTarget <= TREX_MAX_RUN_TURN_ATTACK_DIST))
	{
		// Ran for too long?
		pTRex->m_RunTime += frame_increment ;
		if ((pTRex->m_RunTime > TREX_MAX_RUN_TIME) &&
			 (abs(pTRex->m_Boss.m_LookDeltaAngle) < ANGLE_DTOR(20)))
		{
	#if DEBUG_TREX
			rmonPrintf("Been running too long.. time for an attack..\r\n") ;
	#endif

			pTRex->m_RunTime -= TREX_MAX_RUN_TIME ;
			TRex_RunScriptCommand(pThis, pTRex, &pTRex->m_RunAttackScript, TREX_IDLE_MODE) ;
			return TRUE ;
		}
	}

	return FALSE ;
}




/*****************************************************************************
*
*	Function:		TRex_CheckForMaxTurnTime()
*
******************************************************************************
*
*	Description:	Checks to see if trex has been turning too long without doing
*						an attack. If so, one is performed.
*
*	Inputs:			*pThis				- Ptr to trex game object instance
*						*pTRex				- Ptr to trex boss vars
*
*	Outputs:			TRUE is attack is performed, else FALSE
*
*****************************************************************************/
BOOL TRex_CheckForMaxTurnTime(CGameObjectInstance *pThis, CTRex *pTRex)
{
	// Close enough?
	if ((pTRex->m_Boss.m_Action != TREX_CAVE_ATTACK_ACTION) &&
		 (pTRex->m_Boss.m_DistFromTarget <= TREX_MAX_RUN_TURN_ATTACK_DIST))
	{
		// Turned for too long?
		pTRex->m_TurnTime += frame_increment ;
		if (pTRex->m_TurnTime > TREX_MAX_TURN_TIME)
		{
	#if DEBUG_TREX
			rmonPrintf("Been turning too long.. time for an attack..\r\n") ;
	#endif

			pTRex->m_TurnTime -= TREX_MAX_TURN_TIME ;
			TRex_RunScriptCommand(pThis, pTRex, &pTRex->m_TurnAttackScript, TREX_IDLE_MODE) ;
			return TRUE ;
		}
	}

	return FALSE ;
}








/*****************************************************************************
*
*	Function:		TRex_CheckHitByExplosion()
*
******************************************************************************
*
*	Description:	Checks to see if trex has been hit by an explosion
*
*	Inputs:			*pThis				- Ptr to trex game object instance
*						*pTRex				- Ptr to trex boss vars
*
*	Outputs:			TRUE if explosion flinch is performed
*
*****************************************************************************/
INT16 TRexExplodeModes[] =
{
	TREX_EXPLODE_BACK_MODE,
	TREX_EXPLODE_LEFT_MODE,
	TREX_EXPLODE_FRONT_MODE,
	TREX_EXPLODE_RIGHT_MODE,
} ;

BOOL TRex_CheckHitByExplosion(CGameObjectInstance *pThis, CTRex *pTRex)
{
	CGameObjectInstance *pTurok ;
	FLOAT						AngleDiff ;
	INT32						Index ;

	// Hit by an explosion?
	if ((!(pTRex->m_Boss.m_ModeFlags & (BF_INVINCIBLE | BF_ENTER_MODE | BF_EXPLODE_MODE))) &&
		 (AI_GetDyn(pThis)->m_dwStatusFlags & AI_EV_EXPLOSION))
	{
		// Clear explosion flag
		AI_GetDyn(pThis)->m_dwStatusFlags &= ~AI_EV_EXPLOSION ;


#if DEBUG_TREX
		rmonPrintf("\r\n...EXPLOSION HIT!!...\r\n") ;
#endif

		// Been hit enough?
		if (++pTRex->m_ExplosionHits >= pTRex->m_pStageInfo->HitsBeforeExplode)
		{
			pTRex->m_ExplosionHits = 0 ;

			// Decide what to do
			switch(pTRex->m_Boss.m_Mode)
			{
				case TREX_CAVE_NOSE_HIT_MODE:
				case TREX_DEATH_MODE:
					return FALSE ;

				case TREX_CAVE_STOOP_MODE:
				case TREX_CAVE_LOOK_MODE:
				case TREX_CAVE_BITE_MODE:
					// Force anim to restart if same mode
					pTRex->m_Boss.m_OldMode = -1 ;
					TRex_SetupMode((CGameObjectInstance *)pThis, TREX_CAVE_NOSE_HIT_MODE) ;
					return TRUE ;

				default:
					// Force anim to restart if same mode
					pTRex->m_Boss.m_OldMode = -1 ;

					pTurok = CEngineApp__GetPlayer(GetApp());
					AngleDiff = (pTurok->m_RotY -  pThis->m_RotY - ANGLE_DTOR(45)) / ANGLE_DTOR(90) ;
					Index = (INT32)AngleDiff ;

					TRex_SetupMode((CGameObjectInstance *)pThis, TRexExplodeModes[Index & 3]) ;
					return TRUE ;
			}
		}
	}

	return FALSE ;
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
//	Modes:		 TREX_ON_ELEVATOR_IDLE_MODE,
//					 TREX_ON_ELEVATOR_RAGE_MODE,
//	Description: All movement code
/////////////////////////////////////////////////////////////////////////////
void TRex_Code_OnElevator(CGameObjectInstance *pThis, CTRex *pTRex)
{
	// Swap anims
	if (pThis->m_asCurrent.m_cFrame >= (pThis->m_asCurrent.m_nFrames - 5))
	{
		if (pTRex->m_Boss.m_Mode == TREX_ON_ELEVATOR_IDLE_MODE)
		{
			pTRex->m_Boss.m_Mode = TREX_ON_ELEVATOR_RAGE_MODE ;
			COnScreen__AddBossBar(&GetApp()->m_OnScreen, (CBoss *)pTRex) ;
		}
		else
		{
			pTRex->m_Boss.m_Mode = TREX_ON_ELEVATOR_IDLE_MODE ;
			CPickupMonitor__Activate(&GetApp()->m_Scene.m_PickupMonitor) ;
		}
	}
}



/////////////////////////////////////////////////////////////////////////////
//	Modes:		 TREX_RUN_MODE,
//	Description: All movement code
/////////////////////////////////////////////////////////////////////////////
void TRex_Code_Movement(CGameObjectInstance *pThis, CTRex *pTRex)
{
	FLOAT	DeltaAngle = pTRex->m_Boss.m_DeltaAngle ;

#if 0
	// Look?
	pTRex->m_LookTime += frame_increment ;
	if (pTRex->m_LookTime > SECONDS_TO_FRAMES(4))
		TRex_SetupMode((CGameObjectInstance *)pThis, TREX_LOOK_MODE) ;
	else
#endif

	// Run or limp?
	if (!(pTRex->m_pDamage->m_Flags & TREX_THIGH_TEX_DAMAGE_FLAG))
	{
		// Goto run?
		if ((pTRex->m_Boss.m_DistFromTarget > pTRex->m_AtTargetDist) &&
			 (!TRex_CheckForMaxRunTime(pThis, pTRex)))
		{
			// Assist turn
			BOSS_PerformTurn((CGameObjectInstance *)pThis, (CBoss *)pTRex, TREX_RUN_TURN_SPEED, DeltaAngle) ;

			// Turn left?
			if (DeltaAngle < -TREX_RUN_TURN90_ANGLE)
			{
				if (!TRex_CheckForMaxTurnTime(pThis, pTRex))
					pTRex->m_Boss.m_Mode = TREX_RUN_TURN_LEFT90_MODE ;
			}
			else
			// Turn Right?
			if (DeltaAngle > TREX_RUN_TURN90_ANGLE)
			{
				if (!TRex_CheckForMaxTurnTime(pThis, pTRex))
					pTRex->m_Boss.m_Mode = TREX_RUN_TURN_RIGHT90_MODE ;
			}

			// Attack?
			if (abs(DeltaAngle) <= TREX_RUN_FACING_ANGLE)
			{
				// Perform bite?
				if ((pTRex->m_Boss.m_Action == TREX_ATTACK_ACTION) &&
					 (pTRex->m_Boss.m_DistFromTarget <= TREX_RUN_BITE_DIST) &&
					 (abs(DeltaAngle) <= ANGLE_DTOR(10)))
					TRex_SetupMode((CGameObjectInstance *)pThis, TREX_RUN_BITE_MODE) ;
				else
				if (!TRex_CheckForMaxRunTime(pThis, pTRex))
				{
					pTRex->m_Boss.m_Mode = TREX_RUN_MODE ;
					BOSS_PerformTurn((CGameObjectInstance *)pThis, (CBoss *)pTRex, TREX_RUN_TURN_SPEED, DeltaAngle) ;
				}
			}
		}
	}
	else
	{
		// Goto run?
		if ((pTRex->m_Boss.m_DistFromTarget > pTRex->m_AtTargetDist) &&
			 (!TRex_CheckForMaxRunTime(pThis, pTRex)))
		{
			// Assist turn
			BOSS_PerformTurn((CGameObjectInstance *)pThis, (CBoss *)pTRex, TREX_LIMP_TURN_SPEED, DeltaAngle) ;

			// Turn left?
			if (DeltaAngle < -TREX_LIMP_TURN90_ANGLE)
			{
				if (!TRex_CheckForMaxTurnTime(pThis, pTRex))
				{
					pTRex->m_Boss.m_Mode = TREX_LIMP_TURN_LEFT90_MODE ;
					BOSS_PerformTurn((CGameObjectInstance *)pThis, (CBoss *)pTRex, TREX_LIMP_TURN_SPEED, DeltaAngle) ;
				}
			}
			else
			// Turn Right?
			if (DeltaAngle > TREX_LIMP_TURN90_ANGLE)
			{
				if (!TRex_CheckForMaxTurnTime(pThis, pTRex))
				{
					pTRex->m_Boss.m_Mode = TREX_LIMP_TURN_RIGHT90_MODE ;
					BOSS_PerformTurn((CGameObjectInstance *)pThis, (CBoss *)pTRex, TREX_LIMP_TURN_SPEED, DeltaAngle) ;
				}
			}

			// Run?
			if (abs(DeltaAngle) <= TREX_LIMP_FACING_ANGLE)
			{
				pTRex->m_Boss.m_Mode = TREX_LIMP_MODE ;
				BOSS_PerformTurn((CGameObjectInstance *)pThis, (CBoss *)pTRex, TREX_LIMP_TURN_SPEED, DeltaAngle) ;
			}
		}
	}

	// Attack?
	if (pTRex->m_Boss.m_DistFromTarget <= pTRex->m_AtTargetDist)
	{
		DeltaAngle = pTRex->m_Boss.m_LookDeltaAngle ;

		// Turn?
		if (pTRex->m_Boss.m_Mode != TREX_TURN180_MODE)
		{
			// Help turning
			BOSS_PerformTurn((CGameObjectInstance *)pThis, (CBoss *)pTRex, TREX_IDLE_TURN_SPEED, DeltaAngle) ;

			// Turn 180?
			if (abs(DeltaAngle) > TREX_IDLE_TURN180_ANGLE)
			{
				if (!TRex_CheckForMaxTurnTime(pThis, pTRex))
					pTRex->m_Boss.m_Mode = TREX_TURN180_MODE ;
			}
			else
			// Turn left?
			if (DeltaAngle < -TREX_IDLE_TURN90_ANGLE)
			{
				if (!TRex_CheckForMaxTurnTime(pThis, pTRex))
					pTRex->m_Boss.m_Mode = TREX_TURN_LEFT90_MODE ;
			}
			else
			// Turn Right?
			if (DeltaAngle > TREX_IDLE_TURN90_ANGLE)
			{
				if (!TRex_CheckForMaxTurnTime(pThis, pTRex))
					pTRex->m_Boss.m_Mode = TREX_TURN_RIGHT90_MODE ;
			}
		}

		// Attack?
		if (abs(DeltaAngle) <= TREX_IDLE_FACING_ANGLE)
		{
			// Choose attack
			switch(pTRex->m_Boss.m_Action)
			{
				case TREX_ATTACK_ACTION:

					// Perform close attack or normal attack?
					if (pTRex->m_Boss.m_DistFromTarget <= TREX_CLOSE_ATTACK_RANGE)
						TRex_RunScriptCommand(pThis, pTRex, &pTRex->m_CloseAttackScript, TREX_IDLE_MODE) ;
					else
						TRex_RunScriptCommand(pThis, pTRex, &pTRex->m_AttackScript, TREX_IDLE_MODE) ;
					break ;

				case TREX_LEDGE_ATTACK_ACTION:
					TRex_RunScriptCommand(pThis, pTRex, &pTRex->m_LedgeAttackScript, TREX_IDLE_MODE) ;
					break ;

				case TREX_CAVE_ATTACK_ACTION:
					TRex_SetupMode((CGameObjectInstance *)pThis, TREX_CAVE_STOOP_MODE) ;
					break ;
			}
		}
	}
}


/////////////////////////////////////////////////////////////////////////////
//	Modes:		 TREX_RUN_BITE_MODE,
//	Description: Running and biting at the same time
/////////////////////////////////////////////////////////////////////////////
void TRex_Code_RUN_BITE(CGameObjectInstance *pThis, CTRex *pTRex)
{
	// Finished turn?
//	if (pTRex->m_Boss.m_AnimPlayed)
	if (pThis->m_asCurrent.m_cFrame >= (pThis->m_asCurrent.m_nFrames - 5))
		TRex_SetupMode((CGameObjectInstance *)pThis, TREX_RUN_MODE) ;
}






/////////////////////////////////////////////////////////////////////////////
//	Modes:		 TREX_BITE_MODE,
//					 TREX_TAIL_SLAM_MODE,
//	Description: Biting etc. at player
/////////////////////////////////////////////////////////////////////////////
void TRex_Code_Attack(CGameObjectInstance *pThis, CTRex *pTRex)
{
	// Finished?
//	if (pTRex->m_Boss.m_AnimPlayed)
	if (pThis->m_asCurrent.m_cFrame >= (pThis->m_asCurrent.m_nFrames - 5))
		TRex_SetupMode((CGameObjectInstance *)pThis, TREX_IDLE_MODE) ;
}

/////////////////////////////////////////////////////////////////////////////
// Modes:		 TREX_BITE_UP_MODE,
//					 TREX_FIRE_UP_MODE,
//	Description: Biting etc. at player
/////////////////////////////////////////////////////////////////////////////
void TRex_Code_LedgeAttack(CGameObjectInstance *pThis, CTRex *pTRex)
{
	// Finished?
//	if (pTRex->m_Boss.m_AnimPlayed)
	if (pThis->m_asCurrent.m_cFrame >= (pThis->m_asCurrent.m_nFrames - 5))
		TRex_SetupMode((CGameObjectInstance *)pThis, TREX_IDLE_MODE) ;
}



/////////////////////////////////////////////////////////////////////////////
//	Modes:		 TREX_BREATH_FIRE_MODE,
//					 TREX_EYE_FIRE_MODE,
//	Description: Fire at player(loops)
/////////////////////////////////////////////////////////////////////////////
void TRex_Setup_FireLoop(CGameObjectInstance *pThis, CTRex *pTRex, INT32 NewMode)
{
	// If only one repeat, use single anim in/out instead
	if (pTRex->m_Boss.m_AnimRepeats == 1)
		TRex_SetupMode((CGameObjectInstance *)pThis, NewMode + 1) ;
	else
		pTRex->m_Boss.m_Mode = NewMode ;
}

void TRex_Setup_BREATH_FIRE(CGameObjectInstance *pThis, CTRex *pTRex)
{
	TRex_Setup_FireLoop(pThis, pTRex, TREX_BREATH_FIRE_MODE) ;
}

void TRex_Setup_EYE_FIRE(CGameObjectInstance *pThis, CTRex *pTRex)
{
	TRex_Setup_FireLoop(pThis, pTRex, TREX_EYE_FIRE_MODE) ;
}

void TRex_Code_FireLoop(CGameObjectInstance *pThis, CTRex *pTRex)
{
	// Rotate to aim at Turok when breathing fire
	if (pTRex->m_Boss.m_Mode == TREX_BREATH_FIRE_MODE)
		BOSS_RotateToFaceTurok(pThis, (CBoss *)pTRex, TREX_TURN_ON_THE_SPOT_SPEED) ;

	// Finished?
	if (pTRex->m_Boss.m_AnimPlayed >= pTRex->m_Boss.m_AnimRepeats)
		TRex_SetupMode((CGameObjectInstance *)pThis, pTRex->m_Boss.m_Mode + 2) ;
}


/////////////////////////////////////////////////////////////////////////////
//	Modes:		 TREX_BREATH_FIRE_IN_MODE,
//					 TREX_EYE_FIRE_IN_MODE,
//	Description: Preparing to fire at player
/////////////////////////////////////////////////////////////////////////////
void TRex_Code_FireIn(CGameObjectInstance *pThis, CTRex *pTRex)
{
	// Finished?
//	if (pTRex->m_Boss.m_AnimPlayed)
	if (pThis->m_asCurrent.m_cFrame >= (pThis->m_asCurrent.m_nFrames - 5))
		TRex_SetupMode((CGameObjectInstance *)pThis, pTRex->m_Boss.m_Mode + 1) ;
}

/////////////////////////////////////////////////////////////////////////////
//	Modes:		 TREX_BREATH_FIRE_OUT_MODE,
//					 TREX_EYE_FIRE_OUT_MODE,
//	Description: Returning to idle from firing at player
/////////////////////////////////////////////////////////////////////////////
void TRex_Code_FireOut(CGameObjectInstance *pThis, CTRex *pTRex)
{
	// Finished?
	if (pThis->m_asCurrent.m_cFrame >= (pThis->m_asCurrent.m_nFrames - 5))
		TRex_SetupMode((CGameObjectInstance *)pThis, TREX_IDLE_MODE) ;
}




/////////////////////////////////////////////////////////////////////////////
//	Modes:		 TREX_CAVE_STOOP_MODE,
//	Description: Stooping towards cave
/////////////////////////////////////////////////////////////////////////////

#define TREX_AT_CAVE_DIST_SQR			(TREX_AT_CAVE_DIST * TREX_AT_CAVE_DIST)


void TRex_Code_CAVE_STOOP(CGameObjectInstance *pThis, CTRex *pTRex)
{
	// Make TRex face cave
	if (pTRex->m_Boss.m_Action == TREX_CAVE_ATTACK_ACTION)
		AI_NudgeRotY(pThis, pTRex->m_Boss.m_LookDeltaAngle / 32) ;

	// Save current cave
	if (!pTRex->m_pTargetCave)
		pTRex->m_pTargetCave = pTRex->m_pTurokInCave ;

	// Turok out of cave?
	if ((!pTRex->m_pTurokInCave) || (pTRex->m_pTurokInCave != pTRex->m_pTargetCave))
		TRex_SetupMode(pThis, TREX_CAVE_OUT_MODE) ;
	else
	// Close enough to start attack?
	if ((AI_XZDistanceSquared(pThis, pTRex->m_pTurokInCave->m_vLookPos) < TREX_AT_CAVE_DIST_SQR) ||
		 (pThis->m_asCurrent.m_cFrame >= (pThis->m_asCurrent.m_nFrames - 5)))
	{
		TRex_RunScriptCommand(pThis, pTRex, &pTRex->m_CaveAttackScript, TREX_CAVE_LOOK_MODE) ;

#if DEBUG_TREX
		rmonPrintf("TRex dist from centre of cave look pos:%f\r\n", sqrt(AI_XZDistanceSquared(pThis, pTRex->m_pTurokInCave->m_vLookPos)) / SCALING_FACTOR) ;
#endif
	}

}


/////////////////////////////////////////////////////////////////////////////
//	Modes:		 TREX_CAVE_LOOK_MODE,
//					 TREX_CAVE_BITE_MODE,
//	Description: Attacking Turok in cave
/////////////////////////////////////////////////////////////////////////////
void TRex_Code_CaveAttack(CGameObjectInstance *pThis, CTRex *pTRex)
{
	// Finished?
	if (pTRex->m_Boss.m_AnimPlayed >= pTRex->m_Boss.m_AnimRepeats)
	{
		// Turok out of cave?
		if ((!pTRex->m_pTurokInCave) || (pTRex->m_pTurokInCave != pTRex->m_pTargetCave))
			TRex_SetupMode(pThis, TREX_CAVE_OUT_MODE) ;
		else
			TRex_RunScriptCommand(pThis, pTRex, &pTRex->m_CaveAttackScript, TREX_CAVE_LOOK_MODE) ;
	}
}


/////////////////////////////////////////////////////////////////////////////
//	Modes:		 TREX_CAVE_OUT_MODE,
//	Description: Getting out of cave
/////////////////////////////////////////////////////////////////////////////
void TRex_Setup_CAVE_OUT(CGameObjectInstance *pThis, CTRex *pTRex)
{
	pTRex->m_Boss.m_Mode = TREX_CAVE_OUT_MODE ;
	pTRex->m_TurokInCaveWaitTime = RandomRangeFLOAT(SECONDS_TO_FRAMES(2), SECONDS_TO_FRAMES(4)) ;
}


void TRex_Code_CAVE_OUT(CGameObjectInstance *pThis, CTRex *pTRex)
{
	// Finished?
	if (pThis->m_asCurrent.m_cFrame >= (pThis->m_asCurrent.m_nFrames - 5))
	{
		pTRex->m_pTargetCave = NULL ;
		TRex_SetupMode(pThis, TREX_IDLE_MODE) ;
	}
}



/////////////////////////////////////////////////////////////////////////////
//	Modes:		 TREX_CAVE_NOSE_HIT_MODE,
//	Description: Hit on the end of his konk by an explosion
/////////////////////////////////////////////////////////////////////////////
void TRex_Code_CAVE_NOSE_HIT(CGameObjectInstance *pThis, CTRex *pTRex)
{
	// Finished?
	if (pThis->m_asCurrent.m_cFrame >= (pThis->m_asCurrent.m_nFrames - 5))
		TRex_SetupMode(pThis, TREX_CAVE_RAGE_MODE) ;
}

/////////////////////////////////////////////////////////////////////////////
//	Modes:		 TREX_CAVE_RAGE_MODE,
//	Description: Loosing his rag
/////////////////////////////////////////////////////////////////////////////
void TRex_Code_CAVE_RAGE(CGameObjectInstance *pThis, CTRex *pTRex)
{
	// Finished?
	if (pThis->m_asCurrent.m_cFrame >= (pThis->m_asCurrent.m_nFrames - 5))
		TRex_SetupMode(pThis, TREX_CAVE_TAIL_SLAM_MODE) ;
}

/////////////////////////////////////////////////////////////////////////////
//	Modes:		 TREX_CAVE_TAIL_SLAM_MODE,
//	Description: Getting very nast and angry
/////////////////////////////////////////////////////////////////////////////
void TRex_Code_CAVE_TAIL_SLAM(CGameObjectInstance *pThis, CTRex *pTRex)
{
	// Finished?
	if ((pTRex->m_Boss.m_AnimPlayed >= (pTRex->m_pStageInfo->CaveTailSlams-1)) &&
		 (pThis->m_asCurrent.m_cFrame >= (pThis->m_asCurrent.m_nFrames - 5)))
		TRex_SetupMode(pThis, TREX_IDLE_MODE) ;
}





/////////////////////////////////////////////////////////////////////////////
//	Modes:		 TREX_EXPLODE_LEFT_MODE,
//					 TREX_EXPLODE_RIGHT_MODE,
//					 TREX_EXPLODE_FRONT_MODE,
//					 TREX_EXPLODE_BACK_MODE,
//	Description: Hit by an explosion
/////////////////////////////////////////////////////////////////////////////
void TRex_Code_Explode(CGameObjectInstance *pThis, CTRex *pTRex)
{
	// Finished turn?
//	if (pTRex->m_Boss.m_AnimPlayed)
	if (pThis->m_asCurrent.m_cFrame >= (pThis->m_asCurrent.m_nFrames - 5))
		TRex_SetupMode((CGameObjectInstance *)pThis, TREX_IDLE_MODE) ;
}



/////////////////////////////////////////////////////////////////////////////
//	Modes:		 TREX_STUNNED_MODE
//	Description: Felling a bit dizzy
/////////////////////////////////////////////////////////////////////////////
void TRex_Code_STUNNED(CGameObjectInstance *pThis, CTRex *pTRex)
{
	// Do explosion
	if (!(game_frame_number & 7))
		BOSS_DoNodeDamage(pThis, pTRex->m_pDamage->m_Node1) ;

	// Do explosion
	if (!((game_frame_number+2) & 7))
		BOSS_DoNodeDamage(pThis, pTRex->m_pDamage->m_Node2) ;

	// Speed up stun
	pTRex->m_Boss.m_ModeAnimSpeed += (TREX_STUNNED_MAX_ANIM_SPEED - 1.0) / TREX_STUNNED_TIME ;

	// Finished?
	if (pTRex->m_Boss.m_ModeTime >= TREX_STUNNED_TIME)
		TRex_SetupMode(pThis, TREX_STUN_OUT_MODE) ;
}


/////////////////////////////////////////////////////////////////////////////
//	Modes:		 TREX_STUN_OUT_MODE
//	Description: Going back to idle
/////////////////////////////////////////////////////////////////////////////
void TRex_Code_STUN_OUT(CGameObjectInstance *pThis, CTRex *pTRex)
{
	// Finished turn?
	if (pThis->m_asCurrent.m_cFrame >= (pThis->m_asCurrent.m_nFrames - 5))
		TRex_SetupMode((CGameObjectInstance *)pThis, TREX_IDLE_MODE) ;
}




/////////////////////////////////////////////////////////////////////////////
//	Modes:		 TREX_EAT_TUROK_MODE
//	Description: Eating Turok
/////////////////////////////////////////////////////////////////////////////
void TRex_Code_EAT_TUROK(CGameObjectInstance *pThis, CTRex *pTRex)
{
	// Safety check - Cinema finished?
	if (GetApp()->m_Camera.m_Mode != CAMERA_CINEMA_TREX_KILL_TUROK_MODE)
	{
		// Force into idle mode and start the anim
		pTRex->m_Boss.m_Mode = TREX_IDLE_MODE ;
		pTRex->m_Boss.m_OldMode = -1 ;
	}
}





/*****************************************************************************
*
*	Function:		TRex_DisplayMode()
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
void TRex_DisplayMode(CBoss *pBoss)
{
#if DEBUG_TREX
	switch(pBoss->m_Mode)
	{
		case TREX_ON_ELEVATOR_IDLE_MODE:
			rmonPrintf("TREX_ON_ELEVATOR_IDLE_MODE\r\n") ;
			break ;
		case TREX_ON_ELEVATOR_RAGE_MODE:
			rmonPrintf("TREX_ON_ELEVATOR_RAGE_MODE\r\n") ;
			break ;
		case TREX_ON_ELEVATOR_LOOK_MODE:
			rmonPrintf("TREX_ON_ELEVATOR_LOOK_MODE\r\n") ;
			break ;
		case TREX_EXIT_ELEVATOR_MODE:
			rmonPrintf("TREX_EXIT_ELEVATOR_MODE\r\n") ;
			break ;
		case TREX_LOOK_MODE:
			rmonPrintf("TREX_LOOK_MODE\r\n") ;
			break ;
		case TREX_TURN_LEFT90_MODE:
			rmonPrintf("TREX_TURN_LEFT90_MODE\r\n") ;
			break ;
		case TREX_TURN_RIGHT90_MODE:
			rmonPrintf("TREX_TURN_RIGHT90_MODE\r\n") ;
			break ;
		case TREX_TURN180_MODE:
			rmonPrintf("TREX_TURN180_MODE\r\n") ;
			break ;
		case TREX_RUN_MODE:
			rmonPrintf("TREX_RUN_MODE\r\n") ;
			break ;
		case TREX_RUN_TURN_LEFT90_MODE:
			rmonPrintf("TREX_RUN_TURN_LEFT90_MODE\r\n") ;
			break ;
		case TREX_RUN_TURN_RIGHT90_MODE:
			rmonPrintf("TREX_RUN_TURN_RIGHT90_MODE\r\n") ;
			break ;
		case TREX_RUN_BITE_MODE:
			rmonPrintf("TREX_RUN_BITE_MODE\r\n") ;
			break ;
		case TREX_LIMP_MODE:
			rmonPrintf("TREX_LIMP_MODE\r\n") ;
			break ;
		case TREX_LIMP_TURN_LEFT90_MODE:
			rmonPrintf("TREX_LIMP_TURN_LEFT90_MODE\r\n") ;
			break ;
		case TREX_LIMP_TURN_RIGHT90_MODE:
			rmonPrintf("TREX_LIMP_TURN_RIGHT90_MODE\r\n") ;
			break ;
		case TREX_TAIL_SLAM_MODE:
			rmonPrintf("TREX_TAIL_SLAM_MODE\r\n") ;
			break ;
		case TREX_BITE_MODE:
			rmonPrintf("TREX_BITE_MODE\r\n") ;
			break ;
		case TREX_RAGE_MODE:
			rmonPrintf("TREX_RAGE_MODE\r\n") ;
			break ;
		case TREX_BREATH_FIRE_MODE:
			rmonPrintf("TREX_BREATH_FIRE_MODE\r\n") ;
			break ;
		case TREX_BREATH_FIRE_IN_MODE:
			rmonPrintf("TREX_BREATH_FIRE_IN_MODE\r\n") ;
			break ;
		case TREX_BREATH_FIRE_OUT_MODE:
			rmonPrintf("TREX_BREATH_FIRE_OUT_MODE\r\n") ;
			break ;
		case TREX_EYE_FIRE_MODE:
			rmonPrintf("TREX_EYE_FIRE_MODE\r\n") ;
			break ;
		case TREX_EYE_FIRE_IN_MODE:
			rmonPrintf("TREX_EYE_FIRE_IN_MODE\r\n") ;
			break ;
		case TREX_EYE_FIRE_OUT_MODE:
			rmonPrintf("TREX_EYE_FIRE_OUT_MODE\r\n") ;
			break ;
		case TREX_BITE_UP_MODE:
			rmonPrintf("TREX_BITE_UP_MODE\r\n") ;
			break ;
		case TREX_FIRE_UP_MODE:
			rmonPrintf("TREX_FIRE_UP_MODE\r\n") ;
			break ;
		case TREX_CAVE_STOOP_MODE:
			rmonPrintf("TREX_CAVE_STOOP_MODE\r\n") ;
			break ;
		case TREX_CAVE_LOOK_MODE:
			rmonPrintf("TREX_CAVE_LOOK_MODE\r\n") ;
			break ;
		case TREX_CAVE_BITE_MODE:
			rmonPrintf("TREX_CAVE_BITE_MODE\r\n") ;
			break ;
		case TREX_CAVE_OUT_MODE:
			rmonPrintf("TREX_CAVE_OUT_MODE\r\n") ;
			break ;
		case TREX_CAVE_NOSE_HIT_MODE:
			rmonPrintf("TREX_CAVE_NOSE_HIT_MODE\r\n") ;
			break ;
		case TREX_CAVE_RAGE_MODE:
			rmonPrintf("TREX_CAVE_RAGE_MODE\r\n") ;
			break ;
		case TREX_CAVE_TAIL_SLAM_MODE:
			rmonPrintf("TREX_CAVE_TAIL_SLAM_MODE\r\n") ;
			break ;
		case TREX_EXPLODE_LEFT_MODE:
			rmonPrintf("TREX_EXPLODE_LEFT_MODE\r\n") ;
			break ;
		case TREX_EXPLODE_RIGHT_MODE:
			rmonPrintf("TREX_EXPLODE_RIGHT_MODE\r\n") ;
			break ;
		case TREX_EXPLODE_FRONT_MODE:
			rmonPrintf("TREX_EXPLODE_FRONT_MODE\r\n") ;
			break ;
		case TREX_EXPLODE_BACK_MODE:
			rmonPrintf("TREX_EXPLODE_BACK_MODE\r\n") ;
			break ;
		case TREX_STUNNED_MODE:
			rmonPrintf("TREX_STUNNED_MODE\r\n") ;
			break ;
		case TREX_STUN_OUT_MODE:
			rmonPrintf("TREX_STUN_OUT_MODE\r\n") ;
			break ;
		case TREX_DEATH_MODE:
			rmonPrintf("TREX_DEATH_MODE\r\n") ;
			break ;

		default:
			rmonPrintf("Mode: %d\r\n", pBoss->m_Mode) ;
			break ;
	}
#endif
}
#endif


/////////////////////////////////////////////////////////////////////////////
// TRex texture index
/////////////////////////////////////////////////////////////////////////////
int CGameObjectInstance__GetTRexTextureIndex(CGameObjectInstance *pThis, int nNode)
{
	UINT32	DamageFlags = ((CTRex *)(pThis->m_pBoss))->m_pDamage->m_Flags ;

	switch(nNode)
	{
		case TREX_TAIL1_NODE:
		case TREX_TAIL2_NODE:
		case TREX_TAIL3_NODE:
		case TREX_TAIL4_NODE:
		case TREX_TAIL5_NODE:
			return ((DamageFlags & TREX_TAIL_TEX_DAMAGE_FLAG) != 0) ;

		case TREX_WRIST1_NODE:
		case TREX_NAIL1_NODE:
		case TREX_NAIL2_NODE:
		case TREX_NAIL1_TIP_NODE:
		case TREX_NAIL2_TIP_NODE:

		case TREX_WRIST2_NODE:
		case TREX_NAIL4_NODE:
		case TREX_NAIL5_NODE:
		case TREX_NAIL4_TIP_NODE:
		case TREX_NAIL5_TIP_NODE:
			return ((DamageFlags & TREX_WRIST_TEX_DAMAGE_FLAG) != 0) ;

		case TREX_FOREARM1_NODE:
		case TREX_FOREARM2_NODE:
			return ((DamageFlags & TREX_FOREARM_TEX_DAMAGE_FLAG) != 0) ;

		case TREX_SHOULDER1_NODE:
		case TREX_SHOULDER2_NODE:
		case TREX_BICEP1_NODE:
		case TREX_BICEP2_NODE:
			return ((DamageFlags & TREX_UPPER_ARM_TEX_DAMAGE_FLAG) != 0) ;

		case TREX_CHEST_NODE:
			return ((DamageFlags & TREX_CHEST_TEX_DAMAGE_FLAG) != 0) ;

		case TREX_THIGH1_NODE:
		case TREX_THIGH2_NODE:
			return ((DamageFlags & TREX_THIGH_TEX_DAMAGE_FLAG) != 0) ;

		default:
			return 0 ;
	}
}







