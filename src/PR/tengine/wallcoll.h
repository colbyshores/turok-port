#ifndef __WALLCOLL_H__
#define __WALLCOLL_H__

#include "graphu64.h"

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
//void CLine__CalculateDirection(CLine *pThis) ;
BOOL CLine__IntersectsLine(CLine *pThis, CLine *pLine, FLOAT *pt) ;

#endif	// __WALLCOLL_H__

