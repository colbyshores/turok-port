// scene.c

#include "cppu64.h"
#include "scene.h"
#include "cartdir.h"
#include "tengine.h"
#include "mattable.h"
#include "scaling.h"
#include "audio.h"
#include "sfx.h"
#include "audio.h"
#include "audiocfx.h"
#include "regtype.h"
#include "tmove.h"
#include "qsort.h"
#include "dlists.h"
#include "sun.h"
#include "cammodes.h"
#include "volume.h"
#include "boss.h"
#include "bossflgs.h"
#include "persist.h"

// globals

BOOL persistant_data_loaded = FALSE;

BYTE	light_table_red[64];
BYTE	light_table_green[64];
BYTE	light_table_blue[64];

CGameRegion *p_regions;

// CScene implementation
/////////////////////////////////////////////////////////////////////////////

void		CScene__BuildInstanceCollisionList(CScene *pThis, CUnindexedSet *pusAnimInstances);
void		CScene__DrawParticles(CScene *pThis, Gfx **ppDLP);
void 		CScene__DrawSky(CScene *pThis, Gfx **ppDLP);
void 		CScene__DrawInstances(CScene *pThis, Gfx **ppDLP, CUnindexedSet *pusInstances);
void 		CScene__DrawEnvironment(CScene *pThis, Gfx **ppDLP);
void		CScene__DrawTransparentInstances(CScene *pThis, Gfx **ppDLP);
void		CScene__PreDrawSelf(CScene *pThis, CUnindexedSet *pusInstances, Gfx **ppDLP);
void		CScene__DrawSelf(CScene *pThis, Gfx **ppDLP, CUnindexedSet *pusInstances);
void 		CScene__RequestLevel(CScene *pThis, int nLevel);
void		CScene__SendStaticEvents(CScene *pThis);
BOOL		CScene__BetterSelfClipping(CScene *pThis, CGameObjectInstance *pSelf);
int		CScene__FindGridSection(CScene *pThis, CCacheEntry **ppceGridSection);
//BYTE*		CScene__GetGridPickupFlags(CScene *pThis, int nGridSection);
#define	CScene__AddTransparentInstance(pThis, pInst) if ((pThis)->m_nTransparentInstances != MAX_TRANSPARENT_INSTANCES) (pThis)->m_pTransparentInstances[(pThis)->m_nTransparentInstances++] = (pInst)
#define	CScene__AddStaticEventsInstance(pThis, pInst) if ((pThis)->m_nStaticEventsInstances != STATICEVENTS_MAX_INSTANCES) (pThis)->m_pStaticEventsInstances[(pThis)->m_nStaticEventsInstances++] = (pInst)
#define	CScene__AddActiveAnimInstance(pThis, pInst) if ((pThis)->m_nActiveAnimInstances != MAX_ACTIVE_ANIM_INSTANCES) (pThis)->m_pActiveAnimInstances[(pThis)->m_nActiveAnimInstances++] = (pInst)
void		CScene__GetGridSections(CScene *pThis, CBoundsRect *pBoundsRect,
											CGameGridBounds **ppGrids[], int *pIndices[], int *pnGrids);
void		CScene__RequestWarpPoints(CScene *pThis);
int		CScene__GetAnimInstanceIndex(CScene *pThis, CGameObjectInstance *pAnim);
BYTE*		CScene__GetPersistantBitstream(CScene *pThis, int nLevel, int nBitstream);
BYTE*		CScene__GetRegionBitstream(CScene *pThis);
void		CScene__SetUpActiveFlags(CScene *pThis, CUnindexedSet *pusAnimInstances);
#define	CScene__IsActive(pThis, pInst) ((pThis)->m_bPlayerActiveFlags & ((CInstanceHdr*)(pInst))->m_bActiveFlags)

// callbacks
void 		CScene__CartridgeIndexReceived(CScene *pThis, CCacheEntry **ppceTarget);
void		CScene__PersistantCountsReceived(CScene *pThis, CCacheEntry **ppceTarget);
void		CScene__WarpPointsReceived(CScene *pThis, CCacheEntry **ppceTarget);
void		CScene__BinaryTypesReceived(CScene *pThis, CCacheEntry **ppceTarget);
void		CScene__ObjectTypesReceived(CScene *pThis, CCacheEntry **ppceTarget);
void		CScene__ObjectAttributesReceived(CScene *pThis, CCacheEntry **ppceTarget);
void 		CScene__ObjectsHeaderReceived(CScene *pThis, CCacheEntry **ppceTarget);
void 		CScene__ObjectsIndexReceived(CScene *pThis, CCacheEntry **ppceTarget);
void 		CScene__TextureSetsHeaderReceived(CScene *pThis, CCacheEntry **ppceTarget);
void 		CScene__TextureSetsIndexReceived(CScene *pThis, CCacheEntry **ppceTarget);
void 		CScene__DecompressCollision(CScene *pThis, CCacheEntry **ppceTarget);
void 		CScene__CollisionReceived(CScene *pThis, CCacheEntry **ppceTarget);
void 		CScene__DecompressInstances(CScene *pThis, CCacheEntry **ppceTarget);
void		CScene__InstancesReceived(CScene *pThis, CCacheEntry **ppceTarget);
void 		CScene__LevelsHeaderReceived(CScene *pThis, CCacheEntry **ppceTarget);
void 		CScene__LevelsIndexReceived(CScene *pThis, CCacheEntry **ppceTarget);
void 		CScene__LevelIndexReceived(CScene *pThis, CCacheEntry **ppceTarget);
void 		CScene__LevelInfoReceived(CScene *pThis, CCacheEntry **ppceTarget);
void 		CScene__GridBoundsReceived(CScene *pThis, CCacheEntry **ppceTarget);
void 		CScene__DecompressGridBounds(CScene *pThis, CCacheEntry **ppceTarget);
void 		CScene__DecompressGridSection(CScene *pThis, CCacheEntry **ppceTarget);
void		CScene__DecompressGridPickupCounts(CScene *pThis, CCacheEntry **ppceTarget);

void		CScene__SetInstanceFlagByIndex(CScene *pThis, int nInstance, BOOL Set) ;

// CScene
/////////////////////////////////////////////////////////////////////////////

void CScene__Construct(CScene *pThis, int nWarpID)
{
	TRACE1("CScene construction -- Warp ID:%d\r\n", nWarpID);

	pThis->m_nWarpID = nWarpID;

	CCartCache__Construct(&pThis->m_Cache);
	CAnimationMtxs__Construct();

	// base address of game data on cartridge for DMA operations
	pThis->m_rpBaseAddress = (ROMAddress*) CEngineApp__GetStaticAddress(GetApp());

   pThis->m_pceIndex = pThis->m_pceInstances
							= pThis->m_pceTextureSetsIndex = NULL;
   pThis->m_rpObjectsAddress = pThis->m_rpLevelsAddress
							= pThis->m_rpLevelAddress
							= pThis->m_rpGridSectionsAddress
							= pThis->m_rpParticleEffectsAddress
							= pThis->m_rpGridBoundsAddress = NULL;

   pThis->m_pceObjectsHeader = pThis->m_pceObjectsIndex
							= pThis->m_pceGridSectionsIndex = NULL;
	pThis->m_pceLevelsHeader = pThis->m_pceLevelsIndex = pThis->m_pceLevelIndex = NULL;

	pThis->m_pceGridBounds = pThis->m_pceCollision
							= pThis->m_pceObjectTypes
							= pThis->m_pceWarpPoints
							= pThis->m_pceBinariesIndex
							= pThis->m_pceSoundEffects = NULL;

	pThis->m_nInstances = pThis->m_nActiveAnimInstances = 0;
	pThis->m_pPlayer = NULL;
	pThis->m_bPlayerActiveFlags = (BYTE) -1;
	pThis->m_SkyTextureReady = FALSE;
	pThis->m_SkyHeightSet = FALSE ;
	pThis->m_SkyHeight = 0 ;
	pThis->m_LevelInfoReceived = FALSE;

	//pThis->m_pLastWarpCurrentRegion = NULL;

	pThis->m_RotYPickup = 0;
	CQuatern__Identity(&pThis->m_qRotYPickup);

	// must go before request cartridge index
	CParticleSystem__Construct(&pThis->m_ParticleSystem);
	CSimplePool__Construct(&pThis->m_SimplePool);

	// Prepare the olde pickup monitor
	CPickupMonitor__Construct(&pThis->m_PickupMonitor) ;

	// request cartridge index
   CCartCache__RequestBlock(&pThis->m_Cache,
									 pThis, NULL, (pfnCACHENOTIFY) CScene__CartridgeIndexReceived,
									 &pThis->m_pceIndex,
                        	 pThis->m_rpBaseAddress, IS_INDEX_SIZE(CART_ROOT_nItems),
                        	 "Cartridge Index");

	// Reset misc
	pThis->m_cSendStaticEventsFrame = 0 ;
}

void CScene__Destruct(CScene *pThis)
{
	CCartCache__Destruct(&pThis->m_Cache);
}



#ifdef IG_DEBUG
void CScene__AddCollisionInstance(CScene *pThis, CInstanceHdr *pInst)
{
	ASSERT(pInst);

	if (pThis->m_nInstances == COLLISION_MAX_INSTANCES)
	{
		TRACE0("CScene: WARNING - Allocate more collision instance entries!\r\n");
	}
	else
	{
		pThis->m_pInstances[pThis->m_nInstances++] = pInst;
	}
}
#else
#define CScene__AddCollisionInstance(pThis, pInst) if (pThis->m_nInstances != COLLISION_MAX_INSTANCES) pThis->m_pInstances[pThis->m_nInstances++] = pInst;
#endif

void CScene__CartridgeIndexReceived(CScene *pThis, CCacheEntry **ppceTarget)
{
	CIndexedSet 	isIndex;
	ROMAddress		*rpPersistantCounts,
						*rpSoundEffects,
						*rpBinaryTypes;
	DWORD				persistantCountsSize,
						soundEffectsSize,
						binaryTypesSize;

	ASSERT(ppceTarget == &pThis->m_pceIndex);
	TRACE0("CScene: Cartridge index received.\r\n");

	// attatch to isIndex
   CIndexedSet__ConstructFromRawData(&isIndex,
												 CCacheEntry__GetData(pThis->m_pceIndex),
												 FALSE);

   ASSERT(CIndexedSet__GetBlockCount(&isIndex) >= CART_ROOT_nItems);


	// find address of isLevels
   pThis->m_rpLevelsAddress = CIndexedSet__GetROMAddress(&isIndex,
																			pThis->m_rpBaseAddress,
																			CART_ROOT_isLevels);

	// find address of isTextureSets
	pThis->m_rpTextureSetsAddress = CIndexedSet__GetROMAddress(&isIndex,
																				  pThis->m_rpBaseAddress,
																				  CART_ROOT_isTextureSets);

	// find address and size of particle effects
	pThis->m_rpParticleEffectsAddress = CIndexedSet__GetROMAddressAndSize(&isIndex,
																								 pThis->m_rpBaseAddress,
																								 CART_ROOT_isParticleEffects,
																								 &pThis->m_ParticleEffectsSize);

   // request instances, textures, and particle effects later

   ASSERT(CIndexedSet__GetBlockCount(&isIndex) > CART_ROOT_isGraphicObjects);

   // find address of isObjects
   pThis->m_rpObjectsAddress = CIndexedSet__GetROMAddress(&isIndex,
																			 pThis->m_rpBaseAddress,
																			 CART_ROOT_isGraphicObjects);


	// find address of object attributes
	pThis->m_rpObjectAttributesAddress = CIndexedSet__GetROMAddressAndSize(&isIndex,
																								  pThis->m_rpBaseAddress,
																								  CART_ROOT_usObjectAttributes,
																								  &pThis->m_ObjectAttributesSize);

	// find address of object types
	pThis->m_rpObjectTypesAddress = CIndexedSet__GetROMAddressAndSize(&isIndex,
																							pThis->m_rpBaseAddress,
																							CART_ROOT_usObjectTypes,
																							&pThis->m_ObjectTypesSize);


	// find address of persistant counts
	rpPersistantCounts = CIndexedSet__GetROMAddressAndSize(&isIndex,
																			 pThis->m_rpBaseAddress,
																			 CART_ROOT_isPersistantCounts,
																			 &persistantCountsSize);
	// find address of warp points
	pThis->m_rpWarpPointsAddress = CIndexedSet__GetROMAddressAndSize(&isIndex,
																						  pThis->m_rpBaseAddress,
																						  CART_ROOT_isWarpDests,
																						  &pThis->m_WarpPointsSize);

	if (persistant_data_loaded)
	{
		CScene__RequestWarpPoints(pThis);
	}
	else
	{
		// request persistant counts
		CCartCache__RequestBlock(&pThis->m_Cache,
										 pThis, NULL, (pfnCACHENOTIFY) CScene__PersistantCountsReceived,
										 &pThis->m_pcePersistantCounts,
										 rpPersistantCounts, persistantCountsSize,
										 "Persistant Counts");

		// warp points get requested in CScene__PersistantCountsReceived()
	}

	// find address and size of sound effects
	rpSoundEffects = CIndexedSet__GetROMAddressAndSize(&isIndex,
																		pThis->m_rpBaseAddress,
																		CART_ROOT_isSoundEffects,
																		&soundEffectsSize);
	// request sound effects
   CCartCache__RequestBlock(&pThis->m_Cache,
									 pThis, (pfnCACHENOTIFY) CCacheEntry__DecompressCallback, NULL,
                            &pThis->m_pceSoundEffects,
                            rpSoundEffects, soundEffectsSize,
                            "Sound Effects");


	// find address of binary blocks
	pThis->m_rpBinaries = CIndexedSet__GetROMAddress(&isIndex,
																	 pThis->m_rpBaseAddress,
																	 CART_ROOT_isBinaries);

	// find address and size of binary block types
	rpBinaryTypes = CIndexedSet__GetROMAddressAndSize(&isIndex,
																	  pThis->m_rpBaseAddress,
																	  CART_ROOT_usBinaryTypes,
																	  &binaryTypesSize);
	// request sound effects
   CCartCache__RequestBlock(&pThis->m_Cache,
									 pThis, NULL, (pfnCACHENOTIFY) CScene__BinaryTypesReceived,
                            &pThis->m_pceBinaryTypes,
                            rpBinaryTypes, binaryTypesSize,
                            "Binary Block Types");



   CIndexedSet__Destruct(&isIndex);
}

void CScene__BinaryTypesReceived(CScene *pThis, CCacheEntry **ppceTarget)
{
	CUnindexedSet	usBinaryTypes;
	int				nBinaries;

	ASSERT(ppceTarget == &pThis->m_pceBinaryTypes);
	TRACE0("CScene: Binary block types received.\r\n");

	CUnindexedSet__ConstructFromRawData(&usBinaryTypes,
													CCacheEntry__GetData(pThis->m_pceBinaryTypes),
													FALSE);

	nBinaries = CUnindexedSet__GetBlockCount(&usBinaryTypes);

	// request binaries index
   CCartCache__RequestBlock(&pThis->m_Cache,
									 NULL, NULL, NULL,
                            &pThis->m_pceBinariesIndex,
                            pThis->m_rpBinaries, IS_INDEX_SIZE(nBinaries),
                            "Binary Blocks Index");

	CUnindexedSet__Destruct(&usBinaryTypes);
}

// NOTE: caller is responsible for keeping block in cache!
BOOL CScene__RequestBinaryBlock(CScene *pThis, int nBlockType,
										  CCacheEntry **ppceBinaryBlock,
										  void *pNotifyPtr, pfnCACHENOTIFY pfnBlockReceived)
{
	CUnindexedSet	usBinaryTypes;
	CIndexedSet		isBinaries;
	int				nBinaries,
						pos;
	DWORD				*types,
						size;
	ROMAddress		*rpBlock;

	ASSERT(ppceBinaryBlock);

	if (!pThis->m_pceBinariesIndex || !pThis->m_pceBinaryTypes)
		return FALSE;

	CUnindexedSet__ConstructFromRawData(&usBinaryTypes,
													CCacheEntry__GetData(pThis->m_pceBinaryTypes),
													FALSE);

	nBinaries = CUnindexedSet__GetBlockCount(&usBinaryTypes);
	types = (DWORD*) CUnindexedSet__GetBasePtr(&usBinaryTypes);

	pos = BinarySearch(types, nBinaries, nBlockType)

	CUnindexedSet__Destruct(&usBinaryTypes);

	if (pos == -1)
		return FALSE;

	CIndexedSet__ConstructFromRawData(&isBinaries,
												 CCacheEntry__GetData(pThis->m_pceBinariesIndex),
												 FALSE);

   rpBlock = CIndexedSet__GetROMAddressAndSize(&isBinaries, pThis->m_rpBinaries,
															  pos, &size);

   CCartCache__RequestBlock(&pThis->m_Cache,
									 pNotifyPtr, (pfnCACHENOTIFY) CCacheEntry__DecompressCallback, pfnBlockReceived,
                            ppceBinaryBlock,
                            rpBlock, size,
                            "Binary Block");

	CIndexedSet__Destruct(&isBinaries);

	return TRUE;
}

void CScene__RequestWarpPoints(CScene *pThis)
{
	// request warp destinations
   CCartCache__RequestBlock(&pThis->m_Cache,
									 pThis, NULL, (pfnCACHENOTIFY) CScene__WarpPointsReceived,
                            &pThis->m_pceWarpPoints,
                            pThis->m_rpWarpPointsAddress, pThis->m_WarpPointsSize,
                            "Warp Points");
}

void CScene__WarpPointsReceived(CScene *pThis, CCacheEntry **ppceTarget)
{
	CIndexedSet		isWarpPoints;
	CUnindexedSet	usIDs,
						usWarpDests;
	void				*pbIDs,
						*pbWarpDests;
	DWORD				*ids;
	int				nWarpPoints,
						first, last,
						choice;
	CROMWarpPoint	*warpPoints;

	ASSERT(ppceTarget == &pThis->m_pceWarpPoints);
	TRACE0("CScene: Warp points received.\r\n");

	CIndexedSet__ConstructFromRawData(&isWarpPoints,
												 CCacheEntry__GetData(pThis->m_pceWarpPoints),
												 FALSE);

	pbIDs = CIndexedSet__GetBlock(&isWarpPoints, CART_WARPDESTS_usIDs);
	CUnindexedSet__ConstructFromRawData(&usIDs, pbIDs, FALSE);
	ids = (DWORD*) CUnindexedSet__GetBasePtr(&usIDs);
	nWarpPoints = CUnindexedSet__GetBlockCount(&usIDs);
	/* PORT: warp-dest ID DWORDs are big-endian, but BinaryRange (defs.c) now byte-swaps each key on
	 * read — so the in-place swap that used to be here is gone (it would double-swap and miss the warp). */

	pbWarpDests = CIndexedSet__GetBlock(&isWarpPoints, CART_WARPDESTS_usWarpDests);
	CUnindexedSet__ConstructFromRawData(&usWarpDests, pbWarpDests, FALSE);
	warpPoints = (CROMWarpPoint*) CUnindexedSet__GetBasePtr(&usWarpDests);
	ASSERT(nWarpPoints == CUnindexedSet__GetBlockCount(&usWarpDests));


	// handle ID -1 as return warp
	if (pThis->m_nWarpID == RETURN_WARP_ID)
	{
		if (GetApp()->m_ReturnWarpSaved)
		{
			pThis->m_WarpFound = TRUE;
			pThis->m_WarpPoint = GetApp()->m_ReturnWarp;
			pThis->m_nLevel = pThis->m_WarpPoint.m_nLevel;
		}
		else
		{
			pThis->m_WarpFound = FALSE;
			pThis->m_nLevel = 0;
		}
#ifdef PLATFORM_PORT
		/* TUROK_WARPLOG (blue-portal return bug): was the return point actually saved at portal-entry,
		 * and what level does the return resolve to? If returnSaved=0 the forward store never fired. */
		{ extern char *getenv(const char *); static int wl=-1; if(wl<0){const char*e=getenv("TUROK_WARPLOG"); wl=(e&&atoi(e))?1:0;}
		  if(wl){ extern int fprintf(void*,const char*,...); extern void *stderr;
		    fprintf(stderr,"[WARP] RequestWarpPoints RETURN: returnSaved=%d -> found=%d level=%d\n",
		      GetApp()->m_ReturnWarpSaved, pThis->m_WarpFound, pThis->m_nLevel); } }
#endif
	}
	else
	if (pThis->m_nWarpID == CRASH_WARP_ID)
	{
		pThis->m_WarpFound = TRUE;
		pThis->m_WarpPoint = CrashWarp;
		pThis->m_nLevel = pThis->m_WarpPoint.m_nLevel;
	}
	else
	if (pThis->m_nWarpID == CINEMA_WARP_ID)
	{
			pThis->m_WarpFound = TRUE ;
			pThis->m_WarpPoint = GetApp()->m_CinemaWarp ;
			pThis->m_nLevel = pThis->m_WarpPoint.m_nLevel ;
	}
	else
	{
		pThis->m_WarpFound = BinaryRange(ids, nWarpPoints, pThis->m_nWarpID, &first, &last);
		if (pThis->m_WarpFound)
		{
			choice = first + RANDOM((last + 1) - first);
#ifdef PLATFORM_PORT
			/* TUROK_FORCECHOICE=<n>: force the n-th warp point in the matched range (debug the blue-portal
			 * OOB — a warp ID with multiple points is RANDOM-picked; this pins a specific one). */
			{ extern char *getenv(const char*); const char*_fce=getenv("TUROK_FORCECHOICE");
			  if(_fce && last>first){ extern int fprintf(void*,const char*,...); extern void *stderr;
			    choice = first + (atoi(_fce) % ((last+1)-first));
			    fprintf(stderr,"[WARP]   FORCECHOICE -> choice=%d ids[%d]=0x%x ids[%d]=0x%x\n",
			      choice, first, (unsigned)ORDERBYTES(ids[first]), last, (unsigned)ORDERBYTES(ids[last])); } }
#endif

			// must copy data because pceWarpPoints may not be around later
			pThis->m_WarpPoint = warpPoints[choice];
#ifdef PLATFORM_PORT
			/* warpPoints[] is big-endian cart data. Swap the fields consumed in NATIVE order here: m_vPos
			 * (the instance decode's ORDERBYTES on the CVector3 aggregate is a NO-OP, so it must be native
			 * already) and m_nLevel (used directly to select the level). Leave m_nRegion & m_RotY big-endian
			 * — the instance decode ORDERBYTES'es them, and cinecam ORDERBYTES'es m_nRegion. This is the
			 * BinaryRange branch only; the Crash/Return/Cinema warps are native and untouched. */
			pThis->m_WarpPoint.m_vPos.x = ORDERBYTES(pThis->m_WarpPoint.m_vPos.x);
			pThis->m_WarpPoint.m_vPos.y = ORDERBYTES(pThis->m_WarpPoint.m_vPos.y);
			pThis->m_WarpPoint.m_vPos.z = ORDERBYTES(pThis->m_WarpPoint.m_vPos.z);
			pThis->m_WarpPoint.m_nLevel = ORDERBYTES(pThis->m_WarpPoint.m_nLevel);
#endif
			pThis->m_nLevel = pThis->m_WarpPoint.m_nLevel;
#ifdef PLATFORM_PORT
			{ extern char *getenv(const char*); static int wl=-1; if(wl<0){const char*e=getenv("TUROK_WARPLOG"); wl=(e&&atoi(e))?1:0;}
			  if(wl){ extern int fprintf(void*,const char*,...); extern void *stderr;
			    fprintf(stderr,"[WARP] resolve nWarpID=%d FOUND choice=%d/[%d..%d] vPos=(%.0f,%.0f,%.0f) nRegion raw=0x%x sw=%d nLevel=%d\n",
			      pThis->m_nWarpID, choice, first, last, pThis->m_WarpPoint.m_vPos.x, pThis->m_WarpPoint.m_vPos.y, pThis->m_WarpPoint.m_vPos.z,
			      (unsigned)pThis->m_WarpPoint.m_nRegion, (int)(WORD)ORDERBYTES((WORD)pThis->m_WarpPoint.m_nRegion), pThis->m_nLevel);
				    { int _wi; for(_wi=first;_wi<=last;_wi++) fprintf(stderr,"[WARP]   wp[%d] vPos=(%.0f,%.0f,%.0f) nLvl=%d nReg=0x%x(sw=%d)\n", _wi, ORDERBYTES(warpPoints[_wi].m_vPos.x), ORDERBYTES(warpPoints[_wi].m_vPos.y), ORDERBYTES(warpPoints[_wi].m_vPos.z), (int)(WORD)ORDERBYTES((WORD)warpPoints[_wi].m_nLevel), (unsigned)warpPoints[_wi].m_nRegion, (int)(WORD)ORDERBYTES((WORD)warpPoints[_wi].m_nRegion)); } } }
#endif
		}
		else
		{
			pThis->m_nLevel = 0;
#ifdef PLATFORM_PORT
			{ extern char *getenv(const char*); static int wl=-1; if(wl<0){const char*e=getenv("TUROK_WARPLOG"); wl=(e&&atoi(e))?1:0;}
			  if(wl){ extern int fprintf(void*,const char*,...); extern void *stderr;
			    fprintf(stderr,"[WARP] resolve nWarpID=%d NOT-FOUND in this level's warp points -> m_nLevel=0\n", pThis->m_nWarpID); } }
#endif
		}
	}

	// request object types
   CCartCache__RequestBlock(&pThis->m_Cache,
									 pThis, NULL, (pfnCACHENOTIFY) CScene__ObjectTypesReceived,
                            &pThis->m_pceObjectTypes,
                            pThis->m_rpObjectTypesAddress, pThis->m_ObjectTypesSize,
                            "Objects Types");

	CIndexedSet__Destruct(&isWarpPoints);
	CUnindexedSet__Destruct(&usIDs);
	CUnindexedSet__Destruct(&usWarpDests);
}
/*
void CScene__RequestObjectTypes(CScene *pThis)
{
	// request object types
   CCartCache__RequestBlock(&pThis->m_Cache,
									 pThis, NULL, (pfnCACHENOTIFY) CScene__ObjectTypesReceived,
                            &pThis->m_pceObjectTypes,
                            pThis->m_rpObjectTypesAddress, pThis->m_ObjectTypesSize,
                            "Objects Types");
}
*/

