// Campaignerer boss control file by Stephen Broumley

#include "stdafx.h"

#include "ai.h"
#include "aistand.h"

#include "romstruc.h"
#include "tengine.h"
#include "scaling.h"
#include "audio.h"
#include "sfx.h"
#include "tmove.h"
#include "campaign.h"
#include "boss.h"
#include "bossflgs.h"
#include "onscrn.h"
#include "dlists.h"
#include "parttype.h"
#include "sfx.h"

/////////////////////////////////////////////////////////////////////////////
// General constants
/////////////////////////////////////////////////////////////////////////////

#define	DEBUG_CAMPAIGNER					0
#define	CAMPAIGNER_HEALTH					3000
#define	CAMPAIGNER_CHECK_FOR_EVADE		1


/////////////////////////////////////////////////////////////////////////////
// Damage table
/////////////////////////////////////////////////////////////////////////////

// Damage flags
#define CAMP_SHOULDERS_TEX_DAMAGE_FLAG				(1<<0)
#define CAMP_UPPER_ARMS_TEX_DAMAGE_FLAG			(1<<1)
#define CAMP_UPPER_LEGS_TEX_DAMAGE_FLAG			(1<<2)
#define CAMP_TORSO_TEX_DAMAGE_FLAG					(1<<3)
#define CAMP_HEAD_GEOM_DAMAGE_FLAG					(1<<4)
#define CAMP_NECK_GEOM_DAMAGE_FLAG					(1<<5)
#define CAMP_LEFT_UPPER_ARM_GEOM_DAMAGE_FLAG		(1<<6)
#define CAMP_LEFT_LOWER_ARM_GEOM_DAMAGE_FLAG		(1<<7)
#define CAMP_RIGHT_UPPER_LEG_GEOM_DAMAGE_FLAG	(1<<8)

enum CampaignerHeirarchy
{
	CAMP_ROOT_NODE,
	CAMP_L_UP_LEG_NODE,
	CAMP_L_LOW_LEG_NODE,
	CAMP_L_FOOT_NODE,
	CAMP_L_TOES_NODE,
	CAMP_R_UP_LEG_NODE,
	CAMP_R_LOW_LEG_NODE,
	CAMP_R_FOOT_NODE,
	CAMP_R_TOES_NODE,
	CAMP_TAIL_NODE,
	CAMP_TAIL2_NODE,
	CAMP_TORSO3_NODE,
	CAMP_L_SHOULDER_NODE,
	CAMP_L_UP_ARM_NODE,
	CAMP_L_LOW_ARM_NODE,
	CAMP_L_HAND_NODE,
	CAMP_R_SHOULDER_NODE,
	CAMP_R_UP_ARM_NODE,
	CAMP_R_LOW_ARM_NODE,
	CAMP_R_HAND_NODE,
	CAMP_NECK_NODE,
	CAMP_HEAD_NODE,
	CAMP_PAULDRON_NODE,
	CAMP_SHELL_NODE
} ;

CDamage CampDamageTable[] =
{
	// Stage 1 - No damage
	{87.5,		// Health before next stage
	 -1,-1,	// Damaged nodes
	 0},

	// Stage 2 - Shield model swaps
	{75,		// Health before next stage
	 CAMP_L_LOW_ARM_NODE, CAMP_L_HAND_NODE,	// Damaged nodes
	 CAMP_LEFT_UPPER_ARM_GEOM_DAMAGE_FLAG | CAMP_LEFT_LOWER_ARM_GEOM_DAMAGE_FLAG},

	// Stage 3 - Shoulders and upper arm textures swap
	{62.5,	// Health before next stage
	 CAMP_L_SHOULDER_NODE, CAMP_R_SHOULDER_NODE,	// Damaged nodes
	 CAMP_LEFT_UPPER_ARM_GEOM_DAMAGE_FLAG | CAMP_LEFT_LOWER_ARM_GEOM_DAMAGE_FLAG |
	 CAMP_SHOULDERS_TEX_DAMAGE_FLAG | CAMP_UPPER_ARMS_TEX_DAMAGE_FLAG},

	// Stage 4 - Torso textures swap
	{50,	// Health before next stage
	 CAMP_TORSO3_NODE,-1,	// Damaged nodes
	 CAMP_LEFT_UPPER_ARM_GEOM_DAMAGE_FLAG | CAMP_LEFT_LOWER_ARM_GEOM_DAMAGE_FLAG |
	 CAMP_SHOULDERS_TEX_DAMAGE_FLAG | CAMP_UPPER_ARMS_TEX_DAMAGE_FLAG |
	 CAMP_TORSO_TEX_DAMAGE_FLAG},

	// Stage 5 - Upper leg textures swap
	{37.5,	// Health before next stage
	 CAMP_L_UP_LEG_NODE, CAMP_R_UP_LEG_NODE,	// Damaged nodes
	 CAMP_LEFT_UPPER_ARM_GEOM_DAMAGE_FLAG | CAMP_LEFT_LOWER_ARM_GEOM_DAMAGE_FLAG |
	 CAMP_SHOULDERS_TEX_DAMAGE_FLAG | CAMP_UPPER_ARMS_TEX_DAMAGE_FLAG |
	 CAMP_TORSO_TEX_DAMAGE_FLAG |
	 CAMP_UPPER_LEGS_TEX_DAMAGE_FLAG},

	// Stage 6 - Right upper leg model swap
	{25,	// Health before next stage
	 CAMP_R_UP_LEG_NODE,-1,	// Damaged nodes
	 CAMP_LEFT_UPPER_ARM_GEOM_DAMAGE_FLAG | CAMP_LEFT_LOWER_ARM_GEOM_DAMAGE_FLAG |
	 CAMP_SHOULDERS_TEX_DAMAGE_FLAG | CAMP_UPPER_ARMS_TEX_DAMAGE_FLAG |
	 CAMP_TORSO_TEX_DAMAGE_FLAG |
	 CAMP_UPPER_LEGS_TEX_DAMAGE_FLAG |
	 CAMP_RIGHT_UPPER_LEG_GEOM_DAMAGE_FLAG},

	// Stage 7 - Neck model swap
	{12.5,	// Health before next stage
	 CAMP_NECK_NODE,-1,	// Damaged nodes
	 CAMP_LEFT_UPPER_ARM_GEOM_DAMAGE_FLAG | CAMP_LEFT_LOWER_ARM_GEOM_DAMAGE_FLAG |
	 CAMP_SHOULDERS_TEX_DAMAGE_FLAG | CAMP_UPPER_ARMS_TEX_DAMAGE_FLAG |
	 CAMP_TORSO_TEX_DAMAGE_FLAG |
	 CAMP_UPPER_LEGS_TEX_DAMAGE_FLAG |
	 CAMP_RIGHT_UPPER_LEG_GEOM_DAMAGE_FLAG |
	 CAMP_NECK_GEOM_DAMAGE_FLAG},

	// Stage 8 - Head model swap
	{0,	// Health before next stage
	 CAMP_HEAD_NODE,-1,	// Damaged nodes
	 CAMP_LEFT_UPPER_ARM_GEOM_DAMAGE_FLAG | CAMP_LEFT_LOWER_ARM_GEOM_DAMAGE_FLAG |
	 CAMP_SHOULDERS_TEX_DAMAGE_FLAG | CAMP_UPPER_ARMS_TEX_DAMAGE_FLAG |
	 CAMP_TORSO_TEX_DAMAGE_FLAG |
	 CAMP_UPPER_LEGS_TEX_DAMAGE_FLAG |
	 CAMP_RIGHT_UPPER_LEG_GEOM_DAMAGE_FLAG |
	 CAMP_NECK_GEOM_DAMAGE_FLAG |
	 CAMP_HEAD_GEOM_DAMAGE_FLAG},
} ;


// Shield colors
CPsuedoColor CampaignerShieldColors[] =
{
	// White Color		 Alpha	 Black Color
	{255,255,255,		 1.0},		{0,0,0},		// White
	{255,255,25,		 0.83},		{0,0,0},		// Yellow
	{255,0,255,			 0.66},		{0,0,0},		// Purple
	{255,0,0,			 0.5},		{0,0,0},		// Red
} ;


CSelection CampTauntSounds[] =
{
	{SOUND_CAMPAIGNERS_TAUNT_1_PATHETIC, 10},
	{SOUND_CAMPAIGNERS_TAUNT_2_DIE, 10},
	{SOUND_CAMPAIGNERS_RAGE_UNIVERSE, 1},
	{-1}
} ;


/////////////////////////////////////////////////////////////////////////////
// Movement constants
/////////////////////////////////////////////////////////////////////////////
#define	CAMPAIGNER_GRAVITY					GRAVITY_ACCELERATION


#define CAMPAIGNER_RUN_MAX_WEAPON_DIST		(24*SCALING_FACTOR)
#define CAMPAIGNER_RUN_MIN_WEAPON_DIST		(22*SCALING_FACTOR)


#define	CAMPAIGNER_FACING_ANGLE				ANGLE_DTOR(20)
#define	CAMPAIGNER_TURN90_ANGLE				ANGLE_DTOR(20)
#define	CAMPAIGNER_TURN180_ANGLE			ANGLE_DTOR(150)

#define	CAMPAIGNER_WALK_DIST					(14*SCALING_FACTOR)	// Dist before walking
#define	CAMPAIGNER_WALK_FACING_ANGLE		ANGLE_DTOR(25)
#define	CAMPAIGNER_WALK_TURN90_ANGLE		ANGLE_DTOR(35)
#define	CAMPAIGNER_WALK_TURN180_ANGLE		ANGLE_DTOR(150)
#define	CAMPAIGNER_WALK_TURN_SPEED			ANGLE_DTOR(5)

#define	CAMPAIGNER_RUN_DIST					(20*SCALING_FACTOR)	// Dist before running
#define	CAMPAIGNER_RUN_FACING_ANGLE		ANGLE_DTOR(35)
#define	CAMPAIGNER_RUN_TURN90_ANGLE		ANGLE_DTOR(55)
#define	CAMPAIGNER_RUN_TURN180_ANGLE		ANGLE_DTOR(150)
#define	CAMPAIGNER_RUN_TURN_SPEED			ANGLE_DTOR(10)

// Head tracking stuff
#define	CAMPAIGNER_ROTY_OFFSET				ANGLE_DTOR(-20)

#define	CAMPAIGNER_HEIGHT  					(7.6*SCALING_FACTOR)
#define	CAMPAIGNER_HEAD_HEIGHT				(7*SCALING_FACTOR)
#define	CAMPAIGNER_SHOULDER_HEIGHT			(5*SCALING_FACTOR)
#define	CAMPAIGNER_STOMACH_HEIGHT			(4*SCALING_FACTOR)
#define	CAMPAIGNER_LEG_HEIGHT				(3*SCALING_FACTOR)



#define	CAMPAIGNER_TELEPORT_DEST_DIST		(6*SCALING_FACTOR)
#define	CAMPAIGNER_DO_TELEPORT_DIST		(60*SCALING_FACTOR)


#define	CAMPAIGNER_PROJECTILE_ANGLE 		ANGLE_DTOR(10)


#define	CAMPAIGNER_SUMMON_EXPLOSION_DELAY			SECONDS_TO_FRAMES(0.4)
#define	CAMPAIGNER_SUMMON_EXPLOSION_SPREAD_ANGLE	ANGLE_DTOR(12)

#define	CAMPAIGNER_TURN_ON_THE_SPOT_SPEED			ANGLE_DTOR(7)

#define	CAMPAIGNER_MIN_TAUNT_GAP		SECONDS_TO_FRAMES(8)
#define	CAMPAIGNER_MAX_TAUNT_GAP		SECONDS_TO_FRAMES(12)


/////////////////////////////////////////////////////////////////////////////
// Animation constants
/////////////////////////////////////////////////////////////////////////////

// In hole modes
#define AI_ANIM_BOSS_CAMPAIGNER_IN_HOLE_IDLE				AI_ANIM_IDLE
#define AI_ANIM_BOSS_CAMPAIGNER_IN_HOLE_ATTACK			AI_ANIM_ATTACK_STRONG10

// Stand modes
#define AI_ANIM_BOSS_CAMPAIGNER_TALK						AI_ANIM_INTERACTIVE_ANIMATION1
#define AI_ANIM_BOSS_CAMPAIGNER_RAGE						AI_ANIM_INTERACTIVE_ANIMATION2
#define AI_ANIM_BOSS_CAMPAIGNER_VICTORY					AI_ANIM_INTERACTIVE_ANIMATION3

#define AI_ANIM_BOSS_CAMPAIGNER_IDLE						AI_ANIM_IDLE
#define AI_ANIM_BOSS_CAMPAIGNER_TURN180					AI_ANIM_TURN180
#define AI_ANIM_BOSS_CAMPAIGNER_TURN_LEFT90				AI_ANIM_TURNL90
#define AI_ANIM_BOSS_CAMPAIGNER_TURN_RIGHT90				AI_ANIM_TURNR90

// Walk modes
#define AI_ANIM_BOSS_CAMPAIGNER_WALK						AI_ANIM_WALK
#define AI_ANIM_BOSS_CAMPAIGNER_WALK_TURN180				AI_ANIM_WTURN180
#define AI_ANIM_BOSS_CAMPAIGNER_WALK_TURN_LEFT90		AI_ANIM_WTURNL90
#define AI_ANIM_BOSS_CAMPAIGNER_WALK_TURN_RIGHT90		AI_ANIM_WTURNR90

// Limp modes
#define AI_ANIM_BOSS_CAMPAIGNER_LIMP						AI_ANIM_LIMP
#define AI_ANIM_BOSS_CAMPAIGNER_LIMP_TURN180				AI_ANIM_LTURN180
#define AI_ANIM_BOSS_CAMPAIGNER_LIMP_TURN_LEFT90		AI_ANIM_LTURNL90
#define AI_ANIM_BOSS_CAMPAIGNER_LIMP_TURN_RIGHT90		AI_ANIM_LTURNR90

// Run modes
#define AI_ANIM_BOSS_CAMPAIGNER_RUN							AI_ANIM_RUN
#define AI_ANIM_BOSS_CAMPAIGNER_RUN_TURN180				AI_ANIM_RTURN180
#define AI_ANIM_BOSS_CAMPAIGNER_RUN_TURN_LEFT90			AI_ANIM_RTURNL90
#define AI_ANIM_BOSS_CAMPAIGNER_RUN_TURN_RIGHT90		AI_ANIM_RTURNR90

// Normal close up attacks
#define AI_ANIM_BOSS_CAMPAIGNER_RUN_WEAPON_OVERHEAD_SMASH	AI_ANIM_ATTACK_STRONG14
#define AI_ANIM_BOSS_CAMPAIGNER_WEAPON_FOREARM_SLASH			AI_ANIM_ATTACK_STRONG1
#define AI_ANIM_BOSS_CAMPAIGNER_WEAPON_BACKHAND_SLASH			AI_ANIM_ATTACK_STRONG2
#define AI_ANIM_BOSS_CAMPAIGNER_WEAPON_OVERHEAD_SLASH			AI_ANIM_ATTACK_STRONG3
#define AI_ANIM_BOSS_CAMPAIGNER_WEAPON_FOREARM_SLASH2			AI_ANIM_ATTACK_STRONG4
#define AI_ANIM_BOSS_CAMPAIGNER_WEAPON_BACKHAND_SLASH2		AI_ANIM_ATTACK_STRONG5
#define AI_ANIM_BOSS_CAMPAIGNER_WEAPON_OVERHEAD_SLASH2		AI_ANIM_ATTACK_STRONG6

// Normal close up shield attacks
#define AI_ANIM_BOSS_CAMPAIGNER_ATTACK_BACKHAND				AI_ANIM_ATTACK_STRONG7
#define AI_ANIM_BOSS_CAMPAIGNER_ATTACK_SWIPE					AI_ANIM_ATTACK_STRONG8
#define AI_ANIM_BOSS_CAMPAIGNER_ATTACK_BLOW					AI_ANIM_ATTACK_STRONG9

// Magic attacks
#define AI_ANIM_BOSS_CAMPAIGNER_MAGIC_ENERGY_BLAST			AI_ANIM_ATTACK_STRONG10
#define AI_ANIM_BOSS_CAMPAIGNER_MAGIC_SUMMON_EXPLOSION	AI_ANIM_ATTACK_STRONG11
#define AI_ANIM_BOSS_CAMPAIGNER_MAGIC_SUPER_BLAST			AI_ANIM_ATTACK_STRONG12
#define AI_ANIM_BOSS_CAMPAIGNER_MAGIC_SCATTER_BLAST		AI_ANIM_ATTACK_STRONG13

// Specials
#define AI_ANIM_BOSS_CAMPAIGNER_TELEPORT							AI_ANIM_TELEPORT_AWAY

// Weapon
#define AI_ANIM_BOSS_CAMPAIGNER_WEAPON_IDLE						AI_ANIM_IDLE
#define AI_ANIM_BOSS_CAMPAIGNER_WEAPON_TURN180					AI_ANIM_TURN180
#define AI_ANIM_BOSS_CAMPAIGNER_WEAPON_TURN_LEFT90				AI_ANIM_TURNL90
#define AI_ANIM_BOSS_CAMPAIGNER_WEAPON_TURN_RIGHT90 			AI_ANIM_TURNR90
#define AI_ANIM_BOSS_CAMPAIGNER_WEAPON_LAUGH						AI_ANIM_INTERACTIVE_ANIMATION1
#define AI_ANIM_BOSS_CAMPAIGNER_WEAPON_ATTACK					AI_ANIM_ATTACK_STRONG12

// Evades
#define AI_ANIM_BOSS_CAMPAIGNER_EVADE_BACK_FLIP		 			AI_ANIM_EVADE_CROUCH
#define AI_ANIM_BOSS_CAMPAIGNER_EVADE_MEGA_JUMP	 	  			AI_ANIM_EXTRA8

