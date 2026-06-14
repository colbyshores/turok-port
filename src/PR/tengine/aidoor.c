// aidoor.cpp
//

#include "stdafx.h"
#include "debug.h"

#include "ai.h"
#include "aidoor.h"
#include "tengine.h"
#include "scaling.h"
#include "mantis.h"
#include "trex.h"
#include "particle.h"
#include "regtype.h"
#include "tmove.h"
#include "pickup.h"
#include "sfx.h"
#include "bossflgs.h"

/////////////////////////////////////////////////////////////////////////////
void Elevator_CreateVars(CGameObjectInstance *pThis, float *increment, float *startpos, float *maxpos) ;
void Elevator_Update(CGameObjectInstance *pThis, float startpos, float maxpos) ;
void Elevator_ChangeRegion(CGameObjectInstance *pThis) ;
void Elevator_MoveTo(CGameObjectInstance *pThis) ;
void Elevator_Wait(CGameObjectInstance *pThis) ;
void Elevator_MoveFrom(CGameObjectInstance *pThis) ;



void CInstanceHdr__DeviceParticleCollision(CInstanceHdr *pThis, struct CParticle_t *pParticle)
{
	int objectType;

	ASSERT((pThis->m_Type == I_STATIC) || (pThis->m_Type == I_ANIMATED));
	ASSERT(pThis);
	ASSERT(pParticle);

	objectType = CInstanceHdr__TypeFlag(pThis);
	switch (objectType)
	{
		case AI_OBJECT_DEVICE_ARROWTARGET:
			ArrowTarget_ParticleCollision(pThis, pParticle);
			break;
//		case AI_OBJECT_DEVICE_EXPTARGET:
//			ExplosiveTarget_ParticleCollision(pThis);
//			break;
	}
}

void ArrowTarget_ParticleCollision(CInstanceHdr *pThis, struct CParticle_t *pParticle)
{
	switch (pParticle->ah.ih.m_nObjType)
	{
		case PARTICLE_TYPE_ARROW:
			// send an event to trigger action, uses TIME1 as ID
			AI_Event_Dispatcher(pThis, pThis, AI_MEVENT_PRESSUREPLATE, pThis->m_pEA->m_dwSpecies, pThis->m_vPos, ((CGameObjectInstance *)pThis)->ah.ih.m_pEA->m_AttackStyle);

			CScene__SetInstanceFlag(&GetApp()->m_Scene, (CGameObjectInstance *)pThis, TRUE);
			break;
	}
}

void ExplosiveTarget_ParticleCollision(CGameObjectInstance *pThis)
{
//	switch (pParticle->ah.ih.m_nObjType)
//	{
//		case PARTICLE_TYPE_GRENADE:
//		case PARTICLE_TYPE_ROCKET:
			ExplosiveTargetAI_MEvent_Go(pThis) ;

			// set flags to dead so player can walk through this instance
			// NOTE if health is set to 0, instance will melt away
			AI_GetDyn(pThis)->m_dwStatusFlags |= AI_ALREADY_DEAD ;
			AI_GetDyn(pThis)->m_Health = 0;

			CScene__SetInstanceFlag(&GetApp()->m_Scene, pThis, TRUE);
//			break;
//	}
}


//---------------------------------------------------------------
// Advance routine for DOOR
//---------------------------------------------------------------
void DoorAI_Advance(CGameObjectInstance *pThis)
{
	if (pThis->m_AI.m_dwStatusFlags & AI_STARTED)
	{
		if (pThis->m_AI.m_dwStatusFlags & AI_DOOROPEN)
		{
			// if retreat timer is 0, stay open
			if (pThis->m_AI.m_RetreatTimer != 0)
			{
				// decrease the timer
				pThis->m_AI.m_RetreatTimer -= frame_increment;

				if (pThis->m_AI.m_RetreatTimer <= 0)
				{
					AI_SetDesiredAnim(pThis, AI_ANIM_DOOR_CLOSE, TRUE);
					pThis->m_AI.m_dwStatusFlags &= ~AI_DOOROPEN;
					pThis->m_AI.m_dwStatusFlags &= ~AI_DOORPREOPEN;
				}
			}
		}
		// check timer before door opens
		else if (pThis->m_AI.m_dwStatusFlags & AI_DOORPREOPEN)
		{
			// decrease the timer
			pThis->m_AI.m_RetreatTimer -= frame_increment;

			if (pThis->m_AI.m_RetreatTimer <= 0)
			{
				// setup timer to keep door open
				pThis->m_AI.m_RetreatTimer = pThis->ah.ih.m_pEA->m_Aggression *FRAME_FPS ;
				AI_SetDesiredAnim(pThis, AI_ANIM_DOOR_OPEN, TRUE);
				pThis->m_AI.m_dwStatusFlags |= AI_DOOROPEN;

				// if going to stay open forever, keep in persistant data
				if (pThis->m_AI.m_RetreatTimer == 0)
					CScene__SetInstanceFlag(&GetApp()->m_Scene, pThis, TRUE);
			}
		}
	}
	else
	{
		AI_SetDesiredAnim(pThis, AI_ANIM_DOOR_START, TRUE);
		pThis->m_AI.m_dwStatusFlags |= AI_STARTED;
	}
}


//---------------------------------------------------------------
// Advance routine for ACTIONS
//---------------------------------------------------------------
void ActionAI_Advance(CGameObjectInstance *pThis)
{
	if (!(pThis->m_AI.m_dwStatusFlags & AI_STARTED))
	{
		AI_SetDesiredAnim(pThis, AI_ANIM_ACTION_START, TRUE);
		pThis->m_AI.m_dwStatusFlags |= AI_STARTED;
	}
}

void TimerActionAI_Advance(CGameObjectInstance *pThis)
{
	if (!(pThis->m_AI.m_dwStatusFlags & AI_STARTED))
	{
		AI_SetDesiredAnim(pThis, AI_ANIM_TIMERACTION_START, TRUE);
		pThis->m_AI.m_dwStatusFlags |= AI_STARTED;
		pThis->m_AI.m_RetreatTimer = 0 ;

		// keep copy of loop counter
		pThis->m_AI.m_Time1 = pThis->ah.ih.m_pEA->m_StartHealth ;
	}

	// already started, check ON timer
	else if (pThis->m_AI.m_dwStatusFlags & AI_DOORPREOPEN)
	{
		// if its waiting for player to step off...
		if (pThis->m_AI.m_RetreatTimer == -1)
		{
			if (pThis->m_AI.m_InteractiveTimer)
				pThis->m_AI.m_InteractiveTimer = 0 ;
			else
				pThis->m_AI.m_RetreatTimer = 0 ;
		}
		else
		{
			pThis->m_AI.m_RetreatTimer -= frame_increment ;
			if (pThis->m_AI.m_RetreatTimer <=0)
			{
				// finished ON, goto OFF
				pThis->m_AI.m_dwStatusFlags &= ~AI_DOORPREOPEN ;
				pThis->m_AI.m_dwStatusFlags |= AI_DOOROPEN;
				pThis->m_AI.m_RetreatTimer = SECONDS_TO_FRAMES(pThis->ah.ih.m_pEA->m_Aggression) ;
				AI_SetDesiredAnim(pThis, AI_ANIM_TIMERACTION_START, TRUE);
			}
		}
	}
	// check OFF timer
	else if (pThis->m_AI.m_dwStatusFlags & AI_DOOROPEN)
	{
		if (pThis->m_AI.m_RetreatTimer <=0)
		{
			// finished OFF, restart if nomore loops, or go forever if 0
			// infinite loop
			if (pThis->m_AI.m_Time1 == 0)
			{
				pThis->m_AI.m_dwStatusFlags &= ~AI_DOOROPEN ;
				pThis->m_AI.m_dwStatusFlags |= AI_DOORPREOPEN;
				pThis->m_AI.m_RetreatTimer = SECONDS_TO_FRAMES(pThis->ah.ih.m_pEA->m_AttackStyle) ;
				AI_SetDesiredAnim(pThis, AI_ANIM_TIMERACTION_GO, TRUE);
			}
			// use loop counter
			else
			{
				// if any more loops left, start it over (same as above bit!)
				pThis->m_AI.m_Time1-- ;
				if (pThis->m_AI.m_Time1)
				{
					pThis->m_AI.m_dwStatusFlags &= ~AI_DOOROPEN ;
					pThis->m_AI.m_dwStatusFlags |= AI_DOORPREOPEN;
					pThis->m_AI.m_RetreatTimer = SECONDS_TO_FRAMES(pThis->ah.ih.m_pEA->m_AttackStyle) ;
					AI_SetDesiredAnim(pThis, AI_ANIM_TIMERACTION_GO, TRUE);
				}
				// no loops left, turn it off (will restart if still on pressure pad)
				else
					pThis->m_AI.m_dwStatusFlags &= ~(AI_STARTED | AI_DOOROPEN | AI_DOORPREOPEN) ;
			}
		}
		else
			pThis->m_AI.m_RetreatTimer -= frame_increment ;
	}


	// Need a test here for Trex level (when level support is finally done!!)
	frame_increment *= 2.5 ;
}


void PressureActionAI_Advance(CGameObjectInstance *pThis)
{
	if (!(pThis->m_AI.m_dwStatusFlags & AI_STARTED))
	{
		AI_SetDesiredAnim(pThis, AI_ANIM_ACTION_START, TRUE);
		pThis->m_AI.m_dwStatusFlags |= AI_STARTED;
		pThis->m_AI.m_RetreatTimer = 0 ;
	}
	// time before becoming active
	else if (pThis->m_AI.m_dwStatusFlags & AI_DOORPREOPEN)
	{
		pThis->m_AI.m_RetreatTimer -= frame_increment ;
		if (pThis->m_AI.m_RetreatTimer <0)
		{
			// message received?
			if (pThis->m_AI.m_InteractiveTimer)
			{
				AI_SetDesiredAnim(pThis, AI_ANIM_ACTION_GO, TRUE);
				pThis->m_AI.m_dwStatusFlags &= ~AI_DOORPREOPEN ;
				pThis->m_AI.m_dwStatusFlags |= AI_DOOROPEN ;
			}
			// no, then restart it
			else
				pThis->m_AI.m_dwStatusFlags &= ~(AI_STARTED | AI_DOOROPEN | AI_DOORPREOPEN) ;
		}
	}
	// action going
	else
	{
		if (pThis->m_AI.m_InteractiveTimer == 0)
			pThis->m_AI.m_dwStatusFlags &= ~(AI_STARTED | AI_DOOROPEN | AI_DOORPREOPEN) ;
	}

	if (pThis->m_AI.m_InteractiveTimer)
		pThis->m_AI.m_InteractiveTimer-- ;
}



