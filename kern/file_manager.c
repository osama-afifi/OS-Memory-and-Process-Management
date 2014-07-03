/* See COPYRIGHT for copyright information. */
/*
KEY WORDS
==========
MACROS: 	K_PHYSICAL_ADDRESS, K_VIRTUAL_ADDRESS, PDX, PTX, CONSTRUCT_ENTRY, EXTRACT_ADDRESS, ROUNDUP, ROUNDDOWN, LIST_INIT, LIST_INSERT_HEAD, LIST_FIRST, LIST_REMOVE
CONSTANTS:	PAGE_SIZE, PERM_PRESENT, PERM_WRITEABLE, PERM_USER, KERNEL_STACK_TOP, KERNEL_STACK_SIZE, KERNEL_BASE, READ_ONLY_FRAMES_INFO, PHYS_IO_MEM, PHYS_EXTENDED_MEM, E_NO_MEM
VARIABLES:	ptr_free_mem, ptr_disk_page_directory, phys_page_directory, phys_stack_bottom, Frame_Info, frames_info, disk_free_frame_list, references, prev_next_info, size_of_extended_mem, number_of_frames, ptr_frame_info ,create, perm, va
FUNCTIONS:	to_physical_address, get_frame_info, tlb_invalidate
=====================================================================================================================================================================================================
*/
/// ==========================================================================
/// THIS PAGE FILE MANAGMENT DOES NOT SUPPORT MEMORY SHARING !
/// ==========================================================================

#include <inc/mmu.h>
#include <inc/error.h>
#include <inc/string.h>
#include <inc/assert.h>
#include <inc/disk.h> 
#include <inc/environment_definitions.h>

#include <kern/file_manager.h>
#include <kern/memory_manager.h>


int read_disk_page(uint32 dfn, void* va)
{

	uint32 df_start_sector = PAGE_FILE_START_SECTOR+dfn*SECTOR_PER_PAGE;

	//LOG_STATMENT( cprintf("reading from disk to mem addr %x at sector %d\n",va,df_start_sector);  );
	int success = ide_read(df_start_sector, (void*)va, SECTOR_PER_PAGE);
	//LOG_STATMENT( if(success==0) {cprintf("read from disk successuflly.\n");} else {cprintf("read from disk failed !!\n");} );

	return success;	 
}


int write_disk_page(uint32 dfn, void* va)
{
	//write disk at wanted frame
	uint32 df_start_sector = PAGE_FILE_START_SECTOR+dfn*SECTOR_PER_PAGE;

	//LOG_STATMENT( cprintf(">>> writing to disk from mem addr %x at sector %d\n",va,df_start_sector);  );
	int success = ide_write(df_start_sector, (void*)va, SECTOR_PER_PAGE);
	//LOG_STATMENT( if(success==0) {cprintf(">>> written to disk successfully.\n");} else {cprintf(">>> written to disk failed !!\n");} );

	if(success != 0)
		panic("Error writing on disk\n");
	return success;
}

///========================== PAGE FILE MANAGMENT ==============================

uint32* ptr_disk_page_directory;

struct Frame_Info* disk_frames_info;	
struct Linked_List disk_free_frame_list;

void initialize_disk_page_file();

int read_disk_page(uint32 dfn, void* va); 
int write_disk_page(uint32 dfn, void* va);

int get_disk_page_directory(struct Env* ptr_env, uint32** ptr_disk_table_directory);




// --------------------------------------------------------------
// Tracking of physical frames.
// The 'frames_info' array has one 'struct Frame_Info' entry per physical frame.
// frames_info are reference counted, and free frames are kept on a linked list.
// --------------------------------------------------------------

// Initialize paging structure and disk_free_frame_list.
// After this point, ONLY use the functions below
// to allocate and deallocate physical memory via the disk_free_frame_list,
// and NEVER use boot_allocate_space() or the related boot-time functions above.
//
void initialize_disk_page_file()
{
	int i;
	LIST_INIT(&disk_free_frame_list);

	//LOG_STATMENT(cprintf("PAGES_PER_FILE = %d, PAGE_FILE_START_SECTOR = %d\n",PAGES_PER_FILE,PAGE_FILE_START_SECTOR););
	for (i = 1; i < PAGES_PER_FILE; i++)
	{
		initialize_frame_info(&(disk_frames_info[i]));

		//disk_frames_info[i].references = 0;
		LIST_INSERT_HEAD(&disk_free_frame_list, &disk_frames_info[i]);
	}
}

