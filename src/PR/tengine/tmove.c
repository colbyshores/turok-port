// tmove.cpp : turok movement
//

#include "stdafx.h"
#include <ramrom.h>
#include "tmove.h"
#include "scene.h"
#include "particle.h"
#include "audio.h"
#include "sfx.h"
#include "tengine.h"
#include "regtype.h"
#include "pickup.h"
#include "map.h"
#include "train.h"
#include "collinfo.h"
#include "attract.h"
#include "cammodes.h"


//-------------------------------------------------------------------------
// Text
//-------------------------------------------------------------------------

#ifdef ENGLISH
// ---------------------- English text ---------------------------
static char	text_checkpoint[] = {"checkpoint"} ;
static char	text_doyouwishto[] = {"do you wish to"} ;
static char	text_saveyourgame[] = {"save your game"} ;
									 
static char	text_access2[] = {"access to level 2"} ;
static char	text_jungle[] = {"the jungle"} ;
static char	text_access3[] = {"access to level 3"} ;
static char	text_city[] = {"the ancient city"};
static char	text_access4[] = {"access to level 4"} ;
static char	text_ruins[] = {"the ruins"};
static char	text_access5[] = {"access to level 5"} ;
static char	text_catacombs[] = {"the catacombs"};
static char	text_access6[] = {"access to level 6"} ;
static char	text_treetop[] = {"the treetop village"};
static char	text_access7[] = {"access to level 7"} ;
static char	text_lostland[] = {"the lost land"};
static char	text_access8[] = {"access to level 8"} ;
static char	text_finallevel[] = {"the final confrontation"};
static char text_extralife[] = {"extra life"} ;
#endif

#ifdef GERMAN
// ---------------------- German text ---------------------------
static char	text_checkpoint[] = {"checkpoint"} ;
static char	text_doyouwishto[] = {"m>chtest du das"} ;		//o.
static char	text_saveyourgame[] = {"spiel sichern"} ;
									 
static char	text_access2[] = {"zugang zu level 2"} ;
static char	text_jungle[] = {"der dschungel"} ;
static char	text_access3[] = {"zugang zu level 3"} ;
static char	text_city[] = {"die alte stadt"};
static char	text_access4[] = {"zugang zu level 4"} ;
static char	text_ruins[] = {"die ruinen"};
static char	text_access5[] = {"zugang zu level 5"} ;
static char	text_catacombs[] = {"die katakomben"};
static char	text_access6[] = {"zugang zu level 6"} ;
static char	text_treetop[] = {"das dorf in den baumwipfeln"};
static char	text_access7[] = {"zugang zu level 7"} ;
static char	text_lostland[] = {"das verlorene land"};
static char	text_access8[] = {"zugang zu level 8"} ;
static char	text_finallevel[] = {"die letzte konfrontation"};
static char text_extralife[] = {"1 leben"} ;
#endif

#ifdef KANJI
// ---------------------- Japanese text ---------------------------
static char	text_checkpoint[] = {0x07, 0x25, 0x17, 0x38, 0x01, 0x27, 0x13, -1};
static char	text_doyouwishto[] = {0x2A, 0x41, 0x1D, 0x74, -1};
static char	text_saveyourgame[] = {0x0D, 0x41, 0x32, 0x57, 0x68, 0x58, 0x51, -1};

static char	text_access2[] = {0x2A, 0x41, 0x13, 0x43, -1};
static char	text_jungle[] = {0xB1, 0xB2, -1};
static char	text_access3[] = {0x2A, 0x41, 0x13, 0x44, -1};
static char	text_city[] = {0xB3, 0xB4, 0xB5, 0xB6, -1};
static char	text_access4[] = {0x2A, 0x41, 0x13, 0x45, -1};
static char	text_ruins[] = {0x99, 0x9A, -1};
static char	text_access5[] = {0x2A, 0x41, 0x13, 0x46, -1};
static char	text_catacombs[] = {0xB7, 0x96, 0xB8, -1};
static char	text_access6[] = {0x2A, 0x41, 0x13, 0x47, -1};
static char	text_treetop[] = {0xB9, 0xBA, 0xBB, -1};
static char	text_access7[] = {0x2A, 0x41, 0x13, 0x48, -1};
static char	text_lostland[] = {0xBC, 0x95, -1};
static char	text_access8[] = {0x2A, 0x41, 0x13, 0x49, -1};
static char	text_finallevel[] = {0x94, 0x95, -1};
static char text_extralife[] = {0x42, 0xeb, 0x00, 0x40, 0x36, -1} ;
#endif


// THIS IS ID DEFINE IS USED TO TEST BOSS LEVELS AND SUCH - LEAVE IT ALONE
// BUT PUT IT TO 0 FOR THE FINAL SHIP VERSION!!!!!!!

#define THE_BIG_WHOLE_GAME_START_WARP_ID	0		// Start of game
//#define THE_BIG_WHOLE_GAME_START_WARP_ID		2998	// Longhunter
//#define THE_BIG_WHOLE_GAME_START_WARP_ID		4999	// Mantis
//#define THE_BIG_WHOLE_GAME_START_WARP_ID		8997	// Trex
//#define THE_BIG_WHOLE_GAME_START_WARP_ID		8999	// Campaigner



#define GetSlowFastSlowBlender(u) GetCosineBlender(u)
#define GetSlowFastBlender(u) GetCosineBlender(((u)*0.5)*2.0)
//#define	TMOVE_ALLOW_WEAPON_WHILE_CLIMBING	// allows weapon use on cliffs
//#define	ALLOW_UPSIDEDOWN_UNDERWATER			// allows player to go upside down while under the water


float GetFastSlowBlender(float u)
{
	float v;

	v = 1 - u;

	return 1 - v*v*v;
}

void		CTMove__FlyMode(CTMove *pThis, CEngineApp *pApp, CTControl *pCTControl);
CVector3	CTMove__ControlFlyFBward(CTMove *pThis, CEngineApp *pApp, CTControl *pCTControl, CVector3 vDesiredPos);
CVector3 CTMove__ControlFlySideStep(CTMove *pThis, CEngineApp *pApp, CTControl *pCTControl, CVector3 vDesiredPos);
CVector3 CTMove__ControlFlyUpDown(CTMove *pThis, CEngineApp *pApp, CTControl *pCTControl, CVector3 vDesiredPos);

// swimming
#define	TMOVE_WATER_FLOAT_FORCE			(1.7 *SCALING_FACTOR)
#define	TMOVE_WATER_SWIMMING_FORCE		(0.3 *SCALING_FACTOR)
#define	TMOVE_SWIM_STOP_SMOOTH			3.0
#define	TMOVE_OXYGEN_AIRGULP_TIMER		70								// in seconds
#define	TMOVE_OXYGEN_DISPLAY_TIMER		15								// in seconds

// turok swim defines
#define	TMOVE_MAX_BURSTFORWARDSPEEDUW	(2.2 *SCALING_FACTOR)
#define	TMOVE_MAX_BURSTFORWARDSPEEDOW	(2.0 *SCALING_FACTOR)
#define	TMOVE_MAX_SWIMSPEED				(1.6 *SCALING_FACTOR)
#define	TMOVE_MAX_BACKSWIMSPEED			(1.0 *SCALING_FACTOR)
#define	TMOVE_MAX_SIDESTEPSWIMSPEED	(2.3 *SCALING_FACTOR)
#define	TMOVE_MAX_UPSWIMSPEED			(2.2 *SCALING_FACTOR)	// 1.8
#define	TMOVE_SWIM_SMOOTH					(5.0 *SCALING_FACTOR)

#ifdef PLATFORM_PORT
/* PORT (PC walk/run toggle — bound to E in the SDL2 backend): the keyboard moves on the digital
 * C-buttons which always hit RUN speed; when walk-mode is on, scale the run-speed CAP so the player
 * accelerates to a slower top speed (forward/back/strafe alike). g_turok_walk_mode lives in the
 * SDL2 input backend. */
extern int g_turok_walk_mode;
#define	TUROK_WALKCAP(x)	((g_turok_walk_mode) ? (x)*0.5f : (x))
/* PORT (scroll-wheel weapon cycle): discrete signed notch accumulator fed by the SDL2 backend, consumed
 * ONE step per LOGIC TICK (see the weapon-select block) so each notch = exactly one weapon regardless of
 * render rate. TUROK_IS_TICK is false on render-only frames (frame_increment==0 at FPS>TICK) — used to
 * gate the held-button weapon-switch so it can't over-cycle between ticks. */
extern int g_weapon_cycle;
#define	TUROK_IS_TICK		(frame_increment != 0.0f)
#else
#define	TUROK_WALKCAP(x)	(x)
#define	TUROK_IS_TICK		1
#endif



// private members
CTMove	CTurokMovement;
#define	JUMP_TILT_SECONDS		0.5
#define	JUMP_TILT_ANGLE		23

#define	TMOVE_START_LIVES	2
#define	TMOVE_MAX_LIVES	9


// weapon select order
int WeaponOrder[]= {AI_OBJECT_WEAPON_KNIFE,
						   AI_OBJECT_WEAPON_TOMAHAWK,
							AI_OBJECT_WEAPON_TEKBOW,
							AI_OBJECT_WEAPON_SEMIAUTOMATICPISTOL,
							AI_OBJECT_WEAPON_RIOTSHOTGUN,
							AI_OBJECT_WEAPON_AUTOMATICSHOTGUN,
							AI_OBJECT_WEAPON_ASSAULTRIFLE,
							AI_OBJECT_WEAPON_MACHINEGUN,
							AI_OBJECT_WEAPON_MINIGUN,
							AI_OBJECT_WEAPON_GRENADE_LAUNCHER,
							AI_OBJECT_WEAPON_TECHWEAPON1,
							AI_OBJECT_WEAPON_ROCKET,
							AI_OBJECT_WEAPON_SHOCKWAVE,
							AI_OBJECT_WEAPON_TECHWEAPON2,
							AI_OBJECT_WEAPON_CHRONOSCEPTER,
							-1				};



// initialize turok movement object - start of game only
//
CTMove *CTMove__CTMove(void)
{
	// initialize members of turok movement object
	CTurokMovement.Tokens							= 0;
	CTurokMovement.Lives								= TMOVE_START_LIVES;

	CTurokMovement.WaterFlag						= 0;
	CTurokMovement.RunWalkToggle					= -1;			// start in run mode
	CTurokMovement.MusicID							= 255;
	CTurokMovement.AmbientSound					= REG_AMBIENTNONE;

	// starting weapon (should be one of the following three weapons that he always has)
	CTurokMovement.WeaponSelect					= AI_OBJECT_WEAPON_KNIFE;
	CTurokMovement.WeaponSelectIndex				= 0;
	CTurokMovement.WeaponCurrent					= AI_OBJECT_WEAPON_KNIFE;

	// weapon ammo counts
	CTurokMovement.BulletPool						= 0;
	CTurokMovement.ShotgunPool						= 0;
	CTurokMovement.EnergyPool						= 0;
	CTurokMovement.ExpTekBowAmmo					= 0;
	CTurokMovement.ExpShotgunPool 				= 0;
	CTurokMovement.KnifeAmmo						= -1;	// infinite use of weapon
	CTurokMovement.TomahawkAmmo					= -1;	// infinite use of weapon
	CTurokMovement.TekBowAmmo						= MAX_ARROWS;
	CTurokMovement.MiniGunAmmo						= 0;
	CTurokMovement.GrenadeLauncherAmmo			= 0;
	CTurokMovement.RocketAmmo						= 0;
	CTurokMovement.TechWeapon2Ammo				= 0;
	CTurokMovement.ChronoscepterAmmo				= 0;

	// is all weapon cheat mode on ? - if so give player all ammo once at start of game
	if (GetApp()->m_dwCheatFlags & CHEATFLAG_ALLWEAPONS)
	{
		CTurokMovement.TekBowAmmo						= MAX_ARROWS;
		CTurokMovement.BulletPool						= MAX_BULLETS;
		CTurokMovement.EnergyPool						= MAX_ENERGY;
		CTurokMovement.ShotgunPool						= MAX_SHOTGUN;
		CTurokMovement.MiniGunAmmo						= MAX_MINIGUN;
		CTurokMovement.GrenadeLauncherAmmo			= MAX_GRENADES;
		CTurokMovement.RocketAmmo						= MAX_ROCKETS;
		CTurokMovement.TechWeapon2Ammo				= MAX_TECH2;
		CTurokMovement.ChronoscepterAmmo				= MAX_CHRONOSCEPTER;
		CTurokMovement.AllWeaponsAmmoGiven			= TRUE;
	}
	else
	{
		CTurokMovement.AllWeaponsAmmoGiven			= FALSE;
	}

	CTurokMovement.MaxHealth = AI_HEALTH_PLAYER ;
	CTurokMovement.CheatGiven = FALSE ;

	// does turok possess this weapon (flag)
	CTurokMovement.KnifeFlag						= TRUE;	// turok always has this weapon
																				// - some logic relies on this
																				// weapon being present
																				// (remove at your peril)
	CTurokMovement.TomahawkFlag					= FALSE;
	CTurokMovement.TekBowFlag						= TRUE;	// always has TEKBOW also!!

	CTurokMovement.SemiAutomaticPistolFlag		= FALSE;	// these are now off as default
	CTurokMovement.AssaultRifleFlag				= FALSE;	// so we can get pickups working...
	CTurokMovement.MachineGunFlag					= FALSE;
	CTurokMovement.RiotShotgunFlag				= FALSE;
	CTurokMovement.AutomaticShotgunFlag			= FALSE;
	CTurokMovement.MiniGunFlag						= FALSE;
	CTurokMovement.GrenadeLauncherFlag			= FALSE;
	CTurokMovement.TechWeapon1Flag				= FALSE;
	CTurokMovement.RocketFlag						= FALSE;
	CTurokMovement.ShockwaveFlag					= FALSE;
	CTurokMovement.TechWeapon2Flag				= FALSE;
	CTurokMovement.ChronoscepterFlag				= FALSE;

	CTurokMovement.ArmorFlag						= FALSE;
	CTurokMovement.ArmorAmount						= 0 ;
	CTurokMovement.BackPackFlag					= FALSE;
	CTurokMovement.SpiritualTime					= 0 ;

	CTurokMovement.LastKeyTexture = 0 ;

	CTurokMovement.Level2Keys = 0 ;
	CTurokMovement.Level3Keys = 0 ;
	CTurokMovement.Level4Keys = 0 ;
	CTurokMovement.Level5Keys = 0 ;
	CTurokMovement.Level6Keys = 0 ;
	CTurokMovement.Level7Keys = 0 ;
	CTurokMovement.Level8Keys = 0 ;

	CTurokMovement.Level2Access = 0 ;
	CTurokMovement.Level3Access = 0 ;
	CTurokMovement.Level4Access = 0 ;
	CTurokMovement.Level5Access = 0 ;
	CTurokMovement.Level6Access = 0 ;
	CTurokMovement.Level7Access = 0 ;
	CTurokMovement.Level8Access = 0 ;

	CTurokMovement.ChronoSceptorPieces = 0 ;

	// Setup start id
	GetApp()->m_WarpID = THE_BIG_WHOLE_GAME_START_WARP_ID	;
	CTurokMovement.CurrentCheckpoint = THE_BIG_WHOLE_GAME_START_WARP_ID	;

	// starting weapon (should be one of the following three weapons that he always has)
//	CTurokMovement.WeaponSelect				= AI_OBJECT_WEAPON_KNIFE;
//	CTurokMovement.WeaponSelectIndex			= 0;

	// return pointer to turok movement object
	return &CTurokMovement;
}


void CTMove__UpdateWarpSound(CTMove *pThis)
{
	static CVector3 vWarpPos ;
	CGameObjectInstance *pTurok = CEngineApp__GetPlayer(GetApp()) ;
	INT32	WarpID = GetApp()->m_CurrentWarpID ;

	// Play sound?
	if ((WarpID >= 9000) && (WarpID < 9999))
	{
		// Set sound position
		vWarpPos = pTurok->ah.ih.m_vPos ;
		vWarpPos.x += 15*SCALING_FACTOR * sin(ANGLE_DTOR(game_frame_number*6) + pTurok->m_RotY) ;
		vWarpPos.z += 15*SCALING_FACTOR * cos(ANGLE_DTOR(game_frame_number*7) + pTurok->m_RotY) ;

		// Play sound
		CScene__DoSoundEffect(&GetApp()->m_Scene,
									 SOUND_GENERIC_185, 0,
									 pTurok, &vWarpPos, 0) ;
	}
}


// initialize turok new life
//
void CTMove__NewLifeSetup(CTMove *pThis)
{
	CGameObjectInstance		*pPlayer;

	// lose backpack and cap off any extra ammo stored in it
	pThis->BackPackFlag = FALSE ;
	if (pThis->BulletPool > MAX_BULLETS)
		pThis->BulletPool = MAX_BULLETS ;
	if (pThis->ShotgunPool > MAX_SHOTGUN)
		pThis->ShotgunPool = MAX_SHOTGUN ;
	if (pThis->ExpShotgunPool > MAX_EXPSHOTGUN)
		pThis->ExpShotgunPool = MAX_EXPSHOTGUN ;
	if (pThis->EnergyPool > MAX_ENERGY)
		pThis->EnergyPool = MAX_ENERGY ;
	if (pThis->ExpTekBowAmmo > MAX_EXPARROWS)
		pThis->ExpTekBowAmmo = MAX_EXPARROWS ;
	if (pThis->TekBowAmmo > MAX_ARROWS)
		pThis->TekBowAmmo = MAX_ARROWS ;
	if (pThis->MiniGunAmmo > MAX_MINIGUN)
		pThis->MiniGunAmmo = MAX_MINIGUN ;
	if (pThis->GrenadeLauncherAmmo > MAX_GRENADES)
		pThis->GrenadeLauncherAmmo = MAX_GRENADES ;
	if (pThis->RocketAmmo > MAX_ROCKETS)
		pThis->RocketAmmo = MAX_ROCKETS ;
	if (pThis->TechWeapon2Ammo > MAX_TECH2)
		pThis->TechWeapon2Ammo = MAX_TECH2 ;
	if (pThis->ChronoscepterAmmo > MAX_CHRONOSCEPTER)
		pThis->ChronoscepterAmmo = MAX_CHRONOSCEPTER ;

	// initialize members of turok movement object
	pThis->Mode								= TMMODE_GROUND;			// normal ground movement
	pThis->PrevMode						= TMMODE_GROUND;			// normal ground movement
	pThis->MoveSpeed						= 0;							// no movement speed
	pThis->TurnSpeed						= 0;							// no turn speed
	pThis->ActualTurnSpeed				= 0;							// no turn speed
	pThis->MoveCnt							= 0;							// reset move counter
	pThis->YRotCnt							= 0;							// reset move counter
	pThis->HeadXRot						= 0;							// desired head x rotation offset for head movement
	pThis->HeadYRot						= 0;							// desired head y rotation offset for head movement
	pThis->HeadZRot						= 0;							// desired head z rotation offset for head movement
	pThis->HeadYPos						= 0;							// desired head y position offset for head movement
	pThis->HeadYPosCFrame				= 0;
	pThis->SpdChgXOffset					= 0;
	pThis->SpdChgX    					= 0;
	pThis->SpdChg     					= FALSE;
	pThis->RunTime    					= 0;
	pThis->SideStepSpeed					= 0;
	pThis->HeadZRoll						= 0;
	pThis->HeadZRollCnt					= 0;
	pThis->HeadYPosLand					= 0;
	pThis->JumpUpForce					= 0;
	pThis->JumpLookAng					= 0;
	pThis->WeaponUsed						= FALSE;
	pThis->WeaponChanged					= FALSE;
	pThis->WeaponMode						= TWEAPONMODE_OFF;
	pThis->WeaponCount					= 0;
	pThis->WeaponFired					= TWEAPON_NONE;
	pThis->AmmoUseCounter				= 0.0;
	pThis->GunKickXRot					= 0;
	pThis->DeathStage						= TMOVE_HEALTH_ALIVE;
	pThis->WeaponBarrelZRot				= 0;
	pThis->WeaponBarrelZRotInc			= 0;
	pThis->BowStarted						= 0.0;
	pThis->BowFlashOccurred				= FALSE;
	pThis->ShockwaveStarted				= 0.0;
	pThis->ChronoscepterStarted		= 0.0;
	pThis->DeathTimer						= TMOVE_DEATH_TIMER;
	pThis->MiniGunBarrelSFX				= FALSE;
	pThis->hSFXBarrel						= -1;
	pThis->SwimBurstToggle				= TRUE;						// false = slow swim mode, true = burst swim mode
	pThis->JumpAngle						= 0;
	pThis->StartJumpAngle				= 0;
	pThis->DesiredJumpAngle				= 0;
	pThis->JumpU							= 0;
	pThis->ActualRotXPlayer				= 0;
	pThis->ActualRotYPlayer				= 0;
	pThis->RotXPlayer						= 0;
	pThis->RotYPlayer						= 0;
	pThis->RotZPlayer						= 0;
	pThis->MoveUpSpeed					= 0;
	pThis->CannotSwimUp					= FALSE;
	pThis->Oxygen							= 0;					// no. of seconds turok has been underwater
	pThis->Bubbles							= 0;
	pThis->FeetOnBottomStatus			= TMOVE_LAST_NOTINWATER;
	pThis->Time1							= 0;
	pThis->Time2							= 0;
	pThis->SideStepZRot					= 0;
	pThis->BurstForwardSpeed			= 0;
	pThis->ActualBurstForwardSpeed	= 0;
	pThis->ClimbSwingRotX				= 0;
	pThis->ClimbSwingRotY				= 0;
	pThis->ClimbSwingRotZ				= 0;
	pThis->ActualClimbSwingRotX		= 0;
	pThis->ActualClimbSwingRotY		= 0;
	pThis->ActualClimbSwingRotZ		= 0;
	pThis->ClimbHand						= FALSE;
	pThis->ClimbOneHand					= FALSE;

	pThis->FiredParticle						= FALSE;
	pThis->FiredParticleType			  	= FALSE;
	pThis->FiredProjectileParticle	  	= FALSE;
	pThis->FiredProjectileParticleType 	= FALSE;

	pThis->ClimbCameraWobble			= FALSE;
	pThis->ClimbWeaponRemove			= FALSE;

// REMOVED by Ian - solves problem of dying on cliffs
//	pThis->WeaponAtStartOfClimb		= -1;

	pThis->WeaponAtStartOfDuck			= -1;
	pThis->WeaponAtStartOfWaterDive	= -1;
	pThis->CurrentDuckOffset			= 0;
	pThis->JumpAllowed					= TRUE;
	pThis->ClimbSideStepLookAng		= 0;
	pThis->ClimbSideStep					= FALSE;
	pThis->BurstTimer						= 0.0;
	pThis->SelectWeaponTimer			= 0.0;
	pThis->LookUpDownSpeed				= 0.0;
	pThis->ActualLookUpDownSpeed		= 0.0;
	pThis->SwimBankAngle					= 0.0;
	pThis->ActualSwimBankAngle			= 0.0;
	pThis->CannotJumpFromSurface		= FALSE;
	pThis->vLastPosition.x				= 0.0;
	pThis->vLastPosition.y				= 0.0;
	pThis->vLastPosition.z				= 0.0;
	pThis->WaterSfxTimer					= 0.0;
	pThis->WaterSfxTimerMax				= 0.0;
	pThis->BubblesSfxTimer				= 0.0;
	pThis->PrevOnGround					= FALSE;
	pThis->CurrentOnGround				= FALSE;
	CQuatern__Identity(&pThis->qRealAimFix);
	CQuatern__Identity(&pThis->qAimFix);
	pThis->JustWarped						= FALSE;
	pThis->DeathRegionTimer				= 0.0;
	pThis->JustEnteredSaveRegion		= FALSE;
	pThis->JustEnteredLockRegion		= FALSE;
	pThis->JustEnteredDeathRegion		= FALSE;
	pThis->JustEnteredTrainingRegion	= FALSE;
	pThis->OxygenOut						= FALSE;
	pThis->PrevVelocityY					= 0.0;
	pThis->OnLadder						= FALSE;
	pThis->InAntiGrav						= FALSE;
	pThis->PrevAntiGrav					= FALSE;
	pThis->HitSoundTimer					= TMOVE_HITSOUND_TIMER_MAX;
	pThis->HitSoundTimerScream			= FALSE;
	pThis->CannotJumpFromCliffFace	= FALSE;
	pThis->TremorTimer					= 0.0;
	pThis->FiredBullet1					= FALSE;
	pThis->FiredBullet2					= FALSE;
	pThis->FiredBullet3					= FALSE;
	pThis->FiredShotgunShell			= FALSE;
	pThis->MapButtonTimer				= 0.0;
	pThis->MapToggle						= FALSE;
	pThis->MapScrolling					= FALSE;
	pThis->RealMapOffsetX				= 0.0;
	pThis->RealMapOffsetZ				= 0.0;
	pThis->MapOffsetX						= 0.0;
	pThis->MapOffsetZ						= 0.0;
	pThis->InLava							= FALSE;
	pThis->hSFXPrevShockwaveCharge	= -1;
	pThis->hSFXNextShockwaveCharge	= -1;
	pThis->FireFirstMachineGunBullet	= FALSE;
	pThis->WeaponTakeOffQuickly		= TRUE;
	pThis->WeaponTimer					= 0.0;
	pThis->LookAtShover					= 0.0;
	pThis->pShoverAI						= NULL;
	pThis->JustEnteredPressurePlate	= FALSE;
	pThis->pModifyEnemy					= NULL;
	pThis->Active							= FALSE;
	pThis->InSun							= FALSE;
	pThis->InvincibleTimer				= 2.0;		// in seconds

	// oxygen tank seconds count
	pThis->OxygenTank						= 0;	// no. of seconds in left in any tanks
	pThis->SpiritualTime					= 0;

	// reset players start health
	pPlayer = CEngineApp__GetPlayer(GetApp());
	ASSERT(pPlayer);
	/* PORT: NewLifeSetup is called in MODE_RESETGAME before MODE_RESETLEVEL loads the level
	 * and creates the player (m_pPlayer), so pPlayer can be NULL here. On the N64 (no MMU) the
	 * resulting NULL deref harmlessly reads low RDRAM and is overwritten when the level loads;
	 * on a protected host it faults. Guard it — the level load redoes this setup correctly. */
	if (pPlayer)
		CAIDynamic__SetHealth(&pPlayer->m_AI, pPlayer->ah.ih.m_pEA, AI_OBJECT_CHARACTER_PLAYER);

	// no armor after dying...
	CTurokMovement.ArmorFlag						= FALSE;
	CTurokMovement.ArmorAmount						= 0 ;

	// special case for underwater
	if (pThis->WaterFlag == PLAYER_BELOW_WATERSURFACE)
		pThis->WeaponCurrentAnim = AI_ANIM_WEAPON_UNDERWATER_IDLE;
	else
		pThis->WeaponCurrentAnim = AI_ANIM_WEAPON_ONSCREEN;

	// load correct weapon & animation
	CTMove__LoadPlayerWeapon(pThis);
//	pThis->WeaponCount = 20;
	pThis->WeaponMode = TWEAPONMODE_ON;

	// kill weapon sfx's
	AI_Kill_Weapon_SFX(pThis);

	CTMove__SetCombatTimer(pThis, 0.0);
	COnScreen__SetAmmo(&GetApp()->m_OnScreen, CTMove__GetAmmoOfSelectedWeapon(pThis), CTMove__IsExplosiveAmmo(pThis));


	CCollisionInfo__SetPlayerDefaults(&ci_player);								// floor collision flag modified in cinematic
	CCollisionInfo__SetPlayerUnderwaterDefaults(&ci_playerunderwater);	// gravity modified in cinematic

	CTMove__UpdateMap(pThis, GetApp());

	// warp player to correct starting position
	GetApp()->m_WarpID = pThis->CurrentCheckpoint;

	// set last pointer to collision info
	pThis->pLastCI = NULL;
}



