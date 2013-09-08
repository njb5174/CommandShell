/* CMPSC 311 Project 6 toolkit 
 *
 * This file is provided as part of the project description and starter kit.
 * If you modify it, then put your name, email address, and the date, in this
 * comment, and include this file in the materials you turn in.
 */

/*----------------------------------------------------------------------------*/

/* This is a rather primitive implementation.  It should be improved
 * by changing from a fixed-size array to a dynamically-allocated
 * linked list.
 *
 * The only trick here is the C99 pre-defined identifier __func__ which
 * evaluates to a character string giving the name of the function
 * in which it appears.  (See C:ARM Sec. 2.6.1)
 */

#include <stdio.h>
#include <sys/types.h>

#include "pr6_table.h"

/* fixed-size process table, give the size as a symbolic constant */ 
#define MAX_CHILDREN 8

/* an entry in the process table */ 
typedef struct pr6_process { 
  pid_t pid;		/* process ID, supplied from fork() */ 
                        /* if 0, this entry is currently not in use */ 
  int   state;		/* process state, your own definition */ 
  int   exit_status;	/* supplied from wait() if process has finished */ 
} pr6_process_info;

/* The process table, maintained by the parent process only.
 * The table is static, to restrict access to the implementation.
 */ 
static pr6_process_info process_table[MAX_CHILDREN];

#define STATE_NONE       0
#define STATE_RUNNING    1
#define STATE_TERMINATED 2

static char *state[] = { "none", "running", "terminated" };

/*----------------------------------------------------------------------------*/

void print_process_table(const char * const caller)
{
  int i;

  printf("  process table, printed by %s\n", caller);
  printf("       pid         state        status\n");

  for (i = 0; i < MAX_CHILDREN; i++)
    {
      printf("    %6d    %10s    0x%08x\n",
        (int)process_table[i].pid,
	state[process_table[i].state],
	process_table[i].exit_status);
    }
}

void init_process_table(void)
{
  int i;

  for (i = 0; i < MAX_CHILDREN; i++)
    {
        process_table[i].pid = (pid_t) 0;
	process_table[i].state = STATE_NONE;
	process_table[i].exit_status = 0;
    }

  if (pr6_debug) print_process_table(__func__);
}

int limit_process_table(void)
{
  return MAX_CHILDREN;
}

void insert_process_table(pid_t pid)
{
  int i;

  for (i = 0; i < MAX_CHILDREN; i++)
    {
      if (process_table[i].pid == 0)
        {
          process_table[i].pid = pid;
	  process_table[i].state = STATE_RUNNING;
	  process_table[i].exit_status = 0;
	  break;
	}
    }

  if (pr6_debug) print_process_table(__func__);
}

void update_process_table(pid_t pid, int status)
  /* exit status from completed process */
{
  int i;

  for (i = 0; i < MAX_CHILDREN; i++)
    {
      if (process_table[i].pid == pid)
        {
	  process_table[i].state = STATE_TERMINATED;
	  process_table[i].exit_status = status;
	  break;
	}
    }

  if (pr6_debug) print_process_table(__func__);
}

void remove_process_table(pid_t pid)
{
  int i;

  for (i = 0; i < MAX_CHILDREN; i++)
    {
      if (process_table[i].pid == pid)
        {
          process_table[i].pid = (pid_t) 0;
	  process_table[i].state = STATE_NONE;
	  process_table[i].exit_status = 0;
	  break;
	}
    }

  if (pr6_debug) print_process_table(__func__);
}

/*----------------------------------------------------------------------------*/

