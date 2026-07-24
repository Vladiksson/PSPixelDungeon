#pragma once
#include "common.h"

/* ══════════════════════════════════════════════════════════
   ITEM SPRITE SHEET  —  exact port of ItemSpriteSheet.java
   (com.watabou.pixeldungeon.sprites, original watabou/pixel-dungeon)
   ══════════════════════════════════════════════════════════
   Every constant/value below matches the Java source exactly. In
   the original, ItemSprite.pick() turns a flat index into a cell
   with:  cols = textureWidth/SIZE;  row = index/cols; col = index%cols;
   items.png here is 128x256 (measured) = 8 cols x 16 rows of 16px
   cells, so ITEMS_SHEET_COLS is 8 — same formula, this port's atlas.
   ══════════════════════════════════════════════════════════ */
#define ITEMS_SHEET_COLS 8
 
/* Placeholders (generic "unidentified gear" icons — a plain
   silhouette, used before a weapon/armor/ring's true look is known) */
#define SPRITE_WEAPON            5
#define SPRITE_ARMOR             6
#define SPRITE_RING              7
#define SPRITE_SMTH            127
 
/* Keys */
#define SPRITE_SKELETON_KEY      8
#define SPRITE_IRON_KEY          9
#define SPRITE_GOLDEN_KEY       10
 
/* Melee weapons */
#define SPRITE_SHORT_SWORD       2
#define SPRITE_KNUCKLEDUSTER    16
#define SPRITE_QUARTERSTAFF     17
#define SPRITE_MACE             18
#define SPRITE_DAGGER           19
#define SPRITE_SWORD            20
#define SPRITE_LONG_SWORD       21
#define SPRITE_BATTLE_AXE       22
#define SPRITE_WAR_HAMMER       23
#define SPRITE_SPEAR            29
#define SPRITE_GLAIVE           30
 
/* Missiles */
#define SPRITE_SHURIKEN         15
#define SPRITE_DART             31
#define SPRITE_BOOMERANG       106
#define SPRITE_TOMAHAWK        107
#define SPRITE_INCENDIARY_DART 108
#define SPRITE_CURARE_DART     109
#define SPRITE_JAVELIN         110
 
/* Armor */
#define SPRITE_ARMOR_CLOTH      24
#define SPRITE_ARMOR_LEATHER    25
#define SPRITE_ARMOR_MAIL       26
#define SPRITE_ARMOR_SCALE      27
#define SPRITE_ARMOR_PLATE      28
#define SPRITE_ARMOR_ROGUE      96   /* class-specific starting-armor look */
#define SPRITE_ARMOR_WARRIOR    97
#define SPRITE_ARMOR_MAGE       98
#define SPRITE_ARMOR_HUNTRESS   99
 
/* Wands. WAND_MAGIC_MISSILE is the one wand with a fixed, always-
   known sprite (it's the Mage's starting wand in the original) — the
   other 12 wand types share these 12 "wood" looks, reassigned at
   random each run. That's a perfect 12-wand/12-wood match: 13 wand
   classes total = 12 shuffled + 1 fixed. */
#define SPRITE_WAND_MAGIC_MISSILE 3
#define SPRITE_WAND_HOLLY       48
#define SPRITE_WAND_YEW         49
#define SPRITE_WAND_EBONY       50
#define SPRITE_WAND_CHERRY      51
#define SPRITE_WAND_TEAK        52
#define SPRITE_WAND_ROWAN       53
#define SPRITE_WAND_WILLOW      54
#define SPRITE_WAND_MAHOGANY    55
#define SPRITE_WAND_BAMBOO      68
#define SPRITE_WAND_PURPLEHEART 69
#define SPRITE_WAND_OAK         70
#define SPRITE_WAND_BIRCH       71
 
/* Rings — 12 gem looks, shuffled per run across the 12 ring types */
#define SPRITE_RING_DIAMOND     32
#define SPRITE_RING_OPAL        33
#define SPRITE_RING_GARNET      34
#define SPRITE_RING_RUBY        35
#define SPRITE_RING_AMETHYST    36
#define SPRITE_RING_TOPAZ       37
#define SPRITE_RING_ONYX        38
#define SPRITE_RING_TOURMALINE  39
#define SPRITE_RING_EMERALD     72
#define SPRITE_RING_SAPPHIRE    73
#define SPRITE_RING_QUARTZ      74
#define SPRITE_RING_AGATE       75
 
