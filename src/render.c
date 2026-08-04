#include "render.h"
#include "mob.h" //redo temporar solution for the test
#include <SDL2/SDL_image.h>
#include <stdio.h>

/* ── path builder ─────────────────────────────────── */
static void asset_path(char *buf, size_t sz, const char *dir, const char *f) {
    snprintf(buf, sz, "%s/%s", dir, f);
}

/* ── PNG init ─────────────────────────────────────── */
static int ensure_png(void) {
    int got = IMG_Init(IMG_INIT_PNG);
    if (!(got & IMG_INIT_PNG)) { return 0;}
    return 1;
}

/* ── texture loader ───────────────────────────────── */
static SDL_Texture *load_tex(SDL_Renderer *r, const char *dir, const char *f) {
    char path[512];
    asset_path(path, sizeof(path), dir, f);
    SDL_Surface *s = IMG_Load(path);
    if (!s) 
        return NULL;
    SDL_Texture *t = SDL_CreateTextureFromSurface(r, s);
    SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND);
    SDL_FreeSurface(s);
    return t;
}

/* ── water shore bitmask — exact Level.java getWaterTile() ─
   NEIGHBOURS4 = {-WIDTH,+1,+WIDTH,-1} = {top,right,bottom,left}
   Bit fires when neighbour is UNSTITCHABLE (wall/solid).         */
static int tile_unstitchable(uint8_t t){
    return (gTileFlags[t] & TF_UNSTITCHABLE) != 0;
}
static int water_sprite(const Level *lvl, int tx, int ty) {
    int top   = tile_unstitchable(get_tile_safe(lvl, tx,   ty-1));
    int right = tile_unstitchable(get_tile_safe(lvl, tx+1, ty  ));
    int bot   = tile_unstitchable(get_tile_safe(lvl, tx,   ty+1));
    int left  = tile_unstitchable(get_tile_safe(lvl, tx-1, ty  ));
    return TILE_WATER_TILES + (top | (right<<1) | (bot<<2) | (left<<3));
}

/* ── render_init ──────────────────────────────────── */
static int init_font(BitmapFont *font, SDL_Renderer *renderer, const char *dir){
    memset(font, 0, sizeof(*font));
    font->sheet_h = 8;

    const char *font_files[] = ASSET_FONTS1X_BY_PAGE;
    for (int i = 0; i < 3; i++) {
        font->tex[i] = load_tex(renderer, dir, font_files[i]);
        if (!font->tex[i])
            return 0;
    }
    return 1;
}

int render_init(RenderCtx *ctx, const char *assets_dir) {
    memset(ctx, 0, sizeof(*ctx));
    if (SDL_Init(SDL_INIT_VIDEO) != 0) { 
        return -1; 
    }
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0"); // nearest filtering

    ctx->window = SDL_CreateWindow(
        "PSPixel Dungeon",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_W,
        SCREEN_H,
        SDL_WINDOW_SHOWN);

    if (!ctx->window) { 
        return -1; 
    }
    ctx->renderer = SDL_CreateRenderer(
        ctx->window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!ctx->renderer) {
        SDL_DestroyWindow(ctx->window);
        ctx->window = NULL;
        return -1;
    }

    if (!assets_dir) return -1;
    if (!ensure_png()) return -1;
    
    /*tiles and water*/
    const char *tile_files[]  = ASSET_TILES_BY_THEME;
    const char *water_files[] = ASSET_WATER_BY_THEME;

    for (int i=0; i<TILESET_COUNT; i++) {
        ctx->tilesets[i] = load_tex(ctx->renderer, assets_dir, tile_files[i]);
        ctx->water[i]    = load_tex(ctx->renderer, assets_dir, water_files[i]);
    }

    /*hero*/
    const char *hero_files[] = ASSET_HERO_BY_CLASS;
    for (int i=0; i<HERO_CLASS_COUNT; i++) {
        ctx->heroes[i] = load_tex(ctx->renderer, assets_dir, hero_files[i]);
    }
    /*items*/
    ctx->items = load_tex(ctx->renderer, assets_dir, ASSET_ITEMS);
    /*fonts*/
    init_font(&ctx->font, ctx->renderer, assets_dir);
    /*chrome ui*/
    ctx->chrome = load_tex(ctx->renderer, assets_dir, ASSET_CHROME);
    /*icons*/
    ctx->icons = load_tex(ctx->renderer, assets_dir, ASSET_ICONS);

    return 0;
}

