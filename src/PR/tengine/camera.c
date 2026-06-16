// Top level camera and game camera control file by Stephen Broumley

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

#include "camera.h"
#include "cammodes.h"
#include "introcam.h"
#include "cinecam.h"
#include "galrecam.h"
#include "crdcam.h"



/////////////////////////////////////////////////////////////////////////////
// Constants
/////////////////////////////////////////////////////////////////////////////
#define DO_CINEMATICS			1		// Set to one to activate death cinemas
#define TEST_LETTERBOX_VIEW	0		// Set to one to view letter box screen
#define WATER_WOBBLE				1


/////////////////////////////////////////////////////////////////////////////
// Water wobble
/////////////////////////////////////////////////////////////////////////////

// Wobble Scale
#define CAMERA_ENTER_WATER_WOBBLE_X_SCALE				0.4
#define CAMERA_ENTER_WATER_WOBBLE_Y_SCALE				0.15

#define CAMERA_ZOOM_TO_UNDER_WATER_WOBBLE_SCALE		(1.0/2.0)

#define CAMERA_UNDER_WATER_WOBBLE_X_SCALE				0.2
#define CAMERA_UNDER_WATER_WOBBLE_Y_SCALE				0.1


// Wobble Speed
#define CAMERA_ENTER_WATER_WOBBLE_X_SPEED				ANGLE_DTOR(90)
#define CAMERA_ENTER_WATER_WOBBLE_Y_SPEED				ANGLE_DTOR(75)

#define CAMERA_ZOOM_TO_UNDER_WATER_WOBBLE_SPEED		(1.0/6.0)

#define CAMERA_UNDER_WATER_WOBBLE_X_SLOW_SPEED		ANGLE_DTOR(4)
#define CAMERA_UNDER_WATER_WOBBLE_X_FAST_SPEED		ANGLE_DTOR(6)

#define CAMERA_UNDER_WATER_WOBBLE_Y_SLOW_SPEED		ANGLE_DTOR(2)
#define CAMERA_UNDER_WATER_WOBBLE_Y_FAST_SPEED		ANGLE_DTOR(5)



/////////////////////////////////////////////////////////////////////////////
// Game Functions
/////////////////////////////////////////////////////////////////////////////

void CCamera__Code_TUROK_EYE(CCamera *pThis) ;



/////////////////////////////////////////////////////////////////////////////
// Variables
/////////////////////////////////////////////////////////////////////////////

// Camera viewport
Vp CameraVp[2] =
{
	{ SCREEN_WD*2, SCREEN_HT*2, G_MAXZ, 0,	// scale
	  SCREEN_WD*2, SCREEN_HT*2, 0, 0},	// translate

	{ SCREEN_WD*2, SCREEN_HT*2, G_MAXZ, 0,	// scale
	  SCREEN_WD*2, SCREEN_HT*2, 0, 0},	// translate
};


// The big top level mode table
CCameraModeInfo	CameraModeTable[] =
{
	{CCamera__Setup_LEGALSCREEN_LOGO,				CCamera__Code_LEGALSCREEN_LOGO},
	{CCamera__Setup_INTRO_ACCLAIM_LOGO,				CCamera__Code_INTRO_ACCLAIM_LOGO},
	{CCamera__Setup_INTRO_IGGY,						CCamera__Code_INTRO_IGGY},
	{CCamera__Setup_INTRO_TUROK_DRAWING_BOW,		CCamera__Code_INTRO_TUROK_DRAWING_BOW},
	{CCamera__Setup_INTRO_ZOOM_TO_LOGO,				CCamera__Code_INTRO_ZOOM_TO_LOGO},

	{CCamera__Setup_GAME_INTRO,						CCamera__Code_GAME_INTRO},
	{CCamera__Setup_GAME_KEY_INTRO, 					CCamera__Code_GAME_KEY_INTRO},

	{NULL,													CCamera__Code_TUROK_EYE},

	{CCamera__Setup_CINEMATIC_TUROK_DEATH,			CCamera__Code_CINEMATIC_TUROK_DEATH},
	{CCamera__Setup_CINEMATIC_TUROK_FALL_DEATH,	CCamera__Code_CINEMATIC_TUROK_FALL_DEATH},
	{CCamera__Setup_CINEMATIC_TUROK_WATER_DEATH,	CCamera__Code_CINEMATIC_TUROK_WATER_DEATH},
	{CCamera__Setup_CINEMATIC_TUROK_RESURRECT,	CCamera__Code_CINEMATIC_TUROK_RESURRECT},

	{CCamera__Setup_GALLERY_IDLE,						CCamera__Code_GALLERY_IDLE},

	{CCamera__Setup_CINEMATIC_LONGHUNTER,			CCamera__Code_CINEMATIC_LONGHUNTER},
	{CCamera__Setup_CINEMATIC_FIRST_HUMVEE,		CCamera__Code_CINEMATIC_FIRST_HUMVEE},
	{CCamera__Setup_CINEMATIC_FINAL_HUMVEE,		CCamera__Code_CINEMATIC_FINAL_HUMVEE},

	{CCamera__Setup_CINEMATIC_TUROK_KILL_MANTIS,	CCamera__Code_CINEMATIC_TUROK_KILL_MANTIS},
	{CCamera__Setup_CINEMATIC_MANTIS_KILL_TUROK,	CCamera__Code_CINEMATIC_MANTIS_KILL_TUROK},
	{CCamera__Setup_CINEMATIC_TUROK_KILL_TREX,  	CCamera__Code_CINEMATIC_TUROK_KILL_TREX},
	{CCamera__Setup_CINEMATIC_TREX_KILL_TUROK,  	CCamera__Code_CINEMATIC_TREX_KILL_TUROK},
	{CCamera__Setup_CINEMATIC_TUROK_KILL_LONGHUNTER,  	CCamera__Code_CINEMATIC_TUROK_KILL_LONGHUNTER},
	{CCamera__Setup_CINEMATIC_TUROK_KILL_CAMPAIGNER,  	CCamera__Code_CINEMATIC_TUROK_KILL_CAMPAIGNER},
	{CCamera__Setup_CINEMATIC_CAMPAIGNER_KILL_TUROK,  	CCamera__Code_CINEMATIC_CAMPAIGNER_KILL_TUROK},

	{CCamera__Setup_CINEMATIC_TUROK_PICKUP_KEY, 	CCamera__Code_CINEMATIC_TUROK_PICKUP_KEY},

	{CCamera__Setup_CINEMATIC_END_SEQUENCE, 		CCamera__Code_CINEMATIC_END_SEQUENCE},
	{CCamera__Setup_CINEMATIC_END_SEQUENCE2, 		CCamera__Code_CINEMATIC_END_SEQUENCE2},

	{CCamera__Setup_CINEMATIC_CREDIT,				CCamera__Code_CINEMATIC_CREDIT},

	{CCamera__Setup_CINEMATIC_TUROK_VICTORY,		CCamera__Code_CINEMATIC_TUROK_VICTORY},
} ;




// Collision structure used to move turok away from walls
//CCollisionInfo		ci_moveintoopen ;


void CShake__Construct(CShake *pThis) ;
FLOAT CShake__Update(CShake *pThis) ;



/*****************************************************************************
*
*
*
*	INITIALISATION CODE
*
*
*
*****************************************************************************/

/*
void CCollisionInfo__SetMoveIntoOpenDefaults(CCollisionInfo *pThis)
{
	pThis->m_WallBehavior			= WALL_SLIDE ;
	pThis->m_InstanceBehavior		= INSTANCE_SLIDE ;
	pThis->m_GravityAcceleration	= GRAVITY_WATER_ACCELERATION/4 ;
	pThis->m_BounceReturnEnergy	= 1.0 ;
	pThis->m_GroundFriction			= 0 ;
	pThis->m_AirFriction				= 0 ;
	pThis->m_WaterFriction			= 0 ;
	pThis->m_dwFlags					= COLLISIONFLAG_USEWALLRADIUS |
											  COLLISIONFLAG_IGNOREDEAD | COLLISIONFLAG_WATERCOLLISION |
											  COLLISIONFLAG_MOVETHROUGHDOORS |
											  COLLISIONFLAG_CANENTERRESTRICTEDAREA ;
}
*/


void CShake__Construct(CShake *pThis)
{
	pThis->m_Shake = 0 ;
	pThis->m_LastShake = 0 ;
	pThis->m_Amp = 0 ;
	pThis->m_Angle = 0 ;
}

void CShake__Set(CShake *pThis, FLOAT Shake)
{
	// Limit shake
	if ((GetApp()->m_pBossActive) && (Shake > 15))
		Shake = 15 ;

	// Set it going
	pThis->m_Shake = Shake ;
}

