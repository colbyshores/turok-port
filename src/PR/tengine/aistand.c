// standard ai file in c
//


#include "stdafx.h"
#ifdef WIN32
#include <math.h>
#endif
#include "ai.h"
#include "aistand.h"

#ifdef WIN32
#include "..\romstruc.h"
#else
#include "romstruc.h"
#include "tengine.h"
#include "tmove.h"
#endif

#ifdef WIN32
#define RANDOM(n) (rand()%n)
#define ANGLE_PI			3.14159265358979323846
#define ANGLE_DTOR(deg)	(deg*(ANGLE_PI/180))
#endif

#define	DEBUG_STAND_AI		0

void AI_PerformTurn(CGameObjectInstance *pMe, FLOAT TurnSpeed);

//#define AI_GetAvoidanceAngle(pMe, vPos, pInst) AI_GetAngle(pMe, vPos)


BOOL IsPlayerAlive()
{
#ifdef WIN32
	return TRUE;
#else
	return (CEngineApp__GetPlayer(GetApp())->m_AI.m_Health > 0);
#endif
}

CVector3 GetPlayerPos()
{
#ifdef WIN32
	return CVector3(0,0,0);
#else
	//return CAnimInstanceHdr__GetPosWithGround(&CEngineApp__GetPlayer(GetApp())->ah);
	return CEngineApp__GetPlayer(GetApp())->ah.ih.m_vPos;
#endif
}



//do ai for standard idle
//
void AI_Standard_Idle(CGameObjectInstance *pMe)
{
	// declare variables
	int	curAnim, pickedType, curAnimTest;
	float	turnang,
			Dist,
			avoidrad;
	int	picked,
			waterFlag;
	BOOL	WalkPresent=FALSE;

	int	typeswalk1[] = {AI_ANIM_IDLE_INTEL1,
								 AI_ANIM_IDLE_INTEL2,
								 AI_ANIM_IDLE,
	 							 AI_ANIM_WALK,
								 AI_ANIM_TURNL90,
								 AI_ANIM_TURNR90,
								 AI_ANIM_TURN180	};

	int	typesrun1[] =  {AI_ANIM_IDLE_INTEL1,
								 AI_ANIM_IDLE_INTEL2,
								 AI_ANIM_IDLE,
	 							 AI_ANIM_RUN,
								 AI_ANIM_TURNL90,
								 AI_ANIM_TURNR90,
								 AI_ANIM_TURN180	};

	int	typeswalk2[] = {AI_ANIM_IDLE_INTEL1,
								 AI_ANIM_IDLE_INTEL2,
								 AI_ANIM_IDLE2,
	 							 AI_ANIM_WALK2,
								 AI_ANIM_TURNL902,
								 AI_ANIM_TURNR902,
								 AI_ANIM_TURN1802	};

	int	typesrun2[] =  {AI_ANIM_IDLE_INTEL1,
								 AI_ANIM_IDLE_INTEL2,
								 AI_ANIM_IDLE2,
	 							 AI_ANIM_RUN2,
								 AI_ANIM_TURNL902,
								 AI_ANIM_TURNR902,
								 AI_ANIM_TURN1802	};

	int	weights[] = { 10,
							  10,
							  5,
							  25,
							  12,
							  14,
							  5	};


#if DEBUG_STAND_AI
	rmonPrintf("AI_Standard_Idle\r\n") ;
#endif


	// get water status of ai
	waterFlag = AI_GetWaterStatus(pMe);

	// get avoidance radius
	avoidrad = AI_GetAvoidanceRadiusFactor(pMe);

	// calculate angle to face to point at the leash start position
	AI_GetDyn(pMe)->m_AvoidanceAngle =
	turnang = AI_GetAvoidanceAngle(pMe, AI_GetDyn(pMe)->m_vLeashCoor, CEngineApp__GetPlayer(GetApp()), avoidrad);


	// does this character have a run animation ?
	if (AI_GetEA(pMe)->m_dwTypeFlags2 & AI_TYPE2_USE2NDSETOFMOVES)
	{
		if (CGameObjectInstance__LookupAIAnimType(pMe, AI_ANIM_WALK2) != -1) WalkPresent=TRUE;
	}
	else
	{
		if (CGameObjectInstance__LookupAIAnimType(pMe, AI_ANIM_WALK) != -1) WalkPresent=TRUE;
	}

	// get distance to leash coor
	Dist = AI_DistanceSquared(pMe, AI_GetDyn(pMe)->m_vLeashCoor);

	// should ai return to leash coordinates or just wander about ?
	if (Dist>=(AI_GetEA(pMe)->m_LeashRadius/3))
	{
		// ai underwater ?
		if (    (waterFlag != AI_BELOW_WATERSURFACE)
			  && (waterFlag != AI_ON_WATERSURFACE) )
		{
			// Signal going back to leash co-ord!
			AI_GetDyn(pMe)->m_dwStatusFlags |= AI_GOBACKTOLEASH ;

			if (AI_GetEA(pMe)->m_dwTypeFlags2 & AI_TYPE2_USE2NDSETOFMOVES)
				curAnimTest = CGameObjectInstance__LookupAIAnimType(pMe, AI_ANIM_RUN2);
			else
				curAnimTest = CGameObjectInstance__LookupAIAnimType(pMe, AI_ANIM_RUN);

			// ai should try & return to leash coor
			if ( Dist <= AI_RUN2WALK_DISTANCE )
			{
				// ai is within walking distance is its prey moving ?
				curAnim = AI_GetCurrentAnim ( pMe );

				if (curAnim == curAnimTest)
					AI_Make_Run(pMe, turnang);
				else
					AI_Make_Walk(pMe, turnang);
			}
			else
			{
				if (curAnimTest != -1)
					AI_Make_Run(pMe, turnang);
				else
					AI_Make_Walk(pMe, turnang);
			}
		}
		else
		{
			// ai is underwater
			AI_Make_Swim(pMe, turnang);
		}
	}
	else
	{
		// Signal not going back to leash now
		AI_GetDyn(pMe)->m_dwStatusFlags &= ~AI_GOBACKTOLEASH ;

		// ai underwater ?
		if (    (waterFlag != AI_BELOW_WATERSURFACE)
			  && (waterFlag != AI_ON_WATERSURFACE) )
		{
			if (AI_GetEA(pMe)->m_dwTypeFlags2 & AI_TYPE2_USE2NDSETOFMOVES)
			{
				if (WalkPresent)
					picked = CGameObjectInstance__GetRandomAnimIndex(pMe, 7, typeswalk2,  weights, &pickedType);
				else
					picked = CGameObjectInstance__GetRandomAnimIndex(pMe, 7, typesrun2, weights, &pickedType);
			}
			else
			{
				if (WalkPresent)
					picked = CGameObjectInstance__GetRandomAnimIndex(pMe, 7, typeswalk1,  weights, &pickedType);
				else
					picked = CGameObjectInstance__GetRandomAnimIndex(pMe, 7, typesrun1, weights, &pickedType);
			}


			AI_GetDyn(pMe)->m_dwStatusFlags |= AI_WAITFOR_CYCLE;

			AI_SetDesiredAnimByIndex(pMe, picked, FALSE);
			switch (pickedType)
			{
				case AI_ANIM_IDLE:
				case AI_ANIM_IDLE2:
				case AI_ANIM_WALK:
				case AI_ANIM_WALK2:
					AI_GetDyn(pMe)->m_dwStatusFlags &= (~AI_WAITFOR_CYCLE);
					break;

				default:
					AI_GetDyn(pMe)->m_dwStatusFlags |= AI_WAITFOR_CYCLE;
					break;
			}
		}
		else
		{
			// ai is underwater
			AI_Make_Swim(pMe, turnang);
		}
	}
}




// do ai for standard agitated mood
//
void AI_Standard_Agitated(CGameObjectInstance *pMe)
{
	// declare variables
	int		picked,
				pickedType;

	int	types1[] = {AI_ANIM_WALK,
							AI_ANIM_WTURNL90,
							AI_ANIM_WTURNR90,
							AI_ANIM_WTURN180,
							AI_ANIM_TURNL90,
							AI_ANIM_TURNR90,
							AI_ANIM_TURN180,
							};

	int	types2[] = {AI_ANIM_WALK2,
							AI_ANIM_WTURNL902,
							AI_ANIM_WTURNR902,
							AI_ANIM_WTURN1802,
							AI_ANIM_TURNL902,
							AI_ANIM_TURNR902,
							AI_ANIM_TURN1802,
							};

	int	weights[] = { 10,
							  9,
							  9,
							  9,
							  5,
							  5,
							  5	};

#if DEBUG_STAND_AI
	rmonPrintf("AI_Standard_Agitated\r\n") ;
#endif

	if (AI_GetEA(pMe)->m_dwTypeFlags2 & AI_TYPE2_USE2NDSETOFMOVES)
		picked = CGameObjectInstance__GetRandomAnimIndex(pMe, 7, types2, weights, &pickedType);
	else
		picked = CGameObjectInstance__GetRandomAnimIndex(pMe, 7, types1, weights, &pickedType);

	AI_GetDyn(pMe)->m_dwStatusFlags |= AI_WAITFOR_CYCLE;
	AI_SetDesiredAnimByIndex(pMe, picked, FALSE);
}



// do ai for standard retreat
//
void AI_Standard_Retreat(CGameObjectInstance *pMe, CGameObjectInstance *pDest)
{
	float						turnang,
								avoidrad;
	CVector3					vPos;
	BOOL						rightHanded;

#if DEBUG_STAND_AI
	rmonPrintf("AI_Standard_Retreat\r\n") ;
#endif


	// set destination position
	vPos = AI_GetPos(pDest);

	// get avoidance radius
	avoidrad = AI_GetAvoidanceRadiusFactor(pMe);

	// do special avoid player for deer & pigs ?
	if (AI_GetEA(pMe)->m_wTypeFlags3 & AI_TYPE3_AVOIDPLAYER2)
	{
		rightHanded = ((((DWORD) pMe) % sizeof(CGameObjectInstance)) & 1) ? 1 : 0;

		if (rightHanded)
		{
			turnang = pDest->m_RotY + ANGLE_PI + (ANGLE_PI/4);
			vPos.x += sin(turnang) * (30*SCALING_FACTOR);
			vPos.z += cos(turnang) * (30*SCALING_FACTOR);
		}
		else
		{
			turnang = pDest->m_RotY + ANGLE_PI - (ANGLE_PI/4);
			vPos.x += sin(turnang) * (30*SCALING_FACTOR);
			vPos.z += cos(turnang) * (30*SCALING_FACTOR);
		}

		AI_GetDyn(pMe)->m_AvoidanceAngle =
		turnang = AI_GetAvoidanceAngle(pMe, vPos, pDest, avoidrad);
	}
	else
	{
		// just do normal avoid player (mainly for fish)
		turnang = pDest->m_RotY + ANGLE_PI;

		vPos.x += sin(turnang)*(60*SCALING_FACTOR);
		vPos.z += cos(turnang)*(60*SCALING_FACTOR);

		// get direction to run
		turnang = AI_GetAngle(pMe, vPos);
	}

	if (!(		(pDest == CEngineApp__GetPlayer(GetApp()))
				&&	(CTMove__IsPlayerUpsideDown(&CTurokMovement))) )
	{
		// player right way up - make ai retreat towards player
		turnang += ANGLE_PI;
		NormalizeRotation(&turnang);
	}

	// ai underwater ?
	switch (AI_GetWaterStatus(pMe))
	{
		case AI_BELOW_WATERSURFACE:
		case AI_ON_WATERSURFACE:
			AI_Make_SwimRetreat(pMe, turnang);
			break;

		default:
			AI_Make_Retreat(pMe, turnang);
			break;
	}
}


