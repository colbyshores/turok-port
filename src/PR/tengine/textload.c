// textload.cpp
//

#include "stdafx.h"

#include "textload.h"
#include "romstruc.h"
#include "cartdir.h"
#include "dlists.h"

#include "debug.h"

// global variables
ROMAddress	*rp_current_textureset = NULL;

// CTextureLoader implementation
/////////////////////////////////////////////////////////////////////////////

// callbacks
void	CTextureLoader__Decompress(CTextureLoader *pThis, CCacheEntry **ppceTarget);

/////////////////////////////////////////////////////////////////////////////

void CTextureLoader__InvalidateTextureCache()
{
	rp_current_textureset = NULL;
}

void CTextureLoader__ConstructFromAddressSize(CTextureLoader *pThis, ROMAddress *rpTextureSet, DWORD TextureSetSize)
{
	pThis->m_pceTextureSet = NULL;
	pThis->m_rpTextureSet = rpTextureSet;
	pThis->m_TextureSetSize = TextureSetSize;
}

void CTextureLoader__ConstructFromIndex(CTextureLoader *pThis,
													 CIndexedSet *pisTextureSetsIndex,
													 ROMAddress *rpTextureSets,
													 int nIndex)
{
	ASSERT(pisTextureSetsIndex);
	ASSERT(nIndex < CIndexedSet__GetBlockCount(pisTextureSetsIndex));

	pThis->m_pceTextureSet = NULL;

	pThis->m_rpTextureSet = CIndexedSet__GetROMAddressAndSize(pisTextureSetsIndex,
																				 rpTextureSets,
																				 nIndex,
																				 &pThis->m_TextureSetSize);
}

BOOL CTextureLoader__RequestTextureSet(CTextureLoader *pThis, CCartCache *pCache, char *szDescription)
{
	ASSERT(pThis->m_rpTextureSet);
	ASSERT(pThis->m_TextureSetSize);

	pThis->m_pceTextureSet = NULL;

	pThis->m_pceTextureSet = NULL;
   CCartCache__RequestBlock(pCache,
									 pThis, (pfnCACHENOTIFY) CTextureLoader__Decompress, NULL,
									 &pThis->m_pceTextureSet,
                            pThis->m_rpTextureSet, pThis->m_TextureSetSize,
                            szDescription);
	
	return (pThis->m_pceTextureSet != NULL);
}

void CTextureLoader__Decompress(CTextureLoader *pThis, CCacheEntry **ppceTarget)
{
	CIndexedSet			isTextureSet,
							isPalettes;
	void					*pbPalettes, *pbPalette;
	CROMTextureFormat	*pFormat;
	int					cPalette, nPalettes,
							cColor, nColors;
	DWORD					paletteSize;
	WORD					*colors, *pColor;
	unsigned int		r, g, b, a;

	ASSERT(pThis->m_pceTextureSet == *ppceTarget);

	if (!CCacheEntry__Decompress(pThis->m_pceTextureSet))
		return;

	if (CACHE_BLOCK_GETVERSION(CCacheEntry__GetRequestedAddress(pThis->m_pceTextureSet)))
	{
		// swap red and blue colors
		CIndexedSet__ConstructFromRawData(&isTextureSet,
													 CCacheEntry__GetData(pThis->m_pceTextureSet),
													 FALSE);

		pFormat = (CROMTextureFormat*) CIndexedSet__GetBlock(&isTextureSet, CART_TEXTURESET_format);
		
		switch (pFormat->m_Format)
		{
			// indexed 8 bit RGBA
			case TEXTURE_FORMAT_CI8_RGBA:
			// indexed 4 bit RGBA
			case TEXTURE_FORMAT_CI4_RGBA:
				pbPalettes = CIndexedSet__GetBlock(&isTextureSet, CART_TEXTURESET_isPalettes);
				CIndexedSet__ConstructFromRawData(&isPalettes, pbPalettes, FALSE);
				nPalettes = CIndexedSet__GetBlockCount(&isPalettes);

				for (cPalette=0; cPalette<nPalettes; cPalette++)
				{
					pbPalette = CIndexedSet__GetBlockAndSize(&isPalettes, cPalette, &paletteSize);
					colors = (WORD*) pbPalette;
					nColors = paletteSize/sizeof(WORD);

					for (cColor=0; cColor<nColors; cColor++)
					{
						pColor = &colors[cColor];

						r = ((*pColor) >> 8) & 0xf8;
						g = ((*pColor) >> 3) & 0xf8;
						b = ((*pColor) << 2) & 0xf8;
						a = (*pColor) & 0x01;
						
						*pColor = (WORD) ((g << 8)
											 | (r << 3)
											 | (b >> 2)
											 |  a);
					}
				}

				CIndexedSet__Destruct(&isPalettes);
				
				break;
		}

		CIndexedSet__Destruct(&isTextureSet);
	}
}

