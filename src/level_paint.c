#include "level_paint.h"
#include "item_generator.h"
#include "item_spawn.h"

/* ══════════════════════════════════════════════════════════
    Room.center()
   ══════════════════════════════════════════════════════════ */
static Point room_center(const Room *r) {
    Point p;

    p.x = (r->left + r->right) / 2;
    if ((r->right - r->left) & 1)
        p.x += rng_int(2);

    p.y = (r->top + r->bottom) / 2;
    if ((r->bottom - r->top) & 1)
        p.y += rng_int(2);

    return p;
}

/* ══════════════════════════════════════════════════════════
    Basic room
   ══════════════════════════════════════════════════════════ */
static void paint_room_base(Level *lvl, const Room *r, uint8_t floor){
    fill_rect(lvl, r->left,     r->top,     room_w(r) + 1, room_h(r) + 1, TILE_WALL);
    fill_rect(lvl, r->left + 1, r->top + 1, room_w(r) - 1, room_h(r) - 1, floor);
}
/* ══════════════════════════════════════════════════════════
   SHOP PAINTING
   ══════════════════════════════════════════════════════════ */
//TODO TRANSFER THIS SHOP-STUFF SOMEWHERE
static int perimeter_length(Room *r){
    int w = room_w(r)-2;
    int h = room_h(r)-2;
    return w*2 + h*2;
}
static int shop_p2xy(Room *r, int p){
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
    return xy_to_idx(out.x, out.y);
}
static int shop_xy2p(Room *room, int idx){
    int w = room_w(room)-2;
    int h = room_h(room)-2;

    Point p = idx_to_xy(idx);
    if (p.y == room->top) {
        return p.x - room->left - 1;
    } else if (p.x == room->right) {
        return (p.y - room->top - 1) + w;
    } else if (p.y == room->bottom) {
        return (room->right - p.x - 1) + w + h;
    } else {
        if (p.y == room->top + 1)
            return 0;
        return (room->bottom - p.y - 1) + w*2 + h;
    }
}

static int shop_build_inventory(ItemType *items, int depth){
    int n = 0;

    switch(depth){
    case 5: 
        items[n++] = rng_int(2) ? ITEM_QUARTERSTAFF : ITEM_SPEAR;
        items[n++] = ITEM_LEATHER_ARMOR;
        items[n++] = ITEM_BAG_SEED_POUCH;
        items[n++] = ITEM_WEIGHT;
        break;

    case 10:
        items[n++] = rng_int(2) ? ITEM_SWORD : ITEM_MACE;
        items[n++] = ITEM_MAIL_ARMOR;
        items[n++] = ITEM_BAG_SCROLL_HOLDER;
        items[n++] = ITEM_WEIGHT;
        break;

    case 15:
        items[n++] = rng_int(2) ? ITEM_LONGSWORD : ITEM_BATTLE_AXE;
        items[n++] = ITEM_SCALE_ARMOR;
        items[n++] = ITEM_BAG_WAND_HOLSTER;
        items[n++] = ITEM_WEIGHT;
        break;

    case 20:
        switch(rng_int(3)){
        case 0: items[n++] = ITEM_GLAIVE; break;
        case 1: items[n++] = ITEM_WAR_HAMMER; break;
        case 2: items[n++] = ITEM_PLATE_ARMOR; break;
        }
        items[n++] = ITEM_TORCH;
        items[n++] = ITEM_TORCH;
        break;
    }

    items[n++] = ITEM_POTION_HEALING;

    for(int i=0;i<3;i++)
        items[n++] = random_item_of_category(ITEMCAT_POTION);

    items[n++] = ITEM_SCROLL_IDENTIFY;
    items[n++] = ITEM_SCROLL_REMOVE_CURSE;
    items[n++] = ITEM_SCROLL_MAGIC_MAPPING;
    items[n++] = random_item_of_category(ITEMCAT_SCROLL);

    items[n++] = ITEM_RATION;
    items[n++] = ITEM_RATION;

    items[n++] = ITEM_ANKH;

    return n;
}

static void place_shop_items(Level *lvl, Room *room, int depth){
    int per = perimeter_length(room);

    ItemType items[16];
    int count = shop_build_inventory(items, depth);

    RNG_SHUFFLE(items, count);

    int pos = shop_xy2p(room, room->connections[0].pos);
    pos += (per - count) / 2;

    for (int i = 0; i < count; i++) {

        int idx = shop_p2xy(room, (pos + i) % per);

        if (level_heap_at(lvl, idx) != NULL) {
            int tries = 200;
            do {
                int x = rng_range(room->left + 1, room->right - 1);
                int y = rng_range(room->top + 1, room->bottom - 1);
                idx = xy_to_idx(x, y);
            } while (--tries > 0 && level_heap_at(lvl, idx) != NULL);
        }
        spawn_item_heap(lvl, idx, depth, items[i], HEAP_FOR_SALE);
    }
}

static void place_shopkeeper(Level *lvl, Room *room, int depth){
    const int shopkeeper = depth == LAST_SHOP_DEPTH ? NPC_IMP_SHOPKEEPER : NPC_SHOPKEEPER;

    for (int tries = 0; tries < 200; tries++) {
        int idx = random_passable_in_box(lvl, room->left + 2, room->top + 2, room->right - 2, room->bottom - 2, -1);

        if (idx < 0)  continue;

        if(level_heap_at(lvl, idx) == NULL) { 
            npc_spawn_kind(shopkeeper, idx);
            return;
        }
    }
}

static void paint_shop(Level *lvl, Room *room, int depth){
    paint_room_base(lvl, room, TILE_EMPTY_SP); 
    place_shop_items(lvl, room, depth);
    place_shopkeeper(lvl, room, depth); 
}
/* ══════════════════════════════════════════════════════════
   ENTRANCE/EXIT ROOM PAINTING
   ══════════════════════════════════════════════════════════ */
static void paint_entrance(Level *lvl, Room *r){
    paint_room_base(lvl, r, TILE_EMPTY);

    Point c = room_center(r);
    lvl->entrance = xy_to_idx(c.x, c.y);

    set_tile(lvl, lvl->entrance, TILE_ENTRANCE);
}

static void paint_exit(Level *lvl, Room *r){
    paint_room_base(lvl, r, TILE_EMPTY);

    Point c = room_center(r);
    lvl->exit = xy_to_idx(c.x, c.y);

    set_tile(lvl, lvl->exit, TILE_EXIT);
}

