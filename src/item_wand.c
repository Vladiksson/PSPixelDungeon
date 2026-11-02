#include "item_wand.h"
#include "buff.h"
#include "blob.h"
#include "level_geometry.h"
#include "particle.h"

/* ══════════════════════════════════════════════════════════
   Zap effects  (Lightning / Disintegration)
   ══════════════════════════════════════════════════════════ */
#define LIGHTNING_FX_DURATION 0.3f
#define RAY_FX_DURATION       0.5f

static LightningFxView gLightning;
static float gLightningLife = 0.0f;

static RayFxView gRay;
static float gRayLife = 0.0f;

static void lightning_fx_start(const int *cells, int n) {
    if (n < 1) return;
    if (n > LIGHTNING_FX_MAX_POINTS) n = LIGHTNING_FX_MAX_POINTS;

    gLightning.active = 1;
    gLightning.count = n;
    gLightningLife = LIGHTNING_FX_DURATION;

    for (int i = 0; i < n; i++) {
        Point c = idx_to_xy(cells[i]);
        gLightning.pts_x[i] = (c.x + 0.5f) * (float)TILE_SIZE_MIN;
        gLightning.pts_y[i] = (c.y + 0.5f) * (float)TILE_SIZE_MIN;
    }

    for (int i = 0; i < n - 1; i++) {
        float sx = gLightning.pts_x[i],   sy = gLightning.pts_y[i];
        float ex = gLightning.pts_x[i + 1], ey = gLightning.pts_y[i + 1];
        gLightning.mid_x[i] = (sx + ex) * 0.5f + (-4.0f + rng_float() * 8.0f);
        gLightning.mid_y[i] = (sy + ey) * 0.5f + (-4.0f + rng_float() * 8.0f);
    }
}

static void ray_fx_start(int from_cell, int to_cell) {
    Point f = idx_to_xy(from_cell);
    Point t = idx_to_xy(to_cell);

    gRay.active = 1;
    gRayLife = RAY_FX_DURATION;
    gRay.sx = (f.x + 0.5f) * (float)TILE_SIZE_MIN;
    gRay.sy = (f.y + 0.5f) * (float)TILE_SIZE_MIN;
    gRay.ex = (t.x + 0.5f) * (float)TILE_SIZE_MIN;
    gRay.ey = (t.y + 0.5f) * (float)TILE_SIZE_MIN;
}

void wand_fx_update(float dt) {
    if (gLightning.active) {
        gLightningLife -= dt;
        if (gLightningLife <= 0.0f) {
            gLightning.active = 0;
        } else {
            for (int i = 0; i < gLightning.count - 1; i++) {
                float sx = gLightning.pts_x[i],   sy = gLightning.pts_y[i];
                float ex = gLightning.pts_x[i + 1], ey = gLightning.pts_y[i + 1];
                gLightning.mid_x[i] = (sx + ex) * 0.5f + (-4.0f + rng_float() * 8.0f);
                gLightning.mid_y[i] = (sy + ey) * 0.5f + (-4.0f + rng_float() * 8.0f);
            }
        }
    }

    if (gRay.active) {
        gRayLife -= dt;
        if (gRayLife <= 0.0f) {
            gRay.active = 0;
        }
    }
}

int lightning_fx_snapshot(LightningFxView *out) {
    if (!gLightning.active) return 0;
    gLightning.alpha = gLightningLife / LIGHTNING_FX_DURATION;
    *out = gLightning;
    return 1;
}

int ray_fx_snapshot(RayFxView *out) {
    if (!gRay.active) return 0;
    gRay.alpha = gRayLife / RAY_FX_DURATION;
    *out = gRay;
    return 1;
}

/* ══════════════════════════════════════════════════════════
   Bolt visuals
   ══════════════════════════════════════════════════════════ */
static int     gBoltFlying = 0;
static WandTrailKind gBoltTrail = TRAIL_NONE;
static float gTrailTimer = 0.0f;
static int   gTrailIndex = 0;

