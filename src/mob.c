#include "mob.h"
#include "player.h"
#include "level.h"
#include "level_utils.h"
#include <stdlib.h>

/* TEST*/
const MobStats MOB_DB[MOB_KIND_COUNT] = {
    [MOB_RAT] = {
        .name = "rat", .ht = 5,
        .attack_skill = 8, .defense_skill = 5, .dr = 0,
        .dmg_min = 1, .dmg_max = 3,
        .view_distance = 6, .xp_reward = 1, .base_speed = 1.0f,
    },
    [MOB_GNOLL] = {
        .name = "gnoll", .ht = 12,
        .attack_skill = 14, .defense_skill = 8, .dr = 1,
        .dmg_min = 1, .dmg_max = 6,
        .view_distance = 8, .xp_reward = 3, .base_speed = 1.0f,
    },
};

/*═══════════════════════════════════════════════════════════════
    Storage
═══════════════════════════════════════════════════════════════*/
static MobPool *active_mob_pool = NULL;

void mob_set_pool(MobPool *pool) {
    active_mob_pool = pool;
}

/*═══════════════════════════════════════════════════════════════
    Combat
═══════════════════════════════════════════════════════════════*/
static int char_is_hero(Char *c) { /* guards the Player* cast in mob_die() below */
    return c && c->kind == CHAR_KIND_HERO;
}

static int mob_attack_skill(Char *self, Char *target) {
    (void)target;
    return MOB_DB[((Mob *)self)->kind].attack_skill;
}
static int mob_defense_skill(Char *self, Char *enemy) {
    (void)enemy;
    return MOB_DB[((Mob *)self)->kind].defense_skill;
}
static int mob_dr(Char *self) {
    return MOB_DB[((Mob *)self)->kind].dr;
}
static int mob_damage_roll(Char *self) {
    const MobStats *s = &MOB_DB[((Mob *)self)->kind];
    return rng_range(s->dmg_min, s->dmg_max);
}

static void mob_on_damaged(Char *self, int dmg_taken, void *src, DamageSource src_type) {
    (void)dmg_taken;
    Mob *m = (Mob *)self;
    if (src_type == DMGSRC_CHAR && src) {
        m->target = (Char *)src;
        m->state  = MOB_HUNTING;
    }
}

static void mob_die(Char *self, void *src, DamageSource src_type) {
    Mob *m = (Mob *)self;

    if (src_type == DMGSRC_CHAR && char_is_hero((Char *)src)) {
        player_earn_exp((Player *)src, MOB_DB[m->kind].xp_reward);
    }

    char_die_default(self, src, src_type);
    /* TODO ADD DROP HERE*/
}

/*═══════════════════════════════════════════════════════════════
   stupid AI, fully redo
═══════════════════════════════════════════════════════════════*/
static int mob_try_step(Mob *m, Level *lvl, int dx, int dy) {
    if (dx == 0 && dy == 0) return 0;

    Point p = idx_to_xy(m->base.pos);
    int nx = p.x + dx;
    int ny = p.y + dy;
    if (!in_bounds(nx, ny)) return 0;

    int npos = xy_to_idx(nx, ny);
    if (!(gTileFlags[lvl->map[npos]] & TF_PASSABLE)) return 0;
    if (char_find_at(npos) != NULL) return 0;

    char_move(&m->base, lvl, npos);
    return m->base.pos == npos;
}

/* one step toward `target` per call*/
static void mob_step_toward(Mob *m, Level *lvl, Char *target) {
    Point mp = idx_to_xy(m->base.pos);
    Point tp = idx_to_xy(target->pos);
    int dx = (tp.x > mp.x) - (tp.x < mp.x);
    int dy = (tp.y > mp.y) - (tp.y < mp.y);

    if (abs(tp.x - mp.x) >= abs(tp.y - mp.y)) {
        if (mob_try_step(m, lvl, dx, dy)) return;
        if (mob_try_step(m, lvl, dx, 0)) return;
        if (mob_try_step(m, lvl, 0, dy)) return;
    } else {
        if (mob_try_step(m, lvl, dx, dy)) return;
        if (mob_try_step(m, lvl, 0, dy)) return;
        if (mob_try_step(m, lvl, dx, 0)) return;
    }
}

static void mob_wander(Mob *m, Level *lvl) {
    if (rng_int(4) != 0) return; /* mostly stands still — tune to taste */

    static const int dx8[8] = { -1, 0, 1, -1, 1, -1, 0, 1 };
    static const int dy8[8] = { -1, -1, -1, 0, 0, 1, 1, 1 };
    int i = rng_int(8);
    mob_try_step(m, lvl, dx8[i], dy8[i]);
}

