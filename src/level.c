#include "level.h"
#include <stdio.h>
#include <time.h>

/* ══════════════════════════════════════════════════════════
   LCG RNG
   ══════════════════════════════════════════════════════════ */
static uint32_t rng_state;
static uint32_t rng_next(void) {
    rng_state = rng_state * 1664525u + 1013904223u;
    return rng_state;
}
static int rng_range(int lo, int hi) {
    if (lo >= hi) return lo;
    return lo + (int)(rng_next() % (unsigned)(hi - lo + 1));
}
static float rng_float(void) {
    return (float)(rng_next() >> 8) / (float)(1 << 24);
}
static int rng_int(int hi) {
    if (hi <= 1) return 0;
    return (int)(rng_next() % (unsigned)hi);
}

/* ══════════════════════════════════════════════════════════
   MAP HELPERS
   ══════════════════════════════════════════════════════════ */
static void set_tile(Level *lvl, int x, int y, uint8_t t) {
    if (in_bounds(x,y)) lvl->map[xy_to_idx(x,y)] = t;
}

/*
 * Painter.fill(level, x, y, w, h, value) — exact port.
 * Fills w columns × h rows starting at (x,y).
 */
static void fill_rect(Level *lvl, int x, int y, int w, int h, uint8_t t) {
    for (int r=y; r<y+h; r++)
        for (int c=x; c<x+w; c++)
            set_tile(lvl,c,r,t);
}

/* ══════════════════════════════════════════════════════════
   BSP ROOM SPLITTING  —  RegularLevel.split()
   ══════════════════════════════════════════════════════════ */
#define MIN_ROOM  7
#define MAX_ROOM  9

