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
  char* connsOut[6];
};

void printRoomInfo(struct room room)
{
  int i;
  printf("name: %s\ntype: %s\nnConnsOut: %i\nconnections: ", room.name, room.type, room.nConnsOut);
  
  for (i = 0; i < room.nConnsOut; i++)
  {
    printf("%s ", room.connsOut[i]);
  }

  printf("\n");
}

void printRoomsInfo(struct room* rooms)
{
  int i;

  for (i = 0; i < N_ROOMS; i++)
  {
    printRoomInfo(rooms[i]);
    printf("\n");
  }
}


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

void readFileToStruct(char* filename, char* dir, struct room* new_room)
{
  int i=0;
  int in_len;
  char* where;

  chdir(dir);
   
  // I was looking for a way to read an entire file into a C string, and discovered
  // that directly memory-mapping files was very simple
  int room_file_fd = open(filename, O_RDONLY);
  int room_file_len = lseek(room_file_fd, 0, SEEK_END);
  char* room_file = mmap(0, room_file_len, PROT_READ, MAP_PRIVATE, room_file_fd, 0);

  // Look for the second space on line 1
  where = strpbrk(room_file, " ");
  where = strpbrk(where + 1, " ");
  where++;
  in_len = strcspn(where, "\n");
  strncpy(new_room->name, where, in_len);

  while (strstr(where, "CONNECTION") != 0)
  {
    where = strpbrk(where, " ");
    where = strpbrk(where + 1, " ");
    where++;
    in_len = strcspn(where, "\n");
    new_room->connsOut[i] = malloc(10*sizeof(char));
    strncpy(new_room->connsOut[i], where, in_len);
    new_room->nConnsOut++;
    i++;
  }

  where = strpbrk(where, " ");
  where = strpbrk(where + 1, " ");
  where++;
  in_len = strcspn(where, "\n");
  strncpy(new_room->type, where, in_len);

  //printRoomInfo(*new_room);

  close(room_file_fd);
}

void readFilesFromDir(char* dir, struct room* rooms) 
{
  DIR* dirToCheck; // Holds the directory we're starting in
  struct dirent *fileInDir; // Holds the current subdir of the starting dir
  dirToCheck = opendir(dir); // Open up the directory this program was run in
  int i = 0;

  if (dirToCheck > 0) // Make sure the current directory could be opened
  {
    while ((fileInDir = readdir(dirToCheck)) != NULL) // Check each entry in dir
    {
      if (fileInDir->d_type == DT_REG)
      {
        printf("Found the file: %s\n", fileInDir->d_name);

        readFileToStruct(fileInDir->d_name, dir, &rooms[i]);
        printf("\n");
        i++;
      }
    }
  }

  closedir(dirToCheck); // Close the directory we opened
  return rooms;
}


int main()
{
  int i, j;
  struct room* rooms = malloc(N_ROOMS*sizeof(struct room));
  char* dir;
  dir = getNewestDir();

//  for (i = 0; i < N_ROOMS; i++)
//  {
//    for (j = 0; j < 6; j++)
//    {
//      rooms[i].connsOut[j] = malloc(10*sizeof(char));
//    }
//  }

  printf("\n");
  //printRoomInfo(rooms[0]);

  readFilesFromDir(dir, rooms);
  printRoomsInfo(rooms);



  for (i = 0; i < N_ROOMS; i++)
  {
    for (j = 0; j < rooms[i].nConnsOut; j++)
    {
      free(rooms[i].connsOut[j]);
    }
  }
  free(rooms);
  free(dir);
}
