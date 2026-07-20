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

#define CONTENT_W (GRID_W + PANEL_GAP + EQUIP_W)

#define INV_GRID_X ((SCREEN_W - CONTENT_W) / 2)
#define INV_GRID_Y 36

#define EQUIP_PANEL_X (INV_GRID_X + GRID_W + PANEL_GAP)
#define EQUIP_PANEL_Y INV_GRID_Y

#define DESC_BAR_Y (SCREEN_H - 28)


/* ── Palette  ──────────── */
//static const SDL_Color COL_BG          = { 0x14, 0x10, 0x0c, 0xE6 };
//static const SDL_Color COL_PANEL       = { 0x2a, 0x22, 0x18, 0xFF };
static const SDL_Color COL_SLOT        = { 0x3a, 0x30, 0x24, 0xFF };
//static const SDL_Color COL_SLOT_BORDER = { 0x59, 0x4a, 0x38, 0xFF };
static const SDL_Color COL_CURSOR      = { 0xe8, 0xc5, 0x47, 0xFF }; /* gold  */
//static const SDL_Color COL_EQUIPPED    = { 0x6a, 0x8f, 0x5a, 0xFF }; /* green */
static const SDL_Color COL_TEXT        = { 0xd8, 0xcd, 0xb8, 0xFF }; /* parchment */

static const SDL_Color SLOT_NORMAL = { 0x4A,0x4D,0x44,255 };
static const SDL_Color SLOT_EQUIPPED ={ 0x63,0x66,0x5B,255 };
//static const SDL_Color SLOT_CURSED ={0x7A,0x45,0x45,255};
//static const SDL_Color SLOT_UNKNOWN ={0x45,0x50,0x70,255};




void inventory_ui_open (InventoryUI *ui) { ui->open = 1; ui->cursor = 0; }
void inventory_ui_close(InventoryUI *ui) { ui->open = 0; }

