#include "game.h"

/* ══════════════════════════════════════════════════════════
   Dungeon helper
   ══════════════════════════════════════════════════════════*/
static void game_floor(Game *game) {
    game->lvl = dungeon_current_level(&game->dungeon);
    game->fog = dungeon_current_fog(&game->dungeon);
}

/* ══════════════════════════════════════════════════════════
   Movement repeat
   ══════════════════════════════════════════════════════════*/
static void move_repeat_init(MoveRepeat *mr) { memset(mr, 0, sizeof(*mr)); }

#define MOVE_REPEAT_DELAY  4.5f
#define MOVE_REPEAT_SPEED  9.0f

static uint32_t move_repeat_delay_ms(const Player *p) {
    float speed = char_speed((Char *)&p->base);
    if (speed <= 0.0f)
        speed = 0.01f;
    return (uint32_t)((1000.0f / sqrtf(speed)) / MOVE_REPEAT_DELAY);
}

static uint32_t move_repeat_rate_ms(const Player *p) {
    float speed = char_speed((Char *)&p->base);
    if (speed <= 0.0f)
        speed = 0.01f;
    return (uint32_t)((1000.0f / sqrtf(speed)) / MOVE_REPEAT_SPEED);
}

static int move_repeat_update(MoveRepeat *mr, uint32_t now, Player *p, Level *lvl, Camera *cam, int dx, int dy) {
    if (dx == 0 && dy == 0) {
        mr->held    = 0;
        mr->blocked = 0;
        return 0;
    }
    if (!mr->held) {
        int result = player_bump(p, lvl, dx, dy);
        mr->held = 1;
        mr->next_ms = now + move_repeat_delay_ms(p);
        mr->blocked = (result == PLAYER_BUMP_BLOCKED);

        if (result == PLAYER_BUMP_MOVED)
            camera_center_on(cam, p->base.pos);
        return result;
    }
    if (now >= mr->next_ms) {
        int result = player_bump(p, lvl, dx, dy);
        mr->next_ms = now + move_repeat_rate_ms(p);
        mr->blocked = (result == PLAYER_BUMP_BLOCKED);

        if (result == PLAYER_BUMP_MOVED)
            camera_center_on(cam, p->base.pos);
        return result;
    }
    return 0;
}

#define UI_MOVE_REPEAT_DELAY_MS 160 //mb change???

static int ui_move_repeat_update(MoveRepeat *mr, uint32_t now, int dx, int dy) {
    if (dx == 0 && dy == 0) {
        mr->held = 0;
        return 0;
    }

    if (!mr->held || now >= mr->next_ms) {
        mr->held = 1;
        mr->next_ms = now + UI_MOVE_REPEAT_DELAY_MS;
        return 1;
    }

    return 0;
}

/*══════════════════════════════════════════════════════════
  Initialization
  ══════════════════════════════════════════════════════════*/
void game_init(Game *game) {
    memset(game, 0, sizeof(*game));

    gTileSize = TILE_SIZE_MIN;
    TileSize_Update();

    Terrain_InitFlags();

    dungeon_init(&game->dungeon);

    game_floor(game);

    player_init(&game->player, game->lvl, 0);

    camera_center_on(&game->cam, game->player.base.pos);

    move_repeat_init(&game->move_repeat);
    move_repeat_init(&game->ui_move_repeat);

    game->state = GAME_STATE_PLAYING;
    game->popup = POPUP_NONE;
    game->popup_return_state = GAME_STATE_PLAYING;
}

/* ══════════════════════════════════════════════════════════
   Popup helpers
   ══════════════════════════════════════════════════════════*/

static void game_open_popup(Game *game, PopupType popup) {
    game->popup_return_state = game->state;
    game->popup = popup;
    game->state = GAME_STATE_POPUP;
}

static void game_close_popup(Game *game) {
    game->popup = POPUP_NONE;
    game->state = game->popup_return_state;
}

/*══════════════════════════════════════════════════════════
   Gameplay input
  ══════════════════════════════════════════════════════════*/
static void game_handle_move(Game *game) {
    Player *player = &game->player;
    Level *lvl = game->lvl;

    if (!player_operate(player, lvl, player->base.pos)) {
        player_pickup(player, lvl);
    }

    if (player->base.pos == lvl->exit) {
        dungeon_go_down(&game->dungeon, player, &game->cam);
        game_floor(game);
        return;
    }

    if (player->base.pos == lvl->entrance) {
        dungeon_go_up(&game->dungeon, player, &game->cam);
        game_floor(game);
        return;
    }

    dungeon_event(&game->dungeon, player);
}