static void paint_boss_exit(Level *lvl, Room *r){
    paint_room_base(lvl, r, TILE_EMPTY);

    int x = (r->left + r->right) / 2;
    int y = r->top;

    lvl->exit = xy_to_idx(x, y);
    set_tile(lvl, lvl->exit, TILE_LOCKED_EXIT);
}
/* ══════════════════════════════════════════════════════════
   STANDARD ROOM PAINTING  —  StandardPainter.paint(), etc.
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

        if (!in_bounds_xy(x, y))
            break;
        int idx = xy_to_idx(x, y);
        set_tile(lvl, idx, tile);
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
            set_tile(lvl, xy_to_idx(x,y), t);
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
    for (int i = 0; i < r->n_connections; i++) {

        Point p = idx_to_xy(r->connections[i].pos);
        if (p.x < 0 || p.y < 0)
            continue;

        if (p.x == r->left) {
            set_tile(lvl, xy_to_idx(p.x + 1, p.y), TILE_EMPTY);
        } else if (p.x == r->right) {
            set_tile(lvl, xy_to_idx(p.x - 1, p.y), TILE_EMPTY);
        } else if (p.y == r->top) {
            set_tile(lvl, xy_to_idx(p.x, p.y + 1), TILE_EMPTY);
        } else if (p.y == r->bottom) {
            set_tile(lvl, xy_to_idx(p.x, p.y - 1), TILE_EMPTY);
        }
    }

    Point c = room_center(r);
    set_tile(lvl, xy_to_idx(c.x, c.y), TILE_PEDESTAL);
}

static void paint_graveyard(Level *lvl, Room *r, int depth){
    /* fill room interior with grass */
    fill_rect(lvl, r->left + 1, r->top + 1, room_w(r) - 1, room_h(r) - 1, TILE_GRASS);

    int w = room_w(r) - 1;
    int h = room_h(r) - 1;
    int n_graves = (w > h ? w : h) / 2;

    if (n_graves <= 0)
        return;
    /* random grave that gets special loot */
    int index = rng_int(n_graves);
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

        if (!in_bounds_xy(x, y))
            continue;

        int idx = xy_to_idx(x, y);
        if (i == index)
            spawn_item_heap(lvl, idx, depth, random_item(), HEAP_TOMB);
        else
            spawn_item_heap(lvl, idx, depth, ITEM_GOLD, HEAP_TOMB);
    }
}

static void paint_bridge(Level *lvl, Room *r, int depth){
    uint8_t base = TILE_WATER;

    if (!level_is_boss_depth(depth) && !level_is_boss_depth(depth + 1) && rng_int(3) == 0){
        base = TILE_CHASM;
    }
    fill_rect(lvl, r->left + 1, r->top + 1, room_w(r)-1, room_h(r)-1, base);

    Point p1 = idx_to_xy(r->connections[0].pos);
    Point p2 = idx_to_xy(r->connections[1].pos);

    if ((p1.x == r->left && p2.x == r->right) || (p1.x == r->right && p2.x == r->left)) {
        int s = room_w(r)/2;

        draw_inside(lvl, r, p1.x, p1.y, s, TILE_EMPTY_SP);
        draw_inside(lvl, r, p2.x, p2.y, s, TILE_EMPTY_SP);

        Point c = room_center(r);

        int y0 = p1.y < p2.y ? p1.y : p2.y;
        int h  = abs(p1.y - p2.y) + 1;

        fill_rect(lvl, c.x, y0, 1, h, TILE_EMPTY_SP);
    }
    else if ((p1.y == r->top && p2.y == r->bottom) || (p1.y == r->bottom && p2.y == r->top)) {
        int s = room_h(r)/2;

        draw_inside(lvl, r, p1.x, p1.y, s, TILE_EMPTY_SP);
        draw_inside(lvl, r, p2.x, p2.y, s, TILE_EMPTY_SP);

        Point c = room_center(r);

        int x0 = p1.x < p2.x ? p1.x : p2.x;
        int w  = abs(p1.x - p2.x) + 1;

        fill_rect(lvl, x0, c.y, w, 1, TILE_EMPTY_SP);
    }
    else if (p1.x == p2.x) {
        fill_rect(lvl, p1.x == r->left ? r->left+1 : r->right-1, p1.y < p2.y ? p1.y : p2.y, 1, abs(p1.y-p2.y)+1, TILE_EMPTY_SP);
    }
    else if (p1.y == p2.y) {
        fill_rect(lvl, p1.x < p2.x ? p1.x : p2.x, p1.y == r->top ? r->top+1 : r->bottom-1, abs(p1.x-p2.x)+1, 1, TILE_EMPTY_SP);
    }
    // perpendicular doors
    else if (p1.y == r->top || p1.y == r->bottom) {
        draw_inside(lvl, r, p1.x, p1.y, abs(p1.y-p2.y), TILE_EMPTY_SP);
        draw_inside(lvl, r, p2.x, p2.y, abs(p1.x-p2.x), TILE_EMPTY_SP);
    }
    else if (p1.x == r->left || p1.x == r->right) {
        draw_inside(lvl, r, p1.x, p1.y, abs(p1.x-p2.x), TILE_EMPTY_SP);
        draw_inside(lvl, r, p2.x, p2.y, abs(p1.y-p2.y), TILE_EMPTY_SP);
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
                set_tile(lvl, xy_to_idx(x,y), TILE_CHASM);
            }
        }
    }
}

static void paint_standard_room(Level *lvl, Room *r, int depth){
    paint_room_base(lvl, r, TILE_EMPTY);
    if (!level_is_boss_depth(depth) && rng_int(5) == 0) {
        switch(rng_int(6)) {
        case 0:
            if (lvl->feeling != FEELING_GRASS) {
                if (room_w(r) >= 4 && room_h(r) >= 4 && (room_w(r) >= 6 || room_h(r) >= 6)) {
                    paint_graveyard(lvl, r, depth);
                    return;
                }
                break;
            } else {
                // Burned room
            }
        case 1:
            if (depth > 1) {
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
                if(r->n_connections == 2 && room_w(r) >= 4 && room_h(r) >= 4){ 
                    paint_bridge(lvl, r, depth);
                    return;
                }
                break;    
            } else {
                // Fissure
            }
        case 5:
            if (!level_is_boss_depth(depth) && !level_is_boss_depth(depth + 1) && room_w(r) >= 5 && room_h(r) >= 5) { 
                paint_fissure(lvl,r);
                return;
            }
            break;
        default:
            break;
        }
    }
    //if (rng_int(4) == 0) { place_room_loot(lvl, r, depth, 1); }  
}
/* ══════════════════════════════════════════════════════════
   WEAK FLOOR ROOM  —  WeakFloorPainter.paint()
   ══════════════════════════════════════════════════════════ */
static void paint_weak_floor(Level *lvl, Room *r) {
    paint_room_base(lvl, r, TILE_CHASM);
    if (r->n_connections == 0)
        return;

    Point p = idx_to_xy(r->connections[0].pos);

    if (p.x == r->left) {
        for (int y = r->top + 1; y < r->bottom; y++) {
            draw_inside(lvl, r, p.x, y, rng_range(1, room_w(r) - 2), TILE_EMPTY_SP);
        }
    } else if (p.x == r->right) {
        for (int y = r->top + 1; y < r->bottom; y++) {
            draw_inside(lvl, r, p.x, y, rng_range(1, room_w(r) - 2), TILE_EMPTY_SP);
        }
    } else if (p.y == r->top) {
        for (int x = r->left + 1; x < r->right; x++) {
            draw_inside(lvl, r, x, p.y, rng_range(1, room_h(r) - 2), TILE_EMPTY_SP);
        }
    } else if (p.y == r->bottom) {
        for (int x = r->left + 1; x < r->right; x++) {
            draw_inside(lvl, r, x, p.y, rng_range(1, room_h(r) - 2), TILE_EMPTY_SP);
        }
    }
} 

