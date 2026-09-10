#include "ui_inventory.h"
#include "ui_ninepatch.h"
#include "icons.h"
#include "common.h"

/* ══════════════════════════════════════════════════════════
   UI state
   ══════════════════════════════════════════════════════════  */
void inventory_ui_open (InventoryUI *ui) { ui->open = 1; ui->cursor = 0; ui->active_bag = BAG_BACKPACK; ui->pending_ring = -1; }
void inventory_ui_close(InventoryUI *ui) { ui->open = 0; }

void inventory_ui_desc_open (InventoryUI *ui) { ui->desc_open = 1; }
void inventory_ui_desc_close(InventoryUI *ui) { ui->desc_open = 0; }

void inventory_ui_move(InventoryUI *ui, int dx, int dy) {
    int backpack_tab = (ui->active_bag == BAG_BACKPACK);
    int cols       = backpack_tab ? INV_COLS : BAG_COLS;
    int rows       = backpack_tab ? INV_ROWS : bag_capacity_for_type(ui->active_bag) / cols;
    int slot_count = backpack_tab ? MAX_INVENTORY : bag_capacity_for_type(ui->active_bag);
 
    if (ui->cursor < slot_count) {
        int col = ui->cursor % cols;
        int row = ui->cursor / cols;
        if (dx > 0 && col == cols - 1) {
            int target = CURSOR_WEAPON + row;
            ui->cursor = (target > CURSOR_RING2) ? CURSOR_RING2 : target;
            return;
        }
        col += dx;
        row += dy;
        if (col < 0)
            col = 0;
        if (col >= cols)
            col = cols - 1;
        if (row < 0)
            row = 0;
        if (row >= rows)
            row = rows - 1;
        ui->cursor = row * cols + col;
    } else {
        if (dy < 0 && ui->cursor > CURSOR_WEAPON)
            ui->cursor--;
        if (dy > 0 && ui->cursor < CURSOR_RING2)
            ui->cursor++;
        if (dx < 0) {
            int row = ui->cursor - CURSOR_WEAPON;
            if (row >= rows)
                row = rows - 1;
            ui->cursor = row * cols + (cols - 1);
        }
    }
}
 
void inventory_ui_next_tab(InventoryUI *ui, const Player *p) {
    int t = ui->active_bag;
    for (int step = 0; step < BAG_TYPE_COUNT; step++) {
        t = (t + 1) % BAG_TYPE_COUNT;
        if (player_has_bag(p, (BagType)t)) {
            ui->active_bag = (BagType)t;
            ui->cursor = 0;
            return;
        }
    }
}
 
void inventory_ui_prev_tab(InventoryUI *ui, const Player *p) {
    int t = ui->active_bag;
    for (int step = 0; step < BAG_TYPE_COUNT; step++) {
        t = (t - 1 + BAG_TYPE_COUNT) % BAG_TYPE_COUNT;
        if (player_has_bag(p, (BagType)t)) {
            ui->active_bag = (BagType)t;
            ui->cursor = 0;
            return;
        }
    }
}
 
int inventory_ui_try_open_bag(InventoryUI *ui, const Player *p) {
    if (ui->active_bag != BAG_BACKPACK) return 0;
 
    const Bag *backpack = &p->bags[BAG_BACKPACK];
    if (ui->cursor < 0 || ui->cursor >= backpack->count) return 0;
 
    int type = bag_type_for_item(backpack->items[ui->cursor].type);
    if (type < 0) return 0;
 
    ui->active_bag = (BagType)type;
    ui->cursor = 0;
    return 1;
}

/* ══════════════════════════════════════════════════════════
   drawing helpers
   ══════════════════════════════════════════════════════════  */
static void fill_rect_inventory(SDL_Renderer *r, int x, int y, int w, int h, SDL_Color c) {
    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
    SDL_Rect rect = { x, y, w, h };
    SDL_RenderFillRect(r, &rect);
}

static void draw_text(RenderCtx *ctx, int x, int y, SDL_Color color, const char *text){
    font_draw_text(ctx->renderer, &ctx->font, x, y, FONT_SCALING, color, text);
}

