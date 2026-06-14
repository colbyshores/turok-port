// memory.c

//#include <sys/types.h>
//#include <libetc.h>
//#include <libgte.h>
//#include <libgpu.h>
//#include <libgs.h>
//#include <libsn.h>

//#include "engine.h"

//#define	CHECK		OFF

#include "stdafx.h"
#include "memory.h"

#ifdef WIN32
#define INT32 signed long
#define UINT32 unsigned long
#endif

#define ON 	1
#define OFF 0

//#undef IG_DEBUG
#ifdef IG_DEBUG
#define	CHECK		ON
#else
#define	CHECK		OFF
#endif

#ifndef WIN32
///*
void r_memcpy(void *pDest, void *pSrc, int Count)
{
	int i;

	//TRACE3("memcpy(%x, %x, %x)\r\n", pDest, pSrc, Count);

	for (i=0; i<Count; i++)
		((BYTE*)pDest)[i] = ((BYTE*)pSrc)[i];
}
//*/
void memset(void *pDest, BYTE Val, int Count)
{
	int i;

	//TRACE3("memset(%x, %x, %x)\r\n", pDest, Val, Count);

	for (i=0; i<Count; i++)
		((BYTE*)pDest)[i] = Val;
}
#endif

/*****************************************************************************
 * Variables
 ****************************************************************************/

t_MemoryPool	_global_pool ;
INT32				_memory_largest ;
void				*_global_pool_address	= NULL,
					*_global_pool_end			= NULL;
int				_global_pool_size			= 0;


#if	CHECK
void checklist (void) ;
void checklist (void)
{
	t_Memory	*pointer ;
	t_Memory	*prev ;
	//t_Memory	*next ;
	//t_Memory	*free ;

	pointer = _global_pool.used_head ;

	if (pointer == NULL)
		return ;

	//if ((pointer < (t_Memory *)MEMORY_START) || (pointer > (t_Memory *)MEMORY_END))
	if ((pointer < (t_Memory *)_global_pool_address) || (pointer > (t_Memory *)_global_pool_end))
		ASSERT(FALSE) ;

	if (pointer->prev != NULL)
		ASSERT(FALSE) ;

	while (pointer)
	{
		if (pointer->next)
		{
			//if ((pointer->next < (t_Memory *)MEMORY_START) || (pointer->next > (t_Memory *)MEMORY_END))
			if ((pointer->next < (t_Memory *)_global_pool_address) || (pointer->next > (t_Memory *)_global_pool_end))
		 		ASSERT(FALSE) ;
		}

		prev = pointer ;
		pointer = pointer->next ;

		if (pointer)
		{
			if (pointer->prev != prev)
				ASSERT(FALSE) ;
		}
	}
}
#endif

int i3D_memoryCheckGlobal()
{
	return i3D_memoryCheck(&_global_pool);
}

/*****************************************************************************
 *
 *		Function Title:	i3D_initMemoryAllocation ()
 *
 *****************************************************************************
 *
 *		Synopsis:		void	i3D_initMemoryAllocation (void)
 *
 *		Description:	Initializes memory allocation list.
 *
 *		Inputs:			None
 *
 *		Outputs:		None
 *
 ****************************************************************************/

void	i3D_initMemoryAllocation (void *pAddress, int Size)
{
	_global_pool_address = pAddress;
	_global_pool_size = Size;
	_global_pool_end = (void*) (((INT32)pAddress) + Size);

	//---	Init. memory allocation lists
	_global_pool.addr = NULL ;
 	_global_pool.size = NULL ;
 	//_global_pool.free_head = (t_Memory *)MEMORY_START ;
	_global_pool.free_head = (t_Memory *) pAddress ;
 	_global_pool.free_tail = _global_pool.free_head ;
	_global_pool.free_head->prev = NULL ;
	_global_pool.free_head->next = NULL ;
	_global_pool.free_head->addr = (void *)((int)_global_pool.free_head + sizeof (t_Memory)) ;
	//_global_pool.free_head->size = MEMORY_LENGTH - sizeof (t_Memory) ;
	_global_pool.free_head->size = Size - sizeof (t_Memory) ;

 	_global_pool.used_head = NULL ;
}


/*****************************************************************************
 *
 *		Function Title:	i3D_addPool ()
 *
 *****************************************************************************
 *
 *		Synopsis:		t_MemoryPool *i3D_addPool (int size)
 *
 *		Description:	Allocates new pool for memory allocation.
 *
 *		Inputs:			int size			- Number of bytes to allocate.
 *
 *		Outputs:		Number of bytes available
 *						NULL = Unable to allocate memory.
 *
 ****************************************************************************/

