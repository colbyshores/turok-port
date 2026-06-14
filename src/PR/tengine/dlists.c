// dlists.c

#include "cppu64.h"
#include "tengine.h"
#include "dlists.h"

// Remember, viewport structures have 2 bits of fraction in them
/*
Vp vp =
{
  SCREEN_WD*2, SCREEN_HT*2, G_MAXZ/2, 0,	// scale
  SCREEN_WD*2, SCREEN_HT*2, G_MAXZ/2, 0,	// translate
};
*/

// this version gives better precision
Vp vp =
{
  SCREEN_WD*2, SCREEN_HT*2, G_MAXZ, 0,		// scale
  SCREEN_WD*2, SCREEN_HT*2, 0,		0,		// translate
};


Lights1 thelight = gdSPDefLights1(0x38,0x38,0x38,	// Ambient
				  							 0xff, 0xff, 0xff, 	// Color
				  							 -42, 30, -36);   	// Normal


// initialize the RSP state:
Gfx rspinit_dl[] =
{
  gsSPViewport(&vp),
  gsSPClearGeometryMode(-1),
  gsSPSetGeometryMode(G_ZBUFFER),
  gsSPSetLights1(thelight),
  gsSPEndDisplayList(),
};

// initialize the RDP state:
Gfx rdpinit_dl[] =
{
  	gsDPSetCycleType(G_CYC_1CYCLE),						// one cycle mode (others: G_CYC_FILL)
  	gsDPPipelineMode(G_PM_NPRIMITIVE), // (other: G_PM_1PRIMITIVE)
  	gsDPSetScissor(G_SC_NON_INTERLACE, 0, 0, SCREEN_WD, SCREEN_HT),

  	//gsDPSetScissor(G_SC_NON_INTERLACE, -SCREEN_WD, -SCREEN_HT, 2*SCREEN_WD, 2*SCREEN_HT),
  	gsDPSetTextureLOD(G_TL_TILE),
  	gsDPSetTextureLUT(G_TT_NONE),
  	gsDPSetTextureDetail(G_TD_CLAMP),
  	gsDPSetTexturePersp(G_TP_PERSP),
  	gsDPSetTextureFilter(G_TF_BILERP),
  	gsDPSetTextureConvert(G_TC_FILT),
  	gsDPSetCombineKey(G_CK_NONE),
  	gsDPSetAlphaCompare(G_AC_NONE),
  	gsDPSetDepthImage(zbuffer),

  	gsDPPipeSync(),
  	gsSPEndDisplayList(),
};

Gfx mip_map_dl[] =
{
	// load tile
	gsDPSetTile(0, 2, 0, 0, G_TX_LOADTILE, 0, 0, 0, 0, 0, 0, 0),
	gsDPLoadBlock(G_TX_LOADTILE, 0, 0, 1372, 0),

	// 6 mip-map levels
	gsDPSetTile(0, 2, 8, 0, 0, 0, 0, 5, 0, 0, 5, 0),
	gsDPSetTileSize(0,  0, 0, 31 << G_TEXTURE_IMAGE_FRAC, 31 << G_TEXTURE_IMAGE_FRAC),
	gsDPSetTile(0, 2, 4, 256, 1, 0, 0, 4, 1, 0, 4, 1),
	gsDPSetTileSize(1,  0, 0, 15 << G_TEXTURE_IMAGE_FRAC, 15 << G_TEXTURE_IMAGE_FRAC),
	gsDPSetTile(0, 2, 2, 320, 2, 0, 0, 3, 2, 0, 3, 2),
	gsDPSetTileSize(2,  0, 0, 7 << G_TEXTURE_IMAGE_FRAC, 7 << G_TEXTURE_IMAGE_FRAC),
	gsDPSetTile(0, 2, 1, 336, 3, 0, 0, 2, 3, 0, 2, 3),
	gsDPSetTileSize(3,  0, 0, 3 << G_TEXTURE_IMAGE_FRAC, 3 << G_TEXTURE_IMAGE_FRAC),
	gsDPSetTile(0, 2, 1, 340, 4, 0, 0, 1, 4, 0, 1, 4),
	gsDPSetTileSize(4,  0, 0, 1 << G_TEXTURE_IMAGE_FRAC, 1 << G_TEXTURE_IMAGE_FRAC),
	gsDPSetTile(0, 2, 1, 342, 5, 0, 0, 0, 5, 0, 0, 5),
	gsDPSetTileSize(5,  0, 0, 0 << G_TEXTURE_IMAGE_FRAC, 0 << G_TEXTURE_IMAGE_FRAC),

	gsSPEndDisplayList(),
};

