#include "level.h"
#include "level_utils.h"
#include "level_paint.h"
#include "level_decorate.h"
#include "level_boss.h"
#include "level_last.h"
#include <stdio.h>
#include <time.h>

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

/* ══════════════════════════════════════════════════════════
   ROOM TYPE ASSIGNMENT  —  RegularLevel.assignRoomType(), etc
   ══════════════════════════════════════════════════════════ */
static int entrance_has_special_room(Level *lvl, int entrance_idx) {
    Room *e = &lvl->rooms[entrance_idx];
    for (int i = 0; i < e->n_neighbours; i++) {
        Room *r = &lvl->rooms[e->neighbours[i]];
        if (room_w(r) >= 5 && room_h(r) >= 5)
            return 1;
    }
    return 0;
}


static int shop_on_level(int depth){
    return depth == 5 || depth == 10 || depth == 15;
}

static void place_special_near_entrance(Level *lvl, int entrance_idx, int depth, int blacksmith_depth){
    RoomType type = ROOM_NULL;
    if (depth == blacksmith_depth)
        type = ROOM_BLACKSMITH;
    if (shop_on_level(depth))
        type = ROOM_SHOP;
    if (type == ROOM_NULL)
        return;
    Room *e = &lvl->rooms[entrance_idx];
    int best = -1;
    int best_area = 0;
    for (int i = 0; i < e->n_neighbours; i++) {

        int ni = e->neighbours[i];
        Room *r = &lvl->rooms[ni];

        if (r->type != ROOM_NULL)
            continue;
        if (r->n_connected == 0)
            continue;

        int w = room_w(r);
        int h = room_h(r);

        if (w < 5 || h < 5)
            continue;

        int area = w*h;

        if (area > best_area) {
            best_area = area;
            best = ni;
        }
    }
    if (best >= 0) lvl->rooms[best].type = type;
}

const uint16_t gRoomFlags[ROOM_COUNT] = {
    [ROOM_NULL]      = 0,
    [ROOM_STANDARD]  = 0,
    [ROOM_ENTRANCE]  = 0,
    [ROOM_EXIT]      = 0,
    [ROOM_BOSS_EXIT] = 0,
    [ROOM_TUNNEL]    = ROOMF_TUNNEL,
    [ROOM_PASSAGE]   = ROOMF_TUNNEL,
    [ROOM_SHOP]      = 0,
    [ROOM_BLACKSMITH]= 0,
    [ROOM_TREASURY]  = ROOMF_SPECIAL | ROOMF_LOCKED_DOOR,
    [ROOM_ARMORY]    = ROOMF_SPECIAL | ROOMF_LOCKED_DOOR,
    [ROOM_LIBRARY]   = ROOMF_SPECIAL | ROOMF_LOCKED_DOOR,
    [ROOM_LABORATORY]= ROOMF_SPECIAL,
    [ROOM_VAULT]     = ROOMF_SPECIAL | ROOMF_LOCKED_DOOR,
    [ROOM_TRAPS]     = ROOMF_SPECIAL,
    [ROOM_STORAGE]   = ROOMF_SPECIAL | ROOMF_BARRICADE,
    [ROOM_MAGIC_WELL]= ROOMF_SPECIAL,
    [ROOM_GARDEN]    = ROOMF_SPECIAL,
    [ROOM_CRYPT]     = ROOMF_SPECIAL | ROOMF_LOCKED_DOOR,
    [ROOM_STATUE]    = ROOMF_SPECIAL | ROOMF_LOCKED_DOOR,
    [ROOM_POOL]      = ROOMF_SPECIAL,
    [ROOM_RAT_KING]  = ROOMF_SPECIAL | ROOMF_HIDDEN_DOOR,
    [ROOM_WEAK_FLOOR]= ROOMF_SPECIAL,
    [ROOM_PIT]       = ROOMF_SPECIAL | ROOMF_LOCKED_DOOR,
    [ROOM_ALTAR]     = ROOMF_SPECIAL | ROOMF_EMPTY_DOOR,
};

static const RoomType default_specials[] = { 
    ROOM_ARMORY,
    ROOM_WEAK_FLOOR,
    ROOM_MAGIC_WELL,
    ROOM_CRYPT,
    ROOM_POOL,
    ROOM_GARDEN,
    ROOM_LIBRARY,
    ROOM_TREASURY,
    ROOM_TRAPS,
    ROOM_STORAGE,
    ROOM_STATUE,
    ROOM_LABORATORY,
    ROOM_VAULT,
    ROOM_ALTAR
};

static int special_index(RoomType *specials, int count, RoomType t){
    for (int i = 0; i < count; i++){
        if (specials[i] == t)
            return i;
    }
    return -1;
}


