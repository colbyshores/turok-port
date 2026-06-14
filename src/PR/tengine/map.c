// map.c

#include "cppu64.h"
#include "os_internal.h"

#include "tengine.h"
#include "dlists.h"
#include "map.h"
#include "tmove.h"
#include "scaling.h"
#include "regtype.h"
#include "cartdir.h"



/////////////////////////////////////////////////////////////////////////////
// Constants
/////////////////////////////////////////////////////////////////////////////
#define ENABLE_MAP			1		// Set to 1 for final game! (0 for recording demos)



/////////////////////////////////////////////////////////////////////////////
// Variables
/////////////////////////////////////////////////////////////////////////////

#if ENABLE_MAP

void SetupMapVertices();
void DoDrawMap(Gfx **ppDLP);

#define	NOMAP_CLIPX		64
#define	NOMAP_CLIPY		(NOMAP_CLIPX*SCREEN_HT/SCREEN_WD)



#ifndef KANJI
#define MAX_MAP_VTXS 512
#define MAX_VISIBLE_ENEMIES 10
#else
#define MAX_MAP_VTXS 500
#define MAX_VISIBLE_ENEMIES 8
#endif

Vtx	map_vtxs[2][MAX_MAP_VTXS];		// map
Vtx	ply_vtxs[2][3];					// player
Vtx	enm_vtxs[2][MAX_VISIBLE_ENEMIES*3];				// enemies
Vtx	cmp_vtxs[2][8];					// compass
Mtx	mtx_map;

u16	Connections[MAX_MAP_VTXS+MAX_MAP_VTXS] ;



#define		REVEAL_DIST			(40*SCALING_FACTOR)

#endif



void DoRevealMap(CGameRegion *pRegion)
{
#if ENABLE_MAP

	// limit recursion if we're running out of stack space
	if ( (DWORD)&pRegion < ((DWORD)main_thread_stack + MAIN_STACKSIZE/10) )
		return;

	if (!(pRegion->m_wFlags & REGFLAG_REGIONENTERED))
	{
		CScene__SetRegionFlag(&GetApp()->m_Scene,
									 CScene__GetRegionIndex(&GetApp()->m_Scene, pRegion),
									 TRUE);
	}

	pRegion->m_wFlags |= REGFLAG_REGIONENTERED | REGFLAG_RECURSE;

	if (		((SQR(pRegion->m_pCorners[0]->m_vCorner.x - GetApp()->m_XPos) + SQR(pRegion->m_pCorners[0]->m_vCorner.z - GetApp()->m_ZPos)) < SQR(REVEAL_DIST))
			|| ((SQR(pRegion->m_pCorners[1]->m_vCorner.x - GetApp()->m_XPos) + SQR(pRegion->m_pCorners[1]->m_vCorner.z - GetApp()->m_ZPos)) < SQR(REVEAL_DIST))
			|| ((SQR(pRegion->m_pCorners[2]->m_vCorner.x - GetApp()->m_XPos) + SQR(pRegion->m_pCorners[2]->m_vCorner.z - GetApp()->m_ZPos)) < SQR(REVEAL_DIST)) )
	{
		if (pRegion->m_pNeighbors[0])
			if (!(pRegion->m_pNeighbors[0]->m_wFlags & REGFLAG_RECURSE))
				if (!(pRegion->m_pNeighbors[0]->m_wFlags & REGFLAG_DOOR) || (pRegion->m_pNeighbors[0]->m_wFlags & REGFLAG_OPENDOOR))
					DoRevealMap(pRegion->m_pNeighbors[0]);

		if (pRegion->m_pNeighbors[1])
			if (!(pRegion->m_pNeighbors[1]->m_wFlags & REGFLAG_RECURSE))
				if (!(pRegion->m_pNeighbors[1]->m_wFlags & REGFLAG_DOOR) || (pRegion->m_pNeighbors[1]->m_wFlags & REGFLAG_OPENDOOR))
					DoRevealMap(pRegion->m_pNeighbors[1]);

		if (pRegion->m_pNeighbors[2])
			if (!(pRegion->m_pNeighbors[2]->m_wFlags & REGFLAG_RECURSE))
				if (!(pRegion->m_pNeighbors[2]->m_wFlags & REGFLAG_DOOR) || (pRegion->m_pNeighbors[2]->m_wFlags & REGFLAG_OPENDOOR))
					DoRevealMap(pRegion->m_pNeighbors[2]);
	}
#endif
}


void ClearRecurseFlags(CGameRegion *pRegion)
{
#if ENABLE_MAP

	pRegion->m_wFlags &= ~REGFLAG_RECURSE;

	if (pRegion->m_pNeighbors[0])
		if (pRegion->m_pNeighbors[0]->m_wFlags & REGFLAG_RECURSE)
			ClearRecurseFlags(pRegion->m_pNeighbors[0]);

	if (pRegion->m_pNeighbors[1])
		if (pRegion->m_pNeighbors[1]->m_wFlags & REGFLAG_RECURSE)
			ClearRecurseFlags(pRegion->m_pNeighbors[1]);

	if (pRegion->m_pNeighbors[2])
		if (pRegion->m_pNeighbors[2]->m_wFlags & REGFLAG_RECURSE)
			ClearRecurseFlags(pRegion->m_pNeighbors[2]);

#endif
}

