#include "item.h"
#include "level.h"
#include "level_utils.h"
#include <stdio.h>   /* snprintf — item_display_name */

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

ItemType random_item_of_category(ItemCategory cat) {
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
            /* Identify-aware: an unidentified potion/scroll/wand/ring
               on the ground shows its shuffled alias look here too,
               not just in the backpack. */
            item_display_sprite(h->items[0].type, out_col, out_row);
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

/* Shared by heap_add_item/heap_add_stack below: merge into a matching
   stackable entry, or take a fresh slot. Neither rolls or resets
   anything — the two public wrappers decide whether `stack` arrives
   fresh (heap_add_item) or already-instanced (heap_add_stack). */
static int heap_insert(Heap *heap, ItemStack stack) {
    if (ITEM_DB[stack.type].stackable) {
        for (int i = 0; i < heap->count; i++) {
            if (heap->items[i].type == stack.type) {
                heap->items[i].quantity += stack.quantity;
                return 1;
            }
        }
    }
    if (heap->count >= MAX_HEAP_ITEMS)
        return 0;
 
    heap->items[heap->count] = stack;
    heap->count++;
    return 1;
}
 
int heap_add_stack(Heap *heap, ItemStack stack) {
    return heap_insert(heap, stack);
}
 
int heap_add_item(Heap *heap, ItemType type, int quantity) {
    /* Was a bare {type, quantity} field assignment, which left
       level/cursed/durability as whatever garbage sat in this heap
       slot before (heap tables are reused). Routing through
       item_stack_init means every freshly-spawned heap item — every
       spawn_item()/spawn_item_amount_heap() call below included —
       now gets a properly rolled instance for free. */
    ItemStack stack;
    item_stack_init(&stack, type, quantity);
    return heap_insert(heap, stack);
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
static int tile_free_for_item(const Level *lvl, int x, int y) {
    int idx = xy_to_idx(x, y);
    if (idx == lvl->entrance || idx == lvl->exit) 
        return 0;
    return (gTileFlags[lvl->map[idx]] & TF_OPEN_FLOOR) != 0;
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
 
/* ══════════════════════════════════════════════════════════
   INSTANCE STATE — level / curse / durability
   ══════════════════════════════════════════════════════════ */
int item_curse_chance_pct(ItemCategory cat) {
    switch (cat) {
        case ITEMCAT_WEAPON: return 10;
        case ITEMCAT_ARMOR:  return 12;
        case ITEMCAT_WAND:   return 8;
        case ITEMCAT_RING:   return 8;
        default:             return 0;
    }
}
 
int item_max_durability(ItemType type, int level) {
    int base;
    switch (ITEM_DB[type].cat) {
        case ITEMCAT_WEAPON: base = 24 + level * 8; break;
        case ITEMCAT_ARMOR:  base = 30 + level * 8; break;
        case ITEMCAT_WAND:   base = 12 + level * 4; break;
        case ITEMCAT_RING:   base = 20 + level * 4; break;
        default:             base = 1;              break; /* Item.java's own base-class default */
    }
    return base < 1 ? 1 : base;
}
 
void item_stack_init(ItemStack *stack, ItemType type, int quantity) {
    stack->type     = type;
    stack->quantity = quantity;
    stack->level    = 3; //redo
 
    ItemCategory cat = ITEM_DB[type].cat;
    stack->level_known = (cat == ITEMCAT_WEAPON || cat == ITEMCAT_ARMOR) ? 1 : 0;
 
    /* Stackable kinds share one quantity across the whole stack, so
       they can't carry a per-unit curse — only single-instance
       equipables roll one. */
    int chance = ITEM_DB[type].stackable ? 0 : item_curse_chance_pct(cat);
    stack->cursed       = (chance > 0 && rng_int(100) < chance) ? 1 : 0;
    stack->cursed_known = 0;
 
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
    stack->cursed       = 0;
    stack->cursed_known = 1;
    stack->level++;
    item_fix(stack);
}
void item_upgrade_n(ItemStack *stack, int n) {
    for (int i = 0; i < n; i++) item_upgrade(stack);
}
 
void item_degrade(ItemStack *stack) {
    stack->level--;
    item_fix(stack);
}
void item_degrade_n(ItemStack *stack, int n) {
    for (int i = 0; i < n; i++) item_degrade(stack);
}
 
ItemUseResult item_use(ItemStack *stack) {
    if (!(stack->level > 0) || item_is_broken(stack))
        return ITEM_USE_OK;
 
    int max       = item_max_durability(stack->type, stack->level);
    int threshold = (max * ITEM_DURABILITY_WARN_NUM) / ITEM_DURABILITY_WARN_DEN;
    int before    = stack->durability;
 
    stack->durability--;
 
    if (stack->durability <= 0)                             return ITEM_USE_BROKE;
    if (before >= threshold && stack->durability < threshold) return ITEM_USE_WARN_BREAKING;
    return ITEM_USE_OK;
}
 
void item_identify(ItemStack *stack) {
    stack->level_known  = 1;
    stack->cursed_known = 1;
    item_status_know(stack->type);
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
    "kaunan", "sowilo", "laguz", "yngvi", "gyfu", "raido",
    "isaz", "mannaz", "naudiz", "berkanan", "odal", "tiwaz",
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
 
typedef struct {
    ItemCategory       cat;
    const ItemType    *types;
    const uint8_t      *sprites;
    const char *const  *names;
    const char         *suffix;   /* "potion" / "scroll" / "wand" / "ring" */
} ShuffledCategory;
 
static const ShuffledCategory SHUFFLED[] = {
    { ITEMCAT_POTION, POTION_TYPES, POTION_LOOK_SPRITES, POTION_LOOK_NAMES, "potion" },
    { ITEMCAT_SCROLL, SCROLL_TYPES, SCROLL_LOOK_SPRITES, SCROLL_LOOK_NAMES, "scroll" },
    { ITEMCAT_WAND,   WAND_TYPES,   WAND_LOOK_SPRITES,   WAND_LOOK_NAMES,   "wand"   },
    { ITEMCAT_RING,   RING_TYPES,   RING_LOOK_SPRITES,   RING_LOOK_NAMES,   "ring"   },
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
 
    if (!sc || gKnown[type]) {
        *out_col = ITEM_DB[type].sprite_col;
        *out_row = ITEM_DB[type].sprite_row;
        return;
    }
 
    uint8_t sprite = sc->sprites[gLookIndex[type]];
    *out_col = sprite % ITEMS_SHEET_COLS;
    *out_row = sprite / ITEMS_SHEET_COLS;
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
 
