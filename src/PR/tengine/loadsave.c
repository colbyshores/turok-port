// LoadSave.Cpp

#include "cppu64.h"
#include "tengine.h"
#include "loadsave.h"
#include "tmove.h"
#include "gfx16bit.h"
#include "audio.h"

extern void COnScreen__DrawFontTexture(Gfx **ppDLP,
										void *pTexture,
										float XPos, float YPos,
										float XScale, float YScale) ;

//------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------
// LOAD OPTIONS
#define LOAD_WIDTH	(240)
#define LOAD_HEIGHT	(190)
#define LOAD_X			((320/2) - (LOAD_WIDTH/2))
#define LOAD_Y			((240/2) - (LOAD_HEIGHT/2))


// left hand side box
#define	LOAD_OPTIONSX	20
#define	LOAD_OPTIONSY	64
#define	LOAD_OPTIONSW	120
#define	LOAD_OPTIONSH	150

// right hand side box
#define	LOAD_NAMEX	(LOAD_OPTIONSX+LOAD_OPTIONSW)
#define	LOAD_NAMEY	64
#define	LOAD_NAMEW	(320-LOAD_NAMEX-20)
#define	LOAD_NAMEH	150


// Position of menu selection
#define LOADMENU_Y	 		(LOAD_Y + 48-6)
//#define LOADMENU_PASSWORDY	(LOAD_Y + 64-6)
#define LOADMENU_SPACING	20
//#define LOADPASSWORD_SPACING	20
//#define LOADPASSWORD_SUBWIDTH	24
//#define LOADGRID_X		(LOAD_X + 72)


// heading box
#define	HEADW	(16*14)
#define	HEADH	26
#define	HEADX	((320/2) - (HEADW/2))
#define	HEADY	32




// SAVE OPTIONS
#define SAVE_WIDTH	(220)
#define SAVE_HEIGHT	(140)
#define SAVE_X			((320/2) - (SAVE_WIDTH/2))
#define SAVE_Y			((240/2) - (SAVE_HEIGHT/2))

// left hand side box
#define	SAVE_OPTIONSX	20
#define	SAVE_OPTIONSY	64
#define	SAVE_OPTIONSW	120
#define	SAVE_OPTIONSH	150

// right hand side box
#define	SAVE_NAMEX	(SAVE_OPTIONSX+SAVE_OPTIONSW)
#define	SAVE_NAMEY	64
#define	SAVE_NAMEW	(320-SAVE_NAMEX-20)
#define	SAVE_NAMEH	150



// gamepak manager defines
#define GAMEPAK_WIDTH	(228)
#define GAMEPAK_HEIGHT	(24*8)
#define GAMEPAK_X			((320/2) - (GAMEPAK_WIDTH/2))
#define GAMEPAK_Y			((240/2) - (GAMEPAK_HEIGHT/2))

#define GAMEPAKMENU_Y	(GAMEPAK_Y + 20)



// Position of menu selection
#define SAVEMENU_Y	 		(SAVE_Y + 48)
//#define SAVEMENU_PASSWORDY	(SAVE_Y+SAVE_HEIGHT - 32)
#define SAVEMENU_SPACING	20


int	gamepak_status ;

int	SaveRequestMode ;


//------------------------------------------------------------------------------
// Text
//------------------------------------------------------------------------------
#ifdef ENGLISH
// ---------------------- English text ---------------------------
static char	text_yes[] = {"yes"};
static char	text_no[] = {"no"} ;

static char	text_ok[] = {"ok"} ;
static char	text_enter[] = {"enter"} ;

static char	text_load[] = {"load"} ;
static char	text_exit[] = {"exit"} ;

static char	text_save[] = {"save"} ;
static char	text_delete[] = {"delete"} ;
static char	text_showall[] = {"show all"} ;

static char	text_nocontroller[] = {"no controller"} ;
static char	text_pakfound[] = {"pak found"} ;

static char	text_incompatible[] = {"incompatible"} ;
static char	text_error[] = {"error"} ;
static char	text_novalidfiles[] = {"no valid files"} ;
static char	text_gamenotloaded[] = {"game not loaded"} ;

static char	text_controllerpak[] = {"controller pak"} ;

static char	text_gamesavedpress[] = {"game saved press"} ;
static char	text_buttontoexit[] = {"button to exit"} ;
static char	text_mustsaveover[] = {"must save over"} ;
static char	text_aturokgame[] = {"a turok game"} ;
static char	text_toomanyfiles[] = {"too many files"} ;
static char	text_noroom[] = {"no room"} ;
static char	text_gamenotsaved[] = {"game not saved"} ;
static char	text_cannotdelete[] = {"can not delete"} ;
static char	text_saveover[] = {"save over"} ;
static char	text_existinggame[] = {"existing game"} ;
static char	text_thisgame[] = {"this game"} ;
static char	text_createnewslot[] = {"new note"} ;

static char	text_invincibility[] = {"invincibility"};
static char	text_spiritmode[] = {"spirit mode"};
static char	text_allweapons[] = {"all weapons"};
static char	text_unlimitedammo[] = {"unlimited ammo"};
static char	text_bigheads[] = {"big heads"};
static char	text_allmap[] = {"all map"};
static char	text_showallenemies[] = {"show all enemies"};
static char	text_tinyenemymode[] = {"tiny enemy mode"};
static char	text_bigenemymode[] = {"big enemy mode"};
static char	text_penandink[] = {"pen and ink"};
static char	text_purdycolors[] = {"purdy colors"};
static char	text_levelwarp1[] = {"level warp 1"};
static char	text_levelwarp2[] = {"level warp 2"};
static char	text_levelwarp3[] = {"level warp 3"};
static char	text_levelwarp4[] = {"level warp 4"};
static char	text_levelwarp5[] = {"level warp 5"};
static char	text_levelwarp6[] = {"level warp 6"};
static char	text_levelwarp7[] = {"level warp 7"};
static char	text_levelwarp8[] = {"level warp 8"};
static char	text_gallery[] = {"gallery"};
static char	text_showcredits[] = {"show credits"};
static char	text_infinitelives[] = {"infinite lives"};
static char	text_quackmode[] = {"quack mode"};
static char	text_discomode[] = {"disco mode"};
static char	text_allkeys[] = {"all keys"};
static char	text_longhunterwarp[] = {"longhunter warp"};
static char	text_mantiswarp[] = {"mantis warp"};
static char	text_trexwarp[] = {"trex warp"};
static char	text_campaignerwarp[] = {"campaigner warp"};
static char	text_flymode[] = {"fly mode"};

static char	text_activated[] = {"activated"};
static char	text_invalid[] = {"invalid"};
static char	text_code[] = {"code"};

static char	text_entercheatcode[] = {"enter cheat code"};
static char	text_cheatcode[] = {"cheat code"};

char	text_errorcontroller[] = {"error,controller"} ;
char	text_pakdamaged[] = {"pak damaged."} ;
#endif


#ifdef GERMAN
// ---------------------- German text ---------------------------
static char	text_yes[] = {"ja"};
static char	text_no[] = {"nein"} ;

static char	text_ok[] = {"ok"} ;
static char	text_enter[] = {"eingeben"} ;

static char	text_load[] = {"laden"} ;
static char	text_exit[] = {"beenden"} ;

static char	text_save[] = {"speichern"} ;
static char	text_delete[] = {"l>schen"} ;			//o.
static char	text_showall[] = {"alles zeigen"} ;

static char	text_nocontroller[] = {"kein controller"} ;
static char	text_pakfound[] = {"pak gefunden"} ;

static char	text_incompatible[] = {"nicht kompatibel"} ;
static char	text_error[] = {"fehler"} ;
static char	text_novalidfiles[] = {"keine g/ltigen dateien"} ;		//u.
static char	text_gamenotloaded[] = {"spiel nicht geladen"} ;

static char	text_controllerpak[] = {"controller pak"} ;

static char	text_gamesavedpress[] = {"spiel gespeichert zum"} ;
static char	text_buttontoexit[] = {"verlassen taste dr/cken"} ;		//u.
static char	text_mustsaveover[] = {"ein turok spielstand"} ;
static char	text_aturokgame[] = {"muss /berschrieben werden"} ;		//u.
static char	text_toomanyfiles[] = {"zu viele dateien"} ;
static char	text_noroom[] = {"kein platz"} ;
static char	text_gamenotsaved[] = {"spiel nicht gespeichert"} ;
static char	text_cannotdelete[] = {"l>schen nicht m>glich"} ;		//o.o.
static char	text_saveover[] = {"vorhandenes spiel"} ;
static char	text_existinggame[] = {"/bershreiben"} ;					//u.
static char	text_thisgame[] = {"diesen spielstand"} ;
//static char	text_createnewslot[] = {"neuen spiecherplatz schaffen"} ;
static char	text_createnewslot[] = {"neuen slot schaffen"} ;

static char	text_invincibility[] = {"unverwundbarkeit"};
static char	text_spiritmode[] = {"geistmodus"};
static char	text_allweapons[] = {"alle wafen"};
static char	text_unlimitedammo[] = {"unbergrenzte munition"};
static char	text_bigheads[] = {"grosse k>pfe"};			//o.
static char	text_allmap[] = {"ganze karte"};
static char	text_showallenemies[] = {"alle gegner zeigen"};
static char	text_tinyenemymode[] = {"modus kleine gegner"};
static char	text_bigenemymode[] = {"modus grosse gegner"};
static char	text_penandink[] = {"feder und tinte"};
static char	text_purdycolors[] = {"h/bsche farben"};	//u.
static char	text_levelwarp1[] = {"level warp 1"};
static char	text_levelwarp2[] = {"level warp 2"};
static char	text_levelwarp3[] = {"level warp 3"};
static char	text_levelwarp4[] = {"level warp 4"};
static char	text_levelwarp5[] = {"level warp 5"};
static char	text_levelwarp6[] = {"level warp 6"};
static char	text_levelwarp7[] = {"level warp 7"};
static char	text_levelwarp8[] = {"level warp 8"};
static char	text_gallery[] = {"galerie"};
static char	text_showcredits[] = {"mitarbeiter"};
static char	text_infinitelives[] = {"unendliche leben"};
static char	text_quackmode[] = {"quark modus"};
static char	text_discomode[] = {"disco modus"};
static char	text_allkeys[] = {"alle shl/ssel"};		//u.
static char	text_longhunterwarp[] = {"warp zum longhunter"};
static char	text_mantiswarp[] = {"warp zur anbeterin"};
static char	text_trexwarp[] = {"warp zum trex"};
static char	text_campaignerwarp[] = {"warp zum campaigner"};
static char	text_flymode[] = {"flugmodus"};

static char	text_activated[] = {"aktiviert"};
static char	text_invalid[] = {"ung/ltiger"};			//u.
static char	text_code[] = {"code"};

static char	text_entercheatcode[] = {"cheat code eingeben"};
static char	text_cheatcode[] = {"cheat code"};

char	text_errorcontroller[] = {"fehler, controller"} ;
char	text_pakdamaged[] = {"pak defekt."} ;
#endif


#ifdef KANJI
// ---------------------- Japanese text ---------------------------
static char	text_yes[] = {0x64, 0x4D, -1};
static char	text_no[] = {0x4D, 0x4D, 0x4F, -1} ;

static char	text_ok[] = {0xE8, 0xE6, -1};
static char	text_enter[] = {0xA6, 0xA7, -1};

static char	text_load[] = {0x25, 0x41, 0x2F, -1};
static char	text_exit[] = {0x6B, 0x7B, 0x70, -1};

static char	text_save[] = {0x0D, 0x41, 0x32, -1};
static char	text_delete[] = {0x9E, 0x9F, -1};
static char	text_showall[] = {0x19, 0x39, 0x01, 0x23, 0xA0, 0xA1, -1};

static char	text_nocontroller[] = {0x09, 0x27, 0x13, 0x25, 0x41, 0x21, 0x34, 0x40, 0x07, 0x76, -1};
static char	text_pakfound[] = {0x56, 0x57, 0x55, 0x68, 0x71, 0x5d, 0x4d, 0x68, 0x59, 0x75, -1} ;

static char	text_incompatible[] = {0x03, 0x21, 0x41, -1};
static char	text_error[] = {0x03, 0x21, 0x41, -1} ;
static char	text_novalidfiles[] = {0x2A, 0x41, 0x1D, 0x17, 0x41, 0x13, 0x76, 0x4C, 0x6F, 0x68, 0x59, 0x75, -1} ;
static char	text_gamenotloaded[] = {0x25, 0x41, 0x2F, 0x56, 0x71, 0x68, 0x59, 0x75, 0x7A, 0x57, 0x5A, -1};

static char	text_controllerpak[] = {0x09, 0x27, 0x13, 0x25, 0x41, 0x21, 0x34, 0x40, 0x07, -1};

static char	text_gamesavedpress[] = {0x0D, 0x41, 0x32, 0xA2, 0xA3, -1};
static char	text_buttontoexit[] = {0x0C, 0x0F, 0x41, 0x13, 0x74, 0xA4, 0x57, 0x5D, 0x53, 0x79, 0x56, 0x4d, -1};
static char	text_mustsaveover[] = {0x12, 0x3E, 0x25, 0x40, 0x07, 0x2A, 0x41, 0x1D, 0x63, -1};
static char	text_aturokgame[] = {0x17, 0x41, 0x13, 0x74, 0x51, 0x52, 0x51, 0x4F, 0x70, 0x65, 0x5C, 0x6D, 0x4E, -1};
static char	text_toomanyfiles[] = {0x2A, 0x41, 0x1D, 0x17, 0x41, 0x13, 0x76, 0xA5, 0x58, 0x77, 0x68, 0x58, -1};
static char	text_noroom[] = {0x83, 0x52, 0x37, 0x41, 0x2C, 0x76, 0x4C, 0x6F, 0x68, 0x59, 0x75, -1};
static char	text_gamenotsaved[] = {0x0D, 0x41, 0x32, 0x56, 0x71, 0x68, 0x59, 0x75, 0x7A, 0x57, 0x5A, -1};
static char	text_cannotdelete[] = {0x9E, 0x9F, 0x7A, 0x52, 0x68, 0x59, 0x75, -1};
static char	text_saveover[] = {0x0D, 0x24, 0x07, 0x13, 0x57, 0x5A, 0x17, 0x41, 0x13, 0x74, -1};
static char	text_existinggame[] = {0x51, 0x52, 0x51, 0x4F, 0x68, 0x58, 0x51, -1};
static char	text_thisgame[] = {/*0x9E, 0x9F,*/ 0x57, 0x68, 0x58, 0x51, -1};

static char	text_createnewslot[] = {"new note"} ;

static char	text_invincibility[] = {0x1d, 0x12, 0x06, -1};
static char	text_spiritmode[] = {0x82, 0x83, 0x84, -1};
static char	text_allweapons[] = {0x04, 0x41, 0x23, 0x02, 0x3b, 0x38, 0x27, -1};
static char	text_unlimitedammo[] = {0x85, 0x1d, 0x2a, 0x27, -1};
static char	text_bigheads[] = {0x2e, 0x05, 0x00, 0x0f, 0x1b, -1};
static char	text_allmap[] = {0x19, 0x23, 0x1b, 0x40, 0x36, -1};
static char	text_showallenemies[] = {0x1b, 0x40, 0x36, 0x03, 0x16, 0x1c, 0x41, -1};
static char	text_tinyenemymode[] = {0x1c, 0x15, 0x03, 0x16, 0x1c, 0x41, -1};
static char	text_bigenemymode[] = {0x2e, 0x05, 0x03, 0x16, 0x1c, 0x41, -1};
static char	text_penandink[] = {0x12, 0x07, 0x0c, 0x10, 0x3d, 0x41, -1};
static char	text_purdycolors[] = {0x05, 0x21, 0x41, 0x09, 0x27, 0x13, 0x25, 0x41, 0x23, -1};
static char	text_levelwarp1[] = {0x26, 0x41, 0x23, 0x2F, 0x42, -1};
static char	text_levelwarp2[] = {0x26, 0x41, 0x23, 0x2F, 0x43, -1};
static char	text_levelwarp3[] = {0x26, 0x41, 0x23, 0x2F, 0x44, -1};
static char	text_levelwarp4[] = {0x26, 0x41, 0x23, 0x2F, 0x45, -1};
static char	text_levelwarp5[] = {0x26, 0x41, 0x23, 0x2F, 0x46, -1};
static char	text_levelwarp6[] = {0x26, 0x41, 0x23, 0x2F, 0x47, -1};
static char	text_levelwarp7[] = {0x26, 0x41, 0x23, 0x2F, 0x48, -1};
static char	text_levelwarp8[] = {0x26, 0x41, 0x23, 0x2F, 0x49, -1};
static char	text_gallery[] = {0x03, 0x16, 0x1c, 0x41, 0x2d, -1};
static char	text_showcredits[] = {0x0c, 0x0f, 0x40, 0x19, 0x25, 0x41, 0x23, -1};
static char	text_infinitelives[] = {0x2b, 0x27, 0x06, 0x1d, 0x2a, 0x27, -1};
static char	text_quackmode[] = {0x35, 0x41, 0x0b, 0x41, 0x1f, 0x41, 0x2f, -1};
static char	text_discomode[] = {-1};	// ?
static char	text_allkeys[] = {0x04, 0x41, 0x23, 0x82, 0x83, 0x93, -1};
static char	text_longhunterwarp[] = {0x18, 0x27, 0x0f, 0x41, -1};
static char	text_mantiswarp[] = {0x1b, 0x27, 0x12, 0x3a, 0x0c, -1};
static char	text_trexwarp[] = {0x24, 0x40, 0x07, 0x0c, -1};
static char	text_campaignerwarp[] = {0x94, 0x95, 0x96, -1};
static char	text_flymode[] = {0x0C, 0x05, 0x01, 0x1F, 0x41, 0x2F, -1};

static char	text_activated[] = {0x04, 0x27, -1};
static char	text_invalid[] = {0x34, 0x0c, 0x26, 0x41, 0x2f, 0x76, -1};
static char	text_code[] = {0x5b, 0x76, 0x4d, 0x68, 0x58, -1};

static char	text_entercheatcode[] = {0x34, 0x0C, 0x26, 0x41, 0x2F, 0xA6, 0xA7, -1};
static char	text_cheatcode[] = {0x34, 0x0C, 0x26, 0x41, 0x2F, 0xA6, 0xA7, -1};

// translates as 'controller pak error' (I hope!)
char	text_errorcontroller[] = {0x09, 0x27, 0x13, 0x25, 0x41, 0x21, 0x34, 0x40, 0x07, -1};
char	text_pakdamaged[] = {0x03, 0x21, 0x41, -1} ;
#endif

