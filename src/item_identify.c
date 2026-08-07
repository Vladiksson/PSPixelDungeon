#include "item.h"
#include "rng.h"
#include <stdio.h>   /* snprintf — item_display_name */
/* ══════════════════════════════════════════════════════════
   IDENTIFY — per-TYPE "known" tracking + shuffled unidentified look
   ══════════════════════════════════════════════════════════ */
#define LOOK_COUNT 12
 
static const ItemType POTION_TYPES[LOOK_COUNT] = {
    ITEM_POTION_HEALING, ITEM_POTION_EXPERIENCE, ITEM_POTION_TOXIC_GAS,
    ITEM_POTION_PARALYTIC_GAS, ITEM_POTION_LIQUID_FLAME, ITEM_POTION_LEVITATION,
    ITEM_POTION_STRENGTH, ITEM_POTION_MIND_VISION, ITEM_POTION_PURITY,
    ITEM_POTION_INVISIBILITY, ITEM_POTION_MIGHT, ITEM_POTION_FROST,
};
static const uint8_t POTION_LOOK_SPRITES[LOOK_COUNT] = {
    SPRITE_POTION_TURQUOISE, SPRITE_POTION_CRIMSON, SPRITE_POTION_AZURE,
    SPRITE_POTION_JADE, SPRITE_POTION_GOLDEN, SPRITE_POTION_MAGENTA,
    SPRITE_POTION_CHARCOAL, SPRITE_POTION_IVORY, SPRITE_POTION_AMBER,
    SPRITE_POTION_BISTRE, SPRITE_POTION_INDIGO, SPRITE_POTION_SILVER,
};
static const char *const POTION_LOOK_NAMES[LOOK_COUNT] = {
    "turquoise", "crimson", "azure", "jade", "golden", "magenta",
    "charcoal", "ivory", "amber", "bistre", "indigo", "silver",
};
 
static const ItemType SCROLL_TYPES[LOOK_COUNT] = {
    ITEM_SCROLL_IDENTIFY, ITEM_SCROLL_TELEPORTATION, ITEM_SCROLL_REMOVE_CURSE,
    ITEM_SCROLL_RECHARGING, ITEM_SCROLL_MAGIC_MAPPING, ITEM_SCROLL_CHALLENGE,
    ITEM_SCROLL_TERROR, ITEM_SCROLL_LULLABY, ITEM_SCROLL_PSIONIC_BLAST,
    ITEM_SCROLL_MIRROR_IMAGE, ITEM_SCROLL_UPGRADE, ITEM_SCROLL_ENCHANTMENT,
};
static const uint8_t SCROLL_LOOK_SPRITES[LOOK_COUNT] = {
    SPRITE_SCROLL_KAUNAN, SPRITE_SCROLL_SOWILO, SPRITE_SCROLL_LAGUZ,
    SPRITE_SCROLL_YNGVI, SPRITE_SCROLL_GYFU, SPRITE_SCROLL_RAIDO,
    SPRITE_SCROLL_ISAZ, SPRITE_SCROLL_MANNAZ, SPRITE_SCROLL_NAUDIZ,
    SPRITE_SCROLL_BERKANAN, SPRITE_SCROLL_ODAL, SPRITE_SCROLL_TIWAZ,
};
static const char *const SCROLL_LOOK_NAMES[LOOK_COUNT] = {
    "\"KAUNAN\"", "\"SOWILO\"", "\"LAGUZ\"", "\"YNGVI\"", "\"GYFU\"", "\"RAIDO\"",
    "\"ISAZ\"", "\"MANNAZ\"", "\"NAUDIZ\"", "\"BERKANAN\"", "\"ODAL\"", "\"TIWAZ\"",
};
 
