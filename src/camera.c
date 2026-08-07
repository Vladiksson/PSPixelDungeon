#include "camera.h"
#include "common.h"

void camera_init(Camera *cam, int center_tile_x, int center_tile_y) {
    cam->tile_x   = center_tile_x - gViewCols / 2;
    cam->tile_y   = center_tile_y - gViewRows / 2;
    cam->px_off_x = 0;
    cam->px_off_y = 0;
    camera_clamp(cam);
}

/* Re-centre the camera on a tile (call every time the player moves) */
void camera_center_on(Camera *cam, int tile_x, int tile_y) {
    cam->tile_x = tile_x - gViewCols / 2;
    cam->tile_y = tile_y - gViewRows / 2;
    camera_clamp(cam);
}

void camera_move(Camera *cam, int dx, int dy) {
    cam->tile_x += dx;
    cam->tile_y += dy;
    camera_clamp(cam);
}

void camera_clamp(Camera *cam) {
    int max_x = LEVEL_W - gViewCols;
    int max_y = LEVEL_H - gViewRows;
    if (cam->tile_x < 0)     cam->tile_x = 0;
    if (cam->tile_y < 0)     cam->tile_y = 0;
    if (cam->tile_x > max_x) cam->tile_x = max_x;
    if (cam->tile_y > max_y) cam->tile_y = max_y;
}

Point camera_tile_to_screen(const Camera *cam, int tx, int ty) {
    return (Point){
        (tx - cam->tile_x) * gTileSize - cam->px_off_x,
        (ty - cam->tile_y) * gTileSize - cam->px_off_y
    };
}