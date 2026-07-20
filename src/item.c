#include "item.h"
#include "level.h"
#include "level_utils.h"

/*═══════════════════════════════════════════════════════════════
    Item database
═══════════════════════════════════════════════════════════════*/
#define CELL(sprite) (sprite) % ITEMS_SHEET_COLS, (sprite) / ITEMS_SHEET_COLS
 
const ItemDef ITEM_DB[ITEM_TYPE_COUNT] = {
    /* ── weapons ──────────────────────────────────────────── */
    [ITEM_DAGGER]           = { "Dagger",           ITEMCAT_WEAPON, CELL(SPRITE_DAGGER),           0 },
    [ITEM_KNUCKLES]         = { "Knuckles",         ITEMCAT_WEAPON, CELL(SPRITE_KNUCKLEDUSTER),    0 },
    [ITEM_QUARTERSTAFF]     = { "Quarterstaff",     ITEMCAT_WEAPON, CELL(SPRITE_QUARTERSTAFF),     0 },
    [ITEM_SPEAR]            = { "Spear",            ITEMCAT_WEAPON, CELL(SPRITE_SPEAR),            0 },
    [ITEM_MACE]             = { "Mace",             ITEMCAT_WEAPON, CELL(SPRITE_MACE),             0 },
    [ITEM_SWORD]            = { "Sword",            ITEMCAT_WEAPON, CELL(SPRITE_SWORD),            0 },
    [ITEM_LONGSWORD]        = { "Longsword",        ITEMCAT_WEAPON, CELL(SPRITE_LONG_SWORD),       0 },
    [ITEM_BATTLE_AXE]       = { "Battle Axe",       ITEMCAT_WEAPON, CELL(SPRITE_BATTLE_AXE),       0 },
    [ITEM_WAR_HAMMER]       = { "War Hammer",       ITEMCAT_WEAPON, CELL(SPRITE_WAR_HAMMER),       0 },
    [ITEM_GLAIVE]           = { "Glaive",           ITEMCAT_WEAPON, CELL(SPRITE_GLAIVE),           0 },
    [ITEM_SHORT_SWORD]      = { "Short Sword",      ITEMCAT_WEAPON, CELL(SPRITE_SHORT_SWORD),      0 },
    [ITEM_DART]             = { "Dart",             ITEMCAT_WEAPON, CELL(SPRITE_DART),             1 },
    [ITEM_JAVELIN]          = { "Javelin",          ITEMCAT_WEAPON, CELL(SPRITE_JAVELIN),          1 },
    [ITEM_INCENDIARY_DART]  = { "Incendiary Dart",  ITEMCAT_WEAPON, CELL(SPRITE_INCENDIARY_DART),  1 },
    [ITEM_CURARE_DART]      = { "Curare Dart",      ITEMCAT_WEAPON, CELL(SPRITE_CURARE_DART),      1 },
    [ITEM_SHURIKEN]         = { "Shuriken",         ITEMCAT_WEAPON, CELL(SPRITE_SHURIKEN),         1 },
    [ITEM_BOOMERANG]        = { "Boomerang",        ITEMCAT_WEAPON, CELL(SPRITE_BOOMERANG),        1 },
    [ITEM_TAMAHAWK]         = { "Tamahawk",         ITEMCAT_WEAPON, CELL(SPRITE_TOMAHAWK),         1 },
 
    /* ── armor ────────────────────────────────────────────── */
    [ITEM_CLOTH_ARMOR]      = { "Cloth Armor",      ITEMCAT_ARMOR, CELL(SPRITE_ARMOR_CLOTH),   0 },
    [ITEM_LEATHER_ARMOR]    = { "Leather Armor",    ITEMCAT_ARMOR, CELL(SPRITE_ARMOR_LEATHER), 0 },
    [ITEM_MAIL_ARMOR]       = { "Mail Armor",       ITEMCAT_ARMOR, CELL(SPRITE_ARMOR_MAIL),    0 },
    [ITEM_SCALE_ARMOR]      = { "Scale Armor",      ITEMCAT_ARMOR, CELL(SPRITE_ARMOR_SCALE),   0 },
    [ITEM_PLATE_ARMOR]      = { "Plate Armor",      ITEMCAT_ARMOR, CELL(SPRITE_ARMOR_PLATE),   0 },
 
    /* ── wands — 12 shuffled onto the 12 wood looks, Magic Missile fixed ── */
    [ITEM_WAND_TELEPORTATION] = { "Wand of Teleportation",  ITEMCAT_WAND, CELL(SPRITE_WAND_HOLLY),        0 },
    [ITEM_WAND_SLOWNESS]      = { "Wand of Slowness",       ITEMCAT_WAND, CELL(SPRITE_WAND_YEW),          0 },
    [ITEM_WAND_FIREBOLT]      = { "Wand of Firebolt",       ITEMCAT_WAND, CELL(SPRITE_WAND_EBONY),        0 },
    [ITEM_WAND_REGROWTH]      = { "Wand of Regrowth",       ITEMCAT_WAND, CELL(SPRITE_WAND_CHERRY),       0 },
    [ITEM_WAND_POISON]        = { "Wand of Poison",         ITEMCAT_WAND, CELL(SPRITE_WAND_TEAK),         0 },
    [ITEM_WAND_BLINK]         = { "Wand of Blink",          ITEMCAT_WAND, CELL(SPRITE_WAND_ROWAN),        0 },
    [ITEM_WAND_LIGHTNING]     = { "Wand of Lightning",      ITEMCAT_WAND, CELL(SPRITE_WAND_WILLOW),       0 },
    [ITEM_WAND_AMOK]          = { "Wand of Amok",           ITEMCAT_WAND, CELL(SPRITE_WAND_MAHOGANY),     0 },
    [ITEM_WAND_REACH]         = { "Wand of Reach",          ITEMCAT_WAND, CELL(SPRITE_WAND_BAMBOO),       0 },
    [ITEM_WAND_FLOCK]         = { "Wand of Flock",          ITEMCAT_WAND, CELL(SPRITE_WAND_PURPLEHEART),  0 },
    [ITEM_WAND_MAGIC_MISSILE] = { "Wand of Magic Missile",  ITEMCAT_WAND, CELL(SPRITE_WAND_MAGIC_MISSILE),0 },
    [ITEM_WAND_DISINTEGRATION]= { "Wand of Disintegration", ITEMCAT_WAND, CELL(SPRITE_WAND_OAK),          0 },
    [ITEM_WAND_AVALANCHE]     = { "Wand of Avalanche",      ITEMCAT_WAND, CELL(SPRITE_WAND_BIRCH),        0 },
 
    /* ── potions — 12 shuffled onto the 12 color looks ───────────────── */
    [ITEM_POTION_HEALING]      = { "Potion of Healing",       ITEMCAT_POTION, CELL(SPRITE_POTION_TURQUOISE), 1 },
    [ITEM_POTION_EXPERIENCE]   = { "Potion of Experience",    ITEMCAT_POTION, CELL(SPRITE_POTION_CRIMSON),   1 },
    [ITEM_POTION_TOXIC_GAS]    = { "Potion of Toxic Gas",     ITEMCAT_POTION, CELL(SPRITE_POTION_AZURE),     1 },
    [ITEM_POTION_PARALYTIC_GAS]= { "Potion of Paralytic Gas", ITEMCAT_POTION, CELL(SPRITE_POTION_JADE),      1 },
    [ITEM_POTION_LIQUID_FLAME] = { "Potion of Liquid Flame",  ITEMCAT_POTION, CELL(SPRITE_POTION_GOLDEN),    1 },
    [ITEM_POTION_LEVITATION]   = { "Potion of Levitation",    ITEMCAT_POTION, CELL(SPRITE_POTION_MAGENTA),   1 },
    [ITEM_POTION_STRENGTH]     = { "Potion of Strength",      ITEMCAT_POTION, CELL(SPRITE_POTION_CHARCOAL),  1 },
    [ITEM_POTION_MIND_VISION]  = { "Potion of Mind Vision",   ITEMCAT_POTION, CELL(SPRITE_POTION_IVORY),     1 },
    [ITEM_POTION_PURITY]       = { "Potion of Purity",        ITEMCAT_POTION, CELL(SPRITE_POTION_AMBER),     1 },
    [ITEM_POTION_INVISIBILITY] = { "Potion of Invisibility",  ITEMCAT_POTION, CELL(SPRITE_POTION_BISTRE),    1 },
    [ITEM_POTION_MIGHT]        = { "Potion of Might",         ITEMCAT_POTION, CELL(SPRITE_POTION_INDIGO),    1 },
    [ITEM_POTION_FROST]        = { "Potion of Frost",         ITEMCAT_POTION, CELL(SPRITE_POTION_SILVER),    1 },
 
    /* ── scrolls — 12 shuffled onto the 12 rune looks ────────────────── */
    [ITEM_SCROLL_IDENTIFY]      = { "Scroll of Identify",      ITEMCAT_SCROLL, CELL(SPRITE_SCROLL_KAUNAN),   1 },
    [ITEM_SCROLL_TELEPORTATION] = { "Scroll of Teleportation", ITEMCAT_SCROLL, CELL(SPRITE_SCROLL_SOWILO),   1 },
    [ITEM_SCROLL_REMOVE_CURSE]  = { "Scroll of Remove Curse",  ITEMCAT_SCROLL, CELL(SPRITE_SCROLL_LAGUZ),    1 },
    [ITEM_SCROLL_RECHARGING]    = { "Scroll of Recharging",    ITEMCAT_SCROLL, CELL(SPRITE_SCROLL_YNGVI),    1 },
    [ITEM_SCROLL_MAGIC_MAPPING] = { "Scroll of Magic Mapping", ITEMCAT_SCROLL, CELL(SPRITE_SCROLL_GYFU),     1 },
    [ITEM_SCROLL_CHALLENGE]     = { "Scroll of Challenge",     ITEMCAT_SCROLL, CELL(SPRITE_SCROLL_RAIDO),    1 },
    [ITEM_SCROLL_TERROR]        = { "Scroll of Terror",        ITEMCAT_SCROLL, CELL(SPRITE_SCROLL_ISAZ),     1 },
    [ITEM_SCROLL_LULLABY]       = { "Scroll of Lullaby",       ITEMCAT_SCROLL, CELL(SPRITE_SCROLL_MANNAZ),   1 },
    [ITEM_SCROLL_PSIONIC_BLAST] = { "Scroll of Psionic Blast", ITEMCAT_SCROLL, CELL(SPRITE_SCROLL_NAUDIZ),   1 },
    [ITEM_SCROLL_MIRROR_IMAGE]  = { "Scroll of Mirror Image",  ITEMCAT_SCROLL, CELL(SPRITE_SCROLL_BERKANAN), 1 },
    [ITEM_SCROLL_UPGRADE]       = { "Scroll of Upgrade",       ITEMCAT_SCROLL, CELL(SPRITE_SCROLL_ODAL),     1 },
    [ITEM_SCROLL_ENCHANTMENT]   = { "Scroll of Enchantment",   ITEMCAT_SCROLL, CELL(SPRITE_SCROLL_TIWAZ),    1 },
 
    /* ── rings — 12 shuffled onto the 12 gem looks ───────────────────── */
    [ITEM_RING_MENDING]    = { "Ring of Mending",    ITEMCAT_RING, CELL(SPRITE_RING_DIAMOND),    0 },
    [ITEM_RING_DETECTION]  = { "Ring of Detection",  ITEMCAT_RING, CELL(SPRITE_RING_OPAL),       0 },
    [ITEM_RING_SHADOWS]    = { "Ring of Shadows",    ITEMCAT_RING, CELL(SPRITE_RING_GARNET),     0 },
    [ITEM_RING_POWER]      = { "Ring of Power",      ITEMCAT_RING, CELL(SPRITE_RING_RUBY),       0 },
    [ITEM_RING_HERBALISM]  = { "Ring of Herbalism",  ITEMCAT_RING, CELL(SPRITE_RING_AMETHYST),   0 },
    [ITEM_RING_ACCURACY]   = { "Ring of Accuracy",   ITEMCAT_RING, CELL(SPRITE_RING_TOPAZ),      0 },
    [ITEM_RING_EVASION]    = { "Ring of Evasion",    ITEMCAT_RING, CELL(SPRITE_RING_ONYX),       0 },
    [ITEM_RING_SATIETY]    = { "Ring of Satiety",    ITEMCAT_RING, CELL(SPRITE_RING_TOURMALINE), 0 },
    [ITEM_RING_HASTE]      = { "Ring of Haste",      ITEMCAT_RING, CELL(SPRITE_RING_EMERALD),    0 },
    [ITEM_RING_ELEMENTS]   = { "Ring of Elements",   ITEMCAT_RING, CELL(SPRITE_RING_SAPPHIRE),   0 },
    [ITEM_RING_HAGGLER]    = { "Ring of Haggler",    ITEMCAT_RING, CELL(SPRITE_RING_QUARTZ),     0 },
    [ITEM_RING_THORNS]     = { "Ring of Thorns",     ITEMCAT_RING, CELL(SPRITE_RING_AGATE),      0 },
 
    /* ── seeds — fixed sprites ────────────────────────────── */
    [ITEM_SEED_FIREBLOOM]  = { "Firebloom Seed",  ITEMCAT_SEED, CELL(SPRITE_SEED_FIREBLOOM),  1 },
    [ITEM_SEED_ICECAP]     = { "Icecap Seed",     ITEMCAT_SEED, CELL(SPRITE_SEED_ICECAP),     1 },
    [ITEM_SEED_SORROWMOSS] = { "Sorrowmoss Seed", ITEMCAT_SEED, CELL(SPRITE_SEED_SORROWMOSS), 1 },
    [ITEM_SEED_DREAMWEED]  = { "Dreamweed Seed",  ITEMCAT_SEED, CELL(SPRITE_SEED_DREAMWEED),  1 },
    [ITEM_SEED_SUNGRASS]   = { "Sungrass Seed",   ITEMCAT_SEED, CELL(SPRITE_SEED_SUNGRASS),   1 },
    [ITEM_SEED_EARTHROOT]  = { "Earthroot Seed",  ITEMCAT_SEED, CELL(SPRITE_SEED_EARTHROOT),  1 },
    [ITEM_SEED_FADELEAF]   = { "Fadeleaf Seed",   ITEMCAT_SEED, CELL(SPRITE_SEED_FADELEAF),   1 },
    [ITEM_SEED_ROTBERRY]   = { "Rotberry Seed",   ITEMCAT_SEED, CELL(SPRITE_SEED_ROTBERRY),   1 },
 
    /* ── food — fixed sprites ─────────────────────────────── */
    [ITEM_RATION]       = { "Ration of Food", ITEMCAT_FOOD, CELL(SPRITE_RATION), 1 },
    [ITEM_PASTY]        = { "Pasty",          ITEMCAT_FOOD, CELL(SPRITE_PASTY),  1 },
    [ITEM_MYSTERY_MEAT] = { "Mystery Meat",   ITEMCAT_FOOD, CELL(SPRITE_MEAT),   1 },
 
    /* ── misc ─────────────────────────────────────────────── */
    [ITEM_BOMB]     = { "Bomb",     ITEMCAT_MISC, CELL(SPRITE_BOMB),     1 },
    [ITEM_HONEYPOT] = { "Honeypot", ITEMCAT_MISC, CELL(SPRITE_HONEYPOT), 0 },
 
    /* ── currency ─────────────────────────────────────────── */
    [ITEM_GOLD] = { "Gold", ITEMCAT_GOLD, CELL(SPRITE_GOLD), 1 },
 
    /* ── keys — vault-specific, not in the random loot pool ──────── */
    [ITEM_KEY_IRON]     = { "Iron Key",     ITEMCAT_KEY, CELL(SPRITE_IRON_KEY),     0 },
    [ITEM_KEY_GOLDEN]   = { "Golden Key",   ITEMCAT_KEY, CELL(SPRITE_GOLDEN_KEY),   0 },
    [ITEM_KEY_SKELETON] = { "Skeleton Key", ITEMCAT_KEY, CELL(SPRITE_SKELETON_KEY), 0 },
 
    /* ── bags — not in the random loot pool ──────────────────────── */
    [ITEM_BAG_SEED_POUCH]    = { "Seed Pouch",       ITEMCAT_BAG, CELL(SPRITE_POUCH),  0 },
    [ITEM_BAG_SCROLL_HOLDER] = { "Scroll Holder",    ITEMCAT_BAG, CELL(SPRITE_HOLDER), 0 },
    [ITEM_BAG_WAND_HOLSTER]  = { "Magical Holster",  ITEMCAT_BAG, CELL(SPRITE_HOLSTER),0 },
    [ITEM_BAG_KEYRING]       = { "Keyring",          ITEMCAT_BAG, CELL(SPRITE_KEYRING),0 },
 
    /* ── quest — not in the random loot pool ─────────────────────── */
    [ITEM_AMULET_OF_YENDOR] = { "Amulet of Yendor", ITEMCAT_QUEST, CELL(SPRITE_AMULET), 0 },
};
 