static void special_remove(RoomType *specials, int *count, RoomType t){
    int idx = special_index(specials, *count, t);
    if (idx < 0)
        return;
    for (int i = idx; i < *count - 1; i++)
        specials[i] = specials[i + 1];
    (*count)--;
}

static void shuffle_specials(RoomType *arr, int count){
    for (int i = count - 1; i > 0; i--) {
        int j = rng_int(i + 1);
        RoomType t = arr[i];
        arr[i] = arr[j];
        arr[j] = t;
    }
}


static void init_special_rooms(LevelGenState *gen, int depth){
    memset(gen, 0, sizeof(*gen));

    gen->num_specials = sizeof(default_specials) / sizeof(default_specials[0]);

    memcpy(gen->specials, default_specials, sizeof(default_specials));
    shuffle_specials(gen->specials, gen->num_specials);

    if (level_is_boss_depth(depth+1))
               special_remove(gen->specials, &gen->num_specials, ROOM_WEAK_FLOOR);
}

static void assign_types(Level *lvl, LevelGenState *gen, int entrance_idx, int exit_idx, DungeonTheme theme, int depth) {
    lvl->rooms[entrance_idx].type = ROOM_ENTRANCE;
    lvl->rooms[exit_idx].type = ROOM_EXIT;
    int special_rooms = 0;
    for (int i = 0; i < lvl->num_rooms; i++) {
        Room *r = &lvl->rooms[i];
        if (r->type != ROOM_NULL)
            continue;
        if (r->n_connected != 1)
            continue;
        if (gen->num_specials > 0 &&
            room_w(r) > 3 &&
            room_h(r) > 3 &&
            rng_int(special_rooms * special_rooms + 2) == 0) {
            if (gen->pit_room_needed) {
                r->type = ROOM_PIT;
                gen->pit_room_needed = 0;

                special_remove(gen->specials, &gen->num_specials, ROOM_ARMORY);
                special_remove(gen->specials, &gen->num_specials, ROOM_CRYPT);
                special_remove(gen->specials, &gen->num_specials, ROOM_LABORATORY);
                special_remove(gen->specials, &gen->num_specials, ROOM_LIBRARY);
                special_remove(gen->specials, &gen->num_specials, ROOM_STATUE);
                special_remove(gen->specials, &gen->num_specials, ROOM_TREASURY);
                special_remove(gen->specials, &gen->num_specials, ROOM_VAULT);
                special_remove(gen->specials, &gen->num_specials, ROOM_WEAK_FLOOR);

            } else if ((depth + 1)% 5 == 2 && special_index(gen->specials, gen->num_specials, ROOM_LABORATORY) >= 0) {
                r->type = ROOM_LABORATORY;
            } else {

                int n = gen->num_specials;
                int a = rng_int(n);
                int b = rng_int(n);

                r->type = gen->specials[(a < b) ? a : b];
                if (r->type == ROOM_WEAK_FLOOR)
                    gen->weak_floor_created = 1;

                for (int tries = 0; tries < 20; tries++) { /*TEMPORARY FIX FOR ROOM_ALTAR*/
                    int a = rng_int(gen->num_specials);
                    int b = rng_int(gen->num_specials);
                    RoomType t = gen->specials[(a < b) ? a : b];
                    if (t == ROOM_ALTAR && (room_w(r) < 5 || room_h(r) < 5))
                        continue;
                    r->type = t;
                    break;
                }
            }
            special_remove(gen->specials, &gen->num_specials, r->type);
            special_rooms++;
                } else if (rng_int(2) == 0) {

            int candidates[MAX_NEIGHBOURS];
            int count = 0;

            for (int j = 0; j < r->n_neighbours; j++) {

                int ni = r->neighbours[j];
                Room *n = &lvl->rooms[ni];

                if (already_connected(lvl, i, ni))
                    continue;

                if (gRoomFlags[r->type] & ROOMF_SPECIAL)
                    continue;

                if (n->type == ROOM_PIT)
                    continue;

                candidates[count++] = ni;
            }

            if (count > 1) {
                int ni = candidates[rng_int(count)];
                room_connect(lvl, i, ni);
            }
        }
    }    
    int n_std = 0;
    for (int i=0; i<lvl->num_rooms; i++) {
        Room *r = &lvl->rooms[i];
        if (r->type != ROOM_NULL) continue;
        int nc = r->n_connected;
        if (nc == 0) continue;
        if (rng_int(nc*nc) == 0) { r->type = ROOM_STANDARD; n_std++; }
        else{
            r->type = (rng_int(3) == 0) ? ROOM_PASSAGE : ROOM_TUNNEL;
        }                       
    }
    for (int i=0; n_std<4 && i<lvl->num_rooms; i++)
        if (lvl->rooms[i].type == ROOM_TUNNEL || lvl->rooms[i].type == ROOM_PASSAGE) { lvl->rooms[i].type = ROOM_STANDARD; n_std++; }
}
/* ══════════════════════════════════════════════════════════
   DOOR TYPE ASSIGNMENT 
   ══════════════════════════════════════════════════════════ */
