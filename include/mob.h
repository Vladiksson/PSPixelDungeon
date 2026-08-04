#pragma once
#include "char.h"

/* ══════════════════════════════════════════════════════════
   test of mobs system (as part of chars)
   ══════════════════════════════════════════════════════════ */
typedef struct Level Level;

typedef enum {
    MOB_RAT = 0,
    MOB_GNOLL,
    MOB_KIND_COUNT
} MobKind;

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
} MobStats;

extern const MobStats MOB_DB[MOB_KIND_COUNT];

typedef enum {
    MOB_ASLEEP,     /* not acting until something wakes it — see mob_on_turn() */
    MOB_WANDERING,  /* idles/takes the occasional random step, watching for a target */
    MOB_HUNTING,    /* has a target in range: closes in, attacks once adjacent */
} MobAiState;

typedef struct {
    Char base;

    MobKind    kind;
    MobAiState state;
    Char      *target; 
} Mob;

Mob *mob_spawn(MobKind kind, int pos, Char *target);
int mob_spawn_random(Level *lvl, MobKind kind, int count, Char *target);
int mob_spawn_random_no_target(Level *lvl, MobKind kind, int count);

void mob_unload_all(void);
void mob_clear_all(void);

int  mob_count(void);
Mob *mob_at_index(int i); 

#define MAX_LEVEL_MOBS 8

typedef struct {
    Mob mobs[MAX_LEVEL_MOBS];
    int count;
} MobPool;

void mob_set_pool(MobPool *pool);
void mob_restore_grid(void);
void mob_restore_targets(Char *player);