t_MemoryPool *i3D_addPool (int size)
{
	t_MemoryPool	*pool ;
	int				total_size ;

	total_size = size + sizeof (t_MemoryPool) + sizeof (t_Memory) ;
	pool = (t_MemoryPool*) i3D_malloc(total_size) ;

	if (pool)
	{
	 	pool->addr = (void *)pool ;
	 	pool->size = total_size ;
	 	pool->free_head = (t_Memory *)((int)pool + sizeof (t_MemoryPool)) ;
	 	pool->free_tail = pool->free_head ;
		pool->free_head->prev = NULL ;
		pool->free_head->next = NULL ;
		pool->free_head->addr = (void *)((int)pool->free_head + sizeof (t_Memory)) ;
		pool->free_head->size = size ;

	 	pool->used_head = NULL ;
	}

	return pool ;
}


/*****************************************************************************
 *
 *		Function Title:	i3D_removePool ()
 *
 *****************************************************************************
 *
 *		Synopsis:		int	i3D_removePool (t_MemoryPool *pool)
 *
 *		Description:	Deallocates pool memory
 *
 *		Inputs:			t_MemoryPool *pool	- Pointer to pool
 *
 *		Outputs:		TRUE / FALSE
 *
 ****************************************************************************/

int	i3D_removePool (t_MemoryPool *pool)
{
	return (i3D_free (pool)) ;
}


/*****************************************************************************
 *
 *		Function Title:	i3D_mallocPool ()
 *
 *****************************************************************************
 *
 *		Synopsis:		void	*i3D_mallocPool (t_MemoryPool *pool, int size)
 *
 *		Description:	Allocates memory from the start of memory pool.
 *
 *		Inputs:			t_MemoryPool *pool	- Memory pool to allocate from.
 *						int size			- Number of bytes to allocate.
 *
 *		Outputs:		Address of allocated memory.
 *						NULL = Unable to allocate memory.
 *
 ****************************************************************************/

void	*i3D_mallocPool (t_MemoryPool *pool, int size)
{
	t_Memory	*pointer ;
	t_Memory	*free ;
	t_Memory	*prev ;
	t_Memory	*next ;
	int			total_size ;

	//---	Illegal allocation size ?
	if (size < 0)
	{
		ASSERT(FALSE);
		return	NULL ;
	}

	//---	Align size to MEMORY_ALIGN bytes
	size = (size + MEMORY_ALIGN - 1) & ~(MEMORY_ALIGN - 1) ;

	//---	Find free allocation slot
	pointer = pool->free_head ;
	total_size = size + sizeof (t_Memory) ;

	while (pointer)
	{
		if (pointer->size >= total_size)
		{
			//---	Modify previous and next linked elements
			free = (t_Memory *)((int)pointer + total_size) ;
			prev = pointer->prev ;
			next = pointer->next ;

			if (prev)
				prev->next = free ;
			if (next)
				next->prev = free ;

			//---	Modify free entry
			free->prev = prev ;
			free->next = next ;
			free->addr = (void *)((int)free + sizeof (t_Memory)) ;
			free->size = pointer->size - total_size ;

			if (pool->free_head == pointer)
				pool->free_head = free ;
			if (pool->free_tail == pointer)
				pool->free_tail = free ;

			//---	Setup new used entry
			pointer->prev = NULL ;
			pointer->next = pool->used_head ;
			pointer->addr = (t_Memory *)((int)pointer + sizeof (t_Memory)) ;
			pointer->size = size ;

			if (pool->used_head)
				pool->used_head->prev = pointer ;

			pool->used_head = pointer ;

			return pointer->addr ;
		}

		pointer = pointer->next ;
	}

	return NULL;
}


/*****************************************************************************
 *
 *		Function Title:	i3D_reallocPool ()
 *
 *****************************************************************************
 *
 *		Synopsis:		void	*i3D_reallocPool (t_MemoryPool *pool, void *address, int size)
 *
 *		Description:	ReAllocates memory.
 *
 *		Inputs:			t_MemoryPool *pool	- Memory pool to allocate from.
 *						void *address		- Addres of allocated memory.
 *						int size			- Number of bytes to allocate.
 *
 *		Outputs:		Address of allocated memory.
 *						NULL = Unable to allocate memory.
 *
 ****************************************************************************/

