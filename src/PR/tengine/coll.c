// coll.c
//

#include "coll.h"
#include "scaling.h"
#include "regtype.h"

// TO DO

// Cliffs (test climbing up/down - also player being able to climb them), 
// Bridge collision (yuk), 
// Big drops(need to clear pColl->m_OnGround) - could do this in the "CheckRegion" function, 
// Water collision(should be simple like ground collision)
// include pickup collision calls
// Perform correct "after collision" logic - bounce slide etc.

// Optimize:-
// Better movement bounds check


/*****************************************************************************
*	Defines
*****************************************************************************/
#define COLLISION_SHOW_STATS		0



/*****************************************************************************
*	Variables
*****************************************************************************/
CCollision Collision ;					// Global region collision var
CCollision	*pColl = &Collision ;	// Global ptr


/*****************************************************************************
*
*
*
*	BOUNDS CODE
*
*
*
*****************************************************************************/

//----------------------------------------------------------------------------
// Determine whether a line is inside the collision bounds
//----------------------------------------------------------------------------
BOOL CBounds__OverlapsLine(CBounds *pThis, CLine *pLine)
{
	return (!(((pLine->m_vPt[0].x > pThis->m_vPt[1].x) && (pLine->m_vPt[1].x > pThis->m_vPt[1].x)) ||
			    ((pLine->m_vPt[0].x < pThis->m_vPt[0].x) && (pLine->m_vPt[1].x < pThis->m_vPt[0].x)) ||
			    ((pLine->m_vPt[0].z > pThis->m_vPt[1].z) && (pLine->m_vPt[1].z > pThis->m_vPt[1].z)) ||
			    ((pLine->m_vPt[0].z < pThis->m_vPt[0].z) && (pLine->m_vPt[1].z < pThis->m_vPt[0].z)))) ;
}
//----------------------------------------------------------------------------
// Determine whether a point is inside collision bounds
//----------------------------------------------------------------------------
BOOL CBounds__PtInside(CBounds *pThis, CVector3 *pvPt)
{
	return (pvPt->x >= pThis->m_vPt[0].x) &&
			 (pvPt->x <= pThis->m_vPt[1].x) &&
			 (pvPt->z >= pThis->m_vPt[0].z) &&
			 (pvPt->z <= pThis->m_vPt[1].z) ;
}

//----------------------------------------------------------------------------
// Determine whether a region is inside the collision bounds
//----------------------------------------------------------------------------
#define LEFT_CLIP		(1<<0)
#define RIGHT_CLIP	(1<<1)
#define TOP_CLIP		(1<<2)
#define BOTTOM_CLIP	(1<<3)
BOOL CBounds__OverlapsRegion(CBounds *pThis, CGameRegion *pRegion)
{
	INT32	Corner0Clip = 0 ;
	INT32	Corner1Clip = 0 ;
	INT32	Corner2Clip = 0 ;

	// Check corner0 x
	if (pRegion->m_pCorners[0]->m_vCorner.x < pThis->m_vPt[0].x)
		Corner0Clip |= LEFT_CLIP ;
	else
	if (pRegion->m_pCorners[0]->m_vCorner.x > pThis->m_vPt[1].x)
		Corner0Clip |= RIGHT_CLIP ;

	// Check corner0 z
	if (pRegion->m_pCorners[0]->m_vCorner.z < pThis->m_vPt[0].z)
		Corner0Clip |= TOP_CLIP ;
	else
	if (pRegion->m_pCorners[0]->m_vCorner.z > pThis->m_vPt[1].z)
		Corner0Clip |= BOTTOM_CLIP ;

	// Check corner1 x
	if (pRegion->m_pCorners[1]->m_vCorner.x < pThis->m_vPt[0].x)
		Corner1Clip |= LEFT_CLIP ;
	else
	if (pRegion->m_pCorners[1]->m_vCorner.x > pThis->m_vPt[1].x)
		Corner1Clip |= RIGHT_CLIP ;

	// Check corner1 z
	if (pRegion->m_pCorners[1]->m_vCorner.z < pThis->m_vPt[0].z)
		Corner1Clip |= TOP_CLIP ;
	else
	if (pRegion->m_pCorners[1]->m_vCorner.z > pThis->m_vPt[1].z)
		Corner1Clip |= BOTTOM_CLIP ;

	// Check corner2 x
	if (pRegion->m_pCorners[2]->m_vCorner.x < pThis->m_vPt[0].x)
		Corner2Clip |= LEFT_CLIP ;
	else
	if (pRegion->m_pCorners[2]->m_vCorner.x > pThis->m_vPt[1].x)
		Corner2Clip |= RIGHT_CLIP ;

	// Check corner2 z
	if (pRegion->m_pCorners[2]->m_vCorner.z < pThis->m_vPt[0].z)
		Corner2Clip |= TOP_CLIP ;
	else
	if (pRegion->m_pCorners[2]->m_vCorner.z > pThis->m_vPt[1].z)
		Corner2Clip |= BOTTOM_CLIP ;


	// If all of pts are clipped on the same x,z side, then it's outside
	// otherwise it could overlap with the collision bounds - hence the "not"
	return (!(Corner0Clip & Corner1Clip & Corner2Clip)) ;
}




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




