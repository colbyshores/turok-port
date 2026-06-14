// collinfo.cpp
//

#include "stdafx.h"

#include "tengine.h"
#include "collinfo.h"
#include "scaling.h"

// globals
/////////////////////////////////////////////////////////////////////////////

CCollisionInfo2	ci_player;
CCollisionInfo2	ci_playerdead;
CCollisionInfo2	ci_playerunderwater;
CCollisionInfo2	ci_playerantigrav;
CCollisionInfo2	ci_playeronwatersurface;
CCollisionInfo2	ci_playerfallintowater;
CCollisionInfo2	ci_player_climbing_sidestep_character;
CCollisionInfo2	ci_character;
CCollisionInfo2	ci_character_walkinwater;
CCollisionInfo2	ci_dead_character;
CCollisionInfo2	ci_flying_character;
CCollisionInfo2	ci_climbing_character;
CCollisionInfo2	ci_climbing_swimming_character;
CCollisionInfo2	ci_underwater_climbing_character;
CCollisionInfo2	ci_underwater_character;
//CCollisionInfo2	ci_underwater_walk_character;
CCollisionInfo2	ci_underwater_character_dead;
CCollisionInfo2	ci_underwater_character_float_dead;
CCollisionInfo2	ci_particle;
CCollisionInfo2	ci_pickup;
CCollisionInfo2	ci_nocollisionatall;
CCollisionInfo2	ci_movetest;
CCollisionInfo2	ci_swimming_movetest;
CCollisionInfo2	ci_climbing_movetest;
CCollisionInfo2	ci_climbing_swimming_movetest;
CCollisionInfo2	ci_raycast;
CCollisionInfo2	ci_nearregiontest;
CCollisionInfo2	ci_clifftest;

// CCollisionInfo implementation
/////////////////////////////////////////////////////////////////////////////

void		CCollisionInfo__SetPlayerDefaults(CCollisionInfo2 *pThis);
void		CCollisionInfo__SetPlayerDeadDefaults(CCollisionInfo2 *pThis);
void		CCollisionInfo__SetPlayerUnderwaterDefaults(CCollisionInfo2 *pThis);
void		CCollisionInfo__SetPlayerAntiGravDefaults(CCollisionInfo2 *pThis);
void		CCollisionInfo__SetPlayerOnWaterSurfaceDefaults(CCollisionInfo2 *pThis);
void		CCollisionInfo__SetPlayerFallIntoWaterDefaults(CCollisionInfo2 *pThis);
void		CCollisionInfo__SetPlayerClimbingSideStepCharacterDefaults(CCollisionInfo2 *pThis);
void		CCollisionInfo__SetCharacterDefaults(CCollisionInfo2 *pThis);
void		CCollisionInfo__SetCharacterWalkInWaterDefaults(CCollisionInfo2 *pThis);
void		CCollisionInfo__SetDeadCharacterDefaults(CCollisionInfo2 *pThis);
void		CCollisionInfo__SetFlyingCharacterDefaults(CCollisionInfo2 *pThis);
void		CCollisionInfo__SetClimbingCharacterDefaults(CCollisionInfo2 *pThis);
void		CCollisionInfo__SetClimbingAndSwimmingCharacterDefaults(CCollisionInfo2 *pThis);
void		CCollisionInfo__SetUnderwaterClimbingCharacterDefaults(CCollisionInfo2 *pThis);
void		CCollisionInfo__SetUnderwaterCharacterDefaults(CCollisionInfo2 *pThis);
void		CCollisionInfo__SetUnderwaterCharacterDeadDefaults(CCollisionInfo2 *pThis);
//void		CCollisionInfo__SetUnderwaterWalkCharacterDefaults(CCollisionInfo2 *pThis);
void		CCollisionInfo__SetUnderwaterCharacterFloatDeadDefaults(CCollisionInfo2 *pThis);
void		CCollisionInfo__SetMovementTestDefaults(CCollisionInfo2 *pThis);
void		CCollisionInfo__SetSwimmerMovementTestDefaults(CCollisionInfo2 *pThis);
void		CCollisionInfo__SetClimberMovementTestDefaults(CCollisionInfo2 *pThis);
void		CCollisionInfo__SetClimberAndSwimmerMovementTestDefaults(CCollisionInfo2 *pThis);
void		CCollisionInfo__SetParticleDefaults(CCollisionInfo2 *pThis);
void		CCollisionInfo__SetPickupDefaults(CCollisionInfo2 *pThis);
void		CCollisionInfo__SetNoCollisionAtAllDefaults(CCollisionInfo2 *pThis);
void		CCollisionInfo__SetRayCastDefaults(CCollisionInfo2 *pThis);
void		CCollisionInfo__SetNearRegionTestDefaults(CCollisionInfo2 *pThis);
void		CCollisionInfo__SetCliffTestDefaults(CCollisionInfo2 *pThis);

// CCollisionInfo
/////////////////////////////////////////////////////////////////////////////

void CCollisionInfo__ClearCollision(CCollisionInfo2 *pThis)
{
	pThis->pInstanceCollision = NULL;
	pThis->pWallCollisionRegion = NULL;
	pThis->nGroundCollisionType = GROUNDCOLLISION_NONE;
	pThis->pWaterCollisionRegion = NULL;
}

