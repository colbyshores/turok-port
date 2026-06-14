/*----------------------------------------------------------------------------
 * PROPACK Unpacker 'C' Source Code
 *
 * Copyright (c) 1995 Rob Northen Computing, UK. All Rights Reserved.
 *
 * File: UNPACK.C
 *
 * Date: 18.03.95
 *----------------------------------------------------------------------------*/

/*
#include <malloc.h>
#include <mem.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
*/



#include "stdafx.h"
#include "pp.h"

BYTE BitBuffM2,
     *OutputEnd;

ULONG BitBuffM1;

WORD UnpackRNC(RNC_fileptr, BYTE *OutputBuffer);
WORD UnpackMethod1(RNC_fileptr FilePtr, BYTE *OutputBuffer);
WORD UnpackMethod2(RNC_fileptr FilePtr, BYTE *OutputBuffer);
void InitUnpack(RNC_fileptr FilePtr, BYTE *OutputBuffer);
WORD InputBitsM1(BYTE n);
WORD InputBitsM2(BYTE n);
void InputHuffmanTable(HUFFMAN_tableptr Table, BYTE TableSize);
WORD InputValue(HUFFMAN_tableptr Table);
WORD InputLenM2(void);
WORD InputPosM2(void);

/*
int main(int argc, char *argv[])
{
    BYTE *InputBuffer,
         *OutputBuffer;

    int handle;

    ULONG FileSize,
         UncompressedSize;

    if (argc < 2) {
        printf("filename required.\n");
        exit(EXIT_FAILURE);
    }

    if ((handle = open(argv[1], O_RDONLY | O_BINARY)) == -1) {
        printf("failed to open: %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    FileSize = filelength(handle);

    if ((InputBuffer = (BYTE *)malloc(FileSize)) == NULL) {
        printf("failed to allocate input buffer\n");
        exit(EXIT_FAILURE);
    }

    if (read(handle, InputBuffer, FileSize) != FileSize) {
        close(handle);
        printf("failed to read file\n");
        exit(EXIT_FAILURE);
    }

    close(handle);

    UncompressedSize = BIGENDIANL(((RNC_fileptr)InputBuffer)->UncompressedSize);

    if ((OutputBuffer = (BYTE *)malloc(UncompressedSize)) == NULL) {
        free((void *)InputBuffer);
        printf("failed to allocate output buffer\n");
        exit(EXIT_FAILURE);
    }

    if (UnpackRNC((RNC_fileptr)InputBuffer, OutputBuffer) != 0) {
        printf("failed to unpack RNC file\n");
        free((void *)InputBuffer);
        free((void *)OutputBuffer);
        exit(EXIT_FAILURE);
    }

    free((void *)InputBuffer);
    free((void *)OutputBuffer);
    return(EXIT_SUCCESS);
}
*/

WORD UnpackRNC(RNC_fileptr FilePtr, BYTE *OutputBuffer)
{
    WORD	result;
	 char	*string;

	 string = (char *)&FilePtr->Id[0];

	 if ((string[0] != 'R') || (string[1] != 'N') || (string[2] != 'C'))
		 return RNCERROR_BADFILEID;


    switch (FilePtr->Method) {
        case 0:
				// Rob Northern's original lines - didn't compile
            // _fmemcpy(OutputBuffer, &FilePtr->Data, BIGENDIANL(FilePtr->UncompressedSize));
            memcpy(OutputBuffer, &FilePtr->Data, BIGENDIANL(FilePtr->UncompressedSize));
            result = RNCERROR_OK;
            break;

        case 1:
            result = UnpackMethod1(FilePtr, OutputBuffer);
            break;

        case 2:
            result = UnpackMethod2(FilePtr, OutputBuffer);
            break;

        default:
            result = RNCERROR_UNKNOWNMETHOD;
    }

    return result;
}

WORD UnpackMethod1(RNC_fileptr FilePtr, BYTE *OutputBuffer)
{
    BYTE *Ptr;

    WORD LoopCount,
         Len;

    InitUnpack(FilePtr, OutputBuffer);

    InputBitsM1(2); // lock and key bits

    while (OutputPtr < OutputEnd) {
        InputHuffmanTable(&RawHuffmanTable[0], 16);
        InputHuffmanTable(&PosHuffmanTable[0], 16);
        InputHuffmanTable(&LenHuffmanTable[0], 16);
        LoopCount = InputBitsM1(16);
        goto Start;

        Again:
            Ptr = OutputPtr - (InputValue(PosHuffmanTable) + 1);
            Len = InputValue(LenHuffmanTable) + 2;
            while (Len--)
                *OutputPtr++ = *Ptr++;
        Start:
            Len = InputValue(RawHuffmanTable);
            while (Len--)
                *OutputPtr++ = *InputPtr++;
            BitBuffM1 = ((((ULONG)*(InputPtr+2) << 16) + ((WORD)*(InputPtr+1) << 8) + *InputPtr) << BitBuffBits)
                         + (BitBuffM1 & (1 << BitBuffBits) - 1);
            if (--LoopCount) goto Again;
    }

    return RNCERROR_OK;
}

