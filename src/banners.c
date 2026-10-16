#include "banners.h"

typedef struct {
    uint8_t x, y, w, h;
} BannerInfo;

static const BannerInfo BANNERS[BANNER_COUNT] = {
    [BANNER_PIXEL_DUNGEON]       = { 0,   0, 128, 70 },
    [BANNER_BOSS_SLAIN]          = { 0,  70, 128, 35 },
    [BANNER_GAME_OVER]           = { 0, 105, 128, 35 },
    [BANNER_SELECT_YOUR_HERO]    = { 0, 140, 128, 21 },
    [BANNER_PIXEL_DUNGEON_SIGNS] = { 0, 161, 128, 57 }
};

void banner_sprite(BannerId banner, SDL_Rect *rect) {
    if (!rect || banner < 0 || banner >= BANNER_COUNT) {
        if (rect)
            *rect = (SDL_Rect){0, 0, 0, 0};
        return;
    }

    rect->x = BANNERS[banner].x;
    rect->y = BANNERS[banner].y;
    rect->w = BANNERS[banner].w;
    rect->h = BANNERS[banner].h;
}