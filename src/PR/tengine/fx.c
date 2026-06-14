/*****************************************************************************
*
*
*
*				Special fx by Stephen Broumley
*
*
*
*****************************************************************************/


#include "stdafx.h"
#include "boss.h"
#include "hedtrack.h"
#include "scaling.h"
#include "tengine.h"
#include "fx.h"
#include "stdafx.h"
#include "romstruc.h"
#include "cart.h"
#include "rommodel.h"
#include "scaling.h"
#include "cartdir.h"
#include "regtype.h"
#include "tengine.h"
#include "ai.h"
#include "aistand.h"
#include "aidoor.h"
#include "tmove.h"
#include "tengine.h"
#include "mattable.h"
#include "geometry.h"
#include "boss.h"
#include "textload.h"
#include "dlists.h"
#include "pickup.h"
#include "region.h"
#include "parttype.h"
#include "textload.h"
#include "aistruc.h"
#include "graphu64.h"
#include "debug.h"






/*****************************************************************************
*	Constants
*****************************************************************************/
#define	LEAVE_SWOOSHES_ONSCREEN		0

#define	SWOOSH_EDGE_SCALER	1000.0



/*****************************************************************************
*	Swoosh variables
*****************************************************************************/

//////////////////////////////////////////////////////////////////////////////
//	Different colored swooshes
//////////////////////////////////////////////////////////////////////////////

#if 0

// Bright red swoosh
CSwooshInfo	RedSwooshInfo =
{
	SECONDS_TO_FRAMES(0.75),		// Total time on screen
	SECONDS_TO_FRAMES(0.5),			// Total time before fade swoosh out
	SECONDS_TO_FRAMES(0.25),		// Total edge life time
	{{{255,0,0}, {0,0,0}, MAX_OPAQ/2, MIN_OPAQ},			// Edge1: StCol, EndCol, StTrans,EndTrans
	 {{255,240,240}, {255,0,0}, MAX_OPAQ, MIN_OPAQ}},	// Edge2: StCol, EndCol, StTrans,EndTrans
} ;

// Bright yellow swoosh
CSwooshInfo	YellowSwooshInfo =
{
	SECONDS_TO_FRAMES(0.75),		// Total time on screen
	SECONDS_TO_FRAMES(0.5),			// Total time before fade swoosh out
	SECONDS_TO_FRAMES(0.25),		// Total edge life time
	{{{255,255,5}, {0,0,0}, MAX_OPAQ/2, MIN_OPAQ},			// Edge1: StCol, EndCol, StTrans,EndTrans
	 {{255,255,240}, {255,255,5}, MAX_OPAQ, MIN_OPAQ}},	// Edge2: StCol, EndCol, StTrans,EndTrans
} ;


// Bright blue swoosh
CSwooshInfo	BlueSwooshInfo =
{
	SECONDS_TO_FRAMES(0.75),		// Total time on screen
	SECONDS_TO_FRAMES(0.5),		// Total time before fade swoosh out
	SECONDS_TO_FRAMES(0.4),			// Total edge life time

	// Dim edge
	{{{0,0,255}, {0,0,0}, MAX_OPAQ/4, MIN_OPAQ},			// Edge1: StCol, EndCol, StTrans,EndTrans

	// Bright edge
	 {{240,240,255}, {0,0,255}, MAX_OPAQ, MIN_OPAQ}},	// Edge2: StCol, EndCol, StTrans,EndTrans
} ;

// Generic sublte weapon swoosh
CSwooshInfo	WeaponSwooshInfo =
{
	SECONDS_TO_FRAMES(0.4*5),		// Total time on screen
	SECONDS_TO_FRAMES(0.2*5),		// Total time before fade swoosh out
	SECONDS_TO_FRAMES(0.2*5),		// Total edge life time
	{{{255,255,255}, {140,140,140}, MAX_OPAQ, MIN_OPAQ},			// Edge1: StCol, EndCol, StTrans,EndTrans
	 {{255,255,255}, {140,140,140}, MAX_OPAQ, MIN_OPAQ}},	// Edge2: StCol, EndCol, StTrans,EndTrans
} ;

// Generic sublte weapon swoosh
CSwooshInfo	WeaponSwooshInfo =
{
	SECONDS_TO_FRAMES(0.4),		// Total time on screen
	SECONDS_TO_FRAMES(0.2),		// Total time before fade swoosh out
	SECONDS_TO_FRAMES(0.2),		// Total edge life time
	{{{255,255,255}, {140,140,140}, MAX_OPAQ/4, MIN_OPAQ},			// Edge1: StCol, EndCol, StTrans,EndTrans
	 {{140,140,100}, {40,40,40}, MAX_OPAQ/16, MIN_OPAQ}},	// Edge2: StCol, EndCol, StTrans,EndTrans
} ;
#endif


//////////////////////////////////////////////////////////////////////////////
//	Swoosh node info
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//
//	Mantis
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Mantis hand swooshes
//////////////////////////////////////////////////////////////////////////////
CSwooshEdgeVertexInfo MantisHandSwooshVertices[] =
{
	// Pos							Color					Alpha
	{{{0,		0,		0},  		{255,	255,  5},	MAX_OPAQ/2},	// Start
	 {{0,		-2.5,	-8},		{0,	0,		0},	MIN_OPAQ}},		// end

	{{{0,		-5,	-16},		{255,	255,  240},	MAX_OPAQ},		// Start
	 {{0,		-2.5,	-8},		{255,	255,	5},	MIN_OPAQ}},		// end
} ;

CSwooshInfo	MantisRightHandSwooshInfo =
{
	SECONDS_TO_FRAMES(0.75),		// Total time on screen
	SECONDS_TO_FRAMES(0.5),			// Total time before fade swoosh out
	SECONDS_TO_FRAMES(0.25),		// Total edge life time
	24,								 	// Object node
	SCALING_FACTOR,				 	// Scale
	NULL,								 	// General flags
	2,									 	// No of edge vertices
	MantisHandSwooshVertices		// Edge vertices
} ;

CSwooshInfo	MantisLeftHandSwooshInfo =
{
	SECONDS_TO_FRAMES(0.75),		// Total time on screen
	SECONDS_TO_FRAMES(0.5),			// Total time before fade swoosh out
	SECONDS_TO_FRAMES(0.25),		// Total edge life time
	27,								 	// Object node
	SCALING_FACTOR,				 	// Scale
	NULL,								 	// General flags
	2,									 	// No of edge vertices
	MantisHandSwooshVertices		// Edge vertices
} ;



//////////////////////////////////////////////////////////////////////////////
// Mantis feet swooshes
//////////////////////////////////////////////////////////////////////////////
CSwooshEdgeVertexInfo MantisFeetSwooshVertices[] =
{
	// Pos							Color					Alpha
	{{{0,		0,		0},  		{255,	255,  255},	MAX_OPAQ/4},	// Start
	 {{0,		0,		-2},		{10,	10,	 10},	MIN_OPAQ}},		// end

	{{{0,		0,		-4},		{255,	255,  255},	MAX_OPAQ/4},	// Start
	 {{0,		0,		-2},		{10,	10,	10},	MIN_OPAQ}},		// end
} ;

CSwooshInfo MantisLeftFrontFootSwooshInfo =
{
	SECONDS_TO_FRAMES(1.0),	 		// Total time on screen
	SECONDS_TO_FRAMES(0.8),	 		// Total time before fade swoosh out
	SECONDS_TO_FRAMES(0.25),		// Total edge life time
	5,									 	// Object node
	SCALING_FACTOR,				 	// Scale
	NULL,								 	// General flags
	2,									 	// No of edge vertices
	MantisFeetSwooshVertices		// Edge vertices
} ;

CSwooshInfo MantisLeftRearFootSwooshInfo =
{
	SECONDS_TO_FRAMES(1.0),	 		// Total time on screen
	SECONDS_TO_FRAMES(0.8),	 		// Total time before fade swoosh out
	SECONDS_TO_FRAMES(0.25),		// Total edge life time
	9,									 	// Object node
	SCALING_FACTOR,				 	// Scale
	NULL,								 	// General flags
	2,									 	// No of edge vertices
	MantisFeetSwooshVertices		// Edge vertices
} ;

CSwooshInfo MantisRightFrontFootSwooshInfo =
{
	SECONDS_TO_FRAMES(1.0),	 		// Total time on screen
	SECONDS_TO_FRAMES(0.8),	 		// Total time before fade swoosh out
	SECONDS_TO_FRAMES(0.25),		// Total edge life time
	13,  								 	// Object node
	SCALING_FACTOR,				 	// Scale
	NULL,								 	// General flags
	2,									 	// No of edge vertices
	MantisFeetSwooshVertices		// Edge vertices
} ;

CSwooshInfo MantisRightRearFootSwooshInfo =
{
	SECONDS_TO_FRAMES(1.0),	 		// Total time on screen
	SECONDS_TO_FRAMES(0.8),	 		// Total time before fade swoosh out
	SECONDS_TO_FRAMES(0.25),		// Total edge life time
	17,  								 	// Object node
	SCALING_FACTOR,				 	// Scale
	NULL,								 	// General flags
	2,									 	// No of edge vertices
	MantisFeetSwooshVertices		// Edge vertices
} ;










//////////////////////////////////////////////////////////////////////////////
//
//	Campaigner
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Campaigner weapon swoosh
//////////////////////////////////////////////////////////////////////////////
CSwooshEdgeVertexInfo CampaignerWeaponSwooshVertices[] =
{
	// Pos							Color					Alpha
	{{{0,		1,		0},		{0,	0,		255},	MAX_OPAQ/4},	// Start
	 {{0,		0,		7.5/2},	{0,	0,		0},	MIN_OPAQ}},		// end

	{{{0,		1,		7.5},		{240,	240,  255},	MAX_OPAQ},		// Start
	 {{0,		0,		7.5/2},	{0,	0,		255},	MIN_OPAQ}},		// end
} ;

CSwooshInfo	CampaignerWeaponSwooshInfo =
{
	SECONDS_TO_FRAMES(0.75),			// Total time on screen
	SECONDS_TO_FRAMES(0.5),				// Total time before fade swoosh out
	SECONDS_TO_FRAMES(0.4),				// Total edge life time
	19,										// Object node
	SCALING_FACTOR,						// Scale
	NULL,										// General flags
	2,											// No of edge vertices
	CampaignerWeaponSwooshVertices	// Edge vertices
} ;

//////////////////////////////////////////////////////////////////////////////
// Campaigner feet swooshes
//////////////////////////////////////////////////////////////////////////////
CSwooshEdgeVertexInfo CampaignerFeetSwooshVertices[] =
{
	// Pos							Color					Alpha
	{{{0,		-1,	0},		{50,	255, 50},	MAX_OPAQ},		// Start
	 {{0,		0,		0}, 		{0,	50,	0},	MIN_OPAQ}},		// end

	{{{0,		1,		0},		{50,	255,  50},	MAX_OPAQ},		// Start
	 {{0,		0,		0},		{0,	50,	0},	MIN_OPAQ}},		// end
} ;

CSwooshInfo	CampaignerLeftFootSwooshInfo =
{
	SECONDS_TO_FRAMES(1.0),			// Total time on screen
	SECONDS_TO_FRAMES(0.5),			// Total time before fade swoosh out
	SECONDS_TO_FRAMES(0.3),			// Total edge life time
	4,										// Object node
	SCALING_FACTOR,					// Scale
	NULL,									// General flags
	2,										// No of edge vertices
	CampaignerFeetSwooshVertices	// Edge vertices
} ;

