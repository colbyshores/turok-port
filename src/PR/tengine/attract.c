// Attract mode recording/playback by S.Broumley
// control.c

#include "stdafx.h"
#include <ramrom.h>
#include "tengine.h"
#include "control.h"
#include "attract.h"
#include "boss.h"
#include "options.h"







/////////////////////////////////////////////////////////////////////////////
// Constants
/////////////////////////////////////////////////////////////////////////////
#define PLAY_DEMOS							   1	// Play demo's in frontend
#define PLAY_ROM_DEMOS							1	// 1 = play rom binaries, 0 = play c file


/////////////////////////////////////////////////////////////////////////////
// Record Constants - osSyncPrintf's must be enabled
//
// TO RECORD:
//
// 1) Set "RECORD_ATTRACT_DEMO" to 1, and set "RECORD_WARP_ID" to the required
//		warp.
//
//	2) Compile and download level with "gload", not "gload -s"!!
//	
//	3)	Press start to play game as usual - record will then start.
//
//	4) When finished recording, hit start again - ascii hex will be printed on the
//		telnet screen.
//
//	5) Copy the ascii text BETWEEN the lines "ASC START" and "ASC END"
//		(use "copy" from "edit" menu) and paste it into text file (make sure
//		text file is empty)  "demo.asc" on "p:\design\attract\"
//
//	6) From "p:\design\attract\", type "asc2bin demo.asc demoname.bin" -
//		this will create the binary file "demoname.bin"
//
//	7) Load binary into ".ROK" file and set id to something unique. (0 cannot be used)
//
// 8) Include binary id number in the array "RomBinaryDemoList" which is located
//		a bit further down in this file.
//
//	9) Make sure you set "RECORD_ATTRACT_DEMO" to 0,
//		"PLAY_DEMOS" to 1, and "PLAY_ROM_DEMOS" to 1 for final version
//
/////////////////////////////////////////////////////////////////////////////
#define RECORD_ATTRACT_DEMO					0		// Set to 1 for recording a level
#define RECORD_WARP_ID							7900	// Warp id of level to record


/////////////////////////////////////////////////////////////////////////////
// Debug Constants
/////////////////////////////////////////////////////////////////////////////
#define DEBUG_ATTRACT							0	// Display attract info
#define ATTRACT_SHOW_CHECKSUM				 	0	// Show each frames checksum

#define ATTRACT_SHOW_BUTTON_CHANGES			0	// Show each record
#define ATTRACT_SHOW_STICK_CHANGES			0	// Show each record
#define ATTRACT_SHOW_NEXT_FIELDS_CHANGES 	0	// Show each record

#define ATTRACT_DATA_STICK_BITS				8	// Analog x,y bits to store (8 is all bits)



/////////////////////////////////////////////////////////////////////////////
// Demo record vars
/////////////////////////////////////////////////////////////////////////////

// 30 is a good number - leave it - it works!!!
#define RECORD_NO_MOVE_FRAMES					30	// Number of frames before recording begins

#if RECORD_ATTRACT_DEMO

UINT8		AttractBuffer[1024] ;
UINT8		ButtonBuffer[1024] ;
UINT8		StickXBuffer[2048] ;
UINT8		StickYBuffer[1024] ;
UINT8		NextFieldsBuffer[1024] ;

void CAttractDemo__PrintData() ;

#define DEMO_BUFFER_SIZE						(2*2)

#else

#define DEMO_BUFFER_SIZE						(1024*2)

#endif



/////////////////////////////////////////////////////////////////////////////
// CFile test demo
/////////////////////////////////////////////////////////////////////////////

#if PLAY_DEMOS
#if !PLAY_ROM_DEMOS

UINT32 CFileDemoBuffer[] =
{
        0,
} ;
                                         
#endif
#endif

                                                                                                               

/////////////////////////////////////////////////////////////////////////////
// List of binary demo id's that will play during the front end
/////////////////////////////////////////////////////////////////////////////

INT16 RomBinaryDemoList[] =
{
	1001,
	1002,
	1003,
	1004,
	1005,
	1006,
	1007,
	1008,
	1009,
	1010,
	1011,
	1012,
	1013,
	1014,
	1015,
	1016,

	0,	// Repeat list - THIS MUST BE HERE
} ;

                                                                                                                                                                                                                  
/////////////////////////////////////////////////////////////////////////////
// Structures
/////////////////////////////////////////////////////////////////////////////

typedef struct CAttractBuffer_t
{
	UINT32	m_Value ;			 							// Current value
	UINT8		*m_pData ;										// Ptr to data
	UINT32	m_CurrentBit ;									// Current bit within buffer
	INT32		m_Frames ;										// Frame count
	void (*m_pReadFunction)(void *pThis) ;				// Used when reading data
	void (*m_pWriteFunction)(void *pThis) ;			// Used when writing data
	UINT32 (*m_pGetValueFunction)(void *pThis) ;		// Used when writing data
} CAttractBuffer ;


typedef struct CAttractHeader_t
{
	UINT16	m_WarpID ;					// Level warp id to start at
	INT16		m_TotalFrames ;			// Total frames in demo
	INT16		m_ButtonStartBit ;		// Button start bit number in data
	INT16		m_StickXStartBit ;		// Stick X start bit number in data
	INT16		m_StickYStartBit ;		// Stick Y start bit number in data
	INT16		m_NextFieldsStartBit ;	// Next fields start bit number in data
} CAttractHeader ;