/* ══════════════════════════════════════════════════════════
   VAULT ROOM  -  VaultPainter.paint()
   ══════════════════════════════════════════════════════════ */
static void paint_vault(Level *lvl, Room *r, int depth) {
    fill_rect(lvl, r->left, r->top, room_w(r) + 1, room_h(r) + 1, TILE_WALL);
    fill_rect(lvl, r->left + 1, r->top + 1, room_w(r) - 1, room_h(r) - 1, TILE_EMPTY_SP);
    fill_rect(lvl, r->left + 2, r->top + 2, room_w(r) - 3, room_h(r) - 3, TILE_EMPTY);

    int cx = (r->left + r->right) / 2;
    int cy = (r->top + r->bottom) / 2;
    int center = xy_to_idx(cx, cy);

    switch (rng_int(3)) {
        case 0: {
            spawn_random_item_of_category_heap(lvl, center, depth, rng_int(2) ? ITEMCAT_WAND : ITEMCAT_RING, HEAP_LOCKED_CHEST);
            level_add_item_to_spawn(lvl, ITEM_KEY_GOLDEN);
            break;
        }
        case 1: {
            ItemCategory a = rng_int(2) ? ITEMCAT_WAND : ITEMCAT_RING;
            ItemCategory b = (a == ITEMCAT_WAND)
                ? ITEMCAT_RING
                : ITEMCAT_WAND;

            spawn_random_item_of_category_heap(lvl, center, depth, a, HEAP_CRYSTAL_CHEST);

            static const int dx[8] = {-1,0,1,-1,1,-1,0,1};
            static const int dy[8] = {-1,-1,-1,0,0,1,1,1};

            int dir = rng_int(8);
            spawn_random_item_of_category_heap(lvl, xy_to_idx(cx + dx[dir], cy + dy[dir]), depth, b, HEAP_CRYSTAL_CHEST);
            level_add_item_to_spawn(lvl, ITEM_KEY_GOLDEN);
            break;
        }
        case 2: {
            spawn_random_item_of_category(lvl, center, depth, rng_int(2) ? ITEMCAT_WAND : ITEMCAT_RING);
            set_tile(lvl, center, TILE_PEDESTAL);
            break;
        }
    }

    level_add_item_to_spawn(lvl, ITEM_KEY_IRON);
}
/* ══════════════════════════════════════════════════════════
   TREASURY ROOM  —  TreasuryPainter.paint()
   ══════════════════════════════════════════════════════════ */

static void paint_treasury(Level *lvl, Room *r, int depth){
    paint_room_base(lvl, r, TILE_EMPTY);

    Point c = room_center(r);
    set_tile(lvl, xy_to_idx(c.x, c.y), TILE_STATUE);

    HeapType heap_type = (rng_int(2) == 0)
        ? HEAP_CHEST
        : HEAP_NORMAL;

    int n = rng_range(2, 3);

    for (int i = 0; i < n; i++) {
        int x, y, idx;
        int tries = 0;

        do {
            x = rng_range(r->left + 1, r->right - 1);
            y = rng_range(r->top + 1, r->bottom - 1);
            idx = xy_to_idx(x, y);
        } while ((lvl->map[idx] != TILE_EMPTY || level_heap_at(lvl, idx) != NULL) && ++tries < 100);

        if (tries >= 100)
            break;

        HeapType type = (i == 0 && heap_type == HEAP_CHEST)
            ? HEAP_MIMIC
            : heap_type;

        spawn_item_amount_heap(lvl, idx, depth, ITEM_GOLD, gold_amount(depth), type);
    }
    if (heap_type == HEAP_NORMAL) {
        for (int i = 0; i < 6; i++) {
            int x, y, idx;
            int tries = 0;

            do {
                x = rng_range(r->left + 1, r->right - 1);
                y = rng_range(r->top + 1, r->bottom - 1);
                idx = xy_to_idx(x, y);
            } while (lvl->map[idx] != TILE_EMPTY && ++tries < 100);

            if (tries >= 100)
                break;

            spawn_item_amount_heap(lvl, idx, depth, ITEM_GOLD, gold_amount(depth), heap_type);
        }
    }

    level_add_item_to_spawn(lvl, ITEM_KEY_IRON);
}
/* ══════════════════════════════════════════════════════════
   STORAGE ROOM  —  StoragePainter.paint()
   ══════════════════════════════════════════════════════════ */
static Heap *storage_prize(Level *lvl, int idx, int depth){
    ItemCategory cat;
    switch (rng_int(5)) {
    case 0: cat = ITEMCAT_POTION; break;
    case 1: cat = ITEMCAT_SCROLL; break;
    case 2: cat = ITEMCAT_FOOD;   break;
    case 3: cat = ITEMCAT_GOLD;   break;
    default:cat = ITEMCAT_MISC;   break;
    }

    return spawn_random_item_of_category_heap(lvl, idx, depth, cat, HEAP_NORMAL);
}

static void paint_storage(Level *lvl, Room *r, int depth){
    paint_room_base(lvl, r, TILE_EMPTY_SP);
    int n = rng_range(3, 4);
    for (int i = 0; i < n; i++) {
        int x, y, idx;
        int tries = 0;
        do {
            x = rng_range(r->left + 1, r->right - 1);
            y = rng_range(r->top + 1, r->bottom - 1);
            idx = xy_to_idx(x, y);

        } while (lvl->map[idx] != TILE_EMPTY_SP && ++tries < 100);
        if (tries >= 100)
            continue;

        storage_prize(lvl, idx, depth);
    }

    level_add_item_to_spawn(lvl, ITEM_POTION_LIQUID_FLAME);
}
/* ══════════════════════════════════════════════════════════
   STATUE ROOM  —  StatuePainter.paint()
   ══════════════════════════════════════════════════════════ */

static void paint_statue(Level *lvl, Room *r){
    paint_room_base(lvl, r, TILE_EMPTY);
    
    Point c = room_center(r);

    level_add_item_to_spawn(lvl, ITEM_KEY_IRON);

    Point p = idx_to_xy(r->connections[0].pos);

    if (p.x == r->left) {
        fill_rect(lvl, r->right - 1, r->top + 1, 1, room_h(r) - 1, TILE_STATUE);
        c.x = r->right - 2;
    } else if (p.x == r->right) {
        fill_rect(lvl, r->left + 1, r->top + 1, 1, room_h(r) - 1, TILE_STATUE);
        c.x = r->left + 2;
    } else if (p.y == r->top) {
        fill_rect(lvl, r->left + 1, r->bottom - 1, room_w(r) - 1, 1, TILE_STATUE);
        c.y = r->bottom - 2;
    } else if (p.y == r->bottom) {
        fill_rect( lvl, r->left + 1, r->top + 1, room_w(r) - 1, 1, TILE_STATUE);
        c.y = r->top + 2;
    }

    mob_spawn(MOB_STATUE, xy_to_idx(c.x, c.y), NULL);
}
/* ══════════════════════════════════════════════════════════
   POOL ROOM  —  PoolPainter.paint()
   ══════════════════════════════════════════════════════════ */
