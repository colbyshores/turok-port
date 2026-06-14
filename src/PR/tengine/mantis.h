// mantis.h
/////////////////////////////////////////////////////////////////////////////

#ifndef _INC_MANTIS
#define _INC_MANTIS

#include "boss.h"


/////////////////////////////////////////////////////////////////////////////
// Constants
/////////////////////////////////////////////////////////////////////////////


// Modes
enum MantisModes
{
	MANTIS_SLEEPING_FLOOR_MODE,	// 0
	MANTIS_BREAK_FLOOR_MODE,		// 1

	MANTIS_IDLE_FLOOR_MODE,			// 2
	MANTIS_LOOK_FLOOR_MODE,			// 3

	MANTIS_WALK_FLOOR_MODE,				// 4
	MANTIS_TURN_FLOOR_LEFT90_MODE,	// 5
	MANTIS_TURN_FLOOR_RIGHT90_MODE,	// 6

	MANTIS_JUMP_FLOOR_A_MODE,			// 7
	MANTIS_JUMP_FLOOR_B_MODE,			// 8
	MANTIS_JUMP_FLOOR_C_MODE,			// 9

	// On the floor attacks
	MANTIS_PHASE_FLOOR_MODE,				// 10
	MANTIS_BLAST_FLOOR_MODE,				// 11
	MANTIS_SPIT_FLOOR_MODE,					// 12
	MANTIS_SIDE_SWIPE_FLOOR_MODE,			// 13
	MANTIS_FORWARD_SWIPE_FLOOR_MODE,		// 14

	// Specials
	MANTIS_VICTORY_IDLE_FLOOR_MODE,		// 15
	MANTIS_RAGE_FLOOR_MODE,					// 16

	// Ceiling modes
	MANTIS_JUMP_FLOOR_TO_CEILING_MODE,	// 17

	MANTIS_IDLE_CEILING_MODE,	// 18
	MANTIS_RAIN_CEILING_MODE,	// 19
	MANTIS_SPIT_CEILING_MODE,	// 20

	MANTIS_WALK_CEILING_MODE,				// 21
	MANTIS_TURN_CEILING_LEFT90_MODE,		// 22
	MANTIS_TURN_CEILING_RIGHT90_MODE,	// 23

 	MANTIS_JUMP_CEILING_TO_FLOOR_MODE,	// 24

	MANTIS_JUMP_FLOOR_TO_LEFT_WALL_A_MODE,	// 25
	MANTIS_JUMP_FLOOR_TO_LEFT_WALL_B_MODE,	// 26 
	MANTIS_JUMP_FLOOR_TO_LEFT_WALL_C_MODE,	// 27

	MANTIS_JUMP_RIGHT_WALL_TO_LEFT_WALL_A_MODE,	// 28
	MANTIS_JUMP_RIGHT_WALL_TO_LEFT_WALL_B_MODE,	// 29
	MANTIS_JUMP_RIGHT_WALL_TO_LEFT_WALL_C_MODE,	// 30
	
	MANTIS_IDLE_LEFT_WALL_MODE,				// 31
	MANTIS_JUMP_LEFT_WALL_TO_FLOOR_MODE,	// 32
	MANTIS_JUMP_LEFT_WALL_TO_CEILING_MODE,	// 33
	MANTIS_SPIT_LEFT_WALL_MODE,				// 34
	MANTIS_SPIT_LEFT_WALL_LOOP_MODE,			// 35

	MANTIS_JUMP_FLOOR_TO_RIGHT_WALL_A_MODE,	// 36
	MANTIS_JUMP_FLOOR_TO_RIGHT_WALL_B_MODE,	// 37
	MANTIS_JUMP_FLOOR_TO_RIGHT_WALL_C_MODE,	// 38

	MANTIS_JUMP_LEFT_WALL_TO_RIGHT_WALL_A_MODE,	// 39
	MANTIS_JUMP_LEFT_WALL_TO_RIGHT_WALL_B_MODE,	// 40
	MANTIS_JUMP_LEFT_WALL_TO_RIGHT_WALL_C_MODE,	// 41
				 
	MANTIS_IDLE_RIGHT_WALL_MODE,					// 42
	MANTIS_JUMP_RIGHT_WALL_TO_FLOOR_MODE,		// 43
	MANTIS_JUMP_RIGHT_WALL_TO_CEILING_MODE,	// 44
	MANTIS_SPIT_RIGHT_WALL_MODE,					// 45
	MANTIS_SPIT_RIGHT_WALL_LOOP_MODE,			// 46

	MANTIS_EXPLODE_FLOOR_BACK_MODE,				// 47
	MANTIS_EXPLODE_FLOOR_FRONT_MODE,				// 48
	MANTIS_EXPLODE_FLOOR_LEFT_MODE,				// 49
	MANTIS_EXPLODE_FLOOR_RIGHT_MODE,				// 50

