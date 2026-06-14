// weapon ai file
//

#include "stdafx.h"
#include "aiweap.h"
#include "particle.h"
#include "audio.h"
#include "sfx.h"
#include "pickup.h"





void AI_Update_Weapon_Movement_Anim(CGameObjectInstance *pMe, CTControl *pCTControl, CTMove *pCTMove, int waterFlag);

static BOOL	LastWeapon;


// update turok's on screen weapon animation
//
BOOL AI_Update_Turok_Weapon(CGameObjectInstance *pMe)
{
#ifdef PLATFORM_PORT
	/* PORT: called with the player (GetPlayer()) which is NULL on the legal/title screens
	 * (no level loaded). N64 tolerates the NULL deref; a protected host faults. Bail. */
	if (!pMe) return FALSE;
#endif
#ifndef WIN32

	CTControl	*pCTControl = NULL;
	CTMove		*pCTMove    = NULL;
	CEngineApp	*pApp = NULL;
	CVector3		vPos;
	int			nSound,
					waterFlag;
	CCamera		*pCamera;
	BOOL			ret = FALSE;


	// get pointer to engine
	pApp = GetApp ();
	if (pApp == NULL)
		return FALSE;

	// camera object present ?
	pCamera = &pApp->m_Camera;
	if (!pCamera)
		return FALSE;

	// is this ai the main character turok ?
	ASSERT(pMe == CEngineApp__GetPlayer(GetApp()));

	// control object valid ?
	pCTMove    = &CTurokMovement;
	pCTControl = &CTControlArray;

	if (pCTMove != NULL && pCTControl != NULL)
	{
		waterFlag = pCTMove->WaterFlag;
		if (pCTMove->InAntiGrav)
			waterFlag = PLAYER_NOT_NEAR_WATER;

		// if in cinema, or about to do one, don't run weapon ai
		if ((CCamera__InCinemaMode(pCamera)) || (GetApp()->m_CinemaFlags))
			return FALSE;

		// if in special enemy modify mode don't run weapon ai
//		if (pApp->m_dwCheatFlags & CHEATFLAG_MODIFYENEMY)
//			return FALSE;

		CTMove__LoadPlayerWeapon(&CTurokMovement);

		// do special case for weapons - they have to fire more than one bullet/shell in the anim
		if (!AI_Cycle_Completed(pMe) && !pCTMove->WeaponTakeOffQuickly)
		{
			switch(pCTMove->WeaponCurrent)
			{
				case AI_OBJECT_WEAPON_ASSAULTRIFLE:
				case AI_OBJECT_WEAPON_RIOTSHOTGUN:
					CTMove__SpecialEffect(pCTMove, pCTMove->WeaponFired);
					break;

				default:;
			}
			return FALSE;
		}

		if (pCTMove->WeaponTakeOffQuickly)
		{
			pCTMove->WeaponTimer -= frame_increment*(1.0/FRAME_FPS);
			if (pCTMove->WeaponTimer > 0.0)
			{
//				AI_Update_Weapon_Movement_Anim(pMe, pCTControl, pCTMove, waterFlag);
				AI_SetDesiredAnim(pMe, AI_ANIM_WEAPON_OFFSCREEN, TRUE);
				AI_GetDyn(pMe)->m_dwStatusFlags &= ~AI_WAITFOR_CYCLE;
				return FALSE;
			}

			pCTMove->WeaponTimer = 0.0;
			pCTMove->WeaponTakeOffQuickly = FALSE;
			if (		(pCTMove->WeaponMode != TWEAPONMODE_GOINGTOSTAYOFF)
					&&	(pCTMove->WeaponMode != TWEAPONMODE_ON) )
			{
				pCTMove->WeaponMode = TWEAPONMODE_OFF;
			}

			pCTMove->WeaponCount = 0;
			AI_GetDyn(pMe)->m_dwStatusFlags &= ~AI_WAITFOR_CYCLE;
		}
		else
		{
			// load correct weapon model
			pCTMove->WeaponCurrentAnim = -1;
			CTMove__LoadPlayerWeapon(pCTMove);
		}

		// no gun is fired yet
		CTMove__SetWeaponFiredStatus(pCTMove, TWEAPON_NONE);

		// is player using last weapon ?
		LastWeapon = CTMove__OnLastWeapon(pCTMove);

		// which weapon ?
		if (		(pCTMove->WeaponMode == TWEAPONMODE_ON)
				&&	(pMe->m_AI.m_Health)
				&& (pCTMove->InMenuTimer == 0.0)
				&& (!pCTMove->WeaponChanged) )
		{
			switch (pCTMove->WeaponCurrent)
			{
				case AI_OBJECT_WEAPON_KNIFE:
					if (AI_Update_Weapon_Knife(pMe, pCTControl, pCTMove, waterFlag))
						ret = TRUE;
					break;

				case AI_OBJECT_WEAPON_TOMAHAWK:
					if (AI_Update_Weapon_Tomahawk(pMe, pCTControl, pCTMove, waterFlag))
						ret = TRUE;
					break;

				case AI_OBJECT_WEAPON_TEKBOW:
					if (AI_Update_Weapon_TekBow(pMe, pCTControl, pCTMove, waterFlag))
						ret = TRUE;
					break;

				case AI_OBJECT_WEAPON_SEMIAUTOMATICPISTOL:
					if (AI_Update_Weapon_SemiAutomaticPistol(pMe, pCTControl, pCTMove, waterFlag))
						ret = TRUE;
					break;

				case AI_OBJECT_WEAPON_ASSAULTRIFLE:
					if (AI_Update_Weapon_AssaultRifle(pMe, pCTControl, pCTMove, waterFlag))
						ret = TRUE;
					break;

				case AI_OBJECT_WEAPON_MACHINEGUN:
					if (AI_Update_Weapon_MachineGun(pMe, pCTControl, pCTMove, waterFlag))
						ret = TRUE;
					break;

				case AI_OBJECT_WEAPON_RIOTSHOTGUN:
					if (AI_Update_Weapon_RiotShotgun(pMe, pCTControl, pCTMove, waterFlag))
						ret = TRUE;
					break;

				case AI_OBJECT_WEAPON_AUTOMATICSHOTGUN:
					if (AI_Update_Weapon_AutomaticShotgun(pMe, pCTControl, pCTMove, waterFlag))
						ret = TRUE;
					break;

				case AI_OBJECT_WEAPON_MINIGUN:
					if (AI_Update_Weapon_MiniGun(pMe, pCTControl, pCTMove, waterFlag))
						ret = TRUE;
					break;

				case AI_OBJECT_WEAPON_GRENADE_LAUNCHER:
					if (AI_Update_Weapon_Grenade_Launcher(pMe, pCTControl, pCTMove, waterFlag))
						ret = TRUE;
					break;

				case AI_OBJECT_WEAPON_TECHWEAPON1:
					if (AI_Update_Weapon_TechWeapon1(pMe, pCTControl, pCTMove, waterFlag))
						ret = TRUE;
					break;

				case AI_OBJECT_WEAPON_ROCKET:
					if (AI_Update_Weapon_Rocket(pMe, pCTControl, pCTMove, waterFlag))
						ret = TRUE;
					break;

				case AI_OBJECT_WEAPON_SHOCKWAVE:
					if (AI_Update_Weapon_Shockwave(pMe, pCTControl, pCTMove, waterFlag))
						ret = TRUE;
					break;

				case AI_OBJECT_WEAPON_TECHWEAPON2:
					if (AI_Update_Weapon_TechWeapon2(pMe, pCTControl, pCTMove, waterFlag))
						ret = TRUE;
					break;

				case AI_OBJECT_WEAPON_CHRONOSCEPTER:
					if (AI_Update_Weapon_Chronoscepter(pMe, pCTControl, pCTMove, waterFlag))
						ret = TRUE;
					break;
			}

			// was weapon fired ?
			CTMove__SpecialEffect(pCTMove, pCTMove->WeaponFired);

			if (ret)
				return TRUE;
		}

		// what is weapon doing on screen ?
		switch ( pCTMove->WeaponMode )
		{
			case TWEAPONMODE_OFF:
				pCTMove->WeaponCurrent = pCTMove->WeaponSelect;

				if (pMe->m_AI.m_Health)
					pCTMove->WeaponMode    = TWEAPONMODE_COMINGON;
				else
					pCTMove->WeaponMode = TWEAPONMODE_OFF;

				pCTMove->WeaponChanged = FALSE;
				pCTMove->WeaponCount   = 0;
				pCTMove->WeaponBarrelZRot    = 0;
				pCTMove->WeaponBarrelZRotInc = 0;
				pCTMove->AmmoUseCounter = 0.0;

				if (waterFlag == PLAYER_BELOW_WATERSURFACE)
					pCTMove->WeaponCurrentAnim = AI_ANIM_WEAPON_UNDERWATER_IDLE;
				else
					pCTMove->WeaponCurrentAnim = AI_ANIM_WEAPON_ONSCREEN;

				CTMove__LoadPlayerWeapon(pCTMove);
				break;

			case TWEAPONMODE_STAYOFF:
				if (waterFlag == PLAYER_BELOW_WATERSURFACE)
				{
					CTMove__SelectWeaponByAIType(pCTMove, AI_OBJECT_WEAPON_KNIFE, FALSE);
					AI_SetDesiredAnim(pMe, AI_ANIM_WEAPON_UNDERWATER_IDLE, TRUE);
					AI_GetDyn(pMe)->m_dwStatusFlags &= ~AI_WAITFOR_CYCLE;
				}
				pCTMove->AmmoUseCounter = 0.0;
				break;

			case TWEAPONMODE_COMINGON:
				if (waterFlag == PLAYER_BELOW_WATERSURFACE)
				{
					AI_SetDesiredAnim(pMe, AI_ANIM_WEAPON_UNDERWATER_IDLE, TRUE);
					AI_GetDyn(pMe)->m_dwStatusFlags |= AI_WAITFOR_CYCLE;
				}
				else
				{
					AI_SetDesiredAnim(pMe, AI_ANIM_WEAPON_ONSCREEN, TRUE);
					AI_GetDyn(pMe)->m_dwStatusFlags |= AI_WAITFOR_CYCLE;
				}

				if ( pCTMove->WeaponChanged )
				{
					pCTMove->WeaponMode    = TWEAPONMODE_GOINGOFF;
					pCTMove->WeaponChanged = FALSE;
				}
				else
				{
					if (pCTMove->WeaponCount == 0)
					{
						switch (pCTMove->WeaponCurrent)
						{
							case AI_OBJECT_WEAPON_GRENADE_LAUNCHER:
								nSound = SOUND_READY_GRENADE_LAUNCHER;
								break;
							
							case AI_OBJECT_WEAPON_TECHWEAPON1:
								nSound = SOUND_READY_TEK_WEAPON_1;
								break;

							case AI_OBJECT_WEAPON_CHRONOSCEPTER:
//								nSound = SOUND_READY_;
								nSound = -1;
								break;

							case AI_OBJECT_WEAPON_SEMIAUTOMATICPISTOL:
								nSound = SOUND_READY_PISTOL;
								break;

							case AI_OBJECT_WEAPON_ROCKET:
								nSound = SOUND_READY_MISSILE_LAUNCHER;
								break;

							case AI_OBJECT_WEAPON_TECHWEAPON2:
								nSound = SOUND_READY_TEK_WEAPON_2;
								break;

							case AI_OBJECT_WEAPON_RIOTSHOTGUN:
								nSound = SOUND_READY_SHOTGUN;
								break;
							
							case AI_OBJECT_WEAPON_MACHINEGUN:				
								nSound = SOUND_READY_PULSE_RIFLE;
								break;
							
							case AI_OBJECT_WEAPON_ASSAULTRIFLE:
								nSound = SOUND_READY_ASSAULT_RIFLE;
								break;
							
							case AI_OBJECT_WEAPON_MINIGUN:
								nSound = SOUND_READY_MINI_GUN;
								break;
		
							case AI_OBJECT_WEAPON_AUTOMATICSHOTGUN:
								nSound = SOUND_READY_AUTO_SHOTGUN;
								break;
								
							default:
								nSound = -1;
						}
				
						if (nSound != -1)
						{
							vPos = AI_GetPos(pMe);
							CScene__DoSoundEffect(&GetApp()->m_Scene, nSound, 0, NULL, &vPos, 0);
						}
					}
					
					pCTMove->AmmoUseCounter = 0.0;
					pCTMove->WeaponCount++;
					if ( pCTMove->WeaponCount >= 5*frame_increment )
					{
						pCTMove->WeaponMode = TWEAPONMODE_ON;
					}
				}
				break;

			case TWEAPONMODE_ON:
				// turok did not use weapon - animate it based on walking speed
				AI_Update_Weapon_Movements(pMe, pCTControl, pCTMove, waterFlag);
				break;

			case TWEAPONMODE_GOINGOFF:
			case TWEAPONMODE_GOINGTOSTAYOFF:
				switch (pCTMove->WeaponCurrent)
				{
					case AI_OBJECT_WEAPON_SHOCKWAVE:
						pCTMove->WeaponBarrelZRot = 0;
						break;
				}

				AI_SetDesiredAnim(pMe, AI_ANIM_WEAPON_OFFSCREEN, TRUE);
				AI_GetDyn(pMe)->m_dwStatusFlags |= AI_WAITFOR_CYCLE;

				if ( pCTMove->WeaponCount <= 0 )
				{
					if (pCTMove->WeaponMode == TWEAPONMODE_GOINGOFF)
						pCTMove->WeaponMode    = TWEAPONMODE_OFF;
					else
						pCTMove->WeaponMode    = TWEAPONMODE_STAYOFF;

					pCTMove->WeaponChanged = FALSE;
					if(pCTMove->hSFXBarrel != -1)
					{
						killCFX(pCTMove->hSFXBarrel);
						pCTMove->hSFXBarrel = -1;
						killSoundType(SOUND_MINI_GUN_WHIR);
					}
				}
				pCTMove->WeaponCount --;
				if (pCTMove->WeaponCount < 0)
					pCTMove->WeaponCount = 0;
				break;

			case TWEAPONMODE_GOINGOFF_LASTBULLET:
				AI_SetDesiredAnim(pMe, AI_ANIM_WEAPON_LASTBULLETOFFSCREEN, TRUE);
				AI_GetDyn(pMe)->m_dwStatusFlags |= AI_WAITFOR_CYCLE;

				if ( pCTMove->WeaponCount <= 0 )
				{
					pCTMove->WeaponMode    = TWEAPONMODE_OFF;
					pCTMove->WeaponChanged = FALSE;

					if (pCTMove->hSFXBarrel != -1)
					{
						killCFX(pCTMove->hSFXBarrel);
						pCTMove->hSFXBarrel = -1;
						killSoundType(SOUND_MINI_GUN_WHIR);
					}
				}
				pCTMove->WeaponCount --;
				if (pCTMove->WeaponCount < 0)
					pCTMove->WeaponCount = 0;
				break;

		}
		return TRUE;
	}

#endif

	return FALSE;
}


