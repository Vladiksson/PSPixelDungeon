#include "level_rooms.h"
#include "item_spawn.h"

/* ══════════════════════════════════════════════════════════
   ROOM TYPE ASSIGNMENT  —  RegularLevel.assignRoomType(), etc
   ══════════════════════════════════════════════════════════ */
int entrance_has_large_neighbour(Level *lvl, int entrance_idx) {
    Room *e = &lvl->rooms[entrance_idx];
    for (int i = 0; i < e->n_neighbours; i++) {
        Room *r = &lvl->rooms[e->neighbours[i]];
        if (room_w(r) >= 6 && room_h(r) >= 6)
            return 1;
    }
    return 0;
}

void place_special_near_entrance(Level *lvl, int entrance_idx, int depth, int blacksmith_depth){
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

        int w = room_w(r);
        int h = room_h(r);

        if (w < 6 || h < 6)
            continue;

        if (r->n_connections == 0) {
            room_connect(lvl, entrance_idx, ni);
        }

        int area = w * h;

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

static void shuffle_specials(RoomType *arr, int count){ //redo duplication of code
    for (int i = count - 1; i > 0; i--) {
        int j = rng_int(i + 1);
        RoomType t = arr[i];
        arr[i] = arr[j];
        arr[j] = t;
    }
}


void init_special_rooms(LevelGenState *gen, int depth){
    memset(gen, 0, sizeof(*gen));

    gen->num_specials = sizeof(default_specials) / sizeof(default_specials[0]);

    memcpy(gen->specials, default_specials, sizeof(default_specials));
    shuffle_specials(gen->specials, gen->num_specials);

    if (level_is_boss_depth(depth+1))
               special_remove(gen->specials, &gen->num_specials, ROOM_WEAK_FLOOR);
}

void assign_types(Level *lvl, LevelGenState *gen, int entrance_idx, int exit_idx, DungeonTheme theme, int depth) {
    lvl->rooms[entrance_idx].type = ROOM_ENTRANCE;
    lvl->rooms[exit_idx].type = ROOM_EXIT;
    int special_rooms = 0;
    for (int i = 0; i < lvl->num_rooms; i++) {
        Room *r = &lvl->rooms[i];
        if (r->type != ROOM_NULL)
            continue;
        if (r->n_connections != 1)
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

                if (gRoomFlags[n->type] & ROOMF_SPECIAL)
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
        int nc = r->n_connections;
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
        for (int j = 0; j < r->n_connections; j++) {
            RoomConnection *c = &r->connections[j];
            Room *n = &lvl->rooms[c->room];

            c->type = get_door_type(r, n);
        }
    }
}

/* ══════════════════════════════════════════════════════════
   DOOR POSITION CALCULATION  —  RegularLevel.placeDoors()
   ══════════════════════════════════════════════════════════ */
void compute_doors(Level *lvl) {
    for (int i = 0; i < lvl->num_rooms; i++) {
        Room *r = &lvl->rooms[i];

        for (int j = 0; j < r->n_connections; j++) {
            RoomConnection *c = &r->connections[j];

            if (c->pos != -1)
                continue;

            Room *n = &lvl->rooms[c->room];

            int ix0 = r->left   > n->left   ? r->left   : n->left;
            int ix1 = r->right  < n->right  ? r->right  : n->right;
            int iy0 = r->top    > n->top    ? r->top    : n->top;
            int iy1 = r->bottom < n->bottom ? r->bottom : n->bottom;

            int iw = ix1 - ix0;
            int dx, dy;

            if (iw == 0) {
                dx = ix0;
                dy = rng_range(iy0 + 1, iy1 - 1);
            } else {
                dx = rng_range(ix0 + 1, ix1 - 1);
                dy = iy0;
            }

            int pos = xy_to_idx(dx, dy);
            c->pos = pos;

            /* Same door from the other side */
            for (int k = 0; k < n->n_connections; k++) {
                RoomConnection *nc = &n->connections[k];

                if (nc->room == i) {
                    nc->pos = pos;
                    break;
                }
            }
        }
    }
}