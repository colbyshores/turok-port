// volume.c
//

#include "stdafx.h"
#include "volume.h"
#include "tengine.h"

#include "debug.h"

// CPlane
/////////////////////////////////////////////////////////////////////////////

void CPlane__BuildFromVectors(CPlane *pThis, CVector3 *pvA, CVector3 *pvB, CVector3 *pvC)
{
	CVector3 vAB, vBC;

	ASSERT(pvA && pvB && pvC);

	pThis->m_vPoint = *pvA;

	CVector3__Subtract(&vAB, pvB, pvA);
	CVector3__Subtract(&vBC, pvC, pvB);

	CVector3__Cross(&pThis->m_vNormal, &vAB, &vBC);
}

BOOL CPlane__Inside(CPlane *pThis, CVector3 *pvPoint)
{
	CVector3		vDelta;

	CVector3__Subtract(&vDelta, pvPoint, &pThis->m_vPoint);

	return (CVector3__Dot(&pThis->m_vNormal, &vDelta) >= 0);
}

float CPlane__Intersect(CPlane *pThis, CVector3 *pvA, CVector3 *pvB)
{
	CVector3		vDelta, vPointDelta;
	float			normalDotDelta, normalDotPointDelta;

	CVector3__Subtract(&vDelta, pvB, pvA);
	
	normalDotDelta = CVector3__Dot(&pThis->m_vNormal, &vDelta);

	if (normalDotDelta == 0)
		return 1;

	CVector3__Subtract(&vPointDelta, &pThis->m_vPoint,  pvA);
	normalDotPointDelta = CVector3__Dot(&pThis->m_vNormal, &vPointDelta);

	return normalDotPointDelta/normalDotDelta;
}

// CViewVolume
/////////////////////////////////////////////////////////////////////////////

void CViewVolume__BuildFromVectors(CViewVolume *pThis,
											  CVector3 *pvEye,
											  CVector3 *pvUpLeft,
											  CVector3 *pvUpRight,
											  CVector3 *pvLowLeft,
											  CVector3 *pvLowRight)
{
	// far plane
	CPlane__BuildFromVectors(&pThis->m_Planes[0], pvUpLeft, pvUpRight, pvLowRight);

	// near plane
	pThis->m_Planes[1].m_vNormal.x = -pThis->m_Planes[0].m_vNormal.x;
	pThis->m_Planes[1].m_vNormal.y = -pThis->m_Planes[0].m_vNormal.y;
	pThis->m_Planes[1].m_vNormal.z = -pThis->m_Planes[0].m_vNormal.z;
	pThis->m_Planes[1].m_vPoint = *pvEye;

	// left plane
	CPlane__BuildFromVectors(&pThis->m_Planes[2], pvEye, pvUpLeft, pvLowLeft);

	// right plane
	CPlane__BuildFromVectors(&pThis->m_Planes[3], pvEye, pvLowRight, pvUpRight);

	// top plane
	CPlane__BuildFromVectors(&pThis->m_Planes[4], pvEye, pvUpRight, pvUpLeft);

	// bottom plane
	CPlane__BuildFromVectors(&pThis->m_Planes[5], pvEye, pvLowLeft, pvLowRight);
}

BOOL CViewVolume__Inside(CViewVolume *pThis, CVector3 *pvPoint)
{
	int			p;

	ASSERT(pvPoint);
	
	for (p=0; p<6; p++)
		if (!CPlane__Inside(&pThis->m_Planes[p], pvPoint))
			return FALSE;

	return TRUE;
}

BOOL CViewVolume__IsOverlapping(CViewVolume *pThis, int nPts, CVector3 V[])
{
	int		cPlane, cPoint;
	CPlane	*pPlane;
	BOOL		anyInside;

	for (cPlane=0; cPlane<6; cPlane++)
	{
		pPlane = &pThis->m_Planes[cPlane];

		anyInside = FALSE;
		for (cPoint=0; cPoint<nPts; cPoint++)
		{
			if (CPlane__Inside(pPlane, &V[cPoint]))
			{
				anyInside = TRUE;
				break;
			}
		}

		if (!anyInside)
			return FALSE;
	}

	return TRUE;
}

BOOL CViewVolume__IsOverlappingNoFarClip(CViewVolume *pThis, int nPts, CVector3 V[])
{
	int		cPlane, cPoint;
	CPlane	*pPlane;
	BOOL		anyInside;

	for (cPlane=1; cPlane<6; cPlane++)
	{
		pPlane = &pThis->m_Planes[cPlane];

		anyInside = FALSE;
		for (cPoint=0; cPoint<nPts; cPoint++)
		{
			if (CPlane__Inside(pPlane, &V[cPoint]))
			{
				anyInside = TRUE;
				break;
			}
		}

		if (!anyInside)
			return FALSE;
	}

	return TRUE;
}

BOOL CViewVolume__IsOverlappingBounds(CViewVolume *pThis, CROMBounds *pBounds)
{
	CVector3		vCorners[8];

   vCorners[0].x = pBounds->m_vMin.x;
	vCorners[0].y = pBounds->m_vMin.y;
	vCorners[0].z = pBounds->m_vMin.z;

   vCorners[1].x = pBounds->m_vMax.x;
	vCorners[1].y = pBounds->m_vMin.y;
	vCorners[1].z = pBounds->m_vMin.z;

	vCorners[2].x = pBounds->m_vMax.x;
	vCorners[2].y = pBounds->m_vMin.y;
	vCorners[2].z = pBounds->m_vMax.z;

   vCorners[3].x = pBounds->m_vMin.x;
	vCorners[3].y = pBounds->m_vMin.y;
	vCorners[3].z = pBounds->m_vMax.z;

   vCorners[4].x = pBounds->m_vMin.x;
	vCorners[4].y = pBounds->m_vMax.y;
	vCorners[4].z = pBounds->m_vMin.z;

	vCorners[5].x = pBounds->m_vMax.x;
	vCorners[5].y = pBounds->m_vMax.y;
	vCorners[5].z = pBounds->m_vMin.z;

   vCorners[6].x = pBounds->m_vMax.x;
	vCorners[6].y = pBounds->m_vMax.y;
	vCorners[6].z = pBounds->m_vMax.z;

	vCorners[7].x = pBounds->m_vMin.x;
	vCorners[7].y = pBounds->m_vMax.y;
	vCorners[7].z = pBounds->m_vMax.z;

	return CViewVolume__IsOverlapping(pThis, 8, vCorners);
}

