// camera.h
/////////////////////////////////////////////////////////////////////////////

#ifndef _INC_CAMERA
#define _INC_CAMERA

#include <stddef.h>
#include "spec.h"
#include "defs.h"
#include "scene.h"
#include "graphu64.h"
#include "romstruc.h"
#include "onscrn.h"
#include "volume.h"
#include "frontend.h"
#include "pause.h"
#include "boss.h"
#include "fx.h"
#include "options.h"
#include <sched.h>


/////////////////////////////////////////////////////////////////////////////
// Constants
/////////////////////////////////////////////////////////////////////////////
#define LETTERBOX_SCALE				0.4	// Y Scale of screen height

#define CAMERA_VIEW_A_BLEND		0.0
#define CAMERA_VIEW_B_BLEND		1.0

#define MAX_CAMERA_SOUND_HANDLES	8		// No of user

/////////////////////////////////////////////////////////////////////////////
// Structures
/////////////////////////////////////////////////////////////////////////////


// Tween track structure
typedef struct CCameraTrackInfo_t
{
	FLOAT	m_StartYRotOffset,	m_FinalYRotOffset ;
	FLOAT	m_StartRadius,			m_FinalRadius ;
	FLOAT	m_StartEyeYOffset,	m_FinalEyeYOffset ;
	FLOAT	m_StartFocusYOffset,	m_FinalFocusYOffset ;
} CCameraTrackInfo ;


// Position track structure
typedef struct CCameraPosTrackInfo_t
{
	CVector3	m_vStartEye,	m_vFinalEye ;
	CVector3	m_vStartFocus,	m_vFinalFocus ;
	CVector3	m_vStartRotation,	m_vFinalRotation ;
} CCameraPosTrackInfo ;


// Camera view class
typedef struct CCameraView_t
{
	FLOAT			m_Time ;				// Misc time
	CVector3		m_vEye ;				// Camera position
	CVector3		m_vFocus ;			// Camera focus position
	CVector3		m_vRotation ;		// Rotations
	FLOAT			m_YRotOffset,
					m_Radius, 
					m_EyeYOffset, 
					m_FocusYOffset ;
} CCameraView ;


// Screen shake structure
typedef struct CShake_t
{
	FLOAT	m_Shake ;		// Current shake value (set this to start a shake)
	FLOAT	m_LastShake ;	// Last frames shake
	FLOAT	m_Amp ;			// Shake amplitude
	FLOAT	m_Angle ;		// Shake sine angle
} CShake ;


// Camera class
typedef struct CCamera_t
{
	// Used for display
	float		m_PixelXScale ;
	float		m_PixelYScale ;

	// Mode vars
	INT16		m_Mode ;						// Current tracking mode
	FLOAT		m_Time ;						// Time in mode
	BOOL		m_CinemaMode ;				// TRUE if in cinema mode
	BOOL		m_CinemaFadingOut ;		// TRUE if cinema is fading out
	BOOL		m_NextCinematicStage ;	// Events change this
	BOOL		m_StartPressed ;
	INT16		m_Stage ;				// Current stage
	FLOAT		m_StageTime ;			// Current stage time

	// Letter box vars
	u16 		m_PerspNorm;
	FLOAT		m_LetterBoxScale ;	// Letter box scale (0 = full screen, 1 = infinite)
	INT16		m_LetterBoxTop ;		// Top clipping
	INT16		m_LetterBoxBottom ;	// Bottom clipping

	// View vars
	FLOAT						m_Fov, m_AspectRatio ; // Current settings
	CCameraView				m_View ;
	FLOAT						m_ViewBlend ;
	CCameraView				m_ViewA, m_ViewB, m_ViewC ;	// Misc views (used for blending!)

	CVector3					m_vPos ;				// current position of the camera (used for ears)
	CVector3					m_vRotation ;		// current rotation of the camera "" ""

	// Object tracking vars
	CGameObjectInstance *m_pObject ;					// Object to track

	// Movement vars
	CCameraTrackInfo		*m_pTrackInfo ;		// Ptr to track info
	CVector3					m_vPos1, m_vPos2 ;	// Misc position vars

	// Water wobble vars
	FLOAT	m_XScale, m_YScale ;
	FLOAT	m_WaterWobbleXScale, m_WaterWobbleYScale ;
	FLOAT	m_WaterWobbleXAngle, m_WaterWobbleYAngle ;
	FLOAT	m_WaterWobbleXSpeed, m_WaterWobbleYSpeed ;

	// Screen shake vars
	CShake	m_ShakeX, m_ShakeY, m_ShakeZ ;
	CVector3	m_vTremor ;

	// Warp effect vars
	INT32		m_WarpMode ;			// Warping mode
	FLOAT		m_WarpTime ;			// Current warp time
	CVector3	m_vWarpPos ;			// Position of warp offset
	FLOAT		m_WarpXScale ;			// Field of view x scale
	FLOAT		m_WarpYScale ;			// Field of view y scale
	FLOAT		m_WarpSpin ;			// Z rotation spin factor

	// Misc sound handles (used to cut off exiting cinemas)
	INT32		m_UserSoundHandles[MAX_CAMERA_SOUND_HANDLES] ;

	INT32		m_Flags ;				// Misc flags
} CCamera ;


// Camera mode info class
typedef struct s_CCameraModeInfo
{
	void(*SetupFunction)(CCamera *pThis) ;	/* Ptr to mode function */
	void(*Function)(CCamera *pThis) ;		/* Ptr to mode function */
} CCameraModeInfo ;



