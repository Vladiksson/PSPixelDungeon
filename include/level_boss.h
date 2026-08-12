#pragma once
#include "level_rooms.h"
#include "player.h"

int level_generate_boss(Level *lvl, uint32_t seed, DungeonTheme theme, int depth);

void level_boss_event(Level *lvl, int depth, Player *player);