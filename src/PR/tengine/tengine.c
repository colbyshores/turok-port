// tengine.c

#include "cppu64.h"
#include "os_internal.h"

#ifndef MAKE_CART
#include <PR/ramrom.h>	// needed for argument passing into the app
#endif

#include "tengine.h"
#include "dlists.h"
#include "scaling.h"
#include "particle.h"
#include "mattable.h"
#include "tcontrol.h"
#include "tmove.h"
#include "audio.h"
#include "sfx.h"
#include "textload.h"
#include "regtype.h"
#include "pickup.h"
#include "attract.h"
#include "map.h"
#include "collinfo.h"
#include "onscrn.h"

#include "introcam.h"
#include "galrecam.h"
#include "cammodes.h"
#include "bossflgs.h"
#include "persist.h"

extern	int	WeaponOrder[] ;
extern	int	RequestorMode ;

extern	BOOL persistant_data_loaded ;

extern char	text_errorcontroller[] ;
extern char	text_pakdamaged[] ;

//--------------------------------------------------------------------------------------------------
// Text
//--------------------------------------------------------------------------------------------------
#ifdef ENGLISH
// ---------------------- English text ---------------------------
static char	text_killerfish[] = {"killer fish"};
static char	text_beetle[] = {"giant beetle"};
static char	text_dragonfly[] = {"giant dragonfly"};
static char	text_campaignerssoldier[] = {"campaigners soldier"};
static char	text_campaignersseargent[] = {"campaigners sergeant"};
static char	text_poacher[] = {"poacher"};
static char	text_dimetrodon[] = {"dimetrodon"};
static char	text_raptor[] = {"raptor"};
static char	text_cyborgraptor[] = {"raptor mech"};
static char	text_leaper[] = {"leaper"};
static char	text_hulk[] = {"purlin"};
static char	text_alieninfantry[] = {"alien infantry"};
static char	text_moschops[] = {"moschops"};
static char	text_ancientwarrior[] = {"ancient warrior"};
static char	text_highpriest[] = {"high priest"};
static char	text_cyborg[] = {"cyborg"};
static char	text_demon[] = {"demon"};
static char	text_demonlord[] = {"demon lord"};
static char	text_pteranodon[] = {"pteranodon"};
static char	text_sludgebeast[] = {"sludge beast"};
static char	text_killerplant[] = {"killer plant"};
static char	text_robot[] = {"attack robot"};
static char	text_triceratops[] = {"triceratops"};
static char	text_subterranean[] = {"subterranean"};

static char	text_quittraining[] = {"quit training"} ;
static char	text_areyousure[] = {"are you sure"} ;
#endif


#ifdef GERMAN
// ---------------------- German text ---------------------------
static char	text_killerfish[] = {"killerfisch"};
static char	text_beetle[] = {"riesenk<fer"};	//a.
static char	text_dragonfly[] = {"riesenlibelle"};
static char	text_campaignerssoldier[] = {"campaigner soldat"};
static char	text_campaignersseargent[] = {"campaigner hauptmann"};
static char	text_poacher[] = {"wilddied"};
static char	text_dimetrodon[] = {"dimetrodon"};
static char	text_raptor[] = {"raptor"};
static char	text_cyborgraptor[] = {"mech raptor"};
static char	text_leaper[] = {"springer"};
static char	text_hulk[] = {"riese"};
static char	text_alieninfantry[] = {"ausserirdische infanterie"};
static char	text_moschops[] = {"moschops"};
static char	text_ancientwarrior[] = {"krieger"};
static char	text_highpriest[] = {"hoher priester"};
static char	text_cyborg[] = {"cyborg"};
static char	text_demon[] = {"d<mon"};		//a.
static char	text_demonlord[] = {"d<mon anf/hrer"};	//a.u.
static char	text_pteranodon[] = {"pterandon"};
static char	text_sludgebeast[] = {"schlammungeheuer"};
static char	text_killerplant[] = {"killerpflanze"};
static char	text_robot[] = {"kampfroboter"};
static char	text_triceratops[] = {"trizeratops"};
static char	text_subterranean[] = {"unterirdischer"};

static char	text_quittraining[] = {"training beenden"} ;
static char	text_areyousure[] = {"bist du sicher"} ;
#endif


#ifdef KANJI
// ---------------------- Japanese text ---------------------------
static char	text_killerfish[] = {0x35, 0x21, 0x15, 0x00, -1};
static char	text_beetle[] = {0x25, 0x41, 0x10, -1};
static char	text_dragonfly[] = {0x2e, 0x0c, 0x19, 0x21, 0x01, -1};
static char	text_campaignerssoldier[] = {0x0e, 0x23, 0x2c, 0x3d, 0x41, -1};
static char	text_campaignersseargent[] = {0x29, 0x27, 0x0e, 0x02, -1};
static char	text_poacher[] = {0x30, 0x41, 0x30, 0x22, 0x00, 0x27, -1};
static char	text_dimetrodon[] = {0x0c, 0x34, 0x01, 0x05, 0x41, -1};
static char	text_raptor[] = {0x21, 0x36, 0x0f, 0x41, -1};
static char	text_cyborgraptor[] = {0x30, 0x01, 0x04, 0x21, 0x36, 0x0f, 0x41, -1};
static char	text_leaper[] = {0x22, 0x2b, 0x41, 0x2f, -1};
static char	text_hulk[] = {0x31, 0x41, 0x0c, 0x13, -1};
static char	text_alieninfantry[] = {0x06, 0x21, 0x41, 0x00, 0x27, 0x13, -1};
static char	text_moschops[] = {0x1b, 0x40, 0x07, 0x0c, -1};
static char	text_ancientwarrior[] = {0x0c, 0x08, 0x23, 0x0f, 0x41, -1};
static char	text_highpriest[] = {0x1a, 0x41, 0x22, 0x41, 0x25, 0x41, 0x2f, -1};
static char	text_cyborg[] = {0x33, 0x41, 0x29, -1};
static char	text_demon[] = {0x2e, 0x41, 0x1f, 0x27, -1};
static char	text_demonlord[] = {0x2e, 0x41, 0x1f, 0x27, 0x25, 0x41, 0x2f, -1};
static char	text_pteranodon[] = {0x2e, 0x0c, 0x02, 0x3a, 0x27, 0x29, -1};
static char	text_sludgebeast[] = {0x06, 0x21, 0x41, 0x0b, 0x2b, 0x41, 0x2d, -1};
static char	text_killerplant[] = {0x2e, 0x0c, 0x36, 0x21, 0x27, 0x13, -1};
static char	text_robot[] = {0x25, 0x33, 0x01, 0x2f, -1};
static char	text_triceratops[] = {0x30, 0x01, 0x04, 0x2b, 0x02, 0x23, 0x0c, -1};
static char	text_subterranean[] = {0x26, 0x41, 0x1d, -1};

static char	text_quittraining[] = {0x13, 0x24, 0x41, 0x15, 0x27, 0x29, 0x74, 0x6c, 0x6a, 0x5d, -1} ;
static char	text_areyousure[] = {0x6D, 0x72, 0x57, 0x4D, 0x7A, 0x57, 0x80, 0x4E, 0x51, -1};
#endif

// thread overview
/////////////////////////////////////////////////////////////////////////////
//		rmon thread. Used for debugging.
//			Its main purpose is to send printf's back to host machine.
//		scheduler thread. The highest priority thread of the game. Its
//			function is to see that audio and graphics tasks get built
//			and executed in a timely fashion.
//		audio thread. The second highest thread. Creates the audio task
//			lists to be executed by the audio microcode to synthesize
//			the audio. Audio should run at a higher priority than
//			graphics, since if a frame of audio gets dropped, it will
//			cause a serious click, but a frame of graphics can always
//			redraw the last frame.
//		game thread. Loops waiting for messages from the scheduler.
//			Upon receiving retrace messages, will branch to the graphics
//			routine and the controller read routine.
//		idle thread. Starts out as the init thread, but becomes the idle
//			thread.
//		fault thread.  prints debug information on the screen after
//			a CPU exception.  If the rmon thread is started after the
//			fault thread, it will steal the exception interrupt thereby
//			preventing the fault thread from doing anything.
/////////////////////////////////////////////////////////////////////////////

//#define DISPLAY_TIMING


#ifdef MAKE_CART
#ifdef DISPLAY_TIMING
#undef DISPLAY_TIMING
#endif
#endif



// analog menu control defines
#define	MENU_START_SPEED	249.0
#define	MAX_MENU_SPEED		250.0

#define	MENU_HORIZONTAL_THRESHHOLD	30
#define	MENU_VERTICAL_THRESHHOLD	30

// TEMP
void			CCartCache__MemoryDump(CCartCache *pThis);
void			printFaultData(OSThread *);


// CEngineApp implementation
/////////////////////////////////////////////////////////////////////////////

void			CEngineApp__InitializeGameVars(CEngineApp *pThis) ;
void			CEngineApp__SetupShadowVtxs();
void 			CEngineApp__InitGame(CEngineApp *pThis);
void			CEngineApp__InitGraphics(CEngineApp *pThis);
CFrameData* CEngineApp__CreateGraphicsTask(CEngineApp *pThis);
void			CEngineApp__SendGraphicsTask(CEngineApp *pThis, CFrameData *pFData);
void 			CEngineApp__InitDisplayLists(CEngineApp *pThis);
void 			CEngineApp__ClearBuffers(CEngineApp *pThis);
void 			CEngineApp__SetupSegments(CEngineApp *pThis);
#define		CEngineApp__SetDebugMode(pThis, Debug) (pThis)->m_DebugMode = Debug
void			CEngineApp__SetupDraw(CEngineApp *pThis) ;
void 			CEngineApp__DrawGAMEPAK(CEngineApp *pThis);
void 			CEngineApp__DrawLEGALSCREEN(CEngineApp *pThis);
void 			CEngineApp__DrawTITLE(CEngineApp *pThis);
void 			CEngineApp__DrawGALLERY(CEngineApp *pThis);
void 			CEngineApp__DrawGAME(CEngineApp *pThis);
void 			CEngineApp__DrawCREDITS(CEngineApp *pThis);
void			CEngineApp__DrawFlash(CEngineApp *pThis);
void			CEngineApp__DrawColorFlash(CEngineApp *pThis);
void			CEngineApp__UpdateFade(CEngineApp *pThis);
void			CEngineApp__ResetGallery(CEngineApp *pThis);
void			CEngineApp__UpdateGallery(CEngineApp *pThis);
void			CEngineApp__DrawFade(CEngineApp *pThis);
void			CEngineApp__DrawWarp(CEngineApp *pThis);
void			CEngineApp__DrawSunGlare(CEngineApp *pThis);
void			CEngineApp__DrawDeath(CEngineApp *pThis);
void			CEngineApp__PlayerLostLife(CEngineApp *pThis);
void 			CEngineApp__BuildDispList(CEngineApp *pThis);
void			CEngineApp__ScreenShot(CEngineApp *pThis);
void			CEngineApp__SetCameraToTurok(CEngineApp *pThis);
void			CEngineApp__DoPause(CEngineApp *pThis);
#define		ToggleEvenOdd() even_odd ^= 1
void			CEngineApp__AdvanceFrameData(CEngineApp *pThis);
#define		CEngineApp__GetFrameData(pThis) ((pThis)->m_pCurrentFrameData)
void			CEngineApp__GetFogColor(CEngineApp *pThis, BYTE *pR, BYTE *pG, BYTE *pB);
void			CEngineApp__UpdateSpiritMode(CEngineApp *pThis) ;
void			CEngineApp__UpdateCameraAttributes(CEngineApp *pThis);
void			CEngineApp__UpdatePause(CEngineApp *pThis);
void			CEngineApp__UpdateFrameData(CEngineApp *pThis, GFXMsg *pMsg);
void			CEngineApp__DoWarp(CEngineApp *pThis,
										 int WarpID,
										 float Seconds,
										 int SoundEffect,
										 BOOL WarpIn,
										 BYTE Red, BYTE Green, BYTE Blue,
										 BOOL StoreWarpReturn);

void			CEngineApp__UpdateGAMEPAK(CEngineApp *pThis);
void			CEngineApp__UpdateLEGALSCREEN(CEngineApp *pThis);
void			CEngineApp__UpdateINTRO(CEngineApp *pThis);
void			CEngineApp__UpdateTITLE(CEngineApp *pThis);
void			CEngineApp__UpdateGAME(CEngineApp *pThis);
void			CEngineApp__UpdateCREDITS(CEngineApp *pThis);
void			CEngineApp__PrepareFrame(CEngineApp *pThis, int nMinimumFields);

// Symbol genererated by "makerom" to indicate the end of the code segment
// in virtual (and physical) memory
/////////////////////////////////////////////////////////////////////////////

extern char _codeSegmentEnd[];

// Symbols generated by "makerom" to tell us where the static segment is
// in ROM.
/////////////////////////////////////////////////////////////////////////////

extern char _staticSegmentRomStart[], _staticSegmentRomEnd[];

// Entry points for new threads
/////////////////////////////////////////////////////////////////////////////

// boot thread entry point
void boot();
// idle thread entry point
void idle(void *pArg);
// main thread entry point
void mainproc(void *pArg);
// fault thread entry point
void faultproc(void *pArg);


// Stacks for the threads as well as message queues for synchronization
// This stack is ridiculously large, and could also be reclaimed once
// the main thread is started.
/////////////////////////////////////////////////////////////////////////////

//u64	stack_must_be_16_byte_aligned;

// necessary for RSP tasks:
//u64 dram_stack[SP_DRAM_STACK_SIZE64];	// used for matrix stack


static OSThread	idleThread;
static OSThread	mainThread;
#ifdef MAKE_CART
static OSThread	faultThread;
#else
static OSThread	rmonThread;
#endif

// this number (the depth of the message queue) needs to be equal
// to the maximum number of possible overlapping PI requests.
// For this app, 1 or 2 is probably plenty, other apps might
// require a lot more.
/////////////////////////////////////////////////////////////////////////////

//#define NUM_PI_MSGS  8

// scheduler globals
OSSched			sc;
//OSMesgQueue		*sched_cmdQ;
OSScClient		gfxClient;

OSMesgQueue		faultMsgQ;
OSMesg			faultMsgBuf;

OSMesg 			PiMessages[DMA_QUEUE_SIZE];
OSMesgQueue		PiMessageQ;

OSMesgQueue		dmaMessageQ;
OSMesg			dmaMessageBuf;

OSMesgQueue		romMessageQ;
OSMesg			romMessageBuf;
OSIoMesg			romIOMessageBuf;

OSMesgQueue		gfxFrameMsgQ;
OSMesg			gfxFrameMsgBuf[MAX_MESGS];

OSIoMesg			dmaIOMessageBuf;

DWORD			c_field;
DWORD			n_fields;
DWORD			n_l_fields;
DWORD			n_ll_fields;
DWORD			n_lll_fields;
DWORD			c_waiting;
DWORD			n_waiting;
DWORD			c_finished;
DWORD			n_pending;
CFrameData	*p_waiting_frames[3];
BOOL			allow_task_sends = TRUE;
void			*p_current_fb;
void			*p_pending_fb;

// utility globals

DWORD			frame_number = 0;
DWORD			even_odd = 0;
DWORD			game_frame_number = 0;

DWORD			microcode_version = MICROCODE_HIGH_PRECISION;

float			frame_increment = 1.0;
#ifdef PLATFORM_PORT
/* the frame_increment used on the LAST logic tick (the amount every animation's m_cFrame advanced),
 * captured at the tick gate. Render interpolation of skeletal anims derives prev = m_cFrame - this. */
float			g_turok_anim_step = 1.0f;
#endif
float			enemy_speed_scaler = 1.0;				// these vars are
float			particle_speed_scaler = 1.0;			// also setup
float			sky_speed_scaler = 1.0;					// in MODE_GAME_START

float			old_enemy_speed_scaler = 1.0;
float			old_particle_speed_scaler = 1.0;
float			old_sky_speed_scaler = 1.0;


float			rsp_total_time = 0.0;
float			rsp_audio_time = 0.0;
float			rsp_graphics_time = 0.0;
float			rdp_time = 0.0;
float			cpu_time = 0.0;

float			weapon_z_rotation = 0.0;

//ROMAddress	*rp_current_textureset;
//DWORD			current_texture;

//DWORD			idle_count;

BOOL			player_is_moving = TRUE;
BOOL			cache_is_valid = FALSE;

CViewVolume	view_volume;
CBoundsRect	view_bounds_rect,
				anim_bounds_rect;

BYTE			*scratch_area = (BYTE*) boot_stack;
int			scratch_area_size = STACKSIZE;

// controller data
int			Controller;		// lowest numbered controller connected to system
CTControl	*pCTControl;	// pointer to turok controller data
CTMove		*pCTMove;		// pointer to turok movement object

float			refresh_rate;

//float			pad1;

// global matrices
/////////////////////////////////////////////////////////////////////////////

// floating point matrices
CMtxF		mf_3dstudio_to_u64;

// fixed point matrices
Mtx		mtx_identity;

// for shadows
Vtx		shadow_vtxs[4];

// Main application object
/////////////////////////////////////////////////////////////////////////////

CEngineApp engine_app;


// Boot thread entry point
/////////////////////////////////////////////////////////////////////////////

void boot()
{
	CEngineApp__Boot(GetApp());

	// never reached
	ASSERT(FALSE);
}

// Idle thread entry point
/////////////////////////////////////////////////////////////////////////////

// static
void idle(void *pArg)
{
	CEngineApp__Idle(GetApp(), pArg);

	// never reached
	ASSERT(FALSE);
}

// Fault thread entry point
/////////////////////////////////////////////////////////////////////////////

typedef	void (*pfnReboot)() ;

extern CGameRegion *p_regions;
void faultproc(void *pArg)
{
	OSMesg				msg = NULL;		// supresses warning
	OSTime				startTime;
	u32					fpstat;
	OSThread				*curr;
	CGameObjectInstance *pPlayer ;
	//static OSThread	*curr, *last;


	osSetEventMesg(OS_EVENT_FAULT, &faultMsgQ, (OSMesg) MSG_FAULT);


	// fetch the current floating-point control/status register
	fpstat = __osGetFpcCsr();

	// enable divide-by-zero exception for floating point
	fpstat |= FPCSR_EZ;

	// _Disable_ unimplemented operation exception for floating point.
	// The trunc.w.s instruction causes this exception when the input
	// value is larger than the maximum fixed point value.  This is
	// can happen in the game, so I'm disabling the exception.
	//fpstat &= ~(FPCSR_CE | FPCSR_FS | FPCSR_EV | FPCSR_FI | FPCSR_RM_RN);
	//fpstat &= ~FPCSR_CE;
	//fpstat = 0;

	__osSetFpcCsr(fpstat);

	//last = (OSThread*) NULL;

	while (TRUE)
	{
		osRecvMesg(&faultMsgQ, (OSMesg*) &msg, OS_MESG_BLOCK);

#ifdef SHIP_IT
	// create the persistant data block
	if (		cache_is_valid
			&& (GetApp()->m_Mode == MODE_GAME)
			&& CEngineApp__GetPlayer(GetApp()) )
	{
		pPlayer = CEngineApp__GetPlayer(GetApp());

		if (frame_number > 150)
		{
			CrashId = 0x4341524c ;
			CSave__PrepareData() ;
		}
		else
			CrashId = 0x4341524d ;

		//CrashMode = GetApp()->m_Mode ;

		//CrashFrame = game_frame_number ;

		CrashWarp.m_vPos = pPlayer->ah.ih.m_vPos;
		CrashWarp.m_RotY = pPlayer->m_RotY;
		CrashWarp.m_nLevel = GetApp()->m_Scene.m_nLevel;
		if (pPlayer->ah.ih.m_pCurrentRegion)
			CrashWarp.m_nRegion = ((DWORD)pPlayer->ah.ih.m_pCurrentRegion - (DWORD)p_regions)/sizeof(CGameRegion);
		else
			CrashWarp.m_nRegion = -1;
	}
	else
	{
		CrashId = 0;
	}
/*

	pPlayer = CEngineApp__GetPlayer(GetApp());
	if (pPlayer)
	{
		CSave__PrepareData() ;
		CrashId = 0x4341524c ;
		CrashMode = GetApp()->m_Mode ;

		CrashFrame = game_frame_number ;

		CrashWarp.m_vPos = pPlayer->ah.ih.m_vPos;
		CrashWarp.m_RotY = pPlayer->m_RotY;
		CrashWarp.m_nLevel = GetApp()->m_Scene.m_nLevel;
		if (pPlayer->ah.ih.m_pCurrentRegion)
			CrashWarp.m_nRegion = ((DWORD)pPlayer->ah.ih.m_pCurrentRegion - (DWORD)p_regions)/sizeof(CROMRegion);
		else
			CrashWarp.m_nRegion = -1;
	}
*/
	osWritebackDCacheAll();


	osSetIntMask(OS_IM_NONE) ;
	((pfnReboot)0x80000400)() ;
#endif
//#else

		// returns the most recent faulted thread
		curr = __osGetCurrFaultedThread();

		if (trap_thread == THREAD_MAIN)
			curr = &mainThread;

		if (curr)
		{
			// wait for 1 second for display lists to finish up
			startTime = osGetTime();

			while ((((float)CYCLES_TO_NSEC(osGetTime() - startTime))/1000000) < 2);

			if (osTvType == 0)
				osViSetMode(&osViModeTable[OS_VI_PAL_HPF1]);
			else if (osTvType == 1)
				osViSetMode(&osViModeTable[OS_VI_NTSC_HPF1]);
			else
				osViSetMode(&osViModeTable[OS_VI_MPAL_HPF1]);


			osViSetSpecialFeatures(OS_VI_GAMMA_OFF);
			osViSwapBuffer(cfb_16_a);

			printFaultData(curr);

			while (TRUE);

			// recover?
		}

//#endif
	}
}


// Main thread entry point
/////////////////////////////////////////////////////////////////////////////

void mainproc(void *pArg)
{
	CEngineApp__Main(GetApp());

	// never reached
	ASSERT(FALSE);
}

// CEngineApp
/////////////////////////////////////////////////////////////////////////////
// DO NOT PUT ANYTHING IN HERE THAT NEEDS TO BE RESET EACH GAME!!!!!!!!!!!!!
void CEngineApp__Construct(CEngineApp *pThis)
{
	CMtxF				mfRot, mfFlipZ;
	BYTE				*pTest, *pSig;
#define SIGNATURE_LENGTH	8
	BYTE				signature[SIGNATURE_LENGTH] = "TurokRBC";
	static BYTE		testarea[SIGNATURE_LENGTH];


	pThis->m_FirstRun = FALSE;


	pTest = testarea;
	pSig = signature;
	while (*pSig)
	{
		if (*pTest != *pSig)
			pThis->m_FirstRun = TRUE;

		*pTest++ = *pSig++;
	}

	if (pThis->m_FirstRun)
		COptions__SetDefaults(&pThis->m_Options);

	pThis->m_bReset = FALSE ;

	pThis->m_FadeStatus = FADE_NULL ;
	pThis->m_FadeFast = FALSE ;


	CEngineApp__SetDebugMode(pThis, FALSE);

	pThis->m_pDLP 			= NULL;
	pThis->m_SloMo			= FALSE;
	pThis->m_bPause		= FALSE;
	pThis->m_hMusic		= -1;
	pThis->m_MusicVolume = 0.0;
	pThis->m_SFXVolume   = 0.0;

	CMtxF__RotateX(mfRot, -ANGLE_PI/2) ;
	CMtxF__Scale(mfFlipZ, 1, 1, -1);
	CMtxF__PostMult(mf_3dstudio_to_u64, mfRot, mfFlipZ);

	CEngineApp__SetupMap(pThis);

	guMtxIdent(&mtx_identity);

	CEngineApp__SetupShadowVtxs();
	CSun__Construct(&sun);

	TRACE0("\r\n\r\n\r\n");
	TRACE0("************************************************************************\r\n");
	TRACE0("**                           Program Start                            **\r\n");
	TRACE0("************************************************************************\r\n");
	TRACE0("\r\n");

#if 1
	TRACE("cartridge data: %d kb\r\n", ((DWORD) (_staticSegmentRomEnd - _staticSegmentRomStart))/1024);
	TRACE("cartridge cache: %d kb\r\n", MEMORY_POOL_SIZE/1024);
	TRACE("frame buffer: %d kb\r\n", (2*SCREEN_WD*SCREEN_HT*sizeof(unsigned short))/1024);
	TRACE("z buffer: %d kb\r\n", (SCREEN_WD*SCREEN_HT*sizeof(unsigned short))/1024);
#ifdef USE_FIFO
	TRACE("FIFO buffer: %d kb\r\n", FIFO_SIZE/1024);
#endif
	TRACE("audio heap: %d kb\r\n", AUDIO_HEAP_SIZE/1024);
	TRACE("code & static data: %d kb\r\n", ((DWORD) OS_K0_TO_PHYSICAL(_codeSegmentEnd))/1024);
	TRACE("  particle system: %d kb\r\n", sizeof(CParticleSystem)/1024);
	TRACE("  animation matrices: %d kb\r\n", sizeof(Mtx)*2*MATTABLE_MATRICES/1024);
	TRACE("  cache entries: %d kb\r\n", sizeof(CCartCache)/1024);
	TRACE("  display lists: %d kb\r\n", (2*sizeof(Gfx)*(GLIST_LEN + LINELIST_LEN))/1024);
	TRACE("  main thread stack: %d kb\r\n", MAIN_STACKSIZE/1024);
	TRACE0("\r\n");
	TRACE("RAM remaining: %d kb\r\n", (((DWORD)audioHeap) - ((DWORD)_codeSegmentEnd))/1024);
	TRACE0("\r\n");

	TRACE("_codeSegmentEnd:%x\r\n", _codeSegmentEnd);
	TRACE("_staticSegmentRomStart:%x\r\n", _staticSegmentRomStart);
	TRACE("_staticSegmentRomEnd:%x\r\n", _staticSegmentRomEnd);
	TRACE("cfb_16_a:%x\r\n", cfb_16_a);
	TRACE("dynamic_memory_pool: %x\r\n\r\n", dynamic_memory_pool);
#endif

	pThis->m_DoIdle = TRUE;	// allow idle thread processing
}


