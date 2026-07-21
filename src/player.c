#include "player.h"
#include <stdlib.h> /* qsort — player_sort_inventory */

/*═══════════════════════════════════════════════════════════════
    Animation data
═══════════════════════════════════════════════════════════════*/
/* ── hero animation frame tables ─────────────────────────
   Column indices refer to the shared 21-column layout every class
   (and every armor-tier row) follows on the hero sheets — see the
   comment above AnimState in player.h. */
static const uint8_t F_IDLE[]    = {0,0,0,1,0,0,1,1};
static const uint8_t F_RUN[]     = {2,3,4,5,6,7};
static const uint8_t F_DIE[]     = {8,9,10,11,12,11};
static const uint8_t F_ATTACK[]  = {13,14,15,0};
static const uint8_t F_OPERATE[] = {16,17,16,17};
//static const uint8_t F_FLY[]     = {18};
static const uint8_t F_READ[]    = {19,20,20,20,20,20,20,20,20,19};
/* fps/loop values are sensible defaults for this frame count, not
   pulled from any spec — tune freely to taste. */
const AnimClip HERO_CLIPS[ANIM_COUNT] = {
    { F_IDLE,    8,  1,  1 },   // idle
    { F_RUN,     6, 20,  1 },   // run
    { F_ATTACK,  4, 15,  0 },   // attack
    { F_DIE,     6, 20,  0 },   // die
    { F_OPERATE, 4,  8,  0 },   // operate
    { F_READ,   10, 20,  0 },   // read
    // fly 
};
 
/*═══════════════════════════════════════════════════════════════
    Initialization
═══════════════════════════════════════════════════════════════*/
void player_init(Player *p, const Level *lvl, HeroClass cls){
    Point ep = idx_to_xy(lvl->entrance);
    p->x           = ep.x;
    p->y           = ep.y;
    p->cls         = cls;
    p->facing      = FACE_RIGHT;
    p->anim        = ANIM_IDLE;
    p->frame       = 0;
    p->frame_clock = 0.0f;
    p->anim_done   = 0;

    p->num_inventory = 0;
    p->gold          = 0;

    p->weapon = item_stack_none();
    p->armor  = item_stack_none();
    p->ring1  = item_stack_none();
    p->ring2  = item_stack_none();
}

/*═══════════════════════════════════════════════════════════════
    Animation
═══════════════════════════════════════════════════════════════*/
void player_play_anim(Player *p, AnimState anim){
    if (p->anim == ANIM_OPERATE && !p->anim_done)
        return;
    if (p->anim == anim)
        return;

    p->anim = anim;
    p->frame = 0;
    p->frame_clock = 0.0f;
    p->anim_done = 0;
}

void player_animate(Player *p, float dt) {
    const AnimClip *clip = &HERO_CLIPS[p->anim];
    if (p->anim_done || clip->fps <= 0) return;
 
    p->frame_clock += dt;
    float step = 1.0f / (float)clip->fps;
 
    while (p->frame_clock >= step) {
        p->frame_clock -= step;
        p->frame++;
        if (p->frame >= clip->count) {
            if (clip->loop) {
                p->frame = 0;
            } else {
                p->frame = clip->count - 1; /* hold on the last frame */
                p->anim_done = 1;
                break;
            }
        }
    }
 
    if (p->anim_done) {
        switch (p->anim) {
            //case ANIM_RUN:
            case ANIM_ATTACK:
            case ANIM_OPERATE:
            case ANIM_READ:
                player_play_anim(p, ANIM_IDLE);
                break;
            default:
                break; /* ANIM_DIE holds its final frame on purpose */
        }
    }
}

int player_is_busy(const Player *p) {
    switch (p->anim) {
    case ANIM_OPERATE:
    case ANIM_ATTACK:
    case ANIM_READ:
    case ANIM_DIE:
        return !p->anim_done;

    default:
        return 0;
    }
}
/*═══════════════════════════════════════════════════════════════
    Inventory helpers 
═══════════════════════════════════════════════════════════════*/
/* Active heap at (x,y), creating an empty HEAP_NORMAL one there if
   none exists yet. NULL only if the level's heap table is full
   (MAX_LEVEL_ITEMS). */
static Heap *heap_at_for_drop(Level *lvl, int x, int y) {
    Heap *h = level_heap_at(lvl, x, y);
    if (h) return h;
    return level_create_heap(lvl, x, y, HEAP_NORMAL);
}

