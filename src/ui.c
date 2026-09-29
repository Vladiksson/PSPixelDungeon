#include "ui.h"
#include "ui_ninepatch.h"
#include "icons.h"
#include "common.h"
#include "player.h"

/* ══════════════════════════════════════════════════════════
   UI state
   ══════════════════════════════════════════════════════════  */
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
   INVENTORY HELPER
   ══════════════════════════════════════════════════════════  */
static EquipSlot equip_slot_from_cursor(int cursor) {
    if (cursor < CURSOR_WEAPON || cursor >= CURSOR_TOTAL)
        return EQUIP_SLOT_COUNT;
    return (EquipSlot)(cursor - CURSOR_WEAPON);
}

static ItemStack inventory_get_stack(const Player *p, BagType shown, int cursor, int slot_count) {
    if (cursor < slot_count) {
        if (shown == BAG_BACKPACK && cursor == GOLD_SLOT) {
            ItemStack stack = item_stack_none();
            stack.type = ITEM_GOLD;
            stack.quantity = p->gold;
            return stack;
        }

        const Bag *bag = &p->bags[shown];

        if (cursor < bag->count)
            return bag->items[cursor];
        return item_stack_none();
    }
    EquipSlot slot = equip_slot_from_cursor(cursor);
    if (slot != EQUIP_SLOT_COUNT) {
        return *equip_slot_ptr((Player *)p, slot);
    }
    return item_stack_none();
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
   Description screen (POPUP)
   ══════════════════════════════════════════════════════════  */
static void item_build_description(ItemStack *stack, char *out, size_t out_size, const Player *p, EquipSlot equip_slot) {
    out[0] = '\0';

    if (stack->type == ITEM_NONE)
        return;

    const ItemDef *def = &ITEM_DB[stack->type];

    snprintf(out, out_size, "%s", item_display_desc(stack));

    size_t used = strlen(out);
    int has_item = player_has_item(p, stack->type);

    switch (def->cat) {
        case (ITEMCAT_WEAPON):
            const WeaponDef *weapon = &def->weapon;
            int avg_damage = weapon_average_damage(stack);

            switch (weapon->type){
            case(WEAPON_MELEE): {
                int vis_lvl = item_visibly_upgraded(stack);
                const char *quality = vis_lvl > 0 ? (item_is_broken(stack) ? "broken" : "upgraded") :  vis_lvl < 0 ? "degraded" : "";
                char article_quality[16];
                if (quality[0] == '\0') {
                    snprintf(article_quality, sizeof(article_quality), "a");
                } else {
                    int vowel = strchr("AEIOUaeiou", quality[0]) != NULL;
                    snprintf(article_quality, sizeof(article_quality), "%s%s", vowel ? "an " : "a ", quality);
                }

                used += snprintf(out + used, out_size - used,
                                 "\n\nThis %s is %s tier-%d melee weapon. Its %saverage damage is %d points per hit.",
                                 def->name, article_quality, weapon->melee.tier, stack->level_known ? "" : "typical ", avg_damage);

                if (!stack->level_known) {
                    used += snprintf(out + used, out_size - used, " It usually requires %d points of strength.", weapon->melee.str);
                    if(player_str(p) < weapon->melee.str)
                        used += snprintf(out + used, out_size - used, "\n\nProbably this weapon is too heavy for you. ");
                }

                if (weapon->melee.dly != 1.0f) {
                    used += snprintf(out + used, out_size - used, "\n\nThis is a rather %s", weapon->melee.dly < 1.0f ? "fast" : "slow");
                    if (weapon->melee.acu != 1.0f) {
                        int fast = weapon->melee.dly < 1.0f;
                        int accurate = weapon->melee.acu > 1.0f;
                        used += snprintf(out + used, out_size - used, "%s%s", (accurate == fast) ? " and " : " but ", accurate ? "accurate" : "inaccurate");
                    }
                    used += snprintf(out + used, out_size - used, " weapon.");
                } else if (weapon->melee.acu != 1.0f) {
                    used += snprintf(out + used, out_size - used, "\n\nThis is a rather %s weapon.", weapon->melee.acu > 1.0f ? "accurate" : "inaccurate");
                }

                if (stack->level_known && has_item) {
                    if(player_str(p) < weapon->melee.str) {
                        used += snprintf(out + used, out_size - used, 
                                         "\n\nBecause of your inadequate strength the accuracy and speed of your attack with this %s is decreased.",
                                          def->name);
                    }
                    else if(player_str(p) > weapon->melee.str) {
                        used += snprintf(out + used, out_size - used, 
                                         "\n\nBecause of your excess strength the damage of your attack with this %s is increased.",
                                          def->name);
                    }
                }
                if(equip_slot == EQUIP_WEAPON){
                    used += snprintf(out + used, out_size - used, "\n\nYou hold the %s at the ready%s",
                    def->name, stack->cursed ? ", and because it is cursed, you are powerless to let go." : "."); 
                } else if(item_visibly_cursed(stack)) {
                    used += snprintf(out + used, out_size - used, "\n\nYou can feel a malevolent magic lurking within %s.", def->name);
                }
                break;
            }
            case(WEAPON_MISSILE):
                used += snprintf(out + used, out_size - used, "\n\nAverage damage of this weapon equals to %d points per hit.", avg_damage);

                if (has_item) {
                    if(player_str(p) < weapon->missile.str) {
                        used += snprintf(out + used, out_size - used, 
                                         "\n\nBecause of your inadequate strength the accuracy and speed of your attack with this %s is decreased.",
                                          def->name);
                    } 
                    else if(player_str(p) > weapon->missile.str) {
                        used += snprintf(out + used, out_size - used, 
                                         "\n\nBecause of your excess strength the damage of your attack with this %s is increased.",
                                          def->name);
                    }
                }
                if (equip_slot == EQUIP_WEAPON) {
                    used += snprintf(out + used, out_size - used, "\n\nYou hold the %s at the ready.", def->name);
                }
                break;
            }
            break;
        case (ITEMCAT_ARMOR)://TODO glyph
            const ArmorDef *armor = &def->armor;
            used += snprintf(out + used, out_size - used, "\n\n%s %s provides damage absorption up to %d points per attack. ",
                             stack->level_known ? "This" : "Typical", def->name, armor_dr(stack));
            if (!stack->level_known) {
                used += snprintf(out + used, out_size - used, "It usually requires %d points of strength. ", armor->str);

                if (player_str(p) < armor->str) {
                    used += snprintf(out + used, out_size - used, "\n\nProbably this armor is too heavy for you. " );
                }
            } else if (player_str(p) < armor->str) {
                if (equip_slot == EQUIP_ARMOR) {
						used += snprintf(out + used, out_size - used,
					                     "\n\nBecause of your inadequate strength your movement speed and defense skill is decreased. " );
					} else {
						used += snprintf(out + used, out_size - used, 
					                     "\n\nBecause of your inadequate strength wearing this armor will decrease your movement speed and defense skill. " );
					}
            }
            if(equip_slot == EQUIP_ARMOR){
                used += snprintf(out + used, out_size - used, "\n\nYou are wearing the %s%s",
                def->name, stack->cursed ? ", and because it is cursed, you are powerless to remove it." : "."); 
            } else if(item_visibly_cursed(stack)) {
                used += snprintf(out + used, out_size - used, "\n\nYou can feel a malevolent magic lurking within the %s.", def->name);
            }
            break;
        case (ITEMCAT_WAND): {
            if (p->cls == HERO_MAGE) { 
                if (stack->level_known) { //TODO MOVE DMG RELATED STAFF TO THE WEAPON_MIN_MAX
                    int eff  = item_effective_level(stack);
                    int tier = 1 + eff / 3;
                    int min  = tier;
                    int max  = (tier * tier - tier + 10) / 2 + eff;
                    used += snprintf(out + used, out_size - used,
                                     "\n\nWhen this wand is used as a melee weapon, its average damage is %d points per hit.",
                                     min + (max - min) / 2);
                } else {
                    used += snprintf(out + used, out_size - used, "\n\nYou can use this wand as a melee weapon.");
                }
            }
            break;
        }
        case (ITEMCAT_RING): {
            if (equip_slot == EQUIP_RING1 || equip_slot == EQUIP_RING2) {
                used += snprintf(out + used, out_size - used, "\n\nThe %s is on your finger%s",
                                 def->name, stack->cursed ? ", and because it is cursed, you are powerless to remove it." : ".");
            } else if (item_visibly_cursed(stack)) {
                used += snprintf(out + used, out_size - used, "\n\nYou can feel a malevolent magic lurking within the %s.", def->name);
            }
            break;
        }
        case (ITEMCAT_SEED): {
            char plant[32];
            snprintf(plant, sizeof(plant), "%s", def->name);
            char *cut = strstr(plant, " Seed");
            if (cut)
                *cut = '\0';
            int vowel = strchr("AEIOUaeiou", plant[0]) != NULL;
            used = snprintf(out, out_size, "Throw this seed to the place where you want to grow %s %s.\n\n%s",
                            vowel ? "an" : "a", plant, item_display_desc(stack));
            break;
        }
        case (ITEMCAT_GOLD):
            if (stack->quantity == 0)
                snprintf(out, out_size, "%s", "Collect gold coins to spend them later in a shop.");
            else if (stack->quantity == 1)
                snprintf(out, out_size, "%s", "One gold coin. Collect gold coins to spend them later in a shop.");
            else
                snprintf(out, out_size, "A pile of %d gold coins. Collect gold coins to spend them later in a shop.", stack->quantity);
            break;
        case (ITEMCAT_POTION):
        case (ITEMCAT_SCROLL):
        case (ITEMCAT_FOOD):
        default:
            /* TODO Lloyd's Beacon info() reports the saved return depth */
            break;
    }
}
static void draw_durability_bar(SDL_Renderer *renderer, int x, int y, int width, int height, int current, int max) {
    if (max <= 0) {
        return;
    }

    if (current < 0) current = 0;
    if (current > max) current = max;

    SDL_SetRenderDrawColor(renderer, 220, 20, 50, 255);
    SDL_Rect bg = { x, y, width, height };
    SDL_RenderFillRect(renderer, &bg);

    int green_width = (width * current) / max;

    if (green_width > 0) {
        SDL_SetRenderDrawColor(renderer, 0, 230, 40, 255);
        SDL_Rect fg = { x, y, green_width, height };
        SDL_RenderFillRect(renderer, &fg);
    }
}

void render_item_description(RenderCtx *ctx, const InventoryUI *ui, const Player *p) {
    BagType shown = (ui->active_bag == BAG_BACKPACK || player_has_bag(p, ui->active_bag)) ? ui->active_bag : BAG_BACKPACK;
    int slot_count = shown == BAG_BACKPACK ? MAX_INVENTORY : bag_capacity_for_type(shown);

    ItemStack stack = inventory_get_stack(p, shown, ui->cursor, slot_count);
    EquipSlot equip_slot = equip_slot_from_cursor(ui->cursor);
    
    NinePatch window = chrome_window(ctx->ui.chrome);
    ninepatch_draw(ctx->renderer, &window, DESC_PANEL_X, DESC_PANEL_Y, DESC_PANEL_W, DESC_PANEL_H);
    
    draw_item_icon(ctx, stack, DESC_ICON_X, DESC_ICON_Y, ICON_SIZE);

    int durability = stack.durability;
    int max_durability = item_max_durability(stack.type, stack.level);
    draw_durability_bar(ctx->renderer, DESC_NAME_X, DESC_NAME_Y + 16, 132, 6, durability, max_durability); //REDO numbers

    char name[64];
    item_display_name(&stack, name, sizeof(name));
    draw_text(ctx, DESC_NAME_X, DESC_NAME_Y, COL_GOLD_TEXT, name);

    char description[512];
    item_build_description(&stack, description, sizeof(description), p, equip_slot);
    draw_wrapped_text(ctx, DESC_X, DESC_Y, COL_TEXT, description, DESC_LENGTH);
}

/* ══════════════════════════════════════════════════════════
   Inventory screen
   ══════════════════════════════════════════════════════════  */
void render_inventory(RenderCtx *ctx, const Player *p, const InventoryUI *ui) {
    SDL_SetRenderDrawBlendMode(ctx->renderer, SDL_BLENDMODE_BLEND);

    BagType shown = (ui->active_bag == BAG_BACKPACK || player_has_bag(p, ui->active_bag)) ? ui->active_bag : BAG_BACKPACK;
 
    int backpack_tab = (shown == BAG_BACKPACK);
    int grid_cols     = backpack_tab ? INV_COLS : BAG_COLS;
    int slot_count    = backpack_tab ? MAX_INVENTORY : bag_capacity_for_type(shown);
    
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

        ItemStack stack = inventory_get_stack(p, shown, i, slot_count);
        draw_slot(ctx, x, y, INV_SLOT, stack, i == ui->cursor);
    }

    draw_equip_slot(ctx, EQUIP_PANEL_X, EQUIP_PANEL_Y + 0 * (INV_SLOT + INV_GAP), INV_SLOT, p->weapon, SPRITE_WEAPON, ui->cursor == CURSOR_WEAPON);
    draw_equip_slot(ctx, EQUIP_PANEL_X, EQUIP_PANEL_Y + 1 * (INV_SLOT + INV_GAP), INV_SLOT, p->armor,  SPRITE_ARMOR,  ui->cursor == CURSOR_ARMOR );
    draw_equip_slot(ctx, EQUIP_PANEL_X, EQUIP_PANEL_Y + 2 * (INV_SLOT + INV_GAP), INV_SLOT, p->ring1,  SPRITE_RING,   ui->cursor == CURSOR_RING1 );
    draw_equip_slot(ctx, EQUIP_PANEL_X, EQUIP_PANEL_Y + 3 * (INV_SLOT + INV_GAP), INV_SLOT, p->ring2,  SPRITE_RING,   ui->cursor == CURSOR_RING2 );
    
    ItemStack stack = inventory_get_stack(p, shown, ui->cursor, slot_count);

    if (stack.type != ITEM_NONE) {
        char name_buf[64];
        char item_name[40];
        item_display_name(&stack, item_name, sizeof(item_name));

        if (ITEM_DB[stack.type].stackable && stack.quantity > 1)
            snprintf(name_buf, sizeof(name_buf), "%s x %d", item_name, stack.quantity);
        else
            snprintf(name_buf, sizeof(name_buf), "%s", item_name);

        draw_text(ctx, NAME_X, NAME_Y, COL_TEXT, name_buf);
    }
    SDL_SetRenderDrawBlendMode(ctx->renderer, SDL_BLENDMODE_NONE);
}