#undef CELL

/* ══════════════════════════════════════════════════════════
   RANDOM ITEM SELECTION  —  Generator.random() / Category.prob
   ══════════════════════════════════════════════════════════ */
typedef struct { ItemCategory cat; int weight; } CatWeight;

static const CatWeight SPAWN_WEIGHTS[] = {
    { ITEMCAT_WEAPON, 15 },
    { ITEMCAT_ARMOR,  10 },
    { ITEMCAT_POTION, 50 },
    { ITEMCAT_SCROLL, 40 },
    { ITEMCAT_WAND,    4 },
    { ITEMCAT_RING,    2 },
    { ITEMCAT_SEED,    5 },
    { ITEMCAT_FOOD,    8 },   /* was 0 in Generator.java — see note above */
    { ITEMCAT_GOLD,   50 },
    { ITEMCAT_MISC,    5 },
};
#define N_SPAWN_CATS (int)(sizeof(SPAWN_WEIGHTS) / sizeof(SPAWN_WEIGHTS[0]))

static ItemType random_item_of_category(ItemCategory cat) {
    ItemType pool[ITEM_TYPE_COUNT];
    int n = 0;
    for (int i = 0; i < ITEM_TYPE_COUNT; i++)
        if (ITEM_DB[i].cat == cat) pool[n++] = (ItemType)i;
    return (n > 0) ? pool[rng_int(n)] : ITEM_GOLD;
}

