#pragma once
#include <SDL2/SDL.h>
/* ══════════════════════════════════════════════════════════
   BITMAP FONT — font1x.png 
   com.watabou.noosa.BitmapText.Font.LATIN_FULL =
       " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ" +
       "[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\u007F"
   ══════════════════════════════════════════════════════════*/
typedef struct {
    SDL_Texture *tex[3];
    int sheet_h;
} BitmapFont;

typedef struct {
    uint8_t page;
    uint16_t x;
    uint8_t w;
} GlyphInfo;

int font_text_width(const char *s);

int font_draw_text(SDL_Renderer *r, const BitmapFont *f, int x, int y, float scale, SDL_Color color, const char *s);
