#include "visuals.h"

typedef struct {
    uint8_t x, y, w, h;
} SpriteRect;

static void sprite_get(const SpriteRect *sprites, int count, int index, SDL_Rect *rect) {
    if (!rect)
        return;

    if (!sprites || index < 0 || index >= count) {
        *rect = (SDL_Rect){0};
        return;
    }

    SpriteRect s = sprites[index];

    *rect = (SDL_Rect) {
        .x = s.x,
        .y = s.y,
        .w = s.w,
        .h = s.h
    };
}


static const SpriteRect BANNERS[BANNER_COUNT] = {
    [BANNER_PIXEL_DUNGEON]       = { 0,   0, 128, 70 },
    [BANNER_BOSS_SLAIN]          = { 0,  70, 128, 35 },
    [BANNER_GAME_OVER]           = { 0, 105, 128, 35 },
    [BANNER_SELECT_YOUR_HERO]    = { 0, 140, 128, 21 },
    [BANNER_PIXEL_DUNGEON_SIGNS] = { 0, 161, 128, 57 }
};

void banner_sprite(BannerId banner, SDL_Rect *rect) {
    sprite_get(BANNERS, BANNER_COUNT, banner, rect);
}

static const SpriteRect ICONS[ICON_COUNT] = {
    [ICON_SKULL]          = {  0,  0,  8,  8 },
    [ICON_BUSY]           = {  8,  0,  8,  8 },
    [ICON_COMPASS]        = {  0,  8,  7,  5 },
    [ICON_PREFS]          = { 30,  0, 16, 16 },
    [ICON_WARNING]        = { 46,  0, 12, 12 },
    [ICON_TARGET]         = {  0, 13, 16, 16 },
    [ICON_WATA]           = { 30, 16, 15, 10 },
    [ICON_WARRIOR]        = {  0, 29, 16, 16 },
    [ICON_MAGE]           = { 16, 29, 16, 16 },
    [ICON_ROGUE]          = { 32, 29, 16, 16 },
    [ICON_HUNTRESS]       = { 48, 29, 16, 16 },
    [ICON_CLOSE]          = {  0, 45, 13, 13 },
    [ICON_DEPTH]          = { 45, 12,  9,  8 },
    [ICON_SLEEP]          = { 13, 45,  9,  8 },
    [ICON_ALERT]          = { 22, 45,  8,  8 },
    [ICON_SUPPORT]        = { 30, 45, 16, 16 },
    [ICON_SUPPORTED]      = { 46, 45, 16, 16 },
    [ICON_BACKPACK]       = { 58,  0, 10, 10 },
    [ICON_SCROLL_HOLDER]  = { 68,  0, 10, 10 },
    [ICON_SEED_POUCH]     = { 78,  0, 10, 10 },
    [ICON_WAND_HOLSTER]   = { 88,  0, 10, 10 },
    [ICON_KEYRING]        = { 64, 29, 10, 10 },
    [ICON_CHECKED]        = { 54, 12, 12, 12 },
    [ICON_UNCHECKED]      = { 66, 12, 12, 12 },
    [ICON_EXIT]           = { 98,  0, 16, 16 },
    [ICON_CHALLENGE_OFF]  = { 78, 16, 24, 24 },
    [ICON_CHALLENGE_ON]   = {102, 16, 24, 24 },
    [ICON_RESUME]         = {114,  0, 12, 11 }
};

/*
IconId hero_icon(HeroClass cls) {
    switch (cls) {
    case HERO_WARRIOR:  return ICON_WARRIOR;
    case HERO_MAGE:     return ICON_MAGE;
    case HERO_ROGUE:    return ICON_ROGUE;
    case HERO_HUNTRESS: return ICON_HUNTRESS;
    default:            return ICON_SKULL;
    }
}
*/

IconId bag_icon(BagType type) {
    switch (type) {
    case BAG_BACKPACK:      return ICON_BACKPACK;
    case BAG_SEED_POUCH:    return ICON_SEED_POUCH;
    case BAG_SCROLL_HOLDER: return ICON_SCROLL_HOLDER;
    case BAG_WAND_HOLSTER:  return ICON_WAND_HOLSTER;
    case BAG_KEYRING:       return ICON_KEYRING;
    default:                return ICON_BACKPACK;
    }
}

void icon_sprite(IconId icon, SDL_Rect *rect) {
    sprite_get(ICONS, ICON_COUNT, icon, rect);
}

void speck_sprite(int type, SDL_Rect *rect) {
    if (!rect)
        return;

    int frame;

    switch (type) {
    case SPECK_DISCOVER:
        frame = SPECK_LIGHT;
        break;

    case SPECK_EVOKE:
    case SPECK_MASTERY:
    case SPECK_KIT:
    case SPECK_FORGE:
        frame = SPECK_STAR;
        break;

    case SPECK_RATTLE:
        frame = SPECK_BONE;
        break;

    case SPECK_JET:
    case SPECK_TOXIC:
    case SPECK_PARALYSIS:
    case SPECK_CONFUSION:
    case SPECK_DUST:
        frame = SPECK_STEAM;
        break;

    default:
        frame = type;
        break;
    }

    *rect = (SDL_Rect) {
        .x = frame * 7,
        .y = 0,
        .w = 7,
        .h = 7
    };
}

//TODO ADD EFFECTS + BUFF ICONS