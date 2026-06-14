// simppool.h

#ifndef _INC_SIMPPOOL
#define _INC_SIMPPOOL

#include "romstruc.h"

// forward declarations
struct CScene_t;

/////////////////////////////////////////////////////////////////////////////

#define	DYNSIMPLE_ALLOCATED		(1 << 0)
#define	DYNSIMPLE_HASHITGROUND	(1 << 1)

typedef struct CDynamicSimple_t
{
	CGameSimpleInstance		s;
	
	FLOAT							m_LifeTime ;		// 0 = forever
	FLOAT							m_SparkleTime ;	// Sparkling timer
	DWORD							m_dwFlags;
	struct CDynamicSimple_t	**m_ppPickup ;		// Ptr to monitor var (pickup generator)
	struct CDynamicSimple_t	*m_pLast, *m_pNext;
} CDynamicSimple;

// CDynamicSimple operations
/////////////////////////////////////////////////////////////////////////////

void		CDynamicSimple__Construct(CDynamicSimple *pThis);
void		CDynamicSimple__Advance(CDynamicSimple *pThis);
void		CDynamicSimple__Delete(CDynamicSimple *pThis);

#define	CDynamicSimple__IsAllocated(pThis) ((pThis)->m_dwFlags & DYNSIMPLE_ALLOCATED)
#define	CDynamicSimple__SetAllocated(pThis) ((pThis)->m_dwFlags |= DYNSIMPLE_ALLOCATED)
#define	CDynamicSimple__SetNotAllocated(pThis) ((pThis)->m_dwFlags &= ~DYNSIMPLE_ALLOCATED)

/////////////////////////////////////////////////////////////////////////////

#define SIMPLES_MAX_COUNT	32

typedef struct CSimplePool_t
{
	CDynamicSimple		m_Simples[SIMPLES_MAX_COUNT],
							*m_pFreeHead,
							*m_pActiveHead, *m_pActiveTail;
	CCartCache			*m_pCache;
	CCacheEntry			*m_pceObjectsIndex;
	ROMAddress			*m_rpObjectsAddress;
	BOOL					m_Initialized;
} CSimplePool;

// CSimplePool operations
/////////////////////////////////////////////////////////////////////////////

void			CSimplePool__Construct(CSimplePool *pThis);
void			CSimplePool__Initialize(CSimplePool *pThis, CCartCache *pCache,
												CCacheEntry *pceObjectsIndex, ROMAddress *rpObjectsAddress);

CDynamicSimple* CSimplePool__CreateSimpleInstance(CSimplePool *pThis, int nType,
																  CVector3 vVelocity,
																  CVector3 vPos, CGameRegion *pRegion, FLOAT Time) ;

void			CSimplePool__Advance(CSimplePool *pThis);
void			CSimplePool__Draw(CSimplePool *pThis, Gfx **ppDLP, CCacheEntry *pceTextureSetsIndex);
void			CSimplePool__AddToCollisionList(CSimplePool *pThis, struct CScene_t *pScene);

/////////////////////////////////////////////////////////////////////////////
#endif // _INC_SIMPPOOL
