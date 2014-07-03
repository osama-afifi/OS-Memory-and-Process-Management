/*	Simple command-line kernel prompt useful for
	controlling the kernel and exploring the system interactively.


KEY WORDS
==========
CONSTANTS:	WHITESPACE, NUM_OF_COMMANDS
VARIABLES:	Command, commands, name, description, function_to_execute, number_of_arguments, arguments, command_string, command_line, command_found
FUNCTIONS:	readline, cprintf, execute_command, run_command_prompt, command_kernel_info, command_help, strcmp, strsplit, start_of_kernel, start_of_uninitialized_data_section, end_of_kernel_code_section, end_of_kernel
=====================================================================================================================================================================================================
*/

#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/memlayout.h>
#include <inc/assert.h>
#include <inc/x86.h>


#include <kern/console.h>
#include <kern/command_prompt.h>
#include <kern/memory_manager.h>
#include <kern/trap.h>
#include <kern/kdebug.h>
#include <kern/user_environment.h>

#define	PROGRAM_SEGMENT_FOREACH(Seg, ptr_program_start)					\
		struct ProgramSegment* first; \
		struct ProgramSegment tmp; \
		tmp = (PROGRAM_SEGMENT_FIRST(ptr_program_start));	 \
		first = &tmp; \
		if(first->segment_id == -1) first = NULL;\
		Seg = first; \
		for (;				\
		Seg;							\
		Seg = PROGRAM_SEGMENT_NEXT(Seg,ptr_program_start) )


//Structure for each command
struct Command 
{
	char *name;
	char *description;
	// return -1 to force command prompt to exit
	int (*function_to_execute)(int number_of_arguments, char** arguments);
};

//Functions Declaration
int execute_command(char *command_string);
int command_writemem(int number_of_arguments, char **arguments);
int command_readmem(int number_of_arguments, char **arguments);
int command_readblock(int number_of_arguments, char **arguments);

//Array of commands. (initialized)
struct Command commands[] = 
{
	{ "help", "Display this list of commands", command_help },
	{ "kernel_info", "Display information about the kernel", command_kernel_info },
	{ "writemem", "writes one byte to specific location in given user program" ,command_writemem},
	{ "readmem", "reads one byte from specific location in given user program" ,command_readmem},
	{ "readblock", "reads block of bytes from specific location in given user program" ,command_readblock},
	{ "alloc_page", "allocate single page at the given user virtual address" ,command_allocpage},
	{ "rum", "read single byte at the given user virtual address" ,command_readusermem},
	{ "wum", "write single byte at the given user virtual address" ,command_writeusermem},
	{ "meminfo", "show information about the physical memory" ,command_meminfo},
	{ "showmap", "Lab4.HandsOn", command_show_mapping},
	{ "setperm", "Lab4.HandsOn", command_set_permission},
	{ "sharepa", "Lab4.HandsOn", command_share_pa},
	{ "connect_va", "Connect VA to Physical", command_connect_va},
	{ "show_mappings", "Show all mappings", command_show_mappings},
	{ "cut_paste_page", "cut and paste page", command_cut_paste_page},
	{ "share_4M", " share_4M_readonly ", command_share_4M},
	{ "remove_table", " remove table ", command_remove_table},
	{ "alloc_page", " Allocate Page ", command_alloc_page},
	{ "free_page", " Free Page ", command_free_page},
	{ "alloc_user_mem", " Allocate User Memory ", command_alloc_user_mem},
	{ "copy_page", " copy page ", command_copy_page},
	{ "count_mapped", " count mapped ", command_count_mapped},
	{ "free_chunk", " free chunk ", command_free_chunk},
	{ "create_tables", " create tables ", command_create_tables},
	{ "run", "Lab6.HandsOn: Load and Run User Program", command_run},
	{ "kill", "Lab6.HandsOn: Kill User Program", command_kill},

};

//Number of commands = size of the array / size of command structure
#define NUM_OF_COMMANDS (sizeof(commands)/sizeof(commands[0]))

unsigned read_eip();


//invoke the command prompt
void run_command_prompt()
{
	char command_line[1024];

	while (1==1) 
	{
		//get command line
		readline("FOS> ", command_line);

		//parse and execute the command
		if (command_line != NULL)
			if (execute_command(command_line) < 0)
				break;
	}
}

