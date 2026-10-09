#include "ballistica.h"
#include "level_geometry.h"
#include "char.h"
#include "common.h"
#include "item.h"
#include <math.h>
#include <stdlib.h>

/* ══════════════════════════════════════════════════════════
   BALLISTICA
   ══════════════════════════════════════════════════════════*/
int ballistica_cast(Ballistica *out, const Level *lvl, int from, int to, int magic, int hit_chars) {
    Ballistica scratch;
    Ballistica *b = out ? out : &scratch;

    b->distance = 0;
    if (!in_bounds(from)) {
        return from;
    }

    Point f = idx_to_xy(from);
    Point t = idx_to_xy(to);

    int dx = t.x - f.x;
    int dy = t.y - f.y;

    int stepX = dx > 0 ? +1 : -1;
    int stepY = dy > 0 ? +1 : -1;

    dx = abs(dx);
    dy = abs(dy);

    int stepA, stepB, dA, dB;

    if (dx > dy) {
        stepA = stepX;
        stepB = stepY * LEVEL_W;
        dA = dx;
        dB = dy;
    } else {
        stepA = stepY * LEVEL_W;
        stepB = stepX;
        dA = dy;
        dB = dx;
    }

    b->distance = 1;
    b->trace[0] = from;

    int cell = from;
    int err = dA / 2;

    while (cell != to || magic) {
        cell += stepA;

        err += dB;
        if (err >= dA) {
            err = err - dA;
            cell = cell + stepB;
        }

        if (!in_bounds(cell) || b->distance >= BALLISTICA_MAX_TRACE) {
            return b->trace[b->distance - 1];
        }
        b->trace[b->distance++] = cell;

        if (!cell_passable(lvl, cell) && !cell_avoid(lvl, cell)) {
            return b->trace[--b->distance - 1];     /* stops BEFORE the wall */
        }

        if (blocks_light(lvl, cell) || (hit_chars && char_find_at(cell) != NULL)) {
            return cell;                            /* stops ON the blocker  */
        }
    }

    if (b->distance < BALLISTICA_MAX_TRACE) {
        b->trace[b->distance++] = cell;             /* sic: stored twice in the original */
    }
    return to;
}

/* ══════════════════════════════════════════════════════════
   ITEMS THROWING... MB TRANSFER ELSEWHERE????
   ══════════════════════════════════════════════════════════*/
typedef struct {
    int       active;

    float     x0, y0, x1, y1;
    float     elapsed, duration;
    float     angle, angular_speed;

    uint8_t   col, row;

    MissileCallback on_land;
    void     *user;
} Missile;

static Missile missile;

int missile_launch(int from, int to, ItemType type, MissileSpin spin, MissileCallback on_land, void *user) {
    if (missile.active)
        return 0;

    Point a = idx_to_xy(from);
    Point b = idx_to_xy(to);

    float dx = (float)(b.x - a.x);    /* tiles; only the direction and the length matter, */
    float dy = (float)(b.y - a.y);    /* and both scale together with the 16 px tile.     */
    float len = sqrtf(dx * dx + dy * dy);

    missile.active        = 1;

    missile.x0 = (float)a.x;  missile.y0 = (float)a.y;
    missile.x1 = (float)b.x;  missile.y1 = (float)b.y;

    missile.elapsed  = 0.0f;
    missile.duration = len * MISSILE_TILE_PX / MISSILE_SPEED_PX;

    switch (spin) {
    case MISSILE_NO_SPIN:
        missile.angular_speed = 0.0f;
        missile.angle = 135.0f - atan2f(dx, dy) / M_PI * 180.0f;
        break;
    case MISSILE_SPIN_FAST:
        missile.angular_speed = 1440.0f;
        missile.angle = 0.0f;
        break;
    default:
        missile.angular_speed = 720.0f;
        missile.angle = 0.0f;
        break;
    }

    item_display_sprite(type, &missile.col, &missile.row);

    missile.on_land = on_land;
    missile.user    = user;
    return 1;
}

void missiles_update(float dt) {
    if (!missile.active) 
        return;

    missile.elapsed += dt;
    missile.angle = fmodf(missile.angle + missile.angular_speed * dt, 360.0f);

    if (missile.elapsed >= missile.duration) {
        MissileCallback cb = missile.on_land;
        void *user = missile.user;

        missile.active = 0;                        /* kill() first ...                 */
        if (cb) cb(user);                     /* ... then callback.call()         */
    }
}

int missile_is_flying(void) {
    return missile.active;
}

int missiles_snapshot(MissileView *out) {
    if (!missile.active || !out) return 0;

    float t = missile.duration > 0.0f
        ? missile.elapsed / missile.duration
        : 1.0f;

    if (t > 1.0f) t = 1.0f;

    out[0].x = missile.x0 + (missile.x1 - missile.x0) * t;
    out[0].y = missile.y0 + (missile.y1 - missile.y0) * t;
    out[0].angle = missile.angle;
    out[0].col = missile.col;
    out[0].row = missile.row;

    return 1;
}