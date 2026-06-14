// onscrn.cpp
#include "cppu64.h"
#include "tengine.h"
#include "onscrn.h"
#include "tmove.h"
#include "textload.h"
#include "romstruc.h"
#include "boss.h"
#include "pickup.h"
#include "cartdir.h"
#include "sun.h"

#include "gfx16bit.h"


//#undef DISPLAY_FRAMERATE
//#undef DISPLAY_COORDINATES


/////////////////////////////////////////////////////////////////////////////
// Constants
/////////////////////////////////////////////////////////////////////////////

// health position defines
#define	HEALTH_XPOS								(16+32)
#define	HEALTH_YPOS								212
#define	HEALTH_START_YPOS						300

#define	LOWHEALTH_ALERT						15
#define	LOWARMOR_ALERT							5

// lives position defines
#define	LIVES_XPOS								(16)
#define	LIVES_START_XPOS						(LIVES_XPOS-64)
#define	LIVES_YPOS								16
#define	LIVES_TIMER								96

// tokens position defines
#define	TOKENS_XPOS								(320-40)
#define	TOKENS_START_XPOS						(TOKENS_XPOS+64)
#define	TOKENS_YPOS								16
#define	TOKENS_TIMER							96


// air meter position defines
#define	AIR_XPOS							((320/2)- (((C16BitGraphic *)AirBarOverlay)->m_Width/2))
#define	AIR_YPOS							16

#define	BOSS_XPOS				 		((320/2)- (((C16BitGraphic *)BossBarOverlay)->m_Width/2))
#define	BOSS_YPOS				 		16



// compass position defines
//#define	COMPASS_XPOS							((320/2)-(64/2))
//#define	COMPASS_YPOS							16


// world position defines
#define	POSITION_XPOS							226
#define	POSITION_YPOS							16

//  on screen weapon defines
#define	ONSCRN_WEAPONSELECTION_TIMER		40		// weapon selection timer before dropping off
#define	ONSCRN_WEAPONICON_SEL_X				29
#define	ONSCRN_WEAPONICON_SEL_Y				32
#define	ONSCRN_WEAPONICON_XOFF				-1//2
#define	ONSCRN_WEAPONICON_YOFF				72


// on screen run / walk defines
#define	RUNWALK_XPOS							240
#define	RUNWALK_YPOS							16
#define	RUNWALK_START_YPOS					-96
#define	ONSCRN_RUNWALKSELECTION_TIMER		30		// run / walk selection timer before dropping off



// numeral size & spacing
#define	ONSCRN_NUMERAL_WIDTH		12
#define	ONSCRN_NUMERAL_HEIGHT	12
#define	ONSCRN_NUMERAL_SPACER	-3


#if (ONSCRN_NUMERAL_WIDTH & 0x3)
#define ONSCRN_ALIGNEDWIDTH ((ONSCRN_NUMERAL_WIDTH/4*4) + 4)
#else
#define ONSCRN_ALIGNEDWIDTH ONSCRN_NUMERAL_WIDTH
#endif


/////////////////////////////////////////////////////////////////////////////
// Text
/////////////////////////////////////////////////////////////////////////////
#ifdef ENGLISH
// ---------------------- English text ---------------------------
static char	text_low_health[] = {"low health"} ;
static char	text_low_armor[] = {"low armor"} ;
#endif

#ifdef GERMAN
// ---------------------- German text ---------------------------
static char	text_low_health[] = {"gesundheit schwach"} ;
static char	text_low_armor[] = {"r/stung shwach"} ;		//u.
#endif

#ifdef KANJI
// ---------------------- Japanese text ---------------------------
static char	text_low_health[] = {0x21, 0x01, 0x19, 0xA8, 0x92, -1};
static char	text_low_armor[] = {0x00, 0x41, 0x1B, 0x41, 0x00, 0x21, 0x41, 0x13, -1};
#endif

/////////////////////////////////////////////////////////////////////////////
// Data
/////////////////////////////////////////////////////////////////////////////
typedef struct s_WeaponInfo
{
	int	XOff, YOff ;
	void *Icon ;
} t_WeaponInfo ;

// regular ammo icons
static t_WeaponInfo pAmmoIcons[] =
{
	0, 0,  NULL,						// 1 Knife
	0, 0,  NULL,						// 2 Tomahawk
	0, -1, AmmoArrowOverlay,		// 3 TekBow
	4, 0,  AmmoBulletOverlay,		// 4 SemiAutomaticPistol
	0, 0,  AmmoShellOverlay,		// 5 RiotShotgun
	0, 0,  AmmoShellOverlay,		// 6 AutomaticShotgun
	4, 0,  AmmoBulletOverlay,		// 7 AssaultRifle
	0, 0,  AmmoEnergyOverlay,		// 8 MachinGun
	4, -2, AmmoMinigunOverlay,		// 9 MiniGun
	0, 0,  AmmoGrenadeOverlay,		// 10 GrenadeLauncher
	0, 0,  AmmoEnergyOverlay,		// 11 TechWeapon1
	0, -1, AmmoRocketOverlay,		// 12	Rocket Launcher
	0, 0,  AmmoEnergyOverlay,		// 13 Shockwave
	4, 0,  AmmoKamfOverlay,			// 14 TechWeapon2
	0, 0,  AmmoEnergyOverlay,		// 15 Chronoscepter
} ;

// explosive ammo icons
static t_WeaponInfo pExplosiveAmmoIcons[] =
{
	0, 0,  NULL,						// 1 Knife
	0, 0,  NULL,						// 2 Tomahawk
	0, -1, AmmoExpArrowOverlay,	// 3 TekBow
	4, 0,  AmmoBulletOverlay,		// 4 SemiAutomaticPistol
	0, 0,  AmmoExpShellOverlay,	// 5 RiotShotgun
	0, 0,  AmmoExpShellOverlay,  	// 6 AutomaticShotgun
	4, 0,  AmmoBulletOverlay,		// 7 AssaultRifle
	0, 0,  AmmoEnergyOverlay,		// 8 MachinGun
	4, -2, AmmoMinigunOverlay,		// 9 MiniGun
	0, 0,  AmmoGrenadeOverlay,		// 10 GrenadeLauncher
	0, 0,  AmmoEnergyOverlay,		// 11 TechWeapon1
	0, -1, AmmoRocketOverlay,		// 12	Rocket Launcher
	0, 0,	 AmmoEnergyOverlay,		// 13 Shockwave
	4, 0,	 AmmoKamfOverlay,			// 14 TechWeapon2
	0, 0,  AmmoEnergyOverlay,		// 15 Chronoscepter
} ;



static t_WeaponInfo pWeaponIcons[] =
{
	10, 6, KnifeWeapon,			// 1 Knife
	0, 0, KnifeWeapon,			// 2 Tomahawk
	0, 2, TekBowWeapon,			// 3 TekBow
	16, 4, PistolWeapon,			// 4 SemiAutomaticPistol
	0, 4, ShotgunWeapon,			// 5 RiotShotgun
	0, 4, AutoShotgunWeapon,	// 6 AutomaticShotgun
	0, 2, RifleWeapon,			// 7 AssaultRifle
	0, 1, LongHunterWeapon,		// 8 MachinGun
	0, 0, MinigunWeapon,			// 9 MiniGun
	0, 0, GrenadeWeapon,			// 10 GrenadeLauncher
	0, 2, InfantryWeapon,		// 11 TechWeapon1
	0, 1, RocketWeapon,			// 12 Rocket Launcher
	0, 0, ShockWeapon,			// 13 Shockwave
	0, 3, FusionWeapon,			// 14 TechWeapon2
	0, 2, CronosceptorWeapon,		// 15 Chronoscepter
};


// Pointers to digit data
static void *pNumbers[10]=
{
	Font0,
	Font1,
	Font2,
	Font3,
	Font4,
	Font5,
	Font6,
	Font7,
	Font8,
	Font9,
};

// Pointers to digit data 4bit
static void *pNumbers4Bit[10]=
{
	Font0_4,
	Font1_4,
	Font2_4,
	Font3_4,
	Font4_4,
	Font5_4,
	Font6_4,
	Font7_4,
	Font8_4,
	Font9_4,
};

// Pointers to font data (4bit)
void *pFont[26]=
{
	FontA,
	FontB,
	FontC,
	FontD,
	FontE,
	FontF,
	FontG,
	FontH,
	FontI,
	FontJ,
	FontK,
	FontL,
	FontM,
	FontN,
	FontO,
	FontP,
	FontQ,
	FontR,
	FontS,
	FontT,
	FontU,
	FontV,
	FontW,
	FontX,
	FontY,
	FontZ,
};

#ifdef KANJI
// Pointers to kanji font data (4bit)
static void *pKanjiFont[]=
{
	Kanji00,
	Kanji01,
	Kanji02,
	Kanji03,
	Kanji04,
	Kanji05,
	Kanji06,
	Kanji07,
	Kanji08,
	Kanji09,
	Kanji0a,
	Kanji0b,
	Kanji0c,
	Kanji0d,
	Kanji0e,
	Kanji0f,
	Kanji10,
	Kanji11,
	Kanji12,
	Kanji13,
	Kanji14,
	Kanji15,
	Kanji16,
	Kanji17,
	Kanji18,
	Kanji19,
	Kanji1a,
	Kanji1b,
	Kanji1c,
	Kanji1d,
	Kanji1e,
	Kanji1f,
	Kanji20,
	Kanji21,
	Kanji22,
	Kanji23,
	Kanji24,
	Kanji25,
	Kanji26,
	Kanji27,
	Kanji28,
	Kanji29,
	Kanji2a,
	Kanji2b,
	Kanji2c,
	Kanji2d,
	Kanji2e,
	Kanji2f,
	Kanji30,
	Kanji31,
	Kanji32,
	Kanji33,
	Kanji34,
	Kanji35,
	Kanji36,
	Kanji37,
	Kanji38,
	Kanji39,
	Kanji3a,
	Kanji3b,
	Kanji3c,
	Kanji3d,
	Kanji3e,
	Kanji3f,
	Kanji40,
	Kanji41,
	Kanji42,
	Kanji43,
	Kanji44,
	Kanji45,
	Kanji46,
	Kanji47,
	Kanji48,
	Kanji49,
	Kanji4a,
	Kanji4b,
	Kanji4c,
	Kanji4d,
	Kanji4e,
	Kanji4f,
	Kanji50,
	Kanji51,
	Kanji52,
	Kanji53,
	Kanji54,
	Kanji55,
	Kanji56,
	Kanji57,
	Kanji58,
	Kanji59,
	Kanji5a,
	Kanji5b,
	Kanji5c,
	Kanji5d,
	Kanji5e,
	Kanji5f,
	Kanji60,
	Kanji61,
	Kanji62,
	Kanji63,
	Kanji64,
	Kanji65,
	Kanji66,
	Kanji67,
	Kanji68,
	Kanji69,
	Kanji6a,
	Kanji6b,
	Kanji6c,
	Kanji6d,
	Kanji6e,
	Kanji6f,
	Kanji70,
	Kanji71,
	Kanji72,
	Kanji73,
	Kanji74,
	Kanji75,
	Kanji76,
	Kanji77,
	Kanji78,
	Kanji79,
	Kanji7a,
	Kanji7b,
	Kanji7c,
	Kanji7d,
	Kanji7e,
	Kanji7f,
	Kanji80,
	Kanji81,
	Kanji82,
	Kanji83,
	Kanji84,
	Kanji85,
	Kanji86,
	Kanji87,
	Kanji88,
	Kanji89,
	Kanji8a,
	Kanji8b,
	Kanji8c,
	Kanji8d,
	Kanji8e,
	Kanji8f,
	Kanji90,
	Kanji91,
	Kanji92,
	Kanji93,
	Kanji94,
	Kanji95,
	Kanji96,
	Kanji97,
	Kanji98,
	Kanji99,
	Kanji9a,
	Kanji9b,
	Kanji9c,
	Kanji9d,
	Kanji9e,
	Kanji9f,
	Kanjia0,
	Kanjia1,
	Kanjia2,
	Kanjia3,
	Kanjia4,
	Kanjia5,
	Kanjia6,
	Kanjia7,
	Kanjia8,
	Kanjia9,
	Kanjiaa,
	Kanjiab,
	Kanjiac,
	Kanjiad,
	Kanjiae,
	Kanjiaf,
	Kanjib0,
	Kanjib1,
	Kanjib2,
	Kanjib3,
	Kanjib4,
	Kanjib5,
	Kanjib6,
	Kanjib7,
	Kanjib8,
	Kanjib9,
	Kanjiba,
	Kanjibb,
	Kanjibc,
	Kanjibd,
	Kanjibe,
	Kanjibf,
	Kanjic0,
	Kanjic1,
	Kanjic2,
	Kanjic3,
	Kanjic4,
	Kanjic5,
	Kanjic6,
	Kanjic7,
	Kanjic8,
	Kanjic9,
	Kanjica,
	Kanjicb,
	Kanjicc,
	Kanjicd,
	Kanjice,
	Kanjicf,
	Kanjid0,
	Kanjid1,
	Kanjid2,
	Kanjid3,
	Kanjid4,
	Kanjid5,
	Kanjid6,
	Kanjid7,
	Kanjid8,
	Kanjid9,
	Kanjida,
	Kanjidb,
	Kanjidc,
	Kanjidd,
	Kanjide,
	Kanjidf,
	Kanjie0,
	Kanjie1,
	Kanjie2,
	Kanjie3,
	Kanjie4,
	Kanjie5,
	Kanjie6,
	Kanjie7,
	Kanjie8,
	Kanjie9,
	Kanjiea,
	0,
	Kanjiec
};
#endif

int	FontType ;

// Pointers to walk & run mode icons
static void *pRunWalkIcons[2]=
{
	// 0 = walk mode icon
	// 1 = run mode icon
	WalkOverlay,
	RunOverlay
};



// Ammo overlay scripts
DEFINE_OVERLAY_SCRIPT(AmmoInitOverlayScript)
	OVERLAY_SCRIPT_ENTRY_SET_POS(20+32+68,240+96)
END_OVERLAY_SCRIPT

DEFINE_OVERLAY_SCRIPT(AmmoOnOverlayScript)
	OVERLAY_SCRIPT_ENTRY_ZOOM(20+32+68,192+20,2)
END_OVERLAY_SCRIPT

DEFINE_OVERLAY_SCRIPT(AmmoDropOffOverlayScript)
	OVERLAY_SCRIPT_ENTRY_DROP_OFF(6)
END_OVERLAY_SCRIPT



// Health overlay scripts
DEFINE_OVERLAY_SCRIPT(HealthInitOverlayScript)
	OVERLAY_SCRIPT_ENTRY_ZOOM(HEALTH_XPOS, HEALTH_YPOS, 2)
END_OVERLAY_SCRIPT

DEFINE_OVERLAY_SCRIPT(SwitchHealthArmourOverlayScript)
	OVERLAY_SCRIPT_ENTRY_ZOOM(HEALTH_XPOS, HEALTH_YPOS+32, 2)
END_OVERLAY_SCRIPT


// Lives overlay scripts
DEFINE_OVERLAY_SCRIPT(LivesInitOverlayScript)
	OVERLAY_SCRIPT_ENTRY_SET_POS(LIVES_START_XPOS, LIVES_YPOS)
END_OVERLAY_SCRIPT

DEFINE_OVERLAY_SCRIPT(LivesChangeOverlayScript)
	OVERLAY_SCRIPT_ENTRY_ZOOM(LIVES_XPOS, LIVES_YPOS,2)
	OVERLAY_SCRIPT_ENTRY_WAIT(LIVES_TIMER)
	OVERLAY_SCRIPT_ENTRY_HORZDROP_OFF(-4)
END_OVERLAY_SCRIPT

// Tokens overlay scripts
DEFINE_OVERLAY_SCRIPT(TokensInitOverlayScript)
	OVERLAY_SCRIPT_ENTRY_SET_POS(TOKENS_START_XPOS, TOKENS_YPOS)
END_OVERLAY_SCRIPT

DEFINE_OVERLAY_SCRIPT(TokensChangeOverlayScript)
	OVERLAY_SCRIPT_ENTRY_ZOOM(TOKENS_XPOS, TOKENS_YPOS,2)
	OVERLAY_SCRIPT_ENTRY_WAIT(TOKENS_TIMER)
	OVERLAY_SCRIPT_ENTRY_HORZDROP_OFF(4)
END_OVERLAY_SCRIPT




// Weapon Selection overlay scripts
DEFINE_OVERLAY_SCRIPT(WeaponInitOverlayScript)
	OVERLAY_SCRIPT_ENTRY_SET_POS(ONSCRN_WEAPONICON_SEL_X - 90,24)
