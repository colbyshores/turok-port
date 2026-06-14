// wradcol.cpp
//

#include "stdafx.h"

#include "tengine.h"
#include "wradcol.h"
#include "romstruc.h"
#include "scaling.h"
#include "regtype.h"

// global
CWallRadiusCollision2	*p_wc;

// CWallRadiusCollision2 implementation
/////////////////////////////////////////////////////////////////////////////

void		CWallRadiusCollision2__Recurse(CGameRegion *pCurrentRegion);
void		CWallRadiusCollision2__ClearRegionRecurseFlag(CGameRegion *pRegion);

// CWallRadiusCollision2
/////////////////////////////////////////////////////////////////////////////

void CWallRadiusCollision2__Recurse(CGameRegion *pCurrentRegion)
{
	char	cEdge;
	BYTE	cornersChecked[3];

	// limit recursion if we're running out of stack space
	if ( (DWORD)&pCurrentRegion < ((DWORD)main_thread_stack + MAIN_STACKSIZE/10) )
		return;

	pCurrentRegion->m_wFlags |= REGFLAG_RECURSE;

	cornersChecked[0] = cornersChecked[1] = cornersChecked[2] = 0;

	for (cEdge=0; cEdge<3; cEdge++)
	{
		p_wc->pNeighbor = pCurrentRegion->m_pNeighbors[cEdge];
		if (p_wc->pNeighbor)
		{
			if (p_wc->pNeighbor->m_wFlags & REGFLAG_RECURSE)
			{
				// already been there
				continue;
			}
			else if (pCurrentRegion->m_wFlags & REGFLAG_BRIDGE)
			{
				if (		(p_wc->pNeighbor->m_pNeighbors[0] != pCurrentRegion)
						&& (p_wc->pNeighbor->m_pNeighbors[1] != pCurrentRegion)
						&& (p_wc->pNeighbor->m_pNeighbors[2] != pCurrentRegion) )
				{
					// don't recurse off bridge edges
					continue;
				}
			}
			else
			{
				// connection to another region
				p_wc->pNeighbor = CCollide__CanEnter(p_wc->pCollide, pCurrentRegion, cEdge, &p_wc->pCollide->pInst->ih.m_vPos);
				
				// since we're not recursing off bridges, neighbor should never jump
				ASSERT(!p_wc->pNeighbor || (p_wc->pNeighbor == pCurrentRegion->m_pNeighbors[cEdge]));
			}
		}

		p_wc->pvCornerA = &pCurrentRegion->m_pCorners[cEdge]->m_vCorner;
		p_wc->pvCornerB = &pCurrentRegion->m_pCorners[(cEdge == 2) ? 0 : (cEdge + 1)]->m_vCorner;

		if (		((p_wc->pvCornerA->x <= p_wc->Bounds.m_MinX) && (p_wc->pvCornerB->x <= p_wc->Bounds.m_MinX))
				|| ((p_wc->pvCornerA->x >= p_wc->Bounds.m_MaxX) && (p_wc->pvCornerB->x >= p_wc->Bounds.m_MaxX))
				|| ((p_wc->pvCornerA->z <= p_wc->Bounds.m_MinZ) && (p_wc->pvCornerB->z <= p_wc->Bounds.m_MinZ))
				|| ((p_wc->pvCornerA->z >= p_wc->Bounds.m_MaxZ) && (p_wc->pvCornerB->z >= p_wc->Bounds.m_MaxZ)) )
		{
			// edge is outside of bounds
			continue;
		}

		if (p_wc->pNeighbor)
		{
			// recurse
			CWallRadiusCollision2__Recurse(p_wc->pNeighbor);
		}
		else
		{
			// collide

			p_wc->vEdgeNormal.x = p_wc->pvCornerB->z - p_wc->pvCornerA->z;
			p_wc->vEdgeNormal.z = p_wc->pvCornerA->x - p_wc->pvCornerB->x;

			p_wc->vEdgeDelta.x = p_wc->pCollide->pInst->ih.m_vPos.x - p_wc->pvCornerA->x;
			p_wc->vEdgeDelta.z = p_wc->pCollide->pInst->ih.m_vPos.z - p_wc->pvCornerA->z;
			
			// only test edges that instance is on the proper side of
			p_wc->SideDot = p_wc->vEdgeNormal.x*p_wc->vEdgeDelta.x + p_wc->vEdgeNormal.z*p_wc->vEdgeDelta.z;
			if (p_wc->SideDot >= 0.0)
			{
				p_wc->vNormal.x = -p_wc->pCollide->vDelta.z;
				p_wc->vNormal.z = p_wc->pCollide->vDelta.x;

				p_wc->vMoveDelta.x = p_wc->pvCornerB->x - p_wc->pvCornerA->x;
				p_wc->vMoveDelta.z = p_wc->pvCornerB->z - p_wc->pvCornerA->z;

				p_wc->NormalDotMovementDelta = p_wc->vNormal.x*p_wc->vMoveDelta.x + p_wc->vNormal.z*p_wc->vMoveDelta.z;
				if (p_wc->NormalDotMovementDelta < 0.0)
				{
					// movement is toward edge
					p_wc->Intersected = FALSE;

					if (!cornersChecked[cEdge])
					{
						cornersChecked[cEdge] = 1;

						p_wc->Intersected |= CCollide__IntersectCylinder(p_wc->pCollide,
																						 p_wc->pvCornerA, p_wc->Radius,
																						 FALSE, 0.0, 0.0,
																						 TRUE,
																						 p_wc->pCollide->pCI->WallBehavior,
																						 0.0);
					}
					
					if (!cornersChecked[(cEdge == 2) ? 0 : (cEdge + 1)])
					{
						cornersChecked[(cEdge == 2) ? 0 : (cEdge + 1)] = 1;

						p_wc->Intersected |= CCollide__IntersectCylinder(p_wc->pCollide,
																						 p_wc->pvCornerB, p_wc->Radius,
																						 FALSE, 0.0, 0.0,
																						 TRUE,
																						 p_wc->pCollide->pCI->WallBehavior,
																						 0.0);
					}

					p_wc->Mag = sqrt(p_wc->vEdgeNormal.x*p_wc->vEdgeNormal.x + p_wc->vEdgeNormal.z*p_wc->vEdgeNormal.z);
					if (p_wc->Mag != 0.0)
					{
						p_wc->Mag = p_wc->Radius/p_wc->Mag;

						// project point away from edge
						p_wc->vPoint.x = p_wc->pvCornerA->x + p_wc->vEdgeNormal.x*p_wc->Mag;
						p_wc->vPoint.z = p_wc->pvCornerA->z + p_wc->vEdgeNormal.z*p_wc->Mag;

						p_wc->vPointDelta.x = p_wc->pCollide->vDesired.x - p_wc->vPoint.x;
						p_wc->vPointDelta.z = p_wc->pCollide->vDesired.z - p_wc->vPoint.z;

						p_wc->NormalDotPointDelta = p_wc->vNormal.x*p_wc->vPointDelta.x + p_wc->vNormal.z*p_wc->vPointDelta.z;

						p_wc->t = p_wc->NormalDotPointDelta/p_wc->NormalDotMovementDelta;
						if (		(p_wc->t >= 0.0) && (p_wc->t <= 1.0)
								&&	!CCollide__IsSlideCylinder(p_wc->pCollide, p_wc->pvCornerA, p_wc->Radius)
								&&	!CCollide__IsSlideCylinder(p_wc->pCollide, p_wc->pvCornerB, p_wc->Radius) )
						{
							p_wc->Intersected |= CCollide__IntersectPlane(p_wc->pCollide,
																						 &p_wc->vEdgeNormal, &p_wc->vPoint,
																						 FALSE,	// already done
																						 p_wc->pCollide->pCI->WallBehavior,
																						 0.0);
						}
					}

					if (p_wc->Intersected)
					{
						// set destination of collision data if this turns out to
						// be closest collision
						CClosestIntersect__SetStoreDest(&p_wc->pCollide->clint,
																  &p_wc->pCollide->pCI->vWallCollisionPos,
																  &p_wc->pCollide->pCI->pWallCollisionRegion,
																  NULL, 0,
																  NULL, NULL);

						// wall collision edge doesn't signify collision, so it can be set now
						p_wc->pCollide->pCI->nWallCollisionEdge = cEdge;
					}
				}
			}
		}
	}
}

