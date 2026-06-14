// sun.cpp
//

#include "stdafx.h"
#include "sun.h"
#include "tengine.h"
#include "scaling.h"
#include "geometry.h"
#include "textload.h"
#include "tmove.h"

#include "debug.h"

/////////////////////////////////////////////////////////////////////////////

// globals
CSun		sun;

// CSunFrameData
/////////////////////////////////////////////////////////////////////////////

void CSunFrameData__Construct(CSunFrameData *pThis)
{
	pThis->m_Visible = FALSE;
	pThis->m_VisibleZ = FALSE;
}

// called every frame when RDP is complete
void CSunFrameData__RDPComplete(CSunFrameData *pThis)
{
	unsigned short		*zValues;
	int					i, v;

	// depth data must be on own cache line
	//osSyncPrintf("depth addr:%x\n", pThis->m_DepthData);
	ASSERT(!(((DWORD)pThis->m_DepthData) & 0xf));
	osInvalDCache(pThis->m_DepthData, 2*sizeof(u64));

	// should only be called by scheduler when last RDP task is complete
	ASSERT(osGetThreadPri(NULL) == PRIORITY_SCHEDULER);

	if (pThis->m_Visible)
	{
		zValues = (unsigned short*) pThis->m_DepthData;
		
		v = 0;
		for (i=0; i<8; i++)
		{
			if (zValues[i] > 0xfff0)
			{
				if (i == pThis->m_DepthPos)
					v += 3;	// sun pixel is weighted 3 times surrounding pixels
				else
					v++;		// surrounding pixel
			}
		}
		sun.m_Opacity = v*(1.0/10.0);
	}

	sun.m_Visible = pThis->m_Visible;
}

// CSun
/////////////////////////////////////////////////////////////////////////////

void CSun__Construct(CSun *pThis)
{
	pThis->m_vDir.x = -0.667424;
	pThis->m_vDir.y = 2*0.476731;
	pThis->m_vDir.z = -0.572078;
	CVector3__Normalize(&pThis->m_vDir);

	pThis->m_Visible = FALSE;
}

void CSun__SetSunDir(CSun *pThis, CVector3 *pvDir)
{
	pThis->m_vDir = *pvDir;
	
	CVector3__Normalize(&pThis->m_vDir);
}

// read sun visibility and position
void CSun__GetSunData(CSun *pThis, BOOL *pVisible, float *pX, float *pY, float *pOpacity)
{
	OSPri					ospri;

	ospri = osGetThreadPri(NULL);
	osSetThreadPri(NULL, PRIORITY_SCHEDLOCK);

	// m_Visible and m_Opacity are accessed by two threads, so they need synchronization
	*pVisible = pThis->m_Visible;
	*pOpacity = pThis->m_Opacity;

	osSetThreadPri(NULL, ospri);

	*pX = pThis->m_ScreenX;
	*pY = pThis->m_ScreenY;
}

// must call every frame for CSunFrame__RDPComplete() to work properly
void CSun__ReadZDepth(CSun *pThis, Gfx **ppDLP)
{
	int					alignedX;
	unsigned short		*pSource, *pDest;
	CVector3				vOffset, vPos;
	int					x, y,
							xm3;
	CSunFrameData		*pSunFrame;

	
	pSunFrame = CEngineApp__GetSunFrameData(GetApp());

	if (		(!CTurokMovement.InSun)
			&& (!(GetApp()->m_WarpID == CREDITS_WARP_ID)) )
	{
		pSunFrame->m_Visible = FALSE;
		return;
	}
	
	// calculate visibility

#define SUN_DISTANCE		1000*SCALING_FACTOR
	CVector3__MultScaler(&vOffset, &pThis->m_vDir, SUN_DISTANCE);
	CVector3__Add(&vPos, &(CEngineApp__GetEyePos(GetApp())), &vOffset);
	
	pSunFrame->m_Visible = CViewVolume__IsOverlappingNoFarClip(&view_volume, 1, &vPos);

	if (!pSunFrame->m_Visible)
		return;

	CEngineApp__GetScreenCoordinates(GetApp(), &vPos, &pThis->m_ScreenX, &pThis->m_ScreenY);

	x = (int) pThis->m_ScreenX;
	y = (int) pThis->m_ScreenY;

	x = max(0, min(SCREEN_WD-1, x));
	y = max(0, min(SCREEN_HT-1, y));

	xm3 = max(0, x-3);

	alignedX = xm3 & 0xfffffffc;
	pSunFrame->m_DepthPos = x - alignedX;

	pSource = &zbuffer[alignedX + y*SCREEN_WD];
	pDest = (unsigned short*) pSunFrame->m_DepthData;

	// read value from Z buffer
	gDPLoadTextureBlock(((*ppDLP)++),
							  RDP_ADDRESS(pSource), G_IM_FMT_RGBA, G_IM_SIZ_16b,
							  8, 1,		// width, height
							  0,
							  G_TX_NOMIRROR, G_TX_NOMIRROR,
							  G_TX_NOMASK, G_TX_NOMASK,
							  G_TX_NOLOD, G_TX_NOLOD);
	gDPSetTextureLUT((*ppDLP)++, G_TT_NONE);

	gDPSetTexturePersp((*ppDLP)++, G_TP_NONE);
	CGeometry__SetRenderMode(ppDLP, G_RM_NOOP__G_RM_NOOP2);
	gDPSetColorImage((*ppDLP)++, G_IM_FMT_RGBA, G_IM_SIZ_16b,
						  SCREEN_WD, RDP_ADDRESS(pDest));

	gDPPipeSync((*ppDLP)++);
	gDPSetCycleType((*ppDLP)++, G_CYC_COPY);


	// write value to CSun
	gSPTextureRectangle((*ppDLP)++,
							  0, 0,					// dest left/top (10.2)
							  7<<2, 0<<2,			// dest right/bottom (10.2)
							  0, 0, 0,				// tile, src left/top (S10.5)
							  4<<10, 1<<10);		// dsdx/dsdz (S5.10)


	gDPSetColorImage((*ppDLP)++, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WD,
		   			  RDP_ADDRESS(CEngineApp__GetFrameData(GetApp())->m_pFrameBuffer));
	gDPPipeSync((*ppDLP)++);

	gDPSetTexturePersp((*ppDLP)++, G_TP_PERSP);

	CTextureLoader__InvalidateTextureCache();

	// leaves in copy mode
}