/* Potions — 12 color looks, shuffled per run across the 12 potion types */
#define SPRITE_POTION_TURQUOISE 56
#define SPRITE_POTION_CRIMSON   57
#define SPRITE_POTION_AZURE     58
#define SPRITE_POTION_JADE      59
#define SPRITE_POTION_GOLDEN    60
#define SPRITE_POTION_MAGENTA   61
#define SPRITE_POTION_CHARCOAL  62
#define SPRITE_POTION_IVORY     63
#define SPRITE_POTION_AMBER     64
#define SPRITE_POTION_BISTRE    65
#define SPRITE_POTION_INDIGO    66
#define SPRITE_POTION_SILVER    67
 
/* Scrolls — 12 rune looks, shuffled per run across the 12 scroll types */
#define SPRITE_SCROLL_KAUNAN    40
#define SPRITE_SCROLL_SOWILO    41
#define SPRITE_SCROLL_LAGUZ     42
#define SPRITE_SCROLL_YNGVI     43
#define SPRITE_SCROLL_GYFU      44
#define SPRITE_SCROLL_RAIDO     45
#define SPRITE_SCROLL_ISAZ      46
#define SPRITE_SCROLL_MANNAZ    47
#define SPRITE_SCROLL_NAUDIZ    76
#define SPRITE_SCROLL_BERKANAN  77
#define SPRITE_SCROLL_ODAL      78
#define SPRITE_SCROLL_TIWAZ     79
#define SPRITE_SCROLL_WIPE_OUT 117   /* unused in the original (dup of SPARK_UPGRADE) */
 
/* Seeds — fixed, real sprites (not a shuffled "look") */
#define SPRITE_SEED_FIREBLOOM   88
#define SPRITE_SEED_ICECAP      89
#define SPRITE_SEED_SORROWMOSS  90
#define SPRITE_SEED_DREAMWEED   91
#define SPRITE_SEED_SUNGRASS    92
#define SPRITE_SEED_EARTHROOT   93
#define SPRITE_SEED_FADELEAF    94
#define SPRITE_SEED_ROTBERRY    95
 
/* Quest / unique items — not wired into ItemType yet (each needs its
   own quest logic), sprite constants ready for when they are */
#define SPRITE_ROSE            100
#define SPRITE_PICKAXE         101
#define SPRITE_ORE             102
#define SPRITE_SKULL           103
#define SPRITE_PHANTOM         118
#define SPRITE_DUST            121
#define SPRITE_TOKEN           122
 
/* Heaps — decorative/container sprites, not carryable Items */
#define SPRITE_BONES              0
#define SPRITE_CHEST              11
#define SPRITE_LOCKED_CHEST       12
#define SPRITE_TOMB                13
#define SPRITE_CRYSTAL_CHEST     105
#define SPRITE_HIDDEN            119
 
/* Food — fixed, real sprites */
#define SPRITE_RATION             4
#define SPRITE_PASTY             112
#define SPRITE_MEAT               113
#define SPRITE_STEAK              114
#define SPRITE_OVERPRICED        115  /* Overpriced Ration */
#define SPRITE_CARPACCIO          116
 
/* Bags */
#define SPRITE_POUCH              83   /* Seed Pouch */
#define SPRITE_HOLDER             104  /* Scroll Holder */
#define SPRITE_HOLSTER            111  /* Magical Holster */
#define SPRITE_KEYRING            126
 
/* Misc */
#define SPRITE_ANKH                1
#define SPRITE_GOLD                14
#define SPRITE_STYLUS              80
#define SPRITE_DEWDROP             81
#define SPRITE_MASTERY             82
#define SPRITE_TORCH               84
#define SPRITE_BEACON              85
#define SPRITE_KIT                 86
#define SPRITE_AMULET              87
#define SPRITE_VIAL                120
#define SPRITE_WEIGHT              123
#define SPRITE_BOMB                124
#define SPRITE_HONEYPOT            125
 
