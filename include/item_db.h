#pragma once 
#include <string.h>
#include <stdint.h>

/* ══════════════════════════════════════════════════════════
   ITEM SPRITE SHEET port of ItemSpriteSheet.java
   ══════════════════════════════════════════════════════════*/
#define ITEMS_SHEET_COLS 8
 
/* Placeholders */
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
#define SPRITE_SCROLL_WIPE_OUT 117   /* from wiki: 
The Sacrificial chamber is the only place where this scroll can be obtained.
It is obtained by slaying 5-6 enemies with the debuff, 'Marked for Sacrifice'. */
 
/* Seeds — fixed, real sprites (not a shuffled "look") */
#define SPRITE_SEED_FIREBLOOM   88
#define SPRITE_SEED_ICECAP      89
#define SPRITE_SEED_SORROWMOSS  90
#define SPRITE_SEED_DREAMWEED   91
#define SPRITE_SEED_SUNGRASS    92
#define SPRITE_SEED_EARTHROOT   93
#define SPRITE_SEED_FADELEAF    94
#define SPRITE_SEED_ROTBERRY    95
 
/* Quest items*/
#define SPRITE_ROSE            100
#define SPRITE_PICKAXE         101
#define SPRITE_ORE             102
#define SPRITE_SKULL           103
#define SPRITE_PHANTOM         118
#define SPRITE_DUST            121
#define SPRITE_TOKEN           122
#define SPRITE_AMULET           87
 
/* Heaps — decorative/container sprites, not carryable Items */
#define SPRITE_BONES              0
#define SPRITE_CHEST              11
#define SPRITE_LOCKED_CHEST       12
#define SPRITE_TOMB                13
#define SPRITE_CRYSTAL_CHEST     105
#define SPRITE_HIDDEN            119
 
/* Food */
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
#define SPRITE_STYLUS              80 /*unused*/
#define SPRITE_DEWDROP             81
#define SPRITE_MASTERY             82
#define SPRITE_TORCH               84
#define SPRITE_BEACON              85
#define SPRITE_KIT                 86
#define SPRITE_VIAL                120
#define SPRITE_WEIGHT              123
#define SPRITE_BOMB                124
#define SPRITE_HONEYPOT            125

/*GOLD*/
#define SPRITE_GOLD                14
/* ══════════════════════════════════════════════════════════
   ITEM SYSTEM
   ══════════════════════════════════════════════════════════ */

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
   ITEMCAT_KEY,     /* not in the random loot pool                  */
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
   ITEM_STEAK,
   ITEM_OVERPRICED,
   ITEM_FROZEN_CARPACCIO,
   /* ── Misc ────────────────────────────────────────────── */
   ITEM_ANKH,
   ITEM_DEWDROP,
   ITEM_MASTERY,
   ITEM_TORCH,
   ITEM_BEACON,
   ITEM_KIT,
   ITEM_VIAL,
   ITEM_WEIGHT,
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
   ITEM_ROSE,
   ITEM_PICKAXE,
   ITEM_ORE,
   ITEM_SKULL,
   ITEM_PHANTOM,
   ITEM_DUST,
   ITEM_TOKEN,
   ITEM_AMULET_OF_YENDOR,

   ITEM_TYPE_COUNT
} ItemType;

#define ITEM_NONE ((ItemType)-1)

typedef struct {
   const char  *name;
   const char  *description;
   ItemCategory cat;
   uint8_t      sprite_col, sprite_row;  /* cell in items.png (16px grid) */
   uint8_t      stackable;
} ItemDef;


extern const ItemDef ITEM_DB[ITEM_TYPE_COUNT];
extern const float SPAWN_CATEGORY_WEIGHT[ITEMCAT_COUNT];
extern const float SPAWN_ITEM_WEIGHT[ITEM_TYPE_COUNT];
