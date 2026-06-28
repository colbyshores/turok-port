// FrontEnd.Cpp

#include "cppu64.h"
#include "frontend.h"
#include "tengine.h"
#include "audio.h"
#include "sfx.h"
#include "dlists.h"


/*****************************************************************************
* Text
*****************************************************************************/
#ifdef ENGLISH
static char	text_nocontrollerfound[] = {"there are no"};
static char	text_pleasereboot[] = {"controllers attached"};
static char	text_nintendo[] = {"to the nintendo 64"};
#endif

#ifdef GERMAN
static char	text_nocontrollerfound[] = {"controller nicht"};
static char	text_pleasereboot[] = {"gefunden, bitte"};
static char	text_nintendo[] = {"neustarten"};
#endif

#ifdef KANJI
static char	text_nocontrollerfound[] = {0x09, 0x27, 0x13, 0x25, 0x41, 0x21, 0x76, 0x56, 0x57, 0x55, 0x68, 0x71, 0x5d, 0x4d, 0x68, 0x59, 0x75, -1};
static char	text_pleasereboot[] = {0x22, 0x0d, 0x40, 0x13, 0x57, 0x5d, 0x53, 0x79, 0x56, 0x4d, -1};
#endif


/*****************************************************************************
*
*	Functions:		Legal Screen stuff
*
*****************************************************************************/
CLegalScreen		LegalScreen ;


/*****************************************************************************
*
*	Function:		CLegalScreen__Contruct()
*
******************************************************************************
*
*	Description:	Create the legal screen initial vars
*
*	Inputs:			none
*	Outputs:			none
*
*****************************************************************************/
void CLegalScreen__Construct(void)
{
	LegalScreen.m_Time = SECONDS_TO_FRAMES(3) ;
	LegalScreen.m_Mode = 0 ;

	GetApp()->m_WarpID = LEGALSCREEN_WARP_ID ;
}

/*****************************************************************************
*
*	Function:		CLegalScreen__Update()
*
******************************************************************************
*
*	Description:	Update the legal screen
*
*	Inputs:			none
*	Outputs:			none
*
*****************************************************************************/
void CLegalScreen__Update(void)
{
	/* PORT note: this used to early-return under PLATFORM_PORT to FREEZE the legal screen, because the old
	 * unpaced frame-pump drained m_Time in ~1 frame and cascaded the whole intro (legal->logo->iggy->...->
	 * attract) into a cartridge-reload spin. The §8 logic-tick decouple now paces the timer correctly (one
	 * frame_increment per 30Hz tick), so the intro runs at its intended rate → the freeze is removed and the
	 * normal N64 attract flow (legal → intro logos → title → idle demo) is restored. A dev warp (TUROK_WARP /
	 * turok.cfg `warp`) still skips straight to a level for testing. */
	if (!validcontrollers)
		return ;

	if (LegalScreen.m_Time)
	{
		if (GetApp()->m_bPause == FALSE)
		{
	 		LegalScreen.m_Time -= frame_increment ;
			if (LegalScreen.m_Time <=0)
			{
				LegalScreen.m_Time = 0 ;
				GetApp()->m_WarpID = INTRO_ACCLAIM_LOGO_WARP_ID ;
				CEngineApp__SetupFadeTo(GetApp(), MODE_RESETLEVEL) ;

				GetApp()->m_LegalBypass = TRUE;
			}
		}
	}
}


