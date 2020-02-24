CC = gcc
CFLAGS = -ggdb -Wall -Wextra -std=gnu11 -lreadline

smallsh: main.c array.c config.h
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -v smallsh
