// hash.c
//

#include "stdafx.h"
#include "hash.h"
#include "memory.h"

#include "debug.h"

#define BEFORE_START_HASHPOS ((void*)-1L)

typedef struct CHashPlex_t
{
	struct CHashPlex_t   *pNext;
	UINT 			         nMax,
					         nCur;
} CHashPlex;

// CHashPlex operations
/////////////////////////////////////////////////////////////////////////////

static CHashPlex* CHashPlex__Create(CHashPlex **ppHead, UINT nMax, UINT cbElement);
			// like 'calloc' but no zero fill
			// may throw memory exceptions

#define           CHashPlex__data(pThis) ((void*) ((pThis) + 1))
void					CHashPlex__FreeDataChain(CHashPlex *pThis);	// free this one and links


/////////////////////////////////////////////////////////////////////////////

typedef struct CAssoc_t
{
	struct CAssoc_t   *pNext;
	UINT              nHashValue;  // needed for efficient iteration
	void              *key;
	void              *value;
} CAssoc;

/////////////////////////////////////////////////////////////////////////////
// CHashPlex

CHashPlex* CHashPlex__Create(CHashPlex **ppHead, UINT nMax, UINT cbElement)
{
	CHashPlex *p;

	TRACE0("CHashPlex__Create()\r\n");

	ASSERT(nMax > 0 && cbElement > 0);
	p = (CHashPlex*) ALLOCTOP(sizeof(CHashPlex) + nMax * cbElement);

			// may throw exception
	p->nMax 	= nMax;
	p->nCur 	= 0;
	p->pNext = *ppHead;
	*ppHead 	= p;  // change head (adds in reverse order for simplicity)

	return p;
}

// free this one and links
void CHashPlex__FreeDataChain(CHashPlex *pThis)
{
	CHashPlex 	*p,
					*pNext;
	BYTE 			*bytes;

	p = pThis;
	while (p)
	{
		bytes = (BYTE*) p;
		pNext = p->pNext;
		DEALLOC(bytes);

		p = pNext;
	}
}

// CHashTable implementation
/////////////////////////////////////////////////////////////////////////////

CAssoc* 		CHashTable__NewAssoc(CHashTable *pThis);
void 			CHashTable__FreeAssoc(CHashTable *pThis, CAssoc*);
CAssoc* 		CHashTable__GetAssocAt(CHashTable *pThis, void*, UINT*);
// default identity hash - works for most primitive values
// There is no type-checking.  Parameters should be (CHashTable* pThis, void *key)
#define 		CHashTable__HashKey(key) (((UINT)(void*)(DWORD)(void*)key) >> 4)

/////////////////////////////////////////////////////////////////////////////
// CHashTable

void CHashTable__Construct(CHashTable *pThis, int nBlockSize)
{
	ASSERT(nBlockSize > 0);

	pThis->m_pHashTable 		= NULL;
	pThis->m_nHashTableSize	= 17;  // default size
	pThis->m_nCount 			= 0;
	pThis->m_pFreeList 		= NULL;
	pThis->m_pBlocks 			= NULL;
	pThis->m_nBlockSize 		= nBlockSize;
}

void CHashTable__Destruct(CHashTable *pThis)
{
	CHashTable__RemoveAll(pThis);
	ASSERT(pThis->m_nCount == 0);
}

void CHashTable__InitHashTable(CHashTable *pThis, UINT nHashSize, BOOL bAllocNow /*=TRUE*/)
//
// Used to force allocation of a hash table or to override the default
//   hash table size of (which is fairly small)
{
	TRACE0("CHashTable__InitHashTable()\r\n");

	ASSERT(pThis->m_nCount == 0);
	ASSERT(nHashSize > 0);

	ASSERT(bAllocNow || pThis->m_pHashTable == NULL);
	if (pThis->m_pHashTable != NULL)
	{
		// free hash table
		DEALLOC(pThis->m_pHashTable);

		pThis->m_pHashTable = NULL;
	}

	if (bAllocNow)
	{
		//m_pHashTable = new CAssoc* [nHashSize];
		pThis->m_pHashTable = (CAssoc**) ALLOCTOP(sizeof(CAssoc*) * nHashSize);

		memset(pThis->m_pHashTable, 0, sizeof(CAssoc*) * nHashSize);
	}
	pThis->m_nHashTableSize = nHashSize;
}

void CHashTable__RemoveAll(CHashTable *pThis)
{
	if (pThis->m_pHashTable != NULL)
	{
		// free hash table
		DEALLOC(pThis->m_pHashTable);
		pThis->m_pHashTable = NULL;
	}

	pThis->m_nCount 		= 0;
	pThis->m_pFreeList 	= NULL;

	CHashPlex__FreeDataChain(pThis->m_pBlocks);
	pThis->m_pBlocks 		= NULL;
}

/////////////////////////////////////////////////////////////////////////////
// Assoc helpers
// same as CList implementation except we store CAssoc's not CNode's
//    and CAssoc's are singly linked all the time

