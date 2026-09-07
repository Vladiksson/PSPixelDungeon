#include "animation_db.h"
#include "player.h"
#include "npc.h"
#include "mob_db.h"

/*═══════════════════════════════════════════════════════════════
    Player
═══════════════════════════════════════════════════════════════*/
static const uint8_t F_IDLE[]    = {0,0,0,1,0,0,1,1};
static const uint8_t F_RUN[]     = {2,3,4,5,6,7};
static const uint8_t F_DIE[]     = {8,9,10,11,12,11};
static const uint8_t F_ATTACK[]  = {13,14,15,0};
static const uint8_t F_OPERATE[] = {16,17,16,17};
static const uint8_t F_FLY[]     = {18};
static const uint8_t F_READ[]    = {19,20,20,20,20,20,20,20,20,19};

const AnimClip HERO_CLIPS[ANIM_COUNT] = {
    { F_IDLE,    8,  1,  1 },   // idle
    { F_RUN,     6, 20,  1 },   // run (THERE WAS SPRINT FUNCTION IN ORIGINAL WHICH CHANGE 20-> ~32)
    { F_ATTACK,  4, 15,  0 },   // attack
    { F_ATTACK,  4, 15,  0 },   // ZAP
    { F_DIE,     6, 20,  0 },   // die
    { F_OPERATE, 4,  8,  0 },   // operate
    { F_FLY,     1,  1,  1 },   // fly 
    { F_READ,   10, 20,  0 },   // read
};

/*═══════════════════════════════════════════════════════════════
    Mob
═══════════════════════════════════════════════════════════════*/
/* ── Rat / Albino (elite) / FetidRat (reuses Rat's clips) ────── */
static const uint8_t RAT_IDLE[]      = {0,0,0,1};
static const uint8_t RAT_RUN[]       = {6,7,8,9,10};
static const uint8_t RAT_ATK[]       = {2,3,4,5,0};
static const uint8_t RAT_DIE[]       = {11,12,13,14};

static const uint8_t ALBINO_IDLE[]   = {16,16,16,17};
static const uint8_t ALBINO_RUN[]    = {22,23,24,25,26};
static const uint8_t ALBINO_ATK[]    = {18,19,20,21};
static const uint8_t ALBINO_DIE[]    = {27,28,29,30};

/* ── Gnoll ─────────────────────────────────────────────────────*/
static const uint8_t GNOLL_IDLE[]    = {0,0,0,1,0,0,1,1};
static const uint8_t GNOLL_RUN[]     = {4,5,6,7};
static const uint8_t GNOLL_ATK[]     = {2,3,0};
static const uint8_t GNOLL_DIE[]     = {8,9,10};

/* ── Crab ──────────────────────────────────────────────────────*/
static const uint8_t CRAB_IDLE[]     = {0,1,0,2};
static const uint8_t CRAB_RUN[]      = {3,4,5,6};
static const uint8_t CRAB_ATK[]      = {7,8,9};
static const uint8_t CRAB_DIE[]      = {10,11,12,13};

/* ── Swarm ─────────────────────────────────────────────────────*/
static const uint8_t SWARM_IDLE[]    = {0,1,2,3,4,5};
static const uint8_t SWARM_ATK[]     = {6,7,8,9};
static const uint8_t SWARM_DIE[]     = {10,11,12,13,14};

/* ── Skeleton / King's Undead (own sheet, same layout style) ──── */
static const uint8_t SKELETON_IDLE[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,3};
static const uint8_t SKELETON_RUN[]  = {4,5,6,7,8,9};
static const uint8_t SKELETON_ATK[]  = {14,15,16};
static const uint8_t SKELETON_DIE[]  = {10,11,12,13};

static const uint8_t UNDEAD_IDLE[]   = {0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,3};
static const uint8_t UNDEAD_RUN[]    = {4,5,6,7,8,9};
static const uint8_t UNDEAD_ATK[]    = {14,15,16};
static const uint8_t UNDEAD_DIE[]    = {10,11,12,13};

/* ── Thief / Bandit (elite) ───────────────────────────────────*/
static const uint8_t THIEF_IDLE[]    = {0,0,0,1,0,0,0,0,1};
static const uint8_t THIEF_RUN[]     = {0,0,2,3,3,4};
static const uint8_t THIEF_ATK[]     = {10,11,12,0};
static const uint8_t THIEF_DIE[]     = {5,6,7,8,9};

static const uint8_t BANDIT_IDLE[]   = {21,21,21,22,21,21,21,21,22};
static const uint8_t BANDIT_RUN[]    = {21,21,23,24,24,25};
static const uint8_t BANDIT_ATK[]    = {31,32,33};
static const uint8_t BANDIT_DIE[]    = {25,27,28,29,30};

/* ── Shaman (zap = ranged bolt, same frames as attack) ─────────*/
static const uint8_t SHAMAN_IDLE[]   = {0,0,0,1,0,0,1,1};
static const uint8_t SHAMAN_RUN[]    = {4,5,6,7};
static const uint8_t SHAMAN_ATK[]    = {2,3,0};
static const uint8_t SHAMAN_DIE[]    = {8,9,10};

/* ── Bat ───────────────────────────────────────────────────────*/
static const uint8_t BAT_IDLE[]      = {0,1};
static const uint8_t BAT_ATK[]       = {2,3,0,1};
static const uint8_t BAT_DIE[]       = {4,5,6};

/* ── Brute / Shielded (elite) ─────────────────────────────────*/
static const uint8_t BRUTE_IDLE[]    = {0,0,0,1,0,0,1,1};
static const uint8_t BRUTE_RUN[]     = {4,5,6,7};
static const uint8_t BRUTE_ATK[]     = {2,3,0};
static const uint8_t BRUTE_DIE[]     = {8,9,10};

static const uint8_t SHIELD_IDLE[]   = {21,21,21,22,21,21,22,22};
static const uint8_t SHIELD_RUN[]    = {25,26,27,28};
static const uint8_t SHIELD_ATK[]    = {23,24};
static const uint8_t SHIELD_DIE[]    = {29,30,31};