void CCollisionInfo__Reset()
{
	CCollisionInfo__SetPlayerDefaults(&ci_player);
	CCollisionInfo__ClearCollision(&ci_player);

	CCollisionInfo__SetPlayerDeadDefaults(&ci_playerdead);
	CCollisionInfo__ClearCollision(&ci_playerdead);

	CCollisionInfo__SetPlayerUnderwaterDefaults(&ci_playerunderwater);
	CCollisionInfo__ClearCollision(&ci_playerunderwater);

	CCollisionInfo__SetPlayerAntiGravDefaults(&ci_playerantigrav);
	CCollisionInfo__ClearCollision(&ci_playerantigrav);

	CCollisionInfo__SetPlayerOnWaterSurfaceDefaults(&ci_playeronwatersurface);
	CCollisionInfo__ClearCollision(&ci_playeronwatersurface);

	CCollisionInfo__SetPlayerFallIntoWaterDefaults(&ci_playerfallintowater);
	CCollisionInfo__ClearCollision(&ci_playerfallintowater);

	CCollisionInfo__SetPlayerClimbingSideStepCharacterDefaults(&ci_player_climbing_sidestep_character);
	CCollisionInfo__ClearCollision(&ci_player_climbing_sidestep_character);

	CCollisionInfo__SetCharacterDefaults(&ci_character);
	CCollisionInfo__ClearCollision(&ci_character);

	CCollisionInfo__SetCharacterWalkInWaterDefaults(&ci_character_walkinwater);
	CCollisionInfo__ClearCollision(&ci_character_walkinwater);

	CCollisionInfo__SetDeadCharacterDefaults(&ci_dead_character);
	CCollisionInfo__ClearCollision(&ci_dead_character);

	CCollisionInfo__SetFlyingCharacterDefaults(&ci_flying_character);
	CCollisionInfo__ClearCollision(&ci_flying_character);

	CCollisionInfo__SetClimbingCharacterDefaults(&ci_climbing_character);
	CCollisionInfo__ClearCollision(&ci_climbing_character);

	CCollisionInfo__SetClimbingAndSwimmingCharacterDefaults(&ci_climbing_swimming_character);
	CCollisionInfo__ClearCollision(&ci_climbing_swimming_character);

	CCollisionInfo__SetUnderwaterClimbingCharacterDefaults(&ci_underwater_climbing_character);
	CCollisionInfo__ClearCollision(&ci_underwater_climbing_character);

	CCollisionInfo__SetUnderwaterCharacterDefaults(&ci_underwater_character);
	CCollisionInfo__ClearCollision(&ci_underwater_character);

//	CCollisionInfo__SetUnderwaterWalkCharacterDefaults(&ci_underwater_walk_character);
//	CCollisionInfo__ClearCollision(&ci_underwater_walk_character);

	CCollisionInfo__SetUnderwaterCharacterDeadDefaults(&ci_underwater_character_dead);
	CCollisionInfo__ClearCollision(&ci_underwater_character_dead);

	CCollisionInfo__SetUnderwaterCharacterFloatDeadDefaults(&ci_underwater_character_float_dead);
	CCollisionInfo__ClearCollision(&ci_underwater_character_float_dead);

	CCollisionInfo__SetMovementTestDefaults(&ci_movetest);
	CCollisionInfo__ClearCollision(&ci_movetest);

	CCollisionInfo__SetSwimmerMovementTestDefaults(&ci_swimming_movetest);
	CCollisionInfo__ClearCollision(&ci_swimming_movetest);

	CCollisionInfo__SetClimberMovementTestDefaults(&ci_climbing_movetest);
	CCollisionInfo__ClearCollision(&ci_climbing_movetest);

	CCollisionInfo__SetClimberAndSwimmerMovementTestDefaults(&ci_climbing_swimming_movetest);
	CCollisionInfo__ClearCollision(&ci_climbing_swimming_movetest);

	CCollisionInfo__SetParticleDefaults(&ci_particle);
	CCollisionInfo__ClearCollision(&ci_particle);

	CCollisionInfo__SetPickupDefaults(&ci_pickup);
	CCollisionInfo__ClearCollision(&ci_pickup);

	CCollisionInfo__SetNoCollisionAtAllDefaults(&ci_nocollisionatall);
	CCollisionInfo__ClearCollision(&ci_nocollisionatall);

	CCollisionInfo__SetRayCastDefaults(&ci_raycast);
	CCollisionInfo__ClearCollision(&ci_raycast);

	CCollisionInfo__SetNearRegionTestDefaults(&ci_nearregiontest);
	CCollisionInfo__ClearCollision(&ci_nearregiontest);

	CCollisionInfo__SetCliffTestDefaults(&ci_clifftest);
	CCollisionInfo__ClearCollision(&ci_clifftest);
}