// update the turok game object instance
//
void CTMove__UpdateTurokInstance(CTMove *pThis, CEngineApp *pApp, CTControl *pCTControl)
{
	// declare variables
	CGameObjectInstance	*pIns, *pLook;
	CCamera					*pCamera;

	float						dm,
								ang,
								topCliff,
								time,
								turnang,
								scalerv;
	BOOL						pressurePlate,
								currentOnGround,
								playsound;
	CROMRegionSet			*pRegionSet;
	int						waterFlag,
								climbFlag,
								duckFlag,
								nObjType;
	CCollisionInfo2		*pCI = NULL;
	BYTE						tint[4] = {255, 255, 255, 255};
	CVector3					vPos ;
//	static float			ycoor,
//								ycoorjump = -30000.0;
//	static char				buffer1[60],
//								buffer2[60];
//	static char				ste[80] ;

	vPos.x = AW.Ears.vPos.x;
	vPos.y = AW.Ears.vPos.y;
	vPos.x = AW.Ears.vPos.z;

	if (pThis->InMenuTimer)
	{
		pThis->InMenuTimer -= frame_increment*(1.0/FRAME_FPS) ;
		if (pThis->InMenuTimer < 0.0)
			pThis->InMenuTimer = 0.0 ;
	}


	// get instance pointer to turok
	pIns = CEngineApp__GetPlayer(pApp);
	if (!pIns)
		return;

	pCamera = &pApp->m_Camera;
	if (!pCamera)
		return;

	// Update the spinning warp sound
	CTMove__UpdateWarpSound(pThis) ;

	// in test fly mode ?
	if (GetApp()->m_dwCheatFlags & CHEATFLAG_FLYMODE)
	{
		CTMove__FlyMode(pThis, pApp, pCTControl);
		return;
	}

	pRegionSet = CScene__GetRegionAttributes(&GetApp()->m_Scene, pIns->ah.ih.m_pCurrentRegion) ;

	// Update active flag
	pThis->Active = (pIns->m_AI.m_Health > 0) && (!CCamera__InCinemaMode(pCamera)) ;

	// get player environment status
	waterFlag = CEngineApp__GetPlayerWaterStatus(pApp);
	pThis->WaterFlag = waterFlag;
	climbFlag = CEngineApp__GetPlayerClimbStatus(pApp);
	duckFlag = CEngineApp__GetPlayerDuckStatus(pApp);
	pThis->OnLadder = CTMove__OnLadder(pThis);
	pThis->PrevAntiGrav = pThis->InAntiGrav;
	pThis->InAntiGrav = CTMove__InAntiGrav(pThis);

//	sprintf(ste, "index %d", pThis->m_WeaponSelectIndex);
//	COnScreen__AddGameTextWithId(ste, 4);

	// Update music/ambient sounds so that they work during the intro.. otherwise they erm don't.
	if (pRegionSet)
	{
		if (		(waterFlag == PLAYER_BELOW_WATERSURFACE)
				&&	(!pThis->InAntiGrav) )
		{
			pThis->MusicID = MUSIC_UNDERWATER;
		}
		else
		{
			pThis->MusicID = pRegionSet->m_MusicID ;
		}

		// don't play ambient sfx's if turok is dead
		if (pIns->m_AI.m_Health)
			pThis->AmbientSound = pRegionSet->m_AmbientSounds;
		else
			pThis->AmbientSound = REG_AMBIENTNONE;

//		sprintf(ste, "Music ID %d", pThis->MusicID) ;
//		COnScreen__AddGameTextWithId(ste,32) ;

//		ASSERT(pThis->AmbientSound == REG_AMBIENTNONE);
	}
	else
	{

//		This line was commented out on 12/15/96 By Stephen Broumley.
//		It stops the music restarting during cinemas and such. Let's hope it doesn't break anything......
//		pThis->MusicID = 255;
		pThis->AmbientSound = REG_AMBIENTNONE;
	}

	// spirit cheat mode on ?
	if (GetApp()->m_dwCheatFlags & CHEATFLAG_SPIRITMODE)
		pThis->SpiritualTime = SPIRITUAL_TIMEOUT ;

	// is all weapon cheat mode on ?
	if ((GetApp()->m_dwCheatFlags & CHEATFLAG_ALLWEAPONS) || (CAttractDemo__Active()))
	{
		pThis->KnifeFlag						= TRUE;
		pThis->TekBowFlag						= TRUE;
		pThis->SemiAutomaticPistolFlag	= TRUE;
		pThis->AssaultRifleFlag				= TRUE;
		pThis->MachineGunFlag				= TRUE;
		pThis->RiotShotgunFlag				= TRUE;
		pThis->AutomaticShotgunFlag		= TRUE;
		pThis->MiniGunFlag					= TRUE;
		pThis->GrenadeLauncherFlag			= TRUE;
		pThis->TechWeapon1Flag				= TRUE;
		pThis->RocketFlag						= TRUE;
		pThis->ShockwaveFlag					= TRUE;
		pThis->TechWeapon2Flag				= TRUE;
		pThis->ChronoscepterFlag			= TRUE;

		// has player received all ammo yet ? - this can only be given once per game
		if (!pThis->AllWeaponsAmmoGiven)
		{
			pThis->TekBowAmmo						= MAX_ARROWS;
			pThis->BulletPool						= MAX_BULLETS;
			pThis->EnergyPool						= MAX_ENERGY;
			pThis->ShotgunPool					= MAX_SHOTGUN;
			pThis->MiniGunAmmo					= MAX_MINIGUN;
			pThis->GrenadeLauncherAmmo			= MAX_GRENADES;
			pThis->RocketAmmo						= MAX_ROCKETS;
			pThis->TechWeapon2Ammo				= MAX_TECH2;
			pThis->ChronoscepterAmmo			= MAX_CHRONOSCEPTER;
			pThis->AllWeaponsAmmoGiven			= TRUE;
		}
	}

	if ((GetApp()->m_dwCheatFlags & CHEATFLAG_UNLIMITEDAMMO) || (CAttractDemo__Active()))
	{
		pThis->TekBowAmmo						= MAX_ARROWS;
		pThis->BulletPool						= MAX_BULLETS;
		pThis->EnergyPool						= MAX_ENERGY;
		pThis->ShotgunPool					= MAX_SHOTGUN;
		pThis->MiniGunAmmo					= MAX_MINIGUN;
		pThis->GrenadeLauncherAmmo			= MAX_GRENADES;
		pThis->RocketAmmo						= MAX_ROCKETS;
		pThis->TechWeapon2Ammo				= MAX_TECH2;
		pThis->ChronoscepterAmmo			= MAX_CHRONOSCEPTER;
	}

	// is player invincible at start of new life ?
	if (!CCamera__InCinemaMode(pCamera))
	{
		if (pThis->InvincibleTimer)
		{
			pThis->InvincibleTimer -= frame_increment*(1.0/FRAME_FPS);
			if (pThis->InvincibleTimer < 0.0)
				pThis->InvincibleTimer = 0.0;

			AI_GetDyn(pIns)->m_Invincible = TRUE;
		}
		else
		{
			AI_GetDyn(pIns)->m_Invincible = FALSE;
		}
	}

	// get current on ground status
	currentOnGround = CInstanceHdr__IsOnGround(&pIns->ah.ih);
	pThis->PrevOnGround = pThis->CurrentOnGround;
	pThis->CurrentOnGround = currentOnGround;

	// animate rotating gun barrel weapons on relevant weapons
	CTMove__WeaponBarrelUpdate(pThis, pThis->WeaponCurrent);

	// update tremor shake screen effect
	CTMove__UpdateTremorEffect(pThis);

	// update in combat timer
	CTMove__UpdateCombatTimer(pThis);

	pApp->m_YPosOffset			= TUROK_EYE_HEIGHT*SCALING_FACTOR ;
	pApp->m_RotXOffset			= 0.0;
	pApp->m_RotYOffset			= 0.0;
	pApp->m_RotZOffset			= 0.0;
	pThis->ActualSwimBankAngle = 0.0;

	CCamera__CheckForTurokDeathCinematic(pCamera);

	// if player dies kill any weapon sfx's
	if (		(pIns->m_AI.m_Health == 0)
			||	(CCamera__InCinemaMode(pCamera)) )
	{
		// kill weapon sfx's
		AI_Kill_Weapon_SFX(pThis);

		if (waterFlag == PLAYER_NOT_NEAR_WATER)
		{
#ifndef PLATFORM_PORT
			pIns->ah.m_vVelocity.y = -15*SCALING_FACTOR;
			CAnimInstanceHdr__Collision3(&pIns->ah, pIns->ah.ih.m_vPos, &ci_playerdead);
#else
			/* PORT: the dead/cinema "drop to ground" physics crashed on KEY-PICKUP cinematics — the M5
			 * collision parse is incomplete so some region corner pointers are host-range GARBAGE (they
			 * pass the TUROK_BADPTR/N64-range guard in CGameRegion__GetGroundNormal but deref unmapped
			 * memory). BUT the FALL-DEATH cinematic sets GroundBehavior=INTERSECT_BEHAVIOR_IGNORE on all
			 * ci_player* (cinecam.c:414-419) which gates the ground-corner deref off (unicol.c:1039-1044),
			 * and waterFlag==PLAYER_NOT_NEAR_WATER here so there is no water-region deref either — so its
			 * Collision3 is crash-safe. Run it ONLY for the fall-death cinematic so Turok actually DESCENDS
			 * (else he hangs suspended in the AI_ANIM_DEATH_HIGH_FALL pose); keep the skip for the
			 * crash-prone key-pickup cinematics. (Whole guard becomes dead code once M5 relocates corners.) */
			if (GetApp()->m_Camera.m_Mode == CAMERA_CINEMA_TUROK_FALL_DEATH_MODE)
			{
				pIns->ah.m_vVelocity.y = -6*SCALING_FACTOR*15;   /* match cinecam.c fall velocity */
				CAnimInstanceHdr__Collision3(&pIns->ah, pIns->ah.ih.m_vPos, &ci_playerdead);
			}
#endif
			pThis->pLastCI = &ci_playerdead;
		}
	}


	// Don't do any of this if we are in cinema mode
	if (!CCamera__InCinemaMode(pCamera))
	{
		CTMove__LoadPlayerWeapon(pThis);

		// is player alive ?
		if (pIns->m_AI.m_Health)
		{
			// is turok looking at someone who shoved him ?
			if (pThis->LookAtShover > 0.0)
			{
				pThis->LookAtShover -= frame_increment*(1.0/FRAME_FPS);
				if (pThis->LookAtShover < 0.0)
					pThis->LookAtShover = 0.0;

				pLook = pThis->pShoverAI;
				if (pLook)
				{
					turnang = AI_GetAngle(pIns, AI_GetPos(pLook));
					AI_NudgeRotY(pIns, frame_increment*turnang/1.2);
				}
			}
			else
				pThis->pShoverAI = NULL;

			// increase hit sound timer
			pThis->HitSoundTimer += frame_increment*(1.0/FRAME_FPS);

			// did turok take the peyote?
			if (pThis->SpiritualTime)
			{
				// blend to
				if (pThis->SpiritualTime > (SECONDS_TO_FRAMES(SPIRITUAL_TIMER) - SPIRITUAL_TIMEIN))
				{
					time = (SECONDS_TO_FRAMES(SPIRITUAL_TIMER) - pThis->SpiritualTime) / SPIRITUAL_TIMEIN ;
					sky_speed_scaler = BlendFLOAT(time, 1.0, SPIRITUAL_SKY) ;
					enemy_speed_scaler = BlendFLOAT(time, CEngineApp__GetEnemySpeedScaler(pApp), SPIRITUAL_ENEMY) ;
					particle_speed_scaler = BlendFLOAT(time, 1.0, SPIRITUAL_PARTICLE) ;
				}
				// blend from
				else if (pThis->SpiritualTime <= SPIRITUAL_TIMEOUT)
				{
					time = pThis->SpiritualTime / SPIRITUAL_TIMEOUT ;
					sky_speed_scaler = BlendFLOAT(time, 1.0, SPIRITUAL_SKY) ;
					enemy_speed_scaler = BlendFLOAT(time, CEngineApp__GetEnemySpeedScaler(pApp), SPIRITUAL_ENEMY) ;
					particle_speed_scaler = BlendFLOAT(time, 1.0, SPIRITUAL_PARTICLE) ;
				}

				pThis->SpiritualTime -= frame_increment ;
				if (pThis->SpiritualTime <= 0)
				{								   
					pThis->SpiritualTime = 0 ;
					CScene__SetupRealTimeLight(&GetApp()->m_Scene) ;
				}
			}

			// are we shorter ?
			CTMove__DoDucking(pThis, pApp);

			// turok using his map ?
			CTMove__UsingMap(pThis, pCTControl);

			// alllow player to get closer to wall while jumping
			if (pThis->Mode == TMMODE_GRDJUMP)
				ci_player.dwFlags |= COLLISIONFLAG_SMALLWALLRADIUS;
			else
				ci_player.dwFlags &= ~COLLISIONFLAG_SMALLWALLRADIUS;


			// calc scaler value to adjust vertical analog motion for control
			scalerv = ((float)GetApp()->m_Options.m_VAnalog);
			if (scalerv < 20.0)
				scalerv = 20.0;
			scalerv = scalerv / 128.0;

			// do looking around for swimming
			switch (waterFlag)
			{
				case PLAYER_BELOW_WATERSURFACE:
					if (!pThis->InAntiGrav)
					{
#ifdef ALLOW_UPSIDEDOWN_UNDERWATER
						CTMove__TurnOnXAxis(pThis, pApp, pCTControl);
#else
						pThis->ActualRotXPlayer = (CTControl__IsLookUp   (pCTControl) - CTControl__IsLookDown(pCTControl))*ANGLE_DTOR(-90)/80;
						dm = (pThis->ActualRotXPlayer - pThis->RotXPlayer) / (12/scalerv);
						pThis->RotXPlayer += dm * frame_increment * 2;
#endif
						pCI = &ci_playerunderwater;
					}
					else
					{
						pThis->ActualRotXPlayer = (CTControl__IsLookUp   (pCTControl) - CTControl__IsLookDown(pCTControl))*ANGLE_DTOR(-80)/80;
						dm = (pThis->ActualRotXPlayer - pThis->RotXPlayer) / (6/scalerv);
						pThis->RotXPlayer += dm * frame_increment * 2;
						pCI = &ci_playerunderwater;
					}
					break;

				case PLAYER_ON_WATERSURFACE:
					if (		(climbFlag == PLAYER_AT_CLIMBING_SURFACE)
							&&	(pThis->Mode == TMMODE_CLIMBING) )
					{
						CTMove__SetClimbCamera(pThis, pApp, pCTControl);
					}
					else
					{
						pThis->ActualRotXPlayer = (CTControl__IsLookUp   (pCTControl) - CTControl__IsLookDown(pCTControl))*ANGLE_DTOR(-80)/80;
						dm = (pThis->ActualRotXPlayer - pThis->RotXPlayer) / (6/scalerv);
						pThis->RotXPlayer += dm * frame_increment * 2;
					}
					pCI = &ci_playeronwatersurface;
					break;

				default:
					if (		(climbFlag == PLAYER_AT_CLIMBING_SURFACE)
							&&	(pThis->Mode == TMMODE_CLIMBING) )
					{
						CTMove__SetClimbCamera(pThis, pApp, pCTControl);
					}
					else
					if (CCamera__UpdateTurokHead(pCamera))
					{
						pThis->RotYPlayer = (CTControl__IsLookRight(pCTControl) - CTControl__IsLookLeft(pCTControl))*ANGLE_DTOR( 90)/80;

#ifdef PLATFORM_3DS
						{	extern int g_cfg_recenter_look;
							if (!g_cfg_recenter_look)
							{
								/* HOLD-LOOK (recenter OFF): accumulate the vertical look by the stick RATE and hold
								 * it on release, instead of the position-based auto-return-to-horizon below. Up
								 * (IsLookUp>0) decreases RotXPlayer (matches the position path's -= for up); clamped
								 * to the same [-78deg up, +90deg down] range; scaled by the V-analog sensitivity. */
								float lk = CTControl__IsLookUp(pCTControl) - CTControl__IsLookDown(pCTControl);
								pThis->RotXPlayer -= lk * (ANGLE_DTOR(90)/80.0f/24.0f) * scalerv * frame_increment;
								if (pThis->RotXPlayer < -ANGLE_DTOR(78)) pThis->RotXPlayer = -ANGLE_DTOR(78);
								if (pThis->RotXPlayer >  ANGLE_DTOR(90)) pThis->RotXPlayer =  ANGLE_DTOR(90);
								pThis->ActualRotXPlayer = pThis->RotXPlayer;   /* keep in sync (no jump if toggled) */
							}
							else
#endif
						{
						// make it so you can't look backwards when you jump
						pThis->ActualRotXPlayer = -pThis->JumpLookAng;

						ang = ANGLE_DTOR(90) - pThis->ActualRotXPlayer;
						pThis->ActualRotXPlayer += CTControl__IsLookDown(pCTControl)*ang / 80;

						ang = pThis->ActualRotXPlayer + ANGLE_DTOR(78);
						pThis->ActualRotXPlayer -= CTControl__IsLookUp(pCTControl)*ang / 80;

						dm = (pThis->ActualRotXPlayer - pThis->RotXPlayer) / (6/scalerv);
						pThis->RotXPlayer += dm * frame_increment * 2;
						}
#ifdef PLATFORM_3DS
						}
#endif
					}
					pCI = &ci_player;
					break;
			}

			// make rot x wrap around
			NormalizeRotation(&pThis->RotXPlayer);
			NormalizeRotation(&pThis->RotZPlayer);
			if (climbFlag != PLAYER_AT_CLIMBING_SURFACE)
				NormalizeRotation(&pThis->RotYPlayer);
			NormalizeRotation(&pIns->m_RotY);

			// set rotation offsets
			pApp->m_RotXOffset = pThis->RotXPlayer;
			pApp->m_RotYOffset = pThis->RotYPlayer;
			pApp->m_RotZOffset = pThis->RotZPlayer;

			// do land sfx ?
			if (		(GetApp()->m_Camera.m_Mode != CAMERA_CINEMA_TUROK_FALL_DEATH_MODE)
					&&	(!pThis->PrevOnGround)
					&&	(pThis->CurrentOnGround)
					&& (pThis->PrevVelocityY <= 0) )
			{
				switch (waterFlag)
				{
					case PLAYER_ON_WATERSURFACE:
						if (pThis->PrevVelocityY < -2.0*15.0)		// used to be -0.5
							CTMove__DoSplash(pThis, pApp, TRUE);

						if (pThis->PrevVelocityY < (-65.0*15.0))			// used to be -50.0
							AI_DoSound(&pIns->ah.ih, SOUND_GENERIC_22, 1, 0);
						break;

					case PLAYER_ABOVE_WATERSURFACE:
					case PLAYER_NOT_NEAR_WATER:
						if (		(pRegionSet)
								&& (pRegionSet->m_dwFlags & REGFLAG_FALLDEATH) )
						{
							break;
						}

						if (pThis->PrevVelocityY < (-65.0*15.0))			// used to be -50.0
							AI_DoSound(&pIns->ah.ih, SOUND_GENERIC_22, 1, 0);
						break;
				}
			}

			// store previous velocity
			pThis->PrevVelocityY = pIns->ah.m_vVelocity.y;


			// is player in a ducking area but not below water ?
			if (waterFlag != PLAYER_BELOW_WATERSURFACE)
			{
				switch(duckFlag)
				{
					case	PLAYER_NOT_DUCKING:
						// if player was previously ducking then set weapon back to what it was
						if (pThis->WeaponAtStartOfDuck != -1)
						{
							if (!CTMove__SelectWeaponByAIType(pThis, pThis->WeaponAtStartOfDuck, FALSE))
								CTMove__SelectWeaponByAIType(pThis, AI_OBJECT_WEAPON_KNIFE, FALSE);

							pThis->WeaponAtStartOfDuck = -1;
						}
						break;

					case	PLAYER_DUCKING:
					case	PLAYER_DUCKING_ENTRANCE_EXIT:
						// remember weapon if we're just entering a ducking region
						if (pThis->WeaponAtStartOfDuck == -1)
						{
							pThis->WeaponAtStartOfDuck = CTMove__GetWeaponAIType(pThis);
							pThis->WeaponAtStartOfWaterDive = pThis->WeaponAtStartOfDuck;

							CTMove__SelectWeaponByAIType(pThis, AI_OBJECT_WEAPON_KNIFE, FALSE);
						}
						break;
				}
			}


			// is player on surface or below water ?
			switch (waterFlag)
			{
				case	PLAYER_NOT_NEAR_WATER:
					// allow leaving water in a water cube (from the side)
					if (pThis->PrevMode == TMMODE_SWIMWATER)
					{
						pThis->MoveUpSpeed = 0;
						CTMove__StopJumping(pThis);

						if (!pThis->PrevAntiGrav)
						{
							// do gasp sfx
							if (pThis->OxygenOut)
								AI_DoSound(&pIns->ah.ih, SOUND_GENERIC_17, 1, 0);
							else
								AI_DoSound(&pIns->ah.ih, SOUND_GENERIC_16, 1, 0);

							pIns->ah.m_vVelocity.y	= 0;

							if (!CTMove__SelectWeaponByAIType(pThis, pThis->WeaponAtStartOfWaterDive, FALSE))
								CTMove__SelectWeaponByAIType(pThis, AI_OBJECT_WEAPON_KNIFE, FALSE);

							pThis->WeaponAtStartOfWaterDive = -1;

							COnScreen__SetAirMode(&pApp->m_OnScreen, AIRMODE_FADEOUT) ;
							pThis->CannotJumpFromSurface = FALSE;
							CTMove__ClearHeadRotations(pThis);
						}
						pThis->ActualBurstForwardSpeed = 0.0;
						pThis->BurstForwardSpeed = 0.0;
					}
					// fall through

				default:
				case	PLAYER_ABOVE_WATERSURFACE:
					pThis->FeetOnBottomStatus = TMOVE_LAST_NOTINWATER;
					pThis->Oxygen = 0;
					if (pThis->Mode == TMMODE_SWIMWATER ||
						 pThis->Mode == TMMODE_SWIMSURFACE)
					{
						pThis->Mode = TMMODE_GROUND;
					}
					else
					{
						// is player climbing ?
						if (    (climbFlag == PLAYER_AT_CLIMBING_SURFACE)
							  && (pThis->Mode != TMMODE_CLIMBING)
							  && (pThis->Mode != TMMODE_GRDJUMP) )
						{

							// we can't grab onto to top of cliff
	#define	TMOVE_PLAYER_GRABCLIFF		(4*SCALING_FACTOR)
							topCliff = CEngineApp__GetPlayerHeightFromTopOfCliff(pApp);
							if (topCliff >= TMOVE_PLAYER_GRABCLIFF)
							{
								// player going into climbing mode
								pThis->Mode = TMMODE_CLIMBING;
								CTMove__ResetDrawnWeapons(pThis);
								CTMove__StopJumping(pThis);
								pThis->WeaponAtStartOfClimb = CTMove__GetWeaponAIType(pThis);
								CTMove__RemoveWeaponFromScreen(pThis, pApp);
								pThis->MoveSpeed = 0.0;
							}
						}
						else if (pThis->Mode == TMMODE_CLIMBING && climbFlag != PLAYER_AT_CLIMBING_SURFACE)
						{
							// player leaving climbing mode
							pThis->Mode = TMMODE_GROUND;

							// put weapon back to what it was prior to climbing
							if (!CTMove__SelectWeaponByAIType(pThis, pThis->WeaponAtStartOfClimb, FALSE))
								CTMove__SelectWeaponByAIType(pThis, AI_OBJECT_WEAPON_KNIFE, FALSE);

							pThis->WeaponAtStartOfClimb		= -1;
						}
					}
					break;

				case	PLAYER_BELOW_WATERSURFACE:
					if (!pThis->InAntiGrav)
						CTMove__UpdateSwimBurstToggle(pThis, pCTControl);

					pThis->FeetOnBottomStatus = TMOVE_LAST_UNDERWATER;
					if (pThis->Mode != TMMODE_SWIMWATER)
					{
						if (pThis->InAntiGrav)
						{
							CTMove__StopJumping(pThis);
							pThis->MoveSpeed = 0;
							pThis->MoveUpSpeed = 0;
							pThis->SideStepSpeed = 0;
						}
						else
						{
							CTMove__StopJumping(pThis);
							pThis->MoveSpeed = 0;
							pThis->MoveUpSpeed = 0;
							pThis->SideStepSpeed = 0;
							pThis->CannotSwimUp = TRUE;
							CTMove__ResetDrawnWeapons(pThis);
							CTMove__InitBubbles(pThis, pApp);
							pThis->OxygenOut = FALSE;
							CTMove__DoSplash(pThis, pApp, TRUE);

							if (pThis->WeaponAtStartOfWaterDive == -1)
								pThis->WeaponAtStartOfWaterDive = CTMove__GetWeaponAIType(pThis);

							CTMove__RemoveWeaponFromScreen(pThis, pApp);
							//COnScreen__SetAirMode(&pApp->m_OnScreen, AIRMODE_FADEON) ;

							// Start camera wobble
							CCamera__TurokGoneBelowWater(pCamera) ;
						}
					}

					// if 15 seconds away from death, bring on timer
					if (pThis->Oxygen > (TMOVE_OXYGEN_AIRGULP_TIMER-TMOVE_OXYGEN_DISPLAY_TIMER))
						COnScreen__SetAirMode(&pApp->m_OnScreen, AIRMODE_FADEON) ;

					pThis->Mode = TMMODE_SWIMWATER;
					if (!pThis->InAntiGrav)
						CTMove__UnderwaterOxygen(pThis, pApp);
					break;

				case	PLAYER_ON_WATERSURFACE:
					if (!pThis->InAntiGrav)
						CTMove__UpdateSwimBurstToggle(pThis, pCTControl);

					// are players feet on the bottom of the sea floor ?
					if (		(pThis->PrevOnGround)
							&&	(!pThis->CurrentOnGround) )
					{
						// player has reached drop off - is player ready to fall in quickly ?
						if (pThis->FeetOnBottomStatus == TMOVE_LAST_READYFORFALL)
						{
							// make player fall underwater if they were previously above the water
							if (pThis->Mode == TMMODE_SWIMSURFACE)
							{
								pThis->FeetOnBottomStatus = TMOVE_LAST_UNDERWATER;
								if (pRegionSet)
								{
									if (		(!pThis->InAntiGrav)
											&&	(!(pRegionSet->m_dwFlags & REGFLAG_SHALLOWWATER)) )
									{
										pIns->ah.m_vVelocity.y -= (5*15.0*SCALING_FACTOR);
									}
								}
							}
						}
					}
					else
					{
						// player is ready for a big fall off
						pThis->FeetOnBottomStatus = TMOVE_LAST_READYFORFALL;
					}

					pThis->Oxygen = 0;
					if (    (!CTControl__IsJump(pCTControl))
						  || (pThis->CannotJumpFromSurface) )
					{
						if (pThis->Mode != TMMODE_SWIMSURFACE)
						{
							if (pThis->InAntiGrav)
							{
								pThis->MoveUpSpeed = 0;
								CTMove__StopJumping(pThis);

								// do leaving anti grav chamber sfx
								if (pThis->PrevMode == TMMODE_SWIMWATER)
								{
									//AI_DoSound(&pIns->ah.ih, SOUND_GENERIC_17, 1, 0);
								}
							}
							else
							{
								pThis->MoveUpSpeed = 0;
								CTMove__StopJumping(pThis);
								CTMove__DoRipple(pThis, pApp);

								// do gasp sfx
								if (pThis->PrevMode == TMMODE_SWIMWATER)
								{
									CTMove__DoSplash(pThis, pApp, FALSE);

									if (pThis->OxygenOut)
										AI_DoSound(&pIns->ah.ih, SOUND_GENERIC_17, 1, 0);
									else
										AI_DoSound(&pIns->ah.ih, SOUND_GENERIC_16, 1, 0);
								}

								// put weapon back to what it was prior to water dive
								if (pThis->Mode == TMMODE_SWIMWATER)
								{
									pIns->ah.m_vVelocity.y	= 0;

									if (!CTMove__SelectWeaponByAIType(pThis, pThis->WeaponAtStartOfWaterDive, FALSE))
										CTMove__SelectWeaponByAIType(pThis, AI_OBJECT_WEAPON_KNIFE, FALSE);

									pThis->WeaponAtStartOfWaterDive = -1;

									COnScreen__SetAirMode(&pApp->m_OnScreen, AIRMODE_FADEOUT) ;

								}
							}
						}
						pThis->Mode = TMMODE_SWIMSURFACE;
					}
					break;
			}

			// assume player is not moving - this means camera changes based on the regions will not take place
			player_is_moving = FALSE;

			// is player in the air ? - avoids problem of camera being wonky after jumping onto something
			if (!CInstanceHdr__IsOnGround(&pIns->ah.ih))
				player_is_moving = TRUE;

			// which mode is turok in ?
			if (CCamera__UpdateTurokPosition(pCamera))
			{
				switch(pThis->Mode)
				{
					case TMMODE_GROUND:
						CTMove__GroundMovement ( pThis, pApp, pCTControl );
						break;

					case TMMODE_GRDROLL:
						CTMove__GroundRollMovement ( pThis, pApp, pCTControl );
						player_is_moving = TRUE;
						break;

					case TMMODE_GRDJUMP:
						CTMove__GroundJumpNSMovement ( pThis, pApp, pCTControl );
						player_is_moving = TRUE;
						break;

					case TMMODE_SWIMSURFACE:
						CTMove__SwimSurfaceMovement(pThis, pApp, pCTControl);
						break;

					case TMMODE_SWIMWATER:
						CTMove__SwimWaterMovement(pThis, pApp, pCTControl);
						break;

					case TMMODE_CLIMBING:
						CTMove__ClimbingMovement(pThis, pApp, pCTControl);
						break;
				}
			}
		}
		else
		{
			// player is dead
			if (	 (!CCamera__CheckForTurokDeathCinematic(&GetApp()->m_Camera))
				 && (	  (CInstanceHdr__IsOnGround(&pIns->ah.ih))
					  || (waterFlag != PLAYER_NOT_NEAR_WATER) ) )
			{
				if (pThis->DeathStage == TMOVE_HEALTH_ALIVE)
				{
					// turok is just starting death
					CTMove__StopJumping(pThis);
					pIns->ah.m_vVelocity.x = pIns->m_AI.m_vExplosion.x;
					pIns->ah.m_vVelocity.y = pIns->m_AI.m_vExplosion.y;
					pIns->ah.m_vVelocity.z = pIns->m_AI.m_vExplosion.z;
					pThis->DeathStage = TMOVE_HEALTH_DEADSTART;

				}

				pApp->m_RotXOffset = pThis->RotXPlayer;
				pApp->m_RotYOffset = pThis->RotYPlayer;
				pApp->m_RotZOffset = pThis->RotZPlayer;
				CTMove__KeelOver ( pThis, pApp, pCTControl );
			}
		}
	}


	// update camera motion
	CTMove__JumpIncrement(pThis);

	// straighten knees
	pThis->HeadYPosLand = BlendFLOAT(min(1,TKNEE_STRAIGHTEN_SMOOTH * frame_increment),
												pThis->HeadYPosLand,
												0);

	// stop head z rotation
	if (pThis->RotZPlayer > 0)
	{
		dm = (0 - pThis->RotZPlayer) / 4;
		pThis->RotZPlayer += dm;
		if ( pThis->RotZPlayer < 0) pThis->RotZPlayer = 0;
	}
	else if (pThis->RotZPlayer < 0)
	{
		dm = (0 - pThis->RotZPlayer) / 4;
		pThis->RotZPlayer += dm;
		if (pThis->RotZPlayer > 0) pThis->RotZPlayer = 0;
	}

	// stop gun kick x rotation
	if ( pThis->GunKickXRot > 0 )
	{
		dm = pThis->GunKickXRot / TTHEAD_GUNKICKXROT_SMOOTH;
		pThis->GunKickXRot -= dm;
		if ( pThis->GunKickXRot < 0 ) pThis->GunKickXRot = 0;
	}
	else if ( pThis->GunKickXRot < 0 )
	{
		dm = pThis->GunKickXRot / TTHEAD_GUNKICKXROT_SMOOTH;
		pThis->GunKickXRot -= dm;
		if ( pThis->GunKickXRot > 0 ) pThis->GunKickXRot = 0;
	}

	// add swimming bank angle
	pThis->RotZPlayer += pThis->SwimBankAngle;
	dm = (pThis->ActualSwimBankAngle - pThis->SwimBankAngle) / 9;
	pThis->SwimBankAngle += dm;

	CTMove__DoRotOffsets(pThis, pApp);
	CTMove__UpdateClimbCameraRot(pThis, pApp);

	// pressure plates
	// add support for other collision infos
	if (pIns->m_AI.m_Health && pCI)
	{
		if (pRegionSet)
		{
			if (pRegionSet->m_dwFlags & REGFLAG_PRESSUREPLATE)
			{
				if (pRegionSet->m_dwFlags & REGFLAG_PRESSUREPLATENEEDSCOLLISION)
					pressurePlate = (pCI->pWallCollisionRegion != NULL);
				else
					pressurePlate = TRUE;

				if (pressurePlate)
				{
					if (		(pRegionSet->m_dwFlags & REGFLAG_PRESSUREPLATESOUND)
							&& (!pThis->JustEnteredPressurePlate) )
					{
						playsound = TRUE ;
						pThis->JustEnteredPressurePlate = TRUE;

						if (		(GetApp()->m_bTrainingMode)			// in training mode
								&&	(GetApp()->m_TrainingType)				// in timed mode
								&& (pRegionSet->m_PressureID > 10000)	// low
								&& (pRegionSet->m_PressureID < 11000))	// high
								playsound = FALSE ;

						if (playsound)
							AI_DoSound(&pIns->ah.ih, pRegionSet->m_PressurePlateSoundNumber, 1, 0);
					}
					AI_Event_Dispatcher(&pIns->ah.ih, &pIns->ah.ih, AI_MEVENT_PRESSUREPLATE, AI_SPECIES_ALL, pIns->ah.ih.m_vPos, (float) pRegionSet->m_PressureID);
				}
			}
			else
				pThis->JustEnteredPressurePlate = FALSE;
		}


		// get pickups
		// add support for other collision infos
		if (pThis->pLastCI)
		{
			if (pThis->pLastCI->pInstanceCollision)
			{
				if (pThis->pLastCI->pInstanceCollision->m_Type == I_SIMPLE)
				{
					nObjType = CInstanceHdr__TypeFlag(pThis->pLastCI->pInstanceCollision);

					if (AI_IsPickup(nObjType))
					{
						CPickup_Pickup((CGameSimpleInstance*)pThis->pLastCI->pInstanceCollision);
					}
					else if (AI_IsWarp(nObjType))
					{
						CWarp__Warp((CGameSimpleInstance*)pThis->pLastCI->pInstanceCollision);
					}
				}
			}
		}


		// player standing in area with the sun ?
		if (		pRegionSet
				&& (!GetApp()->m_Camera.m_LetterBoxScale) )
		{
			if (pRegionSet->m_dwFlags & REGFLAG_SUN)
				pThis->InSun = TRUE;
			else
				pThis->InSun = FALSE;
		}
		else
		{
			pThis->InSun = FALSE;
		}

		// player warping ?
		if (pRegionSet)
		{
			// warp points
			if (		(pRegionSet->m_dwFlags & REGFLAG_WARPENTRANCE)
					&&	(!pThis->JustWarped) )
			{
				if (		(CInstanceHdr__IsOnGround(&pIns->ah.ih))
						||	(pRegionSet->m_dwFlags & REGFLAG_WARPIFINAIR) )
				{
					CTMove__ResetDrawnWeapons(pThis);

					CEngineApp__Warp(GetApp(),
										  pRegionSet->m_WarpID,
										  WARP_WITHINLEVEL,
										  pRegionSet->m_dwFlags & REGFLAG_STOREWARPRETURN);

					pThis->JustWarped = TRUE;
				}
			}
			else if (!(pRegionSet->m_dwFlags & REGFLAG_WARPENTRANCE))
			{
				pThis->JustWarped = FALSE;
			}
		}

		// player standing in lava ?
		if (pRegionSet)
		{
			if (		(pRegionSet->m_dwFlags & REGFLAG_LAVA)
					&&	(CInstanceHdr__IsOnGround(&pIns->ah.ih)) )
			{
				pThis->InLava = TRUE;
			}
			else
			{
				pThis->InLava = FALSE;
			}
		}


		// player standing on return warp ?
		if (pRegionSet)
		{
			// return warp
			if (		(pRegionSet->m_dwFlags & REGFLAG_WARPRETURN)
					&&	(!pThis->JustWarped) )
			{
				if (CInstanceHdr__IsOnGround(&pIns->ah.ih))
				{
					CTMove__ResetDrawnWeapons(pThis);

					CEngineApp__WarpReturn(pApp);
					pThis->JustWarped = TRUE;
				}
			}
			else if (		(!(pRegionSet->m_dwFlags & REGFLAG_WARPRETURN))
							&&	(!(pRegionSet->m_dwFlags & REGFLAG_WARPENTRANCE)) )
			{
				pThis->JustWarped = FALSE;
			}
		}


		// player entering checkpoint ?
		if (pRegionSet)
		{
			// different checkpoint than current WARPID?
			if ((pThis->CurrentCheckpoint != pRegionSet->m_SaveCheckpointID) &&
				(pRegionSet->m_dwFlags & REGFLAG_CHECKPOINT))
			{
				// Special case for bosses
				if ((GetApp()->m_BossLevel) && (GetApp()->m_FadeStatus == FADE_NULL))
				{
					// Cinema's and dying will take player back to start pt!
					GetApp()->m_WarpID = pRegionSet->m_SaveCheckpointID ;
					pThis->CurrentCheckpoint = pRegionSet->m_SaveCheckpointID ;
				}
				else
				{
					COnScreen__AddGameText(text_checkpoint) ;
					AI_DoSound(&pIns->ah.ih, SOUND_GENERIC_249, 1, 0);
				}
			}
			// entered a save region?
			if (!CTControl__IsPrimaryFire(pCTControl))
			{
				if (pRegionSet->m_dwFlags & REGFLAG_SAVEPOINT)
				{
					if (CInstanceHdr__IsOnGround(&pIns->ah.ih))
					{
						if (pThis->JustEnteredSaveRegion)
						{
							GetApp()->m_MapMode = FALSE ;
							GetApp()->m_MapAlpha = 0 ;
							GetApp()->m_MapStatus = MAP_NULL ;

							GetApp()->m_bPause = TRUE;
							CPause__Construct(&GetApp()->m_Pause);
							GetApp()->m_Pause.m_Mode = PAUSE_NORMAL;
							CRequestor__Construct(&GetApp()->m_Requestor,
														text_doyouwishto, text_saveyourgame) ;
							AI_Kill_Weapon_SFX(pThis);
							CTMove__ResetDrawnWeapons(pThis);
							GetApp()->m_bRequestor = TRUE ;
							pThis->JustEnteredSaveRegion = FALSE ;
						}
					}
					else
						pThis->JustEnteredSaveRegion = TRUE ;
				}
				else
					pThis->JustEnteredSaveRegion = TRUE ;
			}
			else if (		(!pThis->JustEnteredSaveRegion)
							&&	(!(pRegionSet->m_dwFlags & REGFLAG_SAVEPOINT)) )
			{
				pThis->JustEnteredSaveRegion = TRUE;
			}


			if (pRegionSet->m_dwFlags & (REGFLAG_CHECKPOINT | REGFLAG_SAVEPOINT))
			{
				pThis->CurrentCheckpoint = pRegionSet->m_SaveCheckpointID;
//	 rmonPrintf("touched checkpoint id:%d\n", pThis->CurrentCheckpoint) ;
			}
		}


		// player entering training text region?
		if ((pRegionSet) && (GetApp()->m_bTrainingMode))
		{
			if ((pRegionSet->m_dwFlags & REGFLAG_PRESSUREPLATE) &&
				(pRegionSet->m_PressureID >= 10000) &&
				(pRegionSet->m_PressureID != 10008) &&
				(pRegionSet->m_PressureID != 10014))
			{
				if (pThis->JustEnteredTrainingRegion == FALSE)
				{
					if (	(pRegionSet->m_PressureID == 10021)
						|| (GetApp()->m_TrainingType ==0))
					{
						GetApp()->m_bTraining = TRUE;
						CTraining__Construct(&GetApp()->m_Training, pRegionSet->m_PressureID);
						pThis->JustEnteredTrainingRegion = TRUE ;
					}
				}
			}
			else
			{
				GetApp()->m_bTraining = FALSE;
				GetApp()->m_Training.m_Mode = TRAINING_FADEDOWN ;
				pThis->JustEnteredTrainingRegion = FALSE ;
			}
		}


		// player touching lock?
		if (pRegionSet)
		{
			if ((pRegionSet->m_PressureID <=8) && (pRegionSet->m_dwFlags & REGFLAG_PRESSUREPLATE))
			{
				if (CInstanceHdr__IsOnGround(&pIns->ah.ih))
				{
					//if (pThis->JustEnteredLockRegion)
					{
						switch (pRegionSet->m_PressureID)
						{
							case 2:
								COnScreen__AddGameTextWithId(text_access2, 0x10) ;
								COnScreen__AddGameTextWithId(text_jungle, 0x11) ;
								break ;
							case 3:
								COnScreen__AddGameTextWithId(text_access3, 0x20) ;
								COnScreen__AddGameTextWithId(text_city, 0x21) ;
								break ;
							case 4:
								COnScreen__AddGameTextWithId(text_access4, 0x30) ;
								COnScreen__AddGameTextWithId(text_ruins, 0x31) ;
								break ;
							case 5:
								COnScreen__AddGameTextWithId(text_access5, 0x40) ;
								COnScreen__AddGameTextWithId(text_catacombs, 0x41) ;
								break ;
							case 6:
								COnScreen__AddGameTextWithId(text_access6, 0x50) ;
								COnScreen__AddGameTextWithId(text_treetop, 0x51) ;
								break ;
							case 7:
								COnScreen__AddGameTextWithId(text_access7, 0x70) ;
								COnScreen__AddGameTextWithId(text_lostland, 0x71) ;
								break ;
							case 8:
								COnScreen__AddGameTextWithId(text_access8, 0x80) ;
								COnScreen__AddGameTextWithId(text_finallevel, 0x81) ;
								break ;

							//default:
							//	pThis->JustEnteredLockRegion = TRUE ;
							//	break ;
						}
						//if (txtA)
						//	txtA->DisplayTime = 40 ;
						//if (txtB)
						//	txtB->DisplayTime = 40 ;
						//if ((txtA) || (txtB))
						//	pThis->JustEnteredLockRegion = FALSE ;
					}
				}
//				else
//					pThis->JustEnteredLockRegion = TRUE ;
			}
//			else
//				pThis->JustEnteredLockRegion = TRUE ;
		}


		// player entering instant death region ?
		if (pRegionSet)
		{
			if (pRegionSet->m_dwFlags & REGFLAG_INSTANTDEATH)
			{
				if (!CCamera__InCinemaMode(pCamera))
				{
					if (CInstanceHdr__IsOnGround(&pIns->ah.ih))
					{
						if (pThis->JustEnteredDeathRegion)
						{
							pThis->DeathRegionTimer = pRegionSet->m_DeathTimeDelay;
							pThis->JustEnteredDeathRegion = FALSE;
						}

						if (!pThis->JustEnteredDeathRegion)
						{
							// do hit points damage
							pThis->DeathRegionTimer += frame_increment*(1.0/FRAME_FPS);
							if (pThis->DeathRegionTimer >= pRegionSet->m_DeathTimeDelay)
							{
								pThis->DeathRegionTimer -= pRegionSet->m_DeathTimeDelay;
								AI_Decrease_Health(pIns, pRegionSet->m_DeathHitPoints, TRUE, TRUE);
							}
						}
					}
					else
					{
						// character is not on the ground
						pThis->JustEnteredDeathRegion = TRUE;
					}
				}
			}
			else
			{
				pThis->JustEnteredDeathRegion = TRUE;
			}
		}
	}

	// if in cinema mode or player is dead - switch off map
	if (		(CCamera__InCinemaMode(pCamera))
			||	(!(pIns->m_AI.m_Health)) )
	{
		pThis->MapToggle = FALSE;
		pThis->MapScrolling = FALSE;
		pThis->MapButtonTimer = 0.0;
	}


	// blend the aim direction
	CTMove__UpdateWeaponPosition(pThis);
	pThis->PrevMode = pThis->Mode;

	// update on screen items
	COnScreen__SetAmmo(&pApp->m_OnScreen, CTMove__GetAmmoOfSelectedWeapon(pThis), CTMove__IsExplosiveAmmo(pThis));
	COnScreen__SetHealth(&pApp->m_OnScreen, pIns->m_AI.m_Health);
	COnScreen__SetLives(&pApp->m_OnScreen, pThis->Lives);
	COnScreen__SetTokens(&pApp->m_OnScreen, pThis->Tokens);

//	if (!(pApp->m_dwCheatFlags & CHEATFLAG_MODIFYENEMY))
		CTMove__UpdateMap(pThis, pApp);

	// modify enemies position ?
/*	if (pApp->m_dwCheatFlags & CHEATFLAG_MODIFYENEMY)
	{
		CTMove__ModifyEnemy(pThis, pApp);

		if (CTControl__IsSelectEnemyPrev(pCTControl))
			CTMove__ChangeEnemy(pThis, pApp, FALSE);

		else if (CTControl__IsSelectEnemyNext(pCTControl))
			CTMove__ChangeEnemy(pThis, pApp, TRUE);

		CTMove__MoveEnemy(pThis, pApp, pCTControl);

		if (CTControl__IsGrenadeLauncherFired(pCTControl))
			CTMove__ResetEnemy(pThis);
	}
*/

	// don't allow turoks eyes to get closer than this distance
	if (pApp->m_YPosOffset<0.5*SCALING_FACTOR)
		pApp->m_YPosOffset = 0.5*SCALING_FACTOR;


// should be 116.390594
//	if (pIns && pCamera && (!CCamera__InCinemaMode(pCamera)) )
//	{
//		ycoor = pIns->ah.ih.m_vPos.y;
//		if (ycoorjump < pIns->ah.ih.m_vPos.y)
//			ycoorjump = pIns->ah.ih.m_vPos.y;
//
//		sprintf(buffer1, "%f", ycoor);
//		sprintf(buffer2, "%f", ycoorjump - ycoor);
//
//		COnScreen__AddGameTextWithId(buffer1, 27);
//		COnScreen__AddGameTextWithId(buffer2, 28);
//	}
//		sprintf(buffer1, "%f", pIns->ah.m_vVelocity.y);
//		COnScreen__AddGameTextWithId(buffer1, 27);

}




