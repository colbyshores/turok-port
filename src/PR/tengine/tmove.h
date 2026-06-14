// tmove.h
// turok movement header file
//


#ifndef	__TMOVEH
#define	__TMOVEH


#include "romstruc.h"
#include "tcontrol.h"
#include "scaling.h"
#include "defs.h"
#include "ai.h"
#include "tengine.h"


struct CCollisionInfo2_t;

// remove for no camera rotation in roll move
//#define	Z_ROLL

// in combat max timer (secs) - when it runs out turok is not in combat mode
#define	TMOVE_INCOMBAT_TIMER			2.5

// sound timer info for getting hit very quickly
#define	TMOVE_HITSOUND_TIMER_MAX		0.4//0.1
#define	TMOVE_HITSOUND_TIMER_SCREAM	0.5

// turok movement object defines
#define	TMOVE_MAX_RUNSPEED		(2.5*SCALING_FACTOR)			// max movement run  speed
#define	TMOVE_MAX_WALKSPEED		(1.5*SCALING_FACTOR)			// max movement walk speed
#define	TMOVE_MAX_BACKRUNSPEED	(1.9*SCALING_FACTOR)			// max backwards movement run  speed
#define	TMOVE_MAX_BACKWALKSPEED	(1.2*SCALING_FACTOR)			// max backwards movement walk speed
#define	TMOVE_MAX_SIDESTEPSPEED	(2.5*SCALING_FACTOR)			// max sidestep speed
#define	TMOVE_MAX_ROLLSPEED		(3.5*SCALING_FACTOR)			// max roll speed

// movement speeds
#define	TMOVE_RUNFORWARD_SMOOTH	11.0
#define	TMOVE_FORWARD_SMOOTH		6.0
#define	TMOVE_BACKWARD_SMOOTH	4.0
#define	TMOVE_STOP_SMOOTH			2.0
#define	TMOVE_SIDESTEP_SMOOTH	4.0									// side step smooth

// jump forces
#define	TJUMP_UPFORCE_INC					(1.5*SCALING_FACTOR)			// jump force increment on holding jump button
#define	TJUMP_UPFORCE_MAX					(7*SCALING_FACTOR)			// max upwards jump force
#define	TJUMP_ROLL_FORCE					(2*15.0*SCALING_FACTOR)			// rolling upwards force
//#define	TTHEAD_LAND_KNEEBEND				-0.9								// knee bend caused by landing
#define	TJUMP_MULTIPLIER					5.0									// jump multiplier (bigger means jumps go further along ground)
#define	TJUMP_LOOKANG_SMOOTH				5.0									// look angle change smoother
#define	TJUMP_LOOKANG_INC					0.06								// look down angle (bigger number means camera looks down further for a bigger jump)
#define	TJUMP_UPFORCESPD_STRAIGHTEN	5.0									// makes look down on jumping less the faster you go (bigger number means less looking down at speed)
#define	TJUMP_DOUBLEJUMP_FORCE			(3*SCALING_FACTOR)			// double jump force
#define	TJUMP_DOUBLEJUMP_THRESHOLD		2.1								// when double jump can be performed (bigger number means double jump can be performed sooner)
//																						//												 (1 means the double jump can only be performed at the max height of the normal jump)

// turn speeds
#define	TTURN_MAX_SPEED			12									// max turning angle speed
#define	TTURN_SMOOTH				3									// turning smooth
#define	TTURN_STOP_SMOOTH			2									// turning stop smooth
#define	TTURN_ANALOGUE_SCALER	4.5								// turning scaler to slow down turn motion