#define REST_START_DELAY_MS 500 /*CONSTANTS FOR TEST BULLSHIT VERSION OF RESTING*/
#define REST_DELAY_MS 60

static void gameplay_input(Game *game, InputState *inp, uint32_t now) {
    Player *player = &game->player;

    if (btn_pressed(inp, BTN_SELECT)) {
        game->state = GAME_STATE_INVENTORY;
        game->inv.cursor = 0; game->inv.active_bag = BAG_BACKPACK; game->inv.pending_ring = -1;
        return;
    }

    if (btn_pressed(inp, BTN_START)) {
        game_open_popup(game, POPUP_STATS);
        return;
    }

    if (btn_pressed(inp, BTN_L)) {
        gTileSize = (gTileSize == TILE_SIZE_MIN) ? TILE_SIZE_MAX : TILE_SIZE_MIN;
        TileSize_Update();
        camera_center_on(&game->cam, player->base.pos);
    }

    /*TEST BULLSHIT VERSION OF RESTING*/
    if (btn_pressed(inp, BTN_CROSS)) {
        player->resting = 0;

        player_rest(player);

        game->rest_next_ms = now + REST_START_DELAY_MS;
        game->rest_pending = 1;

        return;
    }

    if (game->rest_pending) {
        if (btn_released(inp, BTN_CROSS)) {
            game->rest_pending = 0;
            return;
        } else if (now >= game->rest_next_ms) {
            game->rest_pending = 0;
            player->resting = 1;
            game->rest_next_ms = now + REST_DELAY_MS;
        }

        return;
    }

    if (player->resting && now >= game->rest_next_ms) {
        player_rest(player);

        game->rest_next_ms = now + REST_DELAY_MS;
        return;
    }


    //Movement
    int dx, dy;
    input_get_move(inp, &dx, &dy);
    
    int moving = (dx != 0 || dy != 0);
    int result = move_repeat_update(&game->move_repeat, now, player, game->lvl, &game->cam, dx, dy);

    switch (result) {
    case PLAYER_BUMP_MOVED:
        player_set_moving(player, moving);
        game_handle_move(game);
        break;

    case PLAYER_BUMP_ATTACKED:
        /*Attack animation / actor processing will resolve the action. */
        break;

    default:
        if (!moving || game->move_repeat.blocked) {
            player_set_moving(player, 0);
        }
        break;
    }
}

/* ══════════════════════════════════════════════════════════
   Inventory
   ══════════════════════════════════════════════════════════*/
static void inventory_input(Game *game, InputState *inp, uint32_t now) {
    Player *player = &game->player;
    InventoryUI *inv = &game->inv;
    Level *lvl = game->lvl;

    if (btn_pressed(inp, BTN_SELECT)) {
        game->state = GAME_STATE_PLAYING;
        return;
    }

    if (inv->pending_ring >= 0) {
        Bag *bag = &player->bags[inv->active_bag];
        int slot = inv->pending_ring;

        if (btn_pressed(inp, BTN_CIRCLE)) {
            inv->pending_ring = -1;
            return;
        }
        if (btn_pressed(inp, BTN_L)) {
            player_equip_ring(player, bag, slot, EQUIP_RING1, 1);
            inv->pending_ring = -1;
        } else if (btn_pressed(inp, BTN_R)) {
            player_equip_ring(player, bag, slot, EQUIP_RING2, 1);
            inv->pending_ring = -1;
        }
        return;
    }

    int dx, dy;
    input_get_move(inp, &dx, &dy);

    if (ui_move_repeat_update(&game->ui_move_repeat, now, dx, dy)) {
        inventory_ui_move(&game->inv, dx, dy);
    }

    if (btn_pressed(inp, BTN_CROSS)) {
        if (inv->cursor >= 0 && inv->cursor < MAX_INVENTORY) {
            Bag *bag = &player->bags[inv->active_bag];
            item_identify(&bag->items[inv->cursor]); //TEST

            int res = player_equip_item(player, bag, inv->cursor, 1);
            if (res == EQUIP_RING_SELECT) {
                inv->pending_ring = inv->cursor;
            }
        } else {
            switch (inv->cursor) {
            case CURSOR_WEAPON:
                player_unequip(player, EQUIP_WEAPON, lvl);
                break;
            case CURSOR_ARMOR:
                player_unequip(player, EQUIP_ARMOR, lvl);
                break;
            case CURSOR_RING1:
                player_unequip(player, EQUIP_RING1, lvl);
                break;
            case CURSOR_RING2:
                player_unequip(player, EQUIP_RING2, lvl);
                break;
            }
        }
    }

    if (btn_pressed(inp, BTN_SQUARE))
        player_drop_from_bag(player, lvl, &player->bags[inv->active_bag], inv->cursor);

    if (btn_pressed(inp, BTN_TRIANGLE)) {
        Bag *bag = &player->bags[inv->active_bag];

        item_identify(&bag->items[inv->cursor]); //TEST
        player_consume_item(player, &player->bags[inv->active_bag], inv->cursor);
    }

    if (btn_pressed(inp, BTN_R))
        inventory_ui_next_tab(inv, player);
    if (btn_pressed(inp, BTN_L))
        inventory_ui_prev_tab(inv, player);

    if (btn_pressed(inp, BTN_CIRCLE)) {
        if (inv->cursor >= 0 && inv->cursor < MAX_INVENTORY) {
            Bag *bag = &player->bags[inv->active_bag];

            if (bag->items[inv->cursor].type != ITEM_NONE)
                game_open_popup(game, POPUP_ITEM_DESC);
        } else {
            ItemStack *item = NULL;

            switch (inv->cursor) {
            case CURSOR_WEAPON: item = &player->weapon; break;
            case CURSOR_ARMOR:  item = &player->armor;  break;
            case CURSOR_RING1:  item = &player->ring1;  break;
            case CURSOR_RING2:  item = &player->ring2;  break;
            }

            if (item && item->type != ITEM_NONE)
                game_open_popup(game, POPUP_ITEM_DESC);
        }
    }
}
/* ══════════════════════════════════════════════════════════
   Popup
   ══════════════════════════════════════════════════════════*/
