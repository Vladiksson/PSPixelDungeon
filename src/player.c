#include "player.h"
#include "npc.h"
#include "item_spawn.h"
#include "rng.h"
#include <math.h>
#include <stddef.h>

/*═══════════════════════════════════════════════════════════════
    HERO COMBAT 
═══════════════════════════════════════════════════════════════*/
static int hero_attack_skill(Char *self, Char *target) {
    Player *p = (Player *)self;

    float accuracy = 1.0f; /* Ring of Accuracy bonus skipped */

    if (p->ranged_attack && char_distance(self, target) == 1) {
        accuracy *= 0.5f; /* melee penalty for a ranged weapon up close — fully portable, kept as-is */
    }

    /* Placeholder for wep.acuracyFactor(this) — scales off enchant
       level instead of a real per-weapon constant. */
    float wep_factor = (p->weapon.type != ITEM_NONE)
        ? 1.0f + 0.1f * item_effective_level(&p->weapon)
        : 1.0f;

    return (int)(p->base_attack_skill * accuracy * wep_factor);
}

static int hero_defense_skill(Char *self, Char *enemy) {
    Player *p = (Player *)self;
    (void)enemy;

    float evasion = 1.0f; /* Ring of Evasion bonus skipped */

    if (self->paralysed) {
        evasion *= 0.5f;
    }

    /* Armor STR-encumbrance penalty skipped */
    if (p->sub_class == HERO_SUB_FREERUNNER && player_is_busy(p)) {
        evasion *= 2.0f;
    }

    return (int)(p->base_defense_skill * evasion);
}

static int hero_dr(Char *self) {
    Player *p = (Player *)self;

    /* Placeholder for armor.DR() */
    if (p->armor.type == ITEM_NONE) return 0;
    int dr = 2 + item_effective_level(&p->armor) * 2;
    return dr < 0 ? 0 : dr;
}

static int hero_damage_roll(Char *self) {
    Player *p = (Player *)self;
    int dmg;

    if (p->weapon.type != ITEM_NONE) {
        /* Placeholder for wep.damageRoll(this) */
        int top = 3 + item_effective_level(&p->weapon) * 2;
        dmg = rng_range(1, top > 1 ? top : 1);
    } else {
        int str = player_str(p);
        dmg = (str > 10) ? rng_range(1, str - 9) : 1;
    }

    return char_buff(self, BUFF_FURY) ? (int)(dmg * 1.5f) : dmg;
}

static int hero_attack_proc(Char *self, Char *enemy, int damage) {
    (void)self; (void)enemy;
    /* Placeholder — the original chains wep.proc(this, enemy, damage)
       here, then a subclass switch (Gladiator's Combo stacks, a
       Battlemage charging a wielded wand, a Sniper marking the enemy
       for bonus damage). */
    return damage;
}

static int hero_defense_proc(Char *self, Char *attacker, int damage) {
    (void)self; (void)attacker;
    /* Placeholder — the original checks Ring of Thorns (reflects a
       cut of damage back), an Earthroot-armor buff (flat absorb), and
       armor.proc(). */
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
        default:
            break;
    }
    /* original also GLog.w()'s a "you are X" message per buff type here */
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
        vt.attack_proc   = hero_attack_proc;
        vt.defense_proc  = hero_defense_proc;
        vt.on_damaged    = hero_on_damaged;
        vt.die           = hero_die;
        vt.on_buff_added = hero_on_buff_added;

        built = 1;
    }
    return &vt;
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

