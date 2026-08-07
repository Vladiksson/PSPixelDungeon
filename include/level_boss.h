#pragma once
#include "level_rooms.h"

/* ══════════════════════════════════════════════════════════
   BOSS LEVEL GENERATION
   ══════════════════════════════════════════════════════════ */
int level_generate_boss(Level *lvl, uint32_t seed, DungeonTheme theme, int depth);

/* Caves arena box */
#define CAVES_ARENA_LEFT   (LEVEL_W / 2 - 2)
#define CAVES_ARENA_RIGHT  (LEVEL_W / 2 + 2)
#define CAVES_ARENA_TOP    (LEVEL_H / 2 - 2)
#define CAVES_ARENA_BOTTOM (LEVEL_H / 2 + 2)