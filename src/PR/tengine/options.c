// Options.Cpp

#include "cppu64.h"
#include "tengine.h"
#include "options.h"
#include "tmove.h"
#include "audio.h"
#include "dlists.h"
#include "gfx16bit.h"

/////////////////////////////////////////////////////////////////////////////
// Constants
/////////////////////////////////////////////////////////////////////////////

// Position and dimentions of options screen
#ifdef GERMAN
#define OPTIONS_WIDTH	(266)
#define OPTIONS_HEIGHT	(200)
#define OPTIONSMENU_Y 			(OPTIONS_Y + 48)
#elif defined(PLATFORM_PORT) && !defined(PLATFORM_3DS)
/* PC: the draw-distance row (24px) plus the two new PC rows (resolution + fullscreen, 12px each) overflow the
 * stock 210-tall box beyond the bottom of the 240px screen — grow the box and tighten the header gap so every
 * row stays on-screen. PC-only, so 3DS/N64 layout is byte-identical (the #else below). */
#define OPTIONS_WIDTH	(208)
#define OPTIONS_HEIGHT	(236)
#define OPTIONSMENU_Y 			(OPTIONS_Y + 30)
#elif defined(PLATFORM_3DS)
/* 3DS: one extra row (the swap-sticks toggle, 12px) vs stock — grow the box + tighten the header gap so
 * every row stays on-screen. 3DS-only; the N64 build keeps the stock 210 box (the #else). */
#define OPTIONS_WIDTH	(208)
#define OPTIONS_HEIGHT	(224)
#define OPTIONSMENU_Y 			(OPTIONS_Y + 34)
#else
#define OPTIONS_WIDTH	(208)
#define OPTIONS_HEIGHT	(210)
#define OPTIONSMENU_Y 			(OPTIONS_Y + 40)
#endif

#define OPTIONS_X			((320/2) - (OPTIONS_WIDTH/2))
#define OPTIONS_Y			((240/2) - (OPTIONS_HEIGHT/2))

// Position of menu selection

#define OPTIONSBIG_SPACING		24
#define OPTIONSMENU_SPACING	12

#define OPTIONSLIGHT_SPACING		24


#define	OPTIONS_SLIDER_HEIGHT		16
#define	OPTIONS_SLIDER_WIDTH			8
#define	OPTIONS_BAR_HEIGHT			8
#define	OPTIONS_BAR_WIDTH				128


typedef	struct s_Option
{
	int	Spacing ;
	char	*String ;
} t_Option;


/////////////////////////////////////////////////////////////////////////////
// Text
/////////////////////////////////////////////////////////////////////////////

#ifdef ENGLISH
// ---------------------- English text ---------------------------
static char	text_control_left[] = {"left handed"};
static char	text_control_right[] = {"right handed"};

static char	text_blood_off[] = {"no blood"};
static char	text_blood_red[] = {"red blood"};
static char	text_blood_green[] = {"green blood"};

static char	text_musicvolume[] = {"music volume"};
static char	text_sfxvolume[] = {"sfx volume"};
static char	text_opacity[] = {"opacity"};
static char	text_hanalog[] = {"horizontal analog"};
static char	text_vanalog[] = {"vertical analog"};
static char	text_exit[] = {"exit"};
static char	text_levelicons[] = {"level icons"};
#endif

#ifdef GERMAN
// ---------------------- German text ---------------------------
static char	text_control_left[] = {"linksh<nder"};		//a.
static char	text_control_right[] = {"rechtsh<nder"};	//a.

//static char	text_blood_off[] = {"kein blut"};
//static char	text_blood_red[] = {"rotes blut"};
//static char	text_blood_green[] = {"gr/nes blut"};		//u.

static char	text_musicvolume[] = {"musik lautst<rke"};	//a.
static char	text_sfxvolume[] = {"soundeffekt lautst<rke"};	//a.
static char	text_opacity[] = {"undurchsichtigkeit"};
static char	text_hanalog[] = {"stick horizontal"};
static char	text_vanalog[] = {"stick vertikal"};
static char	text_exit[] = {"beenden"};
static char	text_levelicons[] = {"level schl/ssel"};
#endif


#ifdef KANJI
// ---------------------- Japanese text ---------------------------
static char	text_control_left[] = {0x24, 0x19, 0x13, 0x18, 0x27, 0x2F, -1};
static char	text_control_right[] = {0x21, 0x01, 0x13, 0x18, 0x27, 0x2F, -1};

static char	text_blood_off[] = {0x32, 0x21, 0x40, 0x2F, 0x04, 0x19, -1};
//static char	text_blood_red[] = {0x00, -1/*"red blood"*/};
static char	text_blood_green[] = {0x32, 0x21, 0x40, 0x2F, 0x04, 0x27, -1};

static char	text_musicvolume[] = {0x1C, 0x3E, 0x41, 0x2C, 0x40, 0x07, -1};
static char	text_sfxvolume[] = {0x0A, 0x02, 0x27, 0x2F, -1};
static char	text_opacity[] = {0x13, 0x02, 0x1e, 0x01, 0xab, -1};
static char	text_hanalog[] = {0x00, 0x14, 0x25, 0x29, 0x8D, 0x8F, -1};
static char	text_vanalog[] = {0x00, 0x14, 0x25, 0x29, 0x91, 0x92, -1};
static char	text_exit[] = {0x6B, 0x7B, 0x70, -1};
static char	text_levelicons[] = {0x82, 0x83, 0x93, 0xeb, 0xa0, 0xa1, -1};
#endif

#ifdef PLATFORM_PORT
static char	text_drawdist[] = {"draw distance"};	// PC+3DS port slider label
static INT32 s_DrawDistSlider = 128;	// draw-distance bar position (0..255). FILE-SCOPE on purpose —
										// NOT a COptions field (growing CEngineApp = stale-build layout corruption).
#endif

#ifdef PLATFORM_3DS
/* 3DS swap-sticks row: the String is reassigned each Draw to show which stick LOOKS (aims). NB the LARGE_FONT
 * the options menu draws with has NO ':' or '-' glyph (they alias other letters), so the labels use plain words.
 * New 3DS only (dual-analog); on OG 3DS the row is inert (single stick = classic move+turn). */
static char	text_look_cpad[]   = {"look circle pad"};	// swap_sticks 0 (default): C-stick nub MOVES, Circle Pad AIMS
static char	text_look_cstick[] = {"look c stick"};		// swap_sticks 1: Circle Pad moves, C-stick aims
#endif

#if defined(PLATFORM_PORT) && !defined(PLATFORM_3DS)
#include "turok_binds.h"
/* ── PC CONTROLS (key/mouse rebind) submenu ─────────────────────────────────────────────────────────────────
 * A sub-mode of the options screen (like EDIT_FOG), branched at the top of COptions__Update/Draw. State =
 * FILE-SCOPE STATICS ONLY (never COptions/CEngineApp fields — the struct-growth layout-corruption gotcha,
 * options.h:114-118). The rows list each rebindable action + its current PRIMARY binding; UP/DOWN select,
 * activate (Enter) arms capture (the NEXT key/mouse press becomes the binding; ESC in gfx_sdl2 cancels).
 * The binding tokens + capture seam live in config.c (turok_binds.h); this menu never touches SDL. */
static char	text_controls[] = {"controls"};
static int	s_ControlsActive = 0;	/* 1 = the CONTROLS submenu is showing. */
static int	s_ControlsSel    = 0;	/* selected row 0..CONTROLS_ROWS-1. */
#define CONTROLS_ROWS			(BIND_MAX + 2)	/* 13 actions + "defaults" + "back" */
#define CONTROLS_ROW_DEFAULTS	(BIND_MAX)
#define CONTROLS_ROW_BACK		(BIND_MAX + 1)
#define CONTROLS_WIDTH			(252)
#define CONTROLS_HEIGHT			(232)
#define CONTROLS_X				((320/2) - (CONTROLS_WIDTH/2))
#define CONTROLS_Y				((240/2) - (CONTROLS_HEIGHT/2))
#define CONTROLS_ROW_SP			(14)
#define CONTROLS_MENU_Y			(CONTROLS_Y + 12)
extern void turokConfigSave(void);
static INT32 options_controls_update(COptions *pThis) ;
static void  options_controls_draw(COptions *pThis, Gfx **ppDLP) ;

/* ── PC screen-resolution + fullscreen rows ─────────────────────────────────────────────────────────────────
 * State = FILE-SCOPE STATICS ONLY (never COptions/CEngineApp fields — struct growth shifts every member after
 * m_Options and a stale build links mismatched layouts → corrupted cache/texture pointers, the documented
 * options.h:114-118 gotcha). The SOURCE OF TRUTH is g_cfg_win_w/h + g_cfg_fullscreen (persisted in turok.cfg);
 * these statics only hold the selector position + the display label. LARGE_FONT has digits + 'x' + space, so
 * "1920 x 1080" renders — but NEVER ':' or '-' (they alias to the Z/M glyphs). */
