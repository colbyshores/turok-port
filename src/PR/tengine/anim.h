// anim.h

#ifndef _INC_ANIM
#define _INC_ANIM

#include "cart.h"

#ifdef WIN32
#include "model.h"
#else
#include "graphu64.h"
#endif

/////////////////////////////////////////////////////////////////////////////

typedef struct CGameAnimateState_t
{
	CCacheEntry    *m_pceAnim;
	DWORD				m_nAnim,
						m_nDesiredAnim,
						m_nFrames;
	float				m_cFrame;
	BOOL				m_CycleCompleted;
} CGameAnimateState;

// CGameAnimateState operations
/////////////////////////////////////////////////////////////////////////////

void		CGameAnimateState__Construct(CGameAnimateState *pThis);
#define	CGameAnimateState__SetDesiredAnim(pThis, nAnim) (pThis)->m_nDesiredAnim = nAnim
int 		CGameAnimateState__GetTransitionAnim(CGameAnimateState *pThis);
int 		CGameAnimateState__GetNextFrame(CGameAnimateState *pThis);
void 		CGameAnimateState__Advance(CGameAnimateState *pThis);
#define	CGameAnimateState__IsCycleCompleted(pThis) ((pThis)->m_CycleCompleted)

/////////////////////////////////////////////////////////////////////////////

typedef struct CROMTransition_t
{
	WORD		m_BlendFromLength,
				m_nExitToFrame,
				m_BlendStart,
				m_BlendFinish;
} CROMTransition;

// CROMTransition operations
/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
void 		CROMTransition__TakeFromTransitionTable(CROMTransition *pThis, CTransitionTable *pTable);
#endif

/////////////////////////////////////////////////////////////////////////////

typedef struct CROMTransitionEntry_t
{
	DWORD		m_nRequestedAnim,
				m_nWaitForFrame,
				m_nDeliveredAnim;
} CROMTransitionEntry;

// CROMTransitionEntry operations
/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
void		CROMTransitionEntry__TakeFromTransitionEntry(CROMTransitionEntry *pThis, CTransitionEntry *pEntry,
																		int AnimIndices[], int nAnims);
#endif

/////////////////////////////////////////////////////////////////////////////

typedef struct CROMEventEntry_t
{
   unsigned short	m_nFrame,
						m_nEvent;

	float				m_Value;
	CVector3			m_vOffset;

} CROMEventEntry;

// CROMEvent operations
/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
void		CROMEventEntry__TakeFromEventEntry(CROMEventEntry *pThis, CEventEntry *pSource, float Scaler);
#endif

/////////////////////////////////////////////////////////////////////////////
#endif // _INC_ANIM
