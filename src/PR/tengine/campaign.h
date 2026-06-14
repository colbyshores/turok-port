// campaigner.h
/////////////////////////////////////////////////////////////////////////////

#ifndef _INC_CAMPAIGNER
#define _INC_CAMPAIGNER

#include "boss.h"


/////////////////////////////////////////////////////////////////////////////
// Constants
/////////////////////////////////////////////////////////////////////////////




// Modes
enum CampaignerModes
{
	// In hole modes
	CAMPAIGNER_IN_HOLE_IDLE_MODE,		// 0
	CAMPAIGNER_IN_HOLE_ATTACK_MODE,	// 1

	// Taunt modes
	CAMPAIGNER_TALK_MODE,				// 2
	CAMPAIGNER_RAGE_MODE,				// 3
	CAMPAIGNER_VICTORY_MODE,			// 4
				  
	// Stand modes
	CAMPAIGNER_IDLE_MODE,				// 5
	CAMPAIGNER_TURN180_MODE,			// 6
	CAMPAIGNER_TURN_LEFT90_MODE,		// 7
	CAMPAIGNER_TURN_RIGHT90_MODE,		// 8

	// Walk modes
	CAMPAIGNER_WALK_MODE,					// 9
	CAMPAIGNER_WALK_TURN180_MODE,			// 10
	CAMPAIGNER_WALK_TURN_LEFT90_MODE,	// 11
	CAMPAIGNER_WALK_TURN_RIGHT90_MODE,	// 12

	// Limp modes
	CAMPAIGNER_LIMP_MODE,					// 13
	CAMPAIGNER_LIMP_TURN180_MODE,			// 14
	CAMPAIGNER_LIMP_TURN_LEFT90_MODE,	// 15
	CAMPAIGNER_LIMP_TURN_RIGHT90_MODE,	// 16

	// Run modes
	CAMPAIGNER_RUN_MODE,						// 17
	CAMPAIGNER_RUN_TURN180_MODE,			// 18
	CAMPAIGNER_RUN_TURN_LEFT90_MODE,		// 19
	CAMPAIGNER_RUN_TURN_RIGHT90_MODE,	// 20

	// Normal close up weapon attacks
	CAMPAIGNER_RUN_WEAPON_OVERHEAD_SMASH_MODE,
	CAMPAIGNER_WEAPON_FOREARM_SLASH_MODE,
	CAMPAIGNER_WEAPON_BACKHAND_SLASH_MODE,
	CAMPAIGNER_WEAPON_OVERHEAD_SLASH_MODE,
	CAMPAIGNER_WEAPON_FOREARM_SLASH2_MODE,
	CAMPAIGNER_WEAPON_BACKHAND_SLASH2_MODE,
	CAMPAIGNER_WEAPON_OVERHEAD_SLASH2_MODE,

	// Normal close up shield attacks
	CAMPAIGNER_ATTACK_BACKHAND_MODE,
	CAMPAIGNER_ATTACK_SWIPE_MODE,
	CAMPAIGNER_ATTACK_BLOW_MODE,

	// Magic attacks
	CAMPAIGNER_MAGIC_ENERGY_BLAST_MODE,
	CAMPAIGNER_MAGIC_SUMMON_EXPLOSION_MODE,
	CAMPAIGNER_MAGIC_SUPER_BLAST_MODE,
	CAMPAIGNER_MAGIC_SCATTER_BLAST_MODE,

	// Specials
	CAMPAIGNER_TELEPORT_MODE,

	// Weapon modes
	CAMPAIGNER_WEAPON_IDLE_MODE,
	CAMPAIGNER_WEAPON_TURN180_MODE,
	CAMPAIGNER_WEAPON_TURN_LEFT90_MODE,
	CAMPAIGNER_WEAPON_TURN_RIGHT90_MODE,
	CAMPAIGNER_WEAPON_LAUGH_MODE,
	CAMPAIGNER_WEAPON_ATTACK_MODE,

	// Evades
	CAMPAIGNER_EVADE_BACK_FLIP_MODE,
	CAMPAIGNER_EVADE_MEGA_JUMP_MODE,

