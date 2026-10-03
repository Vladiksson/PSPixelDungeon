#include "npc.h"
#include "item_spawn.h"
#include "level_geometry.h" 

/*═══════════════════════════════════════════════════════════════
    Storage
═══════════════════════════════════════════════════════════════*/
static NPCPool *active_npc_pool = NULL;

void npc_set_pool(NPCPool *pool) {
    active_npc_pool = pool;
}

/*═══════════════════════════════════════════════════════════════
    Turn / vtable
═══════════════════════════════════════════════════════════════*/
static int npc_on_turn(Char *self) {
    char_spend(self, ACTOR_TICK);
    actor_next(&self->base);
    return 0;
}

static void npc_interact_stub(NPC *self, Char *player) {
    (void)self; (void)player;
    //TODO PLAYER INTERACTION WITH NPCS
}

static const NPCVTable NPC_DEFAULT_VTABLE = {
    .interact = npc_interact_stub,
};

static const CharVTable *npc_vtable(void) {
    static CharVTable vt;
    static int built = 0;
    if (!built) {
        vt = CHAR_DEFAULT_VTABLE;
        vt.on_turn = npc_on_turn;
        built = 1;
    }
    return &vt;
}

/*═══════════════════════════════════════════════════════════════
    Lifecycle
═══════════════════════════════════════════════════════════════*/
void npc_init(NPC *self, const NPCVTable *ivt, int pos, const char *name, int ht) {
    char_init(&self->base, npc_vtable(), CHAR_KIND_NPC, pos, ht > 0 ? ht : 1);
    self->base.name = name ? name : "npc";
    self->ivt    = ivt ? ivt : &NPC_DEFAULT_VTABLE;
    self->facing = FACE_RIGHT;
    self->kind        = NPC_KIND_COUNT;
    self->anim        = NPC_ANIM_IDLE;
    self->frame       = 0;
    self->frame_clock = 0.0f;
    self->anim_done   = 0;
}

NPC *npc_spawn(const NPCVTable *ivt, int pos, const char *name, int ht) {
    if (!active_npc_pool) return NULL;
    if (char_find_at(pos) != NULL) return NULL;
    if (active_npc_pool->count >= MAX_LEVEL_NPCS) return NULL;

    /* No dead-slot reuse yet, unlike mob_spawn() */
    NPC *n = &active_npc_pool->npcs[active_npc_pool->count];
    npc_init(n, ivt, pos, name, ht);

    if (!char_add(&n->base)) return NULL;

    active_npc_pool->count++;
    return n;
}


void npc_unload_all(void) { //temporary solution of npcs saving after level transition... yeah it's temporar copy of temporar mobs bullshit
    if (!active_npc_pool) return;

    for (int i = 0; i < active_npc_pool->count; i++) {
        NPC *n = &active_npc_pool->npcs[i];
        if (char_is_alive(&n->base))
            char_unregister(&n->base);
    }
}

void npc_clear_all(void) { //SAME BULLSHIT AS IN MOBS, DO I REALLY NEED IT???
    if (!active_npc_pool) return;
    for (int i = 0; i < active_npc_pool->count; i++) {
        char_destroy(&active_npc_pool->npcs[i].base);
    }
    active_npc_pool->count = 0;
}

void npc_restore_grid(void) { //same as unload, mb add restoring target?
    for (int i = 0; i < active_npc_pool->count; i++) {
        NPC *n = &active_npc_pool->npcs[i];
        if (!char_is_alive(&n->base))
            continue;
        n->base.base.time = 0.0f;
        char_add(&n->base);
    }
}

int npc_count(void) {
    return active_npc_pool ? active_npc_pool->count : 0;
}

NPC *npc_at_index(int i) {
    if (!active_npc_pool || i < 0 || i >= active_npc_pool->count) return NULL;
    return &active_npc_pool->npcs[i];
}