static char	s_res_label[24]        = {"resolution"};	// rewritten each Draw: "1920 x 1080" preset, or "desktop"
static char	text_fullscreen_on[]   = {"fullscreen"};
static char	text_fullscreen_off[]  = {"windowed"};
/* Curated windowed presets (4:3 + 16:9 + 4:3-hi + 4K). DESKTOP (native) is the extra slot at index == COUNT.
 * 3840x2160 (4K) is included explicitly so a 4K-desktop user sees a "3840 x 2160" number in the list rather
 * than only the DESKTOP entry — while fullscreen it renders at that internal resolution (matches native on a
 * 4K panel = pixel-perfect; supersamples down on a smaller panel). LARGE_FONT has digits + 'x' + space, so
 * "3840 x 2160" renders; NEVER add ':' or '-' (they alias to the Z/M glyphs). */
static const int s_res_presets[][2] =
{
	{960,720}, {1280,720}, {1280,960}, {1600,900}, {1600,1200}, {1920,1080}, {2560,1440}, {3840,2160}
};
#define OPTIONS_RES_COUNT	((int)(sizeof(s_res_presets)/sizeof(s_res_presets[0])))
static int	s_ResIndex = -1;	// 0..COUNT-1 = a preset; COUNT = DESKTOP (native). -1 = unseeded (seeded in SetDefaults).
static int	s_ResLatch = 0;		// LEFT/RIGHT step latch — one step per press (delta must re-center before it steps again).

/* Live-apply seam: options sets the cfg + hands the request to gfx_sdl2, which applies it next frame at the
 * top of gfx_sdl_handle_events (the same game-thread/frame boundary Alt-Enter already uses — strictly safe). */
extern int g_cfg_win_w, g_cfg_win_h, g_cfg_fullscreen;
extern int g_turok_req_win_w, g_turok_req_win_h, g_turok_req_fullscreen, g_turok_req_dirty;

static void options_apply_resolution(void)
{
	if (s_ResIndex >= OPTIONS_RES_COUNT) {
		/* DESKTOP: gfx_sdl2 resolves the native desktop mode and writes g_cfg_win_w/h back for persistence. */
		g_turok_req_win_w = -1 ;
		g_turok_req_win_h = -1 ;
	} else {
		g_cfg_win_w = s_res_presets[s_ResIndex][0] ;
		g_cfg_win_h = s_res_presets[s_ResIndex][1] ;
		g_turok_req_win_w = g_cfg_win_w ;
		g_turok_req_win_h = g_cfg_win_h ;
	}
	g_turok_req_dirty = 1 ;
}

static void options_toggle_fullscreen(void)
{
	g_cfg_fullscreen = g_cfg_fullscreen ? 0 : 1 ;
	g_turok_req_fullscreen = g_cfg_fullscreen ;
	g_turok_req_dirty = 1 ;
}
#endif

t_Option options[]=
{
	OPTIONSBIG_SPACING, text_musicvolume,
	OPTIONSBIG_SPACING, text_sfxvolume,
	OPTIONSBIG_SPACING, text_opacity,
	OPTIONSBIG_SPACING, text_hanalog,
	OPTIONSBIG_SPACING, text_vanalog,
#ifdef PLATFORM_PORT
	OPTIONSBIG_SPACING, text_drawdist,
#endif
#if defined(PLATFORM_PORT) && !defined(PLATFORM_3DS)
	OPTIONSMENU_SPACING, s_res_label,			// String reassigned each Draw (multi-state text row)
	OPTIONSMENU_SPACING, text_fullscreen_off,	// String reassigned each Draw
	OPTIONSMENU_SPACING, text_controls,			// key/mouse rebind submenu
#endif
#ifdef PLATFORM_3DS
	OPTIONSMENU_SPACING, text_look_cpad,		// swap-sticks toggle; String reassigned each Draw
#endif
	OPTIONSMENU_SPACING, text_control_left,
#ifndef GERMAN
	OPTIONSMENU_SPACING, text_blood_off,
#endif
	OPTIONSMENU_SPACING, text_exit,
} ;

#ifdef EDIT_FOG_AND_LIGHTS
char	redstring[20] ;
char	greenstring[20] ;
char	bluestring[20] ;
char	aredstring[20] ;
char	agreenstring[20] ;
char	abluestring[20] ;
char	dredstring[20] ;
char	dgreenstring[20] ;
char	dbluestring[20] ;


t_Option FOGoptions[]=
{
	OPTIONSBIG_SPACING, redstring,
	OPTIONSBIG_SPACING, greenstring,
	OPTIONSBIG_SPACING, bluestring,
} ;

t_Option LIGHToptions[]=
{
	OPTIONSLIGHT_SPACING, aredstring,
	OPTIONSLIGHT_SPACING, agreenstring,
	OPTIONSLIGHT_SPACING, abluestring,
	OPTIONSLIGHT_SPACING, dredstring,
	OPTIONSLIGHT_SPACING, dgreenstring,
	OPTIONSLIGHT_SPACING, dbluestring,
} ;

INT32 COptions__UpdateFOG(COptions *pThis) ;
void COptions__DrawFOG(COptions *pThis, Gfx **ppDLP) ;
INT32 COptions__UpdateLIGHT(COptions *pThis) ;
void COptions__DrawLIGHT(COptions *pThis, Gfx **ppDLP) ;
#endif

/////////////////////////////////////////////////////////////////////////////
// Functions
/////////////////////////////////////////////////////////////////////////////

// Store current options (used before running an attract demo)
void COptions__Store(COptions *pThis)
{
	// Only store if not already stored
	if (!pThis->m_OptionsStored)
	{
		pThis->m_OptionsStored = TRUE ;
		pThis->m_Storage_CheatFlags = GetApp()->m_dwCheatFlags ;
		pThis->m_Storage_RHControl = pThis->m_RHControl ;
		pThis->m_Storage_Blood		= pThis->m_Blood ;
		pThis->m_Storage_Opacity	= pThis->m_Opacity ;
		pThis->m_Storage_HAnalog  	= pThis->m_HAnalog ;
		pThis->m_Storage_VAnalog  	= pThis->m_VAnalog ;
		pThis->m_Storage_Difficulty = GetApp()->m_Difficulty ;
	}
}

// Restore current options (used after running an attract demo)
void COptions__Restore(COptions *pThis)
{
	// Only restore if it was stored
	if (pThis->m_OptionsStored)
	{
		pThis->m_OptionsStored = FALSE ;
		GetApp()->m_dwCheatFlags = pThis->m_Storage_CheatFlags ;
		pThis->m_RHControl	= pThis->m_Storage_RHControl ;
		pThis->m_Blood			= pThis->m_Storage_Blood ;
		pThis->m_Opacity		= pThis->m_Storage_Opacity ;
		pThis->m_HAnalog		= pThis->m_Storage_HAnalog ;
		pThis->m_VAnalog		= pThis->m_Storage_VAnalog ;
		GetApp()->m_Difficulty = pThis->m_Storage_Difficulty ;
		CTControl__CTControl(pThis->m_RHControl) ;
	}
}

// Called before playing/recording an attract demo
void COptions__SetAttractDefaults(COptions *pThis)
{
	GetApp()->m_dwCheatFlags = 0 ;
	pThis->m_Opacity = 255 ;
	pThis->m_HAnalog = 128 ;
	pThis->m_VAnalog = 128 ;
	pThis->m_RHControl = TRUE;
#ifdef KANJI
	pThis->m_Blood = BLOOD_GREEN ;
#endif
#ifdef ENGLISH
	pThis->m_Blood = BLOOD_RED ;
#endif
#ifdef GERMAN
	pThis->m_Blood = BLOOD_OFF ;
#endif

	GetApp()->m_Difficulty = MEDIUM ;

	CTControl__CTControl(pThis->m_RHControl) ;
}				

