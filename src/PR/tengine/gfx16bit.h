// gfx16bit.h

#ifndef _INC_GFX16BIT
#define _INC_GFX16BIT

#include "cppu64.h"
#include "tengine.h"

// numerals 0-9
extern	UINT8	Font0[] ;
extern	UINT8	Font1[] ;
extern	UINT8	Font2[] ;
extern	UINT8	Font3[] ;
extern	UINT8	Font4[] ;
extern	UINT8	Font5[] ;
extern	UINT8	Font6[] ;
extern	UINT8	Font7[] ;
extern	UINT8	Font8[] ;
extern	UINT8	Font9[] ;
extern	UINT8	Font0_4[] ;
extern	UINT8	Font1_4[] ;
extern	UINT8	Font2_4[] ;
extern	UINT8	Font3_4[] ;
extern	UINT8	Font4_4[] ;
extern	UINT8	Font5_4[] ;
extern	UINT8	Font6_4[] ;
extern	UINT8	Font7_4[] ;
extern	UINT8	Font8_4[] ;
extern	UINT8	Font9_4[] ;
extern	UINT8	FontMinus[] ;

// actual font
extern	UINT8	FontA[] ;
extern	UINT8	FontB[] ;
extern	UINT8	FontC[] ;
extern	UINT8	FontD[] ;
extern	UINT8	FontE[] ;
extern	UINT8	FontF[] ;
extern	UINT8	FontG[] ;
extern	UINT8	FontH[] ;
extern	UINT8	FontI[] ;
extern	UINT8	FontJ[] ;
extern	UINT8	FontK[] ;
extern	UINT8	FontL[] ;
extern	UINT8	FontM[] ;
extern	UINT8	FontN[] ;
extern	UINT8	FontO[] ;
extern	UINT8	FontP[] ;
extern	UINT8	FontQ[] ;
extern	UINT8	FontR[] ;
extern	UINT8	FontS[] ;
extern	UINT8	FontT[] ;
extern	UINT8	FontU[] ;
extern	UINT8	FontV[] ;
extern	UINT8	FontW[] ;
extern	UINT8	FontX[] ;
extern	UINT8	FontY[] ;
extern	UINT8	FontZ[] ;
extern	UINT8	FontCopyRight[] ;
extern	UINT8	FontLittleC[] ;
extern	UINT8	FontPeriod[] ;
extern	UINT8	FontComma[] ;
extern	UINT8	FontQuoteL[] ;
extern	UINT8	FontQuoteR[] ;
extern	UINT8	FontTrademark[] ;
extern	UINT8	FontRamp[] ;
#ifdef GERMAN
extern	UINT8	FontGermanA[] ;
extern	UINT8	FontGermanO[] ;
extern	UINT8	FontGermanU[] ;
#endif

extern	UINT8	SmallFont[] ;

// health
extern	UINT8	HealthOverlay[] ;
extern	UINT8	ArmourOverlay[] ;
extern	UINT8	TurokHeadOverlay[] ;
extern	UINT8	LifeForceOverlay[] ;
extern	UINT8	AirBarOverlay[] ;
extern	UINT8	BossBarOverlay[] ;
extern	UINT8	GradientOverlay[] ;
extern	UINT8	BarGradient[] ;
extern	UINT8	LensFlareOverlay[] ;
extern	UINT8	SunFlareOverlay[] ;

// inventory stuff
extern	UINT8	Key2Overlay[] ;
extern	UINT8	Key3Overlay[] ;
extern	UINT8	Key4Overlay[] ;
extern	UINT8	Key5Overlay[] ;
extern	UINT8	Key6Overlay[] ;
extern	UINT8	Key7Overlay[] ;
extern	UINT8	Key8Overlay[] ;
extern	UINT8	ChronoOverlay[] ;


// walk - run icons
extern	UINT8	WalkOverlay[] ;
extern	UINT8	RunOverlay[] ;

// boxes
extern	UINT8	RokOverlay[] ;

// compass
//extern	UINT8	NESWOverlay[] ;