CSwooshInfo	CampaignerRightFootSwooshInfo =
{
	SECONDS_TO_FRAMES(1.0),			// Total time on screen
	SECONDS_TO_FRAMES(0.5),			// Total time before fade swoosh out
	SECONDS_TO_FRAMES(0.3),			// Total edge life time
	8,										// Object node
	SCALING_FACTOR,					// Scale
	NULL,									// General flags
	2,										// No of edge vertices
	CampaignerFeetSwooshVertices	// Edge vertices
} ;

//////////////////////////////////////////////////////////////////////////////
// Campaigner left hand swoosh
//////////////////////////////////////////////////////////////////////////////
CSwooshEdgeVertexInfo CampaignerLeftHandSwooshVertices[] =
{
	// Pos							Color					Alpha
	{{{0,		0,		0},  		{50,	255, 50},	MAX_OPAQ},		// Start
	 {{0,		3,		-2},		{0,	50,	0},	MIN_OPAQ}},		// end

	{{{0,		6,		-4},		{50,	255,  50},	MAX_OPAQ},		// Start
	 {{0,		3,		-2},		{0,	50,	0},	MIN_OPAQ}},		// end
} ;

CSwooshInfo	CampaignerLeftHandSwooshInfo =
{
	SECONDS_TO_FRAMES(1.0),				// Total time on screen
	SECONDS_TO_FRAMES(0.75),			// Total time before fade swoosh out
	SECONDS_TO_FRAMES(0.6),				// Total edge life time
	12,										// Object node
	SCALING_FACTOR,						// Scale
	NULL,										// General flags
	2,											// No of edge vertices
	CampaignerLeftHandSwooshVertices	// Edge vertices
} ;


//////////////////////////////////////////////////////////////////////////////
// Campaigner right hand swoosh
//////////////////////////////////////////////////////////////////////////////
CSwooshEdgeVertexInfo CampaignerRightHandSwooshVertices[] =
{
	// Pos							Color					Alpha
	{{{0,		0,		0},  		{50,	255, 50},	MAX_OPAQ},		// Start
	 {{0,		3,		2},		{0,	50,	0},	MIN_OPAQ}},		// end

	{{{0,		6,		4},		{50,	255,  50},	MAX_OPAQ},		// Start
	 {{0,		3,		2},		{0,	50,	0},	MIN_OPAQ}},		// end
} ;

CSwooshInfo	CampaignerRightHandSwooshInfo =
{
	SECONDS_TO_FRAMES(1.0),				// Total time on screen
	SECONDS_TO_FRAMES(0.75),			// Total time before fade swoosh out
	SECONDS_TO_FRAMES(0.6),				// Total edge life time
	16,										// Object node
	SCALING_FACTOR,						// Scale
	NULL,										// General flags
	2,											// No of edge vertices
	CampaignerRightHandSwooshVertices	// Edge vertices
} ;




//////////////////////////////////////////////////////////////////////////////
//
//	TRex
//
//////////////////////////////////////////////////////////////////////////////
#if 0
CSwooshNodeInfo TRexTailSwooshNodeInfo =
{
	30,								// Object node
	1.0,								// Edge scale
	{{0,0,0},						// Edge pt1
	{-0*SCALING_FACTOR, -0*SCALING_FACTOR, -3*SCALING_FACTOR}}	// Edge pt2
} ;

#endif


//////////////////////////////////////////////////////////////////////////////
//
//	Longhunter
//
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// Longhunter subtle foot swooshes
//////////////////////////////////////////////////////////////////////////////
CSwooshEdgeVertexInfo LonghuntFootSwooshVertices[] =
{
	// Pos					Color				Alpha
	{{{0,	0,	-3},		{0, 0, 0},		MAX_OPAQ},	// Start
	 {{0,	0,	-1.5},	{0, 0, 0},		MIN_OPAQ}},	// end

	{{{0, 0,	0},		{10,10, 10},	MAX_OPAQ},	// Start
	 {{0,	0,	-1.5},	{0, 0, 0},		MIN_OPAQ}},	// end
} ;

CSwooshInfo	LonghuntLeftFootSwooshInfo =
{
	SECONDS_TO_FRAMES(0.75*0.75),	// Total time on screen
	SECONDS_TO_FRAMES(0.5*0.75),	// Total time before fade swoosh out
	SECONDS_TO_FRAMES(0.25*0.75),	// Total edge life time
	3,										// Object node
	SCALING_FACTOR,					// Scale
	NULL,									// General flags
	2,										// No of edge vertices
	LonghuntFootSwooshVertices		// Edge vertices
} ;

CSwooshInfo	LonghuntRightFootSwooshInfo =
{
	SECONDS_TO_FRAMES(0.75*0.75),	// Total time on screen
	SECONDS_TO_FRAMES(0.5*0.75),	// Total time before fade swoosh out
	SECONDS_TO_FRAMES(0.25*0.75),	// Total edge life time
	8,										// Object node
	SCALING_FACTOR,					// Scale
	NULL,									// General flags
	2,										// No of edge vertices
	LonghuntFootSwooshVertices		// Edge vertices
} ;

CSwooshInfo	LonghuntLeftFootEvadeSwooshInfo =
{
	SECONDS_TO_FRAMES(1.0*0.5), 	// Total time on screen
	SECONDS_TO_FRAMES(0.5*0.5),	// Total time before fade swoosh out
	SECONDS_TO_FRAMES(0.3*0.5), 	// Total edge life time
	3,										// Object node
	SCALING_FACTOR,					// Scale
	NULL,									// General flags
	2,										// No of edge vertices
	LonghuntFootSwooshVertices		// Edge vertices
} ;

CSwooshInfo	LonghuntRightFootEvadeSwooshInfo =
{
	SECONDS_TO_FRAMES(1.0*0.5), 	// Total time on screen
	SECONDS_TO_FRAMES(0.5*0.5),	// Total time before fade swoosh out
	SECONDS_TO_FRAMES(0.3*0.5), 	// Total edge life time
	8,										// Object node
	SCALING_FACTOR,					// Scale
	NULL,									// General flags
	2,										// No of edge vertices
	LonghuntFootSwooshVertices		// Edge vertices
} ;




//////////////////////////////////////////////////////////////////////////////
// Longhunter subtle left hand swoosh
//////////////////////////////////////////////////////////////////////////////
#if 0
CSwooshEdgeVertexInfo LonghuntLeftHandSwooshVertices[] =
{
	// Pos			Color					Alpha
	{{{0,	0,	0}, {255,	255,	255},	MAX_OPAQ},	// Start
	 {{0,	3,	0}, {255,	255,	255},	MIN_OPAQ}},		// end

	{{{0,	6,	0}, {255,	255,  255},	MAX_OPAQ},	// Start
	 {{0,	3,	0}, {255,	255,	255},	MIN_OPAQ}},		// end
} ;
CSwooshEdgeVertexInfo LonghuntLeftHandSwooshVertices[] =
{
	// Pos			Color					Alpha
	{{{0,	0,	0}, {0,	0,		0},	MAX_OPAQ/3},	// Start
	 {{0,	3,	0}, {0,	0,		0},	MIN_OPAQ}},		// end

	{{{0,	6,	0}, {10,	10,  10},	MAX_OPAQ/3},	// Start
	 {{0,	3,	0}, {0,	0,		0},	MIN_OPAQ}},		// end
} ;

CSwooshInfo	LonghuntLeftHandSwooshInfo =
{
	SECONDS_TO_FRAMES(0.75*0.75),		// Total time on screen
	SECONDS_TO_FRAMES(0.5*0.75),		// Total time before fade swoosh out
	SECONDS_TO_FRAMES(0.25*0.75),		// Total edge life time
	14,										// Object node
	SCALING_FACTOR,						// Scale
	NULL,										// General flags
	2,											// No of edge vertices
	LonghuntLeftHandSwooshVertices	// Edge vertices
} ;
#endif


//////////////////////////////////////////////////////////////////////////////
// Longhunter subtle right hand swoosh
//////////////////////////////////////////////////////////////////////////////
CSwooshEdgeVertexInfo LonghuntRightHandSwooshVertices[] =
{
	// Pos							Color					Alpha
	{{{0,		0,	 0},	{0,	0, 0},	MAX_OPAQ},	// Start
	 {{0,	11.1/2,0},	{0,	0, 0},	MIN_OPAQ}},		// end

	{{{0,	11.1,  0},	{10,	10, 10},	MAX_OPAQ},	// Start
	 {{0,	11.1/2,0},	{0,	0,	 0},	MIN_OPAQ}},		// end
} ;

CSwooshInfo	LonghuntRightHandSwooshInfo =
{
	SECONDS_TO_FRAMES(0.75*0.75),		// Total time on screen
	SECONDS_TO_FRAMES(0.5*0.75),		// Total time before fade swoosh out
	SECONDS_TO_FRAMES(0.25*0.75),		// Total edge life time
	18,										// Object node
	SCALING_FACTOR,						// Scale
	NULL,										// General flags
	2,											// No of edge vertices
	LonghuntRightHandSwooshVertices	// Edge vertices
} ;

//////////////////////////////////////////////////////////////////////////////
// Longhunt subtle toe jumping swoosh
//////////////////////////////////////////////////////////////////////////////
#if 0
CSwooshEdgeVertexInfo LonghuntToeSwooshVertices[] =
{
	// Pos							Color					Alpha
	{{{0,		-1,		0},	{255,	255, 255},	MAX_OPAQ},	// Start
	 {{0,		0,			0},	{255,	255, 255},	MIN_OPAQ}},		// end

	{{{0,		1, 		0},	{255,	255, 255},	MAX_OPAQ},	// Start
	 {{0,		0,			0},	{255,	255, 255},	MIN_OPAQ}},		// end
} ;

CSwooshInfo	LonghuntLeftToesSwooshInfo =
{
	SECONDS_TO_FRAMES(1.0*0.75),	// Total time on screen
	SECONDS_TO_FRAMES(0.5*0.75),	// Total time before fade swoosh out
	SECONDS_TO_FRAMES(0.3*0.75),	// Total edge life time
	4,										// Object node
	SCALING_FACTOR,					// Scale
	NULL,									// General flags
	2,										// No of edge vertices
	LonghuntToeSwooshVertices		// Edge vertices
} ;

CSwooshInfo	LonghuntRightToesSwooshInfo =
{
	SECONDS_TO_FRAMES(1.0*0.75),	// Total time on screen
	SECONDS_TO_FRAMES(0.5*0.75),	// Total time before fade swoosh out
	SECONDS_TO_FRAMES(0.3*0.75),	// Total edge life time
	9,										// Object node
	SCALING_FACTOR,					// Scale
	NULL,									// General flags
	2,										// No of edge vertices
	LonghuntToeSwooshVertices		// Edge vertices
} ;

#endif





/*****************************************************************************
*	Functions
*****************************************************************************/

void AI_SEvent_HumanSwoosh(CGameObjectInstance *pThis) ;

void CFxSystem__UpdateSwooshes(CFxSystem *pThis) ;
BOOL CFxSystem__UpdateSwoosh(CFxSystem *pThis, CSwoosh *pSwoosh) ;
void CFxSystem__UpdateAllTimerFx(CFxSystem *pThis) ;


/*****************************************************************************
*
*
*
*	SWOOSH CODE
*
*
*
*****************************************************************************/