/* ── Spinner ───────────────────────────────────────────────────*/
static const uint8_t SPINNER_IDLE[]  = {0,0,0,0,0,1,0,1};
static const uint8_t SPINNER_RUN[]   = {0,2,0,3};
static const uint8_t SPINNER_ATK[]   = {0,4,5,0};
static const uint8_t SPINNER_DIE[]   = {6,7,8,9};

/* ── Elemental ─────────────────────────────────────────────────*/
static const uint8_t ELEM_IDLE[]     = {0,1,2};
static const uint8_t ELEM_RUN[]      = {0,1,3};
static const uint8_t ELEM_ATK[]      = {4,5,6};
static const uint8_t ELEM_DIE[]      = {7,8,9,10,11,12,13,12};

/* ── Monk / Senior (elite) — the 50%/30% "kick" follow-up swing
   in source is a second attack sub-clip layered after the first;
   simplified here to just the base attack clip */
static const uint8_t MONK_IDLE[]     = {1,0,1,2};
static const uint8_t MONK_RUN[]      = {11,12,13,14,15,16};
static const uint8_t MONK_ATK[]      = {3,4,3,4};
static const uint8_t MONK_DIE[]      = {1,7,8,8,9,10};

static const uint8_t SENIOR_IDLE[]   = {18,17,18,19};
static const uint8_t SENIOR_RUN[]    = {28,29,30,31,32,33};
static const uint8_t SENIOR_ATK[]    = {20,21,20,21};
static const uint8_t SENIOR_DIE[]    = {18,24,25,25,26,27};

/* ── Warlock (zap = ranged bolt, same frames as attack) ────────*/
static const uint8_t WARLOCK_IDLE[]  = {0,0,0,1,0,0,1,1};
static const uint8_t WARLOCK_RUN[]   = {0,2,3,4};
static const uint8_t WARLOCK_ATK[]   = {0,5,6};
static const uint8_t WARLOCK_DIE[]   = {0,7,8,8,9,10};

/* ── Golem ─────────────────────────────────────────────────────*/
static const uint8_t GOLEM_IDLE[]    = {0,1};
static const uint8_t GOLEM_RUN[]     = {2,3,4,5};
static const uint8_t GOLEM_ATK[]     = {6,7,8};
static const uint8_t GOLEM_DIE[]     = {9,10,11,12,13};

/* ── Succubus ──────────────────────────────────────────────────*/
static const uint8_t SUCCU_IDLE[]    = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,2,2,2,1};
static const uint8_t SUCCU_RUN[]     = {3,4,5,6,7,8};
static const uint8_t SUCCU_ATK[]     = {9,10,11};
static const uint8_t SUCCU_DIE[]     = {12};

/* ── Eye (its "attack" clip triggers the death-ray beam) ───────*/
static const uint8_t EYE_IDLE[]      = {0,1,2};
static const uint8_t EYE_RUN[]       = {5,6};
static const uint8_t EYE_ATK[]       = {4,3};
static const uint8_t EYE_DIE[]       = {7,8,9};

/* ── Scorpio / Acidic (elite) — zap = ranged, same frames as attack */
static const uint8_t SCORPIO_IDLE[]  = {0,0,0,0,0,0,0,0,1,2,1,2,1,2};
static const uint8_t SCORPIO_RUN[]   = {5,5,6,6};
static const uint8_t SCORPIO_ATK[]   = {0,3,4};
static const uint8_t SCORPIO_DIE[]   = {0,7,8,9,10};

static const uint8_t ACIDIC_IDLE[]   = {14,14,14,14,14,14,14,14,15,16,15,16,15,16};
static const uint8_t ACIDIC_RUN[]    = {19,20};
static const uint8_t ACIDIC_ATK[]    = {14,17,18};
static const uint8_t ACIDIC_DIE[]    = {14,21,22,23,24};

/* ── Goo (pump-up/leap sub-states omitted — see mob.c notes) ──── */
static const uint8_t GOO_IDLE[]      = {0,1};
static const uint8_t GOO_ATK[]       = {5,0,6};
static const uint8_t GOO_DIE[]       = {2,3,4};

/* ── Tengu (run doesn't loop in source either — it teleports more
   than it walks) ───────────────────────────────────────────────*/
static const uint8_t TENGU_IDLE[]    = {0,0,0,1};
static const uint8_t TENGU_RUN[]     = {2,3,4,5,0};
static const uint8_t TENGU_ATK[]     = {6,7,7,0};
static const uint8_t TENGU_DIE[]     = {8,9,10,10,10,10,10,10};

/* ── DM-300 ────────────────────────────────────────────────────*/
static const uint8_t DM300_IDLE[]    = {0,1};
static const uint8_t DM300_RUN[]     = {2,3};
static const uint8_t DM300_ATK[]     = {4,5,6};
static const uint8_t DM300_DIE[]     = {0,7,0,7,0,7,0,7,0,7,0,7,8};

/* ── King of Dwarves ───────────────────────────────────────────*/
static const uint8_t KING_IDLE[]     = {0,0,0,0,0,0,0,0,0,0,0,0,1,2};
static const uint8_t KING_RUN[]      = {3,4,5,6,7,8};
static const uint8_t KING_ATK[]      = {9,10,11};
static const uint8_t KING_DIE[]      = {12,13,14,15};

/* ── Yog-Dzewa (run/attack are single held frames — the body
   doesn't actually move or swing, see mob.c's MOB_YOG note) ───── */
static const uint8_t YOG_IDLE[]      = {0,1,2,2,1,0,3,4,4,3,0,5,6,6,5};
static const uint8_t YOG_RUN[]       = {0};
static const uint8_t YOG_ATK[]       = {0};
static const uint8_t YOG_DIE[]       = {0,7,8,9};

/* ── Rotting fist (melee; attack is a single held frame — it's a
   slam, not a swing) ─────────────────────────────────────────── */