static void draw_wrapped_text(RenderCtx *ctx, int x, int y, SDL_Color color, const char *text, int max_width) {
    char line[256] = "";
    char word[128];
    while (*text) {
        if (*text == '\n') {
            if (line[0]) {
                draw_text(ctx, x, y, color, line);
                y += 10;
                line[0] = '\0';
            }
            text++;
            continue;
        }
        while (*text == ' ')
            text++;
        if (!*text)
            break;
        int len = 0;
        while (*text && *text != ' ' && *text != '\n') {
            if (len < (int)sizeof(word) - 1)
                word[len++] = *text;
            text++;
        }
        word[len] = '\0';

        char test[256];
        if (line[0])
            snprintf(test, sizeof(test), "%s %s", line, word);
        else
            snprintf(test, sizeof(test), "%s", word);
        if (line[0] && font_text_width(test) > max_width) {
            draw_text(ctx, x, y, color, line);
            y += 10;
            snprintf(line, sizeof(line), "%s", word);
        } else {
            snprintf(line, sizeof(line), "%s", test);
        }
    }
    if (line[0])
        draw_text(ctx, x, y, color, line);
}

static void draw_icon(RenderCtx *ctx, IconId icon, int x, int y, int size) {
    uint8_t sx, sy, sw, sh;
    icon_sprite(icon, &sx, &sy, &sw, &sh);

    SDL_Rect src = { sx, sy, sw, sh };
    SDL_Rect dst = { x + (size - sw) / 2, y + (size - sh) / 2, sw, sh };
    SDL_RenderCopy(ctx->renderer, ctx->icons, &src, &dst);
}

/* ══════════════════════════════════════════════════════════
   Item rendering
   ══════════════════════════════════════════════════════════  */
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
        draw_text(ctx, x + size - lw - 2, y + 1, COL_GOLD_TEXT, lvl);
    }
    if (ITEM_DB[stack.type].stackable && stack.quantity > 1) {
        char qty[16];
        snprintf(qty, sizeof(qty), "%d", stack.quantity);
        int tw = font_text_width(qty);
        draw_text(ctx, x + size - tw - 2, y + size - 8, COL_GOLD_TEXT, qty);
    }
}

/* ══════════════════════════════════════════════════════════
   slot rendering
   ══════════════════════════════════════════════════════════  */
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

/* ══════════════════════════════════════════════════════════
   bag tabs
   ══════════════════════════════════════════════════════════  */
static void bag_tab_icon_pos(int t, int *x, int *y) {
    *x = BAG_TABS_X + BAG_TAB_ICON_OFFSET_X + t * BAG_TAB_STEP;
    *y = BAG_TABS_Y + BAG_TAB_ICON_OFFSET_Y;
}
static void bag_tab_pos(int t, int *x, int *y) {
    *x = BAG_TABS_X + BAG_TAB_GAP + t * BAG_TAB_STEP;
    *y = BAG_TABS_Y + BAG_TAB_GAP;
}

static void draw_unselected_tab(RenderCtx *ctx, const Player *p, BagType active) {
    for (int t = 0; t < BAG_TYPE_COUNT; ++t) {
        if (!player_has_bag(p, t) || t == active)
            continue;

        int x, y;
        bag_tab_pos(t, &x, &y);
        NinePatch tab_set = chrome_tab_unselected(ctx->ui.chrome);
        ninepatch_draw(ctx->renderer, &tab_set, x, y, BAG_TAB_WIDTH, BAG_TAB_SIZE);

        bag_tab_icon_pos(t, &x, &y);
        draw_icon(ctx, bag_icon((BagType)t), x, y, ICON_SIZE);
    }
}

static void draw_selected_tab(RenderCtx *ctx, const Player *p, BagType active) {   
    int t = active;
    int x, y;
    bag_tab_pos(t, &x, &y);

    NinePatch tab_set = chrome_tab_selected(ctx->ui.chrome);
    ninepatch_draw(ctx->renderer, &tab_set, x, y, BAG_TAB_WIDTH, BAG_TAB_SIZE);

    bag_tab_icon_pos(t, &x, &y);
    draw_icon(ctx, bag_icon(active), x, y, ICON_SIZE);
}

/* ══════════════════════════════════════════════════════════
   Description screen
   ══════════════════════════════════════════════════════════  */