// load correct weapon
//
void AI_Reset_Turok_Weapon(CGameObjectInstance *pAI)
{
	// load correct weapon model
	CTurokMovement.WeaponCurrentAnim = AI_ANIM_WEAPON_IDLE;
	CTMove__LoadPlayerWeapon(&CTurokMovement);
	CTurokMovement.WeaponMode = TWEAPONMODE_ON;
	AI_Kill_Weapon_SFX(&CTurokMovement);		// kill weapon sfx's
}


// update weapon movements - idle, walk & run
//
void AI_Update_Weapon_Movements(CGameObjectInstance *pMe, CTControl *pCTControl, CTMove *pCTMove, int waterFlag)
{

	AI_Update_Weapon_Movement_Anim(pMe, pCTControl, pCTMove, waterFlag);

	if (pCTMove->WeaponChanged || pMe->m_AI.m_Health==0)
	{
		pCTMove->WeaponMode    = TWEAPONMODE_GOINGOFF;
		pCTMove->WeaponChanged = FALSE;
	}
}


// update weapon movements - idle, walk & run
//
void AI_Update_Weapon_Movement_Anim(CGameObjectInstance *pMe, CTControl *pCTControl, CTMove *pCTMove, int waterFlag)
{
	float mspd;


	// calculate speed
	mspd = abs(pCTMove->MoveSpeed);

	// is turok underwater ?
	if (		(waterFlag == PLAYER_BELOW_WATERSURFACE)
			&&	(!pCTMove->InAntiGrav) )
	{
		// player is below water surface
		AI_SetDesiredAnim(pMe, AI_ANIM_WEAPON_UNDERWATER_IDLE, TRUE);
		AI_GetDyn(pMe)->m_dwStatusFlags &= ~AI_WAITFOR_CYCLE;
	}
	else
	{
		// player is not below water surface - what is weapon doing ?
		if (mspd < 0.05 || waterFlag != PLAYER_NOT_NEAR_WATER)
		{
			// idle speed
			AI_SetDesiredAnim(pMe, AI_ANIM_WEAPON_IDLE, TRUE);
			AI_GetDyn(pMe)->m_dwStatusFlags &= ~AI_WAITFOR_CYCLE;
		}
		else if (mspd <= TMOVE_MAX_WALKSPEED)
		{
			// walk speed
			AI_SetDesiredAnim(pMe, AI_ANIM_WEAPON_WALK, TRUE);
			AI_GetDyn(pMe)->m_dwStatusFlags &= ~AI_WAITFOR_CYCLE;
		}
		else
		{
			// run speed
			AI_SetDesiredAnim(pMe, AI_ANIM_WEAPON_RUN, TRUE);
			AI_GetDyn(pMe)->m_dwStatusFlags &= ~AI_WAITFOR_CYCLE;
		}
	}
}


