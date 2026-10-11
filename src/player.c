#include "player.h"
#include "npc.h"
#include "item_spawn.h"
#include "level_geometry.h"
#include <stddef.h>

/*═══════════════════════════════════════════════════════════════
    Animation
═══════════════════════════════════════════════════════════════*/
static int player_throw_in_flight(const Player *p) {
    return p->thrown.active && missile_is_flying();
}

static int player_is_busy(const Player *p) {
    if (!char_is_alive(&p->base))
        return 1;

    if (player_throw_in_flight(p))
        return 1;

    return char_is_busy(&p->base);
}
static int player_can_act(const Player *p) {
    if (!char_is_alive(&p->base))
        return 0;

    if (player_is_busy(p))
        return 0;

    if (actor_cooldown(&p->base.base) > 0.0f)
        return 0;

    return 1;
}
static void player_sync_anim_lock(Player *p) {
    switch (p->anim) {
    case ANIM_OPERATE:
    case ANIM_ATTACK:
    case ANIM_READ:
    case ANIM_DIE:
        p->base.anim_locked = !p->anim_done;
        break;
    default:
        p->base.anim_locked = 0;
    }
}
static void player_play_anim(Player *p, AnimState anim){
    if (p->anim == anim && !p->anim_done)
        return;

    p->anim = anim;
    p->frame = 0;
    p->frame_clock = 0.0f;
    p->anim_done = 0;
    player_sync_anim_lock(p);
}

void player_animate(Player *p, float dt) {
    const AnimClip *clip = &HERO_CLIPS[p->anim];
    if (p->anim_done || clip->fps <= 0) return;
 
    p->frame_clock += dt;
    float step = 1.0f / (float)clip->fps;
 
    while (p->frame_clock >= step) {
        p->frame_clock -= step;
        p->frame++;
        if (p->frame >= clip->count) {
            if (clip->loop) {
                p->frame = 0;
            } else {
                p->frame = clip->count - 1; 
                p->anim_done = 1;
                break;
            }
        }
    }
 
    if (p->anim_done) {
        switch (p->anim) {
            case ANIM_ATTACK:
                char_end_attack_anim(&p->base);
                player_play_anim(p, ANIM_IDLE);
                break;
            case ANIM_OPERATE:
            case ANIM_READ:
            case ANIM_ZAP:
                player_play_anim(p, ANIM_IDLE);
                break;
            default:
                break; /* ANIM_DIE holds its final frame on purpose */
        }
    }
    player_sync_anim_lock(p); /* unconditional: covers ANIM_DIE, which holds its last frame and never re-enters player_play_anim() above */
}
void player_set_moving(Player *p, int moving) {
    if (player_is_busy(p))
        return;

    if (moving) {
        if (p->anim != ANIM_RUN)
            player_play_anim(p, ANIM_RUN);
    } else {
        if (p->anim == ANIM_RUN)
            player_play_anim(p, ANIM_IDLE);
    }
}

/*═══════════════════════════════════════════════════════════════
    Hero stats — STR/leveling
═══════════════════════════════════════════════════════════════*/
int player_str(const Player *p) {
    return p->weakened ? p->STR - 2 : p->STR; /* -2 matches Hero.STR() */
}

int player_max_exp(const Player *p) {
    return 5 + p->lvl * 5; /* matches Hero.maxExp() */
}

static void player_update_awareness(Player *p) {
    double base = (p->cls == HERO_ROGUE) ? 0.85 : 0.90;
    int n = 1 + (p->lvl < 9 ? p->lvl : 9);
    p->awareness = (float)(1.0 - pow(base, n * 0.5));
}

static void player_earn_exp(Player *p, int exp) {
    p->exp += exp;
    while (p->exp >= player_max_exp(p)) {
        p->exp -= player_max_exp(p);
        p->lvl++;
        p->base.HT += 5;
        p->base.HP += 5;
        p->base_attack_skill++;
        p->base_defense_skill++;
        if (p->lvl < 10) player_update_awareness(p);
        /* original shows a level-up message/sound/badge check here */
    }
}
static void hero_receive_kill_xp(Char *killer, int xp) {
    if (killer->kind == CHAR_KIND_HERO) {
        player_earn_exp((Player *)killer, xp);
    }
}
/*═══════════════════════════════════════════════════════════════
    HERO COMBAT 
═══════════════════════════════════════════════════════════════*/
static ItemStack *hero_weapon(Player *p) {
    return p->ranged_attack ? &p->thrown.item : &p->weapon;
}

/* Weapon.acuracyFactor(hero) for any weapon stack */
static float hero_weapon_accuracy(const Player *p, ItemStack *wep) {
    //if(ITEM_DB[wep->type].cat != ITEMCAT_WEAPON) return 0;

    int encumbrance = weapon_str_req(wep) - player_str(p);
    if (ITEM_DB[wep->type].weapon.type == WEAPON_MISSILE) {
        if (p->cls == HERO_WARRIOR)       encumbrance += 3;
        else if (p->cls == HERO_HUNTRESS) encumbrance -= 2;
    }
    float acu = weapon_acu(wep);
    return (encumbrance > 0) ? (float)(acu / pow(1.5, encumbrance)) : acu;
    /* Imbue.ACCURACY (weapon-balancing scroll) isn't ported yet — no 1.5x term */
}