#define N_PIRANHAS 3
static Heap *pool_prize(Level *lvl, int idx, int depth){
    ItemCategory cat;
    switch (rng_int(2)) {
    case 0: cat = ITEMCAT_WEAPON; break;
    default:cat = ITEMCAT_ARMOR;   break;
    }

    return spawn_random_item_of_category_heap(lvl, idx, depth, cat, rng_int(3) == 0 ? HEAP_CHEST : HEAP_NORMAL);
    /* TODO: CHOOSE THE HIGHEST LEVEL OF 5 WEAPON / ARMOR  */
}

static void paint_pool(Level *lvl, Room *r, int depth){
    paint_room_base(lvl, r, TILE_WATER);

    if (r->n_connections == 0)
        return;

    int x = -1;
    int y = -1;

    Point p = idx_to_xy(r->connections[0].pos);

    if (p.x == r->left) {
        x = r->right - 1;
        y = r->top + room_h(r) / 2;
    } else if (p.x == r->right) {
        x = r->left + 1;
        y = r->top + room_h(r) / 2;
    } else if (p.y == r->top) {
        x = r->left + room_w(r) / 2;
        y = r->bottom - 1;
    } else if (p.y == r->bottom) {
        x = r->left + room_w(r) / 2;
        y = r->top + 1;
    }
    int idx = xy_to_idx(x, y);
    pool_prize(lvl, idx, depth);
    set_tile(lvl, idx, TILE_PEDESTAL);
    level_add_item_to_spawn(lvl, ITEM_POTION_INVISIBILITY);
    for (int i = 0; i < N_PIRANHAS; i++) {
        int pos = room_random_passable_point(lvl, r, idx);
        if (pos >= 0)
            mob_spawn(MOB_PIRANHA, pos, NULL);
    }
}
/* ══════════════════════════════════════════════════════════
   PIT ROOM  —  PitPainter.paint()
   ══════════════════════════════════════════════════════════ */
static Heap *pit_prize(Level *lvl, int idx, int depth){
    ItemCategory cat;
    switch (rng_int(4)) {
    case 0: cat = ITEMCAT_POTION; break;
    case 1: cat = ITEMCAT_SCROLL; break;
    case 2: cat = ITEMCAT_FOOD;   break;
    default:cat = ITEMCAT_GOLD;   break;
    }

    return spawn_random_item_of_category_heap(lvl, idx, depth, cat, HEAP_SKELETON);
}

static void paint_pit(Level *lvl, Room *r, int depth){
    paint_room_base(lvl, r, TILE_EMPTY);
    if (r->n_connections == 0)
        return;

    int wx = -1;
    int wy = -1;

    Point p = idx_to_xy(r->connections[0].pos);

    if (p.x == r->left) {
        wx = r->right - 1;
        wy = (rng_int(2) == 0) ? r->top + 1 : r->bottom - 1;
    } else if (p.x == r->right) {
        wx = r->left + 1;
        wy = (rng_int(2) == 0) ? r->top + 1 : r->bottom - 1;
    } else if (p.y == r->top) {
        wx = (rng_int(2) == 0) ? r->left + 1 : r->right - 1;
        wy = r->bottom - 1;
    } else if (p.y == r->bottom) {
        wx = (rng_int(2) == 0) ? r->left + 1 : r->right - 1;
        wy = r->top + 1;
    }
    int widx = xy_to_idx(wx, wy);
    set_tile(lvl, widx, TILE_EMPTY_WELL);

    int remains = random_passable_in_box(lvl, r->left + 1, r->top + 1, r->right - 1, r->bottom - 1, widx);

    if (remains < 0)
        return;

    spawn_item_heap(lvl, remains, depth, ITEM_KEY_IRON, HEAP_SKELETON);

    if (rng_int(5) == 0) {
        spawn_random_item_of_category_heap(lvl, remains, depth, ITEMCAT_RING, HEAP_SKELETON);
    } else {
        ItemCategory cat;
        switch (rng_int(2)) {
        case 0: cat = ITEMCAT_WEAPON; break;
        default:cat = ITEMCAT_ARMOR;   break;
        }
        spawn_random_item_of_category_heap(lvl, remains, depth, cat, HEAP_SKELETON);
    }
    int n = rng_range(1, 2);
    for (int i = 0; i < n; i++) {
        pit_prize(lvl, remains, depth);
    }
}
/* ══════════════════════════════════════════════════════════
   RAT KING ROOM  —  RatKingPainter.paint()
   ══════════════════════════════════════════════════════════ */

static void ratking_add_chest(Level *lvl, int pos, int door, int depth){
    if (pos == door - 1 ||
        pos == door + 1 ||
        pos == door - LEVEL_W ||
        pos == door + LEVEL_W) {
        return;
    }
    switch (rng_int(10)) {
        case 0:
            /* TODO:  ITEMCAT_WEAPON WITH: QUANTITY = 1 DEGRADE FOR rng_int(3) */
            spawn_random_item_of_category_heap(lvl, pos, depth, ITEMCAT_WEAPON, HEAP_CHEST);
            break;
        case 1:
            /* TODO: ITEMCAT_ARMOR WITH: DEGRADE FOR rng_int(3) */
            spawn_random_item_of_category_heap(lvl, pos, depth, ITEMCAT_ARMOR, HEAP_CHEST);
            break;
        default:
            spawn_item_amount_heap(lvl, pos, depth, ITEM_GOLD, rng_range(1,5), HEAP_CHEST); //TODO mb wrong amount of gold
            break;
    }
}

static void paint_rat_king(Level *lvl, Room *r, int depth){
    paint_room_base(lvl, r, TILE_EMPTY_SP);

    int door = r->connections[0].pos;

    for (int x = r->left + 1; x < r->right; x++) {
        ratking_add_chest(lvl, xy_to_idx(x, r->top    + 1), door, depth);
        ratking_add_chest(lvl, xy_to_idx(x, r->bottom - 1), door, depth);
    }
    for (int y = r->top + 2; y < r->bottom - 1; y++) {
        ratking_add_chest(lvl, xy_to_idx(r->left  + 1, y), door, depth);
        ratking_add_chest(lvl, xy_to_idx(r->right - 1, y), door, depth);
    }

    for (int tries = 0; tries < 100; tries++) { //REDO cycle for mimic spawner
        int x = rng_range(r->left + 1, r->right - 1);
        int y = rng_range(r->top + 1, r->bottom - 1);
        int pos = xy_to_idx(x, y);
        Heap *h = level_heap_at(lvl, pos);
        if (h) {
            h->type = HEAP_MIMIC;
            break;
        }
    }
    for (int tries = 0; tries < 200; tries++) {
        int idx = random_passable_in_box(lvl, r->left + 2, r->top + 2, r->right - 2, r->bottom - 2, -1);

        if (idx < 0) continue;

        if(level_heap_at(lvl, idx) == NULL) { 
            npc_spawn_kind(NPC_RATKING, idx);
            return;
        }
    }
}
/* ══════════════════════════════════════════════════════════
   LIBRARY ROOM  —  LibraryPainter.paint()
   ══════════════════════════════════════════════════════════ */

