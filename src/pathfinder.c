#include "pathfinder.h"

static int path_cell_passable(const Level *lvl, int n, int from, int goal, int flying) {
    if (!flying) {
        if (!cell_passable(lvl, n) || cell_avoid(lvl, n))
            return 0;
    } else {
        if (cell_solid(lvl, n) && lvl->map[n] != TILE_DOOR)
            return 0;
    }
    if (n == from || n == goal)
        return 1;
    return char_find_at(n) == NULL;
}

/* ══════════════════════════════════════════════════════════
   port of PathFinder.java REDO 
   TEMPORARY BULLSHIT FOR THE TEST
   ══════════════════════════════════════════════════════════ */

static int     distance[LEVEL_SIZE];
static int     bfs_queue[LEVEL_SIZE];
static uint8_t is_goal[LEVEL_SIZE];

static int build_distance_map_single(const Level *lvl, int from, int to, int flying) {
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
            int nx = sp.x + DX[i], ny = sp.y + DY[i];
            if (!in_bounds_xy(nx, ny))
                continue;
            int n = xy_to_idx(nx, ny);

            if (path_cell_passable(lvl, n, from, to, flying) && distance[n] > next_dist) {
                bfs_queue[tail++] = n;
                distance[n] = next_dist;
            }
        }
    }
    return 0;
}

static int build_distance_map_multi(const Level *lvl, int from, const uint8_t *goals, int flying) {
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
            int nx = sp.x + DX[i], ny = sp.y + DY[i];
            if (!in_bounds_xy(nx, ny))
                continue;
            int n = xy_to_idx(nx, ny);

            if (path_cell_passable(lvl, n, from, -1, flying) && distance[n] > next_dist) {
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
        int nx = fp.x + DX[i], ny = fp.y + DY[i];
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

int pathfinder_get_step(const Level *lvl, int from, int to, int flying) {
    if (!build_distance_map_single(lvl, from, to, flying)) return -1;
    int best = best_neighbour(from);
    return (best == from) ? -1 : best;
}

static int build_escape_distance_map(const Level *lvl, int cur, int from, int flying) {
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
            int nx = sp.x + DX[i], ny = sp.y + DY[i];
            if (!in_bounds_xy(nx, ny))
                continue;
            int n = xy_to_idx(nx, ny);

            if (path_cell_passable(lvl, n, cur, from, flying) && distance[n] > next_dist) {
                bfs_queue[tail++] = n;
                distance[n] = next_dist;
            }
        }
    }
    return dist;
}

int pathfinder_get_step_back(const Level *lvl, int cur, int from, int flying) {
    int d = build_escape_distance_map(lvl, cur, from, flying);

    for (int i = 0; i < LEVEL_SIZE; i++) is_goal[i] = (distance[i] == d);

    if (!build_distance_map_multi(lvl, cur, is_goal, flying)) return -1;

    int best = best_neighbour(cur);
    return (best == cur) ? -1 : best;
}