// control.c

#include "stdafx.h"
#include <ramrom.h>
#include "tengine.h"
#include "control.h"
#include "attract.h"


//  Variables shared with other files
u8    validcontrollers = 0; 
u8    validpaks = 0; 

/**** variables used by this file for handling the controllers ****/
OSContStatus  statusdata[MAXCONTROLLERS];
OSContPad     controllerdata[MAXCONTROLLERS];
OSScMsg       controllermsg;

OSContPad     PlayerControllerData ;


// public variables
CControlState			ControlStates;


// global handle for controller pak file system
OSPfs		ControllerPak ;

OSMesgQueue    serialMsgQ;
OSMesg         serialMsg;

OSPfsState		FileState ;
s32				FilesUsed, MaxFiles ;
s32				TurokFiles, ActualTurokFiles ;
s32				LowestAvailableTurokFile ;
s32				FreePages ;

t_File		FileDirectory[16] ;


u16				CompanyCode	  = '51' ;
u32				GameCode		  = 'NTUE' ;
//char				GameName[16]  = {'t', 'u', 'r', 'o', 'k', 0,0,0,0,0,0,0,0,0,0,0} ;
char				GameName[16]  = {0x2d, 0x2e, 0x2b, 0x28, 0x24, 0,0,0,0,0,0,0,0,0,0,0} ;
char				ExtName[4]    = {0,0,0,0} ;

#ifndef GERMAN
char			NoGamesString[8] = {0x0d, 0x1e, 0x26, 0x29, 0x2d, 0x32, 0x0e, 0} ;
#else
char			NoGamesString[8] = {0x0d, 0x25, 0x1e, 0x1e, 0x2b, 0x0e, 0} ;
#endif






/**********************************************************************
 * Update the controller data
 *********************************************************************/
void UpdateController()
{
	osContGetReadData(controllerdata);
}

/**********************************************************************
 *
 * Routine for initializing the controllers. After initialized, set the
 * controller interrupt to be posted to the gfxFrameMsgQ, used by the 
 * gameproc in tengine.c.
 *
 *********************************************************************/
/*
initializes controller data
returns the lowest numbered controller connected to the system
*/
int InitControllersAdvanced ( void )
{
	/* declare variables */
	//OSMesgQueue    serialMsgQ;
	//OSMesg         serialMsg;

	u16				i, cnt;
	u16				controller ;

	/* setup controllers */
	osCreateMesgQueue(&serialMsgQ, &serialMsg, 1);
	osSetEventMesg(OS_EVENT_SI, &serialMsgQ, (OSMesg)1);
	osContInit(&serialMsgQ, &validcontrollers, &statusdata[0]) ;

//	if (osContInit(&serialMsgQ, &validcontrollers, &statusdata[0]) == 0)
	if (validcontrollers)
	{
		controller = -1 ;
		for (i=0; i < MAXCONTROLLERS; i++)
		{
			// find lowest controller attached
			if ((validcontrollers & (1<<i)) && !(statusdata[i].errno & CONT_NO_RESPONSE_ERROR))
			{
				controller = i ;
				break ;
			}
		}

		controllermsg.type = CONTROLLER_MSG;
		osSetEventMesg(OS_EVENT_SI, &gfxFrameMsgQ, (OSMesg)&controllermsg);
		
		/* reset all variables for all controllers */
		ControlStates.BUp     = 0;
		ControlStates.BDown   = 0;
		ControlStates.BSingle = 0;
		ControlStates.BDbl    = 0;
		ControlStates.StickX  = 0;
		ControlStates.StickY  = 0;
		ControlStates.SStickX = 0;
		ControlStates.SStickY = 0;
		ControlStates.Smooth  = CCONTROL_SMOOTH_STICK;

		for ( cnt = 0; cnt < CCONTROL_MAX_BUTTONS; cnt ++ )
			ControlStates. BTimer[cnt] = CCONTROL_DBL_TIME+1;

		return controller ;
	}

	TRACE0("Failure initing controllers\n");
	return -1;
}