FLOAT CShake__Update(CShake *pThis)
{
	FLOAT	Shake = 0 ;

	// Start shake?
	if (pThis->m_Shake > pThis->m_LastShake)
		pThis->m_LastShake = pThis->m_Shake ;

	// Reset shake angle
	if (pThis->m_Amp == 0)
		pThis->m_Angle = 90 ;

	// Start shake y going?
	if (pThis->m_Amp < (pThis->m_LastShake * 2))
		pThis->m_Amp = pThis->m_LastShake * 2 ;

	// Dampen screen shake counts
	DecelerateVarFLOAT(&pThis->m_Shake, frame_increment * 3) ;
	DecelerateVarFLOAT(&pThis->m_LastShake, frame_increment * 3) ;

	// Calculate "Shake"
	if (pThis->m_Amp > 0)
	{
		Shake = sin(ANGLE_DTOR(pThis->m_Angle)) * pThis->m_Amp ;

		// Make speed proportional to amplitude
		pThis->m_Angle += pThis->m_Amp * 2 * 3 * frame_increment ;

		// Dampen amplitude
		DecelerateVarFLOAT(&pThis->m_Amp, 2*3) ;
	}

	return Shake ;
}







/*****************************************************************************
*
*	Function:		CCamera__Construct()
*
******************************************************************************
*
*	Description:	Put camera in "turok eye" mode and clears vars
*
*	Inputs:			*pThis	-	Ptr to camera
*
*	Outputs:			None
*
*****************************************************************************/
void CCameraView__Construct(CCameraView *pThis)
{
	pThis->m_Time = 0 ;
	pThis->m_vEye.x = 0 ;
	pThis->m_vEye.y = 0 ;
	pThis->m_vEye.z = 0 ;
	pThis->m_vFocus.x = 0 ;
	pThis->m_vFocus.y = 0 ;
	pThis->m_vFocus.z = 0 ;
	pThis->m_vRotation.x = 0 ;
	pThis->m_vRotation.y = 0 ;
	pThis->m_vRotation.z = 0 ;
}


void CCameraView__CalculateXYRotation(CCameraView *pThis)
{
	CVector3	vLookDir ;

	// Calculate viewing angle from eye and focus position
	vLookDir.x = pThis->m_vFocus.x - pThis->m_vEye.x ;
	vLookDir.y = pThis->m_vFocus.y - pThis->m_vEye.y ;
	vLookDir.z = pThis->m_vFocus.z - pThis->m_vEye.z ;
	pThis->m_vRotation.x = -CVector3__XYAngle(&vLookDir) ;
	pThis->m_vRotation.y = CVector3__XZAngle(&vLookDir) ;
}

typedef struct s_CinemaInfo
{
	INT32	m_Flag ;		// Flag(s) to test for
	INT32	m_Mode ;		// Corresponding cinema mode
} CCinemaInfo ;


// Leave these in the correct order for priority!!
// Bosses killing Turok is above Turok killing bosses so that bosses aren't brought
// back to life!!
CCinemaInfo CameraCinemaInfo[] =
{
	// Bosses killing Turok
	{CINEMA_FLAG_MANTIS_KILL_TUROK,		CAMERA_CINEMA_MANTIS_KILL_TUROK_MODE},
	{CINEMA_FLAG_TREX_KILL_TUROK,			CAMERA_CINEMA_TREX_KILL_TUROK_MODE},
	{CINEMA_FLAG_CAMPAIGNER_KILL_TUROK, CAMERA_CINEMA_CAMPAIGNER_KILL_TUROK_MODE},

	// Turok killing bosses
	{CINEMA_FLAG_TUROK_KILL_LONGHUNTER, CAMERA_CINEMA_TUROK_KILL_LONGHUNTER_MODE},
	{CINEMA_FLAG_TUROK_KILL_CAMPAIGNER, CAMERA_CINEMA_TUROK_KILL_CAMPAIGNER_MODE},
	{CINEMA_FLAG_TUROK_KILL_MANTIS,		CAMERA_CINEMA_TUROK_KILL_MANTIS_MODE},
	{CINEMA_FLAG_TUROK_KILL_TREX,			CAMERA_CINEMA_TUROK_KILL_TREX_MODE},

	// Turok's normal death's and ressurection
	{CINEMA_FLAG_PLAY_DEATH,				CAMERA_CINEMA_TUROK_DEATH_MODE},
	{CINEMA_FLAG_PLAY_FALL_DEATH,			CAMERA_CINEMA_TUROK_FALL_DEATH_MODE},
	{CINEMA_FLAG_PLAY_WATER_DEATH,		CAMERA_CINEMA_TUROK_WATER_DEATH_MODE},
	{CINEMA_FLAG_PLAY_LONGHUNTER,			CAMERA_CINEMA_LONGHUNTER_MODE},
	{CINEMA_FLAG_PLAY_FINAL_HUMVEE,		CAMERA_CINEMA_FINAL_HUMVEE_MODE},
	{CINEMA_FLAG_PLAY_RESURRECT,			CAMERA_CINEMA_TUROK_RESURRECT_MODE},

	// Misc cinema's
	{CINEMA_FLAG_TUROK_PICKUP_KEY,		CAMERA_CINEMA_TUROK_PICKUP_KEY_MODE},
	{CINEMA_FLAG_TUROK_VICTORY,			CAMERA_CINEMA_TUROK_VICTORY_MODE},
	{CINEMA_FLAG_END_SEQUENCE,				CAMERA_CINEMA_END_SEQUENCE_MODE},
	{CINEMA_FLAG_END_SEQUENCE2,			CAMERA_CINEMA_END_SEQUENCE2_MODE},
	{CINEMA_FLAG_CREDIT,						CAMERA_CINEMA_CREDIT_MODE},
} ;

void CCamera__PreSceneConstruct(CCamera *pThis)
{
	CEngineApp	*pEngine = GetApp() ;

	// Was start pressed during a cinema during an attract demo?
	// This - Stops player from joining in demo!! - it is set to this in "attract.cpp"
	if (pEngine->m_CinemaFlags == -1)
		pEngine->m_CinemaFlags = 0 ;

	// Init camera
	if (!pEngine->m_CinemaFlags)
		pThis->m_pObject = NULL ;
}

void CCamera__Construct(CCamera *pThis)
{
	CEngineApp	*pEngine = GetApp() ;
	INT32			StartMode,i ;
	CCinemaInfo	*pInfo ;

	// Setup default pixel scale
	pThis->m_PixelXScale = 1.0 ;
	pThis->m_PixelYScale = 1.0 ;

	// Stop warp fx
	CCamera__InitializeWarp(pThis) ;

	// Clear tremor
	CVector3__Set(&pThis->m_vTremor, 0,0,0) ;

	// Setup views
	pThis->m_ViewBlend = CAMERA_VIEW_A_BLEND ;
	CCameraView__Construct(&pThis->m_View) ;
	CCameraView__Construct(&pThis->m_ViewA) ;
	CCameraView__Construct(&pThis->m_ViewB) ;
	CCameraView__Construct(&pThis->m_ViewC) ;

	CShake__Construct(&pThis->m_ShakeX) ;
	CShake__Construct(&pThis->m_ShakeY) ;
	CShake__Construct(&pThis->m_ShakeZ) ;

	pThis->m_Mode = CAMERA_TUROK_EYE_MODE ;	// Set because setup checks for a mode
	pThis->m_NextCinematicStage = FALSE ;
	pThis->m_StartPressed = FALSE ;
	pThis->m_CinemaMode = FALSE ;
	pThis->m_LetterBoxScale = 0 ;
	pThis->m_CinemaFadingOut = FALSE ;

	// Search for a cinema camera mode (default is turok eye mode for normal game play)
	StartMode = CAMERA_TUROK_EYE_MODE ;
	pInfo = CameraCinemaInfo ;
	i = sizeof(CameraCinemaInfo) / sizeof(CCinemaInfo) ;
	while(i--)
	{
		if (pEngine->m_CinemaFlags & pInfo->m_Flag)
		{
			pEngine->m_CinemaFlags &= ~pInfo->m_Flag ;
			StartMode = pInfo->m_Mode ;
			break ;
		}

		pInfo++ ;
	}

	// Put into start mode
	CCamera__SetupMode(pThis, StartMode) ;

	// Fudge to stop borders flickering!!!
	if (pThis->m_CinemaMode)
	{
		pEngine->m_FogColor[0] = 0 ;
		pEngine->m_FogColor[1] = 0 ;
		pEngine->m_FogColor[2] = 0 ;
		pEngine->m_FogColor[3] = 0 ;
	}

	// Prepare collision structure
	//CCollisionInfo__SetMoveIntoOpenDefaults(&ci_moveintoopen) ;
}





/*****************************************************************************
*
*
*
*	SOUND HANDLE CODE
*
*
*
*****************************************************************************/


/////////////////////////////////////////////////////////////////////////////
// Initailize all sound handles to off
/////////////////////////////////////////////////////////////////////////////
void CCamera__InitializeSoundHandles(CCamera *pThis)
{
	INT32	i ;
	for (i = 0 ; i < MAX_CAMERA_SOUND_HANDLES ; i++)
		pThis->m_UserSoundHandles[i] = -1 ;
}

