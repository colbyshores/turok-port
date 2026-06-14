// Cheats.Cpp

#include "cppu64.h"
#include "tengine.h"
#include "cheats.h"
#include "gfx16bit.h"
#include "tmove.h"
#include "bossflgs.h"


/////////////////////////////////////////////////////////////////////////////
// Constants
/////////////////////////////////////////////////////////////////////////////
// Position and dimentions of cheat screen
#define CHEAT_WIDTH	(216)
#define CHEAT_X		((320/2) - (CHEAT_WIDTH/2))

// Position of menu selection 1!
#define CHEATMENU_Y				(7)



// Position and dimentions of cheat screen
#define GINST_WIDTH	(216+24)
#define GINST_HEIGHT	(180)
#define GINST_X		((320/2) - (GINST_WIDTH/2))
#define GINST_Y		((240/2) - (GINST_HEIGHT/2))

// Position of menu selection 1!
#define GINST_INSTY				(GINST_Y+36)
#define GINSTMENU_Y				(GINST_Y+GINST_HEIGHT-28)
#define GINSTMENU_SPACING		15



/////////////////////////////////////////////////////////////////////////////
// Text
/////////////////////////////////////////////////////////////////////////////

#ifdef ENGLISH
// ---------------------- English text ---------------------------
static char	text_invincibility_on[] = {"invincibility on"};
static char	text_invincibility_off[] = {"invincibility off"};

static char	text_spiritmode_on[] = {"spirit mode on"};
static char	text_spiritmode_off[] = {"spirit mode off"};

static char	text_allweapons_on[] = {"all weapons on"};
static char	text_allweapons_off[] = {"all weapons off"};

static char	text_unlimitedammo_on[] = {"unlimited ammo on"};
static char	text_unlimitedammo_off[] = {"unlimited ammo off"};

static char	text_bigheads_on[] = {"big head sesh on"};
static char	text_bigheads_off[] = {"big head sesh off"};

static char	text_allmap_on[] = {"all map on"};
static char	text_allmap_off[] = {"all map off"};

//static char	text_showallenemies_on[] = {"show all enemies on"};
//static char	text_showallenemies_off[] = {"show all enemies off"};

static char	text_tinyenemymode_on[] = {"tiny enemy mode on"};
static char	text_tinyenemymode_off[] = {"tiny enemy mode off"};

//static char	text_bigenemymode_on[] = {"big enemy mode on"};
//static char	text_bigenemymode_off[] = {"big enemy mode off"};

static char	text_penandinkmode_on[] = {"pen and ink mode on"};
static char	text_penandinkmode_off[] = {"pen and ink mode off"};

static char	text_purdycolors_on[] = {"purdy colors on"};
static char	text_purdycolors_off[] = {"purdy colors off"};

//static char	text_modifyenemy_on[] = {"modify enemy on"};
//static char	text_modifyenemy_off[] = {"modify enemy off"};

static char	text_infinitelives_on[] = {"infinite lives on"};
static char	text_infinitelives_off[] = {"infinite lives off"};

static char	text_flymode_on[] = {"fly mode on"};
static char	text_flymode_off[] = {"fly mode off"};

static char	text_quackmode_on[] = {"quack mode on"};
static char	text_quackmode_off[] = {"quack mode off"};

static char	text_discomode_on[] = {"disco mode on"};
static char	text_discomode_off[] = {"disco mode off"};


static char	text_warp_level_1[] = {"warp level 1"};
static char	text_warp_level_2[] = {"warp level 2"};
static char	text_warp_level_3[] = {"warp level 3"};
static char	text_warp_level_4[] = {"warp level 4"};
static char	text_warp_level_5[] = {"warp level 5"};
static char	text_warp_level_6[] = {"warp level 6"};
static char	text_warp_level_7[] = {"warp level 7"};
static char	text_warp_level_8[] = {"warp level 8"};
static char	text_warp_longhunter[] = {"warp longhunter"};
static char	text_warp_mantis[] = {"warp mantis"};
static char	text_warp_trex[] = {"warp trex"};
static char	text_warp_campaigner[] = {"warp campaigner"};
static char	text_gallery[] = {"gallery"};
static char	text_show_credits[] = {"show credits"};
static char	text_all_keys[] = {"all keys"};

static char	text_instructions[] = {"instructions"} ;
static char	text_ginst1[] = {"control stick"};
static char	text_ginst1b[] = {"rotates view"};
static char	text_ginst2[] = {"left prev enemy"};
static char	text_ginst3[] = {"right next enemy"};
static char	text_ginst4[] = {"up zooms in"};
static char	text_ginst5[] = {"down zooms out"};

static char	text_exit[] = {"exit"};
static char	text_ok[] = {"ok"};
#endif


#ifdef GERMAN
// ---------------------- German text ---------------------------
static char	text_invincibility_on[] = {"unverwundbarkeit ein"};
static char	text_invincibility_off[] = {"unverwundbarkeit aus"};

static char	text_spiritmode_on[] = {"geistmodus ein"};
static char	text_spiritmode_off[] = {"geistmodus aus"};

static char	text_allweapons_on[] = {"alle waffen ein"};
static char	text_allweapons_off[] = {"alle waffen aus"};

static char	text_unlimitedammo_on[] = {"unbegrenzte munition ein"};
static char	text_unlimitedammo_off[] = {"unbegrenzte munition aus"};

static char	text_bigheads_on[] = {"modus grosse k>pfe ein"};		//o.
static char	text_bigheads_off[] = {"modus grosse k>pfe aus"};		//o.

static char	text_allmap_on[] = {"ganze karte ein"};
static char	text_allmap_off[] = {"ganze karte aus"};

//static char	text_showallenemies_on[] = {"alle gegner zeigen ein"};
//static char	text_showallenemies_off[] = {"alle gegner zeigen aus"};

static char	text_tinyenemymode_on[] = {"modus kleine gegner ein"};
static char	text_tinyenemymode_off[] = {"modus kleine gegner aus"};

