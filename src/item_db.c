#include "item_db.h"
/*═══════════════════════════════════════════════════════════════
    Item database
═══════════════════════════════════════════════════════════════*/
#define CELL(sprite) (sprite) % ITEMS_SHEET_COLS, (sprite) / ITEMS_SHEET_COLS
 
const ItemDef ITEM_DB[ITEM_TYPE_COUNT] = {
    /* ── weapons ──────────────────────────────────────────── */
    /* ── MELEE ──────────────────────────────────────────── */
    [ITEM_DAGGER]           = { "Dagger",        "A simple iron dagger with a well worn wooden handle.",
                                                 ITEMCAT_WEAPON, CELL(SPRITE_DAGGER),           0 },
    [ITEM_KNUCKLES]         = { "Knuckleduster", "A piece of iron shaped to fit around the knuckles.",
                                                 ITEMCAT_WEAPON, CELL(SPRITE_KNUCKLEDUSTER),    0 },
    [ITEM_QUARTERSTAFF]     = { "Quarterstaff",  "A staff of hardwood, its ends are shod with iron.",
                                                 ITEMCAT_WEAPON, CELL(SPRITE_QUARTERSTAFF),     0 },
    [ITEM_SPEAR]            = { "Spear",         "A slender wooden rod tipped with sharpened iron.",
                                                 ITEMCAT_WEAPON, CELL(SPRITE_SPEAR),            0 },
    [ITEM_MACE]             = { "Mace",          "The iron head of this weapon inflicts substantial damage.", 
                                                 ITEMCAT_WEAPON, CELL(SPRITE_MACE),             0 },
    [ITEM_SWORD]            = { "Sword",         "The razor-sharp length of steel blade shines reassuringly.",
                                                 ITEMCAT_WEAPON, CELL(SPRITE_SWORD),            0 },
    [ITEM_LONGSWORD]        = { "Longsword",     "This towering blade inflicts heavy damage by investing its heft into every cut.",
                                                 ITEMCAT_WEAPON, CELL(SPRITE_LONG_SWORD),       0 },
    [ITEM_BATTLE_AXE]       = { "Battle Axe",    "The enormous steel head of this battle axe puts considerable heft behind each stroke.",
                                                 ITEMCAT_WEAPON, CELL(SPRITE_BATTLE_AXE),       0 },
    [ITEM_WAR_HAMMER]       = { "War Hammer",    "Few creatures can withstand the crushing blow of this towering mass of lead and steel, "
                                                 "but only the strongest of adventurers can use it effectively.",
                                                 ITEMCAT_WEAPON, CELL(SPRITE_WAR_HAMMER),       0 },
    [ITEM_GLAIVE]           = { "Glaive",        "A polearm consisting of a sword blade on the end of a pole.", 
                                                 ITEMCAT_WEAPON, CELL(SPRITE_GLAIVE),           0 },
    [ITEM_SHORT_SWORD]      = { "Short Sword",   "It is indeed quite short, just a few inches longer, than a dagger.",
                                                 ITEMCAT_WEAPON, CELL(SPRITE_SHORT_SWORD),      0 },
    /* ── MISSILES ──────────────────────────────────────────── */
    [ITEM_DART]             = { "Dart",            "These simple metal spikes are weighted to fly true and "
			                                       "sting their prey with a flick of the wrist.",   
                                                   ITEMCAT_WEAPON, CELL(SPRITE_DART), 1 },
    [ITEM_JAVELIN]          = { "Javelin",         "This length of metal is weighted to keep the spike " 
			                                       "at its tip foremost as it sails through the air.",  
                                                   ITEMCAT_WEAPON, CELL(SPRITE_JAVELIN), 1 },
    [ITEM_INCENDIARY_DART]  = { "Incendiary Dart", "The spike on each of these darts is designed to pin it to its target " 
			                                       "while the unstable compounds strapped to its length burst into brilliant flames.", 
                                                   ITEMCAT_WEAPON, CELL(SPRITE_INCENDIARY_DART),  1 },
    [ITEM_CURARE_DART]      = { "Curare Dart",     "These little evil darts don't do much damage but they can paralyze " 
			                                       "the target leaving it helpless and motionless for some time.", 
                                                   ITEMCAT_WEAPON, CELL(SPRITE_CURARE_DART), 1 },
    [ITEM_SHURIKEN]         = { "Shuriken",        "Star-shaped pieces of metal with razor-sharp blades do significant damage " 
			                                       "when they hit a target. They can be thrown at very high rate.", 
                                                   ITEMCAT_WEAPON, CELL(SPRITE_SHURIKEN), 1 },
    [ITEM_BOOMERANG]        = { "Boomerang",       "Thrown to the enemy this flat curved wooden missile will return to the hands of its thrower.", 
                                                   ITEMCAT_WEAPON, CELL(SPRITE_BOOMERANG), 1 },
    [ITEM_TAMAHAWK]         = { "Tamahawk",        "This throwing axe is not that heavy, but it still " 
			                                       "requires significant strength to be used effectively.", 
                                                   ITEMCAT_WEAPON, CELL(SPRITE_TOMAHAWK), 1 },
    /* ── armor ────────────────────────────────────────────── */
    [ITEM_CLOTH_ARMOR]      = { "Cloth Armor",   "This lightweight armor offers basic protection.",
                                                 ITEMCAT_ARMOR, CELL(SPRITE_ARMOR_CLOTH),   0 },
    [ITEM_LEATHER_ARMOR]    = { "Leather Armor", "Armor made from tanned monster hide. Not as light as cloth armor but provides better protection.",
                                                 ITEMCAT_ARMOR, CELL(SPRITE_ARMOR_LEATHER), 0 },
    [ITEM_MAIL_ARMOR]       = { "Mail Armor",    "Interlocking metal links make for a tough but flexible suit of armor.",
                                                 ITEMCAT_ARMOR, CELL(SPRITE_ARMOR_MAIL),    0 },
    [ITEM_SCALE_ARMOR]      = { "Scale Armor",   "The metal scales sewn onto a leather vest create a flexible, yet protective armor.",
                                                 ITEMCAT_ARMOR, CELL(SPRITE_ARMOR_SCALE),   0 },
    [ITEM_PLATE_ARMOR]      = { "Plate Armor",   "Enormous plates of metal are joined together into a suit that provides " 
		                                         "unmatched protection to any adventurer strong enough to bear its staggering weight.",
                                                 ITEMCAT_ARMOR, CELL(SPRITE_ARMOR_PLATE),   0 },
    /* ── wands — 12 shuffled onto the 12 wood looks, Magic Missile fixed ── */
    [ITEM_WAND_TELEPORTATION] = { "Wand of Teleportation", "A blast from this wand will teleport a creature against " 
			                                               "its will to a random place on the current level.",
                                                           ITEMCAT_WAND, CELL(SPRITE_WAND_HOLLY),        0 },
    [ITEM_WAND_SLOWNESS]      = { "Wand of Slowness",      "This wand will cause a creature to move and attack " 
                                                           "at half its ordinary speed until the effect ends",
                                                           ITEMCAT_WAND, CELL(SPRITE_WAND_YEW),          0 },
    [ITEM_WAND_FIREBOLT]      = { "Wand of Firebolt",      "This wand unleashes bursts of magical fire. It will ignite " 
                                                           "flammable terrain, and will damage and burn a creature it hits.",
                                                           ITEMCAT_WAND, CELL(SPRITE_WAND_EBONY),        0 },
    [ITEM_WAND_REGROWTH]      = { "Wand of Regrowth",      "\"When life ceases new life always begins to grow... The eternal cycle always remains!\"",
                                                           ITEMCAT_WAND, CELL(SPRITE_WAND_CHERRY),       0 },
    [ITEM_WAND_POISON]        = { "Wand of Poison",        "The vile blast of this twisted bit of wood will imbue its target " 
                                                           "with a deadly venom. A creature that is poisoned will suffer periodic " 
                                                           "damage until the effect ends. The duration of the effect increases " 
                                                           "with the level of the staff.",
                                                           ITEMCAT_WAND, CELL(SPRITE_WAND_TEAK),         0 },
    [ITEM_WAND_BLINK]         = { "Wand of Blink",         "This wand will allow you to teleport in the chosen direction. " 
                                                           "Creatures and inanimate obstructions will block the teleportation.",
                                                           ITEMCAT_WAND, CELL(SPRITE_WAND_ROWAN),        0 },
    [ITEM_WAND_LIGHTNING]     = { "Wand of Lightning",     "This wand conjures forth deadly arcs of electricity, which deal damage " 
                                                           "to several creatures standing close to each other.",
                                                           ITEMCAT_WAND, CELL(SPRITE_WAND_WILLOW),       0 },
    [ITEM_WAND_AMOK]          = { "Wand of Amok",          "The purple light from this wand will make the target run amok "
                                                           "attacking random creatures in its vicinity.",
                                                           ITEMCAT_WAND, CELL(SPRITE_WAND_MAHOGANY),     0 },
    [ITEM_WAND_REACH]         = { "Wand of Reach",         "This utility wand can be used to grab objects from a distance and to switch places with enemies. " 
                                                           "Waves of magic force radiated from it will affect all cells on their way triggering traps, " 
                                                           "trampling high vegetation, opening closed doors and closing open ones.",
                                                           ITEMCAT_WAND, CELL(SPRITE_WAND_BAMBOO),       0 },
    [ITEM_WAND_FLOCK]         = { "Wand of Flock",         "A flick of this wand summons a flock of magic sheep, creating temporary impenetrable obstacle.",
                                                           ITEMCAT_WAND, CELL(SPRITE_WAND_PURPLEHEART),  0 },
    [ITEM_WAND_MAGIC_MISSILE] = { "Wand of Magic Missile", "This wand launches missiles of pure magical energy, dealing moderate damage to a target creature.",
                                                           ITEMCAT_WAND, CELL(SPRITE_WAND_MAGIC_MISSILE),0 },
    [ITEM_WAND_DISINTEGRATION]= { "Wand of Disintegration","This wand emits a beam of destructive energy, which pierces all creatures in its way. " 
                                                           "The more targets it hits, the more damage it inflicts to each of them.",
                                                           ITEMCAT_WAND, CELL(SPRITE_WAND_OAK),          0 },
    [ITEM_WAND_AVALANCHE]     = { "Wand of Avalanche",     "When a discharge of this wand hits a wall (or any other solid obstacle) it causes " 
                                                           "an avalanche of stones, damaging and stunning all creatures in the affected area.",
                                                           ITEMCAT_WAND, CELL(SPRITE_WAND_BIRCH),        0 },
    /* ── potions — 12 shuffled onto the 12 color looks ───────────────── */
    [ITEM_POTION_HEALING]      = { "Potion of Healing",      "An elixir that will instantly return you to full health and cure poison.",
                                                             ITEMCAT_POTION, CELL(SPRITE_POTION_TURQUOISE), 1 },
    [ITEM_POTION_EXPERIENCE]   = { "Potion of Experience",   "The storied experiences of multitudes of battles reduced to liquid form, " 
                                                             "this draught will instantly raise your experience level.",
                                                             ITEMCAT_POTION, CELL(SPRITE_POTION_CRIMSON),   1 },
    [ITEM_POTION_TOXIC_GAS]    = { "Potion of Toxic Gas",    "Uncorking or shattering this pressurized glass will cause " 
                                                             "its contents to explode into a deadly cloud of toxic green gas. " 
                                                             "You might choose to fling this potion at distant enemies " 
                                                             "instead of uncorking it by hand.",
                                                             ITEMCAT_POTION, CELL(SPRITE_POTION_AZURE),     1 },
    [ITEM_POTION_PARALYTIC_GAS]= { "Potion of Paralytic Gas","Upon exposure to open air, the liquid in this flask will vaporize " 
                                                             "into a numbing yellow haze. Anyone who inhales the cloud will be paralyzed " 
                                                             "instantly, unable to move for some time after the cloud dissipates. This " 
                                                             "item can be thrown at distant enemies to catch them within the effect of the gas.",
                                                             ITEMCAT_POTION, CELL(SPRITE_POTION_JADE),      1 },
    [ITEM_POTION_LIQUID_FLAME] = { "Potion of Liquid Flame", "This flask contains an unstable compound which will burst " 
                                                             "violently into flame upon exposure to open air.",
                                                             ITEMCAT_POTION, CELL(SPRITE_POTION_GOLDEN),    1 },
    [ITEM_POTION_LEVITATION]   = { "Potion of Levitation",   "Drinking this curious liquid will cause you to hover in the air, " 
                                                             "able to drift effortlessly over traps. Flames and gases " 
                                                             "fill the air, however, and cannot be bypassed while airborne.",
                                                             ITEMCAT_POTION, CELL(SPRITE_POTION_MAGENTA),   1 },
    [ITEM_POTION_STRENGTH]     = { "Potion of Strength",     "This powerful liquid will course through your muscles, " 
                                                             "permanently increasing your strength by one point.",
                                                             ITEMCAT_POTION, CELL(SPRITE_POTION_CHARCOAL),  1 },
    [ITEM_POTION_MIND_VISION]  = { "Potion of Mind Vision",  "After drinking this, your mind will become attuned to the psychic signature " 
                                                             "of distant creatures, enabling you to sense biological presences through walls. " 
                                                             "Also this potion will permit you to see through nearby walls and doors.",
                                                             ITEMCAT_POTION, CELL(SPRITE_POTION_IVORY),     1 },
    [ITEM_POTION_PURITY]       = { "Potion of Purity",       "This reagent will quickly neutralize all harmful gases in the area of effect. " 
                                                             "Drinking it will give you a temporary immunity to such gases.",
                                                             ITEMCAT_POTION, CELL(SPRITE_POTION_AMBER),     1 },
    [ITEM_POTION_INVISIBILITY] = { "Potion of Invisibility", "Drinking this potion will render you temporarily invisible. While invisible, " 
                                                             "enemies will be unable to see you. Attacking an enemy, as well as using a wand or a scroll " 
                                                             "before enemy's eyes, will dispel the effect.",
                                                             ITEMCAT_POTION, CELL(SPRITE_POTION_BISTRE),    1 },
    [ITEM_POTION_MIGHT]        = { "Potion of Might",        "This powerful liquid will course through your muscles, permanently " 
                                                             "increasing your strength by one point and health by five points.",
                                                             ITEMCAT_POTION, CELL(SPRITE_POTION_INDIGO),    1 },
    [ITEM_POTION_FROST]        = { "Potion of Frost",        "Upon exposure to open air, this chemical will evaporate into a freezing cloud, causing " 
                                                             "any creature that contacts it to be frozen in place, unable to act and move.",
                                                             ITEMCAT_POTION, CELL(SPRITE_POTION_SILVER),    1 },
    /* ── scrolls — 12 shuffled onto the 12 rune looks ────────────────── */
    [ITEM_SCROLL_IDENTIFY]      = { "Scroll of Identify",     "Permanently reveals all of the secrets of a single item.",
                                                              ITEMCAT_SCROLL, CELL(SPRITE_SCROLL_KAUNAN),   1 },
    [ITEM_SCROLL_TELEPORTATION] = { "Scroll of Teleportation","The spell on this parchment instantly transports the reader " 
                                                              "to a random location on the dungeon level. It can be used " 
                                                              "to escape a dangerous situation, but the unlucky reader might " 
                                                              "find himself in an even more dangerous place.",
                                                              ITEMCAT_SCROLL, CELL(SPRITE_SCROLL_SOWILO),   1 },
    [ITEM_SCROLL_REMOVE_CURSE]  = { "Scroll of Remove Curse", "The incantation on this scroll will instantly strip from " 
                                                              "the reader's weapon, armor, rings and carried items any evil " 
                                                              "enchantments that might prevent the wearer from removing them.",
                                                              ITEMCAT_SCROLL, CELL(SPRITE_SCROLL_LAGUZ),    1 },
    [ITEM_SCROLL_RECHARGING]    = { "Scroll of Recharging",   "The raw magical power bound up in this parchment will, when released, " 
                                                              "recharge all of the reader's wands to full power.",
                                                              ITEMCAT_SCROLL, CELL(SPRITE_SCROLL_YNGVI),    1 },
    [ITEM_SCROLL_MAGIC_MAPPING] = { "Scroll of Magic Mapping","When this scroll is read, an image of crystal clarity will be etched into your memory, " 
                                                              "alerting you to the precise layout of the level and revealing all hidden secrets. " 
                                                              "The locations of items and creatures will remain unknown.",
                                                              ITEMCAT_SCROLL, CELL(SPRITE_SCROLL_GYFU),     1 },
    [ITEM_SCROLL_CHALLENGE]     = { "Scroll of Challenge",    "When read aloud, this scroll will unleash a challenging roar " 
                                                              "that will awaken all monsters and alert them to the reader's location.",
                                                              ITEMCAT_SCROLL, CELL(SPRITE_SCROLL_RAIDO),    1 },
    [ITEM_SCROLL_TERROR]        = { "Scroll of Terror",       "A flash of red light will overwhelm all creatures in your field of view with terror, " 
                                                              "and they will turn and flee. Attacking a fleeing enemy will dispel the effect.",
                                                              ITEMCAT_SCROLL, CELL(SPRITE_SCROLL_ISAZ),     1 },
    [ITEM_SCROLL_LULLABY]       = { "Scroll of Lullaby",      "A soothing melody will put all creatures in your field of view into a deep sleep, " 
                                                              "giving you a chance to flee or make a surprise attack on them.",
                                                              ITEMCAT_SCROLL, CELL(SPRITE_SCROLL_MANNAZ),   1 },
    [ITEM_SCROLL_PSIONIC_BLAST] = { "Scroll of Psionic Blast","This scroll contains destructive energy, that can be psionically channeled to inflict a " 
                                                              "massive damage to all creatures within a field of view. An accompanying flash of light will " 
                                                              "temporarily blind everybody in the area of effect including the reader of the scroll.",
                                                              ITEMCAT_SCROLL, CELL(SPRITE_SCROLL_NAUDIZ),   1 },
    [ITEM_SCROLL_MIRROR_IMAGE]  = { "Scroll of Mirror Image", "The incantation on this scroll will create illusionary twins of the reader, which will chase his enemies.",
                                                              ITEMCAT_SCROLL, CELL(SPRITE_SCROLL_BERKANAN), 1 },
    [ITEM_SCROLL_UPGRADE]       = { "Scroll of Upgrade",      "This scroll will upgrade a single item, improving its quality. A wand will " 
                                                              "increase in power and in number of charges; a weapon will inflict more damage " 
                                                              "or find its mark more frequently; a suit of armor will deflect additional blows; " 
                                                              "the effect of a ring on its wearer will intensify. Weapons and armor will also " 
                                                              "require less strength to use, and any curses on the item will be lifted.",
                                                              ITEMCAT_SCROLL, CELL(SPRITE_SCROLL_ODAL),     1 },
    [ITEM_SCROLL_ENCHANTMENT]   = { "Scroll of Enchantment",  "This scroll is able to imbue a weapon or an armor " 
                                                              "with a random enchantment, granting it a special power.",
                                                              ITEMCAT_SCROLL, CELL(SPRITE_SCROLL_TIWAZ),    1 },
    /* ── rings — 12 shuffled onto the 12 gem looks ───────────────────── */
    [ITEM_RING_MENDING]    = { "Ring of Mending",   "This ring increases the body's regenerative properties, allowing " 
                                                    "one to recover lost health at an accelerated rate. Degraded rings will " 
                                                    "decrease or even halt one's natural regeneration.",
                                                    ITEMCAT_RING, CELL(SPRITE_RING_DIAMOND),    0 },
    [ITEM_RING_DETECTION]  = { "Ring of Detection", "Wearing this ring will allow the wearer to notice hidden secrets - " 
                                                    "traps and secret doors - without taking time to search. Degraded rings of detection " 
                                                    "will dull your senses, making it harder to notice secrets even when actively searching for them.",
                                                    ITEMCAT_RING, CELL(SPRITE_RING_OPAL),       0 },
    [ITEM_RING_SHADOWS]    = { "Ring of Shadows",   "Enemies will be less likely to notice you if you wear this ring. Degraded rings " 
                                                    "of shadows will alert enemies who might otherwise not have noticed your presence.",
                                                    ITEMCAT_RING, CELL(SPRITE_RING_GARNET),     0 },
    [ITEM_RING_POWER]      = { "Ring of Power",     "Your wands will become more powerful in the energy field " 
                                                    "that radiates from this ring. Degraded rings of power will instead weaken your wands.",
                                                    ITEMCAT_RING, CELL(SPRITE_RING_RUBY),       0 },
    [ITEM_RING_HERBALISM]  = { "Ring of Herbalism", "This ring increases your chance to gather dew and seeds from trampled grass.",
                                                    ITEMCAT_RING, CELL(SPRITE_RING_AMETHYST),   0 },
    [ITEM_RING_ACCURACY]   = { "Ring of Accuracy",  "This ring increases your chance to hit the enemy.",
                                                    ITEMCAT_RING, CELL(SPRITE_RING_TOPAZ),      0 },
    [ITEM_RING_EVASION]    = { "Ring of Evasion",   "This ring increases your chance to dodge enemy attack.",
                                                    ITEMCAT_RING, CELL(SPRITE_RING_ONYX),       0 },
    [ITEM_RING_SATIETY]    = { "Ring of Satiety",   "Wearing this ring you can go without food longer. Degraded rings of satiety will cause the opposite effect.",
                                                    ITEMCAT_RING, CELL(SPRITE_RING_TOURMALINE), 0 },
    [ITEM_RING_HASTE]      = { "Ring of Haste",     "This ring accelerates the wearer's flow of time, allowing one to perform all actions a little faster.",
                                                    ITEMCAT_RING, CELL(SPRITE_RING_EMERALD),    0 },
    [ITEM_RING_ELEMENTS]   = { "Ring of Elements",  "This ring provides resistance to different elements, such as fire, "
                                                    "electricity, gases etc. Also it decreases duration of negative effects.",
                                                    ITEMCAT_RING, CELL(SPRITE_RING_SAPPHIRE),   0 },
    [ITEM_RING_HAGGLER]    = { "Ring of Haggler",   "In fact this ring doesn't provide any magic effect, but it demonstrates " 
                                                    "to shopkeepers and vendors, that the owner of the ring is a member of " 
                                                    "The Thieves' Guild. Usually they are glad to give a discount in exchange " 
                                                    "for temporary immunity guarantee. Upgrading this ring won't give any additional " 
                                                    "bonuses.",
                                                    ITEMCAT_RING, CELL(SPRITE_RING_QUARTZ),     0 },
    [ITEM_RING_THORNS]     = { "Ring of Thorns",    "Though this ring doesn't provide real thorns, an enemy that attacks you " 
                                                    "will itself be wounded by a fraction of the damage that it inflicts. " 
                                                    "Upgrading this ring won't give any additional bonuses.",
                                                    ITEMCAT_RING, CELL(SPRITE_RING_AGATE),      0 },
    /* ── seeds — fixed sprites ────────────────────────────── */
    [ITEM_SEED_FIREBLOOM]  = { "Firebloom Seed", "When something touches a Firebloom, it bursts into flames.",
                                                 ITEMCAT_SEED, CELL(SPRITE_SEED_FIREBLOOM),  1 },
    [ITEM_SEED_ICECAP]     = { "Icecap Seed",    "Upon touching an Icecap excretes a pollen, which freezes everything in its vicinity.",
                                                 ITEMCAT_SEED, CELL(SPRITE_SEED_ICECAP),     1 },
    [ITEM_SEED_SORROWMOSS] = { "Sorrowmoss Seed","A Sorrowmoss is a flower (not a moss) with razor-sharp petals, coated with a deadly venom.",
                                                 ITEMCAT_SEED, CELL(SPRITE_SEED_SORROWMOSS), 1 },
    [ITEM_SEED_DREAMWEED]  = { "Dreamweed Seed", "Upon touching a Dreamweed it secretes a glittering cloud of confusing gas.",
                                                 ITEMCAT_SEED, CELL(SPRITE_SEED_DREAMWEED),  1 },
    [ITEM_SEED_SUNGRASS]   = { "Sungrass Seed",  "Sungrass is renowned for its sap's healing properties.",
                                                 ITEMCAT_SEED, CELL(SPRITE_SEED_SUNGRASS),   1 },
    [ITEM_SEED_EARTHROOT]  = { "Earthroot Seed", "When a creature touches an Earthroot, its roots " 
                                                 "create a kind of natural armor around it.",
                                                 ITEMCAT_SEED, CELL(SPRITE_SEED_EARTHROOT),  1 },
    [ITEM_SEED_FADELEAF]   = { "Fadeleaf Seed",  "Touching a Fadeleaf will teleport any creature " 
                                                 "to a random place on the current level.",
                                                 ITEMCAT_SEED, CELL(SPRITE_SEED_FADELEAF),   1 },
    [ITEM_SEED_ROTBERRY]   = { "Rotberry Seed",  "Berries of this shrub taste like sweet, sweet death.",
                                                 ITEMCAT_SEED, CELL(SPRITE_SEED_ROTBERRY),   1 },
    /* ── food — fixed sprites ─────────────────────────────── */
    [ITEM_RATION]           = { "Ration of Food",        "Nothing fancy here: dried meat, " 
                                                         "some biscuits - things like that.",
                                                         ITEMCAT_FOOD, CELL(SPRITE_RATION),     1 },
    [ITEM_PASTY]            = { "Pasty",                 "This is authentic Cornish pasty with traditional filling of beef and potato.",
                                                         ITEMCAT_FOOD, CELL(SPRITE_PASTY),      1 },
    [ITEM_MYSTERY_MEAT]     = { "Mystery Meat",          "Eat at your own risk!",
                                                         ITEMCAT_FOOD, CELL(SPRITE_MEAT),       1 },
    [ITEM_STEAK]            = { "Chargrilled meat",      "It looks like a decent steak.",
                                                         ITEMCAT_FOOD, CELL(SPRITE_STEAK),      1 },
    [ITEM_OVERPRICED]       = { "Overpriced Food Ration","It looks exactly like a standard ration of food but smaller.",
                                                         ITEMCAT_FOOD, CELL(SPRITE_OVERPRICED), 1 },
    [ITEM_FROZEN_CARPACCIO] = { "Frozen Carpaccio",      "It's a piece of frozen raw meat. The only way to eat it is " 
                                                         "by cutting thin slices of it. And this way it's suprisingly good.",
                                                         ITEMCAT_FOOD, CELL(SPRITE_CARPACCIO),  1 },
    /* ── misc ─────────────────────────────────────────────── */ //TODO mb split into subcategories?
    [ITEM_ANKH]     = { "Ankh",           "The ancient symbol of immortality grants an ability to return to life after death. " 
                                          "Upon resurrection all non-equipped items are lost.",
                                          ITEMCAT_MISC, CELL(SPRITE_ANKH),     1 },
    [ITEM_DEWDROP]  = { "Dewdrop",        "A crystal clear dewdrop.",
                                          ITEMCAT_MISC, CELL(SPRITE_DEWDROP),  1 },
    [ITEM_MASTERY]  = { "Tome of Mastery","This worn leather book is not that thick, but you feel somehow, " 
                                          "that you can gather a lot from it. Remember though that reading " 
                                          "this tome may require some time.",
                                          ITEMCAT_MISC, CELL(SPRITE_MASTERY),  0 }, //REDO Name = Dungeon.hero != null && Dungeon.hero.subClass != HeroSubClass.NONE ? "Tome of Remastery" : "Tome of Mastery";
    [ITEM_TORCH]    = { "Torch",          "It's an indispensable item in The Demon Halls, which are notorious for their poor ambient lighting.",
                                          ITEMCAT_MISC, CELL(SPRITE_TORCH),    1 },
    [ITEM_BEACON]   = { "Lloyd's Beacon", "Lloyd's beacon is an intricate magic device, that allows you to return to a place you have already been.",
                                          ITEMCAT_MISC, CELL(SPRITE_BEACON),   0 },
    [ITEM_KIT]      = { "Armor Kit",      "Using this kit of small tools and materials anybody can transform any armor into an \"epic armor\", " 
                                          "which will keep all properties of the original armor, but will also provide its wearer a special ability " 
                                          "depending on his class. No skills in tailoring, leatherworking or blacksmithing are required.",
                                          ITEMCAT_MISC, CELL(SPRITE_KIT),      0 },
    [ITEM_VIAL]     = { "Dew Vial",       "You can store excess dew in this tiny vessel for drinking it later. " 
                                          "If the vial is full, in a moment of deadly peril the dew will be " 
                                          "consumed automatically.",
                                          ITEMCAT_MISC, CELL(SPRITE_VIAL),     0 },
    [ITEM_WEIGHT]   = { "Weightstone",    "Using a weightstone, you can balance your melee weapon to increase its speed or accuracy.",
                                          ITEMCAT_MISC, CELL(SPRITE_WEIGHT),   1 },
    [ITEM_BOMB]     = { "Bomb",           "This is a relatively small bomb, filled with black powder. Conveniently, its fuse is lit automatically when the bomb is thrown.",
                                          ITEMCAT_MISC, CELL(SPRITE_BOMB),     1 },
    [ITEM_HONEYPOT] = { "Honeypot",       "There is not much honey in this small honeypot, but there is a golden bee there and it doesn't want to leave it.",
                                          ITEMCAT_MISC, CELL(SPRITE_HONEYPOT), 0 },
    /* ── currency ─────────────────────────────────────────── */
    [ITEM_GOLD] = { "Gold", "A pile of gold coins. " //ORIGINAL A pile of %d gold coins. + there was unique text for 1 coint
                            "Collect gold coins to spend them later in a shop.",
                            ITEMCAT_GOLD, CELL(SPRITE_GOLD), 1 },
    /* ── keys — vault-specific, not in the random loot pool ──────── */
    [ITEM_KEY_IRON]     = { "Iron Key",    "The notches on this ancient iron key are well worn; its leather lanyard " 
                                           "is battered by age. What door might it open?",
                                           ITEMCAT_KEY, CELL(SPRITE_IRON_KEY),     0 },
    [ITEM_KEY_GOLDEN]   = { "Golden Key",  "The notches on this golden key are tiny and intricate. " 
                                           "Maybe it can open some chest lock?",
                                           ITEMCAT_KEY, CELL(SPRITE_GOLDEN_KEY),   0 },
    [ITEM_KEY_SKELETON] = { "Skeleton Key","This key looks serious: its head is shaped like a skull. " 
                                           "Probably it can open some serious door.",
                                           ITEMCAT_KEY, CELL(SPRITE_SKELETON_KEY), 0 },
    /* ── bags — not in the random loot pool ──────────────────────── */
    [ITEM_BAG_SEED_POUCH]    = { "Seed Pouch",      "This small velvet pouch allows you to store any number of seeds in it. Very convenient.",
                                                    ITEMCAT_BAG, CELL(SPRITE_POUCH),  0 },
    [ITEM_BAG_SCROLL_HOLDER] = { "Scroll Holder",   "You can place any number of scrolls into this tubular container. " 
                                                    "It saves room in your backpack and protects scrolls from fire.",
                                                    ITEMCAT_BAG, CELL(SPRITE_HOLDER), 0 },
    [ITEM_BAG_WAND_HOLSTER]  = { "Wand Holster",    "This slim holder is made of leather of some exotic animal. " 
                                                    "It allows to compactly carry up to 12 wands.",
                                                    ITEMCAT_BAG, CELL(SPRITE_HOLSTER),0 },
    [ITEM_BAG_KEYRING]       = { "Keyring",         "This is a copper key ring, that lets you keep all your keys " 
                                                    "separately from the rest of your belongings.",
                                                    ITEMCAT_BAG, CELL(SPRITE_KEYRING),0 },
    /* ── quest — not in the random loot pool ─────────────────────── */
    [ITEM_ROSE]             = { "Dried Rose",       "The rose has dried long ago, but it has kept all its petals somehow.",
                                                    ITEMCAT_QUEST, CELL(SPRITE_ROSE),    0 },
    [ITEM_PICKAXE]          = { "Pickaxe",          "This is a large and sturdy tool for breaking rocks. Probably it can be used as a weapon.",
                                                    ITEMCAT_QUEST, CELL(SPRITE_PICKAXE), 0 },
    [ITEM_ORE]              = { "Dark Gold Ore",    "This metal is called dark not because of its color (it doesn't differ from the normal gold), " 
                                                    "but because it melts under the daylight, making it useless on the surface.",
                                                    ITEMCAT_QUEST, CELL(SPRITE_ORE),     1 },
    [ITEM_SKULL]            = { "Giant Rat Skull",  "It could be a nice hunting trophy, but it smells too bad to place it on a wall.",
                                                    ITEMCAT_QUEST, CELL(SPRITE_SKULL),   0 },
    [ITEM_PHANTOM]          = { "Phantom Fish",     "You can barely see this tiny translucent fish in the air. " 
                                                    "In the water it becomes effectively invisible.",
                                                    ITEMCAT_QUEST, CELL(SPRITE_PHANTOM), 0 },
    [ITEM_DUST]             = { "Corpse Dust",      "The ball of corpse dust doesn't differ outwardly from a regular dust ball. However, " 
                                                    "you know somehow that it's better to get rid of it as soon as possible.",
                                                    ITEMCAT_QUEST, CELL(SPRITE_DUST),    0 },
    [ITEM_TOKEN]            = { "Dwarf Token",      "Many dwarves and some of their larger creations carry these small pieces of metal of unknown purpose. " 
                                                    "Maybe they are jewelry or maybe some kind of ID. Dwarves are strange folk.",
                                                    ITEMCAT_QUEST, CELL(SPRITE_TOKEN),   1 },
    [ITEM_AMULET_OF_YENDOR] = { "Amulet of Yendor", "The Amulet of Yendor is the most powerful known artifact of unknown origin. It is said that the amulet " 
                                                    "is able to fulfil any wish if its owner's will-power is strong enough to \"persuade\" it to do it.",
                                                    ITEMCAT_QUEST, CELL(SPRITE_AMULET),  0 },
};
 
