// geometry.c
//

#include "stdafx.h"

#include "tengine.h"
#include "geometry.h"
#include "romstruc.h"
#include "cartdir.h"
#include "matdefs.h"
#include "textload.h"
#include "shadowt.h"
#include "dlists.h"
#include "tmove.h"

#include "debug.h"



// globals
DWORD			current_combine_mode ;
DWORD			current_render_mode ;
//DWORD			saved = 0;

BOOL			draw_transparent_geometry = TRUE ;
BOOL			draw_intersect_geometry = TRUE ;

DWORD			fx_mode = FXMODE_NONE;
float			fx_color[5] = { 255, 255, 255, 255, 255 };

// CGeometry implementation
/////////////////////////////////////////////////////////////////////////////

void	CGeometry__DrawSection(CGeometry *pThis, Gfx **ppDLP, CIndexedSet *pisSection, CCartCache *pCache, BOOL Inter, BOOL Better, int nFrame);

// callbacks
void	CGeometry__Decompress(CGeometry *pThis, CCacheEntry **ppceTarget);

/////////////////////////////////////////////////////////////////////////////
void CGeometry__ResetDrawModes(void)
{
	current_render_mode = -1 ;
	current_combine_mode = -1 ;

#ifdef PLATFORM_PORT
	/* PORT: clear the per-object special-effect mode at the per-frame render-state reset (this is
	 * called from tengine.c:1037 before DrawGAME, and from CScene__DrawSelf before the HUD).
	 * CGameObjectInstance__PreDraw sets fx_mode (GLARE / TOTRANSPARENT / TOCOLOR / ...) for an enemy
	 * in a special state, and the reset (romstruc.c:8944 + PostDraw fx_mode=NONE) is gated on
	 * m_asCurrent.m_pceAnim, which DoAI/Advance can NULL mid-draw (a dying/transitioning enemy) —
	 * leaking a non-NONE fx_mode. fx_mode is a global never otherwise reset per frame, so the leak
	 * bleeds into the NEXT frame's WORLD geometry (DrawEnvironment, drawn before objects), which then
	 * renders transparent/glared/solid-color — the sustained, interactive "camera completely fucked
	 * up while walking" regression (ANIMOBJ=0 sets no fx_mode, so it stays NONE = stable). Clearing
	 * it here makes fx_mode strictly per-object-per-frame and cannot corrupt the world or HUD. */
	{ extern char *getenv(const char*); static int s_on=-1; if(s_on<0) s_on=getenv("TUROK_FXLEAK")?1:0;
	  if(s_on && fx_mode != FXMODE_NONE){ extern int fprintf(void*,const char*,...); extern void*stderr; extern DWORD frame_number;
	    static int _c=0; if(_c++<30) fprintf(stderr,"[fxleak] frame=%lu CAUGHT leaked fx_mode=%lu (would render objects/weapon as a solid-color blob) — cleared\n",(unsigned long)frame_number,(unsigned long)fx_mode); } }
	fx_mode = FXMODE_NONE ;
	fx_color[0] = fx_color[1] = fx_color[2] = fx_color[3] = fx_color[4] = 255 ;
#endif

	//rmonPrintf("saved:%d\n", saved) ;
	//saved = 0 ;
}


/////////////////////////////////////////////////////////////////////////////
void CGeometry__SetCombineMode(Gfx **ppDLP, DWORD mode)
{
#ifndef PLATFORM_PORT
	/* N64: memoize to skip redundant combine emits. PORT: the memo (current_combine_mode) is a
	 * game-side shadow of gfx_pc's PERSISTENT rdp.combine_mode. The camera setup re-issues
	 * projection/viewport/scissor each frame but NOT the combine/render/othermode, so these persist
	 * across frames and the only guard is this desyncable cache. If gfx_pc's real combine ever
	 * diverges from the cache (a raw emit, an interactive fx_mode draw order), a cache HIT skips the
	 * needed re-emit and the wrong combiner bleeds into the world AND the HUD. Always emit on the
	 * port: when synced this is a visual no-op; when desynced it re-syncs gfx_pc every section. */
	if (current_combine_mode == mode)
	{
		//saved++;
		return ;
	}
#endif

	current_combine_mode = mode;

	switch (mode)
	{
		case	G_CC_ROB_SELFILLUM_PSEUDO_SHADEALPHA__G_CC_PASS2:
			gDPSetCombineMode((*ppDLP)++,
									G_CC_ROB_SELFILLUM_PSEUDO_SHADEALPHA,
									G_CC_PASS2) ;
			break ;
		case	G_CC_ROB_SELFILLUM_PSEUDO_SHADEALPHA__G_CC_FXMODE_TOTRANSPARENT:
			gDPSetCombineMode((*ppDLP)++,
									G_CC_ROB_SELFILLUM_PSEUDO_SHADEALPHA,
									G_CC_FXMODE_TOTRANSPARENT) ;
			break ;
		case	G_CC_ROB_SELFILLUM_PSEUDO__G_CC_PASS2:
			gDPSetCombineMode((*ppDLP)++,
									G_CC_ROB_SELFILLUM_PSEUDO,
									G_CC_PASS2);
			break ;
		case	G_CC_ROB_SELFILLUM_PSEUDO_SHADEALPHA__G_CC_ROB_LIGHTED2:
			gDPSetCombineMode((*ppDLP)++,
									G_CC_ROB_SELFILLUM_PSEUDO_SHADEALPHA,
									G_CC_ROB_LIGHTED2);
			break ;
		case	G_CC_ROB_SELFILLUM_PSEUDO__G_CC_ROB_LIGHTED2:
			gDPSetCombineMode((*ppDLP)++,
									G_CC_ROB_SELFILLUM_PSEUDO,
									G_CC_ROB_LIGHTED2);
			break ;
		case	G_CC_TRILERP__G_CC_DECALRGB2:
			gDPSetCombineMode((*ppDLP)++,
									G_CC_TRILERP,
									G_CC_DECALRGB2);
			break ;
		case	G_CC_ROB_SELFILLUM_TEXTURE__G_CC_PASS2:
			gDPSetCombineMode((*ppDLP)++,
									G_CC_ROB_SELFILLUM_TEXTURE,
									G_CC_PASS2);
			break ;
		case	G_CC_ROB_SELFILLUM_TEXTURE__G_CC_FXMODE_TOTRANSPARENT:
			gDPSetCombineMode((*ppDLP)++,
									G_CC_ROB_SELFILLUM_TEXTURE,
									G_CC_FXMODE_TOTRANSPARENT);
			break ;
		case	G_CC_ROB_SELFILLUM_TEXTURE__G_CC_FXMODE_TOCOLOR:
			gDPSetCombineMode((*ppDLP)++,
									G_CC_ROB_SELFILLUM_TEXTURE,
									G_CC_FXMODE_TOCOLOR);
			break ;
		case	G_CC_ROB_SELFILLUM_TEXTURE__G_CC_FXMODE_GLARE:
			gDPSetCombineMode((*ppDLP)++,
									G_CC_ROB_SELFILLUM_TEXTURE,
									G_CC_FXMODE_GLARE);
			break ;
		case	G_CC_ROB_SELFILLUM_TEXTURE__G_CC_FXMODE_FROMCOLOR_TOTRANSPARENT:
			gDPSetCombineMode((*ppDLP)++,
									G_CC_ROB_SELFILLUM_TEXTURE,
									G_CC_FXMODE_FROMCOLOR_TOTRANSPARENT);
			break ;
		case	G_CC_ROB_SELFILLUM_TEXTURE__G_CC_FXMODE_TOCOLORANDTRANSPARENT:
			gDPSetCombineMode((*ppDLP)++,
									G_CC_ROB_SELFILLUM_TEXTURE,
									G_CC_FXMODE_TOCOLORANDTRANSPARENT);
			break ;
		case	G_CC_TRILERP__G_CC_MODULATERGB2:
			gDPSetCombineMode((*ppDLP)++,
									G_CC_TRILERP,
									G_CC_MODULATERGB2);
			break ;
		case	G_CC_ROB_LIGHTED_TEXTURE__G_CC_FXMODE_NONE:
			gDPSetCombineMode((*ppDLP)++,
									G_CC_ROB_LIGHTED_TEXTURE,
									G_CC_FXMODE_NONE);
			break ;
		case	G_CC_ROB_LIGHTED_TEXTURE__G_CC_FXMODE_TOTRANSPARENT:
			gDPSetCombineMode((*ppDLP)++,
									G_CC_ROB_LIGHTED_TEXTURE,
									G_CC_FXMODE_TOTRANSPARENT);
			break ;
		case	G_CC_ROB_LIGHTED_TEXTURE__G_CC_FXMODE_TOCOLOR:
			gDPSetCombineMode((*ppDLP)++,
									G_CC_ROB_LIGHTED_TEXTURE,
									G_CC_FXMODE_TOCOLOR);
			break ;
		case	G_CC_ROB_LIGHTED_TEXTURE__G_CC_FXMODE_FROMCOLOR_TOTRANSPARENT:
			gDPSetCombineMode((*ppDLP)++,
									G_CC_ROB_LIGHTED_TEXTURE,
									G_CC_FXMODE_FROMCOLOR_TOTRANSPARENT);
			break ;
		case	G_CC_ROB_LIGHTED_TEXTURE__G_CC_FXMODE_TOCOLORANDTRANSPARENT:
			gDPSetCombineMode((*ppDLP)++,
									G_CC_ROB_LIGHTED_TEXTURE,
									G_CC_FXMODE_TOCOLORANDTRANSPARENT);
			break ;
		case	G_CC_ROB_SELFILLUM_SOLID__G_CC_PASS2:
			gDPSetCombineMode((*ppDLP)++,
									G_CC_ROB_SELFILLUM_SOLID,
									G_CC_PASS2);
			break ;
		case	G_CC_ROB_LIGHTED_SOLID__G_CC_FXMODE_NONE:
			gDPSetCombineMode((*ppDLP)++,
									G_CC_ROB_LIGHTED_SOLID,
									G_CC_FXMODE_NONE);
			break ;
		case	G_CC_ROB_LIGHTED_SOLID__G_CC_FXMODE_TOTRANSPARENT:
			gDPSetCombineMode((*ppDLP)++,
									G_CC_ROB_LIGHTED_SOLID,
									G_CC_FXMODE_TOTRANSPARENT);
			break ;
		case	G_CC_ROB_LIGHTED_SOLID__G_CC_FXMODE_TOCOLOR:
			gDPSetCombineMode((*ppDLP)++,
									G_CC_ROB_LIGHTED_SOLID,
									G_CC_FXMODE_TOCOLOR);
			break ;
		case	G_CC_ROB_SELFILLUM_SOLID__G_CC_FXMODE_GLARE:
			gDPSetCombineMode((*ppDLP)++,
									G_CC_ROB_SELFILLUM_SOLID,
									G_CC_FXMODE_GLARE);
			break ;
		case	G_CC_ROB_LIGHTED_SOLID__G_CC_FXMODE_FROMCOLOR_TOTRANSPARENT:
			gDPSetCombineMode((*ppDLP)++,
									G_CC_ROB_LIGHTED_SOLID,
									G_CC_FXMODE_FROMCOLOR_TOTRANSPARENT);
			break ;


		// ONSCREEN
		case	G_CC_ROB_DECALRGBA_PRIMALPHA__G_CC_ROB_DECALRGBA_PRIMALPHA:
			gDPSetCombineMode((*ppDLP)++,
									G_CC_ROB_DECALRGBA_PRIMALPHA,
									G_CC_ROB_DECALRGBA_PRIMALPHA) ;
			break ;
		case	G_CC_PRIMITIVE__G_CC_PRIMITIVE:
			gDPSetCombineMode((*ppDLP)++,
									G_CC_PRIMITIVE,
									G_CC_PRIMITIVE) ;
			break ;
		case	G_CC_MULTIBIT_ALPHA__G_CC_PASS2:
			gDPSetCombineMode((*ppDLP)++,
									G_CC_MULTIBIT_ALPHA,
									G_CC_PASS2) ;
			break ;
		case	G_CC_16RGBA_4A__G_CC_PASS2:
			gDPSetCombineMode((*ppDLP)++,
									G_CC_16RGBA_4A,
									G_CC_PASS2) ;
			break ;
		// FX
		case	G_CC_SHADE__G_CC_SHADE:
			gDPSetCombineMode((*ppDLP)++,
									G_CC_SHADE,
									G_CC_SHADE) ;
			break ;
		// PARTICLE
		case	G_CC_ROB_PSEUDOCOLOR_PRIMALPHA__G_CC_ROB_PSEUDOCOLOR_PRIMALPHA:
			gDPSetCombineMode((*ppDLP)++,
									G_CC_ROB_PSEUDOCOLOR_PRIMALPHA,
									G_CC_ROB_PSEUDOCOLOR_PRIMALPHA) ;
			break ;
		case G_CC_ROB_QUACK_PRIM__G_CC_ROB_QUACK_PRIM:
			gDPSetCombineMode((*ppDLP)++,
									G_CC_ROB_QUACK_PRIM,
									G_CC_ROB_QUACK_PRIM) ;
		case G_CC_ROB_QUACK_ENV__G_CC_ROB_QUACK_ENV:
			gDPSetCombineMode((*ppDLP)++,
									G_CC_ROB_QUACK_ENV,
									G_CC_ROB_QUACK_ENV) ;
			break ;
		// SCENE
		case	G_CC_ROB_PSEUDOCOLOR_SHADEALPHA__G_CC_ROB_PSEUDOCOLOR_SHADEALPHA:
			gDPSetCombineMode((*ppDLP)++,
									G_CC_ROB_PSEUDOCOLOR_SHADEALPHA,
									G_CC_ROB_PSEUDOCOLOR_SHADEALPHA) ;
			break ;
		case	G_CC_MODULATERGB__G_CC_MODULATERGB:
			gDPSetCombineMode((*ppDLP)++,
									G_CC_MODULATERGB,
									G_CC_MODULATERGB) ;
			break ;
		// ROMSTRUC
		case	G_CC_ROB_BLACK_PRIMALPHA__G_CC_PASS2:
			gDPSetCombineMode((*ppDLP)++,
									G_CC_ROB_BLACK_PRIMALPHA,
									G_CC_PASS2) ;
			break ;
		default:
			ASSERT(FALSE);
			break;
	}
}