// turok is in fly mode
//
void CTMove__FlyMode(CTMove *pThis, CEngineApp *pApp, CTControl *pCTControl)
{
	CGameObjectInstance		*pIns;
	CVector3						vDesiredPos;
	float							ang, dm;


	// get instance pointer to turok
	pIns = CEngineApp__GetPlayer(pApp);
	if (pIns == NULL) return;

	CTMove__Turn(pThis, pApp, pCTControl);

	pThis->RotYPlayer = (CTControl__IsLookRight(pCTControl) - CTControl__IsLookLeft(pCTControl))*ANGLE_DTOR( 90)/80;

	// make it so you can't look backwards when you jump
	pThis->ActualRotXPlayer = -pThis->JumpLookAng;

	ang = ANGLE_DTOR(90) - pThis->ActualRotXPlayer;
	pThis->ActualRotXPlayer += CTControl__IsLookDown(pCTControl)*ang / 80;

	ang = pThis->ActualRotXPlayer + ANGLE_DTOR(78);
	pThis->ActualRotXPlayer -= CTControl__IsLookUp(pCTControl)*ang / 80;

	dm = (pThis->ActualRotXPlayer - pThis->RotXPlayer) / 6;
	pThis->RotXPlayer += dm * frame_increment * 2;

	pApp->m_RotXOffset = pThis->RotXPlayer;
//	pApp->m_RotYOffset = pThis->RotYPlayer;
//	pApp->m_RotZOffset = pThis->RotZPlayer;

	// initialize current position into desired position before we change it
	vDesiredPos = pIns->ah.ih.m_vPos;

	vDesiredPos = CTMove__ControlFlyFBward  (pThis, pApp, pCTControl, vDesiredPos);
	vDesiredPos = CTMove__ControlFlySideStep(pThis, pApp, pCTControl, vDesiredPos);
	vDesiredPos = CTMove__ControlFlyUpDown  (pThis, pApp, pCTControl, vDesiredPos);

//	pIns->ah.ih.m_vPos = vDesiredPos;
//	osSyncPrintf("x:%f y:%f z:%f\n", vDesiredPos.x, vDesiredPos.y, vDesiredPos.z);




//	CAnimInstanceHdr__Collision(&pIns->ah, &pIns->ah.ih.m_pCurrentRegion, &pIns->ah.ih.m_vPos, vDesiredPos, &ci_nocollisionatall, TRUE, TRUE, TRUE, TRUE);
	pIns->ah.ih.m_vPos = vDesiredPos;
}



// turok control for on ground movement
//
void CTMove__GroundMovement(CTMove *pThis, CEngineApp *pApp, CTControl *pCTControl)
{
	// declare variables
	float							resrl, resrr;
	CGameObjectInstance		*pIns;
	CVector3						vDesiredPos;
	int							waterFlag,
									duckFlag;


#if SPRING_JUMP
	float	resjr;
#else
	float	resj;
#endif

	// get instance pointer to turok
	pIns = CEngineApp__GetPlayer(pApp);
	if (pIns == NULL) return;

	// get player water status
	waterFlag = CEngineApp__GetPlayerWaterStatus(pApp);

	// get player ducking status
	duckFlag = CEngineApp__GetPlayerDuckStatus(pApp);

	// initialize current position into desired position before we change it
	vDesiredPos = pIns->ah.ih.m_vPos;

	// run / walk mode ?
	CTMove__UpdateRunWalkToggle(pThis, pCTControl);


	// only allow rolling if player is not in a ducking area
	if (duckFlag==PLAYER_NOT_DUCKING)
	{
		// *** turok rolling right
		resrr = CTControl__IsRollRight(pCTControl);
		if ( resrr < 0 )
		{
			// initialize - digital roll right
			pThis->Mode      = TMMODE_GRDROLL;
			pThis->HeadZRoll = -ANGLE_PI/4;
			pThis->HeadZRollCnt = TTHEAD_ROLL_COUNT;

			if (CInstanceHdr__IsOnGround(&pIns->ah.ih))
				pIns->ah.m_vVelocity.y = TJUMP_ROLL_FORCE;

			return;
		}

		// *** turok rolling left
		resrl = CTControl__IsRollLeft ( pCTControl );
		if ( resrl < 0 )
		{
			// initialize - digital roll left
			pThis->Mode      = TMMODE_GRDROLL;
			pThis->HeadZRoll = ANGLE_PI/4;
			pThis->HeadZRollCnt = TTHEAD_ROLL_COUNT;

			if (CInstanceHdr__IsOnGround(&pIns->ah.ih))
				pIns->ah.m_vVelocity.y = TJUMP_ROLL_FORCE;

			return;
		}
	}


	// jumping ? - let player fall off ledge a bit before they cannot jump
	resj = CTControl__IsJump(pCTControl);
	if (		(resj < 0)
			&&	(CTControl__IsStartJump(pCTControl)<0)
			&&	(pIns->ah.m_vVelocity.y > (-75.0*SCALING_FACTOR))
			&&	(pThis->JumpAllowed)
//			&&	(waterFlag != PLAYER_ABOVE_WATERSURFACE)	// removed to allow fall jump over water
			&&	(waterFlag != PLAYER_ON_WATERSURFACE)
			&&	(!pThis->CannotJumpFromSurface) )
//			&&	(!(pApp->m_dwCheatFlags & CHEATFLAG_MODIFYENEMY)) )
	{
		// initialize - digital jump
		pThis->Mode		         = TMMODE_GRDJUMP;

		//pThis->JumpUpForce		= 0;
		// jump up force must overpower gravity in order to lift the player at all
		pThis->JumpUpForce		= ci_player.GravityAcceleration * (-1.0/FRAME_FPS);

		pThis->Jumped				= FALSE;
		pThis->DoubleJump			= FALSE;
		AI_DoSound(&pIns->ah.ih, SOUND_GENERIC_21, 1, 0);
		CTMove__StartJump(pThis);
		CTMove__QuietNoise(pThis);
		CTMove__GroundJumpNSMovement(pThis, pApp, pCTControl);
		pThis->CannotJumpFromCliffFace = TRUE;
		return;
	}
	else if (resj==0)
	{
		pThis->CannotJumpFromSurface = FALSE;
	}

	// update movement & turning of turok
	vDesiredPos = CTMove__ControlMovement ( pThis, pApp, pCTControl, vDesiredPos );
	CTMove__Turn ( pThis, pApp, pCTControl );


#define	TMOVE_DELAY_BETWEEN_AUTOREPEAT	0.36

	// only allow weapon changes if player is ducking
	if (		(duckFlag == PLAYER_NOT_DUCKING) )
//			&&	(!(pApp->m_dwCheatFlags & CHEATFLAG_MODIFYENEMY)) )
	{
		// get next weapon ?
		if (		(CTControl__IsSelectWeaponNext ( pCTControl ) )
				&&	(pThis->InMenuTimer == 0.0))
		{
			/* PORT: gate the switch on a LOGIC TICK — on render-only frames (frame_increment==0 at
			 * FPS>TICK) SelectWeaponTimer never grows, so an un-gated "timer==0 -> switch" re-fires every
			 * render frame = weapon cycling far too fast. TUROK_IS_TICK is 1 on the N64 (no behavior change). */
			if (pThis->SelectWeaponTimer == 0.0 && TUROK_IS_TICK)
			{
				CTMove__SelectWeapon ( pThis, TWEAPON_ANY, TRUE );
				CTMove__DisplayWeaponsIcons(pThis, TRUE);
			}

			pThis->SelectWeaponTimer += frame_increment*(1.0/FRAME_FPS);
			if (pThis->SelectWeaponTimer > TMOVE_DELAY_BETWEEN_AUTOREPEAT)
				pThis->SelectWeaponTimer = 0.0;
		}

		// get previous weapon ?
		else if (		(CTControl__IsSelectWeaponPrev(pCTControl))
						&&	(pThis->InMenuTimer == 0.0))
		{
			if (pThis->SelectWeaponTimer == 0.0 && TUROK_IS_TICK)
			{
				CTMove__SelectWeapon ( pThis, TWEAPON_ANY, FALSE );
				CTMove__DisplayWeaponsIcons(pThis, FALSE);
			}

			pThis->SelectWeaponTimer += frame_increment*(1.0/FRAME_FPS);
			if (pThis->SelectWeaponTimer > TMOVE_DELAY_BETWEEN_AUTOREPEAT)
				pThis->SelectWeaponTimer = 0.0;
		}
		else
		{
			pThis->SelectWeaponTimer = 0.0;
		}

#ifdef PLATFORM_PORT
		/* PORT scroll-wheel weapon cycle: consume ONE pending notch per LOGIC TICK so each wheel notch
		 * switches exactly one weapon, render-rate independent (discrete — bypasses the SelectWeaponTimer
		 * autorepeat path the held buttons use). Honors the same not-ducking + not-in-menu guards. */
		if (TUROK_IS_TICK && g_weapon_cycle != 0 && pThis->InMenuTimer == 0.0)
		{
			BOOL next = (g_weapon_cycle > 0) ? TRUE : FALSE;
			CTMove__SelectWeapon ( pThis, TWEAPON_ANY, next );
			CTMove__DisplayWeaponsIcons(pThis, next);
			if (next) g_weapon_cycle--; else g_weapon_cycle++;
		}
#endif
	}

	// do collision test to see how far turok really got
	if (CAnimInstanceHdr__Collision3(&pIns->ah, vDesiredPos, &ci_player))
	{
		// player has collided with something
#ifdef IG_DEBUG
		if (ci_player.pInstanceCollision && ci_player.pWallCollisionRegion)
		{
			TRACE0("CTMove: Player collision with instance and boundary\r\n");
		}
		else if (ci_player.pInstanceCollision)
		{
			TRACE0("CTMove: Player collision with instance\r\n");
		}
		else if (ci_player.pWallCollisionRegion)
		{
			TRACE0("CTMove: Player collision with boundary\r\n");
		}
#endif
	}

	pThis->pLastCI = &ci_player;
	CTMove__DoGrdHeadMovements(pThis);
}


// update head movements for ground movement
//
void CTMove__DoGrdHeadMovements(CTMove *pThis)
{
	float		dm;
	int		duckFlag;


	// get player ducking status
	duckFlag = CEngineApp__GetPlayerDuckStatus(GetApp());

	// *** add head motion to turok
	if ( pThis->MoveSpeed <= -0.05 || pThis->MoveSpeed >= 0.05 )
	{
		// turok is moving - calculate head x rotation for bounce effect
		dm = pThis->MoveSpeed / (TMOVE_MAX_RUNSPEED*1.5);
		pThis->MoveCnt += dm;
		if ( pThis->MoveCnt >= TTHEAD_MOVE_TIME )
		{
			// new head dip starting
			pThis->MoveCnt -= TTHEAD_MOVE_TIME;
		}

		// turok is moving - calculate head y rotation for bounce effect
		dm = pThis->MoveSpeed / (TMOVE_MAX_RUNSPEED*1.5);
		pThis->YRotCnt += dm;
		if ( pThis->YRotCnt >= TTHEAD_YROT_TIME )
		{
			// new head dip starting
			pThis->MoveCnt -= TTHEAD_YROT_TIME;
		}

		// make head dip down farther the faster turok moves forward
#if	0
		if ( pThis->MoveSpeed >= 0.05 )
		{
			// turok is moving forwards
			dm = 1 - cosf ( (TMOVE_MAX_RUNSPEED*1.5-pThis->MoveSpeed) * ANGLE_PI / TMOVE_MAX_RUNSPEED*1.5 );
			dm = dm / TTHEAD_HEADSPD_DIP;

			// make head come back up after it has dipped down - we do not want to keep looking at our feet
			pThis->RunTime += TTHEAD_RUN_DIPUP;
			if ( dm > 0 )
			{
				dm -= pThis->RunTime;
				if ( dm < 0 ) dm = 0;
			}
			pThis->SpdChgX = dm;
		}
		else if ( pThis->MoveSpeed <= -0.05 )
		{
			// turok is moving backwards
//			pThis->SpdChgX = TTHEAD_HEADBACK_DIP;
			pThis->SpdChgX = 0;
		}
#endif

		// make speed change x offset match the speed change x
		dm  = pThis->SpdChgX - pThis->SpdChgXOffset;
		dm  = dm / TTHEAD_SPDCHGX_SMOOTH;
		pThis->SpdChgXOffset += dm;


		// calculate head x rotation offset for head bob movement
		dm              = sin ( pThis->MoveCnt * 2 * ANGLE_PI / TTHEAD_MOVE_TIME );
		pThis->HeadXRot = dm / (TTHEAD_XROT_AMPLITUDE + (RANDOM(5)-2));

		// calculate head y position offset for head bob movement
		pThis->HeadYPosCFrame += (2*frame_increment/FRAME_FPS);
		while (pThis->HeadYPosCFrame > 1)
			pThis->HeadYPosCFrame -= 1;
		pThis->HeadYPos = TTHEAD_YPOS_AMPLITUDE*sin(pThis->HeadYPosCFrame*2*ANGLE_PI);

		// calculate head y rotation offset for head bob movement
		if (duckFlag == PLAYER_NOT_DUCKING)
		{
			dm              = sin(pThis->YRotCnt * 2 * ANGLE_PI / TTHEAD_YROT_TIME);
			pThis->HeadYRot = dm / (TTHEAD_YROT_AMPLITUDE + (RANDOM(5)-2));
		}
		else
		{
			dm              = sin(pThis->YRotCnt * 2 * ANGLE_PI / TTHEAD_YROT_TIME);
			pThis->HeadYRot = dm / 30.0;
		}

		// calculate head z rotation offset for head bob movement
		pThis->HeadZRot = dm / (TTHEAD_ZROT_AMPLITUDE + (RANDOM(5)-2));

		// reduce head x rotation the faster turok moves
		dm                = pThis->MoveSpeed * TTHEAD_SPDXROT_TIGHTEN / TMOVE_MAX_RUNSPEED;
		if ( dm < 1 ) dm  = 1;
		pThis->HeadXRot  /= dm;

		// reduce head y rotation the faster turok moves
		dm                = pThis->MoveSpeed * TTHEAD_SPDYROT_TIGHTEN / TMOVE_MAX_RUNSPEED;
		if ( dm < 1 ) dm  = 1;
		pThis->HeadYRot  /= dm;

		// reduce head z rotation the faster turok moves
		dm                = pThis->MoveSpeed * TTHEAD_SPDZROT_TIGHTEN / TMOVE_MAX_RUNSPEED;
		if ( dm < 1 ) dm  = 1;
		pThis->HeadZRot  /= dm;

		// reduce head y position offset the faster turok moves
		dm                = pThis->MoveSpeed * TTHEAD_SPDYPOS_TIGHTEN / TMOVE_MAX_RUNSPEED;
		if ( dm < 1 ) dm  = 1;
		pThis->HeadYPos  /= dm;


#define	TMOVE_SIDESTEP_ANGLE		-0.3
		// do side step z rotation motion
		dm = (pThis->SideStepSpeed * ANGLE_DTOR(TMOVE_SIDESTEP_ANGLE) - pThis->SideStepZRot) / 5;
		pThis->SideStepZRot += dm;

		dm = (pThis->SideStepZRot - pThis->HeadZRot) / 12;
		pThis->HeadZRot += dm;
	}
	else
	{
		// reset move counter
		pThis->MoveCnt = 0;
		pThis->YRotCnt = 0;
		pThis->SpdChgX = pThis->SpdChgXOffset;
		pThis->RunTime = 0;

		CTMove__ClearHeadRotations(pThis);

		// stop speed change x offset rotation
		if ( pThis->SpdChgXOffset > 0 )
		{
			dm = pThis->SpdChgX / (TTHEAD_SPDCHGX_SMOOTH*3);
			pThis->SpdChgXOffset -= dm;
			if ( pThis->SpdChgXOffset < 0 ) pThis->SpdChgXOffset = 0;
		}
		else if ( pThis->SpdChgXOffset < 0 )
		{
			dm = pThis->SpdChgX / (TTHEAD_SPDCHGX_SMOOTH*3);
			pThis->SpdChgXOffset -= dm;
			if ( pThis->SpdChgXOffset > 0 ) pThis->SpdChgXOffset = 0;
		}

		// stop head y position
		if ( pThis->HeadYPos > 0 )
		{
			dm = pThis->HeadYPos / TTHEAD_YPOS_SMOOTH;
			pThis->HeadYPos -= dm;
			if ( pThis->HeadYPos < 0 ) pThis->HeadYPos = 0;
		}
		else if ( pThis->HeadYPos < 0 )
		{
			dm = pThis->HeadYPos / TTHEAD_YPOS_SMOOTH;
			pThis->HeadYPos -= dm;
			if ( pThis->HeadYPos > 0 ) pThis->HeadYPos = 0;
		}

		// do side step z rotation motion
		dm = (pThis->SideStepSpeed * ANGLE_DTOR(TMOVE_SIDESTEP_ANGLE) - pThis->SideStepZRot) / 5;
		pThis->SideStepZRot += dm;

		dm = (pThis->SideStepZRot - pThis->HeadZRot) / 12;
		pThis->HeadZRot += dm;
	}
}


// clear head x,y & z rotations
//
void CTMove__ClearHeadRotations(CTMove *pThis)
{
	float	dm;


	// stop head x rotation
	if ( pThis->HeadXRot > 0 )
	{
		dm = pThis->HeadXRot / TTHEAD_XROT_SMOOTH;
		pThis->HeadXRot -= dm;
		if ( pThis->HeadXRot < 0 ) pThis->HeadXRot = 0;
	}
	else if ( pThis->HeadXRot < 0 )
	{
		dm = pThis->HeadXRot / TTHEAD_XROT_SMOOTH;
		pThis->HeadXRot -= dm;
		if ( pThis->HeadXRot > 0 ) pThis->HeadXRot = 0;
	}

	// stop head y rotation
	if ( pThis->HeadYRot > 0 )
	{
		dm = pThis->HeadYRot / TTHEAD_YROT_SMOOTH;
		pThis->HeadYRot -= dm;
		if ( pThis->HeadYRot < 0 ) pThis->HeadYRot = 0;
	}
	else if ( pThis->HeadYRot < 0 )
	{
		dm = pThis->HeadYRot / TTHEAD_YROT_SMOOTH;
		pThis->HeadYRot -= dm;
		if ( pThis->HeadYRot > 0 ) pThis->HeadYRot = 0;
	}

	// stop head z rotation
	if ( pThis->HeadZRot > 0 )
	{
		dm = pThis->HeadZRot / TTHEAD_ZROT_SMOOTH;
		pThis->HeadZRot -= dm;
		if ( pThis->HeadZRot < 0 ) pThis->HeadZRot = 0;
	}
	else if ( pThis->HeadZRot < 0 )
	{
		dm = pThis->HeadZRot / TTHEAD_ZROT_SMOOTH;
		pThis->HeadZRot -= dm;
		if ( pThis->HeadZRot > 0 ) pThis->HeadZRot = 0;
	}
}

// let turok move
//
CVector3 CTMove__ControlMovement(CTMove *pThis, CEngineApp *pApp, CTControl *pCTControl, CVector3 vDesiredPos)
{
	// declare variables
	CGameObjectInstance *pIns;


	// get instance pointer to turok
	pIns = CEngineApp__GetPlayer(pApp);
	if ( pIns == NULL ) return vDesiredPos;

	// initialize current position into desired position before we change it
	vDesiredPos = pIns->ah.ih.m_vPos;

	// update turok sidestepping
	vDesiredPos = CTMove__ControlFBward  (pThis, pApp, pCTControl, vDesiredPos);
	vDesiredPos = CTMove__ControlSideStep(pThis, pApp, pCTControl, vDesiredPos);


	return vDesiredPos;
}



// let turok fly up & down
//
CVector3 CTMove__ControlFlyUpDown(CTMove *pThis, CEngineApp *pApp, CTControl *pCTControl, CVector3 vDesiredPos)
{
	// declare variables
	float						resu, resd, dm;
	CGameObjectInstance	*pIns;


#define	TMOVE_MAX_UDSPEED				(8*SCALING_FACTOR)


	// get instance pointer to turok
	pIns = CEngineApp__GetPlayer(pApp);
	if (pIns == NULL) return vDesiredPos;

	// *** turok flying up
	resu = CTControl__IsFlyUp(pCTControl);
	if (resu)
	{
		dm = TMOVE_MAX_UDSPEED - pThis->JumpUpForce;
		dm = dm / 6;

		pThis->JumpUpForce += dm;
		vDesiredPos.y += pThis->JumpUpForce*frame_increment;
	}

	// *** turok flying down
	resd = CTControl__IsFlyDown(pCTControl);
	if (resd)
	{
		dm = -TMOVE_MAX_UDSPEED - pThis->JumpUpForce;
		dm = dm / 6;

		pThis->JumpUpForce += dm;
		vDesiredPos.y += pThis->JumpUpForce*frame_increment;
	}

	// *** reduce movement speed if there is no vertical motion
	if (resu == 0 && resd == 0)
	{
		dm = 0 - pThis->JumpUpForce;
		dm = dm / 2;

		pThis->JumpUpForce += dm;
		vDesiredPos.y += pThis->JumpUpForce*frame_increment;
	}


	// return desired position
	return vDesiredPos;
}




// let turok fly forwards & backwards
//
CVector3 CTMove__ControlFlyFBward(CTMove *pThis, CEngineApp *pApp, CTControl *pCTControl, CVector3 vDesiredPos)
{
	// declare variables
	float						resb, resf, dm,
								sinRotY,  cosRotY;
	CGameObjectInstance	*pIns;


#define	TMOVE_MAX_MOVESPEED				(8*SCALING_FACTOR)
#define	TMOVE_MAX_BACKMOVESPEED			(7*SCALING_FACTOR)


	// get instance pointer to turok
	pIns = CEngineApp__GetPlayer(pApp);
	if (pIns == NULL) return vDesiredPos;

	// calculate sin & cos for straight ahead direction
	sinRotY = sin(pIns->m_RotY + ANGLE_PI);
	cosRotY = cos(pIns->m_RotY + ANGLE_PI);

	// *** turok flying forwards
	resf = CTControl__IsFlyForward(pCTControl);
	if (resf)
	{
		dm = TMOVE_MAX_MOVESPEED - pThis->MoveSpeed;
		dm = dm / 6;

		pThis->MoveSpeed += dm;
		vDesiredPos.x += pThis->MoveSpeed*frame_increment*sinRotY;
		vDesiredPos.z += pThis->MoveSpeed*frame_increment*cosRotY;
	}

	// *** turok flying backwards
	resb = CTControl__IsFlyBackward(pCTControl);
	if (resb)
	{
		dm = -TMOVE_MAX_BACKMOVESPEED - pThis->MoveSpeed;
		dm = dm / 6;

		pThis->MoveSpeed += dm;
		vDesiredPos.x += pThis->MoveSpeed*frame_increment*sinRotY;
		vDesiredPos.z += pThis->MoveSpeed*frame_increment*cosRotY;
	}

	// *** reduce movement speed if there is no forwards or bacwards motion
	if (resf == 0 && resb == 0)
	{
		dm = 0 - pThis->MoveSpeed;
		dm = dm / 2;

		pThis->MoveSpeed += dm;
		vDesiredPos.x += pThis->MoveSpeed*frame_increment*sinRotY;
		vDesiredPos.z += pThis->MoveSpeed*frame_increment*cosRotY;
	}


	// return desired position
	return vDesiredPos;
}



// let turok do side step motion
//
CVector3 CTMove__ControlFlySideStep(CTMove *pThis, CEngineApp *pApp, CTControl *pCTControl, CVector3 vDesiredPos)
{
	// declare variables
	float						resl, resr, dm,
								sinSideY, cosSideY;
	CGameObjectInstance	*pIns;


#define	TMOVE_MAX_FLYSIDESTEPSPEED			(7.8*SCALING_FACTOR)


	// get instance pointer to turok
	pIns = CEngineApp__GetPlayer(pApp);
	if (pIns == NULL) return vDesiredPos;

	// calculate sin & cos for side stepping right direction
	sinSideY = sin(pIns->m_RotY - ANGLE_PI/2);
	cosSideY = cos(pIns->m_RotY - ANGLE_PI/2);

	// *** turok flying left
	resl = CTControl__IsFlyLeft(pCTControl);
	if (resl)
	{
		dm = -TMOVE_MAX_FLYSIDESTEPSPEED - pThis->SideStepSpeed;
		dm = dm / 6;

		pThis->SideStepSpeed += dm;
		vDesiredPos.x += pThis->SideStepSpeed*frame_increment*sinSideY;
		vDesiredPos.z += pThis->SideStepSpeed*frame_increment*cosSideY;
	}

	// *** turok flying right
	resr = CTControl__IsFlyRight(pCTControl);
	if (resr)
	{
		dm = TMOVE_MAX_FLYSIDESTEPSPEED - pThis->SideStepSpeed;
		dm = dm / 6;

		pThis->SideStepSpeed += dm;
		vDesiredPos.x += pThis->SideStepSpeed*frame_increment*sinSideY;
		vDesiredPos.z += pThis->SideStepSpeed*frame_increment*cosSideY;
	}

	// *** reduce movement speed if there is no side motion
	if (resl == 0 && resr == 0)
	{
		dm = 0 - pThis->SideStepSpeed;
		dm = dm / 2;

		pThis->SideStepSpeed += dm;
		vDesiredPos.x += pThis->SideStepSpeed*frame_increment*sinSideY;
		vDesiredPos.z += pThis->SideStepSpeed*frame_increment*cosSideY;
	}


	// return desired position
	return vDesiredPos;
}



// let turok move forwards & backwards
//
CVector3 CTMove__ControlFBward(CTMove *pThis, CEngineApp *pApp, CTControl *pCTControl, CVector3 vDesiredPos)
{
	// declare variables
	float						resb, resf, dm,
								sinRotY,  cosRotY;
	CGameObjectInstance	*pIns;
	int						duckFlag;


#define	TMOVE_MAX_DUCKMOVESPEED					(0.7*SCALING_FACTOR)
#define	TMOVE_MAX_BACKDUCKMOVESPEED			(0.5*SCALING_FACTOR)
#define	TMOVE_DUCKMOVE_SMOOTH					6.0
#define	TMOVE_BACKDUCKMOVE_SMOOTH				4.0

// in lava speeds
#define	TMOVE_MAX_DUCKMOVESPEEDLAVA			(0.3*SCALING_FACTOR)
#define	TMOVE_MAX_BACKDUCKMOVESPEEDLAVA		(0.2*SCALING_FACTOR)
#define	TMOVE_MAX_WALKSPEEDLAVA					(0.7*SCALING_FACTOR)
#define	TMOVE_MAX_BACKWALKSPEEDLAVA			(0.5*SCALING_FACTOR)
#define	TMOVE_MAX_RUNSPEEDLAVA					(1.4*SCALING_FACTOR)
#define	TMOVE_MAX_BACKRUNSPEEDLAVA				(0.9*SCALING_FACTOR)


	// get instance pointer to turok
	pIns = CEngineApp__GetPlayer(pApp);
	if (pIns == NULL) return vDesiredPos;

	// get player ducking status
	duckFlag = CEngineApp__GetPlayerDuckStatus(pApp);

	// calculate sin & cos for straight ahead direction
	sinRotY = sin(pIns->m_RotY + ANGLE_PI);
	cosRotY = cos(pIns->m_RotY + ANGLE_PI);

#ifdef PLATFORM_PORT
	/* Port dual-analog FORWARD/BACK seam (the 3DS move-stick / nub Y; + = forward, - = backward). Turok's N64
	 * analog stick is the LOOK stick (its forward = LookUp, CTTYPE_STICK_SFORWARD), so an analog move stick must
	 * translate the player HERE, not via stick_y. Injected as a proportional velocity along the facing dir,
	 * exactly like the analogue-forward paths below; frame_increment-gated (a LEVEL, no FPS coupling). Additive
	 * with the C-button/keyboard digital forward. g_turok_forward lives in input.c (always-linked). */
	{	extern float g_turok_forward;
		/* Gate on the SAME lockouts the engine uses to neutralize player control — the seam bypasses
		 * PlayerControllerData (which control.c zeroes with no controller / at demo start, and attract.c
		 * OVERWRITES with the recorded demo during playback), so without this a nub nudge would desync a
		 * playing/recording attract demo or drive a cutscene. Normal play: both false -> seam applies. */
		if (g_turok_forward != 0.0f && !CAttractDemo__Active()
		    && !CCamera__InCinemaMode(&pApp->m_Camera))
		{
			float sp = g_turok_forward * TUROK_WALKCAP(TMOVE_MAX_RUNSPEED);
			vDesiredPos.x += sp*frame_increment*sinRotY;
			vDesiredPos.z += sp*frame_increment*cosRotY;
			player_is_moving = TRUE;
			CTMove__QuietNoise(pThis);
		}
	}
#endif

	// player cannot run while ducking
	if (duckFlag == PLAYER_NOT_DUCKING)
	{
		resf = CTControl__IsRunForward(pCTControl);
		if (pThis->RunWalkToggle && resf==0)
			resf = CTControl__IsForward(pCTControl);
	}
	else
	{
		resf = 0;
	}


	// *** turok running forwards
	if (resf < 0)
	{
		// digital forward
		if (pThis->InLava)
		{
			dm = TMOVE_MAX_RUNSPEEDLAVA - pThis->MoveSpeed;
			dm = dm / TMOVE_RUNFORWARD_SMOOTH;
		}
		else
		{
			dm = TUROK_WALKCAP(TMOVE_MAX_RUNSPEED) - pThis->MoveSpeed;
			dm = dm / TMOVE_RUNFORWARD_SMOOTH;
		}
		pThis->MoveSpeed += dm;
		vDesiredPos.x += pThis->MoveSpeed*frame_increment*sinRotY;
		vDesiredPos.z += pThis->MoveSpeed*frame_increment*cosRotY;
		CTMove__LoudNoise(pThis);
	}
	else if (resf > 0)
	{
		// analogue forward
		vDesiredPos.x += resf*frame_increment*sinRotY;
		vDesiredPos.z += resf*frame_increment*cosRotY;
		CTMove__LoudNoise(pThis);
	}


	// *** turok moving forwards (only if not running forwards)
	if (resf == 0)
	{
		resf = CTControl__IsForward(pCTControl);
		if (resf < 0)
		{
			// digital forward
			if (duckFlag == PLAYER_NOT_DUCKING)
			{
				if (pThis->InLava)
				{
					dm = TMOVE_MAX_WALKSPEEDLAVA - pThis->MoveSpeed;
					dm = dm / TMOVE_FORWARD_SMOOTH;
				}
				else
				{
					dm = TMOVE_MAX_WALKSPEED - pThis->MoveSpeed;
					dm = dm / TMOVE_FORWARD_SMOOTH;
				}
			}
			else
			{
				if (pThis->InLava)
				{
					dm = TMOVE_MAX_DUCKMOVESPEEDLAVA - pThis->MoveSpeed;
					dm = dm / TMOVE_DUCKMOVE_SMOOTH;
				}
				else
				{
					dm = TMOVE_MAX_DUCKMOVESPEED - pThis->MoveSpeed;
					dm = dm / TMOVE_DUCKMOVE_SMOOTH;
				}
			}
			pThis->MoveSpeed += dm;
			vDesiredPos.x += pThis->MoveSpeed*frame_increment*sinRotY;
			vDesiredPos.z += pThis->MoveSpeed*frame_increment*cosRotY;
			CTMove__QuietNoise(pThis);
		}
		else if (resf > 0)
		{
			// analogue forward
			vDesiredPos.x += resf*frame_increment*sinRotY;
			vDesiredPos.z += resf*frame_increment*cosRotY;
			CTMove__QuietNoise(pThis);
		}
	}


	// player cannot run while ducking
	if (duckFlag == PLAYER_NOT_DUCKING)
	{
		resb = CTControl__IsRunBackward(pCTControl);
		if (pThis->RunWalkToggle && resb==0)
			resb = CTControl__IsBackward(pCTControl);
	}
	else
	{
		resb = 0;
	}


	// *** turok running backwards
	if (resb < 0)
	{
		// digital forward
		if (pThis->InLava)
		{
			dm = -TMOVE_MAX_BACKRUNSPEEDLAVA - pThis->MoveSpeed;
			dm = dm / TMOVE_RUNFORWARD_SMOOTH;
		}
		else
		{
			dm = TUROK_WALKCAP(-TMOVE_MAX_BACKRUNSPEED) - pThis->MoveSpeed;
			dm = dm / TMOVE_RUNFORWARD_SMOOTH;
		}
		pThis->MoveSpeed += dm;
		vDesiredPos.x += pThis->MoveSpeed*frame_increment*sinRotY;
		vDesiredPos.z += pThis->MoveSpeed*frame_increment*cosRotY;
		CTMove__LoudNoise(pThis);
	}
	else if (resb > 0)
	{
		// analogue forward
		vDesiredPos.x -= resf*frame_increment*sinRotY;
		vDesiredPos.z -= resf*frame_increment*cosRotY;
		CTMove__LoudNoise(pThis);
	}


	// *** turok moving backwards
	if (resb == 0)
	{
		resb = CTControl__IsBackward(pCTControl);
		if (resb < 0)
		{
			// digital backward
			if (duckFlag == PLAYER_NOT_DUCKING)
			{
				if (pThis->InLava)
				{
					dm = -TMOVE_MAX_BACKWALKSPEEDLAVA - pThis->MoveSpeed;
					dm = dm / TMOVE_BACKWARD_SMOOTH;
				}
				else
				{
					dm = -TMOVE_MAX_BACKWALKSPEED - pThis->MoveSpeed;
					dm = dm / TMOVE_BACKWARD_SMOOTH;
				}
			}
			else
			{
				if (pThis->InLava)
				{
					dm = -TMOVE_MAX_BACKDUCKMOVESPEEDLAVA - pThis->MoveSpeed;
					dm = dm / TMOVE_BACKDUCKMOVE_SMOOTH;
				}
				else
				{
					dm = -TMOVE_MAX_BACKDUCKMOVESPEED - pThis->MoveSpeed;
					dm = dm / TMOVE_BACKDUCKMOVE_SMOOTH;
				}
			}
			pThis->MoveSpeed += dm;

			vDesiredPos.x += pThis->MoveSpeed*frame_increment*sinRotY;
			vDesiredPos.z += pThis->MoveSpeed*frame_increment*cosRotY;
			CTMove__QuietNoise(pThis);
		}
		else if (resb > 0)
		{
			// analogue backward
			vDesiredPos.x -= resb*frame_increment*sinRotY;
			vDesiredPos.z -= resb*frame_increment*cosRotY;
			CTMove__QuietNoise(pThis);
		}
	}

	// *** reduce movement speed if there is no forwards or bacwards motion
	if (resf == 0 && resb == 0)
	{
		dm = 0 - pThis->MoveSpeed;
		dm = dm / TMOVE_STOP_SMOOTH;
		pThis->MoveSpeed += dm;
		vDesiredPos.x += pThis->MoveSpeed*frame_increment*sinRotY;
		vDesiredPos.z += pThis->MoveSpeed*frame_increment*cosRotY;
	}
	else
	{
		player_is_moving = TRUE;
	}

	// return desired position
	return vDesiredPos;
}




