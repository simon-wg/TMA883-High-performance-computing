#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "helpers.h"

size_t read_file(const char *filepath, struct point *result_points)
{
	FILE *f = fopen(filepath, "r");
	size_t points = 0;

	if (f == NULL) {
		fprintf(stderr, "Unable to open file\n");
		exit(EXIT_FAILURE);
	}

	while (points < MAX_SIZE && parse_line(f, &result_points[points])) {
		points++;
	}

	fclose(f);
	return points;
}

int parse_line(FILE *files, struct point *p)
{
	char line[32];

	if (fgets(line, sizeof line, files) == NULL)
		return 0;

	float x, y, z;

	if (sscanf(line, "%f %f %f", &x, &y, &z) != 3) {
		fprintf(stderr, "Invalid coordinates: %s\n", line);
		exit(EXIT_FAILURE);
	}

	p->x = (uint16_t)(x * 1000);
	p->y = (uint16_t)(y * 1000);
	p->z = (uint16_t)(z * 1000);

	return 1;
}

int calculate_distance_index(struct point *p1, struct point *p2)
{
	int32_t dx = p1->x - p2->x;
	int32_t dy = p1->y - p2->y;
	int32_t dz = p1->z - p2->z;

	int64_t distance_squared = dx * dx + dy * dy + dz * dz;
	double dist = sqrt((double)distance_squared);
	return (int)(dist / 10 + 0.5);
}
