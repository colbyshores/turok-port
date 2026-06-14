// tcontrol.h
// turok control header file
//

#ifndef __TCONTROLH
#define __TCONTROLH

#include "control.h"








// control types
#define		CTTYPE_NONE					0				// no action
#define		CTTYPE_DOWN					1				// action on button down
#define		CTTYPE_UP					2				// action on button up
#define		CTTYPE_SINGLE				3				// action on button single press
#define		CTTYPE_DBL					4				// action on button double press
#define		CTTYPE_DBLHOLD				5				// action on button double press hold
#define		CTTYPE_STICK_FORWARD		6				// analogue stick forwards
#define		CTTYPE_STICK_BACKWARD	7				// analogue stick backwards
#define		CTTYPE_STICK_LEFT			8				// analogue stick left
#define		CTTYPE_STICK_RIGHT		9				// analogue stick right
#define		CTTYPE_STICK_SFORWARD	10				// smooth analogue stick forwards
#define		CTTYPE_STICK_SBACKWARD	11				// smooth analogue stick backwards
#define		CTTYPE_STICK_SLEFT		12				// smooth analogue stick left
#define		CTTYPE_STICK_SRIGHT		13				// smooth analogue stick right


// output control
#define		CT_CENTER_THRESHOLD			8				// centering threshold
#define		CT_MAX_OUTPUT					-1				// digital operation
#define		CT_MAX_STICK					80				// max stick value in any direction


