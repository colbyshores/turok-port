// graphu64.h

#ifndef _INC_GRAPHU64
#define _INC_GRAPHU64

#include "rommodel.h"


// forward declarations
struct CQuatern_t;

#ifndef WIN32
/////////////////////////////////////////////////////////////////////////////

typedef struct CVector3_t
{
	float		x, y, z;
} CVector3;

// CVector3 operations
/////////////////////////////////////////////////////////////////////////////

void		CVector3__Add(CVector3 *pvThis, CVector3 *pvLeft, CVector3 *pvRight);
void 		CVector3__Subtract(CVector3 *pvThis, CVector3 *pvLeft, CVector3 *pvRight);
void		CVector3__MultScaler(CVector3 *pvThis, CVector3 *pvIn, float Scaler);
void		CVector3__Blend(CVector3 *pvThis, float u, CVector3 *pvLeft, CVector3 *pvRight);
void		CVector3__Cross(CVector3 *pvThis, CVector3 *pvLeft, CVector3 *pvRight);
#define	CVector3__Dot(pvLeft, pvRight) ((pvLeft)->x*(pvRight)->x + (pvLeft)->y*(pvRight)->y + (pvLeft)->z*(pvRight)->z)
#define	CVector3__MagnitudeSquared(pvThis) ((pvThis)->x*(pvThis)->x + (pvThis)->y*(pvThis)->y + (pvThis)->z*(pvThis)->z)
#define	CVector3__Magnitude(pvThis) sqrt(CVector3__MagnitudeSquared(pvThis))
#define	CVector3__Set(pThis,X,Y,Z)	(pThis)->x = X ; (pThis)->y = Y ; (pThis)->z = Z ; 
void		CVector3__Normalize(CVector3 *pvThis);
CVector3 CVector3__RandomizeDirection(CVector3 *pvThis, float uRandom);
CVector3 CVector3__RandomizeDirectionX(CVector3 *pvThis, float uRandom);
CVector3 CVector3__RandomizeDirectionY(CVector3 *pvThis, float uRandom);
CVector3 CVector3__RandomizeDirectionZ(CVector3 *pvThis, float uRandom);
void		CVector3__YIntersect(CVector3 *pvThis, CVector3 *pvStart, CVector3 *pvEnd, float Y);
CVector3	CVector3__Reflect(CVector3 *pThis, CVector3 *pvSurfaceUnitNormal, float EnergyFactor);
CVector3 CVector3__Project(CVector3 *pvThis, CVector3 *pvSurfaceUnitNormal);
CVector3 CVector3__ProjectCrease(CVector3 *pvThis, CVector3 *pvNormalA, CVector3 *pvNormalB);
CVector3 CVector3__Redirect(CVector3 *pvThis, CVector3 *pvSurfaceUnitNormal);
BOOL		CVector3__Clamp(CVector3 *pvThis, float MaxVal);
void		CVector3__Print(CVector3 *pvThis);
#define	CVector3__IsEqual(pvLeft, pvRight) (((pvLeft)->x == (pvRight)->x) && ((pvLeft)->y == (pvRight)->y) && ((pvLeft)->z == (pvRight)->z))
struct CQuatern_t CVector3__GetDirectionRotation(CVector3 *pvThis);

FLOAT CVector3__XZAngle(CVector3 *pThis) ;
FLOAT CVector3__XYAngle(CVector3 *pThis) ;
FLOAT CVector3__DistSqr(CVector3 *pThis, CVector3 *pvPos) ;
FLOAT CVector3__Dist(CVector3 *pThis, CVector3 *pvPos) ;


/////////////////////////////////////////////////////////////////////////////
#endif

typedef float CMtxF[4][4];

typedef struct CQuatern_t
{
	float		x, y, z, t;
} CQuatern;

// CMtxF operations
/////////////////////////////////////////////////////////////////////////////