static ItemStack *equip_slot_ptr(Player *p, EquipSlot slot) {
    switch (slot) {
        case EQUIP_WEAPON: return &p->weapon;
        case EQUIP_ARMOR:  return &p->armor;
        case EQUIP_RING1:  return &p->ring1;
        case EQUIP_RING2:  return &p->ring2;
        default:           return NULL;
    }
}

static void inventory_remove_slot(Player *p, int slot) {
    for (int i = slot; i < p->num_inventory - 1; i++) {
        p->inventory[i] = p->inventory[i + 1];
    }
    p->num_inventory--;
}

static int item_type_cmp(const void *a, const void *b) {
    ItemType ta = ((const ItemStack *)a)->type;
    ItemType tb = ((const ItemStack *)b)->type;
    return (int)ta - (int)tb;
}

static int player_use_key(Player *p, ItemType key) {
    int slot = player_find_item(p, key);
    if (slot < 0)
        return 0;
    return player_remove_item(p, slot, 1);
}
/*═══════════════════════════════════════════════════════════════
    Inventory management
═══════════════════════════════════════════════════════════════*/
int player_add_stack(Player *p, ItemStack stack) {
    if (stack.type == ITEM_GOLD) {
        p->gold += stack.quantity;
        return 1;
    }
    if (ITEM_DB[stack.type].stackable) {
        for (int i = 0; i < p->num_inventory; i++) {
            if (p->inventory[i].type == stack.type) {
                p->inventory[i].quantity += stack.quantity;
                return 1;
            }
        }
    }
    if (p->num_inventory >= BACKPACK_SLOTS) return 0;

    p->inventory[p->num_inventory] = stack;
    p->num_inventory++;
    return 1;
}

int player_add_item(Player *p, ItemType type, int quantity) {
    ItemStack stack;
    item_stack_init(&stack, type, quantity);
    return player_add_stack(p, stack);
}

int player_remove_item(Player *p, int slot, int quantity) {
    if (slot < 0 || slot >= p->num_inventory || quantity <= 0) return 0;
    if (p->inventory[slot].quantity < quantity) return 0;

    p->inventory[slot].quantity -= quantity;

    if (p->inventory[slot].quantity == 0) {
        inventory_remove_slot(p, slot);
    }
    return 1;
}

int player_find_item(const Player *p, ItemType type) {
    for (int i = 0; i < p->num_inventory; i++) {
        if (p->inventory[i].type == type) return i;
    }
    return -1;
}

void player_sort_inventory(Player *p) {
    qsort(p->inventory, (size_t)p->num_inventory, sizeof(ItemStack), item_type_cmp);
}
/*═══════════════════════════════════════════════════════════════
    Equipment
═══════════════════════════════════════════════════════════════*/
int player_equip_item(Player *p, int slot) {
    if (slot < 0 || slot >= p->num_inventory) return 0;

    ItemStack incoming = p->inventory[slot];
    ItemCategory cat = ITEM_DB[incoming.type].cat;

    EquipSlot target;
    if (cat == ITEMCAT_WEAPON) {
        target = EQUIP_WEAPON;
    } else if (cat == ITEMCAT_ARMOR) {
        target = EQUIP_ARMOR;
    } else if (cat == ITEMCAT_RING) {
        target = (p->ring1.type == ITEM_NONE) ? EQUIP_RING1
               : (p->ring2.type == ITEM_NONE) ? EQUIP_RING2
               : EQUIP_RING1;
    } else {
        return 0; /* not an equipable category */
    }

    ItemStack *dst = equip_slot_ptr(p, target);

    /* A cursed item stays welded to the hand — can't be swapped out
       to make room for something else. Mirrors EquipableItem.
       doUnequip()'s refusal, checked on the raw cursed flag (not
       cursed_known) same as the original: you don't get advance
       warning, the failed swap *is* how you find out. */
    if (dst->type != ITEM_NONE && dst->cursed) {
        return 0;
    }

    /* Take one unit out of the backpack first, so a swap has the
       slot it needs to put the old gear back into. */
    if (!player_remove_item(p, slot, 1)) return 0;

    if (dst->type != ITEM_NONE) {
        if (!player_add_stack(p, *dst)) {
            /* No room for the item being swapped out — put the new
               one back and bail, matching collect() failing cleanly. */
            player_add_stack(p, incoming);
            return 0;
        }
    }

    *dst = incoming;
    dst->quantity = 1; /* equipped gear is always exactly one unit, even
                           if it came off a stack of stackable weapons */

    /* Equipping is how you find out whether it's cursed — win or
       lose — see KindOfWeapon.doEquip() setting cursedKnown = true
       unconditionally, before checking `cursed`. */
    dst->cursed_known = 1;

    return 1;
}