/* ══════════════════════════════════════════════════════════
   STATS SCREEN (POPUP)
   ══════════════════════════════════════════════════════════  */
static const char *player_buff_name(BuffType type) { //test
    switch (type) {
        case BUFF_POISON:         return "POISON";
        case BUFF_BLEEDING:       return "BLEEDING";
        case BUFF_BURNING:        return "BURNING";
        case BUFF_FROST:          return "FROST";
        case BUFF_PARALYSIS:      return "PARALYSIS";
        case BUFF_ROOTS:          return "ROOTS";
        case BUFF_LEVITATION:     return "LEVITATION";
        case BUFF_INVISIBILITY:   return "INVISIBILITY";
        case BUFF_SHADOWS:        return "SHADOWS";
        case BUFF_LIGHT:          return "LIGHT";
        case BUFF_FURY:           return "FURY";
        case BUFF_MIND_VISION:    return "MIND_VISION";
        case BUFF_OOZE:            return "OOZE";
        case BUFF_BARKSKIN:       return "BARKSKIN";
        case BUFF_HUNGER:         return "HUNGER";
        case BUFF_REGENERATION:   return "REGENERATION";
        case BUFF_COMBO:           return "COMBO";
        case BUFF_SNIPERS_MARK:   return "SNIPERS_MARK";
        case BUFF_AMOK:            return "AMOK";
        case BUFF_CHARM:           return "CHARM";
        case BUFF_VERTIGO:         return "VERTIGO";
        case BUFF_CRIPPLE:         return "CRIPPLE";
        case BUFF_SLEEP:           return "SLEEP";
        case BUFF_HASTE:           return "HASTE";
        case BUFF_SLOW:            return "SLOW";
        case BUFF_TERROR:          return "TERROR";
        case BUFF_WEAKNESS:        return "WEAKNESS";
        case BUFF_BLINDNESS:       return "BLINDNESS";
        case BUFF_AWARENESS:       return "AWARENESS";
        case BUFF_RAGE:            return "RAGE";
        case BUFF_GASES_IMMUNITY:  return "GASES_IMMUNITY";
        default:                   return "UNKNOWN";
    }
}

