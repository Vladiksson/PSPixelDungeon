#pragma once
#include "common.h"
#include "item.h"

typedef enum {
    FEELING_NONE,
    FEELING_CHASM,
    FEELING_WATER,
    FEELING_GRASS
} LevelFeeling;

typedef struct {
    uint8_t      map[LEVEL_SIZE];
    int          entrance;
    int          exit;
    DungeonTheme theme;
    LevelFeeling feeling;
    Room         rooms[MAX_ROOMS];
    int          num_rooms;

    ItemType     items_to_spawn[MAX_ITEMS_TO_SPAWN];
    int          num_items_to_spawn;
    Heap         heaps[MAX_LEVEL_ITEMS];
    int          num_heaps;
} Level;

typedef struct {
    RoomType specials[16];
    int num_specials;

    int pit_room_needed;//TODO genstate doesnt saves after changing levels, so needs rework
    int weak_floor_created;
} LevelGenState;

int  level_generate(Level *lvl, uint32_t seed, DungeonTheme theme, int depth, int blacksmith_depth);

/* Safe tile read — returns TILE_WALL for out-of-bounds */
static inline uint8_t get_tile_safe(const Level *lvl, int x, int y) {
    if (!in_bounds(x,y)) return TILE_WALL;
    return lvl->map[xy_to_idx(x,y)];
}

/* Room geometry helpers matching Rect.java */
static inline int room_w     (const Room *r) { return r->right  - r->left; }
static inline int room_h     (const Room *r) { return r->bottom - r->top;  }
static inline int room_square(const Room *r) { return room_w(r) * room_h(r); }
static inline int room_inside(const Room *r, int x, int y) {
    return x > r->left && y > r->top && x < r->right && y < r->bottom;
}

int level_player_on_exit(const Level *lvl, int x, int y);
int level_player_on_entrance(const Level *lvl, int x, int y);


/* ── Items — implemented in item.c ────────────────────────
   Logically "item system", but those takes a Level* so they
   live here rather than in item.h (mirrors how level_utils.c owns
   the tile helpers that also take a Level*). */
int gold_amount(int depth);

Heap *level_create_heap(Level *lvl, int x, int y, HeapType type);
Heap *level_heap_at(Level *lvl, int x, int y);
Heap *level_heap_at_idx(Level *lvl, int idx);

int heap_add_item(Heap *heap, ItemType type, int quantity);
Heap *spawn_item(Level *lvl, int idx, int depth, ItemType type);
//Heap *spawn_item_amount(Level *lvl, int idx, int depth, ItemType type, int amount);
Heap *spawn_random_item(Level *lvl, int idx, int depth);
Heap *spawn_random_item_of_category(Level *lvl, int idx, int depth, ItemCategory cat);
Heap *spawn_item_heap(Level *lvl, int idx, int depth, ItemType type, HeapType heap_type);
Heap *spawn_item_amount_heap(Level *lvl, int idx, int depth, ItemType type, int amount, HeapType heap_type);
Heap *spawn_random_item_of_category_heap(Level *lvl, int idx, int depth, ItemCategory cat, HeapType heap_type);

void level_add_item_to_spawn(Level *lvl, ItemType type);
int level_take_spawn_item(Level *lvl, ItemType *out);

void level_spawn_items (Level *lvl, int depth);
void place_room_loot(Level *lvl, Room *r, int depth, int count);
