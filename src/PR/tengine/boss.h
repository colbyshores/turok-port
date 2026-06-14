// boss.h
/////////////////////////////////////////////////////////////////////////////

#ifndef _INC_BOSS
#define _INC_BOSS

#include "graphu64.h"
#include "particle.h"
#include "simppool.h"
#include "romstruc.h"
#include "collinfo.h"
#include "scene.h"


/////////////////////////////////////////////////////////////////////////////
// Constants
/////////////////////////////////////////////////////////////////////////////

//#define CGameObjectInstance struct CGameObjectInstance_t


#define BOSS_Health(pThis)	(pThis->m_AI.m_Health)
#define BOSS_GetPos(pThis) (pThis->ah.ih.m_vPos)
#define BOSS_GetVel(pThis) ((pThis)->ah.m_vVelocity)

#define BOSS_CollisionHeightOffset(pThis)	(pThis->ah.ih.m_CollisionHeightOffset)
#define BOSS_MODE_INFO(a,b,c,d,f)		{(void *)a, (void *)b, c, d, f}

#define TUROK_Health(pThis)	(pThis->m_AI.m_Health)
#define TUROK_GetPos(pThis)	(pThis->ah.ih.m_vPos)
#define TUROK_GetVel(pThis)	((pThis)->ah.m_vVelocity)


#define TUROK_WAIST_HEIGHT				(5*SCALING_FACTOR)




/////////////////////////////////////////////////////////////////////////////
// Boss flags
/////////////////////////////////////////////////////////////////////////////

#define	BF_INVINCIBLE					(1<<0)
#define	BF_RUN_AT_MODE_ANIM_SPEED	(1<<1)

#define	BF_FLOOR_MODE					(1<<2)
#define	BF_CEILING_MODE				(1<<3)
#define	BF_LEFT_WALL_MODE				(1<<4)
#define	BF_RIGHT_WALL_MODE			(1<<5)
#define	BF_JUMPING_MODE  				(1<<6)
#define	BF_FLYING_MODE					(1<<7)
#define	BF_WALKING_MODE				(1<<8)
#define	BF_RUNNING_MODE				(1<<9)
#define	BF_TAUNT_MODE					(1<<10)
#define	BF_ATTACK_MODE					(1<<11)
#define	BF_EVADE_MODE					(1<<12)
#define	BF_ENTER_MODE					(1<<13)
#define	BF_KICK_IMPACT_MODE			(1<<14)
#define	BF_PUNCH_IMPACT_MODE			(1<<15)
#define	BF_EXPLODE_MODE  				(1<<16)
#define	BF_DEATH_MODE  				(1<<17)
#define	BF_FLAME_MODE  				(1<<18)

#define	BF_CAN_TELEPORT				(1<<19)
#define	BF_CAN_FIRE						(1<<20)
#define	BF_CAN_EVADE					(1<<21)
#define	BF_CAN_FLINCH					(1<<22)
#define	BF_CAN_EXPLODE_FLINCH		(1<<23)

#define	BF_HEAD_TRACK_TUROK			(1<<24)
#define	BF_LASER_ON						(1<<25)
#define	BF_CLEAR_VELS					(1<<26)



/////////////////////////////////////////////////////////////////////////////
// Structures
/////////////////////////////////////////////////////////////////////////////

typedef struct CBossVars_t
{
	INT32		m_SwitchesOn ;							//	Total switches on
	UINT32	m_SwitchFlags ;						// Holds up to 32 switches
	FLOAT		m_TurokSpeed ;							// Will put this in CTMove eventually
	CGameRegion	*m_pRegionList[4] ;				// Misc region list
	FLOAT		m_StartBossTimer ; 					// Used to delay boss starts
	CGameObjectInstance *m_pMantisInstance ;	// Global ptr to mantis boss
	CVector3	m_vTurokLastPos ;						// Position of Turok last frame
	CVector3	m_vTurokDir ;							// Turoks direction
} CBossVars ;