// Flinches
#define AI_ANIM_BOSS_CAMPAIGNER_RIGHT_SHOULDER_FLINCH		 	AI_ANIM_EXTRA1
#define AI_ANIM_BOSS_CAMPAIGNER_LEFT_SHOULDER_FLINCH	  		AI_ANIM_EXTRA2
#define AI_ANIM_BOSS_CAMPAIGNER_CHEST_FLINCH						AI_ANIM_EXTRA3
#define AI_ANIM_BOSS_CAMPAIGNER_BACK_FLINCH						AI_ANIM_EXTRA4
#define AI_ANIM_BOSS_CAMPAIGNER_SHORTING_OUT_FLINCH 			AI_ANIM_EXTRA5
#define AI_ANIM_BOSS_CAMPAIGNER_KNEE_FALL_FLINCH  				AI_ANIM_EXTRA6
#define AI_ANIM_BOSS_CAMPAIGNER_KNEE_GETUP_FLINCH 				AI_ANIM_EXTRA7

// Impact
#define AI_ANIM_BOSS_CAMPAIGNER_EXPLODE_FRONT					AI_ANIM_IMPACT_EXPLODE_FORWARD
#define AI_ANIM_BOSS_CAMPAIGNER_EXPLODE_BACK						AI_ANIM_IMPACT_EXPLODE_BACKWARD
#define AI_ANIM_BOSS_CAMPAIGNER_LEFT								AI_ANIM_IMPACT_EXPLODE_LEFT
#define AI_ANIM_BOSS_CAMPAIGNER_RIGHT								AI_ANIM_IMPACT_EXPLODE_RIGHT
#define AI_ANIM_BOSS_CAMPAIGNER_DEATH								AI_ANIM_DEATH_NORMAL

// Cinemas
#define AI_ANIM_BOSS_CAMPAIGNER_KILL_TUROK						AI_ANIM_INTERACTIVE_ANIMATION8


/////////////////////////////////////////////////////////////////////////////
// Function prototypes
/////////////////////////////////////////////////////////////////////////////


// Initialisation
void CCollisionInfo__SetCampaignerCollisionDefaults(CCollisionInfo2 *pThis) ;
void Campaigner_SetupStageInfo(CGameObjectInstance *pThis, CCampaigner *pCampaigner, CCampaignerStageInfo *Info) ;


// General code
void Campaigner_PreDraw(CGameObjectInstance *pThis, CCampaigner *pCampaigner, Gfx **ppDLP) ;
void Campaigner_PostDraw(CGameObjectInstance *pThis, CCampaigner *pCampaigner, Gfx **ppDLP) ;

void Campaigner_Update(CGameObjectInstance *pThis, CCampaigner *pCampaigner) ;
INT32 Campaigner_SetupMode(CGameObjectInstance *pThis, INT32 nNewMode) ;
void Campaigner_GetTarget(CGameObjectInstance *pThis, CCampaigner *pCampaigner) ;

void Campaigner_PerformTauntAttack(CGameObjectInstance *pThis, CCampaigner *pCampaigner) ;
void Campaigner_PerformRunAttack(CGameObjectInstance *pThis, CCampaigner *pCampaigner) ;
void Campaigner_PerformCloseAttack(CGameObjectInstance *pThis, CCampaigner *pCampaigner) ;

BOOL Campaigner_CheckForRunAttack(CGameObjectInstance *pThis, CCampaigner *pCampaigner) ;

void Campaigner_CheckForEvade(CGameObjectInstance *pThis, CCampaigner *pCampaigner) ;
void Campaigner_HitByParticle(CGameObjectInstance *pThis, CCampaigner *pCampaigner, CParticle *pParticle) ;


// Mode code
void Campaigner_PositionInHole(CGameObjectInstance *pThis, CCampaigner *pCampaigner) ;

void Campaigner_Setup_IN_HOLE_IDLE(CGameObjectInstance *pThis, CCampaigner *pCampaigner) ;
void Campaigner_Code_IN_HOLE_IDLE(CGameObjectInstance *pThis, CCampaigner *pCampaigner) ;

void Campaigner_Setup_IN_HOLE_ATTACK(CGameObjectInstance *pThis, CCampaigner *pCampaigner) ;
void Campaigner_Code_IN_HOLE_ATTACK(CGameObjectInstance *pThis, CCampaigner *pCampaigner) ;

void Campaigner_Code_IDLE(CGameObjectInstance *pThis, CCampaigner *pCampaigner) ;
void Campaigner_Code_Turn(CGameObjectInstance *pThis, CCampaigner *pCampaigner) ;

void Campaigner_Code_WALK(CGameObjectInstance *pThis, CCampaigner *pCampaigner) ;
void Campaigner_Code_WalkTurn(CGameObjectInstance *pThis, CCampaigner *pCampaigner) ;

void Campaigner_Code_RUN(CGameObjectInstance *pThis, CCampaigner *pCampaigner) ;
void Campaigner_Code_RunTurn(CGameObjectInstance *pThis, CCampaigner *pCampaigner) ;

void Campaigner_Code_CloseAttack(CGameObjectInstance *pThis, CCampaigner *pCampaigner) ;
void Campaigner_Code_RunAttack(CGameObjectInstance *pThis, CCampaigner *pCampaigner) ;

void Campaigner_Setup_MAGIC_SUMMON_EXPLOSION(CGameObjectInstance *pThis, CCampaigner *pCampaigner) ;
void Campaigner_Code_MAGIC_SUMMON_EXPLOSION(CGameObjectInstance *pThis, CCampaigner *pCampaigner) ;

void Campaigner_Setup_TELEPORT(CGameObjectInstance *pThis, CCampaigner *pCampaigner) ;
void Campaigner_Code_TELEPORT(CGameObjectInstance *pThis, CCampaigner *pCampaigner) ;

void Campaigner_Setup_WEAPON_IDLE(CGameObjectInstance *pThis, CCampaigner *pCampaigner) ;
void Campaigner_Code_WEAPON_IDLE(CGameObjectInstance *pThis, CCampaigner *pCampaigner) ;
void Campaigner_Setup_WEAPON_LAUGH(CGameObjectInstance *pThis, CCampaigner *pCampaigner) ;
void Campaigner_Code_WEAPON_LAUGH(CGameObjectInstance *pThis, CCampaigner *pCampaigner) ;
void Campaigner_Setup_WEAPON_ATTACK(CGameObjectInstance *pThis, CCampaigner *pCampaigner) ;
void Campaigner_Code_WEAPON_ATTACK(CGameObjectInstance *pThis, CCampaigner *pCampaigner) ;

void Campaigner_Setup_MAGIC_ENERGY_BLAST(CGameObjectInstance *pThis, CCampaigner *pCampaigner) ;
void Campaigner_Setup_MAGIC_SUPER_BLAST(CGameObjectInstance *pThis, CCampaigner *pCampaigner) ;
void Campaigner_Setup_MAGIC_SCATTER_BLAST(CGameObjectInstance *pThis, CCampaigner *pCampaigner) ;

void Campaigner_Code_Evade(CGameObjectInstance *pThis, CCampaigner *pCampaigner) ;
void Campaigner_Code_MegaJump(CGameObjectInstance *pThis, CCampaigner *pCampaigner) ;

void Campaigner_Code_Flinch(CGameObjectInstance *pThis, CCampaigner *pCampaigner) ;

void Campaigner_Setup_DAMAGE_SHORTING_OUT(CGameObjectInstance *pThis, CCampaigner *pCampaigner) ;
void Campaigner_Code_DAMAGE_SHORTING_OUT(CGameObjectInstance *pThis, CCampaigner *pCampaigner) ;

void Campaigner_Setup_Explode(CGameObjectInstance *pThis, CCampaigner *pCampaigner) ;
void Campaigner_Code_Explode(CGameObjectInstance *pThis, CCampaigner *pCampaigner) ;

void Campaigner_Setup_DEATH(CGameObjectInstance *pThis, CCampaigner *pCampaigner) ;
void Campaigner_Code_DEATH(CGameObjectInstance *pThis, CCampaigner *pCampaigner) ;

void Campaigner_Code_Movement(CGameObjectInstance *pThis, CCampaigner *pCampaigner) ;
void Campaigner_Setup_KILL_TUROK(CGameObjectInstance *pThis, CCampaigner *pCampaigner) ;


void Campaigner_ActivateShield(CGameObjectInstance *pThis, CCampaigner *pCampaigner, FLOAT Time) ;
void Campaigner_DeactivateShield(CGameObjectInstance *pThis, CCampaigner *pCampaigner) ;


#ifndef MAKE_CART
void Campaigner_DisplayMode(CBoss *pBoss) ;
#endif


/////////////////////////////////////////////////////////////////////////////
// Variables
/////////////////////////////////////////////////////////////////////////////

// Campaigner itself
CCampaigner	CampaignerBoss ;



/////////////////////////////////////////////////////////////////////////////
//
// The Top Level AI Mode Table
//
/////////////////////////////////////////////////////////////////////////////
CBossModeInfo	CampaignerModeTable[] =
{
	// In hole mode
	BOSS_MODE_INFO(Campaigner_Setup_IN_HOLE_IDLE,	Campaigner_Code_IN_HOLE_IDLE,		AI_ANIM_BOSS_CAMPAIGNER_IN_HOLE_IDLE,		1, BF_HEAD_TRACK_TUROK),
	BOSS_MODE_INFO(Campaigner_Setup_IN_HOLE_ATTACK,	Campaigner_Code_IN_HOLE_ATTACK,	AI_ANIM_BOSS_CAMPAIGNER_IN_HOLE_ATTACK,	1, BF_HEAD_TRACK_TUROK),

	// Taunt modes
	BOSS_MODE_INFO(NULL,		Campaigner_Code_CloseAttack,		AI_ANIM_BOSS_CAMPAIGNER_TALK,		1, BF_TAUNT_MODE | BF_HEAD_TRACK_TUROK | BF_RUN_AT_MODE_ANIM_SPEED),
	BOSS_MODE_INFO(NULL,		Campaigner_Code_CloseAttack,		AI_ANIM_BOSS_CAMPAIGNER_RAGE,		1, BF_TAUNT_MODE | BF_HEAD_TRACK_TUROK | BF_RUN_AT_MODE_ANIM_SPEED),
	BOSS_MODE_INFO(NULL,		Campaigner_Code_CloseAttack,		AI_ANIM_BOSS_CAMPAIGNER_VICTORY,	1, BF_TAUNT_MODE | BF_HEAD_TRACK_TUROK | BF_RUN_AT_MODE_ANIM_SPEED),

	// Stand modes
	BOSS_MODE_INFO(Campaigner_Code_Movement,	Campaigner_Code_Movement,	AI_ANIM_BOSS_CAMPAIGNER_IDLE,			1, BF_CAN_EVADE | BF_CAN_FLINCH | BF_CAN_EXPLODE_FLINCH | BF_CAN_TELEPORT | BF_HEAD_TRACK_TUROK),
	BOSS_MODE_INFO(NULL,		Campaigner_Code_Movement,		AI_ANIM_BOSS_CAMPAIGNER_TURN180,		1,BF_CAN_EVADE | BF_CAN_FLINCH | BF_CAN_TELEPORT | BF_HEAD_TRACK_TUROK),
	BOSS_MODE_INFO(NULL,		Campaigner_Code_Movement,		AI_ANIM_BOSS_CAMPAIGNER_TURN_LEFT90,	1,BF_CAN_EVADE | BF_CAN_FLINCH | BF_CAN_EXPLODE_FLINCH | BF_CAN_TELEPORT | BF_HEAD_TRACK_TUROK),
	BOSS_MODE_INFO(NULL,		Campaigner_Code_Movement,		AI_ANIM_BOSS_CAMPAIGNER_TURN_RIGHT90,	1,BF_CAN_EVADE | BF_CAN_FLINCH | BF_CAN_EXPLODE_FLINCH | BF_CAN_TELEPORT | BF_HEAD_TRACK_TUROK),

	// Walk modes
	BOSS_MODE_INFO(Campaigner_Code_Movement,	Campaigner_Code_Movement,		AI_ANIM_BOSS_CAMPAIGNER_WALK,		1,BF_CAN_EVADE | BF_CAN_FLINCH | BF_CAN_EXPLODE_FLINCH | BF_RUNNING_MODE | BF_CAN_TELEPORT | BF_HEAD_TRACK_TUROK),
	BOSS_MODE_INFO(NULL,		Campaigner_Code_Movement,		AI_ANIM_BOSS_CAMPAIGNER_WALK_TURN180,			1,BF_CAN_EVADE | BF_CAN_FLINCH | BF_CAN_EXPLODE_FLINCH | BF_RUNNING_MODE | BF_CAN_TELEPORT | BF_HEAD_TRACK_TUROK),
	BOSS_MODE_INFO(NULL,		Campaigner_Code_Movement,		AI_ANIM_BOSS_CAMPAIGNER_WALK_TURN_LEFT90,		1,BF_CAN_EVADE | BF_CAN_FLINCH | BF_CAN_EXPLODE_FLINCH | BF_RUNNING_MODE | BF_CAN_TELEPORT | BF_HEAD_TRACK_TUROK),
	BOSS_MODE_INFO(NULL,		Campaigner_Code_Movement,		AI_ANIM_BOSS_CAMPAIGNER_WALK_TURN_RIGHT90,	1,BF_CAN_EVADE | BF_CAN_FLINCH | BF_CAN_EXPLODE_FLINCH | BF_RUNNING_MODE | BF_CAN_TELEPORT | BF_HEAD_TRACK_TUROK),

	// Limp modes
	BOSS_MODE_INFO(Campaigner_Code_Movement,	Campaigner_Code_Movement,		AI_ANIM_BOSS_CAMPAIGNER_LIMP,	  	1,BF_CAN_EVADE | BF_CAN_FLINCH | BF_CAN_EXPLODE_FLINCH | BF_RUNNING_MODE | BF_CAN_TELEPORT | BF_HEAD_TRACK_TUROK),
	BOSS_MODE_INFO(NULL,		Campaigner_Code_Movement,		AI_ANIM_BOSS_CAMPAIGNER_LIMP_TURN180,			1,BF_CAN_EVADE | BF_CAN_FLINCH | BF_CAN_EXPLODE_FLINCH | BF_RUNNING_MODE | BF_CAN_TELEPORT | BF_HEAD_TRACK_TUROK),
	BOSS_MODE_INFO(NULL,		Campaigner_Code_Movement,		AI_ANIM_BOSS_CAMPAIGNER_LIMP_TURN_LEFT90,		1,BF_CAN_EVADE | BF_CAN_FLINCH | BF_CAN_EXPLODE_FLINCH | BF_RUNNING_MODE | BF_CAN_TELEPORT | BF_HEAD_TRACK_TUROK),
	BOSS_MODE_INFO(NULL,		Campaigner_Code_Movement,		AI_ANIM_BOSS_CAMPAIGNER_LIMP_TURN_RIGHT90,	1,BF_CAN_EVADE | BF_CAN_FLINCH | BF_CAN_EXPLODE_FLINCH | BF_RUNNING_MODE | BF_CAN_TELEPORT | BF_HEAD_TRACK_TUROK),

	// Run modes
	BOSS_MODE_INFO(Campaigner_Code_Movement,		Campaigner_Code_Movement,		AI_ANIM_BOSS_CAMPAIGNER_RUN,	1.0,BF_CAN_EVADE | BF_CAN_FLINCH | BF_CAN_EXPLODE_FLINCH | BF_RUNNING_MODE | BF_CAN_TELEPORT | BF_HEAD_TRACK_TUROK),
	BOSS_MODE_INFO(NULL,		Campaigner_Code_Movement,		AI_ANIM_BOSS_CAMPAIGNER_RUN_TURN180,					1.0,BF_CAN_EVADE | BF_CAN_FLINCH | BF_CAN_EXPLODE_FLINCH | BF_RUNNING_MODE | BF_CAN_TELEPORT | BF_HEAD_TRACK_TUROK),
	BOSS_MODE_INFO(NULL,		Campaigner_Code_Movement,		AI_ANIM_BOSS_CAMPAIGNER_RUN_TURN_LEFT90,				1.0,BF_CAN_EVADE | BF_CAN_FLINCH | BF_CAN_EXPLODE_FLINCH | BF_RUNNING_MODE | BF_CAN_TELEPORT | BF_HEAD_TRACK_TUROK),
	BOSS_MODE_INFO(NULL,		Campaigner_Code_Movement,		AI_ANIM_BOSS_CAMPAIGNER_RUN_TURN_RIGHT90,				1.0,BF_CAN_EVADE | BF_CAN_FLINCH | BF_CAN_EXPLODE_FLINCH | BF_RUNNING_MODE | BF_CAN_TELEPORT | BF_HEAD_TRACK_TUROK),

	// Normal close up attacks
	BOSS_MODE_INFO(NULL,		Campaigner_Code_CloseAttack,		AI_ANIM_BOSS_CAMPAIGNER_RUN_WEAPON_OVERHEAD_SMASH,	1,	0 | BF_HEAD_TRACK_TUROK | BF_PUNCH_IMPACT_MODE),
	BOSS_MODE_INFO(NULL,		Campaigner_Code_CloseAttack,		AI_ANIM_BOSS_CAMPAIGNER_WEAPON_FOREARM_SLASH,		1,	0 | BF_HEAD_TRACK_TUROK | BF_PUNCH_IMPACT_MODE),
	BOSS_MODE_INFO(NULL,		Campaigner_Code_CloseAttack,		AI_ANIM_BOSS_CAMPAIGNER_WEAPON_BACKHAND_SLASH,		1,	0 | BF_HEAD_TRACK_TUROK | BF_PUNCH_IMPACT_MODE),
	BOSS_MODE_INFO(NULL,		Campaigner_Code_CloseAttack,		AI_ANIM_BOSS_CAMPAIGNER_WEAPON_OVERHEAD_SLASH,		1,	0 | BF_HEAD_TRACK_TUROK | BF_PUNCH_IMPACT_MODE),
	BOSS_MODE_INFO(NULL,		Campaigner_Code_CloseAttack,		AI_ANIM_BOSS_CAMPAIGNER_WEAPON_FOREARM_SLASH2,		1,	0 | BF_HEAD_TRACK_TUROK | BF_PUNCH_IMPACT_MODE),
	BOSS_MODE_INFO(NULL,		Campaigner_Code_CloseAttack,		AI_ANIM_BOSS_CAMPAIGNER_WEAPON_BACKHAND_SLASH2,		1,	0 | BF_HEAD_TRACK_TUROK | BF_PUNCH_IMPACT_MODE),
	BOSS_MODE_INFO(NULL,		Campaigner_Code_CloseAttack,		AI_ANIM_BOSS_CAMPAIGNER_WEAPON_OVERHEAD_SLASH2,		1,	0 | BF_HEAD_TRACK_TUROK | BF_PUNCH_IMPACT_MODE),

	// Normal close up shield attacks
	BOSS_MODE_INFO(NULL,		Campaigner_Code_CloseAttack,		AI_ANIM_BOSS_CAMPAIGNER_ATTACK_BACKHAND,	1,	0 | BF_HEAD_TRACK_TUROK | BF_PUNCH_IMPACT_MODE),
	BOSS_MODE_INFO(NULL,		Campaigner_Code_CloseAttack,		AI_ANIM_BOSS_CAMPAIGNER_ATTACK_SWIPE,		1,	0 | BF_HEAD_TRACK_TUROK | BF_PUNCH_IMPACT_MODE),
	BOSS_MODE_INFO(NULL,		Campaigner_Code_CloseAttack,		AI_ANIM_BOSS_CAMPAIGNER_ATTACK_BLOW,		1,	0 | BF_HEAD_TRACK_TUROK | BF_PUNCH_IMPACT_MODE),

	// Magic attacks
	BOSS_MODE_INFO(Campaigner_Setup_MAGIC_ENERGY_BLAST,  		Campaigner_Code_RunAttack,			AI_ANIM_BOSS_CAMPAIGNER_MAGIC_ENERGY_BLAST,			1,	0),
	BOSS_MODE_INFO(Campaigner_Setup_MAGIC_SUMMON_EXPLOSION,	Campaigner_Code_MAGIC_SUMMON_EXPLOSION,		AI_ANIM_BOSS_CAMPAIGNER_MAGIC_SUMMON_EXPLOSION,		1,	0 | BF_HEAD_TRACK_TUROK),
	BOSS_MODE_INFO(Campaigner_Setup_MAGIC_SUPER_BLAST,			Campaigner_Code_RunAttack,			AI_ANIM_BOSS_CAMPAIGNER_MAGIC_SUPER_BLAST,			1,	0),
	BOSS_MODE_INFO(Campaigner_Setup_MAGIC_SCATTER_BLAST,		Campaigner_Code_RunAttack,			AI_ANIM_BOSS_CAMPAIGNER_MAGIC_SCATTER_BLAST,			1,	0),

	// Specials
	BOSS_MODE_INFO(Campaigner_Setup_TELEPORT,	Campaigner_Code_TELEPORT,	AI_ANIM_BOSS_CAMPAIGNER_TELEPORT,	2,	BF_INVINCIBLE | BF_RUN_AT_MODE_ANIM_SPEED),

	// Weapon modes
	BOSS_MODE_INFO(Campaigner_Setup_WEAPON_IDLE,	Campaigner_Code_WEAPON_IDLE,		AI_ANIM_BOSS_CAMPAIGNER_WEAPON_IDLE,			1,	BF_INVINCIBLE),
	BOSS_MODE_INFO(NULL,									Campaigner_Code_WEAPON_IDLE,		AI_ANIM_BOSS_CAMPAIGNER_WEAPON_TURN180,		1,	BF_INVINCIBLE),
	BOSS_MODE_INFO(NULL,									Campaigner_Code_WEAPON_IDLE,		AI_ANIM_BOSS_CAMPAIGNER_WEAPON_TURN_LEFT90,	1,	BF_INVINCIBLE),
	BOSS_MODE_INFO(NULL,									Campaigner_Code_WEAPON_IDLE,		AI_ANIM_BOSS_CAMPAIGNER_WEAPON_TURN_RIGHT90,	1,	BF_INVINCIBLE),
	BOSS_MODE_INFO(Campaigner_Setup_WEAPON_LAUGH,	Campaigner_Code_WEAPON_LAUGH,		AI_ANIM_BOSS_CAMPAIGNER_WEAPON_LAUGH,		1,	BF_INVINCIBLE),
	BOSS_MODE_INFO(Campaigner_Setup_WEAPON_ATTACK,	Campaigner_Code_WEAPON_ATTACK,	AI_ANIM_BOSS_CAMPAIGNER_WEAPON_ATTACK,		1,	BF_INVINCIBLE),

	// Evades
	BOSS_MODE_INFO(NULL,		Campaigner_Code_Evade, 		AI_ANIM_BOSS_CAMPAIGNER_EVADE_BACK_FLIP,	1.5, BF_HEAD_TRACK_TUROK),
	BOSS_MODE_INFO(NULL,		Campaigner_Code_MegaJump,	AI_ANIM_BOSS_CAMPAIGNER_EVADE_MEGA_JUMP,	1,	BF_HEAD_TRACK_TUROK),

	// Flinches
	BOSS_MODE_INFO(NULL,		Campaigner_Code_Flinch,			AI_ANIM_BOSS_CAMPAIGNER_RIGHT_SHOULDER_FLINCH	,1,	0 | BF_CAN_EXPLODE_FLINCH),
	BOSS_MODE_INFO(NULL,		Campaigner_Code_Flinch,			AI_ANIM_BOSS_CAMPAIGNER_LEFT_SHOULDER_FLINCH	  	,1,	0 | BF_CAN_EXPLODE_FLINCH),
	BOSS_MODE_INFO(NULL,		Campaigner_Code_Flinch,			AI_ANIM_BOSS_CAMPAIGNER_CHEST_FLINCH				,1,	0 | BF_CAN_EXPLODE_FLINCH),
	BOSS_MODE_INFO(NULL,		Campaigner_Code_Flinch,			AI_ANIM_BOSS_CAMPAIGNER_BACK_FLINCH					,1,	0 | BF_CAN_EXPLODE_FLINCH),
	BOSS_MODE_INFO(Campaigner_Setup_DAMAGE_SHORTING_OUT,	Campaigner_Code_DAMAGE_SHORTING_OUT,	AI_ANIM_BOSS_CAMPAIGNER_SHORTING_OUT_FLINCH 		,2,	0 | BF_RUN_AT_MODE_ANIM_SPEED),
	BOSS_MODE_INFO(NULL,		BOSS_Code_NextModeAfterAnim,	AI_ANIM_BOSS_CAMPAIGNER_KNEE_FALL_FLINCH  		,1,	0 | BF_CAN_EXPLODE_FLINCH),
	BOSS_MODE_INFO(NULL,		Campaigner_Code_Flinch,			AI_ANIM_BOSS_CAMPAIGNER_KNEE_GETUP_FLINCH 		,1,	0 | BF_CAN_EXPLODE_FLINCH),

	// Impacts
	BOSS_MODE_INFO(Campaigner_Setup_Explode,		Campaigner_Code_Explode,		AI_ANIM_BOSS_CAMPAIGNER_EXPLODE_FRONT,		1, BF_EXPLODE_MODE),
	BOSS_MODE_INFO(NULL,		Campaigner_Code_Explode,		AI_ANIM_BOSS_CAMPAIGNER_EXPLODE_BACK,		1, BF_EXPLODE_MODE),
	BOSS_MODE_INFO(NULL,		Campaigner_Code_Explode,		AI_ANIM_BOSS_CAMPAIGNER_LEFT,					1, BF_EXPLODE_MODE),
	BOSS_MODE_INFO(NULL,		Campaigner_Code_Explode,		AI_ANIM_BOSS_CAMPAIGNER_RIGHT,				1, BF_EXPLODE_MODE),

	// Cinemas
	BOSS_MODE_INFO(Campaigner_Setup_DEATH,	Campaigner_Code_DEATH,	AI_ANIM_BOSS_CAMPAIGNER_DEATH,	0.9, BF_INVINCIBLE | BF_DEATH_MODE | BF_RUN_AT_MODE_ANIM_SPEED | BF_CLEAR_VELS),
	BOSS_MODE_INFO(Campaigner_Setup_KILL_TUROK,	NULL,			AI_ANIM_BOSS_CAMPAIGNER_KILL_TUROK,		1,	BF_INVINCIBLE | BF_RUN_AT_MODE_ANIM_SPEED | BF_CLEAR_VELS),

	{NULL}
} ;






