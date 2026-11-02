#include "particle.h"
#include "level.h"
#include "level_geometry.h"

/* ── color helpers ─────────────────────────────────────────── */
static void color_set(Particle *p, uint32_t hex) {
    p->rm = ((hex >> 16) & 0xFF) / 255.0f;
    p->gm = ((hex >> 8)  & 0xFF) / 255.0f;
    p->bm = ( hex         & 0xFF) / 255.0f;
}

static void color_lerp(Particle *p, uint32_t c1, uint32_t c2, float t) {
    if (t <= 0.0f) { color_set(p, c1); return; }
    if (t >= 1.0f) { color_set(p, c2); return; }
    int r1 = (c1 >> 16) & 0xFF, g1 = (c1 >> 8) & 0xFF, b1 = c1 & 0xFF;
    int r2 = (c2 >> 16) & 0xFF, g2 = (c2 >> 8) & 0xFF, b2 = c2 & 0xFF;
    float t1 = 1.0f - t;
    p->rm = (t1 * r1 + t * r2) / 255.0f;
    p->gm = (t1 * g1 + t * g2) / 255.0f;
    p->bm = (t1 * b1 + t * b2) / 255.0f;
}

static void color_lerp_random(Particle *p, uint32_t c1, uint32_t c2) {
    float t = rng_float();
    color_lerp(p, c1, c2, t);
}
/* ────────────────────────────────────────────────────────────────────── */

static Particle *spawn_slot(Level *lvl) {
    if (!lvl) return NULL;
    if (lvl->num_particles >= MAX_LEVEL_PARTICLES) return NULL;

    Particle *p = &lvl->particles[lvl->num_particles++];
    memset(p, 0, sizeof(*p));
    p->scale_x = p->scale_y = 1.0f;
    p->size    = 1.0f;
    p->rm = p->gm = p->bm =  1.0f;
    p->am = 0.0f;
    return p;
}

static Particle *spawn_speck(Level *lvl, int index, float x, float y, int type) {
    Particle *p = spawn_slot(lvl);
    if (!p) return NULL;
    p->kind = PK_SPECK;
    p->speck_type = type;
    p->x = x; p->y = y;

    switch (type) {
    case SPECK_HEALING:
        p->vy = -20.0f;
        p->lifespan = 1.0f;
        break;

    case SPECK_STAR: {
        float a   = rng_float() * (2.0f * M_PI);
        float len = rng_float() * 128.0f;
        p->vx = len * cosf(a);
        p->vy = len * sinf(a);
        p->ay = 128.0f;
        p->angle = rng_float() * 360.0f;
        p->angular_speed = rng_float_range(-360.0f, 360.0f);
        p->lifespan = 1.0f;
        break;
    }

    case SPECK_FORGE: {
        float a   = -(rng_float() * M_PI);
        float len = rng_float() * 64.0f;
        p->vx = len * cosf(a);
        p->vy = len * sinf(a);
        p->ay = 128.0f;
        p->angle = rng_float() * 360.0f;
        p->angular_speed = rng_float_range(-360.0f, 360.0f);
        p->lifespan = 0.51f;
        break;
    }

    case SPECK_EVOKE: {
        float a = -(rng_float() * M_PI);
        p->vx = 50.0f * cosf(a);
        p->vy = 50.0f * sinf(a);
        p->ay = 50.0f;
        p->angle = rng_float() * 360.0f;
        p->angular_speed = rng_float_range(-180.0f, 180.0f);
        p->lifespan = 1.0f;
        break;
    }

    case SPECK_KIT: {
        float a = (float)index * M_PI / 5.0f;
        p->vx = 50.0f * cosf(a);
        p->vy = 50.0f * sinf(a);
        p->ax = -p->vx;
        p->ay = -p->vy;
        p->angle = (float)index * 36.0f;
        p->angular_speed = 360.0f;
        p->lifespan = 1.0f;
        break;
    }

    case SPECK_MASTERY:
        p->vx = (rng_int(2) == 0) ? rng_float_range(-128.0f, -64.0f) : rng_float_range(64.0f, 128.0f);
        p->angular_speed = (p->vx < 0.0f) ? -180.0f : 180.0f;
        p->ax = -p->vx;
        p->lifespan = 0.5f;
        break;

    case SPECK_LIGHT:
        p->angle = rng_float() * 360.0f;
        p->angular_speed = 90.0f;
        p->lifespan = 1.0f;
        break;

    case SPECK_DISCOVER:
        p->angle = rng_float() * 360.0f;
        p->angular_speed = 90.0f;
        p->lifespan = 0.5f;
        break;

    case SPECK_QUESTION:
        p->lifespan = 0.8f;
        p->am = 1.0f;
        break;

    case SPECK_UP:
        p->vy = -20.0f;
        p->lifespan = 1.0f;
        p->am = 1.0f;
        break;

    case SPECK_SCREAM:
        p->lifespan = 0.9f;
        break;

    case SPECK_BONE: {
        p->lifespan = 0.2f;
        float a   = rng_float() * (2.0f * M_PI);
        float len = 24.0f / p->lifespan;
        p->vx = len * cosf(a);
        p->vy = len * sinf(a);
        p->ay = 128.0f;
        p->angle = rng_float() * 360.0f;
        p->angular_speed = 360.0f;
        break;
    }

    case SPECK_RATTLE:
        p->lifespan = 0.5f;
        p->vy = -200.0f;
        p->ay = -2.0f * p->vy / p->lifespan;
        p->angle = rng_float() * 360.0f;
        p->angular_speed = 360.0f;
        break;

    case SPECK_WOOL:
        p->lifespan = 0.5f;
        p->vy = -50.0f;
        p->angle = rng_float() * 360.0f;
        p->angular_speed = rng_float_range(-360.0f, 360.0f);
        break;

    case SPECK_ROCK:
        p->angle = rng_float() * 360.0f;
        p->angular_speed = rng_float_range(-360.0f, 360.0f);
        p->scale_x = p->scale_y = rng_float_range(1.0f, 2.0f);
        p->vy = 64.0f;
        p->lifespan = 0.2f;
        p->y -= p->vy * p->lifespan;
        break;

    case SPECK_NOTE: {
        p->angular_speed = rng_float_range(-30.0f, 30.0f);
        float rad = (p->angular_speed - 90.0f) * (M_PI / 180.0f);
        p->vx = 30.0f * cosf(rad);
        p->vy = 30.0f * sinf(rad);
        p->lifespan = 1.0f;
        break;
    }

    case SPECK_CHANGE: {
        p->angle = rng_float() * 360.0f;
        float rad = (p->angle - 90.0f) * (M_PI / 180.0f);
        float len = rng_float_range(4.0f, 12.0f);
        p->vx = len * cosf(rad);
        p->vy = len * sinf(rad);
        p->lifespan = 1.5f;
        break;
    }

    case SPECK_HEART:
        p->vx = (float)rng_range(-10, 10);
        p->vy = -40.0f;
        p->angular_speed = rng_float_range(-45.0f, 45.0f);
        p->lifespan = 1.0f;
        break;

    case SPECK_BUBBLE:
        p->vy = -15.0f;
        p->scale_x = p->scale_y = rng_float_range(0.8f, 1.0f);
        p->lifespan = rng_float_range(0.8f, 1.5f);
        break;

    case SPECK_STEAM:
        p->vy = -rng_float_range(20.0f, 30.0f);
        p->angular_speed = rng_float() * 180.0f;
        p->angle = rng_float() * 360.0f;
        p->lifespan = 1.0f;
        break;

    case SPECK_JET:
        p->vy = 32.0f;
        p->ay = -64.0f;
        p->angular_speed = rng_float_range(180.0f, 360.0f);
        p->angle = rng_float() * 360.0f;
        p->lifespan = 0.5f;
        break;

    case SPECK_TOXIC:
        color_set(p, 0x50FF60);
        p->angular_speed = 30.0f;
        p->angle = rng_float() * 360.0f;
        p->lifespan = rng_float_range(1.0f, 3.0f);
        break;

    case SPECK_PARALYSIS:
        color_set(p, 0xFFFF66);
        p->angular_speed = -30.0f;
        p->angle = rng_float() * 360.0f;
        p->lifespan = rng_float_range(1.0f, 3.0f);
        break;

    case SPECK_CONFUSION:
        color_set(p, ((uint32_t)rng_int(0x1000000)) | 0x000080u);
        p->angular_speed = rng_float_range(-20.0f, 20.0f);
        p->angle = rng_float() * 360.0f;
        p->lifespan = rng_float_range(1.0f, 3.0f);
        break;

    case SPECK_DUST: {
        color_set(p, 0xFFFF66);
        p->angle = rng_float() * 360.0f;
        float a   = rng_float() * (2.0f * M_PI);
        float len = rng_float_range(16.0f, 48.0f);
        p->vx = len * cosf(a);
        p->vy = len * sinf(a);
        p->lifespan = 0.5f;
        break;
    }

    case SPECK_COIN: {
        float a   = -M_PI * rng_float_range(0.3f, 0.7f);
        float len = rng_float_range(48.0f, 96.0f);
        p->vx = len * cosf(a);
        p->vy = len * sinf(a);
        p->ay = 256.0f;
        p->lifespan = -p->vy / p->ay * 2.0f;
        break;
    }

    default:
        p->lifespan = 1.0f;
        break;
    }
    p->left = p->lifespan;
    return p;
}