/////////////////////////////////////////////////////////////////////////////
BOOL CGeometry__SetRenderMode(Gfx **ppDLP, DWORD mode)
{
#ifndef PLATFORM_PORT
	/* PORT: always emit — see CGeometry__SetCombineMode. Returning TRUE on every call also makes the
	 * masked-material path (DrawSection ~937) ALWAYS re-emit gDPSetAlphaCompare(G_AC_THRESHOLD) +
	 * gDPSetBlendColor, so the alpha-compare (independent persistent other_mode_l state, untracked by
	 * this cache) can never be left stale across draws. */
	if (current_render_mode == mode)
		return FALSE;
#endif

	current_render_mode = mode;

	switch (mode)
	{
		case	G_RM_PASS__G_RM_AA_ZB_XLU_SURF2:
			gDPSetRenderMode((*ppDLP)++,
									G_RM_PASS,
									G_RM_AA_ZB_XLU_SURF2) ;
			break ;
		case	G_RM_FOG_SHADE_A__G_RM_AA_ZB_XLU_SURF2:
			if (GetApp()->m_dwCheatFlags & CHEATFLAG_QUACKMODE)
			{
				gDPSetRenderMode((*ppDLP)++,
										G_RM_FOG_SHADE_A,
										G_RM_ZB_OPA_SURF2) ;
			}
			else
			{
				gDPSetRenderMode((*ppDLP)++,
										G_RM_FOG_SHADE_A,
										G_RM_AA_ZB_XLU_SURF2) ;
			}
			break ;
		case	G_RM_PASS__G_RM_ZB_XLU_SURF2:
			if (GetApp()->m_dwCheatFlags & CHEATFLAG_QUACKMODE)
			{
				gDPSetRenderMode((*ppDLP)++,
										G_RM_PASS,
										G_RM_ZB_OPA_SURF2) ;
			}
			else
			{
				gDPSetRenderMode((*ppDLP)++,
										G_RM_PASS,
										G_RM_ZB_XLU_SURF2) ;
			}
			break ;
		case	G_RM_PASS__G_RM_ZB_CLD_SURF2:
			if (GetApp()->m_dwCheatFlags & CHEATFLAG_QUACKMODE)
			{
				gDPSetRenderMode((*ppDLP)++,
										G_RM_PASS,
										G_RM_ZB_OPA_SURF2) ;
			}
			else
			{
				gDPSetRenderMode((*ppDLP)++,
										G_RM_PASS,
										G_RM_ZB_CLD_SURF2) ;
			}
			break ;
		case	G_RM_FOG_SHADE_A__G_RM_AA_ZB_XLU_DECAL2:
			gDPSetRenderMode((*ppDLP)++,
									G_RM_FOG_SHADE_A,
									G_RM_AA_ZB_XLU_DECAL2) ;
			break ;
		case	G_RM_FOG_SHADE_A__RM_AA_ZB_XLU_SURF_ZUPD:
			if (GetApp()->m_dwCheatFlags & CHEATFLAG_QUACKMODE)
			{
				gDPSetRenderMode((*ppDLP)++,
										G_RM_FOG_SHADE_A,
										G_RM_ZB_OPA_SURF2) ;
			}
			else
			{
				gDPSetRenderMode((*ppDLP)++,
										G_RM_FOG_SHADE_A,
										RM_AA_ZB_XLU_SURF_ZUPD(2)) ;
			}
			break ;
		case	G_RM_FOG_SHADE_A__G_RM_AA_ZB_TEX_EDGE2:
			gDPSetRenderMode((*ppDLP)++,
									G_RM_FOG_SHADE_A,
									G_RM_AA_ZB_TEX_EDGE2) ;
			break ;
		case	G_RM_FOG_SHADE_A__G_RM_ZB_OPA_DECAL2:
			gDPSetRenderMode((*ppDLP)++,
									G_RM_FOG_SHADE_A,
									G_RM_ZB_OPA_DECAL2) ;
			break ;
		case	G_RM_FOG_SHADE_A__G_RM_AA_ZB_OPA_INTER2:
			if (GetApp()->m_dwCheatFlags & CHEATFLAG_QUACKMODE)
			{
				gDPSetRenderMode((*ppDLP)++,
										G_RM_FOG_SHADE_A,
										G_RM_ZB_OPA_SURF2) ;
			}
			else
			{
				gDPSetRenderMode((*ppDLP)++,
										G_RM_FOG_SHADE_A,
										G_RM_AA_ZB_OPA_INTER2) ;
			}
			break ;
		case	G_RM_FOG_SHADE_A__G_RM_RA_ZB_OPA_INTER2:
			if (GetApp()->m_dwCheatFlags & CHEATFLAG_QUACKMODE)
			{
				gDPSetRenderMode((*ppDLP)++,
										G_RM_FOG_SHADE_A,
										G_RM_ZB_OPA_SURF2) ;
			}
			else
			{
				gDPSetRenderMode((*ppDLP)++,
										G_RM_FOG_SHADE_A,
										G_RM_RA_ZB_OPA_INTER2) ;
			}
			break ;
		case	G_RM_FOG_SHADE_A__G_RM_AA_ZB_OPA_SURF2:
			if (GetApp()->m_dwCheatFlags & CHEATFLAG_QUACKMODE)
			{
				gDPSetRenderMode((*ppDLP)++,
										G_RM_FOG_SHADE_A,
										G_RM_ZB_OPA_SURF2) ;
			}
			else
			{
				gDPSetRenderMode((*ppDLP)++,
										G_RM_FOG_SHADE_A,
										G_RM_AA_ZB_OPA_SURF2) ;
			}
			break ;
		case	G_RM_FOG_SHADE_A__G_RM_RA_ZB_OPA_SURF2:
			if (GetApp()->m_dwCheatFlags & CHEATFLAG_QUACKMODE)
			{
				gDPSetRenderMode((*ppDLP)++,
										G_RM_FOG_SHADE_A,
										G_RM_ZB_OPA_SURF2) ;
			}
			else
			{
				gDPSetRenderMode((*ppDLP)++,
										G_RM_FOG_SHADE_A,
										G_RM_RA_ZB_OPA_SURF2) ;
			}
			break ;

		// scene
		case	G_RM_AA_ZB_XLU_INTER__G_RM_AA_ZB_XLU_INTER2:
			gDPSetRenderMode((*ppDLP)++,
									G_RM_AA_ZB_XLU_INTER,
									G_RM_AA_ZB_XLU_INTER2) ;
			break ;

		case	G_RM_ZB_CLD_SURF__G_RM_ZB_CLD_SURF2:
			gDPSetRenderMode((*ppDLP)++,
									G_RM_ZB_CLD_SURF,
									G_RM_ZB_CLD_SURF2) ;
			break ;

		// fx
		case	G_RM_AA_ZB_XLU_SURF__G_RM_AA_ZB_XLU_SURF2:
			gDPSetRenderMode((*ppDLP)++,
									G_RM_AA_ZB_XLU_SURF,
									G_RM_AA_ZB_XLU_SURF2) ;
			break ;

		// PARTICLE
		case	G_RM_ZB_XLU_DECAL__G_RM_ZB_XLU_DECAL2:
			gDPSetRenderMode((*ppDLP)++,
									G_RM_ZB_XLU_DECAL,
									G_RM_ZB_XLU_DECAL2) ;
			break ;
		case	RM_ROB_ZB_PCL_SURF_BLEND1__RM_ROB_ZB_PCL_SURF_BLEND2:
			gDPSetRenderMode((*ppDLP)++,
									RM_ROB_ZB_PCL_SURF_BLEND(1),
									RM_ROB_ZB_PCL_SURF_BLEND(2));
			break ;
		case	G_RM_AA_ZB_TEX_EDGE__G_RM_AA_ZB_TEX_EDGE2:
			gDPSetRenderMode((*ppDLP)++,
									G_RM_AA_ZB_TEX_EDGE,
									G_RM_AA_ZB_TEX_EDGE2) ;
			break ;

		case	G_RM_AA_TEX_EDGE__G_RM_AA_TEX_EDGE2:
			gDPSetRenderMode((*ppDLP)++,
									G_RM_AA_TEX_EDGE,
									G_RM_AA_TEX_EDGE2) ;
			break ;

		case	G_RM_CLD_SURF__G_RM_CLD_SURF2:
			gDPSetRenderMode((*ppDLP)++,
									G_RM_CLD_SURF,
									G_RM_CLD_SURF2) ;
			break ;


		// tengine
		case	G_RM_NOOP__G_RM_NOOP2:
			gDPSetRenderMode((*ppDLP)++,
									G_RM_NOOP,
									G_RM_NOOP2) ;
			break ;
		case	G_RM_XLU_SURF__G_RM_XLU_SURF2:
			gDPSetRenderMode((*ppDLP)++,
									G_RM_XLU_SURF,
									G_RM_XLU_SURF2) ;
			break ;
		case	G_RM_VISCVG__G_RM_VISCVG2:
			gDPSetRenderMode((*ppDLP)++,
									G_RM_VISCVG,
									G_RM_VISCVG2);
			break ;
		// ROMSTRUC
		case	G_RM_FOG_SHADE_A__G_RM_ZB_CLD_SURF2:
			gDPSetRenderMode((*ppDLP)++,
									G_RM_FOG_SHADE_A,
									G_RM_ZB_CLD_SURF2) ;
			break ;
		case	G_RM_FOG_SHADE_A__G_RM_ZB_XLU_DECAL2:
			gDPSetRenderMode((*ppDLP)++,
									G_RM_FOG_SHADE_A,
									G_RM_ZB_XLU_DECAL2) ;
			break ;
		// ONSCRN
		case	G_RM_PASS__G_RM_XLU_SURF2:
			gDPSetRenderMode((*ppDLP)++,
									G_RM_PASS,
									G_RM_XLU_SURF2) ;
			break ;
		case	G_RM_PASS__G_RM_CLD_SURF2:
			gDPSetRenderMode((*ppDLP)++,
									G_RM_PASS,
									G_RM_CLD_SURF2) ;
			break ;


		default:
			ASSERT(FALSE);
			break;
	}
	return TRUE ;
}




