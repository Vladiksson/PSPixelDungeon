#include "dungeon.h"
#include "level_boss.h"
#include <time.h>

Level *dungeon_current_level(Dungeon *d){
    return &d->floors[d->current_depth];
}

Fog *dungeon_current_fog(Dungeon *d){
    return &d->fog[d->current_depth];
}

void dungeon_init(Dungeon *d){
    memset(d, 0, sizeof(*d));
    char_clear();
    actor_clear();

    d->world_seed = (uint32_t)time(NULL);
    d->current_depth = 0;

    d->blacksmith_depth = 11 + (d->world_seed % 3); 

    mob_set_pool(&d->floors[0].mob_pool);
    npc_set_pool(&d->floors[0].npc_pool);

    level_generate(&d->floors[0], d->world_seed, get_theme(0), d->current_depth, d->blacksmith_depth);
    fog_init(&d->fog[d->current_depth]);

    d->generated[0] = 1;

    char_set_active_level(&d->floors[0]); 
}

int dungeon_go_down(Dungeon *d, Player *player, Camera *cam){

    if (d->current_depth + 1 >= MAX_FLOORS)
        return 0;

    mob_unload_all();
    npc_unload_all();

    d->current_depth++;

    if (!d->generated[d->current_depth]) {
        uint32_t seed = d->world_seed + d->current_depth * 0x9E3779B9u;
        mob_set_pool(&d->floors[d->current_depth].mob_pool);
        npc_set_pool(&d->floors[d->current_depth].npc_pool);
        level_generate(&d->floors[d->current_depth], seed, get_theme(d->current_depth), d->current_depth, d->blacksmith_depth);

        d->generated[d->current_depth] = 1;
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

    Point p = idx_to_xy(player->base.pos);
    camera_center_on(cam, p.x, p.y);

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

    Point p = idx_to_xy(player->base.pos);
    camera_center_on(cam, p.x, p.y);

    return 1;
}

void dungeon_event(Dungeon *dungeon, Player *player){
    Level *lvl = dungeon_current_level(dungeon);
    level_boss_event(lvl, dungeon->current_depth, player);
}