#include "item.h"
#include "item_generator.h"
#include "item_spawn.h"
#include "level_geometry.h"

/*═══════════════════════════════════════════════════════════════
    Level item pool
═══════════════════════════════════════════════════════════════*/
void level_add_item_to_spawn(Level *lvl, ItemType type) {
    if (lvl->num_items_to_spawn >= MAX_ITEMS_TO_SPAWN)
        return;
 
    lvl->items_to_spawn[lvl->num_items_to_spawn++] = type;
}
 
int level_take_spawn_item(Level *lvl, ItemType *out) {
    if (lvl->num_items_to_spawn == 0)
        return 0;
 
    if (rng_int(lvl->num_items_to_spawn + 1) == 0)
        return 0;
 
    int i = rng_int(lvl->num_items_to_spawn);
    *out = lvl->items_to_spawn[i];
    lvl->items_to_spawn[i] =
        lvl->items_to_spawn[--lvl->num_items_to_spawn];
    return 1;
}
/* ══════════════════════════════════════════════════════════
   LEVEL POPULATION  —  called once, at the end of level_generate
   ══════════════════════════════════════════════════════════ */
void level_spawn_items(Level *lvl, int depth) {
    int target = rng_range(4, 8) + depth / 4;
    if (target > MAX_LEVEL_ITEMS)
        target = MAX_LEVEL_ITEMS;
    int attempts = 0;
    while (lvl->num_heaps < target && attempts < 200) {
        attempts++;
        int idx = xy_to_idx(rng_range(0, LEVEL_W - 1), rng_range(0, LEVEL_H - 1));

        if (!tile_free_for_item(lvl, idx))
            continue;

        ItemType type;

        if (level_take_spawn_item(lvl, &type))
            spawn_item(lvl, idx, depth, type);
        else
            spawn_random_item(lvl, idx, depth);
    }
}
 

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

    for (int i = 0; i < LOOK_COUNT; i++)
        perm[i] = (uint8_t)i;

    RNG_SHUFFLE(perm, LOOK_COUNT);

    for (int i = 0; i < LOOK_COUNT; i++)
        gLookIndex[sc->types[i]] = perm[i];
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

/* ══════════════════════════════════════════════════════════
    render + ui stuff
   ══════════════════════════════════════════════════════════ */
