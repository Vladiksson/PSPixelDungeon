#include "avatars.h"

#define AVATAR_W 24
#define AVATAR_H 32

void avatar_sprite(HeroClass hero, SDL_Rect *src) {
    if (!src || hero < HERO_WARRIOR || hero >= HERO_CLASS_COUNT) {
        if (src)
            *src = (SDL_Rect){0, 0, 0, 0};
        return;
    }

    src->x = (int)hero * AVATAR_W;
    src->y = 0;
    src->w = AVATAR_W;
    src->h = AVATAR_H;
}