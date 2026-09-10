#pragma once
#include "render.h"

//test bullshit
typedef struct {
    int     open;
} StatsUI;

void stats_ui_open (StatsUI *ui);
void stats_ui_close(StatsUI *ui);

void render_stats_screen(RenderCtx *ctx, const Player *p);