// List macros
#define CSwoosh__EdgeListHead(pThis)	(CSwooshEdge *)((pThis)->m_EdgeList.m_pHead)
#define CSwoosh__EdgeListTail(pThis)	(CSwooshEdge *)((pThis)->m_EdgeList.m_pTail)
#define CSwoosh__EdgeListLength(pThis)	(pThis)->m_EdgeList.m_Length


/*****************************************************************************
*
*	Function:		CFxSystem__AddEdgeToSwoosh()
*
******************************************************************************
*
*	Description:	Adds edge to swoosh. Needs draw matrix from onbject instance
*						to transform edge of swoosh into world co-ords for drawing
*
*	Inputs:			*pThis			- Ptr to swoosh
*						*pmtDrawMtxs	- Ptr to object instance matrix draw table
*
*	Outputs:			None
*
*****************************************************************************/
void CFxSystem__AddEdgeToSwoosh(CFxSystem *pThis, CSwoosh *pSwoosh, Mtx *pmtDrawMtxs)
{
	CSwooshEdge	*pEdge ;
	CMtxF			mfMtx ;
	FLOAT			Scale ;

	// Matrix there?
	if (!pmtDrawMtxs)
		return ;

	// Try allocate new edge
	pEdge = CFxSystem__AllocateSwooshEdge(pThis, pSwoosh) ;
	if (pEdge)
	{
		// Setup life time
		pEdge->m_LifeTime = pSwoosh->m_pInfo->m_EdgeLifeTime ;

		// Copy orientation matrix
		if (pSwoosh->m_pParticle)
			*(&pEdge->m_mMatrix) = *pmtDrawMtxs ;
		else
		if (pSwoosh->m_pObject)
		{
			if (pSwoosh->m_pInfo->m_Node == -1)
				*(&pEdge->m_mMatrix) = CGameObjectInstance__GetOrientationMatrix(pSwoosh->m_pObject) ;
			else
				*(&pEdge->m_mMatrix) = pmtDrawMtxs[pSwoosh->m_pInfo->m_Node] ;
		}

		// Pre-multiply matrix by scale
		CMtxF__FromMtx(mfMtx, pEdge->m_mMatrix) ;
		Scale = pSwoosh->m_pInfo->m_Scale / SWOOSH_EDGE_SCALER ;
		CMtxF__PreMultScale(mfMtx, Scale, Scale, Scale) ;
		CMtxF__Clamp(mfMtx) ;
		CMtxF__ToMtx(mfMtx, pEdge->m_mMatrix) ;
	}
}




/*****************************************************************************
*
*	Function:		CSwoosh__SetupEdgeVertices()
*
******************************************************************************
*
*	Description:	Sets up position, color, and alpha of all vertices in an edge
*
*	Inputs:			*pThis			- Ptr to swoosh
*						*pEdge			- Ptr to swoosh edge
*						*pVertexList	- Ptr to vertex list
*
*	Outputs:			None.
*
*****************************************************************************/
void CSwoosh__SetupEdgeVertices(CSwoosh *pThis, CSwooshEdge *pEdge, Vtx *pVertexList)
{
	INT32	i ;
	FLOAT	t ;
	CSwooshEdgeVertexInfo *pInfo = pThis->m_pInfo->m_pEdgeVertexList ;

	// Setup tween
	t = COSINE_TWEEN(pEdge->m_LifeTime / pThis->m_pInfo->m_EdgeLifeTime) ;
	if (pThis->m_LifeTime <= pThis->m_pInfo->m_FadeOutTime)
		t *= COSINE_TWEEN(pThis->m_LifeTime / pThis->m_pInfo->m_FadeOutTime) ;
	t = 1.0 - t ;

	// Setup vertex position, color, and alpha.
	i = pThis->m_pInfo->m_NoOfEdgeVertices ;
	pInfo = pThis->m_pInfo->m_pEdgeVertexList ;
	while(i--)
	{
		// Setup position
		pVertexList->v.ob[0] = (SWOOSH_EDGE_SCALER * (pInfo->m_Start.m_vPos.x +
									  (t * (pInfo->m_End.m_vPos.x - pInfo->m_Start.m_vPos.x)))) ;

		pVertexList->v.ob[1] = (SWOOSH_EDGE_SCALER * (pInfo->m_Start.m_vPos.y +
									  (t * (pInfo->m_End.m_vPos.y - pInfo->m_Start.m_vPos.y)))) ;

		pVertexList->v.ob[2] = (SWOOSH_EDGE_SCALER * (pInfo->m_Start.m_vPos.z +
									  (t * (pInfo->m_End.m_vPos.z - pInfo->m_Start.m_vPos.z)))) ;

		// Setup color
		pVertexList->v.cn[0] = pInfo->m_Start.m_Color.Red +
									  (t * (pInfo->m_End.m_Color.Red - pInfo->m_Start.m_Color.Red)) ;

		pVertexList->v.cn[1] = pInfo->m_Start.m_Color.Green +
									  (t * (pInfo->m_End.m_Color.Green - pInfo->m_Start.m_Color.Green)) ;

		pVertexList->v.cn[2] = pInfo->m_Start.m_Color.Blue +
									  (t * (pInfo->m_End.m_Color.Blue - pInfo->m_Start.m_Color.Blue)) ;

		// Setup alpha
		pVertexList->v.cn[3] = pInfo->m_Start.m_Alpha +
									  (t * (pInfo->m_End.m_Alpha - pInfo->m_Start.m_Alpha)) ;

		// Next vertex
		pInfo++ ;
		pVertexList++ ;
	}
}



/*****************************************************************************
*
*	Function:		CSwoosh__Update()
*
******************************************************************************
*
*	Description:	Updates all edges in swoosh, and swoosh life time
*
*	Inputs:			*pThis		- Ptr to swoosh
*
*	Outputs:			TRUE if swoosh is active, else FALSE
*
*****************************************************************************/
BOOL CFxSystem__UpdateSwoosh(CFxSystem *pThis, CSwoosh *pSwoosh)
{
	CSwooshEdge	*pEdge, *pNextEdge ;
	float PrevFrameIncrement = frame_increment ;

	// Makes swooshes correct length for spirit mode
	switch(pSwoosh->m_Type)
	{
		case OBJECT_SWOOSH:
			frame_increment *= enemy_speed_scaler ;
			break ;

		case PARTICLE_SWOOSH:
			frame_increment *= particle_speed_scaler ;
			break ;
	}

	// Decrease life time
	pSwoosh->m_LifeTime -= frame_increment ;
	if (pSwoosh->m_LifeTime < 0)
		pSwoosh->m_LifeTime = 0 ;

	// Loop through edges
	pEdge = CSwoosh__EdgeListHead(pSwoosh) ;
	while(pEdge)
	{
		// Setup next edge incase of deletion
		pNextEdge = (CSwooshEdge *)pEdge->m_Node.m_pNext ;

		// If swoosh life time has ran out, then delete all edges
		if ((pSwoosh->m_LifeTime <= 0) && (pEdge->m_LifeTime > 0))
			pEdge->m_LifeTime = 0 ;

		// Update edge
		if (pEdge->m_LifeTime <= 0)
		{
			// Delete edge after 3 frames so that drawing is finished using it
			pEdge->m_LifeTime -= 1 ;
			if (pEdge->m_LifeTime < -3)
				CFxSystem__DeallocateSwooshEdge(pThis, pSwoosh, pEdge) ;
		}
		else
		{
			// Decrease life time
			pEdge->m_LifeTime -= frame_increment ;
			if (pEdge->m_LifeTime < 0)
				pEdge->m_LifeTime = 0 ;
		}

		// Next edge
		pEdge = pNextEdge ;
	}

	// If object has gone off screen, then stop adding swoosh edges so that big swooshes aren't drawn
	// when it comes back on at a different place
	if ((pSwoosh->m_pObject) && (!CGameObjectInstance__IsVisible(pSwoosh->m_pObject)))
		pSwoosh->m_LifeTime = 0 ;

	// Restore frame increment
	frame_increment = PrevFrameIncrement ;


#if LEAVE_SWOOSHES_ONSCREEN
	return TRUE ;
#else

	// Return true if active edges
	if ((!CSwoosh__EdgeListLength(pSwoosh)) &&
		 ((pSwoosh->m_LifeTime == 0) ||
		 ((!pSwoosh->m_pParticle) && (!pSwoosh->m_pObject))))
		return FALSE ;
	else
		return TRUE ;

#endif
}




/*****************************************************************************
*
*	Function:		CFxSystem__DrawSwoosh()
*
******************************************************************************
*
*	Description:	Goes through all swoosh edges and joins them with polygons
*						 - adding them to the graphics display list
*
*	Inputs:			*pThis		- Ptr to swoosh
*						**ppDLP		- Graphics list ptr (to a ptr)
*
*	Outputs:			None
*
*****************************************************************************/
void CFxSystem__DrawSwoosh(CFxSystem *pThis, CSwoosh *pSwoosh, Gfx **ppDLP)
{
	CSwooshEdge	*pEdge1 ;
	CSwooshEdge	*pEdge2 ;
	INT32			NoOfSides, NoOfVertices, i ;
	Vtx			*pVertexList ;
	INT32			TotalEdges ;

	// Must be at least two edges to draw the thing
	TotalEdges = CSwoosh__EdgeListLength(pSwoosh) ;
	if (TotalEdges >= 2)
	{
		// Start on at newest end of swoosh
		pEdge1 = CSwoosh__EdgeListTail(pSwoosh) ;
		pEdge2 = (CSwooshEdge *)pEdge1->m_Node.m_pPrev ;

		// Every edge has the same number of vertices, and sides.
		NoOfVertices = pSwoosh->m_pInfo->m_NoOfEdgeVertices ;
		NoOfSides = NoOfVertices - 1 ;

		// Add 1st edge matrix to display list
	  	gSPMatrix((*ppDLP)++,
					 RSP_ADDRESS(&pEdge1->m_mMatrix),
		   		 G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH) ;

		while((pEdge1) && (pEdge2) && (pEdge1->m_LifeTime > 0) && (pEdge2->m_LifeTime > 0))
		{
			// Allocate vertices for edge1
			pVertexList = CFxSystem__AllocateVertexList(pThis, NoOfVertices) ;
			if (!pVertexList)
				return ;

			// Setup edge1 vertex list
			CSwoosh__SetupEdgeVertices(pSwoosh, pEdge1, pVertexList) ;

			// Add edge1 vertices to display list
			gSPVertex((*ppDLP)++,
						 RSP_ADDRESS(pVertexList),
						 NoOfVertices, 0) ;					// Total, Start index

			// Add edge2 matrix to display list
		  	gSPMatrix((*ppDLP)++,
						 RSP_ADDRESS(&pEdge2->m_mMatrix),
			   		 G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH) ;

			// Allocate vertices for edge2
			pVertexList = CFxSystem__AllocateVertexList(pThis, NoOfVertices) ;
			if (!pVertexList)
				return ;

			// Setup edge2 vertex list
			CSwoosh__SetupEdgeVertices(pSwoosh, pEdge2, pVertexList) ;

			// Add edge2 vertices to display list
			gSPVertex((*ppDLP)++,
						 RSP_ADDRESS(pVertexList),
						 NoOfVertices, NoOfVertices) ; 	// Total, Start index

			// Add triangles to join edges
			for (i = 0 ; i < NoOfSides ; i++)
			{
				gSP2Triangles((*ppDLP)++,
								  i, NoOfVertices+i, NoOfVertices+i+1, 0,
								  i, NoOfVertices+i+1, i+1, 0);
			}

			// Close edges?
			if (pSwoosh->m_pInfo->m_Flags & SWOOSH_CLOSE_EDGES)
			{
				gSP2Triangles((*ppDLP)++,
								  i, NoOfVertices+i, NoOfVertices, 0,
								  i, NoOfVertices, 0, 0);
			}

			// Next edge
			pEdge1 = pEdge2 ;
			pEdge2 = (CSwooshEdge *)pEdge2->m_Node.m_pPrev ;
		}
	}
}


