static const uint8_t ROTFIST_IDLE[]  = {0,0,1};
static const uint8_t ROTFIST_RUN[]   = {0,1};
static const uint8_t ROTFIST_ATK[]   = {0};
static const uint8_t ROTFIST_DIE[]   = {0,2,3,4};

/* ── Burning fist (ranged-only; "attack" IS the shot) ──────────*/
static const uint8_t BURFIST_IDLE[]  = {0,0,1};
static const uint8_t BURFIST_RUN[]   = {0,1};
static const uint8_t BURFIST_ATK[]   = {0,5,6};
static const uint8_t BURFIST_DIE[]   = {0,2,3,4};

/* ── God's larva ───────────────────────────────────────────────*/
static const uint8_t LARVA_IDLE[]    = {4,4,4,4,4,5,5};
static const uint8_t LARVA_RUN[]     = {0,1,2,3};
static const uint8_t LARVA_ATK[]     = {6,5,7};
static const uint8_t LARVA_DIE[]     = {8};

/* ── Mimic ─────────────────────────────────────────────────────*/
static const uint8_t MIMIC_IDLE[]    = {0,0,0,1,1};
static const uint8_t MIMIC_RUN[]     = {0,1,2,3,3,2,1};
static const uint8_t MIMIC_ATK[]     = {0,4,5,6};
static const uint8_t MIMIC_DIE[]     = {7,8,9};

/* ── Animated statue ──────────────────────────────────────────*/
static const uint8_t STATUE_IDLE[]   = {0,0,0,0,0,1,1};
static const uint8_t STATUE_RUN[]    = {2,3,4,5,6,7};
static const uint8_t STATUE_ATK[]    = {8,9,10};
static const uint8_t STATUE_DIE[]    = {11,12,13,14,15,15};

/* ── Wraith / CursePersonification (reuses Wraith's clips) ─────*/
static const uint8_t WRAITH_IDLE[]   = {0,1};
static const uint8_t WRAITH_ATK[]    = {0,2,3};
static const uint8_t WRAITH_DIE[]    = {0,4,5,6,7};

/* ── Giant piranha ─────────────────────────────────────────────*/
static const uint8_t PIRANHA_IDLE[]  = {0,1,2,1};
static const uint8_t PIRANHA_ATK[]   = {3,4,5,6,7,8,9,10,11};
static const uint8_t PIRANHA_DIE[]   = {12,13,14};

