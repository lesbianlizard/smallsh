C = gcc
CFLAGS = -std=gnu90 -Wall

buildrooms: reesestr.buildrooms.c
	$(C) $(CFLAGS) -o reesestr.buildrooms reesestr.buildrooms.c

adventure: reesestr.adventure.c
	$(C) $(CFLAGS) -lpthread -o reesestr.adventure reesestr.adventure.c

rooms: buildrooms
	./reesestr.buildrooms

play: adventure
	./reesestr.adventure

.PHONY: clean
clean:
	rm -vrf reesestr.buildrooms reesestr.rooms.PIDHERE
