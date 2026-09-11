#pragma once
#include "char.h"
#include "sprite_db.h"

typedef struct NPC NPC;

typedef struct NPCVTable {
    void (*interact)(NPC *self, Char *player);
} NPCVTable;

typedef enum {
    NPC_ANIM_IDLE = 0,
    NPC_ANIM_RUN,
    NPC_ANIM_ATTACK,
    NPC_ANIM_DIE,
    NPC_ANIM_COUNT
} NpcAnimState;

struct NPC {
    Char base;

    const NPCVTable *ivt;
    Facing facing;

    int kind;

    NpcAnimState anim;
    int          frame;
    float        frame_clock;
    int          anim_done;
};

/* ── Lifecycle ────────────────────────────────────────────── */
void npc_init(NPC *self, const NPCVTable *ivt, int pos, const char *name, int ht);

/* ── Animation ────────────────────────────────────────────── */
void npc_play_anim(NPC *self, NpcAnimState anim);
void npc_animate_all(float dt);

/* ── Pool ────────────────────────────────────────────────────────────── */
#define MAX_LEVEL_NPCS 4

typedef struct {
    NPC npcs[MAX_LEVEL_NPCS];
    int count;
} NPCPool;

void npc_set_pool(NPCPool *pool);

NPC *npc_spawn(const NPCVTable *ivt, int pos, const char *name, int ht);

void npc_unload_all(void);
void npc_clear_all(void);
void npc_restore_grid(void);

int  npc_count(void);
NPC *npc_at_index(int i);


typedef enum {
    NPC_GHOST = 0,
    NPC_BLACKSMITH,
    NPC_IMP,
    NPC_IMP_SHOPKEEPER,
    NPC_WANDMAKER,
    NPC_MIRROR_IMAGE,
    NPC_BEE,
    NPC_RATKING,
    NPC_SHOPKEEPER,
    NPC_KIND_COUNT
} NpcKind;

typedef struct {
    const char *name;
    int         ht;
} NpcStats;

extern const NpcStats NPC_DB[NPC_KIND_COUNT];

NPC *npc_spawn_kind(NpcKind kind, int pos);

typedef struct {
    int      frame_w, frame_h;
    AnimClip clips[NPC_ANIM_COUNT];
} NpcSpriteInfo;

extern const NpcSpriteInfo NPC_SPRITES[NPC_KIND_COUNT];
extern const NpcTexId NPC_TEX_FOR_KIND[NPC_KIND_COUNT];
extern const AnimClip *const MIRROR_HERO_CLIPS[NPC_ANIM_COUNT];

/* ── Interaction (HeroAction.java's Interact case) ──────────── */
void npc_interact(NPC *self, Char *player);

void npc_throw_item(NPC *self, Level *lvl);