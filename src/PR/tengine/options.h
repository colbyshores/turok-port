// options.h
/////////////////////////////////////////////////////////////////////////////

#ifndef _INC_OPTIONS
#define _INC_OPTIONS

#include "tcontrol.h"
#include "loadsave.h"

/////////////////////////////////////////////////////////////////////////////
// Constants
/////////////////////////////////////////////////////////////////////////////

enum OptionSelections
{
	OPTIONS_MUSIC_VOLUME,
	OPTIONS_SFX_VOLUME,
	OPTIONS_OPACITY,
	OPTIONS_HANALOG,
	OPTIONS_VANALOG,
	OPTIONS_CONTROL,
#ifndef GERMAN
	OPTIONS_BLOOD,
#endif
	OPTIONS_EXIT,
	OPTIONS_END_SELECTION
} ;


#ifdef EDIT_FOG_AND_LIGHTS
enum FogSelections
{
	FOG_RED,
	FOG_GREEN,
	FOG_BLUE,
	FOG_END_SELECTION
} ;

enum LightSelections
{
	LIGHT_ARED,
	LIGHT_AGREEN,
	LIGHT_ABLUE,
	LIGHT_DRED,
	LIGHT_DGREEN,
	LIGHT_DBLUE,
	LIGHT_END_SELECTION
} ;
#endif


enum OptionsModes
{
	OPTIONS_NULL,
	OPTIONS_FADEUP,
	OPTIONS_NORMAL,
	OPTIONS_FADEDOWN
} ;


enum BloodStyles
{
	BLOOD_OFF,
	BLOOD_RED,
	BLOOD_GREEN,

	BLOOD_END
} ;




/////////////////////////////////////////////////////////////////////////////
// Options structure
/////////////////////////////////////////////////////////////////////////////

typedef struct COption_t
{
	// Mode vars
	INT32				m_Mode ;					// state of fade
	float				m_Alpha ;				// 0..1

	BOOL				b_Active ;
	BOOL				m_RHControl;
	BOOL				m_Blood;					// 0, 1 or 2

#ifdef EDIT_FOG_AND_LIGHTS
// temporary fog edit stuff
	BOOL				m_EditFog ;
	INT32				m_Red ;
	INT32				m_Green ;
	INT32				m_Blue ;
	BOOL				m_EditLight ;
	INT32				m_ARed ;
	INT32				m_AGreen ;
	INT32				m_ABlue ;
	INT32				m_DRed ;
	INT32				m_DGreen ;
	INT32				m_DBlue ;
#endif

	// Selection vars
	INT32				m_Selection ;		// Current selection

	// levels
	INT32				m_MusicVolume ;
	INT32				m_SFXVolume ;
	INT32				m_Opacity ;
	INT32				m_HAnalog ;
	INT32				m_VAnalog ;

	// Options storage (used during attract mode)
	BOOL				m_OptionsStored ;
	INT32				m_Storage_CheatFlags ;
	BOOL				m_Storage_RHControl ;
	BOOL				m_Storage_Blood ;
	INT32				m_Storage_Opacity ;
	INT32				m_Storage_HAnalog ;
	INT32				m_Storage_VAnalog ;
	INT32				m_Storage_Difficulty ;
} COptions ;


/////////////////////////////////////////////////////////////////////////////
// COptions operations
/////////////////////////////////////////////////////////////////////////////

void COptions__Store(COptions *pThis) ;
void COptions__Restore(COptions *pThis) ;
void COptions__SetAttractDefaults(COptions *pThis) ;

void COptions__SetDefaults(COptions *pThis) ;
void COptions__Construct(COptions *pThis) ;
INT32 COptions__Update(COptions *pThis) ;
void COptions__Draw(COptions *pThis, Gfx **ppDLP);





/////////////////////////////////////////////////////////////////////////////
// Keys structure
/////////////////////////////////////////////////////////////////////////////
enum KeySelections
{
	KEYS_EXIT,
	KEYS_END_SELECTION
} ;

enum KeysModes
{
	KEYS_NULL,
	KEYS_FADEUP,
	KEYS_NORMAL,
	KEYS_FADEDOWN
} ;



typedef struct CKeys_t
{
	// Mode vars
	INT32				m_Mode ;					// state of fade
	float				m_Alpha ;				// 0..1

	BOOL				b_Active ;

} CKeys ;


/////////////////////////////////////////////////////////////////////////////
// CKeys operations
/////////////////////////////////////////////////////////////////////////////

void CKeys__Construct(CKeys *pThis) ;
s32 CKeys__Update(CKeys *pThis) ;
void CKeys__Draw(CKeys *pThis, Gfx **ppDLP);


/////////////////////////////////////////////////////////////////////////////
#endif // _INC_OPTIONS
