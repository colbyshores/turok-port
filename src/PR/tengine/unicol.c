// unicol.cpp
//

#include "stdafx.h"

#include "tengine.h"
#include "unicol.h"
#include "romstruc.h"
#include "scaling.h"
#include "regtype.h"
#include "collinfo.h"

// collision systems
#include "instcol.h"
#include "wradcol.h"
#include "regicol.h"

// globals

// collision routine list
/////////////////////////////////////////////////////////////////////////////

typedef void (*pfnCOLLISIONFUNC)(CCollide *pThis);
pfnCOLLISIONFUNC pfn_collision_routines[] =
{
	CCollide__InstanceCollision,			// collision routine for cylindrical
													//   instances
	CCollide__WallRadiusCollision2,		// collision routine to keep characters
													//   away from walls
	CCollide__RegionCollision,				// collision routine for landscapes
													// NOTE: The region collision routine
													//   must be the last in the list.
													//   It updates the instance's current
													//   region after each movement.

	NULL											// end of list
};


// Universal Collision Routine
/////////////////////////////////////////////////////////////////////////////

int CAnimInstanceHdr__Collision3(CAnimInstanceHdr *pThis,
											CVector3 vDesired,
											CCollisionInfo2 *pCI)
{
#define COLLISION_MAX_ITERATIONS	100	// safety cap for loop

	int					cf,					// current collision routine
							cIteration,			// iteration counter
							cCollision,			// collision counter
							nMaxCollisions;	// max number of collisions allowed
//							prevGroundBehavior;
	CCollide				cd;					// collision data shared by collision
													//   routines
	float					remainT,				// remaining motion (range: 0.0 - 1.0)
							dt,					// time (sec) for movement computation
							dtd2,
							friction,			// air/water/ground friction
													//   (range: 0.0 - 1.0)
							groundHeight,
							startYPos;
	CVector3				vAcceleration,		// total acceleration
							vZero = { 0.0, 0.0, 0.0 },
							vGroundUnitNormal,
							vCornerDelta,
							vProjected,
							*pvCorner;
	BOOL					newDesiredPos,		// desired pos changed
							slideHeight;
	CROMRegionSet		*pRegionSet;

	// TEMP
	//pCI->dwFlags |= COLLISIONFLAG_USEWALLRADIUS;

	ASSERT(pThis);
	ASSERT(pCI);

	pRegionSet = CScene__GetRegionAttributes(&GetApp()->m_Scene, pThis->ih.m_pCurrentRegion);

	// clear collision results
	CCollisionInfo__ClearCollision(pCI);

	// initialize CCollide variables for entire collision operation
	cd.pInst = pThis;
	cd.pCI = pCI;
	cd.vDesired = vDesired;
	CCollide__InitSlideSurface(&cd);

	// initialize collision variables
	cIteration = 0;
	cCollision = 0;
	remainT = 1.0;
	
	dt = FRAME_INC_TO_SECONDS(frame_increment);

	if (pCI->dwFlags & COLLISIONFLAG_TWOREDIRECTIONS)
	{
		// allow 2 redirections
		nMaxCollisions = 3;
	}
	else
	{
		// allow 1 redirection
		nMaxCollisions = 2;
	}

	cd.UsePhysics = (pCI->dwFlags & COLLISIONFLAG_PHYSICS) && (dt != 0.0);

	if (cd.UsePhysics)
	{
		startYPos = pThis->ih.m_vPos.y;

		// friction
		friction = pCI->AirFriction;
		if (pCI->WaterFriction != pCI->AirFriction)
		{
			if (pThis->ih.m_pCurrentRegion)
			{
				if (pThis->ih.m_pCurrentRegion->m_wFlags & REGFLAG_HASWATER)
				{
					if (pRegionSet && (pThis->ih.m_vPos.y < pRegionSet->m_WaterElevation))
						friction = pCI->WaterFriction;
				}
			}
		}

		friction = min(1.0/dt, 15*friction);

		// acceleration = -friction*(velocity^2) + gravity
		if (friction == 0.0)
		{
			vAcceleration.x =	0.0;
			vAcceleration.y = pCI->GravityAcceleration;
			vAcceleration.z = 0.0;
		}
		else
		{
			vAcceleration.x = -pThis->m_vVelocity.x*friction;
			vAcceleration.y = -pThis->m_vVelocity.y*friction + pCI->GravityAcceleration;
			vAcceleration.z = -pThis->m_vVelocity.z*friction;
		}

		// pos = initialPos + initialVelocity*dt + 0.5*acceleration*(dt^2)
		dtd2 = 0.5*dt;
		cd.vDesired.x += (pThis->m_vVelocity.x + vAcceleration.x*dtd2)*dt;
		cd.vDesired.y += (pThis->m_vVelocity.y + vAcceleration.y*dtd2)*dt;
		cd.vDesired.z += (pThis->m_vVelocity.z + vAcceleration.z*dtd2)*dt;

		// velocity = initialVelocity + acceleration*dt
		pThis->m_vVelocity.x += vAcceleration.x*dt;
		pThis->m_vVelocity.y += vAcceleration.y*dt;
		pThis->m_vVelocity.z += vAcceleration.z*dt;

		pCI->vCollisionVelocity = pThis->m_vVelocity;
	}

	// skip collision if desired is same as current position
	if (CVector3__IsEqual(&pThis->ih.m_vPos, &cd.vDesired))
		return 0;

	//prevGroundBehavior = pCI->GroundBehavior;
	if (pRegionSet)
		cd.StartedInDeathFall = (pRegionSet->m_dwFlags & REGFLAG_FALLDEATH);
	else
		cd.StartedInDeathFall = FALSE;

	//if (pRegionSet && (pRegionSet->m_dwFlags & REGFLAG_FALLDEATH))
	//	pCI->GroundBehavior = INTERSECT_BEHAVIOR_IGNORE;

	cd.AllowRedirection = (cCollision + 1) < nMaxCollisions;

	CCollide__TrackGround(&cd);
	
	if (cd.TrackingGround && cd.UsePhysics)
	{
		CVector3__MultScaler(&pThis->m_vVelocity,
									&pThis->m_vVelocity,
									1.0 - min(1.0, pCI->GroundFriction));
	}

/*
	if ((pThis == &CEngineApp__GetPlayer(GetApp())->ah) && (pCI == &ci_player))
	{
		osSyncPrintf("ducking:%d\n", (pCI->dwFlags & COLLISIONFLAG_DUCKING) != 0);
		//osSyncPrintf("-- collision start -- pInst:%x\n", pThis);
	}
*/
	do
	{
		// initialize CCollide variables for iteration
		CClosestIntersect__Reset(&cd.clint);
		CVector3__Subtract(&cd.vDelta, &cd.vDesired, &pThis->ih.m_vPos);

		if (cd.UsePhysics)
			cd.clint.vNextVelocity = pThis->m_vVelocity;

		// call collision routines
		for (cf=0; pfn_collision_routines[cf]; cf++)
			pfn_collision_routines[cf](&cd);

		switch (cd.clint.cr)
		{
			case CR_COLLISION:
			case CR_COLLISION_NEWDESIREDPOS:
				cCollision++;
			case CR_NO_COLLISION_NEWDESIREDPOS:
				// update current position
				pThis->ih.m_vPos = cd.clint.vIntersect;

				CClosestIntersect__Store(&cd.clint, pThis);

				if (cd.UsePhysics)
					pThis->m_vVelocity = cd.clint.vNextVelocity;

				CCollide__UpdateNextSlideSurface(&cd);

				break;

			case CR_NO_COLLISION:
				pThis->ih.m_vPos = cd.vDesired;
				break;

			default:
				ASSERT(FALSE);
				break;
		}

		if (		(pCI->dwFlags & COLLISIONFLAG_ENDONBOUNCE)
				&& pCI->pWallCollisionRegion )
		{
			return cCollision;
		}

		// t time has passed
		remainT -= cd.clint.t*remainT;

		if (cd.TrackingGround)
		{
			// project position on ground

			if (pThis->ih.m_pCurrentRegion)
				pvCorner = &pThis->ih.m_pCurrentRegion->m_pCorners[0]->m_vCorner;
			else
				pvCorner = &vZero;

			vGroundUnitNormal = CGameRegion__GetGroundUnitNormal(pThis->ih.m_pCurrentRegion);

			CVector3__Subtract(&vCornerDelta, &pThis->ih.m_vPos, pvCorner);

			vProjected = CVector3__Project(&vCornerDelta, &vGroundUnitNormal);

			CVector3__Add(&pThis->ih.m_vPos, pvCorner, &vProjected);
		}

		newDesiredPos =	 (cd.clint.cr == CR_NO_COLLISION_NEWDESIREDPOS)
							 || (cd.clint.cr == CR_COLLISION_NEWDESIREDPOS);

		if (newDesiredPos)
		{
			// update desired position for next iteration
			cd.vDesired = cd.clint.vNextDesired;

			cd.AllowRedirection = (cCollision + 1) < nMaxCollisions;

			CCollide__TrackGround(&cd);
		}

		cIteration++;

	} while (	newDesiredPos
				&& (cIteration < COLLISION_MAX_ITERATIONS)
				&& (cCollision < nMaxCollisions) );


	// post collision stuff
	if (		(CCollide__GetGroundBehavior(&cd, pThis->ih.m_pCurrentRegion) != INTERSECT_BEHAVIOR_IGNORE)
			&& (pCI->WallBehavior != INTERSECT_BEHAVIOR_IGNORE)
			&& cd.UsePhysics
			&& (remainT > 0.01) )
	{
		// didn't finish motion

		// should instance slide height for remaining motion?
		if (cd.clint.pvIntersectPos == &pCI->vInstanceCollisionPos)
			slideHeight = (pCI->InstanceBehavior != INTERSECT_BEHAVIOR_STICK);
		else if (cd.clint.pvIntersectPos == &pCI->vWallCollisionPos)
			slideHeight = (pCI->WallBehavior != INTERSECT_BEHAVIOR_STICK);
		else
			slideHeight = FALSE;
		
		groundHeight = CInstanceHdr__GetGroundHeight(&pThis->ih);
		pThis->ih.m_vPos.y = max((slideHeight ? cd.vDesired.y : pThis->ih.m_vPos.y), groundHeight);
	}


/*
	if (pCI->GroundBehavior != INTERSECT_BEHAVIOR_IGNORE)
	{
		// keep above ground
		groundHeight = CInstanceHdr__GetGroundHeight(&pThis->ih);
		if (cd.UsePhysics && (remainT > 0.01))
//		if (cd.UsePhysics && (remainT > 0.01) && !cd.TrackingGround)
			pThis->ih.m_vPos.y = max(cd.vDesired.y, groundHeight);
		else
			pThis->ih.m_vPos.y = max(pThis->ih.m_vPos.y, groundHeight);

		// ceiling?
	}
*/
	// set y velocity for ramps
	if (		cd.TrackingGround
			&& cd.UsePhysics
			&& (dt != 0.0)
			&& (pCI->GravityAcceleration != 0.0) )
	{
		if ((pThis->ih.m_vPos.y - startYPos) < (20.0*SCALING_FACTOR))
			pThis->m_vVelocity.y = (pThis->ih.m_vPos.y - startYPos)/dt;
	}

	//CVector3__Clamp(&pThis->ih.m_vPos, WORLD_LIMIT);

	//pCI->GroundBehavior = prevGroundBehavior;

/*
	if (pThis == &CEngineApp__GetPlayer(GetApp())->ah)
	{
		osSyncPrintf("-- collision done -- its:%d, cols:%d, pos(%f,%f,%f)\n\n",
			cIteration,
			cCollision,
			pThis->ih.m_vPos.x,
			pThis->ih.m_vPos.y,
			pThis->ih.m_vPos.z);
	}
*/
	return cCollision;
}