// Reads hardware controller values into player controller
void ReadPlayerController(void)
{
	/* valid controller present */
	if (Controller < 0)
	{
		PlayerControllerData.button = 0 ;
		PlayerControllerData.stick_x = 0 ;
		PlayerControllerData.stick_y = 0 ;
		TRACE0( "no controller connection\n" );
		return;
	}

	// Copy controller vars to player controller vars
	PlayerControllerData = controllerdata[Controller] ;

	// Stop misc presses during very start of demo
	if (CAttractDemo__StartOfRecordingDemo())
	{
		PlayerControllerData.button = 0 ;
		PlayerControllerData.stick_x = 0 ;
		PlayerControllerData.stick_y = 0 ;
	}
}


// Acts upon player controller
void ProcessPlayerController(void)
{
	/* declare variables */
	u16			butprev, cnt, mask;
	s8				sx, sy, lsx, lsy;
	float			dx, dy,
					scalerh,
					scalerv,
					nx, ny;

	/* store information */
	butprev = ControlStates.BDown;
	ControlStates.BDown   = PlayerControllerData.button;
	ControlStates.BUp     =   butprev  & (~ControlStates.BDown);
	ControlStates.BSingle = (~butprev) &   ControlStates.BDown;

	/* make sure stick never does -128 */
	sx = PlayerControllerData.stick_x;
	sy = PlayerControllerData.stick_y;
	if ( sx == -128 ) sx = -127;
	if ( sy == -128 ) sy = -127;

#define	ANALOG_SCALER_TEST
#ifdef ANALOG_SCALER_TEST

	scalerh = ((float)GetApp()->m_Options.m_HAnalog) / 128.0;
	scalerv = ((float)GetApp()->m_Options.m_VAnalog) / 128.0;

	if (scalerh < (20.0/128.0))
		scalerh = 20.0/128.0;

	if (scalerv < (20.0/128.0))
		scalerv = 20.0/128.0;

	nx = ((float)sx) * scalerh;
	ny = ((float)sy) * scalerv;

	if (nx < -80.0) nx = -80.0;
	if (nx > +80.0) nx = +80.0;
	if (ny < -80.0) ny = -80.0;
	if (ny > +80.0) ny = +80.0;

	dy = min(80.0, 80.0*scalerv);
	ny = (ny * 80) / dy;

	ControlStates.StickX = ((s8)nx);
	ControlStates.StickY = ((s8)ny);

#else

	ControlStates.StickX = sx;
	ControlStates.StickY = sy;

#endif

	/* calculate smooth stick movement */
	sx = ControlStates.StickX;
	sy = ControlStates.StickY;



	lsx = ControlStates.SStickX;
	lsy = ControlStates.SStickY;
	ASSERT ( ControlStates.Smooth );				/* smooth should never be below 1 */
	dx = ((float) (sx - lsx) ) / ControlStates.Smooth;
	dy = ((float) (sy - lsy) ) / ControlStates.Smooth;
	ControlStates.SStickX += dx;
	ControlStates.SStickY += dy;


	/* update double press counters & flags */
	ControlStates.BDblHold |= ControlStates.BDbl;
	ControlStates.BDblHold &= ControlStates.BDown;
	ControlStates.BDbl   = 0;
	mask = 0x0001;

	for ( cnt = 0; cnt < CCONTROL_MAX_BUTTONS; cnt ++ )
	{
		/* single press of a button ? */
		if ( ControlStates.BSingle & mask )
		{
			/* single button press - was it a double press ? */
			if ( ControlStates.BTimer [ cnt ] <= CCONTROL_DBL_TIME )
			{
				/* button was a double press */
				ControlStates.BDbl           |= mask;
				ControlStates.BTimer [ cnt ]  = CCONTROL_DBL_TIME + 1;
			}
			else
				ControlStates.BTimer [ cnt ] = 0;		/* button was just a single press */
		}

		/* update double press counter */
		ControlStates.BTimer [ cnt ] ++;

		/* check next button */
		mask = mask << 1;
	}
}