// do standard teleport
//
BOOL AI_Standard_Teleport(CGameObjectInstance *pMe)
{
	// random chance of teleporting ?
	if (RANDOM(1000)<15)
	{
		if (!(AI_GetDyn(pMe)->m_dwStatusFlags & AI_TELEPORTING))
		{
#if DEBUG_STAND_AI
	rmonPrintf("AI_Standard_Teleport\r\n") ;
#endif

			return AI_Make_Teleport_Away(pMe);
		}
	}

	return FALSE;
}


// do standard teleport move slow
//
void AI_Standard_TeleportMoveSlow(CGameObjectInstance *pMe)
{
	float						turnang,
								dist,
								avoidrad;
	CGameObjectInstance	*pDest;
	CVector3					vDestPos;

#if DEBUG_STAND_AI
	rmonPrintf("AI_Standard_TeleportMoveSlow\r\n") ;
#endif

	// get avoidance radius
	avoidrad = AI_GetAvoidanceRadiusFactor(pMe);

	// get target
	pDest = pMe->m_pAttackerCGameObjectInstance;
	vDestPos = AI_GetPos(pDest);

	AI_GetDyn(pMe)->m_AvoidanceAngle =
	turnang = AI_GetAvoidanceAngle(pMe, vDestPos, pDest, avoidrad);

	// how close is this ai to destination ?
	dist = AI_DistanceSquared(pMe, vDestPos);

	// make ai go towards target
	AI_Make_TeleportMoveSlow(pMe, turnang);

	// increase teleport time
	AI_GetDyn(pMe)->m_TeleportTime -= frame_increment*(1.0/FRAME_FPS);
	if (    (AI_GetDyn(pMe)->m_TeleportTime < 0)
		  || (dist <= AI_GetEA(pMe)->m_AttackRadius) )
	{
		AI_GetDyn(pMe)->m_dwStatusFlags &= (~AI_TELEPORTAWAY);
		AI_GetDyn(pMe)->m_dwStatusFlags &= (~AI_TELEPORTMOVESLOW);
	}

	// keep agitation up, now that we are fighting (only if ai we are attacking is alive)
	if (IsPlayerAlive())
		AI_Increase_Agitation(pMe, AI_AGITATION_FIGHTING, 300);
}