// -----------------------------------------------------------------------------------------
// put all game initialization stuff in here, NOT CENGINEAPP__CONSTRUCT
// anything that should be reset each time a new game is started
void CEngineApp__InitializeGameVars(CEngineApp *pThis)
{
	if (		(CrashId == 0x4341524c)
			||	(CrashId == 0x4341524d) )
	{
		persistant_data_loaded = TRUE ;
	}
	else
	{
		CScene__ResetPersistantData();
	}

	pThis->m_nCurrentRegionSet = (DWORD) -1 ;
	pThis->m_Flash = 0;
	pThis->m_Alpha = 0;

	pThis->m_LastFogColor[0] = pThis->m_FogColor[0] = 0 ;		//72;
	pThis->m_LastFogColor[1] = pThis->m_FogColor[1] = 0 ;		//72;
	pThis->m_LastFogColor[2] = pThis->m_FogColor[2] = 0 ;		//80;
	pThis->m_LastFogColor[3] = pThis->m_FogColor[3] = 0;
	pThis->m_cRegionSetBlend = 1;

	pThis->m_LastFarClip			= pThis->m_FarClip		= SCALING_FAR_CLIP;
	pThis->m_LastFieldOfView	= pThis->m_FieldOfView	= DEFAULT_FIELD_OF_VIEW;
	pThis->m_LastFogStart		= pThis->m_FogStart		= (100 - DEFAULT_FOG_THICKNESS)*(1000/100);

	pThis->m_LastSkyElevation = pThis->m_SkyElevation = 28;
	pThis->m_LastSkySpeed = pThis->m_SkySpeed = 100;

	pThis->m_UnderWater = FALSE;
	pThis->m_Warp = WARP_NOT_WARPING;

	pThis->m_Death = DEATH_NOT_DIEING;

	pThis->m_CinemaFlags = NULL ;
	pThis->m_UseCinemaWarp = FALSE ;

	pThis->m_ReturnWarpSaved = FALSE;

	pThis->m_RotY = 0;

	pThis->m_XPos 	= 0;
	pThis->m_YPos 	= 0;
	pThis->m_ZPos 	= 0;

	pThis->m_BossLevel = 0 ;

	pThis->m_MenuUpChangeCount = MENU_START_SPEED;
	pThis->m_MenuDownChangeCount = MENU_START_SPEED;
	pThis->m_MenuLeftChangeCount = MENU_START_SPEED;
	pThis->m_MenuRightChangeCount = MENU_START_SPEED;

	CQuatern__Identity(&pThis->m_qGround);
	CMtxF__Identity(pThis->m_mfViewOrient);
	CMtxF__Identity(pThis->m_mfProjection);
	CQuatern__Identity(&pThis->m_qViewAlign);

	pThis->m_RotXOffset = 0;
	pThis->m_RotYOffset = 0;
	pThis->m_RotZOffset = 0;
	pThis->m_YPosOffset = 0;

	CCollisionInfo__Reset();

	COnScreen__Construct(&pThis->m_OnScreen) ;
}



void CEngineApp__SetupShadowVtxs()
{
	int	v;
	Vtx	*pV;

	for (v=0; v<4; v++)
	{
		pV = &shadow_vtxs[v];

		switch (v)
		{
			case 0:
				pV->v.ob[0] = -1;
				pV->v.ob[2] = 1;
				pV->v.tc[0] = 0;
				pV->v.tc[1] = 0;
				break;
			case 1:
				pV->v.ob[0] = 1;
				pV->v.ob[2] = 1;
				pV->v.tc[0] = 32*(1 << 5);
				pV->v.tc[1] = 0;
				break;
			case 2:
				pV->v.ob[0] = 1;
				pV->v.ob[2] = -1;
				pV->v.tc[0] = 32*(1 << 5);
				pV->v.tc[1] = 32*(1 << 5);
				break;
			case 3:
				pV->v.ob[0] = -1;
				pV->v.ob[2] = -1;
				pV->v.tc[0] = 0;
				pV->v.tc[1] = 32*(1 << 5);
				break;
		}
		pV->v.ob[1] = 0;

		pV->v.flag = 0;

		pV->v.cn[0] = 255;
		pV->v.cn[1] = 255;
		pV->v.cn[2] = 255;
		pV->v.cn[3] = 255;
	}
}

void CEngineApp__Boot(CEngineApp *pThis)
{
#ifndef MAKE_CART
	int 	i;
   u32 	argbuf[16];
	u32 	*pArg;
	char	*ap;
#endif

   // note: can't call rmonPrintf() until you set up the rmon thread.

   osInitialize();

#ifndef MAKE_CART
   pArg = (u32*) RAMROM_APP_WRITE_ADDR;
   for (i=0; i<sizeof(argbuf)/4; i++, pArg++)
	{
	 	// Assume no DMA
		osPiRawReadIo((u32)pArg, &argbuf[i]);
	}

   // Parse the options
   ap = (char*) argbuf;
   while (*ap != '\0')
	{
		while (*ap == ' ')
	   	ap++;

		if ( (*ap == '-') && ((*(ap+1) == 'd') || (*(ap+1) == 'D')) )
		{
	    	CEngineApp__SetDebugMode(pThis, TRUE);
	    	ap += 2;
		}
	}

#endif

	// kick off idle thread
	//////////////////////////////////////////////////////////////////////////

   osCreateThread(&idleThread, THREAD_IDLE, idle, (void*)0,
		   idle_thread_stack+IDLE_STACKSIZE/sizeof(u64), (OSPri) PRIORITY_IDLEINIT);
   osStartThread(&idleThread);

   // never reached
	// (there's no reason for idle thread to yield)
	ASSERT(FALSE);
}

void CEngineApp__Idle(CEngineApp *pThis, void *pArg)
{
	//int i;

	// Initialize video
	//////////////////////////////////////////////////////////////////////////

	osViBlack(TRUE);	// set display to black

	// Start PI Mgr for access to cartridge
	//////////////////////////////////////////////////////////////////////////

	osCreatePiManager((OSPri)OS_PRIORITY_PIMGR, &PiMessageQ, PiMessages,
			//NUM_PI_MSGS);
			DMA_QUEUE_SIZE);

	// Start RMON for debugging & data xfer (make sure to start PI Mgr first)
	//////////////////////////////////////////////////////////////////////////

#ifndef MAKE_CART
	 osCreateThread(&rmonThread, THREAD_RMON, rmonMain, (void*) 0,
						rmon_stack+RMON_STACKSIZE/sizeof(u64),
						(OSPri) OS_PRIORITY_RMON );
	osStartThread(&rmonThread);
#endif

	// fault detection thread
	//////////////////////////////////////////////////////////////////////////

#ifdef MAKE_CART
    osCreateMesgQueue(&faultMsgQ, &faultMsgBuf, 1);
    osCreateThread(&faultThread, THREAD_FAULT, faultproc, pArg,
					    fault_thread_stack+STACKSIZE/sizeof(u64),
					    (OSPri) 50);
    osStartThread(&faultThread);
#endif

	// Create main thread
	//////////////////////////////////////////////////////////////////////////

	osCreateThread(&mainThread, THREAD_MAIN, mainproc, pArg,
						main_thread_stack+MAIN_STACKSIZE/sizeof(u64),
						(OSPri) PRIORITY_MAIN);

	// use "gload -a -d" for debugging with gvd
	if (!pThis->m_DebugMode)
		osStartThread(&mainThread);

	// Become the idle thread
	//////////////////////////////////////////////////////////////////////////

	pThis->m_DoIdle = FALSE;

	osSetThreadPri(NULL, PRIORITY_IDLE);

	// Perform any idle processing here.
	//////////////////////////////////////////////////////////////////////////
	// May need to raise priority of thread or create mutex if
	// databases are modified.  Otherwise, higher priority events
	// can preempt this thread.
	//////////////////////////////////////////////////////////////////////////
	// If idle processing is done with a thread with the same priority as the
	// game thread, mutexes should not be necessary.  The advantage of using
	// a seperate thread is that they can each block on their own message
	// queue.  The background processing thread would call osYieldThread()
	// between critical sections.

	while (TRUE);
}

void CEngineApp__SetupSegments(CEngineApp *pThis)
{
	gSPSegment(pThis->m_pDLP++, 0, 0x0);
}

void CEngineApp__GetFogColor(CEngineApp *pThis, BYTE *pR, BYTE *pG, BYTE *pB)
{
	// mask off bottom 3 bits so background and fog will be the same color
	*pR = pThis->m_FogColor[0] & 0xf8;
	*pG = pThis->m_FogColor[1] & 0xf8;
	*pB = pThis->m_FogColor[2] & 0xf8;
}

void CEngineApp__ClearBuffers(CEngineApp *pThis)
{
	BYTE		r, g, b;
	CCamera	*pCamera = &pThis->m_Camera ;

	// must clear buffers in G_RM_OPA_SURF render mode
	CGeometry__SetRenderMode(&pThis->m_pDLP, G_RM_NOOP__G_RM_NOOP2);

	// Reset Z buffer
	//////////////////////////////////////////////////////////////////////////

//#ifdef	USE_ZBUFFER
	gDPSetColorImage(pThis->m_pDLP++, G_IM_FMT_RGBA, G_IM_SIZ_16b,
			SCREEN_WD, RDP_ADDRESS(zbuffer));

	gDPPipeSync(pThis->m_pDLP++);
	gDPSetCycleType(pThis->m_pDLP++, G_CYC_FILL);

	gDPSetFillColor(pThis->m_pDLP++,
			GPACK_ZDZ(G_MAXFBZ, 0) << 16 | GPACK_ZDZ(G_MAXFBZ, 0));

	gDPFillRectangle(pThis->m_pDLP++, 0, 0, SCREEN_WD-1, SCREEN_HT-1);

//	gDPPipeSync(pThis->m_pDLP++);
//#endif

	// Reset color frame buffer
	//////////////////////////////////////////////////////////////////////////

	//gDPSetCycleType(pThis->m_pDLP++, G_CYC_FILL);
	gDPSetColorImage(pThis->m_pDLP++, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WD,
		   			  RDP_ADDRESS(CEngineApp__GetFrameData(pThis)->m_pFrameBuffer));

	CEngineApp__GetFogColor(pThis, &r, &g, &b);


	if (CCamera__InLetterBoxMode(pCamera))
	{
		// Fill borders with black
		gDPSetFillColor(pThis->m_pDLP++, GPACK_RGBA5551(0,0,0,0) << 16 | GPACK_RGBA5551(0,0,0,0)) ;

		gDPFillRectangle(pThis->m_pDLP++, 0, 0,
													 SCREEN_WD - 1, pCamera->m_LetterBoxTop - 1) ;

		gDPFillRectangle(pThis->m_pDLP++, 0, pCamera->m_LetterBoxBottom,
													 SCREEN_WD - 1, SCREEN_HT - 1) ;

		// Fill rest of screen with fog color
//		gDPPipeSync(pThis->m_pDLP++);
		gDPSetFillColor(pThis->m_pDLP++, GPACK_RGBA5551(r,g,b,1) << 16 |
				GPACK_RGBA5551(r,g,b,1));

		gDPFillRectangle(pThis->m_pDLP++, 0, pCamera->m_LetterBoxTop,
													 SCREEN_WD-1, pCamera->m_LetterBoxBottom-1) ;

	}
	else
	{
		// Fill screen with fog color
		gDPSetFillColor(pThis->m_pDLP++, GPACK_RGBA5551(r,g,b,1) << 16 |
				GPACK_RGBA5551(r,g,b,1));

		gDPFillRectangle(pThis->m_pDLP++, 0, 0, SCREEN_WD-1, SCREEN_HT-1);
	}

	if ((CTurokMovement.InSun) && (!CCamera__InCinemaMode(&GetApp()->m_Camera)))
		COnScreen__DrawSun(&pThis->m_pDLP);

	gDPPipeSync(pThis->m_pDLP++);

	// Return to 1 cycle mode
	//////////////////////////////////////////////////////////////////////////

#ifdef USE_FOG
   gDPSetCycleType(pThis->m_pDLP++, G_CYC_2CYCLE);
#else
	gDPSetCycleType(pThis->m_pDLP++, G_CYC_1CYCLE);
#endif
}

void CEngineApp__ClearZBuffer(CEngineApp *pThis, int sx, int ex, int sy, int ey)
{
	gDPPipeSync(pThis->m_pDLP++);

	CGeometry__SetRenderMode(&pThis->m_pDLP, G_RM_NOOP__G_RM_NOOP2);

	// Reset Z buffer
	//////////////////////////////////////////////////////////////////////////

	gDPSetColorImage(pThis->m_pDLP++, G_IM_FMT_RGBA, G_IM_SIZ_16b,
			SCREEN_WD, RDP_ADDRESS(zbuffer));

	gDPPipeSync(pThis->m_pDLP++);
	gDPSetCycleType(pThis->m_pDLP++, G_CYC_FILL);

	gDPSetFillColor(pThis->m_pDLP++,
			GPACK_ZDZ(G_MAXFBZ, 0) << 16 | GPACK_ZDZ(G_MAXFBZ, 0));

	gDPFillRectangle(pThis->m_pDLP++, sx, sy, ex, ey);

//	gDPPipeSync(pThis->m_pDLP++);

	gDPSetColorImage(pThis->m_pDLP++, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WD,
		   			  RDP_ADDRESS(CEngineApp__GetFrameData(pThis)->m_pFrameBuffer));
	gDPPipeSync(pThis->m_pDLP++);

	// Return to 1 cycle mode
	//////////////////////////////////////////////////////////////////////////

#ifdef USE_FOG
   gDPSetCycleType(pThis->m_pDLP++, G_CYC_2CYCLE);
#else
	gDPSetCycleType(pThis->m_pDLP++, G_CYC_1CYCLE);
#endif

//	gDPPipeSync(pThis->m_pDLP++);
//*/
}

void CEngineApp__BuildDispList(CEngineApp *pThis)
{
	CCamera	*pCamera = &pThis->m_Camera ;

	// clears current render and combine modes
	CGeometry__ResetDrawModes() ;


	CEngineApp__InitDisplayLists(pThis);
	CEngineApp__ClearBuffers(pThis);

	switch (pThis->m_Mode)
	{
		case MODE_LEGALSCREEN:
			CEngineApp__DrawLEGALSCREEN(pThis);
			break ;
		case MODE_GAMEPAK:
			CEngineApp__DrawGAMEPAK(pThis);
			break ;
		case MODE_TITLE:
			CEngineApp__DrawTITLE(pThis);
			break ;
		case MODE_INTRO:
		case MODE_GAME:
		case MODE_GAMEOVER:
			CEngineApp__DrawGAME(pThis);
			break ;
		case MODE_CREDITS:
			CEngineApp__DrawCREDITS(pThis);
			break ;
		case MODE_GALLERY:
			CEngineApp__DrawGAME(pThis);
			CEngineApp__DrawGALLERY(pThis);
			break ;
	}

	if ((pThis->m_Mode == MODE_GAME) && (!CCamera__InCinemaMode(&pThis->m_Camera)))
		CEngineApp__UpdatePause(pThis);

	// Erase top and bottom line of letterbox to fix nasty aliasing pixel bits
	if (CCamera__InLetterBoxMode(pCamera))
	{
		gDPPipeSync(pThis->m_pDLP++);
		gDPSetCycleType(pThis->m_pDLP++, G_CYC_1CYCLE);
		CGeometry__SetRenderMode(&pThis->m_pDLP, G_RM_XLU_SURF__G_RM_XLU_SURF2);
		CGeometry__SetCombineMode(&pThis->m_pDLP, G_CC_PRIMITIVE__G_CC_PRIMITIVE);
		gDPSetPrimColor(pThis->m_pDLP++, 0, 0, 0, 0, 0, 0);

		gDPFillRectangle(pThis->m_pDLP++, 0,			pCamera->m_LetterBoxTop,
													 SCREEN_WD, pCamera->m_LetterBoxTop+1) ;

		gDPFillRectangle(pThis->m_pDLP++, 0,			pCamera->m_LetterBoxBottom-1,
													 SCREEN_WD, pCamera->m_LetterBoxBottom-1+1) ;
	}
}

// get ground height where player is standing
float CEngineApp__GetGroundHeightWherePlayerIs(CEngineApp *pThis)
{
	CGameObjectInstance	*pPlayer;
	CGameRegion				*pRegion;


	pPlayer = CEngineApp__GetPlayer(GetApp());
	ASSERT(pPlayer);

	if (CInstanceHdr__IsOnGround((CInstanceHdr *)pPlayer))
		return 0.0;

	pRegion = pPlayer->ah.ih.m_pCurrentRegion;
	if (!pRegion)
		return 0.0;

	return CGameRegion__GetGroundHeight(pRegion, pPlayer->ah.ih.m_vPos.x, pPlayer->ah.ih.m_vPos.z);
}

// get depth of water where player is standing
float CEngineApp__GetWaterDepthWherePlayerIs(CEngineApp *pThis)
{
	CGameObjectInstance	*pPlayer;
	CGameRegion				*pRegion;
	CROMRegionSet			*pRegionSet;
	int						waterFlag;


	pPlayer = CEngineApp__GetPlayer(GetApp());
	ASSERT(pPlayer);

	pRegion = pPlayer->ah.ih.m_pCurrentRegion;
	if (!pRegion)
		return 0;

	pRegionSet = CScene__GetRegionAttributes(&pThis->m_Scene, pRegion);
	if (!pRegionSet)
		return 0;

	waterFlag = CEngineApp__GetPlayerWaterStatus(pThis);
	if (waterFlag == PLAYER_NOT_NEAR_WATER)
		return 0;
	else
		return pRegionSet->m_WaterElevation;
}

// get height player is submerged by (0 to depth of water)
float CEngineApp__GetPlayerWaterDepth(CEngineApp *pThis)
{
	CGameObjectInstance	*pPlayer;
	CGameRegion				*pRegion;
	CROMRegionSet			*pRegionSet;
	int						waterFlag;
	CVector3					vEyePos;


	pPlayer = CEngineApp__GetPlayer(GetApp());
	ASSERT(pPlayer);

	pRegion = pPlayer->ah.ih.m_pCurrentRegion;
	if (!pRegion)
		return 0;

	pRegionSet = CScene__GetRegionAttributes(&pThis->m_Scene, pRegion);
	if (!pRegionSet)
		return 0;

	waterFlag = CEngineApp__GetPlayerWaterStatus(pThis);
	if (waterFlag == PLAYER_BELOW_WATERSURFACE ||
		 waterFlag == PLAYER_ON_WATERSURFACE )
	{
		vEyePos = CEngineApp__GetEyePos(pThis);
		return pRegionSet->m_WaterElevation - vEyePos.y;
	}

	return 0;
}

int CEngineApp__GetPlayerWaterStatus(CEngineApp *pThis)
{
	CGameObjectInstance	*pPlayer;
	CGameRegion				*pRegion;
	CROMRegionSet			*pRegionSet;
	int						waterFlag;
	CVector3					vEyepos;

	pPlayer = CEngineApp__GetPlayer(pThis);
	if (!pPlayer)
		return PLAYER_NOT_NEAR_WATER;

	pRegion = pPlayer->ah.ih.m_pCurrentRegion;
	if (!pRegion)
		return PLAYER_NOT_NEAR_WATER;

	if (!(pRegion->m_wFlags & REGFLAG_HASWATER))
		return PLAYER_NOT_NEAR_WATER;

	pRegionSet = CScene__GetRegionAttributes(&pThis->m_Scene, pRegion);
	if (!pRegionSet)
		return PLAYER_NOT_NEAR_WATER;

	// is player in a water region ?
	vEyepos = CEngineApp__GetEyePos(pThis);
	if (vEyepos.y <= pRegionSet->m_WaterElevation)
	{
		// player is below water surface level
		if (pRegionSet->m_dwFlags & REGFLAG_SHALLOWWATER)
			waterFlag = PLAYER_ON_WATERSURFACE;
		else
			waterFlag = PLAYER_BELOW_WATERSURFACE;
	}
	else if (pPlayer->ah.ih.m_vPos.y > pRegionSet->m_WaterElevation)
	{
		// player is above water surface level
		waterFlag = PLAYER_ABOVE_WATERSURFACE;
	}
	else
	{
		// player is on water surface
		waterFlag = PLAYER_ON_WATERSURFACE;
	}

	return waterFlag;
}

BOOL CEngineApp__PlayerInShallowWater(CEngineApp *pThis)
{
	CGameObjectInstance	*pPlayer;
	CGameRegion				*pRegion;
	CROMRegionSet			*pRegionSet;
	int						waterFlag;


	waterFlag = CEngineApp__GetPlayerWaterStatus(pThis);
	if (waterFlag == PLAYER_ON_WATERSURFACE)
	{
		pPlayer = CEngineApp__GetPlayer(pThis);
		if (!pPlayer)
			return FALSE;

		pRegion = pPlayer->ah.ih.m_pCurrentRegion;
		if (!pRegion)
			return FALSE;

		pRegionSet = CScene__GetRegionAttributes(&pThis->m_Scene, pRegion);
		if (!pRegionSet)
			return FALSE;

		if (pRegionSet->m_dwFlags & REGFLAG_SHALLOWWATER)
			return TRUE;
	}
	return FALSE;
}

int CEngineApp__GetPlayerClimbStatus(CEngineApp *pThis)
{
	CGameObjectInstance	*pPlayer;
	CGameRegion				*pRegion;
	CROMRegionSet			*pRegionSet;


	pPlayer = CEngineApp__GetPlayer(GetApp());
	if (!pPlayer)
		return PLAYER_NOT_NEAR_CLIMBING_SURFACE;

	pRegion = pPlayer->ah.ih.m_pCurrentRegion;
	if (!pRegion)
		return PLAYER_NOT_NEAR_CLIMBING_SURFACE;

	if (!CGameRegion__IsCliff(pRegion) || !(pRegion->m_wFlags & REGFLAG_CLIMBABLE))
		return PLAYER_NOT_NEAR_CLIMBING_SURFACE;

	pRegionSet = CScene__GetRegionAttributes(&pThis->m_Scene, pRegion);
	if (!pRegionSet)
		return PLAYER_NOT_NEAR_CLIMBING_SURFACE;

	if (!CGameObjectInstance__IsFacingCliff(pPlayer))
		return PLAYER_NOT_NEAR_CLIMBING_SURFACE;

	// player is at climbing surface
	return PLAYER_AT_CLIMBING_SURFACE;
}