static Particle *spawn_flame(Level *lvl, float x, float y) {
    Particle *p = spawn_slot(lvl);
    if (!p) return NULL;
    p->kind = PK_FLAME;
    color_set(p, 0xEE7722);
    p->lifespan = 0.6f;
    p->ay = -80.0f;
    p->x = x; p->y = y;
    p->size = p->size0 = 4.0f;
    p->left = p->lifespan;
    return p;
}

static Particle *spawn_sacrificial(Level *lvl, float x, float y) {
    Particle *p = spawn_slot(lvl);
    if (!p) return NULL;
    p->kind = PK_SACRIFICIAL;
    color_set(p, 0x4488EE);
    p->lifespan = 0.6f;
    p->ay = -100.0f;
    p->x = x; p->y = y - 4.0f;
    p->size = p->size0 = 4.0f;
    p->left = p->lifespan;
    return p;
}

static Particle *spawn_web_one(Level *lvl, float x, float y) {
    Particle *p = spawn_slot(lvl);
    if (!p) return NULL;
    p->kind = PK_WEB;
    color_set(p, 0xCCCCCC);
    p->lifespan = 2.0f;
    p->x = x; p->y = y;
    p->angle = rng_float() * 360.0f;
    p->left = p->lifespan;
    return p;
}

static Particle *spawn_shaft(Level *lvl, float x, float y) {
    Particle *p = spawn_slot(lvl);
    if (!p) return NULL;
    p->kind = PK_SHAFT;
    p->vy = -6.0f;
    p->lifespan = 1.2f;
    p->x = x; p->y = y;
    float offs = -rng_float_range(0.0f, p->lifespan);
    p->left = p->lifespan - offs;
    return p;
}

static Particle *spawn_leaf(Level *lvl, float x, float y) {
    Particle *p = spawn_slot(lvl);
    if (!p) return NULL;
    p->kind = PK_LEAF;
    color_lerp_random(p, 0x004400, 0x88CC44);
    p->lifespan = 1.2f;
    p->ay = 25.0f;
    p->x = x; p->y = y;
    p->vx = rng_float_range(-8.0f, 8.0f);
    p->vy = -20.0f;
    p->size = p->size0 = rng_float_range(2.0f, 3.0f);
    p->am = 1.0f;
    p->left = p->lifespan;
    return p;
}