void CBossVars__Construct(CBossVars *pThis) ;


typedef struct CDamage_t
{
	INT16		m_Health ;
	INT8		m_Node1, m_Node2 ;
	UINT32	m_Flags ;
} CDamage ;



// RGB triplet structure
typedef struct t_RGB
{
	FLOAT	Red,Green,Blue, Alpha ;
} CRGB ;


// Simple light structure
typedef struct t_Light
{
	CRGB		Ambient ;
	CRGB		Directional ;
	CVector3	Normal ;
} CLight ;

typedef struct t_PsuedoColor
{
	CRGB	m_WhiteColor ;
	CRGB	m_BlackColor ;
} CPsuedoColor ;


// Boss event flags
#define BF_EVENT1		(1<<0)
#define BF_EVENT2		(1<<1)
#define BF_EVENT3		(1<<2)
#define BF_EVENT4		(1<<3)
#define BF_EVENT5		(1<<4)
#define BF_EVENT6		(1<<5)
#define BF_EVENT7		(1<<6)
#define BF_EVENT8		(1<<7)


typedef struct CShield_t
{
	INT16				m_Health ;			// Health of shield
	INT16				m_StartHealth ;	// Start health of shield
	INT16				m_Mode ;				// Current mode of shield
	INT16				m_TotalColors ;	// Total colors
	CPsuedoColor	*m_pColors ;		// Ptr to list of colors
	FLOAT				m_Alpha ;			// Alpha
	FLOAT				m_Time ;				// Shield fade in/out time
	CParticle		*m_pParticle ;		// Shield particle
	CPsuedoColor	*m_pColor ;			// Current color
	CGameObjectInstance *m_pOwner ;	// Character that owns shield
	INT32				m_SoundHandle ;	// Sound handle number
} CShield ;


