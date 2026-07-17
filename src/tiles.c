#include "common.h"


uint16_t gTileFlags[256];

void Terrain_InitFlags(void){
    memset(gTileFlags, 0, sizeof(gTileFlags));
    gTileFlags[TILE_CHASM]                  = TF_AVOID | TF_PIT | TF_UNSTITCHABLE;
    gTileFlags[TILE_EMPTY]                  = TF_PASSABLE;
    gTileFlags[TILE_GRASS]                  = TF_PASSABLE | TF_FLAMABLE;
    gTileFlags[TILE_EMPTY_WELL]             = TF_PASSABLE;
    gTileFlags[TILE_WATER]                  = TF_PASSABLE | TF_LIQUID | TF_UNSTITCHABLE;
    gTileFlags[TILE_WALL]                   = TF_LOS_BLOCKING | TF_SOLID | TF_UNSTITCHABLE;
    gTileFlags[TILE_DOOR]                   = TF_PASSABLE | TF_LOS_BLOCKING | TF_FLAMABLE | TF_SOLID | TF_UNSTITCHABLE;
    gTileFlags[TILE_OPEN_DOOR]              = TF_PASSABLE | TF_FLAMABLE | TF_UNSTITCHABLE;
    gTileFlags[TILE_ENTRANCE]               = TF_PASSABLE;
    gTileFlags[TILE_EXIT]                   = TF_PASSABLE;
    gTileFlags[TILE_EMBERS]                 = TF_PASSABLE;
    gTileFlags[TILE_LOCKED_DOOR]            = TF_PASSABLE;//ORIGINALLY TF_LOS_BLOCKING | TF_SOLID | TF_UNSTITCHABLE;
    gTileFlags[TILE_PEDESTAL]               = TF_PASSABLE | TF_UNSTITCHABLE;
    gTileFlags[TILE_WALL_DECO]              = gTileFlags[TILE_WALL];
    gTileFlags[TILE_BARRICADE]              = TF_FLAMABLE | TF_SOLID | TF_LOS_BLOCKING;
    gTileFlags[TILE_EMPTY_SP]               = gTileFlags[TILE_EMPTY] | TF_UNSTITCHABLE; 
    gTileFlags[TILE_HIGH_GRASS]             = TF_PASSABLE | TF_LOS_BLOCKING | TF_FLAMABLE;
    gTileFlags[TILE_EMPTY_DECO]             = gTileFlags[TILE_EMPTY];
    gTileFlags[TILE_LOCKED_EXIT]            = TF_PASSABLE;//ORIGINALLY TF_SOLID;
    gTileFlags[TILE_UNLOCKED_EXIT]          = TF_PASSABLE;
    gTileFlags[TILE_SIGN]                   = TF_PASSABLE | TF_FLAMABLE;
    gTileFlags[TILE_WELL]                   = TF_AVOID;
    gTileFlags[TILE_STATUE]                 = TF_SOLID;
    gTileFlags[TILE_STATUE_SP]              = gTileFlags[TILE_STATUE] | TF_UNSTITCHABLE;
    gTileFlags[TILE_BOOKSHELF]              = gTileFlags[TILE_BARRICADE] | TF_UNSTITCHABLE;
    gTileFlags[TILE_ALCHEMY]                = TF_PASSABLE;
    gTileFlags[TILE_CHASM_WALL]             = gTileFlags[TILE_CHASM];
    gTileFlags[TILE_CHASM_FLOOR]            = gTileFlags[TILE_CHASM];
    gTileFlags[TILE_CHASM_FLOOR_SP]         = gTileFlags[TILE_CHASM];
    gTileFlags[TILE_CHASM_WATER]            = gTileFlags[TILE_CHASM];
    gTileFlags[TILE_SECRET_DOOR]            = gTileFlags[TILE_WALL] | TF_SECRET | TF_UNSTITCHABLE;
    gTileFlags[TILE_TOXIC_TRAP]             = TF_AVOID;
    gTileFlags[TILE_SECRET_TOXIC_TRAP]      = gTileFlags[TILE_EMPTY] | TF_SECRET;
    gTileFlags[TILE_FIRE_TRAP]              = TF_AVOID;
    gTileFlags[TILE_SECRET_FIRE_TRAP]       = gTileFlags[TILE_EMPTY] | TF_SECRET;
    gTileFlags[TILE_PARALYTIC_TRAP]         = TF_AVOID;
    gTileFlags[TILE_SECRET_PARALYTIC_TRAP]  = gTileFlags[TILE_EMPTY] | TF_SECRET;
    gTileFlags[TILE_INACTIVE_TRAP]          = gTileFlags[TILE_EMPTY];
    gTileFlags[TILE_POISON_TRAP]            = TF_AVOID;
    gTileFlags[TILE_SECRET_POISON_TRAP]     = gTileFlags[TILE_EMPTY] | TF_SECRET;
    gTileFlags[TILE_ALARM_TRAP]             = TF_AVOID;
    gTileFlags[TILE_SECRET_ALARM_TRAP]      = gTileFlags[TILE_EMPTY] | TF_SECRET;
    gTileFlags[TILE_LIGHTNING_TRAP]         = TF_AVOID;
    gTileFlags[TILE_SECRET_LIGHTNING_TRAP]  = gTileFlags[TILE_EMPTY] | TF_SECRET;
    gTileFlags[TILE_GRIPPING_TRAP]          = TF_AVOID;
    gTileFlags[TILE_SECRET_GRIPPING_TRAP]   = gTileFlags[TILE_EMPTY] | TF_SECRET;
    gTileFlags[TILE_SUMMONING_TRAP]         = TF_AVOID;
    gTileFlags[TILE_SECRET_SUMMONING_TRAP]  = gTileFlags[TILE_EMPTY] | TF_SECRET;
    for (int i = TILE_WATER_TILES; i < TILE_WATER_TILES + 16; ++i){
        gTileFlags[i]                       = gTileFlags[TILE_WATER];
    }
}
