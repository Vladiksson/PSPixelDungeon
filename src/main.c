#include <pspkernel.h>
#include <pspdebug.h>
#include <pspsysmem.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>

#include "render.h"
#include "input.h"
#include "dungeon.h"
#include "ui_inventory.h" //REDO TEMPORARY
#include "ui_hud.h" //REDO TEMPORARY

/* ── PSP kernel module metadata ────────────────────────── */
PSP_HEAP_SIZE_KB(16384); // 16 MB, in other case sdl gonna eat all of ram, shitty temporary solution

/* ── Movement repeat ──────────────────────────────────── */
#define REPEAT_DELAY_MS  220
#define REPEAT_RATE_MS   (REPEAT_DELAY_MS / 2) 
typedef struct {
    uint32_t next_ms;
    int held;
    int blocked;
} MoveRepeat;
 
static int move_repeat_update(MoveRepeat *mr, uint32_t now, Player *p, Level *lvl, Camera *cam, int dx, int dy) {
    if (dx == 0 && dy == 0) {
        mr->held    = 0;
        mr->blocked = 0;
        return 0;
    }
    if (!mr->held) {
        int result = player_bump(p, lvl, dx, dy);
        mr->held = 1;
        mr->next_ms = now + REPEAT_DELAY_MS;
        mr->blocked = (result == PLAYER_BUMP_BLOCKED);

        if (result == PLAYER_BUMP_MOVED) {
            camera_center_on(cam, p->base.pos);
        }
        return result;
    }
    if (now >= mr->next_ms) {
        int result = player_bump(p, lvl, dx, dy);
        mr->next_ms = now + REPEAT_RATE_MS;
        mr->blocked = (result == PLAYER_BUMP_BLOCKED);

        if (result == PLAYER_BUMP_MOVED)
            camera_center_on(cam, p->base.pos);
        return result;
    }
    return 0;
}