// returns distance to bottom of cliff
// <0 means that player is below bottom of cliff
// +x is distance player is above bottom of cliff
//
float CEngineApp__GetPlayerHeightFromBottomOfCliff(CEngineApp *pThis)
{
	CGameObjectInstance	*pPlayer;

	pPlayer = CEngineApp__GetPlayer(GetApp());
	if (pPlayer)
	{
		return CInstanceHdr__GetCliffDistance(&pPlayer->ah.ih, FALSE, 30*SCALING_FACTOR);
	}
	else
	{
		return 0.0;
	}
}


// returns distance to top of cliff
// <0 means that player is above top of cliff
// +x is distance player is below top of cliff
//
float CEngineApp__GetPlayerHeightFromTopOfCliff(CEngineApp *pThis)
{
	CGameObjectInstance	*pPlayer;

	pPlayer = CEngineApp__GetPlayer(GetApp());
	if (pPlayer)
	{
		return CInstanceHdr__GetCliffDistance(&pPlayer->ah.ih, TRUE, 30*SCALING_FACTOR);
	}
	else
	{
		return 0.0;
	}
}


int CEngineApp__GetPlayerDuckStatus(CEngineApp *pThis)
{
	CGameObjectInstance	*pPlayer;
	CGameRegion				*pRegion;
	float						nDist;


	pPlayer = CEngineApp__GetPlayer(GetApp());
	if (!pPlayer)
		return PLAYER_NOT_DUCKING;

	pRegion = pPlayer->ah.ih.m_pCurrentRegion;
	if (!pRegion)
		return PLAYER_NOT_DUCKING;

	nDist = CGameRegion__GetGroundHeight(pRegion, pPlayer->ah.ih.m_vPos.x, pPlayer->ah.ih.m_vPos.z) - pPlayer->ah.ih.m_vPos.y;
	if (nDist > 1.5*SCALING_FACTOR)
		return PLAYER_NOT_DUCKING;

	if (pRegion->m_wFlags & REGFLAG_DUCKENTRANCEEXIT)
		return PLAYER_DUCKING_ENTRANCE_EXIT;

	if (pRegion->m_wFlags & REGFLAG_DUCK)
		return PLAYER_DUCKING;

	return PLAYER_NOT_DUCKING;
}


/*****************************************************************************
*
*	Function:		CEngineApp__SetupDraw()
*
******************************************************************************
*
*	Description:	Sets up the initial state of the display list with fog,
*						zbuffer, dither etc....
*
*	Inputs:			*pThis	-	Ptr to engine
*	Outputs:			None
*
*****************************************************************************/
void CEngineApp__SetupDraw(CEngineApp *pThis)
{
	BYTE	r, g, b;

	// must be after CCamera__Update()
	CCamera__DisplayListSetup(&pThis->m_Camera, &pThis->m_pDLP);


	gDPSetTextureFilter(pThis->m_pDLP++, G_TF_BILERP);
  	//gDPSetTextureFilter(pThis->m_pDLP++, G_TF_POINT);

#ifdef SCREEN_SHOT
  	gDPSetColorDither(pThis->m_pDLP++, G_CD_ENABLE);
	gDPSetAlphaDither(pThis->m_pDLP++, G_AD_NOISE);
#else
	// G_AD_PATTERN produces darker pixels on the cloud border
	// G_AD_NOTPATTERN produces lighter pixels on the cloud border

  	gDPSetColorDither(pThis->m_pDLP++, G_CD_MAGICSQ);
	gDPSetAlphaDither(pThis->m_pDLP++, G_AD_NOTPATTERN);
	//gDPSetAlphaDither(pThis->m_pDLP++, G_AD_PATTERN);
#endif	// SCREEN_SHOT

#ifdef USE_FOG
	gSPSetGeometryMode(pThis->m_pDLP++, G_FOG);

	//gSPFogPosition(pThis->m_pDLP++, 992, 1000);
	gSPFogPosition(pThis->m_pDLP++, (unsigned short) pThis->m_FogStart, 1000);

	CEngineApp__GetFogColor(pThis, &r, &g, &b);
   gDPSetFogColor(pThis->m_pDLP++, r, g, b, 255);
#endif
   //gDPSetCycleType(pThis->m_pDLP++, G_CYC_2CYCLE);

	CTextureLoader__InvalidateTextureCache();
	CAnimationMtxs__Update() ;
	CFxSystem__Update(&pThis->m_FxSystem) ;

	if (GetApp()->m_dwCheatFlags & CHEATFLAG_QUACKMODE)
	  	gDPSetTextureFilter(pThis->m_pDLP++, G_TF_POINT);
}


/*****************************************************************************
*
*	Function:		CEngineApp__DrawLEGALSCREEN()
*
******************************************************************************
*
*	Description:	Draw code for LEGALSCREEN mode
*
*	Inputs:			*pThis	-	Ptr to engine
*	Outputs:			None
*
*****************************************************************************/
void CEngineApp__DrawLEGALSCREEN(CEngineApp *pThis)
{
//	pThis->m_FogColor[0] = 0 ;
//	pThis->m_FogColor[1] = 0 ;
//	pThis->m_FogColor[2] = 0 ;
//	pThis->m_FogColor[3] = 0 ;
//	pThis->m_LastFogColor[0] = 0 ;
//	pThis->m_LastFogColor[1] = 0 ;
//	pThis->m_LastFogColor[2] = 0 ;
//	pThis->m_LastFogColor[3] = 0 ;

//	pThis->m_Scene.m_pPlayer = NULL ;



	CEngineApp__SetupDraw(pThis) ;

	CScene__Draw(&pThis->m_Scene, &pThis->m_pDLP);

	CLegalScreen__Draw(&pThis->m_pDLP);

	CEngineApp__DrawFade(pThis) ;
}

/*****************************************************************************
*
*	Function:		CEngineApp__DrawGAMEPAK()
*
******************************************************************************
*
*	Description:	Draw code for GAMEPAK manager mode
*
*	Inputs:			*pThis	-	Ptr to engine
*	Outputs:			None
*
*****************************************************************************/
void CEngineApp__DrawGAMEPAK(CEngineApp *pThis)
{
//	pThis->m_FogColor[0] = 0 ;
//	pThis->m_FogColor[1] = 0 ;
//	pThis->m_FogColor[2] = 0 ;
//	pThis->m_FogColor[3] = 0 ;
//	pThis->m_LastFogColor[0] = 0 ;
//	pThis->m_LastFogColor[1] = 0 ;
//	pThis->m_LastFogColor[2] = 0 ;
//	pThis->m_LastFogColor[3] = 0 ;

//	pThis->m_Scene.m_pPlayer = NULL ;

	CEngineApp__SetupDraw(pThis) ;

	CGamePak__Draw(&pThis->m_GamePak, &pThis->m_pDLP);

	CEngineApp__DrawFade(pThis) ;
}


/*****************************************************************************
*
*	Function:		CEngineApp__DrawGALLERY()
*
******************************************************************************
*
*	Description:	Draw code for GALLERY mode
*
*	Inputs:			*pThis	-	Ptr to engine
*	Outputs:			None
*
*****************************************************************************/
void CEngineApp__DrawGALLERY(CEngineApp *pThis)
{
	BYTE	Alpha;
#define	X	17
#define	Y	18


	// draw name of character on screen
	if (pThis->m_PortraitAlpha)
	{
		Alpha = (BYTE) (pThis->m_PortraitAlpha*255);
		COnScreen__InitFontDraw(&pThis->m_pDLP) ;
		COnScreen__SetFontScale(1.0, 1.2) ;
		COnScreen__SetFontColor(&pThis->m_pDLP, 200, 200, 138, 86, 71,47) ; // default colors (from onscrn.cpp)

		switch(pThis->m_PortraitName)
		{
			case 0:	COnScreen__DrawText(&pThis->m_pDLP, text_beetle, X, Y, Alpha, FALSE, TRUE); break;
			case 1:	COnScreen__DrawText(&pThis->m_pDLP, text_dragonfly, X, Y, Alpha, FALSE, TRUE); break;
			case 2:	COnScreen__DrawText(&pThis->m_pDLP, text_campaignerssoldier, X, Y, Alpha, FALSE, TRUE); break;
			case 3:	COnScreen__DrawText(&pThis->m_pDLP, text_campaignerssoldier, X, Y, Alpha, FALSE, TRUE); break;
			case 4:	COnScreen__DrawText(&pThis->m_pDLP, text_campaignerssoldier, X, Y, Alpha, FALSE, TRUE); break;
			case 5:	COnScreen__DrawText(&pThis->m_pDLP, text_campaignerssoldier, X, Y, Alpha, FALSE, TRUE); break;
			case 6:	COnScreen__DrawText(&pThis->m_pDLP, text_campaignersseargent, X, Y, Alpha, FALSE, TRUE); break;
			case 7:	COnScreen__DrawText(&pThis->m_pDLP, text_poacher, X, Y, Alpha, FALSE, TRUE); break;
			case 8:	COnScreen__DrawText(&pThis->m_pDLP, text_poacher, X, Y, Alpha, FALSE, TRUE); break;
			case 9:	COnScreen__DrawText(&pThis->m_pDLP, text_raptor, X, Y, Alpha, FALSE, TRUE); break;
			case 10:	COnScreen__DrawText(&pThis->m_pDLP, text_cyborgraptor, X, Y, Alpha, FALSE, TRUE); break;
			case 11:	COnScreen__DrawText(&pThis->m_pDLP, text_leaper, X, Y, Alpha, FALSE, TRUE); break;
			case 12:	COnScreen__DrawText(&pThis->m_pDLP, text_hulk, X, Y, Alpha, FALSE, TRUE); break;
			case 13:	COnScreen__DrawText(&pThis->m_pDLP, text_alieninfantry, X, Y, Alpha, FALSE, TRUE); break;
			case 14:	COnScreen__DrawText(&pThis->m_pDLP, text_ancientwarrior, X, Y, Alpha, FALSE, TRUE); break;
			case 15:	COnScreen__DrawText(&pThis->m_pDLP, text_ancientwarrior, X, Y, Alpha, FALSE, TRUE); break;
			case 16:	COnScreen__DrawText(&pThis->m_pDLP, text_ancientwarrior, X, Y, Alpha, FALSE, TRUE); break;
			case 17:	COnScreen__DrawText(&pThis->m_pDLP, text_highpriest, X, Y, Alpha, FALSE, TRUE); break;
			case 18:	COnScreen__DrawText(&pThis->m_pDLP, text_cyborg, X, Y, Alpha, FALSE, TRUE); break;
			case 19:	COnScreen__DrawText(&pThis->m_pDLP, text_cyborg, X, Y, Alpha, FALSE, TRUE); break;
			case 20:	COnScreen__DrawText(&pThis->m_pDLP, text_cyborg, X, Y, Alpha, FALSE, TRUE); break;
			case 21:	COnScreen__DrawText(&pThis->m_pDLP, text_demon, X, Y, Alpha, FALSE, TRUE); break;
			case 22:	COnScreen__DrawText(&pThis->m_pDLP, text_demon, X, Y, Alpha, FALSE, TRUE); break;
			case 23:	COnScreen__DrawText(&pThis->m_pDLP, text_demon, X, Y, Alpha, FALSE, TRUE); break;
			case 24:	COnScreen__DrawText(&pThis->m_pDLP, text_demonlord, X, Y, Alpha, FALSE, TRUE); break;
			case 25:	COnScreen__DrawText(&pThis->m_pDLP, text_subterranean, X, Y, Alpha, FALSE, TRUE); break;
			case 26:	COnScreen__DrawText(&pThis->m_pDLP, text_sludgebeast, X, Y, Alpha, FALSE, TRUE); break;
			case 27:	COnScreen__DrawText(&pThis->m_pDLP, text_killerplant, X, Y, Alpha, FALSE, TRUE); break;
			case 28:	COnScreen__DrawText(&pThis->m_pDLP, text_robot, X, Y, Alpha, FALSE, TRUE); break;
			case 29:	COnScreen__DrawText(&pThis->m_pDLP, text_triceratops, X, Y, Alpha, FALSE, TRUE); break;
			case 30:	COnScreen__DrawText(&pThis->m_pDLP, text_dimetrodon, X, Y, Alpha, FALSE, TRUE); break;
		}
	}
}

/*****************************************************************************
*
*	Function:		CEngineApp__DrawTITLE()
*
******************************************************************************
*
*	Description:	Draw code for TITLE mode
*
*	Inputs:			*pThis	-	Ptr to engine
*	Outputs:			None
*
*****************************************************************************/
void CEngineApp__DrawTITLE(CEngineApp *pThis)
{
	pThis->m_TitleAlpha += .1 * frame_increment ;
	if (pThis->m_TitleAlpha > 1.0)
		pThis->m_TitleAlpha = 1.0 ;


	CEngineApp__SetupDraw(pThis) ;

	CScene__Draw(&pThis->m_Scene, &pThis->m_pDLP);

	if (pThis->m_dwCheatFlags & CHEATFLAG_PENANDINKMODE)
	{
		gDPPipeSync(pThis->m_pDLP++);
		gDPSetCycleType(pThis->m_pDLP++, G_CYC_1CYCLE);
		gDPSetBlendColor(pThis->m_pDLP++, 255, 255, 255, 255);
		gDPSetPrimDepth(pThis->m_pDLP++, 0xffff, 0xffff);
		gDPSetDepthSource(pThis->m_pDLP++, G_ZS_PRIM);
		CGeometry__SetRenderMode(&pThis->m_pDLP, G_RM_VISCVG__G_RM_VISCVG2);
		gDPFillRectangle(pThis->m_pDLP++, 0, 0, SCREEN_WD-1, SCREEN_HT-1);
	}

	// prepare to font textures
	// using billinear interpolation for scaled bitmaps
	COnScreen__InitFontDraw(&pThis->m_pDLP) ;


	// draw all menu screens last, over everything else
	if (pThis->m_Pause.m_Mode != PAUSE_NULL)
		CPause__Draw(&pThis->m_Pause, &pThis->m_pDLP) ;


//	if (pThis->m_GamePak.m_Mode != GAMEPAK_NULL)
//		CGamePak__Draw(&pThis->m_GamePak, &pThis->m_pDLP) ;


	CEngineApp__DrawFade(pThis) ;
}



/*****************************************************************************
*
*	Function:		CEngineApp__DrawGAME()
*
******************************************************************************
*
*	Description:	Draw code for GAME mode
*
*	Inputs:			*pThis	-	Ptr to engine
*	Outputs:			None
*
*****************************************************************************/
extern	UINT8		GameOverOverlay[] ;

//char	buffer1[32] ;
//char	buffer2[32] ;
char			timebuffer[8] ;

void CEngineApp__DrawGAME(CEngineApp *pThis)
{
	float		x ;
	int		time, i ;
	C16BitGraphic	*Grid ;
//	DWORD		dls, dle;

//	sprintf(buffer1, "level %d boss lvl %d", pThis->m_Level, pThis->m_BossLevel) ;
//	sprintf(buffer2, "warp %d cur %d", pThis->m_WarpID, pThis->m_CurrentWarpID) ;
//	COnScreen__AddGameTextWithId(buffer1, 32) ;
//	COnScreen__AddGameTextWithId(buffer2, 33) ;

	CEngineApp__SetupDraw(pThis) ;

	CScene__Draw(&pThis->m_Scene, &pThis->m_pDLP);

	if (pThis->m_dwCheatFlags & CHEATFLAG_PENANDINKMODE)
	{
		gDPPipeSync(pThis->m_pDLP++);
		gDPSetCycleType(pThis->m_pDLP++, G_CYC_1CYCLE);
		gDPSetBlendColor(pThis->m_pDLP++, 255, 255, 255, 255);
		gDPSetPrimDepth(pThis->m_pDLP++, 0xffff, 0xffff);
		gDPSetDepthSource(pThis->m_pDLP++, G_ZS_PRIM);
		CGeometry__SetRenderMode(&pThis->m_pDLP, G_RM_VISCVG__G_RM_VISCVG2);
		gDPFillRectangle(pThis->m_pDLP++, 0, 0, SCREEN_WD-1, SCREEN_HT-1);
	}

	CEngineApp__DrawWater(pThis);
	CEngineApp__DrawColorFlash(pThis);
	CEngineApp__DrawFlash(pThis);

//	dls = (DWORD) pThis->m_pDLP;

	// Only draw on screen info if not in letter box or cinematic mode
	if (		!CCamera__InLetterBoxMode(&GetApp()->m_Camera)
			&&	!CCamera__InCinemaMode(&GetApp()->m_Camera) )
	{
#ifdef PLATFORM_PORT
		/* PORT: the HUD overlay graphics (C16BitGraphic m_BlocksAcross/Down + per-block
		 * C16BitPart m_BlockWidth/Height) are big-endian static blobs; the header fields are now
		 * byte-swapped at read time (ONSCRN_SW16/32 in onscrn.c), so the HUD draws correctly.
		 * Default ON now; TUROK_HUD=0 disables it (e.g. for clean geometry captures). */
		{
			extern char *getenv(const char *);
			const char *h = getenv("TUROK_HUD");
			if (h && *h == '0')
				goto skip_hud;
		}
#endif
		COnScreen__Draw(&pThis->m_OnScreen, &pThis->m_pDLP);
#ifdef PLATFORM_PORT
	skip_hud: ;
#endif
	}


	// Display game text
	COnScreen__UpdateGameText(&pThis->m_pDLP) ;


	// if player dead, draw game over
	if (pThis->m_bGameOver)
	{
		Grid = (C16BitGraphic *)GameOverOverlay ;

		if (pThis->m_GameOverMode == 0)
		{
			pThis->m_GameOverAlpha += 16 * frame_increment ;
			if (pThis->m_GameOverAlpha > 255)
			{
				pThis->m_GameOverAlpha = 255 ;
				pThis->m_GameOverMode = 1 ;
			}
		}

		pThis->m_GameOverTime -= 1*frame_increment ;
		if (pThis->m_GameOverTime < 0)
			CEngineApp__SetupFadeTo(pThis, MODE_RESETGAME);

		// prepare to draw
		COnScreen__Init16BitDraw(&pThis->m_pDLP, pThis->m_GameOverAlpha) ;
		x = (SCREEN_WD/2) - (Grid->m_Width /2) ;
   	COnScreen__Draw16BitGraphic(&pThis->m_pDLP,
											Grid,
											x, (SCREEN_HT/2) - (Grid->m_Height/2)) ;
	}

	// draw all menu screens last, over everything else
	if (pThis->m_Pause.m_Mode != PAUSE_NULL)
		CPause__Draw(&pThis->m_Pause, &pThis->m_pDLP) ;


	// draw the training text
	if ((pThis->m_Training.m_Mode != TRAINING_NULL) && (pThis->m_bTrainingMode))
	{
		if (!((pThis->m_Training.m_Mode == TRAINING_FADEDOWN) && (pThis->m_Training.m_Alpha == 0.0)))
		{
			//sprintf(buffer, "mode %d", pThis->m_Training.m_Mode) ;
			//COnScreen__AddGameTextWithId(buffer, 45) ;
			CTraining__Draw(&pThis->m_Training, &pThis->m_pDLP) ;
		}
	}

	// draw the training clock...
	if ((pThis->m_bTrainingMode) && (pThis->m_TrainingType))
	{
		// increase time, if not finished
		if (		(pThis->m_bTraining == FALSE)
				&& (pThis->m_bRequestor == FALSE))
		{
			pThis->m_LevelTime += frame_increment ;
			if (pThis->m_LevelTime >= ((999*60*FRAME_FPS)+(60*FRAME_FPS)))
				pThis->m_LevelTime -= ((999*60*FRAME_FPS)+(60*FRAME_FPS)) ;
		}
		else if (pThis->m_bTrainingMode == 2)
		{
			if (pThis->m_LevelTime < pThis->m_BestTrainTime)
				pThis->m_BestTrainTime = pThis->m_LevelTime ;
		}


		COnScreen__InitFontDraw(&pThis->m_pDLP) ;
		COnScreen__PrepareSmallFont(&pThis->m_pDLP) ;

		COnScreen__SetFontColor(&pThis->m_pDLP, 255, 55, 255, 255, 255, 255) ;
#ifndef GERMAN
		COnScreen__DrawText(&pThis->m_pDLP, "time", (320/2)-(80/2), 22, 255, TRUE, TRUE) ;
#else
		COnScreen__DrawText(&pThis->m_pDLP, "zeit", (320/2)-(80/2), 22, 255, TRUE, TRUE) ;
#endif

		COnScreen__SetFontColor(&pThis->m_pDLP, 255, 255, 55, 255, 255, 255) ;
#ifndef GERMAN
		COnScreen__DrawText(&pThis->m_pDLP, "best time", (320/2)+(80/2), 22, 255, TRUE, TRUE) ;
#else
		COnScreen__DrawText(&pThis->m_pDLP, "bestzeit", (320/2)+(80/2), 22, 255, TRUE, TRUE) ;
#endif

		COnScreen__SetFontScale(1.0, 2.0) ;
		time = pThis->m_BestTrainTime / FRAME_FPS ;
		sprintf(timebuffer, "%02d:%02d", time/60, time%60) ;
		COnScreen__DrawText(&pThis->m_pDLP, timebuffer, (320/2)+(80/2), 32, 255, TRUE, TRUE) ;

		time = pThis->m_LevelTime / FRAME_FPS ;
		i = 192+(63 * cos((float)game_frame_number/3)) ;
		COnScreen__SetFontColor(&pThis->m_pDLP, i, i, i, i, i, i) ;
		sprintf(timebuffer, "%02d:%02d", time/60, time%60) ;
		COnScreen__DrawText(&pThis->m_pDLP, timebuffer, (320/2)-(80/2), 32, 255, TRUE, TRUE) ;
	}

//	dle = (DWORD) pThis->m_pDLP;
//	COnScreen__Init16BitDraw(&pThis->m_pDLP, 255);
//	COnScreen__DrawDigits(&pThis->m_pDLP, (dle - dls)/sizeof(Gfx), 50, 50);

	// must be last if you wanna fade everything...
	if (pThis->m_MapMode == 0)
	{
		CEngineApp__DrawSunGlare(pThis);
		CEngineApp__DrawWarp(pThis);
		CEngineApp__DrawDeath(pThis);
		CEngineApp__DrawFade(pThis);
	}

}


/*****************************************************************************
*
*	Function:		CEngineApp__DrawCREDITS()
*
******************************************************************************
*
*	Description:	Draw code for CREDITS mode
*
*	Inputs:			*pThis	-	Ptr to engine
*	Outputs:			None
*
*****************************************************************************/
void CEngineApp__DrawCREDITS(CEngineApp *pThis)
{
//	static	float	xscale_vel = 3.0 ;
//	static	float	xscale_pos = 0 ;
//	static	float	yscale_vel = 2.0 ;
//	static	float	yscale_pos = 0 ;


	CEngineApp__SetupDraw(pThis) ;

	CScene__Draw(&pThis->m_Scene, &pThis->m_pDLP);

	// prepare to font textures
	// using billinear interpolation for scaled bitmaps
//	COnScreen__InitFontDraw(&pThis->m_pDLP) ;

//	COnScreen__SetFontScale(2.0+xscale_pos, 2.0+yscale_pos) ;
// 	COnScreen__SetFontColor(&pThis->m_pDLP,
//									  200,0,200,
//									  0,200,0) ;
//	COnScreen__DrawText(&pThis->m_pDLP,
//								  "credits",
//								  320/2, 40,
//								  255, TRUE, TRUE) ;
//
//	xscale_pos -= xscale_vel * .02 ;
//	xscale_vel += xscale_pos ;
//	yscale_pos -= yscale_vel * .01 ;
//	yscale_vel += yscale_pos ;

	CCredits__Draw(&pThis->m_pDLP);

	// draw lens flare for sun
	COnScreen__Init16BitDraw(&pThis->m_pDLP, 100) ;		// not neccessarily alpha
	COnScreen__DrawSunLensFlare(&pThis->m_OnScreen, &pThis->m_pDLP);


	if (pThis->m_Pause.m_Mode != PAUSE_NULL)
		CPause__Draw(&pThis->m_Pause, &pThis->m_pDLP) ;


	CEngineApp__DrawFade(pThis) ;
}




