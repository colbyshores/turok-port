// romstruc.h

#ifndef _INC_ROMSTRUC
#define _INC_ROMSTRUC

/////////////////////////////////////////////////////////////////////////////

#include "anim.h"

#ifdef WIN32
#include "graphic.h"
#include "region.h"
#include "partfx.h"
#include "soundfx.h"
#include <afxtempl.h>
#else
#include "rommodel.h"
#include "geometry.h"
#include "region.h"
#include "parttype.h"
#include "textload.h"
#endif

#include "aistruc.h"
#include "graphu64.h"

#ifdef WIN32
class CObjectInstance;
class CObArray;
class CLevel;
#endif

struct CCollisionInfo2_t;

/////////////////////////////////////////////////////////////////////////////

#define FLOAT2INT16(floatValue, maxValue) ( (short) ((floatValue)*32767.0f/(maxValue)) )
#define INT162FLOAT(intValue, maxValue) ( (float) ((intValue)*((float)(maxValue))/32767.0f) )

#define FLOAT2INT32(floatValue, maxValue) ( (long) ((floatValue)*2147483648.0f/(maxValue)) )
#define INT322FLOAT(intValue, maxValue) ( (float) ((intValue)*((float)(maxValue))/2147483648.0f) )

#define FLOAT2INT8(floatValue, maxValue) ( (signed char) ((floatValue)*127.0f/(maxValue)) )
#define INT82FLOAT(intValue, maxValue) ( (float) ((intValue)*((float)(maxValue))/127.0f) )

typedef WORD	SF;

SF FLOAT2SF(float fValue);
float SF2FLOAT(SF sfValue);

/////////////////////////////////////////////////////////////////////////////

typedef struct CROMCornerEnc_t
{
	long			x, y, z, c;
} CROMCornerEnc;

typedef struct CROMCorner_t
{
   CVector3    m_vCorner;
   float       m_Ceiling;
} CROMCorner;

// CROMCorner operations
/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
void			CROMCorner__TakeFromCorner(CROMCorner *pThis, CCorner *pSource);
#endif

/////////////////////////////////////////////////////////////////////////////

typedef struct CROMRegion_t
{
	WORD					m_nRegionSet,
							m_wFlags;
   unsigned short		m_Corners[3],
							m_Neighbors[3];
} CROMRegion;

// CROMRegion operations
/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
void			CROMRegion__TakeFromRegion(CROMRegion *pThis, CRegion *pSource,
													CMap<CRegion*, CRegion*, int, int> &RegionPtrToIndexMap,
													CMap<CCorner*, CCorner*, int, int> &CornerPtrToIndexMap,
													CObArray &Regions,
													int nRegionSet,
													CRegionSet *pRegionSet);
#endif

/////////////////////////////////////////////////////////////////////////////

typedef struct CGameRegion_t
{
	WORD						m_nRegionSet,
								m_wFlags;
	CROMCorner				*m_pCorners[3];
	struct CGameRegion_t	*m_pNeighbors[3];
} CGameRegion;

// CGameRegion operations
/////////////////////////////////////////////////////////////////////////////

void				CGameRegion__TakeFromROMRegion(CGameRegion *pThis, CROMRegion *pSource,
															 CROMCorner Corners[], CGameRegion Regions[]);

CVector3			CGameRegion__GetGroundNormal(CGameRegion *pThis);
CVector3			CGameRegion__GetGroundUnitNormal(CGameRegion *pThis);
float 			CGameRegion__GetGroundHeight(CGameRegion *pThis, float X, float Z);
CVector3			CGameRegion__GetCeilingNormal(CGameRegion *pThis);
CVector3			CGameRegion__GetCeilingUnitNormal(CGameRegion *pThis);
float				CGameRegion__GetCeilingHeight(CGameRegion *pThis, float X, float Z);
CVector3			CGameRegion__GetSlideVector(CGameRegion *pThis, float Gravity);
float 			CGameRegion__GetUphillCosine(CGameRegion *pThis,
														  float StartX, float StartZ,
														  float EndX, float EndZ);
float				CGameRegion__GetHillAngle(CGameRegion *pThis,
													  float StartX, float StartZ,
													  float EndX, float EndZ);
BOOL				CGameRegion__InRegion(CGameRegion *pThis, float X, float Z);
//CGameRegion*	CGameRegion__CanEnter(CGameRegion *pCurrent, CVector3 *pvCurrent, float CompareHeight, float RotY,
//												 int nDesiredEdge, CVector3 *pvDesired,
//												 BOOL MoveUpLedges, BOOL MoveDownLedges, BOOL EnterWater, BOOL ExitWater,
//												 struct CCollisionInfo_t *pCI);
DWORD				CGameRegion__GetGroundMaterial(CGameRegion *pThis);
DWORD				CGameRegion__GetWallMaterial(CGameRegion *pThis);
BOOL				CGameRegion__GetGroundIntersection(CGameRegion *pThis, CVector3 *pvStart, CVector3 *pvEnd, float *Out);
BOOL				CGameRegion__GetCeilingIntersection(CGameRegion *pThis, CVector3 *pvStart, CVector3 *pvEnd, float *Out);
BOOL				CGameRegion__CalculateWaterIntersection(CGameRegion *pThis, CVector3 *pvStart, CVector3 *pvEnd, float *Out);
CQuatern			CGameRegion__GetGroundRotation(CGameRegion *pThis);
CQuatern			CGameRegion__GetCeilingRotation(CGameRegion *pThis);
float				CGameRegion__GetEdgeAngle(CGameRegion *pThis, int nEdge);
float				CGameRegion__GetGroundYAngle(CGameRegion *pThis);
#define			CGameRegion__IsDoorOpen(pThis) ((pThis)->m_wFlags & REGFLAG_OPENDOOR)
void				CGameRegion__OpenDoor(CGameRegion *pThis);
void				CGameRegion__CloseDoor(CGameRegion *pThis);
void				CGameRegion__ToWater(CGameRegion *pThis) ;
void				CGameRegion__FromWater(CGameRegion *pThis) ;
void				CGameRegion__MakeInstantDeath(CGameRegion *pThis);
void				CGameRegion__UnMakeInstantDeath(CGameRegion *pThis);
void				CGameRegion__MakeFallDeath(CGameRegion *pThis) ;
void				CGameRegion__MakeWarp(CGameRegion *pThis) ;
void				CGameRegion__UnMakeWarp(CGameRegion *pThis) ;
void				CGameRegion__SetGroundHeight(CGameRegion *pThis, float Height);
BOOL				CGameRegion__IsCliff(CGameRegion *pThis);
CGameRegion*	CGameRegion__FindNextLowerRegion(CGameRegion *pThis, CVector3 vPos);

