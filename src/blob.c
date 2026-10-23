#include "blob.h"
#include "level.h"
#include "level_geometry.h"
#include <string.h>

/* ══════════════════════════════════════════════════════════
   Module state
   ══════════════════════════════════════════════════════════ */
static BlobPool *current_pool  = NULL;
static int       current_depth = 1;
static Char     *current_hero  = NULL;

void blob_set_pool(BlobPool *pool)   { current_pool  = pool; }
void blob_set_depth(int depth)       { current_depth = depth; }
void blob_set_hero(Char *hero)       { current_hero  = hero; }

#define PARALYSIS_GAS_DURATION 10.0f
#define VERTIGO_GAS_DURATION   10.0f

/* ══════════════════════════════════════════════════════════
   Actor
   ══════════════════════════════════════════════════════════ */
static int blob_actor_act(Actor *a) {
    Blob *b = (Blob *)a;

    actor_spend(a, ACTOR_TICK);

    if (b->volume > 0) {
        b->volume = 0;
        b->vt->evolve(b);

        int *tmp = b->off;
        b->off = b->cur;
        b->cur = tmp;
    }

    return 1;
}

static const ActorVTable BLOB_ACTOR_VTABLE = {
    .act       = blob_actor_act,
    .on_remove = NULL,
};

static void blob_default_evolve(Blob *b) {
    Level *lvl = char_active_level();
    if (!lvl) return;

    for (int y = 1; y < LEVEL_H - 1; y++) {
        int from = y * LEVEL_W + 1;
        int to   = from + LEVEL_W - 2;

        for (int pos = from; pos < to; pos++) {
            if (!cell_solid(lvl, pos)) {

                int count = 1;
                int sum   = b->cur[pos];

                if (!cell_solid(lvl, pos - 1))      { sum += b->cur[pos - 1];      count++; }
                if (!cell_solid(lvl, pos + 1))      { sum += b->cur[pos + 1];      count++; }
                if (!cell_solid(lvl, pos - LEVEL_W)) { sum += b->cur[pos - LEVEL_W]; count++; }
                if (!cell_solid(lvl, pos + LEVEL_W)) { sum += b->cur[pos + LEVEL_W]; count++; }

                int value = sum >= count ? (sum / count) - 1 : 0;
                b->off[pos] = value;
                b->volume  += value;

            } else {
                b->off[pos] = 0;
            }
        }
    }
}


static void single_cell_seed(Blob *b, int cell, int amount) {
    b->cur[b->pos] = 0;
    b->pos         = cell;
    b->volume      = b->cur[b->pos] = amount;
}

Blob *blob_get(BlobType type) {
    if (!current_pool) return NULL;
    Blob *b = &current_pool->slots[type];
    return b->active ? b : NULL;
}

Blob *blob_get_at(int cell, BlobType type) {
    Blob *b = blob_get(type);
    if (!b || b->volume <= 0) return NULL;

    return b->cur[cell] > 0 ? b : NULL;
}

void blob_clear_cell(Blob *b, int cell) {
    b->volume  -= b->cur[cell];
    b->cur[cell] = 0;
}

/* ══════════════════════════════════════════════════════════
   Fire
   ══════════════════════════════════════════════════════════ */
static void fire_burn_cell(int pos) {
    Char *ch = char_find_at(pos);
    if (ch) {
        Buff *burning = char_buff(ch, BUFF_BURNING);
        if (burning) {
            burning->duration = BURNING_DURATION;
        } else {
            char_apply_buff(ch, BUFF_BURNING, BURNING_DURATION);
        }
    }

    /* TODO: original also does Dungeon.level.heaps.get(pos).burn() here —
       burns a Scroll/MysteryMeat sitting in an item heap on this tile. */
}