static Particle *spawn_snow(Level *lvl, float x, float y) {
    Particle *p = spawn_slot(lvl);
    if (!p) return NULL;
    p->kind = PK_SNOW;
    p->lifespan = 1.2f;
    p->vy = rng_float_range(5.0f, 8.0f);
    p->x = x;
    p->y = y - p->vy * p->lifespan;
    p->left = p->lifespan;
    return p;
}

static int   wind_seeded = 0;
static float wind_angle  = 0.0f;
static float wind_vx = 0.0f, wind_vy = 0.0f;

static void wind_seed(void) {
    if (!wind_seeded) {
        wind_angle  = rng_float() * (2.0f * M_PI);
        wind_vx     = 5.0f * cosf(wind_angle);
        wind_vy     = 5.0f * sinf(wind_angle);
        wind_seeded = 1;
    }
}

static Particle *spawn_wind(Level *lvl, float x, float y) {
    Particle *p = spawn_slot(lvl);
    if (!p) return NULL;
    wind_seed();

    p->kind = PK_WIND;
    p->lifespan = rng_float_range(1.0f, 2.0f);

    float size = rng_float() * 3.0f;
    p->scale_x = p->scale_y = size;

    float vx = wind_vx * size;
    float vy = wind_vy * size;
    p->vx = vx;
    p->vy = vy;

    p->x = x - vx * p->lifespan * 0.5f;
    p->y = y - vy * p->lifespan * 0.5f;

    wind_angle += rng_float_range(-0.1f, 0.1f);
    wind_vx = 5.0f * cosf(wind_angle);
    wind_vy = 5.0f * sinf(wind_angle);

    p->left = p->lifespan;
    return p;
}

static Particle *spawn_flow(Level *lvl, float x, float y) {
    Particle *p = spawn_slot(lvl);
    if (!p) return NULL;
    p->kind = PK_FLOW;
    p->lifespan = 0.6f;
    p->ay = 32.0f;
    p->angular_speed = rng_float_range(-360.0f, 360.0f);
    p->x = x; p->y = y;
    p->size = 0.0f;
    p->left = p->lifespan;
    return p;
}

static Particle *spawn_water(Level *lvl, float x, float y) {
    Particle *p = spawn_slot(lvl);
    if (!p) return NULL;
    p->kind = PK_WATER;
    p->ay = 50.0f;
    p->am = 0.5f;
    color_lerp_random(p, 0xb6ccc2, 0x3b6653);
    p->size = 2.0f;
    p->x = x; p->y = y;
    p->vx = rng_float_range(-2.0f, 2.0f);
    p->vy = 0.0f;
    p->lifespan = 0.5f;
    p->left = p->lifespan;
    return p;
}

static Particle *spawn_splash(Level *lvl, float x, float y, uint32_t color) {
    Particle *p = spawn_slot(lvl);
    if (!p) return NULL;
    p->kind = PK_SPLASH;
    color_set(p, color);
    p->am = 1.0f;
    p->x = x; p->y = y;
    float dir  = -M_PI / 2.0f;
    float cone = M_PI;
    float a    = rng_float_range(dir - cone * 0.5f, dir + cone * 0.5f);
    float len  = rng_float_range(40.0f, 80.0f);
    p->vx = len * cosf(a);
    p->vy = len * sinf(a);
    p->ay = 100.0f;
    p->size = p->size0 = 4.0f;
    p->lifespan = rng_float_range(0.5f, 1.0f);
    p->left = p->lifespan;
    return p;
}

static Particle *spawn_earth(Level *lvl, float x, float y) {
    Particle *p = spawn_slot(lvl);
    if (!p) return NULL;
    p->kind = PK_EARTH;
    color_lerp_random(p, 0x444444, 0x777766);
    p->am = 1.0f;
    p->size = 0.0f;
    p->angle = rng_float_range(-30.0f, 30.0f);
    p->x = x; p->y = y;
    p->lifespan = 0.5f;
    p->left = p->lifespan;
    return p;
}

static Particle *spawn_elmo(Level *lvl, float x, float y) {
    Particle *p = spawn_slot(lvl);
    if (!p) return NULL;
    p->kind = PK_ELMO;
    color_set(p, 0x22EE66);
    p->lifespan = 0.6f;
    p->ay = -80.0f;
    p->x = x; p->y = y;
    p->size = p->size0 = 4.0f;
    p->left = p->lifespan;
    return p;
}

static Particle *spawn_spark(Level *lvl, float x, float y) {
    Particle *p = spawn_slot(lvl);
    if (!p) return NULL;
    p->kind = PK_SPARK;
    p->am = 1.0f;
    p->size = 2.0f;
    p->ay = 50.0f;
    p->x = x; p->y = y;
    float a   = -(rng_float() * M_PI);
    float len = rng_float_range(20.0f, 40.0f);
    p->vx = len * cosf(a);
    p->vy = len * sinf(a);
    p->lifespan = rng_float_range(0.5f, 1.0f);
    p->left = p->lifespan;
    return p;
}

static Particle *spawn_smoke(Level *lvl, float x, float y) {
    Particle *p = spawn_slot(lvl);
    if (!p) return NULL;
    p->kind = PK_SMOKE;
    color_set(p, 0x222222);
    p->ay = -40.0f;
    p->x = x; p->y = y;
    p->vx = rng_float_range(-4.0f, 4.0f);
    p->vy = rng_float_range(-8.0f, 8.0f);
    p->lifespan = rng_float_range(0.6f, 1.0f);
    p->left = p->lifespan;
    p->size = 8.0f;
    return p;
}

