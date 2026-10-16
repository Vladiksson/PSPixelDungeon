#pragma once
#include "camera.h"
#include "player.h"
#include "fov.h"
#include "font.h"
#include "banners.h"
#include "avatars.h"

typedef struct {
   SDL_Texture *chrome;
   SDL_Texture *status;
   SDL_Texture *hp_bar;
   SDL_Texture *xp_bar;
} UITextures;

typedef struct {
   SDL_Texture *arcs_bg;
   SDL_Texture *arcs_fg;

   float arcs_bg_offset;
   float arcs_fg_offset;
} MenuTextures;

typedef struct {
   SDL_Window   *window;
   SDL_Renderer *renderer;

   SDL_Texture  *tilesets[TILESET_COUNT];
   SDL_Texture  *water[TILESET_COUNT];
   float water_offset_y;

   SDL_Texture  *heroes[HERO_CLASS_COUNT];

   SDL_Texture  *mob_tex[MOBTEX_COUNT];
   int           mob_tex_cols[MOBTEX_COUNT];

   SDL_Texture  *npc_tex[NPCTEX_COUNT];

   SDL_Texture  *items;

   SDL_Texture  *shadow;

   BitmapFont    font;
   UITextures    ui;
   SDL_Texture  *icons;
   float         busy_angle;

   SDL_Texture  *banners;
   float banner_signs_time;
   SDL_Texture  *avatars;

   DungeonTheme  active_theme;

   MenuTextures  menu;
} RenderCtx;

int  render_init  (RenderCtx *ctx);
void render_free  (RenderCtx *ctx);

void render_menu_update(RenderCtx *ctx, float dt, bool reversed);
void render_menu(RenderCtx *ctx);

void render_update_water(RenderCtx *ctx, float elapsed);

void render_frame  (RenderCtx *ctx, const Level *lvl, const Camera *cam);
void render_items  (RenderCtx *ctx, const Level *lvl, const Fog *fog, const Camera *cam);
void render_fog    (RenderCtx *ctx, const Fog *fog, const Camera *cam);
void render_player (RenderCtx *ctx, const Player *p, const Camera *cam);
void render_mobs   (RenderCtx *ctx, const Fog *fog, const Camera *cam);
void render_npcs(RenderCtx *ctx, const Player *p, const Fog *fog, const Camera *cam);

void render_missiles(RenderCtx *ctx, const Camera *cam);

void render_present (RenderCtx *ctx);


//test
void render_target_cursor(RenderCtx *ctx, int pos, const Camera *cam);

void render_banner(RenderCtx *ctx, BannerId banner, const SDL_Rect *dst);
void render_banner_glow(RenderCtx *ctx, const SDL_Rect *dst);
void render_banner_update(RenderCtx *ctx, float dt);

void render_avatars(RenderCtx *ctx, int scale);