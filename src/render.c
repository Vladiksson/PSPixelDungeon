#include "render.h"
#include "assets.h"
#include "icons.h"
#include <SDL2/SDL_image.h>
#include <stdio.h>

/*camera helper*/
static bool is_visible_on_camera(const Camera *cam, Point pos) {
    return pos.x >= cam->tile_x &&
           pos.x <= cam->tile_x + gViewCols &&
           pos.y >= cam->tile_y &&
           pos.y <= cam->tile_y + gViewRows;
}

/* ── INIT ─────────────────────────────────────── */
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

static bool init_sdl(RenderCtx *ctx) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        return false;

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    ctx->window = SDL_CreateWindow(
        "PSPixel Dungeon",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_W,
        SCREEN_H,
        SDL_WINDOW_SHOWN
    );
    if (!ctx->window)
        return false;

    ctx->renderer = SDL_CreateRenderer(ctx->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!ctx->renderer)
        return false;
    if (!ensure_png())
        return false;

    return true;
}
static bool load_tiles(RenderCtx *ctx) {
    const char *tile_files[] = ASSET_TILES_BY_THEME;
    const char *water_files[] = ASSET_WATER_BY_THEME;

    for (int i = 0; i < TILESET_COUNT; ++i) {
        ctx->tilesets[i] = load_tex(ctx->renderer, tile_files[i]);
        if (!ctx->tilesets[i])
            return false;

        ctx->water[i] = load_tex(ctx->renderer, water_files[i]);
        if (!ctx->water[i])
            return false;
    }
    return true;
}
static bool load_chars(RenderCtx *ctx) { //GONNA BE ACTOR LOAD AFTER ADDING BUFFS/BLOBS 
    /* ── heroes ─────────────────────────────────── */
    const char *hero_files[] = ASSET_HERO_BY_CLASS;

    for (int i = 0; i < HERO_CLASS_COUNT; ++i) {
        ctx->heroes[i] = load_tex(ctx->renderer, hero_files[i]);
        if (!ctx->heroes[i])
            return false;
    }
    /* ── mobs ───────────────────────────────────── */
    const char *mob_files[] = ASSET_MOB_TEX_BY_ID;

    for (int i = 0; i < MOBTEX_COUNT; ++i) {
        ctx->mob_tex[i] = load_tex(ctx->renderer, mob_files[i]);
        ctx->mob_tex_cols[i] = 1;
        if (!ctx->mob_tex[i])
            return false;

        int tex_w = 0;
        SDL_QueryTexture(ctx->mob_tex[i], NULL, NULL, &tex_w, NULL);

        for (int k = 0; k < MOB_KIND_COUNT; ++k) {
            if (MOB_TEX_FOR_KIND[k] != (MobTexId)i)
                continue;

            int frame_w = MOB_SPRITES[k].frame_w;

            if (frame_w > 0 && tex_w > 0)
                ctx->mob_tex_cols[i] = tex_w / frame_w;
            break;
        }
    }
    /* ── NPCs ───────────────────────────────────── */
    const char *npc_files[] = ASSET_NPC_TEX_BY_ID;

    for (int i = 0; i < NPCTEX_COUNT; ++i) {
        ctx->npc_tex[i] = load_tex(ctx->renderer, npc_files[i]);
        if (!ctx->npc_tex[i])
            return false;
    }

    return true;
}

static bool load_ui(RenderCtx *ctx) {
    /* ── fonts ──────────────────────────────────── */
    memset(&ctx->font, 0, sizeof(ctx->font));
    ctx->font.sheet_h = 8;

    const char *font_files[] = ASSET_FONTS1X_BY_PAGE;

    for (int i = 0; i < 3; ++i) {
        ctx->font.tex[i] = load_tex(ctx->renderer, font_files[i]);
        if (!ctx->font.tex[i])
            return false;
    }

    /* ── UI ─────────────────────────────────────── */
    ctx->ui.chrome = load_tex(ctx->renderer, ASSET_CHROME);
    if (!ctx->ui.chrome)
        return false;

    ctx->ui.status = load_tex(ctx->renderer, ASSET_STATUS);
    if (!ctx->ui.status)
        return false;

    ctx->ui.hp_bar = load_tex(ctx->renderer, ASSET_HP_BAR);
    if (!ctx->ui.hp_bar)
        return false;

    ctx->ui.xp_bar = load_tex(ctx->renderer, ASSET_XP_BAR);
    if (!ctx->ui.xp_bar)
        return false;

    /* ── icons ──────────────────────────────────── */
    ctx->icons = load_tex(ctx->renderer, ASSET_ICONS);
    if (!ctx->icons)
        return false;

    return true;
}