#ifndef WIN32
// turoks knife
//
BOOL AI_Update_Weapon_Knife(CGameObjectInstance *pMe, CTControl *pCTControl, CTMove *pCTMove, int waterFlag)
{
	// declare variables
	CVector3	vPos;
	u8	rand = RANDOM ( 100 );
	int		SwishSoundNum = SOUND_KNIFE_SWISH_1;

	// using knife ?
	if ( CTControl__IsPrimaryFire ( pCTControl ) )
	{
		vPos = AI_GetPos(pMe);


		// is turok underwater ?
		if (		(waterFlag == PLAYER_BELOW_WATERSURFACE)
				&&	(!pCTMove->InAntiGrav) )
		{
			// player is below water - do underwater knife
			SwishSoundNum = SOUND_UNDERWATER_SWIM_2;
			AI_SetDesiredAnim ( pMe, AI_ANIM_WEAPON_UNDERWATER_SLASH, TRUE );
		}
		else
		{
			// player not underwater
			if ( rand < 33 )
			{
				// do left slash
				SwishSoundNum = SOUND_KNIFE_SWISH_1;
				AI_SetDesiredAnim ( pMe, AI_ANIM_WEAPON_SLASHLEFT, TRUE );
			}
			else if ( rand >= 33 && rand < 66 )
			{
				// do right slash
				SwishSoundNum = SOUND_KNIFE_SWISH_2;
				AI_SetDesiredAnim ( pMe, AI_ANIM_WEAPON_SLASHRIGHT, TRUE );
			}
			else if ( rand >= 66 )
			{
				// do stab
				SwishSoundNum = SOUND_KNIFE_SWISH_3;
				AI_SetDesiredAnim ( pMe, AI_ANIM_WEAPON_STAB, TRUE );
			}
		}

		CScene__DoSoundEffect(&GetApp()->m_Scene, SwishSoundNum , 0, NULL, &vPos, 0);


		AI_GetDyn(pMe)->m_dwStatusFlags |= AI_WAITFOR_CYCLE;
		CGameObjectInstance__RestartAnim(pMe);
		return TRUE;
	}


	// knife not used
	return FALSE;
}


// turoks tomahawk
//
BOOL AI_Update_Weapon_Tomahawk ( CGameObjectInstance *pMe, CTControl *pCTControl, CTMove *pCTMove, int waterFlag)
{
	// declare variables
	CVector3		vPos;
	u8	rand = RANDOM ( 100 );


	// using tomahawk ?
	if ( CTControl__IsPrimaryFire ( pCTControl ) )
	{
		vPos = AI_GetPos(pMe);
		if ( rand < 33 )
		{
			// do left slash
			AI_SetDesiredAnim ( pMe, AI_ANIM_WEAPON_SLASHLEFT, TRUE );
		}
		else if ( rand >= 33 && rand < 66 )
		{
			// do right slash
			AI_SetDesiredAnim ( pMe, AI_ANIM_WEAPON_SLASHRIGHT, TRUE );
		}
		else if ( rand >= 66 )
		{
			// do stab
			AI_SetDesiredAnim ( pMe, AI_ANIM_WEAPON_STAB, TRUE );
		}

		//CScene__DoSoundEffect(&GetApp()->m_Scene, sfx_num, 0, NULL, &tempvec, 0);

		AI_GetDyn(pMe)->m_dwStatusFlags |= AI_WAITFOR_CYCLE;
		CGameObjectInstance__RestartAnim(pMe);
		return TRUE;
	}


	// tomahawk not used
	return FALSE;
}


// turoks tekbow
//
BOOL AI_Update_Weapon_TekBow ( CGameObjectInstance *pMe, CTControl *pCTControl, CTMove *pCTMove, int waterFlag)
{
	// declare variables
	CVector3			vOffset,
						vParticlePos,
						vBaseVelocity,
						vPos;
	CQuatern			qRotateDirection;
	CGameRegion		*pParticleRegion;
	BOOL				bFiring;


	// is player using tek bow ?
	bFiring = CTControl__IsPrimaryFire(pCTControl);

	// using tek bow ?
	if (bFiring && pCTMove->BowStarted >= 0.0)
	{
		// turok is using the bow, any arrows left ?
		CTMove__SetWeaponFiredStatus(pCTMove, TWEAPON_TEKBOW);
		if (CTMove__WeaponEmpty(pCTMove, AI_OBJECT_WEAPON_TEKBOW, 1)) return FALSE;

		// first time user has pressed trigger ?
		if (pCTMove->BowStarted==0.0)
		{
			vPos = AI_GetPos(pMe);
			CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_BOW_STRETCH, 0, NULL, &vPos, 0);

			// turok has just pressed trigger for bow (bow now starts to charge up)
			AI_SetDesiredAnim ( pMe, AI_ANIM_WEAPON_FIRE1, TRUE );			// charge cycle selected
			AI_GetDyn(pMe)->m_dwStatusFlags &= AI_WAITFOR_CYCLE;
			pCTMove->BowStarted += frame_increment*(1.0/FRAME_FPS);
		}
		else
		{
			// get rotation for arrow special effect
			qRotateDirection = CGameObjectInstance__GetAimRotation(pMe);
			vBaseVelocity.x = vBaseVelocity.y = vBaseVelocity.z = 0;

			// turok is pulling back string on bow
			pCTMove->BowStarted += frame_increment*(1.0/FRAME_FPS);
			if (		(pCTMove->BowStarted>=TMOVE_SUPER_BOWPOSITION)
					&&	(pCTMove->BowStarted<=TMOVE_SUPER_BOWENDPOSITION)
					&& (!pCTMove->BowFlashOccurred) )
			{
				// turok has enabled the super arrow, add special flash effect to arrow to indicated this
				vOffset.x = 0.07*SCALING_FACTOR;
				vOffset.y = -0.6*SCALING_FACTOR;
				vOffset.z = 2.9*SCALING_FACTOR;

				// get particle position
				CGameObjectInstance__GetOffsetPositionAndRegion(CEngineApp__GetPlayer(GetApp()),
																				vOffset,
																				&vParticlePos,
																				&pParticleRegion);

				// create particle
				CParticleSystem__CreateParticle(&GetApp()->m_Scene.m_ParticleSystem, &pMe->ah.ih, PARTICLE_TYPE_SARROWFLASH,
														  vBaseVelocity, qRotateDirection,
														  vParticlePos, pParticleRegion, PARTICLE_NOEDGE, TRUE);

				pCTMove->BowFlashOccurred = TRUE;
			}
			else if (pCTMove->BowStarted>TMOVE_SUPER_BOWENDPOSITION)
			{
				// turok has held arrow too long - super arrow fizzles out (loses its power)
/*				vOffset.x = 0.07*SCALING_FACTOR;
				vOffset.y = -0.6*SCALING_FACTOR;
				vOffset.z = 2.9*SCALING_FACTOR;
				
				// get particle position
				CGameObjectInstance__GetOffsetPositionAndRegion(CEngineApp__GetPlayer(GetApp()),
																				vOffset,
																				&vParticlePos,
																				&pParticleRegion);

				// create particle
				CParticleSystem__CreateParticle(&GetApp()->m_Scene.m_ParticleSystem, &pMe->ah.ih, PARTICLE_TYPE_SARROWFIZZLE,
														  vBaseVelocity, qRotateDirection,
														  vParticlePos, pParticleRegion, PARTICLE_NOEDGE, TRUE);

*/				

			}
		}

		return TRUE;
	}
	else if (		(!bFiring)
					&& (pCTMove->BowStarted > 0.0) )
	{
		// fire an arrow
		vPos = AI_GetPos(pMe);
		CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_BOW_TWANG, 0, NULL, &vPos, 0);
		CTMove__FireBullet   ( pCTMove, TWEAPON_TEKBOW );

		// if any explosive arrows, use em first
		if (pCTMove->ExpTekBowAmmo)
		{	
			CTMove__DecreaseAmmo(pCTMove, &pCTMove->ExpTekBowAmmo, TEKBOW_AMMO_USE);
			CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_ARROW_FLY_TEK, 0, NULL, &vPos, 0);
		}
		else
		{
			CTMove__DecreaseAmmo(pCTMove, &pCTMove->TekBowAmmo, TEKBOW_AMMO_USE);
			CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_ARROW_FLY_NORMAL, 0, NULL, &vPos, 0);
		}


		AI_SetDesiredAnim(pMe, AI_ANIM_WEAPON_FIRE2, TRUE);
		AI_GetDyn(pMe)->m_dwStatusFlags |= AI_WAITFOR_CYCLE;
		CGameObjectInstance__RestartAnim(pMe);
		pCTMove->BowStarted=0.0;
		pCTMove->BowFlashOccurred = FALSE;
		if (CTMove__WeaponEmpty(pCTMove, AI_OBJECT_WEAPON_TEKBOW, 1))
		{
			// no more arrows
			AI_Update_Weapon_Movements ( pMe, pCTControl, pCTMove, waterFlag);
			pCTMove->WeaponMode = TWEAPONMODE_GOINGOFF_LASTBULLET;

			// change to next weapon
			pCTMove->WeaponSelect = pCTMove->WeaponCurrent;
			CTMove__SelectWeapon(pCTMove, TWEAPON_AMMO, TRUE);
			CTMove__DisplayWeaponsIcons(pCTMove, TRUE);
		}
		return TRUE;
	}
	else if (		(!bFiring)
					&&	(pCTMove->BowStarted < 0.0) )
	{
		pCTMove->BowStarted = 0.0;
	}

	return FALSE;
}