static DoorType special_door(RoomType type){
    if (gRoomFlags[type] & ROOMF_LOCKED_DOOR)
        return DOOR_LOCKED;
    if (gRoomFlags[type] & ROOMF_HIDDEN_DOOR)
        return DOOR_HIDDEN;
    if (gRoomFlags[type] & ROOMF_BARRICADE)
        return DOOR_BARRICADE;
    if (gRoomFlags[type] & ROOMF_EMPTY_DOOR)
        return DOOR_EMPTY;
    return DOOR_REGULAR;
}

static DoorType get_door_type(Room *a, Room *b){
    if ((gRoomFlags[a->type] & ROOMF_TUNNEL) && (gRoomFlags[b->type] & ROOMF_TUNNEL))
        return DOOR_TUNNEL;
    DoorType ta = special_door(a->type);
    if (ta != DOOR_REGULAR)
        return ta;
    DoorType tb = special_door(b->type);
    if (tb != DOOR_REGULAR)
        return tb;
    if (a->type == ROOM_LABORATORY || b->type == ROOM_LABORATORY)
        return rng_int(3) == 0 ? DOOR_HIDDEN : DOOR_LOCKED;
    return DOOR_REGULAR;
}

void assign_door_types(Level *lvl){
    for (int i = 0; i < lvl->num_rooms; i++) {
        Room *r = &lvl->rooms[i];
        for (int j = 0; j < r->n_connected; j++) {
            Room *n = &lvl->rooms[r->connected[j]];
            r->door_type[j] = get_door_type(r,n);
        }
    }
}

/* ══════════════════════════════════════════════════════════
   DOOR POSITION CALCULATION  —  RegularLevel.placeDoors()
   ══════════════════════════════════════════════════════════ */
void compute_doors(Level *lvl) {
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
            else         { dx=rng_range(ix0+1, ix1-1);    dy=iy0; }
            r->door_x[j]=dx; r->door_y[j]=dy;
            for (int k=0; k<n->n_connected; k++)
                if (n->connected[k]==i) { n->door_x[k]=dx; n->door_y[k]=dy; break; }
        }
    }
}

/* ══════════════════════════════════════════════════════════
   PUBLIC: level_generate
   ══════════════════════════════════════════════════════════ */
#define LAST_SHOP_DEPTH 20
#define FINAL_LEVEL_DEPTH 25

