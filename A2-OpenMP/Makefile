CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -O2 -fopenmp -g -march=native -Iinc
LDFLAGS = -fopenmp

SOURCES = $(wildcard src/*.c)
OBJECTS = $(patsubst src/%.c,build/%.o,$(SOURCES))
DEPENDENCIES = $(OBJECTS:.o=.d)

.PHONY: all clean stack distances build

all: distances

distances: $(OBJECTS)
	$(CC) $(LDFLAGS) $^ -o $@

stack: $(OBJECTS)
	$(CC) $(CFLAGS) -fstack-usage $(SOURCES) -o distances

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build:
	mkdir -p $@

clean:
	rm -rf build distances *.su