void* CScene__GetPersistantDataAddress(CScene *pThis)
{
	return persistant_data;
}

int CScene__GetPersistantDataSize(CScene *pThis)
{
	CIndexedSet			isPersistantData;
	void					*pData;
	DWORD					Size;


	ASSERT(persistant_data_loaded);

	CIndexedSet__ConstructFromRawData(&isPersistantData, &persistant_data, FALSE);

	CIndexedSet__GetRawData(&isPersistantData, &pData, &Size);

	CIndexedSet__Destruct(&isPersistantData);

	return Size;
}

void CScene__PersistantDataLoaded(CScene *pThis)
{
	persistant_data_loaded = TRUE;
}

CPersistantData* CScene__GetPersistantDataStruct(CScene *pThis)
{
	CIndexedSet			isPersistantData;
	CPersistantData	*pPersist;

	ASSERT(persistant_data_loaded);

	CIndexedSet__ConstructFromRawData(&isPersistantData, &persistant_data, FALSE);

	pPersist = (CPersistantData*) CIndexedSet__GetBlock(&isPersistantData, 0);

	CIndexedSet__Destruct(&isPersistantData);

	return pPersist;
}


#define PERSIST_ROOT_nItems					3

#define PERSIST_ROOT_persistStruct			0
#define PERSIST_ROOT_isLevels					1
#define PERSIST_ROOT_regionStream			2


#define PERSIST_LEVEL_nItems					2

#define PERSIST_LEVEL_pickupStream			0
#define PERSIST_LEVEL_animStream				1


void CScene__ResetPersistantData()
{
	CIndexedSet		isPersistantData,
						isLevels, isLevel;
	void				*pbPersistStruct,
						*pbLevels, *pbLevel,
						*pbPickupStream, *pbAnimStream,
						*pbRegionStream;
	DWORD				persistStructSize,
						pickupStreamSize, animStreamSize,
						regionStreamSize;
	int				cLevel, nLevels;

	if (persistant_data_loaded)
	{
		CIndexedSet__ConstructFromRawData(&isPersistantData, &persistant_data, FALSE);
	
		pbPersistStruct = CIndexedSet__GetBlockAndSize(&isPersistantData, PERSIST_ROOT_persistStruct, &persistStructSize);
		memset(pbPersistStruct, 0, persistStructSize);

		pbLevels = CIndexedSet__GetBlock(&isPersistantData, PERSIST_ROOT_isLevels);
		CIndexedSet__ConstructFromRawData(&isLevels, pbLevels, FALSE);

		nLevels = CIndexedSet__GetBlockCount(&isLevels);
		
		for (cLevel=0; cLevel<nLevels; cLevel++)
		{
			pbLevel = CIndexedSet__GetBlock(&isLevels, cLevel);

			CIndexedSet__ConstructFromRawData(&isLevel, pbLevel, FALSE);

			pbPickupStream = CIndexedSet__GetBlockAndSize(&isLevel, PERSIST_LEVEL_pickupStream, &pickupStreamSize);
			memset(pbPickupStream, 0, pickupStreamSize);

			pbAnimStream = CIndexedSet__GetBlockAndSize(&isLevel, PERSIST_LEVEL_animStream, &animStreamSize);
			memset(pbAnimStream, 0, animStreamSize);
			
			CIndexedSet__Destruct(&isLevel);
		}

		pbRegionStream = CIndexedSet__GetBlockAndSize(&isPersistantData, PERSIST_ROOT_regionStream, &regionStreamSize);
		memset(pbRegionStream, 0, regionStreamSize);

		CIndexedSet__Destruct(&isPersistantData);
		CIndexedSet__Destruct(&isLevels);
	}
}


BYTE* CScene__GetPersistantBitstream(CScene *pThis, int nLevel, int nBitstream)
{
	CIndexedSet		isPersistantData,
						isLevels, isLevel;
	void				*pbLevels, *pbLevel;
	BYTE				*pBitStream;

	ASSERT(persistant_data_loaded);

	CIndexedSet__ConstructFromRawData(&isPersistantData, &persistant_data, FALSE);

	pbLevels = CIndexedSet__GetBlock(&isPersistantData, 1);
	CIndexedSet__ConstructFromRawData(&isLevels, pbLevels, FALSE);

	ASSERT((nLevel >= 0) && (nLevel < CIndexedSet__GetBlockCount(&isLevels)));
	pbLevel = CIndexedSet__GetBlock(&isLevels, nLevel);
	CIndexedSet__ConstructFromRawData(&isLevel, pbLevel, FALSE);

	ASSERT((nBitstream >= 0) && (nBitstream < CIndexedSet__GetBlockCount(&isLevel)));
	pBitStream = (BYTE*) CIndexedSet__GetBlock(&isLevel, nBitstream);

	CIndexedSet__Destruct(&isPersistantData);
	CIndexedSet__Destruct(&isLevels);
	CIndexedSet__Destruct(&isLevel);

	return pBitStream;
}

void CScene__SetPickupFlag(CScene *pThis, CGameSimpleInstance *pSimple, BOOL Set)
{
	BYTE *pBitStream;

	pBitStream = CScene__GetPersistantBitstream(pThis, pThis->m_nLevel, PERSIST_LEVEL_pickupStream);
	SetBitStreamFlag(pBitStream, pSimple->m_nID, Set);
}
BOOL CScene__GetPickupFlag(CScene *pThis, CGameSimpleInstance *pSimple)
{
	BYTE *pBitStream;

	pBitStream = CScene__GetPersistantBitstream(pThis, pThis->m_nLevel, PERSIST_LEVEL_pickupStream);
	return GetBitStreamFlag(pBitStream, pSimple->m_nID);
}

int CScene__GetAnimInstanceIndex(CScene *pThis, CGameObjectInstance *pAnim)
{
   CUnindexedSet			usInstances;
	CGameObjectInstance	*instances;
	int						offset, index;

	ASSERT(pThis->m_pceInstances);

	CUnindexedSet__ConstructFromRawData(&usInstances,
													CCacheEntry__GetData(pThis->m_pceInstances),
													FALSE);

   instances = (CGameObjectInstance*) CUnindexedSet__GetBasePtr(&usInstances);
	offset = (DWORD)pAnim - (DWORD)instances;
	index = offset/sizeof(CGameObjectInstance);

	ASSERT((index >= 0) && (index < CUnindexedSet__GetBlockCount(&usInstances)));

	CUnindexedSet__Destruct(&usInstances);

	return index;
}

void CScene__SetInstanceFlag(CScene *pThis, CGameObjectInstance *pAnim, BOOL Set)
{
	int	index;
	BYTE	*pBitStream;

	index = CScene__GetAnimInstanceIndex(pThis, pAnim);

	pBitStream = CScene__GetPersistantBitstream(pThis, pThis->m_nLevel, PERSIST_LEVEL_animStream);
	SetBitStreamFlag(pBitStream, index, Set);
}
BOOL CScene__GetInstanceFlag(CScene *pThis, CGameObjectInstance *pAnim)
{
	int	index;
	BYTE	*pBitStream;

	index = CScene__GetAnimInstanceIndex(pThis, pAnim);

	pBitStream = CScene__GetPersistantBitstream(pThis, pThis->m_nLevel, PERSIST_LEVEL_animStream);
	return GetBitStreamFlag(pBitStream, index);
}
BOOL CScene__GetInstanceFlagByIndex(CScene *pThis, int nInstance)
{
	BYTE	*pBitStream;

	pBitStream = CScene__GetPersistantBitstream(pThis, pThis->m_nLevel, PERSIST_LEVEL_animStream);
	return GetBitStreamFlag(pBitStream, nInstance);
}

void CScene__SetInstanceFlagByIndex(CScene *pThis, int nInstance, BOOL Set)
{
	BYTE	*pBitStream;

	pBitStream = CScene__GetPersistantBitstream(pThis, pThis->m_nLevel, PERSIST_LEVEL_animStream);
	SetBitStreamFlag(pBitStream, nInstance, Set);
}

void CScene__ClearAllRegionFlags(CScene *pThis)
{
	CIndexedSet			isPersistantData;
	BYTE					*pBitstream;
	DWORD					size;

	ASSERT(persistant_data_loaded);

	CIndexedSet__ConstructFromRawData(&isPersistantData, &persistant_data, FALSE);

	pBitstream = (BYTE*) CIndexedSet__GetBlockAndSize(&isPersistantData, 2, &size);
	memset(pBitstream, 0, size);

	CIndexedSet__Destruct(&isPersistantData);
}

BYTE* CScene__GetRegionBitstream(CScene *pThis)
{
	CIndexedSet			isPersistantData;
	BYTE					*pBitstream;

	ASSERT(persistant_data_loaded);

	CIndexedSet__ConstructFromRawData(&isPersistantData, &persistant_data, FALSE);

	pBitstream = (BYTE*) CIndexedSet__GetBlock(&isPersistantData, 2);

	CIndexedSet__Destruct(&isPersistantData);

	return pBitstream;
}

void CScene__SetRegionFlag(CScene *pThis, int nRegion, BOOL Set)
{
	BYTE	*pBitStream;

	pBitStream = CScene__GetRegionBitstream(pThis);
	SetBitStreamFlag(pBitStream, nRegion, Set);
}

BOOL CScene__GetRegionFlag(CScene *pThis, int nRegion)
{
	BYTE	*pBitStream;

	pBitStream = CScene__GetRegionBitstream(pThis);
	return GetBitStreamFlag(pBitStream, nRegion);
}

void CScene__PersistantCountsReceived(CScene *pThis, CCacheEntry **ppceTarget)
{
	CIndexedSet		isPersistantCounts,
						isPersistantData,
						isLevels, isLevel;
	void				*pbPickupCounts, *pbAnimCounts,
						*pbPersistantData,
						*pbLevels, *pbLevel;
	BYTE				*pBytes;
	CUnindexedSet	usPickupCounts, usAnimCounts;
	int				cLevel, nLevels,
						offset,
						levelsOffset, levelOffset;
	DWORD				*pickupCounts, *animCounts,
						*pMaxRegionCount;

	ASSERT(ppceTarget == &pThis->m_pcePersistantCounts);
	TRACE0("CScene: Persistant counts received.\r\n");

	CIndexedSet__ConstructFromRawData(&isPersistantCounts,
												 CCacheEntry__GetData(pThis->m_pcePersistantCounts),
												 FALSE);

	pbPickupCounts = CIndexedSet__GetBlock(&isPersistantCounts, CART_PERSIST_usPickupCounts);
	CUnindexedSet__ConstructFromRawData(&usPickupCounts, pbPickupCounts, FALSE);
	pickupCounts = (DWORD*) CUnindexedSet__GetBasePtr(&usPickupCounts);
	nLevels = CUnindexedSet__GetBlockCount(&usPickupCounts);

	pbAnimCounts = CIndexedSet__GetBlock(&isPersistantCounts, CART_PERSIST_usAnimCounts);
	CUnindexedSet__ConstructFromRawData(&usAnimCounts, pbAnimCounts, FALSE);
	animCounts = (DWORD*) CUnindexedSet__GetBasePtr(&usAnimCounts);
	ASSERT(nLevels == CUnindexedSet__GetBlockCount(&usAnimCounts));

	pMaxRegionCount = (DWORD*) CIndexedSet__GetBlock(&isPersistantCounts, CART_PERSIST_MaxRegionCount);

	memset(persistant_data, 0, PERSISTANT_DATA_MAX_SIZE);

	pBytes = (BYTE*) persistant_data;
	pbPersistantData = pBytes;
	CIndexedSet__ConstructFromRawData(&isPersistantData, pbPersistantData, FALSE);

	offset = IS_INDEX_SIZE(3);
	CIndexedSet__SetBlockCount(&isPersistantData, 0);
	CIndexedSet__SetNextOffset(&isPersistantData, offset);

	// persistant data struct
	//pbPersistantStruct = pBytes + offset;
	offset += FixAlignment(sizeof(CPersistantData), IS_ALIGNMENT);
	CIndexedSet__SetBlockCount(&isPersistantData, 1);
	CIndexedSet__SetNextOffset(&isPersistantData, offset);

	// isLevels
	pbLevels = pBytes + offset;
	CIndexedSet__ConstructFromRawData(&isLevels, pbLevels, FALSE);

	// isLevels index
	levelsOffset = IS_INDEX_SIZE(nLevels);
	CIndexedSet__SetBlockCount(&isLevels, 0);
	CIndexedSet__SetNextOffset(&isLevels, levelsOffset);

	//rmonPrintf("nLevels:%d\n", nLevels);
	for (cLevel=0; cLevel<nLevels; cLevel++)
	{
		//rmonPrintf("%d  nPickups:%d, nAnims:%d\n", cLevel, pickupCounts[cLevel], animCounts[cLevel]);

		// isLevel
		pbLevel = pBytes + offset + levelsOffset;
		CIndexedSet__ConstructFromRawData(&isLevel, pbLevel, FALSE);

		// isLevel index
		levelOffset = IS_INDEX_SIZE(2);
		CIndexedSet__SetBlockCount(&isLevel, 0);
		CIndexedSet__SetNextOffset(&isLevel, levelOffset);

		// pickup stream
		//pbPickupStream = pBytes + offset + levelsOffset + levelOffset;
		levelOffset += FixAlignment(GetBitStreamSize(ORDERBYTES(pickupCounts[cLevel])), IS_ALIGNMENT);  /* PORT: big-endian count */
		CIndexedSet__SetBlockCount(&isLevel, 1);
		CIndexedSet__SetNextOffset(&isLevel, levelOffset);

		// anim stream
		//pbAnimStream = pBytes + offset + levelsOffset + levelOffset;
		levelOffset += FixAlignment(GetBitStreamSize(ORDERBYTES(animCounts[cLevel])), IS_ALIGNMENT);  /* PORT: big-endian count */
		CIndexedSet__SetBlockCount(&isLevel, 2);
		CIndexedSet__SetNextOffset(&isLevel, levelOffset);


		levelsOffset += levelOffset;
		CIndexedSet__SetBlockCount(&isLevels, cLevel + 1);
		CIndexedSet__SetNextOffset(&isLevels, levelsOffset);


		CIndexedSet__Destruct(&isLevel);
	}

	offset += levelsOffset;
	CIndexedSet__SetBlockCount(&isPersistantData, 2);
	CIndexedSet__SetNextOffset(&isPersistantData, offset);


	// region visibility flags
	offset += FixAlignment(GetBitStreamSize(ORDERBYTES(*pMaxRegionCount)), IS_ALIGNMENT);  /* PORT: big-endian count */
	CIndexedSet__SetBlockCount(&isPersistantData, 3);
	CIndexedSet__SetNextOffset(&isPersistantData, offset);


	//rmonPrintf("persistant data size:%d\n", offset);
	ASSERT(offset < PERSISTANT_DATA_MAX_SIZE);


	CIndexedSet__Destruct(&isPersistantCounts);
	CUnindexedSet__Destruct(&usPickupCounts);
	CUnindexedSet__Destruct(&usAnimCounts);
	CIndexedSet__Destruct(&isLevels);
	CIndexedSet__Destruct(&isPersistantData);

	persistant_data_loaded = TRUE;

	CScene__RequestWarpPoints(pThis);
}

void CScene__ObjectTypesReceived(CScene *pThis, CCacheEntry **ppceTarget)
{
	ASSERT(ppceTarget == &pThis->m_pceObjectTypes);
	TRACE0("CScene: Objects types received.\r\n");

	// request object attributes
   CCartCache__RequestBlock(&pThis->m_Cache,
                            pThis, (pfnCACHENOTIFY) CCacheEntry__DecompressCallback, (pfnCACHENOTIFY) CScene__ObjectAttributesReceived,
                            &pThis->m_pceObjectAttributes,
                            pThis->m_rpObjectAttributesAddress, pThis->m_ObjectAttributesSize,
                            "Objects Attributes");
}


void CScene__ObjectAttributesReceived(CScene *pThis, CCacheEntry **ppceTarget)
{
	ASSERT(ppceTarget == &pThis->m_pceObjectAttributes);
	TRACE0("CScene: Objects attributes received.\r\n");

#ifdef PLATFORM_PORT
	/* PORT (deferred M5 "#6 Variations"): the CEnemyAttributes (Variations) block is consumed RAW
	 * (scene.c casts GetBasePtr -> CEnemyAttributes*), so its big-endian multi-byte fields must be
	 * byte-swapped ONCE here, on the decompressed block. Layout (aistruc.h) = 13 floats + 3 DWORDs
	 * (16 contiguous 4-byte fields), then WORD m_wTypeFlags3, then 4 shorts (StartHealth/Id/DeathId/
	 * Regenerate); the trailing 6 bytes (Aggression/InteractiveAnim/nModel/nTexture/AttackStyle/unused)
	 * are endian-safe. Spec = CEnemyAttributes__TakeFromVariation (aistruc.c). Without this the AI
	 * type-flag tests read garbage so AI_VISIBLE is never set -> every enemy/pickup is culled at the
	 * romstruc.c draw gate, and triggers/AI misbehave. */
	{
		CUnindexedSet usEA; CEnemyAttributes *ea; int n, i;
		CUnindexedSet__ConstructFromRawData(&usEA, CCacheEntry__GetData(*ppceTarget), FALSE);
		n  = CUnindexedSet__GetBlockCount(&usEA);
		ea = (CEnemyAttributes*) CUnindexedSet__GetBasePtr(&usEA);
		{ extern int fprintf(void*,const char*,...); extern void *stderr;
		  if (n < 0 || n > 4096) {   /* a level can't have this many enemy variations -> a bad count would stomp the heap */
		    fprintf(stderr, "[scene] EnemyAttributes count=%d implausible — skipping swap (would OOB)\n", n);
		    n = 0;
		  } }
		for (i = 0; i < n; i++) {
			unsigned int *w = (unsigned int*) &ea[i];
			int j;
			for (j = 0; j < 16; j++) w[j] = __builtin_bswap32(w[j]);   /* 13 floats + species/typeflags/typeflags2 */
			ea[i].m_wTypeFlags3 = __builtin_bswap16(ea[i].m_wTypeFlags3);
			ea[i].m_StartHealth = (short) __builtin_bswap16((unsigned short) ea[i].m_StartHealth);
			ea[i].m_Id          = (short) __builtin_bswap16((unsigned short) ea[i].m_Id);
			ea[i].m_DeathId     = (short) __builtin_bswap16((unsigned short) ea[i].m_DeathId);
			ea[i].m_Regenerate  = (short) __builtin_bswap16((unsigned short) ea[i].m_Regenerate);
		}
	}
#endif

   // request objects header (need to know number of items to know index size)
   CCartCache__RequestBlock(&pThis->m_Cache,
                            pThis, NULL, (pfnCACHENOTIFY) CScene__ObjectsHeaderReceived,
                            &pThis->m_pceObjectsHeader,
                            pThis->m_rpObjectsAddress, IS_HEADER_SIZE,
                            "Objects Header");
}

void CScene__ObjectsHeaderReceived(CScene *pThis, CCacheEntry **ppceTarget)
{
	CIndexedSet isObjects;
	int 			nObjects;

	ASSERT(ppceTarget == &pThis->m_pceObjectsHeader);
	TRACE0("CScene: Objects header received.\r\n");

   // attatch to isObjects
   CIndexedSet__ConstructFromRawData(&isObjects,
												 CCacheEntry__GetData(pThis->m_pceObjectsHeader),
												 FALSE);

	nObjects = CIndexedSet__GetBlockCount(&isObjects);
	TRACE("CScene: There are %d objects.\r\n", nObjects);

   // request objects index
   CCartCache__RequestBlock(&pThis->m_Cache,
                            pThis, NULL, (pfnCACHENOTIFY) CScene__ObjectsIndexReceived,
                            &pThis->m_pceObjectsIndex,
                            pThis->m_rpObjectsAddress, IS_INDEX_SIZE(nObjects),
                            "Objects Index");

   CIndexedSet__Destruct(&isObjects);
}

void CScene__ObjectsIndexReceived(CScene *pThis, CCacheEntry **ppceTarget)
{
	ASSERT(ppceTarget == &pThis->m_pceObjectsIndex);
	TRACE0("CScene: Objects index received.\r\n");

	CSimplePool__Initialize(&pThis->m_SimplePool, &pThis->m_Cache,
									pThis->m_pceObjectsIndex, pThis->m_rpObjectsAddress);

   // request texture sets header (need to know number of items to know index size)
   CCartCache__RequestBlock(&pThis->m_Cache,
                            pThis, NULL, (pfnCACHENOTIFY) CScene__TextureSetsHeaderReceived,
                            &pThis->m_pceTextureSetsHeader,
                            pThis->m_rpTextureSetsAddress, IS_HEADER_SIZE,
                            "TextureSets Header");
}

void CScene__TextureSetsHeaderReceived(CScene *pThis, CCacheEntry **ppceTarget)
{
	CIndexedSet isTextureSets;
	int			nTextures;

	ASSERT(ppceTarget == &pThis->m_pceTextureSetsHeader);
	TRACE0("CScene: Texture sets header received.\r\n");

   // attatch to isTextureSets
   CIndexedSet__ConstructFromRawData(&isTextureSets,
												 CCacheEntry__GetData(pThis->m_pceTextureSetsHeader),
												 FALSE);

	nTextures = CIndexedSet__GetBlockCount(&isTextureSets);
	TRACE("CScene: There are %d texture sets.\r\n", nTextures);

	// request texture sets index
   CCartCache__RequestBlock(&pThis->m_Cache,
                            pThis, NULL, (pfnCACHENOTIFY) CScene__TextureSetsIndexReceived,
                            &pThis->m_pceTextureSetsIndex,
                            pThis->m_rpTextureSetsAddress, IS_INDEX_SIZE(nTextures),
                            "TextureSets Index");

   CIndexedSet__Destruct(&isTextureSets);
}

