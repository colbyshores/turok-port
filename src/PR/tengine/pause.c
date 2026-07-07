// Pause.Cpp

#include "cppu64.h"
#include "tengine.h"
#include "pause.h"
#include "audio.h"
#include "gfx16bit.h"
#include "onscrn.h"      /* ONSCRN_SW16 — the PAUSE heading reads a big-endian C16BitGraphic header field */
#include "tmove.h"
#include "version.h"


/////////////////////////////////////////////////////////////////////////////
// Temporary pause screen storage
/////////////////////////////////////////////////////////////////////////////

//#include "pausescr.h"
//#include "arrow.h"


/////////////////////////////////////////////////////////////////////////////
// Constants
/////////////////////////////////////////////////////////////////////////////

// Pixel dimentions of pause screen
//#define PAUSE_WIDTH	190
//#define PAUSE_HEIGHT	160

// Pixel dimentions of arrow graphics
//#define ARROW_WIDTH	32
//#define ARROW_HEIGHT	32

// Position to draw puase screen at
//#define PAUSE_X_POS	((320/2) - (PAUSE_WIDTH/2))
//#define PAUSE_Y_POS	((240/2) - (PAUSE_HEIGHT/2))

// Position and dimentions of pause screen
#ifdef GERMAN
#define PAUSE_WIDTH	(210)
#else
#define PAUSE_WIDTH	(180)
#endif
//#define PAUSE_HEIGHT	(180)
#define PAUSE_X		((320/2) - (PAUSE_WIDTH/2))
//#define PAUSE_Y		((240/2) - (PAUSE_HEIGHT/2))


// Position of menu selection 1!
#define PAUSEMENU_Y				48
#define PAUSEMENU_SPACING		18


// Position and dimentions of title box
#define TITLE_WIDTH	(180)
//#define TITLE_HEIGHT	(72)
#define TITLE_X		((320/2) - (TITLE_WIDTH/2))
//#define TITLE_Y		(136)


// Position of menu selection 1!
#define TITLEMENU_Y				4
#define TITLEMENU_SPACING		15



#define	MAX_PAUSE_VTXS		16
Vtx	pause_vtxs[2][MAX_PAUSE_VTXS];		// enough for 6 triangles
Mtx	mtx_pause;


// timers used for flashing hilight bar, set to 0 when reseting selection timer
float		BarTimer = 0 ;

int		RequestorMode ;


/////////////////////////////////////////////////////////////////////////////
// Text
/////////////////////////////////////////////////////////////////////////////

#ifdef ENGLISH
// ---------------------- English text ---------------------------
static char	text_resume[] = {"resume game"} ;
static char	text_options[] = {"options"} ;
static char	text_keys[] = {"keys"} ;

#ifdef EDIT_FOG_AND_LIGHTS
static char	text_editfog[] = {"edit fog"} ;
static char	text_editlight[] = {"edit light"} ;
#endif

static char	text_load[] = {"load game"} ;
//static char	text_save[] = {"save"} ;
//static char	text_prevlevel[] = {"prev level"} ;
//static char	text_nextlevel[] = {"next level"} ;
#ifndef SHIP_IT
static char	text_debug_on[] = {"debug on"} ;
static char	text_debug_off[] = {"debug off"} ;
#endif

static char	text_entercheat[] = {"enter cheat"} ;
static char	text_cheatmenu[] = {"cheat menu"} ;
static char	text_restart[] = {"restart game"} ;

static char	text_startgame[] = {"start game"};
static char	text_training[] = {"training"};

static char	text_difficulty_easy[] = {"easy skill"};
static char	text_difficulty_normal[] = {"normal skill"};
static char	text_difficulty_hard[] = {"hard skill"};

static char	text_tutorial[] = {"tutorial"};
static char	text_time_challenge[] = {"time challenge"};

static char	text_train1[] = {"how do you"} ;
static char	text_train2[] = {"wish to play"} ;

static char	text_areyousure[] = {"are you sure"} ;
#endif


#ifdef GERMAN
// ---------------------- German text ---------------------------
static char	text_resume[] = {"spiel fortsetzen"} ;
static char	text_options[] = {"optionen"} ;
static char	text_keys[] = {"schl/ssel"} ;

static char	text_load[] = {"spiel laden"} ;
//static char	text_save[] = {"speichern"} ;
//static char	text_prevlevel[] = {"voriger level"} ;
//static char	text_nextlevel[] = {"n<chster level"} ;	//a.
#ifndef SHIP_IT
static char	text_debug_on[] = {"debug ein"} ;
static char	text_debug_off[] = {"debug aus"} ;
#endif
static char	text_entercheat[] = {"cheat eingeben"} ;
static char	text_cheatmenu[] = {"cheat men/"} ;		//u.
static char	text_restart[] = {"spiel neu starten"} ;

static char	text_startgame[] = {"spiel starten"};
static char	text_training[] = {"training"};

static char	text_difficulty_easy[] = {"leicht"};
static char	text_difficulty_normal[] = {"normal"};
static char	text_difficulty_hard[] = {"schwer"};

static char	text_tutorial[] = {"einf/hrung"};			//u.
static char	text_time_challenge[] = {"wettlauf mit der zeit"};

static char	text_train1[] = {"wie m>chtest"} ;		//o. (possible a, not sure)
static char	text_train2[] = {"du spielen"} ;

static char	text_areyousure[] = {"bist du sicher"} ;
#endif



#ifdef KANJI
// ---------------------- Japanese text ---------------------------
static char	text_resume[] = {0x2A, 0x41, 0x1D, 0x60, 0x6B, 0x7B, 0x70, -1};
static char	text_options[] = {0x04, 0x36, 0x0B, 0x3F, 0x27, -1};
static char	text_keys[] = {0x82, 0x83, 0x93, -1};

static char	text_load[] = {0x25, 0x41, 0x2F, -1};
//static char	text_prevlevel[] = {0x8E, 0x26, 0x41, 0x23, 0x2F, -1};
//static char	text_nextlevel[] = {0xAC, 0x26, 0x41, 0x23, 0x2F, -1};
#ifndef SHIP_IT
static char	text_debug_on[] = {0x2E, 0x30, 0x40, 0x29, 0x04, 0x27, -1};
static char	text_debug_off[] = {0x2E, 0x30, 0x40, 0x29, 0x04, 0x19, -1};
#endif
static char	text_entercheat[] = {0x02, 0x21, 0x26, 0x2b, 0x09, 0x41, 0x2f, -1};
static char	text_cheatmenu[] = {0x02, 0x21, 0x26, 0x2B, 0x1E, 0x15, 0x3E, 0x41, -1};
static char	text_restart[] = {0x22, 0x0C, 0x0F, 0x41, 0x13, 0x74, 0x57, 0x5d, -1};

static char	text_startgame[] = {0x0C, 0x0F, 0x41, 0x13, -1};
static char	text_training[] = {0x13, 0x24, 0x41, 0x15, 0x27, 0x29, 0x1F, 0x41, 0x2F, -1};

static char	text_difficulty_easy[] = {0x01, 0x41, 0x2C, 0x41, -1};
static char	text_difficulty_normal[] = {0x17, 0x41, 0x1B, 0x23, -1};
static char	text_difficulty_hard[] = {0x18, 0x41, 0x2F, -1};

static char	text_tutorial[] = {0x13, 0x24, 0x41, 0x15, 0x27, 0x29, -1};
static char	text_time_challenge[] = {0x0A, 0x30, 0x01, 0x30, 0x23, 0x09, 0x41, 0x0C, -1};

static char	text_train1[] = {0x1f, 0x41, 0x2f, 0x74, -1} ;
static char	text_train2[] = {0x4f, 0x6e, 0x75, 0x7a, 0x53, 0x79, 0x56, 0x4d, -1} ;

static char	text_areyousure[] = {0x6D, 0x72, 0x57, 0x4D, 0x7A, 0x57, 0x80, 0x4E, 0x51, -1};
#endif

#if defined(PLATFORM_PORT) && !defined(PLATFORM_3DS)
static char	text_quit[] = {"quit game"} ;		// PC: quit to desktop
#endif


char *pause_text[]=
{
	text_resume,
	text_options,
	text_keys,
#ifdef EDIT_FOG_AND_LIGHTS
	text_editfog,
	text_editlight,
#endif
	text_load,
#ifdef PAUSE_SAVE
	text_save,
#endif
//	text_prevlevel,
//	text_nextlevel,
#ifndef SHIP_IT
	text_debug_off,
#endif
	text_entercheat,
	text_cheatmenu,
	text_restart,
#if defined(PLATFORM_PORT) && !defined(PLATFORM_3DS)
	text_quit,
#endif
} ;

char pause_menu_items[PAUSE_END_SELECTION];

char	difficulty_string[16] ;

char *title_text[]=
{
	text_startgame,
	text_load,
	text_training,
	text_options,
	text_difficulty_easy,
	text_entercheat,
	text_cheatmenu,
} ;

char title_menu_items[TITLE_END_SELECTION];


char *trainingrequestor_text[]=
{
	text_tutorial,
	text_time_challenge,
} ;



void CPause__ResetMenuItems(CPause *pThis);
int CPause__GetPauseOptionAmt(CPause *pThis);
int CPause__GetTitleOptionAmt(CPause *pThis);


#define SFXTEST        1



#if SFXTEST

