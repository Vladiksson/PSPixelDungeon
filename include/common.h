#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>

/* ── PSP screen ─────────────────────────────────────── */
#define SCREEN_W   480
#define SCREEN_H   272

/* ── Tile / view ───────────────────────────── */
#define TILE_SIZE_MIN 16
#define TILE_SIZE_MAX 32
#define LEVEL_W    32
#define LEVEL_H    32
#define LEVEL_SIZE (LEVEL_W * LEVEL_H)

extern int gTileSize;
extern int gViewCols;
extern int gViewRows;

void TileSize_Update(void);

/* ── Tileset constants ───────────────────────────────── */
#define TILESET_COLS    16
#define TILESET_ROWS     4
#define TILESET_TILE_W  16
#define TILESET_TILE_H  16
#define TILESET_COUNT    5

/* ── HERO constants ───────────────────────────────── */
#define TILESET_HERO_COLS    21
#define TILESET_HERO_ROWS     7
#define TILESET_HERO_W       12
#define TILESET_HERO_H       15

#define HERO_ARMOR_TIER 6 // TEST, REMOVE AFTER ADDING ARMOR TIER

/* ── ITEM ICON ───────────────────────────────── */
#define ICON_SIZE  16

/* ── Palette  ──────────── */
static const SDL_Color COL_WHITE            = { 0xFF, 0xFF, 0xFF, 0xFF };

static const SDL_Color COL_GOLD_TEXT        = { 0xe8, 0xc5, 0x47, 0xFF };
static const SDL_Color COL_TEXT             = { 0xd8, 0xcd, 0xb8, 0xFF };

static const SDL_Color COL_SLOT_NORMAL      = { 0x4A, 0x4D, 0x44, 0xFF };
static const SDL_Color COL_SLOT_SELECTED    = { 0x78, 0x77, 0x71, 0xFF };
static const SDL_Color COL_SLOT_EQUIP       = { 0x63, 0x66, 0x5B, 0xFF };
static const SDL_Color COL_SLOT_CURSED      = { 0x7A, 0x45, 0x45, 0xFF };
static const SDL_Color COL_SLOT_UNKNOWN     = { 0x40, 0x46, 0x5B, 0xFF };
static const SDL_Color COL_SLOT_GOLD        = { 0x3a, 0x30, 0x24, 0xFF };

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
    TF_PASSABLE        = 1 << 0,
    TF_LOS_BLOCKING    = 1 << 1,
    TF_FLAMABLE        = 1 << 2,
    TF_SECRET          = 1 << 3,
    TF_SOLID           = 1 << 4,
    TF_AVOID           = 1 << 5,
    TF_LIQUID          = 1 << 6,
    TF_PIT             = 1 << 7,
    TF_UNSTITCHABLE    = 1 << 8,
    TF_OPEN_FLOOR      = 1 << 9,
} TileFlags;

extern uint16_t gTileFlags[256];
void Terrain_InitFlags(void);

typedef struct { int x, y; } Point;

static inline int   xy_to_idx (int x, int y) { return y * LEVEL_W + x; }
static inline Point idx_to_xy (int idx)       { return (Point){ idx % LEVEL_W, idx / LEVEL_W }; }
static inline int   in_bounds (int x, int y)  { return x>=0 && y>=0 && x<LEVEL_W && y<LEVEL_H; }