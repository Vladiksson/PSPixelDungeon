#include "level_paint.h"

/* ══════════════════════════════════════════════════════════
    Room.center()
   ══════════════════════════════════════════════════════════ */
static void room_center(const Room *r, int *x, int *y)
{
    *x = (r->left + r->right) / 2;
    if ((r->right - r->left) & 1)
        *x += rng_int(2);

    *y = (r->top + r->bottom) / 2;
    if ((r->bottom - r->top) & 1)
        *y += rng_int(2);
}

/* ══════════════════════════════════════════════════════════
    Basic room
   ══════════════════════════════════════════════════════════ */
static void paint_room_base(Level *lvl, const Room *r, uint8_t floor){
    /* Step 1: Painter.fill(room, WALL) — reset whole room to wall width()+1 = room_w(r)+1 cols, height()+1 = room_h(r)+1 rows */
    fill_rect(lvl, r->left,     r->top,     room_w(r) + 1, room_h(r) + 1, TILE_WALL);
    /* Step 2: Painter.fill(room, 1, EMPTY) — carve interior width()-1 = room_w(r)-1, height()-1 = room_h(r)-1 */
    fill_rect(lvl, r->left + 1, r->top + 1, room_w(r) - 1, room_h(r) - 1, floor);
}
/* ══════════════════════════════════════════════════════════
   SHOOP PAINTING
   ══════════════════════════════════════════════════════════ */
//TODO TRANSFER THIS SHOP-STUFF SOMEWHERE
static int perimeter_length(Room *r){
    int w = room_w(r)-2;
    int h = room_h(r)-2;
    return w*2 + h*2;
}
static Point shop_p2xy(Room *r, int p){
    int w = room_w(r)-2;
    int h = room_h(r)-2;
    Point out;
    if (p < w) {
        out.x = r->left + 1 + p;
        out.y = r->top + 1;
    } else if (p < w+h) {
        out.x = r->right - 1;
        out.y = r->top + 1 + (p-w);
    } else if (p < w*2+h) {
        out.x = r->right - 1 - (p-w-h);
        out.y = r->bottom - 1;
    } else {
        out.x = r->left + 1;
        out.y = r->bottom - 1 - (p-w*2-h);
    }
    return out;
}

static void place_shop_items(Level *lvl, Room *room){
    int per = perimeter_length(room);
    int count = 0; 
    count += 15;
    int pos = rng_range(0, per-1);

    for (int i=0; i<count; i++) {
        Point p = shop_p2xy(room, (pos+i)%per);
        int idx = xy_to_idx(p.x,p.y);
        if (lvl->map[idx] == TILE_EMPTY_SP) {
            /* TODO:
               spawn_item_for_shop(depth)
            */
            lvl->map[idx] = TILE_SIGN;
        }
    }
}

static void place_shopkeeper(Level *lvl, Room *room){
    for (int tries = 0; tries < 200; tries++) {
        int x = rng_range(room->left+1, room->right-1);
        int y = rng_range(room->top+1, room->bottom-1);

        int idx = xy_to_idx(x,y);

        if (lvl->map[idx] == TILE_EMPTY_SP) {

            /* TODO ADD MOB */
            // mob_spawn(MOB_SHOPKEEPER,x,y);

            return;
        }
    }
}

static void paint_shop(Level *lvl, Room *room){
    paint_room_base(lvl, room, TILE_EMPTY_SP); 
    place_shop_items(lvl, room);
    place_shopkeeper(lvl, room); 

    for (int i=0; i<room->n_connected; i++) {
        room->door_type[i] = DOOR_REGULAR;
    }
}
/* ══════════════════════════════════════════════════════════
   ENTRANCE/EXIT ROOM PAINTING
   ══════════════════════════════════════════════════════════ */
