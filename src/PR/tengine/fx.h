// fx.h

#ifndef _INC_FX
#define _INC_FX

/////////////////////////////////////////////////////////////////////////////


#include "defs.h"
#include "romstruc.h"
#include "graphu64.h"
#include "boss.h"
#include "mattable.h"
#include "lists.h"
#include "particle.h"



/////////////////////////////////////////////////////////////////////////////
// Constants
/////////////////////////////////////////////////////////////////////////////
#define MAX_FXSYSTEM_VERTICES	400	// Total number of vertices availalbe for all swooshes
#define MAX_SWOOSH_EDGES		150	// Max edges available for swooshes
#define MAX_SWOOSHES				32		// Max swooshes on screen at once
#define MAX_TIMER_FX				3		// Total number of timer fx's



/////////////////////////////////////////////////////////////////////////////
// Definition Structures
/////////////////////////////////////////////////////////////////////////////

// Vertex definition
typedef struct CSwooshVertex_t
{
	CVector3		m_vPos ;		// Position
	CRGB			m_Color ;	// Color
	FLOAT			m_Alpha ;	// Alpha
} CSwooshVertex ;

// Edge vertex definition
typedef struct CSwooshEdgeVertexInfo_t
{
	CSwooshVertex	m_Start, m_End ;	// Start, end definition
} CSwooshEdgeVertexInfo ;

// Swoosh definition
typedef struct CSwooshInfo_t
{
	FLOAT							m_LifeTime ;				// Total time on screen
	FLOAT							m_FadeOutTime ;			// Total time to fade swoosh out
	FLOAT							m_EdgeLifeTime ;			// Total edge life time
	INT16							m_Node ;						// Object node to attatch swoosh to
	FLOAT							m_Scale ;					// Vertex scaler
	INT16							m_Flags ;					// General flags
	INT16							m_NoOfEdgeVertices ;		// No of vertices in edge
	CSwooshEdgeVertexInfo	*m_pEdgeVertexList ;		// Ptr to edge vertices
} CSwooshInfo ;

// General flags
#define SWOOSH_CLOSE_EDGES		(1<<0)


/////////////////////////////////////////////////////////////////////////////
// Structures
/////////////////////////////////////////////////////////////////////////////

// Swoosh edge structures
typedef struct CSwooshEdge_t
{
	CNode			m_Node ;							// Linked list node
	FLOAT			m_LifeTime ;					// Current time for edge
	FLOAT			m_TotalLifeTime ;				// Total time for sedge
	Mtx			m_mMatrix ;						// Orientation matrix
} CSwooshEdge ;

enum SwooshTypes
{
	PARTICLE_SWOOSH,
	OBJECT_SWOOSH,
	END_SWOOSH
} ;

// Swoosh structure
typedef struct CSwoosh_t
{
	CNode						m_Node ;					// Linked list node
	INT16						m_Type ;					// Type of swoosh
	FLOAT						m_LifeTime ;			// Current time for swoosh

	CSwooshInfo				*m_pInfo ;				// Ptr to swoosh color info

	CGameObjectInstance	*m_pObject ;			// Ptr to object swoosh is on
	CParticle				*m_pParticle ;			// Ptr to particle swoosh is on

	CList						m_EdgeList ;			// Edge list
} CSwoosh ;


// Timer fx function
typedef struct CTimerFx_t
{
	CNode		m_Node ;							// Linked list node
	FLOAT		m_Time ;							// Timer
	FLOAT		m_Spacing ;						// Spacing between each function call
	INT32		m_Count ;						// Remaining number of calls
	CGameObjectInstance *m_pInst ;		// Instance origin
	void (*m_pFunction)(void *pThis) ;	// Function to call
} CTimerFx ;


