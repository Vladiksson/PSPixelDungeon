#include "level_boss.h"
#include "level_paint.h"
#include "level_decorate.h"
#include "item_spawn.h"
#include <time.h>
#include <math.h>
/* ══════════════════════════════════════════════════════════
   BOSS LEVEL GENERATION
   Ports build() + decorate() from SewerBossLevel.java, PrisonBossLevel.java, CavesBossLevel.java,
   CityBossLevel.java, HallsBossLevel.java.

   Deliberately NOT ported:
     - createItems()'s Bones.get() call on all five - there's no
       "previous character's remains" system in this port yet.
     - The press()/drop() overrides that seal the arena door (or,
       for Sewers/Halls, hide the entrance) when the hero walks
       in, and re-open it once the boss's dropped key lands on
       the ground. That's entirely combat/mob-state driven and
       there's no mob or combat system yet to wire it to. Once
       one exists: on player entry into the arena, seal the
       relevant tile(s); on the boss's key item touching the
       ground, restore them. The geometry (arena door tile, boss
       room bounds) is all computed below, just not stored
       anywhere for a future caller - add fields to Level if/when
       that gets wired up.
     - Sewer boss's placeTraps()/nTraps(): inherited from
       RegularLevel in the original, which wasn't provided, so
       there's nothing authoritative to port - skipped entirely.
       Prison boss DOES override placeTraps() (ported below), but
       nTraps() itself is the same inherited/unavailable method,
       so its trap count is a placeholder - see the comment at
       its call site.
   ══════════════════════════════════════════════════════════ */


/* Random PASSABLE point strictly inside room r, excluding
   `exclude_idx` (pass -1 to not exclude anything). Mirrors
   Room.random() plus the passable-retry loops the boss levels
   use for key/sign placement. Capped at 100 tries - the Java
   versions loop forever, but a persistently-full room shouldn't
   ever hang the PSP. */
static int room_random_passable_point(const Level *lvl, const Room *r, int exclude_idx) {
    for (int tries = 0; tries < 100; tries++) {
        int x = rng_range(r->left + 1, r->right - 1);
        int y = rng_range(r->top + 1, r->bottom - 1);
        int idx = xy_to_idx(x, y);
        if (idx == exclude_idx) continue;
        if (gTileFlags[lvl->map[idx]] & TF_PASSABLE) return idx;
    }
    return -1;
}

/* ══════════════════════════════════════════════════════════
   SEWERS  —  SewerBossLevel.java
   ══════════════════════════════════════════════════════════ */
#define SEWERS_BOSS_MAX_ATTEMPTS 50

/* One attempt at a full build+decorate. Returns 1 on success, 0
   if this attempt should be thrown away and retried from scratch
   (mirrors build() returning false in the original). */
