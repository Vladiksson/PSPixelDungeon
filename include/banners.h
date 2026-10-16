#pragma once
#include <stdint.h>
#include <SDL2/SDL.h>

typedef enum {
    BANNER_PIXEL_DUNGEON,
    BANNER_BOSS_SLAIN,
    BANNER_GAME_OVER,
    BANNER_SELECT_YOUR_HERO,
    BANNER_PIXEL_DUNGEON_SIGNS,
    BANNER_COUNT
} BannerId;

void banner_sprite(BannerId banner, SDL_Rect *rect);