/////////////////////////////////////////////////////////////////////////////

typedef struct CROMBounds_t
{
	CVector3			m_vMin, m_vMax;
} CROMBounds;

// CROMBounds operations
/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
void			CROMBounds__TakeFromCBounds(CROMBounds *pThis, CBounds *pSource);
#endif
void 			CROMBounds__Print(CROMBounds *pThis);

/////////////////////////////////////////////////////////////////////////////

typedef struct CBoundsRect_t
{
	float				m_MinX, m_MinZ,
						m_MaxX, m_MaxZ;
} CBoundsRect;

// CBoundsRect operations
/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
void			CBoundsRect__TakeFromCBounds(CBoundsRect *pThis, CBounds *pSource);
#endif
#define 		CBoundsRect__IsOverlapping(pThis, pCompare)																\
	   				(((pThis)->m_MaxX > (pCompare)->m_MinX) && ((pThis)->m_MinX < (pCompare)->m_MaxX)  	\
	  					&& ((pThis)->m_MaxZ > (pCompare)->m_MinZ) && ((pThis)->m_MinZ < (pCompare)->m_MaxZ))
#define		CBoundsRect__IsOverlappingBounds(pThis, pCompare)																\
	   				(((pThis)->m_MaxX > (pCompare)->m_vMin.x) && ((pThis)->m_MinX < (pCompare)->m_vMax.x)		\
	  					&& ((pThis)->m_MaxZ > (pCompare)->m_vMin.z) && ((pThis)->m_MinZ < (pCompare)->m_vMax.z))
#define		CBoundsRect__InRect(pThis, pVector3)																\
						(((pVector3)->x >= (pThis)->m_MinX) && ((pVector3)->x <= (pThis)->m_MaxX)		\
						&& ((pVector3)->z >= (pThis)->m_MinZ) && ((pVector3)->z <= (pThis)->m_MaxZ))
void 			CBoundsRect__EncloseVectors(CBoundsRect *pThis, int nPts, CVector3 V[]);
void 			CBoundsRect__AddEncloseVectors(CBoundsRect *pThis, int nPts, CVector3 V[]);
void 			CBoundsRect__Print(CBoundsRect *pThis);

/////////////////////////////////////////////////////////////////////////////

typedef struct CROMRegionBlock_t
{
	CBoundsRect		m_BoundsRect;
	DWORD				m_nRegions;
} CROMRegionBlock;

// CROMRegionBlock operations
/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
void			CROMRegionBlock__TakeFromCBounds(CROMRegionBlock *pThis, CBounds *pSource, int nRegions);
#endif

/////////////////////////////////////////////////////////////////////////////

typedef struct CROMGridBounds_t
{
	CBoundsRect		m_BoundsRect;
} CROMGridBounds;

// CROMGridBounds operations
/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
void			CROMGridBounds__TakeFromCBounds(CROMGridBounds *pThis, CBounds *pSource);
#endif

/////////////////////////////////////////////////////////////////////////////

typedef struct CGameGridBounds_t
{
	CBoundsRect		m_BoundsRect;
	CCacheEntry		*m_pceGridSection;
} CGameGridBounds;

// CGameGridBounds operations
/////////////////////////////////////////////////////////////////////////////

#ifndef WIN32
void 			CGameGridBounds__TakeFromROMGridBounds(CGameGridBounds *pThis, CROMGridBounds *pSource);
#endif

/////////////////////////////////////////////////////////////////////////////

//struct CInstanceHdr_t;
//enum EWallCollisionType { WALL_STICK, WALL_SLIDE, WALL_SLIDE_AND_BOUNCE, WALL_REDIRECT, WALL_BOUNCE };
//enum EInstCollisionType { INSTANCE_STICK, INSTANCE_SLIDE, INSTANCE_NOCOLLISION };
//enum EMoveTestType { MOVETEST_LOOKING, MOVETEST_AVOIDANCE, MOVETEST_MOVEMENT };