static int try_sewers_boss(Level *lvl, int depth) {
    memset(lvl, 0, sizeof(*lvl));
    lvl->theme = THEME_SEWERS;
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

    /* entrance >= 4x4; exit >= 6x6, not touching the top edge;
       at least sqrt(num_rooms) hops apart */
    int min_dist = (int)sqrt((double)lvl->num_rooms);
    int entrance_r = -1, exit_r = -1, outer = 0, distance;
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
        distance = lvl->rooms[entrance_r].distance;

        if (++outer > 10) return 0;
    } while (distance < min_dist);

    lvl->rooms[entrance_r].type = ROOM_ENTRANCE;
    lvl->rooms[exit_r].type = ROOM_BOSS_EXIT;

    /* Single path entrance->exit, re-routed once for variety (the
       same "raise price along the first path, recompute" trick
       level_generate uses before building its two paths). Unlike
       a regular level, only this one final path gets connected. */
    int path[MAX_ROOMS], plen;
    build_distance_map(lvl, exit_r);
    plen = build_path(lvl, entrance_r, exit_r, path);
    set_price(lvl, path, plen, lvl->rooms[entrance_r].distance);
    build_distance_map(lvl, exit_r);
    plen = build_path(lvl, entrance_r, exit_r, path);

    for (int i = 0; i < plen - 1; i++)
        room_connect(lvl, path[i], path[i + 1]);

    /* Reject if the boss room is entered from directly above -
       that edge is reserved for the waterfall decoration. */
    {
        int neighbour = lvl->rooms[exit_r].connected[0];
        if (lvl->rooms[exit_r].top == lvl->rooms[neighbour].bottom)
            return 0;
    }

    for (int i = 0; i < lvl->num_rooms; i++) {
        Room *r = &lvl->rooms[i];
        if (r->type == ROOM_NULL && r->n_connected > 0)
            r->type = ROOM_TUNNEL;
    }

    /* Rat King side room: an unconnected neighbour of the boss
       room touching its left, right, or bottom edge (never top -
       reserved, same as the reject check above). */
    {
        Room *rex = &lvl->rooms[exit_r];
        int candidates[MAX_NEIGHBOURS], n_cand = 0;
        for (int j = 0; j < rex->n_neighbours; j++) {
            int ni = rex->neighbours[j];
            Room *n = &lvl->rooms[ni];
            if (already_connected(lvl, exit_r, ni)) continue;
            if (rex->left == n->right || rex->right == n->left || rex->bottom == n->top)
                candidates[n_cand++] = ni;
        }
        if (n_cand > 0) {
            int king = candidates[rng_int(n_cand)];
            room_connect(lvl, king, exit_r);
            lvl->rooms[king].type = ROOM_RAT_KING;
        }
    }

    compute_doors(lvl);
    paint_rooms(lvl, depth);
    paint_tunnels(lvl);
    assign_door_types(lvl);
    paint_doors(lvl);

    {
        static uint8_t wp[LEVEL_SIZE], gp[LEVEL_SIZE];
        patch_generate(0.50f, 5, wp); paint_water(lvl, wp);
        patch_generate(0.40f, 4, gp); paint_grass(lvl, gp);
    }

    /* placeTraps() not ported here - see file header comment. */

    decorate_sewers_boss(lvl, exit_r);

    /* createItems() only drops Bones here - nothing to spawn. */

    return 1;
}

/* ══════════════════════════════════════════════════════════
   PRISON  —  PrisonBossLevel.java
   ══════════════════════════════════════════════════════════ */
#define PRISON_BOSS_MAX_ATTEMPTS 50

static int try_prison_boss(Level *lvl, int depth) {
    memset(lvl, 0, sizeof(*lvl));
    lvl->theme = THEME_PRISON;
    memset(lvl->map, TILE_WALL, sizeof(lvl->map));
    lvl->feeling = FEELING_NONE;

    int room_retry = 0;
    do {
        lvl->num_rooms = 0;
        bsp_split(lvl, 0, 0, LEVEL_W - 1, LEVEL_H - 1);
        if (++room_retry > 10) return 0;
    } while (lvl->num_rooms < 8);
    build_neighbours(lvl);

    /* entrance >= 4x4; exit >= 7x7, not touching the top edge;
       path of at least 3 rooms (>=1 room between entrance/exit) */
    int entrance_r = -1, exit_r = -1, outer = 0, plen;
    int path[MAX_ROOMS];
    do {
        if (++outer > 10) return 0;

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
                 room_w(&lvl->rooms[exit_r]) < 7 || room_h(&lvl->rooms[exit_r]) < 7 ||
                 lvl->rooms[exit_r].top == 0);

        build_distance_map(lvl, exit_r);
        plen = build_path(lvl, entrance_r, exit_r, path);
    } while (plen < 3);

    lvl->rooms[entrance_r].type = ROOM_ENTRANCE;
    lvl->rooms[exit_r].type = ROOM_BOSS_EXIT;

    /* Same single-path-rerouted-once approach as Sewers. */
    build_distance_map(lvl, exit_r);
    plen = build_path(lvl, entrance_r, exit_r, path);
    set_price(lvl, path, plen, lvl->rooms[entrance_r].distance);
    build_distance_map(lvl, exit_r);
    plen = build_path(lvl, entrance_r, exit_r, path);

    /* anteroom: the room right before the boss room - holds the
       Iron Key. Marked STANDARD *before* the PASSAGE sweep below
       so that sweep skips over it (matches the Java ordering). */
    int anteroom_idx = path[plen - 2];
    lvl->rooms[anteroom_idx].type = ROOM_STANDARD;

    for (int i = 0; i < plen - 1; i++)
        room_connect(lvl, path[i], path[i + 1]);

    for (int i = 0; i < lvl->num_rooms; i++) {
        Room *r = &lvl->rooms[i];
        if (r->type == ROOM_NULL && r->n_connected > 0)
            r->type = ROOM_PASSAGE;
    }

    compute_doors(lvl);
    paint_rooms(lvl, depth);
    paint_tunnels(lvl);
    assign_door_types(lvl);
    paint_doors(lvl);

    /* Reject if the boss room's one door sits on its top wall.
       (Prison checks this *after* painting, unlike Sewers - the
       original really does check at different points for the two
       levels; ported faithfully rather than unified.) */
    {
        int door_y = lvl->rooms[exit_r].door_y[0];
        if (door_y == lvl->rooms[exit_r].top)
            return 0;
    }

    {
        static uint8_t wp[LEVEL_SIZE], gp[LEVEL_SIZE];
        patch_generate(0.45f, 5, wp); paint_water(lvl, wp);
        patch_generate(0.30f, 4, gp); paint_grass(lvl, gp);
    }

    /* placeTraps() IS overridden in the source (scatter POISON_TRAP
       -> TILE_TOXIC_TRAP on EMPTY tiles), but nTraps() - the trap
       count - is the same inherited/unavailable method noted in the
       file header. Placeholder formula below; easy to retune once
       you have a real number in mind. */
    {
        int n_traps = 3 + depth / 10;
        for (int i = 0; i < n_traps; i++) {
            int idx = rng_int(LEVEL_SIZE);
            if (lvl->map[idx] == TILE_EMPTY)
                lvl->map[idx] = TILE_TOXIC_TRAP;
        }
    }

    decorate_prison_boss(lvl, exit_r);

    /* Iron Key in a chest, somewhere passable in the anteroom -
       the one createItems() piece that's actually generation-time
       (Bones aside, skipped as everywhere else). */
    {
        int idx = room_random_passable_point(lvl, &lvl->rooms[anteroom_idx], -1);
        if (idx >= 0)
            spawn_item_heap(lvl, idx, depth, ITEM_KEY_IRON, HEAP_CHEST);
    }

    return 1;
}