/* ══════════════════════════════════════════════════════════
   ITEM SYSTEM
   ══════════════════════════════════════════════════════════
   ItemType/category roster mirrors Generator.java's Category enum
   (WEAPON/ARMOR/POTION/SCROLL/WAND/RING/SEED/FOOD/GOLD/MISC), plus
   keys/bags/the amulet. Each entry's sprite_col/sprite_row is
   computed from the real SPRITE_* constant above via
   ITEMS_SHEET_COLS, so it's exact — not a visual guess. */

typedef enum {
    ITEMCAT_WEAPON,
    ITEMCAT_ARMOR,
    ITEMCAT_WAND,
    ITEMCAT_POTION,
    ITEMCAT_SCROLL,
    ITEMCAT_RING,
    ITEMCAT_SEED,
    ITEMCAT_FOOD,
    ITEMCAT_MISC,
    ITEMCAT_GOLD,
    ITEMCAT_KEY,     /* not in the random loot pool - vault-specific */
    ITEMCAT_BAG,     /* not in the random loot pool                  */
    ITEMCAT_QUEST,   /* not in the random loot pool                  */
    ITEMCAT_COUNT
} ItemCategory;

typedef enum {
    /* ── Weapons — Generator.Category.WEAPON, ────────────────────────────── */
    ITEM_DAGGER = 0,
    ITEM_KNUCKLES,
    ITEM_QUARTERSTAFF,
    ITEM_SPEAR,
    ITEM_MACE,
    ITEM_SWORD,
    ITEM_LONGSWORD,
    ITEM_BATTLE_AXE,
    ITEM_WAR_HAMMER,
    ITEM_GLAIVE,
    ITEM_SHORT_SWORD,
    ITEM_DART,
    ITEM_JAVELIN,
    ITEM_INCENDIARY_DART,
    ITEM_CURARE_DART,
    ITEM_SHURIKEN,
    ITEM_BOOMERANG,
    ITEM_TAMAHAWK,          /* sic — matches Generator.java's spelling */

    /* ── Armor ────────────────────────────────────────────────────── */
    ITEM_CLOTH_ARMOR,
    ITEM_LEATHER_ARMOR,
    ITEM_MAIL_ARMOR,
    ITEM_SCALE_ARMOR,
    ITEM_PLATE_ARMOR,

    /* ── Wands ──────────────────────────────────────────────────── */
    ITEM_WAND_TELEPORTATION,
    ITEM_WAND_SLOWNESS,
    ITEM_WAND_FIREBOLT,
    ITEM_WAND_REGROWTH,
    ITEM_WAND_POISON,
    ITEM_WAND_BLINK,
    ITEM_WAND_LIGHTNING,
    ITEM_WAND_AMOK,
    ITEM_WAND_REACH,
    ITEM_WAND_FLOCK,
    ITEM_WAND_MAGIC_MISSILE,
    ITEM_WAND_DISINTEGRATION,
    ITEM_WAND_AVALANCHE,

    /* ── Potions ─────────────────────────────────────────────── */
    ITEM_POTION_HEALING,
    ITEM_POTION_EXPERIENCE,
    ITEM_POTION_TOXIC_GAS,
    ITEM_POTION_PARALYTIC_GAS,
    ITEM_POTION_LIQUID_FLAME,
    ITEM_POTION_LEVITATION,
    ITEM_POTION_STRENGTH,
    ITEM_POTION_MIND_VISION,
    ITEM_POTION_PURITY,
    ITEM_POTION_INVISIBILITY,
    ITEM_POTION_MIGHT,
    ITEM_POTION_FROST,

    /* ── Scrolls ────────────────────────────────────────────── */
    ITEM_SCROLL_IDENTIFY,
    ITEM_SCROLL_TELEPORTATION,
    ITEM_SCROLL_REMOVE_CURSE,
    ITEM_SCROLL_RECHARGING,
    ITEM_SCROLL_MAGIC_MAPPING,
    ITEM_SCROLL_CHALLENGE,
    ITEM_SCROLL_TERROR,
    ITEM_SCROLL_LULLABY,
    ITEM_SCROLL_PSIONIC_BLAST,
    ITEM_SCROLL_MIRROR_IMAGE,
    ITEM_SCROLL_UPGRADE,
    ITEM_SCROLL_ENCHANTMENT,

    /* ── Rings ─────────────────────────────────────────────── */
    ITEM_RING_MENDING,
    ITEM_RING_DETECTION,
    ITEM_RING_SHADOWS,
    ITEM_RING_POWER,
    ITEM_RING_HERBALISM,
    ITEM_RING_ACCURACY,
    ITEM_RING_EVASION,
    ITEM_RING_SATIETY,
    ITEM_RING_HASTE,
    ITEM_RING_ELEMENTS,
    ITEM_RING_HAGGLER,
    ITEM_RING_THORNS,

    /* ── Seeds ────────────────────────────────────────────── */
    ITEM_SEED_FIREBLOOM,
    ITEM_SEED_ICECAP,
    ITEM_SEED_SORROWMOSS,
    ITEM_SEED_DREAMWEED,
    ITEM_SEED_SUNGRASS,
    ITEM_SEED_EARTHROOT,
    ITEM_SEED_FADELEAF,
    ITEM_SEED_ROTBERRY,

    /* ── Food ────────────────────────────────────────────── */
    ITEM_RATION,
    ITEM_PASTY,
    ITEM_MYSTERY_MEAT,

    /* ── Misc ────────────────────────────────────────────── */
    ITEM_BOMB,
    ITEM_HONEYPOT,

    /* ── Currency ────────────────────────────────────────── */
    ITEM_GOLD,
    /* ── Keys — not in the random loot pool ──────────────── */
    ITEM_KEY_IRON,
    ITEM_KEY_GOLDEN,
    ITEM_KEY_SKELETON,
    /* ── Bags — not in the random loot pool ──────────────── */
    ITEM_BAG_SEED_POUCH,
    ITEM_BAG_SCROLL_HOLDER,
    ITEM_BAG_WAND_HOLSTER,
    ITEM_BAG_KEYRING,
    /* ── Quest — not in the random loot pool ─────────────── */
    ITEM_AMULET_OF_YENDOR,

    ITEM_TYPE_COUNT
} ItemType;

