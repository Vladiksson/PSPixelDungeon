#pragma once
#include "common.h"

/* ── Camera ──────────────────────────────────────────── */
/* Tracks which tile is at the top-left of the screen.
   Pixel-level offset for smooth scrolling (future use). */
typedef struct {
    int tile_x, tile_y;      /* top-left visible tile              */
    int px_off_x, px_off_y;  /* sub-tile pixel offset (0..15)      */
} Camera;

void camera_init   (Camera *cam, int center_tile_x, int center_tile_y);
void camera_move   (Camera *cam, int dx, int dy);   /* dx/dy in tiles */
void camera_clamp  (Camera *cam);                   /* stay in bounds  */

/* Convert a tile coord to screen pixel coord given camera */
Point camera_tile_to_screen(const Camera *cam, int tx, int ty);