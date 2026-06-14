// cart.h

#ifndef _INC_CART
#define _INC_CART

/////////////////////////////////////////////////////////////////////////////

#include "hash.h"
#include "memory.h"

#ifdef WIN32
#include "utils.h"
// don't fuss for singed - unsigned conversions
#pragma warning(disable : 4018)
// don't fuss when functions are automatically made inline
#pragma warning(disable : 4711)
#endif

/////////////////////////////////////////////////////////////////////////////

// preallocated cache and notify entries
#define	CARTCACHE_CACHEENTRY_COUNT		512
#define	CARTCACHE_NOTIFYENTRY_COUNT	16

// compression types
#define	CMPTYPE_FASTER						0
#define	CMPTYPE_MIXED						1
#define	CMPTYPE_BETTER						2

/////////////////////////////////////////////////////////////////////////////

// reverse byte order if for use with little endian processors
#define SWAP_U32(a) ( ((a) << 24) | 	\
		(((a) << 8) & 0x00ff0000) | 		\
		(((a) >> 8) & 0x0000ff00) | 		\
		((unsigned long)(a) >>24) )

#ifdef WIN32
//#define ORDERBYTES(data) SWAP_U32(data)
#define ORDERBYTES(data) RevBytes(data)            /* their PC tools: C++ overloaded per type */
#elif defined(PLATFORM_PORT)
/* PORT endianness (CLAUDE.md §2): little-endian host reading big-endian ROM data — every
 * ORDERBYTES read must byte-swap (the N64 path is a no-op, being natively big-endian). Their
 * WIN32 tools used a C++ overloaded RevBytes; the C port replaces it with a TYPE-GENERIC swap.
 *
 * Dispatch by SIZE via __builtin_choose_expr (whose UN-chosen branch is not type-checked) so
 * 4- and 2-byte scalars (u32/s32/float, u16/s16) swap in a type-preserving union, while larger
 * aggregates (CVector3 = 12 bytes, etc.) pass through unchanged WITHOUT their type needing to
 * be visible in this low-level header. Aggregates that need element-wise swapping are handled
 * in the asset-decode layer, not here. This fixes the cart-directory DWORD offsets (the
 * first-frame wild-memcpy crash) and every other scalar ROM read in one shot. */
#define __TUROK_OB32(data) ({ union { __typeof__(data) _v; unsigned int   _u; } _t; \
                              _t._v = (data); _t._u = __builtin_bswap32(_t._u); _t._v; })
#define __TUROK_OB16(data) ({ union { __typeof__(data) _v; unsigned short _u; } _t; \
                              _t._v = (data); _t._u = __builtin_bswap16(_t._u); _t._v; })
#define ORDERBYTES(data) __builtin_choose_expr(sizeof(data) == 4, __TUROK_OB32(data),  \
                         __builtin_choose_expr(sizeof(data) == 2, __TUROK_OB16(data),  \
                         (data)))
#else
#define ORDERBYTES(data) (data)                    /* N64: native big-endian, no swap */
#endif

#define FixAlignment(Size, Alignment) ( ( (Size) + ((Alignment) - 1) ) & ~((DWORD) (Alignment) - 1) )

typedef ROMAddress;    	// for type checking cartrridge ptrs vs. memory ptrs

/////////////////////////////////////////////////////////////////////////////

#define SS_NHEADERS     1

#define SS_SIZE         0

#define SS_HEADER_SIZE (SS_NHEADERS*sizeof(DWORD))
#define SS_TOTAL_SIZE(Size) (US_HEADER_SIZE + Size)

typedef struct CStreamSet_t
{
   void     *m_pData;
   BOOL     m_AutoDeleteData;
   DWORD    m_CurrentAllocSize;
} CStreamSet;

// CStreamSet operations
/////////////////////////////////////////////////////////////////////////////

void		CStreamSet__Construct(CStreamSet *pThis);
void		CStreamSet__ConstructFromRawData(CStreamSet *pThis,
														void *pData, BOOL AutoDeleteData /*=FALSE*/);
#ifdef WIN32
#define 	CStreamSet__Destruct(pThis) if ((pThis)->m_pData && (pThis)->m_AutoDeleteData) DEALLOC((pThis)->m_pData)
#else
#define 	CStreamSet__Destruct(pThis)
#endif