/*****************************************************************************
*
*
*
*	FX SYSTEM CODE
*
*
*
*****************************************************************************/


// List macros
#define CFxSystem__SwooshListHead(pThis)				(CSwoosh *)pThis->m_SwooshList.m_ActiveList.m_pHead
#define CFxSystem__AllocateSwoosh(pThis)				(CSwoosh *)CDynamicList__AllocateNode(&pThis->m_SwooshList)
#define CFxSystem__DeallocateSwoosh(pThis, pNode)	CDynamicList__DeallocateNode(&pThis->m_SwooshList, (CNode *)pNode)



/*****************************************************************************
*
*	Function:		CFxSystem__Construct()
*
******************************************************************************
*
*	Description:	Initializes all fx system lists to empty
*
*	Inputs:			*pThis		- Ptr to fx system
*
*	Outputs:			None
*
*****************************************************************************/
void CFxSystem__Construct(CFxSystem *pThis)
{
	INT32	i ;

#if 0
	GetApp()->m_dwEnabledCheatFlags = -1;
//	GetApp()->m_dwCheatFlags |= CHEATFLAG_INVINCIBILITY |
//										 CHEATFLAG_ALLWEAPONS |
//										 CHEATFLAG_UNLIMITEDAMMO ;
	GetApp()->m_dwCheatFlags = -1 ;
#endif


	// Initialise swoosh list
	CDynamicList__Construct(&pThis->m_SwooshList, (UINT8 *)pThis->m_SwooshVars, sizeof(CSwoosh), MAX_SWOOSHES) ;

	// Initialise timer fx list
	CDynamicList__Construct(&pThis->m_TimerFxList, (UINT8 *)pThis->m_TimerFxVars, sizeof(CTimerFx), MAX_TIMER_FX) ;

	// Initialise swoosh edge list
	CList__Construct(&pThis->m_SwooshEdgeList) ;
	for (i = 0 ; i < MAX_SWOOSH_EDGES ; i++)
		CList__AppendNode(&pThis->m_SwooshEdgeList, (CNode *)&pThis->m_SwooshEdgeVars[i]) ;

	// Call update to init everying for first frame
	CFxSystem__Update(pThis) ;
}

CSwooshEdge *CFxSystem__AllocateSwooshEdge(CFxSystem *pThis, CSwoosh *pSwoosh)
{
	CSwooshEdge	*pEdge = (CSwooshEdge *)pThis->m_SwooshEdgeList.m_pHead ;
	if (pEdge)
	{
		CList__DeleteNode(&pThis->m_SwooshEdgeList, (CNode *)pEdge) ;
		CList__AppendNode(&pSwoosh->m_EdgeList, (CNode *)pEdge) ;
	}

	return pEdge ;
}

void CFxSystem__DeallocateSwooshEdge(CFxSystem *pThis, CSwoosh *pSwoosh, CSwooshEdge *pEdge)
{
	CList__DeleteNode(&pSwoosh->m_EdgeList, (CNode *)pEdge) ;
	CList__AppendNode(&pThis->m_SwooshEdgeList, (CNode *)pEdge) ;
}


void CFxSystem__Update(CFxSystem *pThis)
{
	// Reset free vertex list
	pThis->m_VtxsFree = MAX_FXSYSTEM_VERTICES ;
	pThis->m_pFreeVertex = &pThis->m_VertexList[even_odd][0] ;

	// Only update if not in pause mode
	if (!GetApp()->m_bPause)
	{
		// Update swooshes
		CFxSystem__UpdateSwooshes(pThis) ;

		// Update timer fx's
		CFxSystem__UpdateAllTimerFx(pThis) ;
	}
}

Vtx *CFxSystem__AllocateVertexList(CFxSystem *pThis, INT32 NoOfVertices)
{
	Vtx	*pVtx ;

	// Enough free?
	if (NoOfVertices > pThis->m_VtxsFree)
		return NULL ;
	else
	{
		// Update free ptrs
		pVtx = pThis->m_pFreeVertex ;
		pThis->m_VtxsFree -= NoOfVertices ;
		pThis->m_pFreeVertex += NoOfVertices ;
		return pVtx ;
	}
}



/*****************************************************************************
*
*	Function:		CFxSystem__CheckForAddingObjectSwooshEdge()
*
******************************************************************************
*
*	Description:	Searches through all swooshes to see if they are attatched
*						to the object - if so a new swoosh edge is added.
*
*	Inputs:			*pThis			- Ptr to fx system
*						*pObject			- Ptr to object instance to check for active swooshes
*						*pmtDrawMtxs	- Ptr to object matrix draw table
*
*	Outputs:			None
*
*****************************************************************************/
void CFxSystem__CheckForAddingObjectSwooshEdge(CFxSystem *pThis, CGameObjectInstance *pObject, Mtx *pmtDrawMtxs)
{
	// Search all swooshes
	CSwoosh	*pSwoosh = CFxSystem__SwooshListHead(pThis) ;
	while(pSwoosh)
	{
		// Does this object own swoosh?
		if ((pSwoosh->m_pObject == pObject) && (pSwoosh->m_LifeTime > 0))
			CFxSystem__AddEdgeToSwoosh(pThis, pSwoosh, pmtDrawMtxs) ;

		// Check next swoosh
		pSwoosh = (CSwoosh *)pSwoosh->m_Node.m_pNext ;
	}
}

void CFxSystem__CheckForAddingParticleSwooshEdge(CFxSystem *pThis, CParticle *pParticle, Mtx *pmtDrawMtxs)
{
	// Search all swooshes
	CSwoosh	*pSwoosh = CFxSystem__SwooshListHead(pThis) ;

	while(pSwoosh)
	{
		// Does this object own swoosh?
		if ((pSwoosh->m_pParticle == pParticle) && (pSwoosh->m_LifeTime > 0))
			CFxSystem__AddEdgeToSwoosh(pThis, pSwoosh, pmtDrawMtxs) ;

		// Check next swoosh
		pSwoosh = (CSwoosh *)pSwoosh->m_Node.m_pNext ;
	}
}


void CSwooosh__Kill(CSwoosh *pThis)
{
	// Update particle
	if (pThis->m_pParticle)
	{
		pThis->m_pParticle->m_ActiveSwooshes-- ;
		pThis->m_pParticle = NULL ;
	}

	// Update object
	if (pThis->m_pObject)
	{
		pThis->m_pObject->m_ActiveSwooshes-- ;
		pThis->m_pObject = NULL ;
	}

	// Stop lifetime
	if (pThis->m_LifeTime < pThis->m_pInfo->m_FadeOutTime)
		pThis->m_LifeTime = pThis->m_pInfo->m_FadeOutTime ;
}

void CFxSystem__StopSwooshesOnParticle(CFxSystem *pThis, CParticle *pParticle)
{
	// Search all swooshes
	CSwoosh	*pSwoosh = CFxSystem__SwooshListHead(pThis) ;
	while(pSwoosh)
	{
		// Does this swoosh own particle?
		if (pSwoosh->m_pParticle == pParticle)
			CSwooosh__Kill(pSwoosh) ;

		// Check next swoosh
		pSwoosh = (CSwoosh *)pSwoosh->m_Node.m_pNext ;
	}
}


void CFxSystem__StopSwooshesOnObject(CFxSystem *pThis, CGameObjectInstance *pObject)
{
	// Search all swooshes
	CSwoosh	*pSwoosh = CFxSystem__SwooshListHead(pThis) ;
	while(pSwoosh)
	{
		// Does this swoosh own particle?
		if (pSwoosh->m_pObject == pObject)
			CSwooosh__Kill(pSwoosh) ;

		// Check next swoosh
		pSwoosh = (CSwoosh *)pSwoosh->m_Node.m_pNext ;
	}
}







/*****************************************************************************
*
*	Function:		CFxSystem__AddObjectSwoosh()
*
******************************************************************************
*
*	Description:	Attaches swoosh to object and adds swoosh to fx system
*
*	Inputs:			*pThis			- Ptr to fx system
*						*pObject			- Ptr to object instance to start swoosh on
*						*pInfo			- Swoosh color, life time, etc info
*						*pNodeInfo		- Ptr to object node, edge position, etc info
*
*	Outputs:			None
*
*****************************************************************************/
void CFxSystem__AddObjectSwoosh(CFxSystem *pThis, CGameObjectInstance *pObject, CSwooshInfo *pInfo)
{
	CSwoosh	*pSwoosh ;

	// Try allocate swoosh
	pSwoosh = CFxSystem__AllocateSwoosh(pThis) ;
	if (pSwoosh)
	{
		// Prepare swoosh
		CList__Construct(&pSwoosh->m_EdgeList) ;
		pSwoosh->m_Type =	OBJECT_SWOOSH ;
		pSwoosh->m_pInfo = pInfo ;
		pSwoosh->m_LifeTime = pInfo->m_LifeTime ;
		pSwoosh->m_pObject = pObject ;
		pSwoosh->m_pParticle = NULL ;

		pObject->m_ActiveSwooshes++ ;
	}
}

void CFxSystem__AddParticleSwoosh(CFxSystem *pThis, CParticle *pParticle, CSwooshInfo *pInfo)
{
	CSwoosh	*pSwoosh ;

	// Try allocate swoosh
	pSwoosh = CFxSystem__AllocateSwoosh(pThis) ;
	if (pSwoosh)
	{
		// Prepare swoosh
		CList__Construct(&pSwoosh->m_EdgeList) ;
		pSwoosh->m_Type =	PARTICLE_SWOOSH ;
		pSwoosh->m_pInfo = pInfo ;
		pSwoosh->m_LifeTime = pInfo->m_LifeTime ;
		pSwoosh->m_pObject = NULL ;
		pSwoosh->m_pParticle = pParticle ;

		pParticle->m_ActiveSwooshes++ ;
	}
}


/*****************************************************************************
*
*	Function:		CFxSystem__UpdateSwooshes()
*
******************************************************************************
*
*	Description:	Updates, deletes etc, all active swooshes
*
*	Inputs:			*pThis			- Ptr to fx system
*
*	Outputs:			None
*
*****************************************************************************/
void CFxSystem__UpdateSwooshes(CFxSystem *pThis)
{
	CSwoosh	*pSwoosh, *pNextSwoosh ;

	// Update all swooshes
	pSwoosh = CFxSystem__SwooshListHead(pThis) ;
	while(pSwoosh)
	{
		// Keep ptr incase of deletion
		pNextSwoosh = (CSwoosh *)pSwoosh->m_Node.m_pNext ;

		// Update and delete if need be..
		if (!CFxSystem__UpdateSwoosh(pThis, pSwoosh))
		{
			if (pSwoosh->m_pObject)
			{
				if (pSwoosh->m_pObject->m_ActiveSwooshes > 0)
					pSwoosh->m_pObject->m_ActiveSwooshes-- ;
			}
			else
			if (pSwoosh->m_pParticle)
			{
				if (pSwoosh->m_pParticle->m_ActiveSwooshes > 0)
					pSwoosh->m_pParticle->m_ActiveSwooshes-- ;
			}

			CDynamicList__DeallocateNode(&pThis->m_SwooshList, (CNode *)pSwoosh)  ;
		}

		// Process swoosh
		pSwoosh = pNextSwoosh ;
	}
}




