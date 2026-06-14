// Intro camera control file by Stephen Broumley

#include "cppu64.h"

#ifndef MAKE_CART
#include <PR/ramrom.h>	// needed for argument passing into the app
#endif

#include "tengine.h"
#include "dlists.h"
#include "scaling.h"
#include "particle.h"
#include "mattable.h"
#include "tcontrol.h"
#include "tmove.h"
#include "audio.h"
#include "sfx.h"
#include "textload.h"
#include "regtype.h"


#include "introcam.h"
#include "cammodes.h"
#include "camera.h"
#include "fx.h"



// NOTE 3 EVENTS ON BOW DRAW - START FOV CHANGE, END FOV CHANGE, END CINEMA


/////////////////////////////////////////////////////////////////////////////
// Constants
/////////////////////////////////////////////////////////////////////////////
#define DO_INTRO			1		// Set to one to activate intro



/////////////////////////////////////////////////////////////////////////////
// Global Variables
/////////////////////////////////////////////////////////////////////////////
BOOL	SkipIntro = FALSE ;
BOOL	IntroFirstRun = TRUE ;


/////////////////////////////////////////////////////////////////////////////
// Text
/////////////////////////////////////////////////////////////////////////////
#ifdef ENGLISH
// ---------------------- English text ---------------------------
static char	text_locatekey1[] = {"locate the keys"} ;
static char	text_locatekey2[] = {"hidden in each level"} ;

static char	text_hub1[] = {"locate the hub ruins"} ;
static char	text_hub2[] = {"use the keys"} ;
static char	text_hub3[] = {"to open level portals"} ;
#endif

#ifdef GERMAN
// ---------------------- German text ---------------------------
static char	text_locatekey1[] = {"finde die schl/ssel"} ;		//u.
static char	text_locatekey2[] = {"die in jedem level versteckt sind"} ;

static char	text_hub1[] = {"finde das ruinenzentrum"} ;
static char	text_hub2[] = {"und benutze die schl/ssel,"} ;		//u.
static char	text_hub3[] = {"um die leveltore zu >ffnen"} ;		//o.
#endif


#ifdef KANJI
// ---------------------- Japanese text ---------------------------
static char	text_locatekey1[] = {0x9C, 0x26, 0x41, 0x23, 0x2F, 0x7A, -1};
static char	text_locatekey2[] = {0x82, 0x83, 0x93, 0x74, 0x9D, 0x59, 0x81, -1};

static char	text_hub1[] = {0x94, 0x95, 0x96, 0x60, 0x97, 0x69, 0x98, 0x82, 0x83, 0x93, 0x74, 0x4C, 0x5C, 0x6A, -1};
static char	text_hub2[] = {0x99, 0x9A, 0x63, 0x9B, 0x74, 0x53, 0x78, 0x71, 0xE3, 0xE3, 0xE3, -1};
#endif





/*****************************************************************************
*
*	Function:		CCamera__ConstructIntro()
*
******************************************************************************
*
*	Description:	Runs correct intro level
*
*	Inputs:			*pThis	-	Ptr to camera
*
*	Outputs:			None
*
*****************************************************************************/
void CCamera__ConstructIntro(CCamera *pThis)
{
	CEngineApp	*pEngine = GetApp() ;


	// Just incase no intro's are loaded, but a boss level is
	pEngine->m_pBossActive = NULL ;

	// Setup appropriate intro
	switch(GetApp()->m_WarpID)
	{
		case LEGALSCREEN_WARP_ID:
			CCamera__SetupMode(pThis, CAMERA_LEGALSCREEN_MODE) ;
			break ;
		case INTRO_ACCLAIM_LOGO_WARP_ID:
			CCamera__SetupMode(pThis, CAMERA_INTRO_ACCLAIM_LOGO_MODE) ;
			break ;
		case INTRO_IGGY_WARP_ID:
			CCamera__SetupMode(pThis, CAMERA_INTRO_IGGY_MODE) ;
			break ;
		case INTRO_TUROK_DRAWING_BOW_WARP_ID:
			CCamera__SetupMode(pThis, CAMERA_INTRO_TUROK_DRAWING_BOW_MODE) ;
			break ;
		case INTRO_ZOOM_TO_LOGO_WARP_ID:
			CCamera__SetupMode(pThis, CAMERA_INTRO_ZOOM_TO_LOGO_MODE) ;
			break ;
		case GAME_INTRO_KEY_WARP_ID:
			CCamera__SetupMode(pThis, CAMERA_GAME_INTRO_KEY_MODE) ;
			break ;
		case GAME_INTRO_WARP_ID:
			CCamera__SetupMode(pThis, CAMERA_GAME_INTRO_MODE) ;
			break ;
	}

	// Fudge to stop background flash before cinema
	pEngine->m_FogColor[0] = 0 ;
	pEngine->m_FogColor[1] = 0 ;
	pEngine->m_FogColor[2] = 0 ;
	pEngine->m_FogColor[3] = 0 ;
}



