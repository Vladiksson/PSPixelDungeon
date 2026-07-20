#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* ── PSP screen ─────────────────────────────────────── */
#define SCREEN_W   480
#define SCREEN_H   272

/* ── Tile / view constants ───────────────────────────── */
#define TILE_SIZE  16
#define LEVEL_W    32       
#define LEVEL_H    32      
#define LEVEL_SIZE (LEVEL_W * LEVEL_H)
#define VIEW_COLS  (SCREEN_W / TILE_SIZE)   /* 30 */
#define VIEW_ROWS  (SCREEN_H / TILE_SIZE)   /* 17 */

/* ── Tileset constants ───────────────────────────────── */
#define TILESET_COLS    16
#define TILESET_ROWS     4
#define TILESET_TILE_W  16
#define TILESET_TILE_H  16
#define TILESET_COUNT    5

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
    int left, top, right, bottom;

    int neighbours   [MAX_NEIGHBOURS];
    int n_neighbours;
    int distance;             /* BFS distance (pathfinding)   */
    int price;                /* Dijkstra cost (default 1)    */

    int connected [MAX_CONN_ROOM];
    int door_x    [MAX_CONN_ROOM];
    int door_y    [MAX_CONN_ROOM];
    int n_connected;

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

/* ── Terrain tile IDs — exact mirror of Terrain.java ─── */
typedef enum {
    TILE_CHASM                  =  0,
    TILE_EMPTY                  =  1,
    TILE_GRASS                  =  2,
    TILE_EMPTY_WELL             =  3,
    TILE_WALL                   =  4,
    TILE_DOOR                   =  5,
    TILE_OPEN_DOOR              =  6,
    TILE_ENTRANCE               =  7,
    TILE_EXIT                   =  8,
    TILE_EMBERS                 =  9,
    TILE_LOCKED_DOOR            = 10,
    TILE_PEDESTAL               = 11,
    TILE_WALL_DECO              = 12,
    TILE_BARRICADE              = 13,
    TILE_EMPTY_SP               = 14,
    TILE_HIGH_GRASS             = 15,
    TILE_EMPTY_DECO             = 24,
    TILE_LOCKED_EXIT            = 25,
    TILE_UNLOCKED_EXIT          = 26,
    TILE_SIGN                   = 29,
    TILE_WELL                   = 34,
    TILE_STATUE                 = 35,
    TILE_STATUE_SP              = 36,
    TILE_BOOKSHELF              = 41,
    TILE_ALCHEMY                = 42,    
    TILE_CHASM_FLOOR		    = 43,
	TILE_CHASM_FLOOR_SP	        = 44,
	TILE_CHASM_WALL		        = 45,
	TILE_CHASM_WATER		    = 46,
    TILE_SECRET_DOOR            = 16,
    TILE_TOXIC_TRAP			    = 17,
    TILE_SECRET_TOXIC_TRAP	    = 18,
    TILE_FIRE_TRAP			    = 19,
    TILE_SECRET_FIRE_TRAP	    = 20,
    TILE_PARALYTIC_TRAP         = 21,
	TILE_SECRET_PARALYTIC_TRAP  = 22,
	TILE_INACTIVE_TRAP			= 23,
    TILE_POISON_TRAP            = 27,
    TILE_SECRET_POISON_TRAP     = 28,
    TILE_ALARM_TRAP             = 30,
    TILE_SECRET_ALARM_TRAP      = 31,
    TILE_LIGHTNING_TRAP         = 32,
    TILE_SECRET_LIGHTNING_TRAP  = 33,
    TILE_GRIPPING_TRAP          = 37,
    TILE_SECRET_GRIPPING_TRAP   = 38,
    TILE_SUMMONING_TRAP         = 39,
    TILE_SECRET_SUMMONING_TRAP  = 40,
    TILE_WATER                  = 63,
    TILE_WATER_TILES            = 48,
} TileType;

typedef enum {
    TF_PASSABLE      = 0x0001,
    TF_LOS_BLOCKING  = 0x0002,
    TF_FLAMABLE      = 0x0004,
    TF_SECRET        = 0x0008,
    TF_SOLID         = 0x0010,
    TF_AVOID         = 0x0020,
    TF_LIQUID        = 0x0040,
    TF_PIT           = 0x0080,
    TF_UNSTITCHABLE  = 0x0100,
    TF_SHADOW        = 0x0200, //NOT ORIGINAL
} TileFlags;

extern uint16_t gTileFlags[256];
void Terrain_InitFlags(void);

typedef enum {
    THEME_SEWERS = 0,
    THEME_PRISON = 1,
    THEME_CAVES  = 2,
    THEME_CITY   = 3,
    THEME_HALLS  = 4,
} DungeonTheme;

typedef struct { int x, y; } Point;

static inline int   xy_to_idx (int x, int y) { return y * LEVEL_W + x; }
static inline Point idx_to_xy (int idx)       { return (Point){ idx % LEVEL_W, idx / LEVEL_W }; }
static inline int   in_bounds (int x, int y)  { return x>=0 && y>=0 && x<LEVEL_W && y<LEVEL_H; }