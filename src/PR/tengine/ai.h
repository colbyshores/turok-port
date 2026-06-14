// ai.h

#ifndef _INC_AI
#define _INC_AI

/////////////////////////////////////////////////////////////////////////////

#include "aistruc.h"
#include "romstruc.h"
#include "graphu64.h"
#include "scaling.h"

/////////////////////////////////////////////////////////////////////////////

// main ai stuff structures etc.

// ultra 64
#define AI_GetEA(pThis) ((pThis)->ah.ih.m_pEA)
#define AI_GetDyn(pThis) (&((pThis)->m_AI))
#define AI_GetPos(pThis) ((pThis)->ah.ih.m_vPos)
#define AI_GetVelocity(pThis) (&(pThis)->ah.m_vVelocity)

extern	BOOL AICollidedWithWall ;


// damage types
#define	DAMAGE_NORMAL		0
#define	DAMAGE_BLADE		1
#define	DAMAGE_KICK			2
#define	DAMAGE_PARTICLE	3


// avoidance distances
#define AVOIDANCE_RADIUS_DISTANCE_FACTOR				1.5
#define AVOIDANCE_FLYING_RADIUS_DISTANCE_FACTOR		4


/////////////////////////////////////////////////////////////////////////////

// external declarations
BOOL						IsExplosionEvent					(int Event);
BOOL						IsSingleEvent						(int Event);
BOOL						SendEventToWholeWorld			(int Event);
void						AI_Single_Event					(CInstanceHdr *pThis, CInstanceHdr *pOrigin, int Event, CVector3 vPos, float Value);
void						AI_Event_Dispatcher				(CInstanceHdr *pThis, CInstanceHdr *pOrigin, int Event, DWORD dwSpecies, CVector3 vPos, float Value);
void						AI_Advance							(CGameObjectInstance *pAI);																	// advance ai
CGameObjectInstance	*AI_Advance_Gallery				(CGameObjectInstance *pMe);
unsigned char			AI_Cycle_Completed				(CGameObjectInstance *pAI);																	// wait for cycle completed
void						AI_Multi_Event						(CGameObjectInstance *pAI, CInstanceHdr *pOrigin, int event, CVector3 vPos, float Value);
BOOL						AI_Can_See_Target					(CGameObjectInstance *pMe, CGameObjectInstance *pTarget);
BOOL						AI_Make_Retreat					(CGameObjectInstance *pAI, float ang);
BOOL						AI_Make_Run							(CGameObjectInstance *pAI, float ang);														// make ai run towards angle direction
void						AI_Make_Walk						(CGameObjectInstance *pAI, float ang);														// make ai walk towards angle direction
BOOL						AI_Make_Turn						(CGameObjectInstance *pAI, float ang);														// make ai turn towards angle direction
void						AI_Increase_Agitation			(CGameObjectInstance *pAI, short amt, short max);										// increase agitation of ai
void						AI_Decrease_Agitation			(CGameObjectInstance *pAI, short amt);														// decrease agitation of ai
short						AI_Get_Agitation_Level			(CGameObjectInstance *pAI);																	// returns agitation state of ai
void						AI_Make_Get_Attention			(CGameObjectInstance *pMe);																	// make ai get attention
void						AI_Decrease_Health				(CGameObjectInstance *pAI, int hits, BOOL FlashScreen, BOOL DoSounds);
BOOL						AI_SetDesiredAnim					(CGameObjectInstance *pAI, int nAnim, BOOL InterruptBlend);
BOOL						AI_SetDesiredAnimByIndex		(CGameObjectInstance *pAI, int nAnim, BOOL InterruptBlend);
void						AI_DoHit								(CGameObjectInstance *pAI, int hits);
void						AI_DoParticle						(CInstanceHdr *pThis, int nType, CVector3 vPos);
void						AI_DoParticleSpread				(CAnimInstanceHdr *pThis, int nType, CVector3 vPos, int nParticles);
#define					AI_DoParticleAtPlayer(pThis, nType, vPos) AI_AimParticleAtPlayer((pThis), (nType), (vPos), ANGLE_DTOR(45))
void						AI_AimParticleAtPlayer			(CInstanceHdr *pThis, int nType, CVector3 vPos, FLOAT MaxYRot);
void						AI_GeneratePickups				(CGameObjectInstance *pThis, CVector3 vPos);
struct CDynamicSimple_t *AI_DoPickup(CInstanceHdr *pThis, CVector3 vPos, int nType, FLOAT Time) ;
int						AI_DoSound							(CInstanceHdr *pThis, int nSound, float Volume, int SndFlags);
float						AI_GetAngle							(CGameObjectInstance *pAI, CVector3 vPos);
float						AI_GetAvoidanceAngle				(CGameObjectInstance *pAI, CVector3 vPos, CGameObjectInstance *pTarget, float radiusFactor);
void						AI_NudgeRotY						(CGameObjectInstance *pAI, float Angle);
float						AI_DistanceSquared				(CGameObjectInstance *pAI, CVector3 vPos);
float						AI_XZDistanceSquared				(CGameObjectInstance *pAI, CVector3 vPos);
int						AI_GetCurrentAnim					(CGameObjectInstance *pAI);
void						AI_Get_Attack_Destination		(CGameObjectInstance *pAI);
int						AI_KnifeTomahawkDamage			(CGameObjectInstance *This, CInstanceHdr *pOrigin, CVector3 vPos, float rad, int nKnife);
int						AI_GetMaterial						(CInstanceHdr *pThis);
void						AI_SetAttacker						(CGameObjectInstance *pThis, CInstanceHdr *pOrigin);
void						AI_FeignDeath						(CGameObjectInstance *pThis);
void						AI_Resurrect						(CGameObjectInstance *pThis);
int						AI_GetWaterStatus					(CGameObjectInstance *pThis);
float						AI_GetWaterHeight					(CGameObjectInstance *pThis);
void						AI_Make_Swim						(CGameObjectInstance *pAI, float ang);
void						AI_Make_Swim_AtHeight			(CGameObjectInstance *pAI);
void						AI_Make_SwimRetreat				(CGameObjectInstance *pAI, float ang);
void						AI_StayInWater						(CGameObjectInstance *pThis);
BOOL						AI_DoesItFloat						(CGameObjectInstance *pThis);
BOOL						AI_Make_Teleport_Away			(CGameObjectInstance *pAI);
BOOL						AI_Make_Teleport_Appear			(CGameObjectInstance *pAI);
void						AI_Make_Teleport					(CGameObjectInstance *pAI);
void						AI_Make_TeleportMoveSlow		(CGameObjectInstance *pAI, float ang);
BOOL						AI_CanWeKillIt						(CGameObjectInstance *pThis);
BOOL						AI_CanWeRemoveIt					(CGameObjectInstance *pThis);
BOOL						AI_DoesItMelt						(CGameObjectInstance *pThis);
BOOL						AI_IsMortalWound					(CGameObjectInstance *pThis);
BOOL						AI_IsPickup							(int nType);
BOOL						AI_IsWarp							(int nType);
BOOL						AI_IsMorph							(int nType);
BOOL						AI_IsBouncy							(int nType);
BOOL						AI_DoesItAvoidWater				(CGameObjectInstance *pThis);
BOOL						AI_DoesItRegenerate				(CGameObjectInstance *pThis);
BOOL						AI_DoTeleport						(CGameObjectInstance *pAI, CVector3 vDestPos);
BOOL						AI_DoesItFireProjectiles		(CGameObjectInstance *pThis);
void						AI_CompletelyKillOff				(CGameObjectInstance *pThis);
BOOL						AI_IsSfxUnderwater				(int nType);
void						AI_UpdateMeltTimer				(CGameObjectInstance *pThis);
float						AI_GetAvoidanceRadiusFactor	(CGameObjectInstance *pThis);