/* ══════════════════════════════════════════════════════════
   CAVES  —  CavesBossLevel.java
   Hand-carved, no room graph - always succeeds in one pass.
   ══════════════════════════════════════════════════════════ */
static void build_caves_boss(Level *lvl, int depth) {
    (void)depth;
    memset(lvl, 0, sizeof(*lvl));
    lvl->theme = THEME_CAVES;
    memset(lvl->map, TILE_WALL, sizeof(lvl->map));
    lvl->feeling = FEELING_NONE;

    int top_most = LEVEL_H + 1;     /* sentinel, larger than any real row */
    int exit_x = -1, exit_y = -1;

    for (int i = 0; i < 8; i++) {
        int left, right, top, bottom;

        if (rng_int(2) == 0) {
            left = rng_range(1, CAVES_ARENA_LEFT - 3);
            right = CAVES_ARENA_RIGHT + 3;
        } else {
            left = CAVES_ARENA_LEFT - 3;
            right = rng_range(CAVES_ARENA_RIGHT + 3, LEVEL_W - 1);
        }

        if (rng_int(2) == 0) {
            top = rng_range(2, CAVES_ARENA_TOP - 3);
            bottom = CAVES_ARENA_BOTTOM + 3;
        } else {
            /* Original literally reads ROOM_LEFT-3 here, not
               ROOM_TOP-3 (likely a copy/paste artefact) - written
               here as CAVES_ARENA_TOP-3 for clarity. Numerically
               identical today since LEVEL_W==LEVEL_H, so behaviour
               is unchanged; flagging in case that ever stops
               being true. */
            top = CAVES_ARENA_TOP - 3;
            bottom = rng_range(CAVES_ARENA_TOP + 3, LEVEL_H - 1);
        }

        fill_rect(lvl, left, top, right - left + 1, bottom - top + 1, TILE_EMPTY);

        if (top < top_most) {
            top_most = top;
            exit_x = rng_range(left, right);
            exit_y = top - 1;
        }
    }

    lvl->exit = xy_to_idx(exit_x, exit_y);
    set_tile(lvl, exit_x, exit_y, TILE_LOCKED_EXIT);

    for (int i = 0; i < LEVEL_SIZE; i++)
        if (lvl->map[i] == TILE_EMPTY && rng_int(6) == 0)
            lvl->map[i] = TILE_INACTIVE_TRAP;

    fill_rect(lvl, CAVES_ARENA_LEFT - 1, CAVES_ARENA_TOP - 1,
              CAVES_ARENA_RIGHT - CAVES_ARENA_LEFT + 3, CAVES_ARENA_BOTTOM - CAVES_ARENA_TOP + 3,
              TILE_WALL);
    fill_rect(lvl, CAVES_ARENA_LEFT, CAVES_ARENA_TOP + 1,
              CAVES_ARENA_RIGHT - CAVES_ARENA_LEFT + 1, CAVES_ARENA_BOTTOM - CAVES_ARENA_TOP,
              TILE_EMPTY);
    fill_rect(lvl, CAVES_ARENA_LEFT, CAVES_ARENA_TOP,
              CAVES_ARENA_RIGHT - CAVES_ARENA_LEFT + 1, 1,
              TILE_TOXIC_TRAP);   /* original: Terrain.TOXIC_TRAP, same tile */

    {
        int door_x = rng_range(CAVES_ARENA_LEFT, CAVES_ARENA_RIGHT);
        int door_y = CAVES_ARENA_BOTTOM + 1;
        set_tile(lvl, door_x, door_y, TILE_DOOR);
    }

    {
        int ex = rng_range(CAVES_ARENA_LEFT + 1, CAVES_ARENA_RIGHT - 1);
        int ey = rng_range(CAVES_ARENA_TOP + 1, CAVES_ARENA_BOTTOM - 1);
        lvl->entrance = xy_to_idx(ex, ey);
        set_tile(lvl, ex, ey, TILE_ENTRANCE);
    }

    {
        static uint8_t wp[LEVEL_SIZE];
        patch_generate(0.45f, 6, wp);
        paint_water(lvl, wp);
    }

    decorate_caves_boss(lvl);
}

