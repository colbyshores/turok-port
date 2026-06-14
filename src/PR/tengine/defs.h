// defs.h

#ifndef _INC_DEFS
#define _INC_DEFS

/////////////////////////////////////////////////////////////////////////////
// settings

#define	MAKE_CART
#define	SHIP_IT
//#define	IG_DEBUG							// turns on traces and memory validation
//#define 	TRACE_MEM
#define	TRAP_ON_ASSERT
#define	DISPLAY_FRAMERATE
#define	DISPLAY_COORDINATES



#define	ENGLISH
//#define	KANJI
//#define	GERMAN



//#define	AUTO_CHEAT							// turns cheats on by default
//#define	EDIT_FOG_AND_LIGHTS				// allows editing of fog and lights

//#define	SCREEN_SHOT						// trigger does screen shot
#define	USE_ZBUFFER
#define	USE_FOG
#define	USE_FIFO
//#define	SHOW_COVERAGE
//#define 	FIFO_SIZE			0x10000	// must be at least 0x100 (256 bytes)
//#define 	FIFO_SIZE			0x20000	// must be at least 0x100 (256 bytes)
#define 	FIFO_SIZE			0x10000	// must be at least 0x100 (256 bytes)

//#define	GLIST_LEN			16384		// should include space for matrix, texture loads, material settings,
//												// and branch to geometry list for each node
#define	LINELIST_LEN		2048
#define	GLIST_LEN			(16384 - LINELIST_LEN)

#define	FRAME_FPS			15					// 3D anim playback fps
//#define	FRAME_FPS			(60/FRAME_RATE)		// anim playback
//#define	FRAME_RATE			4

#define	REFRESHES_TO_FRAME_INC(nRefreshes, RefreshRate) (((nRefreshes)*FRAME_FPS)/RefreshRate)
#define	FRAME_INC_TO_REFRESHES(frameInc) ((frameInc)*refresh_rate*(1.0/FRAME_FPS))
#define	FRAME_INC_TO_SECONDS(frameInc) ((frameInc)*(1.0/FRAME_FPS))

#define	SECONDS_TO_FRAMES(secs)		((secs)*FRAME_FPS)
#define	DISPLAY_LIST_RESERVE_FRONTEND		3000

#define	MAX_FRAME_SKIP		4			// maximum number of animation frames to skip before slowing down
#define	HASH_ALLOC_BLOCK	512		// should be > maximum entries in hash table to reduce fragmentation
#define	HASH_TABLE_SIZE	473		// should be prime number

#define	COLLISION_MAX_INSTANCES	1024
#define	MAX_ACTIVE_ANIM_INSTANCES	64
#define	STATICEVENTS_MAX_INSTANCES 64

//#define	PARTICLES_MAX_COUNT		128
#define	PARTICLES_MAX_COUNT		2

//#define	MATTABLE_MATRICES			400
#define	MATTABLE_MATRICES			32

#define	MAX_TRANSPARENT_INSTANCES	64

#define	TUROK_HEIGHT 		6.0
#define	TUROK_EYE_HEIGHT	(0.9*TUROK_HEIGHT)

#define	COLLISION_GREASE_FACTOR	0.25
#define	PLAYER_TILT_FACTOR		0.8
#define	PLAYER_ZROT_FACTOR		0.25
#define	NEWGROUNDROT_FACTOR		0.125
#define	NEWSHADOWROT_FACTOR		0.4

//#define	WORLD_LIMIT					18000	// based on 3*v*v=sqrt(32767)
#define	WORLD_LIMIT					(1400*SCALING_FACTOR)

/////////////////////////////////////////////////////////////////////////////
// custom render modes

// gouraud shading in primitive color
#define	G_CC_ROB_SHADEPRIMRGBA							\
		PRIMITIVE, 0, SHADE, 0,								\
		0, 0, 0, PRIMITIVE

// like G_CC_DECALRGBA but multiplies alpha by primitive alpha
#define	G_CC_ROB_DECALRGBA_PRIMALPHA					\
		0, 0, 0, TEXEL0,										\
		TEXEL0, 0, PRIMITIVE, 0

// texture intensity determines color between primitive and environment colors
// alpha equals texture alpha multiplied by primitive alpha
#define	G_CC_ROB_PSEUDOCOLOR_PRIMALPHA				\
		PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT,	\
		TEXEL0, 0, PRIMITIVE, 0

// used for shadow
#define	G_CC_ROB_BLACK_PRIMALPHA						\
		0, 0, 0, 0,												\
		TEXEL0, 0, PRIMITIVE, 0

/////////////////////////////////////////////////////////////////////////////
// types

#define TRUE	1
#define FALSE	0

#define BOOL	int
#define BYTE	unsigned char
#define DWORD	unsigned long
#define WORD	unsigned short
#define INT32  signed long
#define UINT32 unsigned long
#define UINT	unsigned long

#define FLOAT	float
#define UINT16 unsigned short
#define INT16	signed short
#define INT8	signed char
#define UINT8	unsigned char

/////////////////////////////////////////////////////////////////////////////
// macros

#ifdef SHIP_IT
#define MAKE_CART
#endif

#ifdef MAKE_CART

#undef IG_DEBUG
#undef SCREEN_SHOT

#ifdef SHIP_IT
#undef TRAP_ON_ASSERT
#undef DISPLAY_FRAMERATE
#undef DISPLAY_COORDINATES
#endif

#endif


#ifdef SCREEN_SHOT
#undef DISPLAY_FRAMERATE
#endif


#ifdef IG_DEBUG

