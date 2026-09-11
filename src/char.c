#include "char.h"
#include "level.h"
#include "rng.h"
#include <string.h>
/* ══════════════════════════════════════════════════════════
   Level-derived geometry HELPERS  //TODO REMOVE SOMEWHERE + USE IT ALREADY
   ══════════════════════════════════════════════════════════ */
static int char_cell_distance(int a, int b) {
    Point pa = idx_to_xy(a);
    Point pb = idx_to_xy(b);
    int dx = pa.x - pb.x; if (dx < 0) dx = -dx;
    int dy = pa.y - pb.y; if (dy < 0) dy = -dy;
    return dx > dy ? dx : dy;
}
static int char_cells_adjacent(int a, int b) {
    return char_cell_distance(a, b) == 1;
}

int char_neighbour_idx(int pos, int dx, int dy) {
    Point p = idx_to_xy(pos);
    int nx = p.x + dx;
    int ny = p.y + dy;
    if (!in_bounds_xy(nx, ny))
        return -1;
    return xy_to_idx(nx, ny);
}

static int char_random_neighbour(int pos) {
    static const int dx8[8] = { -1, 0, 1, -1, 1, -1, 0, 1 };
    static const int dy8[8] = { -1, -1, -1, 0, 0, 1, 1, 1 };

    int i = rng_int(8);
    return char_neighbour_idx(pos, dx8[i], dy8[i]);
}

/* ══════════════════════════════════════════════════════════
   Position grid — this port's replacement for Actor.java's
   Char[] chars, moved here since only Chars occupy it (see the
   layering note in actor.h). Maintained incrementally at char_add()/
   char_move()/char_destroy() rather than rebuilt every scheduler
   scan the way the original's process() does — cheaper, and correct
   as long as a Char's pos only ever changes via char_move(), never
   by writing ->pos directly.
   ══════════════════════════════════════════════════════════ */
static Char *chars_by_pos[LEVEL_SIZE];

static int char_grid_occupy(Char *c, int pos){
    if (!c)
        return 0;
    if (pos < 0 || pos >= LEVEL_SIZE)
        return 0;
    if (chars_by_pos[pos] != NULL &&
        chars_by_pos[pos] != c)
        return 0;
    chars_by_pos[pos] = c;
    return 1;
}
static void char_grid_free(int pos) {
    if (pos >= 0 && pos < LEVEL_SIZE) chars_by_pos[pos] = NULL;
}
Char *char_find_at(int pos) {
    if (pos < 0 || pos >= LEVEL_SIZE) return NULL;
    return chars_by_pos[pos];
}
void char_clear(void) {
    memset(chars_by_pos, 0, sizeof(chars_by_pos));
}

/* ══════════════════════════════════════════════════════════
   Default combat numbers — Char.java's own base-class values.
   ══════════════════════════════════════════════════════════ */
static int         default_attack_skill(Char *self, Char *target)  { (void)self; (void)target; return 0; }
static int         default_defense_skill(Char *self, Char *enemy)  { (void)self; (void)enemy; return 0; }
static const char *default_defense_verb(Char *self)                { (void)self; return "dodged"; }
static int         default_dr(Char *self)                          { (void)self; return 0; }
static int         default_damage_roll(Char *self)                 { (void)self; return 1; }
static float       default_speed(Char *self) { return char_buff(self, BUFF_CRIPPLE) ? self->base_speed * 0.5f : self->base_speed; }
static int         default_attack_proc(Char *self, Char *enemy, int dmg)     { (void)self; (void)enemy; return dmg; }
static int         default_defense_proc(Char *self, Char *attacker, int dmg) { (void)self; (void)attacker; return dmg; }
static uint32_t    default_resistances(Char *self) { (void)self; return 0; }
static uint32_t    default_immunities(Char *self)  { (void)self; return 0; }
static int         default_stealth(Char *self)     { (void)self; return 0; }
static void        default_on_damaged(Char *self, int dmg_taken, void *src, DamageSource t) { (void)self; (void)dmg_taken; (void)src; (void)t; }
static void        default_on_buff_added(Char *self, Buff *buff)   { (void)self; (void)buff; }
static void        default_on_buff_removed(Char *self, Buff *buff) { (void)self; (void)buff; }
static void        default_die(Char *self, void *src, DamageSource t) { char_die_default(self, src, t); }
static int         default_on_turn(Char *self)     { (void)self; return 0; }

