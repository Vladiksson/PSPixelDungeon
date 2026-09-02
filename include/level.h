#pragma once
#include "common.h"
#include "item.h"
#include "mob.h"
#include "npc.h"

/* ── Room constants ──────────────────────────────────── */
#define MAX_ROOMS       64
#define MAX_NEIGHBOURS   8   /* BSP rooms have 2-4 typically     */
#define MAX_CONN_ROOM    8   /* max door-connections per room     */

typedef enum {
    ROOM_NULL       = 0,
    ROOM_STANDARD   = 1,
    ROOM_ENTRANCE   = 2,
    ROOM_EXIT       = 3,
    ROOM_BOSS_EXIT  = 4,
    ROOM_TUNNEL     = 5,
    ROOM_PASSAGE    = 6,
	ROOM_SHOP	    = 7,
	ROOM_BLACKSMITH	= 8,
	ROOM_TREASURY	= 9,
	ROOM_ARMORY		= 10,
	ROOM_LIBRARY	= 11,
	ROOM_LABORATORY	= 12,
	ROOM_VAULT		= 13,
	ROOM_TRAPS		= 14,
	ROOM_STORAGE	= 15,
	ROOM_MAGIC_WELL	= 16,
	ROOM_GARDEN		= 17,
	ROOM_CRYPT		= 18,
	ROOM_STATUE		= 19,
	ROOM_POOL		= 20,
	ROOM_RAT_KING	= 21,
	ROOM_WEAK_FLOOR	= 22,
	ROOM_PIT		= 23,
	ROOM_ALTAR		= 24,
    ROOM_COUNT      = 25,
} RoomType;

typedef enum {
    DOOR_EMPTY,
    DOOR_TUNNEL,
    DOOR_REGULAR,
    DOOR_LOCKED,
    //DOOR_SECRET,
    DOOR_HIDDEN,
    DOOR_BARRICADE,
    DOOR_UNLOCKED
} DoorType;

typedef struct {
    int room;
    int pos;
    DoorType type;
} RoomConnection;

typedef struct {
    int left, top, right, bottom;

    int neighbours   [MAX_NEIGHBOURS];
    int n_neighbours;
    int distance;             /* BFS distance (pathfinding)   */
    int price;                /* Dijkstra cost (default 1)    */

    RoomConnection connections [MAX_CONN_ROOM];
    int n_connections;

    DoorType door_type[MAX_CONN_ROOM];
    RoomType type;
} Room;

typedef enum {
    ROOMF_SPECIAL      = 1 << 0,
    ROOMF_TUNNEL       = 1 << 1,
    ROOMF_LOCKED_DOOR  = 1 << 2,
    ROOMF_HIDDEN_DOOR  = 1 << 3,
    ROOMF_EMPTY_DOOR   = 1 << 4,
    ROOMF_BARRICADE    = 1 << 5,
} RoomFlags;

extern const uint16_t gRoomFlags[ROOM_ALTAR + 1];

typedef enum {
    THEME_SEWERS = 0,
    THEME_PRISON = 1,
    THEME_CAVES  = 2,
    THEME_CITY   = 3,
    THEME_HALLS  = 4,
} DungeonTheme;

typedef enum {
    FEELING_NONE,
    FEELING_CHASM,
    FEELING_WATER,
    FEELING_GRASS
} LevelFeeling;

typedef struct Level {
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

    MobPool mob_pool;
    int boss_triggered;

    NPCPool npc_pool;
} Level;

typedef struct {
    RoomType specials[16];
    int num_specials;

    int pit_room_needed;//TODO genstate doesnt saves after changing levels, so needs rework
    int weak_floor_created;
} LevelGenState;

int  level_generate(Level *lvl, uint32_t seed, DungeonTheme theme, int depth, int blacksmith_depth);

static inline uint8_t get_tile_safe(const Level *lvl, int x, int y) {
    if (!in_bounds(x,y)) return TILE_WALL;
    return lvl->map[xy_to_idx(x,y)];
}

static inline int room_w     (const Room *r) { return r->right  - r->left; }
static inline int room_h     (const Room *r) { return r->bottom - r->top;  }
static inline int room_square(const Room *r) { return room_w(r) * room_h(r); }
static inline int room_inside(const Room *r, int x, int y) {
    return x > r->left && y > r->top && x < r->right && y < r->bottom;
}