//------------------------------------------------------------------------------
// Data and Structures
//------------------------------------------------------------------------------
/*
u16	password_data[128] = {0} ;			// used to store hex data
char	password_string[128] = {0};		// used to store converted password as a string of hex
int	password_nibbles = 0; 				// how many entries in password
int	password_nibble = 0; 				// current nibble using
int	password_bit = 0;						// next bit to add
int	password_active = 0;					// set once a password has been setup


char password_entries[16] =
{
	'b', 'c', 'd', 'f',
	'g', 'h', 'j', 'k',
	'l', 'm', 'n', 'p',
	'q', 'r', 's', 't',
} ;


typedef struct s_PasswordEntry
{
	int	items ;
	int	Spacing ;
	char	*String ;
} t_PasswordEntry ;


t_PasswordEntry loadpassword_text[]=
{
	{4, LOADPASSWORD_SPACING-2, "b c d f"},
	{4, LOADPASSWORD_SPACING-2, "g h j k"},
	{4, LOADPASSWORD_SPACING-2, "l m n p"},
	{4, LOADPASSWORD_SPACING-2, "q r s t"},
	{0, LOADPASSWORD_SPACING, "delete"},
	{0, LOADPASSWORD_SPACING, "enter"},
	{0, LOADPASSWORD_SPACING, "exit"},
} ;
*/
char *loadmemory_text[]=
{
	text_load,
	text_exit,
} ;

char *savememory_text[]=
{
	text_save,
	text_delete,
	text_showall,
	text_exit,
} ;

/*
char *savepassword_text[]=
{
	"exit",
} ;
*/


//------------------------------------------------------------------------------
// Load / Save Data
//------------------------------------------------------------------------------
/*
typedef	struct s_PasswordInfo
{
	void *Data ;
	int	Bits ;
} t_PasswordInfo ;

t_PasswordInfo	PasswordVariables[] =
{
	{&CTurokMovement.SemiAutomaticPistolFlag,	1},
	{&CTurokMovement.AssaultRifleFlag,			1},
	{&CTurokMovement.MachineGunFlag,				1},
	{&CTurokMovement.RiotShotgunFlag,			1},
	{&CTurokMovement.AutomaticShotgunFlag,		1},
	{&CTurokMovement.MiniGunFlag,					1},
	{&CTurokMovement.GrenadeLauncherFlag,		1},
	{&CTurokMovement.TechWeapon1Flag,			1},
	{&CTurokMovement.RocketFlag,					1},
	{&CTurokMovement.ShockwaveFlag,				1},
	{&CTurokMovement.TechWeapon2Flag,			1},
	{&CTurokMovement.BackPackFlag,				1},

	{&CTurokMovement.BulletPool,					8},
	{&CTurokMovement.ShotgunPool,					7},
	{&CTurokMovement.EnergyPool,					7},
	{&CTurokMovement.ExpTekBowAmmo,				4},
	{&CTurokMovement.ExpShotgunPool,				5},
	{&CTurokMovement.TekBowAmmo,					5},
	{&CTurokMovement.MiniGunAmmo,					10},
	{&CTurokMovement.GrenadeLauncherAmmo,		5},
	{&CTurokMovement.RocketAmmo,					4},
	{&CTurokMovement.TechWeapon2Ammo,			2},
	{&CTurokMovement.ArmorAmount,					5},

	{&CTurokMovement.Tokens,						7},
	{&CTurokMovement.Lives,							4},

	{&CTurokMovement.Level2Keys,					2},
	{&CTurokMovement.Level3Keys,					2},
	{&CTurokMovement.Level4Keys,					2},
	{&CTurokMovement.Level5Keys,					2},
	{&CTurokMovement.Level6Keys,					2},
	{&CTurokMovement.Level7Keys,					2},
	{&CTurokMovement.Level8Keys,					3},
	{&CTurokMovement.ChronoSceptorPieces,		4},
	//{&CTurokMovement.Level2Access,			 	2},		// presumes access is given
	//{&CTurokMovement.Level3Access,			 	2},		// although may need to touch
	//{&CTurokMovement.Level4Access,			 	2},		// lock to open portal
	//{&CTurokMovement.Level5Access,			 	2},
	//{&CTurokMovement.Level6Access,			 	2},
	//{&CTurokMovement.Level7Access,			 	2},
	//{&CTurokMovement.Level8Access,			 	3},
} ;
*/


//------------------------------------------------------------------------
// Internal prototypes
//------------------------------------------------------------------------
INT32 CLoad__UpdateMemory(CLoad *pThis) ;
//INT32 CLoad__UpdatePassword(CLoad *pThis) ;

void CLoad__DrawMemory(CLoad *pThis, Gfx **ppDLP) ;
//void CLoad__DrawPassword(CLoad *pThis, Gfx **ppDLP) ;

void CLoad__ExtractData(void) ;
void CSave__PrepareData(void) ;

//void CPassword__ClearData(void) ;
//void CPassword__CreateChecksum(void) ;
//BOOL CPassword__CheckChecksum(void) ;
//void CPassword__Create(void) ;
//void CPassword__DecodeToPlayer(void) ;
//void CPassword__ConvertDataToString(void) ;
//void CPassword__ConvertStringToData(void) ;
//void CPassword__DrawString(Gfx **ppDLP, int y, int opacity) ;
//void CPassword__GetData(void *data, int bits) ;

INT32 CSave__UpdateMemory(CSave *pThis) ;
//INT32 CSave__UpdatePassword(CSave *pThis) ;

void CSave__DrawMemory(CSave *pThis, Gfx **ppDLP) ;
//void CSave__DrawPassword(CSave *pThis, Gfx **ppDLP) ;



/*****************************************************************************
*
*	Functions for:		CLoad class
*
******************************************************************************
*
*	Description:	routines to facilitate the loading process
*
*****************************************************************************/

//------------------------------------------------------------------------
// Construct LOAD structure
//------------------------------------------------------------------------
void CLoad__Construct(CLoad *pThis)
{
	int	ret ;

	// Setup default selection
	pThis->m_ASelection = 0 ;
	pThis->m_BSelection = 0 ;
	pThis->m_Side = 1 ;

	pThis->m_FirstVisible = 0 ;
	pThis->m_MaxVisible = 8 ;

	pThis->m_Selection = 0 ;


	if (pThis->m_Mode == LOAD_NULL)
		pThis->m_Alpha = 0 ;


	GetApp()->m_bMessageBox = FALSE ;

	pThis->m_Mode = LOAD_FADEUP ;
	pThis->b_Active = TRUE ;

	// if setting up the file system causes an error, then use password
	ret = SetupFileSystem() ;
//	if (	(ret == PFS_ERR_CONTRFAIL)
//		|| (ret == PFS_ERR_DEVICE)
//		|| (ret == PFS_ERR_ID_FATAL))
	if (ret)// == PFS_ERR_ID_FATAL)
	{
		pThis->m_Mode = LOAD_FADEDOWN ;
		CMessageBox__Construct(&GetApp()->m_MessageBox,
									text_errorcontroller, text_pakdamaged) ;
		GetApp()->m_bMessageBox = TRUE ;
		GetApp()->m_MessageBox.b_Active = FALSE ;
	}
	else if (validpaks == 0)
	{
		pThis->m_Mode = LOAD_FADEDOWN ;
		CMessageBox__Construct(&GetApp()->m_MessageBox,
									text_nocontroller, text_pakfound) ;
		GetApp()->m_bMessageBox = TRUE ;
		GetApp()->m_MessageBox.b_Active = FALSE ;
//		pThis->m_LoadType = LOAD_PASSWORD ;
//		pThis->m_EntryPos = password_nibbles ;
//		if (pThis->m_EntryPos)
//			pThis->m_Selection = LOADPASSWORD_ENTER ;
//		else
//			pThis->m_Selection = LOADPASSWORD_EXIT ;
	}
	else
	{
		pThis->m_LoadType = LOAD_MEMORYCARD ;
		ScanFileList(FALSE) ;
		if (TurokFiles==0)
			pThis->m_BSelection = LOADMEMORY_EXIT ;
	}
}


//------------------------------------------------------------------------
// Update selection on load screen
//------------------------------------------------------------------------
INT32 CLoad__Update(CLoad *pThis)
{
	INT32				ReturnValue = -1 ;

//	if (pThis->m_LoadType == LOAD_MEMORYCARD)
		ReturnValue = CLoad__UpdateMemory(pThis) ;
//	else
//		ReturnValue = CLoad__UpdatePassword(pThis) ;

	return ReturnValue ;
}

//------------------------------------------------------------------------
INT32 CLoad__UpdateMemory(CLoad *pThis)
{
	INT32				ReturnValue = -1 ;
	int				ret ;

	if (GetApp()->m_bMessageBox)
	{
		ReturnValue = CMessageBox__Update(&GetApp()->m_MessageBox) ;
		if (ReturnValue != -1)
		{
			// force exit load and continue pause
			if (GetApp()->m_MessageBox.b_Active == FALSE)
			{
				pThis->m_Mode = LOAD_FADEDOWN ;
				GetApp()->m_Load.b_Active = FALSE ;
				GetApp()->m_Pause.m_Mode = PAUSE_FADEUP ;
			}
			GetApp()->m_MessageBox.m_Mode = MESSAGEBOX_FADEDOWN ;
			GetApp()->m_MessageBox.b_Active = FALSE ;
			ReturnValue = -1 ;
		}
	}

	else
	{
		// switch sides
		if (CEngineApp__MenuLeft(GetApp()))
			pThis->m_Side ^= 1 ;
		else if (CEngineApp__MenuRight(GetApp()))
			pThis->m_Side ^= 1 ;


		// Goto next selection?
		if (CEngineApp__MenuDown(GetApp()))
		{
			// name side
			if (pThis->m_Side == 0)
			{
				BarTimer = 0 ;
				pThis->m_ASelection++ ;
				if (pThis->m_ASelection >= TurokFiles)
					pThis->m_ASelection = 0 ;

				//i = pThis->m_ASelection + 3 ;
				//if (i >= TurokFiles)
				//	i-=TurokFiles ;
				//ReadFileIndex(i) ;

				//LoadBarrel.m_Direction = BARREL_DOWN ;
				//LoadBarrel.m_Selected = pThis->m_Selection ;
			}
			// options side
			else
			{
				BarTimer = 0 ;
				pThis->m_BSelection++ ;
				if (pThis->m_BSelection >= LOADMEMORY_END_SELECTION)
					pThis->m_BSelection = 0 ;
			}
		}

		// Goto previous selection?
		if (CEngineApp__MenuUp(GetApp()))
		{
			// name side
			if (pThis->m_Side == 0)
			{
				BarTimer = 0 ;
				pThis->m_ASelection-- ;
				if (pThis->m_ASelection < 0)
				{
					if (TurokFiles)
						pThis->m_ASelection = TurokFiles-1 ;
					else
						pThis->m_ASelection = 0 ;
				}

				//i = pThis->m_Selection + 3 ;
				//if (i >= TurokFiles)
				//	i-=TurokFiles ;
				//ReadFileIndex(i) ;
			
				//LoadBarrel.m_Direction = BARREL_UP ;
				//LoadBarrel.m_Selected = pThis->m_Selection ;
			}
			// options side
			else
			{
				BarTimer = 0 ;
				pThis->m_BSelection-- ;
				if (pThis->m_BSelection < 0)
					pThis->m_BSelection = LOADMEMORY_END_SELECTION-1 ;
			}
		}

		// Exit if start pressed
		if (CTControl__IsUseMenu(pCTControl))
			ReturnValue = pThis->m_BSelection ;

		switch (ReturnValue)
		{
			case LOADMEMORY_LOAD:
				if (TurokFiles)
				{
					ret = LoadCurrent(pThis->m_ASelection) ;

					switch (ret)
					{
						case 0:
							CTMove__CTMove() ;

							// done in tengine when loaded
							CLoad__ExtractData() ;
							CEngineApp__ResetBossVars(GetApp()) ;

							// force exit pause and goto waitfordisplay....
							pThis->m_Mode = LOAD_FADEDOWN ;
							GetApp()->m_bPause = FALSE ;
							GetApp()->m_Pause.m_Mode = PAUSE_NULL ;

							GetApp()->m_WarpID = CTurokMovement.CurrentCheckpoint ;
							GetApp()->m_UseCinemaWarp = 0 ;

							GetApp()->m_JustLoadedGame = TRUE ;
							CEngineApp__SetupFadeTo(GetApp(), MODE_RESETLEVEL) ;
							break ;

						case -1:
							CMessageBox__Construct(&GetApp()->m_MessageBox,
														text_incompatible, text_gamenotloaded) ;
							GetApp()->m_bMessageBox = TRUE ;
							ReturnValue = -1 ;
							break ;

						default:
							CMessageBox__Construct(&GetApp()->m_MessageBox,
														text_error, text_gamenotloaded) ;
							GetApp()->m_bMessageBox = TRUE ;
							ReturnValue = -1 ;
							break ;
					}
				}
				else
				{
					CMessageBox__Construct(&GetApp()->m_MessageBox,
												text_novalidfiles, text_gamenotloaded) ;
					GetApp()->m_bMessageBox = TRUE ;
					ReturnValue = -1 ;
				}
				ScanFileList(FALSE);
				break ;

			case LOADMEMORY_EXIT:
				pThis->m_Mode = LOAD_FADEDOWN ;
				break ;
		}

			// load game
			//CMessageBox__Construct(&GetApp()->m_MessageBox, pThis->m_pCTControl,
			//								"game loaded", "checksum ok") ;
			//GetApp()->m_bMessageBox = TRUE ;
			//ReturnValue = -1 ;
	}

#if 0
	if (pThis->m_ASelection > (pThis->m_FirstVisible+pThis->m_MaxVisible))
		pThis->m_FirstVisible = pThis->m_ASelection - pThis->m_MaxVisible ;
	if ((pThis->m_FirstVisible+pThis->m_MaxVisible) > (TurokFiles-1))
		pThis->m_FirstVisible = TurokFiles-1 - pThis->m_MaxVisible ;
	if (pThis->m_FirstVisible<0)
		pThis->m_FirstVisible = 0;
	if (pThis->m_ASelection < pThis->m_FirstVisible)
		pThis->m_FirstVisible = pThis->m_ASelection ;
#endif



	return ReturnValue ;
}


//------------------------------------------------------------------------
/*
void CLoad__PasswordUP(CLoad *pThis)
{
	BarTimer = 0 ;
	pThis->m_Selection-- ;
	if (pThis->m_Selection < 0)
		pThis->m_Selection = LOADPASSWORD_END_SELECTION-1 ;

	if (loadpassword_text[pThis->m_Selection].items != 0)
	{
		if (pThis->m_SubSelection >= loadpassword_text[pThis->m_Selection].items)
			pThis->m_SubSelection = loadpassword_text[pThis->m_Selection].items -1;
	}
	else
 		pThis->m_SubSelection = 0 ;
}

void CLoad__PasswordDOWN(CLoad *pThis)
{
	BarTimer = 0 ;
	pThis->m_Selection++ ;
	if (pThis->m_Selection >= LOADPASSWORD_END_SELECTION)
		pThis->m_Selection = 0 ;

	if (loadpassword_text[pThis->m_Selection].items != 0)
	{
		if (pThis->m_SubSelection >= loadpassword_text[pThis->m_Selection].items)
			pThis->m_SubSelection = loadpassword_text[pThis->m_Selection].items -1;
	}
	else
 		pThis->m_SubSelection = 0 ;
}

void CLoad__PasswordLEFT(CLoad *pThis)
{
	BarTimer = 0 ;
	pThis->m_SubSelection-- ;
	if (pThis->m_SubSelection < 0)
	{
		pThis->m_SubSelection = 0 ;

		if (pThis->m_Selection > 0)
		{
			if (loadpassword_text[pThis->m_Selection-1].items)
				pThis->m_SubSelection = loadpassword_text[pThis->m_Selection-1].items-1 ;
			CLoad__PasswordUP(pThis) ;
		}
	}
}

void CLoad__PasswordRIGHT(CLoad *pThis)
{
	BarTimer = 0 ;
	pThis->m_SubSelection++ ;
	if (pThis->m_SubSelection >=  loadpassword_text[pThis->m_Selection].items)
	{
		pThis->m_SubSelection = 0 ;
  		CLoad__PasswordDOWN(pThis) ;
	}
}


INT32 CLoad__UpdatePassword(CLoad *pThis)
{
	INT32				ReturnValue = -1 ;

	if (GetApp()->m_bMessageBox)
	{
		ReturnValue = CMessageBox__Update(&GetApp()->m_MessageBox) ;
		if (ReturnValue != -1)
		{
			GetApp()->m_bMessageBox = FALSE ;
			ReturnValue = -1 ;
		}
	}

	else
	{
		// Goto next selection?
		if (CEngineApp__MenuDown(GetApp()))
			CLoad__PasswordDOWN(pThis) ;

		// Goto previous selection?
		if (CEngineApp__MenuUp(GetApp()))
			CLoad__PasswordUP(pThis) ;


		// any sub items
//		if (loadpassword_text[pThis->m_Selection].items)
//		{
			// Goto left selection?
			if (CEngineApp__MenuLeft(GetApp()))
   			CLoad__PasswordLEFT(pThis) ;

			// Goto right selection?
			if (CEngineApp__MenuRight(GetApp()))
				CLoad__PasswordRIGHT(pThis) ;
//		}



		// Exit if start pressed
		// returns to game when exited
		if (CTControl__IsUseMenu(pCTControl))
		{
			ReturnValue = pThis->m_Selection ;

			//while (pThis->m_EntryPos > 34)
		  	//{
			//	pThis->m_EntryPos-- ;
			//  	password_nibbles-- ;
		  	//}
#define	MAX_PASSWORD		36			// MAKE THIS EVEN!!!
			if (password_nibbles == MAX_PASSWORD)
				pThis->m_EntryPos = password_nibbles-1 ;

			switch (ReturnValue)
			{
				case LOADPASSWORD_GRID1:
					password_string[pThis->m_EntryPos] = 0 + pThis->m_SubSelection ;
					if (password_nibbles < MAX_PASSWORD)
					{
						password_nibbles++ ;
						pThis->m_EntryPos++ ;
					}
					ReturnValue = -1 ;
					break ;
				case LOADPASSWORD_GRID2:
					password_string[pThis->m_EntryPos] = 4 + pThis->m_SubSelection ;
					if (password_nibbles < MAX_PASSWORD)
					{
						password_nibbles++ ;
						pThis->m_EntryPos++ ;
					}
					ReturnValue = -1 ;
					break ;
				case LOADPASSWORD_GRID3:
					password_string[pThis->m_EntryPos] = 8 + pThis->m_SubSelection ;
					if (password_nibbles < MAX_PASSWORD)
					{
						password_nibbles++ ;
						pThis->m_EntryPos++ ;
					}
					ReturnValue = -1 ;
					break ;
				case LOADPASSWORD_GRID4:
					password_string[pThis->m_EntryPos] = 12 + pThis->m_SubSelection ;
					if (password_nibbles < MAX_PASSWORD)
					{
						password_nibbles++ ;
						pThis->m_EntryPos++ ;
					}
					ReturnValue = -1 ;
					break ;

				case LOADPASSWORD_DELETE:
					if (pThis->m_EntryPos)
					{
						pThis->m_EntryPos-- ;
						password_nibbles-- ;
					}
					ReturnValue = -1 ;
					break ;

				case LOADPASSWORD_ENTER:
					// clear the data
					CPassword__ClearData() ;

					// convert current string to data
					CPassword__ConvertStringToData() ;



					
					// all cheat mode character strings must be less than the level password
					// check for invincibility cheat code
					if (		(password_nibbles == 3)
							&&	(!(GetApp()->m_dwEnabledCheatFlags & CHEATFLAG_INVINCIBILITY))
							&&	(password_data[0] == 0xaaa0) )
					{
						// activate invincibility
						GetApp()->m_dwEnabledCheatFlags |= CHEATFLAG_INVINCIBILITY;
						CMessageBox__Construct(&GetApp()->m_MessageBox,
													"invincibility", "activated") ;
						GetApp()->m_bMessageBox = TRUE ;
					}
					// check for all weapons cheat code
					else if (		(password_nibbles == 3)
									&&	(!(GetApp()->m_dwEnabledCheatFlags & CHEATFLAG_ALLWEAPONS))
									&&	(password_data[0] == 0xbbb0) )
					{
						// activate all weapons
						GetApp()->m_dwEnabledCheatFlags |= CHEATFLAG_ALLWEAPONS;
						CMessageBox__Construct(&GetApp()->m_MessageBox,
													"all weapons", "activated") ;
						GetApp()->m_bMessageBox = TRUE ;
					}
					// check for big heads cheat code
					else if (		(password_nibbles == 3)
									&&	(!(GetApp()->m_dwEnabledCheatFlags & CHEATFLAG_BIGHEADS))
									&&	(password_data[0] == 0xccc0) )
					{
						// activate big heads
						GetApp()->m_dwEnabledCheatFlags |= CHEATFLAG_BIGHEADS;
						CMessageBox__Construct(&GetApp()->m_MessageBox,
													"big heads", "activated") ;
						GetApp()->m_bMessageBox = TRUE ;
					}
					// check for gallery cheat code
					else if (		(password_nibbles == 3)
									&&	(!(GetApp()->m_dwEnabledCheatFlags & CHEATFLAG_GALLERY))
									&&	(password_data[0] == 0xddd0) )
					{
						// activate invincibility
						GetApp()->m_dwEnabledCheatFlags |= CHEATFLAG_GALLERY;
						CMessageBox__Construct(&GetApp()->m_MessageBox,
													"gallery", "activated") ;
						GetApp()->m_bMessageBox = TRUE ;
					}


					
					// check if valid
					else if (CPassword__CheckChecksum() == FALSE)
					{
						CMessageBox__Construct(&GetApp()->m_MessageBox,
													"password invalid", "try again") ;
						GetApp()->m_bMessageBox = TRUE ;
					}
					else
					{
						CScene__ResetPersistantData();

						pThis->b_Active = FALSE ;
						pThis->m_Mode = LOAD_FADEDOWN ;
						GetApp()->m_bLoad = FALSE ;
						GetApp()->m_bPause = FALSE ;
						GetApp()->m_Pause.m_Mode = PAUSE_NULL ;

						// decode the password and setup the player vars
						CPassword__DecodeToPlayer() ;

						GetApp()->m_JustLoadedGame = TRUE ;

						GetApp()->m_WarpID = CTurokMovement.CurrentCheckpoint ;
						CEngineApp__SetupFadeTo(GetApp(), MODE_RESETLEVEL);
					}
					ReturnValue = -1 ;
					break ;

				case LOADPASSWORD_EXIT:
					pThis->m_Mode = LOAD_FADEDOWN ;
					break ;
			}
		}
	}


	return ReturnValue ;
}
*/