const CharVTable CHAR_DEFAULT_VTABLE = {
    .attack_skill    = default_attack_skill,
    .defense_skill   = default_defense_skill,
    .defense_verb    = default_defense_verb,
    .dr              = default_dr,
    .damage_roll     = default_damage_roll,
    .speed           = default_speed,
    .attack_proc     = default_attack_proc,
    .defense_proc    = default_defense_proc,
    .resistances     = default_resistances,
    .immunities      = default_immunities,
    .stealth         = default_stealth,
    .on_damaged      = default_on_damaged,
    .on_buff_added   = default_on_buff_added,
    .on_buff_removed = default_on_buff_removed,
    .die             = default_die,
    .on_turn         = default_on_turn,
};

/* ══════════════════════════════════════════════════════════
   Actor <-> Char glue
   ══════════════════════════════════════════════════════════ */
static void (*turn_started_hook)(Char *self) = NULL;

void char_set_turn_started_hook(void (*fn)(Char *self)) {
    turn_started_hook = fn;
}

int char_base_act(Char *self) {
    if (turn_started_hook) turn_started_hook(self);
    return self->vt->on_turn(self);
}

static void char_on_remove(Char *self); /* fwd decl — full body is below the buff functions it calls */

static int char_actor_act(Actor *a) {
    return char_base_act((Char *)a);
}
static void char_actor_on_remove(Actor *a) {
    char_on_remove((Char *)a);
}
static const ActorVTable char_actor_vtable = {
    .act        = char_actor_act,
    .on_remove  = char_actor_on_remove,
};

/* ══════════════════════════════════════════════════════════
   Buffs
   ══════════════════════════════════════════════════════════ */

static int buff_default_act(Buff *self) {
    if (self->duration < 0.0f) {
        /* infinite / externally-managed duration — mirrors Buff.java's
           own base act(): diactivate(); return true; */
        actor_deactivate(&self->base);
        return 1;
    }
    actor_spend(&self->base, ACTOR_TICK);
    self->duration -= ACTOR_TICK;
    if (self->duration <= 0.0f && self->target) {
        char_remove_buff(self->target, self);
    }
    return 1;
}
static int buff_actor_act(Actor *a) {
    return buff_default_act((Buff *)a);
}
static const ActorVTable buff_actor_vtable = {
    .act        = buff_actor_act,
    .on_remove  = NULL,
};

Buff *char_apply_buff(Char *self, BuffType type, float duration) {
    Buff *existing = char_buff(self, type);
    if (existing) {
        if (duration > existing->duration) existing->duration = duration;
        return existing;
    }

    for (int i = 0; i < MAX_CHAR_BUFFS; i++) {
        if (self->buffs[i].type == BUFF_NONE) {
            Buff *b = &self->buffs[i];
            actor_init(&b->base, &buff_actor_vtable);
            b->type     = type;
            b->target   = self;
            b->duration = duration;
            b->data     = 0;

            actor_add(&b->base);

            self->vt->on_buff_added(self, b);
            return b;
        }
    }
    return NULL; /* out of buff slots — raise MAX_CHAR_BUFFS if this ever fires */
}

void char_remove_buff(Char *self, Buff *buff) {
    if (!buff || buff->type == BUFF_NONE) return;

    self->vt->on_buff_removed(self, buff); /* fires while buff->type still reflects what's being removed */

    actor_remove(&buff->base);
    buff->type   = BUFF_NONE;
    buff->target = NULL;
}

void char_remove_buffs_of_type(Char *self, BuffType type) {
    for (int i = 0; i < MAX_CHAR_BUFFS; i++) {
        if (self->buffs[i].type == type) {
            char_remove_buff(self, &self->buffs[i]);
        }
    }
}

Buff *char_buff(Char *self, BuffType type) {
    for (int i = 0; i < MAX_CHAR_BUFFS; i++) {
        if (self->buffs[i].type == type) return &self->buffs[i];
    }
    return NULL;
}

int char_buffs_of_type(Char *self, BuffType type, Buff **out, int max_out) {
    int n = 0;
    for (int i = 0; i < MAX_CHAR_BUFFS && n < max_out; i++) {
        if (self->buffs[i].type == type) out[n++] = &self->buffs[i];
    }
    return n;
}

int char_is_charmed_by(Char *self, Char *ch) {
    int ch_id = actor_id(&ch->base);
    for (int i = 0; i < MAX_CHAR_BUFFS; i++) {
        if (self->buffs[i].type == BUFF_CHARM && self->buffs[i].data == ch_id) return 1;
    }
    return 0;
}