// let turok do side step motion
//
CVector3 CTMove__ControlSideStep(CTMove *pThis, CEngineApp *pApp, CTControl *pCTControl, CVector3 vDesiredPos)
{
	// declare variables
	float						dm,
								ressl, ressr,
								sinSideY, cosSideY;
	CGameObjectInstance	*pIns;
	int						duckFlag;


#define	TMOVE_MAX_DUCKSIDESTEPSPEED		(0.8*SCALING_FACTOR)
#define	TMOVE_DUCKSIDESTEP_SMOOTH			4.0

#define	TMOVE_MAX_DUCKSIDESTEPSPEEDLAVA	(0.4*SCALING_FACTOR)
#define	TMOVE_MAX_SIDESTEPSPEEDLAVA		(1.4*SCALING_FACTOR)

	// get instance pointer to turok
	pIns = CEngineApp__GetPlayer(pApp);
	if ( pIns == NULL ) return vDesiredPos;

	// get player ducking status
	duckFlag = CEngineApp__GetPlayerDuckStatus(pApp);

	// calculate sin & cos for side stepping right direction
	sinSideY = sin(pIns->m_RotY - ANGLE_PI/2);
	cosSideY = cos(pIns->m_RotY - ANGLE_PI/2);

#ifdef PLATFORM_PORT
	/* Port dual-analog STRAFE seam (the 3DS move-stick X; + = right). Injected as an analog sidestep velocity
	 * exactly like the ressr>0 / ressl>0 analog paths below — a LEVEL scaled by frame_increment (tick-gated),
	 * additive with the C-button/keyboard digital strafe. g_turok_strafe lives in input.c (always-linked). */
	{	extern float g_turok_strafe;
		/* Gate on the same lockouts as the forward seam (see CTMove__ControlFBward) — ignore the analog
		 * strafe during attract demo record/playback and cinematics; normal play applies it. */
		if (g_turok_strafe != 0.0f && !CAttractDemo__Active()
		    && !CCamera__InCinemaMode(&pApp->m_Camera))
		{
			float sp = g_turok_strafe * TUROK_WALKCAP(TMOVE_MAX_SIDESTEPSPEED);
			vDesiredPos.x += sp*frame_increment*sinSideY;
			vDesiredPos.z += sp*frame_increment*cosSideY;
			player_is_moving = TRUE;
			CTMove__QuietNoise(pThis);
		}
	}
#endif

	// *** turok side stepping right
	ressr = CTControl__IsSideStepRight ( pCTControl );
	if ( ressr < 0 )
	{
		// digital side step right
		if (duckFlag == PLAYER_NOT_DUCKING)
		{
			if (pThis->InLava)
			{
				dm = TMOVE_MAX_SIDESTEPSPEEDLAVA - pThis->SideStepSpeed;
				dm = dm / TMOVE_SIDESTEP_SMOOTH;
			}
			else
			{
				dm = TUROK_WALKCAP(TMOVE_MAX_SIDESTEPSPEED) - pThis->SideStepSpeed;
				dm = dm / TMOVE_SIDESTEP_SMOOTH;
			}
		}
		else
		{
			if (pThis->InLava)
			{
				dm = TMOVE_MAX_DUCKSIDESTEPSPEEDLAVA - pThis->SideStepSpeed;
				dm = dm / TMOVE_DUCKSIDESTEP_SMOOTH;
			}
			else
			{
				dm = TMOVE_MAX_DUCKSIDESTEPSPEED - pThis->SideStepSpeed;
				dm = dm / TMOVE_DUCKSIDESTEP_SMOOTH;
			}
		}
		pThis->SideStepSpeed += dm;
		vDesiredPos.x += pThis->SideStepSpeed*frame_increment*sinSideY;
		vDesiredPos.z += pThis->SideStepSpeed*frame_increment*cosSideY;
		CTMove__QuietNoise(pThis);
	}
	else if ( ressr > 0 )
	{
		// analogue side step right
		vDesiredPos.x += ressr*frame_increment*sinSideY;
		vDesiredPos.z += ressr*frame_increment*cosSideY;
		CTMove__QuietNoise(pThis);
	}


	// *** turok side stepping left
	ressl = CTControl__IsSideStepLeft ( pCTControl );
	if ( ressl < 0 )
	{
		// digital side step left
		if (duckFlag == PLAYER_NOT_DUCKING)
		{
			if (pThis->InLava)
			{
				dm = -TMOVE_MAX_SIDESTEPSPEEDLAVA - pThis->SideStepSpeed;
				dm = dm / TMOVE_SIDESTEP_SMOOTH;
			}
			else
			{
				dm = TUROK_WALKCAP(-TMOVE_MAX_SIDESTEPSPEED) - pThis->SideStepSpeed;
				dm = dm / TMOVE_SIDESTEP_SMOOTH;
			}
		}
		else
		{
			if (pThis->InLava)
			{
				dm = -TMOVE_MAX_DUCKSIDESTEPSPEEDLAVA - pThis->SideStepSpeed;
				dm = dm / TMOVE_DUCKSIDESTEP_SMOOTH;
			}
			else
			{
				dm = -TMOVE_MAX_DUCKSIDESTEPSPEED - pThis->SideStepSpeed;
				dm = dm / TMOVE_DUCKSIDESTEP_SMOOTH;
			}
		}

		pThis->SideStepSpeed += dm;
		vDesiredPos.x += pThis->SideStepSpeed*frame_increment*sinSideY;
		vDesiredPos.z += pThis->SideStepSpeed*frame_increment*cosSideY;
		CTMove__QuietNoise(pThis);
	}
	else if ( ressl > 0 )
	{
		// analogue side step left
		vDesiredPos.x -= ressl*frame_increment*sinSideY;
		vDesiredPos.z -= ressl*frame_increment*cosSideY;
		CTMove__QuietNoise(pThis);
	}

	// *** reduce sidestep speed if there is no there is no side motion
	if ( ressr == 0 && ressl == 0 )
	{
		dm = 0 - pThis->SideStepSpeed;
		dm = dm / TMOVE_STOP_SMOOTH;
		pThis->SideStepSpeed += dm;
		vDesiredPos.x += pThis->SideStepSpeed*frame_increment*sinSideY;
		vDesiredPos.z += pThis->SideStepSpeed*frame_increment*cosSideY;
	}
	else
	{
		player_is_moving = TRUE;
	}

	// return desired position
	return vDesiredPos;
}




// turok control for on ground roll movement
//
void CTMove__GroundRollMovement ( CTMove *pThis, CEngineApp *pApp, CTControl *pCTControl )
{
	// declare variables
	float		dm, ss, rs;
	float		sinSideY, cosSideY;
	CGameObjectInstance *pIns;
	CVector3					vDesiredPos;


	// get instance pointer to turok
	pIns = CEngineApp__GetPlayer(pApp);
	if ( pIns == NULL ) return;

	// turok moves view
//	pApp->m_RotYOffset = (CTControl__IsLookRight(pCTControl) - CTControl__IsLookLeft(pCTControl))*ANGLE_DTOR( 90)/80;
//	pApp->m_RotXOffset = (CTControl__IsLookUp   (pCTControl) - CTControl__IsLookDown(pCTControl))*ANGLE_DTOR(-60)/80;

	// turn turok
	CTMove__Turn ( pThis, pApp, pCTControl );

	// initialize current position into desired position before we change it
	vDesiredPos = pIns->ah.ih.m_vPos;

	// calculate sin & cos for side stepping right direction
	sinSideY = sin(pIns->m_RotY - ANGLE_PI/2);
	cosSideY = cos(pIns->m_RotY - ANGLE_PI/2);

	// changing roll speed ?
	rs = CTControl__IsRollSpeedL ( pCTControl ) + CTControl__IsRollSpeedR ( pCTControl );
	if (rs < 0)
	{
		// digital roll speed control
		rs = 80;
	}
	rs = rs * 0.2;
	rs = rs * (rs*0.3);

	// make camera roll over
	if ( pThis->HeadZRollCnt >= (TTHEAD_ROLL_COUNT*4/6) )
	{
		// start & middle of roll
		if ( rs < TMOVE_MAX_ROLLSPEED ) rs = TMOVE_MAX_ROLLSPEED;

		dm = pThis->HeadZRoll - pThis->HeadZRot;
		dm = dm / (TTHEAD_ZROLL_SMOOTH*22);
//		pThis->HeadZRot += dm;
		if ( pThis->HeadZRoll < 0 )
			ss = +rs - pThis->SideStepSpeed;
		else
			ss = -rs - pThis->SideStepSpeed;
		ss = ss / TMOVE_SIDESTEP_SMOOTH;
	}
	else if ( pThis->HeadZRollCnt >= (TTHEAD_ROLL_COUNT*1/6) && pThis->HeadZRollCnt < (TTHEAD_ROLL_COUNT*4/6) )
	{
		// middle of roll
		dm = pThis->HeadZRoll - pThis->HeadZRot;
		dm = dm / (TTHEAD_ZROLL_SMOOTH*8);
//		pThis->HeadZRot += (dm*1.3);

		if ( pThis->HeadZRoll < 0 )
			ss = +rs - pThis->SideStepSpeed;
		else
			ss = -rs - pThis->SideStepSpeed;
		ss = ss / TMOVE_SIDESTEP_SMOOTH;

		// turok is invincible while in middle of rolling
		pIns->m_AI.m_Invincible = TRUE;
	}
	else if ( pThis->HeadZRollCnt >= 0 && pThis->HeadZRollCnt < (TTHEAD_ROLL_COUNT*1/6) )
	{
		// end of roll
		ss = 0 - pThis->SideStepSpeed;
		ss = ss / TMOVE_SIDESTEP_SMOOTH;

		// stop head z rotation
		if ( pThis->HeadZRot > 0 )
		{
			dm = (0-pThis->HeadZRot) / TTHEAD_ZROT_SMOOTH;
//			pThis->HeadZRot += dm;
//			if ( pThis->HeadZRot < 0 ) pThis->HeadZRot = 0;
		}
		else if ( pThis->HeadZRot < 0 )
		{
			dm = (0-pThis->HeadZRot) / TTHEAD_ZROT_SMOOTH;
//			pThis->HeadZRot += dm;
//			if ( pThis->HeadZRot > 0 ) pThis->HeadZRot = 0;
		}



		// turok is no longer invincible
		pIns->m_AI.m_Invincible = FALSE;
	}


	// move turok in side motion, left or right ?
	pThis->SideStepSpeed += ss;
	vDesiredPos.x += pThis->SideStepSpeed*frame_increment * sinSideY;
	vDesiredPos.z += pThis->SideStepSpeed*frame_increment * cosSideY;

	// if sidestep motion is less than normal sidestep motion allow the player to do the sidestep
	if (pThis->SideStepSpeed > -TMOVE_MAX_SIDESTEPSPEED &&
		 pThis->SideStepSpeed <  TMOVE_MAX_SIDESTEPSPEED)
	{
		vDesiredPos = CTMove__ControlSideStep(pThis, pApp, pCTControl, vDesiredPos);
	}
	vDesiredPos = CTMove__ControlFBward  (pThis, pApp, pCTControl, vDesiredPos);

	// do collision test to see how far turok really got
	if (CAnimInstanceHdr__Collision3(&pIns->ah, vDesiredPos, &ci_player))
	{
		// turok has collided with something
		TRACE0("Viewer colliding with boundary during roll.\r\n");
	}
	pThis->pLastCI = &ci_player;

	// end of z roll ?
	pThis->HeadZRollCnt--;
	if ( pThis->HeadZRollCnt < 0 )
	{
		// which mode should we go back to (for now just go back to ground mode)
		pThis->Mode = TMMODE_GROUND;
		return;
	}


	// make camera roll on z-axis
#ifdef Z_ROLL
	pThis->RotZPlayer -= (-pThis->HeadZRot);
#endif
}




// turok control for on ground jump movement with no spring
//
void CTMove__GroundJumpNSMovement ( CTMove *pThis, CEngineApp *pApp, CTControl *pCTControl )
{
	// declare variables
	float						dm;
	CGameObjectInstance	*pIns;
	CVector3					vDesiredPos;
	int						waterFlag;


	// get instance pointer to turok
	pIns = CEngineApp__GetPlayer(pApp);
	if ( pIns == NULL ) return;

	// initialize current position into desired position before we change it
	vDesiredPos = pIns->ah.ih.m_vPos;

	// update turning & movement of turok
	vDesiredPos = CTMove__ControlMovement ( pThis, pApp, pCTControl, vDesiredPos );
	CTMove__Turn ( pThis, pApp, pCTControl );

//#define JUMP_SPECIAL		12.402036
#define JUMP_SPECIAL		14.28292

	// is jump pressed ?
	if ( CTControl__IsJump ( pCTControl ) && !pThis->Jumped )
	{
		dm = ((((float)TJUMP_UPFORCE_MAX)*JUMP_SPECIAL) - pThis->JumpUpForce) / 8;

		pThis->JumpUpForce += dm*2*frame_increment;
		pIns->ah.m_vVelocity.y = pThis->JumpUpForce;

		if ( pThis->JumpUpForce > (float)TJUMP_UPFORCE_MAX/2*JUMP_SPECIAL)
		{
			pThis->JumpUpForce  = (float)TJUMP_UPFORCE_MAX/2*JUMP_SPECIAL;
			pThis->Jumped       = TRUE;
			pIns->ah.m_vVelocity.y = pThis->JumpUpForce;
		}
	}
	else
	{
		pThis->Jumped			= TRUE;
	}


	// do collision test to see how far turok really got
	waterFlag = CEngineApp__GetPlayerWaterStatus(pApp);
	if (    (waterFlag == PLAYER_ON_WATERSURFACE)
		  || (waterFlag == PLAYER_BELOW_WATERSURFACE) )
	{
		if (waterFlag == PLAYER_ON_WATERSURFACE)
		{
			CAnimInstanceHdr__Collision3(&pIns->ah, vDesiredPos, &ci_playeronwatersurface);
			pThis->pLastCI = &ci_playeronwatersurface;
		}
		else
		{
			CAnimInstanceHdr__Collision3(&pIns->ah, vDesiredPos, &ci_playerunderwater);
			pThis->pLastCI = &ci_playerunderwater;
		}
	}
	else
	{
		CAnimInstanceHdr__Collision3(&pIns->ah, vDesiredPos, &ci_player);
		pThis->pLastCI = &ci_player;
//		{
//			// turok has collided with something
//			TRACE0("Viewer colliding with boundary during jump.\r\n");
//		}
	}


	// has turok landed on ground after jumping ?
	if (CInstanceHdr__IsOnGround(&pIns->ah.ih) && pThis->Jumped)
	{
		if (pThis->HeadYPosLand>=-0.3)
			pThis->HeadYPosLand = (ci_player.vCollisionVelocity.y/15.0) * frame_increment;

		pThis->Mode         = TMMODE_GROUND;
		pThis->JumpUpForce  = 0;
		pThis->MoveSpeed    = 0;				// slow down when landing from a jump
		CTMove__StopJumping(pThis);
	}
}



void CTMove__StartJump(CTMove *pThis)
{
	pThis->StartJumpAngle = pThis->JumpAngle;
	pThis->DesiredJumpAngle = ANGLE_DTOR(JUMP_TILT_ANGLE);
	pThis->JumpU = 0;
}

void CTMove__StopJumping(CTMove *pThis)
{
	pThis->StartJumpAngle = pThis->JumpAngle;
	pThis->DesiredJumpAngle = 0;
	pThis->JumpU = 0;
}

void CTMove__JumpIncrement(CTMove *pThis)
{
	pThis->JumpU = min(1, pThis->JumpU + frame_increment/(JUMP_TILT_SECONDS*FRAME_FPS));
	pThis->JumpAngle = BlendFLOAT(GetFastSlowBlender(pThis->JumpU), pThis->StartJumpAngle, pThis->DesiredJumpAngle);
	pThis->JumpLookAng = pThis->JumpAngle;
}

void CTMove__DoRotOffsets(CTMove *pThis, CEngineApp *pApp)
{
	pApp->m_RotXOffset -= pThis->HeadXRot;
	pApp->m_RotXOffset -= pThis->GunKickXRot;
	pApp->m_RotXOffset -= pThis->SpdChgXOffset;
	pApp->m_RotYOffset -= pThis->HeadYRot;
//	pApp->m_RotXOffset -= pThis->JumpLookAng;
	pThis->RotZPlayer -= (-pThis->HeadZRot);
	pApp->m_YPosOffset += pThis->HeadYPosLand;
}


// turok control for turning around on x-axis while swimming underwater
//
void CTMove__TurnOnXAxis(CTMove *pThis, CEngineApp *pApp, CTControl *pCTControl)
{
	// declare variables
	float resu, resd, dm;
	CGameObjectInstance *pIns;
	int	waterFlag;


	// get instance pointer to turok
	pIns = CEngineApp__GetPlayer(pApp);
	if (pIns == NULL)
		return;

	// get player water status
	waterFlag = CEngineApp__GetPlayerWaterStatus(pApp);
	if (waterFlag != PLAYER_BELOW_WATERSURFACE)
		return;

	// *** turok looking down
	resd = CTControl__IsLookUp(pCTControl);
	if (resd > 0)
	{
		// analogue look down
		dm = resd;
		dm /= TTURN_ANALOGUE_SCALER;
		pThis->LookUpDownSpeed = -dm;
	}

	// *** turok looking up
	resu = CTControl__IsLookDown(pCTControl);
	if (resu > 0)
	{
		// analogue look up
		dm = resu;
		dm /= TTURN_ANALOGUE_SCALER;
		pThis->LookUpDownSpeed = dm;
	}

	// where is player turning ?
	dm = (pThis->LookUpDownSpeed - pThis->ActualLookUpDownSpeed) / 2;
	pThis->ActualLookUpDownSpeed += dm;

	if ((resu == 0) && (resd == 0))
	{
		dm = 0 - pThis->LookUpDownSpeed;
		dm = dm / (TTURN_STOP_SMOOTH*3);
		pThis->LookUpDownSpeed += dm;
	}

	pThis->ActualRotXPlayer += ANGLE_DTOR(pThis->ActualLookUpDownSpeed)*frame_increment/2.7;
	NormalizeRotation(&pThis->ActualRotXPlayer);
	pThis->RotXPlayer = BlendRotFLOAT(frame_increment/3, pThis->RotXPlayer, pThis->ActualRotXPlayer);
}


// turok control for turning
//
void CTMove__Turn(CTMove *pThis, CEngineApp *pApp, CTControl *pCTControl)
{
	// declare variables
	float resl, resr, dm;
	CGameObjectInstance *pIns;
	int	waterFlag;


	// get instance pointer to turok
	pIns = CEngineApp__GetPlayer(pApp);
	if ( pIns == NULL ) return;

	// get player water status
	waterFlag = CEngineApp__GetPlayerWaterStatus(pApp);

	// in shallow water ? - (make player behave as if they are on land)
	if (CEngineApp__PlayerInShallowWater(pApp))
		waterFlag = PLAYER_NOT_NEAR_WATER;

	// *** turok turning left
	resl = CTControl__IsLeft ( pCTControl );
	if ( resl < 0 )
	{
		// digital left
		dm = -TTURN_MAX_SPEED - pThis->TurnSpeed;
		dm = dm / TTURN_SMOOTH;
		pThis->TurnSpeed += dm;
	}
	else if ( resl > 0 )
	{
		// analogue turn left
		dm = resl;
		dm /= TTURN_ANALOGUE_SCALER;
		pThis->TurnSpeed = -dm;
	}

	// *** turok turning right
	resr = CTControl__IsRight ( pCTControl );
	if ( resr < 0 )
	{
		// digital right
		dm = TTURN_MAX_SPEED - pThis->TurnSpeed;
		dm = dm / TTURN_SMOOTH;
		pThis->TurnSpeed += dm;
	}
	else if ( resr > 0 )
	{
		// analogue turn right
		dm = resr;
		dm /= TTURN_ANALOGUE_SCALER;
		pThis->TurnSpeed = dm;
	}

	// where is player turning ?
	switch(waterFlag)
	{
		case PLAYER_BELOW_WATERSURFACE:
			if (pThis->InAntiGrav)
			{
				dm = (pThis->TurnSpeed - pThis->ActualTurnSpeed) / 2;
				pThis->ActualTurnSpeed += dm;

				if (resl == 0 && resr == 0)
				{
					dm = 0 - pThis->TurnSpeed;
					dm = dm / (TTURN_STOP_SMOOTH*2);
					pThis->TurnSpeed += dm;
				}
				pIns->m_RotY += ANGLE_DTOR(pThis->ActualTurnSpeed)*frame_increment/1.1;
			}
			else
			{
				if (CTMove__IsPlayerUpsideDown(pThis))
					dm = ((-pThis->TurnSpeed) - pThis->ActualTurnSpeed) / 6;//13;
				else
					dm = (pThis->TurnSpeed - pThis->ActualTurnSpeed) / 6;//13;

				pThis->ActualTurnSpeed += dm;

				if (resl == 0 && resr == 0)
				{
					dm = 0 - pThis->TurnSpeed;
					dm = dm / (TTURN_STOP_SMOOTH*3);
					pThis->TurnSpeed += dm;
				}

				pIns->m_RotY += ANGLE_DTOR(pThis->ActualTurnSpeed)*frame_increment/2.7;
				dm = ANGLE_DTOR(pThis->ActualTurnSpeed)*0.7;

				if (CTMove__IsPlayerUpsideDown(pThis))
					pThis->ActualSwimBankAngle = dm;
				else
					pThis->ActualSwimBankAngle = -dm;
			}
			break;

		case PLAYER_ON_WATERSURFACE:
			dm = (pThis->TurnSpeed - pThis->ActualTurnSpeed) / 5;//10;
			pThis->ActualTurnSpeed += dm;

			if ( resl == 0 && resr == 0 )
			{
				dm = 0 - pThis->TurnSpeed;
				dm = dm / TTURN_STOP_SMOOTH;
				pThis->TurnSpeed += dm;
			}

			pIns->m_RotY += ANGLE_DTOR(pThis->ActualTurnSpeed)*frame_increment/1.4;
			break;

		default:
			if ( resl == 0 && resr == 0 )
			{
				dm = 0 - pThis->TurnSpeed;
				dm = dm / TTURN_STOP_SMOOTH;
				pThis->TurnSpeed += dm;
			}

			pIns->m_RotY += ANGLE_DTOR(pThis->TurnSpeed)*frame_increment/1.1;
			break;
	}
}


// add to the given weapons ammo count (or pool)
// also if backpack is present, can usually carry more
// NOTE:
// if amount is negative it DOES NOT subtract from ammo,
// it means its an explosive amount, which only arrows and shotgun use.
void CTMove__AddAmmo (CTMove *pThis, int weapon, int amount)
{
	switch (weapon)
	{
		case AI_OBJECT_WEAPON_KNIFE:
		case AI_OBJECT_WEAPON_TOMAHAWK:
			break;

		case AI_OBJECT_WEAPON_TEKBOW:
			if (amount >0)
			{
				pThis->TekBowAmmo += amount ;
				if (pThis->TekBowAmmo > MAX_ARROWS)
					pThis->TekBowAmmo = MAX_ARROWS ;
			}
			// explosive arrows...
			else
			{
				pThis->ExpTekBowAmmo += -amount ;
				if (pThis->ExpTekBowAmmo > MAX_EXPARROWS)
					pThis->ExpTekBowAmmo = MAX_EXPARROWS ;
			}
			break;

		case AI_OBJECT_WEAPON_SEMIAUTOMATICPISTOL:
		case AI_OBJECT_WEAPON_ASSAULTRIFLE:
			pThis->BulletPool += amount ;
			if (pThis->BackPackFlag)
			{
				if (pThis->BulletPool > MAX_BULLETS_BACKPACK)
					pThis->BulletPool = MAX_BULLETS_BACKPACK ;
			}
			else
			{
				if (pThis->BulletPool > MAX_BULLETS)
					pThis->BulletPool = MAX_BULLETS ;
			}
			break;

		case AI_OBJECT_WEAPON_MACHINEGUN:
		case AI_OBJECT_WEAPON_TECHWEAPON1:
		case AI_OBJECT_WEAPON_SHOCKWAVE:
			pThis->EnergyPool += amount ;
			if (pThis->EnergyPool > MAX_ENERGY)
				pThis->EnergyPool = MAX_ENERGY ;
			break;

		case AI_OBJECT_WEAPON_RIOTSHOTGUN:
		case AI_OBJECT_WEAPON_AUTOMATICSHOTGUN:
			if (amount >0)
			{
				pThis->ShotgunPool += amount ;
				if (pThis->BackPackFlag)
				{
					if (pThis->ShotgunPool > MAX_SHOTGUN_BACKPACK)
						pThis->ShotgunPool = MAX_SHOTGUN_BACKPACK ;
				}
				else
				{
					if (pThis->ShotgunPool > MAX_SHOTGUN)
						pThis->ShotgunPool = MAX_SHOTGUN ;
				}
			}
			// explosive shotgun shells...
			else
			{
				pThis->ExpShotgunPool += -amount ;
				if (pThis->BackPackFlag)
				{
					if (pThis->ExpShotgunPool > MAX_EXPSHOTGUN_BACKPACK)
						pThis->ExpShotgunPool = MAX_EXPSHOTGUN_BACKPACK ;
				}
				else
				{
					if (pThis->ExpShotgunPool > MAX_EXPSHOTGUN)
						pThis->ExpShotgunPool = MAX_EXPSHOTGUN ;
				}
			}
			break;

		case AI_OBJECT_WEAPON_MINIGUN:
			pThis->MiniGunAmmo += amount ;
			if (pThis->MiniGunAmmo > MAX_MINIGUN)
				pThis->MiniGunAmmo = MAX_MINIGUN ;
			break;

		case AI_OBJECT_WEAPON_CHRONOSCEPTER:
			pThis->ChronoscepterAmmo += amount ;
			if (pThis->ChronoscepterAmmo > MAX_CHRONOSCEPTER)
				pThis->ChronoscepterAmmo = MAX_CHRONOSCEPTER ;
			break;

		case AI_OBJECT_WEAPON_GRENADE_LAUNCHER:
			pThis->GrenadeLauncherAmmo += amount ;
			if (pThis->BackPackFlag)
			{
				if (pThis->GrenadeLauncherAmmo > MAX_GRENADES_BACKPACK)
					pThis->GrenadeLauncherAmmo = MAX_GRENADES_BACKPACK ;
			}
			else
			{
				if (pThis->GrenadeLauncherAmmo > MAX_GRENADES)
					pThis->GrenadeLauncherAmmo = MAX_GRENADES ;
			}
			break;

		case AI_OBJECT_WEAPON_ROCKET:
			pThis->RocketAmmo += amount ;
			if (pThis->BackPackFlag)
			{
				if (pThis->RocketAmmo > MAX_ROCKETS_BACKPACK)
					pThis->RocketAmmo = MAX_ROCKETS_BACKPACK ;
			}
			else
			{
				if (pThis->RocketAmmo > MAX_ROCKETS)
					pThis->RocketAmmo = MAX_ROCKETS ;
			}
			break;

		case AI_OBJECT_WEAPON_TECHWEAPON2:
			pThis->TechWeapon2Ammo += amount ;
			if (pThis->BackPackFlag)
			{
				if (pThis->TechWeapon2Ammo > MAX_TECH2_BACKPACK)
					pThis->TechWeapon2Ammo = MAX_TECH2_BACKPACK ;
			}
			else
			{
				if (pThis->TechWeapon2Ammo > MAX_TECH2)
					pThis->TechWeapon2Ammo = MAX_TECH2 ;
			}
			break;
	}
}


// decrease weapon ammo
//
void CTMove__DecreaseAmmo ( CTMove *pThis, int *pAmt, int nAmt )
{
	if (!(GetApp()->m_dwCheatFlags & CHEATFLAG_UNLIMITEDAMMO))
	{
		// decrease ammo
		(*pAmt) -= nAmt;
		if ( (*pAmt) < 0 ) (*pAmt) = 0;
	}
}


// is this weapon all stocked up with goodies
BOOL CTMove__WeaponFull (CTMove *pThis, int weapon, BOOL explosive)
{
	switch (weapon)
	{
		case AI_OBJECT_WEAPON_KNIFE:
		case AI_OBJECT_WEAPON_TOMAHAWK:
			return TRUE ;

		case AI_OBJECT_WEAPON_TEKBOW:
			if (explosive)
			{
				if (pThis->ExpTekBowAmmo >= MAX_EXPARROWS)
					return TRUE ;
			}
			else
			{
				if (pThis->TekBowAmmo >= MAX_ARROWS)
					return TRUE ;
			}
			break;

		case AI_OBJECT_WEAPON_SEMIAUTOMATICPISTOL:
		case AI_OBJECT_WEAPON_ASSAULTRIFLE:
			if (pThis->BackPackFlag)
			{
				if (pThis->BulletPool >= MAX_BULLETS_BACKPACK)
					return TRUE ;
			}
			else
			{
				if (pThis->BulletPool >= MAX_BULLETS)
					return TRUE ;
			}
			break;

		case AI_OBJECT_WEAPON_MACHINEGUN:
		case AI_OBJECT_WEAPON_TECHWEAPON1:
			if (pThis->EnergyPool >= MAX_ENERGY)
					return TRUE ;
			break;

		case AI_OBJECT_WEAPON_RIOTSHOTGUN:
		case AI_OBJECT_WEAPON_AUTOMATICSHOTGUN:
			if (pThis->BackPackFlag)
			{
				if (explosive)
				{
					if (pThis->ExpShotgunPool >= MAX_EXPSHOTGUN_BACKPACK)
						return TRUE ;
				}
				else
				{
					if (pThis->ShotgunPool >= MAX_SHOTGUN_BACKPACK)
						return TRUE ;
				}
			}
			else
			{
				if (explosive)
				{
					if (pThis->ExpShotgunPool >= MAX_EXPSHOTGUN)
						return TRUE ;
				}
				else
				{
					if (pThis->ShotgunPool >= MAX_SHOTGUN)
						return TRUE ;
				}
			}
			break;

		case AI_OBJECT_WEAPON_MINIGUN:
			if (pThis->MiniGunAmmo >= MAX_MINIGUN)
				return TRUE ;
			break;

		case AI_OBJECT_WEAPON_CHRONOSCEPTER:
			if (pThis->ChronoscepterAmmo >= MAX_CHRONOSCEPTER)
				return TRUE ;
			break;

		case AI_OBJECT_WEAPON_GRENADE_LAUNCHER:
			if (pThis->BackPackFlag)
			{
				if (pThis->GrenadeLauncherAmmo >= MAX_GRENADES_BACKPACK)
					return TRUE ;
			}
			else
			{
				if (pThis->GrenadeLauncherAmmo >= MAX_GRENADES)
					return TRUE ;
			}
			break;

		case AI_OBJECT_WEAPON_ROCKET:
			if (pThis->BackPackFlag)
			{
				if (pThis->RocketAmmo >= MAX_ROCKETS_BACKPACK)
					return TRUE ;
			}
			else
			{
				if (pThis->RocketAmmo >= MAX_ROCKETS)
					return TRUE ;
			}
			break;

//		case AI_OBJECT_WEAPON_SHOCKWAVE:
//			break;

		case AI_OBJECT_WEAPON_TECHWEAPON2:
			if (pThis->BackPackFlag)
			{
				if (pThis->TechWeapon2Ammo >= MAX_TECH2_BACKPACK)
					return TRUE ;
			}
			else
			{
				if (pThis->TechWeapon2Ammo >= MAX_TECH2)
					return TRUE ;
			}
			break;
	}
	return FALSE ;
}




int CTMove__WeaponAmmoPercentage(CTMove *pThis, int weapon, BOOL explosive)
{
	switch (weapon)
	{
		case AI_OBJECT_WEAPON_KNIFE:
		case AI_OBJECT_WEAPON_TOMAHAWK:
			return 100 ;

		case AI_OBJECT_WEAPON_TEKBOW:
			if (explosive)
				return ((pThis->ExpTekBowAmmo*100) / MAX_EXPARROWS) ;
			else
				return ((pThis->TekBowAmmo*100) / MAX_ARROWS) ;

		case AI_OBJECT_WEAPON_SEMIAUTOMATICPISTOL:
		case AI_OBJECT_WEAPON_ASSAULTRIFLE:
			if (pThis->BackPackFlag)
				return ((pThis->BulletPool*100) / MAX_BULLETS_BACKPACK) ;
			else
				return ((pThis->BulletPool*100) / MAX_BULLETS) ;

		case AI_OBJECT_WEAPON_MACHINEGUN:
		case AI_OBJECT_WEAPON_TECHWEAPON1:
			return ((pThis->EnergyPool*100) / MAX_ENERGY) ;

		case AI_OBJECT_WEAPON_RIOTSHOTGUN:
		case AI_OBJECT_WEAPON_AUTOMATICSHOTGUN:
			if (pThis->BackPackFlag)
			{
				if (explosive)
					return ((pThis->ExpShotgunPool*100) / MAX_EXPSHOTGUN_BACKPACK) ;
				else
					return ((pThis->ShotgunPool*100) / MAX_SHOTGUN_BACKPACK) ;
			}
			else
			{
				if (explosive)
					return ((pThis->ExpShotgunPool*100) / MAX_EXPSHOTGUN) ;
				else
					return ((pThis->ShotgunPool*100) / MAX_SHOTGUN) ;
			}

		case AI_OBJECT_WEAPON_MINIGUN:
			return ((pThis->MiniGunAmmo*100) / MAX_MINIGUN) ;

		case AI_OBJECT_WEAPON_CHRONOSCEPTER:
			return ((pThis->ChronoscepterAmmo*100) / MAX_CHRONOSCEPTER) ;

		case AI_OBJECT_WEAPON_GRENADE_LAUNCHER:
			if (pThis->BackPackFlag)
				return ((pThis->GrenadeLauncherAmmo*100) / MAX_GRENADES_BACKPACK) ;
			else
				return ((pThis->GrenadeLauncherAmmo*100) / MAX_GRENADES) ;

		case AI_OBJECT_WEAPON_ROCKET:
			if (pThis->BackPackFlag)
				return ((pThis->RocketAmmo*100) / MAX_ROCKETS_BACKPACK) ;
			else
				return ((pThis->RocketAmmo*100) / MAX_ROCKETS) ;

//		case AI_OBJECT_WEAPON_SHOCKWAVE:
//			break;

		case AI_OBJECT_WEAPON_TECHWEAPON2:
			if (pThis->BackPackFlag)
				return ((pThis->TechWeapon2Ammo*100) / MAX_TECH2_BACKPACK) ;
			else
				return ((pThis->TechWeapon2Ammo*100) / MAX_TECH2) ;
	}
	return FALSE ;
}









