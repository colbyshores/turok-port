/*****************************************************************************
*
*
*
*				Simple lists by Stephen Broumley
*
*
*
*****************************************************************************/


#include "stdafx.h"
#include "lists.h"




/*****************************************************************************
*
*
*
*	LIST CODE
*
*
*
*****************************************************************************/



/*****************************************************************************
*
*	Function:		CList__Construct()
*
******************************************************************************
*
*	Description:	Initializes list pointers and sets length to zero
*
*	Inputs:			*pThis			- Ptr to list
*
*	Outputs:			None
*
*****************************************************************************/
void CList__Construct(CList *pThis)
{
	pThis->m_pHead = NULL ;
	pThis->m_pTail = NULL ;
	pThis->m_Length = 0 ;
}



/*****************************************************************************
*
*	Function:		CList__AppendNode()
*
******************************************************************************
*
*	Description:	Adds general node onto the end of list
*
*	Inputs:			*pThis		- Ptr to list
*						*pNode		- Ptr to node to add
*
*	Outputs:			None
*
*****************************************************************************/
void CList__AppendNode(CList *pThis, CNode *pNode)
{
	// Empty list?
	if (!pThis->m_Length)
	{
		pThis->m_pHead = pNode ;
		pThis->m_pTail = pNode ;

		pNode->m_pPrev = NULL ;
		pNode->m_pNext = NULL ;
	}
	else
	{
		pNode->m_pPrev = pThis->m_pTail ;
		pNode->m_pNext = NULL ;

		pThis->m_pTail->m_pNext = pNode ;
		pThis->m_pTail = pNode ;
	}

	pThis->m_Length++ ;
}


/*****************************************************************************
*
*	Function:		CList__DeleteNode()
*
******************************************************************************
*
*	Description:	Deletes node from list
*
*	Inputs:			*pThis		- Ptr to list
*						*pNode		- Ptr to node to delete (must be in list!!!)
*
*	Outputs:			None
*
*****************************************************************************/
void CList__DeleteNode(CList *pThis, CNode *pNode)
{
	// Rmove from list
	if (pNode->m_pNext)
		pNode->m_pNext->m_pPrev = pNode->m_pPrev ;

	if (pNode->m_pPrev)
		pNode->m_pPrev->m_pNext = pNode->m_pNext ;

	// Was node head?
	if (pThis->m_pHead == pNode)
		pThis->m_pHead = pNode->m_pNext ;

	// Was node tail?
	if (pThis->m_pTail == pNode)
		pThis->m_pTail = pNode->m_pPrev ;

	pThis->m_Length-- ;
}



/*****************************************************************************
*
*	Function:		CDynamicList__Construct()
*
******************************************************************************
*
*	Description:	Initializes dynamic list by adding all data itmes to the
*						free list and clearing the active list
*
*	Inputs:			*pThis		- Ptr to dynamic list
*						*NodeData	- Ptr to array of nodes
*						NodeSize		- Size of structure node is at top of
*						Total			- Total number of nodes in array
*
*	Outputs:			None
*
*****************************************************************************/
void CDynamicList__Construct(CDynamicList *pThis, UINT8 *NodeData, INT32 NodeSize, INT32 Total)
{
	CList__Construct(&pThis->m_FreeList) ;
	CList__Construct(&pThis->m_ActiveList) ;
	while(Total--)
	{
		CList__AppendNode(&pThis->m_FreeList, (CNode *)NodeData) ;
		NodeData += NodeSize; 
	}
}



/*****************************************************************************
*
*	Function:		CDynamicList__AllocateNode()
*
******************************************************************************
*
*	Description:	If available, deletes node from free list and appends it to
*						the active list - hence allocation.
*
*	Inputs:			*pThis		- Ptr to dynamic list
*
*	Outputs:			Ptr to active node, or NULL if none available
*
*****************************************************************************/
CNode *CDynamicList__AllocateNode(CDynamicList *pThis)
{
	CNode *pNode ;

	pNode = pThis->m_FreeList.m_pHead ;
	if (pNode)
	{
		CList__DeleteNode(&pThis->m_FreeList, pNode) ;
		CList__AppendNode(&pThis->m_ActiveList, pNode) ;		
	}	

	return pNode ;
}