void	*i3D_reallocPool (t_MemoryPool *pool, void *address, int size)
{
	void		*newaddr ;
	UINT32		*source ;
	UINT32		*dest ;
	int			i ;
	int			oldsize ;
	int			count ;

	//---	Illegal allocation size ?
	if (size < 0)
		return	NULL ;

	oldsize = i3D_freePool (pool, address) ;

	if (oldsize)
	{
		newaddr = i3D_mallocPool (pool, size) ;

			//printf ("old addr = %p\n", address) ;
			//printf ("new addr = %p\n", newaddr) ;
			//printf ("newsize = %d (%d)\n", size, oldsize) ;

		if (newaddr != address)
		{
			source = (UINT32*) address ;
			dest = (UINT32*) newaddr ;

			if (size < oldsize)
				count = size ;
			else
				count = oldsize ;

			for (i = 0 ; i < count ; i += 4)
				*dest++ = *source++ ;
		}

		return	newaddr ;
	}
	else
		return	NULL ;
}


/*****************************************************************************
 *
 *		Function Title:	i3D_realloc ()
 *
 *****************************************************************************
 *
 *		Synopsis:		void	*i3D_realloc (void *address, int size)
 *
 *		Description:	ReAllocates memory.
 *
 *		Inputs:			void *address		- Addres of allocated memory.
 *						int size			- Number of bytes to allocate.
 *
 *		Outputs:		Address of allocated memory.
 *						NULL = Unable to allocate memory.
 *
 ****************************************************************************/

void	*i3D_realloc (void *address, int size)
{
	return (i3D_reallocPool (&_global_pool, address, size)) ;
}


/*****************************************************************************
 *
 *		Function Title:	i3D_mallocPoolTop ()
 *
 *****************************************************************************
 *
 *		Synopsis:		void	*i3D_mallocPoolTop (t_MemoryPool *pool, int size)
 *
 *		Description:	Allocates memory from the end of memory.
 *
 *		Inputs:			t_MemoryPool *pool	- Memory pool to allocate from.
 *						int size			- Number of bytes to allocate.
 *
 *		Outputs:		Address of allocated memory.
 *						NULL = Unable to allocate memory.
 *
 ****************************************************************************/

void	*i3D_mallocPoolTop (t_MemoryPool *pool, int size)
{
	t_Memory	*pointer ;
	t_Memory	*free ;
	int			total_size ;

	//---	Illegal allocation size ?
	if (size < 0)
		return	NULL ;

	//---	Align size to 4 bytes
	size = (size + MEMORY_ALIGN - 1) & ~(MEMORY_ALIGN - 1) ;

	//---	Find free allocation slot
	pointer = pool->free_tail ;
	total_size = size + sizeof (t_Memory) ;

	while (pointer)
	{
		if (pointer->size >= total_size)
		{
			//---	Modify free entry
			pointer->size -= total_size ;

			//---	Setup new used entry
 			free = (t_Memory *)((int)pointer->addr + pointer->size) ;
			free->prev = NULL ;
			free->next = pool->used_head ;
			free->addr = (t_Memory *)((int)free + sizeof (t_Memory)) ;
			free->size = size ;

			if (pool->used_head)
				pool->used_head->prev = free ;

			pool->used_head = free ;

			return free->addr ;
		}

		pointer = pointer->prev ;
	}

	return NULL ;
}

/*****************************************************************************
 *
 *		Function Title:	i3D_freePool ()
 *
 *****************************************************************************
 *
 *		Synopsis:		int	*i3D_freePool (t_MemoryPool *pool, void *address)
 *
 *		Description:	Free memory.
 *
 *		Inputs:			t_MemoryPool *pool  - Memory pool to deallocate from.
 *						void *address		- Address of memory to deallocate.
 *
 *		Outputs:		Size of memory freed (NULL = error)
 *
 ****************************************************************************/