/*****************************************************************************
*
*	Function:		CLegalScreen__Draw()
*
******************************************************************************
*
*	Description:	Draw the legal screen
*
*	Inputs:			GFX	**ppDLP
*	Outputs:			none
*
*****************************************************************************/
#define	YOFF		9
void CLegalScreen__Draw(Gfx **ppDLP)
{
	int	y ;

	COnScreen__InitFontDraw(ppDLP) ;


	//COnScreen__SetFontScale(0.6, 0.5) ;
	//COnScreen__SetFontColor(ppDLP, 50, 255, 50, 255, 255, 50) ;
	COnScreen__SetFontScale(1.0, 1.0) ;
	COnScreen__SetFontColor(ppDLP, 150, 150, 150, 150, 150, 150) ;


	if (!validcontrollers)
	{
		COnScreen__SetFontScale(1.0, 2.0) ;
		COnScreen__SetFontColor(ppDLP, 50, 250, 50, 250, 250, 50) ;

#ifndef KANJI
		y = 100 ;
#else
		y = 120 ;
#endif
		COnScreen__DrawText(ppDLP, text_nocontrollerfound, 320/2, y, 255, TRUE, FALSE) ;
		y += 40 ;
		COnScreen__DrawText(ppDLP, text_pleasereboot, 320/2, y, 255, TRUE, FALSE) ;
#ifndef KANJI
		y += 40 ;
		COnScreen__DrawText(ppDLP, text_nintendo, 320/2, y, 255, TRUE, FALSE) ;
#endif


	}
	else
	{
		COnScreen__PrepareSmallFont(ppDLP) ;

		y = 92 ;
	//	COnScreen__DrawText(ppDLP, "turok dinosaur hunter", 320/2, y, 255, TRUE, TRUE) ;
	//	y += YOFF ;
		COnScreen__DrawText(ppDLP, "{ 1997 acclaim entertainment,inc.", 320/2, y, 255, TRUE, FALSE) ;
		y += YOFF ;
		COnScreen__DrawText(ppDLP, "all rights reserved.", 320/2, y, 255, TRUE, FALSE) ;
		y += YOFF ;
	//	COnScreen__DrawText(ppDLP, "(turok) # @ 1997, gbpc, a subsidiary of", 320/2, y, 255, TRUE, FALSE) ;
		COnScreen__DrawText(ppDLP, "turok: }&{ 1997,gbpc,a subsidiary", 320/2, y, 255, TRUE, FALSE) ;
		y += YOFF ;
		COnScreen__DrawText(ppDLP, "of golden books family", 320/2, y, 255, TRUE, FALSE) ;
		y += YOFF ;
		COnScreen__DrawText(ppDLP, "entertainment. all rights reserved.", 320/2, y, 255, TRUE, FALSE) ;
		y += YOFF ;
		COnScreen__DrawText(ppDLP, "all other characters herein and the", 320/2, y, 255, TRUE, FALSE) ;
		y += YOFF ;
		COnScreen__DrawText(ppDLP, "distinct likenesses thereof are", 320/2, y, 255, TRUE, FALSE) ;
		y += YOFF ;
		COnScreen__DrawText(ppDLP, "trademarks of acclaim comics,inc.", 320/2, y, 255, TRUE, FALSE) ;
		y += YOFF ;
		COnScreen__DrawText(ppDLP, "all rights reserved.", 320/2, y, 255, TRUE, FALSE) ;
		y += YOFF ;
		COnScreen__DrawText(ppDLP, "acclaim is a division of", 320/2, y, 255, TRUE, FALSE) ;
		y += YOFF ;
		COnScreen__DrawText(ppDLP, "acclaim entertainment,inc.", 320/2, y, 255, TRUE, FALSE) ;
		y += YOFF ;
	//	COnScreen__DrawText(ppDLP, "# @ 1997 acclaim entertainment, inc.", 320/2, y, 255, TRUE, FALSE) ;
		COnScreen__DrawText(ppDLP, "}&{ 1997 acclaim entertainment,inc.", 320/2, y, 255, TRUE, FALSE) ;
		y += YOFF ;
		COnScreen__DrawText(ppDLP, "all rights reserved.", 320/2, y, 255, TRUE, FALSE) ;

		y += YOFF+6 ;
		COnScreen__DrawText(ppDLP, "licensed by nintendo", 320/2, y, 255, TRUE, FALSE) ;
	}

}






/*****************************************************************************
*
*	Functions:		Credits stuff
*
*****************************************************************************/
CCredits		Credits ;

#define CREDIT_SIZE	21
#define CREDIT_XS		0.75
#define CREDIT_YS		0.75
#define CREDIT_TXS	0.85
#define CREDIT_TYS	0.85
#define CREDIT_HXS	1.2
#define CREDIT_HYS	1.2