void ConstantActionAI_Advance(CGameObjectInstance *pThis)
{
	// start it going if not already
	if (!(pThis->m_AI.m_dwStatusFlags & AI_STARTED))
	{
		AI_SetDesiredAnim(pThis, AI_ANIM_CONSTANTACTION_GO, TRUE);
		pThis->m_AI.m_RetreatTimer = SECONDS_TO_FRAMES(pThis->ah.ih.m_pEA->m_AttackStyle) ;
		pThis->m_AI.m_dwStatusFlags |= (AI_STARTED | AI_DOORPREOPEN) ;
	}

	// already started, check ON timer
	else if (pThis->m_AI.m_dwStatusFlags & AI_DOORPREOPEN)
	{
		if (pThis->m_AI.m_RetreatTimer <=0)
		{
			// finished ON, goto OFF
			pThis->m_AI.m_dwStatusFlags &= ~AI_DOORPREOPEN ;
			pThis->m_AI.m_dwStatusFlags |= AI_DOOROPEN;
			pThis->m_AI.m_RetreatTimer = SECONDS_TO_FRAMES(pThis->ah.ih.m_pEA->m_Aggression) ;
			AI_SetDesiredAnim(pThis, AI_ANIM_CONSTANTACTION_START, TRUE);
		}
		else
			pThis->m_AI.m_RetreatTimer -= frame_increment ;
	}
	// check OFF timer
	else if (pThis->m_AI.m_dwStatusFlags & AI_DOOROPEN)
	{
		if (pThis->m_AI.m_RetreatTimer <=0)
		{
			// finished OFF, goto ON
			pThis->m_AI.m_dwStatusFlags &= ~AI_DOOROPEN ;
			pThis->m_AI.m_dwStatusFlags |= AI_DOORPREOPEN;
			pThis->m_AI.m_RetreatTimer = SECONDS_TO_FRAMES(pThis->ah.ih.m_pEA->m_AttackStyle) ;
			AI_SetDesiredAnim(pThis, AI_ANIM_CONSTANTACTION_GO, TRUE);
		}
		else
			pThis->m_AI.m_RetreatTimer -= frame_increment ;
	}
}



void StatueAI_Advance(CGameObjectInstance *pThis)
{
	if (!(pThis->m_AI.m_dwStatusFlags & AI_STARTED))
	{
		AI_SetDesiredAnim(pThis, AI_ANIM_STATUE_START, TRUE);
		pThis->m_AI.m_dwStatusFlags |= AI_STARTED;
	}
}

void WallAI_Advance(CGameObjectInstance *pThis)
{
	if (!(pThis->m_AI.m_dwStatusFlags & AI_STARTED))
	{
		AI_SetDesiredAnim(pThis, AI_ANIM_WALL_START, TRUE);
		pThis->m_AI.m_dwStatusFlags |= AI_STARTED;
	}
}

//void SpinDoorAI_Advance(CGameObjectInstance *pThis)
//{
//	if (!(pThis->m_AI.m_dwStatusFlags & AI_STARTED))
//	{
//		AI_SetDesiredAnim(pThis, AI_ANIM_SPINDOOR_STILL1, TRUE);
//		pThis->m_AI.m_dwStatusFlags |= AI_STARTED;
//		pThis->m_AI.m_RetreatTimer = 0 ;
//	}
//}


//---------------------------------------------------------------
// Advance routine for LASER
//---------------------------------------------------------------
void LaserAI_Advance(CGameObjectInstance *pThis)
{
	if (pThis->m_AI.m_dwStatusFlags & AI_STARTED)
	{
		if (pThis->m_AI.m_dwStatusFlags & AI_DOOROPEN)
		{
			// if retreat timer is 0, stay lasering(?)
			if (pThis->m_AI.m_RetreatTimer != 0)
			{
				// decrease the timer
				pThis->m_AI.m_RetreatTimer -= frame_increment;

				if (pThis->m_AI.m_RetreatTimer <= 0)
				{
					AI_SetDesiredAnim(pThis, AI_ANIM_LASER_STOP, TRUE);
					pThis->m_AI.m_dwStatusFlags &= ~AI_DOOROPEN;
					pThis->m_AI.m_dwStatusFlags &= ~AI_DOORPREOPEN;
				}
			}
		}
		// check timer before door opens
		else if (pThis->m_AI.m_dwStatusFlags & AI_DOORPREOPEN)
		{
			// decrease the timer
			pThis->m_AI.m_RetreatTimer -= frame_increment;

			if (pThis->m_AI.m_RetreatTimer <= 0)
			{
				// setup timer to keep laser shooting
				pThis->m_AI.m_RetreatTimer = pThis->ah.ih.m_pEA->m_Aggression *FRAME_FPS ;
				AI_SetDesiredAnim(pThis, AI_ANIM_LASER_GO, TRUE);
				pThis->m_AI.m_dwStatusFlags |= AI_DOOROPEN;
			}
		}
	}
	else
	{
		AI_SetDesiredAnim(pThis, AI_ANIM_LASER_START, TRUE);
		pThis->m_AI.m_dwStatusFlags |= AI_STARTED;
	}
}


//---------------------------------------------------------------
// Advance routine for EXPLOSIVE TARGET
//---------------------------------------------------------------
void ExplosiveTargetAI_Advance(CGameObjectInstance *pThis)
{
	if (!(pThis->m_AI.m_dwStatusFlags & AI_STARTED))
	{
		AI_SetDesiredAnim(pThis, AI_ANIM_EXPTARGET_START, TRUE);
		pThis->m_AI.m_dwStatusFlags |= AI_STARTED;
		AI_GetDyn(pThis)->m_dwStatusFlags &= ~AI_EXPTARGETEXPLODED;
	}
}


//---------------------------------------------------------------
// Advance routine for ELEVATOR
//---------------------------------------------------------------
// FLAGS
// AI_STARTED - elevator has been activated
// AI_DOOROPEN(1) - moving toward height
// AI_DOORPREOPEN(1) - waiting
// AI_DOOROPEN(0) - moving away from height
// vars
// m_RetreatTimer - Time to wait at top, before descent
// m_Time1	- blend time from 0..1
// m_Time2	- start y position
// m_TeleportTime	- start position of region

void ElevatorAI_Advance(CGameObjectInstance *pThis)
{
	if (pThis->m_AI.m_dwStatusFlags & AI_STARTED)
	{
		// waiting after all over, check if player still on it...
		if ((pThis->m_AI.m_dwStatusFlags & (AI_DOOROPEN | AI_DOORPREOPEN)) == (AI_DOOROPEN | AI_DOORPREOPEN))
		{
			if (CEngineApp__GetPlayer(GetApp())->ah.ih.m_pCurrentRegion && pThis->ah.ih.m_pCurrentRegion)
			{
				if (CEngineApp__GetPlayer(GetApp())->ah.ih.m_pCurrentRegion->m_nRegionSet == pThis->ah.ih.m_pCurrentRegion->m_nRegionSet)
				{
					pThis->m_AI.m_RetreatTimer -= frame_increment ;
					if (pThis->m_AI.m_RetreatTimer <0)
						pThis->m_AI.m_dwStatusFlags &= ~AI_STARTED ;
				}
				// not on it, so restart
				else
				{
					pThis->m_AI.m_dwStatusFlags &= ~AI_STARTED ;
				}
			}
			else
			{
				pThis->m_AI.m_dwStatusFlags &= ~AI_STARTED ;
			}
		}
		else if (pThis->m_AI.m_dwStatusFlags & AI_DOOROPEN)
			Elevator_MoveTo(pThis) ;
		else if (pThis->m_AI.m_dwStatusFlags & AI_DOORPREOPEN)
			Elevator_Wait(pThis) ;
		else
		{
			Elevator_MoveFrom(pThis) ;

			// if finished
			if (!(pThis->m_AI.m_dwStatusFlags & AI_STARTED))
			{
				pThis->m_AI.m_dwStatusFlags |= (AI_STARTED | AI_DOOROPEN | AI_DOORPREOPEN) ;
				pThis->m_AI.m_RetreatTimer = SECONDS_TO_FRAMES(pThis->ah.ih.m_pEA->m_StartHealth) ;
			}
		}
	}
}

//---------------------------------------------------------------
// Advance routine for PLATFORM
//---------------------------------------------------------------
// see ELEVATOR
// HEIGHT NOT MULITPLIED BY ANYTHING
void PlatformAI_Advance(CGameObjectInstance *pThis)
{
	if (pThis->m_AI.m_dwStatusFlags & AI_STARTED)
	{
		if (pThis->m_AI.m_dwStatusFlags & AI_DOOROPEN)
		{
			Elevator_MoveTo(pThis) ;

			// if moved to top, don't wait go straight back
			if (pThis->m_AI.m_dwStatusFlags & AI_DOORPREOPEN)
	 			pThis->m_AI.m_dwStatusFlags &= ~AI_DOORPREOPEN;
		}
		else
		{
			Elevator_MoveFrom(pThis) ;
			if (!(pThis->m_AI.m_dwStatusFlags & AI_STARTED))
				pThis->m_AI.m_dwStatusFlags |= AI_STARTED | AI_DOOROPEN ;
		}
	}
}

//---------------------------------------------------------------
// Advance routine for DEATHELEVATOR
//---------------------------------------------------------------
// same as ELEVATOR, except you die at some point
void DeathElevatorAI_Advance(CGameObjectInstance *pThis)
{
	if (pThis->m_AI.m_dwStatusFlags & AI_STARTED)
	{
		// change to instant death, if below death
		Elevator_ChangeRegion(pThis) ;

		// waiting after all over, check if player still on it...
		if ((pThis->m_AI.m_dwStatusFlags & (AI_DOOROPEN | AI_DOORPREOPEN)) == (AI_DOOROPEN | AI_DOORPREOPEN))
		{
			if (CEngineApp__GetPlayer(GetApp())->ah.ih.m_pCurrentRegion && pThis->ah.ih.m_pCurrentRegion)
			{
				if (CEngineApp__GetPlayer(GetApp())->ah.ih.m_pCurrentRegion->m_nRegionSet == pThis->ah.ih.m_pCurrentRegion->m_nRegionSet)
				{
					pThis->m_AI.m_RetreatTimer -= frame_increment ;
					if (pThis->m_AI.m_RetreatTimer <0)
						pThis->m_AI.m_dwStatusFlags &= ~AI_STARTED ;
				}
				// not on it, so restart
				else
				{
					pThis->m_AI.m_dwStatusFlags &= ~AI_STARTED ;
				}
			}
			else
			{
				pThis->m_AI.m_dwStatusFlags &= ~AI_STARTED ;
			}
		}
		else if (pThis->m_AI.m_dwStatusFlags & AI_DOOROPEN)
			Elevator_MoveTo(pThis) ;
		else if (pThis->m_AI.m_dwStatusFlags & AI_DOORPREOPEN)
			Elevator_Wait(pThis) ;
		else
		{
			Elevator_MoveFrom(pThis) ;

			// if finished
			if (!(pThis->m_AI.m_dwStatusFlags & AI_STARTED))
			{
				pThis->m_AI.m_dwStatusFlags |= (AI_STARTED | AI_DOOROPEN | AI_DOORPREOPEN) ;
				pThis->m_AI.m_RetreatTimer = SECONDS_TO_FRAMES(pThis->ah.ih.m_pEA->m_StartHealth) ;
			}
		}
	}
}

