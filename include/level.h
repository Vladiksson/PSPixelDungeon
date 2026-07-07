#pragma once
#include "common.h"

/* Returns 1 if tile is walkable */
int  player_passable(uint8_t tile);

typedef struct {
    uint8_t  map[LEVEL_SIZE];
    int      entrance;
    int      exit;
    DungeonTheme theme;
    Room     rooms[MAX_ROOMS];
    int      num_rooms;
} Level;

int  level_generate(Level *lvl, uint32_t seed, DungeonTheme theme);
void level_print   (const Level *lvl);

/* Safe tile read — returns TILE_WALL for out-of-bounds */
static inline uint8_t get_tile_safe(const Level *lvl, int x, int y) {
    if (!in_bounds(x,y)) return TILE_WALL;
    return lvl->map[xy_to_idx(x,y)];
}

/* Room geometry helpers matching Rect.java */
static inline int room_w     (const Room *r) { return r->right  - r->left; }
static inline int room_h     (const Room *r) { return r->bottom - r->top;  }
static inline int room_inside(const Room *r, int x, int y) {
    return x > r->left && y > r->top && x < r->right && y < r->bottom;
}

int level_player_on_exit(const Level *lvl, int x, int y);
int level_player_on_entrance(const Level *lvl, int x, int y);