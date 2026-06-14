// frontend.h

#ifndef _INC_FRONTEND
#define _INC_FRONTEND

#include "defs.h"


/////////////////////////////////////////////////////////////////////////////
// LEGALSCREEN
/////////////////////////////////////////////////////////////////////////////
typedef struct CLegalScreen_t
{
	u32		m_Mode ;
	float		m_Time ;
} CLegalScreen;

void CLegalScreen__Contruct(void) ;
void CLegalScreen__Update(void) ;
void CLegalScreen__Draw(Gfx **ppDLP) ;


/////////////////////////////////////////////////////////////////////////////
// CREDITS
/////////////////////////////////////////////////////////////////////////////
typedef	struct s_Credit
{
	float	xs, ys ;
	char	*string ;
} t_Credit ;


typedef struct CCredits_t
{
	t_Credit	*pData ;
	BOOL		Pause ;
	float		ScrollY ;
	float		OffsetY ;
	float		SizeY ;
} CCredits;

void CCredits__Contruct(void) ;
void CCredits__Update(void) ;
void CCredits__Draw(Gfx **ppDLP) ;



#if 0
/////////////////////////////////////////////////////////////////////////////
typedef struct CFrontEnd_t
{
	INT32		m_Mode;
} CFrontEnd;

extern	INT32		option_option ;
extern	INT32		option_music_volume ;
extern	INT32		option_sfx_volume ;
extern	INT32		option_onscreen_opacity ;


// CFrontEnd operations
/////////////////////////////////////////////////////////////////////////////

void		CFrontEnd__Construct(CFrontEnd *pThis);
BOOL		CFrontEnd__Update(CFrontEnd *pThis, BOOL Start);
#endif

/////////////////////////////////////////////////////////////////////////////
#endif // _INC_FRONTEND
