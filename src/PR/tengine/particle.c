// particle.c
//

#include "stdafx.h"
#include "particle.h"
#include "tengine.h"
#include "tmove.h"
#include "cartdir.h"
#include "audio.h"
#include "sfx.h"
#include "regtype.h"
#include "scene.h"
#include "aidoor.h"
#include "boss.h"
#include "fx.h"

#include "debug.h"




#define	USE_ORIGINAL_MATRIX	0


/////////////////////////////////////////////////////////////////////////////
// Angle defines for a pentagon
/////////////////////////////////////////////////////////////////////////////
#define COS0	1
#define COS72	0.3090169
#define COS144	-0.8090169
#define COS216	-0.8090169
#define COS288	0.3090169

#define SIN0	0
#define SIN72	0.9510565
#define SIN144	0.5877852
#define SIN216	-0.5877852
#define SIN288	-0.9510565




/////////////////////////////////////////////////////////////////////////////
// Tek arrow swoosh
/////////////////////////////////////////////////////////////////////////////

// Arrow
CSwooshEdgeVertexInfo ExploArrowSwooshVertices[] =
{
	// Cross line 1
	// Pos							Color					Alpha
	{{{-1,		0,			0}, {100,100,255},	MAX_OPAQ/4},	// Start
	 {{0,			0,			0}, {0,	0,	 255}, 	MIN_OPAQ}},		// end

	{{{1,			0,			0}, {100,100,255},	MAX_OPAQ/4},	// Start
	 {{0,			0,			0}, {0,	0,	 255}, 	MIN_OPAQ}},		// end

	// Cross line 2
	// Pos							Color					Alpha
	{{{0,			-1,		0}, {100,100,255},	MAX_OPAQ/4},	// Start
	 {{0,			0,			0}, {0,	0,	 255}, 	MIN_OPAQ}},		// end

	{{{0,			1,			0}, {100,100,255},	MAX_OPAQ/4},	// Start
	 {{0,			0,			0}, {0,	0,	 255}, 	MIN_OPAQ}},		// end
} ;

CSwooshInfo ExploArrowSwoosh1Info =
{
	SECONDS_TO_FRAMES(4),			// Total time on screen
	SECONDS_TO_FRAMES(3),			// Total time before fade swoosh out
	SECONDS_TO_FRAMES(1), 			// Total edge life time
	0,										// Object node
	0.2,									// Scale
	NULL,									// General flags
	2,										// No of edge vertices
	&ExploArrowSwooshVertices[0]	// Edge vertices
} ;

CSwooshInfo ExploArrowSwoosh2Info =
{
	SECONDS_TO_FRAMES(4),			// Total time on screen
	SECONDS_TO_FRAMES(3),			// Total time before fade swoosh out
	SECONDS_TO_FRAMES(1), 			// Total edge life time
	0,										// Object node
	0.2,									// Scale
	NULL,									// General flags
	2,										// No of edge vertices
	&ExploArrowSwooshVertices[2]	// Edge vertices
} ;



/////////////////////////////////////////////////////////////////////////////
// Chrono swoosh
/////////////////////////////////////////////////////////////////////////////

CSwooshEdgeVertexInfo ChronoSwooshVertices[] =
{
	// Outer pentagon
	// Pos									Color					Alpha
	{{{COS0*1,		SIN0*1,		0}, {100,100,255},	MAX_OPAQ/4},	// Start
	 {{COS0*0,		SIN0*0,		0}, {0,	0,	50},		MIN_OPAQ}},		// end

	{{{COS72*1,		SIN72*1,		0}, {100,100,255},	MAX_OPAQ/4},	// Start
	 {{COS72*0, 	SIN72*0,		0}, {0,	0,	50},		MIN_OPAQ}},		// end

	{{{COS144*1,	SIN144*1,	0}, {100,100,255},	MAX_OPAQ/4},	// Start
	 {{COS144*0,	SIN144*0,	0}, {0,	0,	50},		MIN_OPAQ}},		// end

	{{{COS216*1, 	SIN216*1,	0}, {100,100,255},	MAX_OPAQ/4},	// Start
	 {{COS216*0,	SIN216*0,	0}, {0,	0,	50},		MIN_OPAQ}},		// end

	{{{COS288*1, 	SIN288*1,	0}, {100,100,255},	MAX_OPAQ/4},	// Start
	 {{COS288*0,	SIN288*0,	0}, {0,	0,	50},		MIN_OPAQ}},		// end

	// Inner pentagon
	// Pos									Color					Alpha
	{{{COS0*1,		SIN0*1,		0}, {255,255,255},	MAX_OPAQ/2},	// Start
	 {{COS0*0,		SIN0*0,		0}, {255,255,255},	MIN_OPAQ}},		// end

	{{{COS72*1,		SIN72*1,		0}, {255,255,255},	MAX_OPAQ/2},	// Start
	 {{COS72*0, 	SIN72*0,		0}, {255,255,255},	MIN_OPAQ}},		// end

	{{{COS144*1,	SIN144*1,	0}, {255,255,255},	MAX_OPAQ/2},	// Start
	 {{COS144*0,	SIN144*0,	0}, {255,255,255},	MIN_OPAQ}},		// end

	{{{COS216*1, 	SIN216*1,	0}, {255,255,255},	MAX_OPAQ/2},	// Start
	 {{COS216*0,	SIN216*0,	0}, {255,255,255},	MIN_OPAQ}},		// end

	{{{COS288*1, 	SIN288*1,	0}, {255,255,255},	MAX_OPAQ/2},	// Start
	 {{COS288*0,	SIN288*0,	0}, {255,255,255},	MIN_OPAQ}},		// end
} ;


CSwooshInfo ChronoSwoosh1Info =
{
	SECONDS_TO_FRAMES(6),		// Total time on screen
	SECONDS_TO_FRAMES(4),		// Total time before fade swoosh out
	SECONDS_TO_FRAMES(1),		// Total edge life time
	0,									// Object node
	1.0,								// Scale
	SWOOSH_CLOSE_EDGES,			// General flags
	5,									// No of edge vertices
	&ChronoSwooshVertices[0]	// Edge vertices
} ;

CSwooshInfo ChronoSwoosh2Info =
{
	SECONDS_TO_FRAMES(6),		// Total time on screen
	SECONDS_TO_FRAMES(4),		// Total time before fade swoosh out
	SECONDS_TO_FRAMES(1),		// Total edge life time
	0,									// Object node
	0.5,								// Scale
	SWOOSH_CLOSE_EDGES,			// General flags
	5,									// No of edge vertices
	&ChronoSwooshVertices[5]	// Edge vertices
} ;

/////////////////////////////////////////////////////////////////////////////
// Campaigner big  Swoosh
/////////////////////////////////////////////////////////////////////////////


CSwooshEdgeVertexInfo CampSwooshVertices[] =
{
	// Cross line 1
	// Pos							Color					Alpha
	{{{-1,		0,			0}, {255,240,200},	MAX_OPAQ/2},	// Start
	 {{0,			0,			0}, {0,	0,	 0}, 	MIN_OPAQ}},		// end

	{{{1,			0,			0}, {255,240,200},	MAX_OPAQ/2},	// Start
	 {{0,			0,			0}, {0,	0,	 0}, 	MIN_OPAQ}},		// end

	// Cross line 2
	// Pos							Color					Alpha
	{{{0,			-1,		0}, {255,240,200},	MAX_OPAQ/2},	// Start
	 {{0,			0,			0}, {0,	0,	 0}, 	MIN_OPAQ}},		// end

	{{{0,			1,			0}, {255,240,200},	MAX_OPAQ/2},	// Start
	 {{0,			0,			0}, {0,	0,	 0}, 	MIN_OPAQ}},		// end
} ;

CSwooshInfo CampSwoosh1Info =
{
	SECONDS_TO_FRAMES(4),			// Total time on screen
	SECONDS_TO_FRAMES(3),			// Total time before fade swoosh out
	SECONDS_TO_FRAMES(1), 			// Total edge life time
	0,										// Object node
	3,									// Scale
	NULL,									// General flags
	2,										// No of edge vertices
	&CampSwooshVertices[0]	// Edge vertices
} ;

CSwooshInfo CampSwoosh2Info =
{
	SECONDS_TO_FRAMES(4),			// Total time on screen
	SECONDS_TO_FRAMES(3),			// Total time before fade swoosh out
	SECONDS_TO_FRAMES(1), 			// Total edge life time
	0,										// Object node
	3,									// Scale
	NULL,									// General flags
	2,										// No of edge vertices
	&CampSwooshVertices[2]	// Edge vertices
} ;


/////////////////////////////////////////////////////////////////////////////
// Plasma Rifle Pulse Swoosh
/////////////////////////////////////////////////////////////////////////////

CSwooshEdgeVertexInfo plasmaSwooshVertices[] =
{
	// Outer pentagon
	// Pos									Color					Alpha
	{{{COS0*1,		SIN0*1,		0}, {255,255,255},	MAX_OPAQ/4},	// Start
	 {{COS0*0,		SIN0*0,		0}, {200,	255,	200},		MIN_OPAQ}},		// end

	{{{COS72*1,		SIN72*1,		0}, {255,255,255},	MAX_OPAQ/4},	// Start
	 {{COS72*0, 	SIN72*0,		0}, {200,	255,	200},		MIN_OPAQ}},		// end

	{{{COS144*1,	SIN144*1,	0}, {255,255,255},	MAX_OPAQ/4},	// Start
	 {{COS144*0,	SIN144*0,	0}, {200,	255,	200},		MIN_OPAQ}},		// end

	{{{COS216*1, 	SIN216*1,	0}, {255,255,255},	MAX_OPAQ/4},	// Start
	 {{COS216*0,	SIN216*0,	0}, {200,	255,	200},		MIN_OPAQ}},		// end

	{{{COS288*1, 	SIN288*1,	0}, {255,255,255},	MAX_OPAQ/4},	// Start
	 {{COS288*0,	SIN288*0,	0}, {200,	255,	200},		MIN_OPAQ}},		// end


} ;


CSwooshInfo PlasmaSwooshInfo =
{
	SECONDS_TO_FRAMES(.4),		// Total time on screen
	SECONDS_TO_FRAMES(.2),		// Total time before fade swoosh out
	SECONDS_TO_FRAMES(.15),		// Total edge life time
	0,									// Object node
	.4,								// Scale
	SWOOSH_CLOSE_EDGES,			// General flags
	5,									// No of edge vertices
	&plasmaSwooshVertices[0]		// Edge vertices
} ;


/////////////////////////////////////////////////////////////////////////////
// Generic220 swoosh
/////////////////////////////////////////////////////////////////////////////

CSwooshEdgeVertexInfo Generic220SwooshVertices[] =
{
	// Pos							Color					Alpha
	{{{-2,		0,			0}, {255,255,255},	MIN_OPAQ},	// Start
	 {{0,			0,			0}, {255,255,255}, 	MIN_OPAQ}},		// end

	{{{0,			0,			0}, {255,255,255},	MAX_OPAQ/5},	// Start
	 {{0,			0,			0}, {255,255,255}, 	MIN_OPAQ}},		// end

	{{{2,			0,			0}, {255,255,255},	MIN_OPAQ},	// Start
	 {{0,			0,			0}, {255,255,255}, 	MIN_OPAQ}},		// end
} ;

CSwooshInfo Generic220SwooshInfo =
{
	SECONDS_TO_FRAMES(2),			// Total time on screen
	SECONDS_TO_FRAMES(1),			// Total time before fade swoosh out
	SECONDS_TO_FRAMES(1), 			// Total edge life time
	0,										// Object node
	4,										// Scale
	NULL,									// General flags
	2,										// No of edge vertices
	Generic220SwooshVertices		// Edge vertices
} ;

/////////////////////////////////////////////////////////////////////////////
// Demon Beam Swoosh
/////////////////////////////////////////////////////////////////////////////

CSwooshEdgeVertexInfo demonSwooshVertices[] =
{
	// Outer pentagon
	// Pos									Color					Alpha
	{{{COS0*1,		SIN0*1,		0}, {255,200,100},	MAX_OPAQ/2},	// Start
	 {{COS0*0,		SIN0*0,		0}, {20,	20,	20},		MIN_OPAQ}},		// end

	{{{COS72*1,		SIN72*1,		0}, {255,200,100},	MAX_OPAQ/2},	// Start
	 {{COS72*0, 	SIN72*0,		0}, {20,	20,	20},		MIN_OPAQ}},		// end

	{{{COS144*1,	SIN144*1,	0}, {255,200,100},	MAX_OPAQ/2},	// Start
	 {{COS144*0,	SIN144*0,	0}, {20,	20,	20},		MIN_OPAQ}},		// end

	{{{COS216*1, 	SIN216*1,	0}, {255,200,100},	MAX_OPAQ/2},	// Start
	 {{COS216*0,	SIN216*0,	0}, {20,	20,	20},		MIN_OPAQ}},		// end

	{{{COS288*1, 	SIN288*1,	0}, {255,200,100},	MAX_OPAQ/2},	// Start
	 {{COS288*0,	SIN288*0,	0}, {20,	20,	20},		MIN_OPAQ}},		// end


} ;


CSwooshInfo DemonSwooshInfo =
{
	SECONDS_TO_FRAMES(1),		// Total time on screen
	SECONDS_TO_FRAMES(.5),		// Total time before fade swoosh out
	SECONDS_TO_FRAMES(.3),		// Total edge life time
	0,									// Object node
	.4,								// Scale
	SWOOSH_CLOSE_EDGES,			// General flags
	5,									// No of edge vertices
	demonSwooshVertices		// Edge vertices
} ;


/////////////////////////////////////////////////////////////////////////////
// Generic232 swoosh
/////////////////////////////////////////////////////////////////////////////

CSwooshEdgeVertexInfo Generic232SwooshVertices[] =
{
	// Pos							Color					Alpha
	{{{-2,		0,			0}, {255,240,100},	MIN_OPAQ},	// Start
	 {{0,			0,			0}, {255,240,100 }, 	MIN_OPAQ}},		// end

	{{{0,			0,			0}, {255,255,255},	MAX_OPAQ/2},	// Start
	 {{0,			0,			0}, {255,255,255}, 	MIN_OPAQ}},		// end

	{{{2,			0,			0}, {255,240,100},	MIN_OPAQ},	// Start
	 {{0,			0,			0}, {255,240,100}, 	MIN_OPAQ}},		// end
} ;

CSwooshInfo Generic232SwooshInfo =
{
	SECONDS_TO_FRAMES(2),			// Total time on screen
	SECONDS_TO_FRAMES(1),			// Total time before fade swoosh out
	SECONDS_TO_FRAMES(1), 			// Total edge life time
	0,										// Object node
	4,										// Scale
	NULL,									// General flags
	3,										// No of edge vertices
	Generic232SwooshVertices		// Edge vertices
} ;


