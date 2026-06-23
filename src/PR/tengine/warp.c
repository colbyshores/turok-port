// warp.cpp

#include "cppu64.h"
#include "tengine.h"
#include "tmove.h"
#include "textload.h"
#include "romstruc.h"
#include "warp.h"
#include "sfx.h"


/****************************************************************************
* Defines
****************************************************************************/

/****************************************************************************
* Functions
****************************************************************************/
void CWarp__Warp(CGameSimpleInstance *pWarp)
{
	BOOL						returnWarp;
	CVector3					vPrevPos;
	CGameRegion				*pPrevRegion;
	float						prevRotY;
	CGameObjectInstance	*pPlayer;
	CVector3					vDelta, vDesired, vFinal;
	CGameRegion				*pFinal;

	switch (CInstanceHdr__TypeFlag(&pWarp->ah.ih))
	{
		case AI_OBJECT_WARP_DYNAMIC:
			if (    (!(pWarp->m_wFlags & SIMPLE_FLAG_GONE))
				  && (pWarp->ah.ih.m_pEA->m_Id > 0)
				  && (pWarp->m_wFlags & SIMPLE_FLAG_VISIBLE) )
			{
				if (pWarp->ah.ih.m_pEA->m_wTypeFlags3 & AI_TYPE3_RETURNWARP)
				{
					CEngineApp__WarpReturn(GetApp());
				}
				else
				{
					returnWarp = pWarp->ah.ih.m_pEA->m_dwTypeFlags2 & AI_TYPE2_STOREWARPRETURN;

					// store return point away from warp
					if (returnWarp)
					{
						pPlayer = CEngineApp__GetPlayer(GetApp());
						ASSERT(pPlayer);

						vPrevPos = pPlayer->ah.ih.m_vPos;
						pPrevRegion = pPlayer->ah.ih.m_pCurrentRegion;
						prevRotY = pPlayer->m_RotY;

#define	BACKOFF_WARP_DISTANCE	(12*SCALING_FACTOR)

						vDesired.x = pWarp->ah.ih.m_vPos.x + sin(pWarp->m_RotAngle)*-BACKOFF_WARP_DISTANCE;
						vDesired.y = pPlayer->ah.ih.m_vPos.y;
						vDesired.z = pWarp->ah.ih.m_vPos.z + cos(pWarp->m_RotAngle)*-BACKOFF_WARP_DISTANCE;

						CVector3__Subtract(&vDelta, &pWarp->ah.ih.m_vPos, &vDesired);
						pPlayer->m_RotY = CVector3__XZAngle(&vDelta);

						CInstanceHdr__GetNearPositionAndRegion(&pWarp->ah.ih, vDesired, &vFinal, &pFinal, INTERSECT_BEHAVIOR_SLIDE, INTERSECT_BEHAVIOR_IGNORE);

						pPlayer->ah.ih.m_vPos = vFinal;
						pPlayer->ah.ih.m_pCurrentRegion = pFinal;
					}

					// make player warp to the correct destination
					CEngineApp__Warp(GetApp(),
										  pWarp->ah.ih.m_pEA->m_Id,
										  WARP_WITHINLEVEL,
										  returnWarp);
					if (returnWarp)
					{
						pPlayer->ah.ih.m_vPos = vPrevPos;
						pPlayer->ah.ih.m_pCurrentRegion = pPrevRegion;
						pPlayer->m_RotY = prevRotY;
					}
				}
			}
			break;
	}
}

/*
void CWarp__Warp(CGameSimpleInstance *pWarp)
{
	CGameObjectInstance	*pPlayer;
	float						mag;
	CVector3					vDelta, vDesired, vFinal;
	CGameRegion				*pFinal;

	switch (CInstanceHdr__TypeFlag(pWarp))
	{
		case AI_OBJECT_WARP_DYNAMIC:
			if (    (!(pWarp->m_wFlags & SIMPLE_FLAG_GONE))
				  && (pWarp->ah.ih.m_pEA->m_Id > 0)
				  && (pWarp->m_wFlags & SIMPLE_FLAG_VISIBLE) )
			{
				// back player up a bit before warping
#define	BACKOFF_WARP_DISTANCE	(6*SCALING_FACTOR)

				pPlayer = CEngineApp__GetPlayer(GetApp());
				if (pPlayer)
				{
					vDelta.x = pPlayer->ah.ih.m_vPos.x - pWarp->ah.ih.m_vPos.x;
					vDelta.y = 0;
					vDelta.z = pPlayer->ah.ih.m_vPos.z - pWarp->ah.ih.m_vPos.z;

					mag = sqrt(vDelta.x*vDelta.x + vDelta.z*vDelta.z);
					if (mag != 0)
					{
						CVector3__MultScaler(&vDelta, &vDelta, BACKOFF_WARP_DISTANCE/mag);
						CVector3__Add(&vDesired, &pPlayer->ah.ih.m_vPos, &vDelta);

						CInstanceHdr__GetNearPositionAndRegion(&pPlayer->ah.ih, &vDesired, &vFinal, &pFinal);

						pPlayer->ah.ih.m_vPos = vFinal;
						pPlayer->ah.ih.m_pCurrentRegion = pFinal;
					}
				}

				if (pWarp->ah.ih.m_pEA->m_wTypeFlags3 & AI_TYPE3_RETURNWARP)
				{
					CEngineApp__WarpReturn(GetApp());
				}
				else
				{
					// make player warp to the correct destination
					CEngineApp__Warp(GetApp(),
										  pWarp->ah.ih.m_pEA->m_Id,
										  WARP_WITHINLEVEL,
										  pWarp->ah.ih.m_pEA->m_dwTypeFlags2 & AI_TYPE2_STOREWARPRETURN);
				}
			}
			break;
	}
}
*/