// turok head rotational movements
#define	TTHEAD_MOVE_TIME			7									// frequency of head bobs (bigger number means smaller frequency)
#define	TTHEAD_YROT_TIME			(TTHEAD_MOVE_TIME*2)			// frequency of head y rotation (must be half frequency of head bob above)
#define	TTHEAD_XROT_SMOOTH		8									// smoothing of head rotation changes from movement to stopping
#define	TTHEAD_SPDCHGX_SMOOTH	8									// speed change x rotation offset
#define	TTHEAD_YROT_SMOOTH		8									// smoothing of head rotation changes from movement to stopping
#define	TTHEAD_ZROT_SMOOTH		2									// smoothing of head rotation changes from movement to stopping
#define	TTHEAD_YPOS_SMOOTH		4									// smoothing of head rotation changes from movement to stopping
#define	TTHEAD_XROT_AMPLITUDE	62									// amplitude of x rotation head bob (bigger number means smaller head bob)
#define	TTHEAD_YROT_AMPLITUDE	54									// amplitude of y rotation head bob (bigger number means smaller head bob)
#define	TTHEAD_ZROT_AMPLITUDE	300								// amplitude of z rotation head bob (bigger number means smaller head bob)
#define	TTHEAD_YPOS_AMPLITUDE	0.7								// amplitude of y position head bob (bigger number means larger head bob)
#define	TTHEAD_HEADSPD_DIP		5									// amount of head dip depending on speed (higher means less dip)
#define	TTHEAD_SPDXROT_TIGHTEN	1.9								// head shakes on x axis less with greater speed (higher means less y rot at higher speed)
#define	TTHEAD_SPDYROT_TIGHTEN	2.8								// head shakes on y axis less with greater speed (higher means less y rot at higher speed)
#define	TTHEAD_SPDZROT_TIGHTEN	3									// head shakes on z axis less with greater speed (higher means less y rot at higher speed)
#define	TTHEAD_SPDYPOS_TIGHTEN	1									// head moves  up & down less with greater speed (higher means less y pos at higher speed)
#define	TTHEAD_HEADBACK_DIP		(-ANGLE_PI/23)					// head back dip angle (used when walking backwards)
#define	TTHEAD_RUN_DIPUP			0.008								// makes head come back up after dipping down from a run (larger number means head comes up faster)
#define	TTHEAD_ZROLL_SMOOTH		5									// head z roll smooth (bigger number means smoother change of angle - slower roll)
#define	TTHEAD_ROLL_COUNT			40									// roll count (larger number causes more rolling)
#define	TTHEAD_YPOS_KNEEBEND		(0.1*SCALING_FACTOR)			// drop in camera pos caused by knee bend (larger number means bigger drop in camera)
#define	TKNEE_STRAIGHTEN_SMOOTH	0.6									// smoothing for a knee bend from landing
#define	TTHEAD_GUNKICKXROT_SMOOTH	2								// gun kick x rotation smoother (larger number means slower/smoother motion)

// turok modes
#define	TMMODE_GROUND				0									// normal movement on ground
#define	TMMODE_GRDROLL				1									// rolling on ground
#define	TMMODE_GRDJUMP				2									// jumping on ground
#define	TMMODE_SWIMSURFACE		3									// in water & swimming across surface
#define	TMMODE_SWIMWATER			4									// swimming underwater
#define	TMMODE_CLIMBING			5									// climbing

// turok weapon modes
#define	TWEAPONMODE_OFF							0
#define	TWEAPONMODE_COMINGON						1
#define	TWEAPONMODE_ON								2
#define	TWEAPONMODE_GOINGOFF						3
#define	TWEAPONMODE_GOINGOFF_LASTBULLET		4
#define	TWEAPONMODE_GOINGTOSTAYOFF				5
#define	TWEAPONMODE_STAYOFF						6

#define	TWEAPON_AMMO				TRUE								// any weapon that needs ammo
#define	TWEAPON_ANY					FALSE								// any weapon


