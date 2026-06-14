// Train.Cpp

#include "cppu64.h"
#include "tengine.h"
#include "train.h"
#include "tmove.h"


/////////////////////////////////////////////////////////////////////////////
// Constants
/////////////////////////////////////////////////////////////////////////////

// Position and dimentions of screen
//#ifdef GERMAN
#define TRAINING_WIDTH	(280)
//#else
//#define TRAINING_WIDTH	(268)
//#endif
#define TRAINING_HEIGHT	(180)
#define TRAINING_X		((320/2) - (TRAINING_WIDTH/2))
#define TRAINING_Y		((240/2) - (TRAINING_HEIGHT/2))



//-------------------------------------------------------------------------
// Text
//-------------------------------------------------------------------------
static	char	text_blankline[] = {0} ;

#ifdef ENGLISH
// ---------------------- English text ---------------------------
static char text_traininglevel[] = {"training level"} ;
static char text_atrain1[] = {"to begin, press"} ;
static char text_atrain2[] = {"any button."} ;
static char text_atrain3[] = {"to exit at any time,"} ;
static char text_atrain4[] = {"press start."} ;

static char text_jumping[] = {"jumping"} ;
static char text_btrain1[] = {"press the r"} ;
static char text_btrain2[] = {"button to jump."} ;
static char text_btrain3[] = {"the longer you hold"};
static char text_btrain4[] = {"jump, the further"};
static char text_btrain5[] = {"turok moves."};

static char text_swimming[] = {"swimming"} ;
static char text_ctrain1[] = {"to swim, press the"};
static char text_ctrain2[] = {"c buttons to move."};
static char text_ctrain3[] = {"the r button"};
static char text_ctrain4[] = {"will make turok"};
static char text_ctrain5[] = {"surface."};

static char text_climbing[] = {"climbing"} ;
static char text_dtrain1[] = {"climb up the cliff"};
static char text_dtrain2[] = {"with the c buttons."};
static char text_dtrain3[] = {"try looking around"};
static char text_dtrain4[] = {"with the control stick,"};
static char text_dtrain5[] = {"or jumping off."};

static char text_walking[] = {"walking"} ;
static char text_etrain1[] = {"carefully walk across"};
static char text_etrain2[] = {"the log."};
static char text_etrain3[] = {"use the control pad"};
static char text_etrain4[] = {"to toggle run or walk."};

static char text_longjump[] = {"long jump"} ;
static char text_ftrain1[] = {"remember to hold"};
static char text_ftrain2[] = {"the r button"};
static char text_ftrain3[] = {"to jump further."};

static char text_welldone[] = {"well done,"} ;
static char text_youhavecompleted[] = {"you have completed"} ;
static char text_gtrain1[] = {"the apprentice"};
static char text_gtrain2[] = {"training level."};
static char text_gtrain3[] = {"step onto the teleport"};
static char text_gtrain4[] = {"to try the"};
static char text_gtrain5[] = {"intermediate level."};

										  //"swimming"
static char text_htrain1[] = {"to swim through the"};
static char text_htrain2[] = {"gates, steer with"};
static char text_htrain3[] = {"the control stick and "};
static char text_htrain4[] = {"use the c buttons to"};
static char text_htrain5[] = {"move left and right."};

static char text_runninglongjump[] = {"running long jump"} ;
static char text_itrain1[] = {"grab onto the"};
static char text_itrain2[] = {"cliff by jumping"};
static char text_itrain3[] = {"across the gap."};

										  //"walking"
static char text_jtrain1[] = {"carefully walk across"};
static char text_jtrain2[] = {"the log bridge."};
static char text_jtrain3[] = {"try looking down"};
static char text_jtrain4[] = {"while you are"};
static char text_jtrain5[] = {"walking to help you."};

static char text_targetpractice[] = {"target practice"} ;
static char text_ktrain1[] = {"use the arrows to"};
static char text_ktrain2[] = {"shoot the snipers,"};
static char text_ktrain3[] = {"to make the pillars"};
static char text_ktrain4[] = {"appear."};

							  //"well done"
							  //"you have completed"
static char text_ltrain1[] = {"the intermediate"};
static char text_ltrain2[] = {"training level."};
static char text_ltrain3[] = {"step onto the"};
static char text_ltrain4[] = {"teleport to try"};
static char text_ltrain5[] = {"the advanced level."};

static char text_deepdiving[] = {"deep diving"} ;
static char text_mtrain1[] = {"swim to the bottom"};
static char text_mtrain2[] = {"of the pool to find"};
static char text_mtrain3[] = {"your way to the"};
static char text_mtrain4[] = {"other side."};

						 //"jumping"
static char text_ntrain1[] = {"jump onto the"};
static char text_ntrain2[] = {"platform."};
static char text_ntrain3[] = {"try looking down"};
static char text_ntrain4[] = {"while jumping to"};
static char text_ntrain5[] = {"guide you."};

static char text_mapmode[] = {"map mode"} ;
static char text_otrain1[] = {"press the top left"};
static char text_otrain2[] = {"button to activate"};
static char text_otrain3[] = {"map mode to find"};
static char text_otrain4[] = {"your way through"};
static char text_otrain5[] = {"the caves."};

static char text_diagonaljump[] = {"diagonal jump"} ;
static char text_ptrain1[] = {"press the jump button"};
static char text_ptrain2[] = {"while holding down the"};
static char text_ptrain3[] = {"forward and strafe"};
static char text_ptrain4[] = {"buttons to perform a"};
static char text_ptrain5[] = {"diagonal leap. use"};
static char text_ptrain6[] = {"the control stick to"};
static char text_ptrain7[] = {"turn in the air."};

static char text_advanced[] = {"advanced"} ;
						//"target practice"
static char text_qtrain1[] = {"use the arrows to"};
static char text_qtrain2[] = {"shoot the snipers."};

static char text_activatingswitches[] = {"activating switches"} ;
static char text_rtrain1[] = {"find the button"};
static char text_rtrain2[] = {"to open the"};
static char text_rtrain3[] = {"log gate."};

						//"well done"
static char text_strain1[] = {"you are obviously"};
static char text_strain2[] = {"quite skilled,"};
static char text_strain3[] = {"good hunting."};