typedef struct CAttractDemo_t
{
	INT16			*m_pCurrentDemo ;						// Ptr to current binary demo id
	BOOL			m_DemoRequested ;						// TRUE if new demo should be loaded
	volatile		BOOL m_DemoRecieved ;				// TRUE if demo grabbed from ROM
	UINT8			m_DemoBuffer[DEMO_BUFFER_SIZE] ;	// Demo data

	BOOL			m_Active ;		// TRUE if recording/playing back
	BOOL			m_Record ;		// TRUE if recording
	BOOL			m_Play ;			// TRUE if playing back
	BOOL			m_FirstFrame ;	// First time ran
	BOOL			m_Finished ;	// Set when record/playback is done

	CAttractBuffer	m_ButtonBuffer ;
	CAttractBuffer	m_StickXBuffer ;
	CAttractBuffer	m_StickYBuffer ;
	CAttractBuffer	m_NextFieldsBuffer ;
	CAttractBuffer	m_UpdatedBuffer ;

	INT32			m_PlayRecordFrame ;	// Current play or record frame
	INT32			m_TotalFrames ;		// Current play or record frame
	FLOAT			m_TotalTime ;			// Time demo ran/recorded
	UINT32		m_Checksum ;
} CAttractDemo ;




/////////////////////////////////////////////////////////////////////////////
// Top class var
/////////////////////////////////////////////////////////////////////////////

CAttractDemo	AttractDemo ;




/////////////////////////////////////////////////////////////////////////////
//
//
// ATTRACT BUFFER CLASS CODE	
//
//
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// Setup defaults
/////////////////////////////////////////////////////////////////////////////
void CAttractBuffer__Construct(CAttractBuffer *pThis, UINT8 *pData, UINT32 StartBit, 
										 void *pReadFunc, void *pWriteFunc, void *pGetValueFunc)
{
	pThis->m_pData = pData ;
	pThis->m_CurrentBit = StartBit ;
	pThis->m_Frames = 1 ;
	pThis->m_pReadFunction = pReadFunc ;
	pThis->m_pWriteFunction = pWriteFunc ;
	pThis->m_pGetValueFunction = pGetValueFunc ;
}

/////////////////////////////////////////////////////////////////////////////
// General functions
/////////////////////////////////////////////////////////////////////////////

#if RECORD_ATTRACT_DEMO
void CAttractBuffer__WriteValue(CAttractBuffer *pThis)
{
	pThis->m_pWriteFunction(pThis) ;
}
#endif

void CAttractBuffer__ReadValue(CAttractBuffer *pThis)
{
	pThis->m_pReadFunction(pThis) ;
}

UINT32 CAttractBuffer__GetValue(CAttractBuffer *pThis)
{
	return (pThis->m_pGetValueFunction(pThis)) ;
}

void CAttractBuffer__UpdateValue(CAttractBuffer *pThis)
{
	pThis->m_Value = CAttractBuffer__GetValue(pThis) ;
	pThis->m_Frames = 0 ;
}



/////////////////////////////////////////////////////////////////////////////
// Determine how many bytes are used in an UINT32
/////////////////////////////////////////////////////////////////////////////

#if RECORD_ATTRACT_DEMO
UINT32 BitsInUINT32(UINT32 Data)
{
	UINT32	Mask = 0x80000000 ;
	UINT32	Bits = 32 ;
	UINT32	i = 32 ;

	while(i--)
	{
		if (Data & Mask)
			return Bits ;

		Mask >>= 1 ;
		Bits-- ;
	}

	// Must have mininum of 1!
	return 0 ;
}

void WriteBit(UINT8 *pData, UINT32 Bit, UINT32 Data)
{
	// Get byte offset
	UINT8	*pDest = pData + (Bit >> 3) ;

	// Get bit offset
	UINT8	Mask = 128 >> (Bit & 7)  ;

	if (Data & 1)
		*pDest |= Mask ;
	else
		*pDest &= ~Mask ;
}
#endif

UINT32 ReadBit(UINT8 *pData, UINT32 Bit)
{
	// Get byte offset
	UINT8	*pDest = pData + (Bit >> 3) ;

	// Get bit offset
	UINT8	Mask = 128 >> (Bit & 7)  ;

	if (*pDest & Mask)
		return 1 ;
	else
		return 0 ;
}


/////////////////////////////////////////////////////////////////////////////
// Write a single bit
/////////////////////////////////////////////////////////////////////////////
#if RECORD_ATTRACT_DEMO
void CAttractBuffer__WriteBit(CAttractBuffer *pThis, UINT32 Data)
{
	WriteBit(pThis->m_pData, pThis->m_CurrentBit++, Data) ;
}
#endif

/////////////////////////////////////////////////////////////////////////////
// Read a single bit
/////////////////////////////////////////////////////////////////////////////
UINT32 CAttractBuffer__ReadBit(CAttractBuffer *pThis)
{
	return ReadBit(pThis->m_pData, pThis->m_CurrentBit++) ;
}

/////////////////////////////////////////////////////////////////////////////
// Write a set of bits
/////////////////////////////////////////////////////////////////////////////
#if RECORD_ATTRACT_DEMO
void CAttractBuffer__WriteBits(CAttractBuffer *pThis, UINT32 Bits, UINT32 Data)
{
	// Write in reverse order
	while(Bits--)
		CAttractBuffer__WriteBit(pThis, Data >> Bits) ;
}
#endif

/////////////////////////////////////////////////////////////////////////////
// Read a set of bits
/////////////////////////////////////////////////////////////////////////////
UINT32 CAttractBuffer__ReadBits(CAttractBuffer *pThis, UINT32 Bits)
{
	UINT32	Data = 0 ;

	while(Bits--)
	{
		Data <<= 1 ;
		Data |= CAttractBuffer__ReadBit(pThis) ;
	}

	return Data ;
}

