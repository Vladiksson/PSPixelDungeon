#pragma once
#include "level_utils.h"

int entrance_has_large_neighbour(Level *lvl, int entrance_idx);
void place_special_near_entrance(Level *lvl, int entrance_idx, int depth, int blacksmith_depth);
void init_special_rooms(LevelGenState *gen, int depth);

void assign_types(Level *lvl, LevelGenState *gen, int entrance_idx, int exit_idx, DungeonTheme theme, int depth);

void compute_doors(Level *lvl);
void assign_door_types(Level *lvl);