void CEngineApp__DoFlash(CEngineApp *pThis, u8 Flash, u8 FlashDec)
{
	CGameObjectInstance	*pPlayer;
	CVector3					vPos;
	int						Sound,
								waterFlag;


	pThis->m_Flash    = Flash;
	pThis->m_FlashDec = FlashDec;

	waterFlag = CEngineApp__GetPlayerWaterStatus(pThis);

	pPlayer = CEngineApp__GetPlayer(pThis);
	if (pPlayer)
	{
		//vPos = CAnimInstanceHdr__GetPosWithGround(&pPlayer->ah);
		vPos = pPlayer->ah.ih.m_vPos;

		if (pPlayer->m_AI.m_Health <= 0)
		{
			// dying underwater or above water
			if (		(waterFlag == PLAYER_BELOW_WATERSURFACE)
					&&	(!CTurokMovement.InAntiGrav) )
			{
				// dying underwater
				Sound = SOUND_HUMAN_EFFORTINJURY_GRUNT_3;
			}
			else
			{
				// dying above water
				Sound = SOUND_GENERIC_18;
			}

			CScene__DoSoundEffect(&pThis->m_Scene,
										  Sound, 1,
										  pPlayer, &pPlayer->ah.ih.m_vPos, 0);
		}
		else
		{
/*
	CScene__DoSoundEffect(&pThis->m_Scene,
								  SOUND_HUMAN_EFFORTINJURY_GRUNT_1, 1,
								  pPlayer, &pPlayer->ah.ih.m_vPos);
*/
		}
	}
}

void CEngineApp__DoColorFlash(CEngineApp *pThis, u8 Alpha, u8 AlphaDec, int r, int g, int b)
{
	pThis->m_Alpha		= Alpha ;
	pThis->m_AlphaDec = AlphaDec;

	pThis->m_FlashColor[0] = r ;
	pThis->m_FlashColor[1] = g ;
	pThis->m_FlashColor[2] = b ;
}


void CEngineApp__RenderTint(CEngineApp *pThis, BYTE Color[4])
{
	// Any tint there?
	if (!Color[3])
		return;

	gDPPipeSync(pThis->m_pDLP++);

	gDPSetCycleType(pThis->m_pDLP++, G_CYC_1CYCLE);
	CGeometry__SetRenderMode(&pThis->m_pDLP, G_RM_CLD_SURF__G_RM_CLD_SURF2);

	CGeometry__SetCombineMode(&pThis->m_pDLP, G_CC_PRIMITIVE__G_CC_PRIMITIVE);

	gDPSetPrimColor(pThis->m_pDLP++,
						 255, 255,											// LOD stuff
						 Color[0], Color[1], Color[2],				// r g b
						 Color[3]);											// a

	gDPScisFillRectangle(pThis->m_pDLP++, 0, 0, SCREEN_WD, SCREEN_HT);
}

// Checks camera mode before rendering tint
void CEngineApp__DrawTint(CEngineApp *pThis, BYTE Color[4])
{
	// If in cinematic/letterbox mode - don't draw tint!
	if ((CCamera__InCinemaMode(&pThis->m_Camera)) || (CCamera__InLetterBoxMode(&pThis->m_Camera)))
		return ;

	CEngineApp__RenderTint(pThis, Color) ;
}



void CEngineApp__DrawFlash(CEngineApp *pThis)
{
	BYTE	flashColor[4];

	if (pThis->m_Flash > pThis->m_FlashDec)
		pThis->m_Flash -= (u8) (pThis->m_FlashDec * frame_increment);
	else
		pThis->m_Flash = 0;

	if (pThis->m_Flash)
	{
		flashColor[0] = 255;
		flashColor[1] = 0;
		flashColor[2] = 0;
		flashColor[3] = pThis->m_Flash;

		CEngineApp__DrawTint(pThis, flashColor);
	}
}

void CEngineApp__DrawColorFlash(CEngineApp *pThis)
{
	if (pThis->m_Alpha > pThis->m_AlphaDec)
		pThis->m_Alpha -= (u8) (pThis->m_AlphaDec * frame_increment);
	else
		pThis->m_Alpha = 0;

	if (pThis->m_Alpha)
	{
		pThis->m_FlashColor[3] = pThis->m_Alpha;

		CEngineApp__DrawTint(pThis, pThis->m_FlashColor);
	}
}

void CEngineApp__DrawFade(CEngineApp *pThis)
{
	BYTE	fadecolor[4];
	FLOAT	FadeDownSpeed, FadeUpSpeed ;

	if (pThis->m_FadeStatus != FADE_NULL)
	{
		// Use quicker fade speed if a cinematic happening/about to happen
		if ((pThis->m_FadeFast) || (pThis->m_CinemaFlags))
		{
			FadeUpSpeed = 0.5 ;
			FadeDownSpeed = 0.5 ;
		}
		else
		{
			FadeUpSpeed = 0.1 ;
			FadeDownSpeed = 0.3 ;
		}

		switch (pThis->m_FadeStatus)
		{
			case FADE_UP:

				// Fudge to be black for 2 extra frames
				if (pThis->m_FadeAlpha < (0.001*3))
					pThis->m_FadeAlpha += 0.001 ;
				else
					pThis->m_FadeAlpha += FadeUpSpeed * frame_increment ;

				if (pThis->m_FadeAlpha >= 1.0)
				{
					pThis->m_FadeAlpha = 1.0 ;
					pThis->m_FadeStatus = FADE_NULL ;
					pThis->m_FadeFast = FALSE ;
				}
				break ;

			case FADE_DOWN:
				pThis->m_FadeAlpha -= FadeDownSpeed * frame_increment ;
				if (pThis->m_FadeAlpha <= 0.0)
				{
					pThis->m_FadeAlpha = 0.0 ;
					pThis->m_FadeStatus = FADE_NULL ;
				}
				break ;
		}

		fadecolor[0] = 0 ;
		fadecolor[1] = 0 ;
		fadecolor[2] = 0 ;

		// Set alpha
		fadecolor[3] = 255-(int)(255*pThis->m_FadeAlpha) ;

		CEngineApp__RenderTint(pThis, fadecolor) ;
	}
}

void CEngineApp__UpdateFade(CEngineApp *pThis)
{
	// wait until faded down to switch modes
	if (pThis->m_FadeAlpha == 0.0)
  	{
		CEngineApp__Reset(pThis, pThis->m_NextMode);
  		pThis->m_FadeStatus = FADE_UP ;
  	}
}

void CEngineApp__SetupFadeTo(CEngineApp *pThis, DWORD Mode)
{
	pThis->m_FadeStatus = FADE_DOWN ;
	pThis->m_NextMode = Mode ;

	// Call mode constructors here when needed
//	if (Mode == MODE_LEGALSCREEN)
//		CLegalScreen__Construct() ;
}



#define	PORTRAITNAME_APPEAR			1
#define	PORTRAITNAME_ON				2
#define	PORTRAITNAME_DISAPPEAR		3
#define	PORTRAITNAME_OFF				4

void CEngineApp__ResetGallery(CEngineApp *pThis)
{
	pThis->m_PortraitZoom = 30 * SCALING_FACTOR;
	pThis->m_ActualPortraitZoom = pThis->m_PortraitZoom;
	pThis->m_ActualPortraitRotXSpd = 0;
	pThis->m_ActualPortraitRotYSpd = 0;
	pThis->m_ActualPortraitRotZSpd = 0;
	pThis->m_PortraitRotXSpd = 0;
	pThis->m_PortraitRotYSpd = 0;
	pThis->m_PortraitRotZSpd = 0;
	CQuatern__Identity(&pThis->m_qPortraitOrientation);
	pThis->m_PortraitTimer = 0.0;

	// if name is displayed make it disappear
	pThis->m_PortraitNameMode = PORTRAITNAME_DISAPPEAR;
}


void CEngineApp__UpdateGallery(CEngineApp *pThis)
{
	float						resl, resr, resu, resd, reszl, reszr, dm, zoommax, dist, aidist;
	CQuatern					qRot, qFinal ;
	int						nAIs, cAI;
	CGameObjectInstance	*pAI,
								*pClosestAI;
	CVector3					vDelta,
								vPos;

#define	GALLERY_MAX_CHARACTERS		31
#define	GALLERY_WIDTH					5
//#define	GALLERY_HEIGHT					5
#define	PORTRAIT_START_X				(-239.19 *SCALING_FACTOR)
#define	PORTRAIT_START_Z				( 358.80 *SCALING_FACTOR)
#define	GALLERY_GAP						( 119.60 *SCALING_FACTOR)
#define	GALLERY_MIN_ZOOM				(50*SCALING_FACTOR)
#define	PORTRAIT_USE_TIMER			3.0

	// exiting the gallery ?
	if (CTControl__IsStart(pCTControl) && pThis->m_PortraitExit == FALSE)
	{
		pThis->m_PortraitExit = TRUE;
		pThis->m_PortraitNameMode = PORTRAITNAME_DISAPPEAR;
	}

	// fade name in
	switch(pThis->m_PortraitNameMode)
	{
		case PORTRAITNAME_APPEAR:
			pThis->m_PortraitAlpha += (frame_increment/5);
			if (pThis->m_PortraitAlpha > 1.0)
			{
				pThis->m_PortraitAlpha = 1.0;
				pThis->m_PortraitNameMode = PORTRAITNAME_ON;
			}
			break;

		case PORTRAITNAME_ON:
			break;

		case PORTRAITNAME_DISAPPEAR:
			pThis->m_PortraitAlpha -= (frame_increment/5);
			if (pThis->m_PortraitAlpha < 0.0)
			{
				pThis->m_PortraitAlpha = 0.0;
				pThis->m_PortraitNameMode = PORTRAITNAME_OFF;
			}
			break;

		case PORTRAITNAME_OFF:
			if (pThis->m_PortraitExit)
			{
				pThis->m_WarpID = 0;
				CEngineApp__SetupFadeTo(pThis, MODE_RESETGAME);
				return;
			}
			else
			{
				pThis->m_PortraitNameMode = PORTRAITNAME_APPEAR;
				pThis->m_PortraitName = pThis->m_Portrait;
			}
			break;

	}

	// exiting gallery ?
	if (pThis->m_PortraitExit)
		return;

	// is use timer on ?
	if (pThis->m_PortraitTimer > 0.0)
	{
		pThis->m_PortraitTimer -= frame_increment*(1.0/FRAME_FPS);
		if (pThis->m_PortraitTimer < 0.0)
			pThis->m_PortraitTimer = 0.0;

		pThis->m_PortraitNameMode = PORTRAITNAME_DISAPPEAR;
	}


	// moving to next character ?
	if (CTControl__IsClickRight(pCTControl))
	{
		pThis->m_Portrait++;
		if (pThis->m_Portrait >= GALLERY_MAX_CHARACTERS)
			pThis->m_Portrait = 0;

		CEngineApp__ResetGallery(pThis);
	}
	// moving to previous character ?
	else if (CTControl__IsClickLeft(pCTControl))
	{
		pThis->m_Portrait--;
		if (pThis->m_Portrait < 0)
			pThis->m_Portrait = GALLERY_MAX_CHARACTERS-1;

		CEngineApp__ResetGallery(pThis);
	}

	// get closest ai to camera
	nAIs = CEngineApp__GetAnimInstanceCount(pThis);
	dist = GALLERY_GAP*2;
	pClosestAI = NULL;
	vPos.x = (pThis->m_Portrait%GALLERY_WIDTH)  *  GALLERY_GAP + PORTRAIT_START_X;
	vPos.y = 0;
	vPos.z = (pThis->m_Portrait/GALLERY_WIDTH) * -GALLERY_GAP + PORTRAIT_START_Z;

	for (cAI=0; cAI<nAIs; cAI++)
	{
		pAI = CEngineApp__GetAnimInstance(pThis, cAI);
		CVector3__Subtract(&vDelta, &vPos, &pAI->ah.ih.m_vPos);
		aidist = CVector3__Magnitude(&vDelta);

		if (aidist <= dist)
		{
			dist = aidist;
			pClosestAI = pAI;
		}
	}

	// get max zoom for this character
	if (pClosestAI)
		zoommax = pClosestAI->ah.ih.m_pEA->m_CollisionRadius;
	else
		zoommax = -1;


	if (CTControl__IsDigitalUp(pCTControl) && zoommax >= 0)
	{
		pThis->m_PortraitTimer = PORTRAIT_USE_TIMER;

		pThis->m_ActualPortraitZoom -= (1*SCALING_FACTOR);
		if (pThis->m_ActualPortraitZoom < zoommax)
			pThis->m_ActualPortraitZoom = zoommax;
	}
	else if (CTControl__IsDigitalDown(pCTControl))
	{
		pThis->m_PortraitTimer = PORTRAIT_USE_TIMER;

		pThis->m_ActualPortraitZoom += (1*SCALING_FACTOR);
		if (pThis->m_ActualPortraitZoom > GALLERY_MIN_ZOOM)
			pThis->m_ActualPortraitZoom = GALLERY_MIN_ZOOM;
	}

	dm = (pThis->m_ActualPortraitZoom - pThis->m_PortraitZoom) / 2;
	pThis->m_PortraitZoom += dm * frame_increment * 2;

	reszl = resl = CTControl__IsAnalogLeft(pCTControl);
	reszr = resr = CTControl__IsAnalogRight(pCTControl);
	resu = CTControl__IsAnalogUp(pCTControl);
	resd = CTControl__IsAnalogDown(pCTControl);

	// left & right toggled to do z rotation ?
	if (CTControl__IsZRotTog(pCTControl))
	{
		// do z rotation
		if (reszl) pThis->m_ActualPortraitRotZSpd = -(reszl/4);
		if (reszr) pThis->m_ActualPortraitRotZSpd = +(reszr/4);
		resl = resr = 0;
	}
	else
	{
		// do normal y rotation
		if (resl) pThis->m_ActualPortraitRotYSpd = -(resl/4);
		if (resr) pThis->m_ActualPortraitRotYSpd = +(resr/4);
		reszl = reszr = 0;

		// digital z rotation ?
		if (CTControl__IsZRotLeft(pCTControl))
		{
			reszl = 10;
			pThis->m_ActualPortraitRotZSpd = -reszl;
		}
		else if (CTControl__IsZRotRight(pCTControl))
		{
			reszr = 10;
			pThis->m_ActualPortraitRotZSpd = +reszr;
		}
	}

	if (resd) pThis->m_ActualPortraitRotXSpd = +(resd/4);
	if (resu) pThis->m_ActualPortraitRotXSpd = -(resu/4);

	if (!resd && !resu)
		pThis->m_ActualPortraitRotXSpd = 0;
	else
		pThis->m_PortraitTimer = PORTRAIT_USE_TIMER;

	if (!resl && !resr)
		pThis->m_ActualPortraitRotYSpd = 0;
	else
		pThis->m_PortraitTimer = PORTRAIT_USE_TIMER;

	if (!reszl && !reszr)
		pThis->m_ActualPortraitRotZSpd = 0;
	else
		pThis->m_PortraitTimer = PORTRAIT_USE_TIMER;

	dm = (pThis->m_ActualPortraitRotXSpd - pThis->m_PortraitRotXSpd) / 2;
	pThis->m_PortraitRotXSpd += dm;

	dm = (pThis->m_ActualPortraitRotYSpd - pThis->m_PortraitRotYSpd) / 2;
	pThis->m_PortraitRotYSpd += dm;

	dm = (pThis->m_ActualPortraitRotZSpd - pThis->m_PortraitRotZSpd) / 2;
	pThis->m_PortraitRotZSpd += dm;

	// Add x rotation
	CQuatern__BuildFromAxisAngle(&qRot, 1,0,0, ANGLE_DTOR(pThis->m_PortraitRotXSpd) *(60*FRAME_INC_TO_SECONDS(frame_increment))) ;
	CQuatern__Mult(&qFinal, &pThis->m_qPortraitOrientation, &qRot) ;
	CQuatern__Normalize(&qFinal) ;
	pThis->m_qPortraitOrientation = qFinal ;

	// Add y rotation
	CQuatern__BuildFromAxisAngle(&qRot, 0,1,0, ANGLE_DTOR(pThis->m_PortraitRotYSpd) *(60*FRAME_INC_TO_SECONDS(frame_increment))) ;
	CQuatern__Mult(&qFinal, &pThis->m_qPortraitOrientation, &qRot) ;
	CQuatern__Normalize(&qFinal) ;
	pThis->m_qPortraitOrientation = qFinal ;

	// Add z rotation
	CQuatern__BuildFromAxisAngle(&qRot, 0,0,1, ANGLE_DTOR(pThis->m_PortraitRotZSpd) *(60*FRAME_INC_TO_SECONDS(frame_increment))) ;
	CQuatern__Mult(&qFinal, &pThis->m_qPortraitOrientation, &qRot) ;
	CQuatern__Normalize(&qFinal) ;
	pThis->m_qPortraitOrientation = qFinal ;

	// set camera to look at portrait in gallery
	pThis->m_Camera.m_View.m_vEye.x = (pThis->m_Portrait%GALLERY_WIDTH)  *  GALLERY_GAP + PORTRAIT_START_X;
	pThis->m_Camera.m_View.m_vEye.z = (pThis->m_Portrait/GALLERY_WIDTH) * -GALLERY_GAP + PORTRAIT_START_Z - pThis->m_PortraitZoom;
	pThis->m_Camera.m_View.m_vFocus.x = (pThis->m_Portrait%GALLERY_WIDTH)  *  GALLERY_GAP + PORTRAIT_START_X;
	pThis->m_Camera.m_View.m_vFocus.z = (pThis->m_Portrait/GALLERY_WIDTH) * -GALLERY_GAP + PORTRAIT_START_Z;
}

void CEngineApp__DoDeath(CEngineApp *pThis)
{
	if (pThis->m_Death == DEATH_NOT_DIEING)
	{
		pThis->m_Death = DEATH_DIEING;
		pThis->m_uDeath = 0;
	}
}

#define DEATH_LENGTH			(1*FRAME_FPS)
void CEngineApp__DrawDeath(CEngineApp *pThis)
{
	BYTE	color[4] = { 255, 0, 0, 255 };


	switch (pThis->m_Death)
	{
		case DEATH_DIEING:
			pThis->m_uDeath += frame_increment/DEATH_LENGTH;
			color[3] = min(255, pThis->m_uDeath*255);

// removed because it looks crap & also screws up the game over
//			CEngineApp__DrawTint(pThis, color);

			if ((pThis->m_uDeath >= 1) || (CCamera__InCinemaMode(&pThis->m_Camera)))
			{
				pThis->m_Death = DEATH_NOT_DIEING;
				pThis->m_uDeath = 1 ;

				if (pThis->m_bPause)
				{
					pThis->m_bPause = FALSE ;
					pThis->m_Pause.m_Mode = PAUSE_FADEDOWN ;
				}

				CEngineApp__PlayerLostLife(pThis) ;
			}
			break;
	}
}


// decide what to do after death, GAME OVER, or continue with more lives...
void CEngineApp__PlayerLostLife(CEngineApp *pThis)
{
	if (!pThis->m_bGameOver)
	{
		if (CTMove__DecreaseLives(&CTurokMovement))
		{
			CEngineApp__NewLife(pThis) ;

			if (	(pThis->m_CurrentWarpID >=9000)
				&& (pThis->m_CurrentWarpID < 9999))
			{
				pThis->m_WarpID = RETURN_WARP_ID ;
			}
			else
			{
				pThis->m_CinemaFlags |= CINEMA_FLAG_PLAY_RESURRECT ;
			}
		}
		else
		{
			pThis->m_bGameOver = TRUE ;
			pThis->m_GameOverAlpha = 0 ;
			pThis->m_GameOverMode = 0 ;
			pThis->m_GameOverTime = SECONDS_TO_FRAMES(4) ;
		}
	}
}


void CEngineApp__WarpReturn(CEngineApp *pThis)
{
	CEngineApp__Warp(pThis, RETURN_WARP_ID, WARP_WITHINLEVEL, FALSE);
}



void CEngineApp__Warp(CEngineApp *pThis, int WarpID, int nType, BOOL StoreWarpReturn)
{
//	float Seconds[] = { 0.25, 0.5 };
	float Seconds[] = { 0.5, 0.5 };

	float SoundEffects[] = { SOUND_WARP_INOUT, SOUND_WARP_INOUT };

	float Reds[]	= { 255, 0 };
	float Greens[] = { 255, 0 };
	float Blues[]	= { 255, 100 };

	BOOL WarpIns[] = { TRUE, TRUE, FALSE };

	ASSERT((nType >= WARP_WITHINLEVEL) && (nType <= WARP_BETWEENLEVELS));

	// Skip boss levels if the boss is already dead
	switch(WarpID)
	{
		case LONGHUNTER_BOSS_START_WARP_ID:
			if (pThis->m_BossFlags & BOSS_FLAG_LONGHUNTER_DEAD)
				WarpID = KILLED_LONGHUNTER_BOSS_WARP_ID ;
			break ;

		case MANTIS_BOSS_WARP_ID:
			if (pThis->m_BossFlags & BOSS_FLAG_MANTIS_DEAD)
				WarpID = KILLED_MANTIS_BOSS_WARP_ID ;
			break ;

		case TREX_BOSS_WARP_ID:
			if (pThis->m_BossFlags & BOSS_FLAG_TREX_DEAD)
				WarpID = KILLED_TREX_BOSS_WARP_ID ;
			break ;

		case CAMPAIGNER_BOSS_WARP_ID:
			if (pThis->m_BossFlags & BOSS_FLAG_CAMPAIGNER_DEAD)
				WarpID = KILLED_CAMPAIGNER_BOSS_WARP_ID ;
			break ;
	}

	CEngineApp__DoWarp(pThis,
							 WarpID,
							 Seconds[nType],
							 SoundEffects[nType],
							 WarpIns[nType],
							 Reds[nType], Greens[nType], Blues[nType],
							 StoreWarpReturn);
}

void CEngineApp__DoWarp(CEngineApp *pThis,
								int WarpID,
								float Seconds,
								int SoundEffect,
								BOOL WarpIn,
								BYTE Red, BYTE Green, BYTE Blue,
								BOOL StoreWarpReturn)
{
	CGameObjectInstance *pPlayer;

	if (pThis->m_Warp == WARP_NOT_WARPING)
	{
		if (StoreWarpReturn)
		{
			pThis->m_ReturnWarpSaved = TRUE;
			pPlayer = CEngineApp__GetPlayer(pThis);
			pThis->m_ReturnWarp.m_vPos = pPlayer->ah.ih.m_vPos;
			pThis->m_ReturnWarp.m_RotY = pPlayer->m_RotY;
			pThis->m_ReturnWarp.m_nLevel = pThis->m_Scene.m_nLevel;
			pThis->m_ReturnWarp.m_nRegion = CScene__GetRegionIndex(&pThis->m_Scene, pPlayer->ah.ih.m_pCurrentRegion);
		}

		pThis->m_Warp = WARP_WARPING_OUT;
		pThis->m_uWarp = WarpIn ? 0 : 1;

		pThis->m_WarpID = WarpID;
		pThis->m_WarpLength = Seconds*FRAME_FPS;
		pThis->m_WarpSound = SoundEffect;

		pThis->m_WarpColors[0] = Red;
		pThis->m_WarpColors[1] = Green;
		pThis->m_WarpColors[2] = Blue;
	}
}

void CEngineApp__Reset(CEngineApp *pThis, DWORD NewMode)
{
	pThis->m_Mode = MODE_WAITFORDISPLAY;
	pThis->m_NextMode = NewMode;

	// Save bosses status before a cinema, unless reset is set
	if (!(pThis->m_BossFlags & BOSS_FLAG_RESET_LEVEL))
		CBossesStatus__SaveAllBossesStatus(&pThis->m_BossesStatus) ;
}

