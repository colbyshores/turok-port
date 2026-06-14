// unicol.h

#ifndef _INC_UNICOL
#define _INC_UNICOL

#include "graphu64.h"
#include "behavior.h"

/////////////////////////////////////////////////////////////////////////////

// forward declarations
struct CAnimInstanceHdr_t;
struct CCollisionInfo2_t;

/////////////////////////////////////////////////////////////////////////////

typedef enum ECollisionResult_t
{
	CR_NO_COLLISION,
	CR_NO_COLLISION_NEWDESIREDPOS,
	CR_COLLISION,
	CR_COLLISION_NEWDESIREDPOS
} ECollisionResult;

/////////////////////////////////////////////////////////////////////////////

typedef struct CClosestIntersect_t
{
	float					t;						// closest intersection along path
													//   start + t(desired - start)
	ECollisionResult	cr;					// closest collision result
	CVector3				vNextVelocity,		// velocity for next iteration
							vNextDesired,		// desired position for next iteration
							vIntersect;			// closest collision position

	CVector3				*pvIntersectPos;	// address of pointer to store position
	struct CGameRegion_t	**ppRegion;		// address of pointer to store region
	int					*pCollType,			// address to store collision type
							CollType;			// type to store in pCollisionType
	CInstanceHdr		**ppInst,			// address of pointer to stre instance
							*pInstType;			// pointer to instance
} CClosestIntersect;

// CClosestIntersect operations
/////////////////////////////////////////////////////////////////////////////

void		CClosestIntersect__Reset(CClosestIntersect *pThis);
void		CClosestIntersect__Store(CClosestIntersect *pThis, CAnimInstanceHdr *pInst);
#define	CClosestIntersect__SetStoreDest(pThis, pvDestPos, ppDestRegion, pDestCollType, StoreCollType, ppDestInst, pDestInstType) {\
		(pThis)->pvIntersectPos = (pvDestPos);	\
		(pThis)->ppRegion = (ppDestRegion);		\
		(pThis)->pCollType = (pDestCollType);	\
		(pThis)->CollType = (StoreCollType);	\
		(pThis)->ppInst = (ppDestInst);			\
		(pThis)->pInstType = (pDestInstType); }

/////////////////////////////////////////////////////////////////////////////

typedef struct CCollide_t
{
	struct CAnimInstanceHdr_t	*pInst;	// instance for collision calculation
	struct CCollisionInfo2_t	*pCI;				// contains input behavior and
													//   collision results
	CClosestIntersect	clint;				// data for closest intersection

	CVector3				vDesired,			// desired position
							vDelta,				// desired - start
							vSliding,			// normal to surface instance is
													//   sliding on
							vNextSliding;
	float					SlidingRadius,
							NextSlidingRadius;
#define SLIDING_TYPE_NONE		0
#define SLIDING_TYPE_PLANE		1
#define SLIDING_TYPE_CYLINDER	2
	int					SlidingType,
							NextSlidingType;
	BOOL					AllowRedirection,	// collision routine should calculate
													//   vNextDesired if TRUE
							UsePhysics,			// enables gravity, friction, bounces
													//   and slides
							TrackingGround,
							StartedInDeathFall;
} CCollide;

// CCollide operations
/////////////////////////////////////////////////////////////////////////////

BOOL				CCollide__IntersectPlane(CCollide *pThis,
													 CVector3 *pvNormal,
													 CVector3 *pvPointOnPlane,
													 BOOL BackCull,
													 int Behavior,
													 float SurfaceFriction);
BOOL				CCollide__IntersectCylinder(CCollide *pThis,
														 CVector3 *pvCenter, float Radius,
														 BOOL UseHeight, float Top, float Bottom,
														 BOOL BackCull,
														 int Behavior,
														 float SurfaceFriction);
CGameRegion*	CCollide__CanEnter(CCollide *pThis,
											 CGameRegion *pCurrent,
											 int nDesiredEdge,
											 CVector3 *pvComparePos);
void				CCollide__TrackGround(CCollide *pThis);
int				CCollide__GetGroundBehavior(CCollide *pThis, CGameRegion *pCurrentRegion);

#define			CCollide__InitSlideSurface(pThis) (pThis)->SlidingType = SLIDING_TYPE_NONE
#define			CCollide__ClearSlideSurface(pThis) (pThis)->NextSlidingType = SLIDING_TYPE_NONE

#define			CCollide__IsSliding(pThis) ((pThis)->SlidingType != SLIDING_TYPE_NONE)

#define			CCollide__UpdateNextSlideSurface(pThis) {		\
		(pThis)->SlidingType = (pThis)->NextSlidingType;		\
		(pThis)->vSliding = (pThis)->vNextSliding;				\
		(pThis)->SlidingRadius = (pThis)->NextSlidingRadius; }

#define			CCollide__SetSlidePlane(pThis, pvNormal) {	\
		(pThis)->NextSlidingType = SLIDING_TYPE_PLANE;			\
		(pThis)->vNextSliding = *(pvNormal); }
#define			CCollide__IsSlidePlane(pThis, pvNormal) \
		( ((pThis)->SlidingType == SLIDING_TYPE_PLANE) && (CVector3__IsEqual(&(pThis)->vSliding, (pvNormal))) )

#define			CCollide__SetSlideCylinder(pThis, pvCenter, Radius) {	\
		(pThis)->NextSlidingType = SLIDING_TYPE_CYLINDER;					\
		(pThis)->vNextSliding = *(pvCenter);									\
		(pThis)->NextSlidingRadius = (Radius); }
#define			CCollide__IsSlideCylinder(pThis, pvCenter, Radius) \
		( ((pThis)->SlidingType == SLIDING_TYPE_CYLINDER) && ((pThis)->SlidingRadius == (Radius)) && (CVector3__IsEqual(&(pThis)->vSliding, (pvCenter))) )

/////////////////////////////////////////////////////////////////////////////
// ** Universal Collision Routine **
// supports multiple simultanious collision systems
/////////////////////////////////////////////////////////////////////////////

int CAnimInstanceHdr__Collision3(struct CAnimInstanceHdr_t *pThis,
											CVector3 vDesired,
											struct CCollisionInfo2_t *pCI);

///////////////////////////////////////////////////////////////////////////
#endif // _INC_UNICOL