// turok control object
typedef struct
{
	CControlState	  *pCState;					// pointer to control state for this turok control object
	u8						CenteringX,				// x & y centering threshold
							CenteringY;
	u8						LookIndependant;		// look independant of direction of travel

	// bit field flags for buttons
	u16					Fire1,
							Forward,
							Backward,
							Left,
							Right,
							Duck,
							Jump,
							JumpReleased,
							MapToggle,
							SideStepLeft,
							SideStepRight,
							SelectWeaponPrev,
							SelectWeaponNext,
							SelectEnemyPrev,
							SelectEnemyNext,
							OpenDoor,
							RollRight,
							RollLeft,
							Pause,
							Start,
							StartHeld,
							UseMenu,
							LookUp,
							LookDown,
							LookLeft,
							LookRight,
							RunBackward,
							RunForward,
							RollSpeedL,
							RollSpeedR,
							GrenadeLauncherFired,
							ToggleOnScreen,
							ClickUp,
							ClickDown,
							ClickLeft,
							ClickRight,
							RunWalkToggle,
							SwimForward,
							SwimBackward,
							SwimSideStepL,
							SwimSideStepR,
							SwimUp,
							SwimBurstForward,
							MapLeft,
							MapRight,
							MapDown,
							MapUp,
							StartJump,
							AnalogLeft,
							AnalogRight,
							AnalogDown,
							AnalogUp,
							ClimbLookLeft,
							ClimbLookRight,
							ClimbUp,
							ClimbDown,
							ClimbLeft,
							ClimbRight,
							BurstSwimToggle,
							DigitalUp,
							DigitalDown,
							ZRotTog,
							ZRotLeft,
							ZRotRight,
							EnemyForward,
							EnemyBackward,
							EnemyLeft,
							EnemyRight,
							EnemyRotLeft,
							EnemyRotRight,
							FlyUp,
							FlyDown,
							FlyLeft,
							FlyRight,
							FlyForward,
							FlyBackward;

	// pre-condition bit fields
	u16					PreFire1,
							PreForward,
							PreBackward,
							PreLeft,
							PreRight,
							PreDuck,
							PreJump,
							PreJumpReleased,
							PreMapToggle,
							PreSideStepLeft,
							PreSideStepRight,
							PreSelectWeaponPrev,
							PreSelectWeaponNext,
							PreSelectEnemyPrev,
							PreSelectEnemyNext,
							PreOpenDoor,
							PreRollLeft,
							PreRollRight,
							PrePause,
							PreStart,
							PreStartHeld,
							PreUseMenu,
							PreLookUp,
							PreLookDown,
							PreLookLeft,
							PreLookRight,
							PreRunBackward,
							PreRunForward,
							PreRollSpeedL,
							PreRollSpeedR,
							PreGrenadeLauncherFired,
							PreToggleOnScreen,
							PreClickUp,
							PreClickDown,
							PreClickLeft,
							PreClickRight,
							PreRunWalkToggle,
							PreSwimForward,
							PreSwimBackward,
							PreSwimSideStepL,
							PreSwimSideStepR,
							PreSwimUp,
							PreSwimBurstForward,
							PreMapLeft,
							PreMapRight,
							PreMapDown,
							PreMapUp,
							PreStartJump,
							PreAnalogLeft,
							PreAnalogRight,
							PreAnalogDown,
							PreAnalogUp,
							PreClimbLookLeft,
							PreClimbLookRight,
							PreClimbUp,
							PreClimbDown,
							PreClimbLeft,
							PreClimbRight,
							PreBurstSwimToggle,
							PreDigitalUp,
							PreDigitalDown,
							PreZRotTog,
							PreZRotLeft,
							PreZRotRight,
							PreEnemyForward,
							PreEnemyBackward,
							PreEnemyLeft,
							PreEnemyRight,
							PreEnemyRotLeft,
							PreEnemyRotRight,
							PreFlyUp,
							PreFlyDown,
							PreFlyLeft,
							PreFlyRight,
							PreFlyForward,
							PreFlyBackward;

	// not-condition bit fields
	u16					NotFire1,
							NotForward,
							NotBackward,
							NotLeft,
							NotRight,
							NotDuck,
							NotJump,
							NotJumpReleased,
							NotMapToggle,
							NotSideStepLeft,
							NotSideStepRight,
							NotSelectWeaponPrev,
							NotSelectWeaponNext,
							NotSelectEnemyPrev,
							NotSelectEnemyNext,
							NotOpenDoor,
							NotRollLeft,
							NotRollRight,
							NotPause,
							NotStart,
							NotStartHeld,
							NotUseMenu,
							NotLookUp,
							NotLookDown,
							NotLookLeft,
							NotLookRight,
							NotRunBackward,
							NotRunForward,
							NotRollSpeedL,
							NotRollSpeedR,
							NotGrenadeLauncherFired,
							NotToggleOnScreen,
							NotClickUp,
							NotClickDown,
							NotClickLeft,
							NotClickRight,
							NotRunWalkToggle,
							NotSwimForward,
							NotSwimBackward,
							NotSwimSideStepL,
							NotSwimSideStepR,
							NotSwimUp,
							NotSwimBurstForward,
							NotMapLeft,
							NotMapRight,
							NotMapDown,
							NotMapUp,
							NotStartJump,
							NotAnalogLeft,
							NotAnalogRight,
							NotAnalogDown,
							NotAnalogUp,
							NotClimbLookLeft,
							NotClimbLookRight,
							NotClimbUp,
							NotClimbDown,
							NotClimbLeft,
							NotClimbRight,
							NotBurstSwimToggle,
							NotDigitalUp,
							NotDigitalDown,
							NotZRotTog,
							NotZRotLeft,
							NotZRotRight,
							NotEnemyForward,
							NotEnemyBackward,
							NotEnemyLeft,
							NotEnemyRight,
							NotEnemyRotLeft,
							NotEnemyRotRight,
							NotFlyUp,
							NotFlyDown,
							NotFlyLeft,
							NotFlyRight,
							NotFlyForward,
							NotFlyBackward;

	// type flags
	u8						TypeFire1,
							TypeForward,
							TypeBackward,
							TypeLeft,
							TypeRight,
							TypeDuck,
							TypeJump,
							TypeJumpReleased,
							TypeMapToggle,
							TypeSideStepLeft,
							TypeSideStepRight,
							TypeSelectWeaponPrev,
							TypeSelectWeaponNext,
							TypeSelectEnemyPrev,
							TypeSelectEnemyNext,
							TypeOpenDoor,
							TypeRollLeft,
							TypeRollRight,
							TypePause,
							TypeStart,
							TypeStartHeld,
							TypeUseMenu,
							TypeLookUp,
							TypeLookDown,
							TypeLookLeft,
							TypeLookRight,
							TypeRunBackward,
							TypeRunForward,
							TypeRollSpeedL,
							TypeRollSpeedR,
							TypeGrenadeLauncherFired,
							TypeToggleOnScreen,
							TypeClickUp,
							TypeClickDown,
							TypeClickLeft,
							TypeClickRight,
							TypeRunWalkToggle,
							TypeSwimForward,
							TypeSwimBackward,
							TypeSwimSideStepL,
							TypeSwimSideStepR,
							TypeSwimUp,
							TypeSwimBurstForward,
							TypeMapLeft,
							TypeMapRight,
							TypeMapDown,
							TypeMapUp,
							TypeStartJump,
							TypeAnalogLeft,
							TypeAnalogRight,
							TypeAnalogDown,
							TypeAnalogUp,
							TypeClimbLookLeft,
							TypeClimbLookRight,
							TypeClimbUp,
							TypeClimbDown,
							TypeClimbLeft,
							TypeClimbRight,
							TypeBurstSwimToggle,
							TypeDigitalUp,
							TypeDigitalDown,
							TypeZRotTog,
							TypeZRotLeft,
							TypeZRotRight,
							TypeEnemyForward,
							TypeEnemyBackward,
							TypeEnemyLeft,
							TypeEnemyRight,
							TypeEnemyRotLeft,
							TypeEnemyRotRight,
							TypeFlyUp,
							TypeFlyDown,
							TypeFlyLeft,
							TypeFlyRight,
							TypeFlyForward,
							TypeFlyBackward;

} CTControl;








