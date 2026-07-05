#pragma once
#include "common.h"
#include "level.h"

/*
 * Fog of War — port of FogOfWar.java
 *
 * Three visibility states per tile (matching FogOfWar.java constants):
 *   VISIBLE   (currently in line of sight)   → render at full brightness
 *   VISITED   (seen before, now out of LOS)  → dark overlay 0xCC111111
 *   INVISIBLE (never seen)                   → solid black  0xFF000000
 *
 * visited[] is persistent (never cleared once set).
 * visible[] is recomputed every time the player moves.
 */

#define FOV_RADIUS 8   /* view distance in tiles — matches PD's default */

typedef struct {
    uint8_t visible[LEVEL_SIZE];   /* 1 = currently in LOS              */
    uint8_t visited[LEVEL_SIZE];   /* 1 = has been seen (accumulated)   */
} Fog;

/* Zero all state — call once at level load */
void fog_init  (Fog *fog);

/* Recompute visible[] from (px,py), then OR result into visited[].
   Call every time the player moves.                                */
void fog_update(Fog *fog, const Level *lvl, int px, int py);