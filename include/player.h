#pragma once
#include "level.h"
#include "bag.h"
#include "char.h"
#include "sprite_db.h"

#define HERO_STARTING_HP         2000 //test, original was 20
#define HERO_STARTING_STR        100 //test, original was 10
#define HERO_STARTING_ATTACK     10
#define HERO_STARTING_DEFENSE     5
#define HERO_STARTING_AWARENESS  0.1f

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
   Char base;
   HeroClass cls;
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

   HeroSubClass sub_class;

   int STR;
   int weakened;

   int   lvl;
   int   exp;
   float awareness;

   int base_attack_skill;
   int base_defense_skill;

   int ranged_attack;

   int resting;

   Char *attack_target;
} Player;

void player_animate(Player *p, float dt);
void player_set_moving(Player *p, int moving);

int player_str(const Player *p);
int player_max_exp(const Player *p);

int player_has_bag(const Player *p, BagType type);
int player_has_item(const Player *p, ItemType type);

int player_drop_from_bag(Player *p, Level *lvl, Bag *bag, int slot);

ItemStack *equip_slot_ptr(Player *p, EquipSlot slot);

void player_sort_inventory(Player *p);
void player_consume_item(Player *p, Bag *bag, int slot);

int player_equip_item(Player *p, Bag *bag, int slot, int spend_time);
int player_equip_ring(Player *p, Bag *bag, int slot, EquipSlot target, int spend_time);
int player_unequip(Player *p, EquipSlot slot, Level *lvl);

int player_pickup(Player *p, Level *lvl);
int player_operate(Player *p, Level *lvl);

void player_rest(Player *p, int till_healthy);

int player_move(Player *p, Level *lvl, int dx, int dy);
int player_bump(Player *p, Level *lvl, int dx, int dy);


void player_init(Player *p, const Level *lvl, HeroClass cls);