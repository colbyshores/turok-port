// trex.h
/////////////////////////////////////////////////////////////////////////////

#ifndef _INC_TREX
#define _INC_TREX

#include "boss.h"


/////////////////////////////////////////////////////////////////////////////
// Constants
/////////////////////////////////////////////////////////////////////////////


// Modes
enum TRexModes
{
	// Entering arena on elevator
	TREX_ON_ELEVATOR_IDLE_MODE,
	TREX_ON_ELEVATOR_RAGE_MODE,
	TREX_ON_ELEVATOR_LOOK_MODE,
	TREX_EXIT_ELEVATOR_MODE,

	// Idle modes
	TREX_IDLE_MODE,
	TREX_LOOK_MODE,

	TREX_TURN_LEFT90_MODE,
	TREX_TURN_RIGHT90_MODE,
	TREX_TURN180_MODE,

	// Move modes
	TREX_RUN_MODE,
	TREX_RUN_TURN_LEFT90_MODE,
	TREX_RUN_TURN_RIGHT90_MODE,
	TREX_RUN_BITE_MODE,

	TREX_LIMP_MODE,
	TREX_LIMP_TURN_LEFT90_MODE,
	TREX_LIMP_TURN_RIGHT90_MODE,

	// Attacks
	TREX_TAIL_SLAM_MODE,
	TREX_BITE_MODE,
	TREX_RAGE_MODE,

	// Looping attacks
	TREX_BREATH_FIRE_MODE,
	TREX_BREATH_FIRE_IN_MODE,
	TREX_BREATH_FIRE_OUT_MODE,

	TREX_EYE_FIRE_MODE,
	TREX_EYE_FIRE_IN_MODE,
	TREX_EYE_FIRE_OUT_MODE,

	// Up Attacks
	TREX_BITE_UP_MODE,
	TREX_FIRE_UP_MODE,

	// Cave specials
	TREX_CAVE_STOOP_MODE,		// Walking and stooping into cave
	TREX_CAVE_LOOK_MODE,			// Looking in cave
	TREX_CAVE_BITE_MODE,			// Biting at player in cave
	TREX_CAVE_OUT_MODE,			// Back to idle

	TREX_CAVE_NOSE_HIT_MODE,	// Nose hit while lokking in cave
	TREX_CAVE_RAGE_MODE,			// Rage after nose hit
	TREX_CAVE_TAIL_SLAM_MODE,	// Tail slam after rage

	// Explode modes
	TREX_EXPLODE_LEFT_MODE,
	TREX_EXPLODE_RIGHT_MODE,
	TREX_EXPLODE_FRONT_MODE,
	TREX_EXPLODE_BACK_MODE,

  	// Hurt modes
	TREX_STUNNED_MODE,
	TREX_STUN_OUT_MODE,
	TREX_DEATH_MODE,

	TREX_EATTUROK_MODE,

	TREX_END_MODE
} ;



// Attack actions
enum TRexActions
{
	TREX_ON_ELEVATOR_ACTION,
	TREX_EXIT_ELEVATOR_ACTION,
	TREX_ATTACK_ACTION,
	TREX_LEDGE_ATTACK_ACTION,
	TREX_CAVE_ATTACK_ACTION
} ;



/////////////////////////////////////////////////////////////////////////////
// Script commands
/////////////////////////////////////////////////////////////////////////////
#define TREXCMND_END_ATTACK			0
#define TREXCMND_REPEAT					-1

// Attacks
#define TREXSCRIPT_TAIL_SLAM(n) 		TREX_TAIL_SLAM_MODE,n,
#define TREXSCRIPT_BITE(n)				TREX_BITE_MODE,n,
#define TREXSCRIPT_RAGE(n)				TREX_RAGE_MODE,n,
#define TREXSCRIPT_BREATH_FIRE(n) 	TREX_BREATH_FIRE_MODE,n,
#define TREXSCRIPT_EYE_FIRE(n)		TREX_EYE_FIRE_MODE,n,

// Up attacks
#define TREXSCRIPT_BITE_UP(n)			TREX_BITE_UP_MODE,n,
#define TREXSCRIPT_FIRE_UP(n)			TREX_FIRE_UP_MODE,n,