//------------------------------------------------------------------------
// Draw load screen information
//------------------------------------------------------------------------
void CLoad__Draw(CLoad *pThis, Gfx **ppDLP)
{
	// update fade status
	switch (pThis->m_Mode)
	{
		case LOAD_FADEUP:
			pThis->m_Alpha += .2 * frame_increment ;
			if (pThis->m_Alpha >= 1.0)
			{
				pThis->m_Alpha = 1.0 ;
				pThis->m_Mode = LOAD_NORMAL ;
			}
			break ;

		case LOAD_FADEDOWN:
			pThis->m_Alpha -= .2 * frame_increment;
			if (pThis->m_Alpha <= 0.0)
			{
				pThis->m_Alpha = 0.0 ;
				pThis->m_Mode = LOAD_NULL ;
				if (pThis->b_Active == FALSE)
					GetApp()->m_bLoad = FALSE ;
			}
			break ;
	}

	// prepare to draw boxes
	COnScreen__InitBoxDraw(ppDLP) ;


	if (pThis->m_LoadType == LOAD_MEMORYCARD)
	{
		CLoad__DrawMemory(pThis, ppDLP) ;
//		COnScreen__DrawBox(ppDLP,
//							 0, 0,
//							 SCREEN_WD, SCREEN_HT,
//							 0,0,0, 140*pThis->m_Alpha) ;
	}

//	else
//		CLoad__DrawPassword(pThis, ppDLP) ;

	// draw a message box if needed
	if (GetApp()->m_bMessageBox)
		CMessageBox__Draw(&GetApp()->m_MessageBox, ppDLP) ;
}


//-----------------------------------------------------------------------------------------------------
// void CLoad__DrawMemory(CLoad *pThis, Gfx **ppDLP)
//-----------------------------------------------------------------------------------------------------
void CLoad__DrawMemory(CLoad *pThis, Gfx **ppDLP)
{
	int	i, y, max ;

	// were gonna draw some boxes
	COnScreen__InitBoxDraw(ppDLP) ;

	// draw box for filenames
	COnScreen__DrawHilightBox(ppDLP,
							 LOAD_NAMEX, LOAD_NAMEY,
							 LOAD_NAMEX+LOAD_NAMEW, LOAD_NAMEY+LOAD_NAMEH,
							 2, FALSE,
							 50,30,30, 200 * pThis->m_Alpha) ;
	// draw box for options
	COnScreen__DrawHilightBox(ppDLP,
							 LOAD_OPTIONSX, LOAD_OPTIONSY,
							 LOAD_OPTIONSX+LOAD_OPTIONSW, LOAD_OPTIONSY+LOAD_OPTIONSH,
							 2, FALSE,
							 30,30,50, 200 * pThis->m_Alpha) ;

	if (pThis->m_Side == 0)
	{
		// draw box for selected unfoceused side
		y = pThis->m_BSelection *18 ;
		COnScreen__DrawHilightBox(ppDLP,
							 LOAD_OPTIONSX+4, LOAD_OPTIONSY+7+y,
							 LOAD_OPTIONSX+LOAD_OPTIONSW-4, LOAD_OPTIONSY+7+y+18,
							 1, FALSE,
							 30,30,50, 150 * pThis->m_Alpha) ;

		// highlight left option
		CPause__InitPolygon(ppDLP) ;
		y = (pThis->m_ASelection - pThis->m_FirstVisible) *8 ;
		CPause__DrawBar(ppDLP, LOAD_NAMEX+2, LOAD_NAMEY+8+y,
							LOAD_NAMEW-4, 8, pThis->m_Alpha) ;
	}
	else
	{
		y = (pThis->m_ASelection - pThis->m_FirstVisible) *8 ;
		COnScreen__DrawHilightBox(ppDLP,
							 LOAD_NAMEX+4, LOAD_NAMEY+6+y,
							 LOAD_NAMEX+LOAD_NAMEW-4, LOAD_NAMEY+6+y+11,
							 1, FALSE,
							 50,30,30, 150 * pThis->m_Alpha) ;

		// highlight right option
		CPause__InitPolygon(ppDLP) ;
		y = pThis->m_BSelection *18 ;
		CPause__DrawBar(ppDLP, LOAD_OPTIONSX+2, LOAD_OPTIONSY+8+y,
							LOAD_OPTIONSW-4, 18, pThis->m_Alpha) ;
	}


	// draw heading
	COnScreen__InitBoxDraw(ppDLP) ;
	COnScreen__DrawHilightBox(ppDLP,
						 HEADX, HEADY,
						 HEADX+HEADW, HEADY+HEADH,
						 1, FALSE,
						 50,10,50, 200 * pThis->m_Alpha) ;


	// draw names down left side
	COnScreen__InitFontDraw(ppDLP) ;
	COnScreen__SetFontScale(1.0, 1.0) ;

//	COnScreen__SetFontColor(ppDLP, 50, 255, 50, 255, 255, 50) ;
	COnScreen__SetFontColor(ppDLP, 255, 255, 0, 255, 255, 50) ;
	COnScreen__DrawText(ppDLP,
							  text_controllerpak,
							  320/2, HEADY+4,
							  (int)(255 * pThis->m_Alpha), TRUE, TRUE) ;


	
	COnScreen__PrepareNintendoFont(ppDLP) ;

	y = 0 ;
	max = (TurokFiles) ? TurokFiles : 1 ;
//	for (i=pThis->m_FirstVisible; i<max; i++)
	for (i=0; i<max; i++)
	{
		if (i==pThis->m_ASelection)
			COnScreen__SetFontColor(ppDLP, 255, 255, 255, 255, 0, 255) ;
		else
			COnScreen__SetFontColor(ppDLP, 150, 150, 150, 150, 0, 150) ;

		COnScreen__DrawText(ppDLP,
								  FileDirectory[i].string,
								  LOAD_NAMEX+8, LOAD_NAMEY+8+(y*8),
								  (int)(255 * pThis->m_Alpha), FALSE, FALSE) ;
		y++ ;


//		if (i>(pThis->m_FirstVisible+pThis->m_MaxVisible)-1)
//			break ;
	}

	// draw options down right side
	COnScreen__InitFontDraw(ppDLP) ;
	for (i=0; i<LOADMEMORY_END_SELECTION; i++)
	{
		if (i==pThis->m_BSelection)
			COnScreen__SetFontColor(ppDLP, 200*1.25, 200*1.25, 138*1.25, 86*1.25, 71*1.25, 47*1.25) ;
		else
			COnScreen__SetFontColor(ppDLP, 200*.9, 200*.9, 138*.9, 86*.9, 71*.9, 47*.9) ;

		COnScreen__DrawText(ppDLP,
								  loadmemory_text[i],
								  LOAD_OPTIONSX+8, LOAD_OPTIONSY+8 + (i*18),
								  (int)(255 * pThis->m_Alpha), FALSE, TRUE) ;
	}
}


//-----------------------------------------------------------------------------------------------------
// void CLoad__DrawPassword(CLoad *pThis, Gfx **ppDLP)
//-----------------------------------------------------------------------------------------------------
/*
void CLoad__DrawPassword(CLoad *pThis, Gfx **ppDLP)
{
	int		i, y ;
	int		x, w ;

	// prepare to draw boxes
	COnScreen__InitBoxDraw(ppDLP) ;

	// draw box
	COnScreen__DrawHilightBox(ppDLP,
							 LOAD_X, LOAD_Y,
							 LOAD_X+LOAD_WIDTH, LOAD_Y+LOAD_HEIGHT,
							 2, FALSE, 
							 0,0,0, 150 * pThis->m_Alpha) ;


	// draw current selection box
	y = LOADMENU_PASSWORDY ;
	i = 0;
	while (i!=pThis->m_Selection)
		y += loadpassword_text[i++].Spacing ;

	// each letter in grid
	if (loadpassword_text[pThis->m_Selection].items == 4)
	{
		x = LOADGRID_X + (pThis->m_SubSelection * LOADPASSWORD_SUBWIDTH) ;
		w = LOADPASSWORD_SUBWIDTH ;
	}
	// regular bar
	else
	{
		x = LOAD_X +16 ;
		w = LOAD_WIDTH-32 ;
	}

	CPause__InitPolygon(ppDLP) ;
	CPause__DrawBar(ppDLP, x, y,
						w, 18, pThis->m_Alpha) ;


	// prepare to draw textures
	COnScreen__InitFontDraw(ppDLP) ;

	// draw heading
	COnScreen__SetFontColor(ppDLP,
									200, 0, 200,
									0, 200, 0) ;
	COnScreen__DrawText(ppDLP,
							  "enter password",
							  320/2, 30,
							  (int)(255 * pThis->m_Alpha), TRUE, TRUE) ;



	// write the current password
	CPassword__DrawString(ppDLP, 30+20, (int)(255 * pThis->m_Alpha)) ;

	// draw each option
	COnScreen__InitFontDraw(ppDLP) ;
	y = LOADMENU_PASSWORDY ;
	for (i=0; i<LOADPASSWORD_END_SELECTION; i++)
	{
		COnScreen__DrawText(ppDLP,
								  loadpassword_text[i].String,
								  320/2, y,
								  (int)(255 * pThis->m_Alpha), TRUE, TRUE) ;
		y += loadpassword_text[i].Spacing ;
	}
}
*/


/*****************************************************************************
*
*	Functions for:		CSave class
*
******************************************************************************
*
*	Description:	routines to facilitate the saving process
*
*****************************************************************************/


//-----------------------------------------------------------------------------------------------------
// Construct SAVE structure
//-----------------------------------------------------------------------------------------------------
void CSave__Construct(CSave *pThis)
{
	int	ret ;

	// Setup default selection
	pThis->m_ASelection = 0 ;
	pThis->m_BSelection = 0 ;
	pThis->m_Side = 1 ;

	pThis->m_Selection = 0 ;

	pThis->m_FirstVisible = 0 ;
	pThis->m_MaxVisible = 7 ;

	pThis->m_ShowAll = FALSE ;


	if (pThis->m_Mode == SAVE_NULL)
		pThis->m_Alpha = 0 ;

	pThis->m_Mode = SAVE_FADEUP ;
	pThis->b_Active = TRUE ;

	GetApp()->m_bRequestor = FALSE ;
	GetApp()->m_bMessageBox = FALSE ;

	// if setting up the file system causes an error, then use password
	ret = SetupFileSystem() ;
//	if (	(ret == PFS_ERR_CONTRFAIL)
//		|| (ret == PFS_ERR_DEVICE)
//		|| (ret == PFS_ERR_ID_FATAL))
	if (ret)// == PFS_ERR_ID_FATAL)
	{
		pThis->m_Mode = SAVE_FADEDOWN ;
		CMessageBox__Construct(&GetApp()->m_MessageBox,
									text_errorcontroller, text_pakdamaged) ;
		GetApp()->m_bMessageBox = TRUE ;
		GetApp()->m_MessageBox.b_Active = FALSE ;
	}
	else if (validpaks == 0)
	{
		pThis->m_Mode = SAVE_FADEDOWN ;
		CMessageBox__Construct(&GetApp()->m_MessageBox,
									text_nocontroller, text_pakfound) ;
		GetApp()->m_bMessageBox = TRUE ;
		GetApp()->m_MessageBox.b_Active = FALSE ;
//		pThis->m_SaveType = SAVE_PASSWORD ;
//		CPassword__Create() ;
	}
	else
	{
		pThis->m_SaveType = SAVE_MEMORYCARD ;
		ScanFileList(pThis->m_ShowAll) ;
	}
	SaveRequestMode = NULL ;
}
		




//------------------------------------------------------------------------
// Update selection on save screen
//------------------------------------------------------------------------
INT32 CSave__Update(CSave *pThis)
{
	INT32				ReturnValue = -1 ;

	if (pThis->m_SaveType == SAVE_MEMORYCARD)
		ReturnValue = CSave__UpdateMemory(pThis) ;
//	else
//		ReturnValue = CSave__UpdatePassword(pThis) ;

	return ReturnValue ;
}

//-----------------------------------------------------------------------------------------------------
// ----------- MEMORY ------------------------
//-----------------------------------------------------------------------------------------------------
void CSave__HandleError(int ret)
{
	switch (ret)
	{
		case 0:
			CMessageBox__Construct(&GetApp()->m_MessageBox,
										text_gamesavedpress, text_buttontoexit) ;
			GetApp()->m_bMessageBox = TRUE ;
			GetApp()->m_MessageBox.b_Active = FALSE ;

			// force exit pause and continue game
			//pThis->m_Mode = SAVE_FADEDOWN ;
			//GetApp()->m_bPause = FALSE ;
			//GetApp()->m_Pause.m_Mode = PAUSE_NULL ;
			break ;

		case -1:
			CMessageBox__Construct(&GetApp()->m_MessageBox,
										text_mustsaveover, text_aturokgame) ;
			GetApp()->m_bMessageBox = TRUE ;
			break ;

		case PFS_DIR_FULL:
			CMessageBox__Construct(&GetApp()->m_MessageBox,
										text_toomanyfiles, text_gamenotsaved) ;
			GetApp()->m_bMessageBox = TRUE ;
			break ;

		case PFS_DATA_FULL:
			CMessageBox__Construct(&GetApp()->m_MessageBox,
										text_noroom, text_gamenotsaved) ;
			GetApp()->m_bMessageBox = TRUE ;
			break ;


		default:
			CMessageBox__Construct(&GetApp()->m_MessageBox,
										text_error, text_gamenotsaved) ;
			GetApp()->m_bMessageBox = TRUE ;
			break ;
	}
}

void CSave__HandleDeleteError(int ret)
{
	switch (ret)
	{
		case 0:
			break ;

		default:
			CMessageBox__Construct(&GetApp()->m_MessageBox,
										text_error, text_cannotdelete) ;
			GetApp()->m_bMessageBox = TRUE ;
			break ;
	}
	ScanFileList(GetApp()->m_Save.m_ShowAll);

	if (TurokFiles)
	{
		if (GetApp()->m_Save.m_ASelection > TurokFiles)
			GetApp()->m_Save.m_ASelection = TurokFiles ;
	}
	else
		GetApp()->m_Save.m_ASelection = 0 ;
}