/***** Kernel command prompt command interpreter *****/

//define the white-space symbols 
#define WHITESPACE "\t\r\n "

//Function to parse any command and execute it 
//(simply by calling its corresponding function)
int execute_command(char *command_string)
{
	// Split the command string into whitespace-separated arguments
	int number_of_arguments;
	//allocate array of char * of size MAX_ARGUMENTS = 16 found in string.h
	char *arguments[MAX_ARGUMENTS];


	strsplit(command_string, WHITESPACE, arguments, &number_of_arguments) ;
	if (number_of_arguments == 0)
		return 0;

	// Lookup in the commands array and execute the command
	int command_found = 0;
	int i ;
	for (i = 0; i < NUM_OF_COMMANDS; i++)
	{
		if (strcmp(arguments[0], commands[i].name) == 0)
		{
			command_found = 1;
			break;
		}
	}

	if(command_found)
	{
		int return_value;
		return_value = commands[i].function_to_execute(number_of_arguments, arguments);			
		return return_value;
	}
	else
	{
		//if not found, then it's unknown command
		cprintf("Unknown command '%s'\n", arguments[0]);
		return 0;
	}
}

/***** Implementations of basic kernel command prompt commands *****/
uint32 current_address=0;
int command_alloc_user_mem(int number_of_arguments, char** arguments)
{
	int size = strtol(arguments[1], NULL, 10); // 10 not 16
	if (arguments[2][0] =='m')
		size *= 1024 * 1024;
	else if (arguments[2][0] =='k')
		size *= 1024;
	else
		return 0;
	uint32 start_address =ROUNDDOWN(current_address,PAGE_SIZE);
	uint32 end_address = ROUNDUP(current_address+size, PAGE_SIZE);
	current_address = (uint32) end_address;
	struct Frame_Info *ptr_frame_info = NULL;
	cprintf("start : %x || end : %x || %x\n",start_address,end_address,current_address);
    for (start_address; start_address < end_address; start_address += PAGE_SIZE)
    {
    	allocate_frame(&ptr_frame_info);
    	map_frame(ptr_page_directory, ptr_frame_info, (uint32*)start_address, PERM_USER | PERM_WRITEABLE);
    	//tlb_invalidate(ptr_page_directory, (uint32*)start_address);
    }
    tlbflush();
	return 0;
}



int command_run(int number_of_arguments, char **arguments)
{
	//[1] Create and initialize a new environment for the program to be run
	struct UserProgramInfo* ptr_program_info = env_create(arguments[1]);
	if(ptr_program_info == 0) return 0;

	//[2] Run the created environment using "env_run" function
	env_run(ptr_program_info->environment);
	return 0;
}


int command_kill(int number_of_arguments, char **arguments)
{
	//[1] Get the user program info of the program (by searching in the "userPrograms" array
	struct UserProgramInfo* ptr_program_info = get_user_program_info(arguments[1]) ;
	if(ptr_program_info == 0) return 0;

	//[2] Kill its environment using "env_free" function
	env_free(ptr_program_info->environment);
	ptr_program_info->environment = NULL;
	return 0;
}


