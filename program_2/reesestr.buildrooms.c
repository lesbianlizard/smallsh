#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

enum roomType{START_ROOM, END_ROOM, MID_ROOM};
static int N_ROOMS = 7;

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

  for (i = 0; i < 7; i++)
  {
   // struct room temp;
    //rooms[i] = temp;
    j = getRandInt(0, 9);
    //dup check here
    strcpy(rooms[i].name, roomNamesPossible[j]);
  }
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

  for (i = 0; i < N_ROOMS; i++)
  {
    printRoomInfo(rooms[i]);
    printf("\n");
  }
//  initRoomTypes(rooms);
//  connectRooms(rooms);
//
//  int i;
//  for (i = 0; i < N_ROOMS; i++)
//  {
//    writeRoomToFile(rooms[i]); 
//  }

  return 0;
}