/* Sentinel meaning "no item" — used for Player's equip slots
   (weapon/armor/ring1/ring2 in player.h) so an empty slot has a
   value distinct from every real ItemType, including ITEM_DAGGER==0. */
#define ITEM_NONE ((ItemType)-1)

/* NOTE on "unidentified" items: the real game shows potions/scrolls/
   wands/rings under a random per-seed alias (color/rune/wood/gem)
   until the hero identifies them, and Item.java carries curse/
   enchantment state per-instance. None of that exists yet — every
   item here is a fixed, always-known (type, quantity) pair. That's
   deliberately out of scope for "generate + pick up"; the ItemDef
   table below is the natural place to hang a `Look look;` field and
   a shuffle table later. */
typedef struct {
    const char  *name;
    ItemCategory cat;
    uint8_t      sprite_col, sprite_row;  /* cell in items.png (16px grid) */
    uint8_t      stackable;
} ItemDef;

extern const ItemDef ITEM_DB[ITEM_TYPE_COUNT];

/* Weighted random item type, independent of any Level — see item.c
   for the category weights (adapted from Generator.java). */
ItemType random_item_of_category(ItemCategory cat);
ItemType random_item(void);

static inline const char *item_name(ItemType t) {
    return (t >= 0 && t < ITEM_TYPE_COUNT) ? ITEM_DB[t].name : "unknown item";
}

/* An item stack with no position — used for the player's backpack,
   and as the out-parameter when something is picked up. */
typedef struct {
   ItemType type;
   int      quantity;

   int8_t  level;         /* enchant tier; can go negative (degraded)     */
   uint8_t level_known;   /* has the player seen this instance's level?   */
   uint8_t cursed;        /* is this specific instance cursed?            */
   uint8_t cursed_known;  /* found out yet? equipping always reveals it   */
   int16_t durability;    /* current durability, out of item_max_durability() */
} ItemStack;

/* An item stack sitting on the ground at a tile — used for a level's
   item heap. */
typedef enum {
    HEAP_NORMAL,
    HEAP_FOR_SALE,
    HEAP_CHEST,
    HEAP_LOCKED_CHEST,
    HEAP_CRYSTAL_CHEST,
    HEAP_TOMB,
    HEAP_SKELETON,
    HEAP_MIMIC,
    HEAP_HIDDEN
} HeapType;

#define MAX_HEAP_ITEMS 16   
typedef struct {
    HeapType  type;
    ItemStack items[MAX_HEAP_ITEMS];

    int       count;
    int       x, y;
    uint8_t   active;
} Heap;

