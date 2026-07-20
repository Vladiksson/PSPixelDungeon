#pragma once
#include "common.h"
#include "level.h"
#include "level_utils.h"
#include "item.h"

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
   Mirrors Belongings.java's weapon/armor/ring1/ring2 fields (minus
   the backpack, which stays a flat array on Player — see inventory[]
   below). Two ring slots exist because the original lets the hero
   wear two rings at once. */
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

    ItemStack inventory[BACKPACK_SLOTS]; /* backpack (unequipped items)  */
    int       num_inventory;
    int       gold;

    /* Equipped gear. ITEM_NONE means the slot is empty. An equipped
       item is NOT also present in inventory[] — equipping moves it
       out of the backpack, same as the original where Belongings'
       weapon/armor/ring1/ring2 are separate from the backpack Bag. */
    ItemType  weapon;
    ItemType  armor;
    ItemType  ring1;
    ItemType  ring2;
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

/* Add a picked-up stack to the backpack (gold just increments p->gold;
   stackable kinds merge into an existing slot; otherwise a new slot is
   used). Returns 0 if the backpack is full and the item is dropped. */
int player_add_item(Player *p, ItemType type, int quantity);

/* ── Backpack / ground / equip ─────────────────────────────────*/

int player_pickup(Player *p, Level *lvl);

int player_drop_item(Player *p, Level *lvl, int slot);

int player_remove_item(Player *p, int slot, int quantity);

int player_equip_item(Player *p, int slot);

int player_unequip(Player *p, EquipSlot slot, Level *lvl);

void player_sort_inventory(Player *p);

/* Backpack slot holding `type`, or -1 if none. */
int player_find_item(const Player *p, ItemType type);