void CCollisionInfo__SetPlayerDefaults(CCollisionInfo2 *pThis)
{
	pThis->dwFlags =		COLLISIONFLAG_CANCLIMBCLIMBABLE
							|	COLLISIONFLAG_IGNOREDEAD
							|	COLLISIONFLAG_WATERCOLLISION
							|	COLLISIONFLAG_PICKUPCOLLISION
							|	COLLISIONFLAG_CANENTERRESTRICTEDAREA
							|	COLLISIONFLAG_PHYSICS
							|	COLLISIONFLAG_CLIMBDOWN
							|	COLLISIONFLAG_ENTERWATER
							|	COLLISIONFLAG_EXITWATER
							|	COLLISIONFLAG_INSTANCEUPDOWN
							|	COLLISIONFLAG_TRACKGROUND
							|	COLLISIONFLAG_USEHEIGHT;

	pThis->InstanceBehavior			= INTERSECT_BEHAVIOR_GREASE;
	pThis->WallBehavior				= INTERSECT_BEHAVIOR_SLIDE;
	pThis->GroundBehavior			= INTERSECT_BEHAVIOR_SLIDE;
	pThis->GravityAcceleration		= GRAVITY_ACCELERATION;
	pThis->BounceReturnEnergy		= 0.0;
	pThis->GroundFriction			= 0.7;
	pThis->AirFriction				= 0.0;
	pThis->WaterFriction				= 0.0;
}

void CCollisionInfo__SetPlayerDeadDefaults(CCollisionInfo2 *pThis)
{
	pThis->dwFlags =		COLLISIONFLAG_IGNOREDEAD
							|	COLLISIONFLAG_WATERCOLLISION
							|	COLLISIONFLAG_PICKUPCOLLISION
							|	COLLISIONFLAG_CANENTERRESTRICTEDAREA
							|	COLLISIONFLAG_PHYSICS
							|	COLLISIONFLAG_CLIMBDOWN
							|	COLLISIONFLAG_ENTERWATER
							|	COLLISIONFLAG_EXITWATER
							|	COLLISIONFLAG_INSTANCEUPDOWN
							|	COLLISIONFLAG_TRACKGROUND
							|	COLLISIONFLAG_USEHEIGHT;

	pThis->InstanceBehavior			= INTERSECT_BEHAVIOR_GREASE;
	pThis->WallBehavior				= INTERSECT_BEHAVIOR_SLIDE;
	pThis->GroundBehavior			= INTERSECT_BEHAVIOR_SLIDE;
	pThis->GravityAcceleration		= GRAVITY_ACCELERATION;
	pThis->BounceReturnEnergy		= 0.0;
	pThis->GroundFriction			= 0.7;
	pThis->AirFriction				= 0.0;
	pThis->WaterFriction				= 0.0;
}

void CCollisionInfo__SetPlayerUnderwaterDefaults(CCollisionInfo2 *pThis)
{
	pThis->dwFlags =		COLLISIONFLAG_IGNOREDEAD
							|	COLLISIONFLAG_WATERCOLLISION
							|	COLLISIONFLAG_PICKUPCOLLISION
							|	COLLISIONFLAG_CANENTERRESTRICTEDAREA
							|	COLLISIONFLAG_PHYSICS
							|	COLLISIONFLAG_CLIMBDOWN
							|	COLLISIONFLAG_ENTERWATER
							|	COLLISIONFLAG_EXITWATER
							|	COLLISIONFLAG_INSTANCEUPDOWN
							|	COLLISIONFLAG_TWOREDIRECTIONS
							|	COLLISIONFLAG_TRACKGROUND
							|	COLLISIONFLAG_USEHEIGHT;

	pThis->InstanceBehavior			= INTERSECT_BEHAVIOR_GREASE;
	pThis->WallBehavior				= INTERSECT_BEHAVIOR_SLIDE;
	pThis->GroundBehavior			= INTERSECT_BEHAVIOR_SLIDE;
	pThis->GravityAcceleration		= GRAVITY_WATER_ACCELERATION;
	pThis->BounceReturnEnergy		= 0.0;
	pThis->GroundFriction			= 0.0;
	pThis->AirFriction				= 0.0;
	pThis->WaterFriction				= 0.08;
}

void CCollisionInfo__SetPlayerAntiGravDefaults(CCollisionInfo2 *pThis)
{
	pThis->dwFlags =		COLLISIONFLAG_CANCLIMBCLIMBABLE
							|	COLLISIONFLAG_IGNOREDEAD
							|	COLLISIONFLAG_WATERCOLLISION
							|	COLLISIONFLAG_PICKUPCOLLISION
							|	COLLISIONFLAG_CANENTERRESTRICTEDAREA
							|	COLLISIONFLAG_PHYSICS
							|	COLLISIONFLAG_CLIMBDOWN
							|	COLLISIONFLAG_ENTERWATER
							|	COLLISIONFLAG_EXITWATER
							|	COLLISIONFLAG_INSTANCEUPDOWN
							|	COLLISIONFLAG_USEHEIGHT;

	pThis->InstanceBehavior			= INTERSECT_BEHAVIOR_GREASE;
	pThis->WallBehavior				= INTERSECT_BEHAVIOR_SLIDE;
	pThis->GroundBehavior			= INTERSECT_BEHAVIOR_SLIDE;
	pThis->GravityAcceleration		= 0.0;
	pThis->BounceReturnEnergy		= 0.0;
	pThis->GroundFriction			= 0.9;
	pThis->AirFriction				= 0.0;
	pThis->WaterFriction				= 0.0;
}