WORD UnpackMethod2(RNC_fileptr FilePtr, BYTE *OutputBuffer)
{
    BYTE *Ptr;

    WORD Pos,
         Len;

    InitUnpack(FilePtr, OutputBuffer);

    InputBitsM2(2); // lock and key bits

    while (OutputPtr < OutputEnd) {
        for (;;) {
            while (InputBitsM2(1) == 0)
                *OutputPtr++ = *InputPtr++;
            if (InputBitsM2(1)) {
                if (InputBitsM2(1) == 0) {
                    Len = 2;
                    Pos = *InputPtr++ + 1;
                } else {
                    if (InputBitsM2(1) == 0)
                        Len = 3;
                    else {
                        if ((Len = *InputPtr++ + 8) == 8)
                            break;
                    }
                    Pos = InputPosM2();
                }
                Ptr = OutputPtr - Pos;
                while (Len--)
                    *OutputPtr++ = *Ptr++;
            }
            else {
                if ((Len = InputLenM2()) == 9) {
                    Len = (InputBitsM2(4) << 2) + 12;
                    while (Len--)
                        *OutputPtr++ = *InputPtr++;
                } else {
                    Ptr = OutputPtr - InputPosM2();
                    while (Len--)
                        *OutputPtr++ = *Ptr++;
                }
            }
        }
        InputBitsM2(1);
    }

    return RNCERROR_OK;
}

void InitUnpack(RNC_fileptr FilePtr, BYTE *OutputBuffer)
{
    InputPtr = (BYTE *)&FilePtr->Data;
    OutputPtr = OutputBuffer;
    OutputEnd = OutputBuffer + BIGENDIANL(FilePtr->UncompressedSize);
    BitBuffBits = 0;
}

WORD InputBitsM1(BYTE n)
{
    WORD Bits = 0,
         BitMask = 1;

    while (n--) {
        if (BitBuffBits == 0) {
            BitBuffM1 = *((ULONG *)InputPtr);
            InputPtr += 2;
            BitBuffBits = 16;
        }
        if (BitBuffM1 & 1)
            Bits |= BitMask;
        BitMask <<= 1;
        BitBuffM1 >>= 1;
        BitBuffBits--;
    }

    return Bits;
}

WORD InputBitsM2(BYTE n)
{
    WORD Bits = 0;

    while (n--) {
        if (BitBuffBits == 0) {
            BitBuffM2 = *InputPtr++;
            BitBuffBits = 8;
        }
        Bits <<= 1;
        if (BitBuffM2 & 0x80)
            Bits++;
        BitBuffM2 <<= 1;
        BitBuffBits--;
    }

    return Bits;
}

void InputHuffmanTable(HUFFMAN_tableptr Table, BYTE TableSize)
{
    BYTE n,
         i;

    HUFFMAN_tableptr tableptr;

    InitHuffmanTable(Table, TableSize);

    if ((n = (BYTE)InputBitsM1(5)) != 0) {
        if (n > 16)
            n = 16;
        tableptr = Table;
        for (i = 0; i < n; i++) {
            tableptr->CodeLen = InputBitsM1(4);
            tableptr++;
        }
        MakeHuffmanCodes(Table, n);
    }
}

WORD InputValue(HUFFMAN_tableptr Table)
{
    BYTE Bits = 0;

    HUFFMAN_tableptr tableptr;

    tableptr = Table;

	while ( (tableptr->CodeLen == 0) || ((BitBuffM1 & (1 << tableptr->CodeLen) - 1) != tableptr->Code)) {
		tableptr++;
        Bits++;
    }

    InputBitsM1(tableptr->CodeLen);

    if (Bits < 2)
        return Bits;

    return InputBitsM1(Bits - 1) | (1 << (Bits - 1));
}

WORD InputLenM2(void)
{
    WORD Len = InputBitsM2(1) + 4;

    if (InputBitsM2(1) == 0)
        return Len;

    return ((Len - 1) << 1) + InputBitsM2(1);
}

WORD InputPosM2(void)
{
    WORD Pos = 0;

    if (InputBitsM2(1)) {
        Pos = InputBitsM2(1);
        if (InputBitsM2(1)) {
            Pos = ((Pos << 1) + InputBitsM2(1)) | 4;
            if (InputBitsM2(1) == 0)
                Pos = (Pos << 1) + InputBitsM2(1);
        } else
            if (Pos == 0)
                Pos = InputBitsM2(1) + 2;
    }

    return (Pos << 8) + *InputPtr++ + 1;
}

