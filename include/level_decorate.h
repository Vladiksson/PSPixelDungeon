#pragma once 
#include "level_utils.h"

void decorate_sewers(Level *lvl);
void decorate_prison(Level *lvl);
void decorate_caves(Level *lvl);
void decorate_city(Level *lvl);
void decorate_halls(Level *lvl);
void decorate_generic(Level *lvl);

/* Caves arena box */
#define CAVES_ARENA_LEFT   (LEVEL_W / 2 - 2)
#define CAVES_ARENA_RIGHT  (LEVEL_W / 2 + 2)
#define CAVES_ARENA_TOP    (LEVEL_H / 2 - 2)
#define CAVES_ARENA_BOTTOM (LEVEL_H / 2 + 2)

void decorate_sewers_boss(Level *lvl, int exit_room_idx);
void decorate_prison_boss(Level *lvl, int exit_room_idx);
void decorate_caves_boss(Level *lvl);
void decorate_city_boss(Level *lvl, int arena_door_idx);
void decorate_halls_boss(Level *lvl);

void decorate_last_shop(Level *lvl, int quest_completed);
void decorate_last(Level *lvl);
