// tcontrol.c
// turok control functions for turoks abilities
//



#include "stdafx.h"
#include <ramrom.h>
#include "tengine.h"
#include "tcontrol.h"
#include "control.h"





// private data members
//
CTControl CTControlArray;





// constructor
//
CTControl* CTControl__CTControl(BOOL Right)
{
	// declare variables
	CTControl *pCTControl;


	// allocate space for turok control object
	pCTControl = &CTControlArray;
	pCTControl->pCState = &ControlStates;


	// create initial standard button layout - ian's layout
	// HEY ROB - leave these numbers alone!!
	pCTControl->CenteringX				= 5;									// centering threshold x
	pCTControl->CenteringY				= 4;									// centering threshold y
	ControlStates.Smooth					= 2;									// smoothing for stick
	pCTControl->LookIndependant		= 0;									// looking is direction of travel

	// swim movement
	pCTControl->PreSwimForward		= 0;
	pCTControl->NotSwimForward		= 0;
	pCTControl->SwimForward			= Right ? U_CBUTTONS : U_JPAD;
	pCTControl->TypeSwimForward	= CTTYPE_DOWN;

	pCTControl->PreSwimBackward	= 0;
	pCTControl->NotSwimBackward	= 0;
	pCTControl->SwimBackward		= Right ? D_CBUTTONS : D_JPAD;
	pCTControl->TypeSwimBackward	= CTTYPE_DOWN;

	pCTControl->PreSwimSideStepL	= 0;
	pCTControl->NotSwimSideStepL	= 0;
	pCTControl->SwimSideStepL		= Right ? L_CBUTTONS : L_JPAD;
	pCTControl->TypeSwimSideStepL	= CTTYPE_DOWN;

	pCTControl->PreSwimSideStepR	= 0;
	pCTControl->NotSwimSideStepR	= 0;
	pCTControl->SwimSideStepR		= Right ? R_CBUTTONS : R_JPAD;
	pCTControl->TypeSwimSideStepR	= CTTYPE_DOWN;

	pCTControl->PreSwimUp			= 0;
	pCTControl->NotSwimUp			= 0;
	pCTControl->SwimUp				= Right ? R_TRIG : L_TRIG;
	pCTControl->TypeSwimUp			= CTTYPE_DOWN;

	pCTControl->PreSwimBurstForward	= 0;
	pCTControl->NotSwimBurstForward	= 0;
	pCTControl->SwimBurstForward		= Right ? U_CBUTTONS : U_JPAD;
	pCTControl->TypeSwimBurstForward	= CTTYPE_DOWN;

	// flyiing mode controls (not real flying just test flying)
	pCTControl->PreFlyLeft		= 0;
	pCTControl->NotFlyLeft		= 0;
	pCTControl->FlyLeft			= L_CBUTTONS;
	pCTControl->TypeFlyLeft		= CTTYPE_DOWN;

	pCTControl->PreFlyRight		= 0;
	pCTControl->NotFlyRight		= 0;
	pCTControl->FlyRight			= R_CBUTTONS;
	pCTControl->TypeFlyRight	= CTTYPE_DOWN;

	pCTControl->PreFlyForward		= 0;
	pCTControl->NotFlyForward		= 0;
	pCTControl->FlyForward			= U_CBUTTONS;
	pCTControl->TypeFlyForward		= CTTYPE_DOWN;

	pCTControl->PreFlyBackward		= 0;
	pCTControl->NotFlyBackward		= 0;
	pCTControl->FlyBackward			= D_CBUTTONS;
	pCTControl->TypeFlyBackward	= CTTYPE_DOWN;

	pCTControl->PreFlyUp			= 0;
	pCTControl->NotFlyUp			= 0;
	pCTControl->FlyUp				= R_TRIG;
	pCTControl->TypeFlyUp		= CTTYPE_DOWN;

	pCTControl->PreFlyDown		= 0;
	pCTControl->NotFlyDown		= 0;
	pCTControl->FlyDown			= L_TRIG;
	pCTControl->TypeFlyDown		= CTTYPE_DOWN;

	// map controls
	pCTControl->PreMapLeft		= 0;
	pCTControl->NotMapLeft		= 0;
	pCTControl->MapLeft			= Right ? L_JPAD : L_CBUTTONS;
	pCTControl->TypeMapLeft		= CTTYPE_DOWN;

	pCTControl->PreMapRight		= 0;
	pCTControl->NotMapRight		= 0;
	pCTControl->MapRight			= Right ? R_JPAD : R_CBUTTONS;
	pCTControl->TypeMapRight	= CTTYPE_DOWN;

	pCTControl->PreMapUp			= 0;
	pCTControl->NotMapUp			= 0;
	pCTControl->MapUp				= Right ? U_JPAD : U_CBUTTONS;
	pCTControl->TypeMapUp		= CTTYPE_DOWN;

	pCTControl->PreMapDown		= 0;
	pCTControl->NotMapDown		= 0;
	pCTControl->MapDown			= Right ? D_JPAD : D_CBUTTONS;
	pCTControl->TypeMapDown		= CTTYPE_DOWN;

	// analog controls
	pCTControl->PreAnalogLeft	 	= 0;
	pCTControl->NotAnalogLeft	 	= 0;
	pCTControl->AnalogLeft		 	= 0;
	pCTControl->TypeAnalogLeft	 	= CTTYPE_STICK_SLEFT;

	pCTControl->PreAnalogRight	 	= 0;
	pCTControl->NotAnalogRight	 	= 0;
	pCTControl->AnalogRight		 	= 0;
	pCTControl->TypeAnalogRight 	= CTTYPE_STICK_SRIGHT;

	pCTControl->PreAnalogUp			= 0;
	pCTControl->NotAnalogUp			= 0;
	pCTControl->AnalogUp				= 0;
	pCTControl->TypeAnalogUp		= CTTYPE_STICK_SFORWARD;

	pCTControl->PreAnalogDown	 	= 0;
	pCTControl->NotAnalogDown	 	= 0;
	pCTControl->AnalogDown		 	= 0;
	pCTControl->TypeAnalogDown	 	= CTTYPE_STICK_SBACKWARD;

	// movement forwards backwards + left / right rotation
	pCTControl->PreForward		= 0;
	pCTControl->NotForward		= 0;
	pCTControl->Forward			= Right ? U_CBUTTONS : U_JPAD;
	pCTControl->TypeForward		= CTTYPE_DOWN;

	pCTControl->PreBackward		= 0;
	pCTControl->NotBackward		= 0;
	pCTControl->Backward			= Right ? D_CBUTTONS : D_JPAD;
	pCTControl->TypeBackward	= CTTYPE_DOWN;

	pCTControl->PreLeft			= 0;
	pCTControl->NotLeft			= 0;
	pCTControl->Left				= 0;
	pCTControl->TypeLeft			= CTTYPE_STICK_SLEFT;

	pCTControl->PreRight			= 0;
	pCTControl->NotRight			= 0;
	pCTControl->Right				= 0;
	pCTControl->TypeRight		= CTTYPE_STICK_SRIGHT;

	pCTControl->PreRunForward	= 0;
	pCTControl->NotRunForward	= 0;
	pCTControl->RunForward		= Right ? U_CBUTTONS : U_JPAD;
	pCTControl->TypeRunForward	= CTTYPE_DBLHOLD;

	pCTControl->PreRunBackward		= 0;
	pCTControl->NotRunBackward		= 0;
	pCTControl->RunBackward			= Right ? D_CBUTTONS : D_JPAD;
	pCTControl->TypeRunBackward	= CTTYPE_DBLHOLD;

	// primary firing
	pCTControl->PreFire1			= 0;
	pCTControl->NotFire1			= 0;
	pCTControl->Fire1				= Z_TRIG;
//	pCTControl->TypeFire1		= CTTYPE_DOWN;
	pCTControl->TypeFire1		= CTTYPE_SINGLE;

	// single press - weapons firing
	pCTControl->PreGrenadeLauncherFired			= 0;
	pCTControl->NotGrenadeLauncherFired			= 0;
	pCTControl->GrenadeLauncherFired				= Z_TRIG;
	pCTControl->TypeGrenadeLauncherFired		= CTTYPE_SINGLE;

	// jump
	pCTControl->PreStartJump			= 0;
	pCTControl->NotStartJump			= 0;
	pCTControl->StartJump				= Right ? R_TRIG : L_TRIG;
	pCTControl->TypeStartJump			= CTTYPE_SINGLE;

	pCTControl->PreJump			= 0;
	pCTControl->NotJump			= 0;
	pCTControl->Jump				= Right ? R_TRIG : L_TRIG;
	pCTControl->TypeJump			= CTTYPE_DOWN;

	pCTControl->PreJumpReleased	= 0;
	pCTControl->NotJumpReleased	= 0;
	pCTControl->JumpReleased		= Right ? R_TRIG : L_TRIG;
	pCTControl->TypeJumpReleased	= CTTYPE_UP;

	// select weapon prev & next
	pCTControl->PreSelectWeaponPrev			= 0;
	pCTControl->NotSelectWeaponPrev			= Z_TRIG;
	pCTControl->SelectWeaponPrev				= B_BUTTON;
	pCTControl->TypeSelectWeaponPrev			= CTTYPE_DOWN;

	pCTControl->PreSelectWeaponNext			= 0;
	pCTControl->NotSelectWeaponNext			= Z_TRIG;
	pCTControl->SelectWeaponNext				= A_BUTTON;
	pCTControl->TypeSelectWeaponNext			= CTTYPE_DOWN;

	// select enemy prev & next
	pCTControl->PreSelectEnemyPrev			= 0;
	pCTControl->NotSelectEnemyPrev			= 0;
	pCTControl->SelectEnemyPrev				= B_BUTTON;
	pCTControl->TypeSelectEnemyPrev			= CTTYPE_SINGLE;

	pCTControl->PreSelectEnemyNext			= 0;
	pCTControl->NotSelectEnemyNext			= 0;
	pCTControl->SelectEnemyNext				= A_BUTTON;
	pCTControl->TypeSelectEnemyNext			= CTTYPE_SINGLE;

	pCTControl->PreEnemyForward			= 0;
	pCTControl->NotEnemyForward			= 0;
	pCTControl->EnemyForward				= U_JPAD;
	pCTControl->TypeEnemyForward			= CTTYPE_SINGLE;

	pCTControl->PreEnemyBackward		= 0;
	pCTControl->NotEnemyBackward		= 0;
	pCTControl->EnemyBackward			= D_JPAD;
	pCTControl->TypeEnemyBackward		= CTTYPE_SINGLE;

	pCTControl->PreEnemyLeft		= 0;
	pCTControl->NotEnemyLeft		= 0;
	pCTControl->EnemyLeft	 		= L_JPAD;
	pCTControl->TypeEnemyLeft		= CTTYPE_SINGLE;

	pCTControl->PreEnemyRight		= 0;
	pCTControl->NotEnemyRight		= 0;
	pCTControl->EnemyRight			= R_JPAD;
	pCTControl->TypeEnemyRight		= CTTYPE_SINGLE;

	pCTControl->PreEnemyRotLeft		= 0;
	pCTControl->NotEnemyRotLeft		= 0;
	pCTControl->EnemyRotLeft	 		= L_TRIG;
	pCTControl->TypeEnemyRotLeft		= CTTYPE_SINGLE;

	pCTControl->PreEnemyRotRight		= 0;
	pCTControl->NotEnemyRotRight		= 0;
	pCTControl->EnemyRotRight			= R_TRIG;
	pCTControl->TypeEnemyRotRight		= CTTYPE_SINGLE;

	// run walk toggle
	pCTControl->PreRunWalkToggle		= 0;
	pCTControl->NotRunWalkToggle		= 0;
	pCTControl->RunWalkToggle			= Right	? (D_JPAD | U_JPAD | L_JPAD | R_JPAD) : (D_CBUTTONS | U_CBUTTONS | L_CBUTTONS | R_CBUTTONS);
	pCTControl->TypeRunWalkToggle		= CTTYPE_SINGLE;

	// burst swim toggle
	pCTControl->PreBurstSwimToggle	= 0;
	pCTControl->NotBurstSwimToggle	= 0;
	pCTControl->BurstSwimToggle		= Right	? (D_JPAD | U_JPAD | L_JPAD | R_JPAD) : (D_CBUTTONS | U_CBUTTONS | L_CBUTTONS | R_CBUTTONS);
	pCTControl->TypeBurstSwimToggle	= CTTYPE_SINGLE;

	// open door
	pCTControl->PreOpenDoor		= 0;
	pCTControl->NotOpenDoor		= 0;
	pCTControl->OpenDoor			= Right ? U_CBUTTONS : U_JPAD;
	pCTControl->TypeOpenDoor	= CTTYPE_SINGLE;

	// map toggle
	pCTControl->PreMapToggle	= 0;
	pCTControl->NotMapToggle	= 0;
	pCTControl->MapToggle		= Right ? L_TRIG : R_TRIG ;
	pCTControl->TypeMapToggle	= CTTYPE_DOWN;

	// toggle on screen
	pCTControl->PreToggleOnScreen		= 0;
	pCTControl->NotToggleOnScreen		= 0;
	pCTControl->ToggleOnScreen			= 0;
	pCTControl->TypeToggleOnScreen	= CTTYPE_NONE;

	// side step left & right
	pCTControl->PreSideStepLeft	= 0;
	pCTControl->NotSideStepLeft	= 0;
	pCTControl->SideStepLeft		= Right ? L_CBUTTONS : L_JPAD;
	pCTControl->TypeSideStepLeft	= CTTYPE_DOWN;

	pCTControl->PreSideStepRight	= 0;
	pCTControl->NotSideStepRight	= 0;
	pCTControl->SideStepRight		= Right ? R_CBUTTONS : R_JPAD;
	pCTControl->TypeSideStepRight	= CTTYPE_DOWN;

	// roll left & right
	pCTControl->PreRollLeft		= 0;
	pCTControl->NotRollLeft		= 0;
	pCTControl->RollLeft			= Right ? L_CBUTTONS : L_JPAD;
	pCTControl->TypeRollLeft	= CTTYPE_DBL;

	pCTControl->PreRollRight	= 0;
	pCTControl->NotRollRight	= 0;
	pCTControl->RollRight		= Right ? R_CBUTTONS : R_JPAD;
	pCTControl->TypeRollRight	= CTTYPE_DBL;

	pCTControl->PreRollSpeedL	= 0;
	pCTControl->NotRollSpeedL	= 0;
	pCTControl->RollSpeedL		= 0;
	pCTControl->TypeRollSpeedL	= CTTYPE_STICK_SLEFT;

	pCTControl->PreRollSpeedR	= 0;
	pCTControl->NotRollSpeedR	= 0;
	pCTControl->RollSpeedR		= 0;
	pCTControl->TypeRollSpeedR	= CTTYPE_STICK_SRIGHT;

	// pause & start
	pCTControl->PrePause			= 0;
	pCTControl->NotPause			= 0;
	pCTControl->Pause				= START_BUTTON;
	pCTControl->TypePause		= CTTYPE_SINGLE;

	pCTControl->PreStart			= 0;
	pCTControl->NotStart			= 0;
	pCTControl->Start				= START_BUTTON;
	pCTControl->TypeStart		= CTTYPE_SINGLE;

	pCTControl->PreStartHeld	= 0;
	pCTControl->NotStartHeld	= 0;
	pCTControl->StartHeld		= START_BUTTON;
	pCTControl->TypeStartHeld	= CTTYPE_DOWN;

	// use menu control
	pCTControl->PreUseMenu		= 0;
	pCTControl->NotUseMenu		= 0;
	pCTControl->UseMenu			= A_BUTTON /*| B_BUTTON | Z_TRIG | L_TRIG | R_TRIG*/ | START_BUTTON;
	pCTControl->TypeUseMenu		= CTTYPE_SINGLE;

	pCTControl->PreZRotTog		= 0;
	pCTControl->NotZRotTog		= 0;
	pCTControl->ZRotTog			= L_TRIG | R_TRIG;
	pCTControl->TypeZRotTog		= CTTYPE_DOWN;

	pCTControl->PreZRotLeft		= 0;
	pCTControl->NotZRotLeft		= 0;
	pCTControl->ZRotLeft			= A_BUTTON;
	pCTControl->TypeZRotLeft	= CTTYPE_DOWN;

	pCTControl->PreZRotRight	= 0;
	pCTControl->NotZRotRight	= 0;
	pCTControl->ZRotRight		= B_BUTTON;
	pCTControl->TypeZRotRight	= CTTYPE_DOWN;

	// menu controls
	pCTControl->PreClickUp			= 0;
	pCTControl->NotClickUp			= 0;
	pCTControl->ClickUp				= U_JPAD | U_CBUTTONS;
	pCTControl->TypeClickUp			= CTTYPE_SINGLE;

	pCTControl->PreClickDown		= 0;
	pCTControl->NotClickDown		= 0;
	pCTControl->ClickDown			= D_JPAD | D_CBUTTONS;
	pCTControl->TypeClickDown		= CTTYPE_SINGLE;

	pCTControl->PreClickLeft		= 0;
	pCTControl->NotClickLeft		= 0;
	pCTControl->ClickLeft	 		= L_JPAD | L_CBUTTONS;
	pCTControl->TypeClickLeft		= CTTYPE_SINGLE;

	pCTControl->PreClickRight		= 0;
	pCTControl->NotClickRight		= 0;
	pCTControl->ClickRight			= R_JPAD | R_CBUTTONS;
	pCTControl->TypeClickRight		= CTTYPE_SINGLE;

	// digital controls
	pCTControl->PreDigitalUp		= 0;
	pCTControl->NotDigitalUp		= 0;
	pCTControl->DigitalUp			= U_JPAD | U_CBUTTONS;
	pCTControl->TypeDigitalUp		= CTTYPE_DOWN;

	pCTControl->PreDigitalDown		= 0;
	pCTControl->NotDigitalDown		= 0;
	pCTControl->DigitalDown			= D_JPAD | D_CBUTTONS;
	pCTControl->TypeDigitalDown	= CTTYPE_DOWN;

	// look around
	pCTControl->PreLookUp		= 0;
	pCTControl->NotLookUp		= 0;
	pCTControl->LookUp			= 0;
	pCTControl->TypeLookUp		= CTTYPE_STICK_SFORWARD;
	
	pCTControl->PreLookDown		= 0;
	pCTControl->NotLookDown		= 0;
	pCTControl->LookDown			= 0;
	pCTControl->TypeLookDown	= CTTYPE_STICK_SBACKWARD;

	pCTControl->PreLookLeft		= 0;
	pCTControl->NotLookLeft		= 0;
	pCTControl->LookLeft			= 0;
	pCTControl->TypeLookLeft	= CTTYPE_NONE;

	pCTControl->PreLookRight	= 0;
	pCTControl->NotLookRight	= 0;
	pCTControl->LookRight		= 0;
	pCTControl->TypeLookRight	= CTTYPE_NONE;

	// climbing look left & right
	pCTControl->PreClimbLookLeft		= 0;
	pCTControl->NotClimbLookLeft		= 0;
	pCTControl->ClimbLookLeft			= 0;
	pCTControl->TypeClimbLookLeft		= CTTYPE_STICK_SLEFT;

	pCTControl->PreClimbLookRight		= 0;
	pCTControl->NotClimbLookRight		= 0;
	pCTControl->ClimbLookRight			= 0;
	pCTControl->TypeClimbLookRight	= CTTYPE_STICK_SRIGHT;

	// climbing controls
	pCTControl->PreClimbUp		= 0;
	pCTControl->NotClimbUp		= 0;
	pCTControl->ClimbUp			= Right ? U_CBUTTONS : U_JPAD;
	pCTControl->TypeClimbUp		= CTTYPE_DOWN;

	pCTControl->PreClimbDown	= 0;
	pCTControl->NotClimbDown	= 0;
	pCTControl->ClimbDown		= Right ? D_CBUTTONS : D_JPAD;
	pCTControl->TypeClimbDown	= CTTYPE_DOWN;

	pCTControl->PreClimbLeft	= 0;
	pCTControl->NotClimbLeft	= 0;
	pCTControl->ClimbLeft		= Right ? L_CBUTTONS : L_JPAD;
	pCTControl->TypeClimbLeft	= CTTYPE_DOWN;

	pCTControl->PreClimbRight	= 0;
	pCTControl->NotClimbRight	= 0;
	pCTControl->ClimbRight		= Right ? R_CBUTTONS : R_JPAD;
	pCTControl->TypeClimbRight	= CTTYPE_DOWN;

	
	// return pointer to turok control object
	return pCTControl;
}



