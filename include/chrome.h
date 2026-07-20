#pragma once
#include <SDL2/SDL.h>

typedef struct {
    SDL_Texture *tex;
    SDL_Rect src;
    int left;
    int top;
    int right;
    int bottom;
} NinePatch;

void ninepatch_draw(SDL_Renderer *r, const NinePatch *p, int x, int y, int w, int h);

NinePatch chrome_window(SDL_Texture *tex);
NinePatch chrome_toast(SDL_Texture *tex);
NinePatch chrome_toast_tr(SDL_Texture *tex);
NinePatch chrome_button(SDL_Texture *tex);
NinePatch chrome_tag(SDL_Texture *tex);
NinePatch chrome_scroll(SDL_Texture *tex);
NinePatch chrome_tab_set(SDL_Texture *tex);
NinePatch chrome_tab_selected(SDL_Texture *tex);
NinePatch chrome_tab_unselected(SDL_Texture *tex);