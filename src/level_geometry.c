#include "level_geometry.h"

/* ══════════════════════════════════════════════════════════
   Level-derived geometry HELPERS
   ══════════════════════════════════════════════════════════ */

const int DX[8] = { +1, -1,  0,  0, +1, +1, -1, -1 };
const int DY[8] = {  0,  0, +1, -1, +1, -1, +1, -1 };

int cell_distance(int a, int b) {
    Point pa = idx_to_xy(a);
    Point pb = idx_to_xy(b);
    int dx = pa.x - pb.x; if (dx < 0) dx = -dx;
    int dy = pa.y - pb.y; if (dy < 0) dy = -dy;
    return dx > dy ? dx : dy;
}
int cells_adjacent(int a, int b) {
    return cell_distance(a, b) == 1;
}

int neighbour_idx(int pos, int dx, int dy) {
    Point p = idx_to_xy(pos);
    int nx = p.x + dx;
    int ny = p.y + dy;
    if (!in_bounds_xy(nx, ny))
        return -1;
    return xy_to_idx(nx, ny);
}

int random_neighbour(int pos) {
    int i = rng_int(8);
    return neighbour_idx(pos, DX[i], DY[i]);
}

/* ══════════════════════════════════════════════════════════
   CELL IS... HELPERS
   ══════════════════════════════════════════════════════════ */
int cell_passable(const Level *lvl, int idx) {
    return (gTileFlags[lvl->map[idx]] & TF_PASSABLE) != 0;
}
int blocks_light(const Level *lvl, int idx) {
    return (gTileFlags[lvl->map[idx]] & TF_LOS_BLOCKING) != 0;
}
int cell_flamable(const Level *lvl, int idx) {
    return (gTileFlags[lvl->map[idx]] & TF_FLAMABLE) != 0;
}
int cell_avoid(const Level *lvl, int idx) {
    return (gTileFlags[lvl->map[idx]] & TF_AVOID) != 0;
}
int cell_pit(const Level *lvl, int idx) {
    return (gTileFlags[lvl->map[idx]] & TF_PIT) != 0;
}
int cell_secret(const Level *lvl, int idx) {
    return (gTileFlags[lvl->map[idx]] & TF_SECRET) != 0;
}
int cell_solid(const Level *lvl, int idx) {
    return (gTileFlags[lvl->map[idx]] & TF_SOLID) != 0;
}
int cell_liquid(const Level *lvl, int idx) {
    return (gTileFlags[lvl->map[idx]] & TF_LIQUID) != 0;
}

bool tile_unstitchable(uint8_t t) {
    return (gTileFlags[t] & TF_UNSTITCHABLE) != 0;
}
int tile_free_for_item(const Level *lvl, int idx) {
    if (idx == lvl->entrance || idx == lvl->exit) 
        return 0;
    return (gTileFlags[lvl->map[idx]] & TF_OPEN_FLOOR) != 0;
}

int tile_discover(const Level *lvl, int idx) {
    switch (lvl->map[idx]) {
        case TILE_SECRET_DOOR:            return TILE_DOOR;
        case TILE_SECRET_FIRE_TRAP:       return TILE_FIRE_TRAP;
        case TILE_SECRET_PARALYTIC_TRAP:  return TILE_PARALYTIC_TRAP;
        case TILE_SECRET_TOXIC_TRAP:      return TILE_TOXIC_TRAP;
        case TILE_SECRET_POISON_TRAP:     return TILE_POISON_TRAP;
        case TILE_SECRET_ALARM_TRAP:      return TILE_ALARM_TRAP;
        case TILE_SECRET_LIGHTNING_TRAP:  return TILE_LIGHTNING_TRAP;
        case TILE_SECRET_GRIPPING_TRAP:   return TILE_GRIPPING_TRAP;
        case TILE_SECRET_SUMMONING_TRAP:  return TILE_SUMMONING_TRAP;
        default:                          return lvl->map[idx];
    }
}