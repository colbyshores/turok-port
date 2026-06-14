// cart.c
//


#include "stdafx.h"
#include "cart.h"
#ifdef WIN32
#include "propack\source\c\pp.h"
#else
#include "pp.h"
#endif

#include "debug.h"

#ifdef WIN32

DWORD frame_number = 0;
#define	HASH_ALLOC_BLOCK	48		// should be > maximum entries in hash table to reduce fragmentation
#define	HASH_TABLE_SIZE	31		// should be prime number

#else

#include "tengine.h"

#endif

//extern WORD Propack_Unpack(RNC_fileptr, BYTE *OutputBuffer);

// TEMP
//#define CACHE_DO_MEMORY_DUMP

//DWORD FixAlignment(DWORD Size, DWORD Alignment)
//{
//	return (Size + (Alignment - 1)) & ~(Alignment - 1);
//}

/////////////////////////////////////////////////////////////////////////////
// CIndexedSet

void CIndexedSet__Construct(CIndexedSet *pThis)
{
	int newSize;

#ifndef WIN32
	ASSERT(FALSE);
#endif

	newSize = IS_INDEX_SIZE(0);
   pThis->m_pData = ALLOC(newSize);
	ASSERT(pThis->m_pData);

   CIndexedSet__SetBlockCount(pThis, 0);
   CIndexedSet__SetNextOffset(pThis, newSize);
   pThis->m_AutoDeleteData = TRUE;
}

void CIndexedSet__ConstructFromRawData(CIndexedSet *pThis, void *pData, BOOL AutoDeleteData /*=FALSE*/)
{
   ASSERT(pData);

   pThis->m_pData 			= pData;
   pThis->m_AutoDeleteData = AutoDeleteData;
}

#ifndef SHIP_IT
DWORD CIndexedSet__GetOffset(CIndexedSet *pThis, const int nBlock)
{
	DWORD *offsets;

	ASSERT(pThis->m_pData);
   ASSERT(nBlock >= 0);
   ASSERT(nBlock < CIndexedSet__GetBlockCount(pThis));

   offsets = (DWORD*) (((BYTE*) pThis->m_pData) + IS_HEADER_SIZE);
   return ORDERBYTES(offsets[nBlock]);
}
#endif

DWORD CIndexedSet__GetOffsetAndSize(CIndexedSet *pThis, const int nBlock, DWORD *pLength)
{
	DWORD *offsets;

   ASSERT(pThis->m_pData);
   ASSERT(nBlock >= 0);
   ASSERT(nBlock < CIndexedSet__GetBlockCount(pThis));
   ASSERT(pLength);

   offsets = (DWORD*) (((BYTE*) pThis->m_pData) + IS_HEADER_SIZE);

   *pLength = ORDERBYTES(offsets[nBlock+1]) - ORDERBYTES(offsets[nBlock]);

   return ORDERBYTES(offsets[nBlock]);
}

void CIndexedSet__GetRawData(CIndexedSet *pThis, void **ppData, DWORD *pSize)
{
   ASSERT(pThis->m_pData);
   ASSERT(ppData);
   ASSERT(pSize);

   *ppData  = (void*) pThis->m_pData;
   *pSize   = CIndexedSet__GetNextOffset(pThis);
}

void CIndexedSet__AttatchRawData(CIndexedSet *pThis, void *pData, BOOL AutoDeleteData /*=FALSE*/)
{
   if (pThis->m_pData && pThis->m_AutoDeleteData)
      DEALLOC(pThis->m_pData);

   pThis->m_pData 			= pData;
   pThis->m_AutoDeleteData = AutoDeleteData;
}

void CIndexedSet__DetatchRawData(CIndexedSet *pThis, void **ppData, DWORD *pSize)
{
	CIndexedSet__GetRawData(pThis, ppData, pSize);

	pThis->m_AutoDeleteData = FALSE;
}

void* CIndexedSet__GetAddress(CIndexedSet *pThis, void *pBaseAddress, int nBlock)
{
   DWORD offset;

   offset = CIndexedSet__GetOffset(pThis, nBlock);
   return (void*) (((DWORD) pBaseAddress) + offset);
}

void* CIndexedSet__GetAddressAndSize(CIndexedSet *pThis, void *pBaseAddress, int nBlock, DWORD *pLength)
{
   DWORD offset;

   offset = CIndexedSet__GetOffsetAndSize(pThis, nBlock, pLength);
   return (void*) (((DWORD) pBaseAddress) + offset);
}

ROMAddress* CIndexedSet__GetROMAddress(CIndexedSet *pThis,
                                       ROMAddress *rpBaseAddress, int nBlock)
{
   DWORD offset;

   offset = CIndexedSet__GetOffset(pThis, nBlock);
   return (ROMAddress*) (((DWORD) rpBaseAddress) + offset);
}

ROMAddress* CIndexedSet__GetROMAddressAndSize(CIndexedSet *pThis,
                                              ROMAddress *rpBaseAddress, int nBlock, DWORD *pLength)
{
   DWORD offset;

   offset = CIndexedSet__GetOffsetAndSize(pThis, nBlock, pLength);
   return (ROMAddress*) (((DWORD) rpBaseAddress) + offset);
}

#ifdef WIN32

int CIndexedSet__AddBlockDWORD(CIndexedSet *pThis, DWORD dwData)
{
   DWORD dwOrdered = ORDERBYTES(dwData);

   return CIndexedSet__AddBlock(pThis, &dwOrdered, sizeof(DWORD));
}

int CIndexedSet__AddBlockIS(CIndexedSet *pThis, CIndexedSet *pisBlock)
{
   void     *pData;
   DWORD    dataSize;

   CIndexedSet__GetRawData(pisBlock, &pData, &dataSize);

   return CIndexedSet__AddBlock(pThis, pData, dataSize);
}

int CIndexedSet__AddBlockUS(CIndexedSet *pThis, CUnindexedSet *pusBlock)
{
   void     *pData;
   DWORD    dataSize;

   CUnindexedSet__GetRawData(pusBlock, &pData, &dataSize);

   return CIndexedSet__AddBlock(pThis, pData, dataSize);
}

int CIndexedSet__AddBlockSS(CIndexedSet *pThis, CStreamSet *ssBlock)
{
   void     *pData;
   DWORD    dataSize;

   CStreamSet__GetRawData(ssBlock, &pData, &dataSize);

   return CIndexedSet__AddBlock(pThis, pData, dataSize);
}

int CIndexedSet__AddCompressedBlockIS(CIndexedSet *pThis, CIndexedSet *pisBlock, DWORD *pCompressedSize, BOOL Better)
{
   void     *pData;
   DWORD    dataSize;

   CIndexedSet__GetRawData(pisBlock, &pData, &dataSize);

	return CIndexedSet__CompressAndAddBlock(pThis, pData, dataSize, pCompressedSize, Better);
}

int CIndexedSet__AddCompressedBlockUS(CIndexedSet *pThis, CUnindexedSet *pusBlock, DWORD *pCompressedSize, BOOL Better)
{
   void     *pData;
   DWORD    dataSize;

   CUnindexedSet__GetRawData(pusBlock, &pData, &dataSize);

	return CIndexedSet__CompressAndAddBlock(pThis, pData, dataSize, pCompressedSize, Better);
}

int CIndexedSet__AddCompressedBlockSS(CIndexedSet *pThis, CStreamSet *pssBlock, DWORD *pCompressedSize, BOOL Better)
{
   void     *pData;
   DWORD    dataSize;

   CStreamSet__GetRawData(pssBlock, &pData, &dataSize);

	return CIndexedSet__CompressAndAddBlock(pThis, pData, dataSize, pCompressedSize, Better);
}

int CIndexedSet__CompressAndAddBlock(CIndexedSet *pThis, void *pData, DWORD Length, DWORD *pCompressedSize, BOOL Better)
{
#ifdef WIN32
	BYTE *pOut;
	DWORD outLength;
	CMP_CompressData(Better, pData, Length, &pOut, &outLength);

	if (pOut)
	{
		int pos = CIndexedSet__AddBlock(pThis, pOut, outLength);
		if (pCompressedSize)
			*pCompressedSize = outLength;

		CMP_DeallocData(pOut);

		return pos;
	}
	else
	{
		if (pCompressedSize)
			*pCompressedSize = 0;

		return -1;
	}
#else
	return -1;
#endif
}

int CIndexedSet__AddBlock(CIndexedSet *pThis, const void *pBlock, DWORD Size)
{
   void     			*pNewData,
            			*pSource, *pDest;
   DWORD					*pdwOld, *pdwNew;
   int      			i, numb,
            			length,
							prevIndexSize, newIndexSize,
							prevDataSize,
							newSize,
							slide,
							alignedSize;

#ifndef WIN32
	ASSERT(FALSE);
#endif

   ASSERT(pThis->m_pData);
   ASSERT(pBlock);
   ASSERT(Size > 0);
   ASSERT(pThis->m_AutoDeleteData);  // adding a block is only possible for dynamic data

	alignedSize = FixAlignment(Size, IS_ALIGNMENT);

	prevIndexSize	= IS_INDEX_SIZE(CIndexedSet__GetBlockCount(pThis));
	newIndexSize	= IS_INDEX_SIZE(CIndexedSet__GetBlockCount(pThis) + 1);
	slide = newIndexSize - prevIndexSize;
   prevDataSize = CIndexedSet__GetNextOffset(pThis) - prevIndexSize;
	newSize = newIndexSize + prevDataSize + alignedSize;
	pNewData = ALLOC(newSize);
   ASSERT(pNewData);
   if (pNewData)
   {
#ifdef _DEBUG
		memset(pNewData, 0xcd, newSize);
#else
		memset(pNewData, 0, newSize);
#endif

      pdwOld = (DWORD*) pThis->m_pData;
      pdwNew = (DWORD*) pNewData;

      // increase block count
      pdwNew[IS_BLOCKCOUNT] = ORDERBYTES(ORDERBYTES(pdwOld[IS_BLOCKCOUNT]) + 1);

      // copy offsets and slide for new value
      numb = ORDERBYTES(pdwOld[IS_BLOCKCOUNT]);
      for (i=0; i<=numb; i++)
         pdwNew[IS_NHEADERS + i] = ORDERBYTES(ORDERBYTES(pdwOld[IS_NHEADERS + i]) + slide);

      // update next offset
		pdwNew[IS_NHEADERS + ORDERBYTES(pdwNew[IS_BLOCKCOUNT])] = ORDERBYTES((DWORD) newSize);

      // copy old data
		pSource	= ((BYTE*) pThis->m_pData) + prevIndexSize;

		pDest		= ((BYTE*) pNewData) + newIndexSize;

		length	= prevDataSize;

		memcpy(pDest, pSource, length);

      // copy new data
		pDest = ((BYTE*) pNewData) + newIndexSize + prevDataSize;
      memcpy(pDest, pBlock, Size);

		ASSERT(pThis->m_pData);
      DEALLOC(pThis->m_pData);

      pThis->m_pData = pNewData;

      return CIndexedSet__GetBlockCount(pThis) - 1;
   }
   else
   {
      return -1;
   }
}

