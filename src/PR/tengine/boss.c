// Boss control file by Stephen Broumley

#include "stdafx.h"

#include "romstruc.h"
#include "tengine.h"
#include "ai.h"
#include "boss.h"
#include "bossflgs.h"
#include "mantis.h"
#include "trex.h"
#include "humvee.h"
#include "longhunt.h"
#include "campaign.h"
#include "tcontrol.h"
#include "tmove.h"
#include "cartdir.h"
#include "pickup.h"
#include "sfx.h"


/////////////////////////////////////////////////////////////////////////////
// Debug constants
/////////////////////////////////////////////////////////////////////////////

#define BOSS_SAVE		1




#ifndef MAKE_CART

#define	DEBUG_BOSS					0		// Displays modes if set to TRUE
#define	SHOW_BOSS_HEALTH			1		// Show boss bar or whatever!
#define	GIVE_PLAYER_ALL_WEAPONS	1		// Give player all weapons

#else

#define	DEBUG_BOSS					0		// Displays modes if set to TRUE
#define	SHOW_BOSS_HEALTH			1		// Show boss bar or whatever!
#define	GIVE_PLAYER_ALL_WEAPONS	0	// Give player all weapons

#endif



// Save status of a boss
void CBossData__Save(CBossData *pThis, CGameObjectInstance *pObject, UINT32 Size)
{
#if BOSS_SAVE
	// Must be on a boss level
	if (!GetApp()->m_BossLevel)
		return ;

	pThis->m_pObject = pObject ;
	pThis->m_pBossVars = pObject->m_pBoss ;

	if (CGameObjectInstance__IsGone(pObject))
		pThis->m_IsGone = TRUE ;
	else
		pThis->m_IsGone = FALSE ;

	// Save position
	pThis->m_vPos = pObject->ah.ih.m_vPos ;
	pThis->m_RotY = pObject->m_RotY ;									// Y rotation of boss

	// Save data
	ASSERT(Size <= MAX_BOSS_DATA_SIZE) ;
	pThis->m_BossDataSize = Size ;
	memcpy(pThis->m_BossData, pObject->m_pBoss, Size) ;

	// Save dynamic data
	memcpy(&pThis->m_AIDynamicData, &pObject->m_AI, sizeof(CAIDynamic)) ;

	// Save animation (animation is restored in "scene.cpp"
	pThis->m_Anim = pObject->m_pBoss->m_pModeTable[pObject->m_pBoss->m_Mode].m_Anim ;
	pThis->m_CurrentAnimFrame = pObject->m_asCurrent.m_cFrame ;

	// Save misc
	pThis->m_pCollisionFunction = (void *)pObject->m_pCollisionFunction ;

	// Mode stuff
	pThis->m_Mode = pObject->m_Mode ;
	pThis->m_ModeTime	= pObject->m_ModeTime ;
	pThis->m_ModeMisc1 = pObject->m_ModeMisc1 ;
#endif
}


// Restore status of a boss
void CBossData__Restore(CBossData *pThis)
{
#if BOSS_SAVE

	CGameObjectInstance *pObject = pThis->m_pObject ;

	// Must be on a boss level
	if (!GetApp()->m_BossLevel)
		return ;

	// Clear status
	pThis->m_pObject = NULL ;
	pObject->m_pBoss = pThis->m_pBossVars ;

	if (pThis->m_IsGone)
		CGameObjectInstance__SetGone(pObject) ;
	else
		CGameObjectInstance__SetNotGone(pObject) ;

	// Restore position
	pObject->ah.ih.m_vPos = pThis->m_vPos ;
	pObject->m_RotY = pThis->m_RotY ;
	pObject->ah.ih.m_pCurrentRegion =  CScene__NearestRegion(&GetApp()->m_Scene, &pThis->m_vPos) ;

	// Restore boss data
	memcpy(pObject->m_pBoss, pThis->m_BossData, pThis->m_BossDataSize) ;

	// Restore dynamic data
	memcpy(&pObject->m_AI, &pThis->m_AIDynamicData, sizeof(CAIDynamic)) ;

	// Restore misc
	pObject->m_pCollisionFunction = (void *)pThis->m_pCollisionFunction ;

	// Mode stuff
	pObject->m_Mode = pThis->m_Mode ;
	pObject->m_ModeTime	= pThis->m_ModeTime ;
	pObject->m_ModeMisc1 = pThis->m_ModeMisc1 ;

	// Call restore function
	if (pObject->m_pBoss->m_pRestoreFunction)
		pObject->m_pBoss->m_pRestoreFunction(pObject, pObject->m_pBoss) ;

#endif

}




void CBossesStatus__Construct(CBossesStatus *pThis)
{
	int	i ;

	// Clear boss data structures
	pThis->m_BossData[0].m_pObject = NULL ;
	pThis->m_BossData[1].m_pObject = NULL ;
	pThis->m_BossData[2].m_pObject = NULL ;

	// Clear events
	pThis->m_CurrentEvent = 0 ;
	for (i = 0 ; i < MAX_BOSS_EVENTS ; i++)
		pThis->m_EventData[i].m_pInst = NULL ;

	// Clear pickups
	pThis->m_CurrentPickup = 0 ;
	for (i = 0 ; i < MAX_BOSS_PICKUPS ; i++)
		pThis->m_PickupData[i].m_nType = 0 ;

	// Clear boss active
	pThis->m_pBossActive = NULL ;
	GetApp()->m_pBossActive = NULL ;

	pThis->m_PickupMonitorActive = FALSE ;
	GetApp()->m_Scene.m_PickupMonitor.m_Active = FALSE ;
}

INT32 CGameObjectInstance__GetBossSaveIndex(CGameObjectInstance *pThis)
{
	switch(CGameObjectInstance__TypeFlag(pThis))
	{
		// Make sure longhunter doesn't overlap with humvee's
		case AI_OBJECT_CHARACTER_LONGHUNTER_BOSS:
			return 2 ;

		// Humvee's id is 0 or 1
		case AI_OBJECT_CHARACTER_HUMVEE_BOSS:
			return AI_GetEA(pThis)->m_Id ;

		default:
			return 0 ;
	}
}

INT32 CGameObjectInstance__GetBossDataSize(CGameObjectInstance *pThis)
{
	switch(CGameObjectInstance__TypeFlag(pThis))
	{
		case AI_OBJECT_CHARACTER_LONGHUNTER_BOSS:
			return sizeof(CLonghunt) ;

		case AI_OBJECT_CHARACTER_HUMVEE_BOSS:
			return sizeof(CHumvee) ;

		case AI_OBJECT_CHARACTER_MANTIS_BOSS:
			return sizeof(CMantis) ;

		case AI_OBJECT_CHARACTER_TREX_BOSS:
			return sizeof(CTRex) ;

		case AI_OBJECT_CHARACTER_CAMPAIGNER_BOSS:
			return sizeof(CCampaigner) ;

		default:
			return 0 ;
	}
}

void CBossesStatus__SaveBossStatus(CBossesStatus *pThis, CGameObjectInstance *pObject)
{
#if BOSS_SAVE

	// Must be on a boss level
	if (!GetApp()->m_BossLevel)
		return ;

	CBossData__Save(&pThis->m_BossData[CGameObjectInstance__GetBossSaveIndex(pObject)], pObject,
						 CGameObjectInstance__GetBossDataSize(pObject)) ;
#endif
}

