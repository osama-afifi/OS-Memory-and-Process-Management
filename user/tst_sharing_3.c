// Test the SPECIAL CASES during the creation of shared variables (create_shared_memory)
#include <inc/lib.h>

void
_main(void)
{
	uint32 *x, *y, *z ;
	cprintf("STEP A: checking creation of shared object that is already exists... \n\n");
	{
		int ret = sys_createSharedObject("x", PAGE_SIZE, 1, (void*)&x);
		int freeFrames = sys_calculate_free_frames() ;
		ret = sys_createSharedObject("x", PAGE_SIZE, 1, (void*)&x);
		if (ret != E_SHARED_MEM_EXISTS) panic("Trying to create an already exists object and corresponding error is not returned!!");
		if ((freeFrames - sys_calculate_free_frames()) !=  0) panic("Wrong allocation: make sure that you don't allocate any memory if the shared object exists");
	}

	cprintf("STEP B: checking the creation of shared object that exceeds the SHARED area limit... \n\n");
	{
		int freeFrames = sys_calculate_free_frames() ;
		uint32 size = USER_SHARED_MEM_MAX - USER_SHARED_MEM_START ;
		int ret = sys_createSharedObject("y", size, 1, (void*)&y);
		if (ret != E_NO_MEM) panic("Trying to create a shared object that exceed the SHARED area limit and the corresponding error is not returned!!");
		if ((freeFrames - sys_calculate_free_frames()) !=  0) panic("Wrong allocation: make sure that you don't allocate any memory if the shared object exceed the SHARED area limit");
	}

	cprintf("STEP C: checking the creation of a number of shared objects that exceeds the MAX ALLOWED NUMBER of OBJECTS... \n\n");
	{
		uint32 maxShares = sys_getMaxShares();
		int i ;
		for (i = 0 ; i < maxShares - 1; i++)
		{
			char shareName[10] ;
			ltostr(i, shareName) ;
			int ret = sys_createSharedObject(shareName, 1, 1, (void*)&z);
			if (ret == E_NO_SHARE) panic("WRONG error returns... supposed no error return yet!!");
		}
		int ret = sys_createSharedObject("outOfBounds", 1, 1, (void*)&z);
		if (ret != E_NO_SHARE) panic("Trying to create a shared object that exceed the number of ALLOWED OBJECTS and the corresponding error is not returned!!");
	}
	cprintf("Congratulations!! Test of Shared Variables [Create: Special Cases] completed successfully!!\n\n\n");

	return;
}