void CCamera__GotoNextIntroMode(CCamera *pThis)
{
	CEngineApp	*pEngine = GetApp() ;
 
	// Skipping the intro?
	if ((pThis->m_StartPressed) && (!IntroFirstRun))
	{
		pEngine->m_FadeFast = TRUE ;
		pEngine->m_WarpID = INTRO_ZOOM_TO_LOGO_WARP_ID ;
		SkipIntro = TRUE ;
		GetApp()->m_ModeTime = 0 ;
	}
	else
	{
		// Goto next level
		switch(pEngine->m_WarpID)
		{
			case INTRO_ACCLAIM_LOGO_WARP_ID:
				pEngine->m_WarpID = INTRO_IGGY_WARP_ID ;
				break ;

			case INTRO_IGGY_WARP_ID:
				pEngine->m_WarpID = INTRO_TUROK_DRAWING_BOW_WARP_ID ;
				break ;

			case INTRO_TUROK_DRAWING_BOW_WARP_ID:
				pEngine->m_FadeFast = TRUE ;
				pEngine->m_WarpID = INTRO_ZOOM_TO_LOGO_WARP_ID ;
				break ;
		}

	}

	CEngineApp__SetupFadeTo(pEngine, MODE_RESETLEVEL) ;
}





/////////////////////////////////////////////////////////////////////////////
//	Modes:	  	 CAMERA_LEGALSCREEN_MODE
//	Description: Showing the turok logo
/////////////////////////////////////////////////////////////////////////////
void CCamera__Setup_LEGALSCREEN_LOGO(CCamera *pThis)
{
	pThis->m_Mode = CAMERA_LEGALSCREEN_MODE ;
	pThis->m_CinemaMode = TRUE ;
	pThis->m_LetterBoxScale = 0 ;

	// setup start position
	pThis->m_View.m_vEye.x = 0*SCALING_FACTOR ;
	pThis->m_View.m_vEye.y = 0*SCALING_FACTOR ;
#ifndef KANJI
	pThis->m_View.m_vEye.z = -25*SCALING_FACTOR ;
#else
	pThis->m_View.m_vEye.z = -24*SCALING_FACTOR ;
#endif
	
	pThis->m_View.m_vFocus.x = 0*SCALING_FACTOR ;
	pThis->m_View.m_vFocus.y = -3*SCALING_FACTOR ;
	pThis->m_View.m_vFocus.z = 0*SCALING_FACTOR ;
}

void CCamera__Code_LEGALSCREEN_LOGO(CCamera *pThis)
{
}





/////////////////////////////////////////////////////////////////////////////
//	Modes:	  	 CAMERA_INTRO_ACCLAIM_LOGO_MODE
//	Description: Showing the old cheap'n'nasty acclaim logo
/////////////////////////////////////////////////////////////////////////////

#define CAMERA_INTRO_ACCLAIM_LOGO_TOTAL_TIME	SECONDS_TO_FRAMES(7)
#define CAMERA_INTRO_ACCLAIM_LOGO_TIME			SECONDS_TO_FRAMES(2.8)

//#define CAMERA_INTRO_ACCLAIM_LOGO_TOTAL_TIME	SECONDS_TO_FRAMES(8)
//#define CAMERA_INTRO_ACCLAIM_LOGO_TIME			SECONDS_TO_FRAMES(6)



// eye,rotation
CCameraPosTrackInfo AcclaimLogoEyeAnglePosTrack = 
{
	{-25*SCALING_FACTOR,		-2*SCALING_FACTOR,	-10*SCALING_FACTOR},	// Start eye
	{-1.0*SCALING_FACTOR,	-2*SCALING_FACTOR,	-26*SCALING_FACTOR},	// Final eye

	{-1.0*SCALING_FACTOR,	-2*SCALING_FACTOR,	0*SCALING_FACTOR},	// Start focus
	{-1.0*SCALING_FACTOR,	-2*SCALING_FACTOR,	0*SCALING_FACTOR},	// Final focus

	{ANGLE_DTOR(0), ANGLE_DTOR(-112), ANGLE_DTOR(0)},						// Start angles
	{ANGLE_DTOR(0), ANGLE_DTOR(-180), ANGLE_DTOR(0)},						// Final angles
} ;