/////////////////////////////////////////////////////////////////////////////
// Generic222 swoosh
/////////////////////////////////////////////////////////////////////////////

CSwooshEdgeVertexInfo Generic222SwooshVertices[] =
{
	// Pos							Color					Alpha
	{{{-2,		0,			0}, {255,255,255},	MIN_OPAQ},	// Start
	 {{0,			0,			0}, {255,	255,	 255}, 	MIN_OPAQ}},		// end
	{{{0,		0,			0}, {200,230,255},	MAX_OPAQ/2},	// Start
	 {{0,			0,			0}, {255,	255,	 255}, 	MIN_OPAQ}},		// end

	{{{2,			0,			0}, {255,	255,  255},	MIN_OPAQ},	// Start
	 {{0,			0,			0}, {255,255,255}, 	MIN_OPAQ}},		// end
} ;

CSwooshInfo Generic222SwooshInfo =
{
	SECONDS_TO_FRAMES(4),			// Total time on screen
	SECONDS_TO_FRAMES(3),			// Total time before fade swoosh out
	SECONDS_TO_FRAMES(1), 			// Total edge life time
	0,										// Object node
	.6,									// Scale
	NULL,									// General flags
	2,										// No of edge vertices
	Generic222SwooshVertices		// Edge vertices
} ;

/////////////////////////////////////////////////////////////////////////////
// Generic247 swoosh
/////////////////////////////////////////////////////////////////////////////

CSwooshEdgeVertexInfo Generic247SwooshVertices[] =
{
	// Pos							Color					Alpha
	{{{-2,		-1,			0}, {255,255,255},	MIN_OPAQ},	// Start
	 {{0,			0,			0}, {255,	255,	 255}, 	MIN_OPAQ}},		// end
	{{{0,		0,			0}, {200,230,255},	MAX_OPAQ/4},	// Start
	 {{0,			0,			0}, {255,	255,	 255}, 	MIN_OPAQ}},		// end

	{{{2,			1,			0}, {255,	255,  255},	MIN_OPAQ},	// Start
	 {{0,			0,			0}, {255,255,255}, 	MIN_OPAQ}},		// end
} ;

CSwooshInfo Generic247SwooshInfo =
{
	SECONDS_TO_FRAMES(3),			// Total time on screen
	SECONDS_TO_FRAMES(2),			// Total time before fade swoosh out
	SECONDS_TO_FRAMES(1), 			// Total edge life time
	0,										// Object node
	.6,									// Scale
	NULL,									// General flags
	3,										// No of edge vertices
	Generic247SwooshVertices		// Edge vertices
} ;


/////////////////////////////////////////////////////////////////////////////
// Generic231 swoosh
/////////////////////////////////////////////////////////////////////////////

CSwooshEdgeVertexInfo Generic231SwooshVertices[] =
{
	// Pos							Color					Alpha
	{{{-2,		0,			0}, {255,255,255},	MIN_OPAQ},	// Start
	 {{0,			0,			0}, {255,	255,	 255}, 	MIN_OPAQ}},		// end
	{{{0,		0,			0}, {255,220,220},	MAX_OPAQ/4},	// Start
	 {{0,			0,			0}, {255,	255,	 255}, 	MIN_OPAQ}},		// end

	{{{2,			0,			0}, {255,	255,  255},	MIN_OPAQ},	// Start
	 {{0,			0,			0}, {255,255,255}, 	MIN_OPAQ}},		// end
} ;

CSwooshInfo Generic231SwooshInfo =
{
	SECONDS_TO_FRAMES(1),			// Total time on screen
	SECONDS_TO_FRAMES(.5),			// Total time before fade swoosh out
	SECONDS_TO_FRAMES(.5), 			// Total edge life time
	0,										// Object node
	2,									// Scale
	NULL,									// General flags
	2,										// No of edge vertices
	Generic231SwooshVertices		// Edge vertices
} ;

/////////////////////////////////////////////////////////////////////////////
// Generic225 swoosh
/////////////////////////////////////////////////////////////////////////////

CSwooshEdgeVertexInfo Generic225SwooshVertices[] =
{
	// Pos							Color					Alpha
	{{{-2,		0,			0}, {255,255,255},	MIN_OPAQ},	// Start
	 {{0,			0,			0}, {255,	255,	 255}, 	MIN_OPAQ}},		// end
	{{{0,		0,			0}, {200,230,255},	MAX_OPAQ/4},	// Start
	 {{0,			0,			0}, {255,	255,	 255}, 	MIN_OPAQ}},		// end

	{{{2,			0,			0}, {255,	255,  255},	MIN_OPAQ},	// Start
	 {{0,			0,			0}, {255,255,255}, 	MIN_OPAQ}},		// end
} ;

CSwooshInfo Generic225SwooshInfo =
{
	SECONDS_TO_FRAMES(4),			// Total time on screen
	SECONDS_TO_FRAMES(3),			// Total time before fade swoosh out
	SECONDS_TO_FRAMES(1), 			// Total edge life time
	0,										// Object node
	.7,									// Scale
	NULL,									// General flags
	2,										// No of edge vertices
	Generic225SwooshVertices		// Edge vertices
} ;



/////////////////////////////////////////////////////////////////////////////
// Rocket swoosh
/////////////////////////////////////////////////////////////////////////////

CSwooshEdgeVertexInfo RocketSwooshVertices[] =
{
	// Pos							Color					Alpha
	{{{-1,		0,			0}, {255,255,255},	MAX_OPAQ/5},	// Start
	 {{0,			0,			0}, {255,255, 255}, 	MIN_OPAQ}},		// end

	{{{1,			0,			0}, {255,255, 255},	MAX_OPAQ/5},	// Start
	 {{0,			0,			0}, {255,255,255}, 	MIN_OPAQ}},		// end

} ;

CSwooshInfo RocketSwooshInfo =
{
	SECONDS_TO_FRAMES(15),			// Total time on screen
	SECONDS_TO_FRAMES(14),			// Total time before fade swoosh out
	SECONDS_TO_FRAMES(2), 			// Total edge life time
	0,										// Object node
	0.2,									// Scale
	NULL,									// General flags
	2,										// No of edge vertices
	RocketSwooshVertices				// Edge vertices
} ;




/////////////////////////////////////////////////////////////////////////////
// TREX Laser
/////////////////////////////////////////////////////////////////////////////

CSwooshEdgeVertexInfo TRexSwooshVertices[] =
{
	// Outer pentagon
	// Pos									Color					Alpha
	{{{COS0*1,		SIN0*1,		0}, {255,100,100},	MAX_OPAQ/4},	// Start
	 {{COS0*0,		SIN0*0,		0}, {50,	0,	0},		MIN_OPAQ}},		// end

	{{{COS72*1,		SIN72*1,		0}, {255,100,100},	MAX_OPAQ/4},	// Start
	 {{COS72*0, 	SIN72*0,		0}, {50,	0,	0},		MIN_OPAQ}},		// end

	{{{COS144*1,	SIN144*1,	0}, {255,100,100},	MAX_OPAQ/4},	// Start
	 {{COS144*0,	SIN144*0,	0}, {50,	0,	0},		MIN_OPAQ}},		// end

	{{{COS216*1, 	SIN216*1,	0}, {255,100,100},	MAX_OPAQ/4},	// Start
	 {{COS216*0,	SIN216*0,	0}, {50,	0,	0},		MIN_OPAQ}},		// end

	{{{COS288*1, 	SIN288*1,	0}, {255,100,100},	MAX_OPAQ/4},	// Start
	 {{COS288*0,	SIN288*0,	0}, {50,	0,	0},		MIN_OPAQ}},		// end

	// Inner pentagon
	// Pos									Color					Alpha
	{{{COS0*1,		SIN0*1,		0}, {255,255,255},	MAX_OPAQ/2},	// Start
	 {{COS0*0,		SIN0*0,		0}, {255,255,255},	MIN_OPAQ}},		// end

	{{{COS72*1,		SIN72*1,		0}, {255,255,255},	MAX_OPAQ/2},	// Start
	 {{COS72*0, 	SIN72*0,		0}, {255,255,255},	MIN_OPAQ}},		// end

	{{{COS144*1,	SIN144*1,	0}, {255,255,255},	MAX_OPAQ/2},	// Start
	 {{COS144*0,	SIN144*0,	0}, {255,255,255},	MIN_OPAQ}},		// end

	{{{COS216*1, 	SIN216*1,	0}, {255,255,255},	MAX_OPAQ/2},	// Start
	 {{COS216*0,	SIN216*0,	0}, {255,255,255},	MIN_OPAQ}},		// end

	{{{COS288*1, 	SIN288*1,	0}, {255,255,255},	MAX_OPAQ/2},	// Start
	 {{COS288*0,	SIN288*0,	0}, {255,255,255},	MIN_OPAQ}},		// end
} ;


CSwooshInfo TRexSwoosh1Info =
{
	SECONDS_TO_FRAMES(2),		// Total time on screen
	SECONDS_TO_FRAMES(1),		// Total time before fade swoosh out
	SECONDS_TO_FRAMES(1),		// Total edge life time
	0,									// Object node
	1.0,								// Scale
	SWOOSH_CLOSE_EDGES,			// General flags
	5,									// No of edge vertices
	&TRexSwooshVertices[0]	// Edge vertices
} ;

CSwooshInfo TRexSwoosh2Info =
{
	SECONDS_TO_FRAMES(2),		// Total time on screen
	SECONDS_TO_FRAMES(1),		// Total time before fade swoosh out
	SECONDS_TO_FRAMES(1),		// Total edge life time
	0,									// Object node
	0.5,								// Scale
	SWOOSH_CLOSE_EDGES,			// General flags
	5,									// No of edge vertices
	&TRexSwooshVertices[5]	// Edge vertices
} ;










/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

// globals
Vtx					particle_vtxs[4];
CMtxF					mf_particle_facescreen,
						mf_particle_facescreen_yaxis,
						mf_particle_faceup;
CEnemyAttributes	particle_attributes;

// glare globals
CVector3				v_particle_glare_pos;
BYTE					particle_glare_color[4];
CParticle			*p_particle_glare;


// CParticle implementation
/////////////////////////////////////////////////////////////////////////////

#define		CParticle__GetOrientationMatrix(pThis) ((pThis)->m_mOrientation[even_odd])
CMtxF*		CParticle__GetAlignmentRotation(CParticle *pThis);
void			CParticle__BlowupGrenade(CParticle *pThis);
void			CParticle__ImpactEffect(CParticle *pThis, int nObjType);
void			CParticle__CalculateOrientationMatrix(CParticle *pThis,
																  float *pDistanceFromPlayer /*=NULL*/,
																  BOOL *pIntersectWithGround  /*=NULL*/);

/////////////////////////////////////////////////////////////////////////////
// CParticle

void CParticle__Construct(CParticle *pThis)
{
	pThis->ah.ih.m_CollisionHeightOffset = 0;
	pThis->ah.ih.m_pEA = &particle_attributes;
	pThis->ah.ih.m_Type = I_PARTICLE;
	pThis->m_dwFlags = 0;
	//pThis->ah.m_vPrevBackoff.x = pThis->ah.m_vPrevBackoff.y = pThis->ah.m_vPrevBackoff.z = 0;
}

void CParticle__Delete(CParticle *pThis)
{
	CParticleSystem	*pSystem;

	pSystem = &GetApp()->m_Scene.m_ParticleSystem;
	CParticleSystem__DeallocateParticle(pSystem, pThis);
}

void CParticle__Spawn(CParticle *pThis, int nType, BOOL Blood)
{
	CParticleSystem	*pSystem;
	CQuatern				qRot;
	CVector3				vVelocity,
							vOrigin, vRotated, vOffset,
							vPos, vDesired;
	CMtxF					*pmfAlign, mfRotate;
	CGameRegion			*pRegion;

	pSystem = &GetApp()->m_Scene.m_ParticleSystem;
	ASSERT(pSystem);

	if (pThis->m_pEffect->m_dwFlags & PARTICLE_BEHAVIOR_RETAINVELOCITY)
		vVelocity = pThis->m_CI.vCollisionVelocity;
	else
		vVelocity = pThis->ah.m_vVelocity;

	qRot = CVector3__GetDirectionRotation(&vVelocity);

	if (		(SF2FLOAT(pThis->m_pEffect->m_RotationPivotX) == 0)
			&&	(SF2FLOAT(pThis->m_pEffect->m_RotationPivotY) == 0) )
	{
		vPos = pThis->ah.ih.m_vPos;
		pRegion = pThis->ah.ih.m_pCurrentRegion;
	}
	else
	{
		vOrigin.x = -SF2FLOAT(pThis->m_pEffect->m_RotationPivotX);
		vOrigin.y = -SF2FLOAT(pThis->m_pEffect->m_RotationPivotY);
		vOrigin.z = 0;

		CMtxF__RotateZ(mfRotate, pThis->m_Rotation);
		CMtxF__VectorMult(mfRotate, &vOrigin, &vRotated);

		pmfAlign = CParticle__GetAlignmentRotation(pThis);
		CMtxF__VectorMult(*pmfAlign, &vRotated, &vOffset);

		CVector3__Add(&vDesired, &vOffset, &pThis->ah.ih.m_vPos);

		CInstanceHdr__GetNearPositionAndRegion(&pThis->ah.ih,
															vDesired,
															&vPos,
															&pRegion,
															INTERSECT_BEHAVIOR_IGNORE, INTERSECT_BEHAVIOR_IGNORE);
	}

	CParticleSystem__CreateParticle(pSystem, pThis->m_pSource, nType,
											  vVelocity,
											  qRot,
											  vPos, pRegion, PARTICLE_NOEDGE, Blood);
}

CMtxF* CParticle__GetAlignmentRotation(CParticle *pThis)
{
	static CMtxF	mfAlign;
	CQuatern			qAlign;
	CMtxF				*pmfAlign,
						mfTemp;
	float				angle;

	switch (pThis->m_Alignment)
	{
		case PARTICLE_UP:
			pmfAlign = &mf_particle_faceup;
			break;

		case PARTICLE_GROUND:
			qAlign = CGameRegion__GetGroundRotation(pThis->ah.ih.m_pCurrentRegion);
			CQuatern__ToMatrix(&qAlign, mfTemp);
			CMtxF__PostMult(mfAlign, mf_particle_faceup, mfTemp);
			pmfAlign = &mfAlign;
			break;

		case PARTICLE_CEILING:
			qAlign = CGameRegion__GetCeilingRotation(pThis->ah.ih.m_pCurrentRegion);
			CQuatern__ToMatrix(&qAlign, mfTemp);
			CMtxF__PostMult(mfAlign, mf_particle_faceup, mfTemp);
			pmfAlign = &mfAlign;
			break;

		case 0:
		case 1:
		case 2:
			angle = CGameRegion__GetEdgeAngle(pThis->ah.ih.m_pCurrentRegion, pThis->m_Alignment);
			CMtxF__RotateY(mfAlign, angle) ;
			pmfAlign = &mfAlign;
			break;

		case PARTICLE_SCREENY:
			pmfAlign = &mf_particle_facescreen_yaxis;
			break;

		case PARTICLE_SCREEN:
		case PARTICLE_NOEDGE:
		default:
			pmfAlign = &mf_particle_facescreen;
			break;
	}

	return pmfAlign;
}