// does this weapon have some explosive ammo
BOOL CTMove__IsExplosiveAmmo (CTMove *pThis)
{
	switch (pThis->WeaponSelect)
	{
		case AI_OBJECT_WEAPON_TEKBOW:
			if (pThis->ExpTekBowAmmo)
				return TRUE ;
			else
				return FALSE ;

		case AI_OBJECT_WEAPON_RIOTSHOTGUN:
		case AI_OBJECT_WEAPON_AUTOMATICSHOTGUN:
			if (pThis->ExpShotgunPool)
				return TRUE ;
			else
				return FALSE ;

		default:
			return FALSE ;

	}
}




// has weapon ammo ran out for this weapon ?
//
BOOL CTMove__WeaponEmpty(CTMove *pThis, int weapon, int variation)
{
	// check currently selected weapon
	switch ( weapon )
	{
		case AI_OBJECT_WEAPON_KNIFE:
			if ( pThis->KnifeAmmo == 0 )
			{
				return TRUE;
			}
			break;

		case AI_OBJECT_WEAPON_TOMAHAWK:
			if ( pThis->TomahawkAmmo == 0 )
			{
				return TRUE;
			}
			break;

		case AI_OBJECT_WEAPON_TEKBOW:
			if ((pThis->TekBowAmmo + pThis->ExpTekBowAmmo) < TEKBOW_AMMO_USE)
			{
				return TRUE;
			}
			break;

		case AI_OBJECT_WEAPON_SEMIAUTOMATICPISTOL:
			if (pThis->BulletPool < PISTOL_AMMO_USE) return TRUE;
			break;

		case AI_OBJECT_WEAPON_ASSAULTRIFLE:
			if (pThis->BulletPool < RIFLE_AMMO_USE) return TRUE;
			break;

		case AI_OBJECT_WEAPON_MACHINEGUN:
			if (pThis->EnergyPool < MACHINEGUN_AMMO_USE) return TRUE;
			break;

		case AI_OBJECT_WEAPON_TECHWEAPON1:
			if (pThis->EnergyPool < TECH1_AMMO_USE) return TRUE;
			break;

		case AI_OBJECT_WEAPON_SHOCKWAVE:
			switch(variation)
			{
				case 1:
					if (pThis->EnergyPool < SHOCKWAVE_AMMO_USE1) return TRUE;
					break;

				case 2:
					if (pThis->EnergyPool < SHOCKWAVE_AMMO_USE2) return TRUE;
					break;

				case 3:
					if (pThis->EnergyPool < SHOCKWAVE_AMMO_USE3) return TRUE;
					break;

				case 4:
					if (pThis->EnergyPool < SHOCKWAVE_AMMO_USE4) return TRUE;
					break;

				case 5:
					if (pThis->EnergyPool < SHOCKWAVE_AMMO_USE5) return TRUE;
					break;
			}
			break;

		case AI_OBJECT_WEAPON_RIOTSHOTGUN:
			if ((pThis->ShotgunPool + pThis->ExpShotgunPool) < RIOTSHOTGUN_AMMO_USE) return TRUE;
			break;

		case AI_OBJECT_WEAPON_AUTOMATICSHOTGUN:
			if ((pThis->ShotgunPool + pThis->ExpShotgunPool) < AUTOSHOTGUN_AMMO_USE) return TRUE;
			break;

		case AI_OBJECT_WEAPON_MINIGUN:
			if (pThis->MiniGunAmmo < MINIGUN_AMMO_USE) return TRUE;
			break;

		case AI_OBJECT_WEAPON_CHRONOSCEPTER:
			if (pThis->ChronoscepterAmmo < CHRONOSCEPTER_AMMO_USE) return TRUE;
			break;

		case AI_OBJECT_WEAPON_GRENADE_LAUNCHER:
			if (pThis->GrenadeLauncherAmmo < GRENADELAUNCHER_AMMO_USE) return TRUE;
			break;

		case AI_OBJECT_WEAPON_ROCKET:
			if (pThis->RocketAmmo < ROCKET_AMMO_USE) return TRUE;
			break;

		case AI_OBJECT_WEAPON_TECHWEAPON2:
			if (pThis->TechWeapon2Ammo < TECH2_AMMO_USE) return TRUE;
			break;
	}


	// weapon has not ran out of ammo
	return FALSE;
}




// return currently selected weapon type
//
int CTMove__GetCurrentWeapon(CTMove *pThis)
{
	return (pThis->WeaponCurrent);
}




// return ammo of selected weapon
//
int CTMove__GetAmmoOfSelectedWeapon(CTMove *pThis)
{
	// check the weapon flag
	switch (pThis->WeaponSelect)
	{
		case AI_OBJECT_WEAPON_KNIFE:
			return pThis->KnifeAmmo;

		case AI_OBJECT_WEAPON_TOMAHAWK:
			return pThis->TomahawkAmmo;

		case AI_OBJECT_WEAPON_TEKBOW:
			if (pThis->ExpTekBowAmmo)
					return pThis->ExpTekBowAmmo ;
			else
				return pThis->TekBowAmmo ;

		case AI_OBJECT_WEAPON_SEMIAUTOMATICPISTOL:
		case AI_OBJECT_WEAPON_ASSAULTRIFLE:
			return pThis->BulletPool ;
			//return pThis->SemiAutomaticPistolAmmo;
			//return pThis->AssaultRifleAmmo;

		case AI_OBJECT_WEAPON_MACHINEGUN:
		case AI_OBJECT_WEAPON_TECHWEAPON1:
		case AI_OBJECT_WEAPON_SHOCKWAVE:
			return pThis->EnergyPool ;
			//return pThis->MachineGunAmmo;
			//return pThis->TechWeapon1Ammo;

		case AI_OBJECT_WEAPON_RIOTSHOTGUN:
		case AI_OBJECT_WEAPON_AUTOMATICSHOTGUN:
			if (pThis->ExpShotgunPool)
				return pThis->ExpShotgunPool ;
			else
				return pThis->ShotgunPool ;
			//return pThis->RiotShotgunAmmo;
			//return pThis->AutomaticShotgunAmmo;

		case AI_OBJECT_WEAPON_MINIGUN:
			return pThis->MiniGunAmmo *4;				// gives player sense of using more bullets than they really are

		case AI_OBJECT_WEAPON_CHRONOSCEPTER:
			return pThis->ChronoscepterAmmo;

		case AI_OBJECT_WEAPON_GRENADE_LAUNCHER:
			return pThis->GrenadeLauncherAmmo;

		case AI_OBJECT_WEAPON_ROCKET:
			return pThis->RocketAmmo*4;

		case AI_OBJECT_WEAPON_TECHWEAPON2:
			return pThis->TechWeapon2Ammo;
	}


	// weapon does not exist
	return -1;
}




// does this weapon exist ?
//
BOOL CTMove__WeaponExist ( CTMove *pThis, int weapon )
{
	// check the weapon flag
	switch ( weapon )
	{
		case AI_OBJECT_WEAPON_KNIFE:
			return pThis->KnifeFlag;

		case AI_OBJECT_WEAPON_TOMAHAWK:
			return pThis->TomahawkFlag;

		case AI_OBJECT_WEAPON_TEKBOW:
			return pThis->TekBowFlag;

		case AI_OBJECT_WEAPON_SEMIAUTOMATICPISTOL:
			return pThis->SemiAutomaticPistolFlag;

		case AI_OBJECT_WEAPON_ASSAULTRIFLE:
			return pThis->AssaultRifleFlag;

		case AI_OBJECT_WEAPON_MACHINEGUN:
			return pThis->MachineGunFlag;

		case AI_OBJECT_WEAPON_RIOTSHOTGUN:
			return pThis->RiotShotgunFlag;

		case AI_OBJECT_WEAPON_AUTOMATICSHOTGUN:
			return pThis->AutomaticShotgunFlag;

		case AI_OBJECT_WEAPON_MINIGUN:
			return pThis->MiniGunFlag;

		case AI_OBJECT_WEAPON_GRENADE_LAUNCHER:
			return pThis->GrenadeLauncherFlag;

		case AI_OBJECT_WEAPON_TECHWEAPON1:
			return pThis->TechWeapon1Flag;

		case AI_OBJECT_WEAPON_ROCKET:
			return pThis->RocketFlag;

		case AI_OBJECT_WEAPON_SHOCKWAVE:
			return pThis->ShockwaveFlag;

		case AI_OBJECT_WEAPON_TECHWEAPON2:
			return pThis->TechWeapon2Flag;

		case AI_OBJECT_WEAPON_CHRONOSCEPTER:
			return pThis->ChronoscepterFlag;
	}


	// weapon does not exist
	return FALSE;
}




// set weapon fired status (true or false)
//
void CTMove__SetWeaponFiredStatus ( CTMove *pThis, int status )
{
	pThis->WeaponFired = status;
}




// return a vector for the currently selected weapon offset
//
void CTMove__GetWeaponOffset ( CTMove *pThis, CVector3 *pvVec )
{
	int						currentWeapon,
								waterFlag;
	CGameObjectInstance	*pPlayer;
	float						x, y, z, u;


	// which mode is weapon in ?
	pvVec->x = 0;
	pvVec->y = 0;
	pvVec->z = 0;

	// TEMP
//	if (		(pThis->WeaponMode == TWEAPONMODE_OFF)
//			||	(pThis->WeaponMode == TWEAPONMODE_STAYOFF) )
//	{
//		pvVec->z = 200*SCALING_FACTOR;
//	}
//	else
//	{
//		pvVec->z = 0;
//	}


	pPlayer = CEngineApp__GetPlayer(GetApp());

	if (CGameObjectInstance__IsModelLoaded(pPlayer))
		currentWeapon = pThis->WeaponCurrent;
	else
		currentWeapon = -1;

	waterFlag = CEngineApp__GetPlayerWaterStatus(GetApp());

	switch (currentWeapon)
	{
		case AI_OBJECT_WEAPON_KNIFE:
			x = 0.5;
			y = 0.78;//0.80;
			z = 0.45;
			break;

		case AI_OBJECT_WEAPON_TOMAHAWK:
			x = 0.5;
			y = 0.78;//0.80;
			z = 0.45;
			break;

		case AI_OBJECT_WEAPON_TEKBOW:
			x = 0.39;
			y = 0.77;//0.79;
			z = 0.44;
			break;

		case AI_OBJECT_WEAPON_SEMIAUTOMATICPISTOL:
			x = 0.47;
			y = 0.76;//0.78;
			z = 0.54;
			break;

		case AI_OBJECT_WEAPON_ASSAULTRIFLE:
			x = 0.5;
			y = 0.75;//0.77;
			z = 0.60;
			break;

		case AI_OBJECT_WEAPON_MACHINEGUN:
			x = 0.5;
			y = 0.78;//0.80;
			z = 0.50;
			break;

		case AI_OBJECT_WEAPON_RIOTSHOTGUN:
			x = 0.5;
			y = 0.78;//0.80;
			z = 0.50;
			break;

		case AI_OBJECT_WEAPON_AUTOMATICSHOTGUN:
			x = 0.5;
			y = 0.79;//0.81;
			z = 0.52;
			break;

		case AI_OBJECT_WEAPON_MINIGUN:
			x = 0.48;
			y = 0.79;//0.81;
			z = 0.48;
			break;

		case AI_OBJECT_WEAPON_GRENADE_LAUNCHER:
			x = 0.5;
			y = 0.78;//0.80;
			z = 0.45;
			break;

		case AI_OBJECT_WEAPON_TECHWEAPON1:
			x = 0.5;
			y = 0.78;//0.80;
			z = 0.45;
			break;

		case AI_OBJECT_WEAPON_ROCKET:
			x = 0.5;
			y = 0.78;//0.80;
			z = 0.45;
			break;

		case AI_OBJECT_WEAPON_SHOCKWAVE:
			x = 0.5;
			y = 0.78;//0.80;
			z = 0.45;
			break;

		case AI_OBJECT_WEAPON_TECHWEAPON2:
			x = 0.5;
			y = 0.68;//0.70;
			z = -0.7;
			break;

		case AI_OBJECT_WEAPON_CHRONOSCEPTER:
			x = 0.6;
			y = 0.85;
			z = 0.17;
			break;

		default:
			x = 0.5;
			y = 0.80;
			z = 0.45;
			break;
	}


	// fudge to make off screen anim for all weapons to go off screen while underwater with
	// a large angle of view
	if (!pThis->InAntiGrav)
	{
		switch (currentWeapon)
		{
			default:
				if (waterFlag == PLAYER_BELOW_WATERSURFACE)
				{
					u = pPlayer->m_asCurrent.m_cFrame / pPlayer->m_asCurrent.m_nFrames;
					y -= (u*0.2);
				}
				break;

			case AI_OBJECT_WEAPON_KNIFE:
				break;
		}
	}


	pvVec->x = -x*SCALING_FACTOR/SCALING_PLAYERWEAPON_SCALE;
	pvVec->y = -z*SCALING_FACTOR/SCALING_PLAYERWEAPON_SCALE;
	pvVec->z = y*SCALING_FACTOR/SCALING_PLAYERWEAPON_SCALE;
}



// on last weapon in list ?
//
BOOL CTMove__OnLastWeapon(CTMove *pThis)
{
	// declare variables
	int	idx,
			id;


	// find index of last weapon in list
	idx = AI_OBJECT_WEAPON_TOTAL;
	do
	{
		idx --;
		id = WeaponOrder[idx];
		if (		(CTMove__WeaponExist(pThis, id))
				&&	(		(!CTMove__WeaponEmpty(pThis, id, 1))
						||	(idx == pThis->WeaponSelectIndex) ) )
		{
			break;
		}

	} while (idx>0);

	// is this weapon the same as the player currently has ?
	if (idx == pThis->WeaponSelectIndex)
		return TRUE;
	else
		return FALSE;
}


// select the next weapon (ammo flag means select a weapon with ammo)
// if no valid weapons exist it selects the knife by default
//
void CTMove__SelectWeapon(CTMove *pThis, BOOL ammo, BOOL next)
{
	// declare variables
	int	cnt,
			idx,
			id;


	AI_Kill_Weapon_SFX(pThis);
	CTMove__ResetDrawnWeapons(pThis);

	// search all weapons for ammo left
	for (cnt=0; cnt<AI_OBJECT_WEAPON_TOTAL; cnt++)
	{
		// previous or next weapon ?
		if (next)
		{
			// calculate weapon id of next weapon
			idx  = pThis->WeaponSelectIndex + cnt +1;
			if (idx >= AI_OBJECT_WEAPON_TOTAL) idx -= AI_OBJECT_WEAPON_TOTAL;
		}
		else
		{
			// calculate weapon id of previous weapon
			idx  = pThis->WeaponSelectIndex - cnt -1;
			if (idx < 0) idx += AI_OBJECT_WEAPON_TOTAL;
		}

		// look up weapon order id
		id = WeaponOrder[idx];

		switch(id)
		{
			case AI_OBJECT_WEAPON_KNIFE:
			case AI_OBJECT_WEAPON_TOMAHAWK:
				if ( (!ammo) &&
					   CTMove__WeaponExist(pThis, id) &&
					 (!CTMove__WeaponEmpty(pThis, id, 1))    )
				{
					pThis->WeaponSelect      = id;
					pThis->WeaponSelectIndex = idx;
					pThis->WeaponChanged		 = TRUE;

					if (pThis->WeaponMode != TWEAPONMODE_ON)
					{
						pThis->WeaponTakeOffQuickly = TRUE;
						pThis->WeaponTimer = 0.5;
					}
					return;
				}
				break;

			default:
				if ( (!CTMove__WeaponEmpty(pThis, id, 1)) &&
					    CTMove__WeaponExist(pThis, id)			)
				{
					pThis->WeaponSelect      = id;
					pThis->WeaponSelectIndex = idx;
					pThis->WeaponChanged     = TRUE;

					if (pThis->WeaponMode != TWEAPONMODE_ON)
					{
						pThis->WeaponTakeOffQuickly = TRUE;
						pThis->WeaponTimer = 0.5;
					}
					return;
				}
				break;
		}
	}



	// there are no weapons left with ammo, select the knife weapon by default
	pThis->WeaponSelect      = AI_OBJECT_WEAPON_KNIFE;
	pThis->WeaponSelectIndex = 0;
	pThis->WeaponChanged     = TRUE;

	if (pThis->WeaponMode != TWEAPONMODE_ON)
	{
		pThis->WeaponTakeOffQuickly = TRUE;
		pThis->WeaponTimer = 0.5;
	}

	if (pThis->WeaponMode == TWEAPONMODE_STAYOFF)
		pThis->WeaponMode = TWEAPONMODE_OFF;

	if (pThis->WeaponMode == TWEAPONMODE_GOINGTOSTAYOFF)
		pThis->WeaponMode = TWEAPONMODE_GOINGOFF;
}


// select weapon by ai object number
//
BOOL CTMove__SelectWeaponByAIType(CTMove *pThis, int nType, BOOL Reselect)
{
	int	cnt = 0;


	AI_Kill_Weapon_SFX(pThis);
	CTMove__ResetDrawnWeapons(pThis);

	if (    (!CTMove__WeaponEmpty(pThis, nType, 1))
		  &&   CTMove__WeaponExist(pThis, nType) )
	{
		do
		{
			if (nType == WeaponOrder[cnt])
			{
				if (pThis->WeaponSelect != nType || Reselect)
				{
					//pThis->WeaponCurrent     = -1;
					pThis->WeaponSelect      = nType;
					pThis->WeaponSelectIndex = cnt;
					pThis->WeaponChanged     = TRUE;
					pThis->WeaponTakeOffQuickly = TRUE;
					pThis->WeaponTimer = 0.0;
					COnScreen__QuickWeaponChange(&GetApp()->m_OnScreen, pThis->WeaponSelectIndex);
				}

				if (pThis->WeaponMode == TWEAPONMODE_STAYOFF)
					pThis->WeaponMode = TWEAPONMODE_OFF;

				if (pThis->WeaponMode == TWEAPONMODE_GOINGTOSTAYOFF)
					pThis->WeaponMode = TWEAPONMODE_GOINGOFF;

				return TRUE;
			}

			cnt++;

		} while (WeaponOrder[cnt] != -1);
	}


	// could not select this weapon
	return FALSE;
}


// returns current weapon ai type
//
int CTMove__GetWeaponAIType(CTMove *pThis)
{
	return pThis->WeaponSelect;
}


// remove weapon from screen
//
void CTMove__RemoveWeaponFromScreen(CTMove *pThis, CEngineApp *pApp)
{
	CGameObjectInstance	*pPlayer;


	AI_Kill_Weapon_SFX(pThis);
	CTMove__ResetDrawnWeapons(pThis);

	// get player instance
	pPlayer = CEngineApp__GetPlayer(pApp);
	if (!pPlayer) return;

	// make weapon drop off screen
	//pThis->WeaponCurrent = -1;
	pThis->WeaponSelect  = -1;
	pThis->WeaponSelectIndex = -1;
	pThis->WeaponChanged     = FALSE;
	pThis->WeaponMode        = TWEAPONMODE_GOINGTOSTAYOFF;
	pThis->WeaponTakeOffQuickly = TRUE;
	pThis->WeaponTimer = 0.0;
	COnScreen__QuickWeaponChange(&GetApp()->m_OnScreen, pThis->WeaponSelectIndex);
}


// start a tremor shaking effect
// allow full control of tremor effect
//
void CTMove__DoTremorEffectFull(CTMove *pThis, CVector3 vPos, float shake, float ang, float time)
{
//	CVector3					vDelta;
//	float						distanceSquaredOld,
//								distanceSquaredNew;
//	CGameObjectInstance	*pPlayer;


//	if (pThis->TremorTimer > 0.0)
//	{
//		pPlayer = CEngineApp__GetPlayer(GetApp());
//
//		// get distance between new tremor & turok's current position
//		CVector3__Subtract(&vDelta, &vPos, &pPlayer->ah.ih.m_vPos);
//		distanceSquaredNew = CVector3__MagnitudeSquared(&vDelta);
//
//		// get distance between turok & old tremor
//		CVector3__Subtract(&vDelta, &pThis->vTremorPos, &pPlayer->ah.ih.m_vPos);
//		distanceSquaredOld = CVector3__MagnitudeSquared(&vDelta);
//	}
//
//	// is newer tremor closer ?
//	if (		(distanceSquaredNew <= distanceSquaredOld)
//			||	(pThis->TremorTimer == 0.0) )
//	{
		pThis->vTremorPos = vPos;
		pThis->TremorTimer = time;
		pThis->TremorTimerMax = time;
		pThis->TremorAngle = ang;
		pThis->TremorShake = shake;
		pThis->TremorZRot = fabs(ang/3);

//		rmonPrintf("tx:%f ty:%f, tz:%f\n", vPos.x, vPos.y, vPos.z);
//		rmonPrintf("px:%f py:%f, pz:%f\n", pPlayer->ah.ih.m_vPos.x, pPlayer->ah.ih.m_vPos.y, pPlayer->ah.ih.m_vPos.z);

//	}
}


// start a tremor shaking effect
// quick setup of tremor - just specify length of tremor
//
void CTMove__DoTremorEffectQuick(CTMove *pThis, CVector3 vPos, float time)
{
	CTMove__DoTremorEffectFull(pThis, vPos, time*SCALING_FACTOR/6, -(ANGLE_PI/(60/time)), time);
}


// update a tremor shaking effect
//
void CTMove__UpdateTremorEffect(CTMove *pThis)
{
	// declare variables
	CGameObjectInstance	*pIns;
	float						scaler,
								shake,
								angle,
								zrot,
								distance,
								dx,
								dz;


	// get instance pointer to turok
	pIns = CEngineApp__GetPlayer(GetApp());
	if (pIns == NULL) return;


	// tremor occuring ?
	if (pThis->TremorTimer > 0.0)
	{
		// calc scaler value of time left
		scaler = pThis->TremorTimer / pThis->TremorTimerMax;
		shake = (RandomFloat(pThis->TremorShake/2) + (pThis->TremorShake/2)) * scaler;
		angle = (RandomFloat(pThis->TremorAngle/2) + (pThis->TremorAngle/2)) * scaler;
		zrot  = (RandomFloat(pThis->TremorZRot)    - (pThis->TremorZRot /2)) * scaler;

		// calculate distance from turok
		dx = pThis->vTremorPos.x - pIns->ah.ih.m_vPos.x;
		dz = pThis->vTremorPos.z - pIns->ah.ih.m_vPos.z;
		distance = sqrt(dx*dx + dz*dz);
		distance = max(0, ((100*SCALING_FACTOR) - distance));
//		rmonPrintf("distance:%f\n", distance);
		shake = shake * distance / (100*SCALING_FACTOR);
		angle = angle * distance / (100*SCALING_FACTOR);
		zrot  = zrot  * distance / (100*SCALING_FACTOR);

		// shake screen up & down
		if (CInstanceHdr__IsOnGround(&pIns->ah.ih))
			pIns->ah.m_vVelocity.y = shake * 15.0;

		// shake screen with an angle rotation
		pThis->GunKickXRot += angle;

		// shake screen on z-axis
		pThis->RotZPlayer += zrot;

		// decrease length of tremor
		pThis->TremorTimer -= frame_increment*(1.0/FRAME_FPS);
		if (pThis->TremorTimer <= 0)
			pThis->TremorTimer = 0.0;
	}
}


// shake screen effect
//
void CTMove__ShakeScreen(CTMove *pThis, CEngineApp *pApp, float shake, float ang, BOOL rotstyle)
{
	// declare variables
	CGameObjectInstance *pIns;

	// get instance pointer to turok
	pIns = CEngineApp__GetPlayer(pApp);
	if (pIns == NULL) return;

	if (rotstyle)
	{
		// shake screen using normal head y rot (slower to come back to normal head rotation)
		if (CInstanceHdr__IsOnGround(&pIns->ah.ih))
		{
			pIns->ah.m_vVelocity.y  = shake * 15.0;
			pThis->HeadXRot        += ang;
		}
	}
	else
	{
		// shake screen using gun kick rot x (comes back to normal head rotation very quickly)
		if (CInstanceHdr__IsOnGround(&pIns->ah.ih))
		{
			pIns->ah.m_vVelocity.y  = shake * 15.0;
			pThis->GunKickXRot     += ang;
		}
	}
}




// do special effect
//
void CTMove__SpecialEffect(CTMove *pThis, int wfired)
{
	// declare variables
	CEngineApp				*pApp;
	CGameObjectInstance	*pPlayer;
	int						fireTotal;
	CVector3					vPos;


	// get pointer to app
	pApp = GetApp();
	if ( pApp == NULL ) return;

	pPlayer = CEngineApp__GetPlayer(pApp);

	//vPos = CAnimInstanceHdr__GetPosWithGround(&pPlayer->ah);
	vPos = pPlayer->ah.ih.m_vPos;

	// do weapon special effect
	switch(wfired)
	{
		case TWEAPON_NONE:
			break;

		case TWEAPON_KNIFE:
			break;

		case TWEAPON_TOMAHAWK:
			break;

		case TWEAPON_TEKBOW:
			break;

		case TWEAPON_SEMIAUTOMATICPISTOL:
			break;

		case TWEAPON_CHRONOSCEPTER:
			break;

		case TWEAPON_ASSAULTRIFLE:
			fireTotal = 0;
			if (pThis->BulletPool>0 && pThis->AmmoUseCounter >= 0.0)
			{
				if (!pThis->FiredBullet1)
				{
					pThis->FiredBullet1 = TRUE;
					fireTotal++;
					CTMove__DecreaseAmmo(pThis, &pThis->BulletPool, RIFLE_AMMO_USE);
				}
			}

			if (pThis->BulletPool>0 && pThis->AmmoUseCounter >= 0.1)
			{
				if (!pThis->FiredBullet2)
				{
					pThis->FiredBullet2 = TRUE;
					fireTotal++;
					CTMove__DecreaseAmmo(pThis, &pThis->BulletPool, RIFLE_AMMO_USE);
				}
			}

			if (pThis->BulletPool>0 && pThis->AmmoUseCounter >= 0.2)
			{
				if (!pThis->FiredBullet3)
				{
					pThis->FiredBullet3 = TRUE;
					fireTotal++;
					CTMove__DecreaseAmmo(pThis, &pThis->BulletPool, RIFLE_AMMO_USE);
				}
			}

			if (fireTotal)
			{
				CTMove__ShakeScreen(pThis, pApp, 5*SCALING_FACTOR/12, -(ANGLE_PI/130), FALSE);

				CScene__DoSoundEffect(&GetApp()->m_Scene,
											 SOUND_RIFLE_SHOT, 1,
											 pPlayer, &vPos, 0);

				CTMove__FireBullet(pThis, wfired);
			}
			break;

		case TWEAPON_MACHINEGUN:
			if (pThis->EnergyPool > 0)
			{
				if (pThis->AmmoUseCounter >= TWEAPON_MACHINEGUN_USECNT)
				{
					pThis->AmmoUseCounter -= TWEAPON_MACHINEGUN_USECNT;

					CScene__DoSoundEffect(&GetApp()->m_Scene,
												 SOUND_MACHINE_GUN_SHOT_2, 1,
												 pPlayer, &vPos, 0);

					CTMove__ShakeScreen(pThis, pApp, 5*SCALING_FACTOR/12, -(ANGLE_PI/160), FALSE);
					CTMove__DecreaseAmmo(pThis, &pThis->EnergyPool, MACHINEGUN_AMMO_USE);
					CTMove__FireBullet(pThis, wfired);
				}
			}
			break;

		case TWEAPON_RIOTSHOTGUN:
			if (pThis->AmmoUseCounter < 0.5)
			{
				pThis->FiredShotgunShell = FALSE;
			}
			else if (pThis->AmmoUseCounter >= 0.5 && !pThis->FiredShotgunShell)
			{
				pThis->FiredShotgunShell = TRUE;
				CTMove__FireBulletShell (pThis, TWEAPON_RIOTSHOTGUN);
			}
			break;

		case TWEAPON_AUTOMATICSHOTGUN:
			break;

		case TWEAPON_MINIGUN:
			if (pThis->MiniGunAmmo > 0)
			{
				if (pThis->AmmoUseCounter >= 0.11)
				{
					pThis->AmmoUseCounter -= 0.11;

					CScene__DoSoundEffect(&GetApp()->m_Scene,
												 SOUND_MINI_GUN_SHOT, 1,
												 pPlayer, &vPos, 0);

					CTMove__ShakeScreen(pThis, pApp, 5*SCALING_FACTOR/35, -(ANGLE_PI/180), FALSE);
					CTMove__DecreaseAmmo(pThis, &pThis->MiniGunAmmo, MINIGUN_AMMO_USE);
					CTMove__FireBullet(pThis, wfired);
				}
			}
			break;

		case TWEAPON_GRENADELAUNCHER:
			break;

		case TWEAPON_TECHWEAPON1:
			break;

		case TWEAPON_ROCKET:
			break;

		case TWEAPON_SHOCKWAVE:
			break;

		case TWEAPON_TECHWEAPON2:
			if (pThis->TechWeapon2Ammo>0)
			{
				if (pThis->AmmoUseCounter == 0.0)
				{
					CTMove__DecreaseAmmo(pThis, &pThis->TechWeapon2Ammo, TECH2_AMMO_USE);
				}
			}
			break;
	}


	// update ammo use counter
	pThis->AmmoUseCounter += frame_increment*(1.0/FRAME_FPS);
}




// update barrel rotation on weapon
//
#define TMOVE_PITCH_AMT		0.3
#define TMOVE_VOLUME_AMT	0.7

void CTMove__WeaponBarrelUpdate(CTMove *pThis, int nWeapon)
{
	float diff, finc;


	// set frame increment
	finc = frame_increment*2;

	// which weapon barrel to animate ?
	switch(nWeapon)
	{
		case AI_OBJECT_WEAPON_MINIGUN:

			// minigun constantly spins while being shot then slows down when trigger is released
			pThis->WeaponBarrelZRot += pThis->WeaponBarrelZRotInc * finc;
			diff = (0-pThis->WeaponBarrelZRotInc)/22;
			pThis->WeaponBarrelZRotInc += diff * finc;
			if (pThis->WeaponBarrelZRot>(ANGLE_PI*2))
				pThis->WeaponBarrelZRot -= (ANGLE_PI*2);
			break;

		case AI_OBJECT_WEAPON_AUTOMATICSHOTGUN:
			// automatic shotgun rotates a set position for next shell
			if (pThis->WeaponBarrelZRotInc < 0)
			{
				pThis->WeaponBarrelZRot += pThis->WeaponBarrelZRotInc * finc;
				if (pThis->WeaponBarrelZRot < 0)
					pThis->WeaponBarrelZRotInc = 1;
			}
			else
			{
				diff = (-pThis->WeaponBarrelZRot)/4;
				pThis->WeaponBarrelZRot += diff * finc;
			}
			break;

		case AI_OBJECT_WEAPON_SHOCKWAVE:
			// minigun constantly spins while being shot then slows down when trigger is released
			pThis->WeaponBarrelZRot += pThis->WeaponBarrelZRotInc * finc;
			diff = (0-pThis->WeaponBarrelZRotInc)/22;
			pThis->WeaponBarrelZRotInc += diff * finc;
			if (pThis->WeaponBarrelZRot>(ANGLE_PI*2))
			{
				pThis->WeaponBarrelZRot -= (ANGLE_PI*2);
			}
			break;

		default:
			pThis->WeaponBarrelZRot = 0;
			pThis->WeaponBarrelZRotInc = 0;
			break;
	}

	// set rotation for the model
	weapon_z_rotation = pThis->WeaponBarrelZRot;
}




// fire the weapon causing the weapon barrel to move
//
void CTMove__WeaponBarrelFire(CTMove *pThis, int nWeapon)
{
	float		dm, finc;


	// set frame increment
	finc = frame_increment*2;

	switch (nWeapon)
	{
		case AI_OBJECT_WEAPON_MINIGUN:
			pThis->WeaponBarrelZRotInc = TWEAPON_MINIGUN_ZROT;
			break;

		case AI_OBJECT_WEAPON_AUTOMATICSHOTGUN:
			pThis->WeaponBarrelZRot    = ANGLE_DTOR(36);
			pThis->WeaponBarrelZRotInc = -TWEAPON_AUTOMATICSHOTGUN_ZROT * finc;
			break;

		case AI_OBJECT_WEAPON_SHOCKWAVE:
			dm = TWEAPON_SHOCKWAVE_ZROT - ANGLE_DTOR(pThis->ShockwaveStarted*FRAME_FPS);
			if (dm<=TWEAPON_SHOCKWAVE_ZROTMAX) dm=TWEAPON_SHOCKWAVE_ZROTMAX;
			pThis->WeaponBarrelZRotInc = dm * finc;
			break;

		default:
			pThis->WeaponBarrelZRot = 0;
			pThis->WeaponBarrelZRotInc = 0;
			break;
	}
}