/*
// wall collision types
#define	WALL_STICK			0
#define	WALL_SLIDE			1
#define	WALL_SLIDEBOUNCE	2
#define	WALL_REDIRECT		3
#define	WALL_BOUNCE			4
#define	WALL_NOCOLLISION	5


// instance collision types
#define	INSTANCE_NOCOLLISION		0
#define	INSTANCE_STICK				1
#define	INSTANCE_SLIDE				2
#define	INSTANCE_GREASE			3

// ground collision types
//#define	GROUND_NOCOLLISION		0
//#define	GROUND_STICK				1
//#define	GROUND_SLIDE				2

#define COLLISIONFLAG_BOUNCEOFFGROUND				(1 << 0)
#define COLLISIONFLAG_CANCLIMB						(1 << 1)
#define COLLISIONFLAG_UPDATEVELOCITY				(1 << 2)
#define COLLISIONFLAG_IGNOREDEAD						(1 << 3)
#define COLLISIONFLAG_USEWALLRADIUS					(1 << 4)
#define COLLISIONFLAG_SMALLWALLRADIUS				(1 << 5)
#define COLLISIONFLAG_PREVENTOSC						(1 << 6)
#define COLLISIONFLAG_NOHILLSLOWDOWN				(1 << 7)
#define COLLISIONFLAG_WATERCOLLISION				(1 << 8)
#define COLLISIONFLAG_NOGROUNDMOVEMENT				(1 << 9)
#define COLLISIONFLAG_MOVETHROUGHDOORS				(1 << 10)
#define COLLISIONFLAG_PICKUPCOLLISION				(1 << 11)
#define COLLISIONFLAG_CANCLIMBCLIMBABLE			(1 << 12)
#define COLLISIONFLAG_CLIFFONLY						(1 << 13)
#define COLLISIONFLAG_STICKTOGROUND					(1 << 14)
#define COLLISIONFLAG_DUCKING							(1 << 15)
#define COLLISIONFLAG_CANENTERRESTRICTEDAREA		(1 << 16)
#define COLLISIONFLAG_NOFLOORCOLLISION				(1 << 17)
#define COLLISIONFLAG_AVOIDPRESSUREPLATE			(1 << 18)


//#define COLLISIONFLAG_AVOIDWATER						(1 << 17)


typedef struct CCollisionInfo_t
{
// input
	DWORD 							m_WallBehavior,
										m_InstanceBehavior,
										m_dwFlags;
	float								m_GravityAcceleration,	// in ft/sec^2
										m_BounceReturnEnergy,	// 1.0 means perfectly elastic collision
										m_GroundFriction,			// 0.0 means no friction, 1.0 means 100% friction
										m_AirFriction,
										m_WaterFriction;
// output
	BOOL								m_WallCollision;
	CVector3							m_vWallCollisionPos;
	CGameRegion						*m_pWallCollisionRegion;
	int								m_nWallCollisionEdge;

	struct CInstanceHdr_t		*m_pInstanceCollision;
	CVector3							m_vInstanceCollisionPos;

#define GROUNDCOLLISION_NONE		0
#define GROUNDCOLLISION_GROUND	1
#define GROUNDCOLLISION_CEILING	2
	int								m_GroundCollision;
	CVector3							m_vGroundCollisionPos;
	CGameRegion						*m_pGroundCollisionRegion;

	BOOL								m_WaterCollision;
	CVector3							m_vWaterCollisionPos;
	CGameRegion						*m_pWaterCollisionRegion;

	CVector3							m_vCollisionVelocity;
} CCollisionInfo;

// CCollisionInfo operations
/////////////////////////////////////////////////////////////////////////////

void		CCollisionInfo__SetPlayerDefaults(CCollisionInfo *pThis);
void		CCollisionInfo__SetNoCollisionAtAllDefaults(CCollisionInfo *pThis);
void		CCollisionInfo__SetPlayerUnderwaterDefaults(CCollisionInfo *pThis);
void		CCollisionInfo__SetPlayerAntiGravDefaults(CCollisionInfo *pThis);
void		CCollisionInfo__SetPlayerOnWaterSurfaceDefaults(CCollisionInfo *pThis);
void		CCollisionInfo__SetCharacterDefaults(CCollisionInfo *pThis);
void		CCollisionInfo__SetParticleDefaults(CCollisionInfo *pThis);
void		CCollisionInfo__SetPickupDefaults(CCollisionInfo *pThis);
void		CCollisionInfo__SetUnderwaterCharacterDefaults(CCollisionInfo *pThis);
void		CCollisionInfo__SetUnderwaterCharacterDeadDefaults(CCollisionInfo *pThis);
void		CCollisionInfo__SetUnderwaterCharacterFloatDeadDefaults(CCollisionInfo *pThis);
void		CCollisionInfo__SetUnderwaterClimbingCharacterDefaults(CCollisionInfo *pThis);
void		CCollisionInfo__SetMovementTestDefaults(CCollisionInfo *pThis);
void		CCollisionInfo__SetNearRegionTestDefaults(CCollisionInfo *pThis);
void		CCollisionInfo__SetPlayerClimbingSideStepCharacterDefaults(CCollisionInfo *pThis);
void		CCollisionInfo__SetFlyingCharacterDefaults(CCollisionInfo *pThis);
*/
/////////////////////////////////////////////////////////////////////////////

#define I_STATIC		0
#define I_ANIMATED	1
#define I_PARTICLE	2
#define I_SIMPLE		3
typedef struct CInstanceHdr_t
{
	BYTE					m_Type,
							m_bActiveFlags;
	WORD					m_nObjType;
	CVector3				m_vPos;
	float					m_CollisionHeightOffset;
	CGameRegion			*m_pCurrentRegion;
	CEnemyAttributes	*m_pEA;
} CInstanceHdr;

// CInstanceHdr operations
/////////////////////////////////////////////////////////////////////////////

CQuatern CInstanceHdr__GetAimAtTarget(CInstanceHdr *pThis, CVector3 vPos);
BOOL		CInstanceHdr__IsOnGround(CInstanceHdr *pThis);
BOOL		CInstanceHdr__IsOnGroundWhenClimbing(CInstanceHdr *pThis);
#define	CInstanceHdr__DoSoundEffect(pThis, nSoundType, Looping) CScene__DoSoundEffect(&GetApp()->m_Scene, nSoundType, Looping, pThis, &(pThis)->m_vPos, 0)
void		CInstanceHdr__GetNearPositionAndRegion(CInstanceHdr *pThis,
																CVector3 vDesiredPos,
																CVector3 *pvOutPos, CGameRegion **ppOutRegion,
																int GroundBehavior, int InstanceBehavior);
BOOL		CInstanceHdr__IsDevice(CInstanceHdr *pThis);
float		CInstanceHdr__GetCliffDistance(CInstanceHdr *pThis, BOOL ToTop, float TestDistance);
CQuatern	CInstanceHdr__GetGroundRotation(CInstanceHdr *pThis);
int		CInstanceHdr__GetLockTextureIndex(CInstanceHdr *pThis, int nNode);
int		CInstanceHdr__GetGateTextureIndex(CInstanceHdr *pThis, int nNode);
int		CInstanceHdr__GetTextureIndex(CInstanceHdr *pThis, int nType, int nNode);
int		CInstanceHdr__TypeFlag(CInstanceHdr *pThis);
#define	CInstanceHdr__GetGroundHeight(pThis) CGameRegion__GetGroundHeight((pThis)->m_pCurrentRegion, (pThis)->m_vPos.x, (pThis)->m_vPos.z)

/////////////////////////////////////////////////////////////////////////////

// everything needed	for region collision
typedef struct CAnimInstanceHdr_t
{
	CInstanceHdr		ih;

	CVector3				m_vVelocity;
							//m_vPrevBackoff;
} CAnimInstanceHdr;

// CAnimInstanceHdr operations
/////////////////////////////////////////////////////////////////////////////

#ifndef WIN32
/*
BOOL				CAnimInstanceHdr__Collision(CAnimInstanceHdr *pThis,
														 CGameRegion **ppCurrentRegion,
														 CVector3 *pvCurrentPos,
														 CVector3 vDesiredPos,
														 CCollisionInfo *pCI,
														 BOOL ClimbUp, BOOL ClimbDown, BOOL EnterWater, BOOL ExitWater);
BOOL				CAnimInstanceHdr__HangOnToCliffEdges(CAnimInstanceHdr *pThis,
																	 CGameRegion *pCurrentRegion,
																	 CVector3 vStartPos,
																	 CVector3 *pvCurrentPos,
																	 CCollisionInfo *pCI);
BOOL				CAnimInstanceHdr__BackOffFromWall(CAnimInstanceHdr *pThis,
																 CGameRegion **ppCurrentRegion,
																 CVector3 vStartPos,
																 CVector3 *pvCurrentPos,
																 CCollisionInfo *pCI,
																 BOOL ClimbUp, BOOL ClimbDown);
CInstanceHdr*	CAnimInstanceHdr__InstanceCollision(CAnimInstanceHdr *pThis,
																	CGameRegion *pCurrentRegion,
																   CVector3 *pvCurrentPos, CVector3 vDesiredPos,
																   CCollisionInfo *pCI);
*/
BOOL				CAnimInstanceHdr__IsObstructed(CAnimInstanceHdr *pThis,
															 CVector3 vDesired,
															 struct CCollisionInfo2_t **ppCIUsed /*=NULL*/);