void CParticle__SpawnWallCollision(CParticle *pThis, int nType)
{
	CParticleSystem	*pSystem;
	CQuatern				qRot;
	CVector3				vVelocity;
	CROMRegionSet		*pRegionSet;

	ASSERT(pThis->m_CI.pWallCollisionRegion);

	pSystem = &GetApp()->m_Scene.m_ParticleSystem;
	ASSERT(pSystem);

	if (pThis->m_pEffect->m_dwFlags & PARTICLE_BEHAVIOR_RETAINVELOCITY)
		vVelocity = pThis->m_CI.vCollisionVelocity;
	else
		vVelocity = pThis->ah.m_vVelocity;

	qRot = CVector3__GetDirectionRotation(&vVelocity);

	// Start of FUCKED!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// special fudge to stop blood splats on walls while in caves
	// if the cave music is on for this region don't allow blood splats on walls
	// technically speaking this should check the pEffect of the new nType but Thomas assures me
	// the particle hitting the wall will always be a blood type anyway so we don't need to bother.
	if (pThis->m_pEffect->m_dwFlags & PARTICLE_BEHAVIOR_BLOODEFFECT)
	{
		pRegionSet = CScene__GetRegionAttributes(&GetApp()->m_Scene, pThis->ah.ih.m_pCurrentRegion);
		if (pRegionSet)
		{
			if (pRegionSet->m_AmbientSounds == REG_AMBIENTCAVES)
				return;
		}
	}
	// End of FUCKED!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	CParticleSystem__CreateParticle(pSystem, pThis->m_pSource, nType,
											  vVelocity,
											  qRot,
											  pThis->m_CI.vWallCollisionPos, pThis->m_CI.pWallCollisionRegion, pThis->m_CI.nWallCollisionEdge, TRUE);
}

void CParticle__SpawnGroundCollision(CParticle *pThis, int nType)
{
	CParticleSystem	*pSystem;
	CQuatern				qRot;
	CVector3				vVelocity;

	ASSERT(pThis->m_CI.nGroundCollisionType == GROUNDCOLLISION_GROUND);

	pSystem = &GetApp()->m_Scene.m_ParticleSystem;
	ASSERT(pSystem);

	if (pThis->m_pEffect->m_dwFlags & PARTICLE_BEHAVIOR_RETAINVELOCITY)
		vVelocity = pThis->m_CI.vCollisionVelocity;
	else
		vVelocity = pThis->ah.m_vVelocity;

	qRot = CVector3__GetDirectionRotation(&vVelocity);

	CParticleSystem__CreateParticle(pSystem, pThis->m_pSource, nType,
											  vVelocity,
											  qRot,
											  pThis->m_CI.vGroundCollisionPos, pThis->m_CI.pGroundCollisionRegion, PARTICLE_GROUND, TRUE);
}

void CParticle__SpawnCeilingCollision(CParticle *pThis, int nType)
{
	CParticleSystem	*pSystem;
	CQuatern				qRot;
	CVector3				vVelocity;

	ASSERT(pThis->m_CI.nGroundCollisionType == GROUNDCOLLISION_CEILING);

	pSystem = &GetApp()->m_Scene.m_ParticleSystem;
	ASSERT(pSystem);

	if (pThis->m_pEffect->m_dwFlags & PARTICLE_BEHAVIOR_RETAINVELOCITY)
		vVelocity = pThis->m_CI.vCollisionVelocity;
	else
		vVelocity = pThis->ah.m_vVelocity;

	qRot = CVector3__GetDirectionRotation(&vVelocity);

	CParticleSystem__CreateParticle(pSystem, pThis->m_pSource, nType,
											  vVelocity,
											  qRot,
											  pThis->m_CI.vGroundCollisionPos, pThis->m_CI.pGroundCollisionRegion, PARTICLE_CEILING, TRUE);
}

void CParticle__SpawnWaterSurfaceCollision(CParticle *pThis, int nType)
{
	CParticleSystem	*pSystem;
	CQuatern				qRot;
	CVector3				vVelocity;

	ASSERT(pThis->m_CI.pWaterCollisionRegion);

	pSystem = &GetApp()->m_Scene.m_ParticleSystem;
	ASSERT(pSystem);

	if (pThis->m_pEffect->m_dwFlags & PARTICLE_BEHAVIOR_RETAINVELOCITY)
		vVelocity = pThis->m_CI.vCollisionVelocity;
	else
		vVelocity = pThis->ah.m_vVelocity;

	qRot = CVector3__GetDirectionRotation(&vVelocity);

	CParticleSystem__CreateParticle(pSystem, pThis->m_pSource, nType,
											  vVelocity,
											  qRot,
											  pThis->m_CI.vWaterCollisionPos, pThis->m_CI.pWaterCollisionRegion, PARTICLE_UP, TRUE);
}

// CParticleSystem implementation
/////////////////////////////////////////////////////////////////////////////

CParticle*				CParticleSystem__AllocateParticle(CParticleSystem *pThis, BYTE Priority);
void						CParticleSystem__DeallocateParticleByPriority(CParticleSystem *pThis, BYTE Priority);
//CROMParticleEffect*	CParticleSystem__LookupEffectType(CParticleSystem *pThis, int nType);
CParticle*				CParticleSystem__FindMatchingSource(CParticleSystem *pThis,
																		   CInstanceHdr *pSource,
																		   CROMParticleEffect *pEffect /*=NULL*/);
void						CParticleSystem__AddParticleToDrawList(CParticleSystem *pThis, CParticle *pParticle);
void						CParticleSystem__DecompressParticles(CParticleSystem *pThis, CCacheEntry **ppceTarget);

/////////////////////////////////////////////////////////////////////////////
// CParticleSytem

void CParticleSystem__Construct(CParticleSystem *pThis)
{
	int	i, v;
	Vtx	*pV;

	TRACE0("CParticleSystem construction\r\n");

	ASSERT(PARTICLES_MAX_COUNT >= 1);

	p_particle_glare = NULL;

	pThis->m_pFreeHead = &pThis->m_Particles[0];
	pThis->m_pActiveHead = pThis->m_pActiveTail = NULL;
	pThis->m_pDrawHead = pThis->m_pDrawTail = NULL;

	for (i=0; i<(PARTICLES_MAX_COUNT-1); i++)
		pThis->m_Particles[i].m_pNext = &pThis->m_Particles[i+1];

	pThis->m_Particles[PARTICLES_MAX_COUNT-1].m_pNext	= NULL;

	CEnemyAttributes__SetDefaults(&particle_attributes);
	particle_attributes.m_dwTypeFlags     &= ~AI_TYPE_COLLISION;
	particle_attributes.m_dwTypeFlags     |= AI_TYPE_TRACKGROUND;
	particle_attributes.m_CollisionRadius  = 0*SCALING_FACTOR;
	particle_attributes.m_CollisionHeight = 0.0;

	for (v=0; v<4; v++)
	{
		pV = &particle_vtxs[v];

		// 64 is max texture size
		switch (v)
		{
			case 0:
				pV->v.ob[0] = -1;
				pV->v.ob[1] = 1;
				//pV->v.tc[0] = 64*(1 << 6);
				//pV->v.tc[1] = 64*(1 << 6);
				pV->v.tc[0] = 64*(1 << 7);
				pV->v.tc[1] = 64*(1 << 7);
				break;
			case 1:
				pV->v.ob[0] = 1;
				pV->v.ob[1] = 1;
				pV->v.tc[0] = 0;
				//pV->v.tc[1] = 64*(1 << 6);
				pV->v.tc[1] = 64*(1 << 7);
				break;
			case 2:
				pV->v.ob[0] = 1;
				pV->v.ob[1] = -1;
				pV->v.tc[0] = 0;
				pV->v.tc[1] = 0;
				break;
			case 3:
				pV->v.ob[0] = -1;
				pV->v.ob[1] = -1;
				//pV->v.tc[0] = 64*(1 << 6);
				pV->v.tc[0] = 64*(1 << 7);
				pV->v.tc[1] = 0;
				break;
		}
		pV->v.ob[2] = 0;

		pV->v.flag = 0;

		pV->v.cn[0] = 255;
		pV->v.cn[1] = 255;
		pV->v.cn[2] = 255;
		pV->v.cn[3] = 255;
	}

	CMtxF__RotateX(mf_particle_faceup, ANGLE_PI/2) ;

	for (i=0; i<PARTICLES_MAX_COUNT; i++)
		CParticle__Construct(&pThis->m_Particles[i]);

   pThis->m_pceParticleEffects = NULL;
	pThis->m_pCache = NULL;
	pThis->m_pceTextureSetsIndex = NULL;
}

void CParticleSystem__DeallocateParticleByPriority(CParticleSystem *pThis, BYTE Priority)
{
/*
	CParticle	*pParticle, *pLowestPriorityParticle;
	BYTE			lowestPriority;

	lowestPriority = 100;
	pLowestPriorityParticle = NULL;

	pParticle = pThis->m_pActiveTail;
	while (pParticle)
	{
		if (pParticle->m_pEffect->m_Priority < lowestPriority)
		{
			lowestPriority = pParticle->m_pEffect->m_Priority;
			pLowestPriorityParticle = pParticle;

			if (lowestPriority == 0)
				break;
		}

		pParticle = pParticle->m_pLast;
	}

	if (pLowestPriorityParticle && (lowestPriority <= Priority))
		CParticleSystem__DeallocateParticle(pThis, pLowestPriorityParticle);
*/

// /*
	CParticle	*pParticle;

	pParticle = pThis->m_pActiveTail;
	while (pParticle)
	{
		if (pParticle->m_pEffect->m_Priority <= Priority)
		{
			CParticleSystem__DeallocateParticle(pThis, pParticle);
			break;
		}

		pParticle = pParticle->m_pLast;
	}
// */
/*
	ASSERT(pThis->m_pActiveTail);
	CParticleSystem__DeallocateParticle(pThis, pThis->m_pActiveTail);
	ASSERT(pThis->m_pFreeHead);
*/
}

CParticle* CParticleSystem__AllocateParticle(CParticleSystem *pThis, BYTE Priority)
{
	CParticle	*pParticle;

	if (!pThis->m_pFreeHead)
	{
		ASSERT(pThis->m_pActiveTail);

#ifdef TRACE_MEM
	TRACE("CParticleSystem: Stealing oldest particle: %x.\r\n", pThis->m_pActiveTail);
#endif
		//CParticleSystem__DeallocateParticle(pThis, pThis->m_pActiveTail);

		//rmonPrintf("Stealing oldest particle\n");
		CParticleSystem__DeallocateParticleByPriority(pThis, Priority);
	}

	if (!pThis->m_pFreeHead)
		return NULL;

	// get first node in free list
	pParticle = pThis->m_pFreeHead;

	// remove from free list
	pThis->m_pFreeHead = pParticle->m_pNext;

	// add to head of active list
	if (pThis->m_pActiveHead)
		pThis->m_pActiveHead->m_pLast = pParticle;
	pParticle->m_pNext = pThis->m_pActiveHead;
	pThis->m_pActiveHead = pParticle;
	pParticle->m_pLast = NULL;

	// update tail of active list
	if (!pThis->m_pActiveTail)
		pThis->m_pActiveTail = pParticle;

	// No attatched swooshes
	pParticle->m_ActiveSwooshes = 0;

	// not in draw list yet
	pParticle->m_pDrawLast = pParticle->m_pDrawNext = NULL;

#ifdef TRACE_MEM
	TRACE("CParticleSystem: Allocated particle: %x.\r\n", pParticle);
#endif

	ASSERT(!(pParticle->m_dwFlags & PARTICLE_ALLOCATED));
	pParticle->m_dwFlags |= PARTICLE_ALLOCATED;

	return pParticle;
}

void CParticleSystem__AddParticleToDrawList(CParticleSystem *pThis, CParticle *pParticle)
{
/*
	CGameObjectInstance	*pPlayer;
	CVector3					vDelta;
	CParticle				*pCurrent, *pNext;

	ASSERT(pParticle);

	pPlayer = CEngineApp__GetPlayer(GetApp());
	CVector3__Subtract(&vDelta, &pParticle->ah.ih.m_vPos, &pPlayer->ah.ih.m_vPos);
	pParticle->m_DistanceSquared = CVector3__MagnitudeSquared(&vDelta);

	pCurrent = NULL;
	pNext = pThis->m_pDrawHead;
	while (pNext)
	{
		if (pNext->m_DistanceSquared < pParticle->m_DistanceSquared)
			break;

		pCurrent = pNext;
		pNext = pNext->m_pDrawNext;
	}

	if (pCurrent)
	{
		// add after pCurrent
		pParticle->m_pDrawNext = pCurrent->m_pDrawNext;
		if (pCurrent->m_pDrawNext)
			pCurrent->m_pDrawNext->m_pDrawLast = pParticle;
		pParticle->m_pDrawLast = pCurrent;
		pCurrent->m_pDrawNext = pParticle;
	}
	else
	{
		// add to head of list
		if (pThis->m_pDrawHead)
			pThis->m_pDrawHead->m_pDrawLast = pParticle;
		pParticle->m_pDrawNext = pThis->m_pDrawHead;
		pThis->m_pDrawHead = pParticle;
		pParticle->m_pDrawLast = NULL;
	}
*/

	CGameObjectInstance	*pPlayer;
	CVector3					vDelta;
	CParticle				*pMatch,
								*pCurrent, *pNext;

	ASSERT(pParticle);

	pPlayer = CEngineApp__GetPlayer(GetApp());
	if (pPlayer)
		CVector3__Subtract(&vDelta, &pParticle->ah.ih.m_vPos, &pPlayer->ah.ih.m_vPos);
	else
		vDelta.x = vDelta.y = vDelta.z = 0;

	pParticle->m_DistanceSquared = CVector3__MagnitudeSquared(&vDelta);

	// find matching particle type in draw list
	pMatch = pThis->m_pDrawHead;
	while (pMatch)
	{
//		if (pMatch->ah.ih.m_nObjType == pParticle->ah.ih.m_nObjType)
		if (pMatch->m_pEffect == pParticle->m_pEffect)
			break;

		pMatch = pMatch->m_pDrawNext;
	}

	if (pMatch)
	{
		// add within matching particles in sorted order

		pCurrent = NULL;
		pNext = pMatch;
		while (pNext)
		{
			//if (		(pNext->ah.ih.m_nObjType != pParticle->ah.ih.m_nObjType)
			if (		(pNext->m_pEffect != pParticle->m_pEffect)
					|| (pNext->m_DistanceSquared < pParticle->m_DistanceSquared) )
			{
				break;
			}

			pCurrent = pNext;
			pNext = pNext->m_pDrawNext;
		}

		if (pCurrent)
		{
			// add after pCurrent
			pParticle->m_pDrawNext = pCurrent->m_pDrawNext;
			if (pCurrent->m_pDrawNext)
				pCurrent->m_pDrawNext->m_pDrawLast = pParticle;
			pParticle->m_pDrawLast = pCurrent;
			pCurrent->m_pDrawNext = pParticle;

			// update tail of list
			if (pThis->m_pDrawTail == pCurrent)
				pThis->m_pDrawTail = pParticle;
		}
		else
		{
			// add before pMatch
			pParticle->m_pDrawNext = pMatch;
			pParticle->m_pDrawLast = pMatch->m_pDrawLast;
			if (pMatch->m_pDrawLast)
				pMatch->m_pDrawLast->m_pDrawNext = pParticle;
			pMatch->m_pDrawLast = pParticle;

			// update head of list
			if (pThis->m_pDrawHead == pMatch)
				pThis->m_pDrawHead = pParticle;
		}
	}
	else
	{
		if (pParticle->m_pEffect->m_dwFlags & PARTICLE_BEHAVIOR_DRAWONBOTTOM)
		{
			// add to head of list
			if (pThis->m_pDrawHead)
				pThis->m_pDrawHead->m_pDrawLast = pParticle;
			pParticle->m_pDrawNext = pThis->m_pDrawHead;
			pThis->m_pDrawHead = pParticle;
			pParticle->m_pDrawLast = NULL;

			// update tail of list
			if (!pThis->m_pDrawTail)
				pThis->m_pDrawTail = pParticle;
		}
		else
		{
			// add to tail of list
			if (pThis->m_pDrawTail)
				pThis->m_pDrawTail->m_pDrawNext = pParticle;
			pParticle->m_pDrawLast = pThis->m_pDrawTail;
			pThis->m_pDrawTail = pParticle;
			pParticle->m_pDrawNext = NULL;

			// update head of list
			if (!pThis->m_pDrawHead)
				pThis->m_pDrawHead = pParticle;
		}
	}
}

