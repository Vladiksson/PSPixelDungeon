#include "bag.h"
#include <stdlib.h>   /* qsort — bag_sort */

/*═══════════════════════════════════════════════════════════════
    Bags — see bag.h. Ported from Bag.java / SeedPouch.java /
    ScrollHolder.java / WandHolster.java / Keyring.java. The backpack
    (BAG_BACKPACK) is folded into the same struct/API — see bag.h's
    header comment for why.

    Not ported: WandHolster.collect()/onDetach() charging every held
    wand as it enters/leaves the holster. This port doesn't have a
    wand-charging system yet, so player_acquire_bag() in player.c
    just files the wands away without touching charge state — wire it
    in here once that exists.
═══════════════════════════════════════════════════════════════*/

int bag_capacity_for_type(BagType type) {
    switch (type) {
        case BAG_BACKPACK:      return BACKPACK_SLOTS;
        case BAG_SEED_POUCH:    return 8;
        case BAG_SCROLL_HOLDER: return 12;
        case BAG_WAND_HOLSTER:  return 12;
        case BAG_KEYRING:       return 12;
        default:                return 0;
    }
}

ItemType bag_item_type(BagType type) {
    switch (type) {
        case BAG_SEED_POUCH:    return ITEM_BAG_SEED_POUCH;
        case BAG_SCROLL_HOLDER: return ITEM_BAG_SCROLL_HOLDER;
        case BAG_WAND_HOLSTER:  return ITEM_BAG_WAND_HOLSTER;
        case BAG_KEYRING:       return ITEM_BAG_KEYRING;
        default:                return ITEM_NONE; /* BAG_BACKPACK */
    }
}

int bag_type_for_item(ItemType item_type) {
    switch (item_type) {
        case ITEM_BAG_SEED_POUCH:    return BAG_SEED_POUCH;
        case ITEM_BAG_SCROLL_HOLDER: return BAG_SCROLL_HOLDER;
        case ITEM_BAG_WAND_HOLSTER:  return BAG_WAND_HOLSTER;
        case ITEM_BAG_KEYRING:       return BAG_KEYRING;
        default:                     return -1;
    }
}

int bag_grabs(BagType bag_type, ItemType item_type) {
    if (item_type < 0 || item_type >= ITEM_TYPE_COUNT) return 0;

    ItemCategory cat = ITEM_DB[item_type].cat;
    switch (bag_type) {
        case BAG_SEED_POUCH:    return cat == ITEMCAT_SEED;
        case BAG_SCROLL_HOLDER: return cat == ITEMCAT_SCROLL;
        case BAG_WAND_HOLSTER:  return cat == ITEMCAT_WAND;
        case BAG_KEYRING:       return cat == ITEMCAT_KEY;
        /* General storage: anything except gold, which is never an
           ItemStack (tracked as Player.gold) — this now includes bag
           Items themselves, since a pouch sits in the backpack as a
           real, droppable item. */
        case BAG_BACKPACK:      return cat != ITEMCAT_GOLD;
        default:                return 0;
    }
}

const char *bag_name(BagType type) {
    ItemType it = bag_item_type(type);
    return (it != ITEM_NONE) ? item_name(it) : "Backpack";
}

void bag_init(Bag *bag, BagType type) {
    bag->type     = type;
    bag->count    = 0;
    bag->capacity = bag_capacity_for_type(type);

    ItemType it = bag_item_type(type);
    if (it != ITEM_NONE) {
        item_stack_init(&bag->item, it, 1);
    } else {
        bag->item = item_stack_none();
    }

    for (int i = 0; i < MAX_BAG_ITEMS; i++) {
        bag->items[i] = item_stack_none();
    }
}

void bag_clear(Bag *bag) {
    bag->item     = item_stack_none();
    bag->count    = 0;
    bag->capacity = 0;
    for (int i = 0; i < MAX_BAG_ITEMS; i++) {
        bag->items[i] = item_stack_none();
    }
}

int bag_add_stack(Bag *bag, ItemStack stack) {
    if (bag->capacity <= 0)                return 0;
    if (!bag_grabs(bag->type, stack.type)) return 0;

    if (ITEM_DB[stack.type].stackable) {
        for (int i = 0; i < bag->count; i++) {
            if (bag->items[i].type == stack.type) {
                bag->items[i].quantity += stack.quantity;
                return 1;
            }
        }
    }

    if (bag->count >= bag->capacity) return 0;

    bag->items[bag->count] = stack;
    bag->count++;
    return 1;
}

static void bag_remove_slot(Bag *bag, int slot) {
    for (int i = slot; i < bag->count - 1; i++) {
        bag->items[i] = bag->items[i + 1];
    }
    bag->count--;
}

int bag_remove_item(Bag *bag, int slot, int quantity) {
    if (slot < 0 || slot >= bag->count || quantity <= 0) return 0;
    if (bag->items[slot].quantity < quantity)             return 0;

    bag->items[slot].quantity -= quantity;

    if (bag->items[slot].quantity == 0) {
        bag_remove_slot(bag, slot);
    }
    return 1;
}

int bag_find_item(const Bag *bag, ItemType type) {
    for (int i = 0; i < bag->count; i++) {
        if (bag->items[i].type == type) return i;
    }
    return -1;
}

static int item_type_cmp(const void *a, const void *b) {
    ItemType ta = ((const ItemStack *)a)->type;
    ItemType tb = ((const ItemStack *)b)->type;
    return (int)ta - (int)tb;
}

void bag_sort(Bag *bag) {
    qsort(bag->items, (size_t)bag->count, sizeof(ItemStack), item_type_cmp);
}