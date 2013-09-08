/* CMPSC 311 Project 7 starter kit, version 4
 * 
 * This file is provided as part of the project description and starter kit.
 * If you modify it, then put your name, email address, and the date, in this
 * comment, and include this file in the materials you turn in.
 *
 * Usage:
 *   c99 -v -o pr7 pr7.2.c                        [Sun compiler]
 *   gcc -std=c99 -Wall -Wextra -o pr7 pr7.2.c    [GNU compiler]
 *
 *   pr7
 *   pr7%      [type a command and then return]
 *   pr7% exit
 *  
 * This version is derived from the shellex.c program in CS:APP Sec. 8.4.
 */

/*----------------------------------------------------------------------------*/
 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "pr7.h"
#include "pr7_table.h"
#include "pr6_signal.h"

#define MAXLINE 128
#define MAXCLD  15
#define STDIN 0
#define STDOUT 1
#define STDERR 2

pid_t forgroundProcess = 0;

extern char **environ;
extern char* optarg;
extern int optind;
extern int optopt;

int pr6_debug = 0;
int pr7_debug = 0;
int line_num = 0;
int verbose = 0;

char *program_name;         /* global */
int self_pid;               /* global */

/*----------------------------------------------------------------------------*/

/* Compare to main() in CS:APP Fig. 8.22 */

