#include <pspkernel.h>
#include <pspdebug.h>
#include <SDL2/SDL.h>
#include <stdio.h>

#include "common.h"
#include "level.h"
#include "camera.h"
#include "render.h"
#include "input.h"
#include "player.h"

/* ── PSP kernel module metadata (required for EBOOT.PBP) ──────── */
/*PSP_MODULE_INFO("PixelDungeonPSP", 0, 1, 0);*/
/*PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);*/
/*PSP_HEAP_SIZE_KB(20480);*/   /* 20 MB — be generous for SDL2 */


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
 
 
static void move_repeat_update(
        MoveRepeat *mr, InputState *inp, uint32_t now,
        Player *p, Level *lvl, Camera *cam,
        Button btn, int dx, int dy) {
 
    if (btn_pressed(inp, btn)) {
        /* Immediate first step */
        if (player_move(p, lvl, dx, dy))
            camera_center_on(cam, p->x, p->y);
        mr->held    = btn;
        mr->next_ms = now + REPEAT_DELAY_MS;
 
    } else if (mr->held == (int)btn && btn_held(inp, btn)
               && now >= mr->next_ms) {
        /* Repeat while held */
        if (player_move(p, lvl, dx, dy))
            camera_center_on(cam, p->x, p->y);
        mr->next_ms = now + REPEAT_RATE_MS;
 
    } else if (!btn_held(inp, btn) && mr->held == (int)btn) {
        mr->held = 0;
    }
}
 
/* ── Main ─────────────────────────────────────────────── */
int main(int argc, char *argv[]) {
    (void)argc; (void)argv;
 
    /* Generate dungeon */
    static Level lvl;
    if (level_generate(&lvl, 0, THEME_SEWERS) != 0) {
        fprintf(stderr, "level_generate failed\n");
        return 1;
    }
    level_print(&lvl);
 
    /* Place player at entrance */
    Player player;
    player_init(&player, &lvl);
 
    /* Camera centred on player */
    Camera cam;
    camera_init(&cam, player.x, player.y);

 
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
        move_repeat_update(&mr, &inp, now, &player, &lvl, &cam, BTN_UP,    0, -1);
        move_repeat_update(&mr, &inp, now, &player, &lvl, &cam, BTN_DOWN,  0, +1);
        move_repeat_update(&mr, &inp, now, &player, &lvl, &cam, BTN_LEFT, -1,  0);
        move_repeat_update(&mr, &inp, now, &player, &lvl, &cam, BTN_RIGHT,+1,  0);
 
        /* Draw: tiles → player → present */
        render_frame(&rctx, &lvl, &cam);
        render_player(&rctx, &player, &cam);
        render_present(&rctx);
 
        SDL_Delay(1);
    }

    render_free(&rctx);
    SDL_Quit();
    return 0;
}