// weapon types
#define	TWEAPON_NONE						0
#define	TWEAPON_KNIFE						1
#define	TWEAPON_TOMAHAWK					2
#define	TWEAPON_TEKBOW						3
#define	TWEAPON_SEMIAUTOMATICPISTOL	4
#define	TWEAPON_ASSAULTRIFLE				5
#define	TWEAPON_MACHINEGUN				6
#define	TWEAPON_RIOTSHOTGUN				7
#define	TWEAPON_AUTOMATICSHOTGUN		8
#define	TWEAPON_MINIGUN					9
#define	TWEAPON_GRENADELAUNCHER			10
#define	TWEAPON_TECHWEAPON1				11
#define	TWEAPON_TECHWEAPON2				12
#define	TWEAPON_ROCKET						13
#define	TWEAPON_SHOCKWAVE					14
#define	TWEAPON_CHRONOSCEPTER			15
#define	TWEAPON_TOTAL						15

// weapon hit points for turok
#define	TWEAPON_BULLET1_HITS					5				// bullet type 1 damage
#define	TWEAPON_PLASMA1_HITS					25				// plasma 1 hit damage
#define	TWEAPON_PLASMA2_HITS					35				// plasma 2 hit damage
#define	TWEAPON_ARROW_HITS					20				// arrow damage
#define	TWEAPON_SHOTGUN_HITS					6				// shotgun blast damage

// weapon hit points for enemies
#define	TWEAPON_ENEMY_BULLET_HITS			1				// enemy bullet damage
#define	TWEAPON_EPLASMA1_HITS				3				// plasma hit damage (enemy)
#define	TWEAPON_EPLASMA2_HITS				3				// plasma hit damage (enemy)
#define  TWEAPON_ACIDSPIT_HITS				6				// acid spitting damage (Sub)
#define	TWEAPON_FIREBALL_HITS				10				// fireball hits
#define	TWEAPON_HULK_BLASTER_HITS			5				// hulk blaster hits
#define	TWEAPON_RAPTOR_PROJECTILE_HITS	3				// raptor projectile hits
#define	TWEAPON_SPELLCAST_HITS				7				// spell caster projectile hits
#define	TWEAPON_ROBOT_BULLET_HITS			4				// robot bullet projectile hits
#define	TWEAPON_ROBOT_ROCKET_HITS			25				// robot rocket projectile hits

// weapon types
#define	TWEAPON_BULLET_TUROK_TYPE			1
#define	TWEAPON_BULLET_ENEMY_TYPE			2
#define	TWEAPON_GRENADE_TYPE					3
#define	TWEAPON_PLASMA1_TUROK_TYPE			4
#define	TWEAPON_PLASMA1_ENEMY_TYPE			5
#define	TWEAPON_ARROW_TYPE					6
#define	TWEAPON_ACIDSPIT_TYPE				7
#define	TWEAPON_BREATH_TYPE					8
#define	TWEAPON_DEATHRAIN_TYPE				9
#define	TWEAPON_ENERGYBLAST_TYPE			10
#define	TWEAPON_EYEFIRE_TYPE					11
#define	TWEAPON_FIREBALL_TYPE				12
#define	TWEAPON_SHOCKWAVE_TYPE				13
#define	TWEAPON_SPELLCAST_TYPE				14
#define	TWEAPON_HULK_BLASTER_TYPE			15
#define	TWEAPON_RAPTOR_PROJECTILE_TYPE	16
#define	TWEAPON_PLASMA2_TUROK_TYPE			17
#define	TWEAPON_ROBOT_BULLET_TYPE			18
#define	TWEAPON_ROBOT_ROCKET_TYPE			19
#define	TWEAPON_PLASMA2_ENEMY_TYPE			20




// turok health modes
#define	TMOVE_HEALTH_ALIVE				1				// turok is alive
#define	TMOVE_HEALTH_DEADSTART			2				// turok has just died
#define	TMOVE_HEALTH_DEADGOING			3				// turok is in death mode
#define	TMOVE_DEATH_TIMER					2.0			// (secs)


