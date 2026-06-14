// persist.cpp

#include "stdafx.h"
#include "romstruc.h"
#include "persist.h"

u32					CrashId ;
u32					CrashMode ;
u32					CrashFrame ;
CROMWarpPoint		CrashWarp;
BYTE persistant_data[PERSISTANT_DATA_MAX_SIZE];