// control functions
float CTControl__IsPrimaryFire				( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->Fire1,						pThis->PreFire1,							pThis->TypeFire1,							pThis->NotFire1						);}
float CTControl__IsForward						( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->Forward,						pThis->PreForward,						pThis->TypeForward,						pThis->NotForward						);}
float CTControl__IsBackward					( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->Backward,					pThis->PreBackward,						pThis->TypeBackward,						pThis->NotBackward					);}
float CTControl__IsLeft							( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->Left,							pThis->PreLeft,							pThis->TypeLeft,							pThis->NotLeft							);}
float CTControl__IsRight						( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->Right,						pThis->PreRight,							pThis->TypeRight,							pThis->NotRight						);}
float CTControl__IsJump							( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->Jump,							pThis->PreJump,							pThis->TypeJump,							pThis->NotJump							);}
float CTControl__IsJumpReleased				( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->JumpReleased,				pThis->PreJumpReleased,					pThis->TypeJumpReleased,				pThis->NotJumpReleased				);}
float CTControl__IsOpenDoor					( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->OpenDoor,					pThis->PreOpenDoor,						pThis->TypeOpenDoor,						pThis->NotOpenDoor					);}
float CTControl__IsSelectWeaponPrev			( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->SelectWeaponPrev,			pThis->PreSelectWeaponPrev,			pThis->TypeSelectWeaponPrev,			pThis->NotSelectWeaponPrev			);}
float CTControl__IsSelectWeaponNext			( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->SelectWeaponNext,			pThis->PreSelectWeaponNext,			pThis->TypeSelectWeaponNext,			pThis->NotSelectWeaponNext			);}
float CTControl__IsSelectEnemyPrev			( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->SelectEnemyPrev,			pThis->PreSelectEnemyPrev,				pThis->TypeSelectEnemyPrev,			pThis->NotSelectEnemyPrev			);}
float CTControl__IsSelectEnemyNext			( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->SelectEnemyNext,			pThis->PreSelectEnemyNext,				pThis->TypeSelectEnemyNext,			pThis->NotSelectEnemyNext			);}
float CTControl__IsSideStepLeft				( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->SideStepLeft,				pThis->PreSideStepLeft,					pThis->TypeSideStepLeft,				pThis->NotSideStepLeft				);}
float CTControl__IsSideStepRight				( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->SideStepRight,				pThis->PreSideStepRight,				pThis->TypeSideStepRight,				pThis->NotSideStepRight				);}
float CTControl__IsRollLeft					( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->RollLeft,					pThis->PreRollLeft,						pThis->TypeRollLeft,						pThis->NotRollLeft					);}
float CTControl__IsRollRight					( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->RollRight,					pThis->PreRollRight,						pThis->TypeRollRight,					pThis->NotRollRight					);}
float CTControl__IsPause						( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->Pause,						pThis->PrePause,							pThis->TypePause,							pThis->NotPause						);}
float CTControl__IsStart						( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->Start,						pThis->PreStart,							pThis->TypeStart,							pThis->NotStart						);}
float CTControl__IsStartHeld					( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->StartHeld,					pThis->PreStartHeld,						pThis->TypeStartHeld, 					pThis->NotStartHeld					);}
float CTControl__IsLookUp						( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->LookUp,						pThis->PreLookUp,							pThis->TypeLookUp,						pThis->NotLookUp						);}
float CTControl__IsLookDown					( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->LookDown,					pThis->PreLookDown,						pThis->TypeLookDown,						pThis->NotLookDown					);}
float CTControl__IsLookLeft					( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->LookLeft,					pThis->PreLookLeft,						pThis->TypeLookLeft,						pThis->NotLookLeft					);}
float CTControl__IsLookRight					( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->LookRight,					pThis->PreLookRight,						pThis->TypeLookRight,					pThis->NotLookRight					);}
float CTControl__IsRunBackward				( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->RunBackward,				pThis->PreRunBackward,					pThis->TypeRunBackward,					pThis->NotRunBackward				);}
float CTControl__IsRunForward					( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->RunForward,					pThis->PreRunForward,					pThis->TypeRunForward,					pThis->NotRunForward					);}
float CTControl__IsRollSpeedL					( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->RollSpeedL,					pThis->PreRollSpeedL,					pThis->TypeRollSpeedL,					pThis->NotRollSpeedL					);}
float CTControl__IsRollSpeedR					( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->RollSpeedR,					pThis->PreRollSpeedR,					pThis->TypeRollSpeedR,					pThis->NotRollSpeedR					);}
float CTControl__IsGrenadeLauncherFired	( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->GrenadeLauncherFired,	pThis->PreGrenadeLauncherFired,		pThis->TypeGrenadeLauncherFired,		pThis->NotGrenadeLauncherFired	);}
float CTControl__IsToggleOnScreen			( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->ToggleOnScreen,			pThis->PreToggleOnScreen,				pThis->TypeToggleOnScreen,				pThis->NotToggleOnScreen			);}
float CTControl__IsClickUp						( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->ClickUp,						pThis->PreClickUp,						pThis->TypeClickUp,						pThis->NotClickUp						);}
float CTControl__IsClickDown					( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->ClickDown,					pThis->PreClickDown,						pThis->TypeClickDown,					pThis->NotClickDown					);}
float CTControl__IsClickLeft					( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->ClickLeft,				 	pThis->PreClickLeft,						pThis->TypeClickLeft, 					pThis->NotClickLeft					);}
float CTControl__IsClickRight					( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->ClickRight,					pThis->PreClickRight,					pThis->TypeClickRight,					pThis->NotClickRight					);}
float CTControl__IsRunWalkToggle				( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->RunWalkToggle,				pThis->PreRunWalkToggle,				pThis->TypeRunWalkToggle,				pThis->NotRunWalkToggle				);}
float CTControl__IsBurstSwimToggle			( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->BurstSwimToggle,			pThis->PreBurstSwimToggle,				pThis->TypeBurstSwimToggle,			pThis->NotBurstSwimToggle			);}
float CTControl__IsSwimForward				( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->SwimForward,				pThis->PreSwimForward,					pThis->TypeSwimForward,					pThis->NotSwimForward				);}
float CTControl__IsSwimBackward				( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->SwimBackward,				pThis->PreSwimBackward,					pThis->TypeSwimBackward,				pThis->NotSwimBackward				);}
float CTControl__IsSwimSideStepL				( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->SwimSideStepL,				pThis->PreSwimSideStepL,				pThis->TypeSwimSideStepL,				pThis->NotSwimSideStepL				);}
float CTControl__IsSwimSideStepR				( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->SwimSideStepR,				pThis->PreSwimSideStepR,				pThis->TypeSwimSideStepR,				pThis->NotSwimSideStepR				);}
float CTControl__IsSwimUp						( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->SwimUp,						pThis->PreSwimUp,							pThis->TypeSwimUp,						pThis->NotSwimUp						);}
float CTControl__IsSwimBurstForward			( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->SwimBurstForward,			pThis->PreSwimBurstForward,			pThis->TypeSwimBurstForward,			pThis->NotSwimBurstForward			);}
float CTControl__IsMapLeft						( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->MapLeft,						pThis->PreMapLeft,						pThis->TypeMapLeft,						pThis->NotMapLeft						);}
float CTControl__IsMapRight					( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->MapRight,					pThis->PreMapRight,						pThis->TypeMapRight,						pThis->NotMapRight					);}
float CTControl__IsMapUp						( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->MapUp,						pThis->PreMapUp,							pThis->TypeMapUp,							pThis->NotMapUp						);}
float CTControl__IsMapDown						( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->MapDown,						pThis->PreMapDown,						pThis->TypeMapDown,						pThis->NotMapDown						);}
float CTControl__IsStartJump					( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->StartJump,					pThis->PreStartJump,						pThis->TypeStartJump,					pThis->NotStartJump					);}
float CTControl__IsAnalogLeft				  	( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->AnalogLeft,					pThis->PreAnalogLeft,					pThis->TypeAnalogLeft,					pThis->NotAnalogLeft					);}
float CTControl__IsAnalogRight			  	( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->AnalogRight,				pThis->PreAnalogRight,					pThis->TypeAnalogRight,					pThis->NotAnalogRight				);}
float CTControl__IsAnalogUp				  	( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->AnalogUp,					pThis->PreAnalogUp,						pThis->TypeAnalogUp,						pThis->NotAnalogUp					);}
float CTControl__IsAnalogDown				  	( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->AnalogDown,					pThis->PreAnalogDown,					pThis->TypeAnalogDown,					pThis->NotAnalogDown					);}
float CTControl__IsClimbLookLeft				( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->ClimbLookLeft,				pThis->PreClimbLookLeft,				pThis->TypeClimbLookLeft,				pThis->NotClimbLookLeft				);}
float CTControl__IsClimbLookRight			( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->ClimbLookRight,			pThis->PreClimbLookRight,				pThis->TypeClimbLookRight,				pThis->NotClimbLookRight			);}
float CTControl__IsClimbUp						( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->ClimbUp,						pThis->PreClimbUp,						pThis->TypeClimbUp,						pThis->NotClimbUp						);}
float CTControl__IsClimbDown					( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->ClimbDown,					pThis->PreClimbDown,						pThis->TypeClimbDown,					pThis->NotClimbDown					);}
float CTControl__IsClimbLeft					( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->ClimbLeft,					pThis->PreClimbLeft,						pThis->TypeClimbLeft,					pThis->NotClimbLeft					);}
float CTControl__IsClimbRight					( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->ClimbRight,					pThis->PreClimbRight,					pThis->TypeClimbRight,					pThis->NotClimbRight					);}
float CTControl__IsDigitalUp					( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->DigitalUp,					pThis->PreDigitalUp,						pThis->TypeDigitalUp,					pThis->NotDigitalUp					);}
float CTControl__IsDigitalDown				( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->DigitalDown,				pThis->PreDigitalDown,					pThis->TypeDigitalDown,					pThis->NotDigitalDown				);}
float CTControl__IsZRotTog						( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->ZRotTog,						pThis->PreZRotTog,						pThis->TypeZRotTog,						pThis->NotZRotTog						);}
float CTControl__IsZRotLeft					( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->ZRotLeft,					pThis->PreZRotLeft,						pThis->TypeZRotLeft,						pThis->NotZRotLeft					);}
float CTControl__IsZRotRight					( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->ZRotRight,					pThis->PreZRotRight,						pThis->TypeZRotRight,					pThis->NotZRotRight					);}
float CTControl__IsEnemyForward				( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->EnemyForward,				pThis->PreEnemyForward,					pThis->TypeEnemyForward,				pThis->NotEnemyForward				);}
float CTControl__IsEnemyBackward				( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->EnemyBackward,				pThis->PreEnemyBackward,				pThis->TypeEnemyBackward,				pThis->NotEnemyBackward				);}
float CTControl__IsEnemyLeft					( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->EnemyLeft,				 	pThis->PreEnemyLeft,						pThis->TypeEnemyLeft, 					pThis->NotEnemyLeft					);}
float CTControl__IsEnemyRight					( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->EnemyRight,					pThis->PreEnemyRight,					pThis->TypeEnemyRight,					pThis->NotEnemyRight					);}
float CTControl__IsEnemyRotLeft				( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->EnemyRotLeft,				pThis->PreEnemyRotLeft,					pThis->TypeEnemyRotLeft, 				pThis->NotEnemyRotLeft				);}
float CTControl__IsEnemyRotRight				( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->EnemyRotRight,				pThis->PreEnemyRotRight,				pThis->TypeEnemyRotRight,				pThis->NotEnemyRotRight				);}
float CTControl__IsFlyUp						( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->FlyUp,						pThis->PreFlyUp,							pThis->TypeFlyUp,							pThis->NotFlyUp						);}
float CTControl__IsFlyDown						( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->FlyDown,						pThis->PreFlyDown,						pThis->TypeFlyDown,						pThis->NotFlyDown						);}
float CTControl__IsFlyLeft						( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->FlyLeft,				 		pThis->PreFlyLeft,						pThis->TypeFlyLeft, 						pThis->NotFlyLeft						);}
float CTControl__IsFlyRight					( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->FlyRight,					pThis->PreFlyRight,						pThis->TypeFlyRight,						pThis->NotFlyRight					);}
float CTControl__IsFlyForward					( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->FlyForward,					pThis->PreFlyForward,					pThis->TypeFlyForward,					pThis->NotFlyForward					);}
float CTControl__IsFlyBackward				( CTControl *pThis )	{return CTControl__Action ( pThis, pThis->FlyBackward,				pThis->PreFlyBackward,					pThis->TypeFlyBackward,					pThis->NotFlyBackward				);}

