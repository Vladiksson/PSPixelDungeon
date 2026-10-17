#pragma once
#include "blob.h"

typedef struct Level Level;

#define MAX_LEVEL_PARTICLES 256

typedef enum {
   PK_SPECK = 0,     /* textured icon from specks.png */
   PK_FLAME,         /* Fire blob        — FlameParticle */
   PK_WEB,           /* Web blob         — WebParticle   */
   PK_SHAFT,         /* Foliage blob     — ShaftParticle */
   PK_LEAF,          /* Regrowth blob    — LeafParticle  */
   PK_SACRIFICIAL,   /* Sacrificial Fire — SacrificialParticle */
   PK_SNOW,          /* Freezing, via CellEmitter (not a Blob) — SnowParticle */
} ParticleKind;

typedef enum {
   SPECK_HEALING   = 0,
   SPECK_STAR      = 1,
   SPECK_LIGHT     = 2,
   SPECK_QUESTION  = 3,
   SPECK_UP        = 4,
   SPECK_SCREAM    = 5,
   SPECK_BONE      = 6,
   SPECK_WOOL      = 7,
   SPECK_ROCK      = 8,
   SPECK_NOTE      = 9,
   SPECK_CHANGE    = 10,
   SPECK_HEART     = 11,
   SPECK_BUBBLE    = 12,
   SPECK_STEAM     = 13,
   SPECK_COIN      = 14,

   SPECK_DISCOVER  = 101,  /* draws as LIGHT */
   SPECK_EVOKE     = 102,  /* draws as STAR  */
   SPECK_MASTERY   = 103,  /* draws as STAR  */
   SPECK_KIT       = 104,  /* draws as STAR  */
   SPECK_RATTLE    = 105,  /* draws as BONE  */
   SPECK_JET       = 106,  /* draws as STEAM */
   SPECK_TOXIC     = 107,  /* draws as STEAM */
   SPECK_PARALYSIS = 108,  /* draws as STEAM */
   SPECK_DUST      = 109,  /* draws as STEAM */
   SPECK_FORGE     = 110,  /* draws as STAR  */
   SPECK_CONFUSION = 111,  /* draws as STEAM */
} SpeckType;

typedef struct {
    ParticleKind kind;
    int speck_type;        /* valid when kind==PK_SPECK (a SpeckType value) */

    /* position/velocity/accel */
    float x, y;
    float vx, vy;
    float ax, ay;

    float angle;            /* degrees */
    float angular_speed;    /* degrees/sec */
    float scale_x, scale_y; /* multiplies the base sprite/size */
    float size;             
    float size0;

    float left, lifespan;

    float rm, gm, bm, am; /* (R, G, B, alpha) */
    int light_mode;
} Particle;


void particle_update_all(Level *lvl, float dt);
typedef int (*CellVisibleFn)(int cell, void *user);
void particle_update_blobs(Level *lvl, const BlobPool *pool, float dt, CellVisibleFn is_visible_fn, void *user);

void particle_emit_snow(Level *lvl, int cell);

void speck_sprite(int type, uint8_t *x, uint8_t *y, uint8_t *w, uint8_t *h);

void particle_clear(Level *lvl);