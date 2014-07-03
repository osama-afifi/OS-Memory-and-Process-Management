// Test the free of shared variables
#include <inc/lib.h>

void
_main(void)
{
	uint32 *x;
	sys_getSharedObject("x",(void*)&x) ;
	int freeFrames = sys_calculate_free_frames() ;

	cprintf("Slave env used x (getSharedObject)\n");

	sys_freeSharedObject("x");
	cprintf("Slave env removed x\n");

	int diff = (sys_calculate_free_frames() - freeFrames);
	//cprintf("DIff = %d\n",diff);
	if (diff !=  1) panic("wrong free: frames removed not equal 1 !, correct frames to be removed is 1:\nfrom the env: 1 table for x\nframes_storage: not cleared yet\n");
	return;
}