int CIndexedSet__AddBlocks(CIndexedSet *pThis, void *pBlocks[], DWORD Sizes[], int nBlocks)
{
   void     			*pNewData,
            			*pSource, *pDest;
   DWORD					*pdwOld, *pdwNew;
   int      			i, numb,
            			length,
							prevIndexSize, newIndexSize,
							prevDataSize, newDataSize,
							newSize,
							slide,
							cBlock;


#ifndef WIN32
	ASSERT(FALSE);
#endif

   ASSERT(pThis->m_pData);
   ASSERT(pThis->m_AutoDeleteData);  // adding a block is only possible for dynamic data

	newDataSize = 0;
	for (cBlock=0; cBlock<nBlocks; cBlock++)
	{
		Sizes[cBlock] = FixAlignment(Sizes[cBlock], IS_ALIGNMENT);
		newDataSize += Sizes[cBlock];
	}

	prevIndexSize	= IS_INDEX_SIZE(CIndexedSet__GetBlockCount(pThis));
	newIndexSize	= IS_INDEX_SIZE(CIndexedSet__GetBlockCount(pThis) + nBlocks);
	slide = newIndexSize - prevIndexSize;
   prevDataSize = CIndexedSet__GetNextOffset(pThis) - prevIndexSize;
	newSize = newIndexSize + prevDataSize + newDataSize;
	pNewData = ALLOC(newSize);
   ASSERT(pNewData);
   if (pNewData)
   {
#ifdef _DEBUG
		memset(pNewData, 0xcd, newSize);
#else
		memset(pNewData, 0, newSize);
#endif

      pdwOld = (DWORD*) pThis->m_pData;
      pdwNew = (DWORD*) pNewData;

      // copy offsets and slide for new value
      numb = ORDERBYTES(pdwOld[IS_BLOCKCOUNT]);
      for (i=0; i<=numb; i++)
         pdwNew[IS_NHEADERS + i] = ORDERBYTES(ORDERBYTES(pdwOld[IS_NHEADERS + i]) + slide);

      // copy old data
		pSource	= ((BYTE*) pThis->m_pData) + prevIndexSize;
		pDest		= ((BYTE*) pNewData) + newIndexSize;
		length	= prevDataSize;

		memcpy(pDest, pSource, length);

		// copy block count
		pdwNew[IS_BLOCKCOUNT] = pdwOld[IS_BLOCKCOUNT];

		newDataSize = 0;
		for (cBlock=0; cBlock<nBlocks; cBlock++)
		{
			// copy new data
			pDest = ((BYTE*) pNewData) + newIndexSize + prevDataSize + newDataSize;
			memcpy(pDest, pBlocks[cBlock], Sizes[cBlock]);

			// increase block count
			pdwNew[IS_BLOCKCOUNT] = ORDERBYTES(ORDERBYTES(pdwNew[IS_BLOCKCOUNT]) + 1);

			newDataSize += Sizes[cBlock];

			// update next offset
			pdwNew[IS_NHEADERS + ORDERBYTES(pdwNew[IS_BLOCKCOUNT])] = ORDERBYTES((DWORD) (newIndexSize + prevDataSize + newDataSize));
		}

		ASSERT(pThis->m_pData);
      DEALLOC(pThis->m_pData);

      pThis->m_pData = pNewData;

      return CIndexedSet__GetBlockCount(pThis) - 1;
   }
   else
   {
      return -1;
   }
}

BOOL CIndexedSet__WriteToFile(CIndexedSet *pThis, const char *FileName)
{
   int      size;
   FILE     *pFile;

   ASSERT(pThis->m_pData);

   size = CIndexedSet__GetNextOffset(pThis);

   pFile = fopen(FileName, "wb");
   if (pFile)
   {
      fwrite(pThis->m_pData, 1, size, pFile);

      fclose(pFile);
      return TRUE;
   }
   else
   {
      return FALSE;
   }
}

BOOL CIndexedSet__ReadFromFile(CIndexedSet *pThis, const char *FileName)
{
   FILE     *pFile;
   void     *pNewData;

   pFile = fopen(FileName, "rb");
   if (pFile)
   {
      fseek(pFile, 0, SEEK_END);
      long length = ftell(pFile);
      fseek(pFile, 0, SEEK_SET);

      pNewData = ALLOC(length);
      ASSERT(pNewData);
      if (pThis->m_pData)
      {
         fread(pNewData, 1, length, pFile);

         if (pThis->m_pData && pThis->m_AutoDeleteData)
            DEALLOC(pThis->m_pData);

         pThis->m_pData = pNewData;
         pThis->m_AutoDeleteData = TRUE;

         fclose(pFile);
         return TRUE;
      }
      else
      {
         fclose(pFile);
         return FALSE;
      }
   }
   else
   {
      return FALSE;
   }
}

#endif

// CStreamSet implelementation
/////////////////////////////////////////////////////////////////////////////

void 		CStreamSet__SetSize(CStreamSet *pThis, DWORD Size);
DWORD 	CStreamSet__GetSize(CStreamSet *pThis);
#define 	CStreamSet__GetSize(pThis) ( (DWORD) ORDERBYTES(((DWORD*) (pThis)->m_pData)[SS_SIZE]) )

/////////////////////////////////////////////////////////////////////////////
// CStreamSet

#ifdef WIN32

void CStreamSet__Construct(CStreamSet *pThis)
{
   int newSize;

#ifndef WIN32
	ASSERT(FALSE);
#endif

	newSize = SS_HEADER_SIZE;
   pThis->m_pData = ALLOC(newSize);
   ASSERT(pThis->m_pData);
   CStreamSet__SetSize(pThis, newSize);
   pThis->m_CurrentAllocSize = newSize;
   pThis->m_AutoDeleteData = TRUE;
}

void CStreamSet__ConstructFromRawData(CStreamSet *pThis,
												  void *pData, BOOL AutoDeleteData /*=FALSE*/)
{
   ASSERT(pData);

   pThis->m_pData 				= pData;
   pThis->m_CurrentAllocSize 	= 0;
   pThis->m_AutoDeleteData 	= AutoDeleteData;
}

void CStreamSet__SetSize(CStreamSet *pThis, DWORD Size)
{
   DWORD *pdwData;

	pdwData = (DWORD*) pThis->m_pData;
   pdwData[SS_SIZE] = ORDERBYTES(Size);
}

void CStreamSet__GetRawData(CStreamSet *pThis, void **ppData, DWORD *pSize)
{
   ASSERT(pThis->m_pData);
   ASSERT(ppData);
   ASSERT(pSize);

   *ppData  = (void*) pThis->m_pData;
   *pSize = CStreamSet__GetSize(pThis);
}

void CStreamSet__AttatchRawData(CStreamSet *pThis, void *pData, BOOL AutoDeleteData /*=FALSE*/)
{
   if (pThis->m_pData && pThis->m_AutoDeleteData)
      DEALLOC(pThis->m_pData);

   pThis->m_pData = pData;
   pThis->m_CurrentAllocSize = 0;
   pThis->m_AutoDeleteData = AutoDeleteData;
}

void* CStreamSet__GetBasePtrAndSize(CStreamSet *pThis, DWORD *pSize)
{
   ASSERT(pSize);

   *pSize = CStreamSet__GetSize(pThis) - SS_HEADER_SIZE;
   return CStreamSet__GetBasePtr(pThis);
}

void CStreamSet__AddBlockBYTE(CStreamSet *pThis, BYTE bData)
{
   CStreamSet__AddBlock(pThis, &bData, sizeof(BYTE));
}

void CStreamSet__AddBlockWORD(CStreamSet *pThis, WORD wData)
{
   WORD wOrdered = ORDERBYTES(wData);

   CStreamSet__AddBlock(pThis, &wOrdered, sizeof(WORD));
}

void CStreamSet__AddBlockDWORD(CStreamSet *pThis, DWORD dwData)
{
   DWORD dwOrdered = ORDERBYTES(dwData);

   CStreamSet__AddBlock(pThis, &dwOrdered, sizeof(DWORD));
}

void CStreamSet__AddBlock(CStreamSet *pThis, const void *pBlock, const DWORD Size)
{
	DWORD 	oldSize,
				requiredSize,
				newSize;
	void 		*pNewData;

#ifndef WIN32
	ASSERT(FALSE);
#endif

	ASSERT(pThis->m_pData);
   ASSERT(pBlock);
   ASSERT(Size > 0);
   ASSERT(pThis->m_AutoDeleteData);  // adding a block is only possible for dynamic data

   oldSize = CStreamSet__GetSize(pThis);

   requiredSize = oldSize + Size;
   if (pThis->m_CurrentAllocSize < requiredSize)
   {
      newSize = max(2*pThis->m_CurrentAllocSize, requiredSize);
      pNewData = ALLOC(newSize);
      ASSERT(pNewData);
      if (pNewData)
      {
         // copy old data
         memcpy(pNewData, pThis->m_pData, oldSize);

         DEALLOC(pThis->m_pData);
         pThis->m_pData = pNewData;
         pThis->m_CurrentAllocSize = newSize;
      }
      else
      {
         return;
      }
   }

   // copy new data
   memcpy(((BYTE*)pThis->m_pData) + oldSize, pBlock, Size);

   CStreamSet__SetSize(pThis, requiredSize);
}

#endif

// CUnindexedSet implelementation
/////////////////////////////////////////////////////////////////////////////

void 		CUnindexedSet__SetBlockSize(CUnindexedSet *pThis, DWORD BlockSize);
void 		CUnindexedSet__SetBlockCount(CUnindexedSet *pThis, DWORD BlockSize);
DWORD 	CUnindexedSet__GetSize(CUnindexedSet *pThis);

/////////////////////////////////////////////////////////////////////////////
// CUnindexedSet

void CUnindexedSet__Construct(CUnindexedSet *pThis)
{
   int newSize;

#ifndef WIN32
	ASSERT(FALSE);
#endif

	newSize = US_HEADER_SIZE;
   pThis->m_pData = ALLOC(newSize);
   ASSERT(pThis->m_pData);
   CUnindexedSet__SetBlockSize(pThis, 0);
   CUnindexedSet__SetBlockCount(pThis, 0);
   pThis->m_CurrentAllocSize = newSize;
   pThis->m_AutoDeleteData = TRUE;
}

