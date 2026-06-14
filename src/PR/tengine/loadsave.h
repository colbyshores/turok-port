// loadsave.h
/////////////////////////////////////////////////////////////////////////////

#ifndef _INC_LOADSAVE
#define _INC_LOADSAVE

#include "tcontrol.h"

/////////////////////////////////////////////////////////////////////////////
// Constants
/////////////////////////////////////////////////////////////////////////////
enum LoadMemorySelections
{
	LOADMEMORY_LOAD,
	LOADMEMORY_EXIT,
	LOADMEMORY_END_SELECTION
} ;
/*
enum LoadPasswordSelections
{
	LOADPASSWORD_GRID1,
	LOADPASSWORD_GRID2,
	LOADPASSWORD_GRID3,
	LOADPASSWORD_GRID4,
	LOADPASSWORD_DELETE,
	LOADPASSWORD_ENTER,
	LOADPASSWORD_EXIT,
	LOADPASSWORD_END_SELECTION
} ;
*/

enum LoadTypes
{
	LOAD_MEMORYCARD,
	LOAD_PASSWORD
} ;


enum SaveMemorySelections
{
	SAVEMEMORY_SAVE,
	SAVEMEMORY_DELETE,
	SAVEMEMORY_SHOWALL,
	SAVEMEMORY_EXIT,
	SAVEMEMORY_END_SELECTION
} ;
/*
enum SavePasswordSelections
{
	SAVEPASSWORD_EXIT,
	SAVEPASSWORD_END_SELECTION
} ;
*/
enum SaveTypes
{
	SAVE_MEMORYCARD,
	SAVE_PASSWORD
} ;



enum LoadModes
{
	LOAD_NULL,
	LOAD_FADEUP,
	LOAD_NORMAL,
	LOAD_FADEDOWN
} ;
enum SaveModes
{
	SAVE_NULL,
	SAVE_FADEUP,
	SAVE_NORMAL,
	SAVE_FADEDOWN
} ;


/////////////////////////////////////////////////////////////////////////////
// Load structures
/////////////////////////////////////////////////////////////////////////////
enum RequestorSelections
{
	REQUESTOR_YES,
	REQUESTOR_NO,
	REQUESTOR_END_SELECTION
} ;

enum RequestorModes
{
	REQUESTOR_NULL,
	REQUESTOR_FADEUP,
	REQUESTOR_NORMAL,
	REQUESTOR_FADEDOWN
} ;


typedef struct CRequestor_t
{
	float				m_Alpha ;				// 0..1
	INT16				m_Mode ;					// state of fade

	BOOL				b_Active ;

	// Selection vars
	INT32				m_Selection ;			// Current selection

	char				**Selections ;

	char				*Line1 ;
	char				*Line2 ;

} CRequestor ;


enum MessageBoxSelections
{
	MESSAGEBOX_OK,
	MESSAGEBOX_END_SELECTION
} ;

enum MessageBoxModes
{
	MESSAGEBOX_NULL,
	MESSAGEBOX_FADEUP,
	MESSAGEBOX_NORMAL,
	MESSAGEBOX_FADEDOWN
} ;

typedef struct CMessageBox_t
{
	float				m_Alpha ;				// 0..1
	INT16				m_Mode ;					// state of fade

	BOOL				b_Active ;

	// Selection vars
	INT32				m_Selection ;			// Current selection

	char				*Line1 ;
	char				*Line2 ;

} CMessageBox ;





typedef struct CLoad_t
{
	// Mode vars
	float				m_Alpha ;				// 0..1
	INT16				m_Mode ;					// state of fade

	BOOL				b_Active ;

	INT16				m_LoadType ;			// memory card or password

	// Selection vars
	s32				m_Side ;					// left or right column
	INT32				m_ASelection ;			// Current selection
	INT32				m_BSelection ;			// Current selection
	INT32				m_FirstVisible ;
	INT32				m_MaxVisible ;

	INT32				m_Selection ;			// Current selection
	INT32				m_SubSelection ;		// Selection within name grid or name entry

	INT32				m_EntryPos ;			// position in password buffer

} CLoad ;

typedef struct CSave_t
{
	// Mode vars
	float				m_Alpha ;				// 0..1
	INT16				m_Mode ;					// state of fade

	BOOL				b_Active ;

	INT16				m_SaveType ;			// memory card or password

	// Selection vars
	s32				m_Side ;					// left or right column
	INT32				m_ASelection ;			// Current selection
	INT32				m_BSelection ;			// Current selection
	INT32				m_FirstVisible ;
	INT32				m_MaxVisible ;

	BOOL				m_ShowAll ;				// show all file types

	INT32				m_Selection ;			// Current selection

} CSave ;





