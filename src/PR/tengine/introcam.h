// introcam.h
/////////////////////////////////////////////////////////////////////////////

#ifndef _INC_INTROCAM
#define _INC_INTROCAM

#include "camera.h"


/////////////////////////////////////////////////////////////////////////////
// Externs
/////////////////////////////////////////////////////////////////////////////
extern	BOOL	SkipIntro ;
extern	BOOL	IntroFirstRun ;


/////////////////////////////////////////////////////////////////////////////
// Function prototypes
/////////////////////////////////////////////////////////////////////////////
void CCamera__ConstructIntro(CCamera *pThis) ;

void CCamera__Setup_LEGALSCREEN_LOGO(CCamera *pThis) ;
void CCamera__Code_LEGALSCREEN_LOGO(CCamera *pThis) ;

void CCamera__Setup_INTRO_ACCLAIM_LOGO(CCamera *pThis) ;
void CCamera__Code_INTRO_ACCLAIM_LOGO(CCamera *pThis) ;

void CCamera__Setup_INTRO_IGGY(CCamera *pThis) ;
void CCamera__Code_INTRO_IGGY(CCamera *pThis) ;

void CCamera__Setup_INTRO_TUROK_DRAWING_BOW(CCamera *pThis) ;
void CCamera__Code_INTRO_TUROK_DRAWING_BOW(CCamera *pThis) ;

void CCamera__Setup_INTRO_ZOOM_TO_LOGO(CCamera *pThis) ;
void CCamera__Code_INTRO_ZOOM_TO_LOGO(CCamera *pThis) ;

CQuatern CGameObjectInstance__CinemaTurokArrowOrientation(CGameObjectInstance *pThis) ;

void CCamera__Setup_GAME_INTRO(CCamera *pThis) ;
void CCamera__Code_GAME_INTRO(CCamera *pThis) ;

void CCamera__Setup_GAME_KEY_INTRO(CCamera *pThis) ;
void CCamera__Code_GAME_KEY_INTRO(CCamera *pThis) ;


/////////////////////////////////////////////////////////////////////////////


#endif // _INC_INTROCAM