/////////////////////////////////////////////////////////////////////////////
// Write time packet
/////////////////////////////////////////////////////////////////////////////
#if RECORD_ATTRACT_DEMO
void CAttractBuffer__WriteTime(CAttractBuffer *pThis)
{
//	// Write 4bit time bits, Write ?bit time
//	CAttractBuffer__WriteBits(pThis, 4, BitsInUINT32(pThis->m_Frames)) ;
//	CAttractBuffer__WriteBits(pThis, BitsInUINT32(pThis->m_Frames), pThis->m_Frames) ;
//	pThis->m_Frames = 0 ;

	// Write 3bit time bits, Write ?bit time
	UINT32 Bits = BitsInUINT32(pThis->m_Frames - 1) ;
	CAttractBuffer__WriteBits(pThis, 3, Bits) ;
	if (Bits)
		CAttractBuffer__WriteBits(pThis, Bits, pThis->m_Frames - 1) ;

	pThis->m_Frames = 0 ;
}


BOOL CAttractBuffer__FramesHasTooManyBits(CAttractBuffer *pThis)
{
	// We only have 3 bits to store the time!
	return (BitsInUINT32(pThis->m_Frames) > 7) ;
}


#endif

/////////////////////////////////////////////////////////////////////////////
// Read time packet
/////////////////////////////////////////////////////////////////////////////
void CAttractBuffer__ReadTime(CAttractBuffer *pThis)
{
	UINT32 Bits = CAttractBuffer__ReadBits(pThis, 3) ;
	if (Bits)
		pThis->m_Frames = CAttractBuffer__ReadBits(pThis, Bits) + 1 ;
	else
		pThis->m_Frames = 1 ;
}



/////////////////////////////////////////////////////////////////////////////
// Button packet
/////////////////////////////////////////////////////////////////////////////
#if RECORD_ATTRACT_DEMO
void CAttractBuffer__WriteButton(CAttractBuffer *pThis)
{
#if ATTRACT_SHOW_BUTTON_CHANGES
	osSyncPrintf("WRITE GF:%d AF:%d Tx:%f Tz:%f Butn:%d Frms:%d\r\n",
				  game_frame_number,
				  AttractDemo.m_PlayRecordFrame, 	
				  CEngineApp__GetPlayer(GetApp())->ah.ih.m_vPos.x,	
				  CEngineApp__GetPlayer(GetApp())->ah.ih.m_vPos.z,	
				  pThis->m_Value, 
				  pThis->m_Frames) ;	
#endif

	// Write bit button data
	CAttractBuffer__WriteBit(pThis, ((pThis->m_Value & A_BUTTON) != 0)) ;
	CAttractBuffer__WriteBit(pThis, ((pThis->m_Value & B_BUTTON) != 0)) ;
	CAttractBuffer__WriteBit(pThis, ((pThis->m_Value & R_TRIG) != 0)) ;
	CAttractBuffer__WriteBit(pThis, ((pThis->m_Value & Z_TRIG) != 0)) ;
	CAttractBuffer__WriteBit(pThis, ((pThis->m_Value & U_CBUTTONS) != 0)) ;
	CAttractBuffer__WriteBit(pThis, ((pThis->m_Value & L_CBUTTONS) != 0)) ;
	CAttractBuffer__WriteBit(pThis, ((pThis->m_Value & R_CBUTTONS) != 0)) ;
	CAttractBuffer__WriteBit(pThis, ((pThis->m_Value & D_CBUTTONS) != 0)) ;
	CAttractBuffer__WriteBit(pThis, ((pThis->m_Value & R_JPAD) != 0)) ;

	// Write time bits
	CAttractBuffer__WriteTime(pThis) ;
}
#endif

void CAttractBuffer__ReadButton(CAttractBuffer *pThis)
{
	// Right handed playback
	pThis->m_Value = CAttractBuffer__ReadBit(pThis) * A_BUTTON ;
	pThis->m_Value |= CAttractBuffer__ReadBit(pThis) * B_BUTTON ;
	pThis->m_Value |= CAttractBuffer__ReadBit(pThis) * R_TRIG ;
	pThis->m_Value |= CAttractBuffer__ReadBit(pThis) * Z_TRIG ;
	pThis->m_Value |= CAttractBuffer__ReadBit(pThis) * U_CBUTTONS ;
	pThis->m_Value |= CAttractBuffer__ReadBit(pThis) * L_CBUTTONS ;
	pThis->m_Value |= CAttractBuffer__ReadBit(pThis) * R_CBUTTONS ;
	pThis->m_Value |= CAttractBuffer__ReadBit(pThis) * D_CBUTTONS ;
	pThis->m_Value |= CAttractBuffer__ReadBit(pThis) * R_JPAD ;

	// Read time bits
	CAttractBuffer__ReadTime(pThis) ;

#if ATTRACT_SHOW_BUTTON_CHANGES
	osSyncPrintf("READ GF:%d AF:%d Tx:%f Tz:%f Butn:%d Frms:%d\r\n",
				  game_frame_number,
				  AttractDemo.m_PlayRecordFrame, 	
				  CEngineApp__GetPlayer(GetApp())->ah.ih.m_vPos.x,	
				  CEngineApp__GetPlayer(GetApp())->ah.ih.m_vPos.z,	
				  pThis->m_Value, 
				  pThis->m_Frames) ;	
#endif
}

#if RECORD_ATTRACT_DEMO
UINT32 CAttractBuffer__GetButton(CAttractBuffer *pThis)
{
	return (PlayerControllerData.button &
		(A_BUTTON |	B_BUTTON | R_TRIG | Z_TRIG | U_CBUTTONS | L_CBUTTONS | R_CBUTTONS | D_CBUTTONS | R_JPAD)) ;
}
#endif

