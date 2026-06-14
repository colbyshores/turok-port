// main ai file in c
//


#include "stdafx.h"

#ifdef WIN32
#define AI_DECLARE_EVENT_STRINGS
#endif

//#include "ai.h"

#ifndef WIN32
#include "tengine.h"
#endif

#include "romstruc.h"
#include "scaling.h"
#include "aistruc.h"

#ifdef WIN32
#define RANDOM(n) (rand()%n)
#define ANGLE_PI			3.14159265358979323846
#define ANGLE_DTOR(deg)	(deg*(ANGLE_PI/180))
#endif

// CEnemyAttributes
/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32

void CEnemyAttributes__TakeFromVariation(CEnemyAttributes *pThis, CVariation *pSource)
{
	pThis->m_SightRadius			= ORDERBYTES(pSource->m_ea.m_SightRadius*SCALING_FACTOR
														*pSource->m_ea.m_SightRadius*SCALING_FACTOR);
	pThis->m_SightAngle			= ORDERBYTES(pSource->m_ea.m_SightAngle);	
	pThis->m_QuietRadius			= ORDERBYTES(pSource->m_ea.m_QuietRadius*SCALING_FACTOR
														*pSource->m_ea.m_QuietRadius*SCALING_FACTOR);
	pThis->m_LoudRadius			= ORDERBYTES(pSource->m_ea.m_LoudRadius*SCALING_FACTOR
														*pSource->m_ea.m_LoudRadius*SCALING_FACTOR);
	pThis->m_ProjectileRadius	= ORDERBYTES(pSource->m_ea.m_ProjectileRadius*SCALING_FACTOR
														*pSource->m_ea.m_ProjectileRadius*SCALING_FACTOR);
	pThis->m_LeashRadius	      = ORDERBYTES(pSource->m_ea.m_LeashRadius*SCALING_FACTOR
														*pSource->m_ea.m_LeashRadius*SCALING_FACTOR);

	// NOT SQUARED!
	pThis->m_CollisionRadius		= ORDERBYTES(pSource->m_ea.m_CollisionRadius*SCALING_FACTOR);
	pThis->m_CollisionWallRadius	= ORDERBYTES(pSource->m_ea.m_CollisionWallRadius*SCALING_FACTOR);
	pThis->m_CollisionHeight		= ORDERBYTES(pSource->m_ea.m_CollisionHeight*SCALING_FACTOR);
	pThis->m_DeadHeight				= ORDERBYTES(pSource->m_ea.m_DeadHeight*SCALING_FACTOR);
											
	pThis->m_AttackRadius			= ORDERBYTES(pSource->m_ea.m_AttackRadius*SCALING_FACTOR
															*pSource->m_ea.m_AttackRadius*SCALING_FACTOR);

	// if random resurrection and regeneration are set, only do regeneration
	DWORD dwTemp = pSource->m_ea.m_dwTypeFlags;
	if ((dwTemp & AI_TYPE_RANDOMRESURRECTION) && pSource->m_ea.m_Regenerate)
		dwTemp &= ~AI_TYPE_RANDOMRESURRECTION;

	pThis->m_dwTypeFlags				= ORDERBYTES(dwTemp);	
	pThis->m_dwTypeFlags2			= ORDERBYTES(pSource->m_ea.m_dwTypeFlags2);	
	pThis->m_wTypeFlags3				= ORDERBYTES(pSource->m_ea.m_wTypeFlags3);
	

	pThis->m_dwSpecies				= ORDERBYTES(pSource->m_ea.m_dwSpecies);	
	pThis->m_StartHealth				= ORDERBYTES(pSource->m_ea.m_StartHealth);
	pThis->m_Id							= ORDERBYTES(pSource->m_ea.m_Id);
	pThis->m_DeathId					= ORDERBYTES(pSource->m_ea.m_DeathId);
	pThis->m_Regenerate				= ORDERBYTES(pSource->m_ea.m_Regenerate);
	pThis->m_AttackStyle				= pSource->m_ea.m_AttackStyle;
	pThis->m_Aggression				= pSource->m_ea.m_Aggression;
	pThis->m_InteractiveAnim		= pSource->m_ea.m_InteractiveAnim;
	pThis->m_InteractiveAnimDelay	= ORDERBYTES(pSource->m_ea.m_InteractiveAnimDelay);
	pThis->m_FlyingHeight			= ORDERBYTES(pSource->m_ea.m_FlyingHeight*SCALING_FACTOR);

	pThis->m_nModel					= pSource->m_ea.m_nModel;
	pThis->m_nTexture					= pSource->m_ea.m_nTexture;

	pThis->unused1 = 0;
}

