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

/* ranged_check() */
int mobproc_ranged_check_los(Mob *self, Char *enemy, Level *lvl);
int mobproc_eye_ranged_check(Mob *self, Char *enemy, Level *lvl);
int mobproc_scorpio_ranged_check(Mob *self, Char *enemy, Level *lvl);

/* ranged_attack() */
int mobproc_shaman_bolt(Mob *self, Char *enemy, Level *lvl);       /* lightning bolt, 2..11, x1.5 in water */
int mobproc_warlock_bolt(Mob *self, Char *enemy, Level *lvl);      /* shadow bolt, 12..17, 50% Weakness (hero only) */
int mobproc_eye_gaze(Mob *self, Char *enemy, Level *lvl);          /* deathgaze: hits EVERY char along the beam */
int mobproc_burningfist_bolt(Mob *self, Char *enemy, Level *lvl);  /* Yog's fist: plain ranged damageRoll() */
int mobproc_scorpio_bolt(Mob *self, Char *enemy, Level *lvl);
int mobproc_tengu_bolt(Mob *self, Char *enemy, Level *lvl);