static Particle *spawn_blast(Level *lvl, float x, float y) {
    Particle *p = spawn_slot(lvl);
    if (!p) return NULL;
    p->kind = PK_BLAST;
    color_set(p, 0xEE7722);
    p->ay = 50.0f;
    p->x = x; p->y = y;
    p->size = p->size0 = 8.0f;
    float a   = -(rng_float() * M_PI);
    float len = rng_float_range(32.0f, 64.0f);
    p->vx = len * cosf(a);
    p->vy = len * sinf(a);
    p->lifespan = rng_float();
    p->left = p->lifespan;
    return p;
}

static Particle *spawn_poison(Level *lvl, float x, float y) {
    Particle *p = spawn_slot(lvl);
    if (!p) return NULL;
    p->kind = PK_POISON;
    color_set(p, 0x8844FF);
    p->am = 1.0f;
    p->ay = 30.0f;
    p->x = x; p->y = y;
    float a   = rng_float() * M_PI;
    float len = rng_float_range(10.0f, 20.0f);
    p->vx = len * cosf(a);
    p->vy = len * sinf(a);
    p->lifespan = 0.6f;
    p->left = p->lifespan;
    return p;
}

static Particle *spawn_purple(Level *lvl, float x, float y) {
    Particle *p = spawn_slot(lvl);
    if (!p) return NULL;
    p->kind = PK_PURPLE;
    color_set(p, 0xFF0044);
    p->am = 1.0f;
    p->x = x; p->y = y;
    float a   = rng_float() * (2.0f * M_PI);
    float len = rng_float_range(16.0f, 32.0f);
    p->vx = len * cosf(a);
    p->vy = len * sinf(a);
    p->lifespan = 0.5f;
    p->left = p->lifespan;
    return p;
}

/* wand bolt trail particles */
static Particle *spawn_trail_magic(Level *lvl, float x, float y) {
    Particle *p = spawn_slot(lvl);
    if (!p) return NULL;
    p->kind = PK_TRAIL_MAGIC;
    color_set(p, 0x88CCFF);
    p->lifespan = 0.5f;
    p->x = x; p->y = y;
    p->vx = rng_float_range(-10.0f, 10.0f);
    p->vy = rng_float_range(-10.0f, 10.0f);
    p->am = 1.0f;
    p->left = p->lifespan;
    return p;
}

static Particle *spawn_trail_earth(Level *lvl, float x, float y) {
    Particle *p = spawn_slot(lvl);
    if (!p) return NULL;
    p->kind = PK_TRAIL_EARTH;
    color_lerp_random(p, 0x555555, 0x777766);
    p->lifespan = 0.5f;
    p->ay = 40.0f;
    p->x = x; p->y = y;
    p->vx = rng_float_range(-10.0f, 10.0f);
    p->vy = rng_float_range(-10.0f, 10.0f);
    p->size = p->size0 = 4.0f;
    p->am = 1.0f;
    p->left = p->lifespan;
    return p;
}

static Particle *spawn_trail_white(Level *lvl, float x, float y) {
    Particle *p = spawn_slot(lvl);
    if (!p) return NULL;
    p->kind = PK_TRAIL_WHITE;
    color_set(p, 0xFFFFFF);
    p->lifespan = 0.4f;
    p->x = x; p->y = y;
    p->size = p->size0 = 3.0f;
    p->am = 1.0f;
    p->left = p->lifespan;
    return p;
}

static Particle *spawn_trail_slow(Level *lvl, float x, float y) {
    Particle *p = spawn_slot(lvl);
    if (!p) return NULL;
    p->kind = PK_TRAIL_SLOW;
    color_set(p, 0x664422);
    p->lifespan = 0.6f;
    p->x = x; p->y = y;
    p->vx = rng_float_range(-20.0f, 20.0f);
    p->vy = rng_float_range(-20.0f, 20.0f);
    p->size = p->size0 = 2.0f;
    p->am = 1.0f;
    p->left = p->lifespan;
    return p;
}

static Particle *spawn_trail_force(Level *lvl, int index, float x, float y) {
    Particle *p = spawn_slot(lvl);
    if (!p) return NULL;
    p->kind = PK_TRAIL_FORCE;
    color_set(p, 0xFFFFFF);
    p->lifespan = 0.5f;
    p->size = p->size0 = 8.0f;
    float a = (float)(index % 8) * (2.0f * (float)M_PI / 8.0f);
    p->vx = 12.0f * cosf(a);
    p->vy = 12.0f * sinf(a);
    p->x = x - p->vx * p->lifespan;
    p->y = y - p->vy * p->lifespan;
    p->am = 0.0f;
    p->left = p->lifespan;
    return p;
}

static Particle *spawn_trail_cold(Level *lvl, float x, float y) {
    Particle *p = spawn_slot(lvl);
    if (!p) return NULL;
    p->kind = PK_TRAIL_COLD;
    color_set(p, 0x2244FF);
    p->lifespan = 0.6f;
    p->x = x; p->y = y;
    p->size = p->size0 = 8.0f;
    p->am = 0.0f;
    p->left = p->lifespan;
    return p;
}

static Particle *spawn_trail_wool(Level *lvl, float x, float y) {
    Particle *p = spawn_slot(lvl);
    if (!p) return NULL;
    p->kind = PK_TRAIL_WOOL;
    color_lerp_random(p, 0x999999, 0xEEEEE0);
    p->ay = -40.0f;
    p->x = x; p->y = y;
    p->vx = rng_float_range(-10.0f, 10.0f);
    p->vy = rng_float_range(-10.0f, 10.0f);
    p->size = p->size0 = 5.0f;
    p->am = 1.0f;
    p->lifespan = rng_float_range(0.6f, 1.0f);
    p->left = p->lifespan;
    return p;
}

