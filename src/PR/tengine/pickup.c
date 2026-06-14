// pickup.cpp
#include "cppu64.h"
#include "tengine.h"
#include "tmove.h"
#include "textload.h"
#include "romstruc.h"
#include "pickup.h"
#include "sfx.h"
#include "audio.h"
#include "lists.h"
#include "regtype.h"


/****************************************************************************
* Constants
****************************************************************************/
#define MAX_PERCENTAGE_HEALTH		25		// Pickup generator max health%

/****************************************************************************
* Text
****************************************************************************/


#ifdef ENGLISH
// ---------------------- English text ---------------------------
static char	text_keystofind[] = {"0 keys to find"};
static char	text_keytofind[] = {"0 key to find"};
static char	text_onthislevel[] = {"on this level"} ;
static char	text_allkeyson[] = {"all keys on"} ;
static char	text_thislevelfound[] = {"this level found"} ;


static char	text_2health[] = {"2 health"} ;
static char	text_10health[] = {"10 health"} ;
static char	text_25health[] = {"25 health"} ;
static char	text_fullhealth[] = {"full health"} ;
static char	text_ultrahealth[] = {"ultra health"} ;
static char	text_mortalwound[] = {"mortal wound"} ;
static char	text_backpack[] = {"backpack"} ;
static char	text_spiritual_invincibility[] = {"spiritual invincibility"} ;
static char	text_chronosceptor[] = {"chronosceptor"} ;
static char	text_chronosceptor_piece[] = {"chronosceptor piece"} ;

static char	text_tekarmor[] = {"tek armor"} ;
static char	text_tekbow[] = {"tek bow"} ;
static char	text_pistol[] = {"pistol"} ;
static char	text_assaultrifle[] = {"assault rifle"} ;
static char	text_shotgun[] = {"shotgun"} ;
static char	text_autoshotgun[] = {"auto shotgun"} ;
static char	text_minigun[] = {"minigun"} ;
static char	text_grenadelauncher[] = {"grenade launcher"} ;
static char	text_pulserifle[] = {"pulse rifle"} ;
static char	text_alienweapon[] = {"alien weapon"} ;
static char	text_rocketlauncher[] = {"quad rocket launcher"} ;
static char	text_shockwaveweapon[] = {"shockwave weapon"} ;
static char	text_fusioncannon[] = {"fusion cannon"} ;

static char	text_30arrows[] = {"30 arrows"} ;
static char	text_5tekarrows[] = {"5 tek arrows"} ;
static char	text_15tekarrows[] = {"15 tek arrows"} ;
static char	text_clip[] = {"clip"} ;
static char	text_boxofbullets[] = {"box of bullets"} ;
static char	text_shotgunshells[] = {"shotgun shells"} ;
static char	text_boxofshotgunshells[] = {"box of shotgun shells"} ;
static char	text_explosiveshells[] = {"explosive shells"} ;
static char	text_boxofexplosiveshells[] = {"box of explosive shells"} ;
static char	text_minigunammo[] = {"minigun ammo"} ;
static char	text_1grenade[] = {"1 grenade"} ;
static char	text_boxofgrenades[] = {"box of grenades"} ;
static char	text_energycell[] = {"energy cell"} ;
static char	text_largeenergycell[] = {"large energy cell"} ;
static char	text_4rockets[] = {"4 rockets"} ;
static char	text_fusioncharge[] = {"fusion charge"} ;
#endif

#ifdef GERMAN
// ---------------------- German text ---------------------------
static char	text_keystofind[] = {"0 schl/ssel zu finden"};		//u.
static char	text_keytofind[] = {"0 schl/ssel zu finden"};		//u.
static char	text_onthislevel[] = {"in diesem level"} ;
static char	text_allkeyson[] = {"alle schl/ssel in"} ;			//u.
static char	text_thislevelfound[] = {"diesem level gefunden"} ;


static char	text_2health[] = {"2 gesundheit"} ;
static char	text_10health[] = {"10 gesundheit"} ;
static char	text_25health[] = {"25 gesundheit"} ;
static char	text_fullhealth[] = {"volle gesundheit"} ;
static char	text_ultrahealth[] = {"super gesundheit"} ;
static char	text_mortalwound[] = {"t>dliche wunde"} ;		//o.
static char	text_backpack[] = {"rucksack"} ;
static char	text_spiritual_invincibility[] = {"spirituelle unverwundbarkeit"} ;
static char	text_chronosceptor[] = {"chronosceptor"} ;
static char	text_chronosceptor_piece[] = {"chronosceptor teil"} ;

static char	text_tekarmor[] = {"tek r/stung"} ;		//u.
static char	text_tekbow[] = {"tek bogen"} ;
static char	text_pistol[] = {"pistole"} ;
static char	text_assaultrifle[] = {"flinte"} ;
static char	text_shotgun[] = {"gewehr"} ;
static char	text_autoshotgun[] = {"maschinenpistole"} ;
static char	text_minigun[] = {"minigewehr"} ;
static char	text_grenadelauncher[] = {"granatenwerfer"} ;
static char	text_pulserifle[] = {"impulsgewehr"} ;
static char	text_alienweapon[] = {"alienwaffe"} ;
static char	text_rocketlauncher[] = {"quad raketenwerfer"} ;
static char	text_shockwaveweapon[] = {"druckwellenwaffe"} ;
static char	text_fusioncannon[] = {"fusionskanone"} ;

static char	text_30arrows[] = {"30 pfeile"} ;
static char	text_5tekarrows[] = {"5 tek pfeile"} ;
static char	text_15tekarrows[] = {"15 tek pfeile"} ;
static char	text_clip[] = {"magazin"} ;
static char	text_boxofbullets[] = {"schachtel kugeln"} ;
static char	text_shotgunshells[] = {"gewehrpatronen"} ;
static char	text_boxofshotgunshells[] = {"schachtel gewehrpatronen"} ;
static char	text_explosiveshells[] = {"explosionspatronen"} ;
static char	text_boxofexplosiveshells[] = {"schachtel explosionspatronen"} ;
static char	text_minigunammo[] = {"miniflinten munition"} ;
static char	text_1grenade[] = {"1 granate"} ;
static char	text_boxofgrenades[] = {"schachtel granaten"} ;
static char	text_energycell[] = {"energiezelle"} ;
static char	text_largeenergycell[] = {"gross energiezelle"} ;
static char	text_4rockets[] = {"4 rakaten"} ;
static char	text_fusioncharge[] = {"fusionssprengsatz"} ;
#endif


#ifdef KANJI
// ---------------------- Japanese text ---------------------------
static char	text_keystofind[] = {0x4c, 0x5e, 0x00, 0x09, 0x63 ,0x82, 0x83, 0x93, 0x76, -1};
static char	text_keytofind[] = {0x4c, 0x5e, 0x00, 0x09, 0x63 ,0x82, 0x83, 0x93, 0x76, -1};
static char	text_onthislevel[] = {0x55, 0x63, 0x26, 0x41, 0x23, 0x2F, 0x60, 0x62, 0x69, 0x70, -1};
static char	text_allkeyson[] = {0x55, 0x63, 0x26, 0x41, 0x23, 0x2F, 0x63, -1};
static char	text_thislevelfound[] = {0x82, 0x83, 0x93, 0x58, 0x7E, 0x5D, 0x74, 0xAD, 0x60, 0xA6, 0x71, 0x5A, -1};

static char	text_2health[] = {0x21, 0x01, 0x19, 0xEB, 0x4B, 0x43, -1};
static char	text_10health[] = {0x21, 0x01, 0x19, 0xEB, 0x4B, 0x42, 0x4A, -1};
static char	text_25health[] = {0x21, 0x01, 0x19, 0xEB, 0x4B, 0x43, 0x46, -1};
static char	text_fullhealth[] = {0x19, 0x23, 0x21, 0x01, 0x19, -1};
static char	text_ultrahealth[] = {0x33, 0x41, 0x14, 0x0C, 0x21, 0x01, 0x19, -1};
static char	text_mortalwound[] = {0x21, 0x01, 0x19, 0x05, 0x02, 0x27, 0x13, 0xEB, 0x4B, -1};
static char	text_backpack[] = {0x22, 0x3E, 0x40, 0x07, -1};
static char	text_spiritual_invincibility[] = {0x82, 0x83, 0x84, -1};
static char	text_chronosceptor[] = {0x07, 0x25, 0x17, 0x0D, 0x36, 0x0F, 0x41, -1};
static char	text_chronosceptor_piece[] = {0x07, 0x25, 0x17, 0x35, 0x41, 0x0C, -1};

static char	text_tekarmor[] = {0x12, 0x40, 0x07, 0x00, 0x41, 0x1B, 0x41, -1};
static char	text_tekbow[] = {0x34, 0x26, 0x41, 0x33, 0x02, -1};
static char	text_pistol[] = {0x04, 0x41, 0x13, 0x1B, 0x29, 0x14, 0x1D, -1};
static char	text_assaultrifle[] = {0x00, 0x0A, 0x23, 0x13, 0x21, 0x01, 0x19, 0x23, -1};
static char	text_shotgun[] = {0x0B, 0x3F, 0x40, 0x13, 0x28, 0x27, -1};
static char	text_autoshotgun[] = {0x04, 0x41, 0x13, 0x0B, 0x3F, 0x40, 0x13, 0x28, 0x27, -1};
static char	text_minigun[] = {0x28, 0x13, 0x22, 0x27, 0x29, -1};
static char	text_grenadelauncher[] = {0x29, 0x24, 0x16, 0x41, 0x2F, 0x21, 0x27, 0x10, 0x3D, 0x41, -1};
static char	text_pulserifle[] = {0x34, 0x23, 0x0C, 0x21, 0x01, 0x19, 0x23, -1};
static char	text_alienweapon[] = {0x03, 0x01, 0x22, 0x00, 0x27, 0x32, 0x21, 0x0C, 0x0F, 0x41, -1};
static char	text_rocketlauncher[] = {0x1C, 0x0A, 0x01, 0x23, 0x21, 0x27, 0x10, 0x3D, 0x41, -1};
static char	text_shockwaveweapon[] = {0x0B, 0x3F, 0x40, 0x07, 0x32, 0x21, 0x0C, 0x0F, 0x41, -1};
static char	text_fusioncannon[] = {0x19, 0x3C, 0x13, 0x27, 0x06, 0x3D, 0x17, 0x27, -1};