//static char	text_bigenemymode_on[] = {"modus grosse gegner ein"};
//static char	text_bigenemymode_off[] = {"modus grosse gegner aus"};

static char	text_penandinkmode_on[] = {"modus feder und tinte ein"};
static char	text_penandinkmode_off[] = {"modus feder und tinte aus"};

static char	text_purdycolors_on[] = {"h/bsche farben ein"};		//u.
static char	text_purdycolors_off[] = {"h/bsche farben aus"};		//u.

//static char	text_modifyenemy_on[] = {"gegner <ndern ein"};		//a.
//static char	text_modifyenemy_off[] = {"gegner <ndern aus"};		//a.

static char	text_infinitelives_on[] = {"unendliche leben ein"};
static char	text_infinitelives_off[] = {"unendliche leben aus"};

static char	text_flymode_on[] = {"flugmodus ein"};
static char	text_flymode_off[] = {"flugmodus aus"};

static char	text_quackmode_on[] = {"quark modus ein"};
static char	text_quackmode_off[] = {"quark modus aus"};

static char	text_discomode_on[] = {"disco modus ein"};
static char	text_discomode_off[] = {"disco modus aus"};

static char	text_warp_level_1[] = {"warp level 1"};
static char	text_warp_level_2[] = {"warp level 2"};
static char	text_warp_level_3[] = {"warp level 3"};
static char	text_warp_level_4[] = {"warp level 4"};
static char	text_warp_level_5[] = {"warp level 5"};
static char	text_warp_level_6[] = {"warp level 6"};
static char	text_warp_level_7[] = {"warp level 7"};
static char	text_warp_level_8[] = {"warp level 8"};
static char	text_warp_longhunter[] = {"warp longhunter"};
static char	text_warp_mantis[] = {"warp anbeterin"};
static char	text_warp_trex[] = {"warp trex"};
static char	text_warp_campaigner[] = {"warp campaigner"};
static char	text_gallery[] = {"galerie"};
static char	text_show_credits[] = {"mitarbeiter"};
static char	text_all_keys[] = {"alle shl/ssel"};		//u.

static char	text_instructions[] = {"anleitung"} ;
static char	text_ginst1[] = {"analog stick"};
static char	text_ginst1b[] = {"dreht sichtfeld"};
static char	text_ginst2[] = {"links zeigt vorigen gegner"};
static char	text_ginst3[] = {"rechts zeigt n<chsten gegner"};		//a.
static char	text_ginst4[] = {"oben vergr>ssert"};					//o.
static char	text_ginst5[] = {"unten verkleinert"};

static char	text_exit[] = {"beenden"};
static char	text_ok[] = {"ok"};
#endif


#ifdef KANJI
// ---------------------- Japanese text ---------------------------
static char	text_invincibility_on[] = {0x1D, 0x12, 0x06, 0x04, 0x27, -1} ;
static char	text_invincibility_off[] = {0x1D, 0x12, 0x06, 0x04, 0x19, -1} ;

static char	text_spiritmode_on[] = {0x82, 0x83, 0x84, 0x04, 0x27, -1};
static char	text_spiritmode_off[] = {0x82, 0x83, 0x84, 0x04, 0x19, -1};

static char	text_allweapons_on[] = {0x04, 0x41, 0x23, 0x02, 0x3B, 0x38, 0x27, 0x04, 0x27, -1};
static char	text_allweapons_off[] = {0x04, 0x41, 0x23, 0x02, 0x3B, 0x38, 0x27, 0x04, 0x19, -1};

static char	text_unlimitedammo_on[] = {0x85, 0x1D, 0x2A, 0x27, 0x04, 0x27, -1};
static char	text_unlimitedammo_off[] = {0x85, 0x1D, 0x2A, 0x27, 0x04, 0x19, -1};

static char	text_bigheads_on[] = {0x2E, 0x05, 0x00, 0x0F, 0x1B, 0x04, 0x27, -1};
static char	text_bigheads_off[] = {0x2E, 0x05, 0x00, 0x0F, 0x1B, 0x04, 0x19, -1};

static char	text_allmap_on[] = {0x19, 0x23, 0x1B, 0x40, 0x36, 0x04, 0x27, -1};
static char	text_allmap_off[] = {0x19, 0x23, 0x1B, 0x40, 0x36, 0x04, 0x19, -1};

//static char	text_showallenemies_on[] = {0x1B, 0x40, 0x36, 0x03, 0x16, 0x1C, 0x41, 0x04, 0x27, -1};
//static char	text_showallenemies_off[] = {0x1B, 0x40, 0x36, 0x03, 0x16, 0x1C, 0x41, 0x04, 0x19, -1};

static char	text_tinyenemymode_on[] = {0x1C, 0x15, 0x03, 0x16, 0x1C, 0x41, 0x04, 0x27, -1};
static char	text_tinyenemymode_off[] = {0x1C, 0x15, 0x03, 0x16, 0x1C, 0x41, 0x04, 0x19, -1};

//static char	text_bigenemymode_on[] = {0x2E, 0x05, 0x03, 0x16, 0x1C, 0x41, 0x04, 0x27, -1};
//static char	text_bigenemymode_off[] = {0x2E, 0x05, 0x03, 0x16, 0x1C, 0x41, 0x04, 0x19, -1};

static char	text_penandinkmode_on[] = {0x12, 0x07, 0x0C, 0x10, 0x3D, 0x41, 0x04, 0x27, -1};
static char	text_penandinkmode_off[] = {0x12, 0x07, 0x0C, 0x10, 0x3D, 0x41, 0x04, 0x19, -1};

static char	text_purdycolors_on[] = {0x05, 0x21, 0x41, 0x09, 0x27, 0x13, 0x25, 0x41, 0x23, 0x04, 0x27, -1};
static char	text_purdycolors_off[] = {0x05, 0x21, 0x41, 0x09, 0x27, 0x13, 0x25, 0x41, 0x23, 0x04, 0x19, -1};

