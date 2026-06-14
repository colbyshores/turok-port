// humvee.h
/////////////////////////////////////////////////////////////////////////////

#ifndef _INC_HUMVEE
#define _INC_HUMVEE

#include "boss.h"


/////////////////////////////////////////////////////////////////////////////
// Constants
/////////////////////////////////////////////////////////////////////////////




// Modes
enum HumveeModes
{
	HUMVEE_WAIT_FOR_START_MODE,
	HUMVEE_JUMP_INTO_ARENA_MODE,

	HUMVEE_IDLE_MODE,
	HUMVEE_RUN_MODE,
	HUMVEE_TURN_LEFT90_MODE,
	HUMVEE_TURN_RIGHT90_MODE,
	HUMVEE_TURN180_MODE,
	HUMVEE_STOP_MODE,

	HUMVEE_EXPLODE_LEFT_MODE,
	HUMVEE_EXPLODE_RIGHT_MODE,
	HUMVEE_DEATH_MODE,

	HUMVEE_END_MODE
} ;


/////////////////////////////////////////////////////////////////////////////
// Structures
/////////////////////////////////////////////////////////////////////////////



typedef BOOL(*BossFunction)(CGameObjectInstance *pThis, CBoss *pBoss) ;
typedef BOOL(*BossBooleanFunction)(CGameObjectInstance *pThis, CBoss *pBoss) ;


typedef struct FireWeaponState_t
{
	CGameObjectInstance	*pObject ;
	BossBooleanFunction	pCanFireFunction ;
	BossFunction			pFireFunction ;

	INT32		ShotsLeft ;
	FLOAT		SingleShotTime ;
	FLOAT		FireShotsTime ;
} CFireWeaponState ;


typedef struct CFireWeaponInfo_t
{
	INT32		TotalShots ;			// Total shots to fire
	FLOAT		SingleShotSpacing ;	// Time between each shot
	FLOAT		FireShotsSpacing ;	// Time between firing all shots
} CFireWeaponInfo ;


// Humvee stage info
typedef struct HumveeStageInfo_t
{
	INT32					HealthLimit ;
	FLOAT					AnimSpeed ;
	INT16					HitsBeforeExplode ;

	CFireWeaponInfo	FireRocketsInfo ;
	CFireWeaponInfo	FireGunsInfo ;
} CHumveeStageInfo ;


// Humvee structure
typedef struct CHumvee_t
{
	// Generic boss vars - MUST BE HERE AT TOP!
	CBoss					m_Boss ;

	// Stage info
	INT16					m_ScriptStage ;
	CHumveeStageInfo	*m_pStageInfo ;

	FLOAT					m_DistFromCentre ;

	INT16					m_TargetNumber ;

	INT16					m_ExplosionHits ;

	FLOAT					m_CollisionTime ;

	FLOAT					m_WheelVel ;
	FLOAT					m_WheelAngle ;

	FLOAT					m_LightGlareTime ;

	// Sound
	INT32					m_Sound ;			// Current sound playing!
	INT32					m_SoundHandle ;	// Current sound playing!

	// Weapons
	CFireWeaponState	m_GunsState ;
	CFireWeaponState	m_RocketsState ;

} CHumvee ;




/////////////////////////////////////////////////////////////////////////////
// Externs
/////////////////////////////////////////////////////////////////////////////
extern	CHumvee				HumveeBossList[] ;



/////////////////////////////////////////////////////////////////////////////
// Functions prototypes
/////////////////////////////////////////////////////////////////////////////
typedef void(*HumveeSetupFunction)(CGameObjectInstance *pThis, CHumvee *pHumvee) ;

CBoss *Humvee_Initialise(CGameObjectInstance *pThis) ;
BOOL Humvee_CanFire(CGameObjectInstance *pThis, CHumvee *pHumvee) ;

void Humvee_Update(CGameObjectInstance *pThis, CHumvee *pHumvee) ;
INT32 Humvee_SetupMode(CGameObjectInstance *pThis, INT32 nNewMode) ;

void Humvee_SEvent_GeneralSound(CInstanceHdr *pThis, CVector3 vPos, float Value) ;

/////////////////////////////////////////////////////////////////////////////
#endif // _INC_HUMVEE
