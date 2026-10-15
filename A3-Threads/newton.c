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
#include <time.h>

#define MAX_ITERATIONS 50
#define N_ROOTS 12

uint8_t **attractors;
uint8_t **convergence;
bool *ready;
char attractors_name[32];
char convergence_name[32];

const char colors[][13] = {
	"230 025 075 ", // 0  red
	"060 180 075 ", // 1  green
	"000 130 200 ", // 2  blue
	"255 225 025 ", // 3  yellow
	"245 130 048 ", // 4  orange
	"145 030 180 ", // 5  purple
	"070 240 240 ", // 6  cyan
	"240 050 230 ", // 7  magenta
	"170 110 040 ", // 8  brown
	"000 000 000 ", // 9  black      (stopped near origin)
	"128 128 128 ", // 10 gray       (too large)
	"255 255 255 " // 11 white      (50 steps)
};

char shades[MAX_ITERATIONS + 1][10];

int d = 0;
size_t size = 0;
int thread_count = 1;
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

int find_roots(void *arg)
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

void write_attractors_header(FILE *f)
{
	char header[64];
	int len = sprintf(header, "P3\n%zu %zu\n255\n", size, size);
	fwrite(header, 1, len, f);
}

void write_attractors_to_ppm(FILE *f, int row, char *buffer)
{
	for (size_t col = 0; col < size; col++) {
		char *color = colors[attractors[row][col]];
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
	for (int i = 0; i < size; ++i)
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
