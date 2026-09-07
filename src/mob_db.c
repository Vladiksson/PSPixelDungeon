#include "mob_db.h"

const MobStats MOB_DB[MOB_KIND_COUNT] = {
    [MOB_RAT] = {
        .name = "marsupial rat", .ht = 8,
        .attack_skill = 8, .defense_skill = 3, .dr = 1,
        .dmg_min = 1, .dmg_max = 5,
        .view_distance = 6, .xp_reward = 1, .base_speed = 1.0f, .attack_delay = 1.0f,
    },
    [MOB_GNOLL] = {
        .name = "gnoll scout", .ht = 12,
        .attack_skill = 11, .defense_skill = 4, .dr = 2,
        .dmg_min = 2, .dmg_max = 5,
        .view_distance = 8, .xp_reward = 2, .base_speed = 1.0f, .attack_delay = 1.0f,
    },
    [MOB_CRAB] = {
        .name = "sewer crab", .ht = 15,
        .attack_skill = 12, .defense_skill = 5, .dr = 4,
        .dmg_min = 3, .dmg_max = 6,
        .view_distance = 8, .xp_reward = 3, .base_speed = 2.0f, .attack_delay = 1.0f,
        /* no procs in source — fully covered by the generic AI as-is */
    },
    [MOB_SWARM] = {
        .name = "swarm of flies", .ht = 80,
        .attack_skill = 12, .defense_skill = 5, .dr = 0,
        .dmg_min = 1, .dmg_max = 4,
        .view_distance = 8, .xp_reward = 1, .base_speed = 1.0f, .attack_delay = 1.0f,
        .flying = 1,
        /* low XP is correct, not a typo — source pays it off in splits, not kills:
           any non-magic hit that leaves >=2 HP spawns a second Swarm at half HP
           (defense_proc); needs the split logic, no AiState change */
    },
    [MOB_SKELETON] = {
        .name = "skeleton", .ht = 25,
        .attack_skill = 12, .defense_skill = 9, .dr = 5,
        .dmg_min = 3, .dmg_max = 8,
        .view_distance = 8, .xp_reward = 5, .base_speed = 1.0f, .attack_delay = 1.0f,
        /* on death: explodes, damaging everything adjacent (not just its killer) —
           needs a custom vt->die, not just char_die_default() */
    },
    [MOB_THIEF] = {
        .name = "crazy thief", .ht = 20,
        .attack_skill = 12, .defense_skill = 12, .dr = 3,
        .dmg_min = 1, .dmg_max = 7,
        .view_distance = 8, .xp_reward = 5, .base_speed = 1.0f, .attack_delay = 0.5f,
        /* steals an item on hit then runs (attack_proc + a FLEEING AiState
           MobAiState doesn't have yet); drops the item on death */
    },
    [MOB_SHAMAN] = {
        .name = "gnoll shaman", .ht = 18,
        .attack_skill = 11, .defense_skill = 8, .dr = 4,
        .dmg_min = 2, .dmg_max = 6, /* melee fallback only, see below */
        .view_distance = 8, .xp_reward = 6, .base_speed = 1.0f, .attack_delay = 1.0f,
        /* real threat is a line-of-sight lightning bolt (2-12 dmg, x1.5 if the
           target's in water) that only fires when NOT adjacent — needs ranged
           attack support (Ballistica-equivalent LOS cast), not just melee */
    },
    [MOB_BAT] = {
        .name = "vampire bat", .ht = 30,
        .attack_skill = 16, .defense_skill = 15, .dr = 4,
        .dmg_min = 6, .dmg_max = 12,
        .view_distance = 8, .xp_reward = 7, .base_speed = 2.0f, .attack_delay = 1.0f,
        .flying = 1,
        /* heals itself for the damage it deals (capped at missing HP) — a
           one-line attack_proc, the cheapest of the "needs a proc" mobs */
    },
    [MOB_BRUTE] = {
        .name = "gnoll brute", .ht = 40,
        .attack_skill = 20, .defense_skill = 15, .dr = 8,
        .dmg_min = 8, .dmg_max = 18, /* jumps to 10-40 once enraged, see below */
        .view_distance = 8, .xp_reward = 8, .base_speed = 1.0f, .attack_delay = 1.0f,
        /* permanently enrages the instant HP first drops below HT/4 — needs a
           per-instance "enraged" flag on Mob, checked/set from vt->on_damaged */
    },
    [MOB_SPINNER] = {
        .name = "cave spinner", .ht = 50,
        .attack_skill = 20, .defense_skill = 14, .dr = 6,
        .dmg_min = 12, .dmg_max = 16,
        .view_distance = 8, .xp_reward = 9, .base_speed = 1.0f, .attack_delay = 1.0f,
        /* bite has a 50% chance to poison + flee (attack_proc, FLEEING AiState);
           lays a web blob behind it while fleeing — needs a blob/terrain system
           this port doesn't have yet, so treat as melee-only until then */
    },
    [MOB_ELEMENTAL] = {
        .name = "fire elemental", .ht = 65,
        .attack_skill = 25, .defense_skill = 20, .dr = 5,
        .dmg_min = 16, .dmg_max = 20,
        .view_distance = 8, .xp_reward = 10, .base_speed = 1.0f, .attack_delay = 1.0f,
        .flying = 1,
        /* 50% chance to ignite the target on hit (attack_proc); heals from fire,
           takes bonus damage from frost — needs the Burning/Frost buffs to exist
           first, so it's the "fully generic" fire elemental for now */
    },
    [MOB_MONK] = {
        .name = "dwarf monk", .ht = 70,
        .attack_skill = 30, .defense_skill = 30, .dr = 2,
        .dmg_min = 12, .dmg_max = 16,
        .view_distance = 8, .xp_reward = 11, .base_speed = 1.0f, .attack_delay = 0.5f,
        /* hostile despite the name — 1-in-6 chance to disarm the hero on hit
           (attack_proc); needs a hook into whatever the equipped-weapon slot
           ends up being once player.c grows one */
    },
    [MOB_WARLOCK] = {
        .name = "dwarf warlock", .ht = 70,
        .attack_skill = 25, .defense_skill = 18, .dr = 8,
        .dmg_min = 12, .dmg_max = 20, /* melee fallback only, see below */
        .view_distance = 8, .xp_reward = 11, .base_speed = 1.0f, .attack_delay = 1.0f,
        /* same shape as the shaman: a 12-18 dmg shadow bolt at range (+50%
           chance to apply Weakness to the hero specifically) once it has LOS,
           melee only when already adjacent */
    },
    [MOB_GOLEM] = {
        .name = "golem", .ht = 85,
        .attack_skill = 28, .defense_skill = 18, .dr = 12,
        .dmg_min = 20, .dmg_max = 40,
        .view_distance = 8, .xp_reward = 12, .base_speed = 1.0f, .attack_delay = 1.5f,
    },
    [MOB_SUCCUBUS] = {
        .name = "succubus", .ht = 80,
        .attack_skill = 40, .defense_skill = 25, .dr = 10,
        .dmg_min = 15, .dmg_max = 25,
        .view_distance = 4, .xp_reward = 12, .base_speed = 1.0f, .attack_delay = 1.0f,
        /* view_distance of 4 (not a typo, see Eye/Scorpio too) is correct per
           source — these three see less far than a rat does. 1-in-3 chance to
           charm on hit; teleports itself to spitting distance instead of
           walking when the path is clear (getCloser override) */
    },
    [MOB_EYE] = {
        .name = "evil eye", .ht = 100,
        .attack_skill = 30, .defense_skill = 20, .dr = 10,
        .dmg_min = 1, .dmg_max = 1, /* accurate to source, see note below */
        .view_distance = 4, .xp_reward = 13, .base_speed = 1.0f, .attack_delay = 1.6f,
        .flying = 1,
        /* never melees in source at all — its damageRoll() is never overridden,
           so it's really the engine default of 1. Its actual 14-20 dmg lives
           entirely inside a custom attack() that fires a beam down its whole
           line of sight, hitting (and hurting) anything standing in it,
           hero or mob. Left at the honest 1/1 rather than smuggling the real
           number into a field the source doesn't use it for — bump this to
           14/20 yourself as a stopgap if you want it to threaten anything
           before the beam exists. attack_delay = 1.6f is that beam's real
           cadence (its only attackDelay() override in source); until the
           beam exists it's just what this port's melee fallback uses */
    },
    [MOB_SCORPIO] = {
        .name = "scorpio", .ht = 95,
        .attack_skill = 36, .defense_skill = 24, .dr = 16,
        .dmg_min = 20, .dmg_max = 32,
        .view_distance = 4, .xp_reward = 14, .base_speed = 1.0f, .attack_delay = 1.0f,
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
        .view_distance = 6, .xp_reward = 1, .base_speed = 1.0f, .attack_delay = 1.0f,
        /* 50% chance to also apply Bleeding (attack_proc) */
    },
    [MOB_BANDIT] = {
        .name = "crazy bandit", .ht = 20,   /* stats identical to Thief */
        .attack_skill = 12, .defense_skill = 12, .dr = 3,
        .dmg_min = 1, .dmg_max = 7,
        .view_distance = 8, .xp_reward = 5, .base_speed = 1.0f, .attack_delay = 0.5f,
        /* its steal also blinds you for a few turns, on top of Thief's steal */
    },
    [MOB_SHIELDED] = {
        .name = "shielded brute", .ht = 40,
        .attack_skill = 20, .defense_skill = 20, .dr = 10, /* def/dr bumped from Brute's 15/8 */
        .dmg_min = 8, .dmg_max = 18, /* same 10-40-once-enraged as Brute, unmodeled */
        .view_distance = 8, .xp_reward = 8, .base_speed = 1.0f, .attack_delay = 1.0f,
    },
    [MOB_SENIOR] = {
        .name = "senior monk", .ht = 70,
        .attack_skill = 30, .defense_skill = 30, .dr = 2,
        .dmg_min = 12, .dmg_max = 20, /* bumped from Monk's 12-16 */
        .view_distance = 8, .xp_reward = 11, .base_speed = 1.0f, .attack_delay = 0.5f,
        /* Monk's disarm proc PLUS a separate 10% Paralysis proc, stacked */
    },
    [MOB_ACIDIC] = {
        .name = "acidic scorpio", .ht = 95,  /* stats identical to Scorpio */
        .attack_skill = 36, .defense_skill = 24, .dr = 16,
        .dmg_min = 20, .dmg_max = 32,
        .view_distance = 4, .xp_reward = 14, .base_speed = 1.0f, .attack_delay = 1.0f,
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
        .view_distance = 8, .xp_reward = 10, .base_speed = 1.0f, .attack_delay = 1.0f,
        /* regenerates in water; roughly every 3rd turn it pumps up, then
           either melees at +accuracy or leaps at a ranged target next turn —
           no pump/leap phase modeled here. Fight also seals the level exits */
    },
    [MOB_TENGU] = {
        .name = "Tengu", .ht = 120,
        .attack_skill = 20, .defense_skill = 20, .dr = 5,
        .dmg_min = 8, .dmg_max = 15,
        .view_distance = 8, .xp_reward = 20, .base_speed = 1.0f, .attack_delay = 1.0f,
        /* teleports every 5 turns (or sooner if it can't close in), arming
           random nearby traps as poison traps each time */
    },
    [MOB_DM300] = {
        .name = "DM-300", .ht = 200,
        .attack_skill = 28, .defense_skill = 18, .dr = 10,
        .dmg_min = 18, .dmg_max = 24,
        .view_distance = 8, .xp_reward = 30, .base_speed = 1.0f, .attack_delay = 1.0f,
        /* vents toxic gas every turn, self-repairs on inactive traps, and
           paralyses + rockslides a random adjacent cell on every move */
    },
    [MOB_KING] = {
        .name = "King of Dwarves", .ht = 300,
        .attack_skill = 32, .defense_skill = 25, .dr = 14,
        .dmg_min = 20, .dmg_max = 38,
        .view_distance = 8, .xp_reward = 40, .base_speed = 1.0f, .attack_delay = 1.0f,
        /* shuttles between two pedestals summoning up to 5 Undead
           (MOB_KING_UNDEAD below); its army cap grows as King loses HP */
    },
    [MOB_KING_UNDEAD] = {
        .name = "undead dwarf", .ht = 28,
        .attack_skill = 16, .defense_skill = 15, .dr = 5,
        .dmg_min = 12, .dmg_max = 16,
        .view_distance = 8, .xp_reward = 0, .base_speed = 1.0f, .attack_delay = 1.0f,
        .initial_state = MOB_WANDERING,
        /* 20% chance to Paralyse on hit (attack_proc) */
    },
    [MOB_YOG] = {
        .name = "Yog-Dzewa", .ht = 300,
        .attack_skill = 0, .defense_skill = 0, .dr = 0,  /* not a typo, see below */
        .dmg_min = 1, .dmg_max = 1,
        .view_distance = 8, .xp_reward = 50, .base_speed = 1.0f, .attack_delay = 1.0f,
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
        .view_distance = 8, .xp_reward = 0, .base_speed = 1.0f, .attack_delay = 1.0f,
        .initial_state = MOB_WANDERING,
        /* melee only; regenerates +4 HP/turn standing in water; 33% chance
           of an Ooze proc on hit */
    },
    [MOB_YOG_BURNING_FIST] = {
        .name = "burning fist", .ht = 200,
        .attack_skill = 36, .defense_skill = 25, .dr = 15,
        .dmg_min = 20, .dmg_max = 32,
        .view_distance = 8, .xp_reward = 0, .base_speed = 1.0f, .attack_delay = 1.0f,
        .initial_state = MOB_WANDERING,
        /* purely ranged (LOS, no adjacency needed at all — closer to Eye
           than to Shaman/Warlock's melee-or-ranged split); seeds fire in
           all 9 neighbouring cells every turn */
    },
    [MOB_YOG_LARVA] = {
        .name = "god's larva", .ht = 25,
        .attack_skill = 30, .defense_skill = 20, .dr = 8,
        .dmg_min = 15, .dmg_max = 20,
        .view_distance = 8, .xp_reward = 0, .base_speed = 1.0f, .attack_delay = 1.0f,
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
        .view_distance = 8, .xp_reward = 3, .base_speed = 1.0f, .attack_delay = 1.0f,
        .initial_state = MOB_WANDERING,
        /* defense_proc: releases a paralytic gas cloud when hit */
    },
    [MOB_MIMIC] = {
        .name = "mimic", .ht = 16,   /* depth 1: HT = (3+depth)*4 */
        .attack_skill = 10, .defense_skill = 5, .dr = 0,  /* atk=9+depth, def=atk/2 */
        .dmg_min = 1, .dmg_max = 4,  /* NormalIntRange(HT/10, HT/4) — derived from HT, not fixed */
        .view_distance = 8, .xp_reward = 2, /* EXP = 2+2*(depth-1)/5 */
        .base_speed = 1.0f, .attack_delay = 1.0f,
        .initial_state = MOB_HUNTING,
        /* 1-in-3 chance to drop some of the hero's own gold on hit (hero-only) */
    },
    [MOB_STATUE] = {
        .name = "animated statue", .ht = 20,  /* depth 1: HT=15+depth*5 — sourced */
        .attack_skill = 10, .defense_skill = 5, .dr = 1,  /* def=4+depth, dr=depth — sourced */
        .dmg_min = 1, .dmg_max = 6,  /* NOT sourced, see below */
        .view_distance = 8, .xp_reward = 0, .base_speed = 1.0f, .attack_delay = 1.0f,
        /* attack_skill/damage/attack_delay here are a placeholder assuming a
           basic, unenchanted weapon: source rolls an actual random melee
           weapon at construction and reads accuracy/min/max/attackDelay
           straight off it, so this row (attack_delay = 1.0f included) is
           only right once there's a weapon system to draw real numbers
           from. Also wants PASSIVE (see mob.h) — only wakes if actually
           hit; would currently wake on proximity instead */
    },
    [MOB_WRAITH] = {
        .name = "wraith", .ht = 1,  /* fixed at 1 regardless of depth — glass cannon */
        .attack_skill = 11, .defense_skill = 55, .dr = 0,  /* depth 1: atk=10+depth, def=atk*5 */
        .dmg_min = 1, .dmg_max = 4,  /* depth 1: NormalIntRange(1, 3+depth) */
        .view_distance = 8, .xp_reward = 0, .base_speed = 1.0f, .attack_delay = 1.0f,
        .flying = 1,
        .initial_state = MOB_HUNTING,
        /* event-spawned (e.g. a disturbed statue or tomb), not a normal
           wandering resident — see Wraith.spawnAt()/spawnAround() in source */
    },
    [MOB_PIRANHA] = {
        .name = "giant piranha", .ht = 15,  /* depth 1: HT = 10+depth*5 */
        .attack_skill = 22, .defense_skill = 12, .dr = 1, /* atk=20+depth*2, def=10+depth*2, dr=depth */
        .dmg_min = 1, .dmg_max = 6,  /* depth 1: NormalIntRange(depth, 4+depth*2) */
        .view_distance = 8, .xp_reward = 0, .base_speed = 2.0f, .attack_delay = 1.0f,
        /* instantly dies the moment it's outside water; movement is
           water-tile-restricted pathing, not the normal wander/hunt walk */
    },
    [MOB_CURSE_PERSONIFICATION] = {
        .name = "curse personification", .ht = 13,  /* depth 1: HT = 10+depth*3 */
        .attack_skill = 11, .defense_skill = 11, .dr = 1,  /* atk=10+depth, def=10+depth — sourced */
        .dmg_min = 3, .dmg_max = 5,  /* fixed, not depth-scaled */
        .view_distance = 8, .xp_reward = 3, .base_speed = 0.5f, .attack_delay = 1.0f,
        .flying = 1,
        .initial_state = MOB_HUNTING,
        /* attack_proc knocks the target back a tile; regenerates 1 HP/turn;
           spawned when a cursed item resists forced removal, and on death
           it turns into a passive Ghost NPC rather than dying normally */
    },
};