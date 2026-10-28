#include "buff.h"
#include "char.h"
#include "rng.h"

Buff *char_buff(Char *self, BuffType type) {
    for (int i = 0; i < MAX_CHAR_BUFFS; i++) {
        if (self->buffs[i].type == type) return &self->buffs[i];
    }
    return NULL;
}

/* ---- Poison ---- */
static int buff_poison_act(Buff *self) {
    Char *target = self->target;
    if (!target) return 1;
    if (!char_is_alive(target)) { char_remove_buff(target, self); return 1; }

    int dmg = (int)(self->duration / 3.0f) + 1;
    char_damage(target, dmg, self, DMGSRC_POISON);
    if (self->type == BUFF_NONE) return 1;

    actor_spend(&self->base, ACTOR_TICK);
    self->duration -= ACTOR_TICK;
    if (self->duration <= 0.0f) char_remove_buff(target, self);
    return 1;
}

void char_poison_set(Char *self, float duration) {
    Buff *b = char_apply_buff(self, BUFF_POISON, 0.0f);
    if (b) b->duration = duration;
}

/* ---- Bleeding ---- */
void char_bleeding_grant(Char *self, int level) {
    Buff *b = char_apply_buff(self, BUFF_BLEEDING, -1.0f);
    if (b) b->data = level;
}

static int buff_bleeding_act(Buff *self) {
    Char *target = self->target;
    if (!target) return 1;
    if (!char_is_alive(target)) { char_remove_buff(target, self); return 1; }

    self->data = rng_range(self->data / 2, self->data);
    if (self->data <= 0) { char_remove_buff(target, self); return 1; }

    char_damage(target, self->data, self, DMGSRC_GENERIC);
    if (self->type == BUFF_NONE) return 1;

    actor_spend(&self->base, ACTOR_TICK);
    return 1;
}

/* ---- Burning ---- */
static int buff_burning_act(Buff *self) {
    Char *target = self->target;
    if (!target) return 1;
    if (!char_is_alive(target)) { char_remove_buff(target, self); return 1; }

    if (target->kind == CHAR_KIND_HERO) {
        char_apply_buff(target, BUFF_LIGHT, ACTOR_TICK * 1.01f); /* matches the original's Hero-only "fire lights you up" */
    }

    char_damage(target, rng_range(1, 5), self, DMGSRC_FIRE);
    if (self->type == BUFF_NONE) return 1;

    actor_spend(&self->base, ACTOR_TICK);
    self->duration -= ACTOR_TICK;

    /* TODO: remove buff earlier while standing in water */
    float extinguish_chance = (2.0f + (float)target->HP / (float)BUFF_HP_CAP_SAFE(target->HT)) / 3.0f;
    if (self->duration <= 0.0f || rng_float() > extinguish_chance) {
        char_remove_buff(target, self);
    }
    return 1;
}

/* ---- Shadows ---- */
static int buff_shadows_act(Buff *self) {
    Char *target = self->target;
    if (!target) return 1;
    if (!char_is_alive(target)) { char_remove_buff(target, self); return 1; }

    actor_spend(&self->base, ACTOR_TICK * 2.0f);
    self->duration -= 1.0f;
    /* TODO: the original also breaks shadowmeld the instant the hero can see a live enemy */
    if (self->duration <= 0.0f) char_remove_buff(target, self);
    return 1;
}

/* ---- Fury ---- */
static int buff_fury_act(Buff *self) {
    Char *target = self->target;
    if (target && target->HP > (int)(target->HT * FURY_LEVEL)) {
        char_remove_buff(target, self);
    }
    actor_spend(&self->base, ACTOR_TICK);
    return 1;
}

/* ---- Paralysis / Frost ---- */
static void char_try_unfreeze(Char *self) {
    if (!char_buff(self, BUFF_PARALYSIS) && !char_buff(self, BUFF_FROST)) {
        self->paralysed = 0;
    }
}

static int buff_paralysis_attach(Char *self, Buff *b) { (void)b; self->paralysed = 1; return 1; }
static void buff_paralysis_detach(Char *self, Buff *b) { (void)b; char_try_unfreeze(self); }