/////////////////////////////////////////////////////////////////////////////
void CGeometry__DrawPart(CGeometry *pThis, Gfx **ppDLP, CCartCache *pCache, int nNode, int nPart, BOOL Inter, BOOL Better, int nFrame)
{
	CIndexedSet		isGeometry,
						isNodes, isNode,
						isParts, isPart,
						isSection;
	CUnindexedSet	usPartIndices;
	void				*pbNodes, *pbNode,
						*pbPartIndices,
						*pbParts, *pbPart,
						*pbSection;
	DWORD				*partIndices,
						dlistRemain;
	int				nPartIndices, cSection, nSections;

//	if (GetApp()->m_dwCheatFlags & CHEATFLAG_PENANDINKMODE)
//		Better = TRUE;


	dlistRemain = GLIST_LEN - (((DWORD)*ppDLP) - ((DWORD)CEngineApp__GetFrameData(GetApp())->m_pDisplayList))/sizeof(Gfx);
	if (dlistRemain < DISPLAY_LIST_RESERVE_FRONTEND)
		return;

	if (nPart < 0)
		return;

	CIndexedSet__ConstructFromRawData(&isGeometry,
												 CCacheEntry__GetData(pThis->m_pceGeometry),
												 FALSE);

	pbNodes = CIndexedSet__GetBlock(&isGeometry, CART_GEOMETRY_isNodes);
	CIndexedSet__ConstructFromRawData(&isNodes, pbNodes, FALSE);

	if (CIndexedSet__GetBlockCount(&isNodes))
	{
		ASSERT(nNode < CIndexedSet__GetBlockCount(&isNodes));

		pbNode = CIndexedSet__GetBlock(&isNodes, nNode);
		CIndexedSet__ConstructFromRawData(&isNode, pbNode, FALSE);

		pbPartIndices = CIndexedSet__GetBlock(&isNode, CART_NODE_usPartIndices);

   	// parts are indirected through indices
		CUnindexedSet__ConstructFromRawData(&usPartIndices, pbPartIndices, FALSE);
		nPartIndices = CUnindexedSet__GetBlockCount(&usPartIndices);

		if (nPartIndices)
		{
   		partIndices = (DWORD*) CUnindexedSet__GetBasePtr(&usPartIndices);

   		pbParts = CIndexedSet__GetBlock(&isNode, CART_NODE_isParts);

   		// all parts in node
			CIndexedSet__ConstructFromRawData(&isParts, pbParts, FALSE);

			// single part
   		pbPart = CIndexedSet__GetBlock(&isParts, partIndices[nPart%nPartIndices]);
			CIndexedSet__ConstructFromRawData(&isPart, pbPart, FALSE);

			nSections = CIndexedSet__GetBlockCount(&isPart);
			for (cSection=0; cSection<nSections; cSection++)
			{
				pbSection = CIndexedSet__GetBlock(&isPart, cSection);
				CIndexedSet__ConstructFromRawData(&isSection, pbSection, FALSE);

				CGeometry__DrawSection(pThis, ppDLP, &isSection, pCache, Inter, Better, nFrame);

				CIndexedSet__Destruct(&isSection);
			}

			CIndexedSet__Destruct(&isParts);
			CIndexedSet__Destruct(&isPart);
		}

		CIndexedSet__Destruct(&isNode);
		CUnindexedSet__Destruct(&usPartIndices);
	}

	CIndexedSet__Destruct(&isGeometry);
	CIndexedSet__Destruct(&isNodes);
}