static void fire_evolve(Blob *b) {
    Level *lvl = char_active_level();
    if (!lvl) return;

    int from = LEVEL_W + 1;
    int to   = LEVEL_SIZE - LEVEL_W - 1;

    for (int pos = from; pos < to; pos++) {
        int fire;

        if (b->cur[pos] > 0) {

            fire_burn_cell(pos);

            fire = b->cur[pos] - 1;
            if (fire <= 0 && cell_flamable(lvl, pos)) {
                lvl->map[pos] = TILE_EMBERS;
            }

        } else {

            if (cell_flamable(lvl, pos) &&
                (b->cur[pos - 1] > 0 || b->cur[pos + 1] > 0 ||
                 b->cur[pos - LEVEL_W] > 0 || b->cur[pos + LEVEL_W] > 0)) {
                fire = 4;
                fire_burn_cell(pos);
            } else {
                fire = 0;
            }
        }

        b->off[pos] = fire;
        b->volume  += fire;
    }
}

static void fire_seed(Blob *b, int cell, int amount) {
    /* re-igniting an already-burning cell is a no-op — no extra fuel added */
    if (b->cur[cell] == 0) {
        b->volume    += amount;
        b->cur[cell]  = amount;
    }
}

/* ══════════════════════════════════════════════════════════
   Toxic / Paralytic / Confusion gas
   ══════════════════════════════════════════════════════════ */
static void toxic_gas_evolve(Blob *b) {
    blob_default_evolve(b);

    int level_damage = 5 + current_depth * 5;

    for (int i = 0; i < LEVEL_SIZE; i++) {
        if (b->cur[i] > 0) {
            Char *ch = char_find_at(i);
            if (ch) {
                int damage = (ch->HT + level_damage) / 40;
                if (rng_int(40) < (ch->HT + level_damage) % 40) {
                    damage++;
                }
                char_damage(ch, damage, b, DMGSRC_TOXIC_GAS);
            }
        }
    }

    Blob *par = blob_get(BLOB_PARALYTIC_GAS);
    if (par) {
        for (int i = 0; i < LEVEL_SIZE; i++) {
            int t = b->cur[i];
            int p = par->cur[i];

            if (p >= t) {
                b->volume -= t;
                b->cur[i]  = 0;
            } else {
                par->volume -= p;
                par->cur[i]  = 0;
            }
        }
    }
}

static void paralytic_gas_evolve(Blob *b) {
    blob_default_evolve(b);

    for (int i = 0; i < LEVEL_SIZE; i++) {
        if (b->cur[i] > 0) {
            Char *ch = char_find_at(i);
            if (ch) {
                char_apply_buff(ch, BUFF_PARALYSIS, PARALYSIS_GAS_DURATION);
            }
        }
    }
}

static void confusion_gas_evolve(Blob *b) {
    blob_default_evolve(b);

    for (int i = 0; i < LEVEL_SIZE; i++) {
        if (b->cur[i] > 0) {
            Char *ch = char_find_at(i);
            if (ch) {
                char_apply_buff(ch, BUFF_VERTIGO, VERTIGO_GAS_DURATION);
            }
        }
    }
}

/* ══════════════════════════════════════════════════════════
   Web
   ══════════════════════════════════════════════════════════ */
static void web_evolve(Blob *b) {
    for (int i = 0; i < LEVEL_SIZE; i++) {
        int offv = b->cur[i] > 0 ? b->cur[i] - 1 : 0;
        b->off[i] = offv;

        if (offv > 0) {
            b->volume += offv;

            Char *ch = char_find_at(i);
            if (ch) {
                char_apply_buff(ch, BUFF_ROOTS, ACTOR_TICK);
            }
        }
    }
}

static void web_seed(Blob *b, int cell, int amount) {
    int diff = amount - b->cur[cell];
    if (diff > 0) {
        b->cur[cell]  = amount;
        b->volume    += diff;
    }
}

/* ══════════════════════════════════════════════════════════
   Foliage / Regrowth
   ══════════════════════════════════════════════════════════ */
