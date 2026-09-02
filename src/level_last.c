#include "level_last.h"
#include "level_paint.h"
#include "level_decorate.h"
#include "item_spawn.h"
#include <time.h>
#include <math.h>
/* ══════════════════════════════════════════════════════════
   LAST SHOP  —  LastShopLevel.java
   ══════════════════════════════════════════════════════════ */
#define LAST_SHOP_MAX_ATTEMPTS 50

static int try_last_shop(Level *lvl, int depth, int quest_completed) {
    memset(lvl, 0, sizeof(*lvl));
    lvl->theme = THEME_CITY;   /* tilesTex()/waterTex() are both CITY in the original */
    memset(lvl->map, TILE_WALL, sizeof(lvl->map));
    lvl->feeling = FEELING_NONE;

    /* initRooms() */
    int room_retry = 0;
    do {
        lvl->num_rooms = 0;
        bsp_split(lvl, 0, 0, LEVEL_W - 1, LEVEL_H - 1);
        if (++room_retry > 10) return 0;
    } while (lvl->num_rooms < 8);
    build_neighbours(lvl);

    int min_dist = (int)sqrt((double)lvl->num_rooms);
    int entrance_r = -1, exit_r = -1, outer = 0, plen;
    int path[MAX_ROOMS];
    do {
        int inner = 0;
        do {
            if (++inner > 10) return 0;
            entrance_r = rng_range(0, lvl->num_rooms - 1);
        } while (room_w(&lvl->rooms[entrance_r]) < 4 || room_h(&lvl->rooms[entrance_r]) < 4);

        inner = 0;
        do {
            if (++inner > 10) return 0;
            exit_r = rng_range(0, lvl->num_rooms - 1);
        } while (exit_r == entrance_r ||
                 room_w(&lvl->rooms[exit_r]) < 6 || room_h(&lvl->rooms[exit_r]) < 6 ||
                 lvl->rooms[exit_r].top == 0);

        build_distance_map(lvl, exit_r);
        plen = build_path(lvl, entrance_r, exit_r, path);

        if (++outer > 10) return 0;
    } while (plen < min_dist);

    lvl->rooms[entrance_r].type = ROOM_ENTRANCE;
    lvl->rooms[exit_r].type = ROOM_EXIT;   
    
    build_distance_map(lvl, exit_r);
    plen = build_path(lvl, entrance_r, exit_r, path);
    set_price(lvl, path, plen, lvl->rooms[entrance_r].distance);
    build_distance_map(lvl, exit_r);
    plen = build_path(lvl, entrance_r, exit_r, path);

    for (int i = 0; i < plen - 1; i++)
        room_connect(lvl, path[i], path[i + 1]);

    /* Every other connected room becomes a PASSAGE; the *largest*
       of them (by area) is the shop candidate, provided it's big
       enough (>=30 tiles). No candidate, or too small -> the whole
       level is rejected and retried, same as the original. */
    int shop_idx = -1, shop_square = 0;
    for (int i = 0; i < lvl->num_rooms; i++) {
        Room *r = &lvl->rooms[i];
        if (r->type == ROOM_NULL && r->n_connections > 0) {
            r->type = ROOM_PASSAGE;
            int sq = room_square(r);
            if (sq > shop_square) { shop_idx = i; shop_square = sq; }
        }
    }
    if (shop_idx < 0 || shop_square < 30)
        return 0;
    lvl->rooms[shop_idx].type = quest_completed ? ROOM_SHOP : ROOM_STANDARD;

    compute_doors(lvl);
    paint_rooms(lvl, depth);
    paint_tunnels(lvl);
    assign_door_types(lvl);
    paint_doors(lvl);

    {
        static uint8_t wp[LEVEL_SIZE], gp[LEVEL_SIZE];
        patch_generate(0.35f, 4, wp); paint_water(lvl, wp);
        patch_generate(0.30f, 3, gp); paint_grass(lvl, gp);
    }

    /* No placeTraps() here - the original doesn't call it either. */

    decorate_last_shop(lvl, quest_completed);

    /* createItems() only drops Bones here - nothing to spawn. */

    return 1;
}

int level_generate_last_shop(Level *lvl, uint32_t seed, int depth, int quest_completed) {
    rng_state = seed ? seed : (uint32_t)time(NULL);
    for (int i = 0; i < LAST_SHOP_MAX_ATTEMPTS; i++)
        if (try_last_shop(lvl, depth, quest_completed)) return 0;
    return -1;
}


/* ══════════════════════════════════════════════════════════
   LAST LEVEL  —  LastLevel.java
   ══════════════════════════════════════════════════════════ */
#define LASTLEVEL_SIZE 7
 
int level_generate_last(Level *lvl, uint32_t seed, int depth) {
    rng_state = seed ? seed : (uint32_t)time(NULL);
 
    memset(lvl, 0, sizeof(*lvl));
    lvl->theme = THEME_HALLS;   /* tilesTex()/waterTex() are both HALLS in the original */
    memset(lvl->map, TILE_WALL, sizeof(lvl->map));
    lvl->feeling = FEELING_NONE;
 
    /* Original anchors this whole room at the map's top-left corner
       (1,1). Centered here instead: origin is the water block's
       top-left corner, computed so the *whole* footprint - the 7x7
       water block plus the exit tile sitting one row above it - is
       centered as a unit, not just the water block by itself. Every
       other position below is the same offset from origin that the
       original used from its fixed (1,1). */
    int origin_x = (LEVEL_W - LASTLEVEL_SIZE) / 2;
    int origin_y = (LEVEL_H - (LASTLEVEL_SIZE + 1)) / 2 + 1;
 
    fill_rect(lvl, origin_x, origin_y, LASTLEVEL_SIZE, LASTLEVEL_SIZE, TILE_WATER);
    fill_rect(lvl, origin_x + 1, origin_y + 1, LASTLEVEL_SIZE - 2, LASTLEVEL_SIZE - 2, TILE_EMPTY);
    fill_rect(lvl, origin_x + LASTLEVEL_SIZE/2 - 1, origin_y + LASTLEVEL_SIZE/2 - 1, 3, 3, TILE_EMPTY_SP);
 
    /* original: entrance at the water block's bottom-center edge */
    int ex = origin_x + LASTLEVEL_SIZE / 2, ey = origin_y + LASTLEVEL_SIZE - 1;
    lvl->entrance = xy_to_idx(ex, ey);
    set_tile(lvl, ex, ey, TILE_ENTRANCE);
 
    /* original: exit = entrance - WIDTH*SIZE -> same column, one row
       above the water block's top edge */
    int exit_x = ex, exit_y = origin_y - 1;
    lvl->exit = xy_to_idx(exit_x, exit_y);
    set_tile(lvl, exit_x, exit_y, TILE_LOCKED_EXIT);
 
    /* original: pedestal = (SIZE/2+1)*(WIDTH+1) - the exact center
       of the 7x7 block */
    int px = origin_x + LASTLEVEL_SIZE / 2, py = origin_y + LASTLEVEL_SIZE / 2;
    int pedestal_idx = xy_to_idx(px, py);
    set_tile(lvl, px, py, TILE_PEDESTAL);
    set_tile(lvl, px - 1, py, TILE_STATUE_SP);
    set_tile(lvl, px + 1, py, TILE_STATUE_SP);
 
    decorate_last(lvl);
 
    spawn_item_heap(lvl, pedestal_idx, depth, ITEM_AMULET_OF_YENDOR, HEAP_NORMAL);
 
    return 0;
}