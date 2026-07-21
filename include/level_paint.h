#pragma once
#include "level_utils.h"

void paint_rooms(Level *lvl, int depth);
void paint_tunnels(Level *lvl);
void paint_doors(Level *lvl);

void patch_generate(float seed, int n_gen, uint8_t *out);
void paint_water(Level *lvl, const uint8_t *lake);
void paint_grass(Level *lvl, const uint8_t *grass);
void paint_chasms(Level *lvl);