//---------------------------------------------------------------
// Advance routine for COLLAPSINGPLATFORM
//---------------------------------------------------------------
// initial delay(starthealth), then falls to death(t1 over t2 secs), resets after time(interactiveanim)
void CollapsingPlatformAI_Advance(CGameObjectInstance *pThis)
{
	if (pThis->m_AI.m_dwStatusFlags & AI_STARTED)
	{
		// change to instant death, if below death
		Elevator_ChangeRegion(pThis) ;

		// waiting for initial delay to be over
		if ((pThis->m_AI.m_dwStatusFlags & (AI_DOORPREOPEN | AI_DOOROPEN)) == AI_DOORPREOPEN)
		{
			pThis->m_AI.m_RetreatTimer -= frame_increment ;

			if (pThis->m_AI.m_RetreatTimer <= 0)
			{
				pThis->m_AI.m_dwStatusFlags &= ~AI_DOORPREOPEN ;
				pThis->m_AI.m_dwStatusFlags |= AI_DOOROPEN ;
			}
		}
		else if ((pThis->m_AI.m_dwStatusFlags & (AI_DOORPREOPEN | AI_DOOROPEN)) == AI_DOOROPEN)
		{
			Elevator_MoveTo(pThis) ;

			// finished?
			if (pThis->m_AI.m_dwStatusFlags & AI_DOORPREOPEN)
			{
				// set to reset position
				pThis->m_AI.m_RetreatTimer = SECONDS_TO_FRAMES(pThis->ah.ih.m_pEA->m_InteractiveAnim) ;
				pThis->m_AI.m_dwStatusFlags |= (AI_DOOROPEN | AI_DOORPREOPEN) ;
			}
		}
		else if ((pThis->m_AI.m_dwStatusFlags & (AI_DOOROPEN | AI_DOORPREOPEN)) == (AI_DOOROPEN | AI_DOORPREOPEN))
		{
			if (pThis->m_AI.m_RetreatTimer <= 0)
				Elevator_MoveFrom(pThis) ;
			else
				pThis->m_AI.m_RetreatTimer -= frame_increment ;

			// finished?
			//if (!(pThis->m_AI.m_dwStatusFlags & AI_STARTED))
		  	//{
			//	// fool it to keep moveing
			//	pThis->m_AI.m_dwStatusFlags |= AI_DOOROPEN ;
			//	pThis->m_AI.m_dwStatusFlags &= ~AI_DOORPREOPEN ;
			//}
		}
	}
}


// TREX ELEVATOR AND PLATFORMS NOW GONE!!
#if 0

//---------------------------------------------------------------
// Advance routine for TREXELEVATOR
//---------------------------------------------------------------
// similar to ELEVATOR,
// moves up, trex gets off
// moves down, opens up,
// collision moves down to create a big hole
BOOL TrexElevator_KeepPlayerOn(CGameObjectInstance *pThis)
{
	CGameObjectInstance	*pPlayer ;
	FLOAT						Height ;
	CGameRegion				*pElevatorRegion, *pPlayerRegion ;
	CROMRegionSet			*pPlayerRegionSet ;

	// Setup elevator vars
	pElevatorRegion = pThis->ah.ih.m_pCurrentRegion ;
	if (!pElevatorRegion)
		return FALSE ;

	// Setup player vars
	pPlayer = CEngineApp__GetPlayer(GetApp()) ;
	pPlayerRegion = pPlayer->ah.ih.m_pCurrentRegion ;
	if (!pPlayerRegion)
		return FALSE ;

	// Check region set to see if player is on elevator or platform regions
	pPlayerRegionSet = CScene__GetRegionAttributes(&GetApp()->m_Scene, pPlayerRegion) ;
	if ((!pPlayerRegionSet) || (!(pPlayerRegionSet->m_dwFlags & REGFLAG_PRESSUREPLATE)))
		return FALSE ;

	switch(pPlayerRegionSet->m_PressureID)
	{
		case 10:	// Elevator
		case 1:	// Platform
		case 2:	// Platform
		case 3:	// Platform
		case 4:	// Platform

			// if player is stood on this elevator, drag him with it.
			Height = CGameRegion__GetGroundHeight(pElevatorRegion, pThis->ah.ih.m_vPos.x, pThis->ah.ih.m_vPos.z) ;

//				osSyncPrintf("Checking: ID:%d H:%f  PY:%f PV:%f\r\n",
//								 pPlayerRegionSet->m_PressureID,
//								 Height/SCALING_FACTOR,
//								 pPlayer->ah.ih.m_vPos.y/SCALING_FACTOR,
//								 pPlayer->ah.m_vVelocity.y/SCALING_FACTOR) ;

			// Player below surface?
  			if ((pPlayer->ah.m_vVelocity.y <= 0) && (pPlayer->ah.ih.m_vPos.y <= Height))
			{
//				osSyncPrintf("Positioned!r\n") ;

				pPlayer->ah.ih.m_vPos.y = Height ;
				pPlayer->ah.m_vVelocity.y = 0 ;
				return TRUE ;
			}
			break ;

	}

	return FALSE ;
}

void TrexElevatorAI_Advance(CGameObjectInstance *pThis)
{
	CInstanceHdr			*pHdr ;

	// Okay to process?
	if (pThis->m_AI.m_dwStatusFlags & AI_STARTED)
	{
		// Moving up?
		if (pThis->m_AI.m_dwStatusFlags & AI_DOOROPEN)
			Elevator_MoveTo(pThis) ;

		// Open hole when trex if off elevator
		if ((GetApp()->m_pBossActive) &&
			 (GetApp()->m_pBossActive->m_pBoss->m_Mode > TREX_EXIT_ELEVATOR_MODE) &&
			 (pThis->m_AI.m_dwStatusFlags & AI_DOORPREOPEN))
		{
			CScene__SetInstanceFlag(&GetApp()->m_Scene, pThis, TRUE) ;

			// Don't open again!
			pThis->m_AI.m_RetreatTimer = 48 ;
			pThis->m_AI.m_dwStatusFlags &= ~AI_DOORPREOPEN ;

			// Turn elevator to death
			CGameRegion__SetGroundHeight(pThis->ah.ih.m_pCurrentRegion, pThis->m_AI.m_TeleportTime) ;
			AI_SetDesiredAnim(pThis, AI_ANIM_TREXELEVATOR_OPEN, TRUE) ;
			CGameRegion__MakeFallDeath(pThis->ah.ih.m_pCurrentRegion) ;

			// Position trex platforms to correct region height
			pHdr = &pThis->ah.ih ;
			AI_Event_Dispatcher(pHdr, pHdr, AI_MEVENT_PRESSUREPLATE, pHdr->m_pEA->m_dwSpecies, pHdr->m_vPos, TREX_LEVEL_PLATFORM1_ID) ;
			AI_Event_Dispatcher(pHdr, pHdr, AI_MEVENT_PRESSUREPLATE, pHdr->m_pEA->m_dwSpecies, pHdr->m_vPos, TREX_LEVEL_PLATFORM2_ID) ;
			AI_Event_Dispatcher(pHdr, pHdr, AI_MEVENT_PRESSUREPLATE, pHdr->m_pEA->m_dwSpecies, pHdr->m_vPos, TREX_LEVEL_PLATFORM3_ID) ;
			AI_Event_Dispatcher(pHdr, pHdr, AI_MEVENT_PRESSUREPLATE, pHdr->m_pEA->m_dwSpecies, pHdr->m_vPos, TREX_LEVEL_PLATFORM4_ID) ;
			AI_Event_Dispatcher(pHdr, pHdr, AI_MEVENT_PRESSUREPLATE, pHdr->m_pEA->m_dwSpecies, pHdr->m_vPos, TREX_LEVEL_PLATFORM5_ID) ;
		}

		// Keep player on elevator when moving up / not closed
		if (pThis->m_AI.m_dwStatusFlags & (AI_DOOROPEN | AI_DOORPREOPEN))
			TrexElevator_KeepPlayerOn(pThis) ;
		else
		if (pThis->m_AI.m_RetreatTimer > 0)
		{
			// Stop updating elevator
			if (--pThis->m_AI.m_RetreatTimer <= 0)
				CGameObjectInstance__SetGone(pThis)  ;
		}
	}
}


//---------------------------------------------------------------
// Advance routine for TREXPLATFORM
//---------------------------------------------------------------
// sits and waits for player to stand on it
// if standing for too long will go back into wall,
// and make collision go down big hole to make player fall off
void TrexPlatformAI_Advance(CGameObjectInstance *pThis)
{
	// if NOT STARTED, set it up
	if (!(pThis->m_AI.m_dwStatusFlags & AI_STARTED))
	{
		pThis->m_AI.m_dwStatusFlags |= AI_STARTED ;
		AI_SetDesiredAnim(pThis, AI_ANIM_TREXPLATFORM_CLOSED, TRUE);

		// store original position of collision
		pThis->m_AI.m_Time2 = CGameRegion__GetGroundHeight(pThis->ah.ih.m_pCurrentRegion, pThis->ah.ih.m_vPos.x, pThis->ah.ih.m_vPos.z) ;

		// move to 'BAD' position
		CGameRegion__SetGroundHeight(pThis->ah.ih.m_pCurrentRegion, pThis->m_AI.m_Time2 - (100*SCALING_FACTOR)) ;
	}

	// STARTED, update timer if still on it
	else if (pThis->m_AI.m_dwStatusFlags & AI_DOOROPEN)
	{
		if (CEngineApp__GetPlayer(GetApp())->ah.ih.m_pCurrentRegion->m_nRegionSet == pThis->ah.ih.m_pCurrentRegion->m_nRegionSet)
		{
			pThis->m_AI.m_RetreatTimer += frame_increment ;
			if (pThis->m_AI.m_RetreatTimer >= pThis->m_AI.m_Time1)
			{
				AI_SetDesiredAnim(pThis, AI_ANIM_TREXPLATFORM_FALL, TRUE);
				pThis->m_AI.m_dwStatusFlags &= ~AI_DOOROPEN ;

				// move to 'BAD' position and set to death
				CGameRegion__MakeFallDeath(pThis->ah.ih.m_pCurrentRegion) ;
				CGameRegion__SetGroundHeight(pThis->ah.ih.m_pCurrentRegion, pThis->m_AI.m_Time2 - (100*SCALING_FACTOR)) ;
			}
		}
		// not on it, so restart timer
		else
		{
			pThis->m_AI.m_RetreatTimer = 0 ;
		}
	}
}

#endif



#if 0
// Switches gone!!

//---------------------------------------------------------------
// Advance routine for LONGHUNTER SWITCH
//---------------------------------------------------------------
void LonghunterSwitchAI_Advance(CGameObjectInstance *pThis)
{
	if (!(pThis->m_AI.m_dwStatusFlags & AI_STARTED))
	{
		AI_SetDesiredAnim(pThis, AI_ANIM_LONGHUNTER_SWITCH_START, TRUE) ;
		pThis->m_AI.m_dwStatusFlags |= AI_STARTED ;
	}
}

#endif


//---------------------------------------------------------------
// Advance routine for PORTAL
//---------------------------------------------------------------
void PortalAI_Advance(CGameObjectInstance *pThis)
{
	if (pThis->m_AI.m_dwStatusFlags & AI_STARTED)
	{
		CScene__DoSoundEffect(&GetApp()->m_Scene, 572, 1, &pThis->ah.ih, &pThis->ah.ih.m_vPos, 0) ;
		if (!(pThis->m_AI.m_dwStatusFlags & AI_DOOROPEN))
		{
			CGameRegion__MakeWarp(pThis->ah.ih.m_pCurrentRegion) ;
			pThis->m_AI.m_dwStatusFlags |= AI_DOOROPEN ;
		}
		if (pThis->m_AI.m_Time1 < 1.0)
		{
			pThis->m_AI.m_Time1 += .03 * frame_increment ;
			if (pThis->m_AI.m_Time1 > 1.0)
				pThis->m_AI.m_Time1 = 1.0 ;
		}
	}
	else
 		pThis->m_AI.m_Time1 = 0.0 ;
}