void 		CStreamSet__AddBlock(CStreamSet *pThis, const void *pBlock, const DWORD Size);
void		CStreamSet__AddBlockBYTE(CStreamSet *pThis, BYTE bData);
void		CStreamSet__AddBlockWORD(CStreamSet *pThis, WORD wData);
void		CStreamSet__AddBlockDWORD(CStreamSet *pThis, DWORD dwData);

#define	CStreamSet__GetBasePtr(pThis) ( (void*) ( ((BYTE*)((pThis)->m_pData)) + SS_HEADER_SIZE ) )
void* 	CStreamSet__GetBasePtrAndSize(CStreamSet *pThis, DWORD *pSize);

// storage
void 		CStreamSet__GetRawData(CStreamSet *pThis, void **ppData, DWORD *pSize);
void 		CStreamSet__AttatchRawData(CStreamSet *pThis, void *pData, BOOL AutoDeleteData /*=FALSE*/);

/////////////////////////////////////////////////////////////////////////////

#define US_NHEADERS     2

#define US_BLOCKSIZE    0
#define US_BLOCKCOUNT   1

#define US_HEADER_SIZE (US_NHEADERS*sizeof(DWORD))
#define US_TOTAL_SIZE(BlockSize, BlockCount) (US_HEADER_SIZE + (BlockSize)*(BlockCount))

typedef struct CUnindexedSet_t
{
   void  	*m_pData;
   BOOL  	m_AutoDeleteData;
   DWORD 	m_CurrentAllocSize;
} CUnindexedSet;

struct CIndexedSet_t;

// CUnindexedSet operations
/////////////////////////////////////////////////////////////////////////////

void		CUnindexedSet__Construct(CUnindexedSet *pThis);
void		CUnindexedSet__ConstructWithSize(CUnindexedSet *pThis,
													 	int BlockSize, int BlockCount, BOOL AutoDeleteData /*=TRUE*/);
void		CUnindexedSet__ConstructFromRawData(CUnindexedSet *pThis,
															void *pData, BOOL AutoDeleteData /*=FALSE*/);
void		CUnindexedSet__ConstructWithAllocatedBlock(CUnindexedSet *pThis,
																	 void *pData, int BlockSize, int BlockCount, BOOL AutoDeleteData /*=FALSE*/);
#ifdef WIN32
#define 	CUnindexedSet__Destruct(pThis) if ((pThis)->m_pData && (pThis)->m_AutoDeleteData) DEALLOC((pThis)->m_pData)
#else
#define	CUnindexedSet__Destruct(pThis)
#endif

void		CUnindexedSet__PreAllocate(CUnindexedSet *pThis, int BlockSize, int BlockCount);

int		CUnindexedSet__AddBlock(CUnindexedSet *pThis, const void *pBlock, const DWORD Size);
int		CUnindexedSet__AddBlockBYTE(CUnindexedSet *pThis, BYTE bData);
int		CUnindexedSet__AddBlockWORD(CUnindexedSet *pThis, WORD wData);
int		CUnindexedSet__AddBlockDWORD(CUnindexedSet *pThis, DWORD dwData);
int		CUnindexedSet__AddBlockFloat(CUnindexedSet *pThis, float fData);
int		CUnindexedSet__AddBlockIS(CUnindexedSet *pThis, struct CIndexedSet_t *pisBlock);
int		CUnindexedSet__AddBlockUS(CUnindexedSet *pThis, CUnindexedSet *pusBlock);
int		CUnindexedSet__AddBlockSS(CUnindexedSet *pThis, CStreamSet *ssBlock);

#define	CUnindexedSet__GetBasePtr(pThis) ( (void*) ( ((BYTE*)((pThis)->m_pData)) + US_HEADER_SIZE ) )
void* 	CUnindexedSet__GetBasePtrAndSize(CUnindexedSet *pThis, DWORD *pSize);

#define 	CUnindexedSet__GetBlockSize(pThis) ( (DWORD) ORDERBYTES(((DWORD*) (pThis)->m_pData)[US_BLOCKSIZE]) )
#define 	CUnindexedSet__GetBlockCount(pThis) ( (DWORD) ORDERBYTES(((DWORD*) (pThis)->m_pData)[US_BLOCKCOUNT]) )
#define	CUnindexedSet__GetBlock(pThis, nBlock) ( (void*) ( ((BYTE*) CUnindexedSet__GetBasePtr(pThis)) + (nBlock)*CUnindexedSet__GetBlockSize(pThis) ) )

