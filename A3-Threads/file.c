#include "file.h"

#include <stdlib.h>
#include <string.h>

void write_attractors_header(FILE *f)
{
	char header[64];
	int len = sprintf(header, "P3\n%zu %zu\n255\n", size, size);
	fwrite(header, 1, len, f);
}

void write_attractors_to_ppm(FILE *f, int row, char *buffer)
{
	for (size_t col = 0; col < size; col++) {
		const char *color = colors[attractors[row][col]];
		memcpy(&buffer[12 * col], color, 12);
	}
	fwrite(buffer, 1, 12 * size, f);
}

void write_convergence_header(FILE *f)
{
	char header[64];
	int len = sprintf(header, "P3\n%zu %zu\n%d\n", size, size,
			  MAX_ITERATIONS);
	fwrite(header, 1, len, f);
}

void write_convergence_to_ppm(FILE *f, int row, char *buffer)
{
	for (size_t col = 0; col < size; col++) {
		uint8_t steps = convergence[row][col];
		memcpy(&buffer[9 * col], shades[steps], 9);
	}
	fwrite(buffer, 1, 9 * size, f);
}

int write(void *_)
{
	size_t next_row = 0;
	char *buffer = malloc(12 * size + 1);
	FILE *fa = fopen(attractors_name, "w");
	FILE *fc = fopen(convergence_name, "w");
	write_attractors_header(fa);
	write_convergence_header(fc);
	for (;;) {
		if (next_row == size)
			break;
		if (ready[next_row]) {
			write_attractors_to_ppm(fa, next_row, buffer);
			write_convergence_to_ppm(fc, next_row, buffer);
			next_row++;
		}
	}
	fclose(fa);
	fclose(fc);
	free(buffer);
	return 0;
}