ItemType random_item(void) {
    int total = 0;
    for (int i = 0; i < N_SPAWN_CATS; i++) total += SPAWN_WEIGHTS[i].weight;

    int r = rng_int(total);
    for (int i = 0; i < N_SPAWN_CATS; i++) {
        if (r < SPAWN_WEIGHTS[i].weight)
            return random_item_of_category(SPAWN_WEIGHTS[i].cat);
        r -= SPAWN_WEIGHTS[i].weight;
    }
    return ITEM_GOLD;
}

int gold_amount(int depth) {
    int d = depth + 1;
    return rng_range(20 + d * 10, 40 + d * 20);
}

/* ══════════════════════════════════════════════════════════
   HEAP
   ══════════════════════════════════════════════════════════ */
int heap_sprite_cell(const Heap *h, uint8_t *out_col, uint8_t *out_row) {
    int sprite;
 
    switch (h->type) {
        case HEAP_NORMAL:
        case HEAP_FOR_SALE:
            if (h->count <= 0) return 0;
            *out_col = ITEM_DB[h->items[0].type].sprite_col;
            *out_row = ITEM_DB[h->items[0].type].sprite_row;
            return 1;
 
        case HEAP_CHEST:
        case HEAP_MIMIC:         sprite = SPRITE_CHEST;         break;
        case HEAP_LOCKED_CHEST:  sprite = SPRITE_LOCKED_CHEST;  break;
        case HEAP_CRYSTAL_CHEST: sprite = SPRITE_CRYSTAL_CHEST; break;
        case HEAP_TOMB:          sprite = SPRITE_TOMB;          break;
        case HEAP_SKELETON:      sprite = SPRITE_BONES;         break;
        case HEAP_HIDDEN:        sprite = SPRITE_HIDDEN;        break;
        default: return 0;
    }
 
    *out_col = sprite % ITEMS_SHEET_COLS;
    *out_row = sprite / ITEMS_SHEET_COLS;
    return 1;
}