int main(int argc, char *argv[])
{
    int ret = EXIT_SUCCESS;
    int ch;
    char cmdline[MAX_LINE];                /* command line */
    int interactive = 0;
    char* infile_name = NULL;
    char* program_name = argv[0];
    
    install_signal_handler(SIGINT, INThandler);

    FILE* infile = NULL;

    /* parse the command line options */
    while ((ch = getopt(argc, argv, "hvids:")) != -1)
    {
        switch (ch)
        {
            case 'h':
                usage(argv[0], EXIT_SUCCESS);
                break;
            case 'i': /* interactive mode */
                interactive = 1;
                break;
            case 's':
                infile_name = strdup(optarg);
                interactive = 0;
                break;
            case 'd':
                pr7_debug = 1;
                break;
            case 'v':
                verbose = 1;
                break;
            case '?':
                printf("%s: invalid option '%c'\n", argv[0], optopt);
                usage(argv[0], EXIT_FAILURE);
                break;
            case ':':
                printf("%s: invalid option '%c' (missing argument)\n", argv[0], optopt);
                usage(argv[0], EXIT_FAILURE);
                break;
            default:
                usage(argv[0], EXIT_FAILURE);
                break;
        }
    }

    program_name = argv[0];

    if (interactive || (optind >= argc && infile_name == NULL))
    { infile = stdin; infile_name = "[stdin]"; }
    else
    {
        if (NULL == infile_name)
        { infile_name = strdup(argv[optind]); }
        infile = fopen(infile_name, "r");

        if (NULL == infile)
        {
            fprintf(stderr, "%s: cannot open file %s: %s\n",
                    program_name, infile_name, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    printf("%s\n", infile_name);

    line_num = 0;
    while (1)
    {
        char* succ;
        ++line_num;
        /* issue prompt and read command line */
        if (infile == stdin) printf("%s %d%% ", program_name, line_num);
        succ = fgets(cmdline, MAX_LINE, infile); 
        

        cleanup_terminated_children();
        if (succ != NULL)
        {
            ret = eval_line(cmdline);
        }
        
        if (feof(infile))                  /* end of file */
        { 
            break;
        }
    }

    return ret;
}

/*----------------------------------------------------------------------------*/

/* evaluate control-C signals command line
 *
 * Compare to eval() in CS:APP Fig. 8.23.
 */

int eval_line(char *cmdline)
{
    char *argv1[MAXLINE];  /* argv for execve() */
    char *argv2[MAXLINE];
    char buf[MAX_LINE];    /* holds modified command line */
    int background;       /* should the job run in background or foreground? */
    pid_t pid;            /* process id */
    int ret = EXIT_SUCCESS;
    
    strcpy(buf, cmdline);                 /* buf[] will be modified by parse() */
    int pipe_break = parse_pipeline(buf);
    background = parse(buf, argv1);        /* build the argv array */

    background = parse(&buf[pipe_break], argv2) || background;
	
    if (argv1[0] == NULL)          /* ignore empty lines */
    { return ret; }

    if (builtin(argv1) == 1)       /* the work is done */
    { return ret; }

    if ((pid = fork()) == 0)      /* child runs user job */
    {
    	
        run_children(argv1, argv2); 
        
        _exit(EXIT_FAILURE);
    }

    if (background)               /* parent waits for foreground job to terminate */
    {
        printf("background process %d: %s\n", (int) pid, cmdline);
        //insert_process_table(pid, argv1[0]); 
      //  insert_process_table(pid);
    }
    else
    {
    	printf("HERE!!!!\n");
    	//insert_process_table(pid);
    	
        forgroundProcess = pid;
        if (waitpid(pid, &ret, 0) == -1)
        {
            printf("%s: failed: %s\n", argv1[0], strerror(errno));
            exit(EXIT_FAILURE);
        }
        
        //remove_process_table(pid);
        if (verbose)
        { printf("process %d, completed normally, status %d\n", pid, ret); }
    }

    return ret;
}

/*----------------------------------------------------------------------------*/

/* This function will run a child process, or if two processes
 * are supplied, it will fork and run two children and create
 * a pipeline between them through stdin and stdout
 */
void run_children(char *argv1[], char *argv2[])
{
    if (NULL == argv1 || NULL == argv2)
    { _exit(EXIT_FAILURE); }

    if (NULL == *argv2)
    {
        if (!process_indirection(argv1))
        {
            _exit(EXIT_FAILURE);
        }
        execvp(argv1[0], argv1);
        printf("%s: failed: %s\n", argv1[0], strerror(errno));
        
        _exit(EXIT_FAILURE);
    }
    else
    {
        int fd[2];
        pipe(fd);
        pid_t pid = fork();
        if (pid == (pid_t)-1)
        {
            fprintf(stderr, "Fork failed with: %s\n", strerror(errno));
            _exit(EXIT_FAILURE);
        }
        else if (pid == 0)  /* Right hand side of the pipeline */
        {
            close(fd[1]);
            close(STDIN);
            dup(fd[0]);
            close(fd[0]);
            if (!process_indirection(argv2))
            {
                _exit(EXIT_FAILURE);
            }
            execvp(argv2[0], argv2);
            _exit(EXIT_FAILURE);
        }
        else                /* Left hand side of the pipeline */
        {
            close(fd[0]);
            close(STDOUT);
            dup(fd[1]);
            close(fd[1]);
            if (!process_indirection(argv1))
            {
                _exit(EXIT_FAILURE);
            }
            execvp(argv1[0], argv1);
            _exit(EXIT_FAILURE);
        }
    }
}

/* parses a command into two commands by separating
 * the string where the pipe operator is found
 */
int parse_pipeline(char* cmdline)
{
    int i;
    for (i = 0; cmdline[i] != '\0'; ++i)
    {
        if (cmdline[i] == '|')
        {
            cmdline[i] = '\0';
            return i+1;
        }
    }
    return i;
}

/* Handles setting up file indirection for the > and < operators 
 * This function destructively alters the array pointed to by 
 * argv, this is to prevent the file names and operators from being passed
 * to the program as arguments
 * */
int process_indirection(char* argv[])
{
    for (; *argv != NULL; ++argv)
    {
        if (strcmp(*argv, ">") == 0)    /* output redirection */
        {
            *argv = NULL;   /* Alter the argv array so all args following are not included */
            if (NULL == argv[1])    /* Ensure a file name is given */
            {
                fprintf(stderr, "No file supplied for output indirection\n");
                return 0;
            }
            if (NULL == freopen(argv[1], "w+", stdout)) /* Attempt to open the file */
            {
                fprintf(stderr, "Unable to open file %s for %s\n", 
                        argv[1], strerror(errno));
                return 0;
            }
        }
        else if (strcmp(*argv, "<") == 0)   /* input redirection */
        {
            *argv = NULL;   /* All the following args are for IO redirection */
            if (NULL == argv[1])
            {
                fprintf(stderr, "No file supplied for input redirection\n");
                return 0;
            }
            if (NULL == freopen(argv[1], "r", stdin))
            {
                fprintf(stderr, "Unable to open file %s for %s\n", 
                        argv[1], strerror(errno));
                return 0;
            }
        }
    }
    return 1;
}

/* parse the command line and build the argv array
 *
 * Compare to parseline() in CS:APP Fig. 8.24.
 */

int parse(char *buf, char *argv[])
{
	
    char *delim;          /* points to first whitespace delimiter */
    int argc = 0;         /* number of args */
    int bg;               /* background job? */

    const char whsp[] = " \t\n\v\f\r";          /* whitespace characters */

    /* Note - the trailing '\n' in buf is whitespace, and we need it as a delimiter. */

	char *comment =	strpbrk(buf, "#");
	if(comment != NULL)
	{ *comment = '\0';  }
	
    while (1)                             /* build the argv list */
	{
        buf += strspn(buf, whsp);         /* skip leading whitespace */
        delim = strpbrk(buf, whsp);       /* next whitespace char or NULL */
        if (delim == NULL)                /* end of line */
        { break; }
        argv[argc++] = buf;               /* start argv[i] */
        *delim = '\0';                    /* terminate argv[i] */
        buf = delim + 1;                  /* start argv[i+1]? */
    }
    argv[argc] = NULL;

    if (argc == 0)                        /* blank line */
    { return 0; }

    /* should the job run in the background? */
    if ((bg = (strcmp(argv[argc-1], "&") == 0)))
    { argv[--argc] = NULL; }

    return bg;
}

/*----------------------------------------------------------------------------*/

/* if first arg is a builtin command, run it and return true
 *
 * Compare to builtin_command() in CS:APP Fig. 8.23.
 */

int builtin(char *argv[])
{
    if (strcmp(argv[0], "help") == 0)
    {
        help();
        return 1;
    }
    if (strcmp(argv[0], "exit") == 0)     /* exit command */
    { 
        int count = 0; //job_count();
        if (count > 0)
        { printf("There are %d jobs still running.\n", count); }
        else
        { exit(0); }

        return 1;
    }

    if (strcmp(argv[0], "&") == 0)        /* ignore singleton & */
    { return 1; }

    if (strcmp(argv[0], "echo") == 0)     /* echo command */
    { 
        argv++;
        for (;*argv != NULL; argv++)
        {
            printf("%s%s", *argv, " ");
        }
        printf("\n");
        return 1;
    }

    if (strcmp(argv[0], "dir") == 0)    /* dir command */
    {   /* some implementations of PATH_MAX are not
           compile time constants (appearantly) */
        char* path = (char*) malloc(PATH_MAX * sizeof(char));
        if (NULL == path || NULL == getcwd(path, PATH_MAX))
        {   /* path name exceeded the max length */
            fprintf(stderr, "Error allocating space for getcwd: %s", 
                    strerror(errno));
            exit(EXIT_FAILURE);
        }
        printf("%s\n", path);
        free(path);
        return 1;
    }

    if (strcmp(argv[0], "cdir") == 0)   /* cdir command */
    {
        cdir(argv);
        return 1;
    }

    if (strcmp(argv[0], "penv") == 0)   /* penv command */
    {
        penv(argv);
        return 1;
    }

    if (strcmp(argv[0], "senv") == 0)
    {
        senv(argv);
        return 1;
    }

    if (strcmp(argv[0], "unsenv") == 0)
    {
        unsenv(argv);
        return 1;
    }

    if (strcmp(argv[0], "pjobs") == 0)
    {
        pjobs();
        return 1;
    }

    if (strcmp(argv[0], "set") == 0)
    {
        set(argv);
        return 1;
    }
    if (strcmp(argv[0], "limits") == 0)
    {
        limits();
        return 1;
    }

    return 0;                             /* not a builtin command */
}

void limits()
{
    printf("Maximum number of child processes: %d\n", MAXCLD);
    printf("Maximum input line size:           %d\n", MAXLINE);
}

void INThandler(int sig)
{
	//signal(sig, sigin)
	if ((forgroundProcess) != (pid_t)(0))
	{
		kill(forgroundProcess, SIGUSR1);
		forgroundProcess = 0;
	}
    else
    {
        printf("\n%s %d%% ", program_name, ++line_num);
    }
    fflush(stdout);
    signal(SIGINT, INThandler);
    }

/*----------------------------------------------------------------------------*/

void penv(char *argv[])
{
    ++argv;
    if (NULL == *argv) 
    { /* No args were given */
        for (int i = 0; environ[i] != NULL; ++i)
        {
            printf("%s\n", environ[i]);
        }
    }
    else
    {
        for (; *argv != NULL; ++argv)
        {
            printf("%s=%s\n", *argv, getenv(*argv));
        }
    }
}

void cdir(char* argv[])
{
    char* path = NULL == argv[1] ? getenv("HOME") : argv[1];
    char* rpath = NULL;
    rpath = realpath(path, NULL);   /* realpath calls malloc */
    if (NULL != rpath && chdir(rpath) == 0)
    {   /* sucess resolving the path and changing the directory */
        setenv("PWD", rpath, 1);
    }
    else
    {   /* failure while resolving or changing dir, print error */
        fprintf(stderr, "Error in cdir: %s\n", strerror(errno));
    }

    free(rpath);    /* free space from realpath */
}

void senv(char* argv[])
{
    if (NULL == argv[1] || NULL == argv[2])
    {
        printf("Error: senv takes two arguments\n");
        return;
    }

    if (setenv(argv[1], argv[2], 1) != 0)
    {
        fprintf(stderr, "Error: senv failed with %s\n", strerror(errno));
    }
}

void unsenv(char* argv[])
{
    if (NULL == argv[1])
    {
        printf("Error: unsetenv takes one arguemnt\n");
        return;
    }

    if (unsetenv(argv[1]) != 0)
    {
        fprintf(stderr, "Error: unsetenv failed with %s\n", strerror(errno));
    }
}

void set(char* argv[])
{
    if (NULL == argv[1] || NULL == argv[2])
    { help(); return; }

    int on;
    if (strcmp(argv[2], "on") == 0) 
    { on = 1; }
    else if (strcmp(argv[2], "off") == 0) 
    { on = 0; }
    else
    { help(); return; }

    if (strcmp(argv[1], "debug") == 0)
    { pr7_debug = on; }
    else if (strcmp(argv[1], "verbose") == 0)
    { verbose = on; }
    else
    { help(); return; }
}

void pjobs()
{
    print_process_table(__func__);
}

void help()
{
    printf("Commands:\n  help\n  exit [n]\n  echo\n  dir\n"
           "  cdir [directory]\n  penv\n  penv variable_name\n"
           "  senv variable_name value\n  unsenv variable_name\n"
           "  pjobs\n  limits\n  set\n");
}

/* Find all the child processes that have terminated, without waiting.
 *
 * This code is adapted from the GNU info page on waitpid() and the Solaris
 * man page for waitpid(2).
 */

int cleanup_terminated_children(void)
{
    pid_t pid;
    int status;
    int count = 0;

    while (1)
    {
        pid = waitpid(-1, &status, WNOHANG);

        if (pid == 0)             /* returns 0 if no child process to wait for */
        { break; }

        if (pid == -1)            /* returns -1 if there was an error */
        {
            /* errno will have been set by waitpid() */
            if (errno == ECHILD)  /* no children */
            { break; }
            if (errno == EINTR)   /* waitpid() was interrupted by a signal */
            { continue; }       /* try again */
            else
            {
                printf("unexpected error in cleanup_terminated_children(): %s\n",
                        strerror(errno));
                break;
            }
        }
        //print_wait_status(pid, status);      /* supply this yourself */
        printf("process %d, completed normally, status %d\n", pid, status);
        update_process_table(pid, status);
        remove_process_table(pid);
        count++;
    }

    return count;
}

static void usage(char *prog, int status)
{
  if (status == EXIT_SUCCESS)
    {
      printf("Usage: %s  pr7 [-h] [-v] [-i] [-e] [-s f] [file]\n",
        prog);
      printf("    -h      help\n");
      printf("    -v      verbose mode\n");
      printf("    -i      interactive mode\n");
      printf("    -e      echo commands before execution\n");
      printf("    -s f    use startup file f, default pr7.init\n");
      printf("Shell commands:\n");
      printf("    help\n");
      
   
    }
  else
    {
      fprintf(stderr, "%s: Try '%s -h' for usage information.\n", prog, prog);
    }

  exit(status);
}
