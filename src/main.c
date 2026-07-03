#include <pspkernel.h>
#include <pspdebug.h>
#include <SDL2/SDL.h>
#include <stdio.h>

#include "common.h"
#include "level.h"
#include "camera.h"
#include "render.h"
#include "input.h"

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
 
/* ── Scroll repeat ────────────────────────────────────── */
#define SCROLL_SPEED      1
#define REPEAT_DELAY_MS 300
#define REPEAT_RATE_MS   80
 
typedef struct { uint32_t next_ms; int active; } RepeatState;
 
static void repeat_update(RepeatState *rs, const InputState *inp,
                          Button btn, uint32_t now,
                          Camera *cam, int dx, int dy) {
    if (btn_pressed(inp, btn)) {
        camera_move(cam, dx * SCROLL_SPEED, dy * SCROLL_SPEED);
        rs->next_ms = now + REPEAT_DELAY_MS;
        rs->active  = 1;
    } else if (rs->active && btn_held(inp, btn) && now >= rs->next_ms) {
        camera_move(cam, dx * SCROLL_SPEED, dy * SCROLL_SPEED);
        rs->next_ms = now + REPEAT_RATE_MS;
    } else if (!btn_held(inp, btn)) {
        rs->active = 0;
    }
}
 
/* ── Main ─────────────────────────────────────────────── */
int main(int argc, char *argv[]) {
    (void)argc; (void)argv;
 
    static Level lvl;
    if (level_generate(&lvl, 0, THEME_SEWERS) != 0) {
        fprintf(stderr, "level_generate failed\n");
        return 1;
    }
    level_print(&lvl);
 
    Point entry = idx_to_xy(lvl.entrance);
    Camera cam;
    camera_init(&cam, entry.x, entry.y);
 
    /* render_init always returns 0; check stderr output for load status */
    RenderCtx rctx;
    if (render_init(&rctx, ASSETS_DIR) != 0) {
        fprintf(stderr, "render_init failed (SDL could not start)\n");
        return 1;
    }
 
    InputState inp;
    input_init(&inp);
    RepeatState reps[4] = {0};
 
    while (!inp.quit) {
        uint32_t now = SDL_GetTicks();
        input_update(&inp);
        if (btn_pressed(&inp, BTN_START)) break;
        repeat_update(&reps[0], &inp, BTN_UP,    now, &cam,  0, -1);
        repeat_update(&reps[1], &inp, BTN_DOWN,  now, &cam,  0, +1);
        repeat_update(&reps[2], &inp, BTN_LEFT,  now, &cam, -1,  0);
        repeat_update(&reps[3], &inp, BTN_RIGHT, now, &cam, +1,  0);
        render_frame(&rctx, &lvl, &cam);
        SDL_Delay(1);
    }
 
    render_free(&rctx);
    SDL_Quit();
    return 0;
}