//static char	text_modifyenemy_on[] = {0x03, 0x16, 0x1C, 0x41, 0x1F, 0x2E, 0x3A, 0x19, 0x39, 0x01, 0x04, 0x27, -1};
//static char	text_modifyenemy_off[] = {0x03, 0x16, 0x1C, 0x41, 0x1F, 0x2E, 0x3A, 0x19, 0x39, 0x01, 0x04, 0x19, -1};

static char	text_infinitelives_on[] = {0x2b, 0x27, 0x06, 0x1d, 0x2a, 0x27, 0xeb, 0x04, 0x27, -1};
static char	text_infinitelives_off[] = {0x2b, 0x27, 0x06, 0x1d, 0x2a, 0x27, 0xeb, 0x04, 0x19, -1};

static char	text_flymode_on[] = {0x0C, 0x05, 0x01, 0x1F, 0x41, 0x2F, 0x04, 0x27, -1};
static char	text_flymode_off[] = {0x0C, 0x05, 0x01, 0x1F, 0x41, 0x2F, 0x04, 0x19, -1};

static char	text_quackmode_on[] = {0x35, 0x41, 0x0b, 0x41, 0x1f, 0x41, 0x2f, 0xeb, 0x04, 0x27, -1};
static char	text_quackmode_off[] = {0x35, 0x41, 0x0b, 0x41, 0x1f, 0x41, 0x2f, 0xeb, 0x04, 0x19, -1};

//static char	text_discomode_on[] = {-1};	// ?
//static char	text_discomode_off[] = {-1};	// ?

static char	text_warp_level_1[] = {0x26, 0x41, 0x23, 0x2F, 0x42, -1};
static char	text_warp_level_2[] = {0x26, 0x41, 0x23, 0x2F, 0x43, -1};
static char	text_warp_level_3[] = {0x26, 0x41, 0x23, 0x2F, 0x44, -1};
static char	text_warp_level_4[] = {0x26, 0x41, 0x23, 0x2F, 0x45, -1};
static char	text_warp_level_5[] = {0x26, 0x41, 0x23, 0x2F, 0x46, -1};
static char	text_warp_level_6[] = {0x26, 0x41, 0x23, 0x2F, 0x47, -1};
static char	text_warp_level_7[] = {0x26, 0x41, 0x23, 0x2F, 0x48, -1};
static char	text_warp_level_8[] = {0x26, 0x41, 0x23, 0x2F, 0x49, -1};
static char	text_warp_longhunter[] = {0x18, 0x27, 0x0f, 0x41, -1};
static char	text_warp_mantis[] = {0x1b, 0x27, 0x12, 0x3a, 0x0c, -1};
static char	text_warp_trex[] = {0x24, 0x40, 0x07, 0x0c, -1};
static char	text_warp_campaigner[] = {0x94, 0x95, 0x96, -1};
static char	text_gallery[] = {0x03, 0x16, 0x1C, 0x41, 0x2D, -1} ;
static char	text_show_credits[] = {0x0C, 0x0F, 0x40, 0x19, 0x25, 0x41, 0x23, -1};
static char	text_all_keys[] = {0x04, 0x41, 0x23, 0x82, 0x83, 0x93, -1};

static char	text_instructions[] = {0x86, 0x87, 0x88, -1} ;
static char	text_ginst1[] = {0x44, 0xE5, 0x0C, 0x12, 0x01, 0x40, 0x07, 0x7A, 0x89, 0x8A, -1} ;
static char	text_ginst2[] = {0x8B, 0x8C, 0x33, 0x0F, 0x27, 0x8D, 0x7A, 0x8E, 0x03, 0x16, 0x1C, 0x41, -1} ;
static char	text_ginst3[] = {0x8B, 0x8C, 0x33, 0x0F, 0x27, 0x8F, 0x7A, 0x90, 0x03, 0x16, 0x1C, 0x41, -1} ;
static char	text_ginst4[] = {0x8B, 0x8C, 0x33, 0x0F, 0x27, 0x91, 0x7A, 0x2D, 0x41, 0x1D, 0x01, 0x27, -1} ;
static char	text_ginst5[] = {0x8B, 0x8C, 0x33, 0x0F, 0x27, 0x92, 0x7A, 0x2D, 0x41, 0x1D, 0x00, 0x02, 0x13, -1} ;

static char	text_exit[] = {0x6b, 0x7b, 0x70, -1};
static char	text_ok[] = {0xe8, 0xe6, -1};
#endif


//
// WARNING !!!!!!
// menu text options must appear in same order as cheatselections
//
char *cheat_text[]=
{
	text_invincibility_off,
	text_spiritmode_off,
	text_allweapons_off,
	text_unlimitedammo_off,
	text_infinitelives_off,
	text_all_keys,
	text_allmap_off,
	text_bigheads_off,
//	text_showallenemies_off,
	text_tinyenemymode_off,
//	text_bigenemymode_off,
	text_penandinkmode_off,
	text_purdycolors_off,
#ifndef KANJI
	text_discomode_off,
#endif
	text_quackmode_off,
	text_warp_level_1,
	text_warp_level_2,
	text_warp_level_3,
	text_warp_level_4,
	text_warp_level_5,
	text_warp_level_6,
	text_warp_level_7,
	text_warp_level_8,
	text_warp_longhunter,
	text_warp_mantis,
	text_warp_trex,
	text_warp_campaigner,
	text_gallery,
	text_show_credits,
//	text_modifyenemy_off,
	text_flymode_off,
	text_exit,
} ;
//
//
char *ginst_text[]=
{
	text_ok,
} ;

char cheat_menu_items[CHEAT_END_SELECTION];



int CCheat__GetOptionAmt(CCheat *pThis);

/////////////////////////////////////////////////////////////////////////////
// Functions
/////////////////////////////////////////////////////////////////////////////