void CUnindexedSet__ConstructWithSize(CUnindexedSet *pThis,
												  int BlockSize, int BlockCount, BOOL AutoDeleteData /*=TRUE*/)
{
   int newSize;

#ifndef WIN32
	ASSERT(FALSE);
#endif

	newSize = US_TOTAL_SIZE(BlockSize, BlockCount);
   pThis->m_pData = ALLOC(newSize);
   ASSERT(pThis->m_pData);
   CUnindexedSet__SetBlockSize(pThis, BlockSize);
   CUnindexedSet__SetBlockCount(pThis, BlockCount);
   pThis->m_CurrentAllocSize = newSize;
   pThis->m_AutoDeleteData = AutoDeleteData;
}

void CUnindexedSet__PreAllocate(CUnindexedSet *pThis, int BlockSize, int BlockCount)
{
   int newSize;

#ifndef WIN32
	ASSERT(FALSE);
#endif

   ASSERT(!CUnindexedSet__GetBlockCount(pThis));

	if (pThis->m_pData)
		DEALLOC(pThis->m_pData);

	newSize = US_TOTAL_SIZE(BlockSize, BlockCount);
   pThis->m_pData = ALLOC(newSize);
   ASSERT(pThis->m_pData);
   CUnindexedSet__SetBlockSize(pThis, BlockSize);
   CUnindexedSet__SetBlockCount(pThis, 0);
   pThis->m_CurrentAllocSize = newSize;
}

void CUnindexedSet__ConstructFromRawData(CUnindexedSet *pThis,
													  void *pData, BOOL AutoDeleteData /*=FALSE*/)
{
   ASSERT(pData);

   pThis->m_pData 				= pData;
   pThis->m_CurrentAllocSize 	= 0;
   pThis->m_AutoDeleteData 	= AutoDeleteData;
}

void CUnindexedSet__ConstructWithAllocatedBlock(CUnindexedSet *pThis,
																void *pData, int BlockSize, int BlockCount, BOOL AutoDeleteData /*=FALSE*/)
{
   ASSERT(pData);
   pThis->m_AutoDeleteData = FALSE;
   CUnindexedSet__AttatchRawData(pThis, pData, AutoDeleteData);
   CUnindexedSet__SetBlockSize(pThis, BlockSize);
   CUnindexedSet__SetBlockCount(pThis, BlockCount);
}

void CUnindexedSet__SetBlockSize(CUnindexedSet *pThis, DWORD BlockSize)
{
   DWORD *pdwData;

	pdwData = (DWORD*) pThis->m_pData;
   pdwData[US_BLOCKSIZE] = ORDERBYTES(BlockSize);
}

void CUnindexedSet__SetBlockCount(CUnindexedSet *pThis, DWORD BlockSize)
{
   DWORD *pdwData;

	pdwData = (DWORD*) pThis->m_pData;
   pdwData[US_BLOCKCOUNT] = ORDERBYTES(BlockSize);
}

DWORD CUnindexedSet__GetSize(CUnindexedSet *pThis)
{
	return US_HEADER_SIZE + CUnindexedSet__GetBlockSize(pThis)*CUnindexedSet__GetBlockCount(pThis);
}

void CUnindexedSet__GetRawData(CUnindexedSet *pThis, void **ppData, DWORD *pSize)
{
   ASSERT(pThis->m_pData);
   ASSERT(ppData);
   ASSERT(pSize);

   *ppData  = (void*) pThis->m_pData;
   *pSize = CUnindexedSet__GetSize(pThis);
}

void CUnindexedSet__AttatchRawData(CUnindexedSet *pThis, void *pData, BOOL AutoDeleteData /*=FALSE*/)
{
   if (pThis->m_pData && pThis->m_AutoDeleteData)
      DEALLOC(pThis->m_pData);

   pThis->m_pData = pData;
   pThis->m_CurrentAllocSize = 0;
   pThis->m_AutoDeleteData = AutoDeleteData;
}

void* CUnindexedSet__GetBasePtrAndSize(CUnindexedSet *pThis, DWORD *pSize)
{
   ASSERT(pSize);

   *pSize = CUnindexedSet__GetSize(pThis) - US_HEADER_SIZE;
   return ((BYTE*)pThis->m_pData) + US_HEADER_SIZE;
}

#ifdef WIN32

int CUnindexedSet__AddBlockBYTE(CUnindexedSet *pThis, BYTE bData)
{
   return CUnindexedSet__AddBlock(pThis, &bData, sizeof(BYTE));
}

int CUnindexedSet__AddBlockWORD(CUnindexedSet *pThis, WORD wData)
{
   WORD wOrdered = ORDERBYTES(wData);

   return CUnindexedSet__AddBlock(pThis, &wOrdered, sizeof(WORD));
}

int CUnindexedSet__AddBlockDWORD(CUnindexedSet *pThis, DWORD dwData)
{
   DWORD dwOrdered = ORDERBYTES(dwData);

   return CUnindexedSet__AddBlock(pThis, &dwOrdered, sizeof(DWORD));
}

int CUnindexedSet__AddBlockFloat(CUnindexedSet *pThis, float fData)
{
	float fOrdered = ORDERBYTES(fData);

	return CUnindexedSet__AddBlock(pThis, &fOrdered, sizeof(float));
}

int CUnindexedSet__AddBlockIS(CUnindexedSet *pThis, struct CIndexedSet_t *pisBlock)
{
   void     *pData;
   DWORD    dataSize;

   CIndexedSet__GetRawData(pisBlock, &pData, &dataSize);

   return CUnindexedSet__AddBlock(pThis, pData, dataSize);
}

int CUnindexedSet__AddBlockUS(CUnindexedSet *pThis, CUnindexedSet *pusBlock)
{
   void     *pData;
   DWORD    dataSize;

   CUnindexedSet__GetRawData(pusBlock, &pData, &dataSize);

   return CUnindexedSet__AddBlock(pThis, pData, dataSize);
}

int CUnindexedSet__AddBlockSS(CUnindexedSet *pThis, CStreamSet *ssBlock)
{
   void     *pData;
   DWORD    dataSize;

   CStreamSet__GetRawData(ssBlock, &pData, &dataSize);

   return CUnindexedSet__AddBlock(pThis, pData, dataSize);
}

int CUnindexedSet__AddBlock(CUnindexedSet *pThis, const void *pBlock, const DWORD Size)
{
	DWORD 	oldSize,
				requiredSize,
				newSize,
				oldBlockCount;
	void 		*pNewData;

#ifndef WIN32
	ASSERT(FALSE);
#endif

	ASSERT(pThis->m_pData);
   ASSERT(pBlock);
   ASSERT(Size > 0);
   ASSERT(pThis->m_AutoDeleteData);  // adding a block is only possible for dynamic data

   if (CUnindexedSet__GetBlockCount(pThis))
      ASSERT(CUnindexedSet__GetBlockSize(pThis) == Size);
   else
      CUnindexedSet__SetBlockSize(pThis, Size);

   oldSize = CUnindexedSet__GetSize(pThis);

   requiredSize = oldSize + Size;
   if (pThis->m_CurrentAllocSize < requiredSize)
   {
      newSize = max(2*pThis->m_CurrentAllocSize, requiredSize);
      pNewData = ALLOC(newSize);
      ASSERT(pNewData);
      if (pNewData)
      {
         // copy old data
         memcpy(pNewData, pThis->m_pData, oldSize);

         DEALLOC(pThis->m_pData);
         pThis->m_pData = pNewData;
         pThis->m_CurrentAllocSize = newSize;
      }
      else
      {
         return -1;
      }
   }

   // copy new data
   memcpy(((BYTE*)pThis->m_pData) + oldSize, pBlock, Size);

	oldBlockCount = CUnindexedSet__GetBlockCount(pThis);
   CUnindexedSet__SetBlockCount(pThis, oldBlockCount + 1);

	return oldBlockCount;
}

BOOL CUnindexedSet__Interleave(CUnindexedSet *pThis)
{
	int blockSize = CUnindexedSet__GetBlockSize(pThis);
	int blockCount = CUnindexedSet__GetBlockCount(pThis);
	BYTE *outBytes = new BYTE[blockCount*blockSize];
	BYTE *inBytes = (BYTE*) CUnindexedSet__GetBasePtr(pThis);
	if (outBytes)
	{
		BYTE *pOutPlace = outBytes;

		for (int cSize=0; cSize<blockSize; cSize++)
		{
			for (int cBlock=0; cBlock<blockCount; cBlock++)
			{
				*pOutPlace = inBytes[cBlock*blockSize + cSize];
				pOutPlace++;
			}
		}

		memcpy(inBytes, outBytes, blockCount*blockSize);

		delete [] outBytes;

		return TRUE;
	}

	return FALSE;
}

BOOL CUnindexedSet__Deinterleave(CUnindexedSet *pThis)
{
	int blockSize = CUnindexedSet__GetBlockSize(pThis);
	int blockCount = CUnindexedSet__GetBlockCount(pThis);
	BYTE *outBytes = new BYTE[blockCount*blockSize];
	BYTE *inBytes = (BYTE*) CUnindexedSet__GetBasePtr(pThis);
	if (outBytes)
	{
		BYTE *pOutPlace = outBytes;

		for (int cBlock=0; cBlock<blockCount; cBlock++)
		{
			for (int cSize=0; cSize<blockSize; cSize++)
			{
				*pOutPlace = inBytes[cSize*blockCount + cBlock];
				pOutPlace++;
			}
		}

		memcpy(inBytes, outBytes, blockCount*blockSize);

		delete [] outBytes;

		return TRUE;
	}

	return FALSE;
}

#endif

void CUnindexedSet__DeinterleaveCopy(CUnindexedSet *pThis, void *pDest, BOOL CopyBack)
{
	BYTE	*inBytes, *pOutPlace;
	int	blockSize, blockCount,
			cSize, cBlock;

	ASSERT(pDest);
	ASSERT(pDest != pThis->m_pData);
	ASSERT(pDest != CUnindexedSet__GetBasePtr(pThis));

	inBytes = (BYTE*) CUnindexedSet__GetBasePtr(pThis);
	pOutPlace = (BYTE*) pDest;

	// copy header
	memcpy(pDest, pThis->m_pData, US_HEADER_SIZE);
	pOutPlace += US_HEADER_SIZE;

	// deinterleave and copy data
	blockSize = CUnindexedSet__GetBlockSize(pThis);
	blockCount = CUnindexedSet__GetBlockCount(pThis);

	for (cBlock=0; cBlock<blockCount; cBlock++)
	{
		for (cSize=0; cSize<blockSize; cSize++)
		{
			*pOutPlace = inBytes[cSize*blockCount + cBlock];
			pOutPlace++;
		}
	}

	if (CopyBack)
		memcpy(pThis->m_pData, pDest, US_HEADER_SIZE + blockCount*blockSize);
}

