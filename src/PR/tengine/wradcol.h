// wradcol.h

#ifndef _INC_WRADCOL
#define _INC_WRADCOL

#include "unicol.h"

/////////////////////////////////////////////////////////////////////////////

typedef struct CWallRadiusCollision2_t
{
	CCollide		*pCollide;
	CVector3		*pvCornerA,
					*pvCornerB,
					vNormal,
					vMoveDelta,
					vPointDelta,
					vPoint,
					vEdgeNormal,
					vEdgeDelta;
	float			Radius,
					Mag,
					SideDot,
					NormalDotMovementDelta,
					NormalDotPointDelta,
					t;
	CGameRegion	*pNeighbor;
	BOOL			Intersected;
	CBoundsRect	Bounds;
} CWallRadiusCollision2;

// CWallRadiusCollision2 operations
/////////////////////////////////////////////////////////////////////////////

void		CCollide__WallRadiusCollision2(CCollide *pThis);

/////////////////////////////////////////////////////////////////////////////
#endif // _INC_WRADCOL