/////////////////////////////////////////////////////////////////////////////
// Stick packet
/////////////////////////////////////////////////////////////////////////////
#if RECORD_ATTRACT_DEMO
void CAttractBuffer__WriteStick(CAttractBuffer *pThis)
{
#if ATTRACT_SHOW_STICK_CHANGES
	osSyncPrintf("WRITE GF:%d AF:%d Tx:%f Tz:%f Stck:%d Frms:%d\r\n",
				  game_frame_number,
				  AttractDemo.m_PlayRecordFrame, 	
				  CEngineApp__GetPlayer(GetApp())->ah.ih.m_vPos.x,	
				  CEngineApp__GetPlayer(GetApp())->ah.ih.m_vPos.z,	
				  pThis->m_Value, 
				  pThis->m_Frames) ;	
#endif

	// Write ?bit analog data
	CAttractBuffer__WriteBits(pThis, ATTRACT_DATA_STICK_BITS, pThis->m_Value >> (8-ATTRACT_DATA_STICK_BITS)) ;

	// Write time bits
	CAttractBuffer__WriteTime(pThis) ;
}
#endif

void CAttractBuffer__ReadStick(CAttractBuffer *pThis)
{
	// Read ?bit analog data
	pThis->m_Value = CAttractBuffer__ReadBits(pThis, ATTRACT_DATA_STICK_BITS) << (8-ATTRACT_DATA_STICK_BITS)  ;

	// Read time bits
	CAttractBuffer__ReadTime(pThis) ;

#if ATTRACT_SHOW_STICK_CHANGES
	osSyncPrintf("READ GF:%d AF:%d Tx:%f Tz:%f Stck:%d Frms:%d\r\n",
				  game_frame_number,
				  AttractDemo.m_PlayRecordFrame, 	
				  CEngineApp__GetPlayer(GetApp())->ah.ih.m_vPos.x,	
				  CEngineApp__GetPlayer(GetApp())->ah.ih.m_vPos.z,	
				  pThis->m_Value, 
				  pThis->m_Frames) ;	
#endif
}

#if RECORD_ATTRACT_DEMO
UINT32 CAttractBuffer__GetStickX(CAttractBuffer *pThis)
{
	return (PlayerControllerData.stick_x) & 255 ;
}

UINT32 CAttractBuffer__GetStickY(CAttractBuffer *pThis)
{
	return (PlayerControllerData.stick_y) & 255 ;
}
#endif

/////////////////////////////////////////////////////////////////////////////
// Next fields packet
/////////////////////////////////////////////////////////////////////////////
#if RECORD_ATTRACT_DEMO
void CAttractBuffer__WriteNextFields(CAttractBuffer *pThis)
{
#if ATTRACT_SHOW_NEXT_FIELDS_CHANGES
	osSyncPrintf("WRITE GF:%d AF:%d Tx:%f Tz:%f NtFlds:%d Frms:%d\r\n",
				  game_frame_number,
				  AttractDemo.m_PlayRecordFrame, 	
				  CEngineApp__GetPlayer(GetApp())->ah.ih.m_vPos.x,	
				  CEngineApp__GetPlayer(GetApp())->ah.ih.m_vPos.z,	
				  pThis->m_Value, 
				  pThis->m_Frames) ;	
#endif

	// Write 2bit fields data (value is 2 to 5, so write 0-3)
	CAttractBuffer__WriteBits(pThis, 2, pThis->m_Value - 2) ;

	// Write time bits
	CAttractBuffer__WriteTime(pThis) ;
}
#endif

void CAttractBuffer__ReadNextFields(CAttractBuffer *pThis)
{
	// Read fields data (read 0-3, and then make it 2 to 5)
	pThis->m_Value = CAttractBuffer__ReadBits(pThis, 2) + 2 ;

	// Read time bits
	CAttractBuffer__ReadTime(pThis) ;

#if ATTRACT_SHOW_NEXT_FIELDS_CHANGES
	osSyncPrintf("READ GF:%d AF:%d Tx:%f Tz:%f NtFlds:%d Frms:%d\r\n",
				  game_frame_number,
				  AttractDemo.m_PlayRecordFrame, 	
				  CEngineApp__GetPlayer(GetApp())->ah.ih.m_vPos.x,	
				  CEngineApp__GetPlayer(GetApp())->ah.ih.m_vPos.z,	
				  pThis->m_Value, 
				  pThis->m_Frames) ;	
#endif
}

#if RECORD_ATTRACT_DEMO
UINT32 CAttractBuffer__GetNextFields(CAttractBuffer *pThis)
{
	return nNextFields ;
}
#endif



/////////////////////////////////////////////////////////////////////////////
// Recording
/////////////////////////////////////////////////////////////////////////////
#if RECORD_ATTRACT_DEMO
void CAttractBuffer__UpdateRecord(CAttractBuffer *pThis)
{
	// Value changed - if so write data?
	if ((pThis->m_Value != CAttractBuffer__GetValue(pThis)) ||	 
		 (CAttractBuffer__FramesHasTooManyBits(pThis)))
	{
		CAttractBuffer__WriteValue(pThis) ;
		CAttractBuffer__UpdateValue(pThis) ;
	}

	// Update frame count
	pThis->m_Frames++ ;
}
#endif

/////////////////////////////////////////////////////////////////////////////
// Play back
/////////////////////////////////////////////////////////////////////////////
void CAttractBuffer__UpdatePlay(CAttractBuffer *pThis)
{
	if (--pThis->m_Frames <= 0)
		CAttractBuffer__ReadValue(pThis) ;
}







/////////////////////////////////////////////////////////////////////////////
//
//
// ATTRACT DEMO CLASS CODE	
//
//
/////////////////////////////////////////////////////////////////////////////