BOOL				CAnimInstanceHdr__CastYeThyRay(CAnimInstanceHdr *pThis, CInstanceHdr *pTarget);
void				CAnimInstanceHdr__Teleport(CAnimInstanceHdr *pThis, float X, float Z);
#define			CAnimInstanceHdr__GetVelocityRotation(pThis) CVector3__GetDirectionRotation(&(pThis)->m_vVelocity)

#endif

/////////////////////////////////////////////////////////////////////////////

typedef struct CROMSimpleInstance_t
{
	CVector3       m_vPos;
	float				m_MaxRadius;
	WORD				m_nObjType,
						m_nCurrentRegion,
						m_nVariation,
						m_wFlags,
						m_nID;
	signed char		m_RotY;
	BYTE				m_bActiveFlags;
} CROMSimpleInstance;

// CROMSimpleInstance operations
/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
void 		CROMSimpleInstance__TakeFromCObjectInstance(CROMSimpleInstance *pThis,
																 	  CObjectInstance *pSource,
																	  int nObject, int nVariation,
																	  CObArray *pRegions,
																 	  float Scaler,
																	  int nID,
																	  CTurokDoc *pDoc);
#endif

#define	SIMPLE_FLAG_GONE				(1 << 0)
#define	SIMPLE_FLAG_VISIBLE			(1 << 1)
#define	SIMPLE_FLAG_WARP_DYNAMIC	(1 << 2)
#define	SIMPLE_FLAG_DYNAMIC			(1 << 3)
#define	SIMPLE_FLAG_SCALEUP			(1 << 4)
#define	SIMPLE_FLAG_TRANSPARENCY	(1 << 5)
#define	SIMPLE_FLAG_CLOSEDWARP		(1 << 6)

#ifndef WIN32
/////////////////////////////////////////////////////////////////////////////

typedef struct CGameSimpleInstance_t
{
	CAnimInstanceHdr	ah;

   CCacheEntry			*m_pceObjectIndex,
							*m_pceStaticEvents;
	ROMAddress			*m_rpObjectIndexAddress;

	Mtx					m_mOrientation[2],
							m_mShadow[2];
	CGeometry			m_Geometry;

	CVector3				vRotAxis ;
	float					m_RotAngle, m_AngleVel;

	CROMBounds			m_Bounds;
	float					m_Scale, m_OffsetY;
	WORD					m_wFlags,
							m_nID;
	float					m_Time, m_MorphInc;
} CGameSimpleInstance;

// CGameSimpleInstance operations
/////////////////////////////////////////////////////////////////////////////

void		CGameSimpleInstance__TakeFromROMSimpleInstance(CGameSimpleInstance *pThis, CROMSimpleInstance *pSource,
																		  ROMAddress *rpObjectAddress,
																		  CGameRegion Regions[], CEnemyAttributes Variations[]);
void		CGameSimpleInstance__Draw(CGameSimpleInstance *pThis, Gfx **ppDLP,
											  CCartCache *pCache, CCacheEntry *pceTextureSetsIndex);
CQuatern CGameSimpleInstance__GetAimRotation(CGameSimpleInstance *pThis);

#endif	// !WIN32
/////////////////////////////////////////////////////////////////////////////

typedef struct CROMEfficientStaticInstance_t
{
	float				m_Scale;
	WORD				m_nObjType,
						m_nCurrentRegion,
						m_nVariation,
						m_wFlags;

// store sign bits in y
#define EFFICIENT_Y_ACCURACY	0.5
	short				m_nPosY;				// * 1
#define EFFICIENT_ROT_0		0
#define EFFICIENT_ROT_90	1
#define EFFICIENT_ROT_180	2
#define EFFICIENT_ROT_270	3
#define EFFICIENT_ROT_MASK	(EFFICIENT_ROT_0 | EFFICIENT_ROT_90 | EFFICIENT_ROT_180 | EFFICIENT_ROT_270)
#define EFFICIENT_FLAGS_POSX_NEG		(1 << 4)
#define EFFICIENT_FLAGS_POSZ_NEG		(1 << 5)
	BYTE				m_bFlags,
						m_bActiveFlags,
						m_nPosX, m_nPosZ;	// * grid
	signed char		m_nMinX, m_nMaxX,	// * 1
						m_nMinY, m_nMaxY,	// * 1
						m_nMinZ, m_nMaxZ;	// * 1
} CROMEfficientStaticInstance;

// CROMEfficientStaticInstance operations
/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
BOOL 		CROMEfficientStaticInstance__TakeFromCObjectInstance(CROMEfficientStaticInstance *pThis,
																				  CObjectInstance *pSource,
																				  CTurokDoc *pDoc,
																				  int nObject, int nVariation,
																				  CObArray *pRegions,
																				  float Scaler,
																				  BOOL To3DSSpace,
																				  int nGeometryVersion);
#endif

/////////////////////////////////////////////////////////////////////////////

typedef struct CROMStaticInstance_t
{
   CVector3       m_vPos,
						m_vScale;
	SF					m_BoundsMinX, m_BoundsMinY, m_BoundsMinZ,
						m_BoundsMaxX, m_BoundsMaxY, m_BoundsMaxZ;
	WORD				m_nObjType,
						m_nCurrentRegion,
						m_nVariation;
	BYTE				m_bFlags,
						m_bActiveFlags;
	signed char		m_RotX, m_RotY, m_RotZ, m_RotT;
} CROMStaticInstance;

// CROMStaticInstance operations
/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
void 		CROMStaticInstance__TakeFromCObjectInstance(CROMStaticInstance *pThis,
																 	  CObjectInstance *pSource,
																	  CTurokDoc *pDoc,
																	  int nObject, int nVariation,
																	  CObArray *pRegions,
																 	  float Scaler,
																	  BOOL To3DSSpace,
																	  int nGeometryVersion);
#endif

/////////////////////////////////////////////////////////////////////////////

// don't change the position of these
#define	STATIC_INSTANCE_GEOMVER0		(1 << 0)
#define	STATIC_INSTANCE_GEOMVER1		(1 << 1)
#define	STATIC_INSTANCE_GEOMVER_MASK	(STATIC_INSTANCE_GEOMVER0 | STATIC_INSTANCE_GEOMVER1)

