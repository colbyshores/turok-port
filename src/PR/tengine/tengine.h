// tengine.h
/////////////////////////////////////////////////////////////////////////////

#ifndef _INC_TENGINE
#define _INC_TENGINE

#include <stddef.h>
#include "spec.h"
#include "defs.h"
#include "scene.h"
#include "graphu64.h"
#include "romstruc.h"
#include "onscrn.h"
#include "volume.h"
//#include "frontend.h"
#include "pause.h"
#include "boss.h"
#include "fx.h"
#include "options.h"
#include <sched.h>
#include "camera.h"
#include "sun.h"
#include "train.h"

#define THREAD_RMON				0
#define THREAD_IDLE				1
#define THREAD_AUDIO				3
#define THREAD_SCHED				4
#define THREAD_FAULT				5
#define THREAD_MAIN				6

#define PRIORITY_IDLEINIT		10
#define PRIORITY_IDLE			OS_PRIORITY_IDLE
#define PRIORITY_MAIN			10
#define PRIORITY_AUDIO			12
#define PRIORITY_AUDIOLOCK		13
#define PRIORITY_SCHEDULER		14
#define PRIORITY_SCHEDLOCK		15

#define LOG_SCHEDULE_GFX_TASK	101
#define LOG_RDP_DONE				102
#define LOG_RETRACE				103
#define LOG_INTR					104

// water states
#define	PLAYER_NOT_NEAR_WATER		0
#define	PLAYER_BELOW_WATERSURFACE	1
#define	PLAYER_ABOVE_WATERSURFACE	2
#define	PLAYER_ON_WATERSURFACE		3

// climbing states
#define	PLAYER_NOT_NEAR_CLIMBING_SURFACE		0
#define	PLAYER_AT_CLIMBING_SURFACE				1

// ducking states
#define	PLAYER_NOT_DUCKING				0
#define	PLAYER_DUCKING						1
#define	PLAYER_DUCKING_ENTRANCE_EXIT	2


#define CONTROLLER_MSG			(OS_SC_LAST_MSG + 1)
#define MSG_FAULT					0x10

#define DMA_QUEUE_SIZE			200
#define MAX_MESGS					64

#define NUM_FIELDS				1

typedef union
{
	struct
	{
		short	type;
	} gen;
	struct
	{
		short	type;
	} done;
	OSScMsg	app;
} GFXMsg;

typedef struct CFrameData_t
{
	Mtx					m_mProjection;
	LookAt				m_LookAt;
	CSunFrameData		m_SunFrame;
	OSScTask				m_Task,
							m_LineTask;
	GFXMsg				m_Msg;
	void					*m_pFrameBuffer;
	Gfx					*m_pDisplayList,
							*m_pLineList;
	DWORD					m_DisplayListSize,
							m_LineListSize,
							m_nPredictFields;
	BOOL					m_DrawMap;
} CFrameData;

enum CodeSections { FRONTEND_CODE_SECTION, GAME_CODE_SECTION, END_OF_CODE_SECTIONS };

enum MainModes
{
	MODE_RESETGAME,
	MODE_LEGALSCREEN,

	MODE_INTRO,
	MODE_TITLE,

//	MODE_FRONTEND_START,
//	MODE_FRONTEND_START2,
//	MODE_FRONTEND_TITLE,

//	MODE_GAME_START,
	MODE_GAME,
	MODE_GAMEOVER,

	MODE_RESETLEVEL,

	MODE_WAITFORDISPLAY,

	MODE_CREDITS,

	MODE_GAMEPAK,


	MODE_GALLERY
};


// Special warp id's
#define	INTRO_ACCLAIM_LOGO_WARP_ID			32000
#define	INTRO_IGGY_WARP_ID					32001
#define	INTRO_TUROK_DRAWING_BOW_WARP_ID	32002
#define	INTRO_ZOOM_TO_LOGO_WARP_ID			32003
#define	LEGALSCREEN_WARP_ID					32004