/////////////////////////////////////////////////////////////////////////////
//
// The big stage table
//
/////////////////////////////////////////////////////////////////////////////
CCampaignerStageInfo CampaignerStageInfo[] =
{
	//	STAGE 1
	{75,								// Health limit for this stage
	 1.5,								// Anim speed
	 SECONDS_TO_FRAMES(3),		// Run time before attack

	 60,								// Evade percentage
	 SECONDS_TO_FRAMES(4),		// Evade spacing

	 100,								// Flinch percentage
	 SECONDS_TO_FRAMES(4),		// Flinch spacing

	 100,								//	Explode flinch percentage ;
	 SECONDS_TO_FRAMES(4),		// Explode flinch spacing

	 50,								// Short out percentage (when flinching)
	 SECONDS_TO_FRAMES(4),		// Short out spacing

	 100,								// Block weapons percentage
	},

	//	STAGE 2
	{50,			  					// Health limit for this stage
	 1.55,		  					// Anim speed
	 SECONDS_TO_FRAMES(2.5),	// Run time before attack

	 50,								// Evade percentage
	 SECONDS_TO_FRAMES(5),		// Evade spacing

	 90,								// Flinch percentage
	 SECONDS_TO_FRAMES(5),		// Flinch spacing

	 90,								//	Explode flinch percentage ;
	 SECONDS_TO_FRAMES(5),		// Explode flinch spacing

	 90,								// Short out percentage (when flinching)
	 SECONDS_TO_FRAMES(5),		// Short out spacing

	 75,								// Block weapons percentage
	},

	//	STAGE 3
	{25,								// Health limit for this stage
	 1.6, 				  			// Anim speed
	 SECONDS_TO_FRAMES(2.0),	// Run time before attack

	 40,								// Evade percentage
	 SECONDS_TO_FRAMES(6),		// Evade spacing

	 80,								// Flinch percentage
	 SECONDS_TO_FRAMES(6),		// Flinch spacing

	 80,								//	Explode flinch percentage ;
	 SECONDS_TO_FRAMES(6),		// Explode flinch spacing

	 80,								// Short out percentage (when flinching)
	 SECONDS_TO_FRAMES(6),		// Short out spacing

	 50,								// Block weapons percentage
	},

	//	STAGE 4
	{0,								// Health limit for this stage
	 1.65,				  			// Anim speed
	 SECONDS_TO_FRAMES(1.5),	// Run time before attack

	 30,								// Evade percentage
	 SECONDS_TO_FRAMES(7),		// Evade spacing

	 70,								// Flinch percentage
	 SECONDS_TO_FRAMES(7),		// Flinch spacing

	 70,								//	Explode flinch percentage ;
	 SECONDS_TO_FRAMES(7),		// Explode flinch spacing

	 70,								// Short out percentage (when flinching)
	 SECONDS_TO_FRAMES(7),		// Short out spacing

	 10,								// Block weapons percentage
	},
} ;


/////////////////////////////////////////////////////////////////////////////
// Info tables
/////////////////////////////////////////////////////////////////////////////


// Flinch info structure
CFlinchInfo	CampaignerFlinchInfo =
{
	CAMPAIGNER_ROTY_OFFSET,			// Rotation direction offset

	CAMPAIGNER_HEAD_HEIGHT,			// Bottom of head
	CAMPAIGNER_SHOULDER_HEIGHT,	// Bottom of shoulders
	CAMPAIGNER_STOMACH_HEIGHT,		// Bottom of stomach

	CAMPAIGNER_CHEST_FLINCH_MODE,					// Head flinch mode
	CAMPAIGNER_BACK_FLINCH_MODE,					// Back flinch mode
	CAMPAIGNER_CHEST_FLINCH_MODE,					// Stomach flinch mode
	CAMPAIGNER_KNEE_FALL_FLINCH_MODE,  			// Knee flinch mode
	CAMPAIGNER_LEFT_SHOULDER_FLINCH_MODE,		// Left shoulder flinch mode
	CAMPAIGNER_RIGHT_SHOULDER_FLINCH_MODE,		// Right shoulder flinch mode

} ;


/////////////////////////////////////////////////////////////////////////////
// Attack tables
/////////////////////////////////////////////////////////////////////////////


// Taunt attacks
CSelection	CampaignerTauntAttackSelection[] =
{
	{CAMPAIGNER_TALK_MODE,  	10},
	{CAMPAIGNER_RAGE_MODE,		10},
	{CAMPAIGNER_VICTORY_MODE, 	10},
	{-1},
} ;



// Close attacks
CSelection	CampaignerCloseAttackSelection[] =
{
	{CAMPAIGNER_WEAPON_FOREARM_SLASH_MODE,		10},
	{CAMPAIGNER_WEAPON_BACKHAND_SLASH_MODE,  	10},
	{CAMPAIGNER_WEAPON_OVERHEAD_SLASH_MODE,  	10},
	{CAMPAIGNER_WEAPON_FOREARM_SLASH2_MODE,  	10},
	{CAMPAIGNER_WEAPON_BACKHAND_SLASH2_MODE,	10},
	{CAMPAIGNER_WEAPON_OVERHEAD_SLASH2_MODE,	10},
	{CAMPAIGNER_ATTACK_BACKHAND_MODE,			10},
	{CAMPAIGNER_ATTACK_SWIPE_MODE,				10},
	{CAMPAIGNER_ATTACK_BLOW_MODE,					10},
	{-1},
} ;



// Magic attacks
INT16 CampaignerRunAttackScript[] =
{
	CAMPAIGNER_TELEPORT_MODE,
	CAMPAIGNER_MAGIC_SUMMON_EXPLOSION_MODE,
	CAMPAIGNER_MAGIC_SUPER_BLAST_MODE,
	CAMPAIGNER_EVADE_MEGA_JUMP_MODE,
	CAMPAIGNER_MAGIC_ENERGY_BLAST_MODE,
	CAMPAIGNER_MAGIC_SCATTER_BLAST_MODE,
	0
} ;



/////////////////////////////////////////////////////////////////////////////
// Special fx scripts
/////////////////////////////////////////////////////////////////////////////

// "TELEPORT_MODE" - flash yellow to white, then vanish
DEFINE_SPECIALFX_SCRIPT(CampaignerFxTeleportOut)
	SPECIALFX_SCRIPT_FADE_COLOR(255,255,10, MAX_COL, SECONDS_TO_FRAMES(0.1))
	SPECIALFX_SCRIPT_FADE_COLOR(255,255,255, MAX_COL, SECONDS_TO_FRAMES(0.1))
	SPECIALFX_SCRIPT_FADE_TRANS(MAX_TRANS, SECONDS_TO_FRAMES(0.1))
END_SPECIALFX_SCRIPT

// "TELEPORT_MODE" - appear, flash white to yellow
DEFINE_SPECIALFX_SCRIPT(CampaignerFxTeleportIn)
	SPECIALFX_SCRIPT_FADE_TRANS(MIN_TRANS, SECONDS_TO_FRAMES(0.1))
	SPECIALFX_SCRIPT_FADE_COLOR(255,255,10, MAX_COL, SECONDS_TO_FRAMES(0.1))
	SPECIALFX_SCRIPT_FADE_COLOR(255,255,10, MIN_COL, SECONDS_TO_FRAMES(0.1))
END_SPECIALFX_SCRIPT


// "MAGIC_SUMMON_EXPLOSION_MODE" - glowing firey orange
DEFINE_SPECIALFX_SCRIPT(CampaignerFxExplosionStart)
	SPECIALFX_SCRIPT_FADE_COLOR(255,250,10, MAX_COL/4, SECONDS_TO_FRAMES(0.5))
	SPECIALFX_SCRIPT_FADE_COLOR(255,250,10, MIN_COL, SECONDS_TO_FRAMES(0.5))
	SPECIALFX_SCRIPT_REPEAT
END_SPECIALFX_SCRIPT

// "MAGIC_SUMMON_EXPLOSION_MODE" - end of glowing
DEFINE_SPECIALFX_SCRIPT(CampaignerFxExplosionEnd)
	SPECIALFX_SCRIPT_FADE_COLOR(255,50,0, MIN_COL, SECONDS_TO_FRAMES(0.5))
END_SPECIALFX_SCRIPT


// "MAGIC_SUPER_BLAST_MODE" - Glowing purple before firing weapon, then white flash
DEFINE_SPECIALFX_SCRIPT(CampaignerFxSuperBlast)
	SPECIALFX_SCRIPT_WAIT(SECONDS_TO_FRAMES(1))
	SPECIALFX_SCRIPT_FADE_COLOR(255,100,255, MAX_COL/8, SECONDS_TO_FRAMES(1))
	SPECIALFX_SCRIPT_FADE_COLOR(255,255,255, MAX_COL/2, SECONDS_TO_FRAMES(0.1))
	SPECIALFX_SCRIPT_FADE_COLOR(255,0,255, MIN_COL, SECONDS_TO_FRAMES(0.2))
END_SPECIALFX_SCRIPT


// "MAGIC_SUPER_BLAST_MODE" - Glowing green during scatter
DEFINE_SPECIALFX_SCRIPT(CampaignerFxScatterBlast)
	SPECIALFX_SCRIPT_WAIT(SECONDS_TO_FRAMES(0.5))
	SPECIALFX_SCRIPT_FADE_COLOR(100,255,100, MAX_COL/8, SECONDS_TO_FRAMES(0.25))
	SPECIALFX_SCRIPT_FADE_COLOR(250,255,250, MAX_COL/4, SECONDS_TO_FRAMES(0.25))
	SPECIALFX_SCRIPT_FADE_COLOR(100,255,100, MIN_COL, SECONDS_TO_FRAMES(0.5))
END_SPECIALFX_SCRIPT