/* Weapon.speedFactor(hero) for any weapon stack */
static float hero_weapon_speed(const Player *p, ItemStack *wep) {
    int encumbrance = weapon_str_req(wep) - player_str(p);
    if (ITEM_DB[wep->type].weapon.type == WEAPON_MISSILE && p->cls == HERO_HUNTRESS) {
        encumbrance -= 2;
    }
    float dly = weapon_dly(wep);
    return (encumbrance > 0) ? (float)(dly * pow(1.2, encumbrance)) : dly;
    /* Imbue.SPEED (weapon-balancing scroll) isn't ported yet — no 0.6x term */
}

static int hero_attack_skill(Char *self, Char *target) {
    Player *p = (Player *)self;

    float accuracy = 1.0f; /* Ring of Accuracy bonus skipped */

    if (p->ranged_attack && cells_adjacent(self->pos, target->pos)) {
        accuracy *= 0.5f; /* melee penalty for a ranged weapon up close — fully portable, kept as-is */
    }

    ItemStack *wep = hero_weapon(p);
    float wep_factor = (wep->type != ITEM_NONE) ? hero_weapon_accuracy(p, wep) : 1.0f;

    return (int)(p->base_attack_skill * accuracy * wep_factor);
}

static int hero_defense_skill(Char *self, Char *enemy) {
    Player *p = (Player *)self;
    (void)enemy;

    float evasion = 1.0f; /* Ring of Evasion bonus skipped */

    if (self->paralysed) {
        evasion *= 0.5f;
    }

    /* Hero.defenseSkill()'s armor STR-encumbrance term. */
    int aEnc = (p->armor.type != ITEM_NONE) ? armor_str_req(&p->armor) - player_str(p) : 0;

    if (aEnc > 0) {
        return (int)(p->base_defense_skill * evasion / pow(1.5, aEnc));
    }

    if (p->cls == HERO_ROGUE) {
        /* FreeRunner sprint bonus */
        if (p->sub_class == HERO_SUB_FREERUNNER && !char_hunger_is_starving(self)) {
            evasion *= 2.0f;
        }
        return (int)((p->base_defense_skill - aEnc) * evasion);
    }

    return (int)(p->base_defense_skill * evasion);
}

static int hero_dr(Char *self) {
    Player *p = (Player *)self;

    int dr = armor_dr(&p->armor);

    Buff *barkskin = char_buff(self, BUFF_BARKSKIN);
    if (barkskin) dr += barkskin->data;

    return dr;
}

static int hero_damage_roll(Char *self) {
    Player *p = (Player *)self;
    int dmg;

    ItemStack *wep = hero_weapon(p);
    if (wep->type != ITEM_NONE) {
        int mn, mx;
        weapon_min_max(wep, &mn, &mx);
        dmg = rng_normal_range(mn, mx);  /* KindOfWeapon.damageRoll(): Random.NormalIntRange(min(),max()) */

        /* Weapon.damageRoll()'s exStr top-up: Huntress-with-ranged, or
           anyone-else-with-melee, rolls a uniform 0..exStr bonus. */
        if ((p->ranged_attack != 0) == (p->cls == HERO_HUNTRESS)) {
            int exStr = player_str(p) - weapon_str_req(wep);
            if (exStr > 0) {
                dmg += rng_range(0, exStr);
            }
        }
    } else {
        int str = player_str(p);
        dmg = (str > 10) ? rng_range(1, str - 9) : 1;
    }

    return char_buff(self, BUFF_FURY) ? (int)(dmg * 1.5f) : dmg;
}

static float hero_attack_delay(Player *p) {
    ItemStack *wep = hero_weapon(p);
    return (wep->type == ITEM_NONE) ? 1.0f : hero_weapon_speed(p, wep);
}

static float hero_speed(Char *self) {
    Player *p = (Player *)self;
    float base = CHAR_DEFAULT_VTABLE.speed(self); /* super.speed(): Char's own baseSpeed × Cripple */

    int aEnc = (p->armor.type != ITEM_NONE) ? armor_str_req(&p->armor) - player_str(p) : 0;

    if (aEnc > 0) {
        return (float)(base * pow(1.3, -aEnc));
    }

    /*TODO FreeRunner's 1.6x sprint bonus */
    return base;
}

static void hero_wear_item(ItemStack *it) {
    switch (item_use(it)) {
        case ITEM_USE_WARN_BREAKING: /* TODO "going to break soon" (if it->level_known) */ break;
        case ITEM_USE_BROKE:         /* TODO "has broken" + effect (if it->level_known)  */ break;
        default: break;
    }
}

static int hero_attack_proc(Char *self, Char *enemy, int damage) {
    Player *p = (Player *)self;

    /* Hero.attackProc(): wep = rangedWeapon != null ? rangedWeapon : belongings.weapon;
       wep.proc() -> Weapon.proc() -> use().  char_attack() only gets here on a
       landed hit, so misses don't wear the weapon — same as the original.
       For a ranged attack the weapon is the thrown stack, and the missile
       subclasses add their own proc() on top (Javelin's cripple, Curare's
       paralysis, ...). */
    ItemStack *wep = hero_weapon(p);
    if (wep->type != ITEM_NONE) {
        hero_wear_item(wep);
        if (p->ranged_attack) {
            missile_weapon_proc(wep->type, enemy, damage);
        }
    }

    /* Placeholder — Weapon.proc()'s enchantment + hitsToKnow auto-identify
       still isn't ported. The subclass switch below covers Gladiator and
       Sniper; Battlemage's wand-charging branch needs the Wand item type
       and stays a placeholder too. */
    if (p->sub_class == HERO_SUB_GLADIATOR && !p->ranged_attack && p->weapon.type != ITEM_NONE) {
        damage += char_combo_hit(self, enemy, damage);
    } else if (p->sub_class == HERO_SUB_SNIPER && p->ranged_attack) {
        Buff *mark = char_apply_buff(self, BUFF_SNIPERS_MARK, hero_attack_delay(p) * 1.1f);
        if (mark) mark->data = actor_id(&enemy->base);
    }

    return damage;
}