int command_create_tables(int number_of_arguments, char **arguments)
 {
	/*
	int size[256] = { 0 };
	size['K'] = 1024;
	size['M'] = 1024 * 1024;

	unsigned char* start = (unsigned char*) (~((1 << 22) - 1) & strtol(arguments[1], NULL, 16));
	unsigned char* end = (unsigned char *) ((uint32) strtol(arguments[1], NULL, 16)
			+ strtol(arguments[2], NULL, 10) * size[(int) arguments[3][0]]);
	cprintf("%x\n",end);
	for (; start < end; start += (4*1024*1024))
	{
		if(ptr_page_directory[PDX(start)]&PERM_PRESENT)
			continue;
		cprintf("%x\n",start);
		struct Frame_Info *ptr;
		allocate_frame(&ptr);
		map_frame(ptr_page_directory,ptr,start,PERM_PRESENT|PERM_USER);
		unmap_frame(ptr_page_directory, start);
	}
	return 0 ;*/

		uint32 *va =(uint32 *)  strtol(arguments[1], NULL, 16);
		int x = strtol(arguments[2], NULL, 10) , i;
		if (arguments[3][0] == 'M') {
			x *= 1024*1024;
		}
		//x*=1024;
		uint32 *fva = (uint32*)((uint32)va + (uint32)x), *sva = ROUNDDOWN(va , 4*1024*1024);

		for(i=0 ; sva <= fva ; ++i)
		{
			uint32 *ptr_table = NULL;
			get_page_table(ptr_page_directory, (void*) sva, 1, &ptr_table);
			sva += (1024 *1024);
			//cprintf("va= %x \nsva= %x \nfva= %x\n" , va , sva , fva);
		}
		return 0;

}
int command_free_chunk(int number_of_arguments, char **arguments)
{
	int size[256]={0};size['K']=1024;size['M']=1024*1024;
	unsigned char* start = (unsigned char* )(~((1<<12)-1) & strtol(arguments[1], NULL, 16));
	unsigned char* end= (unsigned char *)(strtol(arguments[1], NULL, 16)
			+strtol(arguments[2], NULL, 10)*size[(int)arguments[3][0]]);
	unsigned char* tables[1024],table_len=0;
	//cprintf("%x\n%x\n",start,end);
	for(;start<end;start+=PAGE_SIZE)
	{
		if((uint32)start == (~((1<<22)-1) & (uint32)start))
			tables[table_len++]= start;
		unmap_frame(ptr_page_directory,start);
	}
	int i=0;
	//cprintf("len : %u\n",table_len);
	uint32* ptr;
	for(;i<table_len;++i)
	{
		get_page_table(ptr_page_directory,tables[i],0,&ptr);
		int j=0;
		uint32* tmp=0;
		cprintf("%u\n",get_frame_info(ptr_page_directory,ptr,&tmp)->references);
		for(;j<1024;++j)
			if(ptr[j]&PERM_PRESENT)
				break;
		//cprintf("%u\n",j);
		if(j==1024)
			unmap_frame(ptr_page_directory,ptr);
	}
	tlbflush();
	return 0;
}
int command_count_mapped(int number_of_arguments, char **arguments)
{
	uint32* va = (uint32 *)(~((1<<12)-1) & strtol(arguments[1], NULL, 16));
	uint32* ptr_page =NULL;
	get_page_table(ptr_page_directory,va,0,&ptr_page);
	//cprintf("%x\n%x\n",ptr_page,PDX(va));
	if(ptr_page==NULL)
		cprintf("No Table Exists\n");
	//else if(ptr_page_directory[PDX(va)]&PERM_PRESENT)
		//cprintf("0 mapped page found.\n");
	else
	{
		int co=0,i=0;
		for(;i<1024;++i)
			if(ptr_page[i]&PERM_PRESENT)
				co++;
		cprintf("%d mapped page found\n",co);
	}
	return 0;
}
int command_copy_page(int number_of_arguments, char **arguments)
{
	uint32* va1 = (uint32 *)(~((1<<12)-1) & strtol(arguments[1], NULL, 16));
	uint32* va2 = (uint32 *)(~((1<<12)-1) & strtol(arguments[2], NULL, 16));
	uint32* ptr;
	get_page_table(ptr_page_directory,va2,0,&ptr);
	if(ptr==NULL || !(ptr[PTX(va2)]&PERM_PRESENT))
	{
		cprintf("creating\n");
		struct Frame_Info *ptr_frame_info1, *ptr_frame_info2;
		allocate_frame(&ptr_frame_info2);
		map_frame(ptr_page_directory, ptr_frame_info2, va2, PERM_WRITEABLE
				| PERM_USER);
	}
	int i=0;
	for(;i<1024;++i)
		va2[i]=va1[i];
	return 0;
}
uint32 alloc_start = 0,cur_va=0;
int command_alloc_user_memA(int number_of_arguments, char **arguments)
{
	int size[256]={0};size['K']=1024;size['M']=1024*1024;
	unsigned char* start = (unsigned char *)alloc_start;
	unsigned char* end= (unsigned char *)(cur_va
			+strtol(arguments[1], NULL, 10)*size[(int)arguments[2][0]]);
		if (alloc_start < (uint32)end)
	{
		for (; start < end; start += PAGE_SIZE)
		{
			struct Frame_Info * ptr_frame_info;
			allocate_frame(&ptr_frame_info);
			map_frame(ptr_page_directory, ptr_frame_info, start, PERM_WRITEABLE
					| PERM_USER);
		}
		alloc_start = (uint32) start;
	}
	cur_va = (uint32) end;
	tlbflush();
	return 0;
}
int command_free_page(int number_of_arguments, char **arguments)
{
	unsigned char *va = (unsigned char *)strtol(arguments[1], NULL, 16) ;
	unmap_frame(ptr_page_directory, va);

	return 0;
}
int command_alloc_page(int number_of_arguments, char **arguments)
{
	uint32 address = strtol(arguments[1], NULL, 16);
	unsigned char *va = (unsigned char *) (address);

	//[1]  Allocate a single frame from the free frame list
	struct Frame_Info * ptr_frame_info;
	int ret = allocate_frame(&ptr_frame_info);
	if (ret == E_NO_MEM)
	{
		cprintf("ERROR: no enough memory\n");
		return 0;
	}
	// [2] Map this frame to the given user virtual address
	map_frame(ptr_page_directory, ptr_frame_info, va, PERM_WRITEABLE
			| PERM_USER);
	return 0;
}

