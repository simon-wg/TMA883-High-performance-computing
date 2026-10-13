#include <assert.h>
#include <complex.h>
#include <getopt.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>

#define MAX_ITERATIONS 50
#define N_ROOTS 12

uint8_t **attractors;
uint8_t **convergences;

char *colors[] = {
	"230 25 75 ", // 0  red
	"60 180 75 ", // 1  green
	"0 130 200 ", // 2  blue
	"255 225 25 ", // 3  yellow
	"245 130 48 ", // 4  orange
	"145 30 180 ", // 5  purple
	"70 240 240 ", // 6  cyan
	"240 50 230 ", // 7  magenta
	"170 110 40 ", // 8  brown
	"0 0 0 ", // 9  black      (stopped near origin)
	"128 128 128 ", // 10 gray       (too large)
	"255 255 255 " // 11 white      (50 steps)
};

int d = 0;
int size = 0;
complex double roots[N_ROOTS];

struct result {
	uint8_t attractor;
	uint8_t steps;
};

//x_{k+1} = x_k - f(x_k) / f'(x_k)

// x_n = x/d * (d - 1 + (1/x^d))

static inline complex double newton_step(complex double x)
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

ssize_t dist_to_roots_too_small(complex double x)
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

void init_roots()
{
	for (int i = 0; i < d; i++) {
		roots[i] =
			CMPLX(cos((2 * M_PI * i) / d), sin((2 * M_PI * i) / d));
	}
}

void find_root(complex double x_0, struct result *res)
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

void generate_attractors_ppm()
{
	FILE *f = fopen("newton_attractors.ppm", "w");

	char header[64];
	int len = sprintf(header, "P3\n%d %d\n255\n", size, size);
	fwrite(header, 1, len, f);

	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			char *color = colors[attractors[i][j]];
			fwrite(color, 1, strlen(color), f);
		}
	}

	fclose(f);
}

void generate_convergence_ppm()
{
	FILE *f = fopen("newton_convergences.ppm", "w");

	char header[64];
	int len =
		sprintf(header, "P2\n%d %d\n%d\n", size, size, MAX_ITERATIONS);
	fwrite(header, 1, len, f);

	char color[8];

	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			int len = sprintf(color, "%d\n", convergences[i][j]);
			fwrite(color, 1, len, f);
		}
	}

	fclose(f);
}

int main(int argc, char **argv)
{
	int opt;
	int threads = 1;

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
	init_roots();

	attractors = malloc(sizeof *attractors * size);
	uint8_t *attractors_entries =
		malloc(sizeof *attractors_entries * size * size);
	for (size_t i = 0; i < size; ++i) {
		attractors[i] = &attractors_entries[i * size];
	}
	convergences = malloc(sizeof *convergences * size);
	uint8_t *convergences_entries =
		malloc(sizeof *convergences_entries * size * size);
	for (size_t i = 0; i < size; ++i) {
		convergences[i] = &convergences_entries[i * size];
	}

	struct result res = {};
	for (size_t row = 0; row < size; row++) {
		double im = 2.0 - 4.0 * row / (size - 1);
		for (size_t col = 0; col < size; col++) {
			double re = -2.0 + 4.0 * col / (size - 1);
			find_root(CMPLX(re, im), &res);
			attractors[row][col] = res.attractor;
			convergences[row][col] = res.steps;
		}
	}

	generate_attractors_ppm();
	generate_convergence_ppm();

	free(convergences_entries);
	free(convergences);
	free(attractors_entries);
	free(attractors);
}
