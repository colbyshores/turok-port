// romstruc.c
//

#include "stdafx.h"

#include "romstruc.h"
#include "cart.h"
#include "rommodel.h"
#include "scaling.h"
#include "cartdir.h"
#include "regtype.h"

#ifdef PLATFORM_PORT
/* A loaded host pointer is in the low (data/heap) range; NULL/small or N64 0x80xxxxxx-range
 * values mean the collision region corners aren't relocated/parsed yet (M5). */
#define TUROK_BADPTR(p) (((unsigned long)(p)) < 0x10000UL || ((unsigned long)(p)) >= 0x80000000UL)
#endif

#ifdef WIN32
#include "turokdoc.h"
#include "objinst.h"
#include "level.h"
#else
#include "tengine.h"
#include "ai.h"
#include "aistruc.h"
#include "aidoor.h"
#include "turret.h"
#include "tmove.h"
#include "tengine.h"
#include "mattable.h"
#include "geometry.h"
#include "boss.h"
#include "campaign.h"
#include "trex.h"
#include "textload.h"
#include "dlists.h"
#include "pickup.h"
#include "hedtrack.h"
#include "fx.h"
#include "particle.h"
#include "introcam.h"
#include "cammodes.h"
#include "cinecam.h"
#endif

#include "debug.h"

//#define RANDOM_AI
//#define BOUNDS_HULKS
//#define PLANT_FIX



#define USE_ORIGINAL_MATRIX		0


#ifdef WIN32
#define ANGLE_PI			3.14159265358979323846
#define ANGLE_DTOR(deg)	   (deg*(ANGLE_PI/180))
#define ANGLE_RTOD(radian) (radian*180.0/ANGLE_PI)
#endif


#define GR_TOTAL_TILT_FACTOR	0.75
#define GR_ZTILT_FACTOR			0.5
#define GR_MAX_XTILT				25
#define GR_MAX_ZTILT				25

#ifndef WIN32
#endif

// globals
CVector3	vt_player_corners[8];
CMtxF		mf_player_orient;

// SF
/////////////////////////////////////////////////////////////////////////////

SF FLOAT2SF(float fValue)
{
	return (WORD) ((*((DWORD*) (&fValue))) >> 16);
}

float SF2FLOAT(SF sfValue)
{
	DWORD dwValue = sfValue << 16;

	return *((float*) (&dwValue));
}

// CROMCorner
/////////////////////////////////////////////////////////////////////////////

#define COLLISION_PRECISION_BITS		20
#define COLLISION_PRECISION_SCALER	(1 << (32 - COLLISION_PRECISION_BITS))
#define COLLISION_MAX_VALUE			(1500.0*COLLISION_PRECISION_SCALER)

#ifdef WIN32
void CROMCorner__TakeFromCorner(CROMCorner *pThis, CCorner *pSource)
{
   CROMCornerEnc *pEnc = (CROMCornerEnc*) pThis;

	//pThis->m_vCorner = ORDERBYTES(pSource->m_vCorner * SCALING_FACTOR);
   //pThis->m_Ceiling = ORDERBYTES((float) (pSource->m_Ceiling * SCALING_FACTOR));

	pEnc->x = ORDERBYTES(EncodeFloatToINT32(&pSource->m_vCorner.x, COLLISION_MAX_VALUE));
	pEnc->y = ORDERBYTES(EncodeFloatToINT32(&pSource->m_vCorner.y, COLLISION_MAX_VALUE));
	pEnc->z = ORDERBYTES(EncodeFloatToINT32(&pSource->m_vCorner.z, COLLISION_MAX_VALUE));
	pEnc->c = ORDERBYTES(EncodeFloatToINT32(&pSource->m_Ceiling, COLLISION_MAX_VALUE));
}
#endif

void CROMCorner__Decode(CROMCorner *pThis)
{
	CROMCornerEnc *pEnc = (CROMCornerEnc*) pThis;

#define COLLISION_DEC_MAX_VALUE	(COLLISION_MAX_VALUE*SCALING_FACTOR)

#ifdef PLATFORM_PORT
	/* PORT: encoded corner coords are big-endian int32 in the cart blob. */
	pThis->m_vCorner.x	= INT322FLOAT(ORDERBYTES(pEnc->x), COLLISION_DEC_MAX_VALUE);
	pThis->m_vCorner.y	= INT322FLOAT(ORDERBYTES(pEnc->y), COLLISION_DEC_MAX_VALUE);
	pThis->m_vCorner.z	= INT322FLOAT(ORDERBYTES(pEnc->z), COLLISION_DEC_MAX_VALUE);
	pThis->m_Ceiling		= INT322FLOAT(ORDERBYTES(pEnc->c), COLLISION_DEC_MAX_VALUE);
#else
	pThis->m_vCorner.x	= INT322FLOAT(pEnc->x, COLLISION_DEC_MAX_VALUE);
	pThis->m_vCorner.y	= INT322FLOAT(pEnc->y, COLLISION_DEC_MAX_VALUE);
	pThis->m_vCorner.z	= INT322FLOAT(pEnc->z, COLLISION_DEC_MAX_VALUE);
	pThis->m_Ceiling		= INT322FLOAT(pEnc->c, COLLISION_DEC_MAX_VALUE);
#endif
}

// CROMRegion
/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
void CROMRegion__TakeFromRegion(CROMRegion *pThis, CRegion *pSource,
										  CMap<CRegion*, CRegion*, int, int> &RegionPtrToIndexMap,
										  CMap<CCorner*, CCorner*, int, int> &CornerPtrToIndexMap,
										  CObArray &Regions,
										  int nRegionSet,
										  CRegionSet *pRegionSet)
{
	pThis->m_nRegionSet = ORDERBYTES((WORD) nRegionSet);

	// note: losing hi word
	WORD wFlags = (WORD) pRegionSet->m_dwFlags;
	if (pSource->IsLedge())
		wFlags |= REGFLAG_CLIFF;

	pThis->m_wFlags = ORDERBYTES(wFlags);

	int i;
	for (i=0; i<3; i++)
	{
		int pos;
		BOOL found = CornerPtrToIndexMap.Lookup(pSource->m_pCorners[i], pos);
		ASSERT(found);
		//int pos = SearchObArray(Corners, pSource->m_pCorners[i]);
		//ASSERT(pos != -1);
		ASSERT(pos < USHRT_MAX);

		pThis->m_Corners[i] = ORDERBYTES((unsigned short) pos);

		if (pSource->m_pNeighbors[i])
		{
			BOOL found = RegionPtrToIndexMap.Lookup(pSource->m_pNeighbors[i], pos);
			ASSERT(found);
			//pos = SearchObArray(Regions, pSource->m_pNeighbors[i]);
			//ASSERT(pos != -1);
			ASSERT(pos < USHRT_MAX);
		}
		else
		{
			pos = -1;

			if (pRegionSet->m_dwFlags & REGFLAG_BRIDGE)
			{
				// find suitable fall guesses
				CCorner *pCornerA = pSource->m_pCorners[i];
				ASSERT_VALID(pCornerA);

				CCorner *pCornerB = pSource->m_pCorners[(i + 1)%3];
				ASSERT_VALID(pCornerB);

				CVector3 vCenter = (pCornerA->m_vCorner + pCornerB->m_vCorner)*0.5;

				float closestDistance;
				int cRegion, nRegions = Regions.GetSize();
				for (cRegion=0; cRegion<nRegions; cRegion++)
				{
					CRegion *pGuess = (CRegion*) Regions[cRegion];
					ASSERT_VALID(pGuess);

					if (pGuess != pSource)
					{
						if (pGuess->InRegion(vCenter))
						{
							float groundHeight = pGuess->GetGroundHeight(vCenter.x, vCenter.z);
							if (groundHeight < vCenter.y)
							{
								float distance = vCenter.y - groundHeight;
								if (pos == -1)
								{
									pos = cRegion;
									closestDistance = distance;
								}
								else
								{
									if (distance < closestDistance)
									{
										pos = cRegion;
										closestDistance = distance;
									}
								}
							}
						}
					}
				}
			}
		}

		pThis->m_Neighbors[i] = ORDERBYTES((unsigned short) pos);
	}
}
#endif

// CGameRegion implementation
/////////////////////////////////////////////////////////////////////////////

void				CGameRegion__DoOpenDoor(CGameRegion *pThis, WORD nRegionSet);
void				CGameRegion__DoCloseDoor(CGameRegion *pThis, WORD nRegionSet);
void				CGameRegion__DoMakeFall(CGameRegion *pThis, WORD nRegionSet);
void				CGameRegion__DoMakeWarp(CGameRegion *pThis, WORD nRegionSet);
//void				CGameRegion__DoMakeInstantDeath(CGameRegion *pThis, WORD nRegionSet);
void				CGameRegion__DoSetGroundHeight(CGameRegion *pThis, float Height, WORD nRegionSet);
CGameRegion*	CGameRegion__FindNextLowerRegion(CGameRegion *pThis, CVector3 vPos);

// CGameRegion
/////////////////////////////////////////////////////////////////////////////

void CGameRegion__TakeFromROMRegion(CGameRegion *pThis, CROMRegion *pSource,
												CROMCorner Corners[], CGameRegion Regions[])
{
	int 				i;
	unsigned short	neighbor;

#ifdef PLATFORM_PORT
	/* PORT: CROMRegion fields are big-endian in the cart blob (the WIN32 encode path
	 * CROMRegion__TakeFromRegion wraps each in ORDERBYTES; the decode here must undo it). Without
	 * this the corner indices below are byte-swapped → &Corners[wrong] → NULL/wild m_pCorners → the
	 * GetGroundNormal/Height crashes during instance placement, and the region geometry is garbage. */
	pThis->m_nRegionSet = ORDERBYTES(pSource->m_nRegionSet);
	pThis->m_wFlags = ORDERBYTES(pSource->m_wFlags);

	for (i=0; i<3; i++)
	{
		pThis->m_pCorners[i] = &Corners[ORDERBYTES(pSource->m_Corners[i])];

		neighbor = ORDERBYTES(pSource->m_Neighbors[i]);
		if (neighbor == ((unsigned short) -1))
			pThis->m_pNeighbors[i] = NULL;
		else
			pThis->m_pNeighbors[i] = &Regions[neighbor];
	}
#else
	pThis->m_nRegionSet = pSource->m_nRegionSet;
	pThis->m_wFlags = pSource->m_wFlags;

	//TRACE3("Neighbors 0:%d, 1:%d, 2:%d\r\n",
	//		pSource->m_Neighbors[0],
	//		pSource->m_Neighbors[1],
	//		pSource->m_Neighbors[2]);

	for (i=0; i<3; i++)
	{
		pThis->m_pCorners[i] = &Corners[pSource->m_Corners[i]];

		neighbor = pSource->m_Neighbors[i];
		if (neighbor == ((unsigned short) -1))
			pThis->m_pNeighbors[i] = NULL;
		else
			pThis->m_pNeighbors[i] = &Regions[neighbor];
	}
#endif

	//TRACE3("pNeighbors 0:%x, 1:%x, 2:%x\r\n",
	//		pThis->m_pNeighbors[0],
	//		pThis->m_pNeighbors[1],
	//		pThis->m_pNeighbors[2]);
}
#ifndef WIN32

void CGameRegion__OpenDoor(CGameRegion *pThis)
{
	if (pThis)
		if (pThis->m_wFlags & REGFLAG_DOOR)
			CGameRegion__DoOpenDoor(pThis, pThis->m_nRegionSet);
}

void CGameRegion__DoOpenDoor(CGameRegion *pThis, WORD nRegionSet)
{
	if ((pThis->m_nRegionSet == nRegionSet) && !(pThis->m_wFlags & REGFLAG_OPENDOOR))
	{
		pThis->m_wFlags |= REGFLAG_OPENDOOR;

		if (pThis->m_pNeighbors[0])
			CGameRegion__DoOpenDoor(pThis->m_pNeighbors[0], nRegionSet);
		if (pThis->m_pNeighbors[1])
			CGameRegion__DoOpenDoor(pThis->m_pNeighbors[1], nRegionSet);
		if (pThis->m_pNeighbors[2])
			CGameRegion__DoOpenDoor(pThis->m_pNeighbors[2], nRegionSet);
	}
}

void CGameRegion__CloseDoor(CGameRegion *pThis)
{
	if (pThis)
		if (pThis->m_wFlags & REGFLAG_DOOR)
			CGameRegion__DoCloseDoor(pThis, pThis->m_nRegionSet);
}

void CGameRegion__DoCloseDoor(CGameRegion *pThis, WORD nRegionSet)
{
	if ((pThis->m_nRegionSet == nRegionSet) && (pThis->m_wFlags & REGFLAG_OPENDOOR))
	{
		pThis->m_wFlags &= ~REGFLAG_OPENDOOR;

		if (pThis->m_pNeighbors[0])
			CGameRegion__DoCloseDoor(pThis->m_pNeighbors[0], nRegionSet);
		if (pThis->m_pNeighbors[1])
			CGameRegion__DoCloseDoor(pThis->m_pNeighbors[1], nRegionSet);
		if (pThis->m_pNeighbors[2])
			CGameRegion__DoCloseDoor(pThis->m_pNeighbors[2], nRegionSet);
	}
}

void CGameRegion__DoToWater(CGameRegion *pThis, WORD nRegionSet)
{
	if ((pThis->m_nRegionSet == nRegionSet) && ((pThis->m_wFlags & REGFLAG_HASWATER)==0))
	{
		pThis->m_wFlags |= REGFLAG_HASWATER;

		if (pThis->m_pNeighbors[0])
			CGameRegion__DoToWater(pThis->m_pNeighbors[0], nRegionSet);
		if (pThis->m_pNeighbors[1])
			CGameRegion__DoToWater(pThis->m_pNeighbors[1], nRegionSet);
		if (pThis->m_pNeighbors[2])
			CGameRegion__DoToWater(pThis->m_pNeighbors[2], nRegionSet);
	}
}

// turn this all of this regions type to have water
void CGameRegion__ToWater(CGameRegion *pThis)
{
	CROMRegionSet			*pRegionSet;

	if (pThis)
	{
		if ((pThis->m_wFlags & REGFLAG_HASWATER) ==0)
			CGameRegion__DoToWater(pThis, pThis->m_nRegionSet);

		pRegionSet = CScene__GetRegionAttributes(&GetApp()->m_Scene, pThis);
		if (pRegionSet)
		{
			pRegionSet->m_dwFlags |= REGFLAG_HASWATER ;
			pRegionSet->m_AmbientSounds |= REG_AMBIENTWATER ;
			pRegionSet->m_MapColor = REG_MAPCOLOR_BLUE ;
		}
	}
}


void CGameRegion__DoFromWater(CGameRegion *pThis)
{
	if (pThis->m_wFlags & REGFLAG_HASWATER)
	{
		pThis->m_wFlags &= ~REGFLAG_HASWATER;

		if (pThis->m_pNeighbors[0])
			CGameRegion__DoFromWater(pThis->m_pNeighbors[0]);
		if (pThis->m_pNeighbors[1])
			CGameRegion__DoFromWater(pThis->m_pNeighbors[1]);
		if (pThis->m_pNeighbors[2])
			CGameRegion__DoFromWater(pThis->m_pNeighbors[2]);
	}
}

// make anything from this region on, if it has water, not have water
void CGameRegion__FromWater(CGameRegion *pThis)
{
	CROMRegionSet			*pRegionSet;

	if (pThis)
	{
		if (pThis->m_wFlags & REGFLAG_HASWATER)
			CGameRegion__DoFromWater(pThis);

		pRegionSet = CScene__GetRegionAttributes(&GetApp()->m_Scene, pThis);
		if (pRegionSet)
		{
			pRegionSet->m_GroundMat = REGMAT_GRASS ;
			pRegionSet->m_dwFlags &= ~REGFLAG_HASWATER ;
			pRegionSet->m_AmbientSounds &= ~REG_AMBIENTWATER ;
			pRegionSet->m_MapColor = REG_MAPCOLOR_WHITE ;
		}
	}
}



// changes the region set to be instant death
// note REGIONSET, not individual triangle...
void CGameRegion__MakeInstantDeath(CGameRegion *pThis)
{
	CROMRegionSet			*pRegionSet;

	pRegionSet = CScene__GetRegionAttributes(&GetApp()->m_Scene, pThis);

	if (pRegionSet)
		pRegionSet->m_dwFlags |= REGFLAG_INSTANTDEATH ;
}

// changes the region set to NOT be instant death
// note REGIONSET, not individual triangle...
void CGameRegion__UnMakeInstantDeath(CGameRegion *pThis)
{
	CROMRegionSet			*pRegionSet;

	pRegionSet = CScene__GetRegionAttributes(&GetApp()->m_Scene, pThis);

	if (pRegionSet)
	 	pRegionSet->m_dwFlags &= ~REGFLAG_INSTANTDEATH ;
}

void CGameRegion__MakeWarp(CGameRegion *pThis)
{
	CROMRegionSet			*pRegionSet;

	pRegionSet = CScene__GetRegionAttributes(&GetApp()->m_Scene, pThis);
	if (pRegionSet)
		pRegionSet->m_dwFlags |= REGFLAG_WARPENTRANCE ;
}

void CGameRegion__UnMakeWarp(CGameRegion *pThis)
{
	CROMRegionSet			*pRegionSet;

	pRegionSet = CScene__GetRegionAttributes(&GetApp()->m_Scene, pThis);
	if (pRegionSet)
		pRegionSet->m_dwFlags &= ~REGFLAG_WARPENTRANCE ;
}





void CGameRegion__MakeFallDeath(CGameRegion *pThis)
{
	CROMRegionSet			*pRegionSet;

	pRegionSet = CScene__GetRegionAttributes(&GetApp()->m_Scene, pThis);
	if (pRegionSet)
		pRegionSet->m_dwFlags |= REGFLAG_FALLDEATH ;

	if (pThis)
		CGameRegion__DoMakeFall(pThis, pThis->m_nRegionSet);
}

void CGameRegion__DoMakeFall(CGameRegion *pThis, WORD nRegionSet)
{
	if ((pThis->m_nRegionSet == nRegionSet) && !(pThis->m_wFlags & REGFLAG_FALLDEATH))
	{
		pThis->m_wFlags |= REGFLAG_FALLDEATH;

		if (pThis->m_pNeighbors[0])
			CGameRegion__DoMakeFall(pThis->m_pNeighbors[0], nRegionSet);
		if (pThis->m_pNeighbors[1])
			CGameRegion__DoMakeFall(pThis->m_pNeighbors[1], nRegionSet);
		if (pThis->m_pNeighbors[2])
			CGameRegion__DoMakeFall(pThis->m_pNeighbors[2], nRegionSet);
	}
}


void CGameRegion__SetGroundHeight(CGameRegion *pThis, float Height)
{
	if (pThis)
		CGameRegion__DoSetGroundHeight(pThis, Height, pThis->m_nRegionSet);
}

void CGameRegion__DoSetGroundHeight(CGameRegion *pThis, float Height, WORD nRegionSet)
{
	if ((pThis->m_nRegionSet == nRegionSet) &&
		((pThis->m_pCorners[0]->m_vCorner.y != Height) ||
		(pThis->m_pCorners[1]->m_vCorner.y != Height) ||
		(pThis->m_pCorners[2]->m_vCorner.y != Height)))
	{
		pThis->m_pCorners[0]->m_vCorner.y = Height;
		pThis->m_pCorners[1]->m_vCorner.y = Height;
		pThis->m_pCorners[2]->m_vCorner.y = Height;

		if (pThis->m_pNeighbors[0])
			CGameRegion__DoSetGroundHeight(pThis->m_pNeighbors[0], Height, nRegionSet);
		if (pThis->m_pNeighbors[1])
			CGameRegion__DoSetGroundHeight(pThis->m_pNeighbors[1], Height, nRegionSet);
		if (pThis->m_pNeighbors[2])
			CGameRegion__DoSetGroundHeight(pThis->m_pNeighbors[2], Height, nRegionSet);
	}
}

CVector3 CGameRegion__GetCeilingNormal(CGameRegion *pThis)
{
	static CGameRegion	*pNormalCache = NULL;
	static CVector3		vNormalCache;
	CVector3					vLeft, vRight;

	if (!pThis)
	{
		vNormalCache.x = 0;
		vNormalCache.y = -1;
		vNormalCache.z = 0;
	}
	else if (pThis != pNormalCache)
	{
		vLeft.x = pThis->m_pCorners[1]->m_vCorner.x - pThis->m_pCorners[0]->m_vCorner.x;
		vLeft.y = pThis->m_pCorners[1]->m_Ceiling   - pThis->m_pCorners[0]->m_Ceiling;
		vLeft.z = pThis->m_pCorners[1]->m_vCorner.z - pThis->m_pCorners[0]->m_vCorner.z;

		vRight.x = pThis->m_pCorners[2]->m_vCorner.x - pThis->m_pCorners[1]->m_vCorner.x;
		vRight.y = pThis->m_pCorners[2]->m_Ceiling   - pThis->m_pCorners[1]->m_Ceiling;
		vRight.z = pThis->m_pCorners[2]->m_vCorner.z - pThis->m_pCorners[1]->m_vCorner.z;

		CVector3__Cross(&vNormalCache, &vRight, &vLeft);
	}

	pNormalCache = pThis;

	return vNormalCache;
}

CVector3 CGameRegion__GetCeilingUnitNormal(CGameRegion *pThis)
{
	static CGameRegion	*pNormalCache = NULL;
	static CVector3		vNormalCache;

	if (!pThis)
	{
		vNormalCache.x = 0;
		vNormalCache.y = -1;
		vNormalCache.z = 0;
	}
	else if (pThis != pNormalCache)
	{
		vNormalCache = CGameRegion__GetCeilingNormal(pThis);
		CVector3__Normalize(&vNormalCache);
	}

	pNormalCache = pThis;

	return vNormalCache;
}

float CGameRegion__GetCeilingHeight(CGameRegion *pThis, float X, float Z)
{
	CVector3	vNormal,
				vPoint;

	if (!pThis)
		return 0;

	vNormal = CGameRegion__GetCeilingNormal(pThis);

   if (vNormal.y == 0)
   {
      // degenerate region
      // return average height
      return (pThis->m_pCorners[0]->m_Ceiling
				  + pThis->m_pCorners[1]->m_Ceiling
				  + pThis->m_pCorners[2]->m_Ceiling)/3;
   }
   else
   {
      // use corner 0 for point on plane
		vPoint.x = pThis->m_pCorners[0]->m_vCorner.x;
		vPoint.y = pThis->m_pCorners[0]->m_Ceiling;
		vPoint.z = pThis->m_pCorners[0]->m_vCorner.z;

      return ((vPoint.z - Z)*vNormal.z + (vPoint.x - X)*vNormal.x + vPoint.y*vNormal.y)/vNormal.y;
   }
}

CVector3 CGameRegion__GetGroundNormal(CGameRegion *pThis)
{
	static CGameRegion	*pNormalCache = NULL;
	static CVector3		vNormalCache;
	CVector3					vLeft, vRight;

	if (!pThis)
	{
		vNormalCache.x = 0;
		vNormalCache.y = 1;
		vNormalCache.z = 0;
	}
	else if (pThis != pNormalCache
#ifdef PLATFORM_PORT
		/* PORT: region corner pointers are not correctly relocated/parsed yet (M5 collision parse) —
		 * they can be NULL or WILD (N64 0x80xxxxxx-range). Guard against any non-host-range pointer
		 * and fall back to an up-normal, like the !pThis case above. (Restores the legal screen, which
		 * now reaches level-0 collision load thanks to the RNC fix.) */
		&& !TUROK_BADPTR(pThis->m_pCorners[0]) && !TUROK_BADPTR(pThis->m_pCorners[1]) && !TUROK_BADPTR(pThis->m_pCorners[2])
#endif
		)
	{
		//ASSERT((DWORD) pThis->m_pCorners[0] > 0x80000000);
		//ASSERT((DWORD) pThis->m_pCorners[1] > 0x80000000);
		//ASSERT((DWORD) pThis->m_pCorners[2] > 0x80000000);

		CVector3__Subtract(&vLeft, &pThis->m_pCorners[1]->m_vCorner, &pThis->m_pCorners[0]->m_vCorner);
		CVector3__Subtract(&vRight, &pThis->m_pCorners[2]->m_vCorner, &pThis->m_pCorners[1]->m_vCorner);

		CVector3__Cross(&vNormalCache, &vLeft, &vRight);
	}
#ifdef PLATFORM_PORT
	else if (pThis != pNormalCache)
	{
		vNormalCache.x = 0; vNormalCache.y = 1; vNormalCache.z = 0;
	}
#endif

	pNormalCache = pThis;

	return vNormalCache;
}

CVector3 CGameRegion__GetGroundUnitNormal(CGameRegion *pThis)
{
	static CGameRegion	*pNormalCache = NULL;
	static CVector3		vNormalCache;

	if (!pThis)
	{
		vNormalCache.x = 0;
		vNormalCache.y = 1;
		vNormalCache.z = 0;
	}
	else if (pThis != pNormalCache)
	{
		vNormalCache = CGameRegion__GetGroundNormal(pThis);
		CVector3__Normalize(&vNormalCache);
	}

	pNormalCache = pThis;

	return vNormalCache;
}

float CGameRegion__GetGroundHeight(CGameRegion *pThis, float X, float Z)
{
	CVector3	vNormal,
				vPoint;

	if (!pThis)
		return -1400.0*SCALING_FACTOR;

#ifdef PLATFORM_PORT
	if (TUROK_BADPTR(pThis->m_pCorners[0]) || TUROK_BADPTR(pThis->m_pCorners[1]) || TUROK_BADPTR(pThis->m_pCorners[2]))
		return -1400.0*SCALING_FACTOR;   /* region corners not relocated/parsed yet (M5) */
#endif

	vNormal = CGameRegion__GetGroundNormal(pThis);

   if (vNormal.y == 0)
   {
      // degenerate region
      // return average height
      return (pThis->m_pCorners[0]->m_vCorner.y
				  + pThis->m_pCorners[1]->m_vCorner.y
				  + pThis->m_pCorners[2]->m_vCorner.y)/3;
   }
   else
   {
      // use corner 0 for point on plane
		vPoint = pThis->m_pCorners[0]->m_vCorner;

      return ((vPoint.z - Z)*vNormal.z + (vPoint.x - X)*vNormal.x + vPoint.y*vNormal.y)/vNormal.y;
   }
}

CVector3 CGameRegion__GetSlideVector(CGameRegion *pThis, float Gravity)
{
	CVector3		vNormal, vUp, vSideways, vDownhill, vDown, vSlide;
	float			cosTheta;

	if (!pThis)
	{
		vSlide.x = vSlide.y = vSlide.z = 0;
		return vSlide;
	}

	vUp.x = 0;
	vUp.y = 1;
	vUp.z = 0;

	vNormal = CGameRegion__GetGroundNormal(pThis);
	CVector3__Cross(&vSideways, &vNormal, &vUp);
	CVector3__Cross(&vDownhill, &vNormal, &vSideways);
	CVector3__Normalize(&vDownhill);

	if (vDownhill.y > 0)
		CVector3__MultScaler(&vDownhill, &vDownhill, -1);

	vDown.x = 0;
	vDown.y = -1;
	vDown.z = 0;

	cosTheta = CVector3__Dot(&vDownhill, &vDown);

	CVector3__MultScaler(&vSlide, &vDownhill, cosTheta * -Gravity*SCALING_FACTOR);

	return vSlide;
}

float CGameRegion__GetUphillCosine(CGameRegion *pThis,
											  float StartX, float StartZ,
										 	  float EndX, float EndZ)
{
	float		currentGroundHeight, desiredGroundHeight,
				dx, dy, dz,
				groundMagSq, magSq;

	if (pThis)
	{
		currentGroundHeight = CGameRegion__GetGroundHeight(pThis, StartX, StartZ);
		desiredGroundHeight = CGameRegion__GetGroundHeight(pThis, EndX, EndZ);
		if (desiredGroundHeight > currentGroundHeight)
		{
			// going up hill
			dx = EndX - StartX;
			dy = desiredGroundHeight - currentGroundHeight;
			dz = EndZ - StartZ;
			groundMagSq = dx*dx + dz*dz;
			magSq = groundMagSq + dy*dy;

			if (magSq != 0)
				return sqrt(groundMagSq/magSq);
		}
	}

	return 1;
}

float CGameRegion__GetHillAngle(CGameRegion *pThis,
										  float StartX, float StartZ,
										  float EndX, float EndZ)
{
	float		currentGroundHeight, desiredGroundHeight,
				dx, dy, dz,
				groundMagSq, magSq,
				cosTheta, theta;

	if (!pThis)
		return 0;

	currentGroundHeight = CGameRegion__GetGroundHeight(pThis, StartX, StartZ);
	desiredGroundHeight = CGameRegion__GetGroundHeight(pThis, EndX, EndZ);

	// going up hill
	dx = EndX - StartX;
	dy = desiredGroundHeight - currentGroundHeight;
	dz = EndZ - StartZ;
	groundMagSq = dx*dx + dz*dz;
	magSq = groundMagSq + dy*dy;

	if (magSq != 0)
	{
		cosTheta = sqrt(groundMagSq/magSq);
		theta = acos(max(-1, min(1, cosTheta)));

		if (desiredGroundHeight > currentGroundHeight)
			return theta;
		else
			return -theta;
	}
	else
	{
		return 0;
	}
}

DWORD CGameRegion__GetGroundMaterial(CGameRegion *pThis)
{
	CROMRegionSet	*pRegionSet;

	pRegionSet = CScene__GetRegionAttributes(&GetApp()->m_Scene, pThis);
	if (!pRegionSet)
		return REGMAT_DEFAULT;

	return pRegionSet->m_GroundMat;
}

DWORD CGameRegion__GetWallMaterial(CGameRegion *pThis)
{
	CROMRegionSet	*pRegionSet;

	pRegionSet = CScene__GetRegionAttributes(&GetApp()->m_Scene, pThis);
	if (!pRegionSet)
		return REGMAT_DEFAULT;

	return pRegionSet->m_WallMat;
}

BOOL CGameRegion__GetGroundIntersection(CGameRegion *pThis, CVector3 *pvStart, CVector3 *pvEnd, float *Out)
{
	CVector3		vNormal, vDelta,
					vCorner, vCornerDelta;
	float			normalDotMovementDelta, normalDotCornerDelta;

	vNormal = CGameRegion__GetGroundNormal(pThis);
	CVector3__Subtract(&vDelta, pvEnd, pvStart);

	if (pThis)
		vCorner = pThis->m_pCorners[0]->m_vCorner;
	else
		vCorner.x = vCorner.y = vCorner.z = 0;

	//if (pvEnd->y >= pvStart->y)
	//	return FALSE;



	normalDotMovementDelta = CVector3__Dot(&vNormal, &vDelta);
	//if (normalDotMovementDelta == 0)
	if (normalDotMovementDelta >= 0)
	{
		*Out = 1;
		return FALSE;
	}

	CVector3__Subtract(&vCornerDelta, pvStart, &vCorner);
	normalDotCornerDelta = CVector3__Dot(&vNormal, &vCornerDelta);

	*Out = -normalDotCornerDelta/normalDotMovementDelta;
	return ((*Out > 0) && (*Out < 1));
}

BOOL CGameRegion__GetCeilingIntersection(CGameRegion *pThis, CVector3 *pvStart, CVector3 *pvEnd, float *Out)
{
	CVector3		vNormal, vDelta,
					vCorner, vCornerDelta;
	float			normalDotMovementDelta, normalDotCornerDelta;

	if (!pThis)
		return FALSE;

	vNormal = CGameRegion__GetCeilingNormal(pThis);
	CVector3__Subtract(&vDelta, pvEnd, pvStart);

	vCorner.x = pThis->m_pCorners[0]->m_vCorner.x;
	vCorner.y = pThis->m_pCorners[0]->m_Ceiling;
	vCorner.z = pThis->m_pCorners[0]->m_vCorner.z;

	//if (pvEnd->y <= pvStart->y)
	//	return FALSE;

	normalDotMovementDelta = CVector3__Dot(&vNormal, &vDelta);
	//if (normalDotMovementDelta == 0)
	if (normalDotMovementDelta >= 0)
	{
		*Out = 1;
		return FALSE;
	}

	CVector3__Subtract(&vCornerDelta, pvStart, &vCorner);
	normalDotCornerDelta = CVector3__Dot(&vNormal, &vCornerDelta);

	*Out = -normalDotCornerDelta/normalDotMovementDelta;
	return ((*Out > 0) && (*Out < 1));
}

BOOL CGameRegion__CalculateWaterIntersection(CGameRegion *pThis, CVector3 *pvStart, CVector3 *pvEnd, float *Out)
{
	static CGameRegion	*pRegionCache = NULL;
	static CROMRegionSet	*pRegionSetCache = NULL;
	float						divisor;

	if (!pThis)
		return FALSE;

	if (!(pThis->m_wFlags & REGFLAG_HASWATER))
		return FALSE;

	if (pThis != pRegionCache)
	{
		pRegionSetCache = CScene__GetRegionAttributes(&GetApp()->m_Scene, pThis);

		pRegionCache = pThis;
	}

	if (!pRegionSetCache)
		return FALSE;

	//if (!(pRegionSetCache->m_dwFlags & REGIONSET_HASWATER))
	//	return FALSE;

	//if (pvEnd->y >= pvStart->y)
	//	return FALSE;

	divisor = pvEnd->y - pvStart->y;
	if (divisor == 0)
	{
		return FALSE;
	}
	else
	{
		*Out = (pRegionSetCache->m_WaterElevation - pvStart->y)/divisor;

		return ((*Out > 0) && (*Out < 1));
	}
}

CQuatern CGameRegion__GetGroundRotation(CGameRegion *pThis)
{
	CVector3 	vNormal,
					vUp,
					vAxis;
	float			mag,
					cosTheta, theta;
	CQuatern		qGround;

	vNormal = CGameRegion__GetGroundUnitNormal(pThis);

	vUp.x = 0;
	vUp.y = 1;
	vUp.z = 0;

	CVector3__Cross(&vAxis, &vUp, &vNormal);
	mag = CVector3__Magnitude(&vAxis);
	if (mag == 0)
	{
		CQuatern__Identity(&qGround);
	}
	else
	{
		CVector3__MultScaler(&vAxis, &vAxis, 1/mag);

		cosTheta = CVector3__Dot(&vUp, &vNormal);

		theta = acos(cosTheta);
		CQuatern__BuildFromAxisAngle(&qGround, vAxis.x, vAxis.y, vAxis.z, theta);
	}

	return qGround;
}

CQuatern CGameRegion__GetCeilingRotation(CGameRegion *pThis)
{
	CVector3 	vNormal,
					vUp,
					vAxis;
	float			mag,
					cosTheta, theta;
	CQuatern		qGround;

	vNormal = CGameRegion__GetCeilingUnitNormal(pThis);

	vUp.x = 0;
	vUp.y = 1;
	vUp.z = 0;

	CVector3__Cross(&vAxis, &vUp, &vNormal);
	mag = CVector3__Magnitude(&vAxis);
	if (mag == 0)
	{
		CQuatern__Identity(&qGround);
	}
	else
	{
		CVector3__MultScaler(&vAxis, &vAxis, 1/mag);

		cosTheta = CVector3__Dot(&vUp, &vNormal);

		theta = acos(cosTheta);
		CQuatern__BuildFromAxisAngle(&qGround, vAxis.x, vAxis.y, vAxis.z, theta);
	}

	return qGround;
}

float CGameRegion__GetEdgeAngle(CGameRegion *pThis, int nEdge)
{
	CVector3		*pvA, *pvB;
	float			deltaX, deltaZ,
					mag, theta;

	ASSERT((nEdge >= 0) && (nEdge <= 2));

	if (!pThis)
		return 0;

	pvA = &pThis->m_pCorners[nEdge]->m_vCorner;
	pvB = &pThis->m_pCorners[(nEdge+1)%3]->m_vCorner;

	deltaX = pvB->x - pvA->x;
	deltaZ = pvB->z - pvA->z;
	mag = sqrt(deltaX*deltaX + deltaZ*deltaZ);
	if (mag == 0)
	{
		return 0;
	}
	else
	{
		theta = acos(deltaX/mag);

		return (deltaZ < 0) ? theta : -theta;
	}
}

float CGameRegion__GetGroundYAngle(CGameRegion *pThis)
{
	CVector3		vNormal;
	float			mag, theta;

	if (!CGameRegion__IsCliff(pThis))
		return 0;

	vNormal = CGameRegion__GetGroundNormal(pThis);

	mag = sqrt(vNormal.x*vNormal.x + vNormal.z*vNormal.z);
	if (mag == 0)
	{
		return 0;
	}
	else
	{
		theta = acos(vNormal.z/mag);

		return (vNormal.x > 0) ? theta : -theta;
	}
}

BOOL CGameRegion__IsCliff(CGameRegion *pThis)
{
	CVector3		vUp, vNormal;
	float			dot;

	if (!pThis)
		return FALSE;

	if (!(pThis->m_wFlags & REGFLAG_RUNTIMECLIFF))
		return (pThis->m_wFlags & REGFLAG_CLIFF);

	vUp.x = 0;
	vUp.y = 1;
	vUp.z = 0;

	vNormal = CGameRegion__GetGroundUnitNormal(pThis);

	dot = CVector3__Dot(&vNormal, &vUp);

	return (dot <= ANGLE_LEDGE_COSINE);
}

#endif

// CROMBounds
/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
void	CROMBounds__TakeFromCBounds(CROMBounds *pThis, CBounds *pSource)
{
	pThis->m_vMin = ORDERBYTES(pSource->GetMinimum() * SCALING_FACTOR);
	pThis->m_vMax = ORDERBYTES(pSource->GetMaximum() * SCALING_FACTOR);
}
#endif

void CROMBounds__Print(CROMBounds *pThis)
{
	TRACE3("CROMBounds: m_vMin.x:%f, m_vMin.y:%f, m_vMin.z:%f\r\n",
			 pThis->m_vMin.x,
			 pThis->m_vMin.y,
			 pThis->m_vMin.z);
	TRACE3("CROMBounds: m_vMax.x:%f, m_vMax.y:%f, m_vMax.z:%f\r\n",
			 pThis->m_vMax.x,
			 pThis->m_vMax.y,
			 pThis->m_vMax.z);
}

// CBoundsRect
/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
void CBoundsRect__TakeFromCBounds(CBoundsRect *pThis, CBounds *pSource)
{
	CVector3 vMin = pSource->GetMinimum();
	CVector3 vMax = pSource->GetMaximum();

	pThis->m_MinX = ORDERBYTES((float) (vMin.x * SCALING_FACTOR));
	pThis->m_MinZ = ORDERBYTES((float) (vMin.z * SCALING_FACTOR));
	pThis->m_MaxX = ORDERBYTES((float) (vMax.x * SCALING_FACTOR));
	pThis->m_MaxZ = ORDERBYTES((float) (vMax.z * SCALING_FACTOR));
}
#endif

void CBoundsRect__Print(CBoundsRect *pThis)
{
#ifdef WIN32
	TRACE3("CBoundRect: MinX:%.1f, MinZ:%.1f, MaxX:%.1f, MaxZ:",
			 pThis->m_MinX,
			 pThis->m_MinZ,
			 pThis->m_MaxX);
	TRACE1("%.1f\r\n",
			 pThis->m_MaxZ);
#else
	TRACE4("CBoundRect: MinX:%.1f, MinZ:%.1f, MaxX:%.1f, MaxZ:%.1f\r\n",
			 pThis->m_MinX,
			 pThis->m_MinZ,
			 pThis->m_MaxX,
			 pThis->m_MaxZ);
#endif
}

void CBoundsRect__EncloseVectors(CBoundsRect *pThis, int nPts, CVector3 V[])
{
	if (nPts)
	{
		pThis->m_MinX = pThis->m_MaxX = V[0].x;
		pThis->m_MinZ = pThis->m_MaxZ = V[0].z;

		CBoundsRect__AddEncloseVectors(pThis, nPts-1, &V[1]);
	}
	else
	{
		ASSERT(FALSE);
	}
}

void CBoundsRect__AddEncloseVectors(CBoundsRect *pThis, int nPts, CVector3 V[])
{
	int p;

	for (p=0; p<nPts; p++)
	{
		pThis->m_MinX = min(pThis->m_MinX, V[p].x);
		pThis->m_MinZ = min(pThis->m_MinZ, V[p].z);

		pThis->m_MaxX = max(pThis->m_MaxX, V[p].x);
		pThis->m_MaxZ = max(pThis->m_MaxZ, V[p].z);
	}
}

// CROMRegionBlock
/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
void CROMRegionBlock__TakeFromCBounds(CROMRegionBlock *pThis, CBounds *pSource, int nRegions)
{
	CBoundsRect__TakeFromCBounds(&pThis->m_BoundsRect, pSource);

	pThis->m_nRegions = ORDERBYTES((DWORD) nRegions);
}
#endif

// CROMGridBounds
/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
void CROMGridBounds__TakeFromCBounds(CROMGridBounds *pThis, CBounds *pSource)
{
	CBoundsRect__TakeFromCBounds(&pThis->m_BoundsRect, pSource);
}
#endif

// CGameGridBounds
/////////////////////////////////////////////////////////////////////////////

void CGameGridBounds__TakeFromROMGridBounds(CGameGridBounds *pThis, CROMGridBounds *pSource)
{
	/* PORT: m_BoundsRect is 4 big-endian floats — swap component-wise (ORDERBYTES is identity off-host) */
	pThis->m_BoundsRect.m_MinX = ORDERBYTES(pSource->m_BoundsRect.m_MinX);
	pThis->m_BoundsRect.m_MinZ = ORDERBYTES(pSource->m_BoundsRect.m_MinZ);
	pThis->m_BoundsRect.m_MaxX = ORDERBYTES(pSource->m_BoundsRect.m_MaxX);
	pThis->m_BoundsRect.m_MaxZ = ORDERBYTES(pSource->m_BoundsRect.m_MaxZ);

	pThis->m_pceGridSection = NULL;
}

// CROMSimpleInstance
/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
void CROMSimpleInstance__TakeFromCObjectInstance(CROMSimpleInstance *pThis,
															    CObjectInstance *pSource,
															    int nObject, int nVariation,
															    CObArray *pRegions,
															    float Scaler,
																 int nID,
																 CTurokDoc *pDoc)
{
	ASSERT(nObject >= 0);
	ASSERT(nVariation >= 0);

	pThis->m_nObjType		= ORDERBYTES((WORD) nObject);
	pThis->m_nVariation	= ORDERBYTES((WORD) nVariation);

	CVector3 vPos = pSource->m_vPos;

	if (pSource->m_pVariation)
		if (!(pSource->m_pVariation->m_ea.m_dwTypeFlags & AI_TYPE_TRACKGROUND))
			ASSERT(!pSource->m_pCurrentRegion);

	if (pSource->m_pCurrentRegion)
		vPos.y += pSource->m_pCurrentRegion->GetGroundHeight(vPos.x, vPos.z);

   pThis->m_vPos		= ORDERBYTES(vPos * SCALING_FACTOR);

	CRegion *pRegion = pSource->FindClosestRegion(pRegions);
	if (pRegion)
		pThis->m_nCurrentRegion	= ORDERBYTES((WORD) SearchObArray(*pRegions, pRegion));
	else
		pThis->m_nCurrentRegion	= ORDERBYTES((WORD) -1);

	pThis->m_nID = ORDERBYTES((WORD) nID);

	//CBounds bounds = pSource->GetTransformedBounds(TRUE);
	//pThis->m_MaxRadius = ORDERBYTES((float) (bounds.GetMaxRadiusFromVector(pSource->m_vPos) * SCALING_FACTOR));


	CVector3 vPrevScale = pSource->m_vScale;
	pSource->m_vScale = CVector3(SCALING_SIMPLE_SCALE,SCALING_SIMPLE_SCALE,SCALING_SIMPLE_SCALE);
	pSource->CalculateOrientMatrix();

	CBounds bounds = pSource->GetTransformedBounds();

	pSource->m_vScale = vPrevScale;
	pSource->CalculateOrientMatrix();

	CVector3 vBasePos = CVector3(0,0,0) * pSource->m_mOrient;

	pThis->m_MaxRadius = ORDERBYTES((float) (bounds.GetMaxRadiusFromVector(vBasePos) * SCALING_FACTOR));


	WORD wFlags = 0;
	if (nObject == AI_OBJECT_WARP_DYNAMIC)
		wFlags |= SIMPLE_FLAG_WARP_DYNAMIC;

	if (pSource->m_pObjType->HasTransparency(pDoc))
		wFlags |= SIMPLE_FLAG_TRANSPARENCY;
	if (pSource->m_pObjType->HasEvents())
		wFlags |= SIMPLE_FLAG_TRANSPARENCY;

	pThis->m_wFlags = ORDERBYTES((WORD) wFlags);

	float rotY = pSource->m_qRot.GetRotationMatrix().ExtractYRotation();
	pThis->m_RotY = FLOAT2INT8(max(-pi, min(pi, rotY)), pi);

	pThis->m_bActiveFlags = (BYTE) pSource->m_dwActiveFlags;
}
#endif


// CGameSimpleInstance implementation
/////////////////////////////////////////////////////////////////////////////

#ifndef WIN32

#define	CGameSimpleInstance__GetOrientationMatrix(pThis) ((pThis)->m_mOrientation[even_odd])
#define	CGameSimpleInstance__GetShadowMatrix(pThis) ((pThis)->m_mShadow[even_odd])
void		CGameSimpleInstance__Advance(CGameSimpleInstance *pThis);
void		CGameSimpleInstance__AdvancePickup(CGameSimpleInstance *pThis);
void		CGameSimpleInstance__AdvanceWarp(CGameSimpleInstance *pThis);
void		CGameSimpleInstance__AdvanceMorph(CGameSimpleInstance *pThis);
void		CGameSimpleInstance__CalculateOrientationMatrix(CGameSimpleInstance *pThis);
void		CGameSimpleInstance__DrawShadow(CGameSimpleInstance *pThis, Gfx **ppDLP);

#endif

// CGameSimpleInstance
/////////////////////////////////////////////////////////////////////////////

#ifndef WIN32

#define WARP_CLOSE_TIME		0.87

void CGameSimpleInstance__TakeFromROMSimpleInstance(CGameSimpleInstance *pThis, CROMSimpleInstance *pSource,
																	 ROMAddress *rpObjectAddress,
																	 CGameRegion Regions[], CEnemyAttributes Variations[])
{
	int	nObjType;

	pThis->ah.ih.m_Type = I_SIMPLE;
	pThis->ah.ih.m_nObjType = ORDERBYTES(pSource->m_nObjType);	/* PORT: big-endian WORD */
	pThis->ah.ih.m_bActiveFlags = pSource->m_bActiveFlags;

	pThis->ah.ih.m_vPos.x = ORDERBYTES(pSource->m_vPos.x);		/* PORT: CVector3 = swap each float */
	pThis->ah.ih.m_vPos.y = ORDERBYTES(pSource->m_vPos.y);
	pThis->ah.ih.m_vPos.z = ORDERBYTES(pSource->m_vPos.z);
	pThis->ah.ih.m_CollisionHeightOffset = 0;

	pThis->m_RotAngle = INT82FLOAT(pSource->m_RotY, ANGLE_PI);

// NOTE: SCALES HIGHER THAN SCALING_SIMPLE_OBJECT_SCALE WILL NOT BE PROPERLY CLIPPED!!
	pThis->m_Scale = SCALING_SIMPLE_OBJECT_SCALE;
	pThis->m_OffsetY = 0;

	{
	float maxRadius = ORDERBYTES(pSource->m_MaxRadius);	/* PORT: big-endian float; m_vPos already swapped */
	pThis->m_Bounds.m_vMin.x = pThis->ah.ih.m_vPos.x - maxRadius;
	pThis->m_Bounds.m_vMin.y = pThis->ah.ih.m_vPos.y - maxRadius;
	pThis->m_Bounds.m_vMin.z = pThis->ah.ih.m_vPos.z - maxRadius;
	pThis->m_Bounds.m_vMax.x = pThis->ah.ih.m_vPos.x + maxRadius;
	pThis->m_Bounds.m_vMax.y = pThis->ah.ih.m_vPos.y + maxRadius;
	pThis->m_Bounds.m_vMax.z = pThis->ah.ih.m_vPos.z + maxRadius;
	}

	pThis->m_rpObjectIndexAddress = rpObjectAddress;

	if (ORDERBYTES(pSource->m_nVariation) == (WORD) -1)		/* PORT: big-endian WORD index */
		pThis->ah.ih.m_pEA = NULL;
	else
		pThis->ah.ih.m_pEA = &Variations[ORDERBYTES(pSource->m_nVariation)];

	if (ORDERBYTES(pSource->m_nCurrentRegion) == (WORD) -1)
		pThis->ah.ih.m_pCurrentRegion	= NULL;
	else
		pThis->ah.ih.m_pCurrentRegion	= &Regions[ORDERBYTES(pSource->m_nCurrentRegion)];

	pThis->m_wFlags = ORDERBYTES(pSource->m_wFlags);	/* PORT: big-endian WORD */


	nObjType = CInstanceHdr__TypeFlag(&pThis->ah.ih);

	if (AI_IsPickup(nObjType) || AI_IsMorph(nObjType))
	{
		pThis->m_wFlags |= SIMPLE_FLAG_VISIBLE;
		pThis->m_Time = 0 ;
	}
	else	// if (AI_IsWarp(nObjType))
	{
		// if returning from warp, or in a warp
		if (	(GetApp()->m_WarpID == -1)
			||	(pThis->ah.ih.m_pEA->m_wTypeFlags3 & AI_TYPE3_RETURNWARP))
		{
			pThis->m_wFlags |= SIMPLE_FLAG_VISIBLE;
			pThis->m_Time = WARP_CLOSE_TIME*pThis->ah.ih.m_pEA->m_Aggression;
		}
		// otherwise start invisible, nearly ready to appear
		else
		{
			pThis->m_Time = .70*pThis->ah.ih.m_pEA->m_AttackStyle ;
		}
	}

	//pThis->m_cFrame = 0;
	pThis->m_MorphInc = 0;

	pThis->m_nID = ORDERBYTES(pSource->m_nID);	/* PORT: big-endian WORD */
}

void CGameSimpleInstance__AdvancePickup(CGameSimpleInstance *pThis)
{
	CVector3		vPos ;

	if (pThis->m_wFlags & SIMPLE_FLAG_SCALEUP)
	{
#define SIMPLE_SCALEUP_TIME	0.75
		pThis->m_Time += frame_increment*(1.0/FRAME_FPS);
		if (pThis->m_Time < SIMPLE_SCALEUP_TIME)
		{
			pThis->m_Scale = pThis->m_Time*(SCALING_SIMPLE_OBJECT_SCALE/(float)SIMPLE_SCALEUP_TIME);
		}
		else
		{
			pThis->m_wFlags &= ~SIMPLE_FLAG_SCALEUP;
			pThis->m_Scale = SCALING_SIMPLE_OBJECT_SCALE;
		}
	}


	// add particle effect to certain pickups
	switch (CInstanceHdr__TypeFlag(&pThis->ah.ih))
	{
		// generate special particle
		case AI_OBJECT_PICKUP_HEALTH_100PLUS:
		case AI_OBJECT_PICKUP_MORTAL_WOUND:
		case AI_OBJECT_PICKUP_SPIRITUAL:
			AI_DoParticle(&pThis->ah.ih, PARTICLE_TYPE_PICKUPSPECIAL1, AI_GetPos(pThis));
			break;

		case AI_OBJECT_PICKUP_KEY2:
		case AI_OBJECT_PICKUP_KEY3:
		case AI_OBJECT_PICKUP_KEY4:
		case AI_OBJECT_PICKUP_KEY5:
		case AI_OBJECT_PICKUP_KEY6:
		case AI_OBJECT_PICKUP_KEY7:
		case AI_OBJECT_PICKUP_KEY8:
		case AI_OBJECT_PICKUP_KEY8MANTIS:
			vPos = pThis->ah.ih.m_vPos ;
			vPos.y += (4.25*SCALING_FACTOR) ;
			AI_DoParticle(&pThis->ah.ih, PARTICLE_TYPE_GENERIC230, vPos);
			break ;
	}
}

#define WARP_START_SPEED	27.5
#define WARP_IDLE_SPEED		8
#define WARP_END_SPEED		-20

#define WARP_START_TIME		0.25
#define WARP_IDLE_TIME		(0.75 - WARP_START_TIME)

#define WARP_FRAMES			10

void CGameSimpleInstance__AdvanceWarp(CGameSimpleInstance *pThis)
{
	float scaler,
			t, end,
			v;

	if (pThis->ah.ih.m_pEA->m_wTypeFlags3 & AI_TYPE3_RETURNWARP)
	{
		//pThis->m_cFrame += frame_increment*((1.0/FRAME_FPS)*WARP_IDLE_SPEED);
		pThis->m_MorphInc = frame_increment*((1.0/FRAME_FPS)*WARP_IDLE_SPEED);

		CScene__DoSoundEffect(&GetApp()->m_Scene, 572, 1, &pThis->ah.ih, &pThis->ah.ih.m_vPos, 0) ;
		return;
	}

	pThis->m_Time += frame_increment*(1.0/FRAME_FPS);

	if (pThis->m_wFlags & SIMPLE_FLAG_VISIBLE)
	{
		CScene__DoSoundEffect(&GetApp()->m_Scene, 572, 1, &pThis->ah.ih, &pThis->ah.ih.m_vPos, 0) ;

		// wait for m_Aggression seconds to disappear
		end = pThis->ah.ih.m_pEA->m_Aggression;
		if (pThis->m_Time >= end)
		{
			pThis->m_wFlags &= ~SIMPLE_FLAG_VISIBLE;
			//pThis->m_cFrame = 0;
			pThis->m_Time = 0;
		}
	}
	else
	{
		// wait for m_AttackStyle seconds to appear
		end = pThis->ah.ih.m_pEA->m_AttackStyle;
		if (pThis->m_Time >= end)
		{
			pThis->m_wFlags |= SIMPLE_FLAG_VISIBLE;
			pThis->m_wFlags &= ~SIMPLE_FLAG_CLOSEDWARP;
			pThis->m_Time = 0;
		}
	}

	if ((end == 0) || !(pThis->m_wFlags & SIMPLE_FLAG_VISIBLE))
	{
		scaler = 1;
	}
	else
	{
		t = pThis->m_Time/end;

		if (t < 0.25)
		{
			v = 4*t - 1;
			scaler = 1 - v*v;
		}
		else if (t > 0.75)
		{
			v = 4*t - 3;
			scaler = 1 - v*v;
		}
		else
		{
			scaler = 1;
		}

		if (t > WARP_CLOSE_TIME)
			pThis->m_wFlags |= SIMPLE_FLAG_CLOSEDWARP;

		if (t < WARP_START_TIME)
		{
			t = t * (1.0/WARP_START_TIME);
			t = BlendFLOAT(t, WARP_START_SPEED, WARP_IDLE_SPEED);
		}
		else if (t < (WARP_IDLE_TIME + WARP_START_TIME))
		{
			t = WARP_IDLE_SPEED;
		}
		else
		{
			t = (t - (WARP_IDLE_TIME + WARP_START_TIME))*(1.0/(1 - (WARP_IDLE_TIME + WARP_START_TIME)));
			t = BlendFLOAT(t, WARP_IDLE_SPEED, WARP_END_SPEED);
		}

		t *= WARP_FRAMES/10.0;

		//pThis->m_cFrame += frame_increment*(1.0/FRAME_FPS)*t;
		pThis->m_MorphInc = frame_increment*(1.0/FRAME_FPS)*t;
	}

	pThis->m_Scale = scaler*SCALING_SIMPLE_OBJECT_SCALE;
	pThis->m_OffsetY = (pThis->m_Bounds.m_vMax.y - pThis->ah.ih.m_vPos.y)*(1 - scaler)*(0.38);
}

#define MORPH_IDLE_SPEED		8
void CGameSimpleInstance__AdvanceMorph(CGameSimpleInstance *pThis)
{
	//pThis->m_cFrame += frame_increment*((1.0/FRAME_FPS)*MORPH_IDLE_SPEED);
	pThis->m_MorphInc = frame_increment*((1.0/FRAME_FPS)*MORPH_IDLE_SPEED);
}

void CGameSimpleInstance__Advance(CGameSimpleInstance *pThis)
{
	int	nObjType;

	// don't update stuff in pause mode...
	if (GetApp()->m_bPause)
		return ;


	nObjType = CInstanceHdr__TypeFlag(&pThis->ah.ih);

	if (AI_IsPickup(nObjType))
		CGameSimpleInstance__AdvancePickup(pThis);
	else if (AI_IsWarp(nObjType))
		CGameSimpleInstance__AdvanceWarp(pThis);
	else if (AI_IsMorph(nObjType))
		CGameSimpleInstance__AdvanceMorph(pThis);
}

void CGameSimpleInstance__CalculateOrientationMatrix(CGameSimpleInstance *pThis)
{
#if USE_ORIGINAL_MATRIX
	CMtxF		mfScale, mfRot, mfPos,
				mfTemp1, mfTemp2;
	int		nObjType;

	nObjType = CInstanceHdr__TypeFlag(&pThis->ah.ih);

	CMtxF__Scale(mfScale, pThis->m_Scale, pThis->m_Scale, pThis->m_Scale);

	if (AI_IsPickup(nObjType))
		CQuatern__ToMatrix(&GetApp()->m_Scene.m_qRotYPickup, mfRot);
	else
		CMtxF__RotateY(mfRot, pThis->m_RotAngle);
//		CMtxF__RotateY(mfRot, pThis->m_RotY);

	CVector3__Clamp(&pThis->ah.ih.m_vPos, WORLD_LIMIT);
	CMtxF__Translate(mfPos, pThis->ah.ih.m_vPos.x, pThis->ah.ih.m_vPos.y + pThis->m_OffsetY, pThis->ah.ih.m_vPos.z);

	CMtxF__PostMult(mfTemp1, mfScale, mfRot);
	CMtxF__PostMult(mfTemp2, mfTemp1, mfPos);

	CMtxF__ToMtx(mfTemp2, CGameSimpleInstance__GetOrientationMatrix(pThis));

#else
	// Using optimized matrix functions
	CMtxF		mfFinal ;
	int		nObjType;

	nObjType = CInstanceHdr__TypeFlag(&pThis->ah.ih);

	//CQuatern__ToMatrix(&pThis->m_qRot, mfFinal) ;
	if (AI_IsPickup(nObjType))
	{
		CQuatern__ToMatrix(&GetApp()->m_Scene.m_qRotYPickup, mfFinal);
	}
	else if (AI_IsBouncy(nObjType))
	{
		CMtxF__Rotate(mfFinal, pThis->m_RotAngle, pThis->vRotAxis.x, pThis->vRotAxis.y, pThis->vRotAxis.z);
		CMtxF__PreMultTranslate(mfFinal, 0, -pThis->m_OffsetY, 0) ;
	}
	else
	{
		CMtxF__RotateY(mfFinal, pThis->m_RotAngle);
//		CMtxF__RotateY(mfFinal, pThis->m_RotY);
	}

	CMtxF__PreMultScale(mfFinal, pThis->m_Scale, pThis->m_Scale, pThis->m_Scale) ;

	CVector3__Clamp(&pThis->ah.ih.m_vPos, WORLD_LIMIT);
	CMtxF__PostMultTranslate(mfFinal, pThis->ah.ih.m_vPos.x, pThis->ah.ih.m_vPos.y + pThis->m_OffsetY, pThis->ah.ih.m_vPos.z) ;

	CMtxF__ToMtx(mfFinal, CGameSimpleInstance__GetOrientationMatrix(pThis)) ;

#endif

}

void CGameSimpleInstance__Draw(CGameSimpleInstance *pThis, Gfx **ppDLP,
										 CCartCache *pCache, CCacheEntry *pceTextureSetsIndex)
{
	CIndexedSet		isObjectIndex,
						isGeometry,
						isNodes;
	ROMAddress		*rpGeometryAddress;
	CCacheEntry		*pceGeometry;
	DWORD				geometrySize;
	void				*pbNodes;
	int				nObjType;
	//static DWORD	morphFrame = (DWORD) -1;
	//static float	morphDist = 0;
	//float				dist;

	// Draw in "pickup key" cinema?
	if (    pThis->ah.ih.m_pEA && (pThis->ah.ih.m_pEA->m_wTypeFlags3 & AI_TYPE3_DONTDRAWONCINEMA) 
		  && (GetApp()->m_Camera.m_Mode == CAMERA_CINEMA_TUROK_PICKUP_KEY_MODE) )
		return ;

	if (CBoundsRect__IsOverlappingBounds(&anim_bounds_rect, &pThis->m_Bounds))
	{
		CGameSimpleInstance__Advance(pThis);

		if (    (CBoundsRect__IsOverlappingBounds(&view_bounds_rect, &pThis->m_Bounds))
			  && (pThis->m_wFlags & SIMPLE_FLAG_VISIBLE) )
		{
			if (CViewVolume__IsOverlappingBounds(&view_volume, &pThis->m_Bounds))
			{
				CGameSimpleInstance__CalculateOrientationMatrix(pThis);

				// make sure object index is current
				pThis->m_pceObjectIndex = NULL;

				// request object index
				CCartCache__RequestBlock(pCache,
												 NULL, NULL, NULL,
												 &pThis->m_pceObjectIndex,
												 pThis->m_rpObjectIndexAddress, IS_INDEX_SIZE(CART_GRAPHOBJ_nItems),
												 "Static Object Index");

				if (pThis->m_pceObjectIndex)
				{
					CIndexedSet__ConstructFromRawData(&isObjectIndex,
												 				 CCacheEntry__GetData(pThis->m_pceObjectIndex),
												 				 FALSE);

					// calculate address of geometry
   				rpGeometryAddress = CIndexedSet__GetROMAddressAndSize(&isObjectIndex,
																	  						pThis->m_rpObjectIndexAddress,
																	  						CART_GRAPHOBJ_isGeometry, &geometrySize);

					// request geometry data (will be decompressed)
					CGeometry__RequestBlock(&pThis->m_Geometry, pCache,
	  						 						rpGeometryAddress, geometrySize,
													pceTextureSetsIndex);

					pceGeometry = CGeometry__GetCacheEntry(&pThis->m_Geometry);
					if (pceGeometry)
					{
						CIndexedSet__ConstructFromRawData(&isGeometry,
																	 CCacheEntry__GetData(pceGeometry),
																	 FALSE);

						pbNodes = CIndexedSet__GetBlock(&isGeometry, CART_GEOMETRY_isNodes);
						CIndexedSet__ConstructFromRawData(&isNodes, pbNodes, FALSE);

						// draw static instance
						ASSERT(CIndexedSet__GetBlockCount(&isNodes) == 1); // static instances should have exactly 1 node

						// draw shadow ?
						if (pThis->ah.ih.m_pEA->m_dwTypeFlags & AI_TYPE_SHADOW)
							CGameSimpleInstance__DrawShadow(pThis, ppDLP);

						// instance orientation
  						gSPMatrix((*ppDLP)++,
									 RSP_ADDRESS(&CGameSimpleInstance__GetOrientationMatrix(pThis)),
	   							 G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);

						nObjType = CInstanceHdr__TypeFlag(&pThis->ah.ih);
						if (AI_IsWarp(nObjType) || AI_IsMorph(nObjType))
						{
/*
							dist = fabs(pThis->ah.ih.m_vPos.x - GetApp()->m_vTCorners[0].x) + fabs(pThis->ah.ih.m_vPos.z - GetApp()->m_vTCorners[0].z);

							// only morph for closest object
							// there is only one copy of the model per object type
							if ((frame_number != morphFrame) || (dist < morphDist))
							{
								morphFrame = frame_number;
								morphDist = dist;

								CGeometry__Morph(&pThis->m_Geometry, 0, pThis->m_cFrame);
							}
*/
							CGeometry__MorphInc(&pThis->m_Geometry, 0, pThis->m_MorphInc);

							CGeometry__DrawPart(&pThis->m_Geometry, ppDLP, pCache, 0, even_odd, FALSE, FALSE, pThis->ah.ih.m_pEA->m_nTexture);
						}
						else
						{
							CGeometry__DrawPart(&pThis->m_Geometry, ppDLP, pCache, 0, 0, TRUE, TRUE, pThis->ah.ih.m_pEA->m_nTexture);
						}

						CIndexedSet__Destruct(&isGeometry);
						CIndexedSet__Destruct(&isNodes);
					}

					CIndexedSet__Destruct(&isObjectIndex);
				}
			}
		}
	}
}

void CGameSimpleInstance__DrawShadow(CGameSimpleInstance *pThis, Gfx **ppDLP)
{
#if USE_ORIGINAL_MATRIX
	CMtxF			mfPos, mfScale,
					mfTemp1, mfTemp2;
#endif
	CMtxF			mfRotate ;
	CVector3		vTranslate;

	float			radius, groundHeight, height;
	CQuatern		qShadow;

	groundHeight = CGameRegion__GetGroundHeight(pThis->ah.ih.m_pCurrentRegion, pThis->ah.ih.m_vPos.x, pThis->ah.ih.m_vPos.z);
	height = pThis->ah.ih.m_vPos.y + pThis->ah.ih.m_CollisionHeightOffset - groundHeight;
	radius = max(0, min(pThis->ah.ih.m_pEA->m_CollisionRadius*0.5, pThis->ah.ih.m_pEA->m_CollisionRadius*0.5 - height*0.1));

	qShadow = CInstanceHdr__GetGroundRotation(&pThis->ah.ih);

	vTranslate.x = pThis->ah.ih.m_vPos.x;
	vTranslate.z = pThis->ah.ih.m_vPos.z;
	vTranslate.y = groundHeight;
	CVector3__Clamp(&vTranslate, WORLD_LIMIT);

#if USE_ORIGINAL_MATRIX
	CQuatern__ToMatrix(&qShadow, mfRotate);
	CMtxF__Scale(mfScale, radius, 1, radius);
	CMtxF__Translate(mfPos, vTranslate.x, vTranslate.y, vTranslate.z);
	CMtxF__PostMult(mfTemp1, mfScale, mfRotate);
	CMtxF__PostMult(mfTemp2, mfTemp1, mfPos);
	CMtxF__ToMtx(mfTemp2, CGameSimpleInstance__GetShadowMatrix(pThis));

#else
	// Optimized matrix version
	CQuatern__ToMatrix(&qShadow, mfRotate);
	CMtxF__PreMultScale(mfRotate, radius, 1, radius);
	CMtxF__PostMultTranslate(mfRotate, vTranslate.x, vTranslate.y, vTranslate.z);
	CMtxF__ToMtx(mfRotate, CGameSimpleInstance__GetShadowMatrix(pThis));
#endif

	CGeometry__DrawShadow(ppDLP, &CGameSimpleInstance__GetShadowMatrix(pThis), TRUE, 150);
}

#endif

// CROMEfficientStaticInstance
/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
BOOL CROMEfficientStaticInstance__TakeFromCObjectInstance(CROMEfficientStaticInstance *pThis,
																			 CObjectInstance *pSource,
																			 CTurokDoc *pDoc,
																			 int nObject, int nVariation,
																			 CObArray *pRegions,
																			 float Scaler,
																			 BOOL To3DSSpace,
																			 int nGeometryVersion)
{
	ASSERT(nObject >= 0);
	ASSERT(nVariation >= 0);

	if (nGeometryVersion == -1)
		return FALSE;

	pThis->m_bFlags = nGeometryVersion;
	ASSERT(!(pThis->m_bFlags & ~EFFICIENT_ROT_MASK));

	float gridDistance = pDoc->GetGridDistance();

	pThis->m_nObjType		= ORDERBYTES((WORD) nObject);
	pThis->m_nVariation	= ORDERBYTES((WORD) nVariation);


   if (		(pSource->m_vScale.x != pSource->m_vScale.y)
			|| (pSource->m_vScale.x != pSource->m_vScale.z) )
	{
		return FALSE;
	}
	pThis->m_Scale	= ORDERBYTES(pSource->m_vScale.x * (SCALING_FACTOR/Scaler));

	if (gridDistance == 0)
		return FALSE;
	CVector3 vPos = pSource->m_vPos;
	if (pSource->m_pCurrentRegion)
		if (pSource->m_pCurrentRegion->GetGroundHeight(vPos.x, vPos.z) != 0)
			return FALSE;

	float fx;
	if (vPos.x >= 0)
		fx = vPos.x + 0.005;
	else
		fx = vPos.x - 0.005;
	int ix = fx/gridDistance;
	if (fabs(ix*gridDistance - vPos.x) > 0.01)
		return FALSE;
	if (abs(ix) > 255)
		return FALSE;
	pThis->m_nPosX = (BYTE) abs(ix);
	if (ix < 0)
		pThis->m_bFlags |= EFFICIENT_FLAGS_POSX_NEG;

	float fz;
	if (vPos.z >= 0)
		fz = vPos.z + 0.005;
	else
		fz = vPos.z - 0.005;
	int iz = fz/gridDistance;
	if (fabs(iz*gridDistance - vPos.z) > 0.01)
		return FALSE;
	if (abs(iz) > 255)
		return FALSE;
	pThis->m_nPosZ = (BYTE) abs(iz);
	if (iz < 0)
		pThis->m_bFlags |= EFFICIENT_FLAGS_POSZ_NEG;

	float fy;
	if (vPos.y >= 0)
		fy = vPos.y + 0.005;
	else
		fy = vPos.y - 0.005;
	int iy = fy/EFFICIENT_Y_ACCURACY;
	if (fabs(iy*EFFICIENT_Y_ACCURACY - vPos.y) > 0.01)
		return FALSE;
	if (abs(iy) > 32767)
		return FALSE;
	pThis->m_nPosY = ORDERBYTES((short) iy);


	CBounds bounds = pSource->GetTransformedBounds(To3DSSpace);

	CVector3 vMin = bounds.GetMinimum() - vPos;

	fx = vMin.x - 0.9;
	ix = (int) fx;
	if (abs(ix) > 127)
		return FALSE;
	pThis->m_nMinX = ix;

	fy = vMin.y - 0.9;
	iy = (int) fy;
	if (abs(iy) > 127)
		return FALSE;
	pThis->m_nMinY = iy;

	fz = vMin.z - 0.9;
	iz = (int) fz;
	if (abs(iz) > 127)
		return FALSE;
	pThis->m_nMinZ = iz;

	CVector3 vMax = bounds.GetMaximum() - vPos;

	fx = vMax.x + 0.9;
	ix = (int) fx;
	if (abs(ix) > 127)
		return FALSE;
	pThis->m_nMaxX = ix;

	fy = vMax.y + 0.9;
	iy = (int) fy;
	if (abs(iy) > 127)
		return FALSE;
	pThis->m_nMaxY = iy;

	fz = vMax.z + 0.9;
	iz = (int) fz;
	if (abs(iz) > 127)
		return FALSE;
	pThis->m_nMaxZ = iz;


	CRegion *pRegion = pSource->FindClosestRegion(pRegions);
	if (pRegion)
		pThis->m_nCurrentRegion	= ORDERBYTES((WORD) SearchObArray(*pRegions, pRegion));
	else
		pThis->m_nCurrentRegion	= ORDERBYTES((WORD) -1);

	WORD wFlags = nGeometryVersion & STATIC_INSTANCE_GEOMVER_MASK;

	if (pSource->m_pObjType->HasTransparency(pDoc))
		wFlags |= STATIC_INSTANCE_TRANSPARENCY;
	if (pSource->m_pObjType->HasEvents())
		wFlags |= STATIC_INSTANCE_HASEVENTS;

	pThis->m_wFlags = ORDERBYTES(wFlags);

	pThis->m_bActiveFlags = (BYTE) pSource->m_dwActiveFlags;

	return TRUE;
}
#endif

// CROMStaticInstance
/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
void CROMStaticInstance__TakeFromCObjectInstance(CROMStaticInstance *pThis,
														 	    CObjectInstance *pSource,
																 CTurokDoc *pDoc,
																 int nObject, int nVariation,
																 CObArray *pRegions,
																 float Scaler,
																 BOOL To3DSSpace,
																 int nGeometryVersion)
{
	ASSERT(nObject >= 0);
	ASSERT(nVariation >= 0);

	pThis->m_nObjType		= ORDERBYTES((WORD) nObject);
	pThis->m_nVariation	= ORDERBYTES((WORD) nVariation);


	CVector3 vScale = pSource->m_vScale * (SCALING_FACTOR/Scaler);
	pThis->m_vScale = ORDERBYTES(vScale);


	CVector3 vPos = pSource->m_vPos;
	//if (pSource->m_pVariation)
	//{
		//if (pSource->m_pVariation->m_ea.m_dwTypeFlags & AI_TYPE_TRACKGROUND)
		//{

	if (pSource->m_pVariation)
		if (!(pSource->m_pVariation->m_ea.m_dwTypeFlags & AI_TYPE_TRACKGROUND))
			ASSERT(!pSource->m_pCurrentRegion);

	if (pSource->m_pCurrentRegion)
		vPos.y += pSource->m_pCurrentRegion->GetGroundHeight(vPos.x, vPos.z);
		//}
	//}
   vPos *= SCALING_FACTOR;
	pThis->m_vPos		= ORDERBYTES(vPos);

	CRegion *pRegion = pSource->FindClosestRegion(pRegions);
	if (pRegion)
		pThis->m_nCurrentRegion	= ORDERBYTES((WORD) SearchObArray(*pRegions, pRegion));
	else
		pThis->m_nCurrentRegion	= ORDERBYTES((WORD) -1);


	// use standard lighting if not standard geometry version
	if (nGeometryVersion == -1)
		nGeometryVersion = 0;

	BYTE bFlags = nGeometryVersion & STATIC_INSTANCE_GEOMVER_MASK;

	if (pSource->m_pObjType->HasTransparency(pDoc))
		bFlags |= STATIC_INSTANCE_TRANSPARENCY;
	if (pSource->m_pObjType->HasEvents())
		bFlags |= STATIC_INSTANCE_HASEVENTS;

	pThis->m_bFlags = bFlags;

	pThis->m_bActiveFlags = (BYTE) pSource->m_dwActiveFlags;


	float x, y, z, t;
	pSource->m_qRot.GetValues(&x, &y, &z, &t);

	x = x*127 + ((x < 0) ? -0.5 : 0.5);
	y = y*127 + ((y < 0) ? -0.5 : 0.5);
	z = z*127 + ((z < 0) ? -0.5 : 0.5);
	t = t*127 + ((t < 0) ? -0.5 : 0.5);

	pThis->m_RotX = (signed char) max(-127, min(127, (int) x));
	pThis->m_RotY = (signed char) max(-127, min(127, (int) y));
	pThis->m_RotZ = (signed char) max(-127, min(127, (int) z));
	pThis->m_RotT = (signed char) max(-127, min(127, (int) t));


	CBounds bounds = pSource->GetTransformedBounds(To3DSSpace);
	CVector3 vMin = (bounds.GetMinimum() * SCALING_FACTOR) - vPos;
	CVector3 vMax = (bounds.GetMaximum() * SCALING_FACTOR) - vPos;

	pThis->m_BoundsMinX = ORDERBYTES(FLOAT2SF(vMin.x));
	pThis->m_BoundsMinY = ORDERBYTES(FLOAT2SF(vMin.y));
	pThis->m_BoundsMinZ = ORDERBYTES(FLOAT2SF(vMin.z));

	pThis->m_BoundsMaxX = ORDERBYTES(FLOAT2SF(vMax.x));
	pThis->m_BoundsMaxY = ORDERBYTES(FLOAT2SF(vMax.y));
	pThis->m_BoundsMaxZ = ORDERBYTES(FLOAT2SF(vMax.z));
}
#endif

// CROMObjectInstance
/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
void CROMObjectInstance__TakeFromCObjectInstance(CROMObjectInstance *pThis,
																 CObjectInstance *pSource, CObArray *pGraphicObjects,
																 float MinScale,
																 CObArray *pRegions, int nVariation,
																 CTurokDoc *pDoc)
{
	BOOL device = pSource->m_pObjType->IsDevice();

	BYTE bFlags = 0;
	if (device)
		bFlags |= ANIM_INSTANCE_DEVICE;
	if (pSource->m_pObjType->HasTransparency(pDoc))
		bFlags |= ANIM_INSTANCE_TRANSPARENCY;

	pThis->m_bFlags = bFlags;

	pThis->m_bActiveFlags = pSource->m_dwActiveFlags;

   pThis->m_nObjType			= ORDERBYTES((WORD) SearchObArray(*pGraphicObjects, pSource->m_pObjType));
	pThis->m_nTypeFlag		= ORDERBYTES((WORD) pSource->m_pObjType->GetObjectTypeFlag());

	if (pSource->m_pCurrentRegion)
	{
		pThis->m_nCurrentRegion	= ORDERBYTES((WORD) SearchObArray(*pRegions, pSource->m_pCurrentRegion));

	   CVector3 vPos = pSource->m_vPos;
		vPos.y += pSource->m_pCurrentRegion->GetGroundHeight(vPos.x, vPos.z);
		pThis->m_vPos				= ORDERBYTES(vPos * SCALING_FACTOR);
	}
	else
	{
		if (device)
		{
			// place devices in regions even when track ground is off
			CRegion *pRegion = pSource->FindClosestRegion(pRegions);
			if (pRegion)
				pThis->m_nCurrentRegion	= ORDERBYTES((WORD) SearchObArray(*pRegions, pRegion));
			else
				pThis->m_nCurrentRegion	= ORDERBYTES((WORD) -1);
		}
		else
		{
			pThis->m_nCurrentRegion	= ORDERBYTES((WORD) -1);
		}

	   pThis->m_vPos				= ORDERBYTES(pSource->m_vPos * SCALING_FACTOR);
	}

	// check to see if wall collision is off for this instance
	if (pSource->m_pVariation->m_ea.m_dwTypeFlags & AI_TYPE_NOWALLCOLLISION)
		pThis->m_nCurrentRegion	= ORDERBYTES((WORD) -1);

	pThis->m_nVariation		= ORDERBYTES((WORD) nVariation);

	CVector3 vScale;
	if (pSource->m_pObjType->IsCharacterPlayer())
		vScale = CVector3(SCALING_PLAYERWEAPON_SCALE, SCALING_PLAYERWEAPON_SCALE, SCALING_PLAYERWEAPON_SCALE);
	else
		vScale = pSource->m_vScale;
	pThis->m_vScale			= ORDERBYTES(vScale * (pSource->m_pObjType->GetDetailScale()/MinScale));

	float rotY = pSource->m_qRot.GetRotationMatrix().ExtractYRotation();
	pThis->m_RotY = ORDERBYTES(FLOAT2INT16(max(-pi, min(pi, rotY)), pi));
}
#endif

// CGameObjectInstance implementation
/////////////////////////////////////////////////////////////////////////////

#ifndef WIN32

void 				CGameObjectInstance__Advance(CGameObjectInstance *pThis);
void 				CGameObjectInstance__AdvanceAccumulation(CGameObjectInstance *pThis);
CCollisionInfo2* CGameObjectInstance__GetCollisionInfo(CGameObjectInstance *pThis);
void				CGameObjectInstance__DrawShadow(CGameObjectInstance *pThis, Gfx **ppDLP, CVector3 *pvOffset);
void 				CGameObjectInstance__DoDraw(CGameObjectInstance *pThis, CAnimDraw *pAnimDraw, int nNode, CMtxF mfParent, BOOL OnlyChild);
int				CGameObjectInstance__GetModelIndex(CGameObjectInstance *pThis, int nType, int nNode);
int				CGameObjectInstance__GetHumanTextureIndex(CGameObjectInstance *pThis, int nNode);
void 				CGameObjectInstance__DoAI(CGameObjectInstance *pThis);
void				CGameObjectInstance__PauseAI(CGameObjectInstance *pThis);
void				CGameObjectInstance__DiscoAI(CGameObjectInstance *pThis);
CROMNodeIndex	CGameObjectInstance__GetNodeAnimIndex(CIndexedSet *pisAnim, int nNode);
CVector3			CGameObjectInstance__GetNodePos(CIndexedSet *pisAnim, int nNode, int nFrame);
CQuatern			CGameObjectInstance__GetNodeRot(CIndexedSet *pisAnim, int nNode, int nFrame);
CVector3			CGameObjectInstance__GetNodeBlendPos(CGameObjectInstance *pThis, CGameAnimateState *pasAnim, int nNode);
void				CGameObjectInstance__SendAnimEvents(CGameObjectInstance *pThis,
																	CIndexedSet *pisAnim,
																	CMtxF mfOrient);
void				CGameObjectInstance__SendAllDoorEvents(CGameObjectInstance *pThis, CIndexedSet *pisAnim);
void				CGameObjectInstance__RequestInitialAnimation(CGameObjectInstance *pThis);
//CQuatern			CGameObjectInstance__GetHeadRotation(CGameObjectInstance *pThis);
int				CGameObjectInstance__GetAnimType(CGameObjectInstance *pThis, int nAnimIndex);
BOOL				CGameObjectInstance__IsAbsoluteAnim(CGameObjectInstance *pThis, int nAnimIndex);
void				CGameObjectInstance__MoveToEndOfAbsoluteAnim(CGameObjectInstance *pThis, CGameAnimateState *pasAnim);

// callbacks
void 				CGameObjectInstance__ObjectIndexReceived(CGameObjectInstance *pThis, CCacheEntry **ppceTarget);
void 				CGameObjectInstance__ObjectInfoReceived(CGameObjectInstance *pThis, CCacheEntry **ppceTarget);
void 				CGameObjectInstance__AnimsHeaderReceived(CGameObjectInstance *pThis, CCacheEntry **ppceTarget);
void 				CGameObjectInstance__AnimsIndexReceived(CGameObjectInstance *pThis, CCacheEntry **ppceTarget);
void				CGameObjectInstance__DecompressAnim(CGameObjectInstance *pThis, CCacheEntry **ppceTarget);
void 				CGameObjectInstance__AnimReceived(CGameObjectInstance *pThis, CCacheEntry **ppceTarget);
void				CGameObjectInstance__PreDraw(CGameObjectInstance *pThis, Gfx **ppDLP);
void				CGameObjectInstance__PostDraw(CGameObjectInstance *pThis, Gfx **ppDLP);

#endif


// CGameStaticInstance
/////////////////////////////////////////////////////////////////////////////

#ifndef WIN32

void CGameStaticInstance__TakeFromROMEfficientStaticInstance(CGameStaticInstance *pThis, CROMEfficientStaticInstance *pSource,
																				 ROMAddress *rpObjectAddress,
																				 CGameRegion Regions[], CEnemyAttributes Variations[],
																				 float GridDistance)
{
#if USE_ORIGINAL_MATRIX
	CMtxF		mfPos, mfScale ;
	CMtxF		mfTemp1, mfTemp2;
#endif

	CMtxF		mfRotate ;
	CVector3	vPos;
	float		theta;

	pThis->ih.m_Type = I_STATIC;
	pThis->ih.m_nObjType = ORDERBYTES(pSource->m_nObjType);	/* PORT: big-endian WORD */
	pThis->ih.m_bActiveFlags = pSource->m_bActiveFlags;

	pThis->m_rpObjectIndexAddress = rpObjectAddress;

	if (ORDERBYTES(pSource->m_nVariation) == (WORD) -1)		/* PORT: big-endian WORD index */
		pThis->ih.m_pEA = NULL;
	else
		pThis->ih.m_pEA = &Variations[ORDERBYTES(pSource->m_nVariation)];

	vPos.x = pSource->m_nPosX*GridDistance;					/* m_nPosX/Z are BYTE — no swap */
	if (pSource->m_bFlags & EFFICIENT_FLAGS_POSX_NEG)
		vPos.x = -vPos.x;
	vPos.z = pSource->m_nPosZ*GridDistance;
	if (pSource->m_bFlags & EFFICIENT_FLAGS_POSZ_NEG)
		vPos.z = -vPos.z;
	vPos.y = ((short)ORDERBYTES(pSource->m_nPosY))*(EFFICIENT_Y_ACCURACY*SCALING_FACTOR);	/* PORT: big-endian short */

	//CVector3__Clamp(&vPos, WORLD_LIMIT);

	pThis->m_Bounds.m_vMin.x = vPos.x + pSource->m_nMinX*SCALING_FACTOR;
	pThis->m_Bounds.m_vMin.y = vPos.y + pSource->m_nMinY*SCALING_FACTOR;
	pThis->m_Bounds.m_vMin.z = vPos.z + pSource->m_nMinZ*SCALING_FACTOR;

	pThis->m_Bounds.m_vMax.x = vPos.x + pSource->m_nMaxX*SCALING_FACTOR;
	pThis->m_Bounds.m_vMax.y = vPos.y + pSource->m_nMaxY*SCALING_FACTOR;
	pThis->m_Bounds.m_vMax.z = vPos.z + pSource->m_nMaxZ*SCALING_FACTOR;

	switch (pSource->m_bFlags & EFFICIENT_ROT_MASK)
	{
		case EFFICIENT_ROT_0:	theta = ANGLE_DTOR(0);		break;
		case EFFICIENT_ROT_90:	theta = ANGLE_DTOR(90);		break;
		case EFFICIENT_ROT_180:	theta = ANGLE_DTOR(180);	break;
		case EFFICIENT_ROT_270:	theta = ANGLE_DTOR(270);	break;
		default:						ASSERT(FALSE);					break;
	}

#if USE_ORIGINAL_MATRIX
	// calculate orientation matrix
	CMtxF__Scale(mfScale, pSource->m_Scale, pSource->m_Scale, pSource->m_Scale);
	CMtxF__RotateY(mfRotate, theta) ;
	CMtxF__Translate(mfPos, vPos.x, vPos.y, vPos.z);

	CMtxF__PostMult(mfTemp1, mfScale, mfRotate);
	CMtxF__PostMult(mfTemp2, mfTemp1, mfPos);

	CMtxF__ToMtx(mfTemp2, pThis->m_mOrientation);

#else

#if 0
	// Optimized matrix version
	CMtxF__RotateY(mfRotate, theta) ;
	CMtxF__PreMultScale(mfRotate, pSource->m_Scale, pSource->m_Scale, pSource->m_Scale);
	CMtxF__PostMultTranslate(mfRotate, vPos.x, vPos.y, vPos.z);
	CMtxF__ToMtx(mfRotate, pThis->m_mOrientation);
#endif

	// Optimized matrix version 2 with fixed point translation
	{
	float scale = ORDERBYTES(pSource->m_Scale);	/* PORT: big-endian float */
	CMtxF__RotateY(mfRotate, theta) ;
	CMtxF__PreMultScale(mfRotate, scale, scale, scale) ;
	CMtxF__PostMultTranslate(mfRotate, vPos.x, vPos.y, vPos.z);
	CMtxF__ToMtx(mfRotate, pThis->m_mOrientation);
	//CMtxF__ToMtxPostMultTranslate(mfRotate, &(pThis->m_mOrientation), vPos.x, vPos.y, vPos.z);
	}

#endif


	pThis->ih.m_vPos = vPos;
	pThis->ih.m_CollisionHeightOffset = 0;

	if (ORDERBYTES(pSource->m_nCurrentRegion) == (WORD) -1)
		pThis->ih.m_pCurrentRegion	= NULL;
	else
		pThis->ih.m_pCurrentRegion	= &Regions[ORDERBYTES(pSource->m_nCurrentRegion)];

	pThis->m_dwFlags = ORDERBYTES(pSource->m_wFlags);	/* PORT: big-endian WORD */
}

/*
typedef struct CROMStaticInstance_t
{
	WORD				m_nObjType,
						m_nCurrentRegion,
						m_nVariation,
						m_wFlags;
   CVector3       m_vPos,
						m_vScale;
	SF					m_BoundsMinX, m_BoundsMinY, m_BoundsMinZ,
						m_BoundsMaxX, m_BoundsMaxY, m_BoundsMaxZ;
	signed char		m_RotX, m_RotY, m_RotZ, m_RotT;
} CROMStaticInstance;
*/
void CGameStaticInstance__TakeFromROMStaticInstance(CGameStaticInstance *pThis, CROMStaticInstance *pSource,
																	 ROMAddress *rpObjectAddress,
																	 CGameRegion Regions[], CEnemyAttributes Variations[])
{
#if USE_ORIGINAL_MATRIX
	CMtxF		mfPos, mfScale ;
	CMtxF		mfTemp1, mfTemp2;
#endif

	CQuatern	qRot;
	CMtxF		mfRotate ;
	CVector3	vPos, vScale;	/* PORT: byte-swapped copies (ORDERBYTES is identity off-host) */

	pThis->ih.m_Type = I_STATIC;
	pThis->ih.m_nObjType = ORDERBYTES(pSource->m_nObjType);	/* PORT: big-endian WORD */

	vPos.x = ORDERBYTES(pSource->m_vPos.x); vPos.y = ORDERBYTES(pSource->m_vPos.y); vPos.z = ORDERBYTES(pSource->m_vPos.z);
	vScale.x = ORDERBYTES(pSource->m_vScale.x); vScale.y = ORDERBYTES(pSource->m_vScale.y); vScale.z = ORDERBYTES(pSource->m_vScale.z);

	pThis->ih.m_bActiveFlags = pSource->m_bActiveFlags;

	/* PORT: m_BoundsMin/Max* are SF (=WORD, 16-bit) big-endian — swap BEFORE SF2FLOAT */
	pThis->m_Bounds.m_vMin.x = SF2FLOAT((SF)ORDERBYTES(pSource->m_BoundsMinX)) + vPos.x;
	pThis->m_Bounds.m_vMin.y = SF2FLOAT((SF)ORDERBYTES(pSource->m_BoundsMinY)) + vPos.y;
	pThis->m_Bounds.m_vMin.z = SF2FLOAT((SF)ORDERBYTES(pSource->m_BoundsMinZ)) + vPos.z;

	pThis->m_Bounds.m_vMax.x = SF2FLOAT((SF)ORDERBYTES(pSource->m_BoundsMaxX)) + vPos.x;
	pThis->m_Bounds.m_vMax.y = SF2FLOAT((SF)ORDERBYTES(pSource->m_BoundsMaxY)) + vPos.y;
	pThis->m_Bounds.m_vMax.z = SF2FLOAT((SF)ORDERBYTES(pSource->m_BoundsMaxZ)) + vPos.z;

	pThis->m_rpObjectIndexAddress = rpObjectAddress;

	if (ORDERBYTES(pSource->m_nVariation) == (WORD) -1)	/* PORT: big-endian WORD index */
		pThis->ih.m_pEA = NULL;
	else
		pThis->ih.m_pEA = &Variations[ORDERBYTES(pSource->m_nVariation)];

	qRot.x = pSource->m_RotX*(1.0/127.0);
	qRot.y = pSource->m_RotY*(1.0/127.0);
	qRot.z = pSource->m_RotZ*(1.0/127.0);
	qRot.t = pSource->m_RotT*(1.0/127.0);
	CQuatern__Normalize(&qRot);

#if USE_ORIGINAL_MATRIX

	// calculate orientation matrix
	CMtxF__Scale(mfScale, pSource->m_vScale.x, pSource->m_vScale.y, pSource->m_vScale.z);
	CQuatern__ToMatrix(&qRot, mfRotate);

	//CVector3__Clamp(&pSource->m_vPos, WORLD_LIMIT);
	CMtxF__Translate(mfPos, pSource->m_vPos.x, pSource->m_vPos.y, pSource->m_vPos.z);

	CMtxF__PostMult(mfTemp1, mfScale, mfRotate);
	CMtxF__PostMult(mfTemp2, mfTemp1, mfPos);

	CMtxF__ToMtx(mfTemp2, pThis->m_mOrientation);
#else

	// Optimized matrix version
	CQuatern__ToMatrix(&qRot, mfRotate);
	CMtxF__PreMultScale(mfRotate, vScale.x, vScale.y, vScale.z);

	//CVector3__Clamp(&pSource->m_vPos, WORLD_LIMIT);
	CMtxF__PostMultTranslate(mfRotate, vPos.x, vPos.y, vPos.z);

	CMtxF__ToMtx(mfRotate, pThis->m_mOrientation);
	//CMtxF__ToMtxPostMultTranslate(mfRotate, &(pThis->m_mOrientation), vPos.x, vPos.y, vPos.z);

#endif


	pThis->ih.m_vPos = vPos;
	pThis->ih.m_CollisionHeightOffset = 0;

	if (ORDERBYTES(pSource->m_nCurrentRegion) == (WORD) -1)
		pThis->ih.m_pCurrentRegion	= NULL;
	else
		pThis->ih.m_pCurrentRegion	= &Regions[ORDERBYTES(pSource->m_nCurrentRegion)];

	pThis->m_dwFlags = pSource->m_bFlags;
}

void CGameStaticInstance__Draw(CGameStaticInstance *pThis, Gfx **ppDLP,
										 CCartCache *pCache, CCacheEntry *pceTextureSetsIndex)
{
	CIndexedSet		isObjectIndex,
						isGeometry,
						isNodes;
	ROMAddress		*rpGeometryAddress;
	DWORD				geometrySize;
	void				*pbNodes;
	CCacheEntry		*pceGeometry;
// TEST
#ifdef BOUNDS_HULKS
	static Mtx     mBound[2][7];
	CMtxF				mfbScale, mfbTrans, mfbProd;
#endif

	// Draw in "pickup key" cinema?
	if (    pThis->ih.m_pEA && (pThis->ih.m_pEA->m_wTypeFlags3 & AI_TYPE3_DONTDRAWONCINEMA) 
		  && (GetApp()->m_Camera.m_Mode == CAMERA_CINEMA_TUROK_PICKUP_KEY_MODE) )
		return ;


	// make sure object index is current
	pThis->m_pceObjectIndex = NULL;

	// request object index
   CCartCache__RequestBlock(pCache,
                            NULL, NULL, NULL,
									 &pThis->m_pceObjectIndex,
                            pThis->m_rpObjectIndexAddress, IS_INDEX_SIZE(CART_GRAPHOBJ_nItems),
                            "Static Object Index");

	if (pThis->m_pceObjectIndex)
	{
		CIndexedSet__ConstructFromRawData(&isObjectIndex,
												 	 CCacheEntry__GetData(pThis->m_pceObjectIndex),
												 	 FALSE);

		// calculate address of geometry
   	rpGeometryAddress = CIndexedSet__GetROMAddressAndSize(&isObjectIndex,
																	  			pThis->m_rpObjectIndexAddress,
																	  			CART_GRAPHOBJ_isGeometry, &geometrySize);

		// pick proper geometry version for lighting
		rpGeometryAddress = (ROMAddress*) ((DWORD) rpGeometryAddress | CACHE_BLOCK_VERSION(pThis->m_dwFlags & STATIC_INSTANCE_GEOMVER_MASK));

		// request geometry data (will be decompressed)
		CGeometry__RequestBlock(&pThis->m_Geometry, pCache,
	  						 			rpGeometryAddress, geometrySize,
										pceTextureSetsIndex);

		pceGeometry = CGeometry__GetCacheEntry(&pThis->m_Geometry);
		if (pceGeometry)
		{
			CIndexedSet__ConstructFromRawData(&isGeometry,
														 CCacheEntry__GetData(pceGeometry),
														 FALSE);

			pbNodes = CIndexedSet__GetBlock(&isGeometry, CART_GEOMETRY_isNodes);
			CIndexedSet__ConstructFromRawData(&isNodes, pbNodes, FALSE);

			// draw static instance
			ASSERT(CIndexedSet__GetBlockCount(&isNodes) == 1); // static instances should have exactly 1 node


#ifdef BOUNDS_HULKS
// TEST
//////////////////////
///*
			CMtxF__Scale(mfbScale, .0003, .0003, .0003);
			CMtxF__Translate(mfbTrans, pThis->m_BoundsRect.m_MinX, 0, pThis->m_BoundsRect.m_MinZ);
			CMtxF__PostMult(mfbProd, mfbScale, mfbTrans);
			CMtxF__ToMtx(mfbProd, mBound[even_odd][0]);

  			gSPMatrix((*ppDLP)++,
						RSP_ADDRESS(&mBound[even_odd][0]),
	   				G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);

			CGeometry__DrawPart(&pThis->m_Geometry, ppDLP, pCache, 0, 0, TRUE, FALSE, pThis->ih.m_pEA->m_nTexture);

			gSPPopMatrix((*ppDLP)++, G_MTX_MODELVIEW);
//////////////////////
			CMtxF__Scale(mfbScale, .0003, .0003, .0003);
			CMtxF__Translate(mfbTrans, pThis->m_BoundsRect.m_MinX, 0, pThis->m_BoundsRect.m_MaxZ);
			CMtxF__PostMult(mfbProd, mfbScale, mfbTrans);
			CMtxF__ToMtx(mfbProd, mBound[even_odd][1]);

  			gSPMatrix((*ppDLP)++,
						RSP_ADDRESS(&mBound[even_odd][1]),
	   				G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);

			CGeometry__DrawPart(&pThis->m_Geometry, ppDLP, pCache, 0, 0, TRUE, FALSE, pThis->ih.m_pEA->m_nTexture);

			gSPPopMatrix((*ppDLP)++, G_MTX_MODELVIEW);
//////////////////////
			CMtxF__Scale(mfbScale, .0003, .0003, .0003);
			CMtxF__Translate(mfbTrans, pThis->m_BoundsRect.m_MaxX, 0, pThis->m_BoundsRect.m_MaxZ);
			CMtxF__PostMult(mfbProd, mfbScale, mfbTrans);
			CMtxF__ToMtx(mfbProd, mBound[even_odd][2]);

  			gSPMatrix((*ppDLP)++,
						RSP_ADDRESS(&mBound[even_odd][2]),
	   				G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);

			CGeometry__DrawPart(&pThis->m_Geometry, ppDLP, pCache, 0, 0, TRUE, FALSE, pThis->ih.m_pEA->m_nTexture);

			gSPPopMatrix((*ppDLP)++, G_MTX_MODELVIEW);
//////////////////////
			CMtxF__Scale(mfbScale, .0003, .0003, .0003);
			CMtxF__Translate(mfbTrans, pThis->m_BoundsRect.m_MaxX, 0, pThis->m_BoundsRect.m_MinZ);
			CMtxF__PostMult(mfbProd, mfbScale, mfbTrans);
			CMtxF__ToMtx(mfbProd, mBound[even_odd][3]);

  			gSPMatrix((*ppDLP)++,
						RSP_ADDRESS(&mBound[even_odd][3]),
	   				G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);

			CGeometry__DrawPart(&pThis->m_Geometry, ppDLP, pCache, 0, 0, TRUE, FALSE, pThis->ih.m_pEA->m_nTexture);

			gSPPopMatrix((*ppDLP)++, G_MTX_MODELVIEW);
//*/
//////////////////////
//////////////////////
			CMtxF__Scale(mfbScale, .0003, .0003, .0003);
			CMtxF__Translate(mfbTrans, view_tri_region.m_X[0], 0, view_tri_region.m_Z[0]);
			CMtxF__PostMult(mfbProd, mfbScale, mfbTrans);
			CMtxF__ToMtx(mfbProd, mBound[even_odd][4]);

  			gSPMatrix((*ppDLP)++,
						RSP_ADDRESS(&mBound[even_odd][4]),
	   				G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);

			CGeometry__DrawPart(&pThis->m_Geometry, ppDLP, pCache, 0, 0, TRUE, FALSE, pThis->ih.m_pEA->m_nTexture);

			gSPPopMatrix((*ppDLP)++, G_MTX_MODELVIEW);
//////////////////////
			CMtxF__Scale(mfbScale, .0003, .0003, .0003);
			CMtxF__Translate(mfbTrans, view_tri_region.m_X[1], 0, view_tri_region.m_Z[1]);
			CMtxF__PostMult(mfbProd, mfbScale, mfbTrans);
			CMtxF__ToMtx(mfbProd, mBound[even_odd][5]);

  			gSPMatrix((*ppDLP)++,
						RSP_ADDRESS(&mBound[even_odd][5]),
	   				G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);

			CGeometry__DrawPart(&pThis->m_Geometry, ppDLP, pCache, 0, 0, TRUE, FALSE, pThis->ih.m_pEA->m_nTexture);

			gSPPopMatrix((*ppDLP)++, G_MTX_MODELVIEW);
//////////////////////
			CMtxF__Scale(mfbScale, .0003, .0003, .0003);
			CMtxF__Translate(mfbTrans, view_tri_region.m_X[2], 0, view_tri_region.m_Z[2]);
			CMtxF__PostMult(mfbProd, mfbScale, mfbTrans);
			CMtxF__ToMtx(mfbProd, mBound[even_odd][6]);

  			gSPMatrix((*ppDLP)++,
						RSP_ADDRESS(&mBound[even_odd][6]),
	   				G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);

			CGeometry__DrawPart(&pThis->m_Geometry, ppDLP, pCache, 0, 0, TRUE, FALSE, pThis->ih.m_pEA->m_nTexture);

			gSPPopMatrix((*ppDLP)++, G_MTX_MODELVIEW);
//////////////////////
#endif

			// instance orientation
  			gSPMatrix((*ppDLP)++,
						 RSP_ADDRESS(&pThis->m_mOrientation),
	   				 G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);

			CGeometry__DrawPart(&pThis->m_Geometry, ppDLP, pCache, 0,
									  pThis->ih.m_pEA->m_nModel,
									  !((GetApp()->m_Mode == MODE_INTRO) || (GetApp()->m_Mode == MODE_TITLE)/* || (GetApp()->m_Mode == MODE_LEGALSCREEN)*/),
#ifdef SCREEN_SHOT
									  TRUE,
#else
									  (GetApp()->m_bPause) || (GetApp()->m_Mode == MODE_INTRO) || (GetApp()->m_Mode == MODE_TITLE) || (GetApp()->m_Mode == MODE_LEGALSCREEN),
#endif
									  pThis->ih.m_pEA->m_nTexture);

			CIndexedSet__Destruct(&isGeometry);
			CIndexedSet__Destruct(&isNodes);
		}

		CIndexedSet__Destruct(&isObjectIndex);
	}
}

#endif


#ifndef WIN32
void CGameStaticInstance__SendEvents(CGameStaticInstance *pThis, CCartCache *pCache, BOOL ConstantRateFrame)
{
	CIndexedSet			isObjectIndex;
	ROMAddress			*rpStaticEvents;
	DWORD					staticEventsSize;
	CUnindexedSet		usStaticEvents;
	int					cEvent, nEvents;
	CROMEventEntry		*pEvent, *events;
	BOOL					first;
	CMtxF					mfOrient;
	CVector3				vWorld;

	ASSERT(pThis->m_dwFlags & STATIC_INSTANCE_HASEVENTS);

	// make sure that entry is current
	pThis->m_pceObjectIndex = NULL;

	// request object index
	CCartCache__RequestBlock(pCache,
									 NULL, NULL, NULL,
									 &pThis->m_pceObjectIndex,
									 pThis->m_rpObjectIndexAddress, IS_INDEX_SIZE(CART_GRAPHOBJ_nItems),
									 "Static Object Index");

	if (pThis->m_pceObjectIndex)
	{
		CIndexedSet__ConstructFromRawData(&isObjectIndex,
												 	 CCacheEntry__GetData(pThis->m_pceObjectIndex),
												 	 FALSE);

		// calculate address of geometry
   	rpStaticEvents = CIndexedSet__GetROMAddressAndSize(&isObjectIndex,
																  			pThis->m_rpObjectIndexAddress,
																  			CART_GRAPHOBJ_usStaticEvents, &staticEventsSize);

		// make sure that entry is current
		pThis->m_pceStaticEvents = NULL;

		// request static events
		CCartCache__RequestBlock(pCache,
										 NULL, NULL, NULL,
										 &pThis->m_pceStaticEvents,
										 rpStaticEvents, staticEventsSize,
										 "Static Events");

		if (pThis->m_pceStaticEvents)
		{
			CUnindexedSet__ConstructFromRawData(&usStaticEvents,
												 			CCacheEntry__GetData(pThis->m_pceStaticEvents),
												 			FALSE);

			nEvents = CUnindexedSet__GetBlockCount(&usStaticEvents);
			ASSERT(nEvents);

			events = (CROMEventEntry*) CUnindexedSet__GetBasePtr(&usStaticEvents);

			first = TRUE;
			for (cEvent=0; cEvent<nEvents; cEvent++)
			{
				pEvent = &events[cEvent];

				if (ConstantRateFrame || (ORDERBYTES(pEvent->m_Value) == -1.0))	/* PORT: big-endian float */
				{
#ifdef PLATFORM_PORT
					CVector3 off;	/* m_vOffset passed by ptr — swap into a local, don't mutate the shared blob */
#endif
					if (first)
					{
						first = FALSE;
						CMtxF__FromMtx(mfOrient, pThis->m_mOrientation);
					}

					// multiply offset position by orientation matrix
					// to find position in world coordinates
#ifdef PLATFORM_PORT
					off.x = ORDERBYTES(pEvent->m_vOffset.x); off.y = ORDERBYTES(pEvent->m_vOffset.y); off.z = ORDERBYTES(pEvent->m_vOffset.z);
					CMtxF__VectorMult(mfOrient, &off, &vWorld);
					AI_Event_Dispatcher(&pThis->ih, &pThis->ih,
											  ORDERBYTES(pEvent->m_nEvent), AI_SPECIES_ALL,
											  vWorld, ORDERBYTES(pEvent->m_Value));
#else
					CMtxF__VectorMult(mfOrient, &pEvent->m_vOffset, &vWorld);

					AI_Event_Dispatcher(&pThis->ih, &pThis->ih,
											  pEvent->m_nEvent, AI_SPECIES_ALL,
											  vWorld, pEvent->m_Value);
#endif
				}
			}

			CUnindexedSet__Destruct(&usStaticEvents);
		}

		CIndexedSet__Destruct(&isObjectIndex);
	}
}
#endif

// CGameObjectInstance
/////////////////////////////////////////////////////////////////////////////

void CGameObjectInstance__TakeFromROMObjectInstance(CGameObjectInstance *pThis, CROMObjectInstance *pSource,
																	 CGameRegion Regions[], CEnemyAttributes Variations[])
{
	float groundHeight;
	//int ObjectType;

	//pThis->ah.m_vPrevBackoff.x = pThis->ah.m_vPrevBackoff.y = pThis->ah.m_vPrevBackoff.z = 0;

	pThis->ah.ih.m_Type		= I_ANIMATED;
	pThis->ah.ih.m_nObjType	= ORDERBYTES(pSource->m_nObjType);
	pThis->ah.ih.m_bActiveFlags = pSource->m_bActiveFlags;

	/* PORT: guard variation==-1 (devices like elevators/doors have no enemy variation), matching the
	 * CGameSimpleInstance (romstruc.c:1319) / CGameStaticInstance (:2251) decoders. Without this,
	 * ORDERBYTES(0xFFFF)=65535 indexes Variations[] far out of bounds -> garbage m_pEA -> garbage
	 * CalculateOrientationMatrix transform (uses m_pEA->m_CollisionHeight) -> the device is culled/clipped. */
	if (ORDERBYTES(pSource->m_nVariation) == (WORD) -1)
		pThis->ah.ih.m_pEA	= NULL;
	else
		pThis->ah.ih.m_pEA	= &Variations[ORDERBYTES(pSource->m_nVariation)];

	if (ORDERBYTES(pSource->m_nCurrentRegion) == (WORD) -1)
		pThis->ah.ih.m_pCurrentRegion	= NULL;
	else
		pThis->ah.ih.m_pCurrentRegion	= &Regions[ORDERBYTES(pSource->m_nCurrentRegion)];

#ifndef WIN32
	pThis->m_qGround = pThis->m_qShadow = CInstanceHdr__GetGroundRotation(&pThis->ah.ih);
#endif

	/* PORT: m_vPos/m_vScale are big-endian CVector3 floats. The aggregate ORDERBYTES is a NO-OP
	 * (it only swaps 2/4-byte scalars), so these were left big-endian — read on LE a big-endian
	 * float decodes to a denormal (~1e-41, prints as 0) or garbage, putting every animated object
	 * at the origin with a junk scale -> culled/clipped (invisible). Swap component-wise. */
#ifdef PLATFORM_PORT
	pThis->ah.ih.m_vPos.x = ORDERBYTES(pSource->m_vPos.x); pThis->ah.ih.m_vPos.y = ORDERBYTES(pSource->m_vPos.y); pThis->ah.ih.m_vPos.z = ORDERBYTES(pSource->m_vPos.z);
#else
   pThis->ah.ih.m_vPos		= ORDERBYTES(pSource->m_vPos);
#endif
	pThis->ah.ih.m_CollisionHeightOffset = 0;

	pThis->m_dwFlags = pSource->m_bFlags;

#ifdef PLATFORM_PORT
	pThis->m_vScale.x = ORDERBYTES(pSource->m_vScale.x); pThis->m_vScale.y = ORDERBYTES(pSource->m_vScale.y); pThis->m_vScale.z = ORDERBYTES(pSource->m_vScale.z);
#else
   pThis->m_vScale 			= ORDERBYTES(pSource->m_vScale);
#endif
   //pThis->m_pceObjectIndex = NULL;
	//pThis->m_pceAnimsHeader = pThis->m_pceAnimsIndex = NULL;
	//pThis->m_pCache = NULL;
	pThis->m_nAnims = 0;
	pThis->m_BlendStart = pThis->m_BlendFinish = 0;

	pThis->m_cMelt = 0;
	pThis->m_ShadowPercentage = 100.0;

	CQuatern__Identity(&pThis->m_qRot);
	pThis->m_RotY				= INT162FLOAT(ORDERBYTES(pSource->m_RotY), ANGLE_PI);

	pThis->ah.m_vVelocity.x = pThis->ah.m_vVelocity.y = pThis->ah.m_vVelocity.z = 0;
	CQuatern__Identity(&pThis->m_qGround);
	CQuatern__Identity(&pThis->m_qShadow);
	//pThis->m_ShadowIntensity = 1;

	CGameObjectInstance__SetDesiredAnim(pThis, 0);
	CGameObjectInstance__SetNotBlending(pThis);
	pThis->m_WaitingForAnim = -1;
	CGameAnimateState__Construct(&pThis->m_asCurrent);
	CGameAnimateState__Construct(&pThis->m_asBlend);

	pThis->m_pEventCGameObjectInstance = NULL;
	pThis->m_pAttackerCGameObjectInstance = NULL;

   pThis->m_pceObjectIndex = pThis->m_pceAnimsHeader = pThis->m_pceAnimsIndex = pThis->m_pceObjectInfo = NULL;


#ifndef WIN32
	CAIDynamic__Reset(&pThis->m_AI, pThis->ah.ih.m_pEA, pThis->ah.ih.m_vPos, pThis);

	// Clear draw vars
	pThis->pmtDrawMtxs = NULL ;

	// Clear special pointers
	pThis->m_pBoss = NULL;
	pThis->m_pCollisionFunction = NULL ;
	pThis->m_pCalculateOrientation = NULL ;

	// Initialise mode
	pThis->m_Mode = IDLE_MODE ;

	// Initialise head tracking
	pThis->m_HeadTurnAngle = 0 ;
	pThis->m_HeadTurnAngleVel = 0 ;
	pThis->m_HeadTiltAngle = 0 ;
	pThis->m_HeadTiltAngleVel = 0 ;
	pThis->m_HeadTrackBlend = 0 ;
	pThis->m_pHeadTrackInfo = NULL ;

	// Initialise swooshes
	pThis->m_ActiveSwooshes = FALSE ;

	pThis->m_nTypeFlag = ORDERBYTES(pSource->m_nTypeFlag);	/* PORT: big-endian WORD */
	//ObjectType = CScene__GetObjectTypeFlag(&GetApp()->m_Scene, pThis->ah.ih.m_nObjType) ;
	CAIDynamic__SetMeltTiming(&pThis->m_AI, pThis->ah.ih.m_pEA, pThis->m_nTypeFlag);

	// Special case setups
	switch (pThis->m_nTypeFlag)
	{
		case AI_OBJECT_CHARACTER_MANTIS_BOSS:
		case AI_OBJECT_CHARACTER_TREX_BOSS:
		case AI_OBJECT_CHARACTER_CAMPAIGNER_BOSS:
		case AI_OBJECT_CHARACTER_LONGHUNTER_BOSS:
		case AI_OBJECT_CHARACTER_HUMVEE_BOSS:

			// Call boss setup function
			BOSS_Initialise(pThis, pThis->m_nTypeFlag) ;
			break;

		case AI_OBJECT_CINEMA_TUROK_ARROW:
			CCamera__SetObjectToTrack(&GetApp()->m_Camera, pThis) ;

			if (GetApp()->m_WarpID == INTRO_ZOOM_TO_LOGO_WARP_ID)
				pThis->m_pCalculateOrientation = (void *)CGameObjectInstance__CinemaTurokArrowOrientation ;
			break ;

		// Add pickup generator to list
		case AI_OBJECT_DEVICE_PICKUP_GERATOR:
			CPickupMonitor__AddGenerators(&GetApp()->m_Scene.m_PickupMonitor, 1, &pThis->ah.ih.m_vPos) ;
			break ;
	}

	if (!CGameObjectInstance__IsDevice(pThis))
	{
		groundHeight = CInstanceHdr__GetGroundHeight(&pThis->ah.ih);
		if ((pThis->ah.ih.m_vPos.y - groundHeight) < 5*SCALING_FACTOR)
			pThis->ah.ih.m_vPos.y = groundHeight;
	}

#endif
}

#ifndef WIN32

int CGameObjectInstance__GetAnimType(CGameObjectInstance *pThis, int nAnimIndex)
{
	CIndexedSet		isObjectInfo;
	CUnindexedSet	usAnimTypes;
	void				*pbAnimTypes;
	DWORD				*animTypes;
	int				nType;

	if (!pThis->m_rpObjectInfo)
		return -1;

	pThis->m_pceObjectInfo = NULL;
	CCartCache__RequestBlock(GetCache(),
									 NULL, NULL, NULL,
									 &pThis->m_pceObjectInfo,
									 pThis->m_rpObjectInfo, pThis->m_ObjectInfoSize,
									 "Object Info (lookup anim)");

	if (!pThis->m_pceObjectInfo)
		return -1;

	CIndexedSet__ConstructFromRawData(&isObjectInfo,
												 CCacheEntry__GetData(pThis->m_pceObjectInfo),
												 FALSE);

	pbAnimTypes = CIndexedSet__GetBlock(&isObjectInfo, CART_OBJECTINFO_usAnimTypes);

	CUnindexedSet__ConstructFromRawData(&usAnimTypes, pbAnimTypes, FALSE);

	animTypes = (DWORD*) CUnindexedSet__GetBasePtr(&usAnimTypes);
	ASSERT(nAnimIndex < CUnindexedSet__GetBlockCount(&usAnimTypes));

	nType = animTypes[nAnimIndex];

	CIndexedSet__Destruct(&isObjectInfo);
	CUnindexedSet__Destruct(&usAnimTypes);

	return nType;
}

BOOL CGameObjectInstance__IsAbsoluteAnim(CGameObjectInstance *pThis, int nAnimIndex)
{
	int	nType;

	nType = CGameObjectInstance__GetAnimType(pThis, nAnimIndex);

	return ((nType >= AI_ABSOLUTE_ANIM_START) && (nType <= AI_ABSOLUTE_ANIM_END));
}

int CGameObjectInstance__LookupAIAnimType(CGameObjectInstance *pThis, int nType)
{
	CIndexedSet		isObjectInfo;
	CUnindexedSet	usAnimTypes;
	void				*pbAnimTypes;
	DWORD				*animTypes;
	int				nAnims, nIndex;
//						nFirst, nLast;
//	static int		nPlace = 0;

	if (!pThis->m_rpObjectInfo)
		return -1;

	pThis->m_pceObjectInfo = NULL;
	CCartCache__RequestBlock(GetCache(),
									 NULL, NULL, NULL,
									 &pThis->m_pceObjectInfo,
									 pThis->m_rpObjectInfo, pThis->m_ObjectInfoSize,
									 "Object Info (lookup anim)");

	if (!pThis->m_pceObjectInfo)
		return -1;

	CIndexedSet__ConstructFromRawData(&isObjectInfo,
												 CCacheEntry__GetData(pThis->m_pceObjectInfo),
												 FALSE);

	pbAnimTypes = CIndexedSet__GetBlock(&isObjectInfo, CART_OBJECTINFO_usAnimTypes);

	CUnindexedSet__ConstructFromRawData(&usAnimTypes, pbAnimTypes, FALSE);

	animTypes = (DWORD*) CUnindexedSet__GetBasePtr(&usAnimTypes);
	nAnims = CUnindexedSet__GetBlockCount(&usAnimTypes);

	nIndex = BinarySearch(animTypes, nAnims, nType);

//	if (BinaryRange(animTypes, nAnims, nType, &nFirst, &nLast))
//		nIndex = nFirst + (nPlace++)%(nLast+1 - nFirst);
//	else
//		nIndex = -1;

	CIndexedSet__Destruct(&isObjectInfo);
	CUnindexedSet__Destruct(&usAnimTypes);

	return nIndex;
}

int CGameObjectInstance__GetRandomAnimIndex(CGameObjectInstance *pThis, int nItems, int AITypes[], int Weights[], int *pPickedType)
{
#define MAX_RANDOM_TABLE_SIZE		64
	int	indices[MAX_RANDOM_TABLE_SIZE],
			weightStart[MAX_RANDOM_TABLE_SIZE],
			types[MAX_RANDOM_TABLE_SIZE],
			totalWeight,
			cItem,
			nIndex,
			cMatch, nMatches;
	DWORD randomNumber;


	if (!pThis->m_rpObjectInfo)
		return -1;

	ASSERT(nItems <= MAX_RANDOM_TABLE_SIZE);
	nItems = min(nItems, MAX_RANDOM_TABLE_SIZE);

	totalWeight = 0;
	nMatches = 0;
	for (cItem=0; cItem<nItems; cItem++)
	{
//		if (!		(		(pThis->ah.ih.m_pEA->m_wTypeFlags3 & AI_TYPE3_DONTDOVIOLENTDEATH)
//						&&	(		(AITypes[cItem] == AI_ANIM_DEATH_VIOLENT)
//								||	(AITypes[cItem] == AI_ANIM_DEATH_VIOLENTLESSCHANCE)
//								||	(AITypes[cItem] == AI_ANIM_DEATH_VIOLENT2LESSCHANCE)
//								||	(AITypes[cItem] == AI_ANIM_DEATH_VIOLENT2) ) ) )
//		{
			nIndex = CGameObjectInstance__LookupAIAnimType(pThis, AITypes[cItem]);
			if (nIndex != -1)
			{
				types[nMatches] = AITypes[cItem];
				indices[nMatches] = nIndex;
				weightStart[nMatches] = totalWeight;

				nMatches++;
				totalWeight += Weights[cItem];
			}
//		}
	}

	if (!totalWeight)
		return -1;

#ifdef WIN32
	randomNumber = rand() % totalWeight;
#else
	randomNumber = RANDOM(totalWeight);
#endif

	for (cMatch=nMatches-1; cMatch>=0; cMatch--)
	{
		if (weightStart[cMatch] <= randomNumber)
		{
			if (pPickedType)
				*pPickedType = types[cMatch];

			return indices[cMatch];
		}
	}

	// should never reach this
	ASSERT(FALSE);

	return -1;
}

int CGameObjectInstance__GetRandomAnimIndexWithCheck(CGameObjectInstance *pThis, int nItems, int Info[][3], int *pPickedType)
{
#define MAX_RANDOM_TABLE_SIZE		64
#define ANIMTYPE						0
#define WEIGHT							1
#define CHECK							2
	int	indices[MAX_RANDOM_TABLE_SIZE],
			weightStart[MAX_RANDOM_TABLE_SIZE],
			types[MAX_RANDOM_TABLE_SIZE],
			totalWeight,
			cItem,
			nIndex,
			cMatch, nMatches;
	DWORD randomNumber;


	if (!pThis->m_rpObjectInfo)
		return -1;

	ASSERT(nItems <= MAX_RANDOM_TABLE_SIZE);
	nItems = min(nItems, MAX_RANDOM_TABLE_SIZE);

	totalWeight = 0;
	nMatches = 0;
	for (cItem=0; cItem<nItems; cItem++)
	{
		nIndex = CGameObjectInstance__LookupAIAnimType(pThis, Info[cItem][ANIMTYPE]);
		if (nIndex != -1 && Info[cItem][CHECK])
		{
			types[nMatches] = Info[cItem][ANIMTYPE];
			indices[nMatches] = nIndex;
			weightStart[nMatches] = totalWeight;

			nMatches++;
			totalWeight += Info[cItem][WEIGHT];
		}
	}

	if (!totalWeight)
		return -1;

#ifdef WIN32
	randomNumber = rand() % totalWeight;
#else
	randomNumber = RANDOM(totalWeight);
#endif

	for (cMatch=nMatches-1; cMatch>=0; cMatch--)
	{
		if (weightStart[cMatch] <= randomNumber)
		{
			if (pPickedType)
				*pPickedType = Info[cMatch][ANIMTYPE];

			return indices[cMatch];
		}
	}

	// should never reach this
	ASSERT(FALSE);

	return -1;
}
#endif

// DON'T CALL BEFORE m_pceObjectInfo HAS BEEN RECEIVED
void CGameObjectInstance__CalculateOrientationMatrix(CGameObjectInstance *pThis, BOOL FollowView, CVector3 vTCorners[8], CMtxF mfOrient)
{
#ifdef WIN32
   CMatrix 	mPos, mScale;

   pThis->m_mOrientation = mScale.Scale(pThis->m_vScale) * mPos.Translate(pThis->ah.ih.m_vPos);
#else
	CIndexedSet	isObjectInfo;
	CROMBounds	*pBounds;

#if USE_ORIGINAL_MATRIX
	CMtxF			mfPos, mfScale, mfRotate ;
#endif

	CMtxF			mfBoundsOrient ;
	CMtxF			mfTemp1, mfTemp2;

//					mfAimRot, mfNPivot, mfPivot, mfView, mfAim;
	CQuatern    qRotY,
					qTemp1, qTemp2;
	CVector3		vWeaponOffset,
//					vRotateAxis,
					vCorners[8];

	//float			groundHeight;
	int			c;
	float			u, u2, ys, xzs;

	ys = xzs = 1;
	if (!FollowView)
	{
		if (AI_CanWeKillIt(pThis) && AI_DoesItMelt(pThis))
		{
			pThis->m_cMelt += frame_increment;

			u = pThis->m_cMelt/MELT_LENGTH;
			u2 = u*u;
			xzs = 1 + 0.5*u2;
			ys = 1 - u2;

			if (u >= 1)
				CGameObjectInstance__SetGone(pThis);
		}
	}

	if (FollowView)
	{
		// TEMP
		// move weapons into position
		CTMove__GetWeaponOffset(&CTurokMovement, &vWeaponOffset);
		vWeaponOffset.z -= 26.9*SCALING_FACTOR;

#if USE_ORIGINAL_MATRIX
		CMtxF__Scale(mfScale, xzs*pThis->m_vScale.x, ys*pThis->m_vScale.y, xzs*pThis->m_vScale.z);
		CMtxF__Translate(mfPos, vWeaponOffset.x, vWeaponOffset.y, vWeaponOffset.z);

		CMtxF__PostMult(mfBoundsOrient, mfScale, GetApp()->m_mfViewOrient);
		CMtxF__PostMult(mfTemp2, mf_3dstudio_to_u64, mfBoundsOrient);
		CMtxF__PostMult(mfOrient, mfPos, mfTemp2);
#else
 		// Using optimized matrix functions
		CMtxF__CopyPreMultScale(mfBoundsOrient, GetApp()->m_mfViewOrient, xzs*pThis->m_vScale.x, ys*pThis->m_vScale.y, xzs*pThis->m_vScale.z);
		CMtxF__CopyPreMult3DSToU64(mfOrient, mfBoundsOrient) ;
		CMtxF__PreMultTranslate(mfOrient, vWeaponOffset.x, vWeaponOffset.y, vWeaponOffset.z);

#endif

	}
	else
	{
		// Special case setup
		if (pThis->m_pCalculateOrientation)
			qTemp2 = pThis->m_pCalculateOrientation(pThis) ;
		else
		{
			CQuatern__BuildFromAxisAngle(&qRotY, 0,1,0, pThis->m_RotY);
			CQuatern__Mult(&qTemp1, &pThis->m_qRot, &qRotY);
			CQuatern__Mult(&qTemp2, &qTemp1, &pThis->m_qGround);
		}
#ifdef PLATFORM_PORT
		{ extern char *getenv(const char*); extern int fprintf(void*,const char*,...); extern void *stderr;
		  static int s_q=-1, s_seen[256], s_ns=0; if(s_q<0) s_q=getenv("TUROK_QLOG")?1:0;
		  if(s_q){ int _t=(int)CGameObjectInstance__TypeFlag(pThis),_i,_d=0;
		    for(_i=0;_i<s_ns;_i++) if(s_seen[_i]==_t){_d=1;break;}
		    if(!_d&&s_ns<256){ s_seen[s_ns++]=_t;
		      fprintf(stderr,"[qlog] type=0x%x RotY=%.3f qRot=(%.2f,%.2f,%.2f,%.2f) qGround=(%.2f,%.2f,%.2f,%.2f) qTemp2=(%.2f,%.2f,%.2f,%.2f) scale=(%.3f,%.3f,%.3f)\n",
		        _t, pThis->m_RotY,
		        pThis->m_qRot.x,pThis->m_qRot.y,pThis->m_qRot.z,pThis->m_qRot.t,
		        pThis->m_qGround.x,pThis->m_qGround.y,pThis->m_qGround.z,pThis->m_qGround.t,
		        qTemp2.x,qTemp2.y,qTemp2.z,qTemp2.t,
		        pThis->m_vScale.x,pThis->m_vScale.y,pThis->m_vScale.z); } } }
#endif


		// allow rotation of gallery portraits on the spot
		if (GetApp()->m_Mode == MODE_GALLERY)
		{
			CQuatern__ToMatrix(&qTemp2, mfBoundsOrient) ;
			CMtxF__PreMultScale(mfBoundsOrient, xzs*pThis->m_vScale.x, ys*pThis->m_vScale.y, xzs*pThis->m_vScale.z) ;

			CMtxF__PostMultTranslate(mfBoundsOrient, 0, -(pThis->ah.ih.m_pEA ? pThis->ah.ih.m_pEA->m_CollisionHeight : 0)/2, 0) ;

			CQuatern__ToMatrix(&GetApp()->m_qPortraitOrientation, mfTemp1) ;
			CMtxF__Copy(mfTemp2, mfBoundsOrient) ;
			CMtxF__PostMult4x4(mfBoundsOrient, mfTemp2, mfTemp1);

			CVector3__Clamp(&pThis->ah.ih.m_vPos, WORLD_LIMIT);
			CMtxF__PostMultTranslate(mfBoundsOrient, pThis->ah.ih.m_vPos.x, pThis->ah.ih.m_vPos.y, pThis->ah.ih.m_vPos.z) ;
			CMtxF__CopyPreMult3DSToU64(mfOrient, mfBoundsOrient) ;
		}
		else
		{
#if USE_ORIGINAL_MATRIX
			CQuatern__ToMatrix(&qTemp2, mfRotate);
			CVector3__Clamp(&pThis->ah.ih.m_vPos, WORLD_LIMIT);
			CMtxF__Translate(mfPos, pThis->ah.ih.m_vPos.x, pThis->ah.ih.m_vPos.y, pThis->ah.ih.m_vPos.z);
			CMtxF__Scale(mfScale, xzs*pThis->m_vScale.x, ys*pThis->m_vScale.y, xzs*pThis->m_vScale.z);
			CMtxF__PostMult(mfTemp1, mfScale, mfRotate);
			CMtxF__PostMult(mfBoundsOrient, mfTemp1, mfPos);
			CMtxF__PostMult(mfOrient, mf_3dstudio_to_u64, mfBoundsOrient);

#else
			// Using optimized matrix functions
			CQuatern__ToMatrix(&qTemp2, mfBoundsOrient) ;

			if (   (GetApp()->m_Mode != MODE_GAME)
				 || (CEngineApp__GetPlayer(GetApp()) == pThis)
				 || (CInstanceHdr__IsDevice(&pThis->ah.ih)) )
			{
				CMtxF__PreMultScale(mfBoundsOrient, xzs*pThis->m_vScale.x, ys*pThis->m_vScale.y, xzs*pThis->m_vScale.z) ;
			}
			else
			{
/*				if (GetApp()->m_dwCheatFlags & CHEATFLAG_BIGENEMYMODE)
					CMtxF__PreMultScale(mfBoundsOrient, xzs*pThis->m_vScale.x*3, ys*pThis->m_vScale.y*3, xzs*pThis->m_vScale.z*3) ;
				else*/ if (GetApp()->m_dwCheatFlags & CHEATFLAG_TINYENEMYMODE)
					CMtxF__PreMultScale(mfBoundsOrient, xzs*pThis->m_vScale.x*0.40, ys*pThis->m_vScale.y*0.40, xzs*pThis->m_vScale.z*0.40) ;
				else
					CMtxF__PreMultScale(mfBoundsOrient, xzs*pThis->m_vScale.x, ys*pThis->m_vScale.y, xzs*pThis->m_vScale.z) ;
			}

			CVector3__Clamp(&pThis->ah.ih.m_vPos, WORLD_LIMIT);
			CMtxF__PostMultTranslate(mfBoundsOrient, pThis->ah.ih.m_vPos.x, pThis->ah.ih.m_vPos.y, pThis->ah.ih.m_vPos.z) ;
			CMtxF__CopyPreMult3DSToU64(mfOrient, mfBoundsOrient) ;
#endif
		}
	}
	CMtxF__ToMtx(mfOrient, CGameObjectInstance__GetOrientationMatrix(pThis));
#ifdef PLATFORM_PORT
	{ extern char *getenv(const char*); extern int fprintf(void*,const char*,...); extern void *stderr;
	  static int s_m=-1, s_seen[256], s_ns=0; if(s_m<0) s_m=getenv("TUROK_QLOG")?1:0;
	  if(s_m && !FollowView){ int _t=(int)CGameObjectInstance__TypeFlag(pThis),_i,_d=0;
	    for(_i=0;_i<s_ns;_i++) if(s_seen[_i]==_t){_d=1;break;}
	    if(!_d&&s_ns<256){ s_seen[s_ns++]=_t;
	      fprintf(stderr,"[mtx] type=0x%x mfOrient diag=(%.3f,%.3f,%.3f) row0=(%.3f,%.3f,%.3f) trans=(%.0f,%.0f,%.0f)\n",
	        _t, mfOrient[0][0],mfOrient[1][1],mfOrient[2][2],
	        mfOrient[0][0],mfOrient[0][1],mfOrient[0][2],
	        mfOrient[3][0],mfOrient[3][1],mfOrient[3][2]); } } }
#endif


	// find bounds corners

	CIndexedSet__ConstructFromRawData(&isObjectInfo,
												 CCacheEntry__GetData(pThis->m_pceObjectInfo),
												 FALSE);

	pBounds = (CROMBounds*) CIndexedSet__GetBlock(&isObjectInfo, CART_OBJECTINFO_bounds);

	//CROMBounds__Print(pBounds);

	{
#ifdef PLATFORM_PORT
	/* CROMBounds m_vMin/m_vMax are big-endian model-space floats. Read raw they yield a
	 * garbage m_BoundsRect (~1e17), so the object FAILS the anim_bounds_rect overlap test in
	 * CScene__DrawInstances and is culled (invisible) — this is why animated objects/devices
	 * (e.g. AI_OBJECT_DEVICE_* platforms/elevators) did not draw. Swap component-wise (the
	 * CVector3 aggregate ORDERBYTES is a no-op); local copies avoid any double-swap. */
	CVector3 bmin, bmax;
	bmin.x = ORDERBYTES(pBounds->m_vMin.x); bmin.y = ORDERBYTES(pBounds->m_vMin.y); bmin.z = ORDERBYTES(pBounds->m_vMin.z);
	bmax.x = ORDERBYTES(pBounds->m_vMax.x); bmax.y = ORDERBYTES(pBounds->m_vMax.y); bmax.z = ORDERBYTES(pBounds->m_vMax.z);
#else
	CVector3 bmin = pBounds->m_vMin, bmax = pBounds->m_vMax;
#endif
	vCorners[0].x = bmin.x; vCorners[0].y = bmin.y; vCorners[0].z = bmin.z;
	vCorners[1].x = bmax.x; vCorners[1].y = bmin.y; vCorners[1].z = bmin.z;
	vCorners[2].x = bmax.x; vCorners[2].y = bmin.y; vCorners[2].z = bmax.z;
	vCorners[3].x = bmin.x; vCorners[3].y = bmin.y; vCorners[3].z = bmax.z;
	vCorners[4].x = bmin.x; vCorners[4].y = bmax.y; vCorners[4].z = bmin.z;
	vCorners[5].x = bmax.x; vCorners[5].y = bmax.y; vCorners[5].z = bmin.z;
	vCorners[6].x = bmax.x; vCorners[6].y = bmax.y; vCorners[6].z = bmax.z;
	vCorners[7].x = bmin.x; vCorners[7].y = bmax.y; vCorners[7].z = bmax.z;
	}

	for (c=0; c<8; c++)
		CMtxF__VectorMult(mfBoundsOrient, &vCorners[c], &vTCorners[c]);

	CBoundsRect__EncloseVectors(&pThis->m_BoundsRect, 8, vTCorners);

	CIndexedSet__Destruct(&isObjectInfo);
#endif
}

void CGameObjectInstance__SetDesiredAnim(CGameObjectInstance *pThis, int nAnim)
{
	ASSERT(nAnim != -1);
	ASSERT(nAnim >= 0);

	if (nAnim != pThis->m_nDesiredAnim)
		CGameObjectInstance__SetAnimChanged(pThis);

	pThis->m_nDesiredAnim = nAnim;
	pThis->m_asCurrent.m_CycleCompleted = FALSE;
}

void CGameObjectInstance__RestartAnim(CGameObjectInstance *pThis)
{
	float fraction;

	if (pThis->m_asCurrent.m_pceAnim)
	{
		fraction = pThis->m_asCurrent.m_cFrame - ((int) pThis->m_asCurrent.m_cFrame);
		pThis->m_asCurrent.m_cFrame 			= 1 + fraction; // skip frame 0

		pThis->m_asCurrent.m_CycleCompleted = FALSE;

		CGameObjectInstance__SetNotBlending(pThis);
	}
}

#ifndef WIN32

void CGameObjectInstance__Reset(CGameObjectInstance *pThis,
										  CIndexedSet *pisObjectsIndex, ROMAddress *rpObjectsAddress,
										  int StartupAnimType)	// = -1
{
	TRACE0("CGameObjectInstance__Reset()\r\n");
	ASSERT(pisObjectsIndex);

	CGameObjectInstance__SetNotVisible(pThis);
	CGameObjectInstance__SetNotBlending(pThis);
	CGameObjectInstance__SetAnimChanged(pThis);

	pThis->m_StartupAnimType = StartupAnimType;

	pThis->m_rpObjectInfo = NULL;
	pThis->m_pceObjectInfo = NULL;

	// calculate address of object index
	pThis->m_rpObject = CIndexedSet__GetROMAddress(pisObjectsIndex,
																  rpObjectsAddress,
																  pThis->ah.ih.m_nObjType);

	// request object index
   CCartCache__RequestBlock(GetCache(),
                            pThis, NULL, (pfnCACHENOTIFY) CGameObjectInstance__ObjectIndexReceived,
									 &pThis->m_pceObjectIndex,
                            pThis->m_rpObject, IS_INDEX_SIZE(CART_GRAPHOBJ_nItems),
                            "Object Index");
}

void CGameObjectInstance__ObjectIndexReceived(CGameObjectInstance *pThis, CCacheEntry **ppceTarget)
{
	CIndexedSet		isObjectIndex;
	DWORD				dwSize;

	ASSERT(pThis->m_pceObjectIndex == *ppceTarget);
	TRACE0("CGameObjectInstance: Object index received.\r\n");

	CIndexedSet__ConstructFromRawData(&isObjectIndex,
												 CCacheEntry__GetData(pThis->m_pceObjectIndex),
												 FALSE);


	// calculate address and size of object info
	pThis->m_rpObjectInfo = CIndexedSet__GetROMAddressAndSize(&isObjectIndex,
																				 pThis->m_rpObject,
																				 CART_GRAPHOBJ_isObjectInfo, &dwSize);
	pThis->m_ObjectInfoSize = (WORD) dwSize;

	// request object info
	CCartCache__RequestBlock(GetCache(),
									 pThis, NULL, (pfnCACHENOTIFY) CGameObjectInstance__ObjectInfoReceived,
									 &pThis->m_pceObjectInfo,
									 pThis->m_rpObjectInfo, pThis->m_ObjectInfoSize,
									 "Object Info");


   // calculate address of animations index
   pThis->m_rpAnims = CIndexedSet__GetROMAddress(&isObjectIndex,
																		pThis->m_rpObject,
																		CART_GRAPHOBJ_isAnims);

   // request animations header (need to know number of items to know index size)
   CCartCache__RequestBlock(GetCache(),
                            pThis, NULL, (pfnCACHENOTIFY) CGameObjectInstance__AnimsHeaderReceived,
                            &pThis->m_pceAnimsHeader,
                            pThis->m_rpAnims, IS_HEADER_SIZE,
                            "Animations Header");

	CIndexedSet__Destruct(&isObjectIndex);
}

void CGameObjectInstance__ObjectInfoReceived(CGameObjectInstance *pThis, CCacheEntry **ppceTarget)
{
	CVector3	vTCorners[8];
	CMtxF		mfOrient;

	ASSERT(pThis->m_pceObjectInfo == *ppceTarget);

	CGameObjectInstance__CalculateOrientationMatrix(pThis, FALSE, vTCorners, mfOrient);
}

void CGameObjectInstance__AnimsHeaderReceived(CGameObjectInstance *pThis, CCacheEntry **ppceTarget)
{
	CIndexedSet		isAnimsHeader;

	ASSERT(pThis->m_pceAnimsHeader == *ppceTarget);
	TRACE0("CGameObjectInstance: Animations header received.\r\n");

	CIndexedSet__ConstructFromRawData(&isAnimsHeader,
												 CCacheEntry__GetData(pThis->m_pceAnimsHeader),
												 FALSE);


	pThis->m_nAnims = (int) CIndexedSet__GetBlockCount(&isAnimsHeader);
	TRACE("CGameObjectIndex: %d animations.\r\n", pThis->m_nAnims);

	// request animations index
   CCartCache__RequestBlock(GetCache(),
                            pThis, NULL, (pfnCACHENOTIFY) CGameObjectInstance__AnimsIndexReceived,
                            &pThis->m_pceAnimsIndex,
                            pThis->m_rpAnims, IS_INDEX_SIZE(pThis->m_nAnims),
                            "Animations Index");

	CIndexedSet__Destruct(&isAnimsHeader);
}

void CGameObjectInstance__AnimsIndexReceived(CGameObjectInstance *pThis, CCacheEntry **ppceTarget)
{
	ASSERT(pThis->m_pceAnimsIndex == *ppceTarget);
	TRACE0("CGameObjectInstance: Animations index received.\r\n");

	if (pThis->m_StartupAnimType == -1)
	{
		pThis->m_nDesiredAnim = 0;
	}
	else
	{
		pThis->m_nDesiredAnim = CGameObjectInstance__LookupAIAnimType(pThis, pThis->m_StartupAnimType);
		if (pThis->m_nDesiredAnim == -1)
			pThis->m_nDesiredAnim = 0;
	}

	// CHECK
	CGameObjectInstance__RequestInitialAnimation(pThis);
}

void CGameObjectInstance__RequestInitialAnimation(CGameObjectInstance *pThis)
{
	CIndexedSet		isAnimsIndex;
	ROMAddress		*rpAnimAddress;
	DWORD				animSize;

	if (pThis->m_nAnims)
	{
		// invalidate pointer first time around only
		// other times, I'll use old animation until new one comes around
		pThis->m_asCurrent.m_pceAnim 	= NULL;
		pThis->m_asCurrent.m_nAnim 	= pThis->m_nDesiredAnim;
		CGameObjectInstance__SetNotBlending(pThis);

		CIndexedSet__ConstructFromRawData(&isAnimsIndex,
												 	 CCacheEntry__GetData(pThis->m_pceAnimsIndex),
												 	 FALSE);

		// calculate anim address
		rpAnimAddress = CIndexedSet__GetROMAddressAndSize(&isAnimsIndex,
																  	  	  pThis->m_rpAnims,
																  	  	  pThis->m_nDesiredAnim, &animSize);

		// request animation data
		CCartCache__RequestBlock(GetCache(),
									 	 pThis, (pfnCACHENOTIFY) CGameObjectInstance__DecompressAnim, (pfnCACHENOTIFY) CGameObjectInstance__AnimReceived,
									 	 &pThis->m_asCurrent.m_pceAnim,
									 	 rpAnimAddress, animSize,
									 	 "Animation (Initial)");

		CIndexedSet__Destruct(&isAnimsIndex);
	}
}

void CGameObjectInstance__AnimReceived(CGameObjectInstance *pThis, CCacheEntry **ppceTarget)
{
	CIndexedSet			isAnim,
							isTransitionTable;
	CUnindexedSet		usYRots;
	void					*pbAnim,
							*pbYRots,
							*pbTransitionTable;
	CROMTransition		*pTransition;
	float					fraction;

	ASSERT(ppceTarget == &pThis->m_asCurrent.m_pceAnim);
	TRACE0("CGameObjectInstance: Animation received.\r\n");

	pbAnim = CCacheEntry__GetData(pThis->m_asCurrent.m_pceAnim);

	CIndexedSet__ConstructFromRawData(&isAnim, pbAnim, FALSE);

	// translation collection
	pbYRots = CIndexedSet__GetBlock(&isAnim, CART_ANIM_usYRotations);
	CUnindexedSet__ConstructFromRawData(&usYRots, pbYRots, FALSE);

	pThis->m_asCurrent.m_nFrames 			= CUnindexedSet__GetBlockCount(&usYRots);
	pThis->m_asCurrent.m_CycleCompleted	= FALSE;

	fraction = pThis->m_asCurrent.m_cFrame - ((int) pThis->m_asCurrent.m_cFrame);

	if (CGameObjectInstance__IsAnimChanged(pThis))
	{
		if (pThis->m_AI.m_dwStatusFlags2 & AI_IGNORESOUNDEVENTS)
		{
			// go to last frame
			pThis->m_asCurrent.m_cFrame = (pThis->m_asCurrent.m_nFrames - 1) + fraction;

			CGameObjectInstance__SendAllDoorEvents(pThis, &isAnim);
		}
		else
		{
			// a new anim has been loaded
			// start on frame one
			pThis->m_asCurrent.m_cFrame = 1 + fraction; // skip frame 0
		}
	}
	else
	{
		// an old animation has been reloaded
		// keep current frame
		if (pThis->m_asCurrent.m_cFrame >= pThis->m_asCurrent.m_nFrames)
		{
			pThis->m_asCurrent.m_cFrame = pThis->m_asCurrent.m_nFrames - 1 + fraction;
		}
	}
	CGameObjectInstance__SetAnimNotChanged(pThis);


	ASSERT(pThis->m_asCurrent.m_nFrames >= 2);	// animations must have at least 2 frames

	// setup blend if necessary
	if (pThis->m_WaitingForAnim != -1)
	{
		// get transition table block
		pbTransitionTable = CIndexedSet__GetBlock(&isAnim, CART_ANIM_isTransitionTable);
		CIndexedSet__ConstructFromRawData(&isTransitionTable, pbTransitionTable, FALSE);

		pTransition = (CROMTransition*) CIndexedSet__GetBlock(&isTransitionTable, CART_TRANTABLE_transition);

		// only start a blend if the object has a blend, and the object is visible
		// or it's a special "animate when not visible" object
	   if (		ORDERBYTES(pTransition->m_BlendFromLength)	/* PORT: big-endian WORDs */
				&& (		(CGameObjectInstance__IsVisible(pThis))
						||	(CGameObjectInstance__TypeFlag(pThis) == AI_OBJECT_DEVICE_ANIMOFFSCREENACTION) ) )
	   {
			TRACE0("CGameObjectInstance: Start blend.\r\n");

			CGameObjectInstance__SetBlending(pThis);
	      pThis->m_BlendLength = ORDERBYTES(pTransition->m_BlendFromLength);
	      pThis->m_BlendPos 	= 0;
			pThis->m_BlendStart	= ORDERBYTES(pTransition->m_BlendStart);
			pThis->m_BlendFinish	= ORDERBYTES(pTransition->m_BlendFinish);
	   }
		else
		{
			CGameObjectInstance__SetNotBlending(pThis);
		}

		pThis->m_asCurrent.m_nAnim = pThis->m_WaitingForAnim;
		pThis->m_WaitingForAnim = -1;

		if (CGameObjectInstance__IsAbsoluteAnim(pThis, pThis->m_asBlend.m_nAnim))
			if (!CGameObjectInstance__IsAbsoluteAnim(pThis, pThis->m_asCurrent.m_nAnim))
				CGameObjectInstance__MoveToEndOfAbsoluteAnim(pThis, &pThis->m_asBlend);

		CIndexedSet__Destruct(&isTransitionTable);
	}

	CIndexedSet__Destruct(&isAnim);
	CUnindexedSet__Destruct(&usYRots);
}

CVector3 CGameObjectInstance__GetNodeBlendPos(CGameObjectInstance *pThis, CGameAnimateState *pasAnim, int nNode)
{
	float				fraction;
	int				nFrameC, nNextFrameC;
	CIndexedSet		isAnim;
	CVector3			vBlend, vA, vB,
						vDelta, vRotDelta;
#if USE_ORIGINAL_MATRIX
	CMtxF				mfRotate, mfRotY;
#endif
	CMtxF				mfRotProd;

	ASSERT(pasAnim->m_pceAnim);

	fraction = pasAnim->m_cFrame - ((int) pasAnim->m_cFrame);

	nFrameC = max(0, (int) pasAnim->m_cFrame);
	nNextFrameC = (int) CGameAnimateState__GetNextFrame(pasAnim);

	CIndexedSet__ConstructFromRawData(&isAnim,
												 CCacheEntry__GetData(pasAnim->m_pceAnim),
												 FALSE);

	vA	= CGameObjectInstance__GetNodePos(&isAnim, nNode, nFrameC);
	vB	= CGameObjectInstance__GetNodePos(&isAnim, nNode, nNextFrameC);

	CVector3__Blend(&vBlend, fraction, &vA, &vB);

	vDelta.x = vBlend.x;
	vDelta.y = 0;
	vDelta.z = vBlend.y;

	// rotate delta
#if USE_ORIGINAL_MATRIX
	CQuatern__ToMatrix(&pThis->m_qRot, mfRotate);
	CMtxF__RotateY(mfRotY, pThis->m_RotY);
	CMtxF__PostMult(mfRotProd, mfRotY, mfRotate);
#else
	// use optimized matrix functions
	CQuatern__ToMatrix(&pThis->m_qRot, mfRotProd);
	CMtxF__PreMultRotateY(mfRotProd, pThis->m_RotY);
#endif

	CMtxF__VectorMult(mfRotProd, &vDelta, &vRotDelta);

	CIndexedSet__Destruct(&isAnim);

	vRotDelta.x *= pThis->m_vScale.x;
	vRotDelta.y *= pThis->m_vScale.y;
	vRotDelta.z *= pThis->m_vScale.z;

	return vRotDelta;
}

void CGameObjectInstance__MoveToEndOfAbsoluteAnim(CGameObjectInstance *pThis, CGameAnimateState *pasAnim)
{
	CIndexedSet		isAnim;
	CVector3			vStart, vCurrent,
						vDelta, vRotDelta,
						vDesired;
#if USE_ORIGINAL_MATRIX
	CMtxF				mfRotate, mfRotY;
#endif
	CMtxF				mfRotProd;

	ASSERT(pasAnim->m_pceAnim);

	CIndexedSet__ConstructFromRawData(&isAnim,
												 CCacheEntry__GetData(pasAnim->m_pceAnim),
												 FALSE);

	vStart	= CGameObjectInstance__GetNodePos(&isAnim, 0, 0);
	vCurrent	= CGameObjectInstance__GetNodePos(&isAnim, 0, (int) pasAnim->m_cFrame);

	vDelta.x = vCurrent.x - vStart.x;
	vDelta.y = 0;
	vDelta.z = vCurrent.y - vStart.y;

	// rotate delta
#if USE_ORIGINAL_MATRIX
	CQuatern__ToMatrix(&pThis->m_qRot, mfRotate);
	CMtxF__RotateY(mfRotY, pThis->m_RotY);
	CMtxF__PostMult(mfRotProd, mfRotY, mfRotate);
#else
	// use optimized matrix functions
	CQuatern__ToMatrix(&pThis->m_qRot, mfRotProd);
	CMtxF__PreMultRotateY(mfRotProd, pThis->m_RotY);
#endif

	CMtxF__VectorMult(mfRotProd, &vDelta, &vRotDelta);

	// make sure there is still no Y change
	vDesired.x = pThis->ah.ih.m_vPos.x + vRotDelta.x*pThis->m_vScale.x;
	vDesired.y = pThis->ah.ih.m_vPos.y;
	vDesired.z = pThis->ah.ih.m_vPos.z + vRotDelta.z*pThis->m_vScale.z;

	CInstanceHdr__GetNearPositionAndRegion(&pThis->ah.ih, vDesired, &pThis->ah.ih.m_vPos, &pThis->ah.ih.m_pCurrentRegion,
														INTERSECT_BEHAVIOR_IGNORE, INTERSECT_BEHAVIOR_IGNORE);

	CIndexedSet__Destruct(&isAnim);
}

/*
#define JERK_FRAMES	3
void CGameObjectInstance__Jerk(CGameObjectInstance *pThis)
{
	float fraction;

	fraction = pThis->m_asCurrent.m_cFrame - ((int) pThis->m_asCurrent.m_cFrame);

	pThis->m_asCurrent.m_cFrame = max(fraction + 1, pThis->m_asCurrent.m_cFrame - JERK_FRAMES);

	if (CGameObjectInstance__IsBlending(pThis))
		pThis->m_asBlend.m_cFrame = max(fraction + 1, pThis->m_asBlend.m_cFrame - JERK_FRAMES);
}
*/

CCollisionInfo2* CGameObjectInstance__GetCollisionInfo(CGameObjectInstance *pThis)
{
	CCollisionInfo2	*pCI;
	BOOL					climber, flyer;
	int					waterState;

	// Special cinema mode checks for the player
	if (	 (CCamera__InCinemaMode(&GetApp()->m_Camera))
		 && (pThis == CEngineApp__GetPlayer(GetApp())) )	
	{
		pCI = &ci_player ;
		switch(CTurokMovement.WaterFlag)
		{
			case PLAYER_BELOW_WATERSURFACE:
			case PLAYER_ON_WATERSURFACE:
				pCI = &ci_playerunderwater ;
				break;

			default:
				pCI = &ci_player ;
				pCI->dwFlags |= COLLISIONFLAG_MOVETHROUGHDOORS ;
				pCI->dwFlags &= ~COLLISIONFLAG_USEWALLRADIUS ;
				pCI->InstanceBehavior = INTERSECT_BEHAVIOR_IGNORE ;
				pCI->WallBehavior = INTERSECT_BEHAVIOR_IGNORE ;

				break;
		}
	}
	else
	if (pThis->m_pBoss)
	{
		pCI = &pThis->m_pBoss->m_CollisionInfo;


// NEEDS TO BE WORKED INTO BOSS COLLISION INFO STRUCT
//	// Allow TRex to get off elevator at beginning of level
//	// and Humvee to enter arena at beginning of level
//	if ((pThis->m_pBoss) && (pThis->m_pBoss->m_ModeFlags & BF_ENTER_MODE))
//	{
//		climber = TRUE ;
//		climbDown = TRUE ;
//	}
	}
	else if (pThis->ah.ih.m_pEA->m_dwTypeFlags & AI_TYPE_NOWALLCOLLISION)
	{
		pCI = &ci_nocollisionatall;
	}
	else
	{
		climber = pThis->ah.ih.m_pEA->m_dwTypeFlags & AI_TYPE_CLIMBER;
		flyer = pThis->ah.ih.m_pEA->m_dwTypeFlags & AI_TYPE_FLYING;

		waterState = AI_GetWaterStatus(pThis);
		if (		(waterState == AI_BELOW_WATERSURFACE)
				||	(waterState == AI_ON_WATERSURFACE) )
		{
			if (pThis->m_AI.m_Health)
			{
				if (climber)
				{
					pCI = &ci_underwater_climbing_character;
				}
				else
				{
//					if (CGameObjectInstance__TypeFlag(pThis) == AI_OBJECT_CHARACTER_SLUDGEBOY)
//						pCI = &ci_underwater_walk_character;
//					else
					pCI = &ci_underwater_character;
				}
			}
			else
			{
				// ai is dead - does it sink or float ?
				if (AI_DoesItFloat(pThis))
					pCI = &ci_underwater_character_float_dead;
				else
					pCI = &ci_underwater_character_dead;
			}
		}
		else
		{
			if (pThis->m_AI.m_Health)
			{
				if (flyer)
				{
					pCI = &ci_flying_character;
				}
				else
				{
					if (pThis->ah.ih.m_pEA->m_dwTypeFlags & AI_TYPE_AVOIDWATER)
					{
						if (climber)
							pCI = &ci_climbing_character;
						else
							pCI = &ci_character;
					}
					else if (pThis->ah.ih.m_pEA->m_dwTypeFlags2 & AI_TYPE2_STAYINWATER)
					{
						pCI = &ci_underwater_character;
					}
					else
					{
						if (climber)
							pCI = &ci_climbing_swimming_character;
						else
							pCI = &ci_character_walkinwater;
					}
				}
			}
			else
			{
				pCI = &ci_dead_character;
			}
		}
	}

	return pCI;
}

void CGameObjectInstance__Advance(CGameObjectInstance *pThis)
{
	int					nTransitionAnim;
	CIndexedSet			isAnimsIndex;
	ROMAddress			*rpAnimAddress;
	DWORD					animSize;
	float					fraction,
							u,
#ifndef SHIP_IT
							prevCurrent,
#endif
							prevBlend;
	CVector3				vOldPos, vDesiredPos;
	CQuatern				qNewGround;
	BOOL					collided,
							player,
							onGround,
							nearWater,
							pressurePlate;
	CCollisionInfo2	*pCI;
	CROMRegionSet		*pRegionSet;

	ASSERT((DWORD) pThis >= 0x80000000);
	ASSERT((DWORD) pThis <= 0x84000000);

	ASSERT((DWORD) (&pThis->m_AI) >= 0x80000000);
	ASSERT((DWORD) (&pThis->m_AI) <= 0x84000000);

	// don't update stuff in pause mode (or training mode)...
	if ((GetApp()->m_bPause) || (GetApp()->m_bTraining))
		return ;



	if (pThis->m_WaitingForAnim == -1)
	{
		// not waiting for animation from cache

		// update desired animation
		ASSERT(pThis->m_nDesiredAnim >= 0);
		CGameAnimateState__SetDesiredAnim(&pThis->m_asCurrent, pThis->m_nDesiredAnim);

		nTransitionAnim = CGameAnimateState__GetTransitionAnim(&pThis->m_asCurrent);

		if (nTransitionAnim != -1)
		{
			// initiate a transition

			ASSERT(nTransitionAnim >= 0);
			ASSERT(nTransitionAnim < pThis->m_nAnims);
			
			// keep AI from waiting for cycle to complete
			if (nTransitionAnim != pThis->m_nDesiredAnim)
				pThis->m_AI.m_dwStatusFlags &= ~AI_WAITFOR_CYCLE;

			pThis->m_asBlend = pThis->m_asCurrent;

			CGameObjectInstance__SetNotBlending(pThis);
			pThis->m_WaitingForAnim	= nTransitionAnim;

			// request new animation
			// blend is set up when anim is received

			CIndexedSet__ConstructFromRawData(&isAnimsIndex,
												 	 	 CCacheEntry__GetData(pThis->m_pceAnimsIndex),
												 	 	 FALSE);

			// calculate anim address
			rpAnimAddress = CIndexedSet__GetROMAddressAndSize(&isAnimsIndex,
																  	  	  	  pThis->m_rpAnims,
																  	  	  	  nTransitionAnim, &animSize);

			// request animation data (NEED TO ADD DECOMPRESSION)
			CCartCache__RequestBlock(GetCache(),
									 	 	 pThis, (pfnCACHENOTIFY) CGameObjectInstance__DecompressAnim, (pfnCACHENOTIFY) CGameObjectInstance__AnimReceived,
									 	 	 &pThis->m_asCurrent.m_pceAnim,
									 	 	 rpAnimAddress, animSize,
									 	 	 "Animation");

			CIndexedSet__Destruct(&isAnimsIndex);
		}

		if (CGameObjectInstance__IsBlending(pThis))
		{
			pThis->m_BlendPos += frame_increment;

			if (pThis->m_BlendPos > pThis->m_BlendLength)
	   	{
				TRACE0("CGameObjectInstance: End blend.\r\n");

				CGameObjectInstance__SetNotBlending(pThis);  // set pceAnim to NULL here
	   	}
	   	else
	   	{
				u = pThis->m_BlendPos / ((pThis->m_BlendLength)+frame_increment);
				//pThis->m_uBlender = GetCosineBlender(u);
				pThis->m_uBlender = GetHermiteBlender(pThis->m_BlendStart, pThis->m_BlendFinish, u);

				CGameAnimateState__Advance(&pThis->m_asBlend);
	   	}
		}

		CGameAnimateState__Advance(&pThis->m_asCurrent);

		// sync blend with current animation
		if (pThis->m_WaitingForAnim != -1)
		{
			pThis->m_asBlend = pThis->m_asCurrent;
		}
		else
		{
			// sync up fraction (avoids accumulated errors)
			fraction = pThis->m_asCurrent.m_cFrame - ((int) pThis->m_asCurrent.m_cFrame);

			// avoid precision problems
			if (fraction < 0)
				fraction = 0;

			prevBlend = pThis->m_asBlend.m_cFrame;
			pThis->m_asBlend.m_cFrame = ((int) pThis->m_asBlend.m_cFrame) + fraction;

			// avoid precision problems
			if (((int) pThis->m_asBlend.m_cFrame) > ((int) prevBlend))
			{
#ifndef SHIP_IT
				prevCurrent = pThis->m_asCurrent.m_cFrame;
#endif
				pThis->m_asCurrent.m_cFrame = max(0, pThis->m_asCurrent.m_cFrame - 0.01);
				pThis->m_asBlend.m_cFrame = max(0, pThis->m_asBlend.m_cFrame - 0.01);

				ASSERT(((int) pThis->m_asCurrent.m_cFrame) == ((int) prevCurrent));
				ASSERT(((int) pThis->m_asBlend.m_cFrame) == ((int) prevBlend));
			}
		}
	}
	else // (pThis->m_WaitingForAnim != -1)
	{
		// waiting for animation from cache

		ASSERT(!CGameObjectInstance__IsBlending(pThis));

		// keep playing current animation
		CGameAnimateState__Advance(&pThis->m_asCurrent);
		// sync blend with current animation
		pThis->m_asBlend = pThis->m_asCurrent;
	}

	if (!(CGameObjectInstance__IsDevice(pThis) || CGameObjectInstance__IsAbsoluteAnim(pThis, pThis->m_asCurrent.m_nAnim)))
	{
		player = (pThis == CEngineApp__GetPlayer(GetApp()));
		onGround = CInstanceHdr__IsOnGround(&pThis->ah.ih);
		if ((!player) || (CCamera__InCinemaMode(&GetApp()->m_Camera)))
		{
			vOldPos = pThis->ah.ih.m_vPos;

			// change to absolute translation for gallery mode!
			if (		(frame_increment != 0)
					&&	(!(pThis->m_AI.m_dwStatusFlags & AI_ALREADY_DEAD))
					&& (!(pThis->m_AI.m_dwStatusFlags & AI_INTERACTIVEANIMATION))
					&& (!(pThis->m_AI.m_dwStatusFlags & AI_INTERANIMDELAY))
					&&	(GetApp()->m_Mode != MODE_GALLERY) )
			{
				CGameObjectInstance__AdvanceAccumulation(pThis);
			}

			vDesiredPos = pThis->ah.ih.m_vPos;
			pThis->ah.ih.m_vPos = vOldPos;

			pCI = CGameObjectInstance__GetCollisionInfo(pThis);
			CCollisionInfo__ClearCollision(pCI);

			// perform collision
			collided = FALSE;
			if (frame_increment != 0)
			{
				if (		(!(pThis->m_AI.m_dwStatusFlags & AI_INTERACTIVEANIMATION))
						&& (!(pThis->m_AI.m_dwStatusFlags & AI_INTERANIMDELAY)) )
				{
					collided = CAnimInstanceHdr__Collision3(&pThis->ah, vDesiredPos, pCI);
				}

				// call collision function if there is one
				if (pThis->m_pCollisionFunction)
					pThis->m_pCollisionFunction(pThis, pCI);
			}

			// Set collision flag for AI
			if (collided && (pCI->pInstanceCollision != &(CEngineApp__GetPlayer(GetApp())->ah.ih)))
				pThis->m_AI.m_dwStatusFlags |= AI_COLLISION;
			else
				pThis->m_AI.m_dwStatusFlags &= (~AI_COLLISION);

			ASSERT(((DWORD)pThis) > 0x80000000);
			if (pThis->ah.ih.m_pCurrentRegion)
				ASSERT(((DWORD)pThis->ah.ih.m_pCurrentRegion) > 0x80000000);

			pRegionSet = CScene__GetRegionAttributes(&GetApp()->m_Scene, pThis->ah.ih.m_pCurrentRegion);
			if (pRegionSet)
			{
				if (pRegionSet->m_dwFlags & REGFLAG_PRESSUREPLATE)
				{
					if (pRegionSet->m_dwFlags & REGFLAG_PRESSUREPLATENEEDSCOLLISION)
						pressurePlate = (BOOL) pCI->pWallCollisionRegion;
					else
						pressurePlate = TRUE;

					if (pressurePlate && (AI_GetEA(pThis)->m_dwTypeFlags & AI_TYPE_USEPRESSUREPLATES))
					{
						AI_Event_Dispatcher(&pThis->ah.ih, &pThis->ah.ih, AI_MEVENT_PRESSUREPLATE, AI_SPECIES_ALL, pThis->ah.ih.m_vPos, (float) pRegionSet->m_PressureID);
					}
				}
			}
		}

		if (pThis->ah.ih.m_pCurrentRegion)
		{
			qNewGround = CInstanceHdr__GetGroundRotation(&pThis->ah.ih);

			//if (!(pThis->m_dwFlags & ANIM_INSTANCE_NEAREDGE))
			//{
			CQuatern__Blend(&pThis->m_qShadow,
								 min(1, NEWSHADOWROT_FACTOR*frame_increment),
								 &pThis->m_qShadow, &qNewGround);
			//}


			nearWater = FALSE;
			if (pThis->ah.ih.m_pCurrentRegion->m_wFlags & REGFLAG_HASWATER)
				nearWater = TRUE;

			if (pThis->ah.ih.m_pEA->m_dwTypeFlags & AI_TYPE_TRACKGROUND)
			{
				if (nearWater)
					CQuatern__Identity(&qNewGround);

				if (player)
				{
					if (		(!(pThis->m_dwFlags & ANIM_INSTANCE_HANGINGON) && onGround && player_is_moving)
							|| nearWater )
					{
						CQuatern__GetCloser(&pThis->m_qGround, &qNewGround);
					}
				}
				else
				{
					if (onGround && !(pThis->m_dwFlags & ANIM_INSTANCE_HANGINGON))
					{
						CQuatern__Blend(&pThis->m_qGround,
											 min(1, NEWGROUNDROT_FACTOR*frame_increment),
											 &pThis->m_qGround, &qNewGround);
					}
					else
					{
						CQuatern__Blend(&pThis->m_qGround,
											 min(1, NEWGROUNDROT_FACTOR*0.3*frame_increment),
											 &pThis->m_qGround, &qNewGround);
					}
				}
			}
		}
	}
}

CROMNodeIndex CGameObjectInstance__GetNodeAnimIndex(CIndexedSet *pisAnim, int nNode)
{
	void				*pbNodeAnimIndices;
	CUnindexedSet	usNodeAnimIndices;
	CROMNodeIndex	*pNodeAnimIndices;

	pbNodeAnimIndices = CIndexedSet__GetBlock(pisAnim, CART_ANIM_usNodeAnimIndices);
	CUnindexedSet__ConstructFromRawData(&usNodeAnimIndices, pbNodeAnimIndices, FALSE);

	ASSERT(nNode < CUnindexedSet__GetBlockCount(&usNodeAnimIndices));
	pNodeAnimIndices = (CROMNodeIndex*) CUnindexedSet__GetBasePtr(&usNodeAnimIndices);

	CUnindexedSet__Destruct(&usNodeAnimIndices);

	return pNodeAnimIndices[nNode];
}

CVector3 CGameObjectInstance__GetNodePos(CIndexedSet *pisAnim, int nNode, int nFrame)
{
	CROMNodeIndex				nodeAnimIndex;
	void							*pbInitialOrients,
									*pbTransSets,
									*pbPosFrames;
	CUnindexedSet				usInitialOrients,
									usTransSets,
									usPosFrames;
	CROMInitialOrientation	*initialOrients;
	CPosFrame					*pPosFrames;

	ASSERT(nFrame >= 0);

	nodeAnimIndex = CGameObjectInstance__GetNodeAnimIndex(pisAnim, nNode);
	if (nodeAnimIndex.m_nTranslationSet == -1)
	{
		// get initial translation
		pbInitialOrients = CIndexedSet__GetBlock(pisAnim, CART_ANIM_usInitialOrients);
		CUnindexedSet__ConstructFromRawData(&usInitialOrients, pbInitialOrients, FALSE);

		ASSERT(nNode < CUnindexedSet__GetBlockCount(&usInitialOrients));
		initialOrients = (CROMInitialOrientation*) CUnindexedSet__GetBasePtr(&usInitialOrients);

		CUnindexedSet__Destruct(&usInitialOrients);

		return initialOrients[nNode].m_vPos;
	}
	else
	{
		// get keyframed translation

		pbTransSets = CIndexedSet__GetBlock(pisAnim, CART_ANIM_usTranslationSets);
		CUnindexedSet__ConstructFromRawData(&usTransSets, pbTransSets, FALSE);

		ASSERT(nodeAnimIndex.m_nTranslationSet < CUnindexedSet__GetBlockCount(&usTransSets));
		pbPosFrames = CUnindexedSet__GetBlock(&usTransSets, nodeAnimIndex.m_nTranslationSet);
		CUnindexedSet__ConstructFromRawData(&usPosFrames, pbPosFrames, FALSE);

		pPosFrames = (CPosFrame*) CUnindexedSet__GetBasePtr(&usPosFrames);
		ASSERT(nFrame < CUnindexedSet__GetBlockCount(&usPosFrames));

		CUnindexedSet__Destruct(&usTransSets);
		CUnindexedSet__Destruct(&usPosFrames);

		return pPosFrames[nFrame].m_vPos;
	}
}

CQuatern CGameObjectInstance__GetNodeRot(CIndexedSet *pisAnim, int nNode, int nFrame)
{
	CROMNodeIndex				nodeAnimIndex;
	void							*pbInitialOrients,
									*pbRotSets,
									*pbRotFrames;
	CUnindexedSet				usInitialOrients,
									usRotSets,
									usRotFrames;
	CROMInitialOrientation	*initialOrients, *pInit;
	CRotFrame					*pRotFrames, *pFrame;
	CQuatern						qRot;

	ASSERT(nFrame >= 0);

	nodeAnimIndex = CGameObjectInstance__GetNodeAnimIndex(pisAnim, nNode);

	if (nodeAnimIndex.m_nRotationSet == -1)
	{
		// get initial rotation

		pbInitialOrients = CIndexedSet__GetBlock(pisAnim, CART_ANIM_usInitialOrients);
		CUnindexedSet__ConstructFromRawData(&usInitialOrients, pbInitialOrients, FALSE);

		ASSERT(nNode < CUnindexedSet__GetBlockCount(&usInitialOrients));
		initialOrients = (CROMInitialOrientation*) CUnindexedSet__GetBasePtr(&usInitialOrients);

		CUnindexedSet__Destruct(&usInitialOrients);

		//return initialOrients[nNode].m_qRot;
		pInit = &initialOrients[nNode];

		qRot.x = INT162FLOAT(pInit->m_RotX, 1);
		qRot.y = INT162FLOAT(pInit->m_RotY, 1);
		qRot.z = INT162FLOAT(pInit->m_RotZ, 1);
		qRot.t = INT162FLOAT(pInit->m_RotT, 1);
	}
	else
	{
		// get keyframed rotation

		pbRotSets = CIndexedSet__GetBlock(pisAnim, CART_ANIM_usRotationSets);
		CUnindexedSet__ConstructFromRawData(&usRotSets, pbRotSets, FALSE);

		ASSERT(nodeAnimIndex.m_nRotationSet < CUnindexedSet__GetBlockCount(&usRotSets));
		pbRotFrames = CUnindexedSet__GetBlock(&usRotSets, nodeAnimIndex.m_nRotationSet);
		CUnindexedSet__ConstructFromRawData(&usRotFrames, pbRotFrames, FALSE);

		pRotFrames = (CRotFrame*) CUnindexedSet__GetBasePtr(&usRotFrames);
		ASSERT(nFrame < CUnindexedSet__GetBlockCount(&usRotFrames));

		CUnindexedSet__Destruct(usRotSets);
		CUnindexedSet__Destruct(&usRotFrames);

		//return pRotFrames[nFrame].m_qRot;
		pFrame = &pRotFrames[nFrame];

		qRot.x = INT162FLOAT(pFrame->x, 1);
		qRot.y = INT162FLOAT(pFrame->y, 1);
		qRot.z = INT162FLOAT(pFrame->z, 1);
		qRot.t = INT162FLOAT(pFrame->t, 1);
	}

	return qRot;
}

void CGameObjectInstance__AdvanceAccumulation(CGameObjectInstance *pThis)
{
	int				frameC, lastFrameC,
						frameB, lastFrameB,
						nextFrameC, nextLastFrameC,
						nextFrameB, nextLastFrameB;
	CIndexedSet		isCurrentAnim, isBlendAnim;
	void				*pbCRotY, *pbBRotY;
	CUnindexedSet	usCRotY, usBRotY;
	CVector3			vCPos, vCLastPos,
						vCNextPos, vCNextLastPos,
						vBPos, vBLastPos,
						vBNextPos, vBNextLastPos,
						vBlendPos, vLastBlendPos,
						vNextBlendPos, vNextLastBlendPos,
						vDelta, vRotDelta;
	WORD				*pCRotYs, *pBRotYs;
	float				rotBlend, lastRotBlend,
						nextRotBlend, nextLastRotBlend,
						finalRotBlend,
						fraction,
						vA, vB;

#if USE_ORIGINAL_MATRIX
	CMtxF				mfRotate, mfRotY ;
#endif
	CMtxF				mfRotProd ;

	BOOL				blending;

	ASSERT(pThis->m_nAnims);

	blending = CGameObjectInstance__IsBlending(pThis);

	fraction = pThis->m_asCurrent.m_cFrame - ((int) pThis->m_asCurrent.m_cFrame);

	frameC = max(0, (int) pThis->m_asCurrent.m_cFrame);
	lastFrameC = max(0, frameC - 1);

	nextFrameC = (int) CGameAnimateState__GetNextFrame(&pThis->m_asCurrent);
	nextLastFrameC = max(0, nextFrameC - 1);

	if (blending)
	{
		frameB = max(0, (int) pThis->m_asBlend.m_cFrame);
		lastFrameB = max(0, frameB - 1);

		nextFrameB = (int) CGameAnimateState__GetNextFrame(&pThis->m_asBlend);
		nextLastFrameB = max(0, nextFrameB - 1);
	}

	ASSERT(pThis->m_asCurrent.m_pceAnim);
	CIndexedSet__ConstructFromRawData(&isCurrentAnim,
												 CCacheEntry__GetData(pThis->m_asCurrent.m_pceAnim),
												 FALSE);

	if (blending)
	{
		ASSERT(pThis->m_asBlend.m_pceAnim);
		CIndexedSet__ConstructFromRawData(&isBlendAnim,
													 CCacheEntry__GetData(pThis->m_asBlend.m_pceAnim),
													 FALSE);
	}


	// get translation data
	vCPos				= CGameObjectInstance__GetNodePos(&isCurrentAnim, 0, frameC);
	vCLastPos		= CGameObjectInstance__GetNodePos(&isCurrentAnim, 0, lastFrameC);
	vCNextPos		= CGameObjectInstance__GetNodePos(&isCurrentAnim, 0, nextFrameC);
	vCNextLastPos	= CGameObjectInstance__GetNodePos(&isCurrentAnim, 0, nextLastFrameC);

	if (blending)
	{
		vBPos				= CGameObjectInstance__GetNodePos(&isBlendAnim, 0, frameB);
		vBLastPos		= CGameObjectInstance__GetNodePos(&isBlendAnim, 0, lastFrameB);
		vBNextPos		= CGameObjectInstance__GetNodePos(&isBlendAnim, 0, nextFrameB);
		vBNextLastPos	= CGameObjectInstance__GetNodePos(&isBlendAnim, 0, nextLastFrameB);
	}

	// calculate translation
	if (blending)
	{
		CVector3__Blend(&vBlendPos, pThis->m_uBlender, &vBPos, &vCPos);
		CVector3__Blend(&vLastBlendPos, pThis->m_uBlender, &vBLastPos, &vCLastPos);

		CVector3__Blend(&vNextBlendPos, pThis->m_uBlender, &vBNextPos, &vCNextPos);
		CVector3__Blend(&vNextLastBlendPos, pThis->m_uBlender, &vBNextLastPos, &vCNextLastPos);
	}
	else
	{
		vBlendPos = vCPos;
		vLastBlendPos = vCLastPos;

		vNextBlendPos = vCNextPos;
		vNextLastBlendPos = vCNextLastPos;
	}


/*
	// collision offset based on root node movement (z axis up and down in 3ds space)
	pThis->ah.ih.m_CollisionHeightOffset = vBlendPos.z*pThis->m_vScale.y;

	if (pThis->m_AI.m_Health > 0)
		pThis->ah.ih.m_CollisionHeightOffset -= pThis->ah.ih.m_pEA->m_CollisionHeight/2;
	else
		pThis->ah.ih.m_CollisionHeightOffset -= pThis->ah.ih.m_pEA->m_DeadHeight/2;

	if ((pThis->ah.ih.m_CollisionHeightOffset < 0) && (CInstanceHdr__IsOnGround(&pThis->ah.ih)))
		pThis->ah.ih.m_CollisionHeightOffset = 0;
*/
	// collision offset based on root node movement (z axis up and down in 3ds space)
	pThis->ah.ih.m_CollisionHeightOffset = vBlendPos.z*pThis->m_vScale.y;
	pThis->ah.ih.m_CollisionHeightOffset -= pThis->ah.ih.m_pEA->m_CollisionHeight*(3.0/5.0);
	if (pThis->ah.ih.m_CollisionHeightOffset < 0)
		pThis->ah.ih.m_CollisionHeightOffset = 0;

	// get rotation data
	pbCRotY = CIndexedSet__GetBlock(&isCurrentAnim, CART_ANIM_usYRotations);
	CUnindexedSet__ConstructFromRawData(&usCRotY, pbCRotY, FALSE);

	pCRotYs = (WORD*) CUnindexedSet__GetBasePtr(&usCRotY);
	ASSERT(frameC < CUnindexedSet__GetBlockCount(&usCRotY));
	ASSERT(nextFrameC < CUnindexedSet__GetBlockCount(&usCRotY));

	if (blending)
	{
		pbBRotY = CIndexedSet__GetBlock(&isBlendAnim, CART_ANIM_usYRotations);
		CUnindexedSet__ConstructFromRawData(&usBRotY, pbBRotY, FALSE);

		pBRotYs = (WORD*) CUnindexedSet__GetBasePtr(&usBRotY);
		ASSERT(frameB < CUnindexedSet__GetBlockCount(&usBRotY));
		ASSERT(nextFrameB < CUnindexedSet__GetBlockCount(&usBRotY));
	}

	// calculate rotation
	if (blending)
	{
		rotBlend = BlendRotFLOAT(pThis->m_uBlender, INT162FLOAT(pBRotYs[frameB], ANGLE_PI), INT162FLOAT(pCRotYs[frameC], ANGLE_PI));
		lastRotBlend = BlendRotFLOAT(pThis->m_uBlender, INT162FLOAT(pBRotYs[lastFrameB], ANGLE_PI), INT162FLOAT(pCRotYs[lastFrameC], ANGLE_PI));

		nextRotBlend = BlendRotFLOAT(pThis->m_uBlender, INT162FLOAT(pBRotYs[nextFrameB], ANGLE_PI), INT162FLOAT(pCRotYs[nextFrameC], ANGLE_PI));
		nextLastRotBlend = BlendRotFLOAT(pThis->m_uBlender, INT162FLOAT(pBRotYs[nextLastFrameB], ANGLE_PI), INT162FLOAT(pCRotYs[nextLastFrameC], ANGLE_PI));
	}
	else
	{
		rotBlend = INT162FLOAT(pCRotYs[frameC], ANGLE_PI);
		lastRotBlend = INT162FLOAT(pCRotYs[lastFrameC], ANGLE_PI);

		nextRotBlend = INT162FLOAT(pCRotYs[nextFrameC], ANGLE_PI);
		nextLastRotBlend = INT162FLOAT(pCRotYs[nextLastFrameC], ANGLE_PI);
	}

	// blend values for accumulated interpolations between animation frames
	vA = max(0, frame_increment - fraction);
	vB = min(frame_increment, fraction);

	//TRACE4("vA:%f, vB:%f inc:%f, frac:%f\r\n", vA, vB, frame_increment, fraction);

	//finalRotBlend = (vA * (rotBlend - lastRotBlend)) + (vB * (nextRotBlend - nextLastRotBlend));
	finalRotBlend = vA*RotDifference(rotBlend, lastRotBlend) + vB*RotDifference(nextRotBlend, nextLastRotBlend);
	pThis->m_RotY -= finalRotBlend;
	NormalizeRotation(&pThis->m_RotY);

// DIDN'T WORK!
/*
	// prevent snapping
	if (blending)
	{
		if (CGameObjectInstance__IsFirstBlendFrame(pThis))
		{
			CGameObjectInstance__SetNotFirstBlendFrame(pThis);

			// decide to blend to right or to left
			if (RotDifference(INT162FLOAT(pBRotYs[frameB], ANGLE_PI), INT162FLOAT(pCRotYs[frameC], ANGLE_PI)) > 0)
				CGameObjectInstance__SetBlendingToRight(pThis);
			else
				CGameObjectInstance__SetNotBlendingToRight(pThis);
		}

		if ((RotDifference(pThis->m_RotY, INT162FLOAT(pCRotYs[frameC], ANGLE_PI)) > 0) ^ (CGameObjectInstance__IsBlendingToRight(pThis) != 0))
		{
			// do 180 flip if it's blending the wrong way
			if (pThis->m_RotY > 0)
				pThis->m_RotY -= ANGLE_PI;
			else
				pThis->m_RotY += ANGLE_PI;
		}
	}
*/

	// don't accumulate Y axis
	// use Y for Z to switch from 3D Studio coordinate system

	// TEMP
	if (		(AI_GetEA(pThis)->m_dwTypeFlags & AI_TYPE_SNIPER)
			&&	(pThis->m_AI.m_Health > 0) )
	{
		vDelta.x = 0;
		vDelta.y = 0;
		vDelta.z = 0;
	}
	else
	{
		vDelta.x = (vA * (vBlendPos.x - vLastBlendPos.x)) + (vB * (vNextBlendPos.x - vNextLastBlendPos.x));
		vDelta.y = 0;
		vDelta.z = (vA * (vBlendPos.y - vLastBlendPos.y)) + (vB * (vNextBlendPos.y - vNextLastBlendPos.y));
	}

	// rotate delta
#if USE_ORIGINAL_MATRIX
	CQuatern__ToMatrix(&pThis->m_qRot, mfRotate);
	CMtxF__RotateY(mfRotY, pThis->m_RotY) ;
	CMtxF__PostMult(mfRotProd, mfRotY, mfRotate);
#else
	// Use optimized matrix functions
	CQuatern__ToMatrix(&pThis->m_qRot, mfRotProd) ;
	CMtxF__PreMultRotateY(mfRotProd, pThis->m_RotY) ;
#endif

	CMtxF__VectorMult(mfRotProd, &vDelta, &vRotDelta);

	// make sure there is still no Y change
	pThis->ah.ih.m_vPos.x += vRotDelta.x * pThis->m_vScale.x;
	pThis->ah.ih.m_vPos.z += vRotDelta.z * pThis->m_vScale.z;


	CIndexedSet__Destruct(&isCurrentAnim);
	CUnindexedSet__Destruct(&usCRotY);
	if (blending)
	{
		CIndexedSet__Destruct(&isBlendAnim);
		CUnindexedSet__Destruct(&usBRotY);
	}
}

void CGameObjectInstance__DoAI(CGameObjectInstance *pThis)
{
	// don't update devices in pause mode...
	if (GetApp()->m_bPause)
	{
		// Bosses may have a pause function (to cut off looping sounds etc)
		if ((pThis->m_pBoss) && (pThis->m_pBoss->m_pPauseFunction))
			pThis->m_pBoss->m_pPauseFunction(pThis, pThis->m_pBoss) ;

		return ;
	}

#ifndef KANJI
	if ( 		(GetApp()->m_Mode == MODE_GAME)
			&&	(GetApp()->m_dwCheatFlags & CHEATFLAG_DISCO)
			&& (pThis->m_AI.m_Health) )
	{
		switch (CGameObjectInstance__TypeFlag(pThis))
		{
			case AI_OBJECT_CHARACTER_RAPTOR:
			case AI_OBJECT_CHARACTER_HUMAN1:
			case AI_OBJECT_CHARACTER_DIMETRODON:
			case AI_OBJECT_CHARACTER_TRICERATOPS:
			case AI_OBJECT_CHARACTER_MOSCHOPS:
			case AI_OBJECT_CHARACTER_LEAPER:
			case AI_OBJECT_CHARACTER_ALIEN:
			case AI_OBJECT_CHARACTER_HULK:
			case AI_OBJECT_CHARACTER_ROBOT:
			case AI_OBJECT_CHARACTER_ANCIENTWARRIOR:
			case AI_OBJECT_CHARACTER_HIGHPRIEST:
				CGameObjectInstance__DiscoAI(pThis);
				return;
		}
	}
#endif

	switch (CGameObjectInstance__TypeFlag(pThis))
	{
		case AI_OBJECT_DEVICE_DOOR:
			// no AI -- all handled by events
			// for now, use random for testing
			//if (CGameObjectInstance__IsCycleCompleted(pThis))
			//	CGameObjectInstance__SetDesiredAnim(pThis, RANDOM(pThis->m_nAnims));
			DoorAI_Advance(pThis);
			break;
		case AI_OBJECT_DEVICE_ACTION:
		case AI_OBJECT_DEVICE_ANIMOFFSCREENACTION:
			ActionAI_Advance(pThis);
			break;
		case AI_OBJECT_DEVICE_TIMERACTION:
			TimerActionAI_Advance(pThis);
			break;
		case AI_OBJECT_DEVICE_PRESSUREACTION:
			PressureActionAI_Advance(pThis);
			break;
		case AI_OBJECT_DEVICE_CONSTANTACTION:
			ConstantActionAI_Advance(pThis);
			break;
		case AI_OBJECT_DEVICE_STATUE:
			StatueAI_Advance(pThis);
			break;
		case AI_OBJECT_DEVICE_WALL:
			WallAI_Advance(pThis);
			break;
//		case AI_OBJECT_DEVICE_SPINDOOR:
//			SpinDoorAI_Advance(pThis);
//			break;
		case AI_OBJECT_DEVICE_LASER:
			LaserAI_Advance(pThis);
			break;
		case AI_OBJECT_DEVICE_EXPTARGET:
			ExplosiveTargetAI_Advance(pThis);
			break;
		case AI_OBJECT_DEVICE_ELEVATOR:
		case AI_OBJECT_DEVICE_FOOTELEVATOR:
			ElevatorAI_Advance(pThis);
			break;
		case AI_OBJECT_DEVICE_PLATFORM:
			PlatformAI_Advance(pThis);
			break;
		case AI_OBJECT_DEVICE_DEATHELEVATOR:
			DeathElevatorAI_Advance(pThis);
			break;
		case AI_OBJECT_DEVICE_COLLAPSINGPLATFORM:
			CollapsingPlatformAI_Advance(pThis);
			break;


// TREX ELEVATOR AND PLATFORMS NOW GONE!!
#if 0

		case AI_OBJECT_DEVICE_TREXELEVATOR:
			TrexElevatorAI_Advance(pThis);
			break;

		case AI_OBJECT_DEVICE_TREXPLATFORM:
			TrexPlatformAI_Advance(pThis);
			break;
#endif

#if 0
// gone now!!
		case AI_OBJECT_DEVICE_LONGHUNTER_SWITCH:
			LonghunterSwitchAI_Advance(pThis);
			break;
#endif

		case AI_OBJECT_DEVICE_PORTAL:
			PortalAI_Advance(pThis);
			break;
		case AI_OBJECT_DEVICE_GATE:
			GateAI_Advance(pThis);
			break;
		case AI_OBJECT_DEVICE_LOCK:
			LockAI_Advance(pThis);
			break;
		case AI_OBJECT_DEVICE_DRAIN:
			DrainAI_Advance(pThis);
			break;
		case AI_OBJECT_DEVICE_FLOOD:
			FloodAI_Advance(pThis);
			break;
		case AI_OBJECT_DEVICE_CRYSTAL:
			CrystalAI_Advance(pThis);
			break;
		case AI_OBJECT_DEVICE_SPINELEVATOR:
			SpinElevatorAI_Advance(pThis);
			break;
		case AI_OBJECT_DEVICE_SPINPLATFORM:
			SpinPlatformAI_Advance(pThis);
			break;
		case AI_OBJECT_DEVICE_KEYFLOOR:
			KeyFloorAI_Advance(pThis);
			break;

		case AI_OBJECT_DEVICE_FADEINFADEOUT:
			FadeInFadeOutAI_Advance(pThis) ;
			break ;

		case AI_OBJECT_CEILING_TURRET:
			CeilingTurretAI_Advance(pThis);
			break;
		case AI_OBJECT_BUNKER_TURRET:
			BunkerTurretAI_Advance(pThis);
			break;

		// the AI for the following is so simple that there isn't any!
		case AI_OBJECT_CINEMA_TUROK_ARROW:
		case AI_OBJECT_DEVICE_PICKUP_GERATOR:
			break ;

		default:

			// leave animating objects alone during intro and title!!
			if (		(GetApp()->m_Mode == MODE_INTRO)
					|| (GetApp()->m_Mode == MODE_TITLE)
					|| (GetApp()->m_Mode == MODE_LEGALSCREEN) )
			{
				return;
			}

#ifdef RANDOM_AI
			if (CGameObjectInstance__IsCycleCompleted(pThis))
				AI_SetDesiredAnimByIndex(pThis, RANDOM(pThis->m_nAnims), FALSE);
#else
			// set cycle completed flag for AI
			if (CGameObjectInstance__IsCycleCompleted(pThis))
				pThis->m_AI.m_dwStatusFlags |= AI_CYCLECOMPLETED;
			else
				pThis->m_AI.m_dwStatusFlags &= (~AI_CYCLECOMPLETED);

// TEMP S.B.
			if ((GetApp()->m_bPause) && (!pThis->m_pBoss))
//			if (1)
			{
				CGameObjectInstance__PauseAI(pThis);
			}
			else
			{
				if (pThis->m_pBoss)
					BOSS_Update(pThis);
				else if (GetApp()->m_Mode == MODE_GALLERY)
					AI_Advance_Gallery(pThis);
				else
					AI_Advance(pThis);
			}
#endif
			break;
	}

	// Update head tracking
	//AI_PerformHeadTracking(pThis) ;
}


BOOL CGameObjectInstance__IsFacingCliff(CGameObjectInstance *pThis)
{
	CVector3		vNormal;
	float			sinTheta, cosTheta,
					dot;
	
	sinTheta = sin(pThis->m_RotY);
	cosTheta = cos(pThis->m_RotY);

	vNormal = CGameRegion__GetGroundNormal(pThis->ah.ih.m_pCurrentRegion);
	
	dot = vNormal.x*(-sinTheta) + vNormal.z*(-cosTheta);
	
	return (dot < 0.0);
}				  

void CGameObjectInstance__PauseAI(CGameObjectInstance *pThis)
{
	int waterFlag, picked;
	int waterAnimTypes[]		= { AI_ANIM_IDLE_SWIM	};
	int waterAnimWeights[]	= { 1							};
	int landAnimTypes[]		= { AI_ANIM_IDLE_INTEL1,	AI_ANIM_IDLE_INTEL2, AI_ANIM_IDLE	};
	int landAnimWeights[]	= { 1,							1,							2					};

	if (CGameObjectInstance__IsCycleCompleted(pThis))
	{
		if ((!CGameObjectInstance__IsDevice(pThis)) && (pThis->m_AI.m_Health))
		{
			waterFlag = AI_GetWaterStatus(pThis);
			if (waterFlag == AI_NOT_NEAR_WATER)
				picked = CGameObjectInstance__GetRandomAnimIndex(pThis, 3, landAnimTypes, landAnimWeights, NULL);
			else
				picked = CGameObjectInstance__GetRandomAnimIndex(pThis, 1, waterAnimTypes, waterAnimWeights, NULL);

			AI_SetDesiredAnimByIndex(pThis, picked, FALSE);
//			AI_SetDesiredAnimByIndex(pThis, AI_ANIM_IDLE, FALSE);
		}
	}
}

void CGameObjectInstance__DiscoAI(CGameObjectInstance *pThis)
{
	int picked;
	int animTypes[] = { AI_ANIM_TURNL90, AI_ANIM_TURNR90, AI_ANIM_TURN180 };
	int animWeights[] = { 1, 1, 1 };

	picked = CGameObjectInstance__GetRandomAnimIndex(pThis, 3, animTypes, animWeights, NULL);
	AI_SetDesiredAnimByIndex(pThis, picked, FALSE);
}

void CGameObjectInstance__WentOutOfAnimBounds(CGameObjectInstance *pThis)
{
	CGameObjectInstance__SetNotVisible(pThis);

	// kill this ai ?
	if (AI_GetDyn(pThis)->m_dwStatusFlags2 & AI_KILLOUTSIDEOFVIEW)
		AI_SEvent_InstantDeath(&pThis->ah.ih);

	// make sure old animation doesn't get used
	pThis->m_asCurrent.m_pceAnim = NULL;
}

void CGameObjectInstance__SendAnimEvents(CGameObjectInstance *pThis,
													  CIndexedSet *pisAnim,
													  CMtxF mfOrient)
{
	void				*pbEvents;
	CUnindexedSet	usEvents;
	CROMEventEntry	*events, *pEvent;
	int				cEvent, nEvents;
	CVector3			vWorld;

	pbEvents = CIndexedSet__GetBlock(pisAnim, CART_ANIM_usEvents);
	CUnindexedSet__ConstructFromRawData(&usEvents, pbEvents, FALSE);

	events = (CROMEventEntry*) CUnindexedSet__GetBasePtr(&usEvents);
	nEvents = CUnindexedSet__GetBlockCount(&usEvents);

	for (cEvent=0; cEvent<nEvents; cEvent++)
	{
		pEvent = &events[cEvent];

		if (     (pThis->m_asCurrent.m_cFrame >= pEvent->m_nFrame)
				&&	(pThis->m_asCurrent.m_cFrame < (pEvent->m_nFrame + frame_increment)) )
		{
			ASSERT(pEvent->m_nEvent < AI_EVENT_TYPES);
			TRACE0("CGameObjectInstance: Animation event\r\n");

			// multiply offset position by orientation matrix
			// to find position in world coordinates
			switch (pThis->m_nTypeFlag)
			{
				case AI_OBJECT_BUNKER_TURRET:
					BunkerTurret_TransformPos(pThis, &pEvent->m_vOffset, &vWorld);
					break;

				case AI_OBJECT_CEILING_TURRET:
					CeilingTurret_TransformPos(pThis, &pEvent->m_vOffset, &vWorld);
					break;

				default:
					CMtxF__VectorMult(mfOrient, &pEvent->m_vOffset, &vWorld);
					break;
			}

//			rmonPrintf("xyz %f,%f,%f\r\n",
//						  pEvent->m_vOffset.x/SCALING_FACTOR,
//						  pEvent->m_vOffset.y/SCALING_FACTOR,
//						  pEvent->m_vOffset.z/SCALING_FACTOR) ;

			AI_Event_Dispatcher(&pThis->ah.ih, &pThis->ah.ih,
									  pEvent->m_nEvent, AI_SPECIES_ALL,
									  vWorld, pEvent->m_Value);
		}
	}
}

void CGameObjectInstance__SendAllDoorEvents(CGameObjectInstance *pThis, CIndexedSet *pisAnim)
{
	void				*pbEvents;
	CUnindexedSet	usEvents;
	CROMEventEntry	*events, *pEvent;
	int				cEvent, nEvents;
	CVector3			vWorld;
	CMtxF				mfOrient;

	pbEvents = CIndexedSet__GetBlock(pisAnim, CART_ANIM_usEvents);
	CUnindexedSet__ConstructFromRawData(&usEvents, pbEvents, FALSE);

	events = (CROMEventEntry*) CUnindexedSet__GetBasePtr(&usEvents);
	nEvents = CUnindexedSet__GetBlockCount(&usEvents);

	CMtxF__FromMtx(mfOrient, CGameObjectInstance__GetOrientationMatrix(pThis));

	for (cEvent=0; cEvent<nEvents; cEvent++)
	{
		pEvent = &events[cEvent];

		switch (pEvent->m_nEvent)
		{
			case AI_SEVENT_DOOR_ALLOWENTRY:
			case AI_SEVENT_DOOR_BLOCKENTRY:
	
				CMtxF__VectorMult(mfOrient, &pEvent->m_vOffset, &vWorld);
				AI_Event_Dispatcher(&pThis->ah.ih, &pThis->ah.ih,
										  pEvent->m_nEvent, AI_SPECIES_ALL,
										  vWorld, pEvent->m_Value);
				break;
		}
	}
}

void CGameObjectInstance__DrawShadow(CGameObjectInstance *pThis, Gfx **ppDLP, CVector3 *pvOffset)
{
#if USE_ORIGINAL_MATRIX
	CMtxF			mfPos, mfScale,
					mfTemp1, mfTemp2;
#endif
	CMtxF			mfRotate ;
	CVector3		vTranslate;

	float			raise, radius, groundHeight, height;
	Mtx			*pmShadow;
	BOOL			drawShadow, isPlayer;
	BYTE			shadowTransparency;
	int			duckFlag;

	isPlayer = (pThis == CEngineApp__GetPlayer(GetApp()));

	drawShadow = TRUE;
	if (CGameRegion__IsCliff(pThis->ah.ih.m_pCurrentRegion))
		drawShadow = FALSE;

	if (		(pThis->ah.ih.m_pCurrentRegion)
			&&	(pThis->ah.ih.m_pCurrentRegion->m_wFlags & REGFLAG_FALLDEATH) )
		drawShadow = FALSE;

	if (!(pThis->ah.ih.m_pEA->m_dwTypeFlags & AI_TYPE_SHADOW))
		drawShadow = FALSE;

	if ((isPlayer) && (GetApp()->m_Camera.m_CinemaMode))
		drawShadow = FALSE;


	if (drawShadow)
	{
		// Allocate mtx table
		pmShadow = CAnimationMtxs__RequestMtxTable(1) ;
		if (!pmShadow)
			return;

		//if (pThis->m_dwFlags & ANIM_INSTANCE_NEAREDGE)
		//	groundHeight = pThis->ah.ih.m_vPos.y;
		//else
		//groundHeight = CInstanceHdr__GetGroundHeight(&pThis->ah.ih);
		groundHeight = CGameRegion__GetGroundHeight(pThis->ah.ih.m_pCurrentRegion,
																  pThis->ah.ih.m_vPos.x + pvOffset->x,
																  pThis->ah.ih.m_vPos.z + pvOffset->z);
		//rmonPrintf("groundHeight:%f\n", groundHeight);


		height = pThis->ah.ih.m_vPos.y + pThis->ah.ih.m_CollisionHeightOffset - groundHeight;
		radius = max(0, min(pThis->ah.ih.m_pEA->m_CollisionRadius, pThis->ah.ih.m_pEA->m_CollisionRadius - height*0.1));

		// raise shadow a bit above ground to avoid problems
		//if (pThis->m_dwFlags & ANIM_INSTANCE_NEAREDGE)
		//{
		//	raise = 0;
		//
		//	// TEMP
		//	raise += 0.5;
		//}
		//else
		//{
		if (isPlayer)
		{
			raise = 1.5*SCALING_FACTOR;
			radius *= 0.5;
		}
		else
		{
			raise = 0;
			radius *= 0.7;
		}
		//}

		vTranslate.x = pThis->ah.ih.m_vPos.x + pvOffset->x;
		vTranslate.z = pThis->ah.ih.m_vPos.z + pvOffset->z;
		vTranslate.y = groundHeight + raise + + pvOffset->y;
		CVector3__Clamp(&vTranslate, WORLD_LIMIT);

#if USE_ORIGINAL_MATRIX
		CQuatern__ToMatrix(&pThis->m_qShadow, mfRotate);
		CMtxF__Scale(mfScale, radius, 1, radius);
		CMtxF__Translate(mfPos, vTranslate.x, vTranslate.y, vTranslate.z);
		CMtxF__PostMult(mfTemp1, mfScale, mfRotate);
		CMtxF__PostMult(mfTemp2, mfTemp1, mfPos);
		CMtxF__ToMtx(mfTemp2, *pmShadow);

#else
		// Optimized matrix version
		CQuatern__ToMatrix(&pThis->m_qShadow, mfRotate);
		CMtxF__PreMultScale(mfRotate, radius, 1, radius);
		CMtxF__PostMultTranslate(mfRotate, vTranslate.x, vTranslate.y, vTranslate.z);
		CMtxF__ToMtx(mfRotate, *pmShadow);
#endif

		// calc shadow transparency value
		if (pThis->m_AI.m_cRegenerateAppearance)
			shadowTransparency = (BYTE) max(0, min(255, (1 - pThis->m_AI.m_cRegenerateAppearance/APPEARANCE_LENGTH)*(200 - height*(isPlayer ? (9/SCALING_FACTOR) : (12.5/SCALING_FACTOR)))));
		else
			//shadowTransparency = (BYTE) max(0, min(255, (1 - pThis->m_cMelt/MELT_LENGTH)*(200 - height*(isPlayer ? (9/SCALING_FACTOR) : (12.5/SCALING_FACTOR)))));
			shadowTransparency = (BYTE) max(0, min(255, (1 - pThis->m_cMelt/MELT_LENGTH)*(200 - height*(12.5/SCALING_FACTOR))));


		// if player is in ducking region remove make the shadow completely transparent
		duckFlag = CEngineApp__GetPlayerDuckStatus(GetApp());
		if (    (duckFlag == PLAYER_DUCKING)
			  || (duckFlag == PLAYER_DUCKING_ENTRANCE_EXIT) )
		{
			pThis->m_ShadowPercentage = 0;
		}
		else
		{
			pThis->m_ShadowPercentage += frame_increment*10;
			pThis->m_ShadowPercentage  = min(100.0, pThis->m_ShadowPercentage);
		}

		shadowTransparency = (BYTE)(((float)shadowTransparency) * pThis->m_ShadowPercentage / 100.0);

		CGeometry__DrawShadow(ppDLP, pmShadow, !isPlayer, shadowTransparency);
	}
}


void CGameObjectInstance__PreDrawPlayer(CGameObjectInstance *pThis, Gfx **ppDLP)
{
	CIndexedSet		isCurrentAnim;
	CVector3			vShadowOffset;
	float				PrevFrameIncrement ;

	// Store frame increment
	PrevFrameIncrement = frame_increment ;

	// only keep player's object info and anims index in cache
	if (pThis->m_pceObjectInfo)
		CCacheEntry__ResetAge(pThis->m_pceObjectInfo);
	if (pThis->m_pceAnimsIndex)
		CCacheEntry__ResetAge(pThis->m_pceAnimsIndex);

	// need object info to figure out bounds and anims index to advance
	if (pThis->m_pceObjectInfo && pThis->m_pceAnimsIndex && pThis->m_asCurrent.m_pceAnim)
	{
		CCacheEntry__ResetAge(pThis->m_asCurrent.m_pceAnim);

		CGameObjectInstance__DoAI(pThis);

		if (pThis->m_asCurrent.m_pceAnim)
		{
			//ASSERT(pThis->m_asCurrent.m_pceAnim);

			// This function in "cinecam.cpp"
			frame_increment *= CGameObjectInstance__AnimSpeedScaler(pThis) ;
			CGameObjectInstance__Advance(pThis);

			// Don't follow turok if in cinema mode, else do!
			if (GetApp()->m_Camera.m_CinemaMode)
				CGameObjectInstance__CalculateOrientationMatrix(pThis, FALSE, vt_player_corners, mf_player_orient);
			else
				CGameObjectInstance__CalculateOrientationMatrix(pThis, TRUE, vt_player_corners, mf_player_orient);

			// draw shadow before z-buffer is cleared
			vShadowOffset.x = vShadowOffset.y = vShadowOffset.z = 0;
			CGameObjectInstance__DrawShadow(pThis, ppDLP, &vShadowOffset);


 			CIndexedSet__ConstructFromRawData(&isCurrentAnim,
														 CCacheEntry__GetData(pThis->m_asCurrent.m_pceAnim),
														 FALSE);

			CGameObjectInstance__SendAnimEvents(pThis, &isCurrentAnim, mf_player_orient);

			CIndexedSet__Destruct(&isCurrentAnim);
		}
	}

	// Restore frame increment
	frame_increment = PrevFrameIncrement ;
}

#endif //!WIN32

// CROMGeometryInfo
/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
void CROMGeometryInfo__TakeFromGraphicObject(CROMGeometryInfo *pThis, CGraphicObject* /* pSource */)
{
	pThis->unused = 0;
}
#endif

// CGameGeometryInfo
/////////////////////////////////////////////////////////////////////////////

void CGameGeometryInfo__TakeFromROMGeometryInfo(CGameGeometryInfo *pThis, CROMGeometryInfo *pSource)
{
	pThis->m_MorphFrameNumber = (DWORD) -1;
	pThis->m_cMorph = 0;
}

// CROMInitialOrientation
/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
void CROMInitialOrientation__TakeFromNode(CROMInitialOrientation *pThis,
													   CNode *pNode, int nAnim, CVector3 vTranslationScaler,
														float RemoveYRotValue)
{
	ASSERT_VALID(pNode);

   CVector3 vInitialTranslation = pNode->GetInitialTranslation(nAnim);
   CVector3 vScaledTranslation(vInitialTranslation.x * vTranslationScaler.x,
                               vInitialTranslation.y * vTranslationScaler.y,
                               vInitialTranslation.z * vTranslationScaler.z);

	pThis->m_vPos = ORDERBYTES(vScaledTranslation);


	CQuaternion qRot = pNode->GetInitialRotation(nAnim);

	// remove y rotation for root node
	// using Z axis becaus rot is in 3D Studio coord system
	qRot *= CQuaternion(0, 0, -RemoveYRotValue);

   //qRot = ORDERBYTES(qRot);

   float x, y, z, t;
   qRot.GetValues(&x, &y, &z, &t);

   //pThis->m_qRot.x = x;
   //pThis->m_qRot.y = y;
   //pThis->m_qRot.z = z;
   //pThis->m_qRot.t = t;

	//pThis->m_PosX = ORDERBYTES(FLOAT2SF(vScaledTranslation.x));
	//pThis->m_PosY = ORDERBYTES(FLOAT2SF(vScaledTranslation.y));
	//pThis->m_PosZ = ORDERBYTES(FLOAT2SF(vScaledTranslation.z));
	//pThis->m_unused = ORDERBYTES((SF) 0);

	pThis->m_RotX = ORDERBYTES(FLOAT2INT16(max(-1, min(1, x)), 1));
	pThis->m_RotY = ORDERBYTES(FLOAT2INT16(max(-1, min(1, y)), 1));
	pThis->m_RotZ = ORDERBYTES(FLOAT2INT16(max(-1, min(1, z)), 1));
	pThis->m_RotT = ORDERBYTES(FLOAT2INT16(max(-1, min(1, t)), 1));
}
#endif

// CROMNodeIndex
/////////////////////////////////////////////////////////////////////////////

void CROMNodeIndex__TakeFromShorts(CROMNodeIndex *pThis,
											  short nTranslationSet, short nRotationSet)
{
	pThis->m_nTranslationSet	= ORDERBYTES(nTranslationSet);
	pThis->m_nRotationSet		= ORDERBYTES(nRotationSet);
}

// CRotFrame
/////////////////////////////////////////////////////////////////////////////

/*
#ifdef WIN32
void CRotFrame__TakeFromQuaternion(CRotFrame *pThis, CQuaternion *pSource)
{
   CQuaternion qRot = ORDERBYTES(*pSource);

   float x, y, z, t;
   qRot.GetValues(&x, &y, &z, &t);

   pThis->m_qRot.x = x;
   pThis->m_qRot.y = y;
   pThis->m_qRot.z = z;
   pThis->m_qRot.t = t;
}
#endif
*/

// CTranslationOffset
/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
void CTranslationOffset__TakeFromOffsetScale(CTranslationOffset *pThis, CVector3 vOffset, float Scale, BOOL Compressed)
{
	pThis->m_Compressed	= ORDERBYTES((DWORD) Compressed);
	pThis->m_vOffset		= ORDERBYTES(vOffset);
	pThis->m_Scale			= ORDERBYTES(Scale);
}
#endif

// CPosFrame
/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
void CPosFrame__TakeFromVector3(CPosFrame *pThis, CVector3 *pSource)
{
   pThis->m_vPos = ORDERBYTES(*pSource);
}
#endif

// CROMSection
/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32

void CROMSection__TakeFromMaterial(CROMSection *pThis, CMaterial *pSource, BOOL PreLit, BOOL ShadeAlpha,
											  WORD MultU, WORD MultV)
{
	DWORD dwAddFlags = 0;

	if (pSource)
		if (pSource->m_dwFlags & MATERIAL_PSEUDOCOLOR)
			dwAddFlags |= MATERIAL_TRANSPARENCY;

	if (ShadeAlpha)
		dwAddFlags |= MATERIAL_SHADE_ALPHA | MATERIAL_TRANSPARENCY;

	if (PreLit)
	{
		dwAddFlags |= MATERIAL_PRELIT;

		//if (pSource)
		//{
		//	if (pSource->m_dwFlags & MATERIAL_REFLECT_MAP)
		//		dwAddFlags |= MATERIAL_SELF_ILLUMINATED;
		//}
	}

	// two sided facets have been duplicated
	DWORD dwRemoveFlags = 0;
#ifdef PLANT_FIX
	dwRemoveFlags |= MATERIAL_TWO_SIDED;
#endif

	pThis->m_MultU = ORDERBYTES(MultU);
	pThis->m_MultV = ORDERBYTES(MultV);

	if (pSource)
	{
		pThis->m_dwMatFlags = ORDERBYTES((pSource->m_dwFlags & ~dwRemoveFlags) | dwAddFlags);
		pThis->m_nTextureSet = ORDERBYTES((DWORD) pSource->m_nTexture);

		pThis->m_Color[0] = pSource->m_Color[0];
		pThis->m_Color[1] = pSource->m_Color[1];
		pThis->m_Color[2] = pSource->m_Color[2];
		pThis->m_Color[3] = pSource->m_Color[3];

		pThis->m_BlackColor[0] = pSource->m_BlackColor[0];
		pThis->m_BlackColor[1] = pSource->m_BlackColor[1];
		pThis->m_BlackColor[2] = pSource->m_BlackColor[2];
		pThis->m_BlackColor[3] = pSource->m_BlackColor[3];
	}
	else
	{
		pThis->m_dwMatFlags = ORDERBYTES(dwAddFlags);
		pThis->m_nTextureSet = ORDERBYTES((DWORD) -1);

		pThis->m_Color[0] = 255;
		pThis->m_Color[1] = 255;
		pThis->m_Color[2] = 255;
		pThis->m_Color[3] = 255;

		pThis->m_BlackColor[0] = 255;
		pThis->m_BlackColor[1] = 255;
		pThis->m_BlackColor[2] = 255;
		pThis->m_BlackColor[3] = 255;
	}
}

#endif

// CGameSection
/////////////////////////////////////////////////////////////////////////////

void CGameSection__TakeFromRomSection(CGameSection *pThis, CROMSection *pSource,
												  CCacheEntry *pceTextureSetsIndex)
{
	CIndexedSet isTextureSets;
	ROMAddress	*rpTextureSet;
	DWORD			textureSetSize;

	pThis->m_dwMatFlags = ORDERBYTES(pSource->m_dwMatFlags);	/* PORT: big-endian (no-op on N64) */

	pThis->m_Color[0] = pSource->m_Color[0];
	pThis->m_Color[1] = pSource->m_Color[1];
	pThis->m_Color[2] = pSource->m_Color[2];
	pThis->m_Color[3] = pSource->m_Color[3];

	pThis->m_BlackColor[0] = pSource->m_BlackColor[0];
	pThis->m_BlackColor[1] = pSource->m_BlackColor[1];
	pThis->m_BlackColor[2] = pSource->m_BlackColor[2];
	pThis->m_BlackColor[3] = pSource->m_BlackColor[3];

	pThis->m_MultU = ORDERBYTES(pSource->m_MultU);	/* PORT: big-endian WORD */
	pThis->m_MultV = ORDERBYTES(pSource->m_MultV);

	// calculate address of object index
	CIndexedSet__ConstructFromRawData(&isTextureSets,
												 CCacheEntry__GetData(pceTextureSetsIndex),
												 FALSE);

#ifndef WIN32
	{
	DWORD nTextureSet = ORDERBYTES(pSource->m_nTextureSet);	/* PORT: big-endian DWORD index */
	if (nTextureSet == (DWORD) -1)
	{
		pThis->m_TextureLoader.m_rpTextureSet = NULL;
	}
	else
	{
		rpTextureSet = CIndexedSet__GetROMAddressAndSize(&isTextureSets,
																		 CCacheEntry__GetRequestedAddress(pceTextureSetsIndex),
																		 nTextureSet, &textureSetSize);

		CTextureLoader__ConstructFromAddressSize(&pThis->m_TextureLoader, rpTextureSet, textureSetSize);
	}
	}	/* PORT: close nTextureSet local scope */
#endif

	CIndexedSet__Destruct(&isTextureSets);
}

// CROMTextureFormat
/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32

void CROMTextureFormat__TakeFromTextureSet(CROMTextureFormat *pThis, CTextureSet *pSource)
{
	ASSERT(pSource->GetTextureCount());

	CTexture *pFirstTexture = pSource->GetTexture(0);
	ASSERT_VALID(pFirstTexture);

	if (pSource->IsSharpen())
		pThis->m_EffectMode = TEXTURE_EFFECT_SHARPEN;
	else
	{
		switch (pSource->GetDetail())
		{
			case TEXTURE_DETAIL_OFF:
				pThis->m_EffectMode = TEXTURE_EFFECT_NONE;
				break;

			case TEXTURE_DETAIL_X2:
				pThis->m_EffectMode = TEXTURE_EFFECT_DETAIL_X2;
				break;

			case TEXTURE_DETAIL_X4:
				pThis->m_EffectMode = TEXTURE_EFFECT_DETAIL_X4;
				break;

			default:
				ASSERT(FALSE);
				break;
		}
	}

	float f = pSource->GetEffect();
	pThis->m_Effect = max(0, min(255, ceil((1 - f*f)*255)));

	if (pSource->IsMipMap())
	{
		pThis->m_Format = TEXTURE_FORMAT_MM16RGBA;
	}
	else if (pSource->IsIA() && (pFirstTexture->m_BitsPerPixel == 4))
	{
		pThis->m_Format = TEXTURE_FORMAT_CI4_IA;
	}
	else if (pFirstTexture->m_BitsPerPixel == 8)
	{
		pThis->m_Format = TEXTURE_FORMAT_CI8_RGBA;
	}
	else if (pFirstTexture->m_BitsPerPixel == 4)
	{
		pThis->m_Format = TEXTURE_FORMAT_CI4_RGBA;
	}
	else
	{
		ASSERT(FALSE);
		return;
	}

	WORD playbackSpeed = 1;
	int pos = pFirstTexture->m_Name.ReverseFind('.');
	if (pos > 0)
	{
		switch (pFirstTexture->m_Name[pos - 1])
		{
			case '2':	playbackSpeed = 2;	break;
			case '3':	playbackSpeed = 3;	break;
			case '4':	playbackSpeed = 4;	break;
			case '5':	playbackSpeed = 5;	break;
			case '6':	playbackSpeed = 6;	break;
			case '7':	playbackSpeed = 7;	break;
			case '8':	playbackSpeed = 8;	break;
			case '9':	playbackSpeed = 9;	break;
		}
	}
	pThis->m_PlaybackSpeed = playbackSpeed;

	if (pSource->IsMipMap())
	{
		pThis->m_WidthShift = 5;
		pThis->m_HeightShift = 5;
	}
	else
	{
		switch (pFirstTexture->m_Width)
		{
			case 1:		pThis->m_WidthShift = 0;	break;
			case 2:		pThis->m_WidthShift = 1;	break;
			case 4:		pThis->m_WidthShift = 2;	break;
			case 8:		pThis->m_WidthShift = 3;	break;
			case 16:		pThis->m_WidthShift = 4;	break;
			case 32:		pThis->m_WidthShift = 5;	break;
			case 64:		pThis->m_WidthShift = 6;	break;
			case 128:	pThis->m_WidthShift = 7;	break;
			case 256:	pThis->m_WidthShift = 8;	break;
			case 1024:	pThis->m_WidthShift = 9;	break;
			case 2048:	pThis->m_WidthShift = 10;	break;
			default:		pThis->m_WidthShift = 0;	ASSERT(FALSE);
		}

		switch (pFirstTexture->m_Height)
		{
			case 1:		pThis->m_HeightShift = 0;	break;
			case 2:		pThis->m_HeightShift = 1;	break;
			case 4:		pThis->m_HeightShift = 2;	break;
			case 8:		pThis->m_HeightShift = 3;	break;
			case 16:		pThis->m_HeightShift = 4;	break;
			case 32:		pThis->m_HeightShift = 5;	break;
			case 64:		pThis->m_HeightShift = 6;	break;
			case 128:	pThis->m_HeightShift = 7;	break;
			case 256:	pThis->m_HeightShift = 8;	break;
			case 1024:	pThis->m_HeightShift = 9;	break;
			case 2048:	pThis->m_HeightShift = 10;	break;
			default:		pThis->m_HeightShift = 0;	ASSERT(FALSE);
		}
	}
}

#endif

// CROMLevel
/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
void CROMLevel__TakeFromLevel(CROMLevel *pThis, CLevel *pSource, float GridDistance)
{
	pThis->m_GridDistance = ORDERBYTES(GridDistance * SCALING_FACTOR);

	COLORREF blackColor, whiteColor;
	pSource->GetSkyColors(&blackColor, &whiteColor);

	pThis->m_BlackColor[0] = GetRValue(blackColor);
	pThis->m_BlackColor[1] = GetGValue(blackColor);
	pThis->m_BlackColor[2] = GetBValue(blackColor);

	pThis->m_WhiteColor[0] = GetRValue(whiteColor);
	pThis->m_WhiteColor[1] = GetGValue(whiteColor);
	pThis->m_WhiteColor[2] = GetBValue(whiteColor);

	pThis->m_DirectionalLight[0] = GetRValue(pSource->m_DirectionalLight);
	pThis->m_DirectionalLight[1] = GetGValue(pSource->m_DirectionalLight);
	pThis->m_DirectionalLight[2] = GetBValue(pSource->m_DirectionalLight);

	pThis->m_AmbientLight[0] = GetRValue(pSource->m_AmbientLight);
	pThis->m_AmbientLight[1] = GetGValue(pSource->m_AmbientLight);
	pThis->m_AmbientLight[2] = GetBValue(pSource->m_AmbientLight);

	pThis->m_bFlags = (BYTE) pSource->m_dwFlags;

	CVector3 vDir = pSource->m_vLight;

//#define LIGHT_MAGNITUDE	60
#define LIGHT_MAGNITUDE	120
	float mag = vDir.Magnitude();
	if (mag != 0)
		vDir *= LIGHT_MAGNITUDE/mag;

	pThis->m_Direction[0] = (signed char) -vDir.x;
	pThis->m_Direction[1] = (signed char) -vDir.y;
	pThis->m_Direction[2] = (signed char) -vDir.z;
}
#endif

/////////////////////////////////////////////////////////////////////////////

#ifndef WIN32

/*
BOOL CGameRegion__IsLedge(CGameRegion *pThis)
{
	CVector3		vNormal, vUp;
	float			dot;

	if (!pThis)
		return FALSE;

	vNormal = CGameRegion__GetGroundUnitNormal(pThis);

	vUp.x = 0;
	vUp.y = 1;
	vUp.z = 0;

	dot = CVector3__Dot(&vNormal, &vUp);
	return (dot <= ANGLE_LEDGE_COSINE);
}
*/

// does no floor or ceiling testing
BOOL CGameRegion__InRegion(CGameRegion *pThis, float X, float Z)
{
	int			e;
	CROMCorner	*pCornerA, *pCornerB;
	float			vNX, vNZ,
					vPX, vPZ;

	if (!pThis)
		return TRUE;

	for (e=0; e<3; e++)
	{
		pCornerA = pThis->m_pCorners[e];
		pCornerB = pThis->m_pCorners[(e + 1)%3];

		vNX = pCornerB->m_vCorner.z - pCornerA->m_vCorner.z;
		vNZ = pCornerA->m_vCorner.x - pCornerB->m_vCorner.x;

		vPX = X - pCornerA->m_vCorner.x;
		vPZ = Z - pCornerA->m_vCorner.z;

		if ((vNX*vPX + vNZ*vPZ) < 0)
			return FALSE;
	}

	return TRUE;
}

CGameRegion* CGameRegion__FindNextLowerRegion(CGameRegion *pThis, CVector3 vPos)
{
	CGameRegion			*pDesired;
	float					distance, closestDistance,
							groundHeight;
	CIndexedSet			isCollision;
	CUnindexedSet		usRegions,
							usBlocks;
	void					*pbRegions,
							*pbBlocks;
	CROMRegionBlock	*blocks, *pBlock;
	CGameRegion			*regions, *pRegion;
	int					cBlock, nBlocks,
							i, cRegion;

	if (!cache_is_valid)
		return NULL;

	CIndexedSet__ConstructFromRawData(&isCollision,
												 CCacheEntry__GetData(GetApp()->m_Scene.m_pceCollision),
												 FALSE);


	pbRegions = CIndexedSet__GetBlock(&isCollision, CART_COLLISION_usRegions);
	CUnindexedSet__ConstructFromRawData(&usRegions, pbRegions, FALSE);

	regions = (CGameRegion*) CUnindexedSet__GetBasePtr(&usRegions);

	pbBlocks =  CIndexedSet__GetBlock(&isCollision, CART_COLLISION_usBlockBounds);
	CUnindexedSet__ConstructFromRawData(&usBlocks, pbBlocks, FALSE);

	blocks = (CROMRegionBlock*) CUnindexedSet__GetBasePtr(&usBlocks);
	nBlocks = CUnindexedSet__GetBlockCount(&usBlocks);

	pDesired = NULL;
	closestDistance = 0;	// supresses warning

	// search through blocks
	cRegion = 0;
	for (cBlock=0; cBlock<nBlocks; cBlock++)
	{
		pBlock = &blocks[cBlock];

		if (CBoundsRect__InRect(&pBlock->m_BoundsRect, &vPos))
		{
			for (i=0; i<pBlock->m_nRegions; i++)
			{
				// get a pointer to current region
				pRegion = &regions[cRegion];

				if (CGameRegion__InRegion(pRegion, vPos.x, vPos.z) && (pRegion != pThis))
				{
					groundHeight = CGameRegion__GetGroundHeight(pRegion, vPos.x, vPos.z);
					if (groundHeight < vPos.y)
					{
						distance = vPos.y - groundHeight;
						if (pDesired)
						{
							if (distance < closestDistance)
							{
								pDesired = pRegion;
								closestDistance = distance;
							}
						}
						else
						{
							pDesired = pRegion;
							closestDistance = distance;
						}
					}
				}

				cRegion++;
			}
		}
		else
		{
			cRegion += pBlock->m_nRegions;
		}
	}

	CUnindexedSet__Destruct(&usRegions);
	CUnindexedSet__Destruct(&usBlocks);
	CIndexedSet__Destruct(&isCollision);

	return pDesired;
}

/*
CGameRegion* CGameRegion__CanEnter(CGameRegion *pCurrent, CVector3 *pvCurrent, float CompareHeight, float RotY,
											  int nDesiredEdge, CVector3 *pvDesired,
											  BOOL MoveUpLedges, BOOL MoveDownLedges, BOOL EnterWater, BOOL ExitWater,
											  struct CCollisionInfo_t *pCI)
{
	CGameRegion		*pDesired;
	CROMRegionSet	*pDesiredRegionSet,
						*pCurrentRegionSet;

//						*regions, *pRegion;
	float				currentHeight,
						desiredHeight,
//						groundHeight,
//						distance, closestDistance,
						edgeAngle, dt;
	int				cEdge;
//						cRegion, nRegions;
	BOOL				found;
//	CIndexedSet		isCollision;
//	CUnindexedSet	usRegions;
//	void				*pbRegions;
	CVector3			vNextLower;

	ASSERT(pCurrent);

	pDesired = pCurrent->m_pNeighbors[nDesiredEdge];
	if (!pDesired)
		return NULL;

	if (pCurrent->m_wFlags & REGFLAG_BRIDGE)
	{
		// if pDesired is linked back to pCurrent, this is a normal
		// transition -- otherwise, the instance went off the edge of
		// a bridge

		found = FALSE;
		for (cEdge=0; cEdge<3; cEdge++)
		{
			if (pDesired->m_pNeighbors[cEdge] == pCurrent)
			{
				found = TRUE;
				break;
			}
		}

		if (!found)
		{
			// this is a bridge transition
			// first, see if pvCurrent is within pDesired

			if (!CGameRegion__InRegion(pDesired, pvCurrent->x, pvCurrent->z))
			{
				// missed with guess
				// search all regions

				vNextLower.x = pvCurrent->x;
				vNextLower.y = CompareHeight;
				vNextLower.z = pvCurrent->z;

				pDesired = CGameRegion__FindNextLowerRegion(pCurrent, vNextLower);
				if (!pDesired)
					return NULL;
			}
		}
	}

	// WHY SPECIAL CASE THIS -- WHAT'S WRONG WITH RESTRICTED AREAS?
	// Used to keep TREX out of big life area
	if ((pCI->m_dwFlags & COLLISIONFLAG_AVOIDPRESSUREPLATE) &&
		 ((pDesired->m_wFlags & REGFLAG_PRESSUREPLATE) ||
		  (pDesired->m_wFlags & REGFLAG_PRESSUREPLATENEEDSCOLLISION)))
		return NULL;


	// keep bad guys out of trouble
	if (		(pDesired->m_wFlags & REGFLAG_RESTRICTEDAREA)
			&& !(pCI->m_dwFlags & COLLISIONFLAG_CANENTERRESTRICTEDAREA) )
	{
		return NULL;
	}

	// need to know if this is shallow water
	pDesiredRegionSet = CScene__GetRegionAttributes(&GetApp()->m_Scene, pDesired);
	pCurrentRegionSet = CScene__GetRegionAttributes(&GetApp()->m_Scene, pCurrent);

	// allowed to exit water ?
	if (		!ExitWater

			&& !(		(pDesired->m_wFlags & REGFLAG_HASWATER)
					&&	(!(pDesiredRegionSet->m_dwFlags & REGFLAG_SHALLOWWATER)) )

			&& (		(pCurrent->m_wFlags & REGFLAG_HASWATER)
					&&	(!(pCurrentRegionSet->m_dwFlags & REGFLAG_SHALLOWWATER)) ) )
	{
		return NULL;
	}

	// avoid water
	if (		!EnterWater

			&& (		(pDesired->m_wFlags & REGFLAG_HASWATER)
					&&	(!(pDesiredRegionSet->m_dwFlags & REGFLAG_SHALLOWWATER)) )

			&& !(		(pCurrent->m_wFlags & REGFLAG_HASWATER)
					&&	(!(pCurrentRegionSet->m_dwFlags & REGFLAG_SHALLOWWATER)) ) )
	{
		return NULL;
	}

	// need to be ducking to enter ducking area
	if (		(pDesired->m_wFlags & REGFLAG_DUCK)
			&&	!(pCI->m_dwFlags & COLLISIONFLAG_DUCKING) )
	{
		return NULL;
	}

	// can't enter closed door
//	if (		!(pCI->m_dwFlags & COLLISIONFLAG_MOVETHROUGHDOORS)
//			&&	(pDesired->m_wFlags & REGFLAG_DOOR) && !(pCurrent->m_wFlags & REGFLAG_DOOR)
//			&& !(pDesired->m_wFlags & REGFLAG_OPENDOOR) )
//	{
//		return NULL;
//	}

	// S.B. 11/30/96 - fixes walk through door on mantis level
	// Can't enter closed door
	if (		!(pCI->m_dwFlags & COLLISIONFLAG_MOVETHROUGHDOORS)
			&&	(pDesired->m_wFlags & REGFLAG_DOOR)
			&& !(pDesired->m_wFlags & REGFLAG_OPENDOOR) )
	{
		return NULL;
	}


	// bump in to wall above ceiling
	if ((pDesired->m_wFlags & REGFLAG_CEILING) && !(pCurrent->m_wFlags & REGFLAG_CEILING))
		if (pvCurrent->y > CGameRegion__GetCeilingHeight(pDesired, pvCurrent->x, pvCurrent->z))
			return NULL;

	if (pCI->m_dwFlags & COLLISIONFLAG_CLIFFONLY)
	{
		if (CGameRegion__IsCliff(pDesired))
			return pDesired;
		else
			return NULL;
	}

	// stay on climbable portion of cliff
	if (		(pCI->m_dwFlags & COLLISIONFLAG_CANCLIMBCLIMBABLE)
			&& CGameRegion__IsCliff(pCurrent)
			&& CGameRegion__IsCliff(pDesired)
			&& (pCurrent->m_wFlags & REGFLAG_CLIMBABLE)
			&& !(pDesired->m_wFlags & REGFLAG_CLIMBABLE) )
	{
		currentHeight = CGameRegion__GetGroundHeight(pCurrent, pvCurrent->x, pvCurrent->z);
		if (CompareHeight < (currentHeight + 0.1*SCALING_FACTOR))
			return NULL;
	}

	if (CGameRegion__IsCliff(pDesired))
	{
		if ((pDesired->m_wFlags & REGFLAG_CLIMBABLE) && (pCI->m_dwFlags & COLLISIONFLAG_CANCLIMBCLIMBABLE))
		{
			// test angle
			edgeAngle = CGameRegion__GetEdgeAngle(pCurrent, nDesiredEdge);
			edgeAngle += ANGLE_PI;
			NormalizeRotation(&edgeAngle);

			dt = RotDifference(edgeAngle, RotY);

#define ALLOW_CLIMB_ANGLE	ANGLE_DTOR(40)

			if (fabs(dt) <= ALLOW_CLIMB_ANGLE)
				MoveUpLedges = TRUE;
		}

		if (MoveUpLedges && MoveDownLedges)
			return pDesired;

		currentHeight = CGameRegion__GetGroundHeight(pCurrent, pvCurrent->x, pvCurrent->z);
		desiredHeight = CGameRegion__GetGroundHeight(pDesired, pvDesired->x, pvDesired->z);

		if (desiredHeight > currentHeight)
		{
			if (MoveUpLedges)
			{
				return pDesired;
			}
			else
			{
				CompareHeight += 0.1*SCALING_FACTOR;

				return (		(CompareHeight > pDesired->m_pCorners[0]->m_vCorner.y)
							&& (CompareHeight > pDesired->m_pCorners[1]->m_vCorner.y)
							&& (CompareHeight > pDesired->m_pCorners[2]->m_vCorner.y)) ? pDesired : NULL;
			}
		}
		else
		{
			return MoveDownLedges ? pDesired : NULL;
		}
	}
	else
	{
		return pDesired;
	}
}

void CCollisionInfo__SetPlayerDefaults(CCollisionInfo *pThis)
{
	pThis->m_WallBehavior			= WALL_SLIDE;
	pThis->m_InstanceBehavior		= INSTANCE_SLIDE;
	pThis->m_GravityAcceleration	= GRAVITY_ACCELERATION;
	pThis->m_BounceReturnEnergy	= 0;
	pThis->m_GroundFriction			= 0.7;
	pThis->m_AirFriction				= 0;
	pThis->m_WaterFriction			= 0;
	pThis->m_dwFlags					= COLLISIONFLAG_CANCLIMBCLIMBABLE | COLLISIONFLAG_UPDATEVELOCITY | COLLISIONFLAG_IGNOREDEAD | COLLISIONFLAG_USEWALLRADIUS | COLLISIONFLAG_PREVENTOSC | COLLISIONFLAG_WATERCOLLISION | COLLISIONFLAG_PICKUPCOLLISION | COLLISIONFLAG_CANENTERRESTRICTEDAREA;
}

void CCollisionInfo__SetNoCollisionAtAllDefaults(CCollisionInfo *pThis)
{
	pThis->m_WallBehavior			= WALL_NOCOLLISION;
	pThis->m_InstanceBehavior		= INSTANCE_NOCOLLISION;
	pThis->m_GravityAcceleration	= 0;
	pThis->m_BounceReturnEnergy	= 0.3;
	pThis->m_GroundFriction			= 0.7;
	pThis->m_AirFriction				= 0;
	pThis->m_WaterFriction			= 0;
	pThis->m_dwFlags					= COLLISIONFLAG_UPDATEVELOCITY | COLLISIONFLAG_USEWALLRADIUS | COLLISIONFLAG_IGNOREDEAD | COLLISIONFLAG_NOFLOORCOLLISION;
}

void CCollisionInfo__SetPlayerUnderwaterDefaults(CCollisionInfo *pThis)
{
	pThis->m_WallBehavior			= WALL_SLIDE;
	pThis->m_InstanceBehavior		= INSTANCE_SLIDE;
	pThis->m_GravityAcceleration	= GRAVITY_WATER_ACCELERATION;
	pThis->m_BounceReturnEnergy	= 0;
	pThis->m_GroundFriction			= 0.9;
	pThis->m_AirFriction				= 0;
	pThis->m_WaterFriction			= 0.08;
	pThis->m_dwFlags					= COLLISIONFLAG_UPDATEVELOCITY | COLLISIONFLAG_IGNOREDEAD | COLLISIONFLAG_USEWALLRADIUS | COLLISIONFLAG_PREVENTOSC | COLLISIONFLAG_NOGROUNDMOVEMENT | COLLISIONFLAG_WATERCOLLISION | COLLISIONFLAG_PICKUPCOLLISION | COLLISIONFLAG_CANENTERRESTRICTEDAREA;
}

void CCollisionInfo__SetPlayerAntiGravDefaults(CCollisionInfo *pThis)
{
	pThis->m_WallBehavior			= WALL_SLIDE;
	pThis->m_InstanceBehavior		= INSTANCE_SLIDE;
	pThis->m_GravityAcceleration	= 0;
	pThis->m_BounceReturnEnergy	= 0;
	pThis->m_GroundFriction			= 0.9;
	pThis->m_AirFriction				= 0;
	pThis->m_WaterFriction			= 0;
	pThis->m_dwFlags					= COLLISIONFLAG_UPDATEVELOCITY | COLLISIONFLAG_IGNOREDEAD | COLLISIONFLAG_USEWALLRADIUS | COLLISIONFLAG_PREVENTOSC | COLLISIONFLAG_NOGROUNDMOVEMENT | COLLISIONFLAG_WATERCOLLISION | COLLISIONFLAG_PICKUPCOLLISION | COLLISIONFLAG_CANENTERRESTRICTEDAREA;
}

void CCollisionInfo__SetPlayerOnWaterSurfaceDefaults(CCollisionInfo *pThis)
{
	pThis->m_WallBehavior			= WALL_SLIDE;
	pThis->m_InstanceBehavior		= INSTANCE_SLIDE;
	pThis->m_GravityAcceleration	= 0;
	pThis->m_BounceReturnEnergy	= 0;
	pThis->m_GroundFriction			= 0.9;
	pThis->m_AirFriction				= 0;
	pThis->m_WaterFriction			= 0;
	pThis->m_dwFlags					= COLLISIONFLAG_CANCLIMBCLIMBABLE | COLLISIONFLAG_UPDATEVELOCITY | COLLISIONFLAG_IGNOREDEAD | COLLISIONFLAG_USEWALLRADIUS | COLLISIONFLAG_PREVENTOSC | COLLISIONFLAG_WATERCOLLISION | COLLISIONFLAG_PICKUPCOLLISION | COLLISIONFLAG_CANENTERRESTRICTEDAREA;
}

void CCollisionInfo__SetCharacterDefaults(CCollisionInfo *pThis)
{
	pThis->m_WallBehavior			= WALL_SLIDEBOUNCE;
	pThis->m_InstanceBehavior		= INSTANCE_SLIDE;
	pThis->m_GravityAcceleration	= GRAVITY_ACCELERATION;
	pThis->m_BounceReturnEnergy	= 0.3;
	pThis->m_GroundFriction			= 0.7;
	pThis->m_AirFriction				= 0;
	pThis->m_WaterFriction			= 0;
	pThis->m_dwFlags					= COLLISIONFLAG_UPDATEVELOCITY | COLLISIONFLAG_USEWALLRADIUS | COLLISIONFLAG_WATERCOLLISION;
}

void CCollisionInfo__SetFlyingCharacterDefaults(CCollisionInfo *pThis)
{
	pThis->m_WallBehavior			= WALL_SLIDEBOUNCE;
	pThis->m_InstanceBehavior		= INSTANCE_SLIDE;
	pThis->m_GravityAcceleration	= 0;
	pThis->m_BounceReturnEnergy	= 0.3;
	pThis->m_GroundFriction			= 0.7;
	pThis->m_AirFriction				= 0;
	pThis->m_WaterFriction			= 0;
	pThis->m_dwFlags					= COLLISIONFLAG_UPDATEVELOCITY | COLLISIONFLAG_USEWALLRADIUS | COLLISIONFLAG_IGNOREDEAD | COLLISIONFLAG_WATERCOLLISION;
}

void CCollisionInfo__SetClimbingCharacterDefaults(CCollisionInfo *pThis)
{
	pThis->m_WallBehavior			= WALL_SLIDEBOUNCE;
	pThis->m_InstanceBehavior		= INSTANCE_SLIDE;
	pThis->m_GravityAcceleration	= GRAVITY_ACCELERATION;
	pThis->m_BounceReturnEnergy	= 0.3;
	pThis->m_GroundFriction			= 0.7;
	pThis->m_AirFriction				= 0;
	pThis->m_WaterFriction			= 0;
	pThis->m_dwFlags					= COLLISIONFLAG_UPDATEVELOCITY | COLLISIONFLAG_CANCLIMB | COLLISIONFLAG_USEWALLRADIUS | COLLISIONFLAG_WATERCOLLISION;
}

void CCollisionInfo__SetPlayerClimbingSideStepCharacterDefaults(CCollisionInfo *pThis)
{
	pThis->m_WallBehavior			= WALL_STICK;
	pThis->m_InstanceBehavior		= INSTANCE_SLIDE;
	pThis->m_GravityAcceleration	= 0;
	pThis->m_BounceReturnEnergy	= 0;
	pThis->m_GroundFriction			= 0;
	pThis->m_AirFriction				= 0;
	pThis->m_WaterFriction			= 0;
	pThis->m_dwFlags					= COLLISIONFLAG_CANCLIMBCLIMBABLE | COLLISIONFLAG_IGNOREDEAD | COLLISIONFLAG_USEWALLRADIUS | COLLISIONFLAG_PREVENTOSC | COLLISIONFLAG_WATERCOLLISION | COLLISIONFLAG_PICKUPCOLLISION | COLLISIONFLAG_STICKTOGROUND | COLLISIONFLAG_CANENTERRESTRICTEDAREA;
}

void CCollisionInfo__SetUnderwaterCharacterDefaults(CCollisionInfo *pThis)
{
	pThis->m_WallBehavior			= WALL_SLIDE;
	pThis->m_InstanceBehavior		= INSTANCE_SLIDE;
	pThis->m_GravityAcceleration	= 0;
	pThis->m_BounceReturnEnergy	= 0.3;
	pThis->m_GroundFriction			= 0.7;
	pThis->m_AirFriction				= 0;
	pThis->m_WaterFriction			= 0;
	pThis->m_dwFlags					= COLLISIONFLAG_UPDATEVELOCITY | COLLISIONFLAG_USEWALLRADIUS | COLLISIONFLAG_WATERCOLLISION;
}

void CCollisionInfo__SetUnderwaterCharacterDeadDefaults(CCollisionInfo *pThis)
{
	pThis->m_WallBehavior			= WALL_SLIDE;
	pThis->m_InstanceBehavior		= INSTANCE_SLIDE;
	pThis->m_GravityAcceleration	= GRAVITY_WATER_ACCELERATION/4;
	pThis->m_BounceReturnEnergy	= 0.3;
	pThis->m_GroundFriction			= 0.7;
	pThis->m_AirFriction				= 0;
	pThis->m_WaterFriction			= 0.04;
	pThis->m_dwFlags					= COLLISIONFLAG_UPDATEVELOCITY | COLLISIONFLAG_USEWALLRADIUS | COLLISIONFLAG_WATERCOLLISION;
}

void CCollisionInfo__SetUnderwaterCharacterFloatDeadDefaults(CCollisionInfo *pThis)
{
	pThis->m_WallBehavior			= WALL_SLIDE;
	pThis->m_InstanceBehavior		= INSTANCE_SLIDE;
	pThis->m_GravityAcceleration	= 0;//-(GRAVITY_WATER_ACCELERATION/9.0);
	pThis->m_BounceReturnEnergy	= 0.3;
	pThis->m_GroundFriction			= 0.7;
	pThis->m_AirFriction				= 0;
	pThis->m_WaterFriction			= 0.04;
	pThis->m_dwFlags					= COLLISIONFLAG_UPDATEVELOCITY | COLLISIONFLAG_USEWALLRADIUS | COLLISIONFLAG_WATERCOLLISION;
}

void CCollisionInfo__SetUnderwaterClimbingCharacterDefaults(CCollisionInfo *pThis)
{
	pThis->m_WallBehavior			= WALL_SLIDE;
	pThis->m_InstanceBehavior		= INSTANCE_SLIDE;
	pThis->m_GravityAcceleration	= 0;
	pThis->m_BounceReturnEnergy	= 0.3;
	pThis->m_GroundFriction			= 0.7;
	pThis->m_AirFriction				= 0;
	pThis->m_WaterFriction			= 0;
	pThis->m_dwFlags					= COLLISIONFLAG_UPDATEVELOCITY | COLLISIONFLAG_CANCLIMB | COLLISIONFLAG_USEWALLRADIUS | COLLISIONFLAG_WATERCOLLISION;
}

void CCollisionInfo__SetParticleDefaults(CCollisionInfo *pThis)
{
	pThis->m_WallBehavior			= WALL_BOUNCE;
	pThis->m_InstanceBehavior		= INSTANCE_STICK;
	pThis->m_GravityAcceleration	= GRAVITY_EARTH/10;
	pThis->m_BounceReturnEnergy	= 17.0/24.0;
	pThis->m_GroundFriction			= 0.1;
	pThis->m_AirFriction				= 0;
	pThis->m_WaterFriction			= 0;
	pThis->m_dwFlags					= COLLISIONFLAG_UPDATEVELOCITY | COLLISIONFLAG_BOUNCEOFFGROUND | COLLISIONFLAG_WATERCOLLISION | COLLISIONFLAG_CANENTERRESTRICTEDAREA;
}

void CCollisionInfo__SetPickupDefaults(CCollisionInfo *pThis)
{
	pThis->m_WallBehavior			= WALL_BOUNCE;
	pThis->m_InstanceBehavior		= INSTANCE_SLIDE;
	pThis->m_GravityAcceleration	= GRAVITY_EARTH/4;
	pThis->m_BounceReturnEnergy	= 0.5;
	pThis->m_GroundFriction			= 0.4;
	pThis->m_AirFriction				= 0;
	pThis->m_WaterFriction			= 0;
	pThis->m_dwFlags					= COLLISIONFLAG_UPDATEVELOCITY | COLLISIONFLAG_USEWALLRADIUS | COLLISIONFLAG_BOUNCEOFFGROUND | COLLISIONFLAG_WATERCOLLISION | COLLISIONFLAG_CANENTERRESTRICTEDAREA;
}

void CCollisionInfo__SetMovementTestDefaults(CCollisionInfo *pThis)
{
	pThis->m_WallBehavior			= WALL_STICK;
	pThis->m_InstanceBehavior		= INSTANCE_STICK;
	pThis->m_GravityAcceleration	= 0;
	pThis->m_BounceReturnEnergy	= 1;
	pThis->m_GroundFriction			= 0;
	pThis->m_AirFriction				= 0;
	pThis->m_WaterFriction			= 0;
	pThis->m_dwFlags					= COLLISIONFLAG_NOHILLSLOWDOWN;
}

void CCollisionInfo__SetRayCastDefaults(CCollisionInfo *pThis)
{
	pThis->m_WallBehavior			= WALL_STICK;
	pThis->m_InstanceBehavior		= INSTANCE_NOCOLLISION;
	pThis->m_GravityAcceleration	= 0;
	pThis->m_BounceReturnEnergy	= 0;
	pThis->m_GroundFriction			= 0;
	pThis->m_AirFriction				= 0;
	pThis->m_WaterFriction			= 0;
	pThis->m_dwFlags					= COLLISIONFLAG_CANENTERRESTRICTEDAREA;
}

void CCollisionInfo__SetNearRegionTestDefaults(CCollisionInfo *pThis)
{
	pThis->m_WallBehavior			= WALL_STICK;
	pThis->m_InstanceBehavior		= INSTANCE_NOCOLLISION;
	pThis->m_GravityAcceleration	= 0;
	pThis->m_BounceReturnEnergy	= 1;
	pThis->m_GroundFriction			= 0;
	pThis->m_AirFriction				= 0;
	pThis->m_WaterFriction			= 0;
	pThis->m_dwFlags					= COLLISIONFLAG_NOHILLSLOWDOWN | COLLISIONFLAG_MOVETHROUGHDOORS | COLLISIONFLAG_CANENTERRESTRICTEDAREA | COLLISIONFLAG_NOFLOORCOLLISION;
}

void CCollisionInfo__SetNoWaterMovementTestDefaults(CCollisionInfo *pThis)
{
	pThis->m_WallBehavior			= WALL_STICK;
	pThis->m_InstanceBehavior		= INSTANCE_STICK;
	pThis->m_GravityAcceleration	= 0;
	pThis->m_BounceReturnEnergy	= 1;
	pThis->m_GroundFriction			= 0;
	pThis->m_AirFriction				= 0;
	pThis->m_WaterFriction			= 0;
	pThis->m_dwFlags					= COLLISIONFLAG_NOHILLSLOWDOWN;
}

void CCollisionInfo__ClearCollision(CCollisionInfo *pThis)
{
	pThis->m_WallCollision = FALSE;
	pThis->m_pInstanceCollision = NULL;
	pThis->m_GroundCollision = GROUNDCOLLISION_NONE;
	pThis->m_WaterCollision = FALSE;
}

// returns TRUE for a collision
// on exit, *ppCurrentRegion contains pointer to new region
// and *pvCurrentPos contains position after collision
// (will be equal to vDesiredPos if no collision)
#define COLLISION_MAX_ITERATIONS		100
#define TRACK_GROUND_HEIGHT			(0.1*SCALING_FACTOR)
BOOL CAnimInstanceHdr__Collision(CAnimInstanceHdr *pThis,
											CGameRegion **ppCurrentRegion,
											CVector3 *pvCurrentPos,
											CVector3 vDesiredPos,
											CCollisionInfo *pCI,
											BOOL ClimbUp, BOOL ClimbDown, BOOL EnterWater, BOOL ExitWater)
{
	int						iterations,
								bouncesLeft,
								cEdge, nLast, nSmallest;
	float						remain,
								uphillCosine, prevUphillCosine,
								friction,
								gravity,
								groundHeight, ceilingHeight,
								t, minT, groundT, waterT, ceilingT, minGroundCeilingT,
								deltaX, deltaZ,
								normalX, normalZ,
								dotNormalDelta,
								collisionEdgeDeltaX, collisionEdgeDeltaZ,
								closestWallDeltaX, closestWallDeltaZ,
								closestInstDeltaX, closestInstDeltaZ,
								magEdge,
								unitEdgeX, unitEdgeZ,
								velocityDotEdge, deltaDotEdge,
								magVelocitySquared, magVelocity, newMagVelocity,
								headHeight,
								rotY;
	BOOL						regionChanged, desiredChanged,
								startedOnGround, movingAlongGround,
								collided, underWater,
								isPlayer,
								slide,
								ceilingCollision;
	CVector3					vStartPos,
								vVelocity,
								vSlide,
								vUseDesired,
								vInstCollisionDesiredPos,
								vGroundNormal,
								vCorner, vNextCorner,
								vWallNormal,
								vPickupPos;
	CGameRegion				*pSmallest;
	CROMRegionSet			*pRegionSet;
	DWORD						prevInstanceBehavior;
	CGameObjectInstance	*pAnim;
	CInstanceHdr			*pPickup;



#if 0
	// Call Steve's collision
	CVector3__MultScaler(&vVelocity, &pThis->m_vVelocity, frame_increment) ;
	CVector3__Add(&vVelocity, &vVelocity, &vDesiredPos) ;
	CVector3__Subtract(&vVelocity, &vVelocity, pvCurrentPos) ;
	return CAnimInstanceHdr__Collision2(pThis,
										  ppCurrentRegion,
										  pvCurrentPos,
										  vVelocity,
										  pCI,
										  ClimbUp, ClimbDown, EnterWater, ExitWater, TRUE) ;
#endif



	//rmonPrintf("climbUp:%d, climbDown:%d\n", ClimbUp, ClimbDown);
	//rmonPrintf("gravity = %f\n", pCI->m_GravityAcceleration);

	// reset collision output
	CCollisionInfo__ClearCollision(pCI);

	if (pThis->ih.m_Type == I_ANIMATED)
	{
		pAnim = (CGameObjectInstance*) pThis;
		rotY = pAnim->m_RotY;
	}
	else
	{
		pAnim = NULL;
		rotY = 0;
	}

	isPlayer = ( pAnim == CEngineApp__GetPlayer(GetApp()) );

//	if (isPlayer)
//		osSyncPrintf("frame:%d\n", game_frame_number) ;

	// hold on to start position to find first collision
	vStartPos = *pvCurrentPos;

	// deal with velocity as distance (velocity*time)
//rmonPrintf("vel.y:%f\n", pThis->m_vVelocity.y);
	CVector3__MultScaler(&vVelocity, &pThis->m_vVelocity, frame_increment);

	// allow appropriate number of bounces
	groundHeight = CGameRegion__GetGroundHeight(*ppCurrentRegion, pvCurrentPos->x, pvCurrentPos->z);
	if (     (pCI->m_WallBehavior == WALL_BOUNCE)
			|| ((pCI->m_WallBehavior == WALL_SLIDEBOUNCE) && ((pvCurrentPos->y - groundHeight) > 0.01)) )
	{
		bouncesLeft = 2;	// RANDOM(3) + 1;
	}
	else
	{
		bouncesLeft = 0;
	}


	// ground friction
	if ((pvCurrentPos->y - groundHeight) <= 0.01)
	{
		friction = max(0, min(1, 1 - pCI->m_GroundFriction*frame_increment));

		vVelocity.x *= friction;
		vVelocity.z *= friction;

		// slow down velocity if it's downhill
		if (vVelocity.y < 0)
			vVelocity.y *= friction;
	}


	// add gravity to velocity
	gravity = pCI->m_GravityAcceleration*(SCALING_FACTOR/FRAME_FPS)*frame_increment*frame_increment;
	vVelocity.y += gravity;

	//if ((pCI->m_dwFlags & COLLISIONFLAG_NOGROUNDMOVEMENT) && (vDesiredPos.y >= pvCurrentPos->y))
	if (pCI->m_dwFlags & COLLISIONFLAG_NOGROUNDMOVEMENT)
		startedOnGround = FALSE;
	else
		startedOnGround = ((pvCurrentPos->y - groundHeight) <= 0.01);


	// air/water friction
	if (pCI->m_WaterFriction == pCI->m_AirFriction)
	{
		friction = pCI->m_AirFriction;
	}
	else
	{
		underWater = FALSE;
		if (*ppCurrentRegion)
		{
			if ((*ppCurrentRegion)->m_wFlags & REGFLAG_HASWATER)
			{
				pRegionSet = CScene__GetRegionAttributes(&GetApp()->m_Scene, *ppCurrentRegion);
				if (pRegionSet)
					underWater = (pvCurrentPos->y < pRegionSet->m_WaterElevation);
			}
		}

		if (underWater)
			friction = pCI->m_WaterFriction;
		else
			friction = pCI->m_AirFriction;
	}

	if (friction != 0)
	{
		magVelocitySquared = CVector3__MagnitudeSquared(&pThis->m_vVelocity);
		magVelocity = sqrt(magVelocitySquared);
		if (magVelocity != 0)
		{
			newMagVelocity = magVelocity - min(magVelocity, friction*magVelocitySquared*frame_increment*(1/SCALING_FACTOR));

			//rmonPrintf("friction:%f, magVelocity:%f, newMagVelocity:%f\n", friction, magVelocity, newMagVelocity);
			CVector3__MultScaler(&vVelocity, &vVelocity, newMagVelocity/magVelocity);
		}
	}


	// add slide to vDesiredPos if on ground
	if ((*ppCurrentRegion) && startedOnGround)
	{
		slide =		CGameRegion__IsCliff(*ppCurrentRegion)
					&& !((*ppCurrentRegion)->m_wFlags & REGFLAG_CLIMBABLE)
					&& (vVelocity.y <= 0)
					&& !(pThis->ih.m_pEA->m_dwTypeFlags & AI_TYPE_CLIMBER)
					&& !(pCI->m_dwFlags & COLLISIONFLAG_NOHILLSLOWDOWN);

		if (pAnim)
			if (pAnim->m_dwFlags & ANIM_INSTANCE_HANGINGON)
				slide = FALSE;

		if (pAnim)
			if ( (pAnim->ah.ih.m_pEA->m_dwTypeFlags & AI_ALREADY_DEAD) && !(pCI->m_dwFlags & COLLISIONFLAG_NOHILLSLOWDOWN) )
				slide = TRUE;

		if (slide)
		{
			//rmonPrintf("sliding\n");

			vSlide = CGameRegion__GetSlideVector(*ppCurrentRegion, gravity);

			uphillCosine = CGameRegion__GetUphillCosine(*ppCurrentRegion,
																	  pvCurrentPos->x, pvCurrentPos->z,
																	  pvCurrentPos->x + vSlide.x, pvCurrentPos->z + vSlide.z);
			ASSERT(uphillCosine != 0);
			vDesiredPos.x += vSlide.x/uphillCosine;
			vDesiredPos.y += vSlide.y;
			vDesiredPos.z += vSlide.z/uphillCosine;
		}
	}

	// add gravity to desired position
	CVector3__Add(&vDesiredPos, &vDesiredPos, &vVelocity);

	// skip collision if there is no movement
// IS THIS WORKING RIGHT?
//
//	if (		(vDesiredPos.x == pvCurrentPos->x)
//			&&	(vDesiredPos.y == pvCurrentPos->y)
//			&& (vDesiredPos.z == pvCurrentPos->z) )
//	{
//		if (pCI->m_dwFlags & COLLISIONFLAG_UPDATEVELOCITY)
//			CVector3__MultScaler(&pThis->m_vVelocity, &vVelocity, 1/frame_increment);
//
//		return FALSE;
//	}


	// initialize variables for loop
	iterations = 0;
	remain = 1;
	regionChanged = TRUE;
	desiredChanged = TRUE;
	movingAlongGround = TRUE;
	collided = FALSE;
	prevUphillCosine = 1;
	nLast = -1;

	// piecewise collision loop
	do
	{
		groundHeight = CGameRegion__GetGroundHeight(*ppCurrentRegion, pvCurrentPos->x, pvCurrentPos->z);

		if (regionChanged)
		{
			regionChanged = FALSE;

			// move slower on ground if going up
			if (		(!(pCI->m_dwFlags & COLLISIONFLAG_NOHILLSLOWDOWN))
					&& ((pvCurrentPos->y - groundHeight) < TRACK_GROUND_HEIGHT) )
			{
				uphillCosine = CGameRegion__GetUphillCosine(*ppCurrentRegion,
																		  pvCurrentPos->x,
																		  pvCurrentPos->z,
																		  vDesiredPos.x,
																		  vDesiredPos.z);
			}
			else
			{
				uphillCosine = 1;
			}

			if (uphillCosine != prevUphillCosine)
			{
				desiredChanged = TRUE;
				prevUphillCosine = uphillCosine;
			}
		}

		if (desiredChanged)
		{
			desiredChanged = FALSE;

			vUseDesired.x = BlendFLOAT(uphillCosine, pvCurrentPos->x, vDesiredPos.x);
			vUseDesired.y = vDesiredPos.y;
			vUseDesired.z = BlendFLOAT(uphillCosine, pvCurrentPos->z, vDesiredPos.z);

			// move along ground
			if (startedOnGround && ((pvCurrentPos->y - groundHeight) < 10*TRACK_GROUND_HEIGHT) && (vVelocity.y <= 0))
			{
				//vVelocity.y = gravity;
				movingAlongGround = TRUE;
			}
			else
			{
				movingAlongGround = FALSE;
			}

			// instance collision
			if (iterations != 0)
			{
				// reduce number of iterations in many cases
				prevInstanceBehavior = pCI->m_InstanceBehavior;

				if (pCI->m_InstanceBehavior != INSTANCE_NOCOLLISION)
					pCI->m_InstanceBehavior = INSTANCE_STICK;
			}

			if (uphillCosine != 0)
			{
				vInstCollisionDesiredPos = *pvCurrentPos;
				CAnimInstanceHdr__InstanceCollision(pThis,
																*ppCurrentRegion,
																&vInstCollisionDesiredPos, vUseDesired,
																pCI);
				// keep pickups from jerking camera
				if ((pCI->m_dwFlags & COLLISIONFLAG_PICKUPCOLLISION) && pCI->m_pInstanceCollision)
				{
					if (pCI->m_pInstanceCollision->m_Type == I_SIMPLE)
					{
						// do collision again without pickup collision
						pPickup = pCI->m_pInstanceCollision;
						vPickupPos = pCI->m_vInstanceCollisionPos;
						pCI->m_dwFlags &= ~COLLISIONFLAG_PICKUPCOLLISION;
						vInstCollisionDesiredPos = *pvCurrentPos;

						CAnimInstanceHdr__InstanceCollision(pThis,
																		*ppCurrentRegion,
																		&vInstCollisionDesiredPos, vUseDesired,
																		pCI);

						pCI->m_dwFlags |= COLLISIONFLAG_PICKUPCOLLISION;
						pCI->m_pInstanceCollision = pPickup;
						pCI->m_vInstanceCollisionPos = vPickupPos;
					}
				}
				vUseDesired = vInstCollisionDesiredPos;

				// convert back to desired
				vDesiredPos.x = BlendFLOAT(1/uphillCosine, pvCurrentPos->x, vUseDesired.x);
				vDesiredPos.y = vUseDesired.y;
				vDesiredPos.z = BlendFLOAT(1/uphillCosine, pvCurrentPos->z, vUseDesired.z);
			}

			if (iterations != 0)
				pCI->m_InstanceBehavior = prevInstanceBehavior;
		}

		// calculate minT
		deltaX = vUseDesired.x - pvCurrentPos->x;
		deltaZ = vUseDesired.z - pvCurrentPos->z;

		minT = 1;
		//pSmallest = (CGameRegion*) -1;
		if (*ppCurrentRegion)
		{
			for (cEdge=0; cEdge<3; cEdge++)
			{
				//pNeighbor = (*ppCurrentRegion)->m_pNeighbors[cEdge];

				// don't test deflected edge (nLast)
				if (cEdge != nLast)
				{
					vCorner		= (*ppCurrentRegion)->m_pCorners[cEdge]->m_vCorner;
					vNextCorner	= (*ppCurrentRegion)->m_pCorners[(cEdge + 1)%3]->m_vCorner;

					normalX = vNextCorner.z - vCorner.z;
					normalZ = vCorner.x - vNextCorner.x;

					dotNormalDelta = normalX*deltaX + normalZ*deltaZ;

					// skip edge if:
					// it is parallel to movement
					// edge or delta is degenerate
					// edge is behind delta
					if (dotNormalDelta < 0)
					{
						// find value t such that ((vCurrent + t*(vDesired - vCurrent)) - vCorner) * vNormal = 0
						t = (   normalX*(pvCurrentPos->x - vCorner.x)
								+ normalZ*(pvCurrentPos->z - vCorner.z) )
								/ -dotNormalDelta;

						// use closest intersection
						if (t < minT)
						{
							minT = t;

							//pSmallest = pNeighbor;
							nSmallest = cEdge;

							collisionEdgeDeltaX = -normalZ;
							collisionEdgeDeltaZ = normalX;
						}
					}
				}
			}
		}
		nLast = -1;

		if (		(pCI->m_dwFlags & COLLISIONFLAG_NOFLOORCOLLISION)
				|| (*ppCurrentRegion && ((*ppCurrentRegion)->m_wFlags & REGFLAG_FALLDEATH)) )
		{
			groundT = 1;
		}
		else if (pCI->m_dwFlags & COLLISIONFLAG_STICKTOGROUND)
		{
			CGameRegion__GetGroundIntersection(*ppCurrentRegion, pvCurrentPos, &vUseDesired, &groundT);
			groundT = max(0, min(1, groundT));
		}
		else
		{
			if (movingAlongGround || (pCI->m_dwFlags & (COLLISIONFLAG_NOHILLSLOWDOWN | COLLISIONFLAG_NOGROUNDMOVEMENT)))
			{
				groundT = 1;
			}
			else
			{
				// calculate groundT
				if (!CGameRegion__GetGroundIntersection(*ppCurrentRegion, pvCurrentPos, &vUseDesired, &groundT))
					groundT = 1;
			}
		}

		if ((pCI->m_dwFlags & COLLISIONFLAG_WATERCOLLISION) && (!pCI->m_WaterCollision))
		{
			if (CGameRegion__CalculateWaterIntersection(*ppCurrentRegion, pvCurrentPos, &vUseDesired, &waterT))
			{
				if ((waterT < groundT) && (waterT < minT))
				{
					pCI->m_WaterCollision = TRUE;
					CVector3__Blend(&pCI->m_vWaterCollisionPos, waterT, pvCurrentPos, &vUseDesired);
					pCI->m_pWaterCollisionRegion = *ppCurrentRegion;
				}
			}
		}

		if (*ppCurrentRegion)
		{
			if ((*ppCurrentRegion)->m_wFlags & REGFLAG_CEILING)
			{
				if (!CGameRegion__GetCeilingIntersection(*ppCurrentRegion, pvCurrentPos, &vUseDesired, &ceilingT))
					ceilingT = 1;
			}
			else
			{
				ceilingT = 1;
			}
		}
		else
		{
			ceilingT = 1;
		}

		ceilingCollision = (ceilingT < groundT);
		minGroundCeilingT = ceilingCollision ? ceilingT : groundT;

		if (minGroundCeilingT < minT)
		{
			// ceiling/ground collison

			CVector3__Blend(pvCurrentPos, minGroundCeilingT, pvCurrentPos, &vUseDesired);

			if (!pCI->m_GroundCollision)
			{
				pCI->m_GroundCollision = ceilingCollision ? GROUNDCOLLISION_CEILING : GROUNDCOLLISION_GROUND;
				pCI->m_vGroundCollisionPos = *pvCurrentPos;
				pCI->m_pGroundCollisionRegion = *ppCurrentRegion;
			}

			if (pCI->m_dwFlags & COLLISIONFLAG_BOUNCEOFFGROUND)
			{
				if (bouncesLeft)
				{
					remain -= minGroundCeilingT;

					vDesiredPos.x -= vVelocity.x*remain;
					vDesiredPos.y -= vVelocity.y*remain;
					vDesiredPos.z -= vVelocity.z*remain;

					vVelocity.y -= gravity*remain;

					// bounce velocity off ceiling/ground
					if (ceilingCollision)
						vGroundNormal = CGameRegion__GetCeilingUnitNormal(*ppCurrentRegion);
					else
						vGroundNormal = CGameRegion__GetGroundUnitNormal(*ppCurrentRegion);

					vVelocity = CVector3__Reflect(&vVelocity, &vGroundNormal, pCI->m_BounceReturnEnergy);

					vVelocity.y += gravity*remain;

					vDesiredPos.x += vVelocity.x*remain;
					vDesiredPos.y += vVelocity.y*remain;
					vDesiredPos.z += vVelocity.z*remain;

					desiredChanged = TRUE;
					bouncesLeft--;
				}
				else
				{
					remain = 0;
				}
			}
			else
			{
				vVelocity.y = 0;

				remain = 0;
			}
		}
		else if (minT < 1)
		{
			// region edge

			// move to edge
			CVector3__Blend(pvCurrentPos, minT, pvCurrentPos, &vUseDesired);
			remain -= minT;

			pSmallest = CGameRegion__CanEnter(*ppCurrentRegion, pvCurrentPos, vStartPos.y, rotY,
														 nSmallest, &vDesiredPos,
														 ClimbUp, ClimbDown, EnterWater, ExitWater, pCI);
			if (pSmallest)
			{
				//rmonPrintf("enteredNewRegion:%x\n", pSmallest);

				// collision with another region
				// make current region and continue collision
				*ppCurrentRegion	= pSmallest;
				regionChanged = TRUE;
			}
			else
			{
				if (!pCI->m_WallCollision)
				{
					pCI->m_WallCollision = TRUE;
					pCI->m_vWallCollisionPos = *pvCurrentPos;
					pCI->m_nWallCollisionEdge = nSmallest;
					pCI->m_pWallCollisionRegion = *ppCurrentRegion;

					if (pCI->m_pInstanceCollision)
					{
						// clear instance collision if wall collision was closer

						closestWallDeltaX = pCI->m_vWallCollisionPos.x - vStartPos.x;
						closestWallDeltaZ = pCI->m_vWallCollisionPos.z - vStartPos.z;
						closestInstDeltaX = pCI->m_vInstanceCollisionPos.x - vStartPos.x;
						closestInstDeltaZ = pCI->m_vInstanceCollisionPos.z - vStartPos.z;

						if (	  (closestWallDeltaX*closestWallDeltaX + closestWallDeltaZ*closestWallDeltaZ)
								< (closestInstDeltaX*closestInstDeltaX + closestInstDeltaZ*closestInstDeltaZ) )
						{
							pCI->m_pInstanceCollision = NULL;
						}
					}
				}

				if (pCI->m_WallBehavior == WALL_STICK)
				{
					vVelocity.x = vVelocity.y = vVelocity.z = 0;

					remain = 0;
				}
				else
				{
					if (bouncesLeft)
					{
						vDesiredPos.x -= vVelocity.x*remain;
						vDesiredPos.y -= vVelocity.y*remain;
						vDesiredPos.z -= vVelocity.z*remain;

						// bounce velocity
						vWallNormal.x = collisionEdgeDeltaZ;
						vWallNormal.y = 0;
						vWallNormal.z = -collisionEdgeDeltaX;
						CVector3__Normalize(&vWallNormal);
						vVelocity = CVector3__Reflect(&vVelocity, &vWallNormal, pCI->m_BounceReturnEnergy);

						vDesiredPos.x += vVelocity.x*remain;
						vDesiredPos.y += vVelocity.y*remain;
						vDesiredPos.z += vVelocity.z*remain;

						nLast = nSmallest;
						desiredChanged = TRUE;
						bouncesLeft--;
						collided = TRUE;
					}
					else
					{
						if (collided)
						{
							pvCurrentPos->y = vUseDesired.y;
							remain = 0;
						}
						else
						{
							magEdge	= sqrt(collisionEdgeDeltaX*collisionEdgeDeltaX
												 + collisionEdgeDeltaZ*collisionEdgeDeltaZ);
							if (magEdge == 0)
							{
								remain = 0;
							}
							else
							{
								unitEdgeX = collisionEdgeDeltaX/magEdge;
								unitEdgeZ = collisionEdgeDeltaZ/magEdge;

								if (pCI->m_WallBehavior == WALL_REDIRECT)
								{
									vDesiredPos.x -= vVelocity.x*remain;
									vDesiredPos.y -= vVelocity.y*remain;
									vDesiredPos.z -= vVelocity.z*remain;

									// redirect velocity
									velocityDotEdge = vVelocity.x*unitEdgeX + vVelocity.z*unitEdgeZ;
									vVelocity.x = velocityDotEdge*unitEdgeX;
									vVelocity.z = velocityDotEdge*unitEdgeZ;

									vDesiredPos.x += vVelocity.x*remain;
									vDesiredPos.y += vVelocity.y*remain;
									vDesiredPos.z += vVelocity.z*remain;
								}
								else
								{
									// slide
									deltaDotEdge = deltaX*unitEdgeX + deltaZ*unitEdgeZ;
									vDesiredPos.x = pvCurrentPos->x + (1 - minT)*deltaDotEdge*unitEdgeX;
									vDesiredPos.z = pvCurrentPos->z + (1 - minT)*deltaDotEdge*unitEdgeZ;
								}

								nLast = nSmallest;
								desiredChanged = TRUE;
							}
						}

						collided = TRUE;
					}
				}
			}
		}
		else
		{
			// completed movement

			*pvCurrentPos = vUseDesired;

			remain = 0;
		}
	} while ((remain > 0.0001) && (++iterations < COLLISION_MAX_ITERATIONS));

	if (*ppCurrentRegion)
	{
		if ((*ppCurrentRegion)->m_wFlags & REGFLAG_CEILING)
		{
			ceilingHeight = CGameRegion__GetCeilingHeight(*ppCurrentRegion, pvCurrentPos->x, pvCurrentPos->z);
			if (isPlayer)
			{
				// keep eye away from ceiling
				headHeight = CEngineApp__GetEyePos(GetApp()).y + 1.5*(TUROK_HEIGHT - TUROK_EYE_HEIGHT);

				if (headHeight > ceilingHeight)
				{
					pvCurrentPos->y -= headHeight - ceilingHeight;
					vVelocity.y = 0;
				}
			}
			else
			{
				pvCurrentPos->y = min(pvCurrentPos->y, ceilingHeight);
			}
		}
	}

	groundHeight = CGameRegion__GetGroundHeight(*ppCurrentRegion, pvCurrentPos->x, pvCurrentPos->z);

	if (		!(pCI->m_dwFlags & COLLISIONFLAG_NOFLOORCOLLISION)
			&&	!(*ppCurrentRegion && ((*ppCurrentRegion)->m_wFlags & REGFLAG_FALLDEATH)) )
	{
		if (pCI->m_dwFlags & COLLISIONFLAG_NOHILLSLOWDOWN)
		{
			pvCurrentPos->y = max(groundHeight, vDesiredPos.y);
		}
		else if (pCI->m_dwFlags & COLLISIONFLAG_NOGROUNDMOVEMENT)
		{
			pvCurrentPos->y = max(pvCurrentPos->y, groundHeight);
		}
		else
		{
			if (startedOnGround && movingAlongGround)
			{
				if ((pvCurrentPos->y - groundHeight) < 10*TRACK_GROUND_HEIGHT)
				{
					pvCurrentPos->y = groundHeight;
					vVelocity.y = 0;
				}
			}
			else
			{
				if (pvCurrentPos->y < groundHeight)
				{
					pvCurrentPos->y = groundHeight;
					vVelocity.y = 0;
				}
			}
		}
	}

	//rmonPrintf("iterations:%d\n", iterations);

	if (iterations == COLLISION_MAX_ITERATIONS)
	{
		// invalid region
		TRACE0("WARNING - Region collision passed maximum iterations -- CHECK FOR INVALID REGIONS\r\n");
		//rmonPrintf("collision problem!\n");
		// TEMP
		//ASSERT(FALSE);
	}

	pCI->m_vCollisionVelocity = pThis->m_vVelocity;

	if ((pCI->m_dwFlags & COLLISIONFLAG_UPDATEVELOCITY) && (frame_increment != 0.0))
		CVector3__MultScaler(&pThis->m_vVelocity, &vVelocity, 1/frame_increment);

	if ((pCI->m_dwFlags & COLLISIONFLAG_USEWALLRADIUS) && pAnim)
	{
		//if (isPlayer || CGameObjectInstance__IsVisible(pAnim))
		if (!isPlayer && CGameObjectInstance__IsVisible(pAnim))
		{
			//CAnimInstanceHdr__HangOnToCliffEdges(pThis, *ppCurrentRegion, vStartPos, pvCurrentPos, pCI);

			CAnimInstanceHdr__BackOffFromWall(pThis, ppCurrentRegion, vStartPos, pvCurrentPos, pCI, ClimbUp, ClimbDown);
		}
	}

	return (pCI->m_pInstanceCollision || pCI->m_WallCollision);
}

//#define	HANGON_SCALER	1.414213562373		// (1/sin(pi/4))
#define	HANGON_SCALER	1
BOOL CAnimInstanceHdr__HangOnToCliffEdges(CAnimInstanceHdr *pThis,
														CGameRegion *pCurrentRegion,
														CVector3 vStartPos,
														CVector3 *pvCurrentPos,
														CCollisionInfo *pCI)
{
	float						radius,
								maxY,
								distFromEdgeSquared, distFromThisEdgeSquared,
								dx, dz;
	int						cRay;
	BOOL						collision,
								nearEdge;
	CVector3					vDesired,
								vVelocity,
								vCurrentPos;
	CGameRegion				*pRegion, *pNeighbor;
	CGameObjectInstance	*pAnim;

	//rmonPrintf("vStartPos.y:%f, pvCurrentPos->y:%f\n", vStartPos.y, pvCurrentPos->y);

	if (!pCurrentRegion)
		return FALSE;

	vVelocity = pThis->m_vVelocity;
	// zero velocity so there won't be extra movement
	pThis->m_vVelocity.x = pThis->m_vVelocity.y = pThis->m_vVelocity.z = 0;

	radius = pThis->ih.m_pEA->m_CollisionWallRadius*HANGON_SCALER;
	distFromEdgeSquared = radius*radius;

	maxY = pvCurrentPos->y + 0.01*SCALING_FACTOR;
	vDesired.y = CGameRegion__GetGroundHeight(pCurrentRegion, pvCurrentPos->x, pvCurrentPos->z);

	collision = FALSE;
	nearEdge = FALSE;

	for (cRay=0; cRay<4; cRay++)
	{
		switch (cRay)
		{
			case 0:
				vDesired.x = pvCurrentPos->x - radius;
				vDesired.z = pvCurrentPos->z - radius;
				break;

			case 1:
				vDesired.x = pvCurrentPos->x + radius;
				vDesired.z = pvCurrentPos->z - radius;
				break;

			case 2:
				vDesired.x = pvCurrentPos->x - radius;
				vDesired.z = pvCurrentPos->z + radius;
				break;

			case 3:
				vDesired.x = pvCurrentPos->x + radius;
				vDesired.z = pvCurrentPos->z + radius;
				break;
		}

		pRegion = pCurrentRegion;
		vCurrentPos = *pvCurrentPos;
		vCurrentPos.y = vDesired.y;

		CAnimInstanceHdr__Collision(pThis, &pRegion, &vCurrentPos, vDesired, &ci_nearregiontest, FALSE, FALSE);
		if (ci_nearregiontest.m_WallCollision)
		{
			//rmonPrintf("hang on wall collision\n");
			pNeighbor = pRegion->m_pNeighbors[ci_nearregiontest.m_nWallCollisionEdge];
			if (pNeighbor)
			{
				if (pNeighbor->m_wFlags & REGFLAG_CLIFF)
				{
					//rmonPrintf("near edge\n");
					nearEdge = TRUE;
					dx = vCurrentPos.x - pvCurrentPos->x;
					dz = vCurrentPos.z - pvCurrentPos->z;
					distFromThisEdgeSquared = dx*dx + dz*dz;
					if (distFromThisEdgeSquared < distFromEdgeSquared)
						distFromEdgeSquared = distFromThisEdgeSquared;
				}
			}
		}

		pRegion = pCurrentRegion;
		vCurrentPos = *pvCurrentPos;
		vCurrentPos.y = vDesired.y;

		CAnimInstanceHdr__Collision(pThis, &pRegion, &vCurrentPos, vDesired, &ci_nearregiontest, FALSE, TRUE);
		if (ci_nearregiontest.m_WallCollision || (pCurrentRegion->m_wFlags & REGFLAG_CLIFF))
		//if ( (ci_nearregiontest.m_WallCollision || (pCurrentRegion->m_wFlags & REGFLAG_CLIFF))
//				&& !(pRegion->m_wFlags & REGFLAG_CLIFF) )
		{
			pRegion = pCurrentRegion;
			vCurrentPos = *pvCurrentPos;
			vCurrentPos.y = vDesired.y;

			//CAnimInstanceHdr__Collision(pThis, &pRegion, &vCurrentPos, vDesired, &ci_nearregiontest, TRUE, FALSE);
			CAnimInstanceHdr__Collision(pThis, &pRegion, &vCurrentPos, vDesired, &ci_nearregiontest, TRUE, TRUE);

			//rmonPrintf("ray:%d, height:%f\n", cRay, vCurrentPos.y);

#if 0
			if (ci_nearregiontest.m_WallCollision)
			{
				//rmonPrintf("hang on wall collision\n");
				pNeighbor = pRegion->m_pNeighbors[ci_nearregiontest.m_nWallCollisionEdge];
				if (pNeighbor)
					if (pNeighbor->m_wFlags & REGFLAG_CLIFF)
					{
						//rmonPrintf("near edge\n");
						nearEdge = TRUE;
					}
			}
#endif
			if ((vCurrentPos.y > maxY) && (vCurrentPos.y <= (vStartPos.y + 0.1*SCALING_FACTOR)))
			{
				//groundHeight = CGameRegion__GetGroundHeight(pCurrentRegion, vCurrentPos.x, vCurrentPos.z);
				//if (vCurrentPos.y > (groundHeight + 0.1*SCALING_FACTOR))
				//{
				maxY = vCurrentPos.y;

				if (pThis->ih.m_Type == I_ANIMATED)
					if ( ((CGameObjectInstance*)pThis)->m_dwFlags & ANIM_INSTANCE_HANGINGON )
						vVelocity.y = 0;

				//vVelocity.y = 0;

				collision = TRUE;
				pCI->m_GroundCollision = GROUNDCOLLISION_GROUND;
				pCI->m_vGroundCollisionPos = *pvCurrentPos;
				pCI->m_vGroundCollisionPos.y = maxY;
				pCI->m_pGroundCollisionRegion = pRegion;
				//}
			}
		}
	}

	if (collision)
	{
		//rmonPrintf("vStartPos.y:%f, pvCurrentPos->y:%f, maxY:%f\n", vStartPos.y, pvCurrentPos->y, maxY);
		pvCurrentPos->y = maxY;
	}

	//rmonPrintf("f:%d, finalY:%f\n", frame_number, maxY);

	if (pThis->ih.m_Type == I_ANIMATED)
	{
		pAnim = (CGameObjectInstance*) pThis;

		if (nearEdge)
		{
			pAnim->m_dwFlags |= ANIM_INSTANCE_NEAREDGE;

			pAnim->m_ShadowIntensity = sqrt(distFromEdgeSquared)/radius;
		}
		else
		{
			pAnim->m_dwFlags &= ~ANIM_INSTANCE_NEAREDGE;
			pAnim->m_ShadowIntensity = 1;
		}

		if (collision)
			pAnim->m_dwFlags |= ANIM_INSTANCE_HANGINGON;
		else
			pAnim->m_dwFlags &= ~ANIM_INSTANCE_HANGINGON;
	}

	pThis->m_vVelocity = vVelocity;

	return collision;
}

typedef struct EDGELISTITEM_t
{
	CGameRegion		*pRegion;
	int				nEdge;
	CROMCorner		*pSCorner, *pECorner;
	float				mag, magP,
						uX, uZ,
						spX, spZ;
	BOOL				box;
} EDGELISTITEM;

#define	BACKOFF_RAYS	7
//#define	BACKOFF_RAYS	4
//define JUMPING_RADIUS_FACTOR		0.25
#define JUMPING_RADIUS_FACTOR		1
//#define BACKOFF_PRINTS
BOOL CAnimInstanceHdr__BackOffFromWall(CAnimInstanceHdr *pThis,
													CGameRegion **ppCurrentRegion,
													CVector3 vStartPos,
													CVector3 *pvCurrentPos,
													CCollisionInfo *pCI,
													BOOL ClimbUp, BOOL ClimbDown)
{
	EDGELISTITEM	edges[BACKOFF_RAYS],
						*pEdge, *pEdge2,
						*pSmallest;
	CROMCorner		*pCorners[2*BACKOFF_RAYS],
						*pClosestCorner, *pNextClosestCorner,
						*pCorner;
	float				rX, rZ,
						deltaX, deltaZ,
						magDeltaSq, magScaler,
						radius,
						theta, incTheta,
						sinTheta, cosTheta,
						rayX, rayZ,
						invMagEdge,
						srX, srZ,
						erX, erZ,
						sDot, eDot,
						distanceSquared, closestDistanceSquared,
						closestDistance, invClosestDistance,
						dx, dz,
						mdx, mdz,
						backOffMag,
						mag,
						radiusSquared,
						t, minT,
						cX, cZ, cnX, cnZ,
						dotNormalDelta,
						normalX, normalZ;
	BOOL				collision,
						ignoreDead, prevIgnoreDead,
						match,
						startedOnGround;
	DWORD				prevInstanceBehavior;
	CVector3			vVelocity,
						vCurrentPos,
						vDesired,
						vBackOffDelta,
						vReflectNormal;
	int				cRay,
						cCorner, nCorners,
						cEdge, nEdges,
						nBoxes;
	CGameRegion		*pRegion;
	CROMCorner		*pA, *pB;
	float				abX, abZ, magAb, unX, unZ, invMag,
						apX, apZ, magAp, cornerCosTheta, pointCosTheta,
						groundHeight;
	BOOL				goToCorner, updateVelocity;
	CCollisionInfo ciTemp;

	if (!*ppCurrentRegion)
		return FALSE;

	//rmonPrintf("bo\n");

	if (pThis->ih.m_Type == I_ANIMATED)
		startedOnGround = ( ((CGameObjectInstance*) pThis)->m_dwFlags | ANIM_INSTANCE_HANGINGON );
	else
		startedOnGround = FALSE;

	if (!startedOnGround)
	{
		groundHeight = CGameRegion__GetGroundHeight(*ppCurrentRegion, vStartPos.x, vStartPos.z);
		startedOnGround = ((vStartPos.y - groundHeight) <= 0.01);
	}

	rX = pvCurrentPos->x;
	rZ = pvCurrentPos->z;

	deltaX = rX - vStartPos.x;
	deltaZ = rZ - vStartPos.z;

	magDeltaSq = deltaX*deltaX + deltaZ*deltaZ;
	if (magDeltaSq == 0)
		return FALSE;

	radius = pThis->ih.m_pEA->m_CollisionWallRadius;
	if (pCI->m_dwFlags & COLLISIONFLAG_SMALLWALLRADIUS)
		radius *= JUMPING_RADIUS_FACTOR;
	if (radius == 0)
		return FALSE;

	radiusSquared = radius*radius;

	magScaler = 2*radius/sqrt(magDeltaSq);
	deltaX *= magScaler;
	deltaZ *= magScaler;

	collision = FALSE;

	// use same ignore dead flag as pCI
	ignoreDead = (pCI->m_dwFlags & COLLISIONFLAG_IGNOREDEAD);
	if (ignoreDead)
	{
		prevIgnoreDead = (ci_movetest.m_dwFlags & COLLISIONFLAG_IGNOREDEAD);
		ci_movetest.m_dwFlags |= COLLISIONFLAG_IGNOREDEAD;
	}

	prevInstanceBehavior = ci_movetest.m_InstanceBehavior;
	ci_movetest.m_InstanceBehavior = INSTANCE_NOCOLLISION;

	vVelocity = pThis->m_vVelocity;
	// zero velocity so there won't be extra movement
	pThis->m_vVelocity.x = pThis->m_vVelocity.y = pThis->m_vVelocity.z = 0;

	vDesired.y = pvCurrentPos->y;

	nCorners = nEdges = nBoxes = 0;

	// find all wall edges
	theta = ANGLE_DTOR(-90);
	incTheta = ANGLE_DTOR(180)/(BACKOFF_RAYS - 1);
	for (cRay=0; cRay<BACKOFF_RAYS; cRay++)
	{
		sinTheta = sin(theta);
		cosTheta = cos(theta);

		rayX = deltaX*cosTheta + deltaZ*sinTheta;
		rayZ = -deltaX*sinTheta + deltaZ*cosTheta;

		vDesired.x = rX + rayX;
		vDesired.z = rZ + rayZ;

		pRegion = *ppCurrentRegion;
		vCurrentPos = *pvCurrentPos;

		CAnimInstanceHdr__Collision(pThis, &pRegion, &vCurrentPos, vDesired, &ci_movetest, ClimbUp, ClimbDown, TRUE, TRUE);
		if (ci_movetest.m_WallCollision)
		{
			if (!pCI->m_WallCollision)
			{
				pCI->m_WallCollision = TRUE;
				pCI->m_vWallCollisionPos = ci_movetest.m_vWallCollisionPos;
				pCI->m_pWallCollisionRegion = ci_movetest.m_pWallCollisionRegion;
				pCI->m_nWallCollisionEdge = ci_movetest.m_nWallCollisionEdge;
			}

			// only add each edge once
			match = FALSE;
			for (cEdge=0; cEdge<nEdges; cEdge++)
			{
				pEdge = &edges[cEdge];

				if (		(ci_movetest.m_pWallCollisionRegion == pEdge->pRegion)
						&&	(ci_movetest.m_nWallCollisionEdge == pEdge->nEdge) )
				{
					match = TRUE;
					break;
				}
			}

			if (!match)
			{
				dx = vCurrentPos.x - pvCurrentPos->x;
				dz = vCurrentPos.z - pvCurrentPos->z;
				if ((dx*dx + dz*dz) < radiusSquared)
				{
					pEdge = &edges[nEdges];

					pEdge->pRegion = ci_movetest.m_pWallCollisionRegion;
					pEdge->nEdge = ci_movetest.m_nWallCollisionEdge;

					pCorners[nCorners++] = pEdge->pSCorner = ci_movetest.m_pWallCollisionRegion->m_pCorners[ci_movetest.m_nWallCollisionEdge];
					pCorners[nCorners++] = pEdge->pECorner = ci_movetest.m_pWallCollisionRegion->m_pCorners[(ci_movetest.m_nWallCollisionEdge + 1)%3];

					pEdge->uX = pEdge->pECorner->m_vCorner.x - pEdge->pSCorner->m_vCorner.x;
					pEdge->uZ = pEdge->pECorner->m_vCorner.z - pEdge->pSCorner->m_vCorner.z;

					pEdge->mag = sqrt(pEdge->uX*pEdge->uX + pEdge->uZ*pEdge->uZ);
					if (pEdge->mag != 0)
					{
						nEdges++;

						invMagEdge = 1/pEdge->mag;
						pEdge->uX *= invMagEdge;
						pEdge->uZ *= invMagEdge;

						srX = rX - pEdge->pSCorner->m_vCorner.x;
						srZ = rZ - pEdge->pSCorner->m_vCorner.z;

						sDot = pEdge->uX*srX + pEdge->uZ*srZ;

						pEdge->spX = sDot*pEdge->uX;
						pEdge->spZ = sDot*pEdge->uZ;
						pEdge->magP = sqrt(pEdge->spX*pEdge->spX + pEdge->spZ*pEdge->spZ);

						erX = rX - pEdge->pECorner->m_vCorner.x;
						erZ = rZ - pEdge->pECorner->m_vCorner.z;

						eDot = pEdge->uX*erX + pEdge->uZ*erZ;


						pEdge->box = ((sDot >= 0) && (eDot <= 0));
						if (pEdge->box)
							nBoxes++;
					}
				}
				else
				{
					// just add corners
					pCorners[nCorners++] = ci_movetest.m_pWallCollisionRegion->m_pCorners[ci_movetest.m_nWallCollisionEdge];
					pCorners[nCorners++] = ci_movetest.m_pWallCollisionRegion->m_pCorners[(ci_movetest.m_nWallCollisionEdge + 1)%3];
				}
			}
		}

		theta += incTheta;
	}

	ci_movetest.m_InstanceBehavior = prevInstanceBehavior;

#ifdef BACKOFF_PRINTS
	rmonPrintf("nBoxes:%d, nEdges:%d, nCorners:%d\n", nBoxes, nEdges, nCorners);
#endif

	if (nBoxes >= 1)
	{
		pEdge = &edges[0];
		if (nEdges >= 2)
		{
			pEdge2 = &edges[nEdges - 1];
			if (pEdge->pECorner == pEdge2->pSCorner)
			{
				// detect if in corner
				theta = (ANGLE_PI - acos(pEdge->uX*pEdge2->uX + pEdge->uZ*pEdge2->uZ))/2;
				sinTheta = sin(theta);
				if (sinTheta != 0)
				{
					mag = radius*cos(theta)/sinTheta;
					if ( ((pEdge->mag - pEdge->magP) <= mag) && (pEdge2->magP <= mag) )
					{
#ifdef BACKOFF_PRINTS
						rmonPrintf("in corner -- mag:%f\n", mag);
#endif

						// in corner, so go to corner
						pEdge = pEdge2;
						pEdge->spX = mag*pEdge->uX;
						pEdge->spZ = mag*pEdge->uZ;
					}
					else
					{
						// otherwise, back off of closer side
						if (pEdge2->magP < (pEdge->mag - pEdge->magP))
							pEdge = pEdge2;
					}
				}

				// back off from pEdge
				vDesired.x = pEdge->pSCorner->m_vCorner.x + pEdge->spX + radius*pEdge->uZ;
				vDesired.y = pvCurrentPos->y;
				vDesired.z = pEdge->pSCorner->m_vCorner.z + pEdge->spZ - radius*pEdge->uX;

				vReflectNormal.x = pEdge->uZ;
				vReflectNormal.y = 0;
				vReflectNormal.z = -pEdge->uX;

				collision = TRUE;
			}
			else
			{
				// back away from corner area
#ifdef BACKOFF_PRINTS
				rmonPrintf("corner area\n");
#endif

				deltaX = rX - vStartPos.x;
				deltaZ = rZ - vStartPos.z;

				minT = 1;
				pSmallest = NULL;
				for (cEdge=0; cEdge<nEdges; cEdge++)
				{
					pEdge = &edges[cEdge];

					cX = pEdge->pSCorner->m_vCorner.x + pEdge->uZ*radius;
					cZ = pEdge->pSCorner->m_vCorner.z - pEdge->uX*radius;
					cnX = pEdge->pECorner->m_vCorner.x + pEdge->uZ*radius;
					cnZ = pEdge->pECorner->m_vCorner.z - pEdge->uX*radius;

					normalX = cnZ - cZ;
					normalZ = cX - cnX;

					dotNormalDelta = normalX*deltaX + normalZ*deltaZ;

					// skip edge if:
					// it is parallel to movement
					// edge or delta is degenerate
					// edge is behind delta
					if (dotNormalDelta < 0)
					{
						// find value t such that ((vCurrent + t*(vDesired - vCurrent)) - vCorner) * vNormal = 0
						t = (   normalX*(vStartPos.x - cX)
								+ normalZ*(vStartPos.z - cZ) )
								/ -dotNormalDelta;

						// use closest intersection
						if (t < minT)
						{
							minT = t;

							pSmallest = pEdge;
						}
					}
				}

				if (pSmallest)
				{
					vDesired.x = BlendFLOAT(minT, vStartPos.x, rX);
					vDesired.y = pvCurrentPos->y;
					vDesired.z = BlendFLOAT(minT, vStartPos.z, rZ);

					vReflectNormal.x = pSmallest->uZ;
					vReflectNormal.y = 0;
					vReflectNormal.z = -pSmallest->uX;

					collision = TRUE;
				}
			}
		}
		else
		{
			// back off from pEdge
			vDesired.x = pEdge->pSCorner->m_vCorner.x + pEdge->spX + radius*pEdge->uZ;
			vDesired.y = pvCurrentPos->y;
			vDesired.z = pEdge->pSCorner->m_vCorner.z + pEdge->spZ - radius*pEdge->uX;

			vReflectNormal.x = pEdge->uZ;
			vReflectNormal.y = 0;
			vReflectNormal.z = -pEdge->uX;

			collision = TRUE;
		}
	}
	else
	{
		// find closest corner
		pClosestCorner = pNextClosestCorner = NULL;
		closestDistanceSquared = 2*2*radiusSquared;
		for (cCorner=0; cCorner<nCorners; cCorner++)
		{
			pCorner = pCorners[cCorner];

			if (pCorner != pClosestCorner)
			{
				dx = pvCurrentPos->x - pCorner->m_vCorner.x;
				dz = pvCurrentPos->z - pCorner->m_vCorner.z;
				distanceSquared = dx*dx + dz*dz;

				if (distanceSquared < closestDistanceSquared)
				{
					mdx = dx;
					mdz = dz;
					closestDistanceSquared = distanceSquared;
					pNextClosestCorner = pClosestCorner;
					pClosestCorner = pCorner;
				}
			}
		}

		if (pNextClosestCorner)
		{
			pA = pClosestCorner;
			pB = pNextClosestCorner;

			abX = pB->m_vCorner.x - pA->m_vCorner.x;
			abZ = pB->m_vCorner.z - pA->m_vCorner.z;
			magAb = sqrt(abX*abX + abZ*abZ);

			if (magAb == 0)
			{
				unX = unZ = 0;
			}
			else
			{
				// last ray was to right
				if ((abX*rayX + abZ*rayZ) < 0)
				{
					unX = -abZ;
					unZ = abX;
				}
				else
				{
					unX = abZ;
					unZ = -abX;
				}

				invMag = 1/magAb;
				unX *= invMag;
				unZ *= invMag;
			}

			apX = pvCurrentPos->x - pA->m_vCorner.x;
			apZ = pvCurrentPos->z - pB->m_vCorner.z;

			if ((abX*unX + abZ*unZ) < 0)
			{
				goToCorner = TRUE;
			}
			else
			{
				magAp = sqrt(apX*apX + apZ*apZ);
				if (magAp == 0)
				{
					goToCorner = TRUE;
				}
				else
				{
					cornerCosTheta = magAb/(2*radius);
					pointCosTheta = (abX*apX + abZ*apZ)/(magAb*magAp);

					goToCorner = (pointCosTheta > cornerCosTheta);
				}
			}

#ifdef BACKOFF_PRINTS
			rmonPrintf("double corner -- go to corner:%d\n", goToCorner);
#endif

			if (goToCorner)
			{
				backOffMag = sqrt((0.5*magAb)*(0.5*magAb) + radiusSquared);

				vDesired.x = 0.5*(pA->m_vCorner.x + pB->m_vCorner.x) + backOffMag*unX;
				vDesired.y = pvCurrentPos->y;
				vDesired.z = 0.5*(pA->m_vCorner.z + pB->m_vCorner.z) + backOffMag*unZ;

				vReflectNormal.x = unX;
				vReflectNormal.y = 0;
				vReflectNormal.z = unZ;

				collision = TRUE;
			}
			else
			{
				if (pClosestCorner && (closestDistanceSquared < radiusSquared))
				{
					closestDistance = sqrt(closestDistanceSquared);
					if (closestDistance != 0)
					{
						backOffMag = radius - closestDistance;

						invClosestDistance = 1/closestDistance;
						vBackOffDelta.x = backOffMag*mdx*invClosestDistance;
						vBackOffDelta.y = 0;
						vBackOffDelta.z = backOffMag*mdz*invClosestDistance;

						CVector3__Add(&vDesired, pvCurrentPos, &vBackOffDelta);

						vReflectNormal.x = mdx*invClosestDistance;
						vReflectNormal.y = 0;
						vReflectNormal.z = mdz*invClosestDistance;

						collision = TRUE;
					}
				}
			}
		}
		else if (pClosestCorner && (closestDistanceSquared < radiusSquared))
		{
#ifdef BACKOFF_PRINTS
			rmonPrintf("single corner\n");
#endif

			closestDistance = sqrt(closestDistanceSquared);
			if (closestDistance != 0)
			{
				backOffMag = radius - closestDistance;

				invClosestDistance = 1/closestDistance;
				vBackOffDelta.x = backOffMag*mdx*invClosestDistance;
				vBackOffDelta.y = 0;
				vBackOffDelta.z = backOffMag*mdz*invClosestDistance;

				CVector3__Add(&vDesired, pvCurrentPos, &vBackOffDelta);

				vReflectNormal.x = mdx*invClosestDistance;
				vReflectNormal.y = 0;
				vReflectNormal.z = mdz*invClosestDistance;

				collision = TRUE;
			}
		}
	}

	if (collision)
	{
		pThis->m_vVelocity.x = pThis->m_vVelocity.y = pThis->m_vVelocity.z = 0;
		vDesired.y = pvCurrentPos->y;

		ciTemp = *pCI;
		ciTemp.m_dwFlags &= ~COLLISIONFLAG_USEWALLRADIUS;

		CAnimInstanceHdr__Collision(pThis, ppCurrentRegion, pvCurrentPos, vDesired, &ciTemp, ClimbUp, ClimbDown, TRUE, TRUE);

		updateVelocity = (pCI->m_dwFlags & COLLISIONFLAG_UPDATEVELOCITY);

		if (updateVelocity)
		{
			if (		(pCI->m_WallBehavior == WALL_BOUNCE)
					|| ((pCI->m_WallBehavior == WALL_SLIDEBOUNCE) && !startedOnGround) )
			{
				pThis->m_vVelocity = CVector3__Reflect(&vVelocity, &vReflectNormal, pCI->m_BounceReturnEnergy);
			}
			else if (pCI->m_WallBehavior == WALL_STICK)
			{
				pThis->m_vVelocity.x = pThis->m_vVelocity.y = pThis->m_vVelocity.z = 0;
			}
			else
			{
				updateVelocity = FALSE;
			}
		}

		if (!updateVelocity)
			pThis->m_vVelocity = vVelocity;
	}
	else
	{
		pThis->m_vVelocity = vVelocity;
	}

	if (ignoreDead)
		if (!prevIgnoreDead)
			ci_movetest.m_dwFlags &= ~COLLISIONFLAG_IGNOREDEAD;

	return collision;
}
*/

void CAnimInstanceHdr__Teleport(CAnimInstanceHdr *pThis, float X, float Z)
{
	CVector3		vVelocity,
					vDesired;
	float			groundHeight, prevHeight;
	BOOL			avoidWater = (pThis->ih.m_pEA->m_dwTypeFlags & AI_TYPE_TELEPORTAVOIDWATER);
	BOOL			avoidCliffs = (pThis->ih.m_pEA->m_dwTypeFlags & AI_TYPE_TELEPORTAVOIDCLIFFS);
	DWORD			dwPrevFlags;

	groundHeight = CGameRegion__GetGroundHeight(pThis->ih.m_pCurrentRegion, pThis->ih.m_vPos.x, pThis->ih.m_vPos.z);
	prevHeight = pThis->ih.m_vPos.y - groundHeight;

	vVelocity = pThis->m_vVelocity;
	pThis->m_vVelocity.x = pThis->m_vVelocity.y = pThis->m_vVelocity.z = 0;
	vDesired.x = X;
	vDesired.y = pThis->ih.m_vPos.y;
	vDesired.z = Z;


	dwPrevFlags = ci_movetest.dwFlags;

	if (avoidWater)
		ci_movetest.dwFlags &= ~COLLISIONFLAG_ENTERWATER;
	else
		ci_movetest.dwFlags |= COLLISIONFLAG_ENTERWATER;

	if (avoidCliffs)
		ci_movetest.dwFlags &= ~(COLLISIONFLAG_CLIMBUP | COLLISIONFLAG_CLIMBDOWN);
	else
		ci_movetest.dwFlags |= (COLLISIONFLAG_CLIMBUP | COLLISIONFLAG_CLIMBDOWN);

	CAnimInstanceHdr__Collision3(pThis, vDesired, &ci_movetest);

	ci_movetest.dwFlags = dwPrevFlags;

	pThis->m_vVelocity = vVelocity;

	groundHeight = CGameRegion__GetGroundHeight(pThis->ih.m_pCurrentRegion, pThis->ih.m_vPos.x, pThis->ih.m_vPos.z);
	pThis->ih.m_vPos.y = groundHeight + prevHeight;
}

/*
// returns TRUE if vPos is within MaxDist
// and there is a region edge between the instance and vPos
BOOL CAnimInstanceHdr__IsObstructed(CAnimInstanceHdr *pThis,
												CVector3 vPos,
												enum EMoveTestType moveTestType)
{
	CGameRegion		*pRegion;
	CVector3			vCurrentPos;
	BOOL				climbUp, climbDown, EnterWater, ExitWater;


	pRegion = pThis->ih.m_pCurrentRegion;
	vCurrentPos = pThis->ih.m_vPos;

	// can this ai enter water or not ?
	if (pThis->ih.m_pEA->m_dwTypeFlags & AI_TYPE_AVOIDWATER)
		EnterWater = FALSE;
	else
		EnterWater = TRUE;

	// can this ai exit the water ?
	if (pThis->ih.m_pEA->m_dwTypeFlags2 & AI_TYPE2_STAYINWATER)
		ExitWater = FALSE;
	else
		ExitWater = TRUE;


//	if ( ((CGameObjectInstance *)pThis) != CEngineApp__GetPlayer(GetApp()) )
//	{
//		rmonPrintf("Enter Water = %d\n", EnterWater);
//		ASSERT(EnterWater);
//	}
//	else
//	{
//		rmonPrintf("player\n");
//	}



	if (pThis->ih.m_pEA->m_dwTypeFlags & AI_TYPE_CLIMBER)
	{
		climbUp = TRUE;
		climbDown = TRUE;
	}
	else
	{
		switch (moveTestType)
		{
			case MOVETEST_LOOKING:
				climbUp = TRUE;
				climbDown = TRUE;
				break;

			case MOVETEST_AVOIDANCE:
				climbUp = FALSE;
				climbDown = FALSE;
				break;

			case MOVETEST_MOVEMENT:
				climbUp = FALSE;
				climbDown = TRUE;
				break;

			default:
				ASSERT(FALSE);
		}
	}

	return CAnimInstanceHdr__Collision(pThis, &pRegion, &vCurrentPos, vPos, &ci_movetest, climbUp, climbDown, EnterWater, ExitWater);
}
*/

/*
BOOL CAnimInstanceHdr__IsObstructed(CAnimInstanceHdr *pThis, CVector3 vDesired, CCollisionInfo2 **ppCIUsed)
{
	CCollisionInfo2	*pCI;
	CAnimInstanceHdr	animInst;
	BOOL					climber;

	climber = pThis->ih.m_pEA->m_dwTypeFlags & AI_TYPE_CLIMBER;

	if (pThis->ih.m_pEA->m_dwTypeFlags & AI_TYPE_AVOIDWATER)
	{
		if (climber)
			pCI = &ci_climbing_movetest;
		else
			pCI = &ci_movetest;
	}
	else if (pThis->ih.m_pEA->m_dwTypeFlags & AI_TYPE2_STAYINWATER)
	{
		pCI = &ci_swimming_movetest;
	}
	else
	{
		pCI = &ci_climbing_swimming_movetest;
	}

	if (ppCIUsed)
		*ppCIUsed = pCI;

	animInst = *pThis;

	return CAnimInstanceHdr__Collision3(&animInst, vDesired, pCI);
}
*/

BOOL CAnimInstanceHdr__IsObstructed(CAnimInstanceHdr *pThis, CVector3 vDesired, CCollisionInfo2 **ppCIUsed /*=NULL*/)
{
	CCollisionInfo2	*pCI;
	CAnimInstanceHdr	ah;
	BOOL					climber;
	int					collisions;

	if (pThis == &CEngineApp__GetPlayer(GetApp())->ah)
	{
		pCI = &ci_movetest;
	}
	else
	{
		climber = pThis->ih.m_pEA->m_dwTypeFlags & AI_TYPE_CLIMBER;

		if (pThis->ih.m_pEA->m_dwTypeFlags & AI_TYPE_AVOIDWATER)
		{
			if (climber)
				pCI = &ci_climbing_movetest;
			else
				pCI = &ci_movetest;
		}
		else if (pThis->ih.m_pEA->m_dwTypeFlags & AI_TYPE2_STAYINWATER)
		{
			pCI = &ci_swimming_movetest;
		}
		else
		{
			pCI = &ci_climbing_swimming_movetest;
		}
	}

	if (ppCIUsed)
		*ppCIUsed = pCI;

	// save pThis data
	ah = *pThis;
	collisions = CAnimInstanceHdr__Collision3(pThis, vDesired, pCI);
	// restore pThis data
	*pThis = ah;

	return collisions;
}

// use for vision
BOOL CAnimInstanceHdr__CastYeThyRay(CAnimInstanceHdr *pThis, CInstanceHdr *pTarget)
{
	CAnimInstanceHdr	ah;
	CVector3				vDesired;
	int					collisions;

	ASSERT(cache_is_valid);

	ah = *pThis;

	pThis->ih.m_vPos.y += 0.8*pThis->ih.m_pEA->m_CollisionHeight;

	vDesired = pTarget->m_vPos;
	vDesired.y += 0.8*pTarget->m_pEA->m_CollisionHeight;

	collisions = CAnimInstanceHdr__Collision3(pThis, vDesired, &ci_raycast);
	
	*pThis = ah;
	
	return !collisions;
}

/*
void CGameObjectInstance__GetNearPositionAndRegion(CGameObjectInstance *pThis,
																	CVector3 vDesiredPos,
																   CVector3 *pvOutPos, CGameRegion **ppOutRegion)
{
	CVector3		vOldVelocity;

	ASSERT(pvOutPos);
	ASSERT(ppOutRegion);

	*ppOutRegion	= pThis->ah.ih.m_pCurrentRegion;
	*pvOutPos		= pThis->ah.ih.m_vPos;

	// don't let velocity affect offset location
	vOldVelocity = pThis->ah.m_vVelocity;
	pThis->ah.m_vVelocity.x = 0;
	pThis->ah.m_vVelocity.y = 0;
	pThis->ah.m_vVelocity.z = 0;

	CAnimInstanceHdr__Collision(&pThis->ah, ppOutRegion, pvOutPos, vDesiredPos, &ci_movetest, FALSE, TRUE);
	pvOutPos->y = vDesiredPos.y;

	pThis->ah.m_vVelocity = vOldVelocity;
}
*/

void CGameObjectInstance__GetOffsetPositionAndRegion(CGameObjectInstance *pThis,
																	  CVector3 vDesiredOffset,
																	  CVector3 *pvOutPos, CGameRegion **ppOutRegion)
{
	CVector3		vRotatedOffset,
					vDesiredPos;
	//float			groundHeight;

	ASSERT(pvOutPos);
	ASSERT(ppOutRegion);

	if (pThis == CEngineApp__GetPlayer(GetApp()))
	{
		vDesiredOffset.x = -vDesiredOffset.x;
		vDesiredOffset.z = -vDesiredOffset.z;

		CMtxF__VectorMult(GetApp()->m_mfViewOrient, &vDesiredOffset, &vDesiredPos);

		//groundHeight = CInstanceHdr__GetGroundHeight(&pThis->ah.ih);

		//vDesiredPos.y -= groundHeight;
	}
	else
	{
		vRotatedOffset = CGameObjectInstance__GetRotatedDirection(pThis, vDesiredOffset);
		CVector3__Add(&vDesiredPos, &pThis->ah.ih.m_vPos, &vRotatedOffset);
	}

	CInstanceHdr__GetNearPositionAndRegion(&pThis->ah.ih,
														vDesiredPos,
														pvOutPos, ppOutRegion,
														INTERSECT_BEHAVIOR_STICK, INTERSECT_BEHAVIOR_STICK);
}

CQuatern CGameSimpleInstance__GetAimRotation(CGameSimpleInstance *pThis)
{
	return CInstanceHdr__GetGroundRotation(&pThis->ah.ih);
}

CQuatern CGameObjectInstance__GetAimRotation(CGameObjectInstance *pThis)
{
	CEngineApp	*pApp;
	CQuatern		qRot, qRotY;

	pApp = GetApp();

	if (pThis == CEngineApp__GetPlayer(pApp))
	{
		CQuatern__BuildFromAxisAngle(&qRotY, 0,1,0, ANGLE_PI);
		CQuatern__Mult(&qRot, &qRotY, &pApp->m_qViewAlign);
	}
	else
	{
		CQuatern__BuildFromAxisAngle(&qRotY, 0,1,0, pThis->m_RotY + ANGLE_PI);
		CQuatern__Mult(&qRot, &qRotY, &pThis->m_qGround);
	}

	return qRot;
}

CQuatern CGameObjectInstance__GetAutoAimRotation(CGameObjectInstance *pThis, CVector3 vPos, CQuatern *pqFixResult, float Angle)
{
	int			cInst, nInsts;
	float			minDot, dot, theta, u;
	CVector3		vDelta, vForward, vAim, vAxis, vChosenDelta, vTarget;
	CQuatern		qAim, qFix, qResult;
	CMtxF			mfAim;
	CGameObjectInstance	*pChosenOne,
								*pTarget;

	vForward.x = 0;
	vForward.y = 0;
	vForward.z = 1;


	qAim = CGameObjectInstance__GetAimRotation(pThis);

	if (Angle == 0)
	{
		if (pqFixResult)
			CQuatern__Identity(pqFixResult);

		return qAim;
	}

	CQuatern__ToMatrix(&qAim, mfAim);
	CMtxF__VectorMult(mfAim, &vForward, &vAim);

	minDot = cos(ANGLE_DTOR(Angle/2));

	pChosenOne = NULL;

	nInsts = GetApp()->m_Scene.m_nActiveAnimInstances;
	for (cInst=0; cInst<nInsts; cInst++)
	{
		pTarget = GetApp()->m_Scene.m_pActiveAnimInstances[cInst];
		if (		(pTarget != pThis)
				&&	(!CGameObjectInstance__IsDevice(pTarget))
				&& (pTarget->m_AI.m_Health) )
		{
			vTarget = pTarget->ah.ih.m_vPos;
			vTarget.y += pTarget->ah.ih.m_CollisionHeightOffset + (pTarget->ah.ih.m_pEA->m_CollisionHeight*0.72);

			CVector3__Subtract(&vDelta, &vTarget, &vPos);
			CVector3__Normalize(&vDelta);

			dot = CVector3__Dot(&vAim, &vDelta);
			if (dot > minDot)
			{
				minDot = dot;
				vChosenDelta = vDelta;
				pChosenOne = pTarget;
			}
		}
	}

	if (pChosenOne)
	{
		CVector3__Cross(&vAxis, &vAim, &vChosenDelta);
		CVector3__Normalize(&vAxis);

		theta = acos(minDot);
		u = 1 - theta/(ANGLE_DTOR(Angle/2));
		u = sqrt(sqrt(u));
		theta *= u;

		CQuatern__BuildFromAxisAngle(&qFix, vAxis.x, vAxis.y, vAxis.z, theta);
		CQuatern__Mult(&qResult, &qAim, &qFix);

		if (pqFixResult)
			*pqFixResult = qFix;

		return qResult;
	}
	else
	{
		if (pqFixResult)
			CQuatern__Identity(pqFixResult);

		return qAim;
	}
}

CVector3 CGameObjectInstance__GetRotatedDirection(CGameObjectInstance *pThis, CVector3 vDirection)
{
	CQuatern		qAim;
	CMtxF			mfRot;
	CVector3		vRotatedDirection;

	qAim = CGameObjectInstance__GetAimRotation(pThis);
	CQuatern__ToMatrix(&qAim, mfRot);

	CMtxF__VectorMult(mfRot, &vDirection, &vRotatedDirection);

	return vRotatedDirection;
}

// CInstanceHdr implementation
/////////////////////////////////////////////////////////////////////////////

CQuatern CInstanceHdr__GetAimAtTarget(CInstanceHdr *pThis, CVector3 vPos)
{
	CEngineApp				*pApp;
	CGameObjectInstance	*pPlayer;
	float						dot, theta;
	CVector3					vPlayerPos,
								vDelta, vZ, vZi, vAxis;
	CQuatern					qAim, qRot, qFinal;
	CMtxF						mfRot;
	CGameObjectInstance	*pAnim;

	if (pThis)
	{
		if (pThis->m_Type == I_ANIMATED)
		{
			pAnim = (CGameObjectInstance*) pThis;

			qRot = CGameObjectInstance__GetAimRotation(pAnim);
			CQuatern__ToMatrix(&qRot, mfRot);
			vZi.x = 0;
			vZi.y = 0;
			vZi.z = 1;
			CMtxF__VectorMult(mfRot, &vZi, &vZ);
		}
		else
		{
			vZ.x = 0;
			vZ.y = 0;
			vZ.z = 1;
		}

		pApp = GetApp();
		if (pThis->m_Type == I_ANIMATED)
		{
			pPlayer = ((CGameObjectInstance*)pThis)->m_pAttackerCGameObjectInstance;
			if (!pPlayer)
				pPlayer = CEngineApp__GetPlayer(pApp);
		}
		else
		{
			pPlayer = CEngineApp__GetPlayer(pApp);
		}
	}
	else
	{
		vZ.x = 0;
		vZ.y = 0;
		vZ.z = 1;

		pPlayer = CEngineApp__GetPlayer(pApp);
	}

	if (!pPlayer)
	{
		CQuatern__Identity(&qFinal);
		return qFinal;
	}

	// find aiming rotation
	vPlayerPos = pPlayer->ah.ih.m_vPos;
	vPlayerPos.y += (TUROK_HEIGHT/2)*SCALING_FACTOR;

	CVector3__Subtract(&vDelta, &vPlayerPos, &vPos);
	CVector3__Normalize(&vDelta);

	CVector3__Cross(&vAxis, &vZ, &vDelta);
	CVector3__Normalize(&vAxis);

	dot = CVector3__Dot(&vZ, &vDelta);
	theta = acos(dot);

	CQuatern__BuildFromAxisAngle(&qAim, vAxis.x, vAxis.y, vAxis.z, theta);

	if (pThis->m_Type == I_ANIMATED)
		CQuatern__Mult(&qFinal, &qRot, &qAim);
	else
		qFinal = qAim;

	return qFinal;
}

BOOL CInstanceHdr__IsOnGround(CInstanceHdr *pThis)
{
	float						groundHeight;
	CGameObjectInstance	*pAnim;

	if (pThis->m_Type == I_ANIMATED)
	{
		pAnim = (CGameObjectInstance*) pThis;
		if (pAnim->m_dwFlags & ANIM_INSTANCE_HANGINGON)
			return TRUE;
	}

	groundHeight = CInstanceHdr__GetGroundHeight(pThis);
	return ((pThis->m_vPos.y - groundHeight) <= (0.05*SCALING_FACTOR));
}

BOOL CInstanceHdr__IsOnGroundWhenClimbing(CInstanceHdr *pThis)
{
	float						groundHeight;
	CGameObjectInstance	*pAnim;

	if (pThis->m_Type == I_ANIMATED)
	{
		pAnim = (CGameObjectInstance*) pThis;
		if (pAnim->m_dwFlags & ANIM_INSTANCE_HANGINGON)
			return TRUE;
	}

	groundHeight = CInstanceHdr__GetGroundHeight(pThis);
	return ((pThis->m_vPos.y - groundHeight) <= 0.4);
}

int CInstanceHdr__TypeFlag(CInstanceHdr *pThis)
{
	if (pThis->m_Type == I_ANIMATED)
		return CGameObjectInstance__TypeFlag((CGameObjectInstance*) pThis);
	else
		return CScene__GetObjectTypeFlag(&GetApp()->m_Scene, pThis->m_nObjType);
}

BOOL CInstanceHdr__IsDevice(CInstanceHdr *pThis)
{
	ASSERT(pThis);

	// all devices are animated
	if (pThis->m_Type == I_ANIMATED)
		return CGameObjectInstance__IsDevice((CGameObjectInstance*) pThis);
	else
		return FALSE;
}

CQuatern CInstanceHdr__GetGroundRotation(CInstanceHdr *pThis)
{
	CVector3 				vNormal,
								vUp,
								vAxis;
	float						cosTheta, theta,
								sinRotY, cosRotY;
	CQuatern					qGround, qGroundXOnly, qBlend;
	CGameObjectInstance	*pPlayer;

	if (pThis->m_pCurrentRegion && !CInstanceHdr__IsDevice(pThis))
	{
		pPlayer = CEngineApp__GetPlayer(GetApp());

		if (pThis == &pPlayer->ah.ih)
		{
			if (CGameRegion__IsCliff(pThis->m_pCurrentRegion))
			{
				CQuatern__Identity(&qGround);
			}
			else
			{
				vNormal = CGameRegion__GetGroundUnitNormal(pThis->m_pCurrentRegion);
				vUp.x = 0;
				vUp.y = 1;
				vUp.z = 0;
				CVector3__Cross(&vAxis, &vUp, &vNormal);
				CVector3__Normalize(&vAxis);
				cosTheta = CVector3__Dot(&vUp, &vNormal);
				theta = acos(cosTheta);
				CQuatern__BuildFromAxisAngle(&qGround, vAxis.x, vAxis.y, vAxis.z, min(ANGLE_DTOR(GR_MAX_ZTILT), theta)*GR_TOTAL_TILT_FACTOR);


				sinRotY = sin(pPlayer->m_RotY);
				cosRotY = cos(pPlayer->m_RotY);

				theta = CGameRegion__GetHillAngle(pThis->m_pCurrentRegion,
															 pThis->m_vPos.x, pThis->m_vPos.z,
															 pThis->m_vPos.x - sinRotY, pThis->m_vPos.z - cosRotY);

				if (!CGameRegion__IsCliff(pThis->m_pCurrentRegion))
					theta = min(GR_MAX_XTILT, theta)*GR_TOTAL_TILT_FACTOR;

				CQuatern__BuildFromAxisAngle(&qGroundXOnly, cosRotY,0,-sinRotY, theta);

				CQuatern__Blend(&qBlend, GR_ZTILT_FACTOR, &qGroundXOnly, &qGround);
				qGround = qBlend;
			}
		}
		else
		{
			vNormal = CGameRegion__GetGroundUnitNormal(pThis->m_pCurrentRegion);

			vUp.x = 0;
			vUp.y = 1;
			vUp.z = 0;

			CVector3__Cross(&vAxis, &vUp, &vNormal);
			CVector3__Normalize(&vAxis);

			cosTheta = CVector3__Dot(&vUp, &vNormal);

			theta = acos(cosTheta);

			CQuatern__BuildFromAxisAngle(&qGround, vAxis.x, vAxis.y, vAxis.z, theta);
		}
	}
	else
	{
		CQuatern__Identity(&qGround);
	}

	return qGround;
}

void CInstanceHdr__GetNearPositionAndRegion(CInstanceHdr *pThis,
														  CVector3 vDesiredPos,
														  CVector3 *pvOutPos, CGameRegion **ppOutRegion,
														  int GroundBehavior, int InstanceBehavior)
{
	CInstanceHdr		ih;
	CVector3				vPos;
	CGameRegion			*pRegion;
	int					prevGround, prevInstance;

	ASSERT(pvOutPos);
	ASSERT(ppOutRegion);

	// save pThis data
	ih = *pThis;

	prevGround = ci_nearregiontest.GroundBehavior;
	ci_nearregiontest.GroundBehavior = GroundBehavior;

	prevInstance = ci_nearregiontest.InstanceBehavior;
	ci_nearregiontest.InstanceBehavior = InstanceBehavior;

	// make sure that COLLISIONFLAG_PHYSICS isn't set
	ASSERT(!(ci_nearregiontest.dwFlags & COLLISIONFLAG_PHYSICS));
	// make sure m_vVelocity is only difference between CAnimInstanceHdr and CInstanceHdr
	ASSERT((sizeof(CAnimInstanceHdr) - sizeof(CInstanceHdr)) == sizeof(CVector3));
	
	// Using pThis so no collision with source flag will work correctly.
	// It's okay to caste a CInstanceHdr to a CAnimInstanceHdr only because
	// m_vVelocity won't be used when COLLISIONFLAG_PHYSICS isn't checked
	CAnimInstanceHdr__Collision3((CAnimInstanceHdr*) pThis, vDesiredPos, &ci_nearregiontest);
	
	pThis->m_vPos.y = vDesiredPos.y;

	// tempory variables allow pvOutPos and ppOutRegion to be same as pThis
	vPos.x = pThis->m_vPos.x;
	vPos.y = vDesiredPos.y;
	vPos.z = pThis->m_vPos.z;
	pRegion = pThis->m_pCurrentRegion;

	// restore pThis data
	*pThis = ih;

	*pvOutPos = vPos;
	*ppOutRegion = pRegion;

	ci_nearregiontest.GroundBehavior = prevGround;
	ci_nearregiontest.InstanceBehavior = prevInstance;
}

float CInstanceHdr__GetCliffDistance(CInstanceHdr *pThis, BOOL ToTop, float TestDistance)
{
	CAnimInstanceHdr	animInst;
	CVector3				vNormal,
							vDesired,
							vDelta;
	float					dist;

	ASSERT(pThis->m_pCurrentRegion);		// can't be climbing if not on cliff region
	ASSERT(CGameRegion__IsCliff(pThis->m_pCurrentRegion));

	vNormal = CGameRegion__GetGroundNormal(pThis->m_pCurrentRegion);
	vNormal.y = 0;
	CVector3__Normalize(&vNormal);

	CVector3__MultScaler(&vNormal, &vNormal, ToTop ? -TestDistance : TestDistance);
	CVector3__Add(&vDesired, &pThis->m_vPos, &vNormal);

	animInst.ih = *pThis;

	if (CAnimInstanceHdr__Collision3(&animInst, vDesired, &ci_clifftest))
	{
		animInst.ih.m_vPos.y = CInstanceHdr__GetGroundHeight(&animInst.ih);
		CVector3__Subtract(&vDelta, &pThis->m_vPos, &animInst.ih.m_vPos);
		dist = CVector3__Magnitude(&vDelta);
	}
	else
	{
		dist = TestDistance;
	}

	return dist;
}

/*
CInstanceHdr* CAnimInstanceHdr__InstanceCollision(CAnimInstanceHdr *pThis,
																  CGameRegion *pCurrentRegion,
																  CVector3 *pvCurrentPos, CVector3 vDesiredPos,
																  CCollisionInfo *pCI)
{
	CEngineApp				*pApp;
	int						cInst, nInsts;
	float						r1, r2, rt,
								startMinX, startMaxX,
								startMinZ, startMaxZ,
								endMinX, endMaxX,
								endMinZ, endMaxZ,
								minX, maxX,
								minZ, maxZ,
								mx, mz,
								magM,
								scaleM,
								minSMagC, sMagC,
								ix, iz,
								iMinX, iMaxX,
								iMinZ, iMaxZ,
								dix, diz,
								diDotm,
								px, pz,
								fx, fz,
								magRSquared, magR,
								magP, scaleP,
								cx, cz,
								rtx, rtz,
								lftx, lftz,
								prtx, prtz,
								magPrt,
								magMmagPrtSquared,
								remainFactorG, remainFactorNG, remainFactor,
								lx, lz,
								finalX, finalZ,
								thisHeight, instHeight,
								thisTall, instTall;
	CInstanceHdr			*pCollisionInstance, *pInst;
	CGameObjectInstance	*pAlive;
	CVector3					vImpactPos;
	BOOL						doCollision;

	pApp = GetApp();

	nInsts = CEngineApp__GetAllInstanceCount(pApp);

	if ((pCI->m_InstanceBehavior == INSTANCE_NOCOLLISION) || !nInsts)
	{
		*pvCurrentPos = vDesiredPos;
		return NULL;
	}

	r1 = pThis->ih.m_pEA->m_CollisionRadius;
	thisTall = pThis->ih.m_pEA->m_CollisionHeight;

	// start bounds
	startMinX = pvCurrentPos->x - r1;
	startMaxX = pvCurrentPos->x + r1;
	startMinZ = pvCurrentPos->z - r1;
	startMaxZ = pvCurrentPos->z + r1;

	// end bounds
	endMinX = vDesiredPos.x - r1;
	endMaxX = vDesiredPos.x + r1;
	endMinZ = vDesiredPos.z - r1;
	endMaxZ = vDesiredPos.z + r1;

	// movement bounds
	minX = min(startMinX, endMinX);
	maxX = max(startMaxX, endMaxX);
	minZ = min(startMinZ, endMinZ);
	maxZ = max(startMaxZ, endMaxZ);

	// calculate constants for collision placement
	mx = vDesiredPos.x - pvCurrentPos->x;
	mz = vDesiredPos.z - pvCurrentPos->z;

	magM = sqrt(mx*mx + mz*mz);
	if (magM == 0)
	{
		// skip collision if no movement

		pvCurrentPos->y = vDesiredPos.y;
		return NULL;
	}

	// find closest collision
	minSMagC = magM;
	pCollisionInstance = NULL;
	for (cInst=0; cInst<nInsts; cInst++)
	{
		pInst = CEngineApp__GetInstance(pApp, cInst);

		pAlive = NULL;
		doCollision = TRUE;

		// don't collide with self
		if (pInst == &pThis->ih)
		{
			doCollision = FALSE;
		}

		// keep certain particles from colliding with their source
		if (pThis->ih.m_Type == I_PARTICLE)
		{
			if (		(pInst == ((CParticle*) pThis)->m_pSource)
					&& (((CParticle*) pThis)->m_pEffect->m_dwFlags & PARTICLE_BEHAVIOR_CANNOTHITSOURCE) )
			{
				doCollision = FALSE;
			}

			// don't allow shooting of invisible types
			if (pInst->m_Type == I_ANIMATED)
			{
				pAlive = (CGameObjectInstance*) pInst;

				if (		(AI_GetDyn(pAlive)->m_cRegenerateAppearance > 0.0)
						||	(pAlive->m_cMelt > 0.0) )
				{
					doCollision = FALSE;
				}
			}
		}

		switch (pInst->m_Type)
		{
			case I_ANIMATED:
				pAlive = (CGameObjectInstance*) pInst;
				if (pAlive->m_AI.m_dwStatusFlags & AI_EXPTARGETEXPLODED)
				{
					doCollision = FALSE;
				}
				else if (pCI->m_dwFlags & COLLISIONFLAG_IGNOREDEAD)
				{
					// some things don't collide with dead stuff
					// (mainly the player and particles)
					if (pAlive->m_AI.m_Health == 0)
					{
						doCollision = FALSE;
					}
				}
				else if (pAlive->m_cMelt != 0.0)
				{
					// don't collide with stuff that's melting
					doCollision = FALSE;
				}
				break;

			case I_SIMPLE:
				if (!(pCI->m_dwFlags & COLLISIONFLAG_PICKUPCOLLISION))
				{
					// only the player should be affected by pickups
					doCollision = FALSE;
				}
				break;

			case I_PARTICLE:
				// this shouldn't happen because particles are never added
				// to the collision list
				ASSERT(FALSE);
		}

		if (doCollision)
		{
			r2 = pInst->m_pEA->m_CollisionRadius;
			rt = r1 + r2;

			// instance position
			ix = pInst->m_vPos.x;
			iz = pInst->m_vPos.z;

			// rough collision check

			// bounding box comparison
			iMinX = ix - r2;
			iMaxX = ix + r2;
			iMinZ = iz - r2;
			iMaxZ = iz + r2;

			if ((maxX > iMinX) && (minX < iMaxX) && (maxZ > iMinZ) && (minZ < iMaxZ))
			{
				if (pInst->m_Type == I_SIMPLE)
				{
					// walk right through pickups that aren't applicable
					doCollision = CPickup__CanInstancePickItUp((CGameSimpleInstance*) pInst, pThis);
				}

				if (doCollision)
				{
					// A lot of these letters are from a diagram that I erased off
					// my whiteboard ages ago.  Oops.  Give me a ring and we'll figure it out.

					dix = ix - pvCurrentPos->x;
					diz = iz - pvCurrentPos->z;

					diDotm = dix*mx + diz*mz;

					// skip instances behind direction of movement
					if (diDotm > 0)
					{
						// exact collision check

						scaleM = diDotm/(magM*magM);

						px = mx*scaleM;
						pz = mz*scaleM;

						fx = dix - px;
						fz = diz - pz;

						magRSquared = rt*rt - (fx*fx + fz*fz);

						// no collision if (magRSquared <= 0)
						if (magRSquared > 0)
						{
							magR = sqrt(magRSquared);
							magP = sqrt(px*px + pz*pz);

							// should always be true when (magM != 0) and (diDotM > 0)
							if (magP > 0)
							{
								sMagC = max(0, magP - magR);
								if (sMagC < minSMagC)
								{
									scaleP = sMagC/magP;

									cx = px*scaleP;
									cz = pz*scaleP;

									vImpactPos.x = pvCurrentPos->x + cx;
									vImpactPos.y = pvCurrentPos->y + (sMagC/magM)*(vDesiredPos.y - pvCurrentPos->y);
									vImpactPos.z = pvCurrentPos->z + cz;

									instTall = pInst->m_pEA->m_CollisionHeight;
									if (pAlive)
										if (pAlive->m_AI.m_Health <= 0)
											instTall = pInst->m_pEA->m_DeadHeight;

									// height offset comes from from animation

									// animation translations are absolute, so the root can move up and down
									// from the instance height

									thisHeight = vImpactPos.y + pThis->ih.m_CollisionHeightOffset;
									instHeight = pInst->m_vPos.y + pInst->m_CollisionHeightOffset;

									if ( (instHeight <= (thisHeight + thisTall)) && (thisHeight <= (instHeight + instTall)) )
									{
										minSMagC = sMagC;

										pCollisionInstance = pInst;

										// slide projects motion onto tangent of intersecion on circle
										// grease aims motion alogin tangent with same amplitude
										// slide actually uses a blend between the two because it feels better
										if ((pCI->m_InstanceBehavior == INSTANCE_SLIDE) || (pCI->m_InstanceBehavior == INSTANCE_GREASE))
										{
											rtx = dix - cx;
											rtz = diz - cz;

											// decide which way to slide

											// left vector
											lftx = -mz;
											lftz = mx;

											if ((lftx*dix + lftz*diz) >= 0)
											{
												// on left size, so slide right
												prtx = rtz;
												prtz = -rtx;
											}
											else
											{
												// on right side, so slide left
												prtx = -rtz;
												prtz = rtx;
											}

											magPrt = sqrt(prtx*prtx + prtz*prtz);
											magMmagPrtSquared = magM*(prtx*prtx + prtz*prtz);
											if ((magPrt == 0) || (magMmagPrtSquared == 0))
											{
												lx = lz = 0;
											}
											else
											{
												remainFactorG = (magM - sMagC)/magPrt;
												remainFactorNG = ((magM - sMagC)*(mx*prtx + mz*prtz))/magMmagPrtSquared;

												if (pCI->m_InstanceBehavior == INSTANCE_GREASE)
													remainFactor = remainFactorG;
												else
													remainFactor = BlendFLOAT(COLLISION_GREASE_FACTOR, remainFactorNG, remainFactorG);

												lx = prtx*remainFactor;
												lz = prtz*remainFactor;
											}

											finalX = vImpactPos.x + lx;
											finalZ = vImpactPos.z + lz;
										}
										else
										{
											finalX = vImpactPos.x;
											finalZ = vImpactPos.z;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	if (pCollisionInstance)
	{
		pvCurrentPos->x = finalX;
		pvCurrentPos->y = vDesiredPos.y;
		pvCurrentPos->z = finalZ;
		if ((pCI->m_InstanceBehavior == INSTANCE_SLIDE) || (pCI->m_InstanceBehavior == INSTANCE_GREASE))
			pvCurrentPos->y = vDesiredPos.y;
		else
			pvCurrentPos->y = vImpactPos.y;

		// update instance collision if this is the first one
		if (!pCI->m_pInstanceCollision)
		{
			pCI->m_pInstanceCollision		= pCollisionInstance;
			pCI->m_vInstanceCollisionPos	= vImpactPos;
		}
	}
	else
	{
		*pvCurrentPos = vDesiredPos;
	}

	return pCollisionInstance;
}
*/
#endif

// CROMParticleImpact implementation
/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32

void CROMParticleImpact__TakeFromParticleEffect(CROMParticleImpact *pThis, CParticleEffect *pSource)
{
	for (int cCnt=0; cCnt<PARTICLE_IMPACTS_AMT; cCnt++)
	{
		pThis->m_ImpactEventType   [cCnt] = ORDERBYTES((WORD)pSource->m_ImpactEventType   [cCnt]);
		pThis->m_ImpactEventNumber [cCnt] = ORDERBYTES(      pSource->m_ImpactEventNumber [cCnt]);
		pThis->m_ImpactParticleType[cCnt] = ORDERBYTES((WORD)pSource->m_ImpactParticleType[cCnt]);
		pThis->m_ImpactSoundType   [cCnt] = ORDERBYTES((WORD)pSource->m_ImpactSoundType   [cCnt]);
	}

#if (PARTICLE_IMPACTS_AMT & 0x1)
	pThis->m_UnusedWord = ORDERBYTES((WORD) 0);
#endif
}

#endif

// CROMParticleEffect implementation
/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32

void CROMParticleEffect__TakeFromParticleEffect(CROMParticleEffect *pThis, CParticleEffect *pSource, int nImpact)
{
	pThis->m_pImpact = (CROMParticleImpact*) ORDERBYTES((DWORD) nImpact);

	pThis->m_dwFlags					= ORDERBYTES(pSource->m_dwFlags);

	pThis->m_WhiteColor[0] = GetRValue(pSource->m_WhiteColor);
	pThis->m_WhiteColor[1] = GetGValue(pSource->m_WhiteColor);
	pThis->m_WhiteColor[2] = GetBValue(pSource->m_WhiteColor);

	pThis->m_BlackColor[0] = GetRValue(pSource->m_BlackColor);
	pThis->m_BlackColor[1] = GetGValue(pSource->m_BlackColor);
	pThis->m_BlackColor[2] = GetBValue(pSource->m_BlackColor);

	pThis->m_Priority					= (BYTE)pSource->m_Priority;
	pThis->m_FPS						= (BYTE)pSource->m_FPS;
	pThis->m_InstanceBehavior		= (BYTE)pSource->m_InstanceBehavior;
	pThis->m_WallBehavior			= (BYTE)pSource->m_WallBehavior;
	pThis->m_GroundBehavior			= (BYTE)pSource->m_GroundBehavior;
	pThis->m_Alignment				= (BYTE)pSource->m_Alignment;
	pThis->m_Playback					= (BYTE)pSource->m_Playback;
	pThis->m_Visibility				= (BYTE)pSource->m_Visibility;
	pThis->m_nFadeInDelay			= (BYTE)pSource->m_nFadeInDelay;
	pThis->m_nFadeOutDelay			= (BYTE)pSource->m_nFadeOutDelay;
	pThis->m_RandomizeHue			= pSource->m_RandomizeHue;
	pThis->m_RandomizeSaturation	= pSource->m_RandomizeSaturation;
	pThis->m_RandomizeBrightness	= pSource->m_RandomizeBrightness;

	pThis->m_nTextureSet						= ORDERBYTES((short) pSource->m_nTexture);
	pThis->m_nParticles						= ORDERBYTES((short)pSource->m_nParticles);
	pThis->m_nParticleRandom				= ORDERBYTES((short)pSource->m_nParticleRandom);
	pThis->m_nParticleFrames				= ORDERBYTES((short)pSource->m_nParticleFrames);
	pThis->m_nParticleFrameRandom			= ORDERBYTES((short)pSource->m_nParticleFrameRandom);
	pThis->m_nParticleDelayFramesRandom = ORDERBYTES((short)pSource->m_nParticleDelayFramesRandom);

	pThis->m_BounceEnergy			= ORDERBYTES(FLOAT2SF(pSource->m_BounceEnergy));
	pThis->m_DirectionSpray			= ORDERBYTES(FLOAT2SF(pSource->m_DirectionSpray));
	pThis->m_SprayX					= ORDERBYTES(FLOAT2SF(pSource->m_SprayX));
	pThis->m_SprayY					= ORDERBYTES(FLOAT2SF(pSource->m_SprayY));
	pThis->m_SprayZ					= ORDERBYTES(FLOAT2SF(pSource->m_SprayZ));
	pThis->m_DirectionX				= ORDERBYTES(FLOAT2SF(pSource->m_DirectionX));
	pThis->m_DirectionY				= ORDERBYTES(FLOAT2SF(pSource->m_DirectionY));
	pThis->m_DirectionZ				= ORDERBYTES(FLOAT2SF(pSource->m_DirectionZ));
	pThis->m_Gravity					= ORDERBYTES(FLOAT2SF(pSource->m_Gravity*SCALING_FACTOR));
	pThis->m_GravityRandom			= ORDERBYTES(FLOAT2SF(pSource->m_GravityRandom*SCALING_FACTOR));
	pThis->m_GroundFriction			= ORDERBYTES(FLOAT2SF(pSource->m_GroundFriction));
	pThis->m_FpsVelThreshold		= ORDERBYTES(FLOAT2SF(pSource->m_FpsVelThreshold*SCALING_FACTOR));
	pThis->m_AirFriction				= ORDERBYTES(FLOAT2SF(pSource->m_AirFriction));
	pThis->m_WaterFriction			= ORDERBYTES(FLOAT2SF(pSource->m_WaterFriction));
	pThis->m_Size						= ORDERBYTES(FLOAT2SF(pSource->m_Size*SCALING_FACTOR));
	pThis->m_SizeRandom				= ORDERBYTES(FLOAT2SF(pSource->m_SizeRandom*SCALING_FACTOR));
	pThis->m_Scaler					= ORDERBYTES(FLOAT2SF(pSource->m_Scaler));
	pThis->m_ScalerRandom			= ORDERBYTES(FLOAT2SF(pSource->m_ScalerRandom));
	pThis->m_Velocity					= ORDERBYTES(FLOAT2SF(pSource->m_Velocity*SCALING_FACTOR));
	pThis->m_VelocityRandom			= ORDERBYTES(FLOAT2SF(pSource->m_VelocityRandom*SCALING_FACTOR));
	pThis->m_PosRandomX				= ORDERBYTES(FLOAT2SF(pSource->m_PosRandomX*SCALING_FACTOR));
	pThis->m_PosRandomY				= ORDERBYTES(FLOAT2SF(pSource->m_PosRandomY*SCALING_FACTOR));
	pThis->m_PosRandomZ				= ORDERBYTES(FLOAT2SF(pSource->m_PosRandomZ*SCALING_FACTOR));
	pThis->m_Rotation					= ORDERBYTES(FLOAT2SF(ANGLE_DTOR(pSource->m_Rotation)));
	pThis->m_RotationRandom			= ORDERBYTES(FLOAT2SF(ANGLE_DTOR(pSource->m_RotationRandom)));
	pThis->m_RotationInc				= ORDERBYTES(FLOAT2SF(ANGLE_DTOR(pSource->m_RotationInc)));
	pThis->m_RotationIncRandom		= ORDERBYTES(FLOAT2SF(ANGLE_DTOR(pSource->m_RotationIncRandom)));
	pThis->m_RotationPivotX			= ORDERBYTES(FLOAT2SF(pSource->m_RotationPivotX*SCALING_FACTOR));
	pThis->m_RotationPivotY			= ORDERBYTES(FLOAT2SF(pSource->m_RotationPivotY*SCALING_FACTOR));
	pThis->m_PosX						= ORDERBYTES(FLOAT2SF(pSource->m_PosX*SCALING_FACTOR));
	pThis->m_PosY						= ORDERBYTES(FLOAT2SF(pSource->m_PosY*SCALING_FACTOR));
	pThis->m_PosZ						= ORDERBYTES(FLOAT2SF(pSource->m_PosZ*SCALING_FACTOR));

	pThis->unused = 0;
}

#endif


// CROMWarpPoint implementation
/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32

void CROMWarpPoint__TakeFromObjectInstance(CROMWarpPoint *pThis, CObjectInstance *pSource,
														 int nLevel, int nRegion)
{
	CVector3 vPos = pSource->m_vPos;

	if (pSource->m_pCurrentRegion)
	{
		pThis->m_nRegion = ORDERBYTES((WORD) nRegion);
		vPos.y += pSource->m_pCurrentRegion->GetGroundHeight(vPos.x, vPos.z);
	}
	else
	{
		pThis->m_nRegion	= ORDERBYTES((WORD) -1);
	}

	pThis->m_vPos = ORDERBYTES(vPos * SCALING_FACTOR);
	pThis->m_RotY = ORDERBYTES(pSource->GetYRot());
	pThis->m_nLevel = ORDERBYTES((WORD) nLevel);
}

#endif

// CROMRegionSet implementation
/////////////////////////////////////////////////////////////////////////////
#ifdef WIN32

void CROMRegionSet__TakeFromRegionSet(CROMRegionSet *pThis, CRegionSet *pSource)
{
	pThis->m_FogColor[0]			= GetRValue(pSource->m_FogColor);
	pThis->m_FogColor[1]			= GetGValue(pSource->m_FogColor);
	pThis->m_FogColor[2]			= GetBValue(pSource->m_FogColor);
	pThis->m_FogColor[3]			= 255;

	pThis->m_WaterColor[0]		= GetRValue(pSource->m_WaterColor);
	pThis->m_WaterColor[1]		= GetGValue(pSource->m_WaterColor);
	pThis->m_WaterColor[2]		= GetBValue(pSource->m_WaterColor);
	pThis->m_WaterColor[3]		= (BYTE)(pSource->m_WaterOpacity*255/100);

	DWORD fogStart					= (DWORD) max(0, min(1000, (100 - pSource->m_FogThickness)*(1000/100)));
	pThis->m_FogStart				= ORDERBYTES(fogStart);

	DWORD waterStart				= (DWORD) max(0, min(1000, (100 - pSource->m_WaterThickness)*(1000/100)));
	pThis->m_WaterStart			= ORDERBYTES(waterStart);

	pThis->m_FarClip				= ORDERBYTES(pSource->m_FarClip*SCALING_FACTOR);
	pThis->m_FieldOfView			= ORDERBYTES(pSource->m_FieldOfView);

	pThis->m_WaterFarClip		= ORDERBYTES(pSource->m_WaterFarClip*SCALING_FACTOR);
	pThis->m_WaterFieldOfView	= ORDERBYTES(pSource->m_WaterFieldOfView);

	pThis->m_WaterElevation		= ORDERBYTES(pSource->m_WaterElevation*SCALING_FACTOR);

	pThis->m_SkyElevation		= ORDERBYTES(pSource->m_SkyElevation);
	pThis->m_SkySpeed				= ORDERBYTES(pSource->m_SkySpeed);

	pThis->m_DeathHitPoints		= ORDERBYTES((WORD)pSource->m_DeathHitPoints);
	pThis->m_DeathTimeDelay		= ORDERBYTES(pSource->m_DeathTimeDelay);

	pThis->m_PressureID			= ORDERBYTES((WORD)pSource->m_PressureID);
	pThis->m_WarpID				= ORDERBYTES((WORD)pSource->m_WarpID);
	pThis->m_MusicID				= (BYTE) pSource->m_MusicID;
	pThis->m_AmbientSounds		= (BYTE) pSource->m_AmbientSounds;
	pThis->m_MapColor				= (BYTE) pSource->m_MapColor;
	pThis->m_SaveCheckpointID	= ORDERBYTES((WORD)pSource->m_SaveCheckpointID);
	pThis->m_PressurePlateSoundNumber = ORDERBYTES((WORD)pSource->m_PressurePlateSoundNumber);

	pThis->m_BlendLength			= ORDERBYTES(pSource->m_BlendLength);

	pThis->m_dwFlags				= ORDERBYTES(pSource->m_dwFlags);

	pThis->m_GroundMat			= (BYTE) pSource->m_GroundMat;
	pThis->m_WallMat				= (BYTE) pSource->m_WallMat;

	pThis->m_bActiveFlags		= (BYTE) pSource->m_dwActiveFlags;
}

#endif

// CROMSoundElement implementation
/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32

void CROMSoundElement__TakeFromElement(CROMSoundElement *pThis, CSoundElement *pSource)
{
	pThis->m_nSampleNum	= ORDERBYTES((WORD) sound_samples[pSource->m_nSample].m_nSample);
	pThis->m_nDelayTime	= ORDERBYTES((WORD) (pSource->m_StartTime*60.0));
	pThis->m_Priority		= ORDERBYTES((WORD) pSource->m_Priority);
	pThis->m_RadioVolume	= pSource->m_RadioVolume;
	pThis->m_Probability	= ORDERBYTES((WORD) pSource->m_Probability);
	pThis->m_wFlags		= ORDERBYTES((WORD)pSource->m_dwFlags);

	pThis->m_Volume.m_MaxValue			= ORDERBYTES(FLOAT2SF(pSource->m_Volume.m_MaxValue));
	pThis->m_Volume.m_StartFactor		= ORDERBYTES(FLOAT2SF(pSource->m_Volume.m_StartFactor));
	pThis->m_Volume.m_EndFactor		= ORDERBYTES(FLOAT2SF(pSource->m_Volume.m_EndFactor));
	pThis->m_Volume.m_nDuration		= ORDERBYTES((WORD)  (pSource->m_Volume.m_Duration*60.0));
	pThis->m_Volume.m_EnvStartTime	= ORDERBYTES((WORD)  (pSource->m_Volume.m_EnvStartTime*60.0));


	pThis->m_Pitch.m_MaxValue			= ORDERBYTES(FLOAT2SF(pSource->m_Pitch.m_MaxValue));
	pThis->m_Pitch.m_StartFactor		= ORDERBYTES(FLOAT2SF(pSource->m_Pitch.m_StartFactor));
	pThis->m_Pitch.m_EndFactor			= ORDERBYTES(FLOAT2SF(pSource->m_Pitch.m_EndFactor));
	pThis->m_Pitch.m_nDuration			= ORDERBYTES((WORD)  (pSource->m_Pitch.m_Duration*60.0));
	pThis->m_Pitch.m_EnvStartTime		= ORDERBYTES((WORD)  (pSource->m_Pitch.m_EnvStartTime*60.0));

//	pThis->m_Volume.m_MaxValue = ORDERBYTES( (float) pSource->m_Volume.m_MaxValue );
//	pThis->m_Volume.m_StartFactor = ORDERBYTES(pSource->m_Volume.m_StartFactor);
//	pThis->m_Volume.m_EndFactor = ORDERBYTES(pSource->m_Volume.m_EndFactor);
//	pThis->m_Volume.m_nDuration = ORDERBYTES((DWORD) (pSource->m_Volume.m_Duration*60.0));
//	pThis->m_Volume.m_EnvStartTime = ORDERBYTES((DWORD) (pSource->m_Volume.m_EnvStartTime*60.0));
//	pThis->m_Pitch.m_MaxValue = ORDERBYTES( (float) pSource->m_Pitch.m_MaxValue);
//	pThis->m_Pitch.m_StartFactor = ORDERBYTES(pSource->m_Pitch.m_StartFactor);
//	pThis->m_Pitch.m_EndFactor = ORDERBYTES(pSource->m_Pitch.m_EndFactor);
//	pThis->m_Pitch.m_nDuration = ORDERBYTES((DWORD) (pSource->m_Pitch.m_Duration*60.0));
//	pThis->m_Pitch.m_EnvStartTime = ORDERBYTES((DWORD) (pSource->m_Pitch.m_EnvStartTime*60.0));

}


#endif



#ifndef WIN32

extern int g_turok_drawall;
void CGameObjectInstance__Draw(CGameObjectInstance *pThis, Gfx **ppDLP,
										 CCacheEntry *pceTextureSetsIndex)
{
	CIndexedSet				isObjectIndex,
								isGeometry,
								isCurrentAnim,
								isBlendAnim,
								isNodes;
	ROMAddress				*rpGeometryAddress;
	DWORD						geometrySize;
	void						*pbNodes;
	CCacheEntry				*pceGeometry;
	BOOL						isPlayer, isDevice, isAbsoluteAnim, isAlive, better, sendEvents;
	CVector3					vTCorners[8],
								vDelta,
								vShadowOffset;
	float						prevFrameIncrement,
								distFromPlayerSquared;
	CGameObjectInstance	*pPlayer;
	CMtxF						mfOrient;
//#define DRAW_BOUNDS
#ifdef DRAW_BOUNDS
	CQuatern					qID;
	int						bc;
	CVector3					vZero;
#endif
	int						nNodes;
	CAnimDraw				AnimDraw;

#ifdef PLATFORM_PORT
	/* PORT: animated-object drawing (model node hierarchy + anim streams: CROMNodeIndex /
	 * CTranslationOffset / CPosFrame / events / transitions) is now byte-swapped (the anim endian
	 * sweep), so it draws by default. TUROK_ANIMOBJ=0 disables it for debugging. */
	{ extern char *getenv(const char *); const char *e = getenv("TUROK_ANIMOBJ");
	  if (e && *e == '0') return; }
#endif

	isPlayer = (pThis == CEngineApp__GetPlayer(GetApp())) ;

#ifdef PLATFORM_PORT
	{ extern int fprintf(void*,const char*,...); extern void *stderr; extern char *getenv(const char*);
	  static int _seen[512], _ns=0; int _i, _ot;
	  if (getenv("TUROK_OBJLOG")) { _ot = (int)CGameObjectInstance__TypeFlag(pThis); for(_i=0;_i<_ns;_i++) if(_seen[_i]==_ot) goto _done;
	    if(_ns<512){_seen[_ns++]=_ot; fprintf(stderr,"[objdraw] type=0x%x isPlayer=%d (call #%d)\n",_ot,isPlayer,_ns);} _done:; }
	  /* TUROK_OBJPOS: per-object world pos + distance from player, so we can aim a close-up. */
	  if (!isPlayer && getenv("TUROK_OBJPOS")) {
	    CGameObjectInstance *pl = (CGameObjectInstance*)CEngineApp__GetPlayer(GetApp());
	    CVector3 o = pThis->ah.ih.m_vPos;
	    if (pl) { CVector3 p = pl->ah.ih.m_vPos; float dx=o.x-p.x,dy=o.y-p.y,dz=o.z-p.z; float d2=dx*dx+dy*dy+dz*dz;
	      if (d2 < 4000.0f*4000.0f) fprintf(stderr,"[objpos] type=0x%x pos=(%.0f,%.0f,%.0f) d=%.0f\n",
	        (int)CGameObjectInstance__TypeFlag(pThis), o.x,o.y,o.z, __builtin_sqrtf(d2)); }
	  } }
#endif

	// Clear incase anything uses it (TRex!!)
	pThis->pmtDrawMtxs = NULL ;

	//rmonPrintf("%x: Draw\n", pThis);

	// this routine will get called every frame for every
	// animated instance
///////////// NOT ANYMORE! ////////////////////////

	// don't draw enemies during ressurect - but update
	if ((GetApp()->m_Camera.m_Mode == CAMERA_CINEMA_TUROK_RESURRECT_MODE) &&
		 (pThis != CEngineApp__GetPlayer(GetApp())) &&
		 (!CGameObjectInstance__IsDevice(pThis)))
		return ;

	// Draw in "pickup key" cinema?
	if (    pThis->ah.ih.m_pEA && (pThis->ah.ih.m_pEA->m_wTypeFlags3 & AI_TYPE3_DONTDRAWONCINEMA)
		  && (GetApp()->m_Camera.m_Mode == CAMERA_CINEMA_TUROK_PICKUP_KEY_MODE) )
		return ;

	// not ready to draw first time yet
	//if (!pThis->m_nAnims || !pThis->m_pceObjectInfo)
	if (!pThis->m_nAnims)
		return;

	
	// scene.cpp checks for this
	ASSERT(pThis->m_pceObjectInfo);

	// make sure the anims index, object index, and object info are handy

	pThis->m_pceAnimsIndex = NULL;
   CCartCache__RequestBlock(GetCache(),
									 NULL, NULL, NULL,
                            &pThis->m_pceAnimsIndex,
                            pThis->m_rpAnims, IS_INDEX_SIZE(pThis->m_nAnims),
                            "Animations Index (draw)");

	pThis->m_pceObjectIndex = NULL;
   CCartCache__RequestBlock(GetCache(),
									 NULL, NULL, NULL,
									 &pThis->m_pceObjectIndex,
                            pThis->m_rpObject, IS_INDEX_SIZE(CART_GRAPHOBJ_nItems),
                            "Object Index (draw)");

	pThis->m_pceObjectInfo = NULL;
	CCartCache__RequestBlock(GetCache(),
									 NULL, NULL, NULL,
									 &pThis->m_pceObjectInfo,
									 pThis->m_rpObjectInfo, pThis->m_ObjectInfoSize,
									 "Object Info (draw)");

	if (!pThis->m_pceAnimsIndex || !pThis->m_pceObjectIndex || !pThis->m_pceObjectInfo)
		return;

	// make sure geometry and animation requests don't push these out of the cache
	CCacheEntry__ResetAge(pThis->m_pceAnimsIndex);
	CCacheEntry__ResetAge(pThis->m_pceObjectIndex);
	CCacheEntry__ResetAge(pThis->m_pceObjectInfo);

	// make sure anims index stays around
	//if (pThis->m_pceAnimsIndex)
	//	CCacheEntry__ResetAge(pThis->m_pceAnimsIndex);

	// make sure object index stays around
	//if (pThis->m_pceObjectIndex)
	//	CCacheEntry__ResetAge(pThis->m_pceObjectIndex);

	// need object info to figure out bounds and anims index to advance
	//if (pThis->m_pceObjectInfo && pThis->m_pceAnimsIndex)
	//{
	// make sure object info stays around
	//CCacheEntry__ResetAge(pThis->m_pceObjectInfo);

	// bounds check against anim rect
	//if (isPlayer || (isAlive || && (pThis->m_pBoss || CBoundsRect__IsOverlapping(&pThis->m_BoundsRect, &anim_bounds_rect))))

	if (CGameObjectInstance__IsGone(pThis))
	{
		ASSERT(pThis != CEngineApp__GetPlayer(GetApp()));
		prevFrameIncrement = frame_increment;
		CGameObjectInstance__DoAI(pThis);
		frame_increment = prevFrameIncrement;

		CGameObjectInstance__SetNotVisible(pThis);

		// make sure old animation doesn't get used
		pThis->m_asCurrent.m_pceAnim = NULL;
	}
	//else if (isPlayer || pThis->m_pBoss || CBoundsRect__IsOverlapping(&pThis->m_BoundsRect, &anim_bounds_rect))
	else
	{
		pPlayer = CEngineApp__GetPlayer(GetApp());
		isPlayer = (pThis == pPlayer);
		isDevice = CGameObjectInstance__IsDevice(pThis);

		isAlive = !(pThis->m_AI.m_dwStatusFlags & AI_ALREADY_DEAD);

		if (!pThis->m_asCurrent.m_pceAnim)
			CGameObjectInstance__RequestInitialAnimation(pThis);

		if (pThis->m_asCurrent.m_pceAnim)
		{
			CCacheEntry__ResetAge(pThis->m_asCurrent.m_pceAnim);

			// allow AI to speed up/slow down animations
			prevFrameIncrement = frame_increment;

			CGameObjectInstance__PreDraw(pThis, ppDLP);

			CVector3__Subtract(&vDelta, &pThis->ah.ih.m_vPos, &pPlayer->ah.ih.m_vPos);
			distFromPlayerSquared = CVector3__MagnitudeSquared(&vDelta);

			sendEvents = FALSE;
			if (!isPlayer)
			{
#define AI_DISTANCE	50*SCALING_FACTOR
				if (		CGameObjectInstance__IsVisible(pThis)
						|| CGameObjectInstance__IsDevice(pThis)
						|| pThis->m_pBoss
						|| pThis->ah.ih.m_pEA->m_InteractiveAnim
						|| !pThis->m_AI.m_Health
						|| (distFromPlayerSquared < SQR(AI_DISTANCE)) )
				{
					sendEvents = TRUE;

					CGameObjectInstance__DoAI(pThis);

					if (pThis->ah.ih.m_pEA->m_dwTypeFlags & AI_TYPE_SLOWENEMY)
						frame_increment *= 0.5;

					// This function in "cinecam.cpp"
					frame_increment *= CGameObjectInstance__AnimSpeedScaler(pThis) ;

					if (pThis->m_asCurrent.m_pceAnim)
						CGameObjectInstance__Advance(pThis);
				}
			}

			if (pThis->m_asCurrent.m_pceAnim)
			{
				CIndexedSet__ConstructFromRawData(&isCurrentAnim,
														 	CCacheEntry__GetData(pThis->m_asCurrent.m_pceAnim),
														 	FALSE);

				if (CGameObjectInstance__IsBlending(pThis))
				{
					ASSERT(pThis->m_asBlend.m_pceAnim);
					CIndexedSet__ConstructFromRawData(&isBlendAnim,
															 	CCacheEntry__GetData(pThis->m_asBlend.m_pceAnim),
															 	FALSE);
				}

				if (!isPlayer)
				{
					//CGameObjectInstance__CalculateOrientationMatrix(pThis, isPlayer, vTCorners);
					CGameObjectInstance__CalculateOrientationMatrix(pThis, FALSE, vTCorners, mfOrient);

					if (sendEvents)
						CGameObjectInstance__SendAnimEvents(pThis, &isCurrentAnim, mfOrient);
				}

#ifdef PLATFORM_PORT
				{ extern char *getenv(const char*); extern int fprintf(void*,const char*,...); extern void *stderr;
				  static int s_gl=-1, s_seen[256], s_ns=0; if(s_gl<0) s_gl=getenv("TUROK_GATELOG")?1:0;
				  if(s_gl && !isPlayer){ int _t=(int)CGameObjectInstance__TypeFlag(pThis), _i, _dup=0;
				    for(_i=0;_i<s_ns;_i++) if(s_seen[_i]==_t){_dup=1;break;}
				    if(!_dup && s_ns<256){ s_seen[s_ns++]=_t;
				      int bov=CBoundsRect__IsOverlapping(&pThis->m_BoundsRect,&view_bounds_rect);
				      int vis=(pThis->m_AI.m_dwStatusFlags2 & AI_VISIBLE)?1:0;
				      int vvo=CViewVolume__IsOverlapping(&view_volume,8,vTCorners);
				      fprintf(stderr,"[gate] type=0x%x dev=%d boundsOverlap=%d AI_VISIBLE=%d viewVol=%d => %s\n",
				        _t,isDevice,bov,vis,vvo, ((bov&&(isDevice||vis))&&vvo)?"DRAWS":"CULLED"); } } }
#endif
				// bounds check against view rect
				if (		isPlayer || g_turok_drawall
						|| (		CBoundsRect__IsOverlapping(&pThis->m_BoundsRect, &view_bounds_rect)
								&& (isDevice || (pThis->m_AI.m_dwStatusFlags2 & AI_VISIBLE)) ) )
				{
					// bounds check against view volume
					if (isPlayer || g_turok_drawall || CViewVolume__IsOverlapping(&view_volume, 8, isPlayer ? vt_player_corners : vTCorners))
					{
						CGameObjectInstance__SetVisible(pThis);

						//ASSERT(pThis->m_pceObjectIndex);
						CIndexedSet__ConstructFromRawData(&isObjectIndex,
													 				CCacheEntry__GetData(pThis->m_pceObjectIndex),
													 				FALSE);

						// calculate address of geometry

   					rpGeometryAddress = CIndexedSet__GetROMAddressAndSize(&isObjectIndex,
																	  							pThis->m_rpObject,
																	  							CART_GRAPHOBJ_isGeometry, &geometrySize);

						// request geometry data (will be decompressed)
						CGeometry__RequestBlock(&pThis->m_Geometry, GetCache(),
	  						 							rpGeometryAddress, geometrySize,
														pceTextureSetsIndex);

						pceGeometry = CGeometry__GetCacheEntry(&pThis->m_Geometry);
						if (pceGeometry)
						{
							// Get total nodes in object
							CIndexedSet__ConstructFromRawData(&isGeometry,
														 			 	CCacheEntry__GetData(pceGeometry),
														 			 	FALSE);

							pbNodes = CIndexedSet__GetBlock(&isGeometry, CART_GEOMETRY_isNodes);
							CIndexedSet__ConstructFromRawData(&isNodes, pbNodes, FALSE);
							nNodes = CIndexedSet__GetBlockCount(&isNodes);

							// get matrix table for drawing animated instances
							pThis->pmtDrawMtxs = CAnimationMtxs__RequestMtxTable(nNodes);
							if (pThis->pmtDrawMtxs)
							{
								// Update head tracking
								AI_PerformHeadTracking(pThis);

								isAbsoluteAnim = CGameObjectInstance__IsAbsoluteAnim(pThis, pThis->m_asCurrent.m_nAnim);

								if (isAbsoluteAnim)
								{
									vShadowOffset = CGameObjectInstance__GetNodeBlendPos(pThis, &pThis->m_asCurrent, 0);
									vShadowOffset.y = 0;
								}
								else
								{
									vShadowOffset.x = vShadowOffset.y = vShadowOffset.z = 0;
								}

								// draw shadow before model so model will be in front of shadow
								// never draw shadow for devices?
								if (!isDevice && !isPlayer)
									CGameObjectInstance__DrawShadow(pThis, ppDLP, &vShadowOffset);

								// instance orientation
  								gSPMatrix((*ppDLP)++,
						 					RSP_ADDRESS(&CGameObjectInstance__GetOrientationMatrix(pThis)),
	   				 					G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);

								// draw some stuff with better quality anti-aliasing
								// || pThis->m_pBoss ?
								if (		(GetApp()->m_Mode == MODE_INTRO)
										|| (GetApp()->m_Mode == MODE_LEGALSCREEN)
										|| (GetApp()->m_Mode == MODE_TITLE)
										|| (isPlayer)
										|| (GetApp()->m_bPause))// || (pThis->ah.ih.m_pEA->m_dwTypeFlags & AI_TYPE_SNIPER))
								{
									better = TRUE;
								}
								else
								{
									// draw dead enemies with better quality
									if (isAlive)
									{
										// draw far enemies with better quality
										// (far enemies look much better with full anti-aliasing
										//  and don't take much screen area so they won't slow
										//  the game down much)
		#define BETTER_AA_DISTANCE	50*SCALING_FACTOR
										better = distFromPlayerSquared > SQR(BETTER_AA_DISTANCE);
									}
									else
									{
										better = TRUE;
									}
								}

								// draw instance
								AnimDraw.ppDLP = ppDLP;
								AnimDraw.pisNodes = &isNodes;
								AnimDraw.pisCurrentAnim = &isCurrentAnim;
								AnimDraw.pisBlendAnim = &isBlendAnim;
								AnimDraw.RemoveRootXZ = !((isPlayer && !GetApp()->m_Camera.m_CinemaMode) || isDevice || isAbsoluteAnim);
								AnimDraw.Better = better;
								//AnimDraw.Inter = isPlayer;
								AnimDraw.Inter = FALSE;



								// Update special general purpose mode ai
								switch(pThis->m_Mode)
								{
									case IDLE_MODE:
										break ;

									case TRANS_FADE_OUT_MODE:

										// Draw with trans mode!
										fx_mode = FXMODE_TOTRANSPARENT ;
										fx_color[3] = pThis->m_ModeTime ;	// Set trans

										// Only fade if not pausing
										if (!GetApp()->m_bPause)
											pThis->m_ModeTime += pThis->m_ModeMisc1 ;	// Misc1 = fade speed

										// Totally transparent?
										if (pThis->m_ModeTime <= MAX_TRANS)
										{
											CGameObjectInstance__SetGone(pThis) ;
											pThis->m_Mode = IDLE_MODE ;
										}
										break ;
								}


								if ((fx_mode != FXMODE_TOTRANSPARENT) || fx_color[3])
								{
									CGameObjectInstance__DoDraw(pThis, &AnimDraw, 0,
																	 	isPlayer ? mf_player_orient : mfOrient,
																	 	TRUE);

									fx_mode = FXMODE_NONE ;

									// add swoosh edge if a swoosh is active
									if (pThis->m_ActiveSwooshes)
										CFxSystem__CheckForAddingObjectSwooshEdge(&GetApp()->m_FxSystem, pThis, pThis->pmtDrawMtxs) ;
								}

		#ifdef DRAW_BOUNDS
								CQuatern__Identity(&qID);
								vZero.x = vZero.y = vZero.z = 0;
								for (bc=0; bc<8; bc++)
								{
									CParticleSystem__CreateParticle(&GetApp()->m_Scene.m_ParticleSystem, &pThis->ah.ih, PARTICLE_TYPE_GENERIC100,
																		  	vZero, qID,
																		  	(isPlayer ? vt_player_corners : vTCorners)[bc],
																		  	NULL, PARTICLE_NOEDGE);
								}
		#endif

								CIndexedSet__Destruct(&isGeometry);
								CIndexedSet__Destruct(&isNodes);
							}
						}

						CIndexedSet__Destruct(&isObjectIndex);
					}
					else
					{
						CGameObjectInstance__SetNotVisible(pThis);
					}
				}
				else
				{
					CGameObjectInstance__SetNotVisible(pThis);
				}
			}

			CGameObjectInstance__PostDraw(pThis, ppDLP);
			frame_increment = prevFrameIncrement;

			CIndexedSet__Destruct(&isCurrentAnim);
			if (CGameObjectInstance__IsBlending(pThis))
				CIndexedSet__Destruct(&isBlendAnim);
		}
	}
/*
	else
	{
		CGameObjectInstance__SetNotVisible(pThis);

		// make sure old animation doesn't get used
		pThis->m_asCurrent.m_pceAnim = NULL;
	}
*/
	//}
}



void CGameObjectInstance__PreDraw(CGameObjectInstance *pThis, Gfx **ppDLP)
{
	CVector3					vPos;
	static Lights1			glareLight;
	float						mag;
	BYTE						bMag;
	CMtxF						mfRotate;
	CQuatern					qRot;


	if (!CGameObjectInstance__IsDevice(pThis))
		frame_increment *= enemy_speed_scaler;

#ifndef KANJI
	if (		(GetApp()->m_Mode == MODE_GAME)
			&&	(GetApp()->m_dwCheatFlags & CHEATFLAG_DISCO)
			&& pThis->m_AI.m_Health )
	{
		if ((frame_number % 250) > 150)
			frame_increment *= 2.0;
	}
#endif

	// Boss function?
	if ((pThis->m_pBoss) && (pThis->m_pBoss->m_pPreDrawFunction))
	{
		pThis->m_pBoss->m_pPreDrawFunction(pThis, pThis->m_pBoss, ppDLP);
	}
	// melting ?
	else if (pThis->m_cMelt > 0)
	{
		fx_mode = FXMODE_TOTRANSPARENT;
		fx_color[3] = max(0, min(255, 255 - (int) (pThis->m_cMelt*(255/MELT_LENGTH))));
	}
	// is character frozen ?
	else if (pThis->m_AI.m_FreezeTimer != 0.0)
	{
#define	DARKNESS_FORBLACK		204.0
#define	DARKNESS_FORYELLOW	204.0

		// which color change ?
		fx_mode = FXMODE_TOCOLOR;
		if (pThis->m_AI.m_FreezeTimer > 0.0)
		{
			if (pThis->m_AI.m_FreezeTimer >= AI_FREEZE_FADETOBLACK)
			{
				// pulse yellow
				mag = (AI_FREEZE_START - pThis->m_AI.m_FreezeTimer) * 180.0 / (AI_FREEZE_START - AI_FREEZE_FADETOBLACK);
				bMag = (BYTE) (sinf(ANGLE_DTOR(mag)) * DARKNESS_FORYELLOW);
				fx_color[0] = 255;
				fx_color[1] = 255;
				fx_color[2] = 0;
				fx_color[3] = bMag;
			}
			else
			{
				// fade to black
				if (pThis->m_AI.m_FreezeTimer >= AI_FREEZE_FADETOBLACKEND)
					mag = (AI_FREEZE_FADETOBLACK - pThis->m_AI.m_FreezeTimer) * DARKNESS_FORBLACK / (AI_FREEZE_FADETOBLACK - AI_FREEZE_FADETOBLACKEND);
				else
					mag = DARKNESS_FORBLACK;

				bMag = (BYTE) mag;
				fx_color[0] = 0;
				fx_color[1] = 0;
				fx_color[2] = 0;
				fx_color[3] = bMag;
			}

			// reduce time
			pThis->m_AI.m_FreezeTimer -= frame_increment*(1.0/FRAME_FPS);
			if (pThis->m_AI.m_FreezeTimer <= 0.0)
				pThis->m_AI.m_FreezeTimer = AI_FREEZE_END;
		}
		else
		{
			fx_color[0] = 0;
			fx_color[1] = 0;
			fx_color[2] = 0;
			fx_color[3] = (BYTE) DARKNESS_FORBLACK;
		}

		// freeze the enemy
		frame_increment = 0.0;
	}
	// regeneration appearance ?
	else if (pThis->m_AI.m_cRegenerateAppearance)
	{
//		fx_mode = FXMODE_TOTRANSPARENT;
//		fx_color[3] = max(0, min(255, 255 - (int) (pThis->m_AI.m_cRegenerateAppearance*(255/APPEARANCE_LENGTH))));

		fx_mode = FXMODE_TOCOLORANDTRANSPARENT;
		fx_color[0] = 240;		// env r
		fx_color[1] = 240;		// env g
		fx_color[2] = 240;		// env b
		fx_color[3] = 255;
		fx_color[4] = max(0, min(255, 255 - (int) (pThis->m_AI.m_cRegenerateAppearance*(255/APPEARANCE_LENGTH))));
	}
	else if (pThis->m_AI.m_cRegenerateAppearanceWhite)
	{
		fx_mode = FXMODE_TOCOLOR;
		fx_color[0] = 240;
		fx_color[1] = 240;
		fx_color[2] = 240;
		fx_color[3] = 255 - max(0, min(255, 255 - (int) (pThis->m_AI.m_cRegenerateAppearanceWhite*(255/APPEARANCEWHITE_LENGTH))));
	}
	// any glaring particles ?
	else if (		(p_particle_glare)
					&&	(CEngineApp__GetPlayer(GetApp()) == pThis) )
	{
		qRot = GetApp()->m_qViewAlign;
		CQuatern__Negate(&qRot);
		CQuatern__ToMatrix(&qRot, mfRotate);
		CVector3__Subtract(&vPos, &v_particle_glare_pos, &pThis->ah.ih.m_vPos);
		CMtxF__VectorMult(mfRotate, &vPos, &vPos);
		mag = CVector3__Magnitude(&vPos);

		if (mag != 0)
			CVector3__MultScaler(&vPos, &vPos, 60.0/mag);

		bMag = 255 - (BYTE) min(255, mag*(255.0/(30.0*SCALING_FACTOR)));

		glareLight.a.l.col [0] = 0;
		glareLight.a.l.col [1] = 0;
		glareLight.a.l.col [2] = 0;
		glareLight.a.l.colc[0] = 0;
		glareLight.a.l.colc[1] = 0;
		glareLight.a.l.colc[2] = 0;

		glareLight.l[0].l.col [0] = bMag;
		glareLight.l[0].l.col [1] = bMag;
		glareLight.l[0].l.col [2] = bMag;
		glareLight.l[0].l.colc[0] = bMag;
		glareLight.l[0].l.colc[1] = bMag;
		glareLight.l[0].l.colc[2] = bMag;

		glareLight.l[0].l.dir [0] = (signed char) vPos.x;
		glareLight.l[0].l.dir [1] = (signed char) vPos.y;
		glareLight.l[0].l.dir [2] = (signed char) vPos.z;

		//gDPPipeSync((*ppDLP)++);
		gSPSetLights1((*ppDLP)++, glareLight);

		fx_mode = FXMODE_GLARE;

		fx_color[0] = particle_glare_color[0];
		fx_color[1] = particle_glare_color[1];
		fx_color[2] = particle_glare_color[2];
	}
	else
	{
/*		if (	  (GetApp()->m_Mode == MODE_GAME)
			  && (GetApp()->m_dwCheatFlags & CHEATFLAG_MODIFYENEMY) )
		{
			if (pThis == CTurokMovement.pModifyEnemy)
			{
				fx_mode = FXMODE_TOCOLOR;
				fx_color[0] = 255 ;
				fx_color[1] = 0 ;
				fx_color[2] = 255 ;
				fx_color[3] = (frame_number%64 <32) ? frame_number%64 : 64-(frame_number%64) ;
				fx_color[3] *= 3 ;

			}
		}
*/		switch (CGameObjectInstance__TypeFlag(pThis))
		{
			case AI_OBJECT_DEVICE_PORTAL:
				fx_mode = FXMODE_TOTRANSPARENT;
				fx_color[3] = 255 * pThis->m_AI.m_Time1 ;
				//fx_color[3] = (BYTE) max(0, min(255, fabs(((frame_number%60) - 30.0)) * 255.0/30.0));
				break ;
		}
	}

	// NOTE: This mode may not work properly with untextured objects
	//fx_mode = FXMODE_TOCOLORANDTRANSPARENT;
	//fx_color[0] = 255;	// env r
	//fx_color[1] = 0;	// env g
	//fx_color[2] = 0;	// env b
	//fx_color[3] = 0;	// env a		0: 0% color, 255: 100% color
	//fx_color[4] = 128;	// prim a	0: transparent, 255: opaque

	//fx_mode = FXMODE_TOTRANSPARENT;
	//fx_color[3] = 128;	// env a		0: transparent, 255: opaque

	// NOTE: This mode is the only mode besides FXMODE_NONE that uses the z-buffer
	//fx_mode = FXMODE_TOCOLOR;
	//fx_color[0] = 255;	// env r
	//fx_color[1] = 0;	// env g
	//fx_color[2] = 0;	// env b
	//fx_color[3] = 0;	// env a		0: 0% color, 255: 100% color

	//fx_mode = FXMODE_FROMCOLOR_TOTRANSPARENT
	//fx_color[0] = 255;	// env r
	//fx_color[1] = 0;	// env g
	//fx_color[2] = 0;	// env b
	//fx_color[3] = 128;	// env a		0: transparent, 255: opaque
}

void CGameObjectInstance__PostDraw(CGameObjectInstance *pThis, Gfx **ppDLP)
{
	// was particle doing glare ?
	if (		(p_particle_glare)
			&&	(CEngineApp__GetPlayer(GetApp()) == pThis) )
	{
		//gDPPipeSync((*ppDLP)++);
		gSPSetLights1((*ppDLP)++, thelight);
	}

	// Boss function?
	if ((pThis->m_pBoss) && (pThis->m_pBoss->m_pPostDrawFunction))
		pThis->m_pBoss->m_pPostDrawFunction(pThis, pThis->m_pBoss, ppDLP) ;

	fx_mode = FXMODE_NONE;
}


/////////////////////////////////////////////////////////////////////////////
// Turok model index
/////////////////////////////////////////////////////////////////////////////
#define TUROK_L_HAND_NODE	17
#define TUROK_R_HAND_NODE	21

int CInstanceHdr__GetTurokTextureIndex(CInstanceHdr *pThis, int nNode)
{
	switch(nNode)
	{
		// Right hand is normal(0),  knife(1), key(3,4 & 5)
		case TUROK_R_HAND_NODE:
			// if in key cinema, use 3, 4 or 5 depending on key
			if (GetApp()->m_Camera.m_Mode == CAMERA_CINEMA_TUROK_PICKUP_KEY_MODE)
			{
				if (CTurokMovement.LastKeyTexture >=0)
					return CTurokMovement.LastKeyTexture ;
				else
					return 0 ;
			}
			break ;
	}

	return 0 ;
}



int CInstanceHdr__GetTextureIndex(CInstanceHdr *pThis, int nType, int nNode)
{
	switch (nType)
	{
		case AI_OBJECT_CHARACTER_PLAYER:
			return CInstanceHdr__GetTurokTextureIndex(pThis, nNode) ;

		case AI_OBJECT_CHARACTER_CAMPAIGNER_BOSS:
			if (pThis->m_Type == I_ANIMATED)
				return CGameObjectInstance__GetCampaignerTextureIndex((CGameObjectInstance*) pThis, nNode);
			break;

		case AI_OBJECT_CHARACTER_TREX_BOSS:
			if (pThis->m_Type == I_ANIMATED)
				return CGameObjectInstance__GetTRexTextureIndex((CGameObjectInstance*) pThis, nNode);
			break;

		case AI_OBJECT_CHARACTER_HUMAN1:
			if (pThis->m_Type == I_ANIMATED)
				return CGameObjectInstance__GetHumanTextureIndex((CGameObjectInstance*) pThis, nNode);
			break;

		case AI_OBJECT_DEVICE_GATE:
			return CInstanceHdr__GetGateTextureIndex(pThis, nNode);

		case AI_OBJECT_DEVICE_LOCK:
			return CInstanceHdr__GetLockTextureIndex(pThis, nNode);

		case AI_OBJECT_DEVICE_KEYFLOOR:
			return CInstanceHdr__GetKeyFloorTextureIndex(pThis, nNode);

		case AI_OBJECT_WEAPON_TECHWEAPON2:
			if (nNode == 3)
			{
				if (CTurokMovement.WeaponMode == TWEAPONMODE_ON)
					return game_frame_number%5 + 1;
				else
					return 0;
			}
			break;
	}

	return pThis->m_pEA->m_nTexture;
}


/////////////////////////////////////////////////////////////////////////////
// Human texture index
/////////////////////////////////////////////////////////////////////////////
enum HumanHeirarchy
{
	ROOT,
		LUPLEG,
		LLOWLEG,
		LFOOT,
		LTOES,
		RUPLEG,
		RLOWLEG,
		RFOOT,
		RTOES,
		TORSO,
		LSHOULDER,
		LUPARM,
		LLOWARM,
		LHAND,
		RSHOULDER,
		RUPARM,
		RLOWARM,
		RHAND,
		NECK,
		HEAD
} ;

int CGameObjectInstance__GetHumanTextureIndex(CGameObjectInstance *pThis, int nNode)
{
	int type;

	type = AI_GetEA(pThis)->m_AttackStyle ;

	// FootSoldier
	if (type < 9)
	{
#ifdef GERMAN
		return AI_GetDyn(pThis)->m_HeadTexture ;
#else
		switch (nNode)
		{
			case HEAD:
				return AI_GetDyn(pThis)->m_HeadTexture ;

			case TORSO:
			case ROOT:
			case LSHOULDER:
			case LUPARM:
			case LLOWARM:
			case LHAND:
			case RSHOULDER:
			case RUPARM:
			case RLOWARM:
			case RHAND:
				return AI_GetDyn(pThis)->m_BodyTexture ;

			// Legs and Neck
			default:
				return 0 ;
		}
#endif
	}

	// Longhunters men
	else if (type < 18)
	{
#ifdef GERMAN
		return pThis->ah.ih.m_pEA->m_nTexture;
#else
		switch (nNode)
		{
			case HEAD:
				return AI_GetDyn(pThis)->m_HeadTexture ;

			case TORSO:
			case ROOT:
				return 6-1 ;

			case LSHOULDER:
			case LUPARM:
			case LLOWARM:
			case LHAND:
			case RSHOULDER:
			case RUPARM:
			case RLOWARM:
			case RHAND:
				return 1-1 ;

			case LUPLEG:
			case LLOWLEG:
			case LFOOT:
			case RUPLEG:
			case RLOWLEG:
			case RFOOT:
				return 3-1 ;

			default:
				return 0 ;
		}
#endif
	}

	// Ancient Warrior 1-3
	else if (type < 22)
	{
#ifdef GERMAN
		return pThis->ah.ih.m_pEA->m_nTexture;
#else
		switch (nNode)
		{
			case HEAD:
				return 0 ;

			case NECK:
				return 2-1 ;

			case LSHOULDER:
			case LUPARM:
			case LLOWARM:
			case LHAND:
			case RSHOULDER:
			case RUPARM:
			case RLOWARM:
			case RHAND:
			case TORSO:
			case ROOT:
				return 3-1 ;

			default:
				return 0 ;
		}
#endif
	}

	// Ancient Warrior 4-5
	else if (type < 25)
	{
#ifdef GERMAN
		return pThis->ah.ih.m_pEA->m_nTexture;
#else
		switch (nNode)
		{
			case NECK:
				return 3-1 ;

			case LSHOULDER:
			case LUPARM:
			case LLOWARM:
			case LHAND:
			case RSHOULDER:
			case RUPARM:
			case RLOWARM:
			case RHAND:
			case TORSO:
			case ROOT:
				return 4-1 ;

			case LUPLEG:
			case LLOWLEG:
			case LFOOT:
			case RUPLEG:
			case RLOWLEG:
			case RFOOT:
				return 2-1 ;

			default:
				return 0 ;
		}
#endif
	}

	// Cyborg
	else if (type < 27)
	{
#ifdef GERMAN
		return pThis->ah.ih.m_pEA->m_nTexture;
#else
		switch (nNode)
		{
			case NECK:
			case LSHOULDER:
			case LUPARM:
			case LLOWARM:
			case LHAND:
			case RSHOULDER:
			case RUPARM:
			case RLOWARM:
			case RHAND:
			case LUPLEG:
			case LLOWLEG:
			case LFOOT:
			case RUPLEG:
			case RLOWLEG:
			case RFOOT:
			case ROOT:
			case TORSO:
				if (type == 25)
					return 1-1 ;
				else
					return 2-1 ;

			case HEAD:
				if (type == 25)
					return game_frame_number%9 ;
				else
					return (game_frame_number%9)+9 ;

			default:
				return 0 ;
		}
#endif
	}
	// Robosuit (same as regular human without armor and face)
	else if (type <28)
	{
#ifdef GERMAN
		return pThis->ah.ih.m_pEA->m_nTexture;
#else
		switch (nNode)
		{
			case TORSO:
			case ROOT:
			case LSHOULDER:
			case LUPARM:
			case LLOWARM:
			case LHAND:
			case RSHOULDER:
			case RUPARM:
			case RLOWARM:
			case RHAND:
				return 5-1 ;		// no armor

			case LUPLEG:
			case LLOWLEG:
			case LFOOT:
			case RUPLEG:
			case RLOWLEG:
			case RFOOT:
				return 2-1 ;

			case NECK:
				return 4-1 ;

			case HEAD:
			default:
				return 0;
		}
#endif
	}
	// Demon / Zombie
	else if (type < 30)
	{
#ifdef GERMAN
		return pThis->ah.ih.m_pEA->m_nTexture;
#else
		switch (nNode)
		{
			case NECK:
				if (type == 28)
					return 5-1 ;
				else
					return 6-1 ;

			case LUPLEG:
			case LLOWLEG:
			case LFOOT:
			case RUPLEG:
			case RLOWLEG:
			case RFOOT:
				if (type == 28)
					return 3-1 ;
				else
					return 4-1 ;

			case ROOT:
			case TORSO:
				if (type == 28)
					return 7-1 ;
				else
					return 8-1 ;


			case LSHOULDER:
			case LUPARM:
			case LLOWARM:
			case LHAND:
			case RSHOULDER:
			case RUPARM:
			case RLOWARM:
			case RHAND:
				if (type == 28)
					return 7-1 ;
				else
					return 8-1 ;

			default:
				return 0 ;
		}
#endif
	}
	return 0 ;
}


/////////////////////////////////////////////////////////////////////////////
// Gate texture index
/////////////////////////////////////////////////////////////////////////////
int CInstanceHdr__GetGateTextureIndex(CInstanceHdr *pThis, int nNode)
{
	int nKey, nTexture;

	switch (pThis->m_pEA->m_Id)
	{
		case 2:
			nKey = CTurokMovement.Level2Access;
			nTexture = 0;
			break ;
		case 3:
			nKey = CTurokMovement.Level3Access;
			nTexture = 1;
			break ;
		case 4:
			nKey = CTurokMovement.Level4Access;
			nTexture = 2;
			break ;
		case 5:
			nKey = CTurokMovement.Level5Access;
			nTexture = 3;
			break ;
		case 6:
			nKey = CTurokMovement.Level6Access;
			nTexture = 4;
			break ;
		case 7:
			nKey = CTurokMovement.Level7Access;
			nTexture = 5;
			break ;

		default:
			return pThis->m_pEA->m_nTexture;
	}
	if ((nNode != 0) && (nKey < nNode))
	{
		fx_mode = FXMODE_TOCOLOR;
		fx_color[0] = 0;		// env r
		fx_color[1] = 0;		// env g
		fx_color[2] = 0;		// env b
		fx_color[3] = 192;	// env a		0: 0% color, 255: 100% color
	}
	return nTexture;
}



/////////////////////////////////////////////////////////////////////////////
// Lock texture index
/////////////////////////////////////////////////////////////////////////////
int CInstanceHdr__GetLockTextureIndex(CInstanceHdr *pThis, int nNode)
{
	int nKey, nTexture;

	switch (pThis->m_pEA->m_Id)
	{
		case 2:
			nKey = CTurokMovement.Level2Access;
			nTexture = 0;
			break ;
		case 3:
			nKey = CTurokMovement.Level3Access;
			nTexture = 1;
			break ;
		case 4:
			nKey = CTurokMovement.Level4Access;
			nTexture = 2;
			break ;
		case 5:
			nKey = CTurokMovement.Level5Access;
			nTexture = 3;
			break ;
		case 6:
			nKey = CTurokMovement.Level6Access;
			nTexture = 4;
			break ;
		case 7:
			nKey = CTurokMovement.Level7Access;
			nTexture = 5;
			break ;

		case 8:
			nKey = CTurokMovement.Level8Access;
			nTexture = 0;
			break ;

		default:
			return pThis->m_pEA->m_nTexture;
	}

	// last level is slightly different (something had to be!)
	if (pThis->m_pEA->m_Id == 8)
	{
		if ((nNode > 0) && (nKey < (nNode-0)))
		{
			fx_mode = FXMODE_TOTRANSPARENT;
			fx_color[3] = 0;
		}
		return nTexture;
	}
	// levels 2-7
	else
	{
		if ((nNode != 0) && (nKey < nNode))
		{
			fx_mode = FXMODE_TOTRANSPARENT;
			fx_color[3] = 0;
		}
		return nTexture;
	}
}


/////////////////////////////////////////////////////////////////////////////
// Key floor texture index
/////////////////////////////////////////////////////////////////////////////
int CInstanceHdr__GetKeyFloorTextureIndex(CInstanceHdr *pThis, int nNode)
{
	int nKey, nTexture, id;
	BOOL	hilite = FALSE ;

	switch (pThis->m_pEA->m_Id)
	{
		case 40:
		case 41:
		case 42:
			nKey = CTurokMovement.Level4Keys;
			nTexture = 2;
			break ;
		case 50:
		case 51:
		case 52:
			nKey = CTurokMovement.Level5Keys;
			nTexture = 3;
			break ;
		case 60:
		case 61:
		case 62:
			nKey = CTurokMovement.Level6Keys;
			nTexture = 4;
			break ;
		case 70:
		case 71:
		case 72:
			nKey = CTurokMovement.Level7Keys;
			nTexture = 5;
			break ;
		case 80:
		case 81:
		case 82:
		case 83:
		case 84:
			nKey = CTurokMovement.Level8Keys;
			nTexture = 6;
			break ;

		// chronosceptor piece
		case 100:
		case 101:
		case 102:
		case 103:
		case 104:
		case 105:
		case 106:
		case 107:
			nKey = CTurokMovement.ChronoSceptorPieces;
			nTexture = 0;
			break ;


		//default:
		//	return pThis->ah.ih.m_pEA->m_nTexture;
	}

	id = pThis->m_pEA->m_Id % 10 ;

	// regular key pieces
	if (!(nKey & (1<<id)))
		hilite = TRUE ;

	if (hilite)
	{
		fx_mode = FXMODE_TOCOLOR;
		fx_color[0] = 0;		// env r
		fx_color[1] = 0;		// env g
 		fx_color[2] = 0;		// env b
 		fx_color[3] = 180;	// env a		0: 0% color, 255: 100% color
 	}
	return nTexture;
}


/////////////////////////////////////////////////////////////////////////////
// Turok model index
/////////////////////////////////////////////////////////////////////////////
#define TUROK_L_HAND_NODE	17
#define TUROK_R_HAND_NODE	21

int CGameObjectInstance__GetTurokModelIndex(CGameObjectInstance *pThis, int nNode)
{
	CGameObjectInstance *pTrex = GetApp()->m_pBossActive ;

	// if TREX is active and eating turok, make turok vanish after frame 17
	// S.B. Added 12/23/96 - fixed bug!!!!
	if (    (pTrex)
		  && (CGameObjectInstance__TypeFlag(pTrex) == AI_OBJECT_CHARACTER_TREX_BOSS)
		  && (GetApp()->m_Camera.m_Mode == CAMERA_CINEMA_TREX_KILL_TUROK_MODE) )
	{
		// This check fixes game over stuff as well
		if (pTrex->m_pBoss->m_Mode == TREX_EATTUROK_MODE)
		{
			if (pTrex->m_asCurrent.m_cFrame >= 17)
				return -1 ;
		}
		else
			return -1 ;
	}

	switch(nNode)
	{
		// Left hand is normal(0), and bow(1)
		case TUROK_L_HAND_NODE:
			if (GetApp()->m_CurrentWarpID == INTRO_TUROK_DRAWING_BOW_WARP_ID)
				return 1 ;
			break ;

		// Right hand is normal(0),  knife(2), key(3, 4 or 5)
		case TUROK_R_HAND_NODE:
			// if in key cinema, use 3, 4 or 5 depending on key
			if (GetApp()->m_Camera.m_Mode == CAMERA_CINEMA_TUROK_PICKUP_KEY_MODE)
			{
				if (CTurokMovement.LastKeyTexture == -1)
					return 4 ;
				else if (CTurokMovement.LastKeyTexture == -2)
					return 5 ;
				else
					return 3 ;
			}

			// Knife for victory
			if (GetApp()->m_Camera.m_Mode == CAMERA_CINEMA_TUROK_VICTORY_MODE)
				return 2 ;

			break ;
	}

	return 0 ;
}



// if TREX is active, make legs partially vanish
int CGameObjectInstance__GetTurokCorpseModelIndex(CGameObjectInstance *pThis, int nNode)
{
	CGameObjectInstance *pTrex = GetApp()->m_pBossActive ;

	if (pTrex->m_pBoss->m_Mode != TREX_EATTUROK_MODE)
		return -1 ;

	switch (nNode)
	{
		case 0:
			if (pTrex->m_asCurrent.m_cFrame > 104)
				return -1 ;
			break ;
		case 1:
		case 2:
			if (pTrex->m_asCurrent.m_cFrame > 122)
				return -1 ;
			break ;
		case 3:
		case 4:
			if (pTrex->m_asCurrent.m_cFrame > 135)
				return -1 ;
			break ;
		case 5:
		case 6:
		case 7:
		case 8:
			if (pTrex->m_asCurrent.m_cFrame > 118)
				return -1 ;
			break ;
		case 9:
			if (pTrex->m_asCurrent.m_cFrame > 121)
				return -1 ;
			break ;
		case 10:
		case 11:
		case 12:
			if (pTrex->m_asCurrent.m_cFrame > 135)
				return -1 ;
			break ;
	}

	if (pTrex->m_asCurrent.m_cFrame <17)
		return -1 ;

	return 0 ;
}

// if TREX is active, make feathers vanish until frame 158
int CGameObjectInstance__GetTurokFeathersModelIndex(CGameObjectInstance *pThis, int nNode)
{
	CGameObjectInstance *pTrex = GetApp()->m_pBossActive ;

	if (pTrex->m_pBoss->m_Mode != TREX_EATTUROK_MODE)
		return -1 ;


	if (pTrex->m_asCurrent.m_cFrame < 158)
		return -1 ;

	return 0 ;
}









#define PORTAL_MORPH_FRAMES	10
#define PORTAL_IDLE_SPEED		6

int CGameObjectInstance__GetModelIndex(CGameObjectInstance *pThis, int nType, int nNode)
{
	int				type;
	//static DWORD	morphFrame = -1;
	//static float	cMorph = 0;

	switch (nType)
	{
		case AI_OBJECT_DEVICE_ACTION:
		case AI_OBJECT_DEVICE_ANIMOFFSCREENACTION:
			// trex stuff
			if ((GetApp()->m_pBossActive) &&
				 (CGameObjectInstance__TypeFlag(GetApp()->m_pBossActive) == AI_OBJECT_CHARACTER_TREX_BOSS))
			{
				// turok corpse
				if (pThis->ah.ih.m_pEA->m_Id == 1)
					return CGameObjectInstance__GetTurokCorpseModelIndex((CGameObjectInstance *)pThis, nNode) ;
				// turok feathers
				else if (pThis->ah.ih.m_pEA->m_Id == 2)
					return CGameObjectInstance__GetTurokFeathersModelIndex((CGameObjectInstance *)pThis, nNode) ;
			}
			break ;

		case AI_OBJECT_DEVICE_TREXELEVATOR:
#ifndef KANJI
			return 0 ;
#else
			return 1 ;
#endif

		case AI_OBJECT_DEVICE_CRYSTAL:
			return CGameObjectInstance__GetCrystalModelIndex(pThis, nNode) ;

		case AI_OBJECT_CHARACTER_CAMPAIGNER_BOSS:
			return CGameObjectInstance__GetCampaignerModelIndex(pThis, nNode) ;

		case AI_OBJECT_CHARACTER_LONGHUNTER_BOSS:
			return CGameObjectInstance__GetLonghunterModelIndex(pThis, nNode) ;

		case AI_OBJECT_CHARACTER_PLAYER:
			return CGameObjectInstance__GetTurokModelIndex(pThis, nNode) ;

		case AI_OBJECT_DEVICE_PORTAL:
			//if (morphFrame != frame_number)
			//{
			//	morphFrame = frame_number;

				//cMorph += frame_increment*((1.0/FRAME_FPS)*PORTAL_IDLE_SPEED*(PORTAL_MORPH_FRAMES/10.0));
			//	if (cMorph >= PORTAL_MORPH_FRAMES)
			//		cMorph -= PORTAL_MORPH_FRAMES;

			CGeometry__MorphInc(&pThis->m_Geometry, 0, frame_increment*((1.0/FRAME_FPS)*PORTAL_IDLE_SPEED*(PORTAL_MORPH_FRAMES/10.0)));
			//}
			return even_odd;

		// FootSoldier (only human with switchable models)
		case AI_OBJECT_CHARACTER_HUMAN1:
			// Bone
//			AI_GetEA(pThis)->m_AttackStyle = 0 ;
//			AI_GetDyn(pThis)->m_WeaponModel = 1 ;

			// Sword
//			AI_GetEA(pThis)->m_AttackStyle = 0 ;
//			AI_GetDyn(pThis)->m_WeaponModel = 14 ;

			// Mace
//			AI_GetEA(pThis)->m_AttackStyle = 0 ;
//			AI_GetDyn(pThis)->m_WeaponModel = 2 ;

			// Sceptre
//			AI_GetEA(pThis)->m_AttackStyle = 22 ;

			// Pole1
//			AI_GetEA(pThis)->m_AttackStyle = 3 ;
//			AI_GetDyn(pThis)->m_WeaponModel = 3 ;

			// Pole2
//			AI_GetEA(pThis)->m_AttackStyle = 3 ;
//			AI_GetDyn(pThis)->m_WeaponModel = 16 ;


			type = AI_GetEA(pThis)->m_AttackStyle ;

			// weapons
			if (nNode == RHAND)
			{
				// corpse should have no weapon....
				if (AI_GetEA(pThis)->m_wTypeFlags3 & AI_TYPE3_DUMB)
					return 0 ;

				switch (type)
				{
					// weapons
					case 0:
					case 1:
					case 3:
						return AI_GetDyn(pThis)->m_WeaponModel ;
					default:
						return pThis->ah.ih.m_pEA->m_nModel ;
				}
			}
#ifndef GERMAN
			// shins
			else if ((nNode == LLOWLEG) || (nNode == RLOWLEG))
			{
				switch (type)
				{
					case 1:
					case 2:
					case 4:
					case 5:
					case 7:
						return AI_GetDyn(pThis)->m_ExtraModel ;
					default:
						return pThis->ah.ih.m_pEA->m_nModel ;
				}
			}

			// shoulders
			else if ((nNode == LSHOULDER) || (nNode == RSHOULDER))
			{
				switch (type)
				{
					case 3:
						return AI_GetDyn(pThis)->m_ExtraModel ;
					default:
						return pThis->ah.ih.m_pEA->m_nModel ;
				}
			}
#endif
			else
				return pThis->ah.ih.m_pEA->m_nModel ;
	}

	return pThis->ah.ih.m_pEA->m_nModel ;
}

void CGameObjectInstance__DoDraw(CGameObjectInstance *pThis, CAnimDraw *pAnimDraw,
											int nNode, CMtxF mfParent, BOOL OnlyChild)
{
	// these variables are declared static because function
	// is recursive and uses lots of stack space

	static BOOL						blending;
	static void 					*pbNode,
										*pbChildIndices;
	static CMtxF					mfTranslate, mfRotate,
										mfProd1;
	static Mtx						*pmFinal;
	static int						nFrameC, nFrameB,
										nNextFrameC, nNextFrameB;
	static CVector3				vBlendPos, vNextBlendPos, vFinalBlendPos;
	static CQuatern				qBlendRot, qNextBlendRot, qFinalBlendRot,
										qFinalRot;
	static float					fraction;
	static CQuatern				qC, qNextC,
										qB, qNextB,
										qTemp, qXRot, qYRot, qZRot;
	static CVector3				vC, vNextC,
										vB, vNextB;

	static CHeadTrackInfo 		*pHeadTrackInfo;
	static CHeadTrackNodeInfo	*pHeadTrackNodeInfo;
	static BOOL						BigHead;

   // these variables cannot be declared static
	CMtxF								mfFinal;
	CIndexedSet 					isNode;
   CUnindexedSet					usChildIndices;
	DWORD								cChild, nChildren,
										*childIndices;

	ASSERT(pThis->pmtDrawMtxs);

	BigHead = FALSE ;

	blending = CGameObjectInstance__IsBlending(pThis);

	if (    (GetApp()->m_Mode == MODE_GAME)
		  && (GetApp()->m_dwCheatFlags & CHEATFLAG_QUACKMODE) )
		fraction = 0;
	else
		fraction = pThis->m_asCurrent.m_cFrame - ((int) pThis->m_asCurrent.m_cFrame);

	// single node
	pbNode = CIndexedSet__GetBlock(pAnimDraw->pisNodes, nNode);
	CIndexedSet__ConstructFromRawData(&isNode, pbNode, FALSE);

	// single node's animation
	if ( 	  (GetApp()->m_Mode == MODE_GAME)
		  && (GetApp()->m_dwCheatFlags & CHEATFLAG_QUACKMODE) )
		nFrameC = max(0, ((int) pThis->m_asCurrent.m_cFrame) & ~1);
	else
		nFrameC = max(0, (int) pThis->m_asCurrent.m_cFrame);

	nNextFrameC = (int) CGameAnimateState__GetNextFrame(&pThis->m_asCurrent);

	qC			= CGameObjectInstance__GetNodeRot(pAnimDraw->pisCurrentAnim, nNode, nFrameC);
	qNextC	= CGameObjectInstance__GetNodeRot(pAnimDraw->pisCurrentAnim, nNode, nNextFrameC);

	vC			= CGameObjectInstance__GetNodePos(pAnimDraw->pisCurrentAnim, nNode, nFrameC);
	vNextC	= CGameObjectInstance__GetNodePos(pAnimDraw->pisCurrentAnim, nNode, nNextFrameC);

#ifdef PLATFORM_PORT
	{ extern char *getenv(const char*); extern int fprintf(void*,const char*,...); extern void *stderr;
	  static int s_qc=-1, s_n=0; if(s_qc<0) s_qc=getenv("TUROK_QCLOG")?1:0;
	  if(s_qc && s_n<16){ s_n++;
	    fprintf(stderr,"[qc] node=%d frameC=%d rotSet=%d qC=(%.4f,%.4f,%.4f,%.4f) vC=(%.1f,%.1f,%.1f)\n",
	      nNode, nFrameC,
	      CGameObjectInstance__GetNodeAnimIndex(pAnimDraw->pisCurrentAnim, nNode).m_nRotationSet,
	      qC.x,qC.y,qC.z,qC.t, vC.x,vC.y,vC.z); } }
#endif

	if (blending)
	{
		nFrameB = max(0, (int) pThis->m_asBlend.m_cFrame);
		nNextFrameB = (int) CGameAnimateState__GetNextFrame(&pThis->m_asBlend);

		qB			= CGameObjectInstance__GetNodeRot(pAnimDraw->pisBlendAnim, nNode, nFrameB);
		qNextB	= CGameObjectInstance__GetNodeRot(pAnimDraw->pisBlendAnim, nNode, nNextFrameB);

		vB			= CGameObjectInstance__GetNodePos(pAnimDraw->pisBlendAnim, nNode, nFrameB);
		vNextB	= CGameObjectInstance__GetNodePos(pAnimDraw->pisBlendAnim, nNode, nNextFrameB);

		CQuatern__Blend(&qBlendRot, pThis->m_uBlender, &qB, &qC);
		CQuatern__Blend(&qNextBlendRot, pThis->m_uBlender, &qNextB, &qNextC);

		CVector3__Blend(&vBlendPos, pThis->m_uBlender, &vB, &vC);
		CVector3__Blend(&vNextBlendPos, pThis->m_uBlender, &vNextB, &vNextC);
	}
	else
	{
		qBlendRot		= qC;
		qNextBlendRot	= qNextC;

		vBlendPos		= vC;
		vNextBlendPos	= vNextC;
	}

	CQuatern__Blend(&qFinalBlendRot, fraction, &qBlendRot, &qNextBlendRot);

	CVector3__Blend(&vFinalBlendPos, fraction, &vBlendPos, &vNextBlendPos);

	// build matrices
	if (nNode)
	{
		// rotation
		if ((pThis == CEngineApp__GetPlayer(GetApp())) && (nNode == 1))
		{
			// hack to rotate minigun and auto-shotgun's barrels
			CQuatern__BuildFromAxisAngle(&qZRot, 0, 1, 0, weapon_z_rotation);
			CQuatern__Mult(&qFinalRot, &qZRot, &qFinalBlendRot);
		}
		else
		if ((CGameObjectInstance__TypeFlag(pThis) == AI_OBJECT_BUNKER_TURRET) && (nNode == 1))
		{
			CQuatern__BuildFromAxisAngle(&qZRot, 1, 0, 0, pThis->m_HeadTiltAngle) ;
			CQuatern__Mult(&qFinalRot, &qFinalBlendRot, &qZRot);
		}
		else
		if ((CGameObjectInstance__TypeFlag(pThis) == AI_OBJECT_CEILING_TURRET) && (nNode == 1))
		{
			CQuatern__BuildFromAxisAngle(&qZRot, 1, 0, 0, pThis->m_HeadTiltAngle) ;
			CQuatern__Mult(&qFinalRot, &qFinalBlendRot, &qZRot);
		}
		else
		if (CGameObjectInstance__TypeFlag(pThis) == AI_OBJECT_CHARACTER_HUMVEE_BOSS)
		{
			switch (nNode)
			{
				case 12:
					CQuatern__BuildFromAxisAngle(&qZRot, 0, 1, 0, pThis->m_pBoss->m_Rot1) ;
					CQuatern__Mult(&qFinalRot, &qZRot, &qFinalBlendRot);
					break ;

				case 18:
					CQuatern__BuildFromAxisAngle(&qZRot, 0, 1, 0, pThis->m_pBoss->m_Rot2) ;
					CQuatern__Mult(&qFinalRot, &qZRot, &qFinalBlendRot);
					break ;

				case 14:
					CQuatern__BuildFromAxisAngle(&qZRot, 0, 1, 0, pThis->m_pBoss->m_Rot3) ;
					CQuatern__Mult(&qTemp, &qZRot, &qFinalBlendRot);

					// Add steer
					CQuatern__BuildFromAxisAngle(&qZRot, 0, 0, 1, pThis->m_pBoss->m_Rot5) ;
					CQuatern__Mult(&qFinalRot, &qTemp, &qZRot);
					break ;

				case 16:
					CQuatern__BuildFromAxisAngle(&qZRot, 0, 1, 0, pThis->m_pBoss->m_Rot4) ;
					CQuatern__Mult(&qTemp, &qZRot, &qFinalBlendRot);

					// Add steer
					CQuatern__BuildFromAxisAngle(&qZRot, 0, 0, 1, pThis->m_pBoss->m_Rot5) ;
					CQuatern__Mult(&qFinalRot, &qTemp, &qZRot);
					break ;

				default:
					qFinalRot = qFinalBlendRot;
			}
		}
		else
		// Generic head tracking
		if (((pHeadTrackInfo = pThis->m_pHeadTrackInfo)) &&
			(nNode >= pHeadTrackInfo->MinHeadNode) &&	(nNode <= pHeadTrackInfo->MaxHeadNode))
		{
			pHeadTrackNodeInfo = &pHeadTrackInfo->pNodeInfo[nNode - pHeadTrackInfo->MinHeadNode] ;
			fraction = pThis->m_pHeadTrackInfo->MaxHeadNode - pThis->m_pHeadTrackInfo->MinHeadNode + 1 ;

			// Use big head?
			if ((GetApp()->m_Mode == MODE_GAME) &&
				 (GetApp()->m_dwCheatFlags & CHEATFLAG_BIGHEADS) &&
				 (pHeadTrackInfo->MaxHeadNode == nNode))
				BigHead = TRUE ;

			// Create turn angle
			if (pHeadTrackNodeInfo->Flags & TURN_FLIP_DIR)
			{
				CQuatern__BuildFromAxisAngle(&qZRot,
													  ((pHeadTrackNodeInfo->Flags & TURN_AXIS_X) != 0),
													  ((pHeadTrackNodeInfo->Flags & TURN_AXIS_Y) != 0),
													  ((pHeadTrackNodeInfo->Flags & TURN_AXIS_Z) != 0),
													  (-pThis->m_HeadTurnAngle / fraction)) ;
			}
			else
			{
				CQuatern__BuildFromAxisAngle(&qZRot,
													  ((pHeadTrackNodeInfo->Flags & TURN_AXIS_X) != 0),
													  ((pHeadTrackNodeInfo->Flags & TURN_AXIS_Y) != 0),
													  ((pHeadTrackNodeInfo->Flags & TURN_AXIS_Z) != 0),
													  (pThis->m_HeadTurnAngle / fraction)) ;
			}

			// Create tilt angle
			if (pHeadTrackNodeInfo->Flags & TILT_FLIP_DIR)
			{
				CQuatern__BuildFromAxisAngle(&qXRot,
													  ((pHeadTrackNodeInfo->Flags & TILT_AXIS_X) != 0),
													  ((pHeadTrackNodeInfo->Flags & TILT_AXIS_Y) != 0),
													  ((pHeadTrackNodeInfo->Flags & TILT_AXIS_Z) != 0),
													  (-pThis->m_HeadTiltAngle / fraction)) ;
			}
			else
			{
				CQuatern__BuildFromAxisAngle(&qXRot,
													  ((pHeadTrackNodeInfo->Flags & TILT_AXIS_X) != 0),
													  ((pHeadTrackNodeInfo->Flags & TILT_AXIS_Y) != 0),
													  ((pHeadTrackNodeInfo->Flags & TILT_AXIS_Z) != 0),
													  (pThis->m_HeadTiltAngle / fraction)) ;
			}

			// Concatenate rotations
			if (pHeadTrackNodeInfo->Flags & REVERSE_CONCAT)
				CQuatern__Mult(&qFinalRot, &qXRot, &qZRot);
			else
				CQuatern__Mult(&qFinalRot, &qZRot, &qXRot);

			// Blend head tracking with identity
			CQuatern__Identity(&qZRot) ;
			CQuatern__Blend(&qXRot, pThis->m_HeadTrackBlend, &qZRot, &qFinalRot) ;

			// Concatenate onto animation rotation
			CQuatern__Mult(&qFinalRot, &qFinalBlendRot, &qXRot) ;
		}
		else
			qFinalRot = qFinalBlendRot;
	}
	else
	{
  		if (pAnimDraw->RemoveRootXZ)
			vFinalBlendPos.x = vFinalBlendPos.y = 0;

		qFinalRot = qFinalBlendRot;
	}


#if USE_ORIGINAL_MATRIX
	CQuatern__ToMatrix(&qFinalRot, mfRotate);

	CMtxF__Translate(mfTranslate,
						  vFinalBlendPos.x, vFinalBlendPos.y, vFinalBlendPos.z);

	CMtxF__PostMult(mfProd1, mfRotate, mfTranslate);

#else

	// Using optimized matrix functions
	CQuatern__ToMatrix(&qFinalRot, mfProd1) ;

	// Make head big?
	if (BigHead)
	{
		CMtxF__PreMultScale(mfProd1,
								  pThis->m_pHeadTrackInfo->BigHeadScale,
								  pThis->m_pHeadTrackInfo->BigHeadScale,
								  pThis->m_pHeadTrackInfo->BigHeadScale) ;
	}

	// TEMP
	//if (nNode == 0)
	//	CMtxF__PreMultScale(mfProd1, 1.6, 1.6, 1.6);
	//else
	//	CMtxF__PreMultScale(mfProd1, 0.8, 0.8, 0.8);

	//if (OnlyChild)
	//	CMtxF__PreMultScale(mfProd1, 3,3,3);

	CMtxF__PostMultTranslate(mfProd1, vFinalBlendPos.x, vFinalBlendPos.y, vFinalBlendPos.z) ;

#endif

	pmFinal = &(pThis->pmtDrawMtxs[nNode]) ;

	// Only keep track of matrices if swooshes are active on object
	if ((pThis->m_ActiveSwooshes) || (pThis->m_pBoss))
	{
		CMtxF__PostMult(mfFinal, mfProd1, mfParent);

		CMtxF__ToMtx(mfFinal, *pmFinal);

		if (OnlyChild)
		{
			// no need to push matrix on stack if only child
			gSPMatrix((*pAnimDraw->ppDLP)++,
				 		 RSP_ADDRESS(pmFinal),
	   		 		 G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
		}
		else
		{
			gSPMatrix((*pAnimDraw->ppDLP)++,
				 		 RSP_ADDRESS(pmFinal),
	   		 		 G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_PUSH);
		}
	}
	else
	{
		CMtxF__ToMtx(mfProd1, *pmFinal);

		if (OnlyChild)
		{
			// no need to push matrix on stack if only child
			gSPMatrix((*pAnimDraw->ppDLP)++,
				 		 RSP_ADDRESS(pmFinal),
	   		 		 G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
		}
		else
		{
			gSPMatrix((*pAnimDraw->ppDLP)++,
				 		 RSP_ADDRESS(pmFinal),
	   		 		 G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);
		}
	}


	CGeometry__DrawPart(&pThis->m_Geometry, pAnimDraw->ppDLP, GetCache(),
							  nNode,
							  CGameObjectInstance__GetModelIndex(pThis, CGameObjectInstance__TypeFlag(pThis), nNode),
							  pAnimDraw->Inter,
#ifdef SCREEN_SHOT
							  TRUE,
#else
							  pAnimDraw->Better,
#endif
							  CInstanceHdr__GetTextureIndex(&pThis->ah.ih, pThis->m_nTypeFlag, nNode));

   // recurse on children
   pbChildIndices = CIndexedSet__GetBlock(&isNode, CART_NODE_usChildNodeIndices);

	CUnindexedSet__ConstructFromRawData(&usChildIndices, pbChildIndices, FALSE);
   nChildren = CUnindexedSet__GetBlockCount(&usChildIndices);

   childIndices = (DWORD*) CUnindexedSet__GetBasePtr(&usChildIndices);

	// must draw node before drawing children so that only child won't
	// overwrite matrix

	for (cChild=0; cChild<nChildren; cChild++)
	{
		CGameObjectInstance__DoDraw(pThis, pAnimDraw, childIndices[cChild],
											 mfFinal,
											 (nChildren == 1));
	}

	if (!OnlyChild)
		gSPPopMatrix((*pAnimDraw->ppDLP)++, G_MTX_MODELVIEW);

	CIndexedSet__Destruct(&isNode);
	CUnindexedSet__Destruct(&usChildIndices);
}

#endif