void CTextureLoader__GetTextureInfo(CTextureLoader *pThis, CTextureInfo *pInfo)
{
	CIndexedSet		isTextureSet,
						isBitmaps,
						isPalettes;
	void				*pbBitmaps,
						*pbPalettes;

	ASSERT(pThis->m_pceTextureSet);
	ASSERT(pInfo);

	CIndexedSet__ConstructFromRawData(&isTextureSet,
												 CCacheEntry__GetData(pThis->m_pceTextureSet),
												 FALSE);

	if (CIndexedSet__GetBlockCount(&isTextureSet))
	{
		pInfo->m_pFormat = (CROMTextureFormat*) CIndexedSet__GetBlock(&isTextureSet, CART_TEXTURESET_format);

		pbBitmaps = CIndexedSet__GetBlock(&isTextureSet, CART_TEXTURESET_isBitmaps);
		CIndexedSet__ConstructFromRawData(&isBitmaps, pbBitmaps, FALSE);
		pInfo->m_nBitmaps = CIndexedSet__GetBlockCount(&isBitmaps);

		pbPalettes = CIndexedSet__GetBlock(&isTextureSet, CART_TEXTURESET_isPalettes);
		CIndexedSet__ConstructFromRawData(&isPalettes, pbPalettes, FALSE);
		pInfo->m_nPalettes = CIndexedSet__GetBlockCount(&isPalettes);

		CIndexedSet__Destruct(&isBitmaps);
		CIndexedSet__Destruct(&isPalettes);
	}
	else
	{
		pInfo->m_pFormat = NULL;
		pInfo->m_nBitmaps = 0;
		pInfo->m_nPalettes = 0;
	}

	CIndexedSet__Destruct(&isTextureSet);
}