#ifdef WIN32
BOOL CUnindexedSet__WriteToFile(CUnindexedSet *pThis, const char *FileName)
{
	FILE *pFile;

	ASSERT(pThis->m_pData);

   pFile = fopen(FileName, "wb");
   if (pFile)
   {
      fwrite(pThis->m_pData, 1, CUnindexedSet__GetSize(pThis), pFile);

      fclose(pFile);
      return TRUE;
   }
   else
   {
      return FALSE;
   }
}

BOOL CUnindexedSet__ReadFromFile(CUnindexedSet *pThis, const char *FileName)
{
   FILE 		*pFile;
	void 		*pNewData;

	pFile = fopen(FileName, "rb");
   if (pFile)
   {
      fseek(pFile, 0, SEEK_END);
      long length = ftell(pFile);
      fseek(pFile, 0, SEEK_SET);

      pNewData = ALLOC(length);
      ASSERT(pNewData);
      if (pThis->m_pData)
      {
         fread(pNewData, 1, length, pFile);

         if (pThis->m_pData && pThis->m_AutoDeleteData)
            DEALLOC(pThis->m_pData);

         pThis->m_pData = pNewData;
         pThis->m_AutoDeleteData = TRUE;

         fclose(pFile);
         return TRUE;
      }
      else
      {
         fclose(pFile);
         return FALSE;
      }
   }
   else
   {
      return FALSE;
   }
}
#endif

///////////////////////////////////////////////////////////////////////////////////

/*
void AddBlocks(CIndexedSet& Set)
{
   char *blocks[] = { "[block_a]", "[block_b   ]", "[bl_c]", "[block_d]" };
   for(int i=0; i<4; i++)
      Set.AddBlock(blocks[i], strlen(blocks[i])+1);
}
void PrintContents(CIndexedSet& Set)
{
   int nBlocks = Set.GetBlockCount();
   printf("There are %d blocks\n\n", nBlocks);

   for (int i=0; i<nBlocks; i++)
   {
      DWORD length;
      char *data = (char*) Set.GetBlock(i, &length);
      printf("Block %d: %s\nlength: %d\n", i, data, length);
   }
}

void AddBlocks(CUnindexedSet& Set)
{
   for(DWORD i=0; i<4; i++)
      Set.AddBlock(&i, sizeof(DWORD));
}
void PrintContents(CUnindexedSet& Set)
{
   DWORD *pData = (DWORD*) Set.GetBasePtr();
   for (int i=0; i<4; i++)
      printf("Block %d: %d\n", i, pData[i]);
}

int main(int argc, char *argv[], char *envp[])
{
   {
      CIndexedSet romtest;

      AddBlocks(romtest);

      romtest.WriteToFile("test.bin");
      romtest.ReadFromFile("test.bin");

      PrintContents(romtest);
   }

   printf("\n\n");

   {
      CUnindexedSet romtest2;

      AddBlocks(romtest2);

      romtest2.WriteToFile("test2.bin");
      romtest2.ReadFromFile("test2.bin");

      PrintContents(romtest2);
   }

   // wait for key
   char buffer[1000];
   gets(buffer);

   return 0;
}
*/

/*
// CCartCache Utility
/////////////////////////////////////////////////////////////////////////////

void* CCartCache__BestAlloc(DWORD Size);

void* CCartCache__BestAlloc(DWORD Size)
{
#ifdef COMPRESS_MEMORY

	void *pData;

	pData = ALLOC(Size);

	if (pData)
	{
		if (BlockBelowFreeChunk(pData))
		{
			// use bottom allocation if block is compressed
			return pData;
		}
		else
		{
			// otherwise alloc off top of heap
			// (data will be compressed later)

			DEALLOC(pData);

			return ALLOCTOP(Size);
		}
	}
	else
	{
		return NULL;
	}

#else

	return ALLOC(Size);

#endif
}
*/

// CCartCache implelementation (gives CCacheEntry friend access)
/////////////////////////////////////////////////////////////////////////////

void 				CCartCache__BlockReceived(CCartCache *pThis,
											  		  ROMAddress *rpRequestedAddress, DWORD rLength,
											  		  void *pData);
CCacheEntry* 	CCartCache__AllocCacheEntry(CCartCache *pThis);
void 				CCartCache__DeallocCacheEntry(CCartCache *pThis, CCacheEntry *pEntry);
CNotifyEntry* 	CCartCache__AllocNotifyEntry(CCartCache *pThis);
void 				CCartCache__DeallocNotifyEntry(CCartCache *pThis, CNotifyEntry *pEntry);
void 				CCartCache__DeleteCacheEntry(CCartCache *pThis, CCacheEntry *pCacheEntry);
void 				CCartCache__DeleteNotifyEntry(CCartCache *pThis, CNotifyEntry *pNotify);
CCacheEntry* 	CCartCache__FindNextLowestCacheEntry(CCartCache *pThis, CCacheEntry *pLastLowest);
void 				CCartCache__PrintMemory(CCartCache *pThis);
BOOL 				CCartCache__DiscardCacheEntry(CCartCache *pThis);
void* 			CCartCache__Alloc(CCartCache *pThis, DWORD Size, BOOL AllocTop);
void				CCartCache__MemoryDump(CCartCache *pThis);
void BrokenUpDMA(void *pRAM, ROMAddress *rpROM, DWORD Length);

/////////////////////////////////////////////////////////////////////////////
// CCacheEntry

void* CCacheEntry__GetData(CCacheEntry *pThis)
{

#ifndef WIN32

	// extra debugging info
#ifndef SHIP_IT
	if (pThis->m_Allocated != CACHEENTRY_ALLOCATED)
		igAssert(__FILE__, __LINE__, pThis->m_szDescription);
#endif

#endif

	ASSERT(pThis->m_Allocated == CACHEENTRY_ALLOCATED);
	ASSERT(!pThis->m_Compressed);

	CCacheEntry__ResetAge(pThis);

   return pThis->m_pData;
}

void*	CCacheEntry__DoAllocDataForReplacement(CCacheEntry *pThis, DWORD NewSize, BOOL AllocTop)
{
	void *pNewData;

	//pNewData = CCartCache__BestAlloc(NewSize);
	CCacheEntry__ResetAge(pThis);		// don't discard self
	pNewData = CCartCache__Alloc(pThis->m_pOwner, NewSize, AllocTop);

	if (pNewData)
	{
		TRACE2("CCacheEntry: Allocated replacement data 0x%x, len 0x%x.\r\n",
				 pNewData, NewSize);
	}
	else
	{
		TRACE2("CCacheEntry: WARNING - Failed to allocate replacement data for \"%s\", len: 0x%x.\r\n",
				 pThis->m_szDescription, NewSize);
		//ASSERT(FALSE);
	}

	return pNewData;
}

void CCacheEntry__DeallocAndReplaceData(CCacheEntry *pThis, void *pNewData, DWORD NewLength)
{
   ASSERT(pThis->m_pData);

#ifndef WIN32
	TRACE4("CCacheEntry: Replacing 0x%x, len 0x%x with 0x%x, len 0x%x.\r\n",
			 pThis->m_pData, pThis->m_DataLength,
			 pNewData, NewLength);
#endif

   DEALLOC(pThis->m_pData);
   pThis->m_pData = pNewData;
	pThis->m_DataLength = NewLength;
}

void CCacheEntry__DeallocData(CCacheEntry *pThis)
{
	if (pThis->m_pData)
		DEALLOC(pThis->m_pData);
}

// version that can be used as notify callback
void CCacheEntry__DecompressCallback(void *pVoid, CCacheEntry **ppceTarget)
{
	CCacheEntry__Decompress(*ppceTarget);
}

BOOL CCacheEntry__DoDecompress(CCacheEntry *pThis, BOOL AllocTop)
{
#ifdef WIN32

	ASSERT(FALSE);
	return FALSE;

#else

#ifdef IG_DEBUG
	OSTime			startTime, endTime, diffTime;
#endif
	DWORD				allocSize;
	void				*pCompressedData, *pBytes;
	BOOL				ret;
	BYTE				*headers;

	pCompressedData = CCacheEntry__GetData(pThis);

	headers = (BYTE*) pCompressedData;
	if (		(headers[0] == 'R')
			&& (headers[1] == 'N')
			&& (headers[2] == 'C') )
	{
#ifdef IG_DEBUG
		TRACE0("CCacheEntry: Starting decompression...\r\n");
		startTime = osGetTime();
#endif

		// allocate block for decompressed data
		allocSize = CMP_GetDecompressedSize(pCompressedData);
		pBytes = CCacheEntry__DoAllocDataForReplacement(pThis, allocSize, AllocTop);
		if (pBytes)
		{
			ret = CMP_DecompressData(pCompressedData, pBytes);
			ASSERT(ret);
		}

		CCacheEntry__DeallocAndReplaceData(pThis, pBytes, allocSize);

#ifdef IG_DEBUG
		endTime = osGetTime();
		diffTime = endTime - startTime;

		TRACE("...Finished cache entry decompression in %8.2f msec.\r\n",
				((float)(CYCLES_TO_NSEC(diffTime)))/1000000);
#endif

		return (pBytes != NULL);
	}
	else
	{
		return TRUE;
	}
#endif
}

/////////////////////////////////////////////////////////////////////////////
// CCartCache

void CCartCache__Construct(CCartCache *pThis)
{
	int i;

	TRACE0("CCartCache construction\r\n");

   pThis->m_DiscardAge = 2;
   pThis->m_pNotifyHead = pThis->m_pNotifyTail = NULL;
   pThis->m_pCacheHead = pThis->m_pCacheTail = NULL;


	// initialize lists of entries in cache
	for (i=1; i<CARTCACHE_CACHEENTRY_COUNT; i++)
		pThis->m_CacheEntries[i-1].m_pNextFree = &pThis->m_CacheEntries[i];

	pThis->m_CacheEntries[CARTCACHE_CACHEENTRY_COUNT-1].m_pNextFree = NULL;
	pThis->m_pCacheFreeHead = &pThis->m_CacheEntries[0];

	for (i=0; i<CARTCACHE_CACHEENTRY_COUNT; i++)
		pThis->m_CacheEntries[i].m_Allocated = CACHEENTRY_UNALLOCATED;

	for (i=1; i<CARTCACHE_NOTIFYENTRY_COUNT; i++)
		pThis->m_NotifyEntries[i-1].m_pNextFree = &pThis->m_NotifyEntries[i];

	pThis->m_NotifyEntries[CARTCACHE_NOTIFYENTRY_COUNT-1].m_pNextFree = NULL;
	pThis->m_pNotifyFreeHead = &pThis->m_NotifyEntries[0];

	CHashTable__Construct(&pThis->m_HashTable, HASH_ALLOC_BLOCK);
	CHashTable__InitHashTable(&pThis->m_HashTable, HASH_TABLE_SIZE, TRUE);

	TRACE0("CCartCache construction DONE\r\n");
}