/////////////////////////////////////////////////////////////////////////////
// Kill a sound
/////////////////////////////////////////////////////////////////////////////
void CCamera__KillSoundHandle(CCamera *pThis, INT32 Index)
{
	// Only kill sound if it's playing
	if (pThis->m_UserSoundHandles[Index] != -1)
	{
 		killCFX(pThis->m_UserSoundHandles[Index]) ;
		pThis->m_UserSoundHandles[Index] = -1 ;
	}
}


/////////////////////////////////////////////////////////////////////////////
// Set sound handle
/////////////////////////////////////////////////////////////////////////////
void CCamera__SetSoundHandle(CCamera *pThis, INT32 Index, INT32 Handle)
{
	pThis->m_UserSoundHandles[Index] = Handle ;
}



/*****************************************************************************
*
*
*
*	WARP FX CODE
*
*
*
*****************************************************************************/
enum CameraWarpModes
{
	CAMERA_WARP_OFF_MODE,
	CAMERA_WARP_IN_MODE,
	CAMERA_WARP_OUT_MODE,
	CAMERA_WARP_END_MODE
} ;


/////////////////////////////////////////////////////////////////////////////
// Warp view in
/////////////////////////////////////////////////////////////////////////////
void CCamera__InitializeWarp(CCamera *pThis)
{
	pThis->m_WarpMode = CAMERA_WARP_OFF_MODE ;
	pThis->m_WarpTime = 0 ;
	CVector3__Set(&pThis->m_vWarpPos,0,0,0) ;
	pThis->m_WarpXScale = 1.0 ;
	pThis->m_WarpYScale = 1.0 ;
	pThis->m_WarpSpin = 0 ;
}

/////////////////////////////////////////////////////////////////////////////
// Update warp
/////////////////////////////////////////////////////////////////////////////

#define CAMERA_WARP_OUT_TIME	SECONDS_TO_FRAMES(0.5)
#define CAMERA_WARP_IN_TIME	SECONDS_TO_FRAMES(0.5)

#define CAMERA_WARP_FINAL_X_SCALE	4.0
#define CAMERA_WARP_X_SPEED			2000

#define CAMERA_WARP_FINAL_Y_SCALE	4.0
#define CAMERA_WARP_Y_SPEED			1600

#define CAMERA_WARP_X_SCALE_RADIUS	(8.0/2)
#define CAMERA_WARP_Y_SCALE_RADIUS	(8.0/2)


void CCamera__UpdateWarp(CCamera *pThis)
{
	FLOAT		t,t1 ;
	FLOAT		sx,sy,z ;

	// Trigger warps
	if (GetApp()->m_Warp == WARP_NOT_WARPING)
		pThis->m_WarpMode = CAMERA_WARP_OFF_MODE ;
	else
	if ((GetApp()->m_Warp == WARP_WARPING_OUT) &&
		 (pThis->m_WarpMode != CAMERA_WARP_OUT_MODE))
		CCamera__WarpOut(pThis) ;
	else
	if ((GetApp()->m_Warp == WARP_WARPING_IN) &&
		 (pThis->m_WarpMode != CAMERA_WARP_IN_MODE))
		CCamera__WarpIn(pThis) ;

	// Process fx
	switch(pThis->m_WarpMode)
	{
		case CAMERA_WARP_OFF_MODE:
			CVector3__Set(&pThis->m_vWarpPos,0,0,0) ;
			pThis->m_WarpXScale = 1.0 ;
			pThis->m_WarpYScale = 1.0 ;
			pThis->m_WarpSpin = 0 ;
			break ;

		// Warping back to game
		case CAMERA_WARP_IN_MODE:

			// Update time
			pThis->m_WarpTime += frame_increment ;
			if (pThis->m_WarpTime >= CAMERA_WARP_IN_TIME)
				pThis->m_WarpTime = CAMERA_WARP_IN_TIME ;

			// Calculate tween time
			t = pThis->m_WarpTime / CAMERA_WARP_IN_TIME ;
			t = 1.0 - t ;
			t = t*t ;
			t = 1.0 - t ;
			t1 = 1.0 - t ;

			// Update spin
			pThis->m_WarpSpin = ANGLE_DTOR(-t1*t1*t1*t1*90) ;

			// Set position
			z = -15 * SCALING_FACTOR ;


			// Set wobble
/*
			sx = CAMERA_WARP_FINAL_X_SCALE + 
				  ((CAMERA_WARP_X_SCALE_RADIUS + 
					(CAMERA_WARP_X_SCALE_RADIUS * sin(ANGLE_DTOR(t1*CAMERA_WARP_X_SPEED)))) * t) ;

			sy = CAMERA_WARP_FINAL_Y_SCALE + 
				  ((CAMERA_WARP_Y_SCALE_RADIUS + 
					(CAMERA_WARP_Y_SCALE_RADIUS * sin(ANGLE_DTOR(t1*CAMERA_WARP_Y_SPEED)))) * t) ;
*/

			sx = CAMERA_WARP_FINAL_X_SCALE + 
				  ((CAMERA_WARP_X_SCALE_RADIUS  ) * t) ;

			sy = CAMERA_WARP_FINAL_Y_SCALE + 
				  ((CAMERA_WARP_Y_SCALE_RADIUS  ) * t) ;




			// Blend
			t = (pThis->m_WarpTime / CAMERA_WARP_IN_TIME) ;
			t = 1.0 - t ;
			pThis->m_WarpXScale = 1.0 + ((sx - 1.0) * t1) ;
			pThis->m_WarpYScale = 1.0 + ((sy - 1.0) * t1) ;
			pThis->m_vWarpPos.z = 0.0 + ((z - 0.0) * t1) ;
			break ;


		// Going into warp
		case CAMERA_WARP_OUT_MODE:

			// Update time
			pThis->m_WarpTime += frame_increment ;
			if (pThis->m_WarpTime >= CAMERA_WARP_OUT_TIME)
				pThis->m_WarpTime = CAMERA_WARP_OUT_TIME ;

			// Calculate tween time
			t = pThis->m_WarpTime / CAMERA_WARP_OUT_TIME ;
			t = 1.0 - t ;
			t = t*t ;
			t = 1.0 - t ;
			t1 = 1.0 - t ;

			// Update spin
			pThis->m_WarpSpin = ANGLE_DTOR(t*t*t*t*90) ;

			// Set position
			z = -15 * SCALING_FACTOR ;

			// Set wobble
/*
			sx = CAMERA_WARP_FINAL_X_SCALE + 
				  ((CAMERA_WARP_X_SCALE_RADIUS + 
					(CAMERA_WARP_X_SCALE_RADIUS * sin(ANGLE_DTOR(t*CAMERA_WARP_X_SPEED)))) * t1) ;

			sy = CAMERA_WARP_FINAL_Y_SCALE + 
				  ((CAMERA_WARP_Y_SCALE_RADIUS + 
					(CAMERA_WARP_Y_SCALE_RADIUS * sin(ANGLE_DTOR(t*CAMERA_WARP_Y_SPEED)))) * t1) ;
*/

			sx = CAMERA_WARP_FINAL_X_SCALE + 
				  ((CAMERA_WARP_X_SCALE_RADIUS) * t1) ;

			sy = CAMERA_WARP_FINAL_Y_SCALE + 
				  ((CAMERA_WARP_Y_SCALE_RADIUS) * t1) ;

			// Blend
			t = (pThis->m_WarpTime / CAMERA_WARP_OUT_TIME) ;

			t *= t;

			pThis->m_WarpXScale = 1.0 + ((sx - 1.0) * t) ;
			pThis->m_WarpYScale = 1.0 + ((sy - 1.0) * t) ;
			pThis->m_vWarpPos.z = 0.0 + ((z - 0.0) * t) ;
			break ;
	}
}

/////////////////////////////////////////////////////////////////////////////
// Coming out of a warp back to game play
/////////////////////////////////////////////////////////////////////////////
void CCamera__WarpIn(CCamera *pThis)
{
	pThis->m_WarpTime = 0 ;
	pThis->m_WarpMode = CAMERA_WARP_IN_MODE ;
}

/////////////////////////////////////////////////////////////////////////////
// Going into a warp gate or teleport pad
/////////////////////////////////////////////////////////////////////////////
void CCamera__WarpOut(CCamera *pThis)
{
	pThis->m_WarpTime = 0 ;
	pThis->m_WarpMode = CAMERA_WARP_OUT_MODE ;
}



/*****************************************************************************
*
*
*
*	GENERAL CODE
*
*
*
*****************************************************************************/



/*****************************************************************************
*
*	Function:		CCamera__Update()
*
******************************************************************************
*
*	Description:	General camera update function - sets up engine for displaying
*						world
*
*	Inputs:			*pThis	-	Ptr to camera
*
*	Outputs:			None
*
*****************************************************************************/

