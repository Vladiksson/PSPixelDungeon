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
    blob_set_pool(&d->floors[depth].blob_pool);

    blob_clear(&d->floors[depth].blob_pool);

    const int DUNGEON_FLOOR_MAX_ATTEMPTS = 5;
    for (int attempt = 0; attempt < DUNGEON_FLOOR_MAX_ATTEMPTS; attempt++) {
        LevelGenState attempt_state = d->gen_state;

        if (level_generate(&d->floors[depth], seed + (uint32_t)attempt, get_theme(depth), depth, &attempt_state) == 0) {
            d->gen_state = attempt_state;
            d->generated[depth] = 1;
            return;
        }
    }
    
    d->generated[depth] = 0;
}

void dungeon_init(Dungeon *d){
    memset(d, 0, sizeof(*d));
    char_clear();
    actor_clear();

    d->world_seed = (uint32_t)time(NULL);
    d->current_depth = 0;

    d->gen_state.blacksmith_depth = 11 + (d->world_seed % 3);

    blob_set_depth(d->current_depth);

    dungeon_create_floor(d, d->current_depth);
    fog_init(&d->fog[d->current_depth]);

    char_set_active_level(&d->floors[d->current_depth]);
}

static int dungeon_get_down_position(Level *lvl, Level *previous_lvl, int previous_pos) {
    if (previous_pos == previous_lvl->exit)
        return lvl->entrance;

    for (int i = 0; i < previous_lvl->num_rooms; i++) {
        Room *r = &previous_lvl->rooms[i];

        if (!room_inside(r, previous_pos))
            continue;

        if (r->type == ROOM_WEAK_FLOOR) {
            for (int j = 0; j < lvl->num_rooms; j++) {
                Room *next = &lvl->rooms[j];

                if (next->type == ROOM_PIT) {
                    return room_random_passable_point(lvl, next, -1);
                }
            }
        }
    }
    return random_passable_in_box(lvl, 1, 1, LEVEL_W - 1, LEVEL_H - 1, lvl->exit);
}

int dungeon_go_down(Dungeon *d, Player *player, Camera *cam, int pos) {

    if (d->current_depth + 1 >= MAX_FLOORS)
        return 0;

    mob_unload_all();
    npc_unload_all();

    Level *previous_lvl = dungeon_current_level(d);
    int previous_pos = pos;

    d->current_depth++;
    blob_set_depth(d->current_depth);

    if (!d->generated[d->current_depth]) {
        dungeon_create_floor(d, d->current_depth);
    }

    Level *lvl = dungeon_current_level(d);

    pos = dungeon_get_down_position(lvl, previous_lvl, previous_pos);

    if (pos != lvl->entrance)
        player_fell(player);

    player->base.pos = player->cursor_pos = player->target_pos = pos;
    player->attack_target = NULL;

    char_clear();
    actor_clear();
    char_set_active_level(lvl);

    mob_set_pool(&lvl->mob_pool);
    npc_set_pool(&lvl->npc_pool);
    blob_set_pool(&lvl->blob_pool);

    player->base.base.time = 0.0f;
    char_add(&player->base);

    actor_fix_time();
    mob_restore_grid();
    mob_restore_targets(&player->base);
    npc_restore_grid();
    blob_restore_pool(&lvl->blob_pool);
    particle_clear(lvl);

    camera_center_on(cam, player->base.pos);

    return 1;
}

int dungeon_go_up(Dungeon *d, Player *player, Camera *cam){

    if (d->current_depth == 0)
        return 0;

    mob_unload_all();
    npc_unload_all();

    d->current_depth--;
    blob_set_depth(d->current_depth);

    Level *lvl = dungeon_current_level(d);
    player->base.pos = player->cursor_pos = player->target_pos = lvl->exit;
    player->attack_target = NULL;

    char_clear();
    actor_clear();
    char_set_active_level(lvl);

    mob_set_pool(&lvl->mob_pool);
    npc_set_pool(&lvl->npc_pool);
    blob_set_pool(&lvl->blob_pool);

    player->base.base.time = 0.0f;
    char_add(&player->base);

    actor_fix_time();
    mob_restore_grid();
    mob_restore_targets(&player->base);
    npc_restore_grid();
    blob_restore_pool(&lvl->blob_pool);
    particle_clear(lvl);

    camera_center_on(cam, player->base.pos);

    return 1;
}

void dungeon_event(Dungeon *dungeon, Player *player){
    Level *lvl = dungeon_current_level(dungeon);
    level_boss_event(lvl, dungeon->current_depth, player);
}