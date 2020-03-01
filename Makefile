CC = gcc
CFLAGS = -ggdb -Wall -Wextra -std=gnu11 -lreadline

smallsh: main.c array.c config.h
	$(CC) $(CFLAGS) $< -o $@

reesestr_program_3.zip: main.c array.c config.h Makefile README
	zip $@ $^

.PHONY: clean
clean:
	rm -v smallsh
