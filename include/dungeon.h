#pragma once
#include "level.h"
#include "player.h"
#include "camera.h"
#include "fov.h"

#define MAX_FLOORS 26

typedef struct {
    Level floors[MAX_FLOORS];
    Fog fog[MAX_FLOORS];
    uint8_t generated[MAX_FLOORS];

    int current_depth;
    uint32_t world_seed;
} Dungeon;

void dungeon_init(Dungeon *d);

Level *dungeon_current_level(Dungeon *d);

Fog *dungeon_current_fog(Dungeon *d);

int dungeon_go_down(Dungeon *d, Player *player, Camera *cam);

int dungeon_go_up(Dungeon *d, Player *player, Camera *cam);