static Particle *spawn_trail_poison(Level *lvl, float x, float y) {
    Particle *p = spawn_slot(lvl);
    if (!p) return NULL;
    p->kind = PK_POISON;
    color_set(p, 0x8844FF);
    p->am = 1.0f;
    p->ay = 30.0f;
    p->x = x; p->y = y;
    float a   = -rng_float() * (float)M_PI;
    float len = rng_float() * 6.0f;
    p->vx = len * cosf(a);
    p->vy = len * sinf(a);
    p->lifespan = 0.6f;
    p->left = p->lifespan;
    return p;
}

static Particle *spawn_trail_purple(Level *lvl, float x, float y) {
    Particle *p = spawn_slot(lvl);
    if (!p) return NULL;
    p->kind = PK_PURPLE;
    color_set(p, 0xFF0044);
    p->am = 1.0f;
    p->x = x; p->y = y;
    p->vx = rng_float_range(-5.0f, 5.0f);
    p->vy = rng_float_range(-5.0f, 5.0f);
    p->lifespan = 0.5f;
    p->left = p->lifespan;
    return p;
}

void particle_update_all(Level *lvl, float dt) {
    if (!lvl) return;

    for (int i = 0; i < lvl->num_particles; ) {
        Particle *p = &lvl->particles[i];

        p->x += p->vx * dt + 0.5f * p->ax * dt * dt;
        p->y += p->vy * dt + 0.5f * p->ay * dt * dt;
        p->vx += p->ax * dt;
        p->vy += p->ay * dt;
        p->angle += p->angular_speed * dt;

        p->left -= dt;
        if (p->left <= 0.0f) {
            lvl->particles[i] = lvl->particles[--lvl->num_particles];
            continue;
        }

        switch (p->kind) {

        case PK_SPECK: {
            float prog = 1.0f - p->left / p->lifespan;
            switch (p->speck_type) {
            case SPECK_STAR:
            case SPECK_FORGE:
                p->scale_x = p->scale_y = 1.0f - prog;
                p->am = (prog < 0.2f) ? prog * 5.0f : (1.0f - prog) * 1.25f;
                break;
            case SPECK_KIT:
            case SPECK_MASTERY:
                p->am = 1.0f - prog * prog;
                break;
            case SPECK_EVOKE:
            case SPECK_HEALING:
                p->am = (prog < 0.5f) ? 1.0f : 2.0f - prog * 2.0f;
                break;
            case SPECK_LIGHT: {
                float v = (prog < 0.2f) ? prog * 5.0f : (1.0f - prog) * 1.25f;
                p->scale_x = p->scale_y = v;
                p->am = v;
                break;
            }
            case SPECK_DISCOVER:
                p->am = 1.0f - prog;
                p->scale_x = p->scale_y = (prog < 0.5f ? prog : 1.0f - prog) * 2.0f;
                break;
            case SPECK_QUESTION:
                p->scale_x = p->scale_y =
                    sqrtf(prog < 0.5f ? prog : 1.0f - prog) * 3.0f;
                break;
            case SPECK_UP:
                p->scale_x = p->scale_y =
                    sqrtf(prog < 0.5f ? prog : 1.0f - prog) * 2.0f;
                break;
            case SPECK_SCREAM:
                p->am = sqrtf((prog < 0.5f ? prog : 1.0f - prog) * 2.0f);
                p->scale_x = p->scale_y = prog * 7.0f;
                break;
            case SPECK_BONE:
            case SPECK_RATTLE:
                p->am = (prog < 0.9f) ? 1.0f : (1.0f - prog) * 10.0f;
                break;
            case SPECK_ROCK:
                p->am = (prog < 0.2f) ? prog * 5.0f : 1.0f;
                break;
            case SPECK_NOTE:
                p->am = 1.0f - prog * prog;
                break;
            case SPECK_WOOL:
                p->scale_x = p->scale_y = 1.0f - prog;
                break;
            case SPECK_CHANGE:
                p->am = sqrtf((prog < 0.5f ? prog : 1.0f - prog) * 2.0f);
                p->scale_y = (1.0f + prog) * 0.5f;
                p->scale_x = p->scale_y * cosf(p->left * 15.0f);
                break;
            case SPECK_HEART:
                p->scale_x = p->scale_y = 1.0f - prog;
                p->am = 1.0f - prog * prog;
                break;
            case SPECK_BUBBLE:
                p->am = (prog < 0.2f) ? prog * 5.0f : 1.0f;
                break;
            case SPECK_STEAM:
            case SPECK_TOXIC:
            case SPECK_PARALYSIS:
            case SPECK_CONFUSION:
            case SPECK_DUST:
                p->am = (prog < 0.5f) ? prog : 1.0f - prog;
                p->scale_x = p->scale_y = 1.0f + prog * 2.0f;
                break;
            case SPECK_JET:
                p->am = ((prog < 0.5f) ? prog : 1.0f - prog) * 2.0f;
                p->scale_x = p->scale_y = prog * 1.5f;
                break;
            case SPECK_COIN:
                p->scale_x = cosf(p->left * 5.0f);
                p->rm = p->gm = p->bm = (fabsf(p->scale_x) + 1.0f) * 0.5f;
                p->am = (prog < 0.9f) ? 1.0f : (1.0f - prog) * 10.0f;
                break;
            default:
                break;
            }
            break;
        }

        case PK_FLAME: {
            float prog = p->left / p->lifespan;
            p->am = (prog > 0.8f) ? (1.0f - prog) * 5.0f : 1.0f;
            p->size = p->size0 * prog;
            break;
        }

        case PK_SACRIFICIAL: {
            float prog = p->left / p->lifespan;
            p->am = (prog > 0.75f) ? (1.0f - prog) * 4.0f : 1.0f;
            p->size = p->size0 * prog;
            break;
        }

        case PK_WEB: {
            float prog = p->left / p->lifespan;
            p->am = (prog < 0.5f) ? prog : 1.0f - prog;
            p->scale_y = 16.0f + prog * 8.0f;
            break;
        }

        case PK_SHAFT: {
            float prog = p->left / p->lifespan;
            p->am = (prog < 0.5f) ? prog : 1.0f - prog;
            p->scale_x = (1.0f - prog) * 4.0f;
            p->scale_y = 16.0f + (1.0f - prog) * 16.0f;
            break;
        }

        case PK_LEAF: {
            float prog = p->left / p->lifespan;
            p->size = p->size0 * prog;
            break;
        }

        case PK_SNOW: {
            float prog = p->left / p->lifespan;
            p->am = ((prog < 0.5f) ? prog : 1.0f - prog) * 1.5f;
            break;
        }

        case PK_WIND: {
            float prog = p->left / p->lifespan;
            p->am = (prog < 0.5f ? prog : 1.0f - prog) * p->scale_x * 0.2f;
            break;
        }

        case PK_FLOW: {
            float prog = p->left / p->lifespan;
            p->am = (prog < 0.5f ? prog : 1.0f - prog) * 0.6f;
            p->size = (1.0f - prog) * 4.0f;
            break;
        }

        case PK_WATER:
            break;

        case PK_SPLASH: {
            float prog = p->left / p->lifespan;
            p->size = p->size0 * prog;
            break;
        }

        case PK_EARTH: {
            float prog = p->left / p->lifespan;
            p->size = (prog < 0.5f ? prog : 1.0f - prog) * 16.0f;
            break;
        }

        case PK_ELMO: {
            float prog = p->left / p->lifespan;
            p->am = (prog > 0.8f) ? (1.0f - prog) * 5.0f : 1.0f;
            p->size = p->size0 * prog;
            break;
        }

        case PK_SPARK:
            p->size = rng_float() * (5.0f * p->left / p->lifespan);
            break;

        case PK_SMOKE: {
            float prog = p->left / p->lifespan;
            p->am = (prog > 0.8f) ? (2.0f - 2.0f * prog) : (prog * 0.5f);
            p->size = 16.0f - prog * 8.0f;
            break;
        }

        case PK_BLAST: {
            float prog = p->left / p->lifespan;
            p->am = (prog > 0.8f) ? (1.0f - prog) * 5.0f : 1.0f;
            p->size = p->size0 * prog;
            break;
        }

        case PK_POISON:
            /* alpha: 1 -> 0; size: 1 -> 4; color: violet -> green */
            p->am = p->left / p->lifespan;
            p->size = 4.0f - p->am * 3.0f;
            color_lerp(p, 0x00FF00, 0x8844FF, p->am);
            break;

        case PK_PURPLE:
            /* alpha: 1 -> 0; size: 1 -> 5; color: pink -> dark purple */
            p->am = p->left / p->lifespan;
            p->size = 5.0f - p->am * 4.0f;
            color_lerp(p, 0x220066, 0xFF0044, p->am);
            break;

        case PK_TRAIL_MAGIC:
            /* alpha: 1 -> 0; size: 1 -> 4 */
            p->am = p->left / p->lifespan;
            p->size = 4.0f - p->am * 3.0f;
            break;

        case PK_TRAIL_EARTH: {
            float prog = p->left / p->lifespan;
            p->size = p->size0 * prog;
            break;
        }

        case PK_TRAIL_WHITE: {
            float prog = p->left / p->lifespan;
            p->size = p->size0 * prog;
            break;
        }

        case PK_TRAIL_SLOW:
            p->am = p->left / p->lifespan;
            break;

        case PK_TRAIL_FORCE: {
            float prog = p->left / p->lifespan;
            p->size = p->size0 * prog;
            p->am = (1.0f - prog) * 0.5f;
            break;
        }

        case PK_TRAIL_COLD: {
            float prog = p->left / p->lifespan;
            p->size = p->size0 * prog;
            p->am = 1.0f - prog;
            break;
        }

        case PK_TRAIL_WOOL: {
            float prog = p->left / p->lifespan;
            p->size = p->size0 * prog;
            break;
        }

        }

        i++;
    }
}

