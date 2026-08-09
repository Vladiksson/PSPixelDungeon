#pragma once
#include "camera.h"
#include "assets.h"
#include "player.h"
#include "mob.h"
#include "fov.h"
#include "font.h"
#include "icons.h"

typedef struct {
   SDL_Window   *window;
   SDL_Renderer *renderer;

   SDL_Texture  *tilesets[TILESET_COUNT];
   SDL_Texture  *water[TILESET_COUNT];
   SDL_Texture  *heroes[HERO_CLASS_COUNT];
   SDL_Texture  *mob_tex[MOBTEX_COUNT];
   int           mob_tex_cols[MOBTEX_COUNT];
   SDL_Texture  *items;
   BitmapFont    font;
   SDL_Texture  *chrome;
   SDL_Texture  *icons;
   
   DungeonTheme  active_theme;
} RenderCtx;

int  render_init  (RenderCtx *ctx);
void render_free  (RenderCtx *ctx);

void render_frame  (RenderCtx *ctx, const Level *lvl, const Camera *cam);
void render_items  (RenderCtx *ctx, const Level *lvl, const Camera *cam);
void render_shadows(RenderCtx *ctx, const Level *lvl, const Fog *fog, const Camera *cam);
void render_fog    (RenderCtx *ctx, const Fog    *fog, const Camera *cam);
void render_player (RenderCtx *ctx, const Player *p, const Camera *cam);
void render_mobs   (RenderCtx *ctx, const Camera *cam);

void render_present (RenderCtx *ctx);