// storage
void 		CUnindexedSet__GetRawData(CUnindexedSet *pThis, void **ppData, DWORD *pSize);
void 		CUnindexedSet__AttatchRawData(CUnindexedSet *pThis, void *pData, BOOL AutoDeleteData /*=FALSE*/);
#ifdef WIN32
BOOL 		CUnindexedSet__WriteToFile(CUnindexedSet *pThis, const char *FileName);
BOOL 		CUnindexedSet__ReadFromFile(CUnindexedSet *pThis, const char *FileName);
BOOL		CUnindexedSet__Interleave(CUnindexedSet *pThis);
BOOL		CUnindexedSet__Deinterleave(CUnindexedSet *pThis);
#endif
void		CUnindexedSet__DeinterleaveCopy(CUnindexedSet *pThis, void *pDest, BOOL CopyBack);

/////////////////////////////////////////////////////////////////////////////

#define IS_ALIGNMENT		8
/*
typedef union {
    DWORD v;
    BYTE	 force_alignment[IS_ALIGNMENT];
} CISAlignedValue;
*/
#define IS_NHEADERS     1

#define IS_BLOCKCOUNT   0

#define IS_HEADER_SIZE (IS_NHEADERS*sizeof(DWORD))
#define IS_INDEX_SIZE(nBlocks) FixAlignment(IS_HEADER_SIZE + ((nBlocks)+1)*sizeof(DWORD), IS_ALIGNMENT)


typedef struct CIndexedSet_t
{
   void  	*m_pData;
   BOOL  	m_AutoDeleteData;
} CIndexedSet;

// CIndexedSet operations
/////////////////////////////////////////////////////////////////////////////

void		   CIndexedSet__Construct(CIndexedSet *pThis);
void		   CIndexedSet__ConstructFromRawData(CIndexedSet *pThis, void *pData, BOOL AutoDeleteData /*=FALSE*/);
#ifdef WIN32
#define		CIndexedSet__Destruct(pThis) if ((pThis)->m_pData && (pThis)->m_AutoDeleteData) DEALLOC((pThis)->m_pData)
#else
#define		CIndexedSet__Destruct(pThis)
#endif

int 		   CIndexedSet__AddBlockDWORD(CIndexedSet *pThis, DWORD dwData);
int         CIndexedSet__AddBlockSS(CIndexedSet *pThis, CStreamSet *ssBlock);
int 		   CIndexedSet__AddBlockUS(CIndexedSet *pThis, CUnindexedSet *pusBlock);
int 		   CIndexedSet__AddBlockIS(CIndexedSet *pThis, CIndexedSet *pisBlock);
int 		   CIndexedSet__AddBlock(CIndexedSet *pThis, const void *pBlock, DWORD Size);
int			CIndexedSet__AddBlocks(CIndexedSet *pThis, void *pBlocks[], DWORD Sizes[], int nBlocks);
int			CIndexedSet__AddCompressedBlockIS(CIndexedSet *pThis, CIndexedSet *pisBlock, DWORD *pCompressedSize, BOOL Better);
int			CIndexedSet__AddCompressedBlockUS(CIndexedSet *pThis, CUnindexedSet *pusBlock, DWORD *pCompressedSize, BOOL Better);
int			CIndexedSet__AddCompressedBlockSS(CIndexedSet *pThis, CStreamSet *pssBlock, DWORD *pCompressedSize, BOOL Better);
int			CIndexedSet__CompressAndAddBlock(CIndexedSet *pThis, void *pData, DWORD Length, DWORD *pCompressedSize, BOOL Better);

void*       CIndexedSet__GetAddress(CIndexedSet *pThis, void *pBaseAddress, int nBlock);
void*       CIndexedSet__GetAddressAndSize(CIndexedSet *pThis, void *pBaseAddress, int nBlock, DWORD *pLength);

ROMAddress* CIndexedSet__GetROMAddress(CIndexedSet *pThis,
                                       ROMAddress *rpBaseAddress, int nBlock);
ROMAddress* CIndexedSet__GetROMAddressAndSize(CIndexedSet *pThis,
                                              ROMAddress *rpBaseAddress, int nBlock, DWORD *pLength);

#define	   CIndexedSet__GetBlock(pThis, nBlock) CIndexedSet__GetAddress((pThis), (pThis)->m_pData, (nBlock))
#define	   CIndexedSet__GetBlockAndSize(pThis, nBlock, pLength) CIndexedSet__GetAddressAndSize((pThis), (pThis)->m_pData, (nBlock), (pLength))
#define	   CIndexedSet__GetBlockCount(pThis) ORDERBYTES(((DWORD*) (pThis)->m_pData)[IS_BLOCKCOUNT])

