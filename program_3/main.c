#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <readline/readline.h>
#include <setjmp.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>

#include "config.h"
#include "cstr.c"

#define NAME Strs
#define DTYPE char*
#define C_STRING_MODE
#include "array.c"
#undef NAME
#undef DTYPE
#undef C_STRING_MODE

#define NAME Pidts
#define DTYPE pid_t
#include "array.c"
#undef NAME
#undef DTYPE

// Save a few bytes by storing some state information
// bitwise in a uint8_t
#define FILE_TO_STDIN  0b10000000
#define STDOUT_TO_FILE 0b01000000
#define EXEC_BG        0b00000001

enum InputMode {INTERACTIVE, PIPE, FILE_IN};
// A few global variables are needed
int bg_enabled = 0;
int blocked_by_readline = 1;
sigjmp_buf ctrlc_buf;


// This function replaces all instances of find with replace in string,
// storing the result in a newly-allocated buffer *dest.
int strReplace(char* string, char* find, char* replace, char** dest)
{
  char *where,
       *result = NULL,
       *string_orig = string;
  size_t len = 0,
         len_from_orig,
         find_len = strlen(find),
         replace_len = strlen(replace),
         string_len = strlen(string),
         len_prev = 0;
  int found = 0;

  // Set where to the first occurance of find in string, if it exists
  where = strstr(string, find);

  while (! (where == NULL))
  {
    found = 1;
    len_from_orig = where - string;
    len_prev = len;
    len += len_from_orig + replace_len;
    result = realloc(result, (len + 1) * sizeof(char));
    memset(result + len_prev, 0, len - len_prev);
    strncat(result, string, len_from_orig);
    strncat(result, replace, replace_len);
    string = where + find_len;

    if (string > (string_orig + string_len))
    {
      where = NULL;
    }
    else
    {
      where = strstr(string, find);
    }
  }

  if (found == 0)
  {
    dest[0] = string;
    return 1;
  }
  else
  {
    dest[0] = result;
    return 0;
  }
}

// Splits line at whitespace characters, putting them into the Strs* arr data structure
void parseCLine(char** line_in, Strs* arr)
{
  char *line = *line_in,
       *whitespace = " \t\n",
       *temp[1],
       *temp2;
  // FIXME: size?
  char pid[10];
  size_t arglen;

  // If the user entered nothing, do not attempt to parse
  if (strlen(line) == 0)
  {
    return;
  }

  // Get PID of self
  sprintf(pid, "%i", getpid());

  // Attempt to replace $$ with pid in user's input
  if (strReplace(line, "$$", pid, temp) == 0)
  {
    // If strReplace succeeded, free the memory of the original string,
    // updating line_in so the rest of the program knows the new memory address
    free(line);
    line = *temp;
    *line_in = line;
  }


  while(! ((line == NULL)))
  {
    // Ignore whitespace
    while (strpbrk(line, whitespace) == line)
    {
      line++;

      if ((line + 1) > line + strlen(line))
      {
        return;
      }
    }

    if (strpbrk(line, whitespace) == NULL)
    {
      // There is no more whitespace, so we want the entire remaining string
      arglen = strlen(line);
    }
    else
    {
      // Read up until the next occurance of whitespace
      arglen = strpbrk(line, whitespace) - line;
    }

    // Read arglen characters into a new string
    temp2 = malloc((arglen + 1) * sizeof(char));
    memset(temp2, 0, (arglen + 1) * sizeof(char));
    strncpy(temp2, line, arglen);
    pushStrs(arr, temp2);
    // Skip to next whitespace
    line = strpbrk(line, whitespace);
  }

}

void printExecError(int exec_code, char* argv0)
{
  char* error_str;
  if (exec_code == EACCES)
  {
    // FIXME: this if statement doesn't work
    error_str = "no such file or directory";
  }
  else
  {
   error_str = "unhandled exec() error";
  }

  printf("%s: error: %s\n", argv0, error_str);
}

void catchSIGINT(int signo)
{
  if (blocked_by_readline == 0)
  {
    siglongjmp(ctrlc_buf, 1);
  }
}

void catchSIGTSTP(int signo)
{
  if (bg_enabled == 0)
  {
    bg_enabled = 1;
  }
  else
  {
    bg_enabled = 0;
  }

  if (blocked_by_readline == 0)
  {
    siglongjmp(ctrlc_buf, 1);
  }
}

size_t printBGProcessStatus(Pidts* pids_bg)
{
    size_t n_pids = pids_bg->used,
           i;
    int wp_status;
    pid_t pid;


    for(i = 0; i < n_pids; i++)
    {
      pid = waitpid(pids_bg->d[i], &wp_status, WNOHANG);

      if (pid == pids_bg->d[i])
      {
        if (WIFSIGNALED(wp_status))
        {
          printf("background pid %i is done: terminated by signal %i\n", pid, WTERMSIG(wp_status));
        }
        else if (WIFEXITED(wp_status))
        {
          printf("background pid %i is done: exit value %i\n", pid, WEXITSTATUS(wp_status));
        }

        removeValsPidts(pids_bg, pid);
        n_pids = pids_bg->used;
      }
      else if (pid == -1)
      {
        printf("Unexpected waitpid error: %s\n", strerror(errno));
      }
    }

    return pids_bg->used;
}