static Heap *library_prize(Level *lvl, int idx, int depth){
    /* TODO:
       Original:
       Item prize = level.itemToSpawnAsPrize();
       if (prize instanceof Scroll)
           return prize;
       if (prize != null)
           level_add_item_to_spawn(lvl, prize);
       Until prize pool exists, always generate a random scroll.
    */
    return spawn_random_item_of_category_heap(lvl, idx, depth, ITEMCAT_SCROLL, HEAP_NORMAL);
}

static void paint_library(Level *lvl, Room *r, int depth){
    paint_room_base(lvl, r, TILE_EMPTY);

    int ax = -1, ay = -1;
    int bx = -1, by = -1;

    Point p = idx_to_xy(r->connections[0].pos);

    if (p.x == r->left) {
        ax = r->left + 1;
        ay = p.y - 1;
        bx = r->left + 1;
        by = p.y + 1;

        fill_rect(lvl, r->right - 1, r->top + 1, 1, room_h(r) - 1, TILE_BOOKSHELF);

    } else if (p.x == r->right) {
        ax = r->right - 1;
        ay = p.y - 1;
        bx = r->right - 1;
        by = p.y + 1;

        fill_rect(lvl, r->left + 1, r->top + 1, 1, room_h(r) - 1, TILE_BOOKSHELF);

    } else if (p.y == r->top) {
        ax = p.x + 1;
        ay = r->top + 1;
        bx = p.x - 1;
        by = r->top + 1;

        fill_rect(lvl, r->left + 1, r->bottom - 1, room_w(r) - 1, 1, TILE_BOOKSHELF);

    } else if (p.y == r->bottom) {
        ax = p.x + 1;
        ay = r->bottom - 1;
        bx = p.x - 1;
        by = r->bottom - 1;

        fill_rect(lvl, r->left + 1, r->top + 1, room_w(r) - 1, 1, TILE_BOOKSHELF);
    }
    if (in_bounds_xy(ax, ay)) {
        int aidx = xy_to_idx(ax, ay);
        if (lvl->map[aidx] == TILE_EMPTY)
            set_tile(lvl, aidx, TILE_STATUE);
    }
    if (in_bounds_xy(bx, by)) {
        int bidx = xy_to_idx(bx, by);
        if (lvl->map[bidx] == TILE_EMPTY)
            set_tile(lvl, bidx, TILE_STATUE);
    }
    int n = rng_range(2, 3);
    for (int i = 0; i < n; i++) {
        int x, y, idx;

        bool found = false;

        for (int tries = 0; tries < 100; tries++) {
            x = rng_range(r->left + 1, r->right - 1);
            y = rng_range(r->top + 1, r->bottom - 1);
            idx = xy_to_idx(x, y);

            if (lvl->map[idx] == TILE_EMPTY && level_heap_at(lvl, idx) == NULL) {
                found = true;
                break;
            }
        }
        if (found) library_prize(lvl, idx, depth);
    }

    level_add_item_to_spawn(lvl, ITEM_KEY_IRON);
}

/* ══════════════════════════════════════════════════════════
   LABORATORY ROOM  —  LaboratoryPainter.paint()
   ══════════════════════════════════════════════════════════ */

static Heap *laboratory_prize(Level *lvl, int idx, int depth) {
    /* TODO:
       Original:
       Item prize = level.itemToSpawnAsPrize();
       if (prize instanceof Potion)
           return prize;
       if (prize != null)
           level_add_item_to_spawn(lvl, prize);
    */
    return spawn_random_item_of_category_heap(lvl, idx, depth, ITEMCAT_POTION, HEAP_NORMAL);
}

static void paint_laboratory(Level *lvl, Room *r, int depth) {
    paint_room_base(lvl, r, TILE_EMPTY_SP);

    int px = -1, py = -1;

    Point p = idx_to_xy(r->connections[0].pos);

    if (p.x == r->left) {
        px = r->right - 1;
        py = rng_int(2) ? r->top + 1 : r->bottom - 1;
    } else if (p.x == r->right) {
        px = r->left + 1;
        py = rng_int(2) ? r->top + 1 : r->bottom - 1;
    } else if (p.y == r->top) {
        px = rng_int(2) ? r->left + 1 : r->right - 1;
        py = r->bottom - 1;
    } else if (p.y == r->bottom) {
        px = rng_int(2) ? r->left + 1 : r->right - 1;
        py = r->top + 1;
    }
    if (in_bounds_xy(px, py)) {
        int pidx = xy_to_idx(px, py);
        set_tile(lvl, pidx, TILE_ALCHEMY);
        /* TODO:
           Alchemy alchemy = new Alchemy();
           alchemy.seed(pos,1);
        */     
    }

    int n = rng_range(2, 3);
    for (int i = 0; i < n; i++) {
        int x, y, idx;
        int tries = 0;
        do {
            x = rng_range(r->left + 1, r->right - 1);
            y = rng_range(r->top + 1, r->bottom - 1);
            idx = xy_to_idx(x, y);
        } while ((lvl->map[idx] != TILE_EMPTY_SP || level_heap_at(lvl, idx) != NULL) &&  ++tries < 100);
        if (tries < 100)
            laboratory_prize(lvl, idx, depth);
    }

    level_add_item_to_spawn(lvl, ITEM_KEY_IRON);
}
/* ══════════════════════════════════════════════════════════
   GARDEN ROOM  —  GardenPainter.paint()
   ══════════════════════════════════════════════════════════ */

static void paint_garden(Level *lvl, Room *r, int depth){
    fill_rect(lvl, r->left, r->top, room_w(r) + 1, room_h(r) + 1, TILE_WALL);
    fill_rect(lvl, r->left + 1, r->top + 1, room_w(r) - 1, room_h(r) - 1, TILE_HIGH_GRASS);
    fill_rect(lvl, r->left + 2, r->top + 2, room_w(r) - 3, room_h(r) - 3, TILE_GRASS);

    if (rng_int(2) == 0) {
        int x, y, idx;
        int tries = 0;
        do {
            x = rng_range(r->left + 1, r->right - 1);
            y = rng_range(r->top + 1, r->bottom - 1);
            idx = xy_to_idx(x, y);
        } while (level_heap_at(lvl, idx) != NULL && ++tries < 100);
        if (tries < 100)
            spawn_item_heap(lvl, idx, depth, ITEM_HONEYPOT, HEAP_NORMAL);
    } else {
        int bushes = (rng_int(5) == 0) ? 2 : 1;
        for (int i = 0; i < bushes; i++) {
            int x, y, idx;

            int tries = 100;
            do {
                x = rng_range(r->left + 1, r->right - 1);
                y = rng_range(r->top + 1, r->bottom - 1);
                idx = xy_to_idx(x, y);
            } while (--tries && lvl->map[idx] != TILE_GRASS);
            if (tries == 0) break;
            set_tile(lvl, idx, TILE_GRASS);

            /* TODO: plant_spawn(lvl, PLANT_SUNGRASS, x, y); */
        }
    }
/* TODO: Foliage blob.
       for (int y = r->top + 1; y < r->bottom; y++) {
           for (int x = r->left + 1; x < r->right; x++) {
               blob_seed(BLOB_FOLIAGE, xy_to_idx(x,y), 1);
           }
       }*/
}
/* ══════════════════════════════════════════════════════════
   CRYPT ROOM — CryptPainter.paint()
   ══════════════════════════════════════════════════════════ */
