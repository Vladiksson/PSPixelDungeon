#include "dungeon.h"
#include "mob.h" //redo temporar solution for the test
#include <time.h>

static DungeonTheme get_theme(int depth){
    if (depth < 5 ) return THEME_SEWERS;
    if (depth < 10) return THEME_PRISON;
    if (depth < 15) return THEME_CAVES;
    if (depth < 20) return THEME_CITY;
    return THEME_HALLS;
}

Level *dungeon_current_level(Dungeon *d){
    return &d->floors[d->current_depth];
}

Fog *dungeon_current_fog(Dungeon *d){
    return &d->fog[d->current_depth];
}

void dungeon_init(Dungeon *d){
    memset(d, 0, sizeof(*d));

    d->world_seed = (uint32_t)time(NULL);
    d->current_depth = 0;

    d->blacksmith_depth = 11 + (d->world_seed % 3); 

    level_generate(&d->floors[0], d->world_seed, get_theme(0), d->current_depth, d->blacksmith_depth);
    fog_init(&d->fog[d->current_depth]);

    d->generated[0] = 1;

    char_set_active_level(&d->floors[0]); 
}

int dungeon_go_down(Dungeon *d, Player *player, Camera *cam){

    if (d->current_depth + 1 >= MAX_FLOORS)
        return 0;

    mob_unload_all();

    d->current_depth++;

    int freshly_generated = !d->generated[d->current_depth];
    if (freshly_generated) {
        uint32_t seed = d->world_seed + d->current_depth * 0x9E3779B9u;
        level_generate(&d->floors[d->current_depth], seed, get_theme(d->current_depth), d->current_depth, d->blacksmith_depth);

        d->generated[d->current_depth] = 1;
    }

    Level *lvl = dungeon_current_level(d);
    player->base.pos = lvl->entrance;

    char_clear();
    actor_clear();
    char_set_active_level(lvl);

    mob_set_pool(&lvl->mob_pool);

    char_add(&player->base);

    actor_fix_time();
    mob_restore_grid();
    mob_restore_targets(&player->base);

    Point p = idx_to_xy(player->base.pos);
    camera_center_on(cam, p.x, p.y);

    /* 2 = this floor was freshly generated this call (nothing's been
       spawned on it yet — main.c uses this to know when to call
       mob_spawn_random()); 1 = you'd already been here. Callers that
       only care about success/failure can still just check truthiness. */
    return freshly_generated ? 2 : 1;
}

int dungeon_go_up(Dungeon *d, Player *player, Camera *cam){

    if (d->current_depth == 0)
        return 0;

    mob_unload_all();
    
    d->current_depth--;

    Level *lvl = dungeon_current_level(d);
    player->base.pos = lvl->exit;

    char_clear();
    actor_clear();
    char_set_active_level(lvl);

    mob_set_pool(&lvl->mob_pool);

    char_add(&player->base);

    actor_fix_time();
    mob_restore_grid();
    mob_restore_targets(&player->base);

    Point p = idx_to_xy(player->base.pos);
    camera_center_on(cam, p.x, p.y);

    return 1;
}

void dungeon_on_player_move(Dungeon *dungeon, Player *player){
    Level *lvl = dungeon_current_level(dungeon);

    /* Halls boss room: seal the entrance once the player enters. */
    if (dungeon->current_depth != 24)
        return;

    if (lvl->entrance == -1)
        return;

    if (player->base.pos == lvl->entrance)
        return;

    const int room_left   = LEVEL_W / 2 - 1;
    const int room_right  = LEVEL_W / 2 + 1;
    const int room_top    = LEVEL_H / 2 - 1;
    const int room_bottom = LEVEL_H / 2 + 1;

    for (int x = room_left - 1; x <= room_right + 1; x++) {
        lvl->map[xy_to_idx(x, room_top - 1)]    = TILE_EMPTY_SP;
        lvl->map[xy_to_idx(x, room_bottom + 1)] = TILE_EMPTY_SP;
    }

    for (int y = room_top; y <= room_bottom; y++) {
        lvl->map[xy_to_idx(room_left - 1, y)]   = TILE_EMPTY_SP;
        lvl->map[xy_to_idx(room_right + 1, y)]  = TILE_EMPTY_SP;
    }

    lvl->map[lvl->entrance] = TILE_EMPTY_SP;
    lvl->entrance = -1;
}