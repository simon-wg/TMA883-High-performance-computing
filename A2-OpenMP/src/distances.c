#include <getopt.h>
#include <inttypes.h>
#include <math.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include "helpers.h"

int main(int argc, char **argv)
{
	int opt;
	int threads = 1;
	const char *filepath = "cells";
	int64_t distances[MAX_DIST_INDEX] = { 0 };

	while ((opt = getopt(argc, argv, "+t:")) != -1) {
		switch (opt) {
		case 't':
			threads = strtol(optarg, NULL, 0);
			break;
		default:
			fprintf(stderr, "Usage: %s [-t threads] file\n",
				argv[0]);
			return EXIT_FAILURE;
		}
	}

	struct point p[MAX_SIZE];

	size_t lines = read_file(filepath, p);

	printf("threads=%d, lines=%zu\n", threads, lines);

	if (threads > 0)
		omp_set_num_threads(threads);

#pragma omp parallel for reduction(+ : distances[0 : MAX_DIST_INDEX])
	for (size_t i = 0; i < lines; i++) {
		for (size_t j = i + 1; j < lines; j++) {
			double dist = calculate_distance(&p[i], &p[j]);
			int dist_index = (int)(dist * 100 + 0.5);
			distances[dist_index]++;
		}
	}

	for (size_t i = 0; i < MAX_DIST_INDEX; i++) {
		if (distances[i] == 0)
			continue;

		printf("%05.2f %" PRId64 "\n", (float)i / 100.f, distances[i]);
	}
}
