#include "file.h"
#include "newton.h"

#include <assert.h>
#include <complex.h>
#include <getopt.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>
#include <time.h>

uint8_t **attractors;
uint8_t **convergence;
bool *ready;
char attractors_name[32];
char convergence_name[32];

char shades[MAX_ITERATIONS + 1][10];

int d = 0;
size_t size = 0;
int thread_count = 1;
complex double roots[N_ROOTS];

//x_{k+1} = x_k - f(x_k) / f'(x_k)

// x_n = x/d * (d - 1 + (1/x^d))

static complex double newton_step(complex double x)
{
	// x_n = x - (x^d - 1) / (d * x^(d-1))
	// x_n = x_n - x_n / d + 1. / (d * x_n ^ (d-1));
	complex double x2, x4;
	switch (d) {
	case 1:
		return 1.0;
	case 2:
		return 0.5 * x + 0.5 / x;
	case 3:
		x2 = x * x;
		return 2.0 / 3.0 * x + 1.0 / 3.0 / x2;
	case 4:
		x2 = x * x;
		return 3.0 / 4.0 * x + 1.0 / 4.0 / (x2 * x);
	case 5:
		x2 = x * x;
		x4 = x2 * x2;
		return 4.0 / 5.0 * x + 1.0 / 5.0 / x4;
	case 6:
		x2 = x * x;
		x4 = x2 * x2;
		return 5.0 / 6.0 * x + 1.0 / 6.0 / (x4 * x);
	case 7:
		x2 = x * x;
		x4 = x2 * x2;
		return 6.0 / 7.0 * x + 1.0 / 7.0 / (x4 * x2);
	case 8:
		x2 = x * x;
		x4 = x2 * x2;
		return 7.0 / 8.0 * x + 1.0 / 8.0 / (x4 * x2 * x);
	case 9:
		x2 = x * x;
		x4 = x2 * x2;
		return 8.0 / 9.0 * x + 1.0 / 9.0 / (x4 * x4);
	default:
		exit(EXIT_FAILURE);
	}
}

static ssize_t dist_to_roots_too_small(complex double x)
{
	ssize_t i;
	for (i = 0; i < d; i++) {
		double re = creal(x) - creal(roots[i]);
		double im = cimag(x) - cimag(roots[i]);
		if (re * re + im * im < 0.001 * 0.001) {
			return i;
		}
	}
	return -i;
}

static inline bool dist_to_origin_too_small(complex double x)
{
	double re = creal(x), im = cimag(x);
	return re * re + im * im < 0.001 * 0.001;
}

static inline bool parts_too_large(complex double x)
{
	return creal(x) > 100000.0 || creal(x) < -100000.0 ||
	       cimag(x) > 100000.0 || cimag(x) < -100000.0;
}

static void init_roots()
{
	for (int i = 0; i < d; i++) {
		roots[i] =
			CMPLX(cos((2 * M_PI * i) / d), sin((2 * M_PI * i) / d));
	}
}

static void find_root(complex double x_0, struct result *res)
{
	complex double x_n = x_0;
	int r;
	for (int i = 0; i < MAX_ITERATIONS; ++i) {
		if ((r = dist_to_roots_too_small(x_n)) > -1) {
			res->attractor = r;
			res->steps = i;
			return;
		}
		if (dist_to_origin_too_small(x_n)) {
			res->attractor = 9;
			res->steps = i;
			return;
		}
		if (parts_too_large(x_n)) {
			res->attractor = 10;
			res->steps = i;
			return;
		}
		x_n = newton_step(x_n);
	}
	res->attractor = 11;
	res->steps = MAX_ITERATIONS;
	return;
}

static int find_roots(void *arg)
{
	int thread_id = *(int *)arg;
	struct result res = {};
	for (size_t row = thread_id; row < size; row += thread_count) {
		double im = 2.0 - 4.0 * row / (size - 1);
		for (size_t col = 0; col < size; col++) {
			double re = -2.0 + 4.0 * col / (size - 1);
			find_root(CMPLX(re, im), &res);
			attractors[row][col] = res.attractor;
			convergence[row][col] = res.steps;
		}
		ready[row] = true;
	}
	return 0;
}

int main(int argc, char **argv)
{
	int opt;

	while ((opt = getopt(argc, argv, "+t:l:")) != -1) {
		switch (opt) {
		case 't':
			thread_count = strtol(optarg, NULL, 0);
			break;
		case 'l':
			size = strtol(optarg, NULL, 0);
			break;
		default:
			fprintf(stderr,
				"Usage: %s <-t threads> <-l size> <exponent> file\n",
				argv[0]);
			return EXIT_FAILURE;
		}
	}

	d = strtol(argv[optind], NULL, 0);
	sprintf(attractors_name, "newton_attractors_x%d.ppm", d);
	sprintf(convergence_name, "newton_convergence_x%d.ppm", d);
	init_roots();
	for (int i = 0; i <= MAX_ITERATIONS; ++i)
		sprintf(shades[i], "%02d %02d %02d\n", i, i, i);

	attractors = (uint8_t **)malloc(sizeof *attractors * size);
	uint8_t *attractors_entries =
		(uint8_t *)malloc(sizeof *attractors_entries * size * size);
	for (size_t i = 0; i < size; ++i)
		attractors[i] = &attractors_entries[i * size];

	convergence = (uint8_t **)malloc(sizeof *convergence * size);
	uint8_t *convergence_entries =
		(uint8_t *)malloc(sizeof *convergence_entries * size * size);
	for (size_t i = 0; i < size; ++i)
		convergence[i] = &convergence_entries[i * size];

	ready = (bool *)malloc(sizeof(bool) * size);
	for (size_t i = 0; i < size; ++i)
		ready[i] = false;

	thrd_t thread[thread_count];
	int ids[thread_count];

	for (int t = 0; t < thread_count; t++) {
		ids[t] = t;
		thrd_create(&thread[t], find_roots, &ids[t]);
	}

	thrd_t writing_thread;
	thrd_create(&writing_thread, write, NULL);

	for (int t = 0; t < thread_count; t++)
		thrd_join(thread[t], NULL);

	thrd_join(writing_thread, NULL);

	free(ready);
	free(convergence_entries);
	free(convergence);
	free(attractors_entries);
	free(attractors);
}