void CCartCache__Destruct(CCartCache *pThis)
{
	CCartCache__Empty(pThis);
}

void CCartCache__KeepAroundAnotherFrame(CCartCache *pThis)
{
	CCacheEntry *pEntry;

	pEntry = pThis->m_pCacheHead;
	while (pEntry)
	{
		pEntry->m_LastAccessed++;

		pEntry = pEntry->m_pNext;
	}
}

void CCartCache__ResetAge(CCartCache *pThis, CCacheEntry *pCacheEntry)
{
#ifndef WIN32
	ASSERT(osGetThreadId(NULL) == THREAD_MAIN);
#endif

	ASSERT(pCacheEntry);
	ASSERT(pCacheEntry->m_pOwner == pThis);

	pCacheEntry->m_LastAccessed = frame_number;

	// remove from linked list
   if (pThis->m_pCacheHead == pCacheEntry)
      pThis->m_pCacheHead = pCacheEntry->m_pNext;
	if (pThis->m_pCacheTail == pCacheEntry)
		pThis->m_pCacheTail = pCacheEntry->m_pLast;
   if (pCacheEntry->m_pLast)
      pCacheEntry->m_pLast->m_pNext = pCacheEntry->m_pNext;
   if (pCacheEntry->m_pNext)
      pCacheEntry->m_pNext->m_pLast = pCacheEntry->m_pLast;

	// add to head of list
   if (pThis->m_pCacheHead)
      pThis->m_pCacheHead->m_pLast = pCacheEntry;
	if (!pThis->m_pCacheTail)
		pThis->m_pCacheTail = pCacheEntry;
   pCacheEntry->m_pLast = NULL;
   pCacheEntry->m_pNext = pThis->m_pCacheHead;
   pThis->m_pCacheHead 	= pCacheEntry;
}

BOOL CCartCache__DiscardCacheEntry(CCartCache *pThis)
{
	//CCartCache__PrintMemory(pThis);

	if (!pThis->m_pCacheTail)
	{
		TRACE0("Cache: WARNING - Tried to discard entry with no entries left.\r\n");
		return FALSE;
	}

	// +1 allows for time for display processor
	if ((pThis->m_pCacheTail->m_LastAccessed + 1) < frame_number)
	{
		CCartCache__DeleteCacheEntry(pThis, pThis->m_pCacheTail);
		return TRUE;
	}
	else
	{
		TRACE0("Cache: Out of discardable entries.\r\n");
		return FALSE;
	}
}

void CCartCache__MemoryDump(CCartCache *pThis)
{
#ifdef CACHE_DO_MEMORY_DUMP
	CCacheEntry *pEntry;
	DWORD			workingSet, totalUsed,
					cacheEntries;
	int			total, largest;

	rmonPrintf("\n\nCCartCache: Start memory dump...\n\n");
	rmonPrintf("       Age        Size   Description\n");
	rmonPrintf("----------  ----------   --------------------------------------------------\n");

	workingSet = 0;
	totalUsed = 0;
	cacheEntries = 0;
	pEntry = pThis->m_pCacheHead;
	while (pEntry)
	{
		cacheEntries++;
		totalUsed += pEntry->m_DataLength;
		if ((pEntry->m_LastAccessed + 1) >= frame_number)
			workingSet += pEntry->m_DataLength;

		rmonPrintf("%10d  %10d   %s\n",
					  frame_number - pEntry->m_LastAccessed,
					  pEntry->m_DataLength,
					  pEntry->m_szDescription);

		pEntry = pEntry->m_pNext;
	}

	rmonPrintf("\n...Finished memory dump\n\n");
	rmonPrintf("working set:  %10d\n", workingSet);
	rmonPrintf("in cache:     %10d\n", totalUsed);
	rmonPrintf("cache size:   %10d\n", MEMORY_POOL_SIZE);

	FreeMem(&total, &largest);
	rmonPrintf("largest free: %10d\n\n", largest);

	rmonPrintf("cache entries in use: %d of %d\n\n", cacheEntries, CARTCACHE_CACHEENTRY_COUNT);
#endif
}

void* CCartCache__Alloc(CCartCache *pThis, DWORD Size, BOOL AllocTop)
{
	void *pData;

	if (AllocTop)
		pData = ALLOCTOP(Size);
	else
		pData = ALLOC(Size);

	if (pData)
	{
		return pData;
	}
	else
	{
		TRACE("Cache: Could not allocate memory, len 0x%x.  Discarding entries.\r\n", Size);

		do
		{
			if (!CCartCache__DiscardCacheEntry(pThis))
			{
				TRACE("Cache: WARNING - Could not allocate memory for %s, len 0x%x.\r\n", Size);
				CCartCache__MemoryDump(pThis);
				//ASSERT(FALSE);

				return NULL;
			}

			if (AllocTop)
				pData = ALLOCTOP(Size);
			else
				pData = ALLOC(Size);

		} while (!pData);

		return pData;
	}
}

CCacheEntry* CCartCache__AllocCacheEntry(CCartCache *pThis)
{
/*
	CCacheEntry *pEntry;

	if (pThis->m_pCacheFreeHead)
	{
		pEntry = pThis->m_pCacheFreeHead;

		pThis->m_pCacheFreeHead = pThis->m_pCacheFreeHead->m_pNextFree;

		ASSERT(pEntry->m_Allocated == CACHEENTRY_UNALLOCATED);
		pEntry->m_Allocated = CACHEENTRY_ALLOCATED;
		pEntry->m_pOwner = pThis;
		pEntry->m_LastAccessed = frame_number;

#ifdef TRACE_MEM
		TRACE("Cache: Allocated cache entry: %x.\r\n", pEntry);
#endif
		return pEntry;
	}
	else
	{
		TRACE0("Cache: WARNING - Could not allocate cache entry.\r\n");
		ASSERT(FALSE);
		return NULL;
	}
*/
	CCacheEntry *pEntry;

	if (!pThis->m_pCacheFreeHead)
	{
		if (CCartCache__DiscardCacheEntry(pThis))
		{
			ASSERT(pThis->m_pCacheFreeHead);
			TRACE0("Cache: WARNING - Had to discard a cache entry.  Increase CARTCACHE_CACHEENTRY_COUNT\r\n");
		}
		else
		{
			TRACE0("Cache: WARNING - Could not allocate cache entry.\r\n");
			//ASSERT(FALSE);
			return NULL;
		}
	}

	pEntry = pThis->m_pCacheFreeHead;

	pThis->m_pCacheFreeHead = pThis->m_pCacheFreeHead->m_pNextFree;

	ASSERT(pEntry->m_Allocated == CACHEENTRY_UNALLOCATED);
	pEntry->m_Allocated = CACHEENTRY_ALLOCATED;
	pEntry->m_pOwner = pThis;
	pEntry->m_LastAccessed = frame_number;

#ifdef TRACE_MEM
	TRACE("Cache: Allocated cache entry: 0x%x.\r\n", pEntry);
#endif
	return pEntry;
}

void CCartCache__DeallocCacheEntry(CCartCache *pThis, CCacheEntry *pEntry)
{
	ASSERT(pEntry);

	ASSERT(pEntry->m_Allocated == CACHEENTRY_ALLOCATED);
	pEntry->m_Allocated = CACHEENTRY_UNALLOCATED;

	pEntry->m_pNextFree = pThis->m_pCacheFreeHead;
	pThis->m_pCacheFreeHead = pEntry;

#ifdef TRACE_MEM
	TRACE("Cache: Deallocated cache entry: 0x%x.\r\n", pEntry);
#endif
}

CNotifyEntry* CCartCache__AllocNotifyEntry(CCartCache *pThis)
{
	CNotifyEntry *pEntry;

	if (pThis->m_pNotifyFreeHead)
	{
		pEntry = pThis->m_pNotifyFreeHead;

		pThis->m_pNotifyFreeHead = pThis->m_pNotifyFreeHead->m_pNextFree;

#ifdef TRACE_MEM
		TRACE("Cache: Allocated notify entry: 0x%x.\r\n", pEntry);
#endif
		return pEntry;
	}
	else
	{
		TRACE0("Cache: WARNING - Could not allocate notify entry.\r\n");
		//ASSERT(FALSE);
		return NULL;
	}
}

void CCartCache__DeallocNotifyEntry(CCartCache *pThis, CNotifyEntry *pEntry)
{
	ASSERT(pEntry);

	pEntry->m_pNextFree = pThis->m_pNotifyFreeHead;
	pThis->m_pNotifyFreeHead = pEntry;

#ifdef TRACE_MEM
	TRACE("Cache: Deallocated notify entry: 0x%x.\r\n", pEntry);
#endif
}

BOOL CCartCache__InCache(CCartCache *pThis, ROMAddress *rpAddress)
{
   CCacheEntry		*pCacheEntry = NULL;	// avoids silly warning

	return CHashTable__Lookup(&pThis->m_HashTable, rpAddress, (void**) &pCacheEntry);
}

