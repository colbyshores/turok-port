/*
control.h
control header
*/
#ifndef __CONTROLH
#define __CONTROLH






/* controller defines */
#define CCONTROL_MAX_BUTTONS	16				/* max no. of buttons (bit fields) */
#define CCONTROL_DBL_TIME		6				/* time second press has to come within for a double press to be registered */
#define CCONTROL_SMOOTH_STICK	2				/* standard stick smooth value */


/* controller states */
typedef struct CControlState_t
{
	u16	BDown,									/* button is down (always set if button is down) */
			BUp,										/* button is up (flags is only set for one cycle - after first cycle of release button may be up but it will not be registered)  */
			BSingle,									/* button is pressed once (button could be held but will only be registered once) */
			BDbl,										/* button is pressed twice consequtively (within timer count defined below) */
			BDblHold,								/* button is presses twice consequtively & then held on second press (within timer count defined below) */
			BTimer [ CCONTROL_MAX_BUTTONS ];	/* timer counts for each button */

	s8		StickX,									/* analogue stick x position -128 to +127 */
			StickY;									/* analogue stick y position -128 to +127 */

	/* smooth stick variables */
	float	Smooth,
			SStickX,
			SStickY;

} CControlState;





/* public variable external declarations */
extern CControlState	ControlStates;				/* controller info's */
extern u8			validcontrollers; 
extern u8			validpaks;


extern	OSContStatus  statusdata[];
extern	OSContPad     controllerdata[];
extern	OSContPad     PlayerControllerData ;



extern OSPfs		ControllerPak ;



extern	OSPfsState		FileState ;
extern	s32				FilesUsed, MaxFiles ;
extern	s32				TurokFiles ;
extern	s32				FreePages ;


typedef		struct s_File
{
	int				index ;
	char				string[20] ;
	int				pages ;
} t_File ;

extern	t_File		FileDirectory[] ;


/* external declarations */
//int		InitControllers			( void );
//u16		ReadController				( int ControllerSlot );
int		InitControllersAdvanced	( void );		  			/* initializes controller data */

void		ReadPlayerController(void) ;
void		ProcessPlayerController(void) ;
void		ReadControllerAdvanced	( void );		  			/* reads standard controller but calculates more information such as (double tapping etc.) */

int		SetupFileSystem	(void) ;
int		GetDirectoryInfo	(void) ;
int		GetFileState		(int file) ;
int		ScanFileList		(BOOL showall) ;

int		DeleteCurrent(int index) ;
int		LoadCurrent(int index) ;
int		SaveNew(void) ;
int		SaveCurrent(int index) ;


#endif		/* __CONTROLH */