char	*Train1[] =
{
		text_traininglevel,
	text_blankline,
		text_atrain1,
		text_atrain2,
	text_blankline,
		text_atrain3,
		text_atrain4,
		0,
} ;
char	*Train2[] =
{
		text_jumping,
	text_blankline,
		text_btrain1,
		text_btrain2,
	text_blankline,
		text_btrain3,
		text_btrain4,
		text_btrain5,
		0,
} ;
char	*Train3[] =
{
		text_swimming,
	text_blankline,
		text_ctrain1,
		text_ctrain2,
	text_blankline,
		text_ctrain3,
		text_ctrain4,
		text_ctrain5,
		0,
} ;
char	*Train4[] =
{
		text_climbing,
	text_blankline,
		text_dtrain1,
		text_dtrain2,
	text_blankline,
		text_dtrain3,
		text_dtrain4,
		text_dtrain5,
		0,
} ;
char	*Train5[] =
{
		text_walking,
	text_blankline,
		text_etrain1,
		text_etrain2,
	text_blankline,
		text_etrain3,
		text_etrain4,	// german	text_etrain5,
		0,
} ;
char	*Train6[] =
{
		text_longjump,
	text_blankline,
		text_ftrain1,
		text_ftrain2,
		text_ftrain3,
		0,
} ;
char	*Train7[] =
{
		text_welldone,
		text_youhavecompleted,
		text_gtrain1,
		text_gtrain2,
	text_blankline,
		text_gtrain3,
		text_gtrain4,
		text_gtrain5,
		0,
} ;

char	*Train8[] =
{
		text_swimming,
	text_blankline,
		text_htrain1,
		text_htrain2,
		text_htrain3,
		text_htrain4,
		text_htrain5,	// upto 6 in german
		0,
} ;
char	*Train9[] =
{
		text_runninglongjump,
	text_blankline,
		text_itrain1,
		text_itrain2,
		text_itrain3,
		0,
} ;
char	*Train10[] =
{
		text_walking,
	text_blankline,
		text_jtrain1,
		text_jtrain2,
	text_blankline,
		text_jtrain3,
		text_jtrain4,
		text_jtrain5,
		0,
} ;
char	*Train11[] =
{
		text_targetpractice,
	text_blankline,
		text_ktrain1,
		text_ktrain2,
		text_ktrain3,
		text_ktrain4,
		0,
} ;
char	*Train12[] =
{
		text_welldone,
		text_youhavecompleted,
		text_ltrain1,
		text_ltrain2,
	text_blankline,
		text_ltrain3,
		text_ltrain4,
		text_ltrain5,
		0,
} ;

char	*Train13[] =
{
		text_deepdiving,
	text_blankline,
		text_mtrain1,
		text_mtrain2,
		text_mtrain3,
		text_mtrain4,
		0,
} ;
char	*Train14[] =
{
		text_jumping,
	text_blankline,
		text_ntrain1,
		text_ntrain2,
	text_blankline,
		text_ntrain3,
		text_ntrain4,
		text_ntrain5,
		0,
} ;
char	*Train15[] =
{
		text_mapmode,
	text_blankline,
		text_otrain1,
		text_otrain2,
		text_otrain3,
		text_otrain4,
		text_otrain5,
		0,
} ;
char	*Train16[] =
{
		text_diagonaljump,
	text_blankline,
		text_ptrain1,
		text_ptrain2,
		text_ptrain3,
		text_ptrain4,
		text_ptrain5,
		text_ptrain6,
		text_ptrain7,
		0,
} ;
char	*Train17[] =
{
		text_advanced,
		text_targetpractice,
	text_blankline,
		text_qtrain1,
		text_qtrain2,
		0,
} ;
char	*Train18[] =
{
		text_activatingswitches,
	text_blankline,
		text_rtrain1,
		text_rtrain2,
		text_rtrain3,
		0,
} ;
char	*Train19[] =
{
		text_welldone,
	text_blankline,
		text_strain1,
		text_strain2,
	text_blankline,
		text_strain3,
		0,
} ;

#endif


#ifdef GERMAN
// ---------------------- German text ---------------------------
static char text_traininglevel[] = {"trainingslevel"} ;
static char text_atrain1[] = {"zum anfangen bitte"} ;
static char text_atrain2[] = {"beliebigen knopf dr/cken."} ;		//u.
static char text_atrain3[] = {"um jederzeit zu beenden,"} ;
static char text_atrain4[] = {"start dr/cken."} ;				//u.

static char text_jumping[] = {"springen"} ;
static char text_btrain1[] = {"zum springen"} ;
static char text_btrain2[] = {"knopf r dr/cken."} ;		//u.
static char text_btrain3[] = {"je l<nger du den sprungknopf"};		//a.
static char text_btrain4[] = {"gedr/ckt h<ltst, desto"};			//u.a.
static char text_btrain5[] = {"weiter bewegt sich turok."};

static char text_swimming[] = {"schwimmen"} ;
static char text_ctrain1[] = {"um zu schwimmen dr/ckst du"};		//u.
static char text_ctrain2[] = {"die c kn>pfe zum bewegen."};
static char text_ctrain3[] = {"mit dem knopf r"};
static char text_ctrain4[] = {"befiehlst du turok"};
static char text_ctrain5[] = {"aufzutauchen."};

static char text_climbing[] = {"klettern"} ;
static char text_dtrain1[] = {"mit den c kn>pfen kletterst"};
static char text_dtrain2[] = {"du die klippen hinauf."};
static char text_dtrain3[] = {"versuche dich beim klettern"};
static char text_dtrain4[] = {"umzusehen indem du den analog"};
static char text_dtrain5[] = {"stick bewegst oder"};
static char text_dtrain6[] = {"springe hinunter."};

static char text_walking[] = {"gehen"} ;
static char text_etrain1[] = {"gehe vorsichtig /ber"};		//u.
static char text_etrain2[] = {"den stamm."};
static char text_etrain3[] = {"mit dem steuerkreuz"};
static char text_etrain4[] = {"schaltest du zwischen gehen"};
static char text_etrain5[] = {"und laufen um."};

static char text_longjump[] = {"weitsprung"} ;
static char text_ftrain1[] = {"halte im laufen modus"};
static char text_ftrain2[] = {"den knopf r gedr/ckt,"};		//u.
static char text_ftrain3[] = {"um weiter zu springen."};

static char text_welldone[] = {"gut gemacht"} ;
static char text_youhavecompleted[] = {"du hast das"} ;
static char text_gtrain1[] = {"training f/r anf<nger"};		//u.a.
static char text_gtrain2[] = {"abgeschlossen."};
static char text_gtrain3[] = {"mit dem teleport"};
static char text_gtrain4[] = {"gelangst du zum training"};
static char text_gtrain5[] = {"f/r fortgeschrittene."};	//u.

										  //"swimming"
static char text_htrain1[] = {"um durch die tore"};
static char text_htrain2[] = {"zu schwimmen, steuere"};
static char text_htrain3[] = {"mit dem analog stick und"};
static char text_htrain4[] = {"verwende die c kn>pfe, um"};
static char text_htrain5[] = {"dich nach links und rechts"};
static char text_htrain6[] = {"zu bewegen."};

static char text_runninglongjump[] = {"weitsprung aus dem lauf"} ;
static char text_itrain1[] = {"finde halt an den klippen"};
static char text_itrain2[] = {"indem du /ber den"};		//u.
static char text_itrain3[] = {"spalt im boden springst."};

										  //"walking"
