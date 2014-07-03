/* See COPYRIGHT for copyright information. */

#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/assert.h>

#include <kern/command_prompt.h>
#include <kern/console.h>
#include <kern/memory_manager.h>
#include <kern/helpers.h>
#include <kern/kclock.h>
#include <kern/user_environment.h>
#include <kern/trap.h>
#include <kern/picirq.h>
#include <kern/sched.h>


//Functions Declaration 
//======================
void print_welcome_message();
//=======================================



//First ever function called in FOS kernel
void FOS_initialize()
{
	//get actual addresses after code linking 
	extern char start_of_uninitialized_data_section[], end_of_kernel[];

	// Before doing anything else,
	// clear the uninitialized global data (BSS) section of our program, from start_of_uninitialized_data_section to end_of_kernel 
	// This ensures that all static/global variables start with zero value.
	memset(start_of_uninitialized_data_section, 0, end_of_kernel - start_of_uninitialized_data_section);

	// Initialize the console.
	// Can't call cprintf until after we do this!
	console_initialize();

	//print welcome message
	print_welcome_message();

	// Lab 2 memory management initialization functions
	detect_memory();
	initialize_kernel_VM();
	initialize_paging();
//	page_check();

	
	// Lab 3 user environment initialization functions
	env_init();
	idt_init();
	setPageReplacmentAlgorithmLRU();
	enableModifiedBuffer(1) ;

	// Lab 4 multitasking initialization functions
	pic_init();
	kclock_start();
	sched_init() ;

//	int h, c = 0 ;
//	for (h = 0 ; h < 10000 ; h++)
//	{
//		c++;
//	}
//	kclock_stop();
//
//	uint16 cnt0 = kclock_read_cnt0() ;
//	cprintf("Timer After Loop: Counter0 Value = %x\n", cnt0 );

	//cprintf("Int Flag before = %d\n", read_eflags() & FL_IF) ;

	// start the kernel command prompt.
	while (1==1)
	{
		cprintf("\nWelcome to the FOS kernel command prompt!\n");
		cprintf("Type 'help' for a list of commands.\n");	
		run_command_prompt();
	}
}


void print_welcome_message()
{
	cprintf("\n\n\n");
	cprintf("\t\t!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	cprintf("\t\t!!                                                             !!\n");
	cprintf("\t\t!!                   !! FCIS says HELLO !!                     !!\n");
	cprintf("\t\t!!                                                             !!\n");
	cprintf("\t\t!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	cprintf("\n\n\n\n");	
}


/*
 * Variable panicstr contains argument to first call to panic; used as flag
 * to indicate that the kernel has already called panic.
 */
static const char *panicstr;

/*
 * Panic is called on unresolvable fatal errors.
 * It prints "panic: mesg", and then enters the kernel command prompt.
 */
void _panic(const char *file, int line, const char *fmt,...)
{
	va_list ap;

	//if (panicstr)
	//	goto dead;
	panicstr = fmt;

	va_start(ap, fmt);
	cprintf("kernel panic at %s:%d: ", file, line);
	vcprintf(fmt, ap);
	cprintf("\n");
	va_end(ap);

dead:
	/* break into the kernel command prompt */
	//2013: Check if the panic occur when running an environment
	if (curenv != NULL && curenv->env_status == ENV_READY)
	{
		env_run_cmd_prmpt() ;
	}
	while (1==1)
		run_command_prompt();

}

/* like panic, but don't enters the kernel command prompt*/
void _warn(const char *file, int line, const char *fmt,...)
{
	va_list ap;

	va_start(ap, fmt);
	cprintf("kernel warning at %s:%d: ", file, line);
	vcprintf(fmt, ap);
	cprintf("\n");
	va_end(ap);
}