// turoks semi automatic pistol
//
BOOL AI_Update_Weapon_SemiAutomaticPistol ( CGameObjectInstance *pMe, CTControl *pCTControl, CTMove *pCTMove, int waterFlag)
{
	CVector3 vPos;

	// declare variables
	CEngineApp *pApp;
	pApp = GetApp ();
	if (pApp == NULL) return FALSE;

	// using semi automatic pistol ?
	if (CTControl__IsPrimaryFire(pCTControl))
	{
		// fire semi automatic pistol
		CTMove__SetWeaponFiredStatus(pCTMove, TWEAPON_SEMIAUTOMATICPISTOL);
		if (CTMove__WeaponEmpty(pCTMove, AI_OBJECT_WEAPON_SEMIAUTOMATICPISTOL, 1))
		{
			if (pCTMove->Mode == TMMODE_CLIMBING)
			{
				CTMove__SelectWeaponByAIType(pCTMove, AI_OBJECT_WEAPON_KNIFE, FALSE);
			}
			else
			{
				if (LastWeapon)
					CTMove__SelectWeapon(pCTMove, TWEAPON_AMMO, FALSE);
				else
					CTMove__SelectWeapon(pCTMove, TWEAPON_AMMO, TRUE);
			}

			CTMove__DisplayWeaponsIcons(pCTMove, TRUE);
			return FALSE;
		}

		vPos = AI_GetPos(pMe);
		CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_PISTOL_SHOT, 0, NULL, &vPos, 0);
		AI_Event_Dispatcher (&pMe->ah.ih, &pMe->ah.ih, AI_MEVENT_LOUDNOISE, AI_SPECIES_ALL, vPos, 10);

		CTMove__DecreaseAmmo(pCTMove, &pCTMove->BulletPool, PISTOL_AMMO_USE);
		CTMove__ShakeScreen(pCTMove, pApp, 5*SCALING_FACTOR/16, -(ANGLE_PI/150), TRUE);
		CTMove__FireBullet(pCTMove, TWEAPON_SEMIAUTOMATICPISTOL);
		AI_SetDesiredAnim(pMe, AI_ANIM_WEAPON_FIRE1, TRUE);
		AI_GetDyn(pMe)->m_dwStatusFlags |= AI_WAITFOR_CYCLE;
		CGameObjectInstance__RestartAnim(pMe);
		return TRUE;
	}
	else
	{
		if (CTMove__WeaponEmpty(pCTMove, AI_OBJECT_WEAPON_SEMIAUTOMATICPISTOL, 1))
		{
			if (pCTMove->Mode == TMMODE_CLIMBING)
			{
				CTMove__SelectWeaponByAIType(pCTMove, AI_OBJECT_WEAPON_KNIFE, FALSE);
			}
			else
			{
				if (LastWeapon)
					CTMove__SelectWeapon(pCTMove, TWEAPON_AMMO, FALSE);
				else
					CTMove__SelectWeapon(pCTMove, TWEAPON_AMMO, TRUE);
			}

			CTMove__DisplayWeaponsIcons(pCTMove, TRUE);
		}
	}


		
	return FALSE;
}


// turoks assault rifle
//
BOOL AI_Update_Weapon_AssaultRifle ( CGameObjectInstance *pMe, CTControl *pCTControl, CTMove *pCTMove, int waterFlag)
{
	// declare variables
	CEngineApp *pApp;
	pApp = GetApp ();
	if (pApp == NULL) return FALSE;

	
	// using assault rifle ?
	if (CTControl__IsPrimaryFire(pCTControl))
	{
		// fire assault rifle
		pCTMove->AmmoUseCounter = 0.0;
		pCTMove->FiredBullet1 = FALSE;
		pCTMove->FiredBullet2 = FALSE;
		pCTMove->FiredBullet3 = FALSE;

		CTMove__SetWeaponFiredStatus(pCTMove, TWEAPON_ASSAULTRIFLE);
		if (CTMove__WeaponEmpty(pCTMove, AI_OBJECT_WEAPON_ASSAULTRIFLE, 1))
		{
			if (LastWeapon)
				CTMove__SelectWeapon(pCTMove, TWEAPON_AMMO, FALSE);
			else
				CTMove__SelectWeapon(pCTMove, TWEAPON_AMMO, TRUE);

			CTMove__DisplayWeaponsIcons(pCTMove, TRUE);
			return FALSE;
		}
		AI_Event_Dispatcher (&pMe->ah.ih, &pMe->ah.ih, AI_MEVENT_LOUDNOISE, AI_SPECIES_ALL, AI_GetPos(pMe), 10);
		AI_SetDesiredAnim ( pMe, AI_ANIM_WEAPON_FIRE1, TRUE );
		AI_GetDyn(pMe)->m_dwStatusFlags |= AI_WAITFOR_CYCLE;
		CGameObjectInstance__RestartAnim(pMe);
		return TRUE;
	}
	else
	{
		if (CTMove__WeaponEmpty(pCTMove, AI_OBJECT_WEAPON_ASSAULTRIFLE, 1))
		{
			if (LastWeapon)
				CTMove__SelectWeapon(pCTMove, TWEAPON_AMMO, FALSE);
			else
				CTMove__SelectWeapon(pCTMove, TWEAPON_AMMO, TRUE);

			CTMove__DisplayWeaponsIcons(pCTMove, TRUE);
		}
	}
	
	return FALSE;
}


