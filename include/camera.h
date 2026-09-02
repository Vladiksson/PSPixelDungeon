#pragma once
#include "common.h"

typedef struct {
    int tile_x, tile_y;      /* top-left visible tile              */
} Camera;

void camera_center_on (Camera *cam, int idx);
void camera_move   (Camera *cam, int dx, int dy);

/* Convert a tile coord to screen pixel coord given camera */
Point camera_tile_to_screen(const Camera *cam, int tx, int ty);