INT32 CSave__UpdateMemory(CSave *pThis)
{
	int				ret ;
	INT32				ReturnValue = -1 ;

	if (GetApp()->m_bMessageBox)
	{
		ReturnValue = CMessageBox__Update(&GetApp()->m_MessageBox) ;
		if (ReturnValue != -1)
		{
			if (GetApp()->m_MessageBox.b_Active == FALSE)
			{
				// force exit pause and continue game
				pThis->m_Mode = SAVE_FADEDOWN ;
				GetApp()->m_Save.b_Active = FALSE ;
				GetApp()->m_Pause.m_Mode = PAUSE_FADEDOWN ;
				GetApp()->m_bPause = FALSE ;
			}
			//GetApp()->m_bMessageBox = FALSE ;
			GetApp()->m_MessageBox.m_Mode = MESSAGEBOX_FADEDOWN ;
			GetApp()->m_MessageBox.b_Active = FALSE ;
			ReturnValue = -1 ;
		}
	}

	else if (GetApp()->m_bRequestor)
	{
		ReturnValue = CRequestor__Update(&GetApp()->m_Requestor) ;
		if (ReturnValue != -1)
		{
			switch (SaveRequestMode)
			{
				case 0:
					// do save
					if (ReturnValue == REQUESTOR_YES)
					{
						CMessageBox__Construct(&GetApp()->m_MessageBox,
													text_gamesavedpress, text_buttontoexit) ;
						GetApp()->m_bMessageBox = TRUE ;
					}
					break ;

				// save over
				case 1:
					if (ReturnValue == REQUESTOR_YES)
					{
						ret = SaveCurrent(pThis->m_ASelection-1) ;
						CSave__HandleError(ret) ;
						ScanFileList(pThis->m_ShowAll);
					}
					GetApp()->m_Requestor.m_Mode = REQUESTOR_FADEDOWN ;
					GetApp()->m_Requestor.b_Active = FALSE ;
					SaveRequestMode = NULL ;
					ReturnValue = -1 ;
					break ;

				// delete
				case 2:
					if (ReturnValue == REQUESTOR_YES)
					{
						ret = DeleteCurrent(pThis->m_ASelection-1) ;
						CSave__HandleDeleteError(ret) ;
					}
					GetApp()->m_Requestor.m_Mode = REQUESTOR_FADEDOWN ;
					GetApp()->m_Requestor.b_Active = FALSE ;
					SaveRequestMode = NULL ;
					ReturnValue = -1 ;
					break ;
			}
		}
	}
	else
	{

		// switch sides
		if (CEngineApp__MenuLeft(GetApp()))
			pThis->m_Side ^= 1 ;
		else if (CEngineApp__MenuRight(GetApp()))
			pThis->m_Side ^= 1 ;


		// Goto next selection?
		if (CEngineApp__MenuDown(GetApp()))
		{
			// name side (+ create new slot)
			if (pThis->m_Side == 0)
			{
				BarTimer = 0 ;
				pThis->m_ASelection++ ;
				if (pThis->m_ASelection > TurokFiles)
					pThis->m_ASelection = 0 ;
			}
			// options side
			else
			{
				BarTimer = 0 ;
				pThis->m_BSelection++ ;
				if (pThis->m_BSelection >= SAVEMEMORY_END_SELECTION)
					pThis->m_BSelection = 0 ;
			}
		}

		// Goto previous selection?
		if (CEngineApp__MenuUp(GetApp()))
		{
			// name side (+ create new slot)
			if (pThis->m_Side == 0)
			{
				BarTimer = 0 ;
				pThis->m_ASelection-- ;
				if (pThis->m_ASelection < 0)
				{
					if (TurokFiles)
						pThis->m_ASelection = TurokFiles ;
					else
						pThis->m_ASelection = 0 ;
				}
			}
			// options side
			else
			{
				BarTimer = 0 ;
				pThis->m_BSelection-- ;
				if (pThis->m_BSelection < 0)
					pThis->m_BSelection = SAVEMEMORY_END_SELECTION-1 ;
			}
		}

		// Exit if start pressed
		// returns to options
		if (CTControl__IsUseMenu(pCTControl))
			ReturnValue = pThis->m_BSelection ;

		switch (ReturnValue)
		{
			case SAVEMEMORY_SAVE:
				ret = 0 ;

				CSave__PrepareData() ;

				// if create new slot, or empty slot, save new
				if (	(pThis->m_ASelection == 0)
					||	(FileDirectory[pThis->m_ASelection-1].pages == 0))
				{
					ret = SaveNew() ;
					CSave__HandleError(ret) ;
					ScanFileList(pThis->m_ShowAll);
					ReturnValue = -1 ;
				}
				else
				{
					CRequestor__Construct(&GetApp()->m_Requestor,
											text_saveover, text_existinggame) ;
					GetApp()->m_bRequestor = TRUE ;
					GetApp()->m_Requestor.m_Selection = REQUESTOR_NO ;
					SaveRequestMode = 1 ;
					ReturnValue = -1 ;
   				//ret = SaveCurrent(pThis->m_ASelection-1) ;
				}
				break ;


			case SAVEMEMORY_DELETE:
				// if create new slot, or empty slot, don't delete
				if (	(pThis->m_ASelection == 0)
					||	(FileDirectory[pThis->m_ASelection-1].pages == 0))
					ret = 1 ;
				else
				{
#ifndef GERMAN
					CRequestor__Construct(&GetApp()->m_Requestor,
									text_delete, text_thisgame) ;
#else
					CRequestor__Construct(&GetApp()->m_Requestor,
									text_thisgame, text_delete) ;
#endif
					GetApp()->m_bRequestor = TRUE ;
					GetApp()->m_Requestor.m_Selection = REQUESTOR_NO ;
					SaveRequestMode = 2 ;
					ReturnValue = -1 ;
					//ret = DeleteCurrent(pThis->m_ASelection-1) ;
				}

				ReturnValue = -1 ;
				break ;

			case SAVEMEMORY_SHOWALL:
				pThis->m_ShowAll = ~pThis->m_ShowAll ;
				ScanFileList(pThis->m_ShowAll) ;

				if (pThis->m_ASelection > TurokFiles)
					pThis->m_ASelection = TurokFiles ;

				ReturnValue = -1 ;
				break ;

			case SAVEMEMORY_EXIT:
				pThis->m_Mode = SAVE_FADEDOWN ;
				break ;
		}
	}
#if 0
	// scroll down check
	if (pThis->m_ASelection > (pThis->m_FirstVisible+pThis->m_MaxVisible+1))
		pThis->m_FirstVisible = pThis->m_ASelection - pThis->m_MaxVisible-1 ;
	// length clip check
	if ((pThis->m_FirstVisible+pThis->m_MaxVisible) > (TurokFiles-1))
		pThis->m_FirstVisible = TurokFiles-1 - pThis->m_MaxVisible ;
	// scroll up check
	if ((pThis->m_ASelection-1) < pThis->m_FirstVisible)
		pThis->m_FirstVisible = pThis->m_ASelection-1 ;
	// error check
	if (pThis->m_FirstVisible<0)
		pThis->m_FirstVisible = 0;
#endif

	return ReturnValue ;
}


//-----------------------------------------------------------------------------------------------------
// ----------- PASSWORD ------------------------
//-----------------------------------------------------------------------------------------------------
/*
INT32 CSave__UpdatePassword(CSave *pThis)
{
	INT32				ReturnValue = -1 ;

	// handle message box if active
	if (GetApp()->m_bMessageBox)
	{
		ReturnValue = CMessageBox__Update(&GetApp()->m_MessageBox) ;
		if (ReturnValue != -1)
		{
			GetApp()->m_bMessageBox = FALSE ;
			ReturnValue = -1 ;
		}
	}
	// otherwise handle password
	else
	{
		// Goto next selection?
		if (CEngineApp__MenuDown(GetApp()))
		{
			BarTimer = 0 ;
			pThis->m_Selection++ ;
			if (pThis->m_Selection >= SAVEPASSWORD_END_SELECTION)
				pThis->m_Selection = 0 ;
		}

		// Goto previous selection?
		if (CEngineApp__MenuUp(GetApp()))
		{
			BarTimer = 0 ;
			pThis->m_Selection-- ;
			if (pThis->m_Selection < 0)
				pThis->m_Selection = SAVEPASSWORD_END_SELECTION-1 ;
		}

		// Exit if start pressed
		// returns to options
		if (CTControl__IsUseMenu(pCTControl))
			ReturnValue = pThis->m_Selection ;

		if (ReturnValue == SAVEPASSWORD_EXIT)
			pThis->m_Mode = SAVE_FADEDOWN ;
	}

	return ReturnValue ;
}
*/


//------------------------------------------------------------------------
// Draw save screen information
//------------------------------------------------------------------------
void CSave__Draw(CSave *pThis, Gfx **ppDLP)
{

	// update fade status
	switch (pThis->m_Mode)
	{
		case SAVE_FADEUP:
			pThis->m_Alpha += .2 * frame_increment ;
			if (pThis->m_Alpha >= 1.0)
			{
				pThis->m_Alpha = 1.0 ;
				pThis->m_Mode = SAVE_NORMAL ;
			}
			break ;

		case SAVE_FADEDOWN:
			pThis->m_Alpha -= .2 * frame_increment ;
			if (pThis->m_Alpha <= 0.0)
			{
				pThis->m_Alpha = 0.0 ;
				pThis->m_Mode = SAVE_NULL ;
				if (pThis->b_Active == FALSE)
					GetApp()->m_bSave = FALSE ;
			}
			break ;
	}

	COnScreen__InitBoxDraw(ppDLP) ;


//	if (pThis->m_SaveType == SAVE_MEMORYCARD)
//	{
//		COnScreen__DrawBox(ppDLP,
//							 0, 0,
//							 SCREEN_WD, SCREEN_HT,
//							 0,0,0, 100*pThis->m_Alpha) ;

		CSave__DrawMemory(pThis, ppDLP) ;
//	}
//	else
//		CSave__DrawPassword(pThis, ppDLP) ;

	// draw the requestor if needed
	if (GetApp()->m_bRequestor)
		CRequestor__Draw(&GetApp()->m_Requestor, ppDLP) ;

	// draw a message box if needed
	if (GetApp()->m_bMessageBox)
		CMessageBox__Draw(&GetApp()->m_MessageBox, ppDLP) ;
}

 
//------------------------------------------------------------------------
// Draw memory card screen
//------------------------------------------------------------------------
void CSave__DrawMemory(CSave *pThis, Gfx **ppDLP)
{	
	int		i, y ;
	//char		buffer[32] ;


	// were gonna draw some boxes
	COnScreen__InitBoxDraw(ppDLP) ;

	// draw box for filenames
	COnScreen__DrawHilightBox(ppDLP,
							 SAVE_NAMEX, SAVE_NAMEY,
							 SAVE_NAMEX+SAVE_NAMEW, SAVE_NAMEY+SAVE_NAMEH,
							 2, FALSE, 
							 50,30,30, 200 * pThis->m_Alpha) ;
	// draw box for options
	COnScreen__DrawHilightBox(ppDLP,
							 SAVE_OPTIONSX, SAVE_OPTIONSY,
							 SAVE_OPTIONSX+SAVE_OPTIONSW, SAVE_OPTIONSY+SAVE_OPTIONSH,
							 2, FALSE,
							 30,30,50, 200 * pThis->m_Alpha) ;

	if (pThis->m_Side == 0)
	{
		// draw box for selected unfocused side
		y = pThis->m_BSelection *18 ;
		COnScreen__DrawHilightBox(ppDLP,
							 SAVE_OPTIONSX+4, SAVE_OPTIONSY+7+y,
							 SAVE_OPTIONSX+SAVE_OPTIONSW-4, SAVE_OPTIONSY+7+y+18,
							 1, FALSE,
							 30,30,50, 150 * pThis->m_Alpha) ;

		// highlight left option
		CPause__InitPolygon(ppDLP) ;
		y = (pThis->m_ASelection - pThis->m_FirstVisible) *8 ;
		CPause__DrawBar(ppDLP, SAVE_NAMEX+2, SAVE_NAMEY+8+y,
							SAVE_NAMEW-4, 8, pThis->m_Alpha) ;
	}
	else
	{
		y = (pThis->m_ASelection - pThis->m_FirstVisible) *8 ;
		COnScreen__DrawHilightBox(ppDLP,
							 SAVE_NAMEX+4, SAVE_NAMEY+6+y,
							 SAVE_NAMEX+SAVE_NAMEW-4, SAVE_NAMEY+6+y+11,
							 1, FALSE,
							 50,30,30, 150 * pThis->m_Alpha) ;

		// highlight right option
		CPause__InitPolygon(ppDLP) ;
		y = pThis->m_BSelection *18 ;
		CPause__DrawBar(ppDLP, SAVE_OPTIONSX+2, SAVE_OPTIONSY+8+y,
							SAVE_OPTIONSW-4, 18, pThis->m_Alpha) ;
	}


	// draw heading
	COnScreen__InitBoxDraw(ppDLP) ;
	COnScreen__DrawHilightBox(ppDLP,
						 HEADX, HEADY,
						 HEADX+HEADW, HEADY+HEADH,
						 1, FALSE,
						 50,10,50, 200 * pThis->m_Alpha) ;


	// draw names down left side
	COnScreen__InitFontDraw(ppDLP) ;
	COnScreen__SetFontScale(1.0, 1.0) ;

	COnScreen__SetFontColor(ppDLP, 50, 255, 50, 255, 255, 50) ;
	COnScreen__DrawText(ppDLP,
							  text_controllerpak,
							  320/2, HEADY+4,
							  (int)(255 * pThis->m_Alpha), TRUE, TRUE) ;



	COnScreen__PrepareSmallFont(ppDLP) ;

	y = 0 ;
	if (pThis->m_ASelection==0)
			COnScreen__SetFontColor(ppDLP, 255, 255, 255, 255, 0, 255) ;
	COnScreen__DrawText(ppDLP,
							  text_createnewslot,
#ifndef GERMAN
							  SAVE_NAMEX+8, SAVE_NAMEY+8+(y*8),
#else
							  SAVE_NAMEX+6, SAVE_NAMEY+8+(y*8),
#endif
							  (int)(255 * pThis->m_Alpha), FALSE, FALSE) ;
	y++ ;

	COnScreen__PrepareNintendoFont(ppDLP) ;
	COnScreen__SetFontScale(1.0, 1.0) ;

	// draw normal names
//	for (i=pThis->m_FirstVisible; i<TurokFiles; i++)
	for (i=0; i<TurokFiles; i++)
	{
		if (i==(pThis->m_ASelection-1))
			COnScreen__SetFontColor(ppDLP, 255, 255, 255, 255, 0, 255) ;
		else
			COnScreen__SetFontColor(ppDLP, 150, 150, 150, 150, 0, 150) ;

		COnScreen__DrawText(ppDLP,
								  FileDirectory[i].string,
								  SAVE_NAMEX+8, SAVE_NAMEY+8+(y*8),
								  (int)(255 * pThis->m_Alpha), FALSE, FALSE) ;
		y++ ;

//		if (i>(pThis->m_FirstVisible+pThis->m_MaxVisible)-1)
//			break ;
	}

	// display blocks free
	//COnScreen__SetFontScale(0.7, 0.7) ;
	//COnScreen__SetFontColor(ppDLP,
	//								200, 200, 200,
	//								200, 200, 0) ;
	//sprintf(buffer, "%d blocks free", FreePages) ;
	//COnScreen__DrawText(ppDLP,
	//						  buffer,
	//						  SAVE_NAMEX+8, SAVE_NAMEY+SAVE_NAMEH-11,
	//						  (int)(255 * pThis->m_Alpha), FALSE, TRUE) ;


	// draw options down right side
	COnScreen__InitFontDraw(ppDLP) ;
#ifdef GERMAN
	COnScreen__SetFontScale(0.7, 1.0) ;
#endif
	for (i=0; i<SAVEMEMORY_END_SELECTION; i++)
	{
		if (i==pThis->m_BSelection)
			COnScreen__SetFontColor(ppDLP, 200*1.25, 200*1.25, 138*1.25, 86*1.25, 71*1.25, 47*1.25) ;
		else
			COnScreen__SetFontColor(ppDLP, 200*.9, 200*.9, 138*.9, 86*.9, 71*.9, 47*.9) ;

		COnScreen__DrawText(ppDLP,
								  savememory_text[i],
								  SAVE_OPTIONSX+8, SAVE_OPTIONSY+8 + (i*18),
								  (int)(255 * pThis->m_Alpha), FALSE, TRUE) ;
	}

}

//------------------------------------------------------------------------
// Draw password screen
//------------------------------------------------------------------------
/*
void CSave__DrawPassword(CSave *pThis, Gfx **ppDLP)
{	
	int	i ;

	// prepare to draw boxes
	COnScreen__InitBoxDraw(ppDLP) ;

	// draw box
	COnScreen__DrawHilightBox(ppDLP,
							 SAVE_X, SAVE_Y,
							 SAVE_X+SAVE_WIDTH, SAVE_Y+SAVE_HEIGHT,
							 2, FALSE, 
							 20,20,20, 150 * pThis->m_Alpha) ;

	// draw current selection box
	CPause__InitPolygon(ppDLP) ;
	CPause__DrawBar(ppDLP, SAVE_X+32, SAVEMENU_PASSWORDY,
						SAVE_WIDTH-64, SAVEMENU_SPACING, pThis->m_Alpha) ;


	// prepare to draw textures
	COnScreen__InitFontDraw(ppDLP) ;

	// draw heading
	COnScreen__SetFontScale(1.0, 2.0) ;
	COnScreen__SetFontColor(ppDLP,
									200, 0, 200,
									0, 200, 0) ;
	COnScreen__DrawText(ppDLP,
							  "password",
							  320/2, 60,
							  (int)(255 * pThis->m_Alpha), TRUE, TRUE);

	// write the current password
	CPassword__DrawString(ppDLP, 60+32+8, (int)(255 * pThis->m_Alpha)) ;


	// draw each option
	COnScreen__InitFontDraw(ppDLP) ;
	for (i=0; i<SAVEPASSWORD_END_SELECTION; i++)
	{
		COnScreen__DrawText(ppDLP,
								  savepassword_text[i],
								  320/2, SAVEMENU_PASSWORDY + (i*SAVEMENU_SPACING),
								  (int)(255 * pThis->m_Alpha), TRUE, TRUE) ;
	}
}

*/


//------------------------------------------------------------------------
// Construct REQUESTOR structure
//------------------------------------------------------------------------
// REQUESTOR OPTIONS
#ifdef GERMAN
#define REQUESTOR_WIDTH		(280)
#define REQUESTOR_HEIGHT	(100)
#else
#define REQUESTOR_WIDTH		(200)
#define REQUESTOR_HEIGHT	(100)
#endif

#define REQUESTOR_X 			((320/2) - (REQUESTOR_WIDTH/2))
#define REQUESTOR_Y			((240/2) - (REQUESTOR_HEIGHT/2))

#define REQUESTORMENU_Y			(REQUESTOR_Y + 48)
#define REQUESTORMENU_SPACING	20


char *requestor_text[]=
{
	text_yes,
	text_no,
} ;



void CRequestor__Construct(CRequestor *pThis, char *String1, char *String2)
{
	// Setup default selection
	pThis->m_Selection = REQUESTOR_YES ;


	pThis->Line1 = String1 ;
	pThis->Line2 = String2 ;

	pThis->m_Alpha = 0 ;
	pThis->m_Mode = REQUESTOR_FADEUP ;
	pThis->b_Active = TRUE ;

	pThis->Selections = requestor_text ;
}


//------------------------------------------------------------------------
// Update selection on requestor
//------------------------------------------------------------------------
INT32 CRequestor__Update(CRequestor *pThis)
{
	INT32				ReturnValue = -1 ;

	if (pThis->m_Mode != REQUESTOR_FADEDOWN)
	{
		// Goto next selection?
		if (CEngineApp__MenuDown(GetApp()))
		{
			BarTimer = 0 ;
			pThis->m_Selection++ ;
			if (pThis->m_Selection >= REQUESTOR_END_SELECTION)
				pThis->m_Selection = 0 ;
		}

		// Goto previous selection?
		if (CEngineApp__MenuUp(GetApp()))
		{
			BarTimer = 0 ;
			pThis->m_Selection-- ;
			if (pThis->m_Selection < 0)
				pThis->m_Selection = REQUESTOR_END_SELECTION-1 ;
		}

		// Exit if start pressed
		if (CTControl__IsUseMenu(pCTControl))
			ReturnValue = pThis->m_Selection ;
	}


	return ReturnValue ;
}