/*
reads standard controller but calculates more information such as (double tapping etc.)
*/
void ReadControllerAdvanced (void)
{
	ReadPlayerController() ;
	ProcessPlayerController() ;
}




/*****************************************************************************
*
*	Function:		StartSerialQ() & EndSerialQ()
*
*****************************************************************************/
void StartSerialQ(void)
{
	osCreateMesgQueue(&serialMsgQ, &serialMsg, 1);
	osSetEventMesg(OS_EVENT_SI, &serialMsgQ, (OSMesg)1);
	osContInit(&serialMsgQ, &validcontrollers, &statusdata[0]) ;

//	cntrlReadYet = FALSE ;
//	cntrlReadInProg = FALSE ;
}

void EndSerialQ(void)
{
	// put SI association back
	osSetEventMesg(OS_EVENT_SI, &gfxFrameMsgQ, (OSMesg)&controllermsg);

	//osContStartReadData(&gfxFrameMsgQ);
	//osContGetReadData(controllerdata);
}


/*****************************************************************************
*
*	Function:		SetupFileSystem()
*
******************************************************************************
*
*	Description:	Initialize (if present) the memory pack connected to the
*						lowest numbered controller(the players controller!)
*
*	Note:				Temporarily re-routes the SI message to a new Q
*
*	Inputs:			none
*	Outputs:			error_numb (0 if ok)
*
*****************************************************************************/
int SetupFileSystem(void)
{
	int	ret ;

	StartSerialQ() ;
	ret = osPfsIsPlug(&serialMsgQ, &validpaks) ;

	// call was a success, continue
	if (ret ==0)
	{
		// does the current controller have a memory pak?
		if (validpaks & (1<<Controller))
			ret = osPfsInit(&serialMsgQ, &ControllerPak, Controller) ;
		else
			validpaks = 0 ;
	}

	EndSerialQ() ;
	return ret ;
}

/*****************************************************************************
*
*	Function:		GetDirectoryInfo()
*
******************************************************************************
*
*	Description:	Read the number of files in use, total files available,
*						and pages free
*
*	Note:				Temporarily re-routes the SI message to a new Q
*
*	Inputs:			none
*	Outputs:			error_numb (0 if ok)
*
*****************************************************************************/
int GetDirectoryInfo(void)
{
	int	ret ;
	s32	bytes ;

	StartSerialQ() ;
	ret = osPfsNumFiles(&ControllerPak, &MaxFiles, &FilesUsed) ;

	if (ret == 0)
	{
		ret = osPfsFreeBlocks(&ControllerPak, &bytes) ;
		if (ret == 0)
			FreePages = bytes / 256 ;
	}
//	else
//		rmonPrintf("numfiles ret:%d\n", ret) ;


	TurokFiles = 0;

	EndSerialQ() ;
	return ret ;
}


/*****************************************************************************
*
*	Function:		GetFileState()
*
******************************************************************************
*
*	Description:	Setup the global FileState struc
*
*	Note:				Temporarily re-routes the SI message to a new Q
*
*	Inputs:			file number
*	Outputs:			error_numb (0 if ok)
*
*****************************************************************************/
int GetFileState(int file)
{
	int	ret ;

	StartSerialQ() ;
	ret = osPfsFileState(&ControllerPak, file, &FileState) ;

	EndSerialQ() ;
	return ret ;
}



