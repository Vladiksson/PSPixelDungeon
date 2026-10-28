#pragma once
#include "item.h"
#include "char.h"
#include "ballistica.h"

int wand_hits_chars(ItemType type);

void wand_consume_charge(ItemStack *stack);

void wand_zap(const ItemStack *stack, Char *caster, Level *lvl, const Ballistica *path, int cell);

void wand_bolt_set_flying(ItemType type);
void wand_bolt_clear(void);
int  wand_bolt_color(uint8_t *r, uint8_t *g, uint8_t *b);