/* ── render_free ──────────────────────────────────── */
static void font_free(BitmapFont *font) {
    for (int i = 0; i < 3; i++) {
        if (font->tex[i])
            SDL_DestroyTexture(font->tex[i]);
    }

    memset(font, 0, sizeof(*font));
}

void render_free(RenderCtx *ctx) {
    for (int i=0; i<TILESET_COUNT; i++) {
        if (ctx->tilesets[i]) SDL_DestroyTexture(ctx->tilesets[i]);
        if (ctx->water[i])    SDL_DestroyTexture(ctx->water[i]);
    }
    for (int i=0; i<HERO_CLASS_COUNT; i++) {
        if (ctx->heroes[i]) SDL_DestroyTexture(ctx->heroes[i]);
    }
    if (ctx->items) SDL_DestroyTexture(ctx->items);

    font_free(&ctx->font);

    if (ctx->chrome) SDL_DestroyTexture(ctx->chrome);
    if (ctx->icons) SDL_DestroyTexture(ctx->icons);

    if (ctx->renderer) {
        SDL_DestroyRenderer(ctx->renderer);
        ctx->renderer = NULL;
    }

    if (ctx->window) {
        SDL_DestroyWindow(ctx->window);
        ctx->window = NULL;
    }

    IMG_Quit();
    SDL_Quit();
}

/* ── render_tile ──────────────────────────────────── */
void render_tile(RenderCtx *ctx, int sprite_idx, int px, int py) {
    SDL_Rect dst = {px, py, TILE_SIZE, TILE_SIZE};
    SDL_Texture *tex = ctx->tilesets[(int)ctx->active_theme];
    if (!tex)
        return;

    SDL_Rect src = {
        (sprite_idx % TILESET_COLS) * TILESET_TILE_W,
        (sprite_idx / TILESET_COLS) * TILESET_TILE_H,
        TILESET_TILE_W, TILESET_TILE_H
    };
    SDL_RenderCopy(ctx->renderer, tex, &src, &dst);
}

static void render_water_fill(RenderCtx *ctx, int tx, int ty, int px, int py) {
    SDL_Rect dst = {px, py, TILE_SIZE, TILE_SIZE};
    SDL_Texture *tex = ctx->water[(int)ctx->active_theme];
    if (!tex) 
        return;

    SDL_Rect src = {(tx*TILE_SIZE)%32, (ty*TILE_SIZE)%32, TILE_SIZE, TILE_SIZE};
    SDL_RenderCopy(ctx->renderer, tex, &src, &dst);
}

/* ── render_frame — draws tiles, does NOT present ── */
void render_frame(RenderCtx *ctx, const Level *lvl, const Camera *cam) {
    ctx->active_theme = lvl->theme;
    SDL_SetRenderDrawColor(ctx->renderer, 0, 0, 0, 255);
    SDL_RenderClear(ctx->renderer);

    for (int ty = cam->tile_y; ty <= cam->tile_y + VIEW_ROWS; ty++) {
        for (int tx = cam->tile_x; tx <= cam->tile_x + VIEW_COLS; tx++) {
            if (!in_bounds(tx, ty)) continue;
            uint8_t tile = lvl->map[xy_to_idx(tx, ty)];
            Point   scr  = camera_tile_to_screen(cam, tx, ty);
            if (tile == TILE_WATER) {
                render_water_fill(ctx, tx, ty, scr.x, scr.y);
                render_tile(ctx, water_sprite(lvl, tx, ty), scr.x, scr.y);
            } else {
                render_tile(ctx, (int)tile, scr.x, scr.y);
            }
        }
    }
}