const MobSpriteInfo MOB_SPRITES[MOB_KIND_COUNT] = {
    [MOB_RAT] = {
        .frame_w = 16, .frame_h = 15,
        .clips = {
            [MOB_ANIM_IDLE]   = { RAT_IDLE, 4, 2, 1 },
            [MOB_ANIM_RUN]    = { RAT_RUN, 5, 10, 1 },
            [MOB_ANIM_ATTACK] = { RAT_ATK, 5, 15, 0 },
            [MOB_ANIM_ZAP]    = { RAT_ATK, 5, 15, 0 },
            [MOB_ANIM_DIE]    = { RAT_DIE, 4, 10, 0 },
        },
    },
    [MOB_GNOLL] = {
        .frame_w = 12, .frame_h = 15,
        .clips = {
            [MOB_ANIM_IDLE]   = { GNOLL_IDLE, 8, 2, 1 },
            [MOB_ANIM_RUN]    = { GNOLL_RUN, 4, 12, 1 },
            [MOB_ANIM_ATTACK] = { GNOLL_ATK, 3, 12, 0 },
            [MOB_ANIM_ZAP]    = { GNOLL_ATK, 3, 12, 0 },
            [MOB_ANIM_DIE]    = { GNOLL_DIE, 3, 12, 0 },
        },
    },
    [MOB_CRAB] = {
        .frame_w = 16, .frame_h = 16,
        .clips = {
            [MOB_ANIM_IDLE]   = { CRAB_IDLE, 4, 5, 1 },
            [MOB_ANIM_RUN]    = { CRAB_RUN, 4, 15, 1 },
            [MOB_ANIM_ATTACK] = { CRAB_ATK, 3, 12, 0 },
            [MOB_ANIM_ZAP]    = { CRAB_ATK, 3, 12, 0 },
            [MOB_ANIM_DIE]    = { CRAB_DIE, 4, 12, 0 },
        },
    },
    [MOB_SWARM] = {
        .frame_w = 16, .frame_h = 16,
        .clips = {
            [MOB_ANIM_IDLE]   = { SWARM_IDLE, 6, 15, 1 },
            [MOB_ANIM_RUN]    = { SWARM_IDLE, 6, 15, 1 }, /* source: run == idle frames for Swarm */
            [MOB_ANIM_ATTACK] = { SWARM_ATK, 4, 20, 0 },
            [MOB_ANIM_ZAP]    = { SWARM_ATK, 4, 20, 0 },
            [MOB_ANIM_DIE]    = { SWARM_DIE, 5, 15, 0 },
        },
    },
    [MOB_SKELETON] = {
        .frame_w = 12, .frame_h = 15,
        .clips = {
            [MOB_ANIM_IDLE]   = { SKELETON_IDLE, 16, 12, 1 },
            [MOB_ANIM_RUN]    = { SKELETON_RUN, 6, 15, 1 },
            [MOB_ANIM_ATTACK] = { SKELETON_ATK, 3, 15, 0 },
            [MOB_ANIM_ZAP]    = { SKELETON_ATK, 3, 15, 0 },
            [MOB_ANIM_DIE]    = { SKELETON_DIE, 4, 12, 0 },
        },
    },
    [MOB_THIEF] = {
        .frame_w = 12, .frame_h = 13,
        .clips = {
            [MOB_ANIM_IDLE]   = { THIEF_IDLE, 9, 1, 1 },
            [MOB_ANIM_RUN]    = { THIEF_RUN, 6, 15, 1 },
            [MOB_ANIM_ATTACK] = { THIEF_ATK, 4, 12, 0 },
            [MOB_ANIM_ZAP]    = { THIEF_ATK, 4, 12, 0 },
            [MOB_ANIM_DIE]    = { THIEF_DIE, 5, 10, 0 },
        },
    },
    [MOB_SHAMAN] = {
        .frame_w = 12, .frame_h = 15,
        .clips = {
            [MOB_ANIM_IDLE]   = { SHAMAN_IDLE, 8, 2, 1 },
            [MOB_ANIM_RUN]    = { SHAMAN_RUN, 4, 12, 1 },
            [MOB_ANIM_ATTACK] = { SHAMAN_ATK, 3, 12, 0 },
            [MOB_ANIM_ZAP]    = { SHAMAN_ATK, 3, 12, 0 }, /* source: zap = attack.clone() */
            [MOB_ANIM_DIE]    = { SHAMAN_DIE, 3, 12, 0 },
        },
    },
    [MOB_BAT] = {
        .frame_w = 15, .frame_h = 15,
        .clips = {
            [MOB_ANIM_IDLE]   = { BAT_IDLE, 2, 8, 1 },
            [MOB_ANIM_RUN]    = { BAT_IDLE, 2, 12, 1 }, /* source: run reuses idle's 2 frames at a faster fps */
            [MOB_ANIM_ATTACK] = { BAT_ATK, 4, 12, 0 },
            [MOB_ANIM_ZAP]    = { BAT_ATK, 4, 12, 0 },
            [MOB_ANIM_DIE]    = { BAT_DIE, 3, 12, 0 },
        },
    },
    [MOB_BRUTE] = {
        .frame_w = 12, .frame_h = 16,
        .clips = {
            [MOB_ANIM_IDLE]   = { BRUTE_IDLE, 8, 2, 1 },
            [MOB_ANIM_RUN]    = { BRUTE_RUN, 4, 12, 1 },
            [MOB_ANIM_ATTACK] = { BRUTE_ATK, 3, 12, 0 },
            [MOB_ANIM_ZAP]    = { BRUTE_ATK, 3, 12, 0 },
            [MOB_ANIM_DIE]    = { BRUTE_DIE, 3, 12, 0 },
        },
    },
    [MOB_SPINNER] = {
        .frame_w = 16, .frame_h = 16,
        .clips = {
            [MOB_ANIM_IDLE]   = { SPINNER_IDLE, 8, 10, 1 },
            [MOB_ANIM_RUN]    = { SPINNER_RUN, 4, 15, 1 },
            [MOB_ANIM_ATTACK] = { SPINNER_ATK, 4, 12, 0 },
            [MOB_ANIM_ZAP]    = { SPINNER_ATK, 4, 12, 0 },
            [MOB_ANIM_DIE]    = { SPINNER_DIE, 4, 12, 0 },
        },
    },
    [MOB_ELEMENTAL] = {
        .frame_w = 12, .frame_h = 14,
        .clips = {
            [MOB_ANIM_IDLE]   = { ELEM_IDLE, 3, 10, 1 },
            [MOB_ANIM_RUN]    = { ELEM_RUN, 3, 12, 1 },
            [MOB_ANIM_ATTACK] = { ELEM_ATK, 3, 15, 0 },
            [MOB_ANIM_ZAP]    = { ELEM_ATK, 3, 15, 0 },
            [MOB_ANIM_DIE]    = { ELEM_DIE, 8, 15, 0 },
        },
    },
    [MOB_MONK] = {
        .frame_w = 15, .frame_h = 14,
        .clips = {
            [MOB_ANIM_IDLE]   = { MONK_IDLE, 4, 6, 1 },
            [MOB_ANIM_RUN]    = { MONK_RUN, 6, 15, 1 },
            [MOB_ANIM_ATTACK] = { MONK_ATK, 4, 12, 0 },
            [MOB_ANIM_ZAP]    = { MONK_ATK, 4, 12, 0 },
            [MOB_ANIM_DIE]    = { MONK_DIE, 6, 15, 0 },
        },
    },
    [MOB_WARLOCK] = {
        .frame_w = 12, .frame_h = 15,
        .clips = {
            [MOB_ANIM_IDLE]   = { WARLOCK_IDLE, 8, 2, 1 },
            [MOB_ANIM_RUN]    = { WARLOCK_RUN, 4, 15, 1 },
            [MOB_ANIM_ATTACK] = { WARLOCK_ATK, 3, 12, 0 },
            [MOB_ANIM_ZAP]    = { WARLOCK_ATK, 3, 12, 0 }, /* source: zap = attack.clone() */
            [MOB_ANIM_DIE]    = { WARLOCK_DIE, 6, 15, 0 },
        },
    },
    [MOB_GOLEM] = {
        .frame_w = 16, .frame_h = 16,
        .clips = {
            [MOB_ANIM_IDLE]   = { GOLEM_IDLE, 2, 4, 1 },
            [MOB_ANIM_RUN]    = { GOLEM_RUN, 4, 12, 1 },
            [MOB_ANIM_ATTACK] = { GOLEM_ATK, 3, 10, 0 },
            [MOB_ANIM_ZAP]    = { GOLEM_ATK, 3, 10, 0 },
            [MOB_ANIM_DIE]    = { GOLEM_DIE, 5, 15, 0 },
        },
    },
    [MOB_SUCCUBUS] = {
        .frame_w = 12, .frame_h = 15,
        .clips = {
            [MOB_ANIM_IDLE]   = { SUCCU_IDLE, 20, 8, 1 },
            [MOB_ANIM_RUN]    = { SUCCU_RUN, 6, 15, 1 },
            [MOB_ANIM_ATTACK] = { SUCCU_ATK, 3, 12, 0 },
            [MOB_ANIM_ZAP]    = { SUCCU_ATK, 3, 12, 0 },
            [MOB_ANIM_DIE]    = { SUCCU_DIE, 1, 10, 0 },
        },
    },
    [MOB_EYE] = {
        .frame_w = 16, .frame_h = 18,
        .clips = {
            [MOB_ANIM_IDLE]   = { EYE_IDLE, 3, 8, 1 },
            [MOB_ANIM_RUN]    = { EYE_RUN, 2, 12, 1 },
            [MOB_ANIM_ATTACK] = { EYE_ATK, 2, 8, 0 },
            [MOB_ANIM_ZAP]    = { EYE_ATK, 2, 8, 0 }, /* this IS the beam trigger, see mob.c note on MOB_EYE */
            [MOB_ANIM_DIE]    = { EYE_DIE, 3, 8, 0 },
        },
    },
    [MOB_SCORPIO] = {
        .frame_w = 18, .frame_h = 17,
        .clips = {
            [MOB_ANIM_IDLE]   = { SCORPIO_IDLE, 14, 12, 1 },
            [MOB_ANIM_RUN]    = { SCORPIO_RUN, 4, 8, 1 },
            [MOB_ANIM_ATTACK] = { SCORPIO_ATK, 3, 15, 0 },
            [MOB_ANIM_ZAP]    = { SCORPIO_ATK, 3, 15, 0 }, /* source: zap = attack.clone() */
            [MOB_ANIM_DIE]    = { SCORPIO_DIE, 5, 12, 0 },
        },
    },

    /* ── Elite variants — same sheet as their base mob, higher frame
       indices; see MOB_DB's elite section in mob.c for the stat side */
    [MOB_ALBINO] = {
        .frame_w = 16, .frame_h = 15, /* rat.png, shared with MOB_RAT */
        .clips = {
            [MOB_ANIM_IDLE]   = { ALBINO_IDLE, 4, 2, 1 },
            [MOB_ANIM_RUN]    = { ALBINO_RUN, 5, 10, 1 },
            [MOB_ANIM_ATTACK] = { ALBINO_ATK, 4, 15, 0 },
            [MOB_ANIM_ZAP]    = { ALBINO_ATK, 4, 15, 0 },
            [MOB_ANIM_DIE]    = { ALBINO_DIE, 4, 10, 0 },
        },
    },
    [MOB_BANDIT] = {
        .frame_w = 12, .frame_h = 13, /* thief.png, shared with MOB_THIEF */
        .clips = {
            [MOB_ANIM_IDLE]   = { BANDIT_IDLE, 9, 1, 1 },
            [MOB_ANIM_RUN]    = { BANDIT_RUN, 6, 15, 1 },
            [MOB_ANIM_ATTACK] = { BANDIT_ATK, 3, 12, 0 },
            [MOB_ANIM_ZAP]    = { BANDIT_ATK, 3, 12, 0 },
            [MOB_ANIM_DIE]    = { BANDIT_DIE, 5, 10, 0 },
        },
    },
    [MOB_SHIELDED] = {
        .frame_w = 12, .frame_h = 16, /* brute.png, shared with MOB_BRUTE */
        .clips = {
            [MOB_ANIM_IDLE]   = { SHIELD_IDLE, 8, 2, 1 },
            [MOB_ANIM_RUN]    = { SHIELD_RUN, 4, 12, 1 },
            [MOB_ANIM_ATTACK] = { SHIELD_ATK, 2, 12, 0 },
            [MOB_ANIM_ZAP]    = { SHIELD_ATK, 2, 12, 0 },
            [MOB_ANIM_DIE]    = { SHIELD_DIE, 3, 12, 0 },
        },
    },
    [MOB_SENIOR] = {
        .frame_w = 15, .frame_h = 14, /* monk.png, shared with MOB_MONK */
        .clips = {
            [MOB_ANIM_IDLE]   = { SENIOR_IDLE, 4, 6, 1 },
            [MOB_ANIM_RUN]    = { SENIOR_RUN, 6, 15, 1 },
            [MOB_ANIM_ATTACK] = { SENIOR_ATK, 4, 12, 0 },
            [MOB_ANIM_ZAP]    = { SENIOR_ATK, 4, 12, 0 },
            [MOB_ANIM_DIE]    = { SENIOR_DIE, 6, 15, 0 },
        },
    },
    [MOB_ACIDIC] = {
        .frame_w = 18, .frame_h = 17, /* scorpio.png, shared with MOB_SCORPIO */
        .clips = {
            [MOB_ANIM_IDLE]   = { ACIDIC_IDLE, 14, 12, 1 },
            [MOB_ANIM_RUN]    = { ACIDIC_RUN, 2, 4, 1 },
            [MOB_ANIM_ATTACK] = { ACIDIC_ATK, 3, 15, 0 },
            [MOB_ANIM_ZAP]    = { ACIDIC_ATK, 3, 15, 0 },
            [MOB_ANIM_DIE]    = { ACIDIC_DIE, 5, 12, 0 },
        },
    },

    /* ── Bosses + what they summon ──────────────────────────────*/
    [MOB_GOO] = {
        .frame_w = 20, .frame_h = 14,
        .clips = {
            [MOB_ANIM_IDLE]   = { GOO_IDLE, 2, 10, 1 },
            [MOB_ANIM_RUN]    = { GOO_IDLE, 2, 10, 1 }, /* source: run == idle frames for Goo */
            [MOB_ANIM_ATTACK] = { GOO_ATK, 3, 10, 0 },
            [MOB_ANIM_ZAP]    = { GOO_ATK, 3, 10, 0 },
            [MOB_ANIM_DIE]    = { GOO_DIE, 3, 10, 0 },
        },
    },
    [MOB_TENGU] = {
        .frame_w = 14, .frame_h = 16,
        .clips = {
            [MOB_ANIM_IDLE]   = { TENGU_IDLE, 4, 2, 1 },
            [MOB_ANIM_RUN]    = { TENGU_RUN, 5, 15, 0 }, /* source: doesn't loop — Tengu teleports more than it walks */
            [MOB_ANIM_ATTACK] = { TENGU_ATK, 4, 15, 0 },
            [MOB_ANIM_ZAP]    = { TENGU_ATK, 4, 15, 0 }, /* source: cast = attack.clone() */
            [MOB_ANIM_DIE]    = { TENGU_DIE, 8, 8, 0 },
        },
    },
    [MOB_DM300] = {
        .frame_w = 22, .frame_h = 20,
        .clips = {
            [MOB_ANIM_IDLE]   = { DM300_IDLE, 2, 10, 1 },
            [MOB_ANIM_RUN]    = { DM300_RUN, 2, 10, 1 },
            [MOB_ANIM_ATTACK] = { DM300_ATK, 3, 15, 0 },
            [MOB_ANIM_ZAP]    = { DM300_ATK, 3, 15, 0 },
            [MOB_ANIM_DIE]    = { DM300_DIE, 13, 20, 0 },
        },
    },
    [MOB_KING] = {
        .frame_w = 16, .frame_h = 16,
        .clips = {
            [MOB_ANIM_IDLE]   = { KING_IDLE, 14, 12, 1 },
            [MOB_ANIM_RUN]    = { KING_RUN, 6, 15, 1 },
            [MOB_ANIM_ATTACK] = { KING_ATK, 3, 15, 0 },
            [MOB_ANIM_ZAP]    = { KING_ATK, 3, 15, 0 },
            [MOB_ANIM_DIE]    = { KING_DIE, 4, 8, 0 },
        },
    },
    [MOB_KING_UNDEAD] = {
        .frame_w = 12, .frame_h = 16,
        .clips = {
            [MOB_ANIM_IDLE]   = { UNDEAD_IDLE, 16, 12, 1 },
            [MOB_ANIM_RUN]    = { UNDEAD_RUN, 6, 15, 1 },
            [MOB_ANIM_ATTACK] = { UNDEAD_ATK, 3, 15, 0 },
            [MOB_ANIM_ZAP]    = { UNDEAD_ATK, 3, 15, 0 },
            [MOB_ANIM_DIE]    = { UNDEAD_DIE, 4, 12, 0 },
        },
    },
    [MOB_YOG] = {
        .frame_w = 20, .frame_h = 19,
        .clips = {
            [MOB_ANIM_IDLE]   = { YOG_IDLE, 15, 10, 1 },
            [MOB_ANIM_RUN]    = { YOG_RUN, 1, 12, 1 },  /* stationary — see mob.c note on MOB_YOG */
            [MOB_ANIM_ATTACK] = { YOG_ATK, 1, 12, 0 },
            [MOB_ANIM_ZAP]    = { YOG_ATK, 1, 12, 0 },
            [MOB_ANIM_DIE]    = { YOG_DIE, 4, 10, 0 },
        },
    },
    [MOB_YOG_ROTTING_FIST] = {
        .frame_w = 24, .frame_h = 17,
        .clips = {
            [MOB_ANIM_IDLE]   = { ROTFIST_IDLE, 3, 2, 1 },
            [MOB_ANIM_RUN]    = { ROTFIST_RUN, 2, 3, 1 },
            [MOB_ANIM_ATTACK] = { ROTFIST_ATK, 1, 2, 0 },
            [MOB_ANIM_ZAP]    = { ROTFIST_ATK, 1, 2, 0 },
            [MOB_ANIM_DIE]    = { ROTFIST_DIE, 4, 10, 0 },
        },
    },
    [MOB_YOG_BURNING_FIST] = {
        .frame_w = 24, .frame_h = 17,
        .clips = {
            [MOB_ANIM_IDLE]   = { BURFIST_IDLE, 3, 2, 1 },
            [MOB_ANIM_RUN]    = { BURFIST_RUN, 2, 3, 1 },
            [MOB_ANIM_ATTACK] = { BURFIST_ATK, 3, 8, 0 },
            [MOB_ANIM_ZAP]    = { BURFIST_ATK, 3, 8, 0 }, /* ranged-only — "attack" IS the shot */
            [MOB_ANIM_DIE]    = { BURFIST_DIE, 4, 10, 0 },
        },
    },
    [MOB_YOG_LARVA] = {
        .frame_w = 12, .frame_h = 8,
        .clips = {
            [MOB_ANIM_IDLE]   = { LARVA_IDLE, 7, 5, 1 },
            [MOB_ANIM_RUN]    = { LARVA_RUN, 4, 12, 1 },
            [MOB_ANIM_ATTACK] = { LARVA_ATK, 3, 15, 0 },
            [MOB_ANIM_ZAP]    = { LARVA_ATK, 3, 15, 0 },
            [MOB_ANIM_DIE]    = { LARVA_DIE, 1, 10, 0 },
        },
    },

    /* ── Non-Bestiary spawns ─────────────────────────────────────*/
    [MOB_FETID_RAT] = { /* identical to MOB_RAT on purpose — see file header note */
        .frame_w = 16, .frame_h = 15,
        .clips = {
            [MOB_ANIM_IDLE]   = { RAT_IDLE, 4, 2, 1 },
            [MOB_ANIM_RUN]    = { RAT_RUN, 5, 10, 1 },
            [MOB_ANIM_ATTACK] = { RAT_ATK, 5, 15, 0 },
            [MOB_ANIM_ZAP]    = { RAT_ATK, 5, 15, 0 },
            [MOB_ANIM_DIE]    = { RAT_DIE, 4, 10, 0 },
        },
    },
    [MOB_MIMIC] = {
        .frame_w = 16, .frame_h = 16,
        .clips = {
            [MOB_ANIM_IDLE]   = { MIMIC_IDLE, 5, 5, 1 },
            [MOB_ANIM_RUN]    = { MIMIC_RUN, 7, 10, 1 },
            [MOB_ANIM_ATTACK] = { MIMIC_ATK, 4, 10, 0 },
            [MOB_ANIM_ZAP]    = { MIMIC_ATK, 4, 10, 0 },
            [MOB_ANIM_DIE]    = { MIMIC_DIE, 3, 5, 0 },
        },
    },
    [MOB_STATUE] = {
        .frame_w = 12, .frame_h = 15,
        .clips = {
            [MOB_ANIM_IDLE]   = { STATUE_IDLE, 7, 2, 1 },
            [MOB_ANIM_RUN]    = { STATUE_RUN, 6, 15, 1 },
            [MOB_ANIM_ATTACK] = { STATUE_ATK, 3, 12, 0 },
            [MOB_ANIM_ZAP]    = { STATUE_ATK, 3, 12, 0 },
            [MOB_ANIM_DIE]    = { STATUE_DIE, 6, 5, 0 },
        },
    },
    [MOB_WRAITH] = {
        .frame_w = 14, .frame_h = 15,
        .clips = {
            [MOB_ANIM_IDLE]   = { WRAITH_IDLE, 2, 5, 1 },
            [MOB_ANIM_RUN]    = { WRAITH_IDLE, 2, 10, 1 }, /* source: run reuses idle's 2 frames at a faster fps */
            [MOB_ANIM_ATTACK] = { WRAITH_ATK, 3, 10, 0 },
            [MOB_ANIM_ZAP]    = { WRAITH_ATK, 3, 10, 0 },
            [MOB_ANIM_DIE]    = { WRAITH_DIE, 5, 8, 0 },
        },
    },
    [MOB_PIRANHA] = {
        .frame_w = 12, .frame_h = 16,
        .clips = {
            [MOB_ANIM_IDLE]   = { PIRANHA_IDLE, 4, 8, 1 },
            [MOB_ANIM_RUN]    = { PIRANHA_IDLE, 4, 20, 1 }, /* source: run reuses idle's 4 frames at a faster fps */
            [MOB_ANIM_ATTACK] = { PIRANHA_ATK, 9, 20, 0 },
            [MOB_ANIM_ZAP]    = { PIRANHA_ATK, 9, 20, 0 },
            [MOB_ANIM_DIE]    = { PIRANHA_DIE, 3, 4, 0 },
        },
    },
    [MOB_CURSE_PERSONIFICATION] = { /* identical to MOB_WRAITH on purpose — see file header note */
        .frame_w = 14, .frame_h = 15,
        .clips = {
            [MOB_ANIM_IDLE]   = { WRAITH_IDLE, 2, 5, 1 },
            [MOB_ANIM_RUN]    = { WRAITH_IDLE, 2, 10, 1 },
            [MOB_ANIM_ATTACK] = { WRAITH_ATK, 3, 10, 0 },
            [MOB_ANIM_ZAP]    = { WRAITH_ATK, 3, 10, 0 },
            [MOB_ANIM_DIE]    = { WRAITH_DIE, 5, 8, 0 },
        },
    },
};