// do ai for standard attack mode
//
void AI_Standard_Attack(CGameObjectInstance *pMe, CGameObjectInstance *pDest)
{
	// declare variables
	CVector3	vDestPos;
	float		dist,
				turnang,
				projectileang,
				avoidrad;

	int		random;
	int		curAnim,
				curAnimTest,
				waterFlag,
				waterEnemyFlag,
				aiType;

	int		picked,
				pickedType,
				nType;

	BOOL		leaper;

	int	swimattacktypes[]   = {AI_ANIM_ATTACK_SWIM1, AI_ANIM_ATTACK_SWIM2, AI_ANIM_ATTACK_SWIM3, AI_ANIM_IDLE_SWIM};
	int	swimattackweights[] = {40, 40, 40,15};

	int strongtypes[]={AI_ANIM_ATTACK_STRONG1,
							 AI_ANIM_ATTACK_STRONG2,
							 AI_ANIM_ATTACK_STRONG3,
							 AI_ANIM_ATTACK_STRONG4,
							 AI_ANIM_ATTACK_STRONG5,
							 AI_ANIM_ATTACK_STRONG6,
							 AI_ANIM_ATTACK_STRONG7,
							 AI_ANIM_ATTACK_STRONG8,
							 AI_ANIM_ATTACK_STRONG9,
							 AI_ANIM_ATTACK_STRONG10,
							 AI_ANIM_ATTACK_STRONG11,
							 AI_ANIM_ATTACK_STRONG12,
							 AI_ANIM_ATTACK_STRONG13,
							 AI_ANIM_ATTACK_STRONG14};

	int strongweights[]={10,10,10,8,8,8,6,6,6,4,4,4,2,2};

	int weaktypes[]=  {AI_ANIM_ATTACK_WEAK1,
							 AI_ANIM_ATTACK_WEAK2,
							 AI_ANIM_ATTACK_WEAK3,
							 AI_ANIM_ATTACK_WEAK4,
							 AI_ANIM_ATTACK_WEAK5,
							 AI_ANIM_ATTACK_WEAK6,
							 AI_ANIM_ATTACK_WEAK7,
							 AI_ANIM_ATTACK_WEAK8,
							 AI_ANIM_ATTACK_WEAK9,
							 AI_ANIM_ATTACK_WEAK10,
							 AI_ANIM_ATTACK_WEAK11,
							 AI_ANIM_ATTACK_WEAK12,
							 AI_ANIM_ATTACK_WEAK13,
							 AI_ANIM_ATTACK_WEAK14};

	int weakweights[]={10,10,10,8,8,8,6,6,6,4,4,4,2,2};

	int swtypes[]={AI_ANIM_ATTACK_STRONG1,
						AI_ANIM_ATTACK_WEAK1,
						AI_ANIM_ATTACK_STRONG2,
						AI_ANIM_ATTACK_WEAK2,
						AI_ANIM_ATTACK_STRONG3,
						AI_ANIM_ATTACK_WEAK3,
						AI_ANIM_ATTACK_STRONG4,
						AI_ANIM_ATTACK_WEAK4,
						AI_ANIM_ATTACK_STRONG5,
						AI_ANIM_ATTACK_WEAK5,
						AI_ANIM_ATTACK_STRONG6,
						AI_ANIM_ATTACK_WEAK6,
						AI_ANIM_ATTACK_STRONG7,
						AI_ANIM_ATTACK_WEAK7,
						AI_ANIM_ATTACK_STRONG8,
						AI_ANIM_ATTACK_WEAK8,
						AI_ANIM_ATTACK_STRONG9,
						AI_ANIM_ATTACK_WEAK9,
						AI_ANIM_ATTACK_STRONG10,
						AI_ANIM_ATTACK_WEAK10,
						AI_ANIM_ATTACK_STRONG11,
						AI_ANIM_ATTACK_WEAK11,
						AI_ANIM_ATTACK_STRONG12,
						AI_ANIM_ATTACK_WEAK12,
						AI_ANIM_ATTACK_STRONG13,
						AI_ANIM_ATTACK_WEAK13,
						AI_ANIM_ATTACK_STRONG14,
						AI_ANIM_ATTACK_WEAK14};

	int swweights[]={5,5,5,5,5,5,5,5,5,5,5,5,5,5,
						  5,5,5,5,5,5,5,5,5,5,5,5,5,5};

	int	evadetypes[] = {AI_ANIM_EVADE_CROUCH,
								 AI_ANIM_EVADE_LEFT,
								 AI_ANIM_EVADE_RIGHT};
	int	evadeweights[]={8, 10, 10};

#if DEBUG_STAND_AI
	rmonPrintf("AI_Standard_Attack\r\n") ;
#endif


	// get ai water status
	waterFlag = AI_GetWaterStatus(pMe);
	waterEnemyFlag = AI_GetWaterStatus(pDest);

	// is this ai a leaper ?
	// special stuff so leapers can chase player into water aslong as they are not avoiding the water
	aiType = CInstanceHdr__TypeFlag(&pMe->ah.ih);
	leaper = (		(aiType == AI_OBJECT_CHARACTER_LEAPER)
					&& (!(AI_GetEA(pMe)->m_dwTypeFlags & AI_TYPE_AVOIDWATER)) );

	// get avoidance radius
	avoidrad = AI_GetAvoidanceRadiusFactor(pMe);

/*
	// DEBUG stuff
	switch(waterFlag)
	{
		case	AI_NOT_NEAR_WATER:
			if (waterEnemyFlag == AI_NOT_NEAR_WATER)     osSyncPrintf("leaper not near water, player not near water\n");
			if (waterEnemyFlag == AI_BELOW_WATERSURFACE) osSyncPrintf("leaper not near water, player below water surface\n");
			if (waterEnemyFlag == AI_ABOVE_WATERSURFACE) osSyncPrintf("leaper not near water, player above water surface\n");
			if (waterEnemyFlag == AI_ON_WATERSURFACE)    osSyncPrintf("leaper not near water, player on water surface\n");
			break;

		case	AI_BELOW_WATERSURFACE:
			if (waterEnemyFlag == AI_NOT_NEAR_WATER)     osSyncPrintf("leaper below water, player not near water\n");
			if (waterEnemyFlag == AI_BELOW_WATERSURFACE) osSyncPrintf("leaper below water, player below water surface\n");
			if (waterEnemyFlag == AI_ABOVE_WATERSURFACE) osSyncPrintf("leaper below water, player above water surface\n");
			if (waterEnemyFlag == AI_ON_WATERSURFACE)    osSyncPrintf("leaper below water, player on water surface\n");
			break;

		case	AI_ABOVE_WATERSURFACE:
			if (waterEnemyFlag == AI_NOT_NEAR_WATER)     osSyncPrintf("leaper above water, player not near water\n");
			if (waterEnemyFlag == AI_BELOW_WATERSURFACE) osSyncPrintf("leaper above water, player below water surface\n");
			if (waterEnemyFlag == AI_ABOVE_WATERSURFACE) osSyncPrintf("leaper above water, player above water surface\n");
			if (waterEnemyFlag == AI_ON_WATERSURFACE)    osSyncPrintf("leaper above water, player on water surface\n");
			break;

		case	AI_ON_WATERSURFACE:
			if (waterEnemyFlag == AI_NOT_NEAR_WATER)     osSyncPrintf("leaper on water, player not near water\n");
			if (waterEnemyFlag == AI_BELOW_WATERSURFACE) osSyncPrintf("leaper on water, player below water surface\n");
			if (waterEnemyFlag == AI_ABOVE_WATERSURFACE) osSyncPrintf("leaper on water, player above water surface\n");
			if (waterEnemyFlag == AI_ON_WATERSURFACE)    osSyncPrintf("leaper on water, player on water surface\n");
			break;
	}
*/

	// calculate angle to face to point at destination
	nType = CInstanceHdr__TypeFlag(&pMe->ah.ih);
	if ((nType >= AI_OBJECT_WEAPON_START) && (nType <= AI_OBJECT_WEAPON_END))
		nType = AI_OBJECT_CHARACTER_PLAYER;

	switch (nType)
	{
		case AI_OBJECT_CHARACTER_HUMAN1:
		case AI_OBJECT_CHARACTER_RAPTOR:
			vDestPos = AI_GetZigZagPosition(pMe, pDest);
//			vDestPos = AI_GetPos(pDest);
			break;

		default:
		case AI_OBJECT_CHARACTER_PLAYER:
			vDestPos = AI_GetPos(pDest);
			break;
	}

	// ai should not attack a dead turok - they should return to there leash coor
	if (		(pDest == CEngineApp__GetPlayer(GetApp()))
			&&	((pDest->m_AI.m_Health == 0) || (CCamera__InCinemaMode(&GetApp()->m_Camera))) )
	{
		AI_GetDyn(pMe)->m_AvoidanceAngle =
		turnang = AI_GetAvoidanceAngle(pMe, AI_GetDyn(pMe)->m_vLeashCoor, pDest, avoidrad);

		if (		(waterFlag == AI_ON_WATERSURFACE)
				||	(waterFlag == AI_BELOW_WATERSURFACE) )
		{
			AI_Make_Swim(pMe, turnang);
		}
		else
		{
			AI_Make_Run(pMe, turnang);
		}
		return;
	}
	else
	{
		AI_GetDyn(pMe)->m_AvoidanceAngle =
		turnang = AI_GetAvoidanceAngle(pMe, vDestPos, pDest, avoidrad);
	}

	// don't attack if the ai cannot see his target
	if (!CAnimInstanceHdr__CastYeThyRay(&pMe->ah, &pMe->m_pAttackerCGameObjectInstance->ah.ih))
	{
		if (		(waterFlag == AI_ON_WATERSURFACE)
				||	(waterFlag == AI_BELOW_WATERSURFACE) )
		{
			AI_Make_Swim(pMe, turnang);
		}
		else
		{
			AI_Make_Run(pMe, turnang);
		}
		return;
	}

	// how close is this ai to destination ?
	dist = AI_DistanceSquared(pMe, vDestPos);

	// first time entering attack mode ? (gets attention if it is a leader)
	if (    (AI_GetDyn(pMe)->m_dwStatusFlags & AI_FIRST_ATTACK)
		  && (AI_GetEA(pMe)->m_dwTypeFlags & AI_TYPE_LEADER)
		  && (waterFlag != AI_BELOW_WATERSURFACE)
		  && (waterFlag != AI_ON_WATERSURFACE) )
	{
		// first attack
		AI_Make_Get_Attention(pMe);
	}
	// (not first attack) should ai run to meet turok or stand & fight
	else if (dist <= AI_GetEA(pMe)->m_AttackRadius)
	{
		if (AI_Make_Turn(pMe, turnang))
		{
			// should ai fight or evade ?
			if (    (waterFlag == AI_BELOW_WATERSURFACE)
				  || (waterFlag == AI_ON_WATERSURFACE) )
			{
				picked = CGameObjectInstance__GetRandomAnimIndex(pMe, 4, swimattacktypes, swimattackweights, &pickedType);
				AI_GetDyn(pMe)->m_dwStatusFlags |= AI_WAITFOR_CYCLE;
				AI_SetDesiredAnimByIndex(pMe, picked, (AI_GetDyn(pMe)->m_dwStatusFlags & AI_FIRST_ATTACK));
			}
			else
			{
				random = RANDOM(99);

				if ((random < AI_GetEA(pMe)->m_Aggression) || (!AI_InTurokWeaponPath(pMe, ANGLE_DTOR(20))))
				{
					if (    (  AI_GetEA(pMe)->m_dwTypeFlags & AI_TYPE_USEWEAKATTACKS)
						  && (!(AI_GetEA(pMe)->m_dwTypeFlags & AI_TYPE_USESTRONGATTACKS)) )
					{
						// weak attacks
						picked = CGameObjectInstance__GetRandomAnimIndex(pMe, 14, weaktypes, weakweights, &pickedType);
					}
					else if (    (  AI_GetEA(pMe)->m_dwTypeFlags & AI_TYPE_USESTRONGATTACKS)
						       && (!(AI_GetEA(pMe)->m_dwTypeFlags & AI_TYPE_USEWEAKATTACKS)) )
					{
						// strong attacks
						picked = CGameObjectInstance__GetRandomAnimIndex(pMe, 14, strongtypes, strongweights, &pickedType);
					}
					else if (    (AI_GetEA(pMe)->m_dwTypeFlags & AI_TYPE_USESTRONGATTACKS)
						       && (AI_GetEA(pMe)->m_dwTypeFlags & AI_TYPE_USEWEAKATTACKS) )
					{
						// both weak & strong attacks
						picked = CGameObjectInstance__GetRandomAnimIndex(pMe, 28, swtypes, swweights, &pickedType);
					}
					else if (    (!(AI_GetEA(pMe)->m_dwTypeFlags & AI_TYPE_USESTRONGATTACKS))
						       && (!(AI_GetEA(pMe)->m_dwTypeFlags & AI_TYPE_USEWEAKATTACKS)) )
					{
						// neither weak or strong attacks
						AI_Make_Run(pMe, turnang);
					}

					AI_GetDyn(pMe)->m_dwStatusFlags |= AI_WAITFOR_CYCLE;
					AI_SetDesiredAnimByIndex(pMe, picked, (AI_GetDyn(pMe)->m_dwStatusFlags & AI_FIRST_ATTACK));
				}
				else
				{
					picked = CGameObjectInstance__GetRandomAnimIndex(pMe, 3, evadetypes, evadeweights, &pickedType);

					AI_GetDyn(pMe)->m_dwStatusFlags |= AI_WAITFOR_CYCLE;
					AI_SetDesiredAnimByIndex(pMe, picked, FALSE);
				}
			}
		}
	}
	else
	{
		// get projectile angle - is it worth firing ?
		projectileang = AI_GetAngle(pMe, AI_GetPos(pDest));

		// ai is not within close attack range - does ai fire projectiles ?
		if (		(AI_DoesItFireProjectiles(pMe))
				&&	(		(dist >= (AI_GetEA(pMe)->m_ProjectileRadius/2))
						|| (waterEnemyFlag != AI_NOT_NEAR_WATER) )
				&&	(fabs(projectileang) <= ANGLE_DTOR(5))
				&&	(waterFlag != AI_BELOW_WATERSURFACE)
				&& (waterFlag != AI_ON_WATERSURFACE) )

		{
			// is ai within projectile firing radius ?
			if ( dist <= AI_GetEA(pMe)->m_ProjectileRadius )
			{
				AI_Standard_Projectile_Attack(pMe, projectileang, TRUE);
			}
			else
			{
				if (RANDOM(100)<4)
					AI_Standard_Projectile_Attack(pMe, projectileang, TRUE);
				else
				{
					AI_Make_Run(pMe, turnang);
				}
			}
		}
		else
		{
			// ai does not fire projectile weapon - make it move towards turok for attacking
			if (		(AI_DoesItFireProjectiles(pMe))
					&&	(		(fabs(projectileang) <= ANGLE_DTOR(10))
							||	(waterEnemyFlag != AI_NOT_NEAR_WATER) )
					&&	(		(RANDOM(100)<20)
							||	(waterEnemyFlag != AI_NOT_NEAR_WATER) )
					&&	(waterFlag != AI_BELOW_WATERSURFACE)
					&&	(waterFlag != AI_ON_WATERSURFACE) )
			{
				AI_Standard_Projectile_Attack(pMe, projectileang, TRUE);
			}
			else
			{
				if (    (waterFlag == AI_BELOW_WATERSURFACE)
					  || (waterFlag == AI_ON_WATERSURFACE) )
				{
					if (		(CInstanceHdr__IsOnGround(&pMe->ah.ih))
							&&	(waterFlag != AI_BELOW_WATERSURFACE) )
					{
						AI_Make_Run(pMe, turnang);
					}
					else
					{
						AI_Make_Swim(pMe, turnang);
					}
				}
				else
				{
					if (		(dist <= AI_RUN2WALK_DISTANCE)
							&&	(		(waterEnemyFlag == AI_NOT_NEAR_WATER)
									||	(leaper) ) )
					{
						// ai is within walking distance is its prey moving ?
						curAnim = AI_GetCurrentAnim(pMe);

						if (AI_GetEA(pMe)->m_dwTypeFlags2 & AI_TYPE2_USE2NDSETOFMOVES)
							curAnimTest = CGameObjectInstance__LookupAIAnimType(pMe, AI_ANIM_RUN2);
						else
							curAnimTest = CGameObjectInstance__LookupAIAnimType(pMe, AI_ANIM_RUN);

						if (curAnim == curAnimTest)
						{
							AI_Make_Run(pMe, turnang);
						}
						else
						{
							AI_Make_Walk(pMe, turnang);
						}
					}
					else if (		(dist > AI_RUN2WALK_DISTANCE)
									&&	(		(waterEnemyFlag == AI_NOT_NEAR_WATER)
											||	(leaper) ) )
					{
						AI_Make_Run(pMe, turnang);
					}
					else if (		(waterEnemyFlag != AI_NOT_NEAR_WATER)
									&&	(waterFlag == AI_NOT_NEAR_WATER) )
					{
						// enemy is in water & this ai does not have any projectile firing means
						// what shall we do - make it run back to its leash coordinate

						// get avoidance radius
						avoidrad = AI_GetAvoidanceRadiusFactor(pMe);

						// calculate angle to face to point at the leash start position
						AI_GetDyn(pMe)->m_AvoidanceAngle =
						turnang = AI_GetAvoidanceAngle(pMe, AI_GetDyn(pMe)->m_vLeashCoor, CEngineApp__GetPlayer(GetApp()), avoidrad);
						AI_Make_Run(pMe, turnang);
//						AI_Make_Get_Attention(pMe);
					}
				}
			}
		}
	}

	// keep agitation up, now that we are fighting (only if ai we are attacking is alive)
	if (IsPlayerAlive())
		AI_Increase_Agitation(pMe, AI_AGITATION_FIGHTING, 300);

	// ai is no longer in first attack
	AI_GetDyn(pMe)->m_dwStatusFlags &= ~AI_FIRST_ATTACK;
}