void COptions__SetDefaults(COptions *pThis)
{
	pThis->m_MusicVolume = 180 ;
	pThis->m_SFXVolume = 255;
	pThis->m_Opacity = 255 ;
	pThis->m_HAnalog = 100 ;
	pThis->m_VAnalog = 100 ;
#ifdef PLATFORM_PORT
	/* seed the draw-distance slider from g_cfg_drawdist (1..max -> 0..255; max = 8 PC, 2 New 3DS, 1 OG=locked) */
	{	extern float g_cfg_drawdist; extern float turok_drawdist_max(void);
		float mxr = turok_drawdist_max() - 1.0f;
		int dd = (mxr > 0.01f) ? (int)(((g_cfg_drawdist - 1.0f) / mxr) * 255.0f + 0.5f) : 0;
		s_DrawDistSlider = (dd < 0) ? 0 : (dd > 255) ? 255 : dd;
		/* OG 3DS (max==1x): HIDE the row — zero spacing makes the selection bar, text list AND slider
		 * accumulation all skip it; the text/slider draws and the navigation check Spacing==0 to skip too. */
		options[OPTIONS_DRAWDIST].Spacing = (mxr <= 0.01f) ? 0 : OPTIONSBIG_SPACING;
	}
#endif
#if defined(PLATFORM_PORT) && !defined(PLATFORM_3DS)
	/* Seed the resolution selector from the saved window size: exact preset match, else the nearest preset by
	 * width (a hand-edited custom size shows the nearest preset label until the user cycles). */
	{	int i, best = 0, bestd = 0x7fffffff;
		s_ResIndex = -1;
		for (i = 0; i < OPTIONS_RES_COUNT; i++) {
			if (s_res_presets[i][0] == g_cfg_win_w && s_res_presets[i][1] == g_cfg_win_h) { s_ResIndex = i; break; }
			{ int d = g_cfg_win_w - s_res_presets[i][0]; if (d < 0) d = -d;
			  if (d < bestd) { bestd = d; best = i; } }
		}
		if (s_ResIndex < 0) s_ResIndex = best;
		s_ResLatch = 0;
	}
	/* HIDE the right/left-handed toggle on PC (Spacing=0 collapses it out of the bar/text/nav — same mechanism
	 * as the hidden OG-3DS draw-dist row). The whole PC scheme (WASD on the C-buttons) is only correct with
	 * m_RHControl=TRUE; left-handed moves movement to the D-pad and makes the C-buttons fire the run/walk toggle,
	 * so every WASD press would toggle run/walk. With SDL-side remapping hand-ness is meaningless, so lock it. */
	options[OPTIONS_CONTROL].Spacing = 0;
#endif
	pThis->m_RHControl = TRUE;
#ifdef KANJI
	pThis->m_Blood = BLOOD_GREEN ;
#endif
#ifdef ENGLISH
	pThis->m_Blood = BLOOD_RED ;
#endif
#ifdef GERMAN
	pThis->m_Blood = BLOOD_OFF ;
#endif

	GetApp()->m_ActualMusicVolume = ((float)pThis->m_MusicVolume / 255.0);
	GetApp()->m_ActualSFXVolume = ((float)pThis->m_SFXVolume / 255.0);

	pThis->m_OptionsStored = FALSE ;
}


void COptions__Construct(COptions *pThis)
{
	CROMRegionSet			*pRegionSet;

	// Setup default selection
	pThis->m_Selection = 0 ;

#ifdef EDIT_FOG_AND_LIGHTS
	pThis->m_EditFog = FALSE ;
	pThis->m_EditLight = FALSE ;
	pRegionSet = CScene__GetRegionAttributes(&GetApp()->m_Scene, CEngineApp__GetPlayer(GetApp())->ah.ih.m_pCurrentRegion);
	if (pRegionSet)
	{
		pThis->m_Red = pRegionSet->m_FogColor[0] ;
		pThis->m_Green = pRegionSet->m_FogColor[1] ;
		pThis->m_Blue = pRegionSet->m_FogColor[2] ;
	}

	pThis->m_ARed = thelight.a.l.col[0] ;
	pThis->m_AGreen = thelight.a.l.col[1] ;
	pThis->m_ABlue = thelight.a.l.col[2] ;
	pThis->m_DRed = thelight.l->l.col[0] ;
	pThis->m_DGreen = thelight.l->l.col[1] ;
	pThis->m_DBlue = thelight.l->l.col[2] ;
#endif

//	if (pThis->m_Mode == OPTIONS_NULL)
		pThis->m_Alpha = 0 ;

	pThis->m_Mode = OPTIONS_FADEUP ;

	pThis->b_Active = TRUE ;
}



//------------------------------------------------------------------------
// Update selection on options (+ load and save) screen
//------------------------------------------------------------------------
INT32 COptions__Update(COptions *pThis)
{
	float				delta ;
//	float				VolChange;
	INT32				ReturnValue = -1 ;

#if defined(PLATFORM_PORT) && !defined(PLATFORM_3DS)
	if (s_ControlsActive)
		return options_controls_update(pThis) ;
#endif

#ifdef EDIT_FOG_AND_LIGHTS
	// do FOG options
	if (pThis->m_EditFog)
	{
		ReturnValue = COptions__UpdateFOG(pThis) ;
	}
	// do LIGHT options
	else if (pThis->m_EditLight)
	{
		ReturnValue = COptions__UpdateLIGHT(pThis) ;
	}


	// do options options
	else
#endif
	{
		// Goto next selection?
		if (CEngineApp__MenuDown(GetApp()))
		{
			BarTimer = 0 ;
			pThis->m_Selection++ ;
			if (pThis->m_Selection >= OPTIONS_END_SELECTION)
				pThis->m_Selection = 0 ;
#ifdef PLATFORM_PORT
			/* skip any HIDDEN row (Spacing==0): the OG-3DS draw-dist row and the PC-hidden control row. */
			{	int guard = 0 ;
				while (options[pThis->m_Selection].Spacing == 0 && guard++ < OPTIONS_END_SELECTION) {
					pThis->m_Selection++ ;
					if (pThis->m_Selection >= OPTIONS_END_SELECTION) pThis->m_Selection = 0 ;
				}
			}
#endif
		}

		// Goto previous selection?
		if (CEngineApp__MenuUp(GetApp()))
		{
			BarTimer = 0 ;
			pThis->m_Selection-- ;
			if (pThis->m_Selection < 0)
				pThis->m_Selection = OPTIONS_END_SELECTION-1 ;
#ifdef PLATFORM_PORT
			{	int guard = 0 ;
				while (options[pThis->m_Selection].Spacing == 0 && guard++ < OPTIONS_END_SELECTION) {
					pThis->m_Selection-- ;
					if (pThis->m_Selection < 0) pThis->m_Selection = OPTIONS_END_SELECTION-1 ;
				}
			}
#endif
		}


		// check movement on sliders
		delta = (CTControl__IsAnalogRight(pCTControl) - CTControl__IsAnalogLeft(pCTControl)) /4 ;
		delta -= CTControl__IsMapRight(pCTControl) ;
		delta += CTControl__IsMapLeft(pCTControl) ;

		switch (pThis->m_Selection)
		{
			case OPTIONS_MUSIC_VOLUME:
				pThis->m_MusicVolume += delta ;

				if(pThis->m_MusicVolume < 0)
					pThis->m_MusicVolume = 0;

				if(pThis->m_MusicVolume > 255)
					pThis->m_MusicVolume = 255;

				GetApp()->m_ActualMusicVolume = ((float)pThis->m_MusicVolume / 255.0);

//				VolChange = pThis->m_MusicVolume/255.0;
//				__GlobalSEQvolume = (INT16)(VolChange * MAX_SEQ_VOL);
//				alCSPSetVol(seqp, __GlobalSEQvolume);
				// update music vol

			break ;

			case OPTIONS_SFX_VOLUME:
				pThis->m_SFXVolume += delta ;

				if(pThis->m_SFXVolume < 0)
					pThis->m_SFXVolume = 0;

				if(pThis->m_SFXVolume > 255)
					pThis->m_SFXVolume = 255;

				GetApp()->m_ActualSFXVolume = ((float)pThis->m_SFXVolume / 255.0);

//				__GlobalSFXscalar = pThis->m_SFXVolume/255.0;

				// update sfx vol
			break ;

			case OPTIONS_OPACITY:
				pThis->m_Opacity += delta ;

				if (pThis->m_Opacity < 0)
					pThis->m_Opacity = 0;
				if (pThis->m_Opacity >255)
					pThis->m_Opacity = 255;
				break ;

			case OPTIONS_HANALOG:
				pThis->m_HAnalog += delta ;

				if (pThis->m_HAnalog < 0)
					pThis->m_HAnalog = 0;
				if (pThis->m_HAnalog >255)
					pThis->m_HAnalog = 255;
				break ;
			case OPTIONS_VANALOG:
				pThis->m_VAnalog += delta ;

				if (pThis->m_VAnalog < 0)
					pThis->m_VAnalog = 0;
				if (pThis->m_VAnalog >255)
					pThis->m_VAnalog = 255;
				break ;
#ifdef PLATFORM_PORT
			case OPTIONS_DRAWDIST:
			{	extern float g_cfg_drawdist; extern float turok_drawdist_max(void);
				s_DrawDistSlider += delta ;
				if (s_DrawDistSlider < 0)   s_DrawDistSlider = 0;
				if (s_DrawDistSlider > 255) s_DrawDistSlider = 255;
				/* ONE slider: far-clip multiplier 1x..max (3x PC, 1.25x New 3DS, 1x OG = locked). The engine's
				 * fog is normalized to the far plane, so pushing the plane out makes the fog RECEDE and THIN on
				 * its own (the gradient just stretches over more distance) — exactly the desired look. */
				g_cfg_drawdist = 1.0f + ((float)s_DrawDistSlider / 255.0f) * (turok_drawdist_max() - 1.0f) ;
				break ;
			}
#endif
#if defined(PLATFORM_PORT) && !defined(PLATFORM_3DS)
			case OPTIONS_RESOLUTION:
			{	int step = (delta > 0.5f) ? 1 : (delta < -0.5f) ? -1 : 0;	/* prev/next preset, one per press */
				if (step && !s_ResLatch) {
					s_ResLatch = 1;
					s_ResIndex += step;
					if (s_ResIndex < 0)                 s_ResIndex = OPTIONS_RES_COUNT;	/* wrap to DESKTOP */
					if (s_ResIndex > OPTIONS_RES_COUNT) s_ResIndex = 0;
					options_apply_resolution();
				} else if (!step) {
					s_ResLatch = 0;
				}
				break ;
			}
			case OPTIONS_FULLSCREEN:
			{	int step = (delta > 0.5f) ? 1 : (delta < -0.5f) ? -1 : 0;
				if (step && !s_ResLatch) {
					s_ResLatch = 1;
					options_toggle_fullscreen();
				} else if (!step) {
					s_ResLatch = 0;
				}
				break ;
			}
#endif
		}

		// check for start button on current item
		if (CTControl__IsUseMenu(pCTControl))
			ReturnValue = pThis->m_Selection ;

#if defined(PLATFORM_PORT) && !defined(PLATFORM_3DS)
		if (ReturnValue == OPTIONS_RESOLUTION)		/* activate cycles forward through the presets + DESKTOP */
		{
			s_ResIndex++ ;
			if (s_ResIndex > OPTIONS_RES_COUNT) s_ResIndex = 0 ;
			options_apply_resolution() ;
			ReturnValue = -1 ;
		}
		else if (ReturnValue == OPTIONS_FULLSCREEN)	/* activate toggles fullscreen */
		{
			options_toggle_fullscreen() ;
			ReturnValue = -1 ;
		}
		else if (ReturnValue == OPTIONS_CONTROLS)	/* activate enters the key/mouse rebind submenu */
		{
			s_ControlsActive = 1 ;
			s_ControlsSel = 0 ;
			ReturnValue = -1 ;
		}
		else
#endif
		if (ReturnValue == OPTIONS_CONTROL)
		{
			pThis->m_RHControl ^= TRUE ;
			CTControl__CTControl(pThis->m_RHControl) ;
			ReturnValue = -1 ;
		}
#ifdef PLATFORM_3DS
		else if (ReturnValue == OPTIONS_SWAPSTICKS)	/* activate toggles which stick moves vs looks */
		{
			extern int g_cfg_swap_sticks;
			g_cfg_swap_sticks ^= 1 ;
			ReturnValue = -1 ;
		}
#endif
#ifndef GERMAN
		else if (ReturnValue == OPTIONS_BLOOD)
		{
			pThis->m_Blood++ ;
			if (pThis->m_Blood >= BLOOD_END)
				pThis->m_Blood = BLOOD_OFF ;

#ifdef KANJI
			if (pThis->m_Blood == BLOOD_RED)
				pThis->m_Blood = BLOOD_GREEN ;
#endif
			ReturnValue = -1 ;
		}
#endif
		else if (ReturnValue == OPTIONS_EXIT)
		{
//			GetApp()->m_Pause.m_MusicVolume = GetApp()->m_ActualMusicVolume;
//			GetApp()->m_Pause.m_SFXVolume = GetApp()->m_ActualSFXVolume;
#if defined(PLATFORM_PORT) && !defined(PLATFORM_3DS)
			/* persist the g_cfg-backed settings (resolution, fullscreen, draw distance, …) once on menu close.
			 * A pending DESKTOP request has been applied by now, so g_cfg_win_w/h holds the resolved dims. */
			{ extern void turokConfigSave(void); turokConfigSave(); }
#elif defined(PLATFORM_3DS)
			/* 3DS: persist the swap-sticks toggle (and the rest of turok.cfg) to sdmc so it survives a reboot. */
			{ extern void turokConfigSave(void); turokConfigSave(); }
#endif
			pThis->m_Mode = OPTIONS_FADEDOWN ;
		}
		else
			ReturnValue = -1 ;

		// do corresponding action
//		switch (ReturnValue)
//		{
//			case OPTIONS_LOAD:
//				ReturnValue = -1 ;
//				pThis->m_Mode = OPTIONS_FADEDOWN ;

//				CLoad__Construct(&GetApp()->m_Load, pThis->m_pCTControl) ;
//				GetApp()->m_bLoad = TRUE ;
//				break ;

//			case OPTIONS_SAVE:
//				ReturnValue = -1 ;
//				pThis->m_Mode = OPTIONS_FADEDOWN ;

//				CSave__Construct(&GetApp()->m_Save, pThis->m_pCTControl) ;
//				GetApp()->m_bSave = TRUE ;
//				break ;
//		}
	}

	return ReturnValue ;
}

