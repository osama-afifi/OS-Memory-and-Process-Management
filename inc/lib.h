// Main public header file for our user-land support library,
// whose code lives in the lib directory.
// This library is roughly our OS's version of a standard C library,
// and is intended to be linked into all user-mode applications
// (NOT the kernel or boot loader).

#ifndef FOS_INC_LIB_H
#define FOS_INC_LIB_H 1

#include <inc/types.h>
#include <inc/stdio.h>
#include <inc/stdarg.h>
#include <inc/string.h>
#include <inc/error.h>
#include <inc/assert.h>
#include <inc/environment_definitions.h>
#include <inc/memlayout.h>
#include <inc/syscall.h>
#include <inc/malloc.h>


#define USED(x)		(void)(x)

// libos.c or entry.S
extern char *binaryname;
extern volatile struct Env *env;
extern volatile struct Env envs[NENV];
extern volatile struct Frame_Info frames_info[];
void	destroy(void);
void	exit(void);

// readline.c
void readline(const char *buf, char*);
void atomic_readline(const char *buf, char*);

// syscall.c
void	sys_cputs(const char *string, uint32 len);
int	sys_cgetc(void);
int32	sys_getenvid(void);
int	sys_env_destroy(int32);
void 	sys_env_exit();
int 	sys_allocate_page(void *va, int perm);
int 	sys_map_frame(int32 srcenv, void *srcva, int32 dstenv, void *dstva, int perm);
int 	sys_unmap_frame(int32 envid, void *va);
uint32 	sys_calculate_required_frames(uint32 start_virtual_address, uint32 size);
uint32 	sys_calculate_free_frames();
uint32 	sys_calculate_modified_frames();
uint32 	sys_calculate_notmod_frames();

void 	sys_freeMem(uint32 virtual_address, uint32 size);
void	sys_allocateMem(uint32 virtual_address, uint32 size);
int 	sys_pf_calculate_allocated_pages(void);
void 	sys_disable_interrupt();
void 	sys_enable_interrupt();

int 	sys_createSemaphore(char* semaphoreName, uint32 initialValue);
void 	sys_waitSemaphore(char* semaphoreName);
void	sys_signalSemaphore(char* semaphoreName);
int		sys_getSemaphoreValue(char* semaphoreName);

int 	sys_createSharedObject(char* shareName, uint32 size, uint8 isWritable, void** returned_shared_address);
int 	sys_getSharedObject(char* shareName, void** returned_shared_address );
int 	sys_freeSharedObject(char* shareName);
uint32 	sys_getCurrentSharedAddress();
uint32 	sys_getMaxShares();

int		sys_create_env(char* programName);
void	sys_free_env(int32 envId);
void	sys_run_env(int32 envId);

void 	sys_cputc(const char c);
uint32	sys_rcr2();
void sys_bypassPageFault(uint8 instrLength);
uint32 sys_calculate_free_frames();
uint32 sys_calculate_modified_frames();
void sys_run_env();
struct uint64 sys_get_virtual_time();


// console.c
void	atomic_cputchar(int c);
void	atomic_cputchar(int c);
int	atomic_getchar(void);
int	atomic_getchar(void);

struct uint64 sys_get_virtual_time();
void env_sleep(uint32 apprxMilliSeconds);
#define CYCLES_PER_MILLISEC 10000

int	iscons(int fd);
int	opencons(void);

/* File open modes */
#define	O_RDONLY	0x0000		/* open for reading only */
#define	O_WRONLY	0x0001		/* open for writing only */
#define	O_RDWR		0x0002		/* open for reading and writing */
#define	O_ACCMODE	0x0003		/* mask for above modes */

#define	O_CREAT		0x0100		/* create if nonexistent */
#define	O_TRUNC		0x0200		/* truncate to zero length */
#define	O_EXCL		0x0400		/* error if already exists */
#define O_MKDIR		0x0800		/* create directory, not regular file */

#endif	// !FOS_INC_LIB_H
