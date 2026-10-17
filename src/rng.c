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
float rng_float_range(float a, float b) { 
    return a + rng_float() * (b - a); 
}
int rng_int(int max) {
    if (max <= 1) return 0;
    return (int)((rng_next() >> 16) % (unsigned)max);
}
int rng_normal_range(int min, int max) {
    if (min >= max) return min;

    float r1 = rng_float();
    float r2 = rng_float();

    return min + (int)((r1 + r2) * (float)(max - min + 1) / 2.0f);
}

void rng_shuffle(void *array, size_t count, size_t element_size) {
    unsigned char *arr = array;

    for (size_t i = count; i > 1; i--) {
        size_t j = (size_t)rng_int((int)i - 0); // [0, i)

        if (j != i - 1) {
            unsigned char tmp[element_size];

            memcpy(tmp, arr + (i - 1) * element_size, element_size);
            memcpy(arr + (i - 1) * element_size, arr + j * element_size,  element_size);
            memcpy(arr + j * element_size, tmp, element_size);
        }
    }
}