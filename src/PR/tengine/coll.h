#ifndef __COLL_H__
#define __COLL_H__


#include "defs.h"
#include "stdafx.h"
#include "cppu64.h"
#include "graphu64.h"
#include "tengine.h"
#include "attract.h"

#include "spec.h"
#include "defs.h"
#include "scene.h"
#include "romstruc.h"
#include "onscrn.h"
#include "volume.h"
#include "pause.h"
#include "boss.h"
#include "fx.h"
#include "options.h"
#include <sched.h>
#include "camera.h"


/*****************************************************************************
*	CLine class
*****************************************************************************/

// Structure
typedef struct CLine_t
{
	CVector3	m_vPt[2] ;
	CVector3	m_vDelta ;
	CVector3	m_vDir ;
	CVector3	m_vNormal ;
} CLine ;

void CLine__Construct(CLine *pThis, CVector3 *pvPt1, CVector3 *pvPt2) ;
void CLine__CalculateXZDirection(CLine *pThis) ;
void CLine__CalculateDirection(CLine *pThis) ;
BOOL CLine__IntersectsLine(CLine *pThis, CLine *pLine, FLOAT *pt) ;

/*****************************************************************************
*	CBounds class
*****************************************************************************/

// Structure
typedef struct CBounds_t
{
	CVector3	m_vPt[2] ;
} CBounds ;

BOOL CBounds__OverlapsLine(CBounds *pThis, CLine *pLine) ;
BOOL CBounds__PtInside(CBounds *pThis, CVector3 *pvPt) ;


/*****************************************************************************
*	CCircle class
*****************************************************************************/

// Structure
typedef struct CCircle_t
{
	CVector3	m_vPos ;
	FLOAT		m_Radius ;
	FLOAT		m_RadiusSqr ;
	CBounds		m_Bounds ;
} CCircle ;



/*****************************************************************************
*	CEdge class
*****************************************************************************/

// Structure
typedef struct CEdge_t
{
	CCircle	m_Circle[2] ;
	CLine		m_Line[2] ;
} CEdge ;


/*****************************************************************************
*	CCollision class
*****************************************************************************/

// Structure
typedef struct CCollision_t
{
	BOOL			m_Collision ;		// TRUE if a collision occurred
	BOOL			m_FirstCall ;
	BOOL			m_OnGround ;
	CVector3		m_vVelocity ;

	CLine			m_CollisionLine ;	// Line collided with
	FLOAT			m_t ;					// % along movement

	CLine			m_Line ;				// Total line movement
	CLine			m_HeadLine ;  		// Total line movement for head
	CBounds		m_Bounds ;			// Total movement bounds

	CGameRegion	*m_pFinalRegion ;	// Final region to put instance in
	CVector3		m_vFinalPos ;		// Final position

	FLOAT			m_Radius ;			// Collision radius of instance
	FLOAT			m_Feet ;
	FLOAT			m_Head ;

	// Stats
	INT32			m_RegionsRecursed ;
	INT32			m_CircleIntersections ;
	INT32			m_LineIntersections ;

	// Copy of vars passed into function
	CAnimInstanceHdr	*m_pThis ;
	CGameRegion			**m_ppCurrentRegion ;
	CVector3				*m_pvCurrentPos ;
	CCollisionInfo		*m_pCI ;
	BOOL					m_ClimbUp ; 
	BOOL					m_ClimbDown ; 
	BOOL					m_EnterWater ; 
	BOOL					m_ExitWater ; 
} CCollision ;


/*****************************************************************************
*	Functions
*****************************************************************************/

BOOL CAnimInstanceHdr__Collision2(CAnimInstanceHdr *pThis, CGameRegion **ppCurrentRegion,
											 CVector3 *pvCurrentPos, CVector3 vVelocity,	CCollisionInfo *pCI,
											BOOL ClimbUp, BOOL ClimbDown, BOOL EnterWater, BOOL ExitWater, BOOL FirstCall) ;


#endif	// __COLL_H__