// The big fx system structure
typedef struct CFxSystem_t
{
	Vtx				m_VertexList[2][MAX_FXSYSTEM_VERTICES] ;	// Vertex pool
	INT32				m_VtxsFree ;										// Total vertices free
	Vtx				*m_pFreeVertex ;  								// Current free vertex

	CSwooshEdge		m_SwooshEdgeVars[MAX_SWOOSH_EDGES] ;		// Swoosh edge data
	CList				m_SwooshEdgeList ;

	CSwoosh			m_SwooshVars[MAX_SWOOSHES] ;					// Swoosh data
	CDynamicList	m_SwooshList ;										// Swoosh list

	CTimerFx			m_TimerFxVars[MAX_TIMER_FX] ;					// Fx data
	CDynamicList	m_TimerFxList ;	 								// Timer fx list
} CFxSystem ;





/////////////////////////////////////////////////////////////////////////////
// CSpecialFx
/////////////////////////////////////////////////////////////////////////////

// Special fx structure
typedef struct CSpeacialFx_t
{
	CGameObjectInstance *m_pInst ;	// Owner
	INT32			m_Command ;				// Current command
	FLOAT			m_Time ;					// General timer
	FLOAT			*m_pScriptStart ;		// Start of script
	FLOAT			*m_pScript ;			// Current script position

	FLOAT			m_Trans ;				// Current trans
	FLOAT			m_StartTrans ;			// Trans before fade
	FLOAT			m_FinalTrans ;			// Trans to aim for
	FLOAT			m_TransFadeTime ;		// Trans fade time

	FLOAT			m_Color ;				// Current color
	FLOAT			m_StartColor ;			// Color before fade
	FLOAT			m_FinalColor ;			// Color to aim for
	FLOAT			m_ColorFadeTime ;		// Color fade time

	CRGB			m_EnvColor ;			// Current env. color
	CRGB			m_StartEnvColor ;		// Current env. color
	CRGB			m_FinalEnvColor ;		// Delta env. color
} CSpecialFx ;

enum SpecialFxCommands
{
	SPECIALFX_COMMAND_IDLE,

	SPECIALFX_COMMAND_WAIT,
	SPECIALFX_COMMAND_RANDOM_WAIT,

	SPECIALFX_COMMAND_SET_TRANS,
	SPECIALFX_COMMAND_FADE_TRANS,

	SPECIALFX_COMMAND_SET_COLOR,
	SPECIALFX_COMMAND_FADE_COLOR,

	SPECIALFX_COMMAND_PLAY_SOUND,

	SPECIALFX_COMMAND_REPEAT
} ;

#define	SPECIALFX_SCRIPT_WAIT(t)						SPECIALFX_COMMAND_WAIT,t,
#define	SPECIALFX_SCRIPT_RANDOM_WAIT(t1,t2)			SPECIALFX_COMMAND_RANDOM_WAIT,t1,t2,

#define	SPECIALFX_SCRIPT_SET_TRANS(v)					SPECIALFX_COMMAND_SET_TRANS,v,
#define	SPECIALFX_SCRIPT_FADE_TRANS(v, t)			SPECIALFX_COMMAND_FADE_TRANS,v,t,

#define	SPECIALFX_SCRIPT_SET_COLOR(r,g,b, v)		SPECIALFX_COMMAND_SET_COLOR,r,g,b,v,
#define	SPECIALFX_SCRIPT_FADE_COLOR(r,g,b, v,t)	SPECIALFX_COMMAND_FADE_COLOR,r,g,b,v,t,

#define	SPECIALFX_SCRIPT_PLAY_SOUND(s)				SPECIALFX_COMMAND_PLAY_SOUND,s,

#define	SPECIALFX_SCRIPT_REPEAT							SPECIALFX_COMMAND_REPEAT,


#define	DEFINE_SPECIALFX_SCRIPT(name)	\
FLOAT	name[] = 								\
{

#define	END_SPECIALFX_SCRIPT	\
	SPECIALFX_COMMAND_IDLE		\
} ;



/////////////////////////////////////////////////////////////////////////////
// Functions prototypes
/////////////////////////////////////////////////////////////////////////////
void CFxSystem__Construct(CFxSystem *pThis) ;