void render_stats_screen(RenderCtx *ctx, const Player *p) {
    SDL_SetRenderDrawBlendMode(ctx->renderer, SDL_BLENDMODE_BLEND);

    NinePatch window = chrome_window(ctx->ui.chrome);
    ninepatch_draw(ctx->renderer, &window, DESC_PANEL_X, DESC_PANEL_Y, DESC_PANEL_W, DESC_PANEL_H);
    
    /*label*/
    const char *class_name = "Unknown";

    switch (p->cls) {
        case HERO_WARRIOR:
            switch (p->sub_class) {
                case HERO_SUB_BERSERKER:  class_name = "Berserker";  break;
                case HERO_SUB_GLADIATOR:  class_name = "Gladiator";  break;
                default:                  class_name = "Warrior";    break;
            }
            break;
        case HERO_MAGE:
            switch (p->sub_class) {
                case HERO_SUB_WARLOCK:    class_name = "Warlock";    break;
                case HERO_SUB_BATTLEMAGE: class_name = "Battlemage"; break;
                default:                  class_name = "Mage";       break;
            }
            break;
        case HERO_ROGUE:
            switch (p->sub_class) {
                case HERO_SUB_FREERUNNER: class_name = "Freerunner"; break;
                case HERO_SUB_ASSASSIN:   class_name = "Assassin";   break;
                default:                  class_name = "Rogue";      break;
            }
            break;
        case HERO_HUNTRESS:
            switch (p->sub_class) {
                case HERO_SUB_SNIPER: class_name = "Sniper";  break;
                case HERO_SUB_WARDEN: class_name = "Warden";  break;
                default:              class_name = "Huntress"; break;
            }
            break;
        default:
            class_name = "Unknown";
            break;
    }

    int lvl = p->lvl;
    char current_lvl[64];
    snprintf(current_lvl, sizeof(current_lvl), "LEVEL %d %s", lvl, class_name);
    font_draw_text(ctx->renderer, &ctx->font, 160, 50, 1.6, COL_GOLD_TEXT, current_lvl);

    /*str*/
    int str = player_str(p);
    char current_str[32];
    snprintf(current_str, sizeof(current_str), "Strength: %d", str);
    font_draw_text(ctx->renderer, &ctx->font, 160, 80, 1.5, COL_WHITE, current_str);

    /*hp*/
    int hp = p->base.HP;
    int max_hp = p->base.HT;

    if (hp < 0) hp = 0;
    if (hp > max_hp) hp = max_hp;

    char current_hp[32];
    snprintf(current_hp, sizeof(current_hp), "Health: %d/%d", hp, max_hp);
    font_draw_text(ctx->renderer, &ctx->font, 160, 95, 1.5, COL_WHITE, current_hp);

    /*exp*/
    int exp = p->exp;
    int max_exp = player_max_exp(p);

    if (exp < 0) exp = 0;
    if (exp > max_exp) exp = max_exp;

    char current_exp[32];
    snprintf(current_exp, sizeof(current_exp), "Experience: %d/%d", exp, max_exp);
    font_draw_text(ctx->renderer, &ctx->font, 160, 110, 1.5, COL_WHITE, current_exp);

        /* buffs — TEST */
    int buff_y = 130;

    font_draw_text(ctx->renderer, &ctx->font, 160, buff_y, 1.3, COL_GOLD_TEXT, "TEST OF BUFFS:");

    buff_y += 15;

    for (int i = 0; i < MAX_CHAR_BUFFS; i++) {
        const Buff *buff = &p->base.buffs[i];

        if (buff->type == BUFF_NONE)
            continue;
        char buff_text[128];
        snprintf(buff_text, sizeof(buff_text), "%s  %.1f  [%d]", player_buff_name(buff->type), buff->duration, buff->data);
        font_draw_text(ctx->renderer, &ctx->font, 160, buff_y, 1.2, COL_WHITE, buff_text);

        buff_y += 13;
    }
        /* buffs — TEST */ 
        
    SDL_SetRenderDrawBlendMode(ctx->renderer, SDL_BLENDMODE_NONE);
}

