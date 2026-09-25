#pragma once
#include <stdint.h>
#include <stdio.h>

#define MAX_SIZE 100000
#define MAX_DIST_INDEX 3465

struct point {
	int16_t x;
	int16_t y;
	int16_t z;
};

size_t read_file(const char *filepath, struct point *result_points);

int parse_line(FILE *file, struct point *p);

double calculate_distance(struct point *p1, struct point *p2);
