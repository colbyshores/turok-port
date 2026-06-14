// onscrn.h
/////////////////////////////////////////////////////////////////////////////

#ifndef _INC_ONSCRN
#define _INC_ONSCRN

/////////////////////////////////////////////////////////////////////////////
// Onscreen defines
/////////////////////////////////////////////////////////////////////////////
#define	AIR_TIMEIN		10
#define	AIR_TIMEOUT		10

#define	AIRMODE_NULL			0
#define	AIRMODE_FADEON			1
#define	AIRMODE_WAIT			2
#define	AIRMODE_FADEOUT		3

#define	BOSSBAR_TIMEIN			10
#define	BOSSBAR_TIMEOUT		10

#define	BOSSBARMODE_NULL		0
#define	BOSSBARMODE_FADEON	1
#define	BOSSBARMODE_WAIT		2
#define	BOSSBARMODE_FADEOUT	3


#define	LARGE_FONT				0
#define	SMALL_FONT				1
#define	NINTENDO_FONT			2
#define	KANJI_FONT				3

/////////////////////////////////////////////////////////////////////////////
// Overlay structures
/////////////////////////////////////////////////////////////////////////////

// Overlay position structure
typedef struct COverlay_t
{
	BOOL	m_OnScreen ;	// On screen or not

	INT32	*m_pScript,		// Pointer to movement script
			m_Mode,			// Current mode
			m_Time ;			// General timer

	FLOAT	m_X,				// Current screen position
			m_Y,

			m_FinalX,		// Final screen position
			m_FinalY,

			m_XVel,			// Movement velocities
			m_YVel,

			m_XAccel,		// Movement accelerations
			m_YAccel,

			m_ZoomFactor ;	// Zoom factor speed
} COverlay ;


// Overlay movement modes
enum OverlayModes
{
	OVERLAY_INACTIVE_MODE,
	OVERLAY_READ_SCRIPT_MODE,
	OVERLAY_WAIT_MODE,
	OVERLAY_ZOOM_MODE,
	OVERLAY_DROP_OFF_MODE,
	OVERLAY_HORZDROP_OFF_MODE
} ;

// Overlay script commands
enum OverlayScriptCommands
{
	OVERLAY_SCRIPT_SET_POS = 1,
	OVERLAY_SCRIPT_WAIT,
	OVERLAY_SCRIPT_ZOOM,
	OVERLAY_SCRIPT_DROP_OFF,
	OVERLAY_SCRIPT_HORZDROP_OFF
} ;


// Overlay script define macros
#define DEFINE_OVERLAY_SCRIPT(Name)	\
INT32 Name[] =								\
{

#define END_OVERLAY_SCRIPT				\
	0											\
} ;


// Overlay script entry macros
#define OVERLAY_SCRIPT_ENTRY_SET_POS(x,y)				\
	OVERLAY_SCRIPT_SET_POS,x,y,

#define OVERLAY_SCRIPT_ENTRY_WAIT(frames)				\
	OVERLAY_SCRIPT_WAIT,frames,

#define OVERLAY_SCRIPT_ENTRY_ZOOM(x,y,zoomfactor)	\
	OVERLAY_SCRIPT_ZOOM,x,y,zoomfactor,

#define OVERLAY_SCRIPT_ENTRY_DROP_OFF(gravity)		\
	OVERLAY_SCRIPT_DROP_OFF,gravity*65536,

#define OVERLAY_SCRIPT_ENTRY_HORZDROP_OFF(gravity)	\
	OVERLAY_SCRIPT_HORZDROP_OFF,gravity*65536,


/////////////////////////////////////////////////////////////////////////////
// COverlay operations
/////////////////////////////////////////////////////////////////////////////

void COverlay__Construct(COverlay *pThis, INT16 nScreenX, INT16 nScreenY) ;
void COverlay__RunScript(COverlay *pThis, INT32 *pScript) ;
void COverlay__Update(COverlay *pThis) ;
#define COverlay__Active(pThis)		((pThis)->m_Mode == OVERLAY_INACTIVE_MODE)
#define COverlay__OnScreen(pThis)	((pThis)->m_OnScreen)