static char	text_30arrows[] = {0x00, 0x25, 0x41, 0xEB, 0x4B, 0x44, 0x4A, -1};
static char	text_5tekarrows[] = {0x34, 0x26, 0x41, 0x00, 0x25, 0x41, 0xEB, 0x4B, 0x46, -1};
static char	text_15tekarrows[] = {0x34, 0x26, 0x41, 0x00, 0x25, 0x41, 0xEB, 0x4B, 0x42, 0x46, -1};
static char	text_clip[] = {0x85, 0xAE, -1};
static char	text_boxofbullets[] = {0x85, 0xAF, 0xB0, -1};
static char	text_shotgunshells[] = {0x05, 0x41, 0x13, 0x22, 0x40, 0x2C, -1};
static char	text_boxofshotgunshells[] = {0x05, 0x41, 0x13, 0x22, 0x40, 0x2C, 0x33, 0x40, 0x07, 0x0C, -1};
static char	text_explosiveshells[] = {0x34, 0x26, 0x41, 0x05, 0x41, 0x13, 0x22, 0x40, 0x2C, -1};
static char	text_boxofexplosiveshells[] = {0x34, 0x26, 0x41, 0x05, 0x41, 0x13, 0x22, 0x40, 0x2C, 0x33, 0x40, 0x07, 0x0C, -1};
static char	text_minigunammo[] = {0x28, 0x13, 0x22, 0x27, 0x29, 0x85, -1};
static char	text_1grenade[] = {0x29, 0x24, 0x16, 0x41, 0x2F, 0xEB, 0x4B, 0x42, -1};
static char	text_boxofgrenades[] = {0x29, 0x24, 0x16, 0x41, 0x2F, 0x33, 0x40, 0x07, 0x0C, -1};
static char	text_energycell[] = {0x34, 0x26, 0x41, 0x20, 0x15, 0x40, 0x13, -1};
static char	text_largeenergycell[] = {0x0C, 0x41, 0x34, 0x41, 0x34, 0x26, 0x41, 0x20, 0x15, 0x40, 0x13, -1};
static char	text_4rockets[] = {0x1C, 0x0A, 0x01, 0x23, 0xEB, 0x4B, 0x45, -1};
static char	text_fusioncharge[] = {0x19, 0x3C, 0x13, 0x27, 0x20, 0x15, 0x40, 0x13, -1};

#endif

/****************************************************************************
* Defines
****************************************************************************/
enum WeaponPowerOrder
{
	KNIFE = 0,
	TEKBOW,
	PISTOL,
	SHOTGUN,
	RIFLE,
	AUTOSHOTGUN,
	PULSERIFLE,
	INFANTRY,
	MINIGUN,
	POWEREDUP_TEKBOW,
	POWEREDUP_SHOTGUN,
	POWEREDUP_AUTOSHOTGUN,
	GRENADE,
	ROCKET,
	SHOCKWAVE,
	FUSION,
	CHRONO
} ;

int	RemapWeapon[] =
{
	KNIFE,			//AI_OBJECT_WEAPON_KNIFE
	0,					//AI_OBJECT_WEAPON_TOMAHAWK
	TEKBOW,			//AI_OBJECT_WEAPON_TEKBOW
	PISTOL,			//AI_OBJECT_WEAPON_SEMIAUTOMATICPISTOL
	RIFLE,			//AI_OBJECT_WEAPON_ASSAULTRIFLE
	PULSERIFLE,		//AI_OBJECT_WEAPON_MACHINEGUN
	SHOTGUN,			//AI_OBJECT_WEAPON_RIOTSHOTGUN
	AUTOSHOTGUN,	//AI_OBJECT_WEAPON_AUTOMATICSHOTGUN
	MINIGUN,			//AI_OBJECT_WEAPON_MINIGUN
	GRENADE,			//AI_OBJECT_WEAPON_GRENADE_LAUNCHER
	INFANTRY,		//AI_OBJECT_WEAPON_TECHWEAPON1
	FUSION,			//AI_OBJECT_WEAPON_TECHWEAPON2
	ROCKET,			//AI_OBJECT_WEAPON_ROCKET
	SHOCKWAVE,		//AI_OBJECT_WEAPON_SHOCKWAVE
	CHRONO,			//AI_OBJECT_WEAPON_CHRONO
} ;

/****************************************************************************
* Functions
****************************************************************************/

// show keys remaining for this level...
void CPickup__DisplayKeysRemaining(void)
{
	int			keys = 0 ;
	t_GameText	*txtA, *txtB ;


	switch (GetApp()->m_Level)
	{
		case 1:
			keys = 6 ;			// 6 keys initially
			if (CTurokMovement.Level2Keys & 1)
				keys-- ;
			if (CTurokMovement.Level2Keys & 2)
				keys-- ;
			if (CTurokMovement.Level2Keys & 4)
				keys-- ;
			if (CTurokMovement.Level3Keys & 1)
				keys-- ;
			if (CTurokMovement.Level3Keys & 2)
				keys-- ;
			if (CTurokMovement.Level3Keys & 4)
				keys-- ;
			break ;

		case 2:
			keys = 3 ;			// 3 keys initially
			if (CTurokMovement.Level4Keys & 1)
				keys-- ;
			if (CTurokMovement.Level4Keys & 2)
				keys-- ;
			if (CTurokMovement.Level5Keys & 4)
				keys-- ;
			break ;

		case 3:
			keys = 3 ;			// 3 keys initially
			if (CTurokMovement.Level4Keys & 4)
				keys-- ;
			if (CTurokMovement.Level5Keys & 1)
				keys-- ;
			if (CTurokMovement.Level5Keys & 2)
				keys-- ;
			break ;

		case 4:
			keys = 3 ;			// 3 keys initially
			if (CTurokMovement.Level6Keys & 1)
				keys-- ;
			if (CTurokMovement.Level6Keys & 2)
				keys-- ;
			if (CTurokMovement.Level8Keys & 1)
				keys-- ;
			break ;

		case 5:
			keys = 3 ;			// 3 keys initially
			if (CTurokMovement.Level6Keys & 4)
				keys-- ;
			if (CTurokMovement.Level8Keys & 2)
				keys-- ;
			if (CTurokMovement.Level8Keys & 4)
				keys-- ;
			break ;

		case 6:
			keys = 3 ;			// 3 keys initially
			if (CTurokMovement.Level7Keys & 1)
				keys-- ;
			if (CTurokMovement.Level7Keys & 2)
				keys-- ;
			if (CTurokMovement.Level7Keys & 4)
				keys-- ;
			break ;

		case 7:
			keys = 2 ;			// 2 keys initially
			if (CTurokMovement.Level8Keys & 8)
				keys-- ;
			if (CTurokMovement.Level8Keys & 16)
				keys-- ;
			break ;
	}

	if (keys)
	{
		if (keys == 1)
		{
#ifdef KANJI
			text_keytofind[2] = keys + 0x41 ;
#else
			text_keytofind[0] = keys | 0x30 ;
#endif
			txtA = COnScreen__AddGameText(text_keytofind) ;
		}
		else
		{
#ifdef KANJI
			text_keystofind[2] = keys + 0x41 ;
#else
			text_keystofind[0] = keys | 0x30 ;
#endif
			txtA = COnScreen__AddGameText(text_keystofind) ;
		}
		txtB = COnScreen__AddGameText(text_onthislevel) ;
	}
	else
	{
		txtA = COnScreen__AddGameText(text_allkeyson) ;
		txtB = COnScreen__AddGameText(text_thislevelfound) ;
	}

	txtA->DisplayTime = 64 ;
	txtB->DisplayTime = 64 ;
}


#define	TMOVE_MAX_LIVES	9		// redefined from TMOVE.C

int	LivesCodes[] =
{
	19,
	-1
} ;