void CCartCache__RequestBlock(CCartCache *pThis,
										void *pNotifyPtr,
                              pfnCACHENOTIFY pfnDecompress,
                              pfnCACHENOTIFY pfnBlockReceived,
                              CCacheEntry **ppceTarget,
                              ROMAddress *rpRequestAddress, DWORD rLength,
                              const char *szDescription /*=""*/)
{
	BOOL 				found,
						retrieve,
						alreadyRetreiving;
   CCacheEntry		*pCacheEntry;
	CNotifyEntry 	*pNotify,
						*pSearchNotify;
	void				*pData;

#ifndef WIN32
	ASSERT(osGetThreadId(NULL) == THREAD_MAIN);
#endif

   ASSERT(rpRequestAddress);
   ASSERT(ppceTarget);
   ASSERT(rLength);

	pCacheEntry = NULL;	// not used, just supresses warning;
	found = CHashTable__Lookup(&pThis->m_HashTable, rpRequestAddress, (void**) &pCacheEntry);

   if (found)
      retrieve = (pCacheEntry->m_rLength < rLength); // retrieve if requested length is larger
   else
      retrieve = TRUE;

   if (retrieve)
   {
      // DON'T clear target address pointer!  This way it can continue using
		// old data until new data comes back
      //*ppceTarget = NULL;

		// ROM address and length should be 2 byte aligned for DMA operation
		ASSERT(!(((DWORD)rpRequestAddress) & 0x1));
		if (rLength & 0x1)
			rLength++;
		ASSERT(!(rLength & 0x1));

		pNotify = CCartCache__AllocNotifyEntry(pThis);

		if (pNotify)
		{
			// don't need to request block from ROM if valid request
			// has already been made
			alreadyRetreiving = FALSE;
			pSearchNotify = pThis->m_pNotifyHead;
			while (pSearchNotify)
			{
				if ((pSearchNotify->m_rpRequestedAddress == rpRequestAddress) && (pSearchNotify->m_rLength >= rLength))
				{
					alreadyRetreiving = TRUE;
					break;
				}

				pSearchNotify = pSearchNotify->m_pNext;
			}

			if (!alreadyRetreiving)
			{
				pData = CCartCache__Alloc(pThis, rLength, pfnDecompress != NULL);
				if (pData)
				{
					// RDRAM address should be 8 byte aligned
					ASSERT(!(((DWORD)pData) & 0x7));
				}
				else
				{
					CCartCache__DeallocNotifyEntry(pThis, pNotify);
				}
			}

			if (pData || alreadyRetreiving)
			{
				pNotify->m_pNotifyPtr         = pNotifyPtr;
				pNotify->m_pfnDecompress      = pfnDecompress;
				pNotify->m_pfnBlockReceived   = pfnBlockReceived;
				pNotify->m_ppceTarget         = ppceTarget;
				pNotify->m_rpRequestedAddress = rpRequestAddress;
				pNotify->m_rLength            = rLength;
				pNotify->m_szDescription      = szDescription;

				// insert at tail of notify list
				if (pThis->m_pNotifyTail)
					pThis->m_pNotifyTail->m_pNext = pNotify;
				else
					pThis->m_pNotifyHead  = pNotify;
				pNotify->m_pNext  	= NULL;
				pNotify->m_pLast  	= pThis->m_pNotifyTail;
				pThis->m_pNotifyTail	= pNotify;
			}

			if (pData && (!alreadyRetreiving))
			{
      		TRACE3("Cache: Request block \"%s\" at 0x%x, len 0x%x.\r\n",
						 szDescription,
						 rpRequestAddress,
						 rLength);

#ifdef WIN32
				// FOR TESTING, I'M JUST COPYING DATA AND CALLING BlockReceived()
				memcpy(pData, (void*) ((DWORD) rpRequestAddress & ~CACHE_BLOCK_VERSION_MASK), rLength);
#else
				// FOR NOW, REQUEST IS SYNCHRONOUS
				BrokenUpDMA(pData,
								(ROMAddress*) ((DWORD) rpRequestAddress & ~CACHE_BLOCK_VERSION_MASK),
								rLength);
/*
				//rmonPrintf("----  rpRequestAddress:%x, rLength:%x, pData:%x\r\n", rpRequestAddress, rLength, pData);

				//memset(pData, 0, rLength);

				osWritebackDCache(pData, rLength);
				osInvalDCache(dynamic_memory_pool, MEMORY_POOL_SIZE);

				osPiStartDma(&dmaIOMessageBuf, OS_MESG_PRI_NORMAL, OS_READ,
								 (DWORD) rpRequestAddress & ~CACHE_BLOCK_VERSION_MASK, pData, rLength,
								 &dmaMessageQ);

				// wait for DMA to finish
				osRecvMesg(&dmaMessageQ, NULL, OS_MESG_BLOCK);
				osInvalDCache(pData, rLength);
*/
//				romcpy(pData, (DWORD) rpRequestAddress & ~CACHE_BLOCK_VERSION_MASK, rLength);
#endif
				CCartCache__BlockReceived(pThis, rpRequestAddress, rLength, pData);
			}
		}
   }
   else
   {
      // ??
      //pCacheEntry->ResetAge();

      *ppceTarget = pCacheEntry;
      if (pfnBlockReceived)
         pfnBlockReceived(pNotifyPtr, ppceTarget);
   }
}

// /*
#ifndef WIN32
void BrokenUpDMA(void *pRAM, ROMAddress *rpROM, DWORD Length)
{
	DWORD transferLength,
			remainLength;

#define MAX_DMA_SIZE		8192

	osWritebackDCache(pRAM, Length);
	osInvalDCache(dynamic_memory_pool, MEMORY_POOL_SIZE);

	remainLength = Length;
	while (remainLength)
	{
		transferLength = min(MAX_DMA_SIZE, remainLength);

		osPiStartDma(&dmaIOMessageBuf, OS_MESG_PRI_NORMAL, OS_READ,
						 (DWORD)rpROM + Length - remainLength,
						 (void*) ((DWORD) pRAM + Length - remainLength),
						 transferLength,
						 &dmaMessageQ);

		// wait for DMA to finish
		osRecvMesg(&dmaMessageQ, NULL, OS_MESG_BLOCK);

		remainLength -= transferLength;
	}

	osInvalDCache(pRAM, Length);
}
#endif
// */

void CCartCache__BlockReceived(CCartCache *pThis,
										 ROMAddress *rpRequestedAddress, DWORD rLength, void *pData)
{
	CCacheEntry 	*pCacheEntry,
      				*pOldCacheEntry,
						*prevPceTarget;
	BOOL				decompressed;

	CNotifyEntry	*pNotify,
						*pDeleteEntry;
	void 				*pNotifyPtr;
	pfnCACHENOTIFY pfnDecompress;
	pfnCACHENOTIFY	pfnBlockReceived;

	CCacheEntry **ppceTarget;

	pCacheEntry = CCartCache__AllocCacheEntry(pThis);
   if (pCacheEntry)
   {
      // is there another one already in cache?

		pOldCacheEntry = NULL;	// not used, just supresses warning;
		if (CHashTable__Lookup(&pThis->m_HashTable, rpRequestedAddress, (void**) &pOldCacheEntry))
      {
         // should only happen if old version has smaller length
         ASSERT(pOldCacheEntry->m_rLength < rLength);

         // remove from hash table but not linked list (it must age out first)
         CHashTable__RemoveKey(&pThis->m_HashTable, pOldCacheEntry->m_rpAddress);
         pOldCacheEntry->m_InHashTable = FALSE;
      }

      pCacheEntry->m_rpAddress   = rpRequestedAddress;
      pCacheEntry->m_pData       = pData;
      pCacheEntry->m_rLength     = rLength;
		pCacheEntry->m_DataLength	= rLength;
      pCacheEntry->m_InHashTable = TRUE;
		pCacheEntry->m_Compressed	= FALSE;
		//CCacheEntry__ResetAge(pCacheEntry);

      // add to cache hash table
      CHashTable__SetAt(&pThis->m_HashTable, rpRequestedAddress, pCacheEntry);

      // add to cache linked list
      if (pThis->m_pCacheHead)
         pThis->m_pCacheHead->m_pLast = pCacheEntry;
		if (!pThis->m_pCacheTail)
			pThis->m_pCacheTail = pCacheEntry;
      pCacheEntry->m_pLast = NULL;
      pCacheEntry->m_pNext = pThis->m_pCacheHead;
      pThis->m_pCacheHead 	= pCacheEntry;

      TRACE2("Cache: Block at 0x%x, len 0x%x added to cache.\r\n",
             pCacheEntry->m_rpAddress,
             pCacheEntry->m_rLength);

      decompressed = FALSE;
      pNotify = pThis->m_pNotifyHead;
      while (pNotify)
      {
         if ((pNotify->m_rpRequestedAddress == rpRequestedAddress) && (pNotify->m_rLength <= rLength))
         {
            pCacheEntry->m_szDescription = pNotify->m_szDescription;

            // remove from linked list before sending notification messages
            ppceTarget 			= pNotify->m_ppceTarget;

            pNotifyPtr        = pNotify->m_pNotifyPtr;
            pfnDecompress     = pNotify->m_pfnDecompress;
            pfnBlockReceived	= pNotify->m_pfnBlockReceived;

            pDeleteEntry = pNotify;

            // move on to next entry
            pNotify = pNotify->m_pNext;

            CCartCache__DeleteNotifyEntry(pThis, pDeleteEntry);


				// assign to target address and save previous value
				prevPceTarget = *ppceTarget;
            *ppceTarget = pCacheEntry;

            // send messages
            if (pCacheEntry->m_pData)
            {
               if ((!decompressed) && pfnDecompress)
					{
						pfnDecompress(pNotifyPtr, ppceTarget);
						decompressed = TRUE;
					}

					if (pCacheEntry->m_pData)
					{
						if (pfnBlockReceived)
							pfnBlockReceived(pNotifyPtr, ppceTarget);
					}
					else
					{
						// restore previous value of *ppceTarget
						*ppceTarget = prevPceTarget;

						// remove from hash table but not linked list
						CHashTable__RemoveKey(&pThis->m_HashTable, pCacheEntry->m_rpAddress);
						pCacheEntry->m_InHashTable = FALSE;
					}

            }
         }
         else
         {
            // move on to next entry
            pNotify = pNotify->m_pNext;
         }
      }

      // memory allocation failed during decompression
      if (!pCacheEntry->m_pData)
         CCartCache__DeleteCacheEntry(pThis, pCacheEntry);
   }
	else
	{
      pNotify = pThis->m_pNotifyHead;
      while (pNotify)
      {
         if ((pNotify->m_rpRequestedAddress == rpRequestedAddress) && (pNotify->m_rLength <= rLength))
         {
            pDeleteEntry = pNotify;

            // move on to next entry
            pNotify = pNotify->m_pNext;

            CCartCache__DeleteNotifyEntry(pThis, pDeleteEntry);
			}
		}
	}
}

/*
void CCartCache__AgeEntries(CCartCache *pThis)
{
	CCacheEntry *pCacheEntry,
					*pDeleteEntry;

	// TEST
	//return;

	pCacheEntry = pThis->m_pCacheHead;
   while (pCacheEntry)
   {
		// delete entry if reached discard age, or age == 2 and block has been compressed
		// TEST
		if ((pCacheEntry->m_Age == pThis->m_DiscardAge) || ((pCacheEntry->m_Age == 2) && pCacheEntry->m_Compressed))
		//if ((pCacheEntry->m_Age == pThis->m_DiscardAge) && (!pCacheEntry->m_Compressed))
      {

         pDeleteEntry = pCacheEntry;

         // move on to next entry
         pCacheEntry = pCacheEntry->m_pNext;

         CCartCache__DeleteCacheEntry(pThis, pDeleteEntry);
      }
      else
      {
         pCacheEntry->m_Age++;

         // move on to next entry
         pCacheEntry = pCacheEntry->m_pNext;
      }
   }
}
*/

void CCartCache__DeleteNotifyEntry(CCartCache *pThis, CNotifyEntry *pNotify)
{
   // remove from notify linked list
   if (pThis->m_pNotifyHead == pNotify)
      pThis->m_pNotifyHead = pNotify->m_pNext;
   if (pThis->m_pNotifyTail == pNotify)
      pThis->m_pNotifyTail = pNotify->m_pLast;
   if (pNotify->m_pLast)
      pNotify->m_pLast->m_pNext = pNotify->m_pNext;
   if (pNotify->m_pNext)
      pNotify->m_pNext->m_pLast = pNotify->m_pLast;

   // deallocate
	CCartCache__DeallocNotifyEntry(pThis, pNotify);
}

