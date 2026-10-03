#pragma once
#include "char.h"
#include "sprite_db.h"

typedef enum {
    MOB_RAT = 0,
    MOB_GNOLL,
    MOB_CRAB,
    MOB_SWARM,
    MOB_SKELETON,
    MOB_THIEF,
    MOB_SHAMAN,
    MOB_BAT,
    MOB_BRUTE,
    MOB_SPINNER,
    MOB_ELEMENTAL,
    MOB_MONK,
    MOB_WARLOCK,
    MOB_GOLEM,
    MOB_SUCCUBUS,
    MOB_EYE,
    MOB_SCORPIO,
    /* elite */
    MOB_ALBINO,               /* elite Rat */
    MOB_BANDIT,               /* elite Thief */
    MOB_SHIELDED,             /* elite Brute */
    MOB_SENIOR,               /* elite Monk */
    MOB_ACIDIC,               /* elite Scorpio */
    /* bosses */
    MOB_GOO,
    MOB_TENGU,
    MOB_DM300,
    MOB_KING,
    MOB_KING_UNDEAD,          /* King's summoned "undead dwarf" army */
    MOB_YOG,                  /* the body */
    MOB_YOG_ROTTING_FIST,
    MOB_YOG_BURNING_FIST,
    MOB_YOG_LARVA,
    /* unique */
    MOB_FETID_RAT,
    MOB_MIMIC,
    MOB_STATUE,
    MOB_WRAITH,
    MOB_PIRANHA,
    MOB_CURSE_PERSONIFICATION,
    MOB_KIND_COUNT
} MobKind;

typedef enum {
    MOB_ASLEEP,
    MOB_WANDERING,
    MOB_HUNTING,
    MOB_FLEEING,
    MOB_PASSIVE,
} MobAiState;

/* ══════════════════════════════════════════════════════════
   ══════════════════════════════════════════════════════════ */

struct Mob;
struct MobStats;

typedef uint64_t BuffMask;
#define BUFFBIT(t)  ((BuffMask)1 << (t))
#define DMGBIT(s)   (1u << (s))

typedef enum {
    MOBHIT_PROLONG = 0,        /* Buff.prolong(): char_apply_buff(), keeps the longer of old/new duration */
    MOBHIT_SET_DURATION,       /* Buff.affect(..).set(x): force the buff's duration to exactly x (Poison) */
    MOBHIT_LEVEL_FROM_DAMAGE,  /* Buff.affect(..).set(damage): the hit's damage becomes the buff's level (Bleeding) */
    MOBHIT_TAG_ATTACKER,       /* Buff.affect(..).object = id(): remember who applied it (Charm) */
} MobHitMode;

typedef struct {
    uint8_t  one_in;      /* 0 = unused slot (ends the list); N = fires when rng_int(N) == 0, so 1 = every hit */
    uint8_t  flee;        /* the attacker switches to FLEEING when it fires (Spinner) */
    uint8_t  mode;        /* MobHitMode */
    BuffType buff;
    float    dur_min;     /* the duration, or the low end of the roll... */
    float    dur_max;     /* ...if > 0: inclusive integer roll dur_min..dur_max (Random.IntRange) */
} MobBuffProc;

#define MOB_MAX_HIT_PROCS 2

typedef int  (*MobAttackProcFn)(struct Mob *self, Char *enemy, int dmg);
typedef int  (*MobDefenseProcFn)(struct Mob *self, Char *attacker, int dmg);
typedef void (*MobDeathFn)(struct Mob *self, void *src, DamageSource src_type);
typedef void (*MobBuffAddedFn)(struct Mob *self, Buff *buff);
typedef void (*MobScaleFn)(struct MobStats *s, int depth);

typedef enum {
    /* Fleeing.nowhereToRun(): cornered -> turn and fight, unless the Terror buff is what's driving it. Thief, Bandit, Spinner. */
    MOBF_FLEE_CORNERED_FIGHTS  = 1u << 0,
    /* Spinner.act(): FLEEING -> HUNTING as soon as its enemy is in view and no longer poisoned. */
    MOBF_FLEE_UNTIL_UNPOISONED = 1u << 1,
    /* Statue.damage(): PASSIVE -> HUNTING when hit. */
    MOBF_HIT_WAKES_PASSIVE     = 1u << 2,
} MobFlags;

typedef struct MobStats {
    const char *name;
    int   ht;
    int   attack_skill;
    int   defense_skill;
    int   dr;
    int   dmg_min, dmg_max;
    int   view_distance;
    int   xp_reward;
    float base_speed;
    float attack_delay;
    int   flying;
    MobAiState initial_state;

    MobScaleFn scale;

    BuffMask buff_immune;   /* BUFFBIT(BUFF_x) | ... */
    uint32_t dmg_immune;    /* DMGBIT(DMGSRC_x) | ... */
    uint32_t dmg_resist;

    MobBuffProc      on_hit[MOB_MAX_HIT_PROCS];
    MobAttackProcFn  attack_proc;
    MobDefenseProcFn defense_proc;
    MobDeathFn       on_death;
    MobBuffAddedFn   on_buff_added;

    /* Brute.damage(): once HP < HT / enrage_hp_div the mob latches "enraged" (costs it a
       tick) and rolls enrage_dmg_min..enrage_dmg_max instead. 0 = never enrages. */
    int      enrage_hp_div;
    int      enrage_dmg_min, enrage_dmg_max;

    uint32_t flags;         /* MobFlags */
} MobStats;

void mob_stats_resolve(MobStats *out, MobKind kind, int depth);

/* ── Rendering / animation ──────────────────────────────────────── */
typedef enum {
    MOB_ANIM_IDLE = 0,
    MOB_ANIM_RUN,
    MOB_ANIM_ATTACK,
    MOB_ANIM_ZAP,
    MOB_ANIM_DIE,
    MOB_ANIM_COUNT
} MobAnimState;

typedef struct {
    int      frame_w, frame_h;
    AnimClip clips[MOB_ANIM_COUNT];
} MobSpriteInfo;

extern const MobSpriteInfo MOB_SPRITES[MOB_KIND_COUNT];
extern const MobTexId MOB_TEX_FOR_KIND[MOB_KIND_COUNT];