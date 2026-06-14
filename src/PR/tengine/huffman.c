/*----------------------------------------------------------------------------
 * PROPACK Unpacker 'C' Source Code
 *
 * Copyright (c) 1995 Rob Northen Computing, UK. All Rights Reserved.
 *
 * File: HUFFMAN.C
 *
 * Date: 07-APR-95
 *----------------------------------------------------------------------------*/

#include "stdafx.h"
#include "pp.h"

BYTE BitBuffBits,
     *InputPtr,
     *OutputPtr;

WORD FirstEntry,
	 SecondEntry;

huffman RawHuffmanTable[16],
		PosHuffmanTable[16],
		LenHuffmanTable[16];

void InitHuffmanTable(HUFFMAN_tableptr Table, BYTE TableSize);
void MakeHuffmanCodes(HUFFMAN_tableptr Table, BYTE n);
void MakeHuffmanTable(HUFFMAN_tableptr Table, BYTE n);
WORD FindLowest(HUFFMAN_tableptr Table, BYTE n);
ULONG SwapBits(ULONG InBits, BYTE n);

void InitHuffmanTable(HUFFMAN_tableptr Table, BYTE TableSize)
{
    while (TableSize--) {
        Table->Frequency = 0;
        Table->EntryPtr = USHRT_MAX;
        Table->Code = 0;
        Table->CodeLen = 0;
        Table++;
    }
}

void MakeHuffmanCodes(HUFFMAN_tableptr Table, BYTE n)
{
    WORD huff_bits = 1,
         i;

    ULONG huff_code = 0,
         huff_base = 0x80000000;

    HUFFMAN_tableptr tableptr;

    while (huff_bits <= 16) {
		tableptr = Table;
        for (i = 0; i < n; i++) {
            if (tableptr->CodeLen == huff_bits) {
                tableptr->Code = SwapBits(huff_code / huff_base, huff_bits);
                huff_code += huff_base;
            }
            tableptr++;
        }
        huff_bits++;
        huff_base >>= 1;
    }
}

void MakeHuffmanTable(HUFFMAN_tableptr Table, BYTE n)
{
    WORD i,
         j,
         k;

    ULONG temp[16];

    for (i = 0; i < 16; i++)
        temp[i] = Table[i].Frequency;

    for (i = j = 0; i < n; i++)
        if (Table[i].Frequency) {
			j++;
            k = i;
        }

    if (j == 0)
        return;

    if (j == 1) {
        Table[k].CodeLen++;
        return;
    }

    while (FindLowest(Table, n)) {
		Table[FirstEntry].Frequency += Table[SecondEntry].Frequency;
        Table[SecondEntry].Frequency = 0;
        Table[FirstEntry].CodeLen++;

        while (Table[FirstEntry].EntryPtr !=  USHRT_MAX) {
            FirstEntry = Table[FirstEntry].EntryPtr;
            Table[FirstEntry].CodeLen++;
        }

        Table[FirstEntry].EntryPtr = SecondEntry;
        Table[SecondEntry].CodeLen++;

        while (Table[SecondEntry].EntryPtr != USHRT_MAX) {
			SecondEntry = Table[SecondEntry].EntryPtr;
            Table[SecondEntry].CodeLen++;
        }
    }

    for (i = 0; i < 16; i++)
        Table[i].Frequency = temp[i];

    MakeHuffmanCodes(Table, n);
}

WORD FindLowest(HUFFMAN_tableptr Table, BYTE n)
{
	BYTE Entry;

    ULONG Freq,
         FirstFreq  = ULONG_MAX,
         SecondFreq = ULONG_MAX;

    for (Entry = 0; Entry < n; Entry++)
		if ((Freq = Table[Entry].Frequency) != 0)
			if (Freq < FirstFreq) {
				SecondFreq  = FirstFreq;
				SecondEntry = FirstEntry;
				FirstFreq   = Freq;
				FirstEntry  = Entry;
			} else
				if (Freq < SecondFreq) {
					SecondFreq  = Freq;
					SecondEntry = Entry;
				}
	if ((FirstFreq == ULONG_MAX) || (SecondFreq == ULONG_MAX))
		return 0;
	return 1;
}

ULONG SwapBits(ULONG InBits, BYTE n)
{
	ULONG OutBits = 0;

	while (n--) {
        OutBits <<= 1;
        if (InBits & 1)
            OutBits |= 1;
        InBits >>= 1;
    }

    return OutBits;
}