t_Credit TheCredits[] =
{
	// start of iguana team turok credits
	{CREDIT_HXS, CREDIT_HYS, ""},
	{CREDIT_HXS, CREDIT_HYS, "turok team"},
	{CREDIT_HXS, CREDIT_HYS, ""},

	{CREDIT_TXS, CREDIT_TYS, "producer"},
	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_XS, CREDIT_YS, "david dienstbier"},
	{CREDIT_XS, CREDIT_YS, ""},

	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_TXS, CREDIT_TYS, "executive producers"},
	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_XS, CREDIT_YS, "jeff spangenberg"},
	{CREDIT_XS, CREDIT_YS, "darrin stubbington"},
	{CREDIT_XS, CREDIT_YS, ""},

	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_TXS, CREDIT_TYS, "lead programmer"},
	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_XS, CREDIT_YS, "rob cohen"},
	{CREDIT_XS, CREDIT_YS, ""},

	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_TXS, CREDIT_TYS, "lead artist"},
	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_XS, CREDIT_YS, "alan d. johnson"},
	{CREDIT_XS, CREDIT_YS, ""},

	// design credits
	{CREDIT_HXS, CREDIT_HYS, ""},
	{CREDIT_HXS, CREDIT_HYS, ""},
	{CREDIT_HXS, CREDIT_HYS, "design"},
	{CREDIT_HXS, CREDIT_HYS, ""},

	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_TXS, CREDIT_TYS, "game design"},
	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_XS, CREDIT_YS, "david dienstbier"},
	{CREDIT_XS, CREDIT_YS, ""},

	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_TXS, CREDIT_TYS, "additional design"},
	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_XS, CREDIT_YS, "nigel cook"},
	{CREDIT_XS, CREDIT_YS, "jason carpenter"},
	{CREDIT_XS, CREDIT_YS, ""},

	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_TXS, CREDIT_TYS, "level design"},
	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_XS, CREDIT_YS, "david dienstbier"},
	{CREDIT_XS, CREDIT_YS, "nigel cook"},
	{CREDIT_XS, CREDIT_YS, "jason carpenter"},
	{CREDIT_XS, CREDIT_YS, "alan d. johnson"},
	{CREDIT_XS, CREDIT_YS, "thomas coles"},
	{CREDIT_XS, CREDIT_YS, ""},

	// programming credits
	{CREDIT_HXS, CREDIT_HYS, ""},
	{CREDIT_HXS, CREDIT_HYS, ""},
	{CREDIT_HXS, CREDIT_HYS, "programming"},
	{CREDIT_HXS, CREDIT_HYS, ""},

	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_TXS, CREDIT_TYS, "game logic"},
	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_XS, CREDIT_YS, "ian dunlop"},
	{CREDIT_XS, CREDIT_YS, "carl wade"},
	{CREDIT_XS, CREDIT_YS, ""},

	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_TXS, CREDIT_TYS, "boss ai"},
	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_XS, CREDIT_YS, "stephen broumley"},
	{CREDIT_XS, CREDIT_YS, ""},

	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_TXS, CREDIT_TYS, "front end"},
	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_XS, CREDIT_YS, "carl wade"},
	{CREDIT_XS, CREDIT_YS, "stephen broumley"},
	{CREDIT_XS, CREDIT_YS, ""},

	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_TXS, CREDIT_TYS, "3d graphics engine"},
	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_XS, CREDIT_YS, "rob cohen"},
	{CREDIT_XS, CREDIT_YS, ""},

	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_TXS, CREDIT_TYS, "sound engine"},
	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_XS, CREDIT_YS, "anthony palomba"},
	{CREDIT_XS, CREDIT_YS, "brian watson"},
	{CREDIT_XS, CREDIT_YS, ""},

	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_TXS, CREDIT_TYS, "cinemas"},
	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_XS, CREDIT_YS, "stephen broumley"},
	{CREDIT_XS, CREDIT_YS, "carl wade"},
	{CREDIT_XS, CREDIT_YS, ""},

	// art credits
	{CREDIT_HXS, CREDIT_HYS, ""},
	{CREDIT_HXS, CREDIT_HYS, ""},
	{CREDIT_HXS, CREDIT_HYS, "art"},
	{CREDIT_HXS, CREDIT_HYS, ""},

	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_TXS, CREDIT_TYS, "texture maps"},
	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_XS, CREDIT_YS, "robbie miller"},
	{CREDIT_XS, CREDIT_YS, "tre zieman"},
	{CREDIT_XS, CREDIT_YS, "ryan tracy"},
	{CREDIT_XS, CREDIT_YS, "greg omelchuck"},
	{CREDIT_XS, CREDIT_YS, ""},

	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_TXS, CREDIT_TYS, "3d modeling"},
	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_XS, CREDIT_YS, "ryan tracy"},
	{CREDIT_XS, CREDIT_YS, "greg omelchuck"},
	{CREDIT_XS, CREDIT_YS, "tre zieman"},
	{CREDIT_XS, CREDIT_YS, "alan d. johnson"},
	{CREDIT_XS, CREDIT_YS, "mike janke"},
	{CREDIT_XS, CREDIT_YS, ""},

	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_TXS, CREDIT_TYS, "3d animators"},
	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_XS, CREDIT_YS, "shane tarrant"},
	{CREDIT_XS, CREDIT_YS, "greg omelchuck"},
	{CREDIT_XS, CREDIT_YS, "alan d. johnson"},
	{CREDIT_XS, CREDIT_YS, "mike janke"},
	{CREDIT_XS, CREDIT_YS, "josh prikryl"},
	{CREDIT_XS, CREDIT_YS, ""},

	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_TXS, CREDIT_TYS, "particle effects"},
	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_XS, CREDIT_YS, "thomas coles"},
	{CREDIT_XS, CREDIT_YS, ""},

	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_TXS, CREDIT_TYS, "motion capture editing"},
	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_XS, CREDIT_YS, "thomas coles"},
	{CREDIT_XS, CREDIT_YS, "marc mackin"},
	{CREDIT_XS, CREDIT_YS, ""},

	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_TXS, CREDIT_TYS, "additional art"},
	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_XS, CREDIT_YS, "peyton duncan"},
	{CREDIT_XS, CREDIT_YS, "marc mackin"},
	{CREDIT_XS, CREDIT_YS, "matt stubbington"},
	{CREDIT_XS, CREDIT_YS, ""},

	// audio credits
	{CREDIT_HXS, CREDIT_HYS, ""},
	{CREDIT_HXS, CREDIT_HYS, ""},
	{CREDIT_HXS, CREDIT_HYS, "audio"},
	{CREDIT_HXS, CREDIT_HYS, ""},

	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_TXS, CREDIT_TYS, "music"},
	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_XS, CREDIT_YS, "darren mitchell"},
	{CREDIT_XS, CREDIT_YS, ""},

	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_TXS, CREDIT_TYS, "sound design"},
	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_XS, CREDIT_YS, "marc schaefgen"},
	{CREDIT_XS, CREDIT_YS, ""},

	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_TXS, CREDIT_TYS, "voice talent"},
	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_XS, CREDIT_YS, "pete suarez"},
	{CREDIT_XS, CREDIT_YS, "david dienstbier"},
	{CREDIT_XS, CREDIT_YS, ""},
	{CREDIT_XS, CREDIT_YS, "and"},
	{CREDIT_XS, CREDIT_YS, ""},
	{CREDIT_XS, CREDIT_YS, "dean seltzer as turok"},
	{CREDIT_XS, CREDIT_YS, ""},

