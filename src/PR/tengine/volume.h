// volume.h

#ifndef _INC_VOLUME
#define _INC_VOLUME

#include "graphu64.h"
#include "romstruc.h"

/////////////////////////////////////////////////////////////////////////////

typedef struct CPlane_t
{
	CVector3		m_vPoint,
					m_vNormal;
} CPlane;

// CPlane operations
/////////////////////////////////////////////////////////////////////////////

void		CPlane__BuildFromVectors(CPlane *pThis, CVector3 *pvA, CVector3 *pvB, CVector3 *pvC);
BOOL		CPlane__Inside(CPlane *pThis, CVector3 *pvPoint);
float		CPlane__Intersect(CPlane *pThis, CVector3 *pvA, CVector3 *pvB);

/////////////////////////////////////////////////////////////////////////////

typedef struct CViewVolume_t
{
#define VOLUME_FAR_PLANE		0
#define VOLUME_NEAR_PLANE		1
#define VOLUME_LEFT_PLANE		2
#define VOLUME_RIGHT_PLANE		3
#define VOLUME_TOP_PLANE		4
#define VOLUME_BOTTOM_PLANE	5
	CPlane	m_Planes[6];
} CViewVolume;

// CVolume operations
/////////////////////////////////////////////////////////////////////////////

void		CViewVolume__BuildFromVectors(CViewVolume *pThis,
												   CVector3 *pvEye,
												   CVector3 *pvUpLeft,
												   CVector3 *pvUpRight,
												   CVector3 *pvLowLeft,
												   CVector3 *pvLowRight);
BOOL		CViewVolume__Inside(CViewVolume *pThis, CVector3 *pvPoint);
BOOL		CViewVolume__IsOverlapping(CViewVolume *pThis, int nPts, CVector3 V[]);
BOOL		CViewVolume__IsOverlappingNoFarClip(CViewVolume *pThis, int nPts, CVector3 V[]);
BOOL		CViewVolume__IsOverlappingBounds(CViewVolume *pThis, CROMBounds *pBounds);
#define	CViewVolume__Intersect(pThis, nPlane, pvA, pvB) CPlane__Intersect(&(pThis)->m_Planes[(nPlane)], (pvA), (pvB))

/////////////////////////////////////////////////////////////////////////////
#endif // _INC_VOLUME
