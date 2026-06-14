// Mantis boss control file by Stephen Broumley

#include "stdafx.h"

#include "ai.h"
#include "aistand.h"

#include	"regtype.h"
#include "romstruc.h"
#include "tengine.h"
#include "scaling.h"
#include "audio.h"
#include "sfx.h"
#include "tmove.h"
#include "mantis.h"
#include "boss.h"
#include "bossflgs.h"
#include "onscrn.h"
#include "dlists.h"
#include "tmove.h"
#include "fx.h"
#include "attract.h"
#include "wallcoll.h"


/////////////////////////////////////////////////////////////////////////////
// Constants
/////////////////////////////////////////////////////////////////////////////

#define	DEBUG_MANTIS	  				0
#define	DEBUG_MANTIS_HEAD_TRACK		0
#define	DEBUG_MANTIS_CEILING_TURNS	0
#define	MANTIS_HEALTH					3000


/////////////////////////////////////////////////////////////////////////////
// Extra animations
/////////////////////////////////////////////////////////////////////////////
#define	AI_ANIM_BOSS_MANTIS_FORWARD_SWIPE_FLOOR		AI_ANIM_ATTACK_STRONG1


/////////////////////////////////////////////////////////////////////////////
// Movement constants
/////////////////////////////////////////////////////////////////////////////
#define	MANTIS_GRAVITY								GRAVITY_ACCELERATION

#define	MANTIS_ON_WALL_COLLISION_RADIUS		(0.25*SCALING_FACTOR)
#define	MANTIS_COLLISION_RADIUS					(10*SCALING_FACTOR)

#define	MANTIS_TILT_X_SPEED						ANGLE_DTOR(1)
#define	MANTIS_TILT_Z_SPEED						ANGLE_DTOR(1)

#define	MANTIS_TARGET_DIST						(0*SCALING_FACTOR)
#define	MANTIS_VICTORY_DIST						(80*SCALING_FACTOR)
#define	MANTIS_WALK_TO_TARGET_DIST				(80*SCALING_FACTOR)
#define	MANTIS_FLY_TO_TARGET_DIST				(140*SCALING_FACTOR)

#define	MANTIS_ATTACK_AT_TARGET_DIST			(50*SCALING_FACTOR)
#define	MANTIS_ATTACK_CLOSE_DIST				(35*SCALING_FACTOR)
#define	MANTIS_VICTORY_AT_TARGET_DIST			(10*SCALING_FACTOR)
#define	MANTIS_BREAK_WALL_AT_TARGET_DIST		(18*SCALING_FACTOR)

#define	MANTIS_STAY_ON_WALL_MAX_DIST			(250*SCALING_FACTOR)
#define	MANTIS_STAY_ON_CEILING_MAX_DIST		(250*SCALING_FACTOR)

#define	MANTIS_SURFACE_TURN_SPEED				ANGLE_DTOR(4)
#define	MANTIS_FLY_TURN_SPEED					ANGLE_DTOR(6)
#define	MANTIS_MAX_JUMP_FLY_TIME				1.5

#define	MANTIS_GOTO_ATTACK_ACTION_RADIUS 	(500*SCALING_FACTOR)

#define	WALL_AT_SIDE_MAX_LOOK_DIST				(150 * SCALING_FACTOR)
#define	WALL_AT_SIDE_MIN_LOOK_DIST				(5 * SCALING_FACTOR)
#define	MANTIS_HALF_LENGTH						(20 * SCALING_FACTOR)


// Head tracking stuff
#define	MANTIS_ROTY_OFFSET						ANGLE_DTOR(0)

#define	MANTIS_WALL_HEIGHT						(60*SCALING_FACTOR)		// Height when on wall


/////////////////////////////////////////////////////////////////////////////
// Other file externs
/////////////////////////////////////////////////////////////////////////////
extern	CTMove		*pCTMove ;		// pointer to turok movement object


/////////////////////////////////////////////////////////////////////////////
// Function prototypes
/////////////////////////////////////////////////////////////////////////////

// Initialisation
void CCollisionInfo__SetMantisCollisionDefaults(CCollisionInfo2 *pThis) ;
void Mantis_SetupStageInfo(CGameObjectInstance *pThis, CMantis *pMantis, CMantisStageInfo *Info) ;


// General code
void Mantis_Update(CGameObjectInstance *pThis, CMantis *pMantis) ;
INT32 Mantis_SetupMode(CGameObjectInstance *pThis, INT32 nNewMode) ;
void Mantis_UpdateAnimSpeed(CGameObjectInstance *pThis, CMantis *pMantis) ;
void Mantis_CheckForDeath(CGameObjectInstance *pThis, CMantis *pMantis) ;
void Mantis_PreDraw(CGameObjectInstance *pThis, CMantis *pMantis, Gfx **ppDLP) ;
void Mantis_PostDraw(CGameObjectInstance *pThis, CMantis *pMantis, Gfx **ppDLP) ;
void Mantis_GetTarget(CGameObjectInstance *pThis, CMantis *pMantis) ;
BOOL Mantis_FacingTurok(CGameObjectInstance *pThis, CMantis *pMantis) ;
BOOL Mantis_WallAtSide(CGameObjectInstance *pThis, CMantis *pMantis, FLOAT Dir) ;
BOOL Mantis_WallAtLeftSide(CGameObjectInstance *pThis, CMantis *pMantis) ;
BOOL Mantis_WallAtRightSide(CGameObjectInstance *pThis, CMantis *pMantis) ;
BOOL Mantis_NearWall(CGameObjectInstance *pThis, CMantis *pMantis) ;
void Mantis_CheckHitByExplosion(CGameObjectInstance *pThis, CMantis *pMantis) ;
BOOL Mantis_MaxWalkCheck(CGameObjectInstance *pThis, CMantis *pMantis, MantisSetupFunction Function) ;
BOOL Mantis_ShouldStayOnWall(CGameObjectInstance *pThis, CMantis *pMantis) ;
BOOL Mantis_CanSpit(CGameObjectInstance *pThis, CMantis *pMantis) ;
void Mantis_KeepOnWall(CGameObjectInstance *pThis, CMantis *pMantis) ;
void Mantis_CheckForCollapsingWall(CGameObjectInstance *pThis, CMantis *pMantis) ;
void Mantis_StartGlow(CGameObjectInstance *pThis, CMantis *pMantis, INT16 Count) ;

void Mantis_UpdateAction(CGameObjectInstance *pThis, CMantis *pMantis) ;

// Mode code
void Mantis_Code_SLEEPING_FLOOR(CGameObjectInstance *pThis, CMantis *pMantis) ;

void Mantis_Setup_BREAK_FLOOR(CGameObjectInstance *pThis, CMantis *pMantis) ;
void Mantis_Code_BREAK_FLOOR(CGameObjectInstance *pThis, CMantis *pMantis) ;

void Mantis_Code_RAGE_FLOOR(CGameObjectInstance *pThis, CMantis *pMantis) ;

void Mantis_Code_FloorMovement(CGameObjectInstance *pThis, CMantis *pMantis) ;
void Mantis_Code_CeilingMovement(CGameObjectInstance *pThis, CMantis *pMantis) ;

void Mantis_RunScript(CGameObjectInstance *pThis, CMantis *pMantis, CBossScript *pScript) ;

void Mantis_Setup_FloorAttack(CGameObjectInstance *pThis, CMantis *pMantis) ;
void Mantis_Code_FloorAttack(CGameObjectInstance *pThis, CMantis *pMantis) ;

void Mantis_Setup_CeilingAttack(CGameObjectInstance *pThis, CMantis *pMantis) ;
void Mantis_Code_CeilingAttack(CGameObjectInstance *pThis, CMantis *pMantis) ;

void Mantis_Setup_LeftWallAttack(CGameObjectInstance *pThis, CMantis *pMantis) ;
void Mantis_Code_LeftWallAttack(CGameObjectInstance *pThis, CMantis *pMantis) ;

void Mantis_Setup_RightWallAttack(CGameObjectInstance *pThis, CMantis *pMantis) ;
void Mantis_Code_RightWallAttack(CGameObjectInstance *pThis, CMantis *pMantis) ;

void Mantis_Code_JUMP_FLOOR_TO_CEILING(CGameObjectInstance *pThis, CMantis *pMantis) ;


void Mantis_Code_JUMP_CEILING_TO_FLOOR(CGameObjectInstance *pThis, CMantis *pMantis) ;

void Mantis_Setup_JUMP_FLOOR_A(CGameObjectInstance *pThis, CMantis *pMantis) ;
void Mantis_Code_JUMP_FLOOR_A(CGameObjectInstance *pThis, CMantis *pMantis) ;
void Mantis_Setup_JUMP_FLOOR_B(CGameObjectInstance *pThis, CMantis *pMantis) ;
void Mantis_Code_JUMP_FLOOR_B(CGameObjectInstance *pThis, CMantis *pMantis) ;
void Mantis_Setup_JUMP_FLOOR_C(CGameObjectInstance *pThis, CMantis *pMantis) ;
void Mantis_Code_JUMP_FLOOR_C(CGameObjectInstance *pThis, CMantis *pMantis) ;

void Mantis_Setup_JUMP_FLOOR_TO_LEFT_WALL_A(CGameObjectInstance *pThis, CMantis *pMantis) ;
void Mantis_Setup_JUMP_LEFT_WALL_TO_RIGHT_WALL_A(CGameObjectInstance *pThis, CMantis *pMantis) ;
void Mantis_Setup_JUMP_FLOOR_TO_RIGHT_WALL_A(CGameObjectInstance *pThis, CMantis *pMantis) ;
void Mantis_Setup_JUMP_RIGHT_WALL_TO_LEFT_WALL_A(CGameObjectInstance *pThis, CMantis *pMantis) ;

void Mantis_Code_JumpToWallA(CGameObjectInstance *pThis, CMantis *pMantis) ;
void Mantis_Code_JumpToWallB(CGameObjectInstance *pThis, CMantis *pMantis) ;
void Mantis_Code_JumpToWallC(CGameObjectInstance *pThis, CMantis *pMantis) ;

void Mantis_Code_JumpWallToFloor(CGameObjectInstance *pThis, CMantis *pMantis) ;
void Mantis_Code_JumpWallToCeiling(CGameObjectInstance *pThis, CMantis *pMantis) ;


void Mantis_Setup_ExplodeFloor(CGameObjectInstance *pThis, CMantis *pMantis) ;
void Mantis_Code_ExplodeFloor(CGameObjectInstance *pThis, CMantis *pMantis) ;
void Mantis_Code_ExplodeCeiling(CGameObjectInstance *pThis, CMantis *pMantis) ;
void Mantis_Code_ExplodeLeftWall(CGameObjectInstance *pThis, CMantis *pMantis) ;
void Mantis_Code_ExplodeRightWall(CGameObjectInstance *pThis, CMantis *pMantis) ;

void Mantis_Setup_DEATH(CGameObjectInstance *pThis, CMantis *pMantis) ;
void Mantis_Code_DEATH(CGameObjectInstance *pThis, CMantis *pMantis) ;

void Mantis_Code_CINEMA(CGameObjectInstance *pThis, CMantis *pMantis) ;


#ifndef MAKE_CART
void Mantis_DisplayMode(CBoss *pBoss) ;
#endif


/////////////////////////////////////////////////////////////////////////////
// Variables
/////////////////////////////////////////////////////////////////////////////


// Special fx lights
//Lights1	MantisLight ;


// Mantis itself
CMantis				MantisBoss ;

