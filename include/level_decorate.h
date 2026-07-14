#pragma once 
#include "level.h"
#include "level_utils.h"

void decorate_sewers(Level *lvl);
void decorate_prison(Level *lvl);
void decorate_caves(Level *lvl);
void decorate_city(Level *lvl);
void decorate_halls(Level *lvl);
void decorate_generic(Level *lvl);

/* Boss variants - separate functions rather than branches inside the
   regular ones, because each XBossLevel.decorate() in the original
   uses its own constants/probabilities, not the regular level's. */
void decorate_sewers_boss(Level *lvl, int exit_room_idx);
void decorate_prison_boss(Level *lvl, int exit_room_idx);
void decorate_caves_boss(Level *lvl);
void decorate_city_boss(Level *lvl, int arena_door_idx);
void decorate_halls_boss(Level *lvl);

/* Epilogue levels (level_last.c) - same reasoning as the boss
   variants above: each has its own constants, not a shared branch. */
void decorate_last_shop(Level *lvl, int quest_completed);
void decorate_last(Level *lvl);
