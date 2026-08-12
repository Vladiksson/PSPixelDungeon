#pragma once

/* assets.h — C mirror of Assets.java*/
/* ── UI / HUD ─────────────────────────────────────────── */
#define ASSET_ARCS_BG       "arcs1.png"
#define ASSET_ARCS_FG       "arcs2.png"
#define ASSET_DASHBOARD     "dashboard.png"
#define ASSET_BANNERS       "banners.png"
#define ASSET_BADGES        "badges.png"
#define ASSET_LOCKED        "locked_badge.png"
#define ASSET_AMULET        "amulet.png"
#define ASSET_CHROME        "chrome.png"
#define ASSET_ICONS         "icons.png"
#define ASSET_STATUS        "status_pane.png"
#define ASSET_HP_BAR        "hp_bar.png"
#define ASSET_XP_BAR        "exp_bar.png"
#define ASSET_TOOLBAR       "toolbar.png"
#define ASSET_SHADOW        "shadow.png"
#define ASSET_BUFFS_SMALL   "buffs.png"
#define ASSET_BUFFS_LARGE   "large_buffs.png"
#define ASSET_SPELL_ICONS   "spell_icons.png"

/* ── Hero sprites ─────────────────────────────────────── */
#define ASSET_WARRIOR   "warrior.png"
#define ASSET_MAGE      "mage.png"
#define ASSET_ROGUE     "rogue.png"
#define ASSET_HUNTRESS  "ranger.png"

#define ASSET_HERO_BY_CLASS { ASSET_WARRIOR, ASSET_MAGE, ASSET_ROGUE, ASSET_HUNTRESS }

#define ASSET_AVATARS   "avatars.png"
#define ASSET_PET       "pet.png"
#define ASSET_SURFACE   "surface.png"

/* ── Effects ──────────────────────────────────────────── */
#define ASSET_FIREBALL  "fireball.png"
#define ASSET_SPECKS    "specks.png"
#define ASSET_EFFECTS   "effects.png"

/* ── Mob sprites ──────────────────────────────────────── */
#define ASSET_RAT       "rat.png"
#define ASSET_GNOLL     "gnoll.png"
#define ASSET_CRAB      "crab.png"
#define ASSET_GOO       "goo.png"
#define ASSET_SWARM     "swarm.png"
#define ASSET_SKELETON  "skeleton.png"
#define ASSET_SHAMAN    "shaman.png"
#define ASSET_THIEF     "thief.png"
#define ASSET_TENGU     "tengu.png"
#define ASSET_SHEEP     "sheep.png"
#define ASSET_KEEPER    "shopkeeper.png"
#define ASSET_BAT       "bat.png"
#define ASSET_BRUTE     "brute.png"
#define ASSET_SPINNER   "spinner.png"
#define ASSET_DM300     "dm300.png"
#define ASSET_WRAITH    "wraith.png"
#define ASSET_ELEMENTAL "elemental.png"
#define ASSET_MONK      "monk.png"
#define ASSET_WARLOCK   "warlock.png"
#define ASSET_GOLEM     "golem.png"
#define ASSET_UNDEAD    "undead.png"
#define ASSET_KING      "king.png"
#define ASSET_STATUE    "statue.png"
#define ASSET_PIRANHA   "piranha.png"
#define ASSET_EYE       "eye.png"
#define ASSET_SUCCUBUS  "succubus.png"
#define ASSET_SCORPIO   "scorpio.png"
#define ASSET_ROTTING   "rotting_fist.png"
#define ASSET_BURNING   "burning_fist.png"
#define ASSET_YOG       "yog.png"
#define ASSET_LARVA     "larva.png"
#define ASSET_GHOST     "ghost.png"
#define ASSET_MAKER     "wandmaker.png"
#define ASSET_TROLL     "blacksmith.png"
#define ASSET_IMP       "demon.png"
#define ASSET_RATKING   "ratking.png"
#define ASSET_BEE       "bee.png"
#define ASSET_MIMIC     "mimic.png"

