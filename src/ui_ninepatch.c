#include "ui_ninepatch.h"
#include <SDL2/SDL_image.h>
#include <stdio.h>

void ninepatch_draw(SDL_Renderer *r, const NinePatch *p, int x, int y, int w, int h){
    int L = p->left;
    int R = p->right;
    int T = p->top;
    int B = p->bottom;

    SDL_Rect src = p->src;

    SDL_Rect s[9] = {
        {src.x,          src.y,          L,             T},
        {src.x+L,        src.y,          src.w-L-R,     T},
        {src.x+src.w-R,  src.y,          R,             T},

        {src.x,          src.y+T,        L,             src.h-T-B},
        {src.x+L,        src.y+T,        src.w-L-R,     src.h-T-B},
        {src.x+src.w-R,  src.y+T,        R,             src.h-T-B},

        {src.x,          src.y+src.h-B,  L,             B},
        {src.x+L,        src.y+src.h-B,  src.w-L-R,     B},
        {src.x+src.w-R,  src.y+src.h-B,  R,             B},
    };
    SDL_Rect d[9] = {
        {x,          y,          L,          T},
        {x+L,        y,          w-L-R,      T},
        {x+w-R,      y,          R,          T},

        {x,          y+T,        L,          h-T-B},
        {x+L,        y+T,        w-L-R,      h-T-B},
        {x+w-R,      y+T,        R,          h-T-B},

        {x,          y+h-B,      L,          B},
        {x+L,        y+h-B,      w-L-R,      B},
        {x+w-R,      y+h-B,      R,          B},
    };

    for (int i = 0; i < 9; i++)
        SDL_RenderCopy(r, p->tex, &s[i], &d[i]);
}

NinePatch chrome_window(SDL_Texture *tex) {
    return (NinePatch){
        .tex = tex,
        .src = {0, 0, 22, 22},
        .left = 7,
        .top = 7,
        .right = 7,
        .bottom = 7
    };
}

NinePatch chrome_toast(SDL_Texture *tex) {
    return (NinePatch){
        .tex = tex,
        .src = {22, 0, 18, 18},
        .left = 5,
        .top = 5,
        .right = 5,
        .bottom = 5
    };
}

NinePatch chrome_toast_tr(SDL_Texture *tex) {
    return (NinePatch){
        .tex = tex,
        .src = {40, 0, 18, 18},
        .left = 5,
        .top = 5,
        .right = 5,
        .bottom = 5
    };
}

NinePatch chrome_button(SDL_Texture *tex) {
    return (NinePatch){
        .tex = tex,
        .src = {58, 0, 6, 6},
        .left = 2,
        .top = 2,
        .right = 2,
        .bottom = 2
    };
}

NinePatch chrome_tag(SDL_Texture *tex) {
    return (NinePatch){
        .tex = tex,
        .src = {22, 18, 16, 14},
        .left = 3,
        .top = 3,
        .right = 3,
        .bottom = 3
    };
}

NinePatch chrome_scroll(SDL_Texture *tex) {
    return (NinePatch){
        .tex = tex,
        .src = {32, 32, 32, 32},
        .left = 5,
        .top = 11,
        .right = 5,
        .bottom = 11
    };
}

NinePatch chrome_tab_set(SDL_Texture *tex) {
    return (NinePatch){
        .tex = tex,
        .src = {64, 0, 22, 22},
        .left = 7,
        .top = 7,
        .right = 7,
        .bottom = 7
    };
}

NinePatch chrome_tab_selected(SDL_Texture *tex) {
    return (NinePatch){
        .tex = tex,
        .src = {64, 22, 10, 14},
        .left = 4,
        .top = 7,
        .right = 4,
        .bottom = 6
    };
}

NinePatch chrome_tab_unselected(SDL_Texture *tex) {
    return (NinePatch){
        .tex = tex,
        .src = {74, 22, 10, 14},
        .left = 4,
        .top = 7,
        .right = 4,
        .bottom = 6
    };
}

NinePatch status_panel(SDL_Texture *tex) {
    return (NinePatch){
        .tex = tex,
        .src = {0, 0, 128, 64},
        .left = 80, 
        .top = 0,
        .right = 40, 
        .bottom = 0
    };
}