void CCollisionInfo__SetPlayerOnWaterSurfaceDefaults(CCollisionInfo2 *pThis)
{
	pThis->dwFlags =		COLLISIONFLAG_CANCLIMBCLIMBABLE
							|	COLLISIONFLAG_IGNOREDEAD
							|	COLLISIONFLAG_WATERCOLLISION
							|	COLLISIONFLAG_PICKUPCOLLISION
							|	COLLISIONFLAG_CANENTERRESTRICTEDAREA
							|	COLLISIONFLAG_PHYSICS
							|	COLLISIONFLAG_CLIMBDOWN
							|	COLLISIONFLAG_ENTERWATER
							|	COLLISIONFLAG_EXITWATER
							|	COLLISIONFLAG_INSTANCEUPDOWN
							|	COLLISIONFLAG_TRACKGROUND
							|	COLLISIONFLAG_USEHEIGHT;

	pThis->InstanceBehavior			= INTERSECT_BEHAVIOR_GREASE;
	pThis->WallBehavior				= INTERSECT_BEHAVIOR_SLIDE;
	pThis->GroundBehavior			= INTERSECT_BEHAVIOR_SLIDE;
	pThis->GravityAcceleration		= 0.0;
	pThis->BounceReturnEnergy		= 0.0;
	pThis->GroundFriction			= 0.0;
	pThis->AirFriction				= 0.0;
	pThis->WaterFriction				= 0.0;
}

void CCollisionInfo__SetPlayerFallIntoWaterDefaults(CCollisionInfo2 *pThis)
{
	pThis->dwFlags =		COLLISIONFLAG_CANCLIMBCLIMBABLE
							|	COLLISIONFLAG_IGNOREDEAD
							|	COLLISIONFLAG_WATERCOLLISION
							|	COLLISIONFLAG_PICKUPCOLLISION
							|	COLLISIONFLAG_CANENTERRESTRICTEDAREA
							|	COLLISIONFLAG_PHYSICS
							|	COLLISIONFLAG_CLIMBDOWN
							|	COLLISIONFLAG_ENTERWATER
							|	COLLISIONFLAG_EXITWATER
							|	COLLISIONFLAG_INSTANCEUPDOWN
							|	COLLISIONFLAG_TRACKGROUND
							|	COLLISIONFLAG_USEHEIGHT;

	pThis->InstanceBehavior			= INTERSECT_BEHAVIOR_GREASE;
	pThis->WallBehavior				= INTERSECT_BEHAVIOR_SLIDE;
	pThis->GroundBehavior			= INTERSECT_BEHAVIOR_SLIDE;
	pThis->GravityAcceleration		= GRAVITY_ACCELERATION;
	pThis->BounceReturnEnergy		= 0.0;
	pThis->GroundFriction			= 0.7;
	pThis->AirFriction				= 0.0;
	pThis->WaterFriction				= 0.0;
}

void CCollisionInfo__SetPlayerClimbingSideStepCharacterDefaults(CCollisionInfo2 *pThis)
{
	pThis->dwFlags =		COLLISIONFLAG_CANCLIMBCLIMBABLE
							|	COLLISIONFLAG_IGNOREDEAD
							|	COLLISIONFLAG_WATERCOLLISION
							|	COLLISIONFLAG_PICKUPCOLLISION
							|	COLLISIONFLAG_CANENTERRESTRICTEDAREA
							|	COLLISIONFLAG_PHYSICS
							|	COLLISIONFLAG_CLIMBDOWN
							|	COLLISIONFLAG_ENTERWATER
							|	COLLISIONFLAG_EXITWATER
							|	COLLISIONFLAG_INSTANCEUPDOWN
							|	COLLISIONFLAG_TRACKGROUND
							|	COLLISIONFLAG_USEHEIGHT;
							//|	COLLISIONFLAG_TWOREDIRECTIONS;

	pThis->InstanceBehavior			= INTERSECT_BEHAVIOR_GREASE;
//	pThis->WallBehavior				= INTERSECT_BEHAVIOR_STICK;
	pThis->WallBehavior				= INTERSECT_BEHAVIOR_SLIDE;
	pThis->GroundBehavior			= INTERSECT_BEHAVIOR_STICK;
	pThis->GravityAcceleration		= 0.0;
	pThis->BounceReturnEnergy		= 0.0;
	pThis->GroundFriction			= 0.0;
	pThis->AirFriction				= 0.0;
	pThis->WaterFriction				= 0.0;
}

void CCollisionInfo__SetCharacterDefaults(CCollisionInfo2 *pThis)
{
	pThis->dwFlags =		COLLISIONFLAG_PHYSICS
							|	COLLISIONFLAG_EXITWATER
							|	COLLISIONFLAG_USEWALLRADIUS
							|	COLLISIONFLAG_TRACKGROUND
							|	COLLISIONFLAG_WATERCOLLISION
							|	COLLISIONFLAG_IGNOREDEAD
							|	COLLISIONFLAG_USEHEIGHT;

	pThis->InstanceBehavior			= INTERSECT_BEHAVIOR_GREASE;
	pThis->WallBehavior				= INTERSECT_BEHAVIOR_SLIDEBOUNCE;
	pThis->GroundBehavior			= INTERSECT_BEHAVIOR_SLIDE;
	pThis->GravityAcceleration		= GRAVITY_ACCELERATION;
	pThis->BounceReturnEnergy		= 0.3;
	pThis->GroundFriction			= 0.7;
	pThis->AirFriction				= 0.0;
	pThis->WaterFriction				= 0.0;
}