#ifdef GERMAN
	// translations
	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_TXS, CREDIT_TYS, "translation"},
	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_XS, CREDIT_YS, "reze memari"},
	{CREDIT_XS, CREDIT_YS, ""},
#endif
#ifdef KANJI
	// translations
	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_TXS, CREDIT_TYS, "translation"},
	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_XS, CREDIT_YS, "bob timbello"},
	{CREDIT_XS, CREDIT_YS, "konno ayaki"},
	{CREDIT_XS, CREDIT_YS, ""},
#endif


	// qa credits
	{CREDIT_HXS, CREDIT_HYS, ""},
	{CREDIT_HXS, CREDIT_HYS, ""},
	{CREDIT_HXS, CREDIT_HYS, "quality assurance"},
	{CREDIT_HXS, CREDIT_HYS, ""},
	{CREDIT_XS, CREDIT_YS, "bey bickerton"},
	{CREDIT_XS, CREDIT_YS, "jim richardson"},
	{CREDIT_XS, CREDIT_YS, ""},


	// iguana company credits
	{CREDIT_HXS, CREDIT_HYS, ""},
	{CREDIT_HXS, CREDIT_HYS, ""},
	{CREDIT_HXS, CREDIT_HYS, ""},
	{CREDIT_HXS, CREDIT_HYS, ""},
	{CREDIT_HXS, CREDIT_HYS, "iguana entertainment"},
	{CREDIT_HXS, CREDIT_HYS, ""},

	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_TXS, CREDIT_TYS, "president"},
	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_XS, CREDIT_YS, "jeff spangenberg"},
	{CREDIT_XS, CREDIT_YS, ""},

	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_TXS, CREDIT_TYS, "vice president"},
	{CREDIT_TXS, CREDIT_TYS, "of"},
	{CREDIT_TXS, CREDIT_TYS, "product development"},
	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_XS, CREDIT_YS, "darrin stubbington"},
	{CREDIT_XS, CREDIT_YS, ""},

	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_TXS, CREDIT_TYS, "technical director"},
	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_XS, CREDIT_YS, "craig galley"},
	{CREDIT_XS, CREDIT_YS, ""},

	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_TXS, CREDIT_TYS, "creative director"},
	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_XS, CREDIT_YS, "nigel cook"},
	{CREDIT_XS, CREDIT_YS, ""},

	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_TXS, CREDIT_TYS, "atg director"},
	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_XS, CREDIT_YS, "cyrus lum"},
	{CREDIT_XS, CREDIT_YS, ""},

	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_TXS, CREDIT_TYS, "art director"},
	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_XS, CREDIT_YS, "matt stubbington"},
	{CREDIT_XS, CREDIT_YS, ""},

	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_TXS, CREDIT_TYS, "music director"},
	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_XS, CREDIT_YS, "rick fox"},
	{CREDIT_XS, CREDIT_YS, ""},

	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_TXS, CREDIT_TYS, "product development"},
	{CREDIT_TXS, CREDIT_TYS, "coordinator"},
	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_XS, CREDIT_YS, "j moon"},
	{CREDIT_XS, CREDIT_YS, ""},

	// acclaim company credits
	{CREDIT_HXS, CREDIT_HYS, ""},
	{CREDIT_HXS, CREDIT_HYS, ""},
	{CREDIT_HXS, CREDIT_HYS, ""},
	{CREDIT_HXS, CREDIT_HYS, ""},
	{CREDIT_HXS, CREDIT_HYS, "acclaim"},
	{CREDIT_HXS, CREDIT_HYS, ""},

	// acclaim product development credits
	{CREDIT_HXS, CREDIT_HYS, ""},
	{CREDIT_HXS, CREDIT_HYS, ""},
	{CREDIT_HXS, CREDIT_HYS, "acclaim pd"},
	{CREDIT_HXS, CREDIT_HYS, ""},

	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_TXS, CREDIT_TYS, "executive"},
	{CREDIT_TXS, CREDIT_TYS, "producer"},
	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_XS, CREDIT_YS, "amy smith boylan"},
	{CREDIT_XS, CREDIT_YS, ""},

	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_TXS, CREDIT_TYS, "producer"},
	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_XS, CREDIT_YS, "steve bremer"},
	{CREDIT_XS, CREDIT_YS, ""},

	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_TXS, CREDIT_TYS, "associate producers"},
	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_XS, CREDIT_YS, "martin de riso"},
	{CREDIT_XS, CREDIT_YS, "eric weiner"},
	{CREDIT_XS, CREDIT_YS, ""},

	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_TXS, CREDIT_TYS, "product manager"},
	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_XS, CREDIT_YS, "don jackson"},
	{CREDIT_XS, CREDIT_YS, ""},

	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_TXS, CREDIT_TYS, "marketing coordinator"},
	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_XS, CREDIT_YS, "john paul carnovale"},
	{CREDIT_XS, CREDIT_YS, ""},

	// acclaim quality assurance credits
	{CREDIT_HXS, CREDIT_HYS, ""},
	{CREDIT_HXS, CREDIT_HYS, ""},
	{CREDIT_HXS, CREDIT_HYS, "acclaim qa"},
	{CREDIT_HXS, CREDIT_HYS, ""},

	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_TXS, CREDIT_TYS, "director"},
	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_XS, CREDIT_YS, "ray boylan"},
	{CREDIT_XS, CREDIT_YS, ""},

	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_TXS, CREDIT_TYS, "supervisors"},
	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_XS, CREDIT_YS, "carol caracciolo"},
	{CREDIT_XS, CREDIT_YS, "tom falzone"},
	{CREDIT_XS, CREDIT_YS, "harry reimer"},
	{CREDIT_XS, CREDIT_YS, ""},

	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_TXS, CREDIT_TYS, "analysts"},
	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_XS, CREDIT_YS, "jeff rosa"},
	{CREDIT_XS, CREDIT_YS, "eric hendrickson"},
	{CREDIT_XS, CREDIT_YS, "mat kraemer"},
	{CREDIT_XS, CREDIT_YS, "john cooper"},
	{CREDIT_XS, CREDIT_YS, "j.j. mazziotto"},
	{CREDIT_XS, CREDIT_YS, "dale taylor"},
	{CREDIT_XS, CREDIT_YS, ""},

	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_TXS, CREDIT_TYS, "testers"},
	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_XS, CREDIT_YS, "alex dela teja"},
	{CREDIT_XS, CREDIT_YS, "rex dickson"},
	{CREDIT_XS, CREDIT_YS, "paul johnson"},
	{CREDIT_XS, CREDIT_YS, "wing shiu"},
	{CREDIT_XS, CREDIT_YS, "paul di carlo"},
	{CREDIT_XS, CREDIT_YS, "joe howell"},
	{CREDIT_XS, CREDIT_YS, "stacy brickel"},
	{CREDIT_XS, CREDIT_YS, "jen walka"},
	{CREDIT_XS, CREDIT_YS, "dave walowitz"},
	{CREDIT_XS, CREDIT_YS, "chris hoffman"},
	{CREDIT_XS, CREDIT_YS, ""},

	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_TXS, CREDIT_TYS, "tsg"},
	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_XS, CREDIT_YS, "john gonzales"},
	{CREDIT_XS, CREDIT_YS, "howard perlman"},
	{CREDIT_XS, CREDIT_YS, "greg m^govern"},					// ^ means the small c in Mc
	{CREDIT_XS, CREDIT_YS, "dan wimpleberg"},
	{CREDIT_XS, CREDIT_YS, "keith robinson"},
	{CREDIT_XS, CREDIT_YS, "andrew fullaytor"},
	{CREDIT_XS, CREDIT_YS, ""},

	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_TXS, CREDIT_TYS, "stunt coordinator"},
	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_XS, CREDIT_YS, "jeffrey lee gibson"},
	{CREDIT_XS, CREDIT_YS, ""},

	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_TXS, CREDIT_TYS, "motion capture talent"},
	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_XS, CREDIT_YS, "brad martin"},
	{CREDIT_XS, CREDIT_YS, "brian smyj"},
	{CREDIT_XS, CREDIT_YS, ""},

	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_TXS, CREDIT_TYS, "manual"},
	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_XS, CREDIT_YS, "bill dickson"},
	{CREDIT_XS, CREDIT_YS, ""},


	// acclaim comics credits
	{CREDIT_HXS, CREDIT_HYS, ""},
	{CREDIT_HXS, CREDIT_HYS, ""},
	{CREDIT_HXS, CREDIT_HYS, "acclaim comics"},
	{CREDIT_HXS, CREDIT_HYS, ""},

	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_TXS, CREDIT_TYS, "story development"},
	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_XS, CREDIT_YS, "jeff gomez"},
	{CREDIT_XS, CREDIT_YS, "bob layton"},
	{CREDIT_XS, CREDIT_YS, ""},

	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_TXS, CREDIT_TYS, "character design"},
	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_XS, CREDIT_YS, "don perlin"},
	{CREDIT_XS, CREDIT_YS, ""},

	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_TXS, CREDIT_TYS, "campaigner character creator"},
	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_XS, CREDIT_YS, "tim truman"},
	{CREDIT_XS, CREDIT_YS, ""},

	// support section
	{CREDIT_HXS, CREDIT_HYS, ""},
	{CREDIT_HXS, CREDIT_HYS, ""},
	{CREDIT_HXS, CREDIT_HYS, "support"},
	{CREDIT_HXS, CREDIT_HYS, ""},

	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_TXS, CREDIT_TYS, "nintendo"},
	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_XS, CREDIT_YS, "mark deloura"},
	{CREDIT_XS, CREDIT_YS, "jim merrick"},
	{CREDIT_XS, CREDIT_YS, ""},

	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_TXS, CREDIT_TYS, "silicon graphics"},
	{CREDIT_TXS, CREDIT_TYS, ""},
	{CREDIT_XS, CREDIT_YS, "nathan pooley"},
	{CREDIT_XS, CREDIT_YS, ""},

	// special thanks section
	{CREDIT_HXS, CREDIT_HYS, ""},
	{CREDIT_HXS, CREDIT_HYS, ""},
	{CREDIT_HXS, CREDIT_HYS, "special thanks"},
	{CREDIT_HXS, CREDIT_HYS, ""},

	{CREDIT_XS, CREDIT_YS, "amy mbgia wooddell"},
	{CREDIT_XS, CREDIT_YS, "kelly coleman"},
	{CREDIT_XS, CREDIT_YS, "laurie rivera"},
	{CREDIT_XS, CREDIT_YS, "jean orrison"},
	{CREDIT_XS, CREDIT_YS, "valerie lozada"},
	{CREDIT_XS, CREDIT_YS, "dan west"},
	{CREDIT_XS, CREDIT_YS, "maria paradiso"},
	{CREDIT_XS, CREDIT_YS, "nicole scharff"},
	{CREDIT_XS, CREDIT_YS, "mary scarola"},
	{CREDIT_XS, CREDIT_YS, "russell byrd"},
	{CREDIT_XS, CREDIT_YS, "chris raimondi"},
	{CREDIT_XS, CREDIT_YS, "jane notgrass"},
	{CREDIT_XS, CREDIT_YS, "melissa marciano"},
	{CREDIT_XS, CREDIT_YS, "robin smith"},
	{CREDIT_XS, CREDIT_YS, ""},
	{-2},
	{CREDIT_HXS, CREDIT_HYS, ""},
	{CREDIT_HXS, CREDIT_HYS, ""},
	{CREDIT_HXS, CREDIT_HYS, ""},
	{CREDIT_HXS, CREDIT_HYS, ""},
