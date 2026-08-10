#include "font.h"
#include <SDL2/SDL_image.h>
#include <stdio.h>

/* '!' (33) through DEL/0x7F (127) — see the layout note in font.h
   for how these were measured and identified. Generated straight
   from font1x.png, not hand-typed, so there's no transcription-
   error risk. 95 entries: standard '!'-'~' (94) plus the special
   95th glyph at the end (see FONT_GLYPH_SPECIAL in font.h). */
static const GlyphInfo FONT_GLYPHS[95] = {
    /* '!' */ {  1,  3, 3 },
    /* '"' */ {  1,  7, 5 },
    /* '#' */ {  1, 13, 5 },
    /* '$' */ {  1, 19, 5 },
    /* '%' */ {  1, 25, 5 },
    /* '&' */ {  1, 31, 5 },
    /* '\' */ {  1, 37, 3 },
    /* '(' */ {  1, 41, 4 },
    /* ')' */ {  1, 46, 4 },
    /* '*' */ {  1, 51, 5 },
    /* '+' */ {  1, 57, 5 },
    /* ',' */ {  1, 63, 4 },
    /* '-' */ {  1, 68, 5 },
    /* '.' */ {  1, 74, 3 },
    /* '/' */ {  1, 78, 4 },
    /* '0' */ {  1, 83, 5 },
    /* '1' */ {  1, 89, 4 },
    /* '2' */ {  1, 94, 5 },
    /* '3' */ {  1,100, 5 },
    /* '4' */ {  1,106, 5 },
    /* '5' */ {  1,112, 5 },
    /* '6' */ {  1,118, 5 },
    /* '7' */ {  1,124, 5 },
    /* '8' */ {  1,130, 5 },
    /* '9' */ {  1,136, 5 },
    /* ':' */ {  1,142, 3 },
    /* ';' */ {  1,146, 4 },
    /* '<' */ {  1,151, 5 },
    /* '=' */ {  1,157, 5 },
    /* '>' */ {  1,163, 5 },
    /* '?' */ {  1,169, 5 },
    /* '@' */ {  1,175, 5 },
    /* 'A' */ {  1,181, 5 },
    /* 'B' */ {  1,187, 5 },
    /* 'C' */ {  1,193, 5 },
    /* 'D' */ {  1,199, 5 },
    /* 'E' */ {  1,205, 5 },
    /* 'F' */ {  1,211, 5 },
    /* 'G' */ {  1,217, 5 },
    /* 'H' */ {  1,223, 5 },
    /* 'I' */ {  1,229, 5 },
    /* 'J' */ {  1,235, 5 },
    /* 'K' */ {  1,241, 5 },
    /* 'L' */ {  1,247, 5 },
    /* 'M' */ {  2,  1, 7 },
    /* 'N' */ {  2,  9, 6 },
    /* 'O' */ {  2, 16, 5 },
    /* 'P' */ {  2, 22, 5 },
    /* 'Q' */ {  2, 28, 5 },
    /* 'R' */ {  2, 34, 5 },
    /* 'S' */ {  2, 40, 5 },
    /* 'T' */ {  2, 46, 5 },
    /* 'U' */ {  2, 52, 5 },
    /* 'V' */ {  2, 58, 5 },
    /* 'W' */ {  2, 64, 7 },
    /* 'X' */ {  2, 72, 5 },
    /* 'Y' */ {  2, 78, 5 },
    /* 'Z' */ {  2, 84, 5 },
    /* '[' */ {  2, 90, 4 },
    /* '\' */ {  2, 95, 5 },
    /* ']' */ {  2,101, 4 },
    /* '^' */ {  2,106, 5 },
    /* '_' */ {  2,112, 5 },
    /* '`' */ {  2,118, 4 },
    /* 'a' */ {  2,123, 5 },
    /* 'b' */ {  2,129, 5 },
    /* 'c' */ {  2,135, 5 },
    /* 'd' */ {  2,141, 5 },
    /* 'e' */ {  2,147, 5 },
    /* 'f' */ {  2,153, 5 },
    /* 'g' */ {  2,159, 5 },
    /* 'h' */ {  2,165, 5 },
    /* 'i' */ {  2,171, 3 },
    /* 'j' */ {  2,175, 4 },
    /* 'k' */ {  2,180, 5 },
    /* 'l' */ {  2,186, 5 },
    /* 'm' */ {  2,192, 7 },
    /* 'n' */ {  2,200, 5 },
    /* 'o' */ {  2,206, 5 },
    /* 'p' */ {  2,212, 5 },
    /* 'q' */ {  2,218, 5 },
    /* 'r' */ {  2,224, 5 },
    /* 's' */ {  2,230, 5 },
    /* 't' */ {  2,236, 5 },
    /* 'u' */ {  2,242, 5 },
    /* 'v' */ {  2,248, 5 },
    /* 'w' */ {  3,  1, 7 },
    /* 'x' */ {  3,  9, 5 },
    /* 'y' */ {  3, 15, 5 },
    /* 'z' */ {  3, 21, 5 },
    /* '{' */ {  3, 27, 5 },
    /* '|' */ {  3, 33, 3 },
    /* '}' */ {  3, 37, 5 },
    /* '~' */ {  3, 43, 5 },
    /*DEL/0x7F*/ {  3, 49, 5 },
};

#define FONT_FIRST_CHAR '!'
#define FONT_LAST_CHAR  0x7F
#define FONT_SPACE_WIDTH 3
#define FONT_GLYPH_GAP    1

static const GlyphInfo *glyph_for(char c) {
    unsigned char uc = (unsigned char)c;
    if (uc < FONT_FIRST_CHAR || uc > FONT_LAST_CHAR) return NULL;
    return &FONT_GLYPHS[uc - FONT_FIRST_CHAR];
}


int font_text_width(const char *s) {
    int w = 0;
    for (const char *c = s; *c; c++) {
        if (*c == ' ') { w += FONT_SPACE_WIDTH + FONT_GLYPH_GAP; continue; }
        const GlyphInfo *g = glyph_for(*c);
        if (!g) continue; /* unmapped character: skipped, no advance */
        w += g->w + FONT_GLYPH_GAP;
    }
    return w > 0 ? w - FONT_GLYPH_GAP : 0; /* no trailing gap after the last glyph */
}

int font_draw_text(SDL_Renderer *r, const BitmapFont *f, int x, int y, float scale, const char *s) {
    int cursor = x;
    for (const char *c = s; *c; c++) {
        if (*c == ' ') { cursor += (FONT_SPACE_WIDTH + FONT_GLYPH_GAP) * scale; continue; }
        const GlyphInfo *g = glyph_for(*c);
        if (!g) continue;

        SDL_Rect src = { g->x, 0, g->w, f->sheet_h };
        SDL_Rect dst = { cursor, y, g->w * scale, f->sheet_h * scale };
        
        int page = g->page - 1;

        if (page >= 0 && page < 3 && f->tex[page]) {
            SDL_RenderCopy(r, f->tex[page], &src, &dst);
        }

        cursor += (g->w) * scale;
    }
    return font_text_width(s) * scale;
}