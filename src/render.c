#include "render.h"
#include <SDL2/SDL_image.h>
#include <stdio.h>

/* ── path builder ─────────────────────────────────── */
static void asset_path(char *buf, size_t sz, const char *dir, const char *f) {
    snprintf(buf, sz, "%s/%s", dir, f);
}

/* ── PNG init ─────────────────────────────────────── */
static int ensure_png(void) {
    int got = IMG_Init(IMG_INIT_PNG);
    if (!(got & IMG_INIT_PNG)) {
        fprintf(stderr,"[render] No PNG support (0x%x): %s\n",got,IMG_GetError());
        return 0;
    }
    return 1;
}

/* ── texture loader ───────────────────────────────── */
static SDL_Texture *load_tex(SDL_Renderer *r, const char *dir, const char *f) {
    char path[512];
    asset_path(path, sizeof(path), dir, f);
    SDL_Surface *s = IMG_Load(path);
    if (!s) { fprintf(stderr,"[render] Load '%s': %s\n",path,IMG_GetError()); return NULL; }
    SDL_Texture *t = SDL_CreateTextureFromSurface(r, s);
    SDL_FreeSurface(s);
    if (t) printf("[render] OK  %s\n", path);
    return t;
}

/* ── colour fallback ──────────────────────────────── */
static SDL_Color fallback_color(uint8_t idx) {
    switch (idx) {
        case TILE_CHASM:       return (SDL_Color){0x00,0x00,0x00,0xFF};
        case TILE_EMPTY:       return (SDL_Color){0x66,0x55,0x44,0xFF};
        case TILE_GRASS:       return (SDL_Color){0x33,0x77,0x22,0xFF};
        case TILE_WALL:        return (SDL_Color){0x44,0x44,0x44,0xFF};
        case TILE_WALL_DECO:   return (SDL_Color){0x55,0x55,0x44,0xFF};
        case TILE_DOOR:        return (SDL_Color){0x88,0x55,0x22,0xFF};
        case TILE_OPEN_DOOR:   return (SDL_Color){0xBB,0x88,0x44,0xFF};
        case TILE_LOCKED_DOOR: return (SDL_Color){0xCC,0x33,0x00,0xFF};
        case TILE_ENTRANCE:    return (SDL_Color){0xFF,0xFF,0x00,0xFF};
        case TILE_EXIT:        return (SDL_Color){0x00,0xFF,0xFF,0xFF};
        case TILE_EMBERS:      return (SDL_Color){0xAA,0x55,0x11,0xFF};
        case TILE_HIGH_GRASS:  return (SDL_Color){0x22,0x88,0x22,0xFF};
        case TILE_EMPTY_DECO:  return (SDL_Color){0x77,0x66,0x44,0xFF};
        case TILE_WATER:       return (SDL_Color){0x22,0x44,0xAA,0xFF};
        default:
            if (idx >= TILE_WATER_TILES) return (SDL_Color){0x22,0x44,0xAA,0xFF};
            return (SDL_Color){0x22,0x22,0x22,0xFF};
    }
}

/* ── water shore bitmask — exact Level.java getWaterTile() ─
   NEIGHBOURS4 = {-WIDTH,+1,+WIDTH,-1} = {top,right,bottom,left}
   Bit fires when neighbour is UNSTITCHABLE (wall/solid).         */
static int tile_unstitchable(uint8_t t) {
    return t == TILE_WALL || t == TILE_WALL_DECO || t == TILE_SECRET_DOOR;
}
static int water_sprite(const Level *lvl, int tx, int ty) {
    int top   = tile_unstitchable(get_tile_safe(lvl, tx,   ty-1));
    int right = tile_unstitchable(get_tile_safe(lvl, tx+1, ty  ));
    int bot   = tile_unstitchable(get_tile_safe(lvl, tx,   ty+1));
    int left  = tile_unstitchable(get_tile_safe(lvl, tx-1, ty  ));
    return TILE_WATER_TILES + (top | (right<<1) | (bot<<2) | (left<<3));
}

