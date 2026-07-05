#include "fov.h"
#include <string.h>

/* ══════════════════════════════════════════════════════════════
   LIGHT BLOCKING  — which tile types stop the FOV ray

   In PD, closed doors block LOS (you can't see through them).
   Open doors and all floor variants are transparent.
   ══════════════════════════════════════════════════════════════ */
static int blocks_light(const Level *lvl, int x, int y) {
    if (!in_bounds(x, y)) return 1;
    switch (lvl->map[xy_to_idx(x, y)]) {
        case TILE_WALL:
        case TILE_WALL_DECO:
        case TILE_SECRET_DOOR:
        case TILE_DOOR:
        case TILE_LOCKED_DOOR:
            return 1;
        default:
            return 0;
    }
}

/* ══════════════════════════════════════════════════════════════
   RECURSIVE OCTANT SHADOW CASTING
   Direct port of the RogueBasin "FOV using recursive shadow
   casting" algorithm (Bjorn Bergstrom / Albert Ford variant).

   Each call covers one octant of the circle.  The octant is
   mapped to world coordinates via the (xx,xy,yx,yy) transform.

   row        = first scan row (starts at 1)
   start/end  = angular range [1.0 … 0.0] — start > end
   ══════════════════════════════════════════════════════════════ */
static void cast_octant(uint8_t *vis, const Level *lvl,
                         int cx, int cy, int row,
                         float start, float end, int radius,
                         int xx, int xy, int yx, int yy) {
    if (start < end) return;

    float r2      = (float)(radius * radius);
    float nstart  = 0.0f;

    for (int j = row; j <= radius + 1; j++) {
        /* dx: lateral offset from -j to 0 in local octant space
           dy: depth (always -j in local octant space)           */
        int dx = -j - 1, dy = -j;
        int blocked = 0;

        while (dx <= 0) {
            dx++;
            int ax = cx + dx * xx + dy * xy;
            int ay = cy + dx * yx + dy * yy;

            /* Slopes for the left and right edges of this tile */
            float l_slope = (dx - 0.5f) / (dy + 0.5f);
            float r_slope = (dx + 0.5f) / (dy - 0.5f);

            if (start < r_slope) continue;   /* tile is outside our range */
            if (end   > l_slope) break;      /* past the end of our range  */

            /* Tile is in the light beam — mark visible if within radius */
            if ((float)(dx*dx + dy*dy) < r2 && in_bounds(ax, ay))
                vis[xy_to_idx(ax, ay)] = 1;

            if (blocked) {
                /* Scanning through shadow: look for gap */
                if (blocks_light(lvl, ax, ay)) {
                    nstart = r_slope;
                } else {
                    /* Found open space: end shadow, resume */
                    blocked = 0;
                    start   = nstart;
                }
            } else {
                /* In open: check for new wall */
                if (blocks_light(lvl, ax, ay) && j < radius) {
                    /* Wall starts a shadow — recurse for the subtended angle */
                    blocked = 1;
                    cast_octant(vis, lvl, cx, cy, j + 1,
                                start, l_slope, radius,
                                xx, xy, yx, yy);
                    nstart = r_slope;
                }
            }
        }

        /* If the entire row was shadowed, stop this octant */
        if (blocked) break;
    }
}

/* ══════════════════════════════════════════════════════════════
   PUBLIC API
   ══════════════════════════════════════════════════════════════ */

void fog_init(Fog *fog) {
    memset(fog->visible, 0, LEVEL_SIZE);
    memset(fog->visited, 0, LEVEL_SIZE);
}

/*
 * fog_update — recompute visible[], accumulate into visited[].
 * Call every time the player moves.
 *
 * The eight octant transforms map local (dx=lateral, dy=depth)
 * to world offsets: world = cx + dx*xx + dy*xy,
 *                           cy + dx*yx + dy*yy
 */
void fog_update(Fog *fog, const Level *lvl, int px, int py) {
    memset(fog->visible, 0, LEVEL_SIZE);

    /* Player's own tile is always visible */
    if (in_bounds(px, py)) fog->visible[xy_to_idx(px, py)] = 1;

    /* Cast into all 8 octants */
    static const int OCT[8][4] = {
        /* xx, xy, yx, yy */
        { 1,  0,  0,  1 },   /* E→NE  */
        { 0,  1,  1,  0 },   /* NE→N  */
        { 0, -1,  1,  0 },   /* N→NW  */
        {-1,  0,  0,  1 },   /* NW→W  */
        {-1,  0,  0, -1 },   /* W→SW  */
        { 0, -1, -1,  0 },   /* SW→S  */
        { 0,  1, -1,  0 },   /* S→SE  */
        { 1,  0,  0, -1 },   /* SE→E  */
    };
    for (int i = 0; i < 8; i++) {
        cast_octant(fog->visible, lvl, px, py, 1,
                    1.0f, 0.0f, FOV_RADIUS,
                    OCT[i][0], OCT[i][1], OCT[i][2], OCT[i][3]);
    }

    /* Accumulate: any visible tile is permanently visited */
    for (int i = 0; i < LEVEL_SIZE; i++)
        if (fog->visible[i]) fog->visited[i] = 1;
}