	// Flinch modes
	CAMPAIGNER_RIGHT_SHOULDER_FLINCH_MODE,
	CAMPAIGNER_LEFT_SHOULDER_FLINCH_MODE,
	CAMPAIGNER_CHEST_FLINCH_MODE,
	CAMPAIGNER_BACK_FLINCH_MODE,
	CAMPAIGNER_DAMAGE_SHORTING_OUT_MODE,
	CAMPAIGNER_KNEE_FALL_FLINCH_MODE,
	CAMPAIGNER_KNEE_GETUP_MODE,
	
	// Impacts
	CAMPAIGNER_EXPLODE_FRONT_MODE,
	CAMPAIGNER_EXPLODE_BACK_MODE,
	CAMPAIGNER_EXPLODE_LEFT_MODE,
	CAMPAIGNER_EXPLODE_RIGHT_MODE,

	// Cinemas
	CAMPAIGNER_DEATH_MODE,
	CAMPAIGNER_KILL_TUROK_MODE,

	CAMPAIGNER_END_MODE
} ;






/////////////////////////////////////////////////////////////////////////////
// Structures
/////////////////////////////////////////////////////////////////////////////


// Campaigner stage info
typedef struct CampaignerStageInfo_t
{
	INT32			HealthLimit ;

	FLOAT			AnimSpeed ;

	FLOAT			RunAttackTime ;

	INT16			EvadePercentage ;
	FLOAT			EvadeSpacing ;

	INT16			FlinchPercentage ;
	FLOAT			FlinchSpacing ;

	INT16			ExplodeFlinchPercentage ;
	FLOAT			ExplodeFlinchSpacing ;

	INT16			ShortOutPercentage ;
	FLOAT			ShortOutSpacing ;

	INT16			BlockWeaponsPercentage ;
} CCampaignerStageInfo ;


// Campaigner structure
typedef struct CCampaigner_t
{
	// Generic boss vars - MUST BE HERE AT TOP!
	CBoss			m_Boss ;

	// Campaigner variables
	CCampaignerStageInfo		*m_pStageInfo ;
	CSpecialFx					m_SpecialFx ;

	INT16							m_HoleNumber ;		// Current hole number
	CVector3						m_vTeleportDest ;	// Position to teleport
	BOOL							m_Limp ;				// TRUE if limping

	BOOL							m_TeleportToHole ;
	BOOL							m_Teleported ;
	BOOL							m_StartedFx ;

	FLOAT							m_RunTime ;
	FLOAT							m_RunAttackTime ;
	FLOAT							m_ExplosionAngle ;

	FLOAT							m_EvadeTime ;
	FLOAT							m_FlinchTime ;
	FLOAT							m_ExplodeFlinchTime ;
	FLOAT							m_ShortOutTime ;

	CShield						m_Shield ;

	INT32							m_WeaponToTakeOff ;

	CDamage						*m_pDamage ;

	FLOAT							m_TauntTimer ;
	FLOAT							m_PlayingSoundTimer ;

	INT32							m_LastSound ;

	CBossScript					m_RunAttackScript ;	// Keeps track of run attacks
} CCampaigner ;


/////////////////////////////////////////////////////////////////////////////
// Externs
/////////////////////////////////////////////////////////////////////////////
extern	CCampaigner				CampaignerBoss ;



/////////////////////////////////////////////////////////////////////////////
// Functions prototypes
/////////////////////////////////////////////////////////////////////////////
typedef void(*CampaignerSetupFunction)(CGameObjectInstance *pThis, CCampaigner *pCampaigner) ;

CBoss *Campaigner_Initialise(CGameObjectInstance *pThis) ;
void Campaigner_Update(CGameObjectInstance *pThis, CCampaigner *pCampaigner) ;
INT32 Campaigner_SetupMode(CGameObjectInstance *pThis, INT32 nNewMode) ;


int CGameObjectInstance__GetCampaignerTextureIndex(CGameObjectInstance *pThis, int nNode) ;
int CGameObjectInstance__GetCampaignerModelIndex(CGameObjectInstance *pThis, int nNode) ;


/////////////////////////////////////////////////////////////////////////////
#endif // _INC_CAMPAIGNER
