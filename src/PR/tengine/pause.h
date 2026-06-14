// pause.h
/////////////////////////////////////////////////////////////////////////////

#ifndef _INC_PAUSE
#define _INC_PAUSE

#include "tcontrol.h"
#include "loadsave.h"
#include "options.h"
#include "cheats.h"

/////////////////////////////////////////////////////////////////////////////
// Constants
/////////////////////////////////////////////////////////////////////////////

enum PauseSelections
{
	PAUSE_RESUME,
	PAUSE_OPTIONS,
	PAUSE_KEYS,
#ifdef EDIT_FOG_AND_LIGHTS
	PAUSE_EDITFOG,
	PAUSE_EDITLIGHT,
#endif

	PAUSE_LOAD,
#ifdef PAUSE_SAVE
	PAUSE_SAVE,
#endif
//	PAUSE_PREVLEVEL,
//	PAUSE_NEXTLEVEL,
#ifndef SHIP_IT
	PAUSE_DEBUG,
#endif
	PAUSE_ENTERCHEAT,
	PAUSE_CHEATMENU,
	PAUSE_RESTART_GAME,
	PAUSE_END_SELECTION
} ;

enum TitleSelections
{
	TITLE_START,
	TITLE_LOAD,
	TITLE_TRAINING,
	TITLE_OPTIONS,
	TITLE_DIFFICULTY,
	TITLE_ENTERCHEAT,
	TITLE_CHEATMENU,
	TITLE_END_SELECTION
} ;



enum PauseModes
{
	PAUSE_NULL,
	PAUSE_FADEUP,
	PAUSE_NORMAL,
	PAUSE_FADEDOWN
} ;



/////////////////////////////////////////////////////////////////////////////
// Pause structures
/////////////////////////////////////////////////////////////////////////////

// Overlay position structure
typedef struct CPause_t
{
	// Mode vars
	INT32				m_Mode ;					// state of fade
	float				m_Alpha ;				// 0..1

	// Selection vars
	INT32				m_Selection ;			// Current selection
	int				m_RealSelection;

//	float				m_MusicVolume,
//						m_SFXVolume;

	// Screen bitmap data pointers								
	//UINT16			*m_pPauseScreen ;			// Pointer to pause screen data
	//UINT16			*m_pPauseScreenArrow ;		// Pointer to pause screen arrow data

} CPause ;


/////////////////////////////////////////////////////////////////////////////
// CPause operations
/////////////////////////////////////////////////////////////////////////////
extern	float		BarTimer ;

void CPause__Construct(CPause *pThis) ;
INT32 CPause__Update(CPause *pThis) ;
void CPause__Draw(CPause *pThis, Gfx **ppDLP) ;

//INT32 CPause__Update(CPause *pThis, void *pBuffer) ;

void CPause__DrawRock(Gfx **ppDLP, int x, int y, float xscale, float yscale, int opacity) ;


void CPause__InitPolygon(Gfx **ppDLP) ;
void CPause__DrawBar(Gfx **ppDLP, float x, float y, float w, float h, float a) ;

/////////////////////////////////////////////////////////////////////////////
#endif // _INC_PAUSE
