// scene.h
/////////////////////////////////////////////////////////////////////////////

#ifndef _INC_SCENE
#define _INC_SCENE

#include "defs.h"
#include "cart.h"
#include "rommodel.h"
#include "romstruc.h"
#include "particle.h"
#include "textload.h"
#include "simppool.h"
#include "pickup.h"

extern BYTE	light_table_red[];
extern BYTE	light_table_green[];
extern BYTE	light_table_blue[];

typedef struct CScene_t
{
	CCartCache				m_Cache;						// caching layer for cartridge data access

	ROMAddress				*m_rpBaseAddress;			// base address of data on cartridge

   CCacheEntry				*m_pceIndex;            // cartridge index data

   ROMAddress				*m_rpObjectsAddress,
								*m_rpTextureSetsAddress,
								*m_rpLevelsAddress,
								*m_rpLevelAddress,
								*m_rpGridSectionsAddress,
								*m_rpObjectTypesAddress,
								*m_rpObjectAttributesAddress,
								*m_rpParticleEffectsAddress,
								*m_rpGridBoundsAddress,
								*m_rpWarpPointsAddress,
								*m_rpBinaries;
   CCacheEntry				*m_pceObjectAttributes,
								*m_pceObjectTypes,
								*m_pceObjectsHeader,
								*m_pceObjectsIndex,
								*m_pceCollision,
								*m_pceInstances,
								*m_pceTextureSetsHeader,
								*m_pceTextureSetsIndex,
								*m_pceLevelsHeader,
								*m_pceLevelsIndex,
								*m_pceLevelIndex,
								*m_pceGridBounds,
								*m_pceGridSectionsIndex,
								*m_pcePersistantCounts,
								*m_pceSkyTextureSet,
								*m_pceWarpPoints,
								*m_pceSoundEffects,
								*m_pceLevelInfo,
								*m_pceBinariesIndex,
								*m_pceBinaryTypes;
	DWORD						m_ObjectTypesSize,
								m_ObjectAttributesSize,
								m_ParticleEffectsSize,
								m_GridBoundsSize,
								m_WarpPointsSize;

	CInstanceHdr			*m_pInstances[COLLISION_MAX_INSTANCES];
	int						m_nInstances;
	
	CGameStaticInstance	*m_pStaticEventsInstances[STATICEVENTS_MAX_INSTANCES];
	int						m_nStaticEventsInstances;

	CGameObjectInstance	*m_pPlayer;
	
	CGameObjectInstance	*m_pActiveAnimInstances[MAX_ACTIVE_ANIM_INSTANCES];
	int						m_nActiveAnimInstances;

	int						m_nTransparentInstances;
	CInstanceHdr			*m_pTransparentInstances[MAX_TRANSPARENT_INSTANCES];

	CParticleSystem		m_ParticleSystem;
	CSimplePool				m_SimplePool;

	CROMLevel				m_LevelInfo;
	signed short			m_LightDirection[4];

	CTextureLoader			m_SkyTextureLoader;
	BOOL						m_SkyTextureReady, m_SkyHeightSet,
								m_LevelInfoReceived;
	float						m_SkyHeight ;
	float						m_RotYPickup;
	CQuatern					m_qRotYPickup;
	int						m_nLevel,
								m_nWarpID;

	BOOL						m_WarpFound;
	CROMWarpPoint			m_WarpPoint;

	float						m_cSendStaticEventsFrame;

	DWORD						pad1, pad2, pad3, m_bPlayerActiveFlags;

	CPickupMonitor			m_PickupMonitor ;
} CScene;

// CScene operations
/////////////////////////////////////////////////////////////////////////////

void 				CScene__Construct(CScene *pThis, int nLevel);
void 				CScene__Destruct(CScene *pThis);
					
void 				CScene__Draw(CScene *pThis, Gfx **ppDLP);
BOOL				CScene__Idle(CScene *pThis);
int				CScene__LookupObjectType(CScene *pThis, int nType);
int				CScene__GetObjectTypeFlag(CScene *pThis, int nObjType);
void				CScene__LoadObjectModelType(CScene *pThis, CGameObjectInstance *pInstance,
														 int nObjectType, int nAnimType);
void				CScene__Decompress(CScene *pThis, CCacheEntry *pceData);

CROMRegionSet*	CScene__GetRegionAttributes(CScene *pThis, CGameRegion *pRegion);
int				CScene__GetRegionIndex(CScene *pThis, CGameRegion *pRegion);
CGameRegion		*CScene__GetRegionPtr(CScene *pThis, int Index) ;
int				CScene__DoSoundEffect(CScene *pThis,
												 int nSoundType, float Volume,
												 void *pSource, CVector3 *pvSourcePos, int SndFlags);
void				CScene__ResetPersistantData();
void				CScene__SetInstanceFlag(CScene *pThis, CGameObjectInstance *pAnim, BOOL Set);
BOOL				CScene__GetInstanceFlag(CScene *pThis, CGameObjectInstance *pAnim);
BOOL				CScene__GetInstanceFlagByIndex(CScene *pThis, int nInstance);
void				CScene__SetPickupFlag(CScene *pThis, CGameSimpleInstance *pSimple, BOOL Set);
BOOL				CScene__GetPickupFlag(CScene *pThis, CGameSimpleInstance *pSimple);
void				CScene__SetRegionFlag(CScene *pThis, int nRegion, BOOL Set);
BOOL				CScene__GetRegionFlag(CScene *pThis, int nRegion);
void				CScene__ClearAllRegionFlags(CScene *pThis);
struct CPersistantData_t* CScene__GetPersistantDataStruct(CScene *pThis);
void*				CScene__GetPersistantDataAddress(CScene *pThis);
int				CScene__GetPersistantDataSize(CScene *pThis);
void				CScene__PersistantDataLoaded(CScene *pThis);
BOOL				CScene__RequestBinaryBlock(CScene *pThis, int nBlockType,
														CCacheEntry **ppceBinaryBlock,
														void *pNotifyPtr, pfnCACHENOTIFY pfnBlockReceived);



CDynamicSimple *CScene__GeneratePickup(CScene *pThis, CGameRegion *pRegion, 
													CVector3 vPos, int nType, FLOAT Time) ;

CGameRegion* CScene__NearestRegion(CScene *pThis, CVector3 *pvPos) ; 

void CScene__SetupRealTimeLight(CScene *pThis) ;

/////////////////////////////////////////////////////////////////////////////
#endif // _INC_SCENE
