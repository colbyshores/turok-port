// regicol.cpp
//

#include "stdafx.h"

#include "tengine.h"
#include "regicol.h"
#include "romstruc.h"
#include "scaling.h"
#include "regtype.h"

// CCollide
/////////////////////////////////////////////////////////////////////////////

void CCollide__RegionCollision(CCollide *pThis)
{
#define REGION_COLLISION_MAX_ITERATIONS	100	// safety cap for loop

	float				t, edgeMinT,
						groundT, ceilingT, waterT,
						normalDotDelta,
						edgeNormalX, edgeNormalZ,
						denom, slideDot,
						lowestT,
						normalDotCornerDelta,
						desiredGroundHeight;
	CGameRegion		**ppCurrentRegion,
						*pEnter;
	int				cEdge, nClosestEdge, cIteration,
						groundBehavior;
	CVector3			*pvCurrentPos,
						vNormal,
						vGroundNormal,
						vCeilingNormal,
						vUpNormal = { 0.0, 1.0, 0.0 },
						vDownNormal = { 0.0, -1.0, 0.0 },
						vPointOnGround = { 0.0, 0.0, 0.0 },
						*pvCornerA, *pvCornerB,
						vCornerDelta,
						vCeilingCorner,
						vIntersect,
						vComparePos;
	BOOL				doGroundCollision, doCeilingCollision;
	CROMRegionSet	*pRegionSet;

	if (pThis->pCI->WallBehavior == INTERSECT_BEHAVIOR_IGNORE)
		return;

	pvCurrentPos = &pThis->pInst->ih.m_vPos;
	ppCurrentRegion = &pThis->pInst->ih.m_pCurrentRegion;
	lowestT = -10000.0;

	if (*ppCurrentRegion)
	{
		cIteration = 0;

		do
		{
			// intersect with ground plane
			groundT = 1.0;

			if (pThis->StartedInDeathFall)
				groundBehavior = CCollide__GetGroundBehavior(pThis, pThis->pInst->ih.m_pCurrentRegion);
			else
				groundBehavior = pThis->pCI->GroundBehavior;

			if (		!pThis->TrackingGround
//					&&	(pThis->pCI->GroundBehavior != INTERSECT_BEHAVIOR_IGNORE) )
					&& (groundBehavior != INTERSECT_BEHAVIOR_IGNORE) )
			{
				vGroundNormal = CGameRegion__GetGroundNormal(*ppCurrentRegion);
				
				doGroundCollision = TRUE;

/*				
				// don't do ground collision if it is nearly parallel to slide
				if (	(		(pThis->pCI->GroundBehavior == INTERSECT_BEHAVIOR_SLIDE)
							|| (pThis->pCI->GroundBehavior == INTERSECT_BEHAVIOR_REDIRECT)
							|| (pThis->pCI->GroundBehavior == INTERSECT_BEHAVIOR_GREASE)
							|| (pThis->pCI->GroundBehavior == INTERSECT_BEHAVIOR_SLIDEBOUNCE) )
						&& (pThis->SlidingType == SLIDING_TYPE_PLANE) )
				{
					denom = CVector3__Magnitude(&vGroundNormal)*CVector3__Magnitude(&pThis->vSliding);
					if (denom != 0.0)
					{
						slideDot = CVector3__Dot(&vGroundNormal, &pThis->vSliding);
						if (slideDot/denom < 0.999)
							doGroundCollision = FALSE;
					}
				}
*/
				if (doGroundCollision)
				{

					normalDotDelta = CVector3__Dot(&vGroundNormal, &pThis->vDelta);
					if (normalDotDelta < 0.0)
					{
						CVector3__Subtract(&vCornerDelta,
												 &(*ppCurrentRegion)->m_pCorners[0]->m_vCorner,
												 pvCurrentPos);
						normalDotCornerDelta = CVector3__Dot(&vGroundNormal, &vCornerDelta);

						groundT = normalDotCornerDelta/normalDotDelta;
						
						if (groundT < lowestT)
							groundT = 1.0;
					}
				}
			}
						
			// intersect with ceiling plane
			ceilingT = 1.0;
			if (		(pThis->pCI->GroundBehavior != INTERSECT_BEHAVIOR_IGNORE)
					&& ((*ppCurrentRegion)->m_wFlags & REGFLAG_CEILING) )
			{
				vCeilingNormal = CGameRegion__GetCeilingNormal(*ppCurrentRegion);
				
				doCeilingCollision = TRUE;
/*				
				// don't do ceiling collision if it is nearly parallel to slide
				if (	(		(pThis->pCI->GroundBehavior == INTERSECT_BEHAVIOR_SLIDE)
							|| (pThis->pCI->GroundBehavior == INTERSECT_BEHAVIOR_REDIRECT)
							|| (pThis->pCI->GroundBehavior == INTERSECT_BEHAVIOR_GREASE)
							|| (pThis->pCI->GroundBehavior == INTERSECT_BEHAVIOR_SLIDEBOUNCE) )
						&& (pThis->SlidingType == SLIDING_TYPE_PLANE) )
				{
					denom = CVector3__Magnitude(&vCeilingNormal)*CVector3__Magnitude(&pThis->vSliding);
					if (denom != 0.0)
					{
						slideDot = CVector3__Dot(&vCeilingNormal, &pThis->vSliding);
						if (slideDot/denom < 0.999)
							doCeilingCollision = FALSE;
					}
				}
*/
				if (doCeilingCollision)
				{

					normalDotDelta = CVector3__Dot(&vCeilingNormal, &pThis->vDelta);
					if (normalDotDelta < 0.0)
					{
						vCeilingCorner.x = (*ppCurrentRegion)->m_pCorners[0]->m_vCorner.x;
						
						vCeilingCorner.y = (*ppCurrentRegion)->m_pCorners[0]->m_Ceiling;
						if (pThis->pCI->dwFlags & COLLISIONFLAG_USEHEIGHT)
						{
							vCeilingCorner.y -= pThis->pInst->ih.m_pEA->m_CollisionHeight;

#define DUCK_HEIGHT_OFFSET	(4.3*SCALING_FACTOR)
							if (pThis->pCI->dwFlags & COLLISIONFLAG_DUCKING)
								vCeilingCorner.y += DUCK_HEIGHT_OFFSET;
						}

						vCeilingCorner.z = (*ppCurrentRegion)->m_pCorners[0]->m_vCorner.z;

						CVector3__Subtract(&vCornerDelta,
												 &vCeilingCorner,
												 pvCurrentPos);
						normalDotCornerDelta = CVector3__Dot(&vCeilingNormal, &vCornerDelta);

						ceilingT = normalDotCornerDelta/normalDotDelta;

						if (ceilingT < lowestT)
							ceilingT = 1.0;
					}
				}
			}

			// intersect with water plane
			waterT = 1.0;
			if (		((*ppCurrentRegion)->m_wFlags & REGFLAG_HASWATER)
					&&	(pThis->pCI->dwFlags & COLLISIONFLAG_WATERCOLLISION)
					&& !pThis->pCI->pWaterCollisionRegion )
			{
				pRegionSet = CScene__GetRegionAttributes(&GetApp()->m_Scene, *ppCurrentRegion);
				if (pRegionSet)
				{
					if (pThis->vDelta.y != 0.0)
					{
						waterT = (pRegionSet->m_WaterElevation - pvCurrentPos->y)/pThis->vDelta.y;

						if (waterT < lowestT)
							waterT = 1.0;
					}
				}
			}


			// only pay attention to edges that are closer than current closest
			// collision
			edgeMinT = pThis->clint.t;

			for (cEdge=0; cEdge<3; cEdge++)
			{
				pvCornerA = &(*ppCurrentRegion)->m_pCorners[cEdge]->m_vCorner;
				pvCornerB = &(*ppCurrentRegion)->m_pCorners[(cEdge == 2) ? 0 : (cEdge + 1)]->m_vCorner;

				edgeNormalX = pvCornerB->z - pvCornerA->z;
				edgeNormalZ = pvCornerA->x - pvCornerB->x;

				normalDotDelta = edgeNormalX*pThis->vDelta.x + edgeNormalZ*pThis->vDelta.z;
				
				// back cull edge
				if (normalDotDelta < 0.0)
				{
					// find intersection with edge
					t = (	edgeNormalX*(pvCornerA->x - pvCurrentPos->x)
							+ edgeNormalZ*(pvCornerA->z - pvCurrentPos->z) )
						 / normalDotDelta;

					if (t < edgeMinT)
					{
						// this is currently closest edge

						if (		(pThis->SlidingType != SLIDING_TYPE_PLANE)
								|| (pThis->vSliding.x != edgeNormalX)
								|| (pThis->vSliding.y != 0.0)
								|| (pThis->vSliding.z != edgeNormalZ) )
						{
							vNormal.x = edgeNormalX;
							vNormal.z = edgeNormalZ;

							edgeMinT = t;
							nClosestEdge = cEdge;
						}
					}
				}
			}

//			if (		(waterT <= groundT)
//					&& (waterT <= ceilingT)
//					&& (waterT <= edgeMinT)
//					&& (waterT < pThis->clint.t) )
			if (		(waterT <= edgeMinT)
					&&	(waterT < pThis->clint.t) )
			{
				// water intersection

				vPointOnGround.y = pRegionSet->m_WaterElevation;

				if (CCollide__IntersectPlane(pThis,
													  (pvCurrentPos->y >= pRegionSet->m_WaterElevation) ? &vUpNormal : &vDownNormal,
													  &vPointOnGround,
													  TRUE,
													  INTERSECT_BEHAVIOR_ITERATE,
													  0.0))
				{
					// set destination of collision data if this turns out to be
					// closest collision
					CClosestIntersect__SetStoreDest(&pThis->clint,
															  &pThis->pCI->vWaterCollisionPos,
															  &pThis->pCI->pWaterCollisionRegion,
															  NULL, 0,
															  NULL, NULL);
				}
			}


//			if (		(groundT <= ceilingT)
//					&& (groundT <= edgeMinT)
//					&& (groundT < pThis->clint.t) )
// TEMP
//			if (doGroundCollision && (groundT >= lowestT) && !pThis->TrackingGround)
			if (		(groundT <= edgeMinT)
					&&	(groundT < pThis->clint.t) )
			{
// TEMP
//vGroundNormal = CGameRegion__GetGroundNormal(*ppCurrentRegion);

				// ground intersection

				if (CCollide__IntersectPlane(pThis,
													  &vGroundNormal,
													  &(*ppCurrentRegion)->m_pCorners[0]->m_vCorner,
													  FALSE,		// already back culled
// TEMP
//													  TRUE,
													  pThis->pCI->GroundBehavior,
//													  (pThis->pCI->dwFlags & COLLISIONFLAG_TRACKGROUND) ? INTERSECT_BEHAVIOR_STICK : pThis->pCI->GroundBehavior,
// TEMP
//													  INTERSECT_BEHAVIOR_STICK,
													  CGameRegion__IsCliff(*ppCurrentRegion) ? 0.0 : pThis->pCI->GroundFriction))
				{
					// set destination of collision data if this turns out to be
					// closest collision
					CClosestIntersect__SetStoreDest(&pThis->clint,
															  &pThis->pCI->vGroundCollisionPos,
															  &pThis->pCI->pGroundCollisionRegion,
															  &pThis->pCI->nGroundCollisionType, GROUNDCOLLISION_GROUND,
															  NULL, NULL);
				}
			}

			if (		(ceilingT <= edgeMinT)
					&& (ceilingT < pThis->clint.t) )
			{
				// ceiling intersection

				if (CCollide__IntersectPlane(pThis,
													  &vCeilingNormal,
													  &vCeilingCorner,
													  FALSE,		// already back culled
													  pThis->pCI->WallBehavior,
													  0.0))
				{
					// set destination of collision data if this turns out to be
					// closest collision
					CClosestIntersect__SetStoreDest(&pThis->clint,
															  &pThis->pCI->vGroundCollisionPos,
															  &pThis->pCI->pGroundCollisionRegion,
															  &pThis->pCI->nGroundCollisionType, GROUNDCOLLISION_CEILING,
															  NULL, NULL);
				}
			}

			if (edgeMinT < pThis->clint.t)
			{
				// region wall intersection

				CVector3__Blend(&vComparePos, edgeMinT, pvCurrentPos, &pThis->vDesired);

				pEnter = CCollide__CanEnter(pThis, *ppCurrentRegion, nClosestEdge, &vComparePos);
/*
				if (		(pThis->pCI->GroundBehavior != INTERSECT_BEHAVIOR_IGNORE)
						&& !pThis->TrackingGround
						&& pEnter )
				{
					// Make sure instance isn't entering region before intersecting ground leading
					// up to it.  This extra test is necessary because of precision problems.
					// Occasionally, with steep slopes, the region edge intersection t is
					// incorrectly lower than the ground t.
				
					CVector3__Blend(&vIntersect, edgeMinT, pvCurrentPos, &pThis->vDesired);

					desiredGroundHeight = CGameRegion__GetGroundHeight(pEnter, vIntersect.x, vIntersect.z);

#define GROUND_INTERSECT_PRECISION_THRESHOLD		(1.0*SCALING_FACTOR)

					if ((desiredGroundHeight - vIntersect.y) > GROUND_INTERSECT_PRECISION_THRESHOLD)
				}
*/

				if (		!pEnter
						|| pThis->TrackingGround
						|| ((pThis->pCI->dwFlags & COLLISIONFLAG_TRACKGROUND) && (pEnter->m_wFlags & REGFLAG_CLIFF)) )	// climbable cliff
// TEMP
//				if (!pEnter || (pThis->pCI->dwFlags & COLLISIONFLAG_TRACKGROUND))
				{
					vNormal.y = 0;

					// intersect with edge plane
					if (CCollide__IntersectPlane(pThis,
														  &vNormal,
														  &(*ppCurrentRegion)->m_pCorners[nClosestEdge]->m_vCorner,
														  FALSE,
														  pEnter ? INTERSECT_BEHAVIOR_ITERATE : pThis->pCI->WallBehavior,
// TEMP
//														  pEnter ? INTERSECT_BEHAVIOR_ITERATE : INTERSECT_BEHAVIOR_STICK,
														  0.0))
					{
						if (pEnter)
						{
							*ppCurrentRegion = pEnter;
							
							CClosestIntersect__SetStoreDest(&pThis->clint,
																	  NULL,
																	  NULL,
																	  NULL, 0,
																	  NULL, NULL);
						}
						else
						{
							// set destination of collision data if this turns out to
							// be closest collision
							CClosestIntersect__SetStoreDest(&pThis->clint,
																	  &pThis->pCI->vWallCollisionPos,
																	  &pThis->pCI->pWallCollisionRegion,
																	  NULL, 0,
																	  NULL, NULL);

							// wall collision edge doesn't signify collision, so it can be set now
							pThis->pCI->nWallCollisionEdge = nClosestEdge;

							// edge was closest collision, so return
						}
					}
					else
					{
						// 2D calculation said motion intersected edge,
						// but 3D calculation said it didn't.
						// A precision error could cause this if edgeMinT
						// is really close to pThis->clint.t, or if movement
						// is parallel to plane.  Either way, it's okay to
						// skip collision and return.
					}

					return;
				}
				else
				{
					// entered neighboring region without collision
					// continue loop
					*ppCurrentRegion = pEnter;

					// don't intersect with ground, ceiling, or water before entering region
					lowestT = edgeMinT;
				}
			}
			else
			{
				// nothing closer than current closest collision
				return;
			}

			cIteration++;

		} while (cIteration < REGION_COLLISION_MAX_ITERATIONS);
	}
	else	// !(*ppCurrentRegion)
	{
		// intersect with ground plane at y=0
		vPointOnGround.y = -1400.0*SCALING_FACTOR;
		
		if (CCollide__IntersectPlane(pThis,
											  &vUpNormal,
											  &vPointOnGround,
											  TRUE,
											  pThis->pCI->GroundBehavior,
											  pThis->pCI->GroundFriction))
		{
			// set destination of collision data if this turns out to be closest
			// collision
			CClosestIntersect__SetStoreDest(&pThis->clint,
													  &pThis->pCI->vGroundCollisionPos,
													  &pThis->pCI->pGroundCollisionRegion,
													  &pThis->pCI->nGroundCollisionType, GROUNDCOLLISION_GROUND,
													  NULL, NULL);
		}
	}
}