static int mob_on_turn(Char *self) {
    Mob *m = (Mob *)self;
    Level *lvl = char_active_level();

    if (!lvl) {
        char_spend(self, ACTOR_TICK);
        actor_next(&self->base);
        return 0;
    }

    if (m->target && !char_is_alive(m->target)) {
        m->target = NULL;
    }

    /* State transitions */
    if (m->state == MOB_ASLEEP || m->state == MOB_WANDERING) {
        if (m->target && char_distance(self, m->target) <= self->view_distance) {
            m->state = MOB_HUNTING;
        }
    } else if (m->state == MOB_HUNTING) {
        if (!m->target || char_distance(self, m->target) > self->view_distance * 2) {
            /* lost track — some slack (2x view distance) so it
               doesn't give up the instant the target is one step
               outside view range */
            m->state = MOB_WANDERING;
        }
    }

    switch (m->state) {
        case MOB_ASLEEP:
            break; /* sits until woken — by proximity above, or by being hit (mob_on_damaged) */

        case MOB_HUNTING:
            if (char_distance(self, m->target) <= 1) {
                char_attack(self, m->target);
                /* CharAttackResult discarded — no combat log yet,
                   same TODO-style gap as player.c's own attack
                   resolution. */
            } else {
                mob_step_toward(m, lvl, m->target);
            }
            break;

        case MOB_WANDERING:
        default:
            mob_wander(m, lvl);
            break;
    }

    char_spend(self, ACTOR_TICK / MOB_DB[m->kind].base_speed);
    actor_next(&self->base);
    return 0;
}

static const CharVTable *mob_vtable(void) {
    static CharVTable vt;
    static int built = 0;
    if (!built) {
        vt = CHAR_DEFAULT_VTABLE;
        vt.attack_skill  = mob_attack_skill;
        vt.defense_skill = mob_defense_skill;
        vt.dr            = mob_dr;
        vt.damage_roll   = mob_damage_roll;
        vt.on_damaged    = mob_on_damaged;
        vt.die           = mob_die;
        vt.on_turn       = mob_on_turn;
        built = 1;
    }
    return &vt;
}

/*═══════════════════════════════════════════════════════════════
    Spawning / pool management
═══════════════════════════════════════════════════════════════*/
Mob *mob_spawn(MobKind kind, int pos, Char *target) {
    if (!active_mob_pool)
        return NULL;

    if (kind < 0 || kind >= MOB_KIND_COUNT) return NULL;
    if (active_mob_pool->count >= MAX_LEVEL_MOBS) return NULL;
    if (char_find_at(pos) != NULL) return NULL;

    Mob *m = &active_mob_pool->mobs[active_mob_pool->count];
    const MobStats *stats = &MOB_DB[kind];

    char_init(&m->base, mob_vtable(), CHAR_KIND_MOB, pos, stats->ht);
    m->base.name          = stats->name;
    m->base.base_speed    = stats->base_speed;
    m->base.view_distance = stats->view_distance;

    m->kind   = kind;
    m->state  = MOB_ASLEEP;
    m->target = target;

    if (!char_add(&m->base)) return NULL; 

    active_mob_pool->count++;
    return m;
}

int mob_spawn_random(Level *lvl, MobKind kind, int count, Char *target) {
    int placed = 0;
    int attempts = 0;
    int max_attempts = count * 20; 

    while (placed < count && attempts < max_attempts) {
        attempts++;

        int x = rng_int(LEVEL_W);
        int y = rng_int(LEVEL_H);
        int pos = xy_to_idx(x, y);

        if (pos == lvl->entrance || pos == lvl->exit) continue;
        if (!(gTileFlags[lvl->map[pos]] & TF_PASSABLE)) continue;
        if (char_find_at(pos) != NULL) continue;

        if (mob_spawn(kind, pos, target)) placed++;
    }

    return placed;
}


int mob_spawn_random_no_target(Level *lvl, MobKind kind, int count) {
    return mob_spawn_random(lvl, kind, count, NULL);
}




void mob_unload_all(void) { //temporar solution of mobs saving after level transition
    if (!active_mob_pool)
        return;

    for (int i = 0; i < active_mob_pool->count; i++) {

        Mob *m = &active_mob_pool->mobs[i];

        if (char_is_alive(&m->base))
            char_unregister(&m->base);
    }
}

void mob_clear_all(void) { //TODO do i really need this one?
    for (int i = 0; i < active_mob_pool->count; i++) {
        char_destroy(&active_mob_pool->mobs[i].base); 
    }
    active_mob_pool->count = 0;
}

int mob_count(void) { //stuff for render
    return active_mob_pool->count;
}

Mob *mob_at_index(int i) {
    if (i < 0 || i >= active_mob_pool->count) return NULL;
    return &active_mob_pool->mobs[i];
}


void mob_restore_grid(void) {
    for (int i = 0; i < active_mob_pool->count; i++) {

        Mob *m = &active_mob_pool->mobs[i];

        if (!char_is_alive(&m->base))
            continue;

        m->base.base.time = 0.0f;

        char_add(&m->base);
    }
}

void mob_restore_targets(Char *player) {
    for(int i=0;i<active_mob_pool->count;i++) {
        active_mob_pool->mobs[i].target = player;
    }
}