#if defined(PLATFORM_PORT) && !defined(PLATFORM_3DS)
//------------------------------------------------------------------------
// CONTROLS submenu (key/mouse rebinding) — a sub-mode of the options screen.
//------------------------------------------------------------------------
static INT32 options_controls_update(COptions *pThis)
{
	// A capture just finished (gfx_sdl2 set done, action back to -1): store the token into the selected
	// action's PRIMARY slot (the alt slot is preserved), flag gfx_sdl2 to re-resolve, and persist.
	if (g_bind_capture_done)
	{
		if (!g_bind_capture_cancel && s_ControlsSel >= 0 && s_ControlsSel < BIND_MAX)
		{
			strncpy(g_cfg_bind[s_ControlsSel][0], g_bind_capture_result, TUROK_BIND_TOKLEN-1) ;
			g_cfg_bind[s_ControlsSel][0][TUROK_BIND_TOKLEN-1] = 0 ;
			g_cfg_bind_dirty = 1 ;
			turokConfigSave() ;
		}
		g_bind_capture_done = 0 ;
		g_bind_capture_cancel = 0 ;
		return -1 ;
	}
	// Capture still armed: waiting for a raw press; suppress navigation (the pad is held neutral meanwhile).
	if (g_bind_capture_action >= 0)
		return -1 ;

	if (CEngineApp__MenuDown(GetApp()))
	{
		BarTimer = 0 ;
		s_ControlsSel++ ;
		if (s_ControlsSel >= CONTROLS_ROWS) s_ControlsSel = 0 ;
	}
	if (CEngineApp__MenuUp(GetApp()))
	{
		BarTimer = 0 ;
		s_ControlsSel-- ;
		if (s_ControlsSel < 0) s_ControlsSel = CONTROLS_ROWS-1 ;
	}

	if (CTControl__IsUseMenu(pCTControl))
	{
		if (s_ControlsSel < BIND_MAX)
		{
			// arm capture for this action's PRIMARY slot; gfx_sdl2 fills it on the next raw press.
			g_bind_capture_done = 0 ;
			g_bind_capture_cancel = 0 ;
			g_bind_capture_action = s_ControlsSel ;
		}
		else if (s_ControlsSel == CONTROLS_ROW_DEFAULTS)
		{
			turokBindsSetDefaults() ;
			g_cfg_bind_dirty = 1 ;
			turokConfigSave() ;
		}
		else	// CONTROLS_ROW_BACK
		{
			turokConfigSave() ;
			s_ControlsActive = 0 ;
			s_ControlsSel = 0 ;
		}
	}

	return -1 ;
}

static void options_controls_draw(COptions *pThis, Gfx **ppDLP)
{
	int		i, y ;
	char	line[48], bindname[24] ;

	if (pThis->m_Alpha == 0)
		return ;

	COnScreen__InitBoxDraw(ppDLP) ;
	COnScreen__DrawHilightBox(ppDLP,
							  CONTROLS_X, CONTROLS_Y,
							  CONTROLS_X+CONTROLS_WIDTH, CONTROLS_Y+CONTROLS_HEIGHT,
							  1, FALSE, 0,0,0, 180 * pThis->m_Alpha) ;

	// selection bar
	y = CONTROLS_MENU_Y + s_ControlsSel * CONTROLS_ROW_SP ;
	CPause__InitPolygon(ppDLP) ;
	CPause__DrawBar(ppDLP, CONTROLS_X+8, y-1, CONTROLS_WIDTH-16, CONTROLS_ROW_SP, pThis->m_Alpha) ;

	// rows (LARGE_FONT is lowercase letters + digits + space; no ':' / '-')
	COnScreen__InitFontDraw(ppDLP) ;
	COnScreen__SetFontScale(0.8, 0.6) ;
	y = CONTROLS_MENU_Y ;
	for (i = 0; i < CONTROLS_ROWS; i++)
	{
		if (i < BIND_MAX)
		{
			if (g_bind_capture_action == i)
				strcpy(bindname, "press a key") ;
			else
				turokBindDisplayName(g_cfg_bind[i][0], bindname, sizeof bindname) ;
			sprintf(line, "%-11s%s", turokBindActionLabel(i), bindname) ;
		}
		else if (i == CONTROLS_ROW_DEFAULTS)
			strcpy(line, "defaults") ;
		else
			strcpy(line, "back") ;

		if (i == s_ControlsSel)
			COnScreen__SetFontColor(ppDLP, 200*1.25, 200*1.25, 138*1.25, 86*1.25, 71*1.25, 47*1.25) ;
		else
			COnScreen__SetFontColor(ppDLP, 200*.9, 200*.9, 138*.9, 86*.9, 71*.9, 47*.9) ;

		COnScreen__DrawText(ppDLP, line, CONTROLS_X+12, y, (int)(255 * pThis->m_Alpha), FALSE, TRUE) ;
		y += CONTROLS_ROW_SP ;
	}
}
#endif	/* PLATFORM_PORT && !PLATFORM_3DS */