char *test_sound_effects[]=
{
	"Menu option select",
	"Game saved",
	"Ready pistol",
	"Ready shotgun",
	"Ready tek weapon 1",
	"Ready tek weapon 2",
	"Charge tek weapon 2",
	"Ready missile launcher",
	"Ready bfg",
	"Knife swish 1",
	"Knife swish 2",
	"Knife swish 3",
	"Bow stretch",
	"Bow twang",
	"Arrow fly normal",
	"Arrow fly tek",
	"Pistol shot",
	"Rifle shot",
	"Machine gun shot 1",
	"Machine gun shot 2",
	"Grenade launch",
	"Auto shotgun shot",
	"Riot shotgun shot",
	"Mini gun shot",
	"Mini gun whir",
	"Tek weapon 1",
	"Tek weapon 2",
	"Missile launch",
	"BFG shot",
	"Blow gun shot",
	"Hulk blaster fire",
	"Campaingers concussion",
	"Campaingers scatter blast",
	"Pendulum swoosh 1",
	"Pendulum swoosh 2",
	"Pendulum swoosh 3",
	"Pendulum swoosh 4",
	"Pendulum swoosh 5",
	"Metal shell land",
	"Shotgun shell land",
	"Incoming projectile 1",
	"Incoming projectile 2",
	"Incoming projectile 3",
	"Bullet ricochet 1",
	"Bullet ricochet 2",
	"Bullet ricochet 3",
	"Bullet ricochet 4",
	"Bullet impact 1",
	"Bullet impact 2",
	"Bullet impact 3",
	"Bullet impact 4",
	"Bullet impact 5",
	"Bullet impact 6",
	"Bullet impact 7",
	"Bullet impact 8",
	"Bullet impact 9",
	"Bullet impact 10",
	"Bullet impact 13",
	"Bullet impact 14",
	"Bullet impact 15",
	"Body armor impact 1",
	"Body armor impact 2",
	"Body armor impact 3",
	"Body armor impact 4",
	"Body armor impact 5",
	"Explosion 1",
	"Explosion 2",
	"Explosion 3",
	"Fire rumble",
	"Fire spark 1",
	"Fire spark 2",
	"Fire spark 3",
	"Fire spark 4",
	"Fire spark 5",
	"Fire spark 6",
	"Fire spark 7",
	"Fire spark 8",
	"Torch ignight",
	"Electrical spark 1",
	"Electrical spark 2",
	"Electrical spark 3",
	"Electrical spark 4",
	"Electrical spark 5",
	"Electrical spark 6",
	"Pulley squeak 1",
	"Pulley squeak 2",
	"Pulley squeak 3",
	"Pulley squeak 4",
	"Pulley squeak 5",
	"Rope stretch 1",
	"Rope stretch 2",
	"Rope stretch 3",
	"Rope stretch 4",
	"Rope stretch 5",
	"Stonerock crumble 1",
	"Stonerock crumble 2",
	"Stonerock crumble 3",
	"Stonerock crumble 4",
	"Stoneboulder thud	",
	"Mud slosh 1",
	"Mud slosh 2",
	"Mud slosh 3",
	"Dirt debris 1",
	"Dirt debris 2",
	"Dirt debris 3",
	"Tree falling",
	"Tree hit earth",
	"Water drip 1",
	"Water drip 2",
	"Water drip 3",
	"Water drip 4",
	"Water drip 5",
	"Water splash 1",
	"Water splash 2",
	"Water splash 3",
	"Water splash 4",
	"Water splash 5",
	"Underwater swim 1",
	"Underwater swim 2",
	"Underwater swim 3",
	"Underwater swim 4",
	"Underwater swim 5",
	"Bird call 1",
	"Bird call 2",
	"Bird call 3",
	"Bird call 4",
	"Bird call 5",
	"Bird call 6",
	"Bird call 7",
	"Bird call 8",
	"Cricket chirp",
	"Cicaada chirp",
	"Locust chirp",
	"Frog chirp 1",
	"Frog chirp 2",
	"Frog chirp 3",
	"Frog chirp 4",
	"Insect buzz 1",
	"Insect buzz 2",
	"Insect buzz 3",
	"Monkey chip/scream 1",
	"Monkey chip/scream 2",
	"Monkey chip/scream 3",
	"Monkey chip/scream 4",
	"Wild boar squeal 1",
	"Wild boar squeal 2",
	"Wild boar squeal 3",
	"Wild boar squeal 4",
	"Bird flushing",
	"Brush rustle 1",
	"Brush rustle 2",
	"Brush rustle 3",
	"Brush rustle 4",
	"Wind blow 1",
	"Wind blow 2",
	"Wind blow 3",
	"Wind blow 4",
	"Wind blow 5",
	"Rain lightly",
	"Rain medium",
	"Rain heavy",
	"Thunder roll 1",
	"Thunder roll 2",
	"Thunder roll 3",
	"Thunder roll 4",
	"Lightning strike 1",
	"Lightning strike 2",
	"Lightning strike 3",
	"River flow lightly",
	"River flow medium",
	"River flow heavy",
	"Waterfall rush",
	"Lava flow",
	"Gyser spouting",
	"Stalactites break",
	"Ominous cave growl 1",
	"Ominous cave growl 2",
	"Ominous cave growl 3",
	"Ghostly catacomb moan 1",
	"Ghostly catacomb moan 2",
	"Ghostly catacomb moan 3",
	"Light jungle footfall",
	"Heavy jungle footfall",
	"Rope bridge footfall",
	"Climb cliff footfall",
	"Swamp footfall",
	"Water footfall",
	"Stone footfall",
	"Metal footfall",
	"Large thud",
	"Fall into pit",
	"Stone scrape on stone",
	"Quicksand swallow",
	"Elevator mechanism",
	"Ceremonial slab rise",
	"Clay pot smash",
	"Rocks squish in swamp",
	"Doorway open/close 1",
	"Doorway open/close 2",
	"Energy shield hum",
	"Energy blast",
	"Warp inout",
	"Machine hum",
	"Power hum",
	"Thermal goggles on",
	"Thermal goggles power",
	"Health pickup 1",
	"Health pickup 2",
	"Health pickup 3",
	"Ultra health pickup",
	"Hit point pickup",
	"Spirituality pickup",
	"Spirituality death attack",
	"All map pickup",
	"Raptor scream",
	"Raptor alert",
	"Raptor footfall",
	"Raptor injury",
	"Raptor attack",
	"Raptor death",
	"Raptor violent death",
	"Raptor sniff",
	"Tiger footfall",
	"Tiger roar",
	"Tiger alert",
	"Tiger injury",
	"Tiger attack",
	"Tiger death",
	"Tiger violent death",
	"Tiger breathe",
	"Dimetrodon footfall",
	"Dimetrodon scream",
	"Dimetrodon alert",
	"Dimetrodon injury",
	"Dimetrodon death",
	"Dimetrodon violent death",
	"Dimetrodon cough/snarl",
	"Moschops footfall",
	"Moschops scream",
	"Moschops alert",
	"Moschops injury",
	"Moschops attack",
	"Moschops death",
	"Moschops violent death",
	"Moschops breathe",
	"Pteranodn scream",
	"Pteranodn wing flap",
	"Pteranodn injury",
	"Pteranodn death",
	"Pteranodn hiss",
	"Triceratops footfall",
	"Triceratops scream",
	"Triceratops alert",
	"Triceratops injury",
	"Dragonfly buzz",
	"Dragonfly clicks/chatter 1",
	"Dragonfly clicks/chatter 2",
	"Dragonfly injury",
	"Dragonfly death",
	"Dragonfly attack",
	"Beetle footfall",
	"Beetle buzz",
	"Beetle clicks/chatter 1",
	"Beetle clicks/chatter 2",
	"Beetle injury",
	"Beetle death",
	"Beetle attack",
	"Human effort/injury grunt 1",
	"Human effort/injury grunt 2",
	"Human effort/injury grunt 3",
	"Human effort/injury grunt 4",
	"Human effort/injury grunt 5",
	"Human alert 1",
	"Human alert 2",
	"Human alert 3",
	"Human death scream 1",
	"Human death scream 2",
	"Human death scream 3",
	"Human violent death 1",
	"Human violent death 2",
	"Human violent death 3",
	"Ancient warrior scream",
	"Ancient warrior alert",
	"Ancient warrior attack",
	"Ancient warrior death",
	"High priest teleport",
	"High priest spell attack",
	"Subterranean scream",
	"Subterranean earth rumble",
	"Subterranean acid attack",
	"Subterranean acid impact",
	"Subterranean death",
	"Subterranean violent death",
	"Leaper footfall",
	"Leaper scream",
	"Leaper alert",
	"Leaper injury",
	"Leaper attack",
	"Leaper death",
	"Leaper violent death",
	"Leaper chitter",
	"Alien footfall",
	"Alien scream",
	"Alien alert",
	"Alien injury",
	"Alien death",
	"Alien jetpack malfunction",
	"Alien violent death",
	"Hulk footfall",
	"Hulk scream",
	"Hulk alert",
	"Hulk injury",
	"Hulk attack",
	"Hulk death",
	"Hulk violent death",
	"Hulk breathe",
	"Robot footfall",
	"Robot servo 1",
	"Robot servo 2",
	"Robot servo 3",
	"Robot servo 4",
	"Robot voice fx 1",
	"Robot voice fx 2",
	"Robot voice fx 3",
	"Robot short 1",
	"Robot short 2",
	"Robot short 3",
	"Robot shutdown",
	"Droid move",
	"Droid servo 1",
	"Droid servo 2",
	"Droid servo 3",
	"Droid object located",
	"Security droid move",
	"Security droid laser fire",
	"Security droid laser lock on",
	"Security droid extinguish",
	"Santis scream 1",
	"Mantis scream 2",
	"Mantis chitter",
	"Mantis injury",
	"Mantis attack",
	"Mantis violent death",
	"Trex footfall",
	"Trex scream",
	"Trex servo 1",
	"Trex servo 2",
	"Trex servo 3",
	"Trex sniff",
	"Trex eye laser fire",
	"Trex fire breathe",
	"Trex bite attack",
	"Trex violent death",
	"Longhunter taunt 1",
	"Longhunter taunt 2",
	"Longhunter taunt 3",
	"Campainger taunt 1",
	"Campainger taunt 2",
	"Campainger rage",
	"Campainger attack",
	"Campainger spell",
	"Campainger shorting out",
	"Campainger death",
	"Throw tomahawk",
	"Tomahawk glow",
	"Tomahawk impact flesh",
	"Shockwave weapon fire",
	"Shockwave weapon ready",
	"Shockwave weapon fry",
	"Raptor laser fire",
	"Rotating lift servo",
	"Pteranodon fly swoosh",
	"Robot malfunction",
	"Mantis fireball",
	"Mantis death rain",
	"Footfall on wood plank",
	"Toucan fly by",
	"Antelope flee",
	"Ready Grenade Launcher",
	"Minigun Stop",
	"Ready pulse rifle",
	"Ready assault rifle",
	"Ready mini gun",
	"Ready auto shotgun",
	"Reload missile launcher",
	"Reload auto shotgun",
	"Generic 1 - bullet pickup",
	"Generic 2 - shell pickup",
	"Generic 3 - energy pickup",
	"Generic 4 - non weapon pickup",
	"Generic 5 - grenade pickup",
	"Generic 6 - arrow pickup ",
	"Generic 7 - rocket pickup",
    "Generic 8 - enemy human gunfire",
    "Generic 9 - Alien Jetpack Engage",
    "Generic 10 - Turok injury 1",
    "Generic 11 - Turok injury 2",
    "Generic 12 - Turok injury 3",
    "Generic 13 - Turok injury 4",
    "Generic 14 - Turok Water injury 1",
    "Generic 15 - Turok Water injury 2",
    "Generic 16 - Turok small water gasp",
    "Generic 17 - Turok big water gasp",
    "Generic 18 - Turok normal death",
    "Generic 19 - Turok mantis death",
    "Generic 20 - Turok T-rex death",
    "Generic 21 - Turok jump",
    "Generic 22 - Turok land",
    "Generic 23 - Turok climb 1",
    "Generic 24 - Turok climb 2",
    "Generic 25 - Turok fall to death",
    "Generic 26 - Bird call 9",
    "Generic 27 - Bird call 10",
    "Generic 28 - Bird call 11",
    "Generic 29 - Bird call 12",
    "Generic 30 - Bird call 13",
    "Generic 31 - Bird call 14",
    "Generic 32 - Bird call 15",
    "Generic 33 - Bird call 16",
    "Generic 34 - Bird call 17",
    "Generic 35 - Bird call 18",
    "Generic 36 - Bird call 19",
    "Generic 37 - Mettalic Moan 1",
    "Generic 38 - Mettalic Moan 2",
    "Generic 39 - Mettalic Moan 3",
    "Generic 40 - Mettalic Moan 4",
    "Generic 41 - Mettalic Moan 5",
    "Generic 42 - Cave wing flutter w/ bat 1",
    "Generic 43 - Cave wing flutter w/ bat 2",
    "Generic 44 - Cave wing flutter 1",
    "Generic 45 - Cave wing flutter 2",
    "Generic 46 - Cave wing flutter 3",
    "Generic 47 - Monkey chirp 5",
    "Generic 48 - Monkey chirp 6",
    "Generic 49 - Monkey chirp 7",
    "Generic 50 - Monkey chirp 8",
    "Generic 51 - Monkey chirp 9",
    "Generic 52 - Monkey chirp 10",
    "Generic 53 - Monkey chirp 11",
    "Generic 54 - Monkey chirp 12",
    "Generic 55 - Monkey chirp 13",
    "Generic 56 - Monkey chirp 14",
    "Generic 57 - Cricket chirp 2",
    "Generic 58 - Locust chirp 2",
    "Generic 59 - Cave wing flutter 4",
    "Generic 60 - Bat screech 1",
    "Generic 61 - Bat screech 2",
    "Generic 62 - Painful moan 4",
    "Generic 63 - Catacomb growl 1",
    "Generic 64 - Catacomb growl 2",
    "Generic 65 - Stone door 3",
    "Generic 66 - EMPTY",
    "Generic 67 - Gear Move 1",
    "Generic 68 - Stone door 4",
    "Generic 69 - Stone door 5",
    "Generic 70 - Door thud",
    "Generic 71 - Barrel door",
    "Generic 72 - Gear click 1",
    "Generic 73 - Stone door 6",
    "Generic 74 - Gear move 2",
    "Generic 75 - Gear move noise",
    "Generic 76 - Trigger plate click 1",
    "Generic 77 - Iggy ricochet",
    "Generic 78 - Iggy hatchet impact",
    "Generic 79 - Iggy arrow fly",
    "Generic 80 - Iggy arrow impact",
    "Generic 81 - Kick impact",
    "Generic 82 - Humvee idle 'Looped'",
    "Generic 83 - Humvee accelerate 'Looped'",
    "Generic 84 - Humvee stop accelerate",
    "Generic 85 - Humvee 180 spinout",
    "Generic 86 - Humvee 180 turn",
    "Generic 87 - Humvee jump",
    "Generic 88 - Kick swish 1",
    "Generic 89 - Jump swish 1",
    "Generic 90 - Punch swoosh",
    "Generic 91 - Sommersault",
    "Generic 92 - Gun twirl",
    "Generic 93 - Land from jump",
    "Generic 94 - Mantis spit land",
    "Generic 95 - Mantis spit injure",
    "Generic 96 - Tar bubble 1",
    "Generic 97 - Tar bubble 2",
    "Generic 98 - Mantis claw slash 1",
    "Generic 99 - Mantis jump swoosh 1",
    "Generic 100 - Mantis jump swoosh 2",
    "Generic 101 - Mantis wing flap 1",
    "Generic 102 - Mantis wing flap 2",
    "Generic 103 - Mantis death 2",
    "Generic 104 - Mantis death 3",
    "Generic 105 - Mantis death 4",
    "Generic 106 - Mantis land on wall",
    "Generic 107 - Mantis land on floor",
    "Generic 108 - Subterranean dive rumble",
    "Generic 109 - Subterranean dive",
    "Generic 110 - Subterranean surface",
    "Generic 111 - Subterranean surface vox",
    "Generic 112 - Iggy finger tick",
    "Generic 113 - Hatchet fly",
    "Generic 114 - Acclaim pad",
    "Generic 115 - Turok bow stretch",
    "Generic 116 - Turok wind",
    "Generic 117 - Raptor thrash 1",
    "Generic 118 - Raptor thrash 2",
    "Generic 119 - Weak swoosh 1",
    "Generic 120 - Weak swoosh 2",
    "Generic 121 - Weak swoosh 3",
    "Generic 122 - Throw grenade swoosh",
    "Generic 123 - Strong weapon swoosh 1",
    "Generic 124 - Strong weapon swoosh 2",
    "Generic 125 - Spell swoosh",
    "Generic 126 - Spell cast 2",
    "Generic 127 - Human land from jump",
    "Generic 128 - Human fall grunt",
    "Generic 129 - Human land from fall 2",
    "Generic 130 - Chimp chat",
    "Generic 131 - Leaper jump swoosh",
    "Generic 132 - Leaper jump thud",
    "Generic 133 - Underwater swish 1",
    "Generic 134 - Underwater swish 2",
    "Generic 135 - Leaper choke",
    "Generic 136 - Leaper slosh 1",
    "Generic 137 - Leaper slosh 2",
    "Generic 138",
    "Generic 139",
    "Generic 140",
    "Generic 141",
    "Generic 142",
    "Generic 143",
    "Generic 144",
    "Generic 145",
    "Generic 146",
    "Generic 147",
    "Generic 148",
    "Generic 149",
    "Generic 150",
    "Generic 151",
    "Generic 152",
    "Generic 153",
    "Generic 154",
    "Generic 155",
    "Generic 156",
    "Generic 157",
    "Generic 158",
    "Generic 159",
    "Generic 160",
    "Generic 161",
    "Generic 162",
    "Generic 163",
    "Generic 164",
    "Generic 165",
    "Generic 166",
    "Generic 167",
    "Generic 168",
    "Generic 169",
    "Generic 170",
    "Generic 171",
    "Generic 172",
    "Generic 173",
    "Generic 174",
    "Generic 175",
    "Generic 176",
    "Generic 177",
    "Generic 178",
    "Generic 179",
    "Generic 180",
    "Generic 181",
    "Generic 182",
    "Generic 183",
    "Generic 184",
    "Generic 185",
    "Generic 186",
    "Generic 187",
    "Generic 188",
    "Generic 189",
    "Generic 190",
    "Generic 191",
    "Generic 192",
    "Generic 193",
    "Generic 194",
    "Generic 195",
    "Generic 196",
    "Generic 197",
    "Generic 198",
    "Generic 199",
    "Generic 200",
	"Generic 201",
	"Generic 202",
	"Generic 203",
	"Generic 204",
	"Generic 205",
	"Generic 206",
	"Generic 207",
	"Generic 208",
	"Generic 209",
	"Generic 210",
	"Generic 211",
	"Generic 212",
	"Generic 213",
	"Generic 214",
	"Generic 215",
	"Generic 216",
	"Generic 217",
	"Generic 218",
	"Generic 219",
	"Generic 220",
	"Generic 221",
	"Generic 222",
	"Generic 223",
	"Generic 224",
	"Generic 225",
	"Generic 226",
	"Generic 227",
	"Generic 228",
	"Generic 229",
	"Generic 230",
	"Generic 231",
	"Generic 232",
	"Generic 233",
	"Generic 234",
	"Generic 235",
	"Generic 236",
	"Generic 237",
	"Generic 238",
	"Generic 239",
	"Generic 240",
	"Generic 241",
	"Generic 242",
	"Generic 243",
	"Generic 244",
	"Generic 245",
	"Generic 246",
	"Generic 247",
	"Generic 248",
	"Generic 249",
	"Generic 250",

};

