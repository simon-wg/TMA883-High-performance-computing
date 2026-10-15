#ifndef NEWTON_H
#define NEWTON_H

#include <stdint.h>

#define MAX_ITERATIONS 50
#define N_ROOTS 12

static const char colors[][13] = {
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

struct result {
	uint8_t attractor;
	uint8_t steps;
};

#endif