//---------------------------------------------------------------
// Advance routine for GATE
//---------------------------------------------------------------
void GateAI_Advance(CGameObjectInstance *pThis)
{
}


//---------------------------------------------------------------
// Advance routine for LOCK
//---------------------------------------------------------------
void LockAI_Advance(CGameObjectInstance *pThis)
{
	if (pThis->m_AI.m_dwStatusFlags & AI_STARTED)
	{
		if (pThis->m_AI.m_dwStatusFlags & AI_DOOROPEN)
		{
			Elevator_MoveTo(pThis) ;

			// if reached bottom, make warp and stop movement
			if (pThis->m_AI.m_dwStatusFlags & AI_DOORPREOPEN)
			{
				CGameRegion__MakeWarp(pThis->ah.ih.m_pCurrentRegion) ;
	 			pThis->m_AI.m_dwStatusFlags &= ~AI_DOORPREOPEN;
			}
		}
	}
}


//---------------------------------------------------------------
// Advance routine for DRAIN
//---------------------------------------------------------------
void DrainAI_Advance(CGameObjectInstance *pThis)
{
	CROMRegionSet			*pRegionSet;

	// make instant drain
	if ((pThis->m_AI.m_dwStatusFlags & AI_STARTED) &&
		 (pThis->m_AI.m_dwStatusFlags & AI_DOORPREOPEN))
	{
		pRegionSet = CScene__GetRegionAttributes(&GetApp()->m_Scene, pThis->ah.ih.m_pCurrentRegion);
		pRegionSet->m_WaterElevation = pThis->ah.ih.m_pEA->m_DeadHeight ;
		pThis->ah.ih.m_vPos.y = pThis->ah.ih.m_pEA->m_DeadHeight ;
		CGameRegion__FromWater(pThis->ah.ih.m_pCurrentRegion) ;
		pThis->m_AI.m_dwStatusFlags &= ~AI_DOORPREOPEN ;
	}

	if ((pThis->m_AI.m_dwStatusFlags & AI_STARTED) &&
		((pThis->m_AI.m_dwStatusFlags & AI_DOOROPEN) == 0))
	{
		pThis->ah.ih.m_vPos.y -= 1 ;

		pRegionSet = CScene__GetRegionAttributes(&GetApp()->m_Scene, pThis->ah.ih.m_pCurrentRegion);
		if (pRegionSet)
			pRegionSet->m_WaterElevation = pThis->ah.ih.m_vPos.y ;

		// reached end, stop it
		if (pThis->ah.ih.m_vPos.y < pThis->ah.ih.m_pEA->m_DeadHeight)
		{
			pThis->m_AI.m_dwStatusFlags |= AI_DOOROPEN ;
			CGameRegion__FromWater(pThis->ah.ih.m_pCurrentRegion) ;
		}
	}
}

//---------------------------------------------------------------
// Advance routine for FLOOD
//---------------------------------------------------------------
void FloodAI_Advance(CGameObjectInstance *pThis)
{
	CROMRegionSet			*pRegionSet;

	if ((pThis->m_AI.m_dwStatusFlags & AI_STARTED) &&
		((pThis->m_AI.m_dwStatusFlags & AI_DOOROPEN) == 0))
	{


	pThis->ah.ih.m_vPos.y = pThis->ah.ih.m_pEA->m_DeadHeight ;
	pRegionSet = CScene__GetRegionAttributes(&GetApp()->m_Scene, pThis->ah.ih.m_pCurrentRegion);
	if (pRegionSet)
		pRegionSet->m_WaterElevation = pThis->ah.ih.m_vPos.y ;

	pThis->m_AI.m_dwStatusFlags |= AI_DOOROPEN ;

	CGameRegion__UnMakeInstantDeath(pThis->ah.ih.m_pCurrentRegion) ;
	CGameRegion__ToWater(pThis->ah.ih.m_pCurrentRegion) ;

//		pThis->ah.ih.m_vPos.y += 10 ;
//
//		pRegionSet = CScene__GetRegionAttributes(&GetApp()->m_Scene, pThis->ah.ih.m_pCurrentRegion);
//		if (pRegionSet)
//			pRegionSet->m_WaterElevation = pThis->ah.ih.m_vPos.y ;
//
//		// reached end, stop it
//		if (pThis->ah.ih.m_vPos.y >= pThis->ah.ih.m_pEA->m_DeadHeight)
//		{
//			// move to correct position
//			pThis->ah.ih.m_vPos.y = pThis->ah.ih.m_pEA->m_DeadHeight ;
//			if (pRegionSet)
//				pRegionSet->m_WaterElevation = pThis->ah.ih.m_vPos.y ;
//
//			pThis->m_AI.m_dwStatusFlags |= AI_DOOROPEN ;
//		}
	}
}

//---------------------------------------------------------------
// Advance routine for CRYSTAL
//---------------------------------------------------------------
void CrystalAI_Advance(CGameObjectInstance *pThis)
{
	CInstanceHdr	*pOrigin = &pThis->ah.ih ;

	if (!(pThis->m_AI.m_dwStatusFlags & AI_STARTED))
	{
		pThis->m_AI.m_TeleportTime = 0 ;
		pThis->m_AI.m_Time1 = 0 ;
		pThis->m_AI.m_Time2 = 0 ;
		AI_SetDesiredAnim(pThis, AI_ANIM_CRYSTAL_START, TRUE);
		pThis->m_AI.m_dwStatusFlags |= AI_STARTED ;
	}

	// Start off mantis coccoon?
	if (pThis->m_AI.m_TeleportTime > 0)
	{
		pThis->m_AI.m_TeleportTime -= frame_increment ;
		if (pThis->m_AI.m_TeleportTime <= 0)
		{
			pThis->m_AI.m_TeleportTime = 0 ;

			AI_Event_Dispatcher(pOrigin, pOrigin, AI_MEVENT_STATUE,
									  AI_SPECIES_ALL, pOrigin->m_vPos, MANTIS_LEVEL_COCOON_ID) ;

		}
	}

}

//---------------------------------------------------------------
// Advance routine for SPINELEVATOR
//---------------------------------------------------------------
// see ELEVATOR

void Elevator_UpdateSpin(CGameObjectInstance *pThis)
{
	CMtxF	mRot ;
	CVector3	vPos, vOut ;

	float	height, prerot, rotinc ;
	CGameRegion	*pRegion = pThis->ah.ih.m_pCurrentRegion ;
	CGameRegion	*pOutRegion ;
	CGameObjectInstance *pPlayer = CEngineApp__GetPlayer(GetApp());

	if (!pRegion)
		return;

	height = (pThis->ah.ih.m_vPos.y - pThis->m_AI.m_Time2) / SCALING_FACTOR ;
	prerot = pThis->m_RotY ;
	pThis->m_RotY = height * (2*M_PI) / AI_GetEA(pThis)->m_InteractiveAnim ;
	pThis->m_RotY += pThis->m_AI.m_ViewAngleOffset ;
	rotinc = pThis->m_RotY - prerot;


	// if player stood on elevator, make him spin with it...
	if (pPlayer->ah.ih.m_pCurrentRegion)
	{
		if ((pPlayer->ah.ih.m_pCurrentRegion->m_nRegionSet == pRegion->m_nRegionSet) &&
			 (CInstanceHdr__IsOnGround(&pPlayer->ah.ih)))
		{
			// rotate around y
			CMtxF__Translate(mRot, -pThis->ah.ih.m_vPos.x, 0, -pThis->ah.ih.m_vPos.z);
			CMtxF__PostMultRotateY(mRot, rotinc);
			CMtxF__PostMultTranslate(mRot, pThis->ah.ih.m_vPos.x, 0, pThis->ah.ih.m_vPos.z);

			CMtxF__VectorMult(mRot, &pPlayer->ah.ih.m_vPos, &vPos);

			CInstanceHdr__GetNearPositionAndRegion(&pPlayer->ah.ih, vPos, &vOut, &pOutRegion, INTERSECT_BEHAVIOR_IGNORE, INTERSECT_BEHAVIOR_IGNORE);
			pPlayer->ah.ih.m_vPos = vOut ;
			pPlayer->ah.ih.m_pCurrentRegion = pOutRegion ;
			pPlayer->m_RotY += rotinc ;
		}
	}
}

void SpinElevatorAI_Advance(CGameObjectInstance *pThis)
{
	if (pThis->m_AI.m_dwStatusFlags & AI_STARTED)
	{
		// waiting after all over, check if player still on it...
		if ((pThis->m_AI.m_dwStatusFlags & (AI_DOOROPEN | AI_DOORPREOPEN)) == (AI_DOOROPEN | AI_DOORPREOPEN))
		{
			if (CEngineApp__GetPlayer(GetApp())->ah.ih.m_pCurrentRegion && pThis->ah.ih.m_pCurrentRegion)
			{
				if (CEngineApp__GetPlayer(GetApp())->ah.ih.m_pCurrentRegion->m_nRegionSet == pThis->ah.ih.m_pCurrentRegion->m_nRegionSet)
				{
					pThis->m_AI.m_RetreatTimer -= frame_increment ;
					if (pThis->m_AI.m_RetreatTimer <0)
						pThis->m_AI.m_dwStatusFlags &= ~AI_STARTED ;
				}
				// not on it, so restart
				else
					pThis->m_AI.m_dwStatusFlags &= ~AI_STARTED ;
			}
			else
				pThis->m_AI.m_dwStatusFlags &= ~AI_STARTED ;
		}
		else if (pThis->m_AI.m_dwStatusFlags & AI_DOOROPEN)
		{
			Elevator_MoveTo(pThis) ;

			Elevator_UpdateSpin(pThis) ;

		}
		else if (pThis->m_AI.m_dwStatusFlags & AI_DOORPREOPEN)
			Elevator_Wait(pThis) ;
		else
		{
			Elevator_MoveFrom(pThis) ;

			Elevator_UpdateSpin(pThis) ;

			// if finished
			if (!(pThis->m_AI.m_dwStatusFlags & AI_STARTED))
			{
				pThis->m_AI.m_dwStatusFlags |= (AI_STARTED | AI_DOOROPEN | AI_DOORPREOPEN) ;
				pThis->m_AI.m_RetreatTimer = SECONDS_TO_FRAMES(pThis->ah.ih.m_pEA->m_StartHealth) ;
			}
		}
	}
}

//---------------------------------------------------------------
// Advance routine for SPINPLATFORM
//---------------------------------------------------------------
// see PLATFORM
void SpinPlatformAI_Advance(CGameObjectInstance *pThis)
{
	if (pThis->m_AI.m_dwStatusFlags & AI_STARTED)
	{
		if (pThis->m_AI.m_dwStatusFlags & AI_DOOROPEN)
		{
			Elevator_UpdateSpin(pThis) ;
			Elevator_MoveTo(pThis) ;

			// if moved to top, don't wait go straight back
			if (pThis->m_AI.m_dwStatusFlags & AI_DOORPREOPEN)
	 			pThis->m_AI.m_dwStatusFlags &= ~AI_DOORPREOPEN;
		}
		else
		{
			Elevator_UpdateSpin(pThis) ;
			Elevator_MoveFrom(pThis) ;
			if (!(pThis->m_AI.m_dwStatusFlags & AI_STARTED))
				pThis->m_AI.m_dwStatusFlags |= AI_STARTED | AI_DOOROPEN ;
		}
	}
}




