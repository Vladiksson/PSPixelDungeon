#include "particle.h"
#include "level.h"
#include "rng.h"

/* ── color helpers ─────────────────────────────────────────── */
static void color_set(Particle *p, uint32_t hex) {
    p->rm = ((hex >> 16) & 0xFF) / 255.0f;
    p->gm = ((hex >> 8)  & 0xFF) / 255.0f;
    p->bm = ( hex         & 0xFF) / 255.0f;
}

static void color_lerp_random(Particle *p, uint32_t c1, uint32_t c2) {
    float t = rng_float();
    int r1 = (c1 >> 16) & 0xFF, g1 = (c1 >> 8) & 0xFF, b1 = c1 & 0xFF;
    int r2 = (c2 >> 16) & 0xFF, g2 = (c2 >> 8) & 0xFF, b2 = c2 & 0xFF;
    p->rm = (r1 + (r2 - r1) * t) / 255.0f;
    p->gm = (g1 + (g2 - g1) * t) / 255.0f;
    p->bm = (b1 + (b2 - b1) * t) / 255.0f;
}
/* ────────────────────────────────────────────────────────────────────── */

static Particle *spawn_slot(Level *lvl) {
    if (!lvl) return NULL;
    if (lvl->num_particles >= MAX_LEVEL_PARTICLES) return NULL;

    Particle *p = &lvl->particles[lvl->num_particles++];
    memset(p, 0, sizeof(*p));
    p->scale_x = p->scale_y = 1.0f;
    p->size    = 1.0f;
    p->rm = p->gm = p->bm = p->am = 1.0f;
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
        p->am = 0.0f;
        break;

    case SPECK_QUESTION:
        p->lifespan = 0.8f;
        break;

    case SPECK_UP:
        p->vy = -20.0f;
        p->lifespan = 1.0f;
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

static Particle *spawn_for_kind(Level *lvl, ParticleKind kind, int speck_type, float x, float y) {
    switch (kind) {
        case PK_SPECK:       return spawn_speck(lvl, 0, x, y, speck_type);
        case PK_FLAME:       return spawn_flame(lvl, x, y);
        case PK_WEB:         return spawn_web_one(lvl, x, y);
        case PK_SHAFT:       return spawn_shaft(lvl, x, y);
        case PK_LEAF:        return spawn_leaf(lvl, x, y);
        case PK_SACRIFICIAL: return spawn_sacrificial(lvl, x, y);
        case PK_SNOW:        break;
    }
    return NULL;
}

static void emit_blob_tick(Level *lvl, const Blob *b, const BlobFx *fx, CellVisibleFn is_visible_fn, void *user) {
    for (int i = 0; i < LEVEL_SIZE; i++) {
        if (b->cur[i] <= 0) continue;
        if (is_visible_fn && !is_visible_fn(i, user)) continue;

        Point cell = idx_to_xy(i);
        float x = (cell.x + rng_float()) * (float)TILE_SIZE_MIN;
        float y = (cell.y + rng_float()) * (float)TILE_SIZE_MIN;

        for (int n = 0; n < fx->spawn_count; n++) {
            Particle *p = spawn_for_kind(lvl, fx->kind, fx->speck_type, x, y);
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

void particle_emit_snow(Level *lvl, int cell) {
    Point c = idx_to_xy(cell);
    float x = (c.x + rng_float()) * (float)TILE_SIZE_MIN;
    float y = (c.y + 1.0f) * (float)TILE_SIZE_MIN;
    spawn_snow(lvl, x, y);
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
        }

        i++;
    }
}

void speck_sprite(int type, uint8_t *x, uint8_t *y, uint8_t *w, uint8_t *h) {
    int frame;
    switch (type) {
    case SPECK_DISCOVER:
        frame = SPECK_LIGHT;
        break;
    case SPECK_EVOKE:
    case SPECK_MASTERY:
    case SPECK_KIT:
    case SPECK_FORGE:
        frame = SPECK_STAR;
        break;
    case SPECK_RATTLE:
        frame = SPECK_BONE;
        break;
    case SPECK_JET:
    case SPECK_TOXIC:
    case SPECK_PARALYSIS:
    case SPECK_CONFUSION:
    case SPECK_DUST:
        frame = SPECK_STEAM;
        break;
    default:
        frame = type;
        break;
    }
    *x = (uint8_t)(frame * 7);
    *y = 0;
    *w = 7;
    *h = 7;
}

void particle_clear(Level *lvl) {
    if (!lvl) return;
    lvl->num_particles = 0;
    seed_blob_emit_timers();
}