static char text_jtrain1[] = {"gehe vorsichtig /ber"};				//u.
static char text_jtrain2[] = {"die br/cke aus baumst<mmen."};		//u.a.
static char text_jtrain3[] = {"schau beim gehen"};
static char text_jtrain4[] = {"nach unten um dir das"};
static char text_jtrain5[] = {"/berqueren leichter"};		//u.
static char text_jtrain6[] = {"zu machen."};

static char text_targetpractice[] = {"ziel/bungen"} ;					//u.
static char text_ktrain1[] = {"schiesse mit den pfeilen"};
static char text_ktrain2[] = {"auf die roboter"};
static char text_ktrain3[] = {"damit die br/cke"};			//u.
static char text_ktrain4[] = {"erscheint."};

							  //"well done"
							  //"you have completed"
static char text_ltrain1[] = {"training f/r fortgeschrittene"};		//u.
static char text_ltrain2[] = {"abgeschlossen."};
static char text_ltrain3[] = {"mit dem teleporter"};
static char text_ltrain4[] = {"gelangst du zum training"};
static char text_ltrain5[] = {"training f/r profis."};			//u.

static char text_deepdiving[] = {"tauchen"} ;
static char text_mtrain1[] = {"tauche bis zum grund"};
static char text_mtrain2[] = {"des teichs, um den weg"};
static char text_mtrain3[] = {"zur anderen seite"};
static char text_mtrain4[] = {"zu finden."};

						 //"jumping"
static char text_ntrain1[] = {"springe auf die"};
static char text_ntrain2[] = {"plattform."};
static char text_ntrain3[] = {"das geht leichter, wenn"};
static char text_ntrain4[] = {"du w<hrend des sprungs nach"};	//a.
static char text_ntrain5[] = {"unten siehst."};

static char text_mapmode[] = {"kartenmodus"} ;
static char text_otrain1[] = {"dr/ck den knopf r,"};		//u.
static char text_otrain2[] = {"um den kartenmodus zu"};
static char text_otrain3[] = {"aktivieren und den weg"};
static char text_otrain4[] = {"durch die h>hlen"};			//o.
static char text_otrain5[] = {"zu finden."};

static char text_diagonaljump[] = {"diagonalsprung"} ;
static char text_ptrain1[] = {"dr/cke den sprungknopf,"};		//u.
static char text_ptrain2[] = {"und halte die c tasten"};
static char text_ptrain3[] = {"vorw<rts und links oder"};		//a.
static char text_ptrain4[] = {"rechts gedr/ckt, um"};		//u.
static char text_ptrain5[] = {"diagonal zu springen."};
static char text_ptrain6[] = {"mit dem analog stick kannst"};
static char text_ptrain7[] = {"du dich in der luft drehen."};

static char text_advanced[] = {"f/r fortgeschrittene"} ;		//u.
						//"target practice"
static char text_qtrain1[] = {"schiesse mit den pfeilen"};
static char text_qtrain2[] = {"auf die roboter."};

static char text_activatingswitches[] = {"schalter aktivieren"} ;
static char text_rtrain1[] = {"finde die schaltfl<che"};			//a.
static char text_rtrain2[] = {"zum >ffnen des"};					//o.
static char text_rtrain3[] = {"holztors."};

						//"well done"
static char text_strain1[] = {"du hast dein k>nnen"};				//o.
static char text_strain2[] = {"unter beweis gestellt."};
static char text_strain3[] = {"waidmannsheil."};


char	*Train1[] =
{
		text_traininglevel,
	text_blankline,
		text_atrain1,
		text_atrain2,
	text_blankline,
		text_atrain3,
		text_atrain4,
		0,
} ;
char	*Train2[] =
{
		text_jumping,
	text_blankline,
		text_btrain1,
		text_btrain2,
	text_blankline,
		text_btrain3,
		text_btrain4,
		text_btrain5,
		0,
} ;
char	*Train3[] =
{
		text_swimming,
	text_blankline,
		text_ctrain1,
		text_ctrain2,
	text_blankline,
		text_ctrain3,
		text_ctrain4,
		text_ctrain5,
		0,
} ;
char	*Train4[] =
{
		text_climbing,
	text_blankline,
		text_dtrain1,
		text_dtrain2,
	text_blankline,
		text_dtrain3,
		text_dtrain4,
		text_dtrain5,
		text_dtrain6,
		0,
} ;
char	*Train5[] =
{
		text_walking,
	text_blankline,
		text_etrain1,
		text_etrain2,
	text_blankline,
		text_etrain3,
		text_etrain4,
		text_etrain5,
		0,
} ;
char	*Train6[] =
{
		text_longjump,
	text_blankline,
		text_ftrain1,
		text_ftrain2,
		text_ftrain3,
		0,
} ;
char	*Train7[] =
{
		text_welldone,
		text_youhavecompleted,
		text_gtrain1,
		text_gtrain2,
	text_blankline,
		text_gtrain3,
		text_gtrain4,
		text_gtrain5,
		0,
} ;

char	*Train8[] =
{
		text_swimming,
	text_blankline,
		text_htrain1,
		text_htrain2,
		text_htrain3,
		text_htrain4,
		text_htrain5,
		text_htrain6,
		0,
} ;
char	*Train9[] =
{
		text_runninglongjump,
	text_blankline,
		text_itrain1,
		text_itrain2,
		text_itrain3,
		0,
} ;
char	*Train10[] =
{
		text_walking,
	text_blankline,
		text_jtrain1,
		text_jtrain2,
	text_blankline,
		text_jtrain3,
		text_jtrain4,
		text_jtrain5,
		text_jtrain6,
		0,
} ;
char	*Train11[] =
{
		text_targetpractice,
	text_blankline,
		text_ktrain1,
		text_ktrain2,
		text_ktrain3,
		text_ktrain4,
		0,
} ;
char	*Train12[] =
{
		text_welldone,
		text_youhavecompleted,
		text_ltrain1,
		text_ltrain2,
	text_blankline,
		text_ltrain3,
		text_ltrain4,
		text_ltrain5,
		0,
} ;

