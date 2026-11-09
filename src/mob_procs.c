#include "mob_procs.h"
#include "level_geometry.h"
#include "ballistica.h"
#include "item_wand.h"
#include "particle.h"
#include "item.h"

/* ══════════════════════════════════════════════════════════
   attackProc()
   ══════════════════════════════════════════════════════════ */

/* Bat.attackProc(): reg = min(damage, HT - HP); HP += reg */
int mobproc_vampiric(Mob *self, Char *enemy, int dmg) {
    (void)enemy;
    Char *c = &self->base;
    int heal = c->HT - c->HP;
    if (heal > dmg) heal = dmg;
    if (heal > 0) c->HP += heal;
    return dmg;
}

/* ══════════════════════════════════════════════════════════
   defenseProc()
   ══════════════════════════════════════════════════════════ */

/* Acidic.defenseProc(): enemy.damage(Random.IntRange(0, damage), this) */
int mobproc_acid_reflect(Mob *self, Char *attacker, int dmg) {
    int splash = rng_range(0, dmg);
    if (splash > 0) char_damage(attacker, splash, &self->base, DMGSRC_ACID);
    return dmg;
}

int mobproc_swarm_split(Mob *self, Char *attacker, int dmg) {
    (void)attacker;
    Char *c = &self->base;
    if (c->HP < dmg + 2) return dmg;                        /* HP >= damage + 2 */

    int cell = random_free_neighbour(c->pos, 0);            /* pos +-1, +-WIDTH */
    if (cell < 0) return dmg;

    Mob *clone = mob_spawn(MOB_SWARM, cell, NULL);          /* NULL: it picks its own enemy, like source */
    if (!clone) return dmg;                                 /* MobPool full */

    Level *lvl = char_active_level();
    if (lvl && lvl->map[cell] == TILE_DOOR) lvl->map[cell] = TILE_OPEN_DOOR;   /* Door.enter() */

    clone->base.HP = (c->HP - dmg) / 2;
    c->HP -= clone->base.HP;                                /* the damage itself lands after this proc */
    clone->state = MOB_HUNTING;
    actor_spend(&clone->base.base, 1.0f);      /* GameScene.add(clone, SPLIT_DELAY) */

    /* split(): the clone inherits Burning, and Poison for 2 more turns */
    if (char_buff(c, BUFF_BURNING)) char_apply_buff(&clone->base, BUFF_BURNING, BURNING_DURATION);
    if (char_buff(c, BUFF_POISON)) {
        Buff *p = char_apply_buff(&clone->base, BUFF_POISON, 0.0f);
        if (p) p->duration = 2.0f;
    }
    return dmg;
}

/* Yog.defenseProc(): a Larva on a random free cell around it, every hit */
int mobproc_spawn_larva(Mob *self, Char *attacker, int dmg) {
    (void)attacker;
    int cell = random_free_neighbour(self->base.pos, 1);
    if (cell >= 0) mob_spawn(MOB_YOG_LARVA, cell, NULL);
    return dmg;
}

/* ══════════════════════════════════════════════════════════
   die()
   ══════════════════════════════════════════════════════════ */

/* Skeleton.die(): everything alive around it takes damageRoll() minus a roll of its dr/2 */
void mobproc_skeleton_burst(Mob *self, void *src, DamageSource src_type) {
    (void)src; (void)src_type;
    Char *c = &self->base;
    for (int i = 0; i < 8; i++) {
        Char *ch = char_find_at(neighbour_idx(c->pos, DX[i], DY[i]));
        if (ch && char_is_alive(ch)) {
            int dmg = c->vt->damage_roll(c) - rng_range(0, ch->vt->dr(ch) / 2);
            if (dmg < 0) dmg = 0;
            char_damage(ch, dmg, c, DMGSRC_CHAR);
        }
    }
}

/* ══════════════════════════════════════════════════════════
   add(Buff)
   ══════════════════════════════════════════════════════════ */

/* Elemental.add(): Frost does NormalIntRange(1, HT*2/3) first, then attaches as usual.
   (char_damage() leaves the Frost that is being applied alone, so it isn't cancelled.) */
void mobproc_elemental_frost(Mob *self, Buff *buff) {
    if (buff->type != BUFF_FROST) return;
    Char *c = &self->base;
    char_damage(c, (int)rng_normal_range(1, c->HT * 2 / 3), buff, DMGSRC_GENERIC);
}

/* ══════════════════════════════════════════════════════════
   ranged_check() / ranged_attack()
   ══════════════════════════════════════════════════════════ */
int mobproc_ranged_check_los(Mob *self, Char *enemy, Level *lvl) {
    return ballistica_cast(NULL, lvl, self->base.pos, enemy->pos, 0, 1) == enemy->pos;
}