void CCollisionInfo__SetCharacterWalkInWaterDefaults(CCollisionInfo2 *pThis)
{
	pThis->dwFlags =		COLLISIONFLAG_PHYSICS
							|	COLLISIONFLAG_ENTERWATER
							|	COLLISIONFLAG_EXITWATER
							|	COLLISIONFLAG_USEWALLRADIUS
							|	COLLISIONFLAG_TRACKGROUND
							|	COLLISIONFLAG_IGNOREDEAD
							|	COLLISIONFLAG_USEHEIGHT;

	pThis->InstanceBehavior			= INTERSECT_BEHAVIOR_GREASE;
	pThis->WallBehavior				= INTERSECT_BEHAVIOR_SLIDEBOUNCE;
	pThis->GroundBehavior			= INTERSECT_BEHAVIOR_SLIDE;
	pThis->GravityAcceleration		= GRAVITY_ACCELERATION;
	pThis->BounceReturnEnergy		= 0.3;
	pThis->GroundFriction			= 0.7;
	pThis->AirFriction				= 0.0;
	pThis->WaterFriction				= 0.0;
}

void CCollisionInfo__SetDeadCharacterDefaults(CCollisionInfo2 *pThis)
{
	pThis->dwFlags =		COLLISIONFLAG_PHYSICS
							|	COLLISIONFLAG_CLIMBDOWN
							|	COLLISIONFLAG_ENTERWATER
							|	COLLISIONFLAG_EXITWATER
							|	COLLISIONFLAG_USEWALLRADIUS
							|	COLLISIONFLAG_TRACKGROUND
							|	COLLISIONFLAG_WATERCOLLISION
							|	COLLISIONFLAG_USEHEIGHT;

	pThis->InstanceBehavior			= INTERSECT_BEHAVIOR_GREASE;
	pThis->WallBehavior				= INTERSECT_BEHAVIOR_BOUNCE;
	pThis->GroundBehavior			= INTERSECT_BEHAVIOR_SLIDE;
	pThis->GravityAcceleration		= GRAVITY_ACCELERATION;
	pThis->BounceReturnEnergy		= 0.3;
	pThis->GroundFriction			= 0.7;
	pThis->AirFriction				= 0.0;
	pThis->WaterFriction				= 0.0;
}

void CCollisionInfo__SetFlyingCharacterDefaults(CCollisionInfo2 *pThis)
{
	pThis->dwFlags =		COLLISIONFLAG_PHYSICS
							|	COLLISIONFLAG_CLIMBDOWN
							|	COLLISIONFLAG_ENTERWATER
							|	COLLISIONFLAG_EXITWATER
							|	COLLISIONFLAG_USEWALLRADIUS
							|	COLLISIONFLAG_TRACKGROUND
							|	COLLISIONFLAG_WATERCOLLISION
							|	COLLISIONFLAG_USEHEIGHT;

	pThis->InstanceBehavior			= INTERSECT_BEHAVIOR_GREASE;
	pThis->WallBehavior				= INTERSECT_BEHAVIOR_SLIDE;
	pThis->GroundBehavior			= INTERSECT_BEHAVIOR_SLIDE;
	pThis->GravityAcceleration		= 0.0;
	pThis->BounceReturnEnergy		= 0.3;
	pThis->GroundFriction			= 0.7;
	pThis->AirFriction				= 0.0;
	pThis->WaterFriction				= 0.0;
}

void CCollisionInfo__SetClimbingCharacterDefaults(CCollisionInfo2 *pThis)
{
	pThis->dwFlags =		COLLISIONFLAG_PHYSICS
							|	COLLISIONFLAG_CLIMBUP
							|	COLLISIONFLAG_CLIMBDOWN
							|	COLLISIONFLAG_EXITWATER
							|	COLLISIONFLAG_USEWALLRADIUS
							|	COLLISIONFLAG_TRACKGROUND
							|	COLLISIONFLAG_WATERCOLLISION
							|	COLLISIONFLAG_IGNOREDEAD
							|	COLLISIONFLAG_USEHEIGHT;

	pThis->InstanceBehavior			= INTERSECT_BEHAVIOR_GREASE;
	pThis->WallBehavior				= INTERSECT_BEHAVIOR_SLIDEBOUNCE;
	pThis->GroundBehavior			= INTERSECT_BEHAVIOR_SLIDE;
	pThis->GravityAcceleration		= GRAVITY_ACCELERATION;
	pThis->BounceReturnEnergy		= 0.3;
	pThis->GroundFriction			= 0.7;
	pThis->AirFriction				= 0.0;
	pThis->WaterFriction				= 0.0;
}

void CCollisionInfo__SetClimbingAndSwimmingCharacterDefaults(CCollisionInfo2 *pThis)
{
	pThis->dwFlags =		COLLISIONFLAG_PHYSICS
							|	COLLISIONFLAG_CLIMBUP
							|	COLLISIONFLAG_CLIMBDOWN
							|	COLLISIONFLAG_ENTERWATER
							|	COLLISIONFLAG_EXITWATER
							|	COLLISIONFLAG_USEWALLRADIUS
							|	COLLISIONFLAG_TRACKGROUND
							|	COLLISIONFLAG_WATERCOLLISION
							|	COLLISIONFLAG_IGNOREDEAD
							|	COLLISIONFLAG_USEHEIGHT;

	pThis->InstanceBehavior			= INTERSECT_BEHAVIOR_GREASE;
	pThis->WallBehavior				= INTERSECT_BEHAVIOR_SLIDEBOUNCE;
	pThis->GroundBehavior			= INTERSECT_BEHAVIOR_SLIDE;
	pThis->GravityAcceleration		= GRAVITY_ACCELERATION;
	pThis->BounceReturnEnergy		= 0.3;
	pThis->GroundFriction			= 0.7;
	pThis->AirFriction				= 0.0;
	pThis->WaterFriction				= 0.0;
}