//#define 	CMtxF__ToMtx(mfThis, mFixed) guMtxF2L((mfThis), &(mFixed))
#define	CMtxF__ToMtx(mfThis, mFixed) CMtxF__ToMtxFast((mfThis), &(mFixed))
#define 	CMtxF__FromMtx(mfThis, mFixed) guMtxL2F((mfThis), &(mFixed))
void 		CMtxF__Identity(CMtxF mfThis);
void 		CMtxF__PostMult4x4(CMtxF mfThis, CMtxF mfLeft, CMtxF mfRight);
void		CMtxF__Clamp(CMtxF mfThis) ;
void 		CMtxF__VectorMult(CMtxF mfThis, const CVector3 *pvIn, CVector3 *pvOut);
void 		CMtxF__VectorMult4x4(CMtxF mfThis, const CVector3 *pvIn, CVector3 *pvOut);
void 		CMtxF__Translate(CMtxF mfThis, float X, float Y, float Z);
void 		CMtxF__Scale(CMtxF mfThis, float X, float Y, float Z);
void 		CMtxF__Rotate(CMtxF mfThis, float Theta, float X, float Y, float Z);
void 		CMtxF__Print(CMtxF mfThis);
void		CMtxF__Copy(CMtxF mfThis, CMtxF mfSource);
void		CMtxF__Invert(CMtxF mfThis, CMtxF mfIn);



// Optimised matrix operations (assumes column 4 is {0,0,0,1}
void CMtxF__PostMult(CMtxF mfThis, CMtxF mfLeft, CMtxF mfRight) ;

void CMtxF__PreMultTranslate(CMtxF mfThis, FLOAT Tx, FLOAT Ty, FLOAT Tz) ;
void CMtxF__PostMultTranslate(CMtxF mfThis, FLOAT Tx, FLOAT Ty, FLOAT Tz) ;

void CMtxF__PreMultScale(CMtxF mfThis, FLOAT Sx, FLOAT Sy, FLOAT Sz) ;
void CMtxF__CopyPreMultScale(CMtxF mfThis, CMtxF mfSource, FLOAT Sx, FLOAT Sy, FLOAT Sz) ;

void CMtxF__PostMultScale(CMtxF mfThis, FLOAT Sx, FLOAT Sy, FLOAT Sz) ;

void CMtxF__RotateX(CMtxF mfThis, float Theta) ;
void CMtxF__RotateY(CMtxF mfThis, float Theta) ;
void CMtxF__RotateZ(CMtxF mfThis, float Theta) ;

void CMtxF__PreMultRotateX(CMtxF mfThis, FLOAT RotX) ;
void CMtxF__PostMultRotateX(CMtxF mfThis, FLOAT RotX) ;

void CMtxF__PreMultRotateY(CMtxF mfThis, FLOAT RotY) ;
void CMtxF__PostMultRotateY(CMtxF mfThis, FLOAT RotY) ;

void CMtxF__PreMultRotateZ(CMtxF mfThis, FLOAT RotZ) ;
void CMtxF__PostMultRotateZ(CMtxF mfThis, FLOAT RotZ) ;

void CMtxF__PreMult3DSToU64(CMtxF mfThis) ;
void CMtxF__CopyPreMult3DSToU64(CMtxF mfThis, CMtxF mfSource) ;

void CMtxF__PostMult3DSToU64(CMtxF mfThis) ;
void CMtxF__CopyPostMult3DSToU64(CMtxF mfThis, CMtxF mfSource) ;

#ifndef WIN32
void CMtxF__ToMtxFast(CMtxF mfThis, Mtx *m) ;
void CMtxF__ToMtxPostMultTranslate(CMtxF mfThis, Mtx *m, FLOAT Tx, FLOAT Ty, FLOAT Tz) ;
#endif

/////////////////////////////////////////////////////////////////////////////

//#define		BlendFLOAT(u, A, B) ((1.0-(u))*(A) + (u)*(B))
#define		BlendFLOAT(u, A, B) ((A) + (u)*(((float)(B)) - (A)))
#ifndef WIN32
#define		GetCosineBlender(u) ((cos((1.0-(u))*ANGLE_PI) + 1.0)/2.0)
#define		GetHermiteBlender(r1, r2, u) ( (-2.0 + ((float)r2) + ((float)r1))*((float)u)*((float)u)*((float)u) + (3.0 - ((float)r2) - 2.0*((float)r1))*((float)u)*((float)u) + ((float)r1)*((float)u) )
#endif
#define		BlendFLOATCosine(u, A, B) (BlendFLOAT(GetCosineBlender(u), A, B))