/*═══════════════════════════════════════════════════════════════
    Animation
═══════════════════════════════════════════════════════════════*/
static void npc_sync_anim_lock(NPC *n) {
    switch (n->anim) {
        case NPC_ANIM_ATTACK:
        case NPC_ANIM_DIE:
            n->base.anim_locked = !n->anim_done;
            break;
        default:
            n->base.anim_locked = 0;
    }
}

void npc_play_anim(NPC *self, NpcAnimState anim) {
    if (self->anim == anim && !self->anim_done)
        return;

    self->anim        = anim;
    self->frame        = 0;
    self->frame_clock  = 0.0f;
    self->anim_done    = 0;
    npc_sync_anim_lock(self);
}

void npc_animate_all(float dt) {
    if (!active_npc_pool) return;

    for (int i = 0; i < active_npc_pool->count; i++) {
        NPC *n = &active_npc_pool->npcs[i];

        if (!char_is_alive(&n->base) && n->anim_done)
            continue;

        if (n->kind < 0 || n->kind >= NPC_KIND_COUNT)
            continue;

        const AnimClip *clip;

        if (n->kind == NPC_MIRROR_IMAGE)
            clip = MIRROR_HERO_CLIPS[n->anim];
        else
            clip = &NPC_SPRITES[n->kind].clips[n->anim];

        if (n->anim_done || clip->fps <= 0 || clip->count <= 0)
            continue;

        n->frame_clock += dt;
        float step = 1.0f / (float)clip->fps;
        while (n->frame_clock >= step) {
            n->frame_clock -= step;
            n->frame++;

            if (n->frame >= clip->count) {
                if (clip->loop) {
                    n->frame = 0;
                } else {
                    n->frame = clip->count - 1;
                    n->anim_done = 1;
                    break;
                }
            }
        }
        if (n->anim_done) {
            switch (n->anim) {
                case NPC_ANIM_ATTACK:
                    char_end_attack_anim(&n->base);
                    npc_play_anim(n, NPC_ANIM_IDLE);
                    break;
                case NPC_ANIM_RUN:
                    npc_play_anim(n, NPC_ANIM_IDLE);
                    break;
                case NPC_ANIM_DIE:
                    break;
                default:
                    break;
            }
        }
        npc_sync_anim_lock(n);
    }
}

