/////////////////////////////////////////////////////////////////////////////
//
// Simple matrix allocation functions by S.Broumley
//
/////////////////////////////////////////////////////////////////////////////



#include "stdafx.h"
#include "mattable.h"
#include "tengine.h"




/////////////////////////////////////////////////////////////////////////////
// Constants
/////////////////////////////////////////////////////////////////////////////
#ifndef MAKE_CART

#define SHOW_MATTABLE_STATS	0

#else

#define SHOW_MATTABLE_STATS	0	// Don't change this!!

#endif


/////////////////////////////////////////////////////////////////////////////
// Variables
/////////////////////////////////////////////////////////////////////////////
Mtx				MtxPool[MATTABLE_MATRICES * 2] ;
CAnimationMtxs	AnimationMtxs ;

#if SHOW_MATTABLE_STATS
INT32	MaxMtxsRequested ;
INT32	MtxsAllocated ;
#endif


/*****************************************************************************
*
*	Function:		CAnimationMtxs__Construct()
*
******************************************************************************
*
*	Description:	Initailizes vars ready for allocation
*
*	Inputs:			None
*
*	Outputs:			None
*
*****************************************************************************/
void CAnimationMtxs__Construct(void)
{
	int	i ;

	// Set all matrices to identity matrix, just to be valid
	// incase something is pointing to them!!!
	for (i = 0 ; i < (MATTABLE_MATRICES * 2) ; i++)
		guMtxIdent(&MtxPool[i]) ;

	AnimationMtxs.m_MtxsFree = MATTABLE_MATRICES ;
	AnimationMtxs.m_pmFreeMtx = MtxPool ;
	AnimationMtxs.m_MtxsRequested = 0 ;

#if SHOW_MATTABLE_STATS
	MaxMtxsRequested = 0 ;
	MtxsAllocated = 0 ;
#endif
}




/*****************************************************************************
*
*	Function:		CAnimationMtxs__Update()
*
******************************************************************************
*
*	Description:	Initailizes vars ready for allocating next frames matrices
*						NOTE: Call this function once per frame
*
*	Inputs:			None
*
*	Outputs:			None
*
*****************************************************************************/
void CAnimationMtxs__Update(void)
{
	// Goto next matrix pool
	AnimationMtxs.m_MtxsFree = MATTABLE_MATRICES ;

	// Set free mtx to start of pool
	AnimationMtxs.m_pmFreeMtx = &MtxPool[MATTABLE_MATRICES * even_odd] ;

#ifndef MAKE_CART
	// Check to see if too many matrices were requested last frame
	if (AnimationMtxs.m_MtxsRequested > MATTABLE_MATRICES)
	{
		rmonPrintf("\r\n\r\n%d Matrices were requested last frame!!\r\n\r\n", AnimationMtxs.m_MtxsRequested) ;
		ASSERT(FALSE) ;
	}
#endif

	AnimationMtxs.m_MtxsRequested = 0 ;


#if SHOW_MATTABLE_STATS
	rmonPrintf("MtxsAlloc:%d MaxMtxsRequested:%d\r\n", 
				  MtxsAllocated, MaxMtxsRequested) ;
	MtxsAllocated = 0 ;
#endif
}




/*****************************************************************************
*
*	Function:		CAnimationMtxs__RequestMtxTable()
*
******************************************************************************
*
*	Description:	Allocates, if possible, an array of mtxs ready for use.
*
*	Inputs:			Size		- Total number of matrices to allocate
*
*	Outputs:			None
*
*****************************************************************************/
Mtx *CAnimationMtxs__RequestMtxTable(INT32 Size)
{
	Mtx *pMtx = NULL ;

	// Update requested count
	AnimationMtxs.m_MtxsRequested += Size ;

	// Enough space?
	if (AnimationMtxs.m_MtxsFree >= Size)
	{
		pMtx = AnimationMtxs.m_pmFreeMtx ;

		// Update mtx vars
		AnimationMtxs.m_pmFreeMtx += Size ;
		AnimationMtxs.m_MtxsFree -= Size ;

#if SHOW_MATTABLE_STATS
		MtxsAllocated += Size ;
#endif
	}

#if SHOW_MATTABLE_STATS
	if (AnimationMtxs.m_MtxsRequested > MaxMtxsRequested)
		MaxMtxsRequested = AnimationMtxs.m_MtxsRequested ;
#endif


	ASSERT(pMtx != NULL) ;

	return pMtx ;
}




