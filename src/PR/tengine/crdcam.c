// credit camera file

#include "cppu64.h"

#ifndef MAKE_CART
#include <PR/ramrom.h>	// needed for argument passing into the app
#endif

#include "tengine.h"
//#include "dlists.h"
#include "scaling.h"
//#include "particle.h"
//#include "mattable.h"
//#include "tcontrol.h"
#include "tmove.h"
//#include "audio.h"
//#include "sfx.h"
//#include "textload.h"
//#include "regtype.h"


#include "crdcam.h"
#include "cammodes.h"
#include "camera.h"
//#include "mantis.h"
//#include "trex.h"
//#include "campaign.h"



// eye,rotation
CCameraPosTrackInfo CreditTrack = 
{

	{(16+20-15)*SCALING_FACTOR, (60-8)*SCALING_FACTOR, (-742+20)*SCALING_FACTOR},	// Start eye
	{112.62*SCALING_FACTOR,	6*SCALING_FACTOR,	-613.19*SCALING_FACTOR},				// Final eye

	{16*SCALING_FACTOR, (70+5-3)*SCALING_FACTOR, -742*SCALING_FACTOR},			// Start focus
	{10*SCALING_FACTOR,	160*SCALING_FACTOR,	-882*SCALING_FACTOR},				// Final focus

	{ANGLE_DTOR(0), ANGLE_DTOR(0), ANGLE_DTOR(0)},								// Start angles
	{ANGLE_DTOR(0), ANGLE_DTOR(0), ANGLE_DTOR(0)},								// Final angles
} ;


void CCamera__Setup_CINEMATIC_CREDIT(CCamera *pThis)
{
	pThis->m_Mode = CAMERA_CINEMA_CREDIT_MODE;

	// Put into cinema mode
	pThis->m_CinemaMode = TRUE;
	pThis->m_Time = 0 ;
	pThis->m_StageTime = 0 ;
}

#define CAMERA_CREDIT_TIME		SECONDS_TO_FRAMES(60)

void CCamera__Code_CINEMATIC_CREDIT(CCamera *pThis)
{
	CVector3 vPos ;
	CGameObjectInstance *pTurok = CEngineApp__GetPlayer(GetApp()) ;

	// Start smoke particle
	pThis->m_Time += frame_increment;
	if (pThis->m_Time >= SECONDS_TO_FRAMES(0.5))
	{
		pThis->m_Time -= SECONDS_TO_FRAMES(0.5) ;

		vPos.x = RandomRangeFLOAT(-40*SCALING_FACTOR, 12*SCALING_FACTOR) ;
		vPos.y = RandomRangeFLOAT(340*SCALING_FACTOR, 380*SCALING_FACTOR) ;
		vPos.z = RandomRangeFLOAT(-880*SCALING_FACTOR, -890*SCALING_FACTOR) ;
		AI_DoParticle(&pTurok->ah.ih, PARTICLE_TYPE_GENERIC238, vPos) ;
	}

	// Zoom final position
	CCameraView__TweenPosTrack(&pThis->m_View, &CreditTrack, 
										COSINE_TWEEN(pThis->m_StageTime / CAMERA_CREDIT_TIME)) ;

	// Update movement time
	pThis->m_StageTime += frame_increment ;
	if (pThis->m_StageTime > CAMERA_CREDIT_TIME)
		pThis->m_StageTime = CAMERA_CREDIT_TIME ;
}