#ifdef SHIP_IT
#define		CIndexedSet__GetOffset(pThis, nBlock) ORDERBYTES( ((DWORD*) ((pThis)->m_pData))[IS_NHEADERS + (nBlock)] )
#else
DWORD 	   CIndexedSet__GetOffset(CIndexedSet *pThis, const int nBlock);
#endif
DWORD 	   CIndexedSet__GetOffsetAndSize(CIndexedSet *pThis, const int nBlock, DWORD *pLength);

#define		CIndexedSet__SetBlockCount(pThis, nBlocks) ((DWORD*) ((pThis)->m_pData))[IS_BLOCKCOUNT] = ORDERBYTES((DWORD) (nBlocks))
#define		CIndexedSet__SetNextOffset(pThis, Offset) ((DWORD*) ((pThis)->m_pData))[IS_NHEADERS + CIndexedSet__GetBlockCount(pThis)] = ORDERBYTES((DWORD) (Offset))
#define		CIndexedSet__GetNextOffset(pThis) ORDERBYTES( ((DWORD*) ((pThis)->m_pData))[IS_NHEADERS + CIndexedSet__GetBlockCount(pThis)] )

// storage
void 		   CIndexedSet__GetRawData(CIndexedSet *pThis, void **ppData, DWORD *pSize);
void 		   CIndexedSet__AttatchRawData(CIndexedSet *pThis, void *pData, BOOL AutoDeleteData /*=FALSE*/);
void			CIndexedSet__DetatchRawData(CIndexedSet *pThis, void **ppData, DWORD *pSize);
#ifdef WIN32
BOOL 		   CIndexedSet__WriteToFile(CIndexedSet *pThis, const char *FileName);
BOOL 		   CIndexedSet__ReadFromFile(CIndexedSet *pThis, const char *FileName);
#endif
/////////////////////////////////////////////////////////////////////////////

#define CACHE_BLOCK_VERSION_MASK			0xc0000000
#define CACHE_BLOCK_VERSION(v)			((v) << 30)
#define CACHE_BLOCK_GETVERSION(addr)	(((DWORD)(addr)) >> 30)

// forward declaration
struct CCartCache_t;

// stored by requested address
typedef struct CCacheEntry_t
{
   void           *m_pData;         // address of data in memory
	DWORD				m_DataLength;		// length of data in memory

   ROMAddress     *m_rpAddress;     // address of requested data
   DWORD          m_rLength;        // length of requested data

	struct CCartCache_t	*m_pOwner;			// cart cache that this cache entry belongs to

	//int            m_Age;            // age cycles left
	DWORD				m_LastAccessed;	// last used frame number
   BOOL           m_InHashTable,    // cache entries can be in linked list, but not hash table
						m_Compressed;		// if TRUE, object has been compressed and is waiting to discard

#define CACHEENTRY_ALLOCATED 		489554
#define CACHEENTRY_UNALLOCATED 	89355
	DWORD				m_Allocated;		// for debugging
   const char     *m_szDescription; // for debugging

   struct CCacheEntry_t 	*m_pLast, *m_pNext,	// in use list
									*m_pNextFree;			// free list
} CCacheEntry;

// CCacheEntry operations
/////////////////////////////////////////////////////////////////////////////

#define		CCacheEntry__ResetAge(pThis) CCartCache__ResetAge((pThis)->m_pOwner, pThis)
// calls ResetAge()
void* 		CCacheEntry__GetData(CCacheEntry *pThis);
// doesn't call ResetAge()
#define		CCacheEntry__GetDataPtr(pThis) ((pThis)->m_pData)
#define		CCacheEntry__GetRequestedAddress(pThis) ((pThis)->m_rpAddress)
// use to decompress data during decompress message
#define		CCacheEntry__AllocDataForReplacement(pThis, NewSize) CCacheEntry__DoAllocDataForReplacement(pThis, NewSize, FALSE)
#define		CCacheEntry__AllocTempDataForReplacement(pThis, NewSize) CCacheEntry__DoAllocDataForReplacement(pThis, NewSize, TRUE)
void*			CCacheEntry__DoAllocDataForReplacement(CCacheEntry *pThis, DWORD NewSize, BOOL AllocTop);
void 			CCacheEntry__DeallocAndReplaceData(CCacheEntry *pThis, void *pNewData, DWORD NewLength);
void 			CCacheEntry__DeallocData(CCacheEntry *pThis);
void			CCacheEntry__DecompressCallback(void *pVoid, CCacheEntry **ppceTarget);
#define		CCacheEntry__Decompress(pThis) CCacheEntry__DoDecompress(pThis, FALSE)
#define		CCacheEntry__DecompressTemp(pThis) CCacheEntry__DoDecompress(pThis, TRUE)
BOOL			CCacheEntry__DoDecompress(CCacheEntry *pThis, BOOL AllocTop);

