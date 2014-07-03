// Test the creation of shared variables and using them
// Master program: create the shared variables, initialize them and run slaves
#include <inc/lib.h>

void
_main(void)
{
	uint32 *x, *y, *z ;

	//x: Readonly
	int freeFrames = sys_calculate_free_frames() ;
	int ret = sys_createSharedObject("x", 4, 0, (void*)&x);
	if (x != (uint32*)USER_SHARED_MEM_START) panic("Create(): Returned address is not correct. make sure that you align the allocation on 4KB boundary");
	if ((freeFrames - sys_calculate_free_frames()) !=  1+1+1) panic("Create(): Wrong allocation- make sure that you allocate the required space in the user environment and add its frames to frames_storage");

	//y: Readonly
	freeFrames = sys_calculate_free_frames() ;
	ret = sys_createSharedObject("y", 4, 0, (void*)&y);
	if (y != (uint32*)(USER_SHARED_MEM_START + 1 * PAGE_SIZE)) panic("Create(): Returned address is not correct. make sure that you align the allocation on 4KB boundary");
	if ((freeFrames - sys_calculate_free_frames()) !=  1+0+1) panic("Create(): Wrong allocation- make sure that you allocate the required space in the user environment and add its frames to frames_storage");

	//z: Writable
	freeFrames = sys_calculate_free_frames() ;
	ret = sys_createSharedObject("z", 4, 1, (void*)&z);
	if (z != (uint32*)(USER_SHARED_MEM_START + 2 * PAGE_SIZE)) panic("Create(): Returned address is not correct. make sure that you align the allocation on 4KB boundary");
	if ((freeFrames - sys_calculate_free_frames()) !=  1+0+1) panic("Create(): Wrong allocation- make sure that you allocate the required space in the user environment and add its frames to frames_storage");

	*x = 10 ;
	*y = 20 ;

	int id1, id2, id3;
	id1 = sys_create_env("shr2Slave1");
	id2 = sys_create_env("shr2Slave1");
	id3 = sys_create_env("shr2Slave1");

	sys_run_env(id1);
	sys_run_env(id2);
	sys_run_env(id3);

	env_sleep(3000) ;
	if (*z != 30)
		cprintf("Error!! Please check the creation (or the getting) of shared variables!!\n\n\n");
	else
		cprintf("Congratulations!! Test of Shared Variables [Create & Get] [2] completed successfully!!\n\n\n");

	cprintf("Now, ILLEGAL MEM ACCESS should be occur, due to attempting to write a ReadOnly variable\n\n\n");

	id1 = sys_create_env("shr2Slave2");

	env_sleep(3000) ;

	sys_run_env(id1);

	return;	
}
