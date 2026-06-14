// regtype.h

#ifndef _INC_REGTYPE
#define _INC_REGTYPE

/////////////////////////////////////////////////////////////////////////////
// Region material types

#define REGMAT_GRASS				0
#define REGMAT_WATERSURFACE	1
#define REGMAT_STEEL				2
#define REGMAT_STONE				3
#define REGMAT_FLESH				4
#define REGMAT_ALIENFLESH		5
#define REGMAT_FLESHWATER		6
#define REGMAT_LAVA				7
#define REGMAT_TAR				8
#define REGMAT_FORCEFIELD		9

#define REGMAT_DEFAULT			REGMAT_GRASS


// pre version 22 of region material types
#define	PRE22_REGMAT_GRASS					0
#define	PRE22_REGMAT_DIRT						1
#define	PRE22_REGMAT_WATERSURFACE			2
#define	PRE22_REGMAT_STEEL					3
#define	PRE22_REGMAT_STONE					4
#define	PRE22_REGMAT_WOOD						5
#define	PRE22_REGMAT_FOLIAGE					6
#define	PRE22_REGMAT_FLESH					7
#define	PRE22_REGMAT_ALIENFLESH				8
#define  PRE22_REGMAT_WATERFLOOR				9
#define	PRE22_REGMAT_FLESHWATER				10
#define	PRE22_REGMAT_LAVA						11
#define	PRE22_REGMAT_TAR						12
#define	PRE22_REGMAT_FORCEFIELD				13

/////////////////////////////////////////////////////////////////////////////
// region flags

// stored in region DWORD
#define REGFLAG_HASWATER							(1 << 0)
#define REGFLAG_DOOR									(1 << 1)
#define REGFLAG_OPENDOOR							(1 << 2)
#define REGFLAG_CLIFF								(1 << 3)
#define REGFLAG_CLIMBABLE							(1 << 4)
#define REGFLAG_BRIDGE								(1 << 5)
#define REGFLAG_CEILING								(1 << 6)
#define REGFLAG_DUCK									(1 << 7)
#define REGFLAG_DUCKENTRANCEEXIT					(1 << 8)
#define REGFLAG_DONTDRAWONMAP						(1 << 9)
#define REGFLAG_REGIONENTERED						(1 << 10)
#define REGFLAG_RESTRICTEDAREA					(1 << 12)
#define REGFLAG_RUNTIMECLIFF						(1 << 13)
#define REGFLAG_FALLDEATH							(1 << 14)
#define REGFLAG_RECURSE								(1 << 15)

// only stored in region set DWORD
#define REGFLAG_PRESSUREPLATE						(1 << 16)
#define REGFLAG_PRESSUREPLATENEEDSCOLLISION	(1 << 17)
#define REGFLAG_WARPENTRANCE						(1 << 18)
#define REGFLAG_INSTANTDEATH						(1 << 19)
#define REGFLAG_SKY									(1 << 20)
#define REGFLAG_WARPIFINAIR						(1 << 21)
#define REGFLAG_LAVA									(1 << 22)
#define REGFLAG_PRESSUREPLATESOUND				(1 << 23)
#define REGFLAG_ANTIGRAV							(1 << 24)
#define REGFLAG_LADDER								(1 << 25)
#define REGFLAG_CHECKPOINT							(1 << 26)
#define REGFLAG_SAVEPOINT							(1 << 27)
#define REGFLAG_WARPRETURN							(1 << 28)
#define REGFLAG_SHALLOWWATER						(1 << 29)
#define REGFLAG_SUN									(1 << 30)
#define REGFLAG_STOREWARPRETURN					(1 << 31)



// pre version 25
#define REGFLAG_PRE25_WARPENTRANCE				(1 << 9)
#define REGFLAG_PRE25_INSTANTDEATH				(1 << 10)
#define REGFLAG_PRE25_SKY							(1 << 11)
#define REGFLAG_PRE25_WARPIFINAIR				(1 << 15)
#define REGFLAG_PRE25_DONTDRAWONMAP				(1 << 30)


/////////////////////////////////////////////////////////////////////////////
// region ambient sound types

#define REG_AMBIENTNONE							0
#define REG_AMBIENTJUNGLE						1
#define REG_AMBIENTWATER						2
#define REG_AMBIENTCAVES						3
#define REG_AMBIENTCATACOMBS					4
#define REG_AMBIENTLOSTLANDS					5
#define REG_AMBIENTCAMPAIGNERSLAIR			6
#define REG_AMBIENTCATACOMBLARGE				7
#define REG_AMBIENTCITY							8
#define REG_AMBIENTRUINS						9
#define REG_AMBIENTTREETOP						10
#define REG_AMBIENTWARP							11


/////////////////////////////////////////////////////////////////////////////
// region map color types

#define REG_MAPCOLOR_WHITE						0
#define REG_MAPCOLOR_GREEN						1
#define REG_MAPCOLOR_BLUE						2
#define REG_MAPCOLOR_RED						3
#define REG_MAPCOLOR_PINK						4
#define REG_MAPCOLOR_PURPLE					5
#define REG_MAPCOLOR_BROWN						6
#define REG_MAPCOLOR_CYAN						7
#define REG_MAPCOLOR_YELLOW					8
#define REG_MAPCOLOR_ORANGE					9
#define REG_MAPCOLOR_BLACK						10


/////////////////////////////////////////////////////////////////////////////
#endif // _INC_REGTYPE

