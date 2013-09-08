/* CMPSC 311 Project 6 toolkit 
 *
 * This file is provided as part of the project description and starter kit.
 * If you modify it, then put your name, email address, and the date, in this
 * comment, and include this file in the materials you turn in.
 */

/*----------------------------------------------------------------------------*/

/* for errno */
#include <errno.h>

/* for waitpid(2) and wait(2) */
#include <sys/types.h>
#include <sys/wait.h>

#include "pr6_wait.h"

/*----------------------------------------------------------------------------*/

/* wait for a child process whose pid you know
 *
 * return 1 if a child was found
 *    *child_status has been updated, and the child has terminated
 *
 * return 0 if no child was found
 *    *child_status has not been updated
 */

int wait_child(pid_t wait_pid, int *child_status)
{
  int s;

  /* loop because waitpid() can be interrupted by a signal and return early */

  while (waitpid(wait_pid, &s, 0) == (pid_t)(-1))
    {
      if (errno == ECHILD)              /* no more children */
        { return 0; }
    }

  *child_status = s;

  return 1;
}

/*----------------------------------------------------------------------------*/

/* wait for a child process whose pid you do not know
 *    if more than one child has terminated, report only one
 *
 * return 1 if a child was found
 *    *wait_pid and *child_status have been updated, and the child has terminated
 *
 * return 0 if no child was found
 *    *wait_pid and *child_status have not been updated
 */

int wait_any_child(pid_t *wait_pid, int *child_status)
{
  pid_t w;
  int s;

  /* loop because wait() can be interrupted by a signal and return early */

  while ((w = wait(&s)) == (pid_t)(-1))
    {
      if (errno == ECHILD)              /* no more children */
        { return 0; }
    }

  *wait_pid = w;
  *child_status = s;

  return 1;
}

/*----------------------------------------------------------------------------*/

