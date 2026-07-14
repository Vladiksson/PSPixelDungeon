#include "level_utils.h"
/* ══════════════════════════════════════════════════════════
   LCG RNG
   ══════════════════════════════════════════════════════════ */
uint32_t rng_state;
uint32_t rng_next(void) {
    rng_state = rng_state * 1664525u + 1013904223u;
    return rng_state;
}
int rng_range(int min, int max) {
    if (min >= max) return min;
    return min + (int)((rng_next() >> 16) % (unsigned)(max - min + 1));
}
float rng_float(void) {
    return (float)(rng_next() >> 8) / (float)(1 << 24);
}
int rng_int(int max) {
    if (max <= 1) return 0;
    return (int)((rng_next() >> 16) % (unsigned)max);
}

int isqrt(int n) { int r=0; while((r+1)*(r+1)<=n) r++; return r; }

/* ══════════════════════════════════════════════════════════
   MAP HELPERS
   ══════════════════════════════════════════════════════════ */
void set_tile(Level *lvl, int x, int y, uint8_t t) {
    if (in_bounds(x,y)) lvl->map[xy_to_idx(x,y)] = t;
}

void fill_rect(Level *lvl, int x, int y, int w, int h, uint8_t t) {
    for (int r=y; r<y+h; r++)
        for (int c=x; c<x+w; c++)
            set_tile(lvl,c,r,t);
}

/* ══════════════════════════════════════════════════════════
   ROOM CONNECTION  —  Room.connect()
   ══════════════════════════════════════════════════════════ */
int already_connected(Level *lvl, int a, int b) {
    Room *ra = &lvl->rooms[a];
    for (int i=0; i<ra->n_connected; i++)
        if (ra->connected[i] == b) return 1;
    return 0;
}
void room_connect(Level *lvl, int a, int b) {
    if (already_connected(lvl,a,b)) return;
    Room *ra = &lvl->rooms[a], *rb = &lvl->rooms[b];
    if (ra->n_connected >= MAX_CONN_ROOM || rb->n_connected >= MAX_CONN_ROOM) return;
    ra->connected[ra->n_connected]=b; ra->door_x[ra->n_connected]=-1; ra->door_y[ra->n_connected]=-1; ra->n_connected++;
    rb->connected[rb->n_connected]=a; rb->door_x[rb->n_connected]=-1; rb->door_y[rb->n_connected]=-1; rb->n_connected++;
}