#define REVEAL_EVERY		5
void RevealMap()
{
#if ENABLE_MAP

	CGameObjectInstance *pPlayer;
//	OSTime	startTime;
	static int occasion = REVEAL_EVERY;

	if (--occasion)
		return;
	else
		occasion = REVEAL_EVERY;

	pPlayer = CEngineApp__GetPlayer(GetApp());
	if (!pPlayer)
		return;

	if (!pPlayer->ah.ih.m_pCurrentRegion)
		return;
	
	DoRevealMap(pPlayer->ah.ih.m_pCurrentRegion);
	ClearRecurseFlags(pPlayer->ah.ih.m_pCurrentRegion);

#endif
}

void CEngineApp__SetupMap(CEngineApp *pThis)
{
#if ENABLE_MAP

  guOrtho(&mtx_map,
	  0, SCREEN_WD-1,
	  0, SCREEN_HT-1,
	  -200, 200,
	  32);		// precision scaler

  SetupMapVertices();

#endif
}

void SetupMapVertices()
{
#if ENABLE_MAP

	int e, i;


	for (e=0; e<2; e++)
	{
		for (i=0; i<MAX_MAP_VTXS; i++)
		{
			map_vtxs[e][i].v.ob[0] = 0;
			map_vtxs[e][i].v.ob[1] = 0;
			map_vtxs[e][i].v.ob[2] = 0;
			map_vtxs[e][i].v.flag = 0;
			map_vtxs[e][i].v.tc[0] = 0;
			map_vtxs[e][i].v.tc[1] = 0;
			map_vtxs[e][i].v.cn[0] = 0;
			map_vtxs[e][i].v.cn[1] = 200;
			map_vtxs[e][i].v.cn[2] = 0;
			map_vtxs[e][i].v.cn[3] = 255;
		}

		for (i=0; i<3; i++)
		{
			ply_vtxs[e][i].v.ob[0] = 0;
			ply_vtxs[e][i].v.ob[1] = 0;
			ply_vtxs[e][i].v.ob[2] = 0;
			ply_vtxs[e][i].v.flag = 0;
			ply_vtxs[e][i].v.tc[0] = 0;
			ply_vtxs[e][i].v.tc[1] = 0;
			ply_vtxs[e][i].v.cn[0] = 200;
			ply_vtxs[e][i].v.cn[1] = 200;
			ply_vtxs[e][i].v.cn[2] = 0;
			ply_vtxs[e][i].v.cn[3] = 255;
		}

		for (i=0; i<MAX_VISIBLE_ENEMIES*3; i++)
		{
			enm_vtxs[e][i].v.ob[0] = 0;
			enm_vtxs[e][i].v.ob[1] = 0;
			enm_vtxs[e][i].v.ob[2] = 0;
			enm_vtxs[e][i].v.flag = 0;
			enm_vtxs[e][i].v.tc[0] = 0;
			enm_vtxs[e][i].v.tc[1] = 0;
			enm_vtxs[e][i].v.cn[0] = 200;
			enm_vtxs[e][i].v.cn[1] = 0;
			enm_vtxs[e][i].v.cn[2] = 0;
			enm_vtxs[e][i].v.cn[3] = 255;
		}

		for (i=0; i<8; i++)
		{
			cmp_vtxs[e][i].v.ob[0] = 0;
			cmp_vtxs[e][i].v.ob[1] = 0;
			cmp_vtxs[e][i].v.ob[2] = 0;
			cmp_vtxs[e][i].v.flag = 0;
			cmp_vtxs[e][i].v.tc[0] = 0;
			cmp_vtxs[e][i].v.tc[1] = 0;
			cmp_vtxs[e][i].v.cn[0] = 200;
			cmp_vtxs[e][i].v.cn[1] = 0;
			cmp_vtxs[e][i].v.cn[2] = 200;
			cmp_vtxs[e][i].v.cn[3] = 255;
		}
	}

#endif
}