/*****************************************************************************
*
*	Function:		CFxSystem__DrawSwooshes()
*
******************************************************************************
*
*	Description:	Draws all active swooshes
*
*	Inputs:			*pThis			- Ptr to fx system
*						**ppDLP		- Graphics list ptr (to a ptr)
*
*	Outputs:			None
*
*****************************************************************************/
void CFxSystem__DrawSwooshes(CFxSystem *pThis, Gfx **ppDLP)
{
	CSwoosh	*pSwoosh ;

	// Update all swooshes
	pSwoosh = CFxSystem__SwooshListHead(pThis) ;
	while(pSwoosh)
	{
		// Draw and goto next
		CFxSystem__DrawSwoosh(pThis, pSwoosh, ppDLP) ;
		pSwoosh = (CSwoosh *)pSwoosh->m_Node.m_pNext ;
	}
}




/*****************************************************************************
*
*	Function:		CFxSystem__Draw()
*
******************************************************************************
*
*	Description:	Draws the complete fx system
*
*	Inputs:			*pThis			- Ptr to fx system
*						**ppDLP		- Graphics list ptr (to a ptr)
*
*	Outputs:			None
*
*****************************************************************************/
void CFxSystem__Draw(CFxSystem *pThis, Gfx **ppDLP)
{
	// Set up draw stuff ready for transparent and colored polygons
	gDPPipeSync((*ppDLP)++) ;
	CGeometry__SetRenderMode(ppDLP, G_RM_AA_ZB_XLU_SURF__G_RM_AA_ZB_XLU_SURF2);
	gDPSetCycleType((*ppDLP)++, G_CYC_1CYCLE) ;

	CGeometry__SetCombineMode(ppDLP, G_CC_SHADE__G_CC_SHADE) ;
	gSPTexture((*ppDLP)++, 0, 0, 0, 0, G_OFF);
	CTextureLoader__InvalidateTextureCache();
  	gSPClearGeometryMode((*ppDLP)++, G_LIGHTING | G_CULL_BACK | G_TEXTURE_GEN | G_FOG);
	gSPSetGeometryMode((*ppDLP)++, G_SHADE | G_SHADING_SMOOTH);

	// Update and draw those nice swooshes
	CFxSystem__DrawSwooshes(pThis, ppDLP) ;

	// Wait just in case
	//gDPPipeSync((*ppDLP)++) ;

//	osSyncPrintf("Frm:%d VerticesFree:%d EdgesFree:%d\r\n",
//					 game_frame_number,
//					 pThis->m_VtxsFree,
//					 pThis->m_SwooshEdgeList.m_Length) ;
}








/*****************************************************************************
*
*
*
*	SWOOSH EVENT CODE
*
*
*
*****************************************************************************/



/*****************************************************************************
*
*	Function:		AI_SEvent_SwooshAllHands()
*
******************************************************************************
*
*	Description:	Adds swoosh to all hands of an object instance
*
*	Inputs:			*pThis		- Ptr to object instance header
*						vPos			- World position of event
*						Value			- Event misc value
*
*	Outputs:			None
*
*****************************************************************************/
void AI_SEvent_SwooshAllHands(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	if (pThis->m_Type == I_ANIMATED)
	{
		switch(CGameObjectInstance__TypeFlag((CGameObjectInstance *)pThis))
		{
			case AI_OBJECT_CHARACTER_MANTIS_BOSS:
				CFxSystem__AddObjectSwoosh(&GetApp()->m_FxSystem, (CGameObjectInstance *)pThis, &MantisLeftHandSwooshInfo) ;
				CFxSystem__AddObjectSwoosh(&GetApp()->m_FxSystem, (CGameObjectInstance *)pThis, &MantisRightHandSwooshInfo) ;
				break ;

			case AI_OBJECT_CHARACTER_CAMPAIGNER_BOSS:
				CFxSystem__AddObjectSwoosh(&GetApp()->m_FxSystem, (CGameObjectInstance *)pThis, &CampaignerLeftHandSwooshInfo) ;
				CFxSystem__AddObjectSwoosh(&GetApp()->m_FxSystem, (CGameObjectInstance *)pThis, &CampaignerRightHandSwooshInfo) ;

			case AI_OBJECT_CHARACTER_LONGHUNTER_BOSS:
//				CFxSystem__AddObjectSwoosh(&GetApp()->m_FxSystem, (CGameObjectInstance *)pThis, &LonghuntLeftHandSwooshInfo) ;
				CFxSystem__AddObjectSwoosh(&GetApp()->m_FxSystem, (CGameObjectInstance *)pThis, &LonghuntRightHandSwooshInfo) ;
				break ;

			case AI_OBJECT_CHARACTER_HUMAN1:
				AI_SEvent_HumanSwoosh((CGameObjectInstance *)pThis) ;
				break ;
		}
	}
}


/*****************************************************************************
*
*	Function:		AI_SEvent_SwooshLeftHand()
*
******************************************************************************
*
*	Description:	Adds swoosh to left hand of an object instance
*
*	Inputs:			*pThis		- Ptr to object instance header
*						vPos			- World position of event
*						Value			- Event misc value
*
*	Outputs:			None
*
*****************************************************************************/
void AI_SEvent_SwooshLeftHand(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	if (pThis->m_Type == I_ANIMATED)
	{
		switch(CGameObjectInstance__TypeFlag((CGameObjectInstance *)pThis))
		{
//			case AI_OBJECT_CHARACTER_LONGHUNTER_BOSS:
//				CFxSystem__AddObjectSwoosh(&GetApp()->m_FxSystem, (CGameObjectInstance *)pThis, &LonghuntLeftHandSwooshInfo) ;
//				break ;

			case AI_OBJECT_CHARACTER_HUMAN1:
				AI_SEvent_HumanSwoosh((CGameObjectInstance *)pThis) ;
				break ;
		}
	}
}



/*****************************************************************************
*
*	Function:		AI_SEvent_SwooshRightHand()
*
******************************************************************************
*
*	Description:	Adds swoosh to right hand of an object instance
*
*	Inputs:			*pThis		- Ptr to object instance header
*						vPos			- World position of event
*						Value			- Event misc value
*
*	Outputs:			None
*
*****************************************************************************/
void AI_SEvent_SwooshRightHand(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	if (pThis->m_Type == I_ANIMATED)
	{
		switch(CGameObjectInstance__TypeFlag((CGameObjectInstance *)pThis))
		{
			case AI_OBJECT_CHARACTER_LONGHUNTER_BOSS:
				CFxSystem__AddObjectSwoosh(&GetApp()->m_FxSystem, (CGameObjectInstance *)pThis, &LonghuntRightHandSwooshInfo) ;
				break ;

			case AI_OBJECT_CHARACTER_HUMAN1:
				AI_SEvent_HumanSwoosh((CGameObjectInstance *)pThis) ;
				break ;
		}
	}
}


/*****************************************************************************
*
*	Function:		AI_SEvent_SwooshAllFeet()
*
******************************************************************************
*
*	Description:	Adds swoosh to all feet of an object instance
*
*	Inputs:			*pThis		- Ptr to object instance header
*						vPos			- World position of event
*						Value			- Event misc value
*
*	Outputs:			None
*
*****************************************************************************/
void AI_SEvent_SwooshAllFeet(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	if (pThis->m_Type == I_ANIMATED)
	{
		switch(CGameObjectInstance__TypeFlag((CGameObjectInstance *)pThis))
		{
			case AI_OBJECT_CHARACTER_MANTIS_BOSS:
				CFxSystem__AddObjectSwoosh(&GetApp()->m_FxSystem, (CGameObjectInstance *)pThis, &MantisLeftFrontFootSwooshInfo) ;
				CFxSystem__AddObjectSwoosh(&GetApp()->m_FxSystem, (CGameObjectInstance *)pThis, &MantisLeftRearFootSwooshInfo) ;
				CFxSystem__AddObjectSwoosh(&GetApp()->m_FxSystem, (CGameObjectInstance *)pThis, &MantisRightFrontFootSwooshInfo) ;
				CFxSystem__AddObjectSwoosh(&GetApp()->m_FxSystem, (CGameObjectInstance *)pThis, &MantisRightRearFootSwooshInfo) ;
				break ;

			case AI_OBJECT_CHARACTER_CAMPAIGNER_BOSS:
				CFxSystem__AddObjectSwoosh(&GetApp()->m_FxSystem, (CGameObjectInstance *)pThis, &CampaignerLeftFootSwooshInfo) ;
				CFxSystem__AddObjectSwoosh(&GetApp()->m_FxSystem, (CGameObjectInstance *)pThis, &CampaignerRightFootSwooshInfo) ;
				break ;

			case AI_OBJECT_CHARACTER_LONGHUNTER_BOSS:

				if (((CGameObjectInstance *)pThis)->m_pBoss->m_ModeFlags & BF_EVADE_MODE)
				{
					CFxSystem__AddObjectSwoosh(&GetApp()->m_FxSystem, (CGameObjectInstance *)pThis, &LonghuntLeftFootEvadeSwooshInfo) ;
					CFxSystem__AddObjectSwoosh(&GetApp()->m_FxSystem, (CGameObjectInstance *)pThis, &LonghuntRightFootEvadeSwooshInfo) ;
				}
				else
				{
					CFxSystem__AddObjectSwoosh(&GetApp()->m_FxSystem, (CGameObjectInstance *)pThis, &LonghuntLeftFootSwooshInfo) ;
					CFxSystem__AddObjectSwoosh(&GetApp()->m_FxSystem, (CGameObjectInstance *)pThis, &LonghuntRightFootSwooshInfo) ;
				}
				break ;

			case AI_OBJECT_CHARACTER_HUMAN1:
				AI_SEvent_HumanSwoosh((CGameObjectInstance *)pThis) ;
				break ;
		}
	}
}



/*****************************************************************************
*
*	Function:		AI_SEvent_SwooshLeftFoot()
*
******************************************************************************
*
*	Description:	Adds swoosh to left foot of an object instance
*
*	Inputs:			*pThis		- Ptr to object instance header
*						vPos			- World position of event
*						Value			- Event misc value
*
*	Outputs:			None
*
*****************************************************************************/
void AI_SEvent_SwooshLeftFoot(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	if (pThis->m_Type == I_ANIMATED)
	{
		switch(CGameObjectInstance__TypeFlag((CGameObjectInstance *)pThis))
		{
			case AI_OBJECT_CHARACTER_LONGHUNTER_BOSS:
				CFxSystem__AddObjectSwoosh(&GetApp()->m_FxSystem, (CGameObjectInstance *)pThis, &LonghuntLeftFootSwooshInfo) ;
				break ;

			case AI_OBJECT_CHARACTER_HUMAN1:
				AI_SEvent_HumanSwoosh((CGameObjectInstance *)pThis) ;
				break ;
		}
	}
}



/*****************************************************************************
*
*	Function:		AI_SEvent_SwooshRightFoot()
*
******************************************************************************
*
*	Description:	Adds swoosh to right foot of an object instance
*
*	Inputs:			*pThis		- Ptr to object instance header
*						vPos			- World position of event
*						Value			- Event misc value
*
*	Outputs:			None
*
*****************************************************************************/
void AI_SEvent_SwooshRightFoot(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	if (pThis->m_Type == I_ANIMATED)
	{
		switch(CGameObjectInstance__TypeFlag((CGameObjectInstance *)pThis))
		{
			case AI_OBJECT_CHARACTER_LONGHUNTER_BOSS:
				CFxSystem__AddObjectSwoosh(&GetApp()->m_FxSystem, (CGameObjectInstance *)pThis, &LonghuntRightFootSwooshInfo) ;
				break ;

			case AI_OBJECT_CHARACTER_HUMAN1:
				AI_SEvent_HumanSwoosh((CGameObjectInstance *)pThis) ;
				break ;
		}
	}
}