void CScene__TextureSetsIndexReceived(CScene *pThis, CCacheEntry **ppceTarget)
{
	ASSERT(ppceTarget == &pThis->m_pceTextureSetsIndex);
	TRACE0("CScene: Texture sets index received.\r\n");


	// request particle effects
	CParticleSystem__RequestData(&pThis->m_ParticleSystem, &pThis->m_Cache,
										  pThis->m_rpParticleEffectsAddress, pThis->m_ParticleEffectsSize,
										  pThis->m_pceTextureSetsIndex);

	// request levels header (need to know number of items to know index size)
   CCartCache__RequestBlock(&pThis->m_Cache,
                            pThis, NULL, (pfnCACHENOTIFY) CScene__LevelsHeaderReceived,
                            &pThis->m_pceLevelsHeader,
                            pThis->m_rpLevelsAddress, IS_HEADER_SIZE,
                            "Levels Header");
}

void CScene__LevelsHeaderReceived(CScene *pThis, CCacheEntry **ppceTarget)
{
	CIndexedSet isLevels;
	int			nLevels;

	ASSERT(ppceTarget == &pThis->m_pceLevelsHeader);
	TRACE0("CScene: Levels header received.\r\n");

	CIndexedSet__ConstructFromRawData(&isLevels,
												 CCacheEntry__GetData(pThis->m_pceLevelsHeader),
												 FALSE);

	nLevels = CIndexedSet__GetBlockCount(&isLevels);
	TRACE("There are %d levels.\r\n", nLevels);

	// request levels index
   CCartCache__RequestBlock(&pThis->m_Cache,
                            pThis, NULL, (pfnCACHENOTIFY) CScene__LevelsIndexReceived,
                            &pThis->m_pceLevelsIndex,
                            pThis->m_rpLevelsAddress, IS_INDEX_SIZE(nLevels),
                            "Levels Index");

	CIndexedSet__Destruct(&isLevels);
}

void CScene__LevelsIndexReceived(CScene *pThis, CCacheEntry **ppceTarget)
{
	ASSERT(ppceTarget == &pThis->m_pceLevelsIndex);
	TRACE0("CScene: Levels index received.\r\n");

	CScene__RequestLevel(pThis, pThis->m_nLevel);
}

void CScene__RequestLevel(CScene *pThis, int nLevel)
{
	CIndexedSet isLevels;

	ASSERT(pThis->m_pceLevelsIndex);

	// invalidate pointers
	pThis->m_pceInstances			= NULL;
	pThis->m_pceLevelIndex			= NULL;
	//pThis->m_pceGridPickupCounts	= NULL;
	pThis->m_pceGridBounds			= NULL;
	pThis->m_pceGridSectionsIndex = NULL;
	pThis->m_pceCollision			= NULL;

	pThis->m_nInstances = pThis->m_nActiveAnimInstances = 0;
	pThis->m_pPlayer = NULL;

	CIndexedSet__ConstructFromRawData(&isLevels,
												 CCacheEntry__GetData(pThis->m_pceLevelsIndex),
												 FALSE);

	ASSERT(CIndexedSet__GetBlockCount(&isLevels));
#ifdef PLATFORM_PORT
	/* TUROK_WARPLOG (blue-portal OOB): which level-geometry block this warp selects. If a bonus warp's
	 * m_nLevel modulos to the wrong/empty block, the wrong geometry streams and the player voids. */
	{ extern char *getenv(const char*); static int wl=-1; if(wl<0){const char*e=getenv("TUROK_WARPLOG"); wl=(e&&atoi(e))?1:0;}
	  if(wl){ extern int fprintf(void*,const char*,...); extern void *stderr; int bc=CIndexedSet__GetBlockCount(&isLevels);
	    fprintf(stderr,"[WARP] RequestLevel nLevel(in)=%d -> block=%d (count=%d)\n", nLevel, ((nLevel%bc)+bc)%bc, bc); } }
#endif
	nLevel %= CIndexedSet__GetBlockCount(&isLevels);

	// find address of level
   pThis->m_rpLevelAddress = CIndexedSet__GetROMAddress(&isLevels,
																  		  pThis->m_rpLevelsAddress,
																  		  nLevel);

	CCartCache__RequestBlock(&pThis->m_Cache,
                            pThis, NULL, (pfnCACHENOTIFY) CScene__LevelIndexReceived,
                            &pThis->m_pceLevelIndex,
                            pThis->m_rpLevelAddress, IS_INDEX_SIZE(CART_LEVEL_nItems),
                            "Level Index");

	CIndexedSet__Destruct(&isLevels);
}

void CScene__LevelIndexReceived(CScene *pThis, CCacheEntry **ppceTarget)
{
	CIndexedSet 	isLevel;
	ROMAddress		*rpLevelInfo,
						*rpCollisionAddress,
						*rpSkyTextureSetAddress;
	DWORD				levelInfoSize,
						collisionSize,
						skyTextureSetSize;

	ASSERT(ppceTarget == &pThis->m_pceLevelIndex);
	TRACE0("CScene: Level index received.\r\n");

	CIndexedSet__ConstructFromRawData(&isLevel,
												 CCacheEntry__GetData(pThis->m_pceLevelIndex),
												 FALSE);

	// find collision address
	rpCollisionAddress = CIndexedSet__GetROMAddressAndSize(&isLevel, pThis->m_rpLevelAddress,
																			 CART_LEVEL_isCollision, &collisionSize);

	// request collision (will be decompressed)
	// (must be received before requesting animated instances)
   CCartCache__RequestBlock(&pThis->m_Cache,
                            pThis, (pfnCACHENOTIFY) CScene__DecompressCollision, (pfnCACHENOTIFY) CScene__CollisionReceived,
                            &pThis->m_pceCollision,
                            rpCollisionAddress, collisionSize,
                            "Collision");


	// find grid bounds address	(must happen before request level info)
	pThis->m_rpGridBoundsAddress = CIndexedSet__GetROMAddressAndSize(&isLevel, pThis->m_rpLevelAddress,
																						  CART_LEVEL_isGridBounds, &pThis->m_GridBoundsSize);

	// find grid sections address (must be done before grid bounds request)
	pThis->m_rpGridSectionsAddress = CIndexedSet__GetROMAddress(&isLevel, pThis->m_rpLevelAddress,
																		  			CART_LEVEL_isGridSections);

	// find level info addres
	rpLevelInfo = CIndexedSet__GetROMAddressAndSize(&isLevel, pThis->m_rpLevelAddress,
																	CART_LEVEL_level, &levelInfoSize);
	// request level info
	CCartCache__RequestBlock(&pThis->m_Cache,
									 pThis, NULL, (pfnCACHENOTIFY) CScene__LevelInfoReceived,
									 &pThis->m_pceLevelInfo,
									 rpLevelInfo, levelInfoSize,
									 "Level Info");


	// find sky texture address
	rpSkyTextureSetAddress = CIndexedSet__GetROMAddressAndSize(&isLevel, pThis->m_rpLevelAddress,
																				  CART_LEVEL_isSkyTextureSet, &skyTextureSetSize);
	CTextureLoader__ConstructFromAddressSize(&pThis->m_SkyTextureLoader, rpSkyTextureSetAddress, skyTextureSetSize);
	pThis->m_SkyTextureReady = TRUE;


	CIndexedSet__Destruct(&isLevel);
}


void CScene__SetupRealTimeLight(CScene *pThis)
{
	int			i ;
	CVector3		vDir ;
	float			mag;

	// set up real-time light
	for (i=0; i<3; i++)
	{
		thelight.a.l.colc[i] = thelight.a.l.col[i] = pThis->m_LevelInfo.m_AmbientLight[i];
		thelight.l[0].l.colc[i] = thelight.l[0].l.col[i] = pThis->m_LevelInfo.m_DirectionalLight[i];
		thelight.l[0].l.dir[i] = pThis->m_LevelInfo.m_Direction[i];
	}

	vDir.x = pThis->m_LevelInfo.m_Direction[0];
	vDir.y = pThis->m_LevelInfo.m_Direction[1];
	vDir.z = pThis->m_LevelInfo.m_Direction[2];

	mag = CVector3__Magnitude(&vDir);
	if (mag != 0)
	{
		CVector3__MultScaler(&vDir, &vDir, 127/mag);

		pThis->m_LightDirection[0] = (signed short) vDir.x;
		pThis->m_LightDirection[1] = (signed short) vDir.y;
		pThis->m_LightDirection[2] = (signed short) vDir.z;
	}
}



void CScene__LevelInfoReceived(CScene *pThis, CCacheEntry **ppceTarget)
{
	void			*pbRomLevel;
	int			i;
	float			lighting;
	int			v;

	ASSERT(ppceTarget == &pThis->m_pceLevelInfo);
	ASSERT(pThis->m_rpGridBoundsAddress);

	pbRomLevel = CCacheEntry__GetData(pThis->m_pceLevelInfo);
	memcpy(&pThis->m_LevelInfo, pbRomLevel, sizeof(CROMLevel));
#ifdef PLATFORM_PORT
	/* CROMLevel.m_GridDistance is the ONLY big-endian (float) field; all others are BYTE/signed char. */
	pThis->m_LevelInfo.m_GridDistance = ORDERBYTES(pThis->m_LevelInfo.m_GridDistance);
#endif

	pThis->m_LevelInfoReceived = TRUE;

	// clear map if player just entered another major level
	if ((pThis->m_LevelInfo.m_bFlags & LEVEL_STORE_MAP) && (pThis->m_nLevel != GetApp()->m_LastSaveMapLevel))
	{
		CScene__ClearAllRegionFlags(pThis);

		GetApp()->m_LastSaveMapLevel = pThis->m_nLevel;
	}

	// pick microcode version
	if (pThis->m_LevelInfo.m_bFlags & LEVEL_LOW_PRECISION)
		microcode_version = MICROCODE_LOW_PRECISION;
	else
		microcode_version = MICROCODE_HIGH_PRECISION;


	// set up real-time light
	CScene__SetupRealTimeLight(pThis) ;

	// build pre-light table
	for (i=0; i<64; i++)
	{
		//lighting = i*(2.0/32.0) - 1;
		//v = (BYTE) (min(max(0, (lighting+0.5)/1.5*(1 - ambient)) + ambient, 1) * 256);
		lighting = i/63.0;
		v = max(0, min(255, lighting*255));
		//rmonPrintf("v:%d\n", v);

		light_table_red[i]	= (BYTE) min(255, ( (((int)pThis->m_LevelInfo.m_AmbientLight[0]) << 8)
												 + pThis->m_LevelInfo.m_DirectionalLight[0]*v ) >> 8);
		light_table_green[i]	= (BYTE) min(255, ( (((int)pThis->m_LevelInfo.m_AmbientLight[1]) << 8)
												 + pThis->m_LevelInfo.m_DirectionalLight[1]*v ) >> 8);
		light_table_blue[i]	= (BYTE) min(255, ( (((int)pThis->m_LevelInfo.m_AmbientLight[2]) << 8)
												 + pThis->m_LevelInfo.m_DirectionalLight[2]*v ) >> 8);

/*
		rmonPrintf("r:%d, g:%d, b:%d\n",
				light_table_red[i],
				light_table_green[i],
				light_table_blue[i]);
*/
	}

	// request grid bounds
	CCartCache__RequestBlock(&pThis->m_Cache,
									 pThis, (pfnCACHENOTIFY) CScene__DecompressGridBounds, (pfnCACHENOTIFY) CScene__GridBoundsReceived,
									 &pThis->m_pceGridBounds,
									 pThis->m_rpGridBoundsAddress, pThis->m_GridBoundsSize,
									 "Grid Bounds");

}

void CScene__DecompressCollision(CScene *pThis, CCacheEntry **ppceTarget)
{
	CIndexedSet			isCollision, isNewCollision;
	CUnindexedSet		usAttributes, usRegions, usCorners, usBlocks,
							usNewRegions, usNewCorners;
	void					*pbRegionAttributes, *pbNewRegionAttributes,
							*pbCorners, *pbNewCorners,
							*pbRegions, *pbNewRegions,
							*pbBlocks, *pbNewBlocks,
							*pbNewCollision;
	DWORD					regionAttributesSize,
							cornersSize,
							blocksSize,
							newRegionsSize;
	int					cRegion, nRegions,
							cCorner, nCorners,
							newSize,
							offset;
	BYTE					*pBytes;
	CROMRegion			*regions;
	CROMCorner			*newCorners;
	CGameRegion			*newRegions;

	ASSERT(ppceTarget == &pThis->m_pceCollision);

	TRACE0("CScene: Decompressing collision data.\r\n");

	if (!CCacheEntry__DecompressTemp(pThis->m_pceCollision))
		return;

	CIndexedSet__ConstructFromRawData(&isCollision,
												 CCacheEntry__GetData(pThis->m_pceCollision),
												 FALSE);

	pbRegionAttributes = CIndexedSet__GetBlockAndSize(&isCollision, CART_COLLISION_usRegionAttributes, &regionAttributesSize);
	CUnindexedSet__ConstructFromRawData(&usAttributes, pbRegionAttributes, FALSE);

	pbCorners = CIndexedSet__GetBlockAndSize(&isCollision, CART_COLLISION_usCorners, &cornersSize);
	CUnindexedSet__ConstructFromRawData(&usCorners, pbCorners, FALSE);

	pbRegions = CIndexedSet__GetBlock(&isCollision, CART_COLLISION_usRegions);
	CUnindexedSet__ConstructFromRawData(&usRegions, pbRegions, FALSE);

	nRegions = CUnindexedSet__GetBlockCount(&usRegions);
	TRACE("There are %d regions.\r\n", nRegions);

	pbBlocks = CIndexedSet__GetBlockAndSize(&isCollision, CART_COLLISION_usBlockBounds, &blocksSize);
	CUnindexedSet__ConstructFromRawData(&usBlocks, pbBlocks, FALSE);

	newRegionsSize = FixAlignment(US_TOTAL_SIZE(sizeof(CGameRegion), nRegions), IS_ALIGNMENT);
	newSize = IS_INDEX_SIZE(4) + regionAttributesSize + cornersSize + newRegionsSize + blocksSize;

	pBytes = CCacheEntry__AllocDataForReplacement(pThis->m_pceCollision, newSize);
	ASSERT(pBytes);	// must always be able to allocate regions!
	if (pBytes)
	{
		pbNewCollision = pBytes;
		CIndexedSet__ConstructFromRawData(&isNewCollision, pbNewCollision, FALSE);

		offset = IS_INDEX_SIZE(4);
		CIndexedSet__SetBlockCount(&isNewCollision, 0);
		CIndexedSet__SetNextOffset(&isNewCollision, offset);


		pbNewRegionAttributes = pBytes + offset;
		//memcpy(pbNewRegionAttributes, pbRegionAttributes, regionAttributesSize);
		CUnindexedSet__DeinterleaveCopy(&usAttributes, pbNewRegionAttributes, FALSE);
#ifdef PLATFORM_PORT
		/* CROMRegionSet has no decode fn — swap once here. 2 DWORD + 9 float (swap4), m_dwFlags (swap4),
		 * 5 WORD (swap2); the BYTE color/material/sound fields stay raw. */
		{
			CUnindexedSet usNewRS; int nRS, r; CROMRegionSet *rs;
			CUnindexedSet__ConstructFromRawData(&usNewRS, pbNewRegionAttributes, FALSE);	/* skip US header */
			nRS = CUnindexedSet__GetBlockCount(&usNewRS);
			rs = (CROMRegionSet*) CUnindexedSet__GetBasePtr(&usNewRS);
			for (r = 0; r < nRS; r++) {
				rs[r].m_FogStart=ORDERBYTES(rs[r].m_FogStart); rs[r].m_WaterStart=ORDERBYTES(rs[r].m_WaterStart);
				rs[r].m_FarClip=ORDERBYTES(rs[r].m_FarClip); rs[r].m_FieldOfView=ORDERBYTES(rs[r].m_FieldOfView);
				rs[r].m_WaterFarClip=ORDERBYTES(rs[r].m_WaterFarClip); rs[r].m_WaterFieldOfView=ORDERBYTES(rs[r].m_WaterFieldOfView);
				rs[r].m_WaterElevation=ORDERBYTES(rs[r].m_WaterElevation); rs[r].m_SkyElevation=ORDERBYTES(rs[r].m_SkyElevation);
				rs[r].m_SkySpeed=ORDERBYTES(rs[r].m_SkySpeed); rs[r].m_BlendLength=ORDERBYTES(rs[r].m_BlendLength);
				rs[r].m_DeathTimeDelay=ORDERBYTES(rs[r].m_DeathTimeDelay); rs[r].m_dwFlags=ORDERBYTES(rs[r].m_dwFlags);
				rs[r].m_WarpID=ORDERBYTES(rs[r].m_WarpID); rs[r].m_PressurePlateSoundNumber=ORDERBYTES(rs[r].m_PressurePlateSoundNumber);
				rs[r].m_SaveCheckpointID=ORDERBYTES(rs[r].m_SaveCheckpointID); rs[r].m_PressureID=ORDERBYTES(rs[r].m_PressureID);
				rs[r].m_DeathHitPoints=ORDERBYTES(rs[r].m_DeathHitPoints);
			}
		}
#endif

		offset += regionAttributesSize;
		CIndexedSet__SetBlockCount(&isNewCollision, 1);
		CIndexedSet__SetNextOffset(&isNewCollision, offset);


		pbNewCorners = pBytes + offset;
		//memcpy(pbNewCorners, pbCorners, cornersSize);
		CUnindexedSet__DeinterleaveCopy(&usCorners, pbNewCorners, FALSE);

		CUnindexedSet__ConstructFromRawData(&usNewCorners, pbNewCorners, FALSE);

		offset += cornersSize;
		CIndexedSet__SetBlockCount(&isNewCollision, 2);
		CIndexedSet__SetNextOffset(&isNewCollision, offset);

		pbNewRegions = pBytes + offset;
		ASSERT(sizeof(CGameRegion) >= sizeof(CROMRegion));
		CUnindexedSet__DeinterleaveCopy(&usRegions, pbNewRegions, TRUE);

		CUnindexedSet__ConstructWithAllocatedBlock(&usNewRegions,
															  	 pbNewRegions,				// data ptr
															  	 sizeof(CGameRegion),	// block size
															  	 nRegions,					// block count
															  	 FALSE);						// don't auto delete data

		regions = (CROMRegion*) CUnindexedSet__GetBasePtr(&usRegions);
		newRegions = (CGameRegion*) CUnindexedSet__GetBasePtr(&usNewRegions);
		newCorners = (CROMCorner*) CUnindexedSet__GetBasePtr(&usNewCorners);


		nCorners = CUnindexedSet__GetBlockCount(&usCorners);
		for (cCorner=0; cCorner<nCorners; cCorner++)
			CROMCorner__Decode(&newCorners[cCorner]);


		p_regions = newRegions;
		
		for (cRegion=0; cRegion<nRegions; cRegion++)
		{
			CGameRegion__TakeFromROMRegion(&newRegions[cRegion], &regions[cRegion],
													 newCorners, newRegions, nCorners);
			if (CScene__GetRegionFlag(pThis, cRegion))
				newRegions[cRegion].m_wFlags |= REGFLAG_REGIONENTERED ;
		}

		offset += newRegionsSize;
		CIndexedSet__SetBlockCount(&isNewCollision, 3);
		CIndexedSet__SetNextOffset(&isNewCollision, offset);

		// region block bounds
		pbNewBlocks = pBytes + offset;
		CUnindexedSet__DeinterleaveCopy(&usBlocks, pbNewBlocks, FALSE);
#ifdef PLATFORM_PORT
		/* CROMRegionBlock = { CBoundsRect m_BoundsRect{float MinX,MinZ,MaxX,MaxZ}; DWORD m_nRegions } —
		 * all 5 scalars big-endian (per WIN32 CROMRegionBlock__TakeFromCBounds), no decode fn. A raw
		 * big-endian m_nRegions overruns regions[] in CGameRegion__FindNextLowerRegion → wild ptr. Swap once. */
		{
			CUnindexedSet usNewBlk; int nBlocks, b; CROMRegionBlock *swb;
			CUnindexedSet__ConstructFromRawData(&usNewBlk, pbNewBlocks, FALSE);	/* blocks start AFTER the US header */
			nBlocks = CUnindexedSet__GetBlockCount(&usNewBlk);
			swb = (CROMRegionBlock*) CUnindexedSet__GetBasePtr(&usNewBlk);
			for (b = 0; b < nBlocks; b++) {
				swb[b].m_nRegions          = ORDERBYTES(swb[b].m_nRegions);
				swb[b].m_BoundsRect.m_MinX = ORDERBYTES(swb[b].m_BoundsRect.m_MinX);
				swb[b].m_BoundsRect.m_MinZ = ORDERBYTES(swb[b].m_BoundsRect.m_MinZ);
				swb[b].m_BoundsRect.m_MaxX = ORDERBYTES(swb[b].m_BoundsRect.m_MaxX);
				swb[b].m_BoundsRect.m_MaxZ = ORDERBYTES(swb[b].m_BoundsRect.m_MaxZ);
			}
		}
#endif

		offset += blocksSize;
		CIndexedSet__SetBlockCount(&isNewCollision, 4);
		CIndexedSet__SetNextOffset(&isNewCollision, offset);


		CIndexedSet__Destruct(&isNewCollision);
		CUnindexedSet__Destruct(&usNewCorners);
		CUnindexedSet__Destruct(&usNewRegions);
	}

	CCacheEntry__DeallocAndReplaceData(pThis->m_pceCollision, pBytes, newSize);

	TRACE0("CScene: ...Finished collision decompression.\r\n");

	CIndexedSet__Destruct(&isCollision);
	CUnindexedSet__Destruct(&usAttributes);
	CUnindexedSet__Destruct(&usRegions);
	CUnindexedSet__Destruct(&usCorners);
	CUnindexedSet__Destruct(&usBlocks);
}

void CScene__CollisionReceived(CScene *pThis, CCacheEntry **ppceTarget)
{
	CIndexedSet		isLevel;
	ROMAddress		*rpInstancesAddress;
	DWORD 			instancesSize;

	ASSERT(ppceTarget == &pThis->m_pceCollision);
	ASSERT(pThis->m_pceLevelIndex);

	TRACE0("CScene: Collision received.\r\n");

	CIndexedSet__ConstructFromRawData(&isLevel,
												 CCacheEntry__GetData(pThis->m_pceLevelIndex),
												 FALSE);

	// find animated instances address
	rpInstancesAddress = CIndexedSet__GetROMAddressAndSize(&isLevel, pThis->m_rpLevelAddress,
																			 CART_LEVEL_usInstances, &instancesSize);

	// request instances (will be decompressed)
   CCartCache__RequestBlock(&pThis->m_Cache,
                            pThis, (pfnCACHENOTIFY) CScene__DecompressInstances, (pfnCACHENOTIFY) CScene__InstancesReceived,
                            &pThis->m_pceInstances,
                            rpInstancesAddress, instancesSize,
                            "Instances");

	CIndexedSet__Destruct(&isLevel);
}