void CEngineApp__DrawMap(CEngineApp *pThis)
{
#if ENABLE_MAP

	CEngineApp__SetupSegments(pThis);

	gSPDisplayList(pThis->m_pDLP++, RSP_ADDRESS(rspinit_dl));
	gSPDisplayList(pThis->m_pDLP++, RDP_ADDRESS(rdpinit_dl));

	gDPSetColorImage(pThis->m_pDLP++, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WD,
		   			  RDP_ADDRESS(CEngineApp__GetFrameData(pThis)->m_pFrameBuffer));

	gDPPipeSync(pThis->m_pDLP++);

	gDPSetCycleType(pThis->m_pDLP++, G_CYC_1CYCLE);

	gDPSetRenderMode(pThis->m_pDLP++, G_RM_AA_XLU_LINE, G_RM_AA_XLU_LINE2);
	gDPSetCombineMode(pThis->m_pDLP++, G_CC_SHADE, G_CC_SHADE);
	gSPClearGeometryMode(pThis->m_pDLP++, -1);
	gSPSetGeometryMode(pThis->m_pDLP++, G_SHADE | G_SHADING_SMOOTH) ;
	gSPTexture(pThis->m_pDLP++, 0, 0, 0, 0, G_OFF);
	gSPClipRatio(pThis->m_pDLP++, FRUSTRATIO_6);

	gSPMatrix(pThis->m_pDLP++, RSP_ADDRESS(&mtx_map),
	   		 G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);

//  	gDPSetScissor(pThis->m_pDLP++, G_SC_NON_INTERLACE, NOMAP_CLIPX, NOMAP_CLIPY-1, SCREEN_WD-NOMAP_CLIPX+1, SCREEN_HT-NOMAP_CLIPY) ;

	if (pThis->m_Scene.m_LevelInfo.m_bFlags & LEVEL_STORE_MAP)
		RevealMap();
	
	DoDrawMap(&pThis->m_pDLP);

/*
	gDPPipeSync(pThis->m_pDLP++);
	gDPSetCycleType(pThis->m_pDLP++, G_CYC_1CYCLE);
	gDPSetBlendColor(pThis->m_pDLP++, 255, 255, 255, 255);
	gDPSetPrimDepth(pThis->m_pDLP++, 0xffff, 0xffff);
	gDPSetDepthSource(pThis->m_pDLP++, G_ZS_PRIM);
	gDPSetRenderMode(pThis->m_pDLP++, G_RM_VISCVG, G_RM_VISCVG2);
	gDPFillRectangle(pThis->m_pDLP++, 0, 0, SCREEN_WD-1, SCREEN_HT-1);
*/

#endif
}

BOOL ClipLine2D(CVector3 vCorners[], CVector3 *pvA, CVector3 *pvB)
{
#if ENABLE_MAP

	int			e;
	CVector3		*pvCornerA, *pvCornerB;
	float			vNX, vNY;

	for (e=0; e<4; e++)
	{
		pvCornerA = &vCorners[e];
		pvCornerB = &vCorners[(e == 3) ? 0 : (e + 1)];

		vNX = pvCornerB->y - pvCornerA->y;
		vNY = pvCornerA->x - pvCornerB->x;

		if (		((vNX*(pvA->x - pvCornerA->x) + vNY*(pvA->z - pvCornerA->y)) > 0)
				&& ((vNX*(pvB->x - pvCornerA->x) + vNY*(pvB->z - pvCornerA->y)) > 0) )
		{
			return FALSE;
		}
	}

	return TRUE;

#endif
}


// carls smashing macro to color a piece of map
#define	COLOR_VERTEX													\
{																				\
	opac = 255 ;															\
	col = 200 ;																\
	diff = ((pvCorner->y-pApp->m_YPos)/SCALING_FACTOR) *4 ;	\
																				\
	col += diff ;															\
	if (col < 150)															\
		col = 150 ;															\
	else if (col >255)													\
		col = 255 ;															\
	if (diff > 50)															\
	{																			\
		opac -= (diff-50) ;												\
		if (opac < 75)														\
			opac = 75 ;														\
	}																			\
	else if (diff < -50)													\
	{																			\
		opac -= (50-diff) ;												\
		if (opac < 75)														\
			opac = 75 ;														\
	}																			\
																				\
	switch (pRegionSet->m_MapColor)									\
	{																			\
		case REG_MAPCOLOR_WHITE:										\
			vtxs[cVertex].v.cn[0] = col; 								\
			vtxs[cVertex].v.cn[1] = col;								\
			vtxs[cVertex].v.cn[2] = col; 								\
			break ;															\
		case REG_MAPCOLOR_GREEN:										\
			vtxs[cVertex].v.cn[0] = 0;									\
			vtxs[cVertex].v.cn[1] = col;								\
			vtxs[cVertex].v.cn[2] = 0;									\
			break ;															\
		case REG_MAPCOLOR_BLUE:											\
			vtxs[cVertex].v.cn[0] = 0;									\
			vtxs[cVertex].v.cn[1] = 0;									\
			vtxs[cVertex].v.cn[2] = col;								\
			break ;															\
		case REG_MAPCOLOR_RED:											\
			vtxs[cVertex].v.cn[0] = col;								\
			vtxs[cVertex].v.cn[1] = 0;									\
			vtxs[cVertex].v.cn[2] = 0;									\
			break ;															\
		case REG_MAPCOLOR_PINK:											\
			vtxs[cVertex].v.cn[0] = col;								\
			vtxs[cVertex].v.cn[1] = 0;									\
			vtxs[cVertex].v.cn[2] = col/2; 							\
			break ;															\
		case REG_MAPCOLOR_PURPLE:										\
			vtxs[cVertex].v.cn[0] = col;								\
			vtxs[cVertex].v.cn[1] = 0;									\
			vtxs[cVertex].v.cn[2] = col; 								\
			break ;															\
		case REG_MAPCOLOR_BROWN:										\
			vtxs[cVertex].v.cn[0] = col/2; 							\
			vtxs[cVertex].v.cn[1] = col/4; 							\
			vtxs[cVertex].v.cn[2] = 0;									\
			break ;															\
		case REG_MAPCOLOR_CYAN:											\
			vtxs[cVertex].v.cn[0] = 0;									\
			vtxs[cVertex].v.cn[1] = col;								\
			vtxs[cVertex].v.cn[2] = col;								\
			break ;															\
		case REG_MAPCOLOR_YELLOW:										\
			vtxs[cVertex].v.cn[0] = col;								\
			vtxs[cVertex].v.cn[1] = col;								\
			vtxs[cVertex].v.cn[2] = 0;									\
			break ;															\
		case REG_MAPCOLOR_ORANGE:										\
			vtxs[cVertex].v.cn[0] = col;								\
			vtxs[cVertex].v.cn[1] = col/2;							\
			vtxs[cVertex].v.cn[2] = 0;									\
			break ;															\
		case REG_MAPCOLOR_BLACK:										\
			vtxs[cVertex].v.cn[0] = 0;									\
			vtxs[cVertex].v.cn[1] = 0;									\
			vtxs[cVertex].v.cn[2] = 0;									\
			break ;															\
	}																			\
	vtxs[cVertex].v.cn[3] = opac * GetApp()->m_MapAlpha ;		\
}


