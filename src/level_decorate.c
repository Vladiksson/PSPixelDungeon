#include "level_decorate.h"

/* ══════════════════════════════════════════════════════════
   DECORATION  —  SewerLevel.decorate() etc.
   ══════════════════════════════════════════════════════════ */
static void place_sign(Level *lvl){
    Point ep=idx_to_xy(lvl->entrance);
    for (int i=0; i<lvl->num_rooms; i++) {
        if (!room_inside(&lvl->rooms[i], ep.x, ep.y)) continue;
        for (int t=0; t<40; t++) {
            Room *r=&lvl->rooms[i];
            int rx=rng_range(r->left+1, r->right-1);
            int ry=rng_range(r->top +1, r->bottom-1);
            int idx=xy_to_idx(rx,ry);
            uint8_t tile = lvl->map[idx];

            if (idx != lvl->entrance && (gTileFlags[tile] & TF_PASSABLE)){
                lvl->map[idx] = TILE_SIGN;
                return;
            }
        }
        return;
    }
}
void decorate_sewers(Level *lvl) {
    for (int i=0; i<LEVEL_W; i++) {
        if (lvl->map[i]==TILE_WALL && lvl->map[i+LEVEL_W]==TILE_WATER && rng_int(4)==0)
            lvl->map[i]=TILE_WALL_DECO;
    }
    for (int i=LEVEL_W; i<LEVEL_SIZE-LEVEL_W; i++) {
        if (lvl->map[i]==TILE_WALL && lvl->map[i-LEVEL_W]==TILE_WALL
            && lvl->map[i+LEVEL_W]==TILE_WATER && rng_int(2)==0)
            lvl->map[i]=TILE_WALL_DECO;
    }
    for (int i=LEVEL_W+1; i<LEVEL_SIZE-LEVEL_W-1; i++) {
        if (lvl->map[i]==TILE_EMPTY) {
            int cnt=(lvl->map[i+1]==TILE_WALL)+(lvl->map[i-1]==TILE_WALL)
                   +(lvl->map[i+LEVEL_W]==TILE_WALL)+(lvl->map[i-LEVEL_W]==TILE_WALL);
            if (rng_int(16)<cnt*cnt) lvl->map[i]=TILE_EMPTY_DECO;
        }
    }
    place_sign(lvl);   
}
void decorate_prison(Level *lvl) {
    for (int i=LEVEL_W+1; i<LEVEL_SIZE-LEVEL_W-1; i++)
        if (lvl->map[i]==TILE_EMPTY){
            float chance = 0.05f;
            if (lvl->map[i+1]==TILE_WALL && lvl->map[i+LEVEL_W]==TILE_WALL){
                chance += 0.2f;
            }
            if (lvl->map[i-1]==TILE_WALL && lvl->map[i+LEVEL_W]==TILE_WALL){
                chance += 0.2f;
            }
            if (lvl->map[i+1]==TILE_WALL && lvl->map[i-LEVEL_W]==TILE_WALL){
                chance += 0.2f;
            }
            if (lvl->map[i-1]==TILE_WALL && lvl->map[i-LEVEL_W]==TILE_WALL){
                chance += 0.2f;
            }
            if (rng_float() < chance) {
                lvl->map[i] = TILE_EMPTY_DECO;
            }
        }
    for (int i=0; i<LEVEL_W; i++)
        if (lvl->map[i]==TILE_WALL &&
            (lvl->map[i+LEVEL_W]==TILE_EMPTY || lvl->map[i+LEVEL_W]==TILE_EMPTY_SP) && rng_int(6)==0)
            lvl->map[i]=TILE_WALL_DECO;
    for (int i=LEVEL_W; i<LEVEL_SIZE-LEVEL_W; i++)
        if (lvl->map[i]==TILE_WALL &&
            (lvl->map[i+LEVEL_W]==TILE_EMPTY || lvl->map[i+LEVEL_W]==TILE_EMPTY_SP) && rng_int(3)==0)
            lvl->map[i]=TILE_WALL_DECO;   
    place_sign(lvl);
}
void decorate_caves(Level *lvl) {
    for (int i = 0; i < lvl->num_rooms; i++) {
        Room *room = &lvl->rooms[i];
        if (room->type != ROOM_STANDARD)
            continue;
        if (room_w(room) <= 3 || room_h(room) <= 3)
            continue;
        int s = (room_w(room)+1) * (room_h(room)+1);

        if (rng_int(s) > 8) {
            int idx = xy_to_idx(room->left+1, room->top+1);
            if (lvl->map[idx-1] == TILE_WALL &&
                lvl->map[idx-LEVEL_W] == TILE_WALL)
                lvl->map[idx] = TILE_WALL;
        }
        if (rng_int(s) > 8) {
            int idx = xy_to_idx(room->right-1, room->top+1);
            if (lvl->map[idx+1] == TILE_WALL &&
                lvl->map[idx-LEVEL_W] == TILE_WALL)
                lvl->map[idx] = TILE_WALL;
        }
        if (rng_int(s) > 8) {
            int idx = xy_to_idx(room->left+1, room->bottom-1);
            if (lvl->map[idx-1] == TILE_WALL &&
                lvl->map[idx+LEVEL_W] == TILE_WALL)
                lvl->map[idx] = TILE_WALL;
        }
        if (rng_int(s) > 8) {
            int idx = xy_to_idx(room->right-1, room->bottom-1);
            if (lvl->map[idx+1] == TILE_WALL &&
                lvl->map[idx+LEVEL_W] == TILE_WALL)
                lvl->map[idx] = TILE_WALL;
        }
        for (int j = 0; j < room->n_connections; j++) {

            Room *r = &lvl->rooms[room->connections[j].room];

            if ((r->type == ROOM_STANDARD || r->type == ROOM_TUNNEL) && rng_int(3) == 0) {

                if (room->connections[j].pos >= 0)
                    set_tile(lvl, room->connections[j].pos, TILE_EMPTY_DECO);
            }
        }
    }    
    for (int i=LEVEL_W+1; i<LEVEL_SIZE-LEVEL_W-1; i++){
        if (lvl->map[i]==TILE_EMPTY){
            int n = 0;
            if (lvl->map[i+1]==TILE_WALL){
                n++;
            }
            if (lvl->map[i-1]==TILE_WALL){
                n++;
            }
            if (lvl->map[i+LEVEL_W]==TILE_WALL){
                n++;
            }
            if (lvl->map[i-LEVEL_W]==TILE_WALL){
                n++;
            }
            if (rng_int(6) <= n) {
                lvl->map[i] = TILE_EMPTY_DECO;
            }
        }
    }    
    for (int i=0; i<LEVEL_SIZE; i++) {
        if (lvl->map[i]==TILE_WALL && rng_int(12)==0)
            lvl->map[i]=TILE_WALL_DECO;
    }
    place_sign(lvl);
    
    //if (level_is_boss_depth(depth+1)) return; TODO i don't wanna add depth in decorators, it needs different solution
    /* Chasms between adjacent standard rooms that are not connected */
    for (int i = 0; i < lvl->num_rooms; i++) {
        Room *r = &lvl->rooms[i];
        if (r->type != ROOM_STANDARD)
            continue;
        for (int j = 0; j < r->n_neighbours; j++) {
            int ni = r->neighbours[j];
            if (ni <= i)
                continue;
            Room *n = &lvl->rooms[ni];
            if (n->type != ROOM_STANDARD)
                continue;
            if (already_connected(lvl, i, ni))
                continue;
            /* Rooms share a vertical wall */
            if (r->right == n->left || n->right == r->left) {
                int wallx = (r->right == n->left) ? r->right : n->right;
                int y0 = (r->top > n->top) ? r->top : n->top;
                int y1 = (r->bottom < n->bottom) ? r->bottom : n->bottom;
                if (y1 - y0 >= 5) {
                    for (int y = y0 + 2; y <= y1 - 2; y++)
                        set_tile(lvl, xy_to_idx(wallx, y), TILE_CHASM);
                }
            }
            /* Rooms share a horizontal wall */
            else if (r->bottom == n->top || n->bottom == r->top) {
                int wally = (r->bottom == n->top) ? r->bottom : n->bottom;
                int x0 = (r->left > n->left) ? r->left : n->left;
                int x1 = (r->right < n->right) ? r->right : n->right;
                if (x1 - x0 >= 5) {
                    for (int x = x0 + 2; x <= x1 - 2; x++)
                        set_tile(lvl, xy_to_idx(x, wally), TILE_CHASM);
                }
            }
        }
    }
}    
void decorate_city(Level *lvl) {
    for (int i=0; i<LEVEL_SIZE; i++) {
        if (lvl->map[i]==TILE_EMPTY && rng_int(10)==0){
            lvl->map[i]=TILE_EMPTY_DECO;
        } else if (lvl->map[i]==TILE_WALL && rng_int(8)==0){
            lvl->map[i]=TILE_WALL_DECO;
        }
    }
    place_sign(lvl);
}
void decorate_halls(Level *lvl) {
    for (int i = LEVEL_W + 1; i < LEVEL_SIZE - LEVEL_W - 1; i++) {

        if (lvl->map[i] == TILE_EMPTY) {

            int count = 0;

            if (gTileFlags[lvl->map[i - LEVEL_W - 1]] & TF_PASSABLE) count++;
            if (gTileFlags[lvl->map[i - LEVEL_W]]     & TF_PASSABLE) count++;
            if (gTileFlags[lvl->map[i - LEVEL_W + 1]] & TF_PASSABLE) count++;
            if (gTileFlags[lvl->map[i - 1]]           & TF_PASSABLE) count++;
            if (gTileFlags[lvl->map[i + 1]]           & TF_PASSABLE) count++;
            if (gTileFlags[lvl->map[i + LEVEL_W - 1]] & TF_PASSABLE) count++;
            if (gTileFlags[lvl->map[i + LEVEL_W]]     & TF_PASSABLE) count++;
            if (gTileFlags[lvl->map[i + LEVEL_W + 1]] & TF_PASSABLE) count++;

            if (rng_int(80) < count) {
                lvl->map[i] = TILE_EMPTY_DECO;
            }

        } else if (lvl->map[i] == TILE_WALL &&
                   lvl->map[i - 1] != TILE_WALL_DECO &&
                   lvl->map[i - LEVEL_W] != TILE_WALL_DECO &&
                   rng_int(20) == 0) {

            lvl->map[i] = TILE_WALL_DECO;
        }
    }
    place_sign(lvl);
}