#define MAX_LEVEL_ITEMS 24
#define MAX_ITEMS_TO_SPAWN 8

#define BACKPACK_SLOTS   19
#define GOLD_SLOT        BACKPACK_SLOTS
#define MAX_INVENTORY    (BACKPACK_SLOTS + 1)

/* Mirrors Heap.image() from the original: HEAP_NORMAL/HEAP_FOR_SALE
   show whatever's on top of the stack, but the container/decoration
   types (chest variants, tomb, skeleton, hidden, mimic) always show
   their own fixed icon no matter what's inside. Writes the cell into
   out_col and out_row and returns 1, or returns 0 for an empty heap
   with nothing to show. */
int heap_sprite_cell(const Heap *h, uint8_t *out_col, uint8_t *out_row);


/* Add `stack` to `heap` as-is (merging into a matching stackable
   entry, or taking a new slot), preserving whatever level/cursed/
   durability/known state it already carries. Use this — not
   heap_add_item below — whenever an EXISTING instance moves onto the
   ground (drop, forced unequip), so it doesn't get re-rolled fresh.
   Returns 1 on success, 0 if the heap's MAX_HEAP_ITEMS slots are full. */
int heap_add_stack(Heap *heap, ItemStack stack);
 
/* ══════════════════════════════════════════════════════════
   INSTANCE STATE — level / curse / durability
   ══════════════════════════════════════════════════════════
   A from-scratch port of Item.java's level/cursed/durability/
   levelKnown/cursedKnown machinery onto ItemStack. Durability only
   ever ticks down for upgraded (level>0) items, exactly like the
   original's use() — a fresh, un-enchanted item never breaks. */
 
#define ITEM_DURABILITY_WARN_NUM 1
#define ITEM_DURABILITY_WARN_DEN 6   /* matches Item.java's DURABILITY_WARNING_LEVEL = 1/6f */
 
typedef enum {
    ITEM_USE_OK = 0,          /* nothing notable happened                    */
    ITEM_USE_WARN_BREAKING,   /* durability just crossed the low threshold   */
    ITEM_USE_BROKE            /* durability just hit 0 this call             */
} ItemUseResult;
 
/* isBroken()/effectiveLevel()/visiblyUpgraded()/visiblyCursed()/
   visiblyBroken()/isIdentified() — direct ports, kept inline since
   they're pure field reads with no ITEM_DB/RNG dependency. */
static inline int item_is_broken(const ItemStack *s) {
    return s->durability <= 0;
}
static inline int item_effective_level(const ItemStack *s) {
    return item_is_broken(s) ? 0 : s->level;
}
static inline int item_visibly_upgraded(const ItemStack *s) {
    return s->level_known ? s->level : 0;
}
static inline int item_visibly_cursed(const ItemStack *s) {
    return s->cursed && s->cursed_known;
}
static inline int item_visibly_broken(const ItemStack *s) {
    return s->level_known && item_is_broken(s);
}
static inline int item_is_identified(const ItemStack *s) {
    return s->level_known && s->cursed_known;
}
 
/* Empty-slot sentinel for Player's equip slots (see player.h) — an
   all-zero ItemStack with type forced to ITEM_NONE. */
static inline ItemStack item_stack_none(void) {
    ItemStack s;
    memset(&s, 0, sizeof(s));
    s.type = ITEM_NONE;
    return s;
}
 
/* Placeholder per-category curse chance out of 100. The real odds
   live in Weapon.java/Armor.java/Wand.java/Ring.java, which weren't
   part of what you shared — this is a reasonable stand-in with the
   same shape (only equipables that stay on your body/in your hand
   roll cursed). Tune freely. */
int item_curse_chance_pct(ItemCategory cat);
 
/* Placeholder max-durability formula — same caveat as above; the
   real per-category numbers live in files I don't have. Clamped to
   at least 1 so a heavily degraded item is merely fragile, not
   instantly indestructible-zero. Shape mirrors Item.java's
   maxDurability(level) being overridden per subclass. */
int item_max_durability(ItemType type, int level);
 