enum GamePakSelections
{
	GAMEPAK_SLOT1,
	GAMEPAK_SLOT2,
	GAMEPAK_SLOT3,
	GAMEPAK_SLOT4,
	GAMEPAK_SLOT5,
	GAMEPAK_SLOT6,
	GAMEPAK_SLOT7,
	GAMEPAK_SLOT8,
	GAMEPAK_SLOT9,
	GAMEPAK_SLOT10,
	GAMEPAK_SLOT11,
	GAMEPAK_SLOT12,
	GAMEPAK_SLOT13,
	GAMEPAK_SLOT14,
	GAMEPAK_SLOT15,
	GAMEPAK_SLOT16,
	GAMEPAK_END_SELECTION
} ;

enum GamePakModes
{
	GAMEPAK_NULL,
	GAMEPAK_FADEUP,
	GAMEPAK_NORMAL,
	GAMEPAK_FADEDOWN
} ;


typedef struct CGamePak_t
{
	// Mode vars
	float				m_Alpha ;				// 0..1
	INT16				m_Mode ;					// state of fade

	BOOL				b_Active ;

	// Selection vars
	INT32				m_Selection ;			// Current selection
	INT32				m_Timer ;

} CGamePak ;



/////////////////////////////////////////////////////////////////////////////
// CLoad & CSave operations
/////////////////////////////////////////////////////////////////////////////

void CLoad__Construct(CLoad *pThis) ;
INT32 CLoad__Update(CLoad *pThis) ;
void CLoad__Draw(CLoad *pThis, Gfx **ppDLP) ;

void CSave__Construct(CSave *pThis) ;
INT32 CSave__Update(CSave *pThis) ;
void CSave__Draw(CSave *pThis, Gfx **ppDLP) ;

void CGamePak__Construct(CGamePak *pThis) ;
INT32 CGamePak__Update(CGamePak *pThis) ;
void CGamePak__Draw(CGamePak *pThis, Gfx **ppDLP) ;


void CRequestor__Construct(CRequestor *pThis, char *String1, char *String2) ;
INT32 CRequestor__Update(CRequestor *pThis) ;
void CRequestor__Draw(CRequestor *pThis, Gfx **ppDLP) ;

void CMessageBox__Construct(CMessageBox *pThis, char *String1, char *String2) ;
INT32 CMessageBox__Update(CMessageBox *pThis) ;
void CMessageBox__Draw(CMessageBox *pThis, Gfx **ppDLP) ;



enum CheatCodeSelections
{
	CHEATCODE_GRID1,
	CHEATCODE_GRID2,
	CHEATCODE_GRID3,
	CHEATCODE_GRID4,
	CHEATCODE_DELETE,
	CHEATCODE_ENTER,
	CHEATCODE_EXIT,
	CHEATCODE_END_SELECTION
} ;

enum CheatCodeModes
{
	CHEATCODE_NULL,
	CHEATCODE_FADEUP,
	CHEATCODE_NORMAL,
	CHEATCODE_FADEDOWN
} ;

typedef struct CCheatCode_t
{
	// Mode vars
	float				m_Alpha ;				// 0..1
	INT16				m_Mode ;					// state of fade

	BOOL				b_Active ;

	// Selection vars
	INT32				m_Selection ;			// Current selection
	INT32				m_SubSelection ;		// Selection within name grid or name entry

	INT32				m_EntryPos ;			// position in password buffer
} CCheatCode ;


typedef struct CGiveCheatCode_t
{
	// Mode vars
	float				m_Alpha ;				// 0..1
	INT16				m_Mode ;					// state of fade

	BOOL				b_Active ;

	int				*m_Codes ;

	// Selection vars
	INT32				m_Selection ;			// Current selection
} CGiveCheatCode ;


void CCheatCode__Construct(CCheatCode *pThis) ;
INT32 CCheatCode__Update(CCheatCode *pThis) ;
void CCheatCode__Draw(CCheatCode *pThis, Gfx **ppDLP) ;

void CGiveCheatCode__Construct(CGiveCheatCode *pThis, int *code) ;
INT32 CGiveCheatCode__Update(CGiveCheatCode *pThis) ;
void CGiveCheatCode__Draw(CGiveCheatCode *pThis, Gfx **ppDLP) ;



/////////////////////////////////////////////////////////////////////////////
#endif // _INC_LOADSAVE