static void paint_entrance(Level *lvl, Room *r){
    paint_room_base(lvl, r, TILE_EMPTY);
    //ORIGINALY WAS int x = rng_range(r->left + 1, r->right - 1), y = rng_range(r->top + 1, r->bottom - 1);
    int x,y;
    room_center(r, &x, &y);

    lvl->entrance = xy_to_idx(x, y);
    set_tile(lvl, x, y, TILE_ENTRANCE);
}

static void paint_exit(Level *lvl, Room *r){
    paint_room_base(lvl, r, TILE_EMPTY);
    //ORIGINALY WAS int x = rng_range(r->left + 1, r->right - 1), y = rng_range(r->top + 1, r->bottom - 1);
    int x,y;
    room_center(r, &x, &y);

    lvl->exit = xy_to_idx(x, y);
    set_tile(lvl, x, y, TILE_EXIT);
}
/* ══════════════════════════════════════════════════════════
   ROOM PAINTING  —  StandardPainter.paint(), etc.
   ══════════════════════════════════════════════════════════ */
static void draw_inside(Level *lvl, Room *r, int dx, int dy, int len, uint8_t tile){
    int sx = 0;
    int sy = 0;

    if (dx == r->left)
        sx = 1;
    else if (dx == r->right)
        sx = -1;
    else if (dy == r->top)
        sy = 1;
    else if (dy == r->bottom)
        sy = -1;

    int x = dx;
    int y = dy;

    for (int i = 0; i < len; i++) {
        x += sx;
        y += sy;

        if (!in_bounds(x,y))
            break;
        set_tile(lvl,x,y,tile);
    }
}

static void paint_burned(Level *lvl, Room *r){
    for (int y=r->top+1; y<r->bottom; y++) {
        for (int x=r->left+1; x<r->right; x++) {

            uint8_t t = TILE_EMBERS;

            switch(rng_int(5)) {

                case 0:
                    t = TILE_EMPTY;
                    break;

                case 1:
                    t = TILE_FIRE_TRAP;
                    break;
                case 2:
                    t = TILE_SECRET_FIRE_TRAP;
                    break;
                case 3:
                    t = TILE_INACTIVE_TRAP;
                    break;
                default:
                    break;
            }
            set_tile(lvl,x,y,t);
        }
    }
}

static void paint_striped(Level *lvl, Room *r){
    fill_rect(lvl, r->left + 1, r->top + 1, room_w(r) - 1, room_h(r) - 1, TILE_EMPTY_SP);

    if (room_w(r) > room_h(r)) {
        /* vertical stripes */
        for (int x = r->left + 2; x < r->right; x += 2) {
            fill_rect(lvl, x, r->top + 1, 1, room_h(r) - 1, TILE_HIGH_GRASS);
        }
    } else {
        /* horizontal stripes */
        for (int y = r->top + 2; y < r->bottom; y += 2) {
            fill_rect(lvl, r->left + 1, y, room_w(r) - 1, 1, TILE_HIGH_GRASS);
        }
    }
}

static void paint_study(Level *lvl, Room *r){
    fill_rect(lvl, r->left + 1, r->top + 1, room_w(r) - 1, room_h(r) - 1, TILE_BOOKSHELF);
    fill_rect(lvl, r->left + 2, r->top + 2, room_w(r) - 3, room_h(r) - 3, TILE_EMPTY_SP);
    /* door openings */
    for (int i = 0; i < r->n_connected; i++) {

        int x = r->door_x[i];
        int y = r->door_y[i];

        if (x < 0 || y < 0)
            continue;

        if (x == r->left) {
            set_tile(lvl, x + 1, y, TILE_EMPTY);
        } else if (x == r->right) {
            set_tile(lvl, x - 1, y, TILE_EMPTY);
        } else if (y == r->top) {
            set_tile(lvl, x, y + 1, TILE_EMPTY);
        } else if (y == r->bottom) {
            set_tile(lvl, x, y - 1, TILE_EMPTY);
        }
    }
    /* room.center() */
    int cx, cy;
    room_center(r, &cx, &cy);
    set_tile(lvl, cx, cy, TILE_PEDESTAL);
}

