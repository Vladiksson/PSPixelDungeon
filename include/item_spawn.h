#pragma once 
#include "level.h"

Heap *level_create_heap(Level *lvl, int pos, HeapType type);
Heap *level_heap_at(Level *lvl, int pos);

int heap_add_item(Heap *heap, ItemType type, int quantity);
Heap *spawn_item(Level *lvl, int idx, int depth, ItemType type);
Heap *spawn_random_item(Level *lvl, int idx, int depth);
Heap *spawn_random_item_of_category(Level *lvl, int idx, int depth, ItemCategory cat);
Heap *spawn_item_heap(Level *lvl, int idx, int depth, ItemType type, HeapType heap_type);
Heap *spawn_item_amount_heap(Level *lvl, int idx, int depth, ItemType type, int amount, HeapType heap_type);
Heap *spawn_random_item_of_category_heap(Level *lvl, int idx, int depth, ItemCategory cat, HeapType heap_type);

void level_add_item_to_spawn(Level *lvl, ItemType type);
int level_take_spawn_item(Level *lvl, ItemType *out);

void level_spawn_items (Level *lvl, int depth);
void place_room_loot(Level *lvl, Room *r, int depth, int count);