/*****************************************************************************
*
*	Function:		CDynamicList__DeallocateNode()
*
******************************************************************************
*
*	Description:	Deletes node from active list and puts it in the free list
*
*	Inputs:			*pThis		- Ptr to dynamic list
*						*pNode		- Ptr to node to delete (must be in list!!!)
*
*	Outputs:			None
*
*****************************************************************************/
void CDynamicList__DeallocateNode(CDynamicList *pThis, CNode *pNode)
{
	CList__DeleteNode(&pThis->m_ActiveList, pNode) ;
	CList__AppendNode(&pThis->m_FreeList, pNode) ;		
}







/////////////////////////////////////////////////////////////////////////////
//
//
//
//
//			Selection code
//
//
//
//
/////////////////////////////////////////////////////////////////////////////



/*****************************************************************************
*
*	Function:		CSelection__Choose()
*
******************************************************************************
*
*	Description:	Chooses value from selection list using weights
*
*	Inputs:			*pThis	-	Ptr to selection list
*
*	Outputs:			Selection number
*
*****************************************************************************/
INT16 CSelection__Choose(CSelection *pThis)
{
	CSelection *pSelect ;
	INT32			TotalWeight, Random ;

	// Calculate total weight in list
	pSelect = pThis ;
	TotalWeight = 0 ;
	while(pSelect->m_Number != -1)
	{
		TotalWeight += pSelect->m_Weight ;
		pSelect++ ;
	}

	// Choose random weight
	Random = RANDOM(TotalWeight) ;

	// Find number associated with weight
	pSelect = pThis ;
	TotalWeight = 0 ;
	while(pSelect->m_Number != -1)
	{
		TotalWeight += pSelect->m_Weight ;
		if (TotalWeight >= Random)
			break ;

		pSelect++ ;
	}

	return pSelect->m_Number ;
}





/////////////////////////////////////////////////////////////////////////////
//
//
//
//
//		Selection List code
//
//
//
//
/////////////////////////////////////////////////////////////////////////////




/*****************************************************************************
*
*	Function:		CSelectionList__Construct()
*
******************************************************************************
*
*	Description:	Initialises selection list to be empty
*
*	Inputs:			*pThis	-	Ptr to selection list
*
*	Outputs:			None
*
*****************************************************************************/
void CSelectionList__Construct(CSelectionList *pThis)
{
	pThis->m_Entries = 0 ;
}


/*****************************************************************************
*
*	Function:		CSelectionList__AddItem()
*
******************************************************************************
*
*	Description:	Adds item to current selection list
*
*	Inputs:			*pThis	-	Ptr to selection list
*						Item		-	Item id
*						Weight	-	Selection weight
*
*	Outputs:			None
*
*****************************************************************************/
void CSelectionList__AddItem(CSelectionList *pThis, INT16 Item, INT16 Weight)
{
	pThis->m_List[pThis->m_Entries].m_Number = Item ;
	pThis->m_List[pThis->m_Entries].m_Weight = Weight ;
	pThis->m_Entries++ ;
}


/*****************************************************************************
*
*	Function:		CSelectionList__ChooseItem()
*
******************************************************************************
*
*	Description:	Chooses item from selection list
*
*	Inputs:			*pThis	-	Ptr to selection list
*
*	Outputs:			Item chosen.
*
*****************************************************************************/
INT16 CSelectionList__ChooseItem(CSelectionList *pThis)
{
	// Default
	if (!pThis->m_Entries)
		return 0 ;

	// Cap off end of list
	pThis->m_List[pThis->m_Entries].m_Number = -1 ;
	return CSelection__Choose(pThis->m_List) ;
}


