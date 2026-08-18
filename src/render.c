#include "render.h"
#include "assets.h"
#include "icons.h" //test
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
    /*mobs*/
    const char *mob_files[] = ASSET_MOB_TEX_BY_ID;
    for (int i = 0; i < MOBTEX_COUNT; i++) {
        ctx->mob_tex[i] = load_tex(ctx->renderer, mob_files[i]);
        ctx->mob_tex_cols[i] = 1; /* safe fallback if the load below finds nothing */
        if (!ctx->mob_tex[i]) continue;

        int tex_w = 0;
        SDL_QueryTexture(ctx->mob_tex[i], NULL, NULL, &tex_w, NULL);
        /* cols = sheet width / frame width. Frame width comes from
           MOB_SPRITES, keyed by MobKind rather than MobTexId, so find
           any kind that uses this texture and read its frame_w — every
           kind sharing a texture shares its frame size by construction
           (they're literally the same sheet). */
        for (int k = 0; k < MOB_KIND_COUNT; k++) {
            if (MOB_TEX_FOR_KIND[k] != (MobTexId)i) continue;
            int fw = MOB_SPRITES[k].frame_w;
            if (fw > 0 && tex_w > 0) ctx->mob_tex_cols[i] = tex_w / fw;
            break;
        }
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
    /*ui*/
    ctx->ui.chrome = load_tex(ctx->renderer, ASSET_CHROME);
    ctx->ui.status = load_tex(ctx->renderer, ASSET_STATUS); 
    ctx->ui.hp_bar = load_tex(ctx->renderer, ASSET_HP_BAR);
    ctx->ui.xp_bar = load_tex(ctx->renderer, ASSET_XP_BAR);
    /*icons*/
    ctx->icons  = load_tex(ctx->renderer, ASSET_ICONS);
    
    /*SHADOW/FOG TEXTURE*/
    ctx->shadow = load_tex(ctx->renderer, ASSET_SHADOW);
    return 0;
}