// how often bullets are used up in the fast weapons (not one shot weapons)
#define	TWEAPON_MACHINEGUN_USECNT			0.16
#define	TWEAPON_MINIGUN_USECNT				2
#define	TWEAPON_ASSAULTRIFLE_USECNT		200
#define	TWEAPON_RIOTSHOTGUNSHELL_USECNT	200
#define	TWEAPON_TECHWEAPON2_USECNT			200


// z rotation on barrel (degrees)
#define TWEAPON_AUTOMATICSHOTGUN_ZROT		ANGLE_DTOR(4)			// speed at which next shell comes into position
#define TWEAPON_MINIGUN_ZROT					ANGLE_DTOR(30)			// speed at which the barrel rotates
#define TWEAPON_SHOCKWAVE_ZROT				ANGLE_DTOR(-5)			// speed at which the barrel rotates
#define TWEAPON_SHOCKWAVE_ZROTMAX			ANGLE_DTOR(-30)		// max speed at which the barrel rotates

// bow defines
#define	TMOVE_NORMAL_BOWMAX				0.7
#define	TMOVE_SUPER_BOWPOSITION			1.4
#define	TMOVE_SUPER_BOWENDPOSITION		2.15
#define	TMOVE_BOW_MAXVELOCITY			(50*SCALING_FACTOR)
#define	TMOVE_SUPERBOW_MAXVELOCITY		TMOVE_BOW_MAXVELOCITY

// chronoscepter defines
#define	TMOVE_CHRONOSCEPTER_CHARGE_TIME		1.0


// players feet on bottom of sea floor status
#define	TMOVE_LAST_NOTINWATER			1
#define	TMOVE_LAST_UNDERWATER			2
#define	TMOVE_LAST_READYFORFALL			3