// eye,focus
CCameraPosTrackInfo AcclaimLogoEyeFocusPosTrack = 
{
	{-25*SCALING_FACTOR,		-2*SCALING_FACTOR,	-10*SCALING_FACTOR},	// Start eye
	{-1.0*SCALING_FACTOR,	-2*SCALING_FACTOR,	-26*SCALING_FACTOR},	// Final eye

	{-1.0*SCALING_FACTOR,	-2*SCALING_FACTOR,	0*SCALING_FACTOR},	// Start focus
	{-1.0*SCALING_FACTOR,	-2*SCALING_FACTOR,	0*SCALING_FACTOR},	// Final focus

	{ANGLE_DTOR(0), ANGLE_DTOR(0), ANGLE_DTOR(0)},						// Start angles
	{ANGLE_DTOR(0), ANGLE_DTOR(0), ANGLE_DTOR(0)},						// Final angles
} ;


#if 0
// Original
CCameraPosTrackInfo AcclaimLogoPosTrack = 
{
	{-36*SCALING_FACTOR,		-2*SCALING_FACTOR,	-28*SCALING_FACTOR},	// Start eye
	{-1.0*SCALING_FACTOR,	-2*SCALING_FACTOR,	-26*SCALING_FACTOR},	// Final eye

	{-1.0*SCALING_FACTOR,	-2*SCALING_FACTOR,	0*SCALING_FACTOR},	// Start focus
	{-1.0*SCALING_FACTOR,	-2*SCALING_FACTOR,	0*SCALING_FACTOR},	// Final focus

	{ANGLE_DTOR(0), ANGLE_DTOR(0), ANGLE_DTOR(0)},						// Start angles
	{ANGLE_DTOR(0), ANGLE_DTOR(0), ANGLE_DTOR(0)},						// Final angles
} ;
#endif




void CCamera__Setup_INTRO_ACCLAIM_LOGO(CCamera *pThis)
{
#if DO_INTRO
	pThis->m_Mode = CAMERA_INTRO_ACCLAIM_LOGO_MODE ;
	pThis->m_CinemaMode = TRUE ;
	pThis->m_LetterBoxScale = 0 ;

#else
	GetApp()->m_Mode = MODE_TITLE ;
	GetApp()->m_WarpID = 0 ;
#endif

	SkipIntro = FALSE ;
}

void CCamera__Code_INTRO_ACCLAIM_LOGO(CCamera *pThis)
{
	FLOAT	t ;

	// Update move time
	pThis->m_StageTime += frame_increment ;
	if (pThis->m_StageTime > CAMERA_INTRO_ACCLAIM_LOGO_TIME)
		pThis->m_StageTime = CAMERA_INTRO_ACCLAIM_LOGO_TIME ;

	t = pThis->m_StageTime / CAMERA_INTRO_ACCLAIM_LOGO_TIME ;

	// Zoom final position
	pThis->m_ViewBlend = COSINE_TWEEN(t*t*t) ;
	CCameraView__TweenPosTrack(&pThis->m_ViewA, &AcclaimLogoEyeFocusPosTrack, COSINE_TWEEN(t)) ;
	CCameraView__TweenPosTrack(&pThis->m_ViewB, &AcclaimLogoEyeAnglePosTrack, COSINE_TWEEN(t)) ;

	// The end?
	pThis->m_Time += frame_increment ;
	if ((GetApp()->m_WarpID == INTRO_ACCLAIM_LOGO_WARP_ID) &&
		 ((pThis->m_Time > CAMERA_INTRO_ACCLAIM_LOGO_TOTAL_TIME) || (pThis->m_StartPressed)))
		CCamera__GotoNextIntroMode(pThis) ;
}





/////////////////////////////////////////////////////////////////////////////
//	Modes:	  	 CAMERA_INTRO_IGGY_MODE
//	Description: Showing the old iggy logo
/////////////////////////////////////////////////////////////////////////////
void CCamera__Setup_INTRO_IGGY(CCamera *pThis)
{
	pThis->m_Mode = CAMERA_INTRO_IGGY_MODE ;
	pThis->m_CinemaMode = TRUE ;
	pThis->m_LetterBoxScale = 0 ;

	// Close up start position
	pThis->m_View.m_vEye.x = 0*SCALING_FACTOR ;
	pThis->m_View.m_vEye.y = 10*SCALING_FACTOR ;
	pThis->m_View.m_vEye.z = -5*SCALING_FACTOR ;
	
	pThis->m_View.m_vFocus.x = 0*SCALING_FACTOR ;
	pThis->m_View.m_vFocus.y = 10*SCALING_FACTOR ;
	pThis->m_View.m_vFocus.z = 0*SCALING_FACTOR ;
}

FLOAT	CVector3__DistFromPt(CVector3 *pThis, CVector3 *pvPt)
{
	FLOAT	dx,dy,dz, d ;
	dx = pThis->x - pvPt->x ;
	dy = pThis->y - pvPt->y ;
	dz = pThis->z - pvPt->z ;

	d = (dx * dx) + (dy * dy) + (dz * dz) ;
	if (d != 0.0)
		return sqrt(d) ;
	else
		return d ;
}