/*
	// cheats
	{CREDIT_XS, CREDIT_YS, ""},
	{CREDIT_HXS, CREDIT_HYS, "cheat codes"},
	{CREDIT_XS, CREDIT_YS, ""},
	{CREDIT_XS, CREDIT_YS, ""},
	{CREDIT_TXS, CREDIT_TYS, "gallery"},
	{CREDIT_XS, CREDIT_YS, "s h n t r r n t"},
	{CREDIT_XS, CREDIT_YS, ""},
	{CREDIT_TXS, CREDIT_TYS, "show credits"},
	{CREDIT_XS, CREDIT_YS, "t h t r t m"},
	{CREDIT_XS, CREDIT_YS, ""},
	{CREDIT_TXS, CREDIT_TYS, "all weapons"},
	{CREDIT_XS, CREDIT_YS, "t h m s c l s"},
	{CREDIT_XS, CREDIT_YS, ""},
	{CREDIT_TXS, CREDIT_TYS, "unlimited ammo"},
	{CREDIT_XS, CREDIT_YS, "l n j h n s n"},
	{CREDIT_XS, CREDIT_YS, ""},
	{CREDIT_TXS, CREDIT_TYS, "spiritual mode"},
	{CREDIT_XS, CREDIT_YS, "n d n l p"},
	{CREDIT_XS, CREDIT_YS, ""},
	{CREDIT_XS, CREDIT_YS, ""},
	{CREDIT_XS, CREDIT_YS, ""},
	{CREDIT_XS, CREDIT_YS, ""},
*/
	{-1},
} ;