void player_earn_exp(Player *p, int exp) {
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

void player_rest(Player *p, int till_healthy) {
    char_spend(&p->base, PLAYER_TIME_TO_REST);
    actor_next(&p->base.base);
    p->resting = till_healthy;
}

/*═══════════════════════════════════════════════════════════════
    Initialization
═══════════════════════════════════════════════════════════════*/
static void hero_receive_kill_xp(Char *killer, int xp) {
    if (killer->kind == CHAR_KIND_HERO) {
        player_earn_exp((Player *)killer, xp);
    }
}

void player_init(Player *p, const Level *lvl, HeroClass cls){
    char_init(&p->base, hero_vtable(), CHAR_KIND_HERO, lvl->entrance, HERO_STARTING_HP);
    p->base.name = "you";
    char_add(&p->base);

    p->cls         = cls;
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

    p->sub_class          = HERO_SUB_NONE;
    p->STR                = HERO_STARTING_STR;
    p->weakened            = 0;
    p->lvl                 = 1;
    p->exp                 = 0;
    p->awareness           = HERO_STARTING_AWARENESS;
    p->base_attack_skill   = HERO_STARTING_ATTACK;
    p->base_defense_skill  = HERO_STARTING_DEFENSE;
    p->ranged_attack       = 0;
    p->resting             = 0;
    p->attack_target       = NULL;

    /* STARTING ITEMS */
    player_add_item(p, ITEM_SCROLL_UPGRADE, 1);//TEST
    player_add_item(p, ITEM_BAG_KEYRING, 1);

    mob_set_hero(&p->base);
    mob_set_kill_reward_hook(hero_receive_kill_xp);
}

/*═══════════════════════════════════════════════════════════════
    Animation
═══════════════════════════════════════════════════════════════*/
int player_is_busy(const Player *p) {
    if (!char_is_alive(&p->base))
        return 1;

    return char_is_busy(&p->base);
}

static void player_sync_anim_lock(Player *p) {
    switch (p->anim) {
    //case ANIM_RUN: //TEST
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

void player_play_anim(Player *p, AnimState anim){
    if (p->anim == anim && !p->anim_done)
        return;

    p->anim = anim;
    p->frame = 0;
    p->frame_clock = 0.0f;
    p->anim_done = 0;
    player_sync_anim_lock(p);
}

static void player_resolve_attack(Player *p) {
    Char *target = p->attack_target;
    p->attack_target = NULL;
    if (!target) return; 

    char_attack(&p->base, target);
    char_on_attack_complete(&p->base);
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
    Inventory helpers 
═══════════════════════════════════════════════════════════════*/
static ItemStack *equip_slot_ptr(Player *p, EquipSlot slot) {
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

int player_add_stack(Player *p, ItemStack stack) {
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

int player_add_item(Player *p, ItemType type, int quantity) {
    ItemStack stack;
    item_stack_init(&stack, type, quantity);
    return player_add_stack(p, stack);
}

/* //TODO UNUSED FUNCTIONS FROM HERE TO THE NEXT PART
int player_remove_item(Player *p, int slot, int quantity) {
    return bag_remove_item(&p->bags[BAG_BACKPACK], slot, quantity);
}

int player_find_item(const Player *p, ItemType type) {
    return bag_find_item(&p->bags[BAG_BACKPACK], type);
}
*/

/*═══════════════════════════════════════════════════════════════
    Equipment
═══════════════════════════════════════════════════════════════*/
int player_equip_item(Player *p, Bag *bag, int slot) {
    if (player_is_busy(p))
        return 0;
    if (!bag || slot < 0 || slot >= bag->count)
        return 0;

    ItemStack incoming = bag->items[slot];
    ItemCategory cat = ITEM_DB[incoming.type].cat;

    EquipSlot target;
    if (cat == ITEMCAT_WEAPON) {
        target = EQUIP_WEAPON;
    } else if (cat == ITEMCAT_ARMOR) {
        target = EQUIP_ARMOR;
    } else if (cat == ITEMCAT_RING) {
        if (p->ring1.type == ITEM_NONE)
            target = EQUIP_RING1;
        else if (p->ring2.type == ITEM_NONE)
            target = EQUIP_RING2;
        else
            return 2; // redo
    } else {
        return 0;
    }

    ItemStack *dst = equip_slot_ptr(p, target);

    if (dst->type != ITEM_NONE && dst->cursed) {
        return 0;
    }

    if (!bag_remove_item(bag, slot, 1)) 
        return 0;

    if (dst->type != ITEM_NONE) {
        if (!player_add_stack(p, *dst)) {
            player_add_stack(p, incoming);
            return 0;
        }
    }
    *dst = incoming;
    dst->quantity = 1; /*TODO MB REWRITE??? equipped gear is always exactly one unit, even if it came off a stack of stackable weapons */
    dst->cursed_known = 1;

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
        player_sort_inventory(p);
        return 1;
    }

    if (lvl) {
        Heap *h = level_heap_at(lvl, p->base.pos);

        if (h && heap_add_stack(h, stack)) {
            *src = item_stack_none();
            player_sort_inventory(p);
            return 1;
        }
    }

    return 0; 
}

int player_equip_ring(Player *p, Bag *bag, int slot, EquipSlot target) {
    if (player_is_busy(p))
        return 0;
    if (!bag || slot < 0 || slot >= bag->count)
        return 0;

    ItemStack *dst = equip_slot_ptr(p, target);

    if (dst->type != ITEM_NONE && dst->cursed)
        return 0;

    ItemStack incoming = bag->items[slot];

    if (!bag_remove_item(bag, slot, 1))
        return 0;

    if (dst->type != ITEM_NONE)
        player_add_stack(p, *dst);

    *dst = incoming;
    dst->quantity = 1;
    dst->cursed_known = 1;

    player_sort_inventory(p);

    return 1;
}
/*═══════════════════════════════════════════════════════════════
    Bags
═══════════════════════════════════════════════════════════════*/
int player_has_bag(const Player *p, BagType type) {
    if (type == BAG_BACKPACK) return 1; 
    if (type < 0 || type >= BAG_TYPE_COUNT) return 0;

    return bag_find_item(&p->bags[BAG_BACKPACK], bag_item_type(type)) >= 0;
}

Bag *player_get_bag(Player *p, BagType type) {
    return player_has_bag(p, type) ? &p->bags[type] : NULL;
}

Bag *player_bag_for_item(Player *p, ItemType item_type) {
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

    return bag_remove_item(bag, slot, stack.quantity);
}


/*═══════════════════════════════════════════════════════════════
    Ground interaction
═══════════════════════════════════════════════════════════════*/
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

    return 1;
}

int player_drop_item(Player *p, Level *lvl, int slot) {//redo as player_drop_from_bag(&player, lvl, &player.bags[inv.active_bag], inv.cursor); for main.c
    return player_drop_from_bag(p, lvl, &p->bags[BAG_BACKPACK], slot);
}

int player_operate(Player *p, Level *lvl) {
    if (player_is_busy(p))
        return 0;
    
    Heap *h = level_heap_at(lvl, p->base.pos);
    if (!h)
        return 0;
    if (h->type == HEAP_NORMAL || h->type == HEAP_FOR_SALE)
        return 0;
    if ((h->type == HEAP_LOCKED_CHEST || h->type == HEAP_CRYSTAL_CHEST)&& !player_use_key(p, ITEM_KEY_GOLDEN))
        return 0;
    player_play_anim(p, ANIM_OPERATE);
    h->type = HEAP_NORMAL;
    char_spend(&p->base, PLAYER_TIME_TO_UNLOCK);
    actor_next(&p->base.base);
    return 1;
}

/*═══════════════════════════════════════════════════════════════
    World interaction
═══════════════════════════════════════════════════════════════*/
int player_move(Player *p, Level *lvl, int dx, int dy){
    int idx = p->base.pos + xy_to_idx(dx, dy);
    uint8_t tile = lvl->map[idx];

    if (!in_bounds(idx)) return 0;
    if (player_is_busy(p)) return 0;
    
    switch (tile) {
    case TILE_LOCKED_DOOR:
        if (!player_use_key(p, ITEM_KEY_IRON))
            return 0;

        lvl->map[idx] = TILE_OPEN_DOOR;
        char_spend(&p->base, PLAYER_TIME_TO_UNLOCK);
        actor_next(&p->base.base);
        player_play_anim(p, ANIM_OPERATE);
        return 1; 
    case TILE_LOCKED_EXIT:
        //if (!player_use_key(p, ITEM_KEY_SKELETON)) return 0; /*TODO: REMOVE COMMENT AFTER ADDING BOSSES WITH KEY DROPS */
        lvl->map[idx] = TILE_UNLOCKED_EXIT;
        char_spend(&p->base, PLAYER_TIME_TO_UNLOCK);
        actor_next(&p->base.base);
        player_play_anim(p, ANIM_OPERATE);
        return 1;
    case TILE_HIGH_GRASS:
        lvl->map[idx] = TILE_GRASS;
        //TODO SPAWN SEEDS + ITEM_DEWDROP 
        //IS IT HAS ANIMATION???
    }
    if (!(gTileFlags[tile] & TF_PASSABLE)) return 0;


    if (dx != 0) p->facing = (dx < 0) ? FACE_LEFT : FACE_RIGHT;

    char_move(&p->base, lvl, idx);

    char_spend(&p->base, 1.0f / char_speed(&p->base));
    actor_next(&p->base.base); 

    return 1;
}

static float hero_attack_delay(const Player *p) {
    (void)p;
    /* Placeholder for wep.speedFactor(this) */
    return 1.0f;
}

static int player_start_attack(Player *p, Char *target) {
    if (player_is_busy(p)) return 0;
    if (!char_is_alive(&p->base) || !char_is_alive(target)) return 0;
    if (char_is_charmed_by(&p->base, target)) return 0; /* mirrors actAttack()'s !isCharmedBy(enemy) guard */

    p->attack_target = target;
    char_spend(&p->base, hero_attack_delay(p));
    player_play_anim(p, ANIM_ATTACK);
    char_begin_attack_anim(&p->base, target);
    player_resolve_attack(p);
    
    return 1;
}

int player_bump(Player *p, Level *lvl, int dx, int dy) {
    if (player_is_busy(p)) return PLAYER_BUMP_BLOCKED;

    int idx = p->base.pos + xy_to_idx(dx, dy);

    if (!in_bounds(idx)) return PLAYER_BUMP_BLOCKED;

    if (dx != 0) p->facing = (dx < 0) ? FACE_LEFT : FACE_RIGHT;

    Char *occupant = char_find_at(idx);
    if (occupant && occupant->kind == CHAR_KIND_MOB) {
        return player_start_attack(p, occupant) ? PLAYER_BUMP_ATTACKED : PLAYER_BUMP_BLOCKED;
    }
    if (occupant && occupant->kind == CHAR_KIND_NPC) {
        /* HeroAction.java's Interact case. npc.c has no concrete
           NPCs wired up yet*/
        npc_interact((NPC *)occupant, &p->base);
        return PLAYER_BUMP_INTERACTED;
    }
    if (occupant) {
        return PLAYER_BUMP_BLOCKED;
    }

    return player_move(p, lvl, dx, dy) ? PLAYER_BUMP_MOVED : PLAYER_BUMP_BLOCKED;
}