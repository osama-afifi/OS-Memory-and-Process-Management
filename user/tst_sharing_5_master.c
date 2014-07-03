// Test the free of shared variables
#include <inc/lib.h>

void
_main(void)
{
	cprintf("STEP A: checking free of shared object using 2 environments... \n");
	{
		uint32 *x;
		int32 envIdSlave1 = sys_create_env("tshr5slave");
		int32 envIdSlave2 = sys_create_env("tshr5slave");

		int freeFrames = sys_calculate_free_frames() ;
		int ret = sys_createSharedObject("x", PAGE_SIZE, 1, (void*)&x);
		cprintf("Master env created x (1 page) \n");
		if (x != (uint32*)USER_SHARED_MEM_START) panic("Returned address is not correct. check the setting of it and/or the updating of the shared_mem_free_address");
		if ((freeFrames - sys_calculate_free_frames()) !=  1+1+1) panic("Wrong allocation: make sure that you allocate the required space in the user environment and add its frames to frames_storage");

		sys_run_env(envIdSlave1);
		sys_run_env(envIdSlave2);

		cprintf("please be patient ...\n");
		env_sleep(3000);

		sys_freeSharedObject("x");
		cprintf("Master env removed x (1 page) \n");
		int diff = (sys_calculate_free_frames() - freeFrames);
		//cprintf("DIff = %d\n",diff);
		if ( diff !=  0) panic("Wrong free: revise your freeSharedObject logic\n");
	}
	cprintf("Step A completed successfully!!\n\n\n");

	cprintf("STEP B: checking free of 2 shared objects ... \n");
	{
		uint32 *x, *z ;
		int32 envIdSlaveB1 = sys_create_env("tshr5slaveB1");
		int32 envIdSlaveB2 = sys_create_env("tshr5slaveB2");

		int ret = sys_createSharedObject("z", PAGE_SIZE, 1, (void*)&z);
		cprintf("Master env created z (1 page) \n");
		ret = sys_createSharedObject("x", PAGE_SIZE, 1, (void*)&x);
		cprintf("Master env created x (1 page) \n");

		sys_run_env(envIdSlaveB1);
		sys_run_env(envIdSlaveB2);

		env_sleep(1500); //give slaves time to catch the shared object before removal

		int freeFrames = sys_calculate_free_frames() ;

		sys_freeSharedObject("z");
		cprintf("Master env removed z\n");

		sys_freeSharedObject("x");
		cprintf("Master env removed x\n");

		int diff = (sys_calculate_free_frames() - freeFrames);
		//cprintf("DIff = %d\n",diff);

		if (diff !=  1) panic("Wrong free: frames removed not equal 1 !, correct frames to be removed are 1:\nfrom the env: 1 table\nframes_storage of z & x: should NOT cleared yet (still in use!)\n");
	}


	return;
}