typedef enum {
    MOBTEX_RAT = 0, MOBTEX_GNOLL, MOBTEX_CRAB, MOBTEX_SWARM, MOBTEX_SKELETON,
    MOBTEX_SHAMAN, MOBTEX_THIEF, MOBTEX_TENGU, MOBTEX_BAT, MOBTEX_BRUTE,
    MOBTEX_SPINNER, MOBTEX_DM300, MOBTEX_WRAITH, MOBTEX_ELEMENTAL, MOBTEX_MONK,
    MOBTEX_WARLOCK, MOBTEX_GOLEM, MOBTEX_UNDEAD, MOBTEX_KING, MOBTEX_STATUE,
    MOBTEX_PIRANHA, MOBTEX_EYE, MOBTEX_SUCCUBUS, MOBTEX_SCORPIO, MOBTEX_ROTTING,
    MOBTEX_BURNING, MOBTEX_YOG, MOBTEX_LARVA, MOBTEX_GOO, MOBTEX_MIMIC,
    MOBTEX_COUNT
} MobTexId;

#define ASSET_MOB_TEX_BY_ID { \
    ASSET_RAT, ASSET_GNOLL, ASSET_CRAB, ASSET_SWARM, ASSET_SKELETON, \
    ASSET_SHAMAN, ASSET_THIEF, ASSET_TENGU, ASSET_BAT, ASSET_BRUTE, \
    ASSET_SPINNER, ASSET_DM300, ASSET_WRAITH, ASSET_ELEMENTAL, ASSET_MONK, \
    ASSET_WARLOCK, ASSET_GOLEM, ASSET_UNDEAD, ASSET_KING, ASSET_STATUE, \
    ASSET_PIRANHA, ASSET_EYE, ASSET_SUCCUBUS, ASSET_SCORPIO, ASSET_ROTTING, \
    ASSET_BURNING, ASSET_YOG, ASSET_LARVA, ASSET_GOO, ASSET_MIMIC }

/* ── Items ────────────────────────────────────────────── */
#define ASSET_ITEMS     "items.png"
#define ASSET_PLANTS    "plants.png"

/* ── Dungeon tile sheets (one per theme) ──────────────── */
#define ASSET_TILES_SEWERS  "tiles0.png"
#define ASSET_TILES_PRISON  "tiles1.png"
#define ASSET_TILES_CAVES   "tiles2.png"
#define ASSET_TILES_CITY    "tiles3.png"
#define ASSET_TILES_HALLS   "tiles4.png"

/* Indexed array for use in render_init loops */
#define ASSET_TILES_BY_THEME { \
    ASSET_TILES_SEWERS, ASSET_TILES_PRISON, ASSET_TILES_CAVES, \
    ASSET_TILES_CITY,   ASSET_TILES_HALLS }

/* ── Water surface sheets (one per theme) ─────────────── */
#define ASSET_WATER_SEWERS  "water0.png"
#define ASSET_WATER_PRISON  "water1.png"
#define ASSET_WATER_CAVES   "water2.png"
#define ASSET_WATER_CITY    "water3.png"
#define ASSET_WATER_HALLS   "water4.png"

#define ASSET_WATER_BY_THEME { \
    ASSET_WATER_SEWERS, ASSET_WATER_PRISON, ASSET_WATER_CAVES, \
    ASSET_WATER_CITY,   ASSET_WATER_HALLS }

/* ── Bitmap fonts ─────────────────────────────────────── */
/* PD uses its own bitmap font renderer.
   Each font sheet is a different scale (1x, 1.5x, 2x …).
   I use the smallest one, 
   but since sdl doesn't wanna work with original resolution 1024x8,
   I've split it into three parts(a,b,c).*/