void CBossesStatus__RestoreBossStatus(CBossesStatus *pThis, CGameObjectInstance *pObject)
{
#if BOSS_SAVE

	// Must be on a boss level
	if (!GetApp()->m_BossLevel)
		return ;

	// Restore this boss?
	if (pThis->m_BossData[CGameObjectInstance__GetBossSaveIndex(pObject)].m_pObject == pObject)
		CBossData__Restore(&pThis->m_BossData[CGameObjectInstance__GetBossSaveIndex(pObject)]) ;
#endif
}

CBossData *CBossesStatus__BossWasSaved(CBossesStatus *pThis, CGameObjectInstance *pObject)
{
#if BOSS_SAVE

	// Must be on a boss level
	if (!GetApp()->m_BossLevel)
		return NULL ;

	if (pThis->m_BossData[CGameObjectInstance__GetBossSaveIndex(pObject)].m_pObject == pObject)
		return &pThis->m_BossData[CGameObjectInstance__GetBossSaveIndex(pObject)] ;
	else
		return NULL ;
#else

	return NULL ;

#endif

}


// Keep track of bosses status
void CBossesStatus__SaveAllBossesStatus(CBossesStatus *pThis)
{
#if BOSS_SAVE

	int				i ;
	CDynamicSimple	*pDyn ;
	CBossPickup		*pPickup ;

	CUnindexedSet			usInstances;
	CScene					*pScene;
	int						cInstance, nInstances;
	CGameObjectInstance	*instances, *pInst;


	// Must be on a boss level
	if (!GetApp()->m_BossLevel)
		return ;

	// Save all boss instances
	pScene = &GetApp()->m_Scene;	
	if (pScene->m_pceInstances)
	{
		CUnindexedSet__ConstructFromRawData(&usInstances,
														CCacheEntry__GetData(pScene->m_pceInstances),
														FALSE);

		instances	= (CGameObjectInstance*) CUnindexedSet__GetBasePtr(&usInstances);
		nInstances	= CUnindexedSet__GetBlockCount(&usInstances);

		for (cInstance=0; cInstance<nInstances; cInstance++)
		{
			pInst = &instances[cInstance];

			// A boss?
			if (pInst->m_pBoss)
				CBossesStatus__SaveBossStatus(pThis, pInst) ;

		}
		
		CUnindexedSet__Destruct(&usInstances);
	}


	// Clear pickups
	pThis->m_CurrentPickup = 0 ;
	for (i = 0 ; i < MAX_BOSS_PICKUPS ; i++)
		pThis->m_PickupData[i].m_nType = 0 ;

	// Save pickups
	pPickup = pThis->m_PickupData ;
	pDyn = GetApp()->m_Scene.m_SimplePool.m_pActiveHead ;
	while(pDyn)
	{
		switch(CInstanceHdr__TypeFlag(&pDyn->s.ah.ih))
		{
			case AI_OBJECT_PICKUP_MACHINEGUN:	// The longhunter's gun
			case AI_OBJECT_PICKUP_KEY5:			// longhunter key
			case AI_OBJECT_PICKUP_KEY8MANTIS:	// mantis key
				pPickup->m_nType = CInstanceHdr__TypeFlag(&pDyn->s.ah.ih) ;
				pPickup->m_vPos = pDyn->s.ah.ih.m_vPos ;
				pPickup++ ;
				break ;
		}

		// Do next
		pDyn = pDyn->m_pNext ;

	}

	// Save misc info
	pThis->m_pBossActive = GetApp()->m_pBossActive ;
	pThis->m_PickupMonitorActive = GetApp()->m_Scene.m_PickupMonitor.m_Active ;
#endif
}





void CBossesStatus__RestoreAllBossesStatus(CBossesStatus *pThis)
{
#if BOSS_SAVE
	int			i ;
	CBossEvent	*pEvent ;
	CBossPickup	*pPickup ;
	CVector3			vVel ;

	// Must be on a boss level
	if (!GetApp()->m_BossLevel)
		return ;

	// Dispatch all saved events
	pEvent = pThis->m_EventData ;
	i = MAX_BOSS_EVENTS ;
	while(i--)
	{
		if (pEvent->m_pInst)
		{
			AI_Event_Dispatcher(pEvent->m_pInst, pEvent->m_pInst, pEvent->m_nType,
									  AI_SPECIES_ALL, pEvent->m_vPos, pEvent->m_Value) ;

//			osSyncPrintf("%d DispatchEvent:%d Value:%f\r\n", i, pEvent->m_nType, pEvent->m_Value) ;
		}

		pEvent++ ;
	}

	// Restore pickups
	pPickup = pThis->m_PickupData ;
	i = MAX_BOSS_PICKUPS ;
	CVector3__Set(&vVel,0,0,0) ;
	while(i--)
	{
		if (pPickup->m_nType)
		{
			CSimplePool__CreateSimpleInstance(&GetApp()->m_Scene.m_SimplePool,
														 pPickup->m_nType, vVel,
														 pPickup->m_vPos,
														 CScene__NearestRegion(&GetApp()->m_Scene, &pPickup->m_vPos),0) ;

			pPickup->m_nType = 0 ;
		}

		pPickup++ ;
	}

	// Restore misc info
	GetApp()->m_pBossActive = pThis->m_pBossActive ;
	GetApp()->m_Scene.m_PickupMonitor.m_Active = pThis->m_PickupMonitorActive ;
#endif
}



void CBossesStatus__SaveEvent(CBossesStatus *pThis, CInstanceHdr *pInst, int nType, CVector3 vPos, float Value)
{
#if BOSS_SAVE
	CBossEvent		*pEvent ;
	int				i ;

	// Must be on a boss level
	if (!GetApp()->m_BossLevel)
		return ;

	// Full up?
	if (pThis->m_CurrentEvent >= MAX_BOSS_EVENTS)
		return ;


	// Check for event already being there
	pEvent = pThis->m_EventData ;
	i = MAX_BOSS_EVENTS ;
	while(i--)
	{

		// Event match?
//		if ((pEvent->m_pInst == pInst) &&
//			 (pEvent->m_nType == nType) &&
//			 (pEvent->m_vPos.x == vPos.x) &&
//			 (pEvent->m_vPos.y == vPos.y) &&
//			 (pEvent->m_vPos.z == vPos.z) &&
//			 (pEvent->m_Value == Value))
//			return ;

		// Event match?
		if ( (pEvent->m_nType == nType) &&
			 (pEvent->m_vPos.x == vPos.x) &&
			 (pEvent->m_vPos.y == vPos.y) &&
			 (pEvent->m_vPos.z == vPos.z) &&
			 (pEvent->m_Value == Value))
			return ;

		pEvent++ ;
	}

//	osSyncPrintf("%d SaveEvent:%d Value:%f\r\n", pThis->m_CurrentEvent, nType, Value) ;

	// Store event
	pEvent = &pThis->m_EventData[pThis->m_CurrentEvent++] ;
	pEvent->m_pInst = pInst ;
	pEvent->m_nType = nType ;
	pEvent->m_vPos = vPos ;
	pEvent->m_Value = Value ;
#endif

}



/////////////////////////////////////////////////////////////////////////////
// External controller data
/////////////////////////////////////////////////////////////////////////////
extern	int			Controller ;	// lowest numbered controller connected to system
extern	CTControl	*pCTControl ;	// pointer to turok controller data
extern	CTMove		*pCTMove ;		// pointer to turok movement object


/////////////////////////////////////////////////////////////////////////////
// Boss vars functions
/////////////////////////////////////////////////////////////////////////////

