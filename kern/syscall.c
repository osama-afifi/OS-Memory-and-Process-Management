/* See COPYRIGHT for copyright information. */

#include <inc/x86.h>
#include <inc/error.h>
#include <inc/string.h>
#include <inc/assert.h>

#include <kern/user_environment.h>
#include <kern/memory_manager.h>
#include <kern/file_manager.h>
#include <kern/trap.h>
#include <kern/syscall.h>
#include <kern/console.h>
#include <kern/semaphore_manager.h>
#include <kern/shared_memory_manager.h>
#include <kern/sched.h>

// Print a string to the system console.
// The string is exactly 'len' characters long.
// Destroys the environment on memory errors.
static void sys_cputs(const char *s, uint32 len)
{
	// Check that the user has permission to read memory [s, s+len).
	// Destroy the environment if not.

	// LAB 3: Your code here.

	// Print the string supplied by the user.
	cprintf("[%s %d] %.*s",curenv->prog_name, curenv->env_id, len, s);
	//cprintf("%.*s", len, s);
}


// Print a char to the system console.
static void sys_cputc(const char c)
{
	// Print the char supplied by the user.
	cprintf("%c",c);
}


// Read a character from the system console.
// Returns the character.
static int
sys_cgetc(void)
{
	int c;

	// The cons_getc() primitive doesn't wait for a character,
	// but the sys_cgetc() system call does.
	//while ((c = cons_getc()) == 0)
	//2013: instead of busywaiting inside kernel code , we moved the waiting to user code,
	// this allows any other running user programs to run beacause the clock interrupt works in user mode
	// (the kernel system call by default disables the interrupts, so any loops inside the kernel while
	// handling an interrupt will cause the clock interrupt to be disabled)
	c = cons_getc2();
		/* do nothing */;

	return c;
}


// Returns the current environment's envid.
static int32 sys_getenvid(void)
{
	//return curenv->env_id;
	return (curenv - envs) ;
}

// Destroy a given environment (possibly the currently running environment).
//
// Returns 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist,
//		or the caller doesn't have permission to change envid.
static int sys_env_destroy(int32  envid)
{
	int r;
	struct Env *e;

	if ((r = envid2env(envid, &e, 1)) < 0)
		return r;
	if (e == curenv)
		cprintf("[%08x] exiting gracefully\n", curenv->env_id);
	else
		cprintf("[%08x] destroying %08x\n", curenv->env_id, e->env_id);
	env_destroy(e);
	return 0;
}

static void sys_env_exit()
{
	env_run_cmd_prmpt();
}


// Allocate a page of memory and map it at 'va' with permission
// 'perm' in the address space of 'envid'.
// The page's contents are set to 0.
// If a page is already mapped at 'va', that page is unmapped as a
// side effect.
//
// perm -- PTE_U | PTE_P must be set, PTE_AVAIL | PTE_W may or may not be set,
//         but no other bits may be set.
//
// Return 0 on success, < 0 on error.  Errors are:
//	E_BAD_ENV if environment envid doesn't currently exist,
//		or the caller doesn't have permission to change envid.
//	E_INVAL if va >= UTOP, or va is not page-aligned.
//	E_INVAL if perm is inappropriate (see above).
//	E_NO_MEM if there's no memory to allocate the new page,
//		or to allocate any necessary page tables.
static int sys_allocate_page(void *va, int perm)
{
	// Hint: This function is a wrapper around page_alloc() and
	//   page_insert() from kern/pmap.c.
	//   Most of the new code you write should be to check the
	//   parameters for correctness.
	//   If page_insert() fails, remember to free the page you
	//   allocated!

	int r;
	struct Env *e = curenv;

	//if ((r = envid2env(envid, &e, 1)) < 0)
	//return r;

	struct Frame_Info *ptr_frame_info ;
	r = allocate_frame(&ptr_frame_info) ;
	if (r == E_NO_MEM)
		return r ;

	//check virtual address to be paged_aligned and < USER_TOP
	if ((uint32)va >= USER_TOP || (uint32)va % PAGE_SIZE != 0)
		return E_INVAL;

	//check permissions to be appropriate
	if ((perm & (~PERM_AVAILABLE & ~PERM_WRITEABLE)) != (PERM_USER))
		return E_INVAL;


	uint32 physical_address = to_physical_address(ptr_frame_info) ;

	memset(K_VIRTUAL_ADDRESS(physical_address), 0, PAGE_SIZE);

	r = map_frame(e->env_pgdir, ptr_frame_info, va, perm) ;
	if (r == E_NO_MEM)
	{
		decrement_references(ptr_frame_info);
		return r;
	}
	return 0 ;
}