#ifdef EDIT_FOG_AND_LIGHTS
//------------------------------------------------------------------------
// EDIT FOG
//------------------------------------------------------------------------
INT32 COptions__UpdateFOG(COptions *pThis)
{
	float				delta ;
	INT32				ReturnValue = -1 ;

	CROMRegionSet			*pRegionSet;
	
	// Goto next selection?
	if (CEngineApp__MenuDown(GetApp()))
	{
		BarTimer = 0 ;
		pThis->m_Selection++ ;
		if (pThis->m_Selection >= FOG_END_SELECTION)
			pThis->m_Selection = 0 ;
	}

	// Goto previous selection?
	if (CEngineApp__MenuUp(GetApp()))
	{
		BarTimer = 0 ;
		pThis->m_Selection-- ;
		if (pThis->m_Selection < 0)
			pThis->m_Selection = FOG_END_SELECTION-1 ;
	}

	// check movement on sliders
	delta = (CTControl__IsAnalogRight(pCTControl) - CTControl__IsAnalogLeft(pCTControl)) /4 ;
	delta -= CTControl__IsMapRight(pCTControl) ;
	delta += CTControl__IsMapLeft(pCTControl) ;

	switch (pThis->m_Selection)
	{
		case FOG_RED:
			pThis->m_Red += delta ;

			if(pThis->m_Red < 0)
	  			pThis->m_Red = 0;
 			if(pThis->m_Red > 255)
 				pThis->m_Red = 255;
			break ;

		case FOG_GREEN:
			pThis->m_Green += delta ;

			if(pThis->m_Green < 0)
	  			pThis->m_Green = 0;
 			if(pThis->m_Green > 255)
 				pThis->m_Green = 255;
			break ;

		case FOG_BLUE:
			pThis->m_Blue += delta ;

			if(pThis->m_Blue < 0)
	  			pThis->m_Blue = 0;
 			if(pThis->m_Blue > 255)
 				pThis->m_Blue = 255;
			break ;
	}

	pRegionSet = CScene__GetRegionAttributes(&GetApp()->m_Scene, CEngineApp__GetPlayer(GetApp())->ah.ih.m_pCurrentRegion);
	if (pRegionSet)
	{
		pRegionSet->m_FogColor[0] = pThis->m_Red ;
		pRegionSet->m_FogColor[1] = pThis->m_Green ;
		pRegionSet->m_FogColor[2] = pThis->m_Blue ;
	}

	// check for start button on current item
	if (CTControl__IsUseMenu(pCTControl))
		ReturnValue = pThis->m_Selection ;

	if (ReturnValue != -1)
		pThis->m_Mode = OPTIONS_FADEDOWN ;


	return ReturnValue ;
}

//------------------------------------------------------------------------
// EDIT LIGHT
//------------------------------------------------------------------------
INT32 COptions__UpdateLIGHT(COptions *pThis)
{
	float				delta ;
	INT32				ReturnValue = -1 ;

	// Goto next selection?
	if (CEngineApp__MenuDown(GetApp()))
	{
		BarTimer = 0 ;
		pThis->m_Selection++ ;
		if (pThis->m_Selection >= LIGHT_END_SELECTION)
			pThis->m_Selection = 0 ;
	}

	// Goto previous selection?
	if (CEngineApp__MenuUp(GetApp()))
	{
		BarTimer = 0 ;
		pThis->m_Selection-- ;
		if (pThis->m_Selection < 0)
			pThis->m_Selection = LIGHT_END_SELECTION-1 ;
	}

	// check movement on sliders
	delta = (CTControl__IsAnalogRight(pCTControl) - CTControl__IsAnalogLeft(pCTControl)) /4 ;
	delta -= CTControl__IsMapRight(pCTControl) ;
	delta += CTControl__IsMapLeft(pCTControl) ;

	switch (pThis->m_Selection)
	{
		case LIGHT_ARED:
			pThis->m_ARed += delta ;

			if(pThis->m_ARed < 0)
	  			pThis->m_ARed = 0;
 			if(pThis->m_ARed > 255)
 				pThis->m_ARed = 255;
			break ;

		case LIGHT_AGREEN:
			pThis->m_AGreen += delta ;

			if(pThis->m_AGreen < 0)
	  			pThis->m_AGreen = 0;
 			if(pThis->m_AGreen > 255)
 				pThis->m_AGreen = 255;
			break ;

		case LIGHT_ABLUE:
			pThis->m_ABlue += delta ;

			if(pThis->m_ABlue < 0)
	  			pThis->m_ABlue = 0;
 			if(pThis->m_ABlue > 255)
 				pThis->m_ABlue = 255;
			break ;

		case LIGHT_DRED:
			pThis->m_DRed += delta ;

			if(pThis->m_DRed < 0)
	  			pThis->m_DRed = 0;
 			if(pThis->m_DRed > 255)
 				pThis->m_DRed = 255;
			break ;

		case LIGHT_DGREEN:
			pThis->m_DGreen += delta ;

			if(pThis->m_DGreen < 0)
	  			pThis->m_DGreen = 0;
 			if(pThis->m_DGreen > 255)
 				pThis->m_DGreen = 255;
			break ;

		case LIGHT_DBLUE:
			pThis->m_DBlue += delta ;

			if(pThis->m_DBlue < 0)
	  			pThis->m_DBlue = 0;
 			if(pThis->m_DBlue > 255)
 				pThis->m_DBlue = 255;
			break ;
	}

	thelight.l->l.col[0] = pThis->m_DRed ;
	thelight.l->l.col[1] = pThis->m_DGreen ;
	thelight.l->l.col[2] = pThis->m_DBlue ;
	thelight.l->l.colc[0] = pThis->m_DRed ;
	thelight.l->l.colc[1] = pThis->m_DGreen ;
	thelight.l->l.colc[2] = pThis->m_DBlue ;
	thelight.a.l.col[0] = pThis->m_ARed ;
	thelight.a.l.col[1] = pThis->m_AGreen ;
	thelight.a.l.col[2] = pThis->m_ABlue ;
	thelight.a.l.colc[0] = pThis->m_ARed ;
	thelight.a.l.colc[1] = pThis->m_AGreen ;
	thelight.a.l.colc[2] = pThis->m_ABlue ;

	// check for start button on current item
	if (CTControl__IsUseMenu(pCTControl))
		ReturnValue = pThis->m_Selection ;

	if (ReturnValue != -1)
		pThis->m_Mode = OPTIONS_FADEDOWN ;


	return ReturnValue ;
}
#endif