float			BlendRotFLOAT(float u, float A, float B);
float			RotDifference(float A, float B);
void			NormalizeRotation(float *pTheta);
void			NormalizeBigRotation(float *pTheta);

// CQuatern operations
/////////////////////////////////////////////////////////////////////////////

#define		CQuatern__BuildFromAxisAngle(pqThis, X, Y, Z, Theta)	\
						sin_angle = sin((Theta)/2);							\
						(pqThis)->x = (X)*sin_angle;                    \
						(pqThis)->y = (Y)*sin_angle;                    \
						(pqThis)->z = (Z)*sin_angle;                    \
						(pqThis)->t = cos((Theta)/2);
#define		CQuatern__Identity(pqThis) { (pqThis)->x = (pqThis)->y = (pqThis)->z = 0; (pqThis)->t = 1; }

#define		CQuatern__Add(pqThis, pqLeft, pqRight)				\
						(pqThis)->x = (pqLeft)->x + (pqRight)->x;	\
						(pqThis)->y = (pqLeft)->y + (pqRight)->y; \
						(pqThis)->z = (pqLeft)->z + (pqRight)->z; \
						(pqThis)->t = (pqLeft)->t + (pqRight)->t;
#define		CQuatern__MultScaler(pqThis, pqIn, Scaler)	\
						(pqThis)->x = (pqIn)->x * Scaler;		\
						(pqThis)->y = (pqIn)->y * Scaler;		\
						(pqThis)->z = (pqIn)->z * Scaler;		\
						(pqThis)->t = (pqIn)->t * Scaler;
void			CQuatern__Mult(CQuatern *pqThis, const CQuatern *pqLeft, const CQuatern *pqRight);
#define		CQuatern__Dot(pqLeft, pqRight) \
						((pqLeft)->x*(pqRight)->x + (pqLeft)->y*(pqRight)->y + (pqLeft)->z*(pqRight)->z + (pqLeft)->t*(pqRight)->t)
#define		CQuatern__MagnitudeSquared(pqThis) \
						((pqThis)->x*(pqThis)->x + (pqThis)->y*(pqThis)->y + (pqThis)->z*(pqThis)->z + (pqThis)->t*(pqThis)->t)
#define 		CQuatern__Magnitude(pqThis) sqrt(CQuatern__MagnitudeSquared(pqThis))
void			CQuatern__Normalize(CQuatern *pqThis);
#define		CQuatern__Inverse(pqThis, pqIn) 												\
						ASSERT(CQuatern__MagnitudeSquared(pqIn) != 0);              \
						inv_mag_squared = (-1.0)/CQuatern__MagnitudeSquared(pqIn);  \
						(pqThis)->x = (pqIn)->x * inv_mag_squared;                  \
						(pqThis)->y = (pqIn)->y * inv_mag_squared;                  \
						(pqThis)->z = (pqIn)->z * inv_mag_squared;                  \
						(pqThis)->t = (pqIn)->t * -inv_mag_squared;
#define		CQuatern__Negate(pqThis)				\
						(pqThis)->x = -((pqThis)->x); \
						(pqThis)->y = -((pqThis)->y); \
						(pqThis)->z = -((pqThis)->z); \
						(pqThis)->t = -((pqThis)->t);
void			CQuatern__ToMatrix(CQuatern *pqThis, CMtxF mfRotate);
void 			CQuatern__BlendLinear(CQuatern *pqThis, float u, CQuatern *pqA, CQuatern *pqB);
void 			CQuatern__BlendSpherical(CQuatern *pqThis, float u, CQuatern *pqA, CQuatern *pqB);
void			CQuatern__BlendThreshold(CQuatern *pqThis, float u, CQuatern *pqA, CQuatern *pqB);
void			CQuatern__GetCloser(CQuatern *pqThis, CQuatern *pqTarget);
void			CQuatern__Print(CQuatern *pqThis);

/////////////////////////////////////////////////////////////////////////////

// utility values
extern float 	sin_angle,
					inv_mag_squared;

extern	DWORD		random_value;
extern	DWORD		sound_random_value;

// utility functions
void InitializeRandomSeed();
int RandomSwapWord();
int SoundRandomSwapWord() ;
float RandomFloat(float Max);
float acos(float x);  // polynomial approximation


/////////////////////////////////////////////////////////////////////////////
#endif // _INC_GRAPHU64