#define	STATIC_INSTANCE_TRANSPARENCY	(1 << 2)
#define	STATIC_INSTANCE_HASEVENTS		(1 << 3)

typedef struct CGameStaticInstance_t
{
	CInstanceHdr		ih;

   CCacheEntry			*m_pceObjectIndex,
							*m_pceStaticEvents;
	ROMAddress			*m_rpObjectIndexAddress;
#ifdef WIN32
   CMatrix				m_mOrientation;
#else
	Mtx					m_mOrientation;
	CGeometry			m_Geometry;
#endif
	CROMBounds			m_Bounds;
	DWORD					m_dwFlags;
} CGameStaticInstance;

// CGameStaticInstance operations
/////////////////////////////////////////////////////////////////////////////

#ifndef WIN32
void		CGameStaticInstance__TakeFromROMEfficientStaticInstance(CGameStaticInstance *pThis, CROMEfficientStaticInstance *pSource,
																					  ROMAddress *rpObjectAddress,
																					  CGameRegion Regions[], CEnemyAttributes Variations[],
																					  float GridDistance);
void		CGameStaticInstance__TakeFromROMStaticInstance(CGameStaticInstance *pThis, CROMStaticInstance *pSource,
																		  ROMAddress *rpObjectAddress,
																		  CGameRegion Regions[], CEnemyAttributes Variations[]);
void 		CGameStaticInstance__Draw(CGameStaticInstance *pThis, Gfx **ppDLP,
										  	  CCartCache *pCache, CCacheEntry *pceTextureSetsIndex);
void		CGameStaticInstance__SendEvents(CGameStaticInstance *pThis, CCartCache *pCache, BOOL ConstantRateFrame);
#endif

/////////////////////////////////////////////////////////////////////////////

typedef struct CROMObjectInstance_t
{
	WORD			m_nObjType,
					m_nTypeFlag,
					m_nVariation,
					m_nCurrentRegion;
	INT16			m_RotY;
	BYTE			m_bFlags,
					m_bActiveFlags;
	CVector3		m_vPos,
					m_vScale;
} CROMObjectInstance;

// CROMObjectInstance operations
/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
void 		CROMObjectInstance__TakeFromCObjectInstance(CROMObjectInstance *pThis,
																 	  CObjectInstance *pSource, CObArray *pGraphicObjects,
																 	  float MinScale,
																	  CObArray *pRegions, int nVariation,
																	  CTurokDoc *pDoc);
#endif

/////////////////////////////////////////////////////////////////////////////

// low word is copied from CROMObjectInstance
#define ANIM_INSTANCE_DEVICE			(1 << 0)
#define ANIM_INSTANCE_TRANSPARENCY	(1 << 1)

#define ANIM_INSTANCE_VISIBLE			(1 << 16)
#define ANIM_INSTANCE_BLENDING		(1 << 17)
#define ANIM_INSTANCE_ANIMCHANGED	(1 << 18)
#define ANIM_INSTANCE_NEAREDGE		(1 << 19)
#define ANIM_INSTANCE_HANGINGON		(1 << 20)
#define ANIM_INSTANCE_GONE				(1 << 21)
#define ANIM_INSTANCE_FIRSTBLENDFR	(1 << 22)
#define ANIM_INSTANCE_BLENDTORIGHT	(1 << 23)

typedef struct CGameObjectInstance_t
{
	CAnimInstanceHdr		ah;

   CVector3       		m_vScale;
	CQuatern					m_qRot,			// y rotation is removed		// is this necessary?
								m_qGround,
								m_qShadow;
	float						m_RotY,
								m_cMelt,
								m_ShadowPercentage; // calculate from duck, rather than store?

#ifdef WIN32
   CMatrix        		m_mOrientation;
#else
	Mtx						m_mOrientation[2];
	CGeometry				m_Geometry;
	Mtx						*pmtDrawMtxs ;
#endif

	ROMAddress				*m_rpObject,
								*m_rpObjectInfo,
								*m_rpAnims;
	CCacheEntry				*m_pceObjectIndex,
								*m_pceObjectInfo,
								*m_pceAnimsHeader,
								*m_pceAnimsIndex;
	WORD						m_ObjectInfoSize,
								m_nTypeFlag;


/*
   CCacheEntry				*m_pceObjectIndex,
								*m_pceAnimsHeader,
								*m_pceAnimsIndex,
								*m_pceObjectInfo;
	ROMAddress				*m_rpAnimsIndexAddress,
								*m_rpObjectIndexAddress;
*/

	//CCartCache				*m_pCache;
	short						m_nAnims,
								m_nDesiredAnim;
	CBoundsRect				m_BoundsRect;

	DWORD						m_dwFlags;

	CGameAnimateState		m_asCurrent,
								m_asBlend;
	short						m_BlendLength,
								m_WaitingForAnim,
								m_StartupAnimType;

	INT16						m_Mode ;			// General purpose mode for fading out ect // use bitfield for mode?

	FLOAT						m_ModeTime ;	// Current mode time
	FLOAT						m_ModeMisc1 ;	// Miscilaneous variable 1

	float						m_uBlender,
								m_BlendPos;

	struct CBoss_t			*m_pBoss ;	// Pts to boss vars or NULL

	void (*m_pCollisionFunction)(void *pThis, struct CCollisionInfo2_t *pCollisionInfo) ;
	CQuatern(*m_pCalculateOrientation)(void *pThis) ;

	BYTE							m_BlendStart,
									m_BlendFinish;

#ifndef WIN32
	INT16							m_ActiveSwooshes ;	// Actice swooshes on object		// use bitfield

	FLOAT							m_HeadTrackBlend ;		// 0 = all anim, 1 = all tracking!

	FLOAT							m_HeadTurnAngle ;			// Head Y axis rotation
	FLOAT							m_HeadTurnAngleVel ;		// "" "" velocity

	FLOAT							m_HeadTiltAngle ;			// Head X axis rotation
	FLOAT							m_HeadTiltAngleVel ;		// ""	"" velocity

	struct CHeadTrackInfo_t	*m_pHeadTrackInfo ;	// Ptr to head track info or NULL if none
#endif

	CAIDynamic				m_AI;
	struct CGameObjectInstance_t	*m_pEventCGameObjectInstance;							// move to CAIDynamic
	struct CGameObjectInstance_t	*m_pAttackerCGameObjectInstance;						// move to CAIDynamic

} CGameObjectInstance;