CAssoc* CHashTable__NewAssoc(CHashTable *pThis)
{
	CHashPlex 	*newBlock;
	CAssoc		*pAssoc;
	int			i;

	if (pThis->m_pFreeList == NULL)
	{
		// add another block
		newBlock = CHashPlex__Create(&pThis->m_pBlocks, pThis->m_nBlockSize, sizeof(CAssoc));
		// chain them into free list
		pAssoc = (CAssoc*) CHashPlex__data(newBlock);
		// free in reverse order to make it easier to debug
		pAssoc += pThis->m_nBlockSize - 1;
		for (i = pThis->m_nBlockSize-1; i >= 0; i--, pAssoc--)
		{
			pAssoc->pNext = pThis->m_pFreeList;
			pThis->m_pFreeList = pAssoc;
		}
	}
	ASSERT(pThis->m_pFreeList != NULL);  // we must have something

	pAssoc = pThis->m_pFreeList;
	pThis->m_pFreeList = pThis->m_pFreeList->pNext;
	pThis->m_nCount++;
	ASSERT(pThis->m_nCount > 0);  // make sure we don't overflow

	pAssoc->key = pAssoc->value = NULL;

	//memset(&pAssoc->key, 0, sizeof(void*));
	//memset(&pAssoc->value, 0, sizeof(void*));

	return pAssoc;
}

void CHashTable__FreeAssoc(CHashTable *pThis, CAssoc *pAssoc)
{
	pAssoc->pNext 			= pThis->m_pFreeList;
	pThis->m_pFreeList 	= pAssoc;
	pThis->m_nCount--;
	ASSERT(pThis->m_nCount >= 0);  // make sure we don't underflow
}

CAssoc* CHashTable__GetAssocAt(CHashTable *pThis, void* key, UINT *pnHash)
// find association (or return NULL)
{
	CAssoc *pAssoc;

	*pnHash = CHashTable__HashKey(key) % pThis->m_nHashTableSize;

	if (pThis->m_pHashTable == NULL)
		return NULL;

	// see if it exists
	for (pAssoc = pThis->m_pHashTable[*pnHash]; pAssoc != NULL; pAssoc = pAssoc->pNext)
	{
		if (pAssoc->key == key)
			return pAssoc;
	}
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////

BOOL CHashTable__Lookup(CHashTable *pThis, void* key, void **prValue)
{
	UINT 		nHash;
	CAssoc	*pAssoc;

	pAssoc = CHashTable__GetAssocAt(pThis, key, &nHash);
	if (pAssoc == NULL)
		return FALSE;  // not in map

	*prValue = pAssoc->value;
	return TRUE;
}

void CHashTable__SetAt(CHashTable *pThis, void* key, void* newValue)
{
	UINT nHash;
	CAssoc* pAssoc;

	if ((pAssoc = CHashTable__GetAssocAt(pThis, key, &nHash)) == NULL)
	{
		if (pThis->m_pHashTable == NULL)
			CHashTable__InitHashTable(pThis, pThis->m_nHashTableSize, TRUE);

		// it doesn't exist, add a new Association
		pAssoc = CHashTable__NewAssoc(pThis);
		pAssoc->nHashValue = nHash;
		pAssoc->key = key;
		// 'pAssoc->value' is a constructed object, nothing more

		// put into hash table
		pAssoc->pNext = pThis->m_pHashTable[nHash];
		pThis->m_pHashTable[nHash] = pAssoc;
	}

	// set new value
	pAssoc->value = newValue;
}


BOOL CHashTable__RemoveKey(CHashTable *pThis, void* key)
// remove key - return TRUE if removed
{
	CAssoc	**ppAssocPrev;
	CAssoc	*pAssoc;

	if (pThis->m_pHashTable == NULL)
		return FALSE;  // nothing in the table

	ppAssocPrev = &pThis->m_pHashTable[CHashTable__HashKey(key) % pThis->m_nHashTableSize];

	for (pAssoc = *ppAssocPrev; pAssoc != NULL; pAssoc = pAssoc->pNext)
	{
		if (pAssoc->key == key)
		{
			// remove it
			*ppAssocPrev = pAssoc->pNext;  // remove from list
			CHashTable__FreeAssoc(pThis, pAssoc);
			return TRUE;
		}
		ppAssocPrev = &pAssoc->pNext;
	}
	return FALSE;  // not found
}


/////////////////////////////////////////////////////////////////////////////
// Iterating

void CHashTable__GetNextAssoc(CHashTable *pThis, HASHPOS *prNextPosition, void **prKey, void **prValue)
{
	CAssoc	*pAssocRet,
				*pAssocNext;
	UINT		nBucket;

	ASSERT(pThis->m_pHashTable != NULL);  // never call on empty map

	pAssocRet = (CAssoc*) *prNextPosition;
	ASSERT(pAssocRet != NULL);

	if (pAssocRet == (CAssoc*) BEFORE_START_HASHPOS)
	{
		// find the first association
		for (nBucket = 0; nBucket < pThis->m_nHashTableSize; nBucket++)
			if ((pAssocRet = pThis->m_pHashTable[nBucket]) != NULL)
				break;
		ASSERT(pAssocRet != NULL);  // must find something
	}

	// find next association
#ifdef WIN32
	ASSERT(AfxIsValidAddress(pAssocRet, sizeof(CAssoc)));
#endif
	if ((pAssocNext = pAssocRet->pNext) == NULL)
	{
		// go to next bucket
		for (nBucket = pAssocRet->nHashValue + 1;
		  nBucket < pThis->m_nHashTableSize; nBucket++)
			if ((pAssocNext = pThis->m_pHashTable[nBucket]) != NULL)
				break;
	}

	*prNextPosition = (HASHPOS) pAssocNext;

	// fill in return data
	*prKey = pAssocRet->key;
	*prValue = pAssocRet->value;
}

/////////////////////////////////////////////////////////////////////////////