void CFxSystem__Construct(CFxSystem *pThis) ;
void CFxSystem__Update(CFxSystem *pThis) ;
Vtx *CFxSystem__AllocateVertexList(CFxSystem *pThis, INT32 NoOfVertices) ;

CSwooshEdge *CFxSystem__AllocateSwooshEdge(CFxSystem *pThis, CSwoosh *pSwoosh) ;
void CFxSystem__DeallocateSwooshEdge(CFxSystem *pThis, CSwoosh *pSwoosh, CSwooshEdge *pEdge) ;

void CFxSystem__AddEdgeToSwoosh(CFxSystem *pThis, CSwoosh *pSwoosh, Mtx *pmtDrawMtxs) ;

void CFxSystem__AddObjectSwoosh(CFxSystem *pThis, CGameObjectInstance *pObject, CSwooshInfo *pInfo) ;
void CFxSystem__AddParticleSwoosh(CFxSystem *pThis, CParticle *pParticle, CSwooshInfo *pInfo) ;


void CFxSystem__CheckForAddingObjectSwooshEdge(CFxSystem *pThis, CGameObjectInstance *pObject, Mtx *pmtDrawMtxs) ;
void CFxSystem__CheckForAddingParticleSwooshEdge(CFxSystem *pThis, CParticle *pParticle, Mtx *pmtDrawMtxs) ;

void CFxSystem__StopSwooshesOnObject(CFxSystem *pThis, CGameObjectInstance *pObject) ;
void CFxSystem__StopSwooshesOnParticle(CFxSystem *pThis, CParticle *pParticle) ;

void CFxSystem__Draw(CFxSystem *pThis, Gfx **ppDLP) ;



// Swoosh events
void AI_SEvent_SwooshAllHands(CInstanceHdr *pThis, CVector3 vPos, float Value) ;
void AI_SEvent_SwooshLeftHand(CInstanceHdr *pThis, CVector3 vPos, float Value) ;
void AI_SEvent_SwooshRightHand(CInstanceHdr *pThis, CVector3 vPos, float Value) ;
void AI_SEvent_SwooshAllFeet(CInstanceHdr *pThis, CVector3 vPos, float Value) ;
void AI_SEvent_SwooshLeftFoot(CInstanceHdr *pThis, CVector3 vPos, float Value) ;
void AI_SEvent_SwooshRightFoot(CInstanceHdr *pThis, CVector3 vPos, float Value) ;
void AI_SEvent_SwooshWeapon1(CInstanceHdr *pThis, CVector3 vPos, float Value) ;
void AI_SEvent_SwooshWeapon2(CInstanceHdr *pThis, CVector3 vPos, float Value) ;
void AI_SEvent_SwooshTail(CInstanceHdr *pThis, CVector3 vPos, float Value) ;
void AI_SEvent_SwooshLeftToes(CInstanceHdr *pThis, CVector3 vPos, float Value) ;
void AI_SEvent_SwooshRightToes(CInstanceHdr *pThis, CVector3 vPos, float Value) ;


// Special fx functions
void CSpecialFx__Construct(CSpecialFx *pThis, CGameObjectInstance *pInst) ;
void CSpecialFx__StartScript(CSpecialFx *pThis, FLOAT *pScript) ;
void CSpecialFx__Update(CSpecialFx *pThis) ;
void CSpecialFx__SetDrawFxVars(CSpecialFx *pThis) ;
void CSpecialFx__AddToDrawFxVars(CSpecialFx *pThis) ;
void RangeCheckFxVar(FLOAT *pVar) ;
void PrepareDrawFxVars(void) ;



// Timer fx functions
typedef void(*TimerFxFunction)(CTimerFx *pThis) ;

CTimerFx *CFxSystem__AddTimerFx(CFxSystem *pThis,
										  TimerFxFunction pFunction,	
										  CGameObjectInstance *pInst, 
										  INT32 Total, 
										  FLOAT Spacing) ;


/////////////////////////////////////////////////////////////////////////////
#endif // _INC_FX

