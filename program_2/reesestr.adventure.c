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
#include <pthread.h>
#define N_ROOMS 7

struct room
{
  char name[10];
  char type[11];
  int nConnsOut;
  char* connsOut[6];
};

// Prints information about a room to stdout, only used for debugging
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

// Prints information about an array of rooms to stdout, only used for debugging
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
        stat(fileInDir->d_name, &dirAttributes); // Get attributes of the entry

        if ((int)dirAttributes.st_mtime > newestDirTime) // If this time is bigger
        {
          newestDirTime = (int)dirAttributes.st_mtime;
          memset(newestDirName, '\0', sizeof(newestDirName));
          strcpy(newestDirName, fileInDir->d_name);
        }
      }
    }
  }

  closedir(dirToCheck); // Close the directory we opened
  return newestDirName;
}

// Converts a room file into a room struct
void readFileToStruct(char* filename, char* dir, struct room* new_room)
{
  int i=0;
  int in_len;
  char* where;

  // Change working directory to be able to use relative file paths
  chdir(dir);
   
  // I was looking for a way to read an entire file into a C string, and discovered
  // that directly memory-mapping files is very simple
  int room_file_fd = open(filename, O_RDONLY);
  int room_file_len = lseek(room_file_fd, 0, SEEK_END);
  // Effectively, room_file is a C string containing the entire file at dir/filename.
  char* room_file = mmap(0, room_file_len, PROT_READ, MAP_PRIVATE, room_file_fd, 0);

  // Look for the second space on line 1
  where = strpbrk(room_file, " ");
  where = strpbrk(where + 1, " ");
  where++; // go one character beyond that space
  in_len = strcspn(where, "\n"); // the length of the "name" string is from here to the end of the line
  strncpy(new_room->name, where, in_len);

  // As long as the string "CONNECTION" appear somewhere ahead of the current position,
  // continue reading connection names
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

  // Finally, read the room type
  where = strpbrk(where, " ");
  where = strpbrk(where + 1, " ");
  where++;
  in_len = strcspn(where, "\n");
  strncpy(new_room->type, where, in_len);

  close(room_file_fd);
  chdir("..");
}

// Heavily modified from example on Canvas
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
        readFileToStruct(fileInDir->d_name, dir, &rooms[i]);
        i++;
      }
    }
  }

  closedir(dirToCheck); // Close the directory we opened
}

struct room* getStartRoom(struct room* rooms)
{
  int i;

  for (i = 0; i < N_ROOMS; i++)
  {
    // Find the start room and return it
    if (strcmp(rooms[i].type, "START_ROOM") == 0)
    {
      return &rooms[i];
    }
  }
}

struct room* getRoomByName(char* name, int name_len, struct room* rooms)
{
  int i;

  for (i = 0; i < N_ROOMS; i++)
  {
    // Find a room with the speficied name and return it.
    if (strncmp(rooms[i].name, name, name_len) == 0)
    {
      return &rooms[i];
    }
  }
}

int roomConnectsTo(char* name, struct room* room, int len)
{
  int i;

  for (i = 0; i < room->nConnsOut; i++)
  {
    // If specified name is in the room's list of connections, return 0
    if (strncmp(room->connsOut[i], name, len) == 0)
    {
      return 0;
    }
  }

  // If not, return 1
  return 1;
}

void* writeTimeToFile(void* mutex)
{
  FILE* room_file;
  time_t t;
  struct tm* tmp;
  char current_time[50];
  pthread_mutex_lock(mutex);
  pthread_testcancel();

  t = time(NULL);
  tmp = localtime(&t);
  strftime(current_time, sizeof(current_time), "%l:%M%p, %A, %B %d, %Y", tmp);

  room_file = fopen("currentTime.txt", "w");
  fprintf(room_file, "%s", current_time);
  fclose(room_file);

  pthread_mutex_unlock(mutex);
  return NULL;
}

