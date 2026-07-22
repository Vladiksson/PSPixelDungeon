#pragma once
#include <SDL2/SDL.h>
#include "player.h"
#include "render.h"
#include "bag.h"

/* ══════════════════════════════════════════════════════════
   INVENTORY SCREEN — a first-pass PSP-side backpack/equip UI.
   ══════════════════════════════════════════════════════════  */
typedef struct {
    int     open;        /* 1 while the inventory screen should be shown/drawn */
    int     cursor;      /* selected slot within the active bag's grid, or a CURSOR_* equip sentinel */
    BagType active_bag;  /* which tab is showing — BAG_BACKPACK or an owned pouch */
} InventoryUI;

#define CURSOR_WEAPON    20
#define CURSOR_ARMOR     21
#define CURSOR_RING1     22
#define CURSOR_RING2     23
#define CURSOR_TOTAL     24

void inventory_ui_open (InventoryUI *ui);
void inventory_ui_close(InventoryUI *ui);
 
void inventory_ui_move(InventoryUI *ui, int dx, int dy);

/* Cycle active_bag forward/backward through every bag the player
   owns (the backpack is always included), skipping ones they don't
   have yet — mirrors WndBag only adding a tab for a non-null bag.
   Resets the cursor, since each tab has its own grid size. */
void inventory_ui_next_tab(InventoryUI *ui, const Player *p);
void inventory_ui_prev_tab(InventoryUI *ui, const Player *p);

void render_inventory(RenderCtx *ctx, const Player *p, const InventoryUI *ui);