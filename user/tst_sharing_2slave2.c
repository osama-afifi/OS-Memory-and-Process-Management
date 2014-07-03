// Test the creation of shared variables and using them
// Slave program2: Get 2 shared variables, edit the writable one, and attempt to edit the readOnly one
#include <inc/lib.h>

void
_main(void)
{
	uint32 *x, *z;

	//GET: z then x, opposite to creation order (x then y then z)
	//So, addresses here will be different from the OWNER addresses
	int freeFrames = sys_calculate_free_frames() ;
	int ret = sys_getSharedObject("z", (void*)&z);
	if (z != (uint32*)(USER_SHARED_MEM_START + 0 * PAGE_SIZE)) panic("Get(): Returned address is not correct. make sure that you align the allocation on 4KB boundary");
	if ((freeFrames - sys_calculate_free_frames()) !=  1) panic("Get(): Wrong sharing- make sure that you share the required space in the current user environment using the correct frames (from frames_storage)");

	freeFrames = sys_calculate_free_frames() ;
	ret = sys_getSharedObject("x", (void*)&x);
	if (x != (uint32*)(USER_SHARED_MEM_START + 1 * PAGE_SIZE)) panic("Get(): Returned address is not correct. make sure that you align the allocation on 4KB boundary");
	if ((freeFrames - sys_calculate_free_frames()) !=  0) panic("Get(): Wrong sharing- make sure that you share the required space in the current user environment using the correct frames (from frames_storage)");
	if (*x != 10) panic("Get(): Shared Variable is not created or got correctly") ;

	//Edit the writable object
	*z = 30;
	if (*z != 30) panic("Get(): Shared Variable is not created or got correctly") ;

	//Attempt to edit the ReadOnly object, it should panic
	*x = 100;
	return;	
}
