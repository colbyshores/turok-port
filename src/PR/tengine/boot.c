//------------------------------------------------------------------------
// BOOT.C - Written by Biscuit. This is the bootstrap loader for the remaining
// game code. It will first, load in 8 bytes from the start of main game code
// to the correct place. If it does not have the RNC2 header, it will load the full
// code amount defined by _codeSegmentRomEnd - _codeSegmentRomStart to the location
// _codeSegmentStart. If the RNC header is present, the second word defines the size
// of the compressed data block and that is loaded to _codeSegmentStart at which point
// it will be RNC decompressed. Once this is done. The entrypoint boot is called.
// Note:
// This section of code is seperate from the main game code but may include some
// functions required by the main game but cannot utilize ANY function calls from the
// main game since that code has not yet been loaded.
//

#include "stdafx.h"
#include "tengine.h"

#include "debug.h"


extern void boot(void);
extern u8 _codeSegmentStart[];
extern u8 _codeSegmentEnd[];
extern u8 _codeSegmentDataStart[];
extern u8 _codeSegmentDataEnd[];
extern u8 _codeSegmentRomStart[];
extern u8 _codeSegmentRomEnd[];
extern u8 _codeSegmentBssStart[];
extern u8 _codeSegmentBssEnd[];

extern u8 _mempoolSegmentStart[];
extern u8 _mempoolSegmentEnd[];

//
// This routine was written courtesy of a disassembly of the u64 version of
// the osPiRawReadIo routines. This was reverse programmed so that we would
// have a more efficient version for use on cart.
//
void BootTransfer(u32 src, u32 dst, u32 length)
{
	register u32 *srcaddr;
	register u32 *dstaddr;
	register volatile u32 *status;

	//
	// Set up the physical addresses of the various data blocks and
	// set the length to be in Long Words
	//
	srcaddr = (u32 *)((u32)osRomBase|(u32)src);
	dstaddr = (u32 *)dst;
	status = (u32 *)K0_TO_K1(PI_STATUS_REG);

	length >>=2;

	while ( length--)
	{
		//
		// Wait for busy IO status, in reality, it should never be busy.
		// the question is: how slow is the ROM and what happens while it's trying
		// to fetch the data?
		//
		while ( *status & (PI_STATUS_IO_BUSY|PI_STATUS_DMA_BUSY));
	  	*dstaddr++ = *srcaddr++;
	}

}
//
// Note: The RNC File header is as follows:
//
// Bytes:			Use:
// 0..3				Header "RNC" + compression type #2
// 4..7				Uncompressed length (big-endian)
// 8..11			Compressed length (big-endian)
// 12..End			Compression data
//
void BootEntry(void)
{
	u32 TransferLength,Dest;
	u32 DecompFlag;
	u32 *ptr;
	u8	 method;
	//
	// Grab the code file header to check for RNC compression
	//
	BootTransfer((u32)_codeSegmentRomStart,(u32)_codeSegmentStart,12);

	DecompFlag = ( (_codeSegmentStart[0]=='R') && (_codeSegmentStart[1]=='N') &&
		 		   (_codeSegmentStart[2]=='C') );
	if ( DecompFlag )
	{
		TransferLength = ((_codeSegmentStart[8])<<24) | ((_codeSegmentStart[9])<<16) |
						 ((_codeSegmentStart[10])<< 8) | (_codeSegmentStart[11]);
		//
		// Round up to nearest word and add on length for header transfers
		//
		TransferLength = (TransferLength+23) & ~3;
		Dest = (u32)_mempoolSegmentStart;
	}
	else
	{
		TransferLength = (u32)_codeSegmentRomEnd - (u32)_codeSegmentRomStart;
		Dest = (u32)_codeSegmentStart;
	}
	//
	// Grab the main body of the file and then decompress if needed
	//
	BootTransfer((u32)_codeSegmentRomStart,Dest,TransferLength);

	if ( DecompFlag )
	{
		method = _codeSegmentStart[3];
		if (method == 1)
		{
			Propack_UnpackM1((void *)Dest,_codeSegmentStart);
		}
		else
		{
			Propack_UnpackM2((void *)Dest,_codeSegmentStart);
		}
	}
	//
	// Clear I & D caches, we cheat a little because the O/S should now
	// be alive!
	osInvalICache(_codeSegmentStart, _codeSegmentEnd-_codeSegmentStart);
	osWritebackDCacheAll();


	//
	// Clear out the code area BSS and then call the entry point
	//
	TransferLength = ((u32)_codeSegmentBssEnd-(u32)_codeSegmentBssStart)/4;
	ptr = (u32 *)_codeSegmentBssStart;
	while ( TransferLength-- )
	{
		*ptr++=0x0;
		// *ptr++=0xc9;		// ?
	}

	boot();
}