Heap *level_create_heap(Level *lvl, int x, int y, HeapType type) {
    if (lvl->num_heaps >= MAX_LEVEL_ITEMS)
        return NULL;
    Heap *h = &lvl->heaps[lvl->num_heaps++];
    h->type   = type;
    h->count  = 0;
    h->x      = x;
    h->y      = y;
    h->active = 1;
    return h;
}

Heap *level_heap_at(Level *lvl, int x, int y) {
    for (int i = 0; i < lvl->num_heaps; i++) {
        Heap *h = &lvl->heaps[i];
        if (h->active && h->x == x && h->y == y)
            return h;
    }
    return NULL;
}

Heap *level_heap_at_idx(Level *lvl, int idx) {
    Point p = idx_to_xy(idx);
    return level_heap_at(lvl, p.x, p.y);
}

int heap_add_item(Heap *heap, ItemType type, int quantity) {
    if (ITEM_DB[type].stackable) {
        for (int i = 0; i < heap->count; i++) {
            if (heap->items[i].type == type) {
                heap->items[i].quantity += quantity;
                return 1;
            }
        }
    }
    if (heap->count >= MAX_HEAP_ITEMS)
        return 0;

    heap->items[heap->count].type = type;
    heap->items[heap->count].quantity = quantity;
    heap->count++;
    return 1;
}

