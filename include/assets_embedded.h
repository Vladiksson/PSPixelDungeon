#pragma once
#include <stddef.h>

typedef struct {
    const unsigned char *data;  /* NULL if the name wasn't found */
    size_t                size;
} EmbeddedAsset;

EmbeddedAsset asset_find(const char *name);