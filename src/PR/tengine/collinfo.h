// collinfo.h

#ifndef _INC_COLLINFO
#define _INC_COLLINFO

#include "unicol.h"

/////////////////////////////////////////////////////////////////////////////

// forward declarations
struct CInstanceHdr_t;
struct CGameRegion_t;

/////////////////////////////////////////////////////////////////////////////

// remove some old flags
//#define COLLISIONFLAG_BOUNCEOFFGROUND				(1 << 0)
//#define COLLISIONFLAG_CANCLIMB						(1 << 1)
//#define COLLISIONFLAG_UPDATEVELOCITY				(1 << 2)
#define COLLISIONFLAG_IGNOREDEAD						(1 << 3)
#define COLLISIONFLAG_USEWALLRADIUS					(1 << 4)
#define COLLISIONFLAG_SMALLWALLRADIUS				(1 << 5)
//#define COLLISIONFLAG_PREVENTOSC						(1 << 6)
//#define COLLISIONFLAG_NOHILLSLOWDOWN				(1 << 7)
#define COLLISIONFLAG_WATERCOLLISION				(1 << 8)
//#define COLLISIONFLAG_NOGROUNDMOVEMENT				(1 << 9)
#define COLLISIONFLAG_MOVETHROUGHDOORS				(1 << 10)
#define COLLISIONFLAG_PICKUPCOLLISION				(1 << 11)
#define COLLISIONFLAG_CANCLIMBCLIMBABLE			(1 << 12)
#define COLLISIONFLAG_CLIFFONLY						(1 << 13)
//#define COLLISIONFLAG_STICKTOGROUND					(1 << 14)
#define COLLISIONFLAG_DUCKING							(1 << 15)
#define COLLISIONFLAG_CANENTERRESTRICTEDAREA		(1 << 16)
//#define COLLISIONFLAG_NOFLOORCOLLISION				(1 << 17)
#define COLLISIONFLAG_AVOIDPRESSUREPLATE			(1 << 18)

// new flags
#define COLLISIONFLAG_PHYSICS							(1 << 19)
#define COLLISIONFLAG_TRACKGROUND					(1 << 20)
#define COLLISIONFLAG_CLIMBUP							(1 << 21)
#define COLLISIONFLAG_CLIMBDOWN						(1 << 22)
#define COLLISIONFLAG_ENTERWATER						(1 << 23)
#define COLLISIONFLAG_EXITWATER						(1 << 24)
#define COLLISIONFLAG_INSTANCEUPDOWN				(1 << 25)
#define COLLISIONFLAG_TWOREDIRECTIONS				(1 << 26)
#define COLLISIONFLAG_USEHEIGHT						(1 << 27)
#define COLLISIONFLAG_ENDONBOUNCE					(1 << 28)

#define GROUNDCOLLISION_NONE		0
#define GROUNDCOLLISION_GROUND	1
#define GROUNDCOLLISION_CEILING	2


typedef struct CCollisionInfo2_t
{
	// input
	DWORD							dwFlags;
	int							InstanceBehavior,
									WallBehavior,
									GroundBehavior;
	float							GravityAcceleration,
									BounceReturnEnergy,
									GroundFriction,
									AirFriction,
									WaterFriction;

	// output
	struct CInstanceHdr_t	*pInstanceCollision;			// NULL if no collision
	struct CGameRegion_t		*pInstanceCollisionRegion;
	CVector3						vInstanceCollisionPos;

	struct CGameRegion_t		*pWallCollisionRegion;		// NULL if no collision
	CVector3						vWallCollisionPos;
	int							nWallCollisionEdge;

	int							nGroundCollisionType;		// GROUNDCOLLISION_NONE
	struct CGameRegion_t		*pGroundCollisionRegion;	//   if no collision
	CVector3						vGroundCollisionPos;

	struct CGameRegion_t		*pWaterCollisionRegion;		// NULL if no collision
	CVector3						vWaterCollisionPos;

	CVector3						vCollisionVelocity;			// velocity of last
																		//   collision
} CCollisionInfo2;

// CCollisionInfo operations
/////////////////////////////////////////////////////////////////////////////

void		CCollisionInfo__Reset();
void		CCollisionInfo__ClearCollision(CCollisionInfo2 *pThis);

// global collision structures
/////////////////////////////////////////////////////////////////////////////

extern CCollisionInfo2	ci_player;
extern CCollisionInfo2	ci_playerdead;
extern CCollisionInfo2	ci_playerunderwater;
extern CCollisionInfo2	ci_playerantigrav;
extern CCollisionInfo2	ci_playeronwatersurface;
extern CCollisionInfo2	ci_playerfallintowater;
extern CCollisionInfo2	ci_player_climbing_sidestep_character;
extern CCollisionInfo2	ci_character;
extern CCollisionInfo2	ci_character_walkinwater;
extern CCollisionInfo2	ci_dead_character;
extern CCollisionInfo2	ci_flying_character;
extern CCollisionInfo2	ci_climbing_character;
extern CCollisionInfo2	ci_climbing_swimming_character;
extern CCollisionInfo2	ci_underwater_climbing_character;
extern CCollisionInfo2	ci_underwater_character;
//extern CCollisionInfo2	ci_underwater_walk_character;
extern CCollisionInfo2	ci_underwater_character_dead;
extern CCollisionInfo2	ci_underwater_character_float_dead;
extern CCollisionInfo2	ci_particle;
extern CCollisionInfo2	ci_pickup;
extern CCollisionInfo2	ci_nocollisionatall;
extern CCollisionInfo2	ci_movetest;
extern CCollisionInfo2	ci_swimming_movetest;
extern CCollisionInfo2	ci_climbing_movetest;
extern CCollisionInfo2	ci_climbing_swimming_movetest;
extern CCollisionInfo2	ci_raycast;
extern CCollisionInfo2	ci_nearregiontest;
extern CCollisionInfo2	ci_clifftest;

///////////////////////////////////////////////////////////////////////////
#endif // _INC_COLLINFO