#define	GAME_INTRO_WARP_ID					32100
#define	GAME_INTRO_KEY_WARP_ID				32101

#define	CHEAT_GALLERY_WARP_ID				32200

#define	END_SEQUENCE_WARP_ID					32300

#define	CREDITS_WARP_ID						32400



enum FadeModes
{
	FADE_NULL,
	FADE_UP,
	FADE_DOWN
} ;

enum LevelStatuses
{
	LEVEL_RESET,
	LEVEL_CHANGE
} ;

/////////////////////////////////////////////////////////////////////////////

typedef struct CPersistantData_t
{
	BOOL	SemiAutomaticPistolFlag,	// weapons owned flags
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
			ChronoSceptorFlag ;

	BOOL	ArmorFlag,						// miscellaneous inventory flags
			BackPackFlag ;

	int	BulletPool,						// weapons ammo
			ShotgunPool,
			EnergyPool,
			ExpTekBowAmmo,
			ExpShotgunPool,
			TekBowAmmo,
			MiniGunAmmo,
			GrenadeLauncherAmmo,
			RocketAmmo,
			TechWeapon2Ammo,
			ChronoSceptorAmmo;

	int	ArmorAmount ;

	int	WeaponUsing ;

	int	Level2Keys,
			Level3Keys,
			Level4Keys,
			Level5Keys,
			Level6Keys,
			Level7Keys,
			Level8Keys ;
	int	Level2Access,
			Level3Access,
			Level4Access,
			Level5Access,
			Level6Access,
			Level7Access,
			Level8Access ;
	int	ChronoSceptorPieces ;

	int	Difficulty ;


	BOOL	RunWalk ;						// TRUE for RUN
	BOOL	ControlSide ;					// TRUE for right handed
	BOOL	Blood ;							// TRUE for blood is on
	BOOL	Stereo ;							// TRUE for stereo
	u32	MusicVolume ;
	u32	SFXVolume ;
	u32	Opacity ;
	u32	HAnalog ;
	u32	VAnalog ;
	u32	CheatFlags ;
	u32	EnabledCheatFlags ;

	int	Health,
			MaxHealth,
			Tokens,
			Lives ;

	u16	Checkpoint ;

	float	BestTrainTime ;

	INT32	BossFlags ;

	u32	Crash ;

} CPersistantData;

/////////////////////////////////////////////////////////////////////////////

// Main applicaiton object (global in tengine.cpp)
/////////////////////////////////////////////////////////////////////////////

