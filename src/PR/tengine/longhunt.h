// longhunt.h
/////////////////////////////////////////////////////////////////////////////

#ifndef _INC_LONGHUNT
#define _INC_LONGHUNT

#include "boss.h"


/////////////////////////////////////////////////////////////////////////////
// Constants
/////////////////////////////////////////////////////////////////////////////



											
// Modes
enum LonghuntModes
{
	// Start modes
	LONGHUNT_WAIT_FOR_START_MODE,
	LONGHUNT_SUMMON_HUMVEE_MODE,
	LONGHUNT_WAIT_FOR_HUMVEE_ENTRANCE_MODE,
	LONGHUNT_WAIT_FOR_HUMVEES_DEATH_MODE,

	// Stand modes
	LONGHUNT_STAND_IDLE_MODE,
	LONGHUNT_STAND_TURN180_MODE,
	LONGHUNT_STAND_TURN_LEFT90_MODE,
	LONGHUNT_STAND_TURN_RIGHT90_MODE,

	// Taunt modes
	LONGHUNT_TAUNT1_MODE,

	// Walk modes
	LONGHUNT_WALK_MODE,
	LONGHUNT_WALK_TURN180_MODE,
	LONGHUNT_WALK_TURN_LEFT90_MODE,
	LONGHUNT_WALK_TURN_RIGHT90_MODE,

	// Run modes
	LONGHUNT_RUN_MODE,
	LONGHUNT_RUN_TURN180_MODE,
	LONGHUNT_RUN_TURN_LEFT90_MODE,
	LONGHUNT_RUN_TURN_RIGHT90_MODE,

	// Evade modes
	LONGHUNT_EVADE_LEFT_MODE,
	LONGHUNT_EVADE_RIGHT_MODE,

	// Close attack modes
	LONGHUNT_ROUNDHOUSE_MODE,
	LONGHUNT_JUMP_KICK_MODE,
	LONGHUNT_SWING_PUNCH_MODE,

	// Running attacks
	LONGHUNT_FIRE_GUN_MODE,
	LONGHUNT_THROW_GRENADE_MODE,
	LONGHUNT_RUN_FIRE_GUN_MODE,

	// Death modes
	LONGHUNT_DEATH_MODE,

	LONGHUNT_END_MODE
} ;



// Attack actions
enum LonghuntActions
{
	LONGHUNT_WAIT_FOR_HUMVEES_DEATH_ACTION,
	LONGHUNT_ATTACK_ACTION,
	LONGHUNT_VICTORY_ACTION,
	LONGHUNT_END_ACTION
} ;





/////////////////////////////////////////////////////////////////////////////
// Structures
/////////////////////////////////////////////////////////////////////////////


// Longhunt stage info
typedef struct LonghuntStageInfo_t
{
	INT32			HealthLimit ;

	FLOAT			AnimSpeed ;

	FLOAT			EvadeTime ;
	INT16			EvadeSkips ;
	FLOAT			NoEvadeTime ;

	INT16			FlinchPercentage ;

	FLOAT			RunAttackTime ;
	INT32			GunFireTotal ;
} CLonghuntStageInfo ;


// Longhunt structure
typedef struct CLonghunt_t
{
	// Generic boss vars - MUST BE HERE AT TOP!
	CBoss			m_Boss ;

	// Stage info
	INT16						m_ScriptStage ;
	CLonghuntStageInfo	*m_pStageInfo ;

	BOOL						m_EvadeActive ;
	FLOAT						m_EvadeTime ;
	INT16						m_EvadeCount ;

	FLOAT						m_RunTime ;
	FLOAT						m_RunAttackTime ;
	FLOAT						m_RunAngle ;

	FLOAT						m_SoundTimer ;
} CLonghunt ;



/////////////////////////////////////////////////////////////////////////////
// Externs
/////////////////////////////////////////////////////////////////////////////
extern	CLonghunt				LonghuntBoss ;



/////////////////////////////////////////////////////////////////////////////
// Functions prototypes
/////////////////////////////////////////////////////////////////////////////
typedef void(*LonghuntSetupFunction)(CGameObjectInstance *pThis, CLonghunt *pLonghunt) ;

CBoss *Longhunt_Initialise(CGameObjectInstance *pThis) ;
void Longhunt_Update(CGameObjectInstance *pThis, CLonghunt *pLonghunt) ;
INT32 Longhunt_SetupMode(CGameObjectInstance *pThis, INT32 nNewMode) ;

int CGameObjectInstance__GetLonghunterModelIndex(CGameObjectInstance *pThis, int nNode) ;

/////////////////////////////////////////////////////////////////////////////
#endif // _INC_LONGHUNT
