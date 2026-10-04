#include <pspkernel.h>
#include <pspdebug.h>
#include <pspsysmem.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>

#include "game.h"

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;

    static Game game;
    game_init(&game);

    RenderCtx rctx;
    if (render_init(&rctx) != 0) {
        SDL_Quit();
        return -1;
    }

    InputState inp;
    input_init(&inp);

    uint32_t last = SDL_GetTicks();

    while (!inp.quit) {
        uint32_t now = SDL_GetTicks();
        float dt = (now - last) / 1000.0f;
        last = now;
        if (dt > 0.1f) 
            dt = 0.1f;

        input_update(&inp);

        game_input(&game, &inp, now);
        game_update(&game, &rctx, dt);
        game_render(&game, &rctx);

        SDL_Delay(1);
    }

    render_free(&rctx);
    IMG_Quit();
    SDL_Quit();
    return 0;
}