int level_generate(Level *lvl, uint32_t seed, DungeonTheme theme, int depth, int blacksmith_depth) {
    if (level_is_boss_depth(depth))
        return level_generate_boss(lvl, seed, theme, depth);
    if (depth == LAST_SHOP_DEPTH)
        return level_generate_last_shop(lvl, seed, depth, 1);
    if (depth == FINAL_LEVEL_DEPTH)
        return level_generate_last(lvl, seed, depth);

    rng_state = seed ? seed : (uint32_t)time(NULL);
    memset(lvl, 0, sizeof(*lvl));
    
    LevelGenState gen;
    init_special_rooms(&gen, depth);

    lvl->theme = theme;
    memset(lvl->map, TILE_WALL, sizeof(lvl->map));

    lvl->feeling = FEELING_NONE;
    if (depth > 0) {
        switch (rng_int(10)) {
        case 0:
            if (!level_is_boss_depth(depth + 1))
                lvl->feeling = FEELING_CHASM;
            break;
        case 1:
            lvl->feeling = FEELING_WATER;
            break;
        case 2:
            lvl->feeling = FEELING_GRASS;
            break;
        }
    }  
    /* 1 BSP split + neighbour graph */
    int retry=0;
    do {
        lvl->num_rooms=0;
        bsp_split(lvl, 0, 0, LEVEL_W-1, LEVEL_H-1);
        if (++retry > 10) return -1;
    } while (lvl->num_rooms < 8);
    build_neighbours(lvl);

    /* 2 Entrance / exit (far apart, width/height ≥ 4) */
    int entrance_r=-1, exit_r=-1, attempts=0;
    int min_dist=isqrt(lvl->num_rooms);
    do {
        do { entrance_r=rng_range(0,lvl->num_rooms-1); }
        while (room_w(&lvl->rooms[entrance_r])<4 || room_h(&lvl->rooms[entrance_r])<4 ||
            !entrance_has_special_room(lvl, entrance_r));
        do { exit_r=rng_range(0,lvl->num_rooms-1); }
        while (exit_r==entrance_r || room_w(&lvl->rooms[exit_r])<4 || room_h(&lvl->rooms[exit_r])<4);
        build_distance_map(lvl, exit_r);
        if (lvl->rooms[entrance_r].distance >= min_dist) break;
    } while (++attempts < 20);
    if (entrance_r<0 || exit_r<0) return -1;

    /* 3 Two paths + random expansion */
    for (int i=0; i<lvl->num_rooms; i++) lvl->rooms[i].price=1;

    int path[MAX_ROOMS], plen;
    build_distance_map(lvl, exit_r);
    plen=build_path(lvl, entrance_r, exit_r, path);
    for (int i=0; i<plen-1; i++) room_connect(lvl, path[i], path[i+1]);

    set_price(lvl, path, plen, lvl->rooms[entrance_r].distance);
    build_distance_map(lvl, exit_r);
    int path2[MAX_ROOMS], plen2;
    plen2=build_path(lvl, entrance_r, exit_r, path2);
    for (int i=0; i<plen2-1; i++) room_connect(lvl, path2[i], path2[i+1]);

    for (int i=0; i<lvl->num_rooms; i++) lvl->rooms[i].price=1;
    build_distance_map(lvl, exit_r);

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

    /* 3.5*/
    place_special_near_entrance(lvl, entrance_r, depth, blacksmith_depth);
    /* 4 Assign room types */
    assign_types(lvl, &gen, entrance_r, exit_r, theme, depth);

    /* 5 Compute door positions */
    compute_doors(lvl);

    /* 6  Paint ROOMS*/
    paint_rooms(lvl, depth);     
    paint_tunnels(lvl);   

    if (lvl->feeling == FEELING_CHASM)
        paint_chasms(lvl);

    /* 7 Assign door types*/
    assign_door_types(lvl);
    paint_doors(lvl);     
    
    /* 8  Water + grass via Patch automaton 
        + Theme decoration*/
    static uint8_t wp[LEVEL_SIZE], gp[LEVEL_SIZE];
    float wseed,gseed; int wgen,ggen;

    void (*decor)(Level *) = decorate_generic;
    switch (theme) {

    case THEME_SEWERS:
        wseed = (lvl->feeling == FEELING_WATER) ? 0.60f : 0.45f; wgen = 5;
        gseed = (lvl->feeling == FEELING_GRASS) ? 0.60f : 0.40f; ggen = 4;

        decor = decorate_sewers;
        break;

    case THEME_PRISON:
        wseed = (lvl->feeling == FEELING_WATER) ? 0.65f : 0.45f; wgen = 4;
        gseed = (lvl->feeling == FEELING_GRASS) ? 0.60f : 0.40f; ggen = 3;

        decor = decorate_prison;
        break;

    case THEME_CAVES:
        wseed = (lvl->feeling == FEELING_WATER) ? 0.60f : 0.45f; wgen = 6;
        gseed = (lvl->feeling == FEELING_GRASS) ? 0.55f : 0.35f; ggen = 3;

        decor = decorate_caves;
        break;

    case THEME_CITY:
        wseed = (lvl->feeling == FEELING_WATER) ? 0.65f : 0.45f; wgen = 4;
        gseed = (lvl->feeling == FEELING_GRASS) ? 0.60f : 0.40f; ggen = 3;

        decor = decorate_city;
        break;

    case THEME_HALLS:
        wseed = (lvl->feeling == FEELING_WATER) ? 0.55f : 0.40f; wgen = 6;
        gseed = (lvl->feeling == FEELING_GRASS) ? 0.55f : 0.30f; ggen = 3;

        decor = decorate_halls;
        break;

    default:
        wseed = 0.0f; wgen = 0;
        gseed = 0.0f; ggen = 0;
    
        decor = decorate_generic;
        break;
    }
    if (wgen>0) { patch_generate(wseed,wgen,wp); paint_water(lvl,wp); }
    if (ggen>0) { patch_generate(gseed,ggen,gp); paint_grass(lvl,gp); }
    
    decor(lvl);

    /* 9 Scatter loot  */
    level_spawn_items(lvl, depth);

    return 0;
}

int level_player_on_exit(const Level *lvl, int x, int y){
    return xy_to_idx(x, y) == lvl->exit;
}
int level_player_on_entrance(const Level *lvl, int x, int y){
    return xy_to_idx(x, y) == lvl->entrance;
}