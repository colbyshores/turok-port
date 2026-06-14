// hash.h

#ifndef _INC_HASH
#define _INC_HASH

/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
// don't fuss when functions are automatically made inline
#pragma warning(disable : 4711)
#endif

/////////////////////////////////////////////////////////////////////////////

// typedefs
typedef void* HASHPOS;
typedef void* BASE_KEY;
typedef void* BASE_ARG_KEY;
typedef void* BASE_VALUE;
typedef void* BASE_ARG_VALUE;

struct CAssoc_t;
struct CHashPlex_t;

typedef struct CHashTable_t
{
	struct CAssoc_t      **m_pHashTable;
	UINT                 m_nHashTableSize;
	int                  m_nCount;
	struct CAssoc_t      *m_pFreeList;
	struct CHashPlex_t   *m_pBlocks;
	int                  m_nBlockSize;
} CHashTable;

// CHashTable operations
/////////////////////////////////////////////////////////////////////////////

void 		CHashTable__Construct(CHashTable *pThis, int nBlockSize /*=10*/);
void		CHashTable__Destruct(CHashTable *pThis);

int 		CHashTable__GetCount(CHashTable *pThis);
BOOL 		CHashTable__IsEmpty(CHashTable *pThis);
BOOL 		CHashTable__Lookup(CHashTable *pThis, void* key, void **prValue);

// add a new (key, value) pair
void		CHashTable__SetAt(CHashTable *pThis, void* key, void* newValue);

// removing existing (key, ?) pair
BOOL 		CHashTable__RemoveKey(CHashTable *pThis, void* key);
void 		CHashTable__RemoveAll(CHashTable *pThis);

// iterating all (key, value) pairs
HASHPOS 	CHashTable__GetStartPosition(CHashTable *pThis);
void 		CHashTable__GetNextAssoc(CHashTable *pThis, HASHPOS *prNextPosition, void **prKey, void **prValue);

// advanced features for derived classes
UINT		CHashTable__GetHashTableSize(CHashTable *pThis);
void 		CHashTable__InitHashTable(CHashTable *pThis, UINT hashSize, BOOL bAllocNow /*=TRUE*/);

/////////////////////////////////////////////////////////////////////////////
#endif // _INC_HASH