static WandTrailKind wand_trail_for_type(ItemType type) {
    switch (type) {
        case ITEM_WAND_MAGIC_MISSILE:  return TRAIL_MAGIC;    /* MagicMissile.blueLight */
        case ITEM_WAND_FIREBOLT:       return TRAIL_FIRE;     /* MagicMissile.fire */
        case ITEM_WAND_LIGHTNING:      return TRAIL_NONE;     /* WandOfLightning.fx(): Lightning(points[]), not MagicMissile */
        case ITEM_WAND_POISON:         return TRAIL_POISON;   /* MagicMissile.poison */
        case ITEM_WAND_TELEPORTATION:  return TRAIL_COLD;     /* MagicMissile.coldLight */
        case ITEM_WAND_SLOWNESS:       return TRAIL_SLOW;     /* MagicMissile.slowness */
        case ITEM_WAND_REGROWTH:       return TRAIL_FOLIAGE;  /* MagicMissile.foliage */
        case ITEM_WAND_BLINK:          return TRAIL_WHITE;    /* MagicMissile.whiteLight */
        case ITEM_WAND_AMOK:           return TRAIL_PURPLE;   /* MagicMissile.purpleLight */
        case ITEM_WAND_REACH:          return TRAIL_FORCE;    /* MagicMissile.force */
        case ITEM_WAND_FLOCK:          return TRAIL_WOOL;     /* MagicMissile.wool */
        case ITEM_WAND_DISINTEGRATION: return TRAIL_NONE;     /* WandOfDisintegration.fx(): DeathRay, not MagicMissile */
        case ITEM_WAND_AVALANCHE:      return TRAIL_EARTH;    /* MagicMissile.earth */
        default:                       return TRAIL_NONE;
    }
}

void wand_bolt_set_flying(ItemType type) {
    gBoltFlying = 1;
    gBoltTrail  = wand_trail_for_type(type);
    gTrailTimer = 0.0f;
    gTrailIndex = 0;
}

void wand_bolt_clear(void) {
    gBoltFlying = 0;
    gBoltTrail  = TRAIL_NONE;
}

int wand_bolt(void) {
    return gBoltFlying;
}

void wand_bolt_update_trail(Level *lvl, float dt) {
    if (!lvl || !gBoltFlying || gBoltTrail == TRAIL_NONE) return;

    MissileView mv;
    if (missiles_snapshot(&mv) <= 0) return;

    float px = (mv.x + 0.5f) * (float)TILE_SIZE_MIN;
    float py = (mv.y + 0.5f) * (float)TILE_SIZE_MIN;

    gTrailTimer -= dt;
    while (gTrailTimer <= 0.0f) {
        particle_emit_wand_trail(lvl, gBoltTrail, px, py, gTrailIndex++);
        gTrailTimer += 0.01f;
    }

    if (gBoltTrail == TRAIL_SLOW) {
        for (int i = 0; i < lvl->num_particles; i++) {
            Particle *p = &lvl->particles[i];
            if (p->kind != PK_TRAIL_SLOW) continue;
            p->ax = (px - p->x) * 10.0f;
            p->ay = (py - p->y) * 10.0f;
        }
    }
}

/* ══════════════════════════════════════════════════════════
   Charges / identify
   ══════════════════════════════════════════════════════════ */

int wand_hits_chars(ItemType type) {
    switch (type) {
        case ITEM_WAND_REACH:
        case ITEM_WAND_DISINTEGRATION:
        case ITEM_WAND_AVALANCHE:
            return 0;
        default:
            return 1;
    }
}

static int wand_power(const ItemStack *stack) {
    /* Ring of Power's bonus */
    return item_effective_level(stack);
}

void wand_consume_charge(ItemStack *stack) {
    if (stack->charges > 0) stack->charges--;
}




/* ---- Magic Missile ---- */
static void wand_magic_missile_zap(const ItemStack *stack, Char *caster, int cell) {
    Char *ch = char_find_at(cell);
    if (!ch) return;

    int level = wand_power(stack);
    char_damage(ch, rng_range(1, 5 + level * 2), caster, DMGSRC_GENERIC);
}

/* ---- Firebolt ---- */
static void wand_firebolt_zap(const ItemStack *stack, Char *caster, Level *lvl, const Ballistica *path, int cell) {
    int level = wand_power(stack);

    for (int i = 1; i < path->distance - 1; i++) {
        int c = path->trace[i];
        if (cell_flamable(lvl, c)) {
            blob_seed(c, 1, BLOB_FIRE);
        }
    }
    blob_seed(cell, 1, BLOB_FIRE);

    Char *ch = char_find_at(cell);
    if (ch) {
        char_damage(ch, rng_range(1, 7 + level * level), caster, DMGSRC_FIRE);
        char_apply_buff(ch, BUFF_BURNING, BURNING_DURATION);
    }
}