//------------------------------------------------------------------------
// Draw requestor information
//------------------------------------------------------------------------
void CRequestor__Draw(CRequestor *pThis, Gfx **ppDLP)
{
	int		i ;

	// update fade status
	switch (pThis->m_Mode)
	{
		case REQUESTOR_FADEUP:
			pThis->m_Alpha += .2 * frame_increment ;
			if (pThis->m_Alpha >= 1.0)
			{
				pThis->m_Alpha = 1.0 ;
				pThis->m_Mode = REQUESTOR_NORMAL ;
			}
			break ;

		case REQUESTOR_FADEDOWN:
			pThis->m_Alpha -= .2 * frame_increment;
			if (pThis->m_Alpha <= 0.0)
			{
				pThis->m_Alpha = 0.0 ;
				pThis->m_Mode = REQUESTOR_NULL ;
				if (pThis->b_Active == FALSE)
					GetApp()->m_bRequestor = FALSE ;
			}
			break ;
	}

	// prepare to draw boxes
	COnScreen__InitBoxDraw(ppDLP) ;

	COnScreen__DrawBox(ppDLP,
							 0, 0, SCREEN_WD, SCREEN_HT,
							 0,0,0, 120*pThis->m_Alpha) ;

	COnScreen__DrawHilightBox(ppDLP,
							 REQUESTOR_X, REQUESTOR_Y,
							 REQUESTOR_X+REQUESTOR_WIDTH, REQUESTORMENU_Y-1,
							 2, TRUE,
							 30,30,30, 220*pThis->m_Alpha) ;
	COnScreen__DrawHilightBox(ppDLP,
							 REQUESTOR_X, REQUESTORMENU_Y,
							 REQUESTOR_X+REQUESTOR_WIDTH, REQUESTOR_Y+REQUESTOR_HEIGHT,
							 2, TRUE,
							 30,30,30, 220*pThis->m_Alpha) ;


	// draw current selection box
	CPause__InitPolygon(ppDLP) ;
	CPause__DrawBar(ppDLP, REQUESTOR_X+32, REQUESTORMENU_Y+6+(pThis->m_Selection*REQUESTORMENU_SPACING),
						REQUESTOR_WIDTH-64, 18, pThis->m_Alpha) ;
//	COnScreen__DrawHilightBox(ppDLP,
//								 REQUESTOR_X+32, REQUESTORMENU_Y+6 + (pThis->m_Selection*REQUESTORMENU_SPACING),
//								 REQUESTOR_X+REQUESTOR_WIDTH-32, REQUESTORMENU_Y+6 + (pThis->m_Selection*REQUESTORMENU_SPACING)+18,
//								 1, TRUE,
//								 70,40,127, 255 *pThis->m_Alpha) ;

	// prepare to draw textures
	COnScreen__InitFontDraw(ppDLP);

	// draw heading
	COnScreen__DrawText(ppDLP,
							  pThis->Line1,
							  320/2, REQUESTOR_Y+4,
							  255*pThis->m_Alpha, TRUE, TRUE) ;
							
	COnScreen__DrawText(ppDLP,
							  pThis->Line2,
							  320/2, REQUESTOR_Y+4+16,
							  255*pThis->m_Alpha, TRUE, TRUE) ;


	// draw each option
	for (i=0; i<REQUESTOR_END_SELECTION; i++)
	{
		COnScreen__DrawText(ppDLP,
									(char *)pThis->Selections[i],
								  320/2, REQUESTORMENU_Y+6 + (i*REQUESTORMENU_SPACING),
								  255*pThis->m_Alpha, TRUE, TRUE) ;
	}
}


//------------------------------------------------------------------------
// Construct MESSAGEBOX structure
//------------------------------------------------------------------------
// MESSAGEBOX OPTIONS
#ifdef GERMAN
#define MESSAGEBOX_WIDTH		(280)
#define MESSAGEBOX_HEIGHT		(100)
#else
#define MESSAGEBOX_WIDTH		(200)
#define MESSAGEBOX_HEIGHT		(100)
#endif

#define MESSAGEBOX_X 			((320/2) - (MESSAGEBOX_WIDTH/2))
#define MESSAGEBOX_Y				((240/2) - (MESSAGEBOX_HEIGHT/2))


#define MESSAGEBOXMENU_Y 		(MESSAGEBOX_Y + MESSAGEBOX_HEIGHT - 32)


char *messagebox_text[]=
{
	text_ok,
} ;



void CMessageBox__Construct(CMessageBox *pThis, char *String1, char *String2)
{
	// Setup default selection
	pThis->m_Selection = MESSAGEBOX_OK ;

	pThis->Line1 = String1 ;
	pThis->Line2 = String2 ;

	pThis->m_Alpha = 0 ;
	pThis->m_Mode = MESSAGEBOX_FADEUP ;
	pThis->b_Active = TRUE ;
}


//------------------------------------------------------------------------
// Update selection on messagebox
//------------------------------------------------------------------------
INT32 CMessageBox__Update(CMessageBox *pThis)
{
	INT32				ReturnValue = -1 ;

	GetApp()->m_ModeTime = 0 ;
	// message boxes have no selections, only 'ok'

	if (pThis->m_Mode != MESSAGEBOX_FADEDOWN)
	{
		// Exit if start pressed
		if (CTControl__IsUseMenu(pCTControl))
			ReturnValue = pThis->m_Selection ;
	}

	return ReturnValue ;
}


//------------------------------------------------------------------------
// Draw messagebox information
//------------------------------------------------------------------------
void CMessageBox__Draw(CMessageBox *pThis, Gfx **ppDLP)
{
	// update fade status
	switch (pThis->m_Mode)
	{
		case MESSAGEBOX_FADEUP:
			pThis->m_Alpha += .2 * frame_increment ;
			if (pThis->m_Alpha >= 1.0)
			{
				pThis->m_Alpha = 1.0 ;
				pThis->m_Mode = MESSAGEBOX_NORMAL ;
			}
			break ;

		case MESSAGEBOX_FADEDOWN:
			pThis->m_Alpha -= .2 * frame_increment;
			if (pThis->m_Alpha <= 0.0)
			{
				pThis->m_Alpha = 0.0 ;
				pThis->m_Mode = MESSAGEBOX_NULL ;
				if (pThis->b_Active == FALSE)
					GetApp()->m_bMessageBox = FALSE ;
			}
			break ;
	}

	// prepare to draw boxes
	COnScreen__InitBoxDraw(ppDLP) ;

	COnScreen__DrawBox(ppDLP,
							 0, 0, SCREEN_WD, SCREEN_HT,
							 0,0,0, 120*pThis->m_Alpha) ;

	COnScreen__DrawHilightBox(ppDLP,
							 MESSAGEBOX_X, MESSAGEBOX_Y,
							 MESSAGEBOX_X+MESSAGEBOX_WIDTH, MESSAGEBOXMENU_Y-16,
							 2, TRUE,
							 30,30,30, 220*pThis->m_Alpha) ;
	COnScreen__DrawHilightBox(ppDLP,
							 MESSAGEBOX_X, MESSAGEBOXMENU_Y-15,
							 MESSAGEBOX_X+MESSAGEBOX_WIDTH, MESSAGEBOX_Y+MESSAGEBOX_HEIGHT,
							 2, TRUE,
							 30,30,30, 220*pThis->m_Alpha) ;


	// draw current selection box
	CPause__InitPolygon(ppDLP) ;
	CPause__DrawBar(ppDLP, MESSAGEBOX_X+32, MESSAGEBOXMENU_Y,
						MESSAGEBOX_WIDTH-64, 18, pThis->m_Alpha) ;
//	COnScreen__DrawHilightBox(ppDLP,
//							 MESSAGEBOX_X+32, MESSAGEBOXMENU_Y,
//							 MESSAGEBOX_X+MESSAGEBOX_WIDTH-32, MESSAGEBOXMENU_Y+18,
//							 1, FALSE, 
//							 127,127,0, 255*pThis->m_Alpha) ;


	// prepare to draw textures
	COnScreen__InitFontDraw(ppDLP);

	// draw heading
	COnScreen__DrawText(ppDLP,
							  pThis->Line1,
							  320/2, MESSAGEBOX_Y+8,
							  255*pThis->m_Alpha, TRUE, TRUE) ;
	COnScreen__DrawText(ppDLP,
							  pThis->Line2,
							  320/2, MESSAGEBOX_Y+8+16,
							  255*pThis->m_Alpha, TRUE, TRUE) ;


	// draw only option
	COnScreen__DrawText(ppDLP,
								  messagebox_text[0],
								  320/2, MESSAGEBOXMENU_Y,
								  255*pThis->m_Alpha, TRUE, TRUE) ;
}



//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//
// PASSWORD STUFF
//
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
/*
// maximum characters of password to display per line
#define	MAX_PASSWORD_LINE		18

//-----------------------------------------------------------------------------------------------------
// draw a line of the password
//-----------------------------------------------------------------------------------------------------
void CPassword__DrawLine(Gfx **ppDLP, int y, int opacity, int chars, int offset)
{
	int	x, i ;
	char	string[2] ;

	// printing 1 char at a time, so make a null terminator
	string[1] = '\0' ;

	// centre x
	x = 320 /2 ;
	x -= (chars *12) /2 ;

	for (i=0; i<chars; i++)
	{
		string[0] = password_entries[password_string[i+offset]] ;

		// if in load mode, flash current letter
		if (((i+offset) == password_nibbles-1) && (GetApp()->m_bLoad))
		{
			if ((frame_number&7)<4)
			{
				COnScreen__DrawText(ppDLP, string,
									  x, y, opacity, FALSE, TRUE) ;
			}
		}
		else
			COnScreen__DrawText(ppDLP, string,
								  x, y, opacity, FALSE, TRUE) ;
		x += 12 ;
	}
}

//-----------------------------------------------------------------------------------------------------
// draw current password, character at a time
//-----------------------------------------------------------------------------------------------------
void CPassword__DrawString(Gfx **ppDLP, int y, int opacity)
{
	int	count ;
	int	offset = 0;

	COnScreen__SetFontScale(1.0, 1.0) ;
	COnScreen__SetFontColor(ppDLP,
									255, 255, 255,
									255, 0, 255) ;

	// single line
	if (password_nibbles < MAX_PASSWORD_LINE)
		CPassword__DrawLine(ppDLP, y, opacity, password_nibbles, 0) ;

	// multi line
	else
	{
		count = password_nibbles ;
		while (count >MAX_PASSWORD_LINE)
		{
			CPassword__DrawLine(ppDLP, y, opacity, MAX_PASSWORD_LINE, offset) ;
			y += 16 ;
			offset += MAX_PASSWORD_LINE ;
			count -= MAX_PASSWORD_LINE ;
		}
		CPassword__DrawLine(ppDLP, y, opacity, count, offset) ;
	}
}




//-----------------------------------------------------------------------------------------------------
// Clear password data, and positional vars
//-----------------------------------------------------------------------------------------------------
void CPassword__ClearData(void)
{
	int	i ;
	password_nibble = 0 ;
	password_bit = 0 ;

	for (i=0; i<sizeof(password_data) / sizeof(u16); i++)
		password_data[i] = 0 ;
}


#ifdef CREATE_PASSWORD
//-----------------------------------------------------------------------------------------------------
// CreateChecksum
// makes a checksum out of all the data, and sticks it in the first byte
//-----------------------------------------------------------------------------------------------------
void CPassword__CreateChecksum(void)
{
	int	i, byte, index  ;
	char	checksum ;

	checksum = 0 ;
	index = 0 ;
	for (i=2; i<password_nibbles; i++)
	{
		if ((i&3)==0)
			index++ ;

		switch (i&3)
		{
			case 0:
			case 1:
				byte = password_data[index]>>8 ;
				break ;
			case 2:
			case 3:
				byte = password_data[index] ;
				break ;
		}
		byte &= 0xff ;
		checksum += byte ;
	}

	if (checksum == 0)
		checksum = 1 ;

	password_data[0] |= (checksum<<8) ;

	//rmonPrintf("checksum is %d\n", checksum) ;
}
	
//-----------------------------------------------------------------------------------------------------
// CheckChecksum
// reads data and checks if password is ok
//-----------------------------------------------------------------------------------------------------
BOOL CPassword__CheckChecksum(void)
{
	int	i, byte, index ;
	char	checksum, original ;


	checksum = 0 ;
	index = 0 ;
	for (i=2; i<password_nibbles; i++)
	{
		if ((i&3)==0)
			index++ ;

		switch (i&3)
		{
			case 0:
			case 1:
				byte = password_data[index]>>8 ;
				break ;
			case 2:
			case 3:
				byte = password_data[index] ;
				break ;
		}
		byte &= 0xff ;
		checksum += byte ;
	}

	if (checksum == 0)
		checksum = 1 ;

	original = (password_data[0] >>8) &0xff ;

	if (checksum != original)
		return FALSE ;
	else
		return TRUE ;
}
	
//-----------------------------------------------------------------------------------------------------
// Add to the password string
// reads a variable, and adds it to the current
// password bit by bit, by reading current bit,
// shifting to bottom bit, then shifting to current bit in password and ORing...
//-----------------------------------------------------------------------------------------------------
void CPassword__AddData(void *data, int bits)
{
	u16	number ;
	u16	bitstatus ;
	float	spare ;

	number = *(int *)data ;
	number &= (1<<bits)-1 ;
//	rmonPrintf("Adding %d (%d bits)\n", number, bits) ;

	while (bits--)
	{
		// read bit from number, and shift to bottom
		bitstatus = number & (1<<bits) ;
		bitstatus >>= bits ;

		// move bit to correct place for ORing
		bitstatus <<= (15-password_bit) ;

		// store in password
		password_data[password_nibble/4] |= bitstatus ;

		// increase and wrap counters
		password_bit++ ;
		if (password_bit >15)
		{
			password_bit = 0 ;
			password_nibble += 4;
		}
	}
	password_nibbles = password_nibble ;

	spare = password_bit/4.0 ;
	spare += .5 ;

	password_nibbles += spare ;
}

//-----------------------------------------------------------------------------------------------------
// Get data from the password string
// gets data and store at correct bit in destination number
//-----------------------------------------------------------------------------------------------------
void CPassword__GetData(void *data, int bits)
{
	u16	number, word ;

	number = 0 ;
	while (bits--)
	{
		// read data from string
		word = password_data[password_nibble/4] ;

		// shift down to correct bit
		word &= (1<<(15-password_bit)) ;
		word >>= (15-password_bit) ;

		// OR into current, and shift up for next bit
		number <<= 1; 
		number |= word;

		// increase and wrap counters
		password_bit++ ;
		if (password_bit >15)
		{
			password_bit = 0 ;
			password_nibble += 4;
		}
	}

	*(int *)data = number ;
//	rmonPrintf("Retreiving %d\n", number) ;
}



//-----------------------------------------------------------------------------------------------------
// Create a password for current game
// reads all variables from table and stores as a bitstream
//-----------------------------------------------------------------------------------------------------
void CPassword__Create(void)
{
	int	i, temp ;

	CPassword__ClearData() ;
	password_nibbles = 0 ;

	// leave 8 bits for a checksum
	password_bit = 8 ;
	password_nibbles = 2;
	for (i=0; i< sizeof(PasswordVariables) / sizeof(t_PasswordInfo); i++)
		CPassword__AddData(PasswordVariables[i].Data, PasswordVariables[i].Bits) ;

	// 8bit health
	temp = CEngineApp__GetPlayer(GetApp())->m_AI.m_Health ;
	CPassword__AddData(&temp, 8);

	// 4bit weapon
	temp = CTurokMovement.WeaponSelectIndex ;
	CPassword__AddData(&temp, 4);



	// 16bit save id
	CPassword__AddData(&CTurokMovement.CurrentCheckpoint, 16) ;

	// prepend the checksum
	CPassword__CreateChecksum() ;

//	rmonPrintf("%d entries added to password\n", i) ;
//	rmonPrintf("%d bits remaining\n", password_bit) ;

//	password_nibble = 0;
//	password_bit = 0 ;
//	for (i=0; i< sizeof(PasswordVariables) / sizeof(t_PasswordInfo); i++)
// 		CPassword__GetData(PasswordVariables[i].Data, PasswordVariables[i].Bits) ;
//
//	rmonPrintf("%d entries read back from password\n", i) ;

	CPassword__ConvertDataToString() ;

	password_active = TRUE ;
}


//-----------------------------------------------------------------------------------------------------
// Decode a password for current game
// puts all variables back where they came from!
//-----------------------------------------------------------------------------------------------------
void CPassword__DecodeToPlayer(void)
{
	int				i, temp ;

	// retreive info from password into player data
	password_nibble = 2;
	password_bit = 8 ;
	for (i=0; i< sizeof(PasswordVariables) / sizeof(t_PasswordInfo); i++)
 		CPassword__GetData(PasswordVariables[i].Data, PasswordVariables[i].Bits) ;

	// 8bit health
	CPassword__GetData(&temp, 8);
	AI_GetDyn(CEngineApp__GetPlayer(GetApp()))->m_Health = temp ;

	// 4bit weapon
	CPassword__GetData(&temp, 4);
	CTurokMovement.WeaponSelectIndex = temp ;


	// 16bit save id
	CPassword__GetData(&CTurokMovement.CurrentCheckpoint, 16) ;

	if (CTurokMovement.ArmorAmount)
		CTurokMovement.ArmorFlag = TRUE ;
	else
		CTurokMovement.ArmorFlag = FALSE ;

	CTurokMovement.Level2Access = CTurokMovement.Level2Keys ;
	CTurokMovement.Level3Access = CTurokMovement.Level3Keys ;
	CTurokMovement.Level4Access = CTurokMovement.Level4Keys ;
	CTurokMovement.Level5Access = CTurokMovement.Level5Keys ;
	CTurokMovement.Level6Access = CTurokMovement.Level6Keys ;
	CTurokMovement.Level7Access = CTurokMovement.Level7Keys ;
	CTurokMovement.Level8Access = CTurokMovement.Level8Keys ;
}

#endif


//----------------------------------------------------------
// Convert the password data to a printable string
//----------------------------------------------------------
void CPassword__ConvertDataToString(void)
{
	u16	dcount = 0;
	u16	scount = 0;
	u16	byte ;
	int	nibble = 0 ;
	BOOL	working = TRUE ;

	while (working)
	{
		// read a byte from data
		byte = password_data[dcount] ;

		// convert to nibble
		switch (nibble)
		{
			case 0:
				byte >>= 12 ;
				break ;
			case 1:
				byte >>= 8 ;
				break ;
			case 2:
				byte >>= 4 ;
				break ;
		}
		byte &= 0x0f ;

		// store converted nibble in string
		password_string[scount] = byte ;

		// move along in nibbles, move along data if nibble 0
		nibble++ ;
		if (nibble >3)
		{
			nibble = 0 ;
			dcount++ ;
		}

		// advance string position and check for end
		scount++ ;
		if (scount >= password_nibbles)
			working = FALSE ;
	}
}

//----------------------------------------------------------
// Convert the password string back to data
//----------------------------------------------------------
void CPassword__ConvertStringToData(void)
{
	u16	dcount = 0;
	u16	scount = 0;
	u16	byte ;
	u16	data ;
	int	nibble = 0 ;
	BOOL	working = TRUE ;

	while (working)
	{
		// read a byte from string
		data = password_string[scount] & 0x0f ;

		// convert to nibble
		switch (nibble)
		{
			case 0:
				byte = (data<<12) ;
				break ;
			case 1:
				byte |= (data<<8) ;
				break ;
			case 2:
				byte |= (data<<4) ;
				break ;
			case 3:
				byte |= (data) ;
				break ;
		}

		// store converted nibble in data
		password_data[dcount] = byte ;

		// move along in nibbles, move along data if nibble 0
		nibble++ ;
		if (nibble >3)
		{
			nibble = 0 ;
			dcount++ ;
		}

		// advance string position and check for end
		scount++ ;
		if (scount >= password_nibbles)
			working = FALSE ;
	}
}
*/