// turoks machine gun
//
BOOL AI_Update_Weapon_MachineGun ( CGameObjectInstance *pMe, CTControl *pCTControl, CTMove *pCTMove, int waterFlag)
{
	// declare variables
	CEngineApp *pApp;

	// get pointer to app
	pApp = GetApp ();
	if ( pApp == NULL ) return FALSE;


	// using machine gun ?
	if ( CTControl__IsPrimaryFire ( pCTControl ) )
	{
		if (pCTMove->FireFirstMachineGunBullet)
		{
			pCTMove->AmmoUseCounter = TWEAPON_MACHINEGUN_USECNT;
			pCTMove->FireFirstMachineGunBullet = FALSE;
		}

		CTMove__SetWeaponFiredStatus ( pCTMove, TWEAPON_MACHINEGUN );
		// fire machine gun
		if ( ! CTMove__WeaponEmpty ( pCTMove, AI_OBJECT_WEAPON_MACHINEGUN, 1 ) )
		{
			AI_Event_Dispatcher (&pMe->ah.ih, &pMe->ah.ih, AI_MEVENT_LOUDNOISE, AI_SPECIES_ALL, AI_GetPos(pMe), 10);
			AI_SetDesiredAnim ( pMe, AI_ANIM_WEAPON_GUNKICKBACK, TRUE );
			AI_GetDyn(pMe)->m_dwStatusFlags &= AI_WAITFOR_CYCLE;
		}
		else
		{
			// weapon did not fire any bullets, so we must idle it depending on turok speed
			AI_Update_Weapon_Movements(pMe, pCTControl, pCTMove, waterFlag);
			pCTMove->WeaponMode = TWEAPONMODE_GOINGOFF;
			if (LastWeapon)
				CTMove__SelectWeapon(pCTMove, TWEAPON_AMMO, FALSE);
			else
				CTMove__SelectWeapon(pCTMove, TWEAPON_AMMO, TRUE);

			CTMove__DisplayWeaponsIcons(pCTMove, TRUE);
		}
		return TRUE;
	}
	else
	{
//		pCTMove->AmmoUseCounter = TWEAPON_MACHINEGUN_USECNT;
		pCTMove->FireFirstMachineGunBullet = TRUE;
		if (CTMove__WeaponEmpty(pCTMove, AI_OBJECT_WEAPON_MACHINEGUN, 1))
		{
			if (LastWeapon)
				CTMove__SelectWeapon(pCTMove, TWEAPON_AMMO, FALSE);
			else
				CTMove__SelectWeapon(pCTMove, TWEAPON_AMMO, TRUE);

			CTMove__DisplayWeaponsIcons(pCTMove, TRUE);
		}
	}

	
	return FALSE;
}


// turoks riot shotgun
//
BOOL AI_Update_Weapon_RiotShotgun ( CGameObjectInstance *pMe, CTControl *pCTControl, CTMove *pCTMove, int waterFlag)
{
	CVector3 vPos;

	// declare variables
	CEngineApp *pApp;
	pApp = GetApp ();
	if ( pApp == NULL ) return FALSE;

	
	// using riot shotgun ?
	if ( CTControl__IsPrimaryFire ( pCTControl ) )
//	if ( CTControl__IsGrenadeLauncherFired ( pCTControl ) )
	{
		// fire riot shotgun
		pCTMove->AmmoUseCounter=0.0;
		CTMove__SetWeaponFiredStatus ( pCTMove, TWEAPON_RIOTSHOTGUN );
		if ( CTMove__WeaponEmpty ( pCTMove, AI_OBJECT_WEAPON_RIOTSHOTGUN, 1 ) )
		{
			if (LastWeapon)
				CTMove__SelectWeapon(pCTMove, TWEAPON_AMMO, FALSE);
			else
				CTMove__SelectWeapon(pCTMove, TWEAPON_AMMO, TRUE);

			CTMove__DisplayWeaponsIcons(pCTMove, TRUE);
			return FALSE;
		}

		vPos = AI_GetPos(pMe);
		CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_RIOT_SHOTGUN_SHOT, 0, NULL, &vPos, 0);

		AI_Event_Dispatcher(&pMe->ah.ih, &pMe->ah.ih, AI_MEVENT_LOUDNOISE, AI_SPECIES_ALL, AI_GetPos(pMe), 10);

		CTMove__FireBullet(pCTMove, TWEAPON_RIOTSHOTGUN);

		// if any explosive shells, use em first
		if (pCTMove->ExpShotgunPool)
			CTMove__DecreaseAmmo(pCTMove, &pCTMove->ExpShotgunPool, RIOTSHOTGUN_AMMO_USE);
		else
			CTMove__DecreaseAmmo(pCTMove, &pCTMove->ShotgunPool, RIOTSHOTGUN_AMMO_USE);

		CTMove__ShakeScreen(pCTMove, pApp, 5*SCALING_FACTOR/12, -(ANGLE_PI/80), TRUE);
		AI_SetDesiredAnim(pMe, AI_ANIM_WEAPON_FIRE1, TRUE);
		AI_GetDyn(pMe)->m_dwStatusFlags |= AI_WAITFOR_CYCLE;
		CGameObjectInstance__RestartAnim(pMe);
		return TRUE;
	}
	else
	{
		pCTMove->AmmoUseCounter=0.0;
		if (CTMove__WeaponEmpty(pCTMove, AI_OBJECT_WEAPON_RIOTSHOTGUN, 1))
		{
			if (LastWeapon)
				CTMove__SelectWeapon(pCTMove, TWEAPON_AMMO, FALSE);
			else
				CTMove__SelectWeapon(pCTMove, TWEAPON_AMMO, TRUE);

			CTMove__DisplayWeaponsIcons(pCTMove, TRUE);
		}
	}

	
	return FALSE;
}


// turoks automatic shotgun
//
BOOL AI_Update_Weapon_AutomaticShotgun ( CGameObjectInstance *pMe, CTControl *pCTControl, CTMove *pCTMove, int waterFlag)
{
	CVector3 vPos;

	// declare variables
	CEngineApp *pApp;
	pApp = GetApp ();
	if ( pApp == NULL ) return FALSE;

	
	// using automatic shotgun ?
	if ( CTControl__IsPrimaryFire ( pCTControl ) )
	{
		// fire automatic shotgun
		CTMove__WeaponBarrelFire (pCTMove, AI_OBJECT_WEAPON_AUTOMATICSHOTGUN);
		CTMove__SetWeaponFiredStatus ( pCTMove, TWEAPON_AUTOMATICSHOTGUN );
		if ( CTMove__WeaponEmpty ( pCTMove, AI_OBJECT_WEAPON_AUTOMATICSHOTGUN, 1 ) )
		{
			if (LastWeapon)
				CTMove__SelectWeapon(pCTMove, TWEAPON_AMMO, FALSE);
			else
				CTMove__SelectWeapon(pCTMove, TWEAPON_AMMO, TRUE);

			CTMove__DisplayWeaponsIcons(pCTMove, TRUE);
			return FALSE;
		}

		vPos = AI_GetPos(pMe);
		CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_AUTO_SHOTGUN_SHOT, 0, NULL, &vPos, 0);


		AI_Event_Dispatcher (&pMe->ah.ih, &pMe->ah.ih, AI_MEVENT_LOUDNOISE, AI_SPECIES_ALL, AI_GetPos(pMe), 10);

		CTMove__FireBullet   ( pCTMove, TWEAPON_AUTOMATICSHOTGUN );

		// if any explosive shells, use em first
		if (pCTMove->ExpShotgunPool)
			CTMove__DecreaseAmmo(pCTMove, &pCTMove->ExpShotgunPool, AUTOSHOTGUN_AMMO_USE);
		else
			CTMove__DecreaseAmmo(pCTMove, &pCTMove->ShotgunPool, AUTOSHOTGUN_AMMO_USE);

		CTMove__ShakeScreen  ( pCTMove, pApp, 5*SCALING_FACTOR/16, -(ANGLE_PI/120), TRUE );
		AI_SetDesiredAnim ( pMe, AI_ANIM_WEAPON_FIRE1, TRUE );
		AI_GetDyn(pMe)->m_dwStatusFlags |= AI_WAITFOR_CYCLE;
		CGameObjectInstance__RestartAnim(pMe);
		return TRUE;
	}
	else
	{
		if (CTMove__WeaponEmpty(pCTMove, AI_OBJECT_WEAPON_AUTOMATICSHOTGUN, 1))
		{
			if (LastWeapon)
				CTMove__SelectWeapon(pCTMove, TWEAPON_AMMO, FALSE);
			else
				CTMove__SelectWeapon(pCTMove, TWEAPON_AMMO, TRUE);

			CTMove__DisplayWeaponsIcons(pCTMove, TRUE);
		}
	}

	
	return FALSE;
}