/* Char.attack(): `dr = hero && rangedWeapon != null && subClass == SNIPER ? 0 : IntRange(0, enemy.dr())` */
/*static int hero_ignores_dr(Char *self) {
    Player *p = (Player *)self;
    return p->ranged_attack && p->sub_class == HERO_SUB_SNIPER;
}*/

static int hero_defense_proc(Char *self, Char *attacker, int damage) {
    Player *p = (Player *)self;
    (void)attacker;

    /* Placeholder — the original first checks Ring of Thorns (reflects a
       cut of damage back) and an Earthroot-armor buff (flat absorb). */

    /* belongings.armor.proc() -> Armor.proc() -> use().  Runs on EVERY hit that
       lands, including one fully soaked up by dr (damage == 0) — which is why
       this can't live in on_damaged behind a dmg_taken > 0 check. Armor.proc()'s
       glyph + hitsToKnow auto-identify go here too once ported. */
    if (p->armor.type != ITEM_NONE) {
        hero_wear_item(&p->armor);
    }
    return damage;
}

static void hero_on_damaged(Char *self, int dmg_taken, void *src, DamageSource src_type) {
    (void)dmg_taken; (void)src; (void)src_type;
    Player *p = (Player *)self;

    p->resting = 0;

    /* Berserker's Fury: original triggers at HP <= HT * Fury.LEVEL */
    if (p->sub_class == HERO_SUB_BERSERKER && char_is_alive(self)
        && self->HP <= self->HT / 2) {
        char_apply_buff(self, BUFF_FURY, -1.0f);
    }
}

static int hero_buff_immune(Char *self, BuffType type) {
    if ((type == BUFF_PARALYSIS || type == BUFF_VERTIGO) && char_buff(self, BUFF_GASES_IMMUNITY)) {
        return 1;
    }
    return 0;
}

static float hero_hunger_tick_scale(Char *self) {
    Player *p = (Player *)self;
    return (p->cls == HERO_ROGUE) ? 1.2f : 1.0f; /* Hunger.act()'s Rogue-specific STEP*1.2 */
}

static void hero_die(Char *self, void *src, DamageSource src_type) {
    Player *p = (Player *)self;

    player_play_anim(p, ANIM_DIE);

    char_die_default(self, src, src_type);
    /* TODO ADD GAMEOVER SCENE HERE
       + BLOCK MOVEMENT */
}

static void hero_on_buff_added(Char *self, Buff *buff) {
    Player *p = (Player *)self;

    switch (buff->type) {
        case BUFF_BURNING:
        case BUFF_PARALYSIS:
        case BUFF_POISON:
        case BUFF_VERTIGO:
            p->resting = 0;
            break;
        case BUFF_WEAKNESS:
            p->weakened = 1;
            break;
        default:
            break;
    }
    /*TODO LOG + HUD ICONS ADDING */
}

static void hero_on_buff_removed(Char *self, Buff *buff) {
    Player *p = (Player *)self;

    if (buff->type == BUFF_WEAKNESS) {
        p->weakened = 0;
    }
    /*TODO HUD ICONS REMOVING */
}

static const CharVTable *hero_vtable(void) {
    static CharVTable vt;
    static int built = 0;
    if (!built) {
        vt = CHAR_DEFAULT_VTABLE;
        vt.attack_skill  = hero_attack_skill;
        vt.defense_skill = hero_defense_skill;
        vt.dr            = hero_dr;
        vt.damage_roll   = hero_damage_roll;
        vt.speed         = hero_speed;
        vt.attack_proc   = hero_attack_proc;
        vt.defense_proc  = hero_defense_proc;
        vt.on_damaged    = hero_on_damaged;
        vt.die           = hero_die;
        vt.on_buff_added = hero_on_buff_added;
        vt.on_buff_removed = hero_on_buff_removed;
        vt.buff_immune   = hero_buff_immune;
        vt.hunger_tick_scale = hero_hunger_tick_scale;

        built = 1;
    }
    return &vt;
}

/*═══════════════════════════════════════════════════════════════
    Bags
═══════════════════════════════════════════════════════════════*/
int player_has_bag(const Player *p, BagType type) {
    if (type == BAG_BACKPACK) return 1; 
    if (type < 0 || type >= BAG_TYPE_COUNT) return 0;

    return bag_find_item(&p->bags[BAG_BACKPACK], bag_item_type(type)) >= 0;
}

/*Bag *player_get_bag(Player *p, BagType type) { //TODO CANT REMEMBER WHY IT WAS WRITTEN 
    return player_has_bag(p, type) ? &p->bags[type] : NULL;
}*/

static Bag *player_bag_for_item(Player *p, ItemType item_type) {
    for (int t = BAG_KEYRING; t < BAG_TYPE_COUNT; t++) {
        if (player_has_bag(p, (BagType)t) && bag_grabs((BagType)t, item_type)) {
            return &p->bags[t];
        }
    }
    return &p->bags[BAG_BACKPACK];
}

int player_locate_item(Player *p, ItemType type, Bag **out_bag, int *out_slot) {
    for (int t = 0; t < BAG_TYPE_COUNT; t++) {
        if (!player_has_bag(p, (BagType)t)) continue;

        int slot = bag_find_item(&p->bags[t], type);
        if (slot >= 0) {
            if (out_bag)  *out_bag  = &p->bags[t];
            if (out_slot) *out_slot = slot;
            return 1;
        }
    }
    return 0;
}