/* ── render_items ──────────────────────────────────── */
void render_items(RenderCtx *ctx, const Level *lvl, const Camera *cam) {
    SDL_Texture *tex = ctx->items;
    if (!tex) return;
 
    for (int i = 0; i < lvl->num_heaps; i++) {
        const Heap *h = &lvl->heaps[i];
        if (!h->active) continue;
        if (h->x < cam->tile_x || h->x > cam->tile_x + VIEW_COLS ||
            h->y < cam->tile_y || h->y > cam->tile_y + VIEW_ROWS) continue;
 
        uint8_t col, row;
        if (!heap_sprite_cell(h, &col, &row)) continue;
 
        Point scr = camera_tile_to_screen(cam, h->x, h->y);
 
        SDL_Rect src = { col * TILE_SIZE, row * TILE_SIZE, TILE_SIZE, TILE_SIZE };
        SDL_Rect dst = { scr.x, scr.y, TILE_SIZE, TILE_SIZE };
 
        SDL_RenderCopy(ctx->renderer, tex, &src, &dst);
    }
}

/* Neighbour helper — returns 1 if tile (nx,ny) is a wall, 0 otherwise */
static int wall_at(const Level *lvl, int nx, int ny) {
    return in_bounds(nx, ny) && ((gTileFlags[lvl->map[xy_to_idx(nx, ny)]] & TF_SHADOW) != 0);
}

/* ── Stepped gradient shadow ──────────────────────────────────
   Draws STEPS rectangles of decreasing alpha along one edge.

   axis    0 = horizontal strip  (N shadow from top / S from bottom)
           1 = vertical   strip  (W shadow from left / E from right)
   flip    0 = start from the near edge  (N/W walls)
           1 = start from the far  edge  (S/E walls)
   max_a   alpha at the wall face (fades to 0 over TILE_SIZE*5/8 px)
   ─────────────────────────────────────────────────────────────── */
static void edge_shadow(SDL_Renderer *r, int px, int py, int axis, int flip, int max_a) {
    const int STEPS = 5;
    int span  = TILE_SIZE / 4;
    int chunk = span / STEPS;
    if (chunk < 1) chunk = 1;

    for (int i = 0; i < STEPS; i++) {
        int a = max_a * (STEPS - i) / STEPS;
        if (a <= 0) continue;
        SDL_SetRenderDrawColor(r, 0, 0, 0, a);

        int off = i * chunk;
        SDL_Rect rect;
        if (axis == 0) {
            int y = flip ? (py + TILE_SIZE - off - chunk) : (py + off);
            rect = (SDL_Rect){ px, y, TILE_SIZE, chunk };
        } else {
            int x = flip ? (px + TILE_SIZE - off - chunk) : (px + off);
            rect = (SDL_Rect){ x, py, chunk, TILE_SIZE };
        }
        SDL_RenderFillRect(r, &rect);
    }
}

