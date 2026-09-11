#pragma once
#include "assets.h"
#include <stdint.h>

typedef enum {
    FACE_RIGHT = 0,
    FACE_LEFT  = 1
} Facing;

typedef struct {
    const uint8_t *frames;
    int count;
    int fps;
    int loop;
} AnimClip;