// Boss structure
typedef struct CBoss_t
{
	BOOL	m_Alive ;						// TRUE if so
	BOOL	m_Initialized ;				// TRUE if so captain.
	INT32	m_ObjectType ;					// Object type

	INT32	m_StartHealth ;				// Start health
	INT32	m_LastHealth ;					// Last frame of health remaining
	INT32	m_LastPercentageHealth ; 	// Percentage of health remaining
	INT32	m_PercentageHealth ;			// Percentage of health remaining

	INT32	m_EventFlags ;					// Signals animation event

	INT32	m_Mode, m_OldMode, m_LastMode ; 	// Mode vars
	FLOAT	m_ModeTime ;					// Time in current mode
	INT32	m_OldModeFlags, m_ModeFlags ;					// Copy of mode table flags
	FLOAT	m_MoveTime ;					// General move timer
	INT32	m_AnimPlayed ;					// Number of times anim has repeated
	INT32	m_AnimEndFrame ; 				// End frame
	BOOL	m_AnimPastEnd ; 				// End frame has been passed
	INT32	m_AnimRepeats ;	 			// Total cylces to play anim
	INT32	m_Stage ;						// Boss attack stage
	CCollisionInfo2	m_CollisionInfo ;	// Collision info structure for boss

	FLOAT	m_AnimSpeed ;					// Boss anim speed
	FLOAT	m_ModeAnimSpeed ;				// Current mode anim speed
	FLOAT	m_PlaybackAnimSpeed ;		// Final playback anim speed

	FLOAT	m_Speed ;						// Movement speed
	FLOAT	m_RotX, m_RotZ ;

	CVector3	m_LastPos, m_LastVel, m_LastDir ;	// Total last vel

	INT16		m_Action ;					// Current action
	INT16		m_ActionStage ;			// Current action stage
	FLOAT		m_ActionTime ;				// Current action time

	FLOAT			m_LookDeltaAngle ;
	FLOAT			m_DeltaAngle ;
	CVector3		m_vTarget ;
	FLOAT			m_DistFromTarget ;
	FLOAT			m_DistFromTargetSqr ;

	FLOAT			m_Rot1 ;					// Misc rotation vars
	FLOAT			m_Rot2 ;
	FLOAT			m_Rot3 ;
	FLOAT			m_Rot4 ;
	FLOAT			m_Rot5 ;

	FLOAT			m_DistFromTurok ;			// Distance from Turok
	FLOAT			m_TurokAngle ;				// Angle from Turok to boss
	FLOAT			m_TurokAngleDiff ;		// Difference between Turok's facing and turok angle
	FLOAT			m_TurokAngleDist ;		// Absolute distance between Turok's facing and turok angle
	FLOAT			m_AtTargetDist ;			// Max distance to get from target
	FLOAT			m_TurokDeltaAngle ;		// Delta angle to add to face turok
	FLOAT			m_LastTurokDeltaAngle ;	// Last frames Delta angle to add, to face turok
	FLOAT			m_AbsTurokDeltaAngle ;	// Absolute Delta angle to add, to face turok

	CShield		*m_pShield ;					// Ptr to shield

	void(*m_pInitializeFunction)(void *pThis, void *pBoss) ;		// Initialize function
	void(*m_pUpdateFunction)(void *pThis, void *pBoss ) ;			// Update function to call
	void (*m_pHitByParticleFunction)(void *pThis, void *pBoss, void *pParticle) ;

	struct CBossModeInfo_t *m_pModeTable ;						// Pointer to bosses mode table
	void (*m_pPreDrawFunction)(void *pThis, void *pBoss, Gfx **ppDLP) ;
	void (*m_pPostDrawFunction)(void *pThis, void *pBoss, Gfx **ppDLP) ;
	void (*m_pPauseFunction)(void *pThis, void *pBoss) ;
	void (*m_pRestoreFunction)(void *pThis, void *pBoss) ;

#ifndef MAKE_CART
	void (*m_pDisplayModeFunction)(void *pBoss) ;			// Debug function
#endif

} CBoss ;





// Mode table structure
typedef struct CBossModeInfo_t
{
	void(*m_pSetupFunction)(CGameObjectInstance *pThis, CBoss *pBoss) ;
	void(*m_pUpdateFunction)(CGameObjectInstance *pThis, CBoss *pBoss) ;
	INT32	m_Anim ;
	FLOAT	m_AnimSpeed ;
	INT32	m_Flags ;
} CBossModeInfo ;






/////////////////////////////////////////////////////////////////////////////
// Misc structures
/////////////////////////////////////////////////////////////////////////////


// Mode script structure
typedef struct CBossScript_t
{
	INT16		*m_pStart ;		// Ptr to start of script
	INT16		*m_pCurrent ;	// Ptr to current script position
} CBossScript ;



// Flinch info structure
typedef struct CFlinchInfo_t
{
	FLOAT	RotYOffset ;		// Rotation direction offset

	FLOAT	HeadHeight ;		// Bottom of head
	FLOAT	ShoulderHeight ;	// Bottom of shoulders
	FLOAT	StomachHeight ;	// Bottom of stomach

	INT32	HeadMode ;				// Head flinch mode
	INT32	BackMode ;				// Back flinch mode
	INT32	StomachMode ;			// Stomach flinch mode
	INT32	KneeMode ;				// Knee flinch mode
	INT32	LeftShoulderMode ;	// Left shoulder flinch mode
	INT32	RightShoulderMode ;	// Right shoulder flinch mode

} CFlinchInfo ;


/////////////////////////////////////////////////////////////////////////////
// Functions
/////////////////////////////////////////////////////////////////////////////

void CBoss__Construct(CBoss *pThis) ;

void BOSS_Initialise(CGameObjectInstance *pThis, INT32 nObjectType) ;


void BOSS_LevelComplete(void) ;

