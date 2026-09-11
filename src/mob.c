#include "mob.h"
#include "mob_bestiary.h"
#include "pathfinder.h"
#include "rng.h"
#include <stdlib.h>

/*═══════════════════════════════════════════════════════════════
    Storage
═══════════════════════════════════════════════════════════════*/
static MobPool *active_mob_pool = NULL;
static Char *hero = NULL;

void mob_set_pool(MobPool *pool) {
    active_mob_pool = pool;
}

void mob_set_hero(Char *player) {
    hero = player;
}

/*═══════════════════════════════════════════════════════════════
    Combat
═══════════════════════════════════════════════════════════════*/
static int mob_attack_skill(Char *self, Char *target) {
    (void)target;
    return MOB_DB[((Mob *)self)->kind].attack_skill;
}
static int mob_defense_skill(Char *self, Char *enemy) {
    (void)enemy;
    Mob *m = (Mob *)self;
    return (m->enemy_seen && !self->paralysed) ? MOB_DB[m->kind].defense_skill : 0;
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
    char_remove_buffs_of_type(self, BUFF_TERROR);

    if (m->state == MOB_ASLEEP) m->state = MOB_WANDERING;
    m->alerted = 1;

    if (src_type == DMGSRC_CHAR && src) {
        /* Not in the original — chooseEnemy() there would fall back
           to the hero here instead — but keeps a mob correctly
           hunting whatever actually hit it, hero or not. Kept from
           this port's existing behavior. */
        m->target = (Char *)src;
    }
}

static void (*kill_reward_hook)(Char *killer, int xp) = NULL;

void mob_set_kill_reward_hook(void (*fn)(Char *killer, int xp)) {
    kill_reward_hook = fn;
}

static void mob_die(Char *self, void *src, DamageSource src_type) {
    Mob *m = (Mob *)self;

    mob_play_anim(m, MOB_ANIM_DIE);

    if (src_type == DMGSRC_CHAR && src && kill_reward_hook) {
        kill_reward_hook((Char *)src, MOB_DB[m->kind].xp_reward);
    }

    char_die_default(self, src, src_type);
    /* TODO ADD DROP HERE*/
}

static void mob_on_buff_added(Char *self, Buff *buff) {
    Mob *m = (Mob *)self;
    switch (buff->type) {
    case BUFF_AMOK:
        m->state = MOB_HUNTING;
        break;
    case BUFF_TERROR:
        m->state = MOB_FLEEING;
        break;
    case BUFF_SLEEP:
        m->state = MOB_ASLEEP;
        actor_postpone(&self->base, ACTOR_TICK);
        break;
    default:
        break;
    }
}

static void mob_on_buff_removed(Char *self, Buff *buff) {
    if (buff->type == BUFF_TERROR) {
        ((Mob *)self)->state = MOB_HUNTING;
    }
}

/*═══════════════════════════════════════════════════════════════
    Animation
═══════════════════════════════════════════════════════════════*/
static void mob_sync_anim_lock(Mob *m) {
    switch (m->anim) {
        case MOB_ANIM_ATTACK:
        case MOB_ANIM_DIE:
            m->base.anim_locked = !m->anim_done;
            break;
        default:
            m->base.anim_locked = 0;
    }
}

static int mob_is_busy(const Mob *m) {
    return m->base.anim_locked;
}

static void mob_resolve_attack(Mob *m) {
    Char *target = m->target;
    if (target && char_is_alive(target)) {
        char_attack(&m->base, target);
    } else {
        m->target = NULL;
    }
    actor_next(&m->base.base);
}

