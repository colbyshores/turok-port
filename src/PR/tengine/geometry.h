// geometry.h

#ifndef _INC_GEOMETRY
#define _INC_GEOMETRY

#include "cart.h"


/////////////////////////////////////////////////////////////////////////////

// globals
extern	DWORD			current_combine_mode ;
extern	DWORD			current_render_mode ;
//DWORD			saved = 0;

extern	BOOL			draw_transparent_geometry ;
extern	BOOL			draw_intersect_geometry ;


/////////////////////////////////////////////////////////////////////////////

typedef struct CGeometry_t
{
	CCacheEntry					*m_pceGeometry,
									*m_pceTextureSetsIndex;
} CGeometry;

// CGeometry operations
/////////////////////////////////////////////////////////////////////////////

#define  CGeometry__GetCacheEntry(pThis) ((pThis)->m_pceGeometry)
void 		CGeometry__RequestBlock(CGeometry *pThis, CCartCache *pCache,
	  						 		  		ROMAddress *rpGeometryAddress, DWORD GeometrySize,
									  		CCacheEntry *pceTextureSetsIndex);
void 		CGeometry__DrawPart(CGeometry *pThis, Gfx **ppDLP, CCartCache *pCache,
									  int nNode, int nPart,
									  BOOL Inter, BOOL Better, int nFrame);
void		CGeometry__DrawShadow(Gfx **ppDLP, Mtx *pmShadow, BOOL Decal, BYTE Alpha);
#define	CGeometry__IsModelLoaded(pThis) ((pThis)->m_pceGeometry != NULL)
void		CGeometry__Morph(CGeometry *pThis, int nNode, float Frame);
void		CGeometry__MorphInc(CGeometry *pThis, int nNode, float Increment);

/////////////////////////////////////////////////////////////////////////////

// special effects globals

#define FXMODE_NONE									0
#define G_CC_FXMODE_NONE									\
		0, 0, 0, COMBINED,									\
		0, 0, 0, COMBINED

#define FXMODE_TOTRANSPARENT						1
#define FXMODE_TOTRANSPARENT_NOISE				2
#define G_CC_FXMODE_TOTRANSPARENT						\
		0, 0, 0, COMBINED,									\
		COMBINED, 0, ENVIRONMENT, 0

#define FXMODE_TOCOLOR								3
#define G_CC_FXMODE_TOCOLOR								\
		ENVIRONMENT, COMBINED, ENV_ALPHA, COMBINED,	\
		0, 0, 0, COMBINED

#define FXMODE_FROMCOLOR_TOTRANSPARENT			4
#define FXMODE_FROMCOLOR_TOTRANSPARENT_NOISE	5
#define G_CC_FXMODE_FROMCOLOR_TOTRANSPARENT			\
		0, 0, 0, ENVIRONMENT,								\
		COMBINED, 0, ENVIRONMENT, 0

#define FXMODE_TOCOLORANDTRANSPARENT			6
#define G_CC_FXMODE_TOCOLORANDTRANSPARENT\
		ENVIRONMENT, COMBINED, ENV_ALPHA, COMBINED,	\
		COMBINED, 0, PRIMITIVE, 0							\

#define FXMODE_GLARE									7
#define G_CC_FXMODE_GLARE								\
		ENVIRONMENT, COMBINED, SHADE, COMBINED,	\
		0, 0, 0, COMBINED

extern DWORD			fx_mode;
extern float			fx_color[5];

// new render modes

#define	RM_AA_ZB_XLU_SURF_ZUPD(clk)					\
	AA_EN | Z_CMP | Z_UPD | IM_RD | CVG_DST_WRAP | CLR_ON_CVG |	\
	FORCE_BL | ZMODE_OPA |					\
	GBL_c##clk(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_1MA)

#define	RM_ROB_ZB_PCL_SURF_BLEND(clk)												\
	Z_CMP | IM_RD | CVG_DST_SAVE | FORCE_BL | ZMODE_XLU |	 G_AC_DITHER | \
	GBL_c##clk(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_1MA)



// new combine modes

#define G_CC_16RGBA_4A						\
	TEXEL0, 0, ENVIRONMENT, 0,				\
	TEXEL1, 0, ENVIRONMENT, 0
	//(a - b)*c + d

#define G_CC_MULTIBIT_ALPHA								\
	ENVIRONMENT, PRIMITIVE, TEXEL1, PRIMITIVE,		\
	TEXEL0, 0, ENVIRONMENT, 0