Gfx mip_map_mirror_dl[] =
{
	// load tile
	gsDPSetTile(0, 2, 0, 0, G_TX_LOADTILE, 0, 0, 0, 0, 0, 0, 0),
	gsDPLoadBlock(G_TX_LOADTILE, 0, 0, 1372, 0),

	// 6 mip-map levels
	gsDPSetTile(0, 2, 8, 0, 0, 0, G_TX_MIRROR, 5, 0, G_TX_MIRROR, 5, 0),
	gsDPSetTileSize(0,  0, 0, 31 << G_TEXTURE_IMAGE_FRAC, 31 << G_TEXTURE_IMAGE_FRAC),
	gsDPSetTile(0, 2, 4, 256, 1, 0, G_TX_MIRROR, 4, 1, G_TX_MIRROR, 4, 1),
	gsDPSetTileSize(1,  0, 0, 15 << G_TEXTURE_IMAGE_FRAC, 15 << G_TEXTURE_IMAGE_FRAC),
	gsDPSetTile(0, 2, 2, 320, 2, 0, G_TX_MIRROR, 3, 2, G_TX_MIRROR, 3, 2),
	gsDPSetTileSize(2,  0, 0, 7 << G_TEXTURE_IMAGE_FRAC, 7 << G_TEXTURE_IMAGE_FRAC),
	gsDPSetTile(0, 2, 1, 336, 3, 0, G_TX_MIRROR, 2, 3, G_TX_MIRROR, 2, 3),
	gsDPSetTileSize(3,  0, 0, 3 << G_TEXTURE_IMAGE_FRAC, 3 << G_TEXTURE_IMAGE_FRAC),
	gsDPSetTile(0, 2, 1, 340, 4, 0, G_TX_MIRROR, 1, 4, G_TX_MIRROR, 1, 4),
	gsDPSetTileSize(4,  0, 0, 1 << G_TEXTURE_IMAGE_FRAC, 1 << G_TEXTURE_IMAGE_FRAC),
	gsDPSetTile(0, 2, 1, 342, 5, 0, G_TX_MIRROR, 0, 5, G_TX_MIRROR, 0, 5),
	gsDPSetTileSize(5,  0, 0, 0 << G_TEXTURE_IMAGE_FRAC, 0 << G_TEXTURE_IMAGE_FRAC),

	gsSPEndDisplayList(),
};

Gfx mip_map_detail2x2_dl[] =
{
	// load tile
	gsDPSetTile(0, 2, 0, 0, G_TX_LOADTILE, 0, 0, 0, 0, 0, 0, 0),
	gsDPLoadBlock(G_TX_LOADTILE, 0, 0, 1372, 0),

	// detail tile uses first mip-map level tiled 2x2

	gsDPSetTile(0, 2, 8, 0, 0, 0, 0, 5, 15, 0, 5, 15),
	gsDPSetTileSize(0,  0, 0, 31 << G_TEXTURE_IMAGE_FRAC, 31 << G_TEXTURE_IMAGE_FRAC),

	// 6 mip-map levels
	gsDPSetTile(0, 2, 8, 0, 1, 0, 0, 5, 0, 0, 5, 0),
	gsDPSetTileSize(1,  0, 0, 31 << G_TEXTURE_IMAGE_FRAC, 31 << G_TEXTURE_IMAGE_FRAC),
	gsDPSetTile(0, 2, 4, 256, 2, 0, 0, 4, 1, 0, 4, 1),
	gsDPSetTileSize(2,  0, 0, 15 << G_TEXTURE_IMAGE_FRAC, 15 << G_TEXTURE_IMAGE_FRAC),
	gsDPSetTile(0, 2, 2, 320, 3, 0, 0, 3, 2, 0, 3, 2),
	gsDPSetTileSize(3,  0, 0, 7 << G_TEXTURE_IMAGE_FRAC, 7 << G_TEXTURE_IMAGE_FRAC),
	gsDPSetTile(0, 2, 1, 336, 4, 0, 0, 2, 3, 0, 2, 3),
	gsDPSetTileSize(4,  0, 0, 3 << G_TEXTURE_IMAGE_FRAC, 3 << G_TEXTURE_IMAGE_FRAC),
	gsDPSetTile(0, 2, 1, 340, 5, 0, 0, 1, 4, 0, 1, 4),
	gsDPSetTileSize(5,  0, 0, 1 << G_TEXTURE_IMAGE_FRAC, 1 << G_TEXTURE_IMAGE_FRAC),
	gsDPSetTile(0, 2, 1, 342, 6, 0, 0, 0, 5, 0, 0, 5),
	gsDPSetTileSize(6,  0, 0, 0 << G_TEXTURE_IMAGE_FRAC, 0 << G_TEXTURE_IMAGE_FRAC),

	gsSPEndDisplayList(),
};