// fire projectile for object
//
void AI_Standard_Projectile_Attack(CGameObjectInstance *pMe, float turnang, BOOL MovementAllowed)
{
	BOOL	turned = FALSE;
	int	nAnimIndex;
	int	AnimInfo[10][3]={AI_ANIM_ATTACK_WEAPON_PROJECTILE,  95, 0,
								 AI_ANIM_ATTACK_WEAPON_PROJECTILE2, 75, 0,
								 AI_ANIM_ATTACK_WEAPON_PROJECTILE3, 20, 0,
								 AI_ANIM_ATTACK_WEAPON_PROJECTILE4,  5, 0,
								 AI_ANIM_ATTACK_WEAPON_PROJECTILE5, 95, 0,
								 AI_ANIM_ATTACK_WEAPON_PROJECTILE6, 50, 0,
								 AI_ANIM_ATTACK_WEAPON_PROJECTILE7, 95, 0,
								 AI_ANIM_ATTACK_WEAPON_PROJECTILE8, 50, 0,
								 AI_ANIM_ATTACK_WEAPON_PROJECTILE9, 100, 0,
								 AI_ANIM_ATTACK_WEAPON_PROJECTILE10, 50, 0		};


	// don't attack if the ai cannot see his target
	if (!CAnimInstanceHdr__CastYeThyRay(&pMe->ah, &pMe->m_pAttackerCGameObjectInstance->ah.ih))
	{
		if (MovementAllowed)
		{
			AI_Make_Run(pMe, turnang);
		}
		else
		{
			if (!(AI_GetEA(pMe)->m_dwTypeFlags & AI_TYPE_SNIPER))
				AI_Make_Turn(pMe, turnang);
		}

		return;
	}

	if (AI_GetEA(pMe)->m_dwTypeFlags & AI_TYPE_SNIPER)
		turned = TRUE;
	else
		turned = AI_Make_Turn(pMe, turnang);

	if (turned)
	{

#if DEBUG_STAND_AI
	rmonPrintf("AI_Standard_Projectile_Attack\r\n") ;
#endif

		if (AI_GetEA(pMe)->m_dwTypeFlags & AI_TYPE_PROJECTILEWEAPON1)
			AnimInfo[0][2] = 1;

		if (AI_GetEA(pMe)->m_dwTypeFlags & AI_TYPE_PROJECTILEWEAPON2)
			AnimInfo[1][2] = 1;

		if (AI_GetEA(pMe)->m_dwTypeFlags2 & AI_TYPE2_PROJECTILEWEAPON3)
			AnimInfo[2][2] = 1;

		if (AI_GetEA(pMe)->m_dwTypeFlags2 & AI_TYPE2_PROJECTILEWEAPON4)
			AnimInfo[3][2] = 1;

		if (AI_GetEA(pMe)->m_dwTypeFlags2 & AI_TYPE2_PROJECTILEWEAPON5)
			AnimInfo[4][2] = 1;

		if (AI_GetEA(pMe)->m_dwTypeFlags2 & AI_TYPE2_PROJECTILEWEAPON6)
			AnimInfo[5][2] = 1;

		if (AI_GetEA(pMe)->m_dwTypeFlags2 & AI_TYPE2_PROJECTILEWEAPON7)
			AnimInfo[6][2] = 1;

		if (AI_GetEA(pMe)->m_dwTypeFlags2 & AI_TYPE2_PROJECTILEWEAPON8)
			AnimInfo[7][2] = 1;

		if (AI_GetEA(pMe)->m_wTypeFlags3 & AI_TYPE3_PROJECTILEWEAPON9)
			AnimInfo[8][2] = 1;

		if (AI_GetEA(pMe)->m_wTypeFlags3 & AI_TYPE3_PROJECTILEWEAPON10)
			AnimInfo[9][2] = 1;

		nAnimIndex = CGameObjectInstance__GetRandomAnimIndexWithCheck(pMe, 10, AnimInfo, NULL);
		AI_SetDesiredAnimByIndex(pMe, nAnimIndex, FALSE);
		AI_GetDyn(pMe)->m_dwStatusFlags |= AI_WAITFOR_CYCLE;
	}
}


// do ai for standard flying death
//
void AI_Standard_FlyingDeath(CGameObjectInstance *pMe)
{

	// has flying ai hit the ground yet ?
	if (CInstanceHdr__IsOnGround(&pMe->ah.ih))
	{
		// ai hits the ground
		if (AI_GetDyn(pMe)->m_DeathAnimType != AI_ANIM_DEATH_FLYINGCRASHLAND)
		{
			AI_SetDesiredAnim(pMe, AI_ANIM_DEATH_FLYINGCRASHLAND, TRUE);
			AI_GetDyn(pMe)->m_dwStatusFlags |= AI_WAITFOR_CYCLE;
			AI_GetDyn(pMe)->m_DeathAnimType = AI_ANIM_DEATH_FLYINGCRASHLAND;
		}
	}
	else
	{
		// ai is falling
		AI_SetDesiredAnim(pMe, AI_ANIM_DEATH_FLYINGFALL, TRUE);
		AI_GetDyn(pMe)->m_dwStatusFlags &= ~AI_WAITFOR_CYCLE;
		AI_GetDyn(pMe)->m_DeathAnimType = -1;
	}
}


// do ai for standard death
//
void AI_Standard_Death(CGameObjectInstance *pMe)
{
	int	picked,
			pickedType=-1;

	int	vdeathtypes1[]={AI_ANIM_DEATH_NORMAL,
							   AI_ANIM_DEATH_VIOLENT,
								AI_ANIM_DEATH_VIOLENTLESSCHANCE};

	int	vdeathtypes2[]={AI_ANIM_DEATH_NORMAL2,
							   AI_ANIM_DEATH_VIOLENT2,
								AI_ANIM_DEATH_VIOLENT2LESSCHANCE};

	int	vdeathweights[]={18, 3, 2};		// used to be 16, 5, 2

	int	vwipeouttypes1[]={AI_ANIM_DEATH_WIPEOUT,
								  AI_ANIM_DEATH_NORMAL,
								  AI_ANIM_DEATH_VIOLENT};

	int	vwipeouttypes2[]={AI_ANIM_DEATH_WIPEOUT2,
								  AI_ANIM_DEATH_NORMAL2,
								  AI_ANIM_DEATH_VIOLENT2};

	int	vwipeoutweights[]={8,2,2};		// used to be 8,4,2


	int	deathtypes1[]={AI_ANIM_DEATH_NORMAL};
	int	deathtypes2[]={AI_ANIM_DEATH_NORMAL2};
	int	deathweights[]={2};
	int	wipeouttypes1[]={AI_ANIM_DEATH_WIPEOUT,
								  AI_ANIM_DEATH_NORMAL};
	int	wipeouttypes2[]={AI_ANIM_DEATH_WIPEOUT2,
								  AI_ANIM_DEATH_NORMAL2};
	int	wipeoutweights[]={2};

	int	swimtypes[]={AI_ANIM_DEATH_SWIM};
	int	swimweights[]={1};

#if DEBUG_STAND_AI
	rmonPrintf("AI_Standard_Death\r\n") ;
#endif

	// violent death's ?
	if (AI_GetEA(pMe)->m_wTypeFlags3 & AI_TYPE3_DONTDOVIOLENTDEATH)
	{
		// violent deaths not allowed
		if (AI_GetWaterStatus(pMe)==AI_BELOW_WATERSURFACE)
		{
			picked = CGameObjectInstance__GetRandomAnimIndex(pMe, 1, swimtypes, swimweights, &pickedType);

			AI_GetDyn(pMe)->m_dwStatusFlags |= AI_WAITFOR_CYCLE;
			AI_SetDesiredAnimByIndex(pMe, picked, TRUE);
		}
		else
		{
			if (AI_GetDyn(pMe)->m_dwStatusFlags & AI_RUNNING)
			{
				if (AI_GetEA(pMe)->m_dwTypeFlags2 & AI_TYPE2_USE2NDSETOFMOVES)
					picked = CGameObjectInstance__GetRandomAnimIndex(pMe, 2, wipeouttypes2, wipeoutweights, &pickedType);
				else
					picked = CGameObjectInstance__GetRandomAnimIndex(pMe, 2, wipeouttypes1, wipeoutweights, &pickedType);

				AI_GetDyn(pMe)->m_dwStatusFlags |= AI_WAITFOR_CYCLE;
				AI_SetDesiredAnimByIndex(pMe, picked, TRUE);
			}
			else
			{
				if (AI_GetEA(pMe)->m_dwTypeFlags2 & AI_TYPE2_USE2NDSETOFMOVES)
					picked = CGameObjectInstance__GetRandomAnimIndex(pMe, 1, deathtypes2, deathweights, &pickedType);
				else
					picked = CGameObjectInstance__GetRandomAnimIndex(pMe, 1, deathtypes1, deathweights, &pickedType);

				AI_GetDyn(pMe)->m_dwStatusFlags |= AI_WAITFOR_CYCLE;
				AI_SetDesiredAnimByIndex(pMe, picked, TRUE);
			}
		}

		// set last death animation
		AI_GetDyn(pMe)->m_DeathAnimType = pickedType;
	}
	else
	{
		// violent deaths allowed
		if (AI_GetWaterStatus(pMe)==AI_BELOW_WATERSURFACE)
		{
			picked = CGameObjectInstance__GetRandomAnimIndex(pMe, 1, swimtypes, swimweights, &pickedType);

			AI_GetDyn(pMe)->m_dwStatusFlags |= AI_WAITFOR_CYCLE;
			AI_SetDesiredAnimByIndex(pMe, picked, TRUE);
		}
		else
		{
			if (AI_GetDyn(pMe)->m_dwStatusFlags & AI_RUNNING)
			{
				if (AI_GetEA(pMe)->m_dwTypeFlags2 & AI_TYPE2_USE2NDSETOFMOVES)
					picked = CGameObjectInstance__GetRandomAnimIndex(pMe, 3, vwipeouttypes2, vwipeoutweights, &pickedType);
				else
					picked = CGameObjectInstance__GetRandomAnimIndex(pMe, 3, vwipeouttypes1, vwipeoutweights, &pickedType);

				AI_GetDyn(pMe)->m_dwStatusFlags |= AI_WAITFOR_CYCLE;
				AI_SetDesiredAnimByIndex(pMe, picked, TRUE);
			}
			else
			{
				if (AI_GetEA(pMe)->m_dwTypeFlags2 & AI_TYPE2_USE2NDSETOFMOVES)
					picked = CGameObjectInstance__GetRandomAnimIndex(pMe, 3, vdeathtypes2, vdeathweights, &pickedType);
				else
					picked = CGameObjectInstance__GetRandomAnimIndex(pMe, 3, vdeathtypes1, vdeathweights, &pickedType);

				AI_GetDyn(pMe)->m_dwStatusFlags |= AI_WAITFOR_CYCLE;
				AI_SetDesiredAnimByIndex(pMe, picked, TRUE);
			}
		}

		// set last death animation
		AI_GetDyn(pMe)->m_DeathAnimType = pickedType;
	}
}