void CGeometry__DrawSection(CGeometry *pThis, Gfx **ppDLP, CIndexedSet *pisSection, CCartCache *pCache, BOOL Inter, BOOL Better, int nFrame)
{
	void					*pbDispList;
	CGameSection		*pGameSect;
	DWORD					geomMode;
	unsigned int		tileU, tileV;
	CTextureInfo		textureInfo;
	int					nBitmap, nPalette,
							multU, multV;
	DWORD					newRenderMode;

	//Inter = FALSE;
	//Better = FALSE;
	newRenderMode = (DWORD) -1;

	// in decompressed section, block 0 is the CGameSection struct
	pGameSect = (CGameSection*) CIndexedSet__GetBlock(pisSection, 0);

	if (pGameSect->m_TextureLoader.m_rpTextureSet)
	{
		if (!CTextureLoader__RequestTextureSet(&pGameSect->m_TextureLoader, pCache, "Geometry Texture"))
			return;

		CTextureLoader__GetTextureInfo(&pGameSect->m_TextureLoader, &textureInfo);

		if (nFrame < 0)
			nFrame = game_frame_number/textureInfo.m_pFormat->m_PlaybackSpeed;

		nBitmap = nFrame % textureInfo.m_nBitmaps;
		if (textureInfo.m_nPalettes)
			nPalette = nFrame % textureInfo.m_nPalettes;
		else
			nPalette = 0;

		if (pGameSect->m_dwMatFlags & MATERIAL_REFLECT_MAP)
		{
			multU = (1 << (6 + textureInfo.m_pFormat->m_WidthShift));
			multV = (1 << (6 + textureInfo.m_pFormat->m_HeightShift));
		}
		else
		{
			multU = pGameSect->m_MultU;
			multV = pGameSect->m_MultV;
		}

		if (pGameSect->m_dwMatFlags & MATERIAL_DECAL_TEXTURE)
		{
			tileU = G_TX_CLAMP;
			tileV = G_TX_CLAMP;
		}
		else
		{
			tileU = G_TX_WRAP;
			tileV = G_TX_WRAP;
		}

		if (pGameSect->m_dwMatFlags & MATERIAL_MIRROR_TEXTURE)
		{
			tileU |= G_TX_MIRROR;
			tileV |= G_TX_MIRROR;
		}


		CTextureLoader__LoadTexture(&pGameSect->m_TextureLoader, ppDLP,
											 nBitmap, nPalette,
											 multU, multV,
											 tileU, tileV,
											 G_TX_NOLOD);
		if (textureInfo.m_pFormat->m_Format == TEXTURE_FORMAT_MM16RGBA)
		{
			gDPSetTextureLOD((*ppDLP)++, G_TL_LOD);
		}


		gDPSetPrimColor((*ppDLP)++,
							 textureInfo.m_pFormat->m_Effect, 0,	// mip-map parameters
						 	 pGameSect->m_Color[0],
						 	 pGameSect->m_Color[1],
						 	 pGameSect->m_Color[2],
						 	 pGameSect->m_Color[3]);

		if (pGameSect->m_dwMatFlags & MATERIAL_PSEUDOCOLOR)
		{
			gDPSetEnvColor((*ppDLP)++,
								pGameSect->m_BlackColor[0],
								pGameSect->m_BlackColor[1],
								pGameSect->m_BlackColor[2],
								(fx_mode == FXMODE_NONE) ? pGameSect->m_Color[3] : fx_color[3]);

			if (pGameSect->m_dwMatFlags & MATERIAL_SELF_ILLUMINATED)
			{
				if (pGameSect->m_dwMatFlags & MATERIAL_SHADE_ALPHA)
				{
					if (fx_mode == FXMODE_NONE)
					{
						CGeometry__SetCombineMode(ppDLP, G_CC_ROB_SELFILLUM_PSEUDO_SHADEALPHA__G_CC_PASS2);
					}
					else
					{
						// assume FXMODE_TOTRANSPARENT to keep things simple
						CGeometry__SetCombineMode(ppDLP, G_CC_ROB_SELFILLUM_PSEUDO_SHADEALPHA__G_CC_FXMODE_TOTRANSPARENT);
					}
				}
				else
				{
					CGeometry__SetCombineMode(ppDLP, G_CC_ROB_SELFILLUM_PSEUDO__G_CC_PASS2);
				}
			}
			else
			{
				if (pGameSect->m_dwMatFlags & MATERIAL_SHADE_ALPHA)
				{
					// can't use FXMODE's at the same time as lighted pseudocolor
					CGeometry__SetCombineMode(ppDLP, G_CC_ROB_SELFILLUM_PSEUDO_SHADEALPHA__G_CC_ROB_LIGHTED2);
				}
				else
				{
					CGeometry__SetCombineMode(ppDLP, G_CC_ROB_SELFILLUM_PSEUDO__G_CC_ROB_LIGHTED2);
				}
			}
		}
		else
		{
			if (pGameSect->m_dwMatFlags & MATERIAL_SELF_ILLUMINATED)
			{
				if (textureInfo.m_pFormat->m_Format == TEXTURE_FORMAT_MM16RGBA)
				{
					CGeometry__SetCombineMode(ppDLP, G_CC_TRILERP__G_CC_DECALRGB2);
				}
				else if (fx_mode == FXMODE_NONE)
				{
					CGeometry__SetCombineMode(ppDLP, G_CC_ROB_SELFILLUM_TEXTURE__G_CC_PASS2);
				}
				else
				{
					gDPSetEnvColor((*ppDLP)++,
										fx_color[0], fx_color[1], fx_color[2], fx_color[3]);

					switch (fx_mode)
					{
						case FXMODE_TOTRANSPARENT:
						case FXMODE_TOTRANSPARENT_NOISE:
							CGeometry__SetCombineMode(ppDLP, G_CC_ROB_SELFILLUM_TEXTURE__G_CC_FXMODE_TOTRANSPARENT);
							break;

						case FXMODE_TOCOLOR:
							CGeometry__SetCombineMode(ppDLP, G_CC_ROB_SELFILLUM_TEXTURE__G_CC_FXMODE_TOCOLOR);
							break;

						case FXMODE_GLARE:
							CGeometry__SetCombineMode(ppDLP, G_CC_ROB_SELFILLUM_TEXTURE__G_CC_FXMODE_GLARE);
							break;

						case FXMODE_FROMCOLOR_TOTRANSPARENT:
						case FXMODE_FROMCOLOR_TOTRANSPARENT_NOISE:
							CGeometry__SetCombineMode(ppDLP, G_CC_ROB_SELFILLUM_TEXTURE__G_CC_FXMODE_FROMCOLOR_TOTRANSPARENT);
							break;

						case FXMODE_TOCOLORANDTRANSPARENT:
							gDPSetPrimColor((*ppDLP)++,
						 						 0, 0,						// LOD stuff
						 						 pGameSect->m_Color[0],
						 						 pGameSect->m_Color[1],
						 						 pGameSect->m_Color[2],
						 						 fx_color[4]);
							CGeometry__SetCombineMode(ppDLP, G_CC_ROB_SELFILLUM_TEXTURE__G_CC_FXMODE_TOCOLORANDTRANSPARENT);
							break;

						default:
							ASSERT(FALSE);
							break;
					}
				}
			}
			else
			{
				if (textureInfo.m_pFormat->m_Format == TEXTURE_FORMAT_MM16RGBA)
				{
					CGeometry__SetCombineMode(ppDLP, G_CC_TRILERP__G_CC_MODULATERGB2);
				}
				else if (fx_mode == FXMODE_NONE)
				{
					CGeometry__SetCombineMode(ppDLP, G_CC_ROB_LIGHTED_TEXTURE__G_CC_FXMODE_NONE);
				}
				else
				{
					gDPSetEnvColor((*ppDLP)++,
										fx_color[0], fx_color[1], fx_color[2], fx_color[3]);

					switch (fx_mode)
					{
						case FXMODE_TOTRANSPARENT:
						case FXMODE_TOTRANSPARENT_NOISE:
							CGeometry__SetCombineMode(ppDLP, G_CC_ROB_LIGHTED_TEXTURE__G_CC_FXMODE_TOTRANSPARENT);
							break;

						case FXMODE_TOCOLOR:
							CGeometry__SetCombineMode(ppDLP, G_CC_ROB_LIGHTED_TEXTURE__G_CC_FXMODE_TOCOLOR);
							break;

						case FXMODE_GLARE:
							CGeometry__SetCombineMode(ppDLP, G_CC_ROB_SELFILLUM_TEXTURE__G_CC_FXMODE_GLARE);
							break;

						case FXMODE_FROMCOLOR_TOTRANSPARENT:
						case FXMODE_FROMCOLOR_TOTRANSPARENT_NOISE:
							CGeometry__SetCombineMode(ppDLP, G_CC_ROB_LIGHTED_TEXTURE__G_CC_FXMODE_FROMCOLOR_TOTRANSPARENT);
							break;

						case FXMODE_TOCOLORANDTRANSPARENT:
							gDPSetPrimColor((*ppDLP)++,
						 						 0, 0,						// LOD stuff
						 						 pGameSect->m_Color[0],
						 						 pGameSect->m_Color[1],
						 						 pGameSect->m_Color[2],
						 						 fx_color[4]);
							CGeometry__SetCombineMode(ppDLP, G_CC_ROB_LIGHTED_TEXTURE__G_CC_FXMODE_TOCOLORANDTRANSPARENT);
							break;

						default:
							ASSERT(FALSE);
							break;
					}
				}
			}
		}
	}
	else
	{
		gDPPipeSync((*ppDLP)++);

		gDPSetPrimColor((*ppDLP)++,
						 	0, 0,						// LOD stuff
						 	pGameSect->m_Color[0],
						 	pGameSect->m_Color[1],
						 	pGameSect->m_Color[2],
						 	pGameSect->m_Color[3]);

		gSPTexture((*ppDLP)++, 0, 0, 0, 0, G_OFF);

		if (pGameSect->m_dwMatFlags & MATERIAL_SELF_ILLUMINATED)
		{
			CGeometry__SetCombineMode(ppDLP, G_CC_ROB_SELFILLUM_SOLID__G_CC_PASS2);
		}
		else
		{

			if (fx_mode == FXMODE_NONE)
			{
				CGeometry__SetCombineMode(ppDLP, G_CC_ROB_LIGHTED_SOLID__G_CC_FXMODE_NONE);
			}
			else
			{
				gDPSetEnvColor((*ppDLP)++,
									fx_color[0], fx_color[1], fx_color[2], fx_color[3]);

				switch (fx_mode)
				{
					case FXMODE_TOTRANSPARENT:
					case FXMODE_TOTRANSPARENT_NOISE:
						CGeometry__SetCombineMode(ppDLP, G_CC_ROB_LIGHTED_SOLID__G_CC_FXMODE_TOTRANSPARENT);
						break;

					case FXMODE_TOCOLOR:
						CGeometry__SetCombineMode(ppDLP, G_CC_ROB_LIGHTED_SOLID__G_CC_FXMODE_TOCOLOR);
						break;

					case FXMODE_GLARE:
						CGeometry__SetCombineMode(ppDLP, G_CC_ROB_SELFILLUM_SOLID__G_CC_FXMODE_GLARE);
						break;

					case FXMODE_FROMCOLOR_TOTRANSPARENT:
					case FXMODE_FROMCOLOR_TOTRANSPARENT_NOISE:
						CGeometry__SetCombineMode(ppDLP, G_CC_ROB_LIGHTED_SOLID__G_CC_FXMODE_FROMCOLOR_TOTRANSPARENT);
						break;

					case FXMODE_TOCOLORANDTRANSPARENT:
						gDPSetPrimColor((*ppDLP)++,
						 					 0, 0,						// LOD stuff
						 					 pGameSect->m_Color[0],
						 					 pGameSect->m_Color[1],
						 					 pGameSect->m_Color[2],
						 					 fx_color[4]);
						CGeometry__SetCombineMode(ppDLP, G_CC_ROB_LIGHTED_TEXTURE__G_CC_FXMODE_TOCOLORANDTRANSPARENT);
						break;

					default:
						ASSERT(FALSE);
						break;
				}
			}
		}
	}

	// Draw transparent materials?
	if ((pGameSect->m_dwMatFlags & MATERIAL_TRANSPARENCY) && (!draw_transparent_geometry))
		return ;

	// Transparent?
	if (		(pGameSect->m_dwMatFlags & MATERIAL_TRANSPARENCY)
			|| (fx_mode == FXMODE_TOTRANSPARENT)
			|| (fx_mode == FXMODE_FROMCOLOR_TOTRANSPARENT)
			|| (fx_mode == FXMODE_TOCOLORANDTRANSPARENT) )
//			|| (fx_mode == FXMODE_TOTRANSPARENT_NOISE)
//			|| (fx_mode == FXMODE_FROMCOLOR_TOTRANSPARENT_NOISE) )
	{
		if (fx_mode == FXMODE_NONE)
		{
			if (pGameSect->m_dwMatFlags & MATERIAL_SHADE_ALPHA)
			{
				newRenderMode = G_RM_PASS__G_RM_ZB_CLD_SURF2;
			}
			else
			{
				if (pGameSect->m_dwMatFlags & MATERIAL_INTERSECT)
				{
					newRenderMode = G_RM_FOG_SHADE_A__G_RM_AA_ZB_XLU_DECAL2;
				}
				else
				{
					newRenderMode = G_RM_FOG_SHADE_A__G_RM_AA_ZB_XLU_SURF2;
				}
			}
		}
		else
		{
			if (pGameSect->m_dwMatFlags & MATERIAL_SHADE_ALPHA)
			{
				// can't have fog and alpha shading at the same time
				newRenderMode = G_RM_PASS__G_RM_ZB_CLD_SURF2;
			}
			else
			{
				newRenderMode = G_RM_FOG_SHADE_A__RM_AA_ZB_XLU_SURF_ZUPD;
			}
		}
	}
	else
	{
		if (pGameSect->m_dwMatFlags & MATERIAL_MASK)
		//if (		(pGameSect->m_dwMatFlags & MATERIAL_MASK)
		//		|| (fx_mode == FXMODE_TOTRANSPARENT_NOISE)
		//		|| (fx_mode == FXMODE_FROMCOLOR_TOTRANSPARENT_NOISE) )
		{
			if (CGeometry__SetRenderMode(ppDLP, G_RM_FOG_SHADE_A__G_RM_AA_ZB_TEX_EDGE2))
			{
				gDPSetBlendColor((*ppDLP)++, 255, 255, 255, 1);
				gDPSetAlphaCompare((*ppDLP)++, G_AC_THRESHOLD);
			}
		}
		else
		{
			if (pGameSect->m_dwMatFlags & MATERIAL_INTERSECT)
			{
				if (!draw_intersect_geometry)
					return ;

				newRenderMode = G_RM_FOG_SHADE_A__G_RM_ZB_OPA_DECAL2;
			}
			else
			{
				if (Inter)
				{
					if (Better)
					{
						newRenderMode = G_RM_FOG_SHADE_A__G_RM_AA_ZB_OPA_INTER2;
					}
					else
					{
						newRenderMode = G_RM_FOG_SHADE_A__G_RM_RA_ZB_OPA_INTER2;
					}
				}
				else
				{
					if (Better)
					{
						newRenderMode = G_RM_FOG_SHADE_A__G_RM_AA_ZB_OPA_SURF2;
					}
					else
					{
						newRenderMode = G_RM_FOG_SHADE_A__G_RM_RA_ZB_OPA_SURF2;
					}
				}
			}
		}
	}

	if (newRenderMode != (DWORD) -1)
		CGeometry__SetRenderMode(ppDLP, newRenderMode);

	geomMode = G_SHADE | G_SHADING_SMOOTH | G_LIGHTING | G_CULL_BACK | G_TEXTURE_GEN;
	if (pGameSect->m_dwMatFlags & MATERIAL_SHADE_ALPHA)
		geomMode |= G_FOG;

	gSPClearGeometryMode((*ppDLP)++, geomMode);

	geomMode = 0;
	if (!(pGameSect->m_dwMatFlags & MATERIAL_SHADE_ALPHA))
		geomMode |= G_FOG;

	if (fx_mode == FXMODE_GLARE)
		geomMode |= G_LIGHTING;

	// light non-self-illuminated objects
	if (!(pGameSect->m_dwMatFlags & MATERIAL_SELF_ILLUMINATED))
	{
		// Light also in spirit mode!
		if (	 (!(pGameSect->m_dwMatFlags & MATERIAL_PRELIT))
			 || (   (GetApp()->m_Mode == MODE_GAME)
				  && (   (CTurokMovement.SpiritualTime > 0)
#ifndef KANJI
						|| (GetApp()->m_dwCheatFlags & CHEATFLAG_DISCO) ) ) )
#else
						)))