void CCamera__Code_INTRO_IGGY(CCamera *pThis)
{
	// Zoom out
	CVector3	vFinalEye = {0.25*SCALING_FACTOR, 7.0*SCALING_FACTOR, -24*SCALING_FACTOR} ;
	CVector3	vFinalFocus = {0.25*SCALING_FACTOR, 12.0*SCALING_FACTOR, 0*SCALING_FACTOR} ;

	// Zoom eye to position (with movement cut off)
	if (CVector3__DistFromPt(&vFinalEye, &pThis->m_View.m_vEye) > (0.3*SCALING_FACTOR))
		CCameraView__ZoomEyeToPos(&pThis->m_View, &vFinalEye, 1.0/12.0) ;

	// Zoom focus to position (with movement cut off)
	if (CVector3__DistFromPt(&vFinalFocus, &pThis->m_View.m_vFocus) > (0.3*SCALING_FACTOR))
		CCameraView__ZoomFocusToPos(&pThis->m_View, &vFinalFocus, 1.0/12.0) ;

	// Goto next stage?
	if ((GetApp()->m_WarpID == INTRO_IGGY_WARP_ID) &&
		 ((pThis->m_StartPressed) || (pThis->m_NextCinematicStage)))
		CCamera__GotoNextIntroMode(pThis) ;
}



/////////////////////////////////////////////////////////////////////////////
//	Modes:	  	 CAMERA_INTRO_TUROK_DRAWING_BOW_MODE
//	Description: Circling Turok as he draws his bow
/////////////////////////////////////////////////////////////////////////////
void CCamera__Setup_INTRO_TUROK_DRAWING_BOW(CCamera *pThis)
{
	pThis->m_Mode = CAMERA_INTRO_TUROK_DRAWING_BOW_MODE ;
	pThis->m_CinemaMode = TRUE ;
	pThis->m_LetterBoxScale = 0 ;
}



CCameraTrackInfo IntroTrack =
{
	ANGLE_DTOR(180),	ANGLE_DTOR(0),			// YRotOffset,		FinalYRotOffset ;
	5*SCALING_FACTOR,	8*SCALING_FACTOR,		//	Radius,			FinalRadius ;
	1*SCALING_FACTOR,	7*SCALING_FACTOR,		// EyeYOffset,	  	FinalEyeYOffset ;
	1*SCALING_FACTOR,	7*SCALING_FACTOR,		// FocusYOffset,	FinalFocusYOffset ;
} ;



#define INTRO_TUROK_DRAWING_BOW_TIME	SECONDS_TO_FRAMES(12.8)


void CCamera__SetFieldOfView(CCamera *pThis)
{
	CEngineApp	*pEngine = GetApp() ;
	CGameObjectInstance	*pPlayer;
	CGameRegion				*pRegion;
	CROMRegionSet			*pRegionSet;

	pPlayer = CEngineApp__GetPlayer(GetApp());
	if (!pPlayer)
		return;

	pRegion = pPlayer->ah.ih.m_pCurrentRegion;
	if (!pRegion)
		return;

	pRegionSet = CScene__GetRegionAttributes(&GetApp()->m_Scene, pRegion);
	if (!pRegionSet)
		return;

	pEngine->m_LastFieldOfView = 
	pRegionSet->m_FieldOfView = pEngine->m_FieldOfView ;
}


void CCamera__Code_INTRO_TUROK_DRAWING_BOW(CCamera *pThis)
{
	CEngineApp	*pEngine = GetApp() ;

	// Zoom up to head
	CCameraView__TweenTrackAroundTurok(&pThis->m_View, &IntroTrack, 
												 COSINE_TWEEN(pThis->m_Time / INTRO_TUROK_DRAWING_BOW_TIME)) ;

	// Goto next position
	pThis->m_Time += frame_increment ;
	if (pThis->m_Time > INTRO_TUROK_DRAWING_BOW_TIME)
		pThis->m_Time = INTRO_TUROK_DRAWING_BOW_TIME ;

	// Start pressed?
	if ((GetApp()->m_WarpID == INTRO_TUROK_DRAWING_BOW_WARP_ID) && (pThis->m_StartPressed))
	{
		pThis->m_Stage = 2 ;
		pThis->m_NextCinematicStage = TRUE ;
	}

	// Goto next stage?
	if (pThis->m_NextCinematicStage)
	{
		if (++pThis->m_Stage == 3)
		{
			CCamera__KillSoundHandle(pThis, 0) ;	// Kill wind sound
			CCamera__GotoNextIntroMode(pThis) ;
		}
	}

	// Change field of view?
	if (pThis->m_Stage == 1)
	{
		pThis->m_StageTime += frame_increment ;
		pEngine->m_FieldOfView += 0.5*frame_increment ;
	}

	// Set fov and move towards turok
	pThis->m_View.m_vEye.z += pThis->m_StageTime * 0.12 ;
	CCamera__SetFieldOfView(pThis) ;
}