int	NormalCodes[] =
{
	17,
	18,
	1,
	-1
} ;

int	HardCodes[] =
{
//	19,
//	20,
	1,
	2,
	3,
	-1
} ;


/*****************************************************************************
*
*	Function:		CCredits__Contruct()
*
******************************************************************************
*
*	Description:	Create the credits initial vars
*
*	Inputs:			none
*	Outputs:			none
*
*****************************************************************************/
void CCredits__Construct(void)
{
	Credits.pData = TheCredits ;
	Credits.ScrollY = 240 ;
	Credits.OffsetY = 0 ;
	Credits.Pause = FALSE ;
	Credits.SizeY = CREDIT_SIZE * Credits.pData->ys ;

//	GetApp()->m_WarpID = CHEAT_GALLERY_WARP_ID;
//	CEngineApp__SetupFadeTo(GetApp(), MODE_CREDITS);
}

/*****************************************************************************
*
*	Function:		CCredits__Update()
*
******************************************************************************
*
*	Description:	Update the scrolling credits
*
*	Inputs:			none
*	Outputs:			none
*
*****************************************************************************/
void CCredits__Update(void)
{
	if (Credits.Pause)
		return ;


	// initial scroll up
	if (Credits.ScrollY)
		Credits.ScrollY -= 1 ;//* frame_increment ;

	// each line scroll
	else
	{
		Credits.OffsetY += 1 ;//* frame_increment ;
		if (Credits.OffsetY >= (int)Credits.SizeY)
		{
			Credits.pData++ ;

			// hold on last page
			if (Credits.pData->xs == -2)
			{
				Credits.pData-- ;
				Credits.Pause = TRUE ;
 
				GetApp()->m_bPause = TRUE;
				CPause__Construct(&GetApp()->m_Pause);
				GetApp()->m_Pause.m_Mode = PAUSE_NORMAL;
				if (GetApp()->m_Difficulty == MEDIUM)
				{
					CGiveCheatCode__Construct(&GetApp()->m_GiveCheatCode, NormalCodes) ;
					GetApp()->m_bGiveCheatCode = TRUE ;
				}
				else if (GetApp()->m_Difficulty == HARD)
				{
					CGiveCheatCode__Construct(&GetApp()->m_GiveCheatCode, HardCodes) ;
					GetApp()->m_bGiveCheatCode = TRUE ;
				}
				else
					CEngineApp__SetupFadeTo(GetApp(), MODE_RESETGAME);
				return ;
			}

			// finished?
			if (Credits.pData->xs == -1)
			{
				CEngineApp__SetupFadeTo(GetApp(), MODE_RESETGAME);
				Credits.pData = TheCredits ;
				Credits.ScrollY = 240 ;
			}

			Credits.OffsetY -= (int)Credits.SizeY ;
			Credits.SizeY = CREDIT_SIZE * Credits.pData->ys ;
		}
	}


}


