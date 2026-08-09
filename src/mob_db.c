#include "mob_db.h"

const MobStats MOB_DB[MOB_KIND_COUNT] = {
    [MOB_RAT] = {
        .name = "marsupial rat", .ht = 8,
        .attack_skill = 8, .defense_skill = 3, .dr = 1,
        .dmg_min = 1, .dmg_max = 5,
        .view_distance = 6, .xp_reward = 1, .base_speed = 1.0f,
    },
    [MOB_GNOLL] = {
        .name = "gnoll scout", .ht = 12,
        .attack_skill = 11, .defense_skill = 4, .dr = 2,
        .dmg_min = 2, .dmg_max = 5,
        .view_distance = 8, .xp_reward = 2, .base_speed = 1.0f,
    },
    [MOB_CRAB] = {
        .name = "sewer crab", .ht = 15,
        .attack_skill = 12, .defense_skill = 5, .dr = 4,
        .dmg_min = 3, .dmg_max = 6,
        .view_distance = 8, .xp_reward = 3, .base_speed = 2.0f,
        /* no procs in source — fully covered by the generic AI as-is */
    },
    [MOB_SWARM] = {
        .name = "swarm of flies", .ht = 80,
        .attack_skill = 12, .defense_skill = 5, .dr = 0,
        .dmg_min = 1, .dmg_max = 4,
        .view_distance = 8, .xp_reward = 1, .base_speed = 1.0f,
        .flying = 1,
        /* low XP is correct, not a typo — source pays it off in splits, not kills:
           any non-magic hit that leaves >=2 HP spawns a second Swarm at half HP
           (defense_proc); needs the split logic, no AiState change */
    },
    [MOB_SKELETON] = {
        .name = "skeleton", .ht = 25,
        .attack_skill = 12, .defense_skill = 9, .dr = 5,
        .dmg_min = 3, .dmg_max = 8,
        .view_distance = 8, .xp_reward = 5, .base_speed = 1.0f,
        /* on death: explodes, damaging everything adjacent (not just its killer) —
           needs a custom vt->die, not just char_die_default() */
    },
    [MOB_THIEF] = {
        .name = "crazy thief", .ht = 20,
        .attack_skill = 12, .defense_skill = 12, .dr = 3,
        .dmg_min = 1, .dmg_max = 7,
        .view_distance = 8, .xp_reward = 5, .base_speed = 1.0f,
        /* steals an item on hit then runs (attack_proc + a FLEEING AiState
           MobAiState doesn't have yet); drops the item on death */
    },
    [MOB_SHAMAN] = {
        .name = "gnoll shaman", .ht = 18,
        .attack_skill = 11, .defense_skill = 8, .dr = 4,
        .dmg_min = 2, .dmg_max = 6, /* melee fallback only, see below */
        .view_distance = 8, .xp_reward = 6, .base_speed = 1.0f,
        /* real threat is a line-of-sight lightning bolt (2-12 dmg, x1.5 if the
           target's in water) that only fires when NOT adjacent — needs ranged
           attack support (Ballistica-equivalent LOS cast), not just melee */
    },
    [MOB_BAT] = {
        .name = "vampire bat", .ht = 30,
        .attack_skill = 16, .defense_skill = 15, .dr = 4,
        .dmg_min = 6, .dmg_max = 12,
        .view_distance = 8, .xp_reward = 7, .base_speed = 2.0f,
        .flying = 1,
        /* heals itself for the damage it deals (capped at missing HP) — a
           one-line attack_proc, the cheapest of the "needs a proc" mobs */
    },
    [MOB_BRUTE] = {
        .name = "gnoll brute", .ht = 40,
        .attack_skill = 20, .defense_skill = 15, .dr = 8,
        .dmg_min = 8, .dmg_max = 18, /* jumps to 10-40 once enraged, see below */
        .view_distance = 8, .xp_reward = 8, .base_speed = 1.0f,
        /* permanently enrages the instant HP first drops below HT/4 — needs a
           per-instance "enraged" flag on Mob, checked/set from vt->on_damaged */
    },
    [MOB_SPINNER] = {
        .name = "cave spinner", .ht = 50,
        .attack_skill = 20, .defense_skill = 14, .dr = 6,
        .dmg_min = 12, .dmg_max = 16,
        .view_distance = 8, .xp_reward = 9, .base_speed = 1.0f,
        /* bite has a 50% chance to poison + flee (attack_proc, FLEEING AiState);
           lays a web blob behind it while fleeing — needs a blob/terrain system
           this port doesn't have yet, so treat as melee-only until then */
    },
    [MOB_ELEMENTAL] = {
        .name = "fire elemental", .ht = 65,
        .attack_skill = 25, .defense_skill = 20, .dr = 5,
        .dmg_min = 16, .dmg_max = 20,
        .view_distance = 8, .xp_reward = 10, .base_speed = 1.0f,
        .flying = 1,
        /* 50% chance to ignite the target on hit (attack_proc); heals from fire,
           takes bonus damage from frost — needs the Burning/Frost buffs to exist
           first, so it's the "fully generic" fire elemental for now */
    },
    [MOB_MONK] = {
        .name = "dwarf monk", .ht = 70,
        .attack_skill = 30, .defense_skill = 30, .dr = 2,
        .dmg_min = 12, .dmg_max = 16,
        .view_distance = 8, .xp_reward = 11, .base_speed = 1.0f,
        /* hostile despite the name — 1-in-6 chance to disarm the hero on hit
           (attack_proc); needs a hook into whatever the equipped-weapon slot
           ends up being once player.c grows one */
    },
    [MOB_WARLOCK] = {
        .name = "dwarf warlock", .ht = 70,
        .attack_skill = 25, .defense_skill = 18, .dr = 8,
        .dmg_min = 12, .dmg_max = 20, /* melee fallback only, see below */
        .view_distance = 8, .xp_reward = 11, .base_speed = 1.0f,
        /* same shape as the shaman: a 12-18 dmg shadow bolt at range (+50%
           chance to apply Weakness to the hero specifically) once it has LOS,
           melee only when already adjacent */
    },
    [MOB_GOLEM] = {
        .name = "golem", .ht = 85,
        .attack_skill = 28, .defense_skill = 18, .dr = 12,
        .dmg_min = 20, .dmg_max = 40,
        .view_distance = 8, .xp_reward = 12, .base_speed = 1.0f,
        /* no procs in source either — slow (1.5x attack delay) but that's a
           speed()-vs-attackDelay() split this port doesn't model yet, so it'll
           just hit at normal pace; otherwise fully covered by the generic AI */
    },
    [MOB_SUCCUBUS] = {
        .name = "succubus", .ht = 80,
        .attack_skill = 40, .defense_skill = 25, .dr = 10,
        .dmg_min = 15, .dmg_max = 25,
        .view_distance = 4, .xp_reward = 12, .base_speed = 1.0f,
        /* view_distance of 4 (not a typo, see Eye/Scorpio too) is correct per
           source — these three see less far than a rat does. 1-in-3 chance to
           charm on hit; teleports itself to spitting distance instead of
           walking when the path is clear (getCloser override) */
    },
    [MOB_EYE] = {
        .name = "evil eye", .ht = 100,
        .attack_skill = 30, .defense_skill = 20, .dr = 10,
        .dmg_min = 1, .dmg_max = 1, /* accurate to source, see note below */
        .view_distance = 4, .xp_reward = 13, .base_speed = 1.0f,
        .flying = 1,
        /* never melees in source at all — its damageRoll() is never overridden,
           so it's really the engine default of 1. Its actual 14-20 dmg lives
           entirely inside a custom attack() that fires a beam down its whole
           line of sight, hitting (and hurting) anything standing in it,
           hero or mob. Left at the honest 1/1 rather than smuggling the real
           number into a field the source doesn't use it for — bump this to
           14/20 yourself as a stopgap if you want it to threaten anything
           before the beam exists */
    },
    [MOB_SCORPIO] = {
        .name = "scorpio", .ht = 95,
        .attack_skill = 36, .defense_skill = 24, .dr = 16,
        .dmg_min = 20, .dmg_max = 32,
        .view_distance = 4, .xp_reward = 14, .base_speed = 1.0f,
        /* unlike shaman/warlock this one's ranged hit *does* run through the
           normal damageRoll(), so 20-32 is accurate whether it's ported as
           melee or ranged. Refuses to be adjacent — it backs away every turn
           it can (getCloser is inverted into a flee-from-target) and cripples
           on 50% of hits. Melee-only for now will make it walk up and swing
           instead of kiting, but the numbers above are correct either way */
    },

    /* ─── Elite variants — Bestiary rolls these 1-in-30 in place of the base
       mob. Each one `extends` its parent in source, inheriting anything it
       doesn't override; the rows below are the resolved, effective stats,
       not just what's textually in the elite's own class. ─── */
    [MOB_ALBINO] = {
        .name = "albino rat", .ht = 15,   /* only HP differs from Rat */
        .attack_skill = 8, .defense_skill = 3, .dr = 1,
        .dmg_min = 1, .dmg_max = 5,
        .view_distance = 6, .xp_reward = 1, .base_speed = 1.0f,
        /* 50% chance to also apply Bleeding (attack_proc) */
    },
    [MOB_BANDIT] = {
        .name = "crazy bandit", .ht = 20,   /* stats identical to Thief */
        .attack_skill = 12, .defense_skill = 12, .dr = 3,
        .dmg_min = 1, .dmg_max = 7,
        .view_distance = 8, .xp_reward = 5, .base_speed = 1.0f,
        /* its steal also blinds you for a few turns, on top of Thief's steal */
    },
    [MOB_SHIELDED] = {
        .name = "shielded brute", .ht = 40,
        .attack_skill = 20, .defense_skill = 20, .dr = 10, /* def/dr bumped from Brute's 15/8 */
        .dmg_min = 8, .dmg_max = 18, /* same 10-40-once-enraged as Brute, unmodeled */
        .view_distance = 8, .xp_reward = 8, .base_speed = 1.0f,
    },
    [MOB_SENIOR] = {
        .name = "senior monk", .ht = 70,
        .attack_skill = 30, .defense_skill = 30, .dr = 2,
        .dmg_min = 12, .dmg_max = 20, /* bumped from Monk's 12-16 */
        .view_distance = 8, .xp_reward = 11, .base_speed = 1.0f,
        /* Monk's disarm proc PLUS a separate 10% Paralysis proc, stacked */
    },
    [MOB_ACIDIC] = {
        .name = "acidic scorpio", .ht = 95,  /* stats identical to Scorpio */
        .attack_skill = 36, .defense_skill = 24, .dr = 16,
        .dmg_min = 20, .dmg_max = 32,
        .view_distance = 4, .xp_reward = 14, .base_speed = 1.0f,
        /* defense_proc: splashes 0-to-incoming-damage acid back at attacker */
    },

    /* ─── Bosses. Every row is baseline/non-enraged combat stats only —
       each fight is scripted and multi-phase (arena sealing, phase-shifted
       attacks, summons; one boss is stat-less by design) in ways a table
       row can't capture. This is "what it hits like on a normal turn," not
       "here's the boss fight." ─── */
    [MOB_GOO] = {
        .name = "Goo", .ht = 80,
        .attack_skill = 15, .defense_skill = 12, .dr = 2, /* 30 atk / 5-30 dmg once "pumped up" */
        .dmg_min = 2, .dmg_max = 12,
        .view_distance = 8, .xp_reward = 10, .base_speed = 1.0f,
        /* regenerates in water; roughly every 3rd turn it pumps up, then
           either melees at +accuracy or leaps at a ranged target next turn —
           no pump/leap phase modeled here. Fight also seals the level exits */
    },
    [MOB_TENGU] = {
        .name = "Tengu", .ht = 120,
        .attack_skill = 20, .defense_skill = 20, .dr = 5,
        .dmg_min = 8, .dmg_max = 15,
        .view_distance = 8, .xp_reward = 20, .base_speed = 1.0f,
        /* teleports every 5 turns (or sooner if it can't close in), arming
           random nearby traps as poison traps each time */
    },
    [MOB_DM300] = {
        .name = "DM-300", .ht = 200,
        .attack_skill = 28, .defense_skill = 18, .dr = 10,
        .dmg_min = 18, .dmg_max = 24,
        .view_distance = 8, .xp_reward = 30, .base_speed = 1.0f,
        /* vents toxic gas every turn, self-repairs on inactive traps, and
           paralyses + rockslides a random adjacent cell on every move */
    },
    [MOB_KING] = {
        .name = "King of Dwarves", .ht = 300,
        .attack_skill = 32, .defense_skill = 25, .dr = 14,
        .dmg_min = 20, .dmg_max = 38,
        .view_distance = 8, .xp_reward = 40, .base_speed = 1.0f,
        /* shuttles between two pedestals summoning up to 5 Undead
           (MOB_KING_UNDEAD below); its army cap grows as King loses HP */
    },
    [MOB_KING_UNDEAD] = {
        .name = "undead dwarf", .ht = 28,
        .attack_skill = 16, .defense_skill = 15, .dr = 5,
        .dmg_min = 12, .dmg_max = 16,
        .view_distance = 8, .xp_reward = 0, .base_speed = 1.0f,
        .initial_state = MOB_WANDERING,
        /* 20% chance to Paralyse on hit (attack_proc) */
    },
    [MOB_YOG] = {
        .name = "Yog-Dzewa", .ht = 300,
        .attack_skill = 0, .defense_skill = 0, .dr = 0,  /* not a typo, see below */
        .dmg_min = 1, .dmg_max = 1,
        .view_distance = 8, .xp_reward = 50, .base_speed = 1.0f,
        /* Yog's body never overrides attackSkill/dr/damageRoll in source at
           all — it's a stationary HP pool, incapable of fighting you
           directly by design. Its two fists (below) do 100% of the
           offense; while either lives, damage TO Yog is halved per living
           fist, and every hit it takes spawns a Larva. Wants PASSIVE (see
           mob.h) — mapped to ASLEEP here since PASSIVE doesn't exist yet */
    },
    [MOB_YOG_ROTTING_FIST] = {
        .name = "rotting fist", .ht = 300,
        .attack_skill = 36, .defense_skill = 25, .dr = 15,
        .dmg_min = 24, .dmg_max = 36,
        .view_distance = 8, .xp_reward = 0, .base_speed = 1.0f,
        .initial_state = MOB_WANDERING,
        /* melee only; regenerates +4 HP/turn standing in water; 33% chance
           of an Ooze proc on hit */
    },
    [MOB_YOG_BURNING_FIST] = {
        .name = "burning fist", .ht = 200,
        .attack_skill = 36, .defense_skill = 25, .dr = 15,
        .dmg_min = 20, .dmg_max = 32,
        .view_distance = 8, .xp_reward = 0, .base_speed = 1.0f,
        .initial_state = MOB_WANDERING,
        /* purely ranged (LOS, no adjacency needed at all — closer to Eye
           than to Shaman/Warlock's melee-or-ranged split); seeds fire in
           all 9 neighbouring cells every turn */
    },
    [MOB_YOG_LARVA] = {
        .name = "god's larva", .ht = 25,
        .attack_skill = 30, .defense_skill = 20, .dr = 8,
        .dmg_min = 15, .dmg_max = 20,
        .view_distance = 8, .xp_reward = 0, .base_speed = 1.0f,
        .initial_state = MOB_HUNTING,
        /* spawned one at a time by Yog's defense_proc whenever it's hit */
    },

    /* ─── Spawned outside Bestiary's per-depth roll by specific triggers,
       not the normal floor population. Several of these scale their stats
       with Dungeon.depth in source instead of being fixed constants — this
       table has no notion of depth, so those rows are the source formula
       evaluated at depth 1 (the weakest they can ever be), with the real
       formula left in a comment. Revisit if/when a mob needs to spawn
       through something other than mob_spawn()'s flat kind->stats lookup */
    [MOB_FETID_RAT] = {
        .name = "fetid rat", .ht = 15,    /* fixed, not depth-scaled */
        .attack_skill = 12, .defense_skill = 5, .dr = 2,
        .dmg_min = 2, .dmg_max = 6,
        .view_distance = 8, .xp_reward = 3, .base_speed = 1.0f,
        .initial_state = MOB_WANDERING,
        /* defense_proc: releases a paralytic gas cloud when hit */
    },
    [MOB_MIMIC] = {
        .name = "mimic", .ht = 16,   /* depth 1: HT = (3+depth)*4 */
        .attack_skill = 10, .defense_skill = 5, .dr = 0,  /* atk=9+depth, def=atk/2 */
        .dmg_min = 1, .dmg_max = 4,  /* NormalIntRange(HT/10, HT/4) — derived from HT, not fixed */
        .view_distance = 8, .xp_reward = 2, .base_speed = 1.0f,  /* EXP = 2+2*(depth-1)/5 */
        .initial_state = MOB_HUNTING,
        /* 1-in-3 chance to drop some of the hero's own gold on hit (hero-only) */
    },
    [MOB_STATUE] = {
        .name = "animated statue", .ht = 20,  /* depth 1: HT=15+depth*5 — sourced */
        .attack_skill = 10, .defense_skill = 5, .dr = 1,  /* def=4+depth, dr=depth — sourced */
        .dmg_min = 1, .dmg_max = 6,  /* NOT sourced, see below */
        .view_distance = 8, .xp_reward = 0, .base_speed = 1.0f,
        /* attack_skill/damage here are a placeholder assuming a basic,
           unenchanted weapon: source rolls an actual random melee weapon at
           construction and reads accuracy/min/max/attackDelay straight off
           it, so this row is only right once there's a weapon system to
           draw real numbers from. Also wants PASSIVE (see mob.h) — only
           wakes if actually hit; would currently wake on proximity instead */
    },
    [MOB_WRAITH] = {
        .name = "wraith", .ht = 1,  /* fixed at 1 regardless of depth — glass cannon */
        .attack_skill = 11, .defense_skill = 55, .dr = 0,  /* depth 1: atk=10+depth, def=atk*5 */
        .dmg_min = 1, .dmg_max = 4,  /* depth 1: NormalIntRange(1, 3+depth) */
        .view_distance = 8, .xp_reward = 0, .base_speed = 1.0f,
        .flying = 1,
        .initial_state = MOB_HUNTING,
        /* event-spawned (e.g. a disturbed statue or tomb), not a normal
           wandering resident — see Wraith.spawnAt()/spawnAround() in source */
    },
    [MOB_PIRANHA] = {
        .name = "giant piranha", .ht = 15,  /* depth 1: HT = 10+depth*5 */
        .attack_skill = 22, .defense_skill = 12, .dr = 1, /* atk=20+depth*2, def=10+depth*2, dr=depth */
        .dmg_min = 1, .dmg_max = 6,  /* depth 1: NormalIntRange(depth, 4+depth*2) */
        .view_distance = 8, .xp_reward = 0, .base_speed = 2.0f,
        /* instantly dies the moment it's outside water; movement is
           water-tile-restricted pathing, not the normal wander/hunt walk */
    },
    [MOB_CURSE_PERSONIFICATION] = {
        .name = "curse personification", .ht = 13,  /* depth 1: HT = 10+depth*3 */
        .attack_skill = 11, .defense_skill = 11, .dr = 1,  /* atk=10+depth, def=10+depth — sourced */
        .dmg_min = 3, .dmg_max = 5,  /* fixed, not depth-scaled */
        .view_distance = 8, .xp_reward = 3, .base_speed = 0.5f,
        .flying = 1,
        .initial_state = MOB_HUNTING,
        /* attack_proc knocks the target back a tile; regenerates 1 HP/turn;
           spawned when a cursed item resists forced removal, and on death
           it turns into a passive Ghost NPC rather than dying normally */
    },
};

/*═══════════════════════════════════════════════════════════════
    Mob animation data
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

/*═══════════════════════════════════════════════════════════════
    MOB SPRITES
═══════════════════════════════════════════════════════════════*/
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