/* ── Main ─────────────────────────────────────────────── */
int main(int argc, char *argv[]) {
    (void)argc; (void)argv;

    gTileSize = TILE_SIZE_MIN;
    TileSize_Update();

    Terrain_InitFlags(); 

    static Dungeon dungeon;
    dungeon_init(&dungeon);

    Level *lvl = dungeon_current_level(&dungeon);
    Fog *fog = dungeon_current_fog(&dungeon);

    Player player;
    player_init(&player, lvl, 0);

    InventoryUI inv = {0};

    Camera cam;
    camera_center_on(&cam, player.base.pos);

    RenderCtx rctx;
    if (render_init(&rctx) != 0) {
        SDL_Quit();
        return -1;
    }
 
    InputState inp;
    input_init(&inp);
    
    MoveRepeat mr = {0};
    
    uint32_t last = SDL_GetTicks();
    /* ── Game loop ──────────────────────────────────── */
    while (!inp.quit) {
        uint32_t now = SDL_GetTicks();
        float dt = (now - last) / 1000.0f;
        last = now;
        if (dt > 0.1f) 
            dt = 0.1f;

        input_update(&inp);
        
        if (btn_pressed(&inp, BTN_SELECT)) {
            if (inv.open)
                inventory_ui_close(&inv);
            else
                inventory_ui_open(&inv);
        }
        if (inv.open) {
            if (!inv.desc_open && !inv.pending_ring) {
                if (btn_pressed(&inp, BTN_UP))
                    inventory_ui_move(&inv, 0, -1);
                if (btn_pressed(&inp, BTN_DOWN))
                    inventory_ui_move(&inv, 0, 1);
                if (btn_pressed(&inp, BTN_LEFT))
                    inventory_ui_move(&inv, -1, 0);
                if (btn_pressed(&inp, BTN_RIGHT))
                    inventory_ui_move(&inv, 1, 0);
                if (btn_pressed(&inp, BTN_CROSS)) {
                    if (inv.cursor >= 0 && inv.cursor < MAX_INVENTORY) {
                        Bag *bag = &player.bags[inv.active_bag];
                        item_identify(&bag->items[inv.cursor]);//TEST

                        int res = player_equip_item(&player, &player.bags[inv.active_bag], inv.cursor);

                        if (res == 2) {
                            inv.pending_ring = inv.cursor + 1;
                        }
                    } else {
                        switch (inv.cursor) {
                        case CURSOR_WEAPON:
                            player_unequip(&player, EQUIP_WEAPON, lvl);
                            break;
                        case CURSOR_ARMOR:
                            player_unequip(&player, EQUIP_ARMOR, lvl);
                            break;
                        case CURSOR_RING1:
                            player_unequip(&player, EQUIP_RING1, lvl);
                            break;
                        case CURSOR_RING2:
                            player_unequip(&player, EQUIP_RING2, lvl);
                            break;
                        }
                    }
                }
                if (btn_pressed(&inp, BTN_SQUARE))
                    player_drop_from_bag(&player, lvl, &player.bags[inv.active_bag], inv.cursor);
                if (btn_pressed(&inp, BTN_TRIANGLE)) {
                    Bag *bag = &player.bags[inv.active_bag];

                    item_identify(&bag->items[inv.cursor]);//TEST
                    if (item_consume(&bag->items[inv.cursor]) == ITEM_USE_CONSUMED) {//TEST
                            bag_remove_item(bag, inv.cursor, 1);
                    }
                }
                if (btn_pressed(&inp, BTN_R))
                    inventory_ui_next_tab(&inv, &player);
                if (btn_pressed(&inp, BTN_L))
                    inventory_ui_prev_tab(&inv, &player);
            }
            if (inv.pending_ring){
                if (btn_pressed(&inp, BTN_L)) {
                    player_equip_ring(&player, &player.bags[inv.active_bag], inv.pending_ring - 1, EQUIP_RING1);
                    inv.pending_ring = 0;
                }
                if (btn_pressed(&inp, BTN_R)) {
                    player_equip_ring(&player, &player.bags[inv.active_bag], inv.pending_ring - 1, EQUIP_RING2);
                    inv.pending_ring = 0;
                }
            }
            if (btn_pressed(&inp, BTN_CIRCLE)) {
                if (inv.pending_ring) { inv.pending_ring = 0; }
                if (inv.desc_open) {
                    inventory_ui_desc_close(&inv);
                } else if (inv.cursor >= 0 && inv.cursor < MAX_INVENTORY) {
                    Bag *bag = &player.bags[inv.active_bag];

                    if (bag->items[inv.cursor].type != ITEM_NONE)
                        inventory_ui_desc_open(&inv);
                } else {
                    ItemStack *item = NULL;

                    switch (inv.cursor) {
                    case CURSOR_WEAPON:
                        item = &player.weapon;
                        break;
                    case CURSOR_ARMOR:
                        item = &player.armor;
                        break;
                    case CURSOR_RING1:
                        item = &player.ring1;
                        break;
                    case CURSOR_RING2:
                        item = &player.ring2;
                        break;
                    }

                    if (item && item->type != ITEM_NONE)
                        inventory_ui_desc_open(&inv);
                }
            }  
        } else {
            if (btn_pressed(&inp, BTN_L)) {
                if (gTileSize == TILE_SIZE_MIN) {
                    gTileSize = TILE_SIZE_MAX;
                } else {
                    gTileSize = TILE_SIZE_MIN;
                }
                TileSize_Update();
                camera_center_on(&cam, player.base.pos);
            }
            /* Player movement — d-pad */
            int dx, dy;
            input_get_move(&inp, &dx, &dy);

            int moving_input = (dx != 0 || dy != 0);

            int moved = move_repeat_update(&mr, now, &player, lvl, &cam, dx, dy);
            if (moved == PLAYER_BUMP_MOVED) {
                if (moving_input)
                    player_set_moving(&player, 1);

                if (!player_operate(&player, lvl))
                    player_pickup(&player, lvl);

                if (player.base.pos == lvl->exit) {
                    dungeon_go_down(&dungeon, &player, &cam);
                    lvl = dungeon_current_level(&dungeon);
                    fog = dungeon_current_fog(&dungeon);
                    continue;
                }
                if (player.base.pos == lvl->entrance) {
                    dungeon_go_up(&dungeon, &player, &cam);
                    lvl = dungeon_current_level(&dungeon);
                    fog = dungeon_current_fog(&dungeon);
                    continue;
                }
                dungeon_event(&dungeon, &player);

            } else if (moved == PLAYER_BUMP_ATTACKED) {
                /* Attack already started; player_animate() will resolve it. */
            } else if (!moving_input || mr.blocked) {
                player_set_moving(&player, 0);
            }
        }
        actor_process();

        fog_update(fog, lvl, player.base.pos, dungeon.current_depth);
        hud_update(&rctx, &player);

        player_animate(&player, dt);
        mob_animate_all(dt);
        npc_animate_all(dt);
        /* Draw: tiles → items → mobs → npcs → fog → player → present */
        render_frame(&rctx, lvl, &cam);
        render_items(&rctx, lvl, &cam);
        render_mobs(&rctx, &cam);
        render_npcs(&rctx, &cam);
        render_fog(&rctx, fog, &cam);
        render_player(&rctx, &player, &cam);

        render_hud(&rctx, &player, &dungeon);

        if (inv.open)
            render_inventory(&rctx, &player, &inv);
        render_present(&rctx);
        
        SDL_Delay(1);
    }

    render_free(&rctx);
    IMG_Quit();
    SDL_Quit();
    return 0;
}