int CScene__GetRegionIndex(CScene *pThis, CGameRegion *pRegion)
{
	CIndexedSet		isCollision;
	void				*pbRegions;
	CUnindexedSet	usRegions;
	CGameRegion		*regions;
	int				index;

	ASSERT(pThis->m_pceCollision);

	if (!pRegion)
		return -1;

	CIndexedSet__ConstructFromRawData(&isCollision,
												 CCacheEntry__GetData(pThis->m_pceCollision),
												 FALSE);

	pbRegions = CIndexedSet__GetBlock(&isCollision, CART_COLLISION_usRegions);

	CUnindexedSet__ConstructFromRawData(&usRegions, pbRegions, FALSE);
	regions = (CGameRegion*) CUnindexedSet__GetBasePtr(&usRegions);

	index = ((DWORD)pRegion - (DWORD)regions)/sizeof(CGameRegion);
	ASSERT((index >= 0) && (index < CUnindexedSet__GetBlockCount(&usRegions)));

	CIndexedSet__Destruct(&isCollision);
	CUnindexedSet__Destruct(&usRegions);

	return index;
}


CGameRegion* CScene__GetRegionPtr(CScene *pThis, int Index)
{
	CUnindexedSet			usRegions ;
	CIndexedSet				isCollision ;
	void						*pbRegions ;
	CGameRegion				*regions, *IndexRegion ;

	ASSERT(pThis->m_pceCollision);
	if (!pThis->m_pceCollision)
		return NULL;

	// get base ptr to regions
	CIndexedSet__ConstructFromRawData(&isCollision,
												 CCacheEntry__GetData(pThis->m_pceCollision),
												 FALSE) ;


	pbRegions = CIndexedSet__GetBlock(&isCollision, CART_COLLISION_usRegions) ;

	CUnindexedSet__ConstructFromRawData(&usRegions, pbRegions, FALSE) ;
	regions = (CGameRegion*) CUnindexedSet__GetBasePtr(&usRegions) ;

	IndexRegion = &regions[Index] ;

	CIndexedSet__Destruct(&isCollision) ;
	CUnindexedSet__Destruct(&usRegions) ;

	return IndexRegion ;
}

void CScene__DecompressInstances(CScene *pThis, CCacheEntry **ppceTarget)
{
	CUnindexedSet			usInstances,
								usGameInstances,
								usRegions,
								usObjectAttributes;
	CIndexedSet				isObjectsIndex,
								isCollision;
	int						newSize,
								cInstance, nInstances,
								nTypeFlag,
								startupAnim, StartupAnimFrame ;
	BOOL						instanceFlag;
	BOOL						access ;
	void						*pNewData,
								*pbRegions;
   CROMObjectInstance   *pROMInstances, *pROMInstance;
   CGameObjectInstance  *pGameInstances, *pGameInstance;
	CGameRegion				*regions;
	CEnemyAttributes		*variations;
	INT32						BossFlags ;


#ifdef IG_DEBUG
	OSTime					startTime, endTime, diffTime;
#endif

	ASSERT(ppceTarget == &pThis->m_pceInstances);
	ASSERT(pThis->m_pceCollision);
	ASSERT(pThis->m_pceObjectsIndex);
	ASSERT(pThis->m_pceInstances);

	// must already have been received in order to decompress instances
	ASSERT(pThis->m_pceObjectsIndex);

	if (!CCacheEntry__DecompressTemp(*ppceTarget))
		return;

	TRACE0("CScene: Decompressing instance data.\r\n");
#ifdef IG_DEBUG
   startTime = osGetTime();
#endif

	// get base ptr to regions
	CIndexedSet__ConstructFromRawData(&isCollision,
												 CCacheEntry__GetData(pThis->m_pceCollision),
												 FALSE);

	pbRegions = CIndexedSet__GetBlock(&isCollision, CART_COLLISION_usRegions);

	CUnindexedSet__ConstructFromRawData(&usRegions, pbRegions, FALSE);
	regions = (CGameRegion*) CUnindexedSet__GetBasePtr(&usRegions);

	// attatch collection for CGameObjectInstance__Reset()
	CIndexedSet__ConstructFromRawData(&isObjectsIndex,
												 CCacheEntry__GetData(pThis->m_pceObjectsIndex),
												 FALSE);

	// attatch collection to old data
   CUnindexedSet__ConstructFromRawData(&usInstances,
													CCacheEntry__GetData(pThis->m_pceInstances),
													FALSE);

	nInstances = CUnindexedSet__GetBlockCount(&usInstances);
	ASSERT(nInstances >= 1);	// first one is viewer
	TRACE("There are %d animated instances.\r\n", nInstances);

	// allocate block for new data collection
   newSize = US_TOTAL_SIZE(sizeof(CGameObjectInstance), nInstances);

	pNewData = CCacheEntry__AllocDataForReplacement(pThis->m_pceInstances, newSize);
   if (pNewData)
   {
      ASSERT(sizeof(CGameObjectInstance) >= sizeof(CROMObjectInstance));
		CUnindexedSet__DeinterleaveCopy(&usInstances, pNewData, TRUE);

		// set up new collection
      CUnindexedSet__ConstructWithAllocatedBlock(&usGameInstances,
                                                 pNewData,                    // data ptr
                                                 sizeof(CGameObjectInstance), // block size
                                                 nInstances,						// block count
                                                 FALSE);                      // don't auto delete data

      // do decompression

      pROMInstances = (CROMObjectInstance*) CUnindexedSet__GetBasePtr(&usInstances);
      pGameInstances = (CGameObjectInstance*) CUnindexedSet__GetBasePtr(&usGameInstances);

		ASSERT(pThis->m_pceObjectAttributes);
		CUnindexedSet__ConstructFromRawData(&usObjectAttributes,
														CCacheEntry__GetData(pThis->m_pceObjectAttributes),
														FALSE);

		variations = (CEnemyAttributes*) CUnindexedSet__GetBasePtr(&usObjectAttributes);

		ASSERT(nInstances);

		if (pThis->m_WarpFound)
		{
			// use m_WarpPoint to position player

         pROMInstance = &pROMInstances[0];

			pROMInstance->m_nCurrentRegion = pThis->m_WarpPoint.m_nRegion;

#ifdef PLATFORM_PORT
			/* pROMInstances[0] is the player; it is decoded by TakeFromROMObjectInstance, which now
			 * ORDERBYTES m_vPos (component-wise) and m_RotY back to host order. So feed it BIG-ENDIAN
			 * values: m_WarpPoint.m_vPos is already host-order (swapped at scene.c:463) -> re-swap to
			 * big; m_RotY is stored big-endian -> decode locally, normalize, re-encode big. Without
			 * this the player double-swaps to a garbage position -> garbage camera -> world clip-rejects. */
			{
				float realRotY = ORDERBYTES(pThis->m_WarpPoint.m_RotY);   /* big -> host */
				NormalizeRotation(&realRotY);
				pROMInstance->m_RotY = ORDERBYTES((INT16)FLOAT2INT16(
				                          max(-ANGLE_PI, min(ANGLE_PI, realRotY)), ANGLE_PI));  /* host -> big */
				pROMInstance->m_vPos.x = ORDERBYTES(pThis->m_WarpPoint.m_vPos.x);  /* host -> big */
				pROMInstance->m_vPos.y = ORDERBYTES(pThis->m_WarpPoint.m_vPos.y);
				pROMInstance->m_vPos.z = ORDERBYTES(pThis->m_WarpPoint.m_vPos.z);
			}
#else
			NormalizeRotation(&pThis->m_WarpPoint.m_RotY);
			pROMInstance->m_RotY = FLOAT2INT16(max(-ANGLE_PI, min(ANGLE_PI, pThis->m_WarpPoint.m_RotY)), ANGLE_PI);

			pROMInstance->m_vPos = pThis->m_WarpPoint.m_vPos;
#endif
		}

		for (cInstance=0; cInstance<nInstances; cInstance++)
		{
         pROMInstance = &pROMInstances[cInstance];
			pGameInstance = &pGameInstances[cInstance];

			CGameObjectInstance__TakeFromROMObjectInstance(pGameInstance, pROMInstance,
																		  regions, variations);

			nTypeFlag = CGameObjectInstance__TypeFlag(pGameInstance);

			CAIDynamic__SetHealth(&pGameInstance->m_AI,
										 pGameInstance->ah.ih.m_pEA,
										 nTypeFlag);

			// Boss reset?
			BossFlags = GetApp()->m_BossFlags ;
			if ((BossFlags & BOSS_FLAG_RESET_LEVEL) && (GetApp()->m_BossLevel))
				CScene__SetInstanceFlagByIndex(pThis, cInstance, FALSE) ;

			instanceFlag = CScene__GetInstanceFlagByIndex(pThis, cInstance);

			startupAnim = -1;	// use first anim in list by default
			StartupAnimFrame = 0 ;

			switch (nTypeFlag)
			{
				// types that need setting properly
				case AI_OBJECT_DEVICE_DOOR:
					if (instanceFlag)
					{
						startupAnim = AI_ANIM_DOOR_OPEN ;
						pGameInstance->m_AI.m_dwStatusFlags |= (AI_STARTED|AI_DOOROPEN) ;
						pGameInstance->m_AI.m_dwStatusFlags2 |= AI_IGNORESOUNDEVENTS ;
						pGameInstance->m_AI.m_RetreatTimer = 0 ;
					}
					break ;

				case AI_OBJECT_DEVICE_ACTION:
				case AI_OBJECT_DEVICE_ANIMOFFSCREENACTION:

					// Setup start
					if (instanceFlag)
					{
						// Special cases
						if ((GetApp()->m_BossLevel == MANTIS_BOSS_LEVEL) &&
							 (AI_GetEA(pGameInstance)->m_Id == MANTIS_LEVEL_UP_WALLS_ID))
							CGameObjectInstance__SetGone(pGameInstance) ;
						else				
						{
							startupAnim = AI_ANIM_ACTION_GO ;
							pGameInstance->m_AI.m_dwStatusFlags |= AI_STARTED ;
							pGameInstance->m_AI.m_dwStatusFlags2 |= AI_IGNORESOUNDEVENTS ;
						}
					}
					break ;

				case AI_OBJECT_DEVICE_EXPTARGET:
					if (instanceFlag)
					{
						startupAnim = AI_ANIM_EXPTARGET_GO ;
						pGameInstance->m_AI.m_dwStatusFlags |= AI_STARTED ;
						pGameInstance->m_AI.m_dwStatusFlags |= AI_ALREADY_DEAD ;
						pGameInstance->m_AI.m_dwStatusFlags2 |= AI_IGNORESOUNDEVENTS ;
					}
					break ;

				// if elevator is set, must be at its extreme
				case AI_OBJECT_DEVICE_ELEVATOR:
				case AI_OBJECT_DEVICE_SPINELEVATOR:
				case AI_OBJECT_DEVICE_FOOTELEVATOR:

					// Set startup
					if (instanceFlag)
					{
						pGameInstance->m_AI.m_dwStatusFlags |= (AI_STARTED | AI_DOOROPEN) ;

						// store original positions
						pGameInstance->m_AI.m_Time2 = pGameInstance->ah.ih.m_vPos.y ;
						pGameInstance->m_AI.m_TeleportTime = CGameRegion__GetGroundHeight(pGameInstance->ah.ih.m_pCurrentRegion, pGameInstance->ah.ih.m_vPos.x, pGameInstance->ah.ih.m_vPos.z) ;

						pGameInstance->m_AI.m_Time1 = 1 ;		// extreme time
					}
					break ;

				case AI_OBJECT_DEVICE_PLATFORM:
					if (instanceFlag)
					{
						pGameInstance->m_AI.m_Time1 = 0 ;
						pGameInstance->m_AI.m_dwStatusFlags |= (AI_STARTED | AI_DOOROPEN) ;
						pGameInstance->m_AI.m_dwStatusFlags &= ~AI_DOORPREOPEN;
						pGameInstance->m_AI.m_dwStatusFlags2 |= AI_IGNORESOUNDEVENTS ;

						// store original positions
						pGameInstance->m_AI.m_Time2 = pGameInstance->ah.ih.m_vPos.y ;
						pGameInstance->m_AI.m_TeleportTime = CGameRegion__GetGroundHeight(pGameInstance->ah.ih.m_pCurrentRegion, pGameInstance->ah.ih.m_vPos.x, pGameInstance->ah.ih.m_vPos.z) ;
					}
					break ;



				case AI_OBJECT_DEVICE_PORTAL:
					access = FALSE ;
					switch (AI_GetEA(pGameInstance)->m_Id)
					{
						case 2:
							if (CTurokMovement.Level2Access >= MAX_KEY2)
								access = TRUE ;
							break ;
						case 3:
							if (CTurokMovement.Level3Access >= MAX_KEY3)
								access = TRUE ;
							break ;
						case 4:
							if (CTurokMovement.Level4Access >= MAX_KEY4)
								access = TRUE ;
							break ;
						case 5:
							if (CTurokMovement.Level5Access >= MAX_KEY5)
								access = TRUE ;
							break ;
						case 6:
							if (CTurokMovement.Level6Access >= MAX_KEY6)
								access = TRUE ;
							break ;
						case 7:
							if (CTurokMovement.Level7Access >= MAX_KEY7)
								access = TRUE ;
							break ;
						case 8:
							if (CTurokMovement.Level8Access >= MAX_KEY8)
								access = TRUE ;
							break ;
					}

					if ((access) || (instanceFlag))
					{
						pGameInstance->m_AI.m_Time1 = 1 ;
						pGameInstance->m_AI.m_dwStatusFlags |= AI_STARTED ;
						pGameInstance->m_AI.m_dwStatusFlags2 |= AI_IGNORESOUNDEVENTS ;
					}
					break ;

				case AI_OBJECT_DEVICE_DRAIN:
					if (instanceFlag)
					{
						pGameInstance->m_AI.m_dwStatusFlags |= (AI_STARTED | AI_DOOROPEN | AI_DOORPREOPEN) ;
						pGameInstance->m_AI.m_dwStatusFlags2 |= AI_IGNORESOUNDEVENTS ;
					}
					break ;

				case AI_OBJECT_DEVICE_FLOOD:
					if (instanceFlag)
					{
						pGameInstance->m_AI.m_dwStatusFlags |= AI_STARTED ;
						pGameInstance->m_AI.m_dwStatusFlags2 |= AI_IGNORESOUNDEVENTS ;

						CGameRegion__UnMakeInstantDeath(pGameInstance->ah.ih.m_pCurrentRegion) ;
						CGameRegion__ToWater(pGameInstance->ah.ih.m_pCurrentRegion) ;
					}
					break ;

				case AI_OBJECT_DEVICE_LOCK:
					if (instanceFlag)
					{
						pGameInstance->m_AI.m_dwStatusFlags |= (AI_STARTED | AI_DOOROPEN) ;
						pGameInstance->m_AI.m_dwStatusFlags2 |= AI_IGNORESOUNDEVENTS ;

						// store original positions
						pGameInstance->m_AI.m_Time2 = pGameInstance->ah.ih.m_vPos.y ;
						pGameInstance->m_AI.m_TeleportTime = CGameRegion__GetGroundHeight(pGameInstance->ah.ih.m_pCurrentRegion, pGameInstance->ah.ih.m_vPos.x, pGameInstance->ah.ih.m_vPos.z) ;

						pGameInstance->m_AI.m_Time1 = 1 ;		// extreme time
					}
					break ;

				// randomize the humans features
				case AI_OBJECT_CHARACTER_HUMAN1:
					if (instanceFlag)
						CGameObjectInstance__SetGone(pGameInstance);

					CAIDynamic__RandomizeHuman(AI_GetDyn(pGameInstance), AI_GetEA(pGameInstance), pGameInstance) ;
					break ;

				// reset to initial state
				case AI_OBJECT_DEVICE_ARROWTARGET:
				case AI_OBJECT_DEVICE_TIMERACTION:
				case AI_OBJECT_DEVICE_CONSTANTACTION:
				case AI_OBJECT_DEVICE_SPINDOOR:
				case AI_OBJECT_DEVICE_LASER:
				case AI_OBJECT_DEVICE_SPINPLATFORM:
				case AI_OBJECT_DEVICE_DEATHELEVATOR:
				case AI_OBJECT_DEVICE_TREXPLATFORM:
				case AI_OBJECT_DEVICE_COLLAPSINGPLATFORM:
				case AI_OBJECT_DEVICE_GATE:
				case AI_OBJECT_DEVICE_KEYFLOOR:
					break ;

// TREX ELEVATOR AND PLATFORMS NOW GONE!!
#if 0
				case AI_OBJECT_DEVICE_TREXELEVATOR:
					if (instanceFlag)
					{
						pGameInstance->m_AI.m_dwStatusFlags = AI_STARTED | AI_DOOROPEN ;
						CGameObjectInstance__SetGone(pGameInstance) ;
					}
					break ;
#endif

				case AI_OBJECT_DEVICE_CRYSTAL:
					if (BossFlags & BOSS_FLAG_MANTIS_CRYSTAL_STARTED)
					{
						pGameInstance->m_AI.m_dwStatusFlags |= AI_STARTED ;
						pGameInstance->m_AI.m_dwStatusFlags2 |= AI_IGNORESOUNDEVENTS ;
						pGameInstance->m_AI.m_TeleportTime = 0 ;
						pGameInstance->m_AI.m_Time1 = 0 ;
						pGameInstance->m_AI.m_Time2 = 0 ;

						if (BossFlags & BOSS_FLAG_MANTIS_CRYSTAL_DOWN)
						{
							startupAnim = AI_ANIM_CRYSTAL_SINK ;
							pGameInstance->m_AI.m_dwStatusFlags |= AI_DOOROPEN;
							pGameInstance->m_AI.m_TeleportTime = 0 ;

						}
						else
						{
							startupAnim = AI_ANIM_CRYSTAL_RISE ;
							pGameInstance->m_AI.m_dwStatusFlags |= AI_DOOROPEN;
							pGameInstance->m_AI.m_dwStatusFlags2 |= AI_IGNORESOUNDEVENTS ;
							pGameInstance->m_AI.m_TeleportTime = 0 ;

						}
					}
					break ;

				case AI_OBJECT_DEVICE_WALL:

					// There?
					if (instanceFlag)
						CGameObjectInstance__SetGone(pGameInstance) ;
					break ;

				case AI_OBJECT_DEVICE_STATUE:

					if (BossFlags & BOSS_FLAG_MANTIS_CACOON_OPEN)
						CGameObjectInstance__SetGone(pGameInstance) ;
					break ;

				// Boss status may need to be restored
				case AI_OBJECT_CHARACTER_MANTIS_BOSS:
				case AI_OBJECT_CHARACTER_TREX_BOSS:
				case AI_OBJECT_CHARACTER_CAMPAIGNER_BOSS:
				case AI_OBJECT_CHARACTER_LONGHUNTER_BOSS:
				case AI_OBJECT_CHARACTER_HUMVEE_BOSS:

					// Start bosses on saved frames
					if (CBossesStatus__BossWasSaved(&GetApp()->m_BossesStatus, pGameInstance))
					{
						startupAnim = CBossesStatus__BossWasSaved(&GetApp()->m_BossesStatus, pGameInstance)->m_Anim ;
						StartupAnimFrame = CBossesStatus__BossWasSaved(&GetApp()->m_BossesStatus, pGameInstance)->m_CurrentAnimFrame ;
						CBossesStatus__RestoreBossStatus(&GetApp()->m_BossesStatus, pGameInstance) ;
					}	
					break ;

				case AI_OBJECT_DEVICE_FADEINFADEOUT:

					switch(AI_GetEA(pGameInstance)->m_Id)
					{
						case LONGHUNT_LEVEL_FORCEFIELD_ID:
							if (!(GetApp()->m_BossFlags & BOSS_FLAG_LONGHUNTER_FORCEFIELD_GONE))
							{	
								pGameInstance->m_AI.m_dwStatusFlags |= AI_STARTED | AI_DOOROPEN ;
								pGameInstance->m_AI.m_dwStatusFlags2 |= AI_IGNORESOUNDEVENTS ;
								CGameObjectInstance__SetNotGone(pGameInstance) ;
							}
							break ;

						case LONGHUNT_LEVEL_WATER1_ID:
						case LONGHUNT_LEVEL_WATER2_ID:
						case LONGHUNT_LEVEL_WATER3_ID:
						case LONGHUNT_LEVEL_WATER4_ID:
//							if (GetApp()->m_BossFlags & BOSS_FLAG_LONGHUNTER_SWITCH_PRESSED)
							if (GetApp()->m_BossFlags & BOSS_FLAG_LONGHUNTER_DEAD)
							{
								pGameInstance->m_AI.m_dwStatusFlags |= AI_STARTED | AI_DOOROPEN ;
								pGameInstance->m_AI.m_dwStatusFlags2 |= AI_IGNORESOUNDEVENTS ;
								CGameObjectInstance__SetNotGone(pGameInstance) ;
							}
							break ;
					}

					break ;
#if 0
// Gone now!!
				case AI_OBJECT_DEVICE_LONGHUNTER_SWITCH:
					if (BossFlags & BOSS_FLAG_LONGHUNTER_SWITCH_PRESSED)
					{
						pGameInstance->m_AI.m_dwStatusFlags |= AI_DOOROPEN | AI_STARTED ;
						pGameInstance->m_AI.m_dwStatusFlags2 |= AI_IGNORESOUNDEVENTS ;
						startupAnim = AI_ANIM_LONGHUNTER_SWITCH_GO ;
					}
					break ;
#endif

				case AI_OBJECT_CINEMA_TUROK_ARROW:
					switch(GetApp()->m_WarpID)
					{
						case INTRO_ZOOM_TO_LOGO_WARP_ID:
							startupAnim = AI_ANIM_WEAPON_FIRE1 ;
							break ;

						default:
							startupAnim = AI_ANIM_WEAPON_IDLE ;
							break ;
					}
					break ;

				case AI_OBJECT_CHARACTER_PLAYER:
					switch(GetApp()->m_WarpID)
					{
						case INTRO_TUROK_DRAWING_BOW_WARP_ID:
							startupAnim = AI_ANIM_EXTRA1 ;
							break ;
					}
					break ;

				// non device animated instances
				default:
					if (instanceFlag)
						CGameObjectInstance__SetGone(pGameInstance);
					break;
			}

			CGameObjectInstance__Reset(pGameInstance,
												&isObjectsIndex, pThis->m_rpObjectsAddress,
												startupAnim);

			// Anim not starting on frame 0?
			if (StartupAnimFrame)
			{
				pGameInstance->m_asCurrent.m_cFrame = StartupAnimFrame ;
				CGameObjectInstance__SetAnimNotChanged(pGameInstance) ;
				CGameObjectInstance__SetNotBlending(pGameInstance) ;
				pGameInstance->m_asCurrent.m_cFrame = StartupAnimFrame ;
			}
		}

      CUnindexedSet__Destruct(&usGameInstances);
		CUnindexedSet__Destruct(&usObjectAttributes);
   }

	CCacheEntry__DeallocAndReplaceData(pThis->m_pceInstances, pNewData, newSize);

#ifdef IG_DEBUG
   endTime = osGetTime();
	diffTime = endTime - startTime;
#endif

	TRACE("CScene: ...Finished instance decompression in %8.2f msec.\r\n",
			((float)(CYCLES_TO_NSEC(diffTime)))/1000000);

	CIndexedSet__Destruct(&isCollision);
	CUnindexedSet__Destruct(&usRegions);
	CIndexedSet__Destruct(&isObjectsIndex);
	CUnindexedSet__Destruct(&usInstances);


	GetApp()->m_BossFlags &= ~BOSS_FLAG_RESET_LEVEL ;
}

void CScene__InstancesReceived(CScene *pThis, CCacheEntry **ppceTarget)
{
	CUnindexedSet			usInstances;
	CGameObjectInstance	*instances;

	ASSERT(ppceTarget == &pThis->m_pceInstances);

	CUnindexedSet__ConstructFromRawData(&usInstances,
													CCacheEntry__GetData(pThis->m_pceInstances),
													FALSE);

	// animated instances
   instances	= (CGameObjectInstance*) CUnindexedSet__GetBasePtr(&usInstances);
   ASSERT(CUnindexedSet__GetBlockCount(&usInstances));

	ASSERT(pThis->m_nInstances == 0);
	ASSERT(pThis->m_nActiveAnimInstances == 0);

	pThis->m_pPlayer = &instances[0];

	CUnindexedSet__Destruct(&usInstances);
}

