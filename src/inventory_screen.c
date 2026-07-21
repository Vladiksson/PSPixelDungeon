#include "inventory_screen.h"
#include "chrome.h"
#include <stdio.h>

/* ── Layout ───────────────────────────────────────────── */
#define INV_COLS   5
#define INV_ROWS   4

#define INV_SLOT   28
#define INV_GAP     1

#define EQUIP_SLOT INV_SLOT
#define EQUIP_GAP  INV_GAP

#define FRAME_PAD      16
#define TITLE_Y        22
#define GOLD_MARGIN    18
#define BOTTOM_MARGIN  12

/* derived */

#define GRID_W   (INV_COLS * INV_SLOT + (INV_COLS - 1) * INV_GAP)
#define GRID_H   (INV_ROWS * INV_SLOT + (INV_ROWS - 1) * INV_GAP)

#define EQUIP_W  (EQUIP_SLOT)
#define EQUIP_H  (4 * EQUIP_SLOT + 3 * EQUIP_GAP)

#define PANEL_GAP 12

#define INV_GRID_X ((SCREEN_W - GRID_W - PANEL_GAP - EQUIP_W) / 2)
#define INV_GRID_Y 36

#define EQUIP_PANEL_X (INV_GRID_X + GRID_W + PANEL_GAP)
#define EQUIP_PANEL_Y INV_GRID_Y

#define ICON_SIZE 16

/* ── Palette  ──────────── */
static const SDL_Color COL_CURSOR           = { 0xe8, 0xc5, 0x47, 0xFF }; /* for gold text */
static const SDL_Color COL_TEXT             = { 0xd8, 0xcd, 0xb8, 0xFF }; 

static const SDL_Color COL_SLOT_NORMAL      = { 0x4A, 0x4D, 0x44, 0xFF };
static const SDL_Color COL_SLOT_SELECTED    = { 0x78, 0x77, 0x71, 0xFF };
static const SDL_Color COL_SLOT_EQUIP       = { 0x63, 0x66, 0x5B, 0xFF };
static const SDL_Color COL_SLOT_CURSED      = { 0x7A, 0x45, 0x45, 0xFF };
static const SDL_Color COL_SLOT_UNKNOWN     = { 0x40, 0x46, 0x5B, 0xFF };
static const SDL_Color COL_SLOT_GOLD        = { 0x3a, 0x30, 0x24, 0xFF };


void inventory_ui_open (InventoryUI *ui) { ui->open = 1; ui->cursor = 0; }
void inventory_ui_close(InventoryUI *ui) { ui->open = 0; }

void inventory_ui_move(InventoryUI *ui, int dx, int dy) {
    if (ui->cursor < MAX_INVENTORY) {
        int col = ui->cursor % INV_COLS;
        int row = ui->cursor / INV_COLS;
        if (dx > 0 && col == INV_COLS - 1) {
            ui->cursor = CURSOR_WEAPON + row;
            return;
        }
        col += dx;
        row += dy;
        if (col < 0)
            col = INV_COLS - 1;
        if (col >= INV_COLS)
            col = 0;
        if (row < 0)
            row = INV_ROWS - 1;
        if (row >= INV_ROWS)
            row = 0;
        ui->cursor = row * INV_COLS + col;
    } else {
        if (dy < 0 && ui->cursor > CURSOR_WEAPON)
            ui->cursor--;
        if (dy > 0 && ui->cursor < CURSOR_RING2)
            ui->cursor++;
        if (dx < 0) {
            int row = ui->cursor - CURSOR_WEAPON;
            ui->cursor = row * INV_COLS + (INV_COLS - 1);
        }
    }
}

static void fill_rect_inventory(SDL_Renderer *r, int x, int y, int w, int h, SDL_Color c) {
    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
    SDL_Rect rect = { x, y, w, h };
    SDL_RenderFillRect(r, &rect);
}

