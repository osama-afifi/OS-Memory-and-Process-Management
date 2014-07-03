#include <inc/assert.h>

#include <kern/sched.h>
#include <kern/user_environment.h>
#include <kern/memory_manager.h>
#include <kern/command_prompt.h>
#include <kern/trap.h>

//void on_clock_update_WS_time_stamps();

///Local Vars
//============
struct Env_list env_ready_queue;	// queue of all running envs
struct Env_list env_new_queue;	// queue of all new envs

struct Env* sched_next = NULL;

struct Env* sched_next_circular(struct Env* env);

void
fos_scheduler(void)
{
	// Implement simple round-robin scheduling.
	// Pick next environment in ready queue,
	// in circular fashion starting after the previously running env,
	// and switch to the first such environment found.
	// It's OK to choose the previously running env if no other env
	// is runnable.

	struct Env* current_env = NULL;

	{
		//we have a runnable env, so update the sched pointer and run it
		current_env = sched_next;
		sched_next = sched_next_circular(sched_next);
	}

	//cprintf("Scheduler\n");
	if(current_env != NULL)
	{
		//cprintf("sched: %s\n",current_env->prog_name);
		env_run(current_env);
	} // Run the special idle environment when nothing else is runnable.
	else //if (envs[0].env_status == ENV_RUNNABLE)
		//	env_run(&envs[0]);
		//else
	{
		//cprintf("[sched] no envs - nothing more to do!\n");
		while (1)
			run_command_prompt(NULL);

	}
}

/*
void
fos_scheduler(void)
{
	// Implement simple round-robin scheduling.
	// Pick next environment in ready queue,
	// in circular fashion starting after the previously running env,
	// and switch to the first such environment found.
	// It's OK to choose the previously running env if no other env
	// is runnable.

	uint32 env_found = 0;
	struct Env* current_env = NULL;

	if (isRunSingleTask())
	{
		if(curenv)
		{
			env_found  =1 ;
			current_env = curenv ;
		}
	}
	if (isRunMultiTask())
	{
		if (current_env == NULL)
			cprintf("current_env = NULL\n") ;
		else
		{
			cprintf("current_env = %s\n", current_env->prog_name) ;
		}

		if(current_env == NULL)
		{
			current_env = LIST_FIRST(&env_ready_queue);
		}
		else
		{
			current_env = LIST_NEXT(current_env);
		}



		while(!env_found)
		{
			if (current_env == NULL)
				cprintf("current_env = NULL\n") ;
			else
				cprintf("current_env = %s\n", current_env->prog_name) ;

			if(current_env == sched_last_run_env)
			{
				if(current_env != NULL && current_env->env_status==ENV_READY)
				{
					env_found=1;
				}
				break;
			}

			if(current_env != NULL)
			{
				if(current_env->env_status == ENV_READY)
				{
					env_found = 1;
					break;
				}
				current_env = LIST_NEXT(current_env);
			}
			else
			{
				current_env = LIST_FIRST(&env_ready_queue);
			}

		}

		//we have a runnable env, so update the sched pointer and run it



	}
	//cprintf("Scheduler\n");
	if(env_found)
	{
		sched_last_run_env = current_env;
		cprintf("sched: %s\n",current_env->prog_name);
		env_run(current_env);
	} // Run the special idle environment when nothing else is runnable.
	else //if (envs[0].env_status == ENV_RUNNABLE)
		//	env_run(&envs[0]);
		//else
	{
		//cprintf("Destroyed all environments - nothing more to do!\n");
		while (1)
			run_command_prompt(NULL);

	}
}
 */

void sched_init()
{
	old_pf_counter = 0;
	LIST_INIT(&env_ready_queue);
	LIST_INIT(&env_new_queue);
	sched_next = NULL;
}

struct Env* sched_next_circular(struct Env* env)
{
	if(env == NULL) return NULL;

	struct Env* next = LIST_NEXT(env);
	if(next == NULL) next = LIST_FIRST(&env_ready_queue);

	return next;
}

void sched_insert_ready(struct Env* env)
{
	if(env != NULL)
	{
		env->env_status = ENV_READY ;
		if(sched_next != NULL)
		{
			LIST_INSERT_BEFORE(&env_ready_queue, sched_next, env);
		}
		else
		{
			LIST_INSERT_TAIL(&env_ready_queue, env);
			sched_next = env;
		}
	}
}

void sched_remove_ready(struct Env* env)
{
	if(env != NULL)
	{
		if(env == sched_next)
		{
			sched_next = sched_next_circular(env);
			if(sched_next == env)
			{
				sched_next = NULL;
			}
		}
		LIST_REMOVE(&env_ready_queue, env);
	}
}

