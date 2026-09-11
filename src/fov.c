#include "fov.h"
#include <string.h>

static int blocks_light(const Level *lvl, int idx) {
    return (gTileFlags[lvl->map[idx]] & TF_LOS_BLOCKING) != 0;
}

static int fov_radius(int depth) {
    if (depth > 9 && depth < 15) {
        return 6;
    }
    if (depth > 20 && depth < 24) {
        int radius = 24 - depth;
        return radius > 1 ? radius : 1;
    }
    if (depth == 24) {
        return 3;
    }
    return 8;
}

/* ══════════════════════════════════════════════════════════════
   RECURSIVE OCTANT SHADOW CASTING
   Direct port of the RogueBasin "FOV using recursive shadow
   casting" algorithm (Bjorn Bergstrom / Albert Ford variant)
   ══════════════════════════════════════════════════════════════ */
static void cast_octant(uint8_t *vis, const Level *lvl, int idx, int row, float start, float end, int radius, int xx, int xy, int yx, int yy) {
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
            Point c = idx_to_xy(idx);

            int ax = c.x + dx * xx + dy * xy;
            int ay = c.y + dx * yx + dy * yy;

            if (!in_bounds_xy(ax, ay))
                continue;

            int aidx = xy_to_idx(ax, ay);

            /* Slopes for the left and right edges of this tile */
            float l_slope = (dx - 0.5f) / (dy + 0.5f);
            float r_slope = (dx + 0.5f) / (dy - 0.5f);

            if (start < r_slope) continue;   /* tile is outside our range */
            if (end   > l_slope) break;      /* past the end of our range  */

            /* Tile is in the light beam — mark visible if within radius */
            if ((float)(dx*dx + dy*dy) < r2)
                vis[aidx] = 1;

            if (blocked) {
                /* Scanning through shadow: look for gap */
                if (blocks_light(lvl, aidx)) {
                    nstart = r_slope;
                } else {
                    /* Found open space: end shadow, resume */
                    blocked = 0;
                    start   = nstart;
                }
            } else {
                /* In open: check for new wall */
                if (blocks_light(lvl, aidx) && j < radius) {
                    /* Wall starts a shadow — recurse for the subtended angle */
                    blocked = 1;
                    cast_octant(vis, lvl, idx, j + 1, start, l_slope, radius, xx, xy, yx, yy);
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

void fog_update(Fog *fog, const Level *lvl, int idx, int depth) {
    memset(fog->visible, 0, LEVEL_SIZE);

    if (in_bounds(idx)) fog->visible[idx] = 1;

    int radius = fov_radius(depth);

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
        cast_octant(fog->visible, lvl, idx, 1, 1.0f, 0.0f, radius, OCT[i][0], OCT[i][1], OCT[i][2], OCT[i][3]);
    }
    for (int i = 0; i < LEVEL_SIZE; i++)
        if (fog->visible[i]) fog->visited[i] = 1;
}