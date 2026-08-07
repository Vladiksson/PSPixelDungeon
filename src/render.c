#include "render.h"
#include "mob.h" //redo temporar solution for the test
#include "assets_embedded.h"
#include <SDL2/SDL_image.h>
#include <stdio.h>

/* ── PNG init ─────────────────────────────────────── */
static int ensure_png(void) {
    return (IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != 0;
}

static SDL_Texture *load_tex(SDL_Renderer *r, const char *f) {
    EmbeddedAsset a = asset_find(f);
    if (!a.data)
        return NULL;
    SDL_RWops *rw = SDL_RWFromConstMem(a.data, (int)a.size);
    if (!rw)
        return NULL;
    SDL_Surface *s = IMG_Load_RW(rw, 1); /* 1 = SDL_image closes rw for us */
    if (!s) 
        return NULL;
    SDL_Texture *t = SDL_CreateTextureFromSurface(r, s);
    SDL_FreeSurface(s);
    if (!t)
        return NULL;
    SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND);
    return t;
}

/* ── render_init ──────────────────────────────────── */
int render_init(RenderCtx *ctx) {
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

    if (!ensure_png()) return -1;
    
    /*tiles and water*/
    const char *tile_files[]  = ASSET_TILES_BY_THEME;
    const char *water_files[] = ASSET_WATER_BY_THEME;

    for (int i=0; i<TILESET_COUNT; i++) {
        ctx->tilesets[i] = load_tex(ctx->renderer, tile_files[i]);
        ctx->water[i]    = load_tex(ctx->renderer, water_files[i]);
    }

    /*hero*/
    const char *hero_files[] = ASSET_HERO_BY_CLASS;
    for (int i=0; i<HERO_CLASS_COUNT; i++) {
        ctx->heroes[i] = load_tex(ctx->renderer, hero_files[i]);
    }
    /*items*/
    ctx->items = load_tex(ctx->renderer, ASSET_ITEMS);
    /*fonts*/
    memset(&ctx->font, 0, sizeof(ctx->font));
    ctx->font.sheet_h = 8;

    const char *font_files[] = ASSET_FONTS1X_BY_PAGE;
    for (int i = 0; i < 3; i++) {
        ctx->font.tex[i] = load_tex(ctx->renderer, font_files[i]);
        if (!ctx->font.tex[i]) {
            return -1;
        }
    }
    /*chrome ui*/
    ctx->chrome = load_tex(ctx->renderer, ASSET_CHROME);
    /*icons*/
    ctx->icons = load_tex(ctx->renderer, ASSET_ICONS);

    return 0;
}

/* ── render_free ──────────────────────────────────── */
void render_free(RenderCtx *ctx) {
    for (int i = 0; i < 3; i++) {
        if (ctx->font.tex[i]) {
            SDL_DestroyTexture(ctx->font.tex[i]);
            ctx->font.tex[i] = NULL;
        }
    }
    memset(&ctx->font, 0, sizeof(ctx->font));

    for (int i = 0; i < TILESET_COUNT; i++) {
        if (ctx->tilesets[i]) {
            SDL_DestroyTexture(ctx->tilesets[i]);
            ctx->tilesets[i] = NULL;
        }
        if (ctx->water[i]) {
            SDL_DestroyTexture(ctx->water[i]);
            ctx->water[i] = NULL;
        }
    }
    for (int i = 0; i < HERO_CLASS_COUNT; i++) {
        if (ctx->heroes[i]) {
            SDL_DestroyTexture(ctx->heroes[i]);
            ctx->heroes[i] = NULL;
        }
    }
    if (ctx->items) {
        SDL_DestroyTexture(ctx->items);
        ctx->items = NULL;
    }
    if (ctx->chrome) {
        SDL_DestroyTexture(ctx->chrome);
        ctx->chrome = NULL;
    }
    if (ctx->icons) {
        SDL_DestroyTexture(ctx->icons);
        ctx->icons = NULL;
    }
    if (ctx->renderer) {
        SDL_DestroyRenderer(ctx->renderer);
        ctx->renderer = NULL;
    }
    if (ctx->window) {
        SDL_DestroyWindow(ctx->window);
        ctx->window = NULL;
    }
}

