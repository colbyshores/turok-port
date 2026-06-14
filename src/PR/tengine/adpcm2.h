#ifndef __ADPCM2_H__
#define	__ADPCM2_H__
//*****************************************************************************
//	ADPCM.H
//*****************************************************************************

#ifdef WIN32
// don't fuss when int's are converted to FLOAT's
#pragma warning(disable : 4244)
// don't fuss when functions are automatically made inline
#pragma warning(disable : 4711)
#endif

/*
#define	DEBUG	1

#if DEBUG
#define	ASSERT(x) {if(!(x)){printf("ASSERTION FAILED: File '%s' Line %d\n", __FILE__, __LINE__) ; exit (EXIT_FAILURE) ; }}
#define	DB(x) printf x
#else
#define	ASSERT(x)
#define	DB(x)
#endif

#define	FALSE	0
#define	TRUE	1
*/

#ifdef WIN32
typedef	unsigned char	u8 ;
typedef unsigned short	u16 ;
typedef unsigned long	u32 ;
typedef signed char		s8 ;
typedef signed short	s16 ;
typedef signed long		s32 ;
#endif

//*****************************************************************************
//	Defines
//*****************************************************************************

//---	The number of available filters
#define	MAX_FILTERS	4

//---	Coefficients for the various Filter types
#define	F0CA	0
#define	F0CB	0
#define	F1CA	0.9375
#define	F1CB	0
#define	F2CA	1.90625
#define	F2CB	-0.9375
#define	F3CA	1.796875
#define	F3CB	-0.8125

#define	MAX_SIGNALS	4					// Maximum number of Signals to process simultaneously
#define	BLOCK		4 					// Block Size for ADPCM Data
#define	BITS		3					// Number of Bits / Sample to use in Encoding

#define	BITM		((1<<BITS)-1)		// BitMask from BITS
#define	BIT_SIGN	(1<<(BITS-1))		// Position of Sign Bit
#define	VMIN		(-(1<<(BITS-1)))	// Maximum Negative Value
#define	VMAX		((1<<(BITS-1))-1)	// Maximum Positive Value

#define	MAX_SHIFTS	(16-BITS)			// Maximum number of Shifts to apply to Data

//*****************************************************************************
//	Prototypes
//*****************************************************************************

extern s32		adpcmGetBufferSize(s32 NumSignals, s32 SampleLen) ;

extern s32		adpcmEncode (u8 *ADPCMBuffer, s16 **RawBuffers, s32 NumSignals, s32 SampleLen) ;
extern s32		adpcmDecode (s16 **RawBuffers, u8 *ADPCMBuffer, s32 NumSignals) ;

#endif /*__ADPCM2_H__*/
