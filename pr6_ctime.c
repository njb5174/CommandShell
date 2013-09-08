/* CMPSC 311 Project 6 toolkit 
 *
 * This file is provided as part of the project description and starter kit.
 * If you modify it, then put your name, email address, and the date, in this
 * comment, and include this file in the materials you turn in.
 */

/*----------------------------------------------------------------------------*/

/* for standard I/O and the C standard library */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/* for time(2) and ctime_r(3C) */
// #include <sys/types.h>
#include <time.h>

#include "pr6_ctime.h"

/*----------------------------------------------------------------------------*/

/* current time as a character string */
char * Ctime(char buf[26])
{
  time_t time_val;
  char * time_str;

  time_val = time(NULL);
  if (time_val == (time_t)(-1))
    {
      /* time() failed, which is highly unlikely */
      strncpy(buf, "*** time unavailable ***", 26);
      return buf;
    }

  /* Sun's man page for ctime_r() explains this hack */
#ifdef _POSIX_PTHREAD_SEMANTICS
  time_str = ctime_r(&time_val,buf);       /* new standard */
#else
  time_str = ctime_r(&time_val,buf,26);    /* old standard */
#endif
  if (time_str == NULL)
    {
      /* ctime_r() failed, which is highly unlikely */
      strncpy(buf, "*** time unavailable ***", 26);
      return buf;
    }

  buf[24] = '\0';    /* get rid of the newline placed by ctime_r() */

  return buf;
}

/*----------------------------------------------------------------------------*/

void print_msg(char *msg)
{
  char buf[26];
  printf("%6d: %s %s\n", (int) getpid(), Ctime(buf), msg);
}

void print_msg_1(char *msg, int n)
{
  char buf[26];
  printf("%6d: %s %s %d\n", (int) getpid(), Ctime(buf), msg, n);
}

void print_msg_2(char *msg, int n1, int n2)
{
  char buf[26];
  printf("%6d: %s %s %d 0x%08x\n", (int) getpid(), Ctime(buf), msg, n1, n2);
  /* n1 in decimal, n2 in C-style hexadecimal */
}

void print_msg_error(char *msg, char *errmsg)
{
  char buf[26];
  printf("%6d: %s serious error: %s %s\n", (int) getpid(), Ctime(buf), msg, errmsg);
}

void print_msg_abort(char *msg)
{
  char buf[26];
  printf("%6d: %s fatal error: %s\n", (int) getpid(), Ctime(buf), msg);
  exit(EXIT_FAILURE);
}

/*----------------------------------------------------------------------------*/