// CClosestIntersect
/////////////////////////////////////////////////////////////////////////////

void CClosestIntersect__Reset(CClosestIntersect *pThis)
{
	pThis->t						= 1.0;
	pThis->cr					= CR_NO_COLLISION;
	pThis->pvIntersectPos	= NULL;
	pThis->ppRegion			= NULL;
	pThis->pCollType			= NULL;
	pThis->ppInst				= NULL;
}

void CClosestIntersect__Store(CClosestIntersect *pThis, CAnimInstanceHdr *pInst)
{
	// store collision position
	if (pThis->pvIntersectPos)
		*pThis->pvIntersectPos = pInst->ih.m_vPos;

	// store collision region
	if (pThis->ppRegion)
		*pThis->ppRegion = pInst->ih.m_pCurrentRegion;

	// store collision type
	if (pThis->pCollType)
		*pThis->pCollType = pThis->CollType;

	// store instance collision
	if (pThis->ppInst)
		*pThis->ppInst = pThis->pInstType;
}

// CCollide
/////////////////////////////////////////////////////////////////////////////
// This routine takes care of all collision housekeeping.  Using it in
// other collision routines will make life a lot easier.  It uses an infinite
// plane to detect collision.  Where this is not appropriate, you should
// detect collision before calling this routine.  Also, depending on the
// nature of the collision routine, sometimes you can eliminate collisions
// faster than this.