/* BLOBS particles */
typedef struct {
    int active;
    ParticleKind kind;
    int speck_type;
    float rate;
    int spawn_count;
    int light_mode;
} BlobFx;

static const BlobFx BLOB_FX[BLOB_TYPE_COUNT] = {
    [BLOB_FIRE]                    = { 1, PK_FLAME,       0,               0.03f, 1, 1 },
    [BLOB_TOXIC_GAS]               = { 1, PK_SPECK,  SPECK_TOXIC,          0.6f,  1, 0 },
    [BLOB_PARALYTIC_GAS]           = { 1, PK_SPECK,  SPECK_PARALYSIS,      0.6f,  1, 0 },
    [BLOB_CONFUSION_GAS]           = { 1, PK_SPECK,  SPECK_CONFUSION,      0.6f,  1, 1 },
    [BLOB_WEB]                     = { 1, PK_WEB,         0,               0.4f,  3, 0 },
    [BLOB_FOLIAGE]                 = { 1, PK_SHAFT,       0,               0.9f,  1, 1 },
    [BLOB_REGROWTH]                = { 1, PK_LEAF,        0,               0.2f,  1, 0 },
    [BLOB_ALCHEMY]                 = { 1, PK_SPECK,  SPECK_BUBBLE,         0.4f,  1, 0 },
    [BLOB_SACRIFICIAL_FIRE]        = { 1, PK_SACRIFICIAL, 0,               0.04f, 1, 1 },
    [BLOB_WATER_OF_AWARENESS]      = { 1, PK_SPECK,  SPECK_QUESTION,       0.3f,  1, 0 },
    [BLOB_WATER_OF_HEALTH]         = { 1, PK_SPECK,  SPECK_HEALING,        0.5f,  1, 0 },
    [BLOB_WATER_OF_TRANSMUTATION]  = { 1, PK_SPECK,  SPECK_CHANGE,         0.2f,  1, 0 },
};