// "MAGIC_ENERGY_BLAST_MODE" - Glowing blue during blue - flame throwing
DEFINE_SPECIALFX_SCRIPT(CampaignerFxEnergyBlast)
	SPECIALFX_SCRIPT_WAIT(SECONDS_TO_FRAMES(0.5))

	SPECIALFX_SCRIPT_FADE_COLOR(200,200,255, MAX_COL/4, SECONDS_TO_FRAMES(0.5))
	SPECIALFX_SCRIPT_FADE_COLOR(200,200,255, MIN_COL, SECONDS_TO_FRAMES(0.5))
	SPECIALFX_SCRIPT_WAIT(SECONDS_TO_FRAMES(0.5))

	SPECIALFX_SCRIPT_FADE_COLOR(200,200,255, MAX_COL/4, SECONDS_TO_FRAMES(0.4))
	SPECIALFX_SCRIPT_FADE_COLOR(200,200,255, MIN_COL, SECONDS_TO_FRAMES(0.4))
END_SPECIALFX_SCRIPT



// "SHORTING_OUT_MODE" - Randomly flashing white
DEFINE_SPECIALFX_SCRIPT(CampaignerFxShortingOut)
	SPECIALFX_SCRIPT_FADE_COLOR(255,255,255, MAX_COL/3, 0.25)
	SPECIALFX_SCRIPT_FADE_COLOR(255,255,255, MIN_COL, 0.25)

	SPECIALFX_SCRIPT_FADE_COLOR(255,255,255, MAX_COL/3, 0.25)
	SPECIALFX_SCRIPT_FADE_COLOR(255,255,255, MIN_COL, 0.25)

	SPECIALFX_SCRIPT_RANDOM_WAIT(SECONDS_TO_FRAMES(0.1), SECONDS_TO_FRAMES(1))

	SPECIALFX_SCRIPT_REPEAT

END_SPECIALFX_SCRIPT


// "IDLE_MODE" - Quickly going back to normal
DEFINE_SPECIALFX_SCRIPT(CampaignerFxNormal)
	SPECIALFX_SCRIPT_SET_TRANS(MIN_TRANS)
	SPECIALFX_SCRIPT_SET_COLOR(0,0,0, MIN_COL)
END_SPECIALFX_SCRIPT



// "DEATH_MODE" - Randomly flashing white
DEFINE_SPECIALFX_SCRIPT(CampaignerFxDeath)

	SPECIALFX_SCRIPT_PLAY_SOUND(SOUND_ROBOT_SHORT_1)
	SPECIALFX_SCRIPT_FADE_COLOR(255,255,255, MAX_COL/3, 0.25)
	SPECIALFX_SCRIPT_FADE_COLOR(255,255,255, MIN_COL, 0.25)

	SPECIALFX_SCRIPT_FADE_COLOR(255,255,255, MAX_COL/3, 0.25)
	SPECIALFX_SCRIPT_FADE_COLOR(255,255,255, MIN_COL, 0.25)

	SPECIALFX_SCRIPT_RANDOM_WAIT(SECONDS_TO_FRAMES(0.1), SECONDS_TO_FRAMES(3))

	SPECIALFX_SCRIPT_PLAY_SOUND(SOUND_ROBOT_SHORT_2)
	SPECIALFX_SCRIPT_FADE_COLOR(255,255,255, MAX_COL/3, 0.25)
	SPECIALFX_SCRIPT_FADE_COLOR(255,255,255, MIN_COL, 0.25)

	SPECIALFX_SCRIPT_FADE_COLOR(255,255,255, MAX_COL/3, 0.25)
	SPECIALFX_SCRIPT_FADE_COLOR(255,255,255, MIN_COL, 0.25)

	SPECIALFX_SCRIPT_RANDOM_WAIT(SECONDS_TO_FRAMES(0.1), SECONDS_TO_FRAMES(3))

	SPECIALFX_SCRIPT_REPEAT

END_SPECIALFX_SCRIPT


// "DEATH_MODE" - The final white flashing and fading out
DEFINE_SPECIALFX_SCRIPT(CampaignerFxFinalDeath)

	SPECIALFX_SCRIPT_PLAY_SOUND(SOUND_ROBOT_SHORT_1)
	SPECIALFX_SCRIPT_FADE_COLOR(255,255,255, MAX_COL/3, 0.25)
	SPECIALFX_SCRIPT_FADE_COLOR(255,255,255, MIN_COL, 0.25)

	SPECIALFX_SCRIPT_FADE_COLOR(255,255,255, MAX_COL/3, 0.25)
	SPECIALFX_SCRIPT_FADE_COLOR(255,255,255, MIN_COL, 0.25)

	SPECIALFX_SCRIPT_FADE_COLOR(255,255,255, MAX_COL/3, 0.25)
	SPECIALFX_SCRIPT_FADE_COLOR(255,255,255, MIN_COL, 0.25)

	SPECIALFX_SCRIPT_FADE_COLOR(255,255,255, MAX_COL/3, 0.25)
	SPECIALFX_SCRIPT_FADE_COLOR(255,255,255, MIN_COL, 0.25)

	SPECIALFX_SCRIPT_PLAY_SOUND(SOUND_ROBOT_SHORT_2)
	SPECIALFX_SCRIPT_FADE_COLOR(255,255,255, MAX_COL/3, 0.25)
	SPECIALFX_SCRIPT_FADE_COLOR(255,255,255, MIN_COL, 0.25)

	SPECIALFX_SCRIPT_FADE_COLOR(255,255,255, MAX_COL/3, 0.25)
	SPECIALFX_SCRIPT_FADE_COLOR(255,255,255, MIN_COL, 0.25)

	SPECIALFX_SCRIPT_FADE_COLOR(255,255,255, MAX_COL/3, 0.25)
	SPECIALFX_SCRIPT_FADE_COLOR(255,255,255, MIN_COL, 0.25)

	SPECIALFX_SCRIPT_PLAY_SOUND(SOUND_ROBOT_SHORT_2)

	SPECIALFX_SCRIPT_FADE_COLOR(255,255,255, MAX_COL, 0.25)
	SPECIALFX_SCRIPT_FADE_COLOR(255,255,255, MIN_COL, SECONDS_TO_FRAMES(2))

END_SPECIALFX_SCRIPT





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
*	Function:		Campaigner_Initialise()
*
******************************************************************************
*
*	Description:	Prepares campaigner boss
*
*	Inputs:			*pThis	-	Ptr to object instance
*
*	Outputs:			CBoss *	-	Ptr to boss vars
*
*****************************************************************************/
CVector3	CampaignerHoleList[] =
{
	{75*SCALING_FACTOR, 60.5*SCALING_FACTOR, 87*SCALING_FACTOR},
	{76*SCALING_FACTOR, 60.5*SCALING_FACTOR, -66*SCALING_FACTOR}
} ;

BOOL Campaigner_InHole(CGameObjectInstance *pThis, CCampaigner *pCampaigner)
{
	return (pCampaigner->m_Boss.m_CollisionInfo.GravityAcceleration == 0) ;
}

void InitializeCampaigner(CGameObjectInstance *pThis, CCampaigner *pCampaigner)
{
	// Initialise fx
	CSpecialFx__Construct(&pCampaigner->m_SpecialFx, pThis) ;
}


CBoss *Campaigner_Initialise(CGameObjectInstance *pThis)
{
	CCampaigner *pCampaigner = &CampaignerBoss ;

	// Setup defaults
	CBoss__Construct(&pCampaigner->m_Boss) ;

	// Setup regions for in hole modes!
	pCampaigner->m_HoleNumber = 0 ;
	GetApp()->m_BossVars.m_pRegionList[0] = CScene__NearestRegion(&GetApp()->m_Scene, &CampaignerHoleList[0]) ;
	GetApp()->m_BossVars.m_pRegionList[1] = CScene__NearestRegion(&GetApp()->m_Scene, &CampaignerHoleList[1]) ;

	// Setup boss vars
	pCampaigner->m_Boss.m_pInitializeFunction = (void *)InitializeCampaigner ;
	pCampaigner->m_Boss.m_pUpdateFunction = (void *)Campaigner_Update ;
	pCampaigner->m_Boss.m_pHitByParticleFunction = (void *)Campaigner_HitByParticle ;
	pCampaigner->m_Boss.m_pModeTable = CampaignerModeTable ;
	pCampaigner->m_Boss.m_Rot1 = 0 ;
	pCampaigner->m_Boss.m_Rot2 = 0 ;

	// Setup campaigner vars
	pCampaigner->m_pStageInfo = CampaignerStageInfo ;
	Campaigner_SetupStageInfo(pThis, pCampaigner, CampaignerStageInfo) ;

	// Put into start mode
	pCampaigner->m_Boss.m_Mode = CAMPAIGNER_TALK_MODE ;

	pCampaigner->m_Boss.m_ModeAnimSpeed = 0 ;
	pCampaigner->m_Boss.m_AnimRepeats = 1 ;
	pCampaigner->m_Boss.m_AnimSpeed = 1.0 ;

	pCampaigner->m_Boss.m_pPreDrawFunction = (void *)Campaigner_PreDraw ;
	pCampaigner->m_Boss.m_pPostDrawFunction = (void *)Campaigner_PostDraw ;

#ifndef MAKE_CART
	pCampaigner->m_Boss.m_pDisplayModeFunction = (void *)Campaigner_DisplayMode ;
#endif

#if DEBUG_CAMPAIGNER
	rmonPrintf("\r\n\r\n\r\nCAMPAIGNERER HERE WE GO!! Prepare for Attack Action\r\n") ;
#endif

	// Setup misc
	CCollisionInfo__SetCampaignerCollisionDefaults(&pCampaigner->m_Boss.m_CollisionInfo) ;
	BOSS_Health(pThis) = CAMPAIGNER_HEALTH ;

	// Make him limp!
	pCampaigner->m_Limp = FALSE ;

	// Reset run timers
	pCampaigner->m_RunTime = 0 ;
	pCampaigner->m_RunAttackTime = 0 ;

	// Reset all timers
	pCampaigner->m_EvadeTime = 0 ;
	pCampaigner->m_FlinchTime = 0 ;
	pCampaigner->m_ExplodeFlinchTime = 0 ;
	pCampaigner->m_ShortOutTime = 0 ;

	// Init shield
	pCampaigner->m_Boss.m_pShield = &pCampaigner->m_Shield ;
	CShield__Construct(&pCampaigner->m_Shield,
							 pThis,
							 CAMPAIGNER_HEALTH/2,
							 CampaignerShieldColors,
							 sizeof(CampaignerShieldColors) / sizeof(CPsuedoColor)) ;

	// Take off weapon vars
	pCampaigner->m_WeaponToTakeOff = 0 ;
	pCampaigner->m_pDamage = CampDamageTable ;

	// Init sound
	pCampaigner->m_TauntTimer = SECONDS_TO_FRAMES(8) ;
	pCampaigner->m_PlayingSoundTimer = 0 ;
	pCampaigner->m_LastSound = 0 ;

	// Init run attack script
	pCampaigner->m_RunAttackScript.m_pStart =
	pCampaigner->m_RunAttackScript.m_pCurrent = CampaignerRunAttackScript ;

	// Return pointer to campaigner boss
	return (CBoss *)pCampaigner ;
}



/*****************************************************************************
*
*	Function:		CCollisionInfo__SetCampaignerCollisionDefaults()
*
******************************************************************************
*
*	Description:	Sets up default campaigner collision info
*
*	Inputs:			*pThis	-	Ptr to collision info structure
*
*	Outputs:			None
*
*****************************************************************************/
void CCollisionInfo__SetCampaignerCollisionDefaults(CCollisionInfo2 *pThis)
{
	pThis->dwFlags =		COLLISIONFLAG_PHYSICS
							|	COLLISIONFLAG_EXITWATER
							|	COLLISIONFLAG_USEWALLRADIUS
							|	COLLISIONFLAG_TRACKGROUND
							|	COLLISIONFLAG_WATERCOLLISION;

	pThis->InstanceBehavior			= INTERSECT_BEHAVIOR_SLIDE;
	pThis->WallBehavior				= INTERSECT_BEHAVIOR_SLIDE;
	pThis->GroundBehavior			= INTERSECT_BEHAVIOR_SLIDE;
	pThis->GravityAcceleration		= CAMPAIGNER_GRAVITY;
	pThis->BounceReturnEnergy		= 0.0;
	pThis->GroundFriction			= 0.0;
	pThis->AirFriction				= 0.0;
	pThis->WaterFriction				= 0.0;
}