void CEngineApp__DrawSunGlare(CEngineApp *pThis)
{
	float		x, y, opacity,
				dx, dy, distSquared,
				opac2;
	BOOL		visible;
	BYTE		color[4] = { 255, 255, 255, 255 };

	CSun__GetSunData(&sun, &visible, &x, &y, &opacity);
	if (!visible)
		return;

	dx = x - SCREEN_WD/2;
	dy = y - SCREEN_HT/2;

	distSquared = dx*dx + dy*dy;

#define SUNGLARE_DIST 0.33
#define SUNGLARE_OPAC (220 & 0xf8)

	opac2 = (SUNGLARE_DIST - sqrt(distSquared)/(SCREEN_WD/2)) * opacity/SUNGLARE_DIST;
	if (opac2 > 0)
	{
		color[3] = (BYTE) min(SUNGLARE_OPAC, opac2*SUNGLARE_OPAC);
		CEngineApp__RenderTint(pThis, color);
	}
}

#define WARP_LENGTH			(0.25*FRAME_FPS)
void CEngineApp__DrawWarp(CEngineApp *pThis)
{
	CGameObjectInstance		*pTurok;

	pTurok = CEngineApp__GetPlayer(pThis);
	if (!pTurok)
		return;

	switch (pThis->m_Warp)
	{
		case WARP_WARPING_OUT:
			pThis->m_uWarp += frame_increment/pThis->m_WarpLength;
			pThis->m_WarpColors[3] = min(255, pThis->m_uWarp*255);
			CEngineApp__RenderTint(pThis, pThis->m_WarpColors);

			if (pThis->m_uWarp >= 1)
			{
				pThis->m_Warp = WARP_WARPING_IN;
				pThis->m_uWarp = 0;
				pThis->m_PlayedSound = FALSE;

				if (pThis->m_FadeStatus == FADE_NULL)
					CEngineApp__Reset(pThis, MODE_RESETLEVEL);

				//if (pThis->m_WarpID < 0)
				//	CScene__ReturnToLastWarp(&pThis->m_Scene, pTurok);
				//else
				//	CScene__WarpInstance(&pThis->m_Scene, pTurok, pThis->m_WarpID);
			}
			break;

		case WARP_WARPING_IN:
			// don't play sound until after first frame
			// so DMAs won't cut it off
			if ((pThis->m_WarpSound >= 0) && (pThis->m_uWarp > 0) && !pThis->m_PlayedSound)
			{
				pThis->m_PlayedSound = TRUE;

				CScene__DoSoundEffect(&pThis->m_Scene,
											 pThis->m_WarpSound, 1,
											 pTurok, &AW.Ears.vPos, 0);
			}

			pThis->m_uWarp += frame_increment/pThis->m_WarpLength;
			pThis->m_WarpColors[3] = 255 - min(255, pThis->m_uWarp*255);
			CEngineApp__RenderTint(pThis, pThis->m_WarpColors);

			if (pThis->m_uWarp >= 1)
				pThis->m_Warp = WARP_NOT_WARPING;
			break;
	}
}

void CEngineApp__DrawWater(CEngineApp *pThis)
{
	if (pThis->m_UnderWater)
		CEngineApp__RenderTint(pThis, pThis->m_FogColor);
}




// menu control left ?
//
BOOL CEngineApp__MenuLeft(CEngineApp *pThis)
{
	float	res;

	if (pThis->m_FadeStatus == FADE_DOWN)
		return FALSE ;

	// check for digital use of menus
	if (		(CTControl__IsClickLeft(pCTControl))
			&& (pThis->m_MenuLeftChangeCount == MENU_START_SPEED)
			&& (pThis->m_MenuRightChangeCount == MENU_START_SPEED)
			&& (pThis->m_MenuDownChangeCount == MENU_START_SPEED)
			&& (pThis->m_MenuUpChangeCount == MENU_START_SPEED) )
	{
		pThis->m_MenuLeftChangeCount = MENU_START_SPEED;
		//CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_1, 0, NULL, &pThis->m_Camera.m_vPos, 0);
		return TRUE;
	}
	else
	{
		res = CTControl__IsAnalogLeft(pCTControl);
		if (res<= MENU_HORIZONTAL_THRESHHOLD)
		{
			pThis->m_MenuLeftChangeCount = MENU_START_SPEED;
			return FALSE;
		}
		else
		{
			pThis->m_MenuLeftChangeCount += res * frame_increment;
			if (pThis->m_MenuLeftChangeCount >= MAX_MENU_SPEED)
			{
				pThis->m_MenuLeftChangeCount = 0;
				//CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_1, 0, NULL, &pThis->m_Camera.m_vPos, 0);
				return TRUE;
			}
			else
				return FALSE;
		}
	}
}


// menu control right ?
//
BOOL CEngineApp__MenuRight(CEngineApp *pThis)
{
	float	res;

	if (pThis->m_FadeStatus == FADE_DOWN)
		return FALSE ;


	// check for digital use of menus
	if (		(CTControl__IsClickRight(pCTControl))
			&& (pThis->m_MenuRightChangeCount == MENU_START_SPEED)
			&& (pThis->m_MenuLeftChangeCount == MENU_START_SPEED)
			&& (pThis->m_MenuDownChangeCount == MENU_START_SPEED)
			&& (pThis->m_MenuUpChangeCount == MENU_START_SPEED) )
	{
		pThis->m_MenuRightChangeCount = MENU_START_SPEED;
		//CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_1, 0, NULL, &pThis->m_Camera.m_vPos, 0);
		return TRUE;
	}
	else
	{
		res = CTControl__IsAnalogRight(pCTControl);
		if (res<= MENU_HORIZONTAL_THRESHHOLD)
		{
			pThis->m_MenuRightChangeCount = MENU_START_SPEED;
			return FALSE;
		}
		else
		{
			pThis->m_MenuRightChangeCount += res * frame_increment;
			if (pThis->m_MenuRightChangeCount >= MAX_MENU_SPEED)
			{
				pThis->m_MenuRightChangeCount = 0;
				//CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_1, 0, NULL, &pThis->m_Camera.m_vPos, 0);
				return TRUE;
			}
			else
				return FALSE;
		}
	}
}


// menu control down ?
//
BOOL CEngineApp__MenuDown(CEngineApp *pThis)
{
	float	res;

	if (pThis->m_FadeStatus == FADE_DOWN)
		return FALSE ;


	// check for digital use of menus
	if (		(CTControl__IsClickDown(pCTControl))
			&& (pThis->m_MenuDownChangeCount == MENU_START_SPEED)
			&& (pThis->m_MenuUpChangeCount == MENU_START_SPEED)
			&& (pThis->m_MenuRightChangeCount == MENU_START_SPEED)
			&& (pThis->m_MenuLeftChangeCount == MENU_START_SPEED) )
	{
		pThis->m_MenuDownChangeCount = MENU_START_SPEED;
		//CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_1, 0, NULL, &pThis->m_Camera.m_vPos, 0);
		return TRUE;
	}
	else
	{
		res = CTControl__IsAnalogDown(pCTControl);
		if (res<= MENU_VERTICAL_THRESHHOLD)
		{
			pThis->m_MenuDownChangeCount = MENU_START_SPEED;
			return FALSE;
		}
		else
		{
			pThis->m_MenuDownChangeCount += res * frame_increment;
			if (pThis->m_MenuDownChangeCount >= MAX_MENU_SPEED)
			{
				pThis->m_MenuDownChangeCount = 0;
				//CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_1, 0, NULL, &pThis->m_Camera.m_vPos, 0);
				return TRUE;
			}
			else
				return FALSE;
		}
	}
}


// menu control up ?
//
BOOL CEngineApp__MenuUp(CEngineApp *pThis)
{
	float	res;

	if (pThis->m_FadeStatus == FADE_DOWN)
		return FALSE ;


	// check for digital use of menus
	if (		(CTControl__IsClickUp(pCTControl))
			&& (pThis->m_MenuUpChangeCount == MENU_START_SPEED)
			&& (pThis->m_MenuDownChangeCount == MENU_START_SPEED)
			&& (pThis->m_MenuRightChangeCount == MENU_START_SPEED)
			&& (pThis->m_MenuLeftChangeCount == MENU_START_SPEED) )
	{
		pThis->m_MenuUpChangeCount = MENU_START_SPEED;
		//CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_1, 0, NULL, &pThis->m_Camera.m_vPos, 0);
		return TRUE;
	}
	else
	{
		res = CTControl__IsAnalogUp(pCTControl);
		if (res<= MENU_VERTICAL_THRESHHOLD)
		{
			pThis->m_MenuUpChangeCount = MENU_START_SPEED;
			return FALSE;
		}
		else
		{
			pThis->m_MenuUpChangeCount += res * frame_increment;
			if (pThis->m_MenuUpChangeCount >= MAX_MENU_SPEED)
			{
				pThis->m_MenuUpChangeCount = 0;
				//CScene__DoSoundEffect(&GetApp()->m_Scene, SOUND_GENERIC_1, 0, NULL, &pThis->m_Camera.m_vPos, 0);
				return TRUE;
			}
			else
				return FALSE;
		}
	}
}


void CEngineApp__ScreenShot(CEngineApp *pThis)
{
#ifdef SCREEN_SHOT
	unsigned short *pFrameBuffer;
	int offset, x, y;

	rmonPrintf("****** BEGIN FRAME DUMP ******\r\n");
	rmonPrintf("%d %d\r\n", SCREEN_WD, SCREEN_HT);

	pFrameBuffer = CEngineApp__GetFrameData(pThis)->m_pFrameBuffer;

	for (y=0; y<SCREEN_HT; y++)
	{
		for (x=0; x<(SCREEN_WD/8); x++)
		{
			offset = x*8 + y*SCREEN_WD;

			rmonPrintf("%x %x %x %x %x %x %x %x\r\n",
					pFrameBuffer[offset + 0],
					pFrameBuffer[offset + 1],
					pFrameBuffer[offset + 2],
					pFrameBuffer[offset + 3],

					pFrameBuffer[offset + 4],
					pFrameBuffer[offset + 5],
					pFrameBuffer[offset + 6],
					pFrameBuffer[offset + 7]);
		}
	}

	rmonPrintf("\r\n\r\n\r\n");
#endif
}

// set camera's position based on turok's position
void CEngineApp__SetCameraToTurok(CEngineApp *pThis)
{
	CGameObjectInstance	*pPlayer;
	t_Ears					ears;
	float						newY, newYSQ;
	CROMRegionSet			*pRegionSet;
	BOOL						keepInSphere;


	pPlayer = CEngineApp__GetPlayer(pThis);
	if (!pPlayer)
		return;

	pRegionSet = CScene__GetRegionAttributes(&pThis->m_Scene, pPlayer->ah.ih.m_pCurrentRegion) ;

	if (CCamera__InCinemaMode(&pThis->m_Camera))
	{
		keepInSphere = TRUE;
	}
	else
	{
		if (pRegionSet)
			keepInSphere = (pRegionSet->m_dwFlags & REGFLAG_FALLDEATH);
		else
			keepInSphere = FALSE;
	}

	pThis->m_YPos = pPlayer->ah.ih.m_vPos.y;

	// let it crash in some cases so player won't get stuck
	if (keepInSphere)
	{
		// deal with problems if player goes out of bounds
		if (CVector3__MagnitudeSquared(&pPlayer->ah.ih.m_vPos) > SQR(WORLD_LIMIT))
		{
			newYSQ = SQR(WORLD_LIMIT) - SQR(pPlayer->ah.ih.m_vPos.x) - SQR(pPlayer->ah.ih.m_vPos.z);
			newY = sqrt(max(0, newYSQ));

			pPlayer->ah.ih.m_vPos.y = (pPlayer->ah.ih.m_vPos.y < 0) ? -newY : newY;

			/* PORT: in cinema mode keepInSphere is forced TRUE even when pRegionSet is NULL (the
			 * player's region has no attributes), so this FALLDEATH deref crashes on a NULL pRegionSet.
			 * The original "let it crash" was an N64-deliberate stuck-player snap; guard it on host. */
			if (pRegionSet && (pRegionSet->m_dwFlags & REGFLAG_FALLDEATH))
			{
				pThis->m_YPos = pPlayer->ah.ih.m_vPos.y;

				pPlayer->ah.ih.m_vPos.y = CInstanceHdr__GetGroundHeight(&pPlayer->ah.ih) - 1.0;
			}
			else
			{
				pThis->m_YPos = pPlayer->ah.ih.m_vPos.y;
			}
		}
	}


	// set camera to this instance
	pThis->m_XPos 		= pPlayer->ah.ih.m_vPos.x;
	pThis->m_ZPos 		= pPlayer->ah.ih.m_vPos.z;
	pThis->m_RotY		= pPlayer->m_RotY;
	pThis->m_qGround	= pPlayer->m_qGround;

	// sound position
//	ears.vPos.x = pThis->m_XPos;
//	ears.vPos.y = pThis->m_YPos;
//	ears.vPos.z = pThis->m_ZPos;
//	ears.YRot = pPlayer->m_RotY;

	// Get ears from camera so pause menu and title menu work properly
	ears.vPos = pThis->m_Camera.m_vPos ;
	ears.YRot = pThis->m_Camera.m_vRotation.y ;

	ears.Radius = SCALING_MAX_DIST(pThis->m_FarClip, FIELD_OF_VIEW_ANGLE(pThis->m_FieldOfView, SCREEN_WD/SCREEN_HT));

	UpdateEars(&ears);
}

void CEngineApp__InitGame(CEngineApp *pThis)
{
	BYTE						viMode;

	// stacks must be 16 byte aligned
	ASSERT(!((DWORD)dram_stack & 0xf));
	ASSERT(!((DWORD)schedule_stack & 0xf));
	ASSERT(!((DWORD)idle_thread_stack & 0xf));
	ASSERT(!((DWORD)fault_thread_stack & 0xf));
	ASSERT(!((DWORD)main_thread_stack & 0xf));
	ASSERT(!((DWORD)boot_stack & 0xf));
	ASSERT(!((DWORD)audio_stack & 0xf));
#ifndef MAKE_CART
	ASSERT(!((DWORD)rmon_stack & 0xf));
#endif

  	// queue for OS messages indicating DMA completions
	osCreateMesgQueue(&dmaMessageQ, &dmaMessageBuf, 1);
	ASSERT(MQ_IS_EMPTY(&dmaMessageQ));

  	// queue for OS messages indicating DMA completions
	osCreateMesgQueue(&romMessageQ, &romMessageBuf, 1);
	ASSERT(MQ_IS_EMPTY(&romMessageQ));

	// queue for retraces from scheduler
	osCreateMesgQueue(&gfxFrameMsgQ, gfxFrameMsgBuf, MAX_MESGS);

	if (osTvType == 0)
		viMode = OS_VI_PAL_LAN1;
	else if (osTvType == 1)
		viMode = OS_VI_NTSC_LAN1;
	else
		viMode = OS_VI_MPAL_LAN1;

	// initialize the RCP task scheduler
	osCreateScheduler(&sc, (void *)(schedule_stack + OS_SC_STACKSIZE/sizeof(u64)),
							PRIORITY_SCHEDULER,
							viMode,
							NUM_FIELDS);

	// add ourselves to the scheduler to receive retrace messages
	osScAddClient(&sc, &gfxClient, &gfxFrameMsgQ);
	//sched_cmdQ = osScGetCmdQ(&sc);

	// call the initialization routines
	CEngineApp__Construct(pThis);
	CEngineApp__InitGraphics(pThis);

	// initialize controller & get pointer to turok controller data
	pCTControl = NULL;

	// audio
	initAudio();
}

void CEngineApp__ClearFrameBuffer(CEngineApp *pThis)
{
	memset(cfb_16_a, 0, 320*240*2*3);
}

void CEngineApp__DoPause(CEngineApp *pThis)
{
	if (pThis->m_FadeStatus == FADE_NULL)
	{
		pThis->m_MapMode = FALSE ;
		pThis->m_bPause = TRUE;
		CPause__Construct(&pThis->m_Pause);
	}
}

void CEngineApp__InitGraphics(CEngineApp *pThis)
{
	int			i;
	CFrameData	*pFrameData;

	for (i=0; i<3; i++)
	{
		pFrameData = &pThis->m_FrameData[i];

		pFrameData->m_Msg.gen.type		= OS_SC_DONE_MSG;

		CSunFrameData__Construct(&pFrameData->m_SunFrame);
	}

	pThis->m_FrameData[0].m_pFrameBuffer = cfb_16_a;
	pThis->m_FrameData[1].m_pFrameBuffer = cfb_16_b;
	pThis->m_FrameData[2].m_pFrameBuffer = cfb_16_c;

	CEngineApp__AdvanceFrameData(pThis);
}

void CEngineApp__AdvanceFrameData(CEngineApp *pThis)
{
	frame_number++;
	game_frame_number++;
	ToggleEvenOdd();

	pThis->m_pCurrentFrameData = &pThis->m_FrameData[frame_number % 3];
	pThis->m_pCurrentFrameData->m_pDisplayList = (even_odd) ? display_list_a : display_list_b;
	pThis->m_pCurrentFrameData->m_pLineList = (even_odd) ? line_list_a : line_list_b;
}

void CEngineApp__InitDisplayLists(CEngineApp *pThis)
{
	// pointers to build the display list
	pThis->m_pDLP = CEngineApp__GetFrameData(pThis)->m_pDisplayList;

	CEngineApp__SetupSegments(pThis);

	// Initialize RSP state
	gSPDisplayList(pThis->m_pDLP++, RSP_ADDRESS(rspinit_dl));
	if (microcode_version == MICROCODE_LOW_PRECISION)
		gSPSetGeometryMode(pThis->m_pDLP++, G_CLIPPING);

	// Initialize RDP state
	gSPDisplayList(pThis->m_pDLP++, RDP_ADDRESS(rdpinit_dl));
}




void CEngineApp__UpdateSpiritMode(CEngineApp *pThis)
{
	CVector3	vDir ;
	int	i ;
	float speed;
	float ambri;

#ifndef KANJI
	if (GetApp()->m_dwCheatFlags & CHEATFLAG_DISCO)
	{
		speed = 2.5;
		ambri = 2.0;
	}
	else
#endif
	{
		speed = 1.0;
		ambri = 1.0;
	}

	// Setup ambient and directional light
	for (i=0; i<3; i++)
	{
		thelight.a.l.colc[i] = thelight.a.l.col[i] = (ambri * 16 * sin(ANGLE_DTOR(speed * game_frame_number * 4 * (2+i)))) + 16 + 48 ;	// ambient color
		thelight.l[0].l.colc[i] = thelight.l[0].l.col[i] = (64 * sin(ANGLE_DTOR(speed * game_frame_number * 4 * (2+i)))) + 255-64-1 ;	// directional color
	}

	// Set direction
	vDir.x = sin(ANGLE_DTOR(game_frame_number * 3*2)) ;
	vDir.y = sin(ANGLE_DTOR(game_frame_number * 4*2)) ;
	vDir.z = cos(ANGLE_DTOR(game_frame_number * 5*2)) ;
	CVector3__Normalize(&vDir) ;
	vDir.x *= 120 ;
	vDir.y *= 120 ;
	vDir.z *= 120 ;
	thelight.l[0].l.dir[0] = vDir.x ;
	thelight.l[0].l.dir[1] = vDir.y ;
	thelight.l[0].l.dir[2] = vDir.z ;

	// Set fog color
#ifndef KANJI
	if (!(GetApp()->m_dwCheatFlags & CHEATFLAG_DISCO))
#endif
	{
		pThis->m_LastFogColor[0] = pThis->m_FogColor[0] = (BYTE)((64 * sin(ANGLE_DTOR(speed * game_frame_number * 2 * 4))) + 128) & 0xf8 ;
		pThis->m_LastFogColor[1] = pThis->m_FogColor[1] = (BYTE)((64 * sin(ANGLE_DTOR(speed * game_frame_number * 3 * 4))) + 128) & 0xf8 ;
		pThis->m_LastFogColor[2] = pThis->m_FogColor[2] = (BYTE)((64 * sin(ANGLE_DTOR(speed * game_frame_number * 4 * 4))) + 128) & 0xf8 ;
	}
}




#define REGIONSET_BLEND_LENGTH	(1*FRAME_FPS)
void CEngineApp__UpdateCameraAttributes(CEngineApp *pThis)
{
	CGameObjectInstance	*pPlayer;
	CGameRegion				*pRegion;
	CROMRegionSet			*pRegionSet;
	int						i,
								waterOffset;
	float						depth,
								farClip,
								fieldOfView,
								skySpeed,
								skyElevation,
								u;
	BOOL						newUnderWater, waterChange;
	BYTE						fogColor[4];
	DWORD						fogStart;

	pPlayer = CEngineApp__GetPlayer(GetApp());
	if (!pPlayer)
		return;

	pRegion = pPlayer->ah.ih.m_pCurrentRegion;
	if (!pRegion)
		return;

	pRegionSet = CScene__GetRegionAttributes(&pThis->m_Scene, pRegion);
	if (!pRegionSet)
		return;

	newUnderWater = (CEngineApp__GetPlayerWaterStatus(pThis) == PLAYER_BELOW_WATERSURFACE);
	waterChange = (newUnderWater != pThis->m_UnderWater);
	pThis->m_UnderWater = newUnderWater;

	if ((pRegion->m_nRegionSet != pThis->m_nCurrentRegionSet) || waterChange)
	{
		if ((pThis->m_nCurrentRegionSet == (DWORD) -1) || waterChange)
			pThis->m_cRegionSetBlend = 1;
		else
			pThis->m_cRegionSetBlend = 0;

		pThis->m_nCurrentRegionSet = pRegion->m_nRegionSet;

		for (i=0; i<4; i++)
			pThis->m_LastFogColor[i] = pThis->m_FogColor[i];

		pThis->m_LastFarClip			= pThis->m_FarClip;
		pThis->m_LastFogStart		= pThis->m_FogStart;
		pThis->m_LastFieldOfView	= pThis->m_FieldOfView;
		pThis->m_LastSkySpeed		= pThis->m_SkySpeed;
		pThis->m_LastSkyElevation	= pThis->m_SkyElevation;
	}

	if (pThis->m_UnderWater)
	{
		if (CTurokMovement.InAntiGrav)
		{
			for (i=0; i<3; i++)
				fogColor[i] = pRegionSet->m_WaterColor[i];
			fogColor[3] = 0;
		}
		else
		{
			depth = CEngineApp__GetPlayerWaterDepth(pThis);
			waterOffset = ((int)depth)/16;
			for (i=0; i<3; i++)
				fogColor[i] = max(0, pRegionSet->m_WaterColor[i] - waterOffset);
			fogColor[3] = min(192, pRegionSet->m_WaterColor[3] + waterOffset);
		}
		fogStart = pRegionSet->m_WaterStart;
		farClip = pRegionSet->m_WaterFarClip;
		fieldOfView = pRegionSet->m_WaterFieldOfView;
		skySpeed = pRegionSet->m_SkySpeed;
		skyElevation = pRegionSet->m_SkyElevation;
	}
	else
	{
		for (i=0; i<3; i++)
			fogColor[i] = pRegionSet->m_FogColor[i];
		fogColor[3] = 0;

		fogStart = pRegionSet->m_FogStart;
		farClip = pRegionSet->m_FarClip;
		fieldOfView = pRegionSet->m_FieldOfView;
		skySpeed = pRegionSet->m_SkySpeed;
		skyElevation = pRegionSet->m_SkyElevation;
	}

//	if (CTurokMovement.SpiritualTime > SPIRITUAL_TIMEOUT)
//	{
//		if (fieldOfView != 2.0)
//		{
//			pThis->m_cRegionSetBlend = 0;
//			pThis->m_LastFieldOfView = pThis->m_FieldOfView;
//			fieldOfView = 2.0 ;
//		}
//	}

	if (pRegionSet->m_BlendLength == 0)
		pThis->m_cRegionSetBlend = 1;
	else
		pThis->m_cRegionSetBlend = min(1, pThis->m_cRegionSetBlend + frame_increment/(pRegionSet->m_BlendLength*FRAME_FPS));

	u = GetHermiteBlender(0, 0, pThis->m_cRegionSetBlend);

	for (i=0; i<4; i++)
		pThis->m_FogColor[i] = (BYTE) max(0, min(255, BlendFLOAT(u, pThis->m_LastFogColor[i], fogColor[i])));

	pThis->m_FogStart	= (DWORD) max(0, min(995, BlendFLOAT(u, pThis->m_LastFogStart, fogStart)));

	pThis->m_FarClip			= BlendFLOAT(u, pThis->m_LastFarClip, farClip);
	pThis->m_FieldOfView		= BlendFLOAT(u, pThis->m_LastFieldOfView, fieldOfView);
	pThis->m_SkySpeed			= BlendFLOAT(u, pThis->m_LastSkySpeed, skySpeed);
	pThis->m_SkyElevation	= BlendFLOAT(u, pThis->m_LastSkyElevation, skyElevation);

	// Special case levels
	if (pThis->m_CurrentWarpID == GAME_INTRO_WARP_ID)
		pThis->m_FarClip = SCALING_FAR_CLIP * 1.6 ;

	// Do spirit effects
	if (	 (GetApp()->m_Mode == MODE_GAME)
		 && (   (CTurokMovement.SpiritualTime > 0)
#ifndef KANJI
			  || (GetApp()->m_dwCheatFlags & CHEATFLAG_DISCO) ) )
#else
			  ))