BOOL BOSS_SetupMode(CGameObjectInstance *pThis, INT32 nNewMode) ;
void BOSS_Update(CGameObjectInstance *pThis) ;
void BOSS_DoNodeDamage(CGameObjectInstance *pThis, INT32 Node) ;

void BOSS_CalculateAngles(CGameObjectInstance *pThis, CBoss *pBoss) ;
INT32 BOSS_InProjectilePath(CGameObjectInstance *pThis, CBoss *pBoss, FLOAT AngleRange) ;

void BOSS_PerformTurn(CGameObjectInstance *pThis, CBoss *pBoss, FLOAT TurnSpeed, FLOAT DeltaAngle) ;
void BOSS_RotateToFaceTurok(CGameObjectInstance *pThis, CBoss *pBoss, FLOAT Speed) ;

INT32	BOSS_HitByParticleFlinchMode(CGameObjectInstance *pThis, CBoss *pBoss, CFlinchInfo *pInfo, CParticle *pParticle) ;

INT32 BOSS_GetClosestPositionIndex(CGameObjectInstance *pThis, CVector3 *pvPos, INT32 Length) ;

void BOSS_SetPosition(CGameObjectInstance *pThis, FLOAT x, FLOAT z) ;

BOOL BOSS_OffsetFromPlayerObstructed(CGameObjectInstance *pThis, FLOAT Radius, FLOAT AngleOffset, CGameRegion **ppRegion, CVector3 *pvOutPos) ;


/////////////////////////////////////////////////////////////////////////////
// Generic mode functions
/////////////////////////////////////////////////////////////////////////////

void BOSS_Code_NextModeAfterAnim(CGameObjectInstance *pThis, CBoss *pBoss) ;


/////////////////////////////////////////////////////////////////////////////
// Orientation functions
/////////////////////////////////////////////////////////////////////////////

CQuatern BOSS_CalculateOrientation(CGameObjectInstance *pThis) ;


/////////////////////////////////////////////////////////////////////////////
// Misc functions
/////////////////////////////////////////////////////////////////////////////

FLOAT AccelerateFLOAT(FLOAT Var, FLOAT Accel, FLOAT MaxSpeed) ;
FLOAT DecelerateFLOAT(FLOAT Var, FLOAT Decel) ;

#define	AccelerateVarFLOAT(pVar, Accel, MaxSpeed)		*(pVar) = AccelerateFLOAT(*(pVar), Accel, MaxSpeed)
#define	DecelerateVarFLOAT(pVar, Decel)					*(pVar) = DecelerateFLOAT(*(pVar), Decel)


INT32 AccelerateINT32(INT32 Var, INT32 Accel, INT32 MaxSpeed) ;

FLOAT ZoomFLOAT(FLOAT Var, FLOAT Target, FLOAT Speed) ;
void ProportionalZoomVarFLOAT(FLOAT *pVar, FLOAT Target, FLOAT Speed) ;

FLOAT AngleDist(FLOAT Phi, FLOAT Theta) ;
FLOAT AngleDiff(FLOAT Phi, FLOAT Theta) ;

FLOAT RandomRangeFLOAT(FLOAT a, FLOAT b) ;
BOOL DecrementTimer(FLOAT *pTimer) ;


void CInstanceHdr__NearestPosition(CInstanceHdr *pThis, CVector3 *pvDestPos, CVector3 *pvOutPos) ;

/////////////////////////////////////////////////////////////////////////////
// Pickup functions
/////////////////////////////////////////////////////////////////////////////

INT16 ChooseHealthPickup(void) ;
INT16 ChooseWeaponPickupTurokIsHolding(void) ;


/////////////////////////////////////////////////////////////////////////////
// Shield functions
/////////////////////////////////////////////////////////////////////////////

void CShield__Construct(CShield *pThis,
								CGameObjectInstance *pOwner,
								INT32 Health, CPsuedoColor *pColors, INT32 TotalColors) ;