// Map the page of memory at 'srcva' in srcenvid's address space
// at 'dstva' in dstenvid's address space with permission 'perm'.
// Perm has the same restrictions as in sys_page_alloc, except
// that it also must not grant write access to a read-only
// page.
//
// Return 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if srcenvid and/or dstenvid doesn't currently exist,
//		or the caller doesn't have permission to change one of them.
//	-E_INVAL if srcva >= UTOP or srcva is not page-aligned,
//		or dstva >= UTOP or dstva is not page-aligned.
//	-E_INVAL is srcva is not mapped in srcenvid's address space.
//	-E_INVAL if perm is inappropriate (see sys_page_alloc).
//	-E_INVAL if (perm & PTE_W), but srcva is read-only in srcenvid's
//		address space.
//	-E_NO_MEM if there's no memory to allocate the new page,
//		or to allocate any necessary page tables.
static int sys_map_frame(int32 srcenvid, void *srcva, int32 dstenvid, void *dstva, int perm)
{
	// Hint: This function is a wrapper around page_lookup() and
	//   page_insert() from kern/pmap.c.
	//   Again, most of the new code you write should be to check the
	//   parameters for correctness.
	//   Use the third argument to page_lookup() to
	//   check the current permissions on the page.

	// LAB 4: Your code here.
	panic("sys_map_frame not implemented");
}

// Unmap the page of memory at 'va' in the address space of 'envid'.
// If no page is mapped, the function silently succeeds.
//
// Return 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist,
//		or the caller doesn't have permission to change envid.
//	-E_INVAL if va >= UTOP, or va is not page-aligned.
static int sys_unmap_frame(int32 envid, void *va)
{
	// Hint: This function is a wrapper around page_remove().

	// LAB 4: Your code here.
	panic("sys_page_unmap not implemented");
}

uint32 sys_calculate_required_frames(uint32 start_virtual_address, uint32 size)
{
	return calculate_required_frames(curenv->env_pgdir, start_virtual_address, size); 
}

uint32 sys_calculate_free_frames()
{
	struct freeFramesCounters counters = calculate_available_frames();
	//	cprintf("Free Frames = %d : Buffered = %d, Not Buffered = %d\n", counters.freeBuffered + counters.freeNotBuffered, counters.freeBuffered ,counters.freeNotBuffered);
	return counters.freeBuffered + counters.freeNotBuffered;

}
uint32 sys_calculate_modified_frames()
{
	struct freeFramesCounters counters = calculate_available_frames();
	//	cprintf("================ Modified Frames = %d\n", counters.modified) ;
	return counters.modified;
}

uint32 sys_calculate_notmod_frames()
{
	struct freeFramesCounters counters = calculate_available_frames();
	//	cprintf("================ Not Modified Frames = %d\n", counters.freeBuffered) ;
	return counters.freeBuffered;
}

int sys_pf_calculate_allocated_pages(void)
{
	return pf_calculate_allocated_pages(curenv);
}

void sys_freeMem(uint32 virtual_address, uint32 size)
{
	freeMem(curenv, virtual_address, size);
	return;
}

void sys_allocateMem(uint32 virtual_address, uint32 size)
{
	allocateMem(curenv, virtual_address, size);
	return;
}

//NEW!! 2012...
void sys_disable_interrupt()
{
	curenv->env_tf.tf_eflags &= ~FL_IF ;
}
void sys_enable_interrupt()
{
	curenv->env_tf.tf_eflags |= FL_IF ;
}

void sys_clearFFL()
{
	int size = LIST_SIZE(&free_frame_list) ;
	int i = 0 ;
	struct Frame_Info* ptr_tmp_FI ;
	for (; i < size ; i++)
	{
		allocate_frame(&ptr_tmp_FI) ;
	}
}

int sys_createSemaphore(char* semaphoreName, uint32 initialValue)
{
	return createSemaphore(semaphoreName, initialValue);
}

void sys_waitSemaphore(char* semaphoreName)
{
	waitSemaphore(semaphoreName);
}

void sys_signalSemaphore(char* semaphoreName)
{
	signalSemaphore(semaphoreName);
}

int sys_getSemaphoreValue(char* semaphoreName)
{
	int semNo = getSemaphoreNumber(semaphoreName);
	assert(semNo>=0 && semNo < nSemaphores) ;

	return semaphores[semNo].value ;
}

int sys_createSharedObject(char* shareName, uint32 size, uint8 isWritable, void** returned_shared_address)
{
	return createSharedObject(shareName, size, isWritable, returned_shared_address);
}


int sys_getSharedObject(char* shareName, void** returned_shared_address)
{
	return getSharedObject(shareName, returned_shared_address);
}

int sys_freeSharedObject(char* shareName)
{
	return freeSharedObject(shareName);
}

uint32 sys_getCurrentSharedAddress()
{
	return curenv->shared_free_address;
}

uint32 sys_getMaxShares()
{
	return MAX_SHARES;
}

//=========

int sys_create_env(char* programName)
{
	struct Env* env =  env_create(programName);
	if(env == NULL)
	{
		return E_ENV_CREATION_ERROR;
	}
	return env->env_id;
}

