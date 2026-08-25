#pragma once
#include "mob_db.h"

typedef struct {
    Char base;

    MobKind    kind;
    MobAiState state;
    Char      *target;
    int        dest;
    int        enemy_seen;
    int        alerted;

    int        alert_icon;   /* show alert icon for one mob turn */

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

void mob_set_kill_reward_hook(void (*fn)(Char *killer, int xp));

void mob_set_fog(const uint8_t *visible);