int command_remove_table(int number_of_arguments, char **arguments)
{
	uint32 *va1 = (uint32 *) strtol(arguments[1], NULL, 16);
	ptr_page_directory[PDX(va1)] &= ~(PERM_PRESENT);
	tlbflush();
	return 0;
}
int command_share_4M(int number_of_arguments, char **arguments)
{
	uint32 *va1 = (uint32 *) strtol(arguments[1], NULL, 16);
	uint32 *va2 = (uint32 *) strtol(arguments[2], NULL, 16);
	ptr_page_directory[PDX(va2)] = ptr_page_directory[PDX(va1)];
	ptr_page_directory[PDX(va2)] &= ~(PERM_WRITEABLE);
	tlbflush();
	return 0;
}
int command_cut_paste_page(int number_of_arguments, char **arguments)
{
	uint32 *va1 = (uint32 *) strtol(arguments[1], NULL, 16);
	uint32 *ptr_page_table1 = NULL;
	get_page_table(ptr_page_directory, va1, 0, &ptr_page_table1);
	if (ptr_page_table1 != NULL)
	{
		uint32 *va2 = (uint32 *) strtol(arguments[2], NULL, 16);
		uint32 *ptr_page_table2 = NULL;
		get_page_table(ptr_page_directory, va2, 1, &ptr_page_table2);
		ptr_page_table2[PTX(va2)] = ptr_page_table1[PTX(va1)];
		ptr_page_table1[PTX(va1)] &= ~(PERM_PRESENT);
		tlbflush();
	}
	return 0;
}
int command_show_mappings(int number_of_arguments, char **arguments)
{
	cprintf("DIR Index \t PAGE Table Index \t Physical Address \t Modified\n");
	uint32 *start = (uint32 *)(~((1<<12)-1) & strtol(arguments[1], NULL, 16));
	uint32 *end = (uint32 *)strtol(arguments[2], NULL, 16);
	for(;start<=end ;start+=1<<10)
	{
		int dir_index = PDX(start);
		uint32 *ptr_page_table = NULL;
		get_page_table(ptr_page_directory, start, 1, &ptr_page_table);
		if (ptr_page_table != NULL)
		{
			uint32 table_index = PTX(start);
			uint32 frame_num = ptr_page_table[table_index]>>12;
			uint32 tmp = (uint32)start;
			cprintf("\n%x\n",tmp);
			char M_bit = ptr_page_table[table_index] & (PERM_MODIFIED) ? 'Y' : 'N';
			uint32 phy = frame_num<<12 | ((tmp<<20)>>20);


			cprintf("%08u \t\t%08u \t\t\t%08x \t\t%c\n",dir_index,table_index,phy,M_bit);
		}
	}
	return 0;
}

int command_connect_va(int number_of_arguments, char **arguments)
{
	uint32 *va = (uint32 *)strtol(arguments[1], NULL, 16);
	uint32 ph = (uint32) strtol(arguments[2], NULL, 16);
	uint32 *ptr_page_table= NULL;
	get_page_table(ptr_page_directory, va, 1, &ptr_page_table);
	uint32 table_index = PTX(va);
	if(ptr_page_table!=NULL)
	{
		uint32 frame_num = ph>>12;
		ptr_page_table[table_index] = frame_num<<12;
		ptr_page_table[table_index] |= PERM_PRESENT;
		if (arguments[3][0]=='r')
			ptr_page_table[table_index] &= ~PERM_WRITEABLE;
		else
			ptr_page_table[table_index] |= PERM_WRITEABLE;
		tlbflush();
	}

	return 0;
}

