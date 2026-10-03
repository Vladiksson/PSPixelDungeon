#include "item.h"
#include "item_generator.h"
#include "item_spawn.h"
#include "level_geometry.h"

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
void level_spawn_items(Level *lvl, int depth) {
    int target = rng_range(4, 8) + depth / 4;
    if (target > MAX_LEVEL_ITEMS)
        target = MAX_LEVEL_ITEMS;
    int attempts = 0;
    while (lvl->num_heaps < target && attempts < 200) {
        attempts++;
        int idx = xy_to_idx(rng_range(0, LEVEL_W - 1), rng_range(0, LEVEL_H - 1));

        if (!tile_free_for_item(lvl, idx))
            continue;

        ItemType type;

        if (level_take_spawn_item(lvl, &type))
            spawn_item(lvl, idx, depth, type);
        else
            spawn_random_item(lvl, idx, depth);
    }
}
 
/* ══════════════════════════════════════════════════════════
   INSTANCE STATE — level / curse / durability
   ══════════════════════════════════════════════════════════ */
int item_max_durability(ItemType type, int level) {
    int base = 0;
    if(ITEM_DB[type].upgradable){
        switch (ITEM_DB[type].cat) {
            case ITEMCAT_WEAPON:
                base = (type == ITEM_BOOMERANG ? 8 : 5) * (level < 16 ? 16 - level : 1);
                break;
            case ITEMCAT_ARMOR:  base = 6 * (level < 16 ? 16 - level : 1); break;
            case ITEMCAT_WAND:   base = 6 * (level < 16 ? 16 - level : 1); break;
            case ITEMCAT_RING:
                base = (level <= 1) ? INT16_MAX : 100 * (level < 16 ? 16 - level : 1);
                break;
            default:
                break; 
        }
    }
    return base;
}
 
void item_stack_init(ItemStack *stack, ItemType type, int quantity) {
    stack->type     = type;
    stack->quantity = quantity;
    stack->level    = 0; 
    stack->cursed = 0;
 
    if (ITEM_DB[type].upgradable) {
        switch (ITEM_DB[type].cat) {
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
    stack->level_known  = 0; 
    stack->cursed_known = 0;
    stack->durability = (int16_t)item_max_durability(type, stack->level);
}

void item_stack_init_start(ItemStack *stack, ItemType type, int quantity) {
    stack->type     = type;
    stack->quantity = quantity;
    stack->level = stack->cursed = 0;
    stack->level_known = stack->cursed_known = 1; 
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
    if (!ITEM_DB[stack->type].upgradable)
        return;
    stack->level++;
    stack->cursed       = 0;
    stack->cursed_known = 1;
    item_fix(stack);
}
 
void item_degrade(ItemStack *stack) {
    if (!ITEM_DB[stack->type].upgradable)
        return;
    stack->level--;
    item_fix(stack);
}

 
ItemUseResult item_use(ItemStack *stack) {
    if (ITEM_DB[stack->type].upgradable)
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
 

ItemConsumeResult item_consume(ItemStack *stack) { //temporary bullshit variant for using consumable items
    if (!ITEM_DB[stack->type].stackable)
        return ITEM_USE_FAIL;

    return ITEM_USE_CONSUMED;
}



/*═══════════════════════════════════════════════════════════════
    Weapon / armor stats
═══════════════════════════════════════════════════════════════*/
int weapon_str_req(ItemStack *w) {
    const ItemDef *d = &ITEM_DB[w->type];
    int base = (d->weapon.type == WEAPON_MELEE) ? d->weapon.melee.str : d->weapon.missile.str;
    return base - item_effective_level(w);
}
float weapon_acu(ItemStack *w) {
    const ItemDef *d = &ITEM_DB[w->type];
    return (d->weapon.type == WEAPON_MELEE) ? d->weapon.melee.acu : d->weapon.missile.acu;
}
float weapon_dly(ItemStack *w) {
    const ItemDef *d = &ITEM_DB[w->type];
    return (d->weapon.type == WEAPON_MELEE) ? d->weapon.melee.dly : d->weapon.missile.dly;
}

/* MeleeWeapon.min()/max0() and MissileWeapon subclasses' min()/max(). */
void weapon_min_max(ItemStack *w, int *out_min, int *out_max) {
    const ItemDef *d = &ITEM_DB[w->type];
    int level = item_effective_level(w);

    if (d->weapon.type == WEAPON_MELEE) {
        int tier = d->weapon.melee.tier;
        int min0 = tier;
        int max0 = (int)((tier * tier - tier + 10) / d->weapon.melee.acu * d->weapon.melee.dly);
        *out_min = min0 + level;
        *out_max = max0 + level * tier;
    } else {
        *out_min = d->weapon.missile.dmg_min;
        *out_max = d->weapon.missile.dmg_max;
        if (d->upgradable) {          /* Boomerang is the only one */
            *out_min += level;
            *out_max += level * 2;
        }
    }
}
int weapon_average_damage(ItemStack *w) {
    int min, max;
    weapon_min_max(w, &min, &max);
    return (min + (max - min) / 2);
}

int armor_str_req(ItemStack *a) {
    return ITEM_DB[a->type].armor.str - item_effective_level(a);
}
int armor_dr(ItemStack *stack) {
    int dr = ITEM_DB[stack->type].armor.tier * (2 + item_effective_level(stack)); // TODO add + (glyph == null ? 0 : 1));
    return dr < 0 ? 0 : dr;
}
int get_armor_tier(const ItemStack *stack) {
    if (stack->type == ITEM_NONE) return 0;
    if (ITEM_DB[stack->type].cat != ITEMCAT_ARMOR) return 0;
    return ITEM_DB[stack->type].armor.tier;
}