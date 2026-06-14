// map.h
/////////////////////////////////////////////////////////////////////////////

#ifndef _INC_MAP
#define _INC_MAP

#include <stddef.h>
#include "spec.h"
#include "defs.h"
#include "scene.h"
#include "graphu64.h"
#include "romstruc.h"

void			CEngineApp__DrawMap(CEngineApp *pThis);
void			CEngineApp__RevealMap(CEngineApp *pThis);
void			CEngineApp__SetupMap(CEngineApp *pThis);
BOOL			CEngineApp__DrawingMap(CEngineApp *pThis) ;
//void			CEngineApp__RevealAllMap(CEngineApp *pThis) ;

enum MapStatus
{
	MAP_NULL,
	MAP_FADEUP,
	MAP_ON,
	MAP_FADEDOWN
} ;

/////////////////////////////////////////////////////////////////////////////
#endif // _INC_MAP