// change to binary search after these are stored in order!
int CScene__LookupObjectType(CScene *pThis, int nType)
{
	CUnindexedSet	usObjectTypes;
	WORD				*objectTypes;
	WORD				wType;
	int				cObj, nObjs,
						foundObject;

	ASSERT(pThis->m_pceObjectTypes);

	CUnindexedSet__ConstructFromRawData(&usObjectTypes,
													CCacheEntry__GetData(pThis->m_pceObjectTypes),
													FALSE);

	objectTypes = (WORD*) CUnindexedSet__GetBasePtr(&usObjectTypes);
	nObjs = CUnindexedSet__GetBlockCount(&usObjectTypes);

	wType = (WORD) nType;
	foundObject = -1;

	for (cObj=0; cObj<nObjs; cObj++)
	{
		/* PORT: the Object Types block is a big-endian WORD array, consumed RAW here; on a little-
		 * endian host objectTypes[cObj] is byte-swapped, so the lookup (e.g. the player's weapon
		 * model type 100=knife) never matches -> CScene__LoadObjectModelType bails -> the player keeps
		 * the full BODY model (anim 0) instead of the first-person WEAPON model. ORDERBYTES is identity
		 * off-PLATFORM_PORT. */
		if (ORDERBYTES(objectTypes[cObj]) == wType)
		{
			foundObject = cObj;
			break;
		}
	}

	CUnindexedSet__Destruct(&usObjectTypes);

	return foundObject;
}

int CScene__GetObjectTypeFlag(CScene *pThis, int nObjType)
{
	CUnindexedSet	usObjectTypes;
	WORD				*objectTypes;

	ASSERT(pThis->m_pceObjectTypes);

	CUnindexedSet__ConstructFromRawData(&usObjectTypes,
													CCacheEntry__GetData(pThis->m_pceObjectTypes),
													FALSE);

	objectTypes = (WORD*) CUnindexedSet__GetBasePtr(&usObjectTypes);
	ASSERT(nObjType < CUnindexedSet__GetBlockCount(&usObjectTypes));

	CUnindexedSet__Destruct(&usObjectTypes);

	/* PORT: the Object Types block is a big-endian WORD array consumed RAW; on a little-endian host
	 * the reverse lookup (object index -> logical type) for simple/static instances returns a byte-
	 * swapped type, so AI_IsPickup() fails -> SIMPLE_FLAG_VISIBLE is never set (romstruc.c:1336) ->
	 * item PICKUPS never draw. (Sibling of the LookupObjectType fix.) ORDERBYTES = identity off-port. */
	return ORDERBYTES(objectTypes[nObjType]);
}

void CScene__LoadObjectModelType(CScene *pThis, CGameObjectInstance *pInstance,
											int nObjectType, int nAnimType)
{
	int				nObject;
	CIndexedSet		isObjectsIndex;

	if (!cache_is_valid)
		return;

	// No model swapping on player when in cinema mode (unless player object is required)
	if (	 (CEngineApp__GetPlayer(GetApp()) == pInstance) 
 		 && (CCamera__InCinemaMode(&GetApp()->m_Camera)) 
		 && (nObjectType != AI_OBJECT_CHARACTER_PLAYER) )
		return ;

	if (pInstance->m_nTypeFlag == nObjectType)
		return;

	if ((pThis->m_pceObjectsIndex) && (pThis->m_pceObjectTypes) && (pThis->m_rpObjectsAddress))
	{
		nObject = CScene__LookupObjectType(pThis, nObjectType);
		if (nObject != -1)
		{
			TRACE("CScene: Loading model type %d\r\n", nObjectType);

			pInstance->ah.ih.m_nObjType = nObject;
			pInstance->m_nTypeFlag = nObjectType;
			//pInstance->m_asCurrent.m_pceAnim = NULL;

			CIndexedSet__ConstructFromRawData(&isObjectsIndex,
														 CCacheEntry__GetData(pThis->m_pceObjectsIndex),
														 FALSE);

			// make sure it doesn't start with a higher animation than the object has
			//CGameObjectInstance__SetDesiredAnim(pInstance, 0);
			// load object data
			CGameObjectInstance__Reset(pInstance,
												&isObjectsIndex, pThis->m_rpObjectsAddress,
												nAnimType);

			CIndexedSet__Destruct(&isObjectsIndex);
		}
	}
}

void CScene__DecompressGridBounds(CScene *pThis, CCacheEntry **ppceTarget)
{
	CIndexedSet			isGridBounds,
							isNewGridBounds;
	void					*pbMinXs, *pbMaxXs,
							*pbMinZs, *pbMaxZs,
							*pbBounds,
							*pbNewGridBounds,
							*pbNewMinXs, *pbNewMaxXs,
							*pbNewMinZs, *pbNewMaxZs,
							*pbNewBounds;
	DWORD					minXSize, maxXSize,
							minZSize, maxZSize;
	CUnindexedSet		usBounds,
							usNewBounds;
	int					cBound, nBounds,
							newBoundsSize,
							newSize,
							offset;
	CROMGridBounds		*romGridBounds;
	CGameGridBounds	*gameGridBounds;
	BYTE					*pBytes;

	ASSERT(ppceTarget == &pThis->m_pceGridBounds);

	TRACE0("CScene: Decompressing grid bounds data.\r\n");

	if (!CCacheEntry__DecompressTemp(*ppceTarget))
		return;

	// attatch collection to old data
	CIndexedSet__ConstructFromRawData(&isGridBounds,
												 CCacheEntry__GetData(pThis->m_pceGridBounds),
												 FALSE);

	pbMinXs = CIndexedSet__GetBlockAndSize(&isGridBounds, CART_GRIDBOUNDS_usMinXs, &minXSize);
	pbMaxXs = CIndexedSet__GetBlockAndSize(&isGridBounds, CART_GRIDBOUNDS_usMaxXs, &maxXSize);
	pbMinZs = CIndexedSet__GetBlockAndSize(&isGridBounds, CART_GRIDBOUNDS_usMinZs, &minZSize);
	pbMaxZs = CIndexedSet__GetBlockAndSize(&isGridBounds, CART_GRIDBOUNDS_usMaxZs, &maxZSize);

	pbBounds = CIndexedSet__GetBlock(&isGridBounds, CART_GRIDBOUNDS_usBounds);
	CUnindexedSet__ConstructFromRawData(&usBounds, pbBounds, FALSE);

	nBounds = CUnindexedSet__GetBlockCount(&usBounds);
	TRACE("There are %d grid sections.\r\n", nBounds);

	newBoundsSize = FixAlignment(US_TOTAL_SIZE(sizeof(CGameGridBounds), nBounds), IS_ALIGNMENT);

	newSize = IS_INDEX_SIZE(5) + minXSize + maxXSize + minZSize + maxZSize + newBoundsSize;

	pBytes = CCacheEntry__AllocDataForReplacement(pThis->m_pceGridBounds, newSize);
	if (pBytes)
	{
		pbNewGridBounds = pBytes;
		CIndexedSet__ConstructFromRawData(&isNewGridBounds, pbNewGridBounds, FALSE);

		offset = IS_INDEX_SIZE(5);
		CIndexedSet__SetBlockCount(&isNewGridBounds, 0);
		CIndexedSet__SetNextOffset(&isNewGridBounds, offset);


		pbNewMinXs = pBytes + offset;
		memcpy(pbNewMinXs, pbMinXs, minXSize);
#ifdef PLATFORM_PORT
		/* swap only the float DATA (after the US header); the big-endian header count stays for GetBlockCount */
		{ CUnindexedSet _us; int _n,_i; DWORD *_d; CUnindexedSet__ConstructFromRawData(&_us,pbNewMinXs,FALSE);
		  _n=CUnindexedSet__GetBlockCount(&_us); _d=(DWORD*)CUnindexedSet__GetBasePtr(&_us);
		  for(_i=0;_i<_n;_i++) _d[_i]=ORDERBYTES(_d[_i]); }
#endif

		offset += minXSize;
		CIndexedSet__SetBlockCount(&isNewGridBounds, 1);
		CIndexedSet__SetNextOffset(&isNewGridBounds, offset);


		pbNewMaxXs = pBytes + offset;
		memcpy(pbNewMaxXs, pbMaxXs, maxXSize);
#ifdef PLATFORM_PORT
		{ CUnindexedSet _us; int _n,_i; DWORD *_d; CUnindexedSet__ConstructFromRawData(&_us,pbNewMaxXs,FALSE);
		  _n=CUnindexedSet__GetBlockCount(&_us); _d=(DWORD*)CUnindexedSet__GetBasePtr(&_us);
		  for(_i=0;_i<_n;_i++) _d[_i]=ORDERBYTES(_d[_i]); }
#endif

		offset += maxXSize;
		CIndexedSet__SetBlockCount(&isNewGridBounds, 2);
		CIndexedSet__SetNextOffset(&isNewGridBounds, offset);


		pbNewMinZs = pBytes + offset;
		memcpy(pbNewMinZs, pbMinZs, minZSize);
#ifdef PLATFORM_PORT
		{ CUnindexedSet _us; int _n,_i; DWORD *_d; CUnindexedSet__ConstructFromRawData(&_us,pbNewMinZs,FALSE);
		  _n=CUnindexedSet__GetBlockCount(&_us); _d=(DWORD*)CUnindexedSet__GetBasePtr(&_us);
		  for(_i=0;_i<_n;_i++) _d[_i]=ORDERBYTES(_d[_i]); }
#endif

		offset += minZSize;
		CIndexedSet__SetBlockCount(&isNewGridBounds, 3);
		CIndexedSet__SetNextOffset(&isNewGridBounds, offset);


		pbNewMaxZs = pBytes + offset;
		memcpy(pbNewMaxZs, pbMaxZs, minZSize);
#ifdef PLATFORM_PORT
		{ CUnindexedSet _us; int _n,_i; DWORD *_d; CUnindexedSet__ConstructFromRawData(&_us,pbNewMaxZs,FALSE);
		  _n=CUnindexedSet__GetBlockCount(&_us); _d=(DWORD*)CUnindexedSet__GetBasePtr(&_us);
		  for(_i=0;_i<_n;_i++) _d[_i]=ORDERBYTES(_d[_i]); }
#endif

		offset += maxZSize;
		CIndexedSet__SetBlockCount(&isNewGridBounds, 4);
		CIndexedSet__SetNextOffset(&isNewGridBounds, offset);


		pbNewBounds = pBytes + offset;
		CUnindexedSet__ConstructWithAllocatedBlock(&usNewBounds,
																 pbNewBounds,					// data ptr
                                                 sizeof(CGameGridBounds),	// block size
                                                 nBounds,						// block count
                                                 FALSE);                   // don't auto delete data

		romGridBounds = (CROMGridBounds*) CUnindexedSet__GetBasePtr(&usBounds);
		gameGridBounds = (CGameGridBounds*) CUnindexedSet__GetBasePtr(&usNewBounds);

		for (cBound=0; cBound<nBounds; cBound++)
			CGameGridBounds__TakeFromROMGridBounds(&gameGridBounds[cBound], &romGridBounds[cBound]);

		offset += newBoundsSize;
		CIndexedSet__SetBlockCount(&isNewGridBounds, 5);
		CIndexedSet__SetNextOffset(&isNewGridBounds, offset);

		CIndexedSet__Destruct(&isNewGridBounds);
		CUnindexedSet__Destruct(&usNewBounds);
	}

	CCacheEntry__DeallocAndReplaceData(pThis->m_pceGridBounds, pBytes, newSize);

	TRACE0("CScene: ...Finished grid bounds decompression.\r\n");

	CUnindexedSet__Destruct(&usBounds);
	CIndexedSet__Destruct(&isGridBounds);
}

void CScene__GridBoundsReceived(CScene *pThis, CCacheEntry **ppceTarget)
{
	CIndexedSet		isGridBounds;
	void				*pbBounds;
	CUnindexedSet	usBounds;
	int				nGridSections;

	ASSERT(ppceTarget == &pThis->m_pceGridBounds);
	TRACE0("CScene: Grid Bounds received.\r\n");

	CIndexedSet__ConstructFromRawData(&isGridBounds,
												 CCacheEntry__GetData(pThis->m_pceGridBounds),
												 FALSE);

	pbBounds = CIndexedSet__GetBlock(&isGridBounds, CART_GRIDBOUNDS_usBounds);
	CUnindexedSet__ConstructFromRawData(&usBounds, pbBounds, FALSE);

	nGridSections = CUnindexedSet__GetBlockCount(&usBounds);


	// request grid sections index
	// the number of entries is known from grid bounds
   CCartCache__RequestBlock(&pThis->m_Cache,
                            NULL, NULL, NULL,
                            &pThis->m_pceGridSectionsIndex,
                            pThis->m_rpGridSectionsAddress, IS_INDEX_SIZE(nGridSections),
                            "Grid Sections Index");

	CIndexedSet__Destruct(&isGridBounds);
	CUnindexedSet__Destruct(&usBounds);
}

void CScene__DecompressGridSection(CScene *pThis, CCacheEntry **ppceTarget)
{
	CCacheEntry				*pceGridSection;
	CIndexedSet				isCollision,
								isObjectsIndex,
								isGridSection,
								isNewGridSection;
	void						*pbEStaticInstances,
								*pbStaticInstances,
								*pbSimpleInstances,
								*pbNewGridSection,
								*pbNewStaticInstances,
								*pbNewSimpleInstances,
								*pbRegions;
	CUnindexedSet			usEStaticInstances,
								usStaticInstances,
								usSimpleInstances,
								usNewStaticInstances,
								usNewSimpleInstances,
								usObjectAttributes,
								usRegions;
	BYTE						*pBytes;
		//						*pickupFlags;
	int						//nGridSection,
								cEStatic, nEStatics,
								cStatic, nStatics,
								cSimple, nSimples,
								newStaticSize, newSimpleSize,
								newSize,
								offset;
   CROMEfficientStaticInstance	*romEStaticInstances;
   CROMStaticInstance   *romStaticInstances;
   CGameStaticInstance  *gameStaticInstances;
   CROMSimpleInstance   *romSimpleInstances;
   CGameSimpleInstance  *gameSimpleInstances;
	CGameRegion				*regions;
	CEnemyAttributes		*variations;
	ROMAddress				*rpObjectAddress;
#ifdef IG_DEBUG
	OSTime					startTime, endTime, diffTime;
#endif

	ASSERT(pThis->m_pceCollision);

	if (!CCacheEntry__DecompressTemp(*ppceTarget))
		return;

	TRACE0("CScene: Decompressing grid section data.\r\n");
#ifdef IG_DEBUG
   startTime = osGetTime();
#endif

	pceGridSection = *ppceTarget;

	// attatch collection to old data
   CIndexedSet__ConstructFromRawData(&isGridSection,
												 CCacheEntry__GetData(pceGridSection),
												 FALSE);

	// empty grid sections have zero entries
	if (!CIndexedSet__GetBlockCount(&isGridSection))
	{
		CIndexedSet__Destruct(&isGridSection);
		return;
	}

	pbEStaticInstances = CIndexedSet__GetBlock(&isGridSection, CART_GRIDSECTION_usEStaticInstances);
	CUnindexedSet__ConstructFromRawData(&usEStaticInstances, pbEStaticInstances, FALSE);
	nEStatics = CUnindexedSet__GetBlockCount(&usEStaticInstances);

	TRACE("There are %d efficient static instances.\r\n", nEStatics);

	pbStaticInstances = CIndexedSet__GetBlock(&isGridSection, CART_GRIDSECTION_usStaticInstances);
	CUnindexedSet__ConstructFromRawData(&usStaticInstances, pbStaticInstances, FALSE);
	nStatics = CUnindexedSet__GetBlockCount(&usStaticInstances);
	TRACE("There are %d static instances.\r\n", nStatics);

	newStaticSize = FixAlignment(US_TOTAL_SIZE(sizeof(CGameStaticInstance), nEStatics + nStatics), IS_ALIGNMENT);

	pbSimpleInstances = CIndexedSet__GetBlock(&isGridSection, CART_GRIDSECTION_usSimpleInstances);
	CUnindexedSet__ConstructFromRawData(&usSimpleInstances, pbSimpleInstances, FALSE);
	nSimples = CUnindexedSet__GetBlockCount(&usSimpleInstances);
	TRACE("There are %d simple instances.\r\n", nSimples);

	// TEMP
	//osSyncPrintf("total:%d, estat:%d, stat:%d, simp:%d\n",
	//	nEStatics + nStatics + nSimples, nEStatics, nStatics, nSimples);

	newSimpleSize = FixAlignment(US_TOTAL_SIZE(sizeof(CGameSimpleInstance), nSimples), IS_ALIGNMENT);

	newSize = IS_INDEX_SIZE(2) + newStaticSize + newSimpleSize;

	pBytes = CCacheEntry__AllocDataForReplacement(pThis->m_pceCollision, newSize);
	if (pBytes)
	{
		pbNewGridSection = pBytes;
		CIndexedSet__ConstructFromRawData(&isNewGridSection, pbNewGridSection, FALSE);

		offset = IS_INDEX_SIZE(2);
		CIndexedSet__SetBlockCount(&isNewGridSection, 0);
		CIndexedSet__SetNextOffset(&isNewGridSection, offset);

		pbNewStaticInstances = pBytes + offset;

		ASSERT(sizeof(CGameStaticInstance) >= sizeof(CROMEfficientStaticInstance));
		CUnindexedSet__DeinterleaveCopy(&usEStaticInstances, pbNewStaticInstances, TRUE);

		ASSERT(sizeof(CGameStaticInstance) >= sizeof(CROMStaticInstance));
		CUnindexedSet__DeinterleaveCopy(&usStaticInstances, pbNewStaticInstances, TRUE);


		CUnindexedSet__ConstructWithAllocatedBlock(&usNewStaticInstances,
																 pbNewStaticInstances,				// data ptr
																 sizeof(CGameStaticInstance),		// block size
																 nEStatics + nStatics,				// block count
																 FALSE);									// don't auto delete data

		romEStaticInstances = (CROMEfficientStaticInstance*) CUnindexedSet__GetBasePtr(&usEStaticInstances);
		romStaticInstances = (CROMStaticInstance*) CUnindexedSet__GetBasePtr(&usStaticInstances);
		gameStaticInstances = (CGameStaticInstance*) CUnindexedSet__GetBasePtr(&usNewStaticInstances);


		ASSERT(pThis->m_pceCollision);
		CIndexedSet__ConstructFromRawData(&isCollision,
													 CCacheEntry__GetData(pThis->m_pceCollision),
													 FALSE);

		pbRegions = CIndexedSet__GetBlock(&isCollision, CART_COLLISION_usRegions);

		CUnindexedSet__ConstructFromRawData(&usRegions, pbRegions, FALSE);
		regions = (CGameRegion*) CUnindexedSet__GetBasePtr(&usRegions);


		ASSERT(pThis->m_pceObjectsIndex);
		CIndexedSet__ConstructFromRawData(&isObjectsIndex,
													 CCacheEntry__GetData(pThis->m_pceObjectsIndex),
													 FALSE);


		ASSERT(pThis->m_pceObjectAttributes);
		CUnindexedSet__ConstructFromRawData(&usObjectAttributes,
														CCacheEntry__GetData(pThis->m_pceObjectAttributes),
														FALSE);
		variations = (CEnemyAttributes*) CUnindexedSet__GetBasePtr(&usObjectAttributes);

		// efficient static instances
		for (cEStatic=0; cEStatic<nEStatics; cEStatic++)
		{
			// find object address
			rpObjectAddress = CIndexedSet__GetROMAddress(&isObjectsIndex,
																		pThis->m_rpObjectsAddress,
																		ORDERBYTES(romEStaticInstances[cEStatic].m_nObjType));	/* PORT: big-endian WORD index */

			CGameStaticInstance__TakeFromROMEfficientStaticInstance(&gameStaticInstances[cEStatic],
																					  &romEStaticInstances[cEStatic],
																					  rpObjectAddress,
																					  regions, variations,
																					  pThis->m_LevelInfo.m_GridDistance);
		}

		// static instances
		for (cStatic=0; cStatic<nStatics; cStatic++)
		{
			// find object address
			rpObjectAddress = CIndexedSet__GetROMAddress(&isObjectsIndex,
																		pThis->m_rpObjectsAddress,
																		ORDERBYTES(romStaticInstances[cStatic].m_nObjType));	/* PORT: big-endian WORD index */

			CGameStaticInstance__TakeFromROMStaticInstance(&gameStaticInstances[cStatic + nEStatics],
																		  &romStaticInstances[cStatic],
																		  rpObjectAddress,
																		  regions, variations);
		}

		offset += newStaticSize;
		CIndexedSet__SetBlockCount(&isNewGridSection, 1);
		CIndexedSet__SetNextOffset(&isNewGridSection, offset);

		pbNewSimpleInstances = pBytes + offset;

		ASSERT(sizeof(CGameSimpleInstance) >= sizeof(CROMSimpleInstance));
		CUnindexedSet__DeinterleaveCopy(&usSimpleInstances, pbNewSimpleInstances, TRUE);


		CUnindexedSet__ConstructWithAllocatedBlock(&usNewSimpleInstances,
																 pbNewSimpleInstances,				// data ptr
																 sizeof(CGameSimpleInstance),		// block size
																 nSimples,								// block count
																 FALSE);									// don't auto delete data

		romSimpleInstances = (CROMSimpleInstance*) CUnindexedSet__GetBasePtr(&usSimpleInstances);
		gameSimpleInstances = (CGameSimpleInstance*) CUnindexedSet__GetBasePtr(&usNewSimpleInstances);

		//nGridSection = CScene__FindGridSection(pThis, ppceTarget);
		//ASSERT(nGridSection != -1);

		// simple instances
		for (cSimple=0; cSimple<nSimples; cSimple++)
		{
			// find object address
			rpObjectAddress = CIndexedSet__GetROMAddress(&isObjectsIndex,
																		pThis->m_rpObjectsAddress,
																		ORDERBYTES(romSimpleInstances[cSimple].m_nObjType));	/* PORT: big-endian WORD index */

			CGameSimpleInstance__TakeFromROMSimpleInstance(&gameSimpleInstances[cSimple],
																		  &romSimpleInstances[cSimple],
																		  rpObjectAddress,
																		  regions, variations);
			{ BOOL _pf = CScene__GetPickupFlag(pThis, &gameSimpleInstances[cSimple]);
			  if (_pf)
				gameSimpleInstances[cSimple].m_wFlags |= SIMPLE_FLAG_GONE; }
		}

		offset += newSimpleSize;
		CIndexedSet__SetBlockCount(&isNewGridSection, 2);
		CIndexedSet__SetNextOffset(&isNewGridSection, offset);


		CUnindexedSet__Destruct(&usNewStaticInstances);
      CUnindexedSet__Destruct(&usNewSimpleInstances);
		CUnindexedSet__Destruct(&usObjectAttributes);
		CIndexedSet__Destruct(&isCollision);
		CUnindexedSet__Destruct(&usRegions);
		CIndexedSet__Destruct(&isObjectsIndex);
		CIndexedSet__Destruct(&isNewGridSection);
	}

	CCacheEntry__DeallocAndReplaceData(pceGridSection, pBytes, newSize);

#ifdef IG_DEBUG
   endTime = osGetTime();
	diffTime = endTime - startTime;
#endif

	TRACE("CScene: ...Finished grid section decompression in %8.2f msec.\r\n",
			((float)(CYCLES_TO_NSEC(diffTime)))/1000000);

	CIndexedSet__Destruct(&isGridSection);
	CUnindexedSet__Destruct(&usStaticInstances);
	CUnindexedSet__Destruct(&usSimpleInstances);
}

