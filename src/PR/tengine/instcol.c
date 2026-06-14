// instcol.cpp
//

#include "stdafx.h"

#include "tengine.h"
#include "instcol.h"
#include "romstruc.h"
#include "scaling.h"

// CCollide
/////////////////////////////////////////////////////////////////////////////

void CCollide__InstanceCollision(CCollide *pThis)
{
	int						cInst, nInsts;
	float						r1, r2, rt,
								x, z,
								db, dt,
								top, bottom,
								dtop, dbottom,
								itop, ibottom,
								ix, iz,
								dx, dz,
								t;
	CAnimInstanceHdr		*pThisInst;
	CGameObjectInstance	*pAlive;
	CInstanceHdr			*pInst;
	CBoundsRect				moveBounds, instBounds;
	BOOL						updown, collided, iterate;
	CVector3					vIntersect,
								vNormal, vCylNormal;
	CCollisionInfo2		*pCI;

	nInsts = CEngineApp__GetAllInstanceCount(GetApp());

	if (		(pThis->pCI->InstanceBehavior == INTERSECT_BEHAVIOR_IGNORE)
			|| !nInsts )
	{
		return;
	}

	pThisInst	= pThis->pInst;
	pCI			= pThis->pCI;

	r1 = pThisInst->ih.m_pEA->m_CollisionRadius;

	updown = pThis->pCI->dwFlags & COLLISIONFLAG_INSTANCEUPDOWN;
	if (updown)
		vNormal.x = vNormal.z = 0.0;
	vCylNormal.y = 0.0;

	db = pThisInst->ih.m_CollisionHeightOffset;
	dt = db + pThisInst->ih.m_pEA->m_CollisionHeight;

	bottom = pThisInst->ih.m_vPos.y + db;
	top = pThisInst->ih.m_vPos.y + dt;

	dbottom = pThis->vDesired.y + db;
	dtop = pThis->vDesired.y + dt;

	x = pThisInst->ih.m_vPos.x;
	z = pThisInst->ih.m_vPos.z;

	// movement bounds
	moveBounds.m_MinX = min(x, pThis->vDesired.x) - r1;
	moveBounds.m_MaxX = max(x, pThis->vDesired.x) + r1;
	moveBounds.m_MinZ = min(z, pThis->vDesired.z) - r1;
	moveBounds.m_MaxZ = max(z, pThis->vDesired.z) + r1;

	for (cInst=0; cInst<nInsts; cInst++)
	{
		iterate = FALSE;

		pInst = CEngineApp__GetInstance(GetApp(), cInst);

		// decide whether to perform collision

		// don't collide with self
		if (pInst == &pThisInst->ih)
			continue;

		// keep certain particles from colliding with their source
		if (pThisInst->ih.m_Type == I_PARTICLE)
		{
			if (		(pInst == ((CParticle*) pThisInst)->m_pSource)
					&& (((CParticle*) pThisInst)->m_pEffect->m_dwFlags & PARTICLE_BEHAVIOR_CANNOTHITSOURCE) )
			{
				continue;
			}

			// don't allow shooting of invisible types
			if (pInst->m_Type == I_ANIMATED)
			{
				pAlive = (CGameObjectInstance*) pInst;

				if (		(pAlive->m_AI.m_cRegenerateAppearance > 0.0)
						||	(pAlive->m_cMelt > 0.0) )
				{
					continue;
				}
			}
		}

		switch (pInst->m_Type)
		{
			case I_ANIMATED:
				pAlive = (CGameObjectInstance*) pInst;
				if (pAlive->m_AI.m_dwStatusFlags & AI_EXPTARGETEXPLODED)
				{
					continue;
				}
				else if (pCI->dwFlags & COLLISIONFLAG_IGNOREDEAD)
				{
					// some things don't collide with dead stuff
					// (mainly the player and particles)
					if (pAlive->m_AI.m_Health == 0)
					{
						continue;
					}
				}
				else if (pAlive->m_cMelt != 0.0)
				{
					// don't collide with stuff that's melting
					continue;
				}
				break;

			case I_SIMPLE:
				if (!(pCI->dwFlags & COLLISIONFLAG_PICKUPCOLLISION))
				{
					// only the player should be affected by pickups
					continue;
				}
				break;

			case I_PARTICLE:
				// this shouldn't happen because particles are never added
				// to the collision list
				ASSERT(FALSE);
				break;
		}

		r2 = pInst->m_pEA->m_CollisionRadius;
		rt = r1 + r2;

		ix = pInst->m_vPos.x;
		iz = pInst->m_vPos.z;

		instBounds.m_MinX = ix - r2;
		instBounds.m_MaxX = ix + r2;
		instBounds.m_MinZ = iz - r2;
		instBounds.m_MaxZ = iz + r2;

		if (CBoundsRect__IsOverlapping(&moveBounds, &instBounds))
		{
			if (pInst->m_Type == I_SIMPLE)
			{
				// walk right through pickups that aren't applicable
				if (CPickup__CanInstancePickItUp((CGameSimpleInstance*) pInst, pThisInst))
					iterate = TRUE;
				else
					continue;
			}

			collided = FALSE;

			ibottom = pInst->m_vPos.y + pInst->m_CollisionHeightOffset;
			
			if (		(pInst->m_Type == I_ANIMATED)
					&& (((CGameObjectInstance*)pInst)->m_AI.m_Health <= 0) )
			{
				itop = pInst->m_pEA->m_DeadHeight;
			}
			else
			{
				itop = pInst->m_pEA->m_CollisionHeight;
			}
			itop += ibottom;
			
			if (updown && (pThis->vDelta.y != 0.0))
			{
				if (CCollide__IsSlideCylinder(pThis, &pInst->m_vPos, rt))
					continue;

				if ((top <= ibottom) && (dtop > ibottom))
				{
					// pThisInst is below pInst

					t = (ibottom - top)/pThis->vDelta.y;
					if (t < pThis->clint.t)
					{
						CVector3__Blend(&vIntersect, t, &pThisInst->ih.m_vPos, &pThis->vDesired);

						dx = ix - vIntersect.x;
						dz = iz - vIntersect.z;

						// within radius?
						if ((dx*dx + dz*dz) < rt*rt)
						{
							vNormal.y = -1.0;

							collided |= CCollide__IntersectPlane(pThis,
																			 &vNormal, &vIntersect,
																			 FALSE,
																			 INTERSECT_BEHAVIOR_ITERATE,
																			 0.0);
						}
					}
				}

				if ((bottom >= itop) && (dbottom < itop))
				{
					// pThisInst is above pInst

					t = (itop - bottom)/pThis->vDelta.y;

					if (t < pThis->clint.t)
					{
						CVector3__Blend(&vIntersect, t, &pThisInst->ih.m_vPos, &pThis->vDesired);

						dx = ix - vIntersect.x;
						dz = iz - vIntersect.z;

						// within radius?
						if ((dx*dx + dz*dz) < rt*rt)
						{
							vNormal.y = 1.0;

							collided |= CCollide__IntersectPlane(pThis,
																			 &vNormal, &vIntersect,
																			 FALSE,
																			 INTERSECT_BEHAVIOR_ITERATE,
																			 0.0);
						}
					}
				}
			}

			collided |= CCollide__IntersectCylinder(pThis,
																 &pInst->m_vPos, rt,
																 TRUE, itop - db, ibottom - dt,
																 TRUE,
																 iterate ? INTERSECT_BEHAVIOR_ITERATE : pThis->pCI->InstanceBehavior,
																 0.0);

			if (collided)
			{
				// Don't overwrite first instance collision.
				// This insures that pickups get processed when another instance is right
				// behind it.  This was a problem where the keyholder overwrote the key.
				if (pThis->pCI->pInstanceCollision)
				{
					CClosestIntersect__Reset(&pThis->clint);
				}
				else
				{
					// set destination of collision data if this turns out to be
					// closest collision
					CClosestIntersect__SetStoreDest(&pThis->clint,
															  &pThis->pCI->vInstanceCollisionPos,
															  &pThis->pCI->pInstanceCollisionRegion,
															  NULL, 0,
															  &pThis->pCI->pInstanceCollision, pInst);
				}

				if (updown)
					CCollide__SetSlideCylinder(pThis, &pInst->m_vPos, rt);
			}
		}
	}
}
