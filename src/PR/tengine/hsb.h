// hsb.h

void GetOffsetHue(signed char Offset,
						BYTE inR, BYTE inG, BYTE inB,
						BYTE *pOutR, BYTE *pOutG, BYTE *pOutB);
void GetReducedSaturation(BYTE Reduce,
								  BYTE inR, BYTE inG, BYTE inB,
								  BYTE *pOutR, BYTE *pOutG, BYTE *pOutB);
void GetReducedBrightness(BYTE Reduce,
								  BYTE inR, BYTE inG, BYTE inB,
								  BYTE *pOutR, BYTE *pOutG, BYTE *pOutB);