// read only the turok files (the first 16!)
// to find the lowest available turok file, uses a word
// and sets bit 0 for A, 1 for B etc, then finds the
// lowest unused bit.
int ScanFileList(BOOL showall)
{
	int	ret, i, c ;
	BOOL	Turok ;
	u16	available ;

	ret = GetDirectoryInfo() ;

	switch (ret)
	{
		case PFS_ERR_NEW_PACK:
			SetupFileSystem() ;
			ScanFileList(showall) ;			// potential for infinite loop, but if nintendo did it right, it shouldn't happen
			break ;

		case PFS_ERR_NOPACK:
			break ;
		case PFS_ERR_CONTRFAIL:
			break ;
		case PFS_ERR_INVALID:
			sprintf(FileDirectory[0].string, "error") ;
			FilesUsed = 0 ;
			TurokFiles = 0 ;
			break ;
	}


	if (ret == 0)
	{
		available = 0 ;
		TurokFiles = 0 ;
		ActualTurokFiles = 0 ;
		LowestAvailableTurokFile = 0x1a ;	// 'a' in nintendo font
		for (i=0; i<MaxFiles; i++)
		{
			Turok = FALSE ;

			ret = GetFileState(i) ;
			if (ret == 0)
			{
				// TUROK file
				if ((FileState.company_code == CompanyCode) && (FileState.game_code == GameCode))
				{
					available |= 1<< (FileState.ext_name[0] - 0x1a) ;
//					if (FileState.ext_name[0] == LowestAvailableTurokFile)
//						LowestAvailableTurokFile++ ;

					ActualTurokFiles++ ;
					Turok = TRUE ;
				}
				
				if ((showall) || (Turok))
				{
					FileDirectory[TurokFiles].index = i ;
					FileDirectory[TurokFiles].pages = FileState.file_size/256 ;
					// copy name
					for (c=0; c<16; c++)
						FileDirectory[TurokFiles].string[c] = (FileState.game_name[c] !=0) ? FileState.game_name[c] : 1 ;

					// space & first letter of extension
					FileDirectory[TurokFiles].string[16] = 1 ;
					FileDirectory[TurokFiles].string[17] = FileState.ext_name[0] ;
					FileDirectory[TurokFiles].string[18] = 0 ;

					TurokFiles++ ;
					if (TurokFiles>15)
						break ;
				}
			}
			else if (showall)
			{
				FileDirectory[TurokFiles].index = i ;
				FileDirectory[TurokFiles].pages = 0 ;
				memcpy(&FileDirectory[TurokFiles].string, NoGamesString, 8) ;
				TurokFiles++ ;
				if (TurokFiles>15)
					break ;
			}
		}

		// find lowest numbered available file...
		for (i=0; i<MaxFiles; i++)
		{
			if ((available & (1<<i)) == 0)
			{
				LowestAvailableTurokFile = 0x1a + i ;
				break ;
			}
		}

		if (TurokFiles == 0)
			memcpy(&FileDirectory[0].string, NoGamesString, 8) ;
	}
	return ret ;
}


// delete the file at the specified index into FILEDIRECTORY
int DeleteCurrent(int index)
{
	int	ret ;

	StartSerialQ() ;

	ret = osPfsFileState(&ControllerPak, FileDirectory[index].index, &FileState) ;

	if (ret == 0)
	{
		osPfsDeleteFile(&ControllerPak,
							 FileState.company_code,
							 FileState.game_code,
							 FileState.game_name,
							 FileState.ext_name) ;
	}
//	rmonPrintf("delete ret:%d\n", ret) ;

	EndSerialQ() ;
	return ret ;
}