// do ai for standard sink
//
void AI_Standard_Sink(CGameObjectInstance *pMe)
{
	CGameRegion		*pRegion;
	float				ghgt;

	int	picked,
			pickedType=-1;

	int	sinktypes[]={AI_ANIM_DEATH_SWIM_SINK};
	int	sinkweights[]={1};

	int	posetypes[]={AI_ANIM_DEATH_SWIM_SINK_POSE};
	int	poseweights[]={1};

#if DEBUG_STAND_AI
	rmonPrintf("AI_Standard_Sink\r\n") ;
#endif

	pRegion = pMe->ah.ih.m_pCurrentRegion;
	if (!pRegion)
		return;

	ghgt = CGameRegion__GetGroundHeight(pRegion, AI_GetPos(pMe).x, AI_GetPos(pMe).z);

	if ( (AI_GetPos(pMe).y - ghgt) < (3*SCALING_FACTOR) )
	{
		// ai is on ground - assume ground sink pose anim
		picked = CGameObjectInstance__GetRandomAnimIndex(pMe, 1, posetypes, poseweights, &pickedType);
		AI_GetDyn(pMe)->m_dwStatusFlags |= AI_WAITFOR_CYCLE;
		AI_SetDesiredAnimByIndex(pMe, picked, TRUE);
	}
	else
	{
		// ai is falling - doing falling sink animation
		picked = CGameObjectInstance__GetRandomAnimIndex(pMe, 1, sinktypes, sinkweights, &pickedType);
		AI_GetDyn(pMe)->m_dwStatusFlags &= (~AI_WAITFOR_CYCLE);
		AI_SetDesiredAnimByIndex(pMe, picked, TRUE);
	}
}


// do ai for standard float
//
void AI_Standard_Float(CGameObjectInstance *pMe)
{
	CGameRegion		*pRegion;

	int	picked,
			pickedType=-1,
			waterFlag;

	int	floattypes[]={AI_ANIM_DEATH_SWIM_FLOAT_UP};
	int	floatweights[]={1};

	int	bobtypes[]={AI_ANIM_DEATH_SWIM_SURFACE_BOB};
	int	bobweights[]={1};

#if DEBUG_STAND_AI
	rmonPrintf("AI_Standard_Float\r\n") ;
#endif

	pRegion = pMe->ah.ih.m_pCurrentRegion;
	if (!pRegion)
		return;

	waterFlag = AI_GetWaterStatus(pMe);

	switch(waterFlag)
	{
		case AI_BELOW_WATERSURFACE:
			picked = CGameObjectInstance__GetRandomAnimIndex(pMe, 1, floattypes, floatweights, &pickedType);
			AI_GetDyn(pMe)->m_dwStatusFlags &= (~AI_WAITFOR_CYCLE);
			AI_SetDesiredAnimByIndex(pMe, picked, TRUE);
			break;

		default:
		case AI_ON_WATERSURFACE:
			picked = CGameObjectInstance__GetRandomAnimIndex(pMe, 1, bobtypes, bobweights, &pickedType);
			AI_GetDyn(pMe)->m_dwStatusFlags |= AI_WAITFOR_CYCLE;
			AI_SetDesiredAnimByIndex(pMe, picked, TRUE);
			break;
	}
}


// do ai for standard hit
//
void AI_Standard_Hit(CGameObjectInstance *pMe)
{
#if DEBUG_STAND_AI
	rmonPrintf("AI_Standard_Hit\r\n") ;
#endif

	// hit is registered
	AI_GetDyn(pMe)->m_dwStatusFlags &= ~AI_EV_HIT;
	AI_Increase_Agitation ( pMe, AI_AGITATION_HIT, 300 );
}




// do ai for standard explosion
//
void AI_Standard_Explosion(CGameObjectInstance *pMe)
{
	int	picked,
			pickedType = -1;

	int	exptypes[]={AI_ANIM_IMPACT_EXPLODE_LEFT,
							AI_ANIM_IMPACT_EXPLODE_RIGHT,
							AI_ANIM_IMPACT_EXPLODE_BACKWARD,
							AI_ANIM_IMPACT_EXPLODE_FORWARD};
	int	expweights[]={10,10,10,10};

	int	dexptypes[]={AI_ANIM_IMPACT_EXPLODE_DEAD1,
		                AI_ANIM_IMPACT_EXPLODE_DEAD2};

	int	dexpweights[]={10,10};

	int	swimtypes[]={AI_ANIM_DEATH_SWIM};
	int	swimweights[]={1};


#if DEBUG_STAND_AI
	rmonPrintf("AI_Standard_Explosion\r\n") ;
#endif

	// get this ai
	AI_GetDyn(pMe)->m_dwStatusFlags &= ~AI_EV_EXPLOSION;
	AI_Increase_Agitation ( pMe, AI_AGITATION_EXPLOSION, 300 );

	// set velocity of character
	if (!(AI_GetDyn(pMe)->m_dwStatusFlags&AI_NOREPEATEXP))
	{
		if (AI_GetWaterStatus(pMe) == AI_BELOW_WATERSURFACE)
		{
			AI_GetVelocity(pMe)->x = (AI_GetDyn(pMe)->m_vExplosion.x/4);
			AI_GetVelocity(pMe)->y = (AI_GetDyn(pMe)->m_vExplosion.y/4);
			AI_GetVelocity(pMe)->z = (AI_GetDyn(pMe)->m_vExplosion.z/4);
		}
		else
		{
			AI_GetVelocity(pMe)->x = AI_GetDyn(pMe)->m_vExplosion.x;

			if (AI_GetDyn(pMe)->m_vExplosion.y)
				AI_GetVelocity(pMe)->y = max(5*SCALING_FACTOR*15, AI_GetDyn(pMe)->m_vExplosion.y);
			else
				AI_GetVelocity(pMe)->y = 0.0;

			AI_GetVelocity(pMe)->z = AI_GetDyn(pMe)->m_vExplosion.z;
		}
	}

	if (AI_GetWaterStatus(pMe) == AI_BELOW_WATERSURFACE)
	{
		picked = CGameObjectInstance__GetRandomAnimIndex(pMe, 1, swimtypes, swimweights, &pickedType);
		AI_GetDyn(pMe)->m_dwStatusFlags |= AI_WAITFOR_CYCLE;
		AI_SetDesiredAnimByIndex(pMe, picked, TRUE);
		CGameObjectInstance__RestartAnim(pMe);
	}
	else
	{
		if ( ((!(AI_GetEA(pMe)->m_dwTypeFlags&AI_TYPE_EXPLOSIONDEATH)) && AI_GetDyn(pMe)->m_Health > 0) ||
				  (AI_GetEA(pMe)->m_dwTypeFlags&AI_TYPE_EXPLOSIONDEATH)													)
		{
			if (AI_GetEA(pMe)->m_dwTypeFlags&AI_TYPE_DIEONEXPLOSION)
				AI_Decrease_Health(pMe, 999999, FALSE, TRUE);

			picked = CGameObjectInstance__GetRandomAnimIndex(pMe, 4, exptypes, expweights, &pickedType);
			AI_GetDyn(pMe)->m_dwStatusFlags |= AI_WAITFOR_CYCLE;
			AI_SetDesiredAnimByIndex(pMe, picked, TRUE);
			CGameObjectInstance__RestartAnim(pMe);
		}

		else if ( (!(AI_GetEA(pMe)->m_dwTypeFlags&AI_TYPE_EXPLOSIONDEATH)) &&
						 AI_GetDyn(pMe)->m_Health <= 0                         &&
					 (!(AI_GetDyn(pMe)->m_dwStatusFlags&AI_NOREPEATEXP))         )
		{
			// do death explosions
			picked = CGameObjectInstance__GetRandomAnimIndex(pMe, 2, dexptypes, dexpweights, &pickedType);

			// if there are no explode dead anims & no repeat explosion is on just do a normal death
			// instead of an explode dead 1
			if (		(picked == -1)
					&&	(AI_GetEA(pMe)->m_dwTypeFlags & AI_TYPE_NOREPEATEXPLOSION) )
			{
				AI_Standard_Death(pMe);
				return;
			}

			AI_GetDyn(pMe)->m_dwStatusFlags |= AI_WAITFOR_CYCLE;
			AI_SetDesiredAnimByIndex(pMe, picked, TRUE);
			CGameObjectInstance__RestartAnim(pMe);
			if (AI_GetEA(pMe)->m_dwTypeFlags&AI_TYPE_NOREPEATEXPLOSION)
				AI_GetDyn(pMe)->m_dwStatusFlags|=AI_NOREPEATEXP;
		}
	}

	// set last death animation
	AI_GetDyn(pMe)->m_DeathAnimType = pickedType;
}


// do proper landing animation when an exploded character hits the ground
//
void AI_Standard_ExplodedLand(CGameObjectInstance *pMe, int waterFlag)
{
	int	nType;


	// ai in water ?
	if (waterFlag == AI_NOT_NEAR_WATER)
	{
		// only do hit ground animation when ai hits the ground
		if (!CInstanceHdr__IsOnGround(&pMe->ah.ih))
			return;
	}
	else
	{
		// wait till ai hits the water surface
		if (waterFlag == AI_ABOVE_WATERSURFACE)
			return;
	}

	// what is the last death animation ?
	switch(AI_GetDyn(pMe)->m_DeathAnimType)
	{
		case AI_ANIM_IMPACT_EXPLODE_LEFT:
			nType = AI_ANIM_IMPACT_EXPLODE_LEFT_LAND;
			break;

		case AI_ANIM_IMPACT_EXPLODE_RIGHT:
			nType = AI_ANIM_IMPACT_EXPLODE_RIGHT_LAND;
			break;

		case AI_ANIM_IMPACT_EXPLODE_BACKWARD:
			nType = AI_ANIM_IMPACT_EXPLODE_BACKWARD_LAND;
			break;

		case AI_ANIM_IMPACT_EXPLODE_FORWARD:
			nType = AI_ANIM_IMPACT_EXPLODE_FORWARD_LAND;
			break;

		default:
			nType = -1;
			break;
	}


	// set the exploded land anim
	if (nType != -1)
	{
		AI_GetDyn(pMe)->m_DeathAnimType = nType;
		AI_SetDesiredAnim(pMe, nType, TRUE);
		AI_GetDyn(pMe)->m_dwStatusFlags |= AI_WAITFOR_CYCLE;

		AI_GetVelocity(pMe)->x = 0;
		AI_GetVelocity(pMe)->y = 0;
		AI_GetVelocity(pMe)->z = 0;
	}

}