int command_show_mapping(int number_of_arguments, char **arguments)
{
	uint32 *va = (uint32 *)strtol(arguments[1], NULL, 16) ;
	uint32 *ptr_page_table = NULL;
	get_page_table(ptr_page_directory, va, 0, &ptr_page_table) ;
	if (ptr_page_table != NULL)
	{
		int dir_index = PDX(va);
		int table_index = PTX(va);
		uint32 fn = ptr_page_table[table_index] >> 12;
		cprintf("DIR Index = %d\nTable Index = %d\nFrame Number = %0d\n", dir_index, table_index, fn) ;

	}
	return 0 ;
}
int command_set_permission(int number_of_arguments, char **arguments)
{
	uint32 *va = (uint32 *)strtol(arguments[1], NULL, 16) ;
	uint32 *ptr_page_table = NULL;
	get_page_table(ptr_page_directory, va, 0, &ptr_page_table) ;
	if (ptr_page_table != NULL)
	{
		char perm = arguments[2][0];
		int table_index = PTX(va) ;

		if (perm == 'r')
		{
			ptr_page_table[table_index] &= (~PERM_WRITEABLE);
		}
		else if (perm == 'w')
		{
			ptr_page_table[table_index] |= (PERM_WRITEABLE);
		}
		//tlb_invalidate(ptr_page_directory, va); // delete the cache of the given address
		tlbflush() ; // delete the whole cache
	}

	return 0 ;
}

int command_share_pa(int number_of_arguments, char **arguments)
{
	uint32 *va1 = (uint32 *)strtol(arguments[1], NULL, 16) ;
	uint32 *ptr_page_table1 = NULL;
	get_page_table(ptr_page_directory, va1, 0, &ptr_page_table1) ;
	if (ptr_page_table1 != NULL)
	{
		uint32 *va2 = (uint32 *)strtol(arguments[2], NULL, 16) ;
		uint32 *ptr_page_table2 = NULL;
		get_page_table(ptr_page_directory, va2, 1, &ptr_page_table2) ;
		ptr_page_table2[PTX(va2)] = ptr_page_table1[PTX(va1)];
	}
	return 0;
}

//print name and description of each command
int command_help(int number_of_arguments, char **arguments)
{
	int i;
	for (i = 0; i < NUM_OF_COMMANDS; i++)
		cprintf("%s - %s\n", commands[i].name, commands[i].description);

	cprintf("-------------------\n");

	return 0;
}

//print information about kernel addresses and kernel size
int command_kernel_info(int number_of_arguments, char **arguments )
{
	extern char start_of_kernel[], end_of_kernel_code_section[], start_of_uninitialized_data_section[], end_of_kernel[];

	cprintf("Special kernel symbols:\n");
	cprintf("  Start Address of the kernel 			%08x (virt)  %08x (phys)\n", start_of_kernel, start_of_kernel - KERNEL_BASE);
	cprintf("  End address of kernel code  			%08x (virt)  %08x (phys)\n", end_of_kernel_code_section, end_of_kernel_code_section - KERNEL_BASE);
	cprintf("  Start addr. of uninitialized data section 	%08x (virt)  %08x (phys)\n", start_of_uninitialized_data_section, start_of_uninitialized_data_section - KERNEL_BASE);
	cprintf("  End address of the kernel   			%08x (virt)  %08x (phys)\n", end_of_kernel, end_of_kernel - KERNEL_BASE);
	cprintf("Kernel executable memory footprint: %d KB\n",
		(end_of_kernel-start_of_kernel+1023)/1024);
	return 0;
}

