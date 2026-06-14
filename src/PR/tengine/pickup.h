// pickup.h
/////////////////////////////////////////////////////////////////////////////

#ifndef _INC_PICKUP
#define _INC_PICKUP


/****************************************************************************
* Global Defines
****************************************************************************/

// default ammo when picking up weapon
#define	DEFAULT_PISTOL_AMMO				10
#define	DEFAULT_RIFLE_AMMO				30
#define	DEFAULT_RIOTSHOTGUN_AMMO		10
#define	DEFAULT_AUTOSHOTGUN_AMMO		12
#define	DEFAULT_MINIGUN_AMMO				125
#define	DEFAULT_GRENADELAUNCHER_AMMO	5
#define	DEFAULT_MACHINEGUN_AMMO			100
#define	DEFAULT_TECH1_AMMO				50
#define	DEFAULT_ROCKET_AMMO				6
#define	DEFAULT_SHOCKWAVE_AMMO 			100
#define	DEFAULT_TECH2_AMMO				2
#define	DEFAULT_CHRONOSCEPTOR_AMMO		3

#define	DEFAULT_TEK_ARMOR					25

// refills for ammo pickups
#define	REFILL_ARROW			 			30
#define	REFILL_EXPARROW_SMALL			5
#define	REFILL_EXPARROW_LARGE			15

#define	REFILL_BULLET_CLIP	 			10
#define	REFILL_BULLET_BOX		 			50

#define	REFILL_SHOTGUN_SHELLS			5
#define	REFILL_SHOTGUN_BOX				20
#define	REFILL_EXPSHOTGUN_SHELLS	  	4
#define	REFILL_EXPSHOTGUN_BOX		  	10
#define	REFILL_MINIGUN						125
#define	REFILL_GRENADE						1
#define	REFILL_GRENADE_BOX				10
#define	REFILL_TECH_SMALL					25
#define	REFILL_TECH_LARGE					50
#define	REFILL_ROCKET						1
#define	REFILL_TECH2						1


// maximum amount can carry
#define	MAX_ARROWS							30
#define	MAX_EXPARROWS						15

#define	MAX_BULLETS							100
#define	MAX_BULLETS_BACKPACK				200

#define	MAX_SHOTGUN							20
#define	MAX_SHOTGUN_BACKPACK				40
#define	MAX_EXPSHOTGUN			  			10
#define	MAX_EXPSHOTGUN_BACKPACK			20

#define	MAX_MINIGUN							125

#define	MAX_GRENADES						20
#define	MAX_GRENADES_BACKPACK			30

#define	MAX_ENERGY							100

#define	MAX_ROCKETS							6
#define	MAX_ROCKETS_BACKPACK				12

#define	MAX_CHRONOSCEPTER					3

#define	MAX_TECH2							2
#define	MAX_TECH2_BACKPACK				3

#define	MAX_KEY2								3
#define	MAX_KEY3								3
#define	MAX_KEY4								3
#define	MAX_KEY5								3
#define	MAX_KEY6								3
#define	MAX_KEY7								3
#define	MAX_KEY8								5


// amounts of ammo used when firing weapon
#define	TEKBOW_AMMO_USE				1
#define	PISTOL_AMMO_USE				1
#define	RIFLE_AMMO_USE					1
#define	RIOTSHOTGUN_AMMO_USE			1
#define	AUTOSHOTGUN_AMMO_USE			1
#define	MINIGUN_AMMO_USE				1
#define	GRENADELAUNCHER_AMMO_USE	1
#define	MACHINEGUN_AMMO_USE			1
#define	TECH1_AMMO_USE					5
#define	ROCKET_AMMO_USE				1
#define	SHOCKWAVE_AMMO_USE1			2
#define	SHOCKWAVE_AMMO_USE2			4
#define	SHOCKWAVE_AMMO_USE3			6
#define	SHOCKWAVE_AMMO_USE4			8
#define	SHOCKWAVE_AMMO_USE5			10
#define	TECH2_AMMO_USE					1
#define	CHRONOSCEPTER_AMMO_USE		1



// spiritual invincibility defines
#define	SPIRITUAL_TIMER					30				// in seconds
#define	SPIRITUAL_TIMEIN					16				// frames to blend to slow speed
#define	SPIRITUAL_TIMEOUT					60

#define	SPIRITUAL_SKY						8.0
#define	SPIRITUAL_ENEMY					0.2
#define	SPIRITUAL_PARTICLE				0.2


/****************************************************************************
* CPickup prototypes
****************************************************************************/
void		CPickup__DisplayKeysRemaining(void) ;
void		CPickup_Pickup(CGameSimpleInstance *pPickup);
BOOL		CPickup__CanInstancePickItUp(CGameSimpleInstance *pPickup, CAnimInstanceHdr *pInst);





/////////////////////////////////////////////////////////////////////////////
// CPickupGenerator class
/////////////////////////////////////////////////////////////////////////////
typedef struct CPickupGenerator_t
{
	FLOAT					m_Time ;					// Time before next generation
	CDynamicSimple		*m_pPickup ;			// Ptr to pickup at current pos
	CVector3				m_vPos ;					// Start position
	CGameRegion			*m_pRegion ;			// Start region
} CPickupGenerator ;

// Functions
void CPickupGenerator__Construct(CPickupGenerator *pThis, CVector3 *pvPos) ;
CDynamicSimple *CPickupGenerator__Generate(CPickupGenerator *pThis, INT32 Type, FLOAT Time) ;


/////////////////////////////////////////////////////////////////////////////
// CPickupMonitor class
/////////////////////////////////////////////////////////////////////////////
#define MAX_PICKUP_GENERATORS		16
typedef struct CPickupMonitor_t
{
	BOOL					m_Active ;
	CPickupGenerator	m_Generators[MAX_PICKUP_GENERATORS] ;
	INT32					m_NoOfGenerators ;
} CPickupMonitor ;

// Functions
void CPickupMonitor__Construct(CPickupMonitor *pThis) ;
void CPickupMonitor__AddGenerators(CPickupMonitor *pThis, INT32 NoOfGenerators, CVector3 *pvPosList) ;
#define	CPickupMonitor__Activate(pThis)		(pThis)->m_Active = TRUE
#define	CPickupMonitor__Deactivate(pThis)	(pThis)->m_Active = FALSE
void CPickupMonitor__UpdateGenerator(CPickupMonitor *pThis, CPickupGenerator *pGenerator) ;
void CPickupMonitor__Update(CPickupMonitor *pThis) ;
void CPickupMonitor__ProduceWeapons(CPickupMonitor *pThis) ;


#endif // _INC_PICKUP
