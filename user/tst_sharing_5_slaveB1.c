// Test the free of shared variables
#include <inc/lib.h>

void
_main(void)
{
	uint32 *x;
	sys_getSharedObject("x",(void*)&x) ;
	cprintf("Slave B1 env used x (getSharedObject)\n");

	cprintf("Slave B1 please be patient ...\n");

	env_sleep(3000);
	int freeFrames = sys_calculate_free_frames() ;

	sys_freeSharedObject("x");
	cprintf("Slave B1 env removed x\n");

	if ((sys_calculate_free_frames() - freeFrames) !=  3) panic("B1 wrong free: frames removed not equal 3 !, correct frames to be removed are 3:\nfrom the env: 1 table and 1 for frame of x\nframes_storage of x: should be cleared now\n");
	return;
}