#endif
		{
			geomMode |= G_LIGHTING;
		}
	}

	// shading must always be on for fog
	geomMode |= G_SHADE | G_SHADING_SMOOTH;

	// only draw backs of double sided faces (normals will face wrong direction)
	if (!(pGameSect->m_dwMatFlags & MATERIAL_TWO_SIDED))
		geomMode |= G_CULL_BACK;

	// reflection maps
	if (pGameSect->m_dwMatFlags & MATERIAL_REFLECT_MAP)
		geomMode |= G_TEXTURE_GEN | G_LIGHTING;
		//geomMode |= G_TEXTURE_GEN | G_LIGHTING | G_TEXTURE_GEN_LINEAR;

	gSPSetGeometryMode((*ppDLP)++, geomMode);

	// in decompressed section, block 1 is the display list
	pbDispList = CIndexedSet__GetBlock(pisSection, 1);
	gSPDisplayList((*ppDLP)++, RSP_ADDRESS((BYTE*)pbDispList + 8));

	if (pGameSect->m_dwMatFlags & MATERIAL_MASK)
		gDPSetAlphaCompare((*ppDLP)++, G_AC_NONE);

	if (pGameSect->m_TextureLoader.m_rpTextureSet)
		if (textureInfo.m_pFormat->m_Format == TEXTURE_FORMAT_MM16RGBA)
			gDPSetTextureLOD((*ppDLP)++, G_TL_TILE);

	// for sparkle testing
	//gDPSetAlphaCompare((*ppDLP)++, G_AC_NONE);
}

void CGeometry__DrawShadow(Gfx **ppDLP, Mtx *pmShadow, BOOL Decal, BYTE Alpha)
{
	gDPLoadTextureBlock(((*ppDLP)++),
							  RDP_ADDRESS(shadow_texture), G_IM_FMT_I, G_IM_SIZ_8b,
							  16, 16, 0,
							  G_TX_CLAMP | G_TX_NOMIRROR, G_TX_CLAMP | G_TX_NOMIRROR,
							  G_TX_NOMASK, G_TX_NOMASK,
							  G_TX_NOLOD, G_TX_NOLOD);

	gDPSetPrimColor((*ppDLP)++,
						 256, 256,				// LOD stuff
						 255, 255, 255,
						 Alpha);

  	gSPMatrix((*ppDLP)++,
				 RSP_ADDRESS(pmShadow),
	   		 G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);

	gSPDisplayList((*ppDLP)++,
						RSP_ADDRESS(Decal ? decal_shadow_dl : shadow_dl));

	CGeometry__ResetDrawModes();
	CTextureLoader__InvalidateTextureCache();
}

void CGeometry__RequestBlock(CGeometry *pThis, CCartCache *pCache,
	  						 		  ROMAddress *rpGeometryAddress, DWORD GeometrySize,
									  CCacheEntry *pceTextureSetsIndex)
{
	// necessary for decompressing sections
	pThis->m_pceTextureSetsIndex = pceTextureSetsIndex;

	// make sure current version is used
	pThis->m_pceGeometry = NULL;

	// request geometry data (will be decompressed)
	CCartCache__RequestBlock(pCache,
									 pThis, (pfnCACHENOTIFY) CGeometry__Decompress, NULL,
									 &pThis->m_pceGeometry,
                            rpGeometryAddress, GeometrySize,
                            "Object Geometry");
}