static void foliage_evolve(Blob *b) {
    Level *lvl = char_active_level();
    if (!lvl) return;

    int from = LEVEL_W + 1;
    int to   = LEVEL_SIZE - LEVEL_W - 1;

    for (int pos = from; pos < to; pos++) {
        if (b->cur[pos] > 0) {
            b->off[pos] = b->cur[pos];
            b->volume  += b->off[pos];

            if (lvl->map[pos] == TILE_EMBERS) {
                /* TODO: GameScene.updateMap() / Dungeon.observe() */
                lvl->map[pos] = TILE_GRASS;
            }

        } else {
            b->off[pos] = 0;
        }
    }
}

void foliage_try_stealth(Char *hero) {
    if (!char_is_alive(hero)) return;

    Blob *foliage = blob_get(BLOB_FOLIAGE);
    if (foliage && foliage->cur[hero->pos] > 0) {
        char_apply_buff(hero, BUFF_SHADOWS, 2.0f);
    }
}

static void regrowth_evolve(Blob *b) {
    blob_default_evolve(b);

    if (b->volume <= 0) return;

    Level *lvl = char_active_level();
    if (!lvl) return;

    for (int i = 0; i < LEVEL_SIZE; i++) {
        if (b->off[i] > 0) {
            int c  = lvl->map[i];
            int c1 = c;

            if (c == TILE_EMPTY || c == TILE_EMBERS || c == TILE_EMPTY_DECO) {
                c1 = b->cur[i] > 9 ? TILE_HIGH_GRASS : TILE_GRASS;
            } else if (c == TILE_GRASS && b->cur[i] > 9) {
                c1 = TILE_HIGH_GRASS;
            }

            if (c1 != c) {
                /* TODO: GameScene.updateMap()/discoverTile()/Dungeon.observe() */
                lvl->map[i] = TILE_HIGH_GRASS;
            }

            Char *ch = char_find_at(i);
            if (ch) {
                char_apply_buff(ch, BUFF_ROOTS, ACTOR_TICK);
            }
        }
    }
}

/* ══════════════════════════════════════════════════════════
   Alchemy / Sacrificial Fire
   ══════════════════════════════════════════════════════════ */
static void alchemy_evolve(Blob *b) {
    b->volume = b->off[b->pos] = b->cur[b->pos];
    /* TODO: Journal.add(ALCHEMY) when Dungeon.visible[pos] */
}

/* TODO: Alchemy.transmute(cell) — needs Heap.transmute() */

static void sacrificial_fire_evolve(Blob *b) {
    b->volume = b->off[b->pos] = b->cur[b->pos];

    /* TODO: the original also applies a "Marked for sacrifice" buff (Buff.prolong(ch, Marked.class, 5f)) */
}

void sacrificial_fire_sacrifice(Char *ch) { //REDO
    /* TODO: Wound.hit(ch) */

    Blob *fire = blob_get(BLOB_SACRIFICIAL_FIRE);
    if (!fire) return;

    /* TODO: the exp computation needs Mob.exp()/Hero.maxExp()  */
    (void)ch;
    (void)fire;
}

/* ══════════════════════════════════════════════════════════
   WellWater family — Water of Awareness / Health / Transmutation
   ══════════════════════════════════════════════════════════ */
static void wellwater_evolve(Blob *b) {
    b->volume = b->off[b->pos] = b->cur[b->pos];
    /* TODO: Journal.add(WELL_OF_x) when Dungeon.visible[pos] — see Alchemy. */
}

static int water_of_health_affect_hero(Blob *b, Char *hero) {
    (void)b;

    /* PotionOfHealing.heal(): full heal + strip Poison/Cripple/Weakness/Bleeding */
    hero->HP = hero->HT;
    char_remove_buffs_of_type(hero, BUFF_POISON);
    char_remove_buffs_of_type(hero, BUFF_CRIPPLE);
    char_remove_buffs_of_type(hero, BUFF_WEAKNESS);
    char_remove_buffs_of_type(hero, BUFF_BLEEDING);

    char_hunger_satisfy(hero, HUNGER_STARVING);

    /* TODO: hero.belongings.uncurseEquipped()   */
    return 1;
}