static void item_desc_input(Game *game, InputState *inp) {
    if (btn_pressed(inp, BTN_CIRCLE)) {
        game_close_popup(game);
        return;
    }
}
static void stats_input(Game *game, InputState *inp) {
    if (btn_pressed(inp, BTN_START)) {
        game_close_popup(game);
        return;
    }
}

static void popup_input(Game *game, InputState *inp) {
    switch (game->popup) {
    case POPUP_ITEM_DESC:
        item_desc_input(game, inp);
        break;
    case POPUP_STATS:
        stats_input(game, inp);
        break;
    case POPUP_NONE:
    default:
        game_close_popup(game);
        break;
    }
}

/* ══════════════════════════════════════════════════════════
   INPUT
   ══════════════════════════════════════════════════════════*/
void game_input(Game *game, InputState *inp, uint32_t now) {
    switch (game->state) {
    case GAME_STATE_PLAYING:
        gameplay_input(game, inp, now);
        break;
    case GAME_STATE_INVENTORY:
        inventory_input(game, inp, now);
        break;
    case GAME_STATE_POPUP:
        popup_input(game, inp);
        break;
    }
}


/* ══════════════════════════════════════════════════════════
   Update
   ══════════════════════════════════════════════════════════*/
void game_update(Game *game, RenderCtx *rctx, float dt) {
    actor_process();

    fog_update(game->fog, game->lvl, game->player.base.pos, fov_radius(&game->player, game->dungeon.current_depth));
    hud_update(rctx, &game->player);

    render_update_water(rctx, dt); //TODO PARTICLES GONNA LIVE HERE

    player_animate(&game->player, dt);
    mob_animate_all(dt);
    npc_animate_all(dt);
    
    actor_process();
}

/* ══════════════════════════════════════════════════════════
   Rendering
   ══════════════════════════════════════════════════════════*/
void game_render(Game *game, RenderCtx *rctx) {
    render_frame(rctx, game->lvl, &game->cam);
    render_items(rctx, game->lvl, game->fog, &game->cam);
    render_mobs(rctx, game->fog, &game->cam);
    render_npcs(rctx, &game->player, game->fog, &game->cam);
    render_fog(rctx, game->fog, &game->cam);
    render_player(rctx, &game->player, &game->cam);
    render_hud(rctx, &game->player, &game->dungeon);

    if (game->state == GAME_STATE_INVENTORY) {
        render_inventory(rctx, &game->player, &game->inv);
    }

    if (game->state == GAME_STATE_POPUP) {
        switch (game->popup) {
        case POPUP_ITEM_DESC:
            render_item_description(rctx, &game->inv, &game->player);
            break;
 
        case POPUP_STATS:
            render_stats_screen(rctx, &game->player);
            break;
 
        case POPUP_NONE:
        default:
            break;
        }
    }
    render_present(rctx);
}