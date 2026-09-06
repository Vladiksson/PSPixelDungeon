#include "dungeon.h"
#include "level_boss.h"
#include <time.h>

Level *dungeon_current_level(Dungeon *d){
    return &d->floors[d->current_depth];
}

Fog *dungeon_current_fog(Dungeon *d){
    return &d->fog[d->current_depth];
}

static void dungeon_create_floor(Dungeon *d, int depth){
    uint32_t seed = d->world_seed + (uint32_t)depth * 0x9E3779B9u;

    mob_set_pool(&d->floors[depth].mob_pool);
    npc_set_pool(&d->floors[depth].npc_pool);

    const int DUNGEON_FLOOR_MAX_ATTEMPTS = 5;
    for (int attempt = 0; attempt < DUNGEON_FLOOR_MAX_ATTEMPTS; attempt++) {
        if (level_generate(&d->floors[depth], seed + (uint32_t)attempt, get_theme(depth), depth, d->blacksmith_depth) == 0)
            break;
    }
    d->generated[depth] = 1;
}

void dungeon_init(Dungeon *d){
    memset(d, 0, sizeof(*d));
    char_clear();
    actor_clear();

    d->world_seed = (uint32_t)time(NULL);
    d->current_depth = 0;

    d->blacksmith_depth = 11 + (d->world_seed % 3); 

    dungeon_create_floor(d, d->current_depth);
    fog_init(&d->fog[d->current_depth]);

    char_set_active_level(&d->floors[d->current_depth]); 
}

int dungeon_go_down(Dungeon *d, Player *player, Camera *cam){

    if (d->current_depth + 1 >= MAX_FLOORS)
        return 0;

    mob_unload_all();
    npc_unload_all();

    d->current_depth++;

    if (!d->generated[d->current_depth]) {
        dungeon_create_floor(d, d->current_depth);
    }

    Level *lvl = dungeon_current_level(d);
    player->base.pos = lvl->entrance;

    char_clear();
    actor_clear();
    char_set_active_level(lvl);

    mob_set_pool(&lvl->mob_pool);
    npc_set_pool(&lvl->npc_pool);

    char_add(&player->base);

    actor_fix_time();
    mob_restore_grid();
    mob_restore_targets(&player->base);
    npc_restore_grid();

    camera_center_on(cam, player->base.pos);

    return 1;
}

int dungeon_go_up(Dungeon *d, Player *player, Camera *cam){

    if (d->current_depth == 0)
        return 0;

    mob_unload_all();
    npc_unload_all();
    
    d->current_depth--;

    Level *lvl = dungeon_current_level(d);
    player->base.pos = lvl->exit;

    char_clear();
    actor_clear();
    char_set_active_level(lvl);

    mob_set_pool(&lvl->mob_pool);
    npc_set_pool(&lvl->npc_pool);

    char_add(&player->base);

    actor_fix_time();
    mob_restore_grid();
    mob_restore_targets(&player->base);
    npc_restore_grid();

    camera_center_on(cam, player->base.pos);

    return 1;
}

void dungeon_event(Dungeon *dungeon, Player *player){
    Level *lvl = dungeon_current_level(dungeon);
    level_boss_event(lvl, dungeon->current_depth, player);
}