void CCheat__Construct(CCheat *pThis)
{
	int	cnt;

	if (pThis->m_Mode == CHEAT_NULL)
		pThis->m_Alpha = 0 ;

	pThis->m_Mode = CHEAT_FADEUP ;
	pThis->b_Active = TRUE ;

/*
GetApp()->m_dwEnabledCheatFlags =		CHEATFLAG_INVINCIBILITY
											  |	CHEATFLAG_SPIRITMODE
											  |	CHEATFLAG_ALLWEAPONS
											  |	CHEATFLAG_UNLIMITEDAMMO
											  |	CHEATFLAG_BIGHEADS
											  |	CHEATFLAG_ALLMAP
											  |	CHEATFLAG_SHOWALLENEMIES
											  |	CHEATFLAG_TINYENEMYMODE
											  |	CHEATFLAG_BIGENEMYMODE
											  |	CHEATFLAG_PENANDINKMODE
											  |	CHEATFLAG_PURDYCOLORS
											  |	CHEATFLAG_WARPLEVEL1
											  |	CHEATFLAG_WARPLEVEL2
											  |	CHEATFLAG_WARPLEVEL3
											  |	CHEATFLAG_WARPLEVEL4
											  |	CHEATFLAG_WARPLEVEL5
											  |	CHEATFLAG_WARPLEVEL6
											  |	CHEATFLAG_WARPLEVEL7
											  |	CHEATFLAG_WARPLEVEL8
											  |	CHEATFLAG_WARPLONGHUNTER
											  |	CHEATFLAG_WARPMANTIS
											  |	CHEATFLAG_WARPTREX
											  |	CHEATFLAG_WARPCAMPAIGNER
											  |	CHEATFLAG_GALLERY
											  |	CHEATFLAG_SHOWCREDITS
											  |	CHEATFLAG_MODIFYENEMY
											  |	CHEATFLAG_ALLKEYS
											  |	CHEATFLAG_INFINITELIVES
											  |	CHEATFLAG_FLYMODE
											  |	CHEATFLAG_QUACKMODE ;

*/
	// reset cheat menu items
	for (cnt=0; cnt<CHEAT_END_SELECTION-1; cnt++)
		cheat_menu_items[cnt] = ((GetApp()->m_dwEnabledCheatFlags & (1<<cnt)) == (1<<cnt));

	// exit option is always available
	cheat_menu_items[CHEAT_EXIT] = 1;

	// setup default selection
	pThis->m_Selection = CHEAT_EXIT;			// position on exit on startup
	pThis->m_RealSelection = CCheat__GetOptionAmt(pThis) - 1;
}


// return no. of options
//
int CCheat__GetOptionAmt(CCheat *pThis)
{
	int	nTotal = 0,
			cnt;


	for (cnt=0; cnt<CHEAT_END_SELECTION; cnt++)
		nTotal += cheat_menu_items[cnt];


	return nTotal;
}