//-----------------------------------------------------------------------------------------------------
// Get data from various places, and put it in the persistant data block
// ready for saving to the controller pack
//-----------------------------------------------------------------------------------------------------
extern	BYTE persistant_data[];

void CSave__PrepareData(void)
{
	CPersistantData	*persist ;
	persist = (CPersistantData *)CScene__GetPersistantDataStruct(&GetApp()->m_Scene) ;

	//rmonPrintf("ptr:%08x\n", persist) ;
	//rmonPrintf("data:%08x\n", persistant_data) ;

	persist->SemiAutomaticPistolFlag		= CTurokMovement.SemiAutomaticPistolFlag ;
	persist->RiotShotgunFlag				= CTurokMovement.RiotShotgunFlag ;
	persist->AutomaticShotgunFlag			= CTurokMovement.AutomaticShotgunFlag ;
	persist->AssaultRifleFlag				= CTurokMovement.AssaultRifleFlag ;
	persist->MachineGunFlag					= CTurokMovement.MachineGunFlag ;
	persist->MiniGunFlag						= CTurokMovement.MiniGunFlag ;
	persist->GrenadeLauncherFlag			= CTurokMovement.GrenadeLauncherFlag ;
	persist->TechWeapon1Flag				= CTurokMovement.TechWeapon1Flag ;
	persist->RocketFlag						= CTurokMovement.RocketFlag ;
	persist->ShockwaveFlag					= CTurokMovement.ShockwaveFlag ;
	persist->TechWeapon2Flag				= CTurokMovement.TechWeapon2Flag;
	persist->ChronoSceptorFlag				= CTurokMovement.ChronoscepterFlag;

	persist->ArmorFlag						= CTurokMovement.ArmorFlag ;
	persist->BackPackFlag					= CTurokMovement.BackPackFlag ;

	persist->BulletPool						= CTurokMovement.BulletPool,
	persist->ShotgunPool						= CTurokMovement.ShotgunPool,
	persist->EnergyPool						= CTurokMovement.EnergyPool,
	persist->ExpTekBowAmmo					= CTurokMovement.ExpTekBowAmmo,
	persist->ExpShotgunPool					= CTurokMovement.ExpShotgunPool,
	persist->TekBowAmmo						= CTurokMovement.TekBowAmmo,
	persist->MiniGunAmmo						= CTurokMovement.MiniGunAmmo,
	persist->GrenadeLauncherAmmo			= CTurokMovement.GrenadeLauncherAmmo,
	persist->RocketAmmo						= CTurokMovement.RocketAmmo,
	persist->TechWeapon2Ammo				= CTurokMovement.TechWeapon2Ammo;
	persist->ChronoSceptorAmmo				= CTurokMovement.ChronoscepterAmmo;

	persist->WeaponUsing						= CTurokMovement.WeaponSelectIndex ;

	persist->ArmorAmount						= CTurokMovement.ArmorAmount ;
	persist->Level2Keys						= CTurokMovement.Level2Keys ;
	persist->Level3Keys						= CTurokMovement.Level3Keys ;
	persist->Level4Keys						= CTurokMovement.Level4Keys ;
	persist->Level5Keys						= CTurokMovement.Level5Keys ;
	persist->Level6Keys						= CTurokMovement.Level6Keys ;
	persist->Level7Keys						= CTurokMovement.Level7Keys ;
	persist->Level8Keys						= CTurokMovement.Level8Keys ;
	persist->Level2Access					= CTurokMovement.Level2Access ;
	persist->Level3Access					= CTurokMovement.Level3Access ;
	persist->Level4Access					= CTurokMovement.Level4Access ;
	persist->Level5Access					= CTurokMovement.Level5Access ;
	persist->Level6Access					= CTurokMovement.Level6Access ;
	persist->Level7Access					= CTurokMovement.Level7Access ;
	persist->Level8Access					= CTurokMovement.Level8Access ;

	persist->ChronoSceptorPieces			= CTurokMovement.ChronoSceptorPieces ;


	persist->RunWalk							= CTurokMovement.RunWalkToggle ;
	persist->ControlSide						= GetApp()->m_Options.m_RHControl ;
	persist->Blood								= GetApp()->m_Options.m_Blood ;
	persist->Stereo							= 0;
	persist->MusicVolume						= GetApp()->m_Options.m_MusicVolume ;
	persist->SFXVolume						= GetApp()->m_Options.m_SFXVolume ;
	persist->Opacity							= GetApp()->m_Options.m_Opacity ;
	persist->HAnalog							= GetApp()->m_Options.m_HAnalog ;
	persist->VAnalog							= GetApp()->m_Options.m_VAnalog ;

//	rmonPrintf("analog write:%d\n", persist->Analog) ;

	persist->Difficulty						= GetApp()->m_Difficulty ;

	persist->Health							= CEngineApp__GetPlayer(GetApp())->m_AI.m_Health ;
	persist->MaxHealth 						= CTurokMovement.MaxHealth ;

	persist->Tokens							= CTurokMovement.Tokens ;
	persist->Lives								= CTurokMovement.Lives ;

	//rmonPrintf("save health:%d\n", persist->Health) ;
	//rmonPrintf("save lives:%d\n", persist->Lives) ;

//	persist->Checkpoint						= GetApp()->m_WarpID;
	persist->Checkpoint						= CTurokMovement.CurrentCheckpoint;
	persist->CheatFlags						= GetApp()->m_dwCheatFlags;
	persist->EnabledCheatFlags	  			= GetApp()->m_dwEnabledCheatFlags;

	persist->BestTrainTime		  			= GetApp()->m_BestTrainTime;

	persist->BossFlags			  			= GetApp()->m_BossFlags ;
}

//-----------------------------------------------------------------------------------------------------
// Put the data from the persistant block back to the game vars
// ready for restarting the loaded game
//-----------------------------------------------------------------------------------------------------


void CLoad__ExtractData(void)
{
//	float				VolChange;
	CPersistantData	*persist ;
	persist = (CPersistantData *)CScene__GetPersistantDataStruct(&GetApp()->m_Scene) ;

	//rmonPrintf("ptr:%08x\n", persist) ;
	//rmonPrintf("data:%08x\n", persistant_data) ;

	CTurokMovement.SemiAutomaticPistolFlag		= persist->SemiAutomaticPistolFlag ;
	CTurokMovement.RiotShotgunFlag				= persist->RiotShotgunFlag ;
	CTurokMovement.AutomaticShotgunFlag			= persist->AutomaticShotgunFlag ;
	CTurokMovement.AssaultRifleFlag				= persist->AssaultRifleFlag ;
	CTurokMovement.MachineGunFlag					= persist->MachineGunFlag ;
	CTurokMovement.MiniGunFlag						= persist->MiniGunFlag ;
	CTurokMovement.GrenadeLauncherFlag			= persist->GrenadeLauncherFlag ;
	CTurokMovement.TechWeapon1Flag				= persist->TechWeapon1Flag ;
	CTurokMovement.RocketFlag						= persist->RocketFlag ;
	CTurokMovement.ShockwaveFlag					= persist->ShockwaveFlag ;
	CTurokMovement.TechWeapon2Flag				= persist->TechWeapon2Flag;
	CTurokMovement.ChronoscepterFlag				= persist->ChronoSceptorFlag;

	CTurokMovement.ArmorFlag						= persist->ArmorFlag ;
	CTurokMovement.BackPackFlag					= persist->BackPackFlag ;

	CTurokMovement.BulletPool						= persist->BulletPool,
	CTurokMovement.ShotgunPool						= persist->ShotgunPool,
	CTurokMovement.EnergyPool						= persist->EnergyPool,
	CTurokMovement.ExpTekBowAmmo					= persist->ExpTekBowAmmo,
	CTurokMovement.ExpShotgunPool					= persist->ExpShotgunPool,
	CTurokMovement.TekBowAmmo						= persist->TekBowAmmo,
	CTurokMovement.MiniGunAmmo						= persist->MiniGunAmmo,
	CTurokMovement.GrenadeLauncherAmmo			= persist->GrenadeLauncherAmmo,
	CTurokMovement.RocketAmmo						= persist->RocketAmmo,
	CTurokMovement.TechWeapon2Ammo				= persist->TechWeapon2Ammo;
	CTurokMovement.ChronoscepterAmmo				= persist->ChronoSceptorAmmo;

	CTurokMovement.WeaponSelectIndex				= persist->WeaponUsing ;

	CTurokMovement.ArmorAmount						= persist->ArmorAmount ;
	CTurokMovement.Level2Keys						= persist->Level2Keys ;
	CTurokMovement.Level3Keys						= persist->Level3Keys ;
	CTurokMovement.Level4Keys						= persist->Level4Keys ;
	CTurokMovement.Level5Keys						= persist->Level5Keys ;
	CTurokMovement.Level6Keys						= persist->Level6Keys ;
	CTurokMovement.Level7Keys						= persist->Level7Keys ;
	CTurokMovement.Level8Keys						= persist->Level8Keys ;
	CTurokMovement.Level2Access					= persist->Level2Access ;
	CTurokMovement.Level3Access					= persist->Level3Access ;
	CTurokMovement.Level4Access					= persist->Level4Access ;
	CTurokMovement.Level5Access					= persist->Level5Access ;
	CTurokMovement.Level6Access					= persist->Level6Access ;
	CTurokMovement.Level7Access					= persist->Level7Access ;
	CTurokMovement.Level8Access					= persist->Level8Access ;

	CTurokMovement.ChronoSceptorPieces			= persist->ChronoSceptorPieces ;


	CTurokMovement.RunWalkToggle					= persist->RunWalk ;
	GetApp()->m_Options.m_RHControl				= persist->ControlSide ;
	GetApp()->m_Options.m_Blood					= persist->Blood ;
//	GetApp()->m_Options.m_Stereo					= persist->Stereo ;
  	GetApp()->m_Options.m_MusicVolume			= persist->MusicVolume ;
  	GetApp()->m_Options.m_SFXVolume				= persist->SFXVolume ;
  	GetApp()->m_Options.m_Opacity					= persist->Opacity ;	
  	GetApp()->m_Options.m_HAnalog					= persist->HAnalog	;
  	GetApp()->m_Options.m_VAnalog					= persist->VAnalog	;

	CTControl__CTControl(GetApp()->m_Options.m_RHControl) ;

	// set music volume
	GetApp()->m_ActualMusicVolume = ((float)GetApp()->m_Options.m_MusicVolume / 255.0);
//	VolChange = GetApp()->m_Options.m_MusicVolume/255.0;
//	__GlobalSEQvolume = (INT16)(VolChange * MAX_SEQ_VOL);
//	alCSPSetVol(seqp, __GlobalSEQvolume);
	// set sfx volume
	GetApp()->m_ActualSFXVolume = ((float)GetApp()->m_Options.m_SFXVolume / 255.0);
//	__GlobalSFXscalar = GetApp()->m_Options.m_SFXVolume/255.0;


//	rmonPrintf("analog read:%d\n", persist->Analog) ;

	GetApp()->m_Difficulty							= persist->Difficulty ;

	CEngineApp__GetPlayer(GetApp())->m_AI.m_Health = persist->Health ;
	CTurokMovement.MaxHealth = persist->MaxHealth ;
	
	CTurokMovement.Tokens							= persist->Tokens ;
	CTurokMovement.Lives								= persist->Lives ;

	//rmonPrintf("load health:%d\n", persist->Health) ;
	//rmonPrintf("load lives:%d\n", persist->Lives) ;

	//GetApp()->m_WarpID								= persist->Checkpoint ;
	CTurokMovement.CurrentCheckpoint				= persist->Checkpoint ;
	GetApp()->m_dwCheatFlags 						= persist->CheatFlags ;
	GetApp()->m_dwEnabledCheatFlags 				= persist->EnabledCheatFlags ;

	GetApp()->m_BestTrainTime		 				= persist->BestTrainTime ;

	GetApp()->m_BossFlags			 				= persist->BossFlags ;
}
















//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//
// ENTER CHEATCODE
//
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------
// CHEATCODE OPTIONS
#define CHEATCODE_WIDTH		(240)
#define CHEATCODE_HEIGHT	(190)
#define CHEATCODE_X			((320/2) - (CHEATCODE_WIDTH/2))
#define CHEATCODE_Y			((240/2) - (CHEATCODE_HEIGHT/2))


// Position of menu selection
#define CHEATCODE_MENUY	(CHEATCODE_Y + 64-6)
#define CHEATCODE_SPACING	20
#define CHEATCODE_SUBWIDTH	24
#define CHEATCODE_GRIDX		(CHEATCODE_X + 72)

#define GIVECHEATCODE_MENUY	(CHEATCODE_Y + CHEATCODE_HEIGHT - 32)


//------------------------------------------------------------------------------
// Data and Structures
//------------------------------------------------------------------------------

u16	cheatcode_data[32] = {0} ;			// used to store hex data
char	cheatcode_string[32] = {0};		// used to store converted password as a string of hex
int	cheatcode_nibbles = 0; 				// how many entries in password
int	cheatcode_nibble = 0; 				// current nibble using
int	cheatcode_bit = 0;	  				// next bit to add
int	cheatcode_active = 0;  				// set once a password has been setup


char cheatcode_entries[16] =
{
	'b', 'c', 'd', 'f',
	'g', 'h', 'j', 'k',
	'l', 'm', 'n', 'p',
	'q', 'r', 's', 't',
} ;


typedef struct s_CheatCodeEntry
{
	int	items ;
	int	Spacing ;
	char	*String ;
} t_CheatCodeEntry ;


t_CheatCodeEntry cheatcode_text[]=
{
	{4, CHEATCODE_SPACING-2, "b c d f"},
	{4, CHEATCODE_SPACING-2, "g h j k"},
	{4, CHEATCODE_SPACING-2, "l m n p"},
	{4, CHEATCODE_SPACING-2, "q r s t"},
	{0, CHEATCODE_SPACING, text_delete},
	{0, CHEATCODE_SPACING, text_enter},
	{0, CHEATCODE_SPACING, text_exit},
} ;


typedef struct s_cheatcode
{
	char	*string ;
	int	length ;
	char	*code ;
	u32	flags ;
} t_cheatcode ;


#define _b	0
#define _c	1
#define _d	2
#define _f	3
#define _g	4
#define _h	5
#define _j	6
#define _k	7
#define _l	8
#define _m	9
#define _n	10
#define _p	11
#define _q	12
#define _r	13
#define _s	14
#define _t	15

void *pCheatCodeFont[16]=
{
	FontB,
	FontC,
	FontD,
	FontF,
	FontG,
	FontH,
	FontJ,
	FontK,
	FontL,
	FontM,
	FontN,
	FontP,
	FontQ,
	FontR,
	FontS,
	FontT,
} ;

char cht_invincible[] =	{_d,_d,_d,_n,_s,_t,_b,_r};
char cht_spirit[] =		{_n,_d,_n,_l,_p};
char cht_weapons[] =		{_t,_h,_m,_s,_c,_l,_s};
char cht_ammo[] =			{_l,_n,_j,_h,_n,_s,_n};
char cht_bigheads[] =	{_r,_b,_c,_h,_n};
char cht_allmap[] =		{_g,_n,_n,_t,_r,_t,_m,_n,_t};
char cht_tinyenemy[] =	{_j,_s,_n,_c,_r,_p,_n,_t,_r};
char cht_penandink[] =	{_r,_b,_b,_m,_l,_l,_r};
char cht_colors[] =		{_c,_r,_l,_s,_f,_n,_d,_n,_g,_s};
char cht_levels1[] =		{_m,_t,_t,_s,_t,_b,_b,_n,_g,_t,_n};
char cht_levels2[] =		{_d,_r,_r,_n,_s,_t,_b,_b,_n,_g,_t,_n};
char cht_levels3[] =		{_m,_k,_j,_n,_k};
char cht_levels4[] =		{_p,_t,_n,_d,_n,_c,_n};
char cht_levels5[] =		{_n,_n,_t,_n,_d};
char cht_levels6[] =		{_t,_r,_m,_n,_n};
char cht_levels7[] =		{_j,_m,_d,_n,_n};
char cht_levels8[] =		{_m,_r,_g,_r,_t,_t,_r,_r};
char cht_gallery[] =		{_s,_h,_n,_t,_r,_r,_n,_t};
char cht_credits[] =		{_t,_h,_t,_r,_t,_m};
char cht_infinite[] = 	{_j,_f,_f,_s,_p,_n,_g,_n,_b,_r,_g};
char cht_quackmode[] = 	{_d,_n,_l,_d,_d,_c,_k};
char cht_allkeys[] = 	{_n,_g,_l,_c,_k,_c,_k};
char cht_longhunter[] =	{_j,_h,_n,_t,_h,_m,_s};
char cht_mantis[] =		{_s,_t,_p,_h,_n,_b,_r,_m,_l};
char cht_trex[] =			{_g,_r,_g,_m,_l,_c,_h,_c,_k};
char cht_campaigner[] =	{_r,_n,_t,_r,_c};
char cht_flymode[] =		{_c,_r,_t,_r,_g,_b,_r,_b,_n,_s,_m,_t,_h};
#ifndef KANJI
char cht_discomode[] = 	{_s,_n,_f};
#endif

#ifdef ENGLISH
char cht_gregmode[] = 	{_g,_r,_g,_c,_h,_n};
char text_gregmode[] = {"greg mode"} ;
char cht_danamode[] = 	{_d,_n,_c,_h,_n};
char text_danamode[] = {"dana mode"} ;
char cht_rhiamode[] = 	{_f,_j,_h,_j,_j};
char text_rhiamode[] = {"rhia jam"} ;
char cht_amymode[] = 	{_g,_l,_t,_b,_n,_k};
char text_amymode[] = {"amy jam"} ;
char cht_robinmode[] = 	{_r,_b,_n,_s,_m,_t,_h};
char text_robinmode[] = {"robins cheat"} ;
char cht_garymode[] = 	{_g,_r,_h,_r,_n,_r};
char text_garymode[] = {"garys cheat"} ;
char cht_bigcheat[] = 	{_m,_s,_t,_r,_b,_g,_s,_h,_t,_p,_r,_g,_r,_m,_m,_r};
char text_bigcheat[] = {"the big cheat"} ;
#endif
//char cht_bigenemy[] =	{_n,_t,_h,_n,_p,_l,_m,_b};
//char cht_allenemies[] =	{_c,_c,_l,_m,_n,_c};

//	{text_showallenemies,	sizeof(cht_allenemies),	cht_allenemies,	CHEATFLAG_SHOWALLENEMIES},	// 6
//	{text_bigenemymode,		sizeof(cht_bigenemy),	cht_bigenemy,		CHEATFLAG_BIGENEMYMODE},	// 8

