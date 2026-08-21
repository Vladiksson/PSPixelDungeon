#include "icons.h"

typedef struct {
    uint8_t x, y, w, h;
} IconInfo;

static const IconInfo ICONS[] = {
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

void icon_sprite(IconId icon, uint8_t *x, uint8_t *y, uint8_t *w, uint8_t *h) {
    *x = ICONS[icon].x;
    *y = ICONS[icon].y;
    *w = ICONS[icon].w;
    *h = ICONS[icon].h;
}
