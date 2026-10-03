#pragma once
#include "actor.h"
#include "buff.h"

/* ══════════════════════════════════════════════════════════
   CHAR port of Char.java
   ══════════════════════════════════════════════════════════ */
typedef struct Level Level;
typedef struct Char Char;

typedef enum {
    DMGSRC_GENERIC = 0,
    DMGSRC_CHAR,     /* mirrors the original's broad `src instanceof Char` check in damage() */
    DMGSRC_FIRE,     /* Burning ticks; also stands in for the Fire enchantment and WandOfFirebolt (Elemental is immune to all three) */
    DMGSRC_POISON,
    DMGSRC_ACID,
    DMGSRC_TRAP,
    DMGSRC_FALL,
    DMGSRC_TOXIC_GAS,       /* ToxicGas blob */
    DMGSRC_DEATH,           /* Death weapon enchantment's instakill */
    DMGSRC_LIGHTNING,       /* LightningTrap.Electricity: the trap, Shock enchantment, WandOfLightning, Potential glyph */
    DMGSRC_PSIONIC,         /* ScrollOfPsionicBlast */
    DMGSRC_DISINTEGRATION,  /* WandOfDisintegration */
    DMGSRC_COUNT
} DamageSource;

typedef struct CharVTable {
   int         (*attack_skill)(Char *self, Char *target);         /* default: 0 */
   int         (*defense_skill)(Char *self, Char *enemy);          /* default: 0 */
   int         (*dr)(Char *self);                                  /* default: 0 */
   int         (*damage_roll)(Char *self);                         /* default: 1 */
   float       (*speed)(Char *self);                                /* default: baseSpeed, halved by Cripple */
   int         (*attack_proc)(Char *self, Char *enemy, int damage);    /* default: passthrough */
   int         (*defense_proc)(Char *self, Char *attacker, int damage); /* default: passthrough */
   uint32_t    (*resistances)(Char *self);                         /* default: 0 — bitmask of 1u << DamageSource; a hit from one rolls 0..dmg. Mobs: MOB_DB .dmg_resist */
   uint32_t    (*immunities)(Char *self);                          /* default: 0 — bitmask of 1u << DamageSource; a hit from one does 0. Mobs: MOB_DB .dmg_immune */
   int         (*buff_immune)(Char *self, BuffType type);          /* default: never immune — mirrors Buff.attachTo()'s immunity check. Mobs answer from MOB_DB (.buff_immune). Hero overrides this for GasesImmunity (blocks Paralysis/Vertigo) — mirrors Hero.immunities() swapping in GasesImmunity.IMMUNITIES while that buff is up */
   float       (*hunger_tick_scale)(Char *self);                   /* default: 1.0 — multiplies the interval between Hunger buff ticks (bigger = hungry more slowly); Hero overrides this for Rogue's 1.2x from Hunger.act() */
   int         (*stealth)(Char *self);                             /* default: 0 */
   void        (*on_damaged)(Char *self, int dmg_taken, void *src, DamageSource src_type);
   void        (*on_buff_added)(Char *self, Buff *buff);
   void        (*on_buff_removed)(Char *self, Buff *buff);
   void        (*die)(Char *self, void *src, DamageSource src_type);
   int         (*on_turn)(Char *self);
} CharVTable;

extern const CharVTable CHAR_DEFAULT_VTABLE;

typedef enum {
    CHAR_KIND_GENERIC = 0,
    CHAR_KIND_HERO,
    CHAR_KIND_MOB,
    CHAR_KIND_NPC,
} CharKind;

struct Char {
   Actor       base;
   const CharVTable *vt;
   CharKind    kind;

   int pos;                
   const char *name;

   int HT;
   int HP;

   float base_speed;

   int paralysed;
   int rooted;
   int flying;
   int invisible;

   int view_distance;

   Buff buffs[MAX_CHAR_BUFFS];

   int   anim_locked;
   int   incoming_hits;
   Char *swing_target;
};

/* Visual state */
typedef enum {
    CHAR_VIS_BURNING     = 1 << 0,
    CHAR_VIS_LEVITATING  = 1 << 1,
    CHAR_VIS_INVISIBLE   = 1 << 2,
    CHAR_VIS_PARALYSED   = 1 << 3,
    CHAR_VIS_FROZEN      = 1 << 4,
    CHAR_VIS_ILLUMINATED = 1 << 5,
} CharVisualFlags;

uint32_t char_visual_flags(const Char *self);


Char *char_find_at(int pos);

void char_clear(void);
void char_unregister(Char *self);
void char_destroy(Char *self);
void char_die_default(Char *self, void *src, DamageSource src_type);

void char_spend(Char *self, float time);

void char_init(Char *self, const CharVTable *vt, CharKind kind, int pos, int max_hp);

void char_set_active_level(Level *lvl);
Level *char_active_level(void);

int char_add(Char *self);
int char_add_delayed(Char *self, float delay);

int char_is_alive(const Char *self);

int random_free_neighbour(int pos, int diagonals);
void char_move(Char *self, Level *lvl, int step);

int char_hit_roll(Char *attacker, Char *defender, int magic);
void char_damage(Char *self, int dmg, void *src, DamageSource src_type);

float char_speed(Char *self);

typedef struct {
    int hit;                  /* 1 = landed, 0 = missed */
    int damage_dealt;         /* effective damage after dr/procs; 0 if missed */
    int enemy_died;           /* 1 if this attack just brought the enemy to <= 0 HP */
} CharAttackResult;

CharAttackResult char_attack(Char *self, Char *enemy);

int char_is_busy(const Char *self);

void char_begin_attack_anim(Char *self, Char *target);
void char_end_attack_anim(Char *self);