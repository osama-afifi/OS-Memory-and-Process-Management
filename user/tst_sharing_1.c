// Test the creation of shared variables (create_shared_memory)
#include <inc/lib.h>

void
_main(void)
{
	uint32 *x, *y, *z ;
	cprintf("STEP A: checking the creation of shared variables... \n");
	{
		int freeFrames = sys_calculate_free_frames() ;
		int ret = sys_createSharedObject("x", PAGE_SIZE, 1, (void*)&x);
		if (x != (uint32*)USER_SHARED_MEM_START) panic("Returned address is not correct. check the setting of it and/or the updating of the shared_mem_free_address");
		if ((freeFrames - sys_calculate_free_frames()) !=  1+1+1) panic("Wrong allocation: make sure that you allocate the required space in the user environment and add its frames to frames_storage");

		freeFrames = sys_calculate_free_frames() ;
		ret = sys_createSharedObject("z", PAGE_SIZE + 4, 1, (void*)&z);
		if (z != (uint32*)(USER_SHARED_MEM_START + 1 * PAGE_SIZE)) panic("Returned address is not correct. check the setting of it and/or the updating of the shared_mem_free_address");
		if ((freeFrames - sys_calculate_free_frames()) !=  2+0+1) panic("Wrong allocation: make sure that you allocate the required space in the user environment and add its frames to frames_storage");

		freeFrames = sys_calculate_free_frames() ;
		ret = sys_createSharedObject("y", 4, 1, (void*)&y);
		if (y != (uint32*)(USER_SHARED_MEM_START + 3 * PAGE_SIZE)) panic("Returned address is not correct. check the setting of it and/or the updating of the shared_mem_free_address");
		if ((freeFrames - sys_calculate_free_frames()) !=  1+0+1) panic("Wrong allocation: make sure that you allocate the required space in the user environment and add its frames to frames_storage");


	}
	cprintf("Step A is completed successfully!!\n\n\n");


	cprintf("STEP B: checking reading & writing... \n");
	{
		int i=0;
		for(;i<PAGE_SIZE/4;i++)
		{
			x[i] = -1;
			y[i] = -1;
		}

		i=0;
		for(;i<2*PAGE_SIZE/4;i++)
		{
			z[i] = -1;
		}

		if( x[0] !=  -1)  					panic("Reading/Writing of shared object is failed");
		if( x[PAGE_SIZE/4 - 1] !=  -1)  	panic("Reading/Writing of shared object is failed");

		if( y[0] !=  -1)  					panic("Reading/Writing of shared object is failed");
		if( y[PAGE_SIZE/4 - 1] !=  -1)  	panic("Reading/Writing of shared object is failed");

		if( z[0] !=  -1)  					panic("Reading/Writing of shared object is failed");
		if( z[2*PAGE_SIZE/4 - 1] !=  -1)  	panic("Reading/Writing of shared object is failed");
	}

	cprintf("Congratulations!! Test of Shared Variables [Create] [1] completed successfully!!\n\n\n");

	return;
}
