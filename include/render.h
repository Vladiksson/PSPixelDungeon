#pragma once
#include "camera.h"
#include "assets.h"
#include "player.h"
#include "fov.h"
#include "font.h"
#include <SDL2/SDL.h>

/* ── RenderCtx ───────────────────────────────────────── */
typedef struct {
   SDL_Window   *window;
   SDL_Renderer *renderer;

   /* Tile sheets — one per theme, indexed by DungeonTheme */
   SDL_Texture  *tilesets[TILESET_COUNT];

   /* Water surface sheets — separate from tile sheets.
       tiles*.png rows 48-63 = shore/transition edges only.
       water*.png = the actual tiling water fill drawn under them. */
   SDL_Texture  *water[TILESET_COUNT];

   /* Hero sheets — one per HeroClass, indexed by the enum in player.h.
   Loaded from ASSET_HERO_BY_CLASS in render_init. */
   SDL_Texture  *heroes[HERO_CLASS_COUNT];

   /* Item icon sheet — ASSET_ITEMS, an 8x16 grid of 16px cells. */
   SDL_Texture  *items;

   /* Bitmap font — ASSET_FONTS1X. PD's own packed-glyph format, not
      SDL_ttf; see font.h for the sheet layout. */
   BitmapFont    font;

   /*ui textures (from Chrome.java)*/
   SDL_Texture *chrome;
   
   DungeonTheme  active_theme;
} RenderCtx;

/* assets_dir: folder containing tiles0-4.png and water0-4.png, and the
   4 hero sheets referenced by ASSET_HERO_BY_CLASS.*/
int  render_init  (RenderCtx *ctx, const char *assets_dir);
void render_free  (RenderCtx *ctx);
void render_frame (RenderCtx *ctx, const Level *lvl, const Camera *cam);
/* Ground item heap. Call between render_frame and render_shadows so
   fog-of-war darkens/hides items exactly like it does terrain. */
void render_items (RenderCtx *ctx, const Level *lvl, const Camera *cam);
void render_shadows (RenderCtx *ctx, const Level *lvl, const Fog *fog, const Camera *cam);
void render_fog    (RenderCtx *ctx, const Fog    *fog, const Camera *cam);
void render_player (RenderCtx *ctx, const Player *p, const Camera *cam);
void render_present (RenderCtx *ctx);

/* Draw one tile sprite (0-63) from the active tileset at screen px (px,py) */
void render_tile  (RenderCtx *ctx, int sprite_idx, int px, int py);

/* Draw a water fill tile at screen px (px,py).
   water*.png is a tiling texture; src_x/src_y select the region. */
void render_water (RenderCtx *ctx, int src_x, int src_y, int px, int py);