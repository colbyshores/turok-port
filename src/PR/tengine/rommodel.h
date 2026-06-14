// rommmodel.h

#ifndef _INC_ROMMMODEL
#define _INC_ROMMMODEL

/////////////////////////////////////////////////////////////////////////////

#include "cart.h"

#ifdef WIN32

#include "graphic.h"

typedef struct {
	short		ob[3];	      // x, y, z
	unsigned short	flag;
	short		tc[2];	      // texture coord
	unsigned char	cn[4];	// color & alpha
} Vtx_t;

typedef struct {
	short		ob[3];	      // x, y, z
	unsigned short	flag;
	short		tc[2];	      // texture coord
	signed char	n[3];	      // normal
	unsigned char   a;      // alpha
} Vtx_tn;

typedef union {
    Vtx_t		v;          // Use this one for colors
    Vtx_tn     n;          // Use this one for normals
    DWORD      force_structure_alignment[2];
} Vtx;

#else

#include "graphu64.h"

#endif

typedef struct CROMFacet_t
{
   DWORD       m_Vertices[3];
} CROMFacet;

/////////////////////////////////////////////////////////////////////////////
#endif // _INC_ROMMMODEL