/*
// do ai for standard evade
// ai will turn away from any object it collides with then walk a short distance
//
void AI_Standard_Evade ( CGameObjectInstance *pMe )
{
	// declare variables
	float		ang;


	// which evade mode are we in ?
	switch (AI_GetDyn(pMe)->m_Evade)
	{
		case AI_EVADE_START:
			TRACE0("AI: Evade Start\r\n");
			// ai should turn 110 degrees from the direction the collision occured at
			// calculate angle to face to point at turok
//			if (RANDOM(100)<50)
//				pMe->m_EvadeAngle = pMe->m_RotY+ANGLE_DTOR(-110);
//			else
//				pMe->m_EvadeAngle = pMe->m_RotY+ANGLE_DTOR(+110);

			// calculate angle to face to point at turok
			ang = AI_GetAngle(pMe, GetPlayerPos());
			if (ang>0)
				pMe->m_EvadeAngle = ang-ANGLE_DTOR(+110);
			else
				pMe->m_EvadeAngle = ang-ANGLE_DTOR(-110);

			// ai now starts the evade turn
			AI_GetDyn(pMe)->m_Evade=AI_EVADE_TURN;
			break;

		case AI_EVADE_TURN:
			TRACE0("AI: Evade Turn\r\n");
			// ai is performing the turn
			ang = pMe->m_EvadeAngle - pMe->m_RotY;
			if ( ang < -ANGLE_PI )
				ang += (2 * ANGLE_PI);
			else if ( ang > ANGLE_PI )
				ang -= (2 * ANGLE_PI);

			if (AI_Make_Turn ( pMe, ang ))
			{
				// ai should walk this new direction for a while to escape the object they collided with
				AI_GetDyn(pMe)->m_Evade=AI_EVADE_MOVE;
				pMe->m_EvadeAngle = RANDOM(5)+15;
			}
			break;

		case AI_EVADE_MOVE:
			TRACE0("AI: Evade Move\r\n");
			if (pMe->m_EvadeAngle-- <0)
			{
				// ai should now face towards turok
				AI_GetDyn(pMe)->m_Evade=AI_EVADE_FACETUROK;
			}
			else
			{
				// ai is performing walk
				AI_SetDesiredAnim(pMe, AI_ANIM_RUN, FALSE);
				AI_GetDyn(pMe)->m_dwStatusFlags &= (~AI_WAITFOR_CYCLE);
			}
			break;

		case AI_EVADE_FACETUROK:
			// calculate angle to face to point at turok
			ang = AI_GetAngle(pMe, GetPlayerPos());
			if (AI_Make_Run(pMe, ang))
			{
				// ai should now go back to normal ai cycle & chase turok
				AI_GetDyn(pMe)->m_Evade=AI_EVADE_NONE;
			}
			break;
	}

}
*/



// do ai for standard follow mode
//
void AI_Standard_Follow(CGameObjectInstance *pMe, CGameObjectInstance *pLeader)
{
	// declare variables
	CVector3	vLeaderPos;
	float		dist,
				turnang,
				avoidrad;
	int		curAnim,
				curAnimTest;


#if DEBUG_STAND_AI
	rmonPrintf("AI_Standard_Follow\r\n") ;
#endif

	// get avoidance radius
	avoidrad = AI_GetAvoidanceRadiusFactor(pMe);

	// calculate angle to face to point at the leader
	vLeaderPos = AI_GetPos(pLeader);

	AI_GetDyn(pMe)->m_AvoidanceAngle =
	turnang = AI_GetAvoidanceAngle(pMe, vLeaderPos, pLeader, avoidrad);

	// how close is this ai to the leader ?
	dist = AI_DistanceSquared(pMe, vLeaderPos);

	// ai should follow the leader
	if (dist <= AI_GetEA(pMe)->m_AttackRadius)
	{
		if (AI_Make_Turn(pMe, turnang))
		{
			// ai has reached & is facing the leader
			if (RANDOM(100)<5)
			{
				AI_Make_Get_Attention(pMe);
			}
			else
			{
				if (AI_GetEA(pMe)->m_dwTypeFlags2 & AI_TYPE2_USE2NDSETOFMOVES)
					AI_SetDesiredAnim(pMe, AI_ANIM_IDLE2, FALSE);
				else
					AI_SetDesiredAnim(pMe, AI_ANIM_IDLE, FALSE);

				AI_GetDyn(pMe)->m_dwStatusFlags &= (~AI_WAITFOR_CYCLE);
			}
		}
	}
	else
	{
		if (dist <= AI_RUN2WALK_DISTANCE)
		{
			// ai is within walking distance of leader ?
			curAnim = AI_GetCurrentAnim(pMe);

			if (AI_GetEA(pMe)->m_dwTypeFlags2 & AI_TYPE2_USE2NDSETOFMOVES)
				curAnimTest = CGameObjectInstance__LookupAIAnimType(pMe, AI_ANIM_RUN2);
			else
				curAnimTest = CGameObjectInstance__LookupAIAnimType(pMe, AI_ANIM_RUN);

			if (curAnim == curAnimTest)
			{
				AI_Make_Run(pMe, turnang);
			}
			else
			{
				AI_Make_Walk(pMe, turnang);
			}
		}
		else
		{
			// make ai run towards leader
			AI_Make_Run(pMe, turnang);
		}
	}

	// keep agitation up, now that we are fighting (only if ai we are attacking is alive)
	if (IsPlayerAlive())
		AI_Increase_Agitation(pMe, AI_AGITATION_FIGHTING, 300);
}




void AI_Standard_DeadBodyRiddle(CGameObjectInstance *pMe)
{
	int	picked=-1;
	int	pickedType=-1;

	int	normaltypes[]={AI_ANIM_TWITCH_NORMAL1,
								AI_ANIM_TWITCH_NORMAL2,
								AI_ANIM_DEATH_NORMAL};
	int	normalweights[]={100000,100000,1};

	int	violenttypes[]={AI_ANIM_TWITCH_VIOLENT1,
								AI_ANIM_TWITCH_VIOLENT2,
								AI_ANIM_DEATH_VIOLENT};
	int	violentweights[]={100000,100000,1};

	int	highfalltypes[]={AI_ANIM_TWITCH_HIGHFALL1,
								AI_ANIM_TWITCH_HIGHFALL2,
								AI_ANIM_DEATH_HIGH_FALL};
	int	highfallweights[]={100000,100000,1};

	int	wipeouttypes[]={AI_ANIM_TWITCH_WIPEOUT1,
								AI_ANIM_TWITCH_WIPEOUT2,
								AI_ANIM_DEATH_WIPEOUT};
	int	wipeoutweights[]={100000,100000,1};

	int	expdead1types[]={AI_ANIM_TWITCH_EXPDEAD1_1,
								AI_ANIM_TWITCH_EXPDEAD1_2,
								AI_ANIM_IMPACT_EXPLODE_DEAD1};
	int	expdead1weights[]={100000,100000,1};

	int	expdead2types[]={AI_ANIM_TWITCH_EXPDEAD2_1,
								AI_ANIM_TWITCH_EXPDEAD2_2,
								AI_ANIM_IMPACT_EXPLODE_DEAD2};
	int	expdead2weights[]={100000,100000,1};

	int	explefttypes[]={AI_ANIM_TWITCH_EXPLEFT1,
								AI_ANIM_TWITCH_EXPLEFT2,
								AI_ANIM_IMPACT_EXPLODE_LEFT};
	int	expleftweights[]={100000,100000,1};

	int	exprighttypes[]={AI_ANIM_TWITCH_EXPRIGHT1,
								AI_ANIM_TWITCH_EXPRIGHT2,
								AI_ANIM_IMPACT_EXPLODE_RIGHT};
	int	exprightweights[]={100000,100000,1};

	int	expbacktypes[]={AI_ANIM_TWITCH_EXPBACK1,
								AI_ANIM_TWITCH_EXPBACK2,
								AI_ANIM_IMPACT_EXPLODE_BACKWARD};
	int	expbackweights[]={100000,100000,1};

	int	expfortypes[]={AI_ANIM_TWITCH_EXPFOR1,
								AI_ANIM_TWITCH_EXPFOR2,
								AI_ANIM_IMPACT_EXPLODE_FORWARD};
	int	expforweights[]={100000,100000,1};


#if DEBUG_STAND_AI
	rmonPrintf("AI_Standard_DeadBodyRiddle\r\n") ;
#endif

	// what is the last death animation ?
	switch(AI_GetDyn(pMe)->m_DeathAnimType)
	{
		case AI_ANIM_DEATH_NORMAL:
		case AI_ANIM_DEATH_NORMAL2:
		case AI_ANIM_TWITCH_NORMAL1:
		case AI_ANIM_TWITCH_NORMAL2:
			picked = CGameObjectInstance__GetRandomAnimIndex(pMe, 3, normaltypes, normalweights, &pickedType);
			break;

		case AI_ANIM_DEATH_VIOLENT:
		case AI_ANIM_DEATH_VIOLENT2:
		case AI_ANIM_DEATH_VIOLENTLESSCHANCE:
		case AI_ANIM_DEATH_VIOLENT2LESSCHANCE:
		case AI_ANIM_TWITCH_VIOLENT1:
		case AI_ANIM_TWITCH_VIOLENT2:
			picked = CGameObjectInstance__GetRandomAnimIndex(pMe, 3, violenttypes, violentweights, &pickedType);
			break;

		case AI_ANIM_DEATH_WIPEOUT:
		case AI_ANIM_DEATH_WIPEOUT2:
		case AI_ANIM_TWITCH_WIPEOUT1:
		case AI_ANIM_TWITCH_WIPEOUT2:
			picked = CGameObjectInstance__GetRandomAnimIndex(pMe, 3, wipeouttypes, wipeoutweights, &pickedType);
			break;

		case AI_ANIM_DEATH_HIGH_FALL:
		case AI_ANIM_TWITCH_HIGHFALL1:
		case AI_ANIM_TWITCH_HIGHFALL2:
			picked = CGameObjectInstance__GetRandomAnimIndex(pMe, 3, highfalltypes, highfallweights, &pickedType);
			break;

		case AI_ANIM_IMPACT_EXPLODE_DEAD1:
		case AI_ANIM_TWITCH_EXPDEAD1_1:
		case AI_ANIM_TWITCH_EXPDEAD1_2:
			picked = CGameObjectInstance__GetRandomAnimIndex(pMe, 3, expdead1types, expdead1weights, &pickedType);
			break;

		case AI_ANIM_IMPACT_EXPLODE_DEAD2:
		case AI_ANIM_TWITCH_EXPDEAD2_1:
		case AI_ANIM_TWITCH_EXPDEAD2_2:
			picked = CGameObjectInstance__GetRandomAnimIndex(pMe, 3, expdead2types, expdead2weights, &pickedType);
			break;

		case AI_ANIM_IMPACT_EXPLODE_LEFT_LAND:
		case AI_ANIM_TWITCH_EXPLEFT1:
		case AI_ANIM_TWITCH_EXPLEFT2:
			picked = CGameObjectInstance__GetRandomAnimIndex(pMe, 3, explefttypes, expleftweights, &pickedType);
			break;

		case AI_ANIM_IMPACT_EXPLODE_RIGHT_LAND:
		case AI_ANIM_TWITCH_EXPRIGHT1:
		case AI_ANIM_TWITCH_EXPRIGHT2:
			picked = CGameObjectInstance__GetRandomAnimIndex(pMe, 3, exprighttypes, exprightweights, &pickedType);
			break;

		case AI_ANIM_IMPACT_EXPLODE_BACKWARD_LAND:
		case AI_ANIM_TWITCH_EXPBACK1:
		case AI_ANIM_TWITCH_EXPBACK2:
			picked = CGameObjectInstance__GetRandomAnimIndex(pMe, 3, expbacktypes, expbackweights, &pickedType);
			break;

		case AI_ANIM_IMPACT_EXPLODE_FORWARD_LAND:
		case AI_ANIM_TWITCH_EXPFOR1:
		case AI_ANIM_TWITCH_EXPFOR2:
			picked = CGameObjectInstance__GetRandomAnimIndex(pMe, 3, expfortypes, expforweights, &pickedType);
			break;

		default:
		case -1:
			break;
	}

	// was valid anim picked ?
	if (picked != -1)
	{
		// was the picked animation a repeatable twitch ?
		switch(pickedType)
		{
			// play chosen twitch animation
			default:
				AI_GetDyn(pMe)->m_dwStatusFlags |= AI_WAITFOR_CYCLE;
				AI_SetDesiredAnimByIndex(pMe, picked, TRUE);
				CGameObjectInstance__RestartAnim(pMe);
				break;

			// should not do any animation for these anims - not proper twitch animations
			case -1:
			case AI_ANIM_IMPACT_EXPLODE_BACKWARD:
			case AI_ANIM_IMPACT_EXPLODE_FORWARD:
			case AI_ANIM_IMPACT_EXPLODE_LEFT:
			case AI_ANIM_IMPACT_EXPLODE_RIGHT:
			case AI_ANIM_IMPACT_EXPLODE_DEAD1:
			case AI_ANIM_IMPACT_EXPLODE_DEAD2:
			case AI_ANIM_DEATH_NORMAL:
			case AI_ANIM_DEATH_NORMAL2:
			case AI_ANIM_DEATH_HIGH_FALL:
			case AI_ANIM_DEATH_VIOLENT:
			case AI_ANIM_DEATH_VIOLENT2:
			case AI_ANIM_DEATH_WIPEOUT:
			case AI_ANIM_DEATH_WIPEOUT2:
				break;
		}
	}

	// set last death animation
	AI_GetDyn(pMe)->m_DeathAnimType = pickedType;
}