void CScene__Draw(CScene *pThis, Gfx **ppDLP)
{
   CUnindexedSet usInstances;
	BOOL				DrawSelf ;

	// keep main indexes in cache
	if (pThis->m_pceObjectsIndex)
		CCacheEntry__ResetAge(pThis->m_pceObjectsIndex);
	if (pThis->m_pceTextureSetsIndex)
		CCacheEntry__ResetAge(pThis->m_pceTextureSetsIndex);
	if (pThis->m_pceLevelsIndex)
		CCacheEntry__ResetAge(pThis->m_pceLevelsIndex);
	if (pThis->m_pceLevelIndex)
		CCacheEntry__ResetAge(pThis->m_pceLevelIndex);
	if (pThis->m_pceGridBounds)
		CCacheEntry__ResetAge(pThis->m_pceGridBounds);
	if (pThis->m_pceCollision)
		CCacheEntry__ResetAge(pThis->m_pceCollision);
	if (pThis->m_pceGridSectionsIndex)
		CCacheEntry__ResetAge(pThis->m_pceGridSectionsIndex);
	if (pThis->m_pceObjectAttributes)
		CCacheEntry__ResetAge(pThis->m_pceObjectAttributes);
	if (pThis->m_pceObjectTypes)
		CCacheEntry__ResetAge(pThis->m_pceObjectTypes);
	if (pThis->m_pceSoundEffects)
		CCacheEntry__ResetAge(pThis->m_pceSoundEffects);
	if (pThis->m_pceBinaryTypes)
		CCacheEntry__ResetAge(pThis->m_pceBinaryTypes);
	if (pThis->m_pceBinariesIndex)
		CCacheEntry__ResetAge(pThis->m_pceBinariesIndex);

	if (pThis->m_pceObjectAttributes && pThis->m_pceObjectsIndex && pThis->m_pceCollision && pThis->m_pceInstances && pThis->m_pceTextureSetsIndex
		 && pThis->m_pceGridBounds && pThis->m_pceGridSectionsIndex && pThis->m_pceObjectTypes
		 && pThis->m_pceBinaryTypes && pThis->m_pceBinariesIndex)
	{
		cache_is_valid = TRUE;

      // attatch to usInstances
		CUnindexedSet__ConstructFromRawData(&usInstances,
														CCacheEntry__GetData(pThis->m_pceInstances),
														FALSE);

		CScene__SetUpActiveFlags(pThis, &usInstances);
		
		CScene__BuildInstanceCollisionList(pThis, &usInstances);

		// dont spin pickups during pause
		if (GetApp()->m_bPause == FALSE)
			pThis->m_RotYPickup += ANGLE_DTOR(5)*frame_increment;
		CQuatern__BuildFromAxisAngle(&pThis->m_qRotYPickup, 0,1,0, pThis->m_RotYPickup);


		// MAKE SURE THIS IS BEFORE DRAW ENVIRONMENT SO THAT IF THE DISPLAY LIST RUNS OUT, TUROK IS STILL DRAWN!!!
		// (fixes the trex level bug!!!)
		// If in cinema mode, draw player like other objects
		if (CCamera__InCinemaMode(&GetApp()->m_Camera))
		{
			DrawSelf = TRUE ;

			// Special warp id's
			// DO NOT CHANGE THIS BACK TO WARP ID (See Carl for more...)
			switch(GetApp()->m_CurrentWarpID)
			{
				case LEGALSCREEN_WARP_ID:
				case INTRO_ACCLAIM_LOGO_WARP_ID:
				case INTRO_IGGY_WARP_ID:
				case INTRO_ZOOM_TO_LOGO_WARP_ID:
				case GAME_INTRO_WARP_ID:
				case CHEAT_GALLERY_WARP_ID:
					DrawSelf = FALSE ;
					break;

				case CREDITS_WARP_ID:
					DrawSelf = FALSE ;
					CSun__ReadZDepth(&sun, ppDLP);
					gDPPipeSync((*ppDLP)++);
				#ifdef USE_FOG
					gDPSetCycleType((*ppDLP)++, G_CYC_2CYCLE);
				#else		// !USE_FOG
					gDPSetCycleType((*ppDLP)++, G_CYC_1CYCLE);
				#endif
					break ;
			}

			// Special camera modes
			switch(GetApp()->m_Camera.m_Mode)
			{
				case CAMERA_CINEMA_LONGHUNTER_MODE:
				case CAMERA_CINEMA_FIRST_HUMVEE_MODE:
				case CAMERA_CINEMA_FINAL_HUMVEE_MODE:
				case CAMERA_CINEMA_TUROK_KILL_MANTIS_MODE:
				case CAMERA_CINEMA_TUROK_KILL_TREX_MODE:
				case CAMERA_CINEMA_TUROK_KILL_LONGHUNTER_MODE:
				case CAMERA_CINEMA_TUROK_KILL_CAMPAIGNER_MODE:

					DrawSelf = FALSE ;
					break ;
			}

			// Don't draw after loaded game or selecting 'training'
			if (!((GetApp()->m_JustLoadedGame == FALSE) &&
				  ((GetApp()->m_bTrainingMode == FALSE) || (GetApp()->m_bTrainingMode == -1))))
					DrawSelf = FALSE ;

			// Do draw if needed
			if (DrawSelf)
			{
				CScene__PreDrawSelf(pThis, &usInstances, ppDLP);
				CScene__DrawSelf(pThis, ppDLP, &usInstances);
			}
		}


		CScene__DrawEnvironment(pThis, ppDLP);
		CScene__SendStaticEvents(pThis);

		CScene__DrawInstances(pThis, ppDLP, &usInstances);

		CScene__DrawTransparentInstances(pThis, ppDLP);

		CSimplePool__Advance(&pThis->m_SimplePool);
		CSimplePool__Draw(&pThis->m_SimplePool, ppDLP, pThis->m_pceTextureSetsIndex);

		// must do some player stuff before particles to position them correctly
		if (!CCamera__InCinemaMode(&GetApp()->m_Camera))
			CScene__PreDrawSelf(pThis, &usInstances, ppDLP);

		CScene__DrawSky(pThis, ppDLP);

		CScene__DrawParticles(pThis, ppDLP);

		CFxSystem__Draw(&GetApp()->m_FxSystem, ppDLP) ;

		// must draw self after particles because it
		// clears the Z buffer
		if (!CCamera__InCinemaMode(&GetApp()->m_Camera))
		{
//			if (CTurokMovement.InSun)
//				COnScreen__DrawSun(ppDLP);

			CScene__DrawSelf(pThis, ppDLP, &usInstances);
		}

		CParticleSystem__Advance(&pThis->m_ParticleSystem);

		CPickupMonitor__Update(&pThis->m_PickupMonitor) ;

		CUnindexedSet__Destruct(&usInstances);
	}
}


#ifdef PLATFORM_PORT
/* M4-S5: CROMSoundElement is a big-endian asset — 15 consecutive WORDs (5 element fields, then
 * 2 CROMEnvelopes x 5 WORDs each; SF==WORD), then 2 endian-safe trailing BYTEs. Swap a per-trigger
 * COPY (the SFX path is read-only on pElement). */
static void turokSwapSoundElement(CROMSoundElement *e)
{
	unsigned short *w = (unsigned short *)e;
	int i;
	for (i = 0; i < 15; i++) w[i] = (unsigned short)__builtin_bswap16(w[i]);
}
#endif

int CScene__DoSoundEffect(CScene *pThis,
								  int nSoundType, float Volume,
								  void *pSource, CVector3 *pvSourcePos, int SndFlags)
{
	CIndexedSet			isSoundEffects,
							isSounds;
	CUnindexedSet		usTypes,
							usSound;
	void					*pbTypes,
							*pbSounds,
							*pbSound;
	DWORD					*types,
							nSounds,
							cElement, nElements;
	int					nIndex,
							cFX;
	CROMSoundElement	*elements, *pElement;
	OSPri					ospri;

	ASSERT(pvSourcePos);

#ifdef PLATFORM_PORT
	/* PORT (M4-S5): the audio manager + players + banks now exist (initAudio runs fully), so the SFX
	 * dispatch is UN-GATED. Guard only on the ready flag so a call before initAudio doesn't reach the
	 * synth. The big-endian CROMSoundElement is byte-swapped into a local COPY per element below — the
	 * SFX path only READS pElement (SetCFXPitch/Volume copy the envelope to a channel buffer before
	 * mutating; initCFX/PlayEnvironmentSound copy field values), and never stores the pointer, so a
	 * stack copy is safe + avoids corrupting the shared (and possibly re-streamed) cart block. */
	{ extern int turok_audio_ready; if (!turok_audio_ready) return -1; }
#endif

	if (!cache_is_valid)
		return -1;

	if (!pThis->m_pceSoundEffects)
		return -1;

	// can this sfx be heard underwater
	if (		(CTurokMovement.WaterFlag == PLAYER_BELOW_WATERSURFACE)
			&&	(!AI_IsSfxUnderwater(nSoundType))
			&& (!CTurokMovement.InAntiGrav) )
	{
		return -1;
	}

	// This function can be called by the audio thread.  Since the audio thread
	// can preempt the game thread, using CCacheEntry__GetData() could corrupt
	// CCartCache if both threads modify it at the same time.  To avoid this,
	// I'm using CCacheEntry__GetDataPtr() instead of CCacheEntry__GetData().
	// This version does not modify CCartCache.
	CIndexedSet__ConstructFromRawData(&isSoundEffects,
												 CCacheEntry__GetDataPtr(pThis->m_pceSoundEffects),
												 FALSE);

	// search for type
	pbTypes = CIndexedSet__GetBlock(&isSoundEffects, CART_SOUNDEFFECTS_usTypes);
	CUnindexedSet__ConstructFromRawData(&usTypes, pbTypes, FALSE);

	nSounds = CUnindexedSet__GetBlockCount(&usTypes);
	types = (DWORD*) CUnindexedSet__GetBasePtr(&usTypes);

	nIndex = BinarySearch(types, nSounds, nSoundType);


	if (nIndex != -1)
	{
		pbSounds = CIndexedSet__GetBlock(&isSoundEffects, CART_SOUNDEFFECTS_isSounds);
		CIndexedSet__ConstructFromRawData(&isSounds, pbSounds, FALSE);
		ASSERT(nSounds == CIndexedSet__GetBlockCount(&isSounds));

		pbSound = CIndexedSet__GetBlock(&isSounds, nIndex);
		CUnindexedSet__ConstructFromRawData(&usSound, pbSound, FALSE);

		nElements = CUnindexedSet__GetBlockCount(&usSound);
		elements = (CROMSoundElement*) CUnindexedSet__GetBasePtr(&usSound);

		// It is possible that the audio thread could preempt the game thread
		// while it is inside this function.  If that happens between the time
		// AW.cfx_counter is incremented and the time the result is stored in
		// cFX, sound elements will get the wrong identifier.  The result of this
		// is usually sounds getting cut off at the wrong time.  Setting the current
		// thread to a higher priority than the audio thread delays the preemption
		// until the result is copied into cFX, fixing this problem.
		ospri = osGetThreadPri(NULL);
		osSetThreadPri(NULL, PRIORITY_AUDIOLOCK);
		cFX = ++AW.cfx_counter;
		osSetThreadPri(NULL, ospri);


		for (cElement=0; cElement<nElements; cElement++)
		{
			pElement = &elements[cElement];
#ifdef PLATFORM_PORT
			{
				CROMSoundElement elSwap = *pElement;   /* big-endian -> host copy (path is read-only) */
				turokSwapSoundElement(&elSwap);
				DoSoundElement(&elSwap, pSource, pvSourcePos, Volume, nSoundType, cFX, SndFlags);
			}
#else
			DoSoundElement(pElement, pSource,
								pvSourcePos,
								Volume,
								nSoundType, cFX, SndFlags);
#endif
		}

		CIndexedSet__Destruct(&isSounds);
		CUnindexedSet__Destruct(&usSound);
	}

	CIndexedSet__Destruct(&isSoundEffects);
	CUnindexedSet__Destruct(&usTypes);

	return AW.cfx_counter;
}

BOOL CScene__Idle(CScene *pThis)
{
	//return CCartCache__Compress(&pThis->m_Cache);
	return FALSE;
}

void CScene__DrawParticles(CScene *pThis, Gfx **ppDLP)
{
	// 1 makes ground aligned particles look better
	gSPClipRatio((*ppDLP)++, FRUSTRATIO_1);

	CParticleSystem__Draw(&pThis->m_ParticleSystem, ppDLP);
}

void CScene__DrawInstances(CScene *pThis, Gfx **ppDLP, CUnindexedSet *pusInstances)
{
	int 						cInstance, nInstances;
	CGameObjectInstance	*instances, *pInst;

	ASSERT(pThis->m_pceTextureSetsIndex);

	// animated instances

   nInstances = CUnindexedSet__GetBlockCount(pusInstances);
   instances = (CGameObjectInstance*) CUnindexedSet__GetBasePtr(pusInstances);

	// performance tuned
	gSPClipRatio((*ppDLP)++, FRUSTRATIO_5);
	// repeated in CGameObjectInstance__DrawShadow()

	// skip self
   for (cInstance=1; cInstance<nInstances; cInstance++)
	{
		pInst = &instances[cInstance];

		if (pInst->m_pceObjectInfo)
		{
			if (pInst->m_asCurrent.m_pceAnim)
			{
				if ((CBoundsRect__IsOverlapping(&pInst->m_BoundsRect, &anim_bounds_rect) || pInst->m_pBoss) && CScene__IsActive(pThis, pInst))
				{
					if (CGameObjectInstance__IsVisible(pInst) && CGameObjectInstance__HasTransparency(pInst))
					{
						CScene__AddTransparentInstance(pThis, &pInst->ah.ih);
					}
					else
					{
						CGameObjectInstance__Draw(pInst, ppDLP,
														  pThis->m_pceTextureSetsIndex);
					}
				}
				else
				{
					CGameObjectInstance__WentOutOfAnimBounds(pInst);
				}
			}
			else
			{
				if ((CBoundsRect__IsOverlapping(&pInst->m_BoundsRect, &anim_bounds_rect) || pInst->m_pBoss) && CScene__IsActive(pThis, pInst))
				{
					CGameObjectInstance__Draw(pInst, ppDLP,
													  pThis->m_pceTextureSetsIndex);
				}
			}
		}
	}
		

/*		
		if (CGameObjectInstance__IsVisible(pInst) && CGameObjectInstance__HasTransparency(pInst))
		{
			CScene__AddTransparentInstance(pThis, &pInst->ah.ih);
		}
		else
		{
			CGameObjectInstance__Draw(pInst, ppDLP,
											  pThis->m_pceTextureSetsIndex);
		}
	}
*/
}

BOOL CScene__BetterSelfClipping(CScene *pThis, CGameObjectInstance *pSelf)
{
	switch (CGameObjectInstance__TypeFlag(pSelf))
	{
		//case AI_OBJECT_WEAPON_KNIFE:
		//case AI_OBJECT_WEAPON_TOMAHAWK:
		case AI_OBJECT_WEAPON_TEKBOW:
		//case AI_OBJECT_WEAPON_SEMIAUTOMATICPISTOL:
		//case AI_OBJECT_WEAPON_ASSAULTRIFLE:
		//case AI_OBJECT_WEAPON_MACHINEGUN:
		case AI_OBJECT_WEAPON_RIOTSHOTGUN:
		//case AI_OBJECT_WEAPON_AUTOMATICSHOTGUN:
		//case AI_OBJECT_WEAPON_MINIGUN:
		//case AI_OBJECT_WEAPON_GRENADE_LAUNCHER:
		//case AI_OBJECT_WEAPON_TECHWEAPON1:
		//case AI_OBJECT_WEAPON_TECHWEAPON2:
		case AI_OBJECT_WEAPON_ROCKET:
		//case AI_OBJECT_WEAPON_SHOCKWAVE:
		case AI_OBJECT_WEAPON_CHRONOSCEPTER:
			return TRUE;

		default:
			return FALSE;
	}
}

void CScene__PreDrawSelf(CScene *pThis, CUnindexedSet *pusInstances, Gfx **ppDLP)
{
	int 						nInstances;
	CGameObjectInstance	*instances;

	// Don't draw player during intro
// CARL
//	if (GetApp()->m_Mode == MODE_INTRO || GetApp()->m_Mode == MODE_GALLERY)
//		return ;

   nInstances = CUnindexedSet__GetBlockCount(pusInstances);
   instances = (CGameObjectInstance*) CUnindexedSet__GetBasePtr(pusInstances);

	if (nInstances)
		CGameObjectInstance__PreDrawPlayer(&instances[0], ppDLP);
}

void CScene__DrawSelf(CScene *pThis, Gfx **ppDLP, CUnindexedSet *pusInstances)
{
	int 						nInstances;
	CGameObjectInstance	*instances, *pSelf;
	int						sx, sy, ex, ey;


	CSun__ReadZDepth(&sun, ppDLP);

	if (!CCamera__InCinemaMode(&GetApp()->m_Camera))
	{
		COnScreen__Init16BitDraw(ppDLP, 100);
		COnScreen__DrawSunLensFlare(&GetApp()->m_OnScreen, ppDLP);

		if (!(GetApp()->m_dwCheatFlags & CHEATFLAG_QUACKMODE))
			gDPSetTextureFilter((*ppDLP)++, G_TF_BILERP);

		gDPSetTexturePersp((*ppDLP)++, G_TP_PERSP);
		CGeometry__ResetDrawModes();
	}

	// Don't draw player during intro
// CARL
//	if (GetApp()->m_Mode == MODE_INTRO || GetApp()->m_Mode == MODE_GALLERY)
//	{
//		gDPPipeSync((*ppDLP)++);
//
//#ifdef USE_FOG
//		gDPSetCycleType((*ppDLP)++, G_CYC_2CYCLE);
//#else		// !USE_FOG
//		gDPSetCycleType((*ppDLP)++, G_CYC_1CYCLE);
//#endif
//		return ;
//	}


	ASSERT(pThis->m_pceTextureSetsIndex);

	// Only clear z buffer if not in cinema mode
	if (CCamera__InCinemaMode(&GetApp()->m_Camera))
	{
		gDPPipeSync((*ppDLP)++);

#ifdef USE_FOG
		gDPSetCycleType((*ppDLP)++, G_CYC_2CYCLE);
#else		// !USE_FOG
		gDPSetCycleType((*ppDLP)++, G_CYC_1CYCLE);
#endif
	}
	else
	{
		if (CTMove__ClearWeaponZBuffer(&CTurokMovement))
		{
			switch (CTurokMovement.WeaponCurrent)
			{
				case AI_OBJECT_WEAPON_KNIFE:
					sx = 0;
					sy = SCREEN_HT*2/5;
					ex = SCREEN_WD-1;
					ey = SCREEN_HT-1;
					break;

				case AI_OBJECT_WEAPON_TOMAHAWK:
					sx = 0;
					sy = SCREEN_HT*2/5;
					ex = SCREEN_WD-1;
					ey = SCREEN_HT-1;
					break;

				case AI_OBJECT_WEAPON_TEKBOW:
					sx = 80;
					sy = SCREEN_HT*3/5;
					ex = SCREEN_WD-1;
					ey = SCREEN_HT-1;
					break;

				case AI_OBJECT_WEAPON_SEMIAUTOMATICPISTOL:
					sx = 180;
					sy = 155;
					ex = SCREEN_WD-30;
					ey = SCREEN_HT-1;
					break;

				case AI_OBJECT_WEAPON_ASSAULTRIFLE:
					sx = 110;
					sy = 135;
					ex = SCREEN_WD-20;
					ey = SCREEN_HT-1;
					break;

				case AI_OBJECT_WEAPON_MACHINEGUN:
					sx = 170;
					sy = 90;
					ex = SCREEN_WD-1;
					ey = SCREEN_HT-1;
					break;

				case AI_OBJECT_WEAPON_RIOTSHOTGUN:
					sx = 50;
					sy = 80;
					ex = SCREEN_WD-1;
					ey = SCREEN_HT-1;
					break;

				case AI_OBJECT_WEAPON_AUTOMATICSHOTGUN:
					sx = 190;
					sy = 150;
					ex = SCREEN_WD-1;
					ey = SCREEN_HT-1;
					break;

				case AI_OBJECT_WEAPON_MINIGUN:
					sx = 140;
					sy = 145;
					ex = SCREEN_WD-1;
					ey = SCREEN_HT-1;
					break;

				case AI_OBJECT_WEAPON_GRENADE_LAUNCHER:
					sx = 220;
					sy = 110;
					ex = SCREEN_WD-1;
					ey = SCREEN_HT-1;
					break;

				case AI_OBJECT_WEAPON_TECHWEAPON1:
					sx = 110;
					sy = 170;
					ex = SCREEN_WD-1;
					ey = SCREEN_HT-1;
					break;

				case AI_OBJECT_WEAPON_ROCKET:
					sx = 194;
					sy = 135;
					ex = SCREEN_WD-1;
					ey = SCREEN_HT-1;
					break;

				case AI_OBJECT_WEAPON_SHOCKWAVE:
					sx = 157;
					sy = 150;
					ex = SCREEN_WD-1;
					ey = SCREEN_HT-1;
					break;

				case AI_OBJECT_WEAPON_TECHWEAPON2:
					sx = 150;
					sy = 135;
					ex = SCREEN_WD-1;
					ey = SCREEN_HT-1;
					break;

				case AI_OBJECT_WEAPON_CHRONOSCEPTER:
					sx = 120;
					sy = 0;
					ex = SCREEN_WD-1;
					ey = SCREEN_HT-1;
					break;

				// fills 2/5 of the screen
				default:
					sx = 0;
					sy = SCREEN_HT*2/5;
					ex = SCREEN_WD-1;
					ey = SCREEN_HT-1;
					break;
			}

			// clear the z buffer to the correct dimensions
			CEngineApp__ClearZBuffer(GetApp(), sx, ex, sy, ey);
		}
	}

#ifdef USE_FOG
	gSPSetGeometryMode((*ppDLP)++, G_FOG);
#endif

   nInstances = CUnindexedSet__GetBlockCount(pusInstances);
   instances = (CGameObjectInstance*) CUnindexedSet__GetBasePtr(pusInstances);

	if (nInstances)
	{
		pSelf = &instances[0];

		if (CScene__BetterSelfClipping(pThis, pSelf))
		{
			gSPClipRatio((*ppDLP)++, FRUSTRATIO_1);
		}
		else
		{
			// performance tuned
			gSPClipRatio((*ppDLP)++, FRUSTRATIO_2);
		}

		if (pSelf->m_pceObjectInfo)
		{
			CGameObjectInstance__Draw(pSelf, ppDLP,
											  pThis->m_pceTextureSetsIndex);
		}
	}
}

/*
int CScene__FindGridSection(CScene *pThis, CCacheEntry **ppceGridSection)
{
	CIndexedSet			isGridBounds;
	void					*pbBounds;
	CUnindexedSet		usBounds;
	CGameGridBounds	*gridBounds;
	int					cBound, nBounds,
							found;

	ASSERT(pThis->m_pceGridBounds);

	CIndexedSet__ConstructFromRawData(&isGridBounds,
												 CCacheEntry__GetData(pThis->m_pceGridBounds),
												 FALSE);

	pbBounds = CIndexedSet__GetBlock(&isGridBounds, CART_GRIDBOUNDS_usBounds);
	CUnindexedSet__ConstructFromRawData(&usBounds, pbBounds, FALSE);

	nBounds = CUnindexedSet__GetBlockCount(&usBounds);
	gridBounds = (CGameGridBounds*) CUnindexedSet__GetBasePtr(&usBounds);

	found = -1;
	for (cBound=0; cBound<nBounds; cBound++)
	{
		if (&gridBounds[cBound].m_pceGridSection == ppceGridSection)
		{
			found = cBound;
			break;
		}
	}

	CIndexedSet__Destruct(&isGridBounds);
	CUnindexedSet__Destruct(&usBounds);

	ASSERT(found != -1);
	return found;
}
*/

