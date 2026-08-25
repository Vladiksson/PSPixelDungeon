#pragma once
//#include "player.h"
#include "render.h"
//#include "bag.h"

/* ══════════════════════════════════════════════════════════
   INVENTORY SCREEN backpack/equip UI.
   ══════════════════════════════════════════════════════════  */
typedef struct {
    int     open;
    int     desc_open;   //TODO temporar solution, expand for more popups
    int     pending_ring;
    int     cursor;
    BagType active_bag;
} InventoryUI;

#define CURSOR_WEAPON    20
#define CURSOR_ARMOR     21
#define CURSOR_RING1     22
#define CURSOR_RING2     23
#define CURSOR_TOTAL     24

void inventory_ui_open (InventoryUI *ui);
void inventory_ui_close(InventoryUI *ui);

void inventory_ui_desc_open(InventoryUI *ui);
void inventory_ui_desc_close(InventoryUI *ui);
 
void inventory_ui_move(InventoryUI *ui, int dx, int dy);

void inventory_ui_next_tab(InventoryUI *ui, const Player *p);
void inventory_ui_prev_tab(InventoryUI *ui, const Player *p);

int inventory_ui_try_open_bag(InventoryUI *ui, const Player *p);

void render_inventory(RenderCtx *ctx, const Player *p, const InventoryUI *ui);