Gfx mip_map_detail4x4_dl[] =
{
	// load tile
	gsDPSetTile(0, 2, 0, 0, G_TX_LOADTILE, 0, 0, 0, 0, 0, 0, 0),
	gsDPLoadBlock(G_TX_LOADTILE, 0, 0, 1372, 0),

	// detail tile uses first mip-map level tiled 4x4

	gsDPSetTile(0, 2, 8, 0, 0, 0, 0, 5, 14, 0, 5, 14),
	gsDPSetTileSize(0,  0, 0, 31 << G_TEXTURE_IMAGE_FRAC, 31 << G_TEXTURE_IMAGE_FRAC),

	// 6 mip-map levels
	gsDPSetTile(0, 2, 8, 0, 1, 0, 0, 5, 0, 0, 5, 0),
	gsDPSetTileSize(1,  0, 0, 31 << G_TEXTURE_IMAGE_FRAC, 31 << G_TEXTURE_IMAGE_FRAC),
	gsDPSetTile(0, 2, 4, 256, 2, 0, 0, 4, 1, 0, 4, 1),
	gsDPSetTileSize(2,  0, 0, 15 << G_TEXTURE_IMAGE_FRAC, 15 << G_TEXTURE_IMAGE_FRAC),
	gsDPSetTile(0, 2, 2, 320, 3, 0, 0, 3, 2, 0, 3, 2),
	gsDPSetTileSize(3,  0, 0, 7 << G_TEXTURE_IMAGE_FRAC, 7 << G_TEXTURE_IMAGE_FRAC),
	gsDPSetTile(0, 2, 1, 336, 4, 0, 0, 2, 3, 0, 2, 3),
	gsDPSetTileSize(4,  0, 0, 3 << G_TEXTURE_IMAGE_FRAC, 3 << G_TEXTURE_IMAGE_FRAC),
	gsDPSetTile(0, 2, 1, 340, 5, 0, 0, 1, 4, 0, 1, 4),
	gsDPSetTileSize(5,  0, 0, 1 << G_TEXTURE_IMAGE_FRAC, 1 << G_TEXTURE_IMAGE_FRAC),
	gsDPSetTile(0, 2, 1, 342, 6, 0, 0, 0, 5, 0, 0, 5),
	gsDPSetTileSize(6,  0, 0, 0 << G_TEXTURE_IMAGE_FRAC, 0 << G_TEXTURE_IMAGE_FRAC),

	gsSPEndDisplayList(),
};

Gfx shadow_dl[] =
{
	gsDPSetTextureLUT(G_TT_NONE),
	gsSPTexture(0x8000, 0x8000,
					0, 0, G_ON),
  	gsSPClearGeometryMode(G_LIGHTING | G_CULL_BACK | G_TEXTURE_GEN),
  	gsSPSetGeometryMode(G_SHADE | G_SHADING_SMOOTH | G_FOG),
	
	gsDPSetCombineMode(G_CC_ROB_BLACK_PRIMALPHA, G_CC_PASS2),

	gsDPSetRenderMode(G_RM_FOG_SHADE_A, G_RM_ZB_CLD_SURF2),

	// helps clipping problems
	gsSPClipRatio(FRUSTRATIO_1),

	gsSPVertex(RSP_ADDRESS(shadow_vtxs), 4, 0),

	gsSP2Triangles(0, 1, 2, 0,
						2, 3, 0, 0),

	// performance tuned for animated instances
	gsSPClipRatio(FRUSTRATIO_5),

	gsSPEndDisplayList(),
};

Gfx decal_shadow_dl[] =
{
	gsDPSetTextureLUT(G_TT_NONE),
	gsSPTextureL(0x8000, 0x8000,
					 0,
					 255,	// z compare range
					 0, G_ON),

  	gsSPClearGeometryMode(G_LIGHTING | G_CULL_BACK | G_TEXTURE_GEN),
  	gsSPSetGeometryMode(G_SHADE | G_SHADING_SMOOTH | G_FOG),
	
	gsDPSetCombineMode(G_CC_ROB_BLACK_PRIMALPHA, G_CC_PASS2),

	gsDPSetRenderMode(G_RM_FOG_SHADE_A, G_RM_ZB_XLU_DECAL2),

	// helps clipping problems
	gsSPClipRatio(FRUSTRATIO_1),

	gsSPVertex(RSP_ADDRESS(shadow_vtxs), 4, 0),

	gsSP2Triangles(0, 1, 2, 0,
						2, 3, 0, 0),

	// performance tuned for animated instances
	gsSPClipRatio(FRUSTRATIO_5),

	gsSPEndDisplayList(),
};