void CParticleSystem__DeallocateParticle(CParticleSystem *pThis, CParticle *pParticle)
{
	ASSERT(pParticle);

	ASSERT(pParticle->m_dwFlags & PARTICLE_ALLOCATED);
	if (!(pParticle->m_dwFlags & PARTICLE_ALLOCATED))
		return;

	// Leave permanent particles there
	if (pParticle->m_dwFlags & PARTICLE_PERMANENT)
		return ;

	// Stop creating swooshes on particle if there are any
	if (pParticle->m_ActiveSwooshes)
	{
		// Add the last one (works with impacts)
		CParticle__CalculateOrientationMatrix(pParticle, NULL, NULL);
		CFxSystem__CheckForAddingParticleSwooshEdge(&GetApp()->m_FxSystem, pParticle, &CParticle__GetOrientationMatrix(pParticle));

		// And now stop all future impacts
		CFxSystem__StopSwooshesOnParticle(&GetApp()->m_FxSystem, pParticle);
	}

	// was this particle glaring ?
	if (pParticle == p_particle_glare)
		p_particle_glare = NULL;

	pParticle->m_dwFlags &= ~PARTICLE_ALLOCATED;

	// remove from active list
	if (pParticle->m_pLast)
		pParticle->m_pLast->m_pNext = pParticle->m_pNext;
	if (pParticle->m_pNext)
		pParticle->m_pNext->m_pLast = pParticle->m_pLast;

	// update head of active list
	if (pThis->m_pActiveHead == pParticle)
		pThis->m_pActiveHead = pParticle->m_pNext;

	// update tail of active list
	if (pThis->m_pActiveTail == pParticle)
		pThis->m_pActiveTail = pParticle->m_pLast;

	// add to free list
	pParticle->m_pNext = pThis->m_pFreeHead;
	pThis->m_pFreeHead = pParticle;

	// remove from draw list
	if (pParticle->m_pDrawLast)
		pParticle->m_pDrawLast->m_pDrawNext = pParticle->m_pDrawNext;
	if (pParticle->m_pDrawNext)
		pParticle->m_pDrawNext->m_pDrawLast = pParticle->m_pDrawLast;

	// update head of draw list
	if (pThis->m_pDrawHead == pParticle)
		pThis->m_pDrawHead = pParticle->m_pDrawNext;

	// update tail of draw list
	if (pThis->m_pDrawTail == pParticle)
		pThis->m_pDrawTail = pParticle->m_pDrawLast;

#ifdef TRACE_MEM
	TRACE("CParticleSystem: Deallocated particle: %x.\r\n", pParticle);
#endif
}

void CParticleSystem__DeallocateAllParticles(CParticleSystem *pThis)
{
	CParticle	*pParticle, *pCurrent;

	pParticle = pThis->m_pActiveHead;
	while (pParticle)
	{
		pCurrent = pParticle;

		pParticle = pParticle->m_pNext;

		CParticleSystem__DeallocateParticle(pThis, pCurrent);
	}
}

CParticle* CParticleSystem__FindMatchingSource(CParticleSystem *pThis,
															  CInstanceHdr *pSource,
															  CROMParticleEffect *pEffect /*=NULL*/)
{
	CParticle	*pParticle;

	pParticle = pThis->m_pActiveHead;

	if (pEffect)
	{
		while (pParticle)
		{
			if ((pParticle->m_pSource == pSource) && (pParticle->m_pEffect == pEffect))
				return pParticle;

			pParticle = pParticle->m_pNext;
		}
	}
	else
	{
		while (pParticle)
		{
			if (pParticle->m_pSource == pSource)
				return pParticle;

			pParticle = pParticle->m_pNext;
		}
	}

	return NULL;
}



void CParticle__CheckForStartingSwoosh(CParticle *pThis)
{
	// Add swooshes
	switch (pThis->ah.ih.m_nObjType)
	{
		case PARTICLE_TYPE_ARROWEXPLOSIVE:
			CFxSystem__AddParticleSwoosh(&GetApp()->m_FxSystem, pThis, &ExploArrowSwoosh1Info) ;
			CFxSystem__AddParticleSwoosh(&GetApp()->m_FxSystem, pThis, &ExploArrowSwoosh2Info) ;
			break ;

		case PARTICLE_TYPE_CHRONOSCEPTER:
			CFxSystem__AddParticleSwoosh(&GetApp()->m_FxSystem, pThis, &ChronoSwoosh1Info) ;
			CFxSystem__AddParticleSwoosh(&GetApp()->m_FxSystem, pThis, &ChronoSwoosh2Info) ;
			break ;

		case PARTICLE_TYPE_GENERIC220:
			CFxSystem__AddParticleSwoosh(&GetApp()->m_FxSystem, pThis, &Generic220SwooshInfo) ;
			break ;

		case PARTICLE_TYPE_GENERIC232:
			CFxSystem__AddParticleSwoosh(&GetApp()->m_FxSystem, pThis, &Generic232SwooshInfo) ;
			break ;

		case PARTICLE_TYPE_GENERIC222:
			CFxSystem__AddParticleSwoosh(&GetApp()->m_FxSystem, pThis, &Generic222SwooshInfo) ;
			break ;

		case PARTICLE_TYPE_GENERIC225:
			CFxSystem__AddParticleSwoosh(&GetApp()->m_FxSystem, pThis, &Generic225SwooshInfo) ;
			break ;

		case PARTICLE_TYPE_GENERIC231:
			CFxSystem__AddParticleSwoosh(&GetApp()->m_FxSystem, pThis, &Generic231SwooshInfo) ;
			break ;

 		case PARTICLE_TYPE_GENERIC258:
 			CFxSystem__AddParticleSwoosh(&GetApp()->m_FxSystem, pThis, &DemonSwooshInfo) ;
 			break ;

		case PARTICLE_TYPE_GENERIC247:
			CFxSystem__AddParticleSwoosh(&GetApp()->m_FxSystem, pThis, &Generic247SwooshInfo) ;
			break ;

		case PARTICLE_TYPE_GENERIC46:
			CFxSystem__AddParticleSwoosh(&GetApp()->m_FxSystem, pThis, &CampSwoosh1Info) ;
			CFxSystem__AddParticleSwoosh(&GetApp()->m_FxSystem, pThis, &CampSwoosh2Info) ;
			break ;

 		case PARTICLE_TYPE_PLASMA1:
 			CFxSystem__AddParticleSwoosh(&GetApp()->m_FxSystem, pThis, &PlasmaSwooshInfo) ;
 			break ;

		case PARTICLE_TYPE_ROCKET:
			CFxSystem__AddParticleSwoosh(&GetApp()->m_FxSystem, pThis, &RocketSwooshInfo) ;
			break ;

		case PARTICLE_TYPE_EYEFIRE:
			CFxSystem__AddParticleSwoosh(&GetApp()->m_FxSystem, pThis, &TRexSwoosh1Info) ;
			CFxSystem__AddParticleSwoosh(&GetApp()->m_FxSystem, pThis, &TRexSwoosh2Info) ;
			break ;
	}
}