//------------------------------------------------------------------------
// Draw options screen information
//------------------------------------------------------------------------
void COptions__Draw(COptions *pThis, Gfx **ppDLP)
{
	int		i, y ;
	int		slidex, slidey ;
	int		x1 ;
//	float		slider_yoff ;
//	float		slider_scale ;

	// update fade status
	switch (pThis->m_Mode)
	{
		case OPTIONS_FADEUP:
			pThis->m_Alpha += .2 * frame_increment ;
			if (pThis->m_Alpha >= 1.0)
			{
				pThis->m_Alpha = 1.0 ;
				pThis->m_Mode = OPTIONS_NORMAL ;
			}
			break ;

		case OPTIONS_FADEDOWN:
			pThis->m_Alpha -= .2 * frame_increment ;
			if (pThis->m_Alpha <= 0.0)
			{
				pThis->m_Alpha = 0.0 ;
				pThis->m_Mode = OPTIONS_NULL ;
				if (pThis->b_Active == FALSE)
					GetApp()->m_bOptions = FALSE ;
			}
			break ;
	}

#if defined(PLATFORM_PORT) && !defined(PLATFORM_3DS)
	if (s_ControlsActive) { options_controls_draw(pThis, ppDLP) ; return ; }
#endif

#ifdef EDIT_FOG_AND_LIGHTS
	if (pThis->m_EditFog)
		COptions__DrawFOG(pThis, ppDLP) ;
	else if (pThis->m_EditLight)
		COptions__DrawLIGHT(pThis, ppDLP) ;
	else
#endif
	{
		if (pThis->m_Alpha)
		{
			// prepare to draw boxes
			COnScreen__InitBoxDraw(ppDLP) ;

			COnScreen__DrawHilightBox(ppDLP,
							 OPTIONS_X, OPTIONS_Y,
							 OPTIONS_X+OPTIONS_WIDTH, OPTIONS_Y+OPTIONS_HEIGHT,
							 1, FALSE,
							 0,0,0, 180 * pThis->m_Alpha) ;


			// draw polygon selection bar
			y = OPTIONSMENU_Y ;
			i = 0;
			while (i!=pThis->m_Selection)
				y += options[i++].Spacing ;
			CPause__InitPolygon(ppDLP) ;
			CPause__DrawBar(ppDLP, OPTIONS_X+8, y-1,
						OPTIONS_WIDTH-16, OPTIONSMENU_SPACING, pThis->m_Alpha) ;


			//-----------------------------------------------
			// draw text
			//-----------------------------------------------
			COnScreen__InitFontDraw(ppDLP) ;


			if (pThis->m_RHControl)
				options[OPTIONS_CONTROL].String = text_control_right ;
			else
				options[OPTIONS_CONTROL].String = text_control_left ;

#if defined(PLATFORM_PORT) && !defined(PLATFORM_3DS)
			/* resolution: "1920 x 1080" (preset) or "desktop" (native). fullscreen: "fullscreen"/"windowed". */
			if (s_ResIndex >= 0 && s_ResIndex < OPTIONS_RES_COUNT)
				sprintf(s_res_label, "%d x %d", s_res_presets[s_ResIndex][0], s_res_presets[s_ResIndex][1]) ;
			else
				strcpy(s_res_label, "desktop") ;
			options[OPTIONS_RESOLUTION].String = s_res_label ;
			options[OPTIONS_FULLSCREEN].String = g_cfg_fullscreen ? text_fullscreen_on : text_fullscreen_off ;
#endif

#ifdef PLATFORM_3DS
			{	extern int g_cfg_swap_sticks;	/* row shows which stick AIMS: swap 0 = Circle Pad (default, nub moves), swap 1 = C-stick */
				options[OPTIONS_SWAPSTICKS].String = g_cfg_swap_sticks ? text_look_cstick : text_look_cpad ; }
#endif

#ifndef GERMAN
			if (pThis->m_Blood == BLOOD_OFF)
				options[OPTIONS_BLOOD].String = text_blood_off ;
#ifndef KANJI
			else if (pThis->m_Blood == BLOOD_RED)
				options[OPTIONS_BLOOD].String = text_blood_red ;
#endif
			else
				options[OPTIONS_BLOOD].String = text_blood_green ;
#endif

			// draw each option
			COnScreen__SetFontScale(1.0, 0.6) ;
			y = OPTIONSMENU_Y ;
			for (i=0; i<OPTIONS_END_SELECTION; i++)
			{
#ifdef PLATFORM_PORT
				if (options[i].Spacing == 0) continue;  /* hidden row: OG draw-dist, PC control toggle */
#endif
				if (options[i].Spacing == OPTIONSBIG_SPACING)
				{
					if (i == pThis->m_Selection)
						COnScreen__SetFontColor(ppDLP, 200*1.25, 200*1.25, 200*1.25, 100*1.25, 100*1.25, 100*1.25) ;
					else
						COnScreen__SetFontColor(ppDLP, 200*.7, 200*.7, 200*.7, 100*.7, 100*.7, 100*.7) ;
				}
				else
				{
					if (i == pThis->m_Selection)
						COnScreen__SetFontColor(ppDLP, 200*1.25, 200*1.25, 138*1.25, 86*1.25, 71*1.25, 47*1.25) ;
					else
						COnScreen__SetFontColor(ppDLP, 200*.9, 200*.9, 138*.9, 86*.9, 71*.9, 47*.9) ;
				}

				COnScreen__DrawText(ppDLP,
										  options[i].String,
										  320/2, y,
										  (int)(255 * pThis->m_Alpha), TRUE, TRUE) ;

				y += options[i].Spacing ;

				//if (i == pThis->m_Selection)
				//	COnScreen__SetFontColor(ppDLP, 200*.9, 200*.9, 138*.9, 86*.9, 71*.9, 47*.9) ;
			}



			//-----------------------------------------------
			// draw sliders
			//-----------------------------------------------
			COnScreen__Init16BitDraw(ppDLP, 255 * pThis->m_Alpha) ;

			// MUSIC
			// bar
			slidex = pThis->m_MusicVolume *118 / 255;
			slidey = OPTIONSMENU_Y + 6 ;
			x1 = (320/2)-(OPTIONS_BAR_WIDTH/2) ;

			COnScreen__Draw16BitGraphic(ppDLP,
												(C16BitGraphic *)BarOverlay,
												x1, slidey+5) ;
			// slider
			//slider_scale = 0.8 + (0.2*sin((float)slidex * M_PI / 118)) ;
			//slider_yoff = (((C16BitGraphic *)SliderOverlay)->m_Height * slider_scale) / 2 ;
			//COnScreen__Draw16BitScaledGraphic(ppDLP,
			//									(C16BitGraphic *)SliderOverlay,
			//									x1+slidex+2, slidey-slider_yoff+12,
			//									slider_scale, slider_scale) ;
			COnScreen__Draw16BitGraphic(ppDLP,
												(C16BitGraphic *)SliderOverlay,
												x1+slidex+2, slidey-3) ;


			// SFX
			// bar
			slidex = pThis->m_SFXVolume *118 / 255;
			slidey += options[1].Spacing ;

			COnScreen__Draw16BitGraphic(ppDLP,
												(C16BitGraphic *)BarOverlay,
												x1, slidey+5) ;
			// slider
			COnScreen__Draw16BitGraphic(ppDLP,
												(C16BitGraphic *)SliderOverlay,
												x1+slidex+2, slidey-3) ;

			// OPACITY
			// bar
			slidex = pThis->m_Opacity *118 / 255;
			slidey += options[2].Spacing ;

			COnScreen__Draw16BitGraphic(ppDLP,
												(C16BitGraphic *)BarOverlay,
												x1, slidey+5) ;
			// slider
			COnScreen__Draw16BitGraphic(ppDLP,
												(C16BitGraphic *)SliderOverlay,
												x1+slidex+2, slidey-3) ;

			
			// H ANALOG
			// bar
			slidex = pThis->m_HAnalog *118 / 255;
			slidey += options[3].Spacing ;

			COnScreen__Draw16BitGraphic(ppDLP,
												(C16BitGraphic *)BarOverlay,
												x1, slidey+5) ;
			// slider
			COnScreen__Draw16BitGraphic(ppDLP,
												(C16BitGraphic *)SliderOverlay,
												x1+slidex+2, slidey-3) ;


			// V ANALOG
			// bar
			slidex = pThis->m_VAnalog *118 / 255;
			slidey += options[4].Spacing ;

			COnScreen__Draw16BitGraphic(ppDLP,
												(C16BitGraphic *)BarOverlay,
												x1, slidey+5) ;
			// slider
			COnScreen__Draw16BitGraphic(ppDLP,
												(C16BitGraphic *)SliderOverlay,
												x1+slidex+2, slidey-3) ;




#ifdef PLATFORM_PORT
			// DRAW DISTANCE (PC+3DS port slider) — hidden on OG 3DS (Spacing collapsed to 0)
			if (options[OPTIONS_DRAWDIST].Spacing != 0)
			{
				slidex = s_DrawDistSlider *118 / 255;
				slidey += options[OPTIONS_DRAWDIST].Spacing ;
				COnScreen__Draw16BitGraphic(ppDLP, (C16BitGraphic *)BarOverlay, x1, slidey+5) ;
				COnScreen__Draw16BitGraphic(ppDLP, (C16BitGraphic *)SliderOverlay, x1+slidex+2, slidey-3) ;
			}
#endif

			// draw heading
			COnScreen__Draw16BitGraphic(ppDLP,
												(C16BitGraphic *)OptionsOverlay,
												320/2 - (((C16BitGraphic *)OptionsOverlay)->m_Width/2), OPTIONS_Y+6) ;


		}
	}
}

#ifdef EDIT_FOG_AND_LIGHTS
//--------------------------------------------------------------------------------------------------
void COptions__DrawFOG(COptions *pThis, Gfx **ppDLP)
{
	int		i, y ;
	int		slidex, slidey ;
	int		x1 ;


	// prepare to draw boxes
	COnScreen__InitBoxDraw(ppDLP) ;


	// draw current selection box
	y = OPTIONSMENU_Y ;
	i = 0;
	while (i!=pThis->m_Selection)
		y += FOGoptions[i++].Spacing ;
	CPause__InitPolygon(ppDLP) ;
	CPause__DrawBar(ppDLP, OPTIONS_X+8, y-2,
					OPTIONS_WIDTH-16, OPTIONSMENU_SPACING, pThis->m_Alpha) ;



	//-----------------------------------------------
	// draw text
	//-----------------------------------------------
	COnScreen__InitFontDraw(ppDLP) ;

	// draw heading
	COnScreen__SetFontScale(1.0, 2.0) ;
	COnScreen__SetFontColor(ppDLP,
									200, 0, 200,
									0, 200, 0) ;
	COnScreen__DrawText(ppDLP,
							  "fog",
							  320/2, 40,
							  (int)(255 * pThis->m_Alpha), TRUE, TRUE) ;

	sprintf(redstring, "%d red", pThis->m_Red) ;
	sprintf(greenstring, "%d green", pThis->m_Green) ;
	sprintf(bluestring, "%d blue", pThis->m_Blue) ;


	// draw each option
	COnScreen__SetFontScale(1.0, 0.7) ;
	COnScreen__SetFontColor(ppDLP, 200*.9, 200*.9, 138*.9, 86*.9, 71*.9, 47*.9) ;
	y = OPTIONSMENU_Y ;
	for (i=0; i<FOG_END_SELECTION; i++)
	{
		// set selected item brighter
		if (i == pThis->m_Selection)
			COnScreen__SetFontColor(ppDLP, 200*1.25, 200*1.25, 138*1.25, 86*1.25, 71*1.25,47*1.25) ;

		COnScreen__DrawText(ppDLP,
								  FOGoptions[i].String,
								  320/2, y,
								  (int)(255 * pThis->m_Alpha), TRUE, TRUE) ;

		y += FOGoptions[i].Spacing ;

		// set back to normal color
		if (i == pThis->m_Selection)
			COnScreen__SetFontColor(ppDLP, 200*.9, 200*.9, 138*.9, 86*.9, 71*.9, 47*.9) ;

	}

	
	//-----------------------------------------------
	// draw sliders
	//-----------------------------------------------
	COnScreen__Init16BitDraw(ppDLP, 255 * pThis->m_Alpha) ;
	// RED
	// bar
	slidex = pThis->m_Red *118 / 255;
	slidey = OPTIONSMENU_Y + 12 ;
	x1 = (320/2)-(OPTIONS_BAR_WIDTH/2) ;

	COnScreen__Draw16BitGraphic(ppDLP,
										(C16BitGraphic *)BarOverlay,
										x1, slidey+5) ;
	// slider
	COnScreen__Draw16BitGraphic(ppDLP,
										(C16BitGraphic *)SliderOverlay,
										x1+slidex+2, slidey-3) ;

	// GREEN
	// bar
	slidex = pThis->m_Green *118 / 255;
	slidey += FOGoptions[0].Spacing ;

	COnScreen__Draw16BitGraphic(ppDLP,
										(C16BitGraphic *)BarOverlay,
										x1, slidey+5) ;
	// slider
	COnScreen__Draw16BitGraphic(ppDLP,
										(C16BitGraphic *)SliderOverlay,
										x1+slidex+2, slidey-3) ;

	// BLUE
	// bar
	slidex = pThis->m_Blue *118 / 255;
	slidey += FOGoptions[1].Spacing ;

	COnScreen__Draw16BitGraphic(ppDLP,
										(C16BitGraphic *)BarOverlay,
										x1, slidey+5) ;
	// slider
	COnScreen__Draw16BitGraphic(ppDLP,
										(C16BitGraphic *)SliderOverlay,
										x1+slidex+2, slidey-3) ;

	

}


