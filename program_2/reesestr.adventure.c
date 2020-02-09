#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/mman.h>
#include <fcntl.h>
#define N_ROOMS 7

struct room
{
  char name[10];
  char type[11];
  int nConnsOut;
  struct room* connsOut[6];

  // temporary storage for connections before converting them to the connsOut array
  int nFakeConns;
  char* fakeConns[6];
};


// From example on Canvas
char* getNewestDir()
{
  int newestDirTime = -1; // Modified timestamp of newest subdir examined
  char targetDirPrefix[32] = "reesestr.rooms."; // Prefix we're looking for
  char* newestDirName = malloc(256*sizeof(char)); // Holds the name of the newest dir that contains prefix
  memset(newestDirName, '\0', sizeof(newestDirName));

  DIR* dirToCheck; // Holds the directory we're starting in
  struct dirent *fileInDir; // Holds the current subdir of the starting dir
  struct stat dirAttributes; // Holds information we've gained about subdir

  dirToCheck = opendir("."); // Open up the directory this program was run in

  if (dirToCheck > 0) // Make sure the current directory could be opened
  {
    while ((fileInDir = readdir(dirToCheck)) != NULL) // Check each entry in dir
    {
      if (strstr(fileInDir->d_name, targetDirPrefix) != NULL) // If entry has prefix
      {
        printf("Found the prefex: %s\n", fileInDir->d_name);
        stat(fileInDir->d_name, &dirAttributes); // Get attributes of the entry

        if ((int)dirAttributes.st_mtime > newestDirTime) // If this time is bigger
        {
          newestDirTime = (int)dirAttributes.st_mtime;
          memset(newestDirName, '\0', sizeof(newestDirName));
          strcpy(newestDirName, fileInDir->d_name);
          printf("Newer subdir: %s, new time: %d\n",
              fileInDir->d_name, newestDirTime);
        }
      }
    }
  }

  closedir(dirToCheck); // Close the directory we opened
  printf("Newest entry found is: %s\n", newestDirName);
  return newestDirName;
}

struct room readFileToStruct(char* filename, char* dir)
{
  int i;
  chdir(dir);
  int room_file_fd = open(filename, O_RDONLY);
  int room_file_len = lseek(room_file_fd, 0, SEEK_END);
  char* data = mmap(0, room_file_len, PROT_READ, MAP_PRIVATE, room_file_fd, 0);
  printf("hi there\n");

  printf("memory mapped file because why not: %s", data);

}

struct room* readFilesFromDir(char* dir) 
{
  DIR* dirToCheck; // Holds the directory we're starting in
  struct dirent *fileInDir; // Holds the current subdir of the starting dir
  dirToCheck = opendir(dir); // Open up the directory this program was run in
  int i = 0;

  struct room* rooms = malloc(N_ROOMS*sizeof(struct room));

  if (dirToCheck > 0) // Make sure the current directory could be opened
  {
    while ((fileInDir = readdir(dirToCheck)) != NULL) // Check each entry in dir
    {
      if (fileInDir->d_type == DT_REG)
      {
        printf("Found the file: %s\n", fileInDir->d_name);

        rooms[i] = readFileToStruct(fileInDir->d_name, dir);
        i++;
      }
    }
  }

  closedir(dirToCheck); // Close the directory we opened
  return rooms;
}



int main()
{
  struct room rooms[N_ROOMS];
  char* dir;
  dir = getNewestDir();

  printf("\n");

  readFilesFromDir(dir);


  free(dir);
}
