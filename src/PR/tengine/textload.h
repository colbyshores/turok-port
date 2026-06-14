// textload.h

#ifndef _INC_TEXTLOAD
#define _INC_TEXTLOAD

#include "cart.h"

// forward declarations
struct CROMTextureFormat_t;
struct CCacheEntry_t;

/////////////////////////////////////////////////////////////////////////////

typedef struct CTextureInfo_t
{
	int								m_nBitmaps,
										m_nPalettes;
	struct CROMTextureFormat_t	*m_pFormat;
} CTextureInfo;

/////////////////////////////////////////////////////////////////////////////

typedef struct CTextureLoader_t
{
	struct CCacheEntry_t	*m_pceTextureSet;
	ROMAddress				*m_rpTextureSet;
	DWORD						m_TextureSetSize;
} CTextureLoader;

// CTextureLoader operations
/////////////////////////////////////////////////////////////////////////////

void		CTextureLoader__ConstructFromAddressSize(CTextureLoader *pThis, ROMAddress *rpTextureSet, DWORD TextureSetSize);
void		CTextureLoader__ConstructFromIndex(CTextureLoader *pThis,
														  CIndexedSet *pisTextureSetsIndex,
														  ROMAddress *rpTextureSets,
														  int nIndex);
BOOL		CTextureLoader__RequestTextureSet(CTextureLoader *pThis, CCartCache *pCache, char *szDescription);
void		CTextureLoader__GetTextureInfo(CTextureLoader *pThis, CTextureInfo *pInfo);
void		CTextureLoader__LoadTexture(CTextureLoader *pThis, Gfx **ppDLP,
												 int nBitmap, int nPalette,
												 int MultU, int MultV,
												 unsigned int TileFlagsU, unsigned int TileFlagsV,
												 unsigned int Shift);
void		CTextureLoader__InvalidateTextureCache();

/////////////////////////////////////////////////////////////////////////////
#endif // _INC_TEXTLOAD
