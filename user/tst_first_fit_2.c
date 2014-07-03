#include <inc/lib.h>

struct MyStruct
{
	char a;
	short b;
	int c;
};
void _main(void)
{	
	int envID = sys_getenvid();

	volatile struct Env* myEnv;
	myEnv = &(envs[envID]);

	int Mega = 1024*1024;
	int kilo = 1024;
	void* ptr_allocations[20] = {0};

	//[1] Attempt to allocate more than heap size
	{
		ptr_allocations[0] = malloc(USER_HEAP_MAX - USER_HEAP_START + 1);
		if (ptr_allocations[0] != NULL) panic("Malloc: Attempt to allocate more than heap size, should return NULL");
	}
	//[2] Attempt to allocate space more than any available fragment
	//	a) Create Fragments
	{
		//2 MB
		int freeFrames = sys_calculate_free_frames() ;
		ptr_allocations[0] = malloc(2*Mega-kilo);
		if ((uint32) ptr_allocations[0] <  (USER_HEAP_START)) panic("Wrong start address for the allocated space... ");
		if ((freeFrames - sys_calculate_free_frames()) != 512+1 ) panic("Wrong allocation: ");

		//2 MB
		freeFrames = sys_calculate_free_frames() ;
		ptr_allocations[1] = malloc(2*Mega-kilo);
		if ((uint32) ptr_allocations[1] < (USER_HEAP_START + 2*Mega)) panic("Wrong start address for the allocated space... ");
		if ((freeFrames - sys_calculate_free_frames()) != 512 ) panic("Wrong allocation: ");

		//2 KB
		freeFrames = sys_calculate_free_frames() ;
		ptr_allocations[2] = malloc(2*kilo);
		if ((uint32) ptr_allocations[2] < (USER_HEAP_START + 4*Mega)) panic("Wrong start address for the allocated space... ");
		if ((freeFrames - sys_calculate_free_frames()) != 1+1 ) panic("Wrong allocation: ");

		//2 KB
		freeFrames = sys_calculate_free_frames() ;
		ptr_allocations[3] = malloc(2*kilo);
		if ((uint32) ptr_allocations[3] < (USER_HEAP_START + 4*Mega + 4*kilo)) panic("Wrong start address for the allocated space... ");
		if ((freeFrames - sys_calculate_free_frames()) != 1 ) panic("Wrong allocation: ");

		//4 KB Hole
		freeFrames = sys_calculate_free_frames() ;
		free(ptr_allocations[2]);
		if ((sys_calculate_free_frames() - freeFrames) != 1) panic("Wrong free: ");

		//7 KB
		freeFrames = sys_calculate_free_frames() ;
		ptr_allocations[4] = malloc(7*kilo);
		if ((uint32) ptr_allocations[4] < (USER_HEAP_START + 4*Mega + 8*kilo)) panic("Wrong start address for the allocated space... ");
		if ((freeFrames - sys_calculate_free_frames()) != 2)panic("Wrong allocation: ");

		//2 MB Hole
		freeFrames = sys_calculate_free_frames() ;
		free(ptr_allocations[0]);
		if ((sys_calculate_free_frames() - freeFrames) != 512) panic("Wrong free: ");

		//3 MB
		freeFrames = sys_calculate_free_frames() ;
		ptr_allocations[5] = malloc(3*Mega-kilo);
		if ((uint32) ptr_allocations[5] < (USER_HEAP_START + 4*Mega + 16*kilo)) panic("Wrong start address for the allocated space... ");
		if ((freeFrames - sys_calculate_free_frames()) != 3*Mega/4096 ) panic("Wrong allocation: ");

		//2 MB + 6 KB
		freeFrames = sys_calculate_free_frames() ;
		ptr_allocations[6] = malloc(2*Mega + 6*kilo);
		if ((uint32) ptr_allocations[6] < (USER_HEAP_START + 7*Mega + 16*kilo)) panic("Wrong start address for the allocated space... ");
		if ((freeFrames - sys_calculate_free_frames()) != 514+1 ) panic("Wrong allocation: ");

		//3 MB Hole
		freeFrames = sys_calculate_free_frames() ;
		free(ptr_allocations[5]);
		if ((sys_calculate_free_frames() - freeFrames) != 768) panic("Wrong free: ");

		//5 MB
		freeFrames = sys_calculate_free_frames() ;
		ptr_allocations[7] = malloc(5*Mega-kilo);
		if ((uint32) ptr_allocations[7] < (USER_HEAP_START + 9*Mega + 24*kilo)) panic("Wrong start address for the allocated space... ");
		if ((freeFrames - sys_calculate_free_frames()) != 5*Mega/4096 + 1) panic("Wrong allocation: ");

		//2 MB + 8 KB Hole
		freeFrames = sys_calculate_free_frames() ;
		free(ptr_allocations[6]);
		if ((sys_calculate_free_frames() - freeFrames) != 514) panic("Wrong free: ");

		//4 MB
		freeFrames = sys_calculate_free_frames() ;
		ptr_allocations[8] = malloc(4*Mega-kilo);
		if ((uint32) ptr_allocations[8] < (USER_HEAP_START + 4*Mega + 16*kilo)) panic("Wrong start address for the allocated space... ");
		if ((freeFrames - sys_calculate_free_frames()) != 4*Mega/4096) panic("Wrong allocation: ");
	}

	//	b) Attempt to allocate large segment with no suitable fragment to fit on
	{
		//Large Allocation
		int freeFrames = sys_calculate_free_frames() ;
		ptr_allocations[9] = malloc((USER_HEAP_MAX - USER_HEAP_START + 14*Mega));
		if (ptr_allocations[9] != NULL) panic("Malloc: Attempt to allocate large segment with no suitable fragment to fit on, should return NULL");

		cprintf("Congratulations!! test FIRST FIT allocation (2) completed successfully.\n");

		return;
	}
}