// CGameObjectInstance modes
/////////////////////////////////////////////////////////////////////////////
enum CGameObjectInstanceModes
{
	IDLE_MODE,					// Do nothing
	TRANS_FADE_OUT_MODE,		// Fade out to transparent
	END_OF_MODES
} ;


// CGameObjectInstance anim draw structure
/////////////////////////////////////////////////////////////////////////////
#ifndef WIN32
typedef struct CAnimDraw_t
{
   Gfx                  **ppDLP;
   CIndexedSet          *pisNodes,
                        *pisCurrentAnim,
                        *pisBlendAnim;
   BOOL                 RemoveRootXZ,
                        Better,
								Inter;
} CAnimDraw;
#endif

// CGameObjectInstance operations
/////////////////////////////////////////////////////////////////////////////

void 		CGameObjectInstance__TakeFromROMObjectInstance(CGameObjectInstance *pThis, CROMObjectInstance *pSource,
																		  CGameRegion Regions[], CEnemyAttributes Variations[]);

#define	CGameObjectInstance__GetOrientationMatrix(pThis) ((pThis)->m_mOrientation[even_odd])

#define	CGameObjectInstance__IsVisible(pThis) ((pThis)->m_dwFlags & ANIM_INSTANCE_VISIBLE)
#define  CGameObjectInstance__SetVisible(pThis) ((pThis)->m_dwFlags |= ANIM_INSTANCE_VISIBLE)
#define  CGameObjectInstance__SetNotVisible(pThis) ((pThis)->m_dwFlags &= ~ANIM_INSTANCE_VISIBLE)

#define	CGameObjectInstance__IsBlending(pThis) ((pThis)->m_dwFlags & ANIM_INSTANCE_BLENDING)
#define  CGameObjectInstance__SetBlending(pThis) ((pThis)->m_dwFlags |= ANIM_INSTANCE_BLENDING)
#define  CGameObjectInstance__SetNotBlending(pThis) ((pThis)->m_dwFlags &= ~ANIM_INSTANCE_BLENDING)

#define	CGameObjectInstance__IsAnimChanged(pThis) ((pThis)->m_dwFlags & ANIM_INSTANCE_ANIMCHANGED)
#define	CGameObjectInstance__SetAnimChanged(pThis) ((pThis)->m_dwFlags |= ANIM_INSTANCE_ANIMCHANGED)
#define	CGameObjectInstance__SetAnimNotChanged(pThis) ((pThis)->m_dwFlags &= ~ANIM_INSTANCE_ANIMCHANGED)

#define	CGameObjectInstance__IsGone(pThis) ((pThis)->m_dwFlags & ANIM_INSTANCE_GONE)
#define  CGameObjectInstance__SetGone(pThis) ((pThis)->m_dwFlags |= ANIM_INSTANCE_GONE)
#define  CGameObjectInstance__SetNotGone(pThis) ((pThis)->m_dwFlags &= ~ANIM_INSTANCE_GONE)

/*
// didn't work
#define	CGameObjectInstance__IsFirstBlendFrame(pThis) ((pThis)->m_dwFlags & ANIM_INSTANCE_FIRSTBLENDFR)
#define  CGameObjectInstance__SetFirstBlendFrame(pThis) ((pThis)->m_dwFlags |= ANIM_INSTANCE_FIRSTBLENDFR)
#define  CGameObjectInstance__SetNotFirstBlendFrame(pThis) ((pThis)->m_dwFlags &= ~ANIM_INSTANCE_FIRSTBLENDFR)

#define	CGameObjectInstance__IsBlendingToRight(pThis) ((pThis)->m_dwFlags & ANIM_INSTANCE_BLENDTORIGHT)
#define  CGameObjectInstance__SetBlendingToRight(pThis) ((pThis)->m_dwFlags |= ANIM_INSTANCE_BLENDTORIGHT)
#define  CGameObjectInstance__SetNotBlendingToRight(pThis) ((pThis)->m_dwFlags &= ~ANIM_INSTANCE_BLENDTORIGHT)
*/

#define	CGameObjectInstance__IsDevice(pThis) ((pThis)->m_dwFlags & ANIM_INSTANCE_DEVICE)
#define	CGameObjectInstance__HasTransparency(pThis) ((pThis)->m_dwFlags & ANIM_INSTANCE_TRANSPARENCY)

void 		CGameObjectInstance__CalculateOrientationMatrix(CGameObjectInstance *pThis, BOOL FollowView, CVector3 vTCorners[8], CMtxF mfOrient);
void 		CGameObjectInstance__SetDesiredAnim(CGameObjectInstance *pThis, int nAnim);
void		CGameObjectInstance__RestartAnim(CGameObjectInstance *pThis);
int		CGameObjectInstance__LookupAIAnimType(CGameObjectInstance *pThis, int nType);
int		CGameObjectInstance__GetRandomAnimIndex(CGameObjectInstance *pThis, int nItems, int AITypes[], int Weights[], int *pPickedType);
int		CGameObjectInstance__GetRandomAnimIndexWithCheck(CGameObjectInstance *pThis, int nItems, int Info[][3], int *pPickedType);
#define	CGameObjectInstance__GetCurrentAnim(pThis) ((pThis)->m_asCurrent.m_nAnim)
#define	CGameObjectInstance__IsModelLoaded(pThis) CGeometry__IsModelLoaded(&(pThis)->m_Geometry)
void		CGameObjectInstance__Jerk(CGameObjectInstance *pThis);

#define	CGameObjectInstance__IsCycleCompleted(pThis) CGameAnimateState__IsCycleCompleted(&(pThis)->m_asCurrent)

#define	CGameObjectInstance__TypeFlag(pThis)	((pThis)->m_nTypeFlag)

#ifndef WIN32
void 		CGameObjectInstance__Reset(CGameObjectInstance *pThis,
										  		CIndexedSet *pisObjectsIndex, ROMAddress *rpObjectsAddress,
												int StartupAnimType);	// = -1
void		CGameObjectInstance__PreDrawPlayer(CGameObjectInstance *pThis, Gfx **ppDLP);
void 		CGameObjectInstance__Draw(CGameObjectInstance *pThis, Gfx **ppDLP, CCacheEntry *pceTextureSetsIndex);

void		CGameObjectInstance__GetOffsetPositionAndRegion(CGameObjectInstance *pThis,
																		   CVector3 vDesiredOffset,
																		   CVector3 *pvOutPos, CGameRegion **ppOutRegion);