// turoks mini gun
//
BOOL AI_Update_Weapon_MiniGun ( CGameObjectInstance *pMe, CTControl *pCTControl, CTMove *pCTMove, int waterFlag)
{
	CVector3 vPos;


	// using mini gun ?
	if ( CTControl__IsPrimaryFire ( pCTControl ) )
	{
		if (!pCTMove->MiniGunBarrelSFX)
		{

			// one time start barrel sfx & previous barrel sfx needs to be killed
			if (pCTMove->hSFXBarrel != -1)
			{
				killCFX(pCTMove->hSFXBarrel);
				pCTMove->hSFXBarrel = -1;
				killSoundType(SOUND_MINI_GUN_WHIR);
			}

			vPos = AI_GetPos(pMe);
			pCTMove->hSFXBarrel = CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_MINI_GUN_WHIR, 0, NULL, &vPos, 0);
			ASSERT(pCTMove->hSFXBarrel != -1);
			pCTMove->MiniGunBarrelSFX = TRUE;
		}

		CTMove__WeaponBarrelFire (pCTMove, AI_OBJECT_WEAPON_MINIGUN);

		// fire mini gun
		if ( ! CTMove__WeaponEmpty ( pCTMove, AI_OBJECT_WEAPON_MINIGUN, 1 ) )
		{
			CTMove__SetWeaponFiredStatus ( pCTMove, TWEAPON_MINIGUN );
			AI_Event_Dispatcher (&pMe->ah.ih, &pMe->ah.ih, AI_MEVENT_LOUDNOISE, AI_SPECIES_ALL, AI_GetPos(pMe), 10);
			AI_SetDesiredAnim ( pMe, AI_ANIM_WEAPON_GUNKICKBACK, TRUE );
			AI_GetDyn(pMe)->m_dwStatusFlags &= AI_WAITFOR_CYCLE;
		}
		else
		{
			// weapon did not fire any bullets but it did spin, so we must idle it depending on turok speed
			AI_Update_Weapon_Movements(pMe, pCTControl, pCTMove, waterFlag);
			pCTMove->WeaponMode = TWEAPONMODE_ON;
		}
		return TRUE;
	}
	else
	{
		pCTMove->AmmoUseCounter = 0.0;

		// play wind down sfx
		if(pCTMove->MiniGunBarrelSFX)
		{
			if (pCTMove->hSFXBarrel != -1)
			{
				killCFX(pCTMove->hSFXBarrel);
				pCTMove->hSFXBarrel = -1;
				killSoundType(SOUND_MINI_GUN_WHIR);
			}

			vPos = AI_GetPos(pMe);
			CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_MINIGUN_STOP, 0, NULL, &vPos, 0);
			//BarrelStop = 1;
		}

		// sfx needs to be restarted if trigger is pressed again
		pCTMove->MiniGunBarrelSFX = FALSE;

		// change to next weapon if there is no ammo left
		if (CTMove__WeaponEmpty(pCTMove, AI_OBJECT_WEAPON_MINIGUN, 1))
		{
			if (LastWeapon)
				CTMove__SelectWeapon(pCTMove, TWEAPON_AMMO, FALSE);
			else
				CTMove__SelectWeapon(pCTMove, TWEAPON_AMMO, TRUE);

			CTMove__DisplayWeaponsIcons(pCTMove, TRUE);

			// change to fade sfx to zero volume
			if (pCTMove->hSFXBarrel != -1)
			{
				killCFX(pCTMove->hSFXBarrel);
				pCTMove->hSFXBarrel = -1;
				killSoundType(SOUND_MINI_GUN_WHIR);
			}
		}



	}

	
	return FALSE;
}


// turoks grenade launcher
//
BOOL AI_Update_Weapon_Grenade_Launcher ( CGameObjectInstance *pMe, CTControl *pCTControl, CTMove *pCTMove, int waterFlag)
{
	// declare variables
	CEngineApp *pApp;
	CVector3		vPos;

	// get pointer to app
	pApp = GetApp ();
	if (pApp == NULL) return FALSE;


	// using machine gun ?
	if (CTControl__IsPrimaryFire(pCTControl))
	{
		// lob grendade
		CTMove__SetWeaponFiredStatus(pCTMove, TWEAPON_GRENADELAUNCHER);
		if (CTMove__WeaponEmpty(pCTMove, AI_OBJECT_WEAPON_GRENADE_LAUNCHER, 1))
		{
			if (LastWeapon)
				CTMove__SelectWeapon(pCTMove, TWEAPON_AMMO, FALSE);
			else
				CTMove__SelectWeapon(pCTMove, TWEAPON_AMMO, TRUE);

			CTMove__DisplayWeaponsIcons(pCTMove, TRUE);
			return FALSE;
		}

		vPos = AI_GetPos(pMe);
		CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GRENADE_LAUNCH, 0, NULL, &vPos, 0);

		AI_Event_Dispatcher (&pMe->ah.ih, &pMe->ah.ih, AI_MEVENT_LOUDNOISE, AI_SPECIES_ALL, AI_GetPos(pMe), 10);
		CTMove__DecreaseAmmo(pCTMove, &pCTMove->GrenadeLauncherAmmo, GRENADELAUNCHER_AMMO_USE);
		CTMove__ShakeScreen  ( pCTMove, pApp, 5*SCALING_FACTOR/12, -(ANGLE_PI/90), TRUE );
		CTMove__FireBullet   ( pCTMove, TWEAPON_GRENADELAUNCHER );
		AI_SetDesiredAnim ( pMe, AI_ANIM_WEAPON_FIRE1, TRUE );
		AI_GetDyn(pMe)->m_dwStatusFlags |= AI_WAITFOR_CYCLE;
		CGameObjectInstance__RestartAnim(pMe);
		return TRUE;
	}
	else
	{
		if (CTMove__WeaponEmpty(pCTMove, AI_OBJECT_WEAPON_GRENADE_LAUNCHER, 1))
		{
			if (LastWeapon)
				CTMove__SelectWeapon(pCTMove, TWEAPON_AMMO, FALSE);
			else
				CTMove__SelectWeapon(pCTMove, TWEAPON_AMMO, TRUE);

			CTMove__DisplayWeaponsIcons(pCTMove, TRUE);
		}
	}

	
	return FALSE;
}


// turoks tech weapon 1
//
BOOL AI_Update_Weapon_TechWeapon1 ( CGameObjectInstance *pMe, CTControl *pCTControl, CTMove *pCTMove, int waterFlag)
{
	CVector3 vPos;

	// using tech weapon 1 ?
	if ( CTControl__IsPrimaryFire ( pCTControl ) )
	{
		// fire tech weapon 1
		CTMove__SetWeaponFiredStatus ( pCTMove, TWEAPON_TECHWEAPON1 );
		if ( CTMove__WeaponEmpty ( pCTMove, AI_OBJECT_WEAPON_TECHWEAPON1, 1 ) )
		{	
			if (LastWeapon)
				CTMove__SelectWeapon(pCTMove, TWEAPON_AMMO, FALSE);
			else
				CTMove__SelectWeapon(pCTMove, TWEAPON_AMMO, TRUE);

			CTMove__DisplayWeaponsIcons(pCTMove, TRUE);
			return FALSE;
		}

		vPos = AI_GetPos(pMe);
		CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_TEK_WEAPON_1, 0, NULL, &vPos, 0);


		AI_Event_Dispatcher (&pMe->ah.ih, &pMe->ah.ih, AI_MEVENT_LOUDNOISE, AI_SPECIES_ALL, AI_GetPos(pMe), 10);

		CTMove__DecreaseAmmo ( pCTMove, &pCTMove->EnergyPool, TECH1_AMMO_USE);
		CTMove__FireBullet   ( pCTMove, TWEAPON_TECHWEAPON1 );
		AI_SetDesiredAnim ( pMe, AI_ANIM_WEAPON_FIRE1, TRUE );
		AI_GetDyn(pMe)->m_dwStatusFlags |= AI_WAITFOR_CYCLE;
		CGameObjectInstance__RestartAnim(pMe);
		return TRUE;
	}
	else
	{
		if (CTMove__WeaponEmpty(pCTMove, AI_OBJECT_WEAPON_TECHWEAPON1, 1))
		{
			if (LastWeapon)
				CTMove__SelectWeapon(pCTMove, TWEAPON_AMMO, FALSE);
			else
				CTMove__SelectWeapon(pCTMove, TWEAPON_AMMO, TRUE);

			CTMove__DisplayWeaponsIcons(pCTMove, TRUE);
		}
	}


	
	return FALSE;
}