void decorate_generic(Level *lvl) {
    for (int i=0; i<LEVEL_SIZE; i++)
        if (lvl->map[i]==TILE_WALL && rng_int(20)==0) lvl->map[i]=TILE_WALL_DECO;
}

/* ══════════════════════════════════════════════════════════
   BOSS DECORATION  —  XBossLevel.decorate() etc.
   ══════════════════════════════════════════════════════════ */
void decorate_sewers_boss(Level *lvl, int exit_room_idx) {
    /* "Waterfall" along the boss room's top wall: WALL_DECO on
       the wall itself, WATER on the row below it, except right
       above the exit tile, which stays open. */
    Room *ex = &lvl->rooms[exit_room_idx];
    int start = xy_to_idx(ex->left + 1, ex->top);
    int end = start + room_w(ex) - 1;
    for (int i = start; i < end; i++) {
        if (i != lvl->exit) {
            lvl->map[i] = TILE_WALL_DECO;
            lvl->map[i + LEVEL_W] = TILE_WATER;
        } else {
            lvl->map[i + LEVEL_W] = TILE_EMPTY;
        }
    }
    place_sign(lvl);
}

void decorate_prison_boss(Level *lvl, int exit_room_idx) {
    for (int i = LEVEL_W + 1; i < LEVEL_SIZE - LEVEL_W - 1; i++) {
        if (lvl->map[i] == TILE_EMPTY) {
            float chance = 0.15f;
            if (lvl->map[i+1]==TILE_WALL && lvl->map[i+LEVEL_W]==TILE_WALL) chance += 0.2f;
            if (lvl->map[i-1]==TILE_WALL && lvl->map[i+LEVEL_W]==TILE_WALL) chance += 0.2f;
            if (lvl->map[i+1]==TILE_WALL && lvl->map[i-LEVEL_W]==TILE_WALL) chance += 0.2f;
            if (lvl->map[i-1]==TILE_WALL && lvl->map[i-LEVEL_W]==TILE_WALL) chance += 0.2f;
            if (rng_float() < chance) lvl->map[i] = TILE_EMPTY_DECO;
        }
    }
    for (int i = 0; i < LEVEL_W; i++)
        if (lvl->map[i] == TILE_WALL &&
            (lvl->map[i+LEVEL_W] == TILE_EMPTY || lvl->map[i+LEVEL_W] == TILE_EMPTY_SP) &&
            rng_int(4) == 0)
            lvl->map[i] = TILE_WALL_DECO;
    for (int i = LEVEL_W; i < LEVEL_SIZE - LEVEL_W; i++)
        if (lvl->map[i] == TILE_WALL && lvl->map[i-LEVEL_W] == TILE_WALL &&
            (lvl->map[i+LEVEL_W] == TILE_EMPTY || lvl->map[i+LEVEL_W] == TILE_EMPTY_SP) &&
            rng_int(2) == 0)
            lvl->map[i] = TILE_WALL_DECO;

    place_sign(lvl);

    /* Boss room's one door gets upgraded from the plain DOOR that
       paint_doors() already placed there to a LOCKED_DOOR, and its
       floor becomes an (currently inert, no trigger yet) trap bed -
       see the deferred-mechanics note at the top of level_boss.c. */
    Room *ex = &lvl->rooms[exit_room_idx];

    set_tile(lvl, ex->connections[0].pos, TILE_LOCKED_DOOR);
    fill_rect(lvl, ex->left + 2, ex->top + 2, room_w(ex) - 3, room_h(ex) - 3, TILE_INACTIVE_TRAP);
}