BOOL CTMove__CanPickItUp(CTMove *pThis, INT32 PickupType)
{
	CGameObjectInstance	*pPlayer = CEngineApp__GetPlayer(GetApp()) ;

	switch (PickupType)
	{
		case AI_OBJECT_PICKUP_SMALL_TOKEN:
		case AI_OBJECT_PICKUP_LARGE_TOKEN:
			if (	(CTurokMovement.Lives >= TMOVE_MAX_LIVES)
				&& (CTurokMovement.Tokens == 99))
			{
				if (CTurokMovement.CheatGiven == FALSE)
				{
					GetApp()->m_bPause = TRUE;
					CPause__Construct(&GetApp()->m_Pause);
					GetApp()->m_Pause.m_Mode = PAUSE_NORMAL;
					CGiveCheatCode__Construct(&GetApp()->m_GiveCheatCode, LivesCodes) ;
					GetApp()->m_bGiveCheatCode = TRUE ;
				}
				CTurokMovement.CheatGiven = TRUE ;
				return FALSE ;
			}
			break ;

		case AI_OBJECT_PICKUP_HEALTH_2:
		case AI_OBJECT_PICKUP_HEALTH_100PLUS:
			if (pPlayer->m_AI.m_Health >= 250)
				return FALSE ;
			break ;

		case AI_OBJECT_PICKUP_HEALTH_10:
		case AI_OBJECT_PICKUP_HEALTH_25:
		case AI_OBJECT_PICKUP_HEALTH_100:
			if (pPlayer->m_AI.m_Health >= CTurokMovement.MaxHealth)
				return FALSE ;
			break ;

		case AI_OBJECT_PICKUP_AMMO_ARROWS:
			if (CTMove__WeaponFull(pThis, AI_OBJECT_WEAPON_TEKBOW, FALSE))
				return FALSE ;
			break ;

		case AI_OBJECT_PICKUP_AMMO_EXPARROWSSMALL:
		case AI_OBJECT_PICKUP_AMMO_EXPARROWSLARGE:
			if (CTMove__WeaponFull(pThis, AI_OBJECT_WEAPON_TEKBOW, TRUE))
				return FALSE ;
			break ;

		case AI_OBJECT_PICKUP_AMMO_BULLETCLIP:
		case AI_OBJECT_PICKUP_AMMO_BULLETBOX:
			if (CTMove__WeaponFull(pThis, AI_OBJECT_WEAPON_SEMIAUTOMATICPISTOL, FALSE))
				return FALSE ;
			break ;

		case AI_OBJECT_PICKUP_AMMO_SHOTGUN:
		case AI_OBJECT_PICKUP_AMMO_SHOTGUNBOX:
			if (CTMove__WeaponFull(pThis, AI_OBJECT_WEAPON_RIOTSHOTGUN, FALSE))
				return FALSE ;
			break ;

		case AI_OBJECT_PICKUP_AMMO_EXPSHOTGUN:
		case AI_OBJECT_PICKUP_AMMO_EXPSHOTGUNBOX:
			if (CTMove__WeaponFull(pThis, AI_OBJECT_WEAPON_RIOTSHOTGUN, TRUE))
				return FALSE ;
			break ;

		case AI_OBJECT_PICKUP_AMMO_MINIGUN:
			if (CTMove__WeaponFull(pThis, AI_OBJECT_WEAPON_MINIGUN, FALSE))
				return FALSE ;
			break ;

		case AI_OBJECT_PICKUP_AMMO_GRENADES:
		case AI_OBJECT_PICKUP_AMMO_GRENADEBOX:
			if (CTMove__WeaponFull(pThis, AI_OBJECT_WEAPON_GRENADE_LAUNCHER, FALSE))
				return FALSE ;
			break ;

		case AI_OBJECT_PICKUP_AMMO_TECHSMALL:
		case AI_OBJECT_PICKUP_AMMO_TECHLARGE:
			if (CTMove__WeaponFull(pThis, AI_OBJECT_WEAPON_TECHWEAPON1, FALSE))
				return FALSE ;
			break ;

		case AI_OBJECT_PICKUP_AMMO_ROCKETS:
			if (CTMove__WeaponFull(pThis, AI_OBJECT_WEAPON_ROCKET, FALSE))
				return FALSE ;
			break ;

		case AI_OBJECT_PICKUP_AMMO_TECHWEAPON2:
			if (CTMove__WeaponFull(pThis, AI_OBJECT_WEAPON_TECHWEAPON2, FALSE))
				return FALSE ;
			break ;

		case AI_OBJECT_PICKUP_BACKPACK:
			if (CTurokMovement.BackPackFlag)
				return FALSE ;
			break ;
	}

	return TRUE ;
}

BOOL CPickup__CanInstancePickItUp(CGameSimpleInstance *pPickup, CAnimInstanceHdr *pInst)
{
	CGameObjectInstance	*pPlayer = CEngineApp__GetPlayer(GetApp()) ;
	if (pInst != &pPlayer->ah)
		return FALSE;
	else
		return CTMove__CanPickItUp(&CTurokMovement, CInstanceHdr__TypeFlag(&pPickup->ah.ih)) ;
}



int CPickup__RemapWeapon(int weapon)
{
	int	remap ;

	remap = RemapWeapon[weapon - AI_OBJECT_WEAPON_START] ;

	if ((remap == TEKBOW) && (CTurokMovement.ExpTekBowAmmo))
		remap = POWEREDUP_TEKBOW ;

	if ((remap == SHOTGUN) && (CTurokMovement.ExpShotgunPool))
		remap = POWEREDUP_SHOTGUN ;

	if ((remap == AUTOSHOTGUN) && (CTurokMovement.ExpShotgunPool))
		remap = POWEREDUP_AUTOSHOTGUN ;

	return remap ;
}



void CPickup_Pickup(CGameSimpleInstance *pPickup)
{
	CVector3					vPos;
	CGameObjectInstance	*pPlayer;
	int						CurrentWeapon,
								duckFlag;
	BOOL						noammo ;
	CInstanceHdr			*pInst ;

	// dynamic simple instance may have been deallocated since the collision
	// list was built
	if (pPickup->m_wFlags & SIMPLE_FLAG_DYNAMIC)
		if (!CDynamicSimple__IsAllocated((CDynamicSimple*) pPickup))
			return;

	vPos.x = AW.Ears.vPos.x;
	vPos.y = AW.Ears.vPos.y;
	vPos.z = AW.Ears.vPos.z;

	// at the end of the switch, should we delete the pickup
	// only time it wouldn't is if a weapon is full...
	pPlayer = CEngineApp__GetPlayer(GetApp());

	duckFlag = CEngineApp__GetPlayerDuckStatus(GetApp());

	CurrentWeapon = CPickup__RemapWeapon(CTurokMovement.WeaponSelect) ;

	noammo = FALSE ;

	// do whatever is needed
	switch (CInstanceHdr__TypeFlag(&pPickup->ah.ih))
	{
		// ------------------------------------------------------------------
		// Health Types
		// ------------------------------------------------------------------
		// Adds to health
		// some types go above 100, some increase max to above 100

		case AI_OBJECT_PICKUP_HEALTH_2:
			pPlayer->m_AI.m_Health += 2 ;
			if (pPlayer->m_AI.m_Health >250)
				pPlayer->m_AI.m_Health = 250 ;
			CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_HEALTH_PICKUP_1, 0, NULL, &vPos, 0);

			COnScreen__AddGameText(text_2health) ;
			break ;

		case AI_OBJECT_PICKUP_HEALTH_10:
			pPlayer->m_AI.m_Health += 10 ;
			if (pPlayer->m_AI.m_Health > CTurokMovement.MaxHealth)
				pPlayer->m_AI.m_Health = CTurokMovement.MaxHealth ;

			CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_HEALTH_PICKUP_1, 0, NULL, &vPos, 0);
			COnScreen__AddGameText(text_10health) ;
			break ;

		case AI_OBJECT_PICKUP_HEALTH_25:
			pPlayer->m_AI.m_Health += 25 ;
			if (pPlayer->m_AI.m_Health > CTurokMovement.MaxHealth)
				pPlayer->m_AI.m_Health = CTurokMovement.MaxHealth ;
			CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_HEALTH_PICKUP_1, 0, NULL, &vPos, 0);
			COnScreen__AddGameText(text_25health) ;
			break ;

		case AI_OBJECT_PICKUP_HEALTH_100:
			pPlayer->m_AI.m_Health = CTurokMovement.MaxHealth ;
			CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_HEALTH_PICKUP_1, 0, NULL, &vPos, 0);
			COnScreen__AddGameText(text_fullhealth) ;
			break ;

		case AI_OBJECT_PICKUP_HEALTH_100PLUS:
			pPlayer->m_AI.m_Health += 100 ;			// increase regardless of current max
			if (pPlayer->m_AI.m_Health >250)
				pPlayer->m_AI.m_Health = 250 ;
			CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_HEALTH_PICKUP_1, 0, NULL, &vPos, 0);
			COnScreen__AddGameText(text_ultrahealth) ;
			break ;


		case AI_OBJECT_PICKUP_MORTAL_WOUND:
			CTurokMovement.MaxHealth += 5 ;
			if (CTurokMovement.MaxHealth > AI_HEALTH_PLAYER_MAX)
				CTurokMovement.MaxHealth = AI_HEALTH_PLAYER_MAX ;

			if (pPlayer->m_AI.m_Health < CTurokMovement.MaxHealth)
				pPlayer->m_AI.m_Health = CTurokMovement.MaxHealth ;
			CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_4, 0, NULL, &vPos, 0);
			COnScreen__AddGameText(text_mortalwound) ;
			break ;

		// ------------------------------------------------------------------
		// Misc Types
		// ------------------------------------------------------------------
		case AI_OBJECT_PICKUP_BACKPACK:
			CTurokMovement.BackPackFlag = TRUE;
			CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_4, 0, NULL, &vPos, 0);
			COnScreen__AddGameText(text_backpack) ;
			break ;

		case AI_OBJECT_PICKUP_SPIRITUAL:
			COnScreen__AddGameText(text_spiritual_invincibility) ;
			CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_4, 0, NULL, &vPos, 0);

			CTurokMovement.SpiritualTime = SECONDS_TO_FRAMES(SPIRITUAL_TIMER) ;
			break ;