/* ══════════════════════════════════════════════════════════
   CITY  —  CityBossLevel.java
   Hand-carved, no room graph - always succeeds in one pass.
   ══════════════════════════════════════════════════════════ */
static void build_city_boss(Level *lvl, int depth) {
    (void)depth;
    memset(lvl, 0, sizeof(*lvl));
    lvl->theme = THEME_CITY;
    memset(lvl->map, TILE_WALL, sizeof(lvl->map));
    lvl->feeling = FEELING_NONE;

    const int top = 2, hall_w = 7, hall_h = 15, chamber_h = 3;
    const int left = (LEVEL_W - hall_w) / 2;
    const int center = left + hall_w / 2;

    fill_rect(lvl, left, top, hall_w, hall_h, TILE_EMPTY);
    fill_rect(lvl, center, top, 1, hall_h, TILE_EMPTY_SP);

    for (int y = top + 1; y < top + hall_h; y += 2) {
        set_tile(lvl, center - 2, y, TILE_STATUE_SP);
        set_tile(lvl, center + 2, y, TILE_STATUE_SP);
    }

    int pedestal_row = top + hall_h / 2;
    int ped_left = center - 2, ped_right = center + 2;
    set_tile(lvl, ped_left,  pedestal_row, TILE_PEDESTAL);
    set_tile(lvl, ped_right, pedestal_row, TILE_PEDESTAL);
    for (int x = ped_left + 1; x < ped_right; x++)
        set_tile(lvl, x, pedestal_row, TILE_EMPTY_SP);

    int exit_x = center, exit_y = top - 1;
    lvl->exit = xy_to_idx(exit_x, exit_y);
    set_tile(lvl, exit_x, exit_y, TILE_LOCKED_EXIT);

    int door_x = center, door_y = top + hall_h;
    set_tile(lvl, door_x, door_y, TILE_DOOR);
    int arena_door_idx = xy_to_idx(door_x, door_y);

    fill_rect(lvl, left, top + hall_h + 1, hall_w, chamber_h, TILE_EMPTY);
    fill_rect(lvl, left, top + hall_h + 1, 1, chamber_h, TILE_BOOKSHELF);
    fill_rect(lvl, left + hall_w - 1, top + hall_h + 1, 1, chamber_h, TILE_BOOKSHELF);

    {
        int ex = left + rng_int(hall_w - 2);
        int ey = top + hall_h + 2 + rng_int(chamber_h - 1);
        lvl->entrance = xy_to_idx(ex, ey);
        set_tile(lvl, ex, ey, TILE_ENTRANCE);
    }

    decorate_city_boss(lvl, arena_door_idx);
}