static void paint_graveyard(Level *lvl, Room *r){
    /* fill room interior with grass */
    fill_rect(lvl, r->left + 1, r->top + 1, room_w(r) - 1, room_h(r) - 1, TILE_GRASS);

    int w = room_w(r) - 1;
    int h = room_h(r) - 1;
    int n_graves = (w > h ? w : h) / 2;

    if (n_graves <= 0)
        return;
    /* random grave that gets special loot */
    //TODO for future int index = rng_int(n_graves);
    int shift = rng_int(2);

    for (int i = 0; i < n_graves; i++) {
        int x, y;
        if (w > h) {
            /* horizontal room */
            x = r->left + 1 + shift + i * 2;
            y = r->top + 2 + rng_int(h - 2);
        } else {
            /* vertical room */
            x = r->left + 2 + rng_int(w - 2);
            y = r->top + 1 + shift + i * 2;
        }

        if (!in_bounds(x,y))
            continue;

        int idx = xy_to_idx(x,y);
        /*
           TODO:
           original:
             level.drop(
                i == index ? Generator.random() : new Gold(),
                pos
             ).type = Heap.Type.TOMB;

           Here later:
             spawn_item(lvl, idx, ITEM_GOLD);
             or
             spawn_random_item(...)
        */
        /* temporary visual marker */
        lvl->map[idx] = TILE_STATUE_SP;
    }
}

static void paint_bridge(Level *lvl, Room *r){
    uint8_t base = TILE_WATER;

    /*TODO change in future from original:
    !boss && !nextBoss && Random.Int(3)==0 ? CHASM : WATER*/

    if (rng_int(3)==0)
        base = TILE_CHASM;
    fill_rect(lvl, r->left + 1, r->top + 1, room_w(r)-1, room_h(r)-1, base);

    if (r->n_connected != 2)
        return;

    int d1x = r->door_x[0];
    int d1y = r->door_y[0];

    int d2x = r->door_x[1];
    int d2y = r->door_y[1];

    if ((d1x == r->left && d2x == r->right) || (d1x == r->right && d2x == r->left)) {
        int s = room_w(r)/2;

        draw_inside(lvl,r,d1x,d1y,s,TILE_EMPTY_SP);
        draw_inside(lvl,r,d2x,d2y,s,TILE_EMPTY_SP);

        int cx,cy;
        room_center(r,&cx,&cy);

        int y0 = d1y < d2y ? d1y : d2y;
        int h  = abs(d1y-d2y)+1;

        fill_rect(lvl, cx, y0, 1, h,TILE_EMPTY_SP);
    }
    else if ((d1y == r->top && d2y == r->bottom) || (d1y == r->bottom && d2y == r->top)) {
        int s = room_h(r)/2;

        draw_inside(lvl,r,d1x,d1y,s,TILE_EMPTY_SP);
        draw_inside(lvl,r,d2x,d2y,s,TILE_EMPTY_SP);

        int cx,cy;
        room_center(r,&cx,&cy);

        int x0 = d1x < d2x ? d1x : d2x;
        int w  = abs(d1x-d2x)+1;

        fill_rect(lvl, x0, cy, w, 1, TILE_EMPTY_SP);
    }
    else if (d1x == d2x) {
        fill_rect(lvl, d1x == r->left ? r->left+1 : r->right-1, d1y < d2y ? d1y : d2y, 1, abs(d1y-d2y)+1, TILE_EMPTY_SP);
    }
    else if (d1y == d2y) {
        fill_rect(lvl, d1x < d2x ? d1x : d2x, d1y == r->top ? r->top+1 : r->bottom-1, abs(d1x-d2x)+1, 1, TILE_EMPTY_SP);
    }
    // perpendicular doors
    else if (d1y == r->top || d1y == r->bottom) {
        draw_inside(lvl,r, d1x,d1y, abs(d1y-d2y), TILE_EMPTY_SP);
        draw_inside(lvl,r, d2x,d2y, abs(d1x-d2x), TILE_EMPTY_SP);
    }
    else if (d1x == r->left || d1x == r->right) {
        draw_inside(lvl,r, d1x,d1y, abs(d1x-d2x), TILE_EMPTY_SP);
        draw_inside(lvl,r, d2x,d2y, abs(d1y-d2y), TILE_EMPTY_SP);
    }
}