/*****************************************************************************
*
*	Function:		AI_SEvent_SwooshWeapon1()
*
******************************************************************************
*
*	Description:	Adds swoosh to weapon of an object instance
*
*	Inputs:			*pThis		- Ptr to object instance header
*						vPos			- World position of event
*						Value			- Event misc value
*
*	Outputs:			None
*
*****************************************************************************/
void AI_SEvent_SwooshWeapon1(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	if (pThis->m_Type == I_ANIMATED)
	{
		switch(CGameObjectInstance__TypeFlag((CGameObjectInstance *)pThis))
		{
			case AI_OBJECT_CHARACTER_CAMPAIGNER_BOSS:
				CFxSystem__AddObjectSwoosh(&GetApp()->m_FxSystem, (CGameObjectInstance *)pThis, &CampaignerWeaponSwooshInfo) ;
				break ;

			case AI_OBJECT_CHARACTER_HUMAN1:
				AI_SEvent_HumanSwoosh((CGameObjectInstance *)pThis) ;
				break ;
		}
	}
}


/*****************************************************************************
*
*	Function:		AI_SEvent_SwooshWeapon2()
*
******************************************************************************
*
*	Description:	Adds swoosh to weapon of an object instance
*
*	Inputs:			*pThis		- Ptr to object instance header
*						vPos			- World position of event
*						Value			- Event misc value
*
*	Outputs:			None
*
*****************************************************************************/
void AI_SEvent_SwooshWeapon2(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	if (pThis->m_Type == I_ANIMATED)
	{
		switch(CGameObjectInstance__TypeFlag((CGameObjectInstance *)pThis))
		{
			case AI_OBJECT_CHARACTER_HUMAN1:
				AI_SEvent_HumanSwoosh((CGameObjectInstance *)pThis) ;
				break ;
		}
	}
}



/*****************************************************************************
*
*	Function:		AI_SEvent_SwooshTail()
*
******************************************************************************
*
*	Description:	Adds swoosh to tail of an object instance
*
*	Inputs:			*pThis		- Ptr to object instance header
*						vPos			- World position of event
*						Value			- Event misc value
*
*	Outputs:			None
*
*****************************************************************************/
void AI_SEvent_SwooshTail(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	if (pThis->m_Type == I_ANIMATED)
	{
		switch(CGameObjectInstance__TypeFlag((CGameObjectInstance *)pThis))
		{
//			case AI_OBJECT_CHARACTER_TREX_BOSS:
//				CFxSystem__AddObjectSwoosh(&GetApp()->m_FxSystem, (CGameObjectInstance *)pThis, &RedSwooshInfo, &TRexTailSwooshNodeInfo) ;
//				break ;

			case AI_OBJECT_CHARACTER_HUMAN1:
				AI_SEvent_HumanSwoosh((CGameObjectInstance *)pThis) ;
				break ;
		}
	}
}




/*****************************************************************************
*
*	Function:		AI_SEvent_SwooshLeftToes()
*
******************************************************************************
*
*	Description:	Adds swoosh to left toes of an object instance
*
*	Inputs:			*pThis		- Ptr to object instance header
*						vPos			- World position of event
*						Value			- Event misc value
*
*	Outputs:			None
*
*****************************************************************************/
void AI_SEvent_SwooshLeftToes(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	if (pThis->m_Type == I_ANIMATED)
	{
		switch(CGameObjectInstance__TypeFlag((CGameObjectInstance *)pThis))
		{
//			case AI_OBJECT_CHARACTER_LONGHUNTER_BOSS:
//				CFxSystem__AddObjectSwoosh(&GetApp()->m_FxSystem, (CGameObjectInstance *)pThis, &LonghuntLeftToesSwooshInfo) ;
//				break ;

			case AI_OBJECT_CHARACTER_HUMAN1:
				AI_SEvent_HumanSwoosh((CGameObjectInstance *)pThis) ;
				break ;
		}
	}
}



/*****************************************************************************
*
*	Function:		AI_SEvent_SwooshRightToes()
*
******************************************************************************
*
*	Description:	Adds swoosh to right toes of an object instance
*
*	Inputs:			*pThis		- Ptr to object instance header
*						vPos			- World position of event
*						Value			- Event misc value
*
*	Outputs:			None
*
*****************************************************************************/
void AI_SEvent_SwooshRightToes(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	if (pThis->m_Type == I_ANIMATED)
	{
		switch(CGameObjectInstance__TypeFlag((CGameObjectInstance *)pThis))
		{
//			case AI_OBJECT_CHARACTER_LONGHUNTER_BOSS:
//				CFxSystem__AddObjectSwoosh(&GetApp()->m_FxSystem, (CGameObjectInstance *)pThis, &LonghuntRightToesSwooshInfo) ;
//				break ;

			case AI_OBJECT_CHARACTER_HUMAN1:
				AI_SEvent_HumanSwoosh((CGameObjectInstance *)pThis) ;
				break ;
		}
	}
}








/*****************************************************************************
*
*
*
*	SPECIAL EFFECT COLOR GLOWING CODE
*
*
*
*****************************************************************************/


/*****************************************************************************
*
*	Function:		CSpecialFx__Construct()
*
******************************************************************************
*
*	Description:	Initialises fx vars
*
*	Inputs:			*pThis	-	Ptr to special fx vars
*
*	Outputs:			None
*
*****************************************************************************/
void CSpecialFx__Construct(CSpecialFx *pThis, CGameObjectInstance *pInst)
{
	pThis->m_pInst = pInst ;
	pThis->m_Command = SPECIALFX_COMMAND_IDLE ;
	pThis->m_pScript = pThis->m_pScriptStart = NULL ;
	pThis->m_Trans = MAX_OPAQ ;
	pThis->m_Color = MIN_COL ;
	pThis->m_EnvColor.Red = 0 ;
	pThis->m_EnvColor.Green = 0 ;
	pThis->m_EnvColor.Blue = 0 ;
}


/*****************************************************************************
*
*	Function:		CSpecialFx__StartScript()
*
******************************************************************************
*
*	Description:	Starts special fx script into action
*
*	Inputs:			*pThis	-	Ptr to special fx vars
*						*pScript	-	Ptr to fx script
*
*	Outputs:			None
*
*****************************************************************************/
void CSpecialFx__StartScript(CSpecialFx *pThis, FLOAT *pScript)
{
	pThis->m_Command = SPECIALFX_COMMAND_IDLE ;
	pThis->m_pScript = pThis->m_pScriptStart = pScript ;
}



/*****************************************************************************
*
*	Function:		CSpecialFx__Update()
*
******************************************************************************
*
*	Description:	Updates special fx vars and read script commands
*
*	Inputs:			*pThis	-	Ptr to special fx vars
*
*	Outputs:			None
*
*****************************************************************************/
void CSpecialFx__Update(CSpecialFx *pThis)
{
	BOOL			ReadScript ;
	FLOAT			r,g,b, v,t,a ;

	// Init vars
	ReadScript = FALSE ;

	// Process current command
	switch(pThis->m_Command)
	{
		case SPECIALFX_COMMAND_IDLE:

			// Start reading script?
			if (pThis->m_pScriptStart)
				ReadScript = TRUE ;
			break ;

		case SPECIALFX_COMMAND_WAIT:

			// Done?
			pThis->m_Time -= frame_increment ;
			if (pThis->m_Time < 0)
				ReadScript = TRUE ;
			break ;


		case SPECIALFX_COMMAND_FADE_TRANS:

			pThis->m_Time += frame_increment ;
			if (pThis->m_Time >= pThis->m_TransFadeTime)
			{
				pThis->m_Time = pThis->m_TransFadeTime ;
				ReadScript = TRUE ;
			}

			b = pThis->m_Time / pThis->m_TransFadeTime ;
			a = 1.0 - b ;

			pThis->m_Trans = (a * pThis->m_StartTrans) +
								  (b * pThis->m_FinalTrans) ;

			break ;


		case SPECIALFX_COMMAND_FADE_COLOR:

			pThis->m_Time += frame_increment ;
			if (pThis->m_Time >= pThis->m_ColorFadeTime)
			{
				pThis->m_Time = pThis->m_ColorFadeTime ;
				ReadScript = TRUE ;
			}

			b = pThis->m_Time / pThis->m_ColorFadeTime ;
			a = 1.0 - b ;

			pThis->m_EnvColor.Red = (a * pThis->m_StartEnvColor.Red) +
											(b * pThis->m_FinalEnvColor.Red) ;

			pThis->m_EnvColor.Green = (a * pThis->m_StartEnvColor.Green) +
											  (b * pThis->m_FinalEnvColor.Green) ;

			pThis->m_EnvColor.Blue = (a * pThis->m_StartEnvColor.Blue) +
											 (b * pThis->m_FinalEnvColor.Blue) ;

			pThis->m_Color = (a * pThis->m_StartColor) +
								  (b * pThis->m_FinalColor) ;

			break ;
	}


	// Process script
	while(ReadScript)
	{
		// Read next command
		pThis->m_Command = *pThis->m_pScript++ ;
		ReadScript = FALSE ;

		switch(pThis->m_Command)
		{
			case SPECIALFX_COMMAND_IDLE:
				pThis->m_pScript = NULL ;
				pThis->m_pScriptStart = NULL ;
				break ;

			case SPECIALFX_COMMAND_WAIT:
				t = *pThis->m_pScript++ ;

				pThis->m_Time = t ;
				break ;

			case SPECIALFX_COMMAND_RANDOM_WAIT:
				a = *pThis->m_pScript++ ;
				b = *pThis->m_pScript++ ;

				pThis->m_Command = SPECIALFX_COMMAND_WAIT ;
				pThis->m_Time = RandomRangeFLOAT(a,b) ;
				break ;

			case SPECIALFX_COMMAND_SET_TRANS:
				v = *pThis->m_pScript++ ;

				pThis->m_Trans = v ;
				ReadScript = TRUE ;
				break ;

			case SPECIALFX_COMMAND_FADE_TRANS:
				v = *pThis->m_pScript++ ;
				t = *pThis->m_pScript++ ;

				pThis->m_StartTrans = pThis->m_Trans ;
				pThis->m_FinalTrans = v ;
				pThis->m_TransFadeTime = t ;
				pThis->m_Time = 0 ;
				break ;

			case SPECIALFX_COMMAND_SET_COLOR:
				r = *pThis->m_pScript++ ;
				g = *pThis->m_pScript++ ;
				b = *pThis->m_pScript++ ;
				v = *pThis->m_pScript++ ;

				pThis->m_EnvColor.Red = r ;
				pThis->m_EnvColor.Green = g ;
				pThis->m_EnvColor.Blue = b ;
				pThis->m_Color = v ;
				ReadScript = TRUE ;
				break ;

			case SPECIALFX_COMMAND_FADE_COLOR:
				r = *pThis->m_pScript++ ;
				g = *pThis->m_pScript++ ;
				b = *pThis->m_pScript++ ;
				v = *pThis->m_pScript++ ;
				t = *pThis->m_pScript++ ;

				pThis->m_StartEnvColor.Red = pThis->m_EnvColor.Red ;
				pThis->m_StartEnvColor.Green = pThis->m_EnvColor.Green ;
				pThis->m_StartEnvColor.Blue = pThis->m_EnvColor.Blue ;
				pThis->m_StartColor = pThis->m_Color ;

				pThis->m_FinalEnvColor.Red = r ;
				pThis->m_FinalEnvColor.Green = g ;
				pThis->m_FinalEnvColor.Blue = b ;
				pThis->m_FinalColor = v ;

				pThis->m_ColorFadeTime = t ;
				pThis->m_Time = 0 ;

				break ;

			case SPECIALFX_COMMAND_PLAY_SOUND:
				v = *pThis->m_pScript++ ;
				AI_SEvent_GeneralSound(&pThis->m_pInst->ah.ih, pThis->m_pInst->ah.ih.m_vPos, v) ;
				ReadScript = TRUE ;
				break ;

			case SPECIALFX_COMMAND_REPEAT:
				pThis->m_pScript = pThis->m_pScriptStart ;
				ReadScript = TRUE ;
				break ;

		}
	}
}



