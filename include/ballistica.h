#pragma once
#include "level.h"

#define BALLISTICA_MAX_TRACE  ((LEVEL_W > LEVEL_H ? LEVEL_W : LEVEL_H) + 2)

typedef struct {
  int trace[BALLISTICA_MAX_TRACE];
  int distance;
} Ballistica;

int ballistica_cast(Ballistica *out, const Level *lvl, int from, int to, int magic, int hit_chars);



#define MISSILE_SPEED_PX  240.0f   /* MissileSprite.SPEED                     */
#define MISSILE_TILE_PX   16.0f    /* DungeonTilemap.SIZE                     */

typedef void (*MissileCallback)(void *user);

typedef struct {
  float   x, y;
  float   angle;
  uint8_t col, row;
} MissileView;

int missile_launch(int from, int to, ItemType type, MissileSpin spin, MissileCallback on_land, void *user);

void missiles_update(float dt);

int missile_is_flying(void);

int missiles_snapshot(MissileView *out);