void CParticleSystem__CreateParticle(CParticleSystem *pThis, CInstanceHdr *pSource,
												 int nType,
												 CVector3 vBaseVelocity, CQuatern qRotateDirection,
												 CVector3 vPos, CGameRegion *pRegion,
												 int nEdge, BOOL Blood)
{
	CIndexedSet				isTextureSetsIndex;
	CMtxF						mfRot;
	CVector3					vDirection, vRotDir,
								vPosRand, vRotPosRand,
								vDelta;
	CParticle				*pParticle;

	signed char				randHue;
	CCollisionInfo2		*pCI;
	int						cParticle, nParticles,
								alignment,
								cEffect, nEffects,
								nFrames,
								first, last;
	CROMParticleEffect	*effects, *pEffect;
	BOOL						create;
	float						groundHeight;
	CIndexedSet				isParticleEffects;
	void						*pbTypes, *pbParticles;
	CUnindexedSet			usTypes,
								usParticles;
	DWORD						*types;
	CQuatern					qUseRotate;
	BYTE						randSat, randBri,
								r, g, b,
								r2, g2, b2;
	CGameObjectInstance	*pPlayer;
	float						distanceSquared;
	ROMAddress				*rpTextureSets;


#define	PARTICLE_THRESHOLD_DISTANCE				(20*SCALING_FACTOR)
#define	PARITCLE_THRESHOLD_DISTANCE_SQUARED		(PARTICLE_THRESHOLD_DISTANCE*PARTICLE_THRESHOLD_DISTANCE)

	if (GetApp()->m_bPause)
		return;

	if (!(pThis->m_pceParticleEffects && pThis->m_pceTextureSetsIndex))
		return;

/*
	// TEMP
	if (nType == PARTICLE_TYPE_ARROW)
//		nType = PARTICLE_TYPE_ARROWEXPLOSIVE ;
		nType = PARTICLE_TYPE_GENERIC220 ;
	// TEMP
*/

	CIndexedSet__ConstructFromRawData(&isParticleEffects,
												 CCacheEntry__GetData(pThis->m_pceParticleEffects),
												 FALSE);

	pbTypes = CIndexedSet__GetBlock(&isParticleEffects, CART_PARTICLEEFFECTS_usTypes);
	CUnindexedSet__ConstructFromRawData(&usTypes, pbTypes, FALSE);
	types = CUnindexedSet__GetBasePtr(&usTypes);
	nEffects = CUnindexedSet__GetBlockCount(&usTypes);

	if (BinaryRange(types, nEffects, nType, &first, &last))
	{
		pbParticles = CIndexedSet__GetBlock(&isParticleEffects, CART_PARTICLEEFFECTS_usParticles);
		CUnindexedSet__ConstructFromRawData(&usParticles, pbParticles, FALSE);
		effects = (CROMParticleEffect*) CUnindexedSet__GetBasePtr(&usParticles);
		ASSERT(nEffects == CUnindexedSet__GetBlockCount(&usParticles));

		CIndexedSet__ConstructFromRawData(&isTextureSetsIndex,
													 CCacheEntry__GetData(pThis->m_pceTextureSetsIndex),
													 FALSE);

		for (cEffect=first; cEffect<=last; cEffect++)
		{
			pEffect = &effects[cEffect];

			create = TRUE;

			if ((pEffect->m_dwFlags & PARTICLE_BEHAVIOR_CREATEONCE) && pSource)
				if (CParticleSystem__FindMatchingSource(pThis, pSource, pEffect))
					create = FALSE;

			pPlayer = CEngineApp__GetPlayer(GetApp());
			if (pPlayer)
				CVector3__Subtract(&vDelta, &vPos, &pPlayer->ah.ih.m_vPos);
			else
				vDelta.x = vDelta.y = vDelta.z = 0;

			distanceSquared = CVector3__MagnitudeSquared(&vDelta);

			if (pEffect->m_dwFlags & PARTICLE_BEHAVIOR_PLAYONLYWHENNEAR)
			{
				if (distanceSquared >= PARITCLE_THRESHOLD_DISTANCE_SQUARED)
					create = FALSE;
			}
			else if (pEffect->m_dwFlags & PARTICLE_BEHAVIOR_PLAYONLYWHENFAR)
			{
				if (distanceSquared <= PARITCLE_THRESHOLD_DISTANCE_SQUARED)
					create = FALSE;
			}

			switch (pEffect->m_Alignment)
			{
				case PARTICLE_ALIGN_YAXIS:
					alignment = PARTICLE_SCREENY;
					break;

				case PARTICLE_ALIGN_SCREEN:
					alignment = PARTICLE_SCREEN;
					break;

				case PARTICLE_ALIGN_UP:
					alignment = PARTICLE_UP;
					break;

				case PARTICLE_ALIGN_GROUND:
					switch (nEdge)
					{
						case PARTICLE_GROUND:
						case PARTICLE_CEILING:
						case PARTICLE_UP:
							alignment = nEdge;
							break;

						default:
							groundHeight = CGameRegion__GetGroundHeight(pRegion, vPos.x, vPos.z);
							if ((vPos.y - groundHeight) <= 0.01)
								alignment = PARTICLE_GROUND;
							else
								create = FALSE;
							break;
					}
					break;

				case PARTICLE_ALIGN_WALL:
					if (nEdge >= 0)
						alignment = nEdge;
					else
						create = FALSE;
					break;

				case PARTICLE_ALIGN_AUTO:
					alignment = nEdge;
					break;

				default:
					ASSERT(FALSE);
			}

			// is blood in options menu enabled ?
			if (		(pEffect->m_dwFlags & PARTICLE_BEHAVIOR_BLOODEFFECT)
					&& (!GetApp()->m_Options.m_Blood || !Blood) )
			{
				create = FALSE;
			}

			// is impact effect enabled ?
			if (		(pEffect->m_dwFlags & PARTICLE_BEHAVIOR_IMPACTEFFECT)
					&&	GetApp()->m_Options.m_Blood )
			{
				create = FALSE;
			}

			if (create)
			{
				if (pEffect->m_dwFlags & PARTICLE_BEHAVIOR_GLOBALCOORD)
				{
					CMtxF__Identity(mfRot);
				}
				else
				{
					if (pEffect->m_dwFlags & PARTICLE_BEHAVIOR_AIMATPLAYER)
						qUseRotate = CInstanceHdr__GetAimAtTarget(pSource, vPos);
					else
						qUseRotate = qRotateDirection;

					CQuatern__ToMatrix(&qUseRotate, mfRot);
				}

				nParticles = pEffect->m_nParticles + CParticle__GetRandom(pEffect->m_nParticleRandom);

				for (cParticle=0; cParticle<nParticles; cParticle++)
				{
					nFrames = pEffect->m_nParticleFrames + CParticle__GetRandom(pEffect->m_nParticleFrameRandom);
					if (nFrames)
					{
						pParticle = CParticleSystem__AllocateParticle(pThis, pEffect->m_Priority);
						if (pParticle)
						{
							rpTextureSets = CCacheEntry__GetRequestedAddress(pThis->m_pceTextureSetsIndex);
							if (		(pEffect->m_dwFlags & PARTICLE_BEHAVIOR_BLOODEFFECT)
									&&	(GetApp()->m_Options.m_Blood == BLOOD_GREEN) )
							{
								// set version to swap red and green entries in palette
								rpTextureSets = (ROMAddress*) ((DWORD)rpTextureSets | CACHE_BLOCK_VERSION(1));
							}

							CTextureLoader__ConstructFromIndex(&pParticle->m_TextureLoader,
																		  &isTextureSetsIndex,
																		  rpTextureSets,
																		  pEffect->m_nTextureSet);

							pParticle->ah.ih.m_nObjType = nType;
							pParticle->m_pEffect = pEffect;

							pParticle->m_Alignment = alignment;
							pParticle->m_pSource = pSource;

							pParticle->m_cFrame		= - (float) CParticle__GetRandom(pEffect->m_nParticleDelayFramesRandom);
							pParticle->m_cFramePos  = 0.0;

							pParticle->m_nFrames		= nFrames;

							pParticle->m_FPS = pEffect->m_FPS;

							pParticle->m_dwFlags &= ~(PARTICLE_HASHITGROUND | PARTICLE_HASHITWALL);

							pParticle->m_cAnimFrame = 0;

							pParticle->m_nRandomFrame = -1;

							pParticle->m_Size			= SF2FLOAT(pEffect->m_Size)   + CParticle__GetRandomFloat(SF2FLOAT(pEffect->m_SizeRandom));

							if (GetApp()->m_dwCheatFlags & CHEATFLAG_QUACKMODE)
								pParticle->m_Size *= 0.35;

							pParticle->m_SizeScaler	= SF2FLOAT(pEffect->m_Scaler) + CParticle__GetRandomFloat(SF2FLOAT(pEffect->m_ScalerRandom));

							pParticle->m_AlphaScaler = 255;

							vDirection.x = SF2FLOAT(pEffect->m_DirectionX);
							vDirection.y = SF2FLOAT(pEffect->m_DirectionY);
							vDirection.z = SF2FLOAT(pEffect->m_DirectionZ);

							vDirection = CVector3__RandomizeDirectionX(&vDirection, SF2FLOAT(pEffect->m_SprayX));
							vDirection = CVector3__RandomizeDirectionY(&vDirection, SF2FLOAT(pEffect->m_SprayY));
							vDirection = CVector3__RandomizeDirectionZ(&vDirection, SF2FLOAT(pEffect->m_SprayZ));

							vPosRand.x = SF2FLOAT(pEffect->m_PosX) + CParticle__GetRandomFloat(SF2FLOAT(pEffect->m_PosRandomX));
							vPosRand.y = SF2FLOAT(pEffect->m_PosY) + CParticle__GetRandomFloat(SF2FLOAT(pEffect->m_PosRandomY));
							vPosRand.z = SF2FLOAT(pEffect->m_PosZ) + CParticle__GetRandomFloat(SF2FLOAT(pEffect->m_PosRandomZ));

							if (pEffect->m_WallBehavior == INTERSECT_BEHAVIOR_IGNORE)
								pParticle->ah.ih.m_pCurrentRegion = NULL;
							else
								pParticle->ah.ih.m_pCurrentRegion	= pRegion;

							pParticle->ah.ih.m_vPos					= vPos;

							if ((vPosRand.x != 0) || (vPosRand.y != 0) || (vPosRand.z != 0))
							{
								CMtxF__VectorMult(mfRot, &vPosRand, &vRotPosRand);
								CVector3__Add(&vRotPosRand, &vPos, &vRotPosRand);

								CInstanceHdr__GetNearPositionAndRegion(&pParticle->ah.ih,
																					vRotPosRand,
																					&pParticle->ah.ih.m_vPos,
																					&pParticle->ah.ih.m_pCurrentRegion,
																					pEffect->m_GroundBehavior,
																					pEffect->m_InstanceBehavior);
							}

							if ((vDirection.x != 0) || (vDirection.y != 0) || (vDirection.z != 0))
							{
								CMtxF__VectorMult(mfRot, &vDirection, &vRotDir);
								vDirection = CVector3__RandomizeDirection(&vRotDir, SF2FLOAT(pEffect->m_DirectionSpray));
							}

							// delay velocity ?
//							pParticle->m_nVelocityDelay = pEffect->m_nVelocityDelay;
//							if (pParticle->m_nVelocityDelay)
//							{
//								pParticle->ah.m_vVelocity.x = pParticle->ah.m_vVelocity.y = pParticle->ah.m_vVelocity.z = 0;
//							}
//							else
//							{
								// Check for starting swoosh
								CParticle__CheckForStartingSwoosh(pParticle) ;


								// set particle velocity with no velocity delay
								CVector3__MultScaler(&pParticle->ah.m_vVelocity, &vDirection,
															SF2FLOAT(pEffect->m_Velocity) + CParticle__GetRandomFloat(SF2FLOAT(pEffect->m_VelocityRandom)));
//							}

							if (pEffect->m_dwFlags & PARTICLE_BEHAVIOR_ADDSOURCEVELOCITY)
								CVector3__Add(&pParticle->ah.m_vVelocity, &pParticle->ah.m_vVelocity, &vBaseVelocity);

							if (GetApp()->m_dwCheatFlags & CHEATFLAG_QUACKMODE)
								pParticle->m_Rotation = 0;

							pParticle->m_Rotation = SF2FLOAT(pEffect->m_Rotation) + CParticle__GetRandomFloat(SF2FLOAT(pEffect->m_RotationRandom));
							pParticle->m_RotationInc = SF2FLOAT(pEffect->m_RotationInc) + CParticle__GetRandomFloat(SF2FLOAT(pEffect->m_RotationIncRandom));

							// colors
							if (pEffect->m_RandomizeHue)
								randHue = (signed char) (RANDOM(pEffect->m_RandomizeHue) - pEffect->m_RandomizeHue/2);
							else
								randHue = 0;

							if (pEffect->m_RandomizeSaturation)
								randSat = (BYTE) RANDOM(pEffect->m_RandomizeSaturation);
							else
								randSat = 0;

							if (pEffect->m_RandomizeBrightness)
								randBri = (BYTE) RANDOM(pEffect->m_RandomizeBrightness);
							else
								randBri = 0;

							if (		(pEffect->m_dwFlags & PARTICLE_BEHAVIOR_BLOODEFFECT)
									&&	(GetApp()->m_Options.m_Blood == BLOOD_GREEN) )
							{
								r = pEffect->m_WhiteColor[1];
								g = pEffect->m_WhiteColor[0];
								b = pEffect->m_WhiteColor[2];
							}
							else
							{
								r = pEffect->m_WhiteColor[0];
								g = pEffect->m_WhiteColor[1];
								b = pEffect->m_WhiteColor[2];
							}

							GetOffsetHue(randHue, r, g, b, &r2, &g2, &b2);
							GetReducedSaturation(randSat, r2, g2, b2, &r, &g, &b);
							GetReducedBrightness(randBri, r, g, b, &r2, &g2, &b2);

							pParticle->m_WhiteColor[0] = r2;
							pParticle->m_WhiteColor[1] = g2;
							pParticle->m_WhiteColor[2] = b2;

							if (		(pEffect->m_dwFlags & PARTICLE_BEHAVIOR_BLOODEFFECT)
									&&	(GetApp()->m_Options.m_Blood == BLOOD_GREEN) )
							{
								r = pEffect->m_BlackColor[1];
								g = pEffect->m_BlackColor[0];
								b = pEffect->m_BlackColor[2];
							}
							else
							{
								r = pEffect->m_BlackColor[0];
								g = pEffect->m_BlackColor[1];
								b = pEffect->m_BlackColor[2];
							}

							GetOffsetHue(randHue, r, g, b, &r2, &g2, &b2);
							GetReducedSaturation(randSat, r2, g2, b2, &r, &g, &b);
							GetReducedBrightness(randBri, r, g, b, &r2, &g2, &b2);

							pParticle->m_BlackColor[0] = r2;
							pParticle->m_BlackColor[1] = g2;
							pParticle->m_BlackColor[2] = b2;


							pCI = &pParticle->m_CI;
							CCollisionInfo__SetParticleDefaults(pCI);

							pCI->WallBehavior				= pEffect->m_WallBehavior;
							pCI->InstanceBehavior		= pEffect->m_InstanceBehavior;
							pCI->GroundBehavior			= pEffect->m_GroundBehavior;
							pCI->GravityAcceleration 	= SF2FLOAT(pEffect->m_Gravity)        + CParticle__GetRandomFloat(SF2FLOAT(pEffect->m_GravityRandom));
							pCI->BounceReturnEnergy  	= SF2FLOAT(pEffect->m_BounceEnergy);
							pCI->GroundFriction      	= SF2FLOAT(pEffect->m_GroundFriction);

							pCI->AirFriction				= SF2FLOAT(pEffect->m_AirFriction);
							pCI->WaterFriction			= SF2FLOAT(pEffect->m_WaterFriction);

							if (pEffect->m_dwFlags & PARTICLE_BEHAVIOR_BOUNCEONLYONCE)
								pCI->dwFlags |= COLLISIONFLAG_ENDONBOUNCE;

							//if (pEffect->m_dwFlags & PARTICLE_BEHAVIOR_RETAINVELOCITY)
							//	pCI->m_dwFlags &= ~COLLISIONFLAG_UPDATEVELOCITY;
							//else
							//	pCI->m_dwFlags |= COLLISIONFLAG_UPDATEVELOCITY;

							if (pEffect->m_Visibility != PARTICLE_VISIBLE_INVISIBLE)
								CParticleSystem__AddParticleToDrawList(pThis, pParticle);

							if (pEffect->m_dwFlags & PARTICLE_BEHAVIOR_GLARE)
							{
								v_particle_glare_pos = pParticle->ah.ih.m_vPos;
								particle_glare_color[0] = pParticle->m_WhiteColor[0];
								particle_glare_color[1] = pParticle->m_WhiteColor[1];
								particle_glare_color[2] = pParticle->m_WhiteColor[2];
								p_particle_glare = pParticle;
							}
						}
						else
						{
							break;
						}
					}
				}
			}
		}

		CUnindexedSet__Destruct(&usParticles);
		CIndexedSet__Destruct(&isTextureSetsIndex);
	}

	CIndexedSet__Destruct(&isParticleEffects);
	CUnindexedSet__Destruct(&usTypes);
}

void CParticleSystem__Advance(CParticleSystem *pThis)
{
	CParticle	*pParticle, *pCurrent;
	float			prevFrameInc;

	prevFrameInc = frame_increment;
	frame_increment *= particle_speed_scaler;

	pParticle = pThis->m_pActiveHead;
	while (pParticle)
	{
		pCurrent = pParticle;
		pParticle = pParticle->m_pNext;

		if ((pCurrent->m_cFrame < pCurrent->m_nFrames) && (pCurrent->m_nFrames != 1) &&
			 (!(pCurrent->m_dwFlags & PARTICLE_DELETE)))
		{
			if (CBoundsRect__InRect(&anim_bounds_rect, &pCurrent->ah.ih.m_vPos))
			{
				CParticle__Advance(pCurrent);

				pCurrent->m_cFrame += frame_increment;
			}
			else
			{
				CParticleSystem__DeallocateParticle(pThis, pCurrent);
			}
		}
		else
		{
			CParticle__Done(pCurrent);
			CParticleSystem__DeallocateParticle(pThis, pCurrent);
		}
	}

	frame_increment = prevFrameInc;
}

void CParticleSystem__DetonateGrenade(CParticleSystem *pThis)
{
	CParticle	*pParticle;

	pParticle = pThis->m_pActiveTail;
	while (pParticle)
	{
		//if ((pParticle->ah.ih.m_nObjType == PARTICLE_TYPE_GRENADE)  && (pParticle->m_nFrames < 0))
		if (pParticle->ah.ih.m_nObjType == PARTICLE_TYPE_GRENADE)
		{
			pParticle->m_cFrame = pParticle->m_nFrames;
			break;
		}

		pParticle = pParticle->m_pLast;
	}
}

void CParticleSystem__DetonateAllGrenades(CParticleSystem *pThis)
{
	CParticle	*pParticle;
	int			time;

	time = 1;

	pParticle = pThis->m_pActiveTail;
	while (pParticle)
	{
		//if ((pParticle->ah.ih.m_nObjType == PARTICLE_TYPE_GRENADE)  && (pParticle->m_nFrames < 0))
		if (pParticle->ah.ih.m_nObjType == PARTICLE_TYPE_GRENADE)
		{
			time += 2;
			pParticle->m_cFrame = pParticle->m_nFrames - time;
		}

		pParticle = pParticle->m_pLast;
	}
}

void CParticleSystem__RequestData(CParticleSystem *pThis, CCartCache *pCache,
											 ROMAddress *rpParticleEffectsAddress,
											 DWORD ParticleEffectsSize,
											 CCacheEntry *pceTextureSetsIndex)
{
	pThis->m_pCache = pCache;
	pThis->m_pceTextureSetsIndex = pceTextureSetsIndex;

   pThis->m_pceParticleEffects = NULL;
	CCartCache__RequestBlock(pThis->m_pCache,
									 pThis, (pfnCACHENOTIFY) CParticleSystem__DecompressParticles, NULL,
									 &pThis->m_pceParticleEffects,
									 rpParticleEffectsAddress, ParticleEffectsSize,
									 "Particle Effects");
}