CVector3					AI_GetZigZagPosition(CGameObjectInstance *pThis, CGameObjectInstance *pTarget) ;


// multi events
void AI_MEvent_GetAttention						(CGameObjectInstance *pAI, CInstanceHdr *pOrigin, CVector3 vPos);
void AI_MEvent_QuietNoise							(CGameObjectInstance *pAI, CVector3 vPos);
void AI_MEvent_LoudNoise							(CGameObjectInstance *pAI, CVector3 vPos);
void AI_MEvent_Damage								(CGameObjectInstance *pAI, CInstanceHdr *pOrigin, CVector3 vPos, int hits, float rad, int nType);
void AI_MEvent_Explosion							(CGameObjectInstance	*pAI,
															 CInstanceHdr			*pOrigin,
															 CVector3				vPos,
															 float					rad,
															 float					ExpHits,
															 BOOL						HurtPlayer,
															 BOOL						EnemyFired,
															 BOOL						DontHurtBigCreatures,
															 float					ForceMultiplier,
															 BOOL						AllowDamageToDevices);
void AI_MEvent_WeaponImpact						(CGameObjectInstance *pAI, CInstanceHdr *pOrigin, CVector3 vPos, float rad);
void AI_MEvent_NudgeBack							(CGameObjectInstance *pAI, CVector3 vPos, float Value);
void AI_MEvent_KnifeLeft_Damage					(CGameObjectInstance *pAI, CInstanceHdr *pOrigin, CVector3 vPos, float Value);
void AI_MEvent_KnifeRight_Damage					(CGameObjectInstance *pAI, CInstanceHdr *pOrigin, CVector3 vPos, float Value);
void AI_MEvent_KnifeForward_Damage				(CGameObjectInstance *pAI, CInstanceHdr *pOrigin, CVector3 vPos, float Value);
void AI_MEvent_TomahawkLeft_Damage				(CGameObjectInstance *pAI, CInstanceHdr *pOrigin, CVector3 vPos, float Value);
void AI_MEvent_TomahawkRight_Damage				(CGameObjectInstance *pAI, CInstanceHdr *pOrigin, CVector3 vPos, float Value);
void AI_MEvent_TomahawkForward_Damage			(CGameObjectInstance *pAI, CInstanceHdr *pOrigin, CVector3 vPos, float Value);
void AI_MEvent_Shockwave_Damage					(CGameObjectInstance *pAI, CInstanceHdr *pOrigin, CVector3 vPos, float Value);
void AI_MEvent_PressurePlate						(CGameObjectInstance *pAI, CInstanceHdr *pOrigin, CVector3 vPos, float Value);
void AI_MEvent_Statue								(CGameObjectInstance *pAI, CInstanceHdr *pOrigin, CVector3 vPos, float Value);
void AI_MEvent_Pillar								(CGameObjectInstance *pAI, CInstanceHdr *pOrigin, CVector3 vPos, float Value);
void AI_MEvent_SlideWall							(CGameObjectInstance *pAI, CInstanceHdr *pOrigin, CVector3 vPos, float Value);
void AI_MEvent_Freeze								(CGameObjectInstance *pAI, CInstanceHdr *pOrigin, CVector3 vPos, float rad, float ExpHits);
void AI_MEvent_ShoveWithCamera					(CGameObjectInstance *pAI, CInstanceHdr *pOrigin, CVector3 vPos, float Value);