/////////////////////////////////////////////////////////////////////////////
// Initialize functions
/////////////////////////////////////////////////////////////////////////////
void CCamera__PreSceneConstruct(CCamera *pThis) ;
void CCamera__Construct(CCamera *pThis) ;


void CCamera__ConstructIntro(CCamera *pThis) ;
void CCamera__ConstructGallery(CCamera *pThis) ;


/////////////////////////////////////////////////////////////////////////////
// External functions
/////////////////////////////////////////////////////////////////////////////
#define CCamera__StopTrackingObject(pThis) (pThis)->m_pObject = NULL
#define CCamera__SetObjectToTrack(pThis, pObject) (pThis)->m_pObject = pObject 


/////////////////////////////////////////////////////////////////////////////
// General functions
/////////////////////////////////////////////////////////////////////////////
void CCamera__Update(CCamera *pThis) ;
void CCamera__DisplayListSetup(CCamera *pThis, Gfx **ppDLP);
void CCamera__SetupMode(CCamera *pThis, INT32 NewMode) ;

void CCamera__TurokGoneBelowWater(CCamera *pThis) ;

BOOL CCamera__PosIsCollisionFree(CCamera *pThis, CVector3 *pvPos, FLOAT Radius) ;

BOOL CGameObjectInstance__CylinderCollisionFree(CGameObjectInstance *pThis, FLOAT Radius) ;

void CCamera__GetCollisionFreePos(CCamera *pThis, CVector3 *pvCurrentPos, 
											 CVector3 *pvOutPos, FLOAT EndRadius) ;

void CCamera__SetTurokPosition(CCamera *pThis, CVector3 vDesired) ;

void CCamera__KeepBelowCeiling(CCamera *pThis) ;


/////////////////////////////////////////////////////////////////////////////
// Camera view functions
/////////////////////////////////////////////////////////////////////////////
void CCameraView__CalculateXYRotation(CCameraView *pThis) ;

void CCameraView__Blend(CCameraView *pThis, FLOAT Time, CCameraView *pViewA, CCameraView *pViewB) ;
void CCameraView__TweenPosTrack(CCameraView *pThis, CCameraPosTrackInfo *pInfo, FLOAT Time) ;
void CCameraView__TweenTrackAroundPos(CCameraView *pThis, CCameraTrackInfo *pInfo, FLOAT Time, CVector3 *pvPos, FLOAT RotYOffset) ;
void CCameraView__TweenTrackAroundObject(CCameraView *pThis, CCameraTrackInfo *pInfo, FLOAT Time, CGameObjectInstance *pObject) ;
void CCameraView__TweenTrackAroundTurok(CCameraView *pThis, CCameraTrackInfo *pInfo, FLOAT Time) ;

void CCameraView__ZoomEyeToPos(CCameraView *pThis, CVector3 *pvPos, FLOAT Speed) ;
void CCameraView__ZoomFocusToPos(CCameraView *pThis, CVector3 *pvPos, FLOAT Speed) ;



/////////////////////////////////////////////////////////////////////////////
// Camera check functions
/////////////////////////////////////////////////////////////////////////////
void CCamera__FadeToCinema(CCamera *pThis, INT32 CinemaFlag) ;
BOOL CCamera__CheckForTurokDeathCinematic(CCamera *pThis) ;
BOOL CCamera__InCinemaMode(CCamera *pThis) ;
BOOL CCamera__UpdateTurokHead(CCamera *pThis) ;
BOOL CCamera__UpdateTurokPosition(CCamera *pThis) ;
BOOL CCamera__InLetterBoxMode(CCamera *pThis) ;



/////////////////////////////////////////////////////////////////////////////
// Camera shake functions
/////////////////////////////////////////////////////////////////////////////
void CShake__Set(CShake *pThis, FLOAT Shake) ;

#define CCamera__SetShakeX(pThis,Shake)	CShake__Set(&((pThis)->m_ShakeX), Shake) ;
#define CCamera__SetShakeY(pThis,Shake)	CShake__Set(&((pThis)->m_ShakeY), Shake) ;
#define CCamera__SetShakeZ(pThis,Shake)	CShake__Set(&((pThis)->m_ShakeZ), Shake) ;

#define CCamera__SetShake(pThis,x,y,z)	\
	CCamera__SetShakeX(pThis,x) ;			\
	CCamera__SetShakeY(pThis,y) ;			\
	CCamera__SetShakeZ(pThis,z) ;


/////////////////////////////////////////////////////////////////////////////
// Camera warp functions
/////////////////////////////////////////////////////////////////////////////
void CCamera__InitializeWarp(CCamera *pThis) ;
void CCamera__UpdateWarp(CCamera *pThis) ;
void CCamera__WarpIn(CCamera *pThis) ;
void CCamera__WarpOut(CCamera *pThis) ;



/////////////////////////////////////////////////////////////////////////////
// Sound handle stuff
/////////////////////////////////////////////////////////////////////////////
void CCamera__InitializeSoundHandles(CCamera *pThis) ;
void CCamera__KillSoundHandle(CCamera *pThis, INT32 Index) ;
void CCamera__SetSoundHandle(CCamera *pThis, INT32 Index, INT32 Handle) ;




/////////////////////////////////////////////////////////////////////////////
#endif // _INC_CAMERA