int player_has_item(const Player *p, ItemType type) {
    for (int t = 0; t < BAG_TYPE_COUNT; t++) {
        if (!player_has_bag(p, (BagType)t))
            continue;

        if (bag_find_item(&p->bags[t], type) >= 0)
            return 1;
    }

    return 0;
}

int player_drop_from_bag(Player *p, Level *lvl, Bag *bag, int slot) {
    if (player_is_busy(p))
        return 0;
    if (!bag || slot < 0 || slot >= bag->count)
        return 0;
    
    Heap *h = level_heap_at(lvl, p->base.pos);
    if (!h)
        h = level_create_heap(lvl, p->base.pos, HEAP_NORMAL);

    if (!h)
        return 0;

    ItemStack stack = bag->items[slot];
    if (!heap_add_stack(h, stack))
        return 0;

    char_spend(&p->base, 0.5f); /* Item.TIME_TO_DROP */
    actor_next(&p->base.base);
    return bag_remove_item(bag, slot, stack.quantity);
}

/*═══════════════════════════════════════════════════════════════
    Inventory helpers 
═══════════════════════════════════════════════════════════════*/
ItemStack *equip_slot_ptr(Player *p, EquipSlot slot) {
    switch (slot) {
        case EQUIP_WEAPON: return &p->weapon;
        case EQUIP_ARMOR:  return &p->armor;
        case EQUIP_RING1:  return &p->ring1;
        case EQUIP_RING2:  return &p->ring2;
        default:           return NULL;
    }
}

static int player_use_key(Player *p, ItemType key) {
    Bag *bag = NULL;
    int slot = -1;
    if (!player_locate_item(p, key, &bag, &slot))
        return 0;
    return bag_remove_item(bag, slot, 1);
}

static void player_acquire_bag(Player *p, BagType type) {
    if (type <= BAG_BACKPACK || type >= BAG_TYPE_COUNT) return;

    Bag *bag = &p->bags[type];
    if (bag->capacity == 0) {
        bag_init(bag, type);
    }

    Bag *backpack = &p->bags[BAG_BACKPACK];
    int i = 0;
    while (i < backpack->count) {
        if (bag_add_stack(bag, backpack->items[i])) {
            bag_remove_item(backpack, i, backpack->items[i].quantity);
        } else {
            i++;
        }
    }
}
/*═══════════════════════════════════════════════════════════════
    Inventory management
═══════════════════════════════════════════════════════════════*/
void player_sort_inventory(Player *p) {
    for (int t = 0; t < BAG_TYPE_COUNT; t++)
        bag_sort(&p->bags[t]);
}

void player_consume_item(Player *p, Bag *bag, int slot) {
    if (item_consume(&bag->items[slot]) == ITEM_USE_CONSUMED) {
        //TODO ADD USING CONSUMABLES
        //MB ADD MORE STATES + USE SWITCH

        /* TEST: random buff with random duration */
        static const BuffType test_buffs[] = {
            BUFF_INVISIBILITY,
            BUFF_SHADOWS,
            BUFF_LIGHT,
            BUFF_FURY,
            BUFF_HASTE,
            BUFF_RAGE,
        };

        int buff_count = sizeof(test_buffs) / sizeof(test_buffs[0]);

        BuffType type = test_buffs[rng_int(buff_count)];
        float duration = (float)rng_range(20, 60);

        char_apply_buff(&p->base, type, duration);
        
        char_spend(&p->base, 1.0f);
        actor_next(&p->base.base);
        /* TEST: random buff with random duration */

        bag_remove_item(bag, slot, 1);
    }
}

static int player_add_stack(Player *p, ItemStack stack) {
    if (stack.type == ITEM_GOLD) {
        p->gold += stack.quantity;
        return 1;
    }

    Bag *bag = player_bag_for_item(p, stack.type);
    if (!bag_add_stack(bag, stack)) return 0;

    int granted = bag_type_for_item(stack.type);
    if (granted >= 0) {
        player_acquire_bag(p, (BagType)granted);
    }

    player_sort_inventory(p);
    return 1;
}

static int player_init_item(Player *p, ItemType type, int quantity) {
    ItemStack stack;
    item_stack_init_start(&stack, type, quantity);
    return player_add_stack(p, stack);
}

/*═══════════════════════════════════════════════════════════════
    Equipment
═══════════════════════════════════════════════════════════════*/
static float equip_time(Player *p, const ItemStack *item) {
    if (ITEM_DB[item->type].cat == ITEMCAT_ARMOR) {
        return 2.0f / char_speed(&p->base);
    }
    return 1.0f;
}

