// sun.h

#ifndef _INC_SUN
#define _INC_SUN

#include "graphu64.h"

/////////////////////////////////////////////////////////////////////////////


typedef struct CSunFrameData_t
{
	u64			m_DepthData[2];
	int			m_DepthPos;
	BOOL			m_Visible,
					m_VisibleZ,
					align;
} CSunFrameData;

// CSunFrameData operations
/////////////////////////////////////////////////////////////////////////////

void		CSunFrameData__Construct(CSunFrameData *pThis);
void		CSunFrameData__RDPComplete(CSunFrameData *pThis);

/////////////////////////////////////////////////////////////////////////////

typedef struct CSun_t
{
	int			m_nBuffer, m_nReceived;
	float			m_ScreenX, m_ScreenY,
					m_Opacity;
	CVector3		m_vDir;
	BOOL			m_Visible;
} CSun;


// CSun operations
/////////////////////////////////////////////////////////////////////////////

void		CSun__Construct(CSun *pThis);
void		CSun__SetSunDir(CSun *pThis, CVector3 *pvDir);
void		CSun__GetSunData(CSun *pThis, BOOL *pVisible, float *pX, float *pY, float *pOpacity);
void		CSun__ReadZDepth(CSun *pThis, Gfx **ppDLP);

/////////////////////////////////////////////////////////////////////////////

// globals
extern CSun sun;

/////////////////////////////////////////////////////////////////////////////
#endif // _INC_SUN