// turok movement object structure
typedef struct
{
	float	MoveSpeed,					// move speed + is forward & - is backward
			TurnSpeed,					// turn speed
			ActualTurnSpeed,			// actual turn speed
			MoveCnt,						// move counter
			YRotCnt,						// y rotation counter
			HeadXRot,					// head x rotation offset
			HeadYRot,					// head y rotation offset
			HeadZRot,					// head z rotation offset
			HeadYPos,					// head ypos offset
			SpdChgXOffset,				// speed change x offset
			SpdChgX,						// speed change x offset
			RunTime,						// running time counter - how long we've been running forwards
			SideStepSpeed,				// side step speed + is right & - is left
			HeadZRoll,					// roll on z axis - destination angle
			HeadZRollCnt,				// roll on z axis - counter
			HeadYPosLand,				// ypos offset for landing
			JumpUpForce,				// jump up force
			JumpLookAng,				// jump look angle offset
			GunKickXRot,				// head x rotation caused by gun kick
			WeaponBarrelZRot,			// z rotation on barrel of weapon
			WeaponBarrelZRotInc,		// z rotation increment on barrel of weapon
			HeadYPosCFrame,
			JumpAngle,
			StartJumpAngle,
			DesiredJumpAngle,
			JumpU,
			ActualRotXPlayer,
			ActualRotYPlayer,
			RotXPlayer,
			RotYPlayer,
			RotZPlayer,
			MoveUpSpeed,
			OxygenTank,
			Oxygen,
			Time1,
			Time2,
			SideStepZRot,
			BurstForwardSpeed,
			ActualBurstForwardSpeed,
			ClimbSwingRotX,
			ClimbSwingRotY,
			ClimbSwingRotZ,
			ActualClimbSwingRotX,
			ActualClimbSwingRotY,
			ActualClimbSwingRotZ,
			CurrentDuckOffset,
			ClimbSideStepLookAng,
			BurstTimer,
			SelectWeaponTimer,
			LookUpDownSpeed,
			ActualLookUpDownSpeed,
			SwimBankAngle,
			ActualSwimBankAngle,
			WaterSfxTimer,
			WaterSfxTimerMax,
			BubblesSfxTimer,
			DeathRegionTimer,
			ShockwaveStarted,
			ChronoscepterStarted,
			PrevVelocityY,
			CombatTimer,
			DeathTimer,
			HitSoundTimer,
			BowStarted,
			TremorTimer,
			TremorTimerMax,
			TremorAngle,
			TremorShake,
			TremorZRot,
			AmmoUseCounter,
			MapButtonTimer,
			RealMapOffsetX,
			RealMapOffsetZ,
			MapOffsetX,
			MapOffsetZ,
			WeaponTimer,
			LookAtShover,
			InvincibleTimer;

	int	MaxHealth ;
	BOOL	CheatGiven ;

	// Make sure:
	//	1. Vars are in correct order of appearence
	//	2.	Ammo vars are in the same order as the weapon flags
	// 3. Look both ways before crossing
	int	BulletPool,
			ShotgunPool,
			EnergyPool ;

	int	ExpTekBowAmmo,				// explosive ammo, takes priority over regular ammo.
			ExpShotgunPool ;

	int	KnifeAmmo,
			TomahawkAmmo,
			TekBowAmmo,
			//SemiAutomaticPistolAmmo,		// these are commented out because
			//RiotShotgunAmmo,				// a) to find compile errors, so can use bullet pools
			//AutomaticShotgunAmmo,			// b) they are not needed anymore (unless the pool thing changes!)
			//AssaultRifleAmmo,
			//MachineGunAmmo,
			MiniGunAmmo,
			GrenadeLauncherAmmo,
			//TechWeapon1Ammo,
			RocketAmmo,
			//ShockwaveAmmo,
			TechWeapon2Ammo,
			ChronoscepterAmmo;

	int	ArmorAmount ;					// hit points armor can take before it runs out

	BOOL	KnifeFlag,
			TomahawkFlag,
			TekBowFlag,
			SemiAutomaticPistolFlag,
			RiotShotgunFlag,
			AutomaticShotgunFlag,
			AssaultRifleFlag,
			MachineGunFlag,
			MiniGunFlag,
			GrenadeLauncherFlag,
			TechWeapon1Flag,
			RocketFlag,
			ShockwaveFlag,
			TechWeapon2Flag,
			ChronoscepterFlag;

	BOOL	ArmorFlag ;			// did turok have his ready-brek this morning
	BOOL	BackPackFlag ;		// does turok have one of them styling backpacks all the kids love

	int	LastKeyTexture ;		// stores which key was last picked up for key cinema texture

	int	Level2Keys,			// which parts of which levels keys does turok posess
			Level3Keys,
			Level4Keys,
			Level5Keys,
			Level6Keys,
			Level7Keys,
			Level8Keys ;

	int	Level2Access,			// which parts of which levels keys does turok have access to
			Level3Access,
			Level4Access,
			Level5Access,
			Level6Access,
			Level7Access,
			Level8Access ;

	int	ChronoSceptorPieces ;

	BOOL	SpdChg,				// speed change on / off
			Jumped,				// turok has jumped
			DoubleJump,			// turok has initiated a double jump
			WeaponUsed,			// turok has used his weapon
			WeaponChanged,		// weapon changed flag
			WeaponEmpty,		// weapon empty flag
			MiniGunBarrelSFX,	// sfx for barrel
			RunWalkToggle,		// run/walk toggle mode
			SwimBurstToggle,	// swim slow / burst fast mode
			CannotSwimUp,
			ClimbHand,			// which hand is turok using to climb up left=OFF, right=ON
			ClimbOneHand,		// turok climbs up at least one hand position every press of up
			FiredParticle,
			FiredParticleType,
			FiredProjectileParticle,
			FiredProjectileParticleType,
			ClimbCameraWobble,
			ClimbWeaponRemove,
			JumpAllowed,
			ClimbSideStep,
			CannotJumpFromSurface,
			PrevOnGround,
			CurrentOnGround,
			JustWarped,
			JustEnteredDeathRegion,
			JustEnteredSaveRegion,
			JustEnteredLockRegion,
			JustEnteredTrainingRegion,
			OxygenOut,
			OnLadder,
			InAntiGrav,
			PrevAntiGrav,
			HitSoundTimerScream,
			CannotJumpFromCliffFace,
			BowFlashOccurred,
			FiredBullet1,
			FiredBullet2,
			FiredBullet3,
			FiredShotgunShell,
			MapToggle,
			MapScrolling,
			InLava,
			FireFirstMachineGunBullet,
			AllWeaponsAmmoGiven,
			WeaponTakeOffQuickly,
			JustEnteredPressurePlate,
			Active,					// TRUE if alive and running around
			InSun;

	u8		Mode,						// mode turok is in
			PrevMode,				// previous mode turok was in
			WeaponSelect,			// turok weapon currently selected
			WeaponSelectIndex,	// turok weapon index
			WeaponCurrent,			// turok weapon currently being held
			WeaponCurrentAnim,	// turok weapon current animation desired
			WeaponMode,				// turok weapon mode
			DeathStage;				// death / health mode of turok

	s8		WeaponCount;			// turok weapon counter
										
	int	WeaponFired,			// which weapon was fired
			hSFXBarrel,				// sfx handle
			Bubbles,					// severity of bubbles when player enters underwater
			FeetOnBottomStatus,	// feet on bottom of sea floor status
			WeaponAtStartOfClimb,
			WeaponAtStartOfWaterDive,
			WeaponAtStartOfDuck,
			Tokens,
			Lives,
			WaterFlag,
			hSFXPrevShockwaveCharge,
			hSFXNextShockwaveCharge;

	float	SpiritualTime;

	DWORD	CurrentCheckpoint;

	CVector3		vLastPosition,
					vTremorPos;
	CQuatern		qRealAimFix,
					qAimFix;

	CGameObjectInstance	*pShoverAI,
								*pModifyEnemy;

	BYTE			MusicID,
					AmbientSound;

	float			InMenuTimer ;

	CCollisionInfo2		*pLastCI;

} CTMove;








