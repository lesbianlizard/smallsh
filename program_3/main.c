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

int main(int argc, char** argv)
{
  char *line = readline ("Enter a line: ");
  printf("got the string '%s' from user\n", line);

  if (strcmp(line, "exit") == 0)
  {
    printf("Exiting smallsh.\n");
    exit(0);
  }
  else if (strcmp(line, "cd") == 0)
  {
    printf("I'd love to cd to %s, but I don't know how.\n", line);
  }
}
