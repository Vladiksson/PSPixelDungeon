#pragma once
#include <stdint.h>
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
   PK_WIND,          /* Chasm updraft, per-tile emitter (not a Blob) — WindParticle */
   PK_FLOW,          /* Water dripping into a chasm, per-tile emitter (not a Blob) — FlowParticle */
   PK_WATER,         /* Sewer WALL_DECO drip, per-tile emitter (not a Blob) — SewerLevel.WaterParticle/Sink */

   /* ── one-shot / triggered effects (not blob- or ambient-driven) ── */
   PK_SPLASH,        /* Colored liquid splash: potion shatter, heap alchemy — Splash + PixelParticle.Shrinking */
   PK_EARTH,         /* Kicked-up earth/dirt: Earthroot, Entanglement glyph — EarthParticle */
   PK_ELMO,          /* St. Elmo's fire sparkle: burning heap, shop/sign shimmer — ElmoParticle */
   PK_SPARK,         /* Electric spark: lightning wand/trap — SparkParticle */
   PK_SMOKE,         /* Smoke puff: bomb aftermath — SmokeParticle */
   PK_BLAST,         /* Explosion flash: bomb — BlastParticle */
   PK_POISON,        /* Toxic splash: poison trap/plant/gas damage — PoisonParticle (SPLASH factory) */
   PK_PURPLE,        /* Dark energy burst: disintegration, eye death gaze — PurpleParticle (BURST factory) */
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

void particle_emit_speck(Level *lvl, int cell, SpeckType type);
void particle_emit_snow(Level *lvl, int cell);
void particle_emit_wind(Level *lvl, int cell);
void particle_emit_flow(Level *lvl, int cell);
void particle_emit_water(Level *lvl, int cell);

void particle_emit_splash(Level *lvl, int cell, uint32_t color); /* potion shatter, heap alchemy puff */
void particle_emit_earth(Level *lvl, int cell);  /* Earthroot / Entanglement dirt kick-up (bottom of cell) */
void particle_emit_elmo(Level *lvl, int cell);   /* burning heap, shopkeeper/sign shimmer */
void particle_emit_spark(Level *lvl, int cell);  /* lightning wand / lightning trap */
void particle_emit_smoke(Level *lvl, int cell);  /* bomb smoke */
void particle_emit_blast(Level *lvl, int cell);  /* bomb explosion flash */
void particle_emit_poison(Level *lvl, int cell); /* poison trap / plant / gas splash */
void particle_emit_purple(Level *lvl, int cell); /* disintegration / eye death-gaze burst */

void particle_clear(Level *lvl);


#define MAX_LEVEL_EMITTERS 32

typedef enum {
   EMITTER_NONE = 0,
   EMITTER_EARTH,
   EMITTER_ELMO,
   EMITTER_SPARK,
   EMITTER_SMOKE,
   EMITTER_BLAST,
   EMITTER_POISON,
   EMITTER_PURPLE,
   EMITTER_SPLASH_POTION,
   EMITTER_SPLASH_BLOOD,
   EMITTER_SPLASH_GRASS,
} EmitterKind;

typedef struct {
   EmitterKind kind;
   int cell;

   float timer;
   float emit_timer;
   float rate;
} ParticleEmitter;

void particle_emitter_start(Level *lvl, EmitterKind kind, int cell, float dt, float rate);
void particle_update_emitters(Level *lvl, float dt, CellVisibleFn is_visible_fn, void *user);

void particle_update_ambient(Level *lvl, float dt, CellVisibleFn is_visible_fn, void *user);