#endif

void CEnemyAttributes__SetDefaults(CEnemyAttributes *pThis)
{
	pThis->m_SightRadius				= 70;
	pThis->m_SightAngle				= ANGLE_DTOR(90);
	pThis->m_QuietRadius				= 20;
	pThis->m_LoudRadius				= 150;
	pThis->m_ProjectileRadius		= 60;
	pThis->m_LeashRadius				= 150;
	pThis->m_CollisionRadius		= 4;
	pThis->m_CollisionWallRadius	= 4;
	pThis->m_CollisionHeight		= 7;
	pThis->m_DeadHeight				= 3;
	pThis->m_AttackRadius			= 12;
	pThis->m_AttackStyle				= 100;
	pThis->m_Aggression				= 100;
	pThis->m_dwTypeFlags				= AI_TYPE_TRACKGROUND | AI_TYPE_SHADOW | AI_TYPE_USEWEAKATTACKS | AI_TYPE_USESTRONGATTACKS;
	pThis->m_StartHealth				= 100;
	pThis->m_Id							= 100;
	pThis->m_DeathId					= 0;
	pThis->m_Regenerate				= 0;
	pThis->m_dwSpecies				= (DWORD) -1;
	pThis->m_InteractiveAnim		= 0;
	pThis->m_InteractiveAnimDelay	= 0;
	pThis->m_FlyingHeight			= 30;
	pThis->m_dwTypeFlags2			= AI_TYPE2_ANTIALIASING_REDUCED;
	pThis->m_nModel					= 0;
	pThis->m_nTexture					= -1;
	pThis->m_wTypeFlags3				= 0;

	pThis->unused1 = 0;
}

// CAIDynamic
/////////////////////////////////////////////////////////////////////////////
#ifndef WIN32

void CAIDynamic__Reset(CAIDynamic *pThis, CEnemyAttributes *pEA, CVector3 vStartPos, struct CGameObjectInstance_t *pInstance)
{
	// reset variables
	pThis->m_ViewAngleOffset	= 0;
	pThis->m_dwStatusFlags		= 0;
	pThis->m_dwStatusFlags2		= 0;
	pThis->m_Agitation			= 0;
	pThis->m_PrevAgitation		= AI_AGITATED_IDLE;
	pThis->m_Evade					= AI_EVADE_NONE;
	pThis->m_vExplosion.x		= 0;
	pThis->m_vExplosion.y		= 0;
	pThis->m_vExplosion.z		= 0;
	pThis->m_Invincible			= FALSE;
	pThis->m_vLeashCoor			= vStartPos;
	pThis->m_vLeashCoor.y		= CGameRegion__GetGroundHeight(pInstance->ah.ih.m_pCurrentRegion, vStartPos.x, vStartPos.z);
	pThis->m_DeathAnimType		= -1;
	pThis->m_RetreatTimer		= 0;
	pThis->m_TeleportTime		= 0;
	pThis->m_Time1					= 0;
	pThis->m_Time2					= 0;
	pThis->m_AvoidanceAngle		= 0;
	pThis->m_FirstRun				= TRUE;
	pThis->m_FreezeTimer			= 0;
	pThis->m_InteractiveTimer	= 0;
	pThis->m_vStartPos			= pInstance->ah.ih.m_vPos;
	pThis->m_pStartRegion		= pInstance->ah.ih.m_pCurrentRegion;

	// should ai start in flocking mode ?
	if (pEA->m_dwTypeFlags & AI_TYPE_FLOCKER)
		pThis->m_dwStatusFlags |= AI_FLOCKING;

	// is this ai going to resurrect ?
	if (pEA->m_dwTypeFlags & AI_TYPE_RANDOMRESURRECTION)
	{
		if (RANDOM(100) < 80)
			pThis->m_dwStatusFlags |= AI_RESURRECTION;
	}

	// is this ai going to feign death ?
	if (pEA->m_dwTypeFlags & AI_TYPE_RANDOMFEIGNDEATH)
	{
		if (RANDOM(100) < 80)
			pThis->m_dwStatusFlags |= AI_FEIGNDEATH;
	}

	// setup random head, weapons, body parts etc...
	CAIDynamic__RandomizeHuman(pThis, pEA, pInstance) ;

	// is this ai going be an interacive animation ?
	CAIDynamic__ResetInteractiveAnim(pThis, pEA);

	// will this ai regenerate upon its death ?
	CAIDynamic__ResetRegenerate(pThis, pEA);

	// reset melt timer for this ai
	CAIDynamic__SetMeltTiming(pThis, pEA, CGameObjectInstance__TypeFlag(pInstance));
}