static float blob_emit_timer[BLOB_TYPE_COUNT];

static void seed_blob_emit_timers(void) {
    for (int t = 0; t < BLOB_TYPE_COUNT; t++)
        blob_emit_timer[t] = rng_float() * BLOB_FX[t].rate;
}

static void emit_blob_tick(Level *lvl, const Blob *b, const BlobFx *fx, CellVisibleFn is_visible_fn, void *user) {
    for (int i = 0; i < LEVEL_SIZE; i++) {
        if (b->cur[i] <= 0) continue;
        if (is_visible_fn && !is_visible_fn(i, user)) continue;

        Point cell = idx_to_xy(i);
        float x = (cell.x + rng_float()) * (float)TILE_SIZE_MIN;
        float y = (cell.y + rng_float()) * (float)TILE_SIZE_MIN;

        for (int n = 0; n < fx->spawn_count; n++) {
            Particle *p = NULL;

            switch (fx->kind) {
                case PK_SPECK:       p = spawn_speck(lvl, 0, x, y, fx->speck_type); break;
                case PK_FLAME:       p = spawn_flame(lvl, x, y); break;
                case PK_WEB:         p = spawn_web_one(lvl, x, y); break;
                case PK_SHAFT:       p = spawn_shaft(lvl, x, y); break;
                case PK_LEAF:        p = spawn_leaf(lvl, x, y); break;
                case PK_SACRIFICIAL: p = spawn_sacrificial(lvl, x, y); break;
                default: break;
            }
            if (p) p->light_mode = fx->light_mode;
        }
    }
}

void particle_update_blobs(Level *lvl, const BlobPool *pool, float dt, CellVisibleFn is_visible_fn, void *user) {
    if (!lvl || !pool) return;

    for (int t = 0; t < BLOB_TYPE_COUNT; t++) {
        const BlobFx *fx = &BLOB_FX[t];
        if (!fx->active) continue;

        const Blob *b = &pool->slots[t];
        if (b->volume <= 0) continue;

        blob_emit_timer[t] -= dt;

        while (blob_emit_timer[t] < 0.0f) {
            blob_emit_timer[t] += fx->rate;
            emit_blob_tick(lvl, b, fx, is_visible_fn, user);
        }
    }
}

/* EMITTERS */
void particle_emit_speck(Level *lvl, int cell, SpeckType type) {
    Point c = idx_to_xy(cell);
    float x = (c.x + rng_float()) * (float)TILE_SIZE_MIN;
    float y = (c.y + rng_float()) * (float)TILE_SIZE_MIN;
    spawn_speck(lvl, 0, x, y, type);
}

void particle_emit_snow(Level *lvl, int cell) { //TODO use with freezing_affect
    Point c = idx_to_xy(cell);
    float x = (c.x + rng_float()) * (float)TILE_SIZE_MIN;
    float y = (c.y + 0.75f) * (float)TILE_SIZE_MIN;
    spawn_snow(lvl, x, y);
}

void particle_emit_wind(Level *lvl, int cell) {
    Point c = idx_to_xy(cell);
    float x = (c.x + rng_float()) * (float)TILE_SIZE_MIN;
    float y = (c.y + rng_float()) * (float)TILE_SIZE_MIN;
    spawn_wind(lvl, x, y);
}

void particle_emit_flow(Level *lvl, int cell) {
    Point c = idx_to_xy(cell);
    float x = (c.x + rng_float()) * (float)TILE_SIZE_MIN;
    float y = (c.y ) * (float)TILE_SIZE_MIN - 1.0f;
    spawn_flow(lvl, x, y);
}

void particle_emit_water(Level *lvl, int cell) {
    Point c = idx_to_xy(cell);
    float cx = (c.x + 0.5f) * (float)TILE_SIZE_MIN;
    float cy = (c.y + 0.5f) * (float)TILE_SIZE_MIN;
    float x = (cx - 2.0f) + rng_float() * 4.0f; 
    float y = cy + 1.0f;
    spawn_water(lvl, x, y);
}

void particle_emit_splash(Level *lvl, int cell, uint32_t color) {
    Point c = idx_to_xy(cell);
    float x = (c.x + 0.5f) * (float)TILE_SIZE_MIN;
    float y = (c.y + 0.5f) * (float)TILE_SIZE_MIN;
    spawn_splash(lvl, x, y, color);
}

void particle_emit_earth(Level *lvl, int cell) {
    Point c = idx_to_xy(cell);
    float x = (c.x + rng_float()) * (float)TILE_SIZE_MIN;
    float y = (c.y + 1.0f) * (float)TILE_SIZE_MIN;
    spawn_earth(lvl, x, y);
}

void particle_emit_elmo(Level *lvl, int cell) {
    Point c = idx_to_xy(cell);
    float x = (c.x + rng_float()) * (float)TILE_SIZE_MIN;
    float y = (c.y + rng_float()) * (float)TILE_SIZE_MIN;
    spawn_elmo(lvl, x, y);
}

void particle_emit_spark(Level *lvl, int cell) {
    Point c = idx_to_xy(cell);
    float x = (c.x + 0.5f) * (float)TILE_SIZE_MIN;
    float y = (c.y + 0.5f) * (float)TILE_SIZE_MIN;
    spawn_spark(lvl, x, y);
}

void particle_emit_smoke(Level *lvl, int cell) {
    Point c = idx_to_xy(cell);
    float x = (c.x + rng_float()) * (float)TILE_SIZE_MIN;
    float y = (c.y + rng_float()) * (float)TILE_SIZE_MIN;
    spawn_smoke(lvl, x, y);
}

void particle_emit_blast(Level *lvl, int cell) {
    Point c = idx_to_xy(cell);
    float x = (c.x + 0.5f) * (float)TILE_SIZE_MIN;
    float y = (c.y + 0.5f) * (float)TILE_SIZE_MIN;
    spawn_blast(lvl, x, y);
}

