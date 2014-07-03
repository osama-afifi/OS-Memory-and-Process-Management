#ifndef FOS_INC_SYSCALL_H
#define FOS_INC_SYSCALL_H

/* system call numbers */
enum
{
	SYS_cputs = 0,
	SYS_cgetc,
	SYS_getenvid,
	SYS_env_destroy,
	SYS_env_exit,
	SYS_allocate_page,
	SYS_map_frame,
	SYS_unmap_frame,
	SYS_calc_req_frames,
	SYS_calc_free_frames,
	SYS_calc_modified_frames,
	SYS_calc_notmod_frames,
	SYS_pf_calc_allocated_pages,
	SYS_freeMem,
	SYS_allocateMem,
	SYS_moveMem,
	SYS_disableINTR,
	SYS_enableINTR,
	SYS_cputc,
	SYS_clearFFL,
	SYS_create_semaphore,
	SYS_wait_semaphore,
	SYS_signal_semaphore,
	SYS_get_semaphore_value,
	SYS_create_shared_object,
	SYS_get_shared_object,
	SYS_free_shared_object,
	SYS_get_current_shared_address,
	SYS_get_max_shares,
	SYS_create_env,
	SYS_run_all,
	SYS_free_env,
	SYS_get_virtual_time,
	SYS_rcr2,
	SYS_bypassPageFault,
	NSYSCALLS
};

#endif /* !FOS_INC_SYSCALL_H */