void decorate_caves_boss(Level *lvl) {
    for (int i = LEVEL_W + 1; i < LEVEL_SIZE - LEVEL_W; i++) {
        if (lvl->map[i] == TILE_EMPTY) {
            int n = 0;
            if (lvl->map[i+1] == TILE_WALL) n++;
            if (lvl->map[i-1] == TILE_WALL) n++;
            if (lvl->map[i+LEVEL_W] == TILE_WALL) n++;
            if (lvl->map[i-LEVEL_W] == TILE_WALL) n++;
            if (rng_int(8) <= n) lvl->map[i] = TILE_EMPTY_DECO;
        }
    }
    for (int i = 0; i < LEVEL_SIZE; i++)
        if (lvl->map[i] == TILE_WALL && rng_int(8) == 0)
            lvl->map[i] = TILE_WALL_DECO;

    int sx, sy, sidx;
    do {
        sx = rng_range(CAVES_ARENA_LEFT, CAVES_ARENA_RIGHT);
        sy = rng_range(CAVES_ARENA_TOP, CAVES_ARENA_BOTTOM);
        sidx = xy_to_idx(sx, sy);
    } while (sidx == lvl->entrance);
    set_tile(lvl, sidx, TILE_SIGN);
}

void decorate_city_boss(Level *lvl, int arena_door_idx) {
    for (int i = 0; i < LEVEL_SIZE; i++) {
        if (lvl->map[i] == TILE_EMPTY && rng_int(10) == 0)
            lvl->map[i] = TILE_EMPTY_DECO;
        else if (lvl->map[i] == TILE_WALL && rng_int(8) == 0)
            lvl->map[i] = TILE_WALL_DECO;
    }
    /* One row below and one column right of the arena door - a
       fixed offset in the original, not a search. */
    lvl->map[arena_door_idx + LEVEL_W + 1] = TILE_SIGN;
}