int		i3D_freePool (t_MemoryPool *pool, void *address)
{
	t_Memory	*pointer ;
	t_Memory	*prev ;
	t_Memory	*next ;
	t_Memory	*free ;
	int			size ;

	if (address == NULL)
	{
		ASSERT(FALSE);
		return FALSE ;
	}


	pointer = (t_Memory*)( ((UINT32)address) - sizeof(t_Memory) ) ;

	if (pointer->addr == address)
	{
		size = pointer->size ;
//		pointer->addr = NULL ;

		//---	Remove deallocated memory from used linked list
		prev = pointer->prev ;
		next = pointer->next ;

		if (prev)
			prev->next = next ;
		if (next)
			next->prev = prev ;

		if (pool->used_head == pointer)
			pool->used_head = next ;


		//---	Find position to insert free entry
		free = pool->free_head ;
		prev = NULL ;

		while (free)
		{
			if (free->addr > address)
				break ;

			prev = free ;
			free = free->next ;
		}

		//---	Insert free entry in found position
		if (free == pool->free_head)
		{
				//TRACE0 ("Free at head\n") ;
			//---	Insert entry at head of list
			pointer->prev = NULL ;
			pointer->next = pool->free_head ;
 			free->prev = pointer ;
			pool->free_head = pointer ;
		}
		else
		{
			if (free == NULL)
			{
					//TRACE0 ("Free at tail\n") ;
				//---	Insert entry at end of list
				prev->next = pointer ;
				pointer->prev = prev ;
				pointer->next = NULL ;
				pool->free_tail = pointer ;
			}
			else
			{
					//TRACE0 ("Free in middle\n") ;
				//---	Insert entry somewhere in the middle of list
				prev->next = pointer ;
				pointer->prev = prev ;
				pointer->next = free ;
				free->prev = pointer ;
			}
		}


		//---	Check if new entry can be combined with previous and next entries
		prev = pointer->prev ;
		next = pointer->next ;

		if (prev)
		{
			if ((int)pointer == ((int)prev->addr + prev->size))
			{
				prev->size += pointer->size + sizeof (t_Memory) ;
				prev->next = next ;

				if (next)
					next->prev = prev ;

				if (pool->free_tail == pointer)
					pool->free_tail = prev ;

				pointer = prev ;

						//TRACE0 ("Link Prev\n");
			}
		}

		if (next)
		{
			if ((int)next == ((int)pointer->addr + pointer->size))
			{
				pointer->size += next->size + sizeof (t_Memory) ;

				if (pool->free_tail == next)
					pool->free_tail = pointer ;

				next = next->next ;

				pointer->next = next ;

				if (next)
					next->prev = pointer ;

						//TRACE0 ("Link Next\n");
			}
		}
		else
			pool->free_tail = pointer ;

//		pointer->addr = NULL ;

		return	size ;
	}
	else
	{
		ASSERT(FALSE);
	}

	return	FALSE ;
}


/*****************************************************************************
 *
 *		Function Title:	i3D_freeAllPool ()
 *
 *****************************************************************************
 *
 *		Synopsis:		int		*i3D_freeAllPool (t_MemoryPool *pool)
 *
 *		Description:	Free all allocated in memory pool.
 *
 *		Inputs:			t_MemoryPool *pool	- Pool to deallocate memory from.
 *
 *		Outputs:		True / False
 *
 ****************************************************************************/

int		i3D_freeAllPool (t_MemoryPool *pool)
{
	t_Memory	*pointer ;

	//---	Find free allocation slot
	pointer = pool->used_head ;

	while (pointer)
	{
		i3D_freePool(pool, pointer->addr) ;

		pointer = pool->used_head ;
	}

	return	TRUE ;
}


/*****************************************************************************
 *
 *		Function Title:	i3D_malloc ()
 *
 *****************************************************************************
 *
 *		Synopsis:		void	*i3D_malloc (int size)
 *
 *		Description:	Allocates memory from the start of memory.
 *
 *		Inputs:			int size		- Number of bytes to allocate.
 *
 *		Outputs:		Address of allocated memory.
 *						NULL = Unable to allocate memory.
 *
 ****************************************************************************/

void	*i3D_malloc (int size)
{
#if CHECK
				void	*addr ;

 				checklist () ;

				addr = i3D_mallocPool (&_global_pool, size) ;
#ifdef TRACE_MEM
				TRACE2("+ Allocated %x, len:%x\r\n", addr, size);
#endif
				checklist () ;

#ifdef TRACE_MEM
				if (addr == FALSE)
				{
					TRACE("ALLOC: WARNING - Could not alloc memory, len %x.\r\n", size);
				}
#endif

				return	addr ;
#else
	void *addr = i3D_mallocPool (&_global_pool, size);
#ifdef TRACE_MEM
	TRACE2("+ Allocated %x, len:%x\r\n", addr, size);
#endif
	return (addr) ;
#endif
}