/* ── render_free ──────────────────────────────────── */
static void destroy_tex(SDL_Texture **tex) {
    if (*tex) {
        SDL_DestroyTexture(*tex);
        *tex = NULL;
    }
}
static void ui_free(UITextures *ui) {
    destroy_tex(&ui->chrome);
    destroy_tex(&ui->status);
    destroy_tex(&ui->hp_bar);
    destroy_tex(&ui->xp_bar);
}
void render_free(RenderCtx *ctx) {
    for (int i = 0; i < 3; i++) {
        destroy_tex(&ctx->font.tex[i]);
    }
    memset(&ctx->font, 0, sizeof(ctx->font));

    for (int i = 0; i < TILESET_COUNT; i++) {
        destroy_tex(&ctx->tilesets[i]);
        destroy_tex(&ctx->water[i]);
    }
    for (int i = 0; i < HERO_CLASS_COUNT; i++) {
        destroy_tex(&ctx->heroes[i]);
    }
    for (int i = 0; i < MOBTEX_COUNT; i++) {
        destroy_tex(&ctx->mob_tex[i]);
    }
    destroy_tex(&ctx->items);
    destroy_tex(&ctx->icons);
    ui_free(&ctx->ui);

    destroy_tex(&ctx->shadow);

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

/* ── render_fog ────────────────────────────────── */
void render_fog(RenderCtx *ctx, const Fog *fog, const Camera *cam) {
    SDL_SetRenderDrawBlendMode(ctx->renderer, SDL_BLENDMODE_BLEND);
    
    for (int ty = cam->tile_y; ty <= cam->tile_y + gViewRows; ty++) {
        for (int tx = cam->tile_x; tx <= cam->tile_x + gViewCols; tx++) {
            if (!in_bounds(tx, ty)) continue;

            int idx = xy_to_idx(tx, ty);

            /* Fully visible — no overlay needed */
            if (fog->visible[idx]) continue;

            Point scr  = camera_tile_to_screen(cam, tx, ty);

            int pad = gTileSize / 2;
            SDL_Rect rect = {
                scr.x - pad,
                scr.y - pad,
                gTileSize + pad * 2,
                gTileSize + pad * 2
            };
            SDL_Rect rect2 = {
                rect.x - pad,
                rect.y - pad,
                rect.w + pad * 2,
                rect.h + pad * 2
            };
            
            if (fog->visited[idx]) {
                /* VISITED = 0xCC111111 (ARGB) */
                SDL_SetTextureColorMod(ctx->shadow, 0x11, 0x11, 0x11);
                SDL_SetTextureAlphaMod(ctx->shadow, 0xCC);

                SDL_RenderCopy(ctx->renderer, ctx->shadow, NULL, &rect);
            } else {
                /* INVISIBLE = 0xFF000000 */
                SDL_SetTextureColorMod(ctx->shadow, 0x00, 0x00, 0x00);
                SDL_SetTextureAlphaMod(ctx->shadow, 0xFF);

                SDL_RenderCopy(ctx->renderer, ctx->shadow, NULL, &rect2);
            }
            
        }
    }
    SDL_SetTextureColorMod(ctx->shadow, 255, 255, 255);
    SDL_SetTextureAlphaMod(ctx->shadow, 255);
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

void render_player(RenderCtx *ctx, const Player *p, const Camera *cam) {
    SDL_Texture *tex = ctx->heroes[p->cls];
    if (!tex) return;

    int draw_w = TILESET_HERO_W * gTileSize / TILE_SIZE_MIN;
    int draw_h = TILESET_HERO_H * gTileSize / TILE_SIZE_MIN;

    Point pos = idx_to_xy(p->base.pos);
    Point scr = camera_tile_to_screen(cam, pos.x, pos.y);

    SDL_Rect dst = {
        scr.x - (draw_w - gTileSize) / 2,
        scr.y - (draw_h - gTileSize),
        draw_w,
        draw_h
    };

    SDL_Rect src = hero_frame_rect(p);
    SDL_RendererFlip flip = (p->facing == FACE_LEFT) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    SDL_RenderCopyEx(ctx->renderer, tex, &src, &dst, 0.0, NULL, flip);
}




/* ── render_mobs ─────────────────────────────────────────── */
static int mob_has_status_icon(const Mob *m) {
    return m->state == MOB_ASLEEP || m->alert_icon;
}

static IconId mob_status_icon(const Mob *m) {
    if (m->state == MOB_ASLEEP)
        return ICON_SLEEP;

    return ICON_ALERT;
}

static SDL_Rect mob_frame_rect(RenderCtx *ctx, const Mob *m, MobTexId tex_id) {
    const MobSpriteInfo *info = &MOB_SPRITES[m->kind];
    const AnimClip *clip = &info->clips[m->anim];
    uint8_t idx = clip->frames[m->frame % clip->count];

    int cols = ctx->mob_tex_cols[tex_id];
    if (cols <= 0) cols = 1;

    SDL_Rect src = {
        (idx % cols) * info->frame_w,
        (idx / cols) * info->frame_h,
        info->frame_w,
        info->frame_h
    };
    return src;
}

void render_mobs(RenderCtx *ctx, const Camera *cam) {
    for (int i = 0; i < mob_count(); i++) {
        Mob *m = mob_at_index(i);
        if (!m)
            continue;

        if (!char_is_alive(&m->base) && m->anim_done)
            continue;

        Point pos = idx_to_xy(m->base.pos);

        /* outside camera */
        if (pos.x < cam->tile_x || pos.x > cam->tile_x + gViewCols ||
            pos.y < cam->tile_y || pos.y > cam->tile_y + gViewRows)
            continue;

        MobTexId tex_id = MOB_TEX_FOR_KIND[m->kind];
        SDL_Texture *tex = ctx->mob_tex[tex_id];
        if (!tex)
            continue;

        const MobSpriteInfo *info = &MOB_SPRITES[m->kind];

        int draw_w = info->frame_w * gTileSize / TILE_SIZE_MIN;
        int draw_h = info->frame_h * gTileSize / TILE_SIZE_MIN;

        Point scr = camera_tile_to_screen(cam, pos.x, pos.y);
        SDL_Rect dst = {
            scr.x - (draw_w - gTileSize) / 2,
            scr.y - (draw_h - gTileSize),
            draw_w,
            draw_h
        };

        SDL_Rect src = mob_frame_rect(ctx, m, tex_id);
        SDL_RendererFlip flip = (m->facing == FACE_LEFT) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
        SDL_RenderCopyEx(ctx->renderer, tex, &src, &dst, 0.0, NULL, flip);

        if (mob_has_status_icon(m)) {
            IconId icon = mob_status_icon(m);

            uint8_t ix, iy, iw, ih;
            icon_sprite(icon, &ix, &iy, &iw, &ih);

            SDL_Rect icon_src = {
                ix, iy, iw, ih
            };

            int scale = gTileSize / TILE_SIZE_MIN;
            if (scale < 1)
                scale = 1;

            SDL_Rect icon_dst = {
                dst.x + dst.w - 3,
                dst.y - 5,
                iw * scale,
                ih * scale
            };

            SDL_RenderCopy(ctx->renderer, ctx->icons, &icon_src, &icon_dst);
        }
    }
}






/* ── render_present ───────────────────────────────── */
void render_present(RenderCtx *ctx) {
    SDL_RenderPresent(ctx->renderer);
}