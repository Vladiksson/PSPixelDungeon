#pragma once
#include "common.h"
#include "level.h"
 
typedef struct {
    int x, y;   /* current tile position */
} Player;
 
/* Place player at level entrance */
void player_init(Player *p, const Level *lvl);
 
/* Try to move by (dx,dy). Opens doors on contact.
   Returns 1 if the move succeeded, 0 if blocked. */
int  player_move(Player *p, Level *lvl, int dx, int dy);
 
/* Returns 1 if tile is walkable */
int  player_passable(uint8_t tile);