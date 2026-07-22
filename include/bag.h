#pragma once
#include "item.h"

/* ══════════════════════════════════════════════════════════
   BAGS — port of Bag.java / SeedPouch.java / ScrollHolder.java /
   WandHolster.java / Keyring.java (com.watabou.pixeldungeon.items.bags)
   ══════════════════════════════════════════════════════════
   Every container the player carries — the backpack included — is
   one of these now. bags[BAG_BACKPACK] (see Player in player.h) is
   set up in player_init() and always owned; it holds everything the
   four specialty pouches don't. bags[BAG_SEED_POUCH..BAG_KEYRING]
   start at capacity == 0 (not owned) until player_acquire_bag()
   grants one. This replaces the old flat Player.inventory[] array
   entirely — there's no separate "inventory" concept left, just
   bags, the same way the original's Belongings holds a backpack Bag
   plus references to whichever specialty Bags have been bought.

   The `item` field is what still lets a bag be drawn through the
   normal ItemStack-based code paths (icon, name) — see the tab strip
   in inventory_screen.c, this port's rough equivalent of WndBag's
   BagTab icons. BAG_BACKPACK has no such Item (there's no "Backpack"
   ItemType — you always start with one, same as the original), so
   its `item` field is just item_stack_none().
   ══════════════════════════════════════════════════════════ */

typedef enum {
    BAG_BACKPACK = 0,
    BAG_SEED_POUCH,
    BAG_SCROLL_HOLDER,
    BAG_WAND_HOLSTER,
    BAG_KEYRING,
    BAG_TYPE_COUNT
} BagType;

/* Largest per-type capacity — the backpack (BACKPACK_SLOTS, from
   item.h) is the biggest bag there is now; ScrollHolder/WandHolster/
   Keyring are 12, SeedPouch is 8 (see bag_capacity_for_type). Every
   Bag reserves this many slots; `capacity` below is what's actually
   usable for that particular instance. */
#define MAX_BAG_ITEMS BACKPACK_SLOTS

/* One container. capacity == 0 means the player doesn't own this bag
   yet — mirrors Belongings only holding a reference to a specialty
   bag once its Item has actually been picked up, in the original.
   bags[BAG_BACKPACK] is the one exception: player_init() gives it a
   capacity right away and it never goes back to 0. */
typedef struct {
    BagType   type;
    ItemStack item;                 /* the bag's own item — icon/name/drop.
                                        item_stack_none() for BAG_BACKPACK. */
    ItemStack items[MAX_BAG_ITEMS];
    int       count;
    int       capacity;             /* 0 = not owned */
} Bag;

/* Capacity a bag of this type has once owned — BACKPACK_SLOTS for
   BAG_BACKPACK, 8/12/12/12 for the specialty pouches. */
int bag_capacity_for_type(BagType type);

/* The ItemType that represents *owning* this bag (ITEM_BAG_SEED_POUCH
   etc.) — its icon and display name both come from here via ITEM_DB,
   so they stay in sync with item.c automatically. ITEM_NONE for
   BAG_BACKPACK, which has no Item of its own. */
ItemType bag_item_type(BagType type);

/* Reverse lookup: which BagType does picking up `item_type` grant, or
   -1 if it doesn't grant one at all. */
int bag_type_for_item(ItemType item_type);

/* Does a bag of this type accept `item_type`? Mirrors each Bag
   subclass's grab() override in the original:
     SeedPouch    -> ITEMCAT_SEED    (Plant.Seed)
     ScrollHolder -> ITEMCAT_SCROLL  (Scroll)
     WandHolster  -> ITEMCAT_WAND    (Wand)
     Keyring      -> ITEMCAT_KEY     (Key)
   The backpack is general storage: it takes anything except gold
   (never an ItemStack — tracked as Player.gold) and bag Items
   themselves (owning one activates a satellite container rather than
   occupying a slot — see player_acquire_bag in player.c). */
int bag_grabs(BagType bag_type, ItemType item_type);

/* Display name ("Seed Pouch", "Keyring", …), or "Backpack" for
   BAG_BACKPACK. */
const char *bag_name(BagType type);

/* Set up `bag` as a freshly-owned container of this type: capacity,
   its own item stack (for display), and an empty items[]. Used both
   for the backpack in player_init() and for a specialty pouch in
   player_acquire_bag(). Does NOT reach into any other bag for
   matching items already lying around loose — that's
   player_acquire_bag()'s job, since it needs the Player to reach into
   bags[BAG_BACKPACK]. */
void bag_init(Bag *bag, BagType type);

/* Empty the bag and mark it unowned (capacity = 0). Leaves bag->type
   alone — this un-owns the slot, it doesn't repurpose it for a
   different bag type. Never call this on bags[BAG_BACKPACK] once
   player_init() has set it up — it has no "unowned" state. */
void bag_clear(Bag *bag);

/* Try to add a stack to this bag, merging into a matching stackable
   slot first (same rule as before: ITEM_DB[type].stackable). Returns
   0 if the bag doesn't accept this item type, isn't owned (capacity
   == 0), or is already full. */
int bag_add_stack(Bag *bag, ItemStack stack);

/* Remove `quantity` from bag slot `slot`, sliding the freed slot down
   over the rest once it empties out. */
int bag_remove_item(Bag *bag, int slot, int quantity);

/* Slot holding `type` in this bag, or -1. */
int bag_find_item(const Bag *bag, ItemType type);

/* Sort this bag's contents by ItemType. */
void bag_sort(Bag *bag);