void render_shadows(RenderCtx *ctx, const Level *lvl, const Fog *fog, const Camera *cam) {
    SDL_SetRenderDrawBlendMode(ctx->renderer, SDL_BLENDMODE_BLEND);

    for (int ty = cam->tile_y; ty <= cam->tile_y + VIEW_ROWS; ty++) {
        for (int tx = cam->tile_x; tx <= cam->tile_x + VIEW_COLS; tx++) {
            if (!in_bounds(tx, ty)) continue;
            int idx = xy_to_idx(tx, ty);

            /* Only shadow tiles the player has at least visited */
            if (!fog->visited[idx]) continue;

            /* Only passable (floor-like) tiles get the shadow overlay */
            if (!(gTileFlags[lvl->map[idx]] & TF_PASSABLE)) continue;

            Point scr = camera_tile_to_screen(cam, tx, ty);

            /* ── Cardinal neighbours ──────────────────────────── */
            int wN = wall_at(lvl, tx,   ty-1);
            int wS = wall_at(lvl, tx,   ty+1);
            int wW = wall_at(lvl, tx-1, ty  );
            int wE = wall_at(lvl, tx+1, ty  );

            /* ── Diagonal neighbours (for corner AO) ─────────── */
            int wNW = wall_at(lvl, tx-1, ty-1);
            int wNE = wall_at(lvl, tx+1, ty-1);
            int wSW = wall_at(lvl, tx-1, ty+1);
            int wSE = wall_at(lvl, tx+1, ty+1);

            /* ── Directional shadows ──────────────────────────── */
            /*    axis 0 = horiz strip, axis 1 = vert strip        */
            /*    flip 0 = from near edge, flip 1 = from far edge  */
            if (wN) edge_shadow(ctx->renderer, scr.x, scr.y, 0, 0, 200); /* N strongest */
            if (wW) edge_shadow(ctx->renderer, scr.x, scr.y, 1, 0, 130); /* W medium    */
            if (wE) edge_shadow(ctx->renderer, scr.x, scr.y, 1, 1,  85); /* E weaker    */
            if (wS) edge_shadow(ctx->renderer, scr.x, scr.y, 0, 1,  45); /* S subtle    */

            /* ── Diagonal corner AO ───────────────────────────── */
            /* A diagonal wall with no matching cardinal wall means
               this tile sits at an "inside corner" — darken that
               quadrant to simulate AO in the tight corner.         */
            int half = TILE_SIZE / 2;

            if (wNW && !wN && !wW) {  /* inside NW corner */
                SDL_SetRenderDrawColor(ctx->renderer, 0, 0, 0, 70);
                SDL_RenderFillRect(ctx->renderer,
                    &(SDL_Rect){ scr.x,      scr.y,      half, half });
            }
            if (wNE && !wN && !wE) {  /* inside NE corner */
                SDL_SetRenderDrawColor(ctx->renderer, 0, 0, 0, 70);
                SDL_RenderFillRect(ctx->renderer,
                    &(SDL_Rect){ scr.x+half, scr.y,      half, half });
            }
            if (wSW && !wS && !wW) {  /* inside SW corner */
                SDL_SetRenderDrawColor(ctx->renderer, 0, 0, 0, 45);
                SDL_RenderFillRect(ctx->renderer,
                    &(SDL_Rect){ scr.x,      scr.y+half, half, half });
            }
            if (wSE && !wS && !wE) {  /* inside SE corner */
                SDL_SetRenderDrawColor(ctx->renderer, 0, 0, 0, 45);
                SDL_RenderFillRect(ctx->renderer,
                    &(SDL_Rect){ scr.x+half, scr.y+half, half, half });
            }

            /* ── Enclosure AO ─────────────────────────────────── */
            /* Subtle uniform darkening: the more walls surround a
               tile, the more "enclosed" it feels. Cardinal walls
               contribute more than diagonals.                      */
            int enclosure = wN*3 + wW*2 + wE*2 + wS
                          + wNW + wNE + wSW + wSE;
            if (enclosure > 0) {
                int a = enclosure * 6;
                if (a > 72) a = 72;
                SDL_SetRenderDrawColor(ctx->renderer, 0, 0, 0, a);
                SDL_RenderFillRect(ctx->renderer,
                    &(SDL_Rect){ scr.x, scr.y, TILE_SIZE, TILE_SIZE });
            }
        }
    }

    SDL_SetRenderDrawBlendMode(ctx->renderer, SDL_BLENDMODE_NONE);
}

/* ── render_fog ────────────────────────────────── */
void render_fog(RenderCtx *ctx, const Fog *fog, const Camera *cam) {
    SDL_SetRenderDrawBlendMode(ctx->renderer, SDL_BLENDMODE_BLEND);

    for (int ty = cam->tile_y; ty <= cam->tile_y + VIEW_ROWS; ty++) {
        for (int tx = cam->tile_x; tx <= cam->tile_x + VIEW_COLS; tx++) {
            if (!in_bounds(tx, ty)) continue;

            int idx = xy_to_idx(tx, ty);

            /* Fully visible — no overlay needed */
            if (fog->visible[idx]) continue;

            Point    scr  = camera_tile_to_screen(cam, tx, ty);
            SDL_Rect rect = { scr.x, scr.y, TILE_SIZE, TILE_SIZE };

            if (fog->visited[idx]) {
                /* VISITED = 0xCC111111 (ARGB) */
                SDL_SetRenderDrawColor(ctx->renderer, 0x11, 0x11, 0x11, 0xCC);
            } else {
                /* INVISIBLE = 0xFF000000 — solid black */
                SDL_SetRenderDrawColor(ctx->renderer, 0x00, 0x00, 0x00, 0xFF);
            }

            SDL_RenderFillRect(ctx->renderer, &rect);
        }
    }

    /* Restore opaque drawing for everything that follows */
    SDL_SetRenderDrawBlendMode(ctx->renderer, SDL_BLENDMODE_NONE);
}

 
/* ── hero source-rect lookup ──────────────────────────
   Column = which frame of the current animation. Row = armor tier. */