#define		VIEW_DEPTH		(SQR(20*SCALING_FACTOR))

#define CLIP_LEFT		(1 << 0)
#define CLIP_TOP		(1 << 1)
#define CLIP_RIGHT	(1 << 2)
#define CLIP_BOTTOM	(1 << 3)
void DoDrawMap(Gfx **ppDLP)
{
#if ENABLE_MAP

	CGameRegion			*regions, *pRegion;
	int					cRegion,
							nBlocks,
							cBlock,
							cCorner,
							cEdge, cNextEdge;
	CIndexedSet			isCollision;
	CUnindexedSet		usRegions,
							usBlocks;
	void					*pbRegions,
							*pbBlocks;
	int					cVertex,
							nRemainingVertices,
							nLoad,
							i;
	DWORD					clip[3], *pClip;
	CVector3				*pvCorner, *pvCorner2;
	Vtx					*vtxs ;
	CMtxF					mfMap, mfInvMap, mfNorth;
	static Mtx			mMap[2], mIdent[2];
	float					scaleX, scaleY;//, yrot;
	CROMRegionBlock	*blocks, *pBlock;
	CBoundsRect			clipRect;
	CEngineApp			*pApp;
	CGameObjectInstance	*pInstance, *pPlayer ;
	int					nInsts ;
	CVector3				vCorners[4], vTCorners[4];

	float					cx, cz ;
	float					opacity ;
	float					sinTheta, cosTheta;
	BOOL					addvertex, drawedge, drawWholeMap, showenemy;
	int					cConnection ;
	int					diff, col, opac ;

	CUnindexedSet			usRegionAttributes;
	CROMRegionSet			*regionSets, *pRegionSet;
	void						*pbRegionAttributes;

	pApp = GetApp();

	if (!cache_is_valid)
		return;

	if (!pApp->m_Scene.m_pceCollision)
		return;

	vtxs = map_vtxs[even_odd];

	// get the centre position
	cx = pApp->m_XPos ;
	cz = pApp->m_ZPos ;

	// rotate the fucker so movements on x and z will be not dependant on the direction you're facing
	sinTheta = sin(pApp->m_RotY);
	cosTheta = cos(pApp->m_RotY);
	cx += (CTurokMovement.MapOffsetZ * sinTheta) - (CTurokMovement.MapOffsetX * cosTheta) ;
	cz += (CTurokMovement.MapOffsetZ * cosTheta) + (CTurokMovement.MapOffsetX * sinTheta) ;

	scaleX = 0.1;
	scaleY = 0.1;

	// Translate around player, and scale it
	cx = max(-32000, min(32000, cx));
	cz = max(-32000, min(32000, cz));
	CMtxF__Translate(mfMap, -cx, -cz, 0);

	CMtxF__PostMultScale(mfMap, scaleX, scaleY, 1);
	CMtxF__PostMultRotateZ(mfMap, pApp->m_RotY + ANGLE_DTOR(180));

	// translate to center of screen
	CMtxF__PostMultTranslate(mfMap, SCREEN_WD/2, SCREEN_HT/2, 0);

	CMtxF__Invert(mfInvMap, mfMap);

#if 1
	vCorners[0].x = 0;			vCorners[0].y = 0;			vCorners[0].z = 0;
	vCorners[1].x = SCREEN_WD;	vCorners[1].y = 0;			vCorners[1].z = 0;
	vCorners[2].x = SCREEN_WD;	vCorners[2].y = SCREEN_HT;	vCorners[2].z = 0;
	vCorners[3].x = 0;			vCorners[3].y = SCREEN_HT;	vCorners[3].z = 0;
#else
	vCorners[0].x = NOMAP_CLIPX;				vCorners[0].y = NOMAP_CLIPY; 	vCorners[0].z = 0;
	vCorners[1].x = SCREEN_WD-NOMAP_CLIPX;	vCorners[1].y = NOMAP_CLIPY;	vCorners[1].z = 0;
	vCorners[2].x = SCREEN_WD-NOMAP_CLIPX;	vCorners[2].y = SCREEN_HT-NOMAP_CLIPY;	vCorners[2].z = 0;
	vCorners[3].x = NOMAP_CLIPX;				vCorners[3].y = SCREEN_HT-NOMAP_CLIPY;	vCorners[3].z = 0;
#endif

	for (i=0; i<4; i++)
		CMtxF__VectorMult(mfInvMap, &vCorners[i], &vTCorners[i]);

	clipRect.m_MinX = clipRect.m_MaxX = vTCorners[0].x;
	clipRect.m_MinZ = clipRect.m_MaxZ = vTCorners[0].y;
	for (i=1; i<4; i++)
	{
		clipRect.m_MinX = min(clipRect.m_MinX, vTCorners[i].x);
		clipRect.m_MinZ = min(clipRect.m_MinZ, vTCorners[i].y);

		clipRect.m_MaxX = max(clipRect.m_MaxX, vTCorners[i].x);
		clipRect.m_MaxZ = max(clipRect.m_MaxZ, vTCorners[i].y);
	}

	CMtxF__ToMtx(mfMap, mMap[even_odd]);
	gSPMatrix((*ppDLP)++, RSP_ADDRESS(&mMap[even_odd]),
	   		 G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);


	// use CCacheEntry__GetDataPtr() if accessing cache in higher priority thread
	CIndexedSet__ConstructFromRawData(&isCollision,
												 CCacheEntry__GetData(pApp->m_Scene.m_pceCollision),
												 FALSE);


	pbRegions = CIndexedSet__GetBlock(&isCollision, CART_COLLISION_usRegions);
	CUnindexedSet__ConstructFromRawData(&usRegions, pbRegions, FALSE);

	regions = (CGameRegion*) CUnindexedSet__GetBasePtr(&usRegions);

	pbBlocks =  CIndexedSet__GetBlock(&isCollision, CART_COLLISION_usBlockBounds);
	CUnindexedSet__ConstructFromRawData(&usBlocks, pbBlocks, FALSE);

	blocks = (CROMRegionBlock*) CUnindexedSet__GetBasePtr(&usBlocks);
	nBlocks = CUnindexedSet__GetBlockCount(&usBlocks);


	pbRegionAttributes = CIndexedSet__GetBlock(&isCollision, CART_COLLISION_usRegionAttributes);

	CUnindexedSet__ConstructFromRawData(&usRegionAttributes, pbRegionAttributes, FALSE);
	regionSets = (CROMRegionSet*) CUnindexedSet__GetBasePtr(&usRegionAttributes);


	cVertex = 0;
	cConnection = 0 ;
	//rmonPrintf("nBlocks:%d, nRegions:%d\n", nBlocks, CUnindexedSet__GetBlockCount(&usRegions));

	drawWholeMap = !(pApp->m_Scene.m_LevelInfo.m_bFlags & LEVEL_STORE_MAP);

	if (GetApp()->m_dwCheatFlags & CHEATFLAG_ALLMAP)
		drawWholeMap = TRUE ;

	// search through blocks
	cRegion = 0;
	for (cBlock=0; cBlock<nBlocks; cBlock++)
	{
		pBlock = &blocks[cBlock];

		if (CBoundsRect__IsOverlapping(&pBlock->m_BoundsRect, &clipRect))
		{
			for (i=0; i<pBlock->m_nRegions; i++)
			{
				// get a pointer to current region
				pRegion = &regions[cRegion];

				pRegionSet = &regionSets[pRegion->m_nRegionSet];
				if ((pRegionSet->m_dwFlags & REGFLAG_DONTDRAWONMAP) == 0)
				{
					// check if entire triangle is clipped
					for (cCorner=0; cCorner<3; cCorner++)
					{
						pvCorner = &pRegion->m_pCorners[cCorner]->m_vCorner;
						pClip = &clip[cCorner];

						if (pvCorner->x < clipRect.m_MinX)
							*pClip = CLIP_LEFT;
						else if (pvCorner->x >= clipRect.m_MaxX)
							*pClip = CLIP_RIGHT;
						else
							*pClip = 0;

						if (pvCorner->z < clipRect.m_MinZ)
							*pClip |= CLIP_BOTTOM;
						else if (pvCorner->z >= clipRect.m_MaxZ)
							*pClip |= CLIP_TOP;
					}

 					// if not completly clipped
					if (!(clip[0] & clip[1] & clip[2]))
					{
#if 0
#if 0
						// make visible if in clipping region
						if ( ((cx - pApp->m_XPos) < 1.0) && ((cz - pApp->m_ZPos) < 1.0))
							pRegion->m_wFlags |= REGFLAG_REGIONENTERED ;
#else
						// check if already visible, or should become visible
						if (!(pRegion->m_wFlags & REGFLAG_REGIONENTERED))
						{
							for (cEdge=0; cEdge<3; cEdge++)
							{
								pvCorner = &pRegion->m_pCorners[cEdge]->m_vCorner;
								dx = pApp->m_XPos - pvCorner->x ;
								dz = pApp->m_ZPos - pvCorner->z ;
								dx = SQR(dx) + SQR(dz) ;
								if (dx < VIEW_DEPTH)
								{
									pRegion->m_wFlags |= REGFLAG_REGIONENTERED ;
									CScene__SetRegionFlag(&GetApp()->m_Scene, cRegion, TRUE) ;
									break ;
								}
							}
						}
#endif
#endif
						if (drawWholeMap || (pRegion->m_wFlags & REGFLAG_REGIONENTERED))
						{
							// check each edge
							for (cEdge=0; cEdge<3; cEdge++)
							{
								drawedge = FALSE ;
								// if this edge has no neighbor, or is different from its neighbor
								if (!pRegion->m_pNeighbors[cEdge])
									drawedge = TRUE ;
								else if (pRegion->m_nRegionSet < pRegion->m_pNeighbors[cEdge]->m_nRegionSet)
									drawedge = TRUE ;

								//else if (pRegion->m_pNeighbors[cEdge]->m_wFlags & REGFLAG_CLIFF)
								//	drawedge = TRUE ;


								if (drawedge)
								{
									cNextEdge = (cEdge == 2) ? 0 : (cEdge + 1);

									// if edge is not completely clipped
									if (!(clip[cEdge] & clip[cNextEdge]))
									{
										pvCorner = &pRegion->m_pCorners[cEdge]->m_vCorner;
										pvCorner2 = &pRegion->m_pCorners[cNextEdge]->m_vCorner;


										// exact clip
										if (ClipLine2D(vTCorners, pvCorner, pvCorner2))
										{

											//---------------------------------- point 1
											//ASSERT(fabs(pvCorner->x) < 32767);
											//ASSERT(fabs(pvCorner->z) < 32767);

											addvertex = TRUE ;

											// check if connecting to previous vertex
								  			if ((cVertex !=0) &&
								  				 (vtxs[cVertex-1].v.ob[0] == (short)pvCorner->x) &&
								  				 (vtxs[cVertex-1].v.ob[1] == (short)pvCorner->z))
								  			{
								  					addvertex = FALSE ;
								  			}

											if (addvertex)
								  			{
												Connections[cConnection++] = cVertex ;
												vtxs[cVertex].v.ob[0] = (short) pvCorner->x;
												vtxs[cVertex].v.ob[1] = (short) pvCorner->z;

												COLOR_VERTEX ;
												cVertex++;
											}
											else
												Connections[cConnection++] = cVertex-1 ;

											//---------------------------------- point 2
											//ASSERT(fabs(pvCorner2->x) < 32767);
											//ASSERT(fabs(pvCorner2->z) < 32767);
											Connections[cConnection++] = cVertex ;
											vtxs[cVertex].v.ob[0] = (short) pvCorner2->x;
											vtxs[cVertex].v.ob[1] = (short) pvCorner2->z;

											COLOR_VERTEX ;
											cVertex++;

											if (cVertex >= (MAX_MAP_VTXS-1))
												goto map_finished_adding_vertices;
										}
									}
								}
							}
						}
					}
				}

				cRegion++;
			}
		}
		else
		{
			cRegion += pBlock->m_nRegions;
		}
	}
	ASSERT(cRegion == CUnindexedSet__GetBlockCount(&usRegions));

map_finished_adding_vertices:

	opacity = 255 * GetApp()->m_MapAlpha ;

	// draw all the added regions
	nRemainingVertices = cVertex;
	cVertex = 0;
	i = 0 ;
	while (nRemainingVertices > 1)
	{
		nLoad = min(VERTEX_CACHE_SIZE, nRemainingVertices);
		gSPVertex((*ppDLP)++,
      			 RSP_ADDRESS(&vtxs[cVertex]),
      			 nLoad, 0);

//rmonPrintf("loaded %d, from %d\n", nLoad, cVertex) ;

		// go through the lines until trying to draw out of loaded vertices range
		//while ((Connections[i]-cVertex < (nLoad-1)) && (i < (cConnection-1)))
		while ((Connections[i+1]-cVertex < nLoad) && (i < (cConnection-1)))
		{
			ASSERT(Connections[i]-cVertex >=0) ;
			ASSERT(Connections[i]-cVertex < nLoad) ;

//rmonPrintf("connecting %d to %d\n", Connections[i]-cVertex, Connections[i+1]-cVertex) ;
			gSPLine3D((*ppDLP)++,
						 Connections[i]-cVertex, Connections[i+1]-cVertex,
						 0);

 			i += 2 ;
		}

		if (Connections[i+1]-cVertex == nLoad)
		{
			nLoad-- ;
	  	}

		nRemainingVertices -= nLoad;
		cVertex += nLoad;
	}


	// draw the player
	vtxs = ply_vtxs[even_odd];

#define XOFFSET	25
#define YOFFSET	40

	vtxs[0].v.ob[0] = pApp->m_XPos - (YOFFSET * sinTheta);
	vtxs[0].v.ob[1] = pApp->m_ZPos - (YOFFSET * cosTheta);

	vtxs[1].v.ob[0] = pApp->m_XPos + (XOFFSET * cosTheta) + (YOFFSET * sinTheta);
	vtxs[1].v.ob[1] = pApp->m_ZPos - (XOFFSET * sinTheta) + (YOFFSET * cosTheta);

	vtxs[2].v.ob[0] = pApp->m_XPos - (XOFFSET * cosTheta) + (YOFFSET * sinTheta);
	vtxs[2].v.ob[1] = pApp->m_ZPos + (XOFFSET * sinTheta) + (YOFFSET * cosTheta);

	vtxs[0].v.cn[3] = opacity ;
	vtxs[1].v.cn[3] = opacity ;
	vtxs[2].v.cn[3] = opacity ;

	gSPVertex((*ppDLP)++,
     			 RSP_ADDRESS(vtxs),
     			 3, 0);
	gSPLine3D((*ppDLP)++,
				 0, 1,
				 0);
	gSPLine3D((*ppDLP)++,
				 0, 2,
				 0);


	// draw animated instances
	vtxs = enm_vtxs[even_odd];
	cVertex = 0 ;
	nInsts = pApp->m_Scene.m_nActiveAnimInstances;

	pPlayer = CEngineApp__GetPlayer(pApp) ;

	for (i=1; i<nInsts; i++)
	{
		pInstance = GetApp()->m_Scene.m_pActiveAnimInstances[i];

		//rmonPrintf("flag:%d\n", AI_GetEA(pInstance)->m_dwTypeFlags2 & AI_TYPE2_DONTDRAWONMAP) ;

		showenemy = FALSE ;


//		if ((GetApp()->m_dwCheatFlags & CHEATFLAG_SHOWALLENEMIES) &&
//			(!CGameObjectInstance__IsDevice(pInstance)))
//			showenemy = TRUE ;

//		else
		if ((CAnimInstanceHdr__CastYeThyRay(&pPlayer->ah, &pInstance->ah.ih)) &&
			(!CGameObjectInstance__IsDevice(pInstance)) &&
			(!(AI_GetEA(pInstance)->m_dwTypeFlags2 & AI_TYPE2_DONTDRAWONMAP)))
			showenemy = TRUE ;

		// don't show interactive animations
		if ((showenemy) && (AI_GetDyn(pInstance)->m_dwStatusFlags & AI_INTERACTIVEANIMATION))
			showenemy = FALSE ;

		// if enemy is dead, don't show it
		if ((showenemy) && (AI_GetDyn(pInstance)->m_Health <=0))
			showenemy = FALSE ;

		if (showenemy)
		{
			sinTheta = sin(pInstance->m_RotY);
			cosTheta = cos(pInstance->m_RotY);
			
			vtxs[(cVertex*3)+0].v.ob[0] = pInstance->ah.ih.m_vPos.x - (YOFFSET * sinTheta);
			vtxs[(cVertex*3)+0].v.ob[1] = pInstance->ah.ih.m_vPos.z - (YOFFSET * cosTheta);

			vtxs[(cVertex*3)+1].v.ob[0] = pInstance->ah.ih.m_vPos.x + (XOFFSET * cosTheta) + (YOFFSET * sinTheta);
			vtxs[(cVertex*3)+1].v.ob[1] = pInstance->ah.ih.m_vPos.z - (XOFFSET * sinTheta) + (YOFFSET * cosTheta);

			vtxs[(cVertex*3)+2].v.ob[0] = pInstance->ah.ih.m_vPos.x - (XOFFSET * cosTheta) + (YOFFSET * sinTheta);
			vtxs[(cVertex*3)+2].v.ob[1] = pInstance->ah.ih.m_vPos.z + (XOFFSET * sinTheta) + (YOFFSET * cosTheta);


			opacity = 255 ;
			diff = ((pInstance->ah.ih.m_vPos.y-pApp->m_YPos)/SCALING_FACTOR) *4 ;
			if (diff > 50)
			{
				opacity -= (diff-50) ;
				if (opacity < 75)
					opacity = 75 ;
			}
			else if (diff < -50)
			{
				opacity -= (50-diff) ;
				if (opacity < 75)
					opacity = 75 ;
			}
			vtxs[(cVertex*3)+0].v.cn[3] = opacity * GetApp()->m_MapAlpha ;
			vtxs[(cVertex*3)+1].v.cn[3] = opacity * GetApp()->m_MapAlpha ;
			vtxs[(cVertex*3)+2].v.cn[3] = opacity * GetApp()->m_MapAlpha ;

			gSPVertex((*ppDLP)++,
		   			 RSP_ADDRESS(&vtxs[cVertex*3]),
		   			 3, 0);
			gSPLine3D((*ppDLP)++,
						 0, 1,
						 0);
			gSPLine3D((*ppDLP)++,
						 0, 2,
						 0);

			cVertex++ ;
			if (cVertex == MAX_VISIBLE_ENEMIES)
				break ;
		}
	}

	// draw a compass
#define	CW		15
#define	CH		15
	vtxs = cmp_vtxs[even_odd];

	CMtxF__Scale(mfNorth, .5, .5, 1);
	CMtxF__PostMultTranslate(mfNorth, SCREEN_WD/2, 32, 0);

	CMtxF__ToMtx(mfNorth, mIdent[even_odd]);
	gSPMatrix((*ppDLP)++, RSP_ADDRESS(&mIdent[even_odd]),
	   		 G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);


	// arrow head
	vtxs[0].v.ob[0] = CW*sin(pApp->m_RotY) ;
	vtxs[0].v.ob[1] = -CH*cos(pApp->m_RotY) ;
	vtxs[1].v.ob[0] = vtxs[0].v.ob[0] - (40 * sin(-pApp->m_RotY)) ;
	vtxs[1].v.ob[1] = vtxs[0].v.ob[1] - (40 * cos(-pApp->m_RotY)) ;
	vtxs[2].v.ob[0] = vtxs[1].v.ob[0] + (15 * sin(-pApp->m_RotY-.4));
	vtxs[2].v.ob[1] = vtxs[1].v.ob[1] + (15 * cos(-pApp->m_RotY-.4));
	vtxs[3].v.ob[0] = vtxs[1].v.ob[0] + (15 * sin(-pApp->m_RotY+.4));
	vtxs[3].v.ob[1] = vtxs[1].v.ob[1] + (15 * cos(-pApp->m_RotY+.4));

	// N
	vtxs[4].v.ob[0] = -CW/2 ;
	vtxs[4].v.ob[1] = -CH/2 ;
	vtxs[5].v.ob[0] = vtxs[4].v.ob[0] ;
	vtxs[5].v.ob[1] = vtxs[4].v.ob[1] +CH;
	vtxs[6].v.ob[0] = vtxs[5].v.ob[0] +CW; 
	vtxs[6].v.ob[1] = vtxs[5].v.ob[1] -CH;
	vtxs[7].v.ob[0] = vtxs[6].v.ob[0] ;
	vtxs[7].v.ob[1] = vtxs[6].v.ob[1] +CH;

	vtxs[0].v.cn[3] = opacity ;
	vtxs[1].v.cn[3] = opacity ;
	vtxs[2].v.cn[3] = opacity ;
	vtxs[3].v.cn[3] = opacity ;
	vtxs[4].v.cn[3] = opacity ;
	vtxs[5].v.cn[3] = opacity ;
	vtxs[6].v.cn[3] = opacity ;
	vtxs[7].v.cn[3] = opacity ;

	gSPVertex((*ppDLP)++,
     			 RSP_ADDRESS(vtxs),
     			 8, 0);
	gSPLine3D((*ppDLP)++,
				 0, 1,
				 0);
	gSPLine3D((*ppDLP)++,
				 1, 2,
				 0);
	gSPLine3D((*ppDLP)++,
				 1, 3,
				 0);

	gSPLine3D((*ppDLP)++,
				 4, 5,
				 0);
	gSPLine3D((*ppDLP)++,
				 5, 6,
				 0);
	gSPLine3D((*ppDLP)++,
				 6, 7,
				 0);






	CUnindexedSet__Destruct(&usRegions);
	CUnindexedSet__Destruct(&usBlocks);
	CUnindexedSet__Destruct(&usRegionAttributes);
	CIndexedSet__Destruct(&isCollision);


	CEngineApp__DrawWarp(GetApp());
	CEngineApp__DrawDeath(GetApp());
	CEngineApp__DrawFade(GetApp()) ;

#endif
}