typedef struct CEngineApp_t
{
	// Frame buffer vars
	CFrameData			m_FrameData[3];
	CFrameData			*m_pCurrentFrameData;

	CPersistantData	*m_pPersistant;

	CScene				m_Scene;
	COnScreen			m_OnScreen;

	CBossesStatus			m_BossesStatus ;	// Used to save bosses status for cinema's etc
	INT32						m_BossFlags ;		// Keeps track of all boss status's
	CGameObjectInstance	*m_pBossActive ;	// (used to be a BOOL), now knows which boss is active
	CBossVars				m_BossVars ;

	CCamera				m_Camera;
	CFxSystem			m_FxSystem ;
//	CFrontEnd			m_FrontEnd;
	CPause				m_Pause;
	COptions				m_Options;
	CKeys					m_Keys;
	CLoad					m_Load;
	CSave					m_Save;
	CRequestor			m_Requestor;
	CMessageBox			m_MessageBox;
	CCheatCode 			m_CheatCode;
	CGiveCheatCode 	m_GiveCheatCode;
	CCheat				m_Cheat;
	CGInst				m_GInst;
	CGamePak				m_GamePak;
	DWORD					m_Mode, m_NextMode ;

	DWORD					m_dwCheatFlags,						// cheat flags
							m_dwEnabledCheatFlags;				// cheat flags
	int					m_Level ;
	int					m_BossLevel ;
	//						m_LevelStatus ;
	INT32					m_hMusic;

	u8						m_FadeFast, m_FadeStatus, unused1, unused2;
	BYTE					m_PortraitNameMode;

	float					m_FadeAlpha ;

	float  				m_RotY,
							m_RotXOffset, m_RotYOffset, m_RotZOffset,
							m_XPos, m_YPos, m_ZPos,
							m_YPosOffset,
							m_TranslateHorizontal,
							m_TranslateVertical;
	CQuatern				m_qGround,
							m_qViewAlign,
							m_qViewAlignNoZ;
	CMtxF					m_mfViewOrient,
							m_mfProjection;
	CVector3				m_vTCorners[5];

	Gfx					*m_pDLP;						// display list position
	void					*m_pStaticSegment;
	BOOL					m_DebugMode,
							m_DoIdle,
							m_SloMo,
							m_LegalBypass,
							m_GamePakCheck,
							m_FirstRun,
							m_JustLoadedGame,
							m_MapMode ;

	int					m_LegalTimer ;

	float					m_MapAlpha ;
	u16					m_MapStatus ;

	float					m_GameOverAlpha ,
							m_GameOverMode,
							m_GameOverTime ;

	float					m_TitleAlpha ;


	float					m_LevelTime ;
	float					m_BestTrainTime ;

	FLOAT					m_ModeTime ;

	BOOL					m_bTrainingMode ;					// TRUE if in Training mode...
	BOOL					m_bTraining ;					// text overlay
	int					m_TrainingType ;				// (0)Normal (1)Timed
	CTraining			m_Training ;


#define			EASY					0
#define			MEDIUM				1
#define			HARD					2
	int					m_Difficulty ;

	BOOL					m_bGameOver,
							m_bReset,
							m_bPause,
							m_bOptions,
							m_bKeys,
							m_bLoad,
							m_bSave,
							m_bRequestor,
							m_bMessageBox,
							m_bCheatCode,
							m_bGiveCheatCode,
							m_bCheat,
							m_bGInst,
							m_bGamePak,
							m_bDebug,
							m_UnderWater;
	u8						m_Flash,
							m_FlashDec,
							m_Alpha,
							m_AlphaDec;

	DWORD					m_nCurrentRegionSet;
	float					m_cRegionSetBlend;
	BYTE					m_FogColor[4],
							m_LastFogColor[4],
							m_FlashColor[4];
	float					m_FarClip,
							m_LastFarClip,
							m_FieldOfView,
							m_LastFieldOfView,
							m_SkySpeed,
							m_SkyElevation,
							m_LastSkySpeed,
							m_LastSkyElevation;
	DWORD					m_FogStart,
							m_LastFogStart;

#define RETURN_WARP_ID	-1
#define CINEMA_WARP_ID	-2
#define CRASH_WARP_ID	-3

#define WARP_NOT_WARPING		0
#define WARP_WARPING_OUT		1
#define WARP_WARPING_IN			2
	int					m_Warp,
							m_WarpID,
							m_CurrentWarpID,
							m_WarpSound;
	int					m_WarpBase ;
	BOOL					m_PlayedSound;
	float					m_uWarp,
							m_WarpLength;
	BYTE					m_WarpColors[4];

#define DEATH_NOT_DIEING	0
#define DEATH_DIEING			1
#define DEATH_WAITING		2
	int					m_Death;
	float					m_uDeath;

	BOOL					m_ReturnWarpSaved;
	CROMWarpPoint		m_ReturnWarp;

#define CINEMA_FLAG_PLAY_DEATH			(1<<0)
#define CINEMA_FLAG_PLAY_WATER_DEATH	(1<<1)
#define CINEMA_FLAG_PLAY_FALL_DEATH		(1<<2)
#define CINEMA_FLAG_PLAY_RESURRECT		(1<<3)
#define CINEMA_FLAG_PLAY_LONGHUNTER		(1<<4)
#define CINEMA_FLAG_PLAY_FINAL_HUMVEE	(1<<5)
#define CINEMA_FLAG_TUROK_KILL_MANTIS	(1<<6)
#define CINEMA_FLAG_MANTIS_KILL_TUROK	(1<<7)
#define CINEMA_FLAG_TUROK_KILL_TREX	 	(1<<8)
#define CINEMA_FLAG_TREX_KILL_TUROK	 	(1<<9)
#define CINEMA_FLAG_TUROK_KILL_LONGHUNTER	 	(1<<10)
#define CINEMA_FLAG_TUROK_KILL_CAMPAIGNER	 	(1<<11)
#define CINEMA_FLAG_CAMPAIGNER_KILL_TUROK	 	(1<<12)
#define CINEMA_FLAG_TUROK_PICKUP_KEY		 	(1<<13)
#define CINEMA_FLAG_END_SEQUENCE			 		(1<<14)
#define CINEMA_FLAG_END_SEQUENCE2		 		(1<<15)
#define CINEMA_FLAG_CREDIT					 		(1<<16)
#define CINEMA_FLAG_TUROK_VICTORY		 		(1<<17)

	INT32					m_CinemaFlags ;
	CROMWarpPoint		m_CinemaWarp ;
	BOOL					m_UseCinemaWarp ;

	float						m_PortraitZoom,
								m_ActualPortraitZoom,
								m_ActualPortraitRotXSpd,
								m_ActualPortraitRotYSpd,
								m_ActualPortraitRotZSpd,
								m_PortraitRotXSpd,
								m_PortraitRotYSpd,
								m_PortraitRotZSpd,
								m_PortraitAlpha,
								m_PortraitTimer;
	CVector3					m_vPortrait;
	int						m_Portrait,
								m_PortraitName;
	CQuatern					m_qPortraitOrientation;
	BOOL						m_PortraitExit;

	int						m_LastSaveMapLevel;
	float						m_MenuUpChangeCount,
								m_MenuDownChangeCount,
								m_MenuLeftChangeCount,
								m_MenuRightChangeCount;

	float						m_ActualMusicVolume,
								m_ActualSFXVolume,
								m_MusicVolume,
								m_SFXVolume;

	// Attract mode vars
	BOOL					m_RecordMode ;				// Record joy pad?
	BOOL					m_AttractMode ;			// Use recorded values

	UINT8					*m_pAttractData ;			// Ptr to current pad data
	INT32					m_AttractCurrentBit ; 	// Current record bit

	UINT8					m_AttractPadFrames ;		// Pad attract frame count
	OSContPad			m_AttractPad ;				// Current attract pad values


} CEngineApp;

