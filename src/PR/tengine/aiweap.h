// weapon ai header file
//



#ifndef __AIWEAP_H
#define __AIWEAP_H


#include "ai.h"
#include "aistand.h"

#ifdef WIN32
#include "..\objinst.h"
#endif

#ifdef WIN32
#include "..\romstruc.h"
#include "..\scaling.h"
#else
#include "romstruc.h"
#include "tengine.h"
#include "scaling.h"
#endif

#ifdef WIN32
#define ANGLE_PI			3.14159265358979323846
#define ANGLE_DTOR(deg)	(deg*(ANGLE_PI/180))
#endif


#ifndef WIN32
#include "tmove.h"
#endif











// external declarations
void AI_Update_Weapon_Movements				(CGameObjectInstance *pMe, CTControl *pCTControl, CTMove *pCTMove, int waterFlag);
BOOL AI_Update_Turok_Weapon					(CGameObjectInstance *pMe);
BOOL AI_Update_Weapon_Knife					(CGameObjectInstance *pMe, CTControl *pCTControl, CTMove *pCTMove, int waterFlag);
BOOL AI_Update_Weapon_Tomahawk				(CGameObjectInstance *pMe, CTControl *pCTControl, CTMove *pCTMove, int waterFlag);
BOOL AI_Update_Weapon_TekBow					(CGameObjectInstance *pMe, CTControl *pCTControl, CTMove *pCTMove, int waterFlag);
BOOL AI_Update_Weapon_SemiAutomaticPistol	(CGameObjectInstance *pMe, CTControl *pCTControl, CTMove *pCTMove, int waterFlag);
BOOL AI_Update_Weapon_AssaultRifle			(CGameObjectInstance *pMe, CTControl *pCTControl, CTMove *pCTMove, int waterFlag);
BOOL AI_Update_Weapon_MachineGun				(CGameObjectInstance *pMe, CTControl *pCTControl, CTMove *pCTMove, int waterFlag);
BOOL AI_Update_Weapon_RiotShotgun			(CGameObjectInstance *pMe, CTControl *pCTControl, CTMove *pCTMove, int waterFlag);
BOOL AI_Update_Weapon_AutomaticShotgun		(CGameObjectInstance *pMe, CTControl *pCTControl, CTMove *pCTMove, int waterFlag);
BOOL AI_Update_Weapon_MiniGun					(CGameObjectInstance *pMe, CTControl *pCTControl, CTMove *pCTMove, int waterFlag);
BOOL AI_Update_Weapon_Grenade_Launcher		(CGameObjectInstance *pMe, CTControl *pCTControl, CTMove *pCTMove, int waterFlag);
BOOL AI_Update_Weapon_TechWeapon1			(CGameObjectInstance *pMe, CTControl *pCTControl, CTMove *pCTMove, int waterFlag);
BOOL AI_Update_Weapon_TechWeapon2			(CGameObjectInstance *pMe, CTControl *pCTControl, CTMove *pCTMove, int waterFlag);
BOOL AI_Update_Weapon_Chronoscepter			(CGameObjectInstance *pMe, CTControl *pCTControl, CTMove *pCTMove, int waterFlag);
void AI_Reset_Turok_Weapon						(CGameObjectInstance *pAI);
void AI_Kill_Weapon_SFX							(CTMove *pThis);


#endif	// __AIWEAP_H



