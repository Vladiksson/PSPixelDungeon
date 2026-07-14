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


/* ══════════════════════════════════════════════════════════
   Formerly `static` inside level.c. Boss levels (level_boss.c)
   need these too - Sewer/Prison boss levels are built on the
   same BSP room graph as regular levels (they extend
   RegularLevel in the original Java), so this is exposed
   rather than duplicated. Definitions still live in level.c;
   this is purely a linkage change, behaviour is untouched.
   ══════════════════════════════════════════════════════════ */
void bsp_split(Level *lvl, int left, int top, int right, int bottom);
void build_neighbours(Level *lvl);
void build_distance_map(Level *lvl, int target);
int  build_path(Level *lvl, int from, int to, int *path);
void set_price(Level *lvl, int *path, int len, int price);
void compute_doors(Level *lvl);
void assign_door_types(Level *lvl);