END_OVERLAY_SCRIPT

DEFINE_OVERLAY_SCRIPT(WeaponOnOverlayScript)
	OVERLAY_SCRIPT_ENTRY_ZOOM(ONSCRN_WEAPONICON_SEL_X,ONSCRN_WEAPONICON_SEL_Y,2)
	OVERLAY_SCRIPT_ENTRY_WAIT(ONSCRN_WEAPONSELECTION_TIMER)
	OVERLAY_SCRIPT_ENTRY_HORZDROP_OFF(-4)
END_OVERLAY_SCRIPT

// Run / Walk Toggle overlay scripts
DEFINE_OVERLAY_SCRIPT(RunWalkInitOverlayScript)
	OVERLAY_SCRIPT_ENTRY_SET_POS(RUNWALK_XPOS, RUNWALK_START_YPOS)
END_OVERLAY_SCRIPT

DEFINE_OVERLAY_SCRIPT(RunWalkOnOverlayScript)
	OVERLAY_SCRIPT_ENTRY_ZOOM(RUNWALK_XPOS, RUNWALK_YPOS, 2)
	OVERLAY_SCRIPT_ENTRY_WAIT(ONSCRN_RUNWALKSELECTION_TIMER)
	OVERLAY_SCRIPT_ENTRY_DROP_OFF(-4)
END_OVERLAY_SCRIPT



CBarrel		WeaponSelectBarrel ;



/////////////////////////////////////////////////////////////////////////////
// Internal Prototypes
/////////////////////////////////////////////////////////////////////////////
//void COnScreen__DrawCompassGraphic(Gfx **ppDLP, C16BitGraphic *Grid, INT16 XPos, INT16 YPos) ;
void COnScreen__DrawBarGraphic(Gfx **ppDLP, s16 X, s16 Y, float pos, float opacity) ;

void COnScreen__Draw16BitScrollTexture(Gfx **ppDLP,
											 void *pTexture,
											 void *pOpacity,
											 int XPos, int YPos,
											 int TextureWidth, int TextureHeight,
											 int XOff, int YOff) ;

void COnScreen__Draw16BitClipTexture(Gfx **ppDLP,
											 void *pTexture,
											 void *pOpacity,
											 int XPos, int YPos,
											 int TextureWidth, int TextureHeight,
											 int XSize, int YSize) ;


void COnScreen__InitFontDraw(Gfx **ppDLP) ;
void COnScreen__DrawFontTexture(Gfx **ppDLP,
										void *pTexture,
										float XPos, float YPos,
										float XScale, float YScale) ;
void COnScreen__WriteText(Gfx **ppDLP, char *String, float nX, float nY, float XScale, float YScale);

/////////////////////////////////////////////////////////////////////////////
// Functions
/////////////////////////////////////////////////////////////////////////////

void COnScreen__Construct(COnScreen *pThis)
{
	// reset vars
	pThis->m_Ammo = 0 ;

	pThis->m_Lives = 0 ;
	pThis->m_Tokens = 0 ;

	pThis->m_Health = 0;
	pThis->m_Armor = 0;
	pThis->m_HealthArmourIcon = ONSCRN_HEALTH_ICON;

	pThis->m_HealthAlert = FALSE ;
	pThis->m_ArmorAlert = FALSE ;

//	pThis->m_CompassPos = 0;
//	pThis->m_CompassAccel = 0;

	pThis->m_SelectPosition = 0;
	pThis->m_BarrelDirection = 0;

	pThis->m_WalkRunIcon = ONSCRN_RUN_ICON ;

	pThis->m_FlashDraw = -1;
	pThis->m_FlashTimer = 0.0;


	COverlay__Construct(&pThis->m_AmmoOverlay, 24, 240+96) ;
	COverlay__RunScript(&pThis->m_AmmoOverlay, AmmoInitOverlayScript) ;

	COverlay__Construct(&pThis->m_HealthOverlay, HEALTH_XPOS, HEALTH_START_YPOS) ;
	COverlay__RunScript(&pThis->m_HealthOverlay, HealthInitOverlayScript) ;

	COverlay__Construct(&pThis->m_LivesOverlay, LIVES_START_XPOS, LIVES_YPOS) ;
	COverlay__RunScript(&pThis->m_LivesOverlay, LivesInitOverlayScript) ;
	COverlay__Construct(&pThis->m_TokensOverlay, TOKENS_START_XPOS, TOKENS_YPOS) ;
	COverlay__RunScript(&pThis->m_TokensOverlay, TokensInitOverlayScript) ;


	COverlay__Construct(&pThis->m_WeaponOverlay, -60, 24) ;
	COverlay__RunScript(&pThis->m_WeaponOverlay, WeaponInitOverlayScript) ;
	CBarrel__Construct(&WeaponSelectBarrel, 1,
							0, 0, 0, 48,
							4, 8) ;

	COverlay__Construct(&pThis->m_RunWalkOverlay, RUNWALK_XPOS, RUNWALK_START_YPOS) ;
	COverlay__RunScript(&pThis->m_RunWalkOverlay, RunWalkInitOverlayScript) ;

	COnScreen__Reset(pThis);
}

void COnScreen__Reset(COnScreen *pThis)
{
	pThis->m_AirMode = AIRMODE_NULL ;
	pThis->m_AirOpacity = 0 ;

	// Only reset boss bar if there isn't a boss
	if ((!GetApp()->m_BossLevel) || (!GetApp()->m_pBossActive))
	{
		pThis->m_BossBarMode = BOSSBARMODE_NULL ;
		pThis->m_BossBarOpacity = 0 ;
		pThis->m_pBoss = NULL ;
	}

	COnScreen__InitializeGameText() ;
}

void COnScreen__Update(COnScreen *pThis)
{
	// Update ammo icons movement
	COverlay__Update(&pThis->m_AmmoOverlay) ;

	// Update health icons movement
	COverlay__Update(&pThis->m_HealthOverlay) ;

	// Update lives and tokens icons movement
	COverlay__Update(&pThis->m_LivesOverlay) ;
	COverlay__Update(&pThis->m_TokensOverlay) ;

	// Update weapon icons movement
	COverlay__Update(&pThis->m_WeaponOverlay) ;

	// Update run / walk icon movement
	COverlay__Update(&pThis->m_RunWalkOverlay);
}



//--------------------------------------------------------------------------------------
// Draw ONSCREEN grafix
//--------------------------------------------------------------------------------------
char	textbuffer[8] ;

void COnScreen__Draw(COnScreen *pThis, Gfx **ppDLP)
{
	INT16				nRunWalkMode;
	INT32				Offset ;
	C16BitGraphic	*Grid ;
	int				xoff, yoff ;
	u32				time ;
	static float 	oxygen,
						health = 0.0;

#ifndef SHIP_IT
//	char			floatbuff[32] ;

	int				i;
	float				slowest, value;
	char				*string;

	CGameObjectInstance *pTurok = CEngineApp__GetPlayer(GetApp());

	if (!pTurok)
		return;
#endif



	//--------------------------------------------------------------------------------------
	// display enemy modify stuff
/*
	if ((GetApp()->m_dwCheatFlags & CHEATFLAG_MODIFYENEMY) && (CTurokMovement.pModifyEnemy))
	{
		COnScreen__InitFontDraw(ppDLP) ;

		sprintf(floatbuff, "x %.2f", AI_GetDyn(CTurokMovement.pModifyEnemy)->m_vLeashCoor.x / SCALING_FACTOR) ;
		COnScreen__DrawText(ppDLP, floatbuff, 64, 20, 255, FALSE, TRUE);
		sprintf(floatbuff, "z %.2f", AI_GetDyn(CTurokMovement.pModifyEnemy)->m_vLeashCoor.z / SCALING_FACTOR) ;
		COnScreen__DrawText(ppDLP, floatbuff, 64, 20+16, 255, FALSE, TRUE);
		sprintf(floatbuff, "yrot %.2f", ANGLE_RTOD(CTurokMovement.pModifyEnemy->m_RotY)) ;
		COnScreen__DrawText(ppDLP, floatbuff, 64, 20+32, 255, FALSE, TRUE);
	}
*/



	// Prepare for drawing texture graphics
	COnScreen__Init16BitDraw(ppDLP, GetApp()->m_Options.m_Opacity) ;


	if (GetApp()->m_Options.m_Opacity)
	{

		//--------------------------------------------------------------------------------------
		// Display ammo info
		if ((pThis->m_SelectPosition != (u8) -1) && COverlay__OnScreen(&pThis->m_AmmoOverlay))
		{
			// Calculate 1/2 of pixel width
			if (pThis->m_ExplosiveAmmo)
			{
				Grid = (C16BitGraphic *)(pExplosiveAmmoIcons[pThis->m_SelectPosition].Icon) ;
				xoff = pExplosiveAmmoIcons[pThis->m_SelectPosition].XOff ;
				yoff = pExplosiveAmmoIcons[pThis->m_SelectPosition].YOff ;
			}
			else
			{
				Grid = (C16BitGraphic *)(pAmmoIcons[pThis->m_SelectPosition].Icon) ;
				xoff = pAmmoIcons[pThis->m_SelectPosition].XOff ;

				yoff = pAmmoIcons[pThis->m_SelectPosition].YOff ;
			}

			if (Grid)
			{
				Offset = 32 ;

				// Draw ammo overlay
				COnScreen__Draw16BitGraphic(ppDLP,
													Grid,
													pThis->m_AmmoOverlay.m_X-Offset+xoff,
													pThis->m_AmmoOverlay.m_Y-8+yoff) ;

				// display rock piece for numbers
				COnScreen__Draw16BitGraphic(ppDLP,
													(C16BitGraphic *)RokOverlay,
													pThis->m_AmmoOverlay.m_X-Offset+18,
													pThis->m_AmmoOverlay.m_Y-8) ;


					// Display ammo numbers
				COnScreen__DrawDigits(ppDLP,
											 pThis->m_Ammo,
											 (int)pThis->m_AmmoOverlay.m_X-12,
											 (int)pThis->m_AmmoOverlay.m_Y-4);
			}
		}

		//--------------------------------------------------------------------------------------
		// Display health info
		if (pThis->m_HealthArmourIcon == ONSCRN_HEALTH_ICON)
			Grid = (C16BitGraphic *)HealthOverlay ;
		else
			Grid = (C16BitGraphic *)ArmourOverlay ;
		Offset = 30 ;

		// Draw health overlay
		COnScreen__Draw16BitGraphic(ppDLP,
											Grid,
											pThis->m_HealthOverlay.m_X-Offset,
											pThis->m_HealthOverlay.m_Y-8) ;
		// draw rock
		COnScreen__Draw16BitGraphic(ppDLP,
											(C16BitGraphic *)RokOverlay,
											pThis->m_HealthOverlay.m_X-Offset+21,
											pThis->m_HealthOverlay.m_Y-8) ;

		// Display numbers - do we need to flash them ?
		#define	ONSCRN_FLASH_TIMER_HEALTH		0.3
		#define	ONSCRN_FLASH_TIMER_ARMOUR		0.3

		if (pThis->m_HealthArmourIcon == ONSCRN_HEALTH_ICON)
		{
			if ((pThis->m_Health <= LOWHEALTH_ALERT) && (pThis->m_HealthAlert))
			{
				//osSyncPrintf("health:%d\n", pThis->m_Health) ;
				COnScreen__AddGameText(text_low_health) ;
				pThis->m_HealthAlert = FALSE ;
			}

			// flash health if <=10
			if (pThis->m_Health > 10)
			{
				pThis->m_FlashDraw = -1;
				pThis->m_FlashTimer = 0.0;
			}
			else
			{
				pThis->m_FlashTimer += frame_increment*(1.0/FRAME_FPS);
				if (pThis->m_FlashTimer >= ONSCRN_FLASH_TIMER_HEALTH)
				{
					pThis->m_FlashTimer -= ONSCRN_FLASH_TIMER_HEALTH;
					pThis->m_FlashDraw = ~pThis->m_FlashDraw;
				}
			}
			if (pThis->m_FlashDraw)
				COnScreen__DrawDigits(ppDLP,
											 pThis->m_Health,
											 (int)pThis->m_HealthOverlay.m_X-6,
											 (int)pThis->m_HealthOverlay.m_Y-4) ;

		}
		else
		{
			if ((pThis->m_Armor <= LOWARMOR_ALERT) && (pThis->m_ArmorAlert))
			{
				COnScreen__AddGameText(text_low_armor) ;
				pThis->m_ArmorAlert = FALSE ;
			}

			//flash armor if <=5
			if (pThis->m_Armor > 5)
			{
				pThis->m_FlashDraw = -1;
				pThis->m_FlashTimer = 0.0;
			}
			else
			{
				pThis->m_FlashTimer += frame_increment*(1.0/FRAME_FPS);
				if (pThis->m_FlashTimer >= ONSCRN_FLASH_TIMER_ARMOUR)
				{
					pThis->m_FlashTimer -= ONSCRN_FLASH_TIMER_ARMOUR;
					pThis->m_FlashDraw = ~pThis->m_FlashDraw;
				}
			}
			if (pThis->m_FlashDraw)
				COnScreen__DrawDigits(ppDLP,
											 pThis->m_Armor,
											 (int)pThis->m_HealthOverlay.m_X-6,
											 (int)pThis->m_HealthOverlay.m_Y-4) ;

		}

		//--------------------------------------------------------------------------------------
		// display tokens info
		if (COverlay__OnScreen(&pThis->m_TokensOverlay))
		{
			xoff = pThis->m_TokensOverlay.m_X ;
			yoff = pThis->m_TokensOverlay.m_Y ;

			// draw icon overlay
			COnScreen__Draw16BitGraphic(ppDLP,
												(C16BitGraphic *)LifeForceOverlay,
												xoff,
												yoff);

			// display numbers
			Offset = (pThis->m_Tokens<10) ? -8 : -8-(ONSCRN_ALIGNEDWIDTH+ONSCRN_NUMERAL_SPACER);
			COnScreen__DrawDigits(ppDLP,
										 pThis->m_Tokens,
										 xoff + Offset,
										 yoff + 8);
		}

		//--------------------------------------------------------------------------------------
		// display lives info
		if (COverlay__OnScreen(&pThis->m_LivesOverlay))
		{
			xoff = pThis->m_LivesOverlay.m_X ;
			yoff = pThis->m_LivesOverlay.m_Y ;

			// draw icon overlay
			COnScreen__Draw16BitGraphic(ppDLP,
												(C16BitGraphic *)TurokHeadOverlay,
												xoff,
												yoff) ;

			// display numbers
			COnScreen__DrawDigits(ppDLP,
										 pThis->m_Lives,
										 xoff+30,
										 yoff+32) ;
		}

//#ifdef DISPLAY_FRAMERATE
#ifndef SHIP_IT
	if (GetApp()->m_bDebug)
	{
		// increase time, if not finished
		if (GetApp()->m_bPause == FALSE)
			GetApp()->m_LevelTime += frame_increment ;

		COnScreen__InitFontDraw(ppDLP) ;
		COnScreen__PrepareSmallFont(ppDLP) ;

		COnScreen__SetFontColor(ppDLP, 255, 55, 255, 255, 255, 255) ;
		COnScreen__DrawText(ppDLP, "time", 80, 22, 255, TRUE, TRUE) ;

		COnScreen__SetFontScale(1.0, 2.0) ;
		time = GetApp()->m_LevelTime / FRAME_FPS ;
		i = 192+(63 * cos((float)game_frame_number/3)) ;
		COnScreen__SetFontColor(ppDLP, i, i, i, i, i, i) ;
		sprintf(textbuffer, "%02d:%02d", time/60, time%60) ;
		COnScreen__DrawText(ppDLP, textbuffer, 80, 32, 255, TRUE, TRUE) ;

		COnScreen__Init16BitDraw(ppDLP, GetApp()->m_Options.m_Opacity) ;

//		if (!(GetApp()->m_dwCheatFlags & CHEATFLAG_MODIFYENEMY))
		{
			//--------------------------------------------------------------------------------------
			COnScreen__DrawDigits(ppDLP,
										 GetApp()->m_Scene.m_nActiveAnimInstances - 1,
										 60,
										 POSITION_YPOS + 40 + (0*16));

			ASSERT(frame_increment) ;
			COnScreen__DrawDigits(ppDLP,
										 (int) (FRAME_FPS/frame_increment),
										 60,
										 POSITION_YPOS + 40 + (1*16));

			COnScreen__DrawDigits(ppDLP,
										 (int) rsp_graphics_time,
										 60,
										 POSITION_YPOS + 50 + (2*16));

			COnScreen__DrawDigits(ppDLP,
										 (int) (rsp_audio_time*FRAME_INC_TO_REFRESHES(frame_increment)),
										 60,
										 POSITION_YPOS + 50 + (3*16));

			COnScreen__DrawDigits(ppDLP,
										 (int) rsp_total_time,
										 60,
										 POSITION_YPOS + 60 + (4*16));

			COnScreen__DrawDigits(ppDLP,
										 (int) rdp_time,
										 60,
										 POSITION_YPOS + 60 + (5*16));

			COnScreen__DrawDigits(ppDLP,
										 (int) cpu_time,
										 60,
										 POSITION_YPOS + 60 + (6*16));

			COnScreen__InitFontDraw(ppDLP) ;
			COnScreen__PrepareLargeFont(ppDLP);
			COnScreen__SetFontScale(0.8, 0.7) ;
			COnScreen__SetFontColor(ppDLP,
											255, 255, 255,
											255, 255, 255) ;

			COnScreen__DrawText(ppDLP, "ais", 25, POSITION_YPOS + 40 + (0*16), 255, FALSE, FALSE);
			COnScreen__DrawText(ppDLP, "fps", 25, POSITION_YPOS + 40 + (1*16), 255, FALSE, FALSE);

			COnScreen__DrawText(ppDLP, "spg", 25, POSITION_YPOS + 50 + (2*16), 255, FALSE, FALSE);
			COnScreen__DrawText(ppDLP, "spa", 25, POSITION_YPOS + 50 + (3*16), 255, FALSE, FALSE);

			slowest = max(rsp_total_time, max(rdp_time, cpu_time));
			for (i=0; i<3; i++)
			{
				switch (i)
				{
					case 0:
						string = "rsp";
						value = rsp_total_time;
						break;
					case 1:
						string = "rdp";
						value = rdp_time;
						break;
					case 2:
						string = "cpu";
						value = cpu_time;
						break;
				}

				if (value == slowest)
				{
					COnScreen__SetFontColor(ppDLP,
													255, 128, 128,
													255, 0, 0) ;
				}

				COnScreen__DrawText(ppDLP, string, 25, POSITION_YPOS + 60 + (4*16) + i*16, 255, FALSE, FALSE);

				if (value == slowest)
				{
					COnScreen__SetFontColor(ppDLP,
													255, 255, 255,
													255, 255, 255) ;
				}
			}

			COnScreen__Init16BitDraw(ppDLP, GetApp()->m_Options.m_Opacity) ;
		}
	}
#endif



//#ifdef DISPLAY_COORDINATES
#ifndef SHIP_IT
	if (GetApp()->m_bDebug)
	{
		//--------------------------------------------------------------------------------------
		// display world position
		COnScreen__DrawDigits(ppDLP,
									 (int)(pTurok->ah.ih.m_vPos.x/SCALING_FACTOR),
									 POSITION_XPOS,
									 POSITION_YPOS);
		COnScreen__DrawDigits(ppDLP,
									 (int)(pTurok->ah.ih.m_vPos.y/SCALING_FACTOR),
									 POSITION_XPOS,
									 POSITION_YPOS+16);
		COnScreen__DrawDigits(ppDLP,
									 (int)(pTurok->ah.ih.m_vPos.z/SCALING_FACTOR),
									 POSITION_XPOS,
									 POSITION_YPOS+32);
	}
#endif

		//--------------------------------------------------------------------------------------
		// Draw compass graphics
//		COnScreen__DrawCompassGraphic(ppDLP,
//											(C16BitGraphic *)NESWOverlay,
//											COMPASS_XPOS,
//											COMPASS_YPOS) ;


		//--------------------------------------------------------------------------------------
		// Display run / walk icon
		if (COverlay__OnScreen(&pThis->m_RunWalkOverlay))
		{
			if (pThis->m_WalkRunIcon==ONSCRN_WALK_ICON)
				nRunWalkMode = 0;
			else
				nRunWalkMode = 1;

			COnScreen__Draw16BitGraphic(ppDLP,
												(C16BitGraphic *)pRunWalkIcons[nRunWalkMode],
												pThis->m_RunWalkOverlay.m_X,
												pThis->m_RunWalkOverlay.m_Y) ;
		}

	}

	//--------------------------------------------------------------------------------------
	// Draw weapon selection
	COnScreen__DrawWeaponSelection(pThis, ppDLP) ;


	//--------------------------------------------------------------------------------------
	// display bossbar last coz it changes opacity
	if (pThis->m_BossBarMode)
	{
		// Make boss bar vanish?
		if ((pThis->m_pBoss) && (pThis->m_pBoss->m_PercentageHealth <= 0))
		{
			pThis->m_pBoss = NULL ;
			COnScreen__SetBossBarMode(pThis, BOSSBARMODE_FADEOUT) ;
		}

		health = 0 ;	// Default
		switch(pThis->m_BossBarMode)
		{
			// fade up
			case BOSSBARMODE_FADEON:
				if (pThis->m_pBoss)
					health = ((float)(pThis->m_pBoss->m_PercentageHealth)) / 100.0 ;

				pThis->m_BossBarOpacity += GetApp()->m_Options.m_Opacity / BOSSBAR_TIMEIN ;
				if (pThis->m_BossBarOpacity >= GetApp()->m_Options.m_Opacity)
				{
					pThis->m_BossBarOpacity = GetApp()->m_Options.m_Opacity ;
					pThis->m_BossBarMode = BOSSBARMODE_WAIT ;
				}
				break ;

			case BOSSBARMODE_FADEOUT:
				pThis->m_BossBarOpacity -= GetApp()->m_Options.m_Opacity / BOSSBAR_TIMEOUT ;
				if (pThis->m_BossBarOpacity <= 0)
				{
					pThis->m_BossBarOpacity = 0 ;
					pThis->m_BossBarMode = BOSSBARMODE_NULL ;
				}
				break ;

			default:
				if (pThis->m_pBoss)
					health = ((float)(pThis->m_pBoss->m_PercentageHealth)) / 100.0;
				break ;
		}

		COnScreen__Init16BitDraw(ppDLP, pThis->m_BossBarOpacity) ;
		COnScreen__Draw16BitGraphic(ppDLP,
											(C16BitGraphic *)BossBarOverlay,
											BOSS_XPOS, BOSS_YPOS) ;
		COnScreen__DrawBarGraphic(ppDLP,
											BOSS_XPOS+11, BOSS_YPOS+15,
											health, pThis->m_BossBarOpacity) ;
	}


	//--------------------------------------------------------------------------------------
	// display airbar last coz it changes opacity
	if (pThis->m_AirMode)
	{
		switch(pThis->m_AirMode)
		{
			// fade up
			case AIRMODE_FADEON:
				oxygen = CTMove__GetOxygenStatus(&CTurokMovement) ;
				pThis->m_AirOpacity += GetApp()->m_Options.m_Opacity / AIR_TIMEIN ;
				if (pThis->m_AirOpacity >= GetApp()->m_Options.m_Opacity)
				{
					pThis->m_AirOpacity = GetApp()->m_Options.m_Opacity ;
					pThis->m_AirMode = AIRMODE_WAIT ;
				}
				break ;
			case AIRMODE_FADEOUT:
				pThis->m_AirOpacity -= GetApp()->m_Options.m_Opacity / AIR_TIMEOUT ;
				if (pThis->m_AirOpacity <= 0)
				{
					pThis->m_AirOpacity = 0 ;
					pThis->m_AirMode = AIRMODE_NULL ;
				}
				break ;
			default:
				oxygen = CTMove__GetOxygenStatus(&CTurokMovement) ;
				break ;
		}
		COnScreen__Init16BitDraw(ppDLP, pThis->m_AirOpacity) ;
		COnScreen__Draw16BitGraphic(ppDLP,
											(C16BitGraphic *)AirBarOverlay,
											AIR_XPOS, AIR_YPOS) ;
		COnScreen__DrawBarGraphic(ppDLP,
											AIR_XPOS+11, AIR_YPOS+15,
											oxygen, pThis->m_AirOpacity) ;
	}


	// draw all lens flares for all particles
	COnScreen__DrawParticleLensFlares(pThis, ppDLP);

	// draw lens flare for sun
	//COnScreen__DrawSunLensFlare(pThis, ppDLP);


	//--------------------------------------------------------------------------------------
}




