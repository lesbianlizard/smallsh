#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#define N_ROOMS 7

//enum roomType{MID_ROOM, START_ROOM, END_ROOM};

struct room
{
  char name[10];
  char type[11];
  int nConnsOut;
  struct room* connsOut[6];
};

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

int getRandInt(int low, int high)
{
  return (rand() % (high - low + 1)) + low;
}

int* makeRandomUniqueArray(int len, int low, int high)
{
  int* result = malloc(len * sizeof(int));
  int i;
  int j;
  int k;
  int rand;
  int dup_count;
  int nodup;

  for (i = 0; i < len; i++)
  {
    result[i] = low - 1;
  }

  for (i = 0; i < len; i++)
  {
    nodup = 1;
    rand = getRandInt(low, high);
    result[i] = rand;
    
    while(! nodup == 0)
    {
      dup_count = 0;

      for (k = 0; k < len; k++)
      {
        if(rand == result[k])
        {
          dup_count++;
        }
      }

      if (dup_count > 1)
      {
        rand = getRandInt(low, high);
        result[i] = rand;
      }
      else
      {
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

  for (i = 0; i < N_ROOMS; i++)
  {
    strcpy(rooms[i].type, "MID_ROOM");
    rooms[i].nConnsOut = 0;
  }

  strcpy(rooms[indicies[0]].type, "START_ROOM");
  strcpy(rooms[indicies[1]].type, "END_ROOM");

  free(indicies);
}

// 0 good
int validateConnections(struct room* rooms)
{
  int i;
  int problem = 0;

  for (i = 0; i < N_ROOMS; i++)
  {
    if (rooms[i].nConnsOut < 3 || rooms[i].nConnsOut > 6)
    {
      fprintf(stdout, "Invalid nConnsOut=%i for %s room\n", rooms[i].nConnsOut, rooms[i].name);
      problem = 1;
    }
  }

  return problem;
}

void connectRooms(struct room* room1, struct room* room2)
{
  room1->connsOut[room1->nConnsOut] = room2;
  room2->connsOut[room2->nConnsOut] = room1;

  room1->nConnsOut += 1;
  room2->nConnsOut += 1;
}

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

void initRoomConnections(struct room* rooms)
{
  int i;
  int connect_iter = 0;

  while (! validateConnections(rooms) == 0)
  {
    printf("connection iteration %i\n", connect_iter);
    int* indicies = makeRandomUniqueArray(2, 0, N_ROOMS - 1);

    if (rooms[indicies[0]].nConnsOut < 6
     && rooms[indicies[1]].nConnsOut < 6
     && ! isConnected(&rooms[indicies[0]], &rooms[indicies[1]]))
    {
      connectRooms(&rooms[indicies[0]], &rooms[indicies[1]]);
    }

    connect_iter++;
    free(indicies);
    //printRoomsInfo(rooms);
  }
}


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

void printRoomsInfo(struct room* rooms)
{
  int i;

  for (i = 0; i < N_ROOMS; i++)
  {
    printRoomInfo(rooms[i]);
    printf("\n");
  }
}

void writeRoomToFile(char* filename, struct room* room)
{
 FILE* room_file;
 int i;
 room_file = fopen(filename, "w");
 fprintf(room_file, "ROOM NAME: %s\n", room->name);

 for (i = 0; i < room->nConnsOut; i++)
 {
  fprintf(room_file, "CONNECTION %i: %s\n", i, room->connsOut[i]->name);
 }

 fprintf(room_file, "ROOM TYPE: %s\n", room->type);

 fclose(room_file);
}


int main()
{
  srand(time(NULL));
  char room_filename[100];
  char dirname[50] = "reesestr.rooms.";
  char pid[10];

  sprintf(pid, "%i", getpid()); 
  strcat(dirname, pid);
  mkdir(dirname, 0755);
  int i;

  struct room rooms[N_ROOMS];
  initRoomNames(rooms, roomNamesPossible);
  initRoomTypes(rooms);
  initRoomConnections(rooms);

  for (i = 0; i < N_ROOMS; i++)
  {
    printRoomInfo(rooms[i]);
    printf("\n");
  }

  for (i = 0; i < N_ROOMS; i++)
  {
    strcpy(room_filename, dirname);
    strcat(room_filename, "/");
    strcat(room_filename, rooms[i].name);
    strcat(room_filename, "_room");
    printf("room_filename: %s\n", room_filename);
    
    writeRoomToFile(room_filename, &rooms[i]); 
  }

  return 0;
}