void decorate_halls_boss(Level *lvl) {
    /* No wall decoration, no sign here - the original really is
       this much sparser than the other four. */
    for (int i = 0; i < LEVEL_SIZE; i++)
        if (lvl->map[i] == TILE_EMPTY && rng_int(10) == 0)
            lvl->map[i] = TILE_EMPTY_DECO;
}


/* ══════════════════════════════════════════════════════════
   EPILOGUE DECORATION  —  LastShopLevel/LastLevel .decorate()
   ══════════════════════════════════════════════════════════ */
void decorate_last_shop(Level *lvl, int quest_completed) {
    for (int i = 0; i < LEVEL_SIZE; i++) {
        if (lvl->map[i] == TILE_EMPTY && rng_int(10) == 0) {
            lvl->map[i] = TILE_EMPTY_DECO;
        } else if (lvl->map[i] == TILE_WALL && rng_int(8) == 0) {
            lvl->map[i] = TILE_WALL_DECO;
        } else if (lvl->map[i] == TILE_SECRET_DOOR) {
            lvl->map[i] = TILE_DOOR;
        }
    }
    if (quest_completed) place_sign(lvl);
}
 
void decorate_last(Level *lvl) {
    for (int i = 0; i < LEVEL_SIZE; i++)
        if (lvl->map[i] == TILE_EMPTY && rng_int(10) == 0)
            lvl->map[i] = TILE_EMPTY_DECO;
}