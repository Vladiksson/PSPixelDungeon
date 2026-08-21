#pragma once
#include <stdint.h>
#include "actor.h"

/* ══════════════════════════════════════════════════════════
   CHAR port of Char.java
   ══════════════════════════════════════════════════════════ */
typedef struct Level Level;
typedef struct Char Char;
typedef struct Buff Buff;

/* ── Sprite facing/animation — shared by Player (player.h) and Mob
   (mob.h), so it lives here rather than in either header ──────── */
typedef enum {
    FACE_RIGHT = 0,
    FACE_LEFT  = 1
} Facing;

typedef struct {
    const uint8_t *frames; /* column indices into the sheet, in play order */
    int count;             /* number of entries in frames[]                */
    int fps;               /* playback speed, frames per second            */
    int loop;              /* 1 = repeat, 0 = play once and hold last frame */
} AnimClip;

typedef enum {
    DMGSRC_GENERIC = 0,
    DMGSRC_CHAR,     /* mirrors the original's broad `src instanceof Char` check in damage() */
    DMGSRC_FIRE,
    DMGSRC_POISON,
    DMGSRC_ACID,
    DMGSRC_TRAP,
    DMGSRC_FALL,
    DMGSRC_COUNT
} DamageSource;

/* Flat stand-in for Buff.java + subclasses. BUFF_NONE marks an empty slot in Char.buffs[]. */
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
    BUFF_TYPE_COUNT
} BuffType;

/* Original per-type visual/text hookup (Char.add()/remove()/
   updateSpriteState()'s instanceof chain), kept here for reference —
   char_visual_flags() 
     BUFF_POISON        status text "poisoned" + a particle burst
     BUFF_AMOK           status text "amok"
     BUFF_SLOW           status text "slowed"
     BUFF_MIND_VISION    status text "mind" then "vision" (two lines)
     BUFF_PARALYSIS      status text "paralysed"   (+ CHAR_VIS_PARALYSED)
     BUFF_TERROR         status text "frightened"
     BUFF_ROOTS          status text "rooted"
     BUFF_CRIPPLE        status text "crippled"
     BUFF_BLEEDING       status text "bleeding"
     BUFF_VERTIGO        status text "dizzy"
     BUFF_SLEEP          forces an idle animation, no status text
     BUFF_BURNING        CHAR_VIS_BURNING,     no status text
     BUFF_LEVITATION     CHAR_VIS_LEVITATING,  no status text
     BUFF_FROST          CHAR_VIS_FROZEN,      no status text
     BUFF_INVISIBILITY   CHAR_VIS_INVISIBLE + status text "invisible"
                         (skipped specifically for BUFF_SHADOWS)
     BUFF_LIGHT          CHAR_VIS_ILLUMINATED, no status text
*/

struct Buff {
    Actor    base;     
    BuffType type;
    Char    *target;
    float    duration;  /* countdown in Actor-clock units*/
    int      data;      /* generic payload slot — e.g. Charm's caster id would live here */
};

#define MAX_CHAR_BUFFS 16

typedef struct CharVTable {
   int         (*attack_skill)(Char *self, Char *target);         /* default: 0 */
   int         (*defense_skill)(Char *self, Char *enemy);          /* default: 0 */
   const char *(*defense_verb)(Char *self);                        /* default: "dodged" */
   int         (*dr)(Char *self);                                  /* default: 0 */
   int         (*damage_roll)(Char *self);                         /* default: 1 */
   int         (*attack_proc)(Char *self, Char *enemy, int damage);    /* default: passthrough */
   int         (*defense_proc)(Char *self, Char *attacker, int damage); /* default: passthrough */
   uint32_t    (*resistances)(Char *self);                         /* default: 0 */
   uint32_t    (*immunities)(Char *self);                          /* default: 0 */
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

/* ── Lifecycle ────────────────────────────────────────────── */

void char_init(Char *self, const CharVTable *vt, CharKind kind, int pos, int max_hp);

int  char_add(Char *self);
int  char_add_delayed(Char *self, float delay);

void   char_set_active_level(Level *lvl);
Level *char_active_level(void);

int  char_is_alive(const Char *self);

/* HP -> 0, Mirrors Char.destroy(). */
void char_destroy(Char *self);

/* Mirrors Char.die(Object)  */
void char_die(Char *self, void *src, DamageSource src_type);
void char_die_default(Char *self, void *src, DamageSource src_type);

void char_clear(void);

/* ── Position / movement ─────────────────────────────────────── */

Char *char_find_at(int pos);

void  char_move(Char *self, Level *lvl, int step);

int   char_distance(const Char *self, const Char *other);

/* ── Combat ───────────────────────────────────────────────────── */

typedef struct {
    int hit;                  /* 1 = landed, 0 = missed */
    int damage_dealt;         /* effective damage after dr/procs; 0 if missed */
    int enemy_died;           /* 1 if this attack just brought the enemy to <= 0 HP */
    const char *defense_verb; /* only set when hit == 0 */
} CharAttackResult;

/* Mirrors Char.attack(Char) */
CharAttackResult char_attack(Char *self, Char *enemy);

/* Mirrors the static Char.hit(Char, Char, boolean). */
int char_hit_roll(Char *attacker, Char *defender, int magic);

/* Mirrors Char.damage(int, Object). src_type stands in for the original's `src.getClass()` */
void char_damage(Char *self, int dmg, void *src, DamageSource src_type);

float char_speed(Char *self);              /* mirrors Char.speed() */
void  char_spend(Char *self, float time);  /* mirrors Char.spend(float) — call this, not actor_spend(), from a Char's own act()/on_turn */

/* ── Turn-lock / busy ─────────────────────────────────────────────
   self's own subtype code (player.c/mob.c) keeps anim_locked in sync
   with its own clip state; char_is_busy() folds that together with
   incoming_hits for callers that also need to wait out an attacker's
   swing (currently just player_is_busy()). */
int  char_is_busy(const Char *self);

void char_begin_attack_anim(Char *self, Char *target); /* call when self's own attack clip starts */
void char_end_attack_anim(Char *self);                 /* call when that clip's anim_done fires */

/* ── Buffs ────────────────────────────────────────────────────── */
Buff *char_apply_buff(Char *self, BuffType type, float duration);

void  char_remove_buff(Char *self, Buff *buff);
void  char_remove_buffs_of_type(Char *self, BuffType type);

Buff *char_buff(Char *self, BuffType type);
int   char_buffs_of_type(Char *self, BuffType type, Buff **out, int max_out);

int   char_is_charmed_by(Char *self, Char *ch);


/* ── Per-turn plumbing ────────────────────────────────────────── */

/* The shared body every Char's act() runs before its vt->on_turn —
   mirrors Char.java's own act() override (an FOV update, then
   return false). The FOV part is left as an optional hook rather
   than a direct call into a fog/FOV system: your fov.h/fog_update()
   takes (Fog*, Level*, x, y) on a fixed x,y and is called once a
   frame centered on the player already, in main.c — a different
   (simpler) concept from the original's per-Char, per-turn FOV
   update that also powers mob "can I see the hero" checks, so this
   is left as its own hook rather than guessing at a call into
   fog_update(). */
void char_set_turn_started_hook(void (*fn)(Char *self));
int  char_base_act(Char *self);

void char_on_motion_complete(Char *self);
void char_on_attack_complete(Char *self);
void char_on_operate_complete(Char *self);






void char_unregister(Char *self);