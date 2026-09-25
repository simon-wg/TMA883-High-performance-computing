#include <math.h>
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

	p->x = (int16_t)(x * 1000);
	p->y = (int16_t)(y * 1000);
	p->z = (int16_t)(z * 1000);

	return 1;
}

double calculate_distance(struct point *p1, struct point *p2)
{
	double dx = ((double)p2->x) / 1000. - ((double)p1->x) / 1000.;
	double dy = ((double)p2->y) / 1000. - ((double)p1->y) / 1000.;
	double dz = ((double)p2->z) / 1000. - ((double)p1->z) / 1000.;
	return sqrt(dx * dx + dy * dy + dz * dz);
}
