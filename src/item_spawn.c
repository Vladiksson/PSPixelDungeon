#include "item.h"
#include "level.h"
#include "level_utils.h"
/* ══════════════════════════════════════════════════════════
   RANDOM ITEM SELECTION  —  Generator.random() / Category.prob
   ══════════════════════════════════════════════════════════ */
typedef struct { ItemCategory cat; int weight; } CatWeight;

static const CatWeight SPAWN_WEIGHTS[] = {
    { ITEMCAT_WEAPON, 15 },
    { ITEMCAT_ARMOR,  10 },
    { ITEMCAT_POTION, 50 },
    { ITEMCAT_SCROLL, 40 },
    { ITEMCAT_WAND,    4 },
    { ITEMCAT_RING,    2 },
    { ITEMCAT_SEED,    5 },
    { ITEMCAT_FOOD,    8 },   /* was 0 in Generator.java */
    { ITEMCAT_GOLD,   50 },
    { ITEMCAT_MISC,    5 },
};
#define N_SPAWN_CATS (int)(sizeof(SPAWN_WEIGHTS) / sizeof(SPAWN_WEIGHTS[0]))

ItemType random_item_of_category(ItemCategory cat) {
    ItemType pool[ITEM_TYPE_COUNT];
    int n = 0;
    for (int i = 0; i < ITEM_TYPE_COUNT; i++)
        if (ITEM_DB[i].cat == cat) pool[n++] = (ItemType)i;
    return (n > 0) ? pool[rng_int(n)] : ITEM_GOLD;
}

ItemType random_item(void) {
    int total = 0;
    for (int i = 0; i < N_SPAWN_CATS; i++) total += SPAWN_WEIGHTS[i].weight;

    int r = rng_int(total);
    for (int i = 0; i < N_SPAWN_CATS; i++) {
        if (r < SPAWN_WEIGHTS[i].weight)
            return random_item_of_category(SPAWN_WEIGHTS[i].cat);
        r -= SPAWN_WEIGHTS[i].weight;
    }
    return ITEM_GOLD;
}

int gold_amount(int depth) {
    int d = depth + 1;
    return rng_range(20 + d * 10, 40 + d * 20);
}

/* ══════════════════════════════════════════════════════════
   HEAP
   ══════════════════════════════════════════════════════════ */
int heap_sprite_cell(const Heap *h, uint8_t *out_col, uint8_t *out_row) {
    int sprite;
 
    switch (h->type) {
        case HEAP_NORMAL:
        case HEAP_FOR_SALE:
            if (h->count <= 0) return 0;
            /* Identify-aware: an unidentified potion/scroll/wand/ring
               on the ground shows its shuffled alias look here too,
               not just in the backpack. */
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

/* Shared by heap_add_item/heap_add_stack below: merge into a matching
   stackable entry, or take a fresh slot. Neither rolls or resets
   anything — the two public wrappers decide whether `stack` arrives
   fresh (heap_add_item) or already-instanced (heap_add_stack). */
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
    /* Was a bare {type, quantity} field assignment, which left
       level/cursed/durability as whatever garbage sat in this heap
       slot before (heap tables are reused). Routing through
       item_stack_init means every freshly-spawned heap item — every
       spawn_item()/spawn_item_amount_heap() call below included —
       now gets a properly rolled instance for free. */
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
 