// cheat.h
/////////////////////////////////////////////////////////////////////////////

#ifndef _INC_CHEAT
#define _INC_CHEAT

#include "tcontrol.h"

/////////////////////////////////////////////////////////////////////////////
// Constants
/////////////////////////////////////////////////////////////////////////////

//
// WARNING !!!!!!!!!!!
// flags must be in same order as cheat selections below
//
//#define	CHEATFLAG_SHOWALLENEMIES	(1<<6)
//#define	CHEATFLAG_BIGENEMYMODE		(1<<8)
//#define	CHEATFLAG_MODIFYENEMY		(1<<25)
#ifndef KANJI
#define	CHEATFLAG_INVINCIBILITY		(1<<0)
#define	CHEATFLAG_SPIRITMODE			(1<<1)
#define	CHEATFLAG_ALLWEAPONS			(1<<2)
#define	CHEATFLAG_UNLIMITEDAMMO		(1<<3)
#define	CHEATFLAG_INFINITELIVES		(1<<4)
#define	CHEATFLAG_ALLKEYS				(1<<5)
#define	CHEATFLAG_ALLMAP				(1<<6)
#define	CHEATFLAG_BIGHEADS			(1<<7)
#define	CHEATFLAG_TINYENEMYMODE		(1<<8)
#define	CHEATFLAG_PENANDINKMODE		(1<<9)
#define	CHEATFLAG_PURDYCOLORS		(1<<10)
#define	CHEATFLAG_DISCO				(1<<11)
#define	CHEATFLAG_QUACKMODE			(1<<12)
#define	CHEATFLAG_WARPLEVEL1			(1<<13)
#define	CHEATFLAG_WARPLEVEL2			(1<<14)
#define	CHEATFLAG_WARPLEVEL3			(1<<15)
#define	CHEATFLAG_WARPLEVEL4			(1<<16)
#define	CHEATFLAG_WARPLEVEL5			(1<<17)
#define	CHEATFLAG_WARPLEVEL6			(1<<18)
#define	CHEATFLAG_WARPLEVEL7			(1<<19)
#define	CHEATFLAG_WARPLEVEL8			(1<<20)
#define	CHEATFLAG_WARPLONGHUNTER	(1<<21)
#define	CHEATFLAG_WARPMANTIS			(1<<22)
#define	CHEATFLAG_WARPTREX			(1<<23)
#define	CHEATFLAG_WARPCAMPAIGNER	(1<<24)
#define	CHEATFLAG_GALLERY				(1<<25)
#define	CHEATFLAG_SHOWCREDITS		(1<<26)
#define	CHEATFLAG_FLYMODE				(1<<27)
#else	// KANJI mode has no DISCO...
#define	CHEATFLAG_INVINCIBILITY		(1<<0)
#define	CHEATFLAG_SPIRITMODE			(1<<1)
#define	CHEATFLAG_ALLWEAPONS			(1<<2)
#define	CHEATFLAG_UNLIMITEDAMMO		(1<<3)
#define	CHEATFLAG_INFINITELIVES		(1<<4)
#define	CHEATFLAG_ALLKEYS				(1<<5)
#define	CHEATFLAG_ALLMAP				(1<<6)
#define	CHEATFLAG_BIGHEADS			(1<<7)
#define	CHEATFLAG_TINYENEMYMODE		(1<<8)
#define	CHEATFLAG_PENANDINKMODE		(1<<9)
#define	CHEATFLAG_PURDYCOLORS		(1<<10)
#define	CHEATFLAG_QUACKMODE			(1<<11)
#define	CHEATFLAG_WARPLEVEL1			(1<<12)
#define	CHEATFLAG_WARPLEVEL2			(1<<13)
#define	CHEATFLAG_WARPLEVEL3			(1<<14)
#define	CHEATFLAG_WARPLEVEL4			(1<<15)
#define	CHEATFLAG_WARPLEVEL5			(1<<16)
#define	CHEATFLAG_WARPLEVEL6			(1<<17)
#define	CHEATFLAG_WARPLEVEL7			(1<<18)
#define	CHEATFLAG_WARPLEVEL8			(1<<19)
#define	CHEATFLAG_WARPLONGHUNTER	(1<<20)
#define	CHEATFLAG_WARPMANTIS			(1<<21)
#define	CHEATFLAG_WARPTREX			(1<<22)
#define	CHEATFLAG_WARPCAMPAIGNER	(1<<23)
#define	CHEATFLAG_GALLERY				(1<<24)
#define	CHEATFLAG_SHOWCREDITS		(1<<25)
#define	CHEATFLAG_FLYMODE				(1<<26)
#endif