/////////////////////////////////////////////////////////////////////////////
//	Modes:	  	 CAMERA_INTRO_ZOOM_TO_LOGO_MODE
//	Description: Zooming to turok logo
/////////////////////////////////////////////////////////////////////////////

#define CAMERA_INTRO_ZOOM_TIME			SECONDS_TO_FRAMES(1)
#define CAMERA_INTRO_ZOOM_TOTAL_TIME 	SECONDS_TO_FRAMES(3)

void CCamera__Setup_INTRO_ZOOM_TO_LOGO(CCamera *pThis)
{
	pThis->m_Mode = CAMERA_INTRO_ZOOM_TO_LOGO_MODE ;
	pThis->m_CinemaMode = TRUE ;
	pThis->m_LetterBoxScale = 0 ;

	IntroFirstRun = FALSE ;

	// Put tweening to end, if skipping the intro
	if (SkipIntro)
	{
		pThis->m_StageTime = CAMERA_INTRO_ZOOM_TIME ;
		pThis->m_Time = CAMERA_INTRO_ZOOM_TOTAL_TIME ;
	}
}

CCameraPosTrackInfo ZoomLogoPosTrack = 
{
	{0,5*SCALING_FACTOR,(-194-0)*SCALING_FACTOR},	// Start eye
	{0,5*SCALING_FACTOR,(-9-0)*SCALING_FACTOR},	// Final eye

	{0,5*SCALING_FACTOR,15*SCALING_FACTOR},	// Start focus
	{0,5*SCALING_FACTOR,15*SCALING_FACTOR},	// Final focus

	{ANGLE_DTOR(0), ANGLE_DTOR(0), ANGLE_DTOR(0)},						// Start angles
	{ANGLE_DTOR(0), ANGLE_DTOR(0), ANGLE_DTOR(0)},						// Final angles
} ;

#define ARROW_LENGTH		(5.5*SCALING_FACTOR)

// Tip of arrow position
#ifndef KANJI
#define ARROW_FINAL_X	(2.21*SCALING_FACTOR)
#define ARROW_FINAL_Y	(8.05*SCALING_FACTOR)
#else
#define ARROW_FINAL_X	(-.5*SCALING_FACTOR)
#define ARROW_FINAL_Y	(7.65*SCALING_FACTOR)
#endif
#define ARROW_FINAL_Z	(7.16*SCALING_FACTOR)


CCameraPosTrackInfo ZoomArrowActualPosTrack = 
{
	{0,(5-1)*SCALING_FACTOR,(-194+3)*SCALING_FACTOR},						// Start eye
	{ARROW_FINAL_X, ARROW_FINAL_Y, ARROW_FINAL_Z-ARROW_LENGTH},							// Final eye

	{0,(5-1)*SCALING_FACTOR,(15+3)*SCALING_FACTOR},							// Start focus
	{3.4 * SCALING_FACTOR,	9*SCALING_FACTOR,8*SCALING_FACTOR},		// Final focus

	{ANGLE_DTOR(0), ANGLE_DTOR(0), ANGLE_DTOR(0)},						// Start angles
	{ANGLE_DTOR(0), ANGLE_DTOR(0), ANGLE_DTOR(360*1)},						// Final angles
} ;


CCameraPosTrackInfo ZoomArrowCameraPosTrack = 
{
	{0,(5-1)*SCALING_FACTOR,(-194+3)*SCALING_FACTOR},						// Start eye
	{0,(5-1)*SCALING_FACTOR,(-9+3)*SCALING_FACTOR},							// Final eye

	{0,(5-1)*SCALING_FACTOR,(15+3)*SCALING_FACTOR},							// Start focus
	{0,(5-1)*SCALING_FACTOR,(15+3)*SCALING_FACTOR},							// Final focus

	{ANGLE_DTOR(0), ANGLE_DTOR(0), ANGLE_DTOR(0)},						// Start angles
	{ANGLE_DTOR(0), ANGLE_DTOR(0), ANGLE_DTOR(360*1)},						// Final angles
} ;



/////////////////////////////////////////////////////////////////////////////
// Arrow swoosh
/////////////////////////////////////////////////////////////////////////////
#define ARROW_SWOOSH_EDGE_LENGTH	(0.5*SCALING_FACTOR)


CSwooshEdgeVertexInfo IntroArrowSwooshVertices[6] ;

