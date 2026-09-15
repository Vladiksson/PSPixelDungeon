#pragma once
#include "render.h"
#include "dungeon.h"

/* ── Inventory ───────────────────────────────────────────── */
#define INV_COLS   5
#define INV_ROWS   4

#define BAG_COLS   4

#define INV_SLOT   30
#define INV_GAP     1

#define FONT_SCALING 1.15
/* derived */

#define GRID_W   (INV_COLS * INV_SLOT + (INV_COLS - 1) * INV_GAP)
#define GRID_H   (INV_ROWS * INV_SLOT + (INV_ROWS - 1) * INV_GAP)

#define PANEL_GAP (INV_SLOT / 3)
/* Main panel */
#define PANEL_W  (GRID_W + PANEL_GAP * 3 + INV_SLOT)
#define PANEL_H  (GRID_H + PANEL_GAP * 4)
#define PANEL_X ((SCREEN_W - PANEL_W) / 2)
#define PANEL_Y ((SCREEN_H - PANEL_H) / 2 - PANEL_GAP) 

#define INV_GRID_X (PANEL_X + PANEL_GAP     )
#define INV_GRID_Y (PANEL_Y + PANEL_GAP * 2 ) 

#define EQUIP_PANEL_X (INV_GRID_X + GRID_W + PANEL_GAP)
#define EQUIP_PANEL_Y INV_GRID_Y
/* Labels */
#define NAME_X (INV_GRID_X)
#define NAME_Y (INV_GRID_Y + GRID_H + INV_GAP * 2)

#define BAG_NAME_X INV_GRID_X
#define BAG_NAME_Y (INV_GRID_Y - PANEL_GAP)

/* Bag tabs */
#define BAG_TAB_SIZE  (INV_SLOT * 3 / 4)

#define BAG_TAB_GAP       INV_GAP
#define BAG_TAB_GAP_H     12//difference between unselected and selected tab's textures 8 (SCALED 12) IDK WHY

#define BAG_TAB_WIDTH           (BAG_TAB_SIZE + INV_SLOT) / 2
#define BAG_TAB_STEP            BAG_TAB_WIDTH
#define BAG_TAB_ICON_OFFSET_X   ( 6 * BAG_TAB_GAP)
#define BAG_TAB_ICON_OFFSET_Y   ( BAG_TAB_GAP)

#define BAG_TABS_X       (PANEL_X + BAG_TAB_GAP)
#define BAG_TABS_Y       (PANEL_Y + PANEL_H - BAG_TAB_GAP_H) 

/* Description panel */
#define DESC_PANEL_X (PANEL_X - PANEL_GAP / 4)
#define DESC_PANEL_Y (PANEL_Y - PANEL_GAP / 2)
#define DESC_PANEL_W (PANEL_W + PANEL_GAP / 2)
#define DESC_PANEL_H (PANEL_H + BAG_TAB_SIZE)

#define DESC_NAME_X  (DESC_PANEL_X + (DESC_PANEL_W * 9 / 32)) 
#define DESC_NAME_Y  (DESC_PANEL_Y + (PANEL_GAP * 1.5))

#define DESC_ICON_X (DESC_PANEL_X + (PANEL_GAP * 3.5))
#define DESC_ICON_Y (DESC_PANEL_Y + PANEL_GAP * 2)

#define DESC_X (DESC_PANEL_X + PANEL_GAP * 1.2)
#define DESC_Y (DESC_PANEL_Y + PANEL_GAP * 5)

#define DESC_LENGTH (DESC_PANEL_W - PANEL_GAP * 2.5)

typedef struct {
    int     pending_ring;
    int     cursor;
    BagType active_bag;
} InventoryUI;

#define CURSOR_WEAPON    20
#define CURSOR_ARMOR     21
#define CURSOR_RING1     22
#define CURSOR_RING2     23
#define CURSOR_TOTAL     24

void inventory_ui_desc_open(InventoryUI *ui);
void inventory_ui_desc_close(InventoryUI *ui);
 
void inventory_ui_move(InventoryUI *ui, int dx, int dy);

void inventory_ui_next_tab(InventoryUI *ui, const Player *p);
void inventory_ui_prev_tab(InventoryUI *ui, const Player *p);

int inventory_ui_try_open_bag(InventoryUI *ui, const Player *p);

void render_item_description(RenderCtx *ctx, const InventoryUI *ui, const Player *p);
void render_inventory(RenderCtx *ctx, const Player *p, const InventoryUI *ui);

void render_stats_screen(RenderCtx *ctx, const Player *p);

void render_hud(RenderCtx *ctx, const Player *p, const Dungeon *d);
void hud_update(RenderCtx *ctx, const Player *p);