void CBossVars__Construct(CBossVars *pThis)
{
	pThis->m_SwitchesOn = 0 ;
	pThis->m_SwitchFlags = 0 ;
	pThis->m_StartBossTimer = 0 ;

#if 0
	osSyncPrintf("Longhunter Size:%d\r\n", sizeof(CLonghunt)) ;
	osSyncPrintf("Humvee Size:%d\r\n", sizeof(CHumvee)) ;
	osSyncPrintf("Mantis Size:%d\r\n", sizeof(CMantis)) ;
	osSyncPrintf("TRex Size:%d\r\n", sizeof(CTRex)) ;
	osSyncPrintf("Campainger Size:%d\r\n", sizeof(CCampaigner)) ;
#endif

}



/////////////////////////////////////////////////////////////////////////////
// General boss functions
/////////////////////////////////////////////////////////////////////////////


void CBoss__Construct(CBoss *pThis)
{
	// Reset misc
	pThis->m_pShield = NULL ;

	// Reset functions
	pThis->m_pInitializeFunction = NULL ;
	pThis->m_pUpdateFunction = NULL ;
	pThis->m_pHitByParticleFunction = NULL ;
	pThis->m_pPreDrawFunction = NULL ;
	pThis->m_pPostDrawFunction = NULL ;
	pThis->m_pPauseFunction = NULL ;
	pThis->m_pRestoreFunction = NULL ;

#ifndef MAKE_CART
	pThis->m_pDisplayModeFunction = NULL ;
#endif
}




/*****************************************************************************
*
*	Function:		BOSS_Initialise()
*
******************************************************************************
*
*	Description:	Initialises boss instance
*
*	Inputs:			*pThis			-	Ptr to game object instance
*						nObjectType		-	Object type
*
*	Outputs:			None
*
*****************************************************************************/
void BOSS_Initialise(CGameObjectInstance *pThis, INT32 nObjectType)
{
	CBoss *pBoss ;

	// Already initialized?
//	if (CBossesStatus__BossWasSaved(&GetApp()->m_BossesStatus, pThis))
//		pBoss = pThis->m_pBoss ;
//	else
	if (1)
	{
		// Setup active flag
		GetApp()->m_pBossActive = pThis ;
		GetApp()->m_BossesStatus.m_pBossActive = pThis ;

		switch(nObjectType)
		{
			case AI_OBJECT_CHARACTER_MANTIS_BOSS:
				GetApp()->m_BossVars.m_pMantisInstance = pThis ;
				pBoss = Mantis_Initialise(pThis) ;
				break ;

			case AI_OBJECT_CHARACTER_TREX_BOSS:
				pBoss = TRex_Initialise(pThis) ;
				break ;

			case AI_OBJECT_CHARACTER_HUMVEE_BOSS:
				pBoss = Humvee_Initialise(pThis) ;
				break ;

			case AI_OBJECT_CHARACTER_LONGHUNTER_BOSS:
				pBoss = Longhunt_Initialise(pThis) ;
				break ;

			case AI_OBJECT_CHARACTER_CAMPAIGNER_BOSS:
				pBoss = Campaigner_Initialise(pThis) ;
				break ;


			default:
				pBoss = NULL ;
		}
	}

	// Keep pointer to boss vars in instance
	pThis->m_pBoss = pBoss ;
	if (!pBoss)
		return ;

	// Call initialize function
	if (pBoss->m_pInitializeFunction)
		pBoss->m_pInitializeFunction(pThis, pBoss) ;

	// Apply difficulty
	BOSS_Health(pThis) = ((float)BOSS_Health(pThis) * CEngineApp__GetBossHitPointScaler(GetApp())) ;

	// Keep copy of full health!
	pBoss->m_LastHealth = pBoss->m_StartHealth = BOSS_Health(pThis) ;
	pBoss->m_Alive = TRUE ;

	// Make sure boss only attacks Turok
	pThis->m_pAttackerCGameObjectInstance = CEngineApp__GetPlayer(GetApp());



	// Initialise boss vars
	pBoss->m_Initialized = FALSE ;
	pBoss->m_ObjectType = nObjectType ;
	pBoss->m_AnimPlayed = 0 ;
	pBoss->m_AnimPastEnd = FALSE ;
	pBoss->m_LastPercentageHealth = 100 ;
	pBoss->m_PercentageHealth = 100 ;
	pBoss->m_AnimSpeed = 2.0 ;

	pThis->m_pCalculateOrientation = (void *)BOSS_CalculateOrientation ;
	pBoss->m_RotX = 0 ;
	pBoss->m_RotZ = 0 ;

	pBoss->m_TurokAngle =
	pBoss->m_TurokAngleDiff =
	pBoss->m_TurokAngleDist =
	pBoss->m_LastTurokDeltaAngle =
	pBoss->m_TurokDeltaAngle =
	pBoss->m_AbsTurokDeltaAngle = 0 ;

}



/*****************************************************************************
*
*	Function:		BOSS_LevelComplete()
*
******************************************************************************
*
*	Description:	Boss is dead, clear engines BossActive flag so next time level
*						is reset, a new scene will be constructed
*
*	Inputs:			none
*	Outputs:			none
*
*****************************************************************************/
void BOSS_LevelComplete(void)
{
//	GetApp()->m_pBossActive = NULL ;
//	CPickupMonitor__Deactivate(&GetApp()->m_Scene.m_PickupMonitor) ;
}




/*****************************************************************************
*
*	Function:		BOSS_UpdateAnimation()
*
******************************************************************************
*
*	Description:	Updates animation changes and keeps it in sync with boss mode
*
*	Inputs:			*pThis			-	Ptr to game object instance
*
*	Outputs:			None
*
*****************************************************************************/
void BOSS_UpdateAnimation(CGameObjectInstance *pThis)
{
	// Get pointer to boss vars
	CBoss						*pBoss = pThis->m_pBoss ;
//	INT32						EndFrame ;

	// Restart new anim if in different mode
	if (pBoss->m_OldMode != pBoss->m_Mode)
	{
		// Start new anim
		AI_SetDesiredAnim(pThis, pBoss->m_pModeTable[pBoss->m_Mode].m_Anim, TRUE) ;
		pThis->m_asCurrent.m_CycleCompleted = FALSE ;

		// Update mode anim speed?
		if (pBoss->m_pModeTable[pBoss->m_Mode].m_AnimSpeed)
			pBoss->m_ModeAnimSpeed = pBoss->m_pModeTable[pBoss->m_Mode].m_AnimSpeed ;

		// Update mode flags
		pBoss->m_OldModeFlags = pBoss->m_ModeFlags ;
		pBoss->m_ModeFlags = pBoss->m_pModeTable[pBoss->m_Mode].m_Flags ;

		// Reset vars
		pBoss->m_AnimPlayed = 0 ;
		pBoss->m_ModeTime = 0 ;

		// Update old mode
		pBoss->m_OldMode = pBoss->m_Mode ;

#ifndef MAKE_CART
#if DEBUG_BOSS
		// Display mode change
		pBoss->m_pDisplayModeFunction(pBoss) ;
#endif
#endif
	}


#if 1
	// Update anim vars
	if (pThis->m_asCurrent.m_CycleCompleted)
	{
		pThis->m_asCurrent.m_CycleCompleted = FALSE ;
		pBoss->m_AnimPlayed++ ;
	}

#else

	// Special
	if (pThis->m_asCurrent.m_CycleCompleted)
		pBoss->m_AnimPastEnd = FALSE ;


	EndFrame = (pThis->m_asCurrent.m_nFrames * 0.8) ;
	if ((!pBoss->m_AnimPastEnd) && (pThis->m_asCurrent.m_cFrame >= EndFrame))
	{
		pBoss->m_AnimPastEnd = TRUE ;
		pBoss->m_AnimPlayed++ ;
	}
#endif

	// Set anim speed - in one of those Rob strange manners
	if (pBoss->m_ModeFlags & BF_RUN_AT_MODE_ANIM_SPEED)
		pBoss->m_PlaybackAnimSpeed = pBoss->m_ModeAnimSpeed ;
	else
		pBoss->m_PlaybackAnimSpeed = (pBoss->m_AnimSpeed * pBoss->m_ModeAnimSpeed) ;

	frame_increment *= pBoss->m_PlaybackAnimSpeed ;
}






