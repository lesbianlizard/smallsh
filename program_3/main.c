#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <sys/stat.h>
//#include <unistd.h>
//#include <time.h>
//#include <sys/types.h>
#include <sys/stat.h>
//#include <dirent.h>
//#include <sys/mman.h>
//#include <fcntl.h>
//#include <pthread.h>
#include <readline/readline.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <sys/wait.h>
#include <stdint.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <setjmp.h>

#include "string.c"


#define FILE_TO_STDIN  0b10000000
#define STDOUT_TO_FILE 0b01000000
#define EXEC_BG        0b00000001

int bg_enabled = 0;
sigjmp_buf ctrlc_buf;

//see execl,execlp,execv,execvp

void parseCLine(char* line, Strs* arr)
{
  char* whitespace = " \t";
  char* temp;
  size_t arglen;
  //printf("[parseCLine] input is of length %i\n", strlen(line));

  if (strlen(line) == 0)
  {
    return;
  }

  while(! ((line == NULL)))
  {
    // Ignore whitespace
    while (strpbrk(line, whitespace) == line)
    {
      //printf("[parseCLine] ignoring whitespace\n");
      line++;

      if ((line + 1) > line + strlen(line))
      {
       // printf("[parseCLine] Breaking because we fell off\n");
        return;
      }
    }

    if (strpbrk(line, whitespace) == NULL)
    {
      arglen = strlen(line);
    }
    else
    {
      arglen = strpbrk(line, whitespace) - line;
    }

    //printf("[parseCLine] getting a string of length %i\n", arglen);
    temp = malloc((arglen + 1) * sizeof(char));
    memset(temp, 0, (arglen + 1) * sizeof(char));
    strncpy(temp, line, arglen);
    //printf("[parseCLine] made new string temp of size %i\n", strlen(temp));
    //printf("[parseCLine] got string '%s' from line '%s'\n", temp, line);
    pushStrs(arr, temp);

    line = strpbrk(line, whitespace);
  }
}