//
// Initialize a Frame_Info structure.
// The result has null links and 0 references.
// Note that the corresponding physical frame is NOT initialized!
//
static inline uint32 to_disk_frame_number(struct Frame_Info *ptr_frame_info)
{
	return ptr_frame_info - disk_frames_info;
}

//
// Allocates a disk frame.
//
// *ptr_frame_info -- is set to point to the Frame_Info struct of the
// newly allocated frame
//
// RETURNS 
//   0 -- on success
//   E_NO_PAGE_FILE_SPACE -- otherwise
//
int allocate_disk_frame(uint32 *dfn)
{
	// Fill this function in	
	struct Frame_Info *ptr_frame_info = LIST_FIRST(&disk_free_frame_list);
	if(ptr_frame_info == NULL)
		return E_NO_PAGE_FILE_SPACE;

	LIST_REMOVE(&disk_free_frame_list, ptr_frame_info);
	initialize_frame_info(ptr_frame_info);
	*dfn = to_disk_frame_number(ptr_frame_info); 
	return 0;
}

//
// Return a frame to the disk_free_frame_list.
//
inline void free_disk_frame(uint32 dfn)
{
	// Fill this function in
	if(dfn == 0) return;
	LIST_INSERT_HEAD(&disk_free_frame_list, &disk_frames_info[dfn]);
}

int get_disk_page_table(uint32 *ptr_disk_page_directory, const void *virtual_address, int create, uint32 **ptr_disk_page_table)
{
	// Fill this function in
	uint32 disk_page_directory_entry = ptr_disk_page_directory[PDX(virtual_address)];
	*ptr_disk_page_table = K_VIRTUAL_ADDRESS(EXTRACT_ADDRESS(disk_page_directory_entry)) ;

	if (disk_page_directory_entry == 0)
	{
		//LOG_STATMENT(cprintf("get_disk_page_table: disk directory at %x",ptr_disk_page_directory));
		//LOG_STATMENT(cprintf("get_disk_page_table: page table not found "));
		if (create)
		{
			struct Frame_Info* ptr_frame_info;			
			allocate_frame(&ptr_frame_info) ;
			
			//LOG_STATMENT(cprintf("created table"));
			uint32 phys_page_table = to_physical_address(ptr_frame_info);
			*ptr_disk_page_table = K_VIRTUAL_ADDRESS(phys_page_table) ;
	
			//initialize new page table by 0's
			memset(*ptr_disk_page_table , 0, PAGE_SIZE);

			ptr_frame_info->references = 1;
			ptr_disk_page_directory[PDX(virtual_address)] = CONSTRUCT_ENTRY(phys_page_table,PERM_PRESENT);
			//LOG_STATMENT(cprintf("get_disk_page_table: disk directory entry # %d (VA = %x) is %x ",PDX(virtual_address),
			//virtual_address, ptr_disk_page_directory[PDX(virtual_address)]));
		}
		else
		{
			//LOG_STATMENT(cprintf("NOT creating table ..."));
			*ptr_disk_page_table = 0;
			return 0;
		}
	}
	//LOG_STATMENT(cprintf("found table at %x", *ptr_disk_page_table));
	return 0;
}

int pf_add_empty_env_page( struct Env* ptr_env, uint32 virtual_address)
{
//	return pf_add_env_page(ptr_env, virtual_address, ptr_zero_page);

	uint32 *ptr_disk_page_table;
	assert((uint32)virtual_address < KERNEL_BASE);

	get_disk_page_directory(ptr_env, &(ptr_env->env_disk_pgdir)) ;

	get_disk_page_table(ptr_env->env_disk_pgdir, (void*) virtual_address, 1, &ptr_disk_page_table) ;

	uint32 dfn=ptr_disk_page_table[PTX(virtual_address)];
	if( dfn == 0)
	{
		if( allocate_disk_frame(&dfn) == E_NO_PAGE_FILE_SPACE) return E_NO_PAGE_FILE_SPACE;
		ptr_disk_page_table[PTX(virtual_address)] = dfn;
	}

	return 0;

}