// draw sun flare
//
void COnScreen__DrawSunLensFlare(COnScreen *pThis, Gfx **ppDLP)
{
	float				x, y, opacity;
	BOOL				visible;


	CSun__GetSunData(&sun, &visible, &x, &y, &opacity);

	if (visible)
		COnScreen__DrawLensFlare(pThis, ppDLP, x, y, opacity, TRUE, FALSE);
}



// draw all particle lens flares
//
void COnScreen__DrawParticleLensFlares(COnScreen *pThis, Gfx **ppDLP)
{
	CParticle		*pParticle;
	CROMBounds		bounds;
	float				diag, diagD2,
						ScreenX,
						ScreenY;


	// go through all particles looking for particle that has a lens flare
	COnScreen__Init16BitDraw(ppDLP, 100) ;		// not neccessarily alpha

	CGeometry__SetRenderMode(ppDLP, G_RM_PASS__G_RM_CLD_SURF2);
	gDPSetTextureFilter((*ppDLP)++, G_TF_BILERP);

	pParticle = GetApp()->m_Scene.m_ParticleSystem.m_pDrawHead;
	while (pParticle)
	{
		if (pParticle->m_pEffect->m_dwFlags & PARTICLE_BEHAVIOR_HASLENSFLARE)
		{
			diag =  1.414213562373*pParticle->m_Size;
			diagD2 = 0.5*diag;

			bounds.m_vMin.x = pParticle->ah.ih.m_vPos.x - diagD2;
			bounds.m_vMax.x = pParticle->ah.ih.m_vPos.x + diagD2;
			bounds.m_vMin.y = pParticle->ah.ih.m_vPos.y - diagD2;
			bounds.m_vMax.y = pParticle->ah.ih.m_vPos.y + diag;
			bounds.m_vMin.z = pParticle->ah.ih.m_vPos.z - diagD2;
			bounds.m_vMax.z = pParticle->ah.ih.m_vPos.z + diagD2;

			if (CBoundsRect__IsOverlappingBounds(&view_bounds_rect, &bounds))
			{
				if (CViewVolume__IsOverlappingBounds(&view_volume, &bounds))
				{
					CEngineApp__GetScreenCoordinates(GetApp(), &pParticle->ah.ih.m_vPos, &ScreenX, &ScreenY);
					COnScreen__DrawLensFlare(pThis, ppDLP, ScreenX, ScreenY, 1, FALSE, FALSE);
				}
			}
		}

		pParticle = pParticle->m_pDrawNext;
	}
}

void COnScreen__DrawSun(Gfx **ppDLP)
{
	float				x, y, opacity,
						size, scaler;
	BOOL				visible;

	CSun__GetSunData(&sun, &visible, &x, &y, &opacity);
	if (!visible)
		return;

	COnScreen__Init16BitDraw(ppDLP, 255*(1 - opacity));

	CGeometry__SetRenderMode(ppDLP, G_RM_PASS__G_RM_CLD_SURF2);
	gDPSetTextureFilter((*ppDLP)++, G_TF_BILERP);

	scaler = 2;

	scaler *= 1.1;

	size = 32*scaler/2;

	if (		(x > (          - size))
			&& (x < (SCREEN_WD + size))
			&&	(y > (          - size))
			&& (y < (SCREEN_HT + size)) )
	{

		COnScreen__Draw16BitScaledGraphic(ppDLP,
													(C16BitGraphic *)SunFlareOverlay,
													x-size, y-size, scaler, scaler) ;

	}

	gDPSetTexturePersp((*ppDLP)++, G_TP_PERSP);
}

// draw a single lens flare
//
void COnScreen__DrawLensFlare(COnScreen *pThis, Gfx **ppDLP, float sx, float sy, float Opacity, BOOL Sun, BOOL Blocked)
{
	float				dx,
						dy,
						xstep,
						ystep,
						distance,
						size,
						scaler;
	int				cnt;

	float				lens[]	=	{16, 16, 24, 32, 16, 32, 48};
	int				alpha[]	=	{24, 24, 24, 24, 24, 24, 24};
	int				color[]	=	{255,150,150,
										 255,255,150,
										 150,255,150,
										 150,255,255,
										 150,150,255,
										 200,150,255,
										 255,150,255};

#define	MAXLENS		7


	if (Opacity == 0)
		return;

	gDPSetTextureFilter((*ppDLP)++, G_TF_BILERP);

	dx = SCREEN_WD/2 - sx;
	dy = SCREEN_HT/2 - sy;

	distance = sqrt(dx*dx + dy*dy) * 0.4;

	if (fabs(dx) > fabs(dy))
	{
		if (dx>0)
			xstep = 1;
		else
			xstep = -1;

		ystep = dy/fabs(dx);
	}
	else
	{
		xstep = dx/fabs(dy);

		if (dy>0)
			ystep = 1;
		else
			ystep = -1;
	}

	if (Sun)
	{
		scaler = 2*1.1;				// size scaler for sun
		size = 32*scaler/2;

		Opacity *=

		max(0,
		min(size,
		min(sx + size,
		min(sy + size,
		min(SCREEN_WD + size - sx,
			 SCREEN_HT + size - sy))))) / size;

		Opacity = max(0, min(1, Opacity));
	}

	for (cnt=0; cnt<MAXLENS; cnt++)
	{
		if (Sun && !cnt)
			scaler = 2;				// size scaler for sun
		else
			scaler = lens[cnt]/32;	// lens flare graphic is 32x32 pixels

		scaler *= 1.1;
		size = 32*scaler/2;

		if (		(sx > (          - size))
				&& (sx < (SCREEN_WD + size))
				&&	(sy > (          - size))
				&& (sy < (SCREEN_HT + size)) )
		{
			// draw lens artifact centered around the point
			if (Sun && !cnt)
			{
				gDPSetEnvColor((*ppDLP)++,
									 255, 255, 255,		// rgb
									 (BYTE) (255*Opacity));					// alpha

				COnScreen__Draw16BitScaledGraphic(ppDLP,
															(C16BitGraphic *)SunFlareOverlay,
															sx-size, sy-size, scaler, scaler) ;
			}
			else
			{
				if (		(Blocked)
						|| (		(CTurokMovement.WaterFlag == PLAYER_BELOW_WATERSURFACE)
								&&	(!CTurokMovement.InAntiGrav) ) )
				{
					break;
				}

				gDPSetEnvColor((*ppDLP)++,
									 color[cnt*3+0],
									 color[cnt*3+1],
									 color[cnt*3+2],
									 (BYTE) (alpha[cnt]*Opacity));

				COnScreen__Draw16BitScaledGraphic(ppDLP,
															(C16BitGraphic *)LensFlareOverlay,
															sx-size, sy-size, scaler, scaler) ;
			}
		}
		else
		{
			break;
		}

		sx += xstep*distance;
		sy += ystep*distance;
	}
}



//------------------------------------------------------------------------------------
//
// New Improved weapon selection, using the CBarrel class(structure!)
//
//------------------------------------------------------------------------------------

int	WeaponBuffer[TWEAPON_TOTAL] ;

void InitWeaponSelectDraw(Gfx **ppDLP)
{
	COnScreen__Init16BitDraw(ppDLP, 0) ;
	gDPSetTextureFilter((*ppDLP)++, G_TF_BILERP);
	CGeometry__SetRenderMode(ppDLP, G_RM_CLD_SURF__G_RM_CLD_SURF2);
}
void DrawWeaponSelectItem(Gfx **ppDLP, int index, float x, float y, float xscale, float yscale, int alpha)
{
	C16BitGraphic	*ppIcon ;
	int xoff, yoff ;
	float	mod ;

	mod = 64.0*cos((FLOAT)frame_number/2.0) ;
	if (WeaponSelectBarrel.m_Selected == index)
	{
		gDPSetEnvColor((*ppDLP)++,
							 192+mod,192+mod,192+mod,	// rgb
							 alpha);				// a
	}
	else
	{
		gDPSetEnvColor((*ppDLP)++,
							 alpha/2, alpha/2, alpha/2,	// rgb
							 alpha);				// a
	}

	ppIcon = (C16BitGraphic *)pWeaponIcons[WeaponBuffer[index]].Icon ;
		xoff = pWeaponIcons[WeaponBuffer[index]].XOff ;
		yoff = pWeaponIcons[WeaponBuffer[index]].YOff ;

		COnScreen__Draw16BitScaledGraphic(ppDLP,
												ppIcon,
												x+xoff, y+yoff,
												xscale, yscale) ;
}