//--------------------------------------------------------------------------------------------------
void COptions__DrawLIGHT(COptions *pThis, Gfx **ppDLP)
{
	int		i, y ;
	int		slidex, slidey ;
	int		x1 ;


	// prepare to draw boxes
	COnScreen__InitBoxDraw(ppDLP) ;


	// draw current selection box
	y = OPTIONSMENU_Y ;
	i = 0;
	while (i!=pThis->m_Selection)
		y += LIGHToptions[i++].Spacing ;
	CPause__InitPolygon(ppDLP) ;
	CPause__DrawBar(ppDLP, OPTIONS_X+8, y-2,
						OPTIONS_WIDTH-16, OPTIONSMENU_SPACING, pThis->m_Alpha) ;


	//-----------------------------------------------
	// draw text
	//-----------------------------------------------
	COnScreen__InitFontDraw(ppDLP) ;

	// draw heading
	COnScreen__SetFontScale(1.0, 2.0) ;
	COnScreen__SetFontColor(ppDLP,
									200, 0, 200,
									0, 200, 0) ;
	COnScreen__DrawText(ppDLP,
							  "light",
							  320/2, 40,
							  (int)(255 * pThis->m_Alpha), TRUE, TRUE) ;


	sprintf(aredstring, "%d ared", pThis->m_ARed) ;
	sprintf(agreenstring, "%d agreen", pThis->m_AGreen) ;
	sprintf(abluestring, "%d ablue", pThis->m_ABlue) ;
	sprintf(dredstring, "%d dred", pThis->m_DRed) ;
	sprintf(dgreenstring, "%d dgreen", pThis->m_DGreen) ;
	sprintf(dbluestring, "%d dblue", pThis->m_DBlue) ;


	// draw each option
	COnScreen__SetFontScale(1.0, 0.6) ;
	COnScreen__SetFontColor(ppDLP, 200*.9, 200*.9, 138*.9, 86*.9, 71*.9, 47*.9) ;
	y = OPTIONSMENU_Y ;
	for (i=0; i<LIGHT_END_SELECTION; i++)
	{
		if (i == pThis->m_Selection)
			COnScreen__SetFontColor(ppDLP, 200*1.25, 200*1.25, 138*1.25, 86*1.25, 71*1.25, 47*1.25) ;

		COnScreen__DrawText(ppDLP,
								  LIGHToptions[i].String,
								  320/2, y,
								  (int)(255 * pThis->m_Alpha), TRUE, TRUE) ;

		y += LIGHToptions[i].Spacing ;

		if (i == pThis->m_Selection)
			COnScreen__SetFontColor(ppDLP, 200*.9, 200*.9, 138*.9, 86*.9, 71*.9, 47*.9) ;
	}

	//-----------------------------------------------
	// draw sliders
	//-----------------------------------------------
	COnScreen__Init16BitDraw(ppDLP, 255 * pThis->m_Alpha) ;
	// ARED
	// bar
	slidex = pThis->m_ARed *118 / 255;
	slidey = OPTIONSMENU_Y + 10 ;
	x1 = (320/2)-(OPTIONS_BAR_WIDTH/2) ;

	COnScreen__Draw16BitGraphic(ppDLP,
										(C16BitGraphic *)BarOverlay,
										x1, slidey+5) ;
	// slider
	COnScreen__Draw16BitGraphic(ppDLP,
										(C16BitGraphic *)SliderOverlay,
										x1+slidex+2, slidey) ;

	// AGREEN
	// bar
	slidex = pThis->m_AGreen *118 / 255;
	slidey += LIGHToptions[0].Spacing ;

	COnScreen__Draw16BitGraphic(ppDLP,
										(C16BitGraphic *)BarOverlay,
										x1, slidey+5) ;
	// slider
	COnScreen__Draw16BitGraphic(ppDLP,
										(C16BitGraphic *)SliderOverlay,
										x1+slidex+2, slidey) ;

	// ABLUE
	// bar
	slidex = pThis->m_ABlue *118 / 255;
	slidey += LIGHToptions[1].Spacing ;

	COnScreen__Draw16BitGraphic(ppDLP,
										(C16BitGraphic *)BarOverlay,
										x1, slidey+5) ;
	// slider
	COnScreen__Draw16BitGraphic(ppDLP,
										(C16BitGraphic *)SliderOverlay,
										x1+slidex+2, slidey) ;

	// DRED
	// bar
	slidex = pThis->m_DRed *118 / 255;
	slidey += LIGHToptions[2].Spacing ;

	COnScreen__Draw16BitGraphic(ppDLP,
										(C16BitGraphic *)BarOverlay,
										x1, slidey+5) ;
	// slider
	COnScreen__Draw16BitGraphic(ppDLP,
										(C16BitGraphic *)SliderOverlay,
										x1+slidex+2, slidey) ;
	// DGREEN
	// bar
	slidex = pThis->m_DGreen *118 / 255;
	slidey += LIGHToptions[3].Spacing ;

	COnScreen__Draw16BitGraphic(ppDLP,
										(C16BitGraphic *)BarOverlay,
										x1, slidey+5) ;
	// slider
	COnScreen__Draw16BitGraphic(ppDLP,
										(C16BitGraphic *)SliderOverlay,
										x1+slidex+2, slidey) ;


	// DBLUE
	// bar
	slidex = pThis->m_DBlue *118 / 255;
	slidey += LIGHToptions[4].Spacing ;

	COnScreen__Draw16BitGraphic(ppDLP,
										(C16BitGraphic *)BarOverlay,
										x1, slidey+5) ;
	// slider
	COnScreen__Draw16BitGraphic(ppDLP,
										(C16BitGraphic *)SliderOverlay,
										x1+slidex+2, slidey) ;

}
#endif










/////////////////////////////////////////////////////////////////////////////
// Constants
/////////////////////////////////////////////////////////////////////////////

// Position and dimentions of keys screen
#define KEYS_WIDTH	(220)
#define KEYS_HEIGHT	(210)
#define KEYS_X			((320/2) - (KEYS_WIDTH/2))
#define KEYS_Y			((240/2) - (KEYS_HEIGHT/2))


// Position of menu selection
#define KEYSMENU_Y 			(KEYS_Y + KEYS_HEIGHT - 20)

char *keyoptions[]=
{
	text_exit
} ;

/*int ConvertKey(int BitKey)
{
	int	count = 0;

	while (BitKey != 0)
	{
		if (BitKey & 1)
			count++ ;
		BitKey >>= 1 ;
	}
	return count ;
}
*/
/////////////////////////////////////////////////////////////////////////////
// Functions
/////////////////////////////////////////////////////////////////////////////

void CKeys__Construct(CKeys *pThis)
{
	if (pThis->m_Mode == KEYS_NULL)
		pThis->m_Alpha = 0 ;

	pThis->m_Mode = KEYS_FADEUP ;

	pThis->b_Active = TRUE ;
}



//------------------------------------------------------------------------
// Update selection on keys screen
//------------------------------------------------------------------------
s32 CKeys__Update(CKeys *pThis)
{
	INT32				ReturnValue = -1 ;

	// check for start button on current item
	if (CTControl__IsUseMenu(pCTControl))
	{
		pThis->m_Mode = KEYS_FADEDOWN ;
		ReturnValue = 0 ;
	}

	return ReturnValue ;
}


