#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <sys/stat.h>
//#include <unistd.h>
//#include <time.h>
//#include <sys/types.h>
//#include <sys/stat.h>
//#include <dirent.h>
//#include <sys/mman.h>
//#include <fcntl.h>
//#include <pthread.h>
#include <readline/readline.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <sys/wait.h>
#include "string.c"

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

void catchSIGINT(int signo) {}

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
  int i;
  Strs* cline;
  char* line, *wd, *prompt, *hostname;
  uid_t uid;
  struct passwd *pw;
  pid_t spawnpid;
  int waitpid_status;

  struct sigaction ignore_action = {0}, SIGINT_action = {0};
  ignore_action.sa_handler = SIG_IGN;
  SIGINT_action.sa_handler = catchSIGINT;
  sigfillset(&SIGINT_action.sa_mask);
  SIGINT_action.sa_flags = 0;

  sigaction(SIGINT, &SIGINT_action, NULL);

  while (1)
  {
    cline = malloc(sizeof(Strs));
    initStrs(cline);
    wd = getcwd(NULL, 0);
    // FIXME: random sizes...
    prompt = malloc(100 * sizeof(char));
    memset(prompt, 0, 100 * sizeof(char));
    hostname = malloc(255 * sizeof(char));
    
    gethostname(hostname, 255);
    uid = geteuid();
    pw = getpwuid(uid);

    strcat(prompt, pw->pw_name);
    strcat(prompt, "@");
    strcat(prompt, hostname);
    strcat(prompt, " ");
    strcat(prompt, wd);
    strcat(prompt, " : ");

    line = readline(prompt);

    if (line == NULL)
    {
      printf("readline returned null, probably EOF, exiting\n");
      exit(0); // FIXME: deallocate memory and stuff before exiting
    }
    //printf("You entered '%s'\n", line);
    parseCLine(line, cline);

    for (i = 0; i < cline->used; i++)
    {
      printf("user provided argument: '%s' of length %i\n", cline->d[i], strlen(cline->d[i]));
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
        pushStrs(cline, NULL); // add null string to end of args list to make exec() happy
        spawnpid = fork();

        switch (spawnpid)
        {
          case -1:
            printf("fork error, halp!\n");
            break;
          case 0: // we are the child
            printf("I am the child trying to execute your command '%s'\n", cline->d[0]);
            execvp(cline->d[0], &cline->d[0]);
            printf("I am the child, and I just failed to execute your command\n");
            exit(0);
            break;
          default: // we are the parent
            printf("I am the parent waiting for my child to exit\n");
            waitpid(spawnpid, &waitpid_status, 0);
            printf("I am the parent, and my child just exited\n");
            exitStatus(&waitpid_status);
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
