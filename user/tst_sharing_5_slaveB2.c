// Test the free of shared variables
#include <inc/lib.h>

void
_main(void)
{
	uint32 *z;
	sys_getSharedObject("z",(void*)&z) ;
	cprintf("Slave B2 env used z (getSharedObject)\n");

	cprintf("Slave B2 please be patient ...\n");

	env_sleep(5000);
	int freeFrames = sys_calculate_free_frames() ;

	sys_freeSharedObject("z");
	cprintf("Slave B2 env removed z\n");

	if ((sys_calculate_free_frames() - freeFrames) !=  3) panic("wrong free: frames removed not equal 3 !, correct frames to be removed are 3:\nfrom the env: 1 table + 1 frame for z\nframes_storage of z: should be cleared now\n");


	cprintf("Step B completed successfully!!\n\n\n");
	cprintf("Congratulations!! Test of freeSharedObjects [5] completed successfully!!\n\n\n");

	return;
}
