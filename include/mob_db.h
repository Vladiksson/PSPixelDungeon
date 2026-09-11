#pragma once
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

typedef struct {
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
} MobStats;

extern const MobStats MOB_DB[MOB_KIND_COUNT];

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
    int      frame_w, frame_h;         /* one frame's size on this kind's own sheet */
    AnimClip clips[MOB_ANIM_COUNT];
} MobSpriteInfo;

extern const MobSpriteInfo MOB_SPRITES[MOB_KIND_COUNT];
extern const MobTexId MOB_TEX_FOR_KIND[MOB_KIND_COUNT];