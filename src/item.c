#include "item.h"
#include "item_generator.h"
#include "item_spawn.h"
#include "level_utils.h"

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
int item_max_durability(ItemType type, int level) {
    int base;
    switch (ITEM_DB[type].cat) {
        /* Formulas straight from Weapon/Armor/Wand/Ring.java's
           maxDurability(lvl) overrides. */
        case ITEMCAT_WEAPON: base = 5 * (level < 16 ? 16 - level : 1); break;
        case ITEMCAT_ARMOR:  base = 6 * (level < 16 ? 16 - level : 1); break;
        case ITEMCAT_WAND:   base = 6 * (level < 16 ? 16 - level : 1); break;
        case ITEMCAT_RING:
            /* Ring.java returns Integer.MAX_VALUE for lvl<=1 — an
               unenchanted (or cursed) ring never wears down.
               durability is int16_t here, so INT16_MAX stands in for
               "infinite": at one tick of wear per turn, a level-1
               ring would need 32,767 turns worn to matter — far
               beyond any real run, so it's a faithful stand-in for
               the storage type actually available. */
            base = (level <= 1) ? INT16_MAX : 100 * (level < 16 ? 16 - level : 1);
            break;
        default:             base = 1;              break; /* Item.java's own base-class default */
    }
    return base < 1 ? 1 : base;
}
 
void item_stack_init(ItemStack *stack, ItemType type, int quantity) {
    stack->type     = type;
    stack->quantity = quantity;
    stack->level    = 0; 
    stack->cursed = 0;
 
    ItemCategory cat = ITEM_DB[type].cat;
    stack->level_known = (cat == ITEMCAT_WEAPON || cat == ITEMCAT_ARMOR) ? 1 : 0;
    if (!ITEM_DB[type].stackable) {
        switch (cat) {
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
    if (ITEM_DB[stack->type].stackable)
        return;
    stack->level++;
    stack->cursed       = 0;
    stack->cursed_known = 1;
    item_fix(stack);
}
 
void item_degrade(ItemStack *stack) {
    if (ITEM_DB[stack->type].stackable)
        return;
    stack->level--;
    item_fix(stack);
}

 
ItemUseResult item_use(ItemStack *stack) {
    if (ITEM_DB[stack->type].stackable)
        return ITEM_USE_OK;
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
 

ItemConsumeResult item_consume(ItemStack *stack) { //temporal bullshit variant for using consumable items
    if (!ITEM_DB[stack->type].stackable)
        return ITEM_USE_FAIL;

    return ITEM_USE_CONSUMED;
}