// external definitions
CTMove	  *CTMove__CTMove							(void);								// initialize turok movement object

void			CTMove__UpdateTurokInstance		(CTMove *pThis, CEngineApp *pApp, CTControl *pCTControl);
void			CTMove__GroundMovement				(CTMove *pThis, CEngineApp *pApp, CTControl *pCTControl);
void			CTMove__GroundRollMovement			(CTMove *pThis, CEngineApp *pApp, CTControl *pCTControl);
void			CTMove__GroundJumpMovement			(CTMove *pThis, CEngineApp *pApp, CTControl *pCTControl);
void			CTMove__GroundJumpNSMovement		(CTMove *pThis, CEngineApp *pApp, CTControl *pCTControl);
void			CTMove__Turn							(CTMove *pThis, CEngineApp *pApp, CTControl *pCTControl);
CVector3		CTMove__ControlSwimMovement		(CTMove *pThis, CEngineApp *pApp, CTControl *pCTControl, CVector3 vDesiredPos, struct CCollisionInfo2_t *pCollInfo, BOOL Underwater);
void			CTMove__SwimSurfaceMovement		(CTMove *pThis, CEngineApp *pApp, CTControl *pCTControl);
void			CTMove__SwimWaterMovement			(CTMove *pThis, CEngineApp *pApp, CTControl *pCTControl);
CVector3		CTMove__ControlMovement				(CTMove *pThis, CEngineApp *pApp, CTControl *pCTControl, CVector3 vDesiredPos);
CVector3		CTMove__ControlSideStep				(CTMove *pThis, CEngineApp *pApp, CTControl *pCTControl, CVector3 vDesiredPos);
CVector3		CTMove__ControlFBward				(CTMove *pThis, CEngineApp *pApp, CTControl *pCTControl, CVector3 vDesiredPos);
void			CTMove__ClimbingMovement			(CTMove *pThis, CEngineApp *pApp, CTControl *pCTControl);
CVector3		CTMove__ControlClimbMovement		(CTMove *pThis, CEngineApp *pApp, CTControl *pCTControl, CVector3 vDesiredPos);
CVector3		CTMove__ControlCliffUpDown			(CTMove *pThis, CEngineApp *pApp, CTControl *pCTControl, CVector3 vDesiredPos);
CVector3		CTMove__ClimbMoveIntoCliff			(CTMove *pThis, CEngineApp *pApp, CTControl *pCTControl, CVector3 vDesiredPos);
void			CTMove__DoDucking						(CTMove *pThis, CEngineApp *pApp);
void			CTMove__UsingMap						(CTMove *pThis, CTControl *pCTControl);
void			CTMove__UpdateMap						(CTMove *pThis, CEngineApp *pApp);
void			CTMove__LoadPlayerWeapon			(CTMove *pThis);
void			CTMove__ResetDrawnWeapons			(CTMove *pThis);
BOOL			CTMove__OnLastWeapon					(CTMove *pThis);

