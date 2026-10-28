#pragma once
#include "level.h"
#include "rng.h"

extern const int DX[8];
extern const int DY[8];

int cell_distance(int a, int b);
int cells_adjacent(int a, int b);

int neighbour_idx(int pos, int dx, int dy);
int random_neighbour(int pos);

int cell_passable(const Level *lvl, int idx);
int blocks_light(const Level *lvl, int idx);
int cell_flamable(const Level *lvl, int idx);
int cell_avoid(const Level *lvl, int idx);
int cell_pit(const Level *lvl, int idx);
bool tile_unstitchable(uint8_t t);
int tile_free_for_item(const Level *lvl, int idx);
int cell_solid(const Level *lvl, int idx);
int cell_liquid(const Level *lvl, int idx);
int cell_secret(const Level *lvl, int idx);

int tile_discover(const Level *lvl, int idx);