void COnScreen__DrawWeaponSelection(COnScreen *pThis, Gfx **ppDLP)
{
	BOOL	*pWeaponFlag ;
	int	i ;
	int	newweapon, maxweapons;

	// Display weapon selection info
	newweapon = 0 ;
	if (pThis->m_WeaponOverlay.m_OnScreen)
	{
		// create a list of currently owned weapons
		pWeaponFlag = &CTurokMovement.KnifeFlag ;
		maxweapons = 0;
		for (i= 0; i< TWEAPON_TOTAL; i++)
		{
			// weapon is owned, add to buffer
			if (*pWeaponFlag)
			{
				WeaponBuffer[maxweapons] = i ;
				if (i == pThis->m_SelectPosition)
					newweapon = maxweapons ;

				maxweapons++ ;
			}
			pWeaponFlag++ ;
		}


		WeaponSelectBarrel.InitGraphics = &InitWeaponSelectDraw ;
		WeaponSelectBarrel.DrawItem = &DrawWeaponSelectItem ;

		WeaponSelectBarrel.m_X = pThis->m_WeaponOverlay.m_X + ONSCRN_WEAPONICON_XOFF ;
		WeaponSelectBarrel.m_Y = pThis->m_WeaponOverlay.m_Y + ONSCRN_WEAPONICON_YOFF ;
		WeaponSelectBarrel.m_Selected = newweapon ;
		WeaponSelectBarrel.m_Buffersize = maxweapons ;
		CBarrel__Update(&WeaponSelectBarrel) ;

		CBarrel__Draw(&WeaponSelectBarrel, ppDLP) ;
	}
}


//----------------------------------------------------------------------------------------
// set on screen health
//
void COnScreen__SetHealth(COnScreen *pThis, int nHealth)
{
	if (CTurokMovement.ArmorFlag)
	{
		if (pThis->m_HealthArmourIcon == ONSCRN_HEALTH_ICON)
		{
			COverlay__RunScript(&pThis->m_HealthOverlay, SwitchHealthArmourOverlayScript) ;
			if (pThis->m_HealthOverlay.m_Y == pThis->m_HealthOverlay.m_FinalY)
			{
				pThis->m_HealthArmourIcon = ONSCRN_ARMOUR_ICON;
				COverlay__RunScript(&pThis->m_HealthOverlay, HealthInitOverlayScript) ;
			}
		}
		else
		{
			pThis->m_Armor = CTurokMovement.ArmorAmount /3;
			if (pThis->m_Armor > LOWARMOR_ALERT)
				pThis->m_ArmorAlert = TRUE ;
		}
	}
	else
	{
		if (pThis->m_HealthArmourIcon == ONSCRN_ARMOUR_ICON)
		{
			COverlay__RunScript(&pThis->m_HealthOverlay, SwitchHealthArmourOverlayScript) ;
			if (pThis->m_HealthOverlay.m_Y == pThis->m_HealthOverlay.m_FinalY)
			{
				pThis->m_HealthArmourIcon = ONSCRN_HEALTH_ICON;
				COverlay__RunScript(&pThis->m_HealthOverlay, HealthInitOverlayScript) ;
			}
		}
		else
		{
			pThis->m_Health = nHealth;
			if (nHealth > LOWHEALTH_ALERT)
				pThis->m_HealthAlert = TRUE ;
		}
	}
}


//----------------------------------------------------------------------------------------
// set on screen lives
//
//char	lifebuffer[32] ;

void COnScreen__SetLives(COnScreen *pThis, int nLives)
{
	if (CCamera__InCinemaMode(&GetApp()->m_Camera))
		return ;

	if (pThis->m_Lives != nLives)
	{
		COverlay__RunScript(&pThis->m_LivesOverlay, LivesChangeOverlayScript) ;
		//COverlay__RunScript(&pThis->m_TokensOverlay, TokensChangeOverlayScript) ;
		pThis->m_Lives = nLives;

		//if (nLives ==1)
		//	COnScreen__AddGameText("1 life left") ;
	  	//else if (nLives ==0)
		//	COnScreen__AddGameText("last life") ;
		//else
	  	//{
		//	sprintf(lifebuffer, "%d lives left", nLives) ;
		//	COnScreen__AddGameText(lifebuffer) ;
		//}
	}
}

//----------------------------------------------------------------------------------------
// set on screen tokens
//
void COnScreen__SetTokens(COnScreen *pThis, int nTokens)
{
	if (pThis->m_Tokens != nTokens)
	{
		COverlay__RunScript(&pThis->m_TokensOverlay, TokensChangeOverlayScript) ;
		pThis->m_Tokens = nTokens ;
	}
}



//----------------------------------------------------------------------------------------
// set on screen ammo count
//
void COnScreen__SetAmmo(COnScreen *pThis, int nAmmo, BOOL explosive)
{
	// Bring Ammo overlay onto screen?
	if ((nAmmo >= 0) && (pThis->m_Health > 0))
	{
		// Only bring on if ammo number is different!
		if ((!COverlay__OnScreen(&pThis->m_AmmoOverlay)) && (pThis->m_Ammo != nAmmo))
			COverlay__RunScript(&pThis->m_AmmoOverlay, AmmoOnOverlayScript) ;
	}
	else if ((COverlay__OnScreen(&pThis->m_AmmoOverlay)) &&
				(!COverlay__Active(&pThis->m_AmmoOverlay)))
	{
		// Make ammo drop off
		COverlay__RunScript(&pThis->m_AmmoOverlay, AmmoDropOffOverlayScript) ;
	}

	pThis->m_Ammo = nAmmo ;

	pThis->m_ExplosiveAmmo = explosive ;
}



//----------------------------------------------------------------------------------------
// set on screen boss bar mode
//


void COnScreen__SetBossBarMode(COnScreen *pThis, int mode)
{
	if (pThis->m_BossBarMode != mode)
	{
		switch(mode)
		{
			case BOSSBARMODE_FADEOUT:
				if (pThis->m_BossBarMode != BOSSBARMODE_NULL)
					pThis->m_BossBarMode = mode ;
				break ;

			default:
				pThis->m_BossBarMode = mode ;
		}
	}
}
BOOL COnScreen__BossHasBar(COnScreen *pThis, CBoss *pBoss)
{
	return (pThis->m_pBoss == pBoss) ;
}

void COnScreen__AddBossBar(COnScreen *pThis, CBoss *pBoss)
{
	pThis->m_pBoss = pBoss ;
	COnScreen__SetBossBarMode(pThis, BOSSBARMODE_FADEON) ;
}




//----------------------------------------------------------------------------------------
// set on screen air mode
//
void COnScreen__SetAirMode(COnScreen *pThis, int mode)
{
	if (pThis->m_AirMode != mode)
	{
		switch(mode)
		{
			case AIRMODE_FADEOUT:
				if (pThis->m_AirMode != AIRMODE_NULL)
					pThis->m_AirMode = mode ;
				break ;

			default:
				pThis->m_AirMode = mode ;
		}
	}
}



//----------------------------------------------------------------------------------------
// Set walk or run icon
//
void COnScreen__SetWalkRunIcon(COnScreen *pThis, INT32 nIcon)
{
	// Alive?
	if (pThis->m_Health > 0)
	{
		// bring run / walk selection icon on screen
		COverlay__RunScript(&pThis->m_RunWalkOverlay, RunWalkOnOverlayScript) ;
		pThis->m_WalkRunIcon = nIcon ;
	}
}


//----------------------------------------------------------------------------------------
// bring on weapon icons for weapon selection
//
void COnScreen__DisplayWeaponChoice(COnScreen *pThis, int SelectPosition, BOOL Direction)
{
	// Alive?
	if (pThis->m_Health > 0)
	{
		// bring weapon selection icons on screen
		COverlay__RunScript(&pThis->m_WeaponOverlay, WeaponOnOverlayScript) ;
		pThis->m_SelectPosition = SelectPosition;
		pThis->m_SelectionTimer = 0 ;

		pThis->m_BarrelDirection = Direction ;
		WeaponSelectBarrel.m_Direction = Direction ;
	}
}

void COnScreen__QuickWeaponChange(COnScreen *pThis, int SelectPosition)
{
	pThis->m_SelectPosition = SelectPosition;
	pThis->m_BarrelDirection = 1 ;

	// force the current weapon selection to be setup...
//	if (COnScreen__IsWeaponOverlayOn(pThis) == FALSE)
//	{
//		WeaponSelectBarrel.m_Selected = pThis->m_SelectPosition ;
//		WeaponSelectBarrel.m_SelectedAngle = pThis->m_SelectPosition * WeaponSelectBarrel.m_DisplayStep ;
//		WeaponSelectBarrel.m_Current = WeaponSelectBarrel.m_Selected ;
//		WeaponSelectBarrel.m_CurrentAngle = WeaponSelectBarrel.m_SelectedAngle ;
//	}
	// make start at 0 and spin down to correct weapon
	if (COnScreen__IsWeaponOverlayOn(pThis) == FALSE)
	{
		WeaponSelectBarrel.m_Selected = 0 ;
		WeaponSelectBarrel.m_SelectedAngle = 0 ;
		WeaponSelectBarrel.m_Current = WeaponSelectBarrel.m_Selected ;
		WeaponSelectBarrel.m_CurrentAngle = WeaponSelectBarrel.m_SelectedAngle ;
	}


	if (pThis->m_Ammo > 0)
	{
		if (!COverlay__OnScreen(&pThis->m_AmmoOverlay))
			COverlay__RunScript(&pThis->m_AmmoOverlay, AmmoOnOverlayScript) ;

	}
}

// is weapon overlay on screen ?
//
BOOL COnScreen__IsWeaponOverlayOn(COnScreen *pThis)
{
	return COverlay__OnScreen(&pThis->m_WeaponOverlay);
}








/////////////////////////////////////////////////////////////////////////////
// Overlay code
/////////////////////////////////////////////////////////////////////////////

void COverlay__Construct(COverlay *pThis, INT16 nScreenX, INT16 nScreenY)
{
	// reset vars
	pThis->m_OnScreen = FALSE ;
	pThis->m_Mode = OVERLAY_INACTIVE_MODE,
	pThis->m_X = (FLOAT)nScreenX ;
	pThis->m_Y = (FLOAT)nScreenY ;
}



// Start script into action
//
void COverlay__RunScript(COverlay *pThis, INT32 *pScript)
{
	pThis->m_pScript = pScript ;
	pThis->m_Mode = OVERLAY_READ_SCRIPT_MODE ;
	COverlay__Update(pThis) ;
}



// Update overlay movement
//
void COverlay__Update(COverlay *pThis)
{
	BOOL	DoNextCommand ;
	FLOAT	XVel, YVel ;

	// Update movement
	do
	{
		DoNextCommand = FALSE ;
		switch(pThis->m_Mode)
		{
			case OVERLAY_INACTIVE_MODE:
				break ;

			case OVERLAY_READ_SCRIPT_MODE:

				// Script active?
				if (pThis->m_pScript)
				{
					// Process new script command
					switch(*pThis->m_pScript++)
					{
						case OVERLAY_SCRIPT_SET_POS:
							pThis->m_X = (FLOAT)*pThis->m_pScript++ ;
							pThis->m_Y = (FLOAT)*pThis->m_pScript++ ;
							DoNextCommand = TRUE ;
							break ;

						case OVERLAY_SCRIPT_WAIT:
							pThis->m_Mode = OVERLAY_WAIT_MODE ;
							pThis->m_Time = *pThis->m_pScript++ ;
							break ;

						case OVERLAY_SCRIPT_ZOOM:
							pThis->m_OnScreen = TRUE ;
							pThis->m_Mode = OVERLAY_ZOOM_MODE ;
							pThis->m_FinalX = (FLOAT)*pThis->m_pScript++ ;
							pThis->m_FinalY = (FLOAT)*pThis->m_pScript++ ;
							pThis->m_ZoomFactor = 1.0 / (FLOAT)*pThis->m_pScript++ ;
							break ;

						case OVERLAY_SCRIPT_DROP_OFF:
							pThis->m_OnScreen = TRUE ;
							pThis->m_Mode = OVERLAY_DROP_OFF_MODE ;
							pThis->m_YVel = 0.0 ;
							pThis->m_YAccel = ((FLOAT)*pThis->m_pScript++) / 65536.0 ;
							break ;

						case OVERLAY_SCRIPT_HORZDROP_OFF:
							pThis->m_OnScreen = TRUE ;
							pThis->m_Mode = OVERLAY_HORZDROP_OFF_MODE ;
							pThis->m_XVel = 0.0 ;
							pThis->m_XAccel = ((FLOAT)*pThis->m_pScript++) / 65536.0 ;
							break ;

						default:
							pThis->m_Mode = OVERLAY_INACTIVE_MODE ;
							pThis->m_pScript = NULL ;
							break ;
					}
				}
				else
					pThis->m_Mode = OVERLAY_INACTIVE_MODE ;

				break ;

			case OVERLAY_WAIT_MODE:

				// Timer ran out?
				if (pThis->m_Time-- <= 0)
					DoNextCommand = TRUE ;

				break ;

			case OVERLAY_ZOOM_MODE:

				// Calculate zoom vels
				XVel = (pThis->m_FinalX - pThis->m_X) * pThis->m_ZoomFactor ;
				YVel = (pThis->m_FinalY - pThis->m_Y) * pThis->m_ZoomFactor ;

				// Move overlay
				pThis->m_X += XVel ;
				pThis->m_Y += YVel ;

				// Snap to final target
				if ((abs(XVel) < 0.25) && (abs(YVel) < 0.25))
				{
					pThis->m_X = pThis->m_FinalX ;
					pThis->m_Y = pThis->m_FinalY ;
					DoNextCommand = TRUE ;
				}
			  	break ;

			case OVERLAY_DROP_OFF_MODE:

				// Move
				pThis->m_Y += pThis->m_YVel ;
				pThis->m_YVel += pThis->m_YAccel ;

				// Off bottom of screen?
				if ((pThis->m_Y > (240+96)) || (pThis->m_Y<-90))
				{
					pThis->m_OnScreen = FALSE ;
					DoNextCommand = TRUE ;
				}

				break ;

			case OVERLAY_HORZDROP_OFF_MODE:

				// Move
				pThis->m_X += pThis->m_XVel ;
				pThis->m_XVel += pThis->m_XAccel ;

				// Off left / right of screen?
				if ((pThis->m_X > (320+96)) || (pThis->m_X<-90))
				{
					pThis->m_OnScreen = FALSE ;
					DoNextCommand = TRUE ;
				}

				break ;
		}

		// Process next command?
		if (DoNextCommand)
			pThis->m_Mode = OVERLAY_READ_SCRIPT_MODE ;

	}
	while(DoNextCommand) ;
}





/////////////////////////////////////////////////////////////////////////////
// Texture draw functions
/////////////////////////////////////////////////////////////////////////////
#if 0
// Prepare for a texture draw
//
void COnScreen__InitTextureDraw(Gfx **ppDLP, int Opacity)
{
	CTextureLoader__InvalidateTextureCache();

	gDPPipeSync((*ppDLP)++);

	gSPTexture((*ppDLP)++,
				  0x8000,
				  0x8000,
				  0, 0, G_ON);
	gDPSetTextureLUT((*ppDLP)++, G_TT_NONE);

	gDPSetCycleType((*ppDLP)++, G_CYC_1CYCLE);

	CGeometry__SetRenderMode(ppDLP, G_RM_XLU_SURF__G_RM_XLU_SURF2);

	CGeometry__SetCombineMode(ppDLP, G_CC_ROB_DECALRGBA_PRIMALPHA__G_CC_ROB_DECALRGBA_PRIMALPHA);
	gDPSetPrimColor((*ppDLP)++,
						 0, 0,				// lod for mip-mapping
						 255, 255, 255,	// rgb
						 Opacity);			// a


	gDPSetTexturePersp((*ppDLP)++, G_TP_NONE);
}



// Draw texture
//
void COnScreen__DrawTexture(Gfx **ppDLP,
									 void *pTexture,
									 int XPos, int YPos,
									 int TextureWidth, int TextureHeight)
{
	gDPLoadTextureBlock(((*ppDLP)++),
							  RDP_ADDRESS(pTexture), G_IM_FMT_RGBA, G_IM_SIZ_32b,
							  TextureWidth, TextureHeight, 0,
							  G_TX_NOMIRROR, G_TX_NOMIRROR,
							  G_TX_NOMASK, G_TX_NOMASK,
							  G_TX_NOLOD, G_TX_NOLOD);

   gSPTextureRectangle((*ppDLP)++,
							  (XPos << 2), (YPos << 2),
							  ((XPos + TextureWidth) << 2), ((YPos + TextureHeight) << 2),
							  G_TX_RENDERTILE,
							  0, 0, (1 << 10), (1 << 10));
}


