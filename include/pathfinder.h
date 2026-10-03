#pragma once
#include "level_geometry.h"

/* ══════════════════════════════════════════════════════════
   port of PathFinder.java STUFF FOR MOB AI REDO
   ══════════════════════════════════════════════════════════ */
int pathfinder_get_step(const Level *lvl, int from, int to);
int pathfinder_get_step_back(const Level *lvl, int cur, int from);