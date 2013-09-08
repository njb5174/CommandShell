#ifndef PR6_TABLE_H
#define PR6_TABLE_H

/* CMPSC 311 Project 6 toolkit 
 *
 * This file is provided as part of the project description and starter kit.
 * If you modify it, then put your name, email address, and the date, in this
 * comment, and include this file in the materials you turn in.
 */

/*----------------------------------------------------------------------------*/

/* This is a rather primitive implementation.  It should be improved. */

#include <sys/types.h>

extern int pr6_debug;

void init_process_table(void);
int limit_process_table(void);				/* max number of entries */

void print_process_table(const char * const caller);

void insert_process_table(pid_t pid);			/* new process */
void update_process_table(pid_t pid, int status);	/* exit status from completed process */
void remove_process_table(pid_t pid);			/* defunct process */

/*----------------------------------------------------------------------------*/

#endif