static void paint_fissure(Level *lvl, Room *r){
    fill_rect(lvl, r->left + 1, r->top + 1, room_w(r) - 1, room_h(r) - 1, TILE_EMPTY);
    for (int y = r->top + 2; y < r->bottom - 1; y++) {
        for (int x = r->left + 2; x < r->right - 1; x++) {
            int v = y - r->top;
            int v2 = r->bottom - y;

            if (v2 < v)
                v = v2;

            int h = x - r->left;
            int h2 = r->right - x;

            if (h2 < h)
                h = h2;
            int min = v < h ? v : h;

            if (min > 2 || rng_int(2)==0) {
                set_tile(lvl,x,y,TILE_CHASM);
            }
        }
    }
}

static void paint_standard_room(Level *lvl, Room *r, int depth){
    paint_room_base(lvl, r, TILE_EMPTY);
    if (rng_int(5)==0) {
        switch(rng_int(6)) {
        case 0:
            if (lvl->feeling != FEELING_GRASS) {
                if (room_w(r) >= 4 && room_h(r) >= 4 && (room_w(r) > 6 || room_h(r) > 6)) {
                    paint_graveyard(lvl,r);
                    return;
                }
            } 
            break;
        case 1:
            if (depth > 2) {
                paint_burned(lvl,r);
                return;
            }
            break;
        case 2:
            if (room_w(r) >= 4 || room_h(r) >= 4) {
                paint_striped(lvl, r);
                return;
            }
            break;
        case 3:
            if (room_w(r) >= 6 && room_h(r) >= 6) {
                paint_study(lvl, r);
                return;
            }
            break;
        case 4:
            if (lvl->feeling != FEELING_WATER){
                paint_bridge(lvl,r);
                return;
            }
            break;
        case 5:
            if (room_w(r) >= 5 && room_h(r) >= 5) { //TODO in future change to !Dungeon.bossLevel() && !Dungeon.bossLevel(Dungeon.depth+1) && Math.min(room.width(), room.height()) >= 5
                paint_fissure(lvl,r);
                return;
            }
            break;
        default:
            break;
        }
    }
    
}
    

void paint_rooms(Level *lvl, int depth){
    for (int i = 0; i < lvl->num_rooms; i++) {
        Room *r = &lvl->rooms[i];

        switch (r->type) {

        case ROOM_STANDARD:
            paint_standard_room(lvl, r, depth);
            break;

        case ROOM_ENTRANCE:
            paint_entrance(lvl, r);
            break;

        case ROOM_EXIT:
            paint_exit(lvl, r);
            break;
        
        case ROOM_SHOP:
            paint_shop(lvl, r);
            break;

        case ROOM_PASSAGE:
        case ROOM_TUNNEL:
        case ROOM_NULL:
        default:
            break;
        }
    }
}
/* ══════════════════════════════════════════════════════════
   Tunnel painter
   ══════════════════════════════════════════════════════════ */