//------------------------------------------------------------------------
// Draw keys screen information
//------------------------------------------------------------------------

typedef	struct	s_LevelKey
{
	C16BitGraphic	*Graphic ;
	int				*Variable ;
	int				Bit ;
} t_LevelKey ;


t_LevelKey	Level1Keys[] =
{
	{(C16BitGraphic *)ChronoOverlay,			&CTurokMovement.ChronoSceptorPieces,		0},
	{(C16BitGraphic *)Key2Overlay,			&CTurokMovement.Level2Keys,			0},
	{(C16BitGraphic *)Key2Overlay,			&CTurokMovement.Level2Keys,			1},
	{(C16BitGraphic *)Key2Overlay,			&CTurokMovement.Level2Keys,			2},
	{(C16BitGraphic *)Key3Overlay,			&CTurokMovement.Level3Keys,			0},
	{(C16BitGraphic *)Key3Overlay,			&CTurokMovement.Level3Keys,			1},
	{(C16BitGraphic *)Key3Overlay,			&CTurokMovement.Level3Keys,			2},
	{0}
} ;

t_LevelKey	Level2Keys[] =
{
	{(C16BitGraphic *)ChronoOverlay,			&CTurokMovement.ChronoSceptorPieces,		1},
	{(C16BitGraphic *)Key4Overlay,			&CTurokMovement.Level4Keys,			0},
	{(C16BitGraphic *)Key4Overlay,			&CTurokMovement.Level4Keys,			1},
	{(C16BitGraphic *)Key5Overlay,			&CTurokMovement.Level5Keys,			2},
	{0}
} ;

t_LevelKey	Level3Keys[] =
{
	{(C16BitGraphic *)ChronoOverlay,			&CTurokMovement.ChronoSceptorPieces,		2},
	{(C16BitGraphic *)Key4Overlay,			&CTurokMovement.Level4Keys,			2},
	{(C16BitGraphic *)Key5Overlay,			&CTurokMovement.Level5Keys,			0},
	{(C16BitGraphic *)Key5Overlay,			&CTurokMovement.Level5Keys,			1},
	{0}
} ;

t_LevelKey	Level4Keys[] =
{
	{(C16BitGraphic *)ChronoOverlay,			&CTurokMovement.ChronoSceptorPieces,		3},
	{(C16BitGraphic *)Key6Overlay,			&CTurokMovement.Level6Keys,			0},
	{(C16BitGraphic *)Key6Overlay,			&CTurokMovement.Level6Keys,			1},
	{(C16BitGraphic *)Key8Overlay,			&CTurokMovement.Level8Keys,			0},
	{0}
} ;

t_LevelKey	Level5Keys[] =
{
	{(C16BitGraphic *)ChronoOverlay,			&CTurokMovement.ChronoSceptorPieces,		4},
	{(C16BitGraphic *)Key6Overlay,			&CTurokMovement.Level6Keys,			2},
	{(C16BitGraphic *)Key8Overlay,			&CTurokMovement.Level8Keys,			1},
	{(C16BitGraphic *)Key8Overlay,			&CTurokMovement.Level8Keys,			2},
	{0}
} ;

t_LevelKey	Level6Keys[] =
{
	{(C16BitGraphic *)ChronoOverlay,			&CTurokMovement.ChronoSceptorPieces,		5},
	{(C16BitGraphic *)Key7Overlay,			&CTurokMovement.Level7Keys,			0},
	{(C16BitGraphic *)Key7Overlay,			&CTurokMovement.Level7Keys,			1},
	{(C16BitGraphic *)Key7Overlay,			&CTurokMovement.Level7Keys,			2},
	{0}
} ;

t_LevelKey	Level7Keys[] =
{
	{(C16BitGraphic *)ChronoOverlay,			&CTurokMovement.ChronoSceptorPieces,		6},
	{(C16BitGraphic *)Key8Overlay,			&CTurokMovement.Level8Keys,			3},
	{(C16BitGraphic *)Key8Overlay,			&CTurokMovement.Level8Keys,			4},
	{0}
} ;

t_LevelKey	Level8Keys[] =
{
	{(C16BitGraphic *)ChronoOverlay,			&CTurokMovement.ChronoSceptorPieces,		7},
	{0}
} ;



void DrawLevelKeys(CKeys *pThis, Gfx **ppDLP, t_LevelKey *Array, int y)
{
	float		x ;
	int		i ;

	COnScreen__Init16BitDraw(ppDLP, 255 * pThis->m_Alpha) ;

	i = 0 ; 
	while (Array->Graphic != 0)
	{
		if (i==0)
			x = KEYS_X + 60 ;
		else
			x = KEYS_X + 68 + (i*20) ;

		gDPPipeSync((*ppDLP)++);
		if (*Array->Variable & (1<<Array->Bit))
		{
			gDPSetEnvColor((*ppDLP)++, 255, 255, 255, 255*pThis->m_Alpha) ;
		}
		else
		{
			gDPSetEnvColor((*ppDLP)++, 55, 55, 255, 200*pThis->m_Alpha) ;
		}

		COnScreen__Draw16BitGraphic(ppDLP,
											Array->Graphic,
											x, y) ;

		i++ ;
		Array++ ;
	}
}



void CKeys__Draw(CKeys *pThis, Gfx **ppDLP)
{
	int		i, y ;
	char		buffer[8] ;

	// update fade status
	switch (pThis->m_Mode)
	{
		case KEYS_FADEUP:
			pThis->m_Alpha += .2 * frame_increment ;
			if (pThis->m_Alpha >= 1.0)
			{
				pThis->m_Alpha = 1.0 ;
				pThis->m_Mode = KEYS_NORMAL ;
			}
			break ;

		case KEYS_FADEDOWN:
			pThis->m_Alpha -= .2 * frame_increment ;
			if (pThis->m_Alpha <= 0.0)
			{
				pThis->m_Alpha = 0.0 ;
				pThis->m_Mode = KEYS_NULL ;
				if (pThis->b_Active == FALSE)
					GetApp()->m_bKeys = FALSE ;
			}
			break ;
	}

	if (pThis->m_Alpha)
	{
		// prepare to draw boxes
		COnScreen__InitBoxDraw(ppDLP) ;

		COnScreen__DrawHilightBox(ppDLP,
						 KEYS_X, KEYS_Y,
						 KEYS_X+KEYS_WIDTH, KEYS_Y+KEYS_HEIGHT,
						 1, FALSE,
						 0,0,0, 150 * pThis->m_Alpha) ;


		// draw polygon selection bar
		y = KEYSMENU_Y ;
		CPause__InitPolygon(ppDLP) ;
		CPause__DrawBar(ppDLP, KEYS_X+8, y-1,
					KEYS_WIDTH-16, 18, pThis->m_Alpha) ;


		// draw icons
		DrawLevelKeys(pThis, ppDLP, Level1Keys, KEYS_Y+24+(20*0)) ;
		DrawLevelKeys(pThis, ppDLP, Level2Keys, KEYS_Y+24+(20*1)) ;
		DrawLevelKeys(pThis, ppDLP, Level3Keys, KEYS_Y+24+(20*2)) ;
		DrawLevelKeys(pThis, ppDLP, Level4Keys, KEYS_Y+24+(20*3)) ;
		DrawLevelKeys(pThis, ppDLP, Level5Keys, KEYS_Y+24+(20*4)) ;
		DrawLevelKeys(pThis, ppDLP, Level6Keys, KEYS_Y+24+(20*5)) ;
		DrawLevelKeys(pThis, ppDLP, Level7Keys, KEYS_Y+24+(20*6)) ;
		DrawLevelKeys(pThis, ppDLP, Level8Keys, KEYS_Y+24+(20*7)) ;
		
		//-----------------------------------------------
		// draw text
		//-----------------------------------------------
		COnScreen__InitFontDraw(ppDLP) ;

		// write headings
		COnScreen__SetFontColor(ppDLP, 50,255,50, 255,255,50) ;
		COnScreen__DrawText(ppDLP,
								  text_levelicons,
								  320/2, KEYS_Y+4,
								  (int)(255 * pThis->m_Alpha), TRUE, TRUE) ;

		// write level numbers
		COnScreen__PrepareLargeFont(ppDLP) ;
		for (i=0; i<8; i++)
		{
			sprintf(buffer, "%d", i+1) ;
			COnScreen__DrawText(ppDLP,
									  buffer,
									  KEYS_X+32, KEYS_Y+24+(i*20),
									  (int)(255 * pThis->m_Alpha), FALSE, TRUE) ;
		}


		// draw each option
		COnScreen__SetFontColor(ppDLP, 200*1.25, 200*1.25, 138*1.25, 86*1.25, 71*1.25, 47*1.25) ;
		y = KEYSMENU_Y ;
#ifdef KANJI
		COnScreen__PrepareKanjiFont(ppDLP);
#endif
		for (i=0; i<KEYS_END_SELECTION; i++)
		{
			COnScreen__DrawText(ppDLP,
									  keyoptions[i],
									  320/2, KEYSMENU_Y,
									  (int)(255 * pThis->m_Alpha), TRUE, TRUE) ;

		}

	}
}