#if RECORD_ATTRACT_DEMO
void CAttractDemo__ConstructRecord()
{
	CAttractDemo	*pThis = &AttractDemo ;

	pThis->m_Active = FALSE ;
	pThis->m_Record = TRUE ;
	pThis->m_Play = FALSE ;
	pThis->m_Finished = FALSE ;
	pThis->m_FirstFrame = TRUE ;

	pThis->m_PlayRecordFrame = 0 ;
	pThis->m_TotalFrames = 0 ;
	pThis->m_TotalTime = 0 ;

	pThis->m_Checksum = 0 ;

	CAttractBuffer__Construct(&pThis->m_ButtonBuffer, ButtonBuffer, 0,
									 CAttractBuffer__ReadButton,
									 CAttractBuffer__WriteButton,
									 CAttractBuffer__GetButton) ;

	CAttractBuffer__Construct(&pThis->m_StickXBuffer, StickXBuffer, 0,
									 CAttractBuffer__ReadStick,
									 CAttractBuffer__WriteStick,
									 CAttractBuffer__GetStickX) ;

	CAttractBuffer__Construct(&pThis->m_StickYBuffer, StickYBuffer, 0,
									 CAttractBuffer__ReadStick,
									 CAttractBuffer__WriteStick,
									 CAttractBuffer__GetStickY) ;

	CAttractBuffer__Construct(&pThis->m_NextFieldsBuffer, NextFieldsBuffer, 0,
									 CAttractBuffer__ReadNextFields,
									 CAttractBuffer__WriteNextFields,
									 CAttractBuffer__GetNextFields) ;
}
#endif



/////////////////////////////////////////////////////////////////////////////
//
//
// PLAYBACK DEMO CODE	
//
//
/////////////////////////////////////////////////////////////////////////////


// This function is called when demo is recieved from rom
void CAttractDemo__DemoFromRomReceived(void *pThis, CCacheEntry **ppceTarget)
{
	CAttractDemo	*pAttractDemoThis = &AttractDemo ;
	void *pbDemo = CCacheEntry__GetData(*ppceTarget) ;
	int	Size = (*ppceTarget)->m_DataLength ;

	// Copy demo from cache to demo buffer
	ASSERT(Size <= DEMO_BUFFER_SIZE) ;
	memcpy(pAttractDemoThis->m_DemoBuffer, pbDemo, Size) ;
	pAttractDemoThis->m_DemoRecieved = TRUE ;
}

// Reads demo binary data from rom
BOOL CAttractDemo__LoadDemoFromRom(int nDemo)
{
	static CCacheEntry *pceDemo ;

	// Is the cache valid?
	if (!cache_is_valid)
		return FALSE;

	// Request block from rom
	return (CScene__RequestBinaryBlock(&GetApp()->m_Scene,
												  nDemo,
												  &pceDemo,
												  NULL, (pfnCACHENOTIFY) CAttractDemo__DemoFromRomReceived)) ;
}

// Reads demo binary data from rom
void CAttractDemo__RequestNewDemo(void)
{
#if PLAY_ROM_DEMOS
	CAttractDemo	*pThis = &AttractDemo ;
	pThis->m_DemoRequested = TRUE ;
	pThis->m_DemoRecieved = FALSE ;
#endif
}

void CAttractDemo__CheckForNewDemoRequest(void)
{
#if PLAY_ROM_DEMOS
	CAttractDemo	*pThis = &AttractDemo ;

	// Requesting a demo?
	if ((pThis->m_DemoRequested) && (CAttractDemo__LoadDemoFromRom(*pThis->m_pCurrentDemo)))
	{
		pThis->m_DemoRequested = FALSE ;

		// Goto demo for next time
		pThis->m_pCurrentDemo++ ;
		if (*pThis->m_pCurrentDemo == 0)
			pThis->m_pCurrentDemo = RomBinaryDemoList ;
	}
#endif
}







void CAttractDemo__ConstructPlay(UINT8 *pDemo)
{
	CAttractDemo	*pThis = &AttractDemo ;
	CAttractHeader *pHeader = (CAttractHeader *)pDemo ;
	UINT8				*pData = (UINT8 *)pDemo ;

	pThis->m_Active = FALSE ;
	pThis->m_Record = FALSE ;
	pThis->m_Play = TRUE ;
	pThis->m_Finished = FALSE ;
	pThis->m_FirstFrame = TRUE ;

	// Set the level
	GetApp()->m_WarpID = pHeader->m_WarpID ;

	// Prepare button playback
	CAttractBuffer__Construct(&pThis->m_ButtonBuffer, pData, pHeader->m_ButtonStartBit,
									 CAttractBuffer__ReadButton,
									 NULL,	// CAttractBuffer__WriteButton
									 NULL) ;	// CAttractBuffer__GetButton

	// Prepare stick x playback
	CAttractBuffer__Construct(&pThis->m_StickXBuffer, pData, pHeader->m_StickXStartBit,
									 CAttractBuffer__ReadStick,
									 NULL,	// CAttractBuffer__WriteStick
									 NULL) ; // CAttractBuffer__GetStickX

	// Prepare stick y playback
	CAttractBuffer__Construct(&pThis->m_StickYBuffer, pData, pHeader->m_StickYStartBit,
									 CAttractBuffer__ReadStick,
									 NULL,	// CAttractBuffer__WriteStick
									 NULL) ;	// CAttractBuffer__GetStickY

	// Prepare framerate playback
	CAttractBuffer__Construct(&pThis->m_NextFieldsBuffer, pData, pHeader->m_NextFieldsStartBit,
									 CAttractBuffer__ReadNextFields,
									 NULL,	// CAttractBuffer__WriteNextFields
									 NULL) ;	// CAttractBuffer__GetNextFields

	// Reset misc
	pThis->m_PlayRecordFrame = 0 ;
	pThis->m_TotalFrames = pHeader->m_TotalFrames ;
	pThis->m_TotalTime = 0 ;
	pThis->m_Checksum = 0 ;
}