// turoks rocket launcher
//
BOOL AI_Update_Weapon_Rocket ( CGameObjectInstance *pMe, CTControl *pCTControl, CTMove *pCTMove, int waterFlag)
{
	CVector3 vPos;

	// using rocket launcher ?
	if ( CTControl__IsPrimaryFire ( pCTControl ) )
	{
		// fire rocket launcher
		CTMove__SetWeaponFiredStatus ( pCTMove, TWEAPON_ROCKET );
		if ( CTMove__WeaponEmpty ( pCTMove, AI_OBJECT_WEAPON_ROCKET, 1 ) )
		{	
			if (LastWeapon)
				CTMove__SelectWeapon(pCTMove, TWEAPON_AMMO, FALSE);
			else
				CTMove__SelectWeapon(pCTMove, TWEAPON_AMMO, TRUE);

			CTMove__DisplayWeaponsIcons(pCTMove, TRUE);
			return FALSE;
		}

		vPos = AI_GetPos(pMe);

		CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_MISSILE_LAUNCH, 0, NULL, &vPos, 0);
		CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_RELOAD_MISSLE_LAUNCHER, 0, NULL, &vPos, 0);

		AI_Event_Dispatcher (&pMe->ah.ih, &pMe->ah.ih, AI_MEVENT_LOUDNOISE, AI_SPECIES_ALL, AI_GetPos(pMe), 10);

		CTMove__DecreaseAmmo ( pCTMove, &pCTMove->RocketAmmo, ROCKET_AMMO_USE);
		CTMove__FireBullet   ( pCTMove, TWEAPON_ROCKET );

		if (CTMove__WeaponEmpty(pCTMove, AI_OBJECT_WEAPON_ROCKET, 1))
		{
			// no more rockets
			if (LastWeapon)
				CTMove__SelectWeapon(pCTMove, TWEAPON_AMMO, FALSE);
			else
				CTMove__SelectWeapon(pCTMove, TWEAPON_AMMO, TRUE);

			CTMove__DisplayWeaponsIcons(pCTMove, TRUE);
			pCTMove->WeaponMode = TWEAPONMODE_GOINGOFF_LASTBULLET;
			return TRUE;
		}

		AI_SetDesiredAnim ( pMe, AI_ANIM_WEAPON_FIRE1, TRUE );
		AI_GetDyn(pMe)->m_dwStatusFlags |= AI_WAITFOR_CYCLE;
		CGameObjectInstance__RestartAnim(pMe);
		return TRUE;
	}
	else
	{
		if (CTMove__WeaponEmpty(pCTMove, AI_OBJECT_WEAPON_ROCKET, 1))
		{
			if (LastWeapon)
				CTMove__SelectWeapon(pCTMove, TWEAPON_AMMO, FALSE);
			else
				CTMove__SelectWeapon(pCTMove, TWEAPON_AMMO, TRUE);

			CTMove__DisplayWeaponsIcons(pCTMove, TRUE);
		}
	}

	
	return FALSE;
}


// turoks shockwave
//
BOOL AI_Update_Weapon_Shockwave(CGameObjectInstance *pMe, CTControl *pCTControl, CTMove *pCTMove, int waterFlag)
{
	// declare variables
	CVector3			vOffset,
						vParticlePos,
						vBaseVelocity,
						vPos;
	CQuatern			qRotateDirection;
	CGameRegion		*pParticleRegion;
	int				nAmt,
						lev;
	float				time;


	vPos = AW.Ears.vPos;

	// using shockwave ?
	CTMove__WeaponBarrelFire (pCTMove, AI_OBJECT_WEAPON_SHOCKWAVE);
	if ( CTControl__IsPrimaryFire ( pCTControl ) )
	{
		// turok is using the shockwave, any pulse's left ?
		CTMove__SetWeaponFiredStatus ( pCTMove, TWEAPON_SHOCKWAVE );
		if ( CTMove__WeaponEmpty ( pCTMove, AI_OBJECT_WEAPON_SHOCKWAVE, 1 ) ) return FALSE;

		// first time user has pressed trigger ?
		if (pCTMove->ShockwaveStarted == 0.0)
		{
			// turok has just pressed trigger for shockwave (shockwave now starts to charge up)
			AI_SetDesiredAnim ( pMe, AI_ANIM_WEAPON_FIRE1, TRUE );			// charge cycle selected
			AI_GetDyn(pMe)->m_dwStatusFlags &= AI_WAITFOR_CYCLE;
			pCTMove->ShockwaveStarted += frame_increment*(1.0/FRAME_FPS);

			pCTMove->hSFXPrevShockwaveCharge = CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_160, 0, NULL, &vPos, 0);
			pCTMove->hSFXNextShockwaveCharge = -1;
		}
		else
		{
			// generate appropriate charge sfx
			if (pCTMove->ShockwaveStarted<1.0)
			{
				if (pCTMove->hSFXPrevShockwaveCharge == -1)
					pCTMove->hSFXPrevShockwaveCharge = CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_160, 0, NULL, &vPos, 0);
				pCTMove->hSFXNextShockwaveCharge = -1;
			}
			else if (pCTMove->ShockwaveStarted<2.0)
			{
				pCTMove->hSFXPrevShockwaveCharge = -1;

				if (pCTMove->hSFXNextShockwaveCharge == -1)
					pCTMove->hSFXNextShockwaveCharge = CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_161, 0, NULL, &vPos, 0);
			}
			else if (pCTMove->ShockwaveStarted<3.0)
			{
				pCTMove->hSFXNextShockwaveCharge = -1;

				if (pCTMove->hSFXPrevShockwaveCharge == -1)
					pCTMove->hSFXPrevShockwaveCharge = CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_162, 0, NULL, &vPos, 0);
			}
			else if (pCTMove->ShockwaveStarted<4.0)
			{
				pCTMove->hSFXPrevShockwaveCharge = -1;

				if (pCTMove->hSFXNextShockwaveCharge == -1)
					pCTMove->hSFXNextShockwaveCharge = CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_163, 0, NULL, &vPos, 0);
			}
			else if (pCTMove->ShockwaveStarted>=4.0)
			{
				pCTMove->hSFXNextShockwaveCharge = -1;

				if (pCTMove->hSFXPrevShockwaveCharge == -1)
					pCTMove->hSFXPrevShockwaveCharge = CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_164, 0, NULL, &vPos, 0);
			}

			// turok is still pressing shockwave trigger
			pCTMove->ShockwaveStarted += frame_increment*(1.0/FRAME_FPS);
			if (RANDOM(100) <= (pCTMove->ShockwaveStarted*FRAME_FPS))
			{
				vOffset.x = 0.4*SCALING_FACTOR;
				vOffset.y = -1.4*SCALING_FACTOR;
				vOffset.z = 2.5*SCALING_FACTOR;
				qRotateDirection = CGameObjectInstance__GetAimRotation(pMe);
				vBaseVelocity.x = vBaseVelocity.y = vBaseVelocity.z = 0;

				// get particle position
				CGameObjectInstance__GetOffsetPositionAndRegion(CEngineApp__GetPlayer(GetApp()),
																				vOffset,
																				&vParticlePos,
																				&pParticleRegion);

				// create particle
				CParticleSystem__CreateParticle(&GetApp()->m_Scene.m_ParticleSystem, &pMe->ah.ih, PARTICLE_TYPE_SHOCKWAVE_CHARGE,
														  vBaseVelocity, qRotateDirection,
														  vParticlePos, pParticleRegion, PARTICLE_NOEDGE, TRUE);
			}
		}

		return TRUE;
	}
	else if (pCTMove->ShockwaveStarted > 0.0)
	{
		// remove charge sfx
		if (pCTMove->hSFXPrevShockwaveCharge != -1)
		{
			killCFX(pCTMove->hSFXPrevShockwaveCharge);
			pCTMove->hSFXPrevShockwaveCharge = -1;
		}

		if (pCTMove->hSFXNextShockwaveCharge != -1)
		{
			killCFX(pCTMove->hSFXNextShockwaveCharge);
			pCTMove->hSFXNextShockwaveCharge = -1;
		}

		// fire a shockwave pulse
		time = pCTMove->ShockwaveStarted;
		if (time >= 4.0)
			time = 4.0;

		lev = (int)(time + 1.0);
		do
		{
			if (!CTMove__WeaponEmpty(pCTMove, AI_OBJECT_WEAPON_SHOCKWAVE, lev))
				break;

			lev --;

		} while(lev >= 1);

		switch(lev)
		{
			case 5:
				nAmt = SHOCKWAVE_AMMO_USE5;
				break;

			case 4:
				nAmt = SHOCKWAVE_AMMO_USE4;
				break;

			case 3:
				nAmt = SHOCKWAVE_AMMO_USE3;
				break;

			case 2:
				nAmt = SHOCKWAVE_AMMO_USE2;
				break;

			default:
				nAmt = SHOCKWAVE_AMMO_USE1;
				break;
		}

		CTMove__DecreaseAmmo(pCTMove, &pCTMove->EnergyPool, nAmt);
		CTMove__FireBullet(pCTMove, TWEAPON_SHOCKWAVE);
		AI_SetDesiredAnim(pMe, AI_ANIM_WEAPON_FIRE2, TRUE);
		AI_GetDyn(pMe)->m_dwStatusFlags |= AI_WAITFOR_CYCLE;
		CGameObjectInstance__RestartAnim(pMe);
		pCTMove->ShockwaveStarted = 0.0;
		if (CTMove__WeaponEmpty(pCTMove, AI_OBJECT_WEAPON_SHOCKWAVE, 1))
		{
			// change to next weapon
			pCTMove->WeaponSelect = pCTMove->WeaponCurrent;

			if (LastWeapon)
				CTMove__SelectWeapon(pCTMove, TWEAPON_AMMO, FALSE);
			else
				CTMove__SelectWeapon(pCTMove, TWEAPON_AMMO, TRUE);

			CTMove__DisplayWeaponsIcons(pCTMove, TRUE);
		}
		return TRUE;
	}

	return FALSE;
}