/* ══════════════════════════════════════════════════════════
   HALLS  —  HallsBossLevel.java
   Hand-carved, no room graph - always succeeds in one pass.
   ══════════════════════════════════════════════════════════ */
static void build_halls_boss(Level *lvl, int depth) {
    (void)depth;
    memset(lvl, 0, sizeof(*lvl));
    lvl->theme = THEME_HALLS;
    memset(lvl->map, TILE_WALL, sizeof(lvl->map));
    lvl->feeling = FEELING_NONE;

    const int room_left = LEVEL_W / 2 - 1, room_right = LEVEL_W / 2 + 1;
    const int room_top  = LEVEL_H / 2 - 1, room_bottom = LEVEL_H / 2 + 1;

    int exit_x = -1, exit_y = -1;

    for (int i = 0; i < 5; i++) {
        int top = rng_range(2, room_top - 1);
        int bottom = rng_range(room_bottom + 1, 22);

        fill_rect(lvl, 2 + i * 4, top, 4, bottom - top + 1, TILE_EMPTY);

        if (i == 2) {
            /* Original uses i*4+3, not (2+i*4)+3 - deliberately
               ported as-is (same asymmetry as the deco loop just
               below). Numerically that's (2+i*4)+1, still inside
               the 4-wide corridor, just its 2nd column rather than
               centered on it. */
            exit_x = i * 4 + 3;
            exit_y = top - 1;
        }

        for (int j = 0; j < 4; j++) {
            if (rng_int(2) == 0) {
                int y = rng_range(top + 1, bottom - 1);
                /* Same as above: original indexes i*4+j, not
                   (2+i*4)+j, so for j=0,1 this can land one or two
                   tiles into the wall left of the corridor rather
                   than inside it. Ported as-is. */
                set_tile(lvl, i * 4 + j, y, TILE_WALL_DECO);
            }
        }
    }

    lvl->exit = xy_to_idx(exit_x, exit_y);
    set_tile(lvl, exit_x, exit_y, TILE_LOCKED_EXIT);

    fill_rect(lvl, room_left - 1, room_top - 1,
              room_right - room_left + 3, room_bottom - room_top + 3, TILE_WALL);
    fill_rect(lvl, room_left, room_top,
              room_right - room_left + 1, room_bottom - room_top + 1, TILE_EMPTY);

    {
        int ex = rng_range(room_left + 1, room_right - 1);
        int ey = rng_range(room_top + 1, room_bottom - 1);
        lvl->entrance = xy_to_idx(ex, ey);
        set_tile(lvl, ex, ey, TILE_ENTRANCE);
    }

    {
        static uint8_t wp[LEVEL_SIZE];
        patch_generate(0.45f, 6, wp);
        paint_water(lvl, wp);
    }

    decorate_halls_boss(lvl);
}

/* ══════════════════════════════════════════════════════════
   PUBLIC ENTRY POINT
   ══════════════════════════════════════════════════════════ */
int level_generate_boss(Level *lvl, uint32_t seed, DungeonTheme theme, int depth) {
    rng_state = seed ? seed : (uint32_t)time(NULL);

    switch (theme) {
    case THEME_SEWERS:
        for (int i = 0; i < SEWERS_BOSS_MAX_ATTEMPTS; i++)
            if (try_sewers_boss(lvl, depth)) return 0;
        return -1;

    case THEME_PRISON:
        for (int i = 0; i < PRISON_BOSS_MAX_ATTEMPTS; i++)
            if (try_prison_boss(lvl, depth)) return 0;
        return -1;

    case THEME_CAVES:
        build_caves_boss(lvl, depth);
        return 0;

    case THEME_CITY:
        build_city_boss(lvl, depth);
        return 0;

    case THEME_HALLS:
        build_halls_boss(lvl, depth);
        return 0;

    default:
        return -1;
    }
}