static void draw_text(RenderCtx *ctx, int x, int y, SDL_Color c, const char *s) {
    const BitmapFont *font = &ctx->font;
    for (int i = 0; i < 3; i++) {
        if (font->tex[i]) {
            SDL_SetTextureColorMod(font->tex[i], c.r, c.g, c.b);
            SDL_SetTextureAlphaMod(font->tex[i], 255);
        }
    }
    font_draw_text(ctx->renderer, font, x, y, 1, s);
}

static void draw_item_icon(RenderCtx *ctx, ItemStack stack, int x, int y, int size) {
    if (stack.type == ITEM_NONE)
        return;

    uint8_t col, row;
    item_display_sprite(stack.type, &col, &row);

    SDL_Rect src = { col * ICON_SIZE, row * ICON_SIZE, ICON_SIZE, ICON_SIZE };
    SDL_Rect dst = { x + (size - ICON_SIZE) / 2, y + (size - ICON_SIZE) / 2, ICON_SIZE, ICON_SIZE };
    SDL_RenderCopy(ctx->renderer, ctx->items, &src, &dst);
}

static void draw_placeholder_icon(RenderCtx *ctx, int sprite, int x, int y, int size) {
    uint8_t col = sprite % ITEMS_SHEET_COLS;
    uint8_t row = sprite / ITEMS_SHEET_COLS;

    SDL_Rect src = { col * ICON_SIZE, row * ICON_SIZE, ICON_SIZE, ICON_SIZE };

    SDL_SetTextureColorMod(ctx->items, 140, 140, 140);
    SDL_SetTextureAlphaMod(ctx->items, 90);

    SDL_Rect dst = { x + (size - ICON_SIZE) / 2, y + (size - ICON_SIZE) / 2, ICON_SIZE, ICON_SIZE };

    SDL_RenderCopy(ctx->renderer, ctx->items, &src, &dst);

    SDL_SetTextureColorMod(ctx->items, 255,255,255);
    SDL_SetTextureAlphaMod(ctx->items, 255);
}

static void draw_item_contents(RenderCtx *ctx, ItemStack stack, int x, int y, int size) {
    if (stack.type == ITEM_NONE)
        return;
    
    int pad = size / 8;
    draw_item_icon(ctx, stack, x + pad, y + pad, size - pad * 2);
    
    if (stack.level_known && stack.level != 0) {
        char lvl[8];
        snprintf(lvl, sizeof(lvl), "%+d", stack.level);
        int lw = font_text_width(lvl);
        draw_text(ctx, x + size - lw - 2, y + 1, COL_CURSOR, lvl);
    }
    if (ITEM_DB[stack.type].stackable && stack.quantity > 1) {
        char qty[16];
        snprintf(qty, sizeof(qty), "%d", stack.quantity);
        int tw = font_text_width(qty);
        draw_text(ctx, x + size - tw - 2, y + size - 8, COL_CURSOR, qty);
    }
}

static void draw_slot_color(RenderCtx *ctx, int x, int y, int size, ItemStack stack, int selected, int equip_slot) {
    SDL_Color color = COL_SLOT_NORMAL;
    if (equip_slot)
        color = COL_SLOT_EQUIP;
    if (item_visibly_cursed(&stack))
        color = COL_SLOT_CURSED;
    if (!item_status_is_known(stack.type))
        color = COL_SLOT_UNKNOWN;
    if (stack.type == ITEM_GOLD)
        color = COL_SLOT_GOLD;
    if (selected)
        color = COL_SLOT_SELECTED;
    fill_rect_inventory(ctx->renderer, x, y, size, size, color);
}


static void draw_slot(RenderCtx *ctx, int x, int y, int size, ItemStack stack, int selected) {
    draw_slot_color(ctx, x, y, size, stack, selected, 0);
    draw_item_contents(ctx, stack, x, y, size);
}