/*****************************************************************************
*
*	Function:		CSpecialFx__SetDrawFxVars()
*
******************************************************************************
*
*	Description:	Copies special fx values to fx values
*
*	Inputs:			*pThis	-	Ptr to special fx vars
*
*	Outputs:			None
*
*****************************************************************************/
void CSpecialFx__SetDrawFxVars(CSpecialFx *pThis)
{

	// Copy draw fx stuff
	if (pThis->m_Color != MIN_COL)
	{
		fx_color[0] = pThis->m_EnvColor.Red ;
		fx_color[1] = pThis->m_EnvColor.Green ;
		fx_color[2] = pThis->m_EnvColor.Blue ;
	}
	else
	{
		fx_color[0] = 0 ;
		fx_color[1] = 0 ;
		fx_color[2] = 0 ;
	}


	fx_color[3] = pThis->m_Color ;
	fx_color[4] = pThis->m_Trans ;
}



/*****************************************************************************
*
*	Function:		CSpecialFx__AddToDrawFxVars()
*
******************************************************************************
*
*	Description:	Adds special fx values to current fx values
*
*	Inputs:			*pThis	-	Ptr to special fx vars
*
*	Outputs:			None
*
*****************************************************************************/
void CSpecialFx__AddToDrawFxVars(CSpecialFx *pThis)
{
	// Copy draw fx stuff
	if (pThis->m_Color != MIN_COL)
	{
		fx_color[0] += pThis->m_EnvColor.Red ;
		fx_color[1] += pThis->m_EnvColor.Green ;
		fx_color[2] += pThis->m_EnvColor.Blue ;
		fx_color[3] += pThis->m_Color ;
	}

	fx_color[4] += pThis->m_Trans ;
}



/*****************************************************************************
*
*	Function:		RangeCheckFxVar()
*
******************************************************************************
*
*	Description:	Makes sure variable is >= 0 and <= 255
*
*	Inputs:			*pVar		-	Ptr to variable for range check
*
*	Outputs:			None
*
*****************************************************************************/
void RangeCheckFxVar(FLOAT *pVar)
{
	if (*pVar < 0)
		*pVar = 0 ;
	else
	if (*pVar > 255)
		*pVar = 255 ;
}



/*****************************************************************************
*
*	Function:		PrepareDrawFxVars()
*
******************************************************************************
*
*	Description:	Range checks all fx vars and sets up the correct draw fx mode
*
*	Inputs:			None
*
*	Outputs:			None
*
*****************************************************************************/
void PrepareDrawFxVars(void)
{
	FLOAT	Trans, Color ;

	// Range check all values
	RangeCheckFxVar(&fx_color[0]) ;	// Env.Red
	RangeCheckFxVar(&fx_color[1]) ;	// Env.Green
	RangeCheckFxVar(&fx_color[2]) ;	// Env.Blue
	RangeCheckFxVar(&fx_color[3]) ;	// Color%
	RangeCheckFxVar(&fx_color[4]) ;	// Trans%

	Color = fx_color[3] ;
	Trans = fx_color[4] ;

	// Set draw fx mode
	if ((Trans == MAX_OPAQ) && (Color == MIN_COL))
		fx_mode = FXMODE_NONE ;
	else
	if ((Trans != MAX_OPAQ) && (Color == MIN_COL))
	{
		fx_mode = FXMODE_TOTRANSPARENT ;
		fx_color[3] = fx_color[4] ;	// [3] is trans amount!
	}
	else
	if ((Trans == MAX_OPAQ) && (Color != MIN_COL))
		fx_mode = FXMODE_TOCOLOR ;
	else
	if ((Trans != MAX_OPAQ) && (Color != MIN_COL))
		fx_mode = FXMODE_TOCOLORANDTRANSPARENT ;
}





//////////////////////////////////////////////////////////////////////////////
//
//	Humans
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//	White bone weapon swoosh
//////////////////////////////////////////////////////////////////////////////
CSwooshEdgeVertexInfo BoneSwooshVertices[] =
{
	// Pos							Color					Alpha
	{{{0,		1,		0},  		{255,	255, 255},	MAX_OPAQ/4},	// Start
	 {{0,		1,		3},		{140,	140, 140},	MIN_OPAQ}},		// end

	{{{0,		1,		6},		{140,	140, 100},	MAX_OPAQ/16},	// Start
	 {{0,		1,		3},		{40,	40,	40},	MIN_OPAQ}},		// end
} ;

CSwooshInfo	BoneSwooshInfo =
{
	SECONDS_TO_FRAMES(0.4),		// Total time on screen
	SECONDS_TO_FRAMES(0.2),		// Total time before fade swoosh out
	SECONDS_TO_FRAMES(0.2),		// Total edge life time
	17,							  	// Object node
	SCALING_FACTOR,			  	// Scale
	NULL,							  	// General flags
	2,								  	// No of edge vertices
	BoneSwooshVertices			// Edge vertices
} ;


//////////////////////////////////////////////////////////////////////////////
//	Silver sword weapon swoosh
//////////////////////////////////////////////////////////////////////////////
CSwooshEdgeVertexInfo SwordSwooshVertices[] =
{
	// Pos							Color					Alpha
	{{{0,		1,		0},  		{255,	255, 255},	MAX_OPAQ/4},	// Start
	 {{0,		1,		2.5},		{140,	140, 140},	MIN_OPAQ}},		// end

	{{{0,		1,		5},		{140,	140, 100},	MAX_OPAQ/16},	// Start
	 {{0,		1,		2.5},		{40,	40,	40},	MIN_OPAQ}},		// end
} ;

CSwooshInfo	SwordSwooshInfo =
{
	SECONDS_TO_FRAMES(0.75*0.75),	// Total time on screen
	SECONDS_TO_FRAMES(0.5*0.75),	// Total time before fade swoosh out
	SECONDS_TO_FRAMES(0.25*0.75),	// Total edge life time
	17,								 	// Object node
	SCALING_FACTOR,					// Scale
	NULL,									// General flags
	2,										// No of edge vertices
	SwordSwooshVertices				// Edge vertices
} ;



//////////////////////////////////////////////////////////////////////////////
//	Silver mace weapon swoosh
//////////////////////////////////////////////////////////////////////////////
CSwooshEdgeVertexInfo MaceSwooshVertices[] =
{
	// Pos							Color					Alpha
	{{{0,		1,		0},  		{255,	255, 255},	MAX_OPAQ/4},	// Start
	 {{0,		1,		3.25},	{140,	140, 140},	MIN_OPAQ}},		// end

	{{{0,		1,		6.5},		{140,	140, 100},	MAX_OPAQ/16},	// Start
	 {{0,		1,		3.25},	{40,	40,	40},	MIN_OPAQ}},		// end
} ;

CSwooshInfo	MaceSwooshInfo =
{
	SECONDS_TO_FRAMES(0.75*0.75),	// Total time on screen
	SECONDS_TO_FRAMES(0.5*0.75),	// Total time before fade swoosh out
	SECONDS_TO_FRAMES(0.25*0.75),	// Total edge life time
	17,								 	// Object node
	SCALING_FACTOR,					// Scale
	NULL,									// General flags
	2,										// No of edge vertices
	MaceSwooshVertices				// Edge vertices
} ;


//////////////////////////////////////////////////////////////////////////////
//	Silver sceptre weapon swoosh
//////////////////////////////////////////////////////////////////////////////
CSwooshEdgeVertexInfo SceptreSwooshVertices[] =
{
	// Pos							Color					Alpha
	{{{0,		1,		0},  		{255,	255, 255},	MAX_OPAQ/4},	// Start
	 {{0,		1,		3.5},		{140,	140, 140},	MIN_OPAQ}},		// end

	{{{0,		1,		7},		{140,	140, 100},	MAX_OPAQ/16},	// Start
	 {{0,		1,		3.5},		{40,	40,	40},	MIN_OPAQ}},		// end
} ;

CSwooshInfo	SceptreSwooshInfo =
{
	SECONDS_TO_FRAMES(0.75*0.75),	// Total time on screen
	SECONDS_TO_FRAMES(0.5*0.75),	// Total time before fade swoosh out
	SECONDS_TO_FRAMES(0.25*0.75),	// Total edge life time
	17,								 	// Object node
	SCALING_FACTOR,					// Scale
	NULL,									// General flags
	2,										// No of edge vertices
	SceptreSwooshVertices 			// Edge vertices
} ;



//////////////////////////////////////////////////////////////////////////////
//	Silver pole1 weapon swoosh
//////////////////////////////////////////////////////////////////////////////
CSwooshEdgeVertexInfo Pole1SwooshVertices[] =
{
	// Pos							Color					Alpha
	{{{0,	14.5,		0},  		{255,	255, 255},	MAX_OPAQ/4},	// Start
	 {{0,	11.25,	0},		{140,	140, 140},	MIN_OPAQ}},		// end

	{{{0,	8,			0},		{140,	140, 100},	MAX_OPAQ/16},	// Start
	 {{0,	11.25,	0},		{40,	40,	40},	MIN_OPAQ}},		// end
} ;

CSwooshInfo	Pole1SwooshInfo =
{
	SECONDS_TO_FRAMES(0.75*0.75),	// Total time on screen
	SECONDS_TO_FRAMES(0.5*0.75),	// Total time before fade swoosh out
	SECONDS_TO_FRAMES(0.25*0.75),	// Total edge life time
	17,								 	// Object node
	SCALING_FACTOR,					// Scale
	NULL,									// General flags
	2,										// No of edge vertices
	Pole1SwooshVertices 				// Edge vertices
} ;


//////////////////////////////////////////////////////////////////////////////
//	Silver pole2a weapon swoosh
//////////////////////////////////////////////////////////////////////////////
CSwooshEdgeVertexInfo Pole2aSwooshVertices[] =
{
	// Pos							Color					Alpha
	{{{0,	14.5,	1},  		{255,	255, 255},	MAX_OPAQ/4},	// Start
	 {{0,	11.5,	1},		{140,	140, 140},	MIN_OPAQ}},		// end

	{{{0,	8.5, 	1},		{140,	140, 100},	MAX_OPAQ/16},	// Start
	 {{0,	11.5,	1},		{40,	40,	40},	MIN_OPAQ}},		// end
} ;

CSwooshInfo	Pole2aSwooshInfo =
{
	SECONDS_TO_FRAMES(0.75*0.75),	// Total time on screen
	SECONDS_TO_FRAMES(0.5*0.75),	// Total time before fade swoosh out
	SECONDS_TO_FRAMES(0.25*0.75),	// Total edge life time
	17,								 	// Object node
	SCALING_FACTOR,					// Scale
	NULL,									// General flags
	2,										// No of edge vertices
	Pole2aSwooshVertices				// Edge vertices
} ;



