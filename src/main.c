#include <pspkernel.h>
#include <pspdebug.h>
#include <pspsysmem.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>

#include "game.h"

typedef enum {
    APP_MENU,
    APP_HERO_SELECT,
    APP_GAME,
} AppState;

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;

    RenderCtx rctx;
    if (render_init(&rctx) != 0) {
        SDL_Quit();
        return -1;
    }

    InputState inp;
    input_init(&inp);

    static Game game;

    AppState app_state = APP_MENU;
    HeroClass hero;
    int hero_selected = 0;

    uint32_t last = SDL_GetTicks();

    while (!inp.quit) {
        uint32_t now = SDL_GetTicks();
        float dt = (now - last) / 1000.0f;
        last = now;
        if (dt > 0.1f) 
            dt = 0.1f;

        input_update(&inp);

        switch (app_state) {
            case APP_MENU:
                render_menu_update(&rctx, dt, false);
                render_menu(&rctx);

                render_banner_update(&rctx, dt);

                SDL_Rect dst = { (SCREEN_W - 128*2) / 2, 20, 128*2, 70*2 };
                render_banner(&rctx, BANNER_PIXEL_DUNGEON, &dst);

                SDL_Rect signs = dst;
                signs.h = 57*2;
                render_banner_glow(&rctx, &signs);

                if (btn_pressed(&inp, BTN_START)) {
                    app_state = APP_HERO_SELECT;
                    break;
                }
                break;

            case APP_HERO_SELECT: {
                render_menu_update(&rctx, dt, false);
                render_menu(&rctx);

                SDL_Rect dst = { (SCREEN_W - 128*2) / 2, 16, 128*2, 21*2 };
                render_banner(&rctx, BANNER_SELECT_YOUR_HERO, &dst);

                render_avatars(&rctx, 3);

                if (btn_pressed(&inp, BTN_SQUARE)) {
                    hero = HERO_WARRIOR;
                    hero_selected = 1;
                } else if (btn_pressed(&inp, BTN_CROSS)) {
                    hero = HERO_MAGE;
                    hero_selected = 1;
                } else if (btn_pressed(&inp, BTN_CIRCLE)) {
                    hero = HERO_ROGUE;
                    hero_selected = 1;
                } else if (btn_pressed(&inp, BTN_TRIANGLE)) {
                    hero = HERO_HUNTRESS;
                    hero_selected = 1;
                }

                if (hero_selected) {
                    app_state = APP_GAME;
                    game_init(&game, hero);
                    break;
                }
                break;
            }

            case APP_GAME: {
                game_input(&game, &inp, now);
                game_update(&game, &rctx, dt);
                game_render(&game, &rctx);
                break;
            }
        }
        render_present(&rctx);
        SDL_Delay(1);
    }

    render_free(&rctx);
    IMG_Quit();
    SDL_Quit();
    return 0;
}