/////////////////////////////////////////////////////////////////////////////
// Onscreen structures
/////////////////////////////////////////////////////////////////////////////

// For displaying grid graphics
typedef struct CGridGraphic_t
{
	UINT16	m_BlocksAcross ;
	UINT16	m_BlocksDown ;
	UINT16	m_BlockWidth ;		// Width in pixels
	UINT16	m_BlockHeight ;	// Height in pixels
	UINT8		m_pData[1] ;  		// Graphic data
} CGridGraphic ;



typedef struct s_C16BitPart
{
	UINT32	m_BlockWidth ;		// Width in pixels
	UINT32	m_BlockHeight ;	// Height in pixels
	UINT8		m_pData[1] ;  		// Graphic data
} C16BitPart ;


typedef struct C16BitGraphic_t
{
	UINT16	m_BlocksAcross ;
	UINT16	m_BlocksDown ;
	UINT16	m_Width ;
	UINT16	m_Height ;
	UINT8		m_pPart[1] ;  		// Graphic data + Opacity data
} C16BitGraphic ;




// onscreen text structure
typedef struct s_GameText
{
	FLOAT	Time ;
	FLOAT	DisplayTime ;
	int	Mode ;
	int	Alpha ;
	float	Scale ;
	char	*String ;
	int	Id ;
} t_GameText ;



// Special icon constants (0 = None)
enum SpecialIcons
{
	ONSCRN_RUN_ICON = 1,
	ONSCRN_WALK_ICON,

	ONSCRN_HEALTH_ICON,
	ONSCRN_ARMOUR_ICON
} ;


// The big all in one on screen display structure
typedef struct COnScreen_t
{
	INT16		m_SelectionTimer ; 	// Selection timer

	BOOL		m_ExplosiveAmmo ;		// Is current ammo explosive (draw different icon)

	int		m_BarrelDirection ;

	int		m_AirMode ;
	float		m_AirOpacity;

	int		m_BossBarMode ;
	float		m_BossBarOpacity;

	float		m_FlashTimer;
	char		m_FlashDraw;

	int		m_TopR,
				m_TopG,
				m_TopB,
				m_BottomR,
				m_BottomG,
				m_BottomB ;
	float		m_FontXScale,
				m_FontYScale ;
	int		m_ShadowXOff,
				m_ShadowYOff ;

	BOOL		m_HealthAlert,
				m_ArmorAlert ;

	// Amounts
	INT32		m_Ammo,					// Current ammo total
				m_Health,				// Current health (or armour)
				m_Armor,
				m_HealthArmourIcon, 	// Icon displaying
				m_Lives,					// onscreen lives
				m_Tokens,	  			// onscreen tokens

				m_WalkRunIcon;			//	Icon type

	float		m_CompassPos,
				m_CompassAccel ;		// acceleration for simply harmonic motion

	// Overlay positions
	COverlay	m_AmmoOverlay,			// Overlay position vars
				m_HealthOverlay,
				m_LivesOverlay,
				m_TokensOverlay,
				m_WeaponOverlay,
				m_RunWalkOverlay;

	// Boss stuff
	struct CBoss_t	*m_pBoss ;

	u8			m_SelectPosition ;	// Weapon selection position
} COnScreen ;




/////////////////////////////////////////////////////////////////////////////
// COnScreen operations
/////////////////////////////////////////////////////////////////////////////

void		COnScreen__Construct(COnScreen *pThis);
void		COnScreen__Reset(COnScreen *pThis);
void		COnScreen__Update(COnScreen *pThis) ;
void		COnScreen__Draw(COnScreen *pThis, Gfx **ppDLP);
void		COnScreen__DrawWeaponSelection(COnScreen *pThis, Gfx **ppDLP) ;
void		COnScreen__SetHealth(COnScreen *pThis, int nHealth);
void		COnScreen__SetLives(COnScreen *pThis, int nLives);
void		COnScreen__SetTokens(COnScreen *pThis, int nTokens) ;