static void render_item_description(RenderCtx *ctx, ItemStack stack) {
    NinePatch window = chrome_window(ctx->ui.chrome);
    ninepatch_draw(ctx->renderer, &window, DESC_PANEL_X, DESC_PANEL_Y, DESC_PANEL_W, DESC_PANEL_H);
    
    draw_item_icon(ctx, stack, DESC_ICON_X, DESC_ICON_Y, ICON_SIZE);

    char name[64];
    item_display_name(&stack, name, sizeof(name));

    draw_text(ctx, DESC_NAME_X, DESC_NAME_Y, COL_GOLD_TEXT, name);
    draw_wrapped_text(ctx, DESC_X, DESC_Y, COL_TEXT, item_display_desc(&stack), DESC_LENGTH);
}

/* ══════════════════════════════════════════════════════════
   Inventory screen
   ══════════════════════════════════════════════════════════  */
void render_inventory(RenderCtx *ctx, const Player *p, const InventoryUI *ui) {
    if (!ui->open) return;
    SDL_SetRenderDrawBlendMode(ctx->renderer, SDL_BLENDMODE_BLEND);

    BagType shown = (ui->active_bag == BAG_BACKPACK || player_has_bag(p, ui->active_bag)) ? ui->active_bag : BAG_BACKPACK;
 
    int backpack_tab = (shown == BAG_BACKPACK);
    int grid_cols     = backpack_tab ? INV_COLS : BAG_COLS;
    int slot_count    = backpack_tab ? MAX_INVENTORY : bag_capacity_for_type(shown);
    const Bag *bag    = &p->bags[shown];
    
    draw_unselected_tab(ctx, p, shown);

    NinePatch tab_set = chrome_tab_set(ctx->ui.chrome);
    ninepatch_draw(ctx->renderer, &tab_set, PANEL_X, PANEL_Y, PANEL_W, PANEL_H);

    draw_selected_tab(ctx, p, shown);
    
    draw_text(ctx, BAG_NAME_X, BAG_NAME_Y, COL_TEXT, bag_name(shown));

    for (int i = 0; i < slot_count; i++) {
        int col = i % grid_cols;
        int row = i / grid_cols;
        int x = INV_GRID_X + col * (INV_SLOT + INV_GAP);
        int y = INV_GRID_Y + row * (INV_SLOT + INV_GAP);

        ItemStack stack = item_stack_none();
        if (backpack_tab && i == GOLD_SLOT) {
            stack.type = ITEM_GOLD;
            stack.quantity = p->gold;
        } else if (i < bag->count) {
            stack = bag->items[i];
        }
        draw_slot(ctx, x, y, INV_SLOT, stack, i == ui->cursor);
    }

    draw_equip_slot(ctx, EQUIP_PANEL_X, EQUIP_PANEL_Y + 0 * (INV_SLOT + INV_GAP), INV_SLOT, p->weapon, SPRITE_WEAPON, ui->cursor == CURSOR_WEAPON);
    draw_equip_slot(ctx, EQUIP_PANEL_X, EQUIP_PANEL_Y + 1 * (INV_SLOT + INV_GAP), INV_SLOT, p->armor,  SPRITE_ARMOR,  ui->cursor == CURSOR_ARMOR );
    draw_equip_slot(ctx, EQUIP_PANEL_X, EQUIP_PANEL_Y + 2 * (INV_SLOT + INV_GAP), INV_SLOT, p->ring1,  SPRITE_RING,   ui->cursor == CURSOR_RING1 );
    draw_equip_slot(ctx, EQUIP_PANEL_X, EQUIP_PANEL_Y + 3 * (INV_SLOT + INV_GAP), INV_SLOT, p->ring2,  SPRITE_RING,   ui->cursor == CURSOR_RING2 );
    
    ItemStack stack = item_stack_none();
 
    if (ui->cursor < slot_count) {
        if (backpack_tab && ui->cursor == GOLD_SLOT ) {
            stack.type = ITEM_GOLD;
            stack.quantity = p->gold;
        } else if (ui->cursor < bag->count) {
            stack = bag->items[ui->cursor];
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

        draw_text(ctx, NAME_X, NAME_Y, COL_TEXT, name_buf);
        if (ui->desc_open)
            render_item_description(ctx, stack);
    }
    SDL_SetRenderDrawBlendMode(ctx->renderer, SDL_BLENDMODE_NONE);
}