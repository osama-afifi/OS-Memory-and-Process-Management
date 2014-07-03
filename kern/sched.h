/* See COPYRIGHT for copyright information. */

#ifndef FOS_KERN_SCHED_H
#define FOS_KERN_SCHED_H
#ifndef FOS_KERNEL
# error "This is a FOS kernel header; user programs should not #include it"
#endif

#include <inc/environment_definitions.h>


//2012
// This function does not return.
void fos_scheduler(void) __attribute__((noreturn));

void sched_init();
void clock_interrupt_handler();
void update_WS_time_stamps();
void sched_insert_ready(struct Env* env);
void sched_remove_ready(struct Env* env);
void sched_print_all();
void sched_run_all();
void sched_run_env(uint32 envId);
void sched_insert_new(struct Env* env);
void sched_remove_new(struct Env* env);

#endif	// !FOS_KERN_SCHED_H
