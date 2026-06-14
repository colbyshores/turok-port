/*----------------------------------------------------------------------------
 * PROPACK compression/decompression 'C' header file
 *
 * Copyright (c) 1995 Rob Northen Computing, UK. All Rights Reserved.
 *
 * File: PP.H
 *
 * Date: 7-APR-95
 *----------------------------------------------------------------------------*/

#include <limits.h>

#define RNCERROR_OK             0
#define RNCERROR_BADFILEID      -1
#define RNCERROR_UNKNOWNMETHOD  -2

#define HEADERLEN       18
#define CODE_SIZE       32768
#define PACKBLOCKSIZE	0x3000
#define CRCPOLY         0xA001
#define MIN_LEN         2
#define MIN_POS         1024

#define MAX_POS_M1      32768
#define MAX_LEN_M1      4096
#define MAX_RAW_M1      65535

#define MAX_POS_M2      4096
#define MAX_LEN_M2      255
#define MAX_RAW_M2      72

/* PLATFORM_PORT is little-endian like WIN32, so it needs the byte-swapping versions: the RNC
 * data (sizes, words) is big-endian, and a no-op here makes InitUnpack compute a multi-GB
 * OutputEnd → the decompressor overruns the output buffer and segfaults. */
#if defined(WIN32) || defined(PLATFORM_PORT)

#define BIGENDIANW(n)           (((n&0xFF)<<8)+((n&0xFF00)>>8))
#define BIGENDIANL(n)           ((BIGENDIANW(n)<<16)+BIGENDIANW((n>>16)))
#define GETBIGENDIANW(x)        (((WORD)*(x)<<8)+(BYTE)*(x+1))

#else

#define BIGENDIANW(n)           (n)
#define BIGENDIANL(n)           (n)
//#define GETBIGENDIANW(x)        (x)

#endif

#define UPDATE_CRC(r,c)         r=CRCTable[((BYTE)(r)^(BYTE)(c))&0xFF]^(r>>8)

//typedef unsigned char  BYTE;        // 8-bits
//typedef unsigned short WORD;        // 16-bits
typedef unsigned long  ULONG;        // 32-bits

/* PORT: the RNC (Rob Northen ProPack) types below were gated WIN32-only, but the
 * runtime decompressor (UnpackRNC / Propack_UnpackM2, RNC method 2 = Huffman) lives
 * in unpack.c/huffman.c and is needed on the host to decode the cart's RNC assets.
 * Widen the guard to the port build so they compile off-Windows. */
#if defined(WIN32) || defined(PLATFORM_PORT)

typedef struct rncfile {
    BYTE Id[3];
    BYTE Method;
    ULONG UncompressedSize;
    ULONG CompressedSize;
    BYTE Reserved[6];
    BYTE Data[];
};

typedef struct huffman {
	ULONG Frequency;
	WORD EntryPtr;
	ULONG Code;
	WORD CodeLen;
} huffman;

typedef struct rncfile *RNC_fileptr;
typedef struct huffman *HUFFMAN_tableptr;

extern huffman RawHuffmanTable[],
			   PosHuffmanTable[],
			   LenHuffmanTable[];

extern void InitHuffmanTable(HUFFMAN_tableptr Table, BYTE TableSize),
			MakeHuffmanCodes(HUFFMAN_tableptr Table, BYTE n),
			MakeHuffmanTable(HUFFMAN_tableptr Table, BYTE n);
extern WORD FindLowest(HUFFMAN_tableptr Table, BYTE n);
extern ULONG SwapBits(ULONG InBits, BYTE n);

extern BYTE BitBuffBits,
			*InputPtr,
			*OutputPtr;
#endif

extern ULONG PackRNC(BYTE *Input, BYTE *Output, ULONG Len, BYTE Method);
//extern WORD UnpackRNC(RNC_fileptr FilePtr, BYTE *OutputBuffer);
extern void Propack_UnpackM1(void *FilePtr, void *OutputBuffer);
extern void Propack_UnpackM2(void *FilePtr, void *OutputBuffer);