static int buff_frost_attach(Char *self, Buff *b) {
    (void)b;
    self->paralysed = 1;
    char_remove_buffs_of_type(self, BUFF_BURNING);
    return 1;
}
static void buff_frost_detach(Char *self, Buff *b) { (void)b; char_try_unfreeze(self); }

/* ---- Roots ---- */
static int buff_roots_attach(Char *self, Buff *b) {
    (void)b;
    if (self->flying) return 0; /* can't root what's off the ground */
    self->rooted = 1;
    return 1;
}
static void buff_roots_detach(Char *self, Buff *b) { (void)b; self->rooted = 0; }

/* ---- Levitation ---- */
static int buff_levitation_attach(Char *self, Buff *b) {
    (void)b;
    self->flying = 1;
    char_remove_buffs_of_type(self, BUFF_ROOTS);
    return 1;
}
static void buff_levitation_detach(Char *self, Buff *b) {
    (void)b;
    self->flying = 0;
    /* TODO: the original also re-checks for a pit underfoot here */
}

/* ---- Invisibility — Shadows ---- */
static int buff_invisibility_attach(Char *self, Buff *b) { (void)b; self->invisible++; return 1; }
static void buff_invisibility_detach(Char *self, Buff *b) { (void)b; if (self->invisible > 0) self->invisible--; }

/* ---- Light ---- */
static int buff_light_attach(Char *self, Buff *b) {
    b->data = self->view_distance;
    if (self->view_distance < LIGHT_DISTANCE) self->view_distance = LIGHT_DISTANCE;
    return 1;
}
static void buff_light_detach(Char *self, Buff *b) { self->view_distance = b->data; }

/* ---- Mind Vision ---- */
static int buff_mind_vision_attach(Char *self, Buff *b) { (void)self; b->data = 2; return 1; } //TODO

/* ---- Ooze ---- */
static int buff_ooze_attach(Char *self, Buff *b) { (void)self; b->data = 1; return 1; }
static int buff_ooze_act(Buff *self) {
    Char *target = self->target;
    if (!target) return 1;
    if (!char_is_alive(target)) { char_remove_buff(target, self); return 1; }

    char_damage(target, self->data, self, DMGSRC_ACID);
    if (self->type == BUFF_NONE) return 1;

    actor_spend(&self->base, ACTOR_TICK);
    /* TODO: the original also force-detaches while standing in water */
    return 1;
}

/* ---- Barkskin ---- */
static int buff_barkskin_act(Buff *self) {
    Char *target = self->target;
    if (!target) return 1;
    if (!char_is_alive(target)) { char_remove_buff(target, self); return 1; }

    actor_spend(&self->base, ACTOR_TICK);
    self->data -= 1;
    if (self->data <= 0) char_remove_buff(target, self);
    return 1;
}
void char_barkskin_grant(Char *self, int level) {
    /* mirrors Buff.affect(ch, Barkskin.class).level(value): find-or-create,
       then extend-if-greater on the LEVEL specifically — char_apply_buff's
       own extend-if-greater merge operates on `duration`, which Barkskin
       doesn't use at all, so this can't just be a plain char_apply_buff() call. */
    Buff *b = char_apply_buff(self, BUFF_BARKSKIN, 0.0f);
    if (b && level > b->data) b->data = level;
}