// Cave attacks
#define TREXSCRIPT_CAVE_LOOK(n)	 	TREX_CAVE_LOOK_MODE,n,
#define TREXSCRIPT_BITE_LOOK(n)	 	TREX_CAVE_BITE_MODE,n,



#define TREXSCRIPT_END_ATTACK			TREXCMND_END_ATTACK,
#define TREXSCRIPT_REPEAT				TREXCMND_REPEAT

#define	DEFINE_TREXSCRIPT(name)		INT16	name[] = {
#define	END_TREXSCRIPT					TREXSCRIPT_REPEAT} ;						





/////////////////////////////////////////////////////////////////////////////
// Structures
/////////////////////////////////////////////////////////////////////////////


// TRex stage info
typedef struct TRexStageInfo_t
{
	INT32			HealthLimit ;

	FLOAT			AnimSpeed ;
	INT16			HitsBeforeExplode ;
	INT16			CaveTailSlams ;

	INT16			*pAttackScript ;
	INT16			*pCloseAttackScript ;
	INT16			*pRunAttackScript ;
	INT16			*pTurnAttackScript ;
	INT16			*pLedgeAttackScript ;
	INT16			*pCaveAttackScript ;

} CTRexStageInfo ;


// Cave info for when Turok in on one
typedef struct CCaveInfo_t
{
	CVector3		m_Corner1 ;		// Corner position 1 (smallest x,z)
	CVector3		m_Corner2 ;		// Corner position 2 (biggest x,z)
	CVector3		m_vTargetPos ;	// Position for TRex to stand at
	CVector3		m_vLookPos ;	// Position to look at
} CCaveInfo ;


// TRex structure
typedef struct CTRex_t
{
	// Generic boss vars - MUST BE HERE AT TOP!
	CBoss			m_Boss ;

	// Stage info
	INT16					m_ScriptStage ;
	CTRexStageInfo		*m_pStageInfo ;

	// Script info
	CBossScript			m_AttackScript ;
	CBossScript			m_CloseAttackScript ;
	CBossScript			m_RunAttackScript ;
	CBossScript			m_TurnAttackScript ;
	CBossScript			m_LedgeAttackScript ;
	CBossScript			m_CaveAttackScript ;

	INT16					m_ExplosionHits ;

	FLOAT					m_LookTime ;
	FLOAT					m_RunTime ;
	FLOAT					m_TurnTime ;

	FLOAT					m_TargetDist ;
	FLOAT					m_AtTargetDist ;

	CCaveInfo			*m_pTurokInCave ;			// Position for trex to stand
	CCaveInfo			*m_pTargetCave ;			// Current cave going for
	FLOAT					m_TurokTimeInCave ;		// Time turok has spent in cave
	FLOAT					m_TurokInCaveWaitTime ;	// Time before re-acting to Turok

	INT32					m_LaserSoundHandle ;		// Laser sound
	CVector3				m_vLaserLastSoundPos ;

	INT32					m_FireSoundHandle ;			// Flame sound 
	INT16					m_FireSoundVolume ;			// Current volume
															
	CDamage				*m_pDamage ;
} CTRex ;




/////////////////////////////////////////////////////////////////////////////
// Externs
/////////////////////////////////////////////////////////////////////////////
extern	CTRex				TRexBoss ;



/////////////////////////////////////////////////////////////////////////////
// Functions prototypes
/////////////////////////////////////////////////////////////////////////////
typedef void(*TRexSetupFunction)(CGameObjectInstance *pThis, CTRex *pTRex) ;

CBoss *TRex_Initialise(CGameObjectInstance *pThis) ;
void TRex_Update(CGameObjectInstance *pThis, CTRex *pTRex) ;
INT32 TRex_SetupMode(CGameObjectInstance *pThis, INT32 nNewMode) ;

int CGameObjectInstance__GetTRexTextureIndex(CGameObjectInstance *pThis, int nNode) ;

void TRex_StartFireSound(CGameObjectInstance *pThis, CTRex *pTRex) ;
void TRex_StopFireSound(CGameObjectInstance *pThis, CTRex *pTRex) ;


/////////////////////////////////////////////////////////////////////////////
#endif // _INC_TREX