char	*Train13[] =
{
		text_deepdiving,
	text_blankline,
		text_mtrain1,
		text_mtrain2,
		text_mtrain3,
		text_mtrain4,
		0,
} ;
char	*Train14[] =
{
		text_jumping,
	text_blankline,
		text_ntrain1,
		text_ntrain2,
	text_blankline,
		text_ntrain3,
		text_ntrain4,
		text_ntrain5,
		0,
} ;
char	*Train15[] =
{
		text_mapmode,
	text_blankline,
		text_otrain1,
		text_otrain2,
		text_otrain3,
		text_otrain4,
		text_otrain5,
		0,
} ;
char	*Train16[] =
{
		text_diagonaljump,
	text_blankline,
		text_ptrain1,
		text_ptrain2,
		text_ptrain3,
		text_ptrain4,
		text_ptrain5,
		text_ptrain6,
		text_ptrain7,
		0,
} ;
char	*Train17[] =
{
		text_targetpractice,
		text_advanced,
	text_blankline,
		text_qtrain1,
		text_qtrain2,
		0,
} ;
char	*Train18[] =
{
		text_activatingswitches,
	text_blankline,
		text_rtrain1,
		text_rtrain2,
		text_rtrain3,
		0,
} ;
char	*Train19[] =
{
	text_blankline,
		text_welldone,
	text_blankline,
		text_strain1,
		text_strain2,
	text_blankline,
		text_strain3,
		0,
} ;

#endif


#ifdef KANJI
// ---------------------- Japanese text ---------------------------
static char text_traininglevel[] = {0x13, 0x24, 0x41, 0x15, 0x27, 0x29, 0x1F, 0x41, 0x2F, -1};
static char text_atrain1[] = {0xec, 0x68, 0x5a, 0x64, 0x0c, 0x0f, 0x41, 0x13, 0x33, 0x0f, 0x27, 0x7A, -1};
static char text_atrain2[] = {0x13, 0x24, 0x41, 0x15, 0x27, 0x29, 0x0c, 0x0f, 0x41, 0x13, -1};
static char text_atrain3[] = {0x6C, 0x6a, 0x70, 0x60, 0x64, -1};
static char text_atrain4[] = {0x0c, 0x0f, 0x41, 0x13, 0x33, 0x0f, 0x27, -1};


static char text_jumping[] = {0x2C, 0x3D, 0x27, 0x35, 0x27, 0x29, -1};
static char text_btrain1[] = {0x2C, 0x3D, 0x27, 0x36, 0x64, -1};
static char text_btrain2[] = {0xE9, 0x13, 0x22, 0x28, 0x41, 0x33, 0x0F, 0x27, -1};
static char text_btrain3[] = {0x33, 0x0F, 0x27, 0x74, 0xA4, 0x58, 0xc3, 0x56, 0x7a, -1};
static char text_btrain4[] = {0xC4, 0xC5, 0x6F, 0x76, 0xc6, 0x7d, 0x70, -1};

static char text_swimming[] = {0x0C, 0x01, 0x1C, 0x27, 0x29, -1};
static char text_ctrain1[] = {0xC7, 0x77, 0x63, 0xc9, 0xca, 0x64, -1};
static char text_ctrain2[] = {0xE4, 0x33, 0x0F, 0x27, 0x20, 0x15, 0x40, 0x13, -1};
//static char text_ctrain3[] = {0xBA, 0xC1, 0x60, -1};
static char text_ctrain4[] = {0xC8, 0x91, 0x58, 0x70, 0x60, 0x64, -1};
static char text_ctrain5[] = {0xE9, 0x13, 0x22, 0x28, 0x41, 0x33, 0x0F, 0x27, -1};


static char text_climbing[] = {0x07, 0x21, 0x01, 0x1C, 0x27, 0x29, -1};
static char text_dtrain1[] = {0x28, 0x08, 0x91, 0x6f, 0x64, -1};
static char text_dtrain2[] = {0xE4, 0x33, 0x0F, 0x27, 0x20, 0x15, 0x40, 0x13, 0x63, 0x91, 0x74, 0xa4, 0x58, -1};
static char text_dtrain3[] = {0x28, 0x08, 0x51, 0x6e, 0x73, 0x6f, 0x74, 0xea, 0x70, 0x63, 0x64, -1};
static char text_dtrain4[] = {0x44, 0xE5, 0x0C, 0x12, 0x3A, 0x40, 0x07, -1};
static char text_dtrain5[] = {0xC4, 0x7D, 0x50, 0x6F, 0x64, 0xE9, 0x13, 0x22, 0x28, 0x41, 0x33, 0x0F, 0x27, -1};

static char text_walking[] = {0xC9, 0xCA, -1};
static char text_etrain1[] = {0x44, 0xe5, 0x0c, 0x12, 0x3a, 0x40, 0x07, 0x7a, 0xd7, 0x6b, 0x5e, 0x60, -1};
static char text_etrain2[] = {0xcb, 0xcc, 0x57, 0x5d, 0xcd, 0xce, 0x74, 0x73, 0x5a, 0x71, -1};
static char text_etrain3[] = {0xCF, 0x53, 0xE3, 0xD0, 0x70, 0x63, 0xD1, 0x6F, 0x51, 0x4F, 0x64, -1};
static char text_etrain4[] = {0x8B, 0x8C, 0x33, 0x0F, 0x27, -1};

static char text_longjump[] = {0x25, 0x27, 0x29, 0x2c, 0x3D, 0x27, 0x36, -1};
static char text_ftrain1[] = {0xd0, 0x6f, 0x5f, 0x76, 0x6e, -1};
static char text_ftrain2[] = {0x2c, 0x3d, 0x27, 0x36, 0x58, 0x70, 0x5e, -1};
static char text_ftrain3[] = {0xC4, 0xc5, 0x6f, 0x76, 0x56, 0x6e, 0x60, 0xc6, 0x7d, 0x68, 0x58, -1};


static char text_welldone[] = {0x50, 0x6A, 0x7A, 0x5E, 0x4E, -1};
static char text_youhavecompleted[] = {0x55, 0x71, 0x7A, 0xD2, 0xD3, 0x09, 0x41, 0x0C, 0x63, -1};
static char text_gtrain1[] = {0x13, 0x24, 0x41, 0x15, 0x27, 0x29, 0x64, -1};
static char text_gtrain2[] = {0xA2, 0xA3, 0x7A, 0x58, -1};
static char text_gtrain3[] = {0xD4, 0xD3, 0x09, 0x41, 0x0C, 0x60, -1};
static char text_gtrain4[] = {0x58, 0x58, 0x69, 0x60, 0x64, -1};
static char text_gtrain5[] = {0x26, 0x41, 0x36, 0x74, 0x66, 0x75, 0x7A, 0x53, 0x79, 0x56, 0x4D, -1};

										  //"swimming"
static char text_htrain1[] = {0xC7, 0x77, 0x5F, 0x76, 0x6E, -1};
static char text_htrain2[] = {0x09, 0x41, 0x0c, 0x74, 0xe0, 0x4f, 0x70, 0x60, 0x64, -1};
static char text_htrain3[] = {0x44, 0xE5, 0x0C, 0x12, 0x3A, 0x40, 0x07, 0x5E, -1};
static char text_htrain4[] = {0xE4, 0x33, 0x0F, 0x27, 0x20, 0x15, 0x40, 0x13, 0x63, 0x8d, 0x8f, 0x74, -1};
static char text_htrain5[] = {0xD5, 0x4F, -1};