void CCamera__Update(CCamera *pThis)
{
#ifdef PLATFORM_PORT
	/* PORT: the camera follows the player (m_pCurrentRegion etc.), which is NULL on the
	 * legal/title screens (no level loaded). N64 tolerates the NULL derefs; a protected host
	 * faults. Skip the camera update until a level/player exists. (Title-screen camera setup
	 * for the M2 render path is a TODO once rendering is wired.) */
	if (!CEngineApp__GetPlayer(GetApp())) return;
#endif
	//Mtx						mProjection[2];
	CMtxF						mfLookAt,
								mfHead, mfEye, mfGround, mfPos,
								mfRot180, mfViewMat, mfView,
								mfTemp1, mfTemp2,
								mfPerspective, mfFlipX,
								mfDirection ;
	CQuatern					qRotX, qRotY, qRotZ,
								qTemp1, qTemp2;
	float 					viewLength;
	CVector3					vCorners[5] ;
	int						i;
	CFrameData				*pFData;
	LookAt					*pl;
	FLOAT						fov, aspect, speed, blend ;

	CEngineApp				*pEngine;

	FLOAT  					RotY,
								RotXOffset, RotYOffset, RotZOffset,
								XPos, YPos, ZPos,
								YPosOffset ;
	CQuatern					qGround ;

	FLOAT						Chop ;

	pEngine = GetApp();

	// Do warp fx
	CCamera__UpdateWarp(pThis) ;

	// Attract mode just finished?
	if (pEngine->m_CinemaFlags == -1)
		pThis->m_StartPressed = FALSE ;

	// Update movement ai
	if (CameraModeTable[pThis->m_Mode].Function)
		CameraModeTable[pThis->m_Mode].Function(pThis) ;

	// Clear vars
	pThis->m_NextCinematicStage = FALSE ;
	pThis->m_StartPressed = FALSE ;

	// Get current frame for correct view matrix and lookat structure
	pFData = CEngineApp__GetFrameData(pEngine);
	pl = &pFData->m_LookAt;
	guLookAtF(mfLookAt,
				 0, 0, 0,	// eye position
				 0, 0, 1,	// target position
				 0, 1, 0);	// up normal

	// Prepare engine variables during cinematic mode
	if (pThis->m_CinemaMode)
	{
		// Call collision to move turok during cinematics
		CCamera__KeepBelowCeiling(pThis) ;
		CTurokMovement.DeathStage = TMOVE_HEALTH_ALIVE ;

		// Acclaim logo is special case!!
		if (pThis->m_Mode == CAMERA_INTRO_ACCLAIM_LOGO_MODE)
		{
			// Setup eye/focus view
			CCameraView__CalculateXYRotation(&pThis->m_ViewA) ;

			// Blend eye/rotation biew
			CCameraView__Blend(&pThis->m_View, pThis->m_ViewBlend, &pThis->m_ViewA, &pThis->m_ViewB) ;
		}
		else
			CCameraView__CalculateXYRotation(&pThis->m_View) ;


//		rmonPrintf("Rot x,y,z: %f,%f,%f\r\n",
//					  ANGLE_RTOD(vRotation.x),
//					  ANGLE_RTOD(vRotation.y),
//					  ANGLE_RTOD(vRotation.z)) ;

		// Set track rotation vars
		RotXOffset = pThis->m_View.m_vRotation.x ;
		RotY = pThis->m_View.m_vRotation.y ;
		RotYOffset = 0 ;
		RotZOffset = pThis->m_View.m_vRotation.z ;
		CQuatern__Identity(&qGround) ;

		// Set track position vars
		XPos = pThis->m_View.m_vEye.x ;
		YPos = pThis->m_View.m_vEye.y ;
		YPosOffset = 0 ;
		ZPos = pThis->m_View.m_vEye.z ;
	}
	else
	{
		// Engine view from turok's eyes
		RotY = pEngine->m_RotY ;
		RotYOffset = pEngine->m_RotYOffset ;
		RotXOffset = pEngine->m_RotXOffset ;
		RotZOffset = -pEngine->m_RotZOffset ;
#ifdef PLATFORM_PORT
		{ extern char *getenv(const char*); extern double atof(const char*);
		  const char*pp=getenv("TUROK_PITCH"); if(pp) RotXOffset=(float)atof(pp);   /* debug: override camera pitch */
		  const char*py=getenv("TUROK_YAW");   if(py) RotYOffset=(float)atof(py);   /* debug: add to camera yaw (radians) */
		  if(getenv("TUROK_CAMLOG")){ extern int fprintf(void*,const char*,...); extern void*stderr; static int _c=0;
		    if(_c++<60) fprintf(stderr,"[camlog] pos=(%.0f,%.0f,%.0f) RotY=%.3f RotX=%.3f qGround=(%.3f,%.3f,%.3f,%.3f)\n",
		      pEngine->m_XPos,pEngine->m_YPos,pEngine->m_ZPos,pEngine->m_RotY,RotXOffset,
		      pEngine->m_qGround.x,pEngine->m_qGround.y,pEngine->m_qGround.z,pEngine->m_qGround.t); } }
#endif
		XPos = pEngine->m_XPos ;
		YPos = pEngine->m_YPos ;
		YPosOffset = pEngine->m_YPosOffset ;
		ZPos = pEngine->m_ZPos ;
		qGround =  pEngine->m_qGround ;
	}

	// Add tremor
	RotXOffset += pThis->m_vTremor.x ;
	YPos += pThis->m_vTremor.y ;
	RotZOffset += pThis->m_vTremor.z ;

#ifdef PLATFORM_PORT
	/* Camera-LOGIC anomaly detector (always-on, capped): a huge/NaN view angle or a non-unit ground
	 * quaternion makes the camera point wildly ("camera completely fucked up") while the resulting
	 * matrix elements stay bounded (sin/cos), so the gfx_pc matrix detectors miss it. Catch it at the
	 * source — names whether the corruption is a bad pitch/yaw/roll or a bad ground-slope (qGround,
	 * from collision under the moving player). */
	{ float qn = qGround.x*qGround.x + qGround.y*qGround.y + qGround.z*qGround.z + qGround.t*qGround.t;
	  int bad = (RotY!=RotY)||(RotXOffset!=RotXOffset)||(RotZOffset!=RotZOffset)||(RotYOffset!=RotYOffset)
	          ||(RotXOffset>100.0f)||(RotXOffset<-100.0f)||(RotZOffset>100.0f)||(RotZOffset<-100.0f)
	          ||(RotY>1000.0f)||(RotY<-1000.0f)||(qn!=qn)||(qn>4.0f)||(qn<0.25f);
	  if(bad){ extern int fprintf(void*,const char*,...); extern void*stderr; static int _c=0;
	    if(_c++<12) fprintf(stderr,"[CAMTRACK] camera anomaly: RotY=%.2f RotX=%.2f RotZ=%.2f RotYoff=%.2f |qGround|^2=%.3f pos=(%.0f,%.0f,%.0f)\n",
	      RotY,RotXOffset,RotZOffset,RotYOffset,qn,XPos,YPos,ZPos); } }
#endif

	// Add warp spin - THIS CRASHES LEVEL 4!!!!!
//	RotZOffset += pThis->m_WarpSpin ;

	// Calculate head rotation
	CQuatern__BuildFromAxisAngle(&qRotZ, 0,0,1, RotZOffset) ;
	CQuatern__BuildFromAxisAngle(&qRotX, 1,0,0, RotXOffset) ;
	CQuatern__BuildFromAxisAngle(&qRotY, 0,1,0, RotY + RotYOffset) ;

	CQuatern__Mult(&qTemp1, &qRotX, &qRotY) ;
	CQuatern__Mult(&pEngine->m_qViewAlignNoZ, &qTemp1, &qGround) ;							// used to make particles face screen
	CQuatern__Mult(&pEngine->m_qViewAlign, &qRotZ, &pEngine->m_qViewAlignNoZ) ;	// used to make particles face screen

	CQuatern__Mult(&qTemp2, &qRotZ, &qTemp1) ;
	CQuatern__ToMatrix(&qTemp2, mfHead) ;

	// Store camera position for ears
	pThis->m_vPos.x = XPos ;
	pThis->m_vPos.y = YPos+YPosOffset ;
	pThis->m_vPos.z = ZPos ;

	// Store camera rotation for ears
	pThis->m_vRotation.x = RotXOffset ;
	pThis->m_vRotation.y = RotY ;
	pThis->m_vRotation.z = RotZOffset ;



	// Eye heaight
	CMtxF__Translate(mfEye, 0, YPosOffset, 0) ;

	// Ground rot
	CQuatern__ToMatrix(&qGround, mfGround) ;

	// Player pos + screen shake
	XPos += CShake__Update(&pThis->m_ShakeX) ;
	YPos += CShake__Update(&pThis->m_ShakeY) ;
	ZPos += CShake__Update(&pThis->m_ShakeZ) ;
	CMtxF__Translate(mfPos, XPos, YPos, ZPos) ;

	// view orient
	CMtxF__PostMult(mfTemp1, mfHead, mfEye) ;
	CMtxF__PostMult(mfTemp2, mfTemp1, mfGround) ;

	CMtxF__PostMult(pEngine->m_mfViewOrient, mfTemp2, mfPos) ;
	CMtxF__RotateY(mfRot180, ANGLE_PI) ;
	CMtxF__PostMult(mfTemp1, mfRot180, pEngine->m_mfViewOrient) ;
	CMtxF__Invert(mfViewMat, mfTemp1) ;

	// projection
	CMtxF__PostMult(mfView, mfViewMat, mfLookAt) ;

#if WATER_WOBBLE
	// Add under water camera wobble
	if (		(CTurokMovement.WaterFlag == PLAYER_BELOW_WATERSURFACE)
			&&	(!CTurokMovement.InAntiGrav)
			&&	(!pThis->m_CinemaMode) )
	{
		// Perform shm
		pThis->m_XScale = 1+ (pThis->m_WaterWobbleXScale * ((1 + sin(pThis->m_WaterWobbleXAngle)) * 0.5)) ;
		pThis->m_YScale = 1+ (pThis->m_WaterWobbleYScale * ((1 + sin(pThis->m_WaterWobbleYAngle)) * 0.5)) ;
		pThis->m_WaterWobbleXAngle += frame_increment * pThis->m_WaterWobbleXSpeed ;
		pThis->m_WaterWobbleYAngle += frame_increment * pThis->m_WaterWobbleYSpeed ;

		// Get wobble speed from turok's speed
		speed = CTMove__GetSpeed(&CTurokMovement) * 2.5 ;
		if (speed < 0)
			speed = 0 ;
		else
		if (speed > 1)
			speed = 1 ;


		// Zoom to under water vars
		ProportionalZoomVarFLOAT(&pThis->m_WaterWobbleXScale,
										 CAMERA_UNDER_WATER_WOBBLE_X_SCALE,
										 CAMERA_ZOOM_TO_UNDER_WATER_WOBBLE_SCALE) ;

		ProportionalZoomVarFLOAT(&pThis->m_WaterWobbleYScale,
										 CAMERA_UNDER_WATER_WOBBLE_Y_SCALE,
										 CAMERA_ZOOM_TO_UNDER_WATER_WOBBLE_SCALE) ;

		ProportionalZoomVarFLOAT(&pThis->m_WaterWobbleXSpeed,
										 (speed * CAMERA_UNDER_WATER_WOBBLE_X_FAST_SPEED) +
										 ((1-speed) * CAMERA_UNDER_WATER_WOBBLE_X_SLOW_SPEED),
										 CAMERA_ZOOM_TO_UNDER_WATER_WOBBLE_SPEED) ;

		ProportionalZoomVarFLOAT(&pThis->m_WaterWobbleYSpeed,
										 (speed * CAMERA_UNDER_WATER_WOBBLE_Y_FAST_SPEED) +
										 ((1-speed) * CAMERA_UNDER_WATER_WOBBLE_Y_SLOW_SPEED),
										 CAMERA_ZOOM_TO_UNDER_WATER_WOBBLE_SPEED) ;
	}
	else
#endif
		pThis->m_XScale = pThis->m_YScale = 1 ;

#if TEST_LETTERBOX_VIEW
	pThis->m_LetterBoxScale = LETTERBOX_SCALE ;
#endif

	// Setup field of view and aspect ratio
	fov = pEngine->m_FieldOfView*pThis->m_YScale ;

	// Keep pixels square
	aspect = (SCREEN_WD*pThis->m_XScale)/(SCREEN_HT*pThis->m_YScale*(1-pThis->m_LetterBoxScale)) ;

	// Keep x scale at 1
//	aspect = (SCREEN_WD*pThis->m_XScale)/(SCREEN_HT*pThis->m_YScale) ;

	// Keep copy
	pThis->m_Fov = fov ;
	pThis->m_AspectRatio = aspect ;

	// Perspective
	guPerspectiveF(mfPerspective, &pThis->m_PerspNorm,
						fov,							// field of view
						aspect,						// aspect ratio
						SCALING_NEAR_CLIP,		// near clip
						pEngine->m_FarClip,	// far clip
						1);							// precision scaler

	CMtxF__Scale(mfFlipX, -1, 1, 1);


	CMtxF__PostMult4x4(mfTemp1, mfView, mfPerspective);
	CMtxF__PostMult4x4(pEngine->m_mfProjection, mfTemp1, mfFlipX);

	//CMtxF__ToMtx(pEngine->m_mfProjection, mProjection[even_odd]);

	CMtxF__Clamp(pEngine->m_mfProjection) ;
	CMtxF__ToMtx(pEngine->m_mfProjection, pFData->m_mProjection);

	// Update letter box
	Chop = SCREEN_HT * (pThis->m_LetterBoxScale/2) ;
	pThis->m_LetterBoxTop = Chop ;
	pThis->m_LetterBoxBottom = SCREEN_HT - Chop ;

	// Extract lookat from mfView
	CMtxF__Copy(mfDirection, mfView) ;

	// Rotate view direction for acclaim logo to keep reflection map moving
	if (pThis->m_Mode == CAMERA_INTRO_ACCLAIM_LOGO_MODE)
	{
		// Make blend go from 0 to 1
		blend = pThis->m_StageTime / SECONDS_TO_FRAMES(2.8) ;
		blend = COSINE_TWEEN(blend) ;

		CMtxF__PostMultRotateX(mfDirection, ANGLE_DTOR((blend*30) + (game_frame_number*0.5))) ;
		CMtxF__PostMultRotateY(mfDirection, ANGLE_DTOR((blend*50) + (-game_frame_number*0.4))) ;
	}

	CMtxF__Clamp(mfDirection) ;
	pl->l[0].l.dir[0] = FTOFRAC8(mfDirection[0][0]);
	pl->l[0].l.dir[1] = FTOFRAC8(mfDirection[1][0]);
	pl->l[0].l.dir[2] = FTOFRAC8(mfDirection[2][0]);
	pl->l[1].l.dir[0] = FTOFRAC8(mfDirection[0][1]);
	pl->l[1].l.dir[1] = FTOFRAC8(mfDirection[1][1]);
	pl->l[1].l.dir[2] = FTOFRAC8(mfDirection[2][1]);
	pl->l[0].l.col[0] = 0x00;
	pl->l[0].l.col[1] = 0x00;
	pl->l[0].l.col[2] = 0x00;
	pl->l[0].l.pad1 = 0x00;
	pl->l[0].l.colc[0] = 0x00;
	pl->l[0].l.colc[1] = 0x00;
	pl->l[0].l.colc[2] = 0x00;
	pl->l[0].l.pad2 = 0x00;
	pl->l[1].l.col[0] = 0x00;
	pl->l[1].l.col[1] = 0x80;
	pl->l[1].l.col[2] = 0x00;
	pl->l[1].l.pad1 = 0x00;
	pl->l[1].l.colc[0] = 0x00;
	pl->l[1].l.colc[1] = 0x80;
	pl->l[1].l.colc[2] = 0x00;
	pl->l[1].l.pad2 = 0x00;

//////////////////////////////////
//	// Should be off for specular highlights
//	gSPLookAt(pEngine->m_pDLP++, RSP_ADDRESS(pl));
//////////////////////////////////


	// Calculate view region (used for clipping)
	viewLength = SCALING_MAX_DIST(pEngine->m_FarClip, FIELD_OF_VIEW_ANGLE(fov, aspect));

	vCorners[0].x = 0;
	vCorners[0].y = 0;
	vCorners[0].z = 0;

	vCorners[1].x = viewLength*sin(ANGLE_DTOR(FIELD_OF_VIEW_ANGLE(fov, aspect)/2.0));
	vCorners[1].y = vCorners[1].x*aspect;
	vCorners[1].z = -pEngine->m_FarClip;

	vCorners[2].x = -vCorners[1].x;
	vCorners[2].y = vCorners[1].y;
	vCorners[2].z = vCorners[1].z;

	vCorners[3].x = vCorners[1].x;
	vCorners[3].y = -vCorners[1].y;
	vCorners[3].z = vCorners[1].z;

	vCorners[4].x = -vCorners[1].x;
	vCorners[4].y = -vCorners[1].y;
	vCorners[4].z = vCorners[1].z;

	for (i=0; i<5; i++)
		CMtxF__VectorMult(pEngine->m_mfViewOrient, &vCorners[i], &pEngine->m_vTCorners[i]);

	CViewVolume__BuildFromVectors(&view_volume,
											&pEngine->m_vTCorners[0],
											&pEngine->m_vTCorners[1], &pEngine->m_vTCorners[2],
											&pEngine->m_vTCorners[3], &pEngine->m_vTCorners[4]);

	// Rectangle enclosing view region
	CBoundsRect__EncloseVectors(&view_bounds_rect, 5, pEngine->m_vTCorners);

	// Animate objects within FAR_CLIP in all directions
	anim_bounds_rect.m_MinX = pEngine->m_vTCorners[0].x - viewLength;
	anim_bounds_rect.m_MaxX = pEngine->m_vTCorners[0].x + viewLength;
	anim_bounds_rect.m_MinZ = pEngine->m_vTCorners[0].z - viewLength;
	anim_bounds_rect.m_MaxZ = pEngine->m_vTCorners[0].z + viewLength;
}