#define	G_CC_ROB_PSEUDOCOLOR_SHADEALPHA				\
		ENVIRONMENT, SHADE, TEXEL0, SHADE,	\
		SHADE, 0, TEXEL0, 0



#define G_CC_ROB_SELFILLUM_PSEUDO						\
		PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT,	\
		TEXEL0, 0, PRIMITIVE, 0

#define G_CC_ROB_SELFILLUM_PSEUDO_SHADEALPHA			\
		PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT,	\
		SHADE, 0, TEXEL0, 0

#define G_CC_ROB_LIGHTED2		\
		COMBINED, 0, SHADE, 0,	\
		0, 0, 0, COMBINED

#define G_CC_ROB_SELFILLUM_TEXTURE	\
		0, 0, 0, TEXEL0,				\
		TEXEL0, 0, PRIMITIVE, 0

#define G_CC_ROB_LIGHTED_TEXTURE	\
		TEXEL0, 0, SHADE, 0,		\
		TEXEL0, 0, PRIMITIVE, 0

#define G_CC_ROB_SELFILLUM_SOLID	\
			0, 0, 0, PRIMITIVE,		\
			0, 0, 0, PRIMITIVE

#define G_CC_ROB_LIGHTED_SOLID	\
			PRIMITIVE, 0, SHADE, 0,		\
			0, 0, 0, PRIMITIVE

#define G_CC_ROB_QUACK_PRIM	\
			0, 0, 0, PRIMITIVE,	\
			0, 0, 0, 1

#define G_CC_ROB_QUACK_PRIM		\
			0, 0, 0, PRIMITIVE,		\
			0, 0, 0, 1

#define G_CC_ROB_QUACK_ENV			\
			0, 0, 0, ENVIRONMENT,	\
			0, 0, 0, 1


// Valid combine modes
#define	G_CC_ROB_SELFILLUM_PSEUDO_SHADEALPHA__G_CC_PASS2						0
#define	G_CC_ROB_SELFILLUM_PSEUDO_SHADEALPHA__G_CC_FXMODE_TOTRANSPARENT	1
#define	G_CC_ROB_SELFILLUM_PSEUDO__G_CC_PASS2										2
#define	G_CC_ROB_SELFILLUM_PSEUDO_SHADEALPHA__G_CC_ROB_LIGHTED2				3
#define	G_CC_ROB_SELFILLUM_PSEUDO__G_CC_ROB_LIGHTED2								4
#define	G_CC_TRILERP__G_CC_DECALRGB2													5
#define	G_CC_ROB_SELFILLUM_TEXTURE__G_CC_PASS2										6
#define	G_CC_ROB_SELFILLUM_TEXTURE__G_CC_FXMODE_TOTRANSPARENT					7
#define	G_CC_ROB_SELFILLUM_TEXTURE__G_CC_FXMODE_TOCOLOR							8
#define	G_CC_ROB_SELFILLUM_TEXTURE__G_CC_FXMODE_GLARE							9
#define	G_CC_ROB_SELFILLUM_TEXTURE__G_CC_FXMODE_FROMCOLOR_TOTRANSPARENT	10
#define	G_CC_ROB_SELFILLUM_TEXTURE__G_CC_FXMODE_TOCOLORANDTRANSPARENT		11
#define	G_CC_TRILERP__G_CC_MODULATERGB2												12
#define	G_CC_ROB_LIGHTED_TEXTURE__G_CC_FXMODE_NONE								13
#define	G_CC_ROB_LIGHTED_TEXTURE__G_CC_FXMODE_TOTRANSPARENT					14
#define	G_CC_ROB_LIGHTED_TEXTURE__G_CC_FXMODE_TOCOLOR							15
#define	G_CC_ROB_LIGHTED_TEXTURE__G_CC_FXMODE_FROMCOLOR_TOTRANSPARENT		16
#define	G_CC_ROB_LIGHTED_TEXTURE__G_CC_FXMODE_TOCOLORANDTRANSPARENT			17
#define	G_CC_ROB_SELFILLUM_SOLID__G_CC_PASS2										18
#define	G_CC_ROB_LIGHTED_SOLID__G_CC_FXMODE_NONE									19
#define	G_CC_ROB_LIGHTED_SOLID__G_CC_FXMODE_TOTRANSPARENT						20
#define	G_CC_ROB_LIGHTED_SOLID__G_CC_FXMODE_TOCOLOR								21
#define	G_CC_ROB_SELFILLUM_SOLID__G_CC_FXMODE_GLARE								22
#define	G_CC_ROB_LIGHTED_SOLID__G_CC_FXMODE_FROMCOLOR_TOTRANSPARENT			23