#endif


/////////////////////////////////////////////////////////////////////////////
// Functions
/////////////////////////////////////////////////////////////////////////////

//void WriteGraphicToScreen(UINT16 *Screen, UINT16 *Data, INT32 Width, INT32 Height, INT32 X, INT32 Y)
//{
//	Screen += X + (Y*320) ;
//
//	// Raster graphic line by line
//	while(Height--)
//	{
//		// Write one line
//		memcpy((UINT8 *)Screen, (UINT8 *)Data, Width*2) ;
//
//		// Next line
//		Data += Width ;
//		Screen += 320 ;
//	}
//}



void CPause__Construct(CPause *pThis)
{
	int		e, i ;


	if (pThis->m_Mode == PAUSE_NULL)
		pThis->m_Alpha = 0 ;

	pThis->m_Mode = PAUSE_FADEUP ;

	GetApp()->m_bLoad = FALSE ;
	GetApp()->m_bSave = FALSE ;
	GetApp()->m_bOptions = FALSE ;
	GetApp()->m_bKeys = FALSE ;
	GetApp()->m_bRequestor = FALSE ;
	GetApp()->m_bMessageBox = FALSE ;
	GetApp()->m_bCheatCode = FALSE ;
	GetApp()->m_bGiveCheatCode = FALSE ;
	GetApp()->m_bCheat = FALSE ;
	GetApp()->m_bGInst = FALSE ;

	if (GetApp()->m_Mode == MODE_GAME)
	{
//		pThis->m_MusicVolume = GetApp()->m_ActualMusicVolume;
// 		pThis->m_SFXVolume = GetApp()->m_ActualSFXVolume;
		GetApp()->m_ActualMusicVolume = 0.0;
//		GetApp()->m_ActualSFXVolume = 0.0;
	}

	RequestorMode = NULL ;

	old_enemy_speed_scaler = enemy_speed_scaler ;
	old_sky_speed_scaler = sky_speed_scaler ;
	old_particle_speed_scaler = particle_speed_scaler ;
	if (GetApp()->m_Mode == MODE_GAME)
	{
		sky_speed_scaler = 0.0 ;
		enemy_speed_scaler = 0.0 ;
		particle_speed_scaler = 0 ;
	}
	CPause__ResetMenuItems(pThis);

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



	//rmonPrintf("PAUSE\n");
	//StopSeq(0);
	//PlaySeq(1);

	// Allocate and setup pointer to title screen data
	//pThis->m_pPauseScreen = (UINT16 *)PauseScreen ;		// Pointer to title screen data
	//pThis->m_pPauseScreenArrow = (UINT16 *)Arrow ;		// Pointer to arrow data

}


