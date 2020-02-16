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

int main(int argc, char** argv)
{
  int i;
  Strs* cline;
  char* line, *wd, *prompt;

  while (1)
  {
    cline = malloc(sizeof(Strs));
    initStrs(cline);
    wd = getcwd(NULL, 0);
    prompt = malloc(100 * sizeof(char));
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
    

    if (strcmp(cline->d[0], "exit") == 0)
    {
      printf("Exiting smallsh.\n");
      exit(0);
    }
    else if (strcmp(cline->d[0], "cd") == 0)
    {
      printf("I'd love to cd to %s, but I don't know how.\n", cline->d[1]);
    }

    deallocStrs(cline);
    free(cline);
    free(line);
    //free(wd);
    //free(prompt);
  }
}