int player_equip_item(Player *p, Bag *bag, int slot, int spend_time) {
    if (player_is_busy(p))
        return EQUIP_FAILED;
    if (!bag || slot < 0 || slot >= bag->count)
        return EQUIP_FAILED;

    ItemStack incoming = bag->items[slot];
    ItemCategory cat = ITEM_DB[incoming.type].cat;

    /* Rings are handled separately because the player must choose a slot. */
    if (cat == ITEMCAT_RING)
        return EQUIP_RING_SELECT;

    EquipSlot target;

    if (cat == ITEMCAT_WEAPON && ITEM_DB[incoming.type].weapon.type == WEAPON_MELEE) {
        target = EQUIP_WEAPON;
    } else if (cat == ITEMCAT_ARMOR) {
        target = EQUIP_ARMOR;
    } else {
        return EQUIP_FAILED;
    }

    ItemStack *dst = equip_slot_ptr(p, target);

    if (dst->type != ITEM_NONE && dst->cursed)
        return EQUIP_FAILED;

    if (!bag_remove_item(bag, slot, 1))
        return EQUIP_FAILED;

    if (dst->type != ITEM_NONE) {
        if (!player_add_stack(p, *dst)) {
            player_add_stack(p, incoming);
            return EQUIP_FAILED;
        }
    }
    *dst = incoming;
    dst->quantity = 1;
    dst->cursed_known = 1;

    if(spend_time) {
        char_spend(&p->base, equip_time(p, dst));
        actor_next(&p->base.base);
    }
    player_sort_inventory(p);
    return EQUIP_OK;
}

int player_equip_ring(Player *p, Bag *bag, int slot, EquipSlot target, int spend_time) {
    if (player_is_busy(p))
        return EQUIP_FAILED;

    if (!bag || slot < 0 || slot >= bag->count)
        return EQUIP_FAILED;

    if (target != EQUIP_RING1 && target != EQUIP_RING2)
        return EQUIP_FAILED;

    ItemStack *dst = equip_slot_ptr(p, target);
    if (!dst)
        return EQUIP_FAILED;

    if (dst->type != ITEM_NONE && dst->cursed)
        return EQUIP_FAILED;

    ItemStack incoming = bag->items[slot];

    if (!bag_remove_item(bag, slot, 1))
        return EQUIP_FAILED;

    if (dst->type != ITEM_NONE) {
        if (!player_add_stack(p, *dst)) {
            player_add_stack(p, incoming);
            return 0;
        }
    }

    *dst = incoming;
    dst->quantity = 1;
    dst->cursed_known = 1;

    if(spend_time) {
        char_spend(&p->base, equip_time(p, dst));
        actor_next(&p->base.base);
    }
    player_sort_inventory(p);
    return 1;
}
int player_unequip(Player *p, EquipSlot slot, Level *lvl) {
    if (player_is_busy(p))
        return 0;
    ItemStack *src = equip_slot_ptr(p, slot);
    if (!src || src->type == ITEM_NONE) return 0;

    if (src->cursed) return 0;

    ItemStack stack = *src;
    stack.quantity = 1;

    if (player_add_stack(p, stack)) {
        *src = item_stack_none();
        char_spend(&p->base, equip_time(p, &stack));
        actor_next(&p->base.base);
        player_sort_inventory(p);
        return 1;
    }

    if (lvl) {
        Heap *h = level_heap_at(lvl, p->base.pos);

        if (h && heap_add_stack(h, stack)) {
            *src = item_stack_none();
            char_spend(&p->base, equip_time(p, &stack));
            actor_next(&p->base.base);
            player_sort_inventory(p);
            return 1;
        }
    }

    return 0; 
}

static void equip_from_bag(Player *p, ItemType type) { //helpers for starting equips
    Bag *bag; int slot;
    if (player_locate_item(p, type, &bag, &slot)) {
        player_equip_item(p, bag, slot, 0);
    }
}
static void equip_ring_from_bag(Player *p, ItemType type, EquipSlot target) {
    Bag *bag; int slot;
    if (player_locate_item(p, type, &bag, &slot)) {
        player_equip_ring(p, bag, slot, target, 0);
    }
}

/*═══════════════════════════════════════════════════════════════
    Throwing
═══════════════════════════════════════════════════════════════*/
static int player_shoot(Player *p, Char *enemy) {
    p->ranged_attack = 1;
    CharAttackResult r = char_attack(&p->base, enemy);
    p->ranged_attack = 0;
    return r.hit;
}

static void player_land_item(Player *p, Level *lvl, ItemStack item, int cell) {
    Heap *heap = level_heap_at(lvl, cell);

    if (!heap) {
        if (cell_pit(lvl, cell)) {
            /* Level.drop(): `map[cell] == CHASM || pit[cell]` -> Dungeon.dropToChasm(item) */
            return;
        }
        heap = level_create_heap(lvl, cell, HEAP_NORMAL);

        if (!heap) return;
    }

    heap_add_stack(heap, item);
    return;
}

static void player_circle_back(Player *p, Level *lvl, int from) {

    missile_launch(from, p->base.pos, p->thrown.item.type, missile_spin_for(p->thrown.item.type), NULL, NULL);

    if (!player_add_stack(p, p->thrown.item)) {
        player_land_item(p, lvl, p->thrown.item, p->base.pos);
    }
}

/* Item.onThrow() / MissileWeapon.onThrow() and the per-missile overrides. */
static void player_throw_resolve(Player *p, Level *lvl) {
    const int cell = p->thrown.cell;
    const ItemStack item = p->thrown.item;
    const MissileWeaponDef *mwd = missile_def(item.type);

    if (!mwd) {
        player_land_item(p, lvl, item, cell);   /* TODO: Potion/Seed/Bomb... override onThrow() */
        return;
    }

    Char *enemy = char_find_at(cell);
    int nobody = (enemy == NULL || enemy == &p->base);

    if (nobody) {
        if ((mwd->flags & MISSILE_F_IGNITES) && cell_flamable(lvl, cell)) {
            /* IncendiaryDart.onThrow(): used up, lights the ground */
        } else {
            player_land_item(p, lvl, item, cell);
        }
        return;
    }

    int hit = player_shoot(p, enemy);

    if (mwd->flags & MISSILE_F_RETURNS) {
        player_circle_back(p, lvl, cell);
    } else if (!hit) {
        player_land_item(p, lvl, p->thrown.item, cell);   /* MissileWeapon.miss() */
    }
}