// if you need to change the order of this table, see Carl first....
t_cheatcode	cheatcodes[] =
{
	{text_invincibility,		sizeof(cht_invincible),	cht_invincible,	CHEATFLAG_INVINCIBILITY},	// 0
	{text_spiritmode,			sizeof(cht_spirit),		cht_spirit,			CHEATFLAG_SPIRITMODE},		// 1
	{text_allweapons,			sizeof(cht_weapons),		cht_weapons,		CHEATFLAG_ALLWEAPONS},		// 2
	{text_unlimitedammo,		sizeof(cht_ammo),			cht_ammo,			CHEATFLAG_UNLIMITEDAMMO},	// 3
	{text_bigheads,			sizeof(cht_bigheads),	cht_bigheads,		CHEATFLAG_BIGHEADS},			// 4
	{text_allmap,				sizeof(cht_allmap),		cht_allmap,			CHEATFLAG_ALLMAP},			// 5
	{text_tinyenemymode,		sizeof(cht_tinyenemy),	cht_tinyenemy,		CHEATFLAG_TINYENEMYMODE},	// 6
	{text_penandink,			sizeof(cht_penandink),	cht_penandink,		CHEATFLAG_PENANDINKMODE},	// 7
	{text_purdycolors,		sizeof(cht_colors),		cht_colors,			CHEATFLAG_PURDYCOLORS},		// 8
	{text_levelwarp1,			sizeof(cht_levels1),		cht_levels1,		CHEATFLAG_WARPLEVEL1},		// 9
	{text_levelwarp2,			sizeof(cht_levels2),		cht_levels2,		CHEATFLAG_WARPLEVEL2},		// 10
	{text_levelwarp3,			sizeof(cht_levels3),		cht_levels3,		CHEATFLAG_WARPLEVEL3},		// 11
	{text_levelwarp4,			sizeof(cht_levels4),		cht_levels4,		CHEATFLAG_WARPLEVEL4},		// 12
	{text_levelwarp5,			sizeof(cht_levels5),		cht_levels5,		CHEATFLAG_WARPLEVEL5},		// 13
	{text_levelwarp6,			sizeof(cht_levels6),		cht_levels6,		CHEATFLAG_WARPLEVEL6},		// 14
	{text_levelwarp7,			sizeof(cht_levels7),		cht_levels7,		CHEATFLAG_WARPLEVEL7},		// 15
	{text_levelwarp8,			sizeof(cht_levels8),		cht_levels8,		CHEATFLAG_WARPLEVEL8},		// 16
	{text_gallery,				sizeof(cht_gallery),		cht_gallery,		CHEATFLAG_GALLERY},			// 17
	{text_showcredits,		sizeof(cht_credits),		cht_credits,		CHEATFLAG_SHOWCREDITS},		// 18
	{text_infinitelives, 	sizeof(cht_infinite),	cht_infinite,		CHEATFLAG_INFINITELIVES},	// 19
	{text_quackmode, 			sizeof(cht_quackmode),	cht_quackmode,		CHEATFLAG_QUACKMODE},		// 20
	{text_allkeys, 			sizeof(cht_allkeys),		cht_allkeys,		CHEATFLAG_ALLKEYS},			// 21
	{text_longhunterwarp, 	sizeof(cht_longhunter),	cht_longhunter,	CHEATFLAG_WARPLONGHUNTER},	// 22
	{text_mantiswarp, 		sizeof(cht_mantis),		cht_mantis,			CHEATFLAG_WARPMANTIS},		// 23
	{text_trexwarp, 			sizeof(cht_trex),			cht_trex,			CHEATFLAG_WARPTREX},			// 24
	{text_campaignerwarp,	sizeof(cht_campaigner),	cht_campaigner,	CHEATFLAG_WARPCAMPAIGNER},	// 25
	{text_flymode,				sizeof(cht_flymode),		cht_flymode,		CHEATFLAG_FLYMODE},			// 26
#ifndef KANJI
	{text_discomode, 			sizeof(cht_discomode),	cht_discomode,		CHEATFLAG_DISCO},				// 26	MUST BE LAST
#endif
#ifdef ENGLISH
	{text_gregmode, 			sizeof(cht_gregmode),	cht_gregmode,		CHEATFLAG_BIGHEADS | CHEATFLAG_ALLWEAPONS | CHEATFLAG_UNLIMITEDAMMO | CHEATFLAG_SHOWCREDITS},
	{text_danamode, 			sizeof(cht_danamode),	cht_danamode,		CHEATFLAG_TINYENEMYMODE | CHEATFLAG_SHOWCREDITS},
	{text_rhiamode, 			sizeof(cht_rhiamode),	cht_rhiamode,		CHEATFLAG_DISCO | CHEATFLAG_PURDYCOLORS},
	{text_amymode, 			sizeof(cht_amymode),		cht_amymode,		CHEATFLAG_DISCO | CHEATFLAG_PURDYCOLORS},
	{text_robinmode, 			sizeof(cht_robinmode),	cht_robinmode,		CHEATFLAG_BIGHEADS | CHEATFLAG_INVINCIBILITY | CHEATFLAG_ALLWEAPONS | CHEATFLAG_UNLIMITEDAMMO | CHEATFLAG_SHOWCREDITS},
	{text_garymode, 			sizeof(cht_garymode),	cht_garymode,		CHEATFLAG_INVINCIBILITY | CHEATFLAG_ALLWEAPONS | CHEATFLAG_UNLIMITEDAMMO | CHEATFLAG_SHOWCREDITS },
	{text_bigcheat, 			sizeof(cht_bigcheat),	cht_bigcheat,		CHEATFLAG_INVINCIBILITY |
																							CHEATFLAG_SPIRITMODE |
																							CHEATFLAG_ALLWEAPONS |
																							CHEATFLAG_UNLIMITEDAMMO |
																							CHEATFLAG_BIGHEADS |
																							CHEATFLAG_ALLMAP |
																							CHEATFLAG_INFINITELIVES |
																							CHEATFLAG_WARPLEVEL1 |
																							CHEATFLAG_WARPLEVEL2 |
																							CHEATFLAG_WARPLEVEL3 |
																							CHEATFLAG_WARPLEVEL4 |
																							CHEATFLAG_WARPLEVEL5 |
																							CHEATFLAG_WARPLEVEL6 |
																							CHEATFLAG_WARPLEVEL7 |
																							CHEATFLAG_WARPLEVEL8 |
																							CHEATFLAG_WARPLONGHUNTER |
																							CHEATFLAG_WARPMANTIS |
																							CHEATFLAG_WARPTREX |
																							CHEATFLAG_WARPCAMPAIGNER},
#endif
} ;


void CCheatCode__ClearData(void) ;
void CCheatCode__DrawString(Gfx **ppDLP, int y, int opacity) ;
void CCheatCode__ConvertDataToString(void) ;
void CCheatCode__ConvertStringToData(void) ;

/*****************************************************************************
*
*	Functions for:		CCheatCode class
*
******************************************************************************
*
*	Description:	routines to facilitate the entering of cheats
*
*****************************************************************************/

//------------------------------------------------------------------------
// Construct structure
//------------------------------------------------------------------------
void CCheatCode__Construct(CCheatCode *pThis)
{
	// Setup default selection
	pThis->m_Selection = 0 ;

	if (pThis->m_Mode == CHEATCODE_NULL)
		pThis->m_Alpha = 0 ;

	GetApp()->m_bMessageBox = FALSE ;

	pThis->m_Mode = CHEATCODE_FADEUP ;
	pThis->b_Active = TRUE ;

	pThis->m_EntryPos = cheatcode_nibbles ;
	pThis->m_Selection = CHEATCODE_EXIT ;
}


//------------------------------------------------------------------------
// Update selection
//------------------------------------------------------------------------
//------------------------------------------------------------------------
void CCheatCode__PasswordUP(CCheatCode *pThis)
{
	BarTimer = 0 ;
	pThis->m_Selection-- ;
	if (pThis->m_Selection < 0)
		pThis->m_Selection = CHEATCODE_END_SELECTION-1 ;

	if (cheatcode_text[pThis->m_Selection].items != 0)
	{
		if (pThis->m_SubSelection >= cheatcode_text[pThis->m_Selection].items)
			pThis->m_SubSelection = cheatcode_text[pThis->m_Selection].items -1;
	}
	else
 		pThis->m_SubSelection = 0 ;
}

void CCheatCode__PasswordDOWN(CCheatCode *pThis)
{
	BarTimer = 0 ;
	pThis->m_Selection++ ;
	if (pThis->m_Selection >= CHEATCODE_END_SELECTION)
		pThis->m_Selection = 0 ;

	if (cheatcode_text[pThis->m_Selection].items != 0)
	{
		if (pThis->m_SubSelection >= cheatcode_text[pThis->m_Selection].items)
			pThis->m_SubSelection = cheatcode_text[pThis->m_Selection].items -1;
	}
	else
 		pThis->m_SubSelection = 0 ;
}

void CCheatCode__PasswordLEFT(CCheatCode *pThis)
{
	BarTimer = 0 ;
	pThis->m_SubSelection-- ;
	if (pThis->m_SubSelection < 0)
	{
		pThis->m_SubSelection = 0 ;

		if (pThis->m_Selection > 0)
		{
			if (cheatcode_text[pThis->m_Selection-1].items)
				pThis->m_SubSelection = cheatcode_text[pThis->m_Selection-1].items-1 ;
			CCheatCode__PasswordUP(pThis) ;
		}
	}
}

void CCheatCode__PasswordRIGHT(CCheatCode *pThis)
{
	BarTimer = 0 ;
	pThis->m_SubSelection++ ;
	if (pThis->m_SubSelection >=  cheatcode_text[pThis->m_Selection].items)
	{
		pThis->m_SubSelection = 0 ;
  		CCheatCode__PasswordDOWN(pThis) ;
	}
}


BOOL CheckCheatCode(int numb)
{
	int	c ;

	for (c=0; c<cheatcodes[numb].length; c++)
	{
		if (cheatcode_string[c] != cheatcodes[numb].code[c])
			return FALSE ;
	}
	return TRUE ;
}

INT32 CCheatCode__Update(CCheatCode *pThis)
{
	INT32				ReturnValue = -1 ;
	INT32				i, cheat ;

	if (GetApp()->m_bMessageBox)
	{
		ReturnValue = CMessageBox__Update(&GetApp()->m_MessageBox) ;
		if (ReturnValue != -1)
		{
			GetApp()->m_bMessageBox = FALSE ;
			ReturnValue = -1 ;
		}
	}

	else
	{
		// Goto next selection?
		if (CEngineApp__MenuDown(GetApp()))
			CCheatCode__PasswordDOWN(pThis) ;

		// Goto previous selection?
		if (CEngineApp__MenuUp(GetApp()))
			CCheatCode__PasswordUP(pThis) ;


		// any sub items
//		if (loadpassword_text[pThis->m_Selection].items)
//		{
			// Goto left selection?
			if (CEngineApp__MenuLeft(GetApp()))
   			CCheatCode__PasswordLEFT(pThis) ;

			// Goto right selection?
			if (CEngineApp__MenuRight(GetApp()))
				CCheatCode__PasswordRIGHT(pThis) ;
//		}



		// Exit if start pressed
		// returns to game when exited
		if (CTControl__IsUseMenu(pCTControl))
		{
			ReturnValue = pThis->m_Selection ;

#define	MAX_CHEATCODE		16			// MAKE THIS EVEN!!!
			if (cheatcode_nibbles == MAX_CHEATCODE)
				pThis->m_EntryPos = cheatcode_nibbles-1 ;

			switch (ReturnValue)
			{
				case CHEATCODE_GRID1:
					cheatcode_string[pThis->m_EntryPos] = 0 + pThis->m_SubSelection ;
					if (cheatcode_nibbles < MAX_CHEATCODE)
					{
						cheatcode_nibbles++ ;
						pThis->m_EntryPos++ ;
					}
					ReturnValue = -1 ;
					break ;
				case CHEATCODE_GRID2:
					cheatcode_string[pThis->m_EntryPos] = 4 + pThis->m_SubSelection ;
					if (cheatcode_nibbles < MAX_CHEATCODE)
					{
						cheatcode_nibbles++ ;
						pThis->m_EntryPos++ ;
					}
					ReturnValue = -1 ;
					break ;
				case CHEATCODE_GRID3:
					cheatcode_string[pThis->m_EntryPos] = 8 + pThis->m_SubSelection ;
					if (cheatcode_nibbles < MAX_CHEATCODE)
					{
						cheatcode_nibbles++ ;
						pThis->m_EntryPos++ ;
					}
					ReturnValue = -1 ;
					break ;
				case CHEATCODE_GRID4:
					cheatcode_string[pThis->m_EntryPos] = 12 + pThis->m_SubSelection ;
					if (cheatcode_nibbles < MAX_CHEATCODE)
					{
						cheatcode_nibbles++ ;
						pThis->m_EntryPos++ ;
					}
					ReturnValue = -1 ;
					break ;

				
				case CHEATCODE_DELETE:
					if (pThis->m_EntryPos)
					{
						pThis->m_EntryPos-- ;
						cheatcode_nibbles-- ;
					}
					ReturnValue = -1 ;
					break ;

				case CHEATCODE_ENTER:
					// clear the data
					CCheatCode__ClearData() ;

					// convert current string to data
					CCheatCode__ConvertStringToData() ;

					// search through all cheat codes
					cheat = -1 ;
					for (i=0; i<(sizeof(cheatcodes)/sizeof(t_cheatcode)); i++)
					{
						if (cheatcode_nibbles == cheatcodes[i].length)
						{
							if (CheckCheatCode(i))
							{
								cheat = i ;
								break ;
							}
						}
					}
					// activate cheat?
					if (cheat != -1)
					{
						GetApp()->m_dwEnabledCheatFlags |= cheatcodes[cheat].flags ;
						CMessageBox__Construct(&GetApp()->m_MessageBox,
													cheatcodes[cheat].string, text_activated) ;
						GetApp()->m_bMessageBox = TRUE ;
					}
					else
					{
						CMessageBox__Construct(&GetApp()->m_MessageBox,
													text_invalid, text_code) ;
						GetApp()->m_bMessageBox = TRUE ;
					}
					ReturnValue = -1 ;
					break ;

				case CHEATCODE_EXIT:
					pThis->m_Mode = CHEATCODE_FADEDOWN ;
					break ;
			}
		}
	}


	return ReturnValue ;
}



//------------------------------------------------------------------------
// Draw screen information
//------------------------------------------------------------------------
void CCheatCode__Draw(CCheatCode *pThis, Gfx **ppDLP)
{
	int		i, y ;
	int		x, w ;

	// update fade status
	switch (pThis->m_Mode)
	{
		case CHEATCODE_FADEUP:
			pThis->m_Alpha += .2 * frame_increment ;
			if (pThis->m_Alpha >= 1.0)
			{
				pThis->m_Alpha = 1.0 ;
				pThis->m_Mode = CHEATCODE_NORMAL ;
			}
			break ;

		case CHEATCODE_FADEDOWN:
			pThis->m_Alpha -= .2 * frame_increment;
			if (pThis->m_Alpha <= 0.0)
			{
				pThis->m_Alpha = 0.0 ;
				pThis->m_Mode = CHEATCODE_NULL ;
				if (pThis->b_Active == FALSE)
					GetApp()->m_bCheatCode = FALSE ;
			}
			break ;
	}

	// prepare to draw boxes
	COnScreen__InitBoxDraw(ppDLP) ;

	// draw box
	COnScreen__DrawHilightBox(ppDLP,
							 CHEATCODE_X, CHEATCODE_Y,
							 CHEATCODE_X+CHEATCODE_WIDTH, CHEATCODE_Y+CHEATCODE_HEIGHT,
							 2, FALSE, 
							 0,0,0, 180 * pThis->m_Alpha) ;


	// draw current selection box
	y = CHEATCODE_MENUY ;
	i = 0;
	while (i!=pThis->m_Selection)
		y += cheatcode_text[i++].Spacing ;

	// each letter in grid
	if (cheatcode_text[pThis->m_Selection].items == 4)
	{
		x = CHEATCODE_GRIDX + (pThis->m_SubSelection * CHEATCODE_SUBWIDTH) ;
		w = CHEATCODE_SUBWIDTH ;
	}
	// regular bar
	else
	{
		x = CHEATCODE_X +16 ;
		w = CHEATCODE_WIDTH-32 ;
	}

	CPause__InitPolygon(ppDLP) ;
	CPause__DrawBar(ppDLP, x, y,
						w, 18, pThis->m_Alpha) ;


	// prepare to draw textures
	COnScreen__InitFontDraw(ppDLP) ;

	// draw heading
	COnScreen__SetFontColor(ppDLP,
									200, 0, 200,
									0, 200, 0) ;
	COnScreen__DrawText(ppDLP,
							  text_entercheatcode,
							  320/2, 30,
							  (int)(255 * pThis->m_Alpha), TRUE, TRUE) ;



	// write the current password
	CCheatCode__DrawString(ppDLP, 30+20, (int)(255 * pThis->m_Alpha)) ;

	// draw each option
	COnScreen__InitFontDraw(ppDLP) ;

	y = CHEATCODE_MENUY ;
	for (i=0; i<CHEATCODE_END_SELECTION; i++)
	{
#ifdef KANJI
		if (i<4)
			COnScreen__PrepareLargeFont(ppDLP);
		else
			COnScreen__PrepareKanjiFont(ppDLP);
#endif

		COnScreen__DrawText(ppDLP,
								  cheatcode_text[i].String,
								  320/2, y,
								  (int)(255 * pThis->m_Alpha), TRUE, TRUE) ;
		y += cheatcode_text[i].Spacing ;
	}

	// draw a message box if needed
	if (GetApp()->m_bMessageBox)
		CMessageBox__Draw(&GetApp()->m_MessageBox, ppDLP) ;
}



//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//
// GIVE CHEATCODE
//
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

/*****************************************************************************
*
*	Functions for:		CGiveCheatCode class
*
******************************************************************************
*
*	Description:	routines to facilitate the giving of cheats
*
*****************************************************************************/

//------------------------------------------------------------------------
// Construct structure
//------------------------------------------------------------------------
void CGiveCheatCode__Construct(CGiveCheatCode *pThis, int *Codes)
{
	// Setup default selection
	pThis->m_Selection = 0 ;

	if (pThis->m_Mode == CHEATCODE_NULL)
		pThis->m_Alpha = 0 ;

	pThis->m_Codes = Codes ;

	pThis->m_Mode = CHEATCODE_FADEUP ;
	pThis->b_Active = TRUE ;
}


//------------------------------------------------------------------------
// Update selection
//------------------------------------------------------------------------
//------------------------------------------------------------------------
INT32 CGiveCheatCode__Update(CGiveCheatCode *pThis)
{
	INT32				ReturnValue = -1 ;

	// Exit if start pressed
	if (CTControl__IsUseMenu(pCTControl))
	{
		ReturnValue = pThis->m_Selection ;
		pThis->m_Mode = CHEATCODE_FADEDOWN ;
	}

	return ReturnValue ;
}