// fire a bullet
//
void CTMove__FireBullet ( CTMove *pThis, int WFired )
{
	CEngineApp				*pApp;
	CGameObjectInstance	*pIns;
	DWORD						nType;
	CVector3					vBaseVelocity,
								vForward, vRotatedForward,
								vOffset,
								vParticlePos,
								vPos;
	CQuatern					qRotateDirection;
	CMtxF						mfRot;
	float						velocity,
								Angle;
	CGameRegion				*pParticleRegion;
	int						nParticle;
	BOOL						firedArrow = FALSE;


	// auto aim angle thresholds
//	#define	TMOVE_AUTOAIM_ANGLE_TEKBOW						22
//	#define	TMOVE_AUTOAIM_ANGLE_SEMIAUTOMATICPISTOL	22
//	#define	TMOVE_AUTOAIM_ANGLE_TECHWEAPON1				22
//
//	#define	TMOVE_AUTOAIM_ANGLE_ASSAULTRIFLE				14
//	#define	TMOVE_AUTOAIM_ANGLE_RIOTSHOTGUN				14
//
//	#define	TMOVE_AUTOAIM_ANGLE_AUTOMATICSHOTGUN		6
//	#define	TMOVE_AUTOAIM_ANGLE_MINIGUN					6
//	#define	TMOVE_AUTOAIM_ANGLE_MACHINEGUN				6
//	#define	TMOVE_AUTOAIM_ANGLE_CHRONOSCEPTER			6
//
//	#define	TMOVE_AUTOAIM_ANGLE_GRENADELAUNCHER			1
//	#define	TMOVE_AUTOAIM_ANGLE_ROCKET						1
//	#define	TMOVE_AUTOAIM_ANGLE_SHOCKWAVE					1
//	#define	TMOVE_AUTOAIM_ANGLE_TECHWEAPON2				1

	#define	TMOVE_AUTOAIM_ANGLE_TEKBOW						18
	#define	TMOVE_AUTOAIM_ANGLE_SEMIAUTOMATICPISTOL	18
	#define	TMOVE_AUTOAIM_ANGLE_TECHWEAPON1				18

	#define	TMOVE_AUTOAIM_ANGLE_ASSAULTRIFLE				12
	#define	TMOVE_AUTOAIM_ANGLE_RIOTSHOTGUN				12

	#define	TMOVE_AUTOAIM_ANGLE_AUTOMATICSHOTGUN		3
	#define	TMOVE_AUTOAIM_ANGLE_MINIGUN					3
	#define	TMOVE_AUTOAIM_ANGLE_MACHINEGUN				3
	#define	TMOVE_AUTOAIM_ANGLE_CHRONOSCEPTER			3

	#define	TMOVE_AUTOAIM_ANGLE_GRENADELAUNCHER			1
	#define	TMOVE_AUTOAIM_ANGLE_ROCKET						1
	#define	TMOVE_AUTOAIM_ANGLE_SHOCKWAVE					1
	#define	TMOVE_AUTOAIM_ANGLE_TECHWEAPON2				1


	// get pointer to engine app
	pApp = GetApp();
	if ( pApp == NULL ) return;

	// get instance pointer to turok
	pIns = CEngineApp__GetPlayer(pApp);
	if ( pIns == NULL ) return;

	// do weapon projectile particle fire (straight ahead out of gun)
	vBaseVelocity.x = vBaseVelocity.y = vBaseVelocity.z = 0;

	// get position for sfx
	vPos = AW.Ears.vPos;

	// get offset for particle from this weapon
	nParticle=1;
	switch ( WFired )
	{

		case TWEAPON_TEKBOW:
			// turok has released arrow, which type of arrow did turok fire ?
			if (pThis->BowStarted>=TMOVE_SUPER_BOWPOSITION && pThis->BowStarted<=TMOVE_SUPER_BOWENDPOSITION)
			{
				// turok has fired the super arrow
				velocity = TMOVE_SUPERBOW_MAXVELOCITY;
			}
			else
			{
				// turok has fired a normal arrow
				if (pThis->BowStarted>TMOVE_NORMAL_BOWMAX)
					pThis->BowStarted=TMOVE_NORMAL_BOWMAX;

				velocity = TMOVE_BOW_MAXVELOCITY*pThis->BowStarted/TMOVE_SUPER_BOWPOSITION;
			}
			vForward.x = 0;
			vForward.y = 0;
			vForward.z = 1;

			vOffset.x = 0.02*SCALING_FACTOR;
			vOffset.y = -0.60*SCALING_FACTOR;
			vOffset.z = 2.5*SCALING_FACTOR;

			// get particle position
			CGameObjectInstance__GetOffsetPositionAndRegion(CEngineApp__GetPlayer(GetApp()),
																			vOffset,
																			&vParticlePos,
																			&pParticleRegion);

			qRotateDirection = CGameObjectInstance__GetAutoAimRotation(pIns, vParticlePos, NULL, TMOVE_AUTOAIM_ANGLE_TEKBOW);

			CQuatern__ToMatrix(&qRotateDirection, mfRot);
			CMtxF__VectorMult(mfRot, &vForward, &vRotatedForward);
			CVector3__MultScaler(&vBaseVelocity, &vRotatedForward, velocity*15.0);

			if (pThis->ExpTekBowAmmo)
				nType = PARTICLE_TYPE_ARROWEXPLOSIVE;
			else
				nType = PARTICLE_TYPE_ARROW;
			CTMove__LoudNoise(pThis);
			firedArrow = TRUE;
			break;

		case TWEAPON_SEMIAUTOMATICPISTOL:
			vOffset.x = 1.28*SCALING_FACTOR;
			vOffset.y = -1*SCALING_FACTOR;
			vOffset.z = 2.5*SCALING_FACTOR;
			nType = PARTICLE_TYPE_BULLET;
			Angle = TMOVE_AUTOAIM_ANGLE_SEMIAUTOMATICPISTOL;
			CTMove__LoudNoise(pThis);
			break;

		case TWEAPON_ASSAULTRIFLE:
			vOffset.x = 1*SCALING_FACTOR;
			vOffset.y = -1*SCALING_FACTOR;
			vOffset.z = 2.5*SCALING_FACTOR;
			nType = PARTICLE_TYPE_BULLET;
			Angle = TMOVE_AUTOAIM_ANGLE_ASSAULTRIFLE;
			CTMove__LoudNoise(pThis);
			break;

		case TWEAPON_MACHINEGUN:
			vOffset.x = 1*SCALING_FACTOR;
			vOffset.y = -0.9*SCALING_FACTOR;
			vOffset.z = 2.5*SCALING_FACTOR;
			nType = PARTICLE_TYPE_LHGUN_PULSE;
			Angle = TMOVE_AUTOAIM_ANGLE_MACHINEGUN;
			CTMove__LoudNoise(pThis);
			break;

		case TWEAPON_RIOTSHOTGUN:
			vOffset.x = 1.2*SCALING_FACTOR;
			vOffset.y = -0.7*SCALING_FACTOR;
			vOffset.z = 2.5*SCALING_FACTOR;
			if (pThis->ExpShotgunPool)
				nType = PARTICLE_TYPE_SHOTGUNEXPLOSIVE;
			else
				nType = PARTICLE_TYPE_SHOTGUN;
			Angle = TMOVE_AUTOAIM_ANGLE_RIOTSHOTGUN;
			CTMove__LoudNoise(pThis);
			break;

		case TWEAPON_AUTOMATICSHOTGUN:
			vOffset.x = 1.2*SCALING_FACTOR;
			vOffset.y = -0.7*SCALING_FACTOR;
			vOffset.z = 2.5*SCALING_FACTOR;
			if (pThis->ExpShotgunPool)
				nType = PARTICLE_TYPE_SHOTGUNEXPLOSIVE;
			else
				nType = PARTICLE_TYPE_SHOTGUN;
			Angle = TMOVE_AUTOAIM_ANGLE_AUTOMATICSHOTGUN;
			CTMove__LoudNoise(pThis);
			break;

		case TWEAPON_MINIGUN:
			vOffset.x = 0.8*SCALING_FACTOR;
			vOffset.y = -1*SCALING_FACTOR;
			vOffset.z = 2.5*SCALING_FACTOR;
			nType = PARTICLE_TYPE_MINIGUN_BULLET;
			Angle = TMOVE_AUTOAIM_ANGLE_MINIGUN;
			CTMove__LoudNoise(pThis);
			break;

		case TWEAPON_GRENADELAUNCHER:
			vOffset.x = 1.8*SCALING_FACTOR;
			vOffset.y = -0.5*SCALING_FACTOR;
			vOffset.z = 2.5*SCALING_FACTOR;
			nType = PARTICLE_TYPE_GRENADE;
			Angle = TMOVE_AUTOAIM_ANGLE_GRENADELAUNCHER;
			CTMove__LoudNoise(pThis);
			break;

		case TWEAPON_TECHWEAPON1:
			vOffset.x = 0.4*SCALING_FACTOR;
			vOffset.y = -1.4*SCALING_FACTOR;
			vOffset.z = 2.5*SCALING_FACTOR;
			nType = PARTICLE_TYPE_PLASMA1;
			Angle = TMOVE_AUTOAIM_ANGLE_TECHWEAPON1;
			CTMove__LoudNoise(pThis);
			break;

		case TWEAPON_ROCKET:
			vOffset.x = 0.4*SCALING_FACTOR;
			vOffset.y = -1.4*SCALING_FACTOR;
			vOffset.z = 2.5*SCALING_FACTOR;
			nType = PARTICLE_TYPE_ROCKET;
			Angle = TMOVE_AUTOAIM_ANGLE_ROCKET;
			CTMove__LoudNoise(pThis);
			break;

		case TWEAPON_SHOCKWAVE:
			vOffset.x = 0.4*SCALING_FACTOR;
			vOffset.y = -1.4*SCALING_FACTOR;
			vOffset.z = 2.5*SCALING_FACTOR;

			if (pThis->ShockwaveStarted < 1.0)
			{
				CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_190, 0, NULL, &vPos, 0);
				nType = PARTICLE_TYPE_SHOCKWAVEPULSE1;
			}
			else if (pThis->ShockwaveStarted < 2.0)
			{
				CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_191, 0, NULL, &vPos, 0);
				nType = PARTICLE_TYPE_SHOCKWAVEPULSE2;
			}
			else if (pThis->ShockwaveStarted < 3.0)
			{
				CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_192, 0, NULL, &vPos, 0);
				nType = PARTICLE_TYPE_SHOCKWAVEPULSE3;
			}
			else if (pThis->ShockwaveStarted < 4.0)
			{
				CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_193, 0, NULL, &vPos, 0);
				nType = PARTICLE_TYPE_SHOCKWAVEPULSE4;
			}
			else
			{
				CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_194, 0, NULL, &vPos, 0);
				nType = PARTICLE_TYPE_SHOCKWAVEPULSE5;
			}

			Angle = TMOVE_AUTOAIM_ANGLE_SHOCKWAVE;
			CTMove__LoudNoise(pThis);
			break;

		case TWEAPON_TECHWEAPON2:
			vOffset.x = 0.4*SCALING_FACTOR;
			vOffset.y = -1.4*SCALING_FACTOR;
			vOffset.z = 2.5*SCALING_FACTOR;
			nType = PARTICLE_TYPE_PLASMA2;
			Angle = TMOVE_AUTOAIM_ANGLE_TECHWEAPON2;
			CTMove__LoudNoise(pThis);
			break;

		case TWEAPON_CHRONOSCEPTER:
			vOffset.x = 0.4*SCALING_FACTOR;
			vOffset.y = -1.4*SCALING_FACTOR;
			vOffset.z = 2.5*SCALING_FACTOR;

			// hold charge for long enough ?
			if (pThis->ChronoscepterStarted < TMOVE_CHRONOSCEPTER_CHARGE_TIME)
			{
				nParticle = 0;
			}
			else
			{
				nType = PARTICLE_TYPE_CHRONOSCEPTER;
				Angle = TMOVE_AUTOAIM_ANGLE_CHRONOSCEPTER;
				CTMove__LoudNoise(pThis);
			}
			break;

		default:
			ASSERT(FALSE);
		case TWEAPON_NONE:
		case TWEAPON_KNIFE:
		case TWEAPON_TOMAHAWK:
			nParticle=0;
			break;
	}

	// Record fire
	pThis->FiredParticle = TRUE ;
	pThis->FiredParticleType = nType ;


	if (nParticle)
	{
		if (!firedArrow)
		{
			// get particle position
			CGameObjectInstance__GetOffsetPositionAndRegion(CEngineApp__GetPlayer(GetApp()),
																			vOffset,
																			&vParticlePos,
																			&pParticleRegion);

			// weapons which auto aim
			if (CTMove__DoesWeaponAutoAim(pThis, WFired))
				qRotateDirection = CGameObjectInstance__GetAutoAimRotation(pIns, vParticlePos, NULL, Angle);
			else
				qRotateDirection = CGameObjectInstance__GetAimRotation(pIns);
		}

		// fire particle
		CParticleSystem__CreateParticle(&pApp->m_Scene.m_ParticleSystem, &pIns->ah.ih, nType,
												  vBaseVelocity, qRotateDirection,
												  vParticlePos, pParticleRegion, PARTICLE_NOEDGE, TRUE);

		// Record projectile particle
		pThis->FiredProjectileParticle = TRUE ;
		pThis->FiredProjectileParticleType = nType ;
	}

	// fire out shell from weapon
	switch(WFired)
	{
		// don't do riot shotgun shell yet
		case TWEAPON_RIOTSHOTGUN:
			break;

		// do shell for weapon that has been fired
		default:
			CTMove__FireBulletShell(pThis, WFired);
	}



	// get offset for weapon effect from this weapon
	nParticle=1;
	switch(WFired)
	{
		case TWEAPON_SEMIAUTOMATICPISTOL:
			vOffset.x = 0.65*SCALING_FACTOR;
			vOffset.y = -0.35*SCALING_FACTOR;
			vOffset.z = 2.9*SCALING_FACTOR;
			nType = PARTICLE_TYPE_WEFFECT_PISTOL;
			break;

		case TWEAPON_ASSAULTRIFLE:
			vOffset.x = 0.50*SCALING_FACTOR;
			vOffset.y = -0.35*SCALING_FACTOR;
			vOffset.z = 2.9*SCALING_FACTOR;
			nType = PARTICLE_TYPE_WEFFECT_ASSAULTRIFLE;
			break;

		case TWEAPON_MACHINEGUN:
			vOffset.x = 0.5*SCALING_FACTOR;
			vOffset.y = -0.2*SCALING_FACTOR;
			vOffset.z = 2.9*SCALING_FACTOR;
			nType = PARTICLE_TYPE_WEFFECT_MACHINEGUN;
			break;

		case TWEAPON_RIOTSHOTGUN:
			vOffset.x = 0.65*SCALING_FACTOR;
			vOffset.y = -0.27*SCALING_FACTOR;
			vOffset.z = 2.9*SCALING_FACTOR;
			nType = PARTICLE_TYPE_WEFFECT_RIOTSHOTGUN;
			break;

		case TWEAPON_AUTOMATICSHOTGUN:
			vOffset.x = 0.65*SCALING_FACTOR;
			vOffset.y = -0.19*SCALING_FACTOR;
			vOffset.z = 2.9*SCALING_FACTOR;
			nType = PARTICLE_TYPE_WEFFECT_AUTOMATICSHOTGUN;
			break;

		case TWEAPON_MINIGUN:
			vOffset.x = 0.45*SCALING_FACTOR;
			vOffset.y = -0.31*SCALING_FACTOR;
			vOffset.z = 2.9*SCALING_FACTOR;
			nType = PARTICLE_TYPE_WEFFECT_MINIGUN;
			break;

		case TWEAPON_GRENADELAUNCHER:
			vOffset.x = 1.01*SCALING_FACTOR;
			vOffset.y = -0.05*SCALING_FACTOR;
			vOffset.z = 2.9*SCALING_FACTOR;
			nType = PARTICLE_TYPE_WEFFECT_GRENADELAUNCHER;
			break;

		case TWEAPON_TECHWEAPON1:
			vOffset.x = 0.2*SCALING_FACTOR;
			vOffset.y = -0.6*SCALING_FACTOR;
			vOffset.z = 2.9*SCALING_FACTOR;
			nType = PARTICLE_TYPE_WEFFECT_TECHWEAPON1;
			break;

		case TWEAPON_ROCKET:
			vOffset.x = 0.2*SCALING_FACTOR;
			vOffset.y = -0.6*SCALING_FACTOR;
			vOffset.z = 2.9*SCALING_FACTOR;
			nType = PARTICLE_TYPE_WEFFECT_ROCKET;
			break;

		case TWEAPON_SHOCKWAVE:
			vOffset.x = 0.2*SCALING_FACTOR;
			vOffset.y = -0.6*SCALING_FACTOR;
			vOffset.z = 2.9*SCALING_FACTOR;
			nType = PARTICLE_TYPE_WEFFECT_SHOCKWAVEPULSE;
			break;

		case TWEAPON_TECHWEAPON2:
			vOffset.x = 0.2*SCALING_FACTOR;
			vOffset.y = -0.6*SCALING_FACTOR;
			vOffset.z = 2.9*SCALING_FACTOR;
			nType = PARTICLE_TYPE_WEFFECT_TECHWEAPON2;
			break;

		case TWEAPON_CHRONOSCEPTER:
			vOffset.x = 0.2*SCALING_FACTOR;
			vOffset.y = -0.6*SCALING_FACTOR;
			vOffset.z = 2.9*SCALING_FACTOR;
			nType = PARTICLE_TYPE_WEFFECT_CHRONOSCEPTER;

			if (pThis->ChronoscepterStarted < TMOVE_CHRONOSCEPTER_CHARGE_TIME)
				nParticle = 0;

			break;

		default:
			ASSERT(FALSE);
		case TWEAPON_NONE:
		case TWEAPON_KNIFE:
		case TWEAPON_TOMAHAWK:
		case TWEAPON_TEKBOW:
			nParticle=0;
			break;
	}


	if (nParticle)
	{
		// get particle position
		CGameObjectInstance__GetOffsetPositionAndRegion(CEngineApp__GetPlayer(GetApp()),
																		vOffset,
																		&vParticlePos,
																		&pParticleRegion);

		// weapons which auto aim so there flashes have to be positioned correctly too
		if (CTMove__DoesWeaponAutoAim(pThis, WFired))
			qRotateDirection = CGameObjectInstance__GetAutoAimRotation(pIns, vParticlePos, NULL, Angle);
		else
			qRotateDirection = CGameObjectInstance__GetAimRotation(pIns);


		// fire particle
		CParticleSystem__CreateParticle(&pApp->m_Scene.m_ParticleSystem, &pIns->ah.ih, nType,
												  vBaseVelocity, qRotateDirection,
												  vParticlePos, pParticleRegion, PARTICLE_NOEDGE, TRUE);
	}
}


// does this weapon auto aim ?
//
BOOL CTMove__DoesWeaponAutoAim(CTMove *pThis, int WFired)
{
	switch(WFired)
	{
		case TWEAPON_TEKBOW:
		case TWEAPON_SEMIAUTOMATICPISTOL:
		case TWEAPON_ASSAULTRIFLE:
		case TWEAPON_MACHINEGUN:
		case TWEAPON_RIOTSHOTGUN:
		case TWEAPON_AUTOMATICSHOTGUN:
		case TWEAPON_MINIGUN:
		case TWEAPON_TECHWEAPON1:
		case TWEAPON_ROCKET:
		case TWEAPON_SHOCKWAVE:
		case TWEAPON_CHRONOSCEPTER:
			return TRUE;

		default:
			return FALSE;
	}
}


// fire a bullet shell
//
void CTMove__FireBulletShell ( CTMove *pThis, int WFired )
{
	CEngineApp				*pApp;
	CGameObjectInstance	*pIns;
	DWORD						nType;
	CVector3					vBaseVelocity,
								vOffset,
								vPos,
								vParticlePos;
	CQuatern					qRotateDirection;
	CGameRegion				*pParticleRegion;
	int						nParticle;
	float						Angle;


	// get pointer to engine app
	pApp = GetApp();
	if (pApp == NULL) return;

	// get instance pointer to turok
	pIns = CEngineApp__GetPlayer(pApp);
	if (pIns == NULL) return;

	// do weapon projectile particle fire (straight ahead out of gun)
	vBaseVelocity.x = vBaseVelocity.y = vBaseVelocity.z = 0;

	// do weapon projectile for shell coming out of weapon
	// get offset for particle from this weapon
	nParticle=1;
	switch ( WFired )
	{
		case TWEAPON_SEMIAUTOMATICPISTOL:
			vOffset.x = 1*SCALING_FACTOR;
			vOffset.y = -1*SCALING_FACTOR;
			vOffset.z = 2.7*SCALING_FACTOR;
			nType = PARTICLE_TYPE_BULLET_SHELL;
			Angle = TMOVE_AUTOAIM_ANGLE_SEMIAUTOMATICPISTOL;
			break;

		case TWEAPON_ASSAULTRIFLE:
			vOffset.x = 1*SCALING_FACTOR;
			vOffset.y = -1*SCALING_FACTOR;
			vOffset.z = 2.7*SCALING_FACTOR;
			nType = PARTICLE_TYPE_BULLET_SHELL;
			Angle = TMOVE_AUTOAIM_ANGLE_ASSAULTRIFLE;
			break;

		case TWEAPON_RIOTSHOTGUN:
			vOffset.x = 1.1*SCALING_FACTOR;
			vOffset.y = -0.8*SCALING_FACTOR;
			vOffset.z = 2.7*SCALING_FACTOR;


			vPos.x  = AW.Ears.vPos.x;
			vPos.y  = AW.Ears.vPos.y;
			vPos.z  = AW.Ears.vPos.z;
			CScene__DoSoundEffect(&GetApp()->m_Scene,
										 SOUND_READY_SHOTGUN, 1,
										 pIns, &vPos, 0);
	      //here

			if (pThis->ExpShotgunPool)
				nType = PARTICLE_TYPE_SHOTGUN_EXPLOSIVE_SHELL;
			else
				nType = PARTICLE_TYPE_SHOTGUN_SHELL;

			Angle = TMOVE_AUTOAIM_ANGLE_RIOTSHOTGUN;
			break;

		case TWEAPON_AUTOMATICSHOTGUN:
			vOffset.x = 1.4*SCALING_FACTOR;
			vOffset.y = -1.2*SCALING_FACTOR;
			vOffset.z = 2.7*SCALING_FACTOR;

			vPos.x  = AW.Ears.vPos.x;
			vPos.y  = AW.Ears.vPos.y;
			vPos.z  = AW.Ears.vPos.z;
			CScene__DoSoundEffect(&GetApp()->m_Scene,
										 SOUND_RELOAD_AUTO_SHOTGUN, 1,
										 pIns, &vPos, 0);

			if (pThis->ExpShotgunPool)
				nType = PARTICLE_TYPE_SHOTGUN_EXPLOSIVE_SHELL;
			else
				nType = PARTICLE_TYPE_SHOTGUN_SHELL;

			Angle = TMOVE_AUTOAIM_ANGLE_AUTOMATICSHOTGUN;
			break;

		case TWEAPON_MINIGUN:
			vOffset.x = 1*SCALING_FACTOR;
			vOffset.y = -1*SCALING_FACTOR;
			vOffset.z = 2.7*SCALING_FACTOR;
			nType = PARTICLE_TYPE_BULLET_SHELL;
			Angle = TMOVE_AUTOAIM_ANGLE_MINIGUN;
			break;

		case TWEAPON_MACHINEGUN:
		case TWEAPON_TEKBOW:
		case TWEAPON_GRENADELAUNCHER:
		case TWEAPON_TECHWEAPON1:
		case TWEAPON_ROCKET:
		case TWEAPON_SHOCKWAVE:
		case TWEAPON_TECHWEAPON2:
		case TWEAPON_CHRONOSCEPTER:
			nParticle=0;
			break;

		default:
			ASSERT(FALSE);
	}


	if (nParticle)
	{
		// get particle position
		CGameObjectInstance__GetOffsetPositionAndRegion(CEngineApp__GetPlayer(GetApp()),
																		vOffset,
																		&vParticlePos,
																		&pParticleRegion);

		if (CTMove__DoesWeaponAutoAim(pThis, WFired))
			qRotateDirection = CGameObjectInstance__GetAutoAimRotation(pIns, vParticlePos, NULL, Angle);
		else
			qRotateDirection = CGameObjectInstance__GetAimRotation(pIns);

		// shoot shell
		CParticleSystem__CreateParticle(&pApp->m_Scene.m_ParticleSystem, &pIns->ah.ih, nType,
												  vBaseVelocity, qRotateDirection,
												  vParticlePos, pParticleRegion, PARTICLE_NOEDGE, TRUE);
	}
}



// turok must keel over (no more health)
//
void CTMove__KeelOver(CTMove *pThis, CEngineApp *pApp, CTControl *pCTControl)
{
	// declare variables
	float						dm,
								sinSideY,
								cosSideY;
	int						waterFlag;
	CVector3					vDesiredPos;
	CGameObjectInstance	*pIns;


	// get instance pointer to turok
	pIns = CEngineApp__GetPlayer(pApp);
	if (pIns == NULL) return;

	// initialize current position into desired position before we change it
	vDesiredPos = pIns->ah.ih.m_vPos;

	// calculate sin & cos for side stepping right direction
	sinSideY = sin(pIns->m_RotY - ANGLE_PI/2);
	cosSideY = cos(pIns->m_RotY - ANGLE_PI/2);


	// if player is underwater make keel over slower
	waterFlag = CEngineApp__GetPlayerWaterStatus(pApp);
	switch (waterFlag)
	{
		case	PLAYER_BELOW_WATERSURFACE:
			pThis->HeadZRot += ANGLE_DTOR(0.1*frame_increment);
			pThis->HeadXRot += ANGLE_DTOR(0.1*frame_increment);
			pThis->HeadYRot += ANGLE_DTOR(0.1*frame_increment);

			CAnimInstanceHdr__Collision3(&pIns->ah, vDesiredPos, &ci_playerunderwater);
			pThis->pLastCI = &ci_playerunderwater;
			break;

		case	PLAYER_ON_WATERSURFACE:
			// make camera rotate on z all way over
			if (pThis->HeadZRot >= 0)
			{
				dm = ((ANGLE_PI/8) - pThis->HeadZRot) / 8;
				pThis->HeadZRot += dm;
				vDesiredPos.x += dm*frame_increment * sinSideY;
				vDesiredPos.z += dm*frame_increment * cosSideY;
			}
			else
			{
				dm = ((-ANGLE_PI/8) - pThis->HeadZRot) / 8;
				pThis->HeadZRot += dm;
				vDesiredPos.x += dm*frame_increment * sinSideY;
				vDesiredPos.z += dm*frame_increment * cosSideY;
			}

			CAnimInstanceHdr__Collision3(&pIns->ah, vDesiredPos, &ci_playeronwatersurface);
			pThis->pLastCI = &ci_playeronwatersurface;
			break;

		default:
			// make camera rotate on z all way over
			if (pThis->HeadZRot >= 0)
			{
				dm = ((+ANGLE_PI/8) - pThis->HeadZRot) / 8;
				pThis->HeadZRot += dm;
				vDesiredPos.x += dm*frame_increment * sinSideY;
				vDesiredPos.z += dm*frame_increment * cosSideY;
			}
			else
			{
				dm = ((-ANGLE_PI/8) - pThis->HeadZRot) / 8;
				pThis->HeadZRot += dm;
				vDesiredPos.x += dm*frame_increment * sinSideY;
				vDesiredPos.z += dm*frame_increment * cosSideY;
			}

			CAnimInstanceHdr__Collision3(&pIns->ah, vDesiredPos, &ci_player);
			pThis->pLastCI = &ci_player;
			break;
	}

	// Update timer all the time just incase of a dodgy lockup
	CTMove__UpdateDeathTimer(pThis);
	if (CTMove__IsCompletelyDead(pThis))
		CEngineApp__DoDeath(pApp);
}


BOOL CTMove__IsCompletelyDead(CTMove *pThis)
{
	return (pThis->DeathTimer <= 0.0);
}


void CTMove__UpdateDeathTimer(CTMove *pThis)
{
	if (pThis->DeathTimer > 0.0)
		pThis->DeathTimer -= frame_increment*(1.0/FRAME_FPS);
	else
		pThis->DeathTimer = 0.0;
}


// turok makes loud sound
//
void CTMove__LoudNoise(CTMove *pThis)
{
	CGameObjectInstance	*pIns;

	pIns = CEngineApp__GetPlayer(GetApp());
	if ( pIns == NULL ) return;

	AI_Event_Dispatcher(&pIns->ah.ih, &pIns->ah.ih,
							  AI_MEVENT_LOUDNOISE, AI_SPECIES_ALL,
							  AI_GetPos(pIns), 0);
}


// turok makes loud sound
//
void CTMove__QuietNoise(CTMove *pThis)
{
	CGameObjectInstance	*pIns;

	pIns = CEngineApp__GetPlayer(GetApp());
	if ( pIns == NULL ) return;

	AI_Event_Dispatcher(&pIns->ah.ih, &pIns->ah.ih,
							  AI_MEVENT_QUIETNOISE, AI_SPECIES_ALL,
							  AI_GetPos(pIns), 0);
}


// update combat timer
//
void CTMove__UpdateCombatTimer(CTMove *pThis)
{
	if (pThis->CombatTimer > 0.0)
	{
		pThis->CombatTimer -= frame_increment*(1.0/FRAME_FPS);
		if (pThis->CombatTimer < 0.0)
			pThis->CombatTimer = 0.0;
	}
}


// set combat timer
//
void CTMove__SetCombatTimer(CTMove *pThis, float Timer)
{
	pThis->CombatTimer = Timer;
}


// is turok in combat mode ?
//
BOOL CTMove__IsInCombat(CTMove *pThis)
{
	if (pThis->CombatTimer > 0.0)
		return TRUE;
	else
		return FALSE;
}


// display weapon icons choice
// Direction == TRUE means next weapon
//
void CTMove__DisplayWeaponsIcons(CTMove *pThis, BOOL Direction)
{
	COnScreen__DisplayWeaponChoice(&GetApp()->m_OnScreen, pThis->WeaponSelectIndex, Direction);
	COnScreen__SetAmmo(&GetApp()->m_OnScreen, CTMove__GetAmmoOfSelectedWeapon(pThis), CTMove__IsExplosiveAmmo(pThis));
}


// toggle run walk mode
//
void CTMove__UpdateRunWalkToggle(CTMove *pThis, CTControl *pCTControl)
{
	// is player changing run/walk mode ?
	// only allow walk/run mode if player is not in map scroll mode
	if (!pThis->MapScrolling)
	{
		if (  (CTControl__IsRunWalkToggle(pCTControl)) )
		{
			pThis->RunWalkToggle = ~pThis->RunWalkToggle;
			COnScreen__SetWalkRunIcon(&GetApp()->m_OnScreen, pThis->RunWalkToggle ? ONSCRN_RUN_ICON : ONSCRN_WALK_ICON);
			if (pThis->RunWalkToggle) pThis->RunTime = 0.1;
		}
	}
}


// burst timing defines
#define	TMOVE_BURST_START_TIME		0.0
#define	TMOVE_BURST_END_TIME			0.5


// toggle swim burst mode
//
void CTMove__UpdateSwimBurstToggle(CTMove *pThis, CTControl *pCTControl)
{
	// is player changing swim/burst mode ?
/*	if (  (CTControl__IsBurstSwimToggle(pCTControl)) )
	{
		pThis->SwimBurstToggle = ~pThis->SwimBurstToggle;
		COnScreen__SetSwimBurstIcon(&GetApp()->m_OnScreen, pThis->SwimBurstToggle ? ONSCRN_BURST_ICON : ONSCRN_SWIM_ICON);
		if (    (pThis->SwimBurstToggle)
			  && (pThis->BurstTimer > TMOVE_BURST_END_TIME) )
		{
			pThis->BurstTimer = 0.0;
		}
	}
*/
}


// you looking for the defines, well they moved to the top of the file...

void CTMove__SwimSurfaceMovement(CTMove *pThis, CEngineApp *pApp, CTControl *pCTControl)
{
	CGameObjectInstance		*pIns;
	CROMRegionSet				*pRegionSet;
	CVector3						vDesiredPos;
	float							depth,
									dm,
									resj,
									prevYVel;
	int							waterFlag, swimFwd;


	// get instance pointer to turok
	pIns = CEngineApp__GetPlayer(pApp);
	if (pIns == NULL) return;

	pRegionSet = CScene__GetRegionAttributes(&GetApp()->m_Scene, pIns->ah.ih.m_pCurrentRegion);
	if (!pRegionSet)
		return;

	// get player water status
	waterFlag = CEngineApp__GetPlayerWaterStatus(pApp);

	// jumping ?
	resj = CTControl__IsJump(pCTControl);
	if (    (resj < 0)
		  && (waterFlag != PLAYER_ABOVE_WATERSURFACE)
		  && (!pThis->CannotJumpFromSurface) )
	{
		// initialize - digital jump
		pThis->Mode		         = TMMODE_GRDJUMP;
		pThis->JumpUpForce		= 0;
		pThis->Jumped				= FALSE;
		pThis->DoubleJump			= FALSE;
		AI_DoSound(&pIns->ah.ih, SOUND_GENERIC_21, 1, 0);
		CTMove__StartJump(pThis);
		CTMove__LoudNoise(pThis);
		CTMove__GroundJumpNSMovement(pThis, pApp, pCTControl);
		pIns->ah.m_vVelocity.y	= 0;
		pThis->CannotJumpFromCliffFace = TRUE;
		return;
	}
	else if (resj==0)
	{
		pThis->CannotJumpFromSurface = FALSE;
	}


	CTMove__ClearHeadRotations(pThis);


	// get next weapon ?
	if (		(CTControl__IsSelectWeaponNext(pCTControl))
			&&	(pThis->InMenuTimer == 0.0))
	{
		if (pThis->SelectWeaponTimer == 0.0)
		{
			CTMove__SelectWeapon ( pThis, TWEAPON_ANY, TRUE );
			CTMove__DisplayWeaponsIcons(pThis, TRUE);
		}

		pThis->SelectWeaponTimer += frame_increment*(1.0/FRAME_FPS);
		if (pThis->SelectWeaponTimer > TMOVE_DELAY_BETWEEN_AUTOREPEAT)
			pThis->SelectWeaponTimer = 0.0;
	}

	// get previous weapon ?
	else if (		(CTControl__IsSelectWeaponPrev(pCTControl))
					&&	(pThis->InMenuTimer == 0.0))
	{
		if (pThis->SelectWeaponTimer == 0.0)
		{
			CTMove__SelectWeapon ( pThis, TWEAPON_ANY, FALSE );
			CTMove__DisplayWeaponsIcons(pThis, FALSE);
		}

		pThis->SelectWeaponTimer += frame_increment*(1.0/FRAME_FPS);
		if (pThis->SelectWeaponTimer > TMOVE_DELAY_BETWEEN_AUTOREPEAT)
			pThis->SelectWeaponTimer = 0.0;
	}
	else
	{
		pThis->SelectWeaponTimer = 0.0;
	}


	// initialize current position into desired position before we change it
	vDesiredPos = pIns->ah.ih.m_vPos;

	// make it easier to get underwater by looking down
	swimFwd = (CTControl__IsSwimForward(pCTControl) != 0);
#ifdef PLATFORM_3DS
	/* 3DS analog MOVE stick (nub) forward also counts as "swim forward" so look-down + nub submerges. */
	{	extern float g_turok_forward;
		if (g_turok_forward > 0.2f && !CAttractDemo__Active() && !CCamera__InCinemaMode(&pApp->m_Camera))
			swimFwd = 1;
	}
#endif
	if (    (ANGLE_RTOD(pThis->RotXPlayer) < -40.0)			// previous value = -3.0
		  && (swimFwd)
		  && (!pThis->CannotJumpFromSurface) )
	{
		// player is trying to leave the water surface

		// update movement & turning of turok
		vDesiredPos = CTMove__ControlSwimMovement(pThis, pApp, pCTControl, vDesiredPos, NULL, FALSE);
	}
	else
	{
		// keep player at correct water surface height
		// water depth player should rest at if on surface of water
		if (!(pRegionSet->m_dwFlags & REGFLAG_SHALLOWWATER))
		{
			depth = CEngineApp__GetWaterDepthWherePlayerIs(pApp) - (4*SCALING_FACTOR);
			dm = (depth - vDesiredPos.y) / 4;
			vDesiredPos.y += dm * frame_increment * 2;
		}

		// update movement & turning of turok
		dm = vDesiredPos.y;
		vDesiredPos = CTMove__ControlSwimMovement(pThis, pApp, pCTControl, vDesiredPos, NULL, FALSE);
		vDesiredPos.y = dm;
	}

	CTMove__Turn(pThis, pApp, pCTControl);

	// if in shallow water do normal collision
	if (pRegionSet->m_dwFlags & REGFLAG_SHALLOWWATER)
	{
		CAnimInstanceHdr__Collision3(&pIns->ah, vDesiredPos, &ci_player);
		pThis->pLastCI = &ci_player;
	}
	else
	{
		if (		(pIns->ah.m_vVelocity.y > 0.0)
				||	(pIns->ah.m_vVelocity.y < 0.0) )
		{
			prevYVel = pIns->ah.m_vVelocity.y;

			CAnimInstanceHdr__Collision3(&pIns->ah, vDesiredPos, &ci_playerfallintowater);

			pIns->ah.m_vVelocity.y = prevYVel;

			pThis->pLastCI = &ci_playerfallintowater;
		}
		else
		{
			CAnimInstanceHdr__Collision3(&pIns->ah, vDesiredPos, &ci_playeronwatersurface);
			pThis->pLastCI = &ci_playeronwatersurface;
		}
	}
}


void CTMove__SwimWaterMovement(CTMove *pThis, CEngineApp *pApp, CTControl *pCTControl)
{
	CGameObjectInstance		*pIns;
	CVector3						vDesiredPos;

	// get instance pointer to turok
	pIns = CEngineApp__GetPlayer(pApp);
	if (pIns == NULL) return;


	// get next weapon ?
	if (pThis->InAntiGrav)
	{
		if (		(CTControl__IsSelectWeaponNext(pCTControl))
				&&	(pThis->InMenuTimer == 0.0))

		{
			if (pThis->SelectWeaponTimer == 0.0)
			{
				CTMove__SelectWeapon ( pThis, TWEAPON_ANY, TRUE );
				CTMove__DisplayWeaponsIcons(pThis, TRUE);
			}

			pThis->SelectWeaponTimer += frame_increment*(1.0/FRAME_FPS);
			if (pThis->SelectWeaponTimer > TMOVE_DELAY_BETWEEN_AUTOREPEAT)
				pThis->SelectWeaponTimer = 0.0;
		}

		// get previous weapon ?
		else if (		(CTControl__IsSelectWeaponPrev(pCTControl))
						&&	(pThis->InMenuTimer == 0.0))
		{
			if (pThis->SelectWeaponTimer == 0.0)
			{
				CTMove__SelectWeapon ( pThis, TWEAPON_ANY, FALSE );
				CTMove__DisplayWeaponsIcons(pThis, FALSE);
			}

			pThis->SelectWeaponTimer += frame_increment*(1.0/FRAME_FPS);
			if (pThis->SelectWeaponTimer > TMOVE_DELAY_BETWEEN_AUTOREPEAT)
				pThis->SelectWeaponTimer = 0.0;
		}
		else
		{
			pThis->SelectWeaponTimer = 0.0;
		}
	}


	// initialize current position into desired position before we change it
	vDesiredPos = pIns->ah.ih.m_vPos;

	// update movement & turning of turok
	if (pThis->InAntiGrav)
	{
		vDesiredPos = CTMove__ControlSwimMovement(pThis, pApp, pCTControl, vDesiredPos, NULL, TRUE);
		CTMove__Turn(pThis, pApp, pCTControl);
		CAnimInstanceHdr__Collision3(&pIns->ah, vDesiredPos, &ci_playerantigrav);
		pThis->pLastCI = &ci_playerantigrav;
	}
	else
	{
		vDesiredPos = CTMove__ControlSwimMovement(pThis, pApp, pCTControl, vDesiredPos, &ci_playerunderwater, TRUE);
		CTMove__Turn(pThis, pApp, pCTControl);
		CAnimInstanceHdr__Collision3(&pIns->ah, vDesiredPos, &ci_playerunderwater);
		pThis->pLastCI = &ci_playerunderwater;
	}
}