void inventory_ui_move(InventoryUI *ui, int dx, int dy) {
    if (ui->cursor < MAX_INVENTORY) {

        int col = ui->cursor % INV_COLS;
        int row = ui->cursor / INV_COLS;

        if (dx > 0 && col == INV_COLS - 1) {
            switch (row) {
                case 0:
                    ui->cursor = CURSOR_WEAPON;
                    break;
                case 1:
                    ui->cursor = CURSOR_ARMOR;
                    break;
                case 2:
                    ui->cursor = CURSOR_RING1;
                    break;
                case 3:
                    ui->cursor = CURSOR_RING2;
                    break;
            }
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
        if (dy < 0) {
            if (ui->cursor > CURSOR_WEAPON)
                ui->cursor--;
        }
        if (dy > 0) {
            if (ui->cursor < CURSOR_RING2)
                ui->cursor++;
        }
        if (dx < 0) {
            int row;
            switch (ui->cursor) {
                case CURSOR_WEAPON:
                    row = 0;
                    break;
                case CURSOR_ARMOR:
                    row = 1;
                    break;
                case CURSOR_RING1:
                    row = 2;
                    break;
                case CURSOR_RING2:
                    row = 3;
                    break;
                default:
                    row = 0;
                    break;
            }
            ui->cursor = row * INV_COLS + (INV_COLS - 1);
        }
    }
}

static void fill_rect_inventory(SDL_Renderer *r, int x, int y, int w, int h, SDL_Color c) {
    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
    SDL_Rect rect = { x, y, w, h };
    SDL_RenderFillRect(r, &rect);
}

static void outline_rect(SDL_Renderer *r, int x, int y, int w, int h, int thickness, SDL_Color c) {
    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
    for (int i = 0; i < thickness; i++) {
        SDL_Rect rect = { x - i, y - i, w + 2 * i, h + 2 * i };
        SDL_RenderDrawRect(r, &rect);
    }
}

/* Tints the next font_draw_text call. Checked the actual pixel data:
   font1x.png glyphs are white fill with a black outline, both fully
   opaque (not anti-aliased via alpha) — SDL_SetTextureColorMod
   multiplies each channel, so white -> your tint color while the
   black outline stays black (0 * anything = 0), keeping glyphs
   readable over any background regardless of tint. */
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

static void draw_item_icon(RenderCtx *ctx, ItemType type, int x, int y, int size) {
    if (type == ITEM_NONE) return;
    SDL_Rect src = { ITEM_DB[type].sprite_col * 16, ITEM_DB[type].sprite_row * 16, 16, 16 };
    int icon_size = 16;
    SDL_Rect dst = {x + (size - icon_size) / 2, y + (size - icon_size) / 2, icon_size, icon_size};//SDL_Rect dst = { x, y, size, size };
    SDL_RenderCopy(ctx->renderer, ctx->items, &src, &dst);
}

static void draw_slot(RenderCtx *ctx, int x, int y, int size, ItemStack stack, int selected) {
    fill_rect_inventory(ctx->renderer, x, y, size, size, SLOT_NORMAL);
    
    if (selected) {
        fill_rect_inventory(ctx->renderer, x, y, size, size, SLOT_EQUIPPED);
        //outline_rect(ctx->renderer, x, y, size, size, selected ? 2 : 1, selected ? COL_CURSOR : COL_SLOT_BORDER);
    }
    if (stack.type == ITEM_GOLD) {
        fill_rect_inventory(ctx->renderer, x, y, size, size, COL_SLOT);
    }
    if (stack.type == ITEM_NONE || stack.quantity < 0)
        return;

    int pad = size / 8;

    draw_item_icon(ctx, stack.type, x + pad, y + pad, size - pad * 2);

    if (ITEM_DB[stack.type].stackable && stack.quantity > 1) {
        char qty[16];
        snprintf(qty,sizeof(qty),"%d",stack.quantity);
        int tw = font_text_width(qty);
        draw_text(ctx, x + size - tw - 2, y + size - 9, COL_CURSOR, qty);
    }
}

static void draw_equip_slot(RenderCtx *ctx, int x, int y, int size, ItemType type, const char *label, int selected) {
    fill_rect_inventory(ctx->renderer, x, y, size, size, SLOT_NORMAL);
    if (selected) {
        fill_rect_inventory(ctx->renderer, x, y, size, size, SLOT_EQUIPPED);
        //outline_rect(ctx->renderer, x, y, size, size, selected ? 2 : 1, selected ? COL_CURSOR : COL_SLOT_BORDER);
    }

    if (type != ITEM_NONE) {
        int pad = size / 8;
        draw_item_icon(ctx, type, x + pad, y + pad, size - pad * 2);
    }
    draw_text(ctx, x + size + 6, y + size / 2 - 4, COL_TEXT, label);
}

void render_inventory(RenderCtx *ctx, const Player *p, const InventoryUI *ui) {
    if (!ui->open) return;
    SDL_SetRenderDrawBlendMode(ctx->renderer, SDL_BLENDMODE_BLEND);

    NinePatch tab_set = chrome_tab_set(ctx->chrome);
    ninepatch_draw(ctx->renderer, &tab_set, 80, 16, SCREEN_W - 160, SCREEN_H - 80);

    draw_text(ctx, 160, 26, COL_TEXT, "EBAnuY RoT SRANOGO OLEGA");

    /* Backpack grid — always draws all 20 cells so empty slots show
       as empty boxes, not gaps. */
    for (int i = 0; i < MAX_INVENTORY; i++) {
        int col = i % INV_COLS;
        int row = i / INV_COLS;
        int x = INV_GRID_X + col * (INV_SLOT + INV_GAP);
        int y = INV_GRID_Y + row * (INV_SLOT + INV_GAP);

        ItemStack stack = { ITEM_NONE, 0 };
        if (i == MAX_INVENTORY - 1) {
            stack.type = ITEM_GOLD;
            stack.quantity = p->gold;
        } else if (i < p->num_inventory) {
            stack = p->inventory[i];
        }
        draw_slot(ctx, x, y, INV_SLOT, stack, i == ui->cursor);
    }

    /* Equip panel */
    int ex = EQUIP_PANEL_X;
    draw_equip_slot(ctx, ex, EQUIP_PANEL_Y + 0 * (EQUIP_SLOT + EQUIP_GAP), EQUIP_SLOT, p->weapon, "Weapon", ui->cursor == CURSOR_WEAPON);
    draw_equip_slot(ctx, ex, EQUIP_PANEL_Y + 1 * (EQUIP_SLOT + EQUIP_GAP), EQUIP_SLOT, p->armor,  "Armor",  ui->cursor == CURSOR_ARMOR );
    draw_equip_slot(ctx, ex, EQUIP_PANEL_Y + 2 * (EQUIP_SLOT + EQUIP_GAP), EQUIP_SLOT, p->ring1,  "Ring",   ui->cursor == CURSOR_RING1 );
    draw_equip_slot(ctx, ex, EQUIP_PANEL_Y + 3 * (EQUIP_SLOT + EQUIP_GAP), EQUIP_SLOT, p->ring2,  "Ring",   ui->cursor == CURSOR_RING2 );

    
    ItemType type = ITEM_NONE;
    int quantity = 1;

    if (ui->cursor < MAX_INVENTORY) {
        if (ui->cursor == MAX_INVENTORY - 1) {
            type = ITEM_GOLD;
            quantity = p->gold;
        }
        else if (ui->cursor < p->num_inventory) {
            type = p->inventory[ui->cursor].type;
            quantity = p->inventory[ui->cursor].quantity;
        }
    }
    else {
        switch (ui->cursor) {
            case CURSOR_WEAPON: type = p->weapon;
                break;
            case CURSOR_ARMOR: type = p->armor;
                break;
            case CURSOR_RING1: type = p->ring1;
                break;
            case CURSOR_RING2: type = p->ring2;
                break;
        }
    }
    if (type != ITEM_NONE) {
        char name_buf[64];
        if (ITEM_DB[type].stackable && quantity > 1)
            snprintf(name_buf, sizeof(name_buf), "%s x%d", ITEM_DB[type].name, quantity);
        else
            snprintf(name_buf, sizeof(name_buf), "%s", ITEM_DB[type].name);
        draw_text(ctx, 100, SCREEN_H - 110, COL_TEXT, name_buf);
    }
}