void AI_Standard_Sniper(CGameObjectInstance *pMe, CGameObjectInstance *pDest)
{
	float		projectileang,
				dist;
//				turnang;
	int		random;
	CVector3	vDestPos;


#if DEBUG_STAND_AI
	rmonPrintf("AI_Standard_Sniper\r\n") ;
#endif


	// get projectile angle - is it worth firing ?
	vDestPos = AI_GetPos(pDest);
//	projectileang = AI_GetAngle(pMe, vDestPos);
	dist = AI_DistanceSquared(pMe, vDestPos);
//	turnang = AI_GetAngle(pMe, vDestPos);
	random = RANDOM(99);

//	AI_GetDyn(pMe)->m_AvoidanceAngle = projectileang;
//	AI_PerformTurn(pMe, ANGLE_DTOR(60));
	projectileang = AI_GetAngle(pMe, vDestPos);

	// ai is not within close attack range - does ai fire projectiles ?
	if (	  (AI_DoesItFireProjectiles(pMe))
//		  && (fabs(projectileang) <= ANGLE_DTOR(7))		//5
		  && (dist <= AI_GetEA(pMe)->m_ProjectileRadius)
		  && (random < AI_GetEA(pMe)->m_Aggression) )
	{
		// make sniper less likely to fire at a greater distance
		dist = sqrt(dist) * 100 / max(1, sqrt(AI_GetEA(pMe)->m_ProjectileRadius));
		if (RANDOM(100) >= dist)
		{
			AI_Standard_Projectile_Attack(pMe, projectileang, FALSE);
//			AI_GetDyn(pMe)->m_dwStatusFlags |= AI_WAITFOR_CYCLE;
		}
		else
		{
			AI_SetDesiredAnim(pMe, AI_ANIM_IDLE, FALSE);
			AI_GetDyn(pMe)->m_dwStatusFlags &= ~AI_WAITFOR_CYCLE;
		}
	}
	else
	{
		AI_SetDesiredAnim(pMe, AI_ANIM_IDLE, FALSE);
		AI_GetDyn(pMe)->m_dwStatusFlags &= ~AI_WAITFOR_CYCLE;
	}
}




// ai heading for wall ?
//
BOOL AI_Standard_HeadingForWall(CGameObjectInstance *pAI, float radiusFactor)
{
//	CGameRegion		*pRegion;
	CVector3			vCurrentPos,
						vPos;


//	pRegion = pAI->ah.ih.m_pCurrentRegion;
	vCurrentPos = pAI->ah.ih.m_vPos;
	vPos.x = pAI->ah.ih.m_vPos.x + sin(pAI->m_RotY + ANGLE_PI)*(radiusFactor*pAI->ah.ih.m_pEA->m_CollisionRadius);
	vPos.z = pAI->ah.ih.m_vPos.z + cos(pAI->m_RotY + ANGLE_PI)*(radiusFactor*pAI->ah.ih.m_pEA->m_CollisionRadius);
	vPos.y = vCurrentPos.y;


	// check to see if ai is heading for a wall
	if (CAnimInstanceHdr__IsObstructed(&pAI->ah, vPos, NULL))
	{
		//osSyncPrintf("heading for wall\n");
		return TRUE;
	}
	else
	{
		//osSyncPrintf("ok\n");
		return FALSE;
	}
}


void AI_Standard_Keep_It_Flying(CGameObjectInstance *pMe, CGameObjectInstance *pDest)
{
	CVector3		vDestPos,
					vFlyPos;
	float			dm,
					turnang,
					avoidrad;
	int			waterFlag;

	// get avoidance radius
	avoidrad = AI_GetAvoidanceRadiusFactor(pMe);

	// keep the ai up in the air if it is alive
	if (    (AI_GetEA(pMe)->m_dwTypeFlags & AI_TYPE_FLYING)
		  && (AI_GetDyn(pMe)->m_Health > 0) )
	{

#if DEBUG_STAND_AI
	rmonPrintf("AI_Standard_Keep_It_Flying\r\n") ;
#endif

		// how far is flying thing from the ai it is attacking
		vDestPos = AI_GetPos(pDest);
		vFlyPos = AI_GetPos(pMe);

		// which height should this ai fly at ?
		waterFlag = AI_GetWaterStatus(pDest);
		switch(waterFlag)
		{
			case AI_NOT_NEAR_WATER:
			case AI_ON_WATERSURFACE:
			case AI_ABOVE_WATERSURFACE:
				// fly above destination target
				dm = (AI_GetEA(pMe)->m_FlyingHeight - (vFlyPos.y - vDestPos.y)) / 12;
				break;

			default:
				// fly 15 feet above water level
				dm = ((AI_GetWaterHeight(pDest) + (15*SCALING_FACTOR)) - vFlyPos.y) / 12;
				break;
		}

		// make flying thing stay a certain height above the destination
		AI_GetPos(pMe).y += dm * frame_increment;


		// turn flying ai constantly
		if (AI_Standard_HeadingForWall(pMe, AVOIDANCE_FLYING_RADIUS_DISTANCE_FACTOR))
		{
			AI_NudgeRotY(pMe, frame_increment*ANGLE_DTOR(12));
		}
		else
		{
			vDestPos = AI_GetPos(pDest);
			AI_GetDyn(pMe)->m_AvoidanceAngle =
			turnang = AI_GetAvoidanceAngle(pMe, vDestPos, pDest, avoidrad);
			AI_NudgeRotY(pMe, frame_increment*turnang/50);
		}
	}
}