float CTControl__IsMapToggle					( CTControl *pThis ) {return CTControl__Action ( pThis, pThis->MapToggle,					pThis->PreMapToggle,						pThis->TypeMapToggle,					pThis->NotMapToggle					);}

float CTControl__IsUseMenu (CTControl *pThis )
{
	float	State ;

	State = CTControl__Action ( pThis, pThis->UseMenu,						pThis->PreUseMenu,						pThis->TypeUseMenu,						pThis->NotUseMenu						);
	if (State)
		CScene__DoSoundEffect(&GetApp()->m_Scene, 21, 0, NULL, &GetApp()->m_Camera.m_vPos, 0);

	return State ;
}

// is control being pressed how it is required to be pressed ?
// returns
// 0                  if not pressed
// -1                 if digital button is pressed
// 0 to CT_MAX_STICK  if analogue control is used
//
float CTControl__Action ( CTControl *pThis, u16 Action, u16 Pre, u16 Type, u16 Not )
{
	// get pointer to control state
	CControlState *pCState;
	pCState = pThis->pCState;


	// is precondition satisfied for this control ?
	if ( (Pre == 0) || (pCState->BDown & Pre) )
	{
		if ( pCState->BDown & Not ) return 0;			// button pressed that should not have been pressed
		switch ( Type )
		{
			case CTTYPE_DOWN:
				return ( Action & pCState->BDown ) ? CT_MAX_OUTPUT : 0;
		
			case CTTYPE_UP:
				return ( Action & pCState->BUp ) ? CT_MAX_OUTPUT : 0;
		
			case CTTYPE_SINGLE:
				return ( Action & pCState->BSingle ) ? CT_MAX_OUTPUT : 0;
		
			case CTTYPE_DBL:
				return ( Action & pCState->BDbl ) ? CT_MAX_OUTPUT : 0;
		
			case CTTYPE_DBLHOLD:
				return ( Action & pCState->BDblHold ) ? CT_MAX_OUTPUT : 0;

			case CTTYPE_STICK_FORWARD:
				if ( pCState->StickY <= +pThis->CenteringY )
					return 0;
				else
					return (pCState->StickY - pThis->CenteringY) * CT_MAX_STICK / (CT_MAX_STICK-pThis->CenteringY);
		
			case CTTYPE_STICK_BACKWARD:
				if ( pCState->StickY >= -pThis->CenteringY )
					return 0;
				else
					return ((-pCState->StickY) - pThis->CenteringY) * CT_MAX_STICK / (CT_MAX_STICK-pThis->CenteringY);
		
			case CTTYPE_STICK_LEFT:
				if ( pCState->StickX >= -pThis->CenteringX )
					return 0;
				else
					return ((-pCState->StickX) - pThis->CenteringX) * CT_MAX_STICK / (CT_MAX_STICK-pThis->CenteringX);
		
			case CTTYPE_STICK_RIGHT:
				if ( pCState->StickX <= +pThis->CenteringX )
					return 0;
				else
					return (pCState->StickX - pThis->CenteringX) * CT_MAX_STICK / (CT_MAX_STICK-pThis->CenteringX);

			case CTTYPE_STICK_SFORWARD:
				if ( pCState->SStickY <= +pThis->CenteringY )
					return 0;
				else
					return (pCState->SStickY - pThis->CenteringY) * CT_MAX_STICK / (CT_MAX_STICK-pThis->CenteringY);
		
			case CTTYPE_STICK_SBACKWARD:
				if ( pCState->SStickY >= -pThis->CenteringY )
					return 0;
				else
					return ((-pCState->SStickY) - pThis->CenteringY) * CT_MAX_STICK / (CT_MAX_STICK-pThis->CenteringY);
		
			case CTTYPE_STICK_SLEFT:
				if ( pCState->SStickX >= -pThis->CenteringX )
					return 0;
				else
					return ((-pCState->SStickX) - pThis->CenteringX) * CT_MAX_STICK / (CT_MAX_STICK-pThis->CenteringX);
		
			case CTTYPE_STICK_SRIGHT:
				if ( pCState->SStickX <= +pThis->CenteringX )
					return 0;
				else
					return (pCState->SStickX - pThis->CenteringX) * CT_MAX_STICK / (CT_MAX_STICK-pThis->CenteringX);
		}
	}
	return 0;
}		
		