// CEngineApp operations
/////////////////////////////////////////////////////////////////////////////

// thread entry points
void 			CEngineApp__Boot(CEngineApp *pThis);
void 			CEngineApp__Idle(CEngineApp *pThis, void *pArg);
void 			CEngineApp__Main(CEngineApp *pThis);
void			CEngineApp__ClearFrameBuffer(CEngineApp *pThis) ;

void 			CEngineApp__Construct(CEngineApp *pThis);

#define		CEngineApp__GetFrameData(pThis) ((pThis)->m_pCurrentFrameData)
#define		CEngineApp__GetSunFrameData(pThis) (&(pThis)->m_pCurrentFrameData->m_SunFrame)
#define		CEngineApp__GetStaticAddress(pThis) ((pThis)->m_pStaticSegment)
void			CEngineApp__ClearZBuffer(CEngineApp *pThis, int sx, int ex, int sy, int ey);


void			CEngineApp__DrawWater(CEngineApp *pThis);

void			CEngineApp__DoFlash(CEngineApp *pThis, u8 Flash, u8 FlashDec);
void			CEngineApp__DoColorFlash(CEngineApp *pThis, u8 Flash, u8 FlashDec, int r, int g, int b);
void			CEngineApp__DrawTint(CEngineApp *pThis, BYTE Color[4]);

#define		CEngineApp__GetInstance(pThis, nInst) ((pThis)->m_Scene.m_pInstances[nInst])
#define		CEngineApp__GetAllInstanceCount(pThis) ((pThis)->m_Scene.m_nInstances)

