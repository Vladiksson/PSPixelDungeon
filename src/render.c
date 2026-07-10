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
int render_init(RenderCtx *ctx, const char *assets_dir) {
    memset(ctx, 0, sizeof(*ctx));
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr,"[render] SDL_Init: %s\n",SDL_GetError()); return -1;
    }
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0"); // nearest filtering
    SDL_Window *win = SDL_CreateWindow("PSPixel Dungeon",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_W, SCREEN_H, SDL_WINDOW_SHOWN);
    if (!win) { fprintf(stderr,"[render] Window: %s\n",SDL_GetError()); return -1; }
    ctx->renderer = SDL_CreateRenderer(win, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!ctx->renderer) { fprintf(stderr,"[render] Renderer: %s\n",SDL_GetError()); return -1; }

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
    
    /* One sheet per hero class — indices line up with the HeroClass enum */
    const char *hero_files[] = ASSET_HERO_BY_CLASS;
    int hl = 0;
    for (int i=0; i<HERO_CLASS_COUNT; i++) {
        ctx->heroes[i] = load_tex(ctx->renderer, assets_dir, hero_files[i]);
        if (ctx->heroes[i]) hl++;
    }
    printf("[render] heroes %d/%d\n", hl, HERO_CLASS_COUNT);

    return 0;
}

/* ── render_free ──────────────────────────────────── */
void render_free(RenderCtx *ctx) {
    for (int i=0; i<TILESET_COUNT; i++) {
        if (ctx->tilesets[i]) SDL_DestroyTexture(ctx->tilesets[i]);
        if (ctx->water[i])    SDL_DestroyTexture(ctx->water[i]);
    }
    for (int i=0; i<HERO_CLASS_COUNT; i++) {
        if (ctx->heroes[i]) SDL_DestroyTexture(ctx->heroes[i]);
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
}

static int is_wall(uint8_t t) {
    return t == TILE_WALL || t == TILE_WALL_DECO || t == TILE_SECRET_DOOR; //TODO rewrite by flags return (gTileFlags[t] & TF_LOS_BLOCKING) != 0;
}
/* Neighbour helper — returns 1 if tile (nx,ny) is a wall, 0 otherwise */
static int wall_at(const Level *lvl, int nx, int ny) {
    return in_bounds(nx, ny) && is_wall(lvl->map[xy_to_idx(nx, ny)]);
}

/* ── Stepped gradient shadow ──────────────────────────────────
   Draws STEPS rectangles of decreasing alpha along one edge.

   axis    0 = horizontal strip  (N shadow from top / S from bottom)
           1 = vertical   strip  (W shadow from left / E from right)
   flip    0 = start from the near edge  (N/W walls)
           1 = start from the far  edge  (S/E walls)
   max_a   alpha at the wall face (fades to 0 over TILE_SIZE*5/8 px)
   ─────────────────────────────────────────────────────────────── */
static void edge_shadow(SDL_Renderer *r, int px, int py,
                        int axis, int flip, int max_a) {
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

void render_shadows(RenderCtx *ctx,
                    const Level *lvl, const Fog *fog, const Camera *cam) {
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
                /* VISITED = 0xCC111111 (ARGB)
                   SDL2 colour order is RGBA:
                   R=0x11 G=0x11 B=0x11 A=0xCC                    */
                SDL_SetRenderDrawColor(ctx->renderer,
                                       0x11, 0x11, 0x11, 0xCC);
            } else {
                /* INVISIBLE = 0xFF000000 — solid black             */
                SDL_SetRenderDrawColor(ctx->renderer,
                                       0x00, 0x00, 0x00, 0xFF);
            }

            SDL_RenderFillRect(ctx->renderer, &rect);
        }
    }

    /* Restore opaque drawing for everything that follows */
    SDL_SetRenderDrawBlendMode(ctx->renderer, SDL_BLENDMODE_NONE);
}

 
/* ── hero source-rect lookup ──────────────────────────
   Column = which frame of the current animation. Row = armor tier.
   Tiers aren't tracked on Player yet, so this is pinned to row 0 (the
   sheet's base/no-armor row) — bump HERO_ARMOR_TIER, or make it a
   real field on Player, once armor visuals are wired up. */
 
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
void render_player(RenderCtx *ctx, const Player *p, const Camera *cam)
{
    SDL_Texture *tex = ctx->heroes[p->cls];
    if (!tex) return;
 
    Point scr = camera_tile_to_screen(cam, p->x, p->y);

    SDL_Rect dst = {
        scr.x - (HERO_W - TILE_SIZE) / 2,
        scr.y - (HERO_H - TILE_SIZE),
        HERO_W,
        HERO_H
    };

    SDL_Rect src = hero_frame_rect(p);
 
    SDL_RendererFlip flip = (p->facing == FACE_LEFT) ? SDL_FLIP_HORIZONTAL
                                                      : SDL_FLIP_NONE;
 
    SDL_RenderCopyEx(ctx->renderer, tex, &src, &dst, 0.0, NULL, flip);
}

/* ── render_present ───────────────────────────────── */
void render_present(RenderCtx *ctx) {
    SDL_RenderPresent(ctx->renderer);
}