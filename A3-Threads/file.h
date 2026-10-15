#ifndef FILE_H
#define FILE_H

#include "newton.h"

#include <stdbool.h>
#include <stdio.h>

extern uint8_t **attractors;
extern uint8_t **convergence;
extern bool *ready;
extern char attractors_name[32];
extern char convergence_name[32];
extern char shades[MAX_ITERATIONS + 1][10];
extern size_t size;

void write_attractors_header(FILE *f);
void write_attractors_to_ppm(FILE *f, int row, char *buffer);
void write_convergence_header(FILE *f);
void write_convergence_to_ppm(FILE *f, int row, char *buffer);
int write(void *_);

#endif
