// System call stubs.

#include <inc/syscall.h>
#include <inc/lib.h>

static inline uint32
syscall(int num, uint32 a1, uint32 a2, uint32 a3, uint32 a4, uint32 a5)
{
	uint32 ret;

	// Generic system call: pass system call number in AX,
	// up to five parameters in DX, CX, BX, DI, SI.
	// Interrupt kernel with T_SYSCALL.
	//
	// The "volatile" tells the assembler not to optimize
	// this instruction away just because we don't use the
	// return value.
	// 
	// The last clause tells the assembler that this can
	// potentially change the condition codes and arbitrary
	// memory locations.

	asm volatile("int %1\n"
		: "=a" (ret)
		: "i" (T_SYSCALL),
		  "a" (num),
		  "d" (a1),
		  "c" (a2),
		  "b" (a3),
		  "D" (a4),
		  "S" (a5)
		: "cc", "memory");
	
	return ret;
}

void
sys_cputs(const char *s, uint32 len)
{
	syscall(SYS_cputs, (uint32) s, len, 0, 0, 0);
}

int
sys_cgetc(void)
{
	return syscall(SYS_cgetc, 0, 0, 0, 0, 0);
}

int sys_env_destroy(int32  envid)
{
	return syscall(SYS_env_destroy, envid, 0, 0, 0, 0);
}

int32 sys_getenvid(void)
{
	 return syscall(SYS_getenvid, 0, 0, 0, 0, 0);
}

void sys_env_exit(void)
{
	syscall(SYS_env_exit, 0, 0, 0, 0, 0);
}


int sys_allocate_page(void *va, int perm)
{
	return syscall(SYS_allocate_page, (uint32) va, perm, 0 , 0, 0);
}

int sys_map_frame(int32 srcenv, void *srcva, int32 dstenv, void *dstva, int perm)
{
	return syscall(SYS_map_frame, srcenv, (uint32) srcva, dstenv, (uint32) dstva, perm);
}

int sys_unmap_frame(int32 envid, void *va)
{
	return syscall(SYS_unmap_frame, envid, (uint32) va, 0, 0, 0);
}

uint32 sys_calculate_required_frames(uint32 start_virtual_address, uint32 size)
{
	return syscall(SYS_calc_req_frames, start_virtual_address, (uint32) size, 0, 0, 0);
}

uint32 sys_calculate_free_frames()
{
	return syscall(SYS_calc_free_frames, 0, 0, 0, 0, 0);
}
uint32 sys_calculate_modified_frames()
{
	return syscall(SYS_calc_modified_frames, 0, 0, 0, 0, 0);
}

uint32 sys_calculate_notmod_frames()
{
	return syscall(SYS_calc_notmod_frames, 0, 0, 0, 0, 0);
}

void sys_freeMem(uint32 virtual_address, uint32 size)
{
	syscall(SYS_freeMem, virtual_address, size, 0, 0, 0);
	return;
}

void sys_allocateMem(uint32 virtual_address, uint32 size)
{
	syscall(SYS_allocateMem, virtual_address, size, 0, 0, 0);
	return ;
}

int sys_pf_calculate_allocated_pages()
{
	return syscall(SYS_pf_calc_allocated_pages, 0,0,0,0,0);
}

//NEW !! 2012...
void
sys_disable_interrupt()
{
	syscall(SYS_disableINTR,0, 0, 0, 0, 0);
}


void
sys_enable_interrupt()
{
	syscall(SYS_enableINTR,0, 0, 0, 0, 0);
}


void
sys_cputc(const char c)
{
	syscall(SYS_cputc, (uint32) c, 0, 0, 0, 0);
}


//NEW'12: BONUS2 Testing
void
sys_clear_ffl()
{
	syscall(SYS_clearFFL,0, 0, 0, 0, 0);
}

int
sys_createSemaphore(char* semaphoreName, uint32 initialValue)
{
	return syscall(SYS_create_semaphore,(uint32)semaphoreName, (uint32)initialValue, 0, 0, 0);
}

int
sys_getSemaphoreValue(char* semaphoreName)
{
	return syscall(SYS_get_semaphore_value,(uint32)semaphoreName, 0, 0, 0, 0);
}

void
sys_waitSemaphore(char* semaphoreName)
{
	syscall(SYS_wait_semaphore,(uint32)semaphoreName, 0, 0, 0, 0);
}

void
sys_signalSemaphore(char* semaphoreName)
{
	syscall(SYS_signal_semaphore,(uint32)semaphoreName, 0, 0, 0, 0);
}

int
sys_createSharedObject(char* shareName, uint32 size, uint8 isWritable, void** returned_shared_address)
{
	return syscall(SYS_create_shared_object,(uint32)shareName, (uint32)size, isWritable, (uint32)returned_shared_address,  0);
}



int
sys_getSharedObject(char* shareName, void** returned_shared_address)
{
	return syscall(SYS_get_shared_object,(uint32)shareName, (uint32)returned_shared_address, 0, 0, 0);
}

int
sys_freeSharedObject(char* shareName)
{
	return syscall(SYS_free_shared_object,(uint32)shareName, 0, 0, 0, 0);
}

uint32 	sys_getCurrentSharedAddress()
{
	return syscall(SYS_get_current_shared_address,0, 0, 0, 0, 0);
}

uint32 	sys_getMaxShares()
{
	return syscall(SYS_get_max_shares,0, 0, 0, 0, 0);
}

int
sys_create_env(char* programName)
{
	return syscall(SYS_create_env,(uint32)programName, 0, 0, 0, 0);
}

void
sys_run_env(int32 envId)
{
	syscall(SYS_run_all, (int32)envId, 0, 0, 0, 0);
}

void
sys_free_env(int32 envId)
{
	syscall(SYS_free_env, (int32)envId, 0, 0, 0, 0);
}

struct uint64
sys_get_virtual_time()
{
	struct uint64 result;
	syscall(SYS_get_virtual_time, (uint32)&(result.low), (uint32)&(result.hi), 0, 0, 0);
	return result;
}

// 2014
void sys_moveMem(uint32 src_virtual_address, uint32 dst_virtual_address, uint32 size)
{
	syscall(SYS_moveMem, src_virtual_address, dst_virtual_address, size, 0, 0);
	return ;
}
uint32 sys_rcr2()
{
	return syscall(SYS_rcr2, 0, 0, 0, 0, 0);
}
void sys_bypassPageFault(uint8 instrLength)
{
	syscall(SYS_bypassPageFault, instrLength, 0, 0, 0, 0);
	return ;
}