static char text_runninglongjump[] = {0x25, 0x27, 0x29, 0x2C, 0x3D, 0x27, 0x36, -1};
static char text_itrain1[] = {0xD6, 0x55, 0x4E, 0x60, 0x4C, 0x70, -1};
static char text_itrain2[] = {0x28, 0x08, 0x60, 0x5c, 0x51, 0x68, 0x70, 0x60, 0x64, -1};
static char text_itrain3[] = {0x25, 0x27, 0x29, 0x2c, 0x3d, 0x27, 0x36, 0x74, 0xd5, 0x4f, -1};

									  //"walking"
static char text_jtrain1[] = {0x44, 0xE5, 0x0C, 0x12, 0x3A, 0x40, 0x07, 0x7A, -1};
static char text_jtrain2[] = {0xD7, 0x6B, 0x5E, 0x74, 0xEA, 0x5F, 0x76, 0x6E, -1};
static char text_jtrain3[] = {0xCB, 0xCC, 0x57, 0x5D, -1};
static char text_jtrain4[] = {0xD6, 0x55, 0x4E, 0x68, 0x7A, -1};
static char text_jtrain5[] = {0x73, 0x5A, 0x71, -1};

static char text_targetpractice[] = {0x0B, 0x3E, 0x41, 0x12, 0x3A, 0x27, 0x29, -1};
static char text_ktrain1[] = {0x33, 0x02, 0x74, 0x5C, 0x51, 0x7F, 0x5D, -1};
static char text_ktrain2[] = {0x03, 0x16, 0x1C, 0x41, 0x74, 0x5A, 0x50, 0x57, -1};
static char text_ktrain3[] = {0xD7, 0xD8, 0x74, 0xD9, 0xDA, 0x56, 0x59, 0x72, -1};

							  //"well done"
							  //"you have completed"
static char text_ltrain1[] = {0x13, 0x24, 0x41, 0x15, 0x27, 0x29, 0x64, -1};
static char text_ltrain2[] = {0xA2, 0xA3, 0x7A, 0x58, -1};
static char text_ltrain3[] = {0x91, 0xD3, 0x09, 0x41, 0x0C, 0x60, -1};
static char text_ltrain4[] = {0x58, 0x58, 0x69, 0x60, 0x64, -1};
static char text_ltrain5[] = {0x26, 0x41, 0x36, 0x74, 0x66, 0x75, 0x7A, 0x53, 0x79, 0x56, 0x4D, -1};

static char text_deepdiving[] = {0x2E, 0x3A, 0x41, 0x36, 0x0C, 0x01, 0x1C, 0x27, 0x29, -1};
static char text_mtrain1[] = {0xDB, 0x4D, 0x7F, 0x52, 0x6F, -1};
static char text_mtrain2[] = {0xDC, 0x68, 0x7A, 0x6B, 0x78, 0x7F, 0x5D, -1};
static char text_mtrain3[] = {0xD6, 0x55, 0x4E, 0x76, 0x73, 0x67, 0x61, 0x54, 0x70, -1};
static char text_mtrain4[] = {0x13, 0x27, 0x16, 0x23, 0x74, 0x56, 0x76, 0x59, -1};

						 //"jumping"
static char text_ntrain1[] = {0x44, 0xE5, 0x0C, 0x12, 0x3A, 0x40, 0x07, 0x7A, -1};
static char text_ntrain2[] = {0xD7, 0x6B, 0x5E, 0x74, 0xEA, 0x5F, 0x76, 0x6E, -1};
static char text_ntrain3[] = {0x0F, 0x01, 0x1C, 0x27, 0x29, 0x74, 0x64, 0x51, 0x7F, 0x5D, -1};
static char text_ntrain5[] = {0x2C, 0x3D, 0x27, 0x36, 0x57, 0x72, -1};


static char text_mapmode[] = {0x1B, 0x40, 0x36, 0x1f, 0x41, 0x2f, -1};
static char text_otrain1[] = {0x1b, 0x40, 0x36, 0x63, 0x04, 0x27, 0xe3, 0x04, 0x19, 0x64, -1};
static char text_otrain2[] = {0xE7, 0x13, 0x22, 0x28, 0x41, 0x33, 0x0F, 0x27, -1};
static char text_otrain3[] = {0x1B, 0x40, 0x36, 0x74, 0x5C, 0x51, 0x7F, 0x5D, -1};
static char text_otrain5[] = {0xDD, 0xDE, 0x74, 0x61, 0x54, 0x72, -1};


static char text_diagonaljump[] = {0xDF, 0x6A, 0x2C, 0x3D, 0x27, 0x36, -1};
static char text_ptrain1[] = {0xDF, 0x6A, 0xC9, 0xCA, 0x74, 0x57, 0x5F, 0x76, 0x6E, -1};
static char text_ptrain3[] = {0xE9, 0x13, 0x22, 0x28, 0x41, 0x33, 0x0F, 0x27, 0x74, 0xA4, 0x58, 0x5E, -1};
static char text_ptrain5[] = {0xc3, 0xc5, 0x6f, 0x2c, 0x3D, 0x27, 0x36, 0x76, 0x7a, 0x52, 0x70, -1};
static char text_ptrain6[] = {0x2C, 0x3D, 0x27, 0x36, 0xD4, 0x60, 0x44, 0xE5, 0x0C, 0x12, 0x3A, 0x40, 0x07, 0x7a, -1};
static char text_ptrain7[] = {0xD6, 0x52, 0x74, 0xE0, 0x4F, 0x6E, 0x71, 0x68, 0x58, -1};


static char text_advanced[] = {0x33, 0x02, 0x74, 0x5C, 0x51, 0x7F, 0x5D, -1};
						//"target practice"
static char text_qtrain1[] = {0x03, 0x16, 0x1C, 0x41, 0x74, 0x5A, 0x50, 0x57, -1};
static char text_qtrain2[] = {0xd7, 0xd8, 0x74, 0xd9, 0xda, 0x56, 0x59, 0x72, -1};

static char text_activatingswitches[] = {0x0b, 0x41, 0x07, 0x24, 0x40, 0x13, 0xe3, 0x0c, 0x01, 0x40, 0x10, -1};
static char text_rtrain1[] = {0x8d, 0x63, 0x09, 0x41, 0x0c, 0x60, 0x4c, 0x70, 0x0c, 0x01, 0x40, 0x10, 0x74, -1};
static char text_rtrain2[] = {0x66, 0x69, 0x5e, 0x8f, 0x09, 0x41, 0x0c, 0x63, -1};
static char text_rtrain3[] = {0xe1, 0x63, 0x2a, 0x41, 0x13, 0x76, 0xe2, 0x53, -1};

						//"well done"
