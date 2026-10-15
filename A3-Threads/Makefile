CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -O2 -g -march=native -lm

.PHONY: all clean

all: newton

newton: newton.c file.c newton.h file.h
	$(CC) $(CFLAGS) newton.c file.c -o newton

clean:
	rm -rf newton *.png *.ppm perf.data*