void CTextureLoader__LoadTexture(CTextureLoader *pThis, Gfx **ppDLP,
											int nBitmap, int nPalette,
											int MultU, int MultV,
											unsigned int TileFlagsU, unsigned int TileFlagsV,
											unsigned int Shift)
{
	CIndexedSet				isTextureSet,
								isBitmaps,
								isPalettes;
	CROMTextureFormat		*pFormat;
	void						*pbBitmaps, *pBitmap,
								*pbPalettes, *pPalette;
	int						nPalettes, detail;
	Gfx						*pDL;
	unsigned int			tileU, tileV;
	BOOL						loadBitmap, loadPalette;
	static int				currentBitmap;
	static int				currentPalette;
	static unsigned int	currentTileFlagsU;
	static unsigned int	currentTileFlagsV;

	// texture hasn't been requested or isn't here yet
	ASSERT(pThis->m_pceTextureSet);

	CIndexedSet__ConstructFromRawData(&isTextureSet,
												 CCacheEntry__GetData(pThis->m_pceTextureSet),
												 FALSE);

	pFormat = (CROMTextureFormat*) CIndexedSet__GetBlock(&isTextureSet, CART_TEXTURESET_format);

	pbBitmaps = CIndexedSet__GetBlock(&isTextureSet, CART_TEXTURESET_isBitmaps);
	CIndexedSet__ConstructFromRawData(&isBitmaps, pbBitmaps, FALSE);
	ASSERT(nBitmap < CIndexedSet__GetBlockCount(&isBitmaps));
	pBitmap = CIndexedSet__GetBlock(&isBitmaps, nBitmap);

	pbPalettes = CIndexedSet__GetBlock(&isTextureSet, CART_TEXTURESET_isPalettes);
	CIndexedSet__ConstructFromRawData(&isPalettes, pbPalettes, FALSE);
	nPalettes = CIndexedSet__GetBlockCount(&isPalettes);
	if (nPalettes)
	{
		ASSERT(nPalette < nPalettes);
		pPalette = CIndexedSet__GetBlock(&isPalettes, nPalette);
	}

	loadBitmap  = (	(rp_current_textureset != pThis->m_rpTextureSet)
						|| (currentBitmap != nBitmap)
						|| (currentTileFlagsU != TileFlagsU)
						|| (currentTileFlagsV != TileFlagsV) );

	loadPalette = (	(rp_current_textureset != pThis->m_rpTextureSet)
						|| (currentPalette != nPalette) );

	if (loadBitmap || loadPalette)
	{
		// update static variables that keep track of current texture in cache
		rp_current_textureset = pThis->m_rpTextureSet;
		currentBitmap = nBitmap;
		currentPalette = nPalette;
		currentTileFlagsU = TileFlagsU;
		currentTileFlagsV = TileFlagsV;

		tileU = (TileFlagsU & G_TX_CLAMP) ? G_TX_NOMASK : pFormat->m_WidthShift;
		tileV = (TileFlagsV & G_TX_CLAMP) ? G_TX_NOMASK : pFormat->m_HeightShift;
		
		ASSERT(((1 << pFormat->m_WidthShift) * (1 << pFormat->m_HeightShift)) <= (64*64));
		ASSERT(pFormat->m_WidthShift <= 8);
		ASSERT(pFormat->m_HeightShift <= 8);
		
		switch (pFormat->m_Format)
		{
			// indexed 8 bit RGBA
			case TEXTURE_FORMAT_CI8_RGBA:
				if (loadBitmap)
				{
					// load texture
					gDPLoadTextureBlock(((*ppDLP)++),
									  		  RDP_ADDRESS(pBitmap), G_IM_FMT_CI, G_IM_SIZ_8b,
									  		  (1 << pFormat->m_WidthShift), (1 << pFormat->m_HeightShift), 0,
									  		  TileFlagsU, TileFlagsV,
									  		  tileU, tileV,
									  		  Shift, Shift);

					// enables lookup textures in 16 bit RGBA format
 					gDPSetTextureLUT((*ppDLP)++, G_TT_RGBA16);
				}

				if (loadPalette)
				{
					// load palette
					gDPLoadTLUT_pal256(((*ppDLP)++), RDP_ADDRESS(pPalette));
				}
				break;

			// indexed 4 bit RGBA
			case TEXTURE_FORMAT_CI4_RGBA:
				if (loadBitmap)
				{
					// load texture
					gDPLoadTextureBlock_4b(((*ppDLP)++),
									  			  RDP_ADDRESS(pBitmap), G_IM_FMT_CI,// G_IM_SIZ_4b,
									  			  (1 << pFormat->m_WidthShift), (1 << pFormat->m_HeightShift), 0,
									  			  TileFlagsU, TileFlagsV,
									  			  tileU, tileV,
									  			  Shift, Shift);

					// enables lookup textures in 16 bit RGBA format
 					gDPSetTextureLUT((*ppDLP)++, G_TT_RGBA16);
				}

				if (loadPalette)
				{
					// load palette
					gDPLoadTLUT_pal16(((*ppDLP)++), 0, RDP_ADDRESS(pPalette));
				}
				break;

			// indexed 4 bit IA
			case TEXTURE_FORMAT_CI4_IA:
				if (loadBitmap)
				{
					// load texture
					gDPLoadTextureBlock_4b(((*ppDLP)++),
									  			  RDP_ADDRESS(pBitmap), G_IM_FMT_CI,
									  			  (1 << pFormat->m_WidthShift), (1 << pFormat->m_HeightShift), 0,
									  			  TileFlagsU, TileFlagsV,
									  			  tileU, tileV,
									  			  Shift, Shift);

					// enables lookup textures in 16 bit IA format
 					gDPSetTextureLUT((*ppDLP)++, G_TT_IA16);
				}

				if (loadPalette)
				{
					// load palette
					gDPLoadTLUT_pal16(((*ppDLP)++), 0, RDP_ADDRESS(pPalette));
				}
				break;

			// true color RGBA
			case TEXTURE_FORMAT_32_RGBA:
				if (loadBitmap)
				{
					// load texture
					gDPLoadTextureBlock(((*ppDLP)++),
									  		  RDP_ADDRESS(pBitmap), G_IM_FMT_RGBA, G_IM_SIZ_32b,
									  		  (1 << pFormat->m_WidthShift), (1 << pFormat->m_HeightShift), 0,
									  		  TileFlagsU, TileFlagsV,
									  		  tileU, tileV,
									  		  Shift, Shift);

					// disable lookup table
					gDPSetTextureLUT((*ppDLP)++, G_TT_NONE);
				}
				break;

			case TEXTURE_FORMAT_MM16RGBA:
				if (loadBitmap)
				{
					gDPPipeSync((*ppDLP)++);
					gDPSetTextureImage((*ppDLP)++, 0, 2, 1, RDP_ADDRESS(pBitmap));
					
					switch (pFormat->m_EffectMode)
					{
						case TEXTURE_EFFECT_DETAIL_X2:
							detail = G_TD_DETAIL;
							pDL = mip_map_detail2x2_dl;
							break;

						case TEXTURE_EFFECT_DETAIL_X4:
							detail = G_TD_DETAIL;
							pDL = mip_map_detail4x4_dl;
							break;

						case TEXTURE_EFFECT_SHARPEN:
							detail = G_TD_SHARPEN;
							pDL = (TileFlagsU & G_TX_MIRROR) ? mip_map_mirror_dl : mip_map_dl;
							break;

						case TEXTURE_EFFECT_NONE:
							detail = G_TD_CLAMP;
							pDL = (TileFlagsU & G_TX_MIRROR) ? mip_map_mirror_dl : mip_map_dl;
							break;

						default:
							ASSERT(FALSE);
					}

					gDPSetTextureDetail((*ppDLP)++, RDP_ADDRESS(detail));
					gSPDisplayList((*ppDLP)++, RSP_ADDRESS(pDL));

					// disable lookup table
					gDPSetTextureLUT((*ppDLP)++, G_TT_NONE);
				}
				break;

			default:
				ASSERT(FALSE);
		}
	}
	else
	{
		// put pipe sync here?
		gDPPipeSync((*ppDLP)++);
	}

	if (pFormat->m_Format == TEXTURE_FORMAT_MM16RGBA)
	{
		gSPTexture((*ppDLP)++,
					  MultU, MultV,
					  5, G_TX_RENDERTILE, G_ON);
					  //5, 1, G_ON);
	}
	else
	{
		gSPTexture((*ppDLP)++,
					  MultU, MultV,
					  0, 0, G_ON);
	}

	CIndexedSet__Destruct(&isBitmaps);
	CIndexedSet__Destruct(&isPalettes);
	CIndexedSet__Destruct(&isTextureSet);
}