// ammo
extern	UINT8	AmmoArrowOverlay[] ;
extern	UINT8	AmmoExpArrowOverlay[] ;
extern	UINT8	AmmoBulletOverlay[] ;
extern	UINT8	AmmoShellOverlay[] ;
extern	UINT8	AmmoExpShellOverlay[] ;
extern	UINT8	AmmoEnergyOverlay[] ;
extern	UINT8	AmmoArrowOverlay[] ;
extern	UINT8	AmmoMinigunOverlay[] ;
extern	UINT8	AmmoGrenadeOverlay[] ;
extern	UINT8	AmmoRocketOverlay[] ;
//extern	UINT8	AmmoShockwaveOverlay[] ;
extern	UINT8	AmmoKamfOverlay[] ;

// weapons
extern	UINT8	KnifeWeapon[] ;
extern	UINT8	TekBowWeapon[] ;
extern	UINT8	PistolWeapon[] ;
extern	UINT8	RifleWeapon[] ;
extern	UINT8	ShotgunWeapon[] ;
extern	UINT8	AutoShotgunWeapon[] ;
extern	UINT8	GrenadeWeapon[] ;
extern	UINT8	RocketWeapon[] ;
extern	UINT8	MinigunWeapon[] ;
extern	UINT8	LongHunterWeapon[] ;
extern	UINT8	InfantryWeapon[] ;
extern	UINT8	ShockWeapon[] ;
extern	UINT8	FusionWeapon[] ;
extern	UINT8	CronosceptorWeapon[] ;

// pause
extern	UINT8	SlabOverlay[] ;
extern	UINT8	PauseOverlay[] ;
extern	UINT8	GameOverOverlay[] ;
extern	UINT8	OptionsOverlay[] ;
extern	UINT8	SliderOverlay[] ;
extern	UINT8	BarOverlay[] ;