#define ASSET_FONTS1X   "font1x.png"
#define ASSET_FONTS1XA  "font1x_a.png"
#define ASSET_FONTS1XB  "font1x_b.png"
#define ASSET_FONTS1XC  "font1x_c.png"
#define ASSET_FONTS15X  "font15x.png"
#define ASSET_FONTS2X   "font2x.png"
#define ASSET_FONTS25X  "font25x.png"
#define ASSET_FONTS3X   "font3x.png"

#define ASSET_FONTS1X_BY_PAGE { \
    ASSET_FONTS1XA, ASSET_FONTS1XB, ASSET_FONTS1XC }
/* ── Music ────────────────────────────────────────────── */
#define ASSET_THEME     "theme.mp3"
#define ASSET_TUNE      "game.mp3"
#define ASSET_HAPPY     "surface.mp3"

/* ── Sound effects ────────────────────────────────────── */
#define ASSET_SND_CLICK     "snd_click.mp3"
#define ASSET_SND_BADGE     "snd_badge.mp3"
#define ASSET_SND_GOLD      "snd_gold.mp3"
#define ASSET_SND_OPEN      "snd_door_open.mp3"
#define ASSET_SND_UNLOCK    "snd_unlock.mp3"
#define ASSET_SND_ITEM      "snd_item.mp3"
#define ASSET_SND_DEWDROP   "snd_dewdrop.mp3"
#define ASSET_SND_HIT       "snd_hit.mp3"
#define ASSET_SND_MISS      "snd_miss.mp3"
#define ASSET_SND_STEP      "snd_step.mp3"
#define ASSET_SND_WATER     "snd_water.mp3"
#define ASSET_SND_DESCEND   "snd_descend.mp3"
#define ASSET_SND_EAT       "snd_eat.mp3"
#define ASSET_SND_READ      "snd_read.mp3"
#define ASSET_SND_LULLABY   "snd_lullaby.mp3"
#define ASSET_SND_DRINK     "snd_drink.mp3"
#define ASSET_SND_SHATTER   "snd_shatter.mp3"
#define ASSET_SND_ZAP       "snd_zap.mp3"
#define ASSET_SND_LIGHTNING "snd_lightning.mp3"
#define ASSET_SND_LEVELUP   "snd_levelup.mp3"
#define ASSET_SND_DEATH     "snd_death.mp3"
#define ASSET_SND_CHALLENGE "snd_challenge.mp3"
#define ASSET_SND_CURSED    "snd_cursed.mp3"
#define ASSET_SND_TRAP      "snd_trap.mp3"
#define ASSET_SND_EVOKE     "snd_evoke.mp3"
#define ASSET_SND_TOMB      "snd_tomb.mp3"
#define ASSET_SND_ALERT     "snd_alert.mp3"
#define ASSET_SND_MELD      "snd_meld.mp3"
#define ASSET_SND_BOSS      "snd_boss.mp3"
#define ASSET_SND_BLAST     "snd_blast.mp3"
#define ASSET_SND_PLANT     "snd_plant.mp3"
#define ASSET_SND_RAY       "snd_ray.mp3"
#define ASSET_SND_BEACON    "snd_beacon.mp3"
#define ASSET_SND_TELEPORT  "snd_teleport.mp3"
#define ASSET_SND_CHARMS    "snd_charms.mp3"
#define ASSET_SND_MASTERY   "snd_mastery.mp3"
#define ASSET_SND_PUFF      "snd_puff.mp3"
#define ASSET_SND_ROCKS     "snd_rocks.mp3"
#define ASSET_SND_BURNING   "snd_burning.mp3"
#define ASSET_SND_FALLING   "snd_falling.mp3"
#define ASSET_SND_GHOST     "snd_ghost.mp3"
#define ASSET_SND_SECRET    "snd_secret.mp3"
#define ASSET_SND_BONES     "snd_bones.mp3"
#define ASSET_SND_BEE       "snd_bee.mp3"
#define ASSET_SND_DEGRADE   "snd_degrade.mp3"
#define ASSET_SND_MIMIC     "snd_mimic.mp3"