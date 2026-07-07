#pragma once
#include "common.h"
#include "level.h"

#define HERO_W 12
#define HERO_H 15
//#define HERO_SPACING 1
#define TILESET_HERO_COLS    21
#define TILESET_HERO_ROWS     7
#define TILESET_HERO_W       12
#define TILESET_HERO_H       15

typedef struct {
    int x, y;   /* current tile position */
} Player;
 
/* Place player at level entrance */
void player_init(Player *p, const Level *lvl);
 
/* Try to move by (dx,dy). Opens doors on contact.
   Returns 1 if the move succeeded, 0 if blocked. */
int  player_move(Player *p, Level *lvl, int dx, int dy);
 