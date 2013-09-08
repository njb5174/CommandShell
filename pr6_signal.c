/* CMPSC 311 Project 6 toolkit 
 *
 * This file is provided as part of the project description and starter kit.
 * If you modify it, then put your name, email address, and the date, in this
 * comment, and include this file in the materials you turn in.
 */

/*----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

/* for sigaction(2), signal(3C), signal numbers */
#include <signal.h>

#include "pr6_signal.h"

/* The default here is to use sigaction() instead of signal().
 * If you decide to simplify the code by allowing only one choice, then you
 * should use sigaction() instead of signal(), and read CS:APP Sec. 8.5.5
 * or APUE Fig. 10.18.
 */

#ifdef PR6_USE_SIGNAL
#undef PR6_USE_SIGACTION
#else
#define PR6_USE_SIGACTION
#endif

/*----------------------------------------------------------------------------*/

/* return 0 if successful, -1 if not */
int install_signal_handler(int sig, sighandler_t func)
{
  int error;

#ifdef PR6_USE_SIGACTION
  struct sigaction sigact;		/* signal handler descriptor */
  int ret;				/* error indicator */

  sigact.sa_handler = func;		/* name of the signal handler function */
  sigemptyset(&sigact.sa_mask);		/* do not mask any interrupts while in handler */
  sigact.sa_flags = SA_RESTART;		/* restart system functions if interrupted */
					/* later experiment - replace SA_RESTART with 0 */
  ret = sigaction(sig, &sigact, NULL);	/* do the installation */
  error = (ret == -1);			/* error condition */
#else
  sighandler_t *ret;			/* previous handler or error indicator */

  ret = signal(sig, func);		/* the old-fashioned non-POSIX way */
  error = (ret == SIG_ERR);		/* error condition */
#endif

  if (error)
    {
      fprintf(stderr, "install_signal_handler(%d) failed: %s\n", sig, strerror(errno));
    }

  return (error ? -1 : 0);
}

void reinstall_signal_handler(int sig, sighandler_t func)
{
#ifdef PR6_USE_SIGACTION
  /* the signal handler remains installed after a signal is received */
  return;
#else
  /* the default signal handler was reinstalled after a signal is received */
  signal(sig, func);
#endif
}

/*----------------------------------------------------------------------------*/