void COnScreen__DrawScaledTexture(Gfx **ppDLP,
											void *pTexture,
											float XPos, float YPos,
											int TextureWidth, int TextureHeight,
											float XScale, float YScale)
{
	int	xstep ;
	int	ystep ;

	int xPosInt, xRightPosInt;
	int yPosInt, yBottomPosInt;

	xstep = (1<<10) ;
	if (XScale != 0.0)
		xstep *= (1/XScale) ;
	ystep = (1<<10) ;
	if (YScale != 0.0)
		ystep *= (1/YScale) ;

	xPosInt = (int) (XPos * (1 << 2));
	xRightPosInt = (int) ((XPos + TextureWidth*XScale) * (1 << 2));
	yPosInt = (int) (YPos * (1 << 2));
	yBottomPosInt = (int) ((YPos + TextureHeight*YScale) * (1 << 2));

	gDPLoadTextureBlock(((*ppDLP)++),
							  RDP_ADDRESS(pTexture), G_IM_FMT_RGBA, G_IM_SIZ_32b,
							  TextureWidth, TextureHeight, 0,
							  G_TX_NOMIRROR, G_TX_NOMIRROR,
							  G_TX_NOMASK, G_TX_NOMASK,
							  G_TX_NOLOD, G_TX_NOLOD);

   gSPTextureRectangle((*ppDLP)++,
							  xPosInt, yPosInt,
							  xRightPosInt, yBottomPosInt,
							  G_TX_RENDERTILE,
							  0, 0, xstep, ystep);
}


void COnScreen__DrawScrollTexture(Gfx **ppDLP,
										 void *pTexture,
										 int XPos, int YPos,
										 int TextureWidth, int TextureHeight,
										 int XOff, int YOff)
{
	gDPLoadTextureBlock(((*ppDLP)++),
							  RDP_ADDRESS(pTexture), G_IM_FMT_RGBA, G_IM_SIZ_32b,
							  TextureWidth, TextureHeight, 0,
							  G_TX_WRAP, G_TX_WRAP,
							  4, 4,
							  G_TX_NOLOD, G_TX_NOLOD);

   gSPTextureRectangle((*ppDLP)++,
							  (XPos << 2), (YPos << 2),
							  ((XPos + (TextureWidth-XOff)) << 2), ((YPos + (TextureHeight-YOff)) << 2),
							  G_TX_RENDERTILE,
							  (XOff<<5), (YOff<<5), (1 << 10), (1 << 10));
}

void COnScreen__DrawClipTexture(Gfx **ppDLP,
										 void *pTexture,
										 int XPos, int YPos,
										 int TextureWidth, int TextureHeight,
										 int XSize, int YSize)
{
	gDPLoadTextureBlock(((*ppDLP)++),
							  RDP_ADDRESS(pTexture), G_IM_FMT_RGBA, G_IM_SIZ_32b,
							  TextureWidth, TextureHeight, 0,
							  G_TX_NOMIRROR, G_TX_NOMIRROR,
							  G_TX_NOMASK, G_TX_NOMASK,
							  G_TX_NOLOD, G_TX_NOLOD);

   gSPTextureRectangle((*ppDLP)++,
							  (XPos << 2), (YPos << 2),
							  ((XPos + XSize) << 2), ((YPos + YSize) << 2),
							  G_TX_RENDERTILE,
							  0, 0, (1 << 10), (1 << 10));
}

#endif





/////////////////////////////////////////////////////////////////////////////
// Box draw functions
/////////////////////////////////////////////////////////////////////////////

// Prepare for box draw
//
void COnScreen__InitBoxDraw(Gfx **ppDLP)
{
	gDPPipeSync((*ppDLP)++);
	gDPSetCycleType((*ppDLP)++, G_CYC_1CYCLE);

	CGeometry__SetRenderMode(ppDLP, G_RM_XLU_SURF__G_RM_XLU_SURF2);

	CGeometry__SetCombineMode(ppDLP, G_CC_PRIMITIVE__G_CC_PRIMITIVE);
}


// Draw box
//
void COnScreen__DrawBox(Gfx **ppDLP, INT32 X1, INT32 Y1, INT32 X2, INT32 Y2,
								UINT8 Red, UINT8 Green, UINT8 Blue, UINT8 Alpha)
{
	// Setup color
	gDPPipeSync((*ppDLP)++);
	gDPSetPrimColor((*ppDLP)++,
						 255, 255,				// LOD stuff
						 Red, Green, Blue,	// R,G,B
						 Alpha);					// Alpha

	gDPScisFillRectangle((*ppDLP)++, X1,Y1,X2,Y2) ;
}


// Draw box
//
void COnScreen__DrawBoxOutLine(Gfx **ppDLP, INT32 X1, INT32 Y1, INT32 X2, INT32 Y2, INT32 Thickness,
										 UINT8 Red, UINT8 Green, UINT8 Blue, UINT8 Alpha)
{
	// Setup color
	gDPPipeSync((*ppDLP)++);
	gDPSetPrimColor((*ppDLP)++,
						 255, 255,				// LOD stuff
						 Red, Green, Blue,	// R,G,B
						 Alpha);					// Alpha

	// Top edge
	gDPScisFillRectangle((*ppDLP)++, X1,Y1, X2,Y1+Thickness) ;

	// Bottom edge
	gDPScisFillRectangle((*ppDLP)++, X1,Y2-Thickness, X2,Y2) ;

	// Left edge
	gDPScisFillRectangle((*ppDLP)++, X1,Y1+Thickness, X1+Thickness,Y2-Thickness) ;

	// Right edge
	gDPScisFillRectangle((*ppDLP)++, X2-Thickness,Y1+Thickness, X2,Y2-Thickness) ;
}

void COnScreen__DrawHilightBox(Gfx **ppDLP, INT32 x1, INT32 y1, INT32 x2, INT32 y2, INT32 thick, BOOL invert,
										 UINT8 red, UINT8 green, UINT8 blue, UINT8 alpha)
{
	int	lr, lg, lb ;
	int	dr, dg, db ;

	lr = (invert) ? red/2 : red*2 ;
	lg = (invert) ? green/2 : green*2 ;
	lb = (invert) ? blue/2 : blue*2 ;
	dr = (invert) ? red*2 : red/2 ;
	dg = (invert) ? green*2 : green/2 ;
	db = (invert) ? blue*2 : blue/2 ;
	// draw main box
	COnScreen__DrawBox(ppDLP,
							 x1, y1,
							 x2, y2,
							 red, green, blue, alpha) ;

	// draw lines around boxes
	COnScreen__DrawBox(ppDLP,
							 x1, y1,
							 x2, y1+thick,
							 lr,lg,lb, alpha) ;
	COnScreen__DrawBox(ppDLP,
							 x2-thick, y1,
							 x2, y2,
							 lr,lg,lb, alpha) ;
	COnScreen__DrawBox(ppDLP,
							 x1, y2-thick,
							 x2, y2,
							 dr,dg,db, alpha) ;
	COnScreen__DrawBox(ppDLP,
							 x1, y1,
							 x1+thick, y2,
							 dr,dg,db, alpha) ;
}





/////////////////////////////////////////////////////////////////////////////
// Misc draw functions
/////////////////////////////////////////////////////////////////////////////
#if 0
void COnScreen__DrawGridGraphic(Gfx **ppDLP, CGridGraphic *Grid, INT16 XPos, INT16 YPos)
{
	UINT32 	X, XCount, YCount, Size ;
	UINT8		*Data = Grid->m_pData ;

	ASSERT((Grid->m_BlocksAcross > 0) && (Grid->m_BlocksDown > 0)) ;
	ASSERT((Grid->m_BlockWidth == 16) && (Grid->m_BlockHeight == 16)) ;

	Size = Grid->m_BlockWidth * Grid->m_BlockHeight * 4 ;
	YCount = Grid->m_BlocksDown;
	while(YCount--)
	{
		XCount = Grid->m_BlocksAcross ;
		X = XPos ;
		while(XCount--)
		{
			COnScreen__DrawTexture(ppDLP, (void *)Data,
										  X, YPos,
										  Grid->m_BlockWidth, Grid->m_BlockHeight) ;

			X += Grid->m_BlockWidth ;
			Data += Size ;
		}

		YPos += Grid->m_BlockHeight ;
	}
}


void COnScreen__DrawScaledGridGraphic(Gfx **ppDLP, CGridGraphic *Grid, float XPos, float YPos, float XScale, float YScale)
{
	float 	X, XCount, YCount ;
	INT32		Size ;
	UINT8		*Data = Grid->m_pData ;


//	rmonPrintf("w:%d h:%d pw:%d ph:%d\r\n", Grid->m_BlocksAcross, Grid->m_BlocksDown,
//					Grid->m_BlockWidth, Grid->m_BlockHeight) ;

	ASSERT((Grid->m_BlocksAcross > 0) && (Grid->m_BlocksDown > 0)) ;
	ASSERT((Grid->m_BlockWidth == 16) && (Grid->m_BlockHeight == 16)) ;

	Size = Grid->m_BlockWidth * Grid->m_BlockHeight * 4 ;
	YCount = Grid->m_BlocksDown;
	while(YCount--)
	{
		XCount = Grid->m_BlocksAcross ;
		X = XPos ;
		while(XCount--)
		{
			COnScreen__DrawScaledTexture(ppDLP, (void *)Data,
												 X, YPos,
												Grid->m_BlockWidth, Grid->m_BlockHeight,
												XScale, YScale) ;

			X += Grid->m_BlockWidth * XScale ;
			Data += Size ;
		}

		YPos += Grid->m_BlockHeight * YScale ;
	}
}
#endif



#if 0
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// based on 8 32x16 blocks (256 wide texture), single block high
// effectively displaying 2 32x16 blocks with correct shift
#define		COMPASS_WIDTH		(2*32)
#define		COMPASS_MAXBLOCKS	(8)

void COnScreen__DrawCompassGraphic(Gfx **ppDLP, C16BitGraphic *Grid, INT16 XPos, INT16 YPos)
{
	UINT32 	x, pixels ;
	void		*image, *opacity ;
	int		block = 0;
	UINT8		*Part = Grid->m_pPart ;

	CGameObjectInstance	*pTurok ;
	FLOAT						yrot, delta ;
	INT32						Shift ;

	COnScreen		*pThis = &GetApp()->m_OnScreen ;


	// get a pointer to the player and extract the view direction (in radians)
	pTurok = CEngineApp__GetPlayer(GetApp());
	yrot = pTurok->m_RotY ;

	// convert radian direction to texel coordinates (-pi to pi)
	yrot += (3.141592654) ;
	Shift = yrot * (COMPASS_MAXBLOCKS * 32) / (2*3.141592654) ;


	// work out difference and check for wrapping wrong direction
	delta = Shift - pThis->m_CompassPos;
	if (delta > 128)
		delta = delta - (COMPASS_MAXBLOCKS*32) ;
	else if (delta < -128)
		delta = delta + (COMPASS_MAXBLOCKS*32) ;
	delta -= pThis->m_CompassAccel;

	// dampen the acceleration to stop it looking crap
	pThis->m_CompassAccel += (delta /8) ;
	if (fabs(pThis->m_CompassAccel) < 0.1)
		pThis->m_CompassAccel = 0 ;

	pThis->m_CompassPos += pThis->m_CompassAccel ;
	if (pThis->m_CompassPos > (COMPASS_MAXBLOCKS*32))
		pThis->m_CompassPos -= (COMPASS_MAXBLOCKS*32) ;
	else if (pThis->m_CompassPos < 0)
		pThis->m_CompassPos += (COMPASS_MAXBLOCKS*32) ;


	// use correct shift value
	Shift = pThis->m_CompassPos ;



	// make this be the centre of the compass
	Shift -= (COMPASS_WIDTH / 2) ;
	Shift += 8 ;							// take a bit more of to centre letter (16wide)
	if (Shift <0)
		Shift += (COMPASS_MAXBLOCKS *32) ;


	Shift %= 256 ;
	while (Shift > 32)
	{
		Part += (32*16*2) + (32*16/2) ;
		Part += 8 ;
		block++ ;
		Shift -= 32 ;
	}
	x = XPos ;

	// loop through each block
	pixels = 0;
	while (pixels < (COMPASS_WIDTH-32))
	{
		image = ((C16BitPart *)Part)->m_pData ;
		opacity = ((C16BitPart *)Part)->m_pData + (32*16*2);
		COnScreen__Draw16BitScrollTexture(ppDLP,
									  image, opacity,
									  x, YPos,
									  32, 16,				// width , height
									  Shift, 0) ;

		// move along to next x position
		x += (32-Shift) ;

		// point to next block to draw, wrap to start if necessary
		Part += (32*16*2) + (32*16/2) ;
		Part += 8 ;
		block++ ;
		if (block >= COMPASS_MAXBLOCKS)
		{
			Part = Grid->m_pPart ;
			block = 0 ;
		}

		// advance pixels drawn
		pixels += (32-Shift) ;

		// amount of next block to skip
		Shift = 0 ;
	}

	// last block is special case to clip right edge
	// 16 or less pixels left to draw
	pixels = COMPASS_WIDTH - pixels ;

	image = ((C16BitPart *)Part)->m_pData ;
	opacity = ((C16BitPart *)Part)->m_pData + (32*16*2);
	COnScreen__Draw16BitClipTexture(ppDLP,
								  image, opacity,
								  x, YPos,
								  32, 16,				// width, height
								  pixels, 16) ;		// height
}
#endif



// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// draw a bar for health etc..
#define	ACTUALBAR_WIDTH		108
#define	BAR_WIDTH		112
#define	BAR_HEIGHT		9

void COnScreen__DrawBarGraphic(Gfx **ppDLP, s16 X, s16 Y, float pos, float opacity)
{
	s16	width = ACTUALBAR_WIDTH * pos ;

	// make sure something is drawn...
	if ((width == 0) && (pos !=0.0))
		width = 1 ;

	gDPPipeSync((*ppDLP)++);
	CGeometry__SetCombineMode(ppDLP, G_CC_MULTIBIT_ALPHA__G_CC_PASS2);
	CGeometry__SetRenderMode(ppDLP, G_RM_XLU_SURF__G_RM_XLU_SURF2);

	_gDPLoadTextureBlock_4b((*ppDLP)++,
									RDP_ADDRESS(BarGradient), 256, G_IM_FMT_I,
									BAR_WIDTH, BAR_HEIGHT, 0,
									G_TX_NOMIRROR, G_TX_NOMIRROR,
								   G_TX_NOMASK, G_TX_NOMASK,
									G_TX_NOLOD, G_TX_NOLOD);

	gDPSetTile((*ppDLP)++,
				  G_IM_FMT_I, G_IM_SIZ_4b, BAR_WIDTH/16, 256, 1,
				  0,
				  0, 0, 0,
				  0, 0, 0);
	gDPSetTileSize((*ppDLP)++,
						1, 0, 0,
						(BAR_WIDTH-1)<< 2, (BAR_HEIGHT-1)<< 2);


	gDPSetPrimColor((*ppDLP)++,
						 0, 0,
						 139, 78, 59,			//0, 200, 200,				// right rgb
						 255);			// unused a

	gDPSetEnvColor((*ppDLP)++,
						 193, 172, 106,		//200, 0, 200, 	// left rgb
						opacity);	  				// a

	gDPLoadTextureBlock_4b((*ppDLP)++,
							  RDP_ADDRESS(GradientOverlay), G_IM_FMT_I,
							  BAR_WIDTH, BAR_HEIGHT, 0,
							  G_TX_NOMIRROR, G_TX_NOMIRROR,
							  G_TX_NOMASK, G_TX_NOMASK,
							  G_TX_NOLOD, G_TX_NOLOD);
   gSPTextureRectangle((*ppDLP)++,
							  X<<2, Y<<2,
							  (X+width)<<2, (Y+BAR_HEIGHT)<<2,
							  G_TX_RENDERTILE,
							  0, 0, 1<<10, 1<<10);

}



