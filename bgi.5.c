/* CMPSC 311, Project Example, version 5
 *
 * This version cleans up a few details about printf().
 *
 * compile and run, same as before but use bgi.5.c
 *
 * include files     for
 *   stdio.h         printf(3), fprintf(3)
 *   stdlib.h        exit(3), atoi(3), getenv(3), putenv(3), setenv(3), unsetenv(3)
 *   unistd.h        getopt(3)
 *   errno.h         errno
 *   string.h        strerror(3)
 *
 * stdin, stdout, stderr
 *   When a C program is started by the operating system, it has three files open,
 *   stdin (standard input), stdout (standard output), and stderr (standard
 *   error).  Each has the type FILE *, where FILE is defined in stdio.h.  These
 *   correspond to the C++ objects cin, cout, cerr, although historically it was
 *   the other way around.  printf() writes to stdout.  The normal Unix behavior
 *   is that error messages are sent to stderr with fprintf(), and "normal" output
 *   is sent to stdout.
 *
 * Output from the -a -e -p options is now separated by a blank line if more than
 *   one of these options is used.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

extern char **environ;

/* for use with getopt(3) */
extern char *optarg;
extern int optind;
extern int optopt;
extern int opterr;

/* Write this first! */
static void usage(char *prog, int status)
{
  if (status == EXIT_SUCCESS)
    {
      printf("usage: %s [-h] [-v] [-a] [-e] [-p]"
             " [-s var=val] [-t var] [-u var] [-x stat]\n",
             prog);
      printf("  -h           print help\n");
      printf("  -v           verbose mode\n");
      printf("  -a           print argc and argv[]\n");
      printf("  -e           print environ[]\n");
      printf("  -p           print envp[]\n");
      printf("  -s var=val   set environment variable var to value val\n");
      printf("  -t var       print value of environment variable var\n");
      printf("  -u var       unset environment variable var\n");
      printf("  -x stat      set exit status to stat\n");
    }
  else
    {
      fprintf(stderr, "%s: Try '%s -h' for usage information.\n", prog, prog);
    }

  exit(status);
}

/* print elements of (*array)[] up to (but not including) a NULL pointer */
static void print_NULL_terminated_array(char *name, char **array[], int verbose)
{
  char **p;

  if (verbose == 0)
    {
      for (p = *array; *p != NULL; p++)
        { printf("%s\n", *p); }
    }
  else
    {
      int i = 0;
      printf("address %10p: %s = %10p\n", array, name, *array);
      for (p = *array; *p != NULL; p++, i++)
        {
          printf("  address %10p: %s[%d] = %10p --> \"%s\"\n",
            p, name, i, *p, *p);
        }
    }
}

int main(int argc, char *argv[], char *envp[])
{
  /* for use with getopt(3) */
  int ch;

  /* option flags and option-arguments set from the command line */
  int status = EXIT_SUCCESS;
  int verbose = 0;
  int a_flag = 0;
  int e_flag = 0;
  int p_flag = 0;

  /* for use with -t option */
  char *t_val;

  int spacer = 0;

  while ((ch = getopt(argc, argv, ":hvaeps:t:u:x:")) != -1)
    {
      switch (ch) {
        case 'h':
          usage(argv[0], EXIT_SUCCESS);
          break;
        case 'v':
          verbose = 1;
          break;
        case 'a':
          a_flag = 1;
          break;
        case 'e':
          e_flag = 1;
          break;
        case 'p':
          p_flag = 1;
          break;
        case 's':
          if (putenv(optarg) != 0)
            {
              fprintf(stderr, "%s: putenv(%s) failed: %s\n",
                argv[0], optarg, strerror(errno));
            }
          break;
        case 't':
          t_val = getenv(optarg);
          if (t_val == NULL)
            { printf("%s: not found\n", optarg); }
          else
            { printf("%s = \"%s\"\n", optarg, t_val); }
          spacer = 1;
          break;
        case 'u':
          if (unsetenv(optarg) != 0)
            {
              fprintf(stderr, "%s: unsetenv(%s) failed: %s\n",
                argv[0], optarg, strerror(errno));
            }
          break;
        case 'x':
          status = atoi(optarg);
          break;
        case '?':
          fprintf(stderr, "%s: invalid option '%c'\n", argv[0], optopt);
          usage(argv[0], EXIT_FAILURE);
          break;
        case ':':
          fprintf(stderr, "%s: invalid option '%c' (missing argument)\n",
            argv[0], optopt);
          usage(argv[0], EXIT_FAILURE);
          break;
        default:
          usage(argv[0], EXIT_FAILURE);
          break;
      }
    }

  if (status < 0 || status > 255)
    {
      fprintf(stderr, "%s: -x %d out of range (ignored)\n", argv[0], status);
    }

  if (a_flag == 1)
    {
      if (spacer == 1)
        { printf("\n"); spacer = 0; }

      if (verbose == 0)
        { printf("argc = %d\n", argc); }
      else
        { printf("address %10p: argc = %d\n", &argc, argc); }
      print_NULL_terminated_array("argv", &argv, verbose);

      spacer = 1;
    }

  if (p_flag == 1)
    {
      if (spacer == 1)
        { printf("\n"); spacer = 0; }

      print_NULL_terminated_array("envp", &envp, verbose);

      spacer = 1;
    }

  if (e_flag == 1)
    {
      if (spacer == 1)
        { printf("\n"); spacer = 0; }

      print_NULL_terminated_array("environ", &environ, verbose);

      spacer = 1;
    }

  return status;
}

