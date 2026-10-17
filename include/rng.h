#pragma once
#include <stdint.h>
#include <stddef.h>
#include <string.h>

extern uint32_t rng_state;
uint32_t rng_next(void);
int rng_range(int min, int max);
float rng_float(void);
float rng_float_range(float a, float b);
int rng_int(int max);
int rng_normal_range(int min, int max);

void rng_shuffle(void *array, size_t count, size_t element_size);

#define RNG_SHUFFLE(array, count)  rng_shuffle((array), (count), sizeof((array)[0]))