// Mantis mode table
CBossModeInfo	MantisModeTable[] =
{
	BOSS_MODE_INFO(NULL,								Mantis_Code_SLEEPING_FLOOR, 	AI_ANIM_BOSS_MANTIS_BREAK_FLOOR,0, BF_HEAD_TRACK_TUROK | BF_FLOOR_MODE),
	BOSS_MODE_INFO(Mantis_Setup_BREAK_FLOOR,	Mantis_Code_BREAK_FLOOR,		AI_ANIM_BOSS_MANTIS_BREAK_FLOOR,1, BF_HEAD_TRACK_TUROK | BF_FLOOR_MODE),

	BOSS_MODE_INFO(Mantis_Code_FloorMovement,	Mantis_Code_FloorMovement,		AI_ANIM_BOSS_MANTIS_IDLE_FLOOR,1, BF_HEAD_TRACK_TUROK | BF_FLOOR_MODE),
	BOSS_MODE_INFO(NULL,								Mantis_Code_FloorAttack,	 	AI_ANIM_BOSS_MANTIS_LOOK,1,0 | BF_FLOOR_MODE),

	BOSS_MODE_INFO(NULL,								Mantis_Code_FloorMovement,		AI_ANIM_BOSS_MANTIS_WALK_FLOOR, 2.5,0 | BF_FLOOR_MODE | BF_HEAD_TRACK_TUROK),
	BOSS_MODE_INFO(NULL,								Mantis_Code_FloorMovement,		AI_ANIM_BOSS_MANTIS_TURN_FLOOR_LEFT90, 1.5,0 | BF_FLOOR_MODE | BF_HEAD_TRACK_TUROK),
	BOSS_MODE_INFO(NULL,								Mantis_Code_FloorMovement,		AI_ANIM_BOSS_MANTIS_TURN_FLOOR_RIGHT90, 1.5,0 | BF_FLOOR_MODE | BF_HEAD_TRACK_TUROK),

	BOSS_MODE_INFO(Mantis_Setup_JUMP_FLOOR_A,	Mantis_Code_JUMP_FLOOR_A,		AI_ANIM_BOSS_MANTIS_JUMP_FLOOR_A,1, BF_FLYING_MODE | BF_FLOOR_MODE | BF_HEAD_TRACK_TUROK),
	BOSS_MODE_INFO(Mantis_Setup_JUMP_FLOOR_B,	Mantis_Code_JUMP_FLOOR_B,		AI_ANIM_BOSS_MANTIS_JUMP_FLOOR_B,1, BF_FLYING_MODE | BF_HEAD_TRACK_TUROK),
	BOSS_MODE_INFO(Mantis_Setup_JUMP_FLOOR_C,	Mantis_Code_JUMP_FLOOR_C,		AI_ANIM_BOSS_MANTIS_JUMP_FLOOR_C,1,0 | BF_HEAD_TRACK_TUROK),

	// On the floor attacks
	BOSS_MODE_INFO(NULL,								Mantis_Code_FloorAttack,	 	AI_ANIM_BOSS_MANTIS_PHASE_FLOOR,1, BF_ATTACK_MODE | BF_FLOOR_MODE | BF_HEAD_TRACK_TUROK),
	BOSS_MODE_INFO(NULL,								Mantis_Code_FloorAttack,		AI_ANIM_BOSS_MANTIS_BLAST_FLOOR,1, BF_ATTACK_MODE | BF_FLOOR_MODE | BF_HEAD_TRACK_TUROK),
	BOSS_MODE_INFO(NULL,								Mantis_Code_FloorAttack,		AI_ANIM_BOSS_MANTIS_SPIT_FLOOR,1, BF_ATTACK_MODE | BF_FLOOR_MODE | BF_HEAD_TRACK_TUROK),
	BOSS_MODE_INFO(NULL,								Mantis_Code_FloorAttack,		AI_ANIM_BOSS_MANTIS_SWIPE_FLOOR,1, BF_ATTACK_MODE | BF_FLOOR_MODE | BF_HEAD_TRACK_TUROK | BF_PUNCH_IMPACT_MODE),
	BOSS_MODE_INFO(NULL,								Mantis_Code_FloorAttack,		AI_ANIM_BOSS_MANTIS_FORWARD_SWIPE_FLOOR,1, BF_ATTACK_MODE | BF_FLOOR_MODE | BF_HEAD_TRACK_TUROK | BF_PUNCH_IMPACT_MODE),

	// Specials
	BOSS_MODE_INFO(NULL,								Mantis_Code_RAGE_FLOOR,			AI_ANIM_BOSS_MANTIS_IDLE_FLOOR,1, BF_HEAD_TRACK_TUROK | BF_FLOOR_MODE | BF_CLEAR_VELS),
	BOSS_MODE_INFO(NULL,								Mantis_Code_RAGE_FLOOR,			AI_ANIM_BOSS_MANTIS_RAGE_FLOOR,1,0 | BF_FLOOR_MODE | BF_CLEAR_VELS),

	// Ceiling modes
	BOSS_MODE_INFO(NULL,								Mantis_Code_JUMP_FLOOR_TO_CEILING,	AI_ANIM_BOSS_MANTIS_JUMP_FLOOR_TO_CEILING_A,1, BF_CEILING_MODE | BF_JUMPING_MODE),

	BOSS_MODE_INFO(Mantis_Code_CeilingMovement,	Mantis_Code_CeilingMovement,	AI_ANIM_BOSS_MANTIS_IDLE_CEILING,1, BF_CEILING_MODE | BF_HEAD_TRACK_TUROK),
	BOSS_MODE_INFO(NULL,								Mantis_Code_CeilingAttack,		AI_ANIM_BOSS_MANTIS_RAIN_CEILING,1, BF_CEILING_MODE | BF_ATTACK_MODE | BF_HEAD_TRACK_TUROK),
	BOSS_MODE_INFO(NULL,								Mantis_Code_CeilingAttack,		AI_ANIM_BOSS_MANTIS_SPIT_CEILING,1, BF_CEILING_MODE | BF_ATTACK_MODE | BF_HEAD_TRACK_TUROK),

	BOSS_MODE_INFO(NULL,								Mantis_Code_CeilingMovement,	AI_ANIM_BOSS_MANTIS_WALK_CEILING, 1.0, BF_CEILING_MODE | BF_HEAD_TRACK_TUROK),
	BOSS_MODE_INFO(NULL,								Mantis_Code_CeilingMovement, 	AI_ANIM_BOSS_MANTIS_TURN_CEILING_RIGHT90, 1.0, BF_CEILING_MODE | BF_HEAD_TRACK_TUROK),
	BOSS_MODE_INFO(NULL,								Mantis_Code_CeilingMovement, 	AI_ANIM_BOSS_MANTIS_TURN_CEILING_LEFT90, 1.0, BF_CEILING_MODE | BF_HEAD_TRACK_TUROK),

	BOSS_MODE_INFO(NULL,								Mantis_Code_JUMP_CEILING_TO_FLOOR,					AI_ANIM_BOSS_MANTIS_JUMP_CEILING_TO_FLOOR,1,0 | BF_HEAD_TRACK_TUROK | BF_KICK_IMPACT_MODE),

	BOSS_MODE_INFO(Mantis_Setup_JUMP_FLOOR_TO_LEFT_WALL_A,	Mantis_Code_JumpToWallA,		AI_ANIM_BOSS_MANTIS_JUMP_FLOOR_TO_LEFT_WALL_A,1.5, BF_FLYING_MODE | BF_JUMPING_MODE),
	BOSS_MODE_INFO(NULL,													Mantis_Code_JumpToWallB,		AI_ANIM_BOSS_MANTIS_JUMP_RIGHT_WALL_TO_LEFT_WALL_B,1.5, BF_LEFT_WALL_MODE | BF_FLYING_MODE | BF_JUMPING_MODE),
	BOSS_MODE_INFO(NULL,													Mantis_Code_JumpToWallC,		AI_ANIM_BOSS_MANTIS_JUMP_RIGHT_WALL_TO_LEFT_WALL_C,1, BF_LEFT_WALL_MODE | BF_FLYING_MODE | BF_JUMPING_MODE),

	BOSS_MODE_INFO(Mantis_Setup_JUMP_RIGHT_WALL_TO_LEFT_WALL_A,	Mantis_Code_JumpToWallA,	AI_ANIM_BOSS_MANTIS_JUMP_RIGHT_WALL_TO_LEFT_WALL_A,1.5, BF_JUMPING_MODE),
	BOSS_MODE_INFO(NULL,														Mantis_Code_JumpToWallB,  	AI_ANIM_BOSS_MANTIS_JUMP_RIGHT_WALL_TO_LEFT_WALL_B,1.5, BF_LEFT_WALL_MODE | BF_FLYING_MODE | BF_JUMPING_MODE),
	BOSS_MODE_INFO(NULL,														Mantis_Code_JumpToWallC,  	AI_ANIM_BOSS_MANTIS_JUMP_RIGHT_WALL_TO_LEFT_WALL_C,1, BF_LEFT_WALL_MODE | BF_FLYING_MODE | BF_JUMPING_MODE),

	BOSS_MODE_INFO(NULL,									Mantis_Code_LeftWallAttack,	AI_ANIM_BOSS_MANTIS_IDLE_LEFT_WALL,					1,	BF_LEFT_WALL_MODE),
	BOSS_MODE_INFO(NULL,									Mantis_Code_JumpWallToFloor,	AI_ANIM_BOSS_MANTIS_JUMP_LEFT_WALL_TO_FLOOR,		1,	BF_LEFT_WALL_MODE | BF_KICK_IMPACT_MODE),
	BOSS_MODE_INFO(NULL,									Mantis_Code_JumpWallToCeiling,AI_ANIM_BOSS_MANTIS_JUMP_LEFT_WALL_TO_CEILING,	1,	BF_LEFT_WALL_MODE | BF_JUMPING_MODE),
	BOSS_MODE_INFO(NULL,									Mantis_Code_LeftWallAttack,  	AI_ANIM_BOSS_MANTIS_SPIT_LEFT_WALL,					1,	BF_LEFT_WALL_MODE | BF_ATTACK_MODE),
	BOSS_MODE_INFO(NULL,									Mantis_Code_LeftWallAttack,	AI_ANIM_BOSS_MANTIS_SPIT_LEFT_WALL2,				1, BF_LEFT_WALL_MODE | BF_ATTACK_MODE),

	BOSS_MODE_INFO(Mantis_Setup_JUMP_FLOOR_TO_RIGHT_WALL_A,	Mantis_Code_JumpToWallA,		AI_ANIM_BOSS_MANTIS_JUMP_FLOOR_TO_RIGHT_WALL_A,		1.5,	0),
	BOSS_MODE_INFO(NULL,													Mantis_Code_JumpToWallB,		AI_ANIM_BOSS_MANTIS_JUMP_LEFT_WALL_TO_RIGHT_WALL_B,1.5, BF_RIGHT_WALL_MODE | BF_FLYING_MODE),
	BOSS_MODE_INFO(NULL,													Mantis_Code_JumpToWallC,		AI_ANIM_BOSS_MANTIS_JUMP_LEFT_WALL_TO_RIGHT_WALL_C,1, BF_RIGHT_WALL_MODE | BF_FLYING_MODE),

	BOSS_MODE_INFO(Mantis_Setup_JUMP_LEFT_WALL_TO_RIGHT_WALL_A,	Mantis_Code_JumpToWallA,		AI_ANIM_BOSS_MANTIS_JUMP_LEFT_WALL_TO_RIGHT_WALL_A,1.5,	BF_JUMPING_MODE),
	BOSS_MODE_INFO(NULL,														Mantis_Code_JumpToWallB,		AI_ANIM_BOSS_MANTIS_JUMP_LEFT_WALL_TO_RIGHT_WALL_B,1.5, BF_RIGHT_WALL_MODE | BF_FLYING_MODE),
	BOSS_MODE_INFO(NULL,														Mantis_Code_JumpToWallC,		AI_ANIM_BOSS_MANTIS_JUMP_LEFT_WALL_TO_RIGHT_WALL_C,1,	BF_RIGHT_WALL_MODE | BF_FLYING_MODE),

	BOSS_MODE_INFO(NULL,									Mantis_Code_RightWallAttack,	AI_ANIM_BOSS_MANTIS_IDLE_RIGHT_WALL,				1, BF_RIGHT_WALL_MODE),
	BOSS_MODE_INFO(NULL,									Mantis_Code_JumpWallToFloor,	AI_ANIM_BOSS_MANTIS_JUMP_RIGHT_WALL_TO_FLOOR,	1, BF_RIGHT_WALL_MODE | BF_KICK_IMPACT_MODE),
	BOSS_MODE_INFO(NULL,									Mantis_Code_JumpWallToCeiling,AI_ANIM_BOSS_MANTIS_JUMP_RIGHT_WALL_TO_CEILING,	1, BF_RIGHT_WALL_MODE | BF_JUMPING_MODE),
	BOSS_MODE_INFO(NULL,									Mantis_Code_RightWallAttack, 	AI_ANIM_BOSS_MANTIS_SPIT_RIGHT_WALL,				1, BF_RIGHT_WALL_MODE | BF_ATTACK_MODE),
	BOSS_MODE_INFO(NULL,									Mantis_Code_RightWallAttack, 	AI_ANIM_BOSS_MANTIS_SPIT_RIGHT_WALL2,				1, BF_RIGHT_WALL_MODE | BF_ATTACK_MODE),

	BOSS_MODE_INFO(Mantis_Setup_ExplodeFloor,		Mantis_Code_ExplodeFloor,		AI_ANIM_BOSS_MANTIS_EXPLODE_FLOOR_BACK,	1,0),
	BOSS_MODE_INFO(Mantis_Setup_ExplodeFloor,		Mantis_Code_ExplodeFloor,		AI_ANIM_BOSS_MANTIS_EXPLODE_FLOOR_FRONT,	1,0),
	BOSS_MODE_INFO(Mantis_Setup_ExplodeFloor,		Mantis_Code_ExplodeFloor,		AI_ANIM_BOSS_MANTIS_EXPLODE_FLOOR_LEFT,	1,0),
	BOSS_MODE_INFO(Mantis_Setup_ExplodeFloor,		Mantis_Code_ExplodeFloor,		AI_ANIM_BOSS_MANTIS_EXPLODE_FLOOR_RIGHT,	1,0),

	BOSS_MODE_INFO(NULL,	Mantis_Code_ExplodeCeiling,	AI_ANIM_BOSS_MANTIS_EXPLODE_CEILING,		1,	BF_CEILING_MODE),
	BOSS_MODE_INFO(NULL,	Mantis_Code_ExplodeLeftWall,	AI_ANIM_BOSS_MANTIS_EXPLODE_LEFT_WALL,		1, BF_LEFT_WALL_MODE),
	BOSS_MODE_INFO(NULL,	Mantis_Code_ExplodeRightWall, AI_ANIM_BOSS_MANTIS_EXPLODE_RIGHT_WALL,	1, BF_RIGHT_WALL_MODE),

	BOSS_MODE_INFO(Mantis_Setup_DEATH,	Mantis_Code_DEATH,	AI_ANIM_BOSS_MANTIS_DEATH_FLOOR,	1,BF_DEATH_MODE | BF_CLEAR_VELS),

	// Cinema modes
	BOSS_MODE_INFO(NULL,						Mantis_Code_CINEMA,	AI_ANIM_BOSS_MANTIS_SWIPE_FLOOR,	1,BF_CLEAR_VELS),
	BOSS_MODE_INFO(NULL,						Mantis_Code_CINEMA,	AI_ANIM_BOSS_MANTIS_FORWARD_SWIPE_FLOOR,	1,0),
	BOSS_MODE_INFO(NULL,						Mantis_Code_CINEMA,	AI_ANIM_BOSS_MANTIS_RAGE_FLOOR,	1,0),
	BOSS_MODE_INFO(NULL,						Mantis_Code_CINEMA,	AI_ANIM_BOSS_MANTIS_SPIT_FLOOR,	1,0),
	BOSS_MODE_INFO(NULL,						Mantis_Code_CINEMA,	AI_ANIM_BOSS_MANTIS_SPIT_FLOOR,	1,0),
	BOSS_MODE_INFO(NULL,						Mantis_Code_CINEMA,	AI_ANIM_BOSS_MANTIS_SPIT_FLOOR,	1,0),

	{NULL}
} ;




/////////////////////////////////////////////////////////////////////////////
//
// Scripts:
//
//		NOTE: WallAttacks scripts are included for every stage
//				since the mantis may jump on a wall to break it
//				if turok is in a different section.
//
/////////////////////////////////////////////////////////////////////////////


// Stage1
INT16	FloorAttackScript_Stage1[]=
{
	MANTIS_SPIT_FLOOR_MODE,	// P
	MANTIS_BLAST_FLOOR_MODE,	// p
	MANTIS_PHASE_FLOOR_MODE,	// P
	MANTIS_SPIT_FLOOR_MODE,
	0
} ;

INT16	FloorCloseAttackScript_Stage1[]=
{
	MANTIS_SIDE_SWIPE_FLOOR_MODE,
	MANTIS_BLAST_FLOOR_MODE,	// p
	MANTIS_FORWARD_SWIPE_FLOOR_MODE,
	MANTIS_BLAST_FLOOR_MODE,	// p
	0
} ;

INT16	LeftWallAttackScript_Stage1[]=
{
	MANTIS_JUMP_LEFT_WALL_TO_FLOOR_MODE,
	0
} ;

INT16	RightWallAttackScript_Stage1[]=
{
	MANTIS_JUMP_RIGHT_WALL_TO_FLOOR_MODE,
	0
} ;

// Stage2
INT16	FloorAttackScript_Stage2[]=
{
	MANTIS_JUMP_FLOOR_TO_LEFT_WALL_A_MODE,
	MANTIS_SPIT_FLOOR_MODE,	// P
	MANTIS_BLAST_FLOOR_MODE,	// p
	MANTIS_PHASE_FLOOR_MODE,	// P

	MANTIS_JUMP_FLOOR_TO_RIGHT_WALL_A_MODE,
	MANTIS_BLAST_FLOOR_MODE,
	MANTIS_SPIT_FLOOR_MODE,
	0
} ;

INT16	FloorCloseAttackScript_Stage2[]=
{
	MANTIS_SIDE_SWIPE_FLOOR_MODE,
	MANTIS_JUMP_FLOOR_TO_LEFT_WALL_A_MODE,
	MANTIS_FORWARD_SWIPE_FLOOR_MODE,
	MANTIS_JUMP_FLOOR_TO_RIGHT_WALL_A_MODE,
	0
} ;

INT16	LeftWallAttackScript_Stage2[]=
{
	MANTIS_SPIT_LEFT_WALL_MODE,
	MANTIS_SPIT_LEFT_WALL_LOOP_MODE,
	MANTIS_JUMP_LEFT_WALL_TO_FLOOR_MODE,

	MANTIS_SPIT_LEFT_WALL_MODE,
	MANTIS_SPIT_LEFT_WALL_LOOP_MODE,
	MANTIS_JUMP_LEFT_WALL_TO_RIGHT_WALL_A_MODE,
	0
} ;

INT16	RightWallAttackScript_Stage2[]=
{
	MANTIS_SPIT_RIGHT_WALL_MODE,
	MANTIS_SPIT_RIGHT_WALL_LOOP_MODE,
	MANTIS_JUMP_RIGHT_WALL_TO_FLOOR_MODE,

	MANTIS_SPIT_RIGHT_WALL_MODE,
	MANTIS_SPIT_RIGHT_WALL_LOOP_MODE,
	MANTIS_JUMP_RIGHT_WALL_TO_LEFT_WALL_A_MODE,
	0
} ;



// Stage3
INT16	FloorAttackScript_Stage3[] =
{
	MANTIS_JUMP_FLOOR_TO_LEFT_WALL_A_MODE,
	MANTIS_PHASE_FLOOR_MODE,
	MANTIS_SPIT_FLOOR_MODE,

	MANTIS_JUMP_FLOOR_TO_RIGHT_WALL_A_MODE,
	MANTIS_BLAST_FLOOR_MODE,
	MANTIS_SPIT_FLOOR_MODE,

	MANTIS_JUMP_FLOOR_TO_CEILING_MODE,
	MANTIS_BLAST_FLOOR_MODE,
	0
} ;

INT16	FloorCloseAttackScript_Stage3[] =
{
	MANTIS_JUMP_FLOOR_TO_LEFT_WALL_A_MODE,
	MANTIS_SIDE_SWIPE_FLOOR_MODE,
	MANTIS_FORWARD_SWIPE_FLOOR_MODE,

	MANTIS_JUMP_FLOOR_TO_RIGHT_WALL_A_MODE,
	MANTIS_SIDE_SWIPE_FLOOR_MODE,
	MANTIS_FORWARD_SWIPE_FLOOR_MODE,

	MANTIS_JUMP_FLOOR_TO_CEILING_MODE,
	MANTIS_FORWARD_SWIPE_FLOOR_MODE,

	0
} ;

INT16	LeftWallAttackScript_Stage3[]=
{
	MANTIS_SPIT_LEFT_WALL_MODE,
	MANTIS_JUMP_LEFT_WALL_TO_FLOOR_MODE,

	MANTIS_SPIT_LEFT_WALL_LOOP_MODE,
	MANTIS_JUMP_LEFT_WALL_TO_RIGHT_WALL_A_MODE,

	MANTIS_SPIT_LEFT_WALL_MODE,
	MANTIS_JUMP_LEFT_WALL_TO_CEILING_MODE,


	MANTIS_SPIT_LEFT_WALL_LOOP_MODE,
	MANTIS_JUMP_LEFT_WALL_TO_FLOOR_MODE,

	MANTIS_SPIT_LEFT_WALL_MODE,
	MANTIS_JUMP_LEFT_WALL_TO_RIGHT_WALL_A_MODE,

	MANTIS_SPIT_LEFT_WALL_LOOP_MODE,
	MANTIS_JUMP_LEFT_WALL_TO_CEILING_MODE,

	0
} ;

INT16	RightWallAttackScript_Stage3[]=
{
	MANTIS_SPIT_RIGHT_WALL_MODE,
	MANTIS_JUMP_RIGHT_WALL_TO_FLOOR_MODE,

	MANTIS_SPIT_RIGHT_WALL_LOOP_MODE,
	MANTIS_JUMP_RIGHT_WALL_TO_LEFT_WALL_A_MODE,

	MANTIS_SPIT_RIGHT_WALL_MODE,
	MANTIS_JUMP_RIGHT_WALL_TO_CEILING_MODE,


	MANTIS_SPIT_RIGHT_WALL_LOOP_MODE,
	MANTIS_JUMP_RIGHT_WALL_TO_FLOOR_MODE,

	MANTIS_SPIT_RIGHT_WALL_MODE,
	MANTIS_JUMP_RIGHT_WALL_TO_LEFT_WALL_A_MODE,

	MANTIS_SPIT_RIGHT_WALL_LOOP_MODE,
	MANTIS_JUMP_RIGHT_WALL_TO_CEILING_MODE,

	0
} ;

INT16	CeilingAttackScript_Stage3[]=
{
	MANTIS_RAIN_CEILING_MODE,
	MANTIS_JUMP_CEILING_TO_FLOOR_MODE,
	MANTIS_SPIT_CEILING_MODE,
	MANTIS_JUMP_CEILING_TO_FLOOR_MODE,
	0
} ;


INT16	CeilingAttackScript_Stage4[]=
{
	MANTIS_RAIN_CEILING_MODE,
	MANTIS_RAIN_CEILING_MODE,
	MANTIS_JUMP_CEILING_TO_FLOOR_MODE,
	MANTIS_SPIT_CEILING_MODE,
	MANTIS_SPIT_CEILING_MODE,
	MANTIS_JUMP_CEILING_TO_FLOOR_MODE,
	0
} ;




/////////////////////////////////////////////////////////////////////////////
//
// Special fx lights
//
/////////////////////////////////////////////////////////////////////////////

DEFINE_SPECIALFX_SCRIPT(MantisFxGlowYellow)
	SPECIALFX_SCRIPT_FADE_COLOR(255,255,10, MAX_COL/2, SECONDS_TO_FRAMES(0.40))
	SPECIALFX_SCRIPT_FADE_COLOR(255,255,10, MIN_COL, SECONDS_TO_FRAMES(0.40))
END_SPECIALFX_SCRIPT

DEFINE_SPECIALFX_SCRIPT(MantisFxGlowGreen)
	SPECIALFX_SCRIPT_FADE_COLOR(50,255,50, MAX_COL/2, SECONDS_TO_FRAMES(0.40))
	SPECIALFX_SCRIPT_FADE_COLOR(50,255,50, MIN_COL, SECONDS_TO_FRAMES(0.40))
END_SPECIALFX_SCRIPT

DEFINE_SPECIALFX_SCRIPT(MantisFxGlowBlue)
	SPECIALFX_SCRIPT_FADE_COLOR(0,128,255, MAX_COL/2, SECONDS_TO_FRAMES(0.40))
	SPECIALFX_SCRIPT_FADE_COLOR(0,128,255, MIN_COL, SECONDS_TO_FRAMES(0.40))
END_SPECIALFX_SCRIPT

