#pragma once
#include "level.h"

#define FOV_RADIUS 8

typedef struct {
    uint8_t visible[LEVEL_SIZE];   /* 1 = currently in LOS              */
    uint8_t visited[LEVEL_SIZE];   /* 1 = has been seen (accumulated)   */
} Fog;

void fog_init  (Fog *fog);
void fog_update(Fog *fog, const Level *lvl, int px, int py);