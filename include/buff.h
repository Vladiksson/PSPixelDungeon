#pragma once
#include "actor.h"

typedef struct Char Char;

typedef enum {
    BUFF_NONE = 0,
    BUFF_POISON,
    BUFF_BLEEDING,
    BUFF_BURNING,
    BUFF_AMOK,
    BUFF_CHARM,
    BUFF_VERTIGO,
    BUFF_CRIPPLE,
    BUFF_FROST,
    BUFF_INVISIBILITY,
    BUFF_SHADOWS,       /* subclasses Invisibility in the original — see char_visual_flags() */
    BUFF_LIGHT,
    BUFF_ROOTS,
    BUFF_SLEEP,
    BUFF_HASTE,         /* original class name is "Speed" — renamed here so it doesn't collide with char_speed() */
    BUFF_SLOW,
    BUFF_LEVITATION,
    BUFF_MIND_VISION,
    BUFF_PARALYSIS,
    BUFF_TERROR,
    BUFF_FURY,          /* Hero.java-specific: Berserker's low-HP damage buff — see hero_damage_roll()/hero_on_damaged() in player.c */
    BUFF_WEAKNESS,      /* Hero-only in the original (hard Hero cast in Weakness.attachTo()) — see hero_on_buff_added()/hero_on_buff_removed() in player.c */
    BUFF_BLINDNESS,
    BUFF_HUNGER,        /* Hero-only in the original (hard Hero cast in Hunger.act()) — duration doubles as the hunger level, see char_hunger_satisfy()/char_hunger_is_starving() */
    BUFF_REGENERATION,  /* Hero-only in the original (hard Hero cast for the isStarving() check) */
    BUFF_OOZE,
    BUFF_BARKSKIN,
    BUFF_AWARENESS,
    BUFF_COMBO,         /* Gladiator-specific: see char_combo_hit() and hero_attack_proc() in player.c */
    BUFF_RAGE,          /* bare marker buff in the original — no attach/detach/act override, and no application site found in the files given here */
    BUFF_GASES_IMMUNITY,/* the immunity it grants is modeled via CharVTable's buff_immune hook (see hero_buff_immune() in player.c), not a per-type check here */
    BUFF_SNIPERS_MARK,  /* Sniper-specific: see char_consume_snipers_mark() in player.c */
    BUFF_TYPE_COUNT
} BuffType;

typedef struct Buff {
    Actor    base;     
    BuffType type;
    Char    *target;
    float    duration;  /* countdown in Actor-clock units*/
    int      data;      /* generic payload slot — e.g. Charm's caster id would live here */
} Buff;

#define BUFF_HP_CAP_SAFE(ht) ((ht) > 0 ? (ht) : 1)

typedef struct {
    int  (*on_attach)(Char *self, Buff *b);
    void (*on_detach)(Char *self, Buff *b);
    int  (*act)(Buff *self);
} BuffVTable;

#define MAX_CHAR_BUFFS 16

/* Constants for buffs*/
#define BURNING_DURATION 8.0f
#define FROST_DURATION   5.0f
#define CRIPPLE_DURATION 10.0f
#define LIGHT_DISTANCE   4 
#define FURY_LEVEL       0.4f
#define SLEEP_SWS        1.5f
#define WEAKNESS_DURATION   40.0f
#define AWARENESS_DURATION  2.0f
#define REGENERATION_DELAY  10.0f
#define HUNGER_STEP      10.0f
#define HUNGER_HUNGRY    260.0f
#define HUNGER_STARVING  360.0f
#define LEVITATION_DURATION 20.0f
#define MIND_VISION_DURATION 20.0f
#define BUFF_GASES_IMMUNITY_DURATION 15.0f
#define INVISIBILITY_DURATION 15.0f

Buff *char_buff(Char *self, BuffType type);

void char_hunger_satisfy(Char *self, float energy);
int  char_hunger_is_starving(Char *self);
void char_barkskin_grant(Char *self, int level);
void char_poison_set(Char *self, float duration);
void char_bleeding_grant(Char *self, int level);
int  char_combo_hit(Char *self, Char *enemy, int damage);
int  char_consume_snipers_mark(Char *self, Char *target);
int char_is_charmed_by(Char *self, Char *ch);

Buff *char_apply_buff(Char *self, BuffType type, float duration);

void char_remove_buff(Char *self, Buff *buff);
void char_remove_buffs_of_type(Char *self, BuffType type);

void char_restore_buffs(Char *self);
