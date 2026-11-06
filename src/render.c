#include "render.h"
#include "assets.h"
#include "ballistica.h"
#include "item_wand.h"
#include <SDL2/SDL_image.h>
#include <stdio.h>

/*visibility helpers*/
static bool is_visible_on_camera(const Camera *cam, Point pos) {
    return pos.x >= cam->tile_x &&
           pos.x <= cam->tile_x + gViewCols &&
           pos.y >= cam->tile_y &&
           pos.y <= cam->tile_y + gViewRows;
}
static bool is_visible(const Fog *fog, const Camera *cam, Point pos) {
    if (!is_visible_on_camera(cam, pos))
        return false;

    int idx = xy_to_idx(pos.x, pos.y);
    return fog->visible[idx];
}
static int fog_cell_visible(int cell, void *user) {
    const Fog *fog = (const Fog *)user;
    return fog->visible[cell];
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
static bool load_chars(RenderCtx *ctx) { 
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
    /* ── banners ──────────────────────────────────── */
    ctx->banners = load_tex(ctx->renderer, ASSET_BANNERS);
    if (!ctx->banners)
        return false;
    /* ── avatars ──────────────────────────────────── */
    ctx->avatars = load_tex(ctx->renderer, ASSET_AVATARS);
    if (!ctx->avatars)
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

static bool load_menu(RenderCtx *ctx) {
    ctx->menu.arcs_bg = load_tex(ctx->renderer, ASSET_ARCS_BG);
    if (!ctx->menu.arcs_bg)
        return false;

    ctx->menu.arcs_fg = load_tex(ctx->renderer, ASSET_ARCS_FG);
    if (!ctx->menu.arcs_fg)
        return false;

    ctx->menu.arcs_bg_offset = 0.0f;
    ctx->menu.arcs_fg_offset = 0.0f;
    
    return true;
}

/* 1x1 white texture */
static SDL_Texture *make_pixel_tex(SDL_Renderer *r) {
    SDL_Texture *t = SDL_CreateTexture(r, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, 1, 1);
    if (!t)
        return NULL;
    Uint32 white = 0xFFFFFFFFu;
    if (SDL_UpdateTexture(t, NULL, &white, sizeof(white)) != 0) {
        SDL_DestroyTexture(t);
        return NULL;
    }
    SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND);
    return t;
}

static bool load_particles(RenderCtx *ctx) {
    ctx->specks = load_tex(ctx->renderer, ASSET_SPECKS);
    if (!ctx->specks)
        return false;

    ctx->particle_pixel = make_pixel_tex(ctx->renderer);
    if (!ctx->particle_pixel)
        return false;

    ctx->effects = load_tex(ctx->renderer, ASSET_EFFECTS);
    if (!ctx->effects)
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
    if (!load_menu(ctx))
        goto fail;
    if (!load_particles(ctx))
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
static void menu_free(MenuTextures *menu) {
    destroy_tex(&menu->arcs_bg);
    destroy_tex(&menu->arcs_fg);
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
    destroy_tex(&ctx->banners);
    destroy_tex(&ctx->avatars);

    destroy_tex(&ctx->specks);
    destroy_tex(&ctx->particle_pixel);
    destroy_tex(&ctx->effects);

    ui_free(&ctx->ui);
    menu_free(&ctx->menu);

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

/* ── menu ────────────────────── */
#define ARCHS_SCROLL_SPEED 20.0f

void render_menu_update(RenderCtx *ctx, float dt, bool reversed) {
    float shift = ARCHS_SCROLL_SPEED * dt;

    if (reversed)
        shift = -shift;

    ctx->menu.arcs_bg_offset += shift;
    ctx->menu.arcs_fg_offset += shift * 2.0f;
}

static int positive_mod(int value, int divisor) {
    int result = value % divisor;
    return result < 0 ? result + divisor : result;
}

static void render_tiled_texture(RenderCtx *ctx, SDL_Texture *tex, float offset_y) {
    if (!tex)
        return;

    int tex_w, tex_h;

    if (SDL_QueryTexture(tex, NULL, NULL, &tex_w, &tex_h) != 0)
        return;

    if (tex_w <= 0 || tex_h <= 0)
        return;

    int offset_x = tex_w / 4 - (SCREEN_W % tex_w) / 2;
    int oy = positive_mod((int)offset_y, tex_h);

    int start_x = -positive_mod(-offset_x, tex_w);
    int start_y = -oy;

    for (int y = start_y; y < SCREEN_H; y += tex_h) {
        for (int x = start_x; x < SCREEN_W; x += tex_w) {
            SDL_Rect dst = { x, y, tex_w, tex_h };
            SDL_RenderCopy(ctx->renderer, tex, NULL, &dst);
        }
    }
}

void render_menu(RenderCtx *ctx) {
    SDL_SetRenderDrawColor(ctx->renderer, 0, 0, 0, 255);
    SDL_RenderClear(ctx->renderer);

    render_tiled_texture(ctx, ctx->menu.arcs_bg, ctx->menu.arcs_bg_offset);
    render_tiled_texture(ctx, ctx->menu.arcs_fg, ctx->menu.arcs_fg_offset);
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

static int water_sprite(const Level *lvl, int tx, int ty) { //mirror Level.java getWaterTile()
    int top   = tile_unstitchable(get_tile_safe(lvl, xy_to_idx(tx,   ty-1)));
    int right = tile_unstitchable(get_tile_safe(lvl, xy_to_idx(tx+1, ty  )));
    int bot   = tile_unstitchable(get_tile_safe(lvl, xy_to_idx(tx,   ty+1)));
    int left  = tile_unstitchable(get_tile_safe(lvl, xy_to_idx(tx-1, ty  )));
    return TILE_WATER_TILES + (top | (right<<1) | (bot<<2) | (left<<3));
}

static void render_water_fill(RenderCtx *ctx, int tx, int ty, int px, int py) {
    SDL_Texture *tex = ctx->water[(int)ctx->active_theme];
    if (!tex) 
        return;

    const int TEX_SIZE = 32;

    int sx = (tx * gTileSize) % TEX_SIZE;
    int sy = ((ty * gTileSize) + (int)ctx->water_offset_y) % TEX_SIZE;

    if (sx < 0)
        sx += TEX_SIZE;

    if (sy < 0)
        sy += TEX_SIZE;

    int remaining_h = gTileSize;

    int src_y = sy;
    int dst_y = py;

    while (remaining_h > 0) {
        int chunk_h = TEX_SIZE - src_y;
        if (chunk_h > remaining_h)
            chunk_h = remaining_h;

        SDL_Rect src = {
            sx,
            src_y,
            gTileSize,
            chunk_h
        };
        SDL_Rect dst = {
            px,
            dst_y,
            gTileSize,
            chunk_h
        };

        SDL_RenderCopy(ctx->renderer, tex, &src, &dst);
        remaining_h -= chunk_h;
        dst_y += chunk_h;
        src_y = 0;
    }
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

void render_update_water(RenderCtx *ctx, float elapsed) { //MB TRANSFER SOMEWHERE???
    ctx->water_offset_y -= 10.0f * elapsed; //ORIGINAL WAS 5

    if (ctx->water_offset_y <= -32.0f)
        ctx->water_offset_y += 32.0f;
}

void render_update_particles(Level *lvl, const Fog *fog, float elapsed) {
    particle_update_all(lvl, elapsed);
    particle_update_blobs(lvl, &lvl->blob_pool, elapsed, fog_cell_visible, (void *)fog);
    particle_update_buffs(lvl, elapsed, fog_cell_visible, (void *)fog);
    particle_update_emitters(lvl, elapsed, fog_cell_visible, (void *)fog);
    particle_update_ambient(lvl, elapsed, fog_cell_visible, (void *)fog);
    wand_bolt_update_trail(lvl, elapsed);
    wand_fx_update(elapsed);
}

/* ── render_items ──────────────────────────────────── */
void render_items(RenderCtx *ctx, const Level *lvl, const Fog *fog, const Camera *cam) {
    SDL_Texture *tex = ctx->items;
    if (!tex) return;
 
    for (int i = 0; i < lvl->num_heaps; i++) {
        const Heap *h = &lvl->heaps[i];
        if (!h->active) continue;

        Point pos = idx_to_xy(h->pos);

        if (!is_visible(fog, cam, pos))
            continue;
 
        uint8_t col, row;
        if (!heap_sprite_cell(h, &col, &row)) continue;
 
        Point screen = camera_tile_to_screen(cam, pos.x, pos.y);
 
        SDL_Rect src = { col * ICON_SIZE, row * ICON_SIZE, ICON_SIZE, ICON_SIZE };
        SDL_Rect dst = { screen.x, screen.y, gTileSize, gTileSize };
 
        SDL_RenderCopy(ctx->renderer, tex, &src, &dst);
    }
}
/* ── render_blobs ───────────────────────────────────── */

static Uint8 to_byte(float v01) {
    if (v01 < 0.0f) v01 = 0.0f;
    if (v01 > 1.0f) v01 = 1.0f;
    return (Uint8)(v01 * 255.0f + 0.5f);
}

static void draw_speck(RenderCtx *ctx, const Particle *pt, float scr_x, float scr_y, float px_scale) {
    if (!ctx->specks) return;

    SDL_Rect src;
    speck_sprite(pt->speck_type, &src);

    float draw_w = src.w * px_scale * pt->scale_x;
    float draw_h = src.h * px_scale * pt->scale_y;

    SDL_Rect dst = {
        (int)(scr_x - draw_w / 2.0f + 0.5f),
        (int)(scr_y - draw_h / 2.0f + 0.5f),
        (int)(draw_w + 0.5f),
        (int)(draw_h + 0.5f)
    };

    SDL_SetTextureColorMod(ctx->specks, to_byte(pt->rm), to_byte(pt->gm), to_byte(pt->bm));
    SDL_SetTextureAlphaMod(ctx->specks, to_byte(pt->am));
    SDL_SetTextureBlendMode(ctx->specks, pt->light_mode ? SDL_BLENDMODE_ADD : SDL_BLENDMODE_BLEND);

    SDL_RenderCopyEx(ctx->renderer, ctx->specks, &src, &dst, (double)pt->angle, NULL, SDL_FLIP_NONE);

    SDL_SetTextureColorMod(ctx->specks, 255, 255, 255);
    SDL_SetTextureBlendMode(ctx->specks, SDL_BLENDMODE_BLEND);
}

static void draw_pixel_particle(RenderCtx *ctx, const Particle *pt, float scr_x, float scr_y, float px_scale) {
    if (!ctx->particle_pixel) return;

    float draw_w = pt->size * pt->scale_x * px_scale;
    float draw_h = pt->size * pt->scale_y * px_scale;

    if (draw_w < 0.0f) draw_w = 0.0f;
    if (draw_h < 0.0f) draw_h = 0.0f;

    SDL_Rect dst = {
        (int)(scr_x - draw_w / 2.0f + 0.5f),
        (int)(scr_y - draw_h / 2.0f + 0.5f),
        (int)(draw_w + 0.5f),
        (int)(draw_h + 0.5f)
    };

    SDL_SetTextureColorMod(ctx->particle_pixel, to_byte(pt->rm), to_byte(pt->gm), to_byte(pt->bm));
    SDL_SetTextureAlphaMod(ctx->particle_pixel, to_byte(pt->am));
    SDL_SetTextureBlendMode(ctx->particle_pixel, pt->light_mode ? SDL_BLENDMODE_ADD : SDL_BLENDMODE_BLEND);

    SDL_RenderCopyEx(ctx->renderer, ctx->particle_pixel, NULL, &dst, (double)pt->angle, NULL, SDL_FLIP_NONE);

    SDL_SetTextureBlendMode(ctx->particle_pixel, SDL_BLENDMODE_BLEND); /* restore — shared texture */
}

void render_particles(RenderCtx *ctx, const Level *lvl, const Camera *cam) {
    float px_scale = (float)gTileSize / (float)TILE_SIZE_MIN;
    Point origin = camera_tile_to_screen(cam, 0, 0);

    for (int i = 0; i < lvl->num_particles; i++) {
        const Particle *pt = &lvl->particles[i];
        if (!pt)
            continue;

        Point tile = { (int)(pt->x / TILE_SIZE_MIN), (int)(pt->y / TILE_SIZE_MIN) };
        if (!is_visible_on_camera(cam, tile))
            continue;

        float scr_x = origin.x + (pt->x / (float)TILE_SIZE_MIN) * gTileSize;
        float scr_y = origin.y + (pt->y / (float)TILE_SIZE_MIN) * gTileSize;

        if (pt->kind == PK_SPECK)
            draw_speck(ctx, pt, scr_x, scr_y, px_scale);
        else
            draw_pixel_particle(ctx, pt, scr_x, scr_y, px_scale);
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
static SDL_Rect hero_frame_rect(const AnimClip *clip, int frame, int armor_tier) {
    uint8_t col = clip->frames[frame % clip->count];

    SDL_Rect src = {
        col * TILESET_HERO_W,
        armor_tier * TILESET_HERO_H,
        TILESET_HERO_W,
        TILESET_HERO_H
    };
    return src;
}
static SDL_Rect player_frame_rect(const Player *p) {
    int armor_tier = get_armor_tier(&p->armor);
    return hero_frame_rect(&HERO_CLIPS[p->anim], p->frame, armor_tier);
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

void render_mobs(RenderCtx *ctx, const Fog *fog, const Camera *cam) {
    for (int i = 0; i < mob_count(); i++) {
        Mob *m = mob_at_index(i);
        if (!m)
            continue;

        if (!char_is_alive(&m->base) && m->anim_done)
            continue;

        Point pos = idx_to_xy(m->base.pos);

        if (!is_visible(fog, cam, pos))
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

            SDL_Rect icon_src;
            icon_sprite(icon, &icon_src);

            int scale = gTileSize / TILE_SIZE_MIN;
            if (scale < 1)
                scale = 1;

            SDL_Rect icon_dst = {
                dst.x + dst.w - 3,
                dst.y - 5,
                icon_src.w * scale,
                icon_src.h * scale
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

    int armor_tier = get_armor_tier(&p->armor);
    SDL_Rect src = hero_frame_rect(clip, n->frame, armor_tier);

    render_char_sprite(ctx->renderer, tex, src, screen, size.x, size.y, n->facing);
}

void render_npcs(RenderCtx *ctx, const Player *p, const Fog *fog, const Camera *cam) {
    for (int i = 0; i < npc_count(); ++i) {
        NPC *n = npc_at_index(i);
        if (!n)
            continue;

        if (n->kind < 0 || n->kind >= NPC_KIND_COUNT)
            continue;

        if (!char_is_alive(&n->base) && n->anim_done)
            continue;

        Point pos = idx_to_xy(n->base.pos);

        if (!is_visible(fog, cam, pos))
            continue;

        if (n->kind == NPC_MIRROR_IMAGE) {
            render_mirror_npc(ctx, p, cam, n, pos);
            continue;
        }

        render_regular_npc(ctx, cam, n, pos);
    }
}


/* ── render_missiles ───────────────────────────────────── */
void render_missiles(RenderCtx *ctx, const Camera *cam) {
    MissileView mv;
    if (missiles_snapshot(&mv) <= 0) return;

    if (mv.x < cam->tile_x - 1 || mv.x > cam->tile_x + gViewCols ||
        mv.y < cam->tile_y - 1 || mv.y > cam->tile_y + gViewRows)
        return;

    if (wand_bolt()) {
        return;
    }

    if (!ctx->items) return;

    Point origin = camera_tile_to_screen(cam, 0, 0);
    float scr_x = origin.x + mv.x * gTileSize;
    float scr_y = origin.y + mv.y * gTileSize;

    SDL_Rect src = { mv.col * ICON_SIZE, mv.row * ICON_SIZE, ICON_SIZE, ICON_SIZE };
    SDL_Rect dst = {
        (int)(scr_x + 0.5f),
        (int)(scr_y + 0.5f),
        gTileSize,
        gTileSize
    };

    SDL_RenderCopyEx(ctx->renderer, ctx->items, &src, &dst, (double)mv.angle, NULL, SDL_FLIP_NONE);
}

/* ── render_wand_fx ───────────────────────────────────── */
static void draw_stretched_effect(RenderCtx *ctx, const Camera *cam, EffectId effect, float wx0, float wy0, float wx1, float wy1, float alpha, float height_scale) {
    if (!ctx->effects || alpha <= 0.0f) return;

    SDL_Rect src;
    effect_sprite(effect, &src);
    if (src.w <= 0 || src.h <= 0) return;

    float px_scale = (float)gTileSize / (float)TILE_SIZE_MIN;
    Point origin = camera_tile_to_screen(cam, 0, 0);

    float sx = origin.x + (wx0 / (float)TILE_SIZE_MIN) * gTileSize;
    float sy = origin.y + (wy0 / (float)TILE_SIZE_MIN) * gTileSize;
    float ex = origin.x + (wx1 / (float)TILE_SIZE_MIN) * gTileSize;
    float ey = origin.y + (wy1 / (float)TILE_SIZE_MIN) * gTileSize;

    float dx = ex - sx;
    float dy = ey - sy;
    float length = sqrtf(dx * dx + dy * dy);
    float angle_deg = atan2f(dy, dx) * 180.0f / (float)M_PI;

    int draw_h = (int)(src.h * px_scale * height_scale + 0.5f);
    if (draw_h < 0) draw_h = 0;

    SDL_Rect dst = {
        (int)(sx + 0.5f),
        (int)(sy - draw_h / 2.0f + 0.5f),
        (int)(length + 0.5f),
        draw_h
    };
    SDL_Point center = { 0, draw_h / 2 };

    SDL_SetTextureAlphaMod(ctx->effects, to_byte(alpha));
    SDL_SetTextureBlendMode(ctx->effects, SDL_BLENDMODE_ADD);

    SDL_RenderCopyEx(ctx->renderer, ctx->effects, &src, &dst, (double)angle_deg, &center, SDL_FLIP_NONE);

    SDL_SetTextureAlphaMod(ctx->effects, 255);
    SDL_SetTextureBlendMode(ctx->effects, SDL_BLENDMODE_BLEND);
}

static void render_lightning_fx(RenderCtx *ctx, const Camera *cam) {
    LightningFxView lv;
    if (!lightning_fx_snapshot(&lv)) return;

    for (int i = 0; i < lv.count - 1; i++) {
        draw_stretched_effect(ctx, cam, EFFECT_LIGHTNING, lv.pts_x[i], lv.pts_y[i], lv.mid_x[i], lv.mid_y[i], lv.alpha, 1.0f);
        draw_stretched_effect(ctx, cam, EFFECT_LIGHTNING, lv.mid_x[i], lv.mid_y[i], lv.pts_x[i + 1], lv.pts_y[i + 1], lv.alpha, 1.0f);
    }
}

static void render_ray_fx(RenderCtx *ctx, const Camera *cam) {
    RayFxView rv;
    if (!ray_fx_snapshot(&rv)) return;

    draw_stretched_effect(ctx, cam, EFFECT_RAY, rv.sx, rv.sy, rv.ex, rv.ey, rv.alpha, rv.alpha);
}

void render_wand_fx(RenderCtx *ctx, const Camera *cam) {
    render_lightning_fx(ctx, cam);
    render_ray_fx(ctx, cam);
}

/* ── render_present ───────────────────────────────── */
void render_present(RenderCtx *ctx) {
    SDL_RenderPresent(ctx->renderer);
}


//test of target

void render_target_cursor(RenderCtx *ctx, int pos, const Camera *cam) {
    Point p = idx_to_xy(pos);
    Point screen = camera_tile_to_screen(cam, p.x, p.y);

    SDL_Rect src;
    icon_sprite(ICON_TARGET, &src);

    SDL_Rect dst = { screen.x, screen.y, gTileSize, gTileSize };

    SDL_RenderCopy(ctx->renderer, ctx->icons, &src, &dst);
}

//todo shift somewhere

void render_banner(RenderCtx *ctx, BannerId banner, const SDL_Rect *dst) {
    if (!ctx->banners || !dst)
        return;

    SDL_Rect src;
    banner_sprite(banner, &src);

    if (src.w <= 0 || src.h <= 0)
        return;

    SDL_RenderCopy(ctx->renderer, ctx->banners, &src, dst);
}

#define BANNER_GLOW_SPEED 1.0f
#define BANNER_GLOW_MIN   40
#define BANNER_GLOW_MAX   255

void render_banner_update(RenderCtx *ctx, float dt) {
    ctx->banner_signs_time += dt * BANNER_GLOW_SPEED;

    if (ctx->banner_signs_time >= 2.0f * (float)M_PI)
        ctx->banner_signs_time -= 2.0f * (float)M_PI;
}

void render_banner_glow(RenderCtx *ctx, const SDL_Rect *dst) {
    if (!ctx->banners || !dst)
        return;

    SDL_Rect src;
    banner_sprite(BANNER_PIXEL_DUNGEON_SIGNS, &src);

    float pulse = (sinf(ctx->banner_signs_time) + 1.0f) * 0.5f;

    Uint8 alpha = (Uint8)(BANNER_GLOW_MIN + pulse * (BANNER_GLOW_MAX - BANNER_GLOW_MIN));

    SDL_SetTextureAlphaMod(ctx->banners, alpha);
    SDL_SetTextureBlendMode(ctx->banners, SDL_BLENDMODE_ADD);

    SDL_RenderCopy(ctx->renderer, ctx->banners, &src, dst);

    SDL_SetTextureAlphaMod(ctx->banners, 255);
    SDL_SetTextureBlendMode(ctx->banners, SDL_BLENDMODE_BLEND);
}

#define AVATAR_W 24
#define AVATAR_H 32

static void avatar_sprite(HeroClass hero, SDL_Rect *src) {
    if (!src || hero < HERO_WARRIOR || hero >= HERO_CLASS_COUNT) {
        if (src)
            *src = (SDL_Rect){0, 0, 0, 0};
        return;
    }

    src->x = (int)hero * AVATAR_W;
    src->y = 0;
    src->w = AVATAR_W;
    src->h = AVATAR_H;
}
static void render_avatar(RenderCtx *ctx, HeroClass hero, const SDL_Rect *dst) {
    if (!ctx->avatars || !dst)
        return;

    SDL_Rect src;
    avatar_sprite(hero, &src);

    if (src.w <= 0 || src.h <= 0)
        return;

    SDL_RenderCopy(ctx->renderer, ctx->avatars, &src, dst);
}

void render_avatars(RenderCtx *ctx, int scale) {
    int avatar_w = AVATAR_W * scale;
    int avatar_h = AVATAR_H * scale;
    int gap = (AVATAR_W / 2);

    int hero_count = HERO_CLASS_COUNT;
    int total_w = hero_count * avatar_w + (hero_count - 1) * gap;

    int start_x = (SCREEN_W - total_w) / 2;
    int y = (SCREEN_H - avatar_h) / 2;

    for (int i = HERO_WARRIOR; i < HERO_CLASS_COUNT; ++i) {
        SDL_Rect dst = {
            start_x + i * (avatar_w + gap),
            y,
            avatar_w,
            avatar_h
        };

        render_avatar(ctx, (HeroClass)i, &dst);
    }
}