//---------------------------------------------------------------
// Advance routine for KEYFLOOR
//---------------------------------------------------------------
void KeyFloorAI_Advance(CGameObjectInstance *pThis)
{
}




//---------------------------------------------------------------
// Advance routine for FADEINFADEOUT
//---------------------------------------------------------------
void FadeInFadeOutAI_Advance(CGameObjectInstance *pThis)
{
	// Setup first appearence
	if (!(pThis->m_AI.m_dwStatusFlags & AI_STARTED))
	{
		// Signal started
		pThis->m_AI.m_dwStatusFlags |= AI_STARTED ;

		// Set initial state to not there!
		pThis->m_AI.m_dwStatusFlags &= ~AI_DOOROPEN ;
		CGameObjectInstance__SetGone(pThis) ;
	}
}




//---------------------------------------------------------------------------------------------
// ELEVATOR MOVEMENT ROUTINES
//---------------------------------------------------------------------------------------------
// create motion vars for elevator
void Elevator_CreateVars(CGameObjectInstance *pThis, float *increment, float *startpos, float *maxpos)
{
	if (pThis->ah.ih.m_pEA->m_Aggression == 0)
		*increment = 1 ;
	else
		*increment = 1/(float)SECONDS_TO_FRAMES(pThis->ah.ih.m_pEA->m_Aggression) ;
	*startpos = pThis->m_AI.m_Time2 ;

	switch (CGameObjectInstance__TypeFlag(pThis))
	{
		// use time1 as FEET
		case AI_OBJECT_DEVICE_PLATFORM:
		case AI_OBJECT_DEVICE_SPINPLATFORM:
		case AI_OBJECT_DEVICE_FOOTELEVATOR:
		case AI_OBJECT_DEVICE_TREXELEVATOR:
			*maxpos = *startpos + (pThis->ah.ih.m_pEA->m_AttackStyle * SCALING_FACTOR) ;
			break ;
		// use time1 as 5 FEET
		default:
			*maxpos = *startpos + ((pThis->ah.ih.m_pEA->m_AttackStyle *5) * SCALING_FACTOR) ;
			break ;
	}
}

//---------------------------------------------------------------
// move object and region to new height
void Elevator_Update(CGameObjectInstance *pThis, float startpos, float maxpos)
{
	float time, height ;
	CGameRegion	*pRegion = pThis->ah.ih.m_pCurrentRegion ;
	CGameObjectInstance *pPlayer = CEngineApp__GetPlayer(GetApp());

	if (!pRegion)
		return;

	time = pThis->m_AI.m_Time1 ;
	pThis->ah.ih.m_vPos.y = BlendFLOATCosine(time, startpos, maxpos) ;

	CGameRegion__SetGroundHeight(pRegion, pThis->m_AI.m_TeleportTime + (pThis->ah.ih.m_vPos.y - startpos)) ;

	if (pPlayer->ah.ih.m_pCurrentRegion)
	{
		// if player is stood on this elevator, drag him with it.
		height = CGameRegion__GetGroundHeight(pRegion, pThis->ah.ih.m_vPos.x, pThis->ah.ih.m_vPos.z) ;

  		if ((pPlayer->ah.ih.m_pCurrentRegion->m_nRegionSet == pRegion->m_nRegionSet) &&
			(pPlayer->ah.m_vVelocity.y <= 0) &&
			(abs(pPlayer->ah.ih.m_vPos.y - height) < (2*SCALING_FACTOR)))
		{
			pPlayer->ah.ih.m_vPos.y = height ;
		}
	}
}

//---------------------------------------------------------------
// checks if elevator above or below 'death' line, and changes region accordingly
void Elevator_ChangeRegion(CGameObjectInstance *pThis)
{
	float height ;
	u8		warp ;

	CGameRegion	*pRegion = pThis->ah.ih.m_pCurrentRegion ;

	if (!pRegion)
		return;

	height = CGameRegion__GetGroundHeight(pRegion, pThis->ah.ih.m_vPos.x, pThis->ah.ih.m_vPos.z) ;

	if (AI_GetEA(pThis)->m_dwTypeFlags2 & AI_TYPE2_DEVICEWARPDEATH)
		warp = TRUE ;
	else
		warp = FALSE ;

	if (height < pThis->ah.ih.m_pEA->m_DeadHeight)
	{
		if (warp)
			CGameRegion__MakeWarp(pThis->ah.ih.m_pCurrentRegion) ;
		else
			CGameRegion__MakeInstantDeath(pThis->ah.ih.m_pCurrentRegion) ;
	}
	else
	{
		if (warp)
			CGameRegion__UnMakeWarp(pThis->ah.ih.m_pCurrentRegion) ;
		else
			CGameRegion__UnMakeInstantDeath(pThis->ah.ih.m_pCurrentRegion) ;
	}
}


//---------------------------------------------------------------
// moving TO height
void Elevator_MoveTo(CGameObjectInstance *pThis)
{
	float	startpos, maxpos, increment ;
	int	sound ;
	CVector3	vPos ;

	Elevator_CreateVars(pThis, &increment, &startpos, &maxpos) ;

	// play a rumble sound?
	if (		(AI_GetEA(pThis)->m_DeathId)
			&& (pThis->m_AI.m_Time1 != 1))
	{
		sound = AI_GetEA(pThis)->m_DeathId ;

		// make slow down and stop sound?
		if ((sound == 561) && (pThis->m_AI.m_Time1 > .75))
		{
			sound = 0 ;		// kill enviromental sound
			if (AI_GetDyn(pThis)->m_Evade == 0)
			{
				AI_GetDyn(pThis)->m_Evade = 1 ;
				AI_DoSound((CInstanceHdr *)pThis, 562, 1, 0);
			}
		}
		else
			AI_GetDyn(pThis)->m_Evade = 0 ;

		if (sound)
		{
			vPos.x = pThis->ah.ih.m_vPos.x ;
			vPos.z = pThis->ah.ih.m_vPos.z ;
			vPos.y = pThis->m_AI.m_TeleportTime + (pThis->ah.ih.m_vPos.y - pThis->m_AI.m_Time2) ;
			CScene__DoSoundEffect(&GetApp()->m_Scene,
										sound, 1,
									   &pThis->ah.ih, &vPos, 0) ;
		}
	}
	pThis->m_AI.m_Time1 += increment * frame_increment ;
	if (pThis->m_AI.m_Time1 >= 1)
	{
 		pThis->m_AI.m_dwStatusFlags &= ~AI_DOOROPEN;
		pThis->m_AI.m_dwStatusFlags |= AI_DOORPREOPEN;
		pThis->m_AI.m_Time1 = 1 ;

		// if this is 0, will wait forever...
		pThis->m_AI.m_RetreatTimer = SECONDS_TO_FRAMES(pThis->ah.ih.m_pEA->m_StartHealth) ;
	}
	Elevator_Update(pThis, startpos, maxpos) ;
}

//---------------------------------------------------------------
// waiting
void Elevator_Wait(CGameObjectInstance *pThis)
{
	if (pThis->m_AI.m_RetreatTimer != 0)
	{
		pThis->m_AI.m_RetreatTimer -= frame_increment ;
		if (pThis->m_AI.m_RetreatTimer <=0)
		{
			pThis->m_AI.m_RetreatTimer = 0 ;
 			pThis->m_AI.m_dwStatusFlags &= ~AI_DOORPREOPEN;
		}
	}
}

//---------------------------------------------------------------
// moving FROM height
void Elevator_MoveFrom(CGameObjectInstance *pThis)
{
	float	startpos, maxpos, increment ;
	int	sound ;
	CVector3	vPos ;

	Elevator_CreateVars(pThis, &increment, &startpos, &maxpos) ;

	// play a rumble sound?
	if (AI_GetEA(pThis)->m_DeathId)
	{
		sound = AI_GetEA(pThis)->m_DeathId ;

		// make slow down and stop sound?
		if ((sound == 561) && (pThis->m_AI.m_Time1 < .25))
		{
			sound = 0 ;		// kill enviromental sound
			if (AI_GetDyn(pThis)->m_Evade == 0)
			{
				AI_GetDyn(pThis)->m_Evade = 1 ;
				AI_DoSound((CInstanceHdr *)pThis, 562, 1, 0);
			}
		}
		else
			AI_GetDyn(pThis)->m_Evade = 0 ;

		if (sound)
		{
			vPos.x = pThis->ah.ih.m_vPos.x ;
			vPos.z = pThis->ah.ih.m_vPos.z ;
			vPos.y = pThis->m_AI.m_TeleportTime + (pThis->ah.ih.m_vPos.y - pThis->m_AI.m_Time2) ;
			CScene__DoSoundEffect(&GetApp()->m_Scene,
										sound, 1,
									   &pThis->ah.ih, &vPos, 0) ;
		}
	}

	pThis->m_AI.m_Time1 -= increment * frame_increment ;
	if (pThis->m_AI.m_Time1 <= 0)
	{
		// finished movement, restart
	 	pThis->m_AI.m_dwStatusFlags &= ~AI_STARTED ;
		pThis->m_AI.m_Time1 = 0 ;

		// turn off in persistant data
		CScene__SetInstanceFlag(&GetApp()->m_Scene, pThis, FALSE);

	}
	Elevator_Update(pThis, startpos, maxpos) ;
}




//----------------------------------------------------------------------------
// DOORS
// used by crystal plynth on MANTIS BOSS level
// still on START
// sinks into ground on OPEN and stays there
// opens up with CLOSE when retreat timer is set not zero by boss dying
void DoorAI_MEvent_OpenClose(CGameObjectInstance *pThis, BOOL Open)
{
	if (Open)
	{
		// if already starting to open, leave it
		if ((pThis->m_AI.m_dwStatusFlags & AI_DOORPREOPEN) == 0)
		{
			// if there is an attackstyle (time before door opens) use it
			if (pThis->m_AI.m_RetreatTimer = pThis->ah.ih.m_pEA->m_AttackStyle != 0)
			{
				pThis->m_AI.m_dwStatusFlags |= AI_DOORPREOPEN;
				pThis->m_AI.m_RetreatTimer = pThis->ah.ih.m_pEA->m_AttackStyle*FRAME_FPS ;
			}
			else
			{
				pThis->m_AI.m_dwStatusFlags |= AI_DOOROPEN;
				pThis->m_AI.m_RetreatTimer = pThis->ah.ih.m_pEA->m_Aggression *FRAME_FPS ;
				AI_SetDesiredAnim(pThis, AI_ANIM_DOOR_OPEN, TRUE);

				// if going to stay open forever, keep in persistant data
				if (pThis->m_AI.m_RetreatTimer == 0)
					CScene__SetInstanceFlag(&GetApp()->m_Scene, pThis, TRUE);
			}
		}
	}
	else
	{
		AI_SetDesiredAnim(pThis, AI_ANIM_DOOR_CLOSE, TRUE);

		pThis->m_AI.m_dwStatusFlags &= ~AI_DOOROPEN;
 		pThis->m_AI.m_dwStatusFlags &= ~AI_DOORPREOPEN;
	}
}