//------------------------------------------------------------------------
// Update selection on cheat screen
//------------------------------------------------------------------------
INT32 CCheat__Update(CCheat *pThis)
{
	INT32						ReturnValue = -1;
	CGameObjectInstance	*pPlayer;


	// do gallery instructions
	if (GetApp()->m_GInst.b_Active)
	{
		CGInst__Update(&GetApp()->m_GInst) ;
		ReturnValue = -1 ;
	}
	else
	{


		// Goto next selection?
		if (CEngineApp__MenuDown(GetApp()))
		{
			BarTimer = 0 ;
			do
			{
				do
				{
					pThis->m_Selection++ ;
					if (pThis->m_Selection >= CHEAT_END_SELECTION)
						pThis->m_Selection = 0 ;

				} while (!cheat_menu_items[pThis->m_Selection]);

				pThis->m_RealSelection++;
				if (pThis->m_RealSelection >= CCheat__GetOptionAmt(pThis))
					pThis->m_RealSelection = 0;

			} while (		(GetApp()->m_Mode == MODE_GAME)
							&& (		(pThis->m_Selection == CHEAT_GALLERY)
									||	(pThis->m_Selection == CHEAT_SHOWCREDITS) ) );
		}

		// Goto previous selection?
		if (CEngineApp__MenuUp(GetApp()))
		{
			BarTimer = 0 ;
			do
			{
				do
				{
					pThis->m_Selection-- ;
					if (pThis->m_Selection < 0)
						pThis->m_Selection = CHEAT_END_SELECTION-1 ;

				} while (!cheat_menu_items[pThis->m_Selection]);

				pThis->m_RealSelection--;
				if (pThis->m_RealSelection < 0)
					pThis->m_RealSelection = CCheat__GetOptionAmt(pThis)-1;

			} while (		(GetApp()->m_Mode == MODE_GAME)
							&& (		(pThis->m_Selection == CHEAT_GALLERY)
									||	(pThis->m_Selection == CHEAT_SHOWCREDITS) ) );
		}


		// Exit if start pressed
		if (CTControl__IsUseMenu(pCTControl))
			ReturnValue = pThis->m_Selection ;

		switch (ReturnValue)
		{
			case CHEAT_INVINCIBILITY:
				ReturnValue = -1 ;
				GetApp()->m_dwCheatFlags ^= CHEATFLAG_INVINCIBILITY;
				break ;

			case	CHEAT_SPIRITMODE:
				ReturnValue = -1 ;
				GetApp()->m_dwCheatFlags ^= CHEATFLAG_SPIRITMODE;
				break ;

			case	CHEAT_ALLWEAPONS:
				ReturnValue = -1 ;
				GetApp()->m_dwCheatFlags ^= CHEATFLAG_ALLWEAPONS;

				if ((GetApp()->m_Mode == MODE_TITLE) && ((GetApp()->m_dwCheatFlags & CHEATFLAG_ALLWEAPONS)==0))
					CTMove__CTMove() ;
				break ;

			case	CHEAT_UNLIMITEDAMMO:
				ReturnValue = -1 ;
				GetApp()->m_dwCheatFlags ^= CHEATFLAG_UNLIMITEDAMMO;
				break ;

			case	CHEAT_BIGHEADS:
				ReturnValue = -1 ;
				GetApp()->m_dwCheatFlags ^= CHEATFLAG_BIGHEADS;
				break ;

			case	CHEAT_ALLMAP:
				ReturnValue = -1 ;
				GetApp()->m_dwCheatFlags ^= CHEATFLAG_ALLMAP;
				break ;

//			case	CHEAT_SHOWALLENEMIES:
//				ReturnValue = -1 ;
//				GetApp()->m_dwCheatFlags ^= CHEATFLAG_SHOWALLENEMIES;
//				break ;

			case	CHEAT_TINYENEMYMODE:
				ReturnValue = -1 ;
				GetApp()->m_dwCheatFlags ^= CHEATFLAG_TINYENEMYMODE;
//				GetApp()->m_dwCheatFlags &= ~CHEATFLAG_BIGENEMYMODE;
				break ;

//			case	CHEAT_BIGENEMYMODE:
//				ReturnValue = -1 ;
//				GetApp()->m_dwCheatFlags ^= CHEATFLAG_BIGENEMYMODE;
//				GetApp()->m_dwCheatFlags &= ~CHEATFLAG_TINYENEMYMODE;
//				break ;

			case	CHEAT_PENANDINKMODE:
				ReturnValue = -1 ;
				GetApp()->m_dwCheatFlags ^= CHEATFLAG_PENANDINKMODE;
				break ;

			case	CHEAT_PURDYCOLORS:
				ReturnValue = -1 ;
				GetApp()->m_dwCheatFlags ^= CHEATFLAG_PURDYCOLORS;
				break ;

			case	CHEAT_WARPLEVEL1:
				ReturnValue = -1 ;
				CEngineApp__NewLife(GetApp()) ;
				GetApp()->m_WarpID = 0;
				CEngineApp__SetupFadeTo(GetApp(), MODE_RESETLEVEL);
				break ;

			case	CHEAT_WARPLEVEL2:
				ReturnValue = -1 ;
				CEngineApp__NewLife(GetApp()) ;
				GetApp()->m_WarpID = 1000;
				CEngineApp__SetupFadeTo(GetApp(), MODE_RESETLEVEL);
				break ;

			case	CHEAT_WARPLEVEL3:
				ReturnValue = -1 ;
				CEngineApp__NewLife(GetApp()) ;
				GetApp()->m_WarpID = 2000;
				CEngineApp__SetupFadeTo(GetApp(), MODE_RESETLEVEL);
				break ;

			case	CHEAT_WARPLEVEL4:
				ReturnValue = -1 ;
				CEngineApp__NewLife(GetApp()) ;
				GetApp()->m_WarpID = 3000;
				CEngineApp__SetupFadeTo(GetApp(), MODE_RESETLEVEL);
				break ;

			case	CHEAT_WARPLEVEL5:
				ReturnValue = -1 ;
				CEngineApp__NewLife(GetApp()) ;
				GetApp()->m_WarpID = 4000;
				CEngineApp__SetupFadeTo(GetApp(), MODE_RESETLEVEL);
				break ;

			case	CHEAT_WARPLEVEL6:
				ReturnValue = -1 ;
				CEngineApp__NewLife(GetApp()) ;
				GetApp()->m_WarpID = 5000;
				CEngineApp__SetupFadeTo(GetApp(), MODE_RESETLEVEL);
				break ;

			case	CHEAT_WARPLEVEL7:
				ReturnValue = -1 ;
				CEngineApp__NewLife(GetApp()) ;
				GetApp()->m_WarpID = 7000;
				CEngineApp__SetupFadeTo(GetApp(), MODE_RESETLEVEL);
				break ;

			case	CHEAT_WARPLEVEL8:
				ReturnValue = -1 ;
				CEngineApp__NewLife(GetApp()) ;
				GetApp()->m_WarpID = 8000;
				CEngineApp__SetupFadeTo(GetApp(), MODE_RESETLEVEL);
				break ;

			case	CHEAT_WARPLONGHUNTER:
				ReturnValue = -1 ;
				CEngineApp__NewLife(GetApp()) ;
				GetApp()->m_WarpID = LONGHUNTER_BOSS_START_WARP_ID;
				GetApp()->m_BossFlags &= ~LONGHUNTER_BOSS_FLAGS ;
				GetApp()->m_BossFlags |= BOSS_FLAG_RESET_LEVEL ;
				CEngineApp__ResetBossVars(GetApp());
				CEngineApp__SetupFadeTo(GetApp(), MODE_RESETLEVEL);
				break ;
			case	CHEAT_WARPMANTIS:
				ReturnValue = -1 ;
				CEngineApp__NewLife(GetApp()) ;
				GetApp()->m_WarpID = MANTIS_BOSS_WARP_ID;
				GetApp()->m_BossFlags &= ~MANTIS_BOSS_FLAGS ;
				GetApp()->m_BossFlags |= BOSS_FLAG_RESET_LEVEL ;
				CEngineApp__ResetBossVars(GetApp());
				CEngineApp__SetupFadeTo(GetApp(), MODE_RESETLEVEL);
				break ;
			case	CHEAT_WARPTREX:
				ReturnValue = -1 ;
				CEngineApp__NewLife(GetApp()) ;
				GetApp()->m_WarpID = TREX_BOSS_WARP_ID;
				GetApp()->m_BossFlags &= ~TREX_BOSS_FLAGS ;
				GetApp()->m_BossFlags |= BOSS_FLAG_RESET_LEVEL ;
				CEngineApp__ResetBossVars(GetApp());
				CEngineApp__SetupFadeTo(GetApp(), MODE_RESETLEVEL);
				break ;
			case	CHEAT_WARPCAMPAIGNER:
				ReturnValue = -1 ;
				CEngineApp__NewLife(GetApp()) ;
				GetApp()->m_WarpID = CAMPAIGNER_BOSS_WARP_ID;
				GetApp()->m_BossFlags &= ~CAMPAIGNER_BOSS_FLAGS ;
				GetApp()->m_BossFlags |= BOSS_FLAG_RESET_LEVEL ;
				CEngineApp__ResetBossVars(GetApp());
				CEngineApp__SetupFadeTo(GetApp(), MODE_RESETLEVEL);
				break ;

			case	CHEAT_GALLERY:
				ReturnValue = -1 ;
				pThis->m_Mode = CHEAT_FADEDOWN ;
				CGInst__Construct(&GetApp()->m_GInst) ;
				GetApp()->m_bGInst = TRUE ;
				break ;

			case	CHEAT_SHOWCREDITS:
				ReturnValue = -1 ;
				pThis->m_Mode = CHEAT_FADEDOWN;
				CCredits__Construct();
				GetApp()->m_WarpID = CREDITS_WARP_ID;
				CEngineApp__SetupFadeTo(GetApp(), MODE_RESETLEVEL);
				break ;

//			case	CHEAT_MODIFYENEMY:
//				ReturnValue = -1 ;
//				GetApp()->m_dwCheatFlags ^= CHEATFLAG_MODIFYENEMY;
//				break ;

			case	CHEAT_ALLKEYS:
				CTurokMovement.Level2Keys = 7 ;
				CTurokMovement.Level3Keys = 7 ;
				CTurokMovement.Level4Keys = 7 ;
				CTurokMovement.Level5Keys = 7 ;
				CTurokMovement.Level6Keys = 7 ;
				CTurokMovement.Level7Keys = 7 ;
				CTurokMovement.Level8Keys = 31 ;
				ReturnValue = -1 ;
				GetApp()->m_WarpID = 666;	// Hub warp
				CEngineApp__SetupFadeTo(GetApp(), MODE_RESETLEVEL);
				break ;

			case	CHEAT_INFINITELIVES:
				ReturnValue = -1 ;
				GetApp()->m_dwCheatFlags ^= CHEATFLAG_INFINITELIVES ;
				break ;


			case	CHEAT_FLYMODE:
				ReturnValue = -1 ;
				GetApp()->m_dwCheatFlags ^= CHEATFLAG_FLYMODE;

				// exiting fly mode - find nearest region
				if (!(GetApp()->m_dwCheatFlags & CHEATFLAG_FLYMODE))
				{
					pPlayer = CEngineApp__GetPlayer(GetApp());;

					if (pPlayer)
					{
						pPlayer->ah.ih.m_pCurrentRegion = CScene__NearestRegion(&GetApp()->m_Scene, &pPlayer->ah.ih.m_vPos);
					}
				}
				break ;

			case	CHEAT_QUACKMODE:
				ReturnValue = -1 ;
				GetApp()->m_dwCheatFlags ^= CHEATFLAG_QUACKMODE;
				break ;

#ifndef KANJI
			case	CHEAT_DISCO:
				ReturnValue = -1 ;
				GetApp()->m_dwCheatFlags ^= CHEATFLAG_DISCO;

				// Reset lights if turning off
				if (!(GetApp()->m_dwCheatFlags & CHEATFLAG_DISCO))
					CScene__SetupRealTimeLight(&GetApp()->m_Scene) ;

				break ;
#endif

			case CHEAT_EXIT:
				pThis->m_Mode = CHEAT_FADEDOWN ;
				break ;
		}
	}

	return ReturnValue ;
}