static char text_strain1[] = {0x55, 0x71, 0x7A, 0x58, 0x7E, 0x5D, 0x63, -1};
static char text_strain2[] = {0x13, 0x24, 0x41, 0x15, 0x27, 0x29, 0x64, -1};
static char text_strain3[] = {0xA2, 0xA3, 0x57, 0x68, 0x57, 0x5A, -1};

char	*Train1[] =
{
		text_traininglevel,
	text_blankline,
		text_atrain1,
		text_atrain2,
	text_blankline,
		text_atrain3,
		text_atrain4,
		0,
} ;
char	*Train2[] =
{
		text_jumping,
	text_blankline,
		text_btrain1,
		text_btrain2,
	text_blankline,
		text_btrain3,
		text_btrain4,
		0,
} ;
char	*Train3[] =
{
		text_swimming,
	text_blankline,
		text_ctrain1,
		text_ctrain2,
	text_blankline,
		text_ctrain4,
		text_ctrain5,
		0,
} ;
char	*Train4[] =
{
		text_climbing,
	text_blankline,
		text_dtrain1,
		text_dtrain2,
	text_blankline,
		text_dtrain3,
		text_dtrain4,
	text_blankline,
		text_dtrain5,
		0,
} ;
char	*Train5[] =
{
		text_walking,
	text_blankline,
		text_etrain1,
		text_etrain2,
	text_blankline,
		text_etrain3,
		text_etrain4,	// german	text_etrain5,
		0,
} ;
char	*Train6[] =
{
		text_longjump,
	text_blankline,
		text_ftrain1,
		text_ftrain2,
		text_ftrain3,
		0,
} ;
char	*Train7[] =
{
		text_welldone,
		text_youhavecompleted,
		text_gtrain1,
		text_gtrain2,
	text_blankline,
		text_gtrain3,
		text_gtrain4,
		text_gtrain5,
		0,
} ;

char	*Train8[] =
{
		text_swimming,
	text_blankline,
		text_htrain1,
		text_htrain2,
		text_htrain3,
		text_htrain4,
		text_htrain5,	// upto 6 in german
		0,
} ;
char	*Train9[] =
{
		text_runninglongjump,
	text_blankline,
		text_itrain1,
		text_itrain2,
		text_itrain3,
		0,
} ;
char	*Train10[] =
{
		text_walking,
	text_blankline,
		text_jtrain1,
		text_jtrain2,
		text_jtrain3,
		text_jtrain4,
		text_jtrain5,
		0,
} ;
char	*Train11[] =
{
		text_targetpractice,
	text_blankline,
		text_ktrain1,
		text_ktrain2,
		text_ktrain3,
		0,
} ;
char	*Train12[] =
{
		text_welldone,
		text_youhavecompleted,
		text_ltrain1,
		text_ltrain2,
	text_blankline,
		text_ltrain3,
		text_ltrain4,
		text_ltrain5,
		0,
} ;

char	*Train13[] =
{
		text_deepdiving,
	text_blankline,
		text_mtrain1,
		text_mtrain2,
		text_mtrain3,
		text_mtrain4,
		0,
} ;
char	*Train14[] =
{
		text_jumping,
	text_blankline,
		text_ntrain1,
		text_ntrain2,
		text_ntrain3,
		text_ntrain5,
		0,
} ;
char	*Train15[] =
{
		text_mapmode,
	text_blankline,
		text_otrain1,
		text_otrain2,
	text_blankline,
		text_otrain3,
		text_otrain5,
		0,
} ;
char	*Train16[] =
{
		text_diagonaljump,
	text_blankline,
		text_ptrain1,
		text_ptrain3,
		text_ptrain5,
	text_blankline,
		text_ptrain6,
		text_ptrain7,
		0,
} ;
char	*Train17[] =
{
		text_targetpractice,
	text_blankline,
		text_advanced,
		text_qtrain1,
		text_qtrain2,
		0,
} ;
char	*Train18[] =
{
		text_activatingswitches,
	text_blankline,
		text_rtrain1,
		text_rtrain2,
		text_rtrain3,
		0,
} ;
char	*Train19[] =
{
		text_welldone,
		text_strain1,
		text_strain2,
		text_strain3,
		0,
} ;

#endif

/////////////////////////////////////////////////////////////////////////////
// Functions
/////////////////////////////////////////////////////////////////////////////

void CTraining__Construct(CTraining *pThis, s32 id)
{
	if (pThis->m_Mode == TRAINING_NULL)
		pThis->m_Alpha = 0 ;

	pThis->m_Mode = TRAINING_FADEUP ;

	pThis->m_ID = id ;

//	old_enemy_speed_scaler = enemy_speed_scaler ;
//	old_sky_speed_scaler = sky_speed_scaler ;
//	old_particle_speed_scaler = particle_speed_scaler ;
//	sky_speed_scaler = 0.0 ;
//	enemy_speed_scaler = 0.0 ;
//	particle_speed_scaler = 0 ;
}


//------------------------------------------------------------------------
// Update selection on training screen
//------------------------------------------------------------------------
void CTraining__Update(CTraining *pThis)
{
	CTurokMovement.InMenuTimer = .3 ;
	
	// Exit if start pressed
	if (CTControl__IsUseMenu(pCTControl))
	{
		pThis->m_Mode = PAUSE_FADEDOWN ;
		GetApp()->m_bTraining = FALSE;
	}
}


//------------------------------------------------------------------------
// Draw training screen information
//------------------------------------------------------------------------

