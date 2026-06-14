// aistand.h

#ifndef _INC_AISTAND
#define _INC_AISTAND

/////////////////////////////////////////////////////////////////////////////

// include ai stuff
#include "ai.h"

/////////////////////////////////////////////////////////////////////////////


// external definitions
void		AI_Standard_Idle					(CGameObjectInstance *pMe);		// do standard idle for ai
void		AI_Standard_Agitated				(CGameObjectInstance *pMe);		// do standard agitated mood for ai
void		AI_Standard_Attack				(CGameObjectInstance *pMe, CGameObjectInstance *pDest);		// do standard attack mode for ai
void		AI_Standard_Death					(CGameObjectInstance *pMe);		// do standard death mode for ai
void		AI_Standard_Hit					(CGameObjectInstance *pMe);		// do standard hit
void		AI_Standard_Explosion			(CGameObjectInstance *pMe);		// do standard explosion
void		AI_Standard_Projectile_Attack	(CGameObjectInstance *pMe, float turnang, BOOL MovementAllowed);
void		AI_Standard_Follow				(CGameObjectInstance *pMe, CGameObjectInstance *pLeader);
void		AI_Standard_DeadBodyRiddle		(CGameObjectInstance *pMe);
void		AI_Standard_Retreat				(CGameObjectInstance *pMe, CGameObjectInstance *pDest);
void		AI_Standard_Sink					(CGameObjectInstance *pMe);
void		AI_Standard_Float					(CGameObjectInstance *pMe);
BOOL		AI_Standard_Teleport				(CGameObjectInstance *pMe);
void		AI_Standard_TeleportMoveSlow	(CGameObjectInstance *pMe);
void		AI_Standard_Sniper				(CGameObjectInstance *pMe, CGameObjectInstance *pDest);
void		AI_Standard_Flying				(CGameObjectInstance *pMe, CGameObjectInstance *pDest);
void		AI_Standard_Keep_It_Flying		(CGameObjectInstance *pMe, CGameObjectInstance *pDest);
void		AI_Standard_ExplodedLand		(CGameObjectInstance *pMe, int waterFlag);
BOOL		AI_Standard_HeadingForWall		(CGameObjectInstance *pAI, float radiusFactor);

BOOL		IsPlayerAlive();
CVector3	GetPlayerPos();



// EXTRA AI-HELPING CODE
void AI_AssistRun(CGameObjectInstance *pMe) ;
BOOL AI_InTurokWeaponPath(CGameObjectInstance *pMe, FLOAT AngleRange) ;
BOOL AI_IsInMovementAnim(CGameObjectInstance *pMe) ;
void AI_UpdateLeashPos(CGameObjectInstance *pMe) ;
 

/////////////////////////////////////////////////////////////////////////////
#endif	// _INC_AISTAND
