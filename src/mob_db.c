#include "mob_db.h"
#include "mob_procs.h"

/* ══════════════════════════════════════════════════════════
   Depth scaling
   ══════════════════════════════════════════════════════════ */

/* Mimic.adjustStats(level) */
static void scale_mimic(MobStats *s, int d) {
    s->ht            = (3 + d) * 4;
    s->attack_skill  = 9 + d;
    s->defense_skill = s->attack_skill / 2;
    s->dmg_min       = s->ht / 10;                /* NormalIntRange(HT / 10, HT / 4) */
    s->dmg_max       = s->ht / 4;
    s->xp_reward     = 2 + 2 * (d - 1) / 5;
}

/* Statue(): HT = 15 + depth*5, defenseSkill = 4 + depth, dr = depth.
   attackSkill is (9 + depth) * weapon.ACU in the original — there's no weapon
   system yet, so the multiplier is taken as 1.0 (same story for damage and
   attack_delay, which the row keeps as placeholders). */
static void scale_statue(MobStats *s, int d) {
    s->ht            = 15 + d * 5;
    s->defense_skill = 4 + d;
    s->attack_skill  = 9 + d;
    s->dr            = d;
}

/* Wraith.adjustStats(level) — HT stays 1 at every depth */
static void scale_wraith(MobStats *s, int d) {
    s->attack_skill  = 10 + d;
    s->defense_skill = s->attack_skill * 5;
    s->dmg_min       = 1;                         /* NormalIntRange(1, 3 + level) */
    s->dmg_max       = 3 + d;
}

/* Piranha() */
static void scale_piranha(MobStats *s, int d) {
    s->ht            = 10 + d * 5;
    s->defense_skill = 10 + d * 2;
    s->attack_skill  = 20 + d * 2;
    s->dr            = d;
    s->dmg_min       = d;                         /* NormalIntRange(depth, 4 + depth*2) */
    s->dmg_max       = 4 + d * 2;
}

/* CursePersonification() — damage (3-5) and dr (1) are fixed */
static void scale_curse(MobStats *s, int d) {
    s->ht            = 10 + d * 3;
    s->defense_skill = 10 + d;
    s->attack_skill  = 10 + d;
}

/* ══════════════════════════════════════════════════════════
   MOB_DB
   ══════════════════════════════════════════════════════════ */