//------------------------------------------------------------------------
// Draw cheat screen information
//------------------------------------------------------------------------
void CCheat__Draw(CCheat *pThis, Gfx **ppDLP)
{
	int		i,
				yStart;

	int		amount ;
	int		CHEAT_Y,
				CHEAT_HEIGHT ;
	int		CHEATMENU_SPACING ;
	float		CHEAT_SCALE ;


	// update fade status
	switch (pThis->m_Mode)
	{
		case CHEAT_FADEUP:
			pThis->m_Alpha += .2 * frame_increment ;
			if (pThis->m_Alpha >= 1.0)
			{
				pThis->m_Alpha = 1.0 ;
				pThis->m_Mode = CHEAT_NORMAL ;
			}
			break ;

		case CHEAT_FADEDOWN:
			pThis->m_Alpha -= .2 * frame_increment ;
			if (pThis->m_Alpha <= 0.0)
			{
				pThis->m_Alpha = 0.0 ;
				pThis->m_Mode = CHEAT_NULL ;
				if (pThis->b_Active == FALSE)
					GetApp()->m_bCheat = FALSE ;
			}
			break ;
	}


	// create a proper scale and size for cheat box
	CHEATMENU_SPACING = 18 ;
	CHEAT_SCALE = 1 ;
	amount = CCheat__GetOptionAmt(pThis) ;
	CHEAT_HEIGHT = amount * CHEATMENU_SPACING ;
	CHEAT_HEIGHT += (CHEATMENU_Y*2) ;

	// temptemptemptemptemptemptemptemp
	//CTurokMovement.Tokens = amount ;

	// time to start scaling
	if (amount > 12)
	{
		switch (amount)
		{
			case	13:		CHEATMENU_SPACING = 16 ;		break ;
			case	14:		CHEATMENU_SPACING = 15 ;		break ;
			case	15:		CHEATMENU_SPACING = 14 ;		break ;
			case	16:		CHEATMENU_SPACING = 13 ;		break ;
			case	17:
			case	18:		CHEATMENU_SPACING = 12 ;		break ;
			case	19:		CHEATMENU_SPACING = 11 ;		break ;
			case	20:
			case	21:		CHEATMENU_SPACING = 10 ;		break ;
			case	22:
			case	23:
			case	24:		CHEATMENU_SPACING = 9 ;			break ;
			case	25:
			case	26:
			case	27:
			case	28:		CHEATMENU_SPACING = 8 ;			break ;
			case	29:
			case	30:		CHEATMENU_SPACING = 7 ;			break ;
 
			default:			CHEATMENU_SPACING = 6 ;			break ;
		}
		CHEAT_HEIGHT = amount * CHEATMENU_SPACING ;
		CHEAT_HEIGHT += (CHEATMENU_Y*2) ;
		CHEAT_SCALE = (float)(CHEATMENU_SPACING+1) / 18.0 ;
	}
/*
	if (CHEAT_HEIGHT > 220)
	{
		CHEATMENU_SPACING = (220-(CHEATMENU_Y*2))/amount ;
		CHEAT_HEIGHT = amount * CHEATMENU_SPACING ;
		CHEAT_HEIGHT += (CHEATMENU_Y*2) ;
		CHEAT_SCALE = (float)CHEATMENU_SPACING / 18 ;
	}
*/

	CHEAT_Y =((240/2) - (CHEAT_HEIGHT/2)) ;

	// prepare to draw boxes
	COnScreen__InitBoxDraw(ppDLP) ;
	COnScreen__DrawHilightBox(ppDLP,
							 CHEAT_X, CHEAT_Y,
							 CHEAT_X+CHEAT_WIDTH, CHEAT_Y+CHEAT_HEIGHT,
							 1, FALSE,
							 0,0,0, 200 * pThis->m_Alpha) ;

	// draw polygon selection bar
	CPause__InitPolygon(ppDLP) ;
	CPause__DrawBar(ppDLP, CHEAT_X+8, CHEAT_Y+CHEATMENU_Y+(pThis->m_RealSelection*CHEATMENU_SPACING),
						CHEAT_WIDTH-16, CHEATMENU_SPACING, pThis->m_Alpha) ;


	// setup pointers to correct on/off text
	if (GetApp()->m_dwCheatFlags & CHEATFLAG_INVINCIBILITY)
		cheat_text[CHEAT_INVINCIBILITY] = text_invincibility_on ;
	else
		cheat_text[CHEAT_INVINCIBILITY] = text_invincibility_off ;

	if (GetApp()->m_dwCheatFlags & CHEATFLAG_SPIRITMODE)
		cheat_text[CHEAT_SPIRITMODE] = text_spiritmode_on ;
	else
		cheat_text[CHEAT_SPIRITMODE] = text_spiritmode_off ;

	if (GetApp()->m_dwCheatFlags & CHEATFLAG_ALLWEAPONS)
		cheat_text[CHEAT_ALLWEAPONS] = text_allweapons_on ;
	else
		cheat_text[CHEAT_ALLWEAPONS] = text_allweapons_off ;

	if (GetApp()->m_dwCheatFlags & CHEATFLAG_UNLIMITEDAMMO)
		cheat_text[CHEAT_UNLIMITEDAMMO] = text_unlimitedammo_on ;
	else
		cheat_text[CHEAT_UNLIMITEDAMMO] = text_unlimitedammo_off ;

	if (GetApp()->m_dwCheatFlags & CHEATFLAG_BIGHEADS)
		cheat_text[CHEAT_BIGHEADS] = text_bigheads_on ;
	else
		cheat_text[CHEAT_BIGHEADS] = text_bigheads_off ;

	if (GetApp()->m_dwCheatFlags & CHEATFLAG_ALLMAP)
		cheat_text[CHEAT_ALLMAP] = text_allmap_on ;
	else
		cheat_text[CHEAT_ALLMAP] = text_allmap_off ;

//	if (GetApp()->m_dwCheatFlags & CHEATFLAG_SHOWALLENEMIES)
//		cheat_text[CHEAT_SHOWALLENEMIES] = text_showallenemies_on ;
//	else
//		cheat_text[CHEAT_SHOWALLENEMIES] = text_showallenemies_off ;

	if (GetApp()->m_dwCheatFlags & CHEATFLAG_TINYENEMYMODE)
		cheat_text[CHEAT_TINYENEMYMODE] = text_tinyenemymode_on ;
	else
		cheat_text[CHEAT_TINYENEMYMODE] = text_tinyenemymode_off ;

//	if (GetApp()->m_dwCheatFlags & CHEATFLAG_BIGENEMYMODE)
//		cheat_text[CHEAT_BIGENEMYMODE] = text_bigenemymode_on ;
//	else
//		cheat_text[CHEAT_BIGENEMYMODE] = text_bigenemymode_off ;

	if (GetApp()->m_dwCheatFlags & CHEATFLAG_PENANDINKMODE)
		cheat_text[CHEAT_PENANDINKMODE] = text_penandinkmode_on ;
	else
		cheat_text[CHEAT_PENANDINKMODE] = text_penandinkmode_off ;

	if (GetApp()->m_dwCheatFlags & CHEATFLAG_PURDYCOLORS)
		cheat_text[CHEAT_PURDYCOLORS] = text_purdycolors_on ;
	else
		cheat_text[CHEAT_PURDYCOLORS] = text_purdycolors_off ;

//	if (GetApp()->m_dwCheatFlags & CHEATFLAG_MODIFYENEMY)
//		cheat_text[CHEAT_MODIFYENEMY] = text_modifyenemy_on ;
//	else
//		cheat_text[CHEAT_MODIFYENEMY] = text_modifyenemy_off ;

	if (GetApp()->m_dwCheatFlags & CHEATFLAG_INFINITELIVES)
		cheat_text[CHEAT_INFINITELIVES] = text_infinitelives_on ;
	else
		cheat_text[CHEAT_INFINITELIVES] = text_infinitelives_off ;

	if (GetApp()->m_dwCheatFlags & CHEATFLAG_FLYMODE)
		cheat_text[CHEAT_FLYMODE] = text_flymode_on ;
	else
		cheat_text[CHEAT_FLYMODE] = text_flymode_off ;

	if (GetApp()->m_dwCheatFlags & CHEATFLAG_QUACKMODE)
		cheat_text[CHEAT_QUACKMODE] = text_quackmode_on ;
	else
		cheat_text[CHEAT_QUACKMODE] = text_quackmode_off ;

#ifndef KANJI
	if (GetApp()->m_dwCheatFlags & CHEATFLAG_DISCO)
		cheat_text[CHEAT_DISCO] = text_discomode_on ;
	else
		cheat_text[CHEAT_DISCO] = text_discomode_off ;
#endif

	// draw each option
	COnScreen__InitFontDraw(ppDLP) ;
	COnScreen__SetFontScale(0.7, CHEAT_SCALE) ;
	COnScreen__SetFontColor(ppDLP, 200, 200, 200, 200, 200, 200) ;
//	COnScreen__SetFontColor(ppDLP, 200, 200, 138, 86, 71,47) ; // default colors (from onscrn.cpp)
	yStart = CHEAT_Y + CHEATMENU_Y;

	for (i=0; i<CHEAT_END_SELECTION; i++)
	{
		if (cheat_menu_items[i])
		{
			COnScreen__DrawText(ppDLP, cheat_text[i], 320/2, yStart, (int)(255 * pThis->m_Alpha), TRUE, FALSE);
			yStart += CHEATMENU_SPACING;
		}
	}
}