BOOL CShield__IsOn(CShield *pThis) ;
void CShield__Update(CShield *pThis) ;
BOOL CShield__Activate(CShield *pThis, FLOAT Time) ;
void CShield__Deactivate(CShield *pThis) ;



/////////////////////////////////////////////////////////////////////////////
// Boss status functions
/////////////////////////////////////////////////////////////////////////////

#define MAX_BOSS_DATA_SIZE	800
#define MAX_BOSS_EVENTS		20
#define MAX_BOSS_PICKUPS	20

// Structure used to store all events on a level
typedef struct CBossEvent_t
{
	CInstanceHdr	*m_pInst ; 	// Instance that sent event
	CVector3			m_vPos ;		// Position of event
	int				m_nType ;  	// Type of event
	float			 	m_Value ;   // Value associated with event
} CBossEvent ;

// Structure used to store all pickups on a level
typedef struct CBossPickup_t
{
	int			 m_nType ;		// Type of pickup
	CVector3		 m_vPos ;		// Position of pickup
} CBossPickup ;

// Structure used to save a bosses status before cinema's etc.
typedef struct CBossData_t
{
	CGameObjectInstance	*m_pObject ;							// Ptr to object instance
	CBoss						*m_pBossVars ;							// Ptr to boss vars

	void (*m_pCollisionFunction)(CGameObjectInstance *pThis, CCollisionInfo2 *pCollisionInfo) ;

	CVector3					m_vPos ;									// Position of boss
	float						m_RotY ;									// Y rotation of boss

	BOOL						m_IsGone ;								// "Gone" status

	INT32						m_Anim ;									// Animation
	INT32						m_CurrentAnimFrame ;	 				// Frame of animation

	UINT32					m_BossDataSize ;						// Size of boss data
	UINT8						m_BossData[MAX_BOSS_DATA_SIZE] ;	// Boss vars

	CAIDynamic				m_AIDynamicData ;						// Dynamic data

	INT16						m_Mode ;			// General purpose mode for fading out ect // use bitfield for mode?
	FLOAT						m_ModeTime ;	// Current mode time
	FLOAT						m_ModeMisc1 ;	// Miscilaneous variable 1

} CBossData ;

// Structure used to save status of a bunch of bosses
typedef struct CBossesStatus_t
{
	CGameObjectInstance	*m_pBossActive ;						// Current active boss

	CBossData		m_BossData[3] ;								// Contains bosses status

	CBossEvent		m_EventData[MAX_BOSS_EVENTS] ;			// All events sent on level
	INT32				m_CurrentEvent ;

	BOOL				m_PickupMonitorActive ;
	CBossPickup		m_PickupData[MAX_BOSS_PICKUPS] ;			// All pickups on level
	INT32				m_CurrentPickup ;

} CBossesStatus ;


INT32 CGameObjectInstance__GetBossSaveIndex(CGameObjectInstance *pThis) ;
INT32 CGameObjectInstance__GetBossDataSize(CGameObjectInstance *pThis) ;

void CBossesStatus__Construct(CBossesStatus *pThis) ;
void CBossesStatus__SaveBossStatus(CBossesStatus *pThis, CGameObjectInstance *pObject) ;
void CBossesStatus__RestoreBossStatus(CBossesStatus *pThis, CGameObjectInstance *pObject) ;
CBossData *CBossesStatus__BossWasSaved(CBossesStatus *pThis, CGameObjectInstance *pObject) ;

void CBossesStatus__SaveAllBossesStatus(CBossesStatus *pThis) ;
void CBossesStatus__RestoreAllBossesStatus(CBossesStatus *pThis) ;

void CBossesStatus__SaveEvent(CBossesStatus *pThis, CInstanceHdr *pInst, int nType, CVector3 vPos, float Value) ;


/////////////////////////////////////////////////////////////////////////////
#endif // _INC_BOSS
