// lists.h

#ifndef __LISTS_H__
#define __LISTS_H__

/////////////////////////////////////////////////////////////////////////////


#include "defs.h"


/////////////////////////////////////////////////////////////////////////////
// Structures
/////////////////////////////////////////////////////////////////////////////

// Linked list structures
typedef struct CNode_t
{
	struct CNode_t	*m_pPrev ;	// Ptr to previous node
	struct CNode_t	*m_pNext ;	// Ptr to next node
} CNode ;

typedef struct CList_t
{
	CNode	*m_pHead ;				// Ptr to head node
	CNode	*m_pTail ;				// Ptr to tail node
	INT32	m_Length ;				// Total nodes in list
} CList ;


// Dynamic list
typedef struct CDynamicList_t
{
	CList	m_FreeList ;		// Free list
	CList	m_ActiveList ;		// Active list
} CDynamicList ;


/////////////////////////////////////////////////////////////////////////////
// Functions prototypes
/////////////////////////////////////////////////////////////////////////////
void CList__Construct(CList *pThis) ;
void CList__AppendNode(CList *pThis, CNode *pNode) ;
void CList__DeleteNode(CList *pThis, CNode *pNode) ;
void CDynamicList__Construct(CDynamicList *pThis, UINT8 *NodeData, INT32 NodeSize, INT32 Total) ;
CNode *CDynamicList__AllocateNode(CDynamicList *pThis) ;
void CDynamicList__DeallocateNode(CDynamicList *pThis, CNode *pNode) ;




// Selection structure
typedef struct CSelection_t
{
	INT16	m_Number ;
	INT16	m_Weight ;
} CSelection ;


// Selection list structure
typedef struct CSelectionList_t
{
	INT16			m_Entries ;				
	CSelection	m_List[24] ;
} CSelectionList ;


/////////////////////////////////////////////////////////////////////////////
// Selection prototypes
/////////////////////////////////////////////////////////////////////////////
INT16 CSelection__Choose(CSelection *pList) ;

void CSelectionList__Construct(CSelectionList *pThis) ;
void CSelectionList__AddItem(CSelectionList *pThis, INT16 Item, INT16 Weight) ;
INT16 CSelectionList__ChooseItem(CSelectionList *pThis) ;



/////////////////////////////////////////////////////////////////////////////
#endif	// __LISTS_H__