//		case AI_OBJECT_PICKUP_MAP:
//			COnScreen__AddGameText("map") ;
//			CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_4, 0, NULL, &vPos, 0);
//			CEngineApp__RevealAllMap(GetApp()) ;
//			// set map TRUE
//			break ;

		case AI_OBJECT_PICKUP_KEY2:
			CTurokMovement.LastKeyTexture = 0 ;
			CTurokMovement.Level2Keys |= (1<<(AI_GetEA(pPickup)->m_Id&3)) ;
			CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_4, 0, NULL, &vPos, 0);
			CCamera__FadeToCinema(&GetApp()->m_Camera, CINEMA_FLAG_TUROK_PICKUP_KEY) ;
			break ;

		case AI_OBJECT_PICKUP_KEY3:
			CTurokMovement.LastKeyTexture = 1 ;
			CTurokMovement.Level3Keys |= (1<<(AI_GetEA(pPickup)->m_Id&3)) ;
			CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_4, 0, NULL, &vPos, 0);
			CCamera__FadeToCinema(&GetApp()->m_Camera, CINEMA_FLAG_TUROK_PICKUP_KEY) ;
			break ;

		case AI_OBJECT_PICKUP_KEY4:
			CTurokMovement.LastKeyTexture = 2 ;
			CTurokMovement.Level4Keys |= (1<<(AI_GetEA(pPickup)->m_Id&3)) ;
			CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_4, 0, NULL, &vPos, 0);
			CCamera__FadeToCinema(&GetApp()->m_Camera, CINEMA_FLAG_TUROK_PICKUP_KEY) ;
			break ;

		case AI_OBJECT_PICKUP_KEY5:
			CTurokMovement.LastKeyTexture = 3 ;
			CTurokMovement.Level5Keys |= (1<<(AI_GetEA(pPickup)->m_Id&3)) ;
			CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_4, 0, NULL, &vPos, 0);
			CCamera__FadeToCinema(&GetApp()->m_Camera, CINEMA_FLAG_TUROK_PICKUP_KEY) ;
			break ;

		case AI_OBJECT_PICKUP_KEY6:
			CTurokMovement.LastKeyTexture = 4 ;
			CTurokMovement.Level6Keys |= (1<<(AI_GetEA(pPickup)->m_Id&3)) ;
			CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_4, 0, NULL, &vPos, 0);
			CCamera__FadeToCinema(&GetApp()->m_Camera, CINEMA_FLAG_TUROK_PICKUP_KEY) ;
			break ;

		case AI_OBJECT_PICKUP_KEY7:
			CTurokMovement.LastKeyTexture = 5 ;
			CTurokMovement.Level7Keys |= (1<<(AI_GetEA(pPickup)->m_Id&3)) ;
			CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_4, 0, NULL, &vPos, 0);
			CCamera__FadeToCinema(&GetApp()->m_Camera, CINEMA_FLAG_TUROK_PICKUP_KEY) ;
			break ;

		case AI_OBJECT_PICKUP_KEY8:
		case AI_OBJECT_PICKUP_KEY8MANTIS:
			// setup for different sun key models, uses texture 0!
			if (AI_GetEA(pPickup)->m_Id != 4)
				CTurokMovement.LastKeyTexture = -1 ;
			else
				CTurokMovement.LastKeyTexture = -2 ;

			CTurokMovement.Level8Keys |= (1<<(AI_GetEA(pPickup)->m_Id%10)) ;
			CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_4, 0, NULL, &vPos, 0);
			CCamera__FadeToCinema(&GetApp()->m_Camera, CINEMA_FLAG_TUROK_PICKUP_KEY) ;
			break ;



		case AI_OBJECT_PICKUP_CHRONOSCEPTOR1:
			CTurokMovement.ChronoSceptorPieces |= 1 ;
			goto chrono;
		case AI_OBJECT_PICKUP_CHRONOSCEPTOR2:
			CTurokMovement.ChronoSceptorPieces |= 2 ;
			goto chrono;
		case AI_OBJECT_PICKUP_CHRONOSCEPTOR3:
			CTurokMovement.ChronoSceptorPieces |= 4 ;
			goto chrono;
		case AI_OBJECT_PICKUP_CHRONOSCEPTOR4:
			CTurokMovement.ChronoSceptorPieces |= 8 ;
			goto chrono;
		case AI_OBJECT_PICKUP_CHRONOSCEPTOR5:
			CTurokMovement.ChronoSceptorPieces |= 16 ;
			goto chrono;
		case AI_OBJECT_PICKUP_CHRONOSCEPTOR6:
			CTurokMovement.ChronoSceptorPieces |= 32 ;
			goto chrono;
		case AI_OBJECT_PICKUP_CHRONOSCEPTOR7:
			CTurokMovement.ChronoSceptorPieces |= 64 ;
			goto chrono;
		case AI_OBJECT_PICKUP_CHRONOSCEPTOR8:
			CTurokMovement.ChronoSceptorPieces |= 128 ;

chrono:
			CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_1, 0, NULL, &vPos, 0);

			// completed weapon
			if (CTurokMovement.ChronoSceptorPieces == 0xff)
			{
				COnScreen__AddGameText(text_chronosceptor) ;
				CTurokMovement.ChronoscepterFlag	= TRUE;
				CTMove__AddAmmo(&CTurokMovement, AI_OBJECT_WEAPON_CHRONOSCEPTER, DEFAULT_CHRONOSCEPTOR_AMMO) ;

				CTMove__SelectWeaponByAIType(&CTurokMovement, AI_OBJECT_WEAPON_CHRONOSCEPTER, FALSE) ;
				CTMove__ResetDrawnWeapons(&CTurokMovement) ;
			}
			else
				COnScreen__AddGameText(text_chronosceptor_piece) ;
			break ;





		// ------------------------------------------------------------------
		// Armor Types
		// ------------------------------------------------------------------
		case AI_OBJECT_PICKUP_ARMOR_TEK:
			CTurokMovement.ArmorFlag = TRUE;
			CTurokMovement.ArmorAmount += (DEFAULT_TEK_ARMOR *3);
			CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_HEALTH_PICKUP_1, 0, NULL, &vPos, 0);
			COnScreen__AddGameText(text_tekarmor) ;
			break ;

		// ------------------------------------------------------------------
		// Token Types
		// ------------------------------------------------------------------
		case AI_OBJECT_PICKUP_SMALL_TOKEN:
			CTurokMovement.Tokens += 1 ;

			if (	(CTurokMovement.Lives >= TMOVE_MAX_LIVES)
				&& (CTurokMovement.Tokens >= 99))
				CTurokMovement.Tokens = 99 ;
			else if (CTurokMovement.Tokens >= 100)
			{
				CTurokMovement.Tokens -= 100 ;
				CTMove__IncreaseLives(&CTurokMovement, 1);
			}

			CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_HEALTH_PICKUP_1, 0, NULL, &vPos, 0);
//   		COnScreen__AddGameText("life force") ;
			break ;

		case AI_OBJECT_PICKUP_LARGE_TOKEN:
			CTurokMovement.Tokens += 10 ;

			if (	(CTurokMovement.Lives >= TMOVE_MAX_LIVES)
				&& (CTurokMovement.Tokens >= 99))
				CTurokMovement.Tokens = 99 ;
			else if (CTurokMovement.Tokens >= 100)
			{
				CTurokMovement.Tokens -= 100 ;
				CTMove__IncreaseLives(&CTurokMovement, 1);
			}

			CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_HEALTH_PICKUP_1, 0, NULL, &vPos, 0);