BOOL CAttractDemo__CheckForRecord()
{
#if RECORD_ATTRACT_DEMO
	// Set level
	GetApp()->m_WarpID = RECORD_WARP_ID ;
	return TRUE ;
#else
	return FALSE ;
#endif
	
}

	  

void CAttractDemo__UpdateChecksum(void)
{
	CAttractDemo	*pThis = &AttractDemo ;

	// Tot up the old checksum value
	pThis->m_Checksum += game_frame_number * pThis->m_ButtonBuffer.m_Value * 0x12345678 ;
	pThis->m_Checksum += game_frame_number * pThis->m_StickXBuffer.m_Value * 0x87654321 ;
	pThis->m_Checksum += game_frame_number * pThis->m_StickYBuffer.m_Value * 0x12348765 ;
	pThis->m_Checksum += game_frame_number * pThis->m_NextFieldsBuffer.m_Value * 0x87651234 ;


#if ATTRACT_SHOW_CHECKSUM
	osSyncPrintf("// GFrm:%d Btn:%d Sx:%d Sy:%d Nf:%d Chksum:%d Rnd:%d\r\n", 
				  game_frame_number, 
				  pThis->m_ButtonBuffer.m_Value,	
				  pThis->m_StickXBuffer.m_Value,	
				  pThis->m_StickYBuffer.m_Value,	
				  pThis->m_NextFieldsBuffer.m_Value,	
				  pThis->m_Checksum,
				  random_value) ;
#endif

}



BOOL CAttractDemo__Play(void)
{
#if PLAY_DEMOS
	CAttractDemo	*pThis = &AttractDemo ;

	BOSS_LevelComplete() ;

#if PLAY_ROM_DEMOS

	// Is demo ready?
	if (!pThis->m_DemoRecieved)
		return FALSE ;

	// Play rom demo
	CAttractDemo__ConstructPlay(pThis->m_DemoBuffer) ;
#else

	// Play C File demo
	CAttractDemo__ConstructPlay((UINT8 *)CFileDemoBuffer) ;
#endif

	return TRUE ;

#else
	return FALSE ;

#endif
}

void CAttractDemo__Construct()
{
	CAttractDemo	*pThis = &AttractDemo ;

	// Setup ptr to demo list
	pThis->m_Active = FALSE ;
	pThis->m_Record = FALSE ;
	pThis->m_Play = FALSE ;
	pThis->m_FirstFrame = TRUE ;

	pThis->m_pCurrentDemo = RomBinaryDemoList ;		
	pThis->m_DemoRequested = TRUE ;
	pThis->m_DemoRecieved = FALSE ;

#if RECORD_ATTRACT_DEMO
	CAttractDemo__ConstructRecord() ;
#endif
}




#if RECORD_ATTRACT_DEMO
void CAttractDemo__UpdateRecord()
{
	CAttractDemo	*pThis = &AttractDemo ;

	// Loose precision for recording - saves lot's of bytes
	PlayerControllerData.stick_x >>= 8-ATTRACT_DATA_STICK_BITS ;
	PlayerControllerData.stick_x <<= 8-ATTRACT_DATA_STICK_BITS ;
	PlayerControllerData.stick_y >>= 8-ATTRACT_DATA_STICK_BITS ;
	PlayerControllerData.stick_y <<= 8-ATTRACT_DATA_STICK_BITS ;

	// All joypad direction buttons (run/walk) count as one button
	if (PlayerControllerData.button & (U_JPAD | D_JPAD | L_JPAD | R_JPAD))
	{
		PlayerControllerData.button &= ~(U_JPAD | D_JPAD | L_JPAD | R_JPAD) ;
		PlayerControllerData.button |= R_JPAD ;
	}

	// Keep nNextFields within 2 bits for smaller storage
	if (nNextFields < 2)
		nNextFields = 2 ;
	else
	if (nNextFields > 5)
		nNextFields = 5 ;
		
	// Setup defaults for first frame
	if (pThis->m_FirstFrame)
	{
		CAttractBuffer__UpdateValue(&pThis->m_ButtonBuffer) ;
		CAttractBuffer__UpdateValue(&pThis->m_StickXBuffer) ;
		CAttractBuffer__UpdateValue(&pThis->m_StickYBuffer) ;
		CAttractBuffer__UpdateValue(&pThis->m_NextFieldsBuffer) ;

		pThis->m_FirstFrame = FALSE ;
	}

	// Force writing on last frames data
	if (GetApp()->m_bPause)
	{
		CAttractBuffer__WriteValue(&pThis->m_ButtonBuffer) ;
		CAttractBuffer__WriteValue(&pThis->m_StickXBuffer) ;
		CAttractBuffer__WriteValue(&pThis->m_StickYBuffer) ;
		CAttractBuffer__WriteValue(&pThis->m_NextFieldsBuffer) ;

		AttractBuffer[1] = pThis->m_TotalFrames ;
		pThis->m_Active = FALSE ;
		pThis->m_Record = FALSE ;
		CAttractDemo__PrintData(pThis) ;

		// Put original options back
		COptions__Restore(&GetApp()->m_Options) ;
	}
	else
	{
		CAttractBuffer__UpdateRecord(&pThis->m_ButtonBuffer) ;
		CAttractBuffer__UpdateRecord(&pThis->m_StickXBuffer) ;
		CAttractBuffer__UpdateRecord(&pThis->m_StickYBuffer) ;
		CAttractBuffer__UpdateRecord(&pThis->m_NextFieldsBuffer) ;
	}

	// Next frame
	pThis->m_TotalFrames++ ;
	pThis->m_TotalTime += REFRESHES_TO_FRAME_INC((FLOAT)pThis->m_NextFieldsBuffer.m_Value, 60) ;

	CAttractDemo__UpdateChecksum() ;
}
#endif