/* ══════════════════════════════════════════════════════════
   HUD
   ══════════════════════════════════════════════════════════  */

   //TODO BUFFS
static void render_busy_indicator(RenderCtx *ctx, const Player *p) {
    if (!char_is_alive(&p->base)) // || p->ready)
        return;
    uint8_t sx, sy, sw, sh;
    icon_sprite(ICON_BUSY, &sx, &sy, &sw, &sh);

    SDL_Rect src = { sx, sy, sw, sh };
    SDL_Rect dst = { 0,  50, sw * 2, sh * 2 };

    SDL_SetTextureBlendMode(ctx->icons, SDL_BLENDMODE_BLEND);
    SDL_SetTextureAlphaMod(ctx->icons, 128);

    SDL_Point center = { dst.w / 2, dst.h / 2 };

    SDL_RenderCopyEx(ctx->renderer, ctx->icons, &src, &dst, ctx->busy_angle, &center, SDL_FLIP_NONE);
    SDL_SetTextureAlphaMod(ctx->icons, 255);
}

//test
void hud_update(RenderCtx *ctx, const Player *p) {
    if (!p || !char_is_alive(&p->base)) { //|| p->ready) {
        ctx->busy_angle = 0.0f;
        return;
    }
    ctx->busy_angle = fmodf(actor_now() * 45.0f, 360.0f);
}

