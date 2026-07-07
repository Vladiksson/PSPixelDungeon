#include "player.h"
 
void player_init(Player *p, const Level *lvl) {
    Point ep = idx_to_xy(lvl->entrance);
    p->x = ep.x;
    p->y = ep.y;
}
 
int player_move(Player *p, Level *lvl, int dx, int dy) {
    int nx = p->x + dx;
    int ny = p->y + dy;
    if (!in_bounds(nx, ny)) return 0;
 
    uint8_t tile = lvl->map[xy_to_idx(nx, ny)];
    if (!player_passable(tile)) return 0;
 
    /* Open closed doors on contact (PD behaviour) */
    if (tile == TILE_DOOR || tile == TILE_LOCKED_DOOR)
        lvl->map[xy_to_idx(nx, ny)] = TILE_OPEN_DOOR;
 
    p->x = nx;
    p->y = ny;
    return 1;
}