// reset menu items
//
void CPause__ResetMenuItems(CPause *pThis)
{
	int	cnt;


	// reset menu items
	for (cnt=0; cnt<PAUSE_END_SELECTION; cnt++)
		pause_menu_items[cnt] = 1;

	for (cnt=0; cnt<TITLE_END_SELECTION; cnt++)
		title_menu_items[cnt] = 1;

	// should cheat menu be displayed ?
	if (!GetApp()->m_dwEnabledCheatFlags)
	{
		pause_menu_items[PAUSE_CHEATMENU] = 0;
		title_menu_items[TITLE_CHEATMENU] = 0;
	}

	// Setup default selection
	pThis->m_Selection = PAUSE_RESUME ;
	pThis->m_RealSelection = 0;
}


// return no. of options
//
int CPause__GetPauseOptionAmt(CPause *pThis)
{
	int	nTotal = 0,
			cnt;


	for (cnt=0; cnt<PAUSE_END_SELECTION; cnt++)
		nTotal += pause_menu_items[cnt];


	return nTotal;
}


// return no. of options
//
int CPause__GetTitleOptionAmt(CPause *pThis)
{
	int	nTotal = 0,
			cnt;


	for (cnt=0; cnt<TITLE_END_SELECTION; cnt++)
		nTotal += title_menu_items[cnt];


	return nTotal;
}