void render_hud(RenderCtx *ctx, const Player *p, const Dungeon *d) {
    /* ── Status panel ───────────────────────────────────── */
    NinePatch status = status_panel(ctx->ui.status);
    ninepatch_draw(ctx->renderer, &status, 0, 0, 480, 96); //SCREEN_W, TEXTURE HIGH * SCALE

    /* ── HP bar ──────────────────────────────────────────── */
    if (ctx->ui.hp_bar && p && p->base.HT > 0) {
        int hp = p->base.HP;
        int max_hp = p->base.HT;
        if (hp < 0)
            hp = 0;
        if (hp > max_hp)
            hp = max_hp;

        float hp_ratio = (float)hp / (float)max_hp;

        const int bar_x = 45;
        const int bar_y = 4;
        const int bar_w = 70; //original texture size 64X4
        const int bar_h = 6;

        int tex_w = 0;
        int tex_h = 0;

        SDL_QueryTexture(ctx->ui.hp_bar, NULL, NULL, &tex_w, &tex_h);

        if (tex_w > 0 && tex_h > 0) {
            int fill_w = (int)(bar_w * hp_ratio);
            if (fill_w > 0) {
                SDL_Rect src = {
                    0,
                    0,
                    (int)(tex_w * hp_ratio),
                    tex_h
                };
                SDL_Rect dst = {
                    bar_x,
                    bar_y,
                    fill_w,
                    bar_h
                };
                SDL_RenderCopy(ctx->renderer, ctx->ui.hp_bar, &src, &dst);
            }
        }
    }
    /* ── XP bar ──────────────────────────────────────────── */
    if (ctx->ui.xp_bar && p) {
        int exp = p->exp;
        int max_exp = player_max_exp(p);

        if (exp < 0) exp = 0;
        if (exp > max_exp) exp = max_exp;

        float exp_ratio = (float)exp / (float)max_exp;

        const int bar_x = 0;
        const int bar_y = 0;
        const int bar_w = 480;
        const int bar_h = 1;

        int tex_w = 0;
        int tex_h = 0;

        SDL_QueryTexture(ctx->ui.xp_bar, NULL, NULL, &tex_w, &tex_h);

        if (tex_w > 0 && tex_h > 0) {
            const int fill_tex_w = tex_w / 2; //stupid fix of broken color gradient 
            int fill_w = (int)(bar_w * exp_ratio);

            if (fill_w > 0) {
                SDL_Rect src = {
                    0,
                    0,
                    fill_tex_w,
                    tex_h
                };

                SDL_Rect dst = {
                    bar_x,
                    bar_y,
                    fill_w,
                    bar_h
                };

                SDL_RenderCopy(ctx->renderer, ctx->ui.xp_bar, &src, &dst);
            }
        }
    }
    /* ── player sprite ───────────────────────────────────────── */
    if (p) { 
        SDL_Texture *hero_tex = ctx->heroes[p->cls]; 
        if (hero_tex) { 
            SDL_Rect src = { 
                0, 
                get_armor_tier(&p->armor) * TILESET_HERO_H, 
                TILESET_HERO_W, 
                TILESET_HERO_H 
            }; 
            const int icon_h = TILESET_HERO_H * 2; 
            const int icon_w = TILESET_HERO_W * 2; 
            SDL_Rect dst = { 
                9, 
                9, 
                icon_w, 
                icon_h 
            }; 
            SDL_RenderCopy(ctx->renderer, hero_tex, &src, &dst ); 
        } 
    }
    /* ── player lvl ──────────────────────────────────────────── */
    char current_lvl[8];
    snprintf(current_lvl, sizeof(current_lvl), "%d", p->lvl);

    int dx = 0;
    if(p->lvl > 9) 
        dx = 3;
    font_draw_text(ctx->renderer, &ctx->font, 38 - dx, 38, 1.39, COL_WHITE, current_lvl);
    /* ── dungeon lvl ──────────────────────────────────────────── */
    char current_floor[16];
    snprintf(current_floor, sizeof(current_floor), "%d", d->current_depth + 1);
    int dt = 0;
    if(d->current_depth + 1 > 9) 
        dt = 7;
    font_draw_text(ctx->renderer, &ctx->font, 435 - dt, 9, 1.5, COL_WHITE, current_floor);
    /* ── keys counter ──────────────────────────────────────────── */
    //TODO rewrite after locking keys to its current floor
    const Bag *keyring = &p->bags[BAG_KEYRING];
    int keys = 0;
    for (int i = 0; i < keyring->count; i++)
        keys += keyring->items[i].quantity;

    char key_count[16];
    snprintf(key_count, sizeof(key_count), "%d", keys);

    font_draw_text(ctx->renderer, &ctx->font, 459, 9, 1.5, COL_WHITE, key_count);

    /* ── busy indicator ───────────────────────────────── */
    render_busy_indicator(ctx, p);
}