/*****************************************************************************
*
*	Function:		BOSS_SetupMode()
*
******************************************************************************
*
*	Description:	Sets up new boss mode
*
*	Inputs:			*pThis	-	Ptr to game object instance
*						nNewMode	-	New mode
*
*	Outputs:			TRUE if new mode was set up
*
*****************************************************************************/
BOOL BOSS_SetupMode(CGameObjectInstance *pThis, INT32 nNewMode)
{
	// Get pointer to boss vars
	CBoss *pBoss = pThis->m_pBoss ;

	// Record old mode
	pBoss->m_LastMode = pBoss->m_Mode ;

	// Call mode table setup function if one is presenet
	if (pBoss->m_pModeTable[nNewMode].m_pSetupFunction)
		(pBoss->m_pModeTable[nNewMode].m_pSetupFunction)(pThis, pBoss) ;
	else
		pBoss->m_Mode = nNewMode ;

	// Update mode flags
	pBoss->m_OldModeFlags = pBoss->m_ModeFlags ;
	pBoss->m_ModeFlags = pBoss->m_pModeTable[pBoss->m_Mode].m_Flags ;

	// Stop skating
	if (pBoss->m_ModeFlags & BF_CLEAR_VELS)
		CVector3__Set(&pThis->ah.m_vVelocity,0,0,0) ;

	// Return TRUE if new mode was implemented
	return (pBoss->m_Mode == nNewMode) ;
}


void BOSS_UpdateSounds(CGameObjectInstance *pThis)
{
	static			CVector3	vWater ;

	switch(CGameObjectInstance__TypeFlag(pThis))
	{
		case AI_OBJECT_CHARACTER_LONGHUNTER_BOSS:
//			if (GetApp()->m_BossFlags & BOSS_FLAG_LONGHUNTER_SWITCH_PRESSED)
			if (GetApp()->m_BossFlags & BOSS_FLAG_LONGHUNTER_DEAD)
			{
				// Start water sound
				CVector3__Set(&vWater,0,0,0) ;
				CScene__DoSoundEffect(&GetApp()->m_Scene,
											 SOUND_GYSER_SPOUTING, 0.0,
//											 SOUND_WATERFALL_RUSH, 0.0,
											 (void *)pThis, &vWater, 0) ;

			}
			break ;
	}
}
#if 0
	SOUND_RIVER_FLOW_LIGHTLY,
	SOUND_RIVER_FLOW_MEDIUM,
	SOUND_RIVER_FLOW_HEAVY,
	SOUND_WATERFALL_RUSH,
#endif


/*****************************************************************************
*
*	Function:		BOSS_Update()
*
******************************************************************************
*
*	Description:	Calls all boss update/animate functions
*
*	Inputs:			*pThis -	Ptr to game object instance
*
*	Outputs:			None
*
*****************************************************************************/
void BOSS_Update(CGameObjectInstance *pThis)
{
	// Get pointer to boss vars
	CBoss						*pBoss = pThis->m_pBoss ;
	CBossVars				*pBossVars = &GetApp()->m_BossVars ;

	CGameObjectInstance	*pTurok = CEngineApp__GetPlayer(GetApp()) ;
	FLOAT						Mag ;

	// Play sounds
	BOSS_UpdateSounds(pThis) ;

	// Make sure boss only attacks Turok
	pThis->m_pAttackerCGameObjectInstance = CEngineApp__GetPlayer(GetApp()) ;


	Mag = (CTurokMovement.MoveSpeed * CTurokMovement.MoveSpeed) +
			(CTurokMovement.SideStepSpeed * CTurokMovement.SideStepSpeed) ;
	if (Mag)
		Mag = sqrt(Mag) ;
	GetApp()->m_BossVars.m_TurokSpeed = Mag ;


	// First update ever?
	if (!pBoss->m_Initialized)
	{
		pBoss->m_Initialized = TRUE ;
		pBoss->m_OldMode = -1 ;
		pBoss->m_ModeTime = 0 ;
	}

	// Increment mode timer
	pBoss->m_ModeTime += frame_increment ;


	// Setup last vel
	pBoss->m_LastVel.x = pThis->ah.ih.m_vPos.x - pBoss->m_LastPos.x ;
	pBoss->m_LastVel.y = pThis->ah.ih.m_vPos.y - pBoss->m_LastPos.y ;
	pBoss->m_LastVel.z = pThis->ah.ih.m_vPos.z - pBoss->m_LastPos.z ;


	// Setup normalised direction and speed
	Mag = (pBoss->m_LastVel.x * pBoss->m_LastVel.x) +
			(pBoss->m_LastVel.y * pBoss->m_LastVel.y) +
			(pBoss->m_LastVel.z * pBoss->m_LastVel.z) ;

	if (Mag)
	{
		Mag = sqrt(Mag) ;
		pBoss->m_Speed = Mag ;
	}
	else
	{
		Mag = 1 ;
		pBoss->m_Speed = 0.0 ;
	}

	pBoss->m_LastDir.x = pBoss->m_LastVel.x / Mag ;
	pBoss->m_LastDir.y = pBoss->m_LastVel.y / Mag ;
	pBoss->m_LastDir.z = pBoss->m_LastVel.z / Mag ;


	// Calculate distance from Turok
	pBoss->m_DistFromTurok = AI_DistanceSquared(pThis,
																CEngineApp__GetPlayer(GetApp())->ah.ih.m_vPos) ;
	if (pBoss->m_DistFromTurok != 0)
		pBoss->m_DistFromTurok = sqrt(pBoss->m_DistFromTurok) ;

	// Calculate all Turok angles etc
	BOSS_CalculateAngles(pThis, pBoss) ;

	// Update health %
	pBoss->m_LastPercentageHealth = pBoss->m_PercentageHealth ;
	if (BOSS_Health(pThis) > 0)
	{
		pBoss->m_PercentageHealth = (BOSS_Health(pThis) * 100) / pBoss->m_StartHealth ;

		// Put % to 1, if boss has a tiny bit of energy remaining!
		if (pBoss->m_PercentageHealth == 0)
			pBoss->m_PercentageHealth = 1 ;
	}
	else
		pBoss->m_PercentageHealth = 0 ;



	// Call boss character update routine
	if (pBoss->m_pUpdateFunction)
		(pBoss->m_pUpdateFunction)(pThis, pBoss) ;

	// Keep copy of current health
	pBoss->m_LastHealth = BOSS_Health(pThis) ;

	// Boss died?
	if ((pBoss->m_Alive) && (BOSS_Health(pThis) <= 0))
   	pBoss->m_Alive = FALSE ;

	// The end?
	if ((pBoss->m_ModeFlags & BF_DEATH_MODE) && (pBoss->m_AnimPlayed))
	{
		pBoss->m_pUpdateFunction = NULL ;
		pThis->m_AI.m_dwStatusFlags |= AI_ALREADY_DEAD ;
	}

	// Keep track of current position
	pBoss->m_LastPos.x = pThis->ah.ih.m_vPos.x ;
	pBoss->m_LastPos.y = pThis->ah.ih.m_vPos.y ;
	pBoss->m_LastPos.z = pThis->ah.ih.m_vPos.z ;

	BOSS_UpdateAnimation(pThis) ;

	// Clear event flags
	pBoss->m_EventFlags = 0 ;

	// Track Turok's direction
	CVector3__Subtract(&pBossVars->m_vTurokDir, &pTurok->ah.ih.m_vPos, &pBossVars->m_vTurokLastPos) ;
	CVector3__Normalize(&pBossVars->m_vTurokDir) ;

	// Keep track of Turok
	pBossVars->m_vTurokLastPos = pTurok->ah.ih.m_vPos ;
}