//------------------------------------------------------------------------
// Draw screen information
//------------------------------------------------------------------------
void CGiveCheatCode__Draw(CGiveCheatCode *pThis, Gfx **ppDLP)
{
	int		*code ;
	int		count ;
	int		c, x, y ;

	// update fade status
	switch (pThis->m_Mode)
	{
		case CHEATCODE_FADEUP:
			pThis->m_Alpha += .2 * frame_increment ;
			if (pThis->m_Alpha >= 1.0)
			{
				pThis->m_Alpha = 1.0 ;
				pThis->m_Mode = CHEATCODE_NORMAL ;
			}
			break ;

		case CHEATCODE_FADEDOWN:
			pThis->m_Alpha -= .2 * frame_increment;
			if (pThis->m_Alpha <= 0.0)
			{
				pThis->m_Alpha = 0.0 ;
				pThis->m_Mode = CHEATCODE_NULL ;
				if (pThis->b_Active == FALSE)
					GetApp()->m_bGiveCheatCode = FALSE ;
			}
			break ;
	}

	// prepare to draw boxes
	COnScreen__InitBoxDraw(ppDLP) ;

	// draw box
	COnScreen__DrawHilightBox(ppDLP,
							 CHEATCODE_X, CHEATCODE_Y,
							 CHEATCODE_X+CHEATCODE_WIDTH, CHEATCODE_Y+CHEATCODE_HEIGHT,
							 2, FALSE, 
							 0,0,0, 180 * pThis->m_Alpha) ;


	CPause__InitPolygon(ppDLP) ;
	CPause__DrawBar(ppDLP, (320/2)-64, GIVECHEATCODE_MENUY,
						128, 18, pThis->m_Alpha) ;


	// prepare to draw textures
	COnScreen__InitFontDraw(ppDLP) ;

	// draw heading
	COnScreen__SetFontColor(ppDLP,
									50, 200, 50,
									200, 200, 50) ;
	COnScreen__DrawText(ppDLP,
							  text_cheatcode,
							  320/2, CHEATCODE_Y+8,
							  (int)(255 * pThis->m_Alpha), TRUE, TRUE) ;


	// draw the cheatcodes
	COnScreen__InitFontDraw(ppDLP) ;
	if (pThis->m_Codes)
	{
		count = 0 ;
		code = pThis->m_Codes ;
		while (*code != -1)
		{
			count++ ;
			code++ ;
		}

		y = 240/2 ; //(CHEATCODE_Y + CHEATCODE_HEIGHT) / 2;
		y -= (count * (18*2))/2 ;

		code = pThis->m_Codes ;
		while (*code != -1)
		{
#ifdef KANJI
			COnScreen__PrepareKanjiFont(ppDLP) ;
#endif
			COnScreen__DrawText(ppDLP, cheatcodes[*code].string, 320/2, y, (int)(255 * pThis->m_Alpha), TRUE, TRUE) ;
			y += 16 ;

			// enable the cheatcode
			GetApp()->m_dwEnabledCheatFlags |= cheatcodes[*code].flags ;

#ifdef KANJI
			COnScreen__PrepareLargeFont(ppDLP) ;
#endif
			// draw the code
			x = cheatcodes[*code].length * 16 ;
			x /= 2 ;
			x = (320/2) - x ;
			for (c=0; c<cheatcodes[*code].length; c++)
			{
				COnScreen__DrawFontTexture(ppDLP, pCheatCodeFont[cheatcodes[*code].code[c]], x, y, 1.0, 1.0) ;
				x+=16 ;
			}
			y+=20 ;

			code++ ;
		}
	}


	// draw each option
	COnScreen__InitFontDraw(ppDLP) ;
	COnScreen__DrawText(ppDLP,
							  text_ok,
							  320/2, GIVECHEATCODE_MENUY,
							  (int)(255 * pThis->m_Alpha), TRUE, TRUE) ;
}




//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//
// CHEATCODE DATA STUFF
//
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

// maximum characters of password to display per line
#define	MAX_CHEATCODE_LINE		18

//-----------------------------------------------------------------------------------------------------
// draw a line of the password
//-----------------------------------------------------------------------------------------------------
void CCheatCode__DrawLine(Gfx **ppDLP, int y, int opacity, int chars, int offset)
{
	int	x, i ;
	char	string[2] ;

	// printing 1 char at a time, so make a null terminator
	string[1] = '\0' ;

	// centre x
	x = 320 /2 ;
	x -= (chars *12) /2 ;

	for (i=0; i<chars; i++)
	{
		string[0] = cheatcode_entries[cheatcode_string[i+offset]] ;

		// if in load mode, flash current letter
		if ((i+offset) == cheatcode_nibbles-1)
		{
			if ((frame_number&7)<4)
			{
				COnScreen__DrawText(ppDLP, string,
									  x, y, opacity, FALSE, TRUE) ;
			}
		}
		else
			COnScreen__DrawText(ppDLP, string,
								  x, y, opacity, FALSE, TRUE) ;
		x += 12 ;
	}
}

//-----------------------------------------------------------------------------------------------------
// draw current cheatcode, character at a time
//-----------------------------------------------------------------------------------------------------
void CCheatCode__DrawString(Gfx **ppDLP, int y, int opacity)
{
	int	count ;
	int	offset = 0;

	COnScreen__SetFontScale(1.0, 1.0) ;
	COnScreen__SetFontColor(ppDLP,
									255, 255, 255,
									255, 0, 255) ;

	COnScreen__PrepareLargeFont(ppDLP);
	// single line
	if (cheatcode_nibbles < MAX_CHEATCODE_LINE)
		CCheatCode__DrawLine(ppDLP, y, opacity, cheatcode_nibbles, 0) ;

	// multi line
	else
	{
		count = cheatcode_nibbles ;
		while (count >MAX_CHEATCODE_LINE)
		{
			CCheatCode__DrawLine(ppDLP, y, opacity, MAX_CHEATCODE_LINE, offset) ;
			y += 16 ;
			offset += MAX_CHEATCODE_LINE ;
			count -= MAX_CHEATCODE_LINE ;
		}
		CCheatCode__DrawLine(ppDLP, y, opacity, count, offset) ;
	}
}


//-----------------------------------------------------------------------------------------------------
// Clear data, and positional vars
//-----------------------------------------------------------------------------------------------------
void CCheatCode__ClearData(void)
{
	int	i ;
	cheatcode_nibble = 0 ;
	cheatcode_bit = 0 ;

	for (i=0; i<sizeof(cheatcode_data) / sizeof(u16); i++)
		cheatcode_data[i] = 0 ;
}

//----------------------------------------------------------
// Convert the data to a printable string
//----------------------------------------------------------
void CCheatCode__ConvertDataToString(void)
{
	u16	dcount = 0;
	u16	scount = 0;
	u16	byte ;
	int	nibble = 0 ;
	BOOL	working = TRUE ;

	while (working)
	{
		// read a byte from data
		byte = cheatcode_data[dcount] ;

		// convert to nibble
		switch (nibble)
		{
			case 0:
				byte >>= 12 ;
				break ;
			case 1:
				byte >>= 8 ;
				break ;
			case 2:
				byte >>= 4 ;
				break ;
		}
		byte &= 0x0f ;

		// store converted nibble in string
		cheatcode_string[scount] = byte ;

		// move along in nibbles, move along data if nibble 0
		nibble++ ;
		if (nibble >3)
		{
			nibble = 0 ;
			dcount++ ;
		}

		// advance string position and check for end
		scount++ ;
		if (scount >= cheatcode_nibbles)
			working = FALSE ;
	}
}

//----------------------------------------------------------
// Convert the string back to data
//----------------------------------------------------------
void CCheatCode__ConvertStringToData(void)
{
	u16	dcount = 0;
	u16	scount = 0;
	u16	byte ;
	u16	data ;
	int	nibble = 0 ;
	BOOL	working = TRUE ;

	while (working)
	{
		// read a byte from string
		data = cheatcode_string[scount] & 0x0f ;

		// convert to nibble
		switch (nibble)
		{
			case 0:
				byte = (data<<12) ;
				break ;
			case 1:
				byte |= (data<<8) ;
				break ;
			case 2:
				byte |= (data<<4) ;
				break ;
			case 3:
				byte |= (data) ;
				break ;
		}

		// store converted nibble in data
		cheatcode_data[dcount] = byte ;

		// move along in nibbles, move along data if nibble 0
		nibble++ ;
		if (nibble >3)
		{
			nibble = 0 ;
			dcount++ ;
		}

		// advance string position and check for end
		scount++ ;
		if (scount >= cheatcode_nibbles)
			working = FALSE ;
	}
}





/*****************************************************************************
*
*	Functions for:		CGamePak class
*
******************************************************************************
*
*	Description:	routines to manage the controller pak
*
*****************************************************************************/


//-----------------------------------------------------------------------------------------------------
// Construct GAMEPAK structure
//-----------------------------------------------------------------------------------------------------
void CGamePak__Construct(CGamePak *pThis)
{
	int	ret ;

	// if setting up the file system causes an error, quit
	ret = SetupFileSystem() ;
	if (ret ==0)	// != PFS_ERR_ID_FATAL)
	{
		pThis->m_Selection = 0 ;
		pThis->m_Timer = 0 ;

		pThis->m_Alpha = 0 ;

		pThis->m_Mode = GAMEPAK_FADEUP ;
		pThis->b_Active = TRUE ;

		GetApp()->m_LegalBypass = TRUE ;

		ScanFileList(TRUE) ;
		if (FileDirectory[pThis->m_Selection].pages == 0)
			gamepak_status = 0;
		else
			gamepak_status = 1;
	}
	else
		pThis->b_Active = FALSE ;
}
		


//------------------------------------------------------------------------
// Update selection on gamepak manager screen
//------------------------------------------------------------------------
INT32 CGamePak__Update(CGamePak *pThis)
{
//	int				ret ;
	INT32				ReturnValue = -1 ;


	// Goto next selection?
	if (CEngineApp__MenuDown(GetApp()))
	{
		pThis->m_Selection++ ;
		if (pThis->m_Selection > 15)
			pThis->m_Selection = 0 ;
		pThis->m_Timer = 0 ;

		if (FileDirectory[pThis->m_Selection].pages == 0)
			gamepak_status = 0;
		else
			gamepak_status = 1;
	}

	// Goto previous selection?
	if (CEngineApp__MenuUp(GetApp()))
	{
		pThis->m_Selection-- ;
		if (pThis->m_Selection < 0)
		{
			pThis->m_Selection = 15 ;
		}
		pThis->m_Timer = 0 ;

		if (FileDirectory[pThis->m_Selection].pages == 0)
			gamepak_status = 0;
		else
			gamepak_status = 1;
	}

	// confirm delete?
	if ((gamepak_status == 2) && (CTControl__IsSelectWeaponPrev(pCTControl)))
	{
		DeleteCurrent(pThis->m_Selection) ;
		ScanFileList(TRUE) ;
		if (FileDirectory[pThis->m_Selection].pages == 0)
			gamepak_status = 0;
		else
			gamepak_status = 1;
	}

	// check for delete
	if ((gamepak_status == 1) && (CTControl__IsSelectWeaponNext(pCTControl)))
		gamepak_status = 2 ;

	if (CTControl__IsStart(pCTControl))
		ReturnValue = 1 ;

	pThis->m_Timer++ ;

	return ReturnValue ;
}



//------------------------------------------------------------------------
// Draw gamepak screen information
//------------------------------------------------------------------------
/*
#define	MAX_PAUSE_VTXS		16
extern	Vtx	pause_vtxs[2][MAX_PAUSE_VTXS];		// enough for 6 triangles
extern	Mtx	mtx_pause;
*/

void CGamePak__Draw(CGamePak *pThis, Gfx **ppDLP)
{
	int		/*e,*/ i, y ;
	float		intensity ;
	char		buffer[32] ;

/*	Vtx 			*vtxs ;*/

	// update fade status
	switch (pThis->m_Mode)
	{
		case GAMEPAK_FADEUP:
			pThis->m_Alpha += .1 * frame_increment ;
			if (pThis->m_Alpha >= 1.0)
			{
				pThis->m_Alpha = 1.0 ;
				pThis->m_Mode = GAMEPAK_NORMAL ;
			}
			break ;

		case GAMEPAK_FADEDOWN:
			pThis->m_Alpha -= .1 * frame_increment ;
			if (pThis->m_Alpha <= 0.0)
			{
				pThis->m_Alpha = 0.0 ;
				pThis->m_Mode = GAMEPAK_NULL ;
			}
			break ;
	}

	COnScreen__InitBoxDraw(ppDLP) ;
	COnScreen__DrawHilightBox(ppDLP,
							 GAMEPAK_X, GAMEPAK_Y,
							 GAMEPAK_X+GAMEPAK_WIDTH, GAMEPAK_Y+GAMEPAK_HEIGHT,
							 1, FALSE, 
							 50,30,30, 150 * pThis->m_Alpha) ;

	// draw heading
	COnScreen__InitFontDraw(ppDLP) ;
	COnScreen__SetFontScale(1.0, 1.0) ;
	COnScreen__PrepareSmallFont(ppDLP) ;
	GetApp()->m_OnScreen.m_ShadowXOff = -4 ;
	GetApp()->m_OnScreen.m_ShadowYOff = 4 ;

	COnScreen__DrawText(ppDLP, "    note names        pages", GAMEPAK_X, GAMEPAK_Y+8, (int)(255 * pThis->m_Alpha), FALSE, TRUE) ;

	// draw note names
	y = 0 ;
	for (i=0; i<16; i++)
	{
		intensity = 192+(63 * cos((float)pThis->m_Timer/8)) ;
		if (i==pThis->m_Selection)
			COnScreen__SetFontColor(ppDLP, intensity, intensity, intensity, intensity, intensity, intensity) ;
		else
			COnScreen__SetFontColor(ppDLP, 100, 100, 100, 100, 100, 100) ;

		COnScreen__PrepareNintendoFont(ppDLP) ;
		COnScreen__DrawText(ppDLP,
								  FileDirectory[i].string,
								  GAMEPAK_X+32, GAMEPAKMENU_Y+(y*8),
								  (int)(255 * pThis->m_Alpha), FALSE, TRUE) ;

		COnScreen__PrepareSmallFont(ppDLP) ;

		if (i==pThis->m_Selection)
			COnScreen__SetFontColor(ppDLP, 0, intensity, intensity, 0, intensity, intensity) ;
		else
			COnScreen__SetFontColor(ppDLP, 0, 150, 150, 0, 150, 150) ;

		sprintf(buffer, "%2d:", i+1) ;
		COnScreen__DrawText(ppDLP,
								  buffer,
								  GAMEPAK_X+8, GAMEPAKMENU_Y+(y*8),
								  (int)(255 * pThis->m_Alpha), FALSE, TRUE) ;

		if (i==pThis->m_Selection)
			COnScreen__SetFontColor(ppDLP, intensity, intensity, intensity/2, intensity, intensity, intensity/2) ;
		else
			COnScreen__SetFontColor(ppDLP, 100, 100, 50, 100, 100, 50) ;


		sprintf(buffer, "%d", FileDirectory[i].pages) ;
		COnScreen__DrawText(ppDLP,
								  buffer,
								  GAMEPAK_X+(24*8), GAMEPAKMENU_Y+(y*8),
								  (int)(255 * pThis->m_Alpha), FALSE, TRUE) ;

		y++ ;

	}

	// display blocks free
	COnScreen__PrepareSmallFont(ppDLP) ;
	COnScreen__SetFontColor(ppDLP, 55, 255, 55, 255, 255, 255) ;
	sprintf(buffer, "%d pages free", FreePages) ;
	COnScreen__DrawText(ppDLP,
							  buffer,
							  GAMEPAK_X+8, GAMEPAKMENU_Y+(17*8),
							  (int)(255 * pThis->m_Alpha), FALSE, TRUE) ;

	// make the pulse out of phase
	intensity = 192+(63 * sin((float)pThis->m_Timer/8)) ;
	switch (gamepak_status)
	{
		case 0:
			COnScreen__SetFontColor(ppDLP, 255, 0, 255, 255, 255, 255) ;
			COnScreen__DrawText(ppDLP, "empty game slot",  320/2, GAMEPAK_Y+GAMEPAK_HEIGHT-16, (int)(255 * pThis->m_Alpha), TRUE, TRUE) ;
			break ;

		case 1:
			COnScreen__SetFontColor(ppDLP, 55, 55, 255, 255, 255, 255) ;
			COnScreen__DrawText(ppDLP, "press 'a' to delete",  320/2, GAMEPAK_Y+GAMEPAK_HEIGHT-16, (int)(255 * pThis->m_Alpha), TRUE, TRUE) ;
			break ;

		case 2:
			COnScreen__SetFontColor(ppDLP, intensity/3, intensity/3, intensity, intensity, intensity, intensity) ;
			COnScreen__DrawText(ppDLP, "press 'b' to confirm delete",  320/2, GAMEPAK_Y+GAMEPAK_HEIGHT-16, (int)(255 * pThis->m_Alpha), TRUE, TRUE) ;
			break ;

	}



/*
	guOrtho(&mtx_pause,
				0, SCREEN_WD-1,
				0, SCREEN_HT-1,
				-200, 200,
				32);		// precision scaler
	for (e=0; e<2; e++)
	{
		for (i=0; i<MAX_PAUSE_VTXS; i++)
		{
			pause_vtxs[e][i].v.ob[0] = 0;
			pause_vtxs[e][i].v.ob[1] = 0;
			pause_vtxs[e][i].v.ob[2] = 0;
			pause_vtxs[e][i].v.flag = 0;
			pause_vtxs[e][i].v.tc[0] = 0;
			pause_vtxs[e][i].v.tc[1] = 0;
			pause_vtxs[e][i].v.cn[0] = 0;
			pause_vtxs[e][i].v.cn[1] = 200;
			pause_vtxs[e][i].v.cn[2] = 0;
			pause_vtxs[e][i].v.cn[3] = 255;
		}
	}
	CPause__InitPolygon(ppDLP) ;

	vtxs = pause_vtxs[even_odd];

	vtxs[0].v.ob[0] = 0 ;
	vtxs[0].v.ob[1] = 0 ;
	vtxs[0].v.cn[0] = 255 ;
	vtxs[0].v.cn[1] = 0 ;
	vtxs[0].v.cn[2] = 0 ;

	vtxs[1].v.ob[0] = 100 ;
	vtxs[1].v.ob[1] = 0 ;
	vtxs[1].v.cn[0] = 0 ;
	vtxs[1].v.cn[1] = 255 ;
	vtxs[1].v.cn[2] = 0 ;

	vtxs[2].v.ob[0] = 100 ;
	vtxs[2].v.ob[1] = 100 ;
	vtxs[2].v.cn[0] = 0 ;
	vtxs[2].v.cn[1] = 0 ;
	vtxs[2].v.cn[2] = 255 ;

	vtxs[3].v.ob[0] = 0 ;
	vtxs[3].v.ob[1] = 100 ;
	vtxs[3].v.cn[0] = 255 ;
	vtxs[3].v.cn[1] = 0 ;
	vtxs[3].v.cn[2] = 255 ;


	gSPVertex((*ppDLP)++,
     			 RSP_ADDRESS(vtxs),
     			 4, 0);
	gSP2Triangles((*ppDLP)++,
					  0, 1, 2, 0,
					  2, 3, 0, 0);
*/


}