//   		COnScreen__AddGameText("10 life force") ;
			break ;




		// ------------------------------------------------------------------
		// Weapon Types
		// ------------------------------------------------------------------
		// when picking up a weapon, it will become the currently chosen one
		// only if it is of higher power than the current weapon using,
		// if weapon is not owned, but you have all the ammo, pick it up with no ammo

		case AI_OBJECT_PICKUP_TEKBOW:
			// you always have this weapon, so any checks are redundant (made payable to carl!)
			COnScreen__AddGameText(text_tekbow) ;
			CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_1, 0, NULL, &vPos, 0);

			if (		(TEKBOW > CurrentWeapon)
					&&	(CTurokMovement.WaterFlag != PLAYER_BELOW_WATERSURFACE)
					&&	(duckFlag == PLAYER_NOT_DUCKING) )
			{
				CTMove__SelectWeaponByAIType(&CTurokMovement, AI_OBJECT_WEAPON_TEKBOW, FALSE) ;
				CTMove__ResetDrawnWeapons(&CTurokMovement) ;
			}
			break ;

		case AI_OBJECT_PICKUP_PISTOL:
			if (CTurokMovement.SemiAutomaticPistolFlag == FALSE)
				COnScreen__AddGameText(text_pistol) ;
			CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_1, 0, NULL, &vPos, 0);
			CTurokMovement.SemiAutomaticPistolFlag	= TRUE;
			CTMove__AddAmmo(&CTurokMovement, AI_OBJECT_WEAPON_SEMIAUTOMATICPISTOL, DEFAULT_PISTOL_AMMO) ;

			if (		(PISTOL > CurrentWeapon)
					&& (CTurokMovement.WaterFlag != PLAYER_BELOW_WATERSURFACE)
					&&	(duckFlag == PLAYER_NOT_DUCKING) )
			{
				CTMove__SelectWeaponByAIType(&CTurokMovement, AI_OBJECT_WEAPON_SEMIAUTOMATICPISTOL, FALSE) ;
				CTMove__ResetDrawnWeapons(&CTurokMovement) ;
			}
			break ;

		case AI_OBJECT_PICKUP_RIFLE:
			if (CTurokMovement.AssaultRifleFlag	== FALSE)
				COnScreen__AddGameText(text_assaultrifle) ;
			CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_1, 0, NULL, &vPos, 0);
			CTurokMovement.AssaultRifleFlag	= TRUE;
			CTMove__AddAmmo(&CTurokMovement, AI_OBJECT_WEAPON_ASSAULTRIFLE, DEFAULT_RIFLE_AMMO) ;

			if (		(RIFLE > CurrentWeapon)
					&& (CTurokMovement.WaterFlag != PLAYER_BELOW_WATERSURFACE)
					&& (duckFlag == PLAYER_NOT_DUCKING) )
			{
				CTMove__SelectWeaponByAIType(&CTurokMovement, AI_OBJECT_WEAPON_ASSAULTRIFLE, FALSE) ;
				CTMove__ResetDrawnWeapons(&CTurokMovement) ;
			}
			break ;

		case AI_OBJECT_PICKUP_RIOTSHOTGUN:
			if (CTurokMovement.RiotShotgunFlag == FALSE)
				COnScreen__AddGameText(text_shotgun) ;
			CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_2, 0, NULL, &vPos, 0);
			CTurokMovement.RiotShotgunFlag = TRUE;
			CTMove__AddAmmo(&CTurokMovement, AI_OBJECT_WEAPON_RIOTSHOTGUN, DEFAULT_RIOTSHOTGUN_AMMO) ;

			if (		(SHOTGUN > CurrentWeapon)
					&& (CTurokMovement.WaterFlag != PLAYER_BELOW_WATERSURFACE)
					&& (duckFlag == PLAYER_NOT_DUCKING) )
			{
				CTMove__SelectWeaponByAIType(&CTurokMovement, AI_OBJECT_WEAPON_RIOTSHOTGUN, FALSE) ;
				CTMove__ResetDrawnWeapons(&CTurokMovement) ;
			}
			break ;

		case AI_OBJECT_PICKUP_AUTOSHOTGUN:
			if (CTurokMovement.AutomaticShotgunFlag == FALSE)
				COnScreen__AddGameText(text_autoshotgun) ;
			CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_2, 0, NULL, &vPos, 0);
			CTurokMovement.AutomaticShotgunFlag = TRUE;
			CTMove__AddAmmo(&CTurokMovement, AI_OBJECT_WEAPON_AUTOMATICSHOTGUN, DEFAULT_AUTOSHOTGUN_AMMO) ;

			if (		(AUTOSHOTGUN > CurrentWeapon)
					&& (CTurokMovement.WaterFlag != PLAYER_BELOW_WATERSURFACE)
					&& (duckFlag == PLAYER_NOT_DUCKING) )
			{
				CTMove__SelectWeaponByAIType(&CTurokMovement, AI_OBJECT_WEAPON_AUTOMATICSHOTGUN, FALSE) ;
				CTMove__ResetDrawnWeapons(&CTurokMovement) ;
			}
			break ;

		case AI_OBJECT_PICKUP_MINIGUN:
			if (CTurokMovement.MiniGunFlag == FALSE)
				COnScreen__AddGameText(text_minigun) ;
			CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_1, 0, NULL, &vPos, 0);
			CTurokMovement.MiniGunFlag = TRUE;
			CTMove__AddAmmo(&CTurokMovement, AI_OBJECT_WEAPON_MINIGUN, DEFAULT_MINIGUN_AMMO) ;

			if (		(MINIGUN > CurrentWeapon)
					&& (CTurokMovement.WaterFlag != PLAYER_BELOW_WATERSURFACE)
					&& (duckFlag == PLAYER_NOT_DUCKING) )
			{
				CTMove__SelectWeaponByAIType(&CTurokMovement, AI_OBJECT_WEAPON_MINIGUN, FALSE) ;
				CTMove__ResetDrawnWeapons(&CTurokMovement) ;
			}
			break ;

		case AI_OBJECT_PICKUP_GRENADELAUNCHER:
			if (CTurokMovement.GrenadeLauncherFlag == FALSE)
				COnScreen__AddGameText(text_grenadelauncher) ;
			CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_5, 0, NULL, &vPos, 0);
			CTurokMovement.GrenadeLauncherFlag = TRUE;
			CTMove__AddAmmo(&CTurokMovement, AI_OBJECT_WEAPON_GRENADE_LAUNCHER, DEFAULT_GRENADELAUNCHER_AMMO) ;

			if (		(GRENADE > CurrentWeapon)
					&& (CTurokMovement.WaterFlag != PLAYER_BELOW_WATERSURFACE)
					&& (duckFlag == PLAYER_NOT_DUCKING) )
			{
				CTMove__SelectWeaponByAIType(&CTurokMovement, AI_OBJECT_WEAPON_GRENADE_LAUNCHER, FALSE) ;
				CTMove__ResetDrawnWeapons(&CTurokMovement) ;
			}
			break ;

		// longhunter pulse rifle
		case AI_OBJECT_PICKUP_MACHINEGUN:
			if (CTurokMovement.MachineGunFlag == FALSE)
				COnScreen__AddGameText(text_pulserifle) ;
			CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_3, 0, NULL, &vPos, 0);
			CTurokMovement.MachineGunFlag = TRUE;
			CTMove__AddAmmo(&CTurokMovement, AI_OBJECT_WEAPON_MACHINEGUN, DEFAULT_MACHINEGUN_AMMO) ;

			if (		(PULSERIFLE > CurrentWeapon)
					&& (CTurokMovement.WaterFlag != PLAYER_BELOW_WATERSURFACE)
					&& (duckFlag == PLAYER_NOT_DUCKING) )
			{
				CTMove__SelectWeaponByAIType(&CTurokMovement, AI_OBJECT_WEAPON_MACHINEGUN, FALSE) ;
				CTMove__ResetDrawnWeapons(&CTurokMovement) ;
			}
			break ;

		case AI_OBJECT_PICKUP_TECHWEAPON1:
			if (CTurokMovement.TechWeapon1Flag == FALSE)
				COnScreen__AddGameText(text_alienweapon) ;
			CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_3, 0, NULL, &vPos, 0);
			CTurokMovement.TechWeapon1Flag = TRUE;
			CTMove__AddAmmo(&CTurokMovement, AI_OBJECT_WEAPON_TECHWEAPON1, DEFAULT_TECH1_AMMO) ;

			if (		(INFANTRY > CurrentWeapon)
					&& (CTurokMovement.WaterFlag != PLAYER_BELOW_WATERSURFACE)
					&& (duckFlag == PLAYER_NOT_DUCKING) )
			{
				CTMove__SelectWeaponByAIType(&CTurokMovement, AI_OBJECT_WEAPON_TECHWEAPON1, FALSE) ;
				CTMove__ResetDrawnWeapons(&CTurokMovement) ;
			}
			break ;


		case AI_OBJECT_PICKUP_ROCKET:
			if (CTurokMovement.RocketFlag == FALSE)
				COnScreen__AddGameText(text_rocketlauncher) ;
			CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_7, 0, NULL, &vPos, 0);
			CTurokMovement.RocketFlag = TRUE;
			CTMove__AddAmmo(&CTurokMovement, AI_OBJECT_WEAPON_ROCKET, DEFAULT_ROCKET_AMMO) ;

			if (		(ROCKET > CurrentWeapon)
					&& (CTurokMovement.WaterFlag != PLAYER_BELOW_WATERSURFACE)
					&& (duckFlag == PLAYER_NOT_DUCKING) )
			{
				CTMove__SelectWeaponByAIType(&CTurokMovement, AI_OBJECT_WEAPON_ROCKET, FALSE) ;
				CTMove__ResetDrawnWeapons(&CTurokMovement) ;
			}
			break ;

		case AI_OBJECT_PICKUP_SHOCKWAVE:
			if (CTurokMovement.ShockwaveFlag == FALSE)
				COnScreen__AddGameText(text_shockwaveweapon) ;
			CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_3, 0, NULL, &vPos, 0);
			CTurokMovement.ShockwaveFlag = TRUE;
			CTMove__AddAmmo(&CTurokMovement, AI_OBJECT_WEAPON_SHOCKWAVE, DEFAULT_SHOCKWAVE_AMMO) ;

			if (		(SHOCKWAVE > CurrentWeapon)
					&& (CTurokMovement.WaterFlag != PLAYER_BELOW_WATERSURFACE)
					&& (duckFlag == PLAYER_NOT_DUCKING) )
			{
				CTMove__SelectWeaponByAIType(&CTurokMovement, AI_OBJECT_WEAPON_SHOCKWAVE, FALSE) ;
				CTMove__ResetDrawnWeapons(&CTurokMovement) ;
			}
			break ;

		case AI_OBJECT_PICKUP_TECHWEAPON2:
			if (CTurokMovement.TechWeapon2Flag == FALSE)
				COnScreen__AddGameText(text_fusioncannon) ;
			CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_3, 0, NULL, &vPos, 0);
			CTurokMovement.TechWeapon2Flag = TRUE;
			CTMove__AddAmmo(&CTurokMovement, AI_OBJECT_WEAPON_TECHWEAPON2, DEFAULT_TECH2_AMMO) ;

			if (		(FUSION > CurrentWeapon)
					&& (CTurokMovement.WaterFlag != PLAYER_BELOW_WATERSURFACE)
					&& (duckFlag == PLAYER_NOT_DUCKING) )
			{
				CTMove__SelectWeaponByAIType(&CTurokMovement, AI_OBJECT_WEAPON_TECHWEAPON2, FALSE) ;
				CTMove__ResetDrawnWeapons(&CTurokMovement) ;
			}
			break ;


		// ------------------------------------------------------------------
		// Ammunition Types
		// ------------------------------------------------------------------
		// adds to pool, or individual weapon ammo
		// if weapon is full, will not pickup.
		// if ammo is currently empty, and weapon is higher power than
		// weapon currently using, switch to it.

		case AI_OBJECT_PICKUP_AMMO_ARROWS:
			if (CTurokMovement.TekBowAmmo == 0)
				noammo = TRUE ;
			COnScreen__AddGameText(text_30arrows) ;
			CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_6, 0, NULL, &vPos, 0);
			CTMove__AddAmmo(&CTurokMovement, AI_OBJECT_WEAPON_TEKBOW, REFILL_ARROW) ;

			if (		(noammo)
					&& (TEKBOW > CurrentWeapon)
					&& (CTurokMovement.WaterFlag != PLAYER_BELOW_WATERSURFACE)
					&& (duckFlag == PLAYER_NOT_DUCKING) )
			{
				CTMove__SelectWeaponByAIType(&CTurokMovement, AI_OBJECT_WEAPON_TEKBOW, FALSE) ;
				CTMove__ResetDrawnWeapons(&CTurokMovement) ;
			}
			break ;

		case AI_OBJECT_PICKUP_AMMO_EXPARROWSSMALL:
			if (CTurokMovement.ExpTekBowAmmo == 0)
				noammo = TRUE ;
			COnScreen__AddGameText(text_5tekarrows) ;
			CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_6, 0, NULL, &vPos, 0);
			CTMove__AddAmmo(&CTurokMovement, AI_OBJECT_WEAPON_TEKBOW, -REFILL_EXPARROW_SMALL) ;

			if (		(noammo)
					&& (POWEREDUP_TEKBOW > CurrentWeapon)
					&& (CTurokMovement.WaterFlag != PLAYER_BELOW_WATERSURFACE)
					&& (duckFlag == PLAYER_NOT_DUCKING) )
			{
				CTMove__SelectWeaponByAIType(&CTurokMovement, AI_OBJECT_WEAPON_TEKBOW, FALSE) ;
				CTMove__ResetDrawnWeapons(&CTurokMovement) ;
			}

			break ;

		case AI_OBJECT_PICKUP_AMMO_EXPARROWSLARGE:
			if (CTurokMovement.ExpTekBowAmmo == 0)
				noammo = TRUE ;
			COnScreen__AddGameText(text_15tekarrows) ;
			CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_6, 0, NULL, &vPos, 0);
			CTMove__AddAmmo(&CTurokMovement, AI_OBJECT_WEAPON_TEKBOW, -REFILL_EXPARROW_LARGE) ;

			if (		(noammo)
					&& (POWEREDUP_TEKBOW > CurrentWeapon)
					&& (CTurokMovement.WaterFlag != PLAYER_BELOW_WATERSURFACE)
					&& (duckFlag == PLAYER_NOT_DUCKING) )
			{
				CTMove__SelectWeaponByAIType(&CTurokMovement, AI_OBJECT_WEAPON_TEKBOW, FALSE) ;
				CTMove__ResetDrawnWeapons(&CTurokMovement) ;
			}
			break ;

		case AI_OBJECT_PICKUP_AMMO_BULLETCLIP:
		case AI_OBJECT_PICKUP_AMMO_BULLETBOX:
			// if you have an empty pistol, or rifle, and its more powerful than current, use it
			if (CTurokMovement.BulletPool == 0)
				noammo = TRUE ;
			if (CInstanceHdr__TypeFlag(&pPickup->ah.ih) == AI_OBJECT_PICKUP_AMMO_BULLETCLIP)
			{
				COnScreen__AddGameText(text_clip) ;
				CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_1, 0, NULL, &vPos, 0);
				CTMove__AddAmmo(&CTurokMovement, AI_OBJECT_WEAPON_SEMIAUTOMATICPISTOL, REFILL_BULLET_CLIP) ;
			}
			else
			{
				COnScreen__AddGameText(text_boxofbullets) ;
				CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_1, 0, NULL, &vPos, 0);
				CTMove__AddAmmo(&CTurokMovement, AI_OBJECT_WEAPON_SEMIAUTOMATICPISTOL, REFILL_BULLET_BOX) ;
			}
			if (noammo)
			{
				if (		(CTurokMovement.AssaultRifleFlag)
						&& (RIFLE > CurrentWeapon)
						&& (CTurokMovement.WaterFlag != PLAYER_BELOW_WATERSURFACE)
						&& (duckFlag == PLAYER_NOT_DUCKING) )
				{
					CTMove__SelectWeaponByAIType(&CTurokMovement, AI_OBJECT_WEAPON_ASSAULTRIFLE, FALSE) ;
					CTMove__ResetDrawnWeapons(&CTurokMovement) ;
				}
				else if (		(CTurokMovement.SemiAutomaticPistolFlag)
								&& (PISTOL > CurrentWeapon)
								&& (CTurokMovement.WaterFlag != PLAYER_BELOW_WATERSURFACE)
								&& (duckFlag == PLAYER_NOT_DUCKING) )
				{
					CTMove__SelectWeaponByAIType(&CTurokMovement, AI_OBJECT_WEAPON_SEMIAUTOMATICPISTOL, FALSE) ;
					CTMove__ResetDrawnWeapons(&CTurokMovement) ;
				}
			}
			break ;


		case AI_OBJECT_PICKUP_AMMO_SHOTGUN:
		case AI_OBJECT_PICKUP_AMMO_SHOTGUNBOX:
			// if you have an empty shotgun, and its more powerful than current, use it
			if (CTurokMovement.ShotgunPool == 0)
				noammo = TRUE ;
			if (CInstanceHdr__TypeFlag(&pPickup->ah.ih) == AI_OBJECT_PICKUP_AMMO_SHOTGUN)
			{
				COnScreen__AddGameText(text_shotgunshells) ;
				CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_2, 0, NULL, &vPos, 0);
				CTMove__AddAmmo(&CTurokMovement, AI_OBJECT_WEAPON_AUTOMATICSHOTGUN, REFILL_SHOTGUN_SHELLS) ;
			}
			else
			{
				COnScreen__AddGameText(text_boxofshotgunshells) ;
				CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_2, 0, NULL, &vPos, 0);
				CTMove__AddAmmo(&CTurokMovement, AI_OBJECT_WEAPON_AUTOMATICSHOTGUN, REFILL_SHOTGUN_BOX) ;
			}
			if (noammo)
			{
				if (		(CTurokMovement.AutomaticShotgunFlag)
						&& (AUTOSHOTGUN > CurrentWeapon)
						&& (CTurokMovement.WaterFlag != PLAYER_BELOW_WATERSURFACE)
						&& (duckFlag == PLAYER_NOT_DUCKING) )
				{
					CTMove__SelectWeaponByAIType(&CTurokMovement, AI_OBJECT_WEAPON_AUTOMATICSHOTGUN, FALSE) ;
					CTMove__ResetDrawnWeapons(&CTurokMovement) ;
				}
				else if (		(CTurokMovement.RiotShotgunFlag)
								&& (SHOTGUN > CurrentWeapon)
								&& (CTurokMovement.WaterFlag != PLAYER_BELOW_WATERSURFACE)
								&& (duckFlag == PLAYER_NOT_DUCKING) )
				{
					CTMove__SelectWeaponByAIType(&CTurokMovement, AI_OBJECT_WEAPON_RIOTSHOTGUN, FALSE) ;
					CTMove__ResetDrawnWeapons(&CTurokMovement) ;
				}
			}
			break ;

		case AI_OBJECT_PICKUP_AMMO_EXPSHOTGUN:
		case AI_OBJECT_PICKUP_AMMO_EXPSHOTGUNBOX:
			// if you have an empty shotgun, and its more powerful than current, use it
			if (CTurokMovement.ExpShotgunPool == 0)
				noammo = TRUE ;
			if (CInstanceHdr__TypeFlag(&pPickup->ah.ih) == AI_OBJECT_PICKUP_AMMO_EXPSHOTGUN)
			{
				COnScreen__AddGameText(text_explosiveshells) ;
				CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_2, 0, NULL, &vPos, 0);
				CTMove__AddAmmo(&CTurokMovement, AI_OBJECT_WEAPON_AUTOMATICSHOTGUN, -REFILL_EXPSHOTGUN_SHELLS) ;
			}
			else
			{
				COnScreen__AddGameText(text_boxofexplosiveshells) ;
				CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_2, 0, NULL, &vPos, 0);
				CTMove__AddAmmo(&CTurokMovement, AI_OBJECT_WEAPON_AUTOMATICSHOTGUN, -REFILL_EXPSHOTGUN_BOX) ;
			}
			if (noammo)
			{
				if (		(CTurokMovement.AutomaticShotgunFlag)
						&& (POWEREDUP_AUTOSHOTGUN > CurrentWeapon)
						&& (CTurokMovement.WaterFlag != PLAYER_BELOW_WATERSURFACE)
						&& (duckFlag == PLAYER_NOT_DUCKING) )
				{
					CTMove__SelectWeaponByAIType(&CTurokMovement, AI_OBJECT_WEAPON_AUTOMATICSHOTGUN, FALSE) ;
					CTMove__ResetDrawnWeapons(&CTurokMovement) ;
				}
				else if (		(CTurokMovement.RiotShotgunFlag)
								&& (POWEREDUP_SHOTGUN > CurrentWeapon)
								&& (CTurokMovement.WaterFlag != PLAYER_BELOW_WATERSURFACE)
								&& (duckFlag == PLAYER_NOT_DUCKING) )
				{
					CTMove__SelectWeaponByAIType(&CTurokMovement, AI_OBJECT_WEAPON_RIOTSHOTGUN, FALSE) ;
					CTMove__ResetDrawnWeapons(&CTurokMovement) ;
				}
			}
			break ;

		case AI_OBJECT_PICKUP_AMMO_MINIGUN:
			// if you have an empty minigun, and its more powerful than current, use it
			if (CTurokMovement.MiniGunAmmo == 0)
				noammo = TRUE ;
			COnScreen__AddGameText(text_minigunammo) ;
			CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_1, 0, NULL, &vPos, 0);
			CTMove__AddAmmo(&CTurokMovement, AI_OBJECT_WEAPON_MINIGUN, REFILL_MINIGUN) ;

			if (		(noammo)
					&& (CTurokMovement.MiniGunFlag)
					&& (MINIGUN > CurrentWeapon)
					&& (CTurokMovement.WaterFlag != PLAYER_BELOW_WATERSURFACE)
					&& (duckFlag == PLAYER_NOT_DUCKING) )
			{
				CTMove__SelectWeaponByAIType(&CTurokMovement, AI_OBJECT_WEAPON_MINIGUN, FALSE) ;
				CTMove__ResetDrawnWeapons(&CTurokMovement) ;
			}
			break ;

		case AI_OBJECT_PICKUP_AMMO_GRENADES:
		case AI_OBJECT_PICKUP_AMMO_GRENADEBOX:
			// if you have an empty grende launcher, and its more powerful than current, use it
			if (CTurokMovement.GrenadeLauncherAmmo == 0)
				noammo = TRUE ;
			if (CInstanceHdr__TypeFlag(&pPickup->ah.ih) == AI_OBJECT_PICKUP_AMMO_GRENADES)
			{
				COnScreen__AddGameText(text_1grenade) ;
				CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_5, 0, NULL, &vPos, 0);
				CTMove__AddAmmo(&CTurokMovement, AI_OBJECT_WEAPON_GRENADE_LAUNCHER, REFILL_GRENADE) ;
			}
			else
			{
				COnScreen__AddGameText(text_boxofgrenades) ;
				CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_5, 0, NULL, &vPos, 0);
				CTMove__AddAmmo(&CTurokMovement, AI_OBJECT_WEAPON_GRENADE_LAUNCHER, REFILL_GRENADE_BOX) ;
			}
			if (		(noammo)
					&& (CTurokMovement.GrenadeLauncherFlag)
					&& (GRENADE > CurrentWeapon)
					&& (CTurokMovement.WaterFlag != PLAYER_BELOW_WATERSURFACE)
					&& (duckFlag == PLAYER_NOT_DUCKING) )
			{
				CTMove__SelectWeaponByAIType(&CTurokMovement, AI_OBJECT_WEAPON_GRENADE_LAUNCHER, FALSE) ;
				CTMove__ResetDrawnWeapons(&CTurokMovement) ;
			}
			break ;

		case AI_OBJECT_PICKUP_AMMO_TECHSMALL:
		case AI_OBJECT_PICKUP_AMMO_TECHLARGE:
			// if you have an pulse rifle, or alien gun, and its more powerful than current, use it
			if (CTurokMovement.EnergyPool == 0)
				noammo = TRUE ;
			if (CInstanceHdr__TypeFlag(&pPickup->ah.ih) == AI_OBJECT_PICKUP_AMMO_TECHSMALL)
			{
				COnScreen__AddGameText(text_energycell) ;
				CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_3, 0, NULL, &vPos, 0);
				CTMove__AddAmmo(&CTurokMovement, AI_OBJECT_WEAPON_TECHWEAPON1, REFILL_TECH_SMALL) ;
			}
			else
			{
				COnScreen__AddGameText(text_largeenergycell) ;
				CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_3, 0, NULL, &vPos , 0);
				CTMove__AddAmmo(&CTurokMovement, AI_OBJECT_WEAPON_TECHWEAPON1, REFILL_TECH_LARGE) ;
			}
			if (noammo)
			{
				if (		(CTurokMovement.TechWeapon1Flag)
						&& (INFANTRY > CurrentWeapon)
						&& (CTurokMovement.WaterFlag != PLAYER_BELOW_WATERSURFACE)
						&& (duckFlag == PLAYER_NOT_DUCKING) )
				{
					CTMove__SelectWeaponByAIType(&CTurokMovement, AI_OBJECT_WEAPON_TECHWEAPON1, FALSE) ;
					CTMove__ResetDrawnWeapons(&CTurokMovement) ;
				}
				else if (		(CTurokMovement.MachineGunFlag)
								&& (PULSERIFLE > CurrentWeapon)
								&& (CTurokMovement.WaterFlag != PLAYER_BELOW_WATERSURFACE)
								&& (duckFlag == PLAYER_NOT_DUCKING) )
				{
					CTMove__SelectWeaponByAIType(&CTurokMovement, AI_OBJECT_WEAPON_MACHINEGUN, FALSE) ;
					CTMove__ResetDrawnWeapons(&CTurokMovement) ;
				}
			}
			break ;

		case AI_OBJECT_PICKUP_AMMO_ROCKETS:
			// if you have an empty rocket launcher, and its more powerful than current, use it
			if (CTurokMovement.RocketAmmo == 0)
				noammo = TRUE ;
			COnScreen__AddGameText(text_4rockets) ;
			CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_7, 0, NULL, &vPos, 0);
			CTMove__AddAmmo(&CTurokMovement, AI_OBJECT_WEAPON_ROCKET, REFILL_ROCKET) ;

			if (		(noammo)
					&& (CTurokMovement.RocketFlag)
					&& (ROCKET > CurrentWeapon)
					&& (CTurokMovement.WaterFlag != PLAYER_BELOW_WATERSURFACE)
					&& (duckFlag == PLAYER_NOT_DUCKING) )
			{
				CTMove__SelectWeaponByAIType(&CTurokMovement, AI_OBJECT_WEAPON_ROCKET, FALSE) ;
				CTMove__ResetDrawnWeapons(&CTurokMovement) ;
			}
			break ;


		case AI_OBJECT_PICKUP_AMMO_TECHWEAPON2:
			// if you have an empty fusion cannon, and its more powerful than current, use it
			if (CTurokMovement.TechWeapon2Ammo == 0)
				noammo = TRUE ;
			COnScreen__AddGameText(text_fusioncharge) ;
			CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_3, 0, NULL, &vPos, 0);
			CTMove__AddAmmo(&CTurokMovement, AI_OBJECT_WEAPON_TECHWEAPON2, REFILL_TECH2) ;

			if (		(noammo)
					&& (CTurokMovement.TechWeapon2Flag)
					&& (FUSION > CurrentWeapon)
					&& (CTurokMovement.WaterFlag != PLAYER_BELOW_WATERSURFACE)
					&& (duckFlag == PLAYER_NOT_DUCKING) )
			{
				CTMove__SelectWeaponByAIType(&CTurokMovement, AI_OBJECT_WEAPON_TECHWEAPON2, FALSE) ;
				CTMove__ResetDrawnWeapons(&CTurokMovement) ;
			}
			break ;
	}


	if (AI_GetEA(pPickup)->m_dwTypeFlags & AI_TYPE_USEPRESSUREPLATES)
	{
		pInst = &pPickup->ah.ih ;
		AI_Event_Dispatcher(pInst, pInst, AI_MEVENT_PRESSUREPLATE, pInst->m_pEA->m_dwSpecies, vPos, ((CGameObjectInstance *)pPickup)->ah.ih.m_pEA->m_Id);
	}

	pPickup->m_wFlags |= SIMPLE_FLAG_GONE;

	if (pPickup->m_wFlags & SIMPLE_FLAG_DYNAMIC)
	{
		// dynamic, delete it
		CDynamicSimple__Delete((CDynamicSimple*) pPickup) ;
	}
	else
	{
		// instance, keep the state when terrain is reloaded
		CScene__SetPickupFlag(&GetApp()->m_Scene, pPickup, TRUE) ;
	}



	CEngineApp__DoColorFlash(GetApp(), 200, 200/4, 50, 50, 255) ;
}