CVector3 CTMove__ControlSwimMovement(CTMove *pThis, CEngineApp *pApp, CTControl *pCTControl, CVector3 vDesiredPos, CCollisionInfo2 *pCollInfo, BOOL Underwater)
{
	CGameObjectInstance	*pIns;
	CVector3					vForward,
								vRotatedForward,
								vSideStepR,
								vRotatedSideStepR,
								vUp,
								vVelocity,
								vStartPos;

	CQuatern					qRotateDirection;
	CMtxF						mfRot;
	float						resf,
								resb,
								resl,
								resr,
								resu,
								resbf,
								velocity,
								dm,
								grd,
								Ang,
								sinSideY,
								cosSideY;
	BOOL						moveSound = FALSE,
								shallow;

	// get instance pointer to turok
	pIns = CEngineApp__GetPlayer(pApp);
	if (pIns == NULL) return vDesiredPos;

	// player in shallow water ?
	shallow = CEngineApp__PlayerInShallowWater(pApp);

#ifdef PLATFORM_3DS
	/* 3DS analog MOVE stick (nub): it bypasses PlayerControllerData (its analog seam only feeds ground
	 * movement), so drive the swim's digital forward/back/side inputs directly at each read below. In shallow
	 * water the code path uses ControlFBward/ControlSideStep instead, where the ground seam already applies —
	 * so this only covers the deep/underwater swim. Gated off attract-demo/cinematics like the ground seam. */
	extern float g_turok_forward, g_turok_strafe;
	int nub_move = (!CAttractDemo__Active() && !CCamera__InCinemaMode(&pApp->m_Camera));
#endif

	// get direction to travel when swimming
	vUp.x = 0;
	vUp.y = 1;
	vUp.z = 0;
	vForward.x  = 0;
	vForward.y  = 0;
	vForward.z  = 1;
	vSideStepR.x = 1;
	vSideStepR.y = 0;
	vSideStepR.z = 0;

	qRotateDirection = CGameObjectInstance__GetAimRotation(pIns);
	CQuatern__ToMatrix(&qRotateDirection, mfRot);
	CMtxF__VectorMult(mfRot, &vForward, &vRotatedForward);
	CMtxF__VectorMult(mfRot, &vSideStepR, &vRotatedSideStepR);

	// calculate sin & cos for side stepping right direction
	sinSideY = sin(pIns->m_RotY - ANGLE_PI/2);
	cosSideY = cos(pIns->m_RotY - ANGLE_PI/2);

	// player swimming up ?
	velocity = 0;
	resu = 0;


	// burst forward ?
	if (		(!pThis->InAntiGrav)
			&&	(!shallow) )
	{
		if (    (pThis->BurstTimer >= TMOVE_BURST_START_TIME)
			  && (pThis->BurstTimer <= TMOVE_BURST_END_TIME)
			  && (pThis->SwimBurstToggle) )
		{
			if (Underwater)
			{
				resbf = CTControl__IsSwimBurstForward(pCTControl);
				if (resbf<0)
				{
					// player bursting forward
					if (pThis->BurstForwardSpeed <= TMOVE_MAX_BURSTFORWARDSPEEDUW/3)
					{
						pThis->BurstForwardSpeed = TMOVE_MAX_BURSTFORWARDSPEEDUW;
						AI_DoSound(&pIns->ah.ih, SOUND_UNDERWATER_SWIM_2, 1, 0);
					}
				}
			}
			else
			{
				resbf = CTControl__IsSwimBurstForward(pCTControl);
				if (resbf<0)
				{
					// player bursting forward
					if (pThis->BurstForwardSpeed <= TMOVE_MAX_BURSTFORWARDSPEEDOW/3)
					{
						pThis->BurstForwardSpeed = TMOVE_MAX_BURSTFORWARDSPEEDOW;
						AI_DoSound(&pIns->ah.ih, SOUND_WATER_SPLASH_2, 1, 0);
					}
				}
			}
		}
	}


	if (Underwater)
	{
		if (!pThis->InAntiGrav)
			CTMove__DoSplashBubbles(pThis, pApp);

		resu = CTControl__IsSwimUp(pCTControl);
		if (!pThis->CannotSwimUp)
		{
			if (resu < 0)
			{
				// digital up
				dm = TMOVE_MAX_UPSWIMSPEED - pThis->MoveUpSpeed;
				dm = dm / TMOVE_SWIM_SMOOTH;
				pThis->MoveUpSpeed += dm;
				velocity = pThis->MoveUpSpeed*frame_increment;
				pThis->CannotJumpFromSurface = TRUE;
			}
		}
		else if (resu==0)
		{
			// allow swimming upwards
			pThis->CannotSwimUp = FALSE;
		}

		// reduce move speed to zero if player has stopped swimming
		if (resu==0)
		{
			// not moving
			dm = 0 - pThis->MoveUpSpeed;
			dm = dm / (TMOVE_SWIM_STOP_SMOOTH*3);
			pThis->MoveUpSpeed += dm;
			velocity = pThis->MoveUpSpeed*frame_increment;
		}

		// move player up
		if (velocity!=0)
		{
			CVector3__MultScaler(&vVelocity, &vUp, velocity);
			CVector3__Add(&vDesiredPos, &vDesiredPos, &vVelocity);
		}
	}

	// if in shallow water do normal walking & running but with water sounds
	if (shallow)
	{
		vStartPos = vDesiredPos;
		vDesiredPos = CTMove__ControlFBward(pThis, pApp, pCTControl, vDesiredPos);
		vDesiredPos = CTMove__ControlSideStep(pThis, pApp, pCTControl, vDesiredPos);

		// did player move ?
		if (		(vDesiredPos.x != vStartPos.x)
				||	(vDesiredPos.y != vStartPos.y)
				||	(vDesiredPos.z != vStartPos.z) )
		{
			pThis->WaterSfxTimer += frame_increment*(1.0/FRAME_FPS);

			if (pThis->WaterSfxTimer > pThis->WaterSfxTimerMax)
			{
				pThis->WaterSfxTimer = 0;
				AI_DoSound(&pIns->ah.ih, SOUND_WATER_SPLASH_3, 1, 0);
				pThis->WaterSfxTimerMax = 0.4 + (((float)RANDOM(10))/10.0);
			}
		}

		pThis->BurstForwardSpeed = 0.0;
		pThis->ActualBurstForwardSpeed = 0.0;
		CTMove__UpdateRunWalkToggle(pThis, pCTControl);
		CTMove__DoGrdHeadMovements(pThis);
		return vDesiredPos;
	}
	else
	{
		resf = CTControl__IsSwimForward(pCTControl);
#ifdef PLATFORM_3DS
		if (nub_move && g_turok_forward >  0.2f) resf = -1.0f;   /* nub up = swim forward */
#endif

		// update burst speed - only if forward control is held in
		if (resf == 0)
		{
			// trying to move forwards slowly
			pThis->BurstTimer = 0.0;
		}
		else
		{
			// trying to move forwards quickly
			pThis->BurstTimer += frame_increment*(1.0/FRAME_FPS);
		}

		dm = (0-pThis->BurstForwardSpeed)/20;
		pThis->BurstForwardSpeed += dm * frame_increment * 2;

		dm = (pThis->BurstForwardSpeed - pThis->ActualBurstForwardSpeed) / 3;
		pThis->ActualBurstForwardSpeed += dm * frame_increment * 2;
	}


	// *** player swimming forwards ?
	velocity = 0;
	if (resf < 0)
	{
		// digital forward
		dm = TMOVE_MAX_SWIMSPEED - pThis->MoveSpeed;
		dm = dm / TMOVE_SWIM_SMOOTH;
		pThis->MoveSpeed += dm;
		velocity = pThis->MoveSpeed*frame_increment;
	}
	else if (resf > 0)
	{
		// analogue forward
		velocity = resf*pThis->MoveSpeed*frame_increment;
	}

	// *** player swimming backwards ?
	resb = CTControl__IsSwimBackward(pCTControl);
#ifdef PLATFORM_3DS
	if (nub_move && g_turok_forward < -0.2f) resb = -1.0f;   /* nub down = swim backward */
#endif
	if (resb < 0)
	{
		// digital backward
		dm = -TMOVE_MAX_BACKSWIMSPEED - pThis->MoveSpeed;
		dm = dm / TMOVE_SWIM_SMOOTH;
		pThis->MoveSpeed += dm;
		velocity = pThis->MoveSpeed*frame_increment;
	}
	else if (resb > 0)
	{
		// analogue backward
		velocity = resb*pThis->MoveSpeed*frame_increment;
	}

	// reduce move speed to zero if player has stopped swimming
	if (resb == 0 && resf == 0)
	{
		// not moving
		dm = 0 - pThis->MoveSpeed;
		dm = dm / (TMOVE_SWIM_STOP_SMOOTH*5);
		pThis->MoveSpeed += dm;
		velocity = pThis->MoveSpeed*frame_increment;
	}
	else
	{
		moveSound = TRUE;
	}

	// add in swim burst forward speed
	velocity += pThis->ActualBurstForwardSpeed*frame_increment;

	// move player forwards
	if (velocity!=0)
	{
		CVector3__MultScaler(&vVelocity, &vRotatedForward, velocity);
		CVector3__Add(&vDesiredPos, &vDesiredPos, &vVelocity);
	}


	// *** player swimming right ?
	velocity = 0;
	resr = CTControl__IsSwimSideStepR(pCTControl);
#ifdef PLATFORM_3DS
	if (nub_move && g_turok_strafe >  0.2f) resr = -1.0f;   /* nub right = swim strafe right */
#endif
	if (resr < 0)
	{
		// digital sidestep right
		dm = TMOVE_MAX_SIDESTEPSWIMSPEED - pThis->SideStepSpeed;
		dm = dm / TMOVE_SWIM_SMOOTH;
		pThis->SideStepSpeed += dm;
		velocity = pThis->SideStepSpeed*frame_increment;
	}
	else if (resr > 0)
	{
		// analogue side step right
		velocity = resr*pThis->SideStepSpeed*frame_increment;
	}


	// *** player swimming left ?
	resl = CTControl__IsSwimSideStepL(pCTControl);
#ifdef PLATFORM_3DS
	if (nub_move && g_turok_strafe < -0.2f) resl = -1.0f;   /* nub left = swim strafe left */
#endif
	if (resl < 0)
	{
		// digital sidestep left
		dm = -TMOVE_MAX_SIDESTEPSWIMSPEED - pThis->SideStepSpeed;
		dm = dm / TMOVE_SWIM_SMOOTH;
		pThis->SideStepSpeed += dm;
		velocity = pThis->SideStepSpeed*frame_increment;
	}
	else if (resl > 0)
	{
		// analogue side step left
		velocity = resl*pThis->SideStepSpeed*frame_increment;
	}

	// reduce side step speed to zero if player has stopped swimming
	if (resl == 0 && resr == 0)
	{
		// not moving
		dm = 0 - pThis->SideStepSpeed;
		dm = dm / (TMOVE_SWIM_STOP_SMOOTH*5);
		pThis->SideStepSpeed += dm;
		velocity = pThis->SideStepSpeed*frame_increment;
	}
	else
	{
		moveSound = TRUE;
	}


	// *** which way should player move ?
	if (velocity!=0)
	{
		// move player right
		vDesiredPos.x += velocity*sinSideY;
		vDesiredPos.z += velocity*cosSideY;
	}


	// play movement sound in water
	if (moveSound && !pThis->InAntiGrav)
	{
		if (Underwater)
		{
			pThis->WaterSfxTimer += frame_increment*(1.0/FRAME_FPS);
			if (pThis->WaterSfxTimer > 2.5)
			{
				pThis->WaterSfxTimer -= 2.5;
				AI_DoSound(&pIns->ah.ih, SOUND_UNDERWATER_SWIM_1, 1, 0);
			}
		}
		else
		{
			pThis->WaterSfxTimer += frame_increment*(1.0/FRAME_FPS);
			if (pThis->WaterSfxTimer > pThis->WaterSfxTimerMax)
			{
				pThis->WaterSfxTimer = 0;
				AI_DoSound(&pIns->ah.ih, SOUND_WATER_SPLASH_3, 1, 0);
				pThis->WaterSfxTimerMax = 0.4 + (((float)RANDOM(10))/10.0);
			}
		}
	}



	// *** adjust gravity underwater based on players speed
	if (pThis->MoveSpeed     <  0.5 &&
		 pThis->MoveSpeed     > -0.5 &&
		 pThis->SideStepSpeed <  0.5 &&
		 pThis->SideStepSpeed > -0.5 &&
		 pThis->MoveUpSpeed   <  0.5 &&
		 pThis->MoveUpSpeed   > -0.5    )
	{
		// no movement
		if (pCollInfo && Underwater)
		{
			dm = (GRAVITY_WATER_ACCELERATION - pCollInfo->GravityAcceleration) / 6;
			pCollInfo->GravityAcceleration += dm;
		}
	}
	else
	{
		// player is moving - underwater ?
		if (Underwater)
		{
			dm = (0 - pIns->ah.m_vVelocity.y) / 8;
			pIns->ah.m_vVelocity.y += dm;
			if (pCollInfo)
			{
				dm = (0 - pCollInfo->GravityAcceleration) / 8;
				pCollInfo->GravityAcceleration += dm;
			}
		}

		if (!Underwater)
		{
			CTMove__DoRipple(pThis, pApp);
		}
	}

	if (!pThis->InAntiGrav)
	{
		if (Underwater)
		{
			// do underwater idle camera motion
			pThis->Time1 += (frame_increment/8);
			if (pThis->Time1>=ANGLE_PI*2)
				pThis->Time1 -= ANGLE_PI*2;

			pThis->Time2 += (frame_increment/5);
			if (pThis->Time2>=ANGLE_PI*2)
				pThis->Time2 -= ANGLE_PI*2;

			Ang = cosf(pThis->Time2+ANGLE_PI)/500;
			pThis->HeadXRot += Ang;

			Ang = cosf(pThis->Time2)/500;
			pThis->RotZPlayer += Ang;

			Ang = cosf(pThis->Time1)/500;
			pThis->HeadYRot += Ang;
		}
		else
		{
			// do water surface idle camera motion
			pThis->Time1 += (frame_increment/6);
			if (pThis->Time1>=ANGLE_PI*2)
				pThis->Time1 -= ANGLE_PI*2;

			pThis->Time2 += (frame_increment/4);
			if (pThis->Time2>=ANGLE_PI*2)
				pThis->Time2 -= ANGLE_PI*2;

			Ang = cosf(pThis->Time2+ANGLE_PI)/400;
			pThis->HeadXRot += Ang;

			Ang = cosf(pThis->Time2)/400;
			pThis->RotZPlayer += Ang;

			Ang = cosf(pThis->Time1)/400;
			pThis->HeadYRot += Ang;
		}
	}

	// as player approaches the ground the player's descent will slow down
	if (Underwater)
	{
		grd = max(2, ((pIns->ah.ih.m_vPos.y - CEngineApp__GetGroundHeightWherePlayerIs(pApp)) / 8) );
		dm = (0 - pIns->ah.m_vVelocity.y) / grd;
		pIns->ah.m_vVelocity.y += dm;
		if (pCollInfo)
		{
			dm = (0 - pCollInfo->GravityAcceleration) / grd;
			pCollInfo->GravityAcceleration += dm;
		}
	}

	// do side step z rotation for water
	if (Underwater)
	{
		dm = (pThis->SideStepSpeed * ANGLE_DTOR(-0.2) - pThis->HeadZRot) / 5;
		pThis->HeadZRot += dm;
	}
	else
	{
		dm = (pThis->SideStepSpeed * ANGLE_DTOR(-0.2) - pThis->HeadZRot) / 5;
		pThis->HeadZRot += dm;
	}

	return vDesiredPos;
}


// count how many seconds player stays underwater
//
void CTMove__UnderwaterOxygen(CTMove *pThis, CEngineApp *pApp)
{
	// declare variables
	CGameObjectInstance	*pIns;
	float						diff;
	int						hits;
	int						temparmor ;


	// get instance pointer to turok
	pIns = CEngineApp__GetPlayer(pApp);
	if (pIns == NULL) return;

	// does player have any oxygen tank seconds left ?
	if (pThis->OxygenTank > 0)
	{
		pThis->Oxygen = 0;				// lungs are full

		pThis->OxygenTank -= frame_increment*(1.0/FRAME_FPS);
		if (pThis->OxygenTank < 0)
			pThis->OxygenTank = 0;

		return;
	}

	// how many seconds has player been underwater without any oxygen tanks ?
	pThis->Oxygen += frame_increment*(1.0/FRAME_FPS);
	if (pThis->Oxygen > TMOVE_OXYGEN_AIRGULP_TIMER)
	{
		diff = ((float)AI_GetDyn(pIns)->m_Health) / 100.0;
		hits = max(1, max(1, 150 - AI_GetDyn(pIns)->m_Health) / 25);
		if (diff < 0.6) diff = 0.6;
		pThis->Oxygen -= diff;

		// if player has armor, temporarily remove it so drowning will
		// decrease health...
		temparmor = pThis->ArmorFlag ;
		pThis->ArmorFlag = 0 ;
		AI_Decrease_Health(pIns, hits, TRUE, TRUE);
		pThis->ArmorFlag = temparmor ;

		pThis->OxygenOut = TRUE;
	}


//	// turok making death sounds  - drowning ?
//	if (pThis->OxygenOut)
//	{
//		pThis->OutOfAirSfxTimer += frame_increment*(1.0/FRAME_FPS);
//		if (pThis->OutOfAirSfxTimer >= TMOVE_OUTOFAIR_SFX_TIMER)
//		{
//			pThis->OutOfAirSfxTimer -= TMOVE_OUTOFAIR_SFX_TIMER;
//			n = RANDOM(2);
//			switch (n)
//			{
//				case 0:	AI_DoSound(&pIns->ah.ih, SOUND_GENERIC_14, 1, 0);		break;
//				case 1:	AI_DoSound(&pIns->ah.ih, SOUND_GENERIC_15, 1, 0);		break;
//			}
//		}
//	}
}


// returns oxygen status for display bar
//
float CTMove__GetOxygenStatus(CTMove *pThis)
{
	float	oxy;
	float val ;

	val = pThis->Oxygen - (TMOVE_OXYGEN_AIRGULP_TIMER-TMOVE_OXYGEN_DISPLAY_TIMER) ;

	if (pThis->OxygenOut)
		oxy = 1.0;
	else
		oxy = min(1.0, val/TMOVE_OXYGEN_DISPLAY_TIMER);
//		oxy = min(1.0, pThis->Oxygen/TMOVE_OXYGEN_AIRGULP_TIMER);


	return (1.0-oxy);
}






// initialize bubbles count based on entry speed underwater
//
void CTMove__InitBubbles(CTMove *pThis, CEngineApp *pApp)
{
	// declare variables
	CGameObjectInstance	*pIns;
	float						nBub;


	// get instance pointer to turok
	pIns = CEngineApp__GetPlayer(pApp);
	if (pIns == NULL) return;


	// what speed is player entering water ?
	if (pIns->ah.m_vVelocity.y<0)
	{
		nBub = -pIns->ah.m_vVelocity.y * 2 / 15.0;
		if (nBub<0) nBub = 0;

		pThis->Bubbles = nBub;
		pThis->BubblesSfxTimer = pThis->Bubbles / 2;
	}
}


// generate splash bubbles
//
void CTMove__DoSplashBubbles(CTMove *pThis, CEngineApp *pApp)
{
	// declare variables
	CGameObjectInstance	*pIns;
	float						nBub;


	if (pThis->Bubbles>0)
	{
		// get instance pointer to turok
		pIns = CEngineApp__GetPlayer(pApp);
		if (pIns == NULL) return;

		pThis->Bubbles -= frame_increment*(1.0/FRAME_FPS);
		if (pThis->Bubbles < 0.0)
			pThis->Bubbles = 0.0;

		// do bubble sfx
		if (pThis->Bubbles < pThis->BubblesSfxTimer)
		{
			AI_DoSound(&pIns->ah.ih, SOUND_UNDERWATER_SWIM_1, 1, 0);
			pThis->BubblesSfxTimer = 0.0;
		}

		nBub = (pThis->Bubbles/6);

		AI_SEvent_Water_Bubble((CInstanceHdr *)pIns, pIns->ah.ih.m_vPos, nBub);
	}
}


// generate splash particle effect
//
void CTMove__DoSplash(CTMove *pThis, CEngineApp *pApp, BOOL DoSound)
{
	// declare variables
	CGameObjectInstance	*pIns;
	CVector3					vPos;


	// get instance pointer to turok
	pIns = CEngineApp__GetPlayer(pApp);
	if (pIns == NULL) return;

	// get position of splash
	vPos.x = pIns->ah.ih.m_vPos.x;
	vPos.y = CEngineApp__GetWaterDepthWherePlayerIs(pApp);
	vPos.z = pIns->ah.ih.m_vPos.z;

	AI_SEvent_Water_Splash((CInstanceHdr *)pIns, vPos, 0);

	if (DoSound)
		AI_DoSound(&pIns->ah.ih, SOUND_WATER_SPLASH_1, 1, 0);
}


// generate ripple particle effect
//
void CTMove__DoRipple(CTMove *pThis, CEngineApp *pApp)
{
	// declare variables
	CGameObjectInstance	*pIns;
	CVector3					vPos;


	// get instance pointer to turok
	pIns = CEngineApp__GetPlayer(pApp);
	if (pIns == NULL) return;

	// get position of splash
	vPos.x = pIns->ah.ih.m_vPos.x;
	vPos.y = CEngineApp__GetWaterDepthWherePlayerIs(pApp);
	vPos.z = pIns->ah.ih.m_vPos.z;

	AI_SEvent_Water_Ripple((CInstanceHdr *)pIns, vPos, 0);
}


// is player upside down ?
//
BOOL CTMove__IsPlayerUpsideDown(CTMove *pThis)
{
	float Ang;


	Ang = pThis->RotXPlayer + (ANGLE_PI/2);
#ifdef PLATFORM_PORT
	Ang = fmodf(Ang, ANGLE_PI*2.0f);   /* O(1); can't spin on a garbage RotXPlayer */
	if (Ang < 0.0f) Ang += ANGLE_PI*2.0f;
	if (Ang != Ang) Ang = 0.0f;
#else
	while (Ang>=ANGLE_PI*2)
		Ang -= ANGLE_PI*2;
	while (Ang<0)
		Ang += ANGLE_PI*2;
#endif


	return (BOOL) (Ang>=ANGLE_PI);
}


// set climbing camera position
//
void CTMove__SetClimbCamera(CTMove *pThis, CEngineApp *pApp, CTControl *pCTControl)
{
	float						botCliff,
								topCliff,
								dm,
								ang;
	CGameRegion				*pRegion;
	CGameObjectInstance	*pPlayer;


	// get current region player is in
	pPlayer = CEngineApp__GetPlayer(GetApp());
	ASSERT(pPlayer);

	pRegion = pPlayer->ah.ih.m_pCurrentRegion;
	if (!pRegion) return;

	CTMove__ClearHeadRotations(pThis);


#define	TMOVE_PLAYER_BCLIFF		(6*SCALING_FACTOR)
#define	TMOVE_PLAYER_TCLIFF		(18*SCALING_FACTOR)//(25*SCALING_FACTOR)


	// if player is not on the ground don't do any climbing camera stuff
	if (!CInstanceHdr__IsOnGroundWhenClimbing(&pPlayer->ah.ih))
		return;

	// get heights
	botCliff = CEngineApp__GetPlayerHeightFromBottomOfCliff(pApp);
	topCliff = CEngineApp__GetPlayerHeightFromTopOfCliff(pApp);

	if (botCliff<0 || topCliff<0)
		return;

	// are we at the bottom of the cliff ?
	if (pThis->OnLadder)
	{
		// player is on a ladder
		if (topCliff <= TMOVE_PLAYER_TCLIFF)
		{
			// player at top of ladder
			pThis->ActualRotXPlayer  = ANGLE_DTOR( 75) * topCliff / TMOVE_PLAYER_TCLIFF;
			pThis->ActualRotXPlayer += ANGLE_DTOR(-30) * (TMOVE_PLAYER_TCLIFF-topCliff) / TMOVE_PLAYER_TCLIFF;
		}
		else
		{
			// player at bottom or in middle of ladder
			pThis->ActualRotXPlayer = ANGLE_DTOR(75);
		}
	}
	else if (botCliff <= TMOVE_PLAYER_BCLIFF)
	{
		// player at bottom of cliff
		pThis->ActualRotXPlayer = ANGLE_DTOR(90) * (botCliff/TMOVE_PLAYER_BCLIFF);
	}
	else if (topCliff <= TMOVE_PLAYER_TCLIFF)
	{
		// player at top of cliff
		pThis->ActualRotXPlayer  = ANGLE_DTOR( 90) * topCliff / TMOVE_PLAYER_TCLIFF;
		pThis->ActualRotXPlayer += ANGLE_DTOR(-50) * (TMOVE_PLAYER_TCLIFF-topCliff) / TMOVE_PLAYER_TCLIFF;
	}
	else
	{
		// player somewhere in middle of the cliff
		pThis->ActualRotXPlayer = ANGLE_DTOR(90);
	}

	// make player orientate towards the cliff
	pThis->ActualRotYPlayer = pThis->ClimbSideStepLookAng;
	ang = RotDifference(pPlayer->m_RotY, CGameRegion__GetGroundYAngle(pRegion));
	ang = (0 - ang) / 12;
	pPlayer->m_RotY += ang;

	if (pThis->ClimbSideStep)
	{
		// make head tilt down as player side steps left & right on cliff face
		ang  = ANGLE_RTOD(pThis->ActualRotYPlayer);
		if (ang < -110) ang = -110;
		if (ang >  110) ang =  110;
		if (ang <   0) ang = -ang;
		pThis->ActualRotXPlayer += ANGLE_DTOR(-ang*1.2);
	}

	// allow player to look left & right while on ladder & moving
	if (pThis->OnLadder)
	{
		ang = ANGLE_DTOR(90) - pThis->ActualRotYPlayer;
		pThis->ActualRotYPlayer += CTControl__IsClimbLookRight(pCTControl)*ang/80;

		ang = pThis->ActualRotYPlayer + ANGLE_DTOR(90);
		pThis->ActualRotYPlayer -= CTControl__IsClimbLookLeft(pCTControl)*ang/80;

		// make head tilt down as player looks left or right
		ang  = ANGLE_RTOD(pThis->ActualRotYPlayer);
		if (ang < -110) ang = -110;
		if (ang >  110) ang =  110;
		if (ang <   0) ang = -ang;
		pThis->ActualRotXPlayer += ANGLE_DTOR(-ang);

		// do up & down
		ang = ANGLE_DTOR(110) - pThis->ActualRotXPlayer;
		pThis->ActualRotXPlayer += CTControl__IsLookDown(pCTControl)*ang / 80;

		ang = pThis->ActualRotXPlayer + ANGLE_DTOR(90);
		pThis->ActualRotXPlayer -= CTControl__IsLookUp(pCTControl)*ang / 80;
	}
	// allow player to look left & right while on cliff while they are not moving
	else if (!pThis->ClimbOneHand && !pThis->ClimbSideStep)
	{
		// left hand holding on can look more to the right
		ang = ANGLE_DTOR(200) - pThis->ActualRotYPlayer;
		pThis->ActualRotYPlayer += CTControl__IsClimbLookRight(pCTControl)*ang/80;

		ang = pThis->ActualRotYPlayer + ANGLE_DTOR(200);
		pThis->ActualRotYPlayer -= CTControl__IsClimbLookLeft(pCTControl)*ang/80;

//		pThis->ActualRotYPlayer  = CTControl__IsClimbLookRight(pCTControl)*ANGLE_DTOR( 270)/80;
//		pThis->ActualRotYPlayer += CTControl__IsClimbLookLeft (pCTControl)*ANGLE_DTOR(-100)/80;

		// make head tilt down as player looks left or right
		ang  = ANGLE_RTOD(pThis->ActualRotYPlayer);
		if (ang < -110) ang = -110;
		if (ang >  110) ang =  110;
		if (ang <   0) ang = -ang;
		pThis->ActualRotXPlayer += ANGLE_DTOR(-ang);

		// do up & down
		ang = ANGLE_DTOR(110) - pThis->ActualRotXPlayer;
		pThis->ActualRotXPlayer += CTControl__IsLookDown(pCTControl)*ang / 80;

		ang = pThis->ActualRotXPlayer + ANGLE_DTOR(90);
		pThis->ActualRotXPlayer -= CTControl__IsLookUp(pCTControl)*ang / 80;
	}

	// set head rotation
	dm = (pThis->ActualRotXPlayer - pThis->RotXPlayer) / 12;
	pThis->RotXPlayer += dm * frame_increment * 2;

	dm = (pThis->ActualRotYPlayer - pThis->RotYPlayer) / 12;
	pThis->RotYPlayer += dm * frame_increment * 2;

}


// update movement of player in climbing
//
void CTMove__ClimbingMovement(CTMove *pThis, CEngineApp *pApp, CTControl *pCTControl)
{
	CGameObjectInstance		*pIns;
	CVector3						vDesiredPos;
//	float							prevY;


	// get instance pointer to turok
	pIns = CEngineApp__GetPlayer(pApp);
	if (pIns == NULL) return;

	// initialize current position into desired position before we change it
//	vDesiredPos = pIns->ah.ih.m_vPos;
//	prevY = vDesiredPos.y;

	// update movement of turok
	vDesiredPos = CTMove__ControlClimbMovement(pThis, pApp, pCTControl, pIns->ah.ih.m_vPos);
	CAnimInstanceHdr__Collision3(&pIns->ah, vDesiredPos, &ci_player);
	pThis->pLastCI = &ci_player;

/*
	// move turok into cliff so he can't sidestep off (if we're not climbing)
	if (fabs(pThis->MoveSpeed) <= 0.2)// && (!pThis->OnLadder))
	{
		pIns->ah.ih.m_vPos.y = prevY;
		vDesiredPos = pIns->ah.ih.m_vPos;
		vDesiredPos = CTMove__ClimbMoveIntoCliff(pThis, pApp, pCTControl, vDesiredPos);
		CAnimInstanceHdr__Collision3(&pIns->ah, vDesiredPos, &ci_player_climbing_sidestep_character);
		pThis->pLastCI = &ci_player_climbing_sidestep_character;

		pIns->ah.ih.m_vPos.y = prevY;
		vDesiredPos = pIns->ah.ih.m_vPos;
	}
*/
}



// control players movement in climbing
//
CVector3 CTMove__ControlClimbMovement(CTMove *pThis, CEngineApp *pApp, CTControl *pCTControl, CVector3 vDesiredPos)
{
	vDesiredPos = CTMove__ControlCliffUpDown  (pThis, pApp, pCTControl, vDesiredPos);
//	if (!pThis->OnLadder)
//		vDesiredPos = CTMove__ControlClimbSideStep(pThis, pApp, pCTControl, vDesiredPos);


	return vDesiredPos;
}


// move turok into wall for so he stays on while sidestepping
//
CVector3 CTMove__ClimbMoveIntoCliff(CTMove *pThis, CEngineApp *pApp, CTControl *pCTControl, CVector3 vDesiredPos)
{
	// declare variables
	float						sinRotY,
								cosRotY;
	CGameObjectInstance	*pIns;


	// get instance pointer to turok
	pIns = CEngineApp__GetPlayer(pApp);
	if (pIns == NULL) return vDesiredPos;

	// calculate sin & cos for straight ahead direction
	sinRotY = sin(pIns->m_RotY + ANGLE_PI);
	cosRotY = cos(pIns->m_RotY + ANGLE_PI);

	// move player
	vDesiredPos.x += (2*SCALING_FACTOR)*frame_increment*sinRotY;
	vDesiredPos.z += (2*SCALING_FACTOR)*frame_increment*cosRotY;


	return vDesiredPos;
}