CQuatern CGameObjectInstance__GetAimRotation(CGameObjectInstance *pThis);
CQuatern CGameObjectInstance__GetAutoAimRotation(CGameObjectInstance *pThis, CVector3 vPos, CQuatern *pqFixResult, float Angle);
CVector3 CGameObjectInstance__GetRotatedDirection(CGameObjectInstance *pThis, CVector3 vDirection);
void		CGameObjectInstance__WentOutOfAnimBounds(CGameObjectInstance *pThis);
BOOL		CGameObjectInstance__IsFacingCliff(CGameObjectInstance *pThis);

#endif

/////////////////////////////////////////////////////////////////////////////

typedef struct CROMGeometryInfo_t
{
	DWORD		unused;		// put object material, or whatever else here
} CROMGeometryInfo;

// CROMGeometryInfo operations
/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
void     CROMGeometryInfo__TakeFromGraphicObject(CROMGeometryInfo *pThis, CGraphicObject *pSource);
#endif

/////////////////////////////////////////////////////////////////////////////

typedef struct CGameGeometryInfo_t
{
	DWORD		m_MorphFrameNumber;
	float		m_cMorph;
} CGameGeometryInfo;

// CROMGeometryInfo operations
/////////////////////////////////////////////////////////////////////////////

void     CGameGeometryInfo__TakeFromROMGeometryInfo(CGameGeometryInfo *pThis, CROMGeometryInfo *pSource);

/////////////////////////////////////////////////////////////////////////////

typedef struct CROMInitialOrientation_t
{
   CVector3		m_vPos;
	//CQuatern    m_qRot;
	//SF			m_PosX, m_PosY, m_PosZ, m_unused;
	INT16		m_RotX, m_RotY, m_RotZ, m_RotT;
} CROMInitialOrientation;

// CROMInitialOrientation operations
/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
void     CROMInitialOrientation__TakeFromNode(CROMInitialOrientation *pThis,
															 CNode *pNode, int nAnim, CVector3 vTranslationScaler,
															 float RemoveYRotValue);
#endif

/////////////////////////////////////////////////////////////////////////////

typedef struct CROMNodeIndex_t
{
	short		m_nTranslationSet,
				m_nRotationSet;
} CROMNodeIndex;

// CROMNodeIndex operations
/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
void		CROMNodeIndex__TakeFromShorts(CROMNodeIndex *pThis,
													short nTranslationSet, short nRotationSet);
#endif

/////////////////////////////////////////////////////////////////////////////

typedef struct CRotFrame_t
{
   //CQuatern    m_qRot;
	INT16		x, y, z, t;
} CRotFrame;

// CRotFrame operations
/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
//void     CRotFrame__TakeFromQuaternion(CRotFrame *pThis, CQuaternion *pSource);
#endif

/////////////////////////////////////////////////////////////////////////////

typedef struct CTranslationOffset_t
{
	BOOL			m_Compressed;
	CVector3		m_vOffset;
	float			m_Scale;
} CTranslationOffset;

// CTranslationOffset operations
/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
void     CTranslationOffset__TakeFromOffsetScale(CTranslationOffset *pThis, CVector3 vOffset, float Scale, BOOL Compressed);
#endif

/////////////////////////////////////////////////////////////////////////////

typedef struct CPosFrame_t
{
   CVector3    m_vPos;
} CPosFrame;

// CPosFrame operations
/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
void     CPosFrame__TakeFromVector3(CPosFrame *pThis, CVector3 *pSource);
#endif

/////////////////////////////////////////////////////////////////////////////

#ifndef WIN32		// already defined in model.h
#define VERTEX_CACHE_SIZE				32
#endif

#define POLYSTREAM_VA_POS        	0
#define POLYSTREAM_VB_POS        	5
#define POLYSTREAM_VC_POS        	10
#define POLYSTREAM_FLAGS_POS     	15

#define POLYSTREAM_VA(val)       	((val) << POLYSTREAM_VA_POS)
#define POLYSTREAM_GETVA(val)    	(((val) >> POLYSTREAM_VA_POS) & (VERTEX_CACHE_SIZE-1))

#define POLYSTREAM_VB(val)       	((val) << POLYSTREAM_VB_POS)
#define POLYSTREAM_GETVB(val)    	(((val) >> POLYSTREAM_VB_POS) & (VERTEX_CACHE_SIZE-1))

#define POLYSTREAM_VC(val)       	((val) << POLYSTREAM_VC_POS)
#define POLYSTREAM_GETVC(val)    	(((val) >> POLYSTREAM_VC_POS) & (VERTEX_CACHE_SIZE-1))

#define POLYSTREAM_LASTINRUN     	(0x1 << POLYSTREAM_FLAGS_POS)
#define POLYSTREAM_ISLASTINRUN(val)	((val) & POLYSTREAM_LASTINRUN)

/////////////////////////////////////////////////////////////////////////////

typedef struct CROMSection_t
{
	DWORD				m_nTextureSet,
						m_dwMatFlags;						// see matdefs.h
	BYTE				m_Color[4],							// RGBA
						m_BlackColor[4];
	WORD				m_MultU, m_MultV;
} CROMSection;

// CROMSection operations
/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
void		CROMSection__TakeFromMaterial(CROMSection *pThis, CMaterial *pSource, BOOL PreLit, BOOL ShadeAlpha,
													WORD MultU, WORD MultV);
#endif

/////////////////////////////////////////////////////////////////////////////

typedef struct CGameSection_t
{
	DWORD				m_dwMatFlags;						// see matdefs.h
#ifndef WIN32
	CTextureLoader	m_TextureLoader;
#endif
	BYTE				m_Color[4],							// RGBA
						m_BlackColor[4];
	WORD				m_MultU, m_MultV;
} CGameSection;

// CGameSection operations
/////////////////////////////////////////////////////////////////////////////

void 		CGameSection__TakeFromRomSection(CGameSection *pThis, CROMSection *pSource,
														CCacheEntry *pceTextureSetsIndex);

/////////////////////////////////////////////////////////////////////////////

typedef struct CROMTextureFormat_t
{
#define TEXTURE_FORMAT_CI8_RGBA	0
#define TEXTURE_FORMAT_CI4_RGBA	1
#define TEXTURE_FORMAT_32_RGBA	2
#define TEXTURE_FORMAT_CI4_IA		3
#define TEXTURE_FORMAT_MM16RGBA	4
	BYTE		m_Format,
				m_PlaybackSpeed,
				m_WidthShift,
				m_HeightShift,
				m_Effect,
#define TEXTURE_EFFECT_NONE		0
#define TEXTURE_EFFECT_SHARPEN	1
#define TEXTURE_EFFECT_DETAIL_X2	2
#define TEXTURE_EFFECT_DETAIL_X4	3
				m_EffectMode;
	// padding is not necessary because all types are BYTEs
} CROMTextureFormat;

