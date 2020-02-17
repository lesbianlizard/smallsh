CC = gcc
CFLAGS = -ggdb -Wall -Wextra -std=gnu11 -lreadline

smallsh: main.c
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -v smallsh