// from ONSCREEN
#define	G_CC_ROB_DECALRGBA_PRIMALPHA__G_CC_ROB_DECALRGBA_PRIMALPHA			24
#define	G_CC_PRIMITIVE__G_CC_PRIMITIVE												25
#define	G_CC_MULTIBIT_ALPHA__G_CC_PASS2												26
#define	G_CC_16RGBA_4A__G_CC_PASS2														27
// FX
#define	G_CC_SHADE__G_CC_SHADE															28
// PARTICLE
#define	G_CC_ROB_PSEUDOCOLOR_PRIMALPHA__G_CC_ROB_PSEUDOCOLOR_PRIMALPHA		29
#define	G_CC_ROB_QUACK_PRIM__G_CC_ROB_QUACK_PRIM									30
#define	G_CC_ROB_QUACK_ENV__G_CC_ROB_QUACK_ENV										31
// SCENE
#define	G_CC_ROB_PSEUDOCOLOR_SHADEALPHA__G_CC_ROB_PSEUDOCOLOR_SHADEALPHA	32
#define	G_CC_MODULATERGB__G_CC_MODULATERGB											33
// ROMSTRUC
#define	G_CC_ROB_BLACK_PRIMALPHA__G_CC_PASS2										34



// valid render modes
#define	G_RM_PASS__G_RM_AA_ZB_XLU_SURF2								0
#define	G_RM_FOG_SHADE_A__G_RM_AA_ZB_XLU_SURF2						1
#define	G_RM_PASS__G_RM_ZB_XLU_SURF2									2
#define	G_RM_PASS__G_RM_ZB_CLD_SURF2									3
#define	G_RM_FOG_SHADE_A__G_RM_AA_ZB_XLU_DECAL2					4
#define	G_RM_FOG_SHADE_A__RM_AA_ZB_XLU_SURF_ZUPD					5
#define	G_RM_FOG_SHADE_A__G_RM_AA_ZB_TEX_EDGE2						6
#define	G_RM_FOG_SHADE_A__G_RM_ZB_OPA_DECAL2						7
#define	G_RM_FOG_SHADE_A__G_RM_AA_ZB_OPA_INTER2					8
#define	G_RM_FOG_SHADE_A__G_RM_RA_ZB_OPA_INTER2					9
#define	G_RM_FOG_SHADE_A__G_RM_AA_ZB_OPA_SURF2						10
#define	G_RM_FOG_SHADE_A__G_RM_RA_ZB_OPA_SURF2						11
// SCENE
#define	G_RM_AA_ZB_XLU_INTER__G_RM_AA_ZB_XLU_INTER2				12
#define	G_RM_ZB_CLD_SURF__G_RM_ZB_CLD_SURF2							13
// FX
#define	G_RM_AA_ZB_XLU_SURF__G_RM_AA_ZB_XLU_SURF2					14
// PARTICLE
#define	G_RM_ZB_XLU_DECAL__G_RM_ZB_XLU_DECAL2						15
#define	RM_ROB_ZB_PCL_SURF_BLEND1__RM_ROB_ZB_PCL_SURF_BLEND2	16
#define	G_RM_AA_ZB_TEX_EDGE__G_RM_AA_ZB_TEX_EDGE2					17
#define	G_RM_AA_TEX_EDGE__G_RM_AA_TEX_EDGE2							18
#define	G_RM_CLD_SURF__G_RM_CLD_SURF2									19
// TENGINE
#define	G_RM_NOOP__G_RM_NOOP2											20
#define	G_RM_XLU_SURF__G_RM_XLU_SURF2									21
#define	G_RM_VISCVG__G_RM_VISCVG2										22
// ROMSTRUC
#define	G_RM_FOG_SHADE_A__G_RM_ZB_CLD_SURF2							23
#define	G_RM_FOG_SHADE_A__G_RM_ZB_XLU_DECAL2						24
// ONSCRN
#define	G_RM_PASS__G_RM_XLU_SURF2										25
#define	G_RM_PASS__G_RM_CLD_SURF2										26

void CGeometry__ResetDrawModes(void) ;
void CGeometry__SetCombineMode(Gfx **ppDLP, DWORD mode) ;
BOOL CGeometry__SetRenderMode(Gfx **ppDLP, DWORD mode) ;

/////////////////////////////////////////////////////////////////////////////
#endif // _INC_GEOMETRY
