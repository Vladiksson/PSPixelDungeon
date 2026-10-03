#pragma once
#include "mob.h"

/* attackProc() — return the damage to apply */
int  mobproc_vampiric(Mob *self, Char *enemy, int dmg);       /* Bat: heals for the damage dealt */

/* defenseProc() — return the damage to apply */
int  mobproc_acid_reflect(Mob *self, Char *attacker, int dmg); /* Acidic: 0..dmg of acid back at the attacker */
int  mobproc_swarm_split(Mob *self, Char *attacker, int dmg);  /* Swarm: splits in two */
int  mobproc_spawn_larva(Mob *self, Char *attacker, int dmg);  /* Yog: a Larva next to it, every hit */

/* die() — runs after the Char is already off the map */
void mobproc_skeleton_burst(Mob *self, void *src, DamageSource src_type);

/* add(Buff) — runs after the generic Amok/Terror/Sleep handling */
void mobproc_elemental_frost(Mob *self, Buff *buff);           /* Elemental: Frost hurts it */