static int water_of_awareness_affect_hero(Blob *b, Char *hero) {
    (void)b;

    Level *lvl = char_active_level();
    if (lvl) {
        for (int i = 0; i < LEVEL_SIZE; i++) {
            if (gTileFlags[lvl->map[i]] & TF_SECRET) {
                /* TODO: GameScene.updateMap(i)/discoverTile(i,terr) — render hooks */
                lvl->map[i] = tile_discover(lvl, i);
            }
        }
    }

    char_apply_buff(hero, BUFF_AWARENESS, AWARENESS_DURATION);

    /* TODO: hero.belongings.observe() — needs the item system. */
    return 1;
}

static int wellwater_affect(Blob *b) {
    if (current_hero && b->pos == current_hero->pos &&
        b->vt->affect_hero && b->vt->affect_hero(b, current_hero)) {
        b->volume = b->off[b->pos] = b->cur[b->pos] = 0;
        return 1;
    }

    /* TODO: original's other branch — Dungeon.level.heaps.get(pos): an item
       heap sitting on the well (affectItem(): identify / transmute / fill a
       DewVial) */
    return 0;
}

void wellwater_affect_cell(int cell) {
    static const BlobType WATERS[3] = {
        BLOB_WATER_OF_HEALTH, BLOB_WATER_OF_AWARENESS, BLOB_WATER_OF_TRANSMUTATION
    };

    for (int i = 0; i < 3; i++) {
        Blob *w = blob_get(WATERS[i]);
        if (w && w->volume > 0 && w->pos == cell && wellwater_affect(w)) {
            return;
        }
    }
}

/* ══════════════════════════════════════════════════════════
   Freezing
   ══════════════════════════════════════════════════════════ */
int freezing_affect(int cell) {
    Char *ch = char_find_at(cell);
    if (ch) {
        char_apply_buff(ch, BUFF_FROST, FROST_DURATION * (1.0f + rng_float() * 0.5f));
    }

    Blob *fire = blob_get_at(cell, BLOB_FIRE);
    if (fire) {
        blob_clear_cell(fire, cell);
    }

    /* TODO: Dungeon.level.heaps.get(cell).freeze()  */

    return 0;
}

/* ══════════════════════════════════════════════════════════
   tile_desc / vtable
   ══════════════════════════════════════════════════════════ */
const char *blob_tile_desc(BlobType type) {
    switch (type) {
        case BLOB_FIRE:                   return "A fire is raging here.";
        case BLOB_TOXIC_GAS:              return "A greenish cloud of toxic gas is swirling here.";
        case BLOB_PARALYTIC_GAS:          return "A cloud of paralytic gas is swirling here.";
        case BLOB_CONFUSION_GAS:          return "A cloud of confusion gas is swirling here.";
        case BLOB_WEB:                    return "Everything is covered with a thick web here.";
        case BLOB_FOLIAGE:                return "Shafts of light pierce the gloom of the underground garden.";
        case BLOB_SACRIFICIAL_FIRE:
            return "Sacrificial fire burns here. Every creature touched by this fire "
                   "is marked as an offering for the spirits of the dungeon.";
        case BLOB_WATER_OF_AWARENESS:
            return "Power of knowledge radiates from the water of this well. "
                   "Take a sip from it to reveal all secrets of equipped items.";
        case BLOB_WATER_OF_HEALTH:
            return "Power of health radiates from the water of this well. "
                   "Take a sip from it to heal your wounds and satisfy hunger.";
        case BLOB_WATER_OF_TRANSMUTATION:
            return "Power of change radiates from the water of this well. "
                   "Throw an item into the well to turn it into something else.";
        case BLOB_REGROWTH:
        case BLOB_ALCHEMY:
        default:
            return NULL;
    }
}