CSwooshInfo IntroArrowSwooshInfo[] =
{
	{SECONDS_TO_FRAMES(1.0),				// Total time on screen
	 SECONDS_TO_FRAMES(0.8),			// Total time before fade swoosh out
	 SECONDS_TO_FRAMES(0.5),			// Total edge life time
	 0,										// Object node
	 1,										// Scale
	 NULL,									// General flags
	 2,										// No of edge vertices		
	 &IntroArrowSwooshVertices[0]},	// Edge vertices

	{SECONDS_TO_FRAMES(1.0),				// Total time on screen
	 SECONDS_TO_FRAMES(0.8),			// Total time before fade swoosh out
	 SECONDS_TO_FRAMES(0.5),			// Total edge life time
	 0,										// Object node
	 1,										// Scale
	 NULL,									// General flags
	 2,										// No of edge vertices		
	 &IntroArrowSwooshVertices[2]},	// Edge vertices

	{SECONDS_TO_FRAMES(1.0),				// Total time on screen
	 SECONDS_TO_FRAMES(0.8),			// Total time before fade swoosh out
	 SECONDS_TO_FRAMES(0.5),			// Total edge life time
	 0,										// Object node
	 1,										// Scale
	 NULL,									// General flags
	 2,										// No of edge vertices		
	 &IntroArrowSwooshVertices[4]},	// Edge vertices
} ;





#define COSINE_ARC(t)	(-((cos(ANGLE_DTOR(t*360)) - 1) * 0.5))

void CCamera__Code_INTRO_ZOOM_TO_LOGO(CCamera *pThis)
{
	CGameObjectInstance	*pArrow = pThis->m_pObject, *pTurok = CEngineApp__GetPlayer(GetApp()) ;
	FLOAT	Blend, Arc, t ;
	CVector3	vPos ;
	INT32		i ;

	// Start arrow swoosh
	if ((pArrow) && (pThis->m_Time == 0))
	{
		for (i = 0 ; i < 6 ; i++)
		{
			if (i & 1)
			{
				CVector3__Set(&IntroArrowSwooshVertices[i].m_Start.m_vPos, 0,0,0) ;
			}				
			else
			{
				CVector3__Set(&IntroArrowSwooshVertices[i].m_Start.m_vPos, 
								  0,
								  ARROW_SWOOSH_EDGE_LENGTH * sin(ANGLE_DTOR((i*120)+15)),
								  ARROW_SWOOSH_EDGE_LENGTH * cos(ANGLE_DTOR((i*120)+15))) ;
			}

			CVector3__Set(&IntroArrowSwooshVertices[i].m_End.m_vPos, 0,0,0) ;

			IntroArrowSwooshVertices[i].m_Start.m_Color.Red = 10 ;
			IntroArrowSwooshVertices[i].m_Start.m_Color.Green = 10 ;
			IntroArrowSwooshVertices[i].m_Start.m_Color.Blue = 255 ;

			IntroArrowSwooshVertices[i].m_End.m_Color.Red = 00 ;
			IntroArrowSwooshVertices[i].m_End.m_Color.Green = 0 ;
			IntroArrowSwooshVertices[i].m_End.m_Color.Blue = 55;

			IntroArrowSwooshVertices[i].m_Start.m_Alpha = MAX_OPAQ/4 ;
			IntroArrowSwooshVertices[i].m_End.m_Alpha = MIN_OPAQ ;
			
		}

		CFxSystem__AddObjectSwoosh(&GetApp()->m_FxSystem, pArrow, &IntroArrowSwooshInfo[0]) ;
		CFxSystem__AddObjectSwoosh(&GetApp()->m_FxSystem, pArrow, &IntroArrowSwooshInfo[1]) ;
		CFxSystem__AddObjectSwoosh(&GetApp()->m_FxSystem, pArrow, &IntroArrowSwooshInfo[2]) ;
	}


	// Update move time
	if (pThis->m_Stage == 0)
	{
		pThis->m_StageTime += frame_increment ;
		if (pThis->m_StageTime >= CAMERA_INTRO_ZOOM_TIME)
		{
			pThis->m_StageTime = CAMERA_INTRO_ZOOM_TIME ;
			pThis->m_Stage++ ;

			if (!SkipIntro)
			{	
				CCamera__SetShakeY(pThis,20) ;

				if (pArrow)
				{
					vPos.x = 3.5*SCALING_FACTOR ;
					vPos.y = 9.1*SCALING_FACTOR ;
					vPos.z = 9*SCALING_FACTOR ;

					CVector3__Set(&vPos, ARROW_FINAL_X, ARROW_FINAL_Y, ARROW_FINAL_Z) ;
					AI_DoParticle(&pArrow->ah.ih, PARTICLE_TYPE_GENERIC39, vPos) ;
					AI_DoSound(&pArrow->ah.ih, SOUND_GENERIC_143, 1, 0) ;
				}
			}
		}
	}

	// Zoom final position
	Blend = pThis->m_StageTime / CAMERA_INTRO_ZOOM_TIME ;

	// Zoom camera
	CCameraView__TweenPosTrack(&pThis->m_View, &ZoomLogoPosTrack, Blend) ;

	// Zoom arrow positions
	CCameraView__TweenPosTrack(&pThis->m_ViewA, &ZoomArrowCameraPosTrack, Blend) ;
	CCameraView__TweenPosTrack(&pThis->m_ViewB, &ZoomArrowActualPosTrack, Blend) ;

	// The end?
	if (GetApp()->m_WarpID == INTRO_ZOOM_TO_LOGO_WARP_ID)
	{
		pThis->m_Time += frame_increment ;
		if ((pThis->m_StartPressed) || (pThis->m_Time > CAMERA_INTRO_ZOOM_TOTAL_TIME))
		{
			GetApp()->m_WarpID = INTRO_ACCLAIM_LOGO_WARP_ID ;		// For next time around!
			GetApp()->m_Mode = MODE_TITLE ;
			GetApp()->m_ModeTime = 0 ;
		}
	}

	// Blend arrow positions
	t = Blend * Blend ;
	t *= t ;
	t *= t ;
	CCameraView__Blend(&pThis->m_ViewC, t, &pThis->m_ViewA, &pThis->m_ViewB) ;

	// Add in arc
	Arc = COSINE_ARC(Blend) * 15 * SCALING_FACTOR ;
	pThis->m_View.m_vEye.y += Arc ;
	pThis->m_ViewC.m_vEye.y += Arc ;

	// Position arrow infont of camera
	if (pArrow)
		pArrow->ah.ih.m_vPos = pThis->m_ViewC.m_vEye ;

	if (pTurok)
		pTurok->ah.ih.m_vPos = pThis->m_ViewC.m_vEye ;
}



