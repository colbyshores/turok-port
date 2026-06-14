// Gallery camera control

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


#include "galrecam.h"
#include "cammodes.h"
#include "camera.h"


/*****************************************************************************
*
*	Function:		CCamera__ConstructGallery()
*
******************************************************************************
*
*	Description:	Perpares for gallery setup
*
*	Inputs:			*pThis	-	Ptr to camera
*
*	Outputs:			None
*
*****************************************************************************/
void CCamera__ConstructGallery(CCamera *pThis)
{
	// Setup start mode
	CCamera__SetupMode(pThis, CAMERA_GALLERY_IDLE_MODE) ;
}





/////////////////////////////////////////////////////////////////////////////
//	Modes:	  	 CAMERA_GALLERY_IDLE_MODE
//	Description: Waiting for player to move joypad perhaps?
/////////////////////////////////////////////////////////////////////////////
void CCamera__Setup_GALLERY_IDLE(CCamera *pThis)
{
	pThis->m_Mode = CAMERA_GALLERY_IDLE_MODE ;
	pThis->m_CinemaMode = TRUE ;

	// Setup start position
	pThis->m_View.m_vEye.x = -239.19*SCALING_FACTOR ;
	pThis->m_View.m_vEye.y = 0*SCALING_FACTOR ;
	pThis->m_View.m_vEye.z = 328.8*SCALING_FACTOR ;
	
	pThis->m_View.m_vFocus.x = -239.19*SCALING_FACTOR ;
	pThis->m_View.m_vFocus.y = 0*SCALING_FACTOR ;
	pThis->m_View.m_vFocus.z = 358.8*SCALING_FACTOR ;
}

void CCamera__Code_GALLERY_IDLE(CCamera *pThis)
{
	// tengine updates camera!!!!
}
