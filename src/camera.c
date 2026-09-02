#include "camera.h"
#include "common.h"

static void camera_clamp(Camera *cam) { // stay in bounds
    int max_x = LEVEL_W - gViewCols;
    int max_y = LEVEL_H - gViewRows;
    if (cam->tile_x < 0)     cam->tile_x = 0;
    if (cam->tile_y < 0)     cam->tile_y = 0;
    if (cam->tile_x > max_x) cam->tile_x = max_x;
    if (cam->tile_y > max_y) cam->tile_y = max_y;
}

void camera_center_on(Camera *cam, int idx) {
    Point center_tile = idx_to_xy(idx);

    cam->tile_x = center_tile.x - gViewCols / 2;
    cam->tile_y = center_tile.y - gViewRows / 2;
    camera_clamp(cam);
}

void camera_move(Camera *cam, int dx, int dy) { //PLACEHOLDER FOR FREE CAMERA FLY
    cam->tile_x += dx;
    cam->tile_y += dy;
    camera_clamp(cam);
}

Point camera_tile_to_screen(const Camera *cam, int tx, int ty) {
    return (Point){
        (tx - cam->tile_x) * gTileSize,
        (ty - cam->tile_y) * gTileSize
    };
}