/* Excludes ITEM_WAND_MAGIC_MISSILE — fixed look, never shuffled. */
static const ItemType WAND_TYPES[LOOK_COUNT] = {
    ITEM_WAND_TELEPORTATION, ITEM_WAND_SLOWNESS, ITEM_WAND_FIREBOLT,
    ITEM_WAND_REGROWTH, ITEM_WAND_POISON, ITEM_WAND_BLINK,
    ITEM_WAND_LIGHTNING, ITEM_WAND_AMOK, ITEM_WAND_REACH,
    ITEM_WAND_FLOCK, ITEM_WAND_DISINTEGRATION, ITEM_WAND_AVALANCHE,
};
static const uint8_t WAND_LOOK_SPRITES[LOOK_COUNT] = {
    SPRITE_WAND_HOLLY, SPRITE_WAND_YEW, SPRITE_WAND_EBONY,
    SPRITE_WAND_CHERRY, SPRITE_WAND_TEAK, SPRITE_WAND_ROWAN,
    SPRITE_WAND_WILLOW, SPRITE_WAND_MAHOGANY, SPRITE_WAND_BAMBOO,
    SPRITE_WAND_PURPLEHEART, SPRITE_WAND_OAK, SPRITE_WAND_BIRCH,
};
static const char *const WAND_LOOK_NAMES[LOOK_COUNT] = {
    "holly", "yew", "ebony", "cherry", "teak", "rowan",
    "willow", "mahogany", "bamboo", "purpleheart", "oak", "birch",
};
 
static const ItemType RING_TYPES[LOOK_COUNT] = {
    ITEM_RING_MENDING, ITEM_RING_DETECTION, ITEM_RING_SHADOWS,
    ITEM_RING_POWER, ITEM_RING_HERBALISM, ITEM_RING_ACCURACY,
    ITEM_RING_EVASION, ITEM_RING_SATIETY, ITEM_RING_HASTE,
    ITEM_RING_ELEMENTS, ITEM_RING_HAGGLER, ITEM_RING_THORNS,
};
static const uint8_t RING_LOOK_SPRITES[LOOK_COUNT] = {
    SPRITE_RING_DIAMOND, SPRITE_RING_OPAL, SPRITE_RING_GARNET,
    SPRITE_RING_RUBY, SPRITE_RING_AMETHYST, SPRITE_RING_TOPAZ,
    SPRITE_RING_ONYX, SPRITE_RING_TOURMALINE, SPRITE_RING_EMERALD,
    SPRITE_RING_SAPPHIRE, SPRITE_RING_QUARTZ, SPRITE_RING_AGATE,
};
static const char *const RING_LOOK_NAMES[LOOK_COUNT] = {
    "diamond", "opal", "garnet", "ruby", "amethyst", "topaz",
    "onyx", "tourmaline", "emerald", "sapphire", "quartz", "agate",
};

typedef enum {
    LOOKDESC_COLOR,
    LOOKDESC_GEM,
    LOOKDESC_RUNE,
    LOOKDESC_WOOD
} LookDescType;

typedef struct {
    ItemCategory       cat;
    const ItemType     *types;
    const uint8_t      *sprites;
    const char *const  *names;
    LookDescType       desc_type;
    const char         *suffix;   /* "potion" / "scroll" / "wand" / "ring" */
} ShuffledCategory;
 
static const ShuffledCategory SHUFFLED[] = {
    { ITEMCAT_POTION, POTION_TYPES, POTION_LOOK_SPRITES, POTION_LOOK_NAMES,  LOOKDESC_COLOR, "potion" },
    { ITEMCAT_SCROLL, SCROLL_TYPES, SCROLL_LOOK_SPRITES, SCROLL_LOOK_NAMES,  LOOKDESC_RUNE,  "scroll" },
    { ITEMCAT_WAND,   WAND_TYPES,   WAND_LOOK_SPRITES,   WAND_LOOK_NAMES,    LOOKDESC_WOOD,  "wand"   },
    { ITEMCAT_RING,   RING_TYPES,   RING_LOOK_SPRITES,   RING_LOOK_NAMES,    LOOKDESC_GEM,   "ring"   },
};
#define N_SHUFFLED (int)(sizeof(SHUFFLED) / sizeof(SHUFFLED[0]))
 
static uint8_t gLookIndex[ITEM_TYPE_COUNT]; /* which of the 12 looks this type shows as   */
static uint8_t gKnown[ITEM_TYPE_COUNT];     /* has the player identified this type yet?   */
static int     gStatusReady = 0;
 
static const ShuffledCategory *shuffled_category_for(ItemType type) {
    if (type == ITEM_WAND_MAGIC_MISSILE) return NULL; /* fixed look, never hidden */
    ItemCategory cat = ITEM_DB[type].cat;
    for (int i = 0; i < N_SHUFFLED; i++) {
        if (SHUFFLED[i].cat == cat) return &SHUFFLED[i];
    }
    return NULL;
}
 
