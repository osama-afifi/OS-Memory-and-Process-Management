#ifndef FOS_KERN_MONITOR_H
#define FOS_KERN_MONITOR_H
#ifndef FOS_KERNEL
# error "This is a FOS kernel header; user programs should not #include it"
#endif

// Function to activate the kernel command prompt
void run_command_prompt();

// Declaration of functions that implement command prompt commands.

int command_create_tables(int , char **);
int command_free_chunk(int , char **);
int command_count_mapped(int , char **);
int command_copy_page(int , char **);
int command_alloc_user_mem(int , char **);
int command_free_page(int , char **);
int command_alloc_page(int , char **);
int command_remove_table(int , char **);
int command_share_4M(int , char **);
int command_cut_paste_page(int , char **);
int command_show_mappings(int , char **);
int command_connect_va(int , char **);
int command_share_pa(int , char **);
int command_set_permission(int , char **);
int command_show_mapping(int , char **);
int command_help(int , char **);
int command_kernel_info(int , char **);
int command_calc_space(int number_of_arguments, char **arguments);
int command_run_program(int argc, char **argv);
int command_allocpage(int , char **);
int command_writeusermem(int , char **);
int command_readusermem(int , char **);
int command_meminfo(int , char **);
int command_kill(int , char **);
int command_run(int , char **);

#endif	// !FOS_KERN_MONITOR_H