/////////////////////////////////////////////////////////////////////////////

typedef  void (*pfnCACHENOTIFY)(void *pNotifyPtr, CCacheEntry **ppceTarget);

// stored in linked list
typedef struct CNotifyEntry_t
{
   void           *m_pNotifyPtr;          // pointer to pass to callback functions
   pfnCACHENOTIFY m_pfnDecompress,        // give to requester to decompress block
                                          // to use, ALLOC() new block, then call CCacheEntry__DeleteAndReplaceData()
                  m_pfnBlockReceived;     // notifies requester that block is ready

   CCacheEntry    **m_ppceTarget;         // address of pointer to copy cache entry address to when data arrives

   ROMAddress     *m_rpRequestedAddress;  // address of data requested from cardridge
   DWORD          m_rLength;              // length of requested data
   const char     *m_szDescription;       // for debugging

   struct CNotifyEntry_t 	*m_pLast, *m_pNext,	// in use list
									*m_pNextFree;			// free list
} CNotifyEntry;

/////////////////////////////////////////////////////////////////////////////

typedef struct CCartCache_t
{
   CHashTable     m_HashTable;
   CNotifyEntry   *m_pNotifyHead, *m_pNotifyTail;
   CCacheEntry    *m_pCacheHead, *m_pCacheTail;
   int            m_DiscardAge;

	CCacheEntry		m_CacheEntries[CARTCACHE_CACHEENTRY_COUNT];
	CCacheEntry		*m_pCacheFreeHead;

	CNotifyEntry	m_NotifyEntries[CARTCACHE_NOTIFYENTRY_COUNT];
	CNotifyEntry	*m_pNotifyFreeHead;

} CCartCache;

// CCartCache operations
/////////////////////////////////////////////////////////////////////////////

void 		CCartCache__Construct(CCartCache *pThis);
void		CCartCache__Destruct(CCartCache *pThis);
void 		CCartCache__RequestBlock(CCartCache *pThis,
											 void *pNotifyPtr,	               // pointer to pass to callback functions
                  					 pfnCACHENOTIFY pfnDecompress,		// decompression callback (can be NULL)
                  					 pfnCACHENOTIFY pfnBlockReceived,	// block ready callback (can be NULL)
                  					 CCacheEntry **ppceTarget,          // address to recieve block ptr
                  					 ROMAddress *rpRequestAddress, DWORD rLength, // where on cartridge
                  					 const char *szDescription /*=""*/);// debugging description

BOOL		CCartCache__InCache(CCartCache *pThis, ROMAddress *rpAddress);
#define	CCartCache__SetDiscardAge(pThis, DiscardAge) (pThis)->m_DiscardAge = DiscardAge
//void 		CCartCache__AgeEntries(CCartCache *pThis);
void 		CCartCache__Empty(CCartCache *pThis);
BOOL 		CCartCache__Compress(CCartCache *pThis);
void 		CCartCache__ResetAge(CCartCache *pThis, CCacheEntry *pCacheEntry);
void		CCartCache__KeepAroundAnotherFrame(CCartCache *pThis);

// Compression operations
/////////////////////////////////////////////////////////////////////////////

void		CMP_CompressData(BOOL better, void *pData, DWORD Length, BYTE **ppOut, DWORD *pLength);
void		CMP_DeallocData(BYTE *pData);
DWORD		CMP_GetDecompressedSize(void *pCompressedData);
BOOL		CMP_DecompressData(void *pCompressedData, void *pOutputBuffer);

/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32

class CFastISBuilder
{
// Construction
public:
	CFastISBuilder(int nBlocks);
	~CFastISBuilder();

// Operations
public:
	int AddBlock(void *pBlock, DWORD Size);
	int AddBlock(CIndexedSet *pisBlock);
	int AddBlock(CUnindexedSet *pusBlock);

	void AddTo(CIndexedSet *pisDest);

// Data members
protected:
	int				m_cBlock, m_nBlocks;
	DWORD				*m_Sizes;
	void				**m_pBlocks;
};

#endif

/////////////////////////////////////////////////////////////////////////////
#endif // _INC_CART