/*****************************************************************************
*
*	Function:		CCredits__Draw()
*
******************************************************************************
*
*	Description:	Draw the credits
*
*	Inputs:			GFX	**ppDLP
*	Outputs:			none
*
*****************************************************************************/
void CCredits__Draw(Gfx **ppDLP)
{
	int	y ;
	float	opacity ;
	float time ;
	float	MaxY, MinY ;
	float	xscale, yscale, yscaleadd ;
	t_Credit	*credit ;




	COnScreen__InitFontDraw(ppDLP) ;
#ifdef KANJI
	COnScreen__PrepareLargeFont(ppDLP);
#endif
	GetApp()->m_OnScreen.m_ShadowXOff = 1 ;
	GetApp()->m_OnScreen.m_ShadowYOff = 1 ;


	y = Credits.ScrollY - Credits.OffsetY ;
	credit = Credits.pData ;

	MinY = GetApp()->m_Camera.m_LetterBoxTop-10 ;
	MaxY = GetApp()->m_Camera.m_LetterBoxBottom ;

	// draw each option
	while ((y < MaxY) && (credit))
 	{

		// skip the pause line
		if (credit->xs == -2)
			credit++ ;

		if (y <= (MinY+40))
		{
			time = (((float)((MinY+40)-y)) / ((float)40)) ;
//			xscale = BlendFLOATCosine(time, credit->xs, credit->xs*2) ;
//			yscale = BlendFLOATCosine(time, credit->ys, 0) ;
			opacity = BlendFLOATCosine(time, 255, 0) ;
//			yscaleadd = credit->ys ;
		}
		else if (y > (MaxY-40))
		{
			time = (((float)(MaxY-y)) / ((float)40)) ;
//			xscale = BlendFLOATCosine(time, credit->xs*2, credit->xs) ;
//			yscale = BlendFLOATCosine(time, 0, credit->ys) ;
			opacity = BlendFLOATCosine(time, 0, 255) ;
//			yscaleadd = yscale ;
		}
		else
		{
			opacity = 255 ;
		}

		xscale = credit->xs ;
		yscale = credit->ys ;
		yscaleadd = yscale ;

		// set font color depending on title size
		if (credit->xs == (float)CREDIT_XS)
		{
			COnScreen__SetFontColor(ppDLP,
											0, 200, 200,
											0, 0, 200);
		}
		else if (credit->xs == (float)CREDIT_TXS)
		{
			COnScreen__SetFontColor(ppDLP,
											200, 200, 200,
											0, 200, 200);
		}
		else if (credit->xs == (float)CREDIT_HXS)
		{
			COnScreen__SetFontColor(ppDLP,
											255, 255, 255,
											200, 200, 200);
		}

		COnScreen__SetFontScale(xscale, yscale) ;
		COnScreen__DrawText(ppDLP,
								  credit->string,
								  320/2, y,
								  opacity, TRUE, TRUE) ;

		y += CREDIT_SIZE * yscaleadd ;

		credit++ ;
		if (credit->xs == -1)
			credit = NULL ;
	}
}