void CAttractDemo__UpdatePlay()
{
	CAttractDemo	*pThis = &AttractDemo ;

	// First time ran - force read
	if (pThis->m_FirstFrame)
	{
		CAttractBuffer__ReadValue(&pThis->m_ButtonBuffer) ;
		CAttractBuffer__ReadValue(&pThis->m_StickXBuffer) ;
		CAttractBuffer__ReadValue(&pThis->m_StickYBuffer) ;
		CAttractBuffer__ReadValue(&pThis->m_NextFieldsBuffer) ;

		pThis->m_FirstFrame = FALSE ;
	}
	else
	{
		// End yet or start pressed?
		pThis->m_TotalTime += frame_increment ;
		if ((--pThis->m_TotalFrames <= 0) || (PlayerControllerData.button & START_BUTTON))
		{
			// Put original options back
			COptions__Restore(&GetApp()->m_Options) ;

			PlayerControllerData.button = 0 ;
			PlayerControllerData.stick_x = 0 ;
			PlayerControllerData.stick_y = 0 ;
			pThis->m_Active = FALSE ;
			pThis->m_Play = FALSE ;

			// Request next demo from rom
			CAttractDemo__RequestNewDemo() ;

			// Stop player from joining in demo!!
			GetApp()->m_CinemaFlags = -1 ;

			// Re-run front end
			CEngineApp__SetupFadeTo(GetApp(), MODE_RESETGAME) ;
		}
		else
		{
			CAttractBuffer__UpdatePlay(&pThis->m_ButtonBuffer) ;
			CAttractBuffer__UpdatePlay(&pThis->m_StickXBuffer) ;
			CAttractBuffer__UpdatePlay(&pThis->m_StickYBuffer) ;
			CAttractBuffer__UpdatePlay(&pThis->m_NextFieldsBuffer) ;
		}
	}

	// All joypad direction buttons (run/walk) count as one button
	if (pThis->m_ButtonBuffer.m_Value & (U_JPAD | D_JPAD | L_JPAD | R_JPAD))
	{
		pThis->m_ButtonBuffer.m_Value &= ~(U_JPAD | D_JPAD | L_JPAD | R_JPAD) ;
		pThis->m_ButtonBuffer.m_Value |= R_JPAD ;
	}


	CAttractDemo__UpdateChecksum() ;

#if 0
	osSyncPrintf("F:%d x:%f z:%f xvl:%f zvl:%f f_inc:%f\r\n",
				  game_frame_number,
				  CEngineApp__GetPlayer(GetApp())->ah.ih.m_vPos.x,	
				  CEngineApp__GetPlayer(GetApp())->ah.ih.m_vPos.z,
				  CEngineApp__GetPlayer(GetApp())->ah.m_vVelocity.x,	
				  CEngineApp__GetPlayer(GetApp())->ah.m_vVelocity.z,
				  frame_increment) ;
#endif

}


void CAttractDemo__Update()
{
	CAttractDemo	*pThis = &AttractDemo ;

	// Start record/playback?
	if ((!pThis->m_Finished) && (!pThis->m_Active) && 
		 ((pThis->m_Record) || (pThis->m_Play)) &&
		 (GetApp()->m_Mode == MODE_GAME))
	{

#if RECORD_ATTRACT_DEMO
		if (pThis->m_Record)
			osSyncPrintf("RECORDING ATTRACT DEMO - RANDOM(10000):%d!\r\n", RANDOM(10000)) ;
#endif

#if DEBUG_ATTRACT
		if (pThis->m_Play)
			osSyncPrintf("PLAYING ATTRACT DEMO - RANDOM(10000):%d!\r\n", RANDOM(10000)) ;
#endif
		pThis->m_Active = TRUE ;
	}


	// Record/playback?
	if ((pThis->m_Active) && (!pThis->m_Finished))
	{
#if RECORD_ATTRACT_DEMO
		// Update recording
		if (pThis->m_Record)
			CAttractDemo__UpdateRecord(pThis) ;
#endif

		// Update playing
		if (pThis->m_Play)
			CAttractDemo__UpdatePlay(pThis) ;

#if DEBUG_ATTRACT
		// Show last frame
		if ((!pThis->m_Active) || (pThis->m_Finished))
			osSyncPrintf("// End Frame:%d, TotalTime:%f Checksum:%d\r\n", 
						  pThis->m_PlayRecordFrame,
						  pThis->m_TotalTime / 15, pThis->m_Checksum) ;

#endif

		// Next frame
		pThis->m_PlayRecordFrame++ ;
	}
}





void CAttractDemo__CheckForUpdatingGameVars()
{
	CAttractDemo	*pThis = &AttractDemo ;

	// Active?
	if (pThis->m_Active)
	{
		// Recording?
		if (pThis->m_Record)
		{
			// Lose precision for recording
			PlayerControllerData.stick_x >>= 8-ATTRACT_DATA_STICK_BITS ;
			PlayerControllerData.stick_x <<= 8-ATTRACT_DATA_STICK_BITS ;
			PlayerControllerData.stick_y >>= 8-ATTRACT_DATA_STICK_BITS ;
			PlayerControllerData.stick_y <<= 8-ATTRACT_DATA_STICK_BITS ;
		}

		// Playing back?
		if (pThis->m_Play)
		{
			PlayerControllerData.button = pThis->m_ButtonBuffer.m_Value ;
			PlayerControllerData.stick_x = pThis->m_StickXBuffer.m_Value ; 
			PlayerControllerData.stick_y = pThis->m_StickYBuffer.m_Value ;  
			nNextFields = pThis->m_NextFieldsBuffer.m_Value ;
		}
	}
}

void CAttractDemo__EventDebug(void)
{
#if DEBUG_ATTRACT
	CAttractDemo	*pThis = &AttractDemo ;
	osSyncPrintf("Event!: AttractFrame:%d  GameFrame:%d Rand(10000):%d frm_inc %f\r\n", 
				  pThis->m_PlayRecordFrame, game_frame_number,
				  RANDOM(10000), frame_increment) ;

#endif

}