void item_display_sprite(ItemType type, uint8_t *out_col, uint8_t *out_row) {
    ensure_status_ready();
    const ShuffledCategory *sc = shuffled_category_for(type);

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
        snprintf(buf, bufsz, "%s%+d", item_name(type), stack->level);
    } else {
        snprintf(buf, bufsz, "%s", item_name(type));
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

/* ══════════════════════════════════════════════════════════
   INSTANCE STATE — level / curse / durability
   ══════════════════════════════════════════════════════════ */
int item_max_durability(ItemType type, int level) {
    int base = 0;
    if(ITEM_DB[type].upgradable){
        switch (ITEM_DB[type].cat) {
            case ITEMCAT_WEAPON:
                base = (type == ITEM_BOOMERANG ? 8 : 5) * (level < 16 ? 16 - level : 1);
                break;
            case ITEMCAT_ARMOR:  base = 6 * (level < 16 ? 16 - level : 1); break;
            case ITEMCAT_WAND:   base = 6 * (level < 16 ? 16 - level : 1); break;
            case ITEMCAT_RING:
                base = (level <= 1) ? INT16_MAX : 100 * (level < 16 ? 16 - level : 1);
                break;
            default:
                break; 
        }
    }
    return base;
}
 
void item_stack_init(ItemStack *stack, ItemType type, int quantity) {
    stack->type     = type;
    stack->quantity = quantity;
    stack->level    = 0; 
    stack->cursed = 0;
 
    if (ITEM_DB[type].upgradable) {
        switch (ITEM_DB[type].cat) {
            case ITEMCAT_WEAPON:
                generate_weapon(stack);
                break;
            case ITEMCAT_ARMOR:
                generate_armor(stack);
                break;
            case ITEMCAT_WAND:
                generate_wand(stack);
                break;
            case ITEMCAT_RING:
                generate_ring(stack);
                break;
            default:
                break;
        }
    }
    stack->level_known  = 0; 
    stack->cursed_known = 0;
    stack->durability = (int16_t)item_max_durability(type, stack->level);
}

void item_stack_init_start(ItemStack *stack, ItemType type, int quantity) {
    stack->type     = type;
    stack->quantity = quantity;
    stack->level = stack->cursed = 0;
    stack->level_known = stack->cursed_known = 1; 
    stack->durability = (int16_t)item_max_durability(type, stack->level);
}

 
void item_fix(ItemStack *stack) {
    stack->durability = (int16_t)item_max_durability(stack->type, stack->level);
}
 
void item_polish(ItemStack *stack) {
    int max = item_max_durability(stack->type, stack->level);
    if (stack->durability < max) stack->durability++;
}
 
void item_upgrade(ItemStack *stack) {
    if (!ITEM_DB[stack->type].upgradable)
        return;
    stack->level++;
    stack->cursed       = 0;
    stack->cursed_known = 1;
    item_fix(stack);
}
 
void item_degrade(ItemStack *stack) {
    if (!ITEM_DB[stack->type].upgradable)
        return;
    stack->level--;
    item_fix(stack);
}

 
ItemUseResult item_use(ItemStack *stack) {
    if (!(stack->level > 0) || item_is_broken(stack))
        return ITEM_USE_OK;
 
    int max       = item_max_durability(stack->type, stack->level);
    int threshold = (max * ITEM_DURABILITY_WARN_NUM) / ITEM_DURABILITY_WARN_DEN;
    int before    = stack->durability;
 
    if (stack->durability > 0)
        stack->durability--;

    if (stack->durability == 0)
        return ITEM_USE_BROKE;
    if (before >= threshold && stack->durability < threshold) return ITEM_USE_WARN_BREAKING;
    return ITEM_USE_OK;
}
 
void item_identify(ItemStack *stack) {
    stack->level_known  = 1;
    stack->cursed_known = 1;
    item_status_know(stack->type);
}
 

ItemConsumeResult item_consume(ItemStack *stack) { //temporary bullshit variant for using consumable items
    const ItemDef item = ITEM_DB[stack->type];

    if (!item.stackable || item.cat == ITEMCAT_WEAPON)
        return ITEM_USE_FAIL;

    if (item.cat == ITEMCAT_POTION)
        return ITEM_CONSUME_POTION;
    
    return ITEM_USE_CONSUMED;
}



/*═══════════════════════════════════════════════════════════════
    Weapon / armor stats
═══════════════════════════════════════════════════════════════*/
int weapon_str_req(ItemStack *w) {
    const ItemDef *d = &ITEM_DB[w->type];
    int base = (d->weapon.type == WEAPON_MELEE) ? d->weapon.melee.str : d->weapon.missile.str;
    return base - item_effective_level(w);
}
float weapon_acu(ItemStack *w) {
    const ItemDef *d = &ITEM_DB[w->type];
    return (d->weapon.type == WEAPON_MELEE) ? d->weapon.melee.acu : d->weapon.missile.acu;
}
float weapon_dly(ItemStack *w) {
    const ItemDef *d = &ITEM_DB[w->type];
    return (d->weapon.type == WEAPON_MELEE) ? d->weapon.melee.dly : d->weapon.missile.dly;
}

/* MeleeWeapon.min()/max0() and MissileWeapon subclasses' min()/max(). */
void weapon_min_max(ItemStack *w, int *out_min, int *out_max) {
    const ItemDef *d = &ITEM_DB[w->type];
    int level = item_effective_level(w);

    if (d->weapon.type == WEAPON_MELEE) {
        int tier = d->weapon.melee.tier;
        int min0 = tier;
        int max0 = (int)((tier * tier - tier + 10) / d->weapon.melee.acu * d->weapon.melee.dly);
        *out_min = min0 + level;
        *out_max = max0 + level * tier;
    } else {
        *out_min = d->weapon.missile.dmg_min;
        *out_max = d->weapon.missile.dmg_max;
        if (d->upgradable) {          /* Boomerang is the only one */
            *out_min += level;
            *out_max += level * 2;
        }
    }
}
int weapon_average_damage(ItemStack *w) {
    int min, max;
    weapon_min_max(w, &min, &max);
    return (min + (max - min) / 2);
}

int armor_str_req(ItemStack *a) {
    return ITEM_DB[a->type].armor.str - item_effective_level(a);
}
int armor_dr(ItemStack *stack) {
    int dr = ITEM_DB[stack->type].armor.tier * (2 + item_effective_level(stack)); // TODO add + (glyph == null ? 0 : 1));
    return dr < 0 ? 0 : dr;
}
int get_armor_tier(const ItemStack *stack) {
    if (stack->type == ITEM_NONE) return 0;
    if (ITEM_DB[stack->type].cat != ITEMCAT_ARMOR) return 0;
    return ITEM_DB[stack->type].armor.tier;
}
/*═══════════════════════════════════════════════════════════════
   Missile Weapon 
═══════════════════════════════════════════════════════════════*/

const MissileWeaponDef *missile_def(ItemType type) {
    if (type < 0 || type >= ITEM_TYPE_COUNT)
        return NULL;

    const ItemDef *item = &ITEM_DB[type];

    if (item->cat != ITEMCAT_WEAPON)
        return NULL;

    if (item->weapon.type != WEAPON_MISSILE)
        return NULL;

    return &item->weapon.missile;
}

MissileSpin missile_spin_for(ItemType type) {
    const MissileWeaponDef *m = missile_def(type);
    return m ? m->spin : MISSILE_NO_SPIN;
}

void missile_weapon_proc(ItemType type, Char *defender, int damage) {
    const MissileWeaponDef *m = missile_def(type);

    if (!m || !defender)
        return;

    if (m->proc_buff != BUFF_NONE) {
        char_apply_buff(defender, m->proc_buff, m->proc_duration);
    }

    if (m->flags & MISSILE_F_BLEEDS) {
        char_bleeding_grant(defender, damage);
    }
}