void DoorAI_SEvent_AllowBlockEntry(CInstanceHdr *pThis, CVector3 vPos, BOOL AllowEntry)
{
	CVector3					vDoorPos;
	CGameRegion				*pDoorRegion;

	CInstanceHdr__GetNearPositionAndRegion(pThis, vPos, &vDoorPos, &pDoorRegion, INTERSECT_BEHAVIOR_IGNORE, INTERSECT_BEHAVIOR_IGNORE);
//	rmonPrintf("thisPos:(%f, %f, %f), desired(%f, %f, %f), out(%f, %f, %f)\n",
//		pThis->m_vPos.x, pThis->m_vPos.y, pThis->m_vPos.z,
//		vPos.x, vPos.y, vPos.z,
//		vDoorPos.x, vDoorPos.y, vDoorPos.z);

	if (AllowEntry)
		CGameRegion__OpenDoor(pDoorRegion);
	else
		CGameRegion__CloseDoor(pDoorRegion);
}


//----------------------------------------------------------------------------
// ACTION
// used by walls that rise on MANTIS BOSS level
void ActionAI_MEvent_Go(CGameObjectInstance *pThis)
{
	AI_SetDesiredAnim(pThis, AI_ANIM_ACTION_GO, TRUE);
	CScene__SetInstanceFlag(&GetApp()->m_Scene, pThis, TRUE);

	// Start sound?
	switch(GetApp()->m_BossLevel)
	{
		case LONGHUNTER_BOSS_LEVEL:

			switch(AI_GetEA(pThis)->m_Id)
			{
				case LONGHUNT_LEVEL_DAM_WALL1_ID:
				case LONGHUNT_LEVEL_DAM_WALL2_ID:
				case LONGHUNT_LEVEL_DAM_WALL3_ID:
				case LONGHUNT_LEVEL_DAM_WALL4_ID:

					// Mantis stone slide sound
					AI_DoSound(&pThis->ah.ih, SOUND_GENERIC_236, 1, 0) ;
					break ;

			}
			break ;

	}
}



//----------------------------------------------------------------------------
// TIMERACTION
// used by thomas for some enviroment particle stuff,
// basically its an action that runs for 'TIME1' goes off for 'TIME2'
void TimerActionAI_MEvent_Go(CGameObjectInstance *pThis)
{
	// if not active
	if (pThis->m_AI.m_dwStatusFlags & AI_STARTED)
	{
		// set flag to say message was received...
		pThis->m_AI.m_InteractiveTimer = 1 ;

		// if not going already, setup the action
		if (!(pThis->m_AI.m_dwStatusFlags & (AI_DOORPREOPEN | AI_DOOROPEN)))
		{
			AI_SetDesiredAnim(pThis, AI_ANIM_TIMERACTION_GO, TRUE);
			if (pThis->ah.ih.m_pEA->m_AttackStyle != -1)
				pThis->m_AI.m_RetreatTimer = SECONDS_TO_FRAMES(pThis->ah.ih.m_pEA->m_AttackStyle) ;
			else
				pThis->m_AI.m_RetreatTimer = pThis->ah.ih.m_pEA->m_AttackStyle ;
			pThis->m_AI.m_dwStatusFlags |= AI_DOORPREOPEN;
		}
	}
}

//----------------------------------------------------------------------------
// PRESSUREACTION
// stays active as long as id is sent out
void PressureActionAI_MEvent_Go(CGameObjectInstance *pThis)
{
	// if not active
	if (pThis->m_AI.m_dwStatusFlags & AI_STARTED)
	{
		// set flag to say message was received...
		pThis->m_AI.m_InteractiveTimer = 2 ;

		// if not already going
		if (!(pThis->m_AI.m_dwStatusFlags & (AI_DOORPREOPEN | AI_DOOROPEN)))
		{
			// setup pretimer, and flag as such
			pThis->m_AI.m_RetreatTimer = SECONDS_TO_FRAMES(pThis->ah.ih.m_pEA->m_AttackStyle) ;
			pThis->m_AI.m_dwStatusFlags |= AI_DOORPREOPEN;
		}
	}
}


//----------------------------------------------------------------------------
// CONSTANTACTION
// always running, used by thomas for some enviroment particle stuff
void ConstantActionAI_MEvent_Go(CGameObjectInstance *pThis)
{
}




//----------------------------------------------------------------------------
// STATUE
// used for MANTIS cacoon
void StatueAI_MEvent_Go(CGameObjectInstance *pThis)
{
	// Has boss been started yet?
	if (GetApp()->m_pBossActive)
	{
		if (MantisBoss.m_Boss.m_Mode == MANTIS_SLEEPING_FLOOR_MODE)
		{
			// Go break out baby!
			Mantis_SetupMode(GetApp()->m_BossVars.m_pMantisInstance, MANTIS_BREAK_FLOOR_MODE) ;

			// Start break open anim
			AI_SetDesiredAnim(pThis, AI_ANIM_STATUE_GO, TRUE);

			GetApp()->m_BossFlags |= BOSS_FLAG_MANTIS_CACOON_OPEN ;
		}
	}
}


//----------------------------------------------------------------------------
// WALLS
// used for Walls on MANTIS BOSS level that fall when hit by boss
void WallAI_MEvent_Left(CGameObjectInstance *pThis)
{
	if (!(pThis->m_AI.m_dwStatusFlags & AI_DOOROPEN))
	{
		pThis->m_AI.m_dwStatusFlags |= AI_DOOROPEN ;
		AI_SetDesiredAnim(pThis, AI_ANIM_WALL_LEFT, TRUE) ;
		CCamera__SetShakeX(&GetApp()->m_Camera, 10) ;
		CCamera__SetShakeY(&GetApp()->m_Camera, 10) ;
		CCamera__SetShakeZ(&GetApp()->m_Camera, 10) ;
		CScene__SetInstanceFlag(&GetApp()->m_Scene, pThis, TRUE) ;
	}
}

void WallAI_MEvent_Right(CGameObjectInstance *pThis)
{
	if (!(pThis->m_AI.m_dwStatusFlags & AI_DOOROPEN))
	{
		pThis->m_AI.m_dwStatusFlags |= AI_DOOROPEN ;
		AI_SetDesiredAnim(pThis, AI_ANIM_WALL_RIGHT, TRUE) ;
		CCamera__SetShakeX(&GetApp()->m_Camera, 10) ;
		CCamera__SetShakeY(&GetApp()->m_Camera, 10) ;
		CCamera__SetShakeZ(&GetApp()->m_Camera, 10) ;
		CScene__SetInstanceFlag(&GetApp()->m_Scene, pThis, TRUE) ;
	}
}


//----------------------------------------------------------------------------
// SPINDOOR
// used by doors in catacombs that rotate 360 degress in 4 stages
//void SpinDoorAI_MEvent_Spin(CGameObjectInstance *pThis)
//{
//	int	position ;
//
//	// if anim completed
//	if (CGameObjectInstance__IsCycleCompleted(pThis))
//	{
//		// goto next anim
//		position = pThis->m_AI.m_RetreatTimer ;
//		position++ ;
//		position &= 7 ;
//
//		pThis->m_AI.m_RetreatTimer = position ;
//
//		switch (position)
//		{
//			case 0:
//				AI_SetDesiredAnim(pThis, AI_ANIM_SPINDOOR_STILL1, TRUE);
//				break ;
//			case 1:
//				AI_SetDesiredAnim(pThis, AI_ANIM_SPINDOOR_ROTATE1, TRUE);
//				break ;
//			case 2:
//				AI_SetDesiredAnim(pThis, AI_ANIM_SPINDOOR_STILL2, TRUE);
//				break ;
//			case 3:
//				AI_SetDesiredAnim(pThis, AI_ANIM_SPINDOOR_ROTATE2, TRUE);
//				break ;
//			case 4:
//				AI_SetDesiredAnim(pThis, AI_ANIM_SPINDOOR_STILL3, TRUE);
//				break ;
//			case 5:
//				AI_SetDesiredAnim(pThis, AI_ANIM_SPINDOOR_ROTATE3, TRUE);
//				break ;
//			case 6:
//				AI_SetDesiredAnim(pThis, AI_ANIM_SPINDOOR_STILL4, TRUE);
//				break ;
//			case 7:
//				AI_SetDesiredAnim(pThis, AI_ANIM_SPINDOOR_ROTATE4, TRUE);
//				break ;
//		}
//	}
//}


//----------------------------------------------------------------------------
// LASERS
// used by laser trap
void LaserAI_MEvent_Start(CGameObjectInstance *pThis)
{
	// if already starting to go, leave it
	if ((pThis->m_AI.m_dwStatusFlags & AI_DOORPREOPEN) == 0)
	{
		// if there is an attackstyle (time before laser starts) use it
		if (pThis->m_AI.m_RetreatTimer = pThis->ah.ih.m_pEA->m_AttackStyle != 0)
		{
			pThis->m_AI.m_dwStatusFlags |= AI_DOORPREOPEN;
			pThis->m_AI.m_RetreatTimer = pThis->ah.ih.m_pEA->m_AttackStyle*FRAME_FPS ;
		}
		else
		{
			pThis->m_AI.m_dwStatusFlags |= AI_DOOROPEN;
			pThis->m_AI.m_RetreatTimer = pThis->ah.ih.m_pEA->m_Aggression *FRAME_FPS ;
			AI_SetDesiredAnim(pThis, AI_ANIM_LASER_GO, TRUE);
		}
	}
}


//----------------------------------------------------------------------------
// EXPLOSIVETARGET
// object that can be hit and blown up, by explosive particles
void ExplosiveTargetAI_MEvent_Go(CGameObjectInstance *pThis)
{
	AI_SetDesiredAnim(pThis, AI_ANIM_EXPTARGET_GO, TRUE);

	CScene__SetInstanceFlag(&GetApp()->m_Scene, pThis, TRUE);
	AI_GetDyn(pThis)->m_dwStatusFlags |= AI_EXPTARGETEXPLODED;
}


//----------------------------------------------------------------------------
// ELEVATOR
// instance that rises, and moves floor collision with it
void ElevatorAI_MEvent_Start(CGameObjectInstance *pThis)
{
	float		time ;

	if (!(pThis->m_AI.m_dwStatusFlags & AI_STARTED))
	{
		// store as being started
		CScene__SetInstanceFlag(&GetApp()->m_Scene, pThis, TRUE);

		pThis->m_AI.m_Time1 = 0 ;
		pThis->m_AI.m_dwStatusFlags |= (AI_STARTED | AI_DOOROPEN) ;
		pThis->m_AI.m_dwStatusFlags &= ~AI_DOORPREOPEN;

		// store original positions
		pThis->m_AI.m_Time2 = pThis->ah.ih.m_vPos.y ;
		pThis->m_AI.m_TeleportTime = CGameRegion__GetGroundHeight(pThis->ah.ih.m_pCurrentRegion, pThis->ah.ih.m_vPos.x, pThis->ah.ih.m_vPos.z) ;

		// start shaking?
		if (pThis->ah.ih.m_pEA->m_dwTypeFlags & AI_TYPE_DEVICESCREENSHAKE)
		{
			// total time of elevator movement
			time = pThis->ah.ih.m_pEA->m_Aggression ;
			CTMove__DoTremorEffectFull(&CTurokMovement, pThis->ah.ih.m_vPos, (9*SCALING_FACTOR)/8, -ANGLE_PI/30, time) ;
		}

		// Start sound?
		switch(GetApp()->m_BossLevel)
		{
			case LONGHUNTER_BOSS_LEVEL:
			case TREX_BOSS_LEVEL:
				// Elevator sound
				AI_DoSound(&pThis->ah.ih, SOUND_GENERIC_237, 1, 0) ;
				break ;
		}
	}
}