uint32_t char_visual_flags(const Char *self) {
    uint32_t flags = 0;
    for (int i = 0; i < MAX_CHAR_BUFFS; i++) {
        switch (self->buffs[i].type) {
            case BUFF_BURNING:      flags |= CHAR_VIS_BURNING;     break;
            case BUFF_LEVITATION:   flags |= CHAR_VIS_LEVITATING;  break;
            case BUFF_FROST:        flags |= CHAR_VIS_FROZEN;      break;
            case BUFF_PARALYSIS:    flags |= CHAR_VIS_PARALYSED;   break;
            case BUFF_LIGHT:        flags |= CHAR_VIS_ILLUMINATED; break;
            case BUFF_INVISIBILITY:
            case BUFF_SHADOWS:      flags |= CHAR_VIS_INVISIBLE;   break; /* Shadows subclasses Invisibility in the original */
            default: break;
        }
    }
    if (self->invisible > 0) flags |= CHAR_VIS_INVISIBLE; /* the original tracks invisibility via both this counter and the buff class */
    return flags;
}

static void char_restore_buffs(Char *self) {
    for (int i = 0; i < MAX_CHAR_BUFFS; i++) {
        Buff *b = &self->buffs[i];
        if (b->type != BUFF_NONE) {
            b->base.time = 0.0f; /* same "start fresh at now" treatment char_add's callers already give self */
            actor_add(&b->base);
        }
    }
}

static void char_on_remove(Char *self) {
    for (int i = 0; i < MAX_CHAR_BUFFS; i++) {
        if (self->buffs[i].type != BUFF_NONE) {
            char_remove_buff(self, &self->buffs[i]);
        }
    }
}

/* ══════════════════════════════════════════════════════════
   Lifecycle
   ══════════════════════════════════════════════════════════ */
void char_init(Char *self, const CharVTable *vt, CharKind kind, int pos, int max_hp) {
    memset(self, 0, sizeof(*self));
    actor_init(&self->base, &char_actor_vtable);
    self->vt   = vt;
    self->kind = kind;
    self->pos  = pos;
    self->name = "mob";
    self->HT   = max_hp;
    self->HP   = max_hp;
    self->base_speed   = 1.0f;
    self->view_distance = 8;
}

static Level *active_level = NULL;

void char_set_active_level(Level *lvl) {
    active_level = lvl;
}

Level *char_active_level(void) {
    return active_level;
}

int char_add(Char *self) {
    if (!self)
        return 0;

    if (!in_bounds(self->pos))
        return 0;

    if (char_find_at(self->pos) != NULL &&
        char_find_at(self->pos) != self)
        return 0;

    int ok = actor_add(&self->base);
    if (!ok)
        return 0;

    if (!char_grid_occupy(self, self->pos)) {
        actor_unregister(&self->base);
        return 0;
    }

    char_restore_buffs(self);

    return 1;
}
int char_add_delayed(Char *self, float delay) {
    int ok = actor_add_delayed(&self->base, delay);
    if (ok) {
        char_grid_occupy(self, self->pos);
        char_restore_buffs(self);
    }
    return ok;
}

int char_is_alive(const Char *self) {
    return self->HP > 0;
}

void char_destroy(Char *self) {
    char_grid_free(self->pos);
    self->HP = 0;
    actor_remove(&self->base);
}

void char_die_default(Char *self, void *src, DamageSource src_type) {
    (void)src; (void)src_type;
    char_destroy(self);
}

void char_die(Char *self, void *src, DamageSource src_type) {
    self->vt->die(self, src, src_type);
}

/* ══════════════════════════════════════════════════════════
   Position / movement
   ══════════════════════════════════════════════════════════ */
void char_move(Char *self, Level *lvl, int step) {
    if (char_cells_adjacent(step, self->pos) && char_buff(self, BUFF_VERTIGO)) {
        int redirected = char_random_neighbour(self->pos);
        if (redirected < 0
            || !(gTileFlags[lvl->map[redirected]] & TF_PASSABLE)
            || char_find_at(redirected) != NULL) {
            return;
        }
        step = redirected;
    }

    if (lvl->map[self->pos] == TILE_OPEN_DOOR) {
        lvl->map[self->pos] = TILE_DOOR; 
    }

    char_grid_free(self->pos);
    self->pos = step;
    char_grid_occupy(self, self->pos);

    if (lvl->map[self->pos] == TILE_DOOR) {
        lvl->map[self->pos] = TILE_OPEN_DOOR; 
    }
}