static const MobStats MOB_DB[MOB_KIND_COUNT] = {
    [MOB_RAT] = {
        .name = "marsupial rat", .ht = 8,
        .attack_skill = 8, .defense_skill = 3, .dr = 1,
        .dmg_min = 1, .dmg_max = 5,
        .view_distance = 8, .xp_reward = 1, .base_speed = 1.0f, .attack_delay = 1.0f,
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
    },
    [MOB_SWARM] = {
        .name = "swarm of flies", .ht = 80,
        .attack_skill = 12, .defense_skill = 5, .dr = 0,
        .dmg_min = 1, .dmg_max = 4,
        .view_distance = 8, .xp_reward = 1, .base_speed = 1.0f, .attack_delay = 1.0f,
        .flying = 1,
        /* low XP is correct, not a typo — source pays it off in splits, not kills:
           any non-magic hit that leaves >= 2 HP spawns a second Swarm with half
           of what's left. Capped by the 8-slot MobPool: no free slot, no split. */
        .defense_proc = mobproc_swarm_split,
    },
    [MOB_SKELETON] = {
        .name = "skeleton", .ht = 25,
        .attack_skill = 12, .defense_skill = 9, .dr = 5,
        .dmg_min = 3, .dmg_max = 8,
        .view_distance = 8, .xp_reward = 5, .base_speed = 1.0f, .attack_delay = 1.0f,
        .dmg_immune = DMGBIT(DMGSRC_DEATH),
        /* on death: explodes, damaging everything adjacent (not just its killer) */
        .on_death = mobproc_skeleton_burst,
    },
    [MOB_THIEF] = {
        .name = "crazy thief", .ht = 20,
        .attack_skill = 12, .defense_skill = 12, .dr = 3,
        .dmg_min = 1, .dmg_max = 7,
        .view_distance = 8, .xp_reward = 5, .base_speed = 1.0f, .attack_delay = 0.5f,
        .flags = MOBF_FLEE_CORNERED_FIGHTS,
        /* TODO(proc): attack_proc steal() — takes a random unequipped item off the hero,
           then FLEEING. defense_proc: while FLEEING, drops Gold at its feet when hit.
           on_death: drops the stolen item. All need inventory access from mob code. */
    },
    [MOB_SHAMAN] = {
        .name = "gnoll shaman", .ht = 18,
        .attack_skill = 11, .defense_skill = 8, .dr = 4,
        .dmg_min = 2, .dmg_max = 6, /* melee fallback only, see below */
        .view_distance = 8, .xp_reward = 6, .base_speed = 1.0f, .attack_delay = 1.0f,
        .dmg_resist = DMGBIT(DMGSRC_LIGHTNING),
        /* TODO(ai): the real threat is a line-of-sight lightning bolt (Random.Int(2, 12),
           x1.5 if the target's in water) that only fires when NOT adjacent — needs ranged
           attack support (Ballistica-equivalent LOS cast), not just melee. Emit it with
           DMGSRC_LIGHTNING so the hero's own resistances/armor glyphs can see it. */
    },
    [MOB_BAT] = {
        .name = "vampire bat", .ht = 30,
        .attack_skill = 16, .defense_skill = 15, .dr = 4,
        .dmg_min = 6, .dmg_max = 12,
        .view_distance = 8, .xp_reward = 7, .base_speed = 2.0f, .attack_delay = 1.0f,
        .flying = 1,
        /* heals itself for the damage it deals, capped at its missing HP */
        .attack_proc = mobproc_vampiric,
    },
    [MOB_BRUTE] = {
        .name = "gnoll brute", .ht = 40,
        .attack_skill = 20, .defense_skill = 15, .dr = 8,
        .dmg_min = 8, .dmg_max = 18,
        .view_distance = 8, .xp_reward = 8, .base_speed = 1.0f, .attack_delay = 1.0f,
        .buff_immune = BUFFBIT(BUFF_TERROR),
        /* enrages for good the instant HP first drops below HT/4 (and loses a tick doing it) */
        .enrage_hp_div = 4, .enrage_dmg_min = 10, .enrage_dmg_max = 40,
    },
    [MOB_SPINNER] = {
        .name = "cave spinner", .ht = 50,
        .attack_skill = 20, .defense_skill = 14, .dr = 6,
        .dmg_min = 12, .dmg_max = 16,
        .view_distance = 8, .xp_reward = 9, .base_speed = 1.0f, .attack_delay = 1.0f,
        .buff_immune = BUFFBIT(BUFF_ROOTS),
        .dmg_resist  = DMGBIT(DMGSRC_POISON),
        /* bite: 50% Poison.set(Random.Int(7, 9) = 7..8), then it runs; it keeps running until
           the poison is off its victim (or it's cornered) */
        .on_hit = {{ .one_in = 2, .buff = BUFF_POISON, .mode = MOBHIT_SET_DURATION,
                     .dur_min = 7, .dur_max = 8, .flee = 1 }},
        .flags = MOBF_FLEE_CORNERED_FIGHTS | MOBF_FLEE_UNTIL_UNPOISONED,
        /* TODO(ai): lays a web blob behind it while fleeing — needs a blob/terrain system;
           melee-only until then */
    },
    [MOB_ELEMENTAL] = {
        .name = "fire elemental", .ht = 65,
        .attack_skill = 25, .defense_skill = 20, .dr = 5,
        .dmg_min = 16, .dmg_max = 20,
        .view_distance = 8, .xp_reward = 10, .base_speed = 1.0f, .attack_delay = 1.0f,
        .flying = 1,
        .buff_immune = BUFFBIT(BUFF_BURNING),
        .dmg_immune  = DMGBIT(DMGSRC_FIRE) | DMGBIT(DMGSRC_PSIONIC),
        .on_hit = {{ .one_in = 2, .buff = BUFF_BURNING, .dur_min = BURNING_DURATION }},
        /* Frost hurts it (NormalIntRange(1, HT*2/3)) before it freezes. Source also has a
           "Burning heals it by 1" branch, but that's dead code: Burning is in its IMMUNITIES,
           so attachTo() bails before add() ever runs. Not ported. */
        .on_buff_added = mobproc_elemental_frost,
    },
    [MOB_MONK] = {
        .name = "dwarf monk", .ht = 70,
        .attack_skill = 30, .defense_skill = 30, .dr = 2,
        .dmg_min = 12, .dmg_max = 16,
        .view_distance = 8, .xp_reward = 11, .base_speed = 1.0f, .attack_delay = 0.5f,
        .buff_immune = BUFFBIT(BUFF_AMOK) | BUFFBIT(BUFF_TERROR),
        /* TODO(proc): attack_proc — 1-in-6 chance to disarm the hero on hit (unequips the
           weapon and drops it); needs a hook into the equipped-weapon slot */
    },
    [MOB_WARLOCK] = {
        .name = "dwarf warlock", .ht = 70,
        .attack_skill = 25, .defense_skill = 18, .dr = 8,
        .dmg_min = 12, .dmg_max = 20, /* melee fallback only, see below */
        .view_distance = 8, .xp_reward = 11, .base_speed = 1.0f, .attack_delay = 1.0f,
        .dmg_resist = DMGBIT(DMGSRC_DEATH),
        /* TODO(ai): same shape as the shaman — a shadow bolt (Random.Int(12, 18)) at range once
           it has LOS, melee only when already adjacent; the bolt gives the hero Weakness
           50% of the time (hero only — Weakness hard-casts to Hero in source) */
    },
    [MOB_GOLEM] = {
        .name = "golem", .ht = 85,
        .attack_skill = 28, .defense_skill = 18, .dr = 12,
        .dmg_min = 20, .dmg_max = 40,
        .view_distance = 8, .xp_reward = 12, .base_speed = 1.0f, .attack_delay = 1.5f,
        .buff_immune = BUFFBIT(BUFF_AMOK) | BUFFBIT(BUFF_TERROR) | BUFFBIT(BUFF_SLEEP),
        .dmg_resist  = DMGBIT(DMGSRC_PSIONIC),
    },
    [MOB_SUCCUBUS] = {
        .name = "succubus", .ht = 80,
        .attack_skill = 40, .defense_skill = 25, .dr = 10,
        .dmg_min = 15, .dmg_max = 25,
        .view_distance = 4, .xp_reward = 12, .base_speed = 1.0f, .attack_delay = 1.0f,
        .buff_immune = BUFFBIT(BUFF_SLEEP),
        /* view_distance of 4 (not a typo, see Eye/Scorpio too) is correct per source — these
           three see less far than a rat does. Charm.durationFactor() is 1 until rings exist. */
        .on_hit = {{ .one_in = 3, .buff = BUFF_CHARM, .mode = MOBHIT_TAG_ATTACKER,
                     .dur_min = 3, .dur_max = 7 }},
        /* TODO(ai): teleports itself to spitting distance instead of walking when the path
           is clear (getCloser override) */
    },
    [MOB_EYE] = {
        .name = "evil eye", .ht = 100,
        .attack_skill = 30, .defense_skill = 20, .dr = 10,
        .dmg_min = 1, .dmg_max = 1, /* accurate to source, see note below */
        .view_distance = 4, .xp_reward = 13, .base_speed = 1.0f, .attack_delay = 1.6f,
        .flying = 1,
        .buff_immune = BUFFBIT(BUFF_TERROR),
        .dmg_resist  = DMGBIT(DMGSRC_DISINTEGRATION) | DMGBIT(DMGSRC_DEATH),
        /* TODO(ai): never melees in source at all — its damageRoll() is never overridden, so
           it's really the engine default of 1. Its actual 14-20 dmg (NormalIntRange(14, 20))
           lives entirely inside a custom attack() that fires a beam down its whole line of
           sight, hitting (and hurting) anything standing in it, hero or mob. Left at the honest
           1/1 rather than smuggling the real number into a field the source doesn't use it for.
           attack_delay = 1.6f is that beam's real cadence (its only attackDelay() override). */
    },
    [MOB_SCORPIO] = {
        .name = "scorpio", .ht = 95,
        .attack_skill = 36, .defense_skill = 24, .dr = 16,
        .dmg_min = 20, .dmg_max = 32,
        .view_distance = 4, .xp_reward = 14, .base_speed = 1.0f, .attack_delay = 1.0f,
        .dmg_resist = DMGBIT(DMGSRC_POISON),
        /* 50% of hits cripple (Buff.prolong, Cripple.DURATION = 10) */
        .on_hit = {{ .one_in = 2, .buff = BUFF_CRIPPLE, .dur_min = CRIPPLE_DURATION }},
        /* TODO(ai): refuses to be adjacent — it backs away every turn it can (getCloser is
           inverted into a flee-from-target). Melee-only for now makes it walk up and swing
           instead of kiting, but the numbers above are correct either way. */
    },

    /* ─── Elite variants — Bestiary.mutable() rolls these 1-in-30 in place of the base mob
       (source only does that for respawns, not the floor's initial population). ─── */
    [MOB_ALBINO] = {
        .name = "albino rat", .ht = 15,   /* only HP differs from Rat */
        .attack_skill = 8, .defense_skill = 3, .dr = 1,
        .dmg_min = 1, .dmg_max = 5,
        .view_distance = 8, .xp_reward = 1, .base_speed = 1.0f, .attack_delay = 1.0f,
        /* 50%: Bleeding.set(damage) — the bleed starts at the hit's damage and decays */
        .on_hit = {{ .one_in = 2, .buff = BUFF_BLEEDING, .mode = MOBHIT_LEVEL_FROM_DAMAGE,
                     .dur_min = -1 }},   /* -1: no duration, like char_bleeding_grant() */
    },
    [MOB_BANDIT] = {
        .name = "crazy bandit", .ht = 20,   /* stats identical to Thief */
        .attack_skill = 12, .defense_skill = 12, .dr = 3,
        .dmg_min = 1, .dmg_max = 7,
        .view_distance = 8, .xp_reward = 5, .base_speed = 1.0f, .attack_delay = 0.5f,
        .flags = MOBF_FLEE_CORNERED_FIGHTS,
        /* TODO(proc): Thief's steal (see MOB_THIEF), plus Buff.prolong(Blindness, Random.Int(5, 12))
           on the victim after a successful steal */
    },
    [MOB_SHIELDED] = {
        .name = "shielded brute", .ht = 40,
        .attack_skill = 20, .defense_skill = 20, .dr = 10, /* def/dr bumped from Brute's 15/8 */
        .dmg_min = 8, .dmg_max = 18,
        .view_distance = 8, .xp_reward = 8, .base_speed = 1.0f, .attack_delay = 1.0f,
        .buff_immune = BUFFBIT(BUFF_TERROR),
        .enrage_hp_div = 4, .enrage_dmg_min = 10, .enrage_dmg_max = 40,   /* same as Brute */
    },
    [MOB_SENIOR] = {
        .name = "senior monk", .ht = 70,
        .attack_skill = 30, .defense_skill = 30, .dr = 2,
        .dmg_min = 12, .dmg_max = 20, /* bumped from Monk's 12-16 */
        .view_distance = 8, .xp_reward = 11, .base_speed = 1.0f, .attack_delay = 0.5f,
        .buff_immune = BUFFBIT(BUFF_AMOK) | BUFFBIT(BUFF_TERROR),
        /* 10%: Buff.prolong(Paralysis, 1.1) — on top of Monk's disarm (TODO(proc), see MOB_MONK) */
        .on_hit = {{ .one_in = 10, .buff = BUFF_PARALYSIS, .dur_min = 1.1f }},
    },
    [MOB_ACIDIC] = {
        .name = "acidic scorpio", .ht = 95,  /* stats identical to Scorpio */
        .attack_skill = 36, .defense_skill = 24, .dr = 16,
        .dmg_min = 20, .dmg_max = 32,
        .view_distance = 4, .xp_reward = 14, .base_speed = 1.0f, .attack_delay = 1.0f,
        .dmg_resist = DMGBIT(DMGSRC_POISON),
        .on_hit = {{ .one_in = 2, .buff = BUFF_CRIPPLE, .dur_min = CRIPPLE_DURATION }},
        /* splashes 0..incoming-damage of acid back at whoever hit it */
        .defense_proc = mobproc_acid_reflect,
    },

    /* ─── Bosses. Every row is baseline/non-enraged combat stats plus the buff/damage data
       that's the same on every turn of the fight — each fight itself is scripted and
       multi-phase (arena sealing, phase-shifted attacks, summons; one boss is stat-less by
       design) in ways a table row can't capture. This is "what it hits like on a normal
       turn," not "here's the boss fight." ─── */
    [MOB_GOO] = {
        .name = "Goo", .ht = 80,
        .attack_skill = 15, .defense_skill = 12, .dr = 2, /* 30 atk / 5-30 dmg once "pumped up" */
        .dmg_min = 2, .dmg_max = 12,
        .view_distance = 8, .xp_reward = 10, .base_speed = 1.0f, .attack_delay = 1.0f,
        .dmg_resist = DMGBIT(DMGSRC_TOXIC_GAS) | DMGBIT(DMGSRC_DEATH) | DMGBIT(DMGSRC_PSIONIC),
        /* 33%: Buff.affect(Ooze) */
        .on_hit = {{ .one_in = 3, .buff = BUFF_OOZE }},
        /* TODO(ai): regenerates in water; roughly every 3rd turn it pumps up, then either melees
           at +accuracy or leaps at a ranged target next turn — no pump/leap phase modeled here.
           Fight also seals the level exits */
    },
    [MOB_TENGU] = {
        .name = "Tengu", .ht = 120,
        .attack_skill = 20, .defense_skill = 20, .dr = 5,
        .dmg_min = 8, .dmg_max = 15,
        .view_distance = 8, .xp_reward = 20, .base_speed = 1.0f, .attack_delay = 1.0f,
        .dmg_resist = DMGBIT(DMGSRC_TOXIC_GAS) | DMGBIT(DMGSRC_POISON) | DMGBIT(DMGSRC_DEATH) | DMGBIT(DMGSRC_PSIONIC),
        /* TODO(ai): teleports every 5 turns (or sooner if it can't close in), arming random
           nearby traps as poison traps each time */
    },
    [MOB_DM300] = {
        .name = "DM-300", .ht = 200,
        .attack_skill = 28, .defense_skill = 18, .dr = 10,
        .dmg_min = 18, .dmg_max = 24,
        .view_distance = 8, .xp_reward = 30, .base_speed = 1.0f, .attack_delay = 1.0f,
        .dmg_immune = DMGBIT(DMGSRC_TOXIC_GAS),
        .dmg_resist = DMGBIT(DMGSRC_DEATH) | DMGBIT(DMGSRC_PSIONIC),
        /* TODO(ai): vents toxic gas every turn, self-repairs on inactive traps, and paralyses +
           rockslides a random adjacent cell on every move */
    },
    [MOB_KING] = {
        .name = "King of Dwarves", .ht = 300,
        .attack_skill = 32, .defense_skill = 25, .dr = 14,
        .dmg_min = 20, .dmg_max = 38,
        .view_distance = 8, .xp_reward = 40, .base_speed = 1.0f, .attack_delay = 1.0f,
        .buff_immune = BUFFBIT(BUFF_PARALYSIS) | BUFFBIT(BUFF_VERTIGO),
        .dmg_resist  = DMGBIT(DMGSRC_TOXIC_GAS) | DMGBIT(DMGSRC_DEATH) | DMGBIT(DMGSRC_PSIONIC) | DMGBIT(DMGSRC_DISINTEGRATION),
        /* TODO(ai): shuttles between two pedestals summoning up to 5 Undead (MOB_KING_UNDEAD
           below); its army cap grows as King loses HP */
    },
    [MOB_KING_UNDEAD] = {
        .name = "undead dwarf", .ht = 28,
        .attack_skill = 16, .defense_skill = 15, .dr = 5,
        .dmg_min = 12, .dmg_max = 16,
        .view_distance = 8, .xp_reward = 0, .base_speed = 1.0f, .attack_delay = 1.0f,
        .initial_state = MOB_WANDERING,
        .buff_immune = BUFFBIT(BUFF_PARALYSIS),
        .dmg_immune  = DMGBIT(DMGSRC_DEATH),
        /* 20%: Buff.prolong(Paralysis, 1) */
        .on_hit = {{ .one_in = 5, .buff = BUFF_PARALYSIS, .dur_min = 1 }},
        /* TODO: ToxicGas damage clears the gas cloud instead (needs blobs) */
    },
    [MOB_YOG] = {
        .name = "Yog-Dzewa", .ht = 300,
        .attack_skill = 0, .defense_skill = 0, .dr = 0,  /* not a typo, see below */
        .dmg_min = 1, .dmg_max = 1,
        .view_distance = 8, .xp_reward = 50, .base_speed = 1.0f, .attack_delay = 1.0f,
        .initial_state = MOB_PASSIVE,
        .buff_immune = BUFFBIT(BUFF_TERROR) | BUFFBIT(BUFF_AMOK) | BUFFBIT(BUFF_CHARM) | BUFFBIT(BUFF_SLEEP) | BUFFBIT(BUFF_BURNING),
        .dmg_immune  = DMGBIT(DMGSRC_DEATH) | DMGBIT(DMGSRC_TOXIC_GAS) | DMGBIT(DMGSRC_PSIONIC),
        /* Yog's body never overrides attackSkill/dr/damageRoll in source at all — it's a
           stationary HP pool, incapable of fighting you directly by design. Its two fists
           (below) do 100% of the offense. Every hit it takes spawns a Larva next to it. */
        .defense_proc = mobproc_spawn_larva,
        /* TODO: damage TO Yog is halved per living fist (dmg >>= fistsCount) — needs a
           pre-damage hook in char_damage plus the fight script that spawns the fists */
    },
    [MOB_YOG_ROTTING_FIST] = {
        .name = "rotting fist", .ht = 300,
        .attack_skill = 36, .defense_skill = 25, .dr = 15,
        .dmg_min = 24, .dmg_max = 36,
        .view_distance = 8, .xp_reward = 0, .base_speed = 1.0f, .attack_delay = 1.0f,
        .initial_state = MOB_WANDERING,
        .buff_immune = BUFFBIT(BUFF_AMOK) | BUFFBIT(BUFF_SLEEP) | BUFFBIT(BUFF_TERROR) | BUFFBIT(BUFF_POISON) | BUFFBIT(BUFF_VERTIGO),
        .dmg_immune  = DMGBIT(DMGSRC_POISON),
        .dmg_resist  = DMGBIT(DMGSRC_TOXIC_GAS) | DMGBIT(DMGSRC_DEATH) | DMGBIT(DMGSRC_PSIONIC),
        .on_hit = {{ .one_in = 3, .buff = BUFF_OOZE }},   /* 33% */
        /* TODO(ai): regenerates +4 HP/turn standing in water */
    },
    [MOB_YOG_BURNING_FIST] = {
        .name = "burning fist", .ht = 200,
        .attack_skill = 36, .defense_skill = 25, .dr = 15,
        .dmg_min = 20, .dmg_max = 32,
        .view_distance = 8, .xp_reward = 0, .base_speed = 1.0f, .attack_delay = 1.0f,
        .initial_state = MOB_WANDERING,
        .buff_immune = BUFFBIT(BUFF_AMOK) | BUFFBIT(BUFF_SLEEP) | BUFFBIT(BUFF_TERROR) | BUFFBIT(BUFF_BURNING),
        .dmg_resist  = DMGBIT(DMGSRC_TOXIC_GAS) | DMGBIT(DMGSRC_DEATH) | DMGBIT(DMGSRC_PSIONIC),
        /* TODO(ai): purely ranged (LOS, no adjacency needed at all — closer to Eye than to
           Shaman/Warlock's melee-or-ranged split); seeds fire in all 9 neighbouring cells
           every turn */
    },
    [MOB_YOG_LARVA] = {
        .name = "god's larva", .ht = 25,
        .attack_skill = 30, .defense_skill = 20, .dr = 8,
        .dmg_min = 15, .dmg_max = 20,
        .view_distance = 8, .xp_reward = 0, .base_speed = 1.0f, .attack_delay = 1.0f,
        .initial_state = MOB_HUNTING,
    },

    /* ─── Spawned outside Bestiary's per-depth roll by specific triggers, not the normal floor
       population. The ones that scale with Dungeon.depth in source have a .scale function
       (above) instead of numbers: ht/attack/defense/dr/damage/xp that a .scale sets are
       deliberately left out of their rows. ─── */
    [MOB_FETID_RAT] = {
        .name = "fetid rat", .ht = 15,    /* fixed, not depth-scaled */
        .attack_skill = 12, .defense_skill = 5, .dr = 2,
        .dmg_min = 2, .dmg_max = 6,
        .view_distance = 8, .xp_reward = 3, .base_speed = 1.0f, .attack_delay = 1.0f,
        .initial_state = MOB_WANDERING,
        .buff_immune = BUFFBIT(BUFF_PARALYSIS),
        /* TODO(proc): defense_proc — seeds a 20-unit paralytic gas cloud at its own position
           when hit (needs blobs) */
    },
    [MOB_MIMIC] = {
        .name = "mimic",
        .view_distance = 8, .base_speed = 1.0f, .attack_delay = 1.0f,
        .initial_state = MOB_HUNTING,
        .scale = scale_mimic,       /* ht, attack, defense, damage, xp */
        .dmg_immune = DMGBIT(DMGSRC_PSIONIC),
        /* TODO(proc): attack_proc — 1-in-3 chance to take some of the hero's own gold on hit
           (hero only) */
    },
    [MOB_STATUE] = {
        .name = "animated statue",
        .dmg_min = 1, .dmg_max = 6,  /* NOT sourced, see below */
        .view_distance = 8, .base_speed = 1.0f, .attack_delay = 1.0f,
        .initial_state = MOB_PASSIVE,
        .scale = scale_statue,      /* ht, attack, defense, dr */
        .dmg_resist = DMGBIT(DMGSRC_TOXIC_GAS) | DMGBIT(DMGSRC_POISON) | DMGBIT(DMGSRC_DEATH) | DMGBIT(DMGSRC_PSIONIC),
        .flags = MOBF_HIT_WAKES_PASSIVE,   /* only wakes if actually hit */
        /* TODO(proc): damage/attack_delay (and the accuracy multiplier in scale_statue) are a
           placeholder assuming a basic, unenchanted weapon: source rolls an actual random melee
           weapon at construction and reads accuracy/min/max/attackDelay straight off it, and
           attackProc() runs that weapon's enchantment. Both want a weapon system. */
    },
    [MOB_WRAITH] = {
        .name = "wraith", .ht = 1,   /* fixed at 1 at every depth — glass cannon */
        .view_distance = 8, .base_speed = 1.0f, .attack_delay = 1.0f,
        .flying = 1,
        .initial_state = MOB_HUNTING,
        .scale = scale_wraith,      /* attack, defense, damage */
        .buff_immune = BUFFBIT(BUFF_TERROR),
        .dmg_immune  = DMGBIT(DMGSRC_DEATH),
        /* event-spawned (e.g. a disturbed statue or tomb), not a normal wandering resident —
           see Wraith.spawnAt()/spawnAround() in source */
    },
    [MOB_PIRANHA] = {
        .name = "giant piranha",
        .view_distance = 8, .base_speed = 2.0f, .attack_delay = 1.0f,
        .scale = scale_piranha,     /* ht, attack, defense, dr, damage */
        .buff_immune = BUFFBIT(BUFF_BURNING) | BUFFBIT(BUFF_PARALYSIS) | BUFFBIT(BUFF_ROOTS) | BUFFBIT(BUFF_FROST),
        .dmg_immune  = DMGBIT(DMGSRC_TOXIC_GAS),
        /* TODO(ai): instantly dies the moment it's outside water; movement is water-tile-
           restricted pathing, not the normal wander/hunt walk */
    },
    [MOB_CURSE_PERSONIFICATION] = {
        .name = "curse personification", .dr = 1,   /* damage and dr are fixed, not depth-scaled */
        .dmg_min = 3, .dmg_max = 5,
        .view_distance = 8, .xp_reward = 3, .base_speed = 0.5f, .attack_delay = 1.0f,
        .flying = 1,
        .initial_state = MOB_HUNTING,
        .scale = scale_curse,       /* ht, attack, defense */
        .buff_immune = BUFFBIT(BUFF_TERROR) | BUFFBIT(BUFF_PARALYSIS) | BUFFBIT(BUFF_ROOTS),
        .dmg_immune  = DMGBIT(DMGSRC_DEATH),
        /* TODO(proc): attack_proc knocks the target back a tile (needs a way to shove a Char
           without char_move()'s Vertigo redirect); regenerates 1 HP/turn; spawned when a cursed
           item resists forced removal, and on death it turns into a passive Ghost NPC rather
           than dying normally */
    },
};

void mob_stats_resolve(MobStats *out, MobKind kind, int depth) {
    *out = MOB_DB[kind];
    if (out->scale) out->scale(out, depth + 1);   /* source's Dungeon.depth is 1-based */
}