void CParticleSystem__DecompressParticles(CParticleSystem *pThis, CCacheEntry **ppceTarget)
{
	CIndexedSet				isParticleEffects;
	void						*pbParticles,
								*pbImpacts;
	CUnindexedSet			usParticles,
								usImpacts;
	CROMParticleEffect	*effects;
	CROMParticleImpact	*impacts;
	int						cEffect, nEffects;

	ASSERT(ppceTarget == &pThis->m_pceParticleEffects);

	TRACE0("CParticleSystem: Decompressing particle data.\r\n");

	if (!CCacheEntry__Decompress(pThis->m_pceParticleEffects))
		return;

	CIndexedSet__ConstructFromRawData(&isParticleEffects,
												 CCacheEntry__GetData(pThis->m_pceParticleEffects),
												 FALSE);

	pbParticles = CIndexedSet__GetBlock(&isParticleEffects, CART_PARTICLEEFFECTS_usParticles);
	CUnindexedSet__ConstructFromRawData(&usParticles, pbParticles, FALSE);
	effects = (CROMParticleEffect*) CUnindexedSet__GetBasePtr(&usParticles);
	nEffects = CUnindexedSet__GetBlockCount(&usParticles);

	pbImpacts = CIndexedSet__GetBlock(&isParticleEffects, CART_PARTICLEEFFECTS_usImpacts);
	CUnindexedSet__ConstructFromRawData(&usImpacts, pbImpacts, FALSE);
	impacts = (CROMParticleImpact*) CUnindexedSet__GetBasePtr(&usImpacts);

	for (cEffect=0; cEffect<nEffects; cEffect++)
		effects[cEffect].m_pImpact = &impacts[(int) effects[cEffect].m_pImpact];
		//effects[cEffect]->m_pImpact = &impacts[(int) effects[cEffect]->m_pImpact];

	CIndexedSet__Destruct(&isParticleEffects);
	CUnindexedSet__Destruct(&usParticles);
	CUnindexedSet__Destruct(&usImpacts;);
}

// LEAVES IN 1 CYCLE MODE
void CParticleSystem__Draw(CParticleSystem *pThis, Gfx **ppDLP)
{
	CParticle		*pParticle;
	CROMBounds		bounds;
	float				diag, diagD2;

	if (pThis->m_pceParticleEffects)
		CCacheEntry__ResetAge(pThis->m_pceParticleEffects);

	if (pThis->m_pActiveHead)
	{
		ASSERT(pThis->m_pceParticleEffects);

		gDPPipeSync((*ppDLP)++);

		gDPSetAlphaDither((*ppDLP)++, G_AD_NOISE);
  		gDPSetColorDither((*ppDLP)++, G_CD_ENABLE);

		CQuatern__ToMatrix(&GetApp()->m_qViewAlignNoZ, mf_particle_facescreen);
		CMtxF__RotateY(mf_particle_facescreen_yaxis, GetApp()->m_RotY) ;

		gDPSetCycleType((*ppDLP)++, G_CYC_1CYCLE);

  		gSPClearGeometryMode((*ppDLP)++, G_LIGHTING | G_CULL_BACK | G_TEXTURE_GEN);
		//gDPSetDepthSource((*ppDLP)++, G_ZS_PRIM);

		pParticle = pThis->m_pDrawHead;
		while (pParticle)
		{
			diag =  1.414213562373*pParticle->m_Size;
			diagD2 = 0.5*diag;

			bounds.m_vMin.x = pParticle->ah.ih.m_vPos.x - diagD2;
			bounds.m_vMax.x = pParticle->ah.ih.m_vPos.x + diagD2;
			bounds.m_vMin.y = pParticle->ah.ih.m_vPos.y - diagD2;
			bounds.m_vMax.y = pParticle->ah.ih.m_vPos.y + diag;
			bounds.m_vMin.z = pParticle->ah.ih.m_vPos.z - diagD2;
			bounds.m_vMax.z = pParticle->ah.ih.m_vPos.z + diagD2;

			// draw particle if visible, if not visible, still add swooshes
			if (		CBoundsRect__IsOverlappingBounds(&view_bounds_rect, &bounds)
					&& CViewVolume__IsOverlappingBounds(&view_volume, &bounds) )
			{
				CParticle__Draw(pParticle, ppDLP, pThis->m_pCache);
			}
			else if (pParticle->m_ActiveSwooshes)
			{
				CParticle__CalculateOrientationMatrix(pParticle, NULL,NULL) ;
				CFxSystem__CheckForAddingParticleSwooshEdge(&GetApp()->m_FxSystem, pParticle, &CParticle__GetOrientationMatrix(pParticle)) ;
			}

			pParticle = pParticle->m_pDrawNext;
		}

		gDPPipeSync((*ppDLP)++);

		//gDPSetDepthSource((*ppDLP)++, G_ZS_PIXEL);

#ifdef SCREEN_SHOT
  		gDPSetColorDither((*ppDLP)++, G_CD_ENABLE);
		gDPSetAlphaDither((*ppDLP)++, G_AD_NOISE);
#else
  		gDPSetColorDither((*ppDLP)++, G_CD_MAGICSQ);
		gDPSetAlphaDither((*ppDLP)++, G_AD_NOTPATTERN);
#endif	// SCREEN_SHOT

		//if (cloud_sparkle)
		//{
		//	gDPSetAlphaCompare((*ppDLP)++, G_AC_NONE);
		//	cloud_sparkle = FALSE ;
	  	//}
	}
}

// process impact collision here
void CParticle__Advance(CParticle *pThis)
{
	//CMtxF			mfRot;
	//CVector3		vDirection, vRotDir;
	float			newSize;

	if (pThis->m_cFrame < 0)
		return;

	if (pThis->m_pEffect->m_FpsVelThreshold)
		pThis->m_FPS = pThis->m_pEffect->m_FPS * min(1, CVector3__Magnitude(&pThis->ah.m_vVelocity) / SF2FLOAT(pThis->m_pEffect->m_FpsVelThreshold));

	pThis->m_cFramePos += frame_increment*pThis->m_FPS*(1.0/FRAME_FPS)*particle_speed_scaler;

	if (pThis->m_pEffect->m_dwFlags & PARTICLE_BEHAVIOR_SCALELINEAR)
	{
		pThis->m_Size += pThis->m_SizeScaler*SCALING_FACTOR*frame_increment;
		pThis->m_Size = min(400*SCALING_FACTOR, pThis->m_Size);
	}
	else
	{
		newSize = min(400*SCALING_FACTOR, pThis->m_Size*pThis->m_SizeScaler);
		pThis->m_Size += (newSize - pThis->m_Size)*frame_increment;
		if (pThis->m_Size < 0)
		{
			CParticle__Done(pThis);
			CParticle__Delete(pThis);
			return;
		}
	}

	if ((GetApp()->m_dwCheatFlags & CHEATFLAG_QUACKMODE) == 0)
	{
	//	pThis->m_Rotation += (SF2FLOAT(pThis->m_pEffect->m_RotationInc)*frame_increment);
		pThis->m_Rotation += (pThis->m_RotationInc * frame_increment);
	}

	// update every frame particle stuff
	CParticle__EveryFrame(pThis);

	// don't do collision if particle has come to rest
	if (		((pThis->m_CI.GravityAcceleration == 0) || ((pThis->ah.ih.m_vPos.y == 0) && (pThis->m_CI.GravityAcceleration < 0)))
			&& (CVector3__MagnitudeSquared(&pThis->ah.m_vVelocity) < 0.001) )
	{
		return;
	}

	CAnimInstanceHdr__Collision3(&pThis->ah, pThis->ah.ih.m_vPos, &pThis->m_CI);

	// update glare effect on particle ?
	if (pThis == p_particle_glare)
		v_particle_glare_pos = pThis->ah.ih.m_vPos;

	// velocity delay ?
//	if (pThis->m_nVelocityDelay)
//	{
//		if (--pThis->m_nVelocityDelay == 0)
//		{
//			// Check for starting swoosh
//			CParticle__CheckForStartingSwoosh(pThis) ;
//
//			vDirection.x = SF2FLOAT(pThis->m_pEffect->m_DirectionX);
//			vDirection.y = SF2FLOAT(pThis->m_pEffect->m_DirectionY);
//			vDirection.z = SF2FLOAT(pThis->m_pEffect->m_DirectionZ);
//
//			// TEMP - vDirection needs to rotate as turok changes view direction
//			CMtxF__VectorMult(mfRot, &vDirection, &vRotDir);
//			vDirection = CVector3__RandomizeDirection(&vRotDir, SF2FLOAT(pThis->m_pEffect->m_DirectionSpray));
//
//			// set particle velocity with no velocity delay
//			CVector3__MultScaler(&pThis->ah.m_vVelocity, &vDirection,
//										SF2FLOAT(pThis->m_pEffect->m_Velocity) + CParticle__GetRandomFloat(SF2FLOAT(pThis->m_pEffect->m_VelocityRandom)));
//		}
//	}

	// collision
	if (pThis->m_CI.pWaterCollisionRegion)
	{
		CParticle__WaterSurfaceCollision(pThis);

		pThis->m_dwFlags |= PARTICLE_HASHITGROUND;
	}

	if (pThis->m_dwFlags & PARTICLE_ALLOCATED)
	{
		if (pThis->m_CI.pWallCollisionRegion)
		{
			if (!(pThis->m_pEffect->m_dwFlags & PARTICLE_BEHAVIOR_NOWALLSPAWN))
				CParticle__WallCollision(pThis);

			pThis->m_dwFlags |= PARTICLE_HASHITWALL;
			if (pThis->m_pEffect->m_dwFlags & PARTICLE_BEHAVIOR_BOUNCEONLYONCE)
				pThis->m_CI.WallBehavior = INTERSECT_BEHAVIOR_STICK;
		}

		if (pThis->m_dwFlags & PARTICLE_ALLOCATED)
		{
			if (pThis->m_CI.pInstanceCollision)
				CParticle__InstanceCollision(pThis);

			if (pThis->m_dwFlags & PARTICLE_ALLOCATED)
			{
				if (		(pThis->m_CI.nGroundCollisionType != GROUNDCOLLISION_NONE)
						&& !(pThis->m_pEffect->m_dwFlags & PARTICLE_BEHAVIOR_NOGRDSPAWN) )
				{
					if (pThis->m_CI.nGroundCollisionType == GROUNDCOLLISION_GROUND)
							CParticle__GroundCollision(pThis);
					else	// (pThis->m_CI.nGroundCollisionType == GROUNDCOLLISION_CEILING)
							CParticle__CeilingCollision(pThis);

					pThis->m_dwFlags |= PARTICLE_HASHITGROUND;
				}
			}
		}
	}
}


// particle wall collision
//
void CParticle__WallCollision(CParticle *pThis)
{
	// declare variables
	int						nWall,
								nParticleType,
								nEventType,
								nSoundType;
	float						EventNumber;
	CROMParticleEffect	*pEffect;

	// don't do anything on second wall hit
	if (!(		(pThis->m_dwFlags & PARTICLE_HASHITWALL)
				&& (pThis->m_pEffect->m_dwFlags & PARTICLE_BEHAVIOR_BOUNCEONLYONCE) ))
	{
		nWall   = CGameRegion__GetWallMaterial(pThis->ah.ih.m_pCurrentRegion);
		pEffect = pThis->m_pEffect;

		nParticleType = pEffect->m_pImpact->m_ImpactParticleType[nWall];
		nEventType    = pEffect->m_pImpact->m_ImpactEventType   [nWall];
		EventNumber   = pEffect->m_pImpact->m_ImpactEventNumber [nWall];
		nSoundType    = pEffect->m_pImpact->m_ImpactSoundType   [nWall];


		if (nParticleType != (WORD) -1)
			CParticle__SpawnWallCollision(pThis, nParticleType);

		if (nEventType != (WORD) -1)
			AI_Event_Dispatcher(&pThis->ah.ih, pThis->m_pSource, nEventType, AI_SPECIES_ALL, pThis->m_CI.vWallCollisionPos, EventNumber);

		if (nSoundType != (WORD) -1)
			CParticle__DoSoundEffect(pThis, nSoundType, FALSE, &pThis->m_CI.vWallCollisionPos);
	}

	// ?
	if (pThis->m_CI.WallBehavior == INTERSECT_BEHAVIOR_STICK)
		CParticle__Delete(pThis);
}


void CParticle__WaterSurfaceCollision(CParticle *pThis)
{
	// declare variables
	int						nParticleType,
								nEventType,
								nSoundType;
	float						EventNumber;
	CROMParticleEffect	*pEffect;


	pEffect = pThis->m_pEffect;

	nParticleType = pEffect->m_pImpact->m_ImpactParticleType[PARTICLE_IMPACTS_WATERSURFACE];
	nEventType    = pEffect->m_pImpact->m_ImpactEventType   [PARTICLE_IMPACTS_WATERSURFACE];
	EventNumber   = pEffect->m_pImpact->m_ImpactEventNumber [PARTICLE_IMPACTS_WATERSURFACE];
	nSoundType    = pEffect->m_pImpact->m_ImpactSoundType   [PARTICLE_IMPACTS_WATERSURFACE];

	if (nParticleType != (WORD) -1)
		CParticle__SpawnWaterSurfaceCollision(pThis, nParticleType);

	if (nEventType != (WORD) -1)
		AI_Event_Dispatcher(&pThis->ah.ih, pThis->m_pSource, nEventType, AI_SPECIES_ALL, pThis->m_CI.vWaterCollisionPos, EventNumber);

	if (nSoundType != (WORD) -1)
		CParticle__DoSoundEffect(pThis, nSoundType, FALSE, &pThis->m_CI.vWaterCollisionPos);

	if (pThis->m_pEffect->m_dwFlags & PARTICLE_BEHAVIOR_DIEONWATERSURFACE)
		CParticle__Delete(pThis);
}

// particle ground collision
//
void CParticle__GroundCollision(CParticle *pThis)
{
	// declare variables
	int						nGrd,
								nParticleType,
								nEventType,
								nSoundType;
	float						EventNumber;
	CROMParticleEffect	*pEffect;

	if (CGameRegion__IsCliff(pThis->ah.ih.m_pCurrentRegion))
		nGrd = CGameRegion__GetWallMaterial(pThis->ah.ih.m_pCurrentRegion);
	else
		nGrd = CGameRegion__GetGroundMaterial(pThis->ah.ih.m_pCurrentRegion);

	pEffect = pThis->m_pEffect;

	nParticleType = pEffect->m_pImpact->m_ImpactParticleType[nGrd];
	nEventType    = pEffect->m_pImpact->m_ImpactEventType   [nGrd];
	EventNumber   = pEffect->m_pImpact->m_ImpactEventNumber [nGrd];
	nSoundType    = pEffect->m_pImpact->m_ImpactSoundType   [nGrd];

	if (nParticleType != (WORD) -1)
		CParticle__SpawnGroundCollision(pThis, nParticleType);

	if (nEventType != (WORD) -1)
		AI_Event_Dispatcher(&pThis->ah.ih, pThis->m_pSource, nEventType, AI_SPECIES_ALL, pThis->m_CI.vGroundCollisionPos, EventNumber);

	if (nSoundType != (WORD) -1)
		CParticle__DoSoundEffect(pThis, nSoundType, FALSE, &pThis->m_CI.vGroundCollisionPos);

	// ?
	if (pThis->m_CI.GroundBehavior == INTERSECT_BEHAVIOR_STICK)
		CParticle__Delete(pThis);
}