static void shuffle_looks(const ShuffledCategory *sc) {
    uint8_t perm[LOOK_COUNT];
    for (int i = 0; i < LOOK_COUNT; i++) perm[i] = (uint8_t)i;
    for (int i = LOOK_COUNT - 1; i > 0; i--) {
        int j = rng_int(i + 1);
        uint8_t tmp = perm[i]; perm[i] = perm[j]; perm[j] = tmp;
    }
    for (int i = 0; i < LOOK_COUNT; i++) {
        gLookIndex[sc->types[i]] = perm[i];
    }
}
 
void item_status_reset(void) {
    memset(gKnown, 0, sizeof(gKnown));
    for (int i = 0; i < N_SHUFFLED; i++) shuffle_looks(&SHUFFLED[i]);
    gStatusReady = 1;
}
 
static void ensure_status_ready(void) {
    if (!gStatusReady) item_status_reset();
}
 
void item_status_know(ItemType type) {
    ensure_status_ready();
    const ShuffledCategory *sc = shuffled_category_for(type);
    if (!sc) return;
 
    gKnown[type] = 1;
 
    /* Elimination: once every other member of the category is known,
       the last one is obvious too — same deduction
       ItemStatusHandler.know() makes. */
    int unknown_idx = -1, unknown_count = 0;
    for (int i = 0; i < LOOK_COUNT; i++) {
        if (!gKnown[sc->types[i]]) { unknown_idx = i; unknown_count++; }
    }
    if (unknown_count == 1) {
        gKnown[sc->types[unknown_idx]] = 1;
    }
}
 
int item_status_is_known(ItemType type) {
    ensure_status_ready();
    const ShuffledCategory *sc = shuffled_category_for(type);
    return sc ? gKnown[type] : 1;
}
 
void item_display_sprite(ItemType type, uint8_t *out_col, uint8_t *out_row) {
    ensure_status_ready();
    const ShuffledCategory *sc = shuffled_category_for(type);
    /* Potion / Scroll / Wand / Ring
       Always use shuffled look. Identification reveals the name, not the sprite. */
    if (sc) {
        uint8_t sprite = sc->sprites[gLookIndex[type]];

        *out_col = sprite % ITEMS_SHEET_COLS;
        *out_row = sprite / ITEMS_SHEET_COLS;
        return;
    }
    /* Normal items have fixed sprites.*/
    *out_col = ITEM_DB[type].sprite_col;
    *out_row = ITEM_DB[type].sprite_row;
}
 
const char *item_display_name(const ItemStack *stack, char *buf, size_t bufsz) {
    ensure_status_ready();
    ItemType type = stack->type;
    const ShuffledCategory *sc = shuffled_category_for(type);
 
    if (sc && !gKnown[type]) {
        snprintf(buf, bufsz, "%s %s", sc->names[gLookIndex[type]], sc->suffix);
        return buf;
    }
 
    if (stack->level_known && stack->level != 0) {
        snprintf(buf, bufsz, "%s%+d%s", item_name(type), stack->level,
                  item_visibly_cursed(stack) ? " (cursed)" : "");
    } else {
        snprintf(buf, bufsz, "%s%s", item_name(type),
                  item_visibly_cursed(stack) ? " (cursed)" : "");
    }
    return buf;
}
 
const char *item_display_desc(const ItemStack *stack) {
    static char buf[256];
    ensure_status_ready();
    ItemType type = stack->type;
    const ShuffledCategory *sc = shuffled_category_for(type);

    if (sc && !gKnown[type]) {
        const char *look = sc->names[gLookIndex[type]];
        switch(sc->desc_type) {
        case LOOKDESC_GEM:
            snprintf(buf, sizeof(buf), "This metal band is adorned with a large %s gem "
                     "that glitters in the darkness. Who knows what effect it has when worn?", look );
            break;
        case LOOKDESC_COLOR:
            snprintf(buf, sizeof(buf), "This flask contains a swirling %s liquid. "
                     "Who knows what it will do when drunk or thrown?", look);
            break;
        case LOOKDESC_RUNE:
            snprintf(buf, sizeof(buf), "This parchment is covered with indecipherable writing, and bears a title of rune %s. "
                     "Who knows what it will do when read aloud?", look);
            break;
        case LOOKDESC_WOOD:
            snprintf(buf, sizeof(buf),
                "This thin %s wand is warm to the touch. Who knows what it will do when used?", look);
            break;
        }
        return buf;
    }
    return ITEM_DB[type].description;
}