/* ---- Hunger ---- */
static int buff_hunger_act(Buff *self) {
    Char *target = self->target;
    if (!target) return 1;
    if (!char_is_alive(target)) { char_remove_buff(target, self); return 1; }

    if (self->duration >= HUNGER_STARVING) {
        if (rng_float() < 0.3f && (target->HP > 1 || !target->paralysed)) {
            char_damage(target, 1, self, DMGSRC_GENERIC);
            if (self->type == BUFF_NONE) return 1;
            /* TODO: the original also calls hero.interrupt() here */
        }
    } else {
        /* Ring of Satiety's bonus isn't ported — always 0 */
        self->duration += HUNGER_STEP;
        /* TODO: the original also logs "You are hungry."/"You are
           starving!" on the two threshold crossings here. */
    }

    float step = HUNGER_STEP * target->vt->hunger_tick_scale(target);
    if (char_buff(target, BUFF_SHADOWS)) step *= 1.5f;
    actor_spend(&self->base, step);
    return 1;
}
void char_hunger_satisfy(Char *self, float energy) {
    Buff *h = char_buff(self, BUFF_HUNGER);
    if (!h) return;
    h->duration -= energy;
    if (h->duration < 0.0f) h->duration = 0.0f;
    else if (h->duration > HUNGER_STARVING) h->duration = HUNGER_STARVING;
}
int char_hunger_is_starving(Char *self) {
    Buff *h = char_buff(self, BUFF_HUNGER);
    return h && h->duration >= HUNGER_STARVING;
}

/* ---- Regeneration ---- */
static int buff_regeneration_act(Buff *self) {
    Char *target = self->target;
    if (!target) return 1;
    if (!char_is_alive(target)) { char_remove_buff(target, self); return 1; }

    if (target->HP < target->HT && !char_hunger_is_starving(target)) {
        target->HP += 1;
    }
    /* Ring of Mending's bonus isn't ported*/
    actor_spend(&self->base, REGENERATION_DELAY);
    return 1;
}

/* ---- Combo: Gladiator-specific ---- */
int char_combo_hit(Char *self, Char *enemy, int damage) {
    (void)enemy;
    Buff *combo = char_apply_buff(self, BUFF_COMBO, 0.0f);
    if (!combo) return 0;

    combo->data++;

    if (combo->data >= 3) {
        actor_postpone(&combo->base, 1.41f - (float)combo->data / 10.0f);
        return (int)(damage * (combo->data - 2) / 5.0f);
    }
    actor_postpone(&combo->base, 1.1f);
    return 0;
}

/* ---- SnipersMark: Sniper-specific ---- */
int char_consume_snipers_mark(Char *self, Char *target) {
    Buff *mark = char_buff(self, BUFF_SNIPERS_MARK);
    if (!mark) return 0;
    int matched = target && mark->data == actor_id(&target->base);
    char_remove_buff(self, mark);
    return matched;
}

/* ---- Charm ---- */
int char_is_charmed_by(Char *self, Char *ch) {
    int ch_id = actor_id(&ch->base);
    for (int i = 0; i < MAX_CHAR_BUFFS; i++) {
        if (self->buffs[i].type == BUFF_CHARM && self->buffs[i].data == ch_id) return 1;
    }
    return 0;
}

static const BuffVTable BUFF_VTABLES[BUFF_TYPE_COUNT] = {
    [BUFF_POISON]       = { .act = buff_poison_act },
    [BUFF_BLEEDING]     = { .act = buff_bleeding_act },
    [BUFF_BURNING]      = { .act = buff_burning_act },
    [BUFF_FROST]        = { .on_attach = buff_frost_attach,       .on_detach = buff_frost_detach },
    [BUFF_PARALYSIS]    = { .on_attach = buff_paralysis_attach,   .on_detach = buff_paralysis_detach },
    [BUFF_ROOTS]        = { .on_attach = buff_roots_attach,       .on_detach = buff_roots_detach },
    [BUFF_LEVITATION]   = { .on_attach = buff_levitation_attach,  .on_detach = buff_levitation_detach },
    [BUFF_INVISIBILITY] = { .on_attach = buff_invisibility_attach, .on_detach = buff_invisibility_detach },
    [BUFF_SHADOWS]      = { .on_attach = buff_invisibility_attach, .on_detach = buff_invisibility_detach, .act = buff_shadows_act },
    [BUFF_LIGHT]        = { .on_attach = buff_light_attach,       .on_detach = buff_light_detach },
    [BUFF_FURY]         = { .act = buff_fury_act },
    [BUFF_MIND_VISION]  = { .on_attach = buff_mind_vision_attach },
    [BUFF_OOZE]         = { .on_attach = buff_ooze_attach, .act = buff_ooze_act },
    [BUFF_BARKSKIN]     = { .act = buff_barkskin_act },
    [BUFF_HUNGER]       = { .act = buff_hunger_act },
    [BUFF_REGENERATION] = { .act = buff_regeneration_act },
};

