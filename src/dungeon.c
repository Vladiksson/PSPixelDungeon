#include "dungeon.h"
#include <time.h>

static DungeonTheme get_theme(int depth)
{
    if (depth < 1) return THEME_SEWERS;
    if (depth < 2) return THEME_PRISON;
    if (depth < 3) return THEME_CAVES;
    if (depth < 4) return THEME_CITY;
    return THEME_HALLS;
}

Level *dungeon_current_level(Dungeon *d)
{
    return &d->floors[d->current_depth];
}

Fog *dungeon_current_fog(Dungeon *d)
{
    return &d->fog[d->current_depth];
}

void dungeon_init(Dungeon *d)
{
    memset(d, 0, sizeof(*d));

    d->world_seed = (uint32_t)time(NULL);
    d->current_depth = 0;

    level_generate(
        &d->floors[0],
        d->world_seed,
        get_theme(0));

    fog_init(&d->fog[d->current_depth]);

    d->generated[0] = 1;
}

int dungeon_go_down(Dungeon *d,
                    Player *player,
                    Camera *cam){

    if (d->current_depth + 1 >= MAX_FLOORS)
        return 0;

    d->current_depth++;

    if (!d->generated[d->current_depth]) {
        uint32_t seed = d->world_seed + d->current_depth * 0x9E3779B9u;
        level_generate(
            &d->floors[d->current_depth],
            seed,
            get_theme(d->current_depth));

        d->generated[d->current_depth] = 1;
    }

    Level *lvl = dungeon_current_level(d);
    Point p = idx_to_xy(lvl->entrance);

    player->x = p.x;
    player->y = p.y;

    camera_center_on(cam, player->x, player->y);

    return 1;
}

int dungeon_go_up(Dungeon *d,
                  Player *player,
                  Camera *cam){

    if (d->current_depth == 0)
        return 0;

    d->current_depth--;

    Level *lvl = dungeon_current_level(d);
    Point p = idx_to_xy(lvl->exit);

    player->x = p.x;
    player->y = p.y;

    camera_center_on(cam, player->x, player->y);

    return 1;
}