static SDL_Rect hero_frame_rect(const Player *p) {
    const AnimClip *clip = &HERO_CLIPS[p->anim];
    uint8_t col = clip->frames[p->frame % clip->count];
    SDL_Rect src = {
        col * TILESET_HERO_W,
        HERO_ARMOR_TIER * TILESET_HERO_H,
        TILESET_HERO_W,
        TILESET_HERO_H
    };
    return src;
}

/* ── render_player ────────────────────────────────── */
void render_player(RenderCtx *ctx, const Player *p, const Camera *cam){
    SDL_Texture *tex = ctx->heroes[p->cls];
    if (!tex) return;
    
    Point pos = idx_to_xy(p->base.pos);
    Point scr = camera_tile_to_screen(cam, pos.x, pos.y);

    SDL_Rect dst = {
        scr.x - (HERO_W - TILE_SIZE) / 2,
        scr.y - (HERO_H - TILE_SIZE),
        HERO_W,
        HERO_H
    };

    SDL_Rect src = hero_frame_rect(p);
    SDL_RendererFlip flip = (p->facing == FACE_LEFT) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    SDL_RenderCopyEx(ctx->renderer, tex, &src, &dst, 0.0, NULL, flip);
}





/* ── render_mobs  REDO TEMPORAR BULLSHIT───────────────────── */
void render_mobs(RenderCtx *ctx, const Camera *cam) {
    for (int i = 0; i < mob_count(); i++) {
        Mob *m = mob_at_index(i);
        if (!m)
            continue;

        if (!char_is_alive(&m->base))
            continue;

        Point pos = idx_to_xy(m->base.pos);

        /* outside camera */
        if (pos.x < cam->tile_x || pos.x > cam->tile_x + VIEW_COLS ||
            pos.y < cam->tile_y || pos.y > cam->tile_y + VIEW_ROWS)
            continue;

        Point scr = camera_tile_to_screen(cam, pos.x, pos.y);

        /* Outer shadow ring */
        SDL_Rect shadow = { scr.x + 1, scr.y + 1, TILE_SIZE - 2, TILE_SIZE - 2 };
        SDL_SetRenderDrawColor(ctx->renderer, 0, 0, 0, 180);
        SDL_RenderFillRect(ctx->renderer, &shadow);

        /* Placeholder body */
        SDL_Rect body = { scr.x + 3, scr.y + 3, TILE_SIZE - 6, TILE_SIZE - 6 };

        switch (m->kind) {
        case MOB_RAT:
            SDL_SetRenderDrawColor(ctx->renderer, 160, 160, 160, 255);
            break;

        case MOB_GNOLL:
            SDL_SetRenderDrawColor(ctx->renderer, 180, 120, 40, 255);
            break;

        default:
            SDL_SetRenderDrawColor(ctx->renderer, 255, 0, 255, 255);
            break;
        }

        SDL_RenderFillRect(ctx->renderer, &body);

        /* White highlight */
        SDL_Rect dot = { scr.x + 5, scr.y + 5, 3, 3 };
        SDL_SetRenderDrawColor(ctx->renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(ctx->renderer, &dot);
    }
}






/* ── render_present ───────────────────────────────── */
void render_present(RenderCtx *ctx) {
    SDL_RenderPresent(ctx->renderer);
}