#pragma once
#include "mob_db.h"

/* ══════════════════════════════════════════════════════════
   test of mobs system (as part of chars)
   ══════════════════════════════════════════════════════════ */
typedef struct Level Level;

typedef struct {
    Char base;

    MobKind    kind;
    MobAiState state;
    Char      *target;     /* current enemy, mirrors Mob.enemy — see mob_choose_enemy() in mob.c */
    int        dest;       /* wander/hunt/flee destination cell — mirrors Mob.target (an int cell index in the original, unlike this struct's own Char* target above); -1 = none */
    int        enemy_seen; /* mirrors Mob.enemySeen — persists across turns for the sneak-attack check in mob_defense_skill() */
    int        alerted;    /* mirrors Mob.alerted/justAlerted — set in mob_on_damaged(), consumed and cleared at the start of the mob's next turn */

    Facing       facing;
    MobAnimState anim;
    int          frame;
    float        frame_clock;
    int          anim_done;
} Mob;

void mob_play_anim(Mob *m, MobAnimState anim);
void mob_animate_all(float dt); /*mirrors player_animate() */

Mob *mob_spawn(MobKind kind, int pos, Char *target);
int mob_spawn_random(Level *lvl, MobKind kind, int count, Char *target);
int mob_spawn_random_no_target(Level *lvl, MobKind kind, int count);

void mob_spawn_initial(Level *lvl, int depth);

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

void mob_set_hero(Char *hero);

void mob_set_fog(const uint8_t *visible);