/* The callback of Item.cast()'s MissileSprite. */
static void player_throw_land(void *ctx) {
    Player *p = (Player *)ctx;
    Level *lvl = char_active_level();

    if (lvl) player_throw_resolve(p, lvl);

    p->thrown.active  = 0;
    p->thrown.item    = item_stack_none();

    char_spend(&p->base, p->thrown.delay);      /* user.spendAndNext(finalDelay) */
    actor_next(&p->base.base);
}


int player_throw(Player *p, Level *lvl, Bag *bag, int slot, int dst) {
    if (!lvl || !bag || slot < 0 || slot >= bag->count)
        return 0;
    if (!player_can_act(p))
        return 0;
    if (!in_bounds(dst))
        return 0;

    ItemStack item = bag->items[slot];
    if ((unsigned)item.type >= (unsigned)ITEM_TYPE_COUNT)   /* ITEM_NONE or garbage */
        return 0;
    item.quantity = 1;

    if (!bag_remove_item(bag, slot, 1))
        return 0;

    int cell = ballistica_cast(NULL, lvl, p->base.pos, dst, 0, 1);

    Point self = idx_to_xy(p->base.pos);
    Point target = idx_to_xy(cell);
    if (target.x != self.x)
        p->facing = (target.x < self.x) ? FACE_LEFT : FACE_RIGHT;
    player_play_anim(p, ANIM_ZAP);

    Char *enemy = char_find_at(cell);
    float delay = 1.0f;
    
    if (missile_def(item.type)) {
        delay *= hero_weapon_speed(p, &item);
        if (enemy && char_consume_snipers_mark(&p->base, enemy))
            delay *= 0.5f;
    }

    p->thrown.active = 1;
    p->thrown.item   = item;
    p->thrown.cell   = cell;
    p->thrown.delay  = delay;

    uint8_t col, row;
    item_display_sprite(item.type, &col, &row);
    
    if (!missile_launch(p->base.pos, cell, item.type, missile_spin_for(item.type), player_throw_land, p)) {
        player_throw_land(p);
    }
    return 1;
}

int player_set_quick_slot(Player *p, Bag *bag, int slot) {
    if (!bag || slot < 0 || slot >= bag->count)
        return 0;

    ItemType type = bag->items[slot].type;

    if (type == ITEM_NONE)
        return 0;

    ItemCategory cat = ITEM_DB[type].cat;

    switch (cat) {
    case ITEMCAT_POTION:
    case ITEMCAT_SCROLL:
    case ITEMCAT_SEED:
    case ITEMCAT_FOOD:
    case ITEMCAT_WAND:
        p->quick_slot.type = type;
        return 1;
    case ITEMCAT_WEAPON:
        if (ITEM_DB[type].weapon.type == WEAPON_MISSILE) {
            p->quick_slot.type = type;
            return 1;
        }
        return 0;

    default:
        return 0;
    }
}

static int player_quick_target(const Player *p) { //REDO temporary 
    if (char_is_alive(p->attack_target))
        return p->attack_target->pos;
    else
        return p->target_pos;
}

int player_use_quick_slot(Player *p) {
    if (player_is_busy(p))
        return 0;

    ItemType type = p->quick_slot.type;

    if (type == ITEM_NONE)
        return 0;

    if (type < 0 || type >= ITEM_TYPE_COUNT)
        return 0;

    Bag *bag = NULL;
    int slot = -1;

    if (!player_locate_item(p, type, &bag, &slot))
        return 0;

    ItemCategory cat = ITEM_DB[type].cat;

    switch (cat) {
    case ITEMCAT_POTION:
    case ITEMCAT_SCROLL:
    case ITEMCAT_SEED:
    case ITEMCAT_FOOD:
        player_consume_item(p, bag, slot);
        return 0;

    case ITEMCAT_WEAPON:
        if (ITEM_DB[type].weapon.type == WEAPON_MISSILE) {
            int dst = player_quick_target(p);
            if (dst != p->base.pos)
                player_throw(p, char_active_level(), bag, slot, dst);
            return 0;
        }
        return 0;

    case ITEMCAT_WAND:
        /* TODO: ZAP */
        return 0;

    default:
        return 0;
    }
}

int player_throw_quick_slot(Player *p, Level *lvl) {
    if (player_is_busy(p))
        return 0;

    ItemType type = p->quick_slot.type;

    if (type == ITEM_NONE)
        return 0;

    if (type < 0 || type >= ITEM_TYPE_COUNT)
        return 0;

    Bag *bag = NULL;
    int slot = -1;

    if (!player_locate_item(p, type, &bag, &slot))
        return 0;

    player_throw(p, lvl, bag, slot, p->target_pos);
    return 0;
}

/*═══════════════════════════════════════════════════════════════
    World interaction
═══════════════════════════════════════════════════════════════*/
int fov_radius(Player *player, int depth) {
    int radius;

    if (depth > 9 && depth < 15) {
        radius = 6;
    } else if (depth > 20 && depth < 24) {
        radius = 24 - depth;
        if (radius < 1) radius = 1;
    } else if (depth == 24) {
        radius = 3;
    } else {
        radius = 8;
    }

    if (char_buff(&player->base, BUFF_LIGHT)) {
        if (radius < LIGHT_DISTANCE)
            radius = LIGHT_DISTANCE;
    }

    return radius;
}