// --------------------------------------------------------------------------
// draw digits
//
void COnScreen__DrawDigits(Gfx **ppDLP,
									int nNumber,
									int nX,
									int nY)
{
	// declare vars
	int	num1000,
			num100,
		   num10,
			num1,
			num,
			negative ;

	int	cnt,
		   maxcnt;


	// make sure correct sign
	negative = FALSE ;
	if (nNumber < 0)
	{
		nNumber = -nNumber ;
		negative = TRUE ;
	}

	// valid number count ?
	if (nNumber >= 0)
	{
		// calculate digits for number
		num1000 =  nNumber * 0.001;
		num100  = (nNumber - (num1000*1000)) * 0.01;
		num10   = (nNumber - (num1000*1000) - (num100*100)) * 0.1;
		num1    =  nNumber - (num1000*1000) - (num100*100) - (num10*10);

		// how many digits to draw (remove leading zeros)
// made left justified by carl, on the basis of it looking nicer!
// I just took out all this nx +- stuff here...
//		nX -= (4-nDigits) * (nNumWidth+nNumSpacer);
		maxcnt = 4-1;
		if (num1000==0)
		{
			maxcnt--;
//			nX += (nNumWidth+nNumSpacer);
			if (num100==0)
			{
				maxcnt--;
//				nX += (nNumWidth+nNumSpacer);
				if (num10==0)
				{
					maxcnt--;
//					nX += (nNumWidth+nNumSpacer);
				}
			}
		}

		// draw negative sign if needed
		if (negative)
	  	{
			COnScreen__Draw16BitGraphic(ppDLP, (C16BitGraphic *)FontMinus, nX, nY) ;
	  		nX += (ONSCRN_ALIGNEDWIDTH+ONSCRN_NUMERAL_SPACER);
	  	}
		// draw number digits
		for (cnt=maxcnt; cnt>=0; cnt--)
		{
			switch (cnt)
			{
				case 3:
					num = num1000;
					break;

				case 2:
					num = num100;
					break;

				case 1:
					num = num10;
					break;

				case 0:
					num = num1;
					break;
			}

			if (num>=0 && num<=9)
				COnScreen__Draw16BitGraphic(ppDLP, (C16BitGraphic *)pNumbers[num], nX, nY) ;

	  		nX += (ONSCRN_ALIGNEDWIDTH+ONSCRN_NUMERAL_SPACER);
		}
	}
}


// -------------------------------------------------------------------------------------------------------------
//
// Prepare for a font draw
//
// -------------------------------------------------------------------------------------------------------------
void COnScreen__InitFontDraw(Gfx **ppDLP)
{
	gDPPipeSync((*ppDLP)++);
	gDPSetCycleType((*ppDLP)++, G_CYC_2CYCLE);

	CGeometry__SetCombineMode(ppDLP, G_CC_MULTIBIT_ALPHA__G_CC_PASS2);
	CGeometry__SetRenderMode(ppDLP, G_RM_XLU_SURF__G_RM_XLU_SURF2);

	gSPTexture((*ppDLP)++,
				  0x8000,
				  0x8000,
				  0, 0, G_ON);
	gDPSetTextureLUT((*ppDLP)++, G_TT_NONE);

	gDPSetTexturePersp((*ppDLP)++, G_TP_NONE);
	gDPSetTextureFilter((*ppDLP)++, G_TF_BILERP);

#ifdef KANJI
	FontType = KANJI_FONT ;
#else
	FontType = LARGE_FONT ;
#endif

	_gDPLoadTextureBlock_4b((*ppDLP)++,
									RDP_ADDRESS(FontRamp), 256, G_IM_FMT_I,
									16, 16, 0,
									G_TX_WRAP, G_TX_WRAP,
									4, 4,
									G_TX_NOLOD, G_TX_NOLOD);

	gDPSetTile((*ppDLP)++,
				  G_IM_FMT_I, G_IM_SIZ_4b, 1, 256, 1,
				  0,
				  0, 0, 0,
				  0, 0, 0);
	gDPSetTileSize((*ppDLP)++,
						1, 0, 0,
						15 << 2, 15 << 2);

	// set the default color
	COnScreen__SetFontColor(ppDLP, 200, 200, 138,
									86, 71,47) ;
	// set the default scale
	COnScreen__SetFontScale(1.0, 1.0) ;

	GetApp()->m_OnScreen.m_ShadowXOff = 2 ;
	GetApp()->m_OnScreen.m_ShadowYOff = 2 ;

}

void COnScreen__SetFontColor(Gfx **ppDLP, int tr, int tg, int tb, int br, int bg, int bb)
{
	GetApp()->m_OnScreen.m_TopR = tr ;
	GetApp()->m_OnScreen.m_TopG = tg ;
	GetApp()->m_OnScreen.m_TopB = tb ;
	GetApp()->m_OnScreen.m_BottomR = br ;
	GetApp()->m_OnScreen.m_BottomG = bg ;
	GetApp()->m_OnScreen.m_BottomB = bb ;
}

void COnScreen__SetFontScale(float XScale, float YScale)
{
	GetApp()->m_OnScreen.m_FontXScale = XScale ;
	GetApp()->m_OnScreen.m_FontYScale = YScale ;
}

void COnScreen__PrepareLargeFont(Gfx **ppDLP)
{
	FontType = LARGE_FONT ;
	_gDPLoadTextureBlock_4b((*ppDLP)++,
									RDP_ADDRESS(FontRamp), 256, G_IM_FMT_I,
									16, 16, 0,
									G_TX_WRAP, G_TX_WRAP,
									4, 4,
									G_TX_NOLOD, G_TX_NOLOD);
}

void COnScreen__PrepareKanjiFont(Gfx **ppDLP)
{
	FontType = KANJI_FONT ;
	_gDPLoadTextureBlock_4b((*ppDLP)++,
									RDP_ADDRESS(FontRamp), 256, G_IM_FMT_I,
									16, 16, 0,
									G_TX_WRAP, G_TX_WRAP,
									4, 4,
									G_TX_NOLOD, G_TX_NOLOD);
}


void COnScreen__PrepareSmallFont(Gfx **ppDLP)
{
	FontType = SMALL_FONT ;
	_gDPLoadTextureBlock_4b((*ppDLP)++,
									RDP_ADDRESS(FontRamp), 256, G_IM_FMT_I,
									16, 8, 0,
									G_TX_WRAP, G_TX_WRAP,
									4, 3,
									G_TX_NOLOD, G_TX_NOLOD);
}

void COnScreen__PrepareNintendoFont(Gfx **ppDLP)
{
	COnScreen__PrepareSmallFont(ppDLP) ;
	FontType = NINTENDO_FONT ;
}




// --------------------------------------------------------------------------
// Draw 4bit texture combined with 4bit opacity for font
// --------------------------------------------------------------------------

Gfx font_texture_load_16x16_dl[] =
{
	gsDPSetTile(G_IM_FMT_I, G_IM_SIZ_16b, 0, 0, G_TX_LOADTILE, 0,
		G_TX_CLAMP, 4, G_TX_NOLOD, G_TX_CLAMP, 4, G_TX_NOLOD),
	gsDPLoadSync(),
	gsDPLoadBlock(G_TX_LOADTILE, 0, 0, (((16)*(16)+3)>>2)-1, CALC_DXT_4b(16)),
	gsDPPipeSync(),
	gsDPSetTile(G_IM_FMT_I, G_IM_SIZ_4b, ((((16)>>1)+7)>>3), 0,
		G_TX_RENDERTILE, 0, G_TX_CLAMP, 4, G_TX_NOLOD, G_TX_CLAMP, 4,
		G_TX_NOLOD),
	gsDPSetTileSize(G_TX_RENDERTILE, 0, 0,
		((16)-1) << G_TEXTURE_IMAGE_FRAC,
		((16)-1) << G_TEXTURE_IMAGE_FRAC),

	gsSPEndDisplayList(),
};
void COnScreen__DrawFontTexture(Gfx **ppDLP,
										void *pTexture,
										float XPos, float YPos,
										float XScale, float YScale)
{
	int	xstep ;
	int	ystep ;

	int xPosInt, xRightPosInt;
	int yPosInt, yBottomPosInt;

	xstep = (1<<10) ;
	if (XScale != 0.0)
		xstep *= (1/XScale) ;
	ystep = (1<<10) ;
	if (YScale != 0.0)
		ystep *= (1/YScale) ;

	xPosInt = (int) (XPos * (1 << 2));
	xRightPosInt = (int) ((XPos + 16*XScale) * (1 << 2));
	yPosInt = (int) (YPos * (1 << 2));
	yBottomPosInt = (int) ((YPos + 16*YScale) * (1 << 2));



	// detail tile uses first mip-map level tiled 4x4

/*
	gDPLoadTextureBlock_4b((*ppDLP)++,
							  RDP_ADDRESS(pTexture), G_IM_FMT_I,
							  16, 16, 0,
							  G_TX_CLAMP, G_TX_CLAMP,
							  4, 4,
							  G_TX_NOLOD, G_TX_NOLOD);
*/

	// make shadows efficient!!!
	// do this type thing for all on screen draw stuff!

	gDPSetTextureImage((*ppDLP)++, G_IM_FMT_I, G_IM_SIZ_16b, 1, RDP_ADDRESS(pTexture));
	gSPDisplayList((*ppDLP)++,	RSP_ADDRESS(font_texture_load_16x16_dl));

   gSPTextureRectangle((*ppDLP)++,
							  xPosInt, yPosInt,
							  xRightPosInt, yBottomPosInt,
							  G_TX_RENDERTILE,
							  0, 0, xstep, ystep);
}

void COnScreen__DrawSmallFontTexture(Gfx **ppDLP,
										void *pTexture,
										float XPos, float YPos,
										float XScale, float YScale)
{
	int	xstep ;
	int	ystep ;

	int xPosInt, xRightPosInt;
	int yPosInt, yBottomPosInt;

	xstep = (1<<10) ;
	if (XScale != 0.0)
		xstep *= (1/XScale) ;
	ystep = (1<<10) ;
	if (YScale != 0.0)
		ystep *= (1/YScale) ;

	xPosInt = (int) (XPos * (1 << 2));
	xRightPosInt = (int) ((XPos + 16*XScale) * (1 << 2));
	yPosInt = (int) (YPos * (1 << 2));
	yBottomPosInt = (int) ((YPos + 8*YScale) * (1 << 2));

	gDPLoadTextureBlock_4b((*ppDLP)++,
							  RDP_ADDRESS(pTexture), G_IM_FMT_I,
							  16, 8, 0,
							  G_TX_CLAMP, G_TX_CLAMP,
							  4, 3,
							  G_TX_NOLOD, G_TX_NOLOD);
   gSPTextureRectangle((*ppDLP)++,
							  xPosInt, yPosInt,
							  xRightPosInt, yBottomPosInt,
							  G_TX_RENDERTILE,
							  0, 0, xstep, ystep);
}






// --------------------------------------------------------------------------
// write string in current format
// --------------------------------------------------------------------------
void COnScreen__WriteText(Gfx **ppDLP, char *String, float nX, float nY, float XScale, float YScale)
{
	int	c ;
	CGridGraphic	*font ;

	while (*String)
	{
		c = *String ;

		if (c>=0x30 && c<=0x39)
			COnScreen__DrawFontTexture(ppDLP, (CGridGraphic *)pNumbers4Bit[c-0x30], nX, nY, XScale, YScale) ;
		else
		{
			font = NULL ;

			if (c == '.')
				font = (CGridGraphic *)FontPeriod ;
			else if (c == ',')
				font = (CGridGraphic *)FontComma ;
			else if (c == '(')
				font = (CGridGraphic *)FontQuoteL ;
			else if (c == ')')
				font = (CGridGraphic *)FontQuoteR ;
			else if (c == '@')
				font = (CGridGraphic *)FontCopyRight ;
			else if (c == '^')
				font = (CGridGraphic *)FontLittleC ;
			else if (c == '#')
				font = (CGridGraphic *)FontTrademark ;
#ifdef GERMAN
			else if (c == '<')
				font = (CGridGraphic *)FontGermanA ;
			else if (c == '>')
				font = (CGridGraphic *)FontGermanO ;
			else if (c == '/')
				font = (CGridGraphic *)FontGermanU ;
#endif
			else
			{
				c &= 0x1f ;
				if (c)
					font = (CGridGraphic *)pFont[c-1] ;
			}

			if (font)
				COnScreen__DrawFontTexture(ppDLP, font, nX, nY, XScale, YScale) ;
		}
  		nX += (12 *XScale);

		String++ ;
	}
}

void COnScreen__WriteKanjiText(Gfx **ppDLP, u8 *String, float nX, float nY, float XScale, float YScale)
{
#ifdef KANJI
	int	c ;
	CGridGraphic	*font ;

	while (*String != 255)
	{
		c = *String ;

		if (c != 0xeb)
		{
			font = (CGridGraphic *)pKanjiFont[c] ;

			if (font)
				COnScreen__DrawFontTexture(ppDLP, font, nX, nY, XScale, YScale) ;
		}
  		nX += (16 *XScale);

		String++ ;
	}
#else
	ASSERT(FALSE);
#endif
}


void COnScreen__WriteSmallText(Gfx **ppDLP, char *String, float nX, float nY, float XScale, float YScale)
{
	int	c ;

	while (*String)
	{
		c = *String ;

		if (FontType == SMALL_FONT)
		{
			if (c==' ')
				c = 0;
			else if (c>=0x30 && c<=0x39)
				c -= 0x20 ;
			else if ((c>='a' && c<='z') || (c>='A' && c<='Z'))
				c = (c &0x1f) + 0x19 ;
			else
			{
				switch (c)
				{
					case '!':	c = 0x34 ; break ;
					case '"':	c = 0x35 ; break ;
					case '#':	c = 0x36 ; break ;
					case 0x27:	c = 0x37 ; break ;	// '
					case '*':	c = 0x38 ; break ;
					case '+':	c = 0x39 ; break ;
					case ',':	c = 0x3a ; break ;
					case '-':	c = 0x3b ; break ;
					case '.':	c = 0x3c ; break ;
					case '/':	c = 0x3d ; break ;
					case ':':	c = 0x3e ; break ;
					case '=':	c = 0x3f ; break ;
					case '?':	c = 0x40 ; break ;
					case '@':	c = 0x41 ; break ;
					case '(':	c = 0x0d ; break ;
					case ')':	c = 0x0e ; break ;
					case '{':	c = 0x0b ; break ;
					case '}':	c = 0x0c ; break ;
					case '&':	c = 0x0a ; break ;


					default:		c = 0 ;

				}
			}
			if (c)
				COnScreen__DrawSmallFontTexture(ppDLP, &SmallFont[c*64], nX, nY, XScale, YScale) ;
		}
		// nintendo format
		else
		{
			if ((c > 0x09) && (c!=15))
				COnScreen__DrawSmallFontTexture(ppDLP, &SmallFont[c*64], nX, nY, XScale, YScale) ;
		}

  		nX += (8 *XScale);

		String++ ;
	}
}



// --------------------------------------------------------------------------
// Draw a string with drop shadow, in the desired colors
// --------------------------------------------------------------------------
void COnScreen__DrawText(Gfx **ppDLP,
								 char *String,
								 int nX, int nY,
								 int Opacity, BOOL centre, BOOL shadow)
{
	float	x, y ;
	float	len ;
	char	*Source ;

	u8	*string ;

	COnScreen	*pThis = &GetApp()->m_OnScreen ;

	if (centre)
	{
		if (FontType == LARGE_FONT)
			len = strlen(String) *(12*pThis->m_FontXScale) ;
		else if (FontType == KANJI_FONT)
		{
			string = String ;
			len = 0 ;
			while (*string++ != 255)
				len++ ;
			len *= (16*pThis->m_FontXScale) ;
		}
		else
			len = strlen(String) *(8*pThis->m_FontXScale) ;
		nX -= len /2 ;
	}

	x = nX ;
	y = nY ;
	Source = String ;

	if (shadow)
	{
		// ---------------------- draw shadow
		// top color
		gDPSetEnvColor((*ppDLP)++,
							pThis->m_TopR/4, pThis->m_TopG/4, pThis->m_TopB/4,						// rgb
							Opacity);	// a

		// bottom
		gDPSetPrimColor((*ppDLP)++,
							 0, 0,
							 pThis->m_BottomR/4, pThis->m_BottomG/4, pThis->m_BottomB/4,		// rgb
							 255);		// unused a

		nX += GetApp()->m_OnScreen.m_ShadowXOff ;
		nY += GetApp()->m_OnScreen.m_ShadowYOff ;
		if (FontType == LARGE_FONT)
			COnScreen__WriteText(ppDLP, String, nX, nY, pThis->m_FontXScale, pThis->m_FontYScale) ;
		else if (FontType == KANJI_FONT)
			COnScreen__WriteKanjiText(ppDLP, String, nX, nY, pThis->m_FontXScale, pThis->m_FontYScale) ;
		else
			COnScreen__WriteSmallText(ppDLP, String, nX, nY, pThis->m_FontXScale, pThis->m_FontYScale) ;
	}

	// ---------------------- draw normal text
	// top color
	gDPSetEnvColor((*ppDLP)++,
						pThis->m_TopR, pThis->m_TopG, pThis->m_TopB,						// rgb
						Opacity);	// a

	// bottom
	gDPSetPrimColor((*ppDLP)++,
						 0, 0,
						 pThis->m_BottomR, pThis->m_BottomG, pThis->m_BottomB,		// rgb
						 255);		// unused a

	String = Source ;
	nX = x ;
	nY = y ;
	if (FontType == LARGE_FONT)
		COnScreen__WriteText(ppDLP, String, nX, nY, pThis->m_FontXScale, pThis->m_FontYScale) ;
	else if (FontType == KANJI_FONT)
		COnScreen__WriteKanjiText(ppDLP, String, nX, nY, pThis->m_FontXScale, pThis->m_FontYScale) ;
	else
		COnScreen__WriteSmallText(ppDLP, String, nX, nY, pThis->m_FontXScale, pThis->m_FontYScale) ;
}