/* ── render_init ──────────────────────────────────── */
int render_init(RenderCtx *ctx, const char *assets_dir) {
    memset(ctx, 0, sizeof(*ctx));
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr,"[render] SDL_Init: %s\n",SDL_GetError()); return -1;
    }
    SDL_Window *win = SDL_CreateWindow("PSPixel Dungeon",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_W, SCREEN_H, SDL_WINDOW_SHOWN);
    if (!win) { fprintf(stderr,"[render] Window: %s\n",SDL_GetError()); return -1; }
    ctx->renderer = SDL_CreateRenderer(win, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!ctx->renderer) { fprintf(stderr,"[render] Renderer: %s\n",SDL_GetError()); return -1; }

    ctx->active_theme  = THEME_SEWERS;
    ctx->png_available = 0;
    if (!assets_dir) return 0;
    if (!ensure_png()) return 0;

    const char *tile_files[]  = ASSET_TILES_BY_THEME;
    const char *water_files[] = ASSET_WATER_BY_THEME;
    int tl=0, wl=0;
    for (int i=0; i<TILESET_COUNT; i++) {
        ctx->tilesets[i] = load_tex(ctx->renderer, assets_dir, tile_files[i]);
        ctx->water[i]    = load_tex(ctx->renderer, assets_dir, water_files[i]);
        if (ctx->tilesets[i]) tl++;
        if (ctx->water[i])    wl++;
    }
    if (tl == 0) {
        char p[512]; asset_path(p, sizeof(p), assets_dir, tile_files[0]);
        fprintf(stderr,"[render] 0 tilesets. Tried: '%s'\n"
                "         assets/ must sit next to EBOOT.PBP\n", p);
    } else {
        printf("[render] tiles %d/%d  water %d/%d\n",tl,TILESET_COUNT,wl,TILESET_COUNT);
        ctx->png_available = 1;
    }
    return 0;
}

/* ── render_free ──────────────────────────────────── */
void render_free(RenderCtx *ctx) {
    for (int i=0; i<TILESET_COUNT; i++) {
        if (ctx->tilesets[i]) SDL_DestroyTexture(ctx->tilesets[i]);
        if (ctx->water[i])    SDL_DestroyTexture(ctx->water[i]);
    }
    IMG_Quit();
}

/* ── render_tile ──────────────────────────────────── */
void render_tile(RenderCtx *ctx, int sprite_idx, int px, int py) {
    SDL_Rect dst = {px, py, TILE_SIZE, TILE_SIZE};
    SDL_Texture *tex = ctx->png_available
                       ? ctx->tilesets[(int)ctx->active_theme] : NULL;
    if (tex) {
        SDL_Rect src = {
            (sprite_idx % TILESET_COLS) * TILESET_TILE_W,
            (sprite_idx / TILESET_COLS) * TILESET_TILE_H,
            TILESET_TILE_W, TILESET_TILE_H
        };
        SDL_RenderCopy(ctx->renderer, tex, &src, &dst);
    } else {
        SDL_Color c = fallback_color((uint8_t)sprite_idx);
        SDL_SetRenderDrawColor(ctx->renderer, c.r, c.g, c.b, c.a);
        SDL_RenderFillRect(ctx->renderer, &dst);
    }
}

static void render_water_fill(RenderCtx *ctx, int tx, int ty, int px, int py) {
    SDL_Rect dst = {px, py, TILE_SIZE, TILE_SIZE};
    SDL_Texture *tex = ctx->png_available
                       ? ctx->water[(int)ctx->active_theme] : NULL;
    if (tex) {
        SDL_Rect src = {(tx*TILE_SIZE)%32, (ty*TILE_SIZE)%32, TILE_SIZE, TILE_SIZE};
        SDL_RenderCopy(ctx->renderer, tex, &src, &dst);
    } else {
        SDL_SetRenderDrawColor(ctx->renderer, 0x22, 0x44, 0xAA, 0xFF);
        SDL_RenderFillRect(ctx->renderer, &dst);
    }
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
    /* NOTE: does NOT call SDL_RenderPresent — caller does that */
}

/* ── render_player ────────────────────────────────── */
/* Draws a bright marker on the player tile.
   When hero sprites are added, replace with sprite draw. */
void render_player(RenderCtx *ctx, const Player *p, const Camera *cam) {
    Point scr = camera_tile_to_screen(cam, p->x, p->y);

    /* Outer shadow ring */
    SDL_Rect shadow = {scr.x + 1, scr.y + 1, TILE_SIZE - 2, TILE_SIZE - 2};
    SDL_SetRenderDrawColor(ctx->renderer, 0, 0, 0, 180);
    SDL_RenderFillRect(ctx->renderer, &shadow);

    /* Bright yellow player square */
    SDL_Rect body = {scr.x + 3, scr.y + 3, TILE_SIZE - 6, TILE_SIZE - 6};
    SDL_SetRenderDrawColor(ctx->renderer, 0xFF, 0xDD, 0x00, 0xFF);
    SDL_RenderFillRect(ctx->renderer, &body);

    /* White highlight dot */
    SDL_Rect dot = {scr.x + 5, scr.y + 5, 3, 3};
    SDL_SetRenderDrawColor(ctx->renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderFillRect(ctx->renderer, &dot);
    /*ONLY FOR TEST SDL_RenderPresent(ctx->renderer);*/
}

/* ── render_present ───────────────────────────────── */
void render_present(RenderCtx *ctx) {
    SDL_RenderPresent(ctx->renderer);
}