#endif
		CEngineApp__UpdateSpiritMode(pThis) ;
}

CFrameData* CEngineApp__CreateGraphicsTask(CEngineApp *pThis)
{
	CFrameData	*pFData;
	OSTime		startTime;

	pFData = CEngineApp__GetFrameData(pThis);

	if ((pThis->m_Mode == MODE_GAME) || (pThis->m_Mode == MODE_GAMEOVER))
	{
		// Update on screen overlays
		COnScreen__Update(&pThis->m_OnScreen);
	}

	// build display list for next frame
	startTime = osGetTime();

	switch (pThis->m_Mode)
	{
		case MODE_INTRO:
		case MODE_GAME:
		case MODE_GAMEOVER:
		case MODE_LEGALSCREEN:
		case MODE_GAMEPAK:
		case MODE_TITLE:
		case MODE_CREDITS:
		case MODE_GALLERY:
			CEngineApp__BuildDispList(pThis);
			break;

		default:
			ASSERT(FALSE);
			break;
	}

	cpu_time = ((float)CYCLES_TO_NSEC(osGetTime() - startTime))/1000000;

	// end the display list
	gDPFullSync(pThis->m_pDLP++);
	gSPEndDisplayList(pThis->m_pDLP++);

	pFData->m_DisplayListSize = (((DWORD)pThis->m_pDLP) - ((DWORD)pFData->m_pDisplayList));

#ifdef PLATFORM_PORT
	{ extern int fprintf(void*,const char*,...); extern void *stderr;
	  static long _mx=0;
	  long _n = pThis->m_pDLP - pFData->m_pDisplayList;
	  if (_n > _mx) { _mx = _n; if(_n>=GLIST_LEN) fprintf(stderr,"[dlcount] new max %ld / GLIST_LEN=%d  *** OVERFLOW — corrupting adjacent memory ***\n",
	    _n, GLIST_LEN); } }
#endif

#ifndef MAKE_CART
	if ((pThis->m_pDLP - pFData->m_pDisplayList) > GLIST_LEN)
		rmonPrintf("over by:%d\n", (pThis->m_pDLP - pFData->m_pDisplayList) - GLIST_LEN) ;
#endif

	ASSERT((pThis->m_pDLP - pFData->m_pDisplayList) < GLIST_LEN);

// CARL, make 1 to draw map
#if 1
	pFData->m_DrawMap = (pThis->m_MapMode && (pThis->m_Mode == MODE_GAME) && cache_is_valid);
#else
	pFData->m_DrawMap = FALSE;
#endif

	// line draw display list
	if (pFData->m_DrawMap)
	{
		pThis->m_pDLP = CEngineApp__GetFrameData(pThis)->m_pLineList;

		CEngineApp__DrawMap(pThis);

		gDPFullSync(pThis->m_pDLP++);
		gSPEndDisplayList(pThis->m_pDLP++);

		pFData->m_LineListSize = (((DWORD)pThis->m_pDLP) - ((DWORD)pFData->m_pLineList));
		ASSERT((pThis->m_pDLP - pFData->m_pLineList) < LINELIST_LEN);
	}
	// map is not being drawn, but still needs to know what is revealed by player, so update silently...
	else
		CEngineApp__RevealMap(pThis) ;

	return pFData;
}


void CEngineApp__SendGraphicsTask(CEngineApp *pThis, CFrameData *pFData)
{
	OSScTask		*t;

	//rmonPrintf("pFData->m_DisplayListSize:%x\n", pFData->m_DisplayListSize);

	// build graphics task
	t = &pFData->m_Task;

	t->list.t.data_ptr			= (u64*) pFData->m_pDisplayList;		// task data pointer (fill in later)
	t->list.t.data_size			= pFData->m_DisplayListSize;			// task data size
	t->list.t.type					= M_GFXTASK;								// task type
	t->list.t.flags				= 0;											// task flags
	t->list.t.ucode_boot			= (u64*) rspbootTextStart;				// boot ucode pointer
	t->list.t.ucode_boot_size	= ((s32) rspbootTextEnd - (s32) rspbootTextStart); // boot ucode size
   t->list.t.ucode_size      	= SP_UCODE_SIZE;							// task ucode size
   t->list.t.ucode_data_size 	= SP_UCODE_DATA_SIZE;					// task ucode data size
   t->list.t.dram_stack      	= dram_stack;								// task dram stack pointer
   t->list.t.dram_stack_size	= SP_DRAM_STACK_SIZE8;					// task dram stack size
	t->list.t.yield_data_ptr	= (u64*) gfx_yield_buf;					// task yield buffer ptr
	t->list.t.yield_data_size	= OS_YIELD_DATA_SIZE;					// task yield buffer size
#ifdef USE_FIFO
	switch (microcode_version)
	{
		case MICROCODE_HIGH_PRECISION:
			t->list.t.ucode				= (u64*) gspF3DEX_NoN_fifoTextStart;	// task ucode pointer
			t->list.t.ucode_data			= (u64*) gspF3DEX_NoN_fifoDataStart;	// task ucode data pointer
			break;

		case MICROCODE_LOW_PRECISION:
			t->list.t.ucode				= (u64*) gspF3DLX_NoN_fifoTextStart;	// task ucode pointer
			t->list.t.ucode_data			= (u64*) gspF3DLX_NoN_fifoDataStart;	// task ucode data pointer
			break;

		default:
			ASSERT(FALSE);
			break;
	}

	t->list.t.output_buff     	= fifo_buffer;								// task output buffer ptr (not always used)
	t->list.t.output_buff_size	= (u64*) ((int)fifo_buffer + (int)FIFO_SIZE);	// task output buffer size ptr
#else
	t->list.t.ucode				= (u64*) gspF3DEX_NoNTextStart;			// task ucode pointer
	t->list.t.ucode_data			= (u64*) gspF3DEX_NoNDataStart;			// task ucode data pointer
	t->list.t.output_buff		= NULL;										// task output buffer ptr (not always used)
	t->list.t.output_buff_size = 0;											// task output buffer size ptr
#endif

	t->next			= 0;									// next task (none)

	t->msgQ			= &gfxFrameMsgQ;					// reply to when finished
	t->msg			= (OSMesg) &pFData->m_Msg;     // reply with this message

	t->framebuffer = (void*) pFData->m_pFrameBuffer;

	if (pFData->m_DrawMap)
		t->flags			= OS_SC_NEEDS_RSP | OS_SC_NEEDS_RDP;
	else
		t->flags			= OS_SC_NEEDS_RSP | OS_SC_NEEDS_RDP | OS_SC_LAST_TASK;

	if (pFData->m_DrawMap)
	{
		// set up line draw task
		t = &pFData->m_LineTask;

		t->list.t.data_ptr			= (u64*) pFData->m_pLineList;		// task data pointer (fill in later)
		t->list.t.data_size			= pFData->m_LineListSize;			// task data size
		t->list.t.type					= M_GFXTASK;								// task type
		t->list.t.flags				= 0;											// task flags
		t->list.t.ucode_boot			= (u64*) rspbootTextStart;				// boot ucode pointer
		t->list.t.ucode_boot_size	= ((s32) rspbootTextEnd - (s32) rspbootTextStart); // boot ucode size
		t->list.t.ucode_size      	= SP_UCODE_SIZE;							// task ucode size
		t->list.t.ucode_data_size 	= SP_UCODE_DATA_SIZE;					// task ucode data size
		t->list.t.dram_stack      	= dram_stack;								// task dram stack pointer
		t->list.t.dram_stack_size	= SP_DRAM_STACK_SIZE8;					// task dram stack size
		t->list.t.yield_data_ptr	= (u64*) gfx_yield_buf;					// task yield buffer ptr
		t->list.t.yield_data_size	= OS_YIELD_DATA_SIZE;					// task yield buffer size
#ifdef USE_FIFO
		t->list.t.ucode				= (u64*) gspL3DEX_fifoTextStart;	// task ucode pointer
		t->list.t.ucode_data			= (u64*) gspL3DEX_fifoDataStart;	// task ucode data pointer
		t->list.t.output_buff     	= fifo_buffer;								// task output buffer ptr (not always used)
		t->list.t.output_buff_size	= (u64*) ((int)fifo_buffer + (int)FIFO_SIZE);	// task output buffer size ptr
#else
		t->list.t.ucode				= (u64*) gspLine3DTextStart;			// task ucode pointer
		t->list.t.ucode_data			= (u64*) gspLine3DDataStart;			// task ucode data pointer
		t->list.t.output_buff		= NULL;										// task output buffer ptr (not always used)
		t->list.t.output_buff_size = 0;											// task output buffer size ptr
#endif

		t->next			= 0;									// next task (none)
		t->flags			= OS_SC_NEEDS_RSP | OS_SC_NEEDS_RDP | OS_SC_LAST_TASK;

		t->msgQ			= &gfxFrameMsgQ;					// reply to when finished
		t->msg			= (OSMesg) &pFData->m_Msg;     // reply with this message

		t->framebuffer = (void*) pFData->m_pFrameBuffer;
	}

	// dump whole cache because of dynamic data
	osWritebackDCacheAll();

	scSendCommand(&sc, &pFData->m_Task);

	if (pFData->m_DrawMap)
		scSendCommand(&sc, &pFData->m_LineTask);

	CEngineApp__AdvanceFrameData(pThis);
}

void CEngineApp__UpdatePause(CEngineApp *pThis)
{
	INT32						ret;
	//CGameObjectInstance	*pPlayer;

	if (pThis->m_bTraining)
	{
		CTraining__Update(&pThis->m_Training) ;

		if ((pThis->m_bTrainingMode == 2) && (CTControl__IsUseMenu(pCTControl)))
		{
			pThis->m_bTrainingMode = FALSE ;
			pThis->m_bTraining	= FALSE;

//			pThis->m_WarpID = 0 ;
//			CEngineApp__SetupFadeTo(pThis, MODE_RESETLEVEL);
			CEngineApp__SetupFadeTo(pThis, MODE_RESETGAME);
		}
	}
	else if	(pThis->m_bPause)
	{
		// read pause info
		ret = CPause__Update(&pThis->m_Pause);

		switch (ret)
		{
			case PAUSE_RESUME:
				pThis->m_bPause = FALSE ;
				break;
#if 0
			case PAUSE_PREVLEVEL:
				pThis->m_pBossActive = NULL ;
				pThis->m_bPause = FALSE;
				switch (pThis->m_WarpBase)
				{
					//case 9000:
					//case 9100:
					//case 9200:
					//case 9300:
					//case 9400:
					//case 9500:
					//case 9600:
					//case 9700:
					//case 9800:
					//	pThis->m_WarpBase -= 100 ;
					//	if (pThis->m_WarpBase <9000)
					//		pThis->m_WarpBase = 8000 ;	// goto lastlevel
					//	break ;

					//case 2000:
					//	pThis->m_WarpBase = 0 ;
					//	break ;


					default:
						pThis->m_WarpBase -= 1000 ;
						if (pThis->m_WarpBase < 0)
							pThis->m_WarpBase = 8000 ;
						break ;
				}
				pThis->m_WarpID = pThis->m_WarpBase ;
				CEngineApp__Warp(pThis, pThis->m_WarpID, WARP_BETWEENLEVELS, FALSE);
				break ;

			case PAUSE_NEXTLEVEL:
				pThis->m_pBossActive = NULL;
				pThis->m_bPause = FALSE;
				switch (pThis->m_WarpBase)
				{
					//case 9000:
					//case 9100:
					//case 9200:
					//case 9300:
					//case 9400:
					//case 9500:
					//case 9600:
					//case 9700:
					//case 9800:
					//	pThis->m_WarpBase += 100 ;
					//	if (pThis->m_WarpBase >9800)
					//		pThis->m_WarpBase = 20000 ;	// goto bosses
					//	break ;

					//case 0:
					//	pThis->m_WarpBase = 2000 ;
					//	break ;

					default:
						pThis->m_WarpBase += 1000 ;
						if (pThis->m_WarpBase > 8000)
							pThis->m_WarpBase = 0 ;
						break ;
				}
				pThis->m_WarpID = pThis->m_WarpBase ;
				CEngineApp__Warp(pThis, pThis->m_WarpID, WARP_BETWEENLEVELS, FALSE);
				break ;
#endif

			case PAUSE_RESTART_GAME:
				pThis->m_bPause = FALSE ;
				CEngineApp__SetupFadeTo(pThis, MODE_RESETGAME);
				break;
		}

		// if come out of pause
		if (pThis->m_bPause == FALSE)
		{
			enemy_speed_scaler = old_enemy_speed_scaler;
			sky_speed_scaler = old_sky_speed_scaler;
			particle_speed_scaler = old_particle_speed_scaler;

			// switch volume back to what it was
//			pThis->m_ActualMusicVolume = pThis->m_Pause.m_MusicVolume;
//			pThis->m_ActualSFXVolume = pThis->m_Pause.m_SFXVolume;
		}
	}
	else
	{
		// Check for pausing game (also pauses if turok is dead!)

		if (		(CTControl__IsStart(pCTControl))
				&& (!CTControl__IsPrimaryFire(pCTControl)) )
		{
			//CCartCache__MemoryDump(&pThis->m_Scene.m_Cache);

			// can only pause if not already paused, and no cinema is active!
			if ((pThis->m_bPause == FALSE) && (!CCamera__InCinemaMode(&pThis->m_Camera)))
			{
				// quit training mode?
				if (		((pThis->m_bTrainingMode == TRUE) || (pThis->m_bTrainingMode == -1))
						&& (!pThis->m_bTraining))
				{
					GetApp()->m_bPause = TRUE;
					CPause__Construct(&GetApp()->m_Pause);
					GetApp()->m_Pause.m_Mode = PAUSE_NORMAL;
					CRequestor__Construct(&GetApp()->m_Requestor,
												text_quittraining, text_areyousure) ;
					GetApp()->m_bRequestor = TRUE ;
					GetApp()->m_Requestor.m_Selection = REQUESTOR_NO ;
					RequestorMode = -1 ;

					//CEngineApp__SetupFadeTo(pThis, MODE_RESETGAME);
				}
				else
				{
					// kill all sfx's going into pause menu
					killAllSFX();

					CEngineApp__DoPause(pThis);
					pThis->m_Flash = 0;
				}
			}
		}
	}
}


void CEngineApp__ResetMenus(CEngineApp *pThis)
{
	pThis->m_bPause = FALSE ;
	pThis->m_bOptions = FALSE ;
	pThis->m_bKeys = FALSE ;
	pThis->m_bLoad = FALSE ;
	pThis->m_bSave = FALSE ;
	pThis->m_bRequestor = FALSE ;
	pThis->m_bMessageBox = FALSE ;
	pThis->m_bCheatCode = FALSE ;
	pThis->m_bGiveCheatCode = FALSE ;
	pThis->m_bCheat = FALSE ;
	pThis->m_bGInst = FALSE ;

	pThis->m_Pause.m_Mode = PAUSE_NULL ;
	pThis->m_Options.m_Mode = OPTIONS_NULL ;
	pThis->m_Keys.m_Mode = KEYS_NULL ;
	pThis->m_Load.m_Mode = LOAD_NULL ;
	pThis->m_Save.m_Mode = SAVE_NULL ;
	pThis->m_Requestor.m_Mode = REQUESTOR_NULL ;
	pThis->m_Requestor.b_Active = FALSE ;
	pThis->m_CheatCode.m_Mode = CHEATCODE_NULL ;
	pThis->m_GiveCheatCode.m_Mode = CHEATCODE_NULL ;
	pThis->m_GiveCheatCode.b_Active = FALSE ;
	pThis->m_Cheat.m_Mode = CHEAT_NULL ;
	pThis->m_Cheat.b_Active = FALSE ;
	pThis->m_GInst.m_Mode = GINST_NULL ;
	pThis->m_GInst.b_Active = FALSE ;
}


/**********************************************************************
 *
 * A continual loop, primarily used for servicing the starts of graphic
 * tasks and controller reads. Audio is serviced by the audio thread.
 * You receive several message on the same queue, they are:
 *    OS_SC_RETRACE_MSG: this comes from the scheduler every retrace.
 *         If you don't already have 2 graphics tasks either pending or
 *         processing, start creating another.
 *    OS_SC_DONE_MSG: sent by the scheduler when a graphics task has
 *         completed.
 *    CONTROLLER_MSG: sent by the controller code indicating that the
 *         controller read has completed, so now you can use that info
 *         in game play.
 *    OS_SC_PRE_NMI_MSG: sent to indicate the reset button has been
 *         pressed, and that the game will reboot momentarily. Might
 *         want to fade or say goodbye.
 *
 **********************************************************************/

//#define DB_LOOP

#define	MAX_DISPLAY_LISTS	(2)

#define CALC_FRAMERATE(nMinimumFields) max(nMinimumFields,((n_fields + n_l_fields + n_ll_fields + n_lll_fields + 2) - max(n_fields, max(n_l_fields, max(n_ll_fields, n_lll_fields))))/3)

BOOL			cntrlReadInProg, cntrlReadYet,
				cntrlStart;
GFXMsg		*pMsg;
int			wait, joyReadCount, nWaitBlack;
CFrameData	*pFrameData;
int			nDisplayLists,
				nNextFields;
float			musicvolume;

// TEMP
extern OSContPad     PlayerControllerData ;