#define INTERSECT_BACKOFF_DIST	(0.02*SCALING_FACTOR)
//#define INTERSECT_BACKOFF_DIST	(0)

BOOL CCollide__IntersectPlane(CCollide *pThis,
										CVector3 *pvNormal, CVector3 *pvPointOnPlane,
										BOOL BackCull,
										int Behavior,
										float SurfaceFriction)
{
	float			normalDotMovementDelta,
					normalDotPointDelta,
					t,
					magNormal;
	CVector3		vPointDelta, vUnitNormal,
					vRemainDelta,
					vReflectDelta, vProjectDelta,
					vProject, vRedirect,
					vBackOffFromEdge;

	if (Behavior == INTERSECT_BEHAVIOR_IGNORE)
		return FALSE;

	// don't intersect with surface that it's sliding on
	if (CCollide__IsSlidePlane(pThis, pvNormal))
		return FALSE;

	normalDotMovementDelta = CVector3__Dot(pvNormal, &pThis->vDelta);
	if (		(normalDotMovementDelta == 0.0)
			||	(BackCull && (normalDotMovementDelta > 0.0)) )
	{
		return FALSE;
	}

	CVector3__Subtract(&vPointDelta, pvPointOnPlane, &pThis->pInst->ih.m_vPos);
	normalDotPointDelta = CVector3__Dot(pvNormal, &vPointDelta);

	t = normalDotPointDelta/normalDotMovementDelta;
	if (t >= pThis->clint.t)
	{
		// this is not the closest intersection
		return FALSE;
	}

	if (t < -0.2)
		t = -0.2;

//	if (		(!pThis->AllowRedirection && (t < 0.0))
//			||	(t < -0.1) )
//	{
//		t = 0.0;
//	}

	pThis->clint.t = t;
	CVector3__Blend(&pThis->clint.vIntersect, t,
						 &pThis->pInst->ih.m_vPos, &pThis->vDesired);

	if (Behavior == INTERSECT_BEHAVIOR_SLIDEBOUNCE)
	{
		if (pThis->TrackingGround)
			Behavior = INTERSECT_BEHAVIOR_SLIDE;
		else
			Behavior = INTERSECT_BEHAVIOR_BOUNCE;
	}

	// roll
	if ((Behavior == INTERSECT_BEHAVIOR_BOUNCE) && (pThis->pCI->BounceReturnEnergy < 1.0))
	{
		if (CVector3__MagnitudeSquared(&pThis->vDelta) < SQR(0.4*SCALING_FACTOR))
		{
			if ((SQR(pThis->vDelta.x) + SQR(pThis->vDelta.z)) < SQR(0.1*SCALING_FACTOR))
				Behavior = INTERSECT_BEHAVIOR_STICK;
			else
				Behavior = INTERSECT_BEHAVIOR_SLIDE;
		}
	}

	switch (Behavior)
	{
		case INTERSECT_BEHAVIOR_ITERATE:

			CCollide__ClearSlideSurface(pThis);

			pThis->clint.cr = CR_NO_COLLISION_NEWDESIREDPOS;

			if (pThis->UsePhysics)
				pThis->clint.vNextVelocity = pThis->pInst->m_vVelocity;

			pThis->clint.vNextDesired = pThis->vDesired;

			break;

		case INTERSECT_BEHAVIOR_STICK:

			CCollide__ClearSlideSurface(pThis);

			pThis->clint.cr = CR_COLLISION;

			if (pThis->UsePhysics)
			{
				// clear velocity
				pThis->clint.vNextVelocity.x
						= pThis->clint.vNextVelocity.y
						= pThis->clint.vNextVelocity.z
						= 0.0;
			}
			break;

		case INTERSECT_BEHAVIOR_BOUNCE:

			CCollide__ClearSlideSurface(pThis);

			if (pThis->UsePhysics || pThis->AllowRedirection)
			{
				magNormal = CVector3__Magnitude(pvNormal);
				if (magNormal == 0.0)
					return FALSE;

				CVector3__MultScaler(&vUnitNormal, pvNormal, 1.0/magNormal);
			}

			if (pThis->UsePhysics)
			{
				// reflect velocity
				pThis->clint.vNextVelocity = CVector3__Reflect(&pThis->pInst->m_vVelocity,
																			  &vUnitNormal,
																			  pThis->pCI->BounceReturnEnergy);
				CVector3__MultScaler(&pThis->clint.vNextVelocity,
											&pThis->clint.vNextVelocity,
											1.0 - SurfaceFriction);
			}

			if (pThis->AllowRedirection)
			{
				pThis->clint.cr = CR_COLLISION_NEWDESIREDPOS;

				// reflect remaining motion
				CVector3__MultScaler(&vRemainDelta, &pThis->vDelta, 1.0 - t);

				vReflectDelta = CVector3__Reflect(&vRemainDelta,
															 &vUnitNormal,
															 pThis->pCI->BounceReturnEnergy);
				CVector3__MultScaler(&vReflectDelta,
											&vReflectDelta,
											1.0 - SurfaceFriction);

				CVector3__Add(&pThis->clint.vNextDesired,
								  &pThis->clint.vIntersect, &vReflectDelta);
			}
			else
			{
				pThis->clint.cr = CR_COLLISION;
			}
			break;

		case INTERSECT_BEHAVIOR_SLIDE:

			CCollide__SetSlidePlane(pThis, pvNormal);

			if (pThis->UsePhysics || pThis->AllowRedirection)
			{
				magNormal = CVector3__Magnitude(pvNormal);
				if (magNormal == 0.0)
					return FALSE;

				CVector3__MultScaler(&vUnitNormal, pvNormal, 1.0/magNormal);
			}

			if (pThis->UsePhysics)
			{
				// project velocity on plane
				pThis->clint.vNextVelocity = CVector3__Project(&pThis->pInst->m_vVelocity,
																			  &vUnitNormal);

				CVector3__MultScaler(&pThis->clint.vNextVelocity,
											&pThis->clint.vNextVelocity,
											1.0 - SurfaceFriction);
			}

			if (pThis->AllowRedirection)
			{
				pThis->clint.cr = CR_COLLISION_NEWDESIREDPOS;

				// project remaining motion
				CVector3__MultScaler(&vRemainDelta, &pThis->vDelta, 1.0 - t);

				vProjectDelta = CVector3__Project(&vRemainDelta, &vUnitNormal);
				CVector3__MultScaler(&vProjectDelta,
											&vProjectDelta,
											1.0 - SurfaceFriction);

				CVector3__Add(&pThis->clint.vNextDesired,
								  &pThis->clint.vIntersect, &vProjectDelta);

				CVector3__MultScaler(&vBackOffFromEdge, &vUnitNormal, INTERSECT_BACKOFF_DIST);
				CVector3__Add(&pThis->clint.vNextDesired, &pThis->clint.vNextDesired, &vBackOffFromEdge);
			}
			else
			{
				pThis->clint.cr = CR_COLLISION;
			}
			break;

		case INTERSECT_BEHAVIOR_REDIRECT:

			CCollide__SetSlidePlane(pThis, pvNormal);

			if (pThis->UsePhysics || pThis->AllowRedirection)
			{
				magNormal = CVector3__Magnitude(pvNormal);
				if (magNormal == 0.0)
					return FALSE;

				CVector3__MultScaler(&vUnitNormal, pvNormal, 1.0/magNormal);
			}

			if (pThis->UsePhysics)
			{
				// project velocity on plane
				pThis->clint.vNextVelocity = CVector3__Redirect(&pThis->pInst->m_vVelocity,
																				&vUnitNormal);
			}

			if (pThis->AllowRedirection)
			{
				pThis->clint.cr = CR_COLLISION_NEWDESIREDPOS;

				// redirect remaining motion
				CVector3__MultScaler(&vRemainDelta, &pThis->vDelta, 1.0 - t);

				vProjectDelta = CVector3__Redirect(&vRemainDelta, &vUnitNormal);

				CVector3__Add(&pThis->clint.vNextDesired,
								  &pThis->clint.vIntersect, &vProjectDelta);

				CVector3__MultScaler(&vBackOffFromEdge, &vUnitNormal, INTERSECT_BACKOFF_DIST);
				CVector3__Add(&pThis->clint.vNextDesired, &pThis->clint.vNextDesired, &vBackOffFromEdge);
			}
			else
			{
				pThis->clint.cr = CR_COLLISION;
			}
			break;

		case INTERSECT_BEHAVIOR_GREASE:

			CCollide__SetSlidePlane(pThis, pvNormal);

			if (pThis->UsePhysics || pThis->AllowRedirection)
			{
				magNormal = CVector3__Magnitude(pvNormal);
				if (magNormal == 0.0)
					return FALSE;

				CVector3__MultScaler(&vUnitNormal, pvNormal, 1.0/magNormal);
			}

			if (pThis->UsePhysics)
			{
				// project velocity on plane
				vProject = CVector3__Project(&pThis->pInst->m_vVelocity, &vUnitNormal);
				vRedirect = CVector3__Redirect(&pThis->pInst->m_vVelocity, &vUnitNormal);

				CVector3__Blend(&pThis->clint.vNextVelocity,
									 COLLISION_GREASE_FACTOR,
									 &vProject, &vRedirect);
				CVector3__MultScaler(&pThis->clint.vNextVelocity,
											&pThis->clint.vNextVelocity,
											1.0 - SurfaceFriction);
			}

			if (pThis->AllowRedirection)
			{
				pThis->clint.cr = CR_COLLISION_NEWDESIREDPOS;

				// project remaining motion
				CVector3__MultScaler(&vRemainDelta, &pThis->vDelta, 1.0 - t);


				vProject = CVector3__Project(&vRemainDelta, &vUnitNormal);
				vRedirect = CVector3__Redirect(&vRemainDelta, &vUnitNormal);

				CVector3__Blend(&vProjectDelta,
									 COLLISION_GREASE_FACTOR,
									 &vProject, &vRedirect);
				CVector3__MultScaler(&vProjectDelta,
											&vProjectDelta,
											1.0 - SurfaceFriction);


				CVector3__Add(&pThis->clint.vNextDesired,
								  &pThis->clint.vIntersect, &vProjectDelta);

				CVector3__MultScaler(&vBackOffFromEdge, &vUnitNormal, INTERSECT_BACKOFF_DIST);
				CVector3__Add(&pThis->clint.vNextDesired, &pThis->clint.vNextDesired, &vBackOffFromEdge);
			}
			else
			{
				pThis->clint.cr = CR_COLLISION;
			}
			break;

		default:
			ASSERT(FALSE);
			break;
	}

	return TRUE;
}