// -------------------------------------------------------------------------------------------------------------
//
// Prepare for 16bit texture 4bit opacity draw
//
// --------------------------------------------------------------------------------------------------------------
void COnScreen__Init16BitDraw(Gfx **ppDLP, int opacity)
{
	gDPPipeSync((*ppDLP)++);
	gDPSetCycleType((*ppDLP)++, G_CYC_2CYCLE);

	CGeometry__SetCombineMode(ppDLP, G_CC_16RGBA_4A__G_CC_PASS2);
	CGeometry__SetRenderMode(ppDLP, G_RM_PASS__G_RM_XLU_SURF2);

  gDPSetEnvColor((*ppDLP)++,
						 255, 255, 255,	// rgb
						 opacity);			// a

	gSPTexture((*ppDLP)++,
				  0x8000,
				  0x8000,
				  0, 0, G_ON);
	gDPSetTextureLUT((*ppDLP)++, G_TT_NONE);

	gDPSetTexturePersp((*ppDLP)++, G_TP_NONE);
	gDPSetTextureFilter((*ppDLP)++, G_TF_POINT);
}

// --------------------------------------------------------------------------
// Draw 16bit texture 4bit opacity
// --------------------------------------------------------------------------
void COnScreen__Draw16BitTexture(Gfx **ppDLP,
										void *pTexture,
										void *pOpacity,
										int XPos, int YPos,
										int TextureWidth, int TextureHeight)
{
//	if (((DWORD) pTexture) & 7)
//		rmonPrintf("pTexture:%x\n", pTexture);
//	if (((DWORD) pOpacity) & 7)
//	{
//		rmonPrintf("pOpacity:%x\n", pOpacity);
//		rmonPrintf("width:%d height:%d\n", TextureWidth, TextureHeight);
//	}

	ASSERT(!((DWORD)pTexture & 0x07));
	ASSERT(!((DWORD)pOpacity & 0x07));

	_gDPLoadTextureBlock_4b((*ppDLP)++,
									RDP_ADDRESS(pOpacity), 256, G_IM_FMT_I,
									TextureWidth, TextureHeight, 0,
									G_TX_NOMIRROR, G_TX_NOMIRROR,
								   G_TX_NOMASK, G_TX_NOMASK,
									G_TX_NOLOD, G_TX_NOLOD);

  gDPLoadTextureBlock(((*ppDLP)++),
							  RDP_ADDRESS(pTexture), G_IM_FMT_RGBA, G_IM_SIZ_16b,
							  TextureWidth, TextureHeight, 0,
							  G_TX_NOMIRROR, G_TX_NOMIRROR,
							  G_TX_NOMASK, G_TX_NOMASK,
							  G_TX_NOLOD, G_TX_NOLOD);

	gDPSetTile((*ppDLP)++,
				  G_IM_FMT_I, G_IM_SIZ_4b, (TextureWidth/16), 256, 1,
				  0,
				  0, 0, 0,
				  0, 0, 0);
	gDPSetTileSize((*ppDLP)++,
						1, 0, 0,
						(TextureWidth-1) << 2, (TextureHeight-1) << 2);


	gSPTextureRectangle((*ppDLP)++,
							  (XPos << 2), (YPos << 2),
							  ((XPos + TextureWidth) << 2), ((YPos + TextureHeight) << 2),
							  G_TX_RENDERTILE,
							  0, 0, (1 << 10), (1 << 10));

}

void COnScreen__Draw16BitScaledTexture(Gfx **ppDLP,
												void *pTexture,
												void *pOpacity,
												float XPos, float YPos,
												int TextureWidth, int TextureHeight,
												float XScale, float YScale)
{
	int	xstep ;
	int	ystep ;

	int xPosInt, xRightPosInt;
	int yPosInt, yBottomPosInt;

	xstep = (1<<10) ;
	if (XScale != 0.0)
		xstep /= XScale;
	ystep = (1<<10) ;
	if (YScale != 0.0)
		ystep /= YScale;

	xPosInt = (int) (XPos * (1 << 2));
	xRightPosInt = (int) ((XPos + TextureWidth*XScale) * (1 << 2));
	yPosInt = (int) (YPos * (1 << 2));
	yBottomPosInt = (int) ((YPos + TextureHeight*YScale) * (1 << 2));


	_gDPLoadTextureBlock_4b((*ppDLP)++,
									RDP_ADDRESS(pOpacity), 256, G_IM_FMT_I,
									TextureWidth, TextureHeight, 0,
									G_TX_NOMIRROR, G_TX_NOMIRROR,
								   G_TX_NOMASK, G_TX_NOMASK,
									G_TX_NOLOD, G_TX_NOLOD);

	gDPLoadTextureBlock(((*ppDLP)++),
							  RDP_ADDRESS(pTexture), G_IM_FMT_RGBA, G_IM_SIZ_16b,
							  TextureWidth, TextureHeight, 0,
							  G_TX_NOMIRROR, G_TX_NOMIRROR,
							  G_TX_NOMASK, G_TX_NOMASK,
							  G_TX_NOLOD, G_TX_NOLOD);


	gDPSetTile((*ppDLP)++,
				  G_IM_FMT_I, G_IM_SIZ_4b, (TextureWidth/16), 256, 1,
				  0,
				  0, 0, 0,
				  0, 0, 0);
	gDPSetTileSize((*ppDLP)++,
						1, 0, 0,
						(TextureWidth-1) << 2, (TextureHeight-1) << 2);

   gSPScisTextureRectangle((*ppDLP)++,
							  xPosInt, yPosInt,
							  xRightPosInt, yBottomPosInt,
							  G_TX_RENDERTILE,
							  0, 0, xstep, ystep);
}


// must be 32 wide!!!
void COnScreen__Draw16BitScrollTexture(Gfx **ppDLP,
											void *pTexture,
											void *pOpacity,
											int XPos, int YPos,
											int TextureWidth, int TextureHeight,
											int XOff, int YOff)
{
	_gDPLoadTextureBlock_4b((*ppDLP)++,
									RDP_ADDRESS(pOpacity), 256, G_IM_FMT_I,
									TextureWidth, TextureHeight, 0,
								   G_TX_WRAP, G_TX_WRAP,
								   5, 5,
									G_TX_NOLOD, G_TX_NOLOD);

	gDPLoadTextureBlock(((*ppDLP)++),
							  RDP_ADDRESS(pTexture), G_IM_FMT_RGBA, G_IM_SIZ_16b,
							  TextureWidth, TextureHeight, 0,
							  G_TX_WRAP, G_TX_WRAP,
							  5, 5,
							  G_TX_NOLOD, G_TX_NOLOD);

	gDPSetTile((*ppDLP)++,
				  G_IM_FMT_I, G_IM_SIZ_4b, (TextureWidth/16), 256, 1,
				  0,
				  0, 0, 0,
				  0, 0, 0);
	gDPSetTileSize((*ppDLP)++,
						1, 0, 0,
						(TextureWidth-1) << 2, (TextureHeight-1) << 2);


   gSPTextureRectangle((*ppDLP)++,
							  (XPos << 2), (YPos << 2),
							  ((XPos + (TextureWidth-XOff)) << 2), ((YPos + (TextureHeight-YOff)) << 2),
							  G_TX_RENDERTILE,
							  (XOff<<5), (YOff<<5), (1 << 10), (1 << 10));
}

void COnScreen__Draw16BitClipTexture(Gfx **ppDLP,
											 void *pTexture,
											 void *pOpacity,
											 int XPos, int YPos,
											 int TextureWidth, int TextureHeight,
											 int XSize, int YSize)
{
	_gDPLoadTextureBlock_4b((*ppDLP)++,
									RDP_ADDRESS(pOpacity), 256, G_IM_FMT_I,
									TextureWidth, TextureHeight, 0,
									G_TX_NOMIRROR, G_TX_NOMIRROR,
									G_TX_NOMASK, G_TX_NOMASK,
									G_TX_NOLOD, G_TX_NOLOD);

	gDPLoadTextureBlock(((*ppDLP)++),
							  RDP_ADDRESS(pTexture), G_IM_FMT_RGBA, G_IM_SIZ_16b,
							  TextureWidth, TextureHeight, 0,
							  G_TX_NOMIRROR, G_TX_NOMIRROR,
							  G_TX_NOMASK, G_TX_NOMASK,
							  G_TX_NOLOD, G_TX_NOLOD);

	gDPSetTile((*ppDLP)++,
				  G_IM_FMT_I, G_IM_SIZ_4b, (TextureWidth/16), 256, 1,
				  0,
				  0, 0, 0,
				  0, 0, 0);
	gDPSetTileSize((*ppDLP)++,
						1, 0, 0,
						(TextureWidth-1) << 2, (TextureHeight-1) << 2);



   gSPTextureRectangle((*ppDLP)++,
							  (XPos << 2), (YPos << 2),
							  ((XPos + XSize) << 2), ((YPos + YSize) << 2),
							  G_TX_RENDERTILE,
							  0, 0, (1 << 10), (1 << 10));
}


// --------------------------------------------------------------------------
// Draw 16bit graphic blocks
// --------------------------------------------------------------------------
void COnScreen__Draw16BitGraphic(Gfx **ppDLP, C16BitGraphic *Graphic, INT16 X, INT16 Y)
{
	UINT32 		curx, XCount, YCount ;
	UINT32		width, height ;
	void			*image, *opacity ;
	UINT8			*Part ;

	Part = (UINT8 *)Graphic->m_pPart ;
	YCount = Graphic->m_BlocksDown;
	while(YCount--)
	{
		XCount = Graphic->m_BlocksAcross ;

		curx = X ;
		while(XCount--)
		{
			width = ((C16BitPart *)Part)->m_BlockWidth ;
			height = ((C16BitPart *)Part)->m_BlockHeight ;
		//rmonPrintf("width %08x height %08x\n", width, height) ;
		//rmonPrintf("Part:%08x\n", Part);
		//rmonPrintf("m_pData:%08x\n", ((C16BitPart*) Part)->m_pData);

			image = ((C16BitPart *)Part)->m_pData ;
			opacity = ((C16BitPart *)Part)->m_pData + (width*height*2);
			COnScreen__Draw16BitTexture(ppDLP, image, opacity,
												curx, Y,
												width, height) ;

			curx += width ;
			Part += (width*height*2) + (width*height/2) ;

			// skip 2 long header
			Part += 8 ;
		}
		Y += height ;
	}
}

void COnScreen__Draw16BitScaledGraphic(Gfx **ppDLP, C16BitGraphic *Graphic, float X, float Y, float XScale, float YScale)
{
	float 		curx, XCount, YCount ;
	UINT32		width, height ;
	void			*image, *opacity ;
	UINT8			*Part ;

	Part = (UINT8 *)Graphic->m_pPart ;
	YCount = Graphic->m_BlocksDown;
	while(YCount--)
	{
		XCount = Graphic->m_BlocksAcross ;

		curx = X ;
		while(XCount--)
		{
			width = ((C16BitPart *)Part)->m_BlockWidth ;
			height = ((C16BitPart *)Part)->m_BlockHeight ;
			image = ((C16BitPart *)Part)->m_pData ;
			opacity = ((C16BitPart *)Part)->m_pData + (width*height*2);
			COnScreen__Draw16BitScaledTexture(ppDLP, image, opacity,
														curx, Y,
														width, height,
														XScale, YScale) ;

			curx += width * XScale ;
			Part += (width*height*2) + (width*height/2) ;

			// skip 2 long header
			Part += 8 ;
		}
		Y += height * YScale ;
	}
}

// -------------------------------------------------------------------------------------------------------------
/*

void COnScreen__DrawCompass(UINT16 *pBuffer, COnScreen *pThis)
{
	INT16			x1, y1 ;
	INT16			x2, y2 ;
	INT16			ax1, ay1 ;

	CGameObjectInstance	*pTurok ;
	FLOAT						yrot ;

  	//pFBuffer = CEngineApp__GetFrameData(GetApp())->m_pFrameBuffer ;

	// get a pointer to the player and extract the view direction
	pTurok = CEngineApp__GetPlayer(GetApp());
	yrot = pTurok->m_RotY ;
	x1 = pThis->m_CompassOverlay.m_X+8 ;
	y1 = pThis->m_CompassOverlay.m_Y+8 ;
	x2 = x1 - (20 * sin(yrot)) ;
	y2 = y1 + (20 * cos(yrot)) ;
	COnScreen__DrawFrameBufferLine(pBuffer, x1, y1, x2, y2, (31<<11)|31) ;

	ax1 = x1 - (16 * sin(yrot-.2)) ;
	ay1 = y1 + (16 * cos(yrot-.2)) ;
	COnScreen__DrawFrameBufferLine(pBuffer, ax1, ay1, x2, y2, (31<<11)|31) ;
	ax1 = x1 - (16 * sin(yrot+.2)) ;
	ay1 = y1 + (16 * cos(yrot+.2)) ;
	COnScreen__DrawFrameBufferLine(pBuffer, ax1, ay1, x2, y2, (31<<11)|31) ;
}
*/



// -------------------------------------------------------------------------------------------------------------
//
// ONSCREEN TEXT DISPLAY
// featuring 'Carl Slam (tm)'
//
// -------------------------------------------------------------------------------------------------------------

#define MAX_GAMETEXT		8

int			GameTextIndex ;
t_GameText	GameTextArray[MAX_GAMETEXT] ;

// setup array to be clear
void COnScreen__InitializeGameText(void)
{
	int	i ;

	GameTextIndex = 0;
	for (i=0; i<MAX_GAMETEXT; i++)
	{
		GameTextArray[i].Mode = GAMETEXT_NULL ;
		GameTextArray[i].Id = 0 ;
	}
}


// put entry in next position
t_GameText *COnScreen__AddGameText(char *String)
{
	t_GameText	*active ;

	active = &GameTextArray[GameTextIndex++] ;
	GameTextIndex &= (MAX_GAMETEXT-1) ;

	active->Mode = GAMETEXT_FADEUP ;
	active->String = String ;
	active->Alpha = 0 ;
	active->Scale = 1.0 ;
	active->DisplayTime = 1*FRAME_FPS ;
	active->Id = 0 ;

	return active ;
}


t_GameText *COnScreen__AddGameTextForTime(char *String, FLOAT Secs)
{
	t_GameText	*Active = COnScreen__AddGameText(String) ;
	if (Active)
		Active->DisplayTime = Secs*FRAME_FPS ;
	return Active ;
}


t_GameText	*activeID ;

// add an entry only if this id does not exist
t_GameText *COnScreen__AddGameTextWithId(char *String, int id)
{
	if (COnScreen__SearchForId(id) == FALSE)
	{
		t_GameText	*Active = COnScreen__AddGameText(String) ;
		if (Active)
		{
			Active->Id = id ;
		}
		return Active ;
	}
	else
	{
		activeID->Time = 2 ;	//activeID->DisplayTime ;
		activeID->String = String ;
	}
	return NULL ;
}

BOOL COnScreen__SearchForId(int id)
{
	int	i ;
	t_GameText	*active ;


	for (i=0; i<MAX_GAMETEXT; i++)
	{
		active = &GameTextArray[i] ;
		if (active->Mode != GAMETEXT_NULL)
		{
			if (active->Id == id)
			{
				activeID = active ;
				return TRUE ;
			}
		}
	}
	return FALSE ;
}