DEFINE_SPECIALFX_SCRIPT(MantisFxGlowPurple)
	SPECIALFX_SCRIPT_FADE_COLOR(255,0,255, MAX_COL/2, SECONDS_TO_FRAMES(0.40))
	SPECIALFX_SCRIPT_FADE_COLOR(255,0,255, MIN_COL, SECONDS_TO_FRAMES(0.40))
END_SPECIALFX_SCRIPT

DEFINE_SPECIALFX_SCRIPT(MantisFxGlowRed)
	SPECIALFX_SCRIPT_FADE_COLOR(255,0,0, MAX_COL/2, SECONDS_TO_FRAMES(0.40))
	SPECIALFX_SCRIPT_FADE_COLOR(255,0,0, MIN_COL, SECONDS_TO_FRAMES(0.40))
END_SPECIALFX_SCRIPT

// Fade to white and back down quickly
DEFINE_SPECIALFX_SCRIPT(MantisFxHit)
	SPECIALFX_SCRIPT_SET_COLOR(255,255,255, MAX_COL/4)
	SPECIALFX_SCRIPT_FADE_COLOR(255,255,255, MIN_COL, 0.1)
END_SPECIALFX_SCRIPT





/////////////////////////////////////////////////////////////////////////////
//
// The big stage table
//
/////////////////////////////////////////////////////////////////////////////
CMantisStageInfo MantisStageInfo[] =
{
	//	STAGE 1
	{75,							// Health limit for this stage
	 1.5, 						// Anim speed
	 2.0 * SCALING_FACTOR,	// Fly speed
	 SECONDS_TO_FRAMES(4),	// Max walk time
	 196,							// Fly skips
	 5,							// Hits before explosion
	 MantisFxGlowYellow,		// Attack glow light

	 FloorAttackScript_Stage1,			// Floor attack script
	 FloorCloseAttackScript_Stage1,	// Floor close attack script
	 NULL,									// Ceiling attack script
	 LeftWallAttackScript_Stage1,		// LeftWall attack script
	 RightWallAttackScript_Stage1, 	// RightWall attack script
	},

	//	STAGE 2
	{50,							// Health limit for this stage
	 1.75,						// Anim speed
	 2.1 * SCALING_FACTOR,	// Fly speed
	 SECONDS_TO_FRAMES(3.5),	// Max walk time
	 48,							// Fly skips
	 4,							// Hits before explosion
	 MantisFxGlowGreen,		// Attack glow light

	 FloorAttackScript_Stage2,			// Floor attack script
	 FloorCloseAttackScript_Stage2,	// Floor close attack script
	 NULL,									// Ceiling attack script
	 LeftWallAttackScript_Stage2,		// LeftWall attack script
	 RightWallAttackScript_Stage2, 	// RightWall attack script
	},

	//	STAGE 3
	{20,							// Health limit for this stage
	 2.0,							// Anim speed
	 2.2 * SCALING_FACTOR,	// Fly speed
	 SECONDS_TO_FRAMES(2.5),// Max walk time
	 24,							// Fly skips
	 3,							// Hits before explosion
	 MantisFxGlowPurple,		// Attack glow light

	 FloorAttackScript_Stage3,			// Floor attack script
	 FloorCloseAttackScript_Stage3,	// Floor close attack script
	 CeilingAttackScript_Stage3,		// Ceiling attack script
	 LeftWallAttackScript_Stage3,		// LeftWall attack script
	 RightWallAttackScript_Stage3, 	// RightWall attack script
	},

	//	STAGE 4
	{0,							// Health limit for this stage
	 2.25,					  	// Anim speed
	 2.3 * SCALING_FACTOR,	// Fly speed
	 SECONDS_TO_FRAMES(0.5),// Max walk time
	 24,							// Fly skips
	 2,							// Hits before explosion
	 MantisFxGlowRed,			// Attack glow light

	 FloorAttackScript_Stage3,			// Floor attack script
	 FloorCloseAttackScript_Stage3,	// Floor close attack script
	 CeilingAttackScript_Stage4,		// Ceiling attack script
	 LeftWallAttackScript_Stage3,		// LeftWall attack script
	 RightWallAttackScript_Stage3, 	// RightWall attack script
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
*	Function:		Mantis_Initialise()
*
******************************************************************************
*
*	Description:	Prepares mantis boss
*
*	Inputs:			*pThis	-	Ptr to object instance
*
*	Outputs:			CBoss *	-	Ptr to boss vars
*
*****************************************************************************/
void InitializeMantis(CGameObjectInstance *pThis, CMantis *pMantis)
{
	// Initialise fx
	CSpecialFx__Construct(&pMantis->m_SpecialFx, pThis) ;
	CSpecialFx__Construct(&pMantis->m_FlashSpecialFx, pThis) ;
}


CBoss *Mantis_Initialise(CGameObjectInstance *pThis)
{
	CMantis *pMantis = &MantisBoss ;

	// Setup defaults
	CBoss__Construct(&pMantis->m_Boss) ;

	// Setup boss vars
	pMantis->m_Boss.m_pInitializeFunction = (void *)InitializeMantis ;
	pMantis->m_Boss.m_pUpdateFunction = (void *)Mantis_Update ;
	pMantis->m_Boss.m_pModeTable = MantisModeTable ;

	// Reset head tracking
	pMantis->m_Boss.m_Rot1 = 0 ;
	pMantis->m_Boss.m_Rot2 = 0 ;

	// Setup mantis vars
	pMantis->m_pStageInfo = MantisStageInfo ;
//	pMantis->m_pStageInfo = &MantisStageInfo[3] ;
	pMantis->m_FlySpitTotal = 0 ;
	pMantis->m_Boss.m_Stage = 0 ;
	Mantis_SetupStageInfo(pThis, pMantis, pMantis->m_pStageInfo) ;

	// Put into start mode

#if DEBUG_MANTIS_HEAD_TRACK
	pMantis->m_Boss.m_Mode = MANTIS_IDLE_FLOOR_MODE ;
#else
	pMantis->m_Boss.m_Mode = MANTIS_SLEEPING_FLOOR_MODE ;
#endif

	pMantis->m_Boss.m_ModeAnimSpeed = 0 ;
	pMantis->m_Boss.m_Action = MANTIS_ATTACK_ACTION ;
	pMantis->m_Boss.m_ActionTime = 0 ;

	pMantis->m_WalkTime = 0 ;

	// Setup light stuff
	pMantis->m_FlinchGlow = FALSE ;
	pMantis->m_Boss.m_pPreDrawFunction = (void *)Mantis_PreDraw ;
	pMantis->m_Boss.m_pPostDrawFunction = (void *)Mantis_PostDraw ;

#ifndef MAKE_CART
	pMantis->m_Boss.m_pDisplayModeFunction = (void *)Mantis_DisplayMode ;
#endif

#if DEBUG_MANTIS
	rmonPrintf("\r\n\r\n\r\n\r\nMANTIS HERE WE GO!!\r\n\r\n") ;
#endif

	// Setup misc
	CCollisionInfo__SetMantisCollisionDefaults(&pMantis->m_Boss.m_CollisionInfo) ;
	BOSS_Health(pThis) = MANTIS_HEALTH ;
	pMantis->m_ExplosionHits = 0 ;
	pMantis->m_AcidHurtTurok = FALSE ;

	// Initialise wall vars
	pMantis->m_WallBroken[0] = FALSE ;
	pMantis->m_WallBroken[1] = FALSE ;
	pMantis->m_WallBroken[2] = FALSE ;
	pMantis->m_WallBroken[3] = FALSE ;
	pMantis->m_WallsToBreak = 4 ;

	pMantis->m_WallToAimFor = -1 ;
	pMantis->m_GotoFloor = FALSE ;
	pMantis->m_ChoosePosition = FALSE ;
	pMantis->m_WallCollapseTime = 0 ;

	// Make invisivle to begin with
	CGameObjectInstance__SetGone(pThis) ;

	// Return pointer to mantis boss
	return (CBoss *)pMantis ;
}





/*****************************************************************************
*
*	Function:		CCollisionInfo__SetMantisCollisionDefaults()
*
******************************************************************************
*
*	Description:	Sets up default mantis collision info
*
*	Inputs:			*pThis	-	Ptr to collision info structure
*
*	Outputs:			None
*
*****************************************************************************/
void CCollisionInfo__SetMantisCollisionDefaults(CCollisionInfo2 *pThis)
{
	pThis->dwFlags =		COLLISIONFLAG_PHYSICS
							|	COLLISIONFLAG_EXITWATER
//							|	COLLISIONFLAG_USEWALLRADIUS			// TEMP UNTIL IT WORKS PROPERLY!!
							|	COLLISIONFLAG_TRACKGROUND
							|	COLLISIONFLAG_CLIMBUP
							|	COLLISIONFLAG_CLIMBDOWN;

	pThis->InstanceBehavior			= INTERSECT_BEHAVIOR_GREASE;
	pThis->WallBehavior				= INTERSECT_BEHAVIOR_STICK;
	pThis->GroundBehavior			= INTERSECT_BEHAVIOR_SLIDE;
	pThis->GravityAcceleration		= MANTIS_GRAVITY;
	pThis->BounceReturnEnergy		= 0.0;
	pThis->GroundFriction			= 0.0;
	pThis->AirFriction				= 0.0;
	pThis->WaterFriction				= 0.0;
}






/*****************************************************************************
*
*	Function:		Mantis_SetupStageInfo()
*
******************************************************************************
*
*	Description:	Sets up stage vars from stage info table
*
*	Inputs:			*pThis			-	Ptr to game object instance
*						*pMantis	-	Ptr to boss vars
*						*Info				-	Ptr to stage info vars
*
*	Outputs:			None
*
*****************************************************************************/
void Mantis_SetupStageInfo(CGameObjectInstance *pThis, CMantis *pMantis, CMantisStageInfo *Info)
{
//	CInstanceHdr	*pOrigin = (CInstanceHdr *)pThis ;

	pMantis->m_FloorAttackScript.m_pStart =
	pMantis->m_FloorAttackScript.m_pCurrent = Info->pFloorAttackScript ;

	pMantis->m_FloorCloseAttackScript.m_pStart =
	pMantis->m_FloorCloseAttackScript.m_pCurrent = Info->pFloorCloseAttackScript ;

	pMantis->m_CeilingAttackScript.m_pStart =
	pMantis->m_CeilingAttackScript.m_pCurrent = Info->pCeilingAttackScript ;

	pMantis->m_LeftWallAttackScript.m_pStart =
	pMantis->m_LeftWallAttackScript.m_pCurrent = Info->pLeftWallAttackScript ;

	pMantis->m_RightWallAttackScript.m_pStart =
	pMantis->m_RightWallAttackScript.m_pCurrent = Info->pRightWallAttackScript ;

	pMantis->m_Boss.m_Stage++ ;

	// Perform rage?
	if (pMantis->m_Boss.m_Stage > 1)
	{
		pMantis->m_Boss.m_Action = MANTIS_RAGE_ACTION ;
		pMantis->m_Boss.m_ActionTime = 0 ;
		pMantis->m_ChoosePosition = TRUE ;
	}

//	// Start pillars moving out of the ground?
//	// SENDS OUT ID 64 - MAKE SURE WALLS HAVE THIS ID!!!!!
//	if (pMantis->m_Boss.m_Stage == 3)
//		AI_Event_Dispatcher(pOrigin, pOrigin, AI_MEVENT_PRESSUREPLATE, pOrigin->m_pEA->m_dwSpecies, pOrigin->m_vPos, 64);
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
*	Function:		Mantis_Update()
*
******************************************************************************
*
*	Description:	The big top level AI update routine - calls mode code,
*						performs checks etc.
*
*	Inputs:			*pThis		-	Ptr to game object instance
*						*pMantis		-	Ptr to boss vars
*
*	Outputs:			None
*
*****************************************************************************/
void Mantis_Update(CGameObjectInstance *pThis, CMantis *pMantis)
{
//	rmonPrintf("Wall At Left Side:%d    Wall At Right Side:%d\r\n",
//				  Mantis_WallAtLeftSide(pThis, pMantis),
//				  Mantis_WallAtRightSide(pThis, pMantis)) ;

//	rmonPrintf("Wall Collision: %d\r\n", pMantis->m_Boss.m_pCollisionInfo->m_WallCollision) ;


	// Update wall collision radius to stop mantis poking through walls
	if (pMantis->m_Boss.m_ModeFlags & (BF_LEFT_WALL_MODE | BF_RIGHT_WALL_MODE))
	{
		pThis->ah.ih.m_pEA->m_CollisionWallRadius	= MANTIS_ON_WALL_COLLISION_RADIUS ;
		pMantis->m_Boss.m_CollisionInfo.WallBehavior = INTERSECT_BEHAVIOR_STICK ;
	}
	else
	{
		pThis->ah.ih.m_pEA->m_CollisionWallRadius	= MANTIS_COLLISION_RADIUS ;
		pMantis->m_Boss.m_CollisionInfo.WallBehavior = INTERSECT_BEHAVIOR_SLIDE ;
	}

	// Hit by explosion?
	Mantis_CheckHitByExplosion(pThis, pMantis) ;

	// No interruptions on blends!
	if (CGameObjectInstance__IsBlending(pThis))
		return ;

	// Check for collapsing walls
	Mantis_CheckForCollapsingWall(pThis, pMantis) ;

	// Update action ai
	Mantis_UpdateAction(pThis, pMantis) ;

	// Get target and distance from it
	Mantis_GetTarget(pThis, pMantis) ;

	// Call update function if there is one
	if ((pMantis->m_Boss.m_Mode >= 0 ) &&
		 (pMantis->m_Boss.m_pModeTable[pMantis->m_Boss.m_Mode].m_pUpdateFunction))
		 (pMantis->m_Boss.m_pModeTable[pMantis->m_Boss.m_Mode].m_pUpdateFunction)(pThis, (CBoss *)pMantis) ;

	// Get rid of x,z tilt
	if (pMantis->m_Boss.m_Mode != MANTIS_JUMP_FLOOR_B_MODE)
	{
		pMantis->m_Boss.m_RotX = ZoomFLOAT(pMantis->m_Boss.m_RotX,
													  0,
													  MANTIS_TILT_X_SPEED*4) ;

		pMantis->m_Boss.m_RotZ = ZoomFLOAT(pMantis->m_Boss.m_RotZ,
													  0,
													  MANTIS_TILT_Z_SPEED*4) ;
	}


	// Check for updating stage
	if (pMantis->m_Boss.m_PercentageHealth < pMantis->m_pStageInfo->HealthLimit)
	{
		Mantis_SetupStageInfo(pThis, pMantis, ++pMantis->m_pStageInfo) ;
#if DEBUG_MANTIS
			rmonPrintf("\r\n....NEXT ATTACK STAGE....\r\n") ;
#endif
	}


	// Is mantis dead?
	Mantis_CheckForDeath(pThis, pMantis) ;


	// Update anim speed
	Mantis_UpdateAnimSpeed(pThis, pMantis) ;


	// Clear acid hurt player flag
	pMantis->m_AcidHurtTurok = FALSE ;
}






/*****************************************************************************
*
*	Function:		Mantis_SetupMode()
*
******************************************************************************
*
*	Description:	Sets up new mantis mode
*
*	Inputs:			*pThis	-	Ptr to game object instance
*						*pMantis	-	Ptr to boss vars
*
*	Outputs:			TRUE if new mode was setup, else FALSE
*
*****************************************************************************/
INT32 Mantis_SetupMode(CGameObjectInstance *pThis, INT32 nNewMode)
{
	BOOL	Result ;

	// Call normal boss setup
	Result = BOSS_SetupMode(pThis, nNewMode) ;

	return Result ;
}


/*****************************************************************************
*
*	Function:		Mantis_CheckForDeath()
*
******************************************************************************
*
*	Description:	Checks to see if the mantis should be put into death mode
*
*	Inputs:			*pThis	-	Ptr to game object instance
*						*pMantis	-	Ptr to boss vars
*
*	Outputs:			None
*
*****************************************************************************/
void Mantis_CheckForDeath(CGameObjectInstance *pThis, CMantis *pMantis)
{
	// Is mantis dead?
	if (BOSS_Health(pThis) <= 0)
	{

		// Makes mantis get to the floor
		pMantis->m_GotoFloor = TRUE ;

		// On floor yet?
		if ((pMantis->m_Boss.m_ModeFlags & BF_FLOOR_MODE) &&
			 (pMantis->m_Boss.m_Mode != MANTIS_DEATH_FLOOR_MODE))
			Mantis_SetupMode(pThis, MANTIS_DEATH_FLOOR_MODE) ;
		else
		// On ceiling?
		if (pMantis->m_Boss.m_ModeFlags & BF_CEILING_MODE)
			Mantis_SetupMode(pThis, MANTIS_JUMP_CEILING_TO_FLOOR_MODE) ;
		else
		// On left wall?
		if (pMantis->m_Boss.m_ModeFlags & BF_LEFT_WALL_MODE)
			Mantis_SetupMode(pThis, MANTIS_JUMP_LEFT_WALL_TO_FLOOR_MODE) ;
		else
		// On right wall?
		if (pMantis->m_Boss.m_ModeFlags & BF_RIGHT_WALL_MODE)
			Mantis_SetupMode(pThis, MANTIS_JUMP_RIGHT_WALL_TO_FLOOR_MODE) ;
		else
		// Jumping/Flying?
		if (pMantis->m_Boss.m_ModeFlags & (BF_JUMPING_MODE | BF_FLYING_MODE))
			Mantis_SetupMode(pThis, MANTIS_JUMP_FLOOR_C_MODE) ;
	}
}



/*****************************************************************************
*
*	Function:		Mantis_UpdateAnimSpeed()
*
******************************************************************************
*
*	Description:	Sets the correct anim speed
*
*	Inputs:			*pThis	-	Ptr to game object instance
*						*pMantis	-	Ptr to boss vars
*
*	Outputs:			None
*
*****************************************************************************/
void Mantis_UpdateAnimSpeed(CGameObjectInstance *pThis, CMantis *pMantis)
{
	// Cinemas (killing Turok) are played at normal speed
	if (CCamera__InCinemaMode(&GetApp()->m_Camera))
		pMantis->m_Boss.m_AnimSpeed = 1 ;
	else
	// Update anim speed
	switch(pMantis->m_Boss.m_Mode)
	{
		case MANTIS_JUMP_FLOOR_A_MODE:
		case MANTIS_JUMP_FLOOR_C_MODE:
//		case MANTIS_JUMP_FLOOR_TO_CEILING_MODE:
//		case MANTIS_JUMP_CEILING_TO_FLOOR_MODE:
		case MANTIS_EXPLODE_FLOOR_BACK_MODE:
		case MANTIS_EXPLODE_FLOOR_FRONT_MODE:
		case MANTIS_EXPLODE_FLOOR_LEFT_MODE:
		case MANTIS_EXPLODE_FLOOR_RIGHT_MODE:
		case MANTIS_EXPLODE_CEILING_MODE:
		case MANTIS_EXPLODE_LEFT_WALL_MODE:
		case MANTIS_EXPLODE_RIGHT_WALL_MODE:
		case MANTIS_DEATH_FLOOR_MODE:
			pMantis->m_Boss.m_AnimSpeed = 1.0 ;
			break ;


		// Jump fast if breaking walls
		case MANTIS_JUMP_FLOOR_TO_RIGHT_WALL_A_MODE:
		case MANTIS_JUMP_FLOOR_TO_RIGHT_WALL_B_MODE:
		case MANTIS_JUMP_FLOOR_TO_RIGHT_WALL_C_MODE:

		case MANTIS_JUMP_FLOOR_TO_LEFT_WALL_A_MODE:
		case MANTIS_JUMP_FLOOR_TO_LEFT_WALL_B_MODE:
		case MANTIS_JUMP_FLOOR_TO_LEFT_WALL_C_MODE:
			if (pMantis->m_Boss.m_Action == MANTIS_BREAK_WALL_ACTION)
			{
				pMantis->m_Boss.m_AnimSpeed = 2 ;
				return ;
			}

		default:
			pMantis->m_Boss.m_AnimSpeed = pMantis->m_pStageInfo->AnimSpeed ;
	}

}




/*****************************************************************************
*
*	Function:		Mantis_PreDraw()
*
******************************************************************************
*
*	Description:	Sets up special fx before draw
*
*	Inputs:			*pThis			-	Ptr to game object instance
*						*pMantis	-	Ptr to boss vars
*
*	Outputs:			None
*
*****************************************************************************/
void Mantis_PreDraw(CGameObjectInstance *pThis, CMantis *pMantis, Gfx **ppDLP)
{
	// Do flinch flash?
	if ((BOSS_Health(pThis) > 0) && (!pMantis->m_FlinchGlow) &&
		 (!AI_GetDyn(pThis)->m_Invincible) &&
		 ((AI_GetDyn(pThis)->m_dwStatusFlags & AI_EV_HIT) ||
		  (AI_GetDyn(pThis)->m_dwStatusFlags & AI_EV_EXPLOSION)))
	{
		pMantis->m_FlinchGlow = TRUE ;
		CSpecialFx__StartScript(&pMantis->m_FlashSpecialFx, MantisFxHit) ;
	}
	else
		pMantis->m_FlinchGlow = FALSE ;


	AI_GetDyn(pThis)->m_dwStatusFlags &= ~AI_EV_HIT ;



	// Read special fx scripts
	CSpecialFx__Update(&pMantis->m_SpecialFx) ;
	CSpecialFx__Update(&pMantis->m_FlashSpecialFx) ;

	// Setup fx draw vars ready for draw
	CSpecialFx__SetDrawFxVars(&pMantis->m_SpecialFx) ;
	CSpecialFx__AddToDrawFxVars(&pMantis->m_FlashSpecialFx) ;
	PrepareDrawFxVars() ;
}


/*****************************************************************************
*
*	Function:		Mantis_PostDraw()
*
******************************************************************************
*
*	Description:	Resets draw mode so special fx don't affect next objects
*
*	Inputs:			*pThis	-	Ptr to game object instance
*						*pMantis	-	Ptr to boss vars
*
*	Outputs:			None
*
*****************************************************************************/
void Mantis_PostDraw(CGameObjectInstance *pThis, CMantis *pMantis, Gfx **ppDLP)
{
	gSPSetLights1((*ppDLP)++, thelight);
}



/*****************************************************************************
*
*	Function:		CGameObjectInstance__MantisArenaSection()
*
******************************************************************************
*
*	Description:	Calculates arena section (0-3). Used for wall test
*
*	Inputs:			*pThis	-	Ptr to game object instance
*
*	Outputs:			Arena section
*
*****************************************************************************/
INT32 CGameObjectInstance__MantisArenaSection(CGameObjectInstance *pThis)
{
	FLOAT	Angle = CVector3__XZAngle(&pThis->ah.ih.m_vPos) ;
	if ((Angle >= ANGLE_DTOR(-135)) && (Angle < ANGLE_DTOR(-45)))
		return 1 ;
	else
	if ((Angle >= ANGLE_DTOR(-45)) && (Angle < ANGLE_DTOR(45)))
		return 2 ;
	else
	if ((Angle >= ANGLE_DTOR(45)) && (Angle < ANGLE_DTOR(135)))
		return 3 ;
	else
		return 0 ;
}

FLOAT CGameObjectInstance__XZDistance(CGameObjectInstance *pThis, CVector3 *vPos)
{
	FLOAT	XDist, ZDist, Dist ;
	XDist = pThis->ah.ih.m_vPos.x - vPos->x ;
	ZDist = pThis->ah.ih.m_vPos.z - vPos->z ;
	Dist = (XDist * XDist) + (ZDist * ZDist) ;
	if (Dist)
		Dist = sqrt(Dist) ;
	return Dist ;
}

#define WALL_PT	(90*SCALING_FACTOR)

CVector3	WallPositions[] =
{
	{-WALL_PT, 0, WALL_PT},		// Wall1
	{WALL_PT, 0, WALL_PT},		// Wall2
	{WALL_PT, 0, -WALL_PT},		// Wall3
	{-WALL_PT, 0, -WALL_PT}, 	// Wall4

	{-WALL_PT, 0, WALL_PT}		// Wall1 again for closeness check!
} ;


#define PT1	(100*SCALING_FACTOR)
#define PT2	(60*SCALING_FACTOR)

CVector3	WallJumpPositions[] =
{
	{-PT1,	0, PT2},		// Wall1
	{-PT2,	0, PT1},

	{PT2,		0, PT1},		// Wall2
	{PT1,		0, PT2},

	{PT1,		0, -PT2},	// Wall3
	{PT2,		0, -PT1},

	{-PT2,	0, -PT1},	// Wall4
	{-PT1,	0, -PT2}
} ;



INT32 Mantis_ChooseWallJumpPos(CGameObjectInstance *pThis, CMantis *pMantis)
{
	INT32	Chosen = -1 ;

	// Choose jump position for wall
	if (pMantis->m_WallToAimFor != -1)
	{
		switch(pMantis->m_WallToAimFor)
		{
			case 0:
				if (CGameObjectInstance__MantisArenaSection(pThis) == 0)
					Chosen = 1 ;
				else
				if (CGameObjectInstance__MantisArenaSection(pThis) == 3)
					Chosen = 0 ;
				break ;

			case 1:
				if (CGameObjectInstance__MantisArenaSection(pThis) == 0)
					Chosen = 2 ;
				else
				if (CGameObjectInstance__MantisArenaSection(pThis) == 1)
					Chosen = 3 ;
				break ;

			case 2:
				if (CGameObjectInstance__MantisArenaSection(pThis) == 1)
					Chosen = 4 ;
				else
				if (CGameObjectInstance__MantisArenaSection(pThis) == 2)
					Chosen = 5 ;
				break ;

			case 3:
				if (CGameObjectInstance__MantisArenaSection(pThis) == 2)
					Chosen = 6 ;
				else
				if (CGameObjectInstance__MantisArenaSection(pThis) == 3)
					Chosen = 7 ;
				break ;
		}

		// Was jump position selected?
		if (Chosen != -1)
			pMantis->m_WallToAimForJumpPos = Chosen ;
		else
		{
			Chosen = pMantis->m_WallToAimFor ;
			pMantis->m_WallToAimForJumpPos = (Chosen * 2) +
													BOSS_GetClosestPositionIndex(pThis, &WallJumpPositions[Chosen * 2], 2) ;
		}
	}

	return pMantis->m_WallToAimForJumpPos ;
}





INT32 Mantis_ClosestWall(CGameObjectInstance *pThis, CMantis *pMantis)
{
	INT32		i ;
	INT32		Chosen ;
	FLOAT		ClosestDistSqr, XDist, ZDist, DistSqr ;

	Chosen = -1 ;

	// Interrupted?
	if ((pMantis->m_WallToAimFor != -1) && (!pMantis->m_WallBroken[pMantis->m_WallToAimFor]))
		Chosen = pMantis->m_WallToAimFor ;
	else
	// Never chosen before?
	if (pMantis->m_WallToAimFor == -1)
		Chosen = BOSS_GetClosestPositionIndex(pThis, &WallPositions[0], 4) ;
	else
	// Choose the logical next wall!
	switch(pMantis->m_WallToAimFor)
	{
		case 0:
			if (pMantis->m_WallToAimForJumpPos == 0)
				Chosen = 1 ;
			else
			if (pMantis->m_WallToAimForJumpPos == 1)
				Chosen = 3 ;
			break ;

		case 1:
			if (pMantis->m_WallToAimForJumpPos == 2)
				Chosen = 2 ;
			else
			if (pMantis->m_WallToAimForJumpPos == 3)
				Chosen = 0 ;
			break ;

		case 2:
			if (pMantis->m_WallToAimForJumpPos == 4)
				Chosen = 3 ;
			else
			if (pMantis->m_WallToAimForJumpPos == 5)
				Chosen = 1 ;
			break ;

		case 3:
			if (pMantis->m_WallToAimForJumpPos == 6)
				Chosen = 0 ;
			else
			if (pMantis->m_WallToAimForJumpPos == 7)
				Chosen = 2 ;
			break ;
	}

	// Select closest if wall has already been knocked down!
	if ((Chosen == -1) || (pMantis->m_WallBroken[Chosen]))
	{
		// Set dist to max
		Chosen = -1 ;
		ClosestDistSqr = 0 ;	// Fix warning!

		// Check all walls
		for (i = 0 ; i < 4 ; i++)
		{
			// This wall broken?
			if (!pMantis->m_WallBroken[i])
			{
				// Get dist squared from mantis
				XDist = WallPositions[i].x - BOSS_GetPos(pThis).x ;
				ZDist = WallPositions[i].z - BOSS_GetPos(pThis).z ;
				DistSqr = (XDist * XDist) + (ZDist * ZDist) ;

				// Is this the closest wall so far?
				if ((Chosen == -1) || (DistSqr < ClosestDistSqr))
				{
					Chosen = i ;
					ClosestDistSqr = DistSqr ;
				}
			}
		}
	}

	// Choose jump position for wall
 	pMantis->m_WallToAimFor = Chosen ;
	if (Chosen != -1)
	{
		Mantis_ChooseWallJumpPos(pThis, pMantis) ;

#if DEBUG_MANTIS
		rmonPrintf("Chosen Wall %d, Jump Pos %d\r\n",
					  pMantis->m_WallToAimFor,
					  pMantis->m_WallToAimForJumpPos) ;
#endif

	}

	return pMantis->m_WallToAimFor ;
}



INT32 Mantis_DividingWall(CGameObjectInstance *pThis, CMantis *pMantis)
{
	CGameObjectInstance *pTurok = CEngineApp__GetPlayer(GetApp()) ;
	INT32	TurokSection = CGameObjectInstance__MantisArenaSection(pTurok) ;
	INT32	MantisSection = CGameObjectInstance__MantisArenaSection(pThis) ;
	INT32 Wall ;
	CVector3	vCentre ;

	vCentre.x = 0 ;
	vCentre.z = 0 ;

	// Both out of middle of arena?
	Wall = -1 ;
	if ((CGameObjectInstance__XZDistance(pThis, &vCentre) > (60*SCALING_FACTOR)) &&
		 (CGameObjectInstance__XZDistance(pTurok, &vCentre) > (60*SCALING_FACTOR)))
	{
		// Go through wall 1?
		if ((pMantis->m_WallBroken[0] == FALSE) &&
			 (((MantisSection == 3) && (TurokSection == 0)) ||
			  ((MantisSection == 0) && (TurokSection == 3))))
			Wall = 0 ;
		else
		// Go through wall 2?
		if ((pMantis->m_WallBroken[1] == FALSE) &&
			 (((MantisSection == 0) && (TurokSection == 1)) ||
			  ((MantisSection == 1) && (TurokSection == 0))))
			Wall = 1 ;
		else
		// Go through wall 3?
		if ((pMantis->m_WallBroken[2] == FALSE) &&
			 (((MantisSection == 1) && (TurokSection == 2)) ||
			  ((MantisSection == 2) && (TurokSection == 1))))
			Wall = 2 ;
		else
		// Go through wall 4?
		if ((pMantis->m_WallBroken[3] == FALSE) &&
			 (((MantisSection == 2) && (TurokSection == 3)) ||
			  ((MantisSection == 3) && (TurokSection == 2))))
			Wall = 3 ;
	}

	// Choose wall
	pMantis->m_WallToAimFor = Wall ;

	// Choose jump position
	if (Wall != -1)
		Mantis_ChooseWallJumpPos(pThis, pMantis) ;

	return Wall ;
}



void Mantis_UpdateAction(CGameObjectInstance *pThis, CMantis *pMantis)
{
#if DEBUG_MANTIS
	INT32	Action = pMantis->m_Boss.m_Action ;
#endif

	// Boss dead?
	if (!BOSS_Health(pThis))
	{
		pMantis->m_Boss.m_Action = MANTIS_ATTACK_ACTION ;
		pMantis->m_GotoFloor = TRUE ;
	}
	else
	// Perform ai logic
	switch(pMantis->m_Boss.m_Action)
	{
		case MANTIS_ATTACK_ACTION:

			// Setup defaults
			pMantis->m_GotoFloor = FALSE ;

			// Goto victory action?
			if (!CTurokMovement.Active)
			{
				pMantis->m_Boss.m_Action = MANTIS_RAGE_ACTION ;
				pMantis->m_Boss.m_ActionTime = 0 ;
				pMantis->m_ChoosePosition = TRUE ;
			}
			else
			// Break all walls?
			if ((pMantis->m_Boss.m_Stage > 1) && (Mantis_ClosestWall(pThis, pMantis) != -1))
			{
				pMantis->m_Boss.m_Action = MANTIS_BREAK_WALL_ACTION ;
#if DEBUG_MANTIS
				rmonPrintf("\r\nGoing to break wall %d\r\n\r\n", pMantis->m_WallToAimFor) ;
#endif
			}
			else
			// Break dividing wall?
			if (Mantis_DividingWall(pThis, pMantis) != -1)
			{
				pMantis->m_Boss.m_Action = MANTIS_BREAK_WALL_ACTION ;

#if DEBUG_MANTIS
				rmonPrintf("\r\nGoing to break dividing wall %d\r\n\r\n", pMantis->m_WallToAimFor) ;
#endif
			}
			break ;

		case MANTIS_BREAK_WALL_ACTION:

			// Setup defaults
			pMantis->m_GotoFloor = TRUE ;

			// Goto victory action?
			if (!CTurokMovement.Active)
			{
				pMantis->m_Boss.m_Action = MANTIS_RAGE_ACTION ;
				pMantis->m_Boss.m_ActionTime = 0 ;
				pMantis->m_ChoosePosition = TRUE ;
			}
			else
			// Wall broken yet? - If so goto attack action.
			if (pMantis->m_WallBroken[pMantis->m_WallToAimFor])
				pMantis->m_Boss.m_Action = MANTIS_ATTACK_ACTION ;

			break ;

		case MANTIS_RAGE_ACTION:

			// Setup defaults - RAGE_FLOOR code takes care of getting mantis out of action
			pMantis->m_WallToAimFor = -1 ;
			pMantis->m_GotoFloor = TRUE ;
			break ;
	}

#if DEBUG_MANTIS
	if (Action != pMantis->m_Boss.m_Action)
	{
		switch(pMantis->m_Boss.m_Action)
		{
			case MANTIS_ATTACK_ACTION:
				rmonPrintf("\r\nMANTIS_ATTACK_ACTION\r\n\r\n") ;
				break ;
			case MANTIS_BREAK_WALL_ACTION:
				rmonPrintf("\r\nMANTIS_BREAK_WALL_ACTION\r\n\r\n") ;
				break ;
			case MANTIS_RAGE_ACTION:
				rmonPrintf("\r\nMANTIS_RAGE_ACTION\r\n\r\n") ;
				break ;
		}
	}
#endif
}



/*****************************************************************************
*
*	Function:		Mantis_GetTarget()
*
******************************************************************************
*
*	Description:	Calculates target to aim for, distance from target, and the
*						correct angle to head.
*
*	Inputs:			*pThis			-	Ptr to game object instance
*						*pMantis	-	Ptr to boss vars
*
*	Outputs:			None
*
*****************************************************************************/
void Mantis_GetTarget(CGameObjectInstance *pThis, CMantis *pMantis)
{
	CGameObjectInstance *pTurok = CEngineApp__GetPlayer(GetApp()) ;

	FLOAT	rot ;
	CVector3 vTarget ;
	INT32 MantisSection ;

	rot = pTurok->m_RotY ;

	// Perform ai logic
	switch(pMantis->m_Boss.m_Action)
	{
		case MANTIS_ATTACK_ACTION:

			pMantis->m_Boss.m_AtTargetDist = MANTIS_ATTACK_AT_TARGET_DIST ;

			vTarget.x = pTurok->ah.ih.m_vPos.x - (MANTIS_TARGET_DIST * sin(rot)) ;
			vTarget.y = BOSS_GetPos(pThis).y ;
			vTarget.z = pTurok->ah.ih.m_vPos.z - (MANTIS_TARGET_DIST * cos(rot)) ;

			pMantis->m_Boss.m_DeltaAngle = AI_GetAvoidanceAngle(pThis, vTarget, pTurok, AVOIDANCE_RADIUS_DISTANCE_FACTOR) ;
			pMantis->m_Boss.m_LookDeltaAngle = pMantis->m_Boss.m_DeltaAngle ;
			break ;

		case MANTIS_BREAK_WALL_ACTION:

			pMantis->m_Boss.m_AtTargetDist = MANTIS_BREAK_WALL_AT_TARGET_DIST ;

			if (pMantis->m_WallToAimFor != -1)
			{
				// Update jump pos to stop "getting stuck bug"
				MantisSection = CGameObjectInstance__MantisArenaSection(pThis) ;
				switch(pMantis->m_WallToAimFor)
				{
					case 0:
						if (MantisSection == 0)
							pMantis->m_WallToAimForJumpPos = 1 ;
						else
						if (MantisSection == 3)
							pMantis->m_WallToAimForJumpPos = 0 ;
						break ;
					case 1:
						if (MantisSection == 0)
							pMantis->m_WallToAimForJumpPos = 2 ;
						else
						if (MantisSection == 1)
							pMantis->m_WallToAimForJumpPos = 3 ;
						break ;
					case 2:
						if (MantisSection == 1)
							pMantis->m_WallToAimForJumpPos = 4 ;
						else
						if (MantisSection == 2)
							pMantis->m_WallToAimForJumpPos = 5 ;
						break ;
					case 3:
						if (MantisSection == 2)
							pMantis->m_WallToAimForJumpPos = 6 ;
						else
						if (MantisSection == 3)
							pMantis->m_WallToAimForJumpPos = 7 ;
						break ;
				}

				// Look towards centre of wall
				vTarget.x = WallPositions[pMantis->m_WallToAimFor].x - BOSS_GetPos(pThis).x ;
				vTarget.y = WallPositions[pMantis->m_WallToAimFor].y - BOSS_GetPos(pThis).y ;
				vTarget.z = WallPositions[pMantis->m_WallToAimFor].z - BOSS_GetPos(pThis).z ;
				rot = CVector3__XZAngle(&vTarget) ;

				// Which direction are jumping to?
				if (pMantis->m_WallToAimForJumpPos & 1)	// From right side
					rot -= ANGLE_DTOR(90) ;
				else
					rot += ANGLE_DTOR(90) ;						// From left side

				pMantis->m_Boss.m_LookDeltaAngle = AngleDiff(pThis->m_RotY, rot) ;

				// Setup target
				vTarget = WallJumpPositions[pMantis->m_WallToAimForJumpPos] ;
				pMantis->m_Boss.m_DeltaAngle = AI_GetAvoidanceAngle(pThis, vTarget, pTurok, AVOIDANCE_RADIUS_DISTANCE_FACTOR) ;
			}
			else
				pMantis->m_Boss.m_DeltaAngle = pMantis->m_Boss.m_LookDeltaAngle = 0 ;

			break ;


		case MANTIS_RAGE_ACTION:

			pMantis->m_Boss.m_AtTargetDist = 0 ;
			vTarget = pThis->ah.ih.m_vPos ;
			pMantis->m_Boss.m_DeltaAngle = AI_GetAvoidanceAngle(pThis, vTarget, pTurok, AVOIDANCE_RADIUS_DISTANCE_FACTOR) ;
			pMantis->m_Boss.m_LookDeltaAngle = AI_GetAvoidanceAngle(pThis, TUROK_GetPos(pTurok), pTurok, AVOIDANCE_RADIUS_DISTANCE_FACTOR) ;
			break ;
	}

	// Calculate distance from target
	pMantis->m_Boss.m_vTarget = vTarget ;
	pMantis->m_Boss.m_DistFromTargetSqr =
	pMantis->m_Boss.m_DistFromTarget = AI_DistanceSquared(pThis, pMantis->m_Boss.m_vTarget) ;
	if (pMantis->m_Boss.m_DistFromTarget)
		pMantis->m_Boss.m_DistFromTarget = sqrt(pMantis->m_Boss.m_DistFromTarget) ;
}





/*****************************************************************************
*
*	Function:		Mantis_FacingTurok()
*
******************************************************************************
*
*	Description:	Pretty self explanitory really!
*
*	Inputs:			*pThis		-	Ptr to game object instance
*						*pMantis		-	Ptr to boss vars
*
*	Outputs:			TRUE if mantis is facing Turok, else FALSE!
*
*****************************************************************************/
BOOL Mantis_FacingTurok(CGameObjectInstance *pThis, CMantis *pMantis)
{
	return (abs(pMantis->m_Boss.m_DeltaAngle) < ANGLE_DTOR(10)) ;
}




/*****************************************************************************
*
*	Function:		Mantis_WallAtSide()
*
******************************************************************************
*
*	Description:	Calculates whether there is a wall to the side of the mantis,
*						on which it can land on
*
*	Inputs:			*pThis		-	Ptr to game object instance
*						*pMantis		-	Ptr to boss vars
*						Dir			-	Direction angle to look (-90 or +90)
*
*	Outputs:			TRUE if there is a wall which could be landed on
*
*****************************************************************************/
BOOL Mantis_WallAtSide(CGameObjectInstance *pThis, CMantis *pMantis, FLOAT Dir)
{
	CVector3	vTarget ;
	CVector3	vTarget2 ;
	FLOAT		Angle ;
	FLOAT		WallAngle1, WallAngle2, WallAngle3 ;

	//	CCollisionInfo *pCollisionInfo = pMantis->m_Boss.m_pCollisionInfo ;
	CCollisionInfo2 *pCollisionInfo = &ci_movetest ;
	Angle = pThis->m_RotY + Dir ;

	// Check min distance - wall can't be too close!
	vTarget.x = BOSS_GetPos(pThis).x - (WALL_AT_SIDE_MIN_LOOK_DIST) * sin(Angle) ;
	vTarget.y = BOSS_GetPos(pThis).y + MANTIS_WALL_HEIGHT ;
	vTarget.z = BOSS_GetPos(pThis).z - (WALL_AT_SIDE_MIN_LOOK_DIST) * cos(Angle) ;

	CAnimInstanceHdr__IsObstructed(&pThis->ah, vTarget, &pCollisionInfo);
	if (pCollisionInfo->pWallCollisionRegion)
		return FALSE ;
	// Check pt 1 - is there a wall inbetween max distance and the mantis?
	vTarget.x = BOSS_GetPos(pThis).x - (WALL_AT_SIDE_MAX_LOOK_DIST) * sin(Angle) ;
	vTarget.y = BOSS_GetPos(pThis).y + MANTIS_WALL_HEIGHT ;
	vTarget.z = BOSS_GetPos(pThis).z - (WALL_AT_SIDE_MAX_LOOK_DIST) * cos(Angle) ;

	CAnimInstanceHdr__IsObstructed(&pThis->ah, vTarget, &pCollisionInfo);
	if (!pCollisionInfo->pWallCollisionRegion)
		return FALSE ;
	WallAngle1 = CGameRegion__GetEdgeAngle(pCollisionInfo->pWallCollisionRegion, pCollisionInfo->nWallCollisionEdge) ;

	// Check pt 2 - look parallel to wall to make sure wall angle next to pt1 is the same
	vTarget2 = vTarget ;
	Angle += ANGLE_DTOR(90) ;
	vTarget2.x += - (MANTIS_HALF_LENGTH) * sin(Angle) ;
	vTarget2.z += - (MANTIS_HALF_LENGTH) * cos(Angle) ;

	CAnimInstanceHdr__IsObstructed(&pThis->ah, vTarget2, &pCollisionInfo);
	if (!pCollisionInfo->pWallCollisionRegion)
		return FALSE ;
	WallAngle2 = CGameRegion__GetEdgeAngle(pCollisionInfo->pWallCollisionRegion, pCollisionInfo->nWallCollisionEdge) ;

	// Check pt 3 - look parallel to wall to make sure wall angle next to pt1 is the same
	vTarget2 = vTarget ;
	Angle += ANGLE_DTOR(180) ;
	vTarget2.x += - (MANTIS_HALF_LENGTH) * sin(Angle) ;
	vTarget2.z += - (MANTIS_HALF_LENGTH) * cos(Angle) ;

	CAnimInstanceHdr__IsObstructed(&pThis->ah, vTarget2, &pCollisionInfo);
	if (!pCollisionInfo->pWallCollisionRegion)
		return FALSE ;
	WallAngle3 = CGameRegion__GetEdgeAngle(pCollisionInfo->pWallCollisionRegion, pCollisionInfo->nWallCollisionEdge) ;

	// Make sure all angles are very close
	if (AngleDist(WallAngle1, WallAngle2) > ANGLE_DTOR(10))
		return FALSE ;
	if (AngleDist(WallAngle1, WallAngle3) > ANGLE_DTOR(10))
		return FALSE ;
	if (AngleDist(WallAngle2, WallAngle3) > ANGLE_DTOR(10))
		return FALSE ;

	// Walls are 90degress off from characters!
	WallAngle1 += (ANGLE_PI/2) ;

	// Use WallAngle or WallAngle+180degrees - whichever is closer to mantis' direction
	if (AngleDist(pThis->m_RotY, WallAngle1 + ANGLE_PI) < AngleDist(pThis->m_RotY, WallAngle1))
		WallAngle1 += ANGLE_PI ;

	NormalizeRotation(&WallAngle1) ;

	// Angle close enough to snap too?
	if (AngleDist(pThis->m_RotY, WallAngle1) < ANGLE_DTOR(90))
		return TRUE ;
	else
		return FALSE ;
}




/*****************************************************************************
*
*	Function:		Mantis_WallAtLeftSide()
*
******************************************************************************
*
*	Description:	Calculates whether there is a landable wall to the left side
*						of the mantis
*
*	Inputs:			*pThis		-	Ptr to game object instance
*						*pMantis		-	Ptr to boss vars
*
*	Outputs:			TRUE if there is a wall which could be landed on
*
*****************************************************************************/
BOOL Mantis_WallAtLeftSide(CGameObjectInstance *pThis, CMantis *pMantis)
{
	return Mantis_WallAtSide(pThis, pMantis, -ANGLE_PI/2) ;
}



/*****************************************************************************
*
*	Function:		Mantis_WallAtRightSide()
*
******************************************************************************
*
*	Description:	Calculates whether there is a landable wall to the right
*						side of the mantis
*
*	Inputs:			*pThis		-	Ptr to game object instance
*						*pMantis		-	Ptr to boss vars
*
*	Outputs:			TRUE if there is a wall which could be landed on
*
*****************************************************************************/
BOOL Mantis_WallAtRightSide(CGameObjectInstance *pThis, CMantis *pMantis)
{
	return Mantis_WallAtSide(pThis, pMantis, ANGLE_PI/2) ;
}



/*****************************************************************************
*
*	Function:		Mantis_NearWall()
*
******************************************************************************
*
*	Description:	Calculates when the mantis is about to get very near to a
*						landable wall
*
*	Inputs:			*pThis		-	Ptr to game object instance
*						*pMantis		-	Ptr to boss vars
*
*	Outputs:			TRUE if mantis is near a landable wall, else FALSE
*
*****************************************************************************/
BOOL Mantis_NearWall(CGameObjectInstance *pThis, CMantis *pMantis)
{
	FLOAT					WallAngle ;
	CCollisionInfo2	*pCollisionInfo ;

	// Record angle of any walls hit
	pCollisionInfo = &pMantis->m_Boss.m_CollisionInfo ;
	if (pCollisionInfo->pWallCollisionRegion)
	{
		// Save all region
		pMantis->m_pWallCollisionRegion = pCollisionInfo->pWallCollisionRegion ;

		// Get wall angle
		WallAngle = CGameRegion__GetEdgeAngle(pCollisionInfo->pWallCollisionRegion, pCollisionInfo->nWallCollisionEdge) ;

		// Walls are 90degress off from characters!
		WallAngle += (ANGLE_PI/2) ;

		// Use WallAngle or WallAngle+180degrees - whichever is closer to mantis' direction
		if (AngleDist(pThis->m_RotY, WallAngle + ANGLE_PI) < AngleDist(pThis->m_RotY, WallAngle))
			WallAngle += ANGLE_PI ;

		NormalizeRotation(&WallAngle) ;

		// Angle close enough to snap too?
		if (AngleDist(pThis->m_RotY, WallAngle) < ANGLE_DTOR(90))
		{
			pMantis->m_WallAngle = WallAngle ;

			// Save wall position
			pMantis->m_vWallPos = pCollisionInfo->vWallCollisionPos ;

			return TRUE ;
		}
	}

	return FALSE ;
}



/*****************************************************************************
*
*	Function:		Mantis_CheckHitByExplosion()
*
******************************************************************************
*
*	Description:	Puts mantis into explosion mode if hit by one
*
*	Inputs:			*pThis		-	Ptr to game object instance
*						*pMantis		-	Ptr to boss vars
*
*	Outputs:			None
*
*****************************************************************************/
void Mantis_CheckHitByExplosion(CGameObjectInstance *pThis, CMantis *pMantis)
{
	// Can't be hit if invincible
	if (AI_GetDyn(pThis)->m_Invincible)
		return ;

	// Hit by an explosion?
	if (AI_GetDyn(pThis)->m_dwStatusFlags & AI_EV_EXPLOSION)
	{
		AI_GetDyn(pThis)->m_dwStatusFlags &= ~AI_EV_EXPLOSION ;

		// Been hit enough?
		if (++pMantis->m_ExplosionHits < pMantis->m_pStageInfo->HitsBeforeExplosion)
			return ;

		// Reset hits
		pMantis->m_ExplosionHits = 0 ;

		// Put into correct mode
		switch(pMantis->m_Boss.m_Mode)
		{
			// Floor mode
			case MANTIS_IDLE_FLOOR_MODE:
			case MANTIS_LOOK_FLOOR_MODE:
			case MANTIS_WALK_FLOOR_MODE:
			case MANTIS_TURN_FLOOR_LEFT90_MODE:
			case MANTIS_TURN_FLOOR_RIGHT90_MODE:
			case MANTIS_PHASE_FLOOR_MODE:
			case MANTIS_RAGE_FLOOR_MODE:
			case MANTIS_BLAST_FLOOR_MODE:
			case MANTIS_SPIT_FLOOR_MODE:
			case MANTIS_SIDE_SWIPE_FLOOR_MODE:
			case MANTIS_FORWARD_SWIPE_FLOOR_MODE:
				Mantis_SetupMode(pThis, MANTIS_EXPLODE_FLOOR_BACK_MODE) ;
				break ;

			// Ceiling modes
			case MANTIS_IDLE_CEILING_MODE:
			case MANTIS_RAIN_CEILING_MODE:
			case MANTIS_SPIT_CEILING_MODE:
			case MANTIS_WALK_CEILING_MODE:
			case MANTIS_TURN_CEILING_LEFT90_MODE:
			case MANTIS_TURN_CEILING_RIGHT90_MODE:
				Mantis_SetupMode(pThis, MANTIS_EXPLODE_CEILING_MODE) ;
				break ;

			// Left wall modes
			case MANTIS_IDLE_LEFT_WALL_MODE:
			case MANTIS_SPIT_LEFT_WALL_MODE:
			case MANTIS_SPIT_LEFT_WALL_LOOP_MODE:
				Mantis_SetupMode(pThis, MANTIS_EXPLODE_LEFT_WALL_MODE) ;
				break ;

			// Right wall modes
			case MANTIS_IDLE_RIGHT_WALL_MODE:
			case MANTIS_SPIT_RIGHT_WALL_MODE:
			case MANTIS_SPIT_RIGHT_WALL_LOOP_MODE:
				Mantis_SetupMode(pThis, MANTIS_EXPLODE_RIGHT_WALL_MODE) ;
				break ;
		}
	}
}




/*****************************************************************************
*
*	Function:		Mantis_MaxWalkCheck()
*
******************************************************************************
*
*	Description:	Checks to see if mantis has been walking too long, if so
*						passed in function is called
*
*	Inputs:			*pThis		-	Ptr to game object instance
*						*pMantis		-	Ptr to boss vars
*						Function		-	Function to call if walked for too long
*
*	Outputs:			TRUE if mantis was walking for too long, else FALSE
*
*****************************************************************************/
BOOL Mantis_MaxWalkCheck(CGameObjectInstance *pThis, CMantis *pMantis, MantisSetupFunction Function)
{
	if (pMantis->m_Boss.m_Action == MANTIS_ATTACK_ACTION)
	{
		pMantis->m_WalkTime += frame_increment ;
		if (pMantis->m_WalkTime >= pMantis->m_pStageInfo->MaxWalkTime)
		{
			pMantis->m_WalkTime = 0 ;
			Function(pThis, pMantis) ;
			return TRUE ;
		}
		else
			return FALSE ;
	}
	else
		return FALSE ;

}



/*****************************************************************************
*
*	Function:		Mantis_ShouldStayOnWall()
*
******************************************************************************
*
*	Description:	Checks to see if the mantis should stay on a wall - if
*						the mantis is too far away from Turok - he shouldn't stay
*						there. This function is called from "on wall" modes
*
*	Inputs:			*pThis		-	Ptr to game object instance
*						*pMantis		-	Ptr to boss vars
*
*	Outputs:			TRUE if mantis should be on wall, else FALSE
*
*****************************************************************************/
BOOL Mantis_ShouldStayOnWall(CGameObjectInstance *pThis, CMantis *pMantis)
{
	// Get target
	return (pMantis->m_Boss.m_DistFromTarget <= MANTIS_STAY_ON_WALL_MAX_DIST) ;
}





/*****************************************************************************
*
*	Function:		Mantis_CanSpit()
*
******************************************************************************
*
*	Description:	Decides if mantis can spit when flying
*
*	Inputs:			*pThis		-	Ptr to game object instance
*						*pMantis		-	Ptr to boss vars
*
*	Outputs:			TRUE if mantis can spit, else FALSE
*
*****************************************************************************/
BOOL Mantis_CanSpit(CGameObjectInstance *pThis, CMantis *pMantis)
{
	FLOAT	Angle = AngleDist(pMantis->m_Boss.m_TurokAngle + ANGLE_DTOR(180), pThis->m_RotY) ;

	// In fly mode?
	if (pMantis->m_Boss.m_Mode == MANTIS_JUMP_FLOOR_B_MODE)
	{
		// Resting or spit count not complete?
		if ((Angle > ANGLE_DTOR(15)) ||
			 (++pMantis->m_FlySpitTotal < pMantis->m_pStageInfo->FlySpitSkips))
			return FALSE ;

		// Reset spit count
		pMantis->m_FlySpitTotal = 0 ;

		// Glow mantis
		Mantis_StartGlow(pThis, pMantis, 1) ;
	}

	return TRUE ;
}





/*****************************************************************************
*
*	Function:		Mantis_KeepOnWall()
*
******************************************************************************
*
*	Description:	Keeps the mantis at the correct position on a wall since
*						the attack anims make him drift off.
*
*	Inputs:			*pThis		-	Ptr to game object instance
*						*pMantis		-	Ptr to boss vars
*
*	Outputs:			None
*
*****************************************************************************/
void Mantis_KeepOnWall(CGameObjectInstance *pThis, CMantis *pMantis)
{
	// Keep mantis in same wall position
	BOSS_GetVel(pThis).x += (pMantis->m_vWallPos.x - BOSS_GetPos(pThis).x) / 4 ;
	BOSS_GetVel(pThis).z += (pMantis->m_vWallPos.z - BOSS_GetPos(pThis).z) / 4 ;

	// Set correct angle
	pThis->m_RotY = pMantis->m_WallAngle ;
}


void Mantis_CheckForCollapsingWall(CGameObjectInstance *pThis, CMantis *pMantis)
{
 	// On broken wall?
	if ((pMantis->m_Boss.m_Action == MANTIS_BREAK_WALL_ACTION) &&
		 (pMantis->m_WallToAimFor != -1) &&
		 (pMantis->m_WallBroken[pMantis->m_WallToAimFor]) &&
		 (pMantis->m_Boss.m_ModeFlags & (BF_LEFT_WALL_MODE | BF_RIGHT_WALL_MODE)))
	{

		pMantis->m_WallCollapseTime += frame_increment ;
//		if (pMantis->m_WallCollapseTime >= SECONDS_TO_FRAMES(0.10))
		{
			pMantis->m_WallCollapseTime = 0 ;

			BOSS_GetVel(pThis).x = 0 ;
			BOSS_GetVel(pThis).z = 0 ;

			if (pMantis->m_Boss.m_ModeFlags & BF_LEFT_WALL_MODE)
				Mantis_SetupMode(pThis, MANTIS_JUMP_LEFT_WALL_TO_FLOOR_MODE) ;
			else
				Mantis_SetupMode(pThis, MANTIS_JUMP_RIGHT_WALL_TO_FLOOR_MODE) ;
		}
	}
}





/*****************************************************************************
*
*	Function:		Mantis_StartGlow()
*
******************************************************************************
*
*	Description:	Starts off an attack glow
*
*	Inputs:			*pThis		-	Ptr to game object instance
*						*pMantis		-	Ptr to boss vars
*
*	Outputs:			None
*
*****************************************************************************/
void Mantis_StartGlow(CGameObjectInstance *pThis, CMantis *pMantis, INT16 Count)
{
	// Only glow if mantis is attacking
//	CSpecialFx__StartScript(&pMantis->m_SpecialFx, pMantis->m_pStageInfo->pAttackGlowFx) ;
}






/*****************************************************************************
*
*	Function:		Mantis_RunScript()
*
******************************************************************************
*
*	Description:	Runs attack script and puts mantis in mode
*
*	Inputs:			*pThis		-	Ptr to game object instance
*						*pMantis		-	Ptr to boss vars
*						*pScript		-	Ptr to attack script
*
*	Outputs:			None
*
*****************************************************************************/
void Mantis_RunScript(CGameObjectInstance *pThis, CMantis *pMantis, CBossScript *pScript)
{

	// Read script mode and repeats
	pMantis->m_Boss.m_Mode = *pScript->m_pCurrent++ ;

	// Trigger off anim!
	pMantis->m_Boss.m_OldMode = -1 ;

	// Setup new mode
	Mantis_SetupMode(pThis, pMantis->m_Boss.m_Mode) ;

	// Start off glow?
	if (pMantis->m_Boss.m_ModeFlags & BF_ATTACK_MODE)
		Mantis_StartGlow(pThis, pMantis, 1) ;

	// End of script? - If so repeat
	if (*pScript->m_pCurrent == 0)
		pScript->m_pCurrent = pScript->m_pStart ;
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
void Mantis_Code_FloorMovement(CGameObjectInstance *pThis, CMantis *pMantis)
{
#if DEBUG_MANTIS_HEAD_TRACK

	pMantis->m_Boss.m_Mode = MANTIS_IDLE_FLOOR_MODE ;

#else

	FLOAT			DeltaAngle ;

	// Dampen vels
	BOSS_GetVel(pThis).x -=	BOSS_GetVel(pThis).x / 4 ;
	BOSS_GetVel(pThis).z -=	BOSS_GetVel(pThis).z / 4 ;

	// Walked for too long?
	if (!Mantis_MaxWalkCheck(pThis, pMantis, Mantis_Setup_FloorAttack))
	{
		// Goto fly?
		if (pMantis->m_Boss.m_DistFromTarget > MANTIS_FLY_TO_TARGET_DIST)
			Mantis_SetupMode(pThis, MANTIS_JUMP_FLOOR_A_MODE) ;
		else
		// Goto walk?
		if (pMantis->m_Boss.m_DistFromTarget > pMantis->m_Boss.m_AtTargetDist)
		{
			DeltaAngle = pMantis->m_Boss.m_DeltaAngle ;

			// Assist turn
			BOSS_PerformTurn(pThis, (CBoss *)pMantis, MANTIS_SURFACE_TURN_SPEED, DeltaAngle) ;

			// Turn left, right or walk?
			if (DeltaAngle < ANGLE_DTOR(-45))
				pMantis->m_Boss.m_Mode = MANTIS_TURN_FLOOR_LEFT90_MODE ;
			else
			if (DeltaAngle > ANGLE_DTOR(45))
				pMantis->m_Boss.m_Mode = MANTIS_TURN_FLOOR_RIGHT90_MODE ;
			else
				pMantis->m_Boss.m_Mode = MANTIS_WALK_FLOOR_MODE ;
		}
		else
		{
			// Facing correct direction?
			DeltaAngle = pMantis->m_Boss.m_LookDeltaAngle ;

			// Turn left, right or walk?
			if (DeltaAngle < ANGLE_DTOR(-35))
				pMantis->m_Boss.m_Mode = MANTIS_TURN_FLOOR_LEFT90_MODE ;
			else
			if (DeltaAngle > ANGLE_DTOR(35))
				pMantis->m_Boss.m_Mode = MANTIS_TURN_FLOOR_RIGHT90_MODE ;
			else
			{
				Mantis_Setup_FloorAttack(pThis, pMantis) ;
			}
		}
	}
#endif

}




/////////////////////////////////////////////////////////////////////////////
//	Modes:		 MANTIS_SLEEPING_FLOOR_MODE,
//	Description: Pausing before breaking out of cacoon
/////////////////////////////////////////////////////////////////////////////
void Mantis_Code_SLEEPING_FLOOR(CGameObjectInstance *pThis, CMantis *pMantis)
{
	// Keep still
	pThis->m_asCurrent.m_cFrame = 0 ;
	BOSS_GetVel(pThis).x = 0 ;
	BOSS_GetVel(pThis).y = 0 ;
	BOSS_GetVel(pThis).z = 0 ;
	AI_GetDyn(pThis)->m_Invincible = TRUE ;

	// Start pillars moving out of the ground?
	if (game_frame_number == 4)
	{
		// Trigger pillars
		AI_Event_Dispatcher(&pThis->ah.ih, &pThis->ah.ih, AI_MEVENT_PRESSUREPLATE, pThis->ah.ih.m_pEA->m_dwSpecies, pThis->ah.ih.m_vPos, MANTIS_LEVEL_PILLARS_ID) ;

		// Mantis stone slide sound
		AI_DoSound(&pThis->ah.ih, SOUND_GENERIC_236, 1, 0) ;
	}
}


/////////////////////////////////////////////////////////////////////////////
//	Modes:		 MANTIS_BREAK_FLOOR_MODE,
//	Description: Breaking out of cacoon
/////////////////////////////////////////////////////////////////////////////
void Mantis_Setup_BREAK_FLOOR(CGameObjectInstance *pThis, CMantis *pMantis)
{
	AI_GetDyn(pThis)->m_Invincible = FALSE ;
	pMantis->m_Boss.m_Mode = MANTIS_BREAK_FLOOR_MODE ;
	COnScreen__AddBossBar(&GetApp()->m_OnScreen, (CBoss *)pMantis) ;

	// Make visible
	CGameObjectInstance__SetNotGone(pThis) ;
}

void Mantis_Code_BREAK_FLOOR(CGameObjectInstance *pThis, CMantis *pMantis)
{
#if DEBUG_MANTIS_CEILING_TURNS
	// Anim finished?
	if (pMantis->m_Boss.m_AnimPlayed)
	{
		Mantis_SetupMode(pThis, MANTIS_JUMP_FLOOR_TO_CEILING_MODE) ;
		AI_GetDyn(pThis)->m_Invincible = FALSE ;
	}
#else
	// Anim finished?
	if (pMantis->m_Boss.m_AnimPlayed)
	{
		Mantis_SetupMode(pThis, MANTIS_IDLE_FLOOR_MODE) ;
		CPickupMonitor__Activate(&GetApp()->m_Scene.m_PickupMonitor) ;
	}
#endif

}


/////////////////////////////////////////////////////////////////////////////
//	Modes:		 MANTIS_RAGE_FLOOR_MODE,
//					 MANTIS_VICTORY_IDLE_FLOOR_MODE,
//	Description: Raging - victory or next stage celebration
/////////////////////////////////////////////////////////////////////////////
void Mantis_Code_RAGE_FLOOR(CGameObjectInstance *pThis, CMantis *pMantis)
{
	// Dampen movement
	BOSS_GetVel(pThis).x -=	BOSS_GetVel(pThis).x / 4 ;
	BOSS_GetVel(pThis).z -=	BOSS_GetVel(pThis).z / 4 ;

	// Anim finished?
	if (pThis->m_asCurrent.m_cFrame >= (pThis->m_asCurrent.m_nFrames - 5))
	{
		Mantis_SetupMode(pThis, MANTIS_IDLE_FLOOR_MODE) ;

		// Goto attack action?
		if (CTurokMovement.Active)
		{
			pMantis->m_Boss.m_Action = MANTIS_ATTACK_ACTION ;
			AI_GetDyn(pThis)->m_Invincible = FALSE ;
		}
	}
}




/////////////////////////////////////////////////////////////////////////////
//	Modes:		 MANTIS_???_FLOOR_MODE,
//	Description: All attacks on the floor
/////////////////////////////////////////////////////////////////////////////
void Mantis_Setup_FloorAttack(CGameObjectInstance *pThis, CMantis *pMantis)
{
	// Goto victory?
	if (pMantis->m_Boss.m_Action == MANTIS_RAGE_ACTION)
	{
		// Glow on rage!
		CSpecialFx__StartScript(&pMantis->m_SpecialFx, pMantis->m_pStageInfo->pAttackGlowFx) ;

//		if (pMantis->m_Boss.m_Mode == MANTIS_RAGE_FLOOR_MODE)
//			Mantis_SetupMode(pThis, MANTIS_VICTORY_IDLE_FLOOR_MODE) ;
//		else
			Mantis_SetupMode(pThis, MANTIS_RAGE_FLOOR_MODE) ;
	}
	else
	// Going to break a wall? If so jump into wall!
	if (pMantis->m_Boss.m_Action == MANTIS_BREAK_WALL_ACTION)
	{
		if (pMantis->m_WallToAimForJumpPos & 1)
			Mantis_SetupMode(pThis, MANTIS_JUMP_FLOOR_TO_RIGHT_WALL_A_MODE) ;
		else
			Mantis_SetupMode(pThis, MANTIS_JUMP_FLOOR_TO_LEFT_WALL_A_MODE) ;
	}
	else
	{
		// Do close attack or normal attack!
#if DEBUG_MANTIS
		rmonPrintf("Dist From Target %f\r\n", pMantis->m_Boss.m_DistFromTarget/SCALING_FACTOR) ;
#endif
		if (pMantis->m_Boss.m_DistFromTarget < MANTIS_ATTACK_CLOSE_DIST)
			Mantis_RunScript(pThis, pMantis, &pMantis->m_FloorCloseAttackScript) ;
		else
			Mantis_RunScript(pThis, pMantis, &pMantis->m_FloorAttackScript) ;
	}
}


void Mantis_Code_FloorAttack(CGameObjectInstance *pThis, CMantis *pMantis)
{
	// Dampen movement
	BOSS_GetVel(pThis).x -=	BOSS_GetVel(pThis).x / 4 ;
	BOSS_GetVel(pThis).z -=	BOSS_GetVel(pThis).z / 4 ;

	// Anim finished?
	if (pThis->m_asCurrent.m_cFrame >= (pThis->m_asCurrent.m_nFrames - 5))
		Mantis_SetupMode(pThis, MANTIS_IDLE_FLOOR_MODE) ;
}





void Mantis_Setup_CeilingAttack(CGameObjectInstance *pThis, CMantis *pMantis)
{
#if DEBUG_MANTIS_CEILING_TURNS
	pMantis->m_Boss.m_Mode = MANTIS_IDLE_CEILING_MODE ;
#else

	// Going to break a wall? If so jump to floor!
	if (pMantis->m_GotoFloor)
	{
		Mantis_SetupMode(pThis, MANTIS_JUMP_CEILING_TO_FLOOR_MODE) ;
		return ;
	}

	// Do move
	Mantis_RunScript(pThis, pMantis, &pMantis->m_CeilingAttackScript) ;
#endif
}

void Mantis_Code_CeilingAttack(CGameObjectInstance *pThis, CMantis *pMantis)
{
	// Turn gravity off
	pMantis->m_Boss.m_CollisionInfo.GravityAcceleration	= 0 ;

	// Anim finished?
	if (pThis->m_asCurrent.m_cFrame >= (pThis->m_asCurrent.m_nFrames - 5))
		Mantis_SetupMode(pThis, MANTIS_IDLE_CEILING_MODE) ;
}



void Mantis_Setup_LeftWallAttack(CGameObjectInstance *pThis, CMantis *pMantis)
{
	// Going to break a wall? If so jump back to the floor
	if (pMantis->m_GotoFloor)
	{
		Mantis_SetupMode(pThis, MANTIS_JUMP_LEFT_WALL_TO_FLOOR_MODE) ;
		return ;
	}

	// Do move
	Mantis_RunScript(pThis, pMantis, &pMantis->m_LeftWallAttackScript) ;
}

void Mantis_Code_LeftWallAttack(CGameObjectInstance *pThis, CMantis *pMantis)
{
	// Keep mantis in wall position
	Mantis_KeepOnWall(pThis, pMantis) ;

	// Anim finished?
	if (pThis->m_asCurrent.m_cFrame >= (pThis->m_asCurrent.m_nFrames - 5))
	{
		// Jump off wall?
		if (!Mantis_ShouldStayOnWall(pThis, pMantis))
			Mantis_SetupMode(pThis, MANTIS_JUMP_LEFT_WALL_TO_FLOOR_MODE) ;				// Jump to floor
		else
			Mantis_Setup_LeftWallAttack(pThis, pMantis) ;
	}
}




void Mantis_Setup_RightWallAttack(CGameObjectInstance *pThis, CMantis *pMantis)
{
	// Going to break a wall? If so jump back to the floor
	if (pMantis->m_GotoFloor)
	{
		Mantis_SetupMode(pThis, MANTIS_JUMP_RIGHT_WALL_TO_FLOOR_MODE) ;
		return ;
	}

	// Do move
	Mantis_RunScript(pThis, pMantis, &pMantis->m_RightWallAttackScript) ;
}

void Mantis_Code_RightWallAttack(CGameObjectInstance *pThis, CMantis *pMantis)
{
	// Keep mantis in wall position
	Mantis_KeepOnWall(pThis, pMantis) ;

	// Anim finished?
	if (pThis->m_asCurrent.m_cFrame >= (pThis->m_asCurrent.m_nFrames - 5))
	{
		// Jump off wall?
		if (!Mantis_ShouldStayOnWall(pThis, pMantis))
			Mantis_SetupMode(pThis, MANTIS_JUMP_RIGHT_WALL_TO_FLOOR_MODE) ;				// Jump to floor
		else
			Mantis_Setup_RightWallAttack(pThis, pMantis) ;
	}
}



/////////////////////////////////////////////////////////////////////////////
//	Modes:		 MANTIS_JUMP_FLOOR_TO_CEILING_MODE,
//	Description: Jumping from floor up to ceiling
/////////////////////////////////////////////////////////////////////////////
void Mantis_Code_JUMP_FLOOR_TO_CEILING(CGameObjectInstance *pThis, CMantis *pMantis)
{
	// Turn gravity off
	pMantis->m_Boss.m_CollisionInfo.GravityAcceleration	= 0 ;

	// On ceiling?
	if (pThis->m_asCurrent.m_cFrame >= (pThis->m_asCurrent.m_nFrames - 5))
		Mantis_Setup_CeilingAttack(pThis, pMantis) ;
}


/////////////////////////////////////////////////////////////////////////////
//	Mode:			 MANTIS_WALK_CEILING_MODE
//					 MANTIS_TURN_CEILING_LEFT90_MODE,
//					 MANTIS_TURN_CEILING_RIGHT90_MODE,
//	Description: Turning 90 degrees on the ceiling
/////////////////////////////////////////////////////////////////////////////
void Mantis_Code_CeilingMovement(CGameObjectInstance *pThis, CMantis *pMantis)
{
	FLOAT			DeltaAngle ;

	// Going to break a wall? If so jump to floor!
	if (pMantis->m_GotoFloor)
	{
		Mantis_SetupMode(pThis, MANTIS_JUMP_CEILING_TO_FLOOR_MODE) ;
		return ;
	}

	// Walked for too long?
	if (!Mantis_MaxWalkCheck(pThis, pMantis, Mantis_Setup_CeilingAttack))
	{
		// Goto fly?
		if (pMantis->m_Boss.m_DistFromTarget > MANTIS_FLY_TO_TARGET_DIST)
			Mantis_SetupMode(pThis, MANTIS_JUMP_CEILING_TO_FLOOR_MODE) ;
		else
		// Goto walk?
		if (pMantis->m_Boss.m_DistFromTarget > pMantis->m_Boss.m_AtTargetDist)
		{
			DeltaAngle = pMantis->m_Boss.m_DeltaAngle ;

			// Turn left, right or walk?
			if (DeltaAngle < ANGLE_DTOR(-25))
				pMantis->m_Boss.m_Mode = MANTIS_TURN_CEILING_LEFT90_MODE ;
			else
			if (DeltaAngle > ANGLE_DTOR(25))
				pMantis->m_Boss.m_Mode = MANTIS_TURN_CEILING_RIGHT90_MODE ;
			else
			{
				pMantis->m_Boss.m_Mode = MANTIS_WALK_CEILING_MODE ;
				BOSS_PerformTurn(pThis, (CBoss *)pMantis, MANTIS_SURFACE_TURN_SPEED, DeltaAngle) ;
			}
		}
		else
		{
			// Facing correct direction?
			DeltaAngle = pMantis->m_Boss.m_LookDeltaAngle ;

			// Turn left, right or walk?
			if (DeltaAngle < ANGLE_DTOR(-25))
				pMantis->m_Boss.m_Mode = MANTIS_TURN_CEILING_LEFT90_MODE ;
			else
			if (DeltaAngle > ANGLE_DTOR(25))
				pMantis->m_Boss.m_Mode = MANTIS_TURN_CEILING_RIGHT90_MODE ;
			else
				Mantis_Setup_CeilingAttack(pThis, pMantis) ;
		}
	}
}




/////////////////////////////////////////////////////////////////////////////
//	Modes:		 MANTIS_JUMP_CEILING_TO_FLOOR_MODE,
//	Description: Jumping from ceiling back down to the floor
/////////////////////////////////////////////////////////////////////////////
void Mantis_Code_JUMP_CEILING_TO_FLOOR(CGameObjectInstance *pThis, CMantis *pMantis)
{
	// Turn gravity on
	pMantis->m_Boss.m_CollisionInfo.GravityAcceleration	= MANTIS_GRAVITY ;

	// Anim finished?
	if (pThis->m_asCurrent.m_cFrame >= (pThis->m_asCurrent.m_nFrames - 5))
		Mantis_SetupMode(pThis, MANTIS_IDLE_FLOOR_MODE) ;
}






/////////////////////////////////////////////////////////////////////////////
//	Mode:			 MANTIS_JUMP_FLOOR_A_MODE
//	Description: Perparing to jump off the floor
/////////////////////////////////////////////////////////////////////////////
void Mantis_Hover(CGameObjectInstance *pThis, CMantis *pMantis)
{
	// Bob mantis up and down a bit
	BOSS_GetVel(pThis).y = -4.0 * cos(pMantis->m_FlyTime) * 15 ;
	pMantis->m_FlyTime += ANGLE_DTOR(360/20) * frame_increment ;

 	// Dampen vel
//	BOSS_GetVel(pThis).y -= BOSS_GetVel(pThis).y / 32 ;
}

void Mantis_Setup_JUMP_FLOOR_A(CGameObjectInstance *pThis, CMantis *pMantis)
{
	pMantis->m_Boss.m_Mode = MANTIS_JUMP_FLOOR_A_MODE ;
	pMantis->m_FlyTargetY = CInstanceHdr__GetGroundHeight(&pThis->ah.ih) + 50.0 ;
	pMantis->m_FlySpeed = 0 ;
	pMantis->m_FlyTime = 0 ;

	// Turn gravity off
	pMantis->m_Boss.m_CollisionInfo.GravityAcceleration	= 0 ;
}

void Mantis_Code_JUMP_FLOOR_A(CGameObjectInstance *pThis, CMantis *pMantis)
{
	// Anim nearly finished?
	if (pThis->m_asCurrent.m_cFrame >= (pThis->m_asCurrent.m_nFrames -3))
		Mantis_SetupMode(pThis, MANTIS_JUMP_FLOOR_B_MODE) ;

	// Call fly code
	if (pThis->m_asCurrent.m_cFrame >= 5)
		Mantis_Code_JUMP_FLOOR_B(pThis, pMantis) ;
}


/////////////////////////////////////////////////////////////////////////////
//	Mode:			 MANTIS_JUMP_FLOOR_B_MODE
//	Description: Hovering chasing a Turok/going to break walls
/////////////////////////////////////////////////////////////////////////////
void Mantis_Setup_JUMP_FLOOR_B(CGameObjectInstance *pThis, CMantis *pMantis)
{
	pMantis->m_Boss.m_Mode = MANTIS_JUMP_FLOOR_B_MODE ;

	// Give a bit of y vel
//	BOSS_GetVel(pThis).y = BOSS_GetPos(pThis).y - pMantis->m_Boss.m_LastPos.y ;
}

void Mantis_Code_JUMP_FLOOR_B(CGameObjectInstance *pThis, CMantis *pMantis)
{
	FLOAT	DeltaAngle = pMantis->m_Boss.m_DeltaAngle ;
	FLOAT	TargetRot, Speed, FlySpeed ;

	// Going to break a wall? If so jump to floor!
	if ((pMantis->m_GotoFloor) && (pMantis->m_Boss.m_Action == MANTIS_RAGE_ACTION))
	{
		Mantis_SetupMode(pThis, MANTIS_JUMP_FLOOR_C_MODE) ;
		return ;
	}

	// Calc fly speed
	Speed = (BOSS_GetVel(pThis).x * BOSS_GetVel(pThis).x) +
			  (BOSS_GetVel(pThis).z * BOSS_GetVel(pThis).z) ;
	if (Speed != 0)
		Speed = sqrt(Speed) ;

	// Alter speed
	FlySpeed = pMantis->m_pStageInfo->FlySpeed ;

	// Speed up?
	if (pMantis->m_Boss.m_Mode == MANTIS_JUMP_FLOOR_B_MODE)
	{
		if ((pMantis->m_Boss.m_DistFromTarget > pMantis->m_Boss.m_AtTargetDist) ||
			 (pMantis->m_Boss.m_Action == MANTIS_BREAK_WALL_ACTION))
			pMantis->m_FlySpeed = AccelerateFLOAT(pMantis->m_FlySpeed, FlySpeed / 16, FlySpeed) ;
		else
			pMantis->m_FlySpeed = DecelerateFLOAT(pMantis->m_FlySpeed, FlySpeed / 8) ;

		// Set vels
		BOSS_GetVel(pThis).x = -pMantis->m_FlySpeed * sin(pThis->m_RotY) * frame_increment * 15 ;
		BOSS_GetVel(pThis).z = -pMantis->m_FlySpeed * cos(pThis->m_RotY) * frame_increment * 15 ;
	}


	// Hover that monster on Y axis
	Mantis_Hover(pThis, pMantis) ;

	// Turn mantis
	BOSS_PerformTurn(pThis, (CBoss *)pMantis, MANTIS_FLY_TURN_SPEED, DeltaAngle) ;

	// At final target and facing turok?
	if ((pMantis->m_Boss.m_AnimPlayed) &&
		 (pMantis->m_Boss.m_DistFromTarget < pMantis->m_Boss.m_AtTargetDist) &&
		 (Mantis_FacingTurok(pThis, pMantis)))
		Mantis_SetupMode(pThis, MANTIS_JUMP_FLOOR_C_MODE) ;

	// Set x rotation
	TargetRot = -Speed / 20 ;
	if (TargetRot)
	{
		if (TargetRot < ANGLE_DTOR(-15))
			TargetRot =  ANGLE_DTOR(-15) ;
		else
		if (TargetRot > ANGLE_DTOR(15))
			TargetRot =  ANGLE_DTOR(15) ;

		pMantis->m_Boss.m_RotX = ZoomFLOAT(pMantis->m_Boss.m_RotX,
													  TargetRot,
													  MANTIS_TILT_X_SPEED) ;
	}
	else
		pMantis->m_Boss.m_RotX = ZoomFLOAT(pMantis->m_Boss.m_RotX,
													  TargetRot,
													  MANTIS_TILT_X_SPEED/2) ;

	// Set z rotation
	TargetRot = DeltaAngle*10 ;
	if (TargetRot)
	{
		if (TargetRot < ANGLE_DTOR(-15))
			TargetRot =  ANGLE_DTOR(-15) ;
		else
		if (TargetRot > ANGLE_DTOR(15))
			TargetRot =  ANGLE_DTOR(15) ;

		pMantis->m_Boss.m_RotZ = ZoomFLOAT(pMantis->m_Boss.m_RotZ,
													  TargetRot,
													  MANTIS_TILT_Z_SPEED) ;
	}
	else
		pMantis->m_Boss.m_RotZ = ZoomFLOAT(pMantis->m_Boss.m_RotZ,
													  TargetRot,
													  MANTIS_TILT_Z_SPEED/2) ;

	// Set anim speed proportional to moving speed
	pMantis->m_Boss.m_ModeAnimSpeed = 1.5 + (Speed/5) ;
	if (pMantis->m_Boss.m_ModeAnimSpeed > 2.5)
		pMantis->m_Boss.m_ModeAnimSpeed = 2.5 ;
}





/////////////////////////////////////////////////////////////////////////////
//	Mode:			 MANTIS_JUMP_FLOOR_C_MODE
//	Description: Landing back on floor
/////////////////////////////////////////////////////////////////////////////
void Mantis_Setup_JUMP_FLOOR_C(CGameObjectInstance *pThis, CMantis *pMantis)
{
	pMantis->m_Boss.m_Mode = MANTIS_JUMP_FLOOR_C_MODE ;

	// Turn gravity on
	pMantis->m_Boss.m_CollisionInfo.GravityAcceleration	= MANTIS_GRAVITY ;
}

void Mantis_Code_JUMP_FLOOR_C(CGameObjectInstance *pThis, CMantis *pMantis)
{
	// Dampen movement
	BOSS_GetVel(pThis).x -=	BOSS_GetVel(pThis).x / 4 ;
	BOSS_GetVel(pThis).z -=	BOSS_GetVel(pThis).z / 4 ;

	// Anim finished?
	if (pThis->m_asCurrent.m_cFrame >= (pThis->m_asCurrent.m_nFrames - 5))
		Mantis_SetupMode(pThis, MANTIS_IDLE_FLOOR_MODE) ;
}




/////////////////////////////////////////////////////////////////////////////
//	Modes:		 MANTIS_JUMP_FLOOR_TO_LEFT_WALL_A_MODE,
//					 MANTIS_JUMP_FLOOR_TO_RIGHT_WALL_A_MODE,
//					 MANTIS_JUMP_LEFT_WALL_TO_RIGHT_WALL_A_MODE,
//					 MANTIS_JUMP_RIGHT_WALL_TO_LEFT_WALL_A_MODE,
//
//	Description: Mantis jumping off floor towards wall
/////////////////////////////////////////////////////////////////////////////
void Mantis_Setup_JUMP_FLOOR_TO_LEFT_WALL_A(CGameObjectInstance *pThis, CMantis *pMantis)
{
	if ((pMantis->m_Boss.m_Action == MANTIS_BREAK_WALL_ACTION) || (Mantis_WallAtLeftSide(pThis, pMantis)))
		pMantis->m_Boss.m_Mode = MANTIS_JUMP_FLOOR_TO_LEFT_WALL_A_MODE ;
	else
	if (Mantis_WallAtRightSide(pThis, pMantis))
		pMantis->m_Boss.m_Mode = MANTIS_JUMP_FLOOR_TO_RIGHT_WALL_A_MODE ;
	else
	{
		if (pMantis->m_CeilingAttackScript.m_pStart)
			pMantis->m_Boss.m_Mode = MANTIS_JUMP_FLOOR_TO_CEILING_MODE ;
		else
			pMantis->m_Boss.m_Mode = MANTIS_IDLE_FLOOR_MODE ;
	}
}

void Mantis_Setup_JUMP_FLOOR_TO_RIGHT_WALL_A(CGameObjectInstance *pThis, CMantis *pMantis)
{
	if ((pMantis->m_Boss.m_Action == MANTIS_BREAK_WALL_ACTION) || (Mantis_WallAtRightSide(pThis, pMantis)))
		pMantis->m_Boss.m_Mode = MANTIS_JUMP_FLOOR_TO_RIGHT_WALL_A_MODE ;
	else
	if (Mantis_WallAtLeftSide(pThis, pMantis))
		pMantis->m_Boss.m_Mode = MANTIS_JUMP_FLOOR_TO_LEFT_WALL_A_MODE ;
	else
	{
		if (pMantis->m_CeilingAttackScript.m_pStart)
			pMantis->m_Boss.m_Mode = MANTIS_JUMP_FLOOR_TO_CEILING_MODE ;
		else
			pMantis->m_Boss.m_Mode = MANTIS_IDLE_FLOOR_MODE ;
	}
}

void Mantis_Setup_JUMP_LEFT_WALL_TO_RIGHT_WALL_A(CGameObjectInstance *pThis, CMantis *pMantis)
{
	if (Mantis_WallAtRightSide(pThis, pMantis))
		pMantis->m_Boss.m_Mode = MANTIS_JUMP_LEFT_WALL_TO_RIGHT_WALL_A_MODE ;
	else
	if (pMantis->m_CeilingAttackScript.m_pStart)
		pMantis->m_Boss.m_Mode = MANTIS_JUMP_LEFT_WALL_TO_CEILING_MODE ;
	else
		pMantis->m_Boss.m_Mode = MANTIS_JUMP_LEFT_WALL_TO_FLOOR_MODE ;
}

void Mantis_Setup_JUMP_RIGHT_WALL_TO_LEFT_WALL_A(CGameObjectInstance *pThis, CMantis *pMantis)
{
	if (Mantis_WallAtLeftSide(pThis, pMantis))
		pMantis->m_Boss.m_Mode = MANTIS_JUMP_RIGHT_WALL_TO_LEFT_WALL_A_MODE ;
	else
	if (pMantis->m_CeilingAttackScript.m_pStart)
		pMantis->m_Boss.m_Mode = MANTIS_JUMP_RIGHT_WALL_TO_CEILING_MODE ;
	else
		pMantis->m_Boss.m_Mode = MANTIS_JUMP_RIGHT_WALL_TO_FLOOR_MODE ;
}


void Mantis_Code_JumpToWallA(CGameObjectInstance *pThis, CMantis *pMantis)
{
	// Turn gravity off
	pMantis->m_Boss.m_CollisionInfo.GravityAcceleration	= 0 ;

	// Make breaking walls more acurate
	if (pMantis->m_Boss.m_Action == MANTIS_BREAK_WALL_ACTION)
		AI_NudgeRotY(pThis, pMantis->m_Boss.m_LookDeltaAngle) ;

	// Collided with wall?
	if ((pMantis->m_Boss.m_Speed > 5.0) && (Mantis_NearWall(pThis, pMantis)))
		Mantis_SetupMode(pThis, pMantis->m_Boss.m_Mode + 2) ;
	else
	// Anim finished?
	if (pMantis->m_Boss.m_AnimPlayed)
//	if (pThis->m_asCurrent.m_cFrame >= (pThis->m_asCurrent.m_nFrames - 5))
		Mantis_SetupMode(pThis, pMantis->m_Boss.m_Mode + 1) ;
}


/////////////////////////////////////////////////////////////////////////////
//	Modes:		 MANTIS_JUMP_FLOOR_TO_LEFT_WALL_B_MODE,
//					 MANTIS_JUMP_FLOOR_TO_RIGHT_WALL_B_MODE,
//					 MANTIS_JUMP_LEFT_WALL_TO_RIGHT_WALL_B_MODE,
//					 MANTIS_JUMP_RIGHT_WALL_TO_LEFT_WALL_B_MODE,
//	Description: Flying towards wall
/////////////////////////////////////////////////////////////////////////////
void Mantis_Code_JumpToWallB(CGameObjectInstance *pThis, CMantis *pMantis)
{
	// Make breaking walls more acurate
	if (pMantis->m_Boss.m_Action == MANTIS_BREAK_WALL_ACTION)
		AI_NudgeRotY(pThis, pMantis->m_Boss.m_LookDeltaAngle) ;

	// Collided with wall?
	if (Mantis_NearWall(pThis, pMantis))
		Mantis_SetupMode(pThis, pMantis->m_Boss.m_Mode + 1) ;
}


/////////////////////////////////////////////////////////////////////////////
//	Modes:		 MANTIS_JUMP_FLOOR_TO_LEFT_WALL_C_MODE,
//					 MANTIS_JUMP_FLOOR_TO_RIGHT_WALL_C_MODE,
//					 MANTIS_JUMP_LEFT_WALL_TO_RIGHT_WALL_C_MODE,
//					 MANTIS_JUMP_RIGHT_WALL_TO_LEFT_WALL_C_MODE,
//	Description: Landing on wall
/////////////////////////////////////////////////////////////////////////////
void Mantis_Code_JumpToWallC(CGameObjectInstance *pThis, CMantis *pMantis)
{
	// Put mantis on the wall
	Mantis_KeepOnWall(pThis, pMantis) ;

	// Anim finished?
	if (pThis->m_asCurrent.m_cFrame >= (pThis->m_asCurrent.m_nFrames - 5))
	{
		// Goto correct idle
		switch(pMantis->m_Boss.m_Mode)
		{
			case MANTIS_JUMP_FLOOR_TO_LEFT_WALL_C_MODE:
			case MANTIS_JUMP_RIGHT_WALL_TO_LEFT_WALL_C_MODE:
				Mantis_Setup_LeftWallAttack(pThis, pMantis) ;
				break ;

			case MANTIS_JUMP_FLOOR_TO_RIGHT_WALL_C_MODE:
			case MANTIS_JUMP_LEFT_WALL_TO_RIGHT_WALL_C_MODE:
				Mantis_Setup_RightWallAttack(pThis, pMantis) ;
				break ;

		}
	}
}



/////////////////////////////////////////////////////////////////////////////
//	Modes:		 MANTIS_JUMP_LEFT_WALL_TO_FLOOR_MODE,
//					 MANTIS_JUMP_RIGHT_WALL_TO_FLOOR_MODE
//	Description: Jumping from wall to the floor
/////////////////////////////////////////////////////////////////////////////
void Mantis_Code_JumpWallToFloor(CGameObjectInstance *pThis, CMantis *pMantis)
{
	// Turn gravity on
	pMantis->m_Boss.m_CollisionInfo.GravityAcceleration	= MANTIS_GRAVITY ;

	// Anim finished?
	if (pThis->m_asCurrent.m_cFrame >= (pThis->m_asCurrent.m_nFrames - 5))
		Mantis_SetupMode(pThis, MANTIS_IDLE_FLOOR_MODE) ;
}


/////////////////////////////////////////////////////////////////////////////
//	Modes:		 MANTIS_JUMP_LEFT_WALL_TO_CEILING_MODE,
//					 MANTIS_JUMP_RIGHT_WALL_TO_CEILING_MODE
//	Description: Jumping from wall to the floor
/////////////////////////////////////////////////////////////////////////////
void Mantis_Code_JumpWallToCeiling(CGameObjectInstance *pThis, CMantis *pMantis)
{
	// Turn gravity off
	pMantis->m_Boss.m_CollisionInfo.GravityAcceleration	= 0 ;

	// Anim finished?
	if (pThis->m_asCurrent.m_cFrame >= (pThis->m_asCurrent.m_nFrames - 5))
		Mantis_Setup_CeilingAttack(pThis, pMantis) ;
}




/////////////////////////////////////////////////////////////////////////////
//	Modes:		 MANTIS_EXPLODE_FLOOR_BACK_MODE,
//					 MANTIS_EXPLODE_FLOOR_LEFT_MODE,
//					 MANTIS_EXPLODE_FLOOR_FRONT_MODE,
//					 MANTIS_EXPLODE_FLOOR_RIGHT_MODE,
//	Description: Puts mantis into correct floor explosion flinch mode
/////////////////////////////////////////////////////////////////////////////
INT16 ExplodeFloorModes[] =
{
	MANTIS_EXPLODE_FLOOR_BACK_MODE,
	MANTIS_EXPLODE_FLOOR_LEFT_MODE,
	MANTIS_EXPLODE_FLOOR_FRONT_MODE,
	MANTIS_EXPLODE_FLOOR_RIGHT_MODE,
} ;

void Mantis_Setup_ExplodeFloor(CGameObjectInstance *pThis, CMantis *pMantis)
{
	CGameObjectInstance *pTurok = CEngineApp__GetPlayer(GetApp()) ;
	FLOAT						AngleDiff = (pTurok->m_RotY -  pThis->m_RotY - ANGLE_DTOR(45)) / ANGLE_DTOR(90) ;
	INT32						Index = (INT32)AngleDiff ;

	// Setup correct mode
	pMantis->m_Boss.m_Mode = ExplodeFloorModes[Index & 3] ;
}

void Mantis_Code_ExplodeFloor(CGameObjectInstance *pThis, CMantis *pMantis)
{
	// Anim finished?
	if (pThis->m_asCurrent.m_cFrame >= (pThis->m_asCurrent.m_nFrames - 5))
		Mantis_Setup_FloorAttack(pThis, pMantis) ;
}


/////////////////////////////////////////////////////////////////////////////
//	Modes:		 MANTIS_EXPLODE_CEILING_MODE,
//	Description: Just been hit by an explosion on the ceiling
/////////////////////////////////////////////////////////////////////////////
void Mantis_Code_ExplodeCeiling(CGameObjectInstance *pThis, CMantis *pMantis)
{
	// Anim finished?
	if (pThis->m_asCurrent.m_cFrame >= (pThis->m_asCurrent.m_nFrames - 5))
		Mantis_Setup_CeilingAttack(pThis, pMantis) ;
}


/////////////////////////////////////////////////////////////////////////////
//	Modes:		 MANTIS_EXPLODE_LEFT_WALL_MODE,
//	Description: Just been hit by an explosion on a left wall
/////////////////////////////////////////////////////////////////////////////
void Mantis_Code_ExplodeLeftWall(CGameObjectInstance *pThis, CMantis *pMantis)
{
	// Keep mantis in wall position
	Mantis_KeepOnWall(pThis, pMantis) ;

	// Anim finished?
	if (pThis->m_asCurrent.m_cFrame >= (pThis->m_asCurrent.m_nFrames - 5))
		Mantis_Setup_LeftWallAttack(pThis, pMantis) ;
}



/////////////////////////////////////////////////////////////////////////////
//	Modes:		 MANTIS_EXPLODE_RIGHT_WALL_MODE,
//	Description: Just been hit by an explosion on a right wall
/////////////////////////////////////////////////////////////////////////////
void Mantis_Code_ExplodeRightWall(CGameObjectInstance *pThis, CMantis *pMantis)
{
	// Keep mantis in wall position
	Mantis_KeepOnWall(pThis, pMantis) ;

	// Anim finished?
	if (pThis->m_asCurrent.m_cFrame >= (pThis->m_asCurrent.m_nFrames - 5))
		Mantis_Setup_RightWallAttack(pThis, pMantis) ;
}





/////////////////////////////////////////////////////////////////////////////
//	Modes:		 MANTIS_DEATH_MODE,
//	Description: Mantis dying and giving out pickups
/////////////////////////////////////////////////////////////////////////////
void Mantis_Setup_DEATH(CGameObjectInstance *pThis, CMantis *pMantis)
{
	pMantis->m_Boss.m_Mode = MANTIS_DEATH_FLOOR_MODE ;
	pMantis->m_Boss.m_MoveTime = -1 ;

	GetApp()->m_BossFlags |= BOSS_FLAG_MANTIS_DEAD	;


	CCamera__SetObjectToTrack(&GetApp()->m_Camera, pThis) ;
	CPickupMonitor__Deactivate(&GetApp()->m_Scene.m_PickupMonitor) ;

	CCamera__FadeToCinema(&GetApp()->m_Camera, CINEMA_FLAG_TUROK_KILL_MANTIS) ;
}

void Mantis_Code_DEATH(CGameObjectInstance *pThis, CMantis *pMantis)
{
	CGameObjectInstance *pTurok = CEngineApp__GetPlayer(GetApp()) ;

	// Make crystal rise
	AI_Event_Dispatcher(&pTurok->ah.ih, &pTurok->ah.ih, AI_MEVENT_PRESSUREPLATE,
							  pTurok->ah.ih.m_pEA->m_dwSpecies, pTurok->ah.ih.m_vPos, MANTIS_LEVEL_CRYSTAL_ID+100) ;
}



/////////////////////////////////////////////////////////////////////////////
//	Modes:		 MANTIS_KILLTUROK_MODE,
//					 MANTIS_CINEMA1_MODE,
//					 MANTIS_CINEMA2_MODE,
//					 MANTIS_CINEMA3_MODE,
//					 MANTIS_CINEMA4_MODE,
//					 MANTIS_CINEMA5_MODE,
//	Description: Cinema stuff
/////////////////////////////////////////////////////////////////////////////
void Mantis_Code_CINEMA(CGameObjectInstance *pThis, CMantis *pMantis)
{
	// Put gravity on so mantis is on the floor
	pMantis->m_Boss.m_CollisionInfo.GravityAcceleration = MANTIS_GRAVITY ;

	// Anim finished?
	if (pThis->m_asCurrent.m_cFrame >= (pThis->m_asCurrent.m_nFrames - 2))
	{
		switch(pMantis->m_Boss.m_Mode)
		{
			case MANTIS_KILLTUROK_MODE:
				Mantis_SetupMode(pThis, MANTIS_CINEMA1_MODE) ;
				break ;
			case MANTIS_CINEMA1_MODE:
				Mantis_SetupMode(pThis, MANTIS_CINEMA2_MODE) ;
				break ;
			case MANTIS_CINEMA2_MODE:
				Mantis_SetupMode(pThis, MANTIS_CINEMA3_MODE) ;
				break ;
			case MANTIS_CINEMA3_MODE:
				Mantis_SetupMode(pThis, MANTIS_CINEMA4_MODE) ;
				break ;
			case MANTIS_CINEMA4_MODE:
				Mantis_SetupMode(pThis, MANTIS_CINEMA5_MODE) ;
				break ;
			case MANTIS_CINEMA5_MODE:
				Mantis_SetupMode(pThis, MANTIS_IDLE_FLOOR_MODE) ;
				break ;
		}
	}
}



/*****************************************************************************
*
*	Function:		Mantis_DisplayMode()
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
void Mantis_DisplayMode(CBoss *pBoss)
{
#if DEBUG_MANTIS
	switch(pBoss->m_Mode)
	{
		case MANTIS_BREAK_FLOOR_MODE:
			rmonPrintf("MANTIS_BREAK_FLOOR_MODE\r\n") ;
			break ;
		case MANTIS_IDLE_FLOOR_MODE:
			rmonPrintf("MANTIS_IDLE_FLOOR_MODE\r\n") ;
			break ;
		case MANTIS_LOOK_FLOOR_MODE:
			rmonPrintf("MANTIS_LOOK_FLOOR_MODE\r\n") ;
			break ;
		case MANTIS_WALK_FLOOR_MODE:
			rmonPrintf("MANTIS_WALK_FLOOR_MODE\r\n") ;
			break ;
		case MANTIS_TURN_FLOOR_LEFT90_MODE:
			rmonPrintf("MANTIS_TURN_FLOOR_LEFT90_MODE\r\n") ;
			break ;
		case MANTIS_TURN_FLOOR_RIGHT90_MODE:
			rmonPrintf("MANTIS_TURN_FLOOR_RIGHT90_MODE\r\n") ;
			break ;
		case MANTIS_JUMP_FLOOR_A_MODE:
			rmonPrintf("MANTIS_JUMP_FLOOR_A_MODE\r\n") ;
			break ;
		case MANTIS_JUMP_FLOOR_B_MODE:
			rmonPrintf("MANTIS_JUMP_FLOOR_B_MODE\r\n") ;
			break ;
		case MANTIS_JUMP_FLOOR_C_MODE:
			rmonPrintf("MANTIS_JUMP_FLOOR_C_MODE\r\n") ;
			break ;
		case MANTIS_PHASE_FLOOR_MODE:
			rmonPrintf("MANTIS_PHASE_FLOOR_MODE\r\n") ;
			break ;
		case MANTIS_BLAST_FLOOR_MODE:
			rmonPrintf("MANTIS_BLAST_FLOOR_MODE\r\n") ;
			break ;
		case MANTIS_SPIT_FLOOR_MODE:
			rmonPrintf("MANTIS_SPIT_FLOOR_MODE\r\n") ;
			break ;
		case MANTIS_SIDE_SWIPE_FLOOR_MODE:
			rmonPrintf("MANTIS_SIDE_SWIPE_FLOOR_MODE\r\n") ;
			break ;
		case MANTIS_FORWARD_SWIPE_FLOOR_MODE:
			rmonPrintf("MANTIS_FORWARD_SWIPE_FLOOR_MODE\r\n") ;
			break ;
		case MANTIS_VICTORY_IDLE_FLOOR_MODE:
			rmonPrintf("MANTIS_VICTORY_IDLE_FLOOR_MODE\r\n") ;
			break ;
		case MANTIS_RAGE_FLOOR_MODE:
			rmonPrintf("MANTIS_RAGE_FLOOR_MODE\r\n") ;
			break ;
		case MANTIS_JUMP_FLOOR_TO_CEILING_MODE:
			rmonPrintf("MANTIS_JUMP_FLOOR_TO_CEILING_MODE\r\n") ;
			break ;
		case MANTIS_IDLE_CEILING_MODE:
			rmonPrintf("MANTIS_IDLE_CEILING_MODE\r\n") ;
			break ;
		case MANTIS_RAIN_CEILING_MODE:
			rmonPrintf("MANTIS_RAIN_CEILING_MODE\r\n") ;
			break ;
		case MANTIS_SPIT_CEILING_MODE:
			rmonPrintf("MANTIS_SPIT_CEILING_MODE\r\n") ;
			break ;
		case MANTIS_WALK_CEILING_MODE:
			rmonPrintf("MANTIS_WALK_CEILING_MODE\r\n") ;
			break ;
		case MANTIS_TURN_CEILING_LEFT90_MODE:
			rmonPrintf("MANTIS_TURN_CEILING_LEFT90_MODE\r\n") ;
			break ;
		case MANTIS_TURN_CEILING_RIGHT90_MODE:
			rmonPrintf("MANTIS_TURN_CEILING_RIGHT90_MODE\r\n") ;
			break ;
		case 	MANTIS_JUMP_CEILING_TO_FLOOR_MODE:
			rmonPrintf("MANTIS_JUMP_CEILING_TO_FLOOR_MODE\r\n") ;
			break ;
		case MANTIS_JUMP_FLOOR_TO_LEFT_WALL_A_MODE:
			rmonPrintf("MANTIS_JUMP_FLOOR_TO_LEFT_WALL_A_MODE\r\n") ;
			break ;
		case MANTIS_JUMP_FLOOR_TO_LEFT_WALL_B_MODE:
			rmonPrintf("MANTIS_JUMP_FLOOR_TO_LEFT_WALL_B_MODE\r\n") ;
			break ;
		case MANTIS_JUMP_FLOOR_TO_LEFT_WALL_C_MODE:
			rmonPrintf("MANTIS_JUMP_FLOOR_TO_LEFT_WALL_C_MODE\r\n") ;
			break ;
		case MANTIS_JUMP_RIGHT_WALL_TO_LEFT_WALL_A_MODE:
			rmonPrintf("MANTIS_JUMP_RIGHT_WALL_TO_LEFT_WALL_A_MODE\r\n") ;
			break ;
		case MANTIS_JUMP_RIGHT_WALL_TO_LEFT_WALL_B_MODE:
			rmonPrintf("MANTIS_JUMP_RIGHT_WALL_TO_LEFT_WALL_B_MODE\r\n") ;
			break ;
		case MANTIS_JUMP_RIGHT_WALL_TO_LEFT_WALL_C_MODE:
			rmonPrintf("MANTIS_JUMP_RIGHT_WALL_TO_LEFT_WALL_C_MODE\r\n") ;
			break ;
		case MANTIS_IDLE_LEFT_WALL_MODE:
			rmonPrintf("MANTIS_IDLE_LEFT_WALL_MODE\r\n") ;
			break ;
		case MANTIS_JUMP_LEFT_WALL_TO_FLOOR_MODE:
			rmonPrintf("MANTIS_JUMP_LEFT_WALL_TO_FLOOR_MODE\r\n") ;
			break ;
		case MANTIS_JUMP_LEFT_WALL_TO_CEILING_MODE:
			rmonPrintf("MANTIS_JUMP_LEFT_WALL_TO_CEILING_MODE\r\n") ;
			break ;
		case MANTIS_SPIT_LEFT_WALL_MODE:
			rmonPrintf("MANTIS_SPIT_LEFT_WALL_MODE\r\n") ;
			break ;
		case MANTIS_SPIT_LEFT_WALL_LOOP_MODE:
			rmonPrintf("MANTIS_SPIT_LEFT_WALL_LOOP_MODE\r\n") ;
			break ;
		case MANTIS_JUMP_FLOOR_TO_RIGHT_WALL_A_MODE:
			rmonPrintf("MANTIS_JUMP_FLOOR_TO_RIGHT_WALL_A_MODE\r\n") ;
			break ;
		case MANTIS_JUMP_FLOOR_TO_RIGHT_WALL_B_MODE:
			rmonPrintf("MANTIS_JUMP_FLOOR_TO_RIGHT_WALL_B_MODE\r\n") ;
			break ;
		case MANTIS_JUMP_FLOOR_TO_RIGHT_WALL_C_MODE:
			rmonPrintf("MANTIS_JUMP_FLOOR_TO_RIGHT_WALL_C_MODE\r\n") ;
			break ;
		case MANTIS_JUMP_LEFT_WALL_TO_RIGHT_WALL_A_MODE:
			rmonPrintf("MANTIS_JUMP_LEFT_WALL_TO_RIGHT_WALL_A_MODE\r\n") ;
			break ;
		case MANTIS_JUMP_LEFT_WALL_TO_RIGHT_WALL_B_MODE:
			rmonPrintf("MANTIS_JUMP_LEFT_WALL_TO_RIGHT_WALL_B_MODE\r\n") ;
			break ;
		case MANTIS_JUMP_LEFT_WALL_TO_RIGHT_WALL_C_MODE:
			rmonPrintf("MANTIS_JUMP_LEFT_WALL_TO_RIGHT_WALL_C_MODE\r\n") ;
			break ;
		case MANTIS_IDLE_RIGHT_WALL_MODE:
			rmonPrintf("MANTIS_IDLE_RIGHT_WALL_MODE\r\n") ;
			break ;
		case MANTIS_JUMP_RIGHT_WALL_TO_FLOOR_MODE:
			rmonPrintf("MANTIS_JUMP_RIGHT_WALL_TO_FLOOR_MODE\r\n") ;
			break ;
		case MANTIS_JUMP_RIGHT_WALL_TO_CEILING_MODE:
			rmonPrintf("MANTIS_JUMP_RIGHT_WALL_TO_CEILING_MODE\r\n") ;
			break ;
		case MANTIS_SPIT_RIGHT_WALL_MODE:
			rmonPrintf("MANTIS_SPIT_RIGHT_WALL_MODE\r\n") ;
			break ;
		case MANTIS_SPIT_RIGHT_WALL_LOOP_MODE:
			rmonPrintf("MANTIS_SPIT_RIGHT_WALL_LOOP_MODE\r\n") ;
			break ;
		case MANTIS_EXPLODE_FLOOR_BACK_MODE:
			rmonPrintf("MANTIS_EXPLODE_FLOOR_BACK_MODE\r\n") ;
			break ;
		case MANTIS_EXPLODE_FLOOR_FRONT_MODE:
			rmonPrintf("MANTIS_EXPLODE_FLOOR_FRONT_MODE\r\n") ;
			break ;
		case MANTIS_EXPLODE_FLOOR_LEFT_MODE:
			rmonPrintf("MANTIS_EXPLODE_FLOOR_LEFT_MODE\r\n") ;
			break ;
		case MANTIS_EXPLODE_FLOOR_RIGHT_MODE:
			rmonPrintf("MANTIS_EXPLODE_FLOOR_RIGHT_MODE\r\n") ;
			break ;
		case MANTIS_EXPLODE_CEILING_MODE:
			rmonPrintf("MANTIS_EXPLODE_CEILING_MODE\r\n") ;
			break ;
		case MANTIS_EXPLODE_LEFT_WALL_MODE:
			rmonPrintf("MANTIS_EXPLODE_LEFT_WALL_MODE\r\n") ;
			break ;
		case MANTIS_EXPLODE_RIGHT_WALL_MODE:
			rmonPrintf("MANTIS_EXPLODE_RIGHT_WALL_MODE\r\n") ;
			break ;
		case MANTIS_DEATH_FLOOR_MODE:
			rmonPrintf("MANTIS_DEATH_FLOOR_MODE\r\n") ;
			break ;

		default:
			rmonPrintf("Mode %d:\r\n", pBoss->m_Mode) ;
			break ;
	}
#endif
}
#endif