//////////////////////////////////////////////////////////////////////////////
//	Silver pole2b weapon swoosh
//////////////////////////////////////////////////////////////////////////////
CSwooshEdgeVertexInfo Pole2bSwooshVertices[] =
{
	// Pos							Color					Alpha
	{{{0,	-4.5,	-1}, 		{255,	255, 255},	MAX_OPAQ/4},	// Start
	 {{0,	-2.5,	-1},		{140,	140, 140},	MIN_OPAQ}},		// end

	{{{0,	-0.5, -1},		{140,	140, 100},	MAX_OPAQ/16},	// Start
	 {{0,	-2.5,	-1},		{40,	40,	40},	MIN_OPAQ}},		// end
} ;

CSwooshInfo	Pole2bSwooshInfo =
{
	SECONDS_TO_FRAMES(0.75*0.75),	// Total time on screen
	SECONDS_TO_FRAMES(0.5*0.75),	// Total time before fade swoosh out
	SECONDS_TO_FRAMES(0.25*0.75),	// Total edge life time
	17,								 	// Object node
	SCALING_FACTOR,					// Scale
	NULL,									// General flags
	2,										// No of edge vertices
	Pole2bSwooshVertices				// Edge vertices
} ;



//////////////////////////////////////////////////////////////////////////////
//	Black shotgun weapon swoosh
//////////////////////////////////////////////////////////////////////////////
CSwooshEdgeVertexInfo ShotgunSwooshVertices[] =
{
	// Pos									Color					Alpha
	{{{0,			0,			-0.5},	{0,	0, 0},	MAX_OPAQ/3},	// Start
	 {{-0.15,	4.25,		0.4},		{0,	0, 0},	MIN_OPAQ}},		// end

	{{{-0.3,		8.5,		1.3},		{10,	10, 10},	MAX_OPAQ/3},	// Start
	 {{-0.15,	4.25,		0.4},		{0,	0,	0},	MIN_OPAQ}},		// end
} ;

CSwooshInfo	ShotgunSwooshInfo =
{
	SECONDS_TO_FRAMES(0.75*0.75),	// Total time on screen
	SECONDS_TO_FRAMES(0.5*0.75),	// Total time before fade swoosh out
	SECONDS_TO_FRAMES(0.25*0.75),	// Total edge life time
	17,								 	// Object node
	SCALING_FACTOR,					// Scale
	NULL,									// General flags
	2,										// No of edge vertices
	ShotgunSwooshVertices			// Edge vertices
} ;



//////////////////////////////////////////////////////////////////////////////
//	Black rifle weapon swoosh
//////////////////////////////////////////////////////////////////////////////
CSwooshEdgeVertexInfo RifleSwooshVertices[] =
{
	// Pos									Color					Alpha
	{{{0,			0,			-0.5},	{0,	0, 0},	MAX_OPAQ/3},	// Start
	 {{-0.15,	4.5,		0.4},		{0,	0, 0},	MIN_OPAQ}},		// end

	{{{-0.3,		9,			1.3},		{10,	10, 10},	MAX_OPAQ/3},	// Start
	 {{-0.15,	4.5,		0.4},		{0,	0,	0},	MIN_OPAQ}},		// end
} ;

CSwooshInfo	RifleSwooshInfo =
{
	SECONDS_TO_FRAMES(0.75*0.75),	// Total time on screen
	SECONDS_TO_FRAMES(0.5*0.75),	// Total time before fade swoosh out
	SECONDS_TO_FRAMES(0.25*0.75),	// Total edge life time
	17,								 	// Object node
	SCALING_FACTOR,					// Scale
	NULL,									// General flags
	2,										// No of edge vertices
	RifleSwooshVertices				// Edge vertices
} ;



//////////////////////////////////////////////////////////////////////////////
//	Black pistol weapon swoosh
//////////////////////////////////////////////////////////////////////////////
CSwooshEdgeVertexInfo PistolSwooshVertices[] =
{
	// Pos									Color					Alpha
	{{{-0.5,		0.5,		1},		{0,	0, 0},	MAX_OPAQ/3},	// Start
	 {{-0.5,		1.5,		1},		{0,	0, 0},	MIN_OPAQ}},		// end

	{{{-0.5,		2.5,		1},		{10,	10, 10},	MAX_OPAQ/3},	// Start
	 {{-0.5,		1.5,		1},		{0,	0,	0},	MIN_OPAQ}},		// end
} ;

CSwooshInfo	PistolSwooshInfo =
{
	SECONDS_TO_FRAMES(0.75*0.75),	// Total time on screen
	SECONDS_TO_FRAMES(0.5*0.75),	// Total time before fade swoosh out
	SECONDS_TO_FRAMES(0.25*0.75),	// Total edge life time
	17,								 	// Object node
	SCALING_FACTOR,					// Scale
	NULL,									// General flags
	2,										// No of edge vertices
	PistolSwooshVertices				// Edge vertices
} ;



//////////////////////////////////////////////////////////////////////////////
//	Brown blow gun weapon swoosh
//////////////////////////////////////////////////////////////////////////////
CSwooshEdgeVertexInfo BlowgunSwooshVertices[] =
{
	// Pos									Color					Alpha
	{{{0,		1,		0},		{10,	0, 0},	MAX_OPAQ/3},	// Start
	 {{0,		1,		3},		{5,	0, 0},	MIN_OPAQ}},		// end

	{{{0,		1,		6},		{20,	10, 10},	MAX_OPAQ/3},	// Start
	 {{0,		1,		3},		{10,	0,	0},	MIN_OPAQ}},		// end
} ;

CSwooshInfo	BlowgunSwooshInfo =
{
	SECONDS_TO_FRAMES(0.75*0.75),	// Total time on screen
	SECONDS_TO_FRAMES(0.5*0.75),	// Total time before fade swoosh out
	SECONDS_TO_FRAMES(0.25*0.75),	// Total edge life time
	17,								 	// Object node
	SCALING_FACTOR,					// Scale
	NULL,									// General flags
	2,										// No of edge vertices
	BlowgunSwooshVertices				// Edge vertices
} ;


CSwooshInfo	*pHumanSwooshTable[] =
{
	&BoneSwooshInfo,
	&SwordSwooshInfo,
	&MaceSwooshInfo,
	&SceptreSwooshInfo,
	&Pole1SwooshInfo,
	&Pole2aSwooshInfo,
	&Pole2bSwooshInfo,
	&ShotgunSwooshInfo,
	&RifleSwooshInfo,
	&PistolSwooshInfo,
	&BlowgunSwooshInfo
} ;

enum Human1Swooshes
{
	HS_BONE,
	HS_SWORD,
	HS_MACE,
	HS_SCEPTRE,
	HS_POLE1,
	HS_POLE2A,
	HS_POLE2B,
	HS_SHOTGUN,
	HS_RIFLE,
	HS_PISTOL,
	HS_BLOW_GUN,
	HS_END
} ;

void AI_SEvent_HumanSwoosh(CGameObjectInstance *pThis)
{
	int	type = AI_GetEA(pThis)->m_AttackStyle ;
	CFxSystem	*pFxSystem = &GetApp()->m_FxSystem ;
	INT32	Swoosh1 = -1 ;
	INT32	Swoosh2 = -1 ;

//	enemy_speed_scaler = 0.25 ;

	switch (type)
	{
		// sword, bone, mace, axe
		case 0:
		case 1:
			switch(AI_GetDyn(pThis)->m_WeaponModel)
			{
				// bone
				case 1:
					Swoosh1 = HS_BONE ;
					break ;

				// sword
				case 14:
					Swoosh1 = HS_SWORD ;
					break ;

				// mace
				case 2:
					Swoosh1 = HS_MACE ;
					break ;

				// axe not done yet

			}
			break ;

		// mace
		case 2:
			Swoosh1 = HS_MACE ;
			break ;

		// sceptre
		case 22:
		case 23:
		case 24:
			Swoosh1 = HS_SCEPTRE ;
			break ;


		// polearm1, polearm2
		case 3:
		case 21:
			switch(AI_GetDyn(pThis)->m_WeaponModel)
			{
				// pole1 - use metal swoosh
				case 3:
					Swoosh1 = HS_POLE1 ;
					break ;

				// pole2 - use metal swoosh
				case 16:
					Swoosh1 = HS_POLE2A ;
					Swoosh2 = HS_POLE2B ;
					break ;
			}

			break ;

		// shotgun - use dark swoosh!
		case 4:
		case 5:
		case 14:
		case 15:
			Swoosh1 = HS_SHOTGUN ;
			break ;

		// rifles
		case 6:
		case 7:
		case 8:
			Swoosh1 = HS_RIFLE ;
			break ;

		// sword
		case 9:
		case 18:
		case 25:
			Swoosh1 = HS_SWORD ;
			break ;

		// pistol
		case 10:
		case 11:
		case 12:
		case 13:
			Swoosh1 = HS_PISTOL ;
			break ;

		// rifles
		case 16:
		case 17:
			Swoosh1 = HS_RIFLE ;
			break ;

		// blow guns - brown
		case 19:
		case 20:
			Swoosh1 = HS_BLOW_GUN ;
			break ;
	}

	// Start off swooshes
	if (Swoosh1 >= 0)
		CFxSystem__AddObjectSwoosh(pFxSystem, pThis, pHumanSwooshTable[Swoosh1]) ;

	if (Swoosh2 >= 0)
		CFxSystem__AddObjectSwoosh(pFxSystem, pThis, pHumanSwooshTable[Swoosh2]) ;
}





/*****************************************************************************
*
*
*
*	TIMER FX CODE
*
*
*
*****************************************************************************/
void CFxSystem__UpdateAllTimerFx(CFxSystem *pThis)
{
	CTimerFx *pFx, *pNextFx ;

	// Search through all of list
	pFx = (CTimerFx *)pThis->m_TimerFxList.m_ActiveList.m_pHead ;
	while(pFx)
	{
		// Keep ptr to next incase of deletion
		pNextFx = (CTimerFx *)pFx->m_Node.m_pNext ;
		
		// Call function?
		pFx->m_Time -= frame_increment ;
		if (pFx->m_Time <= 0)
		{
			pFx->m_Time += pFx->m_Spacing ;
			if (pFx->m_Time < 0)
				pFx->m_Time = 0 ;
			pFx->m_pFunction(pFx) ;

			// Last one?
			if (--pFx->m_Count <= 0)
				CDynamicList__DeallocateNode(&pThis->m_TimerFxList, (CNode *)pFx) ;
		}

		// Goto next one
		pFx = pNextFx ;
	}
}


// Add an fx
CTimerFx *CFxSystem__AddTimerFx(CFxSystem *pThis,
										  TimerFxFunction pFunction,	
										  CGameObjectInstance *pInst, 
										  INT32 Total, 
										  FLOAT Spacing)
{
	CTimerFx *pFx ;

	// Try allocate fx
	pFx = (CTimerFx *)CDynamicList__AllocateNode(&pThis->m_TimerFxList) ;
	if (pFx)
	{
		// Setup vars
		pFx->m_pFunction = (void *)pFunction ;
		pFx->m_pInst = pInst ;
		pFx->m_Time = 0 ;
		pFx->m_Spacing = Spacing ;
		pFx->m_Count = Total ;
	}

	// Return result
	return pFx ;
}