void CScene__GetGridSections(CScene *pThis, CBoundsRect *pBoundsRect,
									  CGameGridBounds **ppGrids[], int *pIndices[], int *pnGrids)
{
#define MAX_GRID	128
	int					nGridSections,
							cX, nXs,
							cZ, nZs,
							leftX, rightX,
							leftZ, rightZ,
							index;
	CIndexedSet			isGridBounds;
	CUnindexedSet		usMinXs, usMaxXs,
							usMinZs, usMaxZs,
							usBounds;
	float					*minXs, *maxXs,
							*minZs, *maxZs;
	CGameGridBounds	*gridBounds, *pBounds;
	void					*pbMinXs, *pbMaxXs,
							*pbMinZs, *pbMaxZs,
							*pbBounds;
	static CGameGridBounds	*pgrids[MAX_GRID];
	static int					indices[MAX_GRID];

	ASSERT(pBoundsRect);
	ASSERT(ppGrids);
	ASSERT(pnGrids);

	if (!pThis->m_pceGridBounds)
	{
		*pnGrids = 0;
		return;
	}

	nGridSections = 0;

	CIndexedSet__ConstructFromRawData(&isGridBounds,
												 CCacheEntry__GetData(pThis->m_pceGridBounds),
												 FALSE);

	pbMinXs = CIndexedSet__GetBlock(&isGridBounds, CART_GRIDBOUNDS_usMinXs);
	CUnindexedSet__ConstructFromRawData(&usMinXs, pbMinXs, FALSE);
	nXs = CUnindexedSet__GetBlockCount(&usMinXs);
	minXs = (float*) CUnindexedSet__GetBasePtr(&usMinXs);

	pbMaxXs = CIndexedSet__GetBlock(&isGridBounds, CART_GRIDBOUNDS_usMaxXs);
	CUnindexedSet__ConstructFromRawData(&usMaxXs, pbMaxXs, FALSE);
	maxXs = (float*) CUnindexedSet__GetBasePtr(&usMaxXs);

	pbMinZs = CIndexedSet__GetBlock(&isGridBounds, CART_GRIDBOUNDS_usMinZs);
	CUnindexedSet__ConstructFromRawData(&usMinZs, pbMinZs, FALSE);
	nZs = CUnindexedSet__GetBlockCount(&usMinZs);
	minZs = (float*) CUnindexedSet__GetBasePtr(&usMinZs);

	pbMaxZs = CIndexedSet__GetBlock(&isGridBounds, CART_GRIDBOUNDS_usMaxZs);
	CUnindexedSet__ConstructFromRawData(&usMaxZs, pbMaxZs, FALSE);
	maxZs = (float*) CUnindexedSet__GetBasePtr(&usMaxZs);

	pbBounds = CIndexedSet__GetBlock(&isGridBounds, CART_GRIDBOUNDS_usBounds);
	CUnindexedSet__ConstructFromRawData(&usBounds, pbBounds, FALSE);
	gridBounds = (CGameGridBounds*) CUnindexedSet__GetBasePtr(&usBounds);
	ASSERT((nXs*nZs) == CUnindexedSet__GetBlockCount(&usBounds));
	ASSERT(nXs && nZs);

	leftX = 0;
	while ((leftX < nXs) && (pBoundsRect->m_MinX > maxXs[leftX]))
		leftX++;

	rightX = nXs - 1;
	while ((rightX >= 0) && (pBoundsRect->m_MaxX < minXs[rightX]))
		rightX--;

	leftZ = 0;
	while ((leftZ < nZs) && (pBoundsRect->m_MinZ > maxZs[leftZ]))
		leftZ++;

	rightZ = nZs - 1;
	while ((rightZ >= 0) && (pBoundsRect->m_MaxZ < minZs[rightZ]))
		rightZ--;

	for (cZ=leftZ; cZ<=rightZ; cZ++)
	{
		for (cX=leftX; cX<=rightX; cX++)
		{
			if (nGridSections != MAX_GRID)
			{
				index = cX + cZ*nXs;

				pBounds = &gridBounds[index];
				if (CBoundsRect__IsOverlapping(&pBounds->m_BoundsRect, pBoundsRect))
				{
					indices[nGridSections] = index;
					pgrids[nGridSections] = pBounds;

					nGridSections++;
				}
			}
		}
	}

	*ppGrids = pgrids;
	*pIndices = indices;
	*pnGrids = nGridSections;


	CIndexedSet__Destruct(&isGridBounds);
	CUnindexedSet__Destruct(&usMinXs);
	CUnindexedSet__Destruct(&usMaxXs);
	CUnindexedSet__Destruct(&usMinZs);
	CUnindexedSet__Destruct(&usMaxZs);
	CUnindexedSet__Destruct(&usBounds);
}

void CScene__SetUpActiveFlags(CScene *pThis, CUnindexedSet *pusAnimInstances)
{
	CGameObjectInstance	*instances, *pInstance;
	CROMRegionSet			*pRegionSet;

   instances = (CGameObjectInstance*) CUnindexedSet__GetBasePtr(pusAnimInstances);
	ASSERT(CUnindexedSet__GetBlockCount(pusAnimInstances));

	pInstance = &instances[0];

	pRegionSet = CScene__GetRegionAttributes(pThis, pInstance->ah.ih.m_pCurrentRegion);

	if (pRegionSet)
		pThis->m_bPlayerActiveFlags = pRegionSet->m_bActiveFlags;
	else
		pThis->m_bPlayerActiveFlags = (BYTE) -1;

}

void CScene__BuildInstanceCollisionList(CScene *pThis, CUnindexedSet *pusAnimInstances)
{
	CUnindexedSet			usStaticInstances,
								usSimpleInstances;
	CIndexedSet				isGridSections, isGridSection;
	void						*pbStaticInstances,
								*pbSimpleInstances;
	int						cGrid, nGrids,
								cStatInst, nStatInsts,
								cSimpleInst, nSimpleInsts,
								*indices;
	CGameGridBounds		**pGridBounds, *pBounds;
	ROMAddress				*rpGridSectionAddress;
	DWORD						gridSectionSize;
	CGameStaticInstance	*staticInstances, *pStatInstance;
	CGameObjectInstance	*instances, *pInstance;
	CGameSimpleInstance	*simpleInstances, *pSimpleInstance;
	int 						cInstance, nInstances;
	BOOL						collision, hasEvents;

	pThis->m_nInstances = 0;
	pThis->m_nActiveAnimInstances = 0;
	pThis->m_nStaticEventsInstances = 0;

	// animated instances
   instances	= (CGameObjectInstance*) CUnindexedSet__GetBasePtr(pusAnimInstances);
   nInstances	= CUnindexedSet__GetBlockCount(pusAnimInstances);

	// always add player to active instance list
	ASSERT(nInstances >= 1);
	pInstance = &instances[0];
	CScene__AddActiveAnimInstance(pThis, pInstance);
	if (pInstance->m_pceObjectInfo && !CGameObjectInstance__IsGone(pInstance))
		if (pInstance->ah.ih.m_pEA->m_dwTypeFlags & AI_TYPE_COLLISION)
			if (CBoundsRect__IsOverlapping(&pInstance->m_BoundsRect, &anim_bounds_rect))
				CScene__AddCollisionInstance(pThis, &pInstance->ah.ih);

	// COMBINE WITH DRAW INSTANCES?
	for (cInstance=1; cInstance<nInstances; cInstance++)
	{
		pInstance = &instances[cInstance];

		// pInstance->m_BoundsRect isn't available until (pInstance->m_pceObjectInfo != NULL)
		if (pInstance->m_pceObjectInfo && !CGameObjectInstance__IsGone(pInstance))
		{
			if (CBoundsRect__IsOverlapping(&pInstance->m_BoundsRect, &anim_bounds_rect) && CScene__IsActive(pThis, pInstance))
			{
				CScene__AddActiveAnimInstance(pThis, pInstance);

				if (pInstance->ah.ih.m_pEA->m_dwTypeFlags & AI_TYPE_COLLISION)
					CScene__AddCollisionInstance(pThis, &pInstance->ah.ih);
			}
		}
	}

	// add static instances if grid section bounds rect intersects anim rect
	if (pThis->m_pceGridBounds && pThis->m_pceGridSectionsIndex)// && pThis->m_pceGridPickupCounts)
	{
		CScene__GetGridSections(pThis, &anim_bounds_rect,
										&pGridBounds, &indices, &nGrids);

		CIndexedSet__ConstructFromRawData(&isGridSections,
													 CCacheEntry__GetData(pThis->m_pceGridSectionsIndex),
													 FALSE);

		for (cGrid=0; cGrid<nGrids; cGrid++)
		{
			pBounds = pGridBounds[cGrid];

			rpGridSectionAddress = CIndexedSet__GetROMAddressAndSize(&isGridSections, pThis->m_rpGridSectionsAddress,
																						indices[cGrid], &gridSectionSize);

			// make sure we use current cache entry
			pBounds->m_pceGridSection = NULL;

			CCartCache__RequestBlock(&pThis->m_Cache,
                        			 //pThis, (pfnCACHENOTIFY) CScene__DecompressStaticInstances, NULL,
											 pThis, (pfnCACHENOTIFY) CScene__DecompressGridSection, NULL,
                        			 &pBounds->m_pceGridSection,
                        			 rpGridSectionAddress, gridSectionSize,
                        			 "Grid Section");

			if (pBounds->m_pceGridSection)
			{
				CIndexedSet__ConstructFromRawData(&isGridSection,
															 CCacheEntry__GetData(pBounds->m_pceGridSection),
															 FALSE);

				// there are zero blocks in an empty grid section
				if (CIndexedSet__GetBlockCount(&isGridSection))
				{
					pbStaticInstances = CIndexedSet__GetBlock(&isGridSection, 0);

					CUnindexedSet__ConstructFromRawData(&usStaticInstances, pbStaticInstances, FALSE);

					staticInstances = (CGameStaticInstance*) CUnindexedSet__GetBasePtr(&usStaticInstances);
					nStatInsts = CUnindexedSet__GetBlockCount(&usStaticInstances);

					for (cStatInst=0; cStatInst<nStatInsts; cStatInst++)
					{
						pStatInstance = &staticInstances[cStatInst];

						if (pStatInstance->ih.m_pEA)
						{
							collision = pStatInstance->ih.m_pEA->m_dwTypeFlags & AI_TYPE_COLLISION;
							hasEvents = pStatInstance->m_dwFlags & STATIC_INSTANCE_HASEVENTS;
							if (collision || hasEvents)
							{
								// check if instance bounds intersects animation rect
								if (CBoundsRect__IsOverlappingBounds(&anim_bounds_rect, &pStatInstance->m_Bounds) && CScene__IsActive(pThis, pStatInstance))
								{
									if (collision)
										CScene__AddCollisionInstance(pThis, &pStatInstance->ih);

									if (hasEvents)
										CScene__AddStaticEventsInstance(pThis, pStatInstance);
								}
							}
						}
					}

					pbSimpleInstances = CIndexedSet__GetBlock(&isGridSection, 1);

					CUnindexedSet__ConstructFromRawData(&usSimpleInstances, pbSimpleInstances, FALSE);

					simpleInstances = CUnindexedSet__GetBasePtr(&usSimpleInstances);
					nSimpleInsts = CUnindexedSet__GetBlockCount(&usSimpleInstances);

					for (cSimpleInst=0; cSimpleInst<nSimpleInsts; cSimpleInst++)
					{
						pSimpleInstance = &simpleInstances[cSimpleInst];

						if (!(pSimpleInstance->m_wFlags & (SIMPLE_FLAG_GONE | SIMPLE_FLAG_CLOSEDWARP)))
						{
							// check if instance bounds intersects animation rect
							if (CBoundsRect__IsOverlappingBounds(&anim_bounds_rect, &pSimpleInstance->m_Bounds) && CScene__IsActive(pThis, pSimpleInstance))
								CScene__AddCollisionInstance(pThis, &pSimpleInstance->ah.ih);
						}
					}

					CUnindexedSet__Destruct(&usStaticInstances);
					CUnindexedSet__Destruct(&usSimpleInstances);
					CIndexedSet__Destruct(&isGridSection);
				}
			}
		}

		CIndexedSet__Destruct(&isGridSections);
	}

	// add pickups
	CSimplePool__AddToCollisionList(&pThis->m_SimplePool, pThis);
}


void CScene__SendStaticEvents(CScene *pThis)
{
	int				cInst;
	BOOL				constantRateFrame;

	pThis->m_cSendStaticEventsFrame += frame_increment ;

	constantRateFrame = (pThis->m_cSendStaticEventsFrame > 0);
	if (constantRateFrame)
		pThis->m_cSendStaticEventsFrame -= ((int) pThis->m_cSendStaticEventsFrame) + 1;

	for (cInst=0; cInst<pThis->m_nStaticEventsInstances; cInst++)
		CGameStaticInstance__SendEvents(pThis->m_pStaticEventsInstances[cInst], &pThis->m_Cache, constantRateFrame);
}

typedef struct SortStruct_t
{
	void	*p;
	float	dist;
} SortStruct;

void SortSwap(const void *pA, const void *pB)
{
	SortStruct temp;

	temp = *((SortStruct*)pA);
	*((SortStruct*)pA) = *((SortStruct*)pB);
	*((SortStruct*)pB) = temp;
}

int RevSortCompare(const void *pA, const void *pB)
{
	return ((((SortStruct*)pA)->dist < ((SortStruct*)pB)->dist)) ? -1 : 1;
}
int SortCompare(const void *pA, const void *pB)
{
	return ((((SortStruct*)pA)->dist > ((SortStruct*)pB)->dist)) ? -1 : 1;
}

#define MAX_SORT 2048
void CScene__DrawEnvironment(CScene *pThis, Gfx **ppDLP)
{
	CUnindexedSet			usStaticInstances,
								usSimpleInstances;
	CIndexedSet				isGridSections, isGridSection;
	void						*pbStaticInstances,
								*pbSimpleInstances;
	int						cGrid, nGrids,
								cStatInst, nStatInsts;
	CGameGridBounds		**pGridBounds, *pBounds;
	ROMAddress				*rpGridSectionAddress;
	DWORD						gridSectionSize;
	CGameStaticInstance	*staticInstances, *pInstance;
	CGameSimpleInstance	*simpleInstances, *pSimple;
	int						gridIndices[MAX_GRID];
	int						cVisible,
								cVisInst, nVisInsts,
								cSimpleInst, nSimpleInsts,
								*indices;
	float						xCenter2, yCenter2, zCenter2,
								xPos2, yPos2, zPos2;
	SortStruct				sorts[MAX_SORT], *pSort;
	CEngineApp				*pApp;

	ASSERT(pThis->m_pceGridBounds);
	ASSERT(pThis->m_pceGridSectionsIndex);
	ASSERT(pThis->m_pceTextureSetsIndex);

	pThis->m_nTransparentInstances = 0;

	CScene__GetGridSections(pThis, &view_bounds_rect,
									&pGridBounds, &indices, &nGrids);

	ASSERT(nGrids <= MAX_GRID);
	ASSERT(MAX_SORT >= MAX_GRID);

	CIndexedSet__ConstructFromRawData(&isGridSections,
												 CCacheEntry__GetData(pThis->m_pceGridSectionsIndex),
												 FALSE);

	pApp = GetApp();

	xPos2 = -2*CEngineApp__GetEyePos(pApp).x;
	yPos2 = -2*CEngineApp__GetEyePos(pApp).y;
	zPos2 = -2*CEngineApp__GetEyePos(pApp).z;

	// TEMP
	//nGrids = (frame_number/5)%(nGrids + 1);

	// make list of visible grid rects
	for (cGrid=0; cGrid<nGrids; cGrid++)
	{
		pBounds = pGridBounds[cGrid];
		pSort = &sorts[cGrid];

		//xCenter2 = pBounds->m_BoundsRect.m_MinX + pBounds->m_BoundsRect.m_MaxX;
		//zCenter2 = pBounds->m_BoundsRect.m_MinZ + pBounds->m_BoundsRect.m_MaxZ;
		xCenter2 = pBounds->m_BoundsRect.m_MinX + pBounds->m_BoundsRect.m_MaxX + xPos2;
		zCenter2 = pBounds->m_BoundsRect.m_MinZ + pBounds->m_BoundsRect.m_MaxZ + zPos2;

		pSort->p = (void*) cGrid;
		//pSort->dist = abs(xPos2 - xCenter2) + abs(zPos2 - zCenter2);
		pSort->dist = xCenter2*xCenter2 + zCenter2*zCenter2;
	}

#ifdef USE_ZBUFFER
	qsort(sorts, nGrids, sizeof(SortStruct), RevSortCompare, SortSwap);
#else
	qsort(sorts, nGrids, sizeof(SortStruct), SortCompare, SortSwap);
#endif

	for (cGrid=0; cGrid<nGrids; cGrid++)
		gridIndices[cGrid] = (int) sorts[cGrid].p;

	if (CTMove__IsClimbing(&CTurokMovement))
	{
		gSPClipRatio((*ppDLP)++, FRUSTRATIO_1);
	}
	else
	{
		// performance tuned
		gSPClipRatio((*ppDLP)++, FRUSTRATIO_2);
	}

	// draw
	for (cVisible=0; cVisible<nGrids; cVisible++)
	{
		cGrid = gridIndices[cVisible];

		pBounds = pGridBounds[cGrid];

		rpGridSectionAddress = CIndexedSet__GetROMAddressAndSize(&isGridSections, pThis->m_rpGridSectionsAddress,
																					indices[cGrid], &gridSectionSize);

		// make sure we use current cache entry
		pBounds->m_pceGridSection = NULL;

		CCartCache__RequestBlock(&pThis->m_Cache,
										 pThis, (pfnCACHENOTIFY) CScene__DecompressGridSection, NULL,
                        		 &pBounds->m_pceGridSection,
                        		 rpGridSectionAddress, gridSectionSize,
                        		 "Grid Section");

		if (pBounds->m_pceGridSection)
		{
			CIndexedSet__ConstructFromRawData(&isGridSection,
														 CCacheEntry__GetData(pBounds->m_pceGridSection),
														 FALSE);

			// there are zero blocks in an empty grid section
			if (CIndexedSet__GetBlockCount(&isGridSection))
			{
				pbStaticInstances = CIndexedSet__GetBlock(&isGridSection, 0);

				CUnindexedSet__ConstructFromRawData(&usStaticInstances, pbStaticInstances, FALSE);

				staticInstances = CUnindexedSet__GetBasePtr(&usStaticInstances);
				nStatInsts = CUnindexedSet__GetBlockCount(&usStaticInstances);

				// make list of visible instances
				nVisInsts = 0;
				for (cStatInst=0; cStatInst<nStatInsts; cStatInst++)
				{
					pInstance = &staticInstances[cStatInst];

					ASSERT(pInstance->ih.m_Type == I_STATIC);

					// bounds check against view rect
					if (CBoundsRect__IsOverlappingBounds(&view_bounds_rect, &pInstance->m_Bounds))
					{
						// bounds check against view volume
						if (CViewVolume__IsOverlappingBounds(&view_volume, &pInstance->m_Bounds) && CScene__IsActive(pThis, pInstance))
						{
							if (pInstance->m_dwFlags & STATIC_INSTANCE_TRANSPARENCY)
							{
								CScene__AddTransparentInstance(pThis, &pInstance->ih);
							}
							else
							{
								pSort = &sorts[nVisInsts++];

								//xCenter2 = pInstance->m_Bounds.m_vMin.x + pInstance->m_Bounds.m_vMax.x;
								//yCenter2 = pInstance->m_Bounds.m_vMin.y + pInstance->m_Bounds.m_vMax.y;
								//zCenter2 = pInstance->m_Bounds.m_vMin.z + pInstance->m_Bounds.m_vMax.z;
								xCenter2 = pInstance->m_Bounds.m_vMin.x + pInstance->m_Bounds.m_vMax.x + xPos2;
								yCenter2 = pInstance->m_Bounds.m_vMin.y + pInstance->m_Bounds.m_vMax.y + yPos2;
								zCenter2 = pInstance->m_Bounds.m_vMin.z + pInstance->m_Bounds.m_vMax.z + zPos2;

								pSort->p = pInstance;
								//pSort->dist = abs(xPos2 - xCenter2) + abs(yPos2 - yCenter2) + abs(zPos2 - zCenter2);
								pSort->dist = xCenter2*xCenter2 + yCenter2*yCenter2 + zCenter2*zCenter2;
							}
						}
					}
				}

#ifdef USE_ZBUFFER
				qsort(sorts, nVisInsts, sizeof(SortStruct), RevSortCompare, SortSwap);
#else
				qsort(sorts, nVisInsts, sizeof(SortStruct), SortCompare, SortSwap);
#endif

				// TEMP
				//nVisInsts = (frame_number/3)%(nVisInsts + 1);

				// draw
				for (cVisInst=0; cVisInst<nVisInsts; cVisInst++)
				{
					CGameStaticInstance__Draw((CGameStaticInstance*) sorts[cVisInst].p, ppDLP,
										  			  &pThis->m_Cache, pThis->m_pceTextureSetsIndex);
				}

				// draw simple instances
				pbSimpleInstances = CIndexedSet__GetBlock(&isGridSection, 1);

				CUnindexedSet__ConstructFromRawData(&usSimpleInstances, pbSimpleInstances, FALSE);

				simpleInstances = CUnindexedSet__GetBasePtr(&usSimpleInstances);
				nSimpleInsts = CUnindexedSet__GetBlockCount(&usSimpleInstances);

				for (cSimpleInst=0; cSimpleInst<nSimpleInsts; cSimpleInst++)
				{
					pSimple = &simpleInstances[cSimpleInst];

					ASSERT(pSimple->ah.ih.m_Type == I_SIMPLE);

					if (!(pSimple->m_wFlags & SIMPLE_FLAG_GONE) && CScene__IsActive(pThis, pSimple))
					{
						if (pSimple->m_wFlags & SIMPLE_FLAG_TRANSPARENCY)
						{
							CScene__AddTransparentInstance(pThis, &pSimple->ah.ih);
						}
						else
						{
							CGameSimpleInstance__Draw(pSimple, ppDLP,
															  &pThis->m_Cache, pThis->m_pceTextureSetsIndex);
						}
					}
				}

				CUnindexedSet__Destruct(&usStaticInstances);
				CUnindexedSet__Destruct(&usSimpleInstances);
				CIndexedSet__Destruct(&isGridSection);
			}
		}
	}

	CIndexedSet__Destruct(&isGridSections);
}