int pf_add_env_page( struct Env* ptr_env, uint32 virtual_address, void* dataSrc)
{
	//LOG_STRING("========================== create_env_page");
	uint32 *ptr_disk_page_table;
	assert((uint32)virtual_address < KERNEL_BASE);
	
	get_disk_page_directory(ptr_env, &(ptr_env->env_disk_pgdir)) ;

	get_disk_page_table(ptr_env->env_disk_pgdir, (void*) virtual_address, 1, &ptr_disk_page_table) ;
	
	uint32 dfn=ptr_disk_page_table[PTX(virtual_address)];
	if( dfn == 0)
	{
		if( allocate_disk_frame(&dfn) == E_NO_PAGE_FILE_SPACE) return E_NO_PAGE_FILE_SPACE;
		ptr_disk_page_table[PTX(virtual_address)] = dfn;
	}

	//TODOO: we should here lcr3 with the env pgdir to make sure that dataSrc is not read mistakenly
	// from another env directory
//	uint32 oldDir = rcr3();
//	lcr3(K_PHYSICAL_ADDRESS(ptr_env->env_pgdir));
//	int ret = write_disk_page(dfn, (void*)dataSrc);
//	lcr3(oldDir);

	int ret = write_disk_page(dfn, (void*)dataSrc);
	return ret;
}

int pf_update_env_page(struct Env* ptr_env, void *virtual_address, struct Frame_Info* modified_page_frame_info)
{
	uint32 *ptr_disk_page_table;
	//ROUND DOWN it on 4 KB boundary in order to update the entire page starting from its first address.
	//virtual_address = ROUNDDOWN(virtual_address, PAGE_SIZE);

	assert((uint32)virtual_address < KERNEL_BASE);
	//char c = *((char*)virtual_address);
	//Get/Create the directory table
	get_disk_page_directory(ptr_env, &(ptr_env->env_disk_pgdir)) ;

	get_disk_page_table(ptr_env->env_disk_pgdir, virtual_address, 0, &ptr_disk_page_table);
	if(ptr_disk_page_table == 0) return E_PAGE_NOT_EXIST_IN_PF;

	uint32 dfn=ptr_disk_page_table[PTX(virtual_address)];
	if( dfn == 0) return E_PAGE_NOT_EXIST_IN_PF;

	//TODOO: we should here lcr3 with the env pgdir to make sure that VA of the modified page
	// is not read mistakenly from another env directory
//	uint32 oldDir = rcr3();
//	lcr3(K_PHYSICAL_ADDRESS(ptr_env->env_pgdir));
//	int ret = write_disk_page(dfn, K_VIRTUAL_ADDRESS(to_physical_address(modified_page_frame_info)));
//	lcr3(oldDir);

	//cprintf("[%s] updating page\n",ptr_env->prog_name);
	int ret = write_disk_page(dfn, K_VIRTUAL_ADDRESS(to_physical_address(modified_page_frame_info)));
	//cprintf("[%s] finished updating page\n",ptr_env->prog_name);
	return ret;
}
/*
int pf_special_update_env_modified_page(struct Env* ptr_env, uint32 virtual_address, struct Frame_Info* page_modified_frame_info)
{
	uint32 *ptr_disk_page_table;
	assert((uint32)virtual_address < KERNEL_BASE);
	//char c = *((char*)virtual_address);
	//Get/Create the directory table
	get_disk_page_directory(ptr_env, &(ptr_env->env_disk_pgdir)) ;

	get_disk_page_table(ptr_env->env_disk_pgdir, (void*)virtual_address, 0, &ptr_disk_page_table);
	if(ptr_disk_page_table == 0) return E_PAGE_NOT_EXIST_IN_PF;

	uint32 dfn=ptr_disk_page_table[PTX(virtual_address)];
	if( dfn == 0) return E_PAGE_NOT_EXIST_IN_PF;

	return write_disk_page(dfn, K_VIRTUAL_ADDRESS(to_physical_address(page_modified_frame_info)));
}
*/
int pf_read_env_page(struct Env* ptr_env, void *virtual_address)
{
	uint32 *ptr_disk_page_table;

	//ROUND DOWN it on 4 KB boundary in order to read the entire page starting from its first address.
	virtual_address = (void*) ROUNDDOWN((uint32)virtual_address, PAGE_SIZE);

	if( ptr_env->env_disk_pgdir == 0) return E_PAGE_NOT_EXIST_IN_PF;

	get_disk_page_table(ptr_env->env_disk_pgdir, virtual_address, 0, &ptr_disk_page_table);
	if(ptr_disk_page_table == 0) return E_PAGE_NOT_EXIST_IN_PF;

	uint32 dfn=ptr_disk_page_table[PTX(virtual_address)];

	if( dfn == 0) return E_PAGE_NOT_EXIST_IN_PF;

	int disk_read_error = read_disk_page(dfn, virtual_address);

	//reset modified bit to 0: because FOS copies the placed or replaced page from
	//HD to memory, the page modified bit is set to 1, but we want the modified bit to be
	// affected only by "user code" modifications, not our (FOS kernel) modifications
	pt_set_page_permissions(curenv, (uint32)virtual_address, 0, PERM_MODIFIED);

	return disk_read_error;
}

