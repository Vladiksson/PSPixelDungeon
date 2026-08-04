#pragma once
#include "item.h"

/* ══════════════════════════════════════════════════════════
   BAGS port of Bag.java / SeedPouch.java / ScrollHolder.java /
   WandHolster.java / Keyring.java (com.watabou.pixeldungeon.items.bags)
   ══════════════════════════════════════════════════════════ */
typedef enum {
   BAG_BACKPACK = 0,
   BAG_KEYRING,
   BAG_SEED_POUCH,
   BAG_SCROLL_HOLDER,
   BAG_WAND_HOLSTER,
   BAG_TYPE_COUNT
} BagType;

#define MAX_BAG_ITEMS BACKPACK_SLOTS

typedef struct {
   BagType   type;
   ItemStack item; /* BAGS, item_stack_none() for BAG_BACKPACK. */
   ItemStack items[MAX_BAG_ITEMS];
   int       count;
   int       capacity;             /* 0 = storage never initialized */
} Bag;

int bag_capacity_for_type(BagType type);

ItemType bag_item_type(BagType type);

int bag_type_for_item(ItemType item_type);

/* Mirrors each Bag subclass's grab() override in the original:
   SeedPouch    -> ITEMCAT_SEED    (Plant.Seed)
   ScrollHolder -> ITEMCAT_SCROLL  (Scroll)
   WandHolster  -> ITEMCAT_WAND    (Wand)
   Keyring      -> ITEMCAT_KEY     (Key)*/
int bag_grabs(BagType bag_type, ItemType item_type);

const char *bag_name(BagType type);

void bag_init(Bag *bag, BagType type);

/* Empty the bag's contents and reset capacity to 0 */
void bag_clear(Bag *bag);

/* Try to add a stack to this bag's contents*/
int bag_add_stack(Bag *bag, ItemStack stack);

/* Remove `quantity` from bag slot `slot`, sliding the freed slot down
   over the rest once it empties out. */
int bag_remove_item(Bag *bag, int slot, int quantity);

/* Slot holding `type` in this bag, or -1. */
int bag_find_item(const Bag *bag, ItemType type);

/* Sort this bag's contents by ItemType. */
void bag_sort(Bag *bag);