void CAIDynamic__RandomizeHuman(CAIDynamic *pThis, CEnemyAttributes *pEA, struct CGameObjectInstance_t *pInstance)
{
	int	ObjectType,
			type,
			r ;

	// setup correct (or random) texture for character
	pThis->m_HeadTexture = 0 ;
	pThis->m_BodyTexture = 0 ;
	pThis->m_WeaponModel = 0 ;
	pThis->m_ExtraModel = 0 ;

	ObjectType = CGameObjectInstance__TypeFlag(pInstance);
	if (ObjectType == AI_OBJECT_CHARACTER_HUMAN1)
	{
		// get variation type
		type = pEA->m_AttackStyle ;

		// FootSoldier
		if (type < 9)
		{
			// choose a random head between 0 and 11
			pThis->m_HeadTexture = RANDOM(12) ;

			// armor is done on a per enemy basis
			switch (type)
			{
				case 2:
				case 3:
				case 6:
				case 8:
					pThis->m_BodyTexture = 1 ;
					break ;

				default:
					pThis->m_BodyTexture = 0 ;
					break ;
			}

			// which weapon to use
			switch (type)
			{
				// can have
				// BONE(1)
				// SWORD(14)
				// MACE(2)
				// AXE(undefined at this time)
				case 0:
				case 1:
					r = RANDOM(100) ;
					if (r<33)
						pThis->m_WeaponModel = 1 ;
					else if (r<66)
						pThis->m_WeaponModel = 14 ;
					else
						pThis->m_WeaponModel = 2 ;
					break ;

				// can have
				// POLEARM(3)
				// POLEARM2(16)
				case 3:
					r = RANDOM(100) ;
					if (r<50)
						pThis->m_WeaponModel = 3 ;
					else
						pThis->m_WeaponModel = 16 ;
					break ;
			}


			// shins or shoulders
			switch (type)
			{
				// can have
				// NOSHINS(0)
				// SHINS(2)
				case 1:		// GRENADIER
				case 2:		// GRENADIER SNIPER
				case 4:		// SHOTGUN
				case 5:		// SHOTGUN SNIPER
				case 7:		// RIFLE SNIPER
					r = RANDOM(100) ;
					if (r<50)
						pThis->m_ExtraModel = 2 ;
					else
						pThis->m_ExtraModel = 0 ;
					break ;

				// can have
				// NOSHOULDERS(0)
				// SHOULDERS(3)
				case 3:		// POLEARM
					r = RANDOM(100) ;
					if (r<50)
						pThis->m_ExtraModel = 3 ;
					else
						pThis->m_ExtraModel = 0 ;
					break ;
			}
		}

		// Longhunters Men
		else if (type < 18)
		{
			pThis->m_HeadTexture = RANDOM(7) ;
		}
	}
}


void CAIDynamic__SetMeltTiming(CAIDynamic *pThis, CEnemyAttributes *pEA, int nObjtype)
{
	float		time;


#ifdef GERMAN

	// no melting should occur
	time = 0.0;

#else

	// setup melting character timing ?
	if (pEA->m_dwTypeFlags & AI_TYPE_MELTINGDEATH)
	{
		switch(nObjtype)
		{
			case AI_OBJECT_CHARACTER_HUMAN1:
			case AI_OBJECT_CHARACTER_HULK:
				time = 5 + RandomFloat(2.0);
				break;

			case AI_OBJECT_CHARACTER_LEAPER:
				time = 0.0;
				break;

			default:
				time = RandomFloat(2.0);
				break;
		}
	}
	else
		time = 0.0;
#endif

	// set melt timer
	pThis->m_MeltTimer = time;
}