static Heap *crypt_prize(Level *lvl, int idx, int depth){
    return spawn_random_item_of_category_heap(lvl, idx, depth, ITEMCAT_ARMOR, HEAP_TOMB); /* TODO: CHOOSE THE HIGHEST LEVEL OF 3 ARMOR  */ 
}

static void paint_crypt(Level *lvl, Room *r, int depth){
    paint_room_base(lvl, r, TILE_EMPTY);
    level_add_item_to_spawn(lvl, ITEM_KEY_IRON);

    Point c = room_center(r);
    Point p = idx_to_xy(r->connections[0].pos);

    if (p.x == r->left) {
        set_tile(lvl, xy_to_idx(r->right-1, r->top   +1), TILE_STATUE);
        set_tile(lvl, xy_to_idx(r->right-1, r->bottom-1), TILE_STATUE);
        c.x = r->right - 2;
    } else if (p.x == r->right) {
        set_tile(lvl, xy_to_idx(r->left+1, r->top   +1), TILE_STATUE);
        set_tile(lvl, xy_to_idx(r->left+1, r->bottom-1), TILE_STATUE);
        c.x = r->left + 2;
    } else if (p.y == r->top) {
        set_tile(lvl, xy_to_idx(r->left +1, r->bottom-1), TILE_STATUE);
        set_tile(lvl, xy_to_idx(r->right-1, r->bottom-1), TILE_STATUE);
        c.y = r->bottom - 2;
    } else if (p.y == r->bottom) {
        set_tile(lvl, xy_to_idx(r->left +1, r->top+1), TILE_STATUE);
        set_tile(lvl, xy_to_idx(r->right-1, r->top+1), TILE_STATUE);
        c.y = r->top + 2;
    }

    crypt_prize(lvl, xy_to_idx(c.x, c.y), depth);
}
/* ══════════════════════════════════════════════════════════
   BLACKSMITH ROOM — BlacksmithPainter.paint()
   ══════════════════════════════════════════════════════════ */
static void paint_blacksmith(Level *lvl, Room *r, int depth){
    fill_rect(lvl, r->left, r->top, room_w(r) + 1, room_h(r) + 1, TILE_WALL);
    fill_rect(lvl, r->left + 1, r->top + 1, room_w(r) - 1, room_h(r) - 1, TILE_FIRE_TRAP);
    fill_rect(lvl, r->left + 2, r->top + 2, room_w(r) - 3, room_h(r) - 3, TILE_EMPTY_SP);
    /* two random weapon/armor items */
    for (int i = 0; i < 2; i++) {
        int x, y, idx;

        int tries = 0;
        do {
            x = rng_range(r->left + 2, r->right - 2);
            y = rng_range(r->top + 2, r->bottom - 2);
            idx = xy_to_idx(x,y);
        } while ((lvl->map[idx] != TILE_EMPTY_SP || level_heap_at(lvl, idx) != NULL) && ++tries < 100);
        if (tries < 100)
            spawn_random_item_of_category_heap(lvl, idx, depth, rng_int(2) == 0 ? ITEMCAT_ARMOR : ITEMCAT_WEAPON, HEAP_NORMAL);
    }

    for (int i = 0; i < r->n_connections; i++) {
        Point p = idx_to_xy(r->connections[i].pos);

        if (p.x == r->left) {
            set_tile(lvl, xy_to_idx(p.x + 1, p.y), TILE_EMPTY);
        } else if (p.x == r->right) {
            set_tile(lvl, xy_to_idx(p.x - 1, p.y), TILE_EMPTY);
        } else if (p.y == r->top) {
            set_tile(lvl, xy_to_idx(p.x, p.y + 1), TILE_EMPTY);
        } else if (p.y == r->bottom) {
            set_tile(lvl, xy_to_idx(p.x, p.y - 1), TILE_EMPTY);
        }
    }
    for (int tries = 0; tries < 200; tries++) {
        int idx = random_passable_in_box(lvl, r->left + 2, r->top + 2, r->right - 2, r->bottom - 2, -1);

        if (idx < 0) continue;

        if(level_heap_at(lvl, idx) == NULL) { 
            npc_spawn_kind(NPC_BLACKSMITH, idx);
            return;
        }
    }
}
/* ══════════════════════════════════════════════════════════
   ARMORY ROOM — ArmoryPainter.paint()
   ══════════════════════════════════════════════════════════ */
static Heap *armory_prize(Level *lvl, int idx, int depth){
    if (rng_int(6) == 0) {
        return spawn_item_heap(lvl, idx, depth, ITEM_BOMB, HEAP_NORMAL);
    } else {
        return spawn_random_item_of_category_heap(lvl, idx, depth, rng_int(2) ? ITEMCAT_ARMOR : ITEMCAT_WEAPON, HEAP_NORMAL);
    }    
}

static void paint_armory(Level *lvl, Room *r, int depth){
    paint_room_base(lvl, r, TILE_EMPTY);
    /* statue opposite entrance */
    if (r->n_connections > 0) {
        Point p = idx_to_xy(r->connections[0].pos);

        if (p.x == r->left) {
            set_tile(lvl, xy_to_idx(r->right - 1, rng_int(2) ? r->top + 1 :  r->bottom - 1), TILE_STATUE);
        } else if (p.x == r->right) {
            set_tile(lvl, xy_to_idx(r->left  + 1, rng_int(2) ? r->top + 1 :  r->bottom - 1), TILE_STATUE);
        } else if (p.y == r->top) {
            set_tile(lvl, xy_to_idx(rng_int(2) ? r->left + 1 : r->right - 1, r->bottom - 1), TILE_STATUE);
        } else if (p.y == r->bottom) {
            set_tile(lvl, xy_to_idx(rng_int(2) ? r->left + 1 : r->right - 1, r->top    + 1), TILE_STATUE);
        }
    }
    int n = 3 + (rng_int(4) == 0);
    for (int i = 0; i < n; i++) {
        int x, y, idx;
        int tries = 0;

        do {
            x = rng_range(r->left + 1, r->right - 1);
            y = rng_range(r->top + 1, r->bottom - 1);
            idx = xy_to_idx(x, y);
        } while ((lvl->map[idx] != TILE_EMPTY || level_heap_at(lvl, idx) != NULL) && ++tries < 100);

        if (tries >= 100)
            break;

        armory_prize(lvl, idx, depth);
    }

    level_add_item_to_spawn(lvl, ITEM_KEY_IRON);
}
/* ══════════════════════════════════════════════════════════
   TRAPS ROOM  —  TrapsPainter.paint()
   ══════════════════════════════════════════════════════════ */