int player_pickup(Player *p, Level *lvl) {
    if (player_is_busy(p))
        return 0;

    Heap *h = level_heap_at(lvl, p->base.pos);
    if (!h) return 0;

    if ((h->type == HEAP_LOCKED_CHEST || h->type == HEAP_CRYSTAL_CHEST)&& !player_use_key(p, ITEM_KEY_GOLDEN))
        return 0;
    
    if (h->type != HEAP_NORMAL && h->type != HEAP_FOR_SALE) {
        h->type = HEAP_NORMAL;
        return 0;
    }
    if (h->count == 0) return 0;

    ItemStack top = h->items[0];
    if (!player_add_stack(p, top)) {
        return 0;
    }

    for (int i = 1; i < h->count; i++) {
        h->items[i - 1] = h->items[i];
    }
    h->count--;
    if (h->count == 0) {
        *h = lvl->heaps[--lvl->num_heaps];
    }

    char_spend(&p->base, 1.0f); /* Item.TIME_TO_PICK_UP */
    actor_next(&p->base.base);
    return 1;
}

int player_operate(Player *p, Level *lvl, int idx) {
    if (player_is_busy(p))
        return 0;
    
    Heap *h = level_heap_at(lvl, idx);
    if (!h)
        return 0;
    if (h->type == HEAP_NORMAL || h->type == HEAP_FOR_SALE)
        return 0;
    if ((h->type == HEAP_LOCKED_CHEST || h->type == HEAP_CRYSTAL_CHEST)&& !player_use_key(p, ITEM_KEY_GOLDEN))
        return 0;
    player_play_anim(p, ANIM_OPERATE);
    h->type = HEAP_NORMAL;
    char_spend(&p->base, 1.0f);
    actor_next(&p->base.base);
    return 1;
}
static int player_interact_tile(Player *p, Level *lvl, int idx) {
    uint8_t tile = lvl->map[idx];

    switch (tile) {
    case TILE_LOCKED_DOOR:
        if (!player_use_key(p, ITEM_KEY_IRON))
            return 1;

        lvl->map[idx] = TILE_OPEN_DOOR;
        char_spend(&p->base, 1.0f);
        actor_next(&p->base.base);
        player_play_anim(p, ANIM_OPERATE);
        return 1;

    case TILE_LOCKED_EXIT:
        //if (!player_use_key(p, ITEM_KEY_SKELETON)) return 0; /*TODO: REMOVE COMMENT AFTER ADDING BOSSES WITH KEY DROPS */
        lvl->map[idx] = TILE_UNLOCKED_EXIT;
        char_spend(&p->base, 1.0f);
        actor_next(&p->base.base);
        player_play_anim(p, ANIM_OPERATE);
        return 1;

    case TILE_HIGH_GRASS:
        lvl->map[idx] = TILE_GRASS;
        //TODO SPAWN SEEDS + ITEM_DEWDROP 
        //IS IT HAS ANIMATION???
        return 0;
    }

    return 0;
}

void player_rest(Player *p) {
    if (!p->resting) {
        char_spend(&p->base, 1.0f);
        actor_next(&p->base.base);
        return;
    }
    if (p->base.HP >= p->base.HT || char_hunger_is_starving(&p->base)) {
        p->resting = 0;
        return;
    }

    char_spend(&p->base, 1.0f);
    actor_next(&p->base.base);

    /* TODO: the original also breaks resting after mob comes into fov (Hero.checkVisibleMobs())*/
}

void player_search(Player *p, Level *lvl) { //test for search
    if (player_is_busy(p))
        return;

    for(int i = 0; i < 8; i++) {
        int idx = neighbour_idx(p->base.pos, DX[i], DY[i]);

        if (lvl->map[idx] == TILE_SECRET_DOOR) //redo by flags
            lvl->map[idx] = TILE_DOOR;
    }
    char_spend(&p->base, 1.0f);
    actor_next(&p->base.base);
}

int player_move(Player *p, Level *lvl, int dx, int dy) {
    if (p->base.rooted) 
        return 0;
    if (player_is_busy(p))
        return 0;
    if (dx == 0 && dy == 0)
        return 0;

    int idx = neighbour_idx(p->base.pos, dx, dy);
    if (idx == -1)
        return 0;
    if (!(cell_passable(lvl, idx)))
        return 0;

    if (dx != 0) p->facing = (dx < 0) ? FACE_LEFT : FACE_RIGHT;

    char_move(&p->base, lvl, idx);
    p->cursor_pos = p->base.pos;

    char_spend(&p->base, 1.0f / char_speed(&p->base));
    actor_next(&p->base.base); 

    return 1;
}

static int player_start_attack(Player *p, Char *target) {
    if (player_is_busy(p)) return 0;
    if (!char_is_alive(&p->base) || !char_is_alive(target)) return 0;
    if (char_is_charmed_by(&p->base, target)) return 0;

    Point self = idx_to_xy(p->base.pos);
    Point enemy = idx_to_xy(target->pos);

    if (enemy.x != self.x)
        p->facing = (enemy.x < self.x) ? FACE_LEFT : FACE_RIGHT;

    p->attack_target = target;
    char_spend(&p->base, hero_attack_delay(p));
    player_play_anim(p, ANIM_ATTACK);
    char_begin_attack_anim(&p->base, target);

    char_attack(&p->base, target);
    actor_next(&p->base.base);

    return 1;
}