/////////////////////////////////////////////////////////////////////////////
//
//
//
//		CSelectionList - pickup related functions
//
//
//
/////////////////////////////////////////////////////////////////////////////


// Only add pickup if Turok can pick it up!
void CSelectionList__AddPickup(CSelectionList *pThis, INT32 Pickup, INT16 Weight)
{
	if (CTMove__CanPickItUp(&CTurokMovement, Pickup))
		CSelectionList__AddItem(pThis, Pickup, Weight) ;
}



/*****************************************************************************
*
*	Function:		CSelectionList__AddHealthPickup()
*
******************************************************************************
*
*	Description:	Adds the appropriate health pickup to the current selection
*						list, depending on how Turok's health is doing
*
*	Inputs:			*pThis	-	Ptr to selection list
*						Weight	-	Selection weight of health id's
*
*	Outputs:			None
*
*****************************************************************************/
void CSelectionList__AddHealthPickup(CSelectionList *pThis, INT16 Weight)
{
	CGameObjectInstance *pTurok = CEngineApp__GetPlayer(GetApp()) ;

#if 0
	if (TUROK_Health(pTurok) > 60)
		CSelectionList__AddPickup(pThis, AI_OBJECT_PICKUP_HEALTH_2, Weight) ;
	else
	if (TUROK_Health(pTurok) > 40)
		CSelectionList__AddPickup(pThis, AI_OBJECT_PICKUP_HEALTH_10, Weight) ;
	else
		CSelectionList__AddPickup(pThis, AI_OBJECT_PICKUP_HEALTH_25, Weight) ;
#endif


	if (TUROK_Health(pTurok) > 20)
		CSelectionList__AddPickup(pThis, AI_OBJECT_PICKUP_HEALTH_2, Weight) ;
	else
		CSelectionList__AddPickup(pThis, AI_OBJECT_PICKUP_HEALTH_10, Weight) ;
}


