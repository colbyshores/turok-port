// hedtrack.h

#ifndef _INC_HEDTRACK
#define _INC_HEDTRACK

/////////////////////////////////////////////////////////////////////////////

#include "defs.h"
#include "romstruc.h"
#include "graphu64.h"


/////////////////////////////////////////////////////////////////////////////
// Constants
/////////////////////////////////////////////////////////////////////////////
#define REVERSE_CONCAT	(1<<0)

#define TURN_FLIP_DIR	(1<<1)
#define TURN_AXIS_X		(1<<2)
#define TURN_AXIS_Y		(1<<3)
#define TURN_AXIS_Z		(1<<4)

#define TILT_FLIP_DIR	(1<<5)
#define TILT_AXIS_X		(1<<6)
#define TILT_AXIS_Y		(1<<7)
#define TILT_AXIS_Z		(1<<8)


/////////////////////////////////////////////////////////////////////////////
// Structures
/////////////////////////////////////////////////////////////////////////////

typedef struct CHeadTrackNodeInfo_t
{
	INT32		Flags ;	
} CHeadTrackNodeInfo ;

// Head trackinfo info structure
typedef struct CHeadTrackInfo_t
{
	FLOAT	EyeHeight ;						// Eye height relative to feet

	FLOAT TurnAngleOffset ;				// Turn angle offset
	FLOAT MaxTurnAngle ;					// Max left/right rotation angle
	FLOAT TurnSpeed ;						// Max left/right rotation tracking speed

	FLOAT MinTiltAngle ;					// Min looking up angle
	FLOAT MaxTiltAngle ;					// Max looking down angle
	FLOAT TiltSpeed ;						// Max up/down rotation tracking speed

	INT16	MinHeadNode ;					// Min head tracking node
	INT16	MaxHeadNode ;					// Max head tracking node

	CHeadTrackNodeInfo *pNodeInfo ;	// Ptr to node info list

	FLOAT	BigHeadScale ;					// Scale of big head cheat
} CHeadTrackInfo ;


/////////////////////////////////////////////////////////////////////////////
// Functions prototypes
/////////////////////////////////////////////////////////////////////////////
void AI_PerformHeadTracking(CGameObjectInstance *pThis) ;



///////////////////////////////////////////////////////////////////////////
#endif // _INC_HEDTRACK
