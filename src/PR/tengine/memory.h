// memory.h

#ifndef	__MEMORY_H
#define	__MEMORY_H

#include <stddef.h>

#ifndef WIN32
#include "defs.h"
#endif

#ifdef WIN32
// don't fuss when functions are automatically made inline
#pragma warning(disable : 4711)
#endif

/*****************************************************************************
 *
 ****************************************************************************/

//#define	MEMORY_START	(0x80000000 + 0x000f0000)
//#define	MEMORY_END	(0x80000000 + (1024 * (2048 - 32)))
//#define	MEMORY_LENGTH	(MEMORY_END - MEMORY_START)

#define	MEMORY_ALIGN	8

// 16 byte alignment to keep with cache data line size
//#define	MEMORY_ALIGN	16

/*****************************************************************************
 * Structures
 ****************************************************************************/

//----	Memory allocation structure
typedef struct s_Memory {
	struct s_Memory *prev ;
	struct s_Memory *next ;
	void			*addr ;
	int				size ;
} t_Memory ;


//----	Pool allocation structure
typedef struct s_MemoryPool {
	void			*addr ;
	int				size ;			// Available memory (not including t_MemoryPool * t_Memory headers)
	struct s_Memory *free_head ;
	struct s_Memory *free_tail ;
	struct s_Memory *used_head ;
} t_MemoryPool ;


/*****************************************************************************
 * Externals
 ****************************************************************************/
//extern	INT32		_memory_largest ;


/*****************************************************************************
 * Protocols
 ****************************************************************************/
void	i3D_initMemoryAllocation (void *pAddress, int Size) ;

t_MemoryPool *i3D_addPool (int size) ;
int		i3D_removePool (t_MemoryPool *pool) ;
void	*i3D_mallocPool (t_MemoryPool *pool, int size) ;
void	*i3D_mallocPoolTop (t_MemoryPool *pool, int size) ;

void	*i3D_reallocPool (t_MemoryPool *pool, void *address, int size) ;
void	*i3D_realloc (void *address, int size) ;

int		i3D_freePool (t_MemoryPool *pool, void *address) ;
int		i3D_freeAllPool (t_MemoryPool *pool) ;

void	*i3D_malloc (int size) ;
void	*i3D_mallocTop (int size) ;
int		i3D_free (void *address) ;
int		i3D_freeAll (void) ;

int		i3D_memoryCheck (t_MemoryPool *pool) ;
int i3D_memoryCheckGlobal();

void FreeMem(int *pTotal, int *pLargest);
BOOL BlockBelowFreeChunk(void *pAddress);
void PrintLowestFree();

// Overloaded memory functions
/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
#define ALLOC(size) (new BYTE[size])
#define ALLOCTOP(size) (new BYTE[size])
#define DEALLOC(ptr) (delete [] (BYTE*)ptr)
#else
#define ALLOC(size) i3D_malloc(size)
#define ALLOCTOP(size) i3D_mallocTop(size)
#define DEALLOC(ptr) i3D_free(ptr)
#endif

#ifndef WIN32
void r_memcpy(void *pDest, void *pSrc, int Count);
void memset(void *pDest, BYTE Val, int Count);
#endif

/////////////////////////////////////////////////////////////////////////////

#endif	/*	#ifndef __MEMORY_H */
