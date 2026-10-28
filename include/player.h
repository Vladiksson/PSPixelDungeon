#pragma once
#include "level.h"
#include "bag.h"
#include "char.h"
#include "sprite_db.h"
#include "ballistica.h"

#define HERO_STARTING_HP         2000 //test, original was 20
#define HERO_STARTING_STR        100 //test, original was 10
#define HERO_STARTING_ATTACK     10
#define HERO_STARTING_DEFENSE     5
#define HERO_STARTING_AWARENESS  0.1f

#define WAND_CHARGE_TURN_INTERVAL 40 //redo

typedef enum {
   HERO_WARRIOR = 0,
   HERO_MAGE,
   HERO_ROGUE,
   HERO_HUNTRESS,
   HERO_CLASS_COUNT
} HeroClass;

typedef enum {
   HERO_SUB_NONE = 0,
   HERO_SUB_BERSERKER,    /* Warrior  */
   HERO_SUB_GLADIATOR,    /* Warrior  */
   HERO_SUB_WARLOCK,      /* Mage     */
   HERO_SUB_BATTLEMAGE,   /* Mage     */
   HERO_SUB_FREERUNNER,   /* Rogue    */
   HERO_SUB_ASSASSIN,     /* Rogue    */
   HERO_SUB_SNIPER,       /* Huntress */
   HERO_SUB_WARDEN,       /* Huntress */
   HERO_SUB_COUNT
} HeroSubClass;

typedef enum {
   ANIM_IDLE = 0,
   ANIM_RUN,
   ANIM_ATTACK,
   ANIM_ZAP,
   ANIM_DIE,
   ANIM_OPERATE,
   ANIM_FLY,
   ANIM_READ,
   ANIM_COUNT
} AnimState;
 
extern const AnimClip HERO_CLIPS[ANIM_COUNT];

typedef enum {
   EQUIP_WEAPON,
   EQUIP_ARMOR,
   EQUIP_RING1,
   EQUIP_RING2,
   EQUIP_SLOT_COUNT
} EquipSlot;

typedef struct {
   ItemType type;
} QuickSlot;

typedef enum {
   EQUIP_FAILED = 0,
   EQUIP_OK = 1,
   EQUIP_RING_SELECT = 2,
} EquipResult;

typedef enum {
   PLAYER_BUMP_BLOCKED    = 0,
   PLAYER_BUMP_MOVED      = 1,
   PLAYER_BUMP_ATTACKED   = 2,
   PLAYER_BUMP_INTERACTED = 3,
} PlayerBumpResult;

typedef struct {
   int       active;
   ItemStack item;
   int       cell;
   float     delay;
} ThrownItem;

typedef struct {
   int        active;
   ItemType   type;     /* re-located by type at land time via player_locate_item(), like the quick slot does */
   int        cell;
   Ballistica path;
} ZapCast;

typedef struct {
   Char base;
   
   HeroClass cls;
   HeroSubClass sub_class;

   Facing    facing;
   AnimState anim;
   int       frame;
   float     frame_clock;
   int       anim_done;

   Bag bags[BAG_TYPE_COUNT];
   int gold;

   /* Equipped gear. */
   ItemStack weapon;
   ItemStack armor;
   ItemStack ring1;
   ItemStack ring2;

   QuickSlot quick_slot;

   int STR;
   int weakened;

   int   lvl;
   int   exp;
   float awareness;

   int base_attack_skill;
   int base_defense_skill;

   int ranged_attack;      /* set only while shoot() resolves a hit; selects thrown.item as the weapon */
   ThrownItem thrown;
   ZapCast    zapping;

   int wand_charge_turns;  /* counts real player turns 0..WAND_CHARGE_TURN_INTERVAL; see player_charge_wands() in player.c */

   int resting;

   int cursor_pos;
   int target_pos;
   Char *attack_target;
} Player;

void player_animate(Player *p, float dt);
void player_set_moving(Player *p, int moving);

int player_str(const Player *p);
int player_max_exp(const Player *p);

int player_has_bag(const Player *p, BagType type);
int player_locate_item(Player *p, ItemType type, Bag **out_bag, int *out_slot);
int player_has_item(const Player *p, ItemType type);

int player_drop_from_bag(Player *p, Level *lvl, Bag *bag, int slot);

ItemStack *equip_slot_ptr(Player *p, EquipSlot slot);

void player_sort_inventory(Player *p);
void player_consume_item(Player *p, Bag *bag, int slot);

int player_equip_item(Player *p, Bag *bag, int slot, int spend_time);
int player_equip_ring(Player *p, Bag *bag, int slot, EquipSlot target, int spend_time);
int player_unequip(Player *p, EquipSlot slot, Level *lvl);

int player_set_quick_slot(Player *p, Bag *bag, int slot);
int player_use_quick_slot(Player *p, Level *lvl);
int player_throw_quick_slot(Player *p, Level *lvl);

int player_throw(Player *p, Level *lvl, Bag *bag, int slot, int dst);
int player_zap(Player *p, Level *lvl, Bag *bag, int slot, int dst);

int fov_radius(Player *player, int depth);

int player_pickup(Player *p, Level *lvl);
int player_operate(Player *p, Level *lvl, int idx);

void player_rest(Player *p);
void player_search(Player *p, Level *lvl);

int player_move(Player *p, Level *lvl, int dx, int dy);
int player_bump(Player *p, Level *lvl, int dx, int dy);

int player_auto_attack(Player *p); //test

void player_init(Player *p, const Level *lvl, HeroClass cls);