void AI_Standard_Flying(CGameObjectInstance *pMe, CGameObjectInstance *pDest)
{
	CVector3		vDestPos;
	float			turnang,
					dist,
					projectileang,
					avoidrad;
	int			picked,
					pickedType,
					random;

	int strongtypes[]={AI_ANIM_ATTACK_STRONG1,
							 AI_ANIM_ATTACK_STRONG2,
							 AI_ANIM_ATTACK_STRONG3,
							 AI_ANIM_ATTACK_STRONG4,
							 AI_ANIM_ATTACK_STRONG5,
							 AI_ANIM_ATTACK_STRONG6,
							 AI_ANIM_ATTACK_STRONG7,
							 AI_ANIM_ATTACK_STRONG8,
							 AI_ANIM_ATTACK_STRONG9,
							 AI_ANIM_ATTACK_STRONG10,
							 AI_ANIM_ATTACK_STRONG11,
							 AI_ANIM_ATTACK_STRONG12,
							 AI_ANIM_ATTACK_STRONG13,
							 AI_ANIM_ATTACK_STRONG14};

	int strongweights[]={10,10,10,8,8,8,6,6,6,4,4,4,2,2};

	int weaktypes[]=  {AI_ANIM_ATTACK_WEAK1,
							 AI_ANIM_ATTACK_WEAK2,
							 AI_ANIM_ATTACK_WEAK3,
							 AI_ANIM_ATTACK_WEAK4,
							 AI_ANIM_ATTACK_WEAK5,
							 AI_ANIM_ATTACK_WEAK6,
							 AI_ANIM_ATTACK_WEAK7,
							 AI_ANIM_ATTACK_WEAK8,
							 AI_ANIM_ATTACK_WEAK9,
							 AI_ANIM_ATTACK_WEAK10,
							 AI_ANIM_ATTACK_WEAK11,
							 AI_ANIM_ATTACK_WEAK12,
							 AI_ANIM_ATTACK_WEAK13,
							 AI_ANIM_ATTACK_WEAK14};

	int weakweights[]={10,10,10,8,8,8,6,6,6,4,4,4,2,2};

	int swtypes[]={AI_ANIM_ATTACK_STRONG1,
						AI_ANIM_ATTACK_WEAK1,
						AI_ANIM_ATTACK_STRONG2,
						AI_ANIM_ATTACK_WEAK2,
						AI_ANIM_ATTACK_STRONG3,
						AI_ANIM_ATTACK_WEAK3,
						AI_ANIM_ATTACK_STRONG4,
						AI_ANIM_ATTACK_WEAK4,
						AI_ANIM_ATTACK_STRONG5,
						AI_ANIM_ATTACK_WEAK5,
						AI_ANIM_ATTACK_STRONG6,
						AI_ANIM_ATTACK_WEAK6,
						AI_ANIM_ATTACK_STRONG7,
						AI_ANIM_ATTACK_WEAK7,
						AI_ANIM_ATTACK_STRONG8,
						AI_ANIM_ATTACK_WEAK8,
						AI_ANIM_ATTACK_STRONG9,
						AI_ANIM_ATTACK_WEAK9,
						AI_ANIM_ATTACK_STRONG10,
						AI_ANIM_ATTACK_WEAK10,
						AI_ANIM_ATTACK_STRONG11,
						AI_ANIM_ATTACK_WEAK11,
						AI_ANIM_ATTACK_STRONG12,
						AI_ANIM_ATTACK_WEAK12,
						AI_ANIM_ATTACK_STRONG13,
						AI_ANIM_ATTACK_WEAK13,
						AI_ANIM_ATTACK_STRONG14,
						AI_ANIM_ATTACK_WEAK14};

	int swweights[]={5,5,5,5,5,5,5,5,5,5,5,5,5,5,
						  5,5,5,5,5,5,5,5,5,5,5,5,5,5};

#if DEBUG_STAND_AI
	rmonPrintf("AI_Standard_Flying\r\n") ;
#endif

	// get avoidance radius
	avoidrad = AI_GetAvoidanceRadiusFactor(pMe);

	// how far is flying thing from the ai it is attacking
	vDestPos = AI_GetPos(pDest);
	dist = AI_DistanceSquared(pMe, vDestPos);
	projectileang = AI_GetAngle(pMe, vDestPos);

	if (AI_Standard_HeadingForWall(pMe, AVOIDANCE_FLYING_RADIUS_DISTANCE_FACTOR))
	{
		AI_GetDyn(pMe)->m_AvoidanceAngle = turnang = ANGLE_DTOR(12);
	}
	else
	{
		AI_GetDyn(pMe)->m_AvoidanceAngle =
		turnang = AI_GetAvoidanceAngle(pMe, vDestPos, pDest, avoidrad);
	}

	// do non-projectile attack
	if (dist <= AI_GetEA(pMe)->m_AttackRadius)
	{
		if (AI_Make_Turn(pMe, turnang))
		{
			random = RANDOM(99);
			if (random < AI_GetEA(pMe)->m_Aggression)
			{
				if (    (  AI_GetEA(pMe)->m_dwTypeFlags & AI_TYPE_USEWEAKATTACKS)
					  && (!(AI_GetEA(pMe)->m_dwTypeFlags & AI_TYPE_USESTRONGATTACKS)) )
				{
					// weak attacks
					picked = CGameObjectInstance__GetRandomAnimIndex(pMe, 14, weaktypes, weakweights, &pickedType);
				}
				else if (    (  AI_GetEA(pMe)->m_dwTypeFlags & AI_TYPE_USESTRONGATTACKS)
						    && (!(AI_GetEA(pMe)->m_dwTypeFlags & AI_TYPE_USEWEAKATTACKS)) )
				{
					// strong attacks
					picked = CGameObjectInstance__GetRandomAnimIndex(pMe, 14, strongtypes, strongweights, &pickedType);
				}
				else if (    (AI_GetEA(pMe)->m_dwTypeFlags & AI_TYPE_USESTRONGATTACKS)
						    && (AI_GetEA(pMe)->m_dwTypeFlags & AI_TYPE_USEWEAKATTACKS) )
				{
					// both weak & strong attacks
					picked = CGameObjectInstance__GetRandomAnimIndex(pMe, 28, swtypes, swweights, &pickedType);
				}
				else if (    (!(AI_GetEA(pMe)->m_dwTypeFlags & AI_TYPE_USESTRONGATTACKS))
						    && (!(AI_GetEA(pMe)->m_dwTypeFlags & AI_TYPE_USEWEAKATTACKS)) )
				{
					// neither weak or strong attacks
					AI_Make_Walk(pMe, turnang);
				}

				AI_GetDyn(pMe)->m_dwStatusFlags |= AI_WAITFOR_CYCLE;
				AI_SetDesiredAnimByIndex(pMe, picked, FALSE);
			}
		}
	}
	else
	{
		// is ai within projectile firing radius ?
		if (dist <= AI_GetEA(pMe)->m_ProjectileRadius)
		{
			if (RANDOM(100)<85)
				AI_Standard_Projectile_Attack(pMe, projectileang, TRUE);
			else
				AI_Make_Walk(pMe, turnang);
		}
		else
		{
			if (RANDOM(100)<3)
				AI_Standard_Projectile_Attack(pMe, projectileang, TRUE);
			else
				AI_Make_Walk(pMe, turnang);
		}
	}
}





/*****************************************************************************
*
*
*
*	EXTRA AI-HELPING CODE
*
*
*
*****************************************************************************/

#define AI_ASSIST_TURN_SPEED			ANGLE_DTOR(2)

#define AI_ASSIST_WTURN_SPEED			ANGLE_DTOR(4)
#define AI_ASSIST_WALK_SPEED			ANGLE_DTOR(7.5)

#define AI_ASSIST_RUN_SPEED			ANGLE_DTOR(15)
#define AI_ASSIST_RTURN_SPEED			ANGLE_DTOR(10)

void AI_PerformTurn(CGameObjectInstance *pMe, FLOAT TurnSpeed)
{
	FLOAT	Temp, DeltaAngle ;

	// Limit turn speed
	DeltaAngle = AI_GetDyn(pMe)->m_AvoidanceAngle ;
	Temp = DeltaAngle ;
	if (DeltaAngle > TurnSpeed)
		DeltaAngle = TurnSpeed ;
	else
	if (DeltaAngle < -TurnSpeed)
		DeltaAngle = -TurnSpeed ;

	// Take frame rate into account
	DeltaAngle *= frame_increment ;
	if (abs(DeltaAngle) > abs(Temp))
		DeltaAngle = Temp ;

	// Finally do the turn!
	AI_NudgeRotY(pMe, DeltaAngle) ;

#if DEBUG_STAND_AI
	rmonPrintf("AI_AssistRun: - Assisting!\r\n") ;
#endif
}


void AI_AssistRun(CGameObjectInstance *pMe)
{
	INT32	Anim ;
	FLOAT	AssistFactor ;

	// Avoidance angle been setup, and attacking someone?
	if ((AI_GetDyn(pMe)->m_AvoidanceAngle == 0) || (!pMe->m_pAttackerCGameObjectInstance))
		return ;

	// Set assist amount
	switch(CGameObjectInstance__TypeFlag(pMe))
	{
		case AI_OBJECT_CHARACTER_HULK:
			AssistFactor = 0.25 ;
			break;

		case AI_OBJECT_CHARACTER_DIMETRODON:
			AssistFactor = 0.0;
			break;

		default:
			AssistFactor = 1 ;
	}

	// Get current anim
	Anim = CGameObjectInstance__GetCurrentAnim(pMe) ;
	Anim = CGameObjectInstance__LookupAIAnimType(pMe, Anim) ;
	switch(Anim)
	{
		case AI_ANIM_TURNL90:
		case AI_ANIM_TURNR90:
		case AI_ANIM_TURN180:
		case AI_ANIM_TURNL902:
		case AI_ANIM_TURNR902:
		case AI_ANIM_TURN1802:
			AI_PerformTurn(pMe, AI_ASSIST_TURN_SPEED*AssistFactor) ;
			break ;

		case AI_ANIM_WTURNL90:
		case AI_ANIM_WTURNR90:
		case AI_ANIM_WTURN180:
		case AI_ANIM_WTURNL902:
		case AI_ANIM_WTURNR902:
		case AI_ANIM_WTURN1802:
			AI_PerformTurn(pMe, AI_ASSIST_WTURN_SPEED*AssistFactor) ;
			break ;

		case AI_ANIM_RTURNL90:
		case AI_ANIM_RTURNR90:
		case AI_ANIM_RTURN180:
		case AI_ANIM_RTURNL902:
		case AI_ANIM_RTURNR902:
		case AI_ANIM_RTURN1802:
			AI_PerformTurn(pMe, AI_ASSIST_RTURN_SPEED*AssistFactor) ;
			break ;

		case AI_ANIM_WALK:
		case AI_ANIM_WALK2:
			AI_PerformTurn(pMe, AI_ASSIST_WALK_SPEED*AssistFactor) ;
			break ;

		case AI_ANIM_RUN:
		case AI_ANIM_RUN2:
			AI_PerformTurn(pMe, AI_ASSIST_RUN_SPEED*AssistFactor) ;
			break ;
	}

	// Clear avoidance angle
	AI_GetDyn(pMe)->m_AvoidanceAngle = 0 ;
}



BOOL AI_InTurokWeaponPath(CGameObjectInstance *pMe, FLOAT AngleRange)
{
	FLOAT	AngleDist = abs(AI_GetAngle(pMe, AI_GetPos(pMe->m_pAttackerCGameObjectInstance))) ;

	// Is Turok firing at enemy?
	if ((CTurokMovement.FiredProjectileParticle) &&
		 (abs(GetApp()->m_RotXOffset) < ANGLE_DTOR(8)) && (AngleDist < AngleRange))
	{
		CTurokMovement.FiredProjectileParticle = FALSE ;
		return CTurokMovement.FiredProjectileParticleType ;
	}

	return FALSE ;
}



BOOL AI_IsInMovementAnim(CGameObjectInstance *pMe)
{
	// Get current anim
	INT16	Anim = CGameObjectInstance__GetCurrentAnim(pMe) ;
	Anim = CGameObjectInstance__LookupAIAnimType(pMe, Anim) ;
	switch(Anim)
	{
		case AI_ANIM_TURNL90:
		case AI_ANIM_TURNR90:
		case AI_ANIM_TURN180:
		case AI_ANIM_WTURNL90:
		case AI_ANIM_WTURNR90:
		case AI_ANIM_WTURN180:
		case AI_ANIM_RTURNL90:
		case AI_ANIM_RTURNR90:
		case AI_ANIM_RTURN180:
		case AI_ANIM_WALK:
		case AI_ANIM_RUN:
		case AI_ANIM_TURNL902:
		case AI_ANIM_TURNR902:
		case AI_ANIM_TURN1802:
		case AI_ANIM_WTURNL902:
		case AI_ANIM_WTURNR902:
		case AI_ANIM_WTURN1802:
		case AI_ANIM_RTURNL902:
		case AI_ANIM_RTURNR902:
		case AI_ANIM_RTURN1802:
		case AI_ANIM_WALK2:
		case AI_ANIM_RUN2:
			return TRUE ;

		default:
			return FALSE ;
	}
}



void AI_UpdateLeashPos(CGameObjectInstance *pMe)
{
	// Going back to leash and hit cliff
	if ((AI_GetDyn(pMe)->m_dwStatusFlags & AI_GOBACKTOLEASH) && (AICollidedWithWall))
	{
#if DEBUG_STAND_AI
		rmonPrintf("Leash Pos Updated!!!!\r\n") ;
#endif
		AI_GetDyn(pMe)->m_vLeashCoor = AI_GetPos(pMe) ;
	}
}




