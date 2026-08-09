#include "common.h" 
//IDK WHERE PUT THIS STUFF FOR MAP/PLAYER SCALING, SO I CREATE THIS TEMPORARY FILE
int gTileSize = TILE_SIZE_MIN;
int gViewCols;
int gViewRows;

void TileSize_Update(void){
    gViewCols = SCREEN_W / gTileSize;
    gViewRows = SCREEN_H / gTileSize;
}