/*****************************************************************************
*
*	Function:		CSelectionList__AddWeaponAmmoPickupsTurokIsHolding()
*
******************************************************************************
*
*	Description:	Adds all of the ammo pickups, associated with the weapons
*						that Turok is currently holding, to the current selection
*
*	Inputs:			*pThis	-	Ptr to selection list
*						Weight	-	Selection weight of ammo id's
*
*	Outputs:			None
*
*****************************************************************************/
void CSelectionList__AddWeaponAmmoPickupsTurokIsHolding(CSelectionList *pThis, INT16 Weight, BOOL GenerateArrows)
{
	BOOL	Campaigner = ((GetApp()->m_pBossActive) &&
							  (CGameObjectInstance__TypeFlag(GetApp()->m_pBossActive) == AI_OBJECT_CHARACTER_CAMPAIGNER_BOSS)) ;

	BOOL	TRex = ((GetApp()->m_pBossActive) &&
					  (CGameObjectInstance__TypeFlag(GetApp()->m_pBossActive) == AI_OBJECT_CHARACTER_TREX_BOSS)) ;

	// Fusion cannon
	if ((!Campaigner) && (!TRex) && (CTurokMovement.TechWeapon2Flag))
		CSelectionList__AddPickup(pThis, AI_OBJECT_PICKUP_AMMO_TECHWEAPON2, Weight) ;

	if ((!Campaigner) && (CTurokMovement.ShockwaveFlag))
		CSelectionList__AddPickup(pThis, AI_OBJECT_PICKUP_AMMO_SHOCKWAVE, Weight) ;

	// Weapons which are blocked by the campaigner
	if (!Campaigner)
	{
		if (CTurokMovement.GrenadeLauncherFlag)
		{
			CSelectionList__AddPickup(pThis, AI_OBJECT_PICKUP_AMMO_GRENADES, Weight*0.6) ;
			CSelectionList__AddPickup(pThis, AI_OBJECT_PICKUP_AMMO_GRENADEBOX, Weight*0.4) ;
		}

		if ((CTurokMovement.TechWeapon1Flag) || (CTurokMovement.MachineGunFlag))
		{
			CSelectionList__AddPickup(pThis, AI_OBJECT_PICKUP_AMMO_TECHSMALL, Weight*0.6) ;
			CSelectionList__AddPickup(pThis, AI_OBJECT_PICKUP_AMMO_TECHLARGE, Weight*0.4) ;
		}

		if (CTurokMovement.RocketFlag)
			CSelectionList__AddPickup(pThis, AI_OBJECT_PICKUP_AMMO_ROCKETS, Weight) ;
	}

	if ((CTurokMovement.TekBowFlag) && (GenerateArrows))
	{
//		CSelectionList__AddPickup(pThis, AI_OBJECT_PICKUP_AMMO_ARROWS,			  Weight*0.33) ;
		CSelectionList__AddPickup(pThis, AI_OBJECT_PICKUP_AMMO_EXPARROWSSMALL, Weight*0.33) ;

//		No 15 lot's of arrows
//		CSelectionList__AddPickup(pThis, AI_OBJECT_PICKUP_AMMO_EXPARROWSLARGE, Weight*0.33) ;
	}

	if ((CTurokMovement.SemiAutomaticPistolFlag) ||
		 (CTurokMovement.AssaultRifleFlag))
	{
		CSelectionList__AddPickup(pThis, AI_OBJECT_PICKUP_AMMO_BULLETCLIP, Weight*0.6) ;
		CSelectionList__AddPickup(pThis, AI_OBJECT_PICKUP_AMMO_BULLETBOX, Weight*0.4) ;
	}

	if ((CTurokMovement.AutomaticShotgunFlag) || (CTurokMovement.RiotShotgunFlag))
	{
//		CSelectionList__AddPickup(pThis, AI_OBJECT_PICKUP_AMMO_SHOTGUN, Weight*0.3) ;
//		CSelectionList__AddPickup(pThis, AI_OBJECT_PICKUP_AMMO_SHOTGUNBOX, Weight*0.2) ;
		CSelectionList__AddPickup(pThis, AI_OBJECT_PICKUP_AMMO_EXPSHOTGUN, Weight*0.3) ;
		CSelectionList__AddPickup(pThis, AI_OBJECT_PICKUP_AMMO_EXPSHOTGUNBOX, Weight*0.2) ;
	}

	if (CTurokMovement.MiniGunFlag)
		CSelectionList__AddPickup(pThis, AI_OBJECT_PICKUP_AMMO_MINIGUN, Weight) ;
}



