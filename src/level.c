#include "level_paint.h"
#include "level_decorate.h"
#include "level_boss.h"
#include "level_last.h"
#include "item_spawn.h"
#include <time.h>
#include <math.h>

/* ══════════════════════════════════════════════════════════
   PUBLIC: level_generate
   ══════════════════════════════════════════════════════════ */
#define LAST_SHOP_DEPTH 20
#define FINAL_LEVEL_DEPTH 25

int level_generate(Level *lvl, uint32_t seed, DungeonTheme theme, int depth, int blacksmith_depth) {
    if (level_is_boss_depth(depth))
        return level_generate_boss(lvl, seed, theme, depth);
    if (depth == LAST_SHOP_DEPTH)
        return level_generate_last_shop(lvl, seed, depth, 1);
    if (depth == FINAL_LEVEL_DEPTH)
        return level_generate_last(lvl, seed, depth);

    rng_state = seed ? seed : (uint32_t)time(NULL);
    memset(lvl, 0, sizeof(*lvl));
    
    mob_set_pool(&lvl->mob_pool);

    LevelGenState gen;
    init_special_rooms(&gen, depth);

    lvl->theme = theme;
    memset(lvl->map, TILE_WALL, sizeof(lvl->map));

    lvl->feeling = FEELING_NONE;
    if (depth > 0) {
        switch (rng_int(10)) {
        case 0:
            if (!level_is_boss_depth(depth + 1))
                lvl->feeling = FEELING_CHASM;
            break;
        case 1:
            lvl->feeling = FEELING_WATER;
            break;
        case 2:
            lvl->feeling = FEELING_GRASS;
            break;
        }
    }  
    /* 1 BSP split + neighbour graph */
    int retry=0;
    do {
        lvl->num_rooms=0;
        bsp_split(lvl, 0, 0, LEVEL_W-1, LEVEL_H-1);
        if (++retry > 10) return -1;
    } while (lvl->num_rooms < 8);
    build_neighbours(lvl);

    /* 2 Entrance / exit (far apart, width/height ≥ 4) */
    int entrance_r=-1, exit_r=-1, attempts=0;
    int min_dist = (int)sqrt((double)lvl->num_rooms);
    do {
        do { entrance_r=rng_range(0,lvl->num_rooms-1); }
        while (room_w(&lvl->rooms[entrance_r])<4 || room_h(&lvl->rooms[entrance_r])<4 ||
            !entrance_has_large_neighbour(lvl, entrance_r));
        do { exit_r=rng_range(0,lvl->num_rooms-1); }
        while (exit_r==entrance_r || room_w(&lvl->rooms[exit_r])<4 || room_h(&lvl->rooms[exit_r])<4 || rooms_are_neighbours(lvl, entrance_r, exit_r));
        build_distance_map(lvl, exit_r);
        if (lvl->rooms[entrance_r].distance >= min_dist) break;
    } while (++attempts < 20);
    if (entrance_r<0 || exit_r<0) return -1;

    /* 3 Two paths + random expansion */
    for (int i=0; i<lvl->num_rooms; i++) lvl->rooms[i].price=1;

    int path[MAX_ROOMS], plen;
    build_distance_map(lvl, exit_r);
    plen=build_path(lvl, entrance_r, exit_r, path);
    for (int i=0; i<plen-1; i++) room_connect(lvl, path[i], path[i+1]);

    set_price(lvl, path, plen, lvl->rooms[entrance_r].distance);
    build_distance_map(lvl, exit_r);
    int path2[MAX_ROOMS], plen2;
    plen2=build_path(lvl, entrance_r, exit_r, path2);
    for (int i=0; i<plen2-1; i++) room_connect(lvl, path2[i], path2[i+1]);

    for (int i=0; i<lvl->num_rooms; i++) lvl->rooms[i].price=1;
    build_distance_map(lvl, exit_r);

    int conn_set[MAX_ROOMS], n_conn=0;
    #define add_set(idx) do { int _f=0; for(int _k=0;_k<n_conn;_k++) if(conn_set[_k]==(idx)){_f=1;break;} if(!_f&&n_conn<MAX_ROOMS) conn_set[n_conn++]=(idx); } while(0)
    add_set(entrance_r);
    for (int i=0;i<plen; i++) add_set(path [i]);
    for (int i=0;i<plen2;i++) add_set(path2[i]);

    int n_target=(int)(lvl->num_rooms*(0.5f+rng_float()*0.2f)), safety=0;
    while (n_conn<n_target && safety++<300) {
        int ci=conn_set[rng_range(0,n_conn-1)];
        Room *cr=&lvl->rooms[ci];
        if (cr->n_neighbours==0) continue;
        int ni=cr->neighbours[rng_range(0,cr->n_neighbours-1)];
        int found=0; for(int k=0;k<n_conn;k++) if(conn_set[k]==ni){found=1;break;}
        if (!found) { room_connect(lvl,ci,ni); add_set(ni); }
    }

    /* 3.5*/
    place_special_near_entrance(lvl, entrance_r, depth, blacksmith_depth); //shop + blacksmith
    /* 4 Assign room types */
    assign_types(lvl, &gen, entrance_r, exit_r, theme, depth);

    /* 5 Compute door positions */
    compute_doors(lvl);

    /* 6  Paint ROOMS*/
    paint_rooms(lvl, depth);     
    paint_tunnels(lvl);   

    
    paint_chasms_feeling(lvl);

    /* 7 Assign door types*/
    assign_door_types(lvl);
    paint_doors(lvl);     
    
    /* 8  Water + grass via Patch automaton 
        + Theme decoration*/
    static uint8_t wp[LEVEL_SIZE], gp[LEVEL_SIZE];
    float wseed,gseed; int wgen,ggen;
    void (*decor)(Level *) = decorate_generic;
    switch (theme) {
    case THEME_SEWERS:
        wseed = (lvl->feeling == FEELING_WATER) ? 0.60f : 0.45f; wgen = 5;
        gseed = (lvl->feeling == FEELING_GRASS) ? 0.60f : 0.40f; ggen = 4;
        decor = decorate_sewers;
        break;
    case THEME_PRISON:
        wseed = (lvl->feeling == FEELING_WATER) ? 0.65f : 0.45f; wgen = 4;
        gseed = (lvl->feeling == FEELING_GRASS) ? 0.60f : 0.40f; ggen = 3;
        decor = decorate_prison;
        break;
    case THEME_CAVES:
        wseed = (lvl->feeling == FEELING_WATER) ? 0.60f : 0.45f; wgen = 6;
        gseed = (lvl->feeling == FEELING_GRASS) ? 0.55f : 0.35f; ggen = 3;
        decor = decorate_caves;
        break;
    case THEME_CITY:
        wseed = (lvl->feeling == FEELING_WATER) ? 0.65f : 0.45f; wgen = 4;
        gseed = (lvl->feeling == FEELING_GRASS) ? 0.60f : 0.40f; ggen = 3;
        decor = decorate_city;
        break;
    case THEME_HALLS:
        wseed = (lvl->feeling == FEELING_WATER) ? 0.55f : 0.40f; wgen = 6;
        gseed = (lvl->feeling == FEELING_GRASS) ? 0.55f : 0.30f; ggen = 3;
        decor = decorate_halls;
        break;
    default:
        wseed = 0.0f; wgen = 0;
        gseed = 0.0f; ggen = 0; 
        decor = decorate_generic;
        break;
    }
    if (wgen>0) { patch_generate(wseed,wgen,wp); paint_water(lvl,wp); }
    if (ggen>0) { patch_generate(gseed,ggen,gp); paint_grass(lvl,gp); }

    decor(lvl);
    paint_chasms(lvl);
    /* 9 Scatter loot  */
    level_spawn_items(lvl, depth);

    /*10 mobs*/
    mob_spawn_random_no_target(lvl, MOB_RAT, rng_range(1, MAX_LEVEL_MOBS)); //TEST

    return 0;
}

int level_player_on_exit(const Level *lvl, int x, int y){
    return xy_to_idx(x, y) == lvl->exit;
}
int level_player_on_entrance(const Level *lvl, int x, int y){
    return xy_to_idx(x, y) == lvl->entrance;
}