// single events
void AI_SEvent_AcidIncoming						(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_AcidSpitPart						(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_AcidSpitSnd							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_BlowGunFire							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_BreathPart							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_BreathSnd							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_ChargeHeadSlam						(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_DeathRainPart						(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_DeathRainSnd						(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Earthquake							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_EnergyBlastPart					(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_EnergyBlastSnd						(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_ExplosiveLaunched					(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_EyeFirePart							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_EyeFireSound						(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_FireBallPart						(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_FireBallSnd							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_JawSnap								(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_ShockWavePart						(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_ShockWaveSnd						(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_SpellCastPart						(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_SpellCastSnd						(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_SwingFistsWeapon					(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_WeaponFire							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_WeaponRound							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_WeaponCock							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_LookOffset							(CInstanceHdr *pThis, float Value);
void AI_SEvent_Test									(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_SurfaceImpact						(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Chitter								(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Alert									(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Hydraulics							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Shutdown								(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Swivel								(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_JetpackOutofControl				(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_BurstFromGround					(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_ChestThump							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Death									(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Explode								(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Footfall								(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Grunt									(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Gurgle								(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Handfall								(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Hiss									(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Hitground							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Licking								(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Rage									(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_RockNoise1							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_RubbingSkin							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Scratch								(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Screech								(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Sniff									(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Swoosh								(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_TurntoStone							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_ViolentDeath						(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_WingFlap								(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Growl									(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Alert1								(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Alert2								(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Alert3								(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Alert4								(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Death1								(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Death2								(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Death3								(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Death4								(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Injury1								(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Injury2								(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Injury3								(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Long1									(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Long2									(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Long3									(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Taunt1								(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Taunt2								(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Taunt3								(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_ViolentDeath1						(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_ViolentDeath2						(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_ScreenShake							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Invincible							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_BloodArcParticles					(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_MuzzleFlashParticles				(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_HulkBlasterParticles				(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_AlienDeathFireParticles			(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_KnifeForwardParticles			(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_KnifeLeftParticles				(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_KnifeRightParticles				(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_GreenBloodArcParticles			(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_SpurtBloodArcParticles			(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_FountainBloodArcParticles		(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_GushBloodArcParticles			(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_DroolParticles						(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_ArterialBloodParticles			(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_SpurtGBloodParticles				(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_FountainGBloodParticles			(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_GushGBloodParticles				(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_ArterialGBloodParticles			(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_KnifeTomahawkBlood_Particles	(CInstanceHdr *pThis, CVector3 vPos, int nTomahawk, int nHits);
void AI_SEvent_WaterDropParticles				(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_WaterFoamParticles				(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_WaterSteamParticles				(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Arrow_Damage						(CInstanceHdr *pThis, CInstanceHdr *pOrigin, CVector3 vPos, float rad);
void AI_SEvent_InstantDeath						(CInstanceHdr *pThis);
void AI_SEvent_Damage_Target						(CInstanceHdr *pThis, CInstanceHdr *pOrigin, CVector3 vPos, float damage);
void AI_SEvent_DamageNudge_Target				(CInstanceHdr *pThis, CInstanceHdr *pOrigin, CVector3 vPos, float damage);
void AI_SEvent_Generic1								(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic2								(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic3								(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic4								(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic5								(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic6								(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic7								(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic8								(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic9								(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic10							(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic11							(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic12							(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic13							(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic14							(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic15							(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic16							(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic17							(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic18							(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic19							(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic20							(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic21							(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic22							(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic23							(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic24							(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic25							(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic26							(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic27							(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic28							(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic29							(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic30							(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic31							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic32							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic33							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic34							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic35							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic36							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic37							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic38							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic39							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic40							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic41							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic42							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic43							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic44							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic45							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic46							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic47							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic48							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic49							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic50							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic51							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic52							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic53							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic54							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic55							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic56							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic57							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic58							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic59							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic60							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic61							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic62							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic63							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic64							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic65							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic66							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic67							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic68							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic69							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic70							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic71							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic72							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic73							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic74							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic75							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic76							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic77							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic78							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic79							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic80							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic81							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic82							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic83							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic84							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic85							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic86							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic87							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic88							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic89							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic90							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic91							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic92							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic93							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic94							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic95							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic96							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic97							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic98							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic99							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic100							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic101							(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic102							(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic103							(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic104							(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic105							(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic106							(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic107							(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic108							(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic109							(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic110							(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic111							(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic112							(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic113							(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic114							(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic115							(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic116							(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic117							(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic118							(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic119							(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic120							(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic121							(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic122							(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic123							(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic124							(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic125							(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic126							(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic127							(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic128							(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic129							(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic130							(CInstanceHdr *pThis, CVector3 vPos, float damage);
void AI_SEvent_Generic131							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic132							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic133							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic134							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic135							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic136							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic137							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic138							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic139							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic140							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic141							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic142							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic143							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic144							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic145							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic146							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic147							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic148							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic149							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic150							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic151							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic152							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic153							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic154							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic155							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic156							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic157							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic158							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic159							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic160							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic161							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic162							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic163							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic164							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic165							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic166							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic167							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic168							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic169							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic170							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic171							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic172							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic173							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic174							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic175							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic176							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic177							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic178							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic179							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic180							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic181							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic182							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic183							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic184							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic185							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic186							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic187							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic188							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic189							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic190							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic191							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic192							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic193							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic194							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic195							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic196							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic197							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic198							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic199							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic200							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic201							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic202							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic203							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic204							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic205							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic206							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic207							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic208							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic209							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic210							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic211							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic212							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic213							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic214							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic215							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic216							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic217							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic218							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic219							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic220							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic221							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic222							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic223							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic224							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic225							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic226							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic227							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic228							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic229							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic230							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic231							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic232							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic233							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic234							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic235							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic236							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic237							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic238							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic239							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic240							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic241							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic242							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic243							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic244							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic245							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic246							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic247							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic248							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic249							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic250							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic251							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic252							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic253							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic254							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic255							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic256							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic257							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic258							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic259							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generic260							(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Plasma2Particle					(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Water_Bubble						(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Water_Ripple						(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Water_Splash						(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_GeneralSound						(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_GeneratePickups					(CInstanceHdr *pThis, CVector3 vPos);
void AI_SEvent_MakeInvisible						(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_ScreenTremor						(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_Generate_Pickup					(CInstanceHdr *pThis, CVector3 vPos, float Value);
void AI_SEvent_RandBigSwooshSnd(CInstanceHdr *pThis, CVector3 vPos, float Value ) ;
void AI_SEvent_RandSmallSwooshSnd(CInstanceHdr *pThis, CVector3 vPos, float Value ) ;
void AI_SEvent_RandGruntSnd(CInstanceHdr *pThis, CVector3 vPos, float Value ) ;
void AI_SEvent_StartFireSound(CInstanceHdr *pThis, CVector3 vPos, float Value) ;
void AI_SEvent_StopFireSound(CInstanceHdr *pThis, CVector3 vPos, float Value) ;



/////////////////////////////////////////////////////////////////////////////
#endif	// _INC_AI