static void bsp_split(Level *lvl, int left, int top, int right, int bottom) {
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
static void build_neighbours(Level *lvl) {
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
static void bfs_from(Level *lvl, int target) {
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

static int build_path(Level *lvl, int from, int to, int *path) {
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

static void set_price(Level *lvl, int *path, int len, int price) {
    for (int i=0; i<len; i++) lvl->rooms[path[i]].price = price;
}

/* ══════════════════════════════════════════════════════════
   ROOM CONNECTION  —  Room.connect()
   ══════════════════════════════════════════════════════════ */
static int already_connected(Level *lvl, int a, int b) {
    Room *ra = &lvl->rooms[a];
    for (int i=0; i<ra->n_connected; i++)
        if (ra->connected[i] == b) return 1;
    return 0;
}
static void room_connect(Level *lvl, int a, int b) {
    if (already_connected(lvl,a,b)) return;
    Room *ra = &lvl->rooms[a], *rb = &lvl->rooms[b];
    if (ra->n_connected >= MAX_CONN_ROOM || rb->n_connected >= MAX_CONN_ROOM) return;
    ra->connected[ra->n_connected]=b; ra->door_x[ra->n_connected]=-1; ra->door_y[ra->n_connected]=-1; ra->n_connected++;
    rb->connected[rb->n_connected]=a; rb->door_x[rb->n_connected]=-1; rb->door_y[rb->n_connected]=-1; rb->n_connected++;
}

/* ══════════════════════════════════════════════════════════
   ROOM TYPE ASSIGNMENT  —  RegularLevel.assignRoomType()
   ══════════════════════════════════════════════════════════ */
static void assign_types(Level *lvl, int entrance_idx, int exit_idx) {
    lvl->rooms[entrance_idx].type = ROOM_ENTRANCE;
    lvl->rooms[exit_idx].type     = ROOM_EXIT;
    int n_std = 0;
    for (int i=0; i<lvl->num_rooms; i++) {
        Room *r = &lvl->rooms[i];
        if (r->type != ROOM_NULL) continue;
        int nc = r->n_connected;
        if (nc == 0) continue;
        if (rng_int(nc*nc) == 0) { r->type = ROOM_STANDARD; n_std++; }
        else                       r->type = ROOM_TUNNEL;
    }
    for (int i=0; n_std<4 && i<lvl->num_rooms; i++)
        if (lvl->rooms[i].type == ROOM_TUNNEL) { lvl->rooms[i].type = ROOM_STANDARD; n_std++; }
}

/* ══════════════════════════════════════════════════════════
   DOOR POSITION CALCULATION  —  RegularLevel.placeDoors()
   ══════════════════════════════════════════════════════════ */
static void compute_doors(Level *lvl) {
    for (int i=0; i<lvl->num_rooms; i++) {
        Room *r = &lvl->rooms[i];
        for (int j=0; j<r->n_connected; j++) {
            if (r->door_x[j] != -1) continue;
            int ni = r->connected[j]; Room *n = &lvl->rooms[ni];
            int ix0 = r->left  > n->left  ? r->left  : n->left;
            int ix1 = r->right < n->right ? r->right : n->right;
            int iy0 = r->top    > n->top    ? r->top    : n->top;
            int iy1 = r->bottom < n->bottom ? r->bottom : n->bottom;
            int iw = ix1-ix0;
            int dx, dy;
            if (iw == 0) { dx=ix0;                        dy=rng_range(iy0+1, iy1-1); }
            else          { dx=rng_range(ix0+1, ix1-1);   dy=iy0; }
            r->door_x[j]=dx; r->door_y[j]=dy;
            for (int k=0; k<n->n_connected; k++)
                if (n->connected[k]==i) { n->door_x[k]=dx; n->door_y[k]=dy; break; }
        }
    }
}

/* ══════════════════════════════════════════════════════════
   ROOM PAINTING  —  StandardPainter.paint()

   Standard/Entrance/Exit rooms:
     1. Fill entire room bounds with WALL   (Painter.fill(room, WALL))
     2. Fill interior (margin 1) with EMPTY (Painter.fill(room, 1, EMPTY))

   Painter.fill(rect, value):
     fill(left, top, width()+1, height()+1, value)
     width()+1 = (right-left)+1  = right-left+1  total columns

   Painter.fill(rect, 1, value):
     fill(left+1, top+1, width()-1, height()-1, value)
     width()-1  = (right-left)-1               interior columns
   ══════════════════════════════════════════════════════════ */
static void paint_rooms(Level *lvl) {
    for (int i=0; i<lvl->num_rooms; i++) {
        Room *r = &lvl->rooms[i];
        /* TUNNEL rooms are painted separately — only their path is carved */
        if (r->type == ROOM_NULL || r->type == ROOM_TUNNEL) continue;

        /* Step 1: Painter.fill(room, WALL) — reset whole room to wall
           width()+1 = room_w(r)+1 cols, height()+1 = room_h(r)+1 rows */
        fill_rect(lvl, r->left, r->top, room_w(r)+1, room_h(r)+1, TILE_WALL);

        /* Step 2: Painter.fill(room, 1, EMPTY) — carve interior
           width()-1 = room_w(r)-1, height()-1 = room_h(r)-1           */
        fill_rect(lvl, r->left+1, r->top+1, room_w(r)-1, room_h(r)-1, TILE_EMPTY);
    }
}

/* ══════════════════════════════════════════════════════════
   TUNNEL PAINTING  —  TunnelPainter.paint()

   KEY INSIGHT: tunnel rooms do NOT fill their interior.
   They draw a narrow L-shaped PATH connecting all doors
   through the room center — this is what creates corridors.

   Algorithm (horizontal-primary, wider rooms):
     - Draw horizontal spine at center.y from 'from' to 'to'
     - For each door draw a vertical stub from door.y to center.y
     - 'from'/'to' track the x-span of the spine
   Vertical-primary mirrors this with axes swapped.
   ══════════════════════════════════════════════════════════ */
static void paint_tunnel_room(Level *lvl, Room *r) {
    int floor = TILE_EMPTY;   /* tunnelTile() = EMPTY for sewers/default */
    int cx = (r->left + r->right)  / 2;
    int cy = (r->top  + r->bottom) / 2;
    int w  = room_w(r);   /* right - left */
    int h  = room_h(r);   /* bottom - top */

    if (w > h || (w == h && rng_int(2) == 0)) {
        /* ── Horizontal-primary tunnel ─────────────────── */
        /* 'from' will become leftmost x of the spine
           'to'   will become rightmost x of the spine
           Initialised to room interior edges so they can
           only shrink inward (from) or grow outward (to). */
        int from = r->right - 1;
        int to   = r->left  + 1;

        for (int j=0; j<r->n_connected; j++) {
            int dx = r->door_x[j], dy = r->door_y[j];
            if (dx < 0 || dy < 0) continue;
            int step = (dy < cy) ? +1 : -1;

            if (dx == r->left) {
                /* Door on left wall: stub goes from door.y toward cy
                   along the left interior column (left+1) */
                from = r->left + 1;
                for (int k=dy; k != cy; k += step)
                    set_tile(lvl, from, k, floor);

            } else if (dx == r->right) {
                /* Door on right wall: stub along right interior col */
                to = r->right - 1;
                for (int k=dy; k != cy; k += step)
                    set_tile(lvl, to, k, floor);

            } else {
                /* Door on top/bottom: extend spine to this x,
                   draw stub from door.y+step toward cy */
                if (dx < from) from = dx;
                if (dx > to)   to   = dx;
                for (int k=dy+step; k != cy; k += step)
                    set_tile(lvl, dx, k, floor);
            }
        }
        /* Horizontal spine */
        for (int k=from; k<=to; k++)
            set_tile(lvl, k, cy, floor);

    } else {
        /* ── Vertical-primary tunnel (axes swapped) ─────── */
        int from = r->bottom - 1;
        int to   = r->top    + 1;

        for (int j=0; j<r->n_connected; j++) {
            int dx = r->door_x[j], dy = r->door_y[j];
            if (dx < 0 || dy < 0) continue;
            int step = (dx < cx) ? +1 : -1;

            if (dy == r->top) {
                from = r->top + 1;
                for (int k=dx; k != cx; k += step)
                    set_tile(lvl, k, from, floor);

            } else if (dy == r->bottom) {
                to = r->bottom - 1;
                for (int k=dx; k != cx; k += step)
                    set_tile(lvl, k, to, floor);

            } else {
                if (dy < from) from = dy;
                if (dy > to)   to   = dy;
                for (int k=dx+step; k != cx; k += step)
                    set_tile(lvl, k, dy, floor);
            }
        }
        /* Vertical spine */
        for (int k=from; k<=to; k++)
            set_tile(lvl, cx, k, floor);
    }
}

static void paint_tunnels(Level *lvl) {
    for (int i=0; i<lvl->num_rooms; i++)
        if (lvl->rooms[i].type == ROOM_TUNNEL)
            paint_tunnel_room(lvl, &lvl->rooms[i]);
}

/* ══════════════════════════════════════════════════════════
   DOOR PAINTING  —  RegularLevel.paintDoors()

   Door types (set by painters):
     TUNNEL  → TILE_EMPTY  (tunnelTile — open passage)
     REGULAR → TILE_DOOR   (closed door)

   In PD, TunnelPainter sets all its doors to Door.Type.TUNNEL,
   StandardPainter sets all its doors to Door.Type.REGULAR.
   We replicate this: if either room is TUNNEL → open passage.
   ══════════════════════════════════════════════════════════ */
static void paint_doors(Level *lvl) {
    for (int i=0; i<lvl->num_rooms; i++) {
        Room *r = &lvl->rooms[i];
        if (r->type == ROOM_NULL) continue;
        for (int j=0; j<r->n_connected; j++) {
            if (r->connected[j] < i) continue;   /* each pair once */
            int ni = r->connected[j];
            Room *n = &lvl->rooms[ni];
            if (n->type == ROOM_NULL) continue;
            int dx = r->door_x[j], dy = r->door_y[j];
            if (dx < 0 || dy < 0) continue;
            
            uint8_t tile = (r->type == ROOM_TUNNEL && n->type == ROOM_TUNNEL)
                ? TILE_EMPTY
                : TILE_DOOR;
            set_tile(lvl, dx, dy, tile);
        }
    }
}

/* ══════════════════════════════════════════════════════════
   PATCH  —  Patch.java (cellular automaton)
   ══════════════════════════════════════════════════════════ */
static void patch_generate(float seed, int n_gen, uint8_t *out) {
    static uint8_t buf_a[LEVEL_SIZE], buf_b[LEVEL_SIZE];
    uint8_t *curr=buf_a, *next=buf_b;
    for (int i=0; i<LEVEL_SIZE; i++) curr[i] = rng_float()<seed ? 1 : 0;
    for (int g=0; g<n_gen; g++) {
        memset(next,0,LEVEL_SIZE);
        for (int y=1; y<LEVEL_H-1; y++) {
            for (int x=1; x<LEVEL_W-1; x++) {
                int p=x+y*LEVEL_W;
                int nb=curr[p-LEVEL_W-1]+curr[p-LEVEL_W]+curr[p-LEVEL_W+1]
                       +curr[p-1]                        +curr[p+1]
                       +curr[p+LEVEL_W-1]+curr[p+LEVEL_W]+curr[p+LEVEL_W+1];
                if (!curr[p]&&nb>=5) next[p]=1;
                else if (curr[p]&&nb>=4) next[p]=1;
            }
        }
        uint8_t *tmp=curr; curr=next; next=tmp;
    }
    memcpy(out,curr,LEVEL_SIZE);
}
static void paint_water(Level *lvl, const uint8_t *lake) {
    for (int i=0; i<LEVEL_SIZE; i++)
        if (lvl->map[i]==TILE_EMPTY && lake[i]) lvl->map[i]=TILE_WATER;
}

static void paint_grass(Level *lvl, const uint8_t *grass) {
    /* paintGrass: for each EMPTY tile with grass patch set,
       probability of HIGH_GRASS vs GRASS depends on how many
       of its 8 neighbours also have the grass patch set.
       map[i] = (Random.Float() < count/12f) ? HIGH_GRASS : GRASS */
    for (int i=LEVEL_W+1; i<LEVEL_SIZE-LEVEL_W-1; i++) {
        if (lvl->map[i]==TILE_EMPTY && grass[i]) {
            int x=i%LEVEL_W, y=i/LEVEL_W;
            int count=1;
            if (grass[i-LEVEL_W-1]) count++;
            if (grass[i-LEVEL_W  ]) count++;
            if (grass[i-LEVEL_W+1]) count++;
            if (grass[i        -1]) count++;
            if (grass[i        +1]) count++;
            if (grass[i+LEVEL_W-1]) count++;
            if (grass[i+LEVEL_W  ]) count++;
            if (grass[i+LEVEL_W+1]) count++;
            (void)x; (void)y;
            lvl->map[i] = (rng_float() < (float)count/12.0f)
                          ? TILE_HIGH_GRASS : TILE_GRASS;
        }
    }
}




/* ══════════════════════════════════════════════════════════
   DECORATION  —  SewerLevel.decorate() etc.
   ══════════════════════════════════════════════════════════ */
static void decorate_sewers(Level *lvl) {
    for (int x=0; x<LEVEL_W; x++) {
        int i=x;
        if (lvl->map[i]==TILE_WALL && lvl->map[i+LEVEL_W]==TILE_WATER && (rng_next()%4)==0)
            lvl->map[i]=TILE_WALL_DECO;
    }
    for (int i=LEVEL_W; i<LEVEL_SIZE-LEVEL_W; i++) {
        if (lvl->map[i]==TILE_WALL && lvl->map[i-LEVEL_W]==TILE_WALL
            && lvl->map[i+LEVEL_W]==TILE_WATER && (rng_next()%2)==0)
            lvl->map[i]=TILE_WALL_DECO;
    }
    for (int i=LEVEL_W+1; i<LEVEL_SIZE-LEVEL_W-1; i++) {
        if (lvl->map[i]==TILE_EMPTY) {
            int cnt=(lvl->map[i+1]==TILE_WALL)+(lvl->map[i-1]==TILE_WALL)
                   +(lvl->map[i+LEVEL_W]==TILE_WALL)+(lvl->map[i-LEVEL_W]==TILE_WALL);
            if ((int)(rng_next()%16)<cnt*cnt) lvl->map[i]=TILE_EMPTY_DECO;
        }
    }
    Point ep=idx_to_xy(lvl->entrance);
    for (int i=0; i<lvl->num_rooms; i++) {
        if (!room_inside(&lvl->rooms[i], ep.x, ep.y)) continue;
        for (int t=0; t<40; t++) {
            Room *r=&lvl->rooms[i];
            int rx=rng_range(r->left+1, r->right-1);
            int ry=rng_range(r->top +1, r->bottom-1);
            int idx=xy_to_idx(rx,ry);
            if (idx!=lvl->entrance && lvl->map[idx]==TILE_EMPTY) { lvl->map[idx]=TILE_SIGN; break; }
        }
        break;
    }
}
static void decorate_prison(Level *lvl) {
    for (int i=LEVEL_W; i<LEVEL_SIZE-LEVEL_W; i++)
        if (lvl->map[i]==TILE_WALL && lvl->map[i-LEVEL_W]==TILE_WALL
            && lvl->map[i+LEVEL_W]==TILE_WATER && (rng_next()%2)==0)
            lvl->map[i]=TILE_WALL_DECO;
    for (int i=LEVEL_W+1; i<LEVEL_SIZE-LEVEL_W-1; i++)
        if (lvl->map[i]==TILE_EMPTY) {
            int cnt=(lvl->map[i+1]==TILE_WALL)+(lvl->map[i-1]==TILE_WALL)
                   +(lvl->map[i+LEVEL_W]==TILE_WALL)+(lvl->map[i-LEVEL_W]==TILE_WALL);
            if ((int)(rng_next()%16)<cnt*cnt) lvl->map[i]=TILE_EMPTY_DECO;
        }
}
static void decorate_generic(Level *lvl) {
    for (int i=0; i<LEVEL_SIZE; i++)
        if (lvl->map[i]==TILE_WALL && (rng_next()%20)==0) lvl->map[i]=TILE_WALL_DECO;
}
static void place_embers(Level *lvl) {
    Point ep=idx_to_xy(lvl->exit);
    for (int dy=-1;dy<=1;dy++) for (int dx=-1;dx<=1;dx++) {
        if (!dx&&!dy) continue;
        int nx=ep.x+dx, ny=ep.y+dy;
        if (in_bounds(nx,ny) && lvl->map[xy_to_idx(nx,ny)]==TILE_EMPTY && (rng_next()%3)==0)
            lvl->map[xy_to_idx(nx,ny)]=TILE_EMBERS;
    }
}

static int isqrt(int n) { int r=0; while((r+1)*(r+1)<=n) r++; return r; }

/* ══════════════════════════════════════════════════════════
   PUBLIC: level_generate
   ══════════════════════════════════════════════════════════ */
int level_generate(Level *lvl, uint32_t seed, DungeonTheme theme) {
    rng_state = seed ? seed : (uint32_t)time(NULL);
    memset(lvl, 0, sizeof(*lvl));
    lvl->theme = theme;
    memset(lvl->map, TILE_WALL, sizeof(lvl->map));

    /* 1 ── BSP split + neighbour graph */
    int retry=0;
    do {
        lvl->num_rooms=0;
        bsp_split(lvl, 0, 0, LEVEL_W-1, LEVEL_H-1);
        if (++retry > 10) return -1;
    } while (lvl->num_rooms < 8);
    build_neighbours(lvl);

    /* 2 ── Entrance / exit (far apart, width/height ≥ 4) */
    int entrance_r=-1, exit_r=-1, attempts=0;
    int min_dist=isqrt(lvl->num_rooms);
    do {
        do { entrance_r=rng_range(0,lvl->num_rooms-1); }
        while (room_w(&lvl->rooms[entrance_r])<4 || room_h(&lvl->rooms[entrance_r])<4);
        do { exit_r=rng_range(0,lvl->num_rooms-1); }
        while (exit_r==entrance_r || room_w(&lvl->rooms[exit_r])<4 || room_h(&lvl->rooms[exit_r])<4);
        bfs_from(lvl, exit_r);
        if (lvl->rooms[entrance_r].distance >= min_dist) break;
    } while (++attempts < 20);
    if (entrance_r<0 || exit_r<0) return -1;

    /* 3 ── Two paths + random expansion */
    for (int i=0; i<lvl->num_rooms; i++) lvl->rooms[i].price=1;

    int path[MAX_ROOMS], plen;
    bfs_from(lvl, exit_r);
    plen=build_path(lvl, entrance_r, exit_r, path);
    for (int i=0; i<plen-1; i++) room_connect(lvl, path[i], path[i+1]);

    set_price(lvl, path, plen, lvl->rooms[entrance_r].distance);
    bfs_from(lvl, exit_r);
    int path2[MAX_ROOMS], plen2;
    plen2=build_path(lvl, entrance_r, exit_r, path2);
    for (int i=0; i<plen2-1; i++) room_connect(lvl, path2[i], path2[i+1]);

    for (int i=0; i<lvl->num_rooms; i++) lvl->rooms[i].price=1;
    bfs_from(lvl, exit_r);

    int conn_set[MAX_ROOMS], n_conn=0;
    #define add_set(idx) do { int _f=0; for(int _k=0;_k<n_conn;_k++) if(conn_set[_k]==(idx)){_f=1;break;} if(!_f&&n_conn<MAX_ROOMS) conn_set[n_conn++]=(idx); } while(0)
    add_set(entrance_r);
    for (int i=0;i<plen; i++) add_set(path [i]);
    for (int i=0;i<plen2;i++) add_set(path2[i]);

    int n_target=(int)(lvl->num_rooms*(0.5f+rng_float()*0.2f)), safety=0;
    while (n_conn<n_target && safety++<300) {
        int ci=conn_set[rng_range(0,n_conn-1)];
        Room *cr=&lvl->rooms[ci];
        if (cr->n_neighbours==0) continue;
        int ni=cr->neighbours[rng_range(0,cr->n_neighbours-1)];
        int found=0; for(int k=0;k<n_conn;k++) if(conn_set[k]==ni){found=1;break;}
        if (!found) { room_connect(lvl,ci,ni); add_set(ni); }
    }

    /* 4 ── Assign room types */
    assign_types(lvl, entrance_r, exit_r);

    /* 5 ── Compute door positions */
    compute_doors(lvl);

    /* 6 ── Paint: Standard rooms → full interior
                   Tunnel  rooms → L-shaped path only   (THE CORRIDORS)
                   Doors         → TILE_DOOR or TILE_EMPTY */
    paint_rooms(lvl);     /* STANDARD/ENTRANCE/EXIT: fill interior  */
    paint_tunnels(lvl);   /* TUNNEL: narrow path through room center */
    paint_doors(lvl);     /* door tiles last (don't get overwritten) */

    /* 7 ── Entrance / exit tiles */
    Room *er=&lvl->rooms[entrance_r], *xr=&lvl->rooms[exit_r];
    int ex=rng_range(er->left+1,er->right-1),  ey=rng_range(er->top+1,er->bottom-1);
    lvl->entrance=xy_to_idx(ex,ey); set_tile(lvl,ex,ey,TILE_ENTRANCE);
    int xx=rng_range(xr->left+1,xr->right-1),  xy_=rng_range(xr->top+1,xr->bottom-1);
    lvl->exit=xy_to_idx(xx,xy_);    set_tile(lvl,xx,xy_,TILE_EXIT);

    /* 8 ── Water + grass via Patch automaton */
    static uint8_t wp[LEVEL_SIZE], gp[LEVEL_SIZE];
    float wseed,gseed; int wgen,ggen;
    switch (theme) {
        case THEME_SEWERS: wseed=0.45f;wgen=5;gseed=0.40f;ggen=4; break;
        case THEME_PRISON: wseed=0.30f;wgen=4;gseed=0.00f;ggen=0; break;
        case THEME_CAVES:  wseed=0.50f;wgen=5;gseed=0.30f;ggen=3; break;
        case THEME_CITY:   wseed=0.20f;wgen=3;gseed=0.00f;ggen=0; break;
        default:           wseed=0.30f;wgen=4;gseed=0.00f;ggen=0; break;
    }
    if (wgen>0) { patch_generate(wseed,wgen,wp); paint_water(lvl,wp); }
    if (ggen>0) { patch_generate(gseed,ggen,gp); paint_grass(lvl,gp); }

    /* 9 ── Theme decoration */
    switch (theme) {
        case THEME_SEWERS: decorate_sewers(lvl); break;
        case THEME_PRISON: decorate_prison(lvl); break;
        default:           decorate_generic(lvl); break;
    }
    place_embers(lvl);
    return 0;
}

/* ── ASCII dump ─────────────────────────────────────────── */
void level_print(const Level *lvl) {
    const char *names[]={"Sewers","Prison","Caves","City","Halls"};
    printf("Theme:%s rooms:%d entrance:%d exit:%d\n",
           names[lvl->theme],lvl->num_rooms,lvl->entrance,lvl->exit);
    for (int y=0;y<LEVEL_H;y++) {
        for (int x=0;x<LEVEL_W;x++) {
            char c;
            switch(lvl->map[xy_to_idx(x,y)]) {
                case TILE_WALL:       c='#'; break;
                case TILE_WALL_DECO:  c='%'; break;
                case TILE_EMPTY:      c='.'; break;
                case TILE_EMPTY_DECO: c=','; break;
                case TILE_GRASS:      c='\'';break;
                case TILE_HIGH_GRASS: c='"'; break;
                case TILE_EMBERS:     c='~'; break;
                case TILE_DOOR:       c='+'; break;
                case TILE_ENTRANCE:   c='<'; break;
                case TILE_EXIT:       c='>'; break;
                case TILE_WATER:      c='='; break;
                case TILE_SIGN:       c='?'; break;
                default:              c=' '; break;
            }
            putchar(c);
        }
        putchar('\n');
    }
}