/* Eye.canAttack(): hitChars=false, magic=true  */
int mobproc_eye_ranged_check(Mob *self, Char *enemy, Level *lvl) {
    Ballistica path;
    ballistica_cast(&path, lvl, self->base.pos, enemy->pos, 1, 0);
    for (int i = 1; i < path.distance; i++) {
        if (path.trace[i] == enemy->pos) return 1;
    }
    return 0;
}

int mobproc_scorpio_ranged_check(Mob *self, Char *enemy, Level *lvl) {
    if (cell_distance(self->base.pos, enemy->pos) <= 1) return 0;
    return ballistica_cast(NULL, lvl, self->base.pos, enemy->pos, 0, 1) == enemy->pos;
}

/* Shaman.zap(): Random.Int(2,12)  */
int mobproc_shaman_bolt(Mob *self, Char *enemy, Level *lvl) {
    Char *c = &self->base;

    int points[2] = { c->pos, enemy->pos };
    lightning_fx_start(points, 2);

    if (char_hit_roll(c, enemy, 1)) {
        int dmg = rng_range(2, 11);
        if (cell_liquid(lvl, enemy->pos) && !enemy->flying) {
            dmg = (int)(dmg * 1.5f);
        }
        char_damage(enemy, dmg, c, DMGSRC_LIGHTNING);

        for (int i = 0; i < 3; i++) particle_emit_spark(lvl, enemy->pos); 
    }
    /* miss: original floats the defense verb over the target -- presentation only */
    return 1;
}

/* Warlock.zap(): */
int mobproc_warlock_bolt(Mob *self, Char *enemy, Level *lvl) {
    Char *c = &self->base;

    if (char_hit_roll(c, enemy, 1)) {
        if (enemy->kind == CHAR_KIND_HERO && rng_int(2) == 0) {
            char_apply_buff(enemy, BUFF_WEAKNESS, WEAKNESS_DURATION);
        }

        int dmg = rng_range(12, 17);
        char_damage(enemy, dmg, c, DMGSRC_CHAR);

        wand_bolt_set_flying(ITEM_NONE);
        if (!missile_launch(self->base.pos, enemy->pos, ITEM_NONE, MISSILE_NO_SPIN, NULL, NULL)) {
            wand_bolt_clear();
        }
        for (int i = 0; i < 2; i++) 
            particle_emit_purple(lvl, enemy->pos);
    }
    return 1;
}

/* Eye.attack(): */
int mobproc_eye_gaze(Mob *self, Char *enemy, Level *lvl) {
    Char *c = &self->base;

    Ballistica path;
    int hit_cell = ballistica_cast(&path, lvl, c->pos, enemy->pos, 1, 0);
    ray_fx_start(c->pos, hit_cell);

    for (int i = 1; i < path.distance; i++) {
        Char *ch = char_find_at(path.trace[i]);
        if (!ch) continue;

        if (char_hit_roll(c, ch, 1)) {
            char_damage(ch, (int)rng_normal_range(14, 20), c, DMGSRC_CHAR);

            int bursts = rng_range(1, 2);
            for (int k = 0; k < bursts; k++) particle_emit_purple(lvl, path.trace[i]);
            /* original also: ch.sprite.flash(), and a death/game-over message if ch was the hero */
        }
        /* miss: original floats the defense verb over ch -- presentation only */
    }
    return 1;
}

/* Yog.BurningFist.attack(): */
int mobproc_burningfist_bolt(Mob *self, Char *enemy, Level *lvl) {
    (void)lvl;
    Char *c = &self->base;

    if (char_hit_roll(c, enemy, 1)) {
        char_damage(enemy, c->vt->damage_roll(c), c, DMGSRC_CHAR);
    }
    /* miss: original floats the defense verb over the target -- presentation only.
       NOT ported here: the rest of the Yog fight script (seeding fire in the fist's own 9
       neighbouring cells every turn, fist-count halving damage to Yog's body) -- that needs
       the arena/summon scripting the existing MOB_YOG comments already flag as separate work. */
    return 1;
}

int mobproc_scorpio_bolt(Mob *self, Char *enemy, Level *lvl) {
    (void)lvl;
    missile_launch(self->base.pos, enemy->pos, ITEM_DART, missile_spin_for(ITEM_DART), NULL, NULL);
    char_attack(&self->base, enemy);
    return 1;
}

int mobproc_tengu_bolt(Mob *self, Char *enemy, Level *lvl) {
    (void)lvl;
    missile_launch(self->base.pos, enemy->pos, ITEM_SHURIKEN, missile_spin_for(ITEM_SHURIKEN), NULL, NULL);
    char_attack(&self->base, enemy);
    return 1;
}