#undef CELL

/* Generator.Category's own weight*/
const float SPAWN_CATEGORY_WEIGHT[ITEMCAT_COUNT] = {
    [ITEMCAT_WEAPON] = 15,
    [ITEMCAT_ARMOR]  = 10,
    [ITEMCAT_WAND]   = 4,
    [ITEMCAT_POTION] = 50,
    [ITEMCAT_SCROLL] = 40,
    [ITEMCAT_RING]   = 2,
    [ITEMCAT_SEED]   = 5,
    [ITEMCAT_FOOD]   = 0,
    [ITEMCAT_MISC]   = 5,
    [ITEMCAT_GOLD]   = 50,
};
 
/* Per-item weight within its category, straight from Generator.java's Category.<X>.probs[] arrays.*/
const float SPAWN_ITEM_WEIGHT[ITEM_TYPE_COUNT] = {
    /* Weapons */
    [ITEM_DAGGER]          = 1,  [ITEM_KNUCKLES]        = 1,
    [ITEM_QUARTERSTAFF]    = 1,  [ITEM_SPEAR]           = 1,
    [ITEM_MACE]            = 1,  [ITEM_SWORD]           = 1,
    [ITEM_LONGSWORD]       = 1,  [ITEM_BATTLE_AXE]      = 1,
    [ITEM_WAR_HAMMER]      = 1,  [ITEM_GLAIVE]          = 1,
    [ITEM_SHORT_SWORD]     = 0,  [ITEM_DART]            = 0,
    [ITEM_JAVELIN]         = 1,  [ITEM_INCENDIARY_DART] = 1,
    [ITEM_CURARE_DART]     = 1,  [ITEM_SHURIKEN]        = 1,
    [ITEM_BOOMERANG]       = 0,  [ITEM_TAMAHAWK]        = 1,
 
    /* Armor */
    [ITEM_CLOTH_ARMOR]     = 1,  [ITEM_LEATHER_ARMOR]   = 1,
    [ITEM_MAIL_ARMOR]      = 1,  [ITEM_SCALE_ARMOR]     = 1,
    [ITEM_PLATE_ARMOR]     = 1,
 
    /* Wands */
    [ITEM_WAND_TELEPORTATION]  = 10, [ITEM_WAND_SLOWNESS]  = 10,
    [ITEM_WAND_FIREBOLT]       = 15, [ITEM_WAND_REGROWTH]  = 6,
    [ITEM_WAND_POISON]         = 10, [ITEM_WAND_BLINK]     = 11,
    [ITEM_WAND_LIGHTNING]      = 15, [ITEM_WAND_AMOK]      = 10,
    [ITEM_WAND_REACH]          = 6,  [ITEM_WAND_FLOCK]     = 10,
    [ITEM_WAND_MAGIC_MISSILE]  = 0,  /* fixed starting wand, never random */
    [ITEM_WAND_DISINTEGRATION] = 5,  [ITEM_WAND_AVALANCHE] = 5,
 
    /* Potions */
    [ITEM_POTION_HEALING]       = 45, [ITEM_POTION_EXPERIENCE]    = 4,
    [ITEM_POTION_TOXIC_GAS]     = 15, [ITEM_POTION_PARALYTIC_GAS] = 10,
    [ITEM_POTION_LIQUID_FLAME]  = 15, [ITEM_POTION_LEVITATION]    = 10,
    [ITEM_POTION_STRENGTH]      = 0,  [ITEM_POTION_MIND_VISION]   = 20,
    [ITEM_POTION_PURITY]        = 12, [ITEM_POTION_INVISIBILITY]  = 10,
    [ITEM_POTION_MIGHT]         = 0,  [ITEM_POTION_FROST]         = 10,
 
    /* Scrolls */
    [ITEM_SCROLL_IDENTIFY]      = 30, [ITEM_SCROLL_TELEPORTATION] = 10,
    [ITEM_SCROLL_REMOVE_CURSE]  = 15, [ITEM_SCROLL_RECHARGING]    = 10,
    [ITEM_SCROLL_MAGIC_MAPPING] = 15, [ITEM_SCROLL_CHALLENGE]     = 12,
    [ITEM_SCROLL_TERROR]        = 8,  [ITEM_SCROLL_LULLABY]       = 8,
    [ITEM_SCROLL_PSIONIC_BLAST] = 4,  [ITEM_SCROLL_MIRROR_IMAGE]  = 6,
    [ITEM_SCROLL_UPGRADE]       = 0,  [ITEM_SCROLL_ENCHANTMENT]   = 1,
 
    /* Rings */
    [ITEM_RING_MENDING]    = 1, [ITEM_RING_DETECTION] = 1,
    [ITEM_RING_SHADOWS]    = 1, [ITEM_RING_POWER]     = 1,
    [ITEM_RING_HERBALISM]  = 1, [ITEM_RING_ACCURACY]  = 1,
    [ITEM_RING_EVASION]    = 1, [ITEM_RING_SATIETY]   = 1,
    [ITEM_RING_HASTE]      = 1, [ITEM_RING_ELEMENTS]  = 1,
    [ITEM_RING_HAGGLER]    = 0, [ITEM_RING_THORNS]    = 0,
 
    /* Seeds */
    [ITEM_SEED_FIREBLOOM]  = 1, [ITEM_SEED_ICECAP]    = 1,
    [ITEM_SEED_SORROWMOSS] = 1, [ITEM_SEED_DREAMWEED]  = 1,
    [ITEM_SEED_SUNGRASS]   = 1, [ITEM_SEED_EARTHROOT]  = 1,
    [ITEM_SEED_FADELEAF]   = 1, [ITEM_SEED_ROTBERRY]   = 0,
 
    /* Food */
    [ITEM_RATION] = 4, [ITEM_PASTY] = 1, [ITEM_MYSTERY_MEAT] = 0,
 
    /* Misc */
    [ITEM_BOMB] = 2, [ITEM_HONEYPOT] = 1,
 
    [ITEM_GOLD] = 1,
};
 