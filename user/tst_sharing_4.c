// Test the free of shared variables (create_shared_memory)
#include <inc/lib.h>

void
_main(void)
{
	cprintf("STEP A: checking free of shared object ... \n");
	{
		uint32 *x;
		int freeFrames = sys_calculate_free_frames() ;
		int ret = sys_createSharedObject("x", PAGE_SIZE, 1, (void*)&x);
		if (x != (uint32*)USER_SHARED_MEM_START) panic("Returned address is not correct. check the setting of it and/or the updating of the shared_mem_free_address");
		if ((freeFrames - sys_calculate_free_frames()) !=  1+1+1) panic("Wrong allocation: make sure that you allocate the required space in the user environment and add its frames to frames_storage");
		sys_freeSharedObject("x");
		if ((freeFrames - sys_calculate_free_frames()) ==  1+1+0) panic("Wrong free: make sure that you remove mapping from user environment and clear the frames_storage of the shared variable");
		else if ((freeFrames - sys_calculate_free_frames()) !=  0) panic("Wrong free: revise your freeSharedObject logic");
	}
	cprintf("Step A completed successfully!!\n\n\n");


	cprintf("STEP B: checking free of 2 shared objects ... \n");
	{
		uint32 *x, *z ;
		int freeFrames = sys_calculate_free_frames() ;
		int ret = sys_createSharedObject("z", PAGE_SIZE, 1, (void*)&z);
		ret = sys_createSharedObject("x", PAGE_SIZE, 1, (void*)&x);
		if(ret != 0) panic("Wrong free: make sure you clear the sharedObject struct in the shares array");

		if ((freeFrames - sys_calculate_free_frames()) !=  2+1+2) panic("Wrong previous free: make sure that you correctly free shared object before (Step A)");

		sys_freeSharedObject("z");
		if ((freeFrames - sys_calculate_free_frames()) !=  1+1+1) panic("Wrong free: check your logic");


		sys_freeSharedObject("x");
		if ((freeFrames - sys_calculate_free_frames()) !=  0) panic("Wrong free: check your logic");

	}
	cprintf("Step B completed successfully!!\n\n\n");

	cprintf("STEP C: checking free loop ranges ... \n");
	{
		uint32 *w, *u;
		int freeFrames = sys_calculate_free_frames() ;
		int ret = sys_createSharedObject("w", 3 * PAGE_SIZE+1, 1, (void*)&w);
		ret = sys_createSharedObject("u", PAGE_SIZE, 1, (void*)&u);
		//if ((freeFrames - sys_calculate_free_frames()) !=  6) panic("Wrong allocation: make sure that you share the required space in both user environment and kernel");
		sys_freeSharedObject("w");
		if ((freeFrames - sys_calculate_free_frames()) !=  1+1+1) panic("Wrong free: check your logic");

//		uint32 *o;
//		int freeFrames = sys_calculate_free_frames() ;
//		int ret = sys_createSharedObject("o", 2 * PAGE_SIZE-1, (void*)&o);
//
//		sys_freeSharedObject("o");
//		if ((freeFrames - sys_calculate_free_frames()) !=  3) panic("Wrong free: check your logic");
	}
	cprintf("Step C completed successfully!!\n\n\n");

	cprintf("Congratulations!! Test of freeSharedObjects [4] completed successfully!!\n\n\n");

	return;
}
