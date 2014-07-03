	
#include <inc/lib.h>

/*
 * Simple malloc()
 *
 * The address space for the dynamic allocation is 
 * from "USER_HEAP_START" to "USER_HEAP_MAX"-1
 * Pages are allocated ON 4KB ALIGMENT
 * On succeed, return void pointer to the allocated space
 * return NULL if
 *	-there's no suitable space for the required allocation
 */

// malloc()
//	This function use FIRST FIT strategy to allocate space in heap with the given size
//  and return void pointer to the start of the allocated space

//	To do this, we need to switch to the kernel, allocate the required space
//	in Main memory then switch back to the user again.
//
//	We can use sys_allocateMem(uint32 virtual_address, uint32 size); which
//		switches to the kernel mode, calls allocateMem(struct Env* e, uint32 virtual_address, uint32 size) in
//		"memory_manager.c", then switch back to the user mode here
//	the allocateMem function is empty, make sure to implement it.

void* malloc(uint32 size)
{	
	//TODO: [PROJECT 2014 - Heap] malloc()
	// your code is here, remove the panic and write your code
	panic("malloc() is not implemented yet...!!");

	// Steps:
	//	1) search for FIRST FIT space in heap that is suitable for the required allocation size
	//	2) if no suitable space found, return NULL
	//	 Else,
	//	3) Call sys_allocateMem to invoke the Kernel for allocation
	// 	4) Return pointer containing the virtual address of allocated space,
	//

	//This function should allocate ALL pages of the required range
	// ******** ON 4KB ALIGNMENT *******************


	return 0;
}


// free():
//	This function frees the allocation of the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND empty PAGE TABLES only
//	from Main memory then switch back to the user again.
//	
//	We can use sys_freeMem(uint32 virtual_address, uint32 size); which
//		switches to the kernel mode, calls freeMem(struct Env* e, uint32 virtual_address, uint32 size) in
//		"memory_manager.c", then switch back to the user mode here
//	the freeMem function is empty, make sure to implement it.

void free(void* virtual_address)
{
	//TODO: [PROJECT 2014 - Heap] free()
	// your code is here, remove the panic and write your code
	panic("free() is not implemented yet...!!");
	//get the size of the given allocation using its address
	//you need to call sys_freeMem()

}


//================= [BONUS] =====================

// realloc():

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to malloc().
//	A call with new_size = zero is equivalent to free().

//  Hint: you may need to use the sys_moveMem(uint32 src_virtual_address, uint32 dst_virtual_address, uint32 size)
//		which switches to the kernel mode, calls moveMem(struct Env* e, uint32 src_virtual_address, uint32 dst_virtual_address, uint32 size)
//		in "memory_manager.c", then switch back to the user mode here
//	the moveMem function is empty, make sure to implement it.

void *realloc(void *virtual_address, uint32 new_size)
{
	//TODO: [PROJECT 2014 - BONUS1] realloc()
	// your code is here, remove the panic and write your code
	panic("realloc() is not implemented yet...!!");

}

