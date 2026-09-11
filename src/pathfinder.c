#include "pathfinder.h"

/* ══════════════════════════════════════════════════════════
   port of PathFinder.java REDO 
   TEMPORARY BULLSHIT FOR THE TEST
   ══════════════════════════════════════════════════════════ */

static int     distance[LEVEL_SIZE];
static int     bfs_queue[LEVEL_SIZE];
static uint8_t is_goal[LEVEL_SIZE];

static const int DX8[8] = { -1, 1,  0, 0, -1,  1, -1, 1 };
static const int DY8[8] = {  0, 0, -1, 1, -1, -1,  1, 1 };

static int cell_passable(const Level *lvl, int idx) {
    return (gTileFlags[lvl->map[idx]] & TF_PASSABLE) != 0;
}

static int build_distance_map_single(const Level *lvl, int from, int to) {
    if (from == to) return 0;

    for (int i = 0; i < LEVEL_SIZE; i++) distance[i] = INT_MAX;

    int head = 0, tail = 0;
    bfs_queue[tail++] = to;
    distance[to] = 0;

    while (head < tail) {
        int step = bfs_queue[head++];
        if (step == from) return 1;

        int next_dist = distance[step] + 1;
        Point sp = idx_to_xy(step);

        for (int i = 0; i < 8; i++) {
            int nx = sp.x + DX8[i], ny = sp.y + DY8[i];
            if (!in_bounds_xy(nx, ny))
                continue;
            int n = xy_to_idx(nx, ny);

            if (n == from || (cell_passable(lvl, n) && distance[n] > next_dist)) {
                bfs_queue[tail++] = n;
                distance[n] = next_dist;
            }
        }
    }
    return 0;
}

static int build_distance_map_multi(const Level *lvl, int from, const uint8_t *goals) {
    if (goals[from]) return 0;

    for (int i = 0; i < LEVEL_SIZE; i++) distance[i] = INT_MAX;

    int head = 0, tail = 0;
    for (int i = 0; i < LEVEL_SIZE; i++) {
        if (goals[i]) {
            bfs_queue[tail++] = i;
            distance[i] = 0;
        }
    }

    while (head < tail) {
        int step = bfs_queue[head++];
        if (step == from) return 1;

        int next_dist = distance[step] + 1;
        Point sp = idx_to_xy(step);

        for (int i = 0; i < 8; i++) {
            int nx = sp.x + DX8[i], ny = sp.y + DY8[i];
            if (!in_bounds_xy(nx, ny))
                continue;
            int n = xy_to_idx(nx, ny);

            if (n == from || (cell_passable(lvl, n) && distance[n] > next_dist)) {
                bfs_queue[tail++] = n;
                distance[n] = next_dist;
            }
        }
    }
    return 0;
}

static int best_neighbour(int from) {
    int min_d = distance[from];
    int best = from;
    Point fp = idx_to_xy(from);

    for (int i = 0; i < 8; i++) {
        int nx = fp.x + DX8[i], ny = fp.y + DY8[i];
        if (!in_bounds_xy(nx, ny))
            continue;
        int n = xy_to_idx(nx, ny);
        if (distance[n] < min_d) {
            min_d = distance[n];
            best = n;
        }
    }
    return best;
}

int pathfinder_get_step(const Level *lvl, int from, int to) {
    if (!build_distance_map_single(lvl, from, to)) return -1;
    int best = best_neighbour(from);
    return (best == from) ? -1 : best;
}

static int build_escape_distance_map(const Level *lvl, int cur, int from) {
    for (int i = 0; i < LEVEL_SIZE; i++) distance[i] = INT_MAX;

    int dest_dist = INT_MAX;
    int head = 0, tail = 0;
    bfs_queue[tail++] = from;
    distance[from] = 0;

    int dist = 0;
    while (head < tail) {
        int step = bfs_queue[head++];
        dist = distance[step];
        if (dist > dest_dist) return dest_dist;
        if (step == cur) dest_dist = dist * 2 + 1;

        int next_dist = dist + 1;
        Point sp = idx_to_xy(step);

        for (int i = 0; i < 8; i++) {
            int nx = sp.x + DX8[i], ny = sp.y + DY8[i];
            if (!in_bounds_xy(nx, ny))
                continue;
            int n = xy_to_idx(nx, ny);

            if (cell_passable(lvl, n) && distance[n] > next_dist) {
                bfs_queue[tail++] = n;
                distance[n] = next_dist;
            }
        }
    }
    return dist;
}

int pathfinder_get_step_back(const Level *lvl, int cur, int from) {
    int d = build_escape_distance_map(lvl, cur, from);

    for (int i = 0; i < LEVEL_SIZE; i++) is_goal[i] = (distance[i] == d);

    if (!build_distance_map_multi(lvl, cur, is_goal)) return -1;

    int best = best_neighbour(cur);
    return (best == cur) ? -1 : best;
}