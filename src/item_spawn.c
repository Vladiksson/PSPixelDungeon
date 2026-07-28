#include "item.h"
#include "item_generator.h"
#include "level_utils.h"

/* ══════════════════════════════════════════════════════════
   HEAP
   ══════════════════════════════════════════════════════════ */
int heap_sprite_cell(const Heap *h, uint8_t *out_col, uint8_t *out_row) {
    int sprite;
 
    switch (h->type) {
        case HEAP_NORMAL:
        case HEAP_FOR_SALE:
            if (h->count <= 0) return 0;
            item_display_sprite(h->items[0].type, out_col, out_row);
            return 1;
 
        case HEAP_CHEST:
        case HEAP_MIMIC:         sprite = SPRITE_CHEST;         break;
        case HEAP_LOCKED_CHEST:  sprite = SPRITE_LOCKED_CHEST;  break;
        case HEAP_CRYSTAL_CHEST: sprite = SPRITE_CRYSTAL_CHEST; break;
        case HEAP_TOMB:          sprite = SPRITE_TOMB;          break;
        case HEAP_SKELETON:      sprite = SPRITE_BONES;         break;
        case HEAP_HIDDEN:        sprite = SPRITE_HIDDEN;        break;
        default: return 0;
    }
 
    *out_col = sprite % ITEMS_SHEET_COLS;
    *out_row = sprite / ITEMS_SHEET_COLS;
    return 1;
}

Heap *level_create_heap(Level *lvl, int x, int y, HeapType type) {
    if (lvl->num_heaps >= MAX_LEVEL_ITEMS)
        return NULL;
    Heap *h = &lvl->heaps[lvl->num_heaps++];
    h->type   = type;
    h->count  = 0;
    h->x      = x;
    h->y      = y;
    h->active = 1;
    return h;
}

Heap *level_heap_at(Level *lvl, int x, int y) {
    for (int i = 0; i < lvl->num_heaps; i++) {
        Heap *h = &lvl->heaps[i];
        if (h->active && h->x == x && h->y == y)
            return h;
    }
    return NULL;
}

Heap *level_heap_at_idx(Level *lvl, int idx) {
    Point p = idx_to_xy(idx);
    return level_heap_at(lvl, p.x, p.y);
}

static int heap_insert(Heap *heap, ItemStack stack) {
    if (ITEM_DB[stack.type].stackable) {
        for (int i = 0; i < heap->count; i++) {
            if (heap->items[i].type == stack.type) {
                heap->items[i].quantity += stack.quantity;
                return 1;
            }
        }
    }
    if (heap->count >= MAX_HEAP_ITEMS)
        return 0;
 
    heap->items[heap->count] = stack;
    heap->count++;
    return 1;
}
 
int heap_add_stack(Heap *heap, ItemStack stack) {
    return heap_insert(heap, stack);
}
 
int heap_add_item(Heap *heap, ItemType type, int quantity) {
    ItemStack stack;
    item_stack_init(&stack, type, quantity);
    return heap_insert(heap, stack);
}


/*═══════════════════════════════════════════════════════════════
    Item spawning
═══════════════════════════════════════════════════════════════*/
Heap *spawn_item(Level *lvl, int idx, int depth, ItemType type) {
    if (type < 0 || type >= ITEM_TYPE_COUNT)
        return NULL;
    Point p = idx_to_xy(idx);
    Heap *heap = level_heap_at(lvl, p.x, p.y);
    if (!heap) {
        heap = level_create_heap(lvl, p.x, p.y, HEAP_NORMAL);
        if (!heap)
            return NULL;
    }
    if (!heap_add_item(heap, type, (type == ITEM_GOLD) ? gold_amount(depth) : 1))
        return NULL;
    return heap;
}
 
Heap *spawn_random_item(Level *lvl, int idx, int depth) {
    return spawn_item(lvl, idx, depth, random_item());
}
 
Heap *spawn_random_item_of_category(Level *lvl, int idx, int depth, ItemCategory cat) {
    return spawn_item(lvl, idx, depth, random_item_of_category(cat));
}
 
Heap *spawn_item_heap(Level *lvl, int idx, int depth, ItemType type, HeapType heap_type){
    Heap *heap = spawn_item(lvl, idx, depth, type);
    if (heap) heap->type = heap_type;
    return heap;
}
 
Heap *spawn_item_amount_heap(Level *lvl, int idx, int depth, ItemType type, int amount, HeapType heap_type){
    if (type < 0 || type >= ITEM_TYPE_COUNT)
        return NULL;
    Point p = idx_to_xy(idx);
    Heap *heap = level_heap_at(lvl, p.x, p.y);
    if (!heap) {
        heap = level_create_heap(lvl, p.x, p.y, heap_type);
        if (!heap)
            return NULL;
    } else {
        heap->type = heap_type;
    }
    int quantity = (amount > 0) ? amount : 1;
    if (!heap_add_item(heap, type, quantity))
        return NULL;
    return heap;
}
 
Heap *spawn_random_item_of_category_heap(Level *lvl, int idx, int depth, ItemCategory cat, HeapType heap_type){
    Heap *heap = spawn_random_item_of_category(lvl, idx, depth, cat);
    if (heap) heap->type = heap_type;
    return heap;
}
 