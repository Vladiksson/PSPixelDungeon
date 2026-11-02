#pragma once
#include "item.h"
#include "char.h"
#include "ballistica.h"

int wand_hits_chars(ItemType type);

void wand_consume_charge(ItemStack *stack);

void wand_zap(const ItemStack *stack, Char *caster, Level *lvl, const Ballistica *path, int cell);

void wand_bolt_set_flying(ItemType type);
void wand_bolt_clear(void);
int wand_bolt(void);

void wand_bolt_update_trail(Level *lvl, float dt);

#define LIGHTNING_FX_MAX_POINTS 21

typedef struct {
    int     active;
    float   alpha;  /* life/DURATION, 1..0 */
    int     count;  /* number of points; draws count-1 zigzag segments */

    float pts_x[LIGHTNING_FX_MAX_POINTS];
    float pts_y[LIGHTNING_FX_MAX_POINTS];

    float mid_x[LIGHTNING_FX_MAX_POINTS - 1];
    float mid_y[LIGHTNING_FX_MAX_POINTS - 1];
} LightningFxView;

typedef struct {
    int   active;
    float alpha; /* life/DURATION, 1..0 -- DeathRay fades alpha AND thickness together */
    float sx, sy, ex, ey; /* world position, TILE_SIZE_MIN-scaled pixel units */
} RayFxView;

void wand_fx_update(float dt);
int lightning_fx_snapshot(LightningFxView *out);
int ray_fx_snapshot(RayFxView *out);