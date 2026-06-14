// turret.h

#ifndef _INC_TURRET
#define _INC_TURRET

/////////////////////////////////////////////////////////////////////////////

#include "aistruc.h"
#include "romstruc.h"
#include "graphu64.h"


/////////////////////////////////////////////////////////////////////////////
// Function prototypes
/////////////////////////////////////////////////////////////////////////////

void CeilingTurret_TransformPos(CGameObjectInstance *pThis, CVector3 *pvPos, CVector3 *pvWorld) ;
void CeilingTurretAI_Advance(CGameObjectInstance *pThis) ;

void BunkerTurret_TransformPos(CGameObjectInstance *pThis, CVector3 *pvPos, CVector3 *pvWorld) ;
void BunkerTurretAI_Advance(CGameObjectInstance *pThis) ;


/////////////////////////////////////////////////////////////////////////////
#endif	// _INC_TURRET
