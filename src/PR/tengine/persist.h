// persist.h

#ifndef _INC_PERSIST
#define _INC_PERSIST

/////////////////////////////////////////////////////////////////////////////

#define PERSISTANT_DATA_MAX_SIZE		7000
//#define PERSISTANT_DATA_MAX_SIZE		7680
//#define PERSISTANT_DATA_MAX_SIZE		1024

extern	u32				CrashId ;
extern	u32				CrashMode ;
extern	u32				CrashFrame ;
extern	CROMWarpPoint	CrashWarp;
extern BYTE persistant_data[];
/////////////////////////////////////////////////////////////////////////////
#endif // _INC_PERSIST