BOOL CCollide__IntersectCylinder(CCollide *pThis,
											CVector3 *pvCenter, float Radius,
											BOOL UseHeight, float Top, float Bottom,
											BOOL BackCull,
											int Behavior,
											float SurfaceFriction)
{
	CVector3		h, M,
					hminusmhDotM,
					vPoint, vNormal;
	float			hDotDelta,
					magDelta,
					invMagDelta,
					hdotM,
					magSqRight, radSq,
					sub,
					t;

	if (CCollide__IsSlideCylinder(pThis, pvCenter, Radius))
		return FALSE;

	h.x = pvCenter->x - pThis->pInst->ih.m_vPos.x;
	h.z = pvCenter->z - pThis->pInst->ih.m_vPos.z;

	if (BackCull)
	{
		hDotDelta = h.x*pThis->vDelta.x + h.z*pThis->vDelta.z;
		if (hDotDelta < 0.0)
			return FALSE;
	}

	magDelta = sqrt(SQR(pThis->vDelta.x) + SQR(pThis->vDelta.z));
	if (magDelta != 0.0)
	{
		invMagDelta = 1.0/magDelta;
		M.x = pThis->vDelta.x*invMagDelta;
		M.z = pThis->vDelta.z*invMagDelta;

		hdotM = h.x*M.x + h.z*M.z;

		hminusmhDotM.x = h.x - M.x*hdotM;
		hminusmhDotM.z = h.z - M.z*hdotM;

		magSqRight = SQR(hminusmhDotM.x) + SQR(hminusmhDotM.z);
		radSq = SQR(Radius);

		sub = radSq - magSqRight;
		if (sub > 0.0)
		{
			// movement did cross cylinder

			//t = max(0.0, (hdotM - sqrt(sub))*invMagDelta);
			t = (hdotM - sqrt(sub))*invMagDelta;
			if (t < pThis->clint.t)
			{
				CVector3__Blend(&vPoint, t, &pThis->pInst->ih.m_vPos, &pThis->vDesired);

				if (UseHeight)
					if ((vPoint.y > Top) || (vPoint.y < Bottom))
						return FALSE;

				vNormal.x = vPoint.x - pvCenter->x;
				vNormal.y = 0.0;
				vNormal.z = vPoint.z - pvCenter->z;
				
				if (CCollide__IntersectPlane(pThis,
													  &vNormal, &vPoint,
													  FALSE,
													  Behavior,
													  SurfaceFriction))
				{
					CCollide__SetSlideCylinder(pThis, pvCenter, Radius);

					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

CGameRegion* CCollide__CanEnter(CCollide *pThis, CGameRegion *pCurrent, int nDesiredEdge, CVector3 *pvComparePos)
{
	CGameRegion			*pDesired;
	CAnimInstanceHdr	*pInst;
	float					currentHeight,
							desiredHeight,
							edgeAngle, dt,
							compareHeight,
							dot;
	int					cEdge;
	BOOL					found,
							moveUpLedges, moveDownLedges,
							currentIsCliff, desiredIsCliff;
	CVector3				vNextLower,
							vCeilingNormal, vDelta;

	ASSERT(pCurrent);

	pInst = pThis->pInst;
	
	pDesired = pCurrent->m_pNeighbors[nDesiredEdge];


	// TEMP
	//return pDesired;


	if (!pDesired)
		return NULL;

	if (pCurrent->m_wFlags & REGFLAG_BRIDGE)
	{
		// if pDesired is linked back to pCurrent, this is a normal
		// transition -- otherwise, the instance went off the edge of
		// a bridge

		found = FALSE;
		for (cEdge=0; cEdge<3; cEdge++)
		{
			if (pDesired->m_pNeighbors[cEdge] == pCurrent)
			{
				found = TRUE;
				break;
			}
		}

		if (!found)
		{
			// this is a bridge transition
			// first, see if pvCurrent is within pDesired

			if (!CGameRegion__InRegion(pDesired, pvComparePos->x, pvComparePos->z))
			{
				// missed with guess
				// search all regions

				vNextLower.x = pvComparePos->x;
				vNextLower.y = pInst->ih.m_vPos.y;
				vNextLower.z = pvComparePos->z;

				pDesired = CGameRegion__FindNextLowerRegion(pCurrent, vNextLower);
				if (!pDesired)
					return NULL;
			}
		}
	}

	// WHY SPECIAL CASE THIS -- WHAT'S WRONG WITH RESTRICTED AREAS?
	// Used to keep TREX out of big life area
	if (		(pThis->pCI->dwFlags & COLLISIONFLAG_AVOIDPRESSUREPLATE)
			&& (		(pDesired->m_wFlags & REGFLAG_PRESSUREPLATE)
					||	(pDesired->m_wFlags & REGFLAG_PRESSUREPLATENEEDSCOLLISION) ) )
	{
		return NULL;
	}

	// keep bad guys out of trouble
	if (		(pDesired->m_wFlags & REGFLAG_RESTRICTEDAREA)
			&& !(pThis->pCI->dwFlags & COLLISIONFLAG_CANENTERRESTRICTEDAREA) )
	{
		return NULL;
	}

	// keep in water
	if (		!(pThis->pCI->dwFlags & COLLISIONFLAG_EXITWATER)
			&& !(pDesired->m_wFlags & REGFLAG_HASWATER)
			&& (pCurrent->m_wFlags & REGFLAG_HASWATER) )
	{
		return NULL;
	}

	// avoid water
	if (		!(pThis->pCI->dwFlags & COLLISIONFLAG_ENTERWATER)
			&& (pDesired->m_wFlags & REGFLAG_HASWATER)
			&& !(pCurrent->m_wFlags & REGFLAG_HASWATER) )
	{
		return NULL;
	}

	// need to be ducking to enter ducking area
	if (		(pDesired->m_wFlags & REGFLAG_DUCK)
			&&	!(pThis->pCI->dwFlags & COLLISIONFLAG_DUCKING) )
	{
		return NULL;
	}

	// can't enter closed door
	if (		!(pThis->pCI->dwFlags & COLLISIONFLAG_MOVETHROUGHDOORS)
			&&	(pDesired->m_wFlags & REGFLAG_DOOR)
			&& !(pDesired->m_wFlags & REGFLAG_OPENDOOR) )
	{
		return NULL;
	}


	// bump in to wall above ceiling
/*
	if ((pDesired->m_wFlags & REGFLAG_CEILING) && !(pCurrent->m_wFlags & REGFLAG_CEILING))
	{
		if (pInst->ih.m_vPos.y > CGameRegion__GetCeilingHeight(pDesired, pInst->ih.m_vPos.x, pInst->ih.m_vPos.z))
		{
			// only hit front side of ceiling
			vCeilingNormal = CGameRegion__GetCeilingNormal(pDesired);
			
			vDelta.x = pInst->ih.m_vPos.x - pDesired->m_pCorners[0]->m_vCorner.x;
			vDelta.y = pInst->ih.m_vPos.y - pDesired->m_pCorners[0]->m_Ceiling;
			vDelta.z = pInst->ih.m_vPos.z - pDesired->m_pCorners[0]->m_vCorner.z;

			dot = CVector3__Dot(&vCeilingNormal, &vDelta);
			if (dot >= 0.0)
				return NULL;
		}
	}
*/
	if ((pDesired->m_wFlags & REGFLAG_CEILING) && !(pCurrent->m_wFlags & REGFLAG_CEILING))
		if (pvComparePos->y > CGameRegion__GetCeilingHeight(pDesired, pvComparePos->x, pvComparePos->z))
			return NULL;

	currentIsCliff = CGameRegion__IsCliff(pCurrent);
	desiredIsCliff = CGameRegion__IsCliff(pDesired);
	
	if (pThis->pCI->dwFlags & COLLISIONFLAG_CLIFFONLY)
	{
		if (desiredIsCliff)
			return pDesired;
		else
			return NULL;
	}

	// stay on climbable portion of cliff
	if (		(pThis->pCI->dwFlags & COLLISIONFLAG_CANCLIMBCLIMBABLE)
			&& currentIsCliff
			&& desiredIsCliff
			&& (pCurrent->m_wFlags & REGFLAG_CLIMBABLE)
			&& !(pDesired->m_wFlags & REGFLAG_CLIMBABLE) )
	{
		currentHeight = CGameRegion__GetGroundHeight(pCurrent, pInst->ih.m_vPos.x, pInst->ih.m_vPos.z);
		if (pInst->ih.m_vPos.y < (currentHeight + 0.1*SCALING_FACTOR))
			return NULL;
	}

	if (desiredIsCliff && !currentIsCliff)
	{
		moveUpLedges	= pThis->pCI->dwFlags & COLLISIONFLAG_CLIMBUP;
		moveDownLedges	= pThis->pCI->dwFlags & COLLISIONFLAG_CLIMBDOWN;
		
		if ((pDesired->m_wFlags & REGFLAG_CLIMBABLE) && (pThis->pCI->dwFlags & COLLISIONFLAG_CANCLIMBCLIMBABLE))
		{
			// test angle
			edgeAngle = CGameRegion__GetEdgeAngle(pCurrent, nDesiredEdge);
			edgeAngle += ANGLE_PI;
			NormalizeRotation(&edgeAngle);

			// COLLISIONFLAG_CANCLIMBCLIMBABLE should only be set for CGameObjectInstance's
			ASSERT(pInst->ih.m_Type == I_ANIMATED);

			dt = RotDifference(edgeAngle, ((CGameObjectInstance*)pInst)->m_RotY);

#define ALLOW_CLIMB_ANGLE	ANGLE_DTOR(40)

			if (fabs(dt) <= ALLOW_CLIMB_ANGLE)
				moveUpLedges = TRUE;
		}

		if (moveUpLedges && moveDownLedges)
			return pDesired;

		currentHeight = CGameRegion__GetGroundHeight(pCurrent, pInst->ih.m_vPos.x, pInst->ih.m_vPos.z);
		desiredHeight = CGameRegion__GetGroundHeight(pDesired, pThis->vDesired.x, pThis->vDesired.z);

		if (desiredHeight > currentHeight)
		{
			if (moveUpLedges)
			{
				return pDesired;
			}
			else
			{
#define COMPARE_HEIGHT_RANGE	(0.1*SCALING_FACTOR)
//#define COMPARE_HEIGHT_RANGE	(1*SCALING_FACTOR)
				compareHeight = pInst->ih.m_vPos.y + COMPARE_HEIGHT_RANGE;
				//compareHeight = pInst->ih.m_vPos.y;

				// change to intersection?

				if (		(compareHeight > pDesired->m_pCorners[0]->m_vCorner.y)
						&&	(compareHeight > pDesired->m_pCorners[1]->m_vCorner.y)
						&& (compareHeight > pDesired->m_pCorners[2]->m_vCorner.y) )
				{
					return pDesired;
				}
				else
				{
					return NULL;
				}
			}
		}
		else
		{
			return moveDownLedges ? pDesired : NULL;
		}
	}
	else
	{
		return pDesired;
	}
}

int CCollide__GetGroundBehavior(CCollide *pThis, CGameRegion *pCurrentRegion)
{
	CROMRegionSet		*pRegionSet;

	pRegionSet = CScene__GetRegionAttributes(&GetApp()->m_Scene, pCurrentRegion);
	if (pRegionSet && (pRegionSet->m_dwFlags & REGFLAG_FALLDEATH))
		return INTERSECT_BEHAVIOR_IGNORE;
	else
		return pThis->pCI->GroundBehavior;
}

void CCollide__TrackGround(CCollide *pThis)
{
	CAnimInstanceHdr	*pInst;
	float					currentGroundHeight, desiredGroundHeight,
							dx, dy, dz,
							groundMagSq, magSq,
							moveFactor,
							dot,
							sinTheta, cosTheta;
	CVector3				vNormal, vDelta;
	int					groundBehavior;

	pInst = pThis->pInst;

	if (pThis->StartedInDeathFall)
		groundBehavior = CCollide__GetGroundBehavior(pThis, pInst->ih.m_pCurrentRegion);
	else
		groundBehavior = pThis->pCI->GroundBehavior;

	if (		!(pThis->pCI->dwFlags & COLLISIONFLAG_TRACKGROUND)
			||	(groundBehavior == INTERSECT_BEHAVIOR_IGNORE) )
	{
		pThis->TrackingGround = FALSE;
		return;
	}
	
	vNormal = CGameRegion__GetGroundNormal(pInst->ih.m_pCurrentRegion);
	
	if (		CGameRegion__IsCliff(pInst->ih.m_pCurrentRegion)
			&& !(pThis->pCI->dwFlags & COLLISIONFLAG_CLIMBUP) )
	{
		if (		(pInst->ih.m_pCurrentRegion->m_wFlags & REGFLAG_CLIMBABLE)
				&& (pThis->pCI->dwFlags & COLLISIONFLAG_CANCLIMBCLIMBABLE) )
		{
			// set to FALSE if player isn't facing cliff
			
			if (pInst->ih.m_Type == I_ANIMATED)
			{
				sinTheta = sin(((CGameObjectInstance*)pInst)->m_RotY);
				cosTheta = cos(((CGameObjectInstance*)pInst)->m_RotY);

				dot = vNormal.x*(-sinTheta) + vNormal.z*(-cosTheta);
				if (dot >= 0.0)
				{
					pThis->TrackingGround = FALSE;
					return;
				}
			}				  
		}
		else
		{
			pThis->TrackingGround = FALSE;

/*				
			if (pThis->AllowRedirection)
			{
				desiredGroundHeight = CGameRegion__GetGroundHeight(pInst->ih.m_pCurrentRegion, pThis->vDesired.x, pThis->vDesired.z);
		
#define ZERO_XZ_THRESHOLD	(0.5*SCALING_FACTOR)
				if ((pThis->vDesired.y - desiredGroundHeight) < ZERO_XZ_THRESHOLD)
				{
					// fall straight down the cliff
					pThis->vDesired.x = pInst->ih.m_vPos.x;
					pThis->vDesired.z = pInst->ih.m_vPos.z;
				}
			}
*/
/*
			if (pThis->AllowRedirection)
			{
				CVector3__Subtract(&vDelta,
										 &pThis->vDesired,
										 &pInst->ih.m_pCurrentRegion->m_pCorners[0]->m_vCorner);

				dot = CVector3__Dot(&vNormal, &vDelta);
				if (dot <= 0.0)
				{
					// fall down the cliff
					pThis->vDesired.x = pInst->ih.m_vPos.x;
					pThis->vDesired.z = pInst->ih.m_vPos.z;
				}

				if (dot <= 0.0)
				{
					sprintf(string, "y %.1f v %.1f",
							pThis->vDesired.y - pInst->ih.m_vPos.y,
							pInst->m_vVelocity.y);
					COnScreen__AddGameTextWithId(string, 1023);
				}
			}
*/
			return;
		}
	}

	currentGroundHeight = CGameRegion__GetGroundHeight(pInst->ih.m_pCurrentRegion, pInst->ih.m_vPos.x, pInst->ih.m_vPos.z);
	desiredGroundHeight = CGameRegion__GetGroundHeight(pInst->ih.m_pCurrentRegion, pThis->vDesired.x, pThis->vDesired.z);

//#define TRACKGROUND_THRESHOLD (0.1*SCALING_FACTOR)
	//pThis->TrackingGround =		((pInst->ih.m_vPos.y - currentGroundHeight) < TRACKGROUND_THRESHOLD)
	//								&& ((pThis->vDesired.y - desiredGroundHeight) < TRACKGROUND_THRESHOLD);

	if (pInst->ih.m_pCurrentRegion)
	{
		CVector3__Subtract(&vDelta,
								 &pThis->vDesired,
								 &pInst->ih.m_pCurrentRegion->m_pCorners[0]->m_vCorner);
	}
	else
	{
		vDelta = pThis->vDesired;
	}

	dot = CVector3__Dot(&vNormal, &vDelta);
	pThis->TrackingGround = (dot <= 0.0);

//	osSyncPrintf("dot:%f norm(%f,%f,%f) delt(%f,%f,%f)\n",
//		dot,
//		vNormal.x,vNormal.y,vNormal.z,
//		vDelta.x,vDelta.y,vDelta.z);

	// TEMP
	//pThis->TrackingGround =	TRUE;
	//pThis->TrackingGround =	FALSE;

//	if (pThis->pInst == &CEngineApp__GetPlayer(GetApp())->ah)
//		osSyncPrintf("tg:%d ", pThis->TrackingGround);

	if (pThis->TrackingGround)
	{
		//pInst->ih.m_vPos.y = currentGroundHeight;
		//pThis->vDesired.y = desiredGroundHeight;

		dx = pThis->vDesired.x - pInst->ih.m_vPos.x;
		dy = desiredGroundHeight - currentGroundHeight;
		dz = pThis->vDesired.z - pInst->ih.m_vPos.z;

		groundMagSq = dx*dx + dz*dz;
		magSq = groundMagSq + dy*dy;
		
		if (magSq == 0.0)
		{
			pThis->vDesired.y = currentGroundHeight;
			return;
		}
//		if (magSq <= SQR(0.1*SCALING_FACTOR))
//		{
//			pThis->TrackingGround = FALSE;
//			return;
//		}

		// don't move as far
		moveFactor = sqrt(groundMagSq/magSq);

//		CVector3__Blend(&pThis->vDesired, moveFactor, &pInst->ih.m_vPos, &pThis->vDesired);

		pThis->vDesired.x = pInst->ih.m_vPos.x + moveFactor*(pThis->vDesired.x - pInst->ih.m_vPos.x);
		pThis->vDesired.y = currentGroundHeight + moveFactor*dy;
		pThis->vDesired.z = pInst->ih.m_vPos.z + moveFactor*(pThis->vDesired.z - pInst->ih.m_vPos.z);


//		if (pThis->pInst == &CEngineApp__GetPlayer(GetApp())->ah)
//			osSyncPrintf("current:%f, desired:%f ", currentGroundHeight, desiredGroundHeight);
	}

//	if (pThis->pInst == &CEngineApp__GetPlayer(GetApp())->ah)
//		osSyncPrintf("\n");
}