#define		CEngineApp__GetPlayer(pThis) ((pThis)->m_Scene.m_pPlayer)

#define		CEngineApp__GetAnimInstance(pThis, nInst) ((pThis)->m_Scene.m_pActiveAnimInstances[nInst])
#define		CEngineApp__GetAnimInstanceCount(pThis) ((pThis)->m_Scene.m_nActiveAnimInstances)

int			CEngineApp__GetPlayerWaterStatus(CEngineApp *pThis);
float			CEngineApp__GetPlayerWaterDepth(CEngineApp *pThis);
BOOL			CEngineApp__PlayerInShallowWater(CEngineApp *pThis);
//float			CEngineApp__GetPlayerWaterFloatForce(CEngineApp *pThis);
float			CEngineApp__GetWaterDepthWherePlayerIs(CEngineApp *pThis);
float			CEngineApp__GetGroundHeightWherePlayerIs(CEngineApp *pThis);
#define		CEngineApp__GetEyePos(pThis) ((pThis)->m_vTCorners[0])
int			CEngineApp__GetPlayerClimbStatus(CEngineApp *pThis);
float			CEngineApp__GetPlayerHeightFromBottomOfCliff(CEngineApp *pThis);
float			CEngineApp__GetPlayerHeightFromTopOfCliff(CEngineApp *pThis);
int			CEngineApp__GetPlayerDuckStatus(CEngineApp *pThis);
//void			CEngineApp__DoWarpReturn(CEngineApp *pThis);
//void			CEngineApp__DoWarp(CEngineApp *pThis, int WarpID);
void			CEngineApp__DoDeath(CEngineApp *pThis);
void			CEngineApp__GetScreenCoordinates(CEngineApp *pThis, CVector3 *pvIn, float *pX, float *pY);

#define WARP_WITHINLEVEL		0
#define WARP_BETWEENLEVELS		1
void			CEngineApp__Warp(CEngineApp *pThis, int WarpID, int nType, BOOL StoreWarpReturn);
void			CEngineApp__WarpReturn(CEngineApp *pThis);

BOOL			CEngineApp__MenuDown(CEngineApp *pThis);
BOOL			CEngineApp__MenuUp(CEngineApp *pThis);
BOOL			CEngineApp__MenuLeft(CEngineApp *pThis);
BOOL			CEngineApp__MenuRight(CEngineApp *pThis);

float			CEngineApp__GetEnemySpeedScaler(CEngineApp *pThis);
float			CEngineApp__GetBossHitPointScaler(CEngineApp *pThis) ;

void			CEngineApp__ResetBossVars(CEngineApp *pThis);

// global matrices
/////////////////////////////////////////////////////////////////////////////

// floating point matrices
extern CMtxF	mf_3dstudio_to_u64;

// fixed point matrices
extern Mtx		mtx_identity;

// for shadows
extern Vtx		shadow_vtxs[4];

// global vars
extern BOOL			cntrlReadInProg, cntrlReadYet,
						cntrlStart;
extern GFXMsg		*pMsg;
extern int			wait, joyReadCount, nWaitBlack;
extern CFrameData	*pFrameData;
extern int			nDisplayLists,
						nNextFields;


// Functions with global scope
/////////////////////////////////////////////////////////////////////////////

//CEngineApp* GetApp();

#define GetApp() (&engine_app)
#define GetCache() (&engine_app.m_Scene.m_Cache)

// Framebuffers
/////////////////////////////////////////////////////////////////////////////
// RSP address for the color frame buffer
extern unsigned short 	cfb_16_a[];
extern unsigned short 	cfb_16_b[];
extern unsigned short 	cfb_16_c[];
extern unsigned short 	zbuffer[];
extern Gfx					display_list_a[];
extern Gfx					display_list_b[];
extern Gfx					line_list_a[];
extern Gfx					line_list_b[];
//extern void     			*cfb_ptrs[2];

extern u64					fifo_buffer[];
extern u64					gfx_yield_buf[];

extern u64					dram_stack[];
extern u64					boot_stack[];
extern u64					schedule_stack[];
extern u64					idle_thread_stack[];