void CWallRadiusCollision2__ClearRegionRecurseFlag(CGameRegion *pRegion)
{
	// clear flag
	pRegion->m_wFlags &= ~REGFLAG_RECURSE;

	// check neighbor 0
	if ((pRegion->m_pNeighbors[0]) && (pRegion->m_pNeighbors[0]->m_wFlags & REGFLAG_RECURSE))
		CWallRadiusCollision2__ClearRegionRecurseFlag(pRegion->m_pNeighbors[0]) ;

	// check neighbor 1
	if ((pRegion->m_pNeighbors[1]) && (pRegion->m_pNeighbors[1]->m_wFlags & REGFLAG_RECURSE))
		CWallRadiusCollision2__ClearRegionRecurseFlag(pRegion->m_pNeighbors[1]) ;

	// check neighbor 2
	if ((pRegion->m_pNeighbors[2]) && (pRegion->m_pNeighbors[2]->m_wFlags & REGFLAG_RECURSE))
		CWallRadiusCollision2__ClearRegionRecurseFlag(pRegion->m_pNeighbors[2]) ;
}


// CCollide
/////////////////////////////////////////////////////////////////////////////

void CCollide__WallRadiusCollision2(CCollide *pThis)
{
	CGameRegion					*pRegion;
	CWallRadiusCollision2	wallCollision;
	float							minX, maxX, minZ, maxZ;

	if (		!(pThis->pCI->dwFlags & COLLISIONFLAG_USEWALLRADIUS)
			|| (pThis->pInst->ih.m_pEA->m_CollisionWallRadius == 0)
			|| (pThis->pCI->WallBehavior == INTERSECT_BEHAVIOR_IGNORE) )
	{
		return;
	}

	pRegion = pThis->pInst->ih.m_pCurrentRegion;
	if (!pRegion)
		return;

	if (pThis->pInst->ih.m_Type == I_ANIMATED)
	{
		if (		(pThis->pInst != &CEngineApp__GetPlayer(GetApp())->ah)
				&& !CGameObjectInstance__IsVisible((CGameObjectInstance*) pThis->pInst) )
		{
			// don't bother to use wall radius collision
			// if character isn't on the screen
			return;
		}
	}

	p_wc = &wallCollision;
	
	p_wc->Radius = pThis->pInst->ih.m_pEA->m_CollisionWallRadius;
	if (pThis->pCI->dwFlags & COLLISIONFLAG_SMALLWALLRADIUS)
		p_wc->Radius *= 0.5;

	p_wc->pCollide = pThis;
	p_wc->vEdgeNormal.y = 0.0;
	p_wc->vPoint.y = 0.0;
	
	if (pThis->pInst->ih.m_vPos.x < pThis->vDesired.x)
	{
		minX = pThis->pInst->ih.m_vPos.x;
		maxX = pThis->vDesired.x;
	}
	else
	{
		minX = pThis->vDesired.x;
		maxX = pThis->pInst->ih.m_vPos.x;
	}
	p_wc->Bounds.m_MinX = minX - p_wc->Radius;
	p_wc->Bounds.m_MaxX = maxX + p_wc->Radius;

	if (pThis->pInst->ih.m_vPos.z < pThis->vDesired.z)
	{
		minZ = pThis->pInst->ih.m_vPos.z;
		maxZ = pThis->vDesired.z;
	}
	else
	{
		minZ = pThis->vDesired.z;
		maxZ = pThis->pInst->ih.m_vPos.z;
	}
	p_wc->Bounds.m_MinZ = minZ - p_wc->Radius;
	p_wc->Bounds.m_MaxZ = maxZ + p_wc->Radius;

	CWallRadiusCollision2__Recurse(pRegion);
	CWallRadiusCollision2__ClearRegionRecurseFlag(pRegion);
}