int player_unequip(Player *p, EquipSlot slot, Level *lvl) {
    ItemStack *src = equip_slot_ptr(p, slot);
    if (!src || src->type == ITEM_NONE) return 0;

    /* Cursed gear refuses to come off at all — same as the original,
       checked on the raw cursed flag rather than cursed_known. */
    if (src->cursed) return 0;

    ItemStack stack = *src;
    stack.quantity = 1;

    if (player_add_stack(p, stack)) {
        *src = item_stack_none();
        return 1;
    }

    if (lvl) {
        Heap *h = heap_at_for_drop(lvl, p->x, p->y);
        if (h && heap_add_stack(h, stack)) {
            *src = item_stack_none();
            return 1;
        }
    }

    return 0; /* nowhere to put it — stays equipped rather than lost */
}

/*═══════════════════════════════════════════════════════════════
    Ground interaction
═══════════════════════════════════════════════════════════════*/
int player_pickup(Player *p, Level *lvl) {
    Heap *h = level_heap_at(lvl, p->x, p->y);
    if (!h) return 0;

    if ((h->type == HEAP_LOCKED_CHEST || h->type == HEAP_CRYSTAL_CHEST)&& !player_use_key(p, ITEM_KEY_GOLDEN))
        return 0;
    
    if (h->type != HEAP_NORMAL && h->type != HEAP_FOR_SALE) {
        h->type = HEAP_NORMAL;
        return 0;
    }
    if (h->count == 0) return 0;

    ItemStack top = h->items[0];
    if (!player_add_stack(p, top)) {
        return 0; /* backpack full — leave it on the ground, don't lose it */
    }

    for (int i = 1; i < h->count; i++) {
        h->items[i - 1] = h->items[i];
    }
    h->count--;
    if (h->count == 0) {
        *h = lvl->heaps[--lvl->num_heaps]; /* keep the heap table compact */
    }

    return 1;
}

int player_drop_item(Player *p, Level *lvl, int slot) {
    if (slot < 0 || slot >= p->num_inventory) return 0;

    Heap *h = heap_at_for_drop(lvl, p->x, p->y);
    if (!h) return 0;

    ItemStack stack = p->inventory[slot];
    if (!heap_add_stack(h, stack)) return 0;

    inventory_remove_slot(p, slot);
    return 1;
}

/*═══════════════════════════════════════════════════════════════
    World interaction
═══════════════════════════════════════════════════════════════*/
int player_move(Player *p, Level *lvl, int dx, int dy) {
    int nx = p->x + dx;
    int ny = p->y + dy;

    if (!in_bounds(nx, ny)) return 0;
    if (player_is_busy(p)) return 0;

    uint8_t tile = lvl->map[xy_to_idx(nx, ny)];
    if (!(gTileFlags[tile] & TF_PASSABLE)) return 0;
 
    /* Open closed doors on contact (PD behaviour) */
    int idx = xy_to_idx(nx, ny);
    switch (tile) {
    case TILE_DOOR:
        lvl->map[idx] = TILE_OPEN_DOOR;
        break;
    case TILE_LOCKED_DOOR:
        if (!player_use_key(p, ITEM_KEY_IRON))
            return 0;

        lvl->map[idx] = TILE_OPEN_DOOR;
        player_play_anim(p, ANIM_OPERATE);
        return 1; 
    case TILE_LOCKED_EXIT:
        //if (!player_use_key(p, ITEM_KEY_SKELETON)) return 0; /*TODO: REMOVE COMMENT AFTER ADDING BOSSES WITH KEY DROPS */
        lvl->map[idx] = TILE_UNLOCKED_EXIT;
        player_play_anim(p, ANIM_OPERATE);
        return 1;
    }
    
    /* Only flip on horizontal movement — keep prior facing on vertical-only
    steps, same convention as the original game. */
    if (dx != 0) p->facing = (dx < 0) ? FACE_LEFT : FACE_RIGHT;

    p->x = nx;
    p->y = ny;
    return 1;
}

int player_operate(Player *p, Level *lvl) {
    Heap *h = level_heap_at(lvl, p->x, p->y);
    if (!h)
        return 0;
    if (h->type == HEAP_NORMAL || h->type == HEAP_FOR_SALE)
        return 0;
    if ((h->type == HEAP_LOCKED_CHEST || h->type == HEAP_CRYSTAL_CHEST)&& !player_use_key(p, ITEM_KEY_GOLDEN))
        return 0;
    player_play_anim(p, ANIM_OPERATE);
    h->type = HEAP_NORMAL;
    return 1;
}