#define						fault_thread_stack		boot_stack
//extern u64					fault_thread_stack[];

extern u64					main_thread_stack[];
extern u64					audio_stack[];
#ifndef MAKE_CART
extern u64					rmon_stack[];
#endif


// Memory pool for dynamic allocations
/////////////////////////////////////////////////////////////////////////////

// 1448k
#define MEMORY_POOL_SIZE 	0x16A000
extern u32 dynamic_memory_pool[];

// message queue
/////////////////////////////////////////////////////////////////////////////

extern OSMesgQueue	dmaMessageQ;
extern OSIoMesg		dmaIOMessageBuf;
extern OSMesgQueue	romMessageQ;
extern OSMesg			romMessageBuf;
extern OSIoMesg		romIOMessageBuf;
extern OSMesgQueue	gfxFrameMsgQ;
extern OSMesg			gfxFrameMsgBuf[MAX_MESGS];

// frame info
/////////////////////////////////////////////////////////////////////////////

extern DWORD			frame_number;
extern DWORD			game_frame_number;
extern DWORD			even_odd;

#define MICROCODE_HIGH_PRECISION		0
#define MICROCODE_LOW_PRECISION		1
extern DWORD			microcode_version;

extern DWORD			c_field;
extern DWORD			n_fields;
extern DWORD			n_l_fields;
extern DWORD			n_ll_fields;
extern DWORD			n_lll_fields;
extern DWORD			c_waiting;
extern DWORD			n_waiting;
extern DWORD			c_finished;
extern DWORD			n_pending;
extern CFrameData		*p_waiting_frames[3];
extern BOOL				allow_task_sends;
extern void				*p_current_fb;
extern void				*p_pending_fb;

extern float			frame_increment;
extern float			enemy_speed_scaler;
extern float			particle_speed_scaler;
extern float			sky_speed_scaler;
extern float			old_enemy_speed_scaler;
extern float			old_particle_speed_scaler;
extern float			old_sky_speed_scaler;

extern float			weapon_z_rotation;

extern float			rsp_total_time;
extern float			rsp_audio_time;
extern float			rsp_graphics_time;
extern float			rdp_time;
extern float			cpu_time;

extern float			refresh_rate;

//extern ROMAddress		*rp_current_textureset;
//extern DWORD			current_texture;

#define RENDERMODE_CLEAR			0
#define RENDERMODE_INTER			1
#define RENDERMODE_CUTOUT			2
#define RENDERMODE_CLOUD			3
#define RENDERMODE_CLOUD_NO_Z		4
#define RENDERMODE_CLOUD_SPARKLE	5
#define RENDERMODE_XLU				6
#define RENDERMODE_XLU_ZUPD		7
#define RENDERMODE_SURF				8
#define RENDERMODE_DECAL			9

extern BOOL				player_is_moving;
extern BOOL				cache_is_valid;

extern CViewVolume	view_volume;
extern CBoundsRect	view_bounds_rect,
							anim_bounds_rect;

extern BYTE				*scratch_area;
extern int				scratch_area_size;

extern	int			Controller;		// lowest numbered controller connected to system
extern	CTControl	*pCTControl;	// pointer to turok controller data

extern	CEngineApp	engine_app;

// dummy new and delete operators so linker will be happy
/////////////////////////////////////////////////////////////////////////////
//void* operator new(size_t nSize);
//void operator delete(void* pAddress);


/////////////////////////////////////////////////////////////////////////////
// Fuunctions for external code
/////////////////////////////////////////////////////////////////////////////
void			CEngineApp__SetupFadeTo(CEngineApp *pThis, DWORD Mode) ;
void			CEngineApp__Reset(CEngineApp *pThis, DWORD NewMode);
void			CEngineApp__NewLife(CEngineApp *pThis) ;


void			scSendCommand(OSSched *sc, OSScTask *pTask);

/////////////////////////////////////////////////////////////////////////////
#endif // _INC_TENGINE