enum CheatSelections
{
	CHEAT_INVINCIBILITY,
	CHEAT_SPIRITMODE,
	CHEAT_ALLWEAPONS,
	CHEAT_UNLIMITEDAMMO,
	CHEAT_INFINITELIVES,
	CHEAT_ALLKEYS,
	CHEAT_ALLMAP,
	CHEAT_BIGHEADS,
//	CHEAT_SHOWALLENEMIES,
	CHEAT_TINYENEMYMODE,
//	CHEAT_BIGENEMYMODE,
	CHEAT_PENANDINKMODE,
	CHEAT_PURDYCOLORS,
#ifndef KANJI
	CHEAT_DISCO,
#endif
	CHEAT_QUACKMODE,
	CHEAT_WARPLEVEL1,
	CHEAT_WARPLEVEL2,
	CHEAT_WARPLEVEL3,
	CHEAT_WARPLEVEL4,
	CHEAT_WARPLEVEL5,
	CHEAT_WARPLEVEL6,
	CHEAT_WARPLEVEL7,
	CHEAT_WARPLEVEL8,
	CHEAT_WARPLONGHUNTER,
	CHEAT_WARPMANTIS,
	CHEAT_WARPTREX,
	CHEAT_WARPCAMPAIGNER,
	CHEAT_GALLERY,
	CHEAT_SHOWCREDITS,
//	CHEAT_MODIFYENEMY,
	CHEAT_FLYMODE,
	CHEAT_EXIT,
	CHEAT_END_SELECTION
} ;

// WARNING !!!!!
//

enum CheatFadeModes
{
	CHEAT_NULL,
	CHEAT_FADEUP,
	CHEAT_NORMAL,
	CHEAT_FADEDOWN
} ;



enum GInstSelections
{
	GINST_EXIT,
	GINST_END_SELECTION
} ;

enum GInstFadeModes
{
	GINST_NULL,
	GINST_FADEUP,
	GINST_NORMAL,
	GINST_FADEDOWN
} ;



/////////////////////////////////////////////////////////////////////////////
// Cheat structures
/////////////////////////////////////////////////////////////////////////////

// Overlay position structure
typedef struct CCheat_t
{
	// Mode vars
	INT32				m_Mode ;					// state of fade
	float				m_Alpha ;				// 0..1

	BOOL				b_Active ;

	// Selection vars
	INT32				m_Selection ;			// Current selection
	int				m_RealSelection ;		// Current selection

} CCheat ;


typedef struct CGInst_t
{
	// Mode vars
	INT32				m_Mode ;					// state of fade
	float				m_Alpha ;				// 0..1

	BOOL				b_Active ;

	// Selection vars
	INT32				m_Selection ;			// Current selection

} CGInst ;



/////////////////////////////////////////////////////////////////////////////
// CCheat operations
/////////////////////////////////////////////////////////////////////////////

void CCheat__Construct(CCheat *pThis) ;
INT32 CCheat__Update(CCheat *pThis) ;
void CCheat__Draw(CCheat *pThis, Gfx **ppDLP) ;

void CGInst__Construct(CGInst *pThis) ;
INT32 CGInst__Update(CGInst *pThis) ;
void CGInst__Draw(CGInst *pThis, Gfx **ppDLP) ;




/////////////////////////////////////////////////////////////////////////////
#endif // _INC_CHEAT