//#define ASSERT(expr) (expr) ? ((void)0) : igAssert(__FILE__, __LINE__, #expr)
#define ASSERT(expr) (expr) ? ((void)0) : igAssert(__FILE__, __LINE__, NULL)

#else	// !IG_DEBUG

#ifdef SHIP_IT
#define ASSERT(expr)
#else
//#define ASSERT(expr) (expr) ? ((void)0) : igAssert(__FILE__, __LINE__, #expr)
#define ASSERT(expr) (expr) ? ((void)0) : igAssert(__FILE__, __LINE__, NULL)
#endif

#endif

#define PASCAL
#define CYCLES_TO_NSEC	OS_CYCLES_TO_NSEC

extern DWORD		frame_number;
extern int			trap_line; 
extern const char	*trap_file;
extern const char	*trap_expr;
extern int			trap_thread;

#ifdef IG_DEBUG
#define TRACE0(a0) { rmonPrintf("Fr.%d: ",frame_number); rmonPrintf(a0); }
#define TRACE(a0, a1) { rmonPrintf("Fr.%d: ",frame_number); rmonPrintf(a0, a1); }
#define TRACE1(a0, a1) { rmonPrintf("Fr.%d: ",frame_number); rmonPrintf(a0, a1); }
#define TRACE2(a0, a1, a2) { rmonPrintf("Fr.%d: ",frame_number); rmonPrintf(a0, a1, a2); }
#define TRACE3(a0, a1, a2, a3) { rmonPrintf("Fr.%d: ",frame_number); rmonPrintf(a0, a1, a2, a3); }
#define TRACE4(a0, a1, a2, a3, a4) { rmonPrintf("Fr.%d: ",frame_number); rmonPrintf(a0, a1, a2, a3, a4); }
#define TRACE5(a0, a1, a2, a3, a4, a5) { rmonPrintf("Fr.%d: ",frame_number); rmonPrintf(a0, a1, a2, a3, a4, a5); }
#define TRACE6(a0, a1, a2, a3, a4, a5, a6) { rmonPrintf("Fr.%d: ",frame_number); rmonPrintf(a0, a1, a2, a3, a4, a5, a6); }
#else
#define TRACE0(a0)
#define TRACE(a0, a1)
#define TRACE1(a0, a1)
#define TRACE2(a0, a1, a2)
#define TRACE3(a0, a1, a2, a3)
#define TRACE4(a0, a1, a2, a3, a4)
#define TRACE5(a0, a1, a2, a3, a4, a5)
#define TRACE6(a0, a1, a2, a3, a4, a5, a6)
#endif

#define max(a, b)  MAX(a, b)
#define min(a, b)  MIN(a, b)
#define abs(n) (((n) < 0) ? -(n) : (n))
#define fabs(n) abs(n)
#define sqrt(n) sqrtf(n)
#define sin(n) sinf(n)
#define cos(n) cosf(n)
#define floor(n) ((n >= 0) ? ((int)(n)) : ((((int)(n)) == n) ? ((int)(n)) : (((int)(n)) - 1)))
#define ceil(n) ((n <= 0) ? ((int)(n)) : ((((int)(n)) == n) ? ((int)(n)) : (((int)(n)) + 1)))
void romcpy(const char *src, const char *dest, const int len);
void igAssert(const char *szFile, int nLine, const char *szExpr);
int BinarySearch(DWORD Keys[], int nItems, DWORD SearchKey);
BOOL BinaryRange(DWORD Keys[], int nItems, DWORD SearchKey, int *pFirst, int *pLast);
int GetBitStreamSize(int nBits);
BOOL GetBitStreamFlag(BYTE Bits[], int nBit);
void SetBitStreamFlag(BYTE Bits[], int nBit, BOOL Set);

#define ANGLE_PI				3.14159265358979323846
#define ANGLE_DTOR(deg)	   ((deg)*(ANGLE_PI/180))
#define ANGLE_RTOD(radian) (((radian)*180.0)/ANGLE_PI)
#define ANGLE_COS_60			0.5
#define ANGLE_LEDGE_COSINE ANGLE_COS_60
#define SQR(a)					((a) * (a))

#define INVERSE_SQR_TWEEN(t) 	(1.0 - ((1.0-(t))*(1.0-(t))))
#define COSINE_TWEEN(Tween)	(0.5 - (cos((Tween) * ANGLE_PI) / 2.0))
#define SQR_TWEEN(Tween)		((Tween)*(Tween))


//#define CQuatern__Blend(pqThis, u, pqA, pqB) CQuatern__BlendSpherical(pqThis, u, pqA, pqB)
//#define CQuatern__Blend(pqThis, u, pqA, pqB) CQuatern__BlendLinear(pqThis, u, pqA, pqB)
#define CQuatern__Blend(pqThis, u, pqA, pqB) CQuatern__BlendThreshold(pqThis, u, pqA, pqB)

// replace with OS_K0_TO_PHYSICAL(p) if appropriate
#define RSP_ADDRESS(p) (p)
#define RDP_ADDRESS(p) (p)
//#define RSP_ADDRESS(p) OS_K0_TO_PHYSICAL(p)
//#define RDP_ADDRESS(p) OS_K0_TO_PHYSICAL(p)

#define RANDOM(range)		 (RandomSwapWord() % (range))
#define SOUND_RANDOM(range) (SoundRandomSwapWord() % (range))

// fxmode constants
#define	MIN_COL		0
#define	MAX_COL		255

#define	MIN_OPAQ		0
#define	MAX_OPAQ		255

#define	MIN_TRANS	255
#define	MAX_TRANS	0



/////////////////////////////////////////////////////////////////////////////
#endif // _INC_DEFS