/*═══════════════════════════════════════════════════════════════
    Item spawning
═══════════════════════════════════════════════════════════════*/
Heap *spawn_item(Level *lvl, int idx, int depth, ItemType type) {
    if (type < 0 || type >= ITEM_TYPE_COUNT)
        return NULL;
    Point p = idx_to_xy(idx);
    Heap *heap = level_heap_at(lvl, p.x, p.y);
    if (!heap) {
        heap = level_create_heap(lvl, p.x, p.y, HEAP_NORMAL);
        if (!heap)
            return NULL;
    }
    if (!heap_add_item(heap, type, (type == ITEM_GOLD) ? gold_amount(depth) : 1))
        return NULL;
    return heap;
}

Heap *spawn_random_item(Level *lvl, int idx, int depth) {
    return spawn_item(lvl, idx, depth, random_item());
}

Heap *spawn_random_item_of_category(Level *lvl, int idx, int depth, ItemCategory cat) {
    return spawn_item(lvl, idx, depth, random_item_of_category(cat));
}

Heap *spawn_item_heap(Level *lvl, int idx, int depth, ItemType type, HeapType heap_type){
    Heap *heap = spawn_item(lvl, idx, depth, type);
    if (heap) heap->type = heap_type;
    return heap;
}

Heap *spawn_item_amount_heap(Level *lvl, int idx, int depth, ItemType type, int amount, HeapType heap_type){
    if (type < 0 || type >= ITEM_TYPE_COUNT)
        return NULL;
    Point p = idx_to_xy(idx);
    Heap *heap = level_heap_at(lvl, p.x, p.y);
    if (!heap) {
        heap = level_create_heap(lvl, p.x, p.y, heap_type);
        if (!heap)
            return NULL;
    } else {
        heap->type = heap_type;
    }
    int quantity = (amount > 0) ? amount : 1;
    if (!heap_add_item(heap, type, quantity))
        return NULL;
    return heap;
}