// external declarations
CTControl*				CTControl__CTControl						(BOOL Right);
float						CTControl__IsPrimaryFire				(CTControl *pThis);
float						CTControl__IsForward						(CTControl *pThis);
float						CTControl__IsBackward					(CTControl *pThis);
float						CTControl__IsLeft							(CTControl *pThis);
float						CTControl__IsRight						(CTControl *pThis);
float						CTControl__IsDuck							(CTControl *pThis);
float						CTControl__IsJump							(CTControl *pThis);
float						CTControl__IsJumpReleased				(CTControl *pThis);
float						CTControl__IsDoubleJump					(CTControl *pThis);
float						CTControl__IsMapToggle					(CTControl *pThis);
float						CTControl__IsSideStepLeft				(CTControl *pThis);
float						CTControl__IsSideStepRight				(CTControl *pThis);
float						CTControl__IsSelectWeaponPrev			(CTControl *pThis);
float						CTControl__IsSelectWeaponNext			(CTControl *pThis);
float						CTControl__IsSelectEnemyPrev			(CTControl *pThis);
float						CTControl__IsSelectEnemyNext			(CTControl *pThis);
float						CTControl__IsOpenDoor					(CTControl *pThis);
float						CTControl__IsRollLeft					(CTControl *pThis);
float						CTControl__IsRollRight					(CTControl *pThis);
float						CTControl__IsPause						(CTControl *pThis);
float						CTControl__IsStart						(CTControl *pThis);
float						CTControl__IsStartHeld					(CTControl *pThis);
float						CTControl__IsUseMenu						(CTControl *pThis);
float						CTControl__IsLookUp						(CTControl *pThis);
float						CTControl__IsLookDown					(CTControl *pThis);
float						CTControl__IsLookLeft					(CTControl *pThis);
float						CTControl__IsLookRight					(CTControl *pThis);
float						CTControl__IsRunBackward				(CTControl *pThis);
float						CTControl__IsRunForward					(CTControl *pThis);
float						CTControl__IsRollSpeedL					(CTControl *pThis);
float						CTControl__IsRollSpeedR					(CTControl *pThis);
float						CTControl__IsFirstFire					(CTControl *pThis);
float						CTControl__IsDetonate					(CTControl *pThis);
float						CTControl__IsDetonateAll				(CTControl *pThis);
float						CTControl__IsGrenadeLauncherFired	(CTControl *pThis);
float						CTControl__IsToggleOnScreen			(CTControl *pThis);
float						CTControl__IsClickUp						(CTControl *pThis);
float						CTControl__IsClickDown					(CTControl *pThis);
float						CTControl__IsClickLeft					(CTControl *pThis);
float						CTControl__IsClickRight					(CTControl *pThis);
float						CTControl__IsRunWalkToggle				(CTControl *pThis);
float						CTControl__IsSwimForward				(CTControl *pThis);
float						CTControl__IsSwimBackward				(CTControl *pThis);
float						CTControl__IsSwimSideStepL				(CTControl *pThis);
float						CTControl__IsSwimSideStepR				(CTControl *pThis);
float						CTControl__IsSwimUp						(CTControl *pThis);
float						CTControl__IsSwimBurstForward			(CTControl *pThis);
float						CTControl__IsMapLeft						(CTControl *pThis);
float						CTControl__IsMapRight					(CTControl *pThis);
float						CTControl__IsMapDown						(CTControl *pThis);
float						CTControl__IsMapUp						(CTControl *pThis);
float						CTControl__IsStartJump					(CTControl *pThis);
float						CTControl__IsAnalogLeft					(CTControl *pThis);
float						CTControl__IsAnalogRight				(CTControl *pThis);
float						CTControl__IsAnalogDown					(CTControl *pThis);
float						CTControl__IsAnalogUp					(CTControl *pThis);
float						CTControl__IsClimbLookLeft				(CTControl *pThis);
float						CTControl__IsClimbLookRight			(CTControl *pThis);
float						CTControl__IsClimbUp						(CTControl *pThis);
float						CTControl__IsClimbDown					(CTControl *pThis);
float						CTControl__IsClimbLeft					(CTControl *pThis);
float						CTControl__IsClimbRight					(CTControl *pThis);
float						CTControl__IsBurstSwimToggle			(CTControl *pThis);
float						CTControl__IsDigitalUp					(CTControl *pThis);
float						CTControl__IsDigitalDown				(CTControl *pThis);
float						CTControl__IsZRotTog						(CTControl *pThis);
float						CTControl__IsZRotLeft					(CTControl *pThis);
float						CTControl__IsZRotRight					(CTControl *pThis);
float						CTControl__IsEnemyForward				(CTControl *pThis);
float						CTControl__IsEnemyBackward				(CTControl *pThis);
float						CTControl__IsEnemyLeft					(CTControl *pThis);
float						CTControl__IsEnemyRight					(CTControl *pThis);
float						CTControl__IsEnemyRotLeft				(CTControl *pThis);
float						CTControl__IsEnemyRotRight				(CTControl *pThis);
float						CTControl__IsFlyUp						(CTControl *pThis);
float						CTControl__IsFlyDown						(CTControl *pThis);
float						CTControl__IsFlyLeft						(CTControl *pThis);
float						CTControl__IsFlyRight					(CTControl *pThis);
float						CTControl__IsFlyForward					(CTControl *pThis);
float						CTControl__IsFlyBackward				(CTControl *pThis);

float						CTControl__Action					( CTControl *pThis,
																	  u16 Action,
																     u16 Pre,
																     u16 Type,
																	  u16 Not );						// is the correct action for this operation being done ?




extern CTControl CTControlArray;




#endif		// __TCONTROLH



