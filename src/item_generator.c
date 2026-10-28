#include "item.h"
#include "item_generator.h"
#include "rng.h"

void generate_ring(ItemStack *stack) {
    int lvl = rng_range(1,3);

    if (rng_int(100) < 30) {
        stack->level -= lvl;
        stack->cursed = 1;
    } else {
        stack->level += lvl;
        stack->cursed = 0;
    }
    item_fix(stack);
    stack->usages_to_id  = ITEM_RING_TICKS_TO_KNOW;
}

/* common part Weapon.random() and Armor.random() */
static void roll_upgrade_or_curse(ItemStack *stack) {
    if (rng_float() < 0.4f) {
        int n = 1;
        if (rng_int(3) == 0) {
            n++;
            if (rng_int(3) == 0) {
                n++;
            }
        }
        if (rng_int(2) == 0) {
            item_upgrade(stack);
        } else {
            item_degrade(stack);
            stack->cursed = 1;
        }
    }
}
 
void generate_weapon(ItemStack *stack) {
    roll_upgrade_or_curse(stack);
    stack->usages_to_id  = ITEM_WEAPON_USAGES_TO_KNOW;
}
 
void generate_armor(ItemStack *stack) {
    roll_upgrade_or_curse(stack);
    stack->usages_to_id  = ITEM_ARMOR_USAGES_TO_KNOW;
    /* Armor.random() also rolls a 1-in-10 chance to inscribe() a
       random glyph. There's no glyph/enchantment system on
       ItemStack yet (same gap as Weapon's enchantments) */
}
 
void generate_wand(ItemStack *stack) {
    if (rng_float() < 0.5f) {
        item_upgrade(stack);
        if (rng_float() < 0.15f) {
            item_upgrade(stack);
        }
    }
    stack->usages_to_id  = ITEM_WAND_USAGES_TO_KNOW;
}
 
/* ══════════════════════════════════════════════════════════
   RANDOM ITEM SELECTION  —  Generator.random() / Category.prob
   ══════════════════════════════════════════════════════════ */

ItemType random_item_of_category(ItemCategory cat) {
    float total = 0.0f;
    for (int t = 0; t < ITEM_TYPE_COUNT; t++)
        if (ITEM_DB[t].cat == cat) total += SPAWN_ITEM_WEIGHT[t];
 
    if (total <= 0.0f)
        return ITEM_NONE;
 
    float roll = rng_float() * total;
    for (int t = 0; t < ITEM_TYPE_COUNT; t++) {
        if (ITEM_DB[t].cat != cat) continue;
        if (roll < SPAWN_ITEM_WEIGHT[t]) return (ItemType)t;
        roll -= SPAWN_ITEM_WEIGHT[t];
    }

    for (int t = ITEM_TYPE_COUNT - 1; t >= 0; t--)
        if (ITEM_DB[t].cat == cat && SPAWN_ITEM_WEIGHT[t] > 0.0f) return (ItemType)t;
    return ITEM_NONE;
}

ItemType random_item(void) {
    float total = 0.0f;
    for (int c = 0; c < ITEMCAT_COUNT; c++) total += SPAWN_CATEGORY_WEIGHT[c];
 
    float roll = rng_float() * total;
    for (int c = 0; c < ITEMCAT_COUNT; c++) {
        if (roll < SPAWN_CATEGORY_WEIGHT[c]) {
            ItemType t = random_item_of_category((ItemCategory)c);
            return (t == ITEM_NONE) ? ITEM_GOLD : t;
        }
        roll -= SPAWN_CATEGORY_WEIGHT[c];
    }
    return ITEM_GOLD;
}
 
int gold_amount(int depth) {
    int d = depth + 1;
    return rng_range(20 + d * 10, 40 + d * 20);
}