/* ---- Lightning ---- */
#define WAND_LIGHTNING_MAX_CHAIN 20

static void wand_lightning_hit(Char *ch, Level *lvl, int damage, Char *caster, Char **affected, int *n_affected, int *points, int *n_points) {
    if (damage < 1) return;
    if (*n_affected >= WAND_LIGHTNING_MAX_CHAIN) return;

    affected[(*n_affected)++] = ch;
    if (*n_points < LIGHTNING_FX_MAX_POINTS) points[(*n_points)++] = ch->pos; /* points[nPoints++] = ch.pos */

    if (cell_liquid(lvl, ch->pos))
        damage *= 2;
    
    char_damage(ch, damage, caster, DMGSRC_LIGHTNING);

    Char *candidates[8];
    int n_candidates = 0;
    for (int i = 0; i < 8; i++) {
        int npos = neighbour_idx(ch->pos, DX[i], DY[i]);
        if (npos == -1) continue;
        Char *n = char_find_at(npos);
        if (!n) continue;

        int already = 0;
        for (int j = 0; j < *n_affected; j++) {
            if (affected[j] == n) { already = 1; break; }
        }
        if (already) continue;

        candidates[n_candidates++] = n;
    }

    if (n_candidates > 0) {
        Char *next = candidates[rng_int(n_candidates)];
        int next_dmg = rng_range(damage / 2, damage - 1);
        wand_lightning_hit(next, lvl, next_dmg, caster, affected, n_affected, points, n_points);
    }
}

static void wand_lightning_zap(const ItemStack *stack, Char *caster, Level *level, int cell) {
    int points[LIGHTNING_FX_MAX_POINTS];
    int n_points = 0;
    points[n_points++] = caster->pos; /* points[nPoints++] = Dungeon.hero.pos */

    Char *ch = char_find_at(cell);
    if (ch) {
        int lvl = wand_power(stack);
        int dmg = rng_range(5 + lvl / 2, 9 + lvl);

        Char *affected[WAND_LIGHTNING_MAX_CHAIN];
        int n_affected = 0;
        wand_lightning_hit(ch, level, dmg, caster, affected, &n_affected, points, &n_points);
    } else {
        if (n_points < LIGHTNING_FX_MAX_POINTS) points[n_points++] = cell;
        for (int i = 0; i < 3; i++) particle_emit_spark(level, cell); /* CellEmitter.center(cell).burst(SparkParticle.FACTORY, 3) */
    }

    lightning_fx_start(points, n_points);
}

/* ---- Poison ---- */
static void wand_poison_zap(const ItemStack *stack, int cell) {
    Char *ch = char_find_at(cell);
    if (!ch) return;

    float duration = 5.0f + (float)wand_power(stack); /* Poison.durationFactor() [Ring of Elements] */
    char_poison_set(ch, duration);
}

/* ---- Teleportation ---- */
static void wand_teleport_zap(Level *lvl, int cell) {
    Char *ch = char_find_at(cell);
    if (!ch) return;
    /* curUser-targets-self branch (ScrollOfTeleportation.teleportHero) is
       skipped: the zapper's own self-target block already refuses a
       cell == caster->pos selection upstream, so `ch == caster` here is
       unreachable in normal play. */

    int pos = -1;
    for (int tries = 0; tries < 10; tries++) {
        int c = xy_to_idx(rng_range(0, LEVEL_W - 1), rng_range(0, LEVEL_H - 1));
        if (cell_passable(lvl, c) && !char_find_at(c)) { pos = c; break; }
    }
    if (pos == -1) return; /* TODO: "no safe cell found" message */

    char_move(ch, lvl, pos);
}

/* ---- Slowness ---- */
static void wand_slowness_zap(const ItemStack *stack, int cell) {
    Char *ch = char_find_at(cell);
    if (!ch) return;

    float duration = 10.0f / 3.0f + (float)wand_power(stack); /* Slow.duration() [Ring of Elements] skipped, == 1 */
    char_apply_buff(ch, BUFF_SLOW, duration);
}