void CEngineApp__Main(CEngineApp *pThis)
{
	short		health ;
	int		warp ;
	float		dm;
	int		ret ;
//	CGameObjectInstance	*pTurok ;
//	CROMWarpPoint			*pWarpPoint ;
	BOOL firstFrame = TRUE;

	if (osTvType == 0)
		refresh_rate = 50.0;	// PAL
	else
		refresh_rate = 60.0;	// NTSC, MPAL

	pMsg = NULL;
	cntrlReadInProg = FALSE;
	cntrlReadYet = FALSE;

	// Initialise attract mode stuff - just once when machine is turned on!
	CAttractDemo__Construct() ;

	nDisplayLists		= MAX_DISPLAY_LISTS;

	c_field				= 0;
	n_fields				= 0;
	n_l_fields			= 0;
	n_ll_fields			= 0;
	n_lll_fields		= 0;
	c_waiting			= 0;
	n_waiting			= 0;
	c_finished			= 0;
	n_pending			= 0;
	p_current_fb = NULL;
	p_pending_fb = NULL;

	pThis->m_LegalBypass = FALSE;

	pThis->m_Mode = MODE_RESETGAME;

	pThis->m_dwCheatFlags = 0;
	pThis->m_MenuUpChangeCount = MENU_START_SPEED;
	pThis->m_MenuDownChangeCount = MENU_START_SPEED;
	pThis->m_MenuLeftChangeCount = MENU_START_SPEED;
	pThis->m_MenuRightChangeCount = MENU_START_SPEED;


	pThis->m_dwEnabledCheatFlags = 0;


	pThis->m_Difficulty = MEDIUM ;


//#ifdef AUTO_CHEAT
//	pThis->m_dwEnabledCheatFlags = -1;
//#endif

	CEngineApp__InitGame(pThis);
	memset(cfb_16_a, 0, 320*240*2);
	osViSwapBuffer(cfb_16_a);

	osViSetSpecialFeatures(OS_VI_GAMMA_OFF);
	osViSetSpecialFeatures(OS_VI_DITHER_FILTER_ON);



	// set the initial pause status to avoid flickering on fade up
	pThis->m_Pause.m_Mode = PAUSE_NULL ;

	pThis->m_BestTrainTime = (6*60)*FRAME_FPS ;

	pThis->m_pStaticSegment = (void*) (u32)_staticSegmentRomStart;

	joyReadCount = 0;

	pThis->m_GamePakCheck = TRUE ;

	if (!validcontrollers)
	{
		Controller = InitControllersAdvanced();
		pCTControl = CTControl__CTControl(pThis->m_Options.m_RHControl);
	}

	// Reset all camera sound handles - only do this once, and never again please.
	CCamera__InitializeSoundHandles(&pThis->m_Camera) ;

	while (TRUE)
	{
		// force trap for test
		//if (frame_number > 400)
		//	*((char*)(NULL)) = 0;

		osRecvMesg(&gfxFrameMsgQ, (OSMesg*) &pMsg, OS_MESG_BLOCK);

		switch (pMsg->gen.type)
		{
			case (OS_SC_RETRACE_MSG):
				if (firstFrame)
				{
					firstFrame = FALSE;
					osViBlack(FALSE);
				}

				// if no controller plugged in upon startup, keep polling for one!
				//if (!validcontrollers)
				//{
				//	Controller = InitControllersAdvanced();
				//	if (validcontrollers)
				//		pCTControl = CTControl__CTControl();
				//}

				if (joyReadCount++ == 10)
				{
					//Controller = InitControllersAdvanced();
					//pCTControl = CTControl__CTControl();

					cntrlReadInProg = FALSE;
					joyReadCount = 0;
				}

				// Increase mode time
				pThis->m_ModeTime += frame_increment ;

				switch (pThis->m_Mode)
				{
					// --------------------------- STARTUP ---------------------------
					case MODE_RESETGAME:
						ASSERT(nDisplayLists == MAX_DISPLAY_LISTS);

						// Set default options for attract mode play back/record
						if (CAttractDemo__Active())
						{
							COptions__Store(&pThis->m_Options) ;
							COptions__SetAttractDefaults(&pThis->m_Options) ;
						}

	
						// Call before CScene__Construct!!
						pThis->m_WarpID = 0 ;
						pThis->m_CurrentWarpID = pThis->m_WarpID ;
						CCamera__PreSceneConstruct(&pThis->m_Camera) ;

						game_frame_number = 0 ;

						// must be cleared when cache is cleared
						CTurokMovement.AmbientSound = REG_AMBIENTNONE;
						cache_is_valid = FALSE;

						// Clear audio list
						CAudioList__Construct() ;

						i3D_initMemoryAllocation(dynamic_memory_pool, MEMORY_POOL_SIZE);

						// initialize stuff that needs to be reset before each game
						CEngineApp__InitializeGameVars(pThis) ;

						// Reset boss vars
						CEngineApp__ResetBossVars(pThis);
						pThis->m_BossFlags = 0 ;

						// Other stuff
						pThis->m_LastSaveMapLevel = -1; 					// must be before scene

						CScene__Construct(&pThis->m_Scene, 0);

						// Call after CScene__Construct!!
						CCamera__Construct(&pThis->m_Camera) ;

						// Construct fx system - swooshes etc
						CFxSystem__Construct(&pThis->m_FxSystem) ;

						pCTMove = CTMove__CTMove();

						pCTMove->InMenuTimer = 0 ;				// init the menu timer

						sky_speed_scaler = 1.0 ;
						enemy_speed_scaler = CEngineApp__GetEnemySpeedScaler(pThis);
						particle_speed_scaler = 1.0 ;

						CEngineApp__ResetMenus(pThis) ;

						pThis->m_GamePak.m_Mode = GAMEPAK_NULL ;
						pThis->m_bGamePak = FALSE ;

						pThis->m_MenuUpChangeCount = MENU_START_SPEED;
						pThis->m_MenuDownChangeCount = MENU_START_SPEED;
						pThis->m_MenuLeftChangeCount = MENU_START_SPEED;
						pThis->m_MenuRightChangeCount = MENU_START_SPEED;

						pThis->m_FadeStatus = FADE_UP ;
						pThis->m_FadeAlpha = 0 ;

						pThis->m_FogColor[0] = 0 ;
						pThis->m_FogColor[1] = 0 ;
						pThis->m_FogColor[2] = 0 ;
						pThis->m_FogColor[3] = 0 ;
						pThis->m_cRegionSetBlend = 1;


						pThis->m_bGameOver = FALSE ;
						pThis->m_GameOverAlpha = 0 ;
						pThis->m_GameOverMode = 0 ;
						pThis->m_GameOverTime = 0 ;

						pThis->m_TitleAlpha = 0 ;

						pThis->m_JustLoadedGame = FALSE ;

						pThis->m_bTrainingMode = FALSE ;
						pThis->m_bTraining	= FALSE;

						pThis->m_LevelTime = 0;

						pThis->m_bDebug = FALSE ;

						pThis->m_MapMode = FALSE ;
						pThis->m_MapAlpha = 0 ;
						pThis->m_MapStatus = MAP_NULL ;

						pThis->m_WarpBase = 0 ;			// start at first level

						pThis->m_WarpID = LEGALSCREEN_WARP_ID ;

						// CRASH
						//if ((CrashId == 0x4341524c) && (CrashMode == MODE_GAME))
						if (		(CrashId == 0x4341524c)
								||	(CrashId == 0x4341524d) )
						{
							//pThis->m_WarpID = 0 ;
							//CEngineApp__NewLife(pThis) ;
							pThis->m_Mode = MODE_RESETLEVEL ;

							CTMove__CTMove();
							CLoad__ExtractData() ;

							if (CrashId == 0x4341524c)
								pThis->m_WarpID = CRASH_WARP_ID ;
							else
								pThis->m_WarpID = CTurokMovement.CurrentCheckpoint ;

							CScene__DoSoundEffect(&pThis->m_Scene,
														 SOUND_WARP_INOUT, 1,
														 CEngineApp__GetPlayer(pThis), &AW.Ears.vPos, 0);

							pThis->m_JustLoadedGame = TRUE ;
						}
						else
						{
							pThis->m_LegalTimer = 0 ;
							CLegalScreen__Construct() ;
							pThis->m_Mode = MODE_RESETLEVEL ;

							// IAN - I added this to fix a bug hopefully i didn't screw anything up
							CTMove__NewLifeSetup(&CTurokMovement);
							pThis->m_WarpID = LEGALSCREEN_WARP_ID ;
						}
#ifdef PLATFORM_PORT
						/* PORT: optional warp-to-level for bring-up. TUROK_WARP=<id> (0,1000,...,8000 =
						 * the 9 level entry points — see the debug warp switch ~tengine.c:3351) skips the
						 * intro/legal screen and loads a real level → exercises 3D geometry + the M5
						 * level-data endianness layer. Unset = normal legal-screen boot. */
						{
							extern char *getenv(const char *);
							const char *w = getenv("TUROK_WARP");
							if (w && *w)
							{
								int id = 0; const char *p = w;
								while (*p >= '0' && *p <= '9') id = id * 10 + (*p++ - '0');
								pThis->m_WarpBase = id;
								pThis->m_WarpID   = id;
								pThis->m_Mode     = MODE_RESETLEVEL;
							}
						}
#endif
						CrashId = 0 ;
						break;

					// --------------------------- GAMEPAK MANAGER ---------------------------
					case MODE_GAMEPAK:
						pThis->m_FogColor[0] = 0 ;
						pThis->m_FogColor[1] = 0 ;
						pThis->m_FogColor[2] = 0 ;
						pThis->m_FogColor[3] = 0 ;
						pThis->m_cRegionSetBlend = 1;

						if (nDisplayLists)
						{
							nDisplayLists--;
							CEngineApp__UpdateGAMEPAK(pThis) ;
							CEngineApp__UpdateFade(pThis) ;
						}
						break;

					// --------------------------- LEGAL SCREEN ---------------------------
					case MODE_LEGALSCREEN:
						//pThis->m_FogColor[0] = 0 ;
						//pThis->m_FogColor[1] = 0 ;
						//pThis->m_FogColor[2] = 0 ;
						//pThis->m_FogColor[3] = 0 ;
						//pThis->m_cRegionSetBlend = 1;

						pThis->m_Pause.m_Mode = PAUSE_FADEUP ;
						pThis->m_Pause.m_Alpha = 0.0 ;

						if (nDisplayLists)
						{
							nDisplayLists--;
							CEngineApp__UpdateLEGALSCREEN(pThis) ;
							CEngineApp__UpdateFade(pThis) ;
						}
						break;

					// --------------------------- TITLE ---------------------------
					case MODE_TITLE:
						if (pThis->m_bPause == FALSE)
						{
							CPause__Construct(&pThis->m_Pause);
							pThis->m_bPause = TRUE ;
							sky_speed_scaler = 1.0 ;

							// check for corrupted controller pak
//							if (pThis->m_FirstRun)
							{
								ret = SetupFileSystem() ;
								if (	(ret == PFS_ERR_CONTRFAIL)
									|| (ret == PFS_ERR_DEVICE)
									|| (ret == PFS_ERR_ID_FATAL))
								{
									//GetApp()->m_bPause = TRUE;
									//CPause__Construct(&GetApp()->m_Pause);
//									GetApp()->m_Pause.m_Mode = PAUSE_NORMAL;
									CMessageBox__Construct(&GetApp()->m_MessageBox,
																text_errorcontroller, text_pakdamaged) ;
									GetApp()->m_bMessageBox = TRUE ;
									GetApp()->m_MessageBox.b_Active = 2 ;
								}
							}
						}

						if (nDisplayLists)
						{
							nDisplayLists--;
							CEngineApp__UpdateTITLE(pThis) ;
							CEngineApp__UpdateFade(pThis) ;
						}
						break;



					// --------------------------- NEXT LEVEL ---------------------------
					case MODE_RESETLEVEL:
						ASSERT(nDisplayLists == MAX_DISPLAY_LISTS) ;

						// Set default options for attract mode play back/record
						if (CAttractDemo__Active())
						{
							COptions__Store(&pThis->m_Options) ;
							COptions__SetAttractDefaults(&pThis->m_Options) ;
						}
	

						// stab in the dark attempt at fixing a bug...
						//draw_transparent_geometry = TRUE ;
						//draw_intersect_geometry = TRUE ;
						//fx_mode = FXMODE_NONE ;


						// store this warp id as the current levels warp id
						pThis->m_CurrentWarpID = pThis->m_WarpID ;


						// Figure out which boss level the game is on
						switch(pThis->m_WarpID)
						{
							case LONGHUNTER_BOSS_START_WARP_ID:
							case LONGHUNTER_BOSS_END_WARP_ID:
								pThis->m_BossLevel = LONGHUNTER_BOSS_LEVEL ;
								CTurokMovement.CurrentCheckpoint = pThis->m_WarpID ;
								break ;

							case MANTIS_BOSS_WARP_ID:
								pThis->m_BossLevel = MANTIS_BOSS_LEVEL ;
								CTurokMovement.CurrentCheckpoint = pThis->m_WarpID ;
								break ;

							case TREX_BOSS_WARP_ID:
								pThis->m_BossLevel = TREX_BOSS_LEVEL ;
								CTurokMovement.CurrentCheckpoint = pThis->m_WarpID ;
								break ;

							case CAMPAIGNER_BOSS_HALL_WARP_ID:
							case CAMPAIGNER_BOSS_WARP_ID:
								pThis->m_BossLevel = CAMPAIGNER_BOSS_LEVEL ;
								CTurokMovement.CurrentCheckpoint = pThis->m_WarpID ;
								break ;

							default:
								pThis->m_BossLevel = 0 ;

								// Clear any status info ready for next boss level
								CEngineApp__ResetBossVars(pThis);
						}

						// This must be called before CScene construct incase camera object ptr is setup
						// by a game object instance being initialized!
						CCamera__PreSceneConstruct(&pThis->m_Camera) ;

						game_frame_number = 0 ;


						// turn off the map
						pThis->m_MapMode = FALSE ;
						pThis->m_MapAlpha = 0 ;
						pThis->m_MapStatus = MAP_NULL ;

						// reset collision info structures
						CCollisionInfo__Reset();
						COnScreen__Reset(&GetApp()->m_OnScreen) ;

						// must be cleared when cache is cleared
						CTurokMovement.AmbientSound = REG_AMBIENTNONE;
						cache_is_valid = FALSE;


						if (CEngineApp__GetPlayer(pThis))
							health = CEngineApp__GetPlayer(pThis)->m_AI.m_Health ;

						// Clear audio list
						CAudioList__Construct() ;

						i3D_initMemoryAllocation(dynamic_memory_pool, MEMORY_POOL_SIZE);

						CBossVars__Construct(&pThis->m_BossVars);		// must be before scene

						// Just got key on a boss level?
						if (pThis->m_UseCinemaWarp == 3)
						{
							CPickup__DisplayKeysRemaining() ;
							pThis->m_UseCinemaWarp = FALSE ;
						}

						// Goto cinema warp?
						if (pThis->m_UseCinemaWarp)
						{
							if (pThis->m_UseCinemaWarp == 2)
							{
								CPickup__DisplayKeysRemaining() ;
							}
							pThis->m_UseCinemaWarp = FALSE ;
							CScene__Construct(&pThis->m_Scene, CINEMA_WARP_ID) ;
						}
						else
						{
							CScene__Construct(&pThis->m_Scene, pThis->m_WarpID) ;

							// so doesn't ask if you wanna save again
							CTurokMovement.JustEnteredSaveRegion = FALSE ;

							// Re-load weapon icon!
							AI_Update_Turok_Weapon(CEngineApp__GetPlayer(pThis)) ;
						}

						// Call after CScene__Construct!!
						CCamera__Construct(&pThis->m_Camera) ;

						// Construct fx system - swooshes etc
						CFxSystem__Construct(&pThis->m_FxSystem) ;

						if (CEngineApp__GetPlayer(pThis))
							CEngineApp__GetPlayer(pThis)->m_AI.m_Health = health ;

//	rmonPrintf("warp id:%d\n", pThis->m_WarpID) ;

						// if at start of level, display keys remaining
						//if (		((pThis->m_WarpID % 999) == 0)
						//		&&	(!CCamera__InCinemaMode(&pThis->m_Camera)) )
					  	//{
						//	CPickup__DisplayKeysRemaining() ;
						//}


						sky_speed_scaler = 1.0 ;
						enemy_speed_scaler = CEngineApp__GetEnemySpeedScaler(pThis);
						particle_speed_scaler = 1.0 ;

						CEngineApp__ResetMenus(pThis) ;

						pThis->m_Training.m_Mode = TRAINING_NULL;
						pThis->m_Training.m_ID = 0 ;
						pThis->m_Training.m_Alpha = 0 ;
						CTurokMovement.JustEnteredTrainingRegion = FALSE ;

						pThis->m_MenuUpChangeCount = MENU_START_SPEED;
						pThis->m_MenuDownChangeCount = MENU_START_SPEED;
						pThis->m_MenuLeftChangeCount = MENU_START_SPEED;
						pThis->m_MenuRightChangeCount = MENU_START_SPEED;

						pThis->m_bGameOver = FALSE ;
						pThis->m_GameOverAlpha = 0 ;
						pThis->m_GameOverMode = 0 ;
						pThis->m_GameOverTime = 0 ;
						pThis->m_Death = DEATH_NOT_DIEING;

						pThis->m_nCurrentRegionSet = (DWORD) -1;



						// auto checkpoint creation, incase die on new level,
						// you should restart on this level, not at the hub...
						switch(pThis->m_WarpID)
						{
							case 10000:	// TRAINING MODE!
								pThis->m_bTrainingMode = -1 ;			// can draw turok mode
								// fall through
							case 1000:
							case 1999:
							case 2000:
							case 2999:
							case 3000:
							case 3999:
							case 4000:
							case 4999:
							case 5000:
							case 5999:
							case 6000:
							case 6999:
							case 7000:
							case 7999:
							case 8000:
							case 8999:
								if (!(CCamera__InCinemaMode(&pThis->m_Camera)))
									CTurokMovement.CurrentCheckpoint = pThis->m_WarpID ;
								break ;
						}


						// figure out which level the game is on
						if ((pThis->m_WarpID >= 0) && (pThis->m_WarpID < 1000))
								pThis->m_Level = 1 ;
						else if ((pThis->m_WarpID >= 1000) && (pThis->m_WarpID < 2000))
								pThis->m_Level = 2 ;
						else if ((pThis->m_WarpID >= 2000) && (pThis->m_WarpID < 3000))
								pThis->m_Level = 3 ;
						else if ((pThis->m_WarpID >= 3000) && (pThis->m_WarpID < 4000))
								pThis->m_Level = 4 ;
						else if ((pThis->m_WarpID >= 4000) && (pThis->m_WarpID < 5000))
								pThis->m_Level = 5 ;
						else if ((pThis->m_WarpID >= 5000) && (pThis->m_WarpID < 6000))
								pThis->m_Level = 6 ;
						else if ((pThis->m_WarpID >= 6000) && (pThis->m_WarpID < 8000))
								pThis->m_Level = 7 ;
						else if ((pThis->m_WarpID >= 8000) && (pThis->m_WarpID < 9000))
								pThis->m_Level = 8 ;


						// Restore all bosses
						CBossesStatus__RestoreAllBossesStatus(&GetApp()->m_BossesStatus) ;

						// Check for front end levels
						switch(pThis->m_WarpID)
						{
							case LEGALSCREEN_WARP_ID:
								pThis->m_Mode = MODE_LEGALSCREEN ;
								CCamera__ConstructIntro(&pThis->m_Camera) ;
								break ;

							case INTRO_ACCLAIM_LOGO_WARP_ID:
							case INTRO_IGGY_WARP_ID:
							case INTRO_TUROK_DRAWING_BOW_WARP_ID:
							case INTRO_ZOOM_TO_LOGO_WARP_ID:
							case GAME_INTRO_KEY_WARP_ID:
							case GAME_INTRO_WARP_ID:
								pThis->m_Mode = MODE_INTRO ;
								CCamera__ConstructIntro(&pThis->m_Camera) ;
								break ;

							case CHEAT_GALLERY_WARP_ID:
								pThis->m_Mode = MODE_GALLERY;
								pThis->m_Portrait = 0;
								pThis->m_PortraitNameMode = PORTRAITNAME_APPEAR;
								pThis->m_PortraitAlpha = 0.0;
								pThis->m_PortraitName = pThis->m_Portrait;
								pThis->m_PortraitExit = FALSE;
								CEngineApp__ResetGallery(pThis);
								CCamera__ConstructGallery(&pThis->m_Camera);
								break;

							case CREDITS_WARP_ID:
								pThis->m_Mode = MODE_CREDITS;
								pThis->m_CinemaFlags = CINEMA_FLAG_CREDIT;
								CCamera__Construct(&pThis->m_Camera);
								CCredits__Construct();
								break ;

							default:
								pThis->m_Mode = MODE_GAME ;

   							if (pThis->m_JustLoadedGame)
								{
									warp = pThis->m_WarpID ;
									CEngineApp__NewLife(GetApp()) ;
									pThis->m_WarpID = warp ;

									// extract data (again) to make sure health and backpack etc.. are set
									CLoad__ExtractData() ;
									pThis->m_JustLoadedGame = FALSE ;

								}

								// check for dying when weapon has been previously removed
								// ie. cliffs & ladders
								if (CTurokMovement.WeaponSelectIndex == 255)
								{
									// put weapon back to what it was prior to climbing
									CTMove__SelectWeaponByAIType(&CTurokMovement, CTurokMovement.WeaponAtStartOfClimb, TRUE);
								}
								else
								{
									// put weapon back to what it was
									CTurokMovement.WeaponSelect = WeaponOrder[CTurokMovement.WeaponSelectIndex];
								}

								// Put turok weapon back on screen
								CTurokMovement.WeaponCurrent = CTurokMovement.WeaponSelect ;
								AI_Reset_Turok_Weapon() ;
								AI_Update_Turok_Weapon(CEngineApp__GetPlayer(pThis)) ;
								COnScreen__QuickWeaponChange(&pThis->m_OnScreen, CTurokMovement.WeaponSelectIndex) ;
								break ;
						}
						break;

					// --------------------------- INTRO & GAME ---------------------------
					case MODE_INTRO:
					case MODE_GAME:
					case MODE_GAMEOVER:
					case MODE_GALLERY:
						if (nDisplayLists)
						{
							nDisplayLists--;
							CEngineApp__UpdateGAME(pThis) ;
							CEngineApp__UpdateFade(pThis) ;
						}
						break;


					// --------------------------- WAITFORDISPLAY ---------------------------
					// wait for all display lists to be freed up, then goto next level, or restart
					case MODE_WAITFORDISPLAY:
						if ((nDisplayLists == MAX_DISPLAY_LISTS) && !pThis->m_bReset)
						{
							pThis->m_Mode = pThis->m_NextMode;
							pThis->m_ModeTime = 0 ;
						}
						break;

					// --------------------------- CREDITS ---------------------------
					case MODE_CREDITS:
//						pThis->m_FogColor[0] = 0 ;
//						pThis->m_FogColor[1] = 0 ;
//						pThis->m_FogColor[2] = 0 ;
//						pThis->m_FogColor[3] = 0 ;
//						pThis->m_cRegionSetBlend = 1;

						if (nDisplayLists)
						{
							nDisplayLists--;
							CEngineApp__UpdateCREDITS(pThis) ;
							CEngineApp__UpdateFade(pThis) ;
						}
						break;


					// ------------------------------------------------------------
					default:
						ASSERT(FALSE);
				}
				break;

			case OS_SC_DONE_MSG:
				CEngineApp__UpdateFrameData(pThis, pMsg);

				// set audio volume
				if (pThis->m_bPause == FALSE)
					pThis->m_ActualMusicVolume = ((float)GetApp()->m_Options.m_MusicVolume / 255.0);

				dm = (pThis->m_ActualMusicVolume - pThis->m_MusicVolume) / 3;
				pThis->m_MusicVolume += dm;

				dm = (pThis->m_ActualSFXVolume - pThis->m_SFXVolume) / 3;
				pThis->m_SFXVolume += dm;

				musicvolume = pThis->m_MusicVolume;

				switch(CTurokMovement.MusicID)
				{
					case 0:		musicvolume *= .87;	break;	// Done
					case 1:		musicvolume *= .78;	break;	// Done
					case 2:		musicvolume *= 1.0;	break;	// Done
					case 3:		musicvolume *= .75;	break;	// Done
					case 4:		musicvolume *= .77;	break;	// Done
					case 5:		musicvolume *= .65;	break;	// Done
					case 6:		musicvolume *= 1.0;	break;	// Done
					case 7:		musicvolume *= .69;	break;	// Done
					case 8:		musicvolume *= .70;	break;	// Done
					case 9:		musicvolume *= .60;	break;	// Done
					case 11:		musicvolume *= 1.0;	break;	// Done
					case 12:		musicvolume *= .75;	break;	// Done
					case 13:		musicvolume *= .58;	break;	// Done
					case 14:		musicvolume *= .78;	break;	// Done
					case 100:	musicvolume *= .90;	break;	// Done

					default:;
				}

				SetAudioVolume(musicvolume, pThis->m_SFXVolume);

				switch (pThis->m_Mode)
				{
					case MODE_GAMEPAK:
						//nDisplayLists++;
						CEngineApp__UpdateGAMEPAK(pThis) ;
						CEngineApp__UpdateFade(pThis) ;
						break;

					case MODE_LEGALSCREEN:
						//nDisplayLists++;
						CEngineApp__UpdateLEGALSCREEN(pThis) ;
						CEngineApp__UpdateFade(pThis) ;
						break;


					case MODE_TITLE:
						//nDisplayLists++;
						CEngineApp__UpdateTITLE(pThis) ;
						CEngineApp__UpdateFade(pThis) ;
						break;


					case MODE_GALLERY:
					case MODE_INTRO:
					case MODE_GAME:
					case MODE_GAMEOVER:
#ifdef DB_LOOP
						rmonPrintf("%x: RCP task complete\n", pFrameData);
#endif
						//nDisplayLists++;
						CEngineApp__UpdateGAME(pThis) ;
						CEngineApp__UpdateFade(pThis) ;

						if (pThis->m_Mode == MODE_GALLERY)
							CEngineApp__UpdateGallery(pThis);

						break;

					case MODE_WAITFORDISPLAY:
						nDisplayLists++;
						break;

					//case MODE_FRONTEND_TITLE:
					//	break;

					case MODE_CREDITS:
						CEngineApp__UpdateCREDITS(pThis) ;
						CEngineApp__UpdateFade(pThis) ;
						break;


					default:
						break;
				}
				break;

			case (CONTROLLER_MSG):
				UpdateController();
				cntrlReadInProg = FALSE;
				cntrlReadYet = TRUE;
				joyReadCount = 0;
				break;

			case (OS_SC_PRE_NMI_MSG):
				CEngineApp__SetupFadeTo(pThis, MODE_WAITFORDISPLAY);
				pThis->m_bReset = TRUE;

				CScene__DoSoundEffect(&pThis->m_Scene,
											 SOUND_RAPTOR_ATTACK, 1,
											 pThis, &CEngineApp__GetPlayer(pThis)->ah.ih.m_vPos, 0);
				break;
        }
    }
}

void CEngineApp__UpdateFrameData(CEngineApp *pThis, GFXMsg *pMsg)
{
	if (pMsg == &pThis->m_FrameData[0].m_Msg)
		pFrameData = &pThis->m_FrameData[0];
	else if (pMsg == &pThis->m_FrameData[1].m_Msg)
		pFrameData = &pThis->m_FrameData[1];
	else if (pMsg == &pThis->m_FrameData[2].m_Msg)
		pFrameData = &pThis->m_FrameData[2];
	else
		ASSERT(FALSE);
}


/*****************************************************************************
*
*	Function:		CEngineApp__UpdateGAMEPAK()
*
******************************************************************************
*
*	Description:	Update GAMEPAK mode stuff
*
*	Inputs:			*pThis	-	Ptr to engine
*	Outputs:			None
*
*****************************************************************************/
void CEngineApp__UpdateGAMEPAK(CEngineApp *pThis)
{
	s32		ret ;

	// request latest controller information
	if (validcontrollers && !cntrlReadInProg)
	{
		cntrlReadInProg = TRUE;
		osContStartReadData(&gfxFrameMsgQ);
	}
	if (cntrlReadYet)
	{
		ReadControllerAdvanced();

//		if ( CTControl__IsStart(pCTControl))
//		{
//				CEngineApp__SetupFadeTo(pThis, MODE_RESETGAME) ;
//		}
	}

	// 60 fps
	CEngineApp__PrepareFrame(pThis, 1);

	ret = CGamePak__Update(&pThis->m_GamePak);
	if (ret != -1)
	{
		pThis->m_bGamePak = FALSE;
		CEngineApp__SetupFadeTo(pThis, MODE_RESETGAME);
	}
}