// turoks tech weapon 2
//
BOOL AI_Update_Weapon_TechWeapon2 ( CGameObjectInstance *pMe, CTControl *pCTControl, CTMove *pCTMove, int waterFlag)
{
	CVector3 vPos;

	// using tech weapon 2 ?
	if ( CTControl__IsPrimaryFire ( pCTControl ) )
	{
		// fire tech weapon 2
		pCTMove->AmmoUseCounter=0.0;
		CTMove__SetWeaponFiredStatus ( pCTMove, TWEAPON_TECHWEAPON2 );
		if ( CTMove__WeaponEmpty ( pCTMove, AI_OBJECT_WEAPON_TECHWEAPON2, 1 ) )
		{
			if (LastWeapon)
				CTMove__SelectWeapon(pCTMove, TWEAPON_AMMO, FALSE);
			else
				CTMove__SelectWeapon(pCTMove, TWEAPON_AMMO, TRUE);

			CTMove__DisplayWeaponsIcons(pCTMove, TRUE);
			return FALSE;
		}

		vPos = AI_GetPos(pMe);
		CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_TEK_WEAPON_2, 0, NULL, &vPos, 0);


		AI_Event_Dispatcher (&pMe->ah.ih, &pMe->ah.ih, AI_MEVENT_LOUDNOISE, AI_SPECIES_ALL, AI_GetPos(pMe), 10);
		AI_SetDesiredAnim ( pMe, AI_ANIM_WEAPON_FIRE1, TRUE );
		AI_GetDyn(pMe)->m_dwStatusFlags |= AI_WAITFOR_CYCLE;
		CGameObjectInstance__RestartAnim(pMe);
		return TRUE;
	}
	else
	{
		if (CTMove__WeaponEmpty(pCTMove, AI_OBJECT_WEAPON_TECHWEAPON2, 1))
		{
			if (LastWeapon)
				CTMove__SelectWeapon(pCTMove, TWEAPON_AMMO, FALSE);
			else
				CTMove__SelectWeapon(pCTMove, TWEAPON_AMMO, TRUE);

			CTMove__DisplayWeaponsIcons(pCTMove, TRUE);
		}

	}

	
	return FALSE;
}




// turoks chronoscepter
//
BOOL AI_Update_Weapon_Chronoscepter(CGameObjectInstance *pMe, CTControl *pCTControl, CTMove *pCTMove, int waterFlag)
{
	// declare variables
	CVector3			vOffset,
						vParticlePos,
						vBaseVelocity,
						vPos;
	CQuatern			qRotateDirection;
	CGameRegion		*pParticleRegion;

	
	vPos = AW.Ears.vPos;

	// using chronoscepter ?
	if (CTControl__IsPrimaryFire(pCTControl))
	{
		// turok is using the chronoscepter, any pulse's left ?
		CTMove__SetWeaponFiredStatus(pCTMove, TWEAPON_CHRONOSCEPTER);
		if (CTMove__WeaponEmpty(pCTMove, AI_OBJECT_WEAPON_CHRONOSCEPTER, 1)) return FALSE;

		// first time user has pressed trigger ?
		if (pCTMove->ChronoscepterStarted == 0.0)
		{
			// turok has just pressed trigger for chronoscepter (chronoscepter now starts to charge up)
			AI_SetDesiredAnim(pMe, AI_ANIM_WEAPON_FIRE1, TRUE);			// charge cycle selected
			AI_GetDyn(pMe)->m_dwStatusFlags &= AI_WAITFOR_CYCLE;
			pCTMove->ChronoscepterStarted += frame_increment*(1.0/FRAME_FPS);
		}
		else
		{
			// turok is still pressing chronoscepter trigger
			pCTMove->ChronoscepterStarted += frame_increment*(1.0/FRAME_FPS);
			if (RANDOM(100) <= (pCTMove->ChronoscepterStarted*FRAME_FPS))
			{
				vOffset.x = 0.4*SCALING_FACTOR;
				vOffset.y = -1.4*SCALING_FACTOR;
				vOffset.z = 2.5*SCALING_FACTOR;

				qRotateDirection = CGameObjectInstance__GetAimRotation(pMe);
				vBaseVelocity.x = vBaseVelocity.y = vBaseVelocity.z = 0;

				// get particle position
				CGameObjectInstance__GetOffsetPositionAndRegion(CEngineApp__GetPlayer(GetApp()),
																				vOffset,
																				&vParticlePos,
																				&pParticleRegion);

				// create particle
				CParticleSystem__CreateParticle(&GetApp()->m_Scene.m_ParticleSystem, &pMe->ah.ih, PARTICLE_TYPE_CHRONOSCEPTERCHARGE,
														  vBaseVelocity, qRotateDirection,
														  vParticlePos, pParticleRegion, PARTICLE_NOEDGE, TRUE);
			}
		}

		return TRUE;
	}
	else if (pCTMove->ChronoscepterStarted >= TMOVE_CHRONOSCEPTER_CHARGE_TIME)
	{
		// fire a chronoscepter pulse
		CTMove__DecreaseAmmo(pCTMove, &pCTMove->ChronoscepterAmmo, CHRONOSCEPTER_AMMO_USE);
		CTMove__FireBullet(pCTMove, TWEAPON_CHRONOSCEPTER);
		AI_SetDesiredAnim(pMe, AI_ANIM_WEAPON_FIRE2, TRUE);
		AI_GetDyn(pMe)->m_dwStatusFlags |= AI_WAITFOR_CYCLE;
		CGameObjectInstance__RestartAnim(pMe);
		pCTMove->ChronoscepterStarted = 0.0;
		CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_SHOCKWAVE_WEAPON_FIRE, 0, NULL, &vPos, 0);

		if (CTMove__WeaponEmpty(pCTMove, AI_OBJECT_WEAPON_CHRONOSCEPTER, 1))
		{
			// change to next weapon
			pCTMove->WeaponSelect = pCTMove->WeaponCurrent;

			if (LastWeapon)
				CTMove__SelectWeapon(pCTMove, TWEAPON_AMMO, FALSE);
			else
				CTMove__SelectWeapon(pCTMove, TWEAPON_AMMO, TRUE);

			CTMove__DisplayWeaponsIcons(pCTMove, TRUE);
		}
		return TRUE;
	}
	else if (		(pCTMove->ChronoscepterStarted > 0.0)
					&& (pCTMove->ChronoscepterStarted < TMOVE_CHRONOSCEPTER_CHARGE_TIME) )
	{
		// chronoscepter was not held in long enough
		AI_SetDesiredAnim(pMe, AI_ANIM_WEAPON_IDLE, TRUE);
		AI_GetDyn(pMe)->m_dwStatusFlags |= AI_WAITFOR_CYCLE;
		CGameObjectInstance__RestartAnim(pMe);
		pCTMove->ChronoscepterStarted = 0.0;
	}

	return FALSE;
}




// kill weapon sfx's
//
void AI_Kill_Weapon_SFX(CTMove *pThis)
{
	// stop minigun barrel
	if (pThis->hSFXBarrel != -1)
	{
		killCFX(pThis->hSFXBarrel);
		pThis->hSFXBarrel = -1;
		killSoundType(SOUND_MINI_GUN_WHIR);
	}

	// stop shockwave charge sfx
	if (pThis->hSFXPrevShockwaveCharge != -1)
	{
		killCFX(pThis->hSFXPrevShockwaveCharge);
		pThis->hSFXPrevShockwaveCharge = -1;
	}

	if (pThis->hSFXNextShockwaveCharge != -1)
	{
		killCFX(pThis->hSFXNextShockwaveCharge);
		pThis->hSFXNextShockwaveCharge = -1;
	}
}



#endif

