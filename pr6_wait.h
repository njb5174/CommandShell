#ifndef PR6_WAIT_H
#define PR6_WAIT_H

/* CMPSC 311 Project 6 toolkit 
 *
 * This file is provided as part of the project description and starter kit.
 * If you modify it, then put your name, email address, and the date, in this
 * comment, and include this file in the materials you turn in.
 */

/*----------------------------------------------------------------------------*/

#include <sys/types.h>

int wait_child(pid_t wait_pid, int *child_status);
int wait_any_child(pid_t *wait_pid, int *child_status);

/*----------------------------------------------------------------------------*/

#endif

