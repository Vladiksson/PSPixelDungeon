#pragma once
#include "level.h"
#include "rng.h"

#define LAST_SHOP_DEPTH 20
#define FINAL_LEVEL_DEPTH 25
DungeonTheme get_theme(int depth);
int level_is_boss_depth(int depth);
int shop_on_level(int depth);

void set_tile(Level *lvl, int idx, uint8_t t); 
void fill_rect(Level *lvl, int x, int y, int w, int h, uint8_t t);

int already_connected(Level *lvl, int a, int b);
void room_connect(Level *lvl, int a, int b);


void bsp_split(Level *lvl, int left, int top, int right, int bottom);
void build_neighbours(Level *lvl);
void build_distance_map(Level *lvl, int target);
int  build_path(Level *lvl, int from, int to, int *path);
void set_price(Level *lvl, int *path, int len, int price);



int rooms_are_neighbours(const Level *lvl, int a, int b);