const MobTexId MOB_TEX_FOR_KIND[MOB_KIND_COUNT] = {
    [MOB_RAT] = MOBTEX_RAT,           [MOB_GNOLL] = MOBTEX_GNOLL,
    [MOB_CRAB] = MOBTEX_CRAB,         [MOB_SWARM] = MOBTEX_SWARM,
    [MOB_SKELETON] = MOBTEX_SKELETON, [MOB_THIEF] = MOBTEX_THIEF,
    [MOB_SHAMAN] = MOBTEX_SHAMAN,     [MOB_BAT] = MOBTEX_BAT,
    [MOB_BRUTE] = MOBTEX_BRUTE,       [MOB_SPINNER] = MOBTEX_SPINNER,
    [MOB_ELEMENTAL] = MOBTEX_ELEMENTAL, [MOB_MONK] = MOBTEX_MONK,
    [MOB_WARLOCK] = MOBTEX_WARLOCK,   [MOB_GOLEM] = MOBTEX_GOLEM,
    [MOB_SUCCUBUS] = MOBTEX_SUCCUBUS, [MOB_EYE] = MOBTEX_EYE,
    [MOB_SCORPIO] = MOBTEX_SCORPIO,

    [MOB_ALBINO] = MOBTEX_RAT,        [MOB_BANDIT] = MOBTEX_THIEF,
    [MOB_SHIELDED] = MOBTEX_BRUTE,    [MOB_SENIOR] = MOBTEX_MONK,
    [MOB_ACIDIC] = MOBTEX_SCORPIO,

    [MOB_GOO] = MOBTEX_GOO,           [MOB_TENGU] = MOBTEX_TENGU,
    [MOB_DM300] = MOBTEX_DM300,       [MOB_KING] = MOBTEX_KING,
    [MOB_KING_UNDEAD] = MOBTEX_UNDEAD, [MOB_YOG] = MOBTEX_YOG,
    [MOB_YOG_ROTTING_FIST] = MOBTEX_ROTTING,
    [MOB_YOG_BURNING_FIST] = MOBTEX_BURNING,
    [MOB_YOG_LARVA] = MOBTEX_LARVA,

    [MOB_FETID_RAT] = MOBTEX_RAT,     [MOB_MIMIC] = MOBTEX_MIMIC,
    [MOB_STATUE] = MOBTEX_STATUE,     [MOB_WRAITH] = MOBTEX_WRAITH,
    [MOB_PIRANHA] = MOBTEX_PIRANHA,   [MOB_CURSE_PERSONIFICATION] = MOBTEX_WRAITH,
};