void exitStatus(int* waitpid_status)
{
  if (WIFSIGNALED(*waitpid_status))
  {
    printf("terminated by signal %i\n", WTERMSIG(*waitpid_status));
  }
  else
  {
    printf("exited with status %i\n", WEXITSTATUS(*waitpid_status));
  }
}

void killBGProcesses(Pidts* pids)
{
  size_t i;

  for (i = 0; i < pids->used; i++)
  {
    if (! (kill(pids->d[i], SIGTERM) == 0))
    {
      printf("unexpected kill error: %s\n", strerror(errno));
    }
  }
}

void exitShell(Pidts* pids)
{
  killBGProcesses(pids);
  while (! (printBGProcessStatus(pids) == 0)) {}
  // FIXME: return exit code of previous process
  exit(0);
}

int main(int argc, char** argv)
{
  int fd_stdin,
      fd_stdout,
      waitpid_status = 0,
      waitpid_status_bg = 0,
      bg_enabled_prev = 0;
  uint8_t special_funcs;
  size_t i,
         getline_len = 0;
  char *line[1] = {NULL},
       *wd,
       *prompt,
       *hostname;
  struct passwd *pw;
  enum InputMode input_mode;
  uid_t uid;
  pid_t spawnpid;
  Strs* cline;
  Pidts pids_bg[1];
  struct sigaction ignore_action = {0},
                   SIGINT_action = {0},
                   SIGTSTP_action = {0};

  initPidts(pids_bg);

  ignore_action.sa_handler = SIG_IGN;
  SIGINT_action.sa_handler =  catchSIGINT;
  SIGTSTP_action.sa_handler = catchSIGTSTP;
  sigfillset(&SIGINT_action.sa_mask);
  sigfillset(&SIGTSTP_action.sa_mask);
  // SA_RESTART needs to be enabled for the SIGINT handler, otherwise if SIGINT is recieved while
  // a foreground process is running, its waitpid() will be interrupted and it will become a zombie
  // for the life of the shell.
  SIGINT_action.sa_flags =  SA_RESTART;
  SIGTSTP_action.sa_flags = SA_RESTART;

  sigaction(SIGINT, &SIGINT_action, NULL);
  sigaction(SIGTSTP, &SIGTSTP_action, NULL);

  // Check whether stdin is interactive
  if (isatty(fileno(stdin)))
  {
    input_mode = INTERACTIVE;
  }
  else
  {
    input_mode = PIPE;
  }

  while (1)
  {
    cline = malloc(sizeof(Strs));
    initStrs(cline);
    wd = getcwd(NULL, 0);
    hostname = malloc(HOSTNAME_SIZE * sizeof(char));
    special_funcs = 0;
    // magic value for "these were never modified"
    fd_stdin = -2;
    fd_stdout = -2;

    // Get hostname of computer
    gethostname(hostname, HOSTNAME_SIZE);
    uid = geteuid();
    pw = getpwuid(uid);

    i = (10 + HOSTNAME_SIZE + strlen(pw->pw_name) + strlen(hostname) + strlen(hostname));
    prompt = malloc(i * sizeof(char));
    memset(prompt, 0, i * sizeof(char));

    // build shell prompt
    strcat(prompt, pw->pw_name);
    strcat(prompt, "@");
    strcat(prompt, hostname);
    strcat(prompt, " ");
    strcat(prompt, wd);
    strcat(prompt, " : ");

    // Jumps from signal handlers land here
    if (! (sigsetjmp(ctrlc_buf, 1) == 0))
    {
      // Print newline so that prompts don't stack on the same line
      printf("\n");
    }

    // Check if children have exited
    printBGProcessStatus(pids_bg);

    // Check whether background mode has been toggled
    if (! (bg_enabled == bg_enabled_prev))
    {
      bg_enabled_prev = bg_enabled;

      if (bg_enabled == 0)
      {
        printf("Leaving foreground-only mode \n");
      }
      else
      {
        printf("Entering foreground-only mode (& is now ignored)\n");
      }
    }

    // If input is a terminal, use fancy readline library for user input,
    // otherwise, just use getline.
    if (input_mode == INTERACTIVE)
    {
      blocked_by_readline = 0;
      *line = readline(prompt);
      blocked_by_readline = 1;
    }
    else if (input_mode == PIPE)
    {
      // Outputting prompts when not in interactive mode makes little sense, but
      // the " : " prompt is explicitly part of the assignment
      #ifdef APPEASE_GRADER
      printf(" : ");
      fflush(stdout);
      #endif

      if (getline(line, &getline_len, stdin) == -1)
      {
        exitShell(pids_bg);
      }
    }


    if (*line == NULL)
    {
      exitShell(pids_bg);
    }

    // Parse input line into strings by whitespace
    parseCLine(line, cline);

    // Ignore comments
    for (i = 0; i < cline->used; i++)
    {
      if((strncmp(cline->d[i], "##", 1) == 0))
      {
        truncateStrs(cline, i);
        break;
      }
    }

    // If the user has actually typed something
    if (cline->used > 0)
    {
      // builtin "exit"
      if (strcmp(cline->d[0], "exit") == 0)
      {
        printf("Exiting smallsh.\n");
        exitShell(pids_bg);
      }
      // builtin "cd"
      else if (strcmp(cline->d[0], "cd") == 0)
      {
        if (cline->used == 1)
        {
          chdir(getenv("HOME"));
        }
        else
        {
          chdir(cline->d[1]);
        }
      }
      // builtin "status"
      else if (strcmp(cline->d[0], "status") == 0)
      {
        exitStatus(&waitpid_status);
      }
      // Run user-specified commands
      else
      {
        // Redirect file to stdin
        if (containsStrs(cline, "<") < cline->used - 1)
        {
          special_funcs |= FILE_TO_STDIN;
          fd_stdin = open(cline->d[containsStrs(cline, "<") + 1], O_RDONLY);
        }

        // Redirect stdout to file
        if ((containsStrs(cline, ">") < cline->used - 1))
        {
          special_funcs |= STDOUT_TO_FILE;
          fd_stdout = open(cline->d[containsStrs(cline, ">") + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
        }

        // Redirect I/O to /dev/null if running in background
        // and the user didn't alreay specify I/O redirection
        if ((containsStrs(cline, "&") == cline->used - 1) && (cline->used > 1) && (bg_enabled == 0))
        {
          special_funcs |= EXEC_BG;

          if (! (FILE_TO_STDIN & special_funcs))
          {
            fd_stdin = open("/dev/null", O_RDONLY);
          }

          if (! (STDOUT_TO_FILE & special_funcs))
          {
            fd_stdout = open("/dev/null", O_WRONLY);
          }
        }

        // If I/O redirection or background symbols (<, >, &) are in cline, remove them so that
        // they never get passed to the program to be executed
        if ((containsStrs(cline, "<") < cline->used - 1))
        {
          truncateStrs(cline, containsStrs(cline, "<"));
        }

        if ((containsStrs(cline, ">") < cline->used - 1))
        {
          truncateStrs(cline, containsStrs(cline, ">"));
        }

        if ((containsStrs(cline, "&") == cline->used - 1) && (cline->used > 1))
        {
          truncateStrs(cline, containsStrs(cline, "&"));
        }

        // add null string to end of args list, which execvp() uses as a terminator
        // FIXME: Put I/O redirection filenames beyond this terminator so that child knows the filenames
        pushStrs(cline, NULL);
        // Fork to execute user's command
        spawnpid = fork();

        switch (spawnpid)
        {
          case -1:
            printf("Unable to fork, this should not happen\n");
            break;
          // we are the child
          case 0:
            // Redirect stdout to file if enabled and the file is writable
            if (fd_stdout > -1)
            {
              dup2(fd_stdout, STDOUT_FILENO);
            }
            else if (special_funcs & STDOUT_TO_FILE)
            {
              // FIXME: print actual filename
              printf("cannot open file for writing\n");
              exit(1);
            }

            // Redirect file to stdin if enabled and the file is readable
            if (fd_stdin > -1)
            {
              dup2(fd_stdin, STDIN_FILENO);
            }
            else if (special_funcs & FILE_TO_STDIN)
            {
              // FIXME: print actual filename
              printf("cannot open file for input\n");
              exit(1);
            }

            // Always ignore SIGTSTP as child
            sigaction(SIGTSTP, &ignore_action, NULL);

            // If we are in background mode, ignore SIGINT as child
            if (special_funcs & EXEC_BG)
            {
              sigaction(SIGINT, &ignore_action, NULL);
            }

            // Attempt to execute user's command
            i = execvp(cline->d[0], cline->d);
            // If command failed to execute, display error message and exit.
            printExecError(i, argv[0]);
            exit(1);
            break;
          // we are the parent
          default:
            // Print background message if needed, otherwise wait for command to complete
            if (special_funcs & EXEC_BG)
            {
              waitpid(spawnpid, &waitpid_status_bg, WNOHANG);
              // FIXME: check whether fork actually worked before putting pids in array
              pushPidts(pids_bg, spawnpid);
              printf("background pid is %i\n", spawnpid);
            }
            else
            {
              waitpid(spawnpid, &waitpid_status, 0);
            }

            // If command was terminated by a signal, print message
            if (WIFSIGNALED(waitpid_status))
            {
              // FIXME: can we detect whether someone just pressed ^C, and print an extra newline if so?
              printf("terminated by signal %i\n", WTERMSIG(waitpid_status));
            }

            // Close files used for I/O redirection if they were opened
            if (fd_stdin > -1)
            {
              close(fd_stdin);
            }

            if (fd_stdout > -1)
            {
              close(fd_stdout);
            }

            break;
        }
      }
    }

    // FIXME: frees need to be in function that can be called upon exit
    // free dynamic memory
    deallocStrs(cline);
    free(cline);
    free(wd);
    free(prompt);
    free(hostname);
    free(*line);
    *line = NULL;
  }
}
