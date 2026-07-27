#include "item.h"
#include "level.h"
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
    stack->level    = 0; 
 
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
 