/////////////////////////////////////////////////////////////////////////////
//
//
//
//		CPickupGenerator class
//
//
//
/////////////////////////////////////////////////////////////////////////////

/*****************************************************************************
*
*	Function:		CPickupGenerator__Construct()
*
******************************************************************************
*
*	Description:	Initialises pickup generator
*
*	Inputs:			*pThis	-	Ptr to pickup generator
*						*pvPos	-	Ptr to position
*
*	Outputs:			None
*
*****************************************************************************/
void CPickupGenerator__Construct(CPickupGenerator *pThis, CVector3 *pvPos)
{
	pThis->m_Time = 0 ;
	pThis->m_pPickup = NULL ;
	pThis->m_vPos = *pvPos ;
	pThis->m_pRegion = CScene__NearestRegion(&GetApp()->m_Scene, pvPos) ;

	// Now set y position to ground height
	if (pThis->m_pRegion)
		pThis->m_vPos.y = CGameRegion__GetGroundHeight(pThis->m_pRegion, pvPos->x, pvPos->z) ;
}


/*****************************************************************************
*
*	Function:		CPickupGenerator__Generate()
*
******************************************************************************
*
*	Description:	Produces specified pickup from generator
*
*	Inputs:			*pThis	-	Ptr to pickup generator
*						Type		-	Pickup id type (see "aistruct.h")
*						Time		-	Pickup life time
*
*	Outputs:			None
*
*****************************************************************************/
CDynamicSimple *CPickupGenerator__Generate(CPickupGenerator *pThis, INT32 Type, FLOAT Time)
{
	CDynamicSimple *pPickup ;
	CVector3			vPos ;

	if (!pThis->m_pRegion)
		return NULL ;

	pPickup = CScene__GeneratePickup(&GetApp()->m_Scene, 
												pThis->m_pRegion, 
												pThis->m_vPos, 
//												Type,Time) ;
												Type,-1) ;

	// Set pickup life time, and montitor flag
	if (pPickup)
	{
		pPickup->m_ppPickup = &pThis->m_pPickup ;

		// Do flash particle
		vPos = pThis->m_vPos ;
		vPos.y += 5*SCALING_FACTOR ;
		AI_SEvent_Generic260(&pPickup->s.ah.ih, vPos, 0) ;

		// Put sparkle on it
		pPickup->m_SparkleTime = SECONDS_TO_FRAMES(4) ;

		// Play sound
		if (!CCamera__InCinemaMode(&GetApp()->m_Camera))
			AI_SEvent_GeneralSound(&pPickup->s.ah.ih, pPickup->s.ah.ih.m_vPos, SOUND_GENERIC_3) ;
	}

	// Set generator pointer
	pThis->m_pPickup = pPickup ;

	return pPickup ;
}


/////////////////////////////////////////////////////////////////////////////
//
//
//
//		CPickupMonitor class
//
//
//
/////////////////////////////////////////////////////////////////////////////



/*****************************************************************************
*
*	Function:		CPickupMonitor__Construct()
*
******************************************************************************
*
*	Description:	Initialises pickup monitor
*
*	Inputs:			*pThis	-	Ptr to pickup generator
*
*	Outputs:			None
*
*****************************************************************************/
void CPickupMonitor__Construct(CPickupMonitor *pThis)
{
	pThis->m_Active = FALSE ;
	pThis->m_NoOfGenerators = 0 ;
}


/*****************************************************************************
*
*	Function:		CPickupMonitor__AddGenerators()
*
******************************************************************************
*
*	Description:	Adds generators to pickup monitor
*
*	Inputs:			*pThis					-	Ptr to pickup generator
*						NoOfGenerators			-	Total number of generators to montitor
*						*pvPosList				-	Ptr to array of generator positions
*
*	Outputs:			None
*
*****************************************************************************/
void CPickupMonitor__AddGenerators(CPickupMonitor *pThis, INT32 NoOfGenerators, CVector3 *pvPosList)
{
	INT32	i ;

	// Add each generator in turn
	for (i = 0 ; i < NoOfGenerators ; i++)
	{
		// Max generators reached?
		if (pThis->m_NoOfGenerators >= MAX_PICKUP_GENERATORS)
			return ;

		// Add that baby
		CPickupGenerator__Construct(&pThis->m_Generators[pThis->m_NoOfGenerators++], &pvPosList[i]) ;
	}
}




/*****************************************************************************
*
*	Function:		CPickupMonitor__UpdateGenerator()
*
******************************************************************************
*
*	Description:	Updates a generators
*
*	Inputs:			*pThis	-	Ptr to pickup generator
*					
*
*	Outputs:			None
*
*****************************************************************************/
void CPickupMonitor__UpdateGenerator(CPickupMonitor *pThis, CPickupGenerator *pGenerator)
{
	CSelectionList	List ;
	CDynamicSimple *pPickup = pGenerator->m_pPickup ;
	INT32				Ammo, Health, PercentageHealth, HealthWeight ;
	CGameRegion		*pRegion = pGenerator->m_pRegion ;
	BOOL				GenerateArrows = TRUE ;

	// Enough room to generate arrow?
	if (	 (pRegion)
		 && (pRegion->m_wFlags & REGFLAG_CEILING)
		 && ( (CGameRegion__GetCeilingHeight(pRegion, pGenerator->m_vPos.x, pGenerator->m_vPos.z) 
				 - pGenerator->m_vPos.y) < (10*SCALING_FACTOR) ) )
		GenerateArrows = FALSE ;

	// Delete pickup if it cannot be picked up!
	if ((pPickup) && 
		 (!CTMove__CanPickItUp(&CTurokMovement, CInstanceHdr__TypeFlag(&pPickup->s.ah.ih))))
	{
		pGenerator->m_Time = SECONDS_TO_FRAMES(3) ;
		pPickup->m_LifeTime = 1 ;
	}

	// Generate pickup only if there is no pickup already
	if (!pPickup)
	{
		// Update timer
		pGenerator->m_Time -= frame_increment ;
	
		// Generate pickup only if timer has ran out
		if (pGenerator->m_Time <= 0)
		{
			// Choose ammo pickup
			CSelectionList__Construct(&List) ;
			CSelectionList__AddWeaponAmmoPickupsTurokIsHolding(&List, 100, GenerateArrows) ;
			Ammo = CSelectionList__ChooseItem(&List) ;

			// Choose health pickup
			CSelectionList__Construct(&List) ;
			CSelectionList__AddHealthPickup(&List, 100) ;
			Health = CSelectionList__ChooseItem(&List) ;

			// The more health, bigger probabilty of choosing ammo and vice versa
			PercentageHealth = CEngineApp__GetPlayer(GetApp())->m_AI.m_Health ;
			if (PercentageHealth < 0)
				PercentageHealth = 0 ;
			else
			if (PercentageHealth > 100)
				PercentageHealth = 100 ;

			// If Health at 0,	Health = Max%,Ammo = 100-Max%
			// If Health at 100, Health = 0%,Ammo = 100%
			HealthWeight = MAX_PERCENTAGE_HEALTH - ((PercentageHealth * MAX_PERCENTAGE_HEALTH) / 100) ;
			if (HealthWeight == 0)
				Health = 0 ;

			// Add an ammo and a health pickup to list
			CSelectionList__Construct(&List) ;
			if (Ammo)
				CSelectionList__AddItem(&List, Ammo, 100-HealthWeight) ;

			if (Health)
				CSelectionList__AddItem(&List, Health, HealthWeight) ;

			// Generate that puppy!
			if (List.m_Entries)
			{
				CPickupGenerator__Generate(pGenerator, 
													CSelectionList__ChooseItem(&List),
													SECONDS_TO_FRAMES(10)) ;		// Time on screen

				// Reset time before another pickup should appear
				pGenerator->m_Time = RandomRangeFLOAT(SECONDS_TO_FRAMES((3)), SECONDS_TO_FRAMES((5))) ;
			}
		}
	}
}



/*****************************************************************************
*
*	Function:		CPickupMonitor__Update()
*
******************************************************************************
*
*	Description:	Updates all generators in montitor
*
*	Inputs:			*pThis	-	Ptr to pickup monitor
*
*	Outputs:			None
*
*****************************************************************************/
void CPickupMonitor__Update(CPickupMonitor *pThis)
{
	INT32	i ;

	if (pThis->m_Active)
	{
		for (i = 0 ; i < pThis->m_NoOfGenerators ; i++)
			CPickupMonitor__UpdateGenerator(pThis, &pThis->m_Generators[i]) ;
	}
}