#if 0
/////////////////////////////////////////////////////////////////////////////
// Externs from spec file
/////////////////////////////////////////////////////////////////////////////
extern char _gfxTitleSegmentRomStart[], _gfxTitleSegmentRomEnd[] ;
extern char _gfxControllerSegmentRomStart[], _gfxControllerSegmentRomEnd[] ;


/////////////////////////////////////////////////////////////////////////////
// globals
/////////////////////////////////////////////////////////////////////////////

INT32		option_option ;
INT32		option_music_volume ;
INT32		option_sfx_volume ;
INT32		option_onscreen_opacity ;

#define	SEL_R		255
#define	SEL_G		0
#define	SEL_B		128
#define	UNSEL_R	255
#define	UNSEL_G	0
#define	UNSEL_B	128


/////////////////////////////////////////////////////////////////////////////
// Functions
/////////////////////////////////////////////////////////////////////////////

void DMATrueColorScreenToFrameBuffer(void *pData);


/////////////////////////////////////////////////////////////////////////////



void DMATrueColorScreenToFrameBuffer(void *pData)
{
	UINT32	*pFB = (UINT32 *) GetApp()->m_FrameData[0].m_pFrameBuffer ;
	UINT32	length = 320*240*4 ;

	//UINT8		r,g,b,a ;
	//UINT8		*pScreen ;
	//UINT32	i ;

	OSPri				ospri;

	// Turn off screen and setup true color display mode
	//osViBlack(TRUE);
	if (osTvType == 1)
		osViSetMode(&osViModeTable[OS_VI_NTSC_LPF2]);
	else
		osViSetMode(&osViModeTable[OS_VI_MPAL_LPF2]);

	osViSetSpecialFeatures(OS_VI_GAMMA_OFF);

	// DMA to frame buffer
	ospri = osGetThreadPri(NULL);
	osSetThreadPri(NULL, PRIORITY_AUDIOLOCK);
	osWritebackDCache(cfb_16_a, length);
	osInvalDCache(cfb_16_a, length);

	osPiStartDma(&romIOMessageBuf, OS_MESG_PRI_NORMAL, OS_READ,
					 (u32) pData, cfb_16_a, length,
					 &romMessageQ);

	// Wait for DMA to finish
	osRecvMesg(&romMessageQ, NULL, OS_MESG_BLOCK);
	osInvalDCache(pData, length);
	osSetThreadPri(NULL, ospri);

#if 0
// FIXED WITH NEW CONVERTOR - SEE CARL
	// Re-order bytes to display correct picture!
	i = 320 * 240 ;
	pScreen = (UINT8 *)pFB ;
	while(i--)
	{
		b = pScreen[0] ;
		g = pScreen[1] ;
		r = pScreen[2] ;
		a = pScreen[3] ;

		pScreen[0] = r ;
		pScreen[1] = g ;
		pScreen[2] = b ;
		pScreen[3] = 0 ;

		pScreen += 4 ;
	}
#endif


	// Turn screen back on
	osViSwapBuffer(cfb_16_a);
	//osViBlack(FALSE);
}




void CFrontEnd__Construct(CFrontEnd *pThis)
{
	// Setup pointers to controller vars
	pThis->m_Mode = 0 ;
}



BOOL CFrontEnd__Update(CFrontEnd *pThis, BOOL Start)
{
	INT32 hSound;
//	static int sfxtest = 34;
//	static float pitchtest = 1;
//	CVector3		tempvec;

//	INT32		x, y ;
//	UINT8		*pFB = (UINT8 *) GetApp()->m_FrameData[0].m_pFrameBuffer ;

	switch(pThis->m_Mode)
	{
		// Display title page
		case 0:
			DMATrueColorScreenToFrameBuffer(_gfxTitleSegmentRomStart) ;
			pThis->m_Mode++ ;
			break ;

		// Wait for start before controller screen
		case 1:
#if 0
			// fade screen away
			for (y=0; y<240; y++)
			{
				for (x=0; x<320; x++)
				{
					if (*pFB)
						(*pFB)-- ;
					pFB++ ;
					if (*pFB)
						(*pFB)-- ;
					pFB++ ;
					if (*pFB)
						(*pFB)-- ;
					pFB++ ;
					pFB++ ;
				}
			}
#endif
			
			if (Start)
			{

#if 0		// this code is to test sfx ordering, please do not delete
				//hSound = initSFX( sfxtest, 1);
				//hSound = initSFX( 99, 1);
				//playSFX(hSound);
				//sfxtest++;


				tempvec.x = 0;
				tempvec.y = 0;
				tempvec.z = 0;
				CScene__DoSoundEffect(&(GetApp()->m_Scene), 0, 0,	NULL, &tempvec, 0);

#else
				pThis->m_Mode++ ;
#endif
			}
			break ;

		// Display joypad controller screen
		case 2:
			DMATrueColorScreenToFrameBuffer(_gfxControllerSegmentRomStart) ;
			pThis->m_Mode++ ;
			break ;

		// Wait for start before quitting to game
		case 3:
			if (Start)
				return TRUE ;
			break ;
	}

	return FALSE ;
}


#endif