CQuatern CGameObjectInstance__CinemaTurokArrowOrientation(CGameObjectInstance *pThis)
{
	CQuatern qRot ;
	CQuatern__BuildFromAxisAngle(&qRot, 0,0,1, GetApp()->m_Camera.m_ViewC.m_vRotation.z) ;

	AI_SetDesiredAnim(pThis, AI_ANIM_WEAPON_FIRE1, TRUE) ;

	return qRot ;
}




/////////////////////////////////////////////////////////////////////////////
//	Modes:	  	 CAMERA_GAME_INTRO_MODE
//	Description: Showing the player what to do
//					 The camera zooms in and rotates around the hub.
//					 Two separate camera paths are blended together. Neat eh?
/////////////////////////////////////////////////////////////////////////////
void CCamera__Setup_GAME_INTRO(CCamera *pThis)
{
	pThis->m_Mode = CAMERA_GAME_INTRO_MODE ;
	pThis->m_Stage = 0 ;
	pThis->m_StageTime = 0 ;

	// Put into cinema mode
	pThis->m_CinemaMode = TRUE ;
//	pThis->m_LetterBoxScale = LETTERBOX_SCALE ;

	COnScreen__InitializeGameText() ;
}

#define HUB_CENTER_X		(-179*SCALING_FACTOR)
#define HUB_CENTER_Z		(358*SCALING_FACTOR)

CCameraPosTrackInfo GameIntroPosTrack1 = 
{
	{HUB_CENTER_X, 250*SCALING_FACTOR, HUB_CENTER_Z},	// Start eye
	{HUB_CENTER_X, 100*SCALING_FACTOR, HUB_CENTER_Z},	// Final eye

	{HUB_CENTER_X, 90*SCALING_FACTOR, HUB_CENTER_Z},	// Start focus
	{HUB_CENTER_X, 90*SCALING_FACTOR, HUB_CENTER_Z},	// Final focus

	{ANGLE_DTOR(0), ANGLE_DTOR(0), ANGLE_DTOR(0)},						// Start angles
	{ANGLE_DTOR(0), ANGLE_DTOR(0), ANGLE_DTOR(0)},						// Final angles
} ;

CCameraTrackInfo GameIntroTrack1 =
{
	ANGLE_DTOR(0),			ANGLE_DTOR(360),		// YRotOffset,		FinalYRotOffset ;
	70*SCALING_FACTOR,	70*SCALING_FACTOR, 	//	Radius,			FinalRadius ;
	10*SCALING_FACTOR,	10*SCALING_FACTOR, 	// EyeYOffset,	  	FinalEyeYOffset ;
	6*SCALING_FACTOR,		6*SCALING_FACTOR,		// FocusYOffset,	FinalFocusYOffset ;
} ;


#define GAME_INTRO_TOTAL_TIME	SECONDS_TO_FRAMES(9.5)