void pf_remove_env_page(struct Env* ptr_env, uint32 virtual_address)
{
	//LOG_STRING("pf_remove_env_page: 0");
	uint32 *ptr_disk_page_table;	

	//LOG_STATMENT(cprintf("ptr_env = %x",ptr_env));
	if( ptr_env->env_disk_pgdir == 0) return;

	//LOG_STRING("pf_remove_env_page: 1");
	get_disk_page_table(ptr_env->env_disk_pgdir, (void*)virtual_address, 0, &ptr_disk_page_table);
	if(ptr_disk_page_table == 0) return;

	//LOG_STRING("pf_remove_env_page: 2");
	uint32 dfn=ptr_disk_page_table[PTX(virtual_address)];
	ptr_disk_page_table[PTX(virtual_address)] = 0;
	free_disk_frame(dfn);
	//LOG_STRING("pf_remove_env_page: 3");
}

void pf_free_env(struct Env* ptr_env)
{
	uint32 *pt;
	uint32 pdeno;
	uint32 pa;

	for (pdeno = 0; pdeno < PDX(USER_TOP) ; pdeno++) 
	{
		// only look at mapped page tables
		if (!(ptr_env->env_disk_pgdir[pdeno] & PERM_PRESENT))
			continue;

		// find the pa and va of the page table
		pa = EXTRACT_ADDRESS(ptr_env->env_disk_pgdir[pdeno]);
		pt = (uint32*) K_VIRTUAL_ADDRESS(pa);

		// unmap all PTEs in this page table
		uint32 pteno;
		for (pteno = 0; pteno < 1024; pteno++) 
		{
			// remove the disk page from disk page table 
			uint32 dfn=pt[pteno];
			pt[pteno] = 0;
			// and declare it free
			free_disk_frame(dfn);
		}

		// free the disk page table itself
		ptr_env->env_disk_pgdir[pdeno] = 0;
		decrement_references(to_frame_info(pa));	
	}

	// free the disk page directory of the environment
	pa = K_PHYSICAL_ADDRESS(ptr_env->env_disk_pgdir);
	ptr_env->env_disk_pgdir = 0;
	decrement_references(to_frame_info(pa));


	// remove all tables and the disk table
	if (ptr_env->env_disk_tabledir == 0)
		return;
	pf_remove_env_all_tables(ptr_env);
	pa = K_PHYSICAL_ADDRESS(ptr_env->env_disk_tabledir);
	ptr_env->env_disk_tabledir = 0;
	decrement_references(to_frame_info(pa));
}


int get_disk_page_directory(struct Env* ptr_env, uint32** ptr_disk_table_directory)
{
	*ptr_disk_table_directory = ptr_env->env_disk_pgdir; 
	if(*ptr_disk_table_directory == 0)
	{
		//	LOG_STATMENT(cprintf(">>>>>>>>>>>>>> disk directory not found, creating one ...\n"););
		int r;
		struct Frame_Info *p = NULL;

		if ((r = allocate_frame(&p)) < 0)
			return r;
		p->references = 1;

		//[4] initialize the new environment by the virtual address of the page directory 
		// Hint: use "initialize_environment" function		
		*ptr_disk_table_directory = K_VIRTUAL_ADDRESS(to_physical_address(p));
		memset(*ptr_disk_table_directory , 0, PAGE_SIZE);

		//	LOG_STATMENT(cprintf(">>>>>>>>>>>>>> Disk directory created at %x", *ptr_disk_page_directory));
	}
	return 0;
}

int pf_calculate_allocated_pages(struct Env* ptr_env)
{
	uint32 *pt;
	uint32 pdIndex;
	uint32 pa;
	uint32 counter=0;

	for (pdIndex = 0; pdIndex < PDX(USER_TOP) ; pdIndex++) 
	{
		// only look at mapped page tables
		if (!(ptr_env->env_disk_pgdir[pdIndex] & PERM_PRESENT))
			continue;

		// find the pa and va of the page table
		pa = EXTRACT_ADDRESS(ptr_env->env_disk_pgdir[pdIndex]);
		pt = (uint32*) K_VIRTUAL_ADDRESS(pa);

		// unmap all PTEs in this page table
		uint32 ptIndex;
		for (ptIndex = 0; ptIndex < 1024; ptIndex++) 
		{
			// remove the disk page from disk page table 
			uint32 dfn=pt[ptIndex];
			if(dfn != 0)
				counter ++;
		}
	}

	return counter;
}
///========================== END OF PAGE FILE MANAGMENT =============================