/* Build a freshly-spawned instance of `type`: level 0, durability at
   max, curse rolled per item_curse_chance_pct() (skipped for
   stackable types — see the ItemStack comment above), and
   level_known set immediately for WEAPON/ARMOR (their enchant level
   is visible on pickup in the original — only whether they're cursed
   stays hidden) but left unknown for WAND/RING (hidden until
   identified, same as their shuffled name/sprite below). That
   WEAPON/ARMOR-vs-WAND/RING split is inferred from vanilla behaviour
   since those subclasses weren't part of what you shared — flip it
   if yours differs. Call this wherever loot is created instead of a
   bare {type, quantity} literal so the new fields start consistent
   rather than uninitialized. heap_add_item below already does. */
void item_stack_init(ItemStack *stack, ItemType type, int quantity);
 
/* fix()/polish() — reset/nudge durability back up. */
void item_fix(ItemStack *stack);
void item_polish(ItemStack *stack);
 
/* upgrade()/degrade() — adjust level and fix(). Matches the original
   exactly: upgrading also clears+reveals curse (a +1 scroll cleanses
   as it enchants); degrading does not touch curse state at all. */
void item_upgrade(ItemStack *stack);
void item_upgrade_n(ItemStack *stack, int n);
void item_degrade(ItemStack *stack);
void item_degrade_n(ItemStack *stack, int n);
 
/* use() — wear an upgraded item down by one hit's worth of
   durability (a no-op for level<=0 or already-broken items, exactly
   like the original's guard). Returns what just happened so the
   caller can react (message/sound/etc.) — this port doesn't have a
   message-log system yet, so that reaction lives wherever you end up
   calling this from, not here. */
ItemUseResult item_use(ItemStack *stack);
 
/* identify() — reveal this instance's level+curse, and (for
   POTION/SCROLL/WAND/RING) mark the whole TYPE known for the rest of
   the run. */
void item_identify(ItemStack *stack);
 
/* ══════════════════════════════════════════════════════════
   IDENTIFY — per-TYPE "known" tracking + shuffled unidentified look
   ══════════════════════════════════════════════════════════
   A from-scratch port of ItemStatusHandler.java's random label/image
   shuffle + known set, adapted to this port's flat ItemType enum (the
   original keyed off Class<? extends Item> per subclass; here it's
   just ItemType). Applies to POTION/SCROLL/WAND/RING — the categories
   that hide their true name behind a per-run color/rune/wood/gem
   alias until identified. WAND_MAGIC_MISSILE is excluded from the
   wand shuffle, same as the original (see the comment above
   SPRITE_WAND_MAGIC_MISSILE) — it's always shown as itself. Every
   other category (weapon/armor/food/misc/gold/key/bag/quest) was
   never hidden by name, so item_status_is_known() just reports true
   for them — only their level/cursed (above) can be unknown. */
 
/* Reset all identify state for a new run: re-shuffle which alias
   look each shuffled type gets, and forget every type. Call once
   when a new game starts (mirrors building a fresh ItemStatusHandler
   + Generator.reset() in the original). If you never call this
   explicitly, the first identify-related call auto-initializes once,
   so it's safe to skip for a single-run build — just re-call it
   yourself for run 2 onward, or the second run reuses run 1's looks. */
void item_status_reset(void);
 
/* Mark `type` identified. If every other member of its shuffled
   category is already known, the last remaining one is auto-known
   too — the same deduction ItemStatusHandler.know() makes. No-op for
   categories that were never shuffled. */
void item_status_know(ItemType type);
 
/* Is `type` identified this run? Always true for categories that
   were never shuffled. */
int item_status_is_known(ItemType type);
 
/* Cell in items.png to actually draw for `type` — the real sprite
   once known, otherwise this run's shuffled alias (a color/rune/
   wood/gem look shared by the whole unidentified category). */
void item_display_sprite(ItemType type, uint8_t *out_col, uint8_t *out_row);
 
/* Display name for `stack`: the shuffled alias ("turquoise potion",
   "kaunan scroll", "holly wand", "diamond ring") while its type is
   unidentified, otherwise the real name with a +N / "(cursed)" suffix
   once level_known/cursed_known say so (mirrors Item.java's
   toString(), minus the quantity — callers still handle "x%d"
   themselves, same as before). Writes into caller-owned `buf`
   (`bufsz` bytes) and returns it, snprintf-return-style. */
const char *item_display_name(const ItemStack *stack, char *buf, size_t bufsz);