static uint8_t tunnel_tile(const Level *lvl){
    return (lvl->feeling == FEELING_CHASM)
        ? TILE_EMPTY_SP
        : TILE_EMPTY;
}
static void paint_tunnel_room(Level *lvl, Room *r) {
    uint8_t floor = (lvl->feeling == FEELING_CHASM)
        ? TILE_EMPTY_SP : TILE_EMPTY;   
    int cx, cy;
    room_center(r, &cx, &cy);

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

/* ══════════════════════════════════════════════════════════
   Passage painter
   ══════════════════════════════════════════════════════════ */
static int passage_xy2p(const Room *room, int width, int height, int x, int y){
    if (y == room->top) {
        return x - room->left - 1;
    } else if (x == room->right) {
        return (y - room->top - 1) + width;
    } else if (y == room->bottom) {
        return (room->right - x - 1) + width + height;
    } else {
        if (y == room->top + 1)
            return 0;
        return (room->bottom - y - 1) + width * 2 + height;
    }
}

static void passage_p2xy(const Room *room, int width, int height, int p, int *x, int *y){
    if (p < width) {
        *x = room->left + 1 + p;
        *y = room->top + 1;
    } else if (p < width + height) {
        *x = room->right - 1;
        *y = room->top + 1 + (p - width);
    } else if (p < width * 2 + height) {
        *x = room->right - 1 - (p - width - height);
        *y = room->bottom - 1;
    } else {
        *x = room->left + 1;
        *y = room->bottom - 1 - (p - width * 2 - height);
    }
}

static void paint_passage(Level *lvl, Room *room){
    int width  = room_w(room) - 2;
    int height = room_h(room) - 2;

    uint8_t floor = tunnel_tile(lvl);
    int joints[MAX_CONN_ROOM];
    int n = 0;

    for (int i = 0; i < room->n_connected; i++) {
        int x = room->door_x[i];
        int y = room->door_y[i];
        joints[n++] = passage_xy2p(room, width, height, x, y);
    }
    if (n == 0)
        return;
    for (int i = 0; i < n-1; i++) {
        for (int j=i+1; j<n; j++) {
            if (joints[j] < joints[i]) {
                int t = joints[i];
                joints[i] = joints[j];
                joints[j] = t;
            }
        }
    }
    int perimeter = width * 2 + height * 2;
    int start = 0;
    int max_dist = joints[0] + perimeter - joints[n-1];

    for (int i=1; i<n; i++) {
        int d = joints[i] - joints[i-1];
        if (d > max_dist) {
            max_dist = d;
            start = i;
        }
    }
    int end = (start + n - 1) % n;
    int p = joints[start];

    while (p != joints[end]) {
        int x, y;
        passage_p2xy(room, width, height, p, &x, &y);
        set_tile(lvl, x, y, floor);

        if (++p >= perimeter)
            p = 0;
    }

    int x, y;
    passage_p2xy(room, width, height, p, &x, &y);
    set_tile(lvl, x, y, floor);

    for (int i = 0; i < room->n_connected; i++) room->door_type[i] = DOOR_TUNNEL;
}

void paint_tunnels(Level *lvl) {
    for (int i=0; i<lvl->num_rooms; i++){
        Room *r = &lvl->rooms[i];
        if (r->type == ROOM_TUNNEL)
            paint_tunnel_room(lvl, &lvl->rooms[i]);
        else if (r->type == ROOM_PASSAGE)
            paint_passage(lvl, &lvl->rooms[i]);
    }
}

/* ══════════════════════════════════════════════════════════
   DOOR PAINTING  —  RegularLevel.paintDoors()
   ══════════════════════════════════════════════════════════ */
void paint_doors(Level *lvl) {
    for (int i = 0; i < lvl->num_rooms; i++) {
    Room *r = &lvl->rooms[i];
        for (int j = 0; j < r->n_connected; j++) {
            if (r->connected[j] < i)
                continue;
            int x = r->door_x[j];
            int y = r->door_y[j];
            switch (r->door_type[j]) {

            case DOOR_EMPTY:
                set_tile(lvl, x, y, TILE_EMPTY);
                break;
            case DOOR_TUNNEL:
                set_tile(lvl, x, y, tunnel_tile(lvl));
                break;
            case DOOR_REGULAR:
                set_tile(lvl, x, y, TILE_DOOR);
                break;
            case DOOR_UNLOCKED:
                set_tile(lvl, x, y, TILE_DOOR);
                break;
            case DOOR_HIDDEN:
                set_tile(lvl, x, y, TILE_SECRET_DOOR);
                break;
            case DOOR_BARRICADE:
                set_tile(lvl, x, y,
                    rng_int(3) == 0 ? TILE_BOOKSHELF : TILE_BARRICADE);
                break;
            case DOOR_LOCKED:
                set_tile(lvl, x, y, TILE_LOCKED_DOOR);
                break;
            }
        }
    }
}


/* ══════════════════════════════════════════════════════════
   PATCH  —  Patch.java (cellular automaton) 
            + Water/Grass painters
   ══════════════════════════════════════════════════════════ */
void patch_generate(float seed, int n_gen, uint8_t *out) {
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
void paint_water(Level *lvl, const uint8_t *lake) {
    for (int i=0; i<LEVEL_SIZE; i++)
        if (lvl->map[i]==TILE_EMPTY && lake[i]) lvl->map[i]=TILE_WATER;
}

void paint_grass(Level *lvl, const uint8_t *grass) {
    /* paintGrass: for each EMPTY tile with grass patch set,
       probability of HIGH_GRASS vs GRASS depends on how many
       of its 8 neighbours also have the grass patch set.
       map[i] = (Random.Float() < count/12f) ? HIGH_GRASS : GRASS */
    uint8_t grass_mask[LEVEL_SIZE];
    memcpy(grass_mask, grass, LEVEL_SIZE);

    if (lvl->feeling == FEELING_GRASS) {
        for (int i = 0; i < lvl->num_rooms; i++) {
            Room *room = &lvl->rooms[i];
            if (room->type != ROOM_NULL && room->type != ROOM_PASSAGE && room->type != ROOM_TUNNEL) {
                grass_mask[xy_to_idx(room->left + 1, room->top + 1)] = 1;
                grass_mask[xy_to_idx(room->right - 1, room->top + 1)] = 1;
                grass_mask[xy_to_idx(room->left + 1, room->bottom - 1)] = 1;
                grass_mask[xy_to_idx(room->right - 1, room->bottom - 1)] = 1;
            }
        }
    }
    for (int i=LEVEL_W+1; i<LEVEL_SIZE-LEVEL_W-1; i++) {
        if (lvl->map[i]==TILE_EMPTY && grass_mask[i]) {
            int count=1;
            if (grass_mask[i-LEVEL_W-1]) count++;
            if (grass_mask[i-LEVEL_W  ]) count++;
            if (grass_mask[i-LEVEL_W+1]) count++;
            if (grass_mask[i        -1]) count++;
            if (grass_mask[i        +1]) count++;
            if (grass_mask[i+LEVEL_W-1]) count++;
            if (grass_mask[i+LEVEL_W  ]) count++;
            if (grass_mask[i+LEVEL_W+1]) count++;
            lvl->map[i] = (rng_float() < (float)count/12.0f)
                          ? TILE_HIGH_GRASS : TILE_GRASS;
        }
    }
}

/* ══════════════════════════════════════════════════════════
   CHASMS painter (for chasms feeling)
   ══════════════════════════════════════════════════════════ */
void paint_chasms(Level *lvl){
    for (int i = 0; i < LEVEL_SIZE; i++) {
        if (lvl->map[i] == TILE_WALL)
            lvl->map[i] = TILE_CHASM;
    }

    for (int i = 0; i < lvl->num_rooms; i++) {
        Room *r = &lvl->rooms[i];

        if (r->type == ROOM_TUNNEL || r->type == ROOM_PASSAGE){//mb add ROOM_NULL too????
            continue;
        }

        for (int x = r->left; x <= r->right; x++) {
            set_tile(lvl, x, r->top, TILE_WALL);
            set_tile(lvl, x, r->bottom, TILE_WALL);
        }
        for (int y = r->top; y <= r->bottom; y++) {
            set_tile(lvl, r->left,  y, TILE_WALL);
            set_tile(lvl, r->right, y, TILE_WALL);
        }
    }
}