void CScene__DrawTransparentInstances(CScene *pThis, Gfx **ppDLP)
{
	CInstanceHdr			*pInstance;
	CGameStaticInstance	*pStatic;
	int						cInst, nInsts;//, ccInst;
	float						xCenter2, yCenter2, zCenter2,
								xPos2, yPos2, zPos2;
	SortStruct				sorts[MAX_SORT], *pSort;
	CEngineApp				*pApp;

	ASSERT(pThis->m_pceTextureSetsIndex);

	gSPClipRatio((*ppDLP)++, FRUSTRATIO_1);

	pApp = GetApp();

	xPos2 = -2*CEngineApp__GetEyePos(pApp).x;
	yPos2 = -2*CEngineApp__GetEyePos(pApp).y;
	zPos2 = -2*CEngineApp__GetEyePos(pApp).z;

	nInsts = pThis->m_nTransparentInstances;
	for (cInst=0; cInst<nInsts; cInst++)
	{
		pInstance = pThis->m_pTransparentInstances[cInst];
		pSort = &sorts[cInst];

		switch (pInstance->m_Type)
		{
			case I_STATIC:
				pStatic = (CGameStaticInstance*) pInstance;

				//xCenter2 = pStatic->m_Bounds.m_vMin.x + pStatic->m_Bounds.m_vMax.x;
				//yCenter2 = pStatic->m_Bounds.m_vMin.y + pStatic->m_Bounds.m_vMax.y;
				//zCenter2 = pStatic->m_Bounds.m_vMin.z + pStatic->m_Bounds.m_vMax.z;
				xCenter2 = pStatic->m_Bounds.m_vMin.x + pStatic->m_Bounds.m_vMax.x + xPos2;
				yCenter2 = pStatic->m_Bounds.m_vMin.y + pStatic->m_Bounds.m_vMax.y + yPos2;
				zCenter2 = pStatic->m_Bounds.m_vMin.z + pStatic->m_Bounds.m_vMax.z + zPos2;
				break;

			case I_SIMPLE:
			case I_ANIMATED:
				//xCenter2 = 2*pInstance->m_vPos.x;
				//yCenter2 = 2*pInstance->m_vPos.y;
				//zCenter2 = 2*pInstance->m_vPos.z;
				xCenter2 = 2*pInstance->m_vPos.x + xPos2;
				yCenter2 = 2*pInstance->m_vPos.y + yPos2;
				zCenter2 = 2*pInstance->m_vPos.z + zPos2;
				break;
		}

		pSort->p = pInstance;
		//pSort->dist = abs(xPos2 - xCenter2) + abs(yPos2 - yCenter2) + abs(zPos2 - zCenter2);
		pSort->dist = xCenter2*xCenter2 + yCenter2*yCenter2 + zCenter2*zCenter2;
	}

	qsort(sorts, nInsts, sizeof(SortStruct), SortCompare, SortSwap);

	// draw
	for (cInst=0; cInst<nInsts; cInst++)
	{
		pInstance = (CInstanceHdr*) sorts[cInst].p;

		switch (pInstance->m_Type)
		{
			case I_STATIC:
				CGameStaticInstance__Draw((CGameStaticInstance*) pInstance, ppDLP,
												  &pThis->m_Cache, pThis->m_pceTextureSetsIndex);
				break;

			case I_SIMPLE:
				CGameSimpleInstance__Draw((CGameSimpleInstance*) pInstance, ppDLP,
												  &pThis->m_Cache, pThis->m_pceTextureSetsIndex);
				break;

			case I_ANIMATED:
				CGameObjectInstance__Draw((CGameObjectInstance*) pInstance, ppDLP,
												  pThis->m_pceTextureSetsIndex);
				break;
		}
	}
}


//#define GROUND_LAYER
//#define SUNSET

//#define SKY_HEIGHT	28

#define SKY_LEVELS	2
#define SKY_DROP 		13
#define TRACK_HEIGHT_MULT	0.5
//#define SKY_XSPEED	(0.1*SCALING_FACTOR)
//#define SKY_ZSPEED	(0.2*SCALING_FACTOR)
#define SKY_XSPEED	(0.075*SCALING_FACTOR)
#define SKY_ZSPEED	(0.15*SCALING_FACTOR)
void CScene__DrawSky(CScene *pThis, Gfx **ppDLP)
{
#ifdef GROUND_LAYER
	static Vtx				fv[2][SKY_LEVELS+1][5][3];
#else
	static Vtx				fv[2][SKY_LEVELS][5][3];
#endif
	int						v, l,
								side;
	Vtx						*pVert;
	float						height,
								offsetX, offsetZ,
								divisor, farClip,
								minX, minZ, maxX, maxZ,
								t;
								//clipDist,
								//maxOpac;
	CVector3					sv[3], vv[3], vAB, vBC, vCross, vC[5], vA, vB;//, vDelta;
	CTextureInfo			textureInfo;
	static float			cFrame = 0;
	int						cAnimFrame;
	static float			cScrollX = 0;
	static float			cScrollZ = 0;
	BOOL						underWater, lookingUp;
	CROMRegionSet			*pRegionSet;
	float						waterOffset,
								skyDistance,
								opacValue;
	BYTE						nearColor[4], farColor[4];

	if (!pThis->m_pPlayer)
		return;

	if (!pThis->m_SkyTextureReady || !pThis->m_LevelInfoReceived)
		return;

	if (!CTextureLoader__RequestTextureSet(&pThis->m_SkyTextureLoader, &pThis->m_Cache, "Sky Texture Set"))
		return;

	CTextureLoader__GetTextureInfo(&pThis->m_SkyTextureLoader, &textureInfo);
	if (!textureInfo.m_nBitmaps)
		return;

	pRegionSet = CScene__GetRegionAttributes(pThis, pThis->m_pPlayer->ah.ih.m_pCurrentRegion);
	if (!pRegionSet)
		return;

	// is sky present for this region ?
	if (!(pRegionSet->m_dwFlags & REGFLAG_SKY))
		return;


	cFrame += frame_increment;
	if (cFrame >= textureInfo.m_nBitmaps)
		cFrame -= textureInfo.m_nBitmaps;
	cAnimFrame = cFrame/textureInfo.m_pFormat->m_PlaybackSpeed;

	farClip = 200*SCALING_FACTOR;

	//divisor = 2.0*GetApp()->m_FarClip*(SKY_LEVELS+1);
	divisor = 2.0*farClip*(SKY_LEVELS+1);
	cScrollX += (frame_increment*sky_speed_scaler*SKY_XSPEED)*GetApp()->m_SkySpeed/100.0;
	if (cScrollX > divisor)
		cScrollX -= divisor;

	cScrollZ += (frame_increment*sky_speed_scaler*SKY_ZSPEED)*GetApp()->m_SkySpeed/100.0;
	if (cScrollZ > divisor)
		cScrollZ -= divisor;

	CTextureLoader__LoadTexture(&pThis->m_SkyTextureLoader, ppDLP,
										 cAnimFrame % textureInfo.m_nBitmaps, 0,
										 //(1 << (8 + textureInfo.m_pFormat->m_WidthShift)),
										 //(1 << (8 + textureInfo.m_pFormat->m_HeightShift)),
										 (1 << textureInfo.m_pFormat->m_WidthShift) * 384,
										 (1 << textureInfo.m_pFormat->m_HeightShift) * 384,
										 G_TX_WRAP | G_TX_NOMIRROR, G_TX_WRAP | G_TX_NOMIRROR,
										 //G_TX_NOLOD);
										 15);

  	gSPMatrix((*ppDLP)++,
				 RSP_ADDRESS(&mtx_identity),
				 G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);

	// makes sky look better (due to alpha values interpolation)
	gSPClipRatio((*ppDLP)++, FRUSTRATIO_1);

#ifdef USE_FOG
	gSPClearGeometryMode((*ppDLP)++, G_FOG);
   gDPSetCycleType((*ppDLP)++, G_CYC_1CYCLE);
#endif

#ifdef SCREEN_SHOT

	CGeometry__SetRenderMode(ppDLP, G_RM_AA_ZB_XLU_INTER__G_RM_AA_ZB_XLU_INTER2);
#else
	CGeometry__SetRenderMode(ppDLP, G_RM_ZB_CLD_SURF__G_RM_ZB_CLD_SURF2);
#endif


	//gDPSetCombineMode((*ppDLP)++,
	//					  	G_CC_MODULATERGB, G_CC_MODULATERGB);


	switch (textureInfo.m_pFormat->m_Format)
	{
		case TEXTURE_FORMAT_CI4_IA:
			CGeometry__SetCombineMode(ppDLP, G_CC_ROB_PSEUDOCOLOR_SHADEALPHA__G_CC_ROB_PSEUDOCOLOR_SHADEALPHA);

			//gDPSetCombineMode((*ppDLP)++, G_CC_BLENDPEDECALA, G_CC_BLENDPEDECALA);
			//gDPSetCombineMode((*ppDLP)++, G_CC_BLENDIA, G_CC_BLENDIA);
			//gDPSetCombineMode((*ppDLP)++, G_CC_MODULATERGBA, G_CC_MODULATERGBA);

// /*
			gDPSetEnvColor((*ppDLP)++,
								 pThis->m_LevelInfo.m_WhiteColor[0],
								 pThis->m_LevelInfo.m_WhiteColor[1],
								 pThis->m_LevelInfo.m_WhiteColor[2],
								 255);
// */
/*
			gDPSetEnvColor((*ppDLP)++,
								 pThis->m_LevelInfo.m_BlackColor[0],
								 pThis->m_LevelInfo.m_BlackColor[1],
								 pThis->m_LevelInfo.m_BlackColor[2],
								 255);

 /*
			nearColor[0] = 255;
			nearColor[1] = 255;
			nearColor[2] = 255;

			farColor[0] = 255;
			farColor[1] = 255;
			farColor[2] = 255;
*/
// /*
			nearColor[0] = pThis->m_LevelInfo.m_BlackColor[0];
			nearColor[1] = pThis->m_LevelInfo.m_BlackColor[1];
			nearColor[2] = pThis->m_LevelInfo.m_BlackColor[2];

			farColor[0] = pThis->m_LevelInfo.m_BlackColor[0];
			farColor[1] = pThis->m_LevelInfo.m_BlackColor[1];
			farColor[2] = pThis->m_LevelInfo.m_BlackColor[2];
			//farColor[0] = 255;
			//farColor[1] = 0;
			//farColor[2] = 0;
// */
			break;

		case TEXTURE_FORMAT_CI4_RGBA:
		case TEXTURE_FORMAT_CI8_RGBA:
			CGeometry__SetCombineMode(ppDLP, G_CC_MODULATERGB__G_CC_MODULATERGB);

			nearColor[0] = 255;
			nearColor[1] = 255;
			nearColor[2] = 255;

			farColor[0] = 255;
			farColor[1] = 255;
			farColor[2] = 255;

			break;

		default:
			ASSERT(FALSE);
			break;
	}












  	gSPClearGeometryMode((*ppDLP)++, G_LIGHTING | G_CULL_BACK | G_TEXTURE_GEN);
  	gSPSetGeometryMode((*ppDLP)++, G_SHADE | G_SHADING_SMOOTH);

	underWater = FALSE;
	if (pRegionSet)
	{
		if (pRegionSet->m_dwFlags & REGFLAG_HASWATER)
		{
			waterOffset = min(2*SCALING_FACTOR, (pRegionSet->m_WaterElevation - (CEngineApp__GetEyePos(GetApp()).y))/2);

			underWater = (CEngineApp__GetEyePos(GetApp()).y < pRegionSet->m_WaterElevation);
		}
	}
	
	// Set sky height, (leave the same in cinema modes)
	if ((!CCamera__InCinemaMode(&GetApp()->m_Camera)) || (!pThis->m_SkyHeightSet))
	{
		pThis->m_SkyHeightSet = TRUE ;

		if (underWater)
			pThis->m_SkyHeight = pRegionSet->m_WaterElevation + ((SKY_LEVELS-1)*SKY_DROP*SCALING_FACTOR) - waterOffset;
		else
			pThis->m_SkyHeight = (TUROK_EYE_HEIGHT + GetApp()->m_SkyElevation)*SCALING_FACTOR + GetApp()->m_YPos*TRACK_HEIGHT_MULT;
	}


	//vC = GetApp()->m_vTCorners;

	lookingUp = (GetApp()->m_vTCorners[1].y + GetApp()->m_vTCorners[2].y + GetApp()->m_vTCorners[3].y + GetApp()->m_vTCorners[4].y)*0.25 > GetApp()->m_vTCorners[0].y;

	//CVector3__Subtract(&vDelta, &GetApp()->m_vTCorners[0], &GetApp()->m_vTCorners[1]);
	//clipDist = CVector3__Magnitude(&vDelta);
	
	for (l=0; l<SKY_LEVELS; l++)
	{
		// copy corners to vC
		for (v=0; v<5; v++)
			vC[v] = GetApp()->m_vTCorners[v];
		
		height = pThis->m_SkyHeight - (l*SKY_DROP*SCALING_FACTOR);

		// don't let sides go thorough far clip

		vA.x = vC[0].x;
		vA.y = height;
		vA.z = vC[0].z;

		for (v=1; v<5; v++)
		//for (v=1; v<2; v++)
		{
			vB.x = vC[v].x;
			vB.y = height;
			vB.z = vC[v].z;

			t = CViewVolume__Intersect(&view_volume, VOLUME_FAR_PLANE, &vA, &vB);
			if ((t >= 0) && (t < 1.01))
			{
				vC[v].x = BlendFLOAT(t, vC[0].x, vC[v].x);
				vC[v].y = height;
				vC[v].z = BlendFLOAT(t, vC[0].z, vC[v].z);
			}
/*			
			if (t > 1)
			{
				divisor = vC[v].y - vC[0].y;
				if (divisor == 0)
					t = 1;
				else
					t = (height - vC[0].y)/divisor;
			}

			if (t >= 0)
			{
				vC[v].x = BlendFLOAT(t, vC[0].x, vC[v].x);
				vC[v].z = BlendFLOAT(t, vC[0].z, vC[v].z);

				vC[v].y = height;
			}
*/
		}


		for (side=0; side<4; side++)
		{
			switch (side)
			{
				case 0:
					vv[0] = sv[0] = vC[0];
					vv[1] = sv[1] = vC[1];
					vv[2] = sv[2] = vC[3];
					break;

				case 1:
					vv[0] = sv[0] = vC[0];
					vv[1] = sv[1] = vC[3];
					vv[2] = sv[2] = vC[4];
					break;

				case 2:
					vv[0] = sv[0] = vC[0];
					vv[1] = sv[1] = vC[4];
					vv[2] = sv[2] = vC[2];
					break;

				case 3:
					vv[0] = sv[0] = vC[0];
					vv[1] = sv[1] = vC[2];
					vv[2] = sv[2] = vC[1];
					break;
			}

			for (v=0; v<3; v++)
			{
				// slow down sky movement
				sv[v].x -= 0.5*vv[0].x;
				sv[v].z -= 0.5*vv[0].z;

				sv[v].x += cScrollX;
				sv[v].z += cScrollZ;
			}

			CVector3__Subtract(&vAB, &vv[1], &vv[0]);
			CVector3__Subtract(&vBC, &vv[2], &vv[1]);
			CVector3__Cross(&vCross, &vAB, &vBC);

			if (lookingUp ^ (vCross.y <= 0))
			{
				maxX = max(sv[0].x, max(sv[1].x, sv[2].x));
				minX = min(sv[0].x, min(sv[1].x, sv[2].x));
				maxZ = max(sv[0].z, max(sv[1].z, sv[2].z));
				minZ = min(sv[0].z, min(sv[1].z, sv[2].z));

				divisor = 2.0*farClip*(l+1);
				if (maxX >= divisor)
					offsetX = ((int) (maxX/divisor)) * -divisor;
				else if (minX <= -divisor)
					offsetX = ((int) (minX/divisor)) * divisor;
				else
					offsetX = 0;

				if (maxZ >= divisor)
					offsetZ = ((int) (maxZ/divisor)) * -divisor;
				else if (minZ <= -divisor)
					offsetZ = ((int) (minZ/divisor)) * divisor;
				else
					offsetZ = 0;

				for (v=0; v<3; v++)
				{
					pVert = &fv[even_odd][l][side][v];

					pVert->v.ob[0] = (short) vv[v].x;
					pVert->v.ob[1] = (short) height;
					pVert->v.ob[2] = (short) vv[v].z;

					pVert->v.flag = 0;

					pVert->v.tc[0] = (short) ((sv[v].x + offsetX) * (16384.0/((l+1)*farClip)));
					pVert->v.tc[1] = (short) ((sv[v].z + offsetZ) * (16384.0/((l+1)*farClip)));

					if (v)
					{
						pVert->v.cn[0] = farColor[0];
						pVert->v.cn[1] = farColor[1];
						pVert->v.cn[2] = farColor[2];

						pVert->v.cn[3] = 0;

/*
						//pVert->v.cn[3] = maxOpac[v];

						
						CVector3__Subtract(&vDelta, &vv[v], &vv[0]);
						divisor = CVector3__Magnitude(&vDelta);
						//divisor = vv[v].y - vv[0].y;
						//if (divisor == 0)
						//	maxOpac = 0;
						//else
							//maxOpac = max(0, min(1, 1 - fabs(height - vv[0].y)/divisor));
							maxOpac = max(0, min(1, 1 - divisor/clipDist));


						if (l)
							pVert->v.cn[3] = (BYTE) (BOTTOM_OPAC*maxOpac);
						else
							pVert->v.cn[3] = (BYTE) (TOP_OPAC*maxOpac);
*/
					}
					else
					{
						pVert->v.cn[0] = nearColor[0];
						pVert->v.cn[1] = nearColor[1];
						pVert->v.cn[2] = nearColor[2];

#define BOTTOM_OPAC	127
#define TOP_OPAC		200

						if (l)
							opacValue = BOTTOM_OPAC;
						else
							opacValue = TOP_OPAC;

						skyDistance = fabs(vv[0].y - height);

#define SKY_FADE_DISTANCE			(6*SCALING_FACTOR)
#define SKY_FADE_GONE_FRACTION	0.25
						if (skyDistance < SKY_FADE_DISTANCE)
						{
							opacValue = max(0, skyDistance - (SKY_FADE_GONE_FRACTION*SKY_FADE_DISTANCE))
											* opacValue / ((1.0 - SKY_FADE_GONE_FRACTION)*SKY_FADE_DISTANCE);
						}

						pVert->v.cn[3] = (BYTE) opacValue;

					}
				}

   			gSPVertex((*ppDLP)++,
							 RSP_ADDRESS(fv[even_odd][l][side]),
							 3, 0);

				gSP1Triangle((*ppDLP)++, 0, 1, 2, 0);
			}
		}
	}
}

CROMRegionSet* CScene__GetRegionAttributes(CScene *pThis, CGameRegion *pRegion)
{
	CIndexedSet				isCollision;
	CUnindexedSet			usRegionAttributes;
	CROMRegionSet			*regionSets, *pRegionSet;
	void						*pbRegionAttributes;

	if (!cache_is_valid)
		return NULL;

	if (!pRegion)
		return NULL;

	if (!pThis->m_pceCollision)
		return NULL;

	CIndexedSet__ConstructFromRawData(&isCollision,
												 CCacheEntry__GetData(pThis->m_pceCollision),
												 FALSE);

	pbRegionAttributes = CIndexedSet__GetBlock(&isCollision, CART_COLLISION_usRegionAttributes);

	CUnindexedSet__ConstructFromRawData(&usRegionAttributes, pbRegionAttributes, FALSE);

	regionSets = (CROMRegionSet*) CUnindexedSet__GetBasePtr(&usRegionAttributes);
	ASSERT(pRegion->m_nRegionSet < CUnindexedSet__GetBlockCount(&usRegionAttributes));

	pRegionSet = &regionSets[pRegion->m_nRegionSet];

	CIndexedSet__Destruct(&isCollision);
	CUnindexedSet__Destruct(&usRegionAttributes);

	return pRegionSet;
}



/*****************************************************************************
*
*	Function:		CScene__GeneratePickup()
*
******************************************************************************
*
*	Description:	Creates pickup
*
*	Inputs:			*pThis	-	Ptr to scene
*						*pRegion	-	Ptr to region start position is in
*						vPos		-	Required start position
*						Type		-	Pickup id type (see "aistruct.h")
*
*	Outputs:			Ptr to pickup that was created, or NULL if it failed
*
*****************************************************************************/
CDynamicSimple *CScene__GeneratePickup(CScene *pThis, CGameRegion *pRegion, 
													CVector3 vPos, int nType, FLOAT Time)
{
	CVector3 vVel ;

	// Setup start velocity
	vVel.x = 0 ;
	vVel.y = 0 ;
	vVel.z = 0 ;

	return CSimplePool__CreateSimpleInstance(&pThis->m_SimplePool, nType,
														 vVel,
														 vPos, pRegion, Time) ;
}



/*****************************************************************************
*
*	Function:		CScene__NearestRegion()
*
******************************************************************************
*
*	Description:	Returns closest region to given position
*
*	Inputs:			*pThis	-	Ptr to scene
*						*pvPos	-	Ptr to position
*
*	Outputs:			Ptr to region closest to specified position, or NULL if none
*
*****************************************************************************/
CGameRegion* CScene__NearestRegion(CScene *pThis, CVector3 *pvPos)
{
	float				groundHeight,
						distance, closestDistance ;

	int				cRegion, nRegions ;

	CGameRegion		*pDesired,
						*regions, *pRegion ;
	void				*pbRegions ;
	CIndexedSet		isCollision ;
	CUnindexedSet	usRegions ;

	// Collision data should already be in RAM when this is called
	ASSERT(pThis->m_pceCollision) ;

	// Set up collision data for parsing
	CIndexedSet__ConstructFromRawData(&isCollision,
												 CCacheEntry__GetData(pThis->m_pceCollision),
												 FALSE) ;

	pbRegions = CIndexedSet__GetBlock(&isCollision, CART_COLLISION_usRegions) ;

	CUnindexedSet__ConstructFromRawData(&usRegions, pbRegions, FALSE) ;

	regions = (CGameRegion*) CUnindexedSet__GetBasePtr(&usRegions) ;
	nRegions = CUnindexedSet__GetBlockCount(&usRegions) ;

	// Search regions
	pDesired = NULL ;
	closestDistance = 0 ;
	for (cRegion=0 ; cRegion < nRegions ; cRegion++)
	{
		pRegion = &regions[cRegion] ;

		// Position in this region?
		if (CGameRegion__InRegion(pRegion, pvPos->x, pvPos->z))
		{
			// Get y distance
			groundHeight = CGameRegion__GetGroundHeight(pRegion, pvPos->x, pvPos->z) ;
			distance = abs(pvPos->y - groundHeight) ;
			if (pDesired)
			{
				// Is this the closest on the y axis?
				if (distance < closestDistance)
				{
					pDesired = pRegion ;
					closestDistance = distance ;
				}
			}
			else
			{
				pDesired = pRegion ;
				closestDistance = distance ;
			}
		}
	}

	CUnindexedSet__Destruct(&usRegions) ;
	CIndexedSet__Destruct(&isCollision) ;

	return pDesired ;
}

#if 0
int CScene__CheckSounds(CScene *pThis)
{
	CIndexedSet			isSoundEffects,
							isSounds;
	CUnindexedSet		usSound;
	void					*pbSounds,
							*pbSound;
	DWORD					cSound, nSounds,
							cElement, nElements;
	CROMSoundElement	*elements, *pElement;

	if (!cache_is_valid)
		return ;

	ASSERT(cache_is_valid);
	ASSERT(pThis->m_pceSoundEffects);

	CIndexedSet__ConstructFromRawData(&isSoundEffects,
												 CCacheEntry__GetDataPtr(pThis->m_pceSoundEffects),
												 FALSE);

	pbSounds = CIndexedSet__GetBlock(&isSoundEffects, CART_SOUNDEFFECTS_isSounds);
	CIndexedSet__ConstructFromRawData(&isSounds, pbSounds, FALSE);
	nSounds = CIndexedSet__GetBlockCount(&isSounds);

	for (cSound=0; cSound<nSounds; cSound++)
	{
		pbSound = CIndexedSet__GetBlock(&isSounds, cSound);
		CUnindexedSet__ConstructFromRawData(&usSound, pbSound, FALSE);

		nElements = CUnindexedSet__GetBlockCount(&usSound);
		elements = (CROMSoundElement*) CUnindexedSet__GetBasePtr(&usSound);

		for (cElement=0; cElement<nElements; cElement++)
		{
			pElement = &elements[cElement];

			// check it
			ASSERT(pElement->m_RadioVolume <= 2) ;
			ASSERT(pElement->m_RadioVolume >= 0) ;
		}

		CUnindexedSet__Destruct(&usSound);
	}

	CIndexedSet__Destruct(&isSounds);

	CIndexedSet__Destruct(&isSoundEffects);
}
#endif