// particle ground collision
//
void CParticle__CeilingCollision(CParticle *pThis)
{
	// declare variables
	int						nWall,
								nParticleType,
								nEventType,
								nSoundType;
	float						EventNumber;
	CROMParticleEffect	*pEffect;

	nWall		= CGameRegion__GetWallMaterial(pThis->ah.ih.m_pCurrentRegion);
	pEffect	= pThis->m_pEffect;

	nParticleType = pEffect->m_pImpact->m_ImpactParticleType[nWall];
	nEventType    = pEffect->m_pImpact->m_ImpactEventType   [nWall];
	EventNumber   = pEffect->m_pImpact->m_ImpactEventNumber [nWall];
	nSoundType    = pEffect->m_pImpact->m_ImpactSoundType   [nWall];

	if (nParticleType != (WORD) -1)
		CParticle__SpawnCeilingCollision(pThis, nParticleType);

	if (nEventType != (WORD) -1)
		AI_Event_Dispatcher(&pThis->ah.ih, pThis->m_pSource, nEventType, AI_SPECIES_ALL, pThis->m_CI.vGroundCollisionPos, EventNumber);

	if (nSoundType != (WORD) -1)
		CParticle__DoSoundEffect(pThis, nSoundType, FALSE, &pThis->m_CI.vGroundCollisionPos);

	// ?
	if (pThis->m_CI.GroundBehavior == INTERSECT_BEHAVIOR_STICK)
		CParticle__Delete(pThis);
}

// do end of life particle
//
void CParticle__EndLife(CParticle *pThis)
{
	// declare variables
	int						nParticleType,
								nEventType,
								nSoundType,
								nImpactType;
	float						EventNumber;
	CROMParticleEffect	*pEffect;

	pEffect = pThis->m_pEffect;

	// what is particle in ?
	if (CParticle__IsInWater(pThis))
		nImpactType = PARTICLE_IMPACTS_ENDLIFEWATER;
	else
		nImpactType = PARTICLE_IMPACTS_ENDLIFE;

	nParticleType = pEffect->m_pImpact->m_ImpactParticleType[nImpactType];
	nEventType    = pEffect->m_pImpact->m_ImpactEventType   [nImpactType];
	EventNumber   = pEffect->m_pImpact->m_ImpactEventNumber [nImpactType];
	nSoundType    = pEffect->m_pImpact->m_ImpactSoundType   [nImpactType];

	if (nParticleType != (WORD) -1)
		CParticle__Spawn(pThis, nParticleType, TRUE);

	if (nEventType != (WORD) -1)
		AI_Event_Dispatcher(&pThis->ah.ih, pThis->m_pSource, nEventType, AI_SPECIES_ALL, pThis->ah.ih.m_vPos, EventNumber);

	if (nSoundType != (WORD) -1)
		CParticle__DoSoundEffect(pThis, nSoundType, FALSE, &pThis->ah.ih.m_vPos);
}


// do every frame particle
//
void CParticle__EveryFrame(CParticle *pThis)
{
	// declare variables
	int						nParticleType,
								nEventType,
								nSoundType,
								nImpactType;
	float						EventNumber;
	CROMParticleEffect	*pEffect;

	pEffect = pThis->m_pEffect;

	// what is particle in ?
	if (CParticle__IsInWater(pThis))
		nImpactType = PARTICLE_IMPACTS_EVERYFRAMEWATER;
	else
		nImpactType = PARTICLE_IMPACTS_EVERYFRAME;

	nParticleType = pEffect->m_pImpact->m_ImpactParticleType[nImpactType];
	nEventType    = pEffect->m_pImpact->m_ImpactEventType   [nImpactType];
	EventNumber   = pEffect->m_pImpact->m_ImpactEventNumber [nImpactType];
	nSoundType    = pEffect->m_pImpact->m_ImpactSoundType   [nImpactType];

	if (nParticleType != (WORD) -1)
		CParticle__Spawn(pThis, nParticleType, TRUE);

	if (nEventType != (WORD) -1)
		AI_Event_Dispatcher(&pThis->ah.ih, pThis->m_pSource, nEventType, AI_SPECIES_ALL, pThis->ah.ih.m_vPos, EventNumber);

	if (nSoundType != (WORD) -1)
		CParticle__DoSoundEffect(pThis, nSoundType, TRUE, &pThis->ah.ih.m_vPos);
}


// particle instance collision
//
void CParticle__InstanceCollision(CParticle *pThis)
{
	// declare variables
	int						nMaterial,
								nParticleType,
								nEventType,
								nSoundType;
	float						EventNumber;
	CROMParticleEffect	*pEffect;
	CInstanceHdr			*pIns;
	BOOL						Blood = TRUE;

	pIns = pThis->m_CI.pInstanceCollision;

	// get type of instance struck
	nMaterial = AI_GetMaterial(pIns);
	pEffect = pThis->m_pEffect;

	if (CParticle__IsInWater(pThis))
	{
		switch(nMaterial)
		{
			case REGMAT_FLESH:
			case REGMAT_ALIENFLESH:
				nMaterial=REGMAT_FLESHWATER;
				break;

			default:;
		}
	}

	nParticleType = pEffect->m_pImpact->m_ImpactParticleType[nMaterial];
	nEventType    = pEffect->m_pImpact->m_ImpactEventType   [nMaterial];
	EventNumber   = pEffect->m_pImpact->m_ImpactEventNumber [nMaterial];
	nSoundType    = pEffect->m_pImpact->m_ImpactSoundType   [nMaterial];

	if (CInstanceHdr__IsDevice(pIns))
		CInstanceHdr__DeviceParticleCollision(pIns, pThis);

	// get pointer to instance we are hitting (must be animated instance)
	switch (pIns->m_Type)
	{
		case I_ANIMATED:
			// Execute special-case boss hit function?
			if ((((CGameObjectInstance *)pIns)->m_pBoss) &&
				(((CGameObjectInstance *)pIns)->m_pBoss->m_pHitByParticleFunction))
				((CGameObjectInstance *)pIns)->m_pBoss->m_pHitByParticleFunction(pIns,
				((CGameObjectInstance *)pIns)->m_pBoss, pThis) ;

			// if the player
			if (		((CGameObjectInstance *)pIns == CEngineApp__GetPlayer(GetApp()))
					&&	(nParticleType == PARTICLE_TYPE_BLOOD)
					&&	(!(CCamera__InCinemaMode(&GetApp()->m_Camera))) )
			{
				nParticleType = PARTICLE_TYPE_PLAYER_IMPACT_BLOOD;
			}

			// move impact position to that of the instance it hit ?
			if (pEffect->m_dwFlags & PARTICLE_BEHAVIOR_CENTERIMPACTONINSTANCE)
			{
				pThis->ah.ih.m_vPos = ((CGameObjectInstance *)pIns)->ah.ih.m_vPos;
				pThis->ah.ih.m_pCurrentRegion = ((CGameObjectInstance *)pIns)->ah.ih.m_pCurrentRegion;
			}

			// can this ai generate blood ?
			if (AI_GetEA((CGameObjectInstance *)pIns)->m_dwTypeFlags2 & AI_TYPE2_NOBLOOD)
				Blood = FALSE;

			if (AI_GetDyn((CGameObjectInstance *)pIns)->m_Invincible)
				Blood = FALSE;

			break;
	}

	if (nParticleType != (WORD) -1)
		CParticle__Spawn(pThis, nParticleType, Blood);

	if (nEventType != (WORD) -1)
	{
		//AI_Event_Dispatcher(IsSingleEvent(nEventType) ? pIns : &pThis->ah.ih, pThis->m_pSource,
		AI_Event_Dispatcher(IsSingleEvent(nEventType) ? pIns : &pThis->ah.ih, &pThis->ah.ih,
								  nEventType, AI_SPECIES_ALL, pThis->m_CI.vInstanceCollisionPos, EventNumber);
	}

	if (nSoundType != (WORD) -1)
		CParticle__DoSoundEffect(pThis, nSoundType, FALSE, &pThis->m_CI.vInstanceCollisionPos);

	// ?
	if ((pThis->m_CI.InstanceBehavior == INTERSECT_BEHAVIOR_STICK) || IsExplosionEvent(EventNumber))
		CParticle__Delete(pThis);
}


// DON'T DELETE THE PARTICLE IN THIS FUNCTION!
void CParticle__Done(CParticle *pThis)
{
	// generate end life particles
	CParticle__EndLife(pThis);
}

// is this particle in the water ?
BOOL CParticle__IsInWater(CParticle *pThis)
{
	CGameRegion				*pRegion;
	CROMRegionSet			*pRegionSet;


	pRegion = pThis->ah.ih.m_pCurrentRegion;
	if (!pRegion)
		return FALSE;

	if (!(pRegion->m_wFlags & REGFLAG_HASWATER))
		return FALSE;

	pRegionSet = CScene__GetRegionAttributes(&GetApp()->m_Scene, pRegion);
	if (!pRegionSet)
		return FALSE;

	// is this water region actually an anti grav area ?
	if (pRegionSet->m_dwFlags & REGFLAG_ANTIGRAV)
		return FALSE;

	// under water?
	return (pThis->ah.ih.m_vPos.y < pRegionSet->m_WaterElevation);
}

// is this particle in anti-grav ?
BOOL CParticle__IsInAntiGrav(CParticle *pThis)
{
	CGameRegion				*pRegion;
	CROMRegionSet			*pRegionSet;


	pRegion = pThis->ah.ih.m_pCurrentRegion;
	if (!pRegion)
		return FALSE;

	if (!(pRegion->m_wFlags & REGFLAG_HASWATER))
		return FALSE;

	pRegionSet = CScene__GetRegionAttributes(&GetApp()->m_Scene, pRegion);
	if (!pRegionSet)
		return FALSE;

	// is this water region actually an anti grav area ?
	if (!(pRegionSet->m_dwFlags & REGFLAG_ANTIGRAV))
		return FALSE;

	// in anti grav
	return (pThis->ah.ih.m_vPos.y <= pRegionSet->m_WaterElevation);
}

#define PARTICLE_RAISE	(SCALING_FACTOR/3)
void CParticle__CalculateOrientationMatrix(CParticle *pThis,
														 float *pDistanceFromPlayer /*=NULL*/,
														 BOOL *pIntersectWithGround  /*=NULL*/)
{
#if USE_ORIGINAL_MATRIX
	CMtxF						mfScale, mfPos,
								mfTemp2;
#endif
	CMtxF						mfRotate,
								mfTemp1,
								*pmfAlign;
	CVector3					vPos, vDelta;
	CGameObjectInstance	*pPlayer;
	float						scaler, ground;

	scaler = pThis->m_Size/2;
	vPos = pThis->ah.ih.m_vPos;

	if (		(SF2FLOAT(pThis->m_pEffect->m_RotationPivotX) == 0)
			&&	(SF2FLOAT(pThis->m_pEffect->m_RotationPivotY) == 0) )
	{
		CMtxF__RotateZ(mfRotate, pThis->m_Rotation) ;
	}
	else
	{
#if USE_ORIGINAL_MATRIX
		CMtxF__Translate(mfTemp1, -SF2FLOAT(pThis->m_pEffect->m_RotationPivotX), -SF2FLOAT(pThis->m_pEffect->m_RotationPivotY), 0);
		CMtxF__RotateZ(mfTemp2, pThis->m_Rotation) ;
		CMtxF__PostMult(mfRotate, mfTemp1, mfTemp2);
#else
		// Optimized matrix version
		CMtxF__RotateZ(mfRotate, pThis->m_Rotation) ;
		CMtxF__PreMultTranslate(mfRotate, -SF2FLOAT(pThis->m_pEffect->m_RotationPivotX), -SF2FLOAT(pThis->m_pEffect->m_RotationPivotY), 0);
#endif
	}

	pmfAlign = CParticle__GetAlignmentRotation(pThis);

	if (pIntersectWithGround)
		*pIntersectWithGround = FALSE;

	switch (pThis->m_Alignment)
	{
		case PARTICLE_UP:
			if (pThis->m_pEffect->m_WallBehavior != INTERSECT_BEHAVIOR_IGNORE)
				vPos.y = max(vPos.y, CGameRegion__GetGroundHeight(pThis->ah.ih.m_pCurrentRegion, vPos.x, vPos.z) + PARTICLE_RAISE);
			break;

		case PARTICLE_GROUND:
			if (pThis->m_pEffect->m_WallBehavior != INTERSECT_BEHAVIOR_IGNORE)
			{
				ground = CGameRegion__GetGroundHeight(pThis->ah.ih.m_pCurrentRegion, vPos.x, vPos.z);
				if (pThis->m_pEffect->m_dwFlags & PARTICLE_BEHAVIOR_INTERSECTION)
				{
					if (vPos.y < (ground + 0.1*SCALING_FACTOR))
					{
						vPos.y = ground;

						if (pIntersectWithGround)
							*pIntersectWithGround = TRUE;
					}
				}
				else
				{
					vPos.y = max(vPos.y, ground + PARTICLE_RAISE);
				}
			}
			break;

		case PARTICLE_CEILING:
			if (pThis->m_pEffect->m_WallBehavior != INTERSECT_BEHAVIOR_IGNORE)
				vPos.y = min(vPos.y, CGameRegion__GetCeilingHeight(pThis->ah.ih.m_pCurrentRegion, vPos.x, vPos.z) - PARTICLE_RAISE);
			break;

		case PARTICLE_SCREENY:
			if (pThis->m_pEffect->m_dwFlags & PARTICLE_BEHAVIOR_ALIGNONBOTTOM)
				vPos.y += pThis->m_Size/2;
			break;

		case 0:
		case 1:
		case 2:
			break;

		case PARTICLE_SCREEN:
		case PARTICLE_NOEDGE:
		default:
			if (pThis->m_pEffect->m_dwFlags & PARTICLE_BEHAVIOR_ALIGNONBOTTOM)
				vPos.y += pThis->m_Size/2;
			break;
	}

	CVector3__Clamp(&vPos, WORLD_LIMIT);

#if USE_ORIGINAL_MATRIX
	CMtxF__Scale(mfScale, scaler, scaler, scaler);
	CMtxF__Translate(mfPos, vPos.x, vPos.y, vPos.z);
	CMtxF__PostMult(mfTemp1, mfScale, mfRotate);
	CMtxF__PostMult(mfTemp2, mfTemp1, *pmfAlign);
	CMtxF__PostMult(mfTemp1, mfTemp2, mfPos);
#else
	// Optimized matrix version
	CMtxF__PreMultScale(mfRotate, scaler, scaler, scaler);
	CMtxF__PostMult(mfTemp1, mfRotate, *pmfAlign);
	CMtxF__PostMultTranslate(mfTemp1, vPos.x, vPos.y, vPos.z);
#endif

	CMtxF__ToMtx(mfTemp1, CParticle__GetOrientationMatrix(pThis));

	if (pDistanceFromPlayer)
	{
		pPlayer = CEngineApp__GetPlayer(GetApp());
		if (pPlayer)
		{
			CVector3__Subtract(&vDelta, &vPos, &pPlayer->ah.ih.m_vPos);
			*pDistanceFromPlayer = CVector3__Magnitude(&vDelta);
		}
		else
		{
			*pDistanceFromPlayer = 0;
		}
	}
}

