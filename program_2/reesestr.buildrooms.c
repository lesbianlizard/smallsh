#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#define N_ROOMS 7

enum roomType{MID_ROOM, START_ROOM, END_ROOM};

struct room
{
  char name[10];
  enum roomType type;
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

void initRoomNames(struct room* rooms, char** roomNamesPossible)
{
  int i;
  int j;
  int k;
  int dont_duplicate[N_ROOMS] = { -1, -1, -1, -1, -1, -1, -1};
  int not_a_duplicate = 1; // 0 = true, 1 = false
  int j_count = 0;

  // For every room in the rooms array, assign a random name to it
  for (i = 0; i < N_ROOMS; i++)
  {
    not_a_duplicate = 1;
    j = getRandInt(0, 9);
    dont_duplicate[i] = j;
    
    // Until we've verified that the room name picked isn't a duplicate
    while (! not_a_duplicate == 0)
    {
      j_count = 0;

      // For all elements in the dont_duplicate array
      for (k = 0; k < N_ROOMS; k++)
      {
        if(j == dont_duplicate[k])
        {
          j_count++;
        }
      }

      // Break out of the loop or not, depending whether we've found a non-duplicate name
      if (j_count > 1)
      {
        j = getRandInt(0, 9);
        dont_duplicate[i] = j;
      }
      else
      {
        not_a_duplicate = 0;
      }
    }

    strcpy(rooms[i].name, roomNamesPossible[j]);
  }
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

void initRoomTypes(struct room* rooms)
{
  int i;
  int* indicies = makeRandomUniqueArray(2, 0, N_ROOMS - 1);

  for (i = 0; i < N_ROOMS; i++)
  {
    rooms[i].type = MID_ROOM;
  }

  rooms[indicies[0]].type = START_ROOM;
  rooms[indicies[1]].type = END_ROOM;
}

void printRoomInfo(struct room room)
{
  printf("name: %s\ntype: %i\nnConnsOut: %i\n", room.name, room.type, room.nConnsOut);
}


int main()
{
  srand(time(NULL));
  char* dirname = "reesestr.rooms.PIDHERE";
  mkdir(dirname, 0755);
  int i;

  struct room rooms[N_ROOMS];
  initRoomNames(rooms, roomNamesPossible);
  initRoomTypes(rooms);

  for (i = 0; i < N_ROOMS; i++)
  {
    printRoomInfo(rooms[i]);
    printf("\n");
  }
//  connectRooms(rooms);
//
//  int i;
//  for (i = 0; i < N_ROOMS; i++)
//  {
//    writeRoomToFile(rooms[i]); 
//  }

  return 0;
}