void sched_print_all()
{
	struct Env* ptr_env ;
	if (!LIST_EMPTY(&env_new_queue))
	{
		cprintf("The processes in new queue are:\n");
		LIST_FOREACH(ptr_env, &env_new_queue)
		{
			cprintf("[%d] %s\n", ptr_env->env_id, ptr_env->prog_name);
		}
	}
	else
	{
		cprintf("No processes in new queue\n");
	}
	if (!LIST_EMPTY(&env_ready_queue))
	{
		cprintf("The processes in ready queue are:\n");
		LIST_FOREACH(ptr_env, &env_ready_queue)
		{
			cprintf("[%d] %s\n", ptr_env->env_id, ptr_env->prog_name);
		}
	}
	else
	{
		cprintf("No processes in ready queue\n");
	}
}

void sched_run_all()
{
	struct Env* ptr_env=NULL;
	LIST_FOREACH(ptr_env, &env_new_queue)
	{
		sched_remove_new(ptr_env);
		sched_insert_ready(ptr_env);
	}
}

void sched_run_env(uint32 envId)
{
	struct Env* ptr_env=NULL;
	LIST_FOREACH(ptr_env, &env_new_queue)
	{
		if(ptr_env->env_id == envId)
		{
			sched_remove_new(ptr_env);
			sched_insert_ready(ptr_env);
			break;
		}
	}
}

void sched_insert_new(struct Env* env)
{
	if(env != NULL)
	{
		env->env_status = ENV_NEW ;
		LIST_INSERT_TAIL(&env_new_queue, env);
	}
}
void sched_remove_new(struct Env* env)
{
	if(env != NULL)
	{
		LIST_REMOVE(&env_new_queue, env) ;
	}
}


void clock_interrupt_handler()
{
	if(isPageReplacmentAlgorithmLRU())
	{
		update_WS_time_stamps();
	}
	//cprintf("Clock Handler\n") ;
	fos_scheduler();
}
/*
void on_clock_update_WS_time_stamps()
{
	//cprintf("Updating time stamps\n");

	struct Env *curr_env_ptr = NULL;

	LIST_FOREACH(curr_env_ptr, &env_ready_queue)
	{
		int i ;
		for (i = 0 ; i < PAGE_WS_MAX_SIZE; i++)
		{
			if( curr_env_ptr->pageWorkingSet[i].empty != 1)
			{
				//update the time if the page was referenced
				uint32 page_va = curr_env_ptr->pageWorkingSet[i].virtual_address ;
				uint32 perm = pt_get_page_permissions(curr_env_ptr, page_va) ;
				if (perm & PERM_USED)
				{
					curr_env_ptr->pageWorkingSet[i].time_stamp = time ;
					pt_set_page_permissions(curr_env_ptr, page_va, 0 , PERM_USED) ;
				}
			}
		}

		{
			//uint32 eflags_val = read_eflags();
			//write_eflags(eflags_val & (~FL_IF));

			//if((curr_env_ptr->pageFaultsCounter-old_pf_counter) > 0)
			{
				//cprintf("[%s]: last clock # of PFs  = %d, timenow = %d\n", curr_env_ptr->prog_name ,curr_env_ptr->pageFaultsCounter-old_pf_counter, time);
			}
			//mydblchk += curr_env_ptr->pageFaultsCounter-old_pf_counter;
			old_pf_counter = curr_env_ptr->pageFaultsCounter;

			//eflags_val = read_eflags();
			//write_eflags(eflags_val | (FL_IF));
		}
	}

}
 */
void update_WS_time_stamps()
{
	struct Env *curr_env_ptr = curenv;

	if(curr_env_ptr != NULL)
	{
		{
			int i ;
			for (i = 0 ; i < PAGE_WS_MAX_SIZE; i++)
			{
				if( curr_env_ptr->pageWorkingSet[i].empty != 1)
				{
					//update the time if the page was referenced
					uint32 page_va = curr_env_ptr->pageWorkingSet[i].virtual_address ;
					uint32 perm = pt_get_page_permissions(curr_env_ptr, page_va) ;
					uint32 oldTimeStamp = curr_env_ptr->pageWorkingSet[i].time_stamp;

					if (perm & PERM_USED)
					{
						curr_env_ptr->pageWorkingSet[i].time_stamp = (oldTimeStamp>>2) | 0x80000000;
						pt_set_page_permissions(curr_env_ptr, page_va, 0 , PERM_USED) ;
					}
					else
					{
						curr_env_ptr->pageWorkingSet[i].time_stamp = (oldTimeStamp>>2);
					}
				}
			}
		}

		{
			int t ;
			for (t = 0 ; t < TABLE_WS_MAX_SIZE; t++)
			{
				if( curr_env_ptr->tableWorkingSet[t].empty != 1)
				{
					//update the time if the page was referenced
					uint32 table_va = curr_env_ptr->tableWorkingSet[t].virtual_address;
					uint32 oldTimeStamp = curr_env_ptr->tableWorkingSet[t].time_stamp;

					if (pd_is_table_used(curr_env_ptr, table_va))
					{
						curr_env_ptr->tableWorkingSet[t].time_stamp = (oldTimeStamp>>2) | 0x80000000;
						pd_set_table_unused(curr_env_ptr, table_va);
					}
					else
					{
						curr_env_ptr->tableWorkingSet[t].time_stamp = (oldTimeStamp>>2);
					}
				}
			}
		}
	}
}


