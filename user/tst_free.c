#include <inc/lib.h>

void _main(void)
{	
	int envID = sys_getenvid();

	volatile struct Env* myEnv;
	myEnv = &(envs[envID]);

	int Mega = 1024*1024;
	int kilo = 1024;

	int start_freeFrames = sys_calculate_free_frames() ;

	void* ptr_allocations[20] = {0};
	{
		int freeFrames = sys_calculate_free_frames() ;
		ptr_allocations[0] = malloc(2*Mega-kilo);
		if ((uint32) ptr_allocations[0] <  (USER_HEAP_START)) panic("Wrong start address for the allocated space... check return address of malloc & updating of heap ptr");
		if ((freeFrames - sys_calculate_free_frames()) != 512+1 ) panic("Wrong allocation: ");

		freeFrames = sys_calculate_free_frames() ;
		ptr_allocations[1] = malloc(2*Mega-kilo);
		if ((uint32) ptr_allocations[1] < (USER_HEAP_START + 2*Mega)) panic("Wrong start address for the allocated space... check return address of malloc & updating of heap ptr");
		if ((freeFrames - sys_calculate_free_frames()) != 512 ) panic("Wrong allocation: ");

		freeFrames = sys_calculate_free_frames() ;
		ptr_allocations[2] = malloc(2*kilo);
		if ((uint32) ptr_allocations[2] < (USER_HEAP_START + 4*Mega)) panic("Wrong start address for the allocated space... check return address of malloc & updating of heap ptr");
		if ((freeFrames - sys_calculate_free_frames()) != 1+1 ) panic("Wrong allocation: ");

		freeFrames = sys_calculate_free_frames() ;
		ptr_allocations[3] = malloc(2*kilo);
		if ((uint32) ptr_allocations[3] < (USER_HEAP_START + 4*Mega + 4*kilo)) panic("Wrong start address for the allocated space... check return address of malloc & updating of heap ptr");
		if ((freeFrames - sys_calculate_free_frames()) != 1 ) panic("Wrong allocation: ");

		freeFrames = sys_calculate_free_frames() ;
		ptr_allocations[4] = malloc(7*kilo);
		if ((uint32) ptr_allocations[4] < (USER_HEAP_START + 4*Mega + 8*kilo)) panic("Wrong start address for the allocated space... check return address of malloc & updating of heap ptr");
		if ((freeFrames - sys_calculate_free_frames()) != 2)panic("Wrong allocation: ");

		freeFrames = sys_calculate_free_frames() ;
		ptr_allocations[5] = malloc(3*Mega-kilo);
		if ((uint32) ptr_allocations[5] < (USER_HEAP_START + 4*Mega + 16*kilo)) panic("Wrong start address for the allocated space... check return address of malloc & updating of heap ptr");
		if ((freeFrames - sys_calculate_free_frames()) != 3*Mega/4096 ) panic("Wrong allocation: ");

		freeFrames = sys_calculate_free_frames() ;
		ptr_allocations[6] = malloc(2*Mega-kilo);
		if ((uint32) ptr_allocations[6] < (USER_HEAP_START + 7*Mega + 16*kilo)) panic("Wrong start address for the allocated space... check return address of malloc & updating of heap ptr");
		if ((freeFrames - sys_calculate_free_frames()) != 512+1 ) panic("Wrong allocation: ");
	}

	{
		int freeFrames = sys_calculate_free_frames() ;
		free(ptr_allocations[0]);
		if ((sys_calculate_free_frames() - freeFrames) != 512 ) panic("Wrong free: ");

		freeFrames = sys_calculate_free_frames() ;
		free(ptr_allocations[1]);
		if ((sys_calculate_free_frames() - freeFrames) != 512 + 1 ) panic("Wrong free: ");

		freeFrames = sys_calculate_free_frames() ;
		free(ptr_allocations[2]);
		if ((sys_calculate_free_frames() - freeFrames) != 1 ) panic("Wrong free: ");

		freeFrames = sys_calculate_free_frames() ;
		free(ptr_allocations[3]);
		if ((sys_calculate_free_frames() - freeFrames) != 1 ) panic("Wrong free: ");

		freeFrames = sys_calculate_free_frames() ;
		free(ptr_allocations[4]);
		if ((sys_calculate_free_frames() - freeFrames) != 2 ) panic("Wrong free: ");

		freeFrames = sys_calculate_free_frames() ;
		free(ptr_allocations[5]);
		if ((sys_calculate_free_frames() - freeFrames) != 3*Mega/4096 ) panic("Wrong free: ");

		freeFrames = sys_calculate_free_frames() ;
		free(ptr_allocations[6]);
		if ((sys_calculate_free_frames() - freeFrames) != 512 + 2) panic("Wrong free: ");

		if(start_freeFrames != sys_calculate_free_frames() ) {panic("Wrong free: not all pages removed correctly at end");}
	}

	cprintf("Congratulations!! test free completed successfully.\n");

	return;
}