//----------------------------------------------------------------------------
// PLATFORM
// instance that rises and falls continously
void PlatformAI_MEvent_Start(CGameObjectInstance *pThis)
{
	if (!(pThis->m_AI.m_dwStatusFlags & AI_STARTED))
	{
		pThis->m_AI.m_Time1 = 0 ;
		pThis->m_AI.m_dwStatusFlags |= (AI_STARTED | AI_DOOROPEN) ;
		pThis->m_AI.m_dwStatusFlags &= ~AI_DOORPREOPEN;

		// store original positions
		pThis->m_AI.m_Time2 = pThis->ah.ih.m_vPos.y ;
		pThis->m_AI.m_TeleportTime = CGameRegion__GetGroundHeight(pThis->ah.ih.m_pCurrentRegion, pThis->ah.ih.m_vPos.x, pThis->ah.ih.m_vPos.z) ;

		CScene__SetInstanceFlag(&GetApp()->m_Scene, pThis, TRUE);
	}
}

//----------------------------------------------------------------------------
// DEATHELEVATOR
// same as elevator, with a death height
void DeathElevatorAI_MEvent_Start(CGameObjectInstance *pThis)
{
	if (!(pThis->m_AI.m_dwStatusFlags & AI_STARTED))
	{
		pThis->m_AI.m_Time1 = 0 ;
		pThis->m_AI.m_dwStatusFlags |= (AI_STARTED | AI_DOOROPEN) ;
		pThis->m_AI.m_dwStatusFlags &= ~AI_DOORPREOPEN;

		// store original positions
		pThis->m_AI.m_Time2 = pThis->ah.ih.m_vPos.y ;
		pThis->m_AI.m_TeleportTime = CGameRegion__GetGroundHeight(pThis->ah.ih.m_pCurrentRegion, pThis->ah.ih.m_vPos.x, pThis->ah.ih.m_vPos.z) ;
	}
}

//----------------------------------------------------------------------------
// COLLAPSINGPLATFORM
// a platform that falls to death
void CollapsingPlatformAI_MEvent_Start(CGameObjectInstance *pThis)
{
	if (!(pThis->m_AI.m_dwStatusFlags & AI_STARTED))
	{
		pThis->m_AI.m_Time1 = 0 ;
		pThis->m_AI.m_dwStatusFlags |= (AI_STARTED | AI_DOORPREOPEN) ;
		pThis->m_AI.m_dwStatusFlags &= ~AI_DOOROPEN;

		// set timer to wait before falling...
		pThis->m_AI.m_RetreatTimer = SECONDS_TO_FRAMES(pThis->ah.ih.m_pEA->m_StartHealth) ;

		// store original positions
		pThis->m_AI.m_Time2 = pThis->ah.ih.m_vPos.y ;
		pThis->m_AI.m_TeleportTime = CGameRegion__GetGroundHeight(pThis->ah.ih.m_pCurrentRegion, pThis->ah.ih.m_vPos.x, pThis->ah.ih.m_vPos.z) ;
	}
}




// TREX ELEVATOR AND PLATFORMS NOW GONE!!
#if 0


//----------------------------------------------------------------------------
// TREXELEVATOR
// instance that falls quickly, and kills player
void TrexElevatorAI_MEvent_Start(CGameObjectInstance *pThis)
{
	if (!(pThis->m_AI.m_dwStatusFlags & AI_STARTED))
	{
		pThis->m_AI.m_Time1 = 0 ;
		pThis->m_AI.m_dwStatusFlags |= (AI_STARTED | AI_DOOROPEN) ;
		pThis->m_AI.m_dwStatusFlags &= ~AI_DOORPREOPEN;

		// store original positions
		pThis->m_AI.m_Time2 = pThis->ah.ih.m_vPos.y ;
		pThis->m_AI.m_TeleportTime = CGameRegion__GetGroundHeight(pThis->ah.ih.m_pCurrentRegion, pThis->ah.ih.m_vPos.x, pThis->ah.ih.m_vPos.z) ;
	}
}


//----------------------------------------------------------------------------
// TREXPLATFORM
// when first triggered, goes to open state
// if you stand on it too long, it goes fall state, and collision moves way down
void TrexPlatformAI_MEvent_Start(CGameObjectInstance *pThis)
{
	if (pThis->m_AI.m_dwStatusFlags & AI_STARTED)
	{
//		osSyncPrintf("Boo!\r\n") ;
		if (!(pThis->m_AI.m_dwStatusFlags & AI_DOOROPEN))
		{
			AI_SetDesiredAnim(pThis, AI_ANIM_TREXPLATFORM_OPEN, TRUE);
			pThis->m_AI.m_dwStatusFlags |= AI_DOOROPEN ;

			// clear 'time stood on' counter
			pThis->m_AI.m_RetreatTimer = 0 ;

			// set time can stand before FALL
			pThis->m_AI.m_Time1 = SECONDS_TO_FRAMES(pThis->ah.ih.m_pEA->m_AttackStyle) ;

			// move to 'GOOD' position
			CGameRegion__SetGroundHeight(pThis->ah.ih.m_pCurrentRegion, pThis->m_AI.m_Time2) ;
		}
	}
}

#endif



#if 0
// Gone now!!

//----------------------------------------------------------------------------
// LONGHUNTER SWITCH
//----------------------------------------------------------------------------
void LonghunterSwitchAI_MEvent_Go(CGameObjectInstance *pThis, FLOAT Value)
{
	CInstanceHdr	*pOrigin = &pThis->ah.ih ;
	INT32				Mask, SwitchFlags, i ;

	// Already started?
	if (!(pThis->m_AI.m_dwStatusFlags & AI_DOOROPEN))
	{
		// Put to open
		pThis->m_AI.m_dwStatusFlags |= AI_DOOROPEN ;
		AI_SetDesiredAnim(pThis, AI_ANIM_LONGHUNTER_SWITCH_GO, TRUE) ;

		// Increase counter
		GetApp()->m_BossVars.m_SwitchesOn++ ;

		// Set individual switch flags (switch id's are 101,102, 103,104, 105,106, 107,108)
//		GetApp()->m_BossVars.m_SwitchFlags |= 1 << (INT32)(Value - 101) ;
//		AI_DoSound(&pThis->ah.ih, SOUND_GENERIC_76 1, 0) ;

		// Set switch flags for all act as one
		if (GetApp()->m_BossVars.m_SwitchFlags != 0xFFFFFFFF)
			AI_DoSound(&pThis->ah.ih, SOUND_GENERIC_76, 1, 0) ;
		GetApp()->m_BossVars.m_SwitchFlags = 0xFFFFFFFF ;

		// This "if" can be taken out if switches are individual
		if (!(GetApp()->m_BossFlags & BOSS_FLAG_LONGHUNTER_SWITCH_PRESSED))
		{
			// Trigger dam walls (id's are 11,12,13,14) and water
			SwitchFlags = GetApp()->m_BossVars.m_SwitchFlags ;
			Mask = 3 ;	// Switches 1 & 2
			for (i = LONGHUNT_LEVEL_DAM_WALL1_ID ; i <= LONGHUNT_LEVEL_DAM_WALL4_ID ; i++)
			{
				// Check for switch groups (1,2), (3,4), (5,6), (7,8)
				if ((SwitchFlags & Mask) == Mask)
				{
					AI_Event_Dispatcher(pOrigin, pOrigin, AI_MEVENT_PRESSUREPLATE,
											  AI_SPECIES_ALL, pOrigin->m_vPos, (FLOAT)i) ;

				}

				// Next switch group
				Mask <<= 2 ;
			}

		}

		// Trigger elevator in middle of arena to rise?
		if (GetApp()->m_BossVars.m_SwitchesOn == 8)
		{
			AI_Event_Dispatcher(pOrigin, pOrigin, AI_MEVENT_PRESSUREPLATE,
									  AI_SPECIES_ALL, pOrigin->m_vPos, LONGHUNT_LEVEL_ELEVATOR_ID) ;
		}

		// Record status in engine
		GetApp()->m_BossFlags |= BOSS_FLAG_LONGHUNTER_SWITCH_PRESSED ;
	}
}
#endif



//----------------------------------------------------------------------------
// PORTAL
// if player has enough keys, opens portal
void PortalAI_MEvent_Start(CGameObjectInstance *pThis)
{
	BOOL	access ;

	access = FALSE ;

	switch (AI_GetEA(pThis)->m_Id)
	{
		case 2:
			if (CTurokMovement.Level2Access >= MAX_KEY2)
				access = TRUE ;
			break ;
		case 3:
			if (CTurokMovement.Level3Access >= MAX_KEY3)
				access = TRUE ;
			break ;
		case 4:
			if (CTurokMovement.Level4Access >= MAX_KEY4)
				access = TRUE ;
			break ;
		case 5:
			if (CTurokMovement.Level5Access >= MAX_KEY5)
				access = TRUE ;
			break ;
		case 6:
			if (CTurokMovement.Level6Access >= MAX_KEY6)
				access = TRUE ;
			break ;
		case 7:
			if (CTurokMovement.Level7Access >= MAX_KEY7)
				access = TRUE ;
			break ;
		case 8:
			if (CTurokMovement.Level8Access >= MAX_KEY8)
				access = TRUE ;
			break ;
	}

	if ((!(pThis->m_AI.m_dwStatusFlags & AI_STARTED)) && (access))
	{
	//	AI_DoSound((CInstanceHdr *)pThis, 572, 1, 0);
		CScene__SetInstanceFlag(&GetApp()->m_Scene, pThis, TRUE);
		pThis->m_AI.m_dwStatusFlags |= AI_STARTED ; ;
		pThis->m_AI.m_Time1 = 0 ;
	}
}