/* ── render_frame ──────────────────────  */
static void render_tile(RenderCtx *ctx, int sprite_idx, int px, int py) {
    SDL_Rect dst = {px, py, gTileSize, gTileSize};
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

static int tile_unstitchable(uint8_t t){
    return (gTileFlags[t] & TF_UNSTITCHABLE) != 0;
}
static int water_sprite(const Level *lvl, int tx, int ty) { //mirror Level.java getWaterTile()
    int top   = tile_unstitchable(get_tile_safe(lvl, tx,   ty-1));
    int right = tile_unstitchable(get_tile_safe(lvl, tx+1, ty  ));
    int bot   = tile_unstitchable(get_tile_safe(lvl, tx,   ty+1));
    int left  = tile_unstitchable(get_tile_safe(lvl, tx-1, ty  ));
    return TILE_WATER_TILES + (top | (right<<1) | (bot<<2) | (left<<3));
}

static void render_water_fill(RenderCtx *ctx, int tx, int ty, int px, int py) {
    SDL_Rect dst = {px, py, gTileSize, gTileSize};
    SDL_Texture *tex = ctx->water[(int)ctx->active_theme];
    if (!tex) 
        return;

    SDL_Rect src = {(tx*gTileSize)%32, (ty*gTileSize)%32, gTileSize, gTileSize};
    SDL_RenderCopy(ctx->renderer, tex, &src, &dst);
}

void render_frame(RenderCtx *ctx, const Level *lvl, const Camera *cam) {
    ctx->active_theme = lvl->theme;
    SDL_SetRenderDrawColor(ctx->renderer, 0, 0, 0, 255);
    SDL_RenderClear(ctx->renderer);

    for (int ty = cam->tile_y; ty <= cam->tile_y + gViewRows; ty++) {
        for (int tx = cam->tile_x; tx <= cam->tile_x + gViewCols; tx++) {
            if (!in_bounds(tx, ty)) continue;
            uint8_t tile = lvl->map[xy_to_idx(tx, ty)];
            Point   scr  = camera_tile_to_screen(cam, tx, ty);
            if (tile == TILE_CHASM_WATER) {
                render_water_fill(ctx, tx, ty, scr.x, scr.y);
                render_tile(ctx, TILE_CHASM_WATER, scr.x, scr.y);
            } else if (tile == TILE_WATER) {
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
        if (h->x < cam->tile_x || h->x > cam->tile_x + gViewCols ||
            h->y < cam->tile_y || h->y > cam->tile_y + gViewRows) continue;
 
        uint8_t col, row;
        if (!heap_sprite_cell(h, &col, &row)) continue;
 
        Point scr = camera_tile_to_screen(cam, h->x, h->y);
 
        SDL_Rect src = { col * ICON_SIZE, row * ICON_SIZE, ICON_SIZE, ICON_SIZE };
        SDL_Rect dst = { scr.x, scr.y, gTileSize, gTileSize };
 
        SDL_RenderCopy(ctx->renderer, tex, &src, &dst);
    }
}

/* ── render_shadow ──────────────────────────────────── */
static int wall_at(const Level *lvl, int nx, int ny) {
    return in_bounds(nx, ny) && ((gTileFlags[lvl->map[xy_to_idx(nx, ny)]] & TF_SHADOW) != 0);
}

static void edge_shadow(SDL_Renderer *r, int px, int py, int axis, int flip, int max_a) {
    const int STEPS = 5;
    int span  = gTileSize / 4;
    int chunk = span / STEPS;
    if (chunk < 1) chunk = 1;

    for (int i = 0; i < STEPS; i++) {
        int a = max_a * (STEPS - i) / STEPS;
        if (a <= 0) continue;
        SDL_SetRenderDrawColor(r, 0, 0, 0, a);

        int off = i * chunk;
        SDL_Rect rect;
        if (axis == 0) {
            int y = flip ? (py + gTileSize - off - chunk) : (py + off);
            rect = (SDL_Rect){ px, y, gTileSize, chunk };
        } else {
            int x = flip ? (px + gTileSize - off - chunk) : (px + off);
            rect = (SDL_Rect){ x, py, chunk, gTileSize };
        }
        SDL_RenderFillRect(r, &rect);
    }
}

void render_shadows(RenderCtx *ctx, const Level *lvl, const Fog *fog, const Camera *cam) {
    SDL_SetRenderDrawBlendMode(ctx->renderer, SDL_BLENDMODE_BLEND);

    for (int ty = cam->tile_y; ty <= cam->tile_y + gViewRows; ty++) {
        for (int tx = cam->tile_x; tx <= cam->tile_x + gViewCols; tx++) {
            if (!in_bounds(tx, ty)) continue;
            int idx = xy_to_idx(tx, ty);

            if (!fog->visited[idx]) continue;
            if (!(gTileFlags[lvl->map[idx]] & TF_RECEIVES_SHADOW)) continue;

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
            int half = gTileSize / 2;

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
                    &(SDL_Rect){ scr.x, scr.y, gTileSize, gTileSize });
            }
        }
    }

    SDL_SetRenderDrawBlendMode(ctx->renderer, SDL_BLENDMODE_NONE);
}

/* ── render_fog ────────────────────────────────── */
void render_fog(RenderCtx *ctx, const Fog *fog, const Camera *cam) {
    SDL_SetRenderDrawBlendMode(ctx->renderer, SDL_BLENDMODE_BLEND);

    for (int ty = cam->tile_y; ty <= cam->tile_y + gViewRows; ty++) {
        for (int tx = cam->tile_x; tx <= cam->tile_x + gViewCols; tx++) {
            if (!in_bounds(tx, ty)) continue;

            int idx = xy_to_idx(tx, ty);

            /* Fully visible — no overlay needed */
            if (fog->visible[idx]) continue;

            Point    scr  = camera_tile_to_screen(cam, tx, ty);
            SDL_Rect rect = { scr.x, scr.y, gTileSize, gTileSize };

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
    SDL_SetRenderDrawBlendMode(ctx->renderer, SDL_BLENDMODE_NONE);
}

 
/* ── render_player ────────────────────────────────── */
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

void render_player(RenderCtx *ctx, const Player *p, const Camera *cam){
    SDL_Texture *tex = ctx->heroes[p->cls];
    if (!tex) return;
    
    Point pos = idx_to_xy(p->base.pos);
    Point scr = camera_tile_to_screen(cam, pos.x, pos.y);

    SDL_Rect dst = {
        scr.x - (gHeroW - gTileSize) / 2,
        scr.y - (gHeroH - gTileSize),
        gHeroW,
        gHeroH
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
        if (pos.x < cam->tile_x || pos.x > cam->tile_x + gViewCols ||
            pos.y < cam->tile_y || pos.y > cam->tile_y + gViewRows)
            continue;

        Point scr = camera_tile_to_screen(cam, pos.x, pos.y);

        /* Outer shadow ring */
        SDL_Rect shadow = { scr.x + 1, scr.y + 1, gTileSize - 2, gTileSize - 2 };
        SDL_SetRenderDrawColor(ctx->renderer, 0, 0, 0, 180);
        SDL_RenderFillRect(ctx->renderer, &shadow);

        /* Placeholder body */
        SDL_Rect body = { scr.x + 3, scr.y + 3, gTileSize - 6, gTileSize - 6 };

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