void printTimeFromFile()
{
  int file_fd = open("currentTime.txt", O_RDONLY);
  int file_len = lseek(file_fd, 0, SEEK_END);
  char* file = mmap(0, file_len, PROT_READ, MAP_PRIVATE, file_fd, 0);

  printf("%s\n\n", file);

  close(file_fd);
}

int main()
{
  int i, j,  n_steps_taken = 0;
  struct room* rooms = malloc(N_ROOMS*sizeof(struct room));
  struct room* current_room;
  size_t userin_s = 100;
  size_t readlen;
  char* dir; 
  char* userin = malloc(userin_s * sizeof(char));
  char userin2[100];
  char* visited_rooms[100];

  // create a pthread thread
  pthread_t threads[1];
  // create a mutex and lock it
  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
  pthread_mutex_lock(&mutex);


  // Determine newest directory and read them into memory
  dir = getNewestDir();
  readFilesFromDir(dir, rooms);
  //printRoomsInfo(rooms);

  // Start in the start room.
  current_room = getStartRoom(rooms);

  // Spawn the initial time thread
  pthread_create(&threads[0], NULL, writeTimeToFile, &mutex);
  
  // Until user has found the end room, loop:
  while (strcmp(current_room->type, "END_ROOM") != 0)
  {
    memset(userin2, "\0", 100*sizeof(char));
    memset(userin, "\0", 100*sizeof(char));


    printf("CURRENT LOCATION: %s\nPOSSIBLE CONNECTIONS: ", current_room->name);

    // Print connections from current room
    for (i = 0; i < current_room->nConnsOut; i++)
    {
      printf("%s", current_room->connsOut[i]);

      // Fancy punctuation
      if (i == current_room->nConnsOut - 1)
      {
        printf(".\n");
      }
      else
      {
        printf(", ");
      }
    }

    printf("WHERE TO? >");

    // Read something from user
    readlen = getline(&userin, &userin_s, stdin);
    // Truncate the newline character from end of string
    strncpy(userin2, userin, readlen - 1);
    printf("\n");

    // Check whether the user's input is the name of one of the current room's connections
    if (strncmp(userin2, "time", readlen - 1) == 0)
    {
      // Unlock the mutex, allowing the time thread to proceed
      pthread_mutex_unlock(&mutex);
      // Wait for the thread to complete
      pthread_join(threads[0], NULL);

      // Read from the file just created
      printTimeFromFile();

      // Relock the mutex
      pthread_mutex_lock(&mutex);
      // Spawn a new time thread
      pthread_create(&threads[0], NULL, writeTimeToFile, &mutex);
    }
    else
    {
      if (roomConnectsTo(userin2, current_room, readlen - 1) == 0)
      {
        // Go to that room
        current_room = getRoomByName(userin2, readlen - 1, rooms);
        n_steps_taken++;

        // Add room we just entered to visited_rooms array
        if (n_steps_taken > 0)
        {
          visited_rooms[n_steps_taken - 1] = malloc(10 * sizeof(char));
          strcpy(visited_rooms[n_steps_taken - 1], current_room->name);
        }
      }
      else
      {
        // Do nothing
        printf("HUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
      }
    }
  }

  // You won!
  printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\nYOU TOOK %i STEPS. YOUR PATH TO VICTORY WAS:\n", n_steps_taken);

  // Print the path to victory
  for (i = 0; i < n_steps_taken; i++)
  {
    printf("%s\n", visited_rooms[i]);
    free(visited_rooms[i]);
  }

  // deallocate dynamic memory
  for (i = 0; i < N_ROOMS; i++)
  {
    for (j = 0; j < rooms[i].nConnsOut; j++)
    {
      free(rooms[i].connsOut[j]);
    }
  }

  free(rooms);
  free(dir);
  free(userin);

  pthread_cancel(threads[0]);
  pthread_mutex_unlock(&mutex);
  pthread_join(threads[0], NULL);

  return 0;
}
