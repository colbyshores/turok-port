// aidoor.h

#ifndef _INC_AIDOOR
#define _INC_AIDOOR

/////////////////////////////////////////////////////////////////////////////

#include "aistruc.h"
#include "romstruc.h"
#include "graphu64.h"

// forward declarations
struct CParticle_t;

/////////////////////////////////////////////////////////////////////////////

void		DoorAI_Advance(CGameObjectInstance *pThis);
void		ActionAI_Advance(CGameObjectInstance *pThis) ;
void		TimerActionAI_Advance(CGameObjectInstance *pThis) ;
void		PressureActionAI_Advance(CGameObjectInstance *pThis) ;
void		ConstantActionAI_Advance(CGameObjectInstance *pThis) ;
void		StatueAI_Advance(CGameObjectInstance *pThis) ;
void		WallAI_Advance(CGameObjectInstance *pThis) ;
//void		SpinDoorAI_Advance(CGameObjectInstance *pThis) ;
void		LaserAI_Advance(CGameObjectInstance *pThis) ;
void		ExplosiveTargetAI_Advance(CGameObjectInstance *pThis) ;
void		ElevatorAI_Advance(CGameObjectInstance *pThis) ;
void		PlatformAI_Advance(CGameObjectInstance *pThis) ;
void		DeathElevatorAI_Advance(CGameObjectInstance *pThis) ;
void		CollapsingPlatformAI_Advance(CGameObjectInstance *pThis) ;


void		PortalAI_Advance(CGameObjectInstance *pThis) ;
void		GateAI_Advance(CGameObjectInstance *pThis) ;
void		LockAI_Advance(CGameObjectInstance *pThis) ;
void		DrainAI_Advance(CGameObjectInstance *pThis) ;
void		FloodAI_Advance(CGameObjectInstance *pThis) ;
void		CrystalAI_Advance(CGameObjectInstance *pThis) ;
void		SpinElevatorAI_Advance(CGameObjectInstance *pThis) ;
void		SpinPlatformAI_Advance(CGameObjectInstance *pThis) ;
void		KeyFloorAI_Advance(CGameObjectInstance *pThis) ;
void		FadeInFadeOutAI_Advance(CGameObjectInstance *pThis) ;


// PLATFORMS HAVE GONE!!!!!
#if 0

void		TrexElevatorAI_Advance(CGameObjectInstance *pThis) ;
void		TrexElevatorAI_MEvent_Start(CGameObjectInstance *pThis);

void		TrexPlatformAI_Advance(CGameObjectInstance *pThis) ;
void		TrexPlatformAI_MEvent_Start(CGameObjectInstance *pThis);

#endif


// multi events
void		DoorAI_MEvent_OpenClose(CGameObjectInstance *pThis, BOOL Open);
void		ActionAI_MEvent_Go(CGameObjectInstance *pThis) ;
void		TimerActionAI_MEvent_Go(CGameObjectInstance *pThis) ;
void		PressureActionAI_MEvent_Go(CGameObjectInstance *pThis) ;
void		ConstantActionAI_MEvent_Go(CGameObjectInstance *pThis) ;
void		StatueAI_MEvent_Go(CGameObjectInstance *pThis) ;
void		WallAI_MEvent_Left(CGameObjectInstance *pThis) ;
void		WallAI_MEvent_Right(CGameObjectInstance *pThis) ;
//void		SpinDoorWallAI_MEvent_Spin(CGameObjectInstance *pThis) ;
void		LaserAI_MEvent_Start(CGameObjectInstance *pThis);
void		ExplosiveTargetAI_MEvent_Go(CGameObjectInstance *pThis);
void		ElevatorAI_MEvent_Start(CGameObjectInstance *pThis);
void		PlatformAI_MEvent_Start(CGameObjectInstance *pThis);
void		DeathElevatorAI_MEvent_Start(CGameObjectInstance *pThis);
void		CollapsingPlatformAI_MEvent_Start(CGameObjectInstance *pThis);
void		PortalAI_MEvent_Start(CGameObjectInstance *pThis);
void		LockAI_MEvent_Start(CGameObjectInstance *pThis);
void		DrainAI_MEvent_Start(CGameObjectInstance *pThis);
void		FloodAI_MEvent_Start(CGameObjectInstance *pThis);
void		CrystalAI_MEvent_Start(CGameObjectInstance *pThis, BOOL Sink);
int CGameObjectInstance__GetCrystalModelIndex(CGameObjectInstance *pThis, int nNode) ;
void		SpinElevatorAI_MEvent_Start(CGameObjectInstance *pThis);
void		SpinPlatformAI_MEvent_Start(CGameObjectInstance *pThis);
void		FadeInFadeOutAI_MEvent_ToggleState(CGameObjectInstance *pThis) ;

// single events
void		DoorAI_SEvent_AllowBlockEntry(CInstanceHdr *pThis, CVector3 vPos, BOOL AllowEntry);

void		CInstanceHdr__DeviceParticleCollision(CInstanceHdr *pThis, struct CParticle_t *pParticle);
void		ArrowTarget_ParticleCollision(CInstanceHdr *pThis, struct CParticle_t *pParticle);
void		ExplosiveTarget_ParticleCollision(CGameObjectInstance *pThis);

#if 0
// gone now!!

// Longhunter switches
void		LonghunterSwitchAI_MEvent_Go(CGameObjectInstance *pThis, FLOAT Value) ;
void		LonghunterSwitchAI_Advance(CGameObjectInstance *pThis) ;
#endif


/////////////////////////////////////////////////////////////////////////////
#endif	// _INC_AIDOOR