void CCartCache__DeleteCacheEntry(CCartCache *pThis, CCacheEntry *pCacheEntry)
{
   ASSERT(pCacheEntry);

   TRACE3("Cache: Block \"%s\" at 0x%x, len %x removed from cache.\r\n",
          pCacheEntry->m_szDescription,
          pCacheEntry->m_rpAddress,
          pCacheEntry->m_DataLength);

   // remove from cache hash table
   if (pCacheEntry->m_InHashTable)
      CHashTable__RemoveKey(&pThis->m_HashTable, pCacheEntry->m_rpAddress);

   // remove from cache linked list
   if (pThis->m_pCacheHead == pCacheEntry)
      pThis->m_pCacheHead = pCacheEntry->m_pNext;
	if (pThis->m_pCacheTail == pCacheEntry)
		pThis->m_pCacheTail = pCacheEntry->m_pLast;
   if (pCacheEntry->m_pLast)
      pCacheEntry->m_pLast->m_pNext = pCacheEntry->m_pNext;
   if (pCacheEntry->m_pNext)
      pCacheEntry->m_pNext->m_pLast = pCacheEntry->m_pLast;

   // deallocate
   CCacheEntry__DeallocData(pCacheEntry);
	CCartCache__DeallocCacheEntry(pThis, pCacheEntry);
}

void CCartCache__Empty(CCartCache *pThis)
{
	int 				nEntries;
	CCacheEntry 	*pCacheEntry,
						*pDeleteEntry;
	CNotifyEntry	*pNotify,
						*pDeleteNotify;

   TRACE0("Cache: Cleaning up...\r\n");

   nEntries = 0;

   pCacheEntry = pThis->m_pCacheHead;
   while (pCacheEntry)
   {
      TRACE3("Cache: Block \"%s\" at 0x%x, len 0x%x removed from cache.\r\n",
             pCacheEntry->m_szDescription,
             pCacheEntry->m_rpAddress,
             pCacheEntry->m_DataLength);

      pDeleteEntry = pCacheEntry;

      // move on to next entry
      pCacheEntry = pCacheEntry->m_pNext;

      // deallocate
      CCacheEntry__DeallocData(pDeleteEntry);
		CCartCache__DeallocCacheEntry(pThis, pDeleteEntry);

      nEntries++;
   }
   pThis->m_pCacheHead = pThis->m_pCacheTail = NULL;

   pNotify = pThis->m_pNotifyHead;
   while (pNotify)
   {
      pDeleteNotify = pNotify;

      // move on to next entry
      pNotify = pNotify->m_pNext;

      // deallocate
		CCartCache__DeallocNotifyEntry(pThis, pDeleteNotify);
   }
   pThis->m_pNotifyHead = pThis->m_pNotifyTail = NULL;

   CHashTable__RemoveAll(&pThis->m_HashTable);

   TRACE("Cache: There were %d entries in the cache.\r\n", nEntries);
}

CCacheEntry* CCartCache__FindNextLowestCacheEntry(CCartCache *pThis, CCacheEntry *pLastLowest)
{
	CCacheEntry 	*pCacheEntry,
						*pLowest;

	pLowest = NULL;

	pCacheEntry = pThis->m_pCacheHead;
   while (pCacheEntry)
   {
		if (pLowest)
		{
			if (pLastLowest)
			{
				if ( (((DWORD)pCacheEntry->m_pData) > ((DWORD)pLastLowest->m_pData))
			  		&& (((DWORD)pCacheEntry->m_pData) < ((DWORD)pLowest->m_pData))  )
				{
					pLowest = pCacheEntry;
				}
			}
			else
			{
				if (((DWORD)pCacheEntry->m_pData) < ((DWORD)pLowest->m_pData))
				{
					pLowest = pCacheEntry;
				}
			}
		}
		else
		{
			if (pLastLowest)
			{
				if ( ((DWORD)pCacheEntry->m_pData) > ((DWORD)pLastLowest->m_pData))
				{
					pLowest = pCacheEntry;
				}
			}
			else
			{
				pLowest = pCacheEntry;
			}
		}

		// try next entry
      pCacheEntry = pCacheEntry->m_pNext;
	}

	return pLowest;
}

void CCartCache__PrintMemory(CCartCache *pThis)
{
/*
	CCacheEntry *pLastLowest;

	pLastLowest = NULL;

	TRACE0("\r\n-----------------------\r\n");
	PrintLowestFree();
	TRACE0("\r\n-----------------------\r\n");

	do
	{
		pLastLowest = CCartCache__FindNextLowestCacheEntry(pThis, pLastLowest);

		if (pLastLowest)
		{
			TRACE5("addr: %s rAddr:%x addr: %x, len: %x %s\r\n",
					 BlockBelowFreeChunk(pLastLowest->m_pData) ? "B " : (pLastLowest->m_Compressed ? " C" : "  "),
					 pLastLowest->m_rpAddress,
					 pLastLowest->m_pData,
					 pLastLowest->m_DataLength,
					 pLastLowest->m_szDescription);
		}

	} while (pLastLowest);

	TRACE0("\r\n-----------------------\r\n");
*/
	CCacheEntry *pEntry = pThis->m_pCacheHead;
	TRACE0("\r\n-----------------------\r\n");
	while (pEntry)
	{
#ifndef WIN32
		TRACE5("rAddr:0x%x, addr:0x%x, len:0x%x, access:%d - \"%s\"\r\n",
				 pEntry->m_rpAddress,
				 pEntry->m_pData,
				 pEntry->m_DataLength,
				 pEntry->m_LastAccessed,
				 pEntry->m_szDescription);
#endif

		pEntry = pEntry->m_pNext;
	}
	TRACE0("\r\n-----------------------\r\n");
}