static const BuffVTable *buff_vt(BuffType type) {
    static const BuffVTable empty = {0};
    if (type <= BUFF_NONE || type >= BUFF_TYPE_COUNT) return &empty;
    return &BUFF_VTABLES[type];
}

/* ══════════════════════════════════════════════════════════
   Actor <-> BUFF glue
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
    Buff *b = (Buff *)a;
    const BuffVTable *vt = buff_vt(b->type);
    return vt->act ? vt->act(b) : buff_default_act(b);
}
static const ActorVTable buff_actor_vtable = {
    .act        = buff_actor_act,
    .on_remove  = NULL,
};

static int buff_is_timed(BuffType type, float duration) {
    return buff_vt(type)->act == NULL && duration > 0.0f;
}
static int buff_ceil_ticks(float duration) {
    int n = (int)duration;
    return ((float)n < duration) ? n + 1 : n;
}
static float buff_first_step(float duration) {
    return duration - (float)(buff_ceil_ticks(duration) - 1);   /* in (0, 1] */
}

Buff *char_apply_buff(Char *self, BuffType type, float duration) {
    Buff *existing = char_buff(self, type);
    if (existing) {
        if (buff_is_timed(type, duration) && existing->duration > 0.0f) {
            /* Buff.prolong(): only ever pushes the expiry later */
            float old_expiry = existing->base.time + (float)(buff_ceil_ticks(existing->duration) - 1);
            float new_expiry = actor_now() + duration;
            if (new_expiry > old_expiry) {
                existing->duration  = duration;
                existing->base.time = actor_now() + buff_first_step(duration);
            }
        } else if (duration > existing->duration) {
            existing->duration = duration;
        }
        return existing;
    }

    if (self->vt->buff_immune && self->vt->buff_immune(self, type)) {
        return NULL; /* mirrors Buff.attachTo()'s immunity check */
    }

    for (int i = 0; i < MAX_CHAR_BUFFS; i++) {
        if (self->buffs[i].type == BUFF_NONE) {
            Buff *b = &self->buffs[i];
            actor_init(&b->base, &buff_actor_vtable);
            b->type     = type;
            b->target   = self;
            b->duration = duration;
            b->data     = 0;

            const BuffVTable *vt = buff_vt(type);
            if (vt->on_attach && !vt->on_attach(self, b)) {
                b->type   = BUFF_NONE; /* refused — e.g. Roots vs. a flying Char */
                b->target = NULL;
                return NULL;
            }

            actor_add(&b->base);
            if (buff_is_timed(type, duration)) actor_postpone(&b->base, buff_first_step(duration));

            self->vt->on_buff_added(self, b);
            return b;
        }
    }
    return NULL; /* out of buff slots — raise MAX_CHAR_BUFFS if this ever fires */
}

void char_remove_buff(Char *self, Buff *buff) {
    if (!buff || buff->type == BUFF_NONE) return;

    BuffType type = buff->type;

    self->vt->on_buff_removed(self, buff); /* fires while buff->type still reflects what's being removed */

    actor_remove(&buff->base);
    buff->type   = BUFF_NONE;   /* cleared before on_detach so its own char_buff() checks (Frost/Paralysis) see this slot as gone */
    buff->target = NULL;

    const BuffVTable *vt = buff_vt(type);
    if (vt->on_detach) vt->on_detach(self, buff);
}

void char_remove_buffs_of_type(Char *self, BuffType type) {
    for (int i = 0; i < MAX_CHAR_BUFFS; i++) {
        if (self->buffs[i].type == type) {
            char_remove_buff(self, &self->buffs[i]);
        }
    }
}

void char_restore_buffs(Char *self) {
    float target = self->base.time;
    for (int i = 0; i < MAX_CHAR_BUFFS; i++) {
        Buff *b = &self->buffs[i];
        if (b->type != BUFF_NONE) {
            b->base.time = target - actor_now();
            actor_add(&b->base);
        }
    }
}