static bool load_items(RenderCtx *ctx) {
    ctx->items = load_tex(ctx->renderer, ASSET_ITEMS);
    if (!ctx->items)
        return false;
    return true;
}
static bool load_shadow(RenderCtx *ctx) {
    ctx->shadow = load_tex(ctx->renderer, ASSET_SHADOW);
    if (!ctx->shadow)
        return false;
    return true;
}

int render_init(RenderCtx *ctx) {
    memset(ctx, 0, sizeof(*ctx));
    if (!init_sdl(ctx))
        goto fail;
    if (!load_tiles(ctx))
        goto fail;
    if (!load_chars(ctx))
        goto fail;
    if (!load_ui(ctx))
        goto fail;
    if (!load_items(ctx))
        goto fail;
    if (!load_shadow(ctx))
        goto fail;
    return 0;
fail:
    render_free(ctx);
    return -1;
}

/* ── render_free ──────────────────────────────────── */
static void destroy_tex(SDL_Texture **tex) {
    if (*tex) {
        SDL_DestroyTexture(*tex);
        *tex = NULL;
    }
}
static void destroy_textures(SDL_Texture **textures, int count) {
    for (int i = 0; i < count; ++i)
        destroy_tex(&textures[i]);
}

static void ui_free(UITextures *ui) {
    destroy_tex(&ui->chrome);
    destroy_tex(&ui->status);
    destroy_tex(&ui->hp_bar);
    destroy_tex(&ui->xp_bar);
}
void render_free(RenderCtx *ctx) {
    destroy_textures(ctx->font.tex, 3);

    destroy_textures(ctx->tilesets, TILESET_COUNT);
    destroy_textures(ctx->water, TILESET_COUNT);

    destroy_textures(ctx->heroes, HERO_CLASS_COUNT);
    destroy_textures(ctx->mob_tex, MOBTEX_COUNT);
    destroy_textures(ctx->npc_tex, NPCTEX_COUNT);

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

static bool tile_unstitchable(uint8_t t){
    return (gTileFlags[t] & TF_UNSTITCHABLE) != 0;
}
static int water_sprite(const Level *lvl, int tx, int ty) { //mirror Level.java getWaterTile()
    int top   = tile_unstitchable(get_tile_safe(lvl, xy_to_idx(tx,   ty-1)));
    int right = tile_unstitchable(get_tile_safe(lvl, xy_to_idx(tx+1, ty  )));
    int bot   = tile_unstitchable(get_tile_safe(lvl, xy_to_idx(tx,   ty+1)));
    int left  = tile_unstitchable(get_tile_safe(lvl, xy_to_idx(tx-1, ty  )));
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
            if (!in_bounds_xy(tx, ty)) 
                continue;
            uint8_t tile = lvl->map[xy_to_idx(tx, ty)];

            Point screen  = camera_tile_to_screen(cam, tx, ty);
            if (tile == TILE_CHASM_WATER) {
                render_water_fill(ctx, tx, ty, screen.x, screen.y);
                render_tile(ctx, TILE_CHASM_WATER, screen.x, screen.y);
            } else if (tile == TILE_WATER) {
                render_water_fill(ctx, tx, ty, screen.x, screen.y);
                render_tile(ctx, water_sprite(lvl, tx, ty), screen.x, screen.y);
            } else {
                render_tile(ctx, (int)tile, screen.x, screen.y);
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

        Point pos = idx_to_xy(h->pos);

        if (!is_visible_on_camera(cam, pos))
            continue;
 
        uint8_t col, row;
        if (!heap_sprite_cell(h, &col, &row)) continue;
 
        Point screen = camera_tile_to_screen(cam, pos.x, pos.y);
 
        SDL_Rect src = { col * ICON_SIZE, row * ICON_SIZE, ICON_SIZE, ICON_SIZE };
        SDL_Rect dst = { screen.x, screen.y, gTileSize, gTileSize };
 
        SDL_RenderCopy(ctx->renderer, tex, &src, &dst);
    }
}

/* ── render_fog ────────────────────────────────── */
void render_fog(RenderCtx *ctx, const Fog *fog, const Camera *cam) {
    if (!ctx->shadow)
        return;
    SDL_SetRenderDrawBlendMode(ctx->renderer, SDL_BLENDMODE_BLEND);

    const int pad = gTileSize / 2;

    for (int ty = cam->tile_y - 1; ty < cam->tile_y + gViewRows + 1; ++ty) {
        for (int tx = cam->tile_x - 1; tx < cam->tile_x + gViewCols + 1; ++tx) {
            
            Point screen = camera_tile_to_screen(cam, tx, ty);

            SDL_Rect visited_rect = {
                screen.x - pad,
                screen.y - pad,
                gTileSize + pad * 2,
                gTileSize + pad * 2
            };

            SDL_Rect invisible_rect = {
                visited_rect.x - pad,
                visited_rect.y - pad,
                visited_rect.w + pad * 2,
                visited_rect.h + pad * 2
            };

            if (!in_bounds_xy(tx, ty)) {
                SDL_SetTextureColorMod(ctx->shadow, 0, 0, 0);
                SDL_SetTextureAlphaMod(ctx->shadow, 255);
                SDL_RenderCopy(ctx->renderer, ctx->shadow, NULL, &invisible_rect);
                continue;
            }
            int idx = xy_to_idx(tx, ty);
            if (fog->visible[idx])
                continue;
            if (fog->visited[idx]) {
                /* VISITED = 0xCC111111 (ARGB), textures was moded as well as color here */
                SDL_SetTextureColorMod(ctx->shadow, 0x11, 0x11, 0x11);
                SDL_SetTextureAlphaMod(ctx->shadow, 0xAA);
                SDL_RenderCopy(ctx->renderer, ctx->shadow, NULL, &visited_rect);
            }
            else {
                /* INVISIBLE = 0xFF000000 */
                SDL_SetTextureColorMod(ctx->shadow, 0, 0, 0);
                SDL_SetTextureAlphaMod(ctx->shadow, 255);
                SDL_RenderCopy(ctx->renderer, ctx->shadow, NULL, &invisible_rect);
            }
        }
    }
    SDL_SetTextureColorMod(ctx->shadow, 255, 255, 255);
    SDL_SetTextureAlphaMod(ctx->shadow, 255);
    SDL_SetRenderDrawBlendMode(ctx->renderer, SDL_BLENDMODE_NONE);
}

 
/* ── render_player ────────────────────────────────── */
static int hero_armor_tier = 0;

static SDL_Rect hero_frame_rect(const AnimClip *clip, int frame) {
    uint8_t col = clip->frames[frame % clip->count];

    SDL_Rect src = {
        col * TILESET_HERO_W,
        hero_armor_tier * TILESET_HERO_H,
        TILESET_HERO_W,
        TILESET_HERO_H
    };
    return src;
}
static SDL_Rect player_frame_rect(const Player *p) {
    return hero_frame_rect(&HERO_CLIPS[p->anim], p->frame);
}

static Point hero_draw_size(void) {
    return (Point) {
        TILESET_HERO_W * gTileSize / TILE_SIZE_MIN,
        TILESET_HERO_H * gTileSize / TILE_SIZE_MIN
    };
}

static void render_char_sprite(SDL_Renderer *renderer, SDL_Texture *tex, SDL_Rect src, Point screen, int draw_w, int draw_h, Facing facing) {
    SDL_Rect dst = {
        screen.x - (draw_w - gTileSize) / 2,
        screen.y - (draw_h - gTileSize),
        draw_w,
        draw_h
    };

    SDL_RendererFlip flip = (facing == FACE_LEFT) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    SDL_RenderCopyEx(renderer, tex, &src, &dst, 0.0, NULL, flip);
}

void render_player(RenderCtx *ctx, const Player *p, const Camera *cam) {
    if (!p) return;
    if (p->cls < 0 || p->cls >= HERO_CLASS_COUNT) return;
    if (p->anim < 0 || p->anim >= ANIM_COUNT) return;

    SDL_Texture *tex = ctx->heroes[p->cls];
    if (!tex) return;

    Point size = hero_draw_size();

    Point pos = idx_to_xy(p->base.pos);
    Point screen = camera_tile_to_screen(cam, pos.x, pos.y);

    hero_armor_tier = get_armor_tier(p);
    SDL_Rect src = player_frame_rect(p);

    render_char_sprite(ctx->renderer, tex, src, screen, size.x, size.y, p->facing); 
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

        if (!is_visible_on_camera(cam, pos))
            continue;

        MobTexId tex_id = MOB_TEX_FOR_KIND[m->kind];
        SDL_Texture *tex = ctx->mob_tex[tex_id];
        if (!tex)
            continue;

        const MobSpriteInfo *info = &MOB_SPRITES[m->kind];

        int draw_w = info->frame_w * gTileSize / TILE_SIZE_MIN;
        int draw_h = info->frame_h * gTileSize / TILE_SIZE_MIN;

        Point screen = camera_tile_to_screen(cam, pos.x, pos.y);
        SDL_Rect src = mob_frame_rect(ctx, m, tex_id);

        SDL_Rect dst = {
            screen.x - (draw_w - gTileSize) / 2,
            screen.y - (draw_h - gTileSize),
            draw_w,
            draw_h
        };
        
        SDL_RendererFlip flip = (m->facing == FACE_LEFT) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
        SDL_RenderCopyEx(ctx->renderer, tex, &src, &dst, 0.0, NULL, flip);

        if (mob_has_status_icon(m)) {
            if (!ctx->icons)
                continue;

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

/* ── render_npcs ────────────────────────────────────────── */
static SDL_Rect npc_frame_rect(const NPC *n) {
    const NpcSpriteInfo *info = &NPC_SPRITES[n->kind];
    const AnimClip *clip = &info->clips[n->anim];
    uint8_t idx = clip->frames[n->frame % clip->count];

    SDL_Rect src = {
        idx * info->frame_w,
        0,
        info->frame_w,
        info->frame_h
    };
    return src;
}

static void render_regular_npc(RenderCtx *ctx, const Camera *cam, const NPC *n, Point pos) {
    if (n->anim < 0 || n->anim >= NPC_ANIM_COUNT)
        return;

    NpcTexId tex_id = NPC_TEX_FOR_KIND[n->kind];
    SDL_Texture *tex = ctx->npc_tex[tex_id];

    if (!tex)
        return;

    const NpcSpriteInfo *info = &NPC_SPRITES[n->kind];

    int draw_w = info->frame_w * gTileSize / TILE_SIZE_MIN;
    int draw_h = info->frame_h * gTileSize / TILE_SIZE_MIN;

    Point screen = camera_tile_to_screen(cam, pos.x, pos.y);
    SDL_Rect src = npc_frame_rect(n);

    render_char_sprite(ctx->renderer, tex, src, screen, draw_w, draw_h, n->facing);
}
static void render_mirror_npc(RenderCtx *ctx, const Player *p, const Camera *cam, const NPC *n, Point pos) {
    if (!p)
        return;

    if (p->cls < 0 || p->cls >= HERO_CLASS_COUNT)
        return;

    if (n->anim < 0 || n->anim >= NPC_ANIM_COUNT)
        return;

    SDL_Texture *tex = ctx->heroes[p->cls];

    if (!tex)
        return;

    const AnimClip *clip = MIRROR_HERO_CLIPS[n->anim];

    if (!clip || !clip->frames || clip->count <= 0)
        return;

    Point size = hero_draw_size();
    Point screen = camera_tile_to_screen(cam, pos.x, pos.y);

    hero_armor_tier = get_armor_tier(p);
    SDL_Rect src = hero_frame_rect(clip, n->frame);

    render_char_sprite(ctx->renderer, tex, src, screen, size.x, size.y, n->facing);
}

void render_npcs(RenderCtx *ctx, const Player *p, const Camera *cam) {
    for (int i = 0; i < npc_count(); ++i) {
        NPC *n = npc_at_index(i);
        if (!n)
            continue;

        if (n->kind < 0 || n->kind >= NPC_KIND_COUNT)
            continue;

        if (!char_is_alive(&n->base) && n->anim_done)
            continue;

        Point pos = idx_to_xy(n->base.pos);

        if (!is_visible_on_camera(cam, pos))
            continue;

        if (n->kind == NPC_MIRROR_IMAGE) {
            render_mirror_npc(ctx, p, cam, n, pos);
            continue;
        }

        render_regular_npc(ctx, cam, n, pos);
    }
}


/* ── render_present ───────────────────────────────── */
void render_present(RenderCtx *ctx) {
    SDL_RenderPresent(ctx->renderer);
}