void		COnScreen__SetBossBarMode(COnScreen *pThis, int mode) ;
BOOL COnScreen__BossHasBar(COnScreen *pThis, struct CBoss_t *pBoss) ;
void COnScreen__AddBossBar(COnScreen *pThis, struct CBoss_t *pBoss) ;


void		COnScreen__SetAirMode(COnScreen *pThis, int mode) ;
void		COnScreen__SetAmmo(COnScreen *pThis, int nAmmo, BOOL explosive);
void		COnScreen__SetWalkRunIcon(COnScreen *pThis, INT32 nIcon) ;
void		COnScreen__DisplayWeaponChoice(COnScreen *pThis, int nSelectPosition, BOOL Direction);
void		COnScreen__QuickWeaponChange(COnScreen *pThis, int SelectPosition);
BOOL		COnScreen__IsWeaponOverlayOn(COnScreen *pThis);
void		COnScreen__DrawParticleLensFlares(COnScreen *pThis, Gfx **ppDLP);
void		COnScreen__DrawLensFlare(COnScreen *pThis, Gfx **ppDLP, float sx, float sy, float Opacity, BOOL Sun, BOOL Blocked);
void		COnScreen__DrawSunLensFlare(COnScreen *pThis, Gfx **ppDLP);
//#define	COnScreen__ResetWeaponSelection(pThis)	((pThis)->m_SelectPosition=0)
void		COnScreen__DrawSun(Gfx **ppDL);



/////////////////////////////////////////////////////////////////////////////
// Texture draw functions
/////////////////////////////////////////////////////////////////////////////
//void COnScreen__InitTextureDraw(Gfx **ppDLP, int Opacity) ;
//
//void COnScreen__DrawTexture(Gfx **ppDLP,
//									 void *pTexture,
//									 int XPos, int YPos,
//									 int TextureWidth, int TextureHeight) ;
//void COnScreen__DrawScaledTexture(Gfx **ppDLP,
//											 void *pTexture,
//											float XPos, float YPos,
//											int TextureWidth, int TextureHeight,
//											float XScale, float YScale) ;


/////////////////////////////////////////////////////////////////////////////
// Box draw functions
/////////////////////////////////////////////////////////////////////////////

void COnScreen__InitBoxDraw(Gfx **ppDLP) ;

void COnScreen__DrawBox(Gfx **ppDLP, INT32 X1, INT32 Y1, INT32 X2, INT32 Y2,
								UINT8 Red, UINT8 Green, UINT8 Blue, UINT8 Alpha) ;

void COnScreen__DrawBoxOutLine(Gfx **ppDLP, INT32 X1, INT32 Y1, INT32 X2, INT32 Y2, INT32 Thickness,
										 UINT8 Red, UINT8 Green, UINT8 Blue, UINT8 Alpha) ;
void COnScreen__DrawHilightBox(Gfx **ppDLP, INT32 x1, INT32 y1, INT32 x2, INT32 y2, INT32 thick, BOOL invert,
										 UINT8 red, UINT8 green, UINT8 blue, UINT8 alpha) ;


/////////////////////////////////////////////////////////////////////////////
// Misc draw functions
/////////////////////////////////////////////////////////////////////////////


//void COnScreen__DrawGridGraphic(Gfx **ppDLP, CGridGraphic *Grid, INT16 XPos, INT16 YPos)  ;
//void COnScreen__DrawScaledGridGraphic(Gfx **ppDLP, CGridGraphic *Grid, float XPos, float YPos, float XScale, float YScale)  ;

void COnScreen__DrawDigits(Gfx **ppDLP,
									int nNumber,
									int nX,
									int nY) ;