/*═══════════════════════════════════════════════════════════════
    NPC
═══════════════════════════════════════════════════════════════*/
const NpcTexId NPC_TEX_FOR_KIND[NPC_KIND_COUNT] = {
    [NPC_GHOST]          = NPCTEX_GHOST,
    [NPC_BLACKSMITH]     = NPCTEX_BLACKSMITH,
    [NPC_IMP]            = NPCTEX_IMP,
    [NPC_IMP_SHOPKEEPER] = NPCTEX_IMP,
    [NPC_WANDMAKER]      = NPCTEX_WANDMAKER,
    [NPC_BEE]            = NPCTEX_BEE,
    [NPC_RATKING]        = NPCTEX_RATKING,
    [NPC_SHOPKEEPER]     = NPCTEX_SHOPKEEPER,
    /* NPC_MIRROR_IMAGE omitted */
};

static const uint8_t GHOST_IDLE[]      = {0,1};
static const uint8_t GHOST_DIE[]       = {0};

static const uint8_t BLACKSMITH_IDLE[] = {0,0,0,0,0,0,0,1,2,2,2,3};
static const uint8_t BLACKSMITH_F0[]   = {0};

static const uint8_t IMP_IDLE[] = {
    0,1,2,3,0,1,2,3,0,0,0,4,4,4,4,4,4,4,4,4,4,
    0,1,2,3,0,1,2,3,0,1,3,0,0,0,4,4,4,4,4,4,4,4,0,0,0,4,4,4,4,4,4,4,4
};
static const uint8_t IMP_F0[]  = {0};
static const uint8_t IMP_DIE[] = {0,3,2,1,0,3,2,1,0};