/*****************************************************************************
*
*	Function:		Campaigner_SetupStageInfo()
*
******************************************************************************
*
*	Description:	Sets up stage vars from stage info table
*
*	Inputs:			*pThis			-	Ptr to game object instance
*						*pCampaigner	-	Ptr to boss vars
*						*Info				-	Ptr to stage info vars
*
*	Outputs:			None
*
*****************************************************************************/
void Campaigner_SetupStageInfo(CGameObjectInstance *pThis, CCampaigner *pCampaigner, CCampaignerStageInfo *Info)
{
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


void Campaigner_RunScript(CGameObjectInstance *pThis, CCampaigner *pCampaigner, CBossScript *pScript)
{
	// Read script mode and repeats
	pCampaigner->m_Boss.m_Mode = *pScript->m_pCurrent++ ;

	// Trigger off anim!
	pCampaigner->m_Boss.m_OldMode = -1 ;

	// Setup new mode
	Campaigner_SetupMode((CGameObjectInstance *)pThis, pCampaigner->m_Boss.m_Mode) ;

	// End of script? - If so repeat
	if (*pScript->m_pCurrent == 0)
		pScript->m_pCurrent = pScript->m_pStart ;
}



void Campaigner_UpdateSound(CGameObjectInstance *pThis, CCampaigner *pCampaigner)
{
	pCampaigner->m_PlayingSoundTimer -= frame_increment ;
	if (pCampaigner->m_PlayingSoundTimer < 0)
		pCampaigner->m_PlayingSoundTimer = 0 ;

	pCampaigner->m_TauntTimer -= frame_increment ;
	if (pCampaigner->m_TauntTimer < 0)
		pCampaigner->m_TauntTimer = 0 ;
}

void Campaigner_PlayTauntSound(CGameObjectInstance *pThis, CCampaigner *pCampaigner)
{
	INT32	Sound ;

	if ((pCampaigner->m_TauntTimer == 0) && (pCampaigner->m_PlayingSoundTimer == 0))
	{
		// Play a taunt sound
		if (!COnScreen__BossHasBar(&GetApp()->m_OnScreen, (CBoss *)pCampaigner))
		{
			AI_SEvent_GeneralSound(&pThis->ah.ih, pThis->ah.ih.m_vPos, SOUND_CAMPAIGNERS_TAUNT_1_PATHETIC) ;

			pCampaigner->m_LastSound = SOUND_CAMPAIGNERS_TAUNT_1_PATHETIC ;
			pCampaigner->m_TauntTimer = RandomRangeFLOAT(CAMPAIGNER_MIN_TAUNT_GAP, CAMPAIGNER_MAX_TAUNT_GAP) ;
			pCampaigner->m_PlayingSoundTimer = SECONDS_TO_FRAMES(2) ;
		}
		else
		{
			Sound = CSelection__Choose(CampTauntSounds) ;

			if (Sound != pCampaigner->m_LastSound)
			{
				pCampaigner->m_LastSound = Sound ;
				AI_SEvent_GeneralSound(&pThis->ah.ih, pThis->ah.ih.m_vPos, Sound) ;
				pCampaigner->m_TauntTimer = RandomRangeFLOAT(CAMPAIGNER_MIN_TAUNT_GAP, CAMPAIGNER_MAX_TAUNT_GAP) ;
				pCampaigner->m_PlayingSoundTimer = SECONDS_TO_FRAMES(2) ;
			}
		}
	}
}

void Campaigner_PlayLaughSound(CGameObjectInstance *pThis, CCampaigner *pCampaigner)
{
	if (pCampaigner->m_LastSound == SOUND_CAMPAIGNERS_SHORTING_OUT_LAUGH)
		Campaigner_PlayTauntSound(pThis, pCampaigner) ;
	else
	if (pCampaigner->m_PlayingSoundTimer == 0)
	{
		AI_SEvent_GeneralSound(&pThis->ah.ih, pThis->ah.ih.m_vPos, SOUND_CAMPAIGNERS_SHORTING_OUT_LAUGH) ;
		pCampaigner->m_LastSound = SOUND_CAMPAIGNERS_SHORTING_OUT_LAUGH ;
		pCampaigner->m_PlayingSoundTimer = SECONDS_TO_FRAMES(2) ;
	}
}





/*****************************************************************************
*
*	Function:		Campaigner_Update()
*
******************************************************************************
*
*	Description:	The big top level AI update routine - calls mode code,
*						performs checks etc.
*
*	Inputs:			*pThis		-	Ptr to game object instance
*						*pCampaigner	-	Ptr to boss vars
*
*	Outputs:			None
*
*****************************************************************************/
void Campaigner_Update(CGameObjectInstance *pThis, CCampaigner *pCampaigner)
{
	// Keep the sound up-to date
	Campaigner_UpdateSound(pThis, pCampaigner) ;

	// Has a weapon been fired at Mr. Campaigner?
	if ((pCampaigner->m_Boss.m_Mode != CAMPAIGNER_TELEPORT_MODE) &&
		 (!pCampaigner->m_WeaponToTakeOff) && (CTurokMovement.WeaponCurrent) &&
		 (CTurokMovement.WeaponMode == TWEAPONMODE_ON) && (CTurokMovement.WeaponFired))
	{
		switch(CTurokMovement.WeaponCurrent)
		{
			// Take away weapons
			case AI_OBJECT_WEAPON_TECHWEAPON2:
			case AI_OBJECT_WEAPON_SHOCKWAVE:
				Campaigner_SetupMode(pThis, CAMPAIGNER_WEAPON_IDLE_MODE) ;
				break ;

			// Block weaponss
			case AI_OBJECT_WEAPON_MACHINEGUN:
			case AI_OBJECT_WEAPON_GRENADE_LAUNCHER:
			case AI_OBJECT_WEAPON_TECHWEAPON1:
			case AI_OBJECT_WEAPON_ROCKET:

				// Start shield
				if (pCampaigner->m_pStageInfo->BlockWeaponsPercentage >= RANDOM(101))
					Campaigner_ActivateShield(pThis, pCampaigner, SECONDS_TO_FRAMES(4)) ;
				break ;
		}
	}

	// Get target and angle to aim for
	Campaigner_GetTarget(pThis, pCampaigner) ;

	// Goto teleport/jump?
	if ((!Campaigner_TurokObstructed(pThis, pCampaigner)) &&
		 (pCampaigner->m_Boss.m_ModeFlags & BF_CAN_TELEPORT) &&
		 (pCampaigner->m_Boss.m_DistFromTarget >= CAMPAIGNER_DO_TELEPORT_DIST))
	{
		if ((RANDOM(2)) || (pCampaigner->m_Limp))
			Campaigner_SetupMode(pThis, CAMPAIGNER_TELEPORT_MODE) ;
		else
			Campaigner_SetupMode(pThis, CAMPAIGNER_EVADE_MEGA_JUMP_MODE) ;
	}

	// Check for death
	if ((BOSS_Health(pThis) <= 0) && (!Campaigner_InHole(pThis, pCampaigner)))
	{
		switch(pCampaigner->m_Boss.m_Mode)
		{
			// On floor modes - goto death
			case CAMPAIGNER_DEATH_MODE:
				break ;

			default:
				Campaigner_SetupMode(pThis, CAMPAIGNER_DEATH_MODE) ;
				CSpecialFx__StartScript(&pCampaigner->m_SpecialFx, CampaignerFxDeath) ;
				break ;
		}
	}
	
	// Update invincible flag
	if ((pCampaigner->m_Boss.m_ModeFlags & BF_INVINCIBLE) ||
		 (CShield__IsOn(&pCampaigner->m_Shield)))
		AI_GetDyn(pThis)->m_Invincible = TRUE ;
	else
		AI_GetDyn(pThis)->m_Invincible = FALSE ;

	// Hit by explosion?
	if ((BOSS_Health(pThis) > 0) && (!AI_GetDyn(pThis)->m_Invincible))
	{
		Campaigner_CheckHitByExplosion(pThis, pCampaigner) ;

		// Increase damage
		if (pCampaigner->m_Boss.m_PercentageHealth < pCampaigner->m_pDamage->m_Health)
		{
			pCampaigner->m_pDamage++ ;
			Campaigner_SetupMode(pThis, CAMPAIGNER_DAMAGE_SHORTING_OUT_MODE) ;
			AI_SEvent_GeneralSound(&pThis->ah.ih, pThis->ah.ih.m_vPos, SOUND_CAMPAIGNERS_DEATH) ;
		}
	}

	// Update run time
	if (!(pCampaigner->m_Boss.m_ModeFlags & BF_RUNNING_MODE))
		pCampaigner->m_RunTime = 0 ;
	else
		pCampaigner->m_RunTime += frame_increment ;

	// Update the shield
	CShield__Update(&pCampaigner->m_Shield) ;

	// No interruptions on blends!
	if (CGameObjectInstance__IsBlending(pThis))
		return ;

//	osSyncPrintf("Mode:%d\r\n", pCampaigner->m_Boss.m_Mode) ;

	// Call update function if there is one
	if ((pCampaigner->m_Boss.m_Mode >= 0 ) &&
		 (pCampaigner->m_Boss.m_pModeTable[pCampaigner->m_Boss.m_Mode].m_pUpdateFunction))
		 (pCampaigner->m_Boss.m_pModeTable[pCampaigner->m_Boss.m_Mode].m_pUpdateFunction)(pThis, (CBoss *)pCampaigner) ;

#if CAMPAIGNER_CHECK_FOR_EVADE
	// Check for evading projectiles
	Campaigner_CheckForEvade(pThis, pCampaigner) ;
#endif

	// Put into limp?
	if (pCampaigner->m_pDamage->m_Flags & CAMP_RIGHT_UPPER_LEG_GEOM_DAMAGE_FLAG)
		pCampaigner->m_Limp = TRUE ;

	// Use limping?
	if (pCampaigner->m_Limp)
	{
		switch(pCampaigner->m_Boss.m_Mode)
		{
			case CAMPAIGNER_WALK_MODE:
			case CAMPAIGNER_RUN_MODE:
				pCampaigner->m_Boss.m_Mode = CAMPAIGNER_LIMP_MODE ;
				break ;
			case CAMPAIGNER_WALK_TURN180_MODE:
			case CAMPAIGNER_RUN_TURN180_MODE:
				pCampaigner->m_Boss.m_Mode = CAMPAIGNER_LIMP_TURN180_MODE ;
				break ;
			case CAMPAIGNER_WALK_TURN_LEFT90_MODE:
			case CAMPAIGNER_RUN_TURN_LEFT90_MODE:
				pCampaigner->m_Boss.m_Mode = CAMPAIGNER_LIMP_TURN_LEFT90_MODE ;
				break ;
			case CAMPAIGNER_WALK_TURN_RIGHT90_MODE:
			case CAMPAIGNER_RUN_TURN_RIGHT90_MODE:
				pCampaigner->m_Boss.m_Mode = CAMPAIGNER_LIMP_TURN_RIGHT90_MODE ;
				break ;
		}
	}


	// Check for updating stage
	if (pCampaigner->m_Boss.m_PercentageHealth < pCampaigner->m_pStageInfo->HealthLimit)
	{
		pCampaigner->m_Boss.m_Stage++ ;
		Campaigner_SetupStageInfo(pThis, pCampaigner, ++pCampaigner->m_pStageInfo) ;
#if DEBUG_CAMPAIGNER
			rmonPrintf("\r\n....NEXT ATTACK STAGE....\r\n") ;
#endif
	}

	// Update anim speed
	switch(pCampaigner->m_Boss.m_Mode)
	{
		default:
			pCampaigner->m_Boss.m_AnimSpeed = pCampaigner->m_pStageInfo->AnimSpeed ;
	}
}






/*****************************************************************************
*
*	Function:		Campaigner_SetupMode()
*
******************************************************************************
*
*	Description:	Sets up new campaigner mode
*
*	Inputs:			*pThis			-	Ptr to game object instance
*						*pCampaigner	-	Ptr to boss vars
*
*	Outputs:			TRUE if new mode was setup, else FALSE
*
*****************************************************************************/
INT32 Campaigner_SetupMode(CGameObjectInstance *pThis, INT32 nNewMode)
{
	BOOL	Result ;
	CCampaigner	*pCampaigner = (CCampaigner *)pThis->m_pBoss ;

	// Reset move time
	pCampaigner->m_Boss.m_MoveTime = 0 ;

	// Call normal boss setup
	Result = BOSS_SetupMode(pThis, nNewMode) ;

	return Result ;
}





/*****************************************************************************
*
*	Function:		Campaigner_PreDraw()
*
******************************************************************************
*
*	Description:	Sets up special fx before draw
*
*	Inputs:			*pThis			-	Ptr to game object instance
*						*pCampaigner	-	Ptr to boss vars
*
*	Outputs:			None
*
*****************************************************************************/
void Campaigner_PreDraw(CGameObjectInstance *pThis, CCampaigner *pCampaigner, Gfx **ppDLP)
{
	// Read special fx scripts
	CSpecialFx__Update(&pCampaigner->m_SpecialFx) ;

	// Setup fx draw vars ready for draw
	CSpecialFx__SetDrawFxVars(&pCampaigner->m_SpecialFx) ;
	PrepareDrawFxVars() ;
}





/*****************************************************************************
*
*	Function:		Campaigner_PostDraw()
*
******************************************************************************
*
*	Description:	Resets draw mode so special fx don't affect next objects
*
*	Inputs:			*pThis			-	Ptr to game object instance
*						*pCampaigner	-	Ptr to boss vars
*
*	Outputs:			None
*
*****************************************************************************/
void Campaigner_PostDraw(CGameObjectInstance *pThis, CCampaigner *pCampaigner, Gfx **ppDLP)
{
	// Turn off draw fx
	fx_mode = FXMODE_NONE ;
}

void Campaigner_ActivateShield(CGameObjectInstance *pThis, CCampaigner *pCampaigner, FLOAT Time)
{
	if (pCampaigner->m_Shield.m_Health > 0)
	{
		if (CShield__Activate(&pCampaigner->m_Shield, Time))
			Campaigner_PlayLaughSound(pThis, pCampaigner) ;
		else
			Campaigner_PlayTauntSound(pThis, pCampaigner) ;
	}
}


void Campaigner_DeactivateShield(CGameObjectInstance *pThis, CCampaigner *pCampaigner)
{
	CShield__Deactivate(&pCampaigner->m_Shield) ;
}






/*****************************************************************************
*
*	Function:		Campaigner_GetTarget()
*
******************************************************************************
*
*	Description:	Calculates target to aim for, distance from target, and the
*						correct angle to head.
*
*	Inputs:			*pThis			-	Ptr to game object instance
*						*pCampaigner	-	Ptr to boss vars
*
*	Outputs:			None
*
*****************************************************************************/
void Campaigner_GetTarget(CGameObjectInstance *pThis, CCampaigner *pCampaigner)
{
	CGameObjectInstance *pTurok ;
	CVector3		vTurokPos, vPos ;
	FLOAT			rot, Radius ;

	// Setup Turok pos
	pTurok = CEngineApp__GetPlayer(GetApp());
	vTurokPos = TUROK_GetPos(pTurok) ;

	// Go infront of turok if he's dead
	if (CTurokMovement.Active)
		Radius = 0 ;
	else
		Radius = 15 * SCALING_FACTOR ;

	// Calculate direction towards centre of arena
	rot = pTurok->m_RotY ;
	vPos.x = vTurokPos.x - (Radius * sin(rot)) ;
	vPos.y = vTurokPos.y ;
	vPos.z = vTurokPos.z - (Radius * cos(rot)) ;

	// Calculate distance from this target
	pCampaigner->m_Boss.m_vTarget = vPos ;
	pCampaigner->m_Boss.m_DistFromTargetSqr = AI_DistanceSquared(pThis, vPos) ;
	if (pCampaigner->m_Boss.m_DistFromTargetSqr)
		pCampaigner->m_Boss.m_DistFromTarget = sqrt(pCampaigner->m_Boss.m_DistFromTargetSqr) ;
	else
		pCampaigner->m_Boss.m_DistFromTarget = 0 ;


	// Calculate angle to look at
//	if ((!CTurokMovement.Active) && (pCampaigner->m_Boss.m_DistFromTarget <= CAMPAIGNER_WALK_DIST))
//		pCampaigner->m_Boss.m_DeltaAngle = AI_GetAvoidanceAngle(pThis, TUROK_GetPos(pTurok), pTurok, AVOIDANCE_RADIUS_DISTANCE_FACTOR) ;
//	else
//		pCampaigner->m_Boss.m_DeltaAngle = AI_GetAvoidanceAngle(pThis, pCampaigner->m_Boss.m_vTarget, pTurok, AVOIDANCE_RADIUS_DISTANCE_FACTOR) ;

	// Calculate angle to look at
//	if (CTurokMovement.Active)
//		pCampaigner->m_Boss.m_DeltaAngle = AI_GetAvoidanceAngle(pThis, TUROK_GetPos(pTurok), pTurok, AVOIDANCE_RADIUS_DISTANCE_FACTOR) ;
//	else
//	if (pCampaigner->m_Boss.m_DistFromTarget <= CAMPAIGNER_WALK_DIST)
//		pCampaigner->m_Boss.m_DeltaAngle = AI_GetAvoidanceAngle(pThis, TUROK_GetPos(pTurok), pTurok, AVOIDANCE_RADIUS_DISTANCE_FACTOR) ;
//	else
//		pCampaigner->m_Boss.m_DeltaAngle = AI_GetAvoidanceAngle(pThis, pCampaigner->m_Boss.m_vTarget, pTurok, AVOIDANCE_RADIUS_DISTANCE_FACTOR) ;


	pCampaigner->m_Boss.m_DeltaAngle = AI_GetAvoidanceAngle(pThis, TUROK_GetPos(pTurok), pTurok, AVOIDANCE_RADIUS_DISTANCE_FACTOR) ;
}



/*****************************************************************************
*
*	Function:		Campaigner_PerformTauntAttack()
*
******************************************************************************
*
*	Description:	Makes campaigner perform a taunt animation
*						(victory anim is played if no other taunt has been played)
*
*	Inputs:			*pThis			-	Ptr to game object instance
*						*pCampaigner	-	Ptr to boss vars
*
*	Outputs:			None
*
*****************************************************************************/
void Campaigner_PerformTauntAttack(CGameObjectInstance *pThis, CCampaigner *pCampaigner)
{
	// If 1st time, it must be victory!
	if (!(pCampaigner->m_Boss.m_ModeFlags & BF_TAUNT_MODE))
		Campaigner_SetupMode(pThis, CAMPAIGNER_VICTORY_MODE) ;
	else
		// Perform a random taunt attack
		Campaigner_SetupMode(pThis, CSelection__Choose(CampaignerTauntAttackSelection)) ;
}


/*****************************************************************************
*
*	Function:		Campaigner_PerformRunAttack()
*
******************************************************************************
*
*	Description:	Makes campaigner perform a run attack
*
*	Inputs:			*pThis			-	Ptr to game object instance
*						*pCampaigner	-	Ptr to boss vars
*
*	Outputs:			None
*
*****************************************************************************/
void Campaigner_PerformRunAttack(CGameObjectInstance *pThis, CCampaigner *pCampaigner)
{
	// Play sound
	Campaigner_PlayTauntSound(pThis, pCampaigner) ;

	// Perform a run attack
	Campaigner_RunScript(pThis, pCampaigner, &pCampaigner->m_RunAttackScript) ;

//	Campaigner_SetupMode(pThis, CSelection__Choose(CampaignerRunAttackSelection)) ;
}


/*****************************************************************************
*
*	Function:		Campaigner_PerformCloseAttack()
*
******************************************************************************
*
*	Description:	Makes campaigner perform a close attack
*						(run attack is performed if running)
*
*	Inputs:			*pThis			-	Ptr to game object instance
*						*pCampaigner	-	Ptr to boss vars
*
*	Outputs:			None
*
*****************************************************************************/
void Campaigner_PerformCloseAttack(CGameObjectInstance *pThis, CCampaigner *pCampaigner)
{
	// Play sound
	Campaigner_PlayTauntSound(pThis, pCampaigner) ;

	// Perform a close attack or taunt?
	if (CTurokMovement.Active)
		Campaigner_SetupMode(pThis, CSelection__Choose(CampaignerCloseAttackSelection)) ;
	else
		Campaigner_PerformTauntAttack(pThis, pCampaigner) ;
}



/*****************************************************************************
*
*	Function:		Campaigner_CheckForRunAttack()
*
******************************************************************************
*
*	Description:	Checks to see if the campaignerer should perform a run attack
*
*	Inputs:			*pThis		-	Ptr to game object instance
*						*pCampaigner	-	Ptr to boss vars
*
*	Outputs:			TRUE - if attack is started, else FALSE
*
*****************************************************************************/
BOOL Campaigner_CheckForRunAttack(CGameObjectInstance *pThis, CCampaigner *pCampaigner)
{
	// Been running too long?
	pCampaigner->m_RunAttackTime += frame_increment ;
	if ((pCampaigner->m_RunAttackTime > pCampaigner->m_pStageInfo->RunAttackTime) &&
		 (pCampaigner->m_Boss.m_TurokAngleDist <= CAMPAIGNER_FACING_ANGLE))
	{
		// Reset time
		pCampaigner->m_RunAttackTime = 0 ;

		// Perform a run attack
		Campaigner_PerformRunAttack(pThis, pCampaigner) ;
	}

	// Return true if mode changed
	return (pCampaigner->m_Boss.m_OldMode != pCampaigner->m_Boss.m_Mode) ;
}

/*****************************************************************************
*
*	Function:		Campaigner_CheckForEvade()
*
******************************************************************************
*
*	Description:	Checks to see if the campaigner should evade - if so evades
*						in the correct direction.
*
*	Inputs:			*pThis		-	Ptr to game object instance
*						*pCampaigner	-	Ptr to boss vars
*
*	Outputs:			None
*
*****************************************************************************/
void Campaigner_CheckForEvade(CGameObjectInstance *pThis, CCampaigner *pCampaigner)
{
	// Evade?
	if ((BOSS_Health(pThis)) && (!pCampaigner->m_Limp) &&
		 (DecrementTimer(&pCampaigner->m_EvadeTime)) &&
		 (!CShield__IsOn(&pCampaigner->m_Shield)) &&
		 (pCampaigner->m_Boss.m_ModeFlags & BF_CAN_EVADE) &&
		 (RANDOM(100) >= (100-pCampaigner->m_pStageInfo->EvadePercentage)) &&
		 (BOSS_InProjectilePath(pThis, (CBoss *)pCampaigner, CAMPAIGNER_PROJECTILE_ANGLE)))
	{
		pCampaigner->m_EvadeTime = pCampaigner->m_pStageInfo->EvadeSpacing ;

		if (RANDOM(2))
			Campaigner_SetupMode(pThis, CAMPAIGNER_EVADE_BACK_FLIP_MODE) ;
		else
			Campaigner_SetupMode(pThis, CAMPAIGNER_EVADE_MEGA_JUMP_MODE) ;
	}
}



/*****************************************************************************
*
*	Function:		Campaigner_HitByParticle()
*
******************************************************************************
*
*	Description:	This function is called when a particle hits the campaigner
*						and it decides which flinch, if any, to do.
*
*	Inputs:			*pThis		-	Ptr to game object instance
*						*pCampaigner	-	Ptr to boss vars
*						*pParticle	-	Ptr to particle that hit campaigner
*
*	Outputs:			None
*
*****************************************************************************/
void Campaigner_HitByParticle(CGameObjectInstance *pThis, CCampaigner *pCampaigner, CParticle *pParticle)
{
	INT32	FlinchMode ;

	// Do randomly
	if ((BOSS_Health(pThis)) &&
		 (DecrementTimer(&pCampaigner->m_FlinchTime)) &&
		 (pCampaigner->m_Boss.m_ModeFlags & BF_CAN_FLINCH) &&
		 (RANDOM(100) >= (100-pCampaigner->m_pStageInfo->FlinchPercentage)))
	{
		// Reset time
		pCampaigner->m_FlinchTime = pCampaigner->m_pStageInfo->FlinchSpacing ;

		// Get flinch mode
		FlinchMode = BOSS_HitByParticleFlinchMode(pThis, (CBoss *)pCampaigner, &CampaignerFlinchInfo, pParticle) ;
		Campaigner_SetupMode(pThis, FlinchMode) ;
	}
}








/*****************************************************************************
*
*	Function:		Campaigner_CheckHitByExplosion(CGameObjectInstance *pThis, CCampaigner *pCampaigner)
*
******************************************************************************
*
*	Description:	Check for campaigner being hit by an explosion
*
*	Inputs:			*pThis			-	Ptr to game object instance
*						*pCampaigner	-	Ptr to boss vars
*
*	Outputs:			None
*
*****************************************************************************/
INT16 CampaignerExplodeModes[] =
{
	CAMPAIGNER_EXPLODE_BACK_MODE,
	CAMPAIGNER_EXPLODE_LEFT_MODE,
	CAMPAIGNER_EXPLODE_FRONT_MODE,
	CAMPAIGNER_EXPLODE_RIGHT_MODE,
} ;

BOOL Campaigner_CheckHitByExplosion(CGameObjectInstance *pThis, CCampaigner *pCampaigner)
{
	CGameObjectInstance *pTurok ;
	FLOAT						AngleDiff ;
	INT32						Index ;

	// Hit by an explosion?
	if ((DecrementTimer(&pCampaigner->m_ExplodeFlinchTime)) &&
		 (pCampaigner->m_Boss.m_ModeFlags & BF_CAN_EXPLODE_FLINCH) &&
		 (RANDOM(100) >= (100-pCampaigner->m_pStageInfo->ExplodeFlinchPercentage)) &&
		 (AI_GetDyn(pThis)->m_dwStatusFlags & AI_EV_EXPLOSION))
	{
		pCampaigner->m_ExplodeFlinchTime = pCampaigner->m_pStageInfo->ExplodeFlinchSpacing ;

		// Clear explosion flag
		AI_GetDyn(pThis)->m_dwStatusFlags &= ~AI_EV_EXPLOSION ;

#if DEBUG_CAMPAIGNER
		rmonPrintf("\r\n...EXPLOSION HIT!!...\r\n") ;
#endif

		// Force anim to restart if same mode
		pCampaigner->m_Boss.m_OldMode = -1 ;

		pTurok = CEngineApp__GetPlayer(GetApp());
		AngleDiff = (pTurok->m_RotY -  pThis->m_RotY - ANGLE_DTOR(45)) / ANGLE_DTOR(90) ;
		Index = (INT32)AngleDiff ;

		Campaigner_SetupMode((CGameObjectInstance *)pThis, CampaignerExplodeModes[Index & 3]) ;
		return TRUE ;
	}

	// Clear flag
	AI_GetDyn(pThis)->m_dwStatusFlags &= ~AI_EV_EXPLOSION ;

	return FALSE ;
}





/*****************************************************************************
*
*	Function:		Campaigner_TurokObstructed()
*
******************************************************************************
*
*	Description:	Checks to see if something is in the way between campaigner
*						and turok
*
*	Inputs:			*pThis			-	Ptr to game object instance
*						*pCampaigner	-	Ptr to boss vars
*
*	Outputs:			TRUE if obstructed, else FALSE
*
*****************************************************************************/
BOOL Campaigner_TurokObstructed(CGameObjectInstance *pThis, CCampaigner *pCampaigner)
{
	CGameObjectInstance	*pTurok = CEngineApp__GetPlayer(GetApp());
	return (CAnimInstanceHdr__IsObstructed(&pThis->ah, pTurok->ah.ih.m_vPos, NULL)) ;
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

void Campaigner_PositionInHole(CGameObjectInstance *pThis, CCampaigner *pCampaigner)
{
	pCampaigner->m_Boss.m_CollisionInfo.GravityAcceleration = 0 ;
	pThis->ah.ih.m_vPos = CampaignerHoleList[pCampaigner->m_HoleNumber] ;
	pThis->m_RotY = ANGLE_DTOR((5 + (pCampaigner->m_HoleNumber * 90))) ;
	pThis->ah.ih.m_pCurrentRegion = GetApp()->m_BossVars.m_pRegionList[pCampaigner->m_HoleNumber] ;
}


/////////////////////////////////////////////////////////////////////////////
//	Modes:	  	 CAMPAIGNER_IN_HOLE_IDLE_MODE
//	Description: Idling in one of his holes
/////////////////////////////////////////////////////////////////////////////
void Campaigner_Setup_IN_HOLE_IDLE(CGameObjectInstance *pThis, CCampaigner *pCampaigner)
{
	pCampaigner->m_Boss.m_Mode = CAMPAIGNER_IN_HOLE_IDLE_MODE ;
	Campaigner_PositionInHole(pThis, pCampaigner) ;
}


void Campaigner_Code_IN_HOLE_IDLE(CGameObjectInstance *pThis, CCampaigner *pCampaigner)
{
	// Finished?
	if (pThis->m_asCurrent.m_cFrame >= (pThis->m_asCurrent.m_nFrames - 5))
		Campaigner_SetupMode(pThis, CAMPAIGNER_TELEPORT_MODE) ;
}


/////////////////////////////////////////////////////////////////////////////
//	Modes:	  	 CAMPAIGNER_IN_HOLE_ATTACK_MODE
//	Description: Attacking from hole
/////////////////////////////////////////////////////////////////////////////
void Campaigner_Setup_IN_HOLE_ATTACK(CGameObjectInstance *pThis, CCampaigner *pCampaigner)
{
	pCampaigner->m_Boss.m_Mode = CAMPAIGNER_IN_HOLE_ATTACK_MODE ;
	Campaigner_PositionInHole(pThis, pCampaigner) ;
}

void Campaigner_Code_IN_HOLE_ATTACK(CGameObjectInstance *pThis, CCampaigner *pCampaigner)
{
	// Finished?
	if (pThis->m_asCurrent.m_cFrame >= (pThis->m_asCurrent.m_nFrames - 5))
		Campaigner_SetupMode(pThis, CAMPAIGNER_IN_HOLE_IDLE_MODE) ;
}






/////////////////////////////////////////////////////////////////////////////
//	Modes:	  	 CAMPAIGNER_IN_HOLE_IDLE_MODE
//					 CAMPAIGNER_IDLE_MODE,
//					 CAMPAIGNER_TURN180_MODE,
//					 CAMPAIGNER_TURN_LEFT90_MODE,
//					 CAMPAIGNER_TURN_RIGHT90_MODE,
//
//					 CAMPAIGNER_WALK_MODE,
//					 CAMPAIGNER_WALK_TURN180_MODE,
//					 CAMPAIGNER_WALK_TURN_LEFT90_MODE,
//					 CAMPAIGNER_WALK_TURN_RIGHT90_MODE,
//
//					 CAMPAIGNER_LIMP_MODE,
//					 CAMPAIGNER_LIMP_TURN180_MODE,
//					 CAMPAIGNER_LIMP_TURN_LEFT90_MODE,
//					 CAMPAIGNER_LIMP_TURN_RIGHT90_MODE,
//
//					 CAMPAIGNER_RUN_MODE,
//					 CAMPAIGNER_RUN_TURN180_MODE,
//					 CAMPAIGNER_RUN_TURN_LEFT90_MODE,
//					 CAMPAIGNER_RUN_TURN_RIGHT90_MODE
//	Description: Movment logic - takes care of all turning etc
/////////////////////////////////////////////////////////////////////////////
void Campaigner_Code_Movement(CGameObjectInstance *pThis, CCampaigner *pCampaigner)
{
	FLOAT	DeltaAngle = pCampaigner->m_Boss.m_DeltaAngle ;
//	CGameObjectInstance *pTurok = CEngineApp__GetPlayer(GetApp());
//	osSyncPrintf("DeltaAngle:%f RDeltaAngle:%f\r\n",
//					 ANGLE_RTOD(DeltaAngle),
//					 ANGLE_RTOD(AI_GetAvoidanceAngle(pThis, TUROK_GetPos(pTurok), pTurok, AVOIDANCE_RADIUS_DISTANCE_FACTOR))) ;

	// Within run distance?
	if ((pCampaigner->m_Boss.m_DistFromTarget	> CAMPAIGNER_RUN_DIST) ||
		 (pCampaigner->m_Boss.m_ModeFlags & BF_RUNNING_MODE))
	{
		// Perform attack?
		if (!Campaigner_CheckForRunAttack(pThis, pCampaigner))
		{
			// Turn?
			if (pCampaigner->m_Boss.m_Mode != CAMPAIGNER_RUN_TURN180_MODE)
			{
				// Perform gradual turn
				BOSS_PerformTurn(pThis, (CBoss *)pCampaigner, CAMPAIGNER_RUN_TURN_SPEED, DeltaAngle) ;

				// Turn 180?
				if (DeltaAngle < -CAMPAIGNER_RUN_TURN180_ANGLE)
					pCampaigner->m_Boss.m_Mode = CAMPAIGNER_RUN_TURN180_MODE  ;
				else
				// Turn left?
				if (DeltaAngle < -CAMPAIGNER_RUN_TURN90_ANGLE)
					pCampaigner->m_Boss.m_Mode = CAMPAIGNER_RUN_TURN_LEFT90_MODE  ;
				else
				// Turn Right?
				if (DeltaAngle > CAMPAIGNER_RUN_TURN90_ANGLE)
					pCampaigner->m_Boss.m_Mode = CAMPAIGNER_RUN_TURN_RIGHT90_MODE  ;
			}
			// Perform running weapon attack?
			if ((pCampaigner->m_Boss.m_ModeFlags & BF_RUNNING_MODE) &&
			    (pCampaigner->m_Boss.m_DistFromTarget	< CAMPAIGNER_RUN_MAX_WEAPON_DIST) &&
			    (pCampaigner->m_Boss.m_DistFromTarget	> CAMPAIGNER_RUN_MIN_WEAPON_DIST) &&
				 (abs(pCampaigner->m_Boss.m_DeltaAngle) < ANGLE_DTOR(5)) &&
				 (GetApp()->m_BossVars.m_TurokSpeed < (SCALING_FACTOR*0.5)))
				Campaigner_SetupMode(pThis, CAMPAIGNER_RUN_WEAPON_OVERHEAD_SMASH_MODE) ;
			else
			if (abs(DeltaAngle) <= CAMPAIGNER_RUN_FACING_ANGLE)
			{
				// Put into run mode
				pCampaigner->m_Boss.m_Mode = CAMPAIGNER_RUN_MODE ;
			}
		}
	}
	else
	// Within walk distance?
	if (pCampaigner->m_Boss.m_DistFromTarget	> CAMPAIGNER_WALK_DIST)
	{
		// Perform attack?
		if (!Campaigner_CheckForRunAttack(pThis, pCampaigner))
		{
			// Turn?
			if (pCampaigner->m_Boss.m_Mode != CAMPAIGNER_WALK_TURN180_ANGLE)
			{
				// Perform gradual turn
				BOSS_PerformTurn(pThis, (CBoss *)pCampaigner, CAMPAIGNER_WALK_TURN_SPEED, DeltaAngle) ;

				// Turn 180?
				if (DeltaAngle < -CAMPAIGNER_WALK_TURN180_ANGLE)
					pCampaigner->m_Boss.m_Mode = CAMPAIGNER_WALK_TURN180_MODE  ;
				else
				// Turn left?
				if (DeltaAngle < -CAMPAIGNER_WALK_TURN90_ANGLE)
					pCampaigner->m_Boss.m_Mode = CAMPAIGNER_WALK_TURN_LEFT90_MODE  ;
				else
				// Turn Right?
				if (DeltaAngle > CAMPAIGNER_WALK_TURN90_ANGLE)
					pCampaigner->m_Boss.m_Mode = CAMPAIGNER_WALK_TURN_RIGHT90_MODE  ;
			}

			// Walk straight
			if (abs(DeltaAngle) <= CAMPAIGNER_WALK_FACING_ANGLE)
			{
				// Put into run mode
				pCampaigner->m_Boss.m_Mode = CAMPAIGNER_WALK_MODE ;
			}
		}
	}

	// Attack?
	if (pCampaigner->m_Boss.m_DistFromTarget <= CAMPAIGNER_WALK_DIST)
	{
		// Turn 180?
		if (pCampaigner->m_Boss.m_Mode != CAMPAIGNER_TURN180_MODE)
		{
			if (DeltaAngle < -CAMPAIGNER_TURN180_ANGLE)
				pCampaigner->m_Boss.m_Mode = CAMPAIGNER_TURN180_MODE ;
			else
			// Turn left, right or walk?
			if (DeltaAngle < -CAMPAIGNER_TURN90_ANGLE)
				pCampaigner->m_Boss.m_Mode = CAMPAIGNER_TURN_LEFT90_MODE ;
			else
			if (DeltaAngle > CAMPAIGNER_TURN90_ANGLE)
				pCampaigner->m_Boss.m_Mode = CAMPAIGNER_TURN_RIGHT90_MODE ;
		}

		// Do attack
		if (abs(DeltaAngle) <= CAMPAIGNER_FACING_ANGLE)
		{
			pCampaigner->m_Boss.m_Mode = CAMPAIGNER_IDLE_MODE ;
			Campaigner_PerformCloseAttack(pThis, pCampaigner) ;
		}
	}
}





/////////////////////////////////////////////////////////////////////////////
//	Modes:	  	 CAMPAIGNER_JUMP_KICK_MODE
//					 CAMPAIGNER_SWING_PUNCH_MODE
//					 CAMPAIGNER_FOREARM_SLASH_MODE,
//					 CAMPAIGNER_BACKHAND_SLASH_MODE,
//					 CAMPAIGNER_OVERHEAD_SLASH_MODE,
//					 CAMPAIGNER_FOREARM_SLASH2_MODE,
//					 CAMPAIGNER_BACKHAND_SLASH2_MODE,
//					 CAMPAIGNER_OVERHEAD_SLASH2_MODE,
//					 CAMPAIGNER_ATTACK_BACKHAND_MODE,
//					 CAMPAIGNER_ATTACK_SWIPE_MODE,
//					 CAMPAIGNER_ATTACK_BLOW_MODE,
//	Description: Generic close up attack moves
/////////////////////////////////////////////////////////////////////////////
void Campaigner_Code_CloseAttack(CGameObjectInstance *pThis, CCampaigner *pCampaigner)
{
	// Add boss bar?
	if (!COnScreen__BossHasBar(&GetApp()->m_OnScreen, (CBoss *)pCampaigner))
	{
		COnScreen__AddBossBar(&GetApp()->m_OnScreen, (CBoss *)pCampaigner) ;
		Campaigner_DeactivateShield(pThis, pCampaigner) ;
		CPickupMonitor__Activate(&GetApp()->m_Scene.m_PickupMonitor) ;
	}

	// Finished?
	if (pThis->m_asCurrent.m_cFrame >= (pThis->m_asCurrent.m_nFrames - 5))
		Campaigner_SetupMode(pThis, CAMPAIGNER_IDLE_MODE) ;
}


/////////////////////////////////////////////////////////////////////////////
//	Modes:	  	 CAMPAIGNER_MAGIC_ENERGY_BLAST_MODE,	(fire comming from shield)
//					 CAMPAIGNER_MAGIC_SUMMON_EXPLOSION_MODE,
//					 CAMPAIGNER_MAGIC_SUPER_BLAST_MODE,		(swings weapon around head and then fires)
//					 CAMPAIGNER_MAGIC_SCATTER_BLAST_MODE,
//	Description: Magic attacks
/////////////////////////////////////////////////////////////////////////////
void Campaigner_Code_RunAttack(CGameObjectInstance *pThis, CCampaigner *pCampaigner)
{
	// Track player during first half of super blast
//	if ((pCampaigner->m_Boss.m_Mode == CAMPAIGNER_MAGIC_SUPER_BLAST_MODE) && (pThis->m_asCurrent.m_cFrame <= 30))
	if ((pCampaigner->m_Boss.m_Mode != CAMPAIGNER_MAGIC_SCATTER_BLAST_MODE) && (pThis->m_asCurrent.m_cFrame <= 30))
		BOSS_RotateToFaceTurok(pThis, (CBoss *)pCampaigner, CAMPAIGNER_TURN_ON_THE_SPOT_SPEED) ;

	// Finished?
	if (pThis->m_asCurrent.m_cFrame >= (pThis->m_asCurrent.m_nFrames - 5))
		Campaigner_SetupMode(pThis, CAMPAIGNER_IDLE_MODE) ;
}


/////////////////////////////////////////////////////////////////////////////
//	Modes:	  	 CAMPAIGNER_TELEPORT_MODE,
//	Description: Teleports boss to Turok
/////////////////////////////////////////////////////////////////////////////
void Campaigner_ChooseTeleportDest(CGameObjectInstance *pThis, CCampaigner *pCampaigner)
{
	CGameObjectInstance	*pTurok = CEngineApp__GetPlayer(GetApp());
	FLOAT						Rot ;

	// Goto hole?
	if (pCampaigner->m_TeleportToHole)
	{
		pCampaigner->m_HoleNumber = RANDOM(2) ;
		pCampaigner->m_vTeleportDest = CampaignerHoleList[pCampaigner->m_HoleNumber] ;
	}
	else
	{
		// Calculate angle
		Rot = pTurok->m_RotY ;
		if (pCampaigner->m_Boss.m_LastMode == CAMPAIGNER_EVADE_BACK_FLIP_MODE)
		{
			// To the side or infront?
			if (RANDOM(2))
				Rot = ANGLE_DTOR(30) ;
			else
				Rot = ANGLE_DTOR(40) ;

			// Other side?
			if (RANDOM(2))
				Rot = -Rot ;
		}

		// Teleport behind turok?
//		if (RANDOM(2))
//			Rot += ANGLE_DTOR(180) ;

		// Calculate destination
		pCampaigner->m_vTeleportDest.x = TUROK_GetPos(pTurok).x - (CAMPAIGNER_TELEPORT_DEST_DIST * sin(Rot)) ;
		pCampaigner->m_vTeleportDest.z = TUROK_GetPos(pTurok).z - (CAMPAIGNER_TELEPORT_DEST_DIST * cos(Rot)) ;

		// Setup face angle
		pCampaigner->m_vTeleportDest.y = Rot + ANGLE_DTOR(180) ;
		NormalizeRotation(&pCampaigner->m_vTeleportDest.y) ;
	}
}


void Campaigner_Setup_TELEPORT(CGameObjectInstance *pThis, CCampaigner *pCampaigner)
{
//	Campaigner_ChooseTeleportDest(pThis, pCampaigner) ;
	pCampaigner->m_Boss.m_Mode = CAMPAIGNER_TELEPORT_MODE ;
	pCampaigner->m_Teleported = FALSE ;
	pCampaigner->m_StartedFx = FALSE ;

	// If in a hole, definitely teleport back out to the ground
	if (Campaigner_InHole(pThis, pCampaigner))
		pCampaigner->m_TeleportToHole = FALSE ;
	else
		pCampaigner->m_TeleportToHole = RANDOM(2) ;
}


void Campaigner_Code_TELEPORT(CGameObjectInstance *pThis, CCampaigner *pCampaigner)
{
	// Start fade going
	if ((!pCampaigner->m_StartedFx) && (pThis->m_asCurrent.m_cFrame >= 12))
	{
		pCampaigner->m_StartedFx = TRUE ;
		CSpecialFx__StartScript(&pCampaigner->m_SpecialFx, CampaignerFxTeleportOut) ;

		// Play teleport sound
		AI_SEvent_GeneralSound(&pThis->ah.ih, pThis->ah.ih.m_vPos, SOUND_CAMPAIGNERS_SPELL) ;
	}

	// Teleport yet?
	if ((!pCampaigner->m_Teleported) && (pThis->m_asCurrent.m_cFrame >= 25))
	{
		// Select destination
		Campaigner_ChooseTeleportDest(pThis, pCampaigner) ;

		// Signal teleported
		pCampaigner->m_Teleported = TRUE ;

		// Goto hole?
		if (pCampaigner->m_TeleportToHole)
			Campaigner_PositionInHole(pThis, pCampaigner) ;
		else
		{
			// Put back onto the floor
			pThis->m_RotY = pCampaigner->m_vTeleportDest.y ;
			pThis->ah.ih.m_vPos.y = 0 ;
			pCampaigner->m_Boss.m_CollisionInfo.GravityAcceleration = CAMPAIGNER_GRAVITY ;

			// Move boss
			CAnimInstanceHdr__Teleport((CAnimInstanceHdr *)pThis,
												pCampaigner->m_vTeleportDest.x,
												pCampaigner->m_vTeleportDest.z) ;


			// Get closest region from the player!
//			CInstanceHdr__GetNearPositionAndRegion(&pTurok->ah.ih,						 // Start
//																pCampaigner->m_vTeleportDest,		 // vDest
//																&pThis->ah.ih.m_vPos,				 // vOut
//																&pThis->ah.ih.m_pCurrentRegion,	 // vOutRegion
//																INTERSECT_BEHAVIOR_IGNORE, INTERSECT_BEHAVIOR_IGNORE);


		}

		// Start fade in
		CSpecialFx__StartScript(&pCampaigner->m_SpecialFx, CampaignerFxTeleportIn) ;

		// Play teleport sound
		AI_SEvent_GeneralSound(&pThis->ah.ih, pThis->ah.ih.m_vPos, SOUND_CAMPAIGNERS_SPELL) ;
	}

	// If finished anim, do quick attack - like "Spinal"!
	if (pThis->m_asCurrent.m_cFrame >= (pThis->m_asCurrent.m_nFrames - 5))
	{
		// Do hole attack?
		if (pCampaigner->m_TeleportToHole)
			Campaigner_SetupMode(pThis, CAMPAIGNER_IN_HOLE_ATTACK_MODE) ;
		else
		// Attack if close enough, else walk etc.
		if (pCampaigner->m_Boss.m_DistFromTarget	> CAMPAIGNER_WALK_DIST)
			Campaigner_SetupMode(pThis, CAMPAIGNER_IDLE_MODE) ;
		else
			Campaigner_SetupMode(pThis, CAMPAIGNER_WEAPON_BACKHAND_SLASH2_MODE) ;
	}
}


/////////////////////////////////////////////////////////////////////////////
//	Modes:	  	 CAMPAIGNER_WEAPON_IDLE_MODE
//	Description: Waiting for player to fire weapon
/////////////////////////////////////////////////////////////////////////////
void Campaigner_Setup_WEAPON_IDLE(CGameObjectInstance *pThis, CCampaigner *pCampaigner)
{
	pCampaigner->m_Boss.m_Mode = CAMPAIGNER_WEAPON_IDLE_MODE ;
	BOSS_GetVel(pThis).x = 0 ;
	BOSS_GetVel(pThis).z = 0 ;

	// Record weapon to take off
	pCampaigner->m_WeaponToTakeOff = CTurokMovement.WeaponCurrent ;
}

void Campaigner_Code_WEAPON_IDLE(CGameObjectInstance *pThis, CCampaigner *pCampaigner)
{
	FLOAT	DeltaAngle = pCampaigner->m_Boss.m_DeltaAngle ;

	// Only turn if not in a hole (or doing a 180)
	if ((!Campaigner_InHole(pThis, pCampaigner)) &&
		 (pCampaigner->m_Boss.m_Mode != CAMPAIGNER_WEAPON_TURN180_MODE))
	{
		// Turn 180?
		if (abs(DeltaAngle) > CAMPAIGNER_TURN180_ANGLE)
			pCampaigner->m_Boss.m_Mode = CAMPAIGNER_WEAPON_TURN180_MODE ;
		else
		// Turn left, right or walk?
		if (DeltaAngle < -CAMPAIGNER_TURN90_ANGLE)
			pCampaigner->m_Boss.m_Mode = CAMPAIGNER_WEAPON_TURN_LEFT90_MODE ;
		else
		if (DeltaAngle > CAMPAIGNER_TURN90_ANGLE)
			pCampaigner->m_Boss.m_Mode = CAMPAIGNER_WEAPON_TURN_RIGHT90_MODE ;
	}

	// Facing yet?
	if (abs(DeltaAngle) <= CAMPAIGNER_FACING_ANGLE)
		pCampaigner->m_Boss.m_Mode = CAMPAIGNER_WEAPON_IDLE_MODE ;

	// Wait for weapon to fire
	if (CTurokMovement.FiredParticle)
	{
		Campaigner_SetupMode(pThis, CAMPAIGNER_WEAPON_LAUGH_MODE) ;
		CTurokMovement.FiredParticle = FALSE ;
	}
}


/////////////////////////////////////////////////////////////////////////////
//	Modes:	  	 CAMPAIGNER_WEAPON_LAUGH_MODE
//	Description: Laughing at player firing weapon
/////////////////////////////////////////////////////////////////////////////
void Campaigner_Setup_WEAPON_LAUGH(CGameObjectInstance *pThis, CCampaigner *pCampaigner)
{
	pCampaigner->m_Boss.m_Mode = CAMPAIGNER_WEAPON_LAUGH_MODE ;
	Campaigner_ActivateShield(pThis, pCampaigner, SECONDS_TO_FRAMES(8)) ;

	// Remove the weapon if it is on screen
	if (pCampaigner->m_WeaponToTakeOff == CTurokMovement.WeaponCurrent)
	{
		CTMove__RemoveWeaponFromScreen(&CTurokMovement, GetApp()) ;
		CTMove__SelectWeapon(&CTurokMovement, TWEAPON_AMMO, TRUE) ;
	}

	// Remove weapon from inventory
	switch(pCampaigner->m_WeaponToTakeOff)
	{
		case AI_OBJECT_WEAPON_TECHWEAPON2:
			CTurokMovement.TechWeapon2Flag = FALSE ;
			break ;

		case AI_OBJECT_WEAPON_SHOCKWAVE:
			CTurokMovement.ShockwaveFlag = FALSE ;
			break ;

	}

	// Clear flag
	pCampaigner->m_WeaponToTakeOff = 0 ;
}

void Campaigner_Code_WEAPON_LAUGH(CGameObjectInstance *pThis, CCampaigner *pCampaigner)
{
	// Laugh for a bit
	if (pCampaigner->m_Boss.m_ModeTime >= SECONDS_TO_FRAMES(3))
		Campaigner_SetupMode(pThis, CAMPAIGNER_WEAPON_ATTACK_MODE) ;
}

/////////////////////////////////////////////////////////////////////////////
//	Modes:	  	 CAMPAIGNER_WEAPON_ATTACK_MODE,
//	Description: Attacking back at player
/////////////////////////////////////////////////////////////////////////////
void Campaigner_Setup_WEAPON_ATTACK(CGameObjectInstance *pThis, CCampaigner *pCampaigner)
{
	pCampaigner->m_Boss.m_Mode = CAMPAIGNER_WEAPON_ATTACK_MODE ;
}

void Campaigner_Code_WEAPON_ATTACK(CGameObjectInstance *pThis, CCampaigner *pCampaigner)
{
	BOSS_RotateToFaceTurok(pThis, (CBoss *)pCampaigner, CAMPAIGNER_TURN_ON_THE_SPOT_SPEED) ;

	// Finished?
	if (pThis->m_asCurrent.m_cFrame >= (pThis->m_asCurrent.m_nFrames - 5))
	{
		if (Campaigner_InHole(pThis, pCampaigner))
			Campaigner_SetupMode(pThis, CAMPAIGNER_IN_HOLE_IDLE_MODE) ;
		else
			Campaigner_SetupMode(pThis, CAMPAIGNER_IDLE_MODE) ;
	}
}






/////////////////////////////////////////////////////////////////////////////
//	Modes:	  	 CAMPAIGNER_MAGIC_SUMMON_EXPLOSION_MODE
//	Description: Create explosion around himself
/////////////////////////////////////////////////////////////////////////////
void Campaigner_Setup_MAGIC_SUMMON_EXPLOSION(CGameObjectInstance *pThis, CCampaigner *pCampaigner)
{
	pCampaigner->m_Boss.m_Mode = CAMPAIGNER_MAGIC_SUMMON_EXPLOSION_MODE ;
	pCampaigner->m_Boss.m_MoveTime = -CAMPAIGNER_SUMMON_EXPLOSION_DELAY ;

	// Start angle directly behind Campaigner and in line with Turok
	pCampaigner->m_ExplosionAngle = pCampaigner->m_Boss.m_DeltaAngle + pThis->m_RotY +
											  ANGLE_DTOR(-180) ;

	pCampaigner->m_StartedFx = FALSE ;
}

void Campaigner_Code_MAGIC_SUMMON_EXPLOSION(CGameObjectInstance *pThis, CCampaigner *pCampaigner)
{
	FLOAT			Dist, Rot ;
	CVector3		vDest, vOutPos ;

	// Create an explosion?
	pCampaigner->m_Boss.m_MoveTime -= frame_increment ;
	if (pCampaigner->m_Boss.m_MoveTime <= 0)
	{
		// Start off glow fx
		if (!pCampaigner->m_StartedFx)
		{
			CSpecialFx__StartScript(&pCampaigner->m_SpecialFx, CampaignerFxExplosionStart) ;
			pCampaigner->m_StartedFx = TRUE ;
		}

		pCampaigner->m_Boss.m_MoveTime += SECONDS_TO_FRAMES(0.25) ;

		// Get radius
		Dist = pCampaigner->m_Boss.m_DistFromTarget - (3*SCALING_FACTOR) ;
		if (Dist < (15*SCALING_FACTOR))
			Dist = 15*SCALING_FACTOR ;
		else
		if (Dist > (80*SCALING_FACTOR))
			Dist = 80*SCALING_FACTOR ;

		// Get angle
		Rot = pCampaigner->m_ExplosionAngle +
				(CAMPAIGNER_SUMMON_EXPLOSION_SPREAD_ANGLE *
				 (pCampaigner->m_Boss.m_ModeTime - CAMPAIGNER_SUMMON_EXPLOSION_DELAY)) ;

		// Setup position
		vDest.x = BOSS_GetPos(pThis).x - (Dist * sin(Rot)) ;
		vDest.y = 0 ;
		vDest.z = BOSS_GetPos(pThis).z - (Dist * cos(Rot)) ;

		// Get nearest position
		CInstanceHdr__NearestPosition((CInstanceHdr *)pThis, &vDest, &vOutPos) ;

		// Start explosion
		AI_DoParticle(&pThis->ah.ih, PARTICLE_TYPE_GENERIC65, vOutPos) ;

		// Send out damage event at explosion position
		AI_Event_Dispatcher((CInstanceHdr *)pThis, (CInstanceHdr *)pThis,
								  AI_MEVENT_HIT15, AI_SPECIES_ALL,
								  vOutPos, 0.5) ;

		// Play explosion sound
		AI_SEvent_GeneralSound(&pThis->ah.ih, pThis->ah.ih.m_vPos, SOUND_EXPLOSION_2) ;
	}

	// Finished anim?
	if (pThis->m_asCurrent.m_cFrame >= (pThis->m_asCurrent.m_nFrames - 5))
	{
		Campaigner_SetupMode(pThis, CAMPAIGNER_IDLE_MODE) ;

		// End glow fx
		CSpecialFx__StartScript(&pCampaigner->m_SpecialFx, CampaignerFxExplosionEnd) ;
	}
}



/////////////////////////////////////////////////////////////////////////////
//	Modes:		 CAMPAIGNER_MAGIC_SUPER_BLAST_MODE
//	Description: Firing a whopping reat stream of magic blast
/////////////////////////////////////////////////////////////////////////////
void Campaigner_Setup_MAGIC_SUPER_BLAST(CGameObjectInstance *pThis, CCampaigner *pCampaigner)
{
	pCampaigner->m_Boss.m_Mode = CAMPAIGNER_MAGIC_SUPER_BLAST_MODE ;
	CSpecialFx__StartScript(&pCampaigner->m_SpecialFx, CampaignerFxSuperBlast) ;
}


/////////////////////////////////////////////////////////////////////////////
//	Modes:		 CAMPAIGNER_MAGIC_SCATTER_BLAST_MODE
//	Description: Scattering nasty things
/////////////////////////////////////////////////////////////////////////////
void Campaigner_Setup_MAGIC_SCATTER_BLAST(CGameObjectInstance *pThis, CCampaigner *pCampaigner)
{
	pCampaigner->m_Boss.m_Mode = CAMPAIGNER_MAGIC_SCATTER_BLAST_MODE ;
	CSpecialFx__StartScript(&pCampaigner->m_SpecialFx, CampaignerFxScatterBlast) ;
}



/////////////////////////////////////////////////////////////////////////////
//	Modes:		 CAMPAIGNER_MAGIC_ENERGY_BLAST_MODE
//	Description: Fire at the moment
/////////////////////////////////////////////////////////////////////////////
void Campaigner_Setup_MAGIC_ENERGY_BLAST(CGameObjectInstance *pThis, CCampaigner *pCampaigner)
{
	pCampaigner->m_Boss.m_Mode = CAMPAIGNER_MAGIC_ENERGY_BLAST_MODE ;
	CSpecialFx__StartScript(&pCampaigner->m_SpecialFx, CampaignerFxEnergyBlast) ;
}







/////////////////////////////////////////////////////////////////////////////
//	Modes:		 CAMPAIGNER_EVADE_BACK_FLIP_MODE
//	Description: Generic evades
/////////////////////////////////////////////////////////////////////////////
void Campaigner_Code_Evade(CGameObjectInstance *pThis, CCampaigner *pCampaigner)
{
	// Finished?
	if (pThis->m_asCurrent.m_cFrame >= (pThis->m_asCurrent.m_nFrames - 5))
//	if (pCampaigner->m_Boss.m_AnimPlayed)
//		Campaigner_SetupMode(pThis, CAMPAIGNER_TELEPORT_MODE) ;
		Campaigner_SetupMode(pThis, CAMPAIGNER_IDLE_MODE) ;
}


/////////////////////////////////////////////////////////////////////////////
//	Modes:		 CAMPAIGNER_EVADE_MEGA_JUMP_MODE
//	Description: Jumping over turok
/////////////////////////////////////////////////////////////////////////////
void Campaigner_Code_MegaJump(CGameObjectInstance *pThis, CCampaigner *pCampaigner)
{
	CGameObjectInstance *pTurok = CEngineApp__GetPlayer(GetApp()) ;

	// Start velocities?
	if (pCampaigner->m_Boss.m_EventFlags & BF_EVENT1)
	{
		BOSS_GetVel(pThis).x = (TUROK_GetPos(pTurok).x - BOSS_GetPos(pThis).x) / (32/15) ;
		BOSS_GetVel(pThis).z = (TUROK_GetPos(pTurok).z - BOSS_GetPos(pThis).z) / (32/15) ;
	}

	// Turn off velocity
	if (pCampaigner->m_Boss.m_EventFlags & BF_EVENT2)
	{
		BOSS_GetVel(pThis).x = 0 ;
		BOSS_GetVel(pThis).z = 0 ;
	}

	// Landed?
	if (pThis->m_asCurrent.m_cFrame >= (pThis->m_asCurrent.m_nFrames - 5))
		Campaigner_SetupMode(pThis, CAMPAIGNER_IDLE_MODE) ;
}







/////////////////////////////////////////////////////////////////////////////
//	Modes:		 CAMPAIGNER_BACK_FLINCH_MODE,
//					 CAMPAIGNER_HEAD_FLINCH_MODE,
//					 CAMPAIGNER_LEFT_SHOULDER_FLINCH_MODE,
//					 CAMPAIGNER_RIGHT_SHOULDER_FLINCH_MODE,
//					 CAMPAIGNER_STOMACH_MODE,
//					 CAMPAIGNER_KNEE_MODE
//	Description: Generic flinch modes
/////////////////////////////////////////////////////////////////////////////
void Campaigner_Code_Flinch(CGameObjectInstance *pThis, CCampaigner *pCampaigner)
{
	// Finished?
	if (pThis->m_asCurrent.m_cFrame >= (pThis->m_asCurrent.m_nFrames - 5))
		Campaigner_SetupMode(pThis, CAMPAIGNER_IDLE_MODE) ;
}



/////////////////////////////////////////////////////////////////////////////
//	Modes:		 CAMPAIGNER_DAMAGE_SHORTING_OUT_MODE
//	Description: Sparking with explostions
/////////////////////////////////////////////////////////////////////////////
void Campaigner_Setup_DAMAGE_SHORTING_OUT(CGameObjectInstance *pThis, CCampaigner *pCampaigner)
{
	pCampaigner->m_Boss.m_Mode = CAMPAIGNER_DAMAGE_SHORTING_OUT_MODE ;

	// Start off flashes
	CSpecialFx__StartScript(&pCampaigner->m_SpecialFx, CampaignerFxShortingOut) ;

	// Do explosions
	BOSS_DoNodeDamage(pThis, pCampaigner->m_pDamage->m_Node1) ;
	BOSS_DoNodeDamage(pThis, pCampaigner->m_pDamage->m_Node2) ;
}

void Campaigner_Code_DAMAGE_SHORTING_OUT(CGameObjectInstance *pThis, CCampaigner *pCampaigner)
{
	// Do explosion
	if (!(game_frame_number & 7))
		BOSS_DoNodeDamage(pThis, pCampaigner->m_pDamage->m_Node1) ;

	// Do explosion
	if (!((game_frame_number+2) & 7))
		BOSS_DoNodeDamage(pThis, pCampaigner->m_pDamage->m_Node2) ;

	// Finished?
	if ((pThis->m_asCurrent.m_cFrame >= (pThis->m_asCurrent.m_nFrames - 5)) ||
		 (pCampaigner->m_Boss.m_ModeTime > SECONDS_TO_FRAMES(1.5)))
	{
		if (Campaigner_InHole(pThis, pCampaigner))
			Campaigner_SetupMode(pThis, CAMPAIGNER_IN_HOLE_IDLE_MODE) ;
		else
			Campaigner_SetupMode(pThis, CAMPAIGNER_IDLE_MODE) ;

		// Fx back to normal
		CSpecialFx__StartScript(&pCampaigner->m_SpecialFx, CampaignerFxNormal) ;
	}
}






/////////////////////////////////////////////////////////////////////////////
//	Modes:		 CAMPAIGNER_EXPLODE_FRONT_MODE,
//					 CAMPAIGNER_EXPLODE_BEHIND_MODE,
//					 CAMPAIGNER_EXPLODE_LEFT_MODE,
//					 CAMPAIGNER_EXPLODE_RIGHT_MODE
//	Description: Hit by a explosion
/////////////////////////////////////////////////////////////////////////////
void Campaigner_Setup_Explode(CGameObjectInstance *pThis, CCampaigner *pCampaigner)
{
	// Fx back to normal
	CSpecialFx__StartScript(&pCampaigner->m_SpecialFx, CampaignerFxNormal) ;
}

void Campaigner_Code_Explode(CGameObjectInstance *pThis, CCampaigner *pCampaigner)
{
	// Finished?
	if (pThis->m_asCurrent.m_cFrame >= (pThis->m_asCurrent.m_nFrames - 5))
	{
		Campaigner_SetupMode(pThis, CAMPAIGNER_IDLE_MODE) ;
	}
}



/////////////////////////////////////////////////////////////////////////////
//	Modes:		 CAMPAIGNER_DEATH_MODE
//	Description: Sparking out and dying in style
/////////////////////////////////////////////////////////////////////////////
void Campaigner_Setup_DEATH(CGameObjectInstance *pThis, CCampaigner *pCampaigner)
{
	pCampaigner->m_Boss.m_Mode = CAMPAIGNER_DEATH_MODE ;

	// Start off flashes
	CSpecialFx__StartScript(&pCampaigner->m_SpecialFx, CampaignerFxDeath) ;

	// tell the engine that the main boss for this level is dead
	//BOSS_LevelComplete() ;

	GetApp()->m_BossFlags |= BOSS_FLAG_CAMPAIGNER_DEAD	;

	CCamera__SetObjectToTrack(&GetApp()->m_Camera, pThis) ;
	CCamera__FadeToCinema(&GetApp()->m_Camera, CINEMA_FLAG_TUROK_KILL_CAMPAIGNER) ;

	// Allow end sequence sound to start
	pCampaigner->m_PlayingSoundTimer = 0 ;
}

void Campaigner_Code_DEATH(CGameObjectInstance *pThis, CCampaigner *pCampaigner)
{
	CCamera	*pCamera = &GetApp()->m_Camera ;

	// Start off final flash?
	if (pCampaigner->m_Boss.m_EventFlags & BF_EVENT1)
	{
		CSpecialFx__StartScript(&pCampaigner->m_SpecialFx, CampaignerFxFinalDeath) ;

		// Make camera goto final track
		pCamera->m_Stage++ ;
		pCamera->m_StageTime = 0 ;
	}
}



/////////////////////////////////////////////////////////////////////////////
//	Modes:		 CAMPAIGNER_KILL_TUROK_MODE
//	Description: Killing Turok cinema
/////////////////////////////////////////////////////////////////////////////
void Campaigner_Setup_KILL_TUROK(CGameObjectInstance *pThis, CCampaigner *pCampaigner)
{
	pCampaigner->m_Boss.m_Mode = CAMPAIGNER_KILL_TUROK_MODE ;

	// Put fx back to normal
	CSpecialFx__StartScript(&pCampaigner->m_SpecialFx, CampaignerFxNormal) ;
}


/*****************************************************************************
*
*	Function:		Campaigner_DisplayMode()
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
void Campaigner_DisplayMode(CBoss *pBoss)
{
#if DEBUG_CAMPAIGNER
	switch(pBoss->m_Mode)
	{
		case CAMPAIGNER_IN_HOLE_IDLE_MODE:
			rmonPrintf("CAMPAIGNER_IN_HOLE_IDLE_MODE\r\n") ;
			break ;
		case CAMPAIGNER_IN_HOLE_ATTACK_MODE:
			rmonPrintf("CAMPAIGNER_IN_HOLE_ATTACK_MODE\r\n") ;
			break ;
		case CAMPAIGNER_TALK_MODE:
			rmonPrintf("CAMPAIGNER_TALK_MODE\r\n") ;
			break ;
		case CAMPAIGNER_RAGE_MODE:
			rmonPrintf("CAMPAIGNER_RAGE_MODE\r\n") ;
			break ;

		case CAMPAIGNER_VICTORY_MODE:
			rmonPrintf("CAMPAIGNER_VICTORY_MODE\r\n") ;
			break ;

		case CAMPAIGNER_IDLE_MODE:
			rmonPrintf("CAMPAIGNER_IDLE_MODE\r\n") ;
			break ;
		case CAMPAIGNER_TURN180_MODE:
			rmonPrintf("CAMPAIGNER_TURN180_MODE\r\n") ;
			break ;
		case CAMPAIGNER_TURN_LEFT90_MODE:
			rmonPrintf("CAMPAIGNER_TURN_LEFT90_MODE\r\n") ;
			break ;
		case CAMPAIGNER_TURN_RIGHT90_MODE:
			rmonPrintf("CAMPAIGNER_TURN_RIGHT90_MODE\r\n") ;
			break ;
		case CAMPAIGNER_WALK_MODE:
			rmonPrintf("CAMPAIGNER_WALK_MODE\r\n") ;
			break ;
		case CAMPAIGNER_WALK_TURN180_MODE:
			rmonPrintf("CAMPAIGNER_WALK_TURN180_MODE\r\n") ;
			break ;
		case CAMPAIGNER_WALK_TURN_LEFT90_MODE:
			rmonPrintf("CAMPAIGNER_WALK_TURN_LEFT90_MODE\r\n") ;
			break ;
		case CAMPAIGNER_WALK_TURN_RIGHT90_MODE:
			rmonPrintf("CAMPAIGNER_WALK_TURN_RIGHT90_MODE\r\n") ;
			break ;
		case CAMPAIGNER_LIMP_MODE:
			rmonPrintf("CAMPAIGNER_LIMP_MODE\r\n") ;
			break ;
		case CAMPAIGNER_LIMP_TURN180_MODE:
			rmonPrintf("CAMPAIGNER_LIMP_TURN180_MODE\r\n") ;
			break ;
		case CAMPAIGNER_LIMP_TURN_LEFT90_MODE:
			rmonPrintf("CAMPAIGNER_LIMP_TURN_LEFT90_MODE\r\n") ;
			break ;
		case CAMPAIGNER_LIMP_TURN_RIGHT90_MODE:
			rmonPrintf("CAMPAIGNER_LIMP_TURN_RIGHT90_MODE\r\n") ;
			break ;
		case CAMPAIGNER_RUN_MODE:
			rmonPrintf("CAMPAIGNER_RUN_MODE\r\n") ;
			break ;
		case CAMPAIGNER_RUN_TURN180_MODE:
			rmonPrintf("CAMPAIGNER_RUN_TURN180_MODE\r\n") ;
			break ;
		case CAMPAIGNER_RUN_TURN_LEFT90_MODE:
			rmonPrintf("CAMPAIGNER_RUN_TURN_LEFT90_MODE\r\n") ;
			break ;
		case CAMPAIGNER_RUN_TURN_RIGHT90_MODE:
			rmonPrintf("CAMPAIGNER_RUN_TURN_RIGHT90_MODE\r\n") ;
			break ;

		// Close up attacks
		case CAMPAIGNER_RUN_WEAPON_OVERHEAD_SMASH_MODE:
			rmonPrintf("CAMPAIGNER_RUN_WEAPON_OVERHEAD_SMASH_MODE\r\n") ;
			break ;
		case CAMPAIGNER_WEAPON_FOREARM_SLASH_MODE:
			rmonPrintf("CAMPAIGNER_WEAPON_FOREARM_SLASH_MODE\r\n") ;
			break ;
		case CAMPAIGNER_WEAPON_BACKHAND_SLASH_MODE:
			rmonPrintf("CAMPAIGNER_WEAPON_BACKHAND_SLASH_MODE\r\n") ;
			break ;
		case CAMPAIGNER_WEAPON_OVERHEAD_SLASH_MODE:
			rmonPrintf("CAMPAIGNER_WEAPON_OVERHEAD_SLASH_MODE\r\n") ;
			break ;
		case CAMPAIGNER_WEAPON_FOREARM_SLASH2_MODE:
			rmonPrintf("CAMPAIGNER_WEAPON_FOREARM_SLASH2_MODE\r\n") ;
			break ;
		case CAMPAIGNER_WEAPON_BACKHAND_SLASH2_MODE:
			rmonPrintf("CAMPAIGNER_WEAPON_BACKHAND_SLASH2_MODE\r\n") ;
			break ;
		case CAMPAIGNER_WEAPON_OVERHEAD_SLASH2_MODE:
			rmonPrintf("CAMPAIGNER_WEAPON_OVERHEAD_SLASH2_MODE\r\n") ;
			break ;

		case CAMPAIGNER_ATTACK_BACKHAND_MODE:
			rmonPrintf("CAMPAIGNER_ATTACK_BACKHAND_MODE\r\n") ;
			break ;
		case CAMPAIGNER_ATTACK_SWIPE_MODE:
			rmonPrintf("CAMPAIGNER_ATTACK_SWIPE_MODE\r\n") ;
			break ;
		case CAMPAIGNER_ATTACK_BLOW_MODE:
			rmonPrintf("CAMPAIGNER_ATTACK_BLOW_MODE\r\n") ;
			break ;

		case CAMPAIGNER_MAGIC_ENERGY_BLAST_MODE:
			rmonPrintf("CAMPAIGNER_MAGIC_ENERGY_BLAST_MODE\r\n") ;
			break ;
		case CAMPAIGNER_MAGIC_SUMMON_EXPLOSION_MODE:
			rmonPrintf("CAMPAIGNER_MAGIC_SUMMON_EXPLOSION_MODE\r\n") ;
			break ;
		case CAMPAIGNER_MAGIC_SUPER_BLAST_MODE:
			rmonPrintf("CAMPAIGNER_MAGIC_SUPER_BLAST_MODE\r\n") ;
			break ;
		case CAMPAIGNER_MAGIC_SCATTER_BLAST_MODE:
			rmonPrintf("CAMPAIGNER_MAGIC_SCATTER_BLAST_MODE\r\n") ;
			break ;

		case CAMPAIGNER_TELEPORT_MODE:
			rmonPrintf("CAMPAIGNER_TELEPORT_MODE\r\n") ;
			break ;

		// Evades
		case CAMPAIGNER_EVADE_BACK_FLIP_MODE:
			rmonPrintf("CAMPAIGNER_EVADE_BACK_FLIP_MODE\r\n") ;
			break ;

		case CAMPAIGNER_EVADE_MEGA_JUMP_MODE:
			rmonPrintf("CAMPAIGNER_EVADE_MEGA_JUMP_MODE\r\n") ;
			break ;

		// Flinches
		case CAMPAIGNER_RIGHT_SHOULDER_FLINCH_MODE:
			rmonPrintf("CAMPAIGNER_RIGHT_SHOULDER_FLINCH_MODE\r\n") ;
			break ;
		case CAMPAIGNER_LEFT_SHOULDER_FLINCH_MODE:
			rmonPrintf("CAMPAIGNER_LEFT_SHOULDER_FLINCH_MODE\r\n") ;
			break ;
		case CAMPAIGNER_CHEST_FLINCH_MODE:
			rmonPrintf("CAMPAIGNER_CHEST_FLINCH_MODE\r\n") ;
			break ;
		case CAMPAIGNER_BACK_FLINCH_MODE:
			rmonPrintf("CAMPAIGNER_BACK_FLINCH_MODE\r\n") ;
			break ;
		case CAMPAIGNER_DAMAGE_SHORTING_OUT_MODE:
			rmonPrintf("CAMPAIGNER_DAMAGE_SHORTING_OUT_MODE\r\n") ;
			break ;
		case CAMPAIGNER_KNEE_FALL_FLINCH_MODE:
			rmonPrintf("CAMPAIGNER_KNEE_FALL_FLINCH_MODE\r\n") ;
			break ;
		case CAMPAIGNER_KNEE_GETUP_MODE:
			rmonPrintf("CAMPAIGNER_KNEE_GETUP_MODE\r\n") ;
			break ;

		// Impacts
		case CAMPAIGNER_EXPLODE_FRONT_MODE:
			rmonPrintf("CAMPAIGNER_EXPLODE_FRONT_MODE\r\n") ;
			break ;
		case CAMPAIGNER_EXPLODE_BACK_MODE:
			rmonPrintf("CAMPAIGNER_EXPLODE_BACK_MODE\r\n") ;
			break ;
		case CAMPAIGNER_EXPLODE_LEFT_MODE:
			rmonPrintf("CAMPAIGNER_EXPLODE_LEFT_MODE\r\n") ;
			break ;
		case CAMPAIGNER_EXPLODE_RIGHT_MODE:
			rmonPrintf("CAMPAIGNER_EXPLODE_RIGHT_MODE\r\n") ;
			break ;
		case CAMPAIGNER_DEATH_MODE:
			rmonPrintf("CAMPAIGNER_DEATH_MODE\r\n") ;
			break ;

		default:
			rmonPrintf("Mode: %d\r\n", pBoss->m_Mode) ;
			break ;
	}
#endif
}
#endif



/////////////////////////////////////////////////////////////////////////////
// Campaigner texture index
/////////////////////////////////////////////////////////////////////////////
int CGameObjectInstance__GetCampaignerTextureIndex(CGameObjectInstance *pThis, int nNode)
{
	UINT32	DamageFlags = ((CCampaigner *)(pThis->m_pBoss))->m_pDamage->m_Flags ;

	switch(nNode)
	{
		case CAMP_L_SHOULDER_NODE:
		case CAMP_R_SHOULDER_NODE:
			return ((DamageFlags & CAMP_SHOULDERS_TEX_DAMAGE_FLAG) != 0) ;

		case CAMP_L_UP_ARM_NODE:
		case CAMP_R_UP_ARM_NODE:
			return ((DamageFlags & CAMP_UPPER_ARMS_TEX_DAMAGE_FLAG) != 0) ;

		case CAMP_L_UP_LEG_NODE:
		case CAMP_R_UP_LEG_NODE:
			return ((DamageFlags & CAMP_UPPER_LEGS_TEX_DAMAGE_FLAG) != 0) ;

		case CAMP_TORSO3_NODE:
			return ((DamageFlags & CAMP_TORSO_TEX_DAMAGE_FLAG) != 0) ;

		default:
			return 0 ;
	}
}

/////////////////////////////////////////////////////////////////////////////
// Campaigner model index
/////////////////////////////////////////////////////////////////////////////
int CGameObjectInstance__GetCampaignerModelIndex(CGameObjectInstance *pThis, int nNode)
{
	UINT32	DamageFlags = ((CCampaigner *)(pThis->m_pBoss))->m_pDamage->m_Flags ;

	switch(nNode)
	{
		case CAMP_HEAD_NODE:
			return ((DamageFlags & CAMP_HEAD_GEOM_DAMAGE_FLAG) != 0) ;

		case CAMP_NECK_NODE:
			return ((DamageFlags & CAMP_NECK_GEOM_DAMAGE_FLAG) != 0) ;

		case CAMP_L_UP_ARM_NODE:
			return ((DamageFlags & CAMP_LEFT_UPPER_ARM_GEOM_DAMAGE_FLAG) != 0) ;

		case CAMP_L_LOW_ARM_NODE:
			return ((DamageFlags & CAMP_LEFT_LOWER_ARM_GEOM_DAMAGE_FLAG) != 0) ;

		case CAMP_R_UP_LEG_NODE:
			return ((DamageFlags & CAMP_RIGHT_UPPER_LEG_GEOM_DAMAGE_FLAG) != 0) ;

		default:
			return 0 ;
	}
}