void sys_free_env(int32 envId)
{
	struct Env* e= NULL;
	envid2env(envId, &e, 0);

	if(e == 0) return;

	env_free(e);
}

void sys_run_env(int32 envId)
{
	sched_run_env(envId);
}

struct uint64 sys_get_virtual_time()
{
	struct uint64 t = get_virtual_time();
	return t;
}

//2014
void sys_moveMem(uint32 src_virtual_address, uint32 dst_virtual_address, uint32 size)
{
	moveMem(curenv, src_virtual_address, dst_virtual_address, size);
	return;
}
uint32 sys_rcr2()
{
	return rcr2();
}
void sys_bypassPageFault(uint8 instrLength)
{
	bypassInstrLength = instrLength;
}


// Dispatches to the correct kernel function, passing the arguments.
uint32 syscall(uint32 syscallno, uint32 a1, uint32 a2, uint32 a3, uint32 a4, uint32 a5)
{
	// Call the function corresponding to the 'syscallno' parameter.
	// Return any appropriate return value.
	// LAB 3: Your code here.
	switch(syscallno)
	{
	case SYS_cputs:
		sys_cputs((const char*)a1,a2);
		return 0;
		break;
	case SYS_cgetc:
		return sys_cgetc();
		break;
	case SYS_getenvid:
		return sys_getenvid();
		break;
	case SYS_env_destroy:
		return sys_env_destroy(a1);
		break;
	case SYS_env_exit:
		sys_env_exit();
		return 0;
		break;
	case SYS_calc_req_frames:
		return sys_calculate_required_frames(a1, a2);
		break;
	case SYS_calc_free_frames:
		return sys_calculate_free_frames();
		break;
	case SYS_calc_modified_frames:
		return sys_calculate_modified_frames();
		break;
	case SYS_calc_notmod_frames:
		return sys_calculate_notmod_frames();
		break;

	case SYS_pf_calc_allocated_pages:
		return sys_pf_calculate_allocated_pages();
		break;
	case SYS_freeMem:
		sys_freeMem(a1, a2);
		return 0;
		break;
		//======================
	case SYS_allocate_page:
		sys_allocate_page((void*)a1, a2);
		return 0;
		break;
	case SYS_map_frame:
		sys_map_frame(a1, (void*)a2, a3, (void*)a4, a5);
		return 0;
		break;
	case SYS_unmap_frame:
		sys_unmap_frame(a1, (void*)a2);
		return 0;
		break;
	case SYS_allocateMem:
		//LOG_STATMENT(cprintf("KERNEL syscall: a2 %x\n", a2));
		sys_allocateMem(a1, (uint32)a2);
		return 0;
		break;
	case SYS_disableINTR:
		sys_disable_interrupt();
		return 0;
		break;

	case SYS_enableINTR:
		sys_enable_interrupt();
		return 0;
		break;

	case SYS_cputc:
		sys_cputc((const char)a1);
		return 0;
		break;

	case SYS_clearFFL:
		sys_clearFFL((const char)a1);
		return 0;
		break;

	case SYS_create_semaphore:
		return sys_createSemaphore((char*)a1, a2);
		break;

	case SYS_wait_semaphore:
		sys_waitSemaphore((char*)a1);
		return 0;
		break;

	case SYS_signal_semaphore:
		sys_signalSemaphore((char*)a1);
		return 0;
		break;

	case SYS_get_semaphore_value:
		return sys_getSemaphoreValue((char*)a1);
		break;

	case SYS_create_shared_object:
		return sys_createSharedObject((char*)a1, a2, a3, (void**)a4);
		break;

	case SYS_get_shared_object:
		return sys_getSharedObject((char*)a1, (void**)a2);
		break;

	case SYS_free_shared_object:
		return sys_freeSharedObject((char*)a1);
		break;

	case SYS_get_current_shared_address:
		return sys_getCurrentSharedAddress();
		break;

	case SYS_get_max_shares:
		return sys_getMaxShares();
		break;

	case SYS_create_env:
		return sys_create_env((char*)a1);
		break;

	case SYS_free_env:
		sys_free_env((int32)a1);
		return 0;
		break;

	case SYS_run_all:
		sys_run_env((int32)a1);
		return 0;
		break;

	case SYS_get_virtual_time:
	{
		struct uint64 res = sys_get_virtual_time();
		uint32* ptrlow = ((uint32*)a1);
		uint32* ptrhi = ((uint32*)a2);
		*ptrlow = res.low;
		*ptrhi = res.hi;
		return 0;
		break;
	}
	case SYS_moveMem:
		sys_moveMem(a1, a2, a3);
		return 0;
		break;
	case SYS_rcr2:
		return sys_rcr2();
		break;
	case SYS_bypassPageFault:
		sys_bypassPageFault(a1);
		return 0;

	case NSYSCALLS:
		return 	-E_INVAL;
		break;
	}
	//panic("syscall not implemented");
	return -E_INVAL;
}