Heap *spawn_random_item_of_category_heap(Level *lvl, int idx, int depth, ItemCategory cat, HeapType heap_type){
    Heap *heap = spawn_random_item_of_category(lvl, idx, depth, cat);
    if (heap) heap->type = heap_type;
    return heap;
}

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
static int tile_is_open_floor(uint8_t t) { 
    switch ((TileType)t) {
        case TILE_EMPTY:
        case TILE_GRASS:
        case TILE_HIGH_GRASS:
        case TILE_EMPTY_SP:
        case TILE_EMPTY_DECO:
            return 1;
        default:
            return 0;
    }
}

static int tile_free_for_item(const Level *lvl, int x, int y) {
    int idx = xy_to_idx(x, y);
    if (idx == lvl->entrance || idx == lvl->exit) return 0;
    if (!tile_is_open_floor(lvl->map[idx])) return 0;
    return 1;
}

void level_spawn_items(Level *lvl, int depth) {
    int target = rng_range(4, 8) + depth / 4;
    if (target > MAX_LEVEL_ITEMS)
        target = MAX_LEVEL_ITEMS;
    int attempts = 0;
    while (lvl->num_heaps < target && attempts < 200) {
        attempts++;
        int x = rng_range(0, LEVEL_W - 1);
        int y = rng_range(0, LEVEL_H - 1);
        if (!tile_free_for_item(lvl, x, y))
            continue;
        ItemType type;
        if (level_take_spawn_item(lvl, &type))
            spawn_item(lvl, xy_to_idx(x, y), depth, type);
        else
            spawn_random_item(lvl, xy_to_idx(x, y), depth);
    }
}