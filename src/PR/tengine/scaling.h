// scaling.h

#define  SCALING_MAX_VISIBLE_DISTANCE     100.0
#define  SCALING_FAR_CLIP                 1024.0

#define	SCALING_NEAR_CLIP						(((int)SCALING_FAR_CLIP) >> 6)

#define  SCALING_FACTOR                   (SCALING_FAR_CLIP/SCALING_MAX_VISIBLE_DISTANCE)
#define  SCALING_MINIMUM_OBJECT_SCALE     0.5
#define	SCALING_SIMPLE_OBJECT_SCALE		0.35
#define	SCALING_DETAIL_OBJECT_SCALE		0.2
#define	SCALING_DEF_INSTANCE_SCALE			0.02
#define	SCALING_PLAYERWEAPON_SCALE			0.03
#define	SCALING_PLAYER_SCALE					0.02
#define	SCALING_SIMPLE_SCALE					1.0

#ifdef WIN32
#define 	SCREEN_WD								320
#define 	SCREEN_HT								240
#endif

#define	DEFAULT_FOG_THICKNESS				0.8
#define	DEFAULT_FIELD_OF_VIEW				47.5
#define	FIELD_OF_VIEW_ANGLE(fov, aspect)	((fov)*(aspect))

#define	SCALING_MAX_DIST(farClip,fov)		(farClip/cos((fov)*(3.14159265358979323846/180.0)/2.0))