static void paint_traps(Level *lvl, Room *r, int depth){
    fill_rect(lvl, r->left, r->top, room_w(r) + 1, room_h(r) + 1, TILE_WALL);
    uint8_t trap;
    switch (rng_int(6)) {
        case 0:
        case 1:
        case 2:
            trap = TILE_TOXIC_TRAP;
            break;
        case 3:
        case 4:
            trap = TILE_PARALYTIC_TRAP;
            break;
        default:
            trap = level_is_boss_depth(depth)
                ? TILE_SUMMONING_TRAP
                : TILE_CHASM;
            break;
    }
    fill_rect(lvl, r->left + 1, r->top + 1, room_w(r) - 1, room_h(r) - 1, trap);
    if (r->n_connections == 0)
        return;

    uint8_t safe_tile = (trap == TILE_CHASM) ? TILE_CHASM : TILE_EMPTY;
    int x = -1;
    int y = -1;

    Point p = idx_to_xy(r->connections[0].pos);

    if (p.x == r->left) {
        x = r->right - 1;
        y = r->top + room_h(r) / 2;
        fill_rect(lvl, x, r->top + 1, 1, room_h(r) - 1, safe_tile);
    } else if (p.x == r->right) {
        x = r->left + 1;
        y = r->top + room_h(r) / 2;
        fill_rect(lvl, x, r->top + 1, 1, room_h(r) - 1, safe_tile);
    } else if (p.y == r->top) {
        x = r->left + room_w(r) / 2;
        y = r->bottom - 1;
        fill_rect(lvl, r->left + 1, y, room_w(r) - 1, 1, safe_tile);
    } else if (p.y == r->bottom) {
        x = r->left + room_w(r) / 2;
        y = r->top + 1;
        fill_rect(lvl,  r->left + 1, y, room_w(r) - 1, 1, safe_tile);
    }
    int idx = xy_to_idx(x, y);
    if (rng_int(3) == 0) {
        if (safe_tile == TILE_CHASM)
            set_tile(lvl, idx, TILE_EMPTY);
        spawn_random_item_of_category_heap(lvl, idx, depth, rng_int(2) ? ITEMCAT_WEAPON : ITEMCAT_ARMOR, HEAP_CHEST);
    } else {
        set_tile(lvl, idx, TILE_PEDESTAL);
        spawn_random_item_of_category(lvl, idx, depth, rng_int(2) ? ITEMCAT_WEAPON : ITEMCAT_ARMOR);
    }
    level_add_item_to_spawn(lvl, ITEM_POTION_LEVITATION);
}
/* ══════════════════════════════════════════════════════════
   ALTAR ROOM — AltarPainter.paint()
   ══════════════════════════════════════════════════════════ */
static void paint_altar(Level *lvl, Room *r, int depth){ //REDO broken
    fill_rect(lvl, r->left, r->top, room_w(r)+1, room_h(r)+1, TILE_WALL);
    uint8_t floor = level_is_boss_depth(depth) ? TILE_HIGH_GRASS : TILE_CHASM;
    fill_rect(lvl, r->left + 1, r->top + 1, room_w(r)-1, room_h(r)-1, floor);
    
    Point c = room_center(r);

    if (r->n_connections > 0) {
        Point p = idx_to_xy(r->connections[0].pos);

        if (p.x == r->left || p.x == r->right) {
            int len = abs(p.x - c.x) - 2;
            draw_inside(lvl, r, p.x, p.y, len, TILE_EMPTY_SP);
            int x = p.x;
            int y = p.y;

            if (p.x == r->left)
                x += len;
            else if (p.x == r->right)
                x -= len;
            else if (p.y == r->top)
                y += len;
            else if (p.y == r->bottom)
                y -= len;

            while (y != c.y) {
                y += y < c.y ? 1 : -1;
                set_tile(lvl, xy_to_idx(x, y), TILE_EMPTY_SP);
            }
        } else {
            int len = abs(p.y - c.y) - 2;
            draw_inside(lvl, r, p.x, p.y, len, TILE_EMPTY_SP);
            int x = p.x;
            int y = p.y;

            if (p.x == r->left)
                x += len;
            else if (p.x == r->right)
                x -= len;
            else if (p.y == r->top)
                y += len;
            else if (p.y == r->bottom)
                y -= len;

            while (x != c.x) {
                x += x < c.x ? 1 : -1;
                set_tile(lvl, xy_to_idx(x, y), TILE_EMPTY_SP);
            }
        }
    }
    fill_rect(lvl, c.x - 1, c.y - 1, 3, 3, TILE_EMBERS);
    set_tile( lvl, xy_to_idx(c.x, c.y), TILE_PEDESTAL);
    /* TODO: SacrificialFire blob
       blob_seed(SACRIFICIAL_FIRE, xy_to_idx(cx,cy), 5 + depth*5); */
}
/* ══════════════════════════════════════════════════════════
   MAGIC WELL ROOM 
   ══════════════════════════════════════════════════════════ */