void CAIDynamic__ResetRegenerate(CAIDynamic *pThis, CEnemyAttributes *pEA)
{
	// will this ai regenerate upon its death ?
	pThis->m_Regenerate = RANDOM(pEA->m_Regenerate + 1);

	if (pThis->m_Regenerate)
	{
		// replay interactive anim with every regeneration ?
		if (pEA->m_dwTypeFlags & AI_TYPE_REPLAYINTANIMONREG)
			pThis->m_dwStatusFlags |= AI_REGENERATEINTERANIM;
		else
			pThis->m_dwStatusFlags &= ~AI_REGENERATEINTERANIM;

//		pThis->m_cRegenerateAppearance = APPEARANCE_LENGTH;		// start off transparent
//		pThis->m_dwStatusFlags |= AI_REGENERATE;
		pThis->m_cRegenerateAppearance = 0.0;
		pThis->m_cRegenerateAppearanceWhite = 0.0;
		pThis->m_dwStatusFlags &= ~AI_REGENERATE;
	}
	else
	{
		// no regeneration will occur on this ai
		pThis->m_dwStatusFlags &= ~AI_REGENERATEINTERANIM;
		pThis->m_cRegenerateAppearance = 0.0;
		pThis->m_cRegenerateAppearanceWhite = 0.0;
		pThis->m_dwStatusFlags &= ~AI_REGENERATE;
	}
}


void CAIDynamic__ResetInteractiveAnim(CAIDynamic *pThis, CEnemyAttributes *pEA)
{
	// is this ai going be an interacive animation ?
	if (pEA->m_InteractiveAnim > 0)
	{
		pThis->m_dwStatusFlags |= AI_INTERACTIVEANIMATION;

		// is this ai visible ?
		if (pEA->m_wTypeFlags3 & AI_TYPE3_MAKEINTANIMVISIBLE)
			pThis->m_dwStatusFlags2 |= AI_VISIBLE;
		else
			pThis->m_dwStatusFlags2 &= (~AI_VISIBLE);

		// should this interactive anim hold on 1st frame ?
		if (pEA->m_dwTypeFlags2 & AI_TYPE2_HOLDINTANIMON1STFRAME)
			pThis->m_dwStatusFlags2 |= AI_HOLDINTANIMON1STFRAME;
		else
			pThis->m_dwStatusFlags2 &= ~AI_HOLDINTANIMON1STFRAME;

		if (pEA->m_InteractiveAnimDelay > 0)
		{
			pThis->m_InteractiveTimer = pEA->m_InteractiveAnimDelay;
			pThis->m_dwStatusFlags |= AI_INTERANIMDELAY;
		}
		else
		{
			pThis->m_InteractiveTimer = 0;
			pThis->m_dwStatusFlags &= ~AI_INTERANIMDELAY;
		}
	}
	else
	{
		pThis->m_dwStatusFlags2 |= AI_VISIBLE;
	}
}


void CAIDynamic__SetHealth(CAIDynamic *pThis, CEnemyAttributes *pEA, int nTypeFlag)
{
	// maxhealth is only used for player, but to be safe set it for every ai.
//	pThis->m_MaxHealth = AI_HEALTH_PLAYER;


	// set health
	switch (nTypeFlag)
	{
		// BOSSES SET THEIR OWN HEALTH - LEAVE THIS ALOVE IAN!!!!
		case AI_OBJECT_CHARACTER_MANTIS_BOSS:
		case AI_OBJECT_CHARACTER_TREX_BOSS:
		case AI_OBJECT_CHARACTER_CAMPAIGNER_BOSS:
		case AI_OBJECT_CHARACTER_LONGHUNTER_BOSS:
		case AI_OBJECT_CHARACTER_HUMVEE_BOSS:
			break;

		case AI_OBJECT_CHARACTER_PLAYER:
			pThis->m_Health = pEA->m_StartHealth;
			break;

		default:
			switch(GetApp()->m_Difficulty)
			{
				case EASY:
					if (pEA->m_StartHealth)
					{
						pThis->m_Health = pEA->m_StartHealth * 70 / 100;

						if (pThis->m_Health <= 0)
							pThis->m_Health = 1;
					}
					else
						pThis->m_Health = 0;
					break;

				case HARD:
					pThis->m_Health = pEA->m_StartHealth * 140 / 100;
					break;

				default:
				case MEDIUM:
					pThis->m_Health = pEA->m_StartHealth;
					break;
			}
			break;
	}
}
#endif

/////////////////////////////////////////////////////////////////////////////

