#pragma once
#include <stdint.h>
#include <SDL2/SDL.h>
#include "particle.h"
#include "bag.h"

typedef enum {
    BANNER_PIXEL_DUNGEON,
    BANNER_BOSS_SLAIN,
    BANNER_GAME_OVER,
    BANNER_SELECT_YOUR_HERO,
    BANNER_PIXEL_DUNGEON_SIGNS,
    BANNER_COUNT
} BannerId;

void banner_sprite(BannerId banner, SDL_Rect *rect);

typedef enum {
    ICON_SKULL,
    ICON_BUSY,
    ICON_COMPASS,
    ICON_PREFS,
    ICON_WARNING,
    ICON_TARGET,
    ICON_WATA,
    ICON_WARRIOR,
    ICON_MAGE,
    ICON_ROGUE,
    ICON_HUNTRESS,
    ICON_CLOSE,
    ICON_DEPTH,
    ICON_SLEEP,
    ICON_ALERT,
    ICON_SUPPORT,
    ICON_SUPPORTED,
    ICON_BACKPACK,
    ICON_SEED_POUCH,
    ICON_SCROLL_HOLDER,
    ICON_WAND_HOLSTER,
    ICON_KEYRING,
    ICON_CHECKED,
    ICON_UNCHECKED,
    ICON_EXIT,
    ICON_CHALLENGE_OFF,
    ICON_CHALLENGE_ON,
    ICON_RESUME,

    ICON_COUNT
} IconId;


/*IconId hero_icon(HeroClass cls);*/
IconId bag_icon(BagType type);

void icon_sprite(IconId icon, SDL_Rect *rect);

void speck_sprite(int type, SDL_Rect *rect);

typedef enum {
    EFFECT_RIPPLE,
    EFFECT_LIGHTNING,
    EFFECT_WOUND,
    EFFECT_RAY,

    EFFECT_COUNT
} EffectId;

void effect_sprite(EffectId effect, SDL_Rect *rect);