int player_bump(Player *p, Level *lvl, int dx, int dy) {
    if (!player_can_act(p))
        return PLAYER_BUMP_BLOCKED;

    int idx = neighbour_idx(p->base.pos, dx, dy);
    if (idx == -1) return PLAYER_BUMP_BLOCKED;

    if (dx != 0) p->facing = (dx < 0) ? FACE_LEFT : FACE_RIGHT;

    /* Tile */
    if (player_interact_tile(p, lvl, idx))
        return PLAYER_BUMP_INTERACTED; //todo change to other type. mb it should live after occupant check???

    /* Character */
    Char *occupant = char_find_at(idx);
    if (occupant && occupant->kind == CHAR_KIND_MOB) {
        return player_start_attack(p, occupant) ? PLAYER_BUMP_ATTACKED : PLAYER_BUMP_BLOCKED;
    }
    if (occupant && occupant->kind == CHAR_KIND_NPC) {
        npc_interact((NPC *)occupant, &p->base);
        return PLAYER_BUMP_INTERACTED;
    }
    if (occupant) {
        return PLAYER_BUMP_BLOCKED;
    }

    /* Heap */
    Heap *h = level_heap_at(lvl, idx);
    if (h && player_operate(p, lvl, idx))
        return PLAYER_BUMP_INTERACTED; //todo change to other type. mb it should live after occupant check???

    /* Movement */
    return player_move(p, lvl, dx, dy) ? PLAYER_BUMP_MOVED : PLAYER_BUMP_BLOCKED;
}

int player_auto_attack(Player *p) {
    Char *target = p->attack_target;

    if (!target || !char_is_alive(target)) {
        p->attack_target = NULL;
        return 0;
    }

    if (!cells_adjacent(p->base.pos, target->pos))
        return 0;

    return player_start_attack(p, target);
}

/*═══════════════════════════════════════════════════════════════
    Initialization
═══════════════════════════════════════════════════════════════*/
void player_init(Player *p, const Level *lvl, HeroClass cls){
    char_init(&p->base, hero_vtable(), CHAR_KIND_HERO, lvl->entrance, HERO_STARTING_HP);
    p->base.name = "you";
    char_add(&p->base);

    char_apply_buff(&p->base, BUFF_REGENERATION, -1.0f);
    char_apply_buff(&p->base, BUFF_HUNGER, 0.0f);

    p->cls         = cls;
    p->sub_class   = HERO_SUB_NONE;

    p->facing      = FACE_RIGHT;
    p->anim        = ANIM_IDLE;
    p->frame       = 0;
    p->frame_clock = 0.0f;
    p->anim_done   = 0;

    p->gold = 0;

    for (int t = 0; t < BAG_TYPE_COUNT; t++) {
        p->bags[t].type = (BagType)t;
        bag_clear(&p->bags[t]);
    }
    bag_init(&p->bags[BAG_BACKPACK], BAG_BACKPACK);

    p->weapon = item_stack_none();
    p->armor  = item_stack_none();
    p->ring1  = item_stack_none();
    p->ring2  = item_stack_none();

    p->quick_slot.type = ITEM_NONE;


    p->STR                = HERO_STARTING_STR;
    p->weakened            = 0;
    p->lvl                 = 1;
    p->exp                 = 0;
    p->awareness           = HERO_STARTING_AWARENESS;
    p->base_attack_skill   = HERO_STARTING_ATTACK;
    p->base_defense_skill  = HERO_STARTING_DEFENSE;

    p->ranged_attack       = 0;
    memset(&p->thrown, 0, sizeof(p->thrown));
    p->thrown.item         = item_stack_none();

    p->resting             = 0;

    p->cursor_pos = p->base.pos;
    p->target_pos = p->base.pos;

    p->attack_target       = NULL;

    player_init_item(p, ITEM_CLOTH_ARMOR, 1);
    equip_from_bag(p, ITEM_CLOTH_ARMOR);
    player_init_item(p, ITEM_RATION, 1);
    player_init_item(p, ITEM_BAG_KEYRING, 1);

    player_init_item(p, ITEM_WARRIOR_ARMOR, 1);//TEST
    player_init_item(p, ITEM_WAR_HAMMER, 1);//TEST
    player_init_item(p, ITEM_SCROLL_UPGRADE, 100);//TEST

    switch (p->cls) {
    case HERO_WARRIOR:
        p->STR += 1;                             /* Warriors start with 11 STR */
        player_init_item(p, ITEM_SHORT_SWORD, 1);
        equip_from_bag(p, ITEM_SHORT_SWORD);
        player_init_item(p, ITEM_DART, 8);
        break;
    case HERO_MAGE:
        player_init_item(p, ITEM_KNUCKLES, 1);
        equip_from_bag(p, ITEM_KNUCKLES);
        player_init_item(p, ITEM_WAND_MAGIC_MISSILE, 1);
        break;
    case HERO_ROGUE:
        player_init_item(p, ITEM_DAGGER, 1);
        equip_from_bag(p, ITEM_DAGGER);
        player_init_item(p, ITEM_DART, 8);
        player_init_item(p, ITEM_RING_SHADOWS, 1);
        equip_ring_from_bag(p, ITEM_RING_SHADOWS, EQUIP_RING1);
        break;
    case HERO_HUNTRESS:
        p->base.HT -= 5;                         /* Huntresses start with 15 HP */
        p->base.HP  = p->base.HT;
        player_init_item(p, ITEM_DAGGER, 1);
        equip_from_bag(p, ITEM_DAGGER);
        player_init_item(p, ITEM_BOOMERANG, 1);
        break;
    default:
        break;
    }

    mob_set_hero(&p->base);
    mob_set_kill_reward_hook(hero_receive_kill_xp);
}