void CCollisionInfo__SetUnderwaterClimbingCharacterDefaults(CCollisionInfo2 *pThis)
{
	pThis->dwFlags =		COLLISIONFLAG_PHYSICS
							|	COLLISIONFLAG_CLIMBUP
							|	COLLISIONFLAG_CLIMBDOWN
							|	COLLISIONFLAG_ENTERWATER
							|	COLLISIONFLAG_EXITWATER
							|	COLLISIONFLAG_USEWALLRADIUS
							|	COLLISIONFLAG_TRACKGROUND
							|	COLLISIONFLAG_WATERCOLLISION
							|	COLLISIONFLAG_IGNOREDEAD
							|	COLLISIONFLAG_USEHEIGHT;

	pThis->InstanceBehavior			= INTERSECT_BEHAVIOR_GREASE;
	pThis->WallBehavior				= INTERSECT_BEHAVIOR_SLIDE;
	pThis->GroundBehavior			= INTERSECT_BEHAVIOR_SLIDE;
	pThis->GravityAcceleration		= 0.0;
	pThis->BounceReturnEnergy		= 0.3;
	pThis->GroundFriction			= 0.7;
	pThis->AirFriction				= 0.0;
	pThis->WaterFriction				= 0.0;
}

void CCollisionInfo__SetUnderwaterCharacterDefaults(CCollisionInfo2 *pThis)
{
	pThis->dwFlags =		COLLISIONFLAG_PHYSICS
							|	COLLISIONFLAG_CLIMBDOWN
							|	COLLISIONFLAG_ENTERWATER
							|	COLLISIONFLAG_USEWALLRADIUS
							|	COLLISIONFLAG_TRACKGROUND
							|	COLLISIONFLAG_WATERCOLLISION
							|	COLLISIONFLAG_IGNOREDEAD
							|	COLLISIONFLAG_USEHEIGHT;

	pThis->InstanceBehavior			= INTERSECT_BEHAVIOR_GREASE;
	pThis->WallBehavior				= INTERSECT_BEHAVIOR_SLIDE;
	pThis->GroundBehavior			= INTERSECT_BEHAVIOR_SLIDE;
	pThis->GravityAcceleration		= 0.0;
	pThis->BounceReturnEnergy		= 0.3;
	pThis->GroundFriction			= 0.7;
	pThis->AirFriction				= 0.0;
	pThis->WaterFriction				= 0.0;
}

/*
void CCollisionInfo__SetUnderwaterWalkCharacterDefaults(CCollisionInfo2 *pThis)
{
	pThis->dwFlags =		COLLISIONFLAG_PHYSICS
							|	COLLISIONFLAG_CLIMBDOWN
							|	COLLISIONFLAG_ENTERWATER
							|	COLLISIONFLAG_USEWALLRADIUS
							|	COLLISIONFLAG_TRACKGROUND
							|	COLLISIONFLAG_WATERCOLLISION;

	pThis->InstanceBehavior			= INTERSECT_BEHAVIOR_GREASE;
	pThis->WallBehavior				= INTERSECT_BEHAVIOR_SLIDE;
	pThis->GroundBehavior			= INTERSECT_BEHAVIOR_SLIDE;
	pThis->GravityAcceleration		= GRAVITY_ACCELERATION;
	pThis->BounceReturnEnergy		= 0.3;
	pThis->GroundFriction			= 0.7;
	pThis->AirFriction				= 0.0;
	pThis->WaterFriction				= 0.0;
}
*/

void CCollisionInfo__SetUnderwaterCharacterDeadDefaults(CCollisionInfo2 *pThis)
{
	pThis->dwFlags =		COLLISIONFLAG_PHYSICS
							|	COLLISIONFLAG_CLIMBDOWN
							|	COLLISIONFLAG_ENTERWATER
							|	COLLISIONFLAG_USEWALLRADIUS
							|	COLLISIONFLAG_WATERCOLLISION
							|	COLLISIONFLAG_IGNOREDEAD
							|	COLLISIONFLAG_USEHEIGHT;

	pThis->InstanceBehavior			= INTERSECT_BEHAVIOR_SLIDE;
	pThis->WallBehavior				= INTERSECT_BEHAVIOR_SLIDE;
	pThis->GroundBehavior			= INTERSECT_BEHAVIOR_SLIDE;
	pThis->GravityAcceleration		= GRAVITY_WATER_ACCELERATION/4.0;
	pThis->BounceReturnEnergy		= 0.3;
	pThis->GroundFriction			= 0.7;
	pThis->AirFriction				= 0.0;
	pThis->WaterFriction				= 0.04;
}