int char_distance(const Char *self, const Char *other) {
    return char_cell_distance(self->pos, other->pos);
}

/* ══════════════════════════════════════════════════════════
   Combat
   ══════════════════════════════════════════════════════════ */
int char_hit_roll(Char *attacker, Char *defender, int magic) {
    float acu_roll = rng_float() * attacker->vt->attack_skill(attacker, defender);
    float def_roll = rng_float() * defender->vt->defense_skill(defender, attacker);
    return (magic ? acu_roll * 2.0f : acu_roll) >= def_roll;
}

void char_damage(Char *self, int dmg, void *src, DamageSource src_type) {
    if (self->HP <= 0) return;

    char_remove_buffs_of_type(self, BUFF_FROST);

    uint32_t imm = self->vt->immunities(self);
    uint32_t res = self->vt->resistances(self);
    uint32_t src_bit = (src_type < DMGSRC_COUNT) ? (1u << src_type) : 0u;

    if (imm & src_bit) {
        dmg = 0;
    } else if (res & src_bit) {
        dmg = rng_range(0, dmg);
    }

    Buff *paralysis = char_buff(self, BUFF_PARALYSIS);
    if (paralysis && dmg > 0) {
        if (rng_int(dmg) >= rng_int(self->HP)) {
            char_remove_buffs_of_type(self, BUFF_PARALYSIS);
            /* original logs "The pain snapped %s out of paralysis"
               here when Dungeon.visible[pos] — message-log hook once
               you have one. */
        }
    }

    self->HP -= dmg;
    /* original also calls sprite.showStatus(...) here whenever
       (dmg > 0 || src is a Char) — floating damage-number popup,
       same TODO-style hook as elsewhere in this file. */

    if (self->HP <= 0) {
        char_die(self, src, src_type);
    }

    self->vt->on_damaged(self, dmg, src, src_type); /* fires whether or not this killed self — mirrors Hero.damage()'s post-super logic running unconditionally */
}

float char_speed(Char *self) {
    return self->vt->speed(self);
}

void char_spend(Char *self, float time) {
    float scale = 1.0f;
    if (char_buff(self, BUFF_SLOW))  scale *= 0.5f;
    if (char_buff(self, BUFF_HASTE)) scale *= 2.0f;
    actor_spend(&self->base, time / scale);
}

CharAttackResult char_attack(Char *self, Char *enemy) {
    CharAttackResult result;
    memset(&result, 0, sizeof(result));

    if (char_hit_roll(self, enemy, 0)) {
        result.hit = 1;

        int dr_roll   = rng_range(0, enemy->vt->dr(enemy));
        int dmg_roll  = self->vt->damage_roll(self);
        int effective = dmg_roll - dr_roll;
        if (effective < 0) effective = 0;

        effective = self->vt->attack_proc(self, enemy, effective);
        effective = enemy->vt->defense_proc(enemy, self, effective);

        result.damage_dealt = effective;

        int enemy_was_alive = char_is_alive(enemy);
        char_damage(enemy, effective, self, DMGSRC_CHAR);
        result.enemy_died = enemy_was_alive && !char_is_alive(enemy);

        /* original also: a GLog hit-message line, camera shake scaled
           to damage, blood-burst + flash sprite effects, and (on
           death) either a hero-death/game-over message or a "X
           defeated Y" log line — all presentation/game-flow, build it
           from this result struct. */
    } else {
        result.hit = 0;
        result.defense_verb = enemy->vt->defense_verb(enemy);
        /* original also shows the defense verb as floating text and
           plays a miss sound here — presentation, same as above. */
    }

    return result;
}

int char_is_busy(const Char *self) {
    return self->anim_locked || self->incoming_hits > 0;
}

void char_begin_attack_anim(Char *self, Char *target) {
    self->swing_target = target;
    if (target) target->incoming_hits++;
}

void char_end_attack_anim(Char *self) {
    if (self->swing_target) {
        if (self->swing_target->incoming_hits > 0)
            self->swing_target->incoming_hits--;
        self->swing_target = NULL;
    }
}

/* Placeholder for the case when those actions gonna do something more than actor_next only...
void char_on_motion_complete(Char *self)  { actor_next(&self->base); }
void char_on_attack_complete(Char *self)  { actor_next(&self->base); }
void char_on_operate_complete(Char *self) { actor_next(&self->base); }
*/




void char_unregister(Char *self){
    actor_unregister(&self->base);
    char_grid_free(self->pos);
}