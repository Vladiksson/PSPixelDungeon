#include "mob.h"
#include "player.h"
#include "rng.h"
#include <stdlib.h>

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

    mob_play_anim(m, MOB_ANIM_DIE);

    if (src_type == DMGSRC_CHAR && char_is_hero((Char *)src)) {
        player_earn_exp((Player *)src, MOB_DB[m->kind].xp_reward);
    }

    char_die_default(self, src, src_type);
    /* TODO ADD DROP HERE*/
}

/*═══════════════════════════════════════════════════════════════
    Animation
═══════════════════════════════════════════════════════════════*/
static void mob_resolve_attack(Mob *m) {
    Char *target = m->target;
    if (!target)
        return;
    if (!char_is_alive(target)) {
        m->target = NULL;
        return;
    }
    char_attack(&m->base, target);
}

void mob_play_anim(Mob *m, MobAnimState anim) {
    if (m->anim == anim)
        return;

    m->anim = anim;
    m->frame = 0;
    m->frame_clock = 0.0f;
    m->anim_done = 0;
}

void mob_animate_all(float dt) {
    if (!active_mob_pool)
        return;
    for (int i = 0; i < active_mob_pool->count; i++) {
        Mob *m = &active_mob_pool->mobs[i];

        if (!char_is_alive(&m->base) && m->anim_done)
            continue;

        const AnimClip *clip = &MOB_SPRITES[m->kind].clips[m->anim];

        if (m->anim_done || clip->fps <= 0 || clip->count <= 0)
            continue;

        m->frame_clock += dt;
        float step = 1.0f / (float)clip->fps;
        while (m->frame_clock >= step) {
            m->frame_clock -= step;
            m->frame++;

            if (m->frame >= clip->count) {
                if (clip->loop) {
                    m->frame = 0;
                } else {
                    m->frame = clip->count - 1;
                    m->anim_done = 1;
                    break;
                }
            }
        }
        if (m->anim_done) {
            switch (m->anim) {
                case MOB_ANIM_ATTACK:
                    mob_resolve_attack(m);
                    mob_play_anim(m, MOB_ANIM_IDLE);
                    break;
                case MOB_ANIM_RUN:
                    mob_play_anim(m, MOB_ANIM_IDLE);
                    break;
                case MOB_ANIM_DIE:
                    break;

                default:
                    break;
            }
        }
    }
}

/*═══════════════════════════════════════════════════════════════
   stupid AI, fully redo
═══════════════════════════════════════════════════════════════*/
static int mob_is_busy(const Mob *m) {
    switch (m->anim) {
        case MOB_ANIM_ATTACK:
        case MOB_ANIM_DIE:
            return !m->anim_done;

        default:
            return 0;
    }
}

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
    if (m->base.pos != npos) return 0;

    if (dx != 0) m->facing = (dx < 0) ? FACE_LEFT : FACE_RIGHT;
    mob_play_anim(m, MOB_ANIM_RUN);
    return 1;
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
    if (mob_is_busy(m)) {
        char_spend(self, ACTOR_TICK / MOB_DB[m->kind].base_speed);
        actor_next(&self->base);
        return 0;
    }
    if (m->target && !char_is_alive(m->target)) {
        m->target = NULL;
    }
    mob_play_anim(m, MOB_ANIM_IDLE);

    /* State transitions */
    if (m->state == MOB_ASLEEP || m->state == MOB_WANDERING) {
        if (m->target && char_distance(self, m->target) <= self->view_distance) {
            m->state = MOB_HUNTING;
        }
    } else if (m->state == MOB_HUNTING) {
        if (!m->target || char_distance(self, m->target) > self->view_distance * 2) {
            m->state = MOB_WANDERING;
        }
    }

    switch (m->state) {
        case MOB_ASLEEP:
            break;

        case MOB_HUNTING:
            if (char_distance(self, m->target) <= 1) {
                Point mp = idx_to_xy(self->pos);
                Point tp = idx_to_xy(m->target->pos);

                if (tp.x != mp.x)
                    m->facing = (tp.x < mp.x) ? FACE_LEFT : FACE_RIGHT;

                mob_play_anim(m, MOB_ANIM_ATTACK);
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
    m->base.flying        = stats->flying;

    m->kind   = kind;
    m->state  = stats->initial_state;
    m->target = target;

    m->facing      = FACE_RIGHT;
    m->anim        = MOB_ANIM_IDLE;
    m->frame       = 0;
    m->frame_clock = 0.0f;
    m->anim_done   = 0;

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