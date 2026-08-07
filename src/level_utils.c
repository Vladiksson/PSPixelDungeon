#include "level_utils.h"

/* ══════════════════════════════════════════════════════════
   SOME DEPTH-DEPENDED STUFF
   ══════════════════════════════════════════════════════════ */
int level_is_boss_depth(int depth) { 
    return depth >= 0 && (depth + 1) % 5 == 0;
}

int shop_on_level(int depth){
    return depth == 5 || depth == 10 || depth == 15;
}

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

/* ══════════════════════════════════════════════════════════
   BSP ROOM SPLITTING  —  RegularLevel.split()
   ══════════════════════════════════════════════════════════ */
#define MIN_ROOM  7
#define MAX_ROOM  9

void bsp_split(Level *lvl, int left, int top, int right, int bottom) {
    if (lvl->num_rooms >= MAX_ROOMS) return;
    int w = right - left;
    int h = bottom - top;

    if (w > MAX_ROOM && h < MIN_ROOM) {
        int vw = rng_range(left+3, right-3);
        bsp_split(lvl, left,top, vw,bottom);
        bsp_split(lvl, vw,  top, right,bottom);
    } else if (h > MAX_ROOM && w < MIN_ROOM) {
        int vh = rng_range(top+3, bottom-3);
        bsp_split(lvl, left,top,   right,vh);
        bsp_split(lvl, left,vh,    right,bottom);
    } else if (w < MIN_ROOM || h < MIN_ROOM ||
               (rng_float() <= (float)(MIN_ROOM*MIN_ROOM)/(float)(w*h)
                && w <= MAX_ROOM && h <= MAX_ROOM)) {
        Room *r = &lvl->rooms[lvl->num_rooms++];
        memset(r, 0, sizeof(*r));
        r->left   = left;  r->top    = top;
        r->right  = right; r->bottom = bottom;
        r->type   = ROOM_NULL;
        r->price  = 1;
        r->distance = 10000;
        for (int k=0; k<MAX_CONN_ROOM; k++) { r->door_x[k]=-1; r->door_y[k]=-1; }
    } else {
        if (rng_float() < (float)(w-2)/(float)(w+h-4)) {
            int vw = rng_range(left+3, right-3);
            bsp_split(lvl, left,top, vw,bottom);
            bsp_split(lvl, vw,  top, right,bottom);
        } else {
            int vh = rng_range(top+3, bottom-3);
            bsp_split(lvl, left,top,  right,vh);
            bsp_split(lvl, left,vh,   right,bottom);
        }
    }
}

/* ══════════════════════════════════════════════════════════
   NEIGHBOUR GRAPH  —  Room.addNeigbour()
   ══════════════════════════════════════════════════════════ */
void build_neighbours(Level *lvl) {
    for (int i=0; i<lvl->num_rooms-1; i++) {
        for (int j=i+1; j<lvl->num_rooms; j++) {
            Room *a = &lvl->rooms[i], *b = &lvl->rooms[j];
            int ix0 = a->left   > b->left   ? a->left   : b->left;
            int ix1 = a->right  < b->right  ? a->right  : b->right;
            int iy0 = a->top    > b->top    ? a->top    : b->top;
            int iy1 = a->bottom < b->bottom ? a->bottom : b->bottom;
            int iw = ix1-ix0, ih = iy1-iy0;
            if ((iw==0 && ih>=3) || (ih==0 && iw>=3)) {
                if (a->n_neighbours < MAX_NEIGHBOURS) a->neighbours[a->n_neighbours++] = j;
                if (b->n_neighbours < MAX_NEIGHBOURS) b->neighbours[b->n_neighbours++] = i;
            }
        }
    }
}

/* ══════════════════════════════════════════════════════════
   GRAPH PATHFINDING  —  Graph.java
   ══════════════════════════════════════════════════════════ */
void build_distance_map(Level *lvl, int target) {
    for (int i=0; i<lvl->num_rooms; i++) lvl->rooms[i].distance = 10000;
    int queue[MAX_ROOMS*4], head=0, tail=0;
    lvl->rooms[target].distance = 0;
    queue[tail++] = target;
    while (head < tail) {
        int ci = queue[head++];
        Room *c = &lvl->rooms[ci];
        for (int i=0; i<c->n_neighbours; i++) {
            int ni = c->neighbours[i];
            int nd = c->distance + lvl->rooms[ni].price;
            if (nd < lvl->rooms[ni].distance) {
                lvl->rooms[ni].distance = nd;
                queue[tail++] = ni;
            }
        }
    }
}

int build_path(Level *lvl, int from, int to, int *path) {
    int len=0, curr=from;
    int visited[MAX_ROOMS]; memset(visited,0,sizeof(visited));
    while (curr != to && len < MAX_ROOMS-1) {
        path[len++] = curr; visited[curr] = 1;
        Room *r = &lvl->rooms[curr];
        int best=-1, best_d=r->distance;
        for (int i=0; i<r->n_neighbours; i++) {
            int ni = r->neighbours[i];
            if (!visited[ni] && lvl->rooms[ni].distance < best_d) {
                best_d = lvl->rooms[ni].distance; best = ni;
            }
        }
        if (best == -1) break;
        curr = best;
    }
    path[len++] = to;
    return len;
}

void set_price(Level *lvl, int *path, int len, int price) {
    for (int i=0; i<len; i++) lvl->rooms[path[i]].price = price;
}





//TEST SOLUTION FOR NEAR ENTRANCE/EXIT
int rooms_are_neighbours(const Level *lvl, int a, int b) {
    const Room *r = &lvl->rooms[a];

    for (int i = 0; i < r->n_neighbours; i++) {
        if (r->neighbours[i] == b)
            return 1;
    }

    return 0;
}