void CCollisionInfo__SetUnderwaterCharacterFloatDeadDefaults(CCollisionInfo2 *pThis)
{
	pThis->dwFlags =		COLLISIONFLAG_PHYSICS
							|	COLLISIONFLAG_CLIMBDOWN
							|	COLLISIONFLAG_ENTERWATER
							|	COLLISIONFLAG_USEWALLRADIUS
							|	COLLISIONFLAG_WATERCOLLISION
							|	COLLISIONFLAG_IGNOREDEAD
							|	COLLISIONFLAG_USEHEIGHT;

	pThis->InstanceBehavior			= INTERSECT_BEHAVIOR_SLIDE;
	pThis->WallBehavior				= INTERSECT_BEHAVIOR_SLIDE;
	pThis->GroundBehavior			= INTERSECT_BEHAVIOR_SLIDE;
	pThis->GravityAcceleration		= 0.0;
	pThis->BounceReturnEnergy		= 0.3;
	pThis->GroundFriction			= 0.7;
	pThis->AirFriction				= 0.0;
	pThis->WaterFriction				= 0.04;
}

void CCollisionInfo__SetParticleDefaults(CCollisionInfo2 *pThis)
{
	pThis->dwFlags =		COLLISIONFLAG_WATERCOLLISION
							|	COLLISIONFLAG_CANENTERRESTRICTEDAREA
							|	COLLISIONFLAG_PHYSICS
							|	COLLISIONFLAG_CLIMBUP
							|	COLLISIONFLAG_CLIMBDOWN
							|	COLLISIONFLAG_ENTERWATER
							|	COLLISIONFLAG_EXITWATER;

	pThis->InstanceBehavior			= INTERSECT_BEHAVIOR_STICK;
	pThis->WallBehavior				= INTERSECT_BEHAVIOR_BOUNCE;
	pThis->GroundBehavior			= INTERSECT_BEHAVIOR_BOUNCE;
	pThis->GravityAcceleration		= GRAVITY_EARTH;
	pThis->BounceReturnEnergy		= 17.0/24.0;
	pThis->GroundFriction			= 0.1;
	pThis->AirFriction				= 0.0;
	pThis->WaterFriction				= 0.0;
}

void CCollisionInfo__SetPickupDefaults(CCollisionInfo2 *pThis)
{
	pThis->dwFlags =		COLLISIONFLAG_IGNOREDEAD
							|	COLLISIONFLAG_WATERCOLLISION
							|	COLLISIONFLAG_CANENTERRESTRICTEDAREA
							|	COLLISIONFLAG_PHYSICS
							|	COLLISIONFLAG_CLIMBUP
							|	COLLISIONFLAG_CLIMBDOWN
							|	COLLISIONFLAG_ENTERWATER
							|	COLLISIONFLAG_EXITWATER
							|	COLLISIONFLAG_USEHEIGHT;

	pThis->InstanceBehavior			= INTERSECT_BEHAVIOR_IGNORE;
	pThis->WallBehavior				= INTERSECT_BEHAVIOR_BOUNCE;
	pThis->GroundBehavior			= INTERSECT_BEHAVIOR_BOUNCE;
	pThis->GravityAcceleration		= GRAVITY_ACCELERATION/1.5;
	pThis->BounceReturnEnergy		= 0.6;
	pThis->GroundFriction			= 0.4;
	pThis->AirFriction				= 0.0;
	pThis->WaterFriction				= 0.0;
}

void CCollisionInfo__SetNoCollisionAtAllDefaults(CCollisionInfo2 *pThis)
{
	pThis->dwFlags =		COLLISIONFLAG_MOVETHROUGHDOORS
							|	COLLISIONFLAG_CANENTERRESTRICTEDAREA
							|	COLLISIONFLAG_CLIMBUP
							|	COLLISIONFLAG_CLIMBDOWN
							|	COLLISIONFLAG_ENTERWATER
							|	COLLISIONFLAG_EXITWATER;

	pThis->InstanceBehavior			= INTERSECT_BEHAVIOR_IGNORE;
	pThis->WallBehavior				= INTERSECT_BEHAVIOR_IGNORE;
	pThis->GroundBehavior			= INTERSECT_BEHAVIOR_IGNORE;
	pThis->GravityAcceleration		= 0.0;
	pThis->BounceReturnEnergy		= 0.3;
	pThis->GroundFriction			= 0.7;
	pThis->AirFriction				= 0.0;
	pThis->WaterFriction				= 0.0;
}

void CCollisionInfo__SetMovementTestDefaults(CCollisionInfo2 *pThis)
{
	pThis->dwFlags =		COLLISIONFLAG_EXITWATER;

	pThis->InstanceBehavior			= INTERSECT_BEHAVIOR_IGNORE;
	pThis->WallBehavior				= INTERSECT_BEHAVIOR_STICK;
	pThis->GroundBehavior			= INTERSECT_BEHAVIOR_IGNORE;
	pThis->GravityAcceleration		= 0.0;
	pThis->BounceReturnEnergy		= 0.0;
	pThis->GroundFriction			= 0.0;
	pThis->AirFriction				= 0.0;
	pThis->WaterFriction				= 0.0;
}

