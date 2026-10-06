#include <getopt.h>
#include <inttypes.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include "helpers.h"
#include <time.h>

int main(int argc, char **argv)
{
	int opt;
	int threads = 1;
	const char *filepath = "cells";
	uint64_t distances[MAX_DIST_INDEX] = { 0 };

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

	if (threads > 0)
		omp_set_num_threads(threads);

	struct point p[MAX_SIZE];

	size_t lines = read_file(filepath, p);

#pragma omp parallel for reduction(+ : distances[0 : MAX_DIST_INDEX]) \
	schedule(dynamic)
	for (size_t i = 0; i < lines; i++) {
		for (size_t j = i + 1; j < lines; j++) {
			int dist_index = calculate_distance_index(&p[i], &p[j]);
			distances[dist_index]++;
		}
	}

	for (size_t i = 0; i < MAX_DIST_INDEX; i++) {
		if (distances[i] == 0)
			continue;

		printf("%05.2f %" PRId64 "\n", (float)i / 100.f, distances[i]);
	}
}