/* ---- Regrowth ---- */
static void wand_regrowth_zap(const ItemStack *stack, Level *lvl, const Ballistica *path, int cell) {
    for (int i = 1; i < path->distance - 1; i++) {
        int p = path->trace[i];
        uint8_t t = lvl->map[p];
        if (t == TILE_EMPTY || t == TILE_EMBERS || t == TILE_EMPTY_DECO) {
            lvl->map[p] = TILE_GRASS;
        }
    }

    uint8_t t = lvl->map[cell];
    if (t == TILE_EMPTY || t == TILE_EMBERS || t == TILE_EMPTY_DECO || t == TILE_GRASS || t == TILE_HIGH_GRASS) {
        blob_seed(cell, (wand_power(stack) + 2) * 20, BLOB_REGROWTH);
    }
}

/* ---- Blink ---- */
static void wand_blink_zap(const ItemStack *stack, Char *caster, Level *lvl, const Ballistica *path, int cell) {
    int level = wand_power(stack);

    if (path->distance > level + 4) {
        cell = path->trace[level + 3];
    } else if (char_find_at(cell) != NULL && path->distance > 1) {
        cell = path->trace[path->distance - 2];
    }

    if (cell_pit(lvl, cell))
        return;

    char_move(caster, lvl, cell);
}

/* ---- Amok ---- */
static void wand_amok_zap(const ItemStack *stack, int cell) {
    Char *ch = char_find_at(cell);
    if (!ch) return;
    /* ch==hero branch (applies Vertigo instead of Amok) is skipped: it
       needs Vertigo.duration(), which isn't among the files given here,
       and is only reachable the same way Teleportation's self-branch is. */

    float duration = 3.0f + (float)wand_power(stack);
    char_apply_buff(ch, BUFF_AMOK, duration);
}

/* ---- Reach ---- */
static void wand_reach_zap(const ItemStack *stack, Char *caster, Level *lvl, const Ballistica *path) {
    int reach = wand_power(stack) + 4;
    if (path->distance < reach) reach = path->distance;

    for (int i = 1; i < reach; i++) {
        int c = path->trace[i];

        Char *ch = char_find_at(c);
        if (ch) {
            /* Swap(curUser,ch): the original schedules this as a
               delayed Actor; this port has no such actor, so it just
               swaps the two positions directly -- same end result. */
            int tmp = caster->pos;
            caster->pos = ch->pos;
            ch->pos = tmp;
            break;
        }

        /* TODO: the original also reaches for a Heap here (pulling a
           loose item into the hero's bag, or opening a chest), presses
           traps, and closes any open door it passes through. None of
           tile-press / remote-pickup / chest-open primitives are
           available in the files given for this port, so Reach only
           ever swaps with the first character in its path for now. */
        /*if (level_heap_at(lvl, c)) {
            break; 
        }*/
    }
}

/* ---- Flock ---- */
static void wand_flock_zap(const ItemStack *stack, int cell) {
    (void)stack; (void)cell;
    /* Wand.Flock spawns power()+2 temporary, invulnerable "Sheep" NPCs
       in a breadth-first ring around the impact cell, each despawning
       after power()+3 time units. This port has no NPC/Mob spawning API
       among the files given here (npc.h/mob.h weren't provided beyond
       npc_interact()) -- nothing to create the sheep with, so this is a
       deliberate no-op until a mob-spawn primitive exists. */
}

/* ---- Disintegration ---- */
static void wand_disintegration_zap(const ItemStack *stack, Char *caster, Level *lvl, const Ballistica *path) {
    int level = wand_power(stack);
    int max_distance = stack->level + 4; /* Wand.distance(): raw level(), deliberately NOT effectiveLevel-masked */
    int limit = (path->distance < max_distance) ? path->distance : max_distance;

    Char *chars[BALLISTICA_MAX_TRACE];
    int n_chars = 0;

    for (int i = 1; i < limit; i++) {
        int c = path->trace[i];

        Char *ch = char_find_at(c);
        if (ch && n_chars < BALLISTICA_MAX_TRACE) chars[n_chars++] = ch;

        if (lvl->map[c] == TILE_HIGH_GRASS) {
            lvl->map[c] = TILE_GRASS;
        }
        /* TODO: the original also destroys doors/signs it passes through
           (Level.destroy()) -- no such primitive is available here yet. */
    }

    int lvl_total = level + n_chars;
    int dmg_min = lvl_total;
    int dmg_max = 8 + lvl_total * lvl_total / 3;

    for (int i = 0; i < n_chars; i++) {
        char_damage(chars[i], rng_normal_range(dmg_min, dmg_max), caster, DMGSRC_DISINTEGRATION);
    }

    int end_cell = (limit >= 1) ? path->trace[limit - 1] : caster->pos;
    ray_fx_start(caster->pos, end_cell);
}