void BOSS_DoNodeDamage(CGameObjectInstance *pThis, INT32 Node)
{
	CMtxF			mfMtx ;
	CVector3	vLocal, vWorld ;

	// Must have a node and a matrix
	if ((Node == -1) || (!pThis->pmtDrawMtxs))
		return ;

	// Transform local pos into world pos using node matrix
	CVector3__Set(&vLocal, 0,0,0) ;
	CMtxF__FromMtx(mfMtx, pThis->pmtDrawMtxs[Node]) ;
	CMtxF__VectorMult(mfMtx, &vLocal, &vWorld) ;

	// Set off explosion
	AI_Event_Dispatcher(&pThis->ah.ih, &pThis->ah.ih,
							  AI_SEVENT_GENERIC224, AI_SPECIES_ALL,
							  vWorld, 1.0) ;
}






/*****************************************************************************
*
*	Function:		BOSS_CalculateAngles()
*
******************************************************************************
*
*	Description:	Calculates direction angles between boss and Turok
*
*	Inputs:			*pThis	-	Ptr to game object instance
*						*pBoss	-	Ptr to boss vars
*
*	Outputs:			TRUE if new mode was set up
*
*****************************************************************************/
void BOSS_CalculateAngles(CGameObjectInstance *pThis, CBoss *pBoss)
{
	FLOAT						TargetAngle ;
	CVector3					vBoss, vTurok ;
	CGameObjectInstance *pTurok ;

	// Get angle between Turok amd Longhunter
	pTurok = CEngineApp__GetPlayer(GetApp());
	vBoss = BOSS_GetPos(pThis) ;
	vTurok = TUROK_GetPos(pTurok) ;
	vBoss.x -= vTurok.x ;
	vBoss.y -= vTurok.y ;
	vBoss.z -= vTurok.z ;
	TargetAngle = CVector3__XZAngle(&vBoss) ;

	pBoss->m_TurokAngle = TargetAngle ;
	pBoss->m_TurokAngleDiff = AngleDiff(TargetAngle, pTurok->m_RotY) ;
	pBoss->m_TurokAngleDist = abs(pBoss->m_TurokAngleDiff) ;

	pBoss->m_LastTurokDeltaAngle = pBoss->m_TurokDeltaAngle ;
	pBoss->m_TurokDeltaAngle = AI_GetAngle(pThis, pTurok->ah.ih.m_vPos) ;
	pBoss->m_AbsTurokDeltaAngle = abs(pBoss->m_TurokDeltaAngle) ;
}



/*****************************************************************************
*
*	Function:		BOSS_InProjectilePath()
*
******************************************************************************
*
*	Description:	Checks to see if boss in line of fire from Turok
*
*	Inputs:			*pThis		-	Ptr to game object instance
*						*pBoss		-	Ptr to boss vars
*						AngleRange	-	Max angle range from projectile path
*
*	Outputs:			TRUE if boss is being fired at and in projectile angle range
*						else FALSE
*
*****************************************************************************/
INT32 BOSS_InProjectilePath(CGameObjectInstance *pThis, CBoss *pBoss, FLOAT AngleRange)
{
	// Is Turok firing at boss?
	if ((pCTMove->FiredProjectileParticle) &&
		 (abs(GetApp()->m_RotXOffset) < ANGLE_DTOR(8)) && (pBoss->m_TurokAngleDist < AngleRange))
	{
		pCTMove->FiredProjectileParticle = FALSE ;
		return pCTMove->FiredProjectileParticleType ;
	}

	return FALSE ;
}





/*****************************************************************************
*
*	Function:		BOSS_PerformTurn()
*
******************************************************************************
*
*	Description:	Turns bos to face target diection
*
*	Inputs:			*pThis		-	Ptr to game object instance
*						*pBoss		-	Ptr to boss vars
*						TurnSpeed	-	Max turn speed
*						DeltaAngle	-	Absoulte delta angle to point in correct direction
*
*	Outputs:			None
*
*****************************************************************************/
void BOSS_PerformTurn(CGameObjectInstance *pThis, CBoss *pBoss, FLOAT TurnSpeed, FLOAT DeltaAngle)
{
	FLOAT	Temp ;

//, DeltaAngle = pBoss->m_DeltaAngle ;

	// Limit turn speed
	Temp = DeltaAngle ;
	if (DeltaAngle > TurnSpeed)
		DeltaAngle = TurnSpeed ;
	else
	if (DeltaAngle < -TurnSpeed)
		DeltaAngle = -TurnSpeed ;

	// Turn boss
	DeltaAngle *= frame_increment ;
	if (abs(DeltaAngle) > abs(Temp))
		DeltaAngle = Temp ;

	// Finally do the turn!
	AI_NudgeRotY(pThis, DeltaAngle) ;
}

void BOSS_RotateToFaceTurok(CGameObjectInstance *pThis, CBoss *pBoss, FLOAT Speed)
{
	CGameObjectInstance *pTurok = CEngineApp__GetPlayer(GetApp()) ;
	FLOAT		DeltaAngle ;

	if (pTurok)
	{
		DeltaAngle = AI_GetAvoidanceAngle(pThis, TUROK_GetPos(pTurok), pTurok, AVOIDANCE_RADIUS_DISTANCE_FACTOR) ;
		BOSS_PerformTurn(pThis, pBoss, Speed, DeltaAngle) ;
	}
}