/*═══════════════════════════════════════════════════════════════
    NPC_DB
═══════════════════════════════════════════════════════════════*/
const NpcStats NPC_DB[NPC_KIND_COUNT] = {

    /* ── Quest-dialogue NPCs. All four hard-code defenseSkill()
       -> 1000 and no-op both damage() and add(Buff) in the
       original — meant to be unhittable outright, not just
       tanky. None of that is modeled here yet, so right now these
       take ordinary damage off their 1 HP like any other Char.
       All four also call throwItem() unconditionally every turn
       in the original, which npc_on_turn() deliberately doesn't do
       (not every NPC wants it). */
    [NPC_GHOST] = {
        .name = "sad ghost", .ht = 1,
        /* also: flying, half speed, chooseEnemy() always returns
           null (can never become hostile), immune to Paralysis and
           Roots. Quest is one of 3 variants picked at spawn, only
           in the Sewers. */
    },
    [NPC_BLACKSMITH] = {
        .name = "troll blacksmith", .ht = 1,
        /* Quest also unlocks a separate item-reforge service after
           completion — a whole second interact() branch beyond the
           quest dialogue itself. */
    },
    [NPC_IMP] = {
        .name = "ambitious imp", .ht = 1,
        /* despawns itself once its bounty quest is turned in. */
    },
    [NPC_WANDMAKER] = {
        .name = "old wandmaker", .ht = 1,
        /* Quest is one of 3 variants picked at spawn, like Ghost's,
           only in the Prison. */
    },

    /* ── Trade. Extends Shopkeeper below, not NPC, in the original
       — same name and sprite sheet as NPC_IMP above, different
       role (runs a shop instead of the bounty quest). Not modeled
       as Shopkeeper-derived here since neither has real behavior
       yet; worth revisiting once Shopkeeper's own logic exists. */
    [NPC_IMP_SHOPKEEPER] = {
        .name = "ambitious imp", .ht = 1,
    },
    [NPC_SHOPKEEPER] = {
        .name = "shopkeeper", .ht = 1,
        /* no Quest/Journal dependency, but does need a buy/sell
           window before interact() means anything. In the original,
           any damage or buff triggers an immediate flee()/despawn
           rather than actually losing HP — not modeled here yet. */
    },

    /* ── Combat allies, not dialogue NPCs — interact() swaps
       position with the hero instead of opening anything. Both
       override chooseEnemy() to pick a real target and both take
       real damage, unlike every other entry in this table; each
       ends itself on its own terms (Bee loses 1 HP every turn,
       MirrorImage dies the instant it lands one hit). Neither
       override is modeled here yet. */
    [NPC_MIRROR_IMAGE] = {
        .name = "mirror image", .ht = 1,
        /* attack/damage are copied from the hero at spawn time in
           the original, not a fixed stat. Also the one entry with
           no sprite_asset — MirrorSprite's sheet isn't in assets.h
           yet, unlike the other 8. */
    },
    [NPC_BEE] = {
        .name = "golden bee", .ht = 1,
        /* ht=1 here is a placeholder, NOT the real value — HT
           isn't fixed in the original, spawn(level) sets it to
           (3+level)*5 (and defenseSkill to 9+level) at spawn time.
           Pass the level-scaled HP straight to npc_spawn() instead
           of using this table until spawn(level) has a real home. */
    },

    /* ── Flavor only. No Quest, no Journal, no spawn() of its own
       in this file — something else places it. chooseEnemy()
       always returns null, same as Ghost — can never become
       hostile either. */
    [NPC_RATKING] = {
        .name = "rat king", .ht = 1,
    },
};

NPC *npc_spawn_kind(NpcKind kind, int pos) {
    if (kind < 0 || kind >= NPC_KIND_COUNT) return NULL;
    const NpcStats *s = &NPC_DB[kind];
    NPC *n = npc_spawn(NULL, pos, s->name, s->ht);
    if (n) n->kind = kind; /* npc_init() left this at the NPC_KIND_COUNT sentinel */
    return n;
}

/*═══════════════════════════════════════════════════════════════
    Interaction
═══════════════════════════════════════════════════════════════*/
void npc_interact(NPC *self, Char *player) {
    if (!self || !self->ivt || !self->ivt->interact) return;
    self->ivt->interact(self, player);
}

/*═══════════════════════════════════════════════════════════════
    throwItem() port
═══════════════════════════════════════════════════════════════*/
void npc_throw_item(NPC *self, Level *lvl) {
    if (!self || !lvl) return;

    Point p = idx_to_xy(self->base.pos);
    Heap *h = level_heap_at(lvl, self->base.pos);
    if (!h) return;

    int dest = -1;
    for (int tries = 0; tries < 32 && dest < 0; tries++) {
        int i = rng_int(8);
        int nx = p.x + DX[i];
        int ny = p.y + DY[i];
        int npos = xy_to_idx(nx, ny);
        if (!in_bounds(npos)) continue;

        if ((cell_passable(lvl, npos))) dest = npos;
    }
    if (dest < 0) return;

    Heap *dst = level_heap_at(lvl, dest);
    if (!dst)
        dst = level_create_heap(lvl, dest, HEAP_NORMAL);
    if (!dst) return;

    for (int i = 0; i < h->count; i++) {
        heap_add_stack(dst, h->items[i]);
    }
    *h = lvl->heaps[--lvl->num_heaps];
    /* original also plays heap.sprite.drop(pos) here — a visual
       toss, presentation-layer, not ported yet. */
}