void CCamera__DisplayListSetup(CCamera *pThis, Gfx **ppDLP)
{
	CFrameData				*pFData;

	INT16	ScaleX, ScaleY ;
	INT16	ScissorLeft = 0 ;
	INT16	ScissorRight = SCREEN_WD ;
	INT16	ScissorTop = 0 ;
	INT16	ScissorBottom = SCREEN_HT ;


	pThis->m_PixelXScale = 1 ;
	pThis->m_PixelYScale = 1 ;

	if (CCamera__InCinemaMode(pThis))
	{
		// Set viewport scale!
		ScaleX = SCREEN_WD * 2 ;
		ScaleY = SCREEN_HT*(1.0 - pThis->m_LetterBoxScale) * 2 ;

		// Set scissor
		ScissorLeft = 0 ;
		ScissorRight = SCREEN_WD ;
		ScissorTop = pThis->m_LetterBoxTop ;
		ScissorBottom = pThis->m_LetterBoxBottom ;
	}
	else
	{
		pThis->m_PixelXScale = 1 * pThis->m_WarpXScale ;
		pThis->m_PixelYScale = 1 * pThis->m_WarpYScale ;

		ScaleX = ((float)SCREEN_WD * 2 * pThis->m_PixelXScale) ;
		ScaleY = ((float)SCREEN_HT * 2 * pThis->m_PixelYScale) ;

		ScissorLeft = (SCREEN_WD / 2) - (pThis->m_PixelXScale * (SCREEN_WD/2)) ;
		ScissorRight = (SCREEN_WD / 2) + (pThis->m_PixelXScale * (SCREEN_WD/2)) ;
		ScissorTop = (SCREEN_HT / 2) - (pThis->m_PixelYScale * (SCREEN_HT/2)) ;
		ScissorBottom = (SCREEN_HT / 2) + (pThis->m_PixelYScale * (SCREEN_HT/2)) ;
	}

	// Range check scissor
	if (ScissorRight > SCREEN_WD)
		ScissorRight = SCREEN_WD ;

	if (ScissorLeft < 0)
		ScissorLeft = 0 ;

	if (ScissorTop < 0)
		ScissorTop = 0 ;

	if (ScissorBottom > SCREEN_HT)
		ScissorBottom = SCREEN_HT ;

  	pFData = CEngineApp__GetFrameData(GetApp());

	gSPMatrix((*ppDLP)++, RSP_ADDRESS(&pFData->m_mProjection),
	   		 G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
	gSPPerspNormalize((*ppDLP)++, pThis->m_PerspNorm);

	CameraVp[even_odd].vp.vscale[0] = ScaleX ;
	CameraVp[even_odd].vp.vscale[1] = ScaleY ;
  	gSPViewport((*ppDLP)++, &CameraVp[even_odd]) ;

	gDPSetScissor((*ppDLP)++,
					  G_SC_NON_INTERLACE, ScissorLeft, ScissorTop,
					  ScissorRight, ScissorBottom) ;

  	// should be off for specular highlights
	gSPLookAt((*ppDLP)++, RSP_ADDRESS(&pFData->m_LookAt));
}



/*****************************************************************************
*
*	Function:		CCamera__SetupMode()
*
******************************************************************************
*
*	Description:	Sets up a new camera mode and calls any special case setup
*						functions.
*
*	Inputs:			*pThis	-	Ptr to camera
*						NewMode	-	New mode id
*
*	Outputs:			None
*
*****************************************************************************/
void CCamera__SetupMode(CCamera *pThis, INT32 NewMode)
{
		// Setup defaults
		pThis->m_Mode = NewMode ;
		pThis->m_Time = 0 ;
		pThis->m_Stage = 0 ;
		pThis->m_StageTime = 0 ;

		pThis->m_CinemaFadingOut = FALSE ;
		pThis->m_CinemaMode = FALSE ;
		pThis->m_LetterBoxScale = 0 ;

		// Setup special cases
		if (CameraModeTable[pThis->m_Mode].SetupFunction)
			CameraModeTable[pThis->m_Mode].SetupFunction(pThis) ;
}




/*****************************************************************************
*
*	Function:		CCamera__TurokGoneBelowWater()
*
******************************************************************************
*
*	Description:	Starts water wobble effect
*
*	Inputs:			*pThis	-	Ptr to camera
*
*	Outputs:			None
*
*****************************************************************************/
void CCamera__TurokGoneBelowWater(CCamera *pThis)
{
	// Prepare for water wobble
	pThis->m_WaterWobbleXScale = CAMERA_ENTER_WATER_WOBBLE_X_SCALE ;
	pThis->m_WaterWobbleYScale = CAMERA_ENTER_WATER_WOBBLE_Y_SCALE ;
	pThis->m_WaterWobbleXSpeed = CAMERA_ENTER_WATER_WOBBLE_X_SPEED ;
	pThis->m_WaterWobbleYSpeed = CAMERA_ENTER_WATER_WOBBLE_Y_SPEED ;
	pThis->m_WaterWobbleXAngle = 0 ;
	pThis->m_WaterWobbleYAngle = 0 ;
}


/*****************************************************************************
*
*	Function:		CCamera__PosIsCollisionFree()
*
******************************************************************************
*
*	Description:	Checks to see if a position has no collision within a radius
*						around it by casting out 16 rays.
*
*	Inputs:			*pThis	- Ptr to camera
*						*pvPos	- Ptr to position to check
*						Radius  	- Radius around pt to check
*
*	Outputs:			TRUE	if no collision around pos, else FALSE.
*
*****************************************************************************/
BOOL CCamera__PosIsCollisionFree(CCamera *pThis, CVector3 *pvPos, FLOAT Radius)
{
	FLOAT		Rot ;
	CVector3	vPos ;
	INT32		i ;
	CGameObjectInstance *pTurok = CEngineApp__GetPlayer(GetApp()) ;

	vPos.y = pvPos->y ;

	// Setup position
	Rot = 0 ;
	i = 360/16 ;
	while(i--)
	{
		// Check position
		vPos.x = (Radius * sin(Rot)) +  pvPos->x ;
		vPos.z = (Radius * cos(Rot)) +  pvPos->z ;
		if (CAnimInstanceHdr__IsObstructed(&pTurok->ah, vPos, NULL))
			return FALSE ;

		// Try next angle
		Rot += ANGLE_DTOR(360/16) ;
	}

	return TRUE ;
}




BOOL CGameObjectInstance__CylinderCollisionFree(CGameObjectInstance *pThis, FLOAT Radius)
{
	FLOAT		Rot ;
	CVector3	vPos ;
	INT32		i ;

	// Setup start position
	vPos.y = pThis->ah.ih.m_vPos.y ;
	Rot = 0 ;
	i = 360/32 ;
	while(i--)
	{
		// Check position
		vPos.x = (Radius * sin(Rot)) + pThis->ah.ih.m_vPos.x ;
		vPos.z = (Radius * cos(Rot)) + pThis->ah.ih.m_vPos.z ;
		if (CAnimInstanceHdr__IsObstructed(&pThis->ah, vPos, NULL))
			return FALSE ;

		// Try next angle
		Rot += ANGLE_DTOR(360/32) ;
	}

	return TRUE ;
}



/*****************************************************************************
*
*	Function:		CCamera__GetCollisionFreePos()
*
******************************************************************************
*
*	Description:	Tries to find a collision free position by circling around
*						a position.
*
*	Inputs:			*pThis			- Ptr to camera
*						*pvCurrentPos	- Start pos to look from
*						*pvOutPos		- Ptr to final position stoarge
*						Radius			- Max radius to check
*												+ve radius = check from 0 to abs(radius)
*												-ve radius = check abs(radius) to 0
*
*	Outputs:			*pvOutPos		= Collision free position
*
*****************************************************************************/
void CCamera__GetCollisionFreePos(CCamera *pThis, CVector3 *pvCurrentPos,
											 CVector3 *pvOutPos, FLOAT EndRadius)
{
	FLOAT		Rot, Radius, DeltaRadius ;
	CVector3	vPos ;
	BOOL		Found ;
	vPos.y = pvCurrentPos->y ;

	// Setup position
	Found = FALSE ;
	Rot = 0 ;

	// Setup radius direction
	if (EndRadius > 0)
	{
		Radius = 0 ;
		DeltaRadius = 5 * SCALING_FACTOR ;
	}
	else
	{
		EndRadius = -EndRadius ;
		Radius = EndRadius ;
		DeltaRadius = -5 * SCALING_FACTOR ;
	}

	while(!Found)
	{
		// Check position
		vPos.x = Radius* sin(Rot) +  pvCurrentPos->x ;
		vPos.z = Radius * cos(Rot) +  pvCurrentPos->z ;

		// Check pos
		if (CCamera__PosIsCollisionFree(pThis, &vPos, 10*SCALING_FACTOR))
			Found = TRUE ;
		else
		{
			// Try next angle
			Rot += ANGLE_DTOR(360/16) ;

			// If done 360, make radius smaller
			if (Rot >= ANGLE_DTOR(360))
			{
				Rot = 0 ;
				Radius += DeltaRadius ;

				// Stop infinite loop
				if (((DeltaRadius > 0) && (Radius > EndRadius)) ||
					 ((DeltaRadius < 0) && (Radius < 0)))
				{
					vPos.x = pvCurrentPos->x ;
					vPos.z = pvCurrentPos->z ;
					Found = TRUE ;
				}
			}
		}
	}

	*pvOutPos = vPos ;
}




/*****************************************************************************
*
*	Function:		CCamera__SetTurokPosition()
*
******************************************************************************
*
*	Description:	Sets turok, as close as possible, to given position.
*
*	Inputs:			*pThis		- Ptr to camera
*						vDesired		- Desired position
*
*	Outputs:			None
*
*****************************************************************************/
void CCamera__SetTurokPosition(CCamera *pThis, CVector3 vDesired)
{
	CGameObjectInstance *pTurok = CEngineApp__GetPlayer(GetApp()) ;
	CVector3			vPos ;
	CGameRegion		*pRegion ;

	// Get closest pos
	CInstanceHdr__GetNearPositionAndRegion(&pTurok->ah.ih,
														vDesired,
														&vPos,
														&pRegion,
														INTERSECT_BEHAVIOR_IGNORE, INTERSECT_BEHAVIOR_IGNORE);

	// Setup position stuff
	pTurok->ah.ih.m_vPos.x = vPos.x ;
	pTurok->ah.ih.m_vPos.z = vPos.z ;
	pTurok->ah.ih.m_pCurrentRegion = pRegion ;
}





/*****************************************************************************
*
*	Function:		CCameraView__Blend()
*
******************************************************************************
*
*	Description:	Blends two camera views to produce a new view
*
*	Inputs:			*pThis		-	Ptr to destination camera view
*						Time			-	Blend time (0 to 1)
*						*pViewA		-	Ptr to view a (when t = 0)
*						*pViewB		-	Ptr to view b (when t = 1)
*
*	Outputs:			None
*
*****************************************************************************/
void CCameraView__Blend(CCameraView *pThis, FLOAT Time, CCameraView *pViewA, CCameraView *pViewB)
{
	CVector3__Blend(&pThis->m_vEye, Time, &pViewA->m_vEye, &pViewB->m_vEye) ;
	CVector3__Blend(&pThis->m_vFocus, Time, &pViewA->m_vFocus, &pViewB->m_vFocus) ;
	CVector3__Blend(&pThis->m_vRotation, Time, &pViewA->m_vRotation, &pViewB->m_vRotation) ;
}




/*****************************************************************************
*
*	Function:		CCameraView__TweenPosTrack()
*
******************************************************************************
*
*	Description:	Blends camera between two positions.
*						(see "CCameraPosTrackInfo" structure for details)
*
*	Inputs:			*pThis		-	Ptr to destination camera view
*						*pInfo		-	Ptr to position info
*						Time			-	Blend time (0 to 1)
*
*	Outputs:			None
*
*****************************************************************************/
void CCameraView__TweenPosTrack(CCameraView *pThis, CCameraPosTrackInfo *pInfo, FLOAT Time)
{
	// Range check time
	if (Time < 0)
		Time = 0 ;
	else
	if (Time > 1)
		Time = 1 ;

	// Blend everything
	CVector3__Blend(&pThis->m_vEye, Time, &pInfo->m_vStartEye, &pInfo->m_vFinalEye) ;
	CVector3__Blend(&pThis->m_vFocus, Time, &pInfo->m_vStartFocus, &pInfo->m_vFinalFocus) ;
	CVector3__Blend(&pThis->m_vRotation, Time, &pInfo->m_vStartRotation, &pInfo->m_vFinalRotation) ;
}




/*****************************************************************************
*
*	Function:		CCameraView__TweenTrackAroundPos()
*
******************************************************************************
*
*	Description:	Blends camera between two points with rotation around a point
*						(see "CCameraTrackInfo" structure for details)
*
*	Inputs:			*pThis		-	Ptr to destination camera view
*						*pViewA		-	Ptr to view a (when t = 0)
*						*pViewB		-	Ptr to view b (when t = 1)
*						Time			-	Blend time (0 to 1)
*						*pvPos		-	Ptr to position to rotate around
*						RotYOffset	-	Rotation off about y-axis
*
*	Outputs:			None
*
*****************************************************************************/
void CCameraView__TweenTrackAroundPos(CCameraView *pThis, CCameraTrackInfo *pInfo, FLOAT Time, CVector3 *pvPos, FLOAT RotYOffset)
{
	FLOAT		Rot ;

	// Range check time
	if (Time < 0)
		Time = 0 ;
	else
	if (Time > 1)
		Time = 1 ;

	// Zoom rotation,radius etc
	pThis->m_YRotOffset		= BlendFLOAT(Time, pInfo->m_StartYRotOffset,		pInfo->m_FinalYRotOffset) ;
	pThis->m_Radius			= BlendFLOAT(Time, pInfo->m_StartRadius,			pInfo->m_FinalRadius) ;
	pThis->m_EyeYOffset		= BlendFLOAT(Time, pInfo->m_StartEyeYOffset,		pInfo->m_FinalEyeYOffset) ;
	pThis->m_FocusYOffset	= BlendFLOAT(Time, pInfo->m_StartFocusYOffset,	pInfo->m_FinalFocusYOffset) ;

	// Calculate position
	Rot = RotYOffset + pThis->m_YRotOffset ;
	pThis->m_vEye.x = pvPos->x - (pThis->m_Radius * sin(Rot)) ;
	pThis->m_vEye.y = pvPos->y + pThis->m_EyeYOffset ;
	pThis->m_vEye.z = pvPos->z - (pThis->m_Radius * cos(Rot)) ;

	// Look at turok
	pThis->m_vFocus.x = pvPos->x ;
	pThis->m_vFocus.y = pvPos->y + pThis->m_FocusYOffset ;
	pThis->m_vFocus.z = pvPos->z ;
}




/*****************************************************************************
*
*	Function:		CCameraView__TweenTrackAroundObject()
*
******************************************************************************
*
*	Description:	Blends camera between two points with rotation around an object
*						(see "CCameraTrackInfo" structure for details)
*
*	Inputs:			*pThis		-	Ptr to destination camera view
*						*pViewA		-	Ptr to view a (when t = 0)
*						*pViewB		-	Ptr to view b (when t = 1)
*						Time			-	Blend time (0 to 1)
*						*pObject		-	Ptr to object to rotate around
*
*	Outputs:			None
*
*****************************************************************************/
void CCameraView__TweenTrackAroundObject(CCameraView *pThis, CCameraTrackInfo *pInfo, FLOAT Time, CGameObjectInstance *pObject)
{
	CCameraView__TweenTrackAroundPos(pThis, pInfo, Time, &pObject->ah.ih.m_vPos, pObject->m_RotY) ;
}



/*****************************************************************************
*
*	Function:		CCameraView__TweenTrackAroundTurok()
*
******************************************************************************
*
*	Description:	Blends camera between two points with rotation around Turok
*						(see "CCameraTrackInfo" structure for details)
*
*	Inputs:			*pThis		-	Ptr to destination camera view
*						*pViewA		-	Ptr to view a (when t = 0)
*						*pViewB		-	Ptr to view b (when t = 1)
*						Time			-	Blend time (0 to 1)
*
*	Outputs:			None
*
*****************************************************************************/
void CCameraView__TweenTrackAroundTurok(CCameraView *pThis, CCameraTrackInfo *pInfo, FLOAT Time)
{
	CCameraView__TweenTrackAroundObject(pThis, pInfo, Time, CEngineApp__GetPlayer(GetApp())) ;
}





/*****************************************************************************
*
*	Function:		CCamera__ZoomEyeToPos()
*
******************************************************************************
*
*	Description:	Gradually moves camera eye to specified position
*
*	Inputs:			*pThis	-	Ptr to camera
*						*pvPos	-	Ptr to destination position
*						Speed		-	Movement factor
*
*	Outputs:			None
*
*****************************************************************************/
void CCameraView__ZoomEyeToPos(CCameraView *pThis, CVector3 *pvPos, FLOAT Speed)
{
	ProportionalZoomVarFLOAT(&pThis->m_vEye.x, pvPos->x, Speed) ;
	ProportionalZoomVarFLOAT(&pThis->m_vEye.y, pvPos->y, Speed) ;
	ProportionalZoomVarFLOAT(&pThis->m_vEye.z, pvPos->z, Speed) ;
}



/*****************************************************************************
*
*	Function:		CCamera__ZoomFocusToPos()
*
******************************************************************************
*
*	Description:	Gradually moves camera focus to specified position
*
*	Inputs:			*pThis	-	Ptr to camera
*						*pvPos	-	Ptr to destination position
*						Speed		-	Movement factor
*
*	Outputs:			None
*
*****************************************************************************/
void CCameraView__ZoomFocusToPos(CCameraView *pThis, CVector3 *pvPos, FLOAT Speed)
{
	ProportionalZoomVarFLOAT(&pThis->m_vFocus.x, pvPos->x, Speed) ;
	ProportionalZoomVarFLOAT(&pThis->m_vFocus.y, pvPos->y, Speed) ;
	ProportionalZoomVarFLOAT(&pThis->m_vFocus.z, pvPos->z, Speed) ;
}





void CCamera__KeepBelowCeiling(CCamera *pThis)
{
	CGameObjectInstance	*pTurok = CEngineApp__GetPlayer(GetApp()) ;
	CROMRegionSet			*pRegionSet ;
	FLOAT	Ceiling ;

	if (pThis->m_Mode == CAMERA_CINEMA_TUROK_KILL_TREX_MODE)
	{
		pTurok = pThis->m_pObject ;
	}

	// Keep camera below ceiling
	pRegionSet = CScene__GetRegionAttributes(&GetApp()->m_Scene, pTurok->ah.ih.m_pCurrentRegion) ;
	if ((pRegionSet) && (pRegionSet->m_dwFlags & REGFLAG_CEILING))
	{
		Ceiling = CGameRegion__GetCeilingHeight(pTurok->ah.ih.m_pCurrentRegion,
															 pThis->m_View.m_vEye.x,
															 pThis->m_View.m_vEye.z) ;
		Ceiling -= 1 * SCALING_FACTOR ;
		if (pThis->m_View.m_vEye.y > Ceiling)
			pThis->m_View.m_vEye.y =  Ceiling ;
	}
}



BOOL CCamera__CeilingHighEnoughAboveTurok(CCamera *pThis)
{
	CGameObjectInstance	*pTurok = CEngineApp__GetPlayer(GetApp()) ;
	CROMRegionSet			*pRegionSet ;
	FLOAT	Ceiling ;

	// Is there enough room above turok?
	pRegionSet = CScene__GetRegionAttributes(&GetApp()->m_Scene, pTurok->ah.ih.m_pCurrentRegion) ;
	if ((pRegionSet) && (pRegionSet->m_dwFlags & REGFLAG_CEILING))
	{
		Ceiling = CGameRegion__GetCeilingHeight(pTurok->ah.ih.m_pCurrentRegion, pTurok->ah.ih.m_vPos.x, pTurok->ah.ih.m_vPos.z) ;
		if ((Ceiling - pTurok->ah.ih.m_vPos.y) < (10*SCALING_FACTOR))
			return FALSE ;
	}

	return TRUE ;
}






/*****************************************************************************
*
*
*
*	CHECK FUNCTIONS
*
*
*
*****************************************************************************/



/*****************************************************************************
*
*	Function:		CCamera__FadeToCinema()
*
******************************************************************************
*
*	Description:	Fades game down to black and sets cinema flag. Also keeps
*						Turok in same position
*						NOTE: CCamera__Construct must check for the flag to start
*								cinema
*
*	Inputs:			*pThis		-	Ptr to camera
*						CinemaFlag	-	Cinema flag from tengine.h
*
*	Outputs:			None.
*
*****************************************************************************/
void CCamera__FadeToCinema(CCamera *pThis, INT32 CinemaFlag)
{
	CEngineApp				*pEngine = GetApp() ;
	CGameObjectInstance	*pTurok = CEngineApp__GetPlayer(GetApp()) ;

	// Cause reset level to goto cinema warp
	pEngine->m_UseCinemaWarp = TRUE ;
	pEngine->m_CinemaFlags |= CinemaFlag ;

	// Save warp position
	pEngine->m_CinemaWarp.m_vPos = pTurok->ah.ih.m_vPos ;
	pEngine->m_CinemaWarp.m_RotY = pTurok->m_RotY ;
	pEngine->m_CinemaWarp.m_nLevel = pEngine->m_Scene.m_nLevel ;
	pEngine->m_CinemaWarp.m_nRegion = CScene__GetRegionIndex(&pEngine->m_Scene, pTurok->ah.ih.m_pCurrentRegion) ;

	// Reset the level so object can be allocated!
	CEngineApp__SetupFadeTo(pEngine, MODE_RESETLEVEL) ;
}


BOOL CCamera__CheckForTurokDeathCinematic(CCamera *pThis)
{
#if DO_CINEMATICS

	CEngineApp				*pEngine = GetApp() ;
	CGameObjectInstance	*pTurok ;
	CROMRegionSet			*pRegionSet ;
	BOOL						climbFlag;


	// If in cinema mode - return TRUE!
	if (pThis->m_CinemaMode)
		return TRUE ;

	// Waiting to do a cinematic?
	if (pEngine->m_CinemaFlags)
		return TRUE ;

	// Get turok ptr
	pTurok = CEngineApp__GetPlayer(GetApp()) ;
	if (!pTurok)
		return FALSE ;

	// if in keel over don't do a cinema
	if (CTurokMovement.DeathStage != TMOVE_HEALTH_ALIVE)
		return FALSE;

	// if turok is on a cliff - just return
	climbFlag = CEngineApp__GetPlayerClimbStatus(GetApp());
	if (climbFlag == PLAYER_AT_CLIMBING_SURFACE)
		return FALSE;

	// Get region ptr
	pRegionSet = CScene__GetRegionAttributes(&GetApp()->m_Scene, pTurok->ah.ih.m_pCurrentRegion) ;
	if (!pRegionSet)
		return FALSE ;

	// Do fall death?
	if (		(pRegionSet->m_dwFlags & REGFLAG_FALLDEATH)
			&&	(pTurok->ah.ih.m_vPos.y < CInstanceHdr__GetGroundHeight(&pTurok->ah.ih)) )
	{
		pEngine->m_CinemaFlags |= CINEMA_FLAG_PLAY_FALL_DEATH;
	}

	// Do water death?
	else if (		(pTurok->m_AI.m_Health <= 0)
					&&	(!CTurokMovement.InAntiGrav)
					&& (CCamera__CeilingHighEnoughAboveTurok(pThis))
					&& (		(CTurokMovement.WaterFlag == PLAYER_BELOW_WATERSURFACE)
							||	(		(CTurokMovement.WaterFlag == PLAYER_ON_WATERSURFACE)
									&&	(!CInstanceHdr__IsOnGround(&pTurok->ah.ih)) ) ) )
	{
		pEngine->m_CinemaFlags |= CINEMA_FLAG_PLAY_WATER_DEATH;
	}

	// Do normal death?
	else if (		(CInstanceHdr__IsOnGround(&pTurok->ah.ih))
					&&	(pTurok->m_AI.m_Health <= 0)
					&&	(CCamera__CeilingHighEnoughAboveTurok(pThis)) )
	{
		// if a boss is active, set up special death cinema...
		if ((GetApp()->m_pBossActive) && (BOSS_Health(GetApp()->m_pBossActive) > 0))
		{
			switch (CGameObjectInstance__TypeFlag(GetApp()->m_pBossActive))
			{
				case AI_OBJECT_CHARACTER_MANTIS_BOSS:
					pEngine->m_CinemaFlags |= CINEMA_FLAG_MANTIS_KILL_TUROK;
					break;
				case AI_OBJECT_CHARACTER_TREX_BOSS:
					pEngine->m_CinemaFlags |= CINEMA_FLAG_TREX_KILL_TUROK;
					break;
				case AI_OBJECT_CHARACTER_CAMPAIGNER_BOSS:
					pEngine->m_CinemaFlags |= CINEMA_FLAG_CAMPAIGNER_KILL_TUROK;
					break;
				default:
					pEngine->m_CinemaFlags |= CINEMA_FLAG_PLAY_DEATH;
					break;
			}
		}
		else
		{
			pEngine->m_CinemaFlags |= CINEMA_FLAG_PLAY_DEATH;
		}
	}

	// Perform a cinema?
	if (pEngine->m_CinemaFlags)
	{
		CCamera__FadeToCinema(pThis, pEngine->m_CinemaFlags) ;
		return TRUE ;
	}
	else
		return FALSE ;

#else

	return (pThis->m_CinemaMode) ;

#endif
}


BOOL CCamera__InCinemaMode(CCamera *pThis)
{
	return (pThis->m_CinemaMode) ;
}



BOOL CCamera__UpdateTurokHead(CCamera *pThis)
{
	if (CCamera__InCinemaMode(pThis))
		return FALSE ;
	else
		return TRUE ;
}


BOOL CCamera__UpdateTurokPosition(CCamera *pThis)
{
	return (!CCamera__InCinemaMode(pThis)) ;
}



BOOL CCamera__InLetterBoxMode(CCamera *pThis)
{
#if TEST_LETTERBOX_VIEW
	return TRUE ;
#else
	return (pThis->m_LetterBoxScale != 0.0) ;
#endif
}






/*****************************************************************************
*
*
*
*	GAME MODE CODE
*
*
*
*****************************************************************************/




/////////////////////////////////////////////////////////////////////////////
//	Modes:	  	 CAMERA_TUROK_EYE_MODE
//	Description: Camera being at turok's eye
/////////////////////////////////////////////////////////////////////////////
void CCamera__Code_TUROK_EYE(CCamera *pThis)
{
}