void particle_emit_poison(Level *lvl, int cell) {
    Point c = idx_to_xy(cell);
    float x = (c.x + 0.5f) * (float)TILE_SIZE_MIN;
    float y = (c.y + 0.5f) * (float)TILE_SIZE_MIN;
    spawn_poison(lvl, x, y);
}

void particle_emit_purple(Level *lvl, int cell) {
    Point c = idx_to_xy(cell);
    float x = (c.x + 0.5f) * (float)TILE_SIZE_MIN;
    float y = (c.y + 0.5f) * (float)TILE_SIZE_MIN;
    spawn_purple(lvl, x, y);
}

void particle_emit_wand_trail(Level *lvl, WandTrailKind trail, float x, float y, int index) {
    switch (trail) {
        case TRAIL_NONE:                                   break;
        case TRAIL_MAGIC:   spawn_trail_magic(lvl, x, y);          break;
        case TRAIL_FIRE:    spawn_flame(lvl, x, y);                break; /* FlameParticle.FACTORY, same as the fire blob */
        case TRAIL_EARTH:   spawn_trail_earth(lvl, x, y);          break;
        case TRAIL_WHITE:   spawn_trail_white(lvl, x, y);          break;
        case TRAIL_PURPLE:  spawn_trail_purple(lvl, x, y);         break;
        case TRAIL_WOOL:    spawn_trail_wool(lvl, x, y);           break;
        case TRAIL_POISON:  spawn_trail_poison(lvl, x, y);         break;
        case TRAIL_FOLIAGE: spawn_leaf(lvl, x, y);                 break; /* LeafParticle.GENERAL, same as Plant's leaf burst */
        case TRAIL_SLOW:    spawn_trail_slow(lvl, x, y);           break;
        case TRAIL_FORCE:   spawn_trail_force(lvl, index, x, y);   break;
        case TRAIL_COLD:    spawn_trail_cold(lvl, x, y);           break;
    }
}

void particle_clear(Level *lvl) {
    if (!lvl) return;
    lvl->num_particles = 0;
    lvl->num_emitters = 0;

    memset(lvl->particles, 0, sizeof(lvl->particles));
    memset(lvl->emitters, 0, sizeof(lvl->emitters));
    seed_blob_emit_timers();
}

void particle_emitter_start(Level *lvl, EmitterKind kind, int cell, float duration, float rate) {
    if (!lvl || !in_bounds(cell) || duration <= 0.0f || rate <= 0.0f)
        return;

    for (int i = 0; i < MAX_LEVEL_EMITTERS; i++) {
        ParticleEmitter *e = &lvl->emitters[i];

        if (e->kind != EMITTER_NONE)
            continue;

        e->kind = kind;
        e->cell = cell;
        e->timer = duration;
        e->emit_timer = 0.0f;
        e->rate = rate;

        if (lvl->num_emitters <= i)
            lvl->num_emitters = i + 1;

        return;
    }
}

void particle_update_emitters(Level *lvl, float dt, CellVisibleFn is_visible_fn, void *user) {
    if (!lvl)
        return;

    for (int i = 0; i < lvl->num_emitters; i++) {
        ParticleEmitter *e = &lvl->emitters[i];

        if (e->kind == EMITTER_NONE)
            continue;

        e->timer -= dt;

        if (e->timer <= 0.0f) {
            e->kind = EMITTER_NONE;
            continue;
        }

        if (is_visible_fn && !is_visible_fn(e->cell, user)) 
            continue;

        e->emit_timer -= dt;

        while (e->emit_timer <= 0.0f) {
            switch (e->kind) {
                case EMITTER_EARTH:
                    particle_emit_earth(lvl, e->cell);
                    break;
                case EMITTER_ELMO:
                    particle_emit_elmo(lvl, e->cell);
                    break;
                case EMITTER_SPARK:
                    particle_emit_spark(lvl, e->cell);
                    break;
                case EMITTER_SMOKE:
                    particle_emit_smoke(lvl, e->cell);
                    break;
                case EMITTER_BLAST:
                    particle_emit_blast(lvl, e->cell);
                    break;
                case EMITTER_POISON:
                    particle_emit_poison(lvl, e->cell);
                    break;
                case EMITTER_PURPLE:
                    particle_emit_purple(lvl, e->cell);
                    break;
                case EMITTER_SPLASH_POTION:
                    particle_emit_splash(lvl, e->cell, 0x79998a); //temporary for potion throwing
                    break;
                case EMITTER_SPLASH_BLOOD:
                    particle_emit_splash(lvl, e->cell, 0x8a0303);
                    break;
                case EMITTER_SPLASH_GRASS:
                    particle_emit_splash(lvl, e->cell, 0x67933d);
                    break;
                default:
                    break;
            }

            e->emit_timer += 1.0f / e->rate;
        }
    }
}

static float wind_emit_timer = 0.0f;

void particle_update_ambient(Level *lvl, float dt, CellVisibleFn is_visible_fn, void *user) {
    wind_emit_timer -= dt;

    bool emit_wind = wind_emit_timer <= 0.0f;
    if (emit_wind) {
        wind_emit_timer += 0.75f;
    }

    for (int cur = 0; cur < LEVEL_SIZE; cur++) {
        if (is_visible_fn && !is_visible_fn(cur, user))
            continue;

        int tile = lvl->map[cur];

        if (emit_wind && cell_pit(lvl, cur)) {
            particle_emit_wind(lvl, cur);
        }

        if (lvl->theme == THEME_SEWERS && tile == TILE_WALL_DECO) { //TODO add for the rest of themes
            particle_emit_water(lvl, cur);
        }

        if (tile == TILE_CHASM_WATER) {
            particle_emit_flow(lvl, cur);
        }
    }
}