/*****************************************************************************
*
*	Function:		CEngineApp__UpdateLEGALSCREEN()
*
******************************************************************************
*
*	Description:	Update LEGALSCREEN mode stuff
*
*	Inputs:			*pThis	-	Ptr to engine
*	Outputs:			None
*
*****************************************************************************/
void CEngineApp__UpdateLEGALSCREEN(CEngineApp *pThis)
{
	// request latest controller information
	if (validcontrollers && !cntrlReadInProg)
	{
		cntrlReadInProg = TRUE;
		osContStartReadData(&gfxFrameMsgQ);
	}
	if (cntrlReadYet)
	{
		ReadControllerAdvanced();

		// skip a frame, then check if the start button is ever released
		if (	(pThis->m_LegalTimer++ > 0)
			&& (PlayerControllerData.button != START_BUTTON))
		{
//			ASSERT(FALSE) ;
			pThis->m_GamePakCheck = FALSE ;
		}

		if (	(pThis->m_GamePakCheck)
			&& (pThis->m_LegalTimer >1)
			&& (pThis->m_FadeStatus == FADE_NULL))
		{
			CGamePak__Construct(&pThis->m_GamePak) ;
			if (pThis->m_GamePak.b_Active)
				CEngineApp__SetupFadeTo(pThis, MODE_GAMEPAK) ;
		}

//		else if ((pThis->m_LegalBypass) && (CTControl__IsStart(pCTControl)))
		else if ((CTControl__IsStart(pCTControl)))
		{
//			if (!IntroFirstRun)
			if (TRUE)
			{
				pThis->m_WarpID = INTRO_ZOOM_TO_LOGO_WARP_ID ;
				SkipIntro = TRUE ;
			}
			else
				pThis->m_WarpID = INTRO_ACCLAIM_LOGO_WARP_ID ;

			CEngineApp__SetupFadeTo(pThis, MODE_RESETLEVEL) ;
		}
	}
	// 60 fps
	CEngineApp__PrepareFrame(pThis, 1);

	CLegalScreen__Update();
}



/*****************************************************************************
*
*	Function:		CEngineApp__UpdateTITLE()
*
******************************************************************************
*
*	Description:	Update TITLE mode stuff
*
*	Inputs:			*pThis	-	Ptr to engine
*	Outputs:			None
*
*****************************************************************************/
void CEngineApp__UpdateTITLE(CEngineApp *pThis)
{
	// Needs this for correct record/playback of attract demo!!
	BOOL	Sync ;
	BOOL	StartPressed = FALSE ;

	INT32	Warp ;

	// Check for grabbing a new demo from rom
	CAttractDemo__CheckForNewDemoRequest() ;

	// 60 fps
	CEngineApp__PrepareFrame(pThis, 1) ;
	CEngineApp__UpdateCameraAttributes(pThis);

	// Setup sync var
	Sync = (pThis->m_FadeStatus == FADE_NULL) && ((frame_number % 3) == 0) && (even_odd == 0) ;

	// Request latest controller information
	if ((validcontrollers) && (!cntrlReadInProg))
	{
		cntrlReadInProg = TRUE;
		osContStartReadData(&gfxFrameMsgQ) ;
	}

	// Controller ready?
	if (cntrlReadYet)
	{
		ReadControllerAdvanced() ;
		//StartPressed = CTControl__IsStart(pCTControl) ;

		if (CPause__Update(&pThis->m_Pause) != -1)
			StartPressed = TRUE ;
		else
			StartPressed = FALSE ;
	}

	// Play/Record a demo?
	if ((Sync) &&
		 (((pThis->m_ModeTime >= SECONDS_TO_FRAMES(20)) && (CAttractDemo__Play())) ||
		  ((StartPressed) && (CAttractDemo__CheckForRecord()))))
	{
		Warp = pThis->m_WarpID ;

		InitializeRandomSeed() ;
		frame_number = 0 ;
		game_frame_number = 0 ;


		frame_increment = 1.0;
		enemy_speed_scaler = 1.0;
		particle_speed_scaler = 1.0;
		sky_speed_scaler = 1.0;


		// Clear and re-initialise movement vars!!
		memset(&CTurokMovement, 0, sizeof(CTMove)) ;
		memset(&CTControlArray, 0, sizeof(CTControlArray)) ;

		memset(controllerdata, 0, sizeof(OSContPad) * MAXCONTROLLERS) ;
		memset(&PlayerControllerData, 0, sizeof(OSContPad)) ;
		memset(&ControlStates, 0, sizeof(CControlState)) ;
		InitControllersAdvanced() ;


		pCTControl = CTControl__CTControl(pThis->m_Options.m_RHControl) ;
		CTMove__CTMove() ;

		// Start game
		CEngineApp__NewLife(pThis) ;

		CEngineApp__InitializeGameVars(pThis) ;

		// Put warp back to what it was for attract code sets it up
		// and "CEngineApp__NewLife" changes it !
		pThis->m_WarpID = Warp ;
	}
	else
	if ((Sync) && (pThis->m_ModeTime >= SECONDS_TO_FRAMES(30)))
	{
		CLegalScreen__Construct() ;
		CEngineApp__SetupFadeTo(pThis, MODE_RESETLEVEL) ;
	}
	else
	// Start a normal game
	if ((pThis->m_Mode == MODE_TITLE) && (StartPressed) && (!CAttractDemo__CheckForRecord()))
	{
		// in training mode, bypass the 'instructions'
 		if (pThis->m_bTrainingMode)
 		{
 			CEngineApp__NewLife(GetApp()) ;
			pThis->m_WarpID = 10000 ;
		}
		else
			pThis->m_WarpID = GAME_INTRO_WARP_ID ;

		CEngineApp__SetupFadeTo(pThis, MODE_RESETLEVEL) ;
	}
}









void CEngineApp__NewLife(CEngineApp *pThis)
{
	pThis->m_Death = DEATH_WAITING;
	pThis->m_Flash = 0;
	pThis->m_Alpha = 0;

	// setup life
	CTMove__NewLifeSetup(&CTurokMovement);

//	rmonPrintf("checkpoint id:%d\n", CTurokMovement.CurrentCheckpoint) ;

	// DO NOT SET WARPID TO ANYTHING HERE COZ IT CONTAINS THE CHECKPOINT ID TO START AT!

	CEngineApp__SetupFadeTo(pThis, MODE_RESETLEVEL);
}


/*****************************************************************************
*
*	Function:		CEngineApp__UpdateGAME()
*
******************************************************************************
*
*	Description:	Update movement for GAME mode, reads controllers and moves
*						camera with turok
*
*	Inputs:			*pThis	-	Ptr to engine
*	Outputs:			None
*
*****************************************************************************/
//char	buff[32] ;

void CEngineApp__UpdateGAME(CEngineApp *pThis)
{
#ifdef PLATFORM_PORT
	/* render-interpolation state: the player's true pos+yaw at the last two logic ticks. The render draws
	 * the player (and thus the camera that follows it + the 1st-person weapon) at lerp(prev,cur,alpha) so
	 * motion is smooth at 60fps despite 30Hz logic. Snapshot below after CTMove; restore after the task. */
	static CVector3 _ipPrevPos, _ipCurPos; static float _ipPrevRotY, _ipCurRotY;
	static float _ipPrevPitch, _ipCurPitch;   /* m_RotXOffset (look up/down) — engine field, set by CTMove */
	static float _ipPrevRotYO, _ipCurRotYO;   /* m_RotYOffset (head yaw offset) — engine field */
	static float _ipPrevRotZO, _ipCurRotZO;   /* m_RotZOffset (head roll offset) — engine field */
	static CQuatern _ipPrevQG, _ipCurQG;      /* player m_qGround (ground-slope quat, fed into the view) */
	static int _ipHave = 0, _ipActive = 0;
#endif
#ifdef PLATFORM_PORT
	/* PORT (M5 collision streaming): the N64 streams collision through a small cart cache, so the
	 * collision buffer is periodically re-decompressed at a NEW address (e.g. when a key cinematic's
	 * model-swap loads the body model). The engine never updates the player's m_pCurrentRegion after a
	 * relocation, so it goes stale (corner pointers point into the freed/old buffer). N64 (no MMU)
	 * tolerates the stale reads; a protected host faults, and the previous workarounds (NULL the region)
	 * lost fog + dropped the player through the floor. CORRECT FIX: re-acquire the player's region at its
	 * current position in the LIVE buffer via CScene__NearestRegion, so collision, ground, fog and the
	 * camera all keep working with a valid region. Detected once per frame here, before the draw runs the
	 * player's collision. (The Collision3 entry guard still covers the same-frame window before this.) */
	{ CGameObjectInstance *_pl = CEngineApp__GetPlayer(pThis);
	  if (_pl && PORT_REGION_BAD(_pl->ah.ih.m_pCurrentRegion))
	    _pl->ah.ih.m_pCurrentRegion = CScene__NearestRegion(&pThis->m_Scene, &_pl->ah.ih.m_vPos); }
#endif

	// Request latest controller information
	if (validcontrollers && !cntrlReadInProg)
	{
		cntrlReadInProg = TRUE;
		osContStartReadData(&gfxFrameMsgQ);
	}

	// Read controller
	if (cntrlReadYet)
	{
		ReadPlayerController() ;

		// Start pressed? - if so signal it to camera
		if ((CTControl__IsStart(pCTControl)) && (pThis->m_FadeStatus == FADE_NULL))
			pThis->m_Camera.m_StartPressed = TRUE ;
	}

//	sprintf(buff, "crash %d", CrashId) ;
//	COnScreen__AddGameTextWithId(buff, 39) ;

	// Calculate frame rate
	switch(pThis->m_CurrentWarpID)
	{
		case INTRO_TUROK_DRAWING_BOW_WARP_ID:
		case INTRO_ZOOM_TO_LOGO_WARP_ID:
			nNextFields = CALC_FRAMERATE(3) ;
			break ;

		case GAME_INTRO_WARP_ID:
		case GAME_INTRO_KEY_WARP_ID:
			nNextFields = CALC_FRAMERATE(4) ;
			break ;

		case CHEAT_GALLERY_WARP_ID:
			nNextFields = CALC_FRAMERATE(1) ;
			break ;


		default:
			nNextFields = CALC_FRAMERATE(2) ;
	}

	// Other special case frame rates more more smoothness
	switch(pThis->m_Camera.m_Mode)
	{
		case CAMERA_CINEMA_TUROK_RESURRECT_MODE:

		case CAMERA_CINEMA_END_SEQUENCE_MODE: 			// Turok escaping from campainer
		case CAMERA_CINEMA_END_SEQUENCE2_MODE:   		// Turok in extra bit, running away

			nNextFields = CALC_FRAMERATE(3) ;
			break ;
	}


	// Attract code may modify controller values/nNextFields
	CAttractDemo__Update() ;
	CAttractDemo__CheckForUpdatingGameVars() ;

	// Demo's were recorded at 60 frames per sec!!
	if (CAttractDemo__Active())
		frame_increment = REFRESHES_TO_FRAME_INC(nNextFields, 60.0) ;
	else
		frame_increment = REFRESHES_TO_FRAME_INC(nNextFields, refresh_rate) ;

#ifdef PLATFORM_PORT
	/* PORT: Turok's frame_increment is sized for a 30fps step (CALC_FRAMERATE floors nNextFields at 2),
	 * but the host renders at 60fps — applying a 30fps step every 60fps frame ran the game ~2x too fast.
	 * Render at TUROK_FPS but advance the LOGIC only at TUROK_TICK_FPS (default 30): on render-only
	 * frames the frame-pump clears g_turok_logic_tick and we freeze the step so the frame just
	 * re-presents the same state. (port/src/os_shim.c owns the timing.) */
	{ extern int g_turok_logic_tick; if (!g_turok_logic_tick) frame_increment = 0.0; else g_turok_anim_step = frame_increment; }
#endif

	// Process player controller values
	ProcessPlayerController() ;

	// Start pressed? - if so signal it to camera
	if ((CTControl__IsStart(pCTControl)) && (pThis->m_FadeStatus == FADE_NULL))
		pThis->m_Camera.m_StartPressed = TRUE ;

	// Update turok
	if ((pThis->m_bPause==FALSE) && (pThis->m_Warp == WARP_NOT_WARPING) && (pThis->m_Death == DEATH_NOT_DIEING) &&(pThis->m_bTraining ==FALSE))
		CTMove__UpdateTurokInstance(pCTMove, pThis, pCTControl);

#ifdef PLATFORM_PORT
	/* PORT render interpolation: snapshot the player's true pos/yaw on logic-tick frames, then render
	 * the player at lerp(prev,cur,alpha) every frame so the camera (which follows the player) and the
	 * 1st-person weapon move smoothly between the 30Hz logic ticks. The graphics task built below uses
	 * the interpolated pos; it's restored to the exact logic pos right after, so the next tick is exact.
	 * A large jump between ticks (warp/respawn/teleport) snaps instead of sliding across the level. */
	{
		extern int g_turok_logic_tick; extern float turok_render_alpha(void);
		CGameObjectInstance *_pl = CEngineApp__GetPlayer(pThis);
		if (_pl)
		{
			if (g_turok_logic_tick || !_ipHave)
			{
				if (_ipHave) { _ipPrevPos = _ipCurPos; _ipPrevRotY = _ipCurRotY; _ipPrevPitch = _ipCurPitch;
				               _ipPrevRotYO = _ipCurRotYO; _ipPrevRotZO = _ipCurRotZO; _ipPrevQG = _ipCurQG; }
				else         { _ipPrevPos = _pl->ah.ih.m_vPos; _ipPrevRotY = _pl->m_RotY; _ipPrevPitch = pThis->m_RotXOffset;
				               _ipPrevRotYO = pThis->m_RotYOffset; _ipPrevRotZO = pThis->m_RotZOffset; _ipPrevQG = _pl->m_qGround; }
				_ipCurPos = _pl->ah.ih.m_vPos; _ipCurRotY = _pl->m_RotY; _ipCurPitch = pThis->m_RotXOffset;
				_ipCurRotYO = pThis->m_RotYOffset; _ipCurRotZO = pThis->m_RotZOffset; _ipCurQG = _pl->m_qGround; _ipHave = 1;
				{ float dx=_ipCurPos.x-_ipPrevPos.x, dy=_ipCurPos.y-_ipPrevPos.y, dz=_ipCurPos.z-_ipPrevPos.z;
				  /* SNAP (don't lerp) across a big inter-tick jump — a warp/teleport — since interpolating across
				   * it would slide the model and drag the camera through the world. Normal per-tick motion is far
				   * under this threshold. (Death respawns no longer teleport the player: the region re-acquire
				   * below grounds them in place, so this just guards genuine level warps.) */
				  if (dx*dx+dy*dy+dz*dz > 1000.0f*1000.0f) {
				    _ipPrevPos = _ipCurPos; _ipPrevRotY = _ipCurRotY; _ipPrevPitch = _ipCurPitch;
				    _ipPrevRotYO = _ipCurRotYO; _ipPrevRotZO = _ipCurRotZO; _ipPrevQG = _ipCurQG; } }
			}
			{
				float a = turok_render_alpha();
				if (a > 0.0f)
				{
					_pl->ah.ih.m_vPos.x = _ipPrevPos.x + (_ipCurPos.x - _ipPrevPos.x)*a;
					_pl->ah.ih.m_vPos.y = _ipPrevPos.y + (_ipCurPos.y - _ipPrevPos.y)*a;
					_pl->ah.ih.m_vPos.z = _ipPrevPos.z + (_ipCurPos.z - _ipPrevPos.z)*a;
					{ float d = _ipCurRotY - _ipPrevRotY;
					  while (d >  3.14159265f) d -= 6.28318531f;
					  while (d < -3.14159265f) d += 6.28318531f;
					  _pl->m_RotY = _ipPrevRotY + d*a; }
					/* look-pitch (m_RotXOffset, engine field set by CTMove): interpolate so looking up/down
					 * is smooth too. Camera reads it in CCamera__Update right after SetCameraToTurok. */
					{ float dp = _ipCurPitch - _ipPrevPitch;
					  while (dp >  3.14159265f) dp -= 6.28318531f;
					  while (dp < -3.14159265f) dp += 6.28318531f;
					  pThis->m_RotXOffset = _ipPrevPitch + dp*a; }
					/* head yaw/roll offsets (m_RotYOffset/m_RotZOffset): also fed into the view (camera.c
					 * RotY+RotYOffset / RotZOffset). Interpolate so they don't snap each tick during turns. */
					{ float dyo = _ipCurRotYO - _ipPrevRotYO;
					  while (dyo >  3.14159265f) dyo -= 6.28318531f;
					  while (dyo < -3.14159265f) dyo += 6.28318531f;
					  pThis->m_RotYOffset = _ipPrevRotYO + dyo*a; }
					{ float dzo = _ipCurRotZO - _ipPrevRotZO;
					  while (dzo >  3.14159265f) dzo -= 6.28318531f;
					  while (dzo < -3.14159265f) dzo += 6.28318531f;
					  pThis->m_RotZOffset = _ipPrevRotZO + dzo*a; }
					/* ground-slope quat (player m_qGround -> engine via SetCameraToTurok -> view matrix):
					 * shortest-path nlerp so the camera's ground-relative tilt doesn't snap at tick edges. */
					{ float qdot = _ipPrevQG.x*_ipCurQG.x + _ipPrevQG.y*_ipCurQG.y + _ipPrevQG.z*_ipCurQG.z + _ipPrevQG.t*_ipCurQG.t;
					  float s = (qdot < 0.0f) ? -1.0f : 1.0f;
					  float qx = _ipPrevQG.x + (s*_ipCurQG.x - _ipPrevQG.x)*a;
					  float qy = _ipPrevQG.y + (s*_ipCurQG.y - _ipPrevQG.y)*a;
					  float qz = _ipPrevQG.z + (s*_ipCurQG.z - _ipPrevQG.z)*a;
					  float qt = _ipPrevQG.t + (s*_ipCurQG.t - _ipPrevQG.t)*a;
					  float qm = (float)sqrt((double)(qx*qx + qy*qy + qz*qz + qt*qt));
					  if (qm > 1e-6f) { qx/=qm; qy/=qm; qz/=qm; qt/=qm; }
					  _pl->m_qGround.x = qx; _pl->m_qGround.y = qy; _pl->m_qGround.z = qz; _pl->m_qGround.t = qt; }
					_ipActive = 1;
				}
			}
		}
	}
#endif

	CEngineApp__SetCameraToTurok(pThis);
	CCamera__Update(&pThis->m_Camera) ;
#ifdef PLATFORM_PORT
	/* PORT (death fall-through fix, corrected): the death/resurrect cinematic respawns the player but leaves
	 * m_pCurrentRegion pointing at a VALID-but-WRONG region — one whose pointer passes PORT_REGION_BAD yet
	 * does NOT contain the player's new X/Z — so the ground query finds nothing and the player free-falls
	 * through the floor. (Confirmed empirically: the region pointer is valid (rbad=0) while Y free-falls, and
	 * forcing a NearestRegion re-acquire holds the player rock-solid on the ground.) PORT_REGION_BAD only
	 * catches a NULL/stale/wild pointer, never a wrong-but-valid one, so re-acquire the CORRECT region
	 * (NearestRegion at the live position) for the whole cinematic + a window after it — the respawn lands as
	 * the cinematic ends. Scoped to cinematics, so normal play keeps the game's own region tracking. */
	{ static int _rw = 0; extern int CCamera__InCinemaMode(CCamera*);
	  CGameObjectInstance *_pl = CEngineApp__GetPlayer(pThis);
	  if (CCamera__InCinemaMode(&pThis->m_Camera)) _rw = 30; else if (_rw > 0) _rw--;
	  if (_pl && (_rw > 0 || PORT_REGION_BAD(_pl->ah.ih.m_pCurrentRegion)))
	    _pl->ah.ih.m_pCurrentRegion = CScene__NearestRegion(&pThis->m_Scene, &_pl->ah.ih.m_vPos); }
#endif
	CEngineApp__UpdateCameraAttributes(pThis);


	// Start tasks
	pFrameData = CEngineApp__CreateGraphicsTask(pThis);
	pFrameData->m_nPredictFields = nNextFields;
	CEngineApp__SendGraphicsTask(pThis, pFrameData);

#ifdef PLATFORM_PORT
	/* restore the player's exact logic pos/yaw/pitch + head offsets + ground quat after the interpolated render */
	if (_ipActive) { CGameObjectInstance *_pl = CEngineApp__GetPlayer(pThis);
	  if (_pl) { _pl->ah.ih.m_vPos = _ipCurPos; _pl->m_RotY = _ipCurRotY; _pl->m_qGround = _ipCurQG; }
	  pThis->m_RotXOffset = _ipCurPitch; pThis->m_RotYOffset = _ipCurRotYO; pThis->m_RotZOffset = _ipCurRotZO; _ipActive = 0; }
#endif

	// Update Region Music
	UpdateSeq();

	// Play sounds in audio list
	if (game_frame_number == 3)
		CAudioList__Flush() ;
}


/*****************************************************************************
*
*	Function:		CEngineApp__UpdateCREDITS()
*
******************************************************************************
*
*	Description:	Update CREDITS mode stuff
*
*	Inputs:			*pThis	-	Ptr to engine
*	Outputs:			None
*
*****************************************************************************/
void CEngineApp__UpdateCREDITS(CEngineApp *pThis)
{
	// request latest controller information
	if (validcontrollers && !cntrlReadInProg)
	{
		cntrlReadInProg = TRUE;
		osContStartReadData(&gfxFrameMsgQ);
	}

	if (cntrlReadYet)
	{
		ReadControllerAdvanced();

//		if (CTControl__IsStart(pCTControl))
//			CEngineApp__SetupFadeTo(pThis, MODE_RESETGAME) ;
	}

	CEngineApp__UpdateCameraAttributes(pThis);
	CEngineApp__PrepareFrame(pThis, 4);				// 15 fps
	CCredits__Update();

	if	(pThis->m_bPause)
	{
		if (CTControl__IsStart(pCTControl))
			CEngineApp__SetupFadeTo(pThis, MODE_RESETGAME) ;
//		ret = CPause__Update(&pThis->m_Pause);
//		if (ret != -1)
//				CEngineApp__SetupFadeTo(GetApp(), MODE_RESETGAME);
	}
}

void CEngineApp__PrepareFrame(CEngineApp *pThis, int nMinimumFields)
{
	// hey Rob, these are here to stop the fog being black on the
	// title page, and to setup the camera position correctly for
	// sound
	//CEngineApp__SetCameraToTurok(pThis);
	CCamera__Update(&pThis->m_Camera) ;
	//CEngineApp__UpdateCameraAttributes(pThis);


	nNextFields = CALC_FRAMERATE(nMinimumFields);

//	frame_increment = nNextFields/(float) FRAME_RATE;
	frame_increment = REFRESHES_TO_FRAME_INC(nNextFields, refresh_rate);
	pFrameData = CEngineApp__CreateGraphicsTask(pThis);
	pFrameData->m_nPredictFields = nNextFields;

	CEngineApp__SendGraphicsTask(pThis, pFrameData);
}


void CEngineApp__GetScreenCoordinates(CEngineApp *pThis, CVector3 *pvIn, float *pX, float *pY)
{
	CVector3 vProj;

	CMtxF__VectorMult4x4(pThis->m_mfProjection, pvIn, &vProj);

	*pX = (vProj.x * (SCREEN_WD/2)) + (SCREEN_WD/2);
	*pY = (vProj.y * (-SCREEN_HT/2)) + (SCREEN_HT/2);
}


float CEngineApp__GetEnemySpeedScaler(CEngineApp *pThis)
{
	// If in a cinema, run anims at default speed
	if ((CCamera__InCinemaMode(&pThis->m_Camera)) || (GetApp()->m_Mode != MODE_GAME))
		return 1.0 ;

	// Bosses are special case - not too much change
	if (pThis->m_pBossActive)
	{
		switch(pThis->m_Difficulty)
		{
			case EASY:
				return 0.85 ;

			case MEDIUM:
			default:
				return 1.0 ;

			case HARD:
				return 1.15 ;
		}
	}
	else
	{
		// Enemies
		switch(pThis->m_Difficulty)
		{
			case HARD:
				return 1.5;

			default:
			case EASY:
			case MEDIUM:
				return 1.0;
		}
	}
}

float CEngineApp__GetBossHitPointScaler(CEngineApp *pThis)
{
		switch(pThis->m_Difficulty)
		{
			case EASY:
				return 0.9 ;

			case MEDIUM:
			default:
				return 1.0 ;

			case HARD:
				return 1.1 ;

		}
}


void CEngineApp__ResetBossVars(CEngineApp *pThis)
{
	CBossesStatus__Construct(&pThis->m_BossesStatus) ;
	CBossVars__Construct(&pThis->m_BossVars);		// must be before scene
	pThis->m_pBossActive = NULL ;


	// includes the boss bar (must be called after above !!!)
	COnScreen__Reset(&pThis->m_OnScreen);
}