//#define PRINT_VTX_LOADS
#define BOUNDS_CHECK_THRESHOLD	24
//#define BOUNDS_CHECK_THRESHOLD	16
void CGeometry__Decompress(CGeometry *pThis, CCacheEntry **ppceTarget)
{
	int				allocSize,
						dispListSize, verticesSize,
						cNode, nNodes,
						cPart, nParts,
						cSection, nSections,
						cVertex, nVertices,
						cVertexLoad, nVertexLoads,
						vertexCount,
						cFacet, nFacets, nFacetLoads, nThisFacetLoad,
						gameSectionSize,
						newSectionSize,
						offset,
						nodesOffset, nodeOffset,
						partsOffset, partOffset,
						sectionOffset;
	void				*pbGeometry,
						*pbGeometryInfo,
						*pbNodes, *pbNode,
						*pbChildNodeIndices,
						*pbPartIndices,
						*pbParts, *pbPart,
						*pbSection, *pbNewSection,
						*pbROMSection,
						*pbPStream,
						*pbVertices,
						*pbNewGeometry,
						*pbNewGeometryInfo,
						*pbNewNodes, *pbNewNode,
						*pbNewChildNodeIndices,
						*pbNewPartIndices,
						*pbNewParts, *pbNewPart,
						*pbDispList;
	CGameSection	*pbNewGameSection;
	CIndexedSet		isGeometry, isNewGeometry,
						isNodes, isNode,
						isParts, isPart,
						isSection, isNewSection,
						isNewPart, isNewParts,
						isNewNodes, isNewNode;
	CUnindexedSet	usVertices,
						usPStream;
	DWORD				childNodeIndicesSize,
						partIndicesSize,
						*pVertexCount;
	BYTE				*pBytes;
	Gfx 				*pDLP;
	Vtx				*vertices, *newVertices, *pVert, *boundsVertices;
	BYTE				*inBytes, *pOutPlace;
	int				cBlock, cSize;
	BOOL				boundsCheck;
	int				dir[3], dot;
	WORD				*facets;
	short				minX, minY, minZ, maxX, maxY, maxZ;
	WORD				poly, nextPoly;
	BOOL				last;
#ifdef IG_DEBUG
	OSTime			startTime, endTime, diffTime;
#endif

	ASSERT(pThis->m_pceGeometry == *ppceTarget);
	ASSERT(BOUNDS_CHECK_THRESHOLD >= 1);

	if (!CCacheEntry__DecompressTemp(*ppceTarget))
		return;

	TRACE0("CGeometry: Starting geometry decompression...\r\n");
#ifdef IG_DEBUG
   startTime = osGetTime();
#endif

	// calculate allocation size
	allocSize = 0;

	// geometry
	pbGeometry = CCacheEntry__GetData(pThis->m_pceGeometry);
	CIndexedSet__ConstructFromRawData(&isGeometry, pbGeometry, FALSE);
	allocSize += IS_INDEX_SIZE(CART_GEOMETRY_nItems);

	// geometry info
	pbGeometryInfo = CIndexedSet__GetBlock(&isGeometry, CART_GEOMETRY_info);
	allocSize += FixAlignment(sizeof(CGameGeometryInfo), IS_ALIGNMENT);

	// nodes
	pbNodes = CIndexedSet__GetBlock(&isGeometry, CART_GEOMETRY_isNodes);
	CIndexedSet__ConstructFromRawData(&isNodes, pbNodes, FALSE);
	nNodes = CIndexedSet__GetBlockCount(&isNodes);
	allocSize += IS_INDEX_SIZE(nNodes);

	for (cNode=0; cNode<nNodes; cNode++)
	{
		// node
		pbNode = CIndexedSet__GetBlock(&isNodes, cNode);
		CIndexedSet__ConstructFromRawData(&isNode, pbNode, FALSE);
		allocSize += IS_INDEX_SIZE(CART_NODE_nItems);

		// child node indices
		pbChildNodeIndices = CIndexedSet__GetBlockAndSize(&isNode, CART_NODE_usChildNodeIndices, &childNodeIndicesSize);
		allocSize += childNodeIndicesSize;

		// part indices
		pbPartIndices = CIndexedSet__GetBlockAndSize(&isNode, CART_NODE_usPartIndices, &partIndicesSize);
		allocSize += partIndicesSize;

		// parts
		pbParts = CIndexedSet__GetBlock(&isNode, CART_NODE_isParts);
		CIndexedSet__ConstructFromRawData(&isParts, pbParts, FALSE);
		nParts = CIndexedSet__GetBlockCount(&isParts);
		allocSize += IS_INDEX_SIZE(nParts);

		for (cPart=0; cPart<nParts; cPart++)
		{
			// part
			pbPart = CIndexedSet__GetBlock(&isParts, cPart);
			CIndexedSet__ConstructFromRawData(&isPart, pbPart, FALSE);
			nSections = CIndexedSet__GetBlockCount(&isPart);
			allocSize += IS_INDEX_SIZE(nSections);

			for (cSection=0; cSection<nSections; cSection++)
			{
				// section
				pbSection = CIndexedSet__GetBlock(&isPart, cSection);
				CIndexedSet__ConstructFromRawData(&isSection, pbSection, FALSE);
				allocSize += IS_INDEX_SIZE(2);

				// poly stream
				pbPStream = CIndexedSet__GetBlock(&isSection, CART_SECTION_usPStream);
				CUnindexedSet__ConstructFromRawData(&usPStream, pbPStream, FALSE);
				nFacets = CUnindexedSet__GetBlockCount(&usPStream);
				facets = (WORD*) CUnindexedSet__GetBasePtr(&usPStream);
#ifdef PLATFORM_PORT
				/* PORT: poly-stream facets are big-endian packed 16-bit WORDs (vtx indices bits 0/5/10,
				 * LASTINRUN bit 15). Swap ONCE here in the size pass; the build pass shares this blob.
				 * CGeometry__Decompress is a one-time decompress callback, so no re-entry double-swap. */
				{ int _f; for (_f = 0; _f < nFacets; _f++) facets[_f] = ORDERBYTES(facets[_f]); }
#endif

				// vertices
				pbVertices = CIndexedSet__GetBlock(&isSection, CART_SECTION_usVertices);
				CUnindexedSet__ConstructFromRawData(&usVertices, pbVertices, FALSE);
				nVertices = CUnindexedSet__GetBlockCount(&usVertices);

				TRACE2("CGeometry: nVertices:%u, nFacets:%u\r\n", nVertices, nFacets);

				// gameSection struct
				gameSectionSize = FixAlignment(sizeof(CGameSection), IS_ALIGNMENT);
				allocSize += gameSectionSize;

				// number of vertex loads required
				if (nVertices == 0)
				{
					nVertexLoads = 0;
				}
				else if (nVertices <= VERTEX_CACHE_SIZE)
				{
					nVertexLoads = 1;
				}
				else
				{
					nVertexLoads = 1 + ((nVertices - VERTEX_CACHE_SIZE) / (VERTEX_CACHE_SIZE/2));

					if ((nVertices - VERTEX_CACHE_SIZE) % (VERTEX_CACHE_SIZE/2))
						nVertexLoads++;
				}

				// number of polygon draw commands required
				nFacetLoads = nThisFacetLoad = 0;
				for (cFacet=0; cFacet<nFacets; cFacet++)
				{
					if (nThisFacetLoad)
					{
						nThisFacetLoad = 0;
						nFacetLoads++;
					}
					else
					{
						if (POLYSTREAM_ISLASTINRUN(facets[cFacet]))
							nFacetLoads++;
						else
							nThisFacetLoad++;
					}
				}

				dispListSize = (nVertexLoads + nFacetLoads + 1)*sizeof(Gfx);
				verticesSize = nVertices*sizeof(Vtx);

				// bounds check
				boundsCheck = (nNodes && (nVertices > BOUNDS_CHECK_THRESHOLD));
				if (boundsCheck)
				{
					dispListSize += 2*sizeof(Gfx);
					verticesSize += 8*sizeof(Vtx);
				}

				newSectionSize = 8 + dispListSize + verticesSize;
				newSectionSize = FixAlignment(newSectionSize, IS_ALIGNMENT);

				allocSize += newSectionSize;

				CIndexedSet__Destruct(&isSection);
				CUnindexedSet__Destruct(&usPStream);
				CUnindexedSet__Destruct(&usVertices);
			}

			CIndexedSet__Destruct(&isPart);
		}

		CIndexedSet__Destruct(&isParts);
		CIndexedSet__Destruct(&isNode);
	}

	// allocate block for decompressed data
	pBytes = CCacheEntry__AllocDataForReplacement(pThis->m_pceGeometry, allocSize);
	if (pBytes)
	{
		// for debugging
		//memset(pBytes, 0xcd, allocSize);

		// isNewGeometry
		pbNewGeometry = pBytes;
		CIndexedSet__ConstructFromRawData(&isNewGeometry, pbNewGeometry, FALSE);

		// isNewGeometry index
		offset = IS_INDEX_SIZE(CART_GEOMETRY_nItems);
		CIndexedSet__SetBlockCount(&isNewGeometry, 0);
		CIndexedSet__SetNextOffset(&isNewGeometry, offset);

		// scaler
		pbNewGeometryInfo = pBytes + offset;
		CGameGeometryInfo__TakeFromROMGeometryInfo((CGameGeometryInfo*) pbNewGeometryInfo, (CROMGeometryInfo*) pbGeometryInfo);

		offset += FixAlignment(sizeof(CGameGeometryInfo), IS_ALIGNMENT);
		CIndexedSet__SetBlockCount(&isNewGeometry, 1);
		CIndexedSet__SetNextOffset(&isNewGeometry, offset);

		// isNewNodes
		pbNewNodes = pBytes + offset;
		CIndexedSet__ConstructFromRawData(&isNewNodes, pbNewNodes, FALSE);

		// isNewNodes index
		nodesOffset = IS_INDEX_SIZE(nNodes);
		CIndexedSet__SetBlockCount(&isNewNodes, 0);
		CIndexedSet__SetNextOffset(&isNewNodes, nodesOffset);

		for (cNode=0; cNode<nNodes; cNode++)
		{
			// isNode
			pbNode = CIndexedSet__GetBlock(&isNodes, cNode);
			CIndexedSet__ConstructFromRawData(&isNode, pbNode, FALSE);

			// isNewNode
			pbNewNode = pBytes + offset + nodesOffset;
			CIndexedSet__ConstructFromRawData(&isNewNode, pbNewNode, FALSE);

			// isNewNode index
			nodeOffset = IS_INDEX_SIZE(CART_NODE_nItems);
			CIndexedSet__SetBlockCount(&isNewNode, 0);
			CIndexedSet__SetNextOffset(&isNewNode, nodeOffset);

			// usChildNodeIndices
			pbChildNodeIndices = CIndexedSet__GetBlockAndSize(&isNode, CART_NODE_usChildNodeIndices, &childNodeIndicesSize);

			// usNewChildNodeIndices
			pbNewChildNodeIndices = pBytes + offset + nodesOffset + nodeOffset;
			memcpy(pbNewChildNodeIndices, pbChildNodeIndices, childNodeIndicesSize);
#ifdef PLATFORM_PORT
			/* PORT: DWORD child-node indices are big-endian; DoDraw recurses with them as nNode.
			 * Swap the DATA only (skip the US header — swapping it corrupts the block-count). */
			{ CUnindexedSet _us; int _n,_i; DWORD *_d; CUnindexedSet__ConstructFromRawData(&_us,pbNewChildNodeIndices,FALSE);
			  _n=CUnindexedSet__GetBlockCount(&_us); _d=(DWORD*)CUnindexedSet__GetBasePtr(&_us);
			  for(_i=0;_i<_n;_i++) _d[_i]=ORDERBYTES(_d[_i]); }
#endif

			nodeOffset += childNodeIndicesSize;
			CIndexedSet__SetBlockCount(&isNewNode, 1);
			CIndexedSet__SetNextOffset(&isNewNode, nodeOffset);

			// usPartIndices
			pbPartIndices = CIndexedSet__GetBlockAndSize(&isNode, CART_NODE_usPartIndices, &partIndicesSize);

			// usNewChildNodeIndices
			pbNewPartIndices = pBytes + offset + nodesOffset + nodeOffset;
			memcpy(pbNewPartIndices, pbPartIndices, partIndicesSize);
#ifdef PLATFORM_PORT
			/* PORT: DWORD part indices are big-endian (DrawPart partIndices[...]). Swap DATA only. */
			{ CUnindexedSet _us; int _n,_i; DWORD *_d; CUnindexedSet__ConstructFromRawData(&_us,pbNewPartIndices,FALSE);
			  _n=CUnindexedSet__GetBlockCount(&_us); _d=(DWORD*)CUnindexedSet__GetBasePtr(&_us);
			  for(_i=0;_i<_n;_i++) _d[_i]=ORDERBYTES(_d[_i]); }
#endif

			nodeOffset += partIndicesSize;
			CIndexedSet__SetBlockCount(&isNewNode, 2);
			CIndexedSet__SetNextOffset(&isNewNode, nodeOffset);

			// isParts
			pbParts = CIndexedSet__GetBlock(&isNode, CART_NODE_isParts);
			CIndexedSet__ConstructFromRawData(&isParts, pbParts, FALSE);
			nParts = CIndexedSet__GetBlockCount(&isParts);

			// isNewParts
			pbNewParts = pBytes + offset + nodesOffset + nodeOffset;
			CIndexedSet__ConstructFromRawData(&isNewParts, pbNewParts, FALSE);

			// isNewParts index
			partsOffset = IS_INDEX_SIZE(nParts);
			CIndexedSet__SetBlockCount(&isNewParts, 0);
			CIndexedSet__SetNextOffset(&isNewParts, partsOffset);

			for (cPart=0; cPart<nParts; cPart++)
			{
				// isPart
				pbPart = CIndexedSet__GetBlock(&isParts, cPart);
				CIndexedSet__ConstructFromRawData(&isPart, pbPart, FALSE);
				nSections = CIndexedSet__GetBlockCount(&isPart);

				// isNewPart
				pbNewPart = pBytes + offset + nodesOffset + nodeOffset + partsOffset;
				CIndexedSet__ConstructFromRawData(&isNewPart, pbNewPart, FALSE);

				// isNewPart index
				partOffset = IS_INDEX_SIZE(nSections);
				CIndexedSet__SetBlockCount(&isNewPart, 0);
				CIndexedSet__SetNextOffset(&isNewPart, partOffset);

				for (cSection=0; cSection<nSections; cSection++)
				{
					// isSection
					pbSection = CIndexedSet__GetBlock(&isPart, cSection);
					CIndexedSet__ConstructFromRawData(&isSection, pbSection, FALSE);

					// isNewSection
					pbNewSection = pBytes + offset + nodesOffset + nodeOffset + partsOffset + partOffset;
					CIndexedSet__ConstructFromRawData(&isNewSection, pbNewSection, FALSE);

					// isNewSection index
					sectionOffset = IS_INDEX_SIZE(2);
					CIndexedSet__SetBlockCount(&isNewSection, 0);
					CIndexedSet__SetNextOffset(&isNewSection, sectionOffset);

					// romSection
               pbROMSection = CIndexedSet__GetBlock(&isSection, CART_SECTION_section);

               // newGameSection
               pbNewGameSection = (CGameSection*) (pBytes + offset + nodesOffset + nodeOffset + partsOffset + partOffset + sectionOffset);

               CGameSection__TakeFromRomSection(pbNewGameSection, pbROMSection,
																pThis->m_pceTextureSetsIndex);

					gameSectionSize = FixAlignment(sizeof(CGameSection), IS_ALIGNMENT);
         		sectionOffset += gameSectionSize;
         		CIndexedSet__SetBlockCount(&isNewSection, 1);
         		CIndexedSet__SetNextOffset(&isNewSection, sectionOffset);

					// poly stream
					pbPStream = CIndexedSet__GetBlock(&isSection, CART_SECTION_usPStream);
					CUnindexedSet__ConstructFromRawData(&usPStream, pbPStream, FALSE);
					nFacets = CUnindexedSet__GetBlockCount(&usPStream);
					facets = (WORD*) CUnindexedSet__GetBasePtr(&usPStream);

					// vertices
					pbVertices = CIndexedSet__GetBlock(&isSection, CART_SECTION_usVertices);
					CUnindexedSet__ConstructFromRawData(&usVertices, pbVertices, FALSE);
					nVertices = CUnindexedSet__GetBlockCount(&usVertices);
					vertices = (Vtx*) CUnindexedSet__GetBasePtr(&usVertices);

					// number of vertex loads required
					if (nVertices == 0)
					{
						nVertexLoads = 0;
					}
					else if (nVertices <= VERTEX_CACHE_SIZE)
					{
						nVertexLoads = 1;
					}
					else
					{
						nVertexLoads = 1 + ((nVertices - VERTEX_CACHE_SIZE) / (VERTEX_CACHE_SIZE/2));

						if ((nVertices - VERTEX_CACHE_SIZE) % (VERTEX_CACHE_SIZE/2))
							nVertexLoads++;
					}

					// number of polygon draw commands required
					nFacetLoads = nThisFacetLoad = 0;
					for (cFacet=0; cFacet<nFacets; cFacet++)
					{
						if (nThisFacetLoad)
						{
							nFacetLoads++;
							nThisFacetLoad = 0;
						}
						else
						{
							if (POLYSTREAM_ISLASTINRUN(facets[cFacet]))
								nFacetLoads++;
							else
								nThisFacetLoad++;
						}
					}

   				dispListSize = (nVertexLoads + nFacetLoads + 1)*sizeof(Gfx);
   				verticesSize = nVertices*sizeof(Vtx);

					// bounds check
					boundsCheck = (nNodes && (nVertices > BOUNDS_CHECK_THRESHOLD));
					if (boundsCheck)
					{
						dispListSize += 2*sizeof(Gfx);
						verticesSize += 8*sizeof(Vtx);
					}

   				newSectionSize = 8 + dispListSize + verticesSize;
   				newSectionSize = FixAlignment(newSectionSize, IS_ALIGNMENT);

   				// vertex count
					pVertexCount = (DWORD*) (pBytes + offset + nodesOffset + nodeOffset + partsOffset + partOffset + sectionOffset);
					*pVertexCount = nVertices;

					// display list
               pbDispList = ((BYTE*)pVertexCount) + 8;

   				// new vertices
   				newVertices = (Vtx*) (((BYTE*)pbDispList) + dispListSize);
					// bounds check
					if (boundsCheck)
					{
						boundsVertices = newVertices;
						newVertices += 8;
					}

					// vertex address
					pVertexCount++;
					*pVertexCount = (DWORD) newVertices;

					// deinterleave and copy outside of CUnindexedSet
					pOutPlace = (BYTE*) newVertices;
					inBytes = (BYTE*) vertices;
					for (cBlock=0; cBlock<nVertices; cBlock++)
					{
						for (cSize=0; cSize<sizeof(Vtx); cSize++)
						{
							*pOutPlace = inBytes[cSize*nVertices + cBlock];
							pOutPlace++;
						}
					}
#ifdef PLATFORM_PORT
					/* PORT: the de-interleave is a pure byte-plane transpose (no byte swap). Each Vtx's
					 * six 16-bit fields (ob[0..2] pos, flag, tc[0..1]) are big-endian — swap them. The
					 * cn[4]/n[3]/a fields are single BYTEs (shared union tail) and stay raw. */
					for (cBlock=0; cBlock<nVertices; cBlock++)
					{
						Vtx *vt = &newVertices[cBlock];
						vt->v.ob[0]=ORDERBYTES(vt->v.ob[0]); vt->v.ob[1]=ORDERBYTES(vt->v.ob[1]); vt->v.ob[2]=ORDERBYTES(vt->v.ob[2]);
						vt->v.flag =ORDERBYTES(vt->v.flag);
						vt->v.tc[0]=ORDERBYTES(vt->v.tc[0]); vt->v.tc[1]=ORDERBYTES(vt->v.tc[1]);
					}
#endif

					// light vertices
					if (		(pbNewGameSection->m_dwMatFlags & MATERIAL_PRELIT)
							&& !(GetApp()->m_dwCheatFlags & CHEATFLAG_PURDYCOLORS) )
					{
						switch (CACHE_BLOCK_GETVERSION(CCacheEntry__GetRequestedAddress(pThis->m_pceGeometry)))
						{
							case 0:
								// rotate 0
								dir[0] = GetApp()->m_Scene.m_LightDirection[0];
								dir[1] = GetApp()->m_Scene.m_LightDirection[1];
								dir[2] = GetApp()->m_Scene.m_LightDirection[2];
								break;

							case 1:
								// rotate 90
								dir[0] = -GetApp()->m_Scene.m_LightDirection[2];
								dir[1] = GetApp()->m_Scene.m_LightDirection[1];
								dir[2] = GetApp()->m_Scene.m_LightDirection[0];
								break;

							case 2:
								// rotate 180
								dir[0] = -GetApp()->m_Scene.m_LightDirection[0];
								dir[1] = GetApp()->m_Scene.m_LightDirection[1];
								dir[2] = -GetApp()->m_Scene.m_LightDirection[2];
								break;

							case 3:
								// rotate 270
								dir[0] = GetApp()->m_Scene.m_LightDirection[2];
								dir[1] = GetApp()->m_Scene.m_LightDirection[1];
								dir[2] = -GetApp()->m_Scene.m_LightDirection[0];
								break;

							default:
								ASSERT(FALSE);
								break;
						}

						for (cVertex=0; cVertex<nVertices; cVertex++)
						{
							pVert = &newVertices[cVertex];

							dot = ((pVert->n.n[0]*dir[0]
									+ pVert->n.n[1]*dir[1]
									+ pVert->n.n[2]*dir[2]) + 16384) >> 9;

							//dot = max(0, min(63, dot));
							// slightly faster version
							dot = (dot < 0) ? 0 : ((dot > 63) ? 63 : dot);

							pVert->v.cn[0] = light_table_red[dot];
							pVert->v.cn[1] = light_table_green[dot];
							pVert->v.cn[2] = light_table_blue[dot];
						}
					}

					// set up display list
   				pDLP = (Gfx*) pbDispList;

               if (boundsCheck)
					{
      				gSPVertex(pDLP++,
      							 RSP_ADDRESS(boundsVertices),
      							 8, 0);
						gSPCullDisplayList(pDLP++, 0, 7);

						minX = maxX = newVertices[0].v.ob[0];
						minY = maxY = newVertices[0].v.ob[1];
						minZ = maxZ = newVertices[0].v.ob[2];
						for (cVertex=1; cVertex<nVertices; cVertex++)
						{
							pVert = &newVertices[cVertex];

							minX = min(minX, pVert->v.ob[0]);
							maxX = max(maxX, pVert->v.ob[0]);

							minY = min(minY, pVert->v.ob[1]);
							maxY = max(maxY, pVert->v.ob[1]);

							minZ = min(minZ, pVert->v.ob[2]);
							maxZ = max(maxZ, pVert->v.ob[2]);
						}

						memset(boundsVertices, 0, 8*sizeof(Vtx));

						for (cVertex=0; cVertex<8; cVertex++)
						{
							pVert = &boundsVertices[cVertex];

							pVert->v.ob[0] = (cVertex & 1) ? minX : maxX;
							pVert->v.ob[1] = (cVertex & 2) ? minY : maxY;
							pVert->v.ob[2] = (cVertex & 4) ? minZ : maxZ;
						}
					}

					cVertex = 0;
               cFacet = 0;
               for (cVertexLoad=0; cVertexLoad<nVertexLoads; cVertexLoad++)
               {
                  // load a set of vertices
                  if (cVertexLoad)
                  {
                     vertexCount = min(nVertices - cVertex, (VERTEX_CACHE_SIZE/2));

      					gSPVertex(pDLP++,
      								 RSP_ADDRESS(&newVertices[cVertex]),
      								 vertexCount,
      								 (cVertexLoad & 1) ? 0 : (VERTEX_CACHE_SIZE/2));  // load at VERTEX_CACHE_SIZE/2 on evens, 0 on odds

#ifdef PRINT_VTX_LOADS
							TRACE2("Loading verts %d - %d.\r\n",
									 (cVertexLoad & 1) ? 0 : (VERTEX_CACHE_SIZE/2),
									 (cVertexLoad & 1) ? (-1 + vertexCount) : (((VERTEX_CACHE_SIZE/2)-1) + vertexCount));
#endif
                  }
                  else
                  {
                     vertexCount = min(nVertices, VERTEX_CACHE_SIZE);      // load up to 16 on first pass

      					gSPVertex(pDLP++,
      								 RSP_ADDRESS(&newVertices[cVertex]),
      								 vertexCount, 0);             // load at 0

#ifdef PRINT_VTX_LOADS
							TRACE1("Loading verts 0 - %d.\r\n", vertexCount - 1);
#endif
                  }

                  cVertex += vertexCount;

                  // draw polygons
                  do
                  {
                     poly = facets[cFacet++];

							last = POLYSTREAM_ISLASTINRUN(poly);

							if (last)
							{
								gSP1Triangle(pDLP++,
												 POLYSTREAM_GETVA(poly),
												 POLYSTREAM_GETVB(poly),
												 POLYSTREAM_GETVC(poly),
												 0);  // flag
							}
							else
							{
								nextPoly = facets[cFacet++];

								last = POLYSTREAM_ISLASTINRUN(nextPoly);

								gSP2Triangles(pDLP++,
												  POLYSTREAM_GETVA(poly),
												  POLYSTREAM_GETVB(poly),
												  POLYSTREAM_GETVC(poly),
												  0,	// flag
												  POLYSTREAM_GETVA(nextPoly),
												  POLYSTREAM_GETVB(nextPoly),
												  POLYSTREAM_GETVC(nextPoly),
												  0);	// flag
							}
                  } while (!last);
               }
               ASSERT(cFacet == nFacets);
               ASSERT(cVertex == nVertices);

   			 	gSPEndDisplayList(pDLP++);

					ASSERT((pDLP == (Gfx*)(((BYTE*)pbDispList) + dispListSize)));

					sectionOffset += newSectionSize;
					CIndexedSet__SetBlockCount(&isNewSection, 2);
					CIndexedSet__SetNextOffset(&isNewSection, sectionOffset);

   				partOffset += sectionOffset;
   				CIndexedSet__SetBlockCount(&isNewPart, cSection + 1);
   				CIndexedSet__SetNextOffset(&isNewPart, partOffset);

					CIndexedSet__Destruct(&isSection);
					CIndexedSet__Destruct(&isNewSection);
            }

				partsOffset += partOffset;
				CIndexedSet__SetBlockCount(&isNewParts, cPart + 1);
				CIndexedSet__SetNextOffset(&isNewParts, partsOffset);

				CIndexedSet__Destruct(&isPart);
				CUnindexedSet__Destruct(&usPStream);
				CUnindexedSet__Destruct(&usVertices);
			}

			nodeOffset += partsOffset;
			CIndexedSet__SetBlockCount(&isNewNode, 4);
			CIndexedSet__SetNextOffset(&isNewNode, nodeOffset);

			nodesOffset += nodeOffset;
			CIndexedSet__SetBlockCount(&isNewNodes, cNode + 1);
			CIndexedSet__SetNextOffset(&isNewNodes, nodesOffset);

			CIndexedSet__Destruct(&isNode);
			CIndexedSet__Destruct(&isNewNode);
			CIndexedSet__Destruct(&isParts);
			CIndexedSet__Destruct(&isNewParts);
		}

		offset += nodesOffset;
		ASSERT(offset == allocSize);
		CIndexedSet__SetBlockCount(&isNewGeometry, 2);
		CIndexedSet__SetNextOffset(&isNewGeometry, offset);

		CIndexedSet__Destruct(&isNewGeometry);
		CIndexedSet__Destruct(&isNewNodes);
	}
	else
	{
		TRACE("CGeometry: WARNING - could not allocate %x bytes for geometry.\r\n", allocSize);
	}

	CCacheEntry__DeallocAndReplaceData(pThis->m_pceGeometry, pBytes, allocSize);

#ifdef IG_DEBUG
   endTime = osGetTime();
	diffTime = endTime - startTime;
#endif
	TRACE("...Finished geometry decompression in %8.2f msec.\r\n",
			((float)(CYCLES_TO_NSEC(diffTime)))/1000000);

	CIndexedSet__Destruct(&isNodes);
	CIndexedSet__Destruct(&isGeometry);
}

