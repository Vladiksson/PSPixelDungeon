#pragma once 
#include "render.h"
#include "dungeon.h"
void render_hud(RenderCtx *ctx, const Player *p, const Dungeon *d);

void hud_update(RenderCtx *ctx, const Player *p);