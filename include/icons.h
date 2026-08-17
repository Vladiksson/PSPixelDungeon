#pragma once
//#include "player.h"
#include "bag.h"

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

void icon_sprite(IconId icon, uint8_t *x, uint8_t *y, uint8_t *w, uint8_t *h);