void CTraining__Draw(CTraining *pThis, Gfx **ppDLP)
{
	char	**ppText ;
	int		i, y ;

	// update fade status
	switch (pThis->m_Mode)
	{
		case TRAINING_FADEUP:
			pThis->m_Alpha += .2 * frame_increment ;
			if (pThis->m_Alpha >= 1.0)
			{
				pThis->m_Alpha = 1.0 ;
				pThis->m_Mode = TRAINING_NORMAL ;
			}
			break ;

		case TRAINING_FADEDOWN:
			pThis->m_Alpha -= .2 * frame_increment ;
			if (pThis->m_Alpha <= 0.0)
			{
				pThis->m_Alpha = 0.0 ;
				pThis->m_Mode = TRAINING_NULL ;
			}
			break ;
	}


	// prepare to draw boxes
	COnScreen__InitBoxDraw(ppDLP) ;
	COnScreen__DrawHilightBox(ppDLP,
							 TRAINING_X, TRAINING_Y,
							 TRAINING_X+TRAINING_WIDTH, TRAINING_Y+TRAINING_HEIGHT,
							 1, FALSE,
							 0,0,0, 150 * pThis->m_Alpha) ;


	// draw text option
	COnScreen__InitFontDraw(ppDLP) ;
#ifdef GERMAN
	COnScreen__SetFontScale(0.8, 1.0) ;
#else
	COnScreen__SetFontScale(1.0, 1.0) ;
#endif
	COnScreen__SetFontColor(ppDLP, 200*1.25, 200*1.25, 138*1.25, 86*1.25, 71*1.25,47*1.25) ;

	switch (pThis->m_ID)
	{
		case 10001:
				ppText = Train1 ;
				break ;
		case 10002:
				ppText = Train2 ;
				break ;
		case 10003:
				ppText = Train3 ;
				break ;
		case 10004:
				ppText = Train4 ;
				break ;
		case 10005:
				ppText = Train5 ;
				break ;
		case 10006:
				ppText = Train6 ;
				break ;
		case 10007:
				ppText = Train7 ;
				break ;
		case 10009:
				ppText = Train8 ;
				break ;
		case 10010:
				ppText = Train9 ;
				break ;
		case 10011:
				ppText = Train10 ;
				break ;
		case 10012:
				ppText = Train11 ;
				break ;
		case 10013:
				ppText = Train12 ;
				break ;
		case 10015:
				ppText = Train13 ;
				break ;
		case 10016:
				ppText = Train14 ;
				break ;
		case 10017:
				ppText = Train15 ;
				break ;
		case 10018:
				ppText = Train16 ;
				break ;
		case 10019:
				ppText = Train17 ;
				break ;
		case 10020:
				ppText = Train18 ;
				break ;
		case 10021:
				GetApp()->m_bTrainingMode = 2 ;
				ppText = Train19 ;
				break ;
	}

	i = 0 ;
	y = TRAINING_Y+16 ;

	if (ppText)
	{
		while (ppText[i] != 0)
		{
			if (ppText[i] == text_blankline)
			{
				y += 8 ;
			}
			else
			{
				COnScreen__DrawText(ppDLP, ppText[i], 320/2, y, (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				y+=18 ;
			}

			i++;
		}
	}


/*


		case 10001:
				COnScreen__DrawText(ppDLP,
										  text_traininglevel,
										  320/2, TRAINING_Y+16+(18*0),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);

				COnScreen__DrawText(ppDLP,
										  text_atrain1,
										  320/2, TRAINING_Y+24+(18*1),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_atrain2,
										  320/2, TRAINING_Y+24+(18*2),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_atrain3,
										  320/2, TRAINING_Y+32+(18*3),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_atrain4,
										  320/2, TRAINING_Y+32+(18*4),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
			break ;

		case 10002:
				COnScreen__DrawText(ppDLP,
										  text_jumping,
										  320/2, TRAINING_Y+16+(18*0),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);

				COnScreen__DrawText(ppDLP,
										  text_btrain1,
										  320/2, TRAINING_Y+24+(18*1),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_btrain2,
										  320/2, TRAINING_Y+24+(18*2),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);

				COnScreen__DrawText(ppDLP,
										  text_btrain3,
										  320/2, TRAINING_Y+32+(18*3),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_btrain4,
										  320/2, TRAINING_Y+32+(18*4),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
#ifndef KANJI
				COnScreen__DrawText(ppDLP,
										  text_btrain5,
										  320/2, TRAINING_Y+32+(18*5),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
#endif
			break ;

		case 10003:
				COnScreen__DrawText(ppDLP,
										  text_swimming,
										  320/2, TRAINING_Y+16+(18*0),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);

				COnScreen__DrawText(ppDLP,
										  text_ctrain1,
										  320/2, TRAINING_Y+24+(18*1),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_ctrain2,
										  320/2, TRAINING_Y+24+(18*2),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);

#ifndef KANJI
				COnScreen__DrawText(ppDLP,
										  text_ctrain3,
										  320/2, TRAINING_Y+32+(18*3),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
#endif
				COnScreen__DrawText(ppDLP,
										  text_ctrain4,
										  320/2, TRAINING_Y+32+(18*4),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_ctrain5,
										  320/2, TRAINING_Y+32+(18*5),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
			break ;

		case 10004:
				COnScreen__DrawText(ppDLP,
										  text_climbing,
										  320/2, TRAINING_Y+16+(18*0),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);

				COnScreen__DrawText(ppDLP,
										  text_dtrain1,
										  320/2, TRAINING_Y+24+(18*1),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_dtrain2,
										  320/2, TRAINING_Y+24+(18*2),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);

				COnScreen__DrawText(ppDLP,
										  text_dtrain3,
										  320/2, TRAINING_Y+32+(18*3),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_dtrain4,
										  320/2, TRAINING_Y+32+(18*4),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_dtrain5,
										  320/2, TRAINING_Y+32+(18*5),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
			break ;

		case 10005:
				COnScreen__DrawText(ppDLP,
										  text_walking,
										  320/2, TRAINING_Y+16+(18*0),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);

				COnScreen__DrawText(ppDLP,
										  text_etrain1,
										  320/2, TRAINING_Y+24+(18*1),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_etrain2,
										  320/2, TRAINING_Y+24+(18*2),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);

				COnScreen__DrawText(ppDLP,
										  text_etrain3,
										  320/2, TRAINING_Y+32+(18*3),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_etrain4,
										  320/2, TRAINING_Y+32+(18*4),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
#ifdef GERMAN
				COnScreen__DrawText(ppDLP,
										  text_etrain5,
										  320/2, TRAINING_Y+32+(18*5),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
#endif
			break ;

		case 10006:
				COnScreen__DrawText(ppDLP,
										  text_longjump,
										  320/2, TRAINING_Y+16+(18*0),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);

				COnScreen__DrawText(ppDLP,
										  text_ftrain1,
										  320/2, TRAINING_Y+24+(18*1),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_ftrain2,
										  320/2, TRAINING_Y+24+(18*2),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_ftrain3,
										  320/2, TRAINING_Y+24+(18*3),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
			break ;

		case 10007:
				COnScreen__DrawText(ppDLP,
										  text_welldone,
										  320/2, TRAINING_Y+16+(18*0),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_youhavecompleted,
										  320/2, TRAINING_Y+16+(18*1),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_gtrain1,
										  320/2, TRAINING_Y+16+(18*2),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_gtrain2,
										  320/2, TRAINING_Y+16+(18*3),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_gtrain3,
										  320/2, TRAINING_Y+24+(18*4),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_gtrain4,
										  320/2, TRAINING_Y+24+(18*5),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_gtrain5,
										  320/2, TRAINING_Y+24+(18*6),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
			break ;

		// skip 10008

		case 10009:
				COnScreen__DrawText(ppDLP,
										  text_swimming,
										  320/2, TRAINING_Y+16+(18*0),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);

				COnScreen__DrawText(ppDLP,
										  text_htrain1,
										  320/2, TRAINING_Y+24+(18*1),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_htrain2,
										  320/2, TRAINING_Y+24+(18*2),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_htrain3,
										  320/2, TRAINING_Y+24+(18*3),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_htrain4,
										  320/2, TRAINING_Y+24+(18*4),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_htrain5,
										  320/2, TRAINING_Y+24+(18*5),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
#ifdef GERMAN
				COnScreen__DrawText(ppDLP,
										  text_htrain6,
										  320/2, TRAINING_Y+24+(18*6),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
#endif
			break ;

		case 10010:
				COnScreen__DrawText(ppDLP,
										  text_runninglongjump,
										  320/2, TRAINING_Y+16+(18*0),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);

				COnScreen__DrawText(ppDLP,
										  text_itrain1,
										  320/2, TRAINING_Y+24+(18*1),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_itrain2,
										  320/2, TRAINING_Y+24+(18*2),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_itrain3,
										  320/2, TRAINING_Y+24+(18*3),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
			break ;

		case 10011:
				COnScreen__DrawText(ppDLP,
										  text_walking,
										  320/2, TRAINING_Y+16+(18*0),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);

				COnScreen__DrawText(ppDLP,
										  text_jtrain1,
										  320/2, TRAINING_Y+24+(18*1),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_jtrain2,
										  320/2, TRAINING_Y+24+(18*2),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_jtrain3,
										  320/2, TRAINING_Y+24+(18*3),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_jtrain4,
										  320/2, TRAINING_Y+24+(18*4),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_jtrain5,
										  320/2, TRAINING_Y+24+(18*5),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
			break ;

		case 10012:
				COnScreen__DrawText(ppDLP,
										  text_targetpractice,
										  320/2, TRAINING_Y+16+(18*0),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_ktrain1,
										  320/2, TRAINING_Y+24+(18*1),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_ktrain2,
										  320/2, TRAINING_Y+24+(18*2),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_ktrain3,
										  320/2, TRAINING_Y+24+(18*3),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
#ifndef KANJI
				COnScreen__DrawText(ppDLP,
										  text_ktrain4,
										  320/2, TRAINING_Y+24+(18*4),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
#endif
			break ;

		case 10013:
				COnScreen__DrawText(ppDLP,
										  text_welldone,
										  320/2, TRAINING_Y+16+(18*0),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_youhavecompleted,
										  320/2, TRAINING_Y+16+(18*1),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_ltrain1,
										  320/2, TRAINING_Y+16+(18*2),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_ltrain2,
										  320/2, TRAINING_Y+16+(18*3),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_ltrain3,
										  320/2, TRAINING_Y+24+(18*4),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_ltrain4,
										  320/2, TRAINING_Y+24+(18*5),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_ltrain5,
										  320/2, TRAINING_Y+24+(18*6),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
			break ;

			// skip 10014

		case 10015:
				COnScreen__DrawText(ppDLP,
										  text_deepdiving,
										  320/2, TRAINING_Y+16+(18*0),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);

				COnScreen__DrawText(ppDLP,
										  text_mtrain1,
										  320/2, TRAINING_Y+24+(18*1),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_mtrain2,
										  320/2, TRAINING_Y+24+(18*2),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_mtrain3,
										  320/2, TRAINING_Y+24+(18*3),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_mtrain4,
										  320/2, TRAINING_Y+24+(18*4),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
			break ;

		case 10016:
				COnScreen__DrawText(ppDLP,
										  text_jumping,
										  320/2, TRAINING_Y+16+(18*0),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);

				COnScreen__DrawText(ppDLP,
										  text_ntrain1,
										  320/2, TRAINING_Y+24+(18*1),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_ntrain2,
										  320/2, TRAINING_Y+24+(18*2),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_ntrain3,
										  320/2, TRAINING_Y+24+(18*3),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
#ifndef KANJI
				COnScreen__DrawText(ppDLP,
										  text_ntrain4,
										  320/2, TRAINING_Y+24+(18*4),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
#endif
				COnScreen__DrawText(ppDLP,
										  text_ntrain5,
										  320/2, TRAINING_Y+24+(18*5),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
			break ;


		case 10017:
				COnScreen__DrawText(ppDLP,
										  text_mapmode,
										  320/2, TRAINING_Y+16+(18*0),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);

				COnScreen__DrawText(ppDLP,
										  text_otrain1,
										  320/2, TRAINING_Y+24+(18*1),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_otrain2,
										  320/2, TRAINING_Y+24+(18*2),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_otrain3,
										  320/2, TRAINING_Y+24+(18*3),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
#ifndef KANJI
				COnScreen__DrawText(ppDLP,
										  text_otrain4,
										  320/2, TRAINING_Y+24+(18*4),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
#endif
				COnScreen__DrawText(ppDLP,
										  text_otrain5,
										  320/2, TRAINING_Y+24+(18*5),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
			break ;

		case 10018:
				COnScreen__DrawText(ppDLP,
										  text_diagonaljump,
										  320/2, TRAINING_Y+16+(18*0),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_ptrain1,
										  320/2, TRAINING_Y+24+(18*1),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
#ifndef KANJI
				COnScreen__DrawText(ppDLP,
										  text_ptrain2,
										  320/2, TRAINING_Y+24+(18*2),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
#endif
				COnScreen__DrawText(ppDLP,
										  text_ptrain3,
										  320/2, TRAINING_Y+24+(18*3),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
#ifndef KANJI
				COnScreen__DrawText(ppDLP,
										  text_ptrain4,
										  320/2, TRAINING_Y+24+(18*4),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
#endif
				COnScreen__DrawText(ppDLP,
										  text_ptrain5,
										  320/2, TRAINING_Y+24+(18*5),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_ptrain6,
										  320/2, TRAINING_Y+24+(18*6),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_ptrain7,
										  320/2, TRAINING_Y+24+(18*7),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
			break ;


		case 10019:
				COnScreen__DrawText(ppDLP,
										  text_advanced,
										  320/2, TRAINING_Y+16+(18*0),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_targetpractice,
										  320/2, TRAINING_Y+16+(18*1),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_qtrain1,
										  320/2, TRAINING_Y+24+(18*2),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_qtrain2,
										  320/2, TRAINING_Y+24+(18*3),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
			break ;

		case 10020:
				COnScreen__DrawText(ppDLP,
										  text_activatingswitches,
										  320/2, TRAINING_Y+16+(18*0),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_rtrain1,
										  320/2, TRAINING_Y+24+(18*1),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_rtrain2,
										  320/2, TRAINING_Y+24+(18*2),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_rtrain3,
										  320/2, TRAINING_Y+24+(18*3),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
			break ;


		case 10021:
				GetApp()->m_bTrainingMode = 2 ;
				COnScreen__DrawText(ppDLP,
										  text_welldone,
										  320/2, TRAINING_Y+16+(18*0),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_strain1,
										  320/2, TRAINING_Y+24+(18*1),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_strain2,
										  320/2, TRAINING_Y+24+(18*2),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				COnScreen__DrawText(ppDLP,
										  text_strain3,
										  320/2, TRAINING_Y+32+(18*3),
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE);
				break ;
	}
*/

}


