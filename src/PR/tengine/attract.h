// attract.h
/////////////////////////////////////////////////////////////////////////////

#ifndef _INC_ATTRACT
#define _INC_ATTRACT

#include "defs.h"

/////////////////////////////////////////////////////////////////////////////
// Functions prototypes
/////////////////////////////////////////////////////////////////////////////

void CAttractDemo__Construct() ;
BOOL CAttractDemo__Play(void) ;
BOOL CAttractDemo__CheckForPlay() ;
BOOL CAttractDemo__CheckForRecord() ;
BOOL CAttractDemo__Active(void) ;

BOOL CAttractDemo__PlayingDemo(void) ;
BOOL CAttractDemo__StartOfRecordingDemo(void) ;

void CAttractDemo__Update() ;
void CAttractDemo__CheckForUpdatingGameVars() ;
void CAttractDemo__EventDebug() ;

void CAttractDemo__RequestNewDemo(void) ;
void CAttractDemo__CheckForNewDemoRequest(void) ;

/////////////////////////////////////////////////////////////////////////////
#endif // _INC_ATTRACT
