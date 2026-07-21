#pragma once
#include <SDL2/SDL.h>
#include "player.h"
#include "render.h"

/* ══════════════════════════════════════════════════════════
   INVENTORY SCREEN — a first-pass PSP-side backpack/equip UI.
   ══════════════════════════════════════════════════════════  */
typedef struct {
    int open;    /* 1 while the inventory screen should be shown/drawn */
    int cursor;  /* selected backpack slot, 0..BACKPACK_SLOTS         */
} InventoryUI;

#define CURSOR_WEAPON    20
#define CURSOR_ARMOR     21
#define CURSOR_RING1     22
#define CURSOR_RING2     23
#define CURSOR_TOTAL     24

void inventory_ui_open (InventoryUI *ui);
void inventory_ui_close(InventoryUI *ui);
 
void inventory_ui_move(InventoryUI *ui, int dx, int dy);

void render_inventory(RenderCtx *ctx, const Player *p, const InventoryUI *ui);