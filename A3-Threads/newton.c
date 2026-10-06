#include <assert.h>
#include <complex.h>
#include <getopt.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_ITERATIONS 50

uint8_t **attractors;
uint8_t **convergences;

int d = 0;
complex double *roots;

complex double f(complex double x)
{
	// WE CAN NOT USE THIS IN THE FINAL VERSION
	return cpow(x, d) - 1;
}

complex double fprime(complex double x)
{
	// WE CAN NOT USE THIS IN THE FINAL VERSION
	return d * cpow(x, d - 1);
}

double dist_between(complex double x, complex double y)
{
	double da = creal(x) - creal(y);
	double db = cimag(x) - cimag(y);
	// THIS MIGHT BE SLOW
	return sqrt(da * da + db * db);
}

bool dist_to_roots_too_small(complex double x)
{
	for (size_t i = 0; i < d; i++) {
		if (dist_between(x, roots[i]) < 0.001) {
			return true;
		}
	}
	return false;
}

bool dist_to_origin_too_small(complex double x)
{
	return sqrt(creal(x) * creal(x) + cimag(x) * cimag(x)) < 0.001;
}

bool parts_too_large(complex double x)
{
	return creal(x) > 100000. || cimag(x) > 100000.;
}

void init_roots()
{
	for (int i = 0; i < d; i++) {
		roots[i] =
			CMPLX(cos((2 * M_PI * i) / d), sin((2 * M_PI * i) / d));
	}
}

complex double iterate(complex double x_0)
{
	complex double x_n = x_0;
	for (int i = 0; i < MAX_ITERATIONS; ++i) {
		if (dist_to_roots_too_small(x_n) ||
		    dist_to_origin_too_small(x_n) || parts_too_large(x_n)) {
			break;
		}
	}
	return x_n;
}

int main(int argc, char **argv)
{
	int opt;
	int threads = 1;
	int size = 0;

	while ((opt = getopt(argc, argv, "+t:l:")) != -1) {
		switch (opt) {
		case 't':
			threads = strtol(optarg, NULL, 0);
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
	roots = (complex double *)malloc(sizeof(complex double) * size);
	init_roots();

	double step_size = 4. / size;

	for (size_t i; i < size; ++i) {
		// WRONG
		attractors[i] = (uint8_t *)malloc(sizeof(uint8_t) * size);
	}

	// for (double a = -2.; a <= 2.; a += step_size) {
	// 	for (double b = -2.; b <= 2.; b += step_size) {
	// 		complex double x_n = iterate(CMPLX(a, b));
	// 	}
	// }
	free(roots);
}