BOOL CAttractDemo__Active(void)
{
	CAttractDemo	*pThis = &AttractDemo ;
	return ((pThis->m_Record) || (pThis->m_Play)) ;
}

BOOL CAttractDemo__PlayingDemo(void)
{
	CAttractDemo	*pThis = &AttractDemo ;
	return (pThis->m_Play) ;
}


BOOL CAttractDemo__StartOfRecordingDemo(void)
{
	CAttractDemo	*pThis = &AttractDemo ;
	return ((pThis->m_Record) && (game_frame_number <= RECORD_NO_MOVE_FRAMES)) ;
}






/////////////////////////////////////////////////////////////////////////////
//
//
// CREATING FINAL BUFFER CODE	
//
//
/////////////////////////////////////////////////////////////////////////////

#if RECORD_ATTRACT_DEMO
INT32 CAttractBuffer__CopyBuffer(CAttractBuffer *pThis, CAttractBuffer *pSource)
{
	UINT32 Bits = pSource->m_CurrentBit ;
	pSource->m_CurrentBit = 0 ;
	while(Bits--)
		CAttractBuffer__WriteBit(pThis, CAttractBuffer__ReadBit(pSource)) ;
	return pSource->m_CurrentBit ;
}


void CAttractDemo__PrintHexByte(UINT8 Byte)
{
	static char DecimalToHex[] = "0123456789ABCDEF" ;
	static char SingleChar[] = "00\0" ;
	SingleChar[0] = DecimalToHex[Byte>>4] ;
	SingleChar[1] = DecimalToHex[Byte & 0xF] ;
	osSyncPrintf("%s", SingleChar) ;
}

void CAttractDemo__PrintData()
{
	CAttractDemo		*pThis = &AttractDemo ;
	CAttractBuffer		FinalBuffer ;
	CAttractHeader		*pHeader = (CAttractHeader *)AttractBuffer ;
	INT32					Bits = sizeof(CAttractHeader) * 8 ;
	INT32					Bytes, Longs, Col ;
	UINT32				*pData ;
	INT32					Length ;
	UINT8					*pOutput ;

	CAttractBuffer__Construct(&FinalBuffer, AttractBuffer + sizeof(CAttractHeader), 
									 0,
									 NULL,
									 NULL,
									 NULL) ;

	// Prepare header
	pHeader->m_WarpID = RECORD_WARP_ID ;
	pHeader->m_TotalFrames = pThis->m_TotalFrames ;
	osSyncPrintf("\r\n// DEMO WAS %d FRAMES", pThis->m_TotalFrames) ;

	// Merge all buffers into one

	// Copy button buffer
	pHeader->m_ButtonStartBit = Bits ;
	Length = CAttractBuffer__CopyBuffer(&FinalBuffer, &pThis->m_ButtonBuffer) ;
	Bits += Length ;
	osSyncPrintf("\r\n// ButtonBytes:%d\r\n", (Length/8) + 1) ;

	// Copy stick x buffer
	pHeader->m_StickXStartBit = Bits ;
	Length = CAttractBuffer__CopyBuffer(&FinalBuffer, &pThis->m_StickXBuffer) ;
	Bits += Length ;
	osSyncPrintf("// StickXBytes:%d\r\n", (Length/8) + 1) ;

	// Copy stick y buffer
	pHeader->m_StickYStartBit = Bits ;
	Length = CAttractBuffer__CopyBuffer(&FinalBuffer, &pThis->m_StickYBuffer) ;
	Bits += Length ;
	osSyncPrintf("// StickYBytes:%d\r\n", (Length/8) + 1) ;

	// Copy next fields buffer
	pHeader->m_NextFieldsStartBit = Bits ;
	Length = CAttractBuffer__CopyBuffer(&FinalBuffer, &pThis->m_NextFieldsBuffer) ;
	Bits += Length ;
	osSyncPrintf("// NextFieldsBytes:%d\r\n", (Length/8) + 1) ;

	// Calculate total data to print
	pData = (UINT32 *)AttractBuffer ;
	Bytes = (Bits / 8) + 1 ;
	Longs = (Bytes + 3) / 4 ;
	Col = 1 ;

	osSyncPrintf("// WholeDemo = %d Bytes\r\n", Longs * 4) ;
#if 0
	osSyncPrintf("\r\nUINT32 CFileDemoBuffer[] =\r\n{\r\n") ;
	while(Longs--)
	{
		if (Col == 1)
			osSyncPrintf("\t0x%x", *pData++) ;			// New line
		else
		if (Col < 6)
			osSyncPrintf(",0x%x", *pData++) ;			// Middle
		else
			osSyncPrintf(",0x%x,\r\n", *pData++) ;	// End of line

		// Was that the last one?
		if (Longs == 0)
			osSyncPrintf("\r\n} ;\r\n\r\n") ;

		if (++Col > 6)
			Col = 1 ;
	}
#endif

	// Output asc format
	osSyncPrintf("\r\nASC START - DON'T COPY THIS LINE\r\n\r\n") ;
	pOutput = (UINT8 *)AttractBuffer ;
	Bytes = (Bits / 8) + 1 ;
	Longs = (Bytes + 3) / 4 ;

	Bytes = Longs*4 ;
	Col = 0 ;
	while(Bytes--)
	{
		CAttractDemo__PrintHexByte(*pOutput++) ;
		if (++Col > 30)
		{
			osSyncPrintf("\r\n") ;
			Col = 0 ;
		}
	}
	osSyncPrintf("\r\n\r\nASC END - DON'T COPY THIS LINE\r\n") ;
}
#endif