void CParticle__Draw(CParticle *pThis, Gfx **ppDLP, CCartCache *pCache)
{
	CTextureInfo			textureInfo;
	BOOL						intersectWithGround;
	float						farClip, distance, advanceAmount;
	int						cFrame,
								alpha,
								multU, multV;

	BOOL	crossfade;
	float cAnimFrame,
			fraction;
	int	cUseFrame[2], nUseFrames, i;
	int	*pUseFrame ;
	BYTE	alphaValues[2];
	DWORD	dlistRemain;


	dlistRemain = GLIST_LEN - (((DWORD)*ppDLP) - ((DWORD)CEngineApp__GetFrameData(GetApp())->m_pDisplayList))/sizeof(Gfx);
	if (dlistRemain < DISPLAY_LIST_RESERVE_FRONTEND)
		return;

	if (pThis->m_cFrame < 0)
		return;

	if (!CTextureLoader__RequestTextureSet(&pThis->m_TextureLoader, pCache, "Particle Texture Set"))
		return;

	CTextureLoader__GetTextureInfo(&pThis->m_TextureLoader, &textureInfo);

	if ((pThis->m_dwFlags & PARTICLE_HASHITGROUND) && (pThis->m_pEffect->m_dwFlags & PARTICLE_BEHAVIOR_HOLDANIMONBOUNCE))
	{
		crossfade = FALSE;
		nUseFrames = 0;
		cUseFrame[0] = pThis->m_cAnimFrame;
	}
	else
	{
		//if (pThis->m_pEffect->m_FpsVelThreshold)
		//	rmonPrintf("fps:%f\n", Fps);

		//cAnimFrame = pThis->m_cFrame*pThis->m_FPS*(1.0/FRAME_FPS);
		cAnimFrame = pThis->m_cFramePos;
		cUseFrame[0] = (int) cAnimFrame;

		advanceAmount = frame_increment*pThis->m_FPS*(1.0/FRAME_FPS)*particle_speed_scaler;
		crossfade =		((pThis->m_pEffect->m_dwFlags & PARTICLE_BEHAVIOR_CROSSFADE) || CTurokMovement.SpiritualTime)
						&& ((advanceAmount + 0.01) < frame_increment);
		if (crossfade)
		{
			nUseFrames = 2;
			cUseFrame[1] = cUseFrame[0] + 1;
			fraction = cAnimFrame - cUseFrame[0];
		}
		else
	 	{
			nUseFrames = 1;
		}
	}

	cFrame = (int) pThis->m_cFrame;

	for (i=0; i<nUseFrames; i++)
	{
		pUseFrame = &cUseFrame[i] ;

		switch (pThis->m_pEffect->m_Playback)
		{
			case PARTICLE_PLAYBACK_RUNONCE:
				if (*pUseFrame >= (textureInfo.m_nBitmaps - 1))
				{
					*pUseFrame = textureInfo.m_nBitmaps - 1;
					pThis->m_cFrame = cFrame = pThis->m_nFrames;
					crossfade = FALSE;
				}
				break;

			case PARTICLE_PLAYBACK_RUNANDHOLD:
				if (*pUseFrame >= textureInfo.m_nBitmaps)
				{
					*pUseFrame = textureInfo.m_nBitmaps - 1;
					crossfade = FALSE;
				}
				break;

			case PARTICLE_PLAYBACK_LOOP:
				*pUseFrame %= textureInfo.m_nBitmaps;
				break;

			case PARTICLE_PLAYBACK_PINGPONG:
				*pUseFrame %= max(1, (textureInfo.m_nBitmaps*2 - 2));
				if (*pUseFrame >= textureInfo.m_nBitmaps)
					*pUseFrame = (textureInfo.m_nBitmaps*2 - 2) - *pUseFrame;
				break;

			case PARTICLE_PLAYBACK_RANDOM:
				*pUseFrame = RANDOM(textureInfo.m_nBitmaps);
				crossfade = FALSE;
				break;

			case PARTICLE_PLAYBACK_RANDOMANDHOLD:
				if (pThis->m_nRandomFrame == -1)
					pThis->m_nRandomFrame = RANDOM(textureInfo.m_nBitmaps);
				*pUseFrame = pThis->m_nRandomFrame;
				crossfade = FALSE;
				break;

			default:
				ASSERT(FALSE);
		}
	}
	pThis->m_cAnimFrame = cUseFrame[0];

	if (crossfade)
		if (cUseFrame[0] == cUseFrame[1])
			crossfade = FALSE;

	CParticle__CalculateOrientationMatrix(pThis, &distance, &intersectWithGround);

	multU = (1 << ( ((pThis->m_pEffect->m_dwFlags & PARTICLE_BEHAVIOR_MIRRORWIDTH)  ? 9 : 8) + textureInfo.m_pFormat->m_WidthShift) );
	multV = (1 << ( ((pThis->m_pEffect->m_dwFlags & PARTICLE_BEHAVIOR_MIRRORHEIGHT) ? 9 : 8) + textureInfo.m_pFormat->m_HeightShift) );

	CTextureLoader__LoadTexture(&pThis->m_TextureLoader, ppDLP,
										 cUseFrame[0], 0,
										 multU, multV,
										 G_TX_WRAP | G_TX_MIRROR, G_TX_WRAP | G_TX_MIRROR,
										 G_TX_NOLOD);

	if (GetApp()->m_dwCheatFlags & CHEATFLAG_QUACKMODE)
	{
		CGeometry__SetCombineMode(ppDLP, G_CC_ROB_QUACK_PRIM__G_CC_ROB_QUACK_PRIM);
	}
	else
	{
		switch (textureInfo.m_pFormat->m_Format)
		{
			case TEXTURE_FORMAT_CI4_IA:
				CGeometry__SetCombineMode(ppDLP, G_CC_ROB_PSEUDOCOLOR_PRIMALPHA__G_CC_ROB_PSEUDOCOLOR_PRIMALPHA);
				break;

			case TEXTURE_FORMAT_CI4_RGBA:
			case TEXTURE_FORMAT_CI8_RGBA:
				CGeometry__SetCombineMode(ppDLP, G_CC_ROB_DECALRGBA_PRIMALPHA__G_CC_ROB_DECALRGBA_PRIMALPHA);
				break;

			default:
				ASSERT(FALSE);
				break;
		}
	}

 	if (intersectWithGround)
	{
		CGeometry__SetRenderMode(ppDLP, G_RM_ZB_XLU_DECAL__G_RM_ZB_XLU_DECAL2);

		gSPTextureL((*ppDLP)++,
						multU, multV,
						0,
						255,	// z compare range
						0, G_ON);
	}
	else if ((pThis->m_pEffect->m_dwFlags & PARTICLE_BEHAVIOR_INTERSECTION) && (pThis->m_pEffect->m_Alignment != PARTICLE_ALIGN_AUTO))
	{
		//if (cloud_sparkle)
		//{
		//	gDPSetAlphaCompare((*ppDLP)++, G_AC_NONE);
		//	cloud_sparkle = FALSE ;
		//}
		CGeometry__SetRenderMode(ppDLP, G_RM_ZB_XLU_DECAL__G_RM_ZB_XLU_DECAL2);
	}
	else
	{
		if (pThis->m_pEffect->m_dwFlags & PARTICLE_BEHAVIOR_SPARKLE)
		{
			// cloud sparkle
			//cloud_sparkle = TRUE ;
			CGeometry__SetRenderMode(ppDLP, RM_ROB_ZB_PCL_SURF_BLEND1__RM_ROB_ZB_PCL_SURF_BLEND2);
		}
		else
		{
			if (pThis->m_pEffect->m_dwFlags & PARTICLE_BEHAVIOR_ZBUFFER)
			{
				//if (cloud_sparkle)
				//{
				//	gDPSetAlphaCompare((*ppDLP)++, G_AC_NONE);
				//	cloud_sparkle = FALSE ;
				//}

				if (GetApp()->m_dwCheatFlags & CHEATFLAG_PENANDINKMODE)
					CGeometry__SetRenderMode(ppDLP, G_RM_AA_ZB_TEX_EDGE__G_RM_AA_ZB_TEX_EDGE2);

				else
					CGeometry__SetRenderMode(ppDLP, G_RM_ZB_CLD_SURF__G_RM_ZB_CLD_SURF2);
			}
			else
			{
				//if (cloud_sparkle)
				//{
				// 	gDPSetAlphaCompare((*ppDLP)++, G_AC_NONE);
				//	cloud_sparkle = FALSE ;
				//}

				if (GetApp()->m_dwCheatFlags & CHEATFLAG_PENANDINKMODE)
					CGeometry__SetRenderMode(ppDLP, G_RM_AA_TEX_EDGE__G_RM_AA_TEX_EDGE2);

				else
					CGeometry__SetRenderMode(ppDLP, G_RM_CLD_SURF__G_RM_CLD_SURF2);
			}
		}
	}

	// add distance to alpha value
	if ((pThis->m_pEffect->m_dwFlags & PARTICLE_BEHAVIOR_FADEOUT) && (pThis->m_nFrames > 0))
		alpha = (pThis->m_nFrames - cFrame)*255/pThis->m_nFrames;
	else
		alpha = 255;

	if (cFrame < pThis->m_pEffect->m_nFadeInDelay)
		alpha = (alpha*(cFrame + 1))/(pThis->m_pEffect->m_nFadeInDelay + 1);

	if (max(0, (pThis->m_nFrames - cFrame)) < pThis->m_pEffect->m_nFadeOutDelay)
		alpha = max(0, (alpha*(pThis->m_nFrames - cFrame)))/(pThis->m_pEffect->m_nFadeOutDelay + 1);

	farClip = GetApp()->m_FarClip;

	switch (pThis->m_pEffect->m_Visibility)
	{
		case PARTICLE_VISIBLE_ALWAYS:
			alpha = alpha*min(1, (farClip - distance)*10/farClip);
			break;

		case PARTICLE_VISIBLE_CLOSE:
			alpha = alpha*min(1, (farClip - distance)*5/farClip);
			break;

		case PARTICLE_VISIBLE_FAR:
			alpha = alpha*(farClip/2 - fabs(farClip/2 - distance))/(farClip/2);
			break;
	}

	if (textureInfo.m_pFormat->m_Format == TEXTURE_FORMAT_CI4_IA)
	{
		gDPSetEnvColor((*ppDLP)++,
							 pThis->m_BlackColor[0],
							 pThis->m_BlackColor[1],
							 pThis->m_BlackColor[2],
							 255);
	}

	if (crossfade)
	{
/*
		totalSprite = alpha/255.0;


		fTemp = totalSprite*fraction;
		denom = 1 - fTemp;
		if (denom == 0)
		{
			if (fraction == 0)
				alphaValues[0] = 255;
			else
				alphaValues[0] = 0;
		}
		else
		{
			alphaValues[0] = max(0, min(255, 255*(totalSprite - fTemp)/denom));
		}

		alphaValues[1] = max(0, min(255, 255*fTemp));
*/
		//alphaValues[0] = max(0, min(255, 255*cos(fraction*(ANGLE_PI/2))));
		//alphaValues[1] = max(0, min(255, 255*cos((1-fraction)*(ANGLE_PI/2))));

		alphaValues[0] = max(0, min(255, alpha*(1-fraction)));
		alphaValues[1] = max(0, min(255, alpha*fraction));
	}
	else
	{
		alphaValues[0] = max(0, min(255, alpha));
		nUseFrames = 1;
	}

  	gSPMatrix((*ppDLP)++,
				 RSP_ADDRESS(&CParticle__GetOrientationMatrix(pThis)),
	   		 G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);

	gSPVertex((*ppDLP)++,
				 RSP_ADDRESS(particle_vtxs),
				 4, 0);

	for (i=0; i<nUseFrames; i++)
	{
		if (i)
		{
			CTextureLoader__LoadTexture(&pThis->m_TextureLoader, ppDLP,
												 cUseFrame[i], 0,
												 multU, multV,
												 G_TX_WRAP | G_TX_MIRROR, G_TX_WRAP | G_TX_MIRROR,
												 G_TX_NOLOD);

			if (intersectWithGround)
			{
				gSPTextureL((*ppDLP)++,
								multU, multV,
								0,
								255,	// z compare range
								0, G_ON);
			}
		}

		gDPSetPrimColor((*ppDLP)++,
							 256, 256,				// LOD stuff
							 pThis->m_WhiteColor[0],
							 pThis->m_WhiteColor[1],
							 pThis->m_WhiteColor[2],
							 (alphaValues[i]*pThis->m_AlphaScaler) >> 8);

		gSP2Triangles((*ppDLP)++,
						  0, 1, 2, 0,
						  2, 3, 0, 0);
	}

	// add swoosh edge if a swoosh is active
	if (pThis->m_ActiveSwooshes)
		CFxSystem__CheckForAddingParticleSwooshEdge(&GetApp()->m_FxSystem, pThis, &CParticle__GetOrientationMatrix(pThis)) ;

/*
	CMtxF__VectorMult4x4(GetApp()->m_mfProjection, &pThis->ah.ih.m_vPos, &vProj);


	vProj.x = (vProj.x * (SCREEN_WD/2)) + (SCREEN_WD/2);
	vProj.y = (vProj.y * -(SCREEN_HT/2)) + (SCREEN_HT/2);


	gDPPipeSync((*ppDLP)++);

	gDPSetRenderMode((*ppDLP)++,
							G_RM_CLD_SURF,
							G_RM_CLD_SURF2);

	gDPSetCombineMode((*ppDLP)++,
							G_CC_PRIMITIVE,
							G_CC_PRIMITIVE);

	gDPSetPrimColor((*ppDLP)++,
						 256, 256,				// LOD stuff
						 255, 0, 0,
						 128);

	gDPScisFillRectangle((*ppDLP)++,
								(int) vProj.x, (int) vProj.y,
								(int) (vProj.x + 10), (int) (vProj.y + 10));


	CGeometry__ResetDrawModes();
*/
}


