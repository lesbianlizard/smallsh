#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#define N_ROOMS 7

struct room
{
  char name[10];
  char type[11];
  int nConnsOut;
  struct room* connsOut[6];
};

// Possible room names
char* roomNamesPossible[10] = {
  "dog",
  "cat",
  "giraffe",
  "hyena",
  "elephant",
  "badger",
  "ocelot",
  "meerkat",
  "targ",
  "dov",
};

// Returns a pseudorandom integer between low and high
int getRandInt(int low, int high)
{
  return (rand() % (high - low + 1)) + low;
}

// Returns an int array of length len, where each entry is a random unique integer between low and high
int* makeRandomUniqueArray(int len, int low, int high)
{
  int* result = malloc(len * sizeof(int));
  int i, k, rand, dup_count, nodup;

  // Initialize array with magic low values
  for (i = 0; i < len; i++)
  {
    result[i] = low - 1;
  }

  // For every position in the array, assign a random value
  for (i = 0; i < len; i++)
  {
    nodup = 1;
    rand = getRandInt(low, high);
    result[i] = rand;
    
    while(! nodup == 0)
    {
      dup_count = 0;

      // Check whether current value has already been chosen.
      for (k = 0; k < len; k++)
      {
        if(rand == result[k])
        {
          dup_count++;
        }
      }

      if (dup_count > 1)
      {
        // Current value is a duplicate, reroll
        rand = getRandInt(low, high);
        result[i] = rand;
      }
      else
      {
        // Current value is not a duplicate, continue
        nodup = 0;
      }
    }
  }

  return result;
}

void initRoomNames(struct room* rooms, char** roomNamesPossible)
{
  int i;
  int* indicies = makeRandomUniqueArray(N_ROOMS, 0, 9);

  // For every room in the rooms array, assign a random name to it
  for (i = 0; i < N_ROOMS; i++)
  {
    strcpy(rooms[i].name, roomNamesPossible[indicies[i]]);
  }

  free(indicies);
}


void initRoomTypes(struct room* rooms)
{
  int i;
  int* indicies = makeRandomUniqueArray(2, 0, N_ROOMS - 1);

  // Make every room a mid room
  for (i = 0; i < N_ROOMS; i++)
  {
    strcpy(rooms[i].type, "MID_ROOM");
    rooms[i].nConnsOut = 0;
  }

  // make two random rooms the start and end
  strcpy(rooms[indicies[0]].type, "START_ROOM");
  strcpy(rooms[indicies[1]].type, "END_ROOM");

  free(indicies);
}

// Check whether current room structure satisfies connection parameters
int validateConnections(struct room* rooms)
{
  int i;
  int problem = 0;

  for (i = 0; i < N_ROOMS; i++)
  {
    // If any room has the wrong number of connections, return 1
    if (rooms[i].nConnsOut < 3 || rooms[i].nConnsOut > 6)
    {
      problem = 1;
    }
  }

  return problem;
}

// Reciprocally connects two rooms with each other
void connectRooms(struct room* room1, struct room* room2)
{
  room1->connsOut[room1->nConnsOut] = room2;
  room2->connsOut[room2->nConnsOut] = room1;

  room1->nConnsOut += 1;
  room2->nConnsOut += 1;
}

// Checks whether two rooms have already been connected
int isConnected(struct room* room1, struct room* room2)
{
  int is_connected = 0;
  int i;

  for (i = 0; i < room2->nConnsOut; i++)
  {
    if (strcmp(room1->name, room2->connsOut[i]->name) == 0)
    {
      is_connected = 1;
    }
  }

  return is_connected;
}

// Make connections between rooms until parameters are satisfied
void initRoomConnections(struct room* rooms)
{
  int connect_iter = 0;

  // Check whether rooms are valid
  while (! validateConnections(rooms) == 0)
  {
    // pick two random rooms to try to connect
    int* indicies = makeRandomUniqueArray(2, 0, N_ROOMS - 1);

    if (rooms[indicies[0]].nConnsOut < 6 // room1
     && rooms[indicies[1]].nConnsOut < 6 // and room2 both must be able to accept more connections
     && ! isConnected(&rooms[indicies[0]], &rooms[indicies[1]])) // And they must not already be connected
    {
      connectRooms(&rooms[indicies[0]], &rooms[indicies[1]]);
    }

    connect_iter++;
    free(indicies);
  }
}


// Prints information about a room to stdout, only used for debugging
void printRoomInfo(struct room room)
{
  int i;
  printf("name: %s\ntype: %s\nnConnsOut: %i\nconnections: ", room.name, room.type, room.nConnsOut);
  
  for (i = 0; i < room.nConnsOut; i++)
  {
    printf("%s ", room.connsOut[i]->name);
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

// Writes information from a room struct into a file
void writeRoomToFile(char* filename, struct room* room)
{
  FILE* room_file;
  int i;

  // Open file for writing
  room_file = fopen(filename, "w");
  // Write room name
  fprintf(room_file, "ROOM NAME: %s\n", room->name);

  // Write each connection on a separate line
  for (i = 0; i < room->nConnsOut; i++)
  {
    fprintf(room_file, "CONNECTION %i: %s\n", i, room->connsOut[i]->name);
  }

  // Write room type
  fprintf(room_file, "ROOM TYPE: %s\n", room->type);

  // Done, close file
  fclose(room_file);
}


int main()
{
  // Seed pseudorandom number generator with system clock
  srand(time(NULL));
  char room_filename[100];
  char dirname[50] = "reesestr.rooms.";
  char pid[10];
  struct room rooms[N_ROOMS];
  int i;

  // convert PID integer to char* with sprintf
  sprintf(pid, "%i", getpid()); 
  strcat(dirname, pid);
  // make directory to store files in
  mkdir(dirname, 0755);

  // Build the room structs
  initRoomNames(rooms, roomNamesPossible);
  initRoomTypes(rooms);
  initRoomConnections(rooms);

  for (i = 0; i < N_ROOMS; i++)
  {
    // Build the filename for each room
    strcpy(room_filename, dirname);
    strcat(room_filename, "/");
    strcat(room_filename, rooms[i].name);
    strcat(room_filename, "_room");
    
    // Write the files
    writeRoomToFile(room_filename, &rooms[i]); 
  }

  return 0;
}