void COnScreen__UpdateGameText(Gfx **ppDLP)
{
	int	i, index ;
	int	y ;
	t_GameText	*active ;

	// move text up slightly in cinema modes (key cinema!)
	if (CCamera__InCinemaMode(&GetApp()->m_Camera))
		y = 192 - (MAX_GAMETEXT*18);
	else
		y = 200 - (MAX_GAMETEXT*18);

	index = GameTextIndex ;

	// using billinear interpolation for scaled bitmaps
	COnScreen__InitFontDraw(ppDLP) ;
	CGeometry__SetRenderMode(ppDLP, G_RM_XLU_SURF__G_RM_XLU_SURF2);

	for (i=0; i<MAX_GAMETEXT; i++)
	{
		active = &GameTextArray[index] ;
		if (active->Mode != GAMETEXT_NULL)
		{
			switch (active->Mode)
			{
				case GAMETEXT_FADEUP:
					//active->Scale += (1.0/5) ;
					active->Alpha += (255/8) ;
					if (active->Alpha >=255)
					{
						active->Alpha = 255 ;
						active->Mode = GAMETEXT_DISPLAY ;
						//active->Scale = 2.0 ;
						active->Time = active->DisplayTime ;
					}
					active->Scale = sin(active->Alpha * (M_PI/2)/255) ;
					break ;

				case GAMETEXT_DISPLAY:
					//active->Scale -= .3 ;
				  	//if (active->Scale < 1.0)
					active->Scale = 1.0 ;

					active->Time -= frame_increment ;
					if (active->Time <0)
						active->Mode = GAMETEXT_FADEDOWN ;
					break ;

				case GAMETEXT_FADEDOWN:
					active->Scale -= sin(255-(active->Alpha * M_PI/255))/3 ;
					//active->Scale -= .1 ;
					if (active->Scale <0)
						active->Scale = 0 ;
					active->Alpha -= 24 ;
					if (active->Alpha <0)
					{
						active->Alpha = 0 ;
						active->Mode = GAMETEXT_NULL ;
						//active->Mode = GAMETEXT_FADEUP ;
					}
					break ;

			}
#ifdef GERMAN
			COnScreen__SetFontScale(active->Scale*.8, active->Scale) ;
#else
			COnScreen__SetFontScale(active->Scale, active->Scale) ;
#endif
			COnScreen__DrawText(ppDLP, active->String, 320/2, y, active->Alpha, TRUE, TRUE) ;
		}

		index++ ;
		index &= (MAX_GAMETEXT-1) ;
		y += 18 ;

	}
}


//-----------------------------------------------------------------------------------------------------
//
// Barrel Scroll o'rama
//
// Spin stuff around a barrel, fading and scaling for distance
//-----------------------------------------------------------------------------------------------------
//#define	BARREL_DISPLAYED		12
//#define	BARREL_STEP				((M_PI*2)/BARREL_DISPLAYED)

void CBarrel__Construct(CBarrel *pThis, int items,
								int x, int y, int xstep, int ystep,
								float speed, int display)
{
	pThis->m_Current = 0 ;				// internally selected item
	pThis->m_Selected = 0 ;				// user chosen item
	pThis->m_CurrentAngle = 0;			// angle of internally chosen item
	pThis->m_SelectedAngle = 0 ;		// angle of chosen item to move to
	pThis->m_delta = 0 ;					// angle difference
	pThis->m_AngleAccel = 0 ;			// acceleration
	pThis->m_ViewAngleOffset = 0 ;	// angle to add to sin position of drawn object to make it spin

	pThis->m_Buffersize = items ;			// items in buffer

	pThis->m_X = x ;
	pThis->m_Y = y ;
	pThis->m_XStep = xstep ;
	pThis->m_YStep = ystep ;
	pThis->m_SpinSpeed = speed ;

	pThis->m_Display = display ;
	pThis->m_DisplayStep = (M_PI*2)/(float)display ;

	pThis->m_Direction = BARREL_UP ;

}

void CBarrel__Update(CBarrel *pThis)
{
	pThis->m_SelectedAngle = pThis->m_Selected * pThis->m_DisplayStep ;
	// if current is different from selected, find angle difference,
	// and interpolate to it.
	if (pThis->m_SelectedAngle != pThis->m_CurrentAngle)
	{
		// get difference in radians
		pThis->m_delta = pThis->m_SelectedAngle - pThis->m_CurrentAngle ;

		// if direction was DOWN, make sure accel correct sign
		if (pThis->m_Direction == BARREL_DOWN)
		{
			while (pThis->m_delta < 0)
				pThis->m_delta += (pThis->m_Buffersize * pThis->m_DisplayStep) ;
		}
		// direction was UP, make sure accel correct sign
		else
		{
			while (pThis->m_delta > 0)
				pThis->m_delta -= (pThis->m_Buffersize * pThis->m_DisplayStep) ;
		}


		pThis->m_AngleAccel = pThis->m_delta / pThis->m_SpinSpeed ;
		pThis->m_CurrentAngle += pThis->m_AngleAccel ;
		pThis->m_ViewAngleOffset += pThis->m_AngleAccel ;

		// normalize rotation (no negatives!)
		if (pThis->m_CurrentAngle < 0)
			pThis->m_CurrentAngle += (pThis->m_Buffersize * pThis->m_DisplayStep) ;
		else if (pThis->m_CurrentAngle > (pThis->m_Buffersize * pThis->m_DisplayStep))
			pThis->m_CurrentAngle -= (pThis->m_Buffersize * pThis->m_DisplayStep) ;


		// reached selected item (or very close to it), stop it
		if (fabs(pThis->m_delta) < 0.01)
		{
			pThis->m_CurrentAngle = pThis->m_SelectedAngle ;
			pThis->m_Current = pThis->m_Selected ;
			pThis->m_AngleAccel = 0;
			pThis->m_ViewAngleOffset = 0;
		}
	}

	// move along array to avoid 'popping'
	while (pThis->m_ViewAngleOffset >= pThis->m_DisplayStep)
	{
		pThis->m_ViewAngleOffset -= pThis->m_DisplayStep ;
		pThis->m_Current++ ;
		if (pThis->m_Current >= pThis->m_Buffersize)
			pThis->m_Current = 0 ;
	}
	while (pThis->m_ViewAngleOffset <= -pThis->m_DisplayStep)
	{
		pThis->m_ViewAngleOffset += pThis->m_DisplayStep ;
		pThis->m_Current-- ;
		if (pThis->m_Current < 0)
			pThis->m_Current = pThis->m_Buffersize-1 ;
	}
}


void CBarrel__Draw(CBarrel *pThis, Gfx **ppDLP)
{
	float	XPos, YPos ;
	int	i, index ;
	float	angle, alpha ;

	// draw list, starting with current
	pThis->InitGraphics(ppDLP) ;

	// item to start with
	i = pThis->m_Display/2 ;
	index = pThis->m_Current ;
	while (i--)
	{
		index-- ;
		if (index <0)
			index += pThis->m_Buffersize ;
	}

//	index = pThis->m_Current -4;
//	if (index <0)
//		index += pThis->m_Buffersize ;

	// check incase not enough items!
//	if (index == -2)
//		index = 0 ;
//	else if ((index <0) || (index > pThis->m_Buffersize-1))
//		index = pThis->m_Buffersize-1 ;

	for (i=0 ; i<pThis->m_Display; i++)
	{
		angle = i*(3.141592654*2) / pThis->m_Display ;
		angle += 3.141592654 ;
		angle -= pThis->m_ViewAngleOffset ;

		XPos = pThis->m_X ;
		YPos = pThis->m_Y ;
		XPos += pThis->m_XStep * cos(angle) ;
		YPos += pThis->m_YStep * sin(angle) ;

		alpha = 128+(127*cos(angle)) ;

		if (alpha >0)
		{
			pThis->DrawItem(ppDLP,
								 index, XPos, YPos,
								 0.6+(0.4*cos(angle)), 0.6+(0.4*cos(angle)),
								 alpha) ;
		}
		index++ ;
		if (index > pThis->m_Buffersize-1)
			index = 0 ;
	}
}


//-----------------------------------------------------------------------------------------------------
// an attempt at a map
//-----------------------------------------------------------------------------------------------------
/*
#define CLIP_LEFT		0
#define CLIP_RIGHT 	319
#define CLIP_TOP		0
#define CLIP_BOTTOM	239
// draw a line into the frame buffer
void COnScreen__DrawFrameBufferLine(UINT16 *pBuffer, INT16 x1, INT16 y1, INT16 x2, INT16 y2, UINT16 Color)
{
	INT32	yadd ;					// add for line bias
	INT32	dx, dy ;					// signed delta x and y
	INT32	i ;						// temporary var
	INT32	error_term ;			// bresnham decision var


	//---	Swap endpoints if necessary to make line go left to right
	if (x2 < x1)
	{
		i = x1 ;
		x1 = x2 ;
		x2 = i ;
		i = y1 ;
		y1 = y2 ;
		y2 = i ;
	}

	//---	Do a Trivial rejection clip test
	if ((x1 > CLIP_RIGHT) || (x2 < CLIP_LEFT))
		return ;
	if (y1 < y2)
	{
		if ((y1 > CLIP_BOTTOM) || (y2 < CLIP_TOP))
			return ;
	}
	else
	{
		if ((y2 > CLIP_BOTTOM) || (y1 < CLIP_TOP))
			return ;
	}

	//---	Get DeltaX & DeltaY for line in current orientation
	dx = x2 - x1 ;
	dy = y2 - y1 ;

	//---	Clip the line against each edge of the screen in turn
	if (x1 < CLIP_LEFT)
	{
		y1 += dy * (CLIP_LEFT - x1) / dx ;
		x1 = CLIP_LEFT ;
	}
	if (y1 < CLIP_TOP)
	{
		x1 += dx * (CLIP_TOP - y1) / dy ;
		y1 = CLIP_TOP ;
	}
	if (y1 > CLIP_BOTTOM)
	{
		x1 += dx * (CLIP_BOTTOM - y1) / dy ;
		y1 = CLIP_BOTTOM ;
	}
	if (x2 > CLIP_RIGHT)
	{
		y2 += dy * (CLIP_RIGHT - x2) / dx ;
		x2 = CLIP_RIGHT ;
	}
	if (y2 < CLIP_TOP)
	{
		x2 += dx * (CLIP_TOP - y2) / dy ;
		y2 = CLIP_TOP ;
	}
	if (y2 > CLIP_BOTTOM)
	{
		x2 += dx * (CLIP_BOTTOM - y2) / dy ;
		y2 = CLIP_BOTTOM ;
	}

	//---	Get new DeltaX & DeltaY for clipped line
	dx = x2 - x1 ;
	dy = y2 - y1 ;

	if ( ((dx <  0) && ((x1 > CLIP_RIGHT) || (x2 < CLIP_LEFT))) ||
		 ((dx >= 0) && ((x2 > CLIP_RIGHT) || (x1 < CLIP_LEFT))) ||
		 ((dy <  0) && ((y1 > CLIP_BOTTOM) || (y2 < CLIP_TOP))) ||
		 ((dy >= 0) && ((y2 > CLIP_BOTTOM) || (y1 < CLIP_TOP))))
		 return ;

	//---	Get modulos & pointers to the first pixels
	pBuffer +=	(y1 * 320) + x1 ;

	if (dy < 0)
	{
		dy = -dy ;
		yadd = -320 ;
	}
	else
		yadd = 320 ;

	if (dx < dy)
	{
		error_term = -dy / 2 ;
		for (i = dy ; i >= 0 ; i--)
		{
			*pBuffer = Color ;
			pBuffer += yadd ;

			error_term += dx ;

			if (error_term > 0)
			{
				error_term -= dy ;
				pBuffer++ ;
			}
		}
	}
	else
	{
		error_term = -dx / 2 ;
		for (i = dx ; i >= 0 ; i--)
		{
			*pBuffer++ = Color ;

			error_term += dy ;

			if (error_term > 0)
			{
				error_term -= dx ;
				pBuffer += yadd ;
			}
		}
	}
}


#define LINESCALE		10
#define MIDX			(SCREEN_WD/2)
#define MIDY			(SCREEN_HT/2)

void COnScreen__DrawMap(UINT16 *framebuffer)
{
	CGameRegion		*regions, *pRegion;
	int				cRegion, nRegions;
	CIndexedSet		isCollision;
	CUnindexedSet	usRegions;
	void				*pbRegions;

	int				x1, y1,
						x2, y2,
						cx, cy ;
	CGameObjectInstance	*pTurok ;
	FLOAT						yrot ;


	if (!cache_is_valid)
		return ;

	// get a pointer to the player and extract the view direction
	pTurok = CEngineApp__GetPlayer(GetApp());
	cx = pTurok->ah.ih.m_vPos.x ;
	cy = pTurok->ah.ih.m_vPos.z ;


	// use CCacheEntry__GetDataPtr() if accessing cache in higher priority thread
	CIndexedSet__ConstructFromRawData(&isCollision,
												 //CCacheEntry__GetData(GetApp()->m_Scene.m_pceCollision),
												 CCacheEntry__GetDataPtr(GetApp()->m_Scene.m_pceCollision),
												 FALSE);

	pbRegions = CIndexedSet__GetBlock(&isCollision, CART_COLLISION_usRegions);

	CUnindexedSet__ConstructFromRawData(&usRegions, pbRegions, FALSE);

	regions = (CGameRegion*) CUnindexedSet__GetBasePtr(&usRegions);
	nRegions = CUnindexedSet__GetBlockCount(&usRegions);

	for (cRegion=0; cRegion<nRegions; cRegion++)
	{
		pRegion = &regions[cRegion];

		if ((!pRegion->m_pNeighbors[0]) || (pRegion->m_pNeighbors[0]->m_nRegionSet != pRegion->m_nRegionSet))
		{
			x1 = (pRegion->m_pCorners[0]->m_vCorner.x - cx) / LINESCALE;
			y1 = (cy - pRegion->m_pCorners[0]->m_vCorner.z) / LINESCALE;
			x2 = (pRegion->m_pCorners[1]->m_vCorner.x - cx) / LINESCALE;
			y2 = (cy - pRegion->m_pCorners[1]->m_vCorner.z)/ LINESCALE;
			COnScreen__DrawFrameBufferLine(framebuffer, x1+MIDX, y1+MIDY, x2+MIDX, y2+MIDY, 31) ;
		}
		if ((!pRegion->m_pNeighbors[1]) || (pRegion->m_pNeighbors[1]->m_nRegionSet != pRegion->m_nRegionSet))
		{
			x1 = (pRegion->m_pCorners[1]->m_vCorner.x - cx) / LINESCALE;
			y1 = (cy - pRegion->m_pCorners[1]->m_vCorner.z) / LINESCALE;
			x2 = (pRegion->m_pCorners[2]->m_vCorner.x - cx) / LINESCALE;
			y2 = (cy - pRegion->m_pCorners[2]->m_vCorner.z)/ LINESCALE;
			COnScreen__DrawFrameBufferLine(framebuffer, x1+MIDX, y1+MIDY, x2+MIDX, y2+MIDY, 31) ;
		}
		if ((!pRegion->m_pNeighbors[2]) || (pRegion->m_pNeighbors[2]->m_nRegionSet != pRegion->m_nRegionSet))
		{
			x1 = (pRegion->m_pCorners[2]->m_vCorner.x - cx) / LINESCALE;
			y1 = (cy - pRegion->m_pCorners[2]->m_vCorner.z) / LINESCALE;
			x2 = (pRegion->m_pCorners[0]->m_vCorner.x - cx) / LINESCALE;
			y2 = (cy - pRegion->m_pCorners[0]->m_vCorner.z)/ LINESCALE;
			COnScreen__DrawFrameBufferLine(framebuffer, x1+MIDX, y1+MIDY, x2+MIDX, y2+MIDY, 31) ;
		}
	}

	CUnindexedSet__Destruct(&usRegions);
	CIndexedSet__Destruct(&isCollision);


	// draw a pointer in direction player facing
	yrot = pTurok->m_RotY ;

	x1 = (8 * sin(yrot-.3)) ;
	y1 = -(8 * cos(yrot-.3)) ;
	COnScreen__DrawFrameBufferLine(framebuffer, MIDX, MIDY, x1+MIDX, y1+MIDY, (31<<11)|31) ;
	x1 = (8 * sin(yrot+.3)) ;
	y1 = -(8 * cos(yrot+.3)) ;
	COnScreen__DrawFrameBufferLine(framebuffer, MIDX, MIDY, x1+MIDX, y1+MIDY, (31<<11)|31) ;


}
*/
