#pragma once
#include <SDL2/SDL.h>
#include "common.h"
#include "player.h"
#include "render.h"
#include "font.h"

/* ══════════════════════════════════════════════════════════
   INVENTORY SCREEN — a first-pass PSP-side backpack/equip UI.
   ══════════════════════════════════════════════════════════
   The original's WndBag is a touch UI (tap a slot, tap a button in a
   popup) — PSP has no touchscreen, so this is a D-pad + face-button
   grid instead: move the cursor over a slot, X to act on it.

   Deliberately takes raw SDL_Renderer and SDL_Texture pointers rather
   than your RenderCtx (render.h pulls in camera.h/assets.h/fov.h, which
   weren't part of what you've shared with me, so I couldn't compile
   against the real RenderCtx). If you'd rather call this with ctx,
   wrap it in a one-liner:
       render_inventory(ctx->renderer, ctx->items, &my_font1x, p, ui);

   Text now draws real glyphs via font.h/font.c (font1x.png) — see
   font.h for the sheet layout and the one unmapped glyph at its tail. */

typedef struct {
    int open;    /* 1 while the inventory screen should be shown/drawn */
    int cursor;  /* selected backpack slot, 0..MAX_INVENTORY-1         */
} InventoryUI;


#define CURSOR_WEAPON    20
#define CURSOR_ARMOR     21
#define CURSOR_RING1     22
#define CURSOR_RING2     23
#define CURSOR_TOTAL     24

void inventory_ui_open (InventoryUI *ui);
void inventory_ui_close(InventoryUI *ui);

/* Move the cursor by (dx,dy) across the INV_COLS x INV_ROWS grid,
   wrapping at the edges. Call from your input code on d-pad presses
   while ui->open — e.g. dx=+1 on right, dy=-1 on up. */
void inventory_ui_move(InventoryUI *ui, int dx, int dy);

/* Draws the panel: dims the screen, then the backpack grid, equip
   panel, description bar, and PSP button-color hints. Call this
   AFTER your normal world render (render_frame/render_player/etc.)
   while ui->open, so the dungeon shows dimmed behind it rather than
   being replaced by a blank screen. `font` should already be loaded
   (font_load) — typically once at startup alongside your other
   textures, not per-frame. */
void render_inventory(RenderCtx *ctx, const Player *p, const InventoryUI *ui);