#include "mob_procs.h"
#include "level_geometry.h"

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