void COnScreen__DrawText(Gfx **ppDLP,
								 char *String,
								 int nX, int nY,
								 int Opacity, BOOL centre, BOOL shadow) ;
void COnScreen__SetFontColor(Gfx **ppDLP, int tr, int tg, int tb, int br, int bg, int bb) ;
void COnScreen__SetFontScale(float XScale, float YScale) ;
void COnScreen__PrepareLargeFont(Gfx **ppDLP) ;
void COnScreen__PrepareSmallFont(Gfx **ppDLP) ;
void COnScreen__PrepareNintendoFont(Gfx **ppDLP) ;

void COnScreen__Init16BitDraw(Gfx **ppDLP, int opacity) ;
void COnScreen__Draw16BitTexture(Gfx **ppDLP,
										void *pTexture,
										void *pOpacity,
										int XPos, int YPos,
										int TextureWidth, int TextureHeight) ;
void COnScreen__Draw16BitScaledTexture(Gfx **ppDLP,
											void *pTexture,
											void *pOpacity,
											float XPos, float YPos,
											int TextureWidth, int TextureHeight,
											float XScale, float YScale) ;


void COnScreen__Draw16BitGraphic(Gfx **ppDLP, C16BitGraphic *Graphic, INT16 X, INT16 Y) ;
void COnScreen__Draw16BitScaledGraphic(Gfx **ppDLP, C16BitGraphic *Graphic, float X, float Y, float XScale, float YScale) ;



//void COnScreen__DrawFrameBufferLine(UINT16 *pBuffer, INT16 x1, INT16 y1, INT16 x2, INT16 y2, UINT16 Color) ;
//void COnScreen__DrawCompass(UINT16 *pBuffer, COnScreen *pThis) ;


/////////////////////////////////////////////////////////////////////////////
// game text functions
/////////////////////////////////////////////////////////////////////////////
enum GameTextModes
{
	GAMETEXT_NULL,
	GAMETEXT_FADEUP,
	GAMETEXT_DISPLAY,
	GAMETEXT_FADEDOWN
} ;


void COnScreen__InitializeGameText(void) ;
t_GameText *COnScreen__AddGameText(char *String) ;
t_GameText *COnScreen__AddGameTextForTime(char *String, FLOAT Secs) ;
t_GameText *COnScreen__AddGameTextWithId(char *String, int id) ;
BOOL COnScreen__SearchForId(int id) ;
void COnScreen__UpdateGameText(Gfx **ppDLP) ;


/////////////////////////////////////////////////////////////////////////////
// Barrel structures
/////////////////////////////////////////////////////////////////////////////
enum BARREL_MODES
{
	BARREL_UP,
	BARREL_DOWN
} ;

typedef struct s_CBarrel
{
	int	m_Current ;				// internally selected item
	int	m_Selected ;			// user chosen item
	float	m_CurrentAngle ;		// angle of internally chosen item
	float	m_SelectedAngle ;		// angle of chosen item to move to

	float	m_delta ;				// angle difference
	float	m_AngleAccel ;			// acceleration
	float	m_ViewAngleOffset ;	// angle to add to sin position of drawn object to make it spin

	int	m_Display ;
	float	m_DisplayStep ;

	float	m_SpinSpeed ;			// amount to divide delta by for speed

	int	m_Buffersize ;			// items in buffer
	int	m_X,
			m_Y,
			m_XStep,
			m_YStep ;
	int	m_Direction ;

	void(*InitGraphics)(Gfx **ppDLP) ;
	void(*DrawItem)(Gfx **ppDLP, int index, float x, float y, float xscale, float yscale, int alpha) ;

} CBarrel ;

void CBarrel__Construct(CBarrel *pThis, int items,
								int x, int y, int xstep, int ystep,
								float speed, int display) ;
void CBarrel__Update(CBarrel *pThis) ;
void CBarrel__Draw(CBarrel *pThis, Gfx **ppDLP) ;


/////////////////////////////////////////////////////////////////////////////
#endif // _INC_ONSCRN