//----------------------------------------------------------------------------
// LOCK
// switch textures if enough keys, also elevator for level8 (last level)
void LockAI_MEvent_Start(CGameObjectInstance *pThis)
{
	int		prev ;

	switch (AI_GetEA(pThis)->m_Id)
	{
		case 2:
			prev = CTurokMovement.Level2Access ;
			CTurokMovement.Level2Access = 0 ;
			if (CTurokMovement.Level2Keys &1)
				CTurokMovement.Level2Access++ ;
			if (CTurokMovement.Level2Keys &2)
				CTurokMovement.Level2Access++ ;
			if (CTurokMovement.Level2Keys &4)
				CTurokMovement.Level2Access++ ;

			if (CTurokMovement.Level2Access != prev)
				AI_DoSound((CInstanceHdr *)pThis, 206, 1, 0);
			break ;

		case 3:
			prev = CTurokMovement.Level3Access ;
			CTurokMovement.Level3Access = 0 ;
			if (CTurokMovement.Level3Keys &1)
				CTurokMovement.Level3Access++ ;
			if (CTurokMovement.Level3Keys &2)
				CTurokMovement.Level3Access++ ;
			if (CTurokMovement.Level3Keys &4)
				CTurokMovement.Level3Access++ ;

			if (CTurokMovement.Level3Access != prev)
				AI_DoSound((CInstanceHdr *)pThis, 206, 1, 0);
			break ;

		case 4:
			prev = CTurokMovement.Level4Access ;
			CTurokMovement.Level4Access = 0 ;
			if (CTurokMovement.Level4Keys &1)
				CTurokMovement.Level4Access++ ;
			if (CTurokMovement.Level4Keys &2)
				CTurokMovement.Level4Access++ ;
			if (CTurokMovement.Level4Keys &4)
				CTurokMovement.Level4Access++ ;

			if (CTurokMovement.Level4Access != prev)
				AI_DoSound((CInstanceHdr *)pThis, 206, 1, 0);
			break ;

		case 5:
			prev = CTurokMovement.Level5Access ;
			CTurokMovement.Level5Access = 0 ;
			if (CTurokMovement.Level5Keys &1)
				CTurokMovement.Level5Access++ ;
			if (CTurokMovement.Level5Keys &2)
				CTurokMovement.Level5Access++ ;
			if (CTurokMovement.Level5Keys &4)
				CTurokMovement.Level5Access++ ;

			if (CTurokMovement.Level5Access != prev)
				AI_DoSound((CInstanceHdr *)pThis, 206, 1, 0);
			break ;

		case 6:
			prev = CTurokMovement.Level6Access ;
			CTurokMovement.Level6Access = 0 ;
			if (CTurokMovement.Level6Keys &1)
				CTurokMovement.Level6Access++ ;
			if (CTurokMovement.Level6Keys &2)
				CTurokMovement.Level6Access++ ;
			if (CTurokMovement.Level6Keys &4)
				CTurokMovement.Level6Access++ ;

			if (CTurokMovement.Level6Access != prev)
				AI_DoSound((CInstanceHdr *)pThis, 206, 1, 0);
			break ;

		case 7:
			prev = CTurokMovement.Level7Access ;
			CTurokMovement.Level7Access = 0 ;
			if (CTurokMovement.Level7Keys &1)
				CTurokMovement.Level7Access++ ;
			if (CTurokMovement.Level7Keys &2)
				CTurokMovement.Level7Access++ ;
			if (CTurokMovement.Level7Keys &4)
				CTurokMovement.Level7Access++ ;

			if (CTurokMovement.Level7Access != prev)
				AI_DoSound((CInstanceHdr *)pThis, 206, 1, 0);
			break ;

		// at least one had to be different!
		case 8:
			prev = CTurokMovement.Level8Access ;
			CTurokMovement.Level8Access = 0 ;
			if (CTurokMovement.Level8Keys &1)
				CTurokMovement.Level8Access++ ;
			if (CTurokMovement.Level8Keys &2)
				CTurokMovement.Level8Access++ ;
			if (CTurokMovement.Level8Keys &4)
				CTurokMovement.Level8Access++ ;
			if (CTurokMovement.Level8Keys &8)
				CTurokMovement.Level8Access++ ;
			if (CTurokMovement.Level8Keys &16)
				CTurokMovement.Level8Access++ ;

			if (CTurokMovement.Level8Access != prev)
				AI_DoSound((CInstanceHdr *)pThis, 206, 1, 0);

			if ((CTurokMovement.Level8Access >= MAX_KEY8) && (!(pThis->m_AI.m_dwStatusFlags & AI_STARTED)))
			{
				pThis->m_AI.m_Time1 = 0 ;
				pThis->m_AI.m_dwStatusFlags |= (AI_STARTED | AI_DOOROPEN) ;
				pThis->m_AI.m_dwStatusFlags &= ~AI_DOORPREOPEN;

				// store original positions
				pThis->m_AI.m_Time2 = pThis->ah.ih.m_vPos.y ;
				pThis->m_AI.m_TeleportTime = CGameRegion__GetGroundHeight(pThis->ah.ih.m_pCurrentRegion, pThis->ah.ih.m_vPos.x, pThis->ah.ih.m_vPos.z) ;

				// keep state for saved game
				CScene__SetInstanceFlag(&GetApp()->m_Scene, pThis, TRUE);
			}
			break ;
	}
}




//----------------------------------------------------------------------------
// DRAIN
// make water 'drain' away!
void DrainAI_MEvent_Start(CGameObjectInstance *pThis)
{
	CROMRegionSet			*pRegionSet;

	if (!(pThis->m_AI.m_dwStatusFlags & AI_STARTED))
	{
		AI_DoSound((CInstanceHdr *)pThis, SOUND_RIVER_FLOW_HEAVY, 1, 0);
//		CScene__DoSoundEffect(&GetApp()->m_Scene,
//									SOUND_RIVER_FLOW_HEAVY, 1,
//								   &pThis->ah.ih, &pThis->ah.ih.m_vPos, 0) ;


		CScene__SetInstanceFlag(&GetApp()->m_Scene, pThis, TRUE);
		pThis->m_AI.m_dwStatusFlags |= AI_STARTED ;
		pThis->m_AI.m_dwStatusFlags &= ~AI_DOOROPEN ;

		pRegionSet = CScene__GetRegionAttributes(&GetApp()->m_Scene, pThis->ah.ih.m_pCurrentRegion);
		if (pRegionSet)
			pThis->ah.ih.m_vPos.y = pRegionSet->m_WaterElevation ;
	}
}

//----------------------------------------------------------------------------
// FLOOD
// make water 'flood' area!
void FloodAI_MEvent_Start(CGameObjectInstance *pThis)
{
	if (!(pThis->m_AI.m_dwStatusFlags & AI_STARTED))
	{
		CScene__SetInstanceFlag(&GetApp()->m_Scene, pThis, TRUE);
		pThis->m_AI.m_dwStatusFlags |= AI_STARTED ;

		CGameRegion__UnMakeInstantDeath(pThis->ah.ih.m_pCurrentRegion) ;
		CGameRegion__ToWater(pThis->ah.ih.m_pCurrentRegion) ;
	}
}


//----------------------------------------------------------------------------
// CRYSTAL
// still on START
// sinks into ground and stays there
// opens up with event from boss dying
void CrystalAI_MEvent_Start(CGameObjectInstance *pThis, BOOL Sink)
{
	CVector3	vPos ;

	if (pThis->m_AI.m_dwStatusFlags & AI_STARTED)
	{
		if (Sink)
		{
			if ((pThis->m_AI.m_dwStatusFlags & AI_DOOROPEN) == 0)
			{
				// Record
				GetApp()->m_BossFlags |= BOSS_FLAG_MANTIS_CRYSTAL_STARTED | BOSS_FLAG_MANTIS_CRYSTAL_DOWN ;

				AI_SetDesiredAnim(pThis, AI_ANIM_CRYSTAL_SINK, TRUE);
				pThis->m_AI.m_dwStatusFlags |= AI_DOOROPEN;
				pThis->m_AI.m_TeleportTime = SECONDS_TO_FRAMES(2) ;
			}
		}
		else
		{
			// Record
			GetApp()->m_BossFlags &= ~BOSS_FLAG_MANTIS_CRYSTAL_DOWN ;

			AI_SetDesiredAnim(pThis, AI_ANIM_CRYSTAL_RISE, TRUE);
			pThis->m_AI.m_Time1 = 1 ;

			if (pThis->m_AI.m_Time2 == 0)
			{
				pThis->m_AI.m_Time2 = 1 ;

				// Generate key level 8c
				CVector3__Set(&vPos,0.613*SCALING_FACTOR,0,-9.0*SCALING_FACTOR) ;
				AI_DoPickup(&pThis->ah.ih, vPos, AI_OBJECT_PICKUP_KEY8MANTIS,0) ;
			}
		}
	}
}

int CGameObjectInstance__GetCrystalModelIndex(CGameObjectInstance *pThis, int nNode)
{
	if ((GetApp()->m_BossFlags & (BOSS_FLAG_MANTIS_CRYSTAL_STARTED | BOSS_FLAG_MANTIS_CRYSTAL_DOWN)) ==
		 BOSS_FLAG_MANTIS_CRYSTAL_STARTED)
		return 1 ;
	else
		return 0 ;
}











//----------------------------------------------------------------------------
// SPINELEVATOR
// instance that rises, and moves floor collision with it, and spins around on Y
void SpinElevatorAI_MEvent_Start(CGameObjectInstance *pThis)
{
	if (!(pThis->m_AI.m_dwStatusFlags & AI_STARTED))
	{
		// store as being started
		CScene__SetInstanceFlag(&GetApp()->m_Scene, pThis, TRUE);

		pThis->m_AI.m_Time1 = 0 ;
		pThis->m_AI.m_dwStatusFlags |= (AI_STARTED | AI_DOOROPEN) ;
		pThis->m_AI.m_dwStatusFlags &= ~AI_DOORPREOPEN;

		// store original positions
		pThis->m_AI.m_Time2 = pThis->ah.ih.m_vPos.y ;
		pThis->m_AI.m_TeleportTime = CGameRegion__GetGroundHeight(pThis->ah.ih.m_pCurrentRegion, pThis->ah.ih.m_vPos.x, pThis->ah.ih.m_vPos.z) ;

		pThis->m_AI.m_ViewAngleOffset = pThis->m_RotY ;
		// start shaking?
		//if (pThis->ah.ih.m_pEA->m_dwTypeFlags & AI_TYPE_DEVICESCREENSHAKE)
	  	//{
		//	// total time of elevator movement
		//	time = pThis->ah.ih.m_pEA->m_Aggression ;
		//	CTMove__DoTremorEffectFull(&CTurokMovement, pThis->ah.ih.m_vPos, (9*SCALING_FACTOR)/8, -ANGLE_PI/30, time) ;
	  	//}
	}
}

//----------------------------------------------------------------------------
// SPINPLATFORM
// instance that rises, and moves floor collision with it, and spins around on Y
void SpinPlatformAI_MEvent_Start(CGameObjectInstance *pThis)
{
	if (!(pThis->m_AI.m_dwStatusFlags & AI_STARTED))
	{
		pThis->m_AI.m_Time1 = 0 ;
		pThis->m_AI.m_dwStatusFlags |= (AI_STARTED | AI_DOOROPEN) ;
		pThis->m_AI.m_dwStatusFlags &= ~AI_DOORPREOPEN;

		// store original positions
		pThis->m_AI.m_Time2 = pThis->ah.ih.m_vPos.y ;
		pThis->m_AI.m_TeleportTime = CGameRegion__GetGroundHeight(pThis->ah.ih.m_pCurrentRegion, pThis->ah.ih.m_vPos.x, pThis->ah.ih.m_vPos.z) ;
	}
}



//---------------------------------------------------------------
// FADEINFADEOUT - Toggle fade
//---------------------------------------------------------------
void FadeInFadeOutAI_MEvent_ToggleState(CGameObjectInstance *pThis)
{
	// Fade object in or out?
	if (!(pThis->m_AI.m_dwStatusFlags & AI_DOOROPEN))
	{
		// Fade object in
		pThis->m_AI.m_dwStatusFlags |= AI_DOOROPEN ;
		CGameObjectInstance__SetNotGone(pThis) ;
		CScene__SetInstanceFlag(&GetApp()->m_Scene, pThis, TRUE) ;
	}
	else
	{
		// Fade object out
		pThis->m_AI.m_dwStatusFlags &= ~AI_DOOROPEN ;
		CGameObjectInstance__SetGone(pThis) ;
		CScene__SetInstanceFlag(&GetApp()->m_Scene, pThis, FALSE) ;

		// Setup fade out mode
		if (pThis->m_Mode != TRANS_FADE_OUT_MODE)
		{
			pThis->m_Mode = TRANS_FADE_OUT_MODE ;
			pThis->m_ModeTime = MAX_OPAQ ;
			pThis->m_ModeMisc1 = (MIN_OPAQ - MAX_OPAQ) / SECONDS_TO_FRAMES(2) ;
		}
	}
}