// let turok move up & down cliff wall
//
CVector3 CTMove__ControlCliffUpDown(CTMove *pThis, CEngineApp *pApp, CTControl *pCTControl, CVector3 vDesiredPos)
{
	// declare variables
	float						resf, resb, dm,
								sinRotY,  cosRotY,
								resj;
	CGameObjectInstance *pIns;


#define	TMOVE_MAX_CLIMB_SPEED				(1.4*SCALING_FACTOR)
#define	TMOVE_CLIMB_SPEED_SMOOTH			4
#define	TMOVE_JUMPOFF_SPEED					(2.3*15.0*SCALING_FACTOR)
#define	TMOVE_CLIMB_SPEED_STOP_SMOOTH		2

#define	TMOVE_MAX_LADDER_SPEED				(0.8*SCALING_FACTOR)//(0.55*SCALING_FACTOR)
#define	TMOVE_LADDER_SPEED_SMOOTH			6
#define	TMOVE_LADDER_SPEED_STOP_SMOOTH	4


	// get instance pointer to turok
	pIns = CEngineApp__GetPlayer(pApp);
	if (pIns == NULL) return vDesiredPos;

	// calculate sin & cos for straight ahead direction
	sinRotY = sin(pIns->m_RotY + ANGLE_PI);
	cosRotY = cos(pIns->m_RotY + ANGLE_PI);

	// jumping ?
	resj = CTControl__IsJump(pCTControl);
	if (		(resj < 0)
			&&	(!pThis->CannotJumpFromCliffFace) )
	{
		// initialize - digital jump
		pThis->Mode		         = TMMODE_GRDJUMP;
		pThis->JumpUpForce		= 0;
		pThis->Jumped				= FALSE;
		pThis->DoubleJump			= FALSE;
		AI_DoSound(&pIns->ah.ih, SOUND_GENERIC_21, 1, 0);
		CTMove__StartJump(pThis);
		CTMove__LoudNoise(pThis);
		CTMove__GroundJumpNSMovement(pThis, pApp, pCTControl);

		// make turok jump from cliff away from cliff
		sinRotY = sin(pIns->m_RotY + ANGLE_PI*2);
		cosRotY = cos(pIns->m_RotY + ANGLE_PI*2);
		pIns->ah.m_vVelocity.x += TMOVE_JUMPOFF_SPEED*frame_increment*sinRotY;
		pIns->ah.m_vVelocity.z += TMOVE_JUMPOFF_SPEED*frame_increment*cosRotY;
		pIns->ah.m_vVelocity.y	= 0;

		// put weapon back to what it was prior to climbing
		if (!CTMove__SelectWeaponByAIType(pThis, pThis->WeaponAtStartOfClimb, FALSE))
			CTMove__SelectWeaponByAIType(pThis, AI_OBJECT_WEAPON_KNIFE, FALSE);

		pThis->WeaponAtStartOfClimb		= -1;

		pThis->CannotJumpFromCliffFace = TRUE;
		return vDesiredPos;
	}
	else if (resj == 0)
	{
		pThis->CannotJumpFromCliffFace = FALSE;
	}

	// do counter
	if (pThis->ClimbOneHand)
	{
		pThis->Time1 += frame_increment;
		if (pThis->Time1 >= 11)
		{
			// camera should swing
			pThis->Time1 -= 11;
			pThis->ClimbOneHand = FALSE;
		}
	}

	// turok moving up
	resf = CTControl__IsClimbUp(pCTControl);
#ifdef PLATFORM_3DS
	/* 3DS analog MOVE stick (nub): forward = climb up. The nub bypasses PlayerControllerData (its analog
	 * seam only feeds ground movement), so drive the climb's digital IsClimbUp result directly here. Gated
	 * off attract-demo/cinematics like the ground seam. Climb is a digital hand-over-hand cycle, so a
	 * digital assert (not analog) is correct/faithful. */
	{	extern float g_turok_forward;
		if (g_turok_forward > 0.2f && !CAttractDemo__Active() && !CCamera__InCinemaMode(&pApp->m_Camera))
			resf = -1.0f;
	}
#endif
	if (    (resf < 0)
		  && (!pThis->ClimbOneHand)
		  && (!CTControl__IsClimbLeft(pCTControl))
		  && (!CTControl__IsClimbRight(pCTControl)) )
	{
		pThis->ClimbOneHand = TRUE;
		pThis->Time1 = 0;
		pThis->ClimbCameraWobble = TRUE;

		if (pThis->ClimbHand)
			AI_DoSound(&pIns->ah.ih, SOUND_GENERIC_23,  1, 0);
		else
			AI_DoSound(&pIns->ah.ih, SOUND_GENERIC_24,  1, 0);

		if (pThis->ClimbWeaponRemove)
		{
			CTMove__RemoveWeaponFromScreen(pThis, pApp);
			pThis->ClimbWeaponRemove = FALSE;
		}
	}

	// turok moving down
/*	if (resf==0 && pThis->OnLadder)
	{
		resb = CTControl__IsClimbDown(pCTControl);
		if (    (resb < 0)
			  && (!pThis->ClimbOneHand)
			  && (!CTControl__IsClimbLeft(pCTControl))
			  && (!CTControl__IsClimbRight(pCTControl)) )
		{
			pThis->ClimbOneHand = TRUE;
			pThis->Time1 = 0;
			pThis->ClimbCameraWobble = TRUE;

			if (pThis->ClimbHand)
				AI_DoSound(&pIns->ah.ih, SOUND_GENERIC_23,  1, 0);
			else
				AI_DoSound(&pIns->ah.ih, SOUND_GENERIC_24,  1, 0);

			if (pThis->ClimbWeaponRemove)
			{
				CTMove__RemoveWeaponFromScreen(pThis, pApp);
				pThis->ClimbWeaponRemove = FALSE;
			}
		}
		else
			resb = 0;
	}
	else*/
		resb = 0;

#ifdef TMOVE_ALLOW_WEAPON_WHILE_CLIMBING
	if (!pThis->ClimbOneHand)
	{
		if (!pThis->ClimbWeaponRemove)
		{
			if ( (!CTMove__WeaponEmpty(pThis, AI_OBJECT_WEAPON_SEMIAUTOMATICPISTOL, 1)) &&
				    CTMove__WeaponExist(pThis, AI_OBJECT_WEAPON_SEMIAUTOMATICPISTOL)			)
			{
				CTMove__SelectWeaponByAIType(pThis, AI_OBJECT_WEAPON_SEMIAUTOMATICPISTOL, TRUE);
			}
			else
			{
				CTMove__SelectWeaponByAIType(pThis, AI_OBJECT_WEAPON_KNIFE, TRUE);
			}
			pThis->ClimbWeaponRemove = TRUE;
		}
	}
#endif

	// do climbing
	if (pThis->ClimbOneHand)
	{
		// climbing up
		if (resf)
		{
			if (pThis->OnLadder)
			{
				dm = TMOVE_MAX_LADDER_SPEED - pThis->MoveSpeed;
				dm = dm / TMOVE_LADDER_SPEED_SMOOTH;
			}
			else
			{
				dm = TMOVE_MAX_CLIMB_SPEED - pThis->MoveSpeed;
				dm = dm / TMOVE_CLIMB_SPEED_SMOOTH;
			}

			pThis->MoveSpeed += dm;
		}

		// allow climbing down on ladders
		if (resb)
		{
			if (pThis->OnLadder)
			{
				dm = (-TMOVE_MAX_LADDER_SPEED/2) - pThis->MoveSpeed;
				dm = dm / TMOVE_LADDER_SPEED_SMOOTH;
			}

			pThis->MoveSpeed += dm;
		}

	}

	// reduce movement speed if we are not climbing
	if (!pThis->ClimbOneHand || pThis->Time1 >= 8)
	{
		if (pThis->OnLadder)
		{
			dm = 0 - pThis->MoveSpeed;
			dm = dm / TMOVE_LADDER_SPEED_STOP_SMOOTH;
		}
		else
		{
			dm = 0 - pThis->MoveSpeed;
			dm = dm / TMOVE_CLIMB_SPEED_STOP_SMOOTH;
		}
		pThis->MoveSpeed += dm;

		if (pThis->ClimbCameraWobble)
		{
			if (pThis->ClimbHand)
			{
				// right hand - camera swings to left
				pThis->ActualClimbSwingRotX = ANGLE_DTOR(-13);
				pThis->ActualClimbSwingRotZ = ANGLE_DTOR(-13);
			}
			else
			{
				// left hand - camera swings to right
				pThis->ActualClimbSwingRotX = ANGLE_DTOR(-13);
				pThis->ActualClimbSwingRotZ = ANGLE_DTOR(13);
			}
			pThis->ClimbHand = !pThis->ClimbHand;
			pThis->ClimbCameraWobble = FALSE;
		}
	}
	else
	{
		player_is_moving = TRUE;
	}

	// move player
	vDesiredPos.x += pThis->MoveSpeed*frame_increment*sinRotY;
	vDesiredPos.z += pThis->MoveSpeed*frame_increment*cosRotY;
	CTMove__QuietNoise(pThis);


	// return desired position
	return vDesiredPos;
}


// update camera climb rotation
//
void CTMove__UpdateClimbCameraRot(CTMove *pThis, CEngineApp *pApp)
{
	float		dm;


	// change camera angle in x
	dm = (0 - pThis->ActualClimbSwingRotX) / 7;
	pThis->ActualClimbSwingRotX += dm * frame_increment;

	dm = (pThis->ActualClimbSwingRotX - pThis->ClimbSwingRotX) / 4;
	pThis->ClimbSwingRotX += dm * frame_increment;

	// change camera angle in y
	dm = (0 - pThis->ActualClimbSwingRotY) / 8;
	pThis->ActualClimbSwingRotY += dm * frame_increment;

	dm = (pThis->ActualClimbSwingRotY - pThis->ClimbSwingRotY) / 5;
	pThis->ClimbSwingRotY += dm * frame_increment;

	// change camera angle in z
	dm = (0 - pThis->ActualClimbSwingRotZ) / 8;
	pThis->ActualClimbSwingRotZ += dm * frame_increment;

	dm = (pThis->ActualClimbSwingRotZ - pThis->ClimbSwingRotZ) / 5;
	pThis->ClimbSwingRotZ += dm * frame_increment;


	pApp->m_RotXOffset += pThis->ClimbSwingRotX;
	pApp->m_RotYOffset += pThis->ClimbSwingRotY;
	pApp->m_RotZOffset += pThis->ClimbSwingRotZ;
}


// let turok do climbing side step motion
//
CVector3 CTMove__ControlClimbSideStep(CTMove *pThis, CEngineApp *pApp, CTControl *pCTControl, CVector3 vDesiredPos)
{
	// declare variables
	float						dm,
								ressl, ressr,
								sinSideY, cosSideY;
	CGameObjectInstance	*pIns;


#define	TMOVE_MAX_CLIMB_SIDESTEP_SPEED				(0.5*SCALING_FACTOR)
//#define	TMOVE_MAX_CLIMB_SIDESTEP_SPEED				(0*SCALING_FACTOR)
#define	TMOVE_CLIMB_SIDESTEP_SPEED_SMOOTH			8
#define	TMOVE_CLIMB_SIDESTEP_SPEED_STOP_SMOOTH		4
#define	TMOVE_CLIMB_SIDESTEP_LEFT_ANG					ANGLE_DTOR(-65)
#define	TMOVE_CLIMB_SIDESTEP_RIGHT_ANG				ANGLE_DTOR( 65)


	// get instance pointer to turok
	pIns = CEngineApp__GetPlayer(pApp);
	if (pIns == NULL) return vDesiredPos;

	// calculate sin & cos for side stepping right direction
	sinSideY = sin(pIns->m_RotY - ANGLE_PI/2);
	cosSideY = cos(pIns->m_RotY - ANGLE_PI/2);


	// turok side stepping right
	ressr = CTControl__IsClimbRight(pCTControl);
	if (    (ressr < 0)
		  && (!CTControl__IsClimbUp(pCTControl))
		  && (!CTControl__IsClimbDown(pCTControl))
		  && (fabs(pThis->MoveSpeed) <= 0.2) )
	{
		// digital side step right
		dm = TMOVE_MAX_CLIMB_SIDESTEP_SPEED - pThis->SideStepSpeed;
		dm = dm / TMOVE_CLIMB_SIDESTEP_SPEED_SMOOTH;
		pThis->SideStepSpeed += dm * frame_increment * 2;
		vDesiredPos.x += pThis->SideStepSpeed*frame_increment*sinSideY;
		vDesiredPos.z += pThis->SideStepSpeed*frame_increment*cosSideY;
		CTMove__QuietNoise(pThis);
		pThis->ClimbSideStepLookAng = TMOVE_CLIMB_SIDESTEP_RIGHT_ANG;
		pThis->ClimbSideStep = TRUE;
	}

	// turok side stepping left
	ressl = CTControl__IsClimbLeft(pCTControl);
	if (    (ressl < 0)
		  && (!CTControl__IsClimbUp(pCTControl))
		  && (!CTControl__IsClimbDown(pCTControl))
		  && (fabs(pThis->MoveSpeed) <= 0.2) )
	{
		// digital side step left
		dm = -TMOVE_MAX_CLIMB_SIDESTEP_SPEED - pThis->SideStepSpeed;
		dm = dm / TMOVE_CLIMB_SIDESTEP_SPEED_SMOOTH;
		pThis->SideStepSpeed += dm * frame_increment * 2;
		vDesiredPos.x += pThis->SideStepSpeed*frame_increment*sinSideY;
		vDesiredPos.z += pThis->SideStepSpeed*frame_increment*cosSideY;
		CTMove__QuietNoise(pThis);
		pThis->ClimbSideStepLookAng = TMOVE_CLIMB_SIDESTEP_LEFT_ANG;
		pThis->ClimbSideStep = TRUE;
	}

	// reduce sidestep speed if there is no there is no side motion
	if (    (ressr == 0 && ressl == 0)
		  || (CTControl__IsClimbUp(pCTControl))
		  || (CTControl__IsClimbDown(pCTControl)) )
	{
		dm = 0 - pThis->SideStepSpeed;
		dm = dm / TMOVE_CLIMB_SIDESTEP_SPEED_STOP_SMOOTH;
		pThis->SideStepSpeed += dm * frame_increment * 2;
		vDesiredPos.x += pThis->SideStepSpeed*frame_increment*sinSideY;
		vDesiredPos.z += pThis->SideStepSpeed*frame_increment*cosSideY;

		pThis->ClimbSideStepLookAng = 0;
		pThis->ClimbSideStep = FALSE;
	}
	else
	{
		player_is_moving = TRUE;
	}

	// return desired position
	return vDesiredPos;
}


// do the duck
//
void CTMove__DoDucking(CTMove *pThis, CEngineApp *pApp)
{
	int	duckFlag;
	float	dm;


#define	TMOVE_DUCK_HEIGHT					(4*SCALING_FACTOR)
#define	TMOVE_DUCK_SMOOTH					4
#define	TMOVE_DUCK_THRESHOLD_BARRIER	(1*SCALING_FACTOR)


//	// if big turok cheat mode is on - do not allow ducking
//	if (GetApp()->m_dwCheatFlags & CHEATFLAG_BIGTUROKMODE)
//	{
//		ci_player.dwFlags											&= (~COLLISIONFLAG_DUCKING);
//		ci_playerunderwater.dwFlags							&= (~COLLISIONFLAG_DUCKING);
//		ci_playeronwatersurface.dwFlags						&= (~COLLISIONFLAG_DUCKING);
//		ci_player_climbing_sidestep_character.dwFlags	&= (~COLLISIONFLAG_DUCKING);
//		pThis->JumpAllowed = TRUE;
//		return;
//	}
//	// is tiny turok cheat mode is on - allow ducking without ducking
//	else if (GetApp()->m_dwCheatFlags & CHEATFLAG_TINYTUROKMODE)
//	{
//		// player is ducked down - they may enter a ducking region
//		ci_player.dwFlags											|= COLLISIONFLAG_DUCKING;
//		ci_playerunderwater.dwFlags							|= COLLISIONFLAG_DUCKING;
//		ci_playeronwatersurface.dwFlags						|= COLLISIONFLAG_DUCKING;
//		ci_player_climbing_sidestep_character.dwFlags	|= COLLISIONFLAG_DUCKING;
//		pThis->JumpAllowed = TRUE;
//		return;
//	}

	// get ducking state
	duckFlag = CEngineApp__GetPlayerDuckStatus(pApp);

	if (    (duckFlag == PLAYER_DUCKING_ENTRANCE_EXIT)
		  || (duckFlag == PLAYER_DUCKING) )
	{
		// move player to duck height
		dm = (TMOVE_DUCK_HEIGHT - pThis->CurrentDuckOffset) / TMOVE_DUCK_SMOOTH;
		pThis->JumpAllowed = FALSE;
	}
	else if (duckFlag != PLAYER_DUCKING)
	{
		// make player stand up
		dm = (0 - pThis->CurrentDuckOffset) / TMOVE_DUCK_SMOOTH;
		pThis->JumpAllowed = TRUE;
	}

	// modify the duck
	pThis->CurrentDuckOffset += dm * frame_increment;


	// has player reached duck height ?
	dm = (TMOVE_DUCK_HEIGHT - pThis->CurrentDuckOffset);
	if (dm >= -TMOVE_DUCK_THRESHOLD_BARRIER && dm <= +TMOVE_DUCK_THRESHOLD_BARRIER)
	{
		// player is ducked down - they may enter a ducking region
		ci_player.dwFlags											|= COLLISIONFLAG_DUCKING;
		ci_playerunderwater.dwFlags							|= COLLISIONFLAG_DUCKING;
		ci_playeronwatersurface.dwFlags						|= COLLISIONFLAG_DUCKING;
		ci_player_climbing_sidestep_character.dwFlags	|= COLLISIONFLAG_DUCKING;
	}
	else
	{
		// player is not ducked down - they can't enter a ducking region yet
		ci_player.dwFlags											&= (~COLLISIONFLAG_DUCKING);
		ci_playerunderwater.dwFlags							&= (~COLLISIONFLAG_DUCKING);
		ci_playeronwatersurface.dwFlags						&= (~COLLISIONFLAG_DUCKING);
		ci_player_climbing_sidestep_character.dwFlags	&= (~COLLISIONFLAG_DUCKING);
	}


	// make turok duck
	pApp->m_YPosOffset -= pThis->CurrentDuckOffset;
}


// update gun position as it auto aims
//
void CTMove__UpdateWeaponPosition(CTMove *pThis)
{
	CEngineApp				*pApp;
	CGameObjectInstance	*pIns;
	CVector3					vOffset,
								vPos;
	CGameRegion				*pRegion;


	// get pointer to engine app
	pApp = GetApp();
	if (pApp == NULL) return;

	// get instance pointer to turok
	pIns = CEngineApp__GetPlayer(pApp);
	if (pIns == NULL) return;

	vOffset.x = vOffset.y = vOffset.z = 0;

	// get position for this weapon
	CGameObjectInstance__GetOffsetPositionAndRegion(CEngineApp__GetPlayer(GetApp()),
																	vOffset,
																	&vPos,
																	&pRegion);

	CGameObjectInstance__GetAutoAimRotation(pIns, vPos, &pThis->qRealAimFix, 1.0);
	CQuatern__BlendLinear(&pThis->qAimFix, min(1, 0.2*frame_increment), &pThis->qAimFix, &pThis->qRealAimFix);
}


/*****************************************************************************
*
*	Function:		CTMove__TurokHasAnyWeaponInList()
*
******************************************************************************
*
*	Description:	Determines if Turok is holding any weapon from given list
*
*	Inputs:			*pThis		-	Ptr to turok move structure
*						pWeaponList	-	List of weapon object Ids (terminated with 0)
*
*	Outputs:			TRUE if turok is holding any weapon in list, else FALSE
*
*****************************************************************************/
BOOL CTMove__TurokHasAnyWeaponInList(CTMove *pThis, INT16 *pWeaponList)
{
	while(*pWeaponList)
	{
		if (CTMove__WeaponExist(pThis, *pWeaponList++))
			return TRUE ;
	}

	return FALSE ;
}


// is player in an anti grav area ?
//
BOOL CTMove__InAntiGrav(CTMove *pThis)
{
	CROMRegionSet			*pRegionSet;
	CGameObjectInstance	*pPlayer;
	BOOL						AntiGrav = FALSE;


	pPlayer = CEngineApp__GetPlayer(GetApp());
	if (!pPlayer)
		return AntiGrav;

	pRegionSet = CScene__GetRegionAttributes(&GetApp()->m_Scene, pPlayer->ah.ih.m_pCurrentRegion);
	if (pRegionSet)
	{
		if (pRegionSet->m_dwFlags & REGFLAG_ANTIGRAV)
		{
			switch (pThis->WaterFlag)
			{
				case PLAYER_BELOW_WATERSURFACE:
				case PLAYER_ON_WATERSURFACE:
					AntiGrav = TRUE;
					break;
			}
		}
	}


	return AntiGrav;
}


// is player on a ladder ?
//
BOOL CTMove__OnLadder(CTMove *pThis)
{
	CROMRegionSet			*pRegionSet;
	CGameObjectInstance	*pPlayer;
	BOOL						Ladder = FALSE;


	pPlayer = CEngineApp__GetPlayer(GetApp());
	if (!pPlayer)
		return Ladder;

	pRegionSet = CScene__GetRegionAttributes(&GetApp()->m_Scene, pPlayer->ah.ih.m_pCurrentRegion);
	if (pRegionSet)
	{
		if (		(CEngineApp__GetPlayerClimbStatus(GetApp()) == PLAYER_AT_CLIMBING_SURFACE)
				&&	(pRegionSet->m_dwFlags & REGFLAG_LADDER) )
		{
			Ladder = TRUE;
		}
	}


	return Ladder;
}


// calculate speed of player
// 0.0 = no movement
// 0.5 = approx max movement (in water)
// 1.0 = max movement (on land)
//
float CTMove__GetSpeed(CTMove *pThis)
{
	float speed;
#define	TMOVE_MAX_TOTAL_MOVEMENT		(3.515625 * SCALING_FACTOR)

	speed  = (pThis->MoveSpeed * pThis->MoveSpeed);
	speed += (pThis->ActualBurstForwardSpeed * pThis->ActualBurstForwardSpeed);
	speed += (pThis->SideStepSpeed * pThis->SideStepSpeed);
	speed += (pThis->MoveUpSpeed * pThis->MoveUpSpeed);
	speed  = sqrt(speed);
	speed  = min (1.0, speed / TMOVE_MAX_TOTAL_MOVEMENT);


	return speed;
}


#ifdef PLATFORM_PORT
// Blue-portal key gate (port): has the player collected ALL level-access keys — the central-hub condition
// that opens the Campaigner portal. Mirrors aidoor.c PortalAI, which opens each LevelN hub portal on
// LevelN_Access >= MAX_KEYN. Used by the warp-point selection (scene.c) to keep a bonus/hub warp's
// key-gated boss destination unreachable until every key is collected.
BOOL CTMove__HasAllKeys(void)
{
	return (CTurokMovement.Level2Access >= MAX_KEY2)
	    && (CTurokMovement.Level3Access >= MAX_KEY3)
	    && (CTurokMovement.Level4Access >= MAX_KEY4)
	    && (CTurokMovement.Level5Access >= MAX_KEY5)
	    && (CTurokMovement.Level6Access >= MAX_KEY6)
	    && (CTurokMovement.Level7Access >= MAX_KEY7)
	    && (CTurokMovement.Level8Access >= MAX_KEY8);
}
#endif

// decrease lives & return true if turok is still alive
//
BOOL CTMove__DecreaseLives(CTMove *pThis)
{
	// infinite lives cheat
	if (GetApp()->m_dwCheatFlags & CHEATFLAG_INFINITELIVES)
		return TRUE ;

	if (pThis->Lives==0)
		return FALSE;

	pThis->Lives--;
	return TRUE;
}


// increase lives
//
void CTMove__IncreaseLives(CTMove *pThis, int nLives)
{
	CGameObjectInstance	*pIns = CEngineApp__GetPlayer(GetApp());

	if (pThis->Lives < TMOVE_MAX_LIVES)
	{
		COnScreen__AddGameTextForTime(text_extralife, 3) ;
		AI_DoSound(&pIns->ah.ih, SOUND_GENERIC_234, 1, 0);

		pThis->Lives += nLives;
		if (pThis->Lives > TMOVE_MAX_LIVES)
			pThis->Lives = TMOVE_MAX_LIVES;
	}
}


// is turok using his map ?
//
void CTMove__UsingMap(CTMove *pThis, CTControl *pCTControl)
{

#define TMOVE_MAPTOGGLEHOLDTIME		0.3

#if defined(PLATFORM_PORT) && !defined(PLATFORM_3DS)
	// PC/host controls feel fix: the map is a clean, NON-MODAL edge toggle.
	// The N64 idiom (hold >0.3s to enter a "map scrolling" mode where the
	// movement/C-buttons pan the map) does not fit KB+M — on a keyboard "press
	// Tab" is trivially held >0.3s, latching MapScrolling and hijacking WASD =
	// "the keys are messed up". So on the host: L/Tab flips MapToggle on the
	// press EDGE only, we never set MapScrolling, and the player keeps
	// moving/looking/shooting under the translucent overlay.
	//
	// Edge detection is done from the held (CTTYPE_DOWN) level plus our own
	// stored previous-state, which is cadence-independent (this function is
	// called every render frame, not only on 30Hz logic ticks). We reuse the
	// now-unused MapButtonTimer field as that prev-state (0.0 = up, 1.0 = down);
	// it is already reset by the cinema/dead force-off and by init.
	{
		BOOL	isDown, wasDown;

		isDown  = ((CTControl__IsMapToggle(pCTControl) != 0.0)
				&& (pThis->InMenuTimer == 0.0)) ? TRUE : FALSE;
		wasDown = (pThis->MapButtonTimer != 0.0) ? TRUE : FALSE;

		// rising edge -> flip the map overlay on/off
		if (isDown && !wasDown)
			pThis->MapToggle = pThis->MapToggle ? FALSE : TRUE;

		pThis->MapButtonTimer = isDown ? 1.0 : 0.0;

		// never hijack player movement on the host
		pThis->MapScrolling   = FALSE;
		pThis->RealMapOffsetX = 0.0;
		pThis->RealMapOffsetZ = 0.0;
		return;
	}
#endif

	// is map button being pressed ?
	if ((CTControl__IsMapToggle(pCTControl)) && (pThis->InMenuTimer == 0.0))
	{
		pThis->MapButtonTimer += frame_increment*(1.0/FRAME_FPS);
		if (pThis->MapButtonTimer >= TMOVE_MAPTOGGLEHOLDTIME)
		{
			// if map is not on bring it on
			if (pThis->MapToggle)
			{
				// player is holding map toggle button down - move map around using map control functions
				pThis->MapScrolling = TRUE;
//				rmonPrintf("map is on : holding map button in\n");
			}
			else
			{
				// map is not on - bring it on & player is holding map button
				pThis->MapToggle = TRUE;
				pThis->MapScrolling = TRUE;
//				rmonPrintf("map is now on : holding map button\n");
			}
		}
	}
	else
	{
		// map button is no longer pressed,
		// was this a map toggle or did player let go of holding the button ?
		if	(		(pThis->MapButtonTimer < TMOVE_MAPTOGGLEHOLDTIME)
				&&	(pThis->MapButtonTimer > 0.0) )
		{
			// map was toggled
			if (pThis->MapToggle)
			{
				pThis->MapToggle = FALSE;
				pThis->MapScrolling = FALSE;
			}
			else
				pThis->MapToggle = TRUE;

//			if (pThis->MapToggle)
//				rmonPrintf("map toggled on\n");
//			else
//				rmonPrintf("map toggled off\n");
		}
		else
		{
			// player let got of holding map button
			pThis->MapScrolling = FALSE;

//			if (pThis->MapToggle)
//				rmonPrintf("map is on : player stopped moving the map\n");
//			else
//				rmonPrintf("map is off : player stopped moving the map\n");

		}

		// reset timer
		pThis->MapButtonTimer = 0.0;
	}


	// in map scrolling mode ?
	if (pThis->MapScrolling)
	{
		// update real map offsets
		if (CTControl__IsMapLeft(pCTControl))
		{
			pThis->RealMapOffsetX += 15*SCALING_FACTOR * frame_increment;
		}
		else if (CTControl__IsMapRight(pCTControl))
		{
			pThis->RealMapOffsetX -= 15*SCALING_FACTOR * frame_increment;
		}

		if (CTControl__IsMapUp(pCTControl))
		{
			pThis->RealMapOffsetZ += 15*SCALING_FACTOR * frame_increment;
		}
		else if (CTControl__IsMapDown(pCTControl))
		{
			pThis->RealMapOffsetZ -= 15*SCALING_FACTOR * frame_increment;
		}
	}
	else
	{
		pThis->RealMapOffsetX = 0.0;
		pThis->RealMapOffsetZ = 0.0;
	}

}


// update map status
//
void CTMove__UpdateMap(CTMove *pThis, CEngineApp *pApp)
{
	float		dm;


	if (pApp->m_MapMode)
	{
		switch (pApp->m_MapStatus)
		{
			case MAP_FADEUP:
				pApp->m_MapAlpha += .1 * frame_increment ;
				if (pApp->m_MapAlpha >= 1.0)
				{
					pApp->m_MapAlpha = 1 ;
					pApp->m_MapStatus = MAP_ON ;
				}
				break ;
			case MAP_FADEDOWN:
				pApp->m_MapAlpha -= .1 * frame_increment ;
				if (pApp->m_MapAlpha <= 0.0)
				{
					pApp->m_MapAlpha = 0 ;
					pApp->m_MapStatus = MAP_NULL ;
					pApp->m_MapMode = FALSE ;
				}
				break ;
		}
	}

	// map on ?
	if (pThis->MapToggle)
	{
		// map is on
		if (pApp->m_MapMode == FALSE)
			pApp->m_MapStatus = MAP_FADEUP ;

		pApp->m_MapMode = TRUE;

		// calculate used map offsets
		dm = (pThis->RealMapOffsetX - pThis->MapOffsetX) / 3;
		pThis->MapOffsetX += dm * frame_increment;

		dm = (pThis->RealMapOffsetZ - pThis->MapOffsetZ) / 4;
		pThis->MapOffsetZ += dm * frame_increment;
	}
	else
	{
		// map is off
		if (pApp->m_MapStatus != MAP_NULL)
			pApp->m_MapStatus = MAP_FADEDOWN ;

//		pApp->m_MapMode = FALSE;
		pThis->RealMapOffsetX = 0.0;
		pThis->RealMapOffsetZ = 0.0;
		pThis->MapOffsetX = 0.0;
		pThis->MapOffsetZ = 0.0;
	}
}


// load player weapon model
//
void CTMove__LoadPlayerWeapon(CTMove *pThis)
{
	CScene__LoadObjectModelType(&GetApp()->m_Scene, CEngineApp__GetPlayer(GetApp()), pThis->WeaponCurrent, pThis->WeaponCurrentAnim);
}


// is player in climbing mode ?
//
BOOL CTMove__IsClimbing(CTMove *pThis)
{
	return (pThis->Mode == TMMODE_CLIMBING);
}


// should weapon z buffer be cleared ?
//
BOOL CTMove__ClearWeaponZBuffer(CTMove *pThis)
{
/*	CGameRegion				*pRegion;
	CVector3					vCurrent,
								vDesired,
								vCurrentPos;
	CGameObjectInstance	*pTurok;
	float						sinRotY,
								cosRotY;


	ASSERT(cache_is_valid);

	pTurok = CEngineApp__GetPlayer(GetApp());
	if (!pTurok)
		return TRUE;

	pRegion = pTurok->ah.ih.m_pCurrentRegion;

	sinRotY = sin(pTurok->m_RotY + ANGLE_PI);
	cosRotY = cos(pTurok->m_RotY + ANGLE_PI);

	vCurrent = pTurok->ah.ih.m_vPos;
	vCurrent.y += 0.8*pTurok->ah.ih.m_pEA->m_CollisionHeight;

	// test for straight infront of turok
	vCurrentPos = vCurrent;
	vDesired = vCurrent;
	vDesired.x += (6*SCALING_FACTOR)*sinRotY;
	vDesired.z += (6*SCALING_FACTOR)*cosRotY;

	if (CAnimInstanceHdr__Collision(&pTurok->ah, &pRegion, &vCurrentPos, vDesired, &ci_movetest, FALSE, TRUE, TRUE, TRUE))
		return TRUE;



	return FALSE;
*/
	return TRUE;
}




// modify enemy
//
/*
int CTMove__ModifyEnemy(CTMove *pThis, CEngineApp *pApp)
{
	int						nInsts,
								i,
								iCurrent = -1;
	CGameObjectInstance	*pPlayer,
								*pInstance,
								*pCurrent = NULL;


	nInsts = pApp->m_Scene.m_nActiveAnimInstances;
	pPlayer = CEngineApp__GetPlayer(pApp) ;

	for (i=0; i<nInsts; i++)
	{
		pInstance = GetApp()->m_Scene.m_pActiveAnimInstances[i];

		if (		(!CGameObjectInstance__IsDevice(pInstance))
				&& (pInstance != pPlayer) )
		{
			pCurrent = pInstance;
			iCurrent = i;
			if (pCurrent == pThis->pModifyEnemy)
				break;
		}
	}

	if (i == nInsts)
	{
		pThis->pModifyEnemy = pCurrent;
	}

	return iCurrent;
}


// change enemy
// TRUE = next
// FALSE = previous
//
void CTMove__ChangeEnemy(CTMove *pThis, CEngineApp *pApp, BOOL Direction)
{
	int						index,
								nInsts;
	CGameObjectInstance	*pPlayer,
								*pInstance;


	index = CTMove__ModifyEnemy(pThis, pApp);
	if (index == -1)
		return;


	nInsts = pApp->m_Scene.m_nActiveAnimInstances;
	pPlayer = CEngineApp__GetPlayer(pApp);

	do
	{
		if (Direction)
		{
			index++;
			if (index >= nInsts)
				index = 0;
		}
		else
		{
			index--;
			if (index<0)
				index = nInsts-1;
		}

		pInstance = GetApp()->m_Scene.m_pActiveAnimInstances[index];
		if (pInstance == pThis->pModifyEnemy)
			break;

	} while (		(CGameObjectInstance__IsDevice(pInstance))
					|| (pInstance == pPlayer) );


	pThis->pModifyEnemy = pInstance;
}



// move enemy
//

void CTMove__MoveEnemy(CTMove *pThis, CEngineApp *pApp, CTControl *pCTControl)
{
	float						sinRotY,
								cosRotY,
								sinSideY,
								cosSideY;
	CVector3					vDesiredPos;
	CGameObjectInstance	*pInstance;


	pInstance = pThis->pModifyEnemy;
	if (!pInstance)
		return;

	// rotate enemy left
	if (CTControl__IsEnemyRotLeft(pCTControl))
	{
		pInstance->m_RotY -= ANGLE_DTOR(0.25);
		NormalizeRotation(&pInstance->m_RotY);
	}
	// rotate enemy right
	if (CTControl__IsEnemyRotRight(pCTControl))
	{
		pInstance->m_RotY += ANGLE_DTOR(0.25);
		NormalizeRotation(&pInstance->m_RotY);
	}

	// calculate sin & cos for straight ahead direction
	sinRotY = sin(pInstance->m_RotY + ANGLE_PI);
	cosRotY = cos(pInstance->m_RotY + ANGLE_PI);

	// calculate sin & cos for side stepping right direction
	sinSideY = sin(pInstance->m_RotY - ANGLE_PI/2);
	cosSideY = cos(pInstance->m_RotY - ANGLE_PI/2);

	vDesiredPos = AI_GetPos(pInstance);

	// move enemy forward
	if (CTControl__IsEnemyForward(pCTControl))
	{
		vDesiredPos.x += (0.25*SCALING_FACTOR)*sinRotY;
		vDesiredPos.z += (0.25*SCALING_FACTOR)*cosRotY;
		AI_GetDyn(pInstance)->m_vLeashCoor.x += (0.25*SCALING_FACTOR)*sinRotY;
		AI_GetDyn(pInstance)->m_vLeashCoor.z += (0.25*SCALING_FACTOR)*cosRotY;
	}
	// move enemy backward
	if (CTControl__IsEnemyBackward(pCTControl))
	{
		vDesiredPos.x -= (0.25*SCALING_FACTOR)*sinRotY;
		vDesiredPos.z -= (0.25*SCALING_FACTOR)*cosRotY;
		AI_GetDyn(pInstance)->m_vLeashCoor.x -= (0.25*SCALING_FACTOR)*sinRotY;
		AI_GetDyn(pInstance)->m_vLeashCoor.z -= (0.25*SCALING_FACTOR)*cosRotY;
	}
	// move enemy left
	if (CTControl__IsEnemyLeft(pCTControl))
	{
		vDesiredPos.x -= (0.25*SCALING_FACTOR)*sinSideY;
		vDesiredPos.z -= (0.25*SCALING_FACTOR)*cosSideY;
		AI_GetDyn(pInstance)->m_vLeashCoor.x -= (0.25*SCALING_FACTOR)*sinSideY;
		AI_GetDyn(pInstance)->m_vLeashCoor.z -= (0.25*SCALING_FACTOR)*cosSideY;
	}
	// move enemy right
	if (CTControl__IsEnemyRight(pCTControl))
	{
		vDesiredPos.x += (0.25*SCALING_FACTOR)*sinSideY;
		vDesiredPos.z += (0.25*SCALING_FACTOR)*cosSideY;
		AI_GetDyn(pInstance)->m_vLeashCoor.x += (0.25*SCALING_FACTOR)*sinSideY;
		AI_GetDyn(pInstance)->m_vLeashCoor.z += (0.25*SCALING_FACTOR)*cosSideY;
	}

	CAnimInstanceHdr__Collision3(&pInstance->ah, vDesiredPos, &ci_character);
	pThis->pLastCI = &ci_character;
}



// reset enemy position
//
void CTMove__ResetEnemy(CTMove *pThis)
{
	CGameObjectInstance	*pInstance;
	CVector3					vDesiredPos;
	int						nAnim;


	pInstance = pThis->pModifyEnemy;
	if (pInstance)
	{
		vDesiredPos = AI_GetDyn(pInstance)->m_vLeashCoor;
		CAnimInstanceHdr__Collision3(&pInstance->ah, vDesiredPos, &ci_raycast);
		pThis->pLastCI = &ci_raycast;

		CAIDynamic__ResetInteractiveAnim(AI_GetDyn(pInstance), AI_GetEA(pInstance));

		AI_GetDyn(pInstance)->m_dwStatusFlags |= AI_WAITFOR_CYCLE;
		nAnim = AI_GetEA(pInstance)->m_InteractiveAnim - 1 + AI_ANIM_INTERACTIVE_ANIMATION1;
		AI_SetDesiredAnim(pInstance, nAnim, TRUE);
		CGameObjectInstance__RestartAnim(pInstance);
	}
}
*/


// reset drawn weapons
//
void CTMove__ResetDrawnWeapons(CTMove *pThis)
{
	pThis->BowStarted = -1.0;					// stop bow from firing if held before climbing cliff
	pThis->ShockwaveStarted = 0.0;
	pThis->ChronoscepterStarted = 0.0;
	pThis->BowFlashOccurred = FALSE;			// reset bow flash
}