void mob_play_anim(Mob *m, MobAnimState anim) {
    if (m->anim == anim && !m->anim_done)
        return;

    m->anim = anim;
    m->frame = 0;
    m->frame_clock = 0.0f;
    m->anim_done = 0;
    mob_sync_anim_lock(m);
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
                    char_end_attack_anim(&m->base);
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
        mob_sync_anim_lock(m); /* unconditional: covers MOB_ANIM_DIE, which holds its last frame and never re-enters mob_play_anim() above */
    }
}
/*═══════════════════════════════════════════════════════════════
    MOB FOV
═══════════════════════════════════════════════════════════════*/
static int mob_has_los(const Level *lvl, int from_pos, int to_pos) { //REDO AS PLAYER FOV
    Point a = idx_to_xy(from_pos);
    Point b = idx_to_xy(to_pos);

    int x0 = a.x;
    int y0 = a.y;
    int x1 = b.x;
    int y1 = b.y;

    int dx = abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;

    while (true) {
        if (!(x0 == a.x && y0 == a.y)) {
            int pos = xy_to_idx(x0, y0);

            if (gTileFlags[lvl->map[pos]] & TF_LOS_BLOCKING)
                return 0;
        }
        if (x0 == x1 && y0 == y1)
            break;
        int e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
    return 1;
}
static int mob_can_see(Mob *m, Char *target, Level *lvl){
    if (!target || !char_is_alive(target))
        return 0;
    if (target->invisible > 0)
        return 0;
    if (char_distance(&m->base, target) > m->base.view_distance)
        return 0;

    return mob_has_los(lvl, m->base.pos, target->pos);
}
/*═══════════════════════════════════════════════════════════════
    MOB AI
═══════════════════════════════════════════════════════════════*/
#define MOB_TIME_TO_WAKE_UP 1.0f

static int mob_try_step(Mob *m, Level *lvl, int npos) {
    if (m->base.rooted) return 0;
    if (!in_bounds(npos)) return 0;
    if (npos == m->base.pos) return 0;

    Point mp = idx_to_xy(m->base.pos);
    Point np = idx_to_xy(npos);

    int dx = np.x - mp.x;
    int dy = np.y - mp.y;

    /* Must be an adjacent tile. */
    if (dx < -1 || dx > 1 || dy < -1 || dy > 1) return 0;
    if (!(gTileFlags[lvl->map[npos]] & TF_PASSABLE)) return 0;
    if (char_find_at(npos) != NULL) return 0;

    char_move(&m->base, lvl, npos);
    if (m->base.pos != npos) return 0;

    if (dx != 0) m->facing = (dx < 0) ? FACE_LEFT : FACE_RIGHT;
    mob_play_anim(m, MOB_ANIM_RUN);
    return 1;
}

static int mob_step_toward_pos(Mob *m, Level *lvl, int dest_pos) {
    int next = pathfinder_get_step(lvl, m->base.pos, dest_pos);
    if (next == -1)
        return 0;
    return mob_try_step(m, lvl, next);
}

static int mob_step_away_pos(Mob *m, Level *lvl, int from_pos) {
    int next = pathfinder_get_step_back(lvl, m->base.pos, from_pos);
    if (next == -1)
        return 0;
    return mob_try_step(m, lvl, next);
}

static int mob_random_destination(Level *lvl) {
    for (int tries = 0; tries < 100; tries++) {
        int pos = rng_int(LEVEL_SIZE);
        if (gTileFlags[lvl->map[pos]] & TF_PASSABLE) return pos;
    }
    return -1;
}

static int char_stealth_of(Char *c) {
    return (c->vt && c->vt->stealth) ? c->vt->stealth(c) : 0;
}

static Mob *mob_pick_amok_target(Mob *self, Level *lvl) {
    int count = mob_count();
    if (count <= 1) return NULL;

    int start = rng_int(count);
    for (int i = 0; i < count; i++) {
        Mob *cand = mob_at_index((start + i) % count);
        if (cand != self && char_is_alive(&cand->base) && mob_can_see(self, &cand->base, lvl))
            return cand;
    }
    return NULL;
}

static int mob_has_valid_target(const Mob *m) {
    return m->target && char_is_alive(m->target);
}

static Char *mob_choose_enemy(Mob *m, Level *lvl) {
    if (char_buff(&m->base, BUFF_AMOK)) {
        if (m->target == hero || m->target == NULL) {
            Mob *pick = mob_pick_amok_target(m, lvl);
            if (pick) return &pick->base;
        }
    }

    if (mob_has_valid_target(m)) return m->target;
    return hero;
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

    /* TEST alert icon lasted until this turn */
    m->alert_icon = 0;

    int just_alerted = m->alerted;
    m->alerted = 0;

    if (self->paralysed) {
        m->enemy_seen = 0;
        char_spend(self, ACTOR_TICK);
        actor_next(&self->base);
        return 0;
    }

    m->target = mob_choose_enemy(m, lvl);
    int enemy_in_view = mob_can_see(m, m->target, lvl);

    int attacking = 0;

    switch (m->state) {
    case MOB_ASLEEP: {
        int wake_odds = enemy_in_view
            ? char_distance(self, m->target) + char_stealth_of(m->target) + (m->target->flying ? 2 : 0)
            : 1;
        if (enemy_in_view && (wake_odds <= 0 || rng_int(wake_odds) == 0)) {
            m->enemy_seen = 1;
            m->state = MOB_HUNTING;
            m->dest = m->target->pos;

            m->alert_icon = 1;   //TEST

            char_spend(self, MOB_TIME_TO_WAKE_UP);
        } else {
            m->enemy_seen = 0;
            char_spend(self, ACTOR_TICK);
        }
        break;
    }

    case MOB_WANDERING: {
        int wake_odds = enemy_in_view
            ? char_distance(self, m->target) / 2 + char_stealth_of(m->target)
            : 1;
        if (enemy_in_view && (just_alerted || wake_odds <= 0 || rng_int(wake_odds) == 0)) {
            m->enemy_seen = 1;
            m->state = MOB_HUNTING;
            m->dest = m->target->pos;
            char_spend(self, ACTOR_TICK / MOB_DB[m->kind].base_speed);
        } else {
            m->enemy_seen = 0;
            if (m->dest != -1 && mob_step_toward_pos(m, lvl, m->dest)) {
                char_spend(self, ACTOR_TICK / MOB_DB[m->kind].base_speed);
            } else {
                m->dest = mob_random_destination(lvl);
                char_spend(self, ACTOR_TICK);
            }
        }
        break;
    }

    case MOB_HUNTING:
        m->enemy_seen = enemy_in_view;
        if (enemy_in_view && char_distance(self, m->target) <= 1 && !char_is_charmed_by(self, m->target)) {
            Point mp = idx_to_xy(self->pos);
            Point tp = idx_to_xy(m->target->pos);

            if (tp.x != mp.x)
                m->facing = (tp.x < mp.x) ? FACE_LEFT : FACE_RIGHT;

            mob_play_anim(m, MOB_ANIM_ATTACK);
            char_begin_attack_anim(self, m->target);
            char_spend(self, MOB_DB[m->kind].attack_delay);
            
            attacking = 1;
            mob_resolve_attack(m);
        } else {
            if (enemy_in_view) m->dest = m->target->pos;
            if (m->dest != -1 && mob_step_toward_pos(m, lvl, m->dest)) {
                char_spend(self, ACTOR_TICK / MOB_DB[m->kind].base_speed);
            } else {
                char_spend(self, ACTOR_TICK);
                m->state = MOB_WANDERING;
                m->dest = mob_random_destination(lvl);
            }
        }
        break;

    case MOB_FLEEING:
        m->enemy_seen = enemy_in_view;
        if (enemy_in_view) m->dest = m->target->pos;
        if (m->dest != -1 && mob_step_away_pos(m, lvl, m->dest)) {
            char_spend(self, ACTOR_TICK / MOB_DB[m->kind].base_speed);
        } else {
            char_spend(self, ACTOR_TICK); /* nowhereToRun() in the original is an empty hook (no non-Fleeing-mob overrides it) — nothing to port */
        }
        break;

    case MOB_PASSIVE:
    default:
        m->enemy_seen = 0;
        char_spend(self, ACTOR_TICK);
        break;
    }

    if (!attacking) actor_next(&self->base);
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
        vt.on_buff_added   = mob_on_buff_added;
        vt.on_buff_removed = mob_on_buff_removed;
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
    if (char_find_at(pos) != NULL) return NULL;

    /* Reuse a dead mob's slot */
    Mob *m = NULL;
    for (int i = 0; i < active_mob_pool->count; i++) {
        Mob *candidate = &active_mob_pool->mobs[i];

        if (!char_is_alive(&candidate->base) &&
            candidate->anim == MOB_ANIM_DIE &&
            candidate->anim_done) {
            m = candidate;
            break;
        }
    }

    int reused = (m != NULL);
    if (!m) {
        if (active_mob_pool->count >= MAX_LEVEL_MOBS) return NULL;
        m = &active_mob_pool->mobs[active_mob_pool->count];
    }

    const MobStats *stats = &MOB_DB[kind];

    char_init(&m->base, mob_vtable(), CHAR_KIND_MOB, pos, stats->ht);
    m->base.name          = stats->name;
    m->base.base_speed    = stats->base_speed;
    m->base.view_distance = stats->view_distance;
    m->base.flying        = stats->flying;

    m->kind   = kind;
    m->state  = stats->initial_state;
    m->target = target;
    m->dest       = -1;
    m->enemy_seen = 0;
    m->alerted    = 0;

    m->facing      = FACE_RIGHT;
    m->anim        = MOB_ANIM_IDLE;
    m->frame       = 0;
    m->frame_clock = 0.0f;
    m->anim_done   = 0;

    if (!char_add(&m->base)) return NULL; 

    if (!reused) active_mob_pool->count++;
    return m;
}

int mob_spawn_random(Level *lvl, MobKind kind, int count, Char *target) {
    int placed = 0;
    int attempts = 0;
    int max_attempts = count * 20; 

    while (placed < count && attempts < max_attempts) {
        attempts++;

        int pos = rng_int(LEVEL_SIZE);

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

void mob_spawn_initial(Level *lvl, int depth) {
    int n = bestiary_mob_count(depth);
    if (n > MAX_LEVEL_MOBS) n = MAX_LEVEL_MOBS;

    for (int i = 0; i < n; i++) {
        MobKind kind = bestiary_pick(depth);
        mob_spawn_random_no_target(lvl, kind, 1);
    }
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
    hero = player;
    for (int i = 0; i < active_mob_pool->count; i++) {
        Mob *m = &active_mob_pool->mobs[i];
        if (!mob_has_valid_target(m)) m->target = player;
    }
}