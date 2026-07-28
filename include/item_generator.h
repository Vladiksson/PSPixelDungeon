#pragma once 

void generate_ring(ItemStack *stack);
void generate_weapon(ItemStack *stack);
void generate_armor(ItemStack *stack);
void generate_wand(ItemStack *stack);
 
/* Weighted random item type (adapted from Generator.java). */
ItemType random_item_of_category(ItemCategory cat);
ItemType random_item(void);
int gold_amount(int depth);