/*****************************************************************************
 *
 *		Function Title:	i3D_mallocTop ()
 *
 *****************************************************************************
 *
 *		Synopsis:		void	*i3D_mallocTop (int size)
 *
 *		Description:	Allocates memory from the end of memory.
 *
 *		Inputs:			int size		- Number of bytes to allocate.
 *
 *		Outputs:		Address of allocated memory.
 *						NULL = Unable to allocate memory.
 *
 ****************************************************************************/

void	*i3D_mallocTop (int size)
{
				void	*addr ;
#if CHECK
 					checklist () ;
				addr = i3D_mallocPoolTop (&_global_pool, size) ;
 					checklist () ;
#ifdef TRACE_MEM
				TRACE2("+ AllocatedTop %x, len:%x\r\n", addr, size);
#endif

#ifdef TRACE_MEM
				if (addr == FALSE)
				{
					TRACE("ALLOCTOP: WARNING - Could not alloc memory, len %x.\r\n", size);
				}
#endif

				return	addr ;
#else
				addr = i3D_mallocPoolTop (&_global_pool, size);
#ifdef TRACE_MEM
				TRACE2("+ AllocatedTop %x, len:%x\r\n", addr, size);
#endif
	return addr ;
#endif
}


/*****************************************************************************
 *
 *		Function Title:	i3D_free ()
 *
 *****************************************************************************
 *
 *		Synopsis:		int	*i3D_free (void *address)
 *
 *		Description:	Free memory.
 *
 *		Inputs:			void *address	- Address of memory to deallocate.
 *
 *		Outputs:		True / False
 *
 ****************************************************************************/

int		i3D_free (void *address)
{
#if CHECK
				int	flag ;

// TEST
//return 0;

				ASSERT(address);
 					checklist () ;

				flag = i3D_freePool (&_global_pool, address) ;
#ifdef TRACE_MEM
				TRACE("- Deallocated %x\r\n", address);
#endif

 					checklist () ;

				if (flag == FALSE)
					ASSERT(FALSE) ;


				return	flag ;
#else
	ASSERT(address);
#ifdef TRACE_MEM
	TRACE("- Deallocated %x\r\n", address);
#endif
	return (i3D_freePool (&_global_pool, address)) ;
#endif
}


/*****************************************************************************
 *
 *		Function Title:	i3D_freeAll ()
 *
 *****************************************************************************
 *
 *		Synopsis:		int		*i3D_freeAll (void)
 *
 *		Description:	Free all allocated memory.
 *
 *		Inputs:			None.
 *
 *		Outputs:		True / False
 *
 ****************************************************************************/

int		i3D_freeAll (void)
{
	return (i3D_freeAllPool (&_global_pool)) ;
}


/*****************************************************************************
 *
 *		Function Title:	i3D_memoryCheck ()
 *
 *****************************************************************************
 *
 *		Synopsis:		void	i3D_memoryCheck (t_MemoryPool *pool)
 *
 *		Description:	Checks how much memory is free in a given pool
 *
 *		Inputs:			t_MemoryPool	*pool	- Memory pool to check.
 *												  0 = Global pool
 *
 *		Outputs:		Size of free memory
 *
 ****************************************************************************/

int		i3D_memoryCheck (t_MemoryPool *pool)
{
	t_Memory	*pointer ;
	//int			total_size ;
	int			size ;
	int			largest ;

	size = 0 ;

	//---	Find free allocation slot
	if (pool == 0)
		pool = &_global_pool ;

	pointer = pool->free_head ;
	//total_size = size + sizeof (t_Memory) ;

	largest = 0 ;

	while (pointer)
	{
		if (pointer->size > largest)
			largest = pointer->size ;

		size += pointer->size ;
		pointer = pointer->next ;
	}

	_memory_largest = largest ;

	return size ;
}

void FreeMem(int *pTotal, int *pLargest)
{
	*pTotal = i3D_memoryCheckGlobal();
	*pLargest = _memory_largest;
}

BOOL BlockBelowFreeChunk(void *pAddress)
{
	BOOL below;

	ASSERT(_global_pool.free_head);

	if (_global_pool.free_head)
	{
		below = (((DWORD)pAddress) < ((DWORD)_global_pool.free_head));

		return below;
	}
	else
	{
		return TRUE;
	}
}

void PrintLowestFree()
{
	TRACE2("lowest free:%x, len:%x\r\n",
			 _global_pool.free_head,
			 _global_pool.free_head->size);
}