void CGeometry__MorphInc(CGeometry *pThis, int nNode, float Increment)
{
	CIndexedSet				isGeometry;
	CGameGeometryInfo		*pInfo;

	if (!pThis->m_pceGeometry)
		return;

	CIndexedSet__ConstructFromRawData(&isGeometry,
												 CCacheEntry__GetData(pThis->m_pceGeometry),
												 FALSE);

	pInfo = (CGameGeometryInfo*) CIndexedSet__GetBlock(&isGeometry, CART_GEOMETRY_info);
	// only one morph per frame
	if (pInfo->m_MorphFrameNumber != frame_number)
	{
		pInfo->m_cMorph += Increment;

		CGeometry__Morph(pThis, nNode, pInfo->m_cMorph);
	}


	CIndexedSet__Destruct(&isGeometry);
}

void CGeometry__Morph(CGeometry *pThis, int nNode, float Frame)
{
	CIndexedSet				isGeometry,
								isNodes, isNode,
								isParts, isFramePart[2], isDestPart,
								isSection[2], isDestSection;
	CGameGeometryInfo		*pInfo;
	void						*pbNodes, *pbNode,
								*pbParts, *pbPart,
								*pbSection, *pbData;
	int						nParts,
								cSection, nDestSections, nSections[2],
								nDestVertices, nVertices[2], cVert;

	float						fraction;
	int						frame,
								i,
								c1, c2,
								s1, s2;

	Vtx						*destVertices, *pDestVert,
								*vertices[2], *pV1, *pV2;

	if (!pThis->m_pceGeometry)
		return;

	CIndexedSet__ConstructFromRawData(&isGeometry,
												 CCacheEntry__GetData(pThis->m_pceGeometry),
												 FALSE);

	pInfo = (CGameGeometryInfo*) CIndexedSet__GetBlock(&isGeometry, CART_GEOMETRY_info);
	// only one morph per frame
	if (pInfo->m_MorphFrameNumber != frame_number)
	{
		pInfo->m_MorphFrameNumber = frame_number;

		pbNodes = CIndexedSet__GetBlock(&isGeometry, CART_GEOMETRY_isNodes);
		CIndexedSet__ConstructFromRawData(&isNodes, pbNodes, FALSE);

		if (nNode < CIndexedSet__GetBlockCount(&isNodes))
		{
			pbNode = CIndexedSet__GetBlock(&isNodes, nNode);
			CIndexedSet__ConstructFromRawData(&isNode, pbNode, FALSE);

   		pbParts = CIndexedSet__GetBlock(&isNode, CART_NODE_isParts);

   		// all parts in node
			CIndexedSet__ConstructFromRawData(&isParts, pbParts, FALSE);

			nParts = CIndexedSet__GetBlockCount(&isParts);
			if (nParts >= 3)
			{
				pbPart = CIndexedSet__GetBlock(&isParts, even_odd);
				CIndexedSet__ConstructFromRawData(&isDestPart, pbPart, FALSE);
				nDestSections = CIndexedSet__GetBlockCount(&isDestPart);

				while (Frame < 0)
					Frame += nParts - 2;
				while (Frame >= (nParts - 2))
					Frame -= nParts - 2;

				pInfo->m_cMorph = Frame;

				fraction = Frame - (int) Frame;
				c2 = (int) (fraction*256);
				c1 = 256 - c2;

				s2 = (int) (fraction*65536);
				s1 = 65536 - s2;

				for (i=0; i<2; i++)
				{
					frame = ((((int) Frame) + i) % (nParts - 2)) + 2;
					pbPart = CIndexedSet__GetBlock(&isParts, frame);

					CIndexedSet__ConstructFromRawData(&isFramePart[i], pbPart, FALSE);

					nSections[i] = CIndexedSet__GetBlockCount(&isFramePart[i]);
				}


				if ((nSections[0] == nDestSections) && (nSections[1] == nDestSections))
				{
					for (cSection=0; cSection<nDestSections; cSection++)
					{
						pbSection = CIndexedSet__GetBlock(&isDestPart, cSection);
						CIndexedSet__ConstructFromRawData(&isDestSection, pbSection, FALSE);

						pbData = CIndexedSet__GetBlock(&isDestSection, 1);
						nDestVertices = *((DWORD*) pbData);
						destVertices = (Vtx*) *(((DWORD*) pbData) + 1);

						for (i=0; i<2; i++)
						{
							pbSection = CIndexedSet__GetBlock(&isFramePart[i], cSection);
							CIndexedSet__ConstructFromRawData(&isSection[i], pbSection, FALSE);

							pbData = CIndexedSet__GetBlock(&isSection[i], 1);
							nVertices[i] = *((DWORD*) pbData);
							vertices[i] = (Vtx*) *(((DWORD*) pbData) + 1);
						}

						if ((nVertices[0] == nDestVertices) && (nVertices[1] == nDestVertices))
						{
							for (cVert=0; cVert<nDestVertices; cVert++)
							{
								pDestVert = &destVertices[cVert];

								pV1 = &vertices[0][cVert];
								pV2 = &vertices[1][cVert];

	#define BlendShort(s1, s2, v1, v2) ((short) ((((v1)*(int)(s1)) + ((v2)*(int)(s2))) >> 16))
	#define BlendChar(c1, c2, v1, v2) ((signed char) ((((v1)*(int)(c1)) + ((v2)*(int)(c2))) >> 8))

								pDestVert->n.ob[0] = BlendShort(s1, s2, pV1->n.ob[0], pV2->n.ob[0]);
								pDestVert->n.ob[1] = BlendShort(s1, s2, pV1->n.ob[1], pV2->n.ob[1]);
								pDestVert->n.ob[2] = BlendShort(s1, s2, pV1->n.ob[2], pV2->n.ob[2]);

								pDestVert->n.n[0] = BlendChar(c1, c2, pV1->n.n[0], pV2->n.n[0]);
								pDestVert->n.n[1] = BlendChar(c1, c2, pV1->n.n[1], pV2->n.n[1]);
								pDestVert->n.n[2] = BlendChar(c1, c2, pV1->n.n[2], pV2->n.n[2]);
							}
						}

						CIndexedSet__Destruct(&isDestSection);
						CIndexedSet__Destruct(&isSection[0]);
						CIndexedSet__Destruct(&isSection[1]);
					}
				}

				CIndexedSet__Destruct(&isDestPart);
				CIndexedSet__Destruct(&isFramePart[0]);
				CIndexedSet__Destruct(&isFramePart[1]);
			}

			CIndexedSet__Destruct(&isParts);
			CIndexedSet__Destruct(&isNode);
		}

		CIndexedSet__Destruct(&isNodes);
	}

	CIndexedSet__Destruct(&isGeometry);
}


