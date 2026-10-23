#pragma once
#include "char.h"
#include "common.h"

typedef enum {
   BLOB_FIRE = 0,
   BLOB_TOXIC_GAS,
   BLOB_PARALYTIC_GAS,
   BLOB_CONFUSION_GAS,
   BLOB_WEB,
   BLOB_FOLIAGE,
   BLOB_REGROWTH,
   BLOB_ALCHEMY,
   BLOB_SACRIFICIAL_FIRE,
   BLOB_WATER_OF_AWARENESS,
   BLOB_WATER_OF_HEALTH,
   BLOB_WATER_OF_TRANSMUTATION,
   BLOB_TYPE_COUNT
} BlobType;

typedef struct Blob Blob;

typedef struct {
   void (*evolve)(Blob *self);
   void (*do_seed)(Blob *self, int cell, int amount);
   int (*affect_hero)(Blob *self, Char *hero);
} BlobVTable;

struct Blob {
   Actor base;
   BlobType type;
   const BlobVTable *vt;

   int volume;

   int *cur;
   int *off;
   int buf_a[LEVEL_SIZE];
   int buf_b[LEVEL_SIZE];

   int pos;     /* single-cell blobs only: WellWater family, Alchemy, Sacrificial Fire */
   int active;
};

typedef struct {
   Blob slots[BLOB_TYPE_COUNT];
} BlobPool;

void blob_set_pool(BlobPool *pool);
void blob_set_depth(int depth);
void blob_set_hero(Char *hero);

Blob *blob_get(BlobType type);
void blob_clear_cell(Blob *b, int cell);

void wellwater_affect_cell(int cell);
void sacrificial_fire_sacrifice(Char *ch);
int freezing_affect(int cell);
void foliage_try_stealth(Char *hero);

const char *blob_tile_desc(BlobType type);

void blob_clear(BlobPool *pool);
void blob_restore_pool(BlobPool *pool);
Blob *blob_seed(int cell, int amount, BlobType type);