extern	CTMove	CTurokMovement;
extern	void		CTMove__GetWeaponOffset				(CTMove *pThis, CVector3 *pvVec);
extern	BOOL		CTMove__WeaponExist					(CTMove *pThis, int weapon);
extern	BOOL		CTMove__WeaponEmpty					(CTMove *pThis, int weapon, int variation);
extern	BOOL		CTMove__WeaponFull					(CTMove *pThis, int weapon, BOOL explosive) ;
extern	int		CTMove__WeaponAmmoPercentage(CTMove *pThis, int weapon, BOOL explosive) ;
extern	BOOL		CTMove__IsExplosiveAmmo				(CTMove *pThis) ;
extern	void		CTMove__SetWeaponFiredStatus		(CTMove *pThis, BOOL status);
extern	void		CTMove__AddAmmo						(CTMove *pThis, int weapon, int amount) ;
extern	void		CTMove__DecreaseAmmo					(CTMove *pThis, int *pAmt, int nAmt);
extern	void		CTMove__SelectWeapon					(CTMove *pThis, BOOL ammo, BOOL next);
extern	void		CTMove__ShakeScreen					(CTMove *pThis, CEngineApp *pApp, float shake, float ang, BOOL rotstyle);
extern	void		CTMove__SpecialEffect				(CTMove *pThis, int wfired);
extern	void		CTMove__FireBullet					(CTMove *pThis, int WFired);
void					CTMove__FireBulletShell				(CTMove *pThis, int WFired);
extern	void		CTMove__KeelOver						(CTMove *pThis, CEngineApp *pApp, CTControl *pCTControl);
extern	void		CTMove__WeaponBarrelUpdate			(CTMove *pThis, int nWeapon);
extern	void		CTMove__WeaponBarrelFire			(CTMove *pThis, int nWeapon);
extern	int		CTMove__GetAmmoOfSelectedWeapon	(CTMove *pThis);
void					CTMove__LoudNoise						(CTMove *pThis);
void					CTMove__QuietNoise					(CTMove *pThis);
//BOOL					CTMove__IsCompletelyDead			(CTMove *pThis);
BOOL					CTMove__IsInCombat					(CTMove *pThis);
void					CTMove__SetCombatTimer				(CTMove *pThis, float Timer);
void					CTMove__UpdateCombatTimer			(CTMove *pThis);
void					CTMove__DisplayWeaponsIcons		(CTMove *pThis, BOOL Direction);
void					CTMove__UpdateRunWalkToggle		(CTMove *pThis, CTControl *pCTControl);
void					CTMove__UpdateSwimBurstToggle		(CTMove *pThis, CTControl *pCTControl);
int					CTMove__GetCurrentWeapon			(CTMove *pThis);
void					CTMove__DoRotOffsets					(CTMove *pThis, CEngineApp *pApp);
void					CTMove__TiltJumping					(CTMove *pThis);
void					CTMove__TiltNotJumping				(CTMove *pThis);
float					CTMove__GetJumpAngle					(CTMove *pThis);
void					CTMove__StartJump						(CTMove *pThis);
void					CTMove__StopJumping					(CTMove *pThis);
void					CTMove__JumpIncrement				(CTMove *pThis);
void					CTMove__ClearHeadRotations			(CTMove *pThis);
void					CTMove__UnderwaterOxygen			(CTMove *pThis, CEngineApp *pApp);
void					CTMove__InitBubbles					(CTMove *pThis, CEngineApp *pApp);
void					CTMove__DoSplashBubbles				(CTMove *pThis, CEngineApp *pApp);
void					CTMove__DoSplash						(CTMove *pThis, CEngineApp *pApp, BOOL DoSound);
void					CTMove__DoRipple						(CTMove *pThis, CEngineApp *pApp);
BOOL					CTMove__IsPlayerUpsideDown			(CTMove *pThis);
void					CTMove__SetClimbCamera				(CTMove *pThis, CEngineApp *pApp, CTControl *pCTControl);
void					CTMove__UpdateClimbCameraRot		(CTMove *pThis, CEngineApp *pApp);
BOOL					CTMove__SelectWeaponByAIType		(CTMove *pThis, int nType, BOOL Reselect);
void					CTMove__RemoveWeaponFromScreen	(CTMove *pThis, CEngineApp *pApp);
int					CTMove__GetWeaponAIType				(CTMove *pThis);
CVector3				CTMove__ControlClimbSideStep		(CTMove *pThis, CEngineApp *pApp, CTControl *pCTControl, CVector3 vDesiredPos);
void					CTMove__TurnOnXAxis					(CTMove *pThis, CEngineApp *pApp, CTControl *pCTControl);
BOOL					CTMove__TurokHasAnyWeaponInList	(CTMove *pThis, INT16 *pWeaponList);
void					CTMove__UpdateWeaponPosition		(CTMove *pThis);
BOOL					CTMove__DoesWeaponAutoAim			(CTMove *pThis, int WFired);
float					CTMove__GetOxygenStatus				(CTMove *pThis);
BOOL					CTMove__InAntiGrav					(CTMove *pThis);
BOOL					CTMove__OnLadder						(CTMove *pThis);
float					CTMove__GetSpeed						(CTMove *pThis);
void					CTMove__UpdateDeathTimer			(CTMove *pThis);
BOOL					CTMove__DecreaseLives				(CTMove *pThis);
void					CTMove__IncreaseLives				(CTMove *pThis, int nLives);
void					CTMove__NewLifeSetup					(CTMove *pThis);
void					CTMove__DoTremorEffectFull			(CTMove *pThis, CVector3 vPos, float shake, float ang, float time);
void					CTMove__DoTremorEffectQuick		(CTMove *pThis, CVector3 vPos, float time);
void					CTMove__UpdateTremorEffect			(CTMove *pThis);
BOOL					CTMove__IsClimbing					(CTMove *pThis);
BOOL					CTMove__ClearWeaponZBuffer			(CTMove *pThis);
int					CTMove__ModifyEnemy					(CTMove *pThis, CEngineApp *pApp);
void					CTMove__ChangeEnemy					(CTMove *pThis, CEngineApp *pApp, BOOL Direction);
void					CTMove__MoveEnemy						(CTMove *pThis, CEngineApp *pApp, CTControl *pCTControl);
void					CTMove__ResetEnemy					(CTMove *pThis);
void					CTMove__DoGrdHeadMovements			(CTMove *pThis);

#endif		// __TMOVEH