int command_writemem(int number_of_arguments, char **arguments)
{
	char* user_program_name = arguments[1];
	int address = strtol(arguments[3], NULL, 16);

	struct UserProgramInfo* ptr_user_program_info = get_user_program_info(user_program_name);
	if(ptr_user_program_info == NULL) return 0;

	uint32 oldDir = rcr3();
	lcr3((uint32) K_PHYSICAL_ADDRESS( ptr_user_program_info->environment->env_pgdir));

	unsigned char *ptr = (unsigned char *)(address) ;

	//Write the given Character
	*ptr = arguments[2][0];
	lcr3(oldDir);

	return 0;
}

int command_readmem(int number_of_arguments, char **arguments)
{
	char* user_program_name = arguments[1];
	int address = strtol(arguments[2], NULL, 16);

	struct UserProgramInfo* ptr_user_program_info = get_user_program_info(user_program_name);
	if(ptr_user_program_info == NULL) return 0;

	uint32 oldDir = rcr3();
	lcr3((uint32) K_PHYSICAL_ADDRESS( ptr_user_program_info->environment->env_pgdir));

	unsigned char *ptr = (unsigned char *)(address) ;

	//Write the given Character
	cprintf("value at address %x = %c\n", address, *ptr);

	lcr3(oldDir);
	return 0;
}

int command_readblock(int number_of_arguments, char **arguments)
{
	char* user_program_name = arguments[1];
	int address = strtol(arguments[2], NULL, 16);
	int nBytes = strtol(arguments[3], NULL, 10);

	unsigned char *ptr = (unsigned char *)(address) ;
	//Write the given Character

	struct UserProgramInfo* ptr_user_program_info = get_user_program_info(user_program_name);
	if(ptr_user_program_info == NULL) return 0;

	uint32 oldDir = rcr3();
	lcr3((uint32) K_PHYSICAL_ADDRESS( ptr_user_program_info->environment->env_pgdir));

	int i;
	for(i = 0;i<nBytes; i++)
	{
		cprintf("%08x : %02x  %c\n", ptr, *ptr, *ptr);
		ptr++;
	}
	lcr3(oldDir);

	return 0;
}

int command_allocpage(int number_of_arguments, char **arguments)
{
	unsigned int address = strtol(arguments[1], NULL, 16);
	unsigned char *ptr = (unsigned char *)(address) ;

	struct Frame_Info * ptr_frame_info ;
	allocate_frame(&ptr_frame_info);

	map_frame(ptr_page_directory, ptr_frame_info, ptr, PERM_WRITEABLE|PERM_USER);

	return 0;
}


int command_readusermem(int number_of_arguments, char **arguments)
{
	unsigned int address = strtol(arguments[1], NULL, 16);
	unsigned char *ptr = (unsigned char *)(address) ;

	cprintf("value at address %x = %c\n", ptr, *ptr);

	return 0;
}
int command_writeusermem(int number_of_arguments, char **arguments)
{
	unsigned int address = strtol(arguments[1], NULL, 16);
	unsigned char *ptr = (unsigned char *)(address) ;

	*ptr = arguments[2][0];

	return 0;
}

int command_meminfo(int number_of_arguments, char **arguments)
{
	cprintf("Free frames = %d\n", calculate_free_frames());
	//cprintf("a:%u\n",ptr_UserPrograms[0].environment->env_status);
	int i;
	bool Prompt=0;
	for(i=0;i<NUM_USER_PROGS;++i)
	{
		if (ptr_UserPrograms[i].environment!=NULL) {
			if(!Prompt && (Prompt=1))
				cprintf("Program Name \t Env. ID \t Main Size(KB) \t Tables Size(KB)\n");
			cprintf("%14s \t %4u",ptr_UserPrograms[i].name,ptr_UserPrograms[i].environment->env_id);
			uint32 *ptr_Dir = ptr_UserPrograms[i].environment->env_pgdir;
			uint32 last = USER_TOP / (1024 * PAGE_SIZE);
			int j, size = 0, pageTable = 0;

			for (j = 0; j < last; ++j)
				if (ptr_Dir[j] & PERM_PRESENT) {
					pageTable++;
					uint32 *pTable, ptr_DirValue = (uint32) ptr_Dir;
					get_page_table(ptr_Dir, (uint32*)(j<<22), 0, &pTable);
					int k;
					for (k = 0; k < 1024; ++k)
						if (pTable[k] & PERM_PRESENT)
							size++;
				}
			cprintf(" \t %10d \t %10d\n", size*4, (pageTable+1)*4);
		}
	}
	return 0;
}