#if 0
/*
BOOL CCartCache__Compress(CCartCache *pThis)
{
	CCacheEntry		*pCacheEntry,
						*pOldEntry,
						*pNextFree;
	void				*pNewData;

#ifdef CARTCACHE_SHOW_COMPRESSION
 	CCartCache__PrintMemory(pThis);
#endif

	// get last cache entry
	pCacheEntry = pThis->m_pCacheHead;
	if (pCacheEntry)
	{
		while (pCacheEntry->m_pNext)
			pCacheEntry = pCacheEntry->m_pNext;

		if (!pCacheEntry->m_Compressed)
		{
			TRACE0("COMPRESSING\r\n");

			if (RandomDWORD() % 2)
				pNewData = ALLOC(pCacheEntry->m_DataLength);
			else
				pNewData = ALLOCTOP(pCacheEntry->m_DataLength);
			if (pNewData)
			{
				BlockBelowFreeChunk(pNewData);
				if (TRUE)
				{
					pOldEntry = CCartCache__AllocCacheEntry(pThis);
					if (pOldEntry)
					{
						// copy attributes for older version
						// maintaining position in free list
						pNextFree = pOldEntry->m_pNextFree;
						*pOldEntry = *pCacheEntry;
						pOldEntry->m_pNextFree = pNextFree;

      				// add to cache linked list
      				if (pThis->m_pCacheHead)
         				pThis->m_pCacheHead->m_pLast = pOldEntry;
						if (!pThis->m_pCacheTail)
							pThis->m_pCacheTail = pOldEntry;
      				pOldEntry->m_pLast = NULL;
      				pOldEntry->m_pNext = pThis->m_pCacheHead;
      				pThis->m_pCacheHead 	= pOldEntry;

						// old version is not in hash table
						pOldEntry->m_InHashTable = FALSE;

						// don't compress old version again
						pOldEntry->m_Compressed = TRUE;

						// copy data to lower spot
						memcpy(pNewData, pCacheEntry->m_pData, pCacheEntry->m_DataLength);
						pCacheEntry->m_pData = pNewData;

#ifdef CARTCACHE_SHOW_COMPRESSION
      				TRACE6("* Cache: \"%s\" %s at %x, len %x from %x to %x.\r\n",
					 			 pCacheEntry->m_szDescription,
						 		 "\\|/  Lowered",
						 		 pCacheEntry->m_rpAddress,
						 		 pCacheEntry->m_DataLength,
						 		 pOldEntry->m_pData,
						 		 pCacheEntry->m_pData);
#endif

						// done moving block; continue compressing
						return TRUE;
					}
					else
					{
						DEALLOC(pNewData);
						TRACE0("Cache: WARNING - Compress() could not allocate cache entry.\r\n");

						// out of cache entries; stop compressing
						return FALSE;
					}
				}
				else
				{
					DEALLOC(pNewData);
				}
			}
		}

		return TRUE;
	}

	return FALSE;
}
*/
///*
BOOL CCartCache__Compress(CCartCache *pThis)
{
	CCacheEntry		*pCacheEntry,
						*pOldEntry,
						*pNextFree,
						*pLookup;
	void				*pNewData;
	BOOL				found;

#ifdef CARTCACHE_SHOW_COMPRESSION
 	CCartCache__PrintMemory(pThis);
#endif

	// first, try to move blocks down
	pCacheEntry = pThis->m_pCacheHead;
	while (pCacheEntry)
	{
		if (!pCacheEntry->m_Compressed && pCacheEntry->m_InHashTable && !BlockBelowFreeChunk(pCacheEntry->m_pData))
		{
			//TRACE(" #### : %x\r\n", pCacheEntry->m_pData);

			// try to alloc on bottom of heap
			pNewData = ALLOC(pCacheEntry->m_DataLength);
			if (pNewData)
			{
				if (BlockBelowFreeChunk(pNewData))
				{
					pOldEntry = CCartCache__AllocCacheEntry(pThis);
					if (pOldEntry)
					{
						ASSERT(!pCacheEntry->m_Compressed);
						pLookup = NULL;
						found = CHashTable__Lookup(&pThis->m_HashTable, pCacheEntry->m_rpAddress, (void**) &pLookup);
						ASSERT(found);
						ASSERT(pLookup == pCacheEntry);

						// copy attributes for older version
						// maintaining position in free list
						pNextFree = pOldEntry->m_pNextFree;
						*pOldEntry = *pCacheEntry;
						pOldEntry->m_pNextFree = pNextFree;

      				// add to cache linked list
      				if (pThis->m_pCacheHead)
         				pThis->m_pCacheHead->m_pLast = pOldEntry;
						if (!pThis->m_pCacheTail)
							pThis->m_pCacheTail = pOldEntry;
      				pOldEntry->m_pLast = NULL;
      				pOldEntry->m_pNext = pThis->m_pCacheHead;
      				pThis->m_pCacheHead 	= pOldEntry;

						// old version is not in hash table
						pOldEntry->m_InHashTable = FALSE;

						// don't compress old version again
						pOldEntry->m_Compressed = TRUE;

						// copy data to lower spot
						memcpy(pNewData, pCacheEntry->m_pData, pCacheEntry->m_DataLength);
						pCacheEntry->m_pData = pNewData;

#ifdef CARTCACHE_SHOW_COMPRESSION
      				TRACE6("* Cache: \"%s\" %s at 0x%x, len 0x%x from 0x%x to 0x%x.\r\n",
					 			 pCacheEntry->m_szDescription,
						 		 "\\|/  Lowered",
						 		 pCacheEntry->m_rpAddress,
						 		 pCacheEntry->m_DataLength,
						 		 pOldEntry->m_pData,
						 		 pCacheEntry->m_pData);
#endif

						pLookup = NULL;
						ASSERT(!pCacheEntry->m_Compressed);
						found = CHashTable__Lookup(&pThis->m_HashTable, pCacheEntry->m_rpAddress, (void**) &pLookup);
						ASSERT(found);
						ASSERT(pLookup == pCacheEntry);

						// done moving block; continue compressing
						return TRUE;
					}
					else
					{
						DEALLOC(pNewData);
						TRACE0("Cache: WARNING - Compress() could not allocate cache entry.\r\n");

						// out of cache entries; stop compressing
						return FALSE;
					}
				}
				else
				{
					DEALLOC(pNewData);
				}
			}
			else
			{
				TRACE("Cache: WARNING - Compress() could not allocate block, len:0x%x.\r\n",
						pCacheEntry->m_DataLength);

				// out of memory; stop compressing
				return FALSE;
			}
		}

		// try next entry
   	pCacheEntry = pCacheEntry->m_pNext;
	}

	// looks like we'll have to move a block up to make room
	pCacheEntry = NULL;

	while (TRUE)
	{
		pCacheEntry = CCartCache__FindNextLowestCacheEntry(pThis, pCacheEntry);

		if (pCacheEntry)
		{
			if (pCacheEntry->m_Compressed)
			{
				// wait until next frame before moving more blocks up
				return FALSE;
			}
			else if (!BlockBelowFreeChunk(pCacheEntry->m_pData))
			{
				// pCacheEntry is in the way, move it on up
				break;
			}
		}
		else
		{
			// memory is compressed
			return FALSE;
		}
	}

#ifdef CARTCACHE_SHOW_COMPRESSION
	TRACE2("lowest: \"%s\" m_pData: 0x%x, descr:0x%x\r\n",
			 pCacheEntry->m_szDescription,
			 pCacheEntry->m_pData);
#endif

	//return FALSE;

#if 0
	// TEST
	//return FALSE;
	pCacheEntry = pThis->m_pCacheHead;
	if (pCacheEntry)
	{
		while (pCacheEntry->m_pNext)
			pCacheEntry = pCacheEntry->m_pNext;
	}
	else
	{
		return FALSE;
	}
#endif

	//ASSERT(!pCacheEntry->m_Compressed);
	//ASSERT(!BlockBelowFreeChunk(pCacheEntry->m_pData));

	// allocate block at top of heap
	pNewData = ALLOCTOP(pCacheEntry->m_DataLength);
	if (pNewData)
	{
		// move block

		pOldEntry = CCartCache__AllocCacheEntry(pThis);
		if (pOldEntry)
		{
			ASSERT(!pCacheEntry->m_Compressed);
			pLookup = NULL;
			found = CHashTable__Lookup(&pThis->m_HashTable, pCacheEntry->m_rpAddress, (void**) &pLookup);
			ASSERT(found);
			ASSERT(pLookup == pCacheEntry);

			// copy attributes for older version
			// maintaining position in free list
			pNextFree = pOldEntry->m_pNextFree;
			*pOldEntry = *pCacheEntry;
			pOldEntry->m_pNextFree = pNextFree;

      	// add to cache linked list
      	if (pThis->m_pCacheHead)
         	pThis->m_pCacheHead->m_pLast = pOldEntry;
			if (!pThis->m_pCacheTail)
				pThis->m_pCacheTail = pOldEntry;
      	pOldEntry->m_pLast = NULL;
      	pOldEntry->m_pNext = pThis->m_pCacheHead;
      	pThis->m_pCacheHead 	= pOldEntry;

			// old version is not in hash table
			pOldEntry->m_InHashTable = FALSE;

			// don't compress old version again
			pOldEntry->m_Compressed = TRUE;

			// TEST
			//pOldEntry->m_Age = 5;

			// copy data to higher spot
			memcpy(pNewData, pCacheEntry->m_pData, pCacheEntry->m_DataLength);
			pCacheEntry->m_pData = pNewData;

#ifdef CARTCACHE_SHOW_COMPRESSION
      	TRACE6("* Cache: \"%s\" %s at 0x%x, len 0x%x from 0x%x to 0x%x.\r\n",
					 pCacheEntry->m_szDescription,
					 "/|\\  Raised",
					 pCacheEntry->m_rpAddress,
					 pCacheEntry->m_DataLength,
					 pOldEntry->m_pData,
					 pCacheEntry->m_pData);
#endif

			ASSERT(!pCacheEntry->m_Compressed);
			pLookup = NULL;
			found = CHashTable__Lookup(&pThis->m_HashTable, pCacheEntry->m_rpAddress, (void**) &pLookup);
			ASSERT(found);
			ASSERT(pLookup == pCacheEntry);

			// done moving block up; wait for next frame to continue decompressing
			return FALSE;
		}
		else
		{
			DEALLOC(pNewData);
			TRACE0("Cache: WARNING - Compress() could not allocate cache entry.\r\n");

			// out of cache entries; stop compressing
			return FALSE;
		}
	}
	else
	{
		TRACE("Cache: WARNING - Compress() could not allocate block, len:0x%x.\r\n",
				pCacheEntry->m_DataLength);

		// out of memory; stop compressing
		return FALSE;
	}
}
//*/
#endif

// Compression
/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32

// method 1 - for high performance compression and normal unpacking
// method 2 - for good compression and very fast unpacking
void CMP_CompressData(BOOL better, void *pData, DWORD Length, BYTE **ppOut, DWORD *pLength)
{
	ASSERT(ppOut);
	ASSERT(pLength);
	VERIFY(Length);

	BYTE *pBuffer = new BYTE[Length + 1000000];
	if (pBuffer)
	{
//		BOOL better = AfxGetApp()->GetProfileInt("Compression", "Better", FALSE);
//		AfxGetApp()->WriteProfileInt("Compression", "Better", better);

		*pLength = (DWORD) PackRNC((BYTE*) pData, pBuffer, Length, (BYTE) (better ? 1 : 2));
		if (*pLength > 0)
		{
			*ppOut = new BYTE[*pLength];
			if (ppOut)
				memcpy(*ppOut, pBuffer, *pLength);
			else
				*pLength = 0;
		}
		else if (*pLength == 0)
		{
			// failed to pack data
			*ppOut = new BYTE[Length];
			if (ppOut)
				memcpy(*ppOut, pData, Length);
			else
				*pLength = 0;
		}
		else
		{
			VERIFY(FALSE);
		}

		delete [] pBuffer;
	}
	else
	{
		*pLength = 0;
		*ppOut = NULL;
		MessageBeep(MB_ICONEXCLAMATION);
		AfxMessageBox("ERROR!\n\nOut of memory", MB_ICONEXCLAMATION | MB_OK);
	}
}

void CMP_DeallocData(BYTE *pData)
{
	if (pData)
		delete [] pData;
}

#endif

DWORD CMP_GetDecompressedSize(void *pCompressedData)
{
#ifdef WIN32
	ASSERT(FALSE);
	return 0;
#else
	ASSERT(pCompressedData);

	//return (DWORD) (BIGENDIANL(((RNC_fileptr)pCompressedData)->UncompressedSize));
#if defined(PLATFORM_PORT)
	/* The RNC unpacked-size field (DWORD at offset 4) is BIG-ENDIAN. On N64 the raw read below is
	 * correct; on a little-endian host it byte-swaps to a multi-GB bogus size → the decompress alloc
	 * fails → the cache entry's "received" callback never fires → the whole level-load streaming chain
	 * stalls (no instances/collision/grid → empty world). Swap to host order. */
	return __builtin_bswap32(((DWORD*)pCompressedData)[1]);
#else
	return ((DWORD*)pCompressedData)[1];
#endif
#endif
}

BOOL CMP_DecompressData(void *pCompressedData, void *pOutputBuffer)
{
#ifdef WIN32
	ASSERT(FALSE);
	return FALSE;
#else
	BYTE	*bytes = (BYTE*) pCompressedData;

	ASSERT(pCompressedData);
	ASSERT(pOutputBuffer);

//	return (UnpackRNC((RNC_fileptr)pCompressedData, pOutputBuffer) == RNCERROR_OK);

	// assembly version does not return anything
	//Propack_Unpack((RNC_fileptr)pCompressedData, pOutputBuffer);

	ASSERT((bytes[0] == 'R') || (bytes[1] == 'N') || (bytes[2] == 'C'));

	switch (bytes[3])
	{
		case 1:
			Propack_UnpackM1(pCompressedData, pOutputBuffer);
			break;

		case 2:
			Propack_UnpackM2(pCompressedData, pOutputBuffer);
			break;

		default:
			ASSERT(FALSE);
			break;
	}

	return TRUE;

#endif
}


// CFastISBuilder
/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32

CFastISBuilder::CFastISBuilder(int nBlocks)
{
	m_nBlocks = nBlocks;
	m_cBlock = 0;

	if (nBlocks)
	{
		m_Sizes = new DWORD[nBlocks];
		m_pBlocks = new void*[nBlocks];
	}
	else
	{
		m_Sizes = NULL;
		m_pBlocks = NULL;
	}
}

void CFastISBuilder::AddTo(CIndexedSet *pisDest)
{
	ASSERT(m_cBlock == m_nBlocks);

	if (m_Sizes && m_pBlocks)
		CIndexedSet__AddBlocks(pisDest, m_pBlocks, m_Sizes, m_nBlocks);
}

CFastISBuilder::~CFastISBuilder()
{
	if (m_Sizes)
		delete [] m_Sizes;
	if (m_pBlocks)
		delete [] m_pBlocks;
}

int CFastISBuilder::AddBlock(void *pBlock, DWORD Size)
{
	ASSERT(m_cBlock < m_nBlocks);

	m_pBlocks[m_cBlock] = pBlock;
	m_Sizes[m_cBlock] = Size;

	return m_cBlock++;
}

int CFastISBuilder::AddBlock(CIndexedSet *pisBlock)
{
   void     *pData;
   DWORD    dataSize;
   CIndexedSet__GetRawData(pisBlock, &pData, &dataSize);

	return AddBlock(pData, dataSize);
}

int CFastISBuilder::AddBlock(CUnindexedSet *pusBlock)
{
   void     *pData;
   DWORD    dataSize;
   CUnindexedSet__GetRawData(pusBlock, &pData, &dataSize);

	return AddBlock(pData, dataSize);
}

#endif