static const BlobVTable VT_FIRE                    = { fire_evolve,            fire_seed,         NULL };
static const BlobVTable VT_TOXIC_GAS               = { toxic_gas_evolve,       NULL,              NULL };
static const BlobVTable VT_PARALYTIC_GAS           = { paralytic_gas_evolve,   NULL,              NULL };
static const BlobVTable VT_CONFUSION_GAS           = { confusion_gas_evolve,   NULL,              NULL };
static const BlobVTable VT_WEB                     = { web_evolve,             web_seed,          NULL };
static const BlobVTable VT_FOLIAGE                 = { foliage_evolve,         NULL,              NULL };
static const BlobVTable VT_REGROWTH                = { regrowth_evolve,        NULL,              NULL };
static const BlobVTable VT_ALCHEMY                 = { alchemy_evolve,         single_cell_seed,  NULL };
static const BlobVTable VT_SACRIFICIAL_FIRE        = { sacrificial_fire_evolve,single_cell_seed,  NULL };
static const BlobVTable VT_WATER_OF_AWARENESS      = { wellwater_evolve,       single_cell_seed,  water_of_awareness_affect_hero };
static const BlobVTable VT_WATER_OF_HEALTH         = { wellwater_evolve,       single_cell_seed,  water_of_health_affect_hero };
static const BlobVTable VT_WATER_OF_TRANSMUTATION  = { wellwater_evolve,       single_cell_seed,  NULL };

static const BlobVTable *vtable_for(BlobType type) {
    switch (type) {
        case BLOB_FIRE:                   return &VT_FIRE;
        case BLOB_TOXIC_GAS:              return &VT_TOXIC_GAS;
        case BLOB_PARALYTIC_GAS:          return &VT_PARALYTIC_GAS;
        case BLOB_CONFUSION_GAS:          return &VT_CONFUSION_GAS;
        case BLOB_WEB:                    return &VT_WEB;
        case BLOB_FOLIAGE:                return &VT_FOLIAGE;
        case BLOB_REGROWTH:               return &VT_REGROWTH;
        case BLOB_ALCHEMY:                return &VT_ALCHEMY;
        case BLOB_SACRIFICIAL_FIRE:       return &VT_SACRIFICIAL_FIRE;
        case BLOB_WATER_OF_AWARENESS:     return &VT_WATER_OF_AWARENESS;
        case BLOB_WATER_OF_HEALTH:        return &VT_WATER_OF_HEALTH;
        case BLOB_WATER_OF_TRANSMUTATION: return &VT_WATER_OF_TRANSMUTATION;
        default:                          return &VT_FIRE; /* unreachable */
    }
}
/* ══════════════════════════════════════════════════════════
   Pool lifecycle
   ══════════════════════════════════════════════════════════ */
static void blob_init_slot(Blob *b, BlobType type) {
    memset(b->buf_a, 0, sizeof(b->buf_a));
    memset(b->buf_b, 0, sizeof(b->buf_b));
    b->cur    = b->buf_a;
    b->off    = b->buf_b;
    b->volume = 0;
    b->pos    = 0; 
    b->type   = type;
    b->vt     = vtable_for(type);
    b->active = 1;
    actor_init(&b->base, &BLOB_ACTOR_VTABLE);
}

void blob_clear(BlobPool *pool) {
    for (int i = 0; i < BLOB_TYPE_COUNT; i++) {
        blob_init_slot(&pool->slots[i], (BlobType)i);
        pool->slots[i].active = 0;
    }
}

void blob_restore_pool(BlobPool *pool) {
    for (int i = 0; i < BLOB_TYPE_COUNT; i++) {
        if (pool->slots[i].active) {
            pool->slots[i].base.time = 0.0f;
            actor_add(&pool->slots[i].base);
        }
    }
}

Blob *blob_seed(int cell, int amount, BlobType type) {
    if (!current_pool) return NULL;

    Blob *b = &current_pool->slots[type];
    if (!b->active) {
        blob_init_slot(b, type);
    }

    if (b->vt->do_seed) {
        b->vt->do_seed(b, cell, amount);
    } else {
        b->cur[cell] += amount;
        b->volume    += amount;
    }

    actor_add(&b->base);

    return b;
}