// load the file at the specified index into FILEDIRECTORY
int LoadCurrent(int index)
{
	int	ret ;
	s32	size ;
	u8		*data ;

	StartSerialQ() ;

	size = CScene__GetPersistantDataSize(&GetApp()->m_Scene) ;
	data = (u8 *)CScene__GetPersistantDataAddress(&GetApp()->m_Scene) ;

	size /= 256 ;
	size++ ;
	size *= 256 ;

	ret = osPfsFileState(&ControllerPak, FileDirectory[index].index, &FileState) ;
	if (ret == 0)
	{
		// check if turok game
		if ((FileState.company_code == CompanyCode) && (FileState.game_code == GameCode) && (FileState.file_size == size))
		{
			//osWritebackDCache(data, FileState.file_size);
			ret = osPfsReadWriteFile(&ControllerPak,				// *pfs
								 FileDirectory[index].index,	// file number
								 PFS_READ,							// flag
								 0,									// start offset
								 FileState.file_size,			// size
								 data);								// area to read to
			//osInvalDCache(data, FileState.file_size);
	 
			//rmonPrintf("actual read ret:%d\n", ret);
			//rmonPrintf("reading %d bytes from %s\n", FileState.file_size, FileDirectory[index].string);

			CScene__PersistantDataLoaded(&GetApp()->m_Scene) ;
		}
		else
			ret = -1 ;
	}

	//rmonPrintf("load ret:%d\n", ret) ;

	EndSerialQ() ;
	return ret ;
}

// save a new file
int SaveNew(void)
{
	int	ret ;
	s32	size ;
	u8		*data ;
	s32	filenumb = 0 ;

	StartSerialQ() ;

	size = CScene__GetPersistantDataSize(&GetApp()->m_Scene) ;
	data = (u8 *)CScene__GetPersistantDataAddress(&GetApp()->m_Scene) ;

	size /= 256 ;
	size++ ;
	size *= 256 ;

	ExtName[0] = LowestAvailableTurokFile ;
	//rmonPrintf("attempting to create turok %.4s\n", ExtName) ;
	ret = osPfsAllocateFile(&ControllerPak, CompanyCode, GameCode, GameName, ExtName, size, &filenumb) ;

	if (ret == 0)
	{
		//rmonPrintf("allocation ok, new file:%d\n", filenumb) ;
		//osWritebackDCache(data, size);
		ret = osPfsReadWriteFile(&ControllerPak,				// *pfs
							 filenumb,							// freshly allocated file number
							 PFS_WRITE,							// flag
							 0,									// start offset
							 size,								// size
							 data);								// area to write from
		//rmonPrintf("actual save ret:%d\n", ret);
		//rmonPrintf("writing %d bytes to %5s%4s\n", size, GameName, ExtName);
	}
//	else
//		rmonPrintf("save new ret:%d\n", ret) ;

	EndSerialQ() ;
	return ret ;
}

// save the file to the specified index into FILEDIRECTORY
int SaveCurrent(int index)
{
	int	ret ;
	s32	size ;
	u8		*data ;

	StartSerialQ() ;

	size = CScene__GetPersistantDataSize(&GetApp()->m_Scene) ;
	data = (u8 *)CScene__GetPersistantDataAddress(&GetApp()->m_Scene) ;

	size /= 256 ;
	size++ ;
	size *= 256 ;

	ret = osPfsFileState(&ControllerPak, FileDirectory[index].index, &FileState) ;
	if (ret == 0)
	{
		// check if turok game
		if ((FileState.company_code == CompanyCode) && (FileState.game_code == GameCode) && (FileState.file_size == size))
		{
			//osWritebackDCache(data, size);
			ret = osPfsReadWriteFile(&ControllerPak,				// *pfs
								 FileDirectory[index].index,	// file number
								 PFS_WRITE,							// flag
								 0,									// start offset
								 size,								// size
								 data);								// area to write from
			//rmonPrintf("actual save ret:%d\n", ret);
			//rmonPrintf("writing %d bytes to %5s%4s\n", FileState.file_size, FileState.game_name, FileState.ext_name);
		}
		else
		{
			//rmonPrintf("filesize:%d  datasize:%d\n", FileState.file_size, size);

			ret = -1 ;
		}
	}
//	else
//		rmonPrintf("save current ret:%d\n", ret) ;

	EndSerialQ() ;
	return ret ;
}



