#include "rng.h"

uint32_t rng_state;
uint32_t rng_next(void) {
    rng_state = rng_state * 1664525u + 1013904223u;
    return rng_state;
}
int rng_range(int min, int max) {
    if (min >= max) return min;
    return min + (int)((rng_next() >> 16) % (unsigned)(max - min + 1));
}
float rng_float(void) {
    return (float)(rng_next() >> 8) / (float)(1 << 24);
}
int rng_int(int max) {
    if (max <= 1) return 0;
    return (int)((rng_next() >> 16) % (unsigned)max);
}