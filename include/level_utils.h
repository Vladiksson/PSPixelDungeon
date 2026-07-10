#pragma once
#include "level.h"

extern uint32_t rng_state;
uint32_t rng_next(void);
int rng_range(int min, int max);
float rng_float(void);
int rng_int(int max);
int isqrt(int n);

void set_tile(Level *lvl, int x, int y, uint8_t t); 
void fill_rect(Level *lvl, int x, int y, int w, int h, uint8_t t);

int already_connected(Level *lvl, int a, int b);
void room_connect(Level *lvl, int a, int b);