//------------------------------------------------------------------------
// Update selection on pause screen
//------------------------------------------------------------------------
INT32 CPause__Update(CPause *pThis)
{
	INT32				ReturnValue = -1 ;


#if SFXTEST
	static int		sfx_num = 0;
	//CVector3			tempvec;
#endif

	CTurokMovement.InMenuTimer = .3 ;

	// force the icons to be displayed
	GetApp()->m_OnScreen.m_Lives = -1 ;
	GetApp()->m_OnScreen.m_Tokens = -1 ;
	COnScreen__SetLives(&GetApp()->m_OnScreen, CTurokMovement.Lives);
	COnScreen__SetTokens(&GetApp()->m_OnScreen, CTurokMovement.Tokens);

	// do cheat options
	if (GetApp()->m_Cheat.b_Active)
	{
		GetApp()->m_ModeTime = 0 ;
		// returns quite a lot of stuff
		ReturnValue = CCheat__Update(&GetApp()->m_Cheat) ;
		if (ReturnValue != -1)
		{
			pThis->m_Mode = PAUSE_FADEUP ;
			GetApp()->m_Cheat.b_Active = FALSE ;
			CPause__ResetMenuItems(pThis);
		}
		ReturnValue = -1 ;
	}

	// do cheatcode options
	else if (GetApp()->m_CheatCode.b_Active)
	{
		GetApp()->m_ModeTime = 0 ;
		// returns quite a lot of stuff
		ReturnValue = CCheatCode__Update(&GetApp()->m_CheatCode) ;
		if (ReturnValue != -1)
		{
			pThis->m_Mode = PAUSE_FADEUP ;
			GetApp()->m_CheatCode.b_Active = FALSE ;
			CPause__ResetMenuItems(pThis);
		}
		ReturnValue = -1 ;
	}

	// do givecheatcode options
	else if (GetApp()->m_GiveCheatCode.b_Active)
	{
		GetApp()->m_ModeTime = 0 ;
		ReturnValue = CGiveCheatCode__Update(&GetApp()->m_GiveCheatCode) ;
		if (ReturnValue != -1)
		{
			pThis->m_Mode = PAUSE_FADEUP ;
			GetApp()->m_GiveCheatCode.b_Active = FALSE ;
			CPause__ResetMenuItems(pThis);
		}
		ReturnValue = -1 ;
	}


	// do game options
	else if (GetApp()->m_Options.b_Active)
	{
		GetApp()->m_ModeTime = 0 ;
		// returns quite a lot of stuff
		ReturnValue = COptions__Update(&GetApp()->m_Options) ;
		if (ReturnValue != -1)
		{
			pThis->m_Mode = PAUSE_FADEUP ;
			GetApp()->m_Options.b_Active = FALSE ;
			CPause__ResetMenuItems(pThis);
		}
		ReturnValue = -1 ;
	}


	// do key options
	else if (GetApp()->m_Keys.b_Active)
	{
		GetApp()->m_ModeTime = 0 ;
		ReturnValue = CKeys__Update(&GetApp()->m_Keys) ;
		if (ReturnValue != -1)
		{
			pThis->m_Mode = PAUSE_FADEUP ;
			GetApp()->m_Keys.b_Active = FALSE ;
			CPause__ResetMenuItems(pThis);
		}
		ReturnValue = -1 ;
	}


	// do load options
	else if (GetApp()->m_Load.b_Active)
	{
		GetApp()->m_ModeTime = 0 ;
		// will only return -1 or LOAD_EXIT
		ReturnValue = CLoad__Update(&GetApp()->m_Load) ;
		if (ReturnValue != -1)
		{
			CPause__ResetMenuItems(pThis);
			pThis->m_Mode = PAUSE_FADEUP ;
			GetApp()->m_Load.b_Active = FALSE ;
			ReturnValue = -1 ;
			CPause__ResetMenuItems(pThis);
		}
	}

	// do save options - ONLY FROM REQUESTOR (NO LONGER PART OF PAUSE) SO QUIT TO GAME ON EXIT
	else if (GetApp()->m_Save.b_Active)
	{
		GetApp()->m_ModeTime = 0 ;
		// will only return -1 or SAVE_EXIT
		ReturnValue = CSave__Update(&GetApp()->m_Save) ;
		if (ReturnValue != -1)
		{
			GetApp()->m_Pause.m_Mode = PAUSE_FADEDOWN ;
			GetApp()->m_bPause = FALSE ;

			//pThis->m_Mode = PAUSE_FADEUP ;
			GetApp()->m_Save.b_Active = FALSE ;
			ReturnValue = -1 ;
			//pThis->m_Selection = PAUSE_RESUME ;
		}
	}

	// do you wish to save?
	else if (GetApp()->m_Requestor.b_Active)
	{
		GetApp()->m_ModeTime = 0 ;
		ReturnValue = CRequestor__Update(&GetApp()->m_Requestor) ;
		if (ReturnValue != -1)
		{
			switch (RequestorMode)
			{
				case 0:
					// do save
					if (ReturnValue == REQUESTOR_YES)
					{
						CSave__Construct(&GetApp()->m_Save) ;
						GetApp()->m_bSave = TRUE ;
					}
					// return to game
					else
					{
						GetApp()->m_Pause.m_Mode = PAUSE_FADEDOWN ;
						GetApp()->m_bPause = FALSE ;
					}
					ReturnValue = -1 ;
					break ;

				case PAUSE_RESTART_GAME:
					if (ReturnValue == REQUESTOR_YES)
					{
						ReturnValue = PAUSE_RESTART_GAME ;
						RequestorMode = NULL ;
						GetApp()->m_Pause.m_Mode = PAUSE_FADEDOWN ;
						GetApp()->m_bPause = FALSE ;
					}
					else
						ReturnValue = -1 ;
					break ;

				case TITLE_TRAINING:
					GetApp()->m_bTrainingMode = TRUE ;
					pThis->m_Mode = PAUSE_FADEDOWN ;

					if (ReturnValue == REQUESTOR_YES)
						GetApp()->m_TrainingType = 0 ;
					else
						GetApp()->m_TrainingType = 1 ;
					break ;

				// exit from training level
				case -1:
					if (ReturnValue == REQUESTOR_YES)
						ReturnValue = PAUSE_RESTART_GAME ;
					else
						ReturnValue = -1 ;

					RequestorMode = NULL ;
					GetApp()->m_Pause.m_Mode = PAUSE_FADEDOWN ;
					GetApp()->m_bPause = FALSE ;
					break ;
			}
			GetApp()->m_Requestor.m_Mode = REQUESTOR_FADEDOWN ;
			GetApp()->m_Requestor.b_Active = FALSE ;

		}
	}
	// only from titlescreen for DAMAGED CONTROLLER message
	else if (GetApp()->m_MessageBox.b_Active == 2)
	{
		GetApp()->m_ModeTime = 0 ;
		ReturnValue = CMessageBox__Update(&GetApp()->m_MessageBox) ;
		if (ReturnValue != -1)
		{
			ReturnValue = -1 ;
			GetApp()->m_MessageBox.m_Mode = MESSAGEBOX_FADEDOWN ;
			GetApp()->m_MessageBox.b_Active = FALSE ;
		}

	}


	// do pause options
	else
	{
		if (GetApp()->m_Mode == MODE_TITLE)
		{
			// in title mode
			if (CEngineApp__MenuDown(GetApp()))
			{
				BarTimer = 0 ;
				GetApp()->m_ModeTime = 0 ;
				do
				{
					pThis->m_Selection++ ;
					if (pThis->m_Selection >= TITLE_END_SELECTION)
						pThis->m_Selection = 0 ;

				} while (!title_menu_items[pThis->m_Selection]);

				pThis->m_RealSelection++;
				if (pThis->m_RealSelection >= CPause__GetTitleOptionAmt(pThis))
					pThis->m_RealSelection = 0;


	#if SFXTEST
			sfx_num++;
			//osSyncPrintf("SFX: %d, %s\n", sfx_num, test_sound_effects[sfx_num]);
			osSyncPrintf("SFX: %d\n", sfx_num);
	#endif

			}

			if (CEngineApp__MenuUp(GetApp()))
			{
				BarTimer = 0 ;
				GetApp()->m_ModeTime = 0 ;
				do
				{
					pThis->m_Selection-- ;
					if (pThis->m_Selection < 0)
						pThis->m_Selection = TITLE_END_SELECTION-1 ;

				} while (!title_menu_items[pThis->m_Selection]);

				pThis->m_RealSelection--;
				if (pThis->m_RealSelection < 0)
					pThis->m_RealSelection = CPause__GetTitleOptionAmt(pThis)-1;

	#if SFXTEST
			sfx_num--;
			if(sfx_num < 0)
				sfx_num = 0;
			osSyncPrintf("SFX: %d\n", sfx_num);
	#endif

			}
		}
		else
		{
			// in pause mode
			if (CEngineApp__MenuDown(GetApp()))
			{
				BarTimer = 0 ;
				do
				{
					pThis->m_Selection++ ;
					if (pThis->m_Selection >= PAUSE_END_SELECTION)
						pThis->m_Selection = 0 ;

				} while (!pause_menu_items[pThis->m_Selection]);

				pThis->m_RealSelection++;
				if (pThis->m_RealSelection >= CPause__GetPauseOptionAmt(pThis))
					pThis->m_RealSelection = 0;

	#if SFXTEST
			sfx_num++;
			osSyncPrintf("SFX: %d\n", sfx_num);
	#endif

			}

			if (CEngineApp__MenuUp(GetApp()))
			{
				BarTimer = 0 ;
				do
				{
					pThis->m_Selection-- ;
					if (pThis->m_Selection < 0)
						pThis->m_Selection = PAUSE_END_SELECTION-1 ;

				} while (!pause_menu_items[pThis->m_Selection]);

				pThis->m_RealSelection--;
				if (pThis->m_RealSelection < 0)
					pThis->m_RealSelection = CPause__GetPauseOptionAmt(pThis)-1;

	#if SFXTEST
			sfx_num--;
			if(sfx_num < 0)
				sfx_num = 0;
			osSyncPrintf("SFX: %d\n", sfx_num);
	#endif

			}
		}

	#if SFXTEST

		if(CTControl__IsPrimaryFire(pCTControl))
		{
			CScene__DoSoundEffect(&GetApp()->m_Scene, sfx_num, 0, NULL, &AW.Ears.vPos, 0);
		}

		if(CTControl__IsSelectWeaponNext(pCTControl))
			killAllSFX();

	#endif

	#if SFXTEST

		if(CTControl__IsSideStepRight(pCTControl))
		{
			sfx_num += 5;
			osSyncPrintf("sfxnum: %d\n", sfx_num);
		}

		if(CTControl__IsSideStepLeft(pCTControl))
		{
			sfx_num -= 5;
			if(sfx_num < 0)
				sfx_num = 0;
			osSyncPrintf("sfxnum: %d\n", sfx_num);
		}

	#endif


		// Exit if start pressed
		if (CTControl__IsUseMenu(pCTControl))
			ReturnValue = pThis->m_Selection ;

#ifdef PLATFORM_3DS
		/* 3DS: B (cancel) from the in-game pause menu backs out = resume the game. Ignored on the title
		 * screen (no "back" from title). */
		{	extern int g_turok_menu_cancel;
			if (g_turok_menu_cancel && GetApp()->m_Mode != MODE_TITLE)
			{
				g_turok_menu_cancel = 0;
				pThis->m_Mode = PAUSE_FADEDOWN ;
			}
		}
#endif

		// do corresponding action TITLE SCREEN
		if (GetApp()->m_Mode == MODE_TITLE)
		{
			switch (ReturnValue)
			{
				case TITLE_LOAD:
					ReturnValue = -1 ;
					pThis->m_Mode = PAUSE_FADEDOWN ;

					CLoad__Construct(&GetApp()->m_Load) ;
					GetApp()->m_bLoad = TRUE ;
					break ;

				case TITLE_OPTIONS:
					ReturnValue = -1 ;
					pThis->m_Mode = PAUSE_FADEDOWN ;

					COptions__Construct(&GetApp()->m_Options) ;
					GetApp()->m_bOptions = TRUE ;
					break ;

				case TITLE_DIFFICULTY:
					GetApp()->m_ModeTime = 0 ;
					ReturnValue = -1 ;
					GetApp()->m_Difficulty++ ;
					if (GetApp()->m_Difficulty > 2)
						GetApp()->m_Difficulty=0 ;
					break ;


				case TITLE_START:
					pThis->m_Mode = PAUSE_FADEDOWN ;
					break ;

				case TITLE_TRAINING:
					CRequestor__Construct(&GetApp()->m_Requestor,
									text_train1, text_train2) ;
					GetApp()->m_Requestor.Selections = trainingrequestor_text ;
					GetApp()->m_bRequestor = TRUE ;
					RequestorMode = TITLE_TRAINING ;
					ReturnValue = -1 ;

//					GetApp()->m_bTrainingMode = TRUE ;
//					pThis->m_Mode = PAUSE_FADEDOWN ;
					break ;


				case TITLE_CHEATMENU:
					ReturnValue = -1 ;

					pThis->m_Mode = PAUSE_FADEDOWN ;

					CCheat__Construct(&GetApp()->m_Cheat) ;
					GetApp()->m_bCheat = TRUE ;
					break;

				case TITLE_ENTERCHEAT:
					ReturnValue = -1 ;

					pThis->m_Mode = PAUSE_FADEDOWN ;

					CCheatCode__Construct(&GetApp()->m_CheatCode) ;
					GetApp()->m_bCheatCode = TRUE ;
					break ;

			}
		}
		// do corresponding action GAME PAUSED
		else
		{
			switch (ReturnValue)
			{
#ifndef SHIP_IT
				case PAUSE_DEBUG:
					ReturnValue = -1 ;
					GetApp()->m_bDebug ^= TRUE ;
					break ;
#endif
				case PAUSE_ENTERCHEAT:
					ReturnValue = -1 ;

					pThis->m_Mode = PAUSE_FADEDOWN ;

					CCheatCode__Construct(&GetApp()->m_CheatCode) ;
					GetApp()->m_bCheatCode = TRUE ;
					break ;

				case PAUSE_CHEATMENU:
					ReturnValue = -1 ;

					pThis->m_Mode = PAUSE_FADEDOWN ;

					CCheat__Construct(&GetApp()->m_Cheat) ;
					GetApp()->m_bCheat = TRUE ;
					break ;

				case PAUSE_LOAD:
					ReturnValue = -1 ;
					pThis->m_Mode = PAUSE_FADEDOWN ;

					CLoad__Construct(&GetApp()->m_Load) ;
					GetApp()->m_bLoad = TRUE ;
					break ;

#ifdef PAUSE_SAVE
				case PAUSE_SAVE:
					ReturnValue = -1 ;
					pThis->m_Mode = PAUSE_FADEDOWN ;

					CSave__Construct(&GetApp()->m_Save) ;
					GetApp()->m_bSave = TRUE ;
					break ;
#endif

				case PAUSE_OPTIONS:
					ReturnValue = -1 ;
					pThis->m_Mode = PAUSE_FADEDOWN ;

					COptions__Construct(&GetApp()->m_Options) ;
					GetApp()->m_bOptions = TRUE ;
					break ;

				case PAUSE_KEYS:
					ReturnValue = -1 ;
					pThis->m_Mode = PAUSE_FADEDOWN ;

					CKeys__Construct(&GetApp()->m_Keys) ;
					GetApp()->m_bKeys = TRUE ;
					break ;

#ifdef EDIT_FOG_AND_LIGHTS
				case PAUSE_EDITFOG:
					ReturnValue = -1 ;
					pThis->m_Mode = PAUSE_FADEDOWN ;

					COptions__Construct(&GetApp()->m_Options) ;
					GetApp()->m_Options.m_EditFog = TRUE ;
					GetApp()->m_bOptions = TRUE ;
					break ;

				case PAUSE_EDITLIGHT:
					ReturnValue = -1 ;
					pThis->m_Mode = PAUSE_FADEDOWN ;

					COptions__Construct(&GetApp()->m_Options) ;
					GetApp()->m_Options.m_EditLight = TRUE ;
					GetApp()->m_bOptions = TRUE ;
					break ;
#endif


				case PAUSE_RESUME:
//				case PAUSE_PREVLEVEL:
//				case PAUSE_NEXTLEVEL:
					pThis->m_Mode = PAUSE_FADEDOWN ;
					break ;

				case PAUSE_RESTART_GAME:
					CRequestor__Construct(&GetApp()->m_Requestor,
									text_restart, text_areyousure) ;
					GetApp()->m_bRequestor = TRUE ;
					GetApp()->m_Requestor.m_Selection = REQUESTOR_NO ;
					RequestorMode = PAUSE_RESTART_GAME ;
					ReturnValue = -1 ;
					break ;

#if defined(PLATFORM_PORT) && !defined(PLATFORM_3DS)
				case PAUSE_QUIT:			// PC: quit to desktop (matches the SDL window-close path)
					{ extern void exit(int); exit(0); }
					break ;
#endif

			}
		}
	}

	return ReturnValue ;
}


