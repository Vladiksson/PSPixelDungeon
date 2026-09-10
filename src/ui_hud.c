#include "ui_hud.h"
#include "ui_ninepatch.h"
#include "icons.h"
#include "common.h"
//TODO BUFFS
static void render_busy_indicator(RenderCtx *ctx, const Player *p) {
    if (!char_is_alive(&p->base)) // || p->ready)
        return;
    uint8_t sx, sy, sw, sh;
    icon_sprite(ICON_BUSY, &sx, &sy, &sw, &sh);

    SDL_Rect src = { sx, sy, sw, sh };
    SDL_Rect dst = { 0,  50, sw * 2, sh * 2 };

    SDL_SetTextureBlendMode(ctx->icons, SDL_BLENDMODE_BLEND);
    SDL_SetTextureAlphaMod(ctx->icons, 128);

    SDL_Point center = { dst.w / 2, dst.h / 2 };

    SDL_RenderCopyEx(ctx->renderer, ctx->icons, &src, &dst, ctx->busy_angle, &center, SDL_FLIP_NONE);
    SDL_SetTextureAlphaMod(ctx->icons, 255);
}

//test
void hud_update(RenderCtx *ctx, const Player *p) {
    if (!p || !char_is_alive(&p->base)) { //|| p->ready) {
        ctx->busy_angle = 0.0f;
        return;
    }
    ctx->busy_angle = fmodf(actor_now() * 45.0f, 360.0f);
}

void render_hud(RenderCtx *ctx, const Player *p, const Dungeon *d) {
    /* ── Status panel ───────────────────────────────────── */
    NinePatch status = status_panel(ctx->ui.status);
    ninepatch_draw(ctx->renderer, &status, 0, 0, 480, 96); //SCREEN_W, TEXTURE HIGH * SCALE

    /* ── HP bar ──────────────────────────────────────────── */
    if (ctx->ui.hp_bar && p && p->base.HT > 0) {
        int hp = p->base.HP;
        int max_hp = p->base.HT;
        if (hp < 0)
            hp = 0;
        if (hp > max_hp)
            hp = max_hp;

        float hp_ratio = (float)hp / (float)max_hp;

        const int bar_x = 45;
        const int bar_y = 4;
        const int bar_w = 70; //original texture size 64X4
        const int bar_h = 6;

        int tex_w = 0;
        int tex_h = 0;

        SDL_QueryTexture(ctx->ui.hp_bar, NULL, NULL, &tex_w, &tex_h);

        if (tex_w > 0 && tex_h > 0) {
            int fill_w = (int)(bar_w * hp_ratio);
            if (fill_w > 0) {
                SDL_Rect src = {
                    0,
                    0,
                    (int)(tex_w * hp_ratio),
                    tex_h
                };
                SDL_Rect dst = {
                    bar_x,
                    bar_y,
                    fill_w,
                    bar_h
                };
                SDL_RenderCopy(
                    ctx->renderer,
                    ctx->ui.hp_bar,
                    &src,
                    &dst
                );
            }
        }
    }
    /* ── XP bar ──────────────────────────────────────────── */
    if (ctx->ui.xp_bar && p) {
        int exp = p->exp;
        int max_exp = player_max_exp(p);

        if (exp < 0) exp = 0;
        if (exp > max_exp) exp = max_exp;

        float exp_ratio = (float)exp / (float)max_exp;

        const int bar_x = 0;
        const int bar_y = 0;
        const int bar_w = 480;
        const int bar_h = 1;

        int tex_w = 0;
        int tex_h = 0;

        SDL_QueryTexture(ctx->ui.xp_bar, NULL, NULL, &tex_w, &tex_h);

        if (tex_w > 0 && tex_h > 0) {
            const int fill_tex_w = tex_w / 2; //stupid fix of broken color gradient 
            int fill_w = (int)(bar_w * exp_ratio);

            if (fill_w > 0) {
                SDL_Rect src = {
                    0,
                    0,
                    fill_tex_w,
                    tex_h
                };

                SDL_Rect dst = {
                    bar_x,
                    bar_y,
                    fill_w,
                    bar_h
                };

                SDL_RenderCopy(
                    ctx->renderer,
                    ctx->ui.xp_bar,
                    &src,
                    &dst
                );
            }
        }
    }
    /* ── player sprite ───────────────────────────────────────── */
    if (p) { 
        SDL_Texture *hero_tex = ctx->heroes[p->cls]; 
        if (hero_tex) { 
            SDL_Rect src = { 
                0, 
                get_armor_tier(p) * TILESET_HERO_H, 
                TILESET_HERO_W, 
                TILESET_HERO_H 
            }; 
            const int icon_h = TILESET_HERO_H * 2; 
            const int icon_w = TILESET_HERO_W * 2; 
            SDL_Rect dst = { 
                9, 
                9, 
                icon_w, 
                icon_h 
            }; 
            SDL_RenderCopy(ctx->renderer, hero_tex, &src, &dst ); 
        } 
    }
    /* ── player lvl ──────────────────────────────────────────── */
    char current_lvl[8];
    snprintf(current_lvl, sizeof(current_lvl), "%d", p->lvl);

    int dx = 0;
    if(p->lvl > 9) 
        dx = 3;
    font_draw_text(ctx->renderer, &ctx->font, 38 - dx, 38, 1.39, COL_WHITE, current_lvl);
    /* ── dungeon lvl ──────────────────────────────────────────── */
    char current_floor[16];
    snprintf(current_floor, sizeof(current_floor), "%d", d->current_depth + 1);
    int dt = 0;
    if(d->current_depth + 1 > 9) 
        dt = 7;
    font_draw_text(ctx->renderer, &ctx->font, 435 - dt, 9, 1.5, COL_WHITE, current_floor);
    /* ── keys counter ──────────────────────────────────────────── */
    //TODO rewrite after locking keys to its current floor
    const Bag *keyring = &p->bags[BAG_KEYRING];
    int keys = 0;
    for (int i = 0; i < keyring->count; i++)
        keys += keyring->items[i].quantity;

    char key_count[16];
    snprintf(key_count, sizeof(key_count), "%d", keys);

    font_draw_text(ctx->renderer, &ctx->font, 459, 9, 1.5, COL_WHITE, key_count);

    /* ── busy indicator ───────────────────────────────── */
    render_busy_indicator(ctx, p);
}