/* ---- Avalanche ---- */
#define WAND_AVALANCHE_UNREACHED 0x7fffffff

static void wand_avalanche_zap(const ItemStack *stack, Char *caster, Level *lvl, int cell) {
    int level = wand_power(stack);
    int size = 1 + level / 3;

    static int dist[LEVEL_SIZE];
    static int queue[LEVEL_SIZE];
    int qhead = 0, qtail = 0;

    for (int i = 0; i < LEVEL_SIZE; i++) dist[i] = WAND_AVALANCHE_UNREACHED;
    dist[cell] = 0;
    queue[qtail++] = cell;

    while (qhead < qtail) {
        int c = queue[qhead++];
        int d = dist[c];
        if (d >= size) continue;

        for (int i = 0; i < 8; i++) {
            int n = neighbour_idx(c, DX[i], DY[i]);
            if (n == -1) continue;
            if (cell_solid(lvl, n)) continue;
            if (dist[n] != WAND_AVALANCHE_UNREACHED) continue;
            dist[n] = d + 1;
            queue[qtail++] = n;
        }
    }

    for (int i = 0; i < LEVEL_SIZE; i++) {
        int d = dist[i];
        if (d == WAND_AVALANCHE_UNREACHED) continue;

        Char *ch = char_find_at(i);
        if (ch) {
            int dmg_hi = 5 + (size - d) * 2;
            int dmg = (dmg_hi >= 2) ? rng_range(2, dmg_hi) : 2;
            char_damage(ch, dmg, caster, DMGSRC_GENERIC);

            if (char_is_alive(ch) && rng_int(2 + d) == 0) {
                char_apply_buff(ch, BUFF_PARALYSIS, (float)rng_range(2, 6));
            }
        }
        /* TODO: the original also presses traps / mob-triggers at every
           affected cell here (Level.press()/mobPress()) -- no such
           primitive is available in the files given for this port yet. */
    }
}

/* ══════════════════════════════════════════════════════════
   Dispatch
   ══════════════════════════════════════════════════════════ */
void wand_zap(const ItemStack *stack, Char *caster, Level *lvl, const Ballistica *path, int cell) {
    switch (stack->type) {
        case ITEM_WAND_MAGIC_MISSILE:  wand_magic_missile_zap(stack, caster, cell); break;
        case ITEM_WAND_FIREBOLT:       wand_firebolt_zap(stack, caster, lvl, path, cell); break;
        case ITEM_WAND_LIGHTNING:      wand_lightning_zap(stack, caster, lvl, cell); break;
        case ITEM_WAND_POISON:         wand_poison_zap(stack, cell); break;
        case ITEM_WAND_TELEPORTATION:  wand_teleport_zap(lvl, cell); break;
        case ITEM_WAND_SLOWNESS:       wand_slowness_zap(stack, cell); break;
        case ITEM_WAND_REGROWTH:       wand_regrowth_zap(stack, lvl, path, cell); break;
        case ITEM_WAND_BLINK:          wand_blink_zap(stack, caster, lvl, path, cell); break;
        case ITEM_WAND_AMOK:           wand_amok_zap(stack, cell); break;
        case ITEM_WAND_REACH:          wand_reach_zap(stack, caster, lvl, path); break;
        case ITEM_WAND_FLOCK:          wand_flock_zap(stack, cell); break;
        case ITEM_WAND_DISINTEGRATION: wand_disintegration_zap(stack, caster, lvl, path); break;
        case ITEM_WAND_AVALANCHE:      wand_avalanche_zap(stack, caster, lvl, cell); break;
        default: break;
    }
}