void CCollisionInfo__SetSwimmerMovementTestDefaults(CCollisionInfo2 *pThis)
{
	pThis->dwFlags =		COLLISIONFLAG_CLIMBDOWN
							|	COLLISIONFLAG_ENTERWATER;

	pThis->InstanceBehavior			= INTERSECT_BEHAVIOR_IGNORE;
	pThis->WallBehavior				= INTERSECT_BEHAVIOR_STICK;
	pThis->GroundBehavior			= INTERSECT_BEHAVIOR_IGNORE;
	pThis->GravityAcceleration		= 0.0;
	pThis->BounceReturnEnergy		= 0.0;
	pThis->GroundFriction			= 0.0;
	pThis->AirFriction				= 0.0;
	pThis->WaterFriction				= 0.0;
}

void CCollisionInfo__SetClimberMovementTestDefaults(CCollisionInfo2 *pThis)
{
	pThis->dwFlags =		COLLISIONFLAG_CLIMBUP
							|	COLLISIONFLAG_CLIMBDOWN
							|	COLLISIONFLAG_EXITWATER;

	pThis->InstanceBehavior			= INTERSECT_BEHAVIOR_IGNORE;
	pThis->WallBehavior				= INTERSECT_BEHAVIOR_STICK;
	pThis->GroundBehavior			= INTERSECT_BEHAVIOR_IGNORE;
	pThis->GravityAcceleration		= 0.0;
	pThis->BounceReturnEnergy		= 0.0;
	pThis->GroundFriction			= 0.0;
	pThis->AirFriction				= 0.0;
	pThis->WaterFriction				= 0.0;
}

void CCollisionInfo__SetClimberAndSwimmerMovementTestDefaults(CCollisionInfo2 *pThis)
{
	pThis->dwFlags =		COLLISIONFLAG_CLIMBUP
							|	COLLISIONFLAG_CLIMBDOWN
							|	COLLISIONFLAG_ENTERWATER
							|	COLLISIONFLAG_EXITWATER;

	pThis->InstanceBehavior			= INTERSECT_BEHAVIOR_IGNORE;
	pThis->WallBehavior				= INTERSECT_BEHAVIOR_STICK;
	pThis->GroundBehavior			= INTERSECT_BEHAVIOR_IGNORE;
	pThis->GravityAcceleration		= 0.0;
	pThis->BounceReturnEnergy		= 0.0;
	pThis->GroundFriction			= 0.0;
	pThis->AirFriction				= 0.0;
	pThis->WaterFriction				= 0.0;
}

void CCollisionInfo__SetRayCastDefaults(CCollisionInfo2 *pThis)
{
	pThis->dwFlags =		COLLISIONFLAG_CANENTERRESTRICTEDAREA
							|	COLLISIONFLAG_CLIMBUP
							|	COLLISIONFLAG_CLIMBDOWN
							|	COLLISIONFLAG_ENTERWATER
							|	COLLISIONFLAG_EXITWATER;

	pThis->InstanceBehavior			= INTERSECT_BEHAVIOR_IGNORE;
	pThis->WallBehavior				= INTERSECT_BEHAVIOR_STICK;
	pThis->GroundBehavior			= INTERSECT_BEHAVIOR_STICK;
	pThis->GravityAcceleration		= 0.0;
	pThis->BounceReturnEnergy		= 0.0;
	pThis->GroundFriction			= 0.0;
	pThis->AirFriction				= 0.0;
	pThis->WaterFriction				= 0.0;
}

void CCollisionInfo__SetNearRegionTestDefaults(CCollisionInfo2 *pThis)
{
	pThis->dwFlags =		COLLISIONFLAG_MOVETHROUGHDOORS
							|	COLLISIONFLAG_CANENTERRESTRICTEDAREA
							|	COLLISIONFLAG_CLIMBUP
							|	COLLISIONFLAG_CLIMBDOWN
							|	COLLISIONFLAG_ENTERWATER
							|	COLLISIONFLAG_EXITWATER;

	pThis->InstanceBehavior			= INTERSECT_BEHAVIOR_IGNORE;
	pThis->WallBehavior				= INTERSECT_BEHAVIOR_STICK;
	pThis->GroundBehavior			= INTERSECT_BEHAVIOR_IGNORE;
	pThis->GravityAcceleration		= 0.0;
	pThis->BounceReturnEnergy		= 0.0;
	pThis->GroundFriction			= 0.0;
	pThis->AirFriction				= 0.0;
	pThis->WaterFriction				= 0.0;
}

void CCollisionInfo__SetCliffTestDefaults(CCollisionInfo2 *pThis)
{
	pThis->dwFlags =		COLLISIONFLAG_CANENTERRESTRICTEDAREA
							|	COLLISIONFLAG_CLIMBUP
							|	COLLISIONFLAG_CLIMBDOWN
							|	COLLISIONFLAG_CLIFFONLY
							|	COLLISIONFLAG_TWOREDIRECTIONS;

	pThis->InstanceBehavior			= INTERSECT_BEHAVIOR_IGNORE;
	pThis->WallBehavior				= INTERSECT_BEHAVIOR_SLIDE;
	pThis->GroundBehavior			= INTERSECT_BEHAVIOR_IGNORE;
	pThis->GravityAcceleration		= 0.0;
	pThis->BounceReturnEnergy		= 0.0;
	pThis->GroundFriction			= 0.0;
	pThis->AirFriction				= 0.0;
	pThis->WaterFriction				= 0.0;
}














