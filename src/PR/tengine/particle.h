// particle.h

#ifndef _INC_PARTICLE
#define _INC_PARTICLE

#include "romstruc.h"
#include "parttype.h"
#include "textload.h"
#include "collinfo.h"

/////////////////////////////////////////////////////////////////////////////

#define	PARTICLE_ALLOCATED		(1 << 0)
#define	PARTICLE_HASHITGROUND	(1 << 1)
#define	PARTICLE_HASHITWALL		(1 << 2)
#define	PARTICLE_PERMANENT 		(1 << 3)	// Can't be deleted if set
#define	PARTICLE_DELETE	 		(1 << 4)	// Deallocate as soon as possible

typedef struct CParticle_t
{
	CAnimInstanceHdr		ah;

	Mtx						m_mOrientation[2];
	CROMParticleEffect	*m_pEffect;
	CTextureLoader			m_TextureLoader;
	CInstanceHdr			*m_pSource;
	DWORD						m_dwFlags;
	float						m_cFrame,
								m_cFramePos,
								m_FPS;
	int						m_nFrames,
								m_cAnimFrame,
								m_nRandomFrame;
	float						m_Size, m_SizeScaler,
								m_Rotation,
								m_RotationInc,
								m_DistanceSquared;
	UINT8						m_AlphaScaler ;
	UINT8						m_ActiveSwooshes ;
	CCollisionInfo2		m_CI;
	short						m_Alignment;
	BYTE						m_WhiteColor[3],
								m_BlackColor[3];

	struct CParticle_t	*m_pLast, *m_pNext,
								*m_pDrawLast, *m_pDrawNext;
} CParticle;

// Globals
extern	CVector3				v_particle_glare_pos;
extern	BYTE					particle_glare_color[];
extern	CParticle			*p_particle_glare;


// CParticle operations
/////////////////////////////////////////////////////////////////////////////

void		CParticle__Construct				(CParticle *pThis);

void		CParticle__Draw(CParticle *pThis, Gfx **ppDLP, CCartCache *pCache);

void		CParticle__Delete							(CParticle *pThis);
void		CParticle__Spawn							(CParticle *pThis, int nType, BOOL Blood);
void		CParticle__SpawnWallCollision			(CParticle *pThis, int nType);
void		CParticle__SpawnGroundCollision		(CParticle *pThis, int nType);
void		CParticle__SpawnCeilingCollision		(CParticle *pThis, int nType);
void		CParticle__SpawnWaterSurfaceCollision(CParticle *pThis, int nType);
void		CParticle__Advance						(CParticle *pThis);
void		CParticle__WallCollision				(CParticle *pThis);
void		CParticle__GroundCollision				(CParticle *pThis);
void		CParticle__CeilingCollision			(CParticle *pThis);
void		CParticle__WaterSurfaceCollision		(CParticle *pThis);
void		CParticle__InstanceCollision			(CParticle *pThis);
void		CParticle__EveryFrame					(CParticle *pThis);
void		CParticle__EndLife						(CParticle *pThis);
void		CParticle__Done							(CParticle *pThis);
BOOL		CParticle__IsInAntiGrav					(CParticle *pThis);
BOOL		CParticle__IsInWater						(CParticle *pThis);
#define	CParticle__DoSoundEffect(pThis, nSoundType, Looping, pvPos) CScene__DoSoundEffect(&GetApp()->m_Scene, nSoundType, Looping, pThis, pvPos, 0)

#define	CParticle__GetRandom(max) ((((int)max) < 0) ? -RANDOM((-((int)max))+1) : (((max) == 0) ? 0 : RANDOM((max) + 1)))
#define	CParticle__GetRandomFloat(max) ((max == 0.0) ? 0.0 : RandomFloat(max))


/////////////////////////////////////////////////////////////////////////////

typedef struct CParticleSystem_t
{
	CParticle	m_Particles[PARTICLES_MAX_COUNT],
					*m_pFreeHead,
					*m_pActiveHead, *m_pActiveTail,
					*m_pDrawHead, *m_pDrawTail;
	CCartCache	*m_pCache;
   CCacheEntry	*m_pceParticleEffects,
					*m_pceTextureSetsIndex;
} CParticleSystem;

// CParticleSystem operations
/////////////////////////////////////////////////////////////////////////////

void			CParticleSystem__Construct(CParticleSystem *pThis);
void			CParticleSystem__RequestData(CParticleSystem *pThis, CCartCache *pCache,
													  ROMAddress *rpParticleEffectsAddress,
													  DWORD ParticleEffectsSize,
													  CCacheEntry *pceTextureSetsIndex);

#define PARTICLE_SCREENY	-6
#define PARTICLE_UP			-5
#define PARTICLE_SCREEN		-4
#define PARTICLE_GROUND		-3
#define PARTICLE_CEILING	-2
#define PARTICLE_NOEDGE		-1
void			CParticleSystem__CreateParticle(CParticleSystem *pThis, CInstanceHdr *pSource,
														  int nType,
														  CVector3 vBaseVelocity, CQuatern qRotateDirection,
														  CVector3 vPos, CGameRegion *pRegion, int nEdge /*=PARTICLE_NOEDGE*/, BOOL Blood);
void			CParticleSystem__DeallocateParticle(CParticleSystem *pThis, CParticle *pParticle);
void			CParticleSystem__DeallocateAllParticles(CParticleSystem *pThis);
void			CParticleSystem__Draw(CParticleSystem *pThis, Gfx **ppDLP);
void			CParticleSystem__DetonateGrenade(CParticleSystem *pThis);
void			CParticleSystem__DetonateAllGrenades(CParticleSystem *pThis);

/////////////////////////////////////////////////////////////////////////////
#endif // _INC_PARTICLE