#define GAME_INTRO_ZOOM_TIME	SECONDS_TO_FRAMES(6)
#define GAME_INTRO_SPIN_SPEED	0.0035
#define GAME_INTRO_BLEND_TIME	SECONDS_TO_FRAMES(6)

CVector3 HubCentre = {HUB_CENTER_X, 100*SCALING_FACTOR, HUB_CENTER_Z} ;

void CCamera__Code_GAME_INTRO(CCamera *pThis)
{

	// Update ViewA
	CCameraView__TweenTrackAroundPos(&pThis->m_ViewA, &GameIntroTrack1, pThis->m_StageTime, &HubCentre, 0) ;
	pThis->m_StageTime += frame_increment*GAME_INTRO_SPIN_SPEED ;
	if (pThis->m_StageTime > 1)
		pThis->m_StageTime -= 1 ;

	// Update ViewB
	CCameraView__TweenPosTrack(&pThis->m_ViewB, &GameIntroPosTrack1, pThis->m_ViewB.m_Time / GAME_INTRO_ZOOM_TIME) ;
	pThis->m_ViewB.m_Time += frame_increment ;
	if (pThis->m_ViewB.m_Time > GAME_INTRO_ZOOM_TIME)
		pThis->m_ViewB.m_Time = GAME_INTRO_ZOOM_TIME ;

	// Blend views
	CCameraView__Blend(&pThis->m_View, COSINE_TWEEN(pThis->m_View.m_Time / GAME_INTRO_BLEND_TIME),
							 &pThis->m_ViewB, &pThis->m_ViewA) ;
	pThis->m_View.m_Time += frame_increment ;
	if (pThis->m_View.m_Time > GAME_INTRO_BLEND_TIME)
		pThis->m_View.m_Time = GAME_INTRO_BLEND_TIME ;

	// Display text
	if (pThis->m_ViewC.m_Time < SECONDS_TO_FRAMES(1))
	{
		pThis->m_ViewC.m_Time += frame_increment ;
		if (pThis->m_ViewC.m_Time >= SECONDS_TO_FRAMES(1))
		{
			COnScreen__AddGameTextForTime(text_hub1,7) ;
			COnScreen__AddGameTextForTime(text_hub2,7) ;
#ifndef KANJI
			COnScreen__AddGameTextForTime(text_hub3,7) ;
#endif
//#ifdef GERMAN
//			COnScreen__AddGameTextForTime(text_hub4,7) ;
//#endif
		}
	}
	// Exit/ end
	pThis->m_Time += frame_increment ;
	if ((pThis->m_Stage == 0) &&
		 ((pThis->m_Time >= GAME_INTRO_TOTAL_TIME) || (pThis->m_StartPressed)))
	{
		pThis->m_Stage++ ;
		CEngineApp__NewLife(GetApp()) ;
	}
}


/////////////////////////////////////////////////////////////////////////////
//	Modes:	  	 CAMERA_GAME_INTRO_KEY_MODE
//	Description: Show the player what a key looks like
/////////////////////////////////////////////////////////////////////////////
void CCamera__Setup_GAME_KEY_INTRO(CCamera *pThis)
{
	pThis->m_Mode = CAMERA_GAME_INTRO_KEY_MODE ;
	pThis->m_Stage = 0 ;
	pThis->m_StageTime = 0 ;

	// Put into cinema mode
	pThis->m_CinemaMode = TRUE ;
//	pThis->m_LetterBoxScale = LETTERBOX_SCALE ;

	COnScreen__AddGameTextForTime(text_locatekey1,7) ;
	COnScreen__AddGameTextForTime(text_locatekey2,7) ;
}


#define GAME_INTRO_KEY_TOTAL_TIME	SECONDS_TO_FRAMES(5)

//CVector3 KeyCentre = {-1136*SCALING_FACTOR, 50*SCALING_FACTOR, 372*SCALING_FACTOR} ;

void CCamera__Code_GAME_KEY_INTRO(CCamera *pThis)
{
	pThis->m_View.m_vEye.x = -1136*SCALING_FACTOR ;
	pThis->m_View.m_vEye.y = 55*SCALING_FACTOR ;
	pThis->m_View.m_vEye.z = 355*SCALING_FACTOR ;

	pThis->m_View.m_vFocus.x = -1136*SCALING_FACTOR ;
	pThis->m_View.m_vFocus.y = 53*SCALING_FACTOR ;
	pThis->m_View.m_vFocus.z = 372*SCALING_FACTOR ;


	pThis->m_StageTime += frame_increment ;

	// Exit, done?
	if ((pThis->m_StartPressed) || (pThis->m_StageTime >= GAME_INTRO_KEY_TOTAL_TIME))
	{
		GetApp()->m_WarpID = GAME_INTRO_WARP_ID ;
		CEngineApp__SetupFadeTo(GetApp(), MODE_RESETLEVEL) ;
	}
}