/*****************************************************************************
*
*	Function:		BOSS_HitByParticleFlinchMode()
*
******************************************************************************
*
*	Description:	Calculates correct finch to use when hit by a particle
*
*	Inputs:			*pThis -	Ptr to game object instance
*						*pBoss -	Ptr to boss vars
*						*pInfo - Ptr to flinch info
*						*pInfo - Ptr to colliding particle
*
*	Outputs:			None
*
*****************************************************************************/
INT32	BOSS_HitByParticleFlinchMode(CGameObjectInstance *pThis, CBoss *pBoss, CFlinchInfo *pInfo, CParticle *pParticle)
{
	FLOAT			VelAngle, Dist, PosAngle, Height ;
	CVector3		vPos ;

	// Get velcity angle
	VelAngle = CVector3__XZAngle(&pParticle->m_CI.vCollisionVelocity) + pInfo->RotYOffset;
	Dist = AngleDist(VelAngle, pThis->m_RotY) ;

	// Get position angle
	vPos = BOSS_GetPos(pThis) ;

	vPos.x -= pParticle->m_CI.vInstanceCollisionPos.x ;
	vPos.y -= pParticle->m_CI.vInstanceCollisionPos.y ;
	vPos.z -= pParticle->m_CI.vInstanceCollisionPos.z ;
	PosAngle = CVector3__XZAngle(&vPos) + pThis->m_RotY + pInfo->RotYOffset ;
	NormalizeRotation(&PosAngle) ;

	// Get height from base of longhunter
	Height = pParticle->m_CI.vInstanceCollisionPos.y - BOSS_GetPos(pThis).y ;

	// Restart anim
	pBoss->m_OldMode = -1 ;

	// Been hit in the front?
	if (Dist < ANGLE_DTOR(90))
	{
		if (Height > pInfo->HeadHeight)
			return pInfo->HeadMode ;
		else
		if (Height > pInfo->StomachHeight)
			return pInfo->BackMode ;
		else
			return pInfo->KneeMode ;
	}
	else
	{
		if (Height > pInfo->HeadHeight)
			return pInfo->HeadMode ;
		else
		if (Height > pInfo->ShoulderHeight)
		{
			if (PosAngle < 0)
				return pInfo->LeftShoulderMode ;
			else
				return pInfo->RightShoulderMode ;
		}
		else
		if (Height > pInfo->StomachHeight)
			return pInfo->StomachMode ;
		else
			return pInfo->KneeMode ;
	}
}





/*****************************************************************************
*
*	Function:		BOSS_GetClosestPositionIndex()
*
******************************************************************************
*
*	Description:	Returns index of closest position to boss, from vector list
*
*	Inputs:			*pThis	-	Ptr to boss
*						*pvPos	-	Ptr to vector position list
*						Length	-	Length of list
*
*	Outputs:			Returns index of closest vector in list
*
*****************************************************************************/
INT32 BOSS_GetClosestPositionIndex(CGameObjectInstance *pThis, CVector3 *pvPos, INT32 Length)
{
	INT32		i ;
	BOOL		Chosen ;
	FLOAT		ClosestDistSqr, XDist, ZDist, DistSqr ;

	// Set dist to max
	Chosen = -1 ;
	ClosestDistSqr = 0 ;	// Fix warning!

	// Check all positions
	for (i = 0 ; i < Length ; i++)
	{
		// Get dist squared from mantis
		XDist = pvPos->x - BOSS_GetPos(pThis).x ;
		ZDist = pvPos->z - BOSS_GetPos(pThis).z ;
		DistSqr = (XDist * XDist) + (ZDist * ZDist) ;

		// Is this the closest wall so far?
		if ((Chosen == -1) || (DistSqr < ClosestDistSqr))
		{
			Chosen = i ;
			ClosestDistSqr = DistSqr ;
		}

		// Next position
		pvPos++ ;
	}

	return Chosen ;
}





BOOL BOSS_OffsetFromPlayerObstructed(CGameObjectInstance *pThis, FLOAT Radius, FLOAT AngleOffset, CGameRegion **ppRegion, CVector3 *pvOutPos)
{
	CGameObjectInstance	*pTurok = CEngineApp__GetPlayer(GetApp());
	CVector3					vDesiredPos ;
	FLOAT						Angle ;
	BOOL						Result ;

	// Setup desired pos
	Angle = pTurok->m_RotY + AngleOffset ;
	vDesiredPos.x = pTurok->ah.ih.m_vPos.x - (Radius * sin(Angle)) ;
	vDesiredPos.y = pTurok->ah.ih.m_vPos.y ;
	vDesiredPos.z = pTurok->ah.ih.m_vPos.z - (Radius * cos(Angle)) ;

	// Check collision
	Result = CAnimInstanceHdr__IsObstructed(&pTurok->ah, vDesiredPos, NULL) ;

	// Get closest region!
	CInstanceHdr__GetNearPositionAndRegion(&pTurok->ah.ih,
														vDesiredPos,
														pvOutPos,
														ppRegion,
														INTERSECT_BEHAVIOR_IGNORE, INTERSECT_BEHAVIOR_IGNORE);
	return Result ;
}




/////////////////////////////////////////////////////////////////////////////
// Generic mode code
/////////////////////////////////////////////////////////////////////////////

void BOSS_Code_NextModeAfterAnim(CGameObjectInstance *pThis, CBoss *pBoss)
{
	// Anim finished?
	if (pThis->m_asCurrent.m_cFrame >= (pThis->m_asCurrent.m_nFrames - 5))
		BOSS_SetupMode(pThis, pBoss->m_Mode + 1) ;
}


/////////////////////////////////////////////////////////////////////////////
// Orientation functions
/////////////////////////////////////////////////////////////////////////////



/*****************************************************************************
*
*	Function:		BOSS_CalculateOrientation()
*
******************************************************************************
*
*	Description:	Calculate orientation quarternion - used for user orientation
*
*	Inputs:			*pThis -	Ptr to game object instance
*
*	Outputs:			Orientation quarternion
*
*****************************************************************************/
CQuatern BOSS_CalculateOrientation(CGameObjectInstance *pThis)
{
	CBoss *pBoss = pThis->m_pBoss ;

	CQuatern    qRotX, qRotY, qRotZ,
					qTemp1, qTemp2;


	CQuatern__BuildFromAxisAngle(&qRotX, 1,0,0, pBoss->m_RotX);
	CQuatern__BuildFromAxisAngle(&qRotY, 0,1,0, pThis->m_RotY);
	CQuatern__BuildFromAxisAngle(&qRotZ, 0,0,1, pBoss->m_RotZ);

	CQuatern__Mult(&qTemp1, &pThis->m_qRot, &qRotX);
	CQuatern__Mult(&qTemp2, &qTemp1, &qRotZ);
	CQuatern__Mult(&qTemp1, &qTemp2, &qRotY);

	CQuatern__Mult(&qTemp2, &qTemp1, &pThis->m_qGround);

	return qTemp2 ;
}



/*****************************************************************************
*
*	Function:	   BOOS_SetPosition()
*
******************************************************************************
*
*	Description:	Sets instance as close as possible to requested position
*						- on the ground.
*
*	Inputs:			*pThis		-	Pointer to game object instance
*						x				-	Desired x pos
*						z				-	Desired y pos
*
*	Outputs:			None.
*
*****************************************************************************/
void BOSS_SetPosition(CGameObjectInstance *pThis, FLOAT x, FLOAT z)
{
	CGameRegion	 		*pRegion ;
	CVector3				vDesiredPos, vNearestPos ;

	// Stop stray movement
	pThis->ah.m_vVelocity.x = 0 ;
	pThis->ah.m_vVelocity.y = 0 ;
	pThis->ah.m_vVelocity.z = 0 ;

	// Setup desired pos
	vDesiredPos.x = x ;
	vDesiredPos.y = pThis->ah.ih.m_vPos.y ;
	vDesiredPos.z = z ;

#if 0
	// Get closest region!
	CInstanceHdr__GetNearPositionAndRegion(&pThis->ah.ih,
														vDesiredPos,
														&vNearestPos,
														&pRegion,
														INTERSECT_BEHAVIOR_IGNORE, INTERSECT_BEHAVIOR_IGNORE) ;
#else

	// Get absolute region
	vNearestPos = vDesiredPos ;
	pRegion = CScene__NearestRegion(&GetApp()->m_Scene, &vDesiredPos) ;

#endif

	// Get y position
	vNearestPos.y = CGameRegion__GetGroundHeight(pRegion, vNearestPos.x, vNearestPos.z) ;

	// Alrighty, setup instance
	pThis->ah.ih.m_vPos = vNearestPos ;
	pThis->ah.ih.m_pCurrentRegion = pRegion ;
}