static void paint_magic_well(Level *lvl, Room *r) {
    paint_room_base(lvl, r, TILE_EMPTY);
    
    Point c = room_center(r);
    set_tile(lvl, xy_to_idx(c.x, c.y), TILE_EMPTY_WELL);
    switch (rng_int(3)) {
        case 0:
            /* TODO: Water of Awareness */
            break;
        case 1:
            /* TODO: Water of Health */
            break;
        case 2:
            /* TODO: Water of Transmutation */
            break;
    }
}
/* ══════════════════════════════════════════════════════════
   ROOM PAINTING
   ══════════════════════════════════════════════════════════ */
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
        case ROOM_BOSS_EXIT:
            paint_boss_exit(lvl, r);
            break;   
        case ROOM_SHOP:
            paint_shop(lvl, r, depth);
            break;
        case ROOM_WEAK_FLOOR:
            paint_weak_floor(lvl, r);
            break;       
        case ROOM_VAULT:
            paint_vault(lvl, r, depth);
            break;
        case ROOM_TREASURY:
            paint_treasury(lvl, r, depth);
            break;
        case ROOM_STORAGE:
            paint_storage(lvl, r, depth);
            break;
        case ROOM_STATUE:
            paint_statue(lvl, r);
            break;
        case ROOM_POOL:
            paint_pool(lvl, r, depth);
            break;
        case ROOM_PIT:
            paint_pit(lvl, r, depth);
            break;
        case ROOM_RAT_KING:
            paint_rat_king(lvl, r, depth);
            break;
        case ROOM_LIBRARY:
            paint_library(lvl, r, depth);
            break;
        case ROOM_LABORATORY:
            paint_laboratory(lvl, r, depth);
            break;
        case ROOM_GARDEN:
            paint_garden(lvl, r, depth);
            break;
        case ROOM_CRYPT:
            paint_crypt(lvl, r, depth);
            break;
        case ROOM_BLACKSMITH:
            paint_blacksmith(lvl, r, depth);
            break;
        case ROOM_ARMORY:
            paint_armory(lvl, r, depth);
            break;
        case ROOM_TRAPS:
            paint_traps(lvl, r, depth);
            break;
        case ROOM_ALTAR:
            paint_altar(lvl, r, depth);
            break;
        case ROOM_MAGIC_WELL:
            paint_magic_well(lvl, r);
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
    uint8_t floor = tunnel_tile(lvl);
    
    Point c = room_center(r);

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

        for (int j=0; j<r->n_connections; j++) {
            Point p = idx_to_xy(r->connections[j].pos);

            if (p.x < 0 || p.y < 0) continue;
            int step = (p.y < c.y) ? +1 : -1;

            if (p.x == r->left) {
                /* Door on left wall: stub goes from door.y toward cy
                   along the left interior column (left+1) */
                from = r->left + 1;
                for (int k=p.y; k != c.y; k += step)
                    set_tile(lvl, xy_to_idx(from, k), floor);

            } else if (p.x == r->right) {
                /* Door on right wall: stub along right interior col */
                to = r->right - 1;
                for (int k=p.y; k != c.y; k += step)
                    set_tile(lvl, xy_to_idx(to, k), floor);

            } else {
                /* Door on top/bottom: extend spine to this x,
                   draw stub from door.y+step toward cy */
                if (p.x < from) from = p.x;
                if (p.x > to)   to   = p.x;
                for (int k=p.y+step; k != c.y; k += step)
                    set_tile(lvl, xy_to_idx(p.x, k), floor);
            }
        }
        /* Horizontal spine */
        for (int k=from; k<=to; k++)
            set_tile(lvl, xy_to_idx(k, c.y), floor);

    } else {
        /* ── Vertical-primary tunnel (axes swapped) ─────── */
        int from = r->bottom - 1;
        int to   = r->top    + 1;

        for (int j=0; j<r->n_connections; j++) {
            Point p = idx_to_xy(r->connections[j].pos);

            if (p.x < 0 || p.y < 0) continue;
            int step = (p.x < c.x) ? +1 : -1;

            if (p.y == r->top) {
                from = r->top + 1;
                for (int k=p.x; k != c.x; k += step)
                    set_tile(lvl, xy_to_idx(k, from), floor);

            } else if (p.y == r->bottom) {
                to = r->bottom - 1;
                for (int k=p.x; k != c.x; k += step)
                    set_tile(lvl, xy_to_idx(k, to), floor);

            } else {
                if (p.y < from) from = p.y;
                if (p.y > to)   to   = p.y;
                for (int k=p.x+step; k != c.x; k += step)
                    set_tile(lvl, xy_to_idx(k, p.y), floor);
            }
        }
        /* Vertical spine */
        for (int k=from; k<=to; k++)
            set_tile(lvl, xy_to_idx(c.x, k), floor);
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

static int passage_p2xy(const Room *room, int width, int height, int p){
    Point point;
    if (p < width) {
        point.x = room->left + 1 + p;
        point.y = room->top + 1;
    } else if (p < width + height) {
        point.x = room->right - 1;
        point.y = room->top + 1 + (p - width);
    } else if (p < width * 2 + height) {
        point.x = room->right - 1 - (p - width - height);
        point.y = room->bottom - 1;
    } else {
        point.x = room->left + 1;
        point.y = room->bottom - 1 - (p - width * 2 - height);
    }
    return xy_to_idx(point.x, point.y);
}

static void paint_passage(Level *lvl, Room *room){
    int width  = room_w(room) - 2;
    int height = room_h(room) - 2;

    uint8_t floor = tunnel_tile(lvl);
    int joints[MAX_CONN_ROOM];
    int n = 0;

    for (int i = 0; i < room->n_connections; i++) {
        Point p = idx_to_xy(room->connections[i].pos);

        joints[n++] = passage_xy2p(room, width, height, p.x, p.y);
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
        set_tile(lvl, passage_p2xy(room, width, height, p), floor);

        if (++p >= perimeter)
            p = 0;
    }
    set_tile(lvl, passage_p2xy(room, width, height, p), floor);
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
        for (int j = 0; j < r->n_connections; j++) {
            if (r->connections[j].room < i)
                continue;

            switch (r->connections[j].type) {

            case DOOR_EMPTY:
                set_tile(lvl, r->connections[j].pos, TILE_EMPTY);
                break;
            case DOOR_TUNNEL:
                set_tile(lvl, r->connections[j].pos, tunnel_tile(lvl));
                break;
            case DOOR_REGULAR:
                set_tile(lvl, r->connections[j].pos, TILE_DOOR);
                break;
            case DOOR_UNLOCKED:
                set_tile(lvl, r->connections[j].pos, TILE_DOOR);
                break;
            case DOOR_HIDDEN:
                set_tile(lvl, r->connections[j].pos, TILE_SECRET_DOOR);
                break;
            case DOOR_BARRICADE:
                set_tile(lvl, r->connections[j].pos, rng_int(3) == 0 ? TILE_BOOKSHELF : TILE_BARRICADE);
                break;
            case DOOR_LOCKED:
                set_tile(lvl, r->connections[j].pos, TILE_LOCKED_DOOR);
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
                int p = xy_to_idx (x, y);
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
   CHASMS painter
   ══════════════════════════════════════════════════════════ */
void paint_chasms_feeling(Level *lvl){
    if (lvl->feeling == FEELING_CHASM){
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
                set_tile(lvl, xy_to_idx(x, r->top), TILE_WALL);
                set_tile(lvl, xy_to_idx(x, r->bottom), TILE_WALL);
            }
            for (int y = r->top; y <= r->bottom; y++) {
                set_tile(lvl, xy_to_idx(r->left,  y), TILE_WALL);
                set_tile(lvl, xy_to_idx(r->right, y), TILE_WALL);
            }
        }
    }
}

void paint_chasms(Level *lvl){
    for (int cur = 0; cur < LEVEL_SIZE; cur++) {
        int above = cur - LEVEL_W;

        if (lvl->map[cur] != TILE_CHASM)
            continue;

        switch (lvl->map[above]) {
            case TILE_WALL:
            case TILE_WALL_DECO:
            case TILE_DOOR:
            case TILE_OPEN_DOOR:
                lvl->map[cur] = TILE_CHASM_WALL;
                break;

            case TILE_EMPTY:
            case TILE_EMPTY_DECO:
            case TILE_GRASS:
            case TILE_HIGH_GRASS:
            case TILE_EMBERS: //stupid fix for altar (i really hate this room...)
                lvl->map[cur] = TILE_CHASM_FLOOR;
                break;

            case TILE_EMPTY_SP:
                lvl->map[cur] = TILE_CHASM_FLOOR_SP;
                break;

            case TILE_WATER:
                lvl->map[cur] = TILE_CHASM_WATER;
                break;
        }
    }
}