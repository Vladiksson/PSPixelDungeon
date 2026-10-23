#pragma once
#include "item_db.h"

static inline const char *item_name(ItemType t) {
    return (t >= 0 && t < ITEM_TYPE_COUNT) ? ITEM_DB[t].name : "unknown item";
}

typedef struct {
   ItemType type;
   int      quantity;

   int8_t  level;         /* enchant tier; can go negative (degraded)     */
   uint8_t level_known;   /* has the player seen this instance's level?   */
   uint8_t cursed;        /* is this specific instance cursed?            */
   uint8_t cursed_known;  /* found out yet? equipping always reveals it   */
   int16_t durability;    /* current durability, out of item_max_durability() */
} ItemStack;

typedef enum {
    HEAP_NORMAL,
    HEAP_FOR_SALE,
    HEAP_CHEST,
    HEAP_LOCKED_CHEST,
    HEAP_CRYSTAL_CHEST,
    HEAP_TOMB,
    HEAP_SKELETON,
    HEAP_MIMIC,
    HEAP_HIDDEN
} HeapType;

#define MAX_HEAP_ITEMS 16   
typedef struct {
    HeapType  type;
    ItemStack items[MAX_HEAP_ITEMS];

    int       count;
    int       pos;
    uint8_t   active;
} Heap;

#define MAX_LEVEL_ITEMS 24 //todo mb need bigger numbers for those two???
#define MAX_ITEMS_TO_SPAWN 8

#define BACKPACK_SLOTS   19
#define GOLD_SLOT        BACKPACK_SLOTS
#define MAX_INVENTORY    (BACKPACK_SLOTS + 1)

/* Mirrors Heap.image() */
int heap_sprite_cell(const Heap *h, uint8_t *out_col, uint8_t *out_row);


/* Add `stack` to `heap` */
int heap_add_stack(Heap *heap, ItemStack stack);
 
/* ══════════════════════════════════════════════════════════
   INSTANCE STATE — level / curse / durability
   ══════════════════════════════════════════════════════════
   A from-scratch port of Item.java's level/cursed/durability/
   levelKnown/cursedKnown machinery onto ItemStack. Durability only
   ever ticks down for upgraded (level>0) items, exactly like the
   original's use() — a fresh, un-enchanted item never breaks. */
 
#define ITEM_DURABILITY_WARN_NUM 1
#define ITEM_DURABILITY_WARN_DEN 6   /* matches Item.java's DURABILITY_WARNING_LEVEL = 1/6f */
 
typedef enum {
    ITEM_USE_OK = 0,          /* nothing notable happened                    */
    ITEM_USE_WARN_BREAKING,   /* durability just crossed the low threshold   */
    ITEM_USE_BROKE            /* durability just hit 0 this call             */
} ItemUseResult;
 
/* isBroken()/effectiveLevel()/visiblyUpgraded()/visiblyCursed()/
   visiblyBroken()/isIdentified() — direct ports, kept inline since
   they're pure field reads with no ITEM_DB/RNG dependency. */
static inline int item_is_broken(const ItemStack *s) {
    return s->durability <= 0;
}
static inline int item_effective_level(const ItemStack *s) {
    return item_is_broken(s) ? 0 : s->level;
}
static inline int item_visibly_upgraded(const ItemStack *s) {
    return s->level_known ? s->level : 0;
}
static inline int item_visibly_cursed(const ItemStack *s) {
    return s->cursed && s->cursed_known;
}
static inline int item_visibly_broken(const ItemStack *s) {
    return s->level_known && item_is_broken(s);
}
static inline int item_is_identified(const ItemStack *s) {
    return s->level_known && s->cursed_known;
}
 
/* Empty-slot sentinel for Player's equip slots (see player.h) — an
   all-zero ItemStack with type forced to ITEM_NONE. */
static inline ItemStack item_stack_none(void) {
    ItemStack s;
    memset(&s, 0, sizeof(s));
    s.type = ITEM_NONE;
    return s;
}
 
/* Placeholder max-durability formula */
int item_max_durability(ItemType type, int level);
 
void item_stack_init(ItemStack *stack, ItemType type, int quantity);
void item_stack_init_start(ItemStack *stack, ItemType type, int quantity);

/* fix()/polish() — reset/nudge durability back up. */
void item_fix(ItemStack *stack);
void item_polish(ItemStack *stack);
 
/* upgrade()/degrade() — adjust level and fix(). Matches the original
   exactly: upgrading also clears+reveals curse (a +1 scroll cleanses
   as it enchants); degrading does not touch curse state at all. */
void item_upgrade(ItemStack *stack);
void item_degrade(ItemStack *stack);
//void item_upgrade_n(ItemStack *stack, int n);
//void item_degrade_n(ItemStack *stack, int n);
 
/* use() — wear an upgraded item down by one hit's worth of
   durability (a no-op for level<=0 or already-broken items, exactly
   like the original's guard). Returns what just happened so the
   caller can react (message/sound/etc.) — this port doesn't have a
   message-log system yet, so that reaction lives wherever you end up
   calling this from, not here. */
ItemUseResult item_use(ItemStack *stack);
 
/* identify() — reveal this instance's level+curse, and (for
   POTION/SCROLL/WAND/RING) mark the whole TYPE known for the rest of
   the run. */
void item_identify(ItemStack *stack);


//temporary bullshit variant for using consumable items:
typedef enum {
    ITEM_USE_FAIL,
    ITEM_CONSUME_POTION,
    ITEM_USE_CONSUMED,
} ItemConsumeResult;

ItemConsumeResult item_consume(ItemStack *stack);

/* ══════════════════════════════════════════════════════════
   IDENTIFY — per-TYPE "known" tracking + shuffled unidentified look
   ══════════════════════════════════════════════════════════ */
 
/* Reset all identify state for a new run */
void item_status_reset(void);
 
/* Mark `type` identified. */
void item_status_know(ItemType type);
 
/* Is `type` identified this run? Always true for categories that were never shuffled. */
int item_status_is_known(ItemType type);
 
void item_display_sprite(ItemType type, uint8_t *out_col, uint8_t *out_row);
const char *item_display_name(const ItemStack *stack, char *buf, size_t bufsz);
const char *item_display_desc(const ItemStack *stack);


int weapon_str_req(ItemStack *w);
float weapon_acu(ItemStack *w);
float weapon_dly(ItemStack *w);

void weapon_min_max(ItemStack *w, int *out_min, int *out_max);
int weapon_average_damage(ItemStack *w);

int armor_str_req(ItemStack *a);
int armor_dr(ItemStack *stack);
int get_armor_tier(const ItemStack *stack);

extern const MissileWeaponDef *missile_def(ItemType type);

MissileSpin missile_spin_for(ItemType type);

struct Char;
void missile_weapon_proc(ItemType type, struct Char *defender, int damage);