// wallcoll.c
//

#include "stdafx.h"

#include "wallcoll.h"

/*****************************************************************************
*
*
*
*	LINE CODE
*
*
*
*****************************************************************************/

//----------------------------------------------------------------------------
// Setup line variables
//----------------------------------------------------------------------------
void CLine__Construct(CLine *pThis, CVector3 *pvPt1, CVector3 *pvPt2)
{
	// Setup end points
	pThis->m_vPt[0] = *pvPt1 ;
	pThis->m_vPt[1] = *pvPt2 ;

	// Setup deltas
	pThis->m_vDelta.x = pvPt2->x - pvPt1->x ;
	pThis->m_vDelta.y = pvPt2->y - pvPt1->y ;
	pThis->m_vDelta.z = pvPt2->z - pvPt1->z ;

	// Setup normal
	pThis->m_vNormal.x = pThis->m_vDelta.z ;
	pThis->m_vNormal.y = 0 ;
	pThis->m_vNormal.z = -pThis->m_vDelta.x ;
}


//----------------------------------------------------------------------------
// Calculates normalized direction vector for line
//----------------------------------------------------------------------------
void CLine__CalculateXZDirection(CLine *pThis)
{
	FLOAT	Mag ;

	Mag = (pThis->m_vDelta.x * pThis->m_vDelta.x) + (pThis->m_vDelta.z * pThis->m_vDelta.z) ;

	if (Mag != 0)
	{
		Mag = 1 / sqrt(Mag) ;
		pThis->m_vDir.x = pThis->m_vDelta.x * Mag ;
		pThis->m_vDir.y = pThis->m_vDelta.y * Mag ;
		pThis->m_vDir.z = pThis->m_vDelta.z * Mag ;
	}
	else
	{
		pThis->m_vDir.x = 0 ;
		pThis->m_vDir.y = 0 ;
		pThis->m_vDir.z = 0 ;
	}
}

/*
//----------------------------------------------------------------------------
// Calculates normalized direction vector for line
//----------------------------------------------------------------------------
void CLine__CalculateDirection(CLine *pThis)
{
	FLOAT	Mag ;

	Mag = (pThis->m_vDelta.x * pThis->m_vDelta.x) +
			(pThis->m_vDelta.y * pThis->m_vDelta.y) +
			(pThis->m_vDelta.z * pThis->m_vDelta.z) ;

	if (Mag != 0)
	{
		Mag = 1 / sqrt(Mag) ;
		pThis->m_vDir.x = pThis->m_vDelta.x * Mag ;
		pThis->m_vDir.y = pThis->m_vDelta.y * Mag ;
		pThis->m_vDir.z = pThis->m_vDelta.z * Mag ;
	}
	else
	{
		pThis->m_vDir.x = 0 ;
		pThis->m_vDir.y = 0 ;
		pThis->m_vDir.z = 0 ;
	}
}
*/



//----------------------------------------------------------------------------
// Determines whether two infinite lines intersect each other
//----------------------------------------------------------------------------
BOOL CLine__IntersectsLine(CLine *pThis, CLine *pLine, FLOAT *pt)
{
	CVector3	vAS ;
	FLOAT		ASDotN, MDotN ;

	// Calculate direction from pThis to pLine
	vAS.x = pLine->m_vPt[0].x - pThis->m_vPt[0].x ;
	vAS.z = pLine->m_vPt[0].z - pThis->m_vPt[0].z ;

	// Calculate MDotN
	MDotN = (pLine->m_vDelta.x * pThis->m_vNormal.x) + (pLine->m_vDelta.z * pThis->m_vNormal.z) ;
	if (MDotN == 0)
		return FALSE ;

	// Calculate ASDotN ;
	ASDotN = (vAS.x * pThis->m_vNormal.x) + (vAS.z * pThis->m_vNormal.z) ;

	// Find intersection t
	*pt = ASDotN / -MDotN ;
	return TRUE ;
}



/*
//----------------------------------------------------------------------------
// Determines which side of a line a point is on
//----------------------------------------------------------------------------
INT32 CLine__PtSide(CLine *pLine, CVector3 *pvPt)
{
	CVector3	vN, vDir ;

	// Calculate normal of line
	vN = pLine->m_vNormal ;

	// Calculate direction from point to pt0 of line
	vDir.x = pvPt->x - pLine->m_vPt[0].x ;
	vDir.z = pvPt->z - pLine->m_vPt[0].z ;

	// Calc Dot product
	if (((vN.x * vDir.x) + (vN.z * vDir.z)) <= 0)
		return 0 ;
	else
		return 1 ;
}
*/

