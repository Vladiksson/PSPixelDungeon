#pragma once
#include <stdint.h>

extern uint32_t rng_state;
uint32_t rng_next(void);
int rng_range(int min, int max);
float rng_float(void);
int rng_int(int max);