// CROMTextureFormat operations
/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
void		CROMTextureFormat__TakeFromTextureSet(CROMTextureFormat *pThis, CTextureSet *pSource);
#endif

/////////////////////////////////////////////////////////////////////////////

#define LEVEL_STORE_MAP			(1 << 0)
#define LEVEL_LOW_PRECISION	(1 << 1)

typedef struct CROMLevel_t
{
	float				m_GridDistance;
	BYTE				m_BlackColor[3],
						m_WhiteColor[3],
						m_DirectionalLight[3],
						m_AmbientLight[3],
						m_bFlags;
	signed char		m_Direction[3];
} CROMLevel;

// CROMLevel operations
/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
void		CROMLevel__TakeFromLevel(CROMLevel *pThis, CLevel *pSource, float GridDistance);
#endif

/////////////////////////////////////////////////////////////////////////////

typedef struct CROMParticleImpact_t
{
	float			m_ImpactEventNumber[PARTICLE_IMPACTS_AMT];
	WORD			m_ImpactParticleType[PARTICLE_IMPACTS_AMT],
					m_ImpactEventType[PARTICLE_IMPACTS_AMT],
					m_ImpactSoundType[PARTICLE_IMPACTS_AMT];
#if (PARTICLE_IMPACTS_AMT & 0x1)
	WORD			m_UnusedWord;
#endif
} CROMParticleImpact;

/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
void CROMParticleImpact__TakeFromParticleEffect(CROMParticleImpact *pThis, CParticleEffect *pSource);
#endif

/////////////////////////////////////////////////////////////////////////////

typedef struct CROMParticleEffect_t
{
	CROMParticleImpact *m_pImpact;		// stored on cart as index!
	DWORD			m_dwFlags;
	SF				m_BounceEnergy,
					m_DirectionSpray,
					m_SprayX,
					m_SprayY,
					m_SprayZ,
					m_DirectionX,
					m_DirectionY,
					m_DirectionZ,
					m_Gravity,
					m_GravityRandom,
					m_GroundFriction,
					m_FpsVelThreshold,
					m_AirFriction,
					m_WaterFriction,
					m_Size,
					m_SizeRandom,
					m_Scaler,
					m_ScalerRandom,
					m_Velocity,
					m_VelocityRandom,
					m_PosRandomX,
					m_PosRandomY,
					m_PosRandomZ,
					m_Rotation,
					m_RotationRandom,
					m_RotationInc,
					m_RotationIncRandom,
					m_RotationPivotX,
					m_RotationPivotY,
					m_PosX,
					m_PosY,
					m_PosZ;
	short			m_nTextureSet,
					m_nParticles,
					m_nParticleRandom,
					m_nParticleFrames,
					m_nParticleFrameRandom,
					m_nParticleDelayFramesRandom;
	BYTE			m_FPS,
					m_InstanceBehavior,
					m_WallBehavior,
					m_GroundBehavior,
					m_Alignment,
					m_WhiteColor[3],
					m_BlackColor[3],
					m_RandomizeHue,
					m_RandomizeSaturation,
					m_RandomizeBrightness,
					m_Priority,
					m_Playback,
					m_Visibility,
					m_nFadeInDelay,
					m_nFadeOutDelay,
					unused;

} CROMParticleEffect;


// CROMParticleEffect operations
/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
void		CROMParticleEffect__TakeFromParticleEffect(CROMParticleEffect *pThis, CParticleEffect *pSource, int nImpact);
#endif

/////////////////////////////////////////////////////////////////////////////

typedef struct CROMWarpPoint_t
{
	CVector3		m_vPos;
	float			m_RotY;
	WORD			m_nLevel,
					m_nRegion;
} CROMWarpPoint;

// CROMWarpPoint operations
/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
void		CROMWarpPoint__TakeFromObjectInstance(CROMWarpPoint *pThis, CObjectInstance *pSource,
															  int nLevel, int nRegion);
#endif

/////////////////////////////////////////////////////////////////////////////

typedef struct CROMRegionSet_t
{
	BYTE			m_FogColor[4],
					m_WaterColor[4];
	DWORD			m_FogStart,
					m_WaterStart;
	float			m_FarClip,
					m_FieldOfView,
					m_WaterFarClip,
					m_WaterFieldOfView,
					m_WaterElevation,
					m_SkyElevation,
					m_SkySpeed,
					m_BlendLength,
					m_DeathTimeDelay;
	DWORD			m_dwFlags;
	WORD			m_WarpID,
					m_PressurePlateSoundNumber,
					m_SaveCheckpointID,
					m_PressureID,
					m_DeathHitPoints;
	BYTE			m_GroundMat,
					m_WallMat,
					m_MusicID,
					m_AmbientSounds,
					m_MapColor,
					m_bActiveFlags;
} CROMRegionSet;

// CROMRegionSet operations
/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
void		CROMRegionSet__TakeFromRegionSet(CROMRegionSet *pThis, CRegionSet *pSource);
#endif

/////////////////////////////////////////////////////////////////////////////

typedef struct CROMEnvelope_t
{
	SF					m_MaxValue,
						m_StartFactor, m_EndFactor;
	WORD				m_nDuration, m_EnvStartTime;

//	float				m_MaxValue,
//						m_StartFactor, m_EndFactor;
//	DWORD				m_nDuration, m_EnvStartTime;

} CROMEnvelope;

///////////////////////////////////////////////////////////////////////////

typedef struct CROMSoundElement_t
{
	WORD				m_nSampleNum,
						m_nDelayTime,
						m_Priority,
						m_wFlags,
						m_Probability;
	CROMEnvelope	m_Volume,
						m_Pitch;
	BYTE				m_RadioVolume,
						unused2;

//	DWORD				m_nSampleNum,
//						m_nDelayTime,
//						m_Priority,
//						m_dwFlags,
//						m_Probability;
//	CROMEnvelope	m_Volume,
//						m_Pitch;
//	BYTE				m_RadioVolume,
//						unused2,
//						unused3,
//						unused4;
} CROMSoundElement;

// CROMSoundElement operations
/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
void		CROMSoundElement__TakeFromElement(CROMSoundElement *pThis, CSoundElement *pSource);
#endif

///////////////////////////////////////////////////////////////////////////
#endif // _INC_ROMSTRUC
