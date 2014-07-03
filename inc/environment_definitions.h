/* See COPYRIGHT for copyright information. */

#ifndef FOS_INC_ENV_H
#define FOS_INC_ENV_H

#include <inc/types.h>
#include <inc/queue.h>
#include <inc/trap.h>
#include <inc/memlayout.h>

// An environment ID 'envid_t' has three parts:
//
// +1+---------------21-----------------+--------10--------+
// |0|          Uniqueifier             |   Environment    |
// | |                                  |      Index       |
// +------------------------------------+------------------+
//                                       \--- ENVX(eid) --/
//
// The environment index ENVX(eid) equals the environment's offset in the
// 'envs[]' array.  The uniqueifier distinguishes environments that were
// created at different times, but share the same environment index.
//
// All real environments are greater than 0 (so the sign bit is zero).
// envid_ts less than 0 signify errors.


// Values of env_status in struct Env
#define ENV_FREE		0
#define ENV_READY		1
#define ENV_BLOCKED			2
#define ENV_NEW			3
#define ENV_EXIT			4

#define PAGE_WS_MAX_SIZE 	1000	 //7 //100 //8
#define TABLE_WS_MAX_SIZE 	20
#define MAX_MODIFIED_LIST_COUNT 1000

uint32 old_pf_counter;
//uint32 mydblchk;
struct WorkingSetElement {
	uint32 virtual_address;
	uint8 empty;

	//2012
	uint32 time_stamp ;
};

struct Env {
	struct Trapframe env_tf;	// Saved registers
	LIST_ENTRY(Env) prev_next_info;	// Free list link pointers
	int32 env_id;			// Unique environment identifier
	int32 env_parent_id;		// env_id of this env's parent
	unsigned env_status;		// Status of the environment
	uint32 env_runs;		// Number of times environment has run

	// Address space
	uint32 *env_pgdir;		// Kernel virtual address of page dir
	uint32 env_cr3;		// Physical address of page dir
	
	//for page file management
	uint32* env_disk_pgdir;
	
	//for table file management
	uint32* env_disk_tabledir;

	//page working set management
	struct WorkingSetElement pageWorkingSet[PAGE_WS_MAX_SIZE];
	
	//table working set management
	struct WorkingSetElement tableWorkingSet[TABLE_WS_MAX_SIZE];

	uint32 page_last_WS_index;
	uint32 table_last_WS_index;

	uint32 pageFaultsCounter;
	uint32 tableFaultsCounter;

	uint32 nModifiedPages;
	uint32 nNotModifiedPages;

	//Program name (to print it via USER.cprintf in multitasking)
	const char *prog_name ;

	//2014
	uint32 shared_free_address;
};

#define LOG2NENV		10
//#define NENV			(1 << LOG2NENV)
#define NENV			( (PTSIZE/4) / sizeof(struct Env) )
#define ENVX(envid)		((envid) & (NENV - 1))

#endif // !FOS_INC_ENV_H