static void draw_equip_slot(RenderCtx *ctx, int x, int y, int size, ItemStack stack, int placeholder, int selected) {
    draw_slot_color(ctx, x, y, size, stack, selected, 1);
    if (stack.type != ITEM_NONE) {
        draw_item_contents(ctx, stack, x, y, size);
    } else {
        draw_placeholder_icon(ctx, placeholder, x, y, size);
    }
}

void render_inventory(RenderCtx *ctx, const Player *p, const InventoryUI *ui) {
    if (!ui->open) return;
    SDL_SetRenderDrawBlendMode(ctx->renderer, SDL_BLENDMODE_BLEND);

    NinePatch tab_set = chrome_tab_set(ctx->chrome);
    ninepatch_draw(ctx->renderer, &tab_set, 120, 16, SCREEN_W - 240, SCREEN_H - 80);

    draw_text(ctx, INV_GRID_X, INV_GRID_Y - PANEL_GAP, COL_TEXT, "EBAnuY RoT SRANOGO OLEGA");

    /* Backpack grid */
    for (int i = 0; i < MAX_INVENTORY; i++) {
        int col = i % INV_COLS;
        int row = i / INV_COLS;
        int x = INV_GRID_X + col * (INV_SLOT + INV_GAP);
        int y = INV_GRID_Y + row * (INV_SLOT + INV_GAP);

        ItemStack stack = item_stack_none();
        if (i == GOLD_SLOT) {
            stack.type = ITEM_GOLD;
            stack.quantity = p->gold;
        } else if (i < p->num_inventory) {
            stack = p->inventory[i];
        }
        draw_slot(ctx, x, y, INV_SLOT, stack, i == ui->cursor);
    }

    /* Equip panel */
    draw_equip_slot(ctx, EQUIP_PANEL_X, EQUIP_PANEL_Y + 0 * (EQUIP_SLOT + EQUIP_GAP), EQUIP_SLOT, p->weapon, SPRITE_WEAPON, ui->cursor == CURSOR_WEAPON);
    draw_equip_slot(ctx, EQUIP_PANEL_X, EQUIP_PANEL_Y + 1 * (EQUIP_SLOT + EQUIP_GAP), EQUIP_SLOT, p->armor,  SPRITE_ARMOR,  ui->cursor == CURSOR_ARMOR );
    draw_equip_slot(ctx, EQUIP_PANEL_X, EQUIP_PANEL_Y + 2 * (EQUIP_SLOT + EQUIP_GAP), EQUIP_SLOT, p->ring1,  SPRITE_RING,   ui->cursor == CURSOR_RING1 );
    draw_equip_slot(ctx, EQUIP_PANEL_X, EQUIP_PANEL_Y + 3 * (EQUIP_SLOT + EQUIP_GAP), EQUIP_SLOT, p->ring2,  SPRITE_RING,   ui->cursor == CURSOR_RING2 );

    
    ItemStack stack = item_stack_none();

    if (ui->cursor < MAX_INVENTORY) {
        if (ui->cursor == GOLD_SLOT ) {
            stack.type = ITEM_GOLD;
            stack.quantity = p->gold;
        } else if (ui->cursor < p->num_inventory) {
            stack = p->inventory[ui->cursor];
        }
    } else {
        switch (ui->cursor) {
            case CURSOR_WEAPON: stack = p->weapon; break;
            case CURSOR_ARMOR:  stack = p->armor;  break;
            case CURSOR_RING1:  stack = p->ring1;  break;
            case CURSOR_RING2:  stack = p->ring2;  break;
        }
    }

    if (stack.type != ITEM_NONE) {
        char name_buf[64];
        char item_name[40];
        item_display_name(&stack, item_name, sizeof(item_name));

        if (ITEM_DB[stack.type].stackable && stack.quantity > 1)
            snprintf(name_buf, sizeof(name_buf), "%s x %d", item_name, stack.quantity);
        else
            snprintf(name_buf, sizeof(name_buf), "%s", item_name);

        draw_text(ctx, INV_GRID_X, SCREEN_H - EQUIP_H, COL_TEXT, name_buf);
    }
}