	MANTIS_EXPLODE_CEILING_MODE,		// 51
	MANTIS_EXPLODE_LEFT_WALL_MODE,	// 52
	MANTIS_EXPLODE_RIGHT_WALL_MODE,	// 53

	MANTIS_DEATH_FLOOR_MODE,			// 54

	// Cinema modes
	MANTIS_KILLTUROK_MODE,	// 55
	MANTIS_CINEMA1_MODE,		// 56
	MANTIS_CINEMA2_MODE,		// 57
	MANTIS_CINEMA3_MODE,		// 58
	MANTIS_CINEMA4_MODE,		// 59
	MANTIS_CINEMA5_MODE,		// 60

	MANTIS_END_MODE
} ;




/////////////////////////////////////////////////////////////////////////////
// Structures
/////////////////////////////////////////////////////////////////////////////

typedef struct MantisStageInfo_t
{
	INT32			HealthLimit ;

	FLOAT			AnimSpeed ;
	FLOAT			FlySpeed ;
	FLOAT			MaxWalkTime ;
	INT32			FlySpitSkips ;
	INT32			HitsBeforeExplosion ;
	FLOAT			*pAttackGlowFx ;

	INT16			*pFloorAttackScript ;
	INT16			*pFloorCloseAttackScript ;
	INT16			*pCeilingAttackScript ;
	INT16			*pLeftWallAttackScript ;
	INT16			*pRightWallAttackScript ;
} CMantisStageInfo ;



// Mantis structure
typedef struct CMantis_t
{
	// Generic boss vars - MUST BE HERE AT TOP!
	CBoss			m_Boss ;

	INT16					m_ScriptStage ;
	CMantisStageInfo	*m_pStageInfo ;

	CBossScript	m_FloorAttackScript ;
	CBossScript	m_FloorCloseAttackScript ;
	CBossScript	m_CeilingAttackScript ;
	CBossScript	m_LeftWallAttackScript ;
	CBossScript	m_RightWallAttackScript ;

	BOOL			m_WallBroken[4] ;					// Walls 1-4 broken flags
	INT16			m_WallsToBreak ;					// Total walls left to break
	INT16			m_WallToAimFor ;					// Current wall to go for
	INT16			m_WallToAimForJumpPos ;			// Current wall position to go for

	CGameRegion	*m_pWallCollisionRegion ;		// Wall region
	CVector3		m_vWallPos ;						// Position on wall
	FLOAT			m_WallCollapseTime ;

	BOOL			m_GotoFloor ;			// Makes mantis get to the floor
	FLOAT			m_FlyTargetY ;
	FLOAT			m_FlySpeed ;
	FLOAT			m_FlyTime ;
	FLOAT			m_WallAngle ;			// Angle of wall mantis just hit

	FLOAT			m_WalkTime ;

	INT16			m_FlySpitTotal ;		// Attempt at fly spits
	BOOL			m_FlinchGlow ;

	BOOL			m_ChoosePosition ;	// Set to TRUE for get target logic
	CSpecialFx	m_SpecialFx ;
	CSpecialFx	m_FlashSpecialFx ;

	INT32			m_ExplosionHits ;

	BOOL			m_AcidHurtTurok ;
} CMantis ;


enum MantisActions
{
	MANTIS_ATTACK_ACTION,
	MANTIS_BREAK_WALL_ACTION,
	MANTIS_RAGE_ACTION,

	MANTIS_END_ACTION
} ;


typedef void(*MantisSetupFunction)(CGameObjectInstance *pThis, CMantis *pMantis) ;



/////////////////////////////////////////////////////////////////////////////
// Externs
/////////////////////////////////////////////////////////////////////////////
extern	CMantis				MantisBoss ;



/////////////////////////////////////////////////////////////////////////////
// Functions prototypes
/////////////////////////////////////////////////////////////////////////////
CBoss *Mantis_Initialise(CGameObjectInstance *pThis) ;
void Mantis_Update(CGameObjectInstance *pThis, CMantis *pMantis) ;
BOOL Mantis_CanSpit(CGameObjectInstance *pThis, CMantis *pMantis) ;

INT32 Mantis_SetupMode(CGameObjectInstance *pThis, INT32 nNewMode) ;

void Mantis_UpdateGlow(CGameObjectInstance *pThis, CMantis *pMantis) ;
void Mantis_StartGlow(CGameObjectInstance *pThis, CMantis *pMantis, INT16 Count) ;
void Mantis_PreDraw(CGameObjectInstance *pThis, CMantis *pMantis, Gfx **ppDLP) ;
void Mantis_PostDraw(CGameObjectInstance *pThis, CMantis *pMantis, Gfx **ppDLP) ;

/////////////////////////////////////////////////////////////////////////////
#endif // _INC_MANTIS