//----------------------------------------------------------------------------
// Determines whether two infinite lines intersect each other
//----------------------------------------------------------------------------
BOOL CLine__IntersectsLine(CLine *pThis, CLine *pLine, FLOAT *pt)
{
	CVector3	vAS ;
	FLOAT		ASDotN, MDotN ;

	// Update totals
	pColl->m_LineIntersections++ ;

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



/*****************************************************************************
*
*
*
*	CIRCLE CODE
*
*
*
*****************************************************************************/

//----------------------------------------------------------------------------
// Setup all of circle variables
//----------------------------------------------------------------------------
void CCircle__Construct(CCircle *pThis, CVector3 *pvPos, FLOAT Radius)
{
	pThis->m_vPos = *pvPos ;
	pThis->m_Radius = Radius ;
	pThis->m_RadiusSqr = Radius * Radius ;
	pThis->m_Bounds.m_vPt[0].x = pvPos->x - Radius ;
	pThis->m_Bounds.m_vPt[0].z = pvPos->z - Radius ;
	pThis->m_Bounds.m_vPt[1].x = pvPos->x + Radius ;
	pThis->m_Bounds.m_vPt[1].z = pvPos->z + Radius ;
}


//----------------------------------------------------------------------------
// Determines if an infinite line intersects a circle
//----------------------------------------------------------------------------
BOOL CCircle__IntersectsLine(CCircle *pThis, CLine *pLine, FLOAT *t)
{
	CVector3	h, m,M ;
	FLOAT		hDotM, Mag, InvMagm ;

	// OPTIMIZATION - DO QUICK BOUNDS TEST WITH LINE
	if (!CBounds__OverlapsLine(&pThis->m_Bounds, pLine))
		return FALSE ;

	// Update totals
	pColl->m_CircleIntersections++ ;

	// Calc h
	h.x = pThis->m_vPos.x - pLine->m_vPt[0].x ; 	
	h.z = pThis->m_vPos.z - pLine->m_vPt[0].z ; 	

	// Calc m
	m.x = pLine->m_vDelta.x ;
	m.z = pLine->m_vDelta.z ;
	InvMagm = (m.x * m.x) + (m.z * m.z) ;
	if (InvMagm == 0)
		return FALSE ;
	InvMagm = 1.0 / sqrt(InvMagm) ;

	// Calc M
	M.x = m.x * InvMagm ;
	M.z = m.z * InvMagm ;
	
	// Lets go for it
	hDotM = (h.x * M.x) + (h.z * M.z) ;

	h.x -= M.x * hDotM ;	
	h.z -= M.z * hDotM ;	

	Mag = (h.x * h.x) + (h.z * h.z) ;
	if (Mag != 0)
		Mag = sqrt(Mag) ;
	Mag *= Mag ;

	if (Mag > pThis->m_RadiusSqr)
		return FALSE ;
	
	*t = (hDotM - sqrt(pThis->m_RadiusSqr - Mag)) * InvMagm ;
	return TRUE ;
}



/*****************************************************************************
*
*
*
*	COLLISION INTERSECTION CODE
*
*
*
*****************************************************************************/


//----------------------------------------------------------------------------
// Get intersection between movement line and a line (1 == no intersect)
//----------------------------------------------------------------------------
FLOAT CCollision__GetLineIntersection(CLine *pLine)
{
	FLOAT					t ;

	// Get t for edge line
	if ((!CLine__IntersectsLine(&pColl->m_Line, pLine, &t)) || (t <= 0) || (t >= 1))
		return 1 ;

	// Get t for movement line
	if ((!CLine__IntersectsLine(pLine, &pColl->m_Line, &t)) || (t <= 0) || (t >= 1))
		return 1 ;

	return t ;
}


//----------------------------------------------------------------------------
// Get intersection between movement line and a circle (1 == no intersect)
//----------------------------------------------------------------------------
FLOAT CCollision__GetCircleIntersection(CCircle *pCircle)
{
	FLOAT		t ;

	// Get intersectiont for movement line
	if ((!CCircle__IntersectsLine(pCircle, &pColl->m_Line, &t)) || (t <= 0) || (t >= 1))
		return 1 ;

	return t ;
}


//----------------------------------------------------------------------------
// Given intersection t, tangent on circle is calculated
//----------------------------------------------------------------------------
void CCollision__GetCircleIntersectionTangent(CCircle *pCircle, FLOAT t, CLine *pTangentLine)
{
	CVector3	vDir, vN, vI ;

	// Calculate intersection pt
	vI.x = pColl->m_Line.m_vPt[0].x + (t * pColl->m_Line.m_vDelta.x) ;
	vI.y = 0 ;
	vI.z = pColl->m_Line.m_vPt[0].z + (t * pColl->m_Line.m_vDelta.z) ;

	// Calculate normal of line from vI to center of circle
	vN.x = pCircle->m_vPos.z - vI.z ;
	vN.y = 0 ;
	vN.z = vI.x - pCircle->m_vPos.x ;

	// Create tangent vector and construct line from it
	CVector3__Add(&vDir, &vN, &vI) ;
	CLine__Construct(pTangentLine, &vI, &vDir) ;
}




/*****************************************************************************
*
*
*
*	GROUND/CEILING COLLISION REGION CODE
*
*
*
*****************************************************************************/

//----------------------------------------------------------------------------
// Checks for ground intersection and sees if it is the closest so far
//----------------------------------------------------------------------------
BOOL CCollision__CheckRegionGroundIntersection(CGameRegion *pRegion)
{
	CCollisionInfo	*pCI = pColl->m_pCI ;
	FLOAT				t ;
	CVector3			vI, vN ;

	// Perform ground collision?
	if ((pCI->m_dwFlags & COLLISIONFLAG_NOFLOORCOLLISION) || (pRegion->m_wFlags & REGFLAG_FALLDEATH))
		return FALSE ;

	// Should we check this region?
	vN = CGameRegion__GetGroundNormal(pRegion) ;
	if (CVector3__Dot(&vN, &pColl->m_Line.m_vDelta) <= 0)
		return FALSE ;

	// Get ground intersection and make sure it's on the movement line
	if ((CGameRegion__GetGroundIntersection(pRegion, 
														 &pColl->m_Line.m_vPt[0], &pColl->m_Line.m_vPt[1], &t)) &&
		 (t >= 0) && (t <= 1))
	{
		// Calculate intersection pt (x,z only)
		vI.x = pColl->m_Line.m_vPt[0].x + (t * pColl->m_Line.m_vDelta.x) ;
		vI.z = pColl->m_Line.m_vPt[0].z + (t * pColl->m_Line.m_vDelta.z) ;
		if (CGameRegion__InRegion(pRegion, vI.x, vI.z))
		{
			// Signal a collision occured
			pColl->m_Collision = TRUE ;

			// Closest collision to start position so far?
			if (t < pColl->m_t)
			{
				// Keep copy of this collision
				pColl->m_t = t ;
				if (pColl->m_FirstCall)
				{
					CCollisionInfo__ClearCollision(pColl->m_pCI) ;

					// Setup collision info
					pCI->m_GroundCollision = GROUNDCOLLISION_GROUND ;
					pCI->m_pGroundCollisionRegion = pRegion ;
				}

				return TRUE ;
			}
		}
	}	

	return FALSE ;
}



//----------------------------------------------------------------------------
// Checks for ceiling intersection and sees if it is the closest so far
//----------------------------------------------------------------------------
BOOL CCollision__CheckRegionCeilingIntersection(CGameRegion *pRegion)
{
	CCollisionInfo	*pCI = pColl->m_pCI ;
	FLOAT				t ;
	CVector3			vI, vN ;

	// Check for hitting head
 	if (!(pRegion->m_wFlags & REGFLAG_CEILING))
		return FALSE ;

	// Should we check this region?
	vN = CGameRegion__GetCeilingNormal(pRegion) ;
	if (CVector3__Dot(&vN, &pColl->m_Line.m_vDelta) <= 0)
		return FALSE ;

	// Get ground intersection and make sure it's on the movement line
	if ((CGameRegion__GetCeilingIntersection(pRegion, 
														  &pColl->m_HeadLine.m_vPt[0], &pColl->m_HeadLine.m_vPt[1], &t)) &&
		 (t >= 0) && (t <= 1))
	{
		// Calculate intersection pt (x,z only)
		vI.x = pColl->m_Line.m_vPt[0].x + (t * pColl->m_Line.m_vDelta.x) ;
		vI.z = pColl->m_Line.m_vPt[0].z + (t * pColl->m_Line.m_vDelta.z) ;
		if (CGameRegion__InRegion(pRegion, vI.x, vI.z))
		{
			// Closest collision to start position so far?
			if (t < pColl->m_t)
			{
				// Keep copy of this collision
				pColl->m_Collision = TRUE ;
				pColl->m_t = t ;
  
				if (pColl->m_FirstCall)
				{
					CCollisionInfo__ClearCollision(pColl->m_pCI) ;

					// Setup collision info
					pCI->m_GroundCollision = GROUNDCOLLISION_CEILING ;
					pCI->m_pGroundCollisionRegion = pRegion ;
				}

				return TRUE ;
			}
		}
	}	

	return FALSE ;
}









/*****************************************************************************
*
*
*
*	WALL COLLISION REGION CODE
*
*
*
*****************************************************************************/

//----------------------------------------------------------------------------
// Can this region be entered?
//----------------------------------------------------------------------------
BOOL CCollision__CanEnterRegion(CGameRegion *pCurrentRegion, CGameRegion *pDestRegion, 
										  CLine *pLine, FLOAT t, INT32 EdgeNumber)
{
	CCollisionInfo	*pCI = pColl->m_pCI ;
	CVector3			vI, vN ;

	if ((!((pColl->m_ClimbUp) && (pColl->m_ClimbDown))) && (CGameRegion__IsCliff(pDestRegion)))
	{
		vI.x = pColl->m_Line.m_vPt[0].x - pDestRegion->m_pCorners[0]->m_vCorner.x ;
		vI.y = pColl->m_Line.m_vPt[0].y - pDestRegion->m_pCorners[0]->m_vCorner.y ;
		vI.z = pColl->m_Line.m_vPt[0].z - pDestRegion->m_pCorners[0]->m_vCorner.z ;
		vN = CGameRegion__GetGroundNormal(pDestRegion) ;
		if (CVector3__Dot(&vN, &vI) >= 0)
		{
//			rmonPrintf("Trying to climb up\r\n") ;
//			if (!pColl->m_ClimbUp)
//				return FALSE ;
		}
		else
		{
//			rmonPrintf("Trying to climb down\r\n") ;
//			if (!pColl->m_ClimbDown)
//				return FALSE ;
		}
	}

	// Keep bad guys out of trouble
	if ((pDestRegion->m_wFlags & REGFLAG_RESTRICTEDAREA) && 
		 (!(pCI->m_dwFlags & COLLISIONFLAG_CANENTERRESTRICTEDAREA)))
		return FALSE ;

	// Allowed to exit water ?
	if ((!pColl->m_ExitWater) && 
		 (!(pDestRegion->m_wFlags & REGFLAG_HASWATER)) &&
		 (pCurrentRegion->m_wFlags & REGFLAG_HASWATER))
		return FALSE ;

	// Avoid water
	if ((!pColl->m_EnterWater) && (pDestRegion->m_wFlags & REGFLAG_HASWATER) &&
		 (!(pCurrentRegion->m_wFlags & REGFLAG_HASWATER)))
		return FALSE ;

	// Need to be ducking to enter ducking area
	if ((pDestRegion->m_wFlags & REGFLAG_DUCK) && (!(pCI->m_dwFlags & COLLISIONFLAG_DUCKING)))
		return FALSE ;

	// Can't enter closed door
	if ((!(pCI->m_dwFlags & COLLISIONFLAG_MOVETHROUGHDOORS)) &&
		 (pDestRegion->m_wFlags & REGFLAG_DOOR) && (!(pDestRegion->m_wFlags & REGFLAG_OPENDOOR)))
		return FALSE ;

	return TRUE ;
}



//----------------------------------------------------------------------------
// Recursively clears the recurse flag
//----------------------------------------------------------------------------
void CCollision__ClearRegionRecurseFlag(CGameRegion *pRegion)
{
	// Clear flag
	pRegion->m_wFlags &= ~REGFLAG_RECURSE ;
	pColl->m_RegionsRecursed++ ;

	// Is final position in this region?
	if (CGameRegion__InRegion(pRegion, pColl->m_vFinalPos.x, pColl->m_vFinalPos.z))
		pColl->m_pFinalRegion = pRegion ;

	// Check neighbor0
	if ((pRegion->m_pNeighbors[0]) && (pRegion->m_pNeighbors[0]->m_wFlags & REGFLAG_RECURSE))
		CCollision__ClearRegionRecurseFlag(pRegion->m_pNeighbors[0]) ;

	// Check neighbor1
	if ((pRegion->m_pNeighbors[1]) && (pRegion->m_pNeighbors[1]->m_wFlags & REGFLAG_RECURSE))
		CCollision__ClearRegionRecurseFlag(pRegion->m_pNeighbors[1]) ;

	// Check neighbor2
	if ((pRegion->m_pNeighbors[2]) && (pRegion->m_pNeighbors[2]->m_wFlags & REGFLAG_RECURSE))
		CCollision__ClearRegionRecurseFlag(pRegion->m_pNeighbors[2]) ;
}


//----------------------------------------------------------------------------
// Checks collision with region edge line
//----------------------------------------------------------------------------
void CEdge__Construct(CEdge *pThis, CLine *pLine)
{
	CVector3	vN, vPt1, vPt2 ;
	
	// Create circles around end pts
	CCircle__Construct(&pThis->m_Circle[0], &pLine->m_vPt[0], pColl->m_Radius) ;
	CCircle__Construct(&pThis->m_Circle[1], &pLine->m_vPt[1], pColl->m_Radius) ;

	// Calculate unit normal of line
	vN = pLine->m_vNormal ;
	CVector3__Normalize(&vN) ;	

	// Side lines will be expanded from region line by instance radius
	vN.x *= pColl->m_Radius ;
	vN.z *= pColl->m_Radius ;

	// Construct side line0
	CVector3__Subtract(&vPt1, &pLine->m_vPt[0], &vN) ;
	CVector3__Subtract(&vPt2, &pLine->m_vPt[1], &vN) ;
	CLine__Construct(&pThis->m_Line[0], &vPt1, &vPt2) ;

	// Construct side line1
	CVector3__Add(&vPt1, &pLine->m_vPt[0], &vN) ;
	CVector3__Add(&vPt2, &pLine->m_vPt[1], &vN) ;
	CLine__Construct(&pThis->m_Line[1], &vPt1, &vPt2) ;
}


//----------------------------------------------------------------------------
// Checks region edge for intersection
//----------------------------------------------------------------------------
void CCollision__CheckRegionEdgeIntersection(CGameRegion *pRegion, INT32 EdgeNumber)
{
	FLOAT		t, MinT ;
	CLine		Line, Tangent ;
	CEdge		Edge ;

	// Construct a line from region edge
	CLine__Construct(&Line, &pRegion->m_pCorners[EdgeNumber]->m_vCorner, 
									&pRegion->m_pCorners[(EdgeNumber+1)%3]->m_vCorner) ;
	
	// Check this edge?
	if (((Line.m_vNormal.x * pColl->m_Line.m_vDir.x) + (Line.m_vNormal.z * pColl->m_Line.m_vDir.z)) >= 0)
		return ; 



// If CCollision__CanEnterRegion doesn't require t(cliff collision may need it),
// then the intersection test could be done only if this edge has no neighbor or region can't be entered.
// It will perform less intersection tests

	// Find smallest intersection
	MinT = pColl->m_t ;
	if (pColl->m_Radius == 0)
	{
		MinT = min(MinT, CCollision__GetLineIntersection(&Line)) ;
		Tangent = Line ;
	}
	else
	{
		// Create edge boundary (expands the edge into 2 circles, and 2 lines
		CEdge__Construct(&Edge, &Line) ;

		// Check line1
		t = CCollision__GetLineIntersection(&Edge.m_Line[0]) ;
		if (t < MinT)
		{
			Tangent = Edge.m_Line[0] ;
			MinT = t ;
		}

		// Check line2
		t = CCollision__GetLineIntersection(&Edge.m_Line[1]) ;
		if (t < MinT)
		{
			Tangent = Edge.m_Line[1] ;
			MinT = t ;
		}

		// Check circle1
		t = CCollision__GetCircleIntersection(&Edge.m_Circle[0]) ;
		if (t < MinT)
		{
			CCollision__GetCircleIntersectionTangent(&Edge.m_Circle[0], t, &Tangent) ;
			MinT = t ;
		}

		// Check circle2
		t = CCollision__GetCircleIntersection(&Edge.m_Circle[1]) ;
		if (t < MinT)
		{
			CCollision__GetCircleIntersectionTangent(&Edge.m_Circle[1], t, &Tangent) ;
			MinT = t ;
		}
	}

	// Only perform region checks if this is the closest collision so far!
	if ((MinT < pColl->m_t) &&
		 ((!pRegion->m_pNeighbors[EdgeNumber]) || 
		  (!CCollision__CanEnterRegion(pRegion, pRegion->m_pNeighbors[EdgeNumber], &Line, MinT, EdgeNumber))))
	{
		// Update collision position
		pColl->m_Collision = TRUE ;
		pColl->m_t = MinT ;
		pColl->m_CollisionLine = Tangent ;

		// Keep record of collision
		if (pColl->m_FirstCall)
		{
			CCollisionInfo__ClearCollision(pColl->m_pCI) ;

			// Setup collision info
			pColl->m_pCI->m_WallCollision = TRUE ;
			pColl->m_pCI->m_pWallCollisionRegion = pRegion ;
			pColl->m_pCI->m_nWallCollisionEdge = EdgeNumber ;
		}
	}
}


//----------------------------------------------------------------------------
// Checks collision with region
//----------------------------------------------------------------------------
void CCollision__CheckRegion(CGameRegion *pRegion)
{
	// Signal region has been recursed on
	pRegion->m_wFlags |= REGFLAG_RECURSE ;

	// Check ground collision
	if ((!pColl->m_OnGround) && (pColl->m_vVelocity.y < 0))
		CCollision__CheckRegionGroundIntersection(pRegion) ;

	// Check ceiling collision
	if (pColl->m_vVelocity.y > 0)
		CCollision__CheckRegionCeilingIntersection(pRegion) ;

	// Check wall collision
	if (pColl->m_pCI->m_WallBehavior != WALL_NOCOLLISION)
	{
		CCollision__CheckRegionEdgeIntersection(pRegion,0) ;
		CCollision__CheckRegionEdgeIntersection(pRegion,1) ;
		CCollision__CheckRegionEdgeIntersection(pRegion,2) ;
	}

	// Recurse on edge0
	if ((pRegion->m_pNeighbors[0]) && 
		 (!(pRegion->m_pNeighbors[0]->m_wFlags & REGFLAG_RECURSE)) &&
		 (CBounds__OverlapsRegion(&pColl->m_Bounds, pRegion->m_pNeighbors[0])))
		CCollision__CheckRegion(pRegion->m_pNeighbors[0]) ;

	// Recurse on edge1
	if ((pRegion->m_pNeighbors[1]) && 
		 (!(pRegion->m_pNeighbors[1]->m_wFlags & REGFLAG_RECURSE)) &&
		 (CBounds__OverlapsRegion(&pColl->m_Bounds, pRegion->m_pNeighbors[1])))
		CCollision__CheckRegion(pRegion->m_pNeighbors[1]) ;

	// Recurse on edge2
	if ((pRegion->m_pNeighbors[2]) && 
		 (!(pRegion->m_pNeighbors[2]->m_wFlags & REGFLAG_RECURSE)) &&
		 (CBounds__OverlapsRegion(&pColl->m_Bounds, pRegion->m_pNeighbors[2])))
		CCollision__CheckRegion(pRegion->m_pNeighbors[2]) ;
}



/*****************************************************************************
*
*
*
*	COLLISION INSTANCE CODE
*
*
*
*****************************************************************************/

//----------------------------------------------------------------------------
// Checks collision with a single instance - all pickup logic is not here yet
//----------------------------------------------------------------------------
BOOL CCollision__CheckInstanceIntersection(CInstanceHdr *pInst)
{
	CAnimInstanceHdr		*pThis = pColl->m_pThis ;
	CCollisionInfo			*pCI = pColl->m_pCI ;
	CCircle					Circle ;
	CGameObjectInstance	*pAlive ;
	FLOAT						InstHeight, InstFeet ;
	FLOAT						ThisHeight, ThisFeet ;
	FLOAT						t ;
	CLine						Tangent ;

	// Don't collide with self
	if (pInst == &pThis->ih)
		return FALSE ;

	// Keep certain particles from colliding with their source
	if (pThis->ih.m_Type == I_PARTICLE)
	{
		if (		(pInst == ((CParticle*) pThis)->m_pSource)
				&& (((CParticle*) pThis)->m_pEffect->m_dwFlags & PARTICLE_BEHAVIOR_CANNOTHITSOURCE) )
			return FALSE ;
	}

	// Special cases
	switch (pInst->m_Type)
	{
		case I_ANIMATED:
			pAlive = (CGameObjectInstance*) pInst;
			if (pAlive->m_AI.m_dwStatusFlags & AI_EXPTARGETEXPLODED)
				return FALSE ;
			else if (pCI->m_dwFlags & COLLISIONFLAG_IGNOREDEAD)
			{
				// some things don't collide with dead stuff
				// (mainly the player and particles)
				if (pAlive->m_AI.m_dwStatusFlags & AI_ALREADY_DEAD)
					return FALSE ;
			}
			else if (pAlive->m_cMelt != 0.0)
			{
				// don't collide with stuff that's melting
				return FALSE ;
			}			  
			break;

		case I_SIMPLE:
			return FALSE ;
//			if (!(pCI->m_dwFlags & COLLISIONFLAG_PICKUPCOLLISION))
//			{
//				// Only the player should be affected by pickups
//				return FALSE ;
//			}
//			break;

		case I_PARTICLE:
			// this shouldn't happen because particles are never added
			// to the collision list
			ASSERT(FALSE);
	}

	// Perform y check
	InstHeight = pInst->m_pEA->m_CollisionHeight ;
	if ((pAlive) && (pAlive->m_AI.m_dwStatusFlags & AI_ALREADY_DEAD))
			InstHeight = pInst->m_pEA->m_DeadHeight ;

	ThisHeight = pThis->ih.m_pEA->m_CollisionHeight ;
	if ((pAlive) && (pAlive->m_AI.m_dwStatusFlags & AI_ALREADY_DEAD))
			ThisHeight = pThis->ih.m_pEA->m_DeadHeight ;

	// height offset comes from from animation
	// animation translations are absolute, so the root can move up and down
	// from the instance height
	ThisFeet = pThis->ih.m_vPos.y + pThis->ih.m_CollisionHeightOffset ;
	InstFeet = pInst->m_vPos.y + pInst->m_CollisionHeightOffset ;
	if (!((InstFeet <= (ThisFeet + ThisHeight)) && (ThisFeet <= (InstFeet + InstHeight))))
		return FALSE ;

	// Perform collision
	CCircle__Construct(&Circle, &pInst->m_vPos, 
							 pColl->m_pThis->ih.m_pEA->m_CollisionRadius + pInst->m_pEA->m_CollisionRadius) ;

	t = CCollision__GetCircleIntersection(&Circle) ;
	if (t < pColl->m_t)
	{
		// Calc circle intersection tangent
		CCollision__GetCircleIntersectionTangent(&Circle, t, &Tangent) ;

		// Update collision position
		pColl->m_Collision = TRUE ;
		pColl->m_t = t ;
		pColl->m_CollisionLine = Tangent ;

		// Keep record of collision
		if (pColl->m_FirstCall)
		{
			CCollisionInfo__ClearCollision(pColl->m_pCI) ;

			// Setup collision info
			pColl->m_pCI->m_pInstanceCollision = pInst ;
		}
		return TRUE ;
	}

	return FALSE ;
}


//----------------------------------------------------------------------------
// Checks collision with all instances
//----------------------------------------------------------------------------
void CCollision__CheckInstanceIntersections(void)
{
	INT32				cInst, nInsts ;
	CEngineApp		*pEngine = GetApp() ;

	// Perform collision?
	nInsts = CEngineApp__GetAllInstanceCount(pEngine) ;
	if ((pColl->m_pCI->m_InstanceBehavior == INSTANCE_NOCOLLISION) || (!nInsts))
		return ;

	// Search through all instances
	for (cInst=0; cInst<nInsts; cInst++)
		CCollision__CheckInstanceIntersection(CEngineApp__GetInstance(pEngine, cInst)) ;
}






/*****************************************************************************
*
*
*
*	THE BIG OLD TOP LEVEL INSTANCE COLLISION CODE
*
*
*
*****************************************************************************/

BOOL CAnimInstanceHdr__Collision2(CAnimInstanceHdr *pThis, CGameRegion **ppCurrentRegion,
											 CVector3 *pvCurrentPos, CVector3 vVelocity,	CCollisionInfo *pCI,
											BOOL ClimbUp, BOOL ClimbDown, BOOL EnterWater, BOOL ExitWater, BOOL FirstCall)
{
	static CVector3		vSlide, vStartPos, vNormal, vDestPos ;
	static FLOAT			Dot, Speed, Radius, Gravity, GroundHeight, CeilingHeight, Feet, Head, Friction ;
	static FLOAT			MagVelocity, MagVelocitySquared, NewMagVelocity, UphillCosine ;
	static FLOAT			t, Height ;
	static BOOL				UnderWater, Slide, Bounce, Stick, Grease ;
	static CROMRegionSet	*pRegionSet ;
	static CGameObjectInstance	*pAnim ;


//----------------------------------------------------------------------------
//	Copy vars into global collision structure to avoid passing during recursion
//----------------------------------------------------------------------------
	pColl->m_FirstCall = FirstCall ;

	if (FirstCall)
		CCollisionInfo__ClearCollision(pCI) ;

	pColl->m_pThis = pThis ;
	pColl->m_ppCurrentRegion = ppCurrentRegion ;
	pColl->m_pvCurrentPos = pvCurrentPos ;
	pColl->m_pCI = pCI ;
	pColl->m_ClimbUp = ClimbUp ; 
	pColl->m_ClimbDown = ClimbDown ; 
	pColl->m_EnterWater = EnterWater ; 
	pColl->m_ExitWater = ExitWater ; 

	if (*ppCurrentRegion)
	{
		GroundHeight = CGameRegion__GetGroundHeight(*ppCurrentRegion, pvCurrentPos->x, pvCurrentPos->z) ;
		pColl->m_OnGround = ((pvCurrentPos->y - GroundHeight) <= 0.01) && (vVelocity.y <= 0) ;
	}
	else
		pColl->m_OnGround = FALSE ;



//----------------------------------------------------------------------------
//	Calculate movement velocity
//----------------------------------------------------------------------------
	if (FirstCall)
	{
		//----------------------------------------------------------------------------
		//	Ground Friction
		//----------------------------------------------------------------------------
		if (pColl->m_OnGround)
		{
			Friction = max(0, min(1, 1 - pCI->m_GroundFriction*frame_increment)) ;

			if (pCI->m_dwFlags & COLLISIONFLAG_UPDATEVELOCITY)
			{
				pThis->m_vVelocity.x *= Friction ;
				pThis->m_vVelocity.z *= Friction ;

				// Slow down velocity if it's downhill
				if (pThis->m_vVelocity.y < 0)
			 		pThis->m_vVelocity.y *= Friction ;
			}
			else
			{
				vVelocity.x *= Friction ;
				vVelocity.z *= Friction ;

				// Slow down velocity if it's downhill
				if (vVelocity.y < 0)
					vVelocity.y *= Friction ;
			}
		}

		//----------------------------------------------------------------------------
		// Air/Water Friction
		//----------------------------------------------------------------------------

		// Get friction value
		if (pCI->m_WaterFriction == pCI->m_AirFriction)
			Friction = pCI->m_AirFriction ;
		else
		{
			UnderWater = FALSE ;
			if ((*ppCurrentRegion) && ((*ppCurrentRegion)->m_wFlags & REGFLAG_HASWATER))
			{
				pRegionSet = CScene__GetRegionAttributes(&GetApp()->m_Scene, *ppCurrentRegion) ;
				if (pRegionSet)
					UnderWater = (pvCurrentPos->y < pRegionSet->m_WaterElevation) ;
			}

			if (UnderWater)
				Friction = pCI->m_WaterFriction ;
			else
				Friction = pCI->m_AirFriction ;
		}

		// Apply the friction
		if (Friction != 0)
		{
			if (pCI->m_dwFlags & COLLISIONFLAG_UPDATEVELOCITY)
				MagVelocitySquared = CVector3__MagnitudeSquared(&pThis->m_vVelocity) ;
			else
				MagVelocitySquared = CVector3__MagnitudeSquared(&vVelocity) ;

			MagVelocity = sqrt(MagVelocitySquared) ;
			if (MagVelocity != 0)
			{
				NewMagVelocity = MagVelocity - min(MagVelocity, Friction*MagVelocitySquared*frame_increment*(1/SCALING_FACTOR)) ;
				if (pCI->m_dwFlags & COLLISIONFLAG_UPDATEVELOCITY)
					CVector3__MultScaler(&pThis->m_vVelocity, &pThis->m_vVelocity, NewMagVelocity/MagVelocity) ;
				else
					CVector3__MultScaler(&vVelocity, &vVelocity, NewMagVelocity/MagVelocity) ;

			}
		}

		//----------------------------------------------------------------------------
		// Add gravity
		//----------------------------------------------------------------------------
		if ((pCI->m_dwFlags & COLLISIONFLAG_UPDATEVELOCITY) && (frame_increment != 0.0))
		{
			if (!pColl->m_OnGround)
			{
//				Gravity = pCI->m_GravityAcceleration*(SCALING_FACTOR/FRAME_FPS) * frame_increment * frame_increment ;
				Gravity = pCI->m_GravityAcceleration*(SCALING_FACTOR/FRAME_FPS) * frame_increment ;
				pThis->m_vVelocity.y += Gravity ;
			}
		}

		//----------------------------------------------------------------------------
		//	Add ground Slide
		//----------------------------------------------------------------------------
		if (pThis->ih.m_Type == I_ANIMATED)
			pAnim = (CGameObjectInstance*) pThis ;
		else
			pAnim = NULL ;

		// add Slide to vDesiredPos if on ground
		if ((*ppCurrentRegion) && (pColl->m_OnGround))
		{
			Slide =		CGameRegion__IsCliff(*ppCurrentRegion)
						&& !((*ppCurrentRegion)->m_wFlags & REGFLAG_CLIMBABLE)
						&& (vVelocity.y <= 0)
						&& !(pThis->ih.m_pEA->m_dwTypeFlags & AI_TYPE_CLIMBER)
						&& !(pCI->m_dwFlags & COLLISIONFLAG_NOHILLSLOWDOWN);

			if (pAnim)
				if (pAnim->m_dwFlags & ANIM_INSTANCE_HANGINGON)
					Slide = FALSE ;

			if (pAnim)
				if ( (pAnim->ah.ih.m_pEA->m_dwTypeFlags & AI_ALREADY_DEAD) && !(pCI->m_dwFlags & COLLISIONFLAG_NOHILLSLOWDOWN) )
					Slide = TRUE ;

			if (Slide)
			{
				vSlide = CGameRegion__GetSlideVector(*ppCurrentRegion, Gravity) ;

				UphillCosine = CGameRegion__GetUphillCosine(*ppCurrentRegion,
																		  pvCurrentPos->x, pvCurrentPos->z,
																		  pvCurrentPos->x + vSlide.x, pvCurrentPos->z + vSlide.z) ;
				ASSERT(UphillCosine != 0) ;
				if (pCI->m_dwFlags & COLLISIONFLAG_UPDATEVELOCITY)
				{
					pThis->m_vVelocity.x += vSlide.x/UphillCosine ;
					pThis->m_vVelocity.y += vSlide.y ;
					pThis->m_vVelocity.z += vSlide.z/UphillCosine ;
				}
				else
				{
					vVelocity.x += vSlide.x/UphillCosine ;
					vVelocity.y += vSlide.y ;
					vVelocity.z += vSlide.z/UphillCosine ;
				}
			}
		}
	}

//----------------------------------------------------------------------------
// Store final movement velocity
//----------------------------------------------------------------------------
	pCI->m_vCollisionVelocity = pThis->m_vVelocity ;
	pColl->m_vVelocity = vVelocity ;

//----------------------------------------------------------------------------
//	Prepare global collision structure
//----------------------------------------------------------------------------

	// Get wall radius/height
	switch (pThis->ih.m_Type)
	{
		case I_ANIMATED:
		case I_SIMPLE:
			Radius = pThis->ih.m_pEA->m_CollisionWallRadius ;

			// Dead height should also be test for i guess
			Height = pThis->ih.m_pEA->m_CollisionHeight ;
			break;
		default:
			Radius = 0 ;
			Height = 0 ;
	}

	// Setup collision vars
	pColl->m_Collision = FALSE ;
	pColl->m_t = 1 ;

	// Setup movement line
	CVector3__Add(&vDestPos, pvCurrentPos, &vVelocity) ;
	CLine__Construct(&pColl->m_Line, pvCurrentPos, &vDestPos) ;
	CLine__CalculateXZDirection(&pColl->m_Line) ;

	// Setup head movement line
	pColl->m_Line.m_vPt[0].y += Height ;
	pColl->m_Line.m_vPt[1].y += Height ;
	CLine__Construct(&pColl->m_HeadLine, &pColl->m_Line.m_vPt[0], &pColl->m_Line.m_vPt[1]) ;
	CLine__CalculateXZDirection(&pColl->m_HeadLine) ;
	pColl->m_Line.m_vPt[0].y -= Height ;
	pColl->m_Line.m_vPt[1].y -= Height ;

	// Setup bounds
	pColl->m_Radius = Radius ;
	Radius += 1 ;
	pColl->m_Bounds.m_vPt[0].x = min(pColl->m_Line.m_vPt[0].x, pColl->m_Line.m_vPt[1].x) - Radius ;
	pColl->m_Bounds.m_vPt[1].x = max(pColl->m_Line.m_vPt[0].x, pColl->m_Line.m_vPt[1].x) + Radius ;
	pColl->m_Bounds.m_vPt[0].z = min(pColl->m_Line.m_vPt[0].z, pColl->m_Line.m_vPt[1].z) - Radius ;
	pColl->m_Bounds.m_vPt[1].z = max(pColl->m_Line.m_vPt[0].z, pColl->m_Line.m_vPt[1].z) + Radius ;

	// Setup final position vars
	pColl->m_pFinalRegion = NULL ;
	pColl->m_vFinalPos = pColl->m_Line.m_vPt[1] ;

	// Reset stats
	if (FirstCall)
	{
		pColl->m_RegionsRecursed = 0 ;
		pColl->m_CircleIntersections = 0 ;
		pColl->m_LineIntersections = 0 ;
	}


	// Setup global feet,head position
	pColl->m_Feet = pvCurrentPos->y ;
	pColl->m_Head = pColl->m_Feet + Height ;
	vStartPos = *pvCurrentPos ;

//----------------------------------------------------------------------------
//	Perform collision tests
//----------------------------------------------------------------------------

	// Check instance collision
	CCollision__CheckInstanceIntersections() ;

	// Perform collision test with all regions
	if (*ppCurrentRegion)
		CCollision__CheckRegion(*ppCurrentRegion) ;

	// Calculate final position
	if (pColl->m_Collision)
	{
		// Calculate intersection pt with line/circle
		t = pColl->m_t ;

		// Move to intersection pt
		pColl->m_vFinalPos.x = pColl->m_Line.m_vPt[0].x + (t * pColl->m_Line.m_vDelta.x) ;
		pColl->m_vFinalPos.z = pColl->m_Line.m_vPt[0].z + (t * pColl->m_Line.m_vDelta.z) ;

		// It's fudge time - kinda -move back a tad more to fix walking through stuff bugette!
		pColl->m_vFinalPos.x -= pColl->m_Line.m_vDir.x * 0.1 ;
		pColl->m_vFinalPos.z -= pColl->m_Line.m_vDir.z * 0.1 ;
	}
	else
		pColl->m_vFinalPos = pColl->m_Line.m_vPt[1] ;
	
	// Clear recursion flag and find final position region
	if (*ppCurrentRegion)
		CCollision__ClearRegionRecurseFlag(*ppCurrentRegion) ;

	// Put instance to final position
	if (pColl->m_pFinalRegion)
	{
		pvCurrentPos->x = pColl->m_vFinalPos.x ;
		pvCurrentPos->y = pColl->m_vFinalPos.y ;
		pvCurrentPos->z = pColl->m_vFinalPos.z ;
		*ppCurrentRegion = pColl->m_pFinalRegion ;

		// Stick instance back to the ground if it was on the ground to begin with
		// Also puts instance to correct ceiling height so that particle fx's show up (z buffer thing)
		if ((pCI->m_GroundCollision == GROUNDCOLLISION_GROUND) || (pColl->m_OnGround))
			pvCurrentPos->y = CGameRegion__GetGroundHeight(*ppCurrentRegion, pvCurrentPos->x, pvCurrentPos->z) ;
		else
		if (pCI->m_GroundCollision == GROUNDCOLLISION_CEILING)
			pvCurrentPos->y = CGameRegion__GetCeilingHeight(*ppCurrentRegion, pvCurrentPos->x, pvCurrentPos->z) - (1*SCALING_FACTOR) ;

	}
	else
		return TRUE ;

//----------------------------------------------------------------------------
//	Check ground/ceiling collision
//----------------------------------------------------------------------------

	if (pCI->m_GroundCollision)
	{
		pCI->m_vGroundCollisionPos = *pvCurrentPos ;
		pThis->m_vVelocity.y = 0 ;

#if 0
		// Bounce?
		if (pCI->m_dwFlags & COLLISIONFLAG_BOUNCEOFFGROUND)
		{
			// Bounce velocity off ceiling/ground
			if (pCI->m_GroundCollision == GROUNDCOLLISION_GROUND)
				vNormal = CGameRegion__GetGroundUnitNormal(*ppCurrentRegion) ;
			else
				vNormal = CGameRegion__GetCeilingUnitNormal(*ppCurrentRegion);

			pThis->m_vVelocity = CVector3__Reflect(&pThis->m_vVelocity, &vNormal, pCI->m_BounceReturnEnergy) ;
		}
		else
			pThis->m_vVelocity.y = 0 ;
#endif

	}



//----------------------------------------------------------------------------
//	Decide what to if a collision occurs
//----------------------------------------------------------------------------
	Slide = FALSE ;
	Bounce = FALSE ;
	Stick = FALSE ;
	Grease = FALSE ;
	if ((pColl->m_Collision) && (FirstCall))
	{
		// Instance collision
		if (pCI->m_pInstanceCollision)
		{
			pCI->m_vInstanceCollisionPos = *pvCurrentPos ;

			// Slide?
			if (pCI->m_InstanceBehavior == INSTANCE_SLIDE)
				Slide = TRUE ;

			// With grease?
			if (pCI->m_InstanceBehavior == INSTANCE_GREASE)
			{
				Slide = TRUE ;
				Grease = TRUE ;
			}

			// Stick?
			if (pCI->m_InstanceBehavior == INSTANCE_STICK)
				Stick = TRUE ;
		}

		// Wall collision>
		if (pCI->m_WallCollision)
		{
			pCI->m_vWallCollisionPos = *pvCurrentPos ;

			// Slide?
			if (pCI->m_WallBehavior == WALL_SLIDE)
				Slide = TRUE ;

			// Slide and bounce?
			if (pCI->m_WallBehavior == WALL_SLIDEBOUNCE)
			{
				Slide = TRUE ;
				Bounce = TRUE ;
			}

			// Bounce?
			if (pCI->m_WallBehavior == WALL_BOUNCE)
				Bounce = TRUE ;

			// Redirect?
			if (pCI->m_WallBehavior == WALL_REDIRECT)
				Bounce = TRUE ;

			// Stick?
			if (pCI->m_WallBehavior == WALL_STICK)
				Stick = TRUE ;
		}
	}

//----------------------------------------------------------------------------
//	Stick
//----------------------------------------------------------------------------
	if (Stick)
	{
		pThis->m_vVelocity.x = 0 ;
		pThis->m_vVelocity.y = 0 ;
		pThis->m_vVelocity.z = 0 ;
	}		

//----------------------------------------------------------------------------
//	Slide movement 
//----------------------------------------------------------------------------
	if (Slide)
	{
		t = 1.0 - t ;

		// Calculate Slide position
		CLine__CalculateXZDirection(&pColl->m_CollisionLine) ;

		Dot = (pColl->m_CollisionLine.m_vDir.x * pColl->m_Line.m_vDir.x) +
				(pColl->m_CollisionLine.m_vDir.z * pColl->m_Line.m_vDir.z) ;

		// This quick grease test could be improved
		if (Grease)
			Dot *= 2 ;

		Speed = sqrt((vVelocity.x * vVelocity.x) + (vVelocity.z * vVelocity.z)) ;
		vVelocity.x = (t*pColl->m_CollisionLine.m_vDir.x*Dot*Speed) ; 
		vVelocity.y = 0 ;
		vVelocity.z = (t*pColl->m_CollisionLine.m_vDir.z*Dot*Speed) ; 

		// Call collision again
		CAnimInstanceHdr__Collision2(pThis, ppCurrentRegion, pvCurrentPos, vVelocity, pCI, 
											  ClimbUp, ClimbDown, EnterWater, ExitWater, FALSE) ;

		return TRUE ;
	}

//----------------------------------------------------------------------------
//	Bounce instance if collision occured
//----------------------------------------------------------------------------
//	if (Bounce)
//		pThis->m_vVelocity = CVector3__Reflect(&vVelocity, &pColl->m_CollisionLine.m_vNormal, pCI->m_BounceReturnEnergy);


//----------------------------------------------------------------------------
//	The end
//----------------------------------------------------------------------------

#if COLLISION_SHOW_STATS
	// Update totals
	if (FirstCall)
		rmonPrintf("Results: RegionsRcrsd:%d CircleInts:%d LinesInts:%d\r\n", 
					  pColl->m_RegionsRecursed, pColl->m_CircleIntersections, pColl->m_LineIntersections) ;
#endif

	return (pColl->m_Collision) ;
}






// INSERT THIS INTO ROMSTRUC.CPP COLLISION FUNCTION

#if 0
	CVector3__MultScaler(&vVelocity, &pThis->m_vVelocity, frame_increment) ;
	CVector3__Add(&vVelocity, &vVelocity, &vDesiredPos) ;
	CVector3__Subtract(&vVelocity, &vVelocity, pvCurrentPos) ;

	return CAnimInstanceHdr__Collision2(pThis,
										  ppCurrentRegion,
										  pvCurrentPos,
										  vVelocity,
										  pCI,
										  ClimbUp, ClimbDown, EnterWater, ExitWater, TRUE) ;
#endif
		
// INSERT THIS INTO ROMSTRUC.CPP COLLISION FUNCTION