BOOL CEngineApp__DrawingMap(CEngineApp *pThis)
{
	return (pThis->m_MapMode && (pThis->m_Mode == MODE_GAME) && cache_is_valid);
}


// non drawing version, for walking through map and having it reveal iteslf
void CEngineApp__RevealMap(CEngineApp *pThis)
{
	RevealMap();
}

/*
void CEngineApp__RevealAllMap(CEngineApp *pThis)
{
	CGameRegion			*regions, *pRegion;
	int					cRegion,
							nBlocks,
							cBlock,
							i ;
	CIndexedSet			isCollision;
	CUnindexedSet		usRegions,
							usBlocks;
	void					*pbRegions,
							*pbBlocks;
	CROMRegionBlock	*blocks, *pBlock;


	if (!cache_is_valid)
		return ;


	// use CCacheEntry__GetDataPtr() if accessing cache in higher priority thread
	CIndexedSet__ConstructFromRawData(&isCollision,
												 CCacheEntry__GetData(pThis->m_Scene.m_pceCollision),
												 FALSE);


	pbRegions = CIndexedSet__GetBlock(&isCollision, CART_COLLISION_usRegions);
	CUnindexedSet__ConstructFromRawData(&usRegions, pbRegions, FALSE);

	regions = (CGameRegion*) CUnindexedSet__GetBasePtr(&usRegions);

	pbBlocks =  CIndexedSet__GetBlock(&isCollision, CART_COLLISION_usBlockBounds);
	CUnindexedSet__ConstructFromRawData(&usBlocks, pbBlocks, FALSE);

	blocks = (CROMRegionBlock*) CUnindexedSet__GetBasePtr(&usBlocks);
	nBlocks = CUnindexedSet__GetBlockCount(&usBlocks);

	// search through blocks
	cRegion = 0;
	for (cBlock=0; cBlock<nBlocks; cBlock++)
	{
		pBlock = &blocks[cBlock];

		for (i=0; i<pBlock->m_nRegions; i++)
		{
			// get a pointer to current region
			pRegion = &regions[cRegion];

			pRegion->m_wFlags |= REGFLAG_REGIONENTERED ;
			CScene__SetRegionFlag(&pThis->m_Scene, cRegion, TRUE) ;

			cRegion++;
		}
	}
}
*/