//------------------------------------------------------------------------
// Draw pause screen information
//------------------------------------------------------------------------
int		polyvertex ;

void CPause__InitPolygon(Gfx **ppDLP)
{
	static Mtx	mPause ;
	CMtxF	 		mfPause ;

	gDPPipeSync((*ppDLP)++) ;
	gDPSetRenderMode((*ppDLP)++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
	gDPSetCycleType((*ppDLP)++, G_CYC_1CYCLE) ;
	CGeometry__SetCombineMode(ppDLP, G_CC_SHADE__G_CC_SHADE) ;
  	gSPClearGeometryMode((*ppDLP)++, G_LIGHTING | G_CULL_BACK | G_TEXTURE_GEN | G_FOG);
	gSPSetGeometryMode((*ppDLP)++, G_SHADE | G_SHADING_SMOOTH);

	// set up the orthographic projection matrix
	gSPMatrix((*ppDLP)++, RSP_ADDRESS(&mtx_pause),
	   		 G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);

	// create a model matrix, with proper 2d coordinates (0,0 topleft)
	CMtxF__Scale(mfPause, 1, -1, 1);
	CMtxF__PostMultTranslate(mfPause, 0, SCREEN_HT, 0);

	CMtxF__ToMtx(mfPause, mPause);
	gSPMatrix((*ppDLP)++, RSP_ADDRESS(&mPause),
	   		 G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);

	polyvertex = 0 ;
}



void CPause__DrawBar(Gfx **ppDLP, float x, float y, float w, float h, float a)
{
	Vtx 			*vtxs ;
	float			fade ;
	float			h1, h2 ;
	float			r1, g1, b1 ;
	float			r2, g2, b2 ;
	float			r1a, g1a, b1a ;
	float			r1b, g1b, b1b ;
	int			vertex,
					i;
	float			blend, t1, t2 ;

	vtxs = pause_vtxs[even_odd];
	vertex = polyvertex ;

	fade = w/3 ;			// xoffset for fading alpha

	h1 = (h/3) ;			// yoffsets for fading alphas
	h2 = h-(h/3) ;

#if 0
	r = 96+(32 * sin((float)frame_number/8)) ;
	b = 96+(32 * cos((float)frame_number/8)) ;
	//g = 96+(32 * cos((float)frame_number/42)) ;
	g = 128-r ;
#endif

	BarTimer += .06 * frame_increment ;
	if (BarTimer >2)
		BarTimer -= 2 ;

	if (BarTimer >1)
		blend = 2-BarTimer ;
	else
		blend = BarTimer ;

	r1 = BlendFLOAT(blend, 128, 128/2) ;
	g1 = BlendFLOAT(blend, 80, 80/2) ;
	b1 = BlendFLOAT(blend, 80, 80/2) ;
	r2 = BlendFLOAT(blend, 128, 128/2) ;
	g2 = BlendFLOAT(blend, 128, 128/2) ;
	b2 = BlendFLOAT(blend, 60, 60/2) ;

	t1 = h1/h ;
	t2 = h2/h ;
	r1a = BlendFLOAT(t1, r1, r2) ;
	g1a = BlendFLOAT(t1, g1, g2) ;
	b1a = BlendFLOAT(t1, b1, b2) ;
	r1b = BlendFLOAT(t2, r1, r2) ;
	g1b = BlendFLOAT(t2, g1, g2) ;
	b1b = BlendFLOAT(t2, b1, b2) ;




	// top left
	vtxs[polyvertex].v.ob[0] = x ;
	vtxs[polyvertex].v.ob[1] = y ;
	vtxs[polyvertex].v.cn[0] = r1 ;
	vtxs[polyvertex].v.cn[1] = g1 ;
	vtxs[polyvertex].v.cn[2] = b1 ;
	vtxs[polyvertex].v.cn[3] = 0 ;
	polyvertex++ ; 

	// top mid left
	vtxs[polyvertex].v.ob[0] = x+fade ;
	vtxs[polyvertex].v.ob[1] = y ;
	vtxs[polyvertex].v.cn[0] = BlendFLOAT(fade/w, r1, r2) ;
	vtxs[polyvertex].v.cn[1] = BlendFLOAT(fade/w, g1, g2) ;
	vtxs[polyvertex].v.cn[2] = BlendFLOAT(fade/w, b1, b2) ;
	vtxs[polyvertex].v.cn[3] = 0 ;
	polyvertex++ ; 

	// top mid right
	vtxs[polyvertex].v.ob[0] = x+w-fade ;
	vtxs[polyvertex].v.ob[1] = y ;
	vtxs[polyvertex].v.cn[0] = BlendFLOAT((w-fade)/w, r1, r2) ;
	vtxs[polyvertex].v.cn[1] = BlendFLOAT((w-fade)/w, g1, g2) ;
	vtxs[polyvertex].v.cn[2] = BlendFLOAT((w-fade)/w, b1, b2) ;
	vtxs[polyvertex].v.cn[3] = 0 ;
	polyvertex++ ; 

	// top right
	vtxs[polyvertex].v.ob[0] = x+w ;
	vtxs[polyvertex].v.ob[1] = y ;
	vtxs[polyvertex].v.cn[0] = r2 ;
	vtxs[polyvertex].v.cn[1] = b2 ;
	vtxs[polyvertex].v.cn[2] = g2 ;
	vtxs[polyvertex].v.cn[3] = 0 ;
	polyvertex++ ; 


	
	// top2 left
	vtxs[polyvertex].v.ob[0] = x ;
	vtxs[polyvertex].v.ob[1] = y+h1 ;
	vtxs[polyvertex].v.cn[0] = r1a ;
	vtxs[polyvertex].v.cn[1] = g1a ;
	vtxs[polyvertex].v.cn[2] = b1a ;
	vtxs[polyvertex].v.cn[3] = 0 ;
	polyvertex++ ; 

	// top2 mid left
	vtxs[polyvertex].v.ob[0] = x+fade ;
	vtxs[polyvertex].v.ob[1] = y+h1 ;
	vtxs[polyvertex].v.cn[0] = BlendFLOAT(fade/w, r1a, r1b) ;
	vtxs[polyvertex].v.cn[1] = BlendFLOAT(fade/w, g1a, g1b) ;
	vtxs[polyvertex].v.cn[2] = BlendFLOAT(fade/w, b1a, b1b) ;
	vtxs[polyvertex].v.cn[3] = 255 *a;
	polyvertex++ ; 

	// top2 mid right
	vtxs[polyvertex].v.ob[0] = x+w-fade ;
	vtxs[polyvertex].v.ob[1] = y+h1 ;
	vtxs[polyvertex].v.cn[0] = BlendFLOAT((w-fade)/w, r1a, r1b) ;
	vtxs[polyvertex].v.cn[1] = BlendFLOAT((w-fade)/w, g1a, g1b) ;
	vtxs[polyvertex].v.cn[2] = BlendFLOAT((w-fade)/w, b1a, b1b) ;
	vtxs[polyvertex].v.cn[3] = 255 *a;
	polyvertex++ ; 

	// top2 right
	vtxs[polyvertex].v.ob[0] = x+w ;
	vtxs[polyvertex].v.ob[1] = y+h1 ;
	vtxs[polyvertex].v.cn[0] = r1b ;
	vtxs[polyvertex].v.cn[1] = b1b ;
	vtxs[polyvertex].v.cn[2] = g1b ;
	vtxs[polyvertex].v.cn[3] = 0 ;
	polyvertex++ ; 



	// top3 left
	vtxs[polyvertex].v.ob[0] = x ;
	vtxs[polyvertex].v.ob[1] = y+h2 ;
	vtxs[polyvertex].v.cn[0] = r1b ;
	vtxs[polyvertex].v.cn[1] = g1b ;
	vtxs[polyvertex].v.cn[2] = b1b ;
	vtxs[polyvertex].v.cn[3] = 0 ;
	polyvertex++ ; 

	// top3 mid left
	vtxs[polyvertex].v.ob[0] = x+fade ;
	vtxs[polyvertex].v.ob[1] = y+h2 ;
	vtxs[polyvertex].v.cn[0] = BlendFLOAT(fade/w, r1b, r1a) ;
	vtxs[polyvertex].v.cn[1] = BlendFLOAT(fade/w, g1b, g1a) ;
	vtxs[polyvertex].v.cn[2] = BlendFLOAT(fade/w, b1b, b1a) ;
	vtxs[polyvertex].v.cn[3] = 255 *a;
	polyvertex++ ; 

	// top3 mid right
	vtxs[polyvertex].v.ob[0] = x+w-fade ;
	vtxs[polyvertex].v.ob[1] = y+h2 ;
	vtxs[polyvertex].v.cn[0] = BlendFLOAT((w-fade)/w, r1b, r1a) ;
	vtxs[polyvertex].v.cn[1] = BlendFLOAT((w-fade)/w, g1b, g1a) ;
	vtxs[polyvertex].v.cn[2] = BlendFLOAT((w-fade)/w, b1b, b1a) ;
	vtxs[polyvertex].v.cn[3] = 255 *a;
	polyvertex++ ; 

	// top3 right
	vtxs[polyvertex].v.ob[0] = x+w ;
	vtxs[polyvertex].v.ob[1] = y+h2 ;
	vtxs[polyvertex].v.cn[0] = r1a ;
	vtxs[polyvertex].v.cn[1] = b1a ;
	vtxs[polyvertex].v.cn[2] = g1a ;
	vtxs[polyvertex].v.cn[3] = 0 ;
	polyvertex++ ; 




	// bottom left
	vtxs[polyvertex].v.ob[0] = x ;
	vtxs[polyvertex].v.ob[1] = y+h ;
	vtxs[polyvertex].v.cn[0] = r2 ;
	vtxs[polyvertex].v.cn[1] = g2 ;
	vtxs[polyvertex].v.cn[2] = b2 ;
	vtxs[polyvertex].v.cn[3] = 0 ;
	polyvertex++ ; 

	// bottom mid left
	vtxs[polyvertex].v.ob[0] = x+fade ;
	vtxs[polyvertex].v.ob[1] = y+h ;
	vtxs[polyvertex].v.cn[0] = BlendFLOAT(fade/w, r2, r1) ;
	vtxs[polyvertex].v.cn[1] = BlendFLOAT(fade/w, g2, b1) ;
	vtxs[polyvertex].v.cn[2] = BlendFLOAT(fade/w, b2, g1) ;
	vtxs[polyvertex].v.cn[3] = 0 ;
	polyvertex++ ; 

	// bottom mid right
	vtxs[polyvertex].v.ob[0] = x+w-fade ;
	vtxs[polyvertex].v.ob[1] = y+h ;
	vtxs[polyvertex].v.cn[0] = BlendFLOAT((w-fade)/w, r2, r1) ;
	vtxs[polyvertex].v.cn[1] = BlendFLOAT((w-fade)/w, g2, g1) ;
	vtxs[polyvertex].v.cn[2] = BlendFLOAT((w-fade)/w, b2, b1) ;
	vtxs[polyvertex].v.cn[3] = 0 ;
	polyvertex++ ; 

	// bottom right
	vtxs[polyvertex].v.ob[0] = x+w ;
	vtxs[polyvertex].v.ob[1] = y+h ;
	vtxs[polyvertex].v.cn[0] = r1 ;
	vtxs[polyvertex].v.cn[1] = g1 ;
	vtxs[polyvertex].v.cn[2] = b1 ;
	vtxs[polyvertex].v.cn[3] = 0 ;
	polyvertex++ ; 

	gSPVertex((*ppDLP)++,
     			 RSP_ADDRESS(vtxs),
     			 polyvertex, 0);

	for (i=0; i<3; i++)
	{
		gSP2Triangles((*ppDLP)++,
						  vertex+0, vertex+1, vertex+4, 0,
						  vertex+1, vertex+5, vertex+4, 0);
		gSP2Triangles((*ppDLP)++,
						  vertex+1, vertex+2, vertex+5, 0,
						  vertex+2, vertex+6, vertex+5, 0);
		gSP2Triangles((*ppDLP)++,
						  vertex+2, vertex+3, vertex+6, 0,
						  vertex+3, vertex+7, vertex+6, 0);

		vertex += 4;
	}
}



void CPause__Draw(CPause *pThis, Gfx **ppDLP)
{
	int			i,
					yStart,
					PAUSE_HEIGHT,
					PAUSE_Y,
					TITLE_HEIGHT,
					TITLE_Y;
#ifndef SHIP_IT
	char		buffer[16] ;
#endif

	// update fade status
	switch (pThis->m_Mode)
	{
		case PAUSE_FADEUP:
			pThis->m_Alpha += .2 * frame_increment ;
			if (pThis->m_Alpha >= 1.0)
			{
				pThis->m_Alpha = 1.0 ;
				pThis->m_Mode = PAUSE_NORMAL ;
			}
			break ;

		case PAUSE_FADEDOWN:
			pThis->m_Alpha -= .2 * frame_increment ;
			if (pThis->m_Alpha <= 0.0)
			{
				pThis->m_Alpha = 0.0 ;
				if (GetApp()->m_bPause == FALSE)
				{
					if ((GetApp()->m_bRequestor == FALSE) && (GetApp()->m_bSave == FALSE))
						pThis->m_Mode = PAUSE_NULL ;
				}
				else
					pThis->m_Mode = PAUSE_NORMAL ;
			}
			break ;
	}


	if (pThis->m_Alpha != 0.0)
	{
		// TITLE OPTIONS
		if (GetApp()->m_Mode == MODE_TITLE)
		{
			TITLE_HEIGHT = CPause__GetTitleOptionAmt(pThis);
			TITLE_HEIGHT *= TITLEMENU_SPACING ;
			TITLE_HEIGHT += TITLEMENU_Y*2;
			TITLE_Y = (178 - (TITLE_HEIGHT/2));

			// prepare to draw boxes
			COnScreen__InitBoxDraw(ppDLP) ;
			COnScreen__DrawHilightBox(ppDLP,
									 TITLE_X, TITLE_Y,
									 TITLE_X+TITLE_WIDTH, TITLE_Y+TITLE_HEIGHT,
									 1, FALSE,
									 0,0,0, 150 * pThis->m_Alpha) ;


			// draw polygon selection bar
			CPause__InitPolygon(ppDLP) ;
			CPause__DrawBar(ppDLP, TITLE_X+8, TITLE_Y+TITLEMENU_Y+(pThis->m_RealSelection*TITLEMENU_SPACING),
								TITLE_WIDTH-16, TITLEMENU_SPACING, pThis->m_Alpha) ;



			switch (GetApp()->m_Difficulty)
			{
				case EASY:
					title_text[TITLE_DIFFICULTY] = text_difficulty_easy ;
					break ;
				case MEDIUM:
					title_text[TITLE_DIFFICULTY] = text_difficulty_normal ;
					break ;
				case HARD:
					title_text[TITLE_DIFFICULTY] = text_difficulty_hard ;
					break ;
			}


			// draw each option
			COnScreen__InitFontDraw(ppDLP) ;
			COnScreen__SetFontScale(1.0, 0.8) ;
			COnScreen__SetFontColor(ppDLP, 200*.9, 200*.9, 138*.9, 86*.9, 71*.9, 47*.9) ;

			// display version number
	#ifndef SHIP_IT
			sprintf(buffer, "build %d", VERSION_NUMBER) ;
			COnScreen__DrawText(ppDLP, buffer, 320/2, 16, (int)(255 * pThis->m_Alpha), TRUE, TRUE);
	#endif

			yStart = TITLE_Y + TITLEMENU_Y;

			for (i=0; i<TITLE_END_SELECTION; i++)
			{
				if (title_menu_items[i])
				{
					// set selected item brighter
					if (i == pThis->m_Selection)
						COnScreen__SetFontColor(ppDLP, 200*1.25, 200*1.25, 138*1.25, 86*1.25, 71*1.25,47*1.25) ;

					COnScreen__DrawText(ppDLP, title_text[i], 320/2, yStart, (int)(255 * pThis->m_Alpha), TRUE, TRUE);
					yStart += TITLEMENU_SPACING;

					// set back to normal color
					if (i == pThis->m_Selection)
						COnScreen__SetFontColor(ppDLP, 200*.9, 200*.9, 138*.9, 86*.9, 71*.9, 47*.9) ;
				}
			}
		}
		// GAME PAUSE
		else
		{
			PAUSE_HEIGHT = CPause__GetPauseOptionAmt(pThis) ;
			PAUSE_HEIGHT *= PAUSEMENU_SPACING ;
			PAUSE_HEIGHT += PAUSEMENU_Y ;
			PAUSE_HEIGHT += 6 ;
			PAUSE_Y = ((240/2) - (PAUSE_HEIGHT/2)) ;

			// prepare to draw boxes
			COnScreen__InitBoxDraw(ppDLP) ;
			COnScreen__DrawHilightBox(ppDLP,
									 PAUSE_X, PAUSE_Y,
									 PAUSE_X+PAUSE_WIDTH, PAUSE_Y+PAUSE_HEIGHT,
									 1, FALSE,
									 0,0,0, 150 * pThis->m_Alpha) ;

			// draw polygon selection bar
			CPause__InitPolygon(ppDLP) ;
			CPause__DrawBar(ppDLP, PAUSE_X+8, PAUSE_Y+PAUSEMENU_Y+(pThis->m_RealSelection*PAUSEMENU_SPACING),
						PAUSE_WIDTH-16, PAUSEMENU_SPACING, pThis->m_Alpha) ;





			// draw heading
			COnScreen__Init16BitDraw(ppDLP, (int)(255 * pThis->m_Alpha)) ;
			// ★ m_Width is a BIG-ENDIAN C16BitGraphic header field — read RAW here it byte-swaps to garbage on
			// the LE host, so the heading centred way off the box (PC + 3DS). Swap it like onscrn.c's own reads.
			COnScreen__Draw16BitGraphic(ppDLP,
												(C16BitGraphic *)PauseOverlay,
												320/2 - (ONSCRN_SW16(((C16BitGraphic *)PauseOverlay)->m_Width)/2), PAUSE_Y+8) ;


#ifndef SHIP_IT
			if (GetApp()->m_bDebug)
				pause_text[PAUSE_DEBUG] = text_debug_on ;
			else
				pause_text[PAUSE_DEBUG] = text_debug_off ;
#endif

			// draw each option
			COnScreen__InitFontDraw(ppDLP) ;
			COnScreen__SetFontScale(1.0, 1.0) ;
			COnScreen__SetFontColor(ppDLP, 200*.9, 200*.9, 138*.9, 86*.9, 71*.9, 47*.9) ;
			yStart = PAUSE_Y + PAUSEMENU_Y;

			for (i=0; i<PAUSE_END_SELECTION; i++)
			{
				if (pause_menu_items[i])
				{
					// set selected item brighter
					if (i == pThis->m_Selection)
						COnScreen__SetFontColor(ppDLP, 200*1.25, 200*1.25, 138*1.25, 86*1.25, 71*1.25,47*1.25) ; // default colors (from onscrn.cpp)

					COnScreen__DrawText(ppDLP, pause_text[i], 320/2, yStart, (int)(255 * pThis->m_Alpha), TRUE, TRUE);
					yStart += PAUSEMENU_SPACING;

					// set back to normal color
					if (i == pThis->m_Selection)
						COnScreen__SetFontColor(ppDLP, 200*.9, 200*.9, 138*.9, 86*.9, 71*.9, 47*.9) ;
				}
			}
		}
	}
	// only draw other screens when this has faded away...
//	else
//	{
		if (GetApp()->m_bLoad)
			CLoad__Draw(&GetApp()->m_Load, ppDLP) ;

		if (GetApp()->m_bSave)
			CSave__Draw(&GetApp()->m_Save, ppDLP) ;

		if (GetApp()->m_bOptions)
			COptions__Draw(&GetApp()->m_Options, ppDLP) ;

		if (GetApp()->m_bKeys)
			CKeys__Draw(&GetApp()->m_Keys, ppDLP) ;

		if (GetApp()->m_bCheat)
			CCheat__Draw(&GetApp()->m_Cheat, ppDLP) ;

		if (GetApp()->m_bCheatCode)
			CCheatCode__Draw(&GetApp()->m_CheatCode, ppDLP) ;

		if (GetApp()->m_bGiveCheatCode)
			CGiveCheatCode__Draw(&GetApp()->m_GiveCheatCode, ppDLP) ;

		if (GetApp()->m_bGInst)
			CGInst__Draw(&GetApp()->m_GInst, ppDLP) ;
//	}

	if (GetApp()->m_bRequestor)
		CRequestor__Draw(&GetApp()->m_Requestor, ppDLP) ;

	if (GetApp()->m_bMessageBox)
		CMessageBox__Draw(&GetApp()->m_MessageBox, ppDLP) ;


}



/*
//------------------------------------------------------------------------
//
// FRAMEBUFFER Method
//
//------------------------------------------------------------------------
INT32 CPause__Update(CPause *pThis, void *pBuffer)
{
	static BOOL		pressed = FALSE;
	INT32				ReturnValue = -1 ;
	static int		sfx_num = 0;
	CVector3			tempvec;

	// Goto next selection?
	if (CTControl__IsClickDown(pThis->m_pCTControl))
	{
		pThis->m_Time = 0 ;
		pThis->m_Selection++ ;
		if (pThis->m_Selection >= PAUSE_END_SELECTION)
			pThis->m_Selection = 0 ;


#if SFXTEST
		sfx_num++;
		rmonPrintf("SFX: %d\n", sfx_num);
#endif

	}

	// Goto previous selection?
	if (CTControl__IsClickUp(pThis->m_pCTControl))
	{
		pThis->m_Time = 0 ;
		pThis->m_Selection-- ;
		if (pThis->m_Selection < 0)
			pThis->m_Selection = PAUSE_END_SELECTION-1 ;

#if SFXTEST
		sfx_num--;
		if(sfx_num < 0)
			sfx_num = 0;
		rmonPrintf("SFX: %d\n", sfx_num);

#endif
	}

#if SFXTEST

	if(CTControl__IsSelectWeaponPrev(pThis->m_pCTControl))
	{
		tempvec.x = 0.00;
		tempvec.y = 0.00;
		tempvec.z = 0.00;

		CScene__DoSoundEffect(&GetApp()->m_Scene, sfx_num, 0, NULL, &tempvec);
	}

	if(CTControl__IsSelectWeaponNext(pThis->m_pCTControl))
		killAllSFX();

#endif

#if SFXTEST
	if(CTControl__IsSideStepRight(pThis->m_pCTControl))
	{
		sfx_num += 5;
		rmonPrintf("sfxnum: %d\n", sfx_num);
	}

	if(CTControl__IsSideStepLeft(pThis->m_pCTControl))
	{
		sfx_num -= 5;
		if(sfx_num < 0)
			sfx_num = 0;
		rmonPrintf("sfxnum: %d\n", sfx_num);
	}

#endif


	// Draw pause screen
	WriteGraphicToScreen((UINT16 *)pBuffer, pThis->m_pPauseScreen, PAUSE_WIDTH, PAUSE_HEIGHT,
									  PAUSE_X_POS, PAUSE_Y_POS) ;

	// Draw selection arrow
	WriteGraphicToScreen((UINT16 *)pBuffer, pThis->m_pPauseScreenArrow, ARROW_WIDTH, ARROW_HEIGHT,
									  MENU_X_POS, MENU_Y_POS + (pThis->m_Selection * MENU_SPACING)) ;


	// Exit if start pressed
	if (pressed)
	{
		if (!CTControl__IsStart(pThis->m_pCTControl))
		{
			ReturnValue = pThis->m_Selection ;
			pressed = FALSE;
		}
	}
	else
	{
		if (CTControl__IsStart(pThis->m_pCTControl))
			pressed = TRUE;
	}

	// Go back to front end if paused for too long
	if (pThis->m_Time++ > (60*30))
		ReturnValue = PAUSE_RESTART_GAME ;

	return ReturnValue ;
}

*/