static const uint8_t WANDMAKER_IDLE[] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,3,3,3,3,3,3,2,1
};
static const uint8_t WANDMAKER_F0[] = {0};

static const uint8_t RATKING_IDLE[] = {0,0,0,1};
static const uint8_t RATKING_RUN[]  = {2,3,4,5,6};
static const uint8_t RATKING_F0[]   = {0};

static const uint8_t BEE_IDLE[] = {0,1,1,0,2,2};
static const uint8_t BEE_ATK[]  = {3,4,5,6};
static const uint8_t BEE_DIE[]  = {7,8,9,10};

static const uint8_t SHOPKEEPER_IDLE[] = {1,1,1,1,1,0,0,0,0};
static const uint8_t SHOPKEEPER_DIE[]  = {0};

const NpcSpriteInfo NPC_SPRITES[NPC_KIND_COUNT] = {
    [NPC_GHOST] = {
        .frame_w = 14, .frame_h = 15,
        .clips = {
            [NPC_ANIM_IDLE] = { GHOST_IDLE, 2, 5, 1 },
            [NPC_ANIM_RUN]  = { GHOST_IDLE, 2, 10, 1 }, /* source: run == idle frames, just faster */
            [NPC_ANIM_DIE]  = { GHOST_DIE, 1, 20, 0 },
        },
    },
    [NPC_BLACKSMITH] = {
        .frame_w = 13, .frame_h = 16,
        .clips = {
            [NPC_ANIM_IDLE] = { BLACKSMITH_IDLE, 12, 15, 1 },
            [NPC_ANIM_RUN]  = { BLACKSMITH_F0, 1, 20, 1 },
            [NPC_ANIM_DIE]  = { BLACKSMITH_F0, 1, 20, 0 },
        },
    },
    [NPC_IMP] = {
        .frame_w = 12, .frame_h = 14,
        .clips = {
            [NPC_ANIM_IDLE] = { IMP_IDLE, 54, 10, 1 },
            [NPC_ANIM_RUN]  = { IMP_F0, 1, 20, 1 },
            [NPC_ANIM_DIE]  = { IMP_DIE, 9, 10, 0 },
        },
    },
    [NPC_IMP_SHOPKEEPER] = {
        .frame_w = 12, .frame_h = 14,
        .clips = {
            [NPC_ANIM_IDLE] = { IMP_IDLE, 54, 10, 1 },
            [NPC_ANIM_RUN]  = { IMP_F0, 1, 20, 1 },
            [NPC_ANIM_DIE]  = { IMP_DIE, 9, 10, 0 },
        },
    },
    [NPC_WANDMAKER] = {
        .frame_w = 12, .frame_h = 14,
        .clips = {
            [NPC_ANIM_IDLE] = { WANDMAKER_IDLE, 24, 10, 1 },
            [NPC_ANIM_RUN]  = { WANDMAKER_F0, 1, 20, 1 },
            [NPC_ANIM_DIE]  = { WANDMAKER_F0, 1, 20, 0 },
        },
    },
    [NPC_SHOPKEEPER] = {
        .frame_w = 14, .frame_h = 14,
        .clips = {
            [NPC_ANIM_IDLE]   = { SHOPKEEPER_IDLE, 9, 10, 1 },
            [NPC_ANIM_RUN]    = { SHOPKEEPER_IDLE, 9, 10, 1 }, /* source: run/attack are literal clones of idle */
            [NPC_ANIM_ATTACK] = { SHOPKEEPER_IDLE, 9, 10, 1 },
            [NPC_ANIM_DIE]    = { SHOPKEEPER_DIE, 1, 20, 0 },
        },
    },
    [NPC_BEE] = {
        .frame_w = 16, .frame_h = 16,
        .clips = {
            [NPC_ANIM_IDLE]   = { BEE_IDLE, 6, 12, 1 },
            [NPC_ANIM_RUN]    = { BEE_IDLE, 6, 15, 1 }, /* source: run == idle frames, just faster */
            [NPC_ANIM_ATTACK] = { BEE_ATK, 4, 20, 0 },
            [NPC_ANIM_DIE]    = { BEE_DIE, 4, 20, 0 },
        },
    },
    [NPC_RATKING] = {
        .frame_w = 16, .frame_h = 16,
        .clips = {
            [NPC_ANIM_IDLE]   = { RATKING_IDLE, 4, 2, 1 },
            [NPC_ANIM_RUN]    = { RATKING_RUN, 5, 10, 1 },
            [NPC_ANIM_ATTACK] = { RATKING_F0, 1, 15, 0 },
            [NPC_ANIM_DIE]    = { RATKING_F0, 1, 10, 0 },
        },
    },
};

const AnimClip *const MIRROR_HERO_CLIPS[NPC_ANIM_COUNT] = {
    [NPC_ANIM_IDLE]   = &HERO_CLIPS[ANIM_IDLE],
    [NPC_ANIM_RUN]    = &HERO_CLIPS[ANIM_RUN],
    [NPC_ANIM_ATTACK] = &HERO_CLIPS[ANIM_ATTACK],
    [NPC_ANIM_DIE]    = &HERO_CLIPS[ANIM_DIE],
};