void printExecError(int exec_code, char* argv0)
{
  char* error_str;
  if (exec_code == EACCES)
  {
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
  //write(STDOUT_FILENO, "\n", 1);
  siglongjmp(ctrlc_buf, 1);
}

void catchSIGTSTP(int signo)
{
  if (bg_enabled == 0)
  {
    //printf("Entering foreground-only mode (& is now ignored) %i\n", bg_enabled);
    bg_enabled = 1;
  }
  else
  {
    //printf("Exiting foreground-only mode %i\n", bg_enabled);
    bg_enabled = 0;
  }
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

int main(int argc, char** argv)
{
  int i, fd_stdin, fd_stdout;
  uint8_t special_funcs = 0;
  Strs* cline;
  char* line, *wd, *prompt, *hostname;
  uid_t uid;
  struct passwd *pw;
  pid_t spawnpid;
  int waitpid_status = 0;
  int waitpid_status_bg = 0;
  int pid_bg = -10;


  struct sigaction ignore_action = {0}, SIGINT_action = {0}, SIGTSTP_action = {0};
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

  while (1)
  {
    cline = malloc(sizeof(Strs));
    initStrs(cline);
    wd = getcwd(NULL, 0);
    // FIXME: random sizes...
    prompt = malloc(100 * sizeof(char));
    memset(prompt, 0, 100 * sizeof(char));
    hostname = malloc(255 * sizeof(char));
    special_funcs = 0;
    fd_stdin = -2;
    fd_stdout = -2;
    
    gethostname(hostname, 255);
    uid = geteuid();
    pw = getpwuid(uid);

    strcat(prompt, pw->pw_name);
    strcat(prompt, "@");
    strcat(prompt, hostname);
    strcat(prompt, " ");
    strcat(prompt, wd);
    strcat(prompt, " : ");

    if (! (sigsetjmp(ctrlc_buf, 1) == 0))
    {
      printf("\n");
    }

    // Check if children have exited
    i = waitpid(pid_bg, &waitpid_status_bg, WNOHANG);
    
    // Check if any background processes have exited
    if (i == pid_bg)
    {
      if (WIFSIGNALED(waitpid_status_bg))
      {
        printf("background pid %i is done: terminated by signal %i\n", pid_bg, WTERMSIG(waitpid_status_bg));
      }
      else if (WIFEXITED(waitpid_status_bg))
      {
        printf("background pid %i is done: exit value %i\n", pid_bg, WEXITSTATUS(waitpid_status_bg));
      }

      pid_bg = -10;
    }
//    else if (i == 0)
//    {
//      printf("background pid %i still running\n", pid_bg);
//    }
//    else
//    {
//      printf("no background process running\n");
//    }

//    while (! (sigsetjmp(ctrlc_buf, 1) == 0))
//    {
//      printf("something happened\n");
//    }

    line = readline(prompt);


    if (line == NULL)
    {
      printf("readline returned null, probably EOF, exiting\n");
      exit(0); // FIXME: deallocate memory and stuff before exiting
    }
    //printf("You entered '%s'\n", line);

    // Parse input line into strings by whitespace
    parseCLine(line, cline);
    // Ignore comments
    if (containsStrs(cline, "#") > -1)
    {
      truncateStrs(cline, containsStrs(cline, "#"));
    }

    for (i = 0; i < cline->used; i++)
    {
      if((strncmp(cline->d[i], "##", 1) == 0))
      {
        truncateStrs(cline, i);
        break;
      }
    }

    for (i = 0; i < cline->used; i++)
    {
      //printf("user provided argument: '%s' of length %i\n", cline->d[i], strlen(cline->d[i]));
    }
    

    if (cline->used > 0)
    {
      if (strcmp(cline->d[0], "exit") == 0)
      {
        printf("Exiting smallsh.\n");
        exit(0);
      }
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
      else if (strcmp(cline->d[0], "status") == 0)
      {
        exitStatus(&waitpid_status);
      }
      else // Run user-specified commands
      {
        // FIXME: do re really need this special_funcs register?
        // Run command in background

        // Redirect file to stdin
        if ((containsStrs(cline, "<") > -1) && (containsStrs(cline, "<") < cline->used - 1))
        {
          special_funcs |= FILE_TO_STDIN;
          fd_stdin = open(cline->d[containsStrs(cline, "<") + 1], O_RDONLY);

          printf("got fd %i for file %s to redirect to stdin\n", fd_stdin, cline->d[containsStrs(cline, "<") + 1]);
        }

        //printf("halp\n");
        // Redirect stdout to file
        if ((containsStrs(cline, ">") > -1))
        {
         // printf("found >\n");

          if ((containsStrs(cline, ">") < cline->used - 1))
          {
          //  printf("found > and it wasn't at end\n");
            special_funcs |= STDOUT_TO_FILE;
            fd_stdout = open(cline->d[containsStrs(cline, ">") + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);

            printf("got fd %i for file %s to redirect to stdin\n", fd_stdout, cline->d[containsStrs(cline, ">") + 1]);
          }
        }

        // Redirect I/O to /dev/null if running in background
        // and if the user didn't alreay specify I/O redirection
        if ((containsStrs(cline, "&") == cline->used - 1) && (bg_enabled == 0))
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

        // If I/O redirection or background symbols are in cline, remove them so that
        // they never get passed to the program to be executed
        if ((containsStrs(cline, "<") > -1) && (containsStrs(cline, "<") < cline->used - 1))
        {
          truncateStrs(cline, containsStrs(cline, "<"));
        }

        if ((containsStrs(cline, ">") > -1) && (containsStrs(cline, ">") < cline->used - 1))
        {
          truncateStrs(cline, containsStrs(cline, ">"));
        }

        if (containsStrs(cline, "&") == cline->used - 1)
        {
          truncateStrs(cline, containsStrs(cline, "&"));
        }
  
        pushStrs(cline, NULL); // add null string to end of args list to make exec() happy
        spawnpid = fork();

        switch (spawnpid)
        {
          case -1:
            printf("fork error, halp!\n");
            break;
          case 0: // we are the child
            //printf("I am the child trying to execute your command '%s'\n", cline->d[0]);
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

            // Always ignore SIGTSTP
            sigaction(SIGTSTP, &ignore_action, NULL);

            // If we are in background mode, ignore SIGINT
            if (special_funcs & EXEC_BG)
            {
              sigaction(SIGINT, &ignore_action, NULL);
            }

            i = execvp(cline->d[0], cline->d);
            printExecError(i, argv[0]);
            //printf("I am the child, and I just failed to execute your command\n");
            exit(1);
            break;
          default: // we are the parent
            //printf("I am the parent waiting for my child %i to exit\n", spawnpid);
            if (special_funcs & EXEC_BG)
            {
              waitpid(spawnpid, &waitpid_status_bg, WNOHANG);
              pid_bg = spawnpid;
              printf("background pid is %i\n", pid_bg);
            }
            else
            {
              waitpid(spawnpid, &waitpid_status, 0);
            }
            //printf("I am the parent, and my child %i just exited\n", spawnpid);

            if (WIFSIGNALED(waitpid_status))
            {
              printf("terminated by signal %i\n", WTERMSIG(waitpid_status));
            }

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

    deallocStrs(cline);
    free(cline);
    free(line);
    free(wd);
    free(prompt);
    free(hostname);
  }
}
