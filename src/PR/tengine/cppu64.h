/* cppu64.h */

#define F3DEX_GBI

#ifdef _LANGUAGE_CPP

#define _LANGUAGE_C
#undef _LANGUAGE_CPP

extern "C"
{
#include <ultra64.h>
}

#define _LANGUANGE_CPP

#else

#include <ultra64.h>

#endif
