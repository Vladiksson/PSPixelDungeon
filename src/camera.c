#include "camera.h"

void camera_init(Camera *cam, int center_tile_x, int center_tile_y) {
    cam->tile_x   = center_tile_x - VIEW_COLS / 2;
    cam->tile_y   = center_tile_y - VIEW_ROWS / 2;
    cam->px_off_x = 0;
    cam->px_off_y = 0;
    camera_clamp(cam);
}

void camera_move(Camera *cam, int dx, int dy) {
    cam->tile_x += dx;
    cam->tile_y += dy;
    camera_clamp(cam);
}

void camera_clamp(Camera *cam) {
    int max_x = LEVEL_W - VIEW_COLS;
    int max_y = LEVEL_H - VIEW_ROWS;
    if (cam->tile_x < 0)     cam->tile_x = 0;
    if (cam->tile_y < 0)     cam->tile_y = 0;
    if (cam->tile_x > max_x) cam->tile_x = max_x;
    if (cam->tile_y > max_y) cam->tile_y = max_y;
}

Point camera_tile_to_screen(const Camera *cam, int tx, int ty) {
    return (Point){
        (tx - cam->tile_x) * TILE_SIZE - cam->px_off_x,
        (ty - cam->tile_y) * TILE_SIZE - cam->px_off_y
    };
}