/////////////////////////////////////////////////////////////////////////////
// Misc functions
/////////////////////////////////////////////////////////////////////////////



/*****************************************************************************
*
*	Function:		AccelerateFLOAT()
*
******************************************************************************
*
*	Description:	Accelerates float variable
*
*	Inputs:			Var		-	Variable to accelerate
*						Accel		-	Acceleration (has magnitude & direction)
*						MaxSpeed	-	Max speed of variable
*
*	Outputs:			Accelerated variable
*
*****************************************************************************/
FLOAT AccelerateFLOAT(FLOAT Var, FLOAT Accel, FLOAT MaxSpeed)
{

	Var += Accel * frame_increment ;

	if (Accel < 0)
	{
		if (Var < -MaxSpeed)
			Var = -MaxSpeed ;
	}
	else
	{
		if (Var > MaxSpeed)
			Var = MaxSpeed ;
	}

	return Var ;
}


/*****************************************************************************
*
*	Function:		DecelerateFLOAT()
*
******************************************************************************
*
*	Description:	Decelerates float variable to zero
*
*	Inputs:			Var		-	Variable to accelerate
*						Decel		-	Deceleration (positive)
*
*	Outputs:			Decelerated variable
*
*****************************************************************************/
FLOAT DecelerateFLOAT(FLOAT Var, FLOAT Decel)
{
	if (Var > 0)
	{
		Var -= Decel * frame_increment ;
		if (Var < 0)
			Var = 0 ;
	}
	else
	if (Var < 0)
	{
		Var += Decel * frame_increment ;
		if (Var > 0)
			Var = 0 ;
	}

	return Var ;
}



/*****************************************************************************
*
*	Function:		AccelerateINT32()
*
******************************************************************************
*
*	Description:	Accelerates 32-bit
*
*	Inputs:			Var		-	Variable to accelerate
*						Accel		-	Acceleration (has magnitude & direction)
*						MaxSpeed	-	Max speed of variable
*
*	Outputs:			Accelerated variable
*
*****************************************************************************/
INT32 AccelerateINT32(INT32 Var, INT32 Accel, INT32 MaxSpeed)
{

	Var += Accel * frame_increment ;

	if (Accel < 0)
	{
		if (Var < -MaxSpeed)
			Var = -MaxSpeed ;
	}
	else
	{
		if (Var > MaxSpeed)
			Var = MaxSpeed ;
	}

	return Var ;
}




/*****************************************************************************
*
*	Function:		ZoomFLOAT()
*
******************************************************************************
*
*	Description:	Zooms float variable towards target variable
*
*	Inputs:			Var		-	Variable to zoom
*						Target	-	Target value
*						Speed		-	Speed to zoom at
*
*	Outputs:			Zoomed variable!
*
*****************************************************************************/
FLOAT ZoomFLOAT(FLOAT Var, FLOAT Target, FLOAT Speed)
{
	if (Var < Target)
	{
		Var += Speed * frame_increment ;
		if (Var > Target)
			Var = Target ;
	}
	else
	if (Var > Target)
	{
		Var -= Speed * frame_increment ;
		if (Var < Target)
			Var = Target ;
	}

	return Var ;
}




/*****************************************************************************
*
*	Function:		ProportionalZoomVarFLOAT()
*
******************************************************************************
*
*	Description:	Zooms float variable towards target variable. The bigger the
*						difference, the bigger the zoom speed
*
*	Inputs:			*pVar		-	Ptr to variable
*						Target	-	Target value to zoom to
*						Speed		-	Speed to zoom at
*
*	Outputs:			None
*
*****************************************************************************/
void ProportionalZoomVarFLOAT(FLOAT *pVar, FLOAT Target, FLOAT Speed)
{
	FLOAT	Delta = (Target - *pVar) * Speed * frame_increment ;


	if (Speed == 1)
		*pVar = Target ;
	else
	if (*pVar < Target)
	{
		*pVar += Delta ;
		if (*pVar > Target)
			*pVar = Target ;
	}
	else
	if (*pVar > Target)
	{
		*pVar += Delta ;
		if (*pVar < Target)
			*pVar = Target ;
	}
}





/*****************************************************************************
*
*	Function:		AngleDiff()
*
******************************************************************************
*
*	Description:	Calculate the smallest angle difference between two angles
*
*	Inputs:			Phi		-	Angle1 (Radians)
*						Theta		-	Angle2 (Radians)
*
*	Outputs:			Angle difference (Radians and signed)
*
*****************************************************************************/

FLOAT AngleDiffFromZero(FLOAT Theta)
{
	while(Theta < -ANGLE_PI)
		Theta += 2*ANGLE_PI ;
	while(Theta > ANGLE_PI)
		Theta -= 2*ANGLE_PI ;

	return -Theta ;
}


FLOAT AngleDiff(FLOAT Phi, FLOAT Theta)
{
	return AngleDiffFromZero(Phi + AngleDiffFromZero(Theta)) ;
}




/*****************************************************************************
*
*	Function:		AngleDist()
*
******************************************************************************
*
*	Description:	Calculate the abosulte angle difference between two angles
*
*	Inputs:			Phi		-	Angle1 (Radians)
*						Theta		-	Angle2 (Radians)
*
*	Outputs:			Absolute angle distance (Radians and positive)
*
*****************************************************************************/
FLOAT AngleDist(FLOAT Phi, FLOAT Theta)
{
	return abs(AngleDiff(Phi, Theta)) ;
}


/*****************************************************************************
*
*	Function:		RandomRangeFLOAT()
*
******************************************************************************
*
*	Description:	Returns random number between given range
*
*	Inputs:			a			-	Limit1
*						b			-	Limit2
*
*	Outputs:			Random number between limits
*
*****************************************************************************/
FLOAT RandomRangeFLOAT(FLOAT a, FLOAT b)
{
	FLOAT	t ;

	// Get random number between 0 and 1
	t = ((FLOAT)RANDOM(10000)) / (10000-1) ;

	return (t * a) + ((1-t) * b) ;
}




/*****************************************************************************
*
*	Function:		DecreasesTimer()
*
******************************************************************************
*
*	Description:	Decrements timer
*
*	Inputs:			*pTimer	-	Ptr to timer
*
*	Outputs:			TRUE if timer is zero
*
*****************************************************************************/
BOOL DecrementTimer(FLOAT *pTimer)
{
	*pTimer -= frame_increment ;
	if (*pTimer <= 0)
	{
		*pTimer = 0 ;
		return TRUE ;
	}
	else
		return FALSE ;
}



/*****************************************************************************
*
*	Function:		CInstanceHdr__NearestPosition()
*
******************************************************************************
*
*	Description:	Returns nearest position (with correct ground height) at
*						specified position
*
*	Inputs:			*pThis		-	Pointer to instance header
*						*pvDestPos	-	Pointer to destination
*						*pvOutPos 	-	Pointer to result position
*
*	Outputs:			None.
*
*****************************************************************************/
void CInstanceHdr__NearestPosition(CInstanceHdr *pThis, CVector3 *pvDestPos, CVector3 *pvOutPos)
{
	CGameRegion	 		*pRegion ;
	CVector3				vDesiredPos ;

	// Setup desired pos
	vDesiredPos.x = pvDestPos->x ;
	vDesiredPos.y = 0 ;
	vDesiredPos.z = pvDestPos->z ;

	// Get closest region!
	CInstanceHdr__GetNearPositionAndRegion(pThis,
														vDesiredPos,
														pvOutPos,
														&pRegion,
														INTERSECT_BEHAVIOR_IGNORE, INTERSECT_BEHAVIOR_IGNORE) ;

	// Return position
	pvOutPos->y = CGameRegion__GetGroundHeight(pRegion, pvOutPos->x, pvOutPos->z) ;
}




enum ShieldModes
{
	SHIELD_OFF_MODE,
	SHIELD_FADE_IN_MODE,
	SHIELD_ON_MODE,
	SHIELD_FADE_OUT_MODE,
	SHIELD_END_MODE
} ;

void CShield__Construct(CShield *pThis,
								CGameObjectInstance *pOwner,
								INT32 Health, CPsuedoColor *pColors, INT32 TotalColors)
{
	pThis->m_StartHealth = Health ;
	pThis->m_Health = Health ;
	pThis->m_pOwner = pOwner ;
	pThis->m_Mode = SHIELD_OFF_MODE ;
	pThis->m_Alpha = MIN_OPAQ ;
	pThis->m_pParticle = NULL ;
	pThis->m_Time = 0 ;
	pThis->m_pColors = pColors ;
	pThis->m_TotalColors = TotalColors ;
	pThis->m_SoundHandle = -1 ;
}


BOOL CShield__IsOn(CShield *pThis)
{
	return (pThis->m_Mode == SHIELD_FADE_IN_MODE) ||
			 (pThis->m_Mode == SHIELD_ON_MODE) ;
}


void CShield__Update(CShield *pThis)
{
	CVector3			vPos ;
	CParticle		*pParticle ;
	CPsuedoColor	*pColor ;
	INT32				Index ;

	// Turn off shield due to no health remaining
	if (pThis->m_Health <= 0)
		CShield__Deactivate(pThis) ;

	// Update time
	pThis->m_Time -= frame_increment ;
	if (pThis->m_Time < 0)
	{
		pThis->m_Time = 0 ;

		// Turn shield off?
		if ((pThis->m_Mode != SHIELD_OFF_MODE) &&
			 (pThis->m_Mode != SHIELD_FADE_OUT_MODE))
			pThis->m_Mode = SHIELD_FADE_OUT_MODE ;
	}

	pParticle = pThis->m_pParticle ;
	switch(pThis->m_Mode)
	{
		case SHIELD_OFF_MODE:
			break ;

		case SHIELD_FADE_IN_MODE:
			pThis->m_Alpha += (1.0 / SECONDS_TO_FRAMES(0.5)) * frame_increment ;
			if (pThis->m_Alpha >= 1)
			{
				pThis->m_Alpha = 1 ;
				pThis->m_Mode = SHIELD_ON_MODE ;
			}

			break ;

		case SHIELD_ON_MODE:
			break ;

		case SHIELD_FADE_OUT_MODE:

			// Turn off sound
			if (pThis->m_SoundHandle != -1)
			{
				killCFX(pThis->m_SoundHandle) ;
				pThis->m_SoundHandle = -1 ;

				// Play go out sound
				AI_DoSound(&pThis->m_pOwner->ah.ih, SOUND_GENERIC_189, 1, 0) ;
			}

			pThis->m_Alpha -= (1.0 / SECONDS_TO_FRAMES(0.5)) * frame_increment ;
			if (pThis->m_Alpha <= 0)
			{
				pThis->m_Mode = SHIELD_OFF_MODE ;
				pThis->m_pParticle = NULL ;

				// Delete particle
				pParticle->m_dwFlags &= ~PARTICLE_PERMANENT ;
				pParticle->m_dwFlags |= PARTICLE_DELETE ;
				pParticle = NULL ;
			}

			break ;
	}

	// Display shield
	if (pThis->m_Mode != SHIELD_OFF_MODE)
	{
		// Start shield?
		if (!pThis->m_pParticle)
		{
			AI_DoParticle(&pThis->m_pOwner->ah.ih, PARTICLE_TYPE_GENERIC205, pThis->m_pOwner->ah.ih.m_vPos) ;
			pThis->m_pParticle = GetApp()->m_Scene.m_ParticleSystem.m_pActiveHead ;
			pParticle = pThis->m_pParticle ;
		}

		// Update shield particle
		if (pParticle)
		{
			pParticle->m_dwFlags |= PARTICLE_PERMANENT ;

			vPos = pThis->m_pOwner->ah.ih.m_vPos ;
			vPos.y += (2.5*SCALING_FACTOR) + pThis->m_pOwner->ah.ih.m_CollisionHeightOffset ;
			pParticle->ah.ih.m_vPos = vPos ;

			pParticle->m_cFrame = 0 ;
			pParticle->m_nFrames = 999 ;

			pParticle->m_RotationInc = 0.3 + (0.2 * sin(ANGLE_DTOR(game_frame_number*2))) ;

			pParticle->m_Size = (15*SCALING_FACTOR) +
									  (2*SCALING_FACTOR * sin(ANGLE_DTOR(game_frame_number*16))) ;

			// Setup color
			Index = ((pThis->m_StartHealth - pThis->m_Health) * pThis->m_TotalColors) /
					   (pThis->m_StartHealth+1) ;

			pColor = &pThis->m_pColors[Index] ;

			pParticle->m_WhiteColor[0] = pColor->m_WhiteColor.Red ;
			pParticle->m_WhiteColor[1] = pColor->m_WhiteColor.Green ;
			pParticle->m_WhiteColor[2] = pColor->m_WhiteColor.Blue ;
			//pParticle->m_WhiteColor[3] = pColor->m_WhiteColor.Alpha ;

			pParticle->m_BlackColor[0] = pColor->m_BlackColor.Red ;
			pParticle->m_BlackColor[1] = pColor->m_BlackColor.Green ;
			pParticle->m_BlackColor[2] = pColor->m_BlackColor.Blue ;
			//pParticle->m_BlackColor[3] = pColor->m_BlackColor.Alpha ;

			// Setup alpha
			pParticle->m_AlphaScaler = ((128 + (100 * sin(ANGLE_DTOR(game_frame_number*16)))) *
												pThis->m_Alpha) * pColor->m_WhiteColor.Alpha ;
		}
	}

	// Update sound position
	if (pThis->m_SoundHandle != -1)
		UpdateSoundVector(pThis->m_SoundHandle, &pThis->m_pOwner->ah.ih.m_vPos) ;
}



BOOL CShield__Activate(CShield *pThis, FLOAT Time)
{
	// Set time
	pThis->m_Time = Time ;
	if ((pThis->m_Mode != SHIELD_FADE_IN_MODE) &&
		 (pThis->m_Mode != SHIELD_ON_MODE))
	{
		pThis->m_Mode = SHIELD_FADE_IN_MODE ;
		pThis->m_Alpha = 0 ;

		// Start sound
		if (pThis->m_SoundHandle == -1)
			pThis->m_SoundHandle = AI_DoSound(&pThis->m_pOwner->ah.ih, SOUND_ENERGY_SHIELD_HUM, 1, 0) ;

		return TRUE ;
	}
	else
		return FALSE ;
}


void CShield__Deactivate(CShield *pThis)
{
	if (pThis->m_Mode != SHIELD_OFF_MODE)
		pThis->m_Mode = SHIELD_FADE_OUT_MODE ;
}








