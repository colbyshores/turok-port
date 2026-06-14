// defs.cpp

#include "cppu64.h"
#include "tengine.h"

// globals
int			trap_line;
const char	*trap_file = NULL;
const char	*trap_expr;
int			trap_thread = -1;

void igAssert(const char *szFile, int nLine, const char *szExpr)
{
#ifndef MAKE_CART
	rmonPrintf("\r\n\r\nASSERTION FAILED!  file:%s, line:%d\r\n", szFile, nLine);
	rmonPrintf("%s = 0\r\n\r\n", szExpr);
#endif

#ifdef TRAP_ON_ASSERT
	trap_line = nLine;
	trap_file = szFile;
	trap_expr = szExpr;

	*((char*)(NULL)) = 0;
#endif
}

void romcpy(const char *src, const char *dest, const int len)
{
#if 0
	//osSyncPrintf("2-1\n");
   ASSERT(MQ_IS_EMPTY(&romMessageQ));
	//rmonPrintf("empty:%d\n", MQ_IS_EMPTY(&romMessageQ));
	//osSyncPrintf("2-2\n");
	 
	osWritebackDCache((void*) dest, (s32) len);

	//osSyncPrintf("2-3\n");
	osInvalDCache((void*) dest, (s32) len);
   //osPiStartDma(&romIOMessageBuf, OS_MESG_PRI_NORMAL, OS_READ,
   osPiStartDma(&romIOMessageBuf, OS_MESG_PRI_HIGH, OS_READ,
                (u32)src, (void*) dest, (u32) len, &romMessageQ);
	//osSyncPrintf("2-4\n");

	//osRecvMesg(&dmaMessageQ, &dummyMesg, OS_MESG_BLOCK);
   while (osRecvMesg(&romMessageQ, NULL, OS_MESG_NOBLOCK) == -1);

	//osSyncPrintf("2-5\n");
	//while (MQ_IS_EMPTY(&romMessageQ));
	//osSyncPrintf("&romMessageQ:%x\n", &romMessageQ);
	//osSyncPrintf("2-6\n");
	//osRecvMesg(&romMessageQ, NULL, OS_MESG_BLOCK);
	//osSyncPrintf("2-7\n");

	osInvalDCache((void*) dest, (s32) len);
	//osSyncPrintf("2-8\n");


#else

    OSIoMesg dmaIoMesgBuf;
    OSMesg dummyMesg;
    
    osInvalDCache((void *)dest, (s32) len);
    osPiStartDma(&dmaIoMesgBuf, OS_MESG_PRI_NORMAL, OS_READ,
                 (u32)src, (void *)dest, (u32)len, &dmaMessageQ);
    (void) osRecvMesg(&dmaMessageQ, &dummyMesg, OS_MESG_BLOCK);


#endif
}


int BinarySearch(DWORD Keys[], int nItems, DWORD SearchKey)
{
	unsigned int	left, right, mid;
	
	if (nItems)
	{
		left = 0;
		right = ((unsigned int) nItems) - 1;

		while (left < right)
		{
			mid = (left + right) >> 1;
			
			if (Keys[mid] < SearchKey)
				left = mid + 1;
			else
				right = mid;
		}

		if (Keys[left] == SearchKey)
			return (int) left;
		else
			return -1;
	}
	else
	{
		return -1;
	}
}

BOOL BinaryRange(DWORD Keys[], int nItems, DWORD SearchKey, int *pFirst, int *pLast)
{
	int	pos, first, last;

	pos = BinarySearch(Keys, nItems, SearchKey);
	if (pos == -1)
		return FALSE;

	first = pos;
	while (first--)
		if (Keys[first] != SearchKey)
			break;
	*pFirst = first + 1;

	last = pos;
	while (++last < nItems)
		if (Keys[last] != SearchKey)
			break;
	*pLast = last - 1;

	return TRUE;
}

int GetBitStreamSize(int nBits)
{
	return (nBits + 7)/8;
}

BOOL GetBitStreamFlag(BYTE Bits[], int nBit)
{
	int	nByte, nByteBit;
	BYTE	mask;
	
	nByte		= nBit/8;
	nByteBit	= nBit%8;

	mask = 1 << nByteBit;

	return Bits[nByte] & mask;
}

void SetBitStreamFlag(BYTE Bits[], int nBit, BOOL Set)
{
	int	nByte, nByteBit;
	BYTE	mask;
	
	nByte		= nBit/8;
	nByteBit	= nBit%8;

	mask = 1 << nByteBit;

	if (Set)
		Bits[nByte] |= mask;
	else
		Bits[nByte] &= ~mask;
}
