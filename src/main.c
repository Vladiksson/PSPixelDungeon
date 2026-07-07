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

/* ── PSP kernel module metadata (required for EBOOT.PBP) ──────── */
/*PSP_MODULE_INFO("PixelDungeonPSP", PSP_MODULE_USER, 1, 0);*/
/*PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);*/
PSP_HEAP_SIZE_KB(16384); // 16 MB safe


/* ── Asset path ───────────────────────────────────────── */
/* PPSSPP and real PSP both resolve paths relative to the
   folder that contains EBOOT.PBP — so assets/ must sit
   next to the EBOOT, NOT inside the build/ subdirectory.
   The CMakeLists custom command below copies it there.    */
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
    /* Generate dungeon */
    static Dungeon dungeon;
    dungeon_init(&dungeon);

    Level *lvl = dungeon_current_level(&dungeon);
    Fog *fog = dungeon_current_fog(&dungeon);

    Player player;
    player_init(&player, lvl);

    Camera cam;
    camera_init(&cam, 0, 0);
    camera_center_on(&cam, player.x, player.y);

    /* render_init always returns 0; check stderr output for load status */
    RenderCtx rctx;
    if (render_init(&rctx, ASSETS_DIR) != 0) {
        fprintf(stderr, "render_init failed (SDL could not start)\n");
        return 1;
    }
 
    InputState inp;
    input_init(&inp);
    
    MoveRepeat mr = {0, 0};
 
    /* ── Game loop ──────────────────────────────────── */
    while (!inp.quit) {
        uint32_t now = SDL_GetTicks();
        input_update(&inp);
 
        if (btn_pressed(&inp, BTN_START)) break;
 
        /* Player movement — d-pad / WASD
           Camera follows automatically on every successful move */
        int moved = 0;

        moved |= move_repeat_update(&mr, &inp, now, &player, lvl, &cam, BTN_UP,    0, -1);
        moved |= move_repeat_update(&mr, &inp, now, &player, lvl, &cam, BTN_DOWN,  0, +1);
        moved |= move_repeat_update(&mr, &inp, now, &player, lvl, &cam, BTN_LEFT, -1,  0);
        moved |= move_repeat_update(&mr, &inp, now, &player, lvl, &cam, BTN_RIGHT,+1,  0);

        if (moved) {
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
        }
        
        fog_update(fog, lvl, player.x, player.y);
        /* Draw: tiles → fog → player → present */
        render_frame(&rctx, lvl, &cam);
        render_shadows(&rctx, lvl, fog, &cam);
        render_fog(&rctx, fog, &cam);
        render_player(&rctx, &player, &cam);
        render_present(&rctx);
        
        SDL_Delay(1);
    }

    render_free(&rctx);
    SDL_Quit();
    return 0;
}