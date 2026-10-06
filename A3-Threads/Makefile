CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -O2 -g -march=native -lm

.PHONY: all clean

all:
	$(CC) $(CFLAGS) newton.c -o newton

clean:
	rm -rf newton