/////////////////////////////////////////////////////////////////////////////
// Functions for gallery instructions
/////////////////////////////////////////////////////////////////////////////

void CGInst__Construct(CGInst *pThis)
{
	if (pThis->m_Mode == GINST_NULL)
		pThis->m_Alpha = 0 ;

	pThis->m_Mode = GINST_FADEUP ;
	pThis->b_Active = TRUE ;

	// setup default selection
	pThis->m_Selection = GINST_EXIT;			// position on exit on startup
}


//------------------------------------------------------------------------
// Update selection on gallery instructions screen
//------------------------------------------------------------------------
INT32 CGInst__Update(CGInst *pThis)
{
	// Exit if start pressed
	if (CTControl__IsUseMenu(pCTControl))
	{
		GetApp()->m_WarpID = CHEAT_GALLERY_WARP_ID;
		CEngineApp__SetupFadeTo(GetApp(), MODE_RESETLEVEL);
	}
	return -1;
}


//------------------------------------------------------------------------
// Draw gallery instructions screen information
//------------------------------------------------------------------------
void CGInst__Draw(CGInst *pThis, Gfx **ppDLP)
{
	int		i, y ;
//	float		mod ;

	// update fade status
	switch (pThis->m_Mode)
	{
		case GINST_FADEUP:
			pThis->m_Alpha += .2 * frame_increment ;
			if (pThis->m_Alpha >= 1.0)
			{
				pThis->m_Alpha = 1.0 ;
				pThis->m_Mode = GINST_NORMAL ;
			}
			break ;

		case GINST_FADEDOWN:
			pThis->m_Alpha -= .2 * frame_increment ;
			if (pThis->m_Alpha <= 0.0)
			{
				pThis->m_Alpha = 0.0 ;
				pThis->m_Mode = GINST_NULL ;
				if (pThis->b_Active == FALSE)
					GetApp()->m_bGInst = FALSE ;
			}
			break ;
	}

	// prepare to draw boxes
	COnScreen__InitBoxDraw(ppDLP) ;
	COnScreen__DrawHilightBox(ppDLP,
							 GINST_X, GINST_Y,
							 GINST_X+GINST_WIDTH, GINST_Y+GINST_HEIGHT,
							 1, FALSE,
							 0,0,0, 200 * pThis->m_Alpha) ;

	// draw polygon selection bar
	CPause__InitPolygon(ppDLP) ;
	CPause__DrawBar(ppDLP, GINST_X+8, /*GINST_Y+*/GINSTMENU_Y+(pThis->m_Selection*GINSTMENU_SPACING),
						GINST_WIDTH-16, GINSTMENU_SPACING, pThis->m_Alpha) ;

//	mod = 27.0*cos((FLOAT)pThis->m_SelectionTimer/4.0) ;
//	COnScreen__DrawHilightBox(ppDLP,
//							 GINST_X+8, GINSTMENU_Y+(pThis->m_Selection*GINSTMENU_SPACING),
//							 GINST_X+GINST_WIDTH-8, GINSTMENU_Y+(pThis->m_Selection*GINSTMENU_SPACING)+16,
//							 1, FALSE,
//							 100+mod,0,100+mod, pThis->m_Alpha * 255) ;


	// draw heading
	COnScreen__InitFontDraw(ppDLP) ;
	COnScreen__SetFontScale(1.0, 1.6) ;
	COnScreen__SetFontColor(ppDLP, 200, 0, 200, 0, 200, 0) ;
	COnScreen__DrawText(ppDLP,
							  text_instructions,
							  320/2, GINST_Y + 6,
							  (int)(255 * pThis->m_Alpha), TRUE, TRUE) ;

	// draw instrcutions
#ifndef GERMAN
	COnScreen__SetFontScale(1.0, 1.0) ;
#else
	COnScreen__SetFontScale(0.6, 1.0) ;
#endif
	COnScreen__SetFontColor(ppDLP, 200, 200, 138, 86, 71,47) ; // default colors (from onscrn.cpp)
	y = GINST_INSTY + 16 ;
	COnScreen__DrawText(ppDLP, text_ginst1, 320/2, y, (int)(255 * pThis->m_Alpha), TRUE, TRUE) ;
#ifndef KANJI
	y += 16 ;
	COnScreen__DrawText(ppDLP, text_ginst1b, 320/2, y, (int)(255 * pThis->m_Alpha), TRUE, TRUE) ;
#endif

	y += 16 ;
	COnScreen__DrawText(ppDLP, text_ginst2, 320/2, y, (int)(255 * pThis->m_Alpha), TRUE, TRUE) ;

	y += 16 ;
	COnScreen__DrawText(ppDLP, text_ginst3, 320/2, y, (int)(255 * pThis->m_Alpha), TRUE, TRUE) ;

	y += 16 ;
	COnScreen__DrawText(ppDLP, text_ginst4, 320/2, y, (int)(255 * pThis->m_Alpha), TRUE, TRUE) ;

	y += 16 ;
	COnScreen__DrawText(ppDLP, text_ginst5, 320/2, y, (int)(255 * pThis->m_Alpha), TRUE, TRUE) ;

	// draw each option
	COnScreen__SetFontScale(1.0, 1.0) ;
	COnScreen__SetFontColor(ppDLP, 200, 200, 138, 86, 71,47) ; // default colors (from onscrn.cpp)

	for (i=0; i<GINST_END_SELECTION; i++)
	{
		COnScreen__DrawText(ppDLP,
								  ginst_text[i],
								  320/2, GINSTMENU_Y, (int)(255 * pThis->m_Alpha), TRUE, TRUE);
	}
}


