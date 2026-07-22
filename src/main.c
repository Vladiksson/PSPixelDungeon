#include <pspkernel.h>
#include <pspdebug.h>
#include <pspsysmem.h>
#include <SDL2/SDL.h>
#include <stdio.h>

#include "common.h"
#include "level.h"
#include "camera.h"
#include "render.h"
#include "input.h"
#include "player.h"
#include "fov.h"
#include "dungeon.h"
#include "item.h"
#include "inventory_screen.h" 
#include "font.h"

/* ── PSP kernel module metadata (required for EBOOT.PBP) ──────── */
/*PSP_MODULE_INFO("PixelDungeonPSP", PSP_MODULE_USER, 1, 0);*/
/*PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);*/
PSP_HEAP_SIZE_KB(16384); // 16 MB safe


/* ── Asset path ───────────────────────────────────────── */
#define ASSETS_DIR "assets"
 
/* ── Movement repeat (hold-to-walk) ──────────────────
   First press moves immediately.
   After REPEAT_DELAY_MS, moves every REPEAT_RATE_MS.   */
#define REPEAT_DELAY_MS  220
#define REPEAT_RATE_MS    80
 
typedef struct { uint32_t next_ms; int held; } MoveRepeat;
 
static int move_repeat_update(
        MoveRepeat *mr, InputState *inp, uint32_t now,
        Player *p, Level *lvl, Camera *cam,
        Button btn, int dx, int dy){
    if (btn_pressed(inp, btn)) {

        if (player_move(p, lvl, dx, dy)) {
            camera_center_on(cam, p->x, p->y);
            mr->held = btn;
            mr->next_ms = now + REPEAT_DELAY_MS;
            return 1;
        }

        mr->held = btn;
        mr->next_ms = now + REPEAT_DELAY_MS;

    } else if (mr->held == (int)btn &&
               btn_held(inp, btn) &&
               now >= mr->next_ms) {

        if (player_move(p, lvl, dx, dy)) {
            camera_center_on(cam, p->x, p->y);
            mr->next_ms = now + REPEAT_RATE_MS;
            return 1;
        }

        mr->next_ms = now + REPEAT_RATE_MS;

    } else if (!btn_held(inp, btn) && mr->held == (int)btn) {
        mr->held = 0;
    }

    return 0;
}



/* ── Main ─────────────────────────────────────────────── */
int main(int argc, char *argv[]) {
    (void)argc; (void)argv;
    Terrain_InitFlags(); 

    static Dungeon dungeon;
    dungeon_init(&dungeon);

    Level *lvl = dungeon_current_level(&dungeon);
    Fog *fog = dungeon_current_fog(&dungeon);

    Player player;
    player_init(&player, lvl, 1);

    InventoryUI inv = {0};

    Camera cam;
    camera_init(&cam, 0, 0);
    camera_center_on(&cam, player.x, player.y);

    RenderCtx rctx;
    render_init(&rctx, ASSETS_DIR);
 
    InputState inp;
    input_init(&inp);
    
    MoveRepeat mr = {0, 0};
    
    uint32_t last = SDL_GetTicks();
    /* ── Game loop ──────────────────────────────────── */
    while (!inp.quit) {
        uint32_t now = SDL_GetTicks();
        float dt = (now - last) / 1000.0f;
        last = now;
        input_update(&inp);
        
        if (btn_pressed(&inp, BTN_SELECT)) { //test of sdl inventory
            if (inv.open)
                inventory_ui_close(&inv);
            else
                inventory_ui_open(&inv);
        }
        if (inv.open) {
            if (btn_pressed(&inp, BTN_UP))
                inventory_ui_move(&inv, 0, -1);
            if (btn_pressed(&inp, BTN_DOWN))
                inventory_ui_move(&inv, 0, 1);
            if (btn_pressed(&inp, BTN_LEFT))
                inventory_ui_move(&inv, -1, 0);
            if (btn_pressed(&inp, BTN_RIGHT))
                inventory_ui_move(&inv, 1, 0);
            if (btn_pressed(&inp, BTN_B))
                inventory_ui_close(&inv);
            if (btn_pressed(&inp, BTN_A)) {
                if (inv.cursor < MAX_INVENTORY) {
                    player_equip_item(&player, inv.cursor);
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
            if (btn_pressed(&inp, BTN_Y))
                player_drop_item(&player, lvl, inv.cursor);
            if (btn_pressed(&inp, BTN_X))
                player_sort_inventory(&player);
            if (btn_pressed(&inp, BTN_R))
                inventory_ui_next_tab(&inv, &player);
            if (btn_pressed(&inp, BTN_L))
                inventory_ui_prev_tab(&inv, &player);

        } else {
            /* Player movement — d-pad */
            int moved = 0;

            moved |= move_repeat_update(&mr, &inp, now, &player, lvl, &cam, BTN_UP,    0, -1);
            moved |= move_repeat_update(&mr, &inp, now, &player, lvl, &cam, BTN_DOWN,  0, +1);
            moved |= move_repeat_update(&mr, &inp, now, &player, lvl, &cam, BTN_LEFT, -1,  0);
            moved |= move_repeat_update(&mr, &inp, now, &player, lvl, &cam, BTN_RIGHT,+1,  0);

            if (moved) {
                if (player.anim == ANIM_IDLE)
                    player_play_anim(&player, ANIM_RUN);

                if (!player_operate(&player, lvl))
                    player_pickup(&player, lvl); /* TODO: route this to a message log / HUD once one exists */

                if (level_player_on_exit(lvl, player.x, player.y)) {
                    dungeon_go_down(&dungeon, &player, &cam);
                    lvl = dungeon_current_level(&dungeon);
                    fog = dungeon_current_fog(&dungeon);
                    continue;
                }
                if (level_player_on_entrance(lvl, player.x, player.y)) {
                    dungeon_go_up(&dungeon, &player, &cam);
                    lvl = dungeon_current_level(&dungeon);
                    fog = dungeon_current_fog(&dungeon);
                    continue;
                }
                dungeon_on_player_move(&dungeon, &player);

            } else {
                if (player.anim == ANIM_RUN)
                    player_play_anim(&player, ANIM_IDLE);
            }
        }
        
        fog_update(fog, lvl, player.x, player.y);
        player_animate(&player, dt);
        /* Draw: tiles → items → fog → player → present */
        render_frame(&rctx, lvl, &cam);
        render_items(&rctx, lvl, &cam);
        render_shadows(&rctx, lvl, fog, &cam);
        render_fog(&rctx, fog, &cam);
        render_player(&rctx, &player, &cam);
        if (inv.open)
            render_inventory(&rctx, &player, &inv);
        render_present(&rctx);
        
        SDL_Delay(1);
    }

    render_free(&rctx);
    SDL_Quit();
    return 0;
}