/*========================== TABLE FILE MANAGMENT ==============================*/
int get_disk_table_directory(struct Env* ptr_env, uint32** ptr_disk_table_directory)
{
	*ptr_disk_table_directory = ptr_env->env_disk_tabledir;
	if(*ptr_disk_table_directory == 0)
	{
		//	LOG_STATMENT(cprintf(">>>>>>>>>>>>>> disk directory not found, creating one ...\n"););
		int r;
		struct Frame_Info *p = NULL;

		if ((r = allocate_frame(&p)) < 0)
			return r;
		p->references = 1;

		//[4] initialize the new environment by the virtual address of the page directory
		// Hint: use "initialize_environment" function
		*ptr_disk_table_directory = K_VIRTUAL_ADDRESS(to_physical_address(p));
		memset(*ptr_disk_table_directory , 0, PAGE_SIZE);

		//	LOG_STATMENT(cprintf(">>>>>>>>>>>>>> Disk directory created at %x", *ptr_disk_page_directory));
	}
	return 0;
}

int pf_write_env_table( struct Env* ptr_env, uint32 virtual_address, uint32* tableKVirtualAddress)
{
	//LOG_STRING("========================== create_env_page");
	assert((uint32)virtual_address < KERNEL_BASE);

	get_disk_table_directory(ptr_env, &(ptr_env->env_disk_tabledir)) ;

	uint32 dfn=ptr_env->env_disk_tabledir[PDX(virtual_address)];
	if( dfn == 0)
	{
		if( allocate_disk_frame(&dfn) == E_NO_PAGE_FILE_SPACE) return E_NO_PAGE_FILE_SPACE;
		ptr_env->env_disk_tabledir[PDX(virtual_address)] = dfn;
	}


	//TODOO: we should here lcr3 with the env pgdir to make sure that tableKVirtualAddress is not read mistakenly
	// from another env directory
//	uint32 oldDir = rcr3();
//	lcr3(K_PHYSICAL_ADDRESS(ptr_env->env_pgdir));
//	int ret = write_disk_page(dfn, (void*)tableKVirtualAddress);
//	lcr3(oldDir);

	//cprintf("[%s] writing table\n",ptr_env->prog_name);
	int ret = write_disk_page(dfn, (void*)tableKVirtualAddress);
	//cprintf("[%s] finished writing table\n",ptr_env->prog_name);
	return ret;
}

int pf_read_env_table(struct Env* ptr_env, uint32 virtual_address, uint32* tableKVirtualAddress)
{
	if( ptr_env->env_disk_tabledir == 0) return E_TABLE_NOT_EXIST_IN_PF;

	uint32 dfn=ptr_env->env_disk_tabledir[PDX(virtual_address)];

	if( dfn == 0) return E_TABLE_NOT_EXIST_IN_PF;

	int disk_read_error = read_disk_page(dfn, tableKVirtualAddress);

	return disk_read_error;
}

void pf_remove_env_all_tables(struct Env* ptr_env)
{
	//LOG_STRING("pf_remove_env_page: 0");

	//LOG_STATMENT(cprintf("ptr_env = %x",ptr_env));
	if( ptr_env->env_disk_tabledir == 0) return;

	uint32 pdeno;
	for (pdeno = 0; pdeno < PDX(USER_TOP) ; pdeno++)
	{
		uint32 dfn=ptr_env->env_disk_tabledir[pdeno];
		ptr_env->env_disk_tabledir[pdeno] = 0;
		free_disk_frame(dfn);
	}
	//LOG_STRING("pf_remove_env_page: 3");
}

void pf_remove_env_table(struct Env* ptr_env, uint32 virtual_address)
{
	if (virtual_address == 0)
			cprintf("REMOVING table 0 from page file\n");
	if( ptr_env->env_disk_tabledir == 0) return;

	uint32 dfn=ptr_env->env_disk_tabledir[PDX(virtual_address)];
	ptr_env->env_disk_tabledir[PDX(virtual_address)] = 0;
	free_disk_frame(dfn);
}
///========================== END OF TABLE FILE MANAGMENT =============================


void test_disk_01(void *virtual_address)
{
	LOG_STATMENT(cprintf("doing tests for ide_write()\n"));
	int i=90157;
	for(;i<140000;i += 500)
	{			
		if(ide_write(i,(void *)virtual_address,8) != 0)
		{
			LOG_STATMENT(cprintf("FAILURE to write sector %d\n",i););
			break;
		}
		else
		{
			//LOG_STATMENT(cprintf("written at sector %d\n",i););
		}
	}
	LOG_STATMENT(cprintf("ide_write() test done\n"););
}