#ifdef KANJI
extern	UINT8	Kanji00[] ;
extern	UINT8	Kanji01[] ;
extern	UINT8	Kanji02[] ;
extern	UINT8	Kanji03[] ;
extern	UINT8	Kanji04[] ;
extern	UINT8	Kanji05[] ;
extern	UINT8	Kanji06[] ;
extern	UINT8	Kanji07[] ;
extern	UINT8	Kanji08[] ;
extern	UINT8	Kanji09[] ;
extern	UINT8	Kanji0a[] ;
extern	UINT8	Kanji0b[] ;
extern	UINT8	Kanji0c[] ;
extern	UINT8	Kanji0d[] ;
extern	UINT8	Kanji0e[] ;
extern	UINT8	Kanji0f[] ;
extern	UINT8	Kanji10[] ;
extern	UINT8	Kanji11[] ;
extern	UINT8	Kanji12[] ;
extern	UINT8	Kanji13[] ;
extern	UINT8	Kanji14[] ;
extern	UINT8	Kanji15[] ;
extern	UINT8	Kanji16[] ;
extern	UINT8	Kanji17[] ;
extern	UINT8	Kanji18[] ;
extern	UINT8	Kanji19[] ;
extern	UINT8	Kanji1a[] ;
extern	UINT8	Kanji1b[] ;
extern	UINT8	Kanji1c[] ;
extern	UINT8	Kanji1d[] ;
extern	UINT8	Kanji1e[] ;
extern	UINT8	Kanji1f[] ;
extern	UINT8	Kanji20[] ;
extern	UINT8	Kanji21[] ;
extern	UINT8	Kanji22[] ;
extern	UINT8	Kanji23[] ;
extern	UINT8	Kanji24[] ;
extern	UINT8	Kanji25[] ;
extern	UINT8	Kanji26[] ;
extern	UINT8	Kanji27[] ;
extern	UINT8	Kanji28[] ;
extern	UINT8	Kanji29[] ;
extern	UINT8	Kanji2a[] ;
extern	UINT8	Kanji2b[] ;
extern	UINT8	Kanji2c[] ;
extern	UINT8	Kanji2d[] ;
extern	UINT8	Kanji2e[] ;
extern	UINT8	Kanji2f[] ;
extern	UINT8	Kanji30[] ;
extern	UINT8	Kanji31[] ;
extern	UINT8	Kanji32[] ;
extern	UINT8	Kanji33[] ;
extern	UINT8	Kanji34[] ;
extern	UINT8	Kanji35[] ;
extern	UINT8	Kanji36[] ;
extern	UINT8	Kanji37[] ;
extern	UINT8	Kanji38[] ;
extern	UINT8	Kanji39[] ;
extern	UINT8	Kanji3a[] ;
extern	UINT8	Kanji3b[] ;
extern	UINT8	Kanji3c[] ;
extern	UINT8	Kanji3d[] ;
extern	UINT8	Kanji3e[] ;
extern	UINT8	Kanji3f[] ;
extern	UINT8	Kanji40[] ;
extern	UINT8	Kanji41[] ;
extern	UINT8	Kanji42[] ;
extern	UINT8	Kanji43[] ;
extern	UINT8	Kanji44[] ;
extern	UINT8	Kanji45[] ;
extern	UINT8	Kanji46[] ;
extern	UINT8	Kanji47[] ;
extern	UINT8	Kanji48[] ;
extern	UINT8	Kanji49[] ;
extern	UINT8	Kanji4a[] ;
extern	UINT8	Kanji4b[] ;
extern	UINT8	Kanji4c[] ;
extern	UINT8	Kanji4d[] ;
extern	UINT8	Kanji4e[] ;
extern	UINT8	Kanji4f[] ;
extern	UINT8	Kanji50[] ;
extern	UINT8	Kanji51[] ;
extern	UINT8	Kanji52[] ;
extern	UINT8	Kanji53[] ;
extern	UINT8	Kanji54[] ;
extern	UINT8	Kanji55[] ;
extern	UINT8	Kanji56[] ;
extern	UINT8	Kanji57[] ;
extern	UINT8	Kanji58[] ;
extern	UINT8	Kanji59[] ;
extern	UINT8	Kanji5a[] ;
extern	UINT8	Kanji5b[] ;
extern	UINT8	Kanji5c[] ;
extern	UINT8	Kanji5d[] ;
extern	UINT8	Kanji5e[] ;
extern	UINT8	Kanji5f[] ;
extern	UINT8	Kanji60[] ;
extern	UINT8	Kanji61[] ;
extern	UINT8	Kanji62[] ;
extern	UINT8	Kanji63[] ;
extern	UINT8	Kanji64[] ;
extern	UINT8	Kanji65[] ;
extern	UINT8	Kanji66[] ;
extern	UINT8	Kanji67[] ;
extern	UINT8	Kanji68[] ;
extern	UINT8	Kanji69[] ;
extern	UINT8	Kanji6a[] ;
extern	UINT8	Kanji6b[] ;
extern	UINT8	Kanji6c[] ;
extern	UINT8	Kanji6d[] ;
extern	UINT8	Kanji6e[] ;
extern	UINT8	Kanji6f[] ;
extern	UINT8	Kanji70[] ;
extern	UINT8	Kanji71[] ;
extern	UINT8	Kanji72[] ;
extern	UINT8	Kanji73[] ;
extern	UINT8	Kanji74[] ;
extern	UINT8	Kanji75[] ;
extern	UINT8	Kanji76[] ;
extern	UINT8	Kanji77[] ;
extern	UINT8	Kanji78[] ;
extern	UINT8	Kanji79[] ;
extern	UINT8	Kanji7a[] ;
extern	UINT8	Kanji7b[] ;
extern	UINT8	Kanji7c[] ;
extern	UINT8	Kanji7d[] ;
extern	UINT8	Kanji7e[] ;
extern	UINT8	Kanji7f[] ;
extern	UINT8	Kanji80[] ;
extern	UINT8	Kanji81[] ;
extern	UINT8	Kanji82[] ;
extern	UINT8	Kanji83[] ;
extern	UINT8	Kanji84[] ;
extern	UINT8	Kanji85[] ;
extern	UINT8	Kanji86[] ;
extern	UINT8	Kanji87[] ;
extern	UINT8	Kanji88[] ;
extern	UINT8	Kanji89[] ;
extern	UINT8	Kanji8a[] ;
extern	UINT8	Kanji8b[] ;
extern	UINT8	Kanji8c[] ;
extern	UINT8	Kanji8d[] ;
extern	UINT8	Kanji8e[] ;
extern	UINT8	Kanji8f[] ;
extern	UINT8	Kanji90[] ;
extern	UINT8	Kanji91[] ;
extern	UINT8	Kanji92[] ;
extern	UINT8	Kanji93[] ;
extern	UINT8	Kanji94[] ;
extern	UINT8	Kanji95[] ;
extern	UINT8	Kanji96[] ;
extern	UINT8	Kanji97[] ;
extern	UINT8	Kanji98[] ;
extern	UINT8	Kanji99[] ;
extern	UINT8	Kanji9a[] ;
extern	UINT8	Kanji9b[] ;
extern	UINT8	Kanji9c[] ;
extern	UINT8	Kanji9d[] ;
extern	UINT8	Kanji9e[] ;
extern	UINT8	Kanji9f[] ;
extern	UINT8	Kanjia0[] ;
extern	UINT8	Kanjia1[] ;
extern	UINT8	Kanjia2[] ;
extern	UINT8	Kanjia3[] ;
extern	UINT8	Kanjia4[] ;
extern	UINT8	Kanjia5[] ;
extern	UINT8	Kanjia6[] ;
extern	UINT8	Kanjia7[] ;
extern	UINT8	Kanjia8[] ;
extern	UINT8	Kanjia9[] ;
extern	UINT8	Kanjiaa[] ;
extern	UINT8	Kanjiab[] ;
extern	UINT8	Kanjiac[] ;
extern	UINT8	Kanjiad[] ;
extern	UINT8	Kanjiae[] ;
extern	UINT8	Kanjiaf[] ;
extern	UINT8	Kanjib0[] ;
extern	UINT8	Kanjib1[] ;
extern	UINT8	Kanjib2[] ;
extern	UINT8	Kanjib3[] ;
extern	UINT8	Kanjib4[] ;
extern	UINT8	Kanjib5[] ;
extern	UINT8	Kanjib6[] ;
extern	UINT8	Kanjib7[] ;
extern	UINT8	Kanjib8[] ;
extern	UINT8	Kanjib9[] ;
extern	UINT8	Kanjiba[] ;
extern	UINT8	Kanjibb[] ;
extern	UINT8	Kanjibc[] ;
extern	UINT8	Kanjibd[] ;
extern	UINT8	Kanjibe[] ;
extern	UINT8	Kanjibf[] ;
extern	UINT8	Kanjic0[] ;
extern	UINT8	Kanjic1[] ;
extern	UINT8	Kanjic2[] ;
extern	UINT8	Kanjic3[] ;
extern	UINT8	Kanjic4[] ;
extern	UINT8	Kanjic5[] ;
extern	UINT8	Kanjic6[] ;
extern	UINT8	Kanjic7[] ;
extern	UINT8	Kanjic8[] ;
extern	UINT8	Kanjic9[] ;
extern	UINT8	Kanjica[] ;
extern	UINT8	Kanjicb[] ;
extern	UINT8	Kanjicc[] ;
extern	UINT8	Kanjicd[] ;
extern	UINT8	Kanjice[] ;
extern	UINT8	Kanjicf[] ;
extern	UINT8	Kanjid0[] ;
extern	UINT8	Kanjid1[] ;
extern	UINT8	Kanjid2[] ;
extern	UINT8	Kanjid3[] ;
extern	UINT8	Kanjid4[] ;
extern	UINT8	Kanjid5[] ;
extern	UINT8	Kanjid6[] ;
extern	UINT8	Kanjid7[] ;
extern	UINT8	Kanjid8[] ;
extern	UINT8	Kanjid9[] ;
extern	UINT8	Kanjida[] ;
extern	UINT8	Kanjidb[] ;
extern	UINT8	Kanjidc[] ;
extern	UINT8	Kanjidd[] ;
extern	UINT8	Kanjide[] ;
extern	UINT8	Kanjidf[] ;
extern	UINT8	Kanjie0[] ;
extern	UINT8	Kanjie1[] ;
extern	UINT8	Kanjie2[] ;
extern	UINT8	Kanjie3[] ;
extern	UINT8	Kanjie4[] ;
extern	UINT8	Kanjie5[] ;
extern	UINT8	Kanjie6[] ;
extern	UINT8	Kanjie7[] ;
extern	UINT8	Kanjie8[] ;
extern	UINT8	Kanjie9[] ;
extern	UINT8	Kanjiea[] ;
extern	UINT8	Kanjiec[] ;
#endif



#endif
