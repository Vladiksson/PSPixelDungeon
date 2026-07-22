#pragma once
#include "level.h"
#include "bag.h"

#define HERO_W 12
#define HERO_H 15
//#define HERO_SPACING 1
#define TILESET_HERO_COLS    21
#define TILESET_HERO_ROWS     7
#define TILESET_HERO_W       12
#define TILESET_HERO_H       15

#define HERO_ARMOR_TIER 6

/* ── Hero class ───────────────────────────────────────*/
typedef enum {
    HERO_WARRIOR = 0,
    HERO_MAGE,
    HERO_ROGUE,
    HERO_HUNTRESS,
    HERO_CLASS_COUNT
} HeroClass;
 
typedef enum {
    FACE_RIGHT = 0,
    FACE_LEFT  = 1
} Facing;
 
/* ── Animation state ──────────────────────────────────*/
typedef enum {
    ANIM_IDLE = 0,
    ANIM_RUN,
    ANIM_ATTACK,
    ANIM_DIE,
    ANIM_OPERATE,
    ANIM_READ,
    ANIM_COUNT
} AnimState;
 
typedef struct {
    const uint8_t *frames; /* column indices into the sheet, in play order */
    int count;             /* number of entries in frames[]                */
    int fps;                /* playback speed, frames per second            */
    int loop;               /* 1 = repeat, 0 = play once and hold last frame */
} AnimClip;
 
/* Defined once in player.c; render.c reads it to pick the source rect. */
extern const AnimClip HERO_CLIPS[ANIM_COUNT];

/* ── Equip slots ───────────────────────────────────────
   Mirrors Belongings.java's weapon/armor/ring1/ring2 fields. Two ring
   slots exist because the original lets the hero wear two rings at
   once. */
typedef enum {
    EQUIP_WEAPON,
    EQUIP_ARMOR,
    EQUIP_RING1,
    EQUIP_RING2,
    EQUIP_SLOT_COUNT
} EquipSlot;

typedef struct {
    int x, y;              /* current tile position                     */
    HeroClass cls;         /* which sprite sheet to draw                */
    Facing    facing;      /* left/right — drives the horizontal flip   */
    AnimState anim;        /* animation currently playing               */
    int       frame;       /* index into HERO_CLIPS[anim].frames        */
    float     frame_clock; /* seconds accumulated toward the next frame */
    int       anim_done;   /* 1 once a non-looping clip has played out  */

    /* All storage — the backpack included — lives here. bags[BAG_
       BACKPACK] is set up in player_init() and always owned; it's
       what used to be a flat inventory[]/num_inventory pair, and now
       holds everything the four specialty pouches
       (bags[BAG_SEED_POUCH..BAG_KEYRING]) don't. Those four start at
       capacity == 0 (not owned) until player_acquire_bag() grants
       one. See bag.h. */
    Bag bags[BAG_TYPE_COUNT];
    int gold;   /* never an ItemStack — tracked separately, same as before */

    /* Equipped gear. ITEM_NONE means the slot is empty. Equipping
       moves an item out of bags[BAG_BACKPACK] into one of these,
       same as the original where Belongings' weapon/armor/ring1/
       ring2 are separate from the backpack Bag. */
    ItemStack weapon;
    ItemStack armor;
    ItemStack ring1;
    ItemStack ring2;
} Player;
 
/* Place player at level entrance */
void player_init(Player *p, const Level *lvl, HeroClass cls);
 
/* Try to move by (dx,dy). Opens doors on contact.
   Returns 1 if the move succeeded, 0 if blocked. */
int  player_move(Player *p, Level *lvl, int dx, int dy);


/* Advance the current animation by dt seconds. Call once per frame,
   regardless of whether the player moved that frame. */
void player_animate(Player *p, float dt);
 
/* Immediately switch to and (re)start a given animation, e.g. from
   combat code: player_play_anim(&p, ANIM_ATTACK); */
void player_play_anim(Player *p, AnimState anim);

int player_operate(Player *p, Level *lvl);

int player_is_busy(const Player *p);

/* Add a freshly-generated stack of `type` to wherever it belongs: gold
   increments p->gold; picking up a bag Item (Seed Pouch, Scroll
   Holder, Magical Holster, Keyring) activates that satellite
   container instead of taking a slot; anything else lands in the
   owned bag that accepts it, or the backpack otherwise. Returns 0 if
   that landing spot turns out to be full. */
int player_add_item(Player *p, ItemType type, int quantity);

/* Same as player_add_item, but takes an already-built ItemStack and
   adds it as-is — no fresh roll, no reset. This is what
   player_pickup and player_unequip use, so a +2 cursed ring found on
   the ground (or just taken off) keeps being a +2 cursed ring once
   it's put away. */
int player_add_stack(Player *p, ItemStack stack);

/* ── Backpack / ground / equip ─────────────────────────────────
   "Backpack" below always means bags[BAG_BACKPACK] specifically —
   for the specialty pouches, use the Bag-scoped functions in the
   Bags section further down. */

int player_pickup(Player *p, Level *lvl);

int player_drop_item(Player *p, Level *lvl, int slot);

int player_remove_item(Player *p, int slot, int quantity);

int player_equip_item(Player *p, int slot);

int player_unequip(Player *p, EquipSlot slot, Level *lvl);

void player_sort_inventory(Player *p);

/* Backpack slot holding `type`, or -1 if none. Only looks in
   bags[BAG_BACKPACK] — see player_locate_item to also check the
   specialty pouches. */
int player_find_item(const Player *p, ItemType type);

/* ── Bags ─────────────────────────────────────────────────────
   The backpack itself (bags[BAG_BACKPACK]) is reachable through
   these too — see bag.h. */

/* Grant the player a specialty bag (e.g. after picking one up as
   loot): sets its capacity and absorbs any matching items already
   loose in the backpack into it, mirroring Bag.collect() reaching
   back into the container it just joined, in the original game.
   Returns 0 for BAG_BACKPACK (already set up in player_init) or an
   out-of-range type; 1 (no-op, doesn't reset it) if the player
   already owns that bag. */
int player_acquire_bag(Player *p, BagType type);

/* Bag of this type, or NULL if the player doesn't own it yet.
   bags[BAG_BACKPACK] is always owned, so BAG_BACKPACK never
   returns NULL. */
Bag *player_get_bag(Player *p, BagType type);

/* Owned bag that `item_type` should land in — a specialty pouch if
   one accepts it, otherwise the backpack. Used to route newly
   collected items, same as Item.collect() checking the container for
   a sub-Bag whose grab() matches, in the original game. Never NULL. */
Bag *player_bag_for_item(Player *p, ItemType item_type);

/* Find `type` anywhere the player is carrying it — the backpack or
   any owned specialty pouch. Returns 1 and sets *out_bag*out_slot if
   found (out_bag is never NULL on success). Used so e.g. a key filed
   into the Keyring still works for unlocking doors. */
int player_locate_item(Player *p, ItemType type, Bag **out_bag, int *out_slot);

/* Drop one stack out of any bag onto the tile the player is standing
